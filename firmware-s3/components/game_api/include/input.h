/*
 * Abstracao de entrada: o jogo nunca le GPIO. O driver (GPIO direto agora,
 * MCP23017 depois) so entrega uma mascara de botoes; aqui ficam as bordas
 * (apertou/soltou) e a repeticao automatica.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BTN_UP = 0, BTN_DOWN, BTN_LEFT, BTN_RIGHT,
    BTN_A, BTN_B, BTN_START, BTN_SELECT,
    BTN_COUNT
} input_btn_t;

#define BTN_MASK(b) ((uint16_t)(1u << (b)))

typedef struct {
    uint16_t down;      /* botoes segurados agora            */
    uint16_t pressed;   /* subiu de 0 para 1 neste quadro    */
    uint16_t released;  /* desceu de 1 para 0 neste quadro   */
} input_state_t;

/* Chamar UMA vez por quadro com a mascara crua (bit = botao apertado). */
void input_state_update(input_state_t *s, uint16_t raw);

/* Repeticao automatica (esquerda/direita/baixo). Um input_repeat_t por botao. */
typedef struct { uint32_t next_ms; } input_repeat_t;

bool input_repeat(input_repeat_t *r, const input_state_t *s, input_btn_t b,
                  uint32_t now_ms, uint32_t first_ms, uint32_t every_ms);

#ifdef __cplusplus
}
#endif
