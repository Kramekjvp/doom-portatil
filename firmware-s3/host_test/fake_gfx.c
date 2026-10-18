#include <stdio.h>
#include <string.h>
#include "gfx.h"
#include "fake_gfx.h"

#define W 240
#define H 320

static uint16_t fb[H][W];
fake_gfx_stats_t fake_gfx_stats;

void fake_gfx_reset_stats(void) { memset(&fake_gfx_stats, 0, sizeof fake_gfx_stats); }
uint16_t fake_gfx_pixel(int x, int y) { return fb[y][x]; }

int gfx_width(void)  { return W; }
int gfx_height(void) { return H; }

void gfx_fill_rect(int x, int y, int w, int h, uint16_t c)
{
    fake_gfx_stats.fill_calls++;
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++) {
            int px = x + i, py = y + j;
            if (px < 0 || px >= W || py < 0 || py >= H) { fake_gfx_stats.oob_pixels++; continue; }
            fb[py][px] = c;
        }
}

void gfx_blit(int x, int y, int w, int h, const uint16_t *p)
{
    fake_gfx_stats.blit_calls++;
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++) {
            int px = x + i, py = y + j;
            if (px < 0 || px >= W || py < 0 || py >= H) { fake_gfx_stats.oob_pixels++; continue; }
            fb[py][px] = p[j * w + i];
        }
}

int fake_gfx_dump_ppm(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            uint16_t c = fb[y][x];
            uint8_t rgb[3] = {
                (uint8_t)(((c >> 11) & 0x1F) * 255 / 31),
                (uint8_t)(((c >> 5) & 0x3F) * 255 / 63),
                (uint8_t)((c & 0x1F) * 255 / 31),
            };
            fwrite(rgb, 1, 3, f);
        }
    fclose(f);
    return 0;
}
