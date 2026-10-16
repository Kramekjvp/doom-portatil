/* Backend grafico falso para testes no PC: desenha num framebuffer 240x320. */
#pragma once
#include <stdint.h>

typedef struct {
    unsigned long fill_calls, blit_calls;
    unsigned long oob_pixels;       /* pixels desenhados fora da tela (deve ficar 0) */
} fake_gfx_stats_t;

extern fake_gfx_stats_t fake_gfx_stats;

void     fake_gfx_reset_stats(void);
uint16_t fake_gfx_pixel(int x, int y);
int      fake_gfx_dump_ppm(const char *path);   /* 0 = ok */
