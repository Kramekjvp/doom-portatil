/*
 * Firmware unico do console: lcd_i80 + input_gpio + launcher + jogos.
 * Faz o papel de "dono do laco de quadros" (no PC, quem faz isso e o sim/sim_main.c).
 * A lista de jogos vem do componente game_registry: a MESMA na placa e no simulador.
 *
 * O Doom e "uma vez por boot" (motor nao reentrante): ao sair, o launcher pede
 * reinicio e este laco chama esp_restart(); o menu volta limpo.
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
#include "launcher.h"
#include "game_registry.h"

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

#if CONFIG_PORTATIL_TELA_DE_TESTE
    tela_de_teste();
    vTaskDelay(pdMS_TO_TICKS(2500));
#endif

    int n_jogos = 0;
    const launcher_entry_t *jogos = game_registry_entries(&n_jogos);
    launcher_set_entries(jogos, n_jogos);
    ESP_LOGI(TAG, "launcher iniciado (heap livre: %u bytes)", (unsigned)esp_get_free_heap_size());
    roda_jogo(&launcher_game);

    ESP_LOGW(TAG, "%s; reiniciando o chip", launcher_reboot_requested() ? "jogo nao reentrante terminou" : "launcher encerrou");
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_restart();
}

void app_main(void)
{
    /* O Doom usa muita pilha (D_DoomMain, renderizador): 32 KB, na RAM interna. */
    xTaskCreate(tarefa_jogo, "portatil", 32 * 1024, NULL, 5, NULL);
}
