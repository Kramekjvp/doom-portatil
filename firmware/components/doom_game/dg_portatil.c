/*
 * Plataforma "portatil" do doomgeneric: as funcoes DG_* que o motor exige.
 * Identico na placa e no simulador do PC - so usa gfx.h (tela) e doom_port.h (tempo).
 *
 * Quadro: o Doom entrega 320x200 com 8 bits (indices de paleta, build CMAP256).
 * Aqui cada linha vira RGB565 por uma tabela de 256 entradas e vai para a tela em
 * faixas. Por padrao as 200 linhas sao esticadas para 240 (proporcao 4:3 do VGA
 * original, onde o pixel nao era quadrado) repetindo 1 linha a cada 5; em paisagem
 * o painel de 320x240 fica totalmente preenchido.
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include "sdkconfig.h"
#else
#define CONFIG_DOOM_STRETCH_4_3 1      /* no PC: mesmos padroes do Kconfig */
#endif

#include "doomgeneric.h"
#include "i_video.h"     /* palette_changed, colors[] (build CMAP256) */
#include "gfx.h"
#include "doom_port.h"
#include "dg_portatil.h"

#ifndef CMAP256
#error "doom_game exige CMAP256 (framebuffer de 8 bits): defina -DCMAP256"
#endif

#define SRC_W  320
#define SRC_H  200
#if CONFIG_DOOM_STRETCH_4_3
#define DST_H  240
#else
#define DST_H  200
#endif
#define BAND_ROWS 20           /* 320*20*2 = 12,8 KB: casa com o buffer de DMA do lcd_i80 */

#if (DOOMGENERIC_RESX != SRC_W) || (DOOMGENERIC_RESY != SRC_H)
#error "compile o doomgeneric com -DDOOMGENERIC_RESX=320 -DDOOMGENERIC_RESY=200"
#endif

/* ---------------- fila de teclas (DG_GetKey) ---------------- */

#define QN 64
static struct { uint8_t pressed, key; } s_q[QN];
static unsigned s_qh, s_qt;           /* uma unica task: sem trava */

void dg_reset(void) { s_qh = s_qt = 0; }

void dg_key_push(int pressed, unsigned char key)
{
    const unsigned next = (s_qt + 1) % QN;
    if (next == s_qh) return;         /* cheia: descarta (nao deve ocorrer: 8 botoes) */
    s_q[s_qt].pressed = (uint8_t)(pressed != 0);
    s_q[s_qt].key = key;
    s_qt = next;
}

int DG_GetKey(int *pressed, unsigned char *key)
{
    if (s_qh == s_qt) return 0;
    *pressed = s_q[s_qh].pressed;
    *key = s_q[s_qh].key;
    s_qh = (s_qh + 1) % QN;
    return 1;
}

/* ---------------- tempo ---------------- */

void     DG_SleepMs(uint32_t ms)  { doom_port_sleep_ms(ms); }
uint32_t DG_GetTicksMs(void)      { return doom_port_millis(); }
void     DG_SetWindowTitle(const char *title) { (void)title; }

/* ---------------- video ---------------- */

static uint16_t s_lut[256];           /* indice de paleta -> RGB565 nativo */
static bool     s_lut_ok;
static uint16_t s_band[SRC_W * BAND_ROWS];

void DG_Init(void)
{
    dg_reset();
    s_lut_ok = false;
}

static void rebuild_lut(void)
{
    for (int i = 0; i < 256; i++) {
        s_lut[i] = GFX_RGB565(colors[i].r, colors[i].g, colors[i].b);
    }
    palette_changed = false;
    s_lut_ok = true;
}

void DG_DrawFrame(void)
{
    if (!s_lut_ok || palette_changed) rebuild_lut();

    const uint8_t *src = (const uint8_t *)DG_ScreenBuffer;
    const int x0 = (gfx_width()  - SRC_W) / 2;
    const int y0 = (gfx_height() - DST_H) / 2;

    int prev_sy = -1;
    const uint16_t *prev = NULL;
    for (int dy0 = 0; dy0 < DST_H; dy0 += BAND_ROWS) {
        const int n = (DST_H - dy0 < BAND_ROWS) ? (DST_H - dy0) : BAND_ROWS;
        for (int r = 0; r < n; r++) {
            const int sy = (dy0 + r) * SRC_H / DST_H;      /* DST_H == SRC_H: identidade */
            uint16_t *out = &s_band[r * SRC_W];
            if (sy == prev_sy) {
                memcpy(out, prev, SRC_W * sizeof(uint16_t));   /* linha repetida do esticamento */
            } else {
                const uint8_t *in = &src[sy * SRC_W];
                for (int x = 0; x < SRC_W; x++) out[x] = s_lut[in[x]];
            }
            prev_sy = sy;
            prev = out;
        }
        gfx_blit(x0, y0 + dy0, SRC_W, n, s_band);
    }
}
