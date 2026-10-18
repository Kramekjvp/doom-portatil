/*
 * Camada grafica minima usada pelos jogos. Cores em RGB565 "nativo" (uint16_t
 * normal); quem fala com a tela cuida da ordem dos bytes.
 *
 * Backend (hardware): gfx_width/height, gfx_fill_rect, gfx_blit  -> lcd_i80
 * Portatil (so usa o backend): texto 5x7 e retangulo arredondado -> gfx_util.c
 */
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GFX_RGB565(r, g, b) \
    ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xFF) >> 3)))

/* ---- backend ---- */
int  gfx_width(void);
int  gfx_height(void);
void gfx_fill_rect(int x, int y, int w, int h, uint16_t color);
void gfx_blit(int x, int y, int w, int h, const uint16_t *pixels);  /* w*h pixels, linha a linha */

/* ---- utilitarios portateis ---- */
#define GFX_FONT_W 6   /* celula do glifo (5 de largura + 1 de espaco) */
#define GFX_FONT_H 8   /* 7 de altura + 1 de espaco                    */
#define GFX_TEXT_MAX_SCALE 4

int  gfx_text_width(const char *s, int scale);
void gfx_text(int x, int y, const char *s, int scale, uint16_t fg, uint16_t bg);
void gfx_text_centered(int cx, int y, const char *s, int scale, uint16_t fg, uint16_t bg);
/* Texto sem fundo: so os pixels acesos (varias fill_rects). Para titulos sobre desenhos. */
void gfx_text_transparent(int x, int y, const char *s, int scale, uint16_t fg);
void gfx_fill_round_rect(int x, int y, int w, int h, uint16_t color);

#ifdef __cplusplus
}
#endif
