/*
 * Teste da tela ILI9341 (shield paralelo de 8 bits) SO com GPIO simples.
 *
 * Nao usa o LCD_CAM nem o esp_lcd: mexe nos pinos um a um, devagar, como um Arduino faria.
 * Serve para separar duas causas de "tela branca":
 *   - mostra cores  -> fiacao e painel estao bons; o problema esta no driver do firmware;
 *   - continua branca -> problema de fiacao, de alimentacao ou o painel nao e ILI9341.
 *
 * Os pinos sao os padroes do firmware (hardware/pinos.md). Se voce mudou algum no
 * menuconfig, mude aqui tambem.
 *
 * ANTES de desenhar, pergunta ao painel quem ele e (le o ID do controlador) e imprime no log.
 * Para isso o fio RD do shield precisa estar no GPIO 21 (e NAO no 3V3).
 *
 * Sequencia na tela (repete): vermelho, verde, azul, branco, preto. Cada preenchimento
 * leva cerca de 1 s. Vermelho e azul trocados = fiacao certa; so o MADCTL difere.
 */
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "lcd_teste";

static const int PIN_D[8] = { 4, 5, 6, 7, 15, 16, 17, 18 };   /* LCD_D0 .. LCD_D7 */
#define PIN_WR   8
#define PIN_RS   9
#define PIN_CS   10
#define PIN_RST  14
#define PIN_RD   21    /* neste teste o RD tem de estar no GPIO 21 para ler o ID */

#define ATRASO_US  1   /* meio periodo do WR (us); 0 = o mais rapido que o GPIO simples faz */
#define LARGURA    240
#define ALTURA     320

static void atraso(void)
{
    if (ATRASO_US) esp_rom_delay_us(ATRASO_US);
}

static void saida(int pin, int nivel)
{
    gpio_reset_pin((gpio_num_t)pin);
    gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)pin, nivel);
}

static void escreve_byte(uint8_t b)
{
    for (int i = 0; i < 8; i++) gpio_set_level((gpio_num_t)PIN_D[i], (b >> i) & 1);
    gpio_set_level((gpio_num_t)PIN_WR, 0);
    atraso();
    gpio_set_level((gpio_num_t)PIN_WR, 1);   /* o painel le o dado na SUBIDA do WR */
    atraso();
}

static void comando(uint8_t c, const uint8_t *dados, int n)
{
    gpio_set_level((gpio_num_t)PIN_CS, 0);
    gpio_set_level((gpio_num_t)PIN_RS, 0);
    escreve_byte(c);
    gpio_set_level((gpio_num_t)PIN_RS, 1);
    for (int i = 0; i < n; i++) escreve_byte(dados[i]);
    gpio_set_level((gpio_num_t)PIN_CS, 1);
}

/* Mesma sequencia de init do driver do firmware (components/lcd_i80/lcd_i80.c) */
typedef struct { uint8_t cmd, len, data[15]; } init_cmd_t;
static const init_cmd_t INIT[] = {
    { 0xEF, 3,  { 0x03, 0x80, 0x02 } },
    { 0xCF, 3,  { 0x00, 0xC1, 0x30 } },
    { 0xED, 4,  { 0x64, 0x03, 0x12, 0x81 } },
    { 0xE8, 3,  { 0x85, 0x00, 0x78 } },
    { 0xCB, 5,  { 0x39, 0x2C, 0x00, 0x34, 0x02 } },
    { 0xF7, 1,  { 0x20 } },
    { 0xEA, 2,  { 0x00, 0x00 } },
    { 0xC0, 1,  { 0x23 } },
    { 0xC1, 1,  { 0x10 } },
    { 0xC5, 2,  { 0x3E, 0x28 } },
    { 0xC7, 1,  { 0x86 } },
    { 0x3A, 1,  { 0x55 } },                 /* 16 bits por pixel */
    { 0xB1, 2,  { 0x00, 0x13 } },
    { 0xB6, 3,  { 0x08, 0x82, 0x27 } },
    { 0xF2, 1,  { 0x00 } },
    { 0x26, 1,  { 0x01 } },
    { 0xE0, 15, { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                  0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 } },
    { 0xE1, 15, { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                  0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F } },
};

/* ---- leitura do ID do controlador (precisa do RD no GPIO 21) ---- */
static uint8_t le_barramento(void)
{
    uint8_t b = 0;
    for (int i = 0; i < 8; i++) b |= (uint8_t)((gpio_get_level((gpio_num_t)PIN_D[i]) & 1) << i);
    return b;
}

/* Depois de enviar o comando (RS=0), solta o barramento e le n bytes com RS=1. */
static void le_resposta(uint8_t *buf, int n)
{
    for (int i = 0; i < 8; i++) gpio_set_direction((gpio_num_t)PIN_D[i], GPIO_MODE_INPUT);
    gpio_set_level((gpio_num_t)PIN_RS, 1);
    for (int k = 0; k < n; k++) {
        gpio_set_level((gpio_num_t)PIN_RD, 0);
        esp_rom_delay_us(2);
        buf[k] = le_barramento();
        gpio_set_level((gpio_num_t)PIN_RD, 1);
        esp_rom_delay_us(2);
    }
    for (int i = 0; i < 8; i++) gpio_set_direction((gpio_num_t)PIN_D[i], GPIO_MODE_OUTPUT);
}

static void le_comando(uint8_t cmd, int n_zeros_antes, uint8_t *buf, int n)
{
    gpio_set_level((gpio_num_t)PIN_CS, 0);
    gpio_set_level((gpio_num_t)PIN_RS, 0);
    escreve_byte(cmd);
    for (int i = 0; i < n_zeros_antes; i++) escreve_byte(0x00);   /* indice de 16 bits (familia ILI932x) */
    le_resposta(buf, n);
    gpio_set_level((gpio_num_t)PIN_CS, 1);
}

static void identifica(void)
{
    uint8_t a[4] = {0}, b[4] = {0}, c[3] = {0};
    le_comando(0xD3, 0, a, 4);
    le_comando(0x04, 0, b, 4);
    le_comando(0x00, 1, c, 3);
    ESP_LOGI(TAG, "ID 0xD3  : %02X %02X %02X %02X   (ILI9341 responde 00 00 93 41)", a[0], a[1], a[2], a[3]);
    ESP_LOGI(TAG, "ID 0x04  : %02X %02X %02X %02X", b[0], b[1], b[2], b[3]);
    ESP_LOGI(TAG, "reg16 0  : %02X %02X %02X      (familia ILI932x responde 00 93 25 / 93 28 ...)", c[0], c[1], c[2]);
    if (a[2] == 0x93 && a[3] == 0x41) ESP_LOGI(TAG, ">> o painel respondeu como ILI9341");
    else ESP_LOGW(TAG, ">> nao respondeu como ILI9341 (tudo 00 ou tudo FF = leitura nao funcionou: RD no GPIO 21?)");
}

static void inicializa_tela(void)
{
    /* reset por hardware */
    gpio_set_level((gpio_num_t)PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(5));
    gpio_set_level((gpio_num_t)PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level((gpio_num_t)PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(150));

    identifica();                           /* pergunta quem e o painel (antes de qualquer init) */

    comando(0x01, NULL, 0);                 /* software reset */
    vTaskDelay(pdMS_TO_TICKS(150));
    for (size_t i = 0; i < sizeof INIT / sizeof INIT[0]; i++) comando(INIT[i].cmd, INIT[i].data, INIT[i].len);
    const uint8_t madctl = 0x48;            /* retrato */
    comando(0x36, &madctl, 1);
    comando(0x11, NULL, 0);                 /* sair do sleep */
    vTaskDelay(pdMS_TO_TICKS(120));
    comando(0x29, NULL, 0);                 /* ligar o display */
    vTaskDelay(pdMS_TO_TICKS(20));
}

static void preenche(uint16_t cor)
{
    const uint8_t ca[4] = { 0, 0, (LARGURA - 1) >> 8, (LARGURA - 1) & 0xFF };
    const uint8_t pa[4] = { 0, 0, (ALTURA - 1) >> 8, (ALTURA - 1) & 0xFF };
    comando(0x2A, ca, 4);
    comando(0x2B, pa, 4);

    gpio_set_level((gpio_num_t)PIN_CS, 0);
    gpio_set_level((gpio_num_t)PIN_RS, 0);
    escreve_byte(0x2C);                     /* escrita na memoria */
    gpio_set_level((gpio_num_t)PIN_RS, 1);
    for (int i = 0; i < LARGURA * ALTURA; i++) {
        escreve_byte((uint8_t)(cor >> 8));
        escreve_byte((uint8_t)(cor & 0xFF));
    }
    gpio_set_level((gpio_num_t)PIN_CS, 1);
}

void app_main(void)
{
    ESP_LOGI(TAG, "pinos: D0..D7 = 4,5,6,7,15,16,17,18 | WR=%d RS=%d CS=%d RST=%d RD=%d",
             PIN_WR, PIN_RS, PIN_CS, PIN_RST, PIN_RD);

    for (int i = 0; i < 8; i++) saida(PIN_D[i], 0);
    saida(PIN_WR, 1);
    saida(PIN_RS, 0);
    saida(PIN_CS, 1);
    saida(PIN_RST, 1);
    saida(PIN_RD, 1);

    inicializa_tela();
    ESP_LOGI(TAG, "init enviado; vou preencher a tela com cores (cada uma leva cerca de 1 s)");

    static const struct { const char *nome; uint16_t cor; } CORES[] = {
        { "vermelho", 0xF800 }, { "verde", 0x07E0 }, { "azul", 0x001F },
        { "branco",   0xFFFF }, { "preto", 0x0000 },
    };
    for (;;) {
        for (size_t i = 0; i < sizeof CORES / sizeof CORES[0]; i++) {
            ESP_LOGI(TAG, "preenchendo: %s", CORES[i].nome);
            preenche(CORES[i].cor);
            vTaskDelay(pdMS_TO_TICKS(1500));
        }
    }
}