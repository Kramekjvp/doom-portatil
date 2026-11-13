/* Interfaces internas do simulador (nada disso existe na placa). */
#pragma once
#include <stdbool.h>
#include <stdint.h>

/* --- tela simulada (sim_gfx.c): framebuffer RGB565 que implementa gfx.h --- */
typedef struct {
    unsigned long frames_presented;
    unsigned long pixels_sent;      /* pixels enviados ao "LCD" (fill_rect + blit visiveis) */
    unsigned long oob_pixels;       /* pixels desenhados fora da tela: deve ficar em 0      */
} sim_gfx_stats_t;

extern sim_gfx_stats_t sim_gfx_stats;
const uint16_t *sim_gfx_framebuffer(void);          /* gfx_width() x gfx_height(), RGB565 */
int  sim_gfx_dump_ppm(const char *path);            /* 0 = ok                             */

/* --- relogio (doom_port_pc.c) --- */
void sim_clock_use_virtual(bool on);                /* virtual = mais rapido que o tempo real, deterministico */
void sim_clock_advance(uint32_t ms);                /* so no modo virtual                  */
