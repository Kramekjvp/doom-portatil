/*
 * ILI9341 240x320 por SPI no ESP32-S3 (esp_lcd panel IO SPI, ESP-IDF 6.x). ADR-0013.
 *
 * Irmao do lcd_i80.c: a sequencia de init, as janelas e o desenho sao os mesmos;
 * so a criacao do barramento muda. Diferencas que importam:
 *  - No SPI NAO existe troca automatica de bytes das cores (o swap_color_bytes e so do i80):
 *    o ILI9341 quer o byte ALTO primeiro, entao trocamos os bytes ao preencher o buffer.
 *  - O buffer de DMA e alocado aqui, em memoria interna com capacidade de DMA.
 *
 * Todo desenho passa por UM buffer de DMA interno (STAGE_PIXELS), enviado em faixas de linhas
 * inteiras, cada uma com sua janela (CASET/PASET/RAMWR).
 *
 * Compila vazio se a tela escolhida for o shield paralelo (CONFIG_PORTATIL_TELA_SPI desligado).
 */
#include "sdkconfig.h"

#if CONFIG_PORTATIL_TELA_SPI

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_io_spi.h"
#include "gfx.h"
#include "lcd_spi.h"

static const char *TAG = "lcd_spi";

#define LCD_HOST       SPI2_HOST       /* FSPI: CS 10, MOSI 11, SCK 12 sao os pinos nativos */
#define LCD_NATIVE_W   240             /* retrato */
#define LCD_NATIVE_H   320
/* 12,8 KB de DMA interno. Em paisagem (320 px/linha) sao faixas de 20 linhas. */
#define STAGE_PIXELS   6400

#define ILI_SWRESET    0x01
#define ILI_SLPOUT     0x11
#define ILI_INVON      0x21
#define ILI_DISPON     0x29
#define ILI_CASET      0x2A
#define ILI_PASET      0x2B
#define ILI_RAMWR      0x2C
#define ILI_MADCTL     0x36
#define ILI_PIXFMT     0x3A

static int s_w = LCD_NATIVE_W, s_h = LCD_NATIVE_H;   /* dimensoes da orientacao atual */
static esp_lcd_panel_io_handle_t s_io;
static SemaphoreHandle_t         s_done;
static uint16_t                 *s_stage;     /* buffer DMA interno (cores ja com os bytes trocados) */

typedef struct {
    uint8_t cmd;
    uint8_t len;
    uint8_t data[15];
} init_cmd_t;

/* Mesma sequencia de init do lcd_i80.c (TFT_eSPI / Adafruit), sem MADCTL e sem SLPOUT/DISPON */
static const init_cmd_t INIT[] = {
    { 0xEF, 3,  { 0x03, 0x80, 0x02 } },
    { 0xCF, 3,  { 0x00, 0xC1, 0x30 } },
    { 0xED, 4,  { 0x64, 0x03, 0x12, 0x81 } },
    { 0xE8, 3,  { 0x85, 0x00, 0x78 } },
    { 0xCB, 5,  { 0x39, 0x2C, 0x00, 0x34, 0x02 } },
    { 0xF7, 1,  { 0x20 } },
    { 0xEA, 2,  { 0x00, 0x00 } },
    { 0xC0, 1,  { 0x23 } },                       /* power control: VRH        */
    { 0xC1, 1,  { 0x10 } },                       /* power control: SAP, BT    */
    { 0xC5, 2,  { 0x3E, 0x28 } },                 /* VCOM 1                    */
    { 0xC7, 1,  { 0x86 } },                       /* VCOM 2                    */
    { ILI_PIXFMT, 1, { 0x55 } },                  /* 16 bits por pixel         */
    { 0xB1, 2,  { 0x00, 0x13 } },                 /* taxa de quadros ~100 Hz   */
    { 0xB6, 3,  { 0x08, 0x82, 0x27 } },           /* display function control  */
    { 0xF2, 1,  { 0x00 } },                       /* 3-gamma desligado         */
    { 0x26, 1,  { 0x01 } },                       /* curva gama 1              */
    { 0xE0, 15, { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                  0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 } },
    { 0xE1, 15, { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                  0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F } },
};

/* RGB565 nativo (little-endian na memoria) -> byte alto primeiro no fio */
static inline uint16_t troca_bytes(uint16_t c)
{
    return (uint16_t)((c << 8) | (c >> 8));
}

static bool IRAM_ATTR on_color_done(esp_lcd_panel_io_handle_t io,
                                    esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    (void)io;
    (void)edata;
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR((SemaphoreHandle_t)user_ctx, &woken);
    return woken == pdTRUE;
}

static esp_err_t cmd(uint8_t c, const void *param, size_t len)
{
    return esp_lcd_panel_io_tx_param(s_io, c, param, len);
}

static esp_err_t set_window(int x, int y, int w, int h)
{
    const int x1 = x + w - 1, y1 = y + h - 1;
    const uint8_t ca[4] = { (uint8_t)(x >> 8), (uint8_t)x, (uint8_t)(x1 >> 8), (uint8_t)x1 };
    const uint8_t pa[4] = { (uint8_t)(y >> 8), (uint8_t)y, (uint8_t)(y1 >> 8), (uint8_t)y1 };
    ESP_RETURN_ON_ERROR(cmd(ILI_CASET, ca, 4), TAG, "CASET");
    return cmd(ILI_PASET, pa, 4);
}

/* Envia os primeiros `pixels` do s_stage para a janela (x,y,w,h) e espera o DMA */
static void send_stage(int x, int y, int w, int h, size_t pixels)
{
    esp_err_t err = set_window(x, y, w, h);
    if (err == ESP_OK) err = esp_lcd_panel_io_tx_color(s_io, ILI_RAMWR, s_stage, pixels * 2);
    if (err == ESP_OK && xSemaphoreTake(s_done, pdMS_TO_TICKS(200)) != pdTRUE) err = ESP_ERR_TIMEOUT;
    if (err != ESP_OK) ESP_LOGE(TAG, "falha ao enviar retangulo (%d,%d %dx%d): %s", x, y, w, h, esp_err_to_name(err));
}

/* ---------------- backend de gfx.h ---------------- */

int gfx_width(void)  { return s_w; }
int gfx_height(void) { return s_h; }

void gfx_fill_rect(int x, int y, int w, int h, uint16_t color)
{
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > s_w) w = s_w - x;
    if (y + h > s_h) h = s_h - y;
    if (w <= 0 || h <= 0) return;

    int rows = STAGE_PIXELS / w;
    if (rows > h) rows = h;
    const size_t n = (size_t)rows * (size_t)w;
    const uint16_t c = troca_bytes(color);
    for (size_t i = 0; i < n; i++) s_stage[i] = c;

    while (h > 0) {
        int r = (h < rows) ? h : rows;
        send_stage(x, y, w, r, (size_t)r * (size_t)w);
        y += r;
        h -= r;
    }
}

void gfx_blit(int x, int y, int w, int h, const uint16_t *pixels)
{
    /* recorte: so envia a parte visivel */
    int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
    int x1 = x + w > s_w ? s_w : x + w, y1 = y + h > s_h ? s_h : y + h;
    int vw = x1 - x0, vh = y1 - y0;
    if (vw <= 0 || vh <= 0) return;

    const int rows = STAGE_PIXELS / vw;
    for (int yy = y0; yy < y1; ) {
        int r = (y1 - yy < rows) ? (y1 - yy) : rows;
        for (int i = 0; i < r; i++) {
            const uint16_t *src = &pixels[(yy + i - y) * w + (x0 - x)];
            uint16_t *dst = &s_stage[i * vw];
            for (int k = 0; k < vw; k++) dst[k] = troca_bytes(src[k]);
        }
        send_stage(x0, yy, vw, r, (size_t)r * (size_t)vw);
        yy += r;
    }
}

void gfx_set_orientation(gfx_orientation_t o)
{
    const bool land = (o == GFX_LANDSCAPE);
    const uint8_t madctl = (uint8_t)(land ? CONFIG_LCD_SPI_MADCTL_LANDSCAPE : CONFIG_LCD_SPI_MADCTL);
    if (cmd(ILI_MADCTL, &madctl, 1) != ESP_OK) {
        ESP_LOGE(TAG, "falha ao trocar MADCTL (orientacao)");
        return;
    }
    s_w = land ? LCD_NATIVE_H : LCD_NATIVE_W;
    s_h = land ? LCD_NATIVE_W : LCD_NATIVE_H;
    gfx_fill_rect(0, 0, s_w, s_h, 0x0000);
}

/* ---------------- init ---------------- */

static esp_err_t gpio_out(int pin, int level)
{
    if (pin < 0) return ESP_OK;
    const gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << pin,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&cfg), TAG, "gpio_config %d", pin);
    return gpio_set_level((gpio_num_t)pin, level);
}

esp_err_t lcd_spi_init(void)
{
    ESP_RETURN_ON_FALSE(s_io == NULL, ESP_ERR_INVALID_STATE, TAG, "ja inicializado");

    /* backlight (se estiver num GPIO) e reset por hardware */
    ESP_RETURN_ON_ERROR(gpio_out(CONFIG_LCD_SPI_PIN_BL, 1), TAG, "BL");
    if (CONFIG_LCD_SPI_PIN_RST >= 0) {
        ESP_RETURN_ON_ERROR(gpio_out(CONFIG_LCD_SPI_PIN_RST, 1), TAG, "RST");
        vTaskDelay(pdMS_TO_TICKS(5));
        gpio_set_level((gpio_num_t)CONFIG_LCD_SPI_PIN_RST, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_set_level((gpio_num_t)CONFIG_LCD_SPI_PIN_RST, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
    }

    s_done = xSemaphoreCreateBinary();
    ESP_RETURN_ON_FALSE(s_done, ESP_ERR_NO_MEM, TAG, "semaforo");

    const spi_bus_config_t bus_cfg = {
        .sclk_io_num = CONFIG_LCD_SPI_PIN_SCLK,
        .mosi_io_num = CONFIG_LCD_SPI_PIN_MOSI,
        .miso_io_num = -1,                       /* so escrevemos (SDO do modulo nao e usado) */
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = STAGE_PIXELS * (int)sizeof(uint16_t),
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(LCD_HOST, &bus_cfg, SPI_DMA_CH_AUTO), TAG, "spi_bus_initialize");

    const esp_lcd_panel_io_spi_config_t io_cfg = {
        .dc_gpio_num = CONFIG_LCD_SPI_PIN_DC,
        .cs_gpio_num = CONFIG_LCD_SPI_PIN_CS,
        .pclk_hz = CONFIG_LCD_SPI_CLK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 4,
        .on_color_trans_done = on_color_done,
        .user_ctx = s_done,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_cfg, &s_io),
                        TAG, "esp_lcd_new_panel_io_spi");

    s_stage = heap_caps_malloc(STAGE_PIXELS * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    ESP_RETURN_ON_FALSE(s_stage, ESP_ERR_NO_MEM, TAG, "buffer DMA");

    /* init do ILI9341 */
    ESP_RETURN_ON_ERROR(cmd(ILI_SWRESET, NULL, 0), TAG, "SWRESET");
    vTaskDelay(pdMS_TO_TICKS(150));

    for (size_t i = 0; i < sizeof(INIT) / sizeof(INIT[0]); i++) {
        ESP_RETURN_ON_ERROR(cmd(INIT[i].cmd, INIT[i].data, INIT[i].len), TAG, "init 0x%02X", INIT[i].cmd);
    }
    const uint8_t madctl = (uint8_t)CONFIG_LCD_SPI_MADCTL;
    ESP_RETURN_ON_ERROR(cmd(ILI_MADCTL, &madctl, 1), TAG, "MADCTL");
#if CONFIG_LCD_SPI_INVERT
    ESP_RETURN_ON_ERROR(cmd(ILI_INVON, NULL, 0), TAG, "INVON");
#endif
    ESP_RETURN_ON_ERROR(cmd(ILI_SLPOUT, NULL, 0), TAG, "SLPOUT");
    vTaskDelay(pdMS_TO_TICKS(120));
    ESP_RETURN_ON_ERROR(cmd(ILI_DISPON, NULL, 0), TAG, "DISPON");

    gfx_fill_rect(0, 0, s_w, s_h, 0x0000);
    ESP_LOGI(TAG, "ILI9341 SPI pronto (%d Hz)", CONFIG_LCD_SPI_CLK_HZ);
    return ESP_OK;
}

#endif /* CONFIG_PORTATIL_TELA_SPI */