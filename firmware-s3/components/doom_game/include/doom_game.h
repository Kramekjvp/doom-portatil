/*
 * Doom (doomgeneric) como um "jogo" do launcher: implementa game_ops_t.
 * Mesmo codigo na placa e no simulador do PC (sim/). So depende de game_api
 * (gfx.h, input.h, game.h) e de doom_port.h; nunca toca GPIO nem a tela.
 *
 * Limitacao herdada do Doom: o motor NAO e reentrante. Depois de deinit()
 * (ou de sair pelo menu "Quit Game"), init() so funciona de novo apos reiniciar
 * o chip. O launcher deve tratar o Doom como "uma vez por boot" (esp_restart()).
 */
#pragma once
#include "game.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const game_ops_t doom_game;

/* Opcional, antes de init(): caminho do IWAD (-iwad). Padrao "doom1.wad".
 * Na placa o nome e virtual (o WAD vem da particao "wad"); no PC e um arquivo. */
void doom_game_set_iwad(const char *path);

#ifdef __cplusplus
}
#endif
