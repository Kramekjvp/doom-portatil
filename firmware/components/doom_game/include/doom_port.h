/*
 * Fronteira de plataforma do Doom: o que o doomgeneric precisa e muda entre a
 * placa (ESP-IDF) e o simulador no PC. Duas implementacoes:
 *   - components/doom_game/doom_port_esp.c   (esp_timer + vTaskDelay + particao do WAD)
 *   - sim/doom_port_pc.c                      (relogio real ou virtual)
 * O resto do port (dg_portatil.c, doom_game.c) e o MESMO nos dois alvos.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t doom_port_millis(void);            /* relogio monotonico, ms                       */
void     doom_port_sleep_ms(uint32_t ms);   /* o Doom espera assim por um novo "tic" (1/35 s) */

/* true se `name` e um WAD que o alvo sabe abrir sem sistema de arquivos (placa: o nome
 * virtual "doom1.wad" -> particao "wad"). No PC devolve false (usa o arquivo de verdade). */
bool     doom_port_wad_is_virtual(const char *name);

#ifdef __cplusplus
}
#endif
