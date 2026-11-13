/*
 * Tela simulada: um framebuffer RGB565 que implementa o MESMO backend de gfx.h
 * que o lcd_i80 implementa na placa (recorte, orientacao, fill_rect, blit).
 * Os jogos nao percebem a diferenca; so esta camada e a de botoes mudam.
 */
#include <stdio.h>
#include <string.h>
#include "gfx.h"
#include "sim_platform.h"

#define NATIVE_W 240
#define NATIVE_H 320

static uint16_t s_fb[320 * 320];                  /* cabe retrato e paisagem */
static int s_w = NATIVE_W, s_h = NATIVE_H;
sim_gfx_stats_t sim_gfx_stats;

int gfx_width(void)  { return s_w; }
int gfx_height(void) { return s_h; }

void gfx_set_orientation(gfx_orientation_t o)
{
    s_w = (o == GFX_LANDSCAPE) ? NATIVE_H : NATIVE_W;
    s_h = (o == GFX_LANDSCAPE) ? NATIVE_W : NATIVE_H;
    memset(s_fb, 0, sizeof s_fb);                 /* como o driver: limpa para preto */
}

void gfx_fill_rect(int x, int y, int w, int h, uint16_t color)
{
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++) {
            const int px = x + i, py = y + j;
            if (px < 0 || px >= s_w || py < 0 || py >= s_h) { sim_gfx_stats.oob_pixels++; continue; }
            s_fb[py * s_w + px] = color;
            sim_gfx_stats.pixels_sent++;
        }
}

void gfx_blit(int x, int y, int w, int h, const uint16_t *p)
{
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++) {
            const int px = x + i, py = y + j;
            if (px < 0 || px >= s_w || py < 0 || py >= s_h) { sim_gfx_stats.oob_pixels++; continue; }
            s_fb[py * s_w + px] = p[j * w + i];
            sim_gfx_stats.pixels_sent++;
        }
}

const uint16_t *sim_gfx_framebuffer(void) { return s_fb; }

int sim_gfx_dump_ppm(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    fprintf(f, "P6\n%d %d\n255\n", s_w, s_h);
    for (int i = 0; i < s_w * s_h; i++) {
        const uint16_t c = s_fb[i];
        const uint8_t rgb[3] = {
            (uint8_t)(((c >> 11) & 0x1F) * 255 / 31),
            (uint8_t)(((c >> 5) & 0x3F) * 255 / 63),
            (uint8_t)((c & 0x1F) * 255 / 31),
        };
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
    return 0;
}
