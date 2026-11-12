/*
 * Tetris standalone no ESP32-S3: tela ILI9341 paralela (i80) + botoes em GPIO.
 * Este main faz o papel do "launcher": dono do laco de quadros. Quando o
 * launcher de verdade existir, ele chama o mesmo game_ops_t (tetris_game).
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "game.h"
#include "gfx.h"
#include "input.h"
#include "input_gpio.h"
#include "lcd_i80.h"
#include "tetris_game.h"

static const char *TAG = "main";

#define FRAME_MS 16   /* ~60 quadros/s */

/* Diagnostico no monitor serial (1 linha por segundo + 1 linha a cada botao).
 * Com a tela ainda nao ligada isto e o que mostra se o laco, o DMA e os botoes
 * estao funcionando. Coloque 0 para desligar. */
#ifndef DIAGNOSTICO
#define DIAGNOSTICO 1
#endif

static const char *const NOME_BOTAO[BTN_COUNT] = {
    "CIMA", "BAIXO", "ESQ", "DIR", "A", "B", "START", "SELECT",
};

static void lista_botoes(uint16_t mask, char *buf, size_t n)
{
    size_t len = 0;
    buf[0] = '\0';
    for (int b = 0; b < BTN_COUNT && len < n; b++) {
        if (!(mask & BTN_MASK(b))) continue;
        len += (size_t)snprintf(buf + len, n - len, "%s%s", len ? "+" : "", NOME_BOTAO[b]);
    }
    if (!len) snprintf(buf, n, "-");
}

/* Barras de cor + marcador no canto superior esquerdo: confere fiacao, ordem
 * de cor (BGR) e orientacao antes de jogar. Vermelho, verde, azul, de cima p/ baixo. */
static void tela_de_teste(void)
{
    const int w = gfx_width(), h = gfx_height();
    gfx_fill_rect(0,         0, w, h / 3,     GFX_RGB565(255, 0, 0));
    gfx_fill_rect(0,     h / 3, w, h / 3,     GFX_RGB565(0, 255, 0));
    gfx_fill_rect(0, 2 * h / 3, w, h - 2 * (h / 3), GFX_RGB565(0, 0, 255));
    gfx_fill_rect(0, 0, 24, 24, GFX_RGB565(255, 255, 255));            /* canto sup. esquerdo */
    gfx_text_centered(w / 2, h / 6 - 8,     "VERMELHO", 2, GFX_RGB565(255, 255, 255), GFX_RGB565(255, 0, 0));
    gfx_text_centered(w / 2, h / 2 - 8,     "VERDE",    2, GFX_RGB565(0, 0, 0),       GFX_RGB565(0, 255, 0));
    gfx_text_centered(w / 2, 5 * h / 6 - 8, "AZUL",     2, GFX_RGB565(255, 255, 255), GFX_RGB565(0, 0, 255));
}

static void roda_jogo(const game_ops_t *g)
{
    input_state_t in = { 0 };
    g->init();

    TickType_t acorda = xTaskGetTickCount();
    uint32_t anterior = (uint32_t)(esp_timer_get_time() / 1000);
#if DIAGNOSTICO
    uint32_t ult_log = anterior, quadros = 0, pior_us = 0;
#endif
    for (bool rodando = true; rodando; ) {
        const uint32_t agora = (uint32_t)(esp_timer_get_time() / 1000);
        input_state_update(&in, input_gpio_read());
        const game_ctx_t ctx = {
            .in = &in, .now_ms = agora, .dt_ms = agora - anterior, .entropy = esp_random(),
        };
        anterior = agora;

#if DIAGNOSTICO
        if (in.pressed || in.released) {
            char a[40], b[40];
            lista_botoes(in.down, a, sizeof a);
            lista_botoes(in.pressed, b, sizeof b);
            ESP_LOGI(TAG, "botoes segurados: %s (acabou de apertar: %s)", a, b);
        }
        const int64_t t0 = esp_timer_get_time();
#endif
        rodando = g->tick(&ctx);
#if DIAGNOSTICO
        const uint32_t dur_us = (uint32_t)(esp_timer_get_time() - t0);
        if (dur_us > pior_us) pior_us = dur_us;
        quadros++;
        if (agora - ult_log >= 1000) {
            ESP_LOGI(TAG, "%u quadros/s | pior tick %u us | heap interna livre %u B (minimo %u B)",
                     (unsigned)quadros, (unsigned)pior_us,
                     (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                     (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL));
            ult_log = agora; quadros = 0; pior_us = 0;
        }
#endif
        vTaskDelayUntil(&acorda, pdMS_TO_TICKS(FRAME_MS));
    }
    g->deinit();
}

static void tarefa_jogo(void *arg)
{
    (void)arg;
    ESP_ERROR_CHECK(lcd_i80_init());
    ESP_ERROR_CHECK(input_gpio_init());

    tela_de_teste();
    vTaskDelay(pdMS_TO_TICKS(2500));

    for (;;) {
        ESP_LOGI(TAG, "iniciando %s", tetris_game.name);
        roda_jogo(&tetris_game);      /* SELECT sai; aqui so reinicia */
    }
}

void app_main(void)
{
    xTaskCreate(tarefa_jogo, "jogo", 6144, NULL, 5, NULL);
}