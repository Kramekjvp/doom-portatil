/*
 * Lista oficial de jogos do console: a UNICA fonte da ordem do menu.
 * Quem monta o firmware (main/main.c na placa, sim/sim_main.c no PC) pede a lista aqui
 * em vez de ter uma copia propria. Jogo novo = uma linha em game_registry.c.
 *
 * O Doom so aparece se o componente doom_game estiver na compilacao
 * (na placa: sem -DPORTATIL_SEM_DOOM=1; no simulador: sempre).
 */
#pragma once
#include "launcher.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Entradas na ordem do menu; *count recebe quantas sao. O vetor e estatico (vale para sempre). */
const launcher_entry_t *game_registry_entries(int *count);

/* Procura um jogo pelo nome ("tetris", "doom"; maiusculas/minusculas tanto faz). NULL se nao existir. */
const game_ops_t *game_registry_find(const char *name);

#ifdef __cplusplus
}
#endif
