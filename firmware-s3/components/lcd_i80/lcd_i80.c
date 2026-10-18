/*
 * ILI9341 240x320 em paralelo 8 bits no ESP32-S3 (esp_lcd i80, ESP-IDF 6.x).
 *
 * Projeto do driver:
 *  - Sem panel driver pronto: so o "panel IO" do esp_lcd (envia comando +
 *    parametros/cor). A sequencia de init do ILI9341 esta aqui, visivel e
 *    documentavel (e a mesma do TFT_eSPI, ILI9341_Init.h).
 *  - Todo desenho passa por UM buffer de DMA interno (STAGE_PIXELS) e e
 *    enviado em faixas de linhas inteiras, cada faixa com sua propria janela
 *    (CASET/PASET/RAMWR). Assim nao dependemos de "continuar" um RAMWR.
 *  - Cores chegam como uint16_t RGB565 nativo; o LCD_CAM troca os bytes
 *    (flags.swap_color_bytes) para o ILI9341 receber o byte alto primeiro.
 */
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_io_i80.h"
#include "gfx.h"
#include "lcd_i80.h"

static const char *TAG = "lcd_i80";

#define LCD_W          240
#define LCD_H          320
#define STAGE_PIXELS   2048            /* 4 KB de DMA; faixas de 8+ linhas */

#define ILI_SWRESET    0x01
#define ILI_SLPOUT     0x11
#define ILI_INVON      0x21
#define ILI_DISPON     0x29
#define ILI_CASET      0x2A
#define ILI_PASET      0x2B
#define ILI_RAMWR      0x2C
#define ILI_MADCTL     0x36
#define ILI_PIXFMT     0x3A

static esp_lcd_panel_io_handle_t s_io;
static SemaphoreHandle_t         s_done;
static uint16_t                 *s_stage;     /* buffer DMA interno */

typedef struct {
    uint8_t cmd;
    uint8_t len;
    uint8_t data[15];
} init_cmd_t;

/* Sequencia de init do ILI9341 (TFT_eSPI / Adafruit), sem MADCTL e sem SLPOUT/DISPON */
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

int gfx_width(void)  { return LCD_W; }
int gfx_height(void) { return LCD_H; }

void gfx_fill_rect(int x, int y, int w, int h, uint16_t color)
{
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > LCD_W) w = LCD_W - x;
    if (y + h > LCD_H) h = LCD_H - y;
    if (w <= 0 || h <= 0) return;

    int rows = STAGE_PIXELS / w;
    if (rows > h) rows = h;
    const size_t n = (size_t)rows * (size_t)w;
    for (size_t i = 0; i < n; i++) s_stage[i] = color;

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
    int x1 = x + w > LCD_W ? LCD_W : x + w, y1 = y + h > LCD_H ? LCD_H : y + h;
    int vw = x1 - x0, vh = y1 - y0;
    if (vw <= 0 || vh <= 0) return;

    const int rows = STAGE_PIXELS / vw;
    for (int yy = y0; yy < y1; ) {
        int r = (y1 - yy < rows) ? (y1 - yy) : rows;
        for (int i = 0; i < r; i++) {
            memcpy(&s_stage[i * vw], &pixels[(yy + i - y) * w + (x0 - x)], (size_t)vw * 2);
        }
        send_stage(x0, yy, vw, r, (size_t)r * (size_t)vw);
        yy += r;
    }
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

esp_err_t lcd_i80_init(void)
{
    ESP_RETURN_ON_FALSE(s_io == NULL, ESP_ERR_INVALID_STATE, TAG, "ja inicializado");

    /* RD fica em nivel alto (so escrevemos); RST faz o reset por hardware */
    ESP_RETURN_ON_ERROR(gpio_out(CONFIG_LCD_I80_PIN_RD, 1), TAG, "RD");
    if (CONFIG_LCD_I80_PIN_RST >= 0) {
        ESP_RETURN_ON_ERROR(gpio_out(CONFIG_LCD_I80_PIN_RST, 1), TAG, "RST");
        vTaskDelay(pdMS_TO_TICKS(5));
        gpio_set_level((gpio_num_t)CONFIG_LCD_I80_PIN_RST, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_set_level((gpio_num_t)CONFIG_LCD_I80_PIN_RST, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
    }

    s_done = xSemaphoreCreateBinary();
    ESP_RETURN_ON_FALSE(s_done, ESP_ERR_NO_MEM, TAG, "semaforo");

    esp_lcd_i80_bus_handle_t bus = NULL;
    const esp_lcd_i80_bus_config_t bus_cfg = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .dc_gpio_num = CONFIG_LCD_I80_PIN_DC,
        .wr_gpio_num = CONFIG_LCD_I80_PIN_WR,
        .data_gpio_nums = {
            CONFIG_LCD_I80_PIN_D0, CONFIG_LCD_I80_PIN_D1, CONFIG_LCD_I80_PIN_D2, CONFIG_LCD_I80_PIN_D3,
            CONFIG_LCD_I80_PIN_D4, CONFIG_LCD_I80_PIN_D5, CONFIG_LCD_I80_PIN_D6, CONFIG_LCD_I80_PIN_D7,
        },
        .bus_width = 8,
        .max_transfer_bytes = STAGE_PIXELS * sizeof(uint16_t),
        .dma_burst_size = 32,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_i80_bus(&bus_cfg, &bus), TAG, "esp_lcd_new_i80_bus");

    const esp_lcd_panel_io_i80_config_t io_cfg = {
        .cs_gpio_num = CONFIG_LCD_I80_PIN_CS,
        .pclk_hz = CONFIG_LCD_I80_PCLK_HZ,
        .trans_queue_depth = 4,
        .on_color_trans_done = on_color_done,
        .user_ctx = s_done,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .flags = {
            .swap_color_bytes = 1,   /* RGB565 nativo -> byte alto primeiro */
        },
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i80(bus, &io_cfg, &s_io), TAG, "esp_lcd_new_panel_io_i80");

    s_stage = esp_lcd_i80_alloc_draw_buffer(s_io, STAGE_PIXELS * sizeof(uint16_t), MALLOC_CAP_INTERNAL);
    ESP_RETURN_ON_FALSE(s_stage, ESP_ERR_NO_MEM, TAG, "buffer DMA");

    /* init do ILI9341 */
    ESP_RETURN_ON_ERROR(cmd(ILI_SWRESET, NULL, 0), TAG, "SWRESET");
    vTaskDelay(pdMS_TO_TICKS(150));

    for (size_t i = 0; i < sizeof(INIT) / sizeof(INIT[0]); i++) {
        ESP_RETURN_ON_ERROR(cmd(INIT[i].cmd, INIT[i].data, INIT[i].len), TAG, "init 0x%02X", INIT[i].cmd);
    }
    const uint8_t madctl = (uint8_t)CONFIG_LCD_I80_MADCTL;
    ESP_RETURN_ON_ERROR(cmd(ILI_MADCTL, &madctl, 1), TAG, "MADCTL");
#if CONFIG_LCD_I80_INVERT
    ESP_RETURN_ON_ERROR(cmd(ILI_INVON, NULL, 0), TAG, "INVON");
#endif
    ESP_RETURN_ON_ERROR(cmd(ILI_SLPOUT, NULL, 0), TAG, "SLPOUT");
    vTaskDelay(pdMS_TO_TICKS(120));
    ESP_RETURN_ON_ERROR(cmd(ILI_DISPON, NULL, 0), TAG, "DISPON");

    gfx_fill_rect(0, 0, LCD_W, LCD_H, 0x0000);
    ESP_LOGI(TAG, "ILI9341 i80 8 bits pronto (%d Hz)", CONFIG_LCD_I80_PCLK_HZ);
    return ESP_OK;
}
