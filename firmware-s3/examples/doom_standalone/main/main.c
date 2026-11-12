/*
 * Doom standalone no ESP32-S3: tela ILI9341 paralela (i80) + botoes em GPIO.
 * Este main faz o papel do "launcher" (laco de quadros), igual ao tetris_standalone.
 * O Doom e "uma vez por boot" (o motor nao e reentrante): ao sair, reinicia o chip.
 */
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "game.h"
#include "gfx.h"
#include "input.h"
#include "input_gpio.h"
#include "lcd_i80.h"
#include "doom_game.h"

static const char *TAG = "main";

#define FRAME_MS 16

/* Barras R/G/B + quadrado branco no canto sup. esquerdo (confere fiacao, cor e orientacao) */
static void tela_de_teste(void)
{
    const int w = gfx_width(), h = gfx_height();
    gfx_fill_rect(0,         0, w, h / 3,             GFX_RGB565(255, 0, 0));
    gfx_fill_rect(0,     h / 3, w, h / 3,             GFX_RGB565(0, 255, 0));
    gfx_fill_rect(0, 2 * h / 3, w, h - 2 * (h / 3),   GFX_RGB565(0, 0, 255));
    gfx_fill_rect(0, 0, 24, 24, GFX_RGB565(255, 255, 255));
}

static void roda_jogo(const game_ops_t *g)
{
    input_state_t in = { 0 };
    g->init();

    TickType_t acorda = xTaskGetTickCount();
    uint32_t anterior = (uint32_t)(esp_timer_get_time() / 1000);
    for (bool rodando = true; rodando; ) {
        const uint32_t agora = (uint32_t)(esp_timer_get_time() / 1000);
        input_state_update(&in, input_gpio_read());
        const game_ctx_t ctx = {
            .in = &in, .now_ms = agora, .dt_ms = agora - anterior, .entropy = esp_random(),
        };
        anterior = agora;
        rodando = g->tick(&ctx);
        vTaskDelayUntil(&acorda, pdMS_TO_TICKS(FRAME_MS));
    }
    g->deinit();
}

static void tarefa_jogo(void *arg)
{
    (void)arg;
    ESP_ERROR_CHECK(lcd_i80_init());
    ESP_ERROR_CHECK(input_gpio_init());

#if CONFIG_DOOM_STANDALONE_TELA_DE_TESTE
    tela_de_teste();
    vTaskDelay(pdMS_TO_TICKS(2500));
#endif

    ESP_LOGI(TAG, "iniciando %s (heap livre: %u bytes)", doom_game.name, (unsigned)esp_get_free_heap_size());
    roda_jogo(&doom_game);

    ESP_LOGW(TAG, "Doom terminou; reiniciando o chip");
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_restart();
}

void app_main(void)
{
    /* O Doom usa muita pilha (D_DoomMain, renderizador): 32 KB, na RAM interna. */
    xTaskCreate(tarefa_jogo, "doom", 32 * 1024, NULL, 5, NULL);
}
