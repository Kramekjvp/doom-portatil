/*
 * Launcher: o menu que escolhe qual jogo roda. E ele proprio um game_ops_t, entao o
 * laco de quadros (main.c da placa, sim_main.c no PC) trata o launcher como qualquer
 * jogo: chama tick() a cada quadro. Dentro do tick, o launcher mostra o menu e, depois
 * da escolha, repassa init/tick/deinit ao jogo escolhido.
 *
 * Nao conhece Tetris nem Doom: quem monta o firmware passa a lista (launcher_set_entries).
 * So depende de game_api, por isso roda igual na placa, no simulador e no host_test.
 *
 * Jogos "uma vez por boot" (once_per_boot): o motor nao e reentrante (o Doom). Quando um
 * deles sai, o launcher faz tick() devolver false e launcher_reboot_requested() virar true:
 * quem tem o laco reinicia o chip (esp_restart() na placa) e o menu volta limpo.
 */
#pragma once
#include <stdbool.h>
#include "game.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const game_ops_t *game;
    bool once_per_boot;       /* true: ao sair, reiniciar o chip em vez de voltar ao menu */
} launcher_entry_t;

#define LAUNCHER_MAX_ENTRIES 5   /* o que cabe na tela em retrato */

/* Antes de init(). O vetor precisa continuar valido (use static const). */
void launcher_set_entries(const launcher_entry_t *entries, int count);

extern const game_ops_t launcher_game;

/* true depois que um jogo once_per_boot terminou: o chamador deve reiniciar. */
bool launcher_reboot_requested(void);

#ifdef __cplusplus
}
#endif
