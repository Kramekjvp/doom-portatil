/*
 * Nucleo do Tetris em C puro (sem Arduino, sem tela, sem ESP-IDF).
 * Quem desenha e le botoes e o tetris_game; aqui so ficam as regras.
 *
 * Autoria: dupla do projeto (Tetris_ESP32/src/tetris.c e tetris.h).
 * Unica alteracao neste porte: tetris_rotate() ganhou chute vertical (ver
 * tetris.c), para a peca I deitada no chao conseguir girar.
 */
#ifndef TETRIS_H
#define TETRIS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define T_COLS 10
#define T_ROWS 20

typedef struct {
    uint8_t  board[T_ROWS][T_COLS];   /* 0 = vazio, senao tipo+1 (1..7) */
    uint8_t  shape[4][4];             /* peca atual */
    uint8_t  n, tipo, proxima;
    int8_t   px, py;
    uint8_t  saco[7], saco_i;         /* "saco de 7" para sortear as pecas */
    uint32_t pontos;
    uint16_t linhas;
    uint8_t  nivel;
    uint32_t acc_ms;                  /* tempo acumulado para a gravidade */
    uint32_t rng;
    bool     game_over;
} Tetris;

void    tetris_init(Tetris *t, uint32_t semente);
bool    tetris_move(Tetris *t, int dx);          /* true se moveu */
void    tetris_rotate(Tetris *t);
void    tetris_soft_drop(Tetris *t);             /* desce 1 linha (seta para baixo) */
void    tetris_hard_drop(Tetris *t);             /* derruba e trava */
void    tetris_update(Tetris *t, uint32_t dt_ms);/* gravidade */

/* Conteudo da celula (x,y) do campo, ja incluindo a peca atual: 0 = vazio, 1..7 = cor */
uint8_t tetris_cell(const Tetris *t, int x, int y);
/* Celula (x,y), 0..3, da proxima peca: 0 = vazio, 1..7 = cor */
uint8_t tetris_next_cell(const Tetris *t, int x, int y);

#ifdef __cplusplus
}
#endif
#endif
