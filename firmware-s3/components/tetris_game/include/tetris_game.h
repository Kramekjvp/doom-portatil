/*
 * Tetris completo (menu, partida, pausa, fim de jogo) como um "jogo" do
 * launcher: implementa game_ops_t, desenha via gfx.h e le botoes via input.h.
 * As regras vem de tetris_core (autoria: dupla do projeto).
 */
#pragma once
#include "game.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const game_ops_t tetris_game;

#ifdef __cplusplus
}
#endif
