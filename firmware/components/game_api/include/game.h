/*
 * Contrato entre o launcher e cada jogo (Tetris, Doom, Game Boy...).
 * O launcher e dono do laco de quadros: le a entrada, monta o contexto e chama
 * tick(). O jogo desenha dentro do tick() e devolve false quando quer sair.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "input.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const input_state_t *in;  /* entrada ja atualizada neste quadro            */
    uint32_t now_ms;          /* relogio monotonico em ms                      */
    uint32_t dt_ms;           /* ms desde o tick anterior                      */
    uint32_t entropy;         /* valor aleatorio fresco (esp_random no ESP32)  */
} game_ctx_t;

typedef struct {
    const char *name;
    void (*init)(void);                  /* prepara estado e desenha a 1a tela */
    bool (*tick)(const game_ctx_t *ctx); /* false => voltar ao launcher        */
    void (*deinit)(void);
} game_ops_t;

#ifdef __cplusplus
}
#endif
