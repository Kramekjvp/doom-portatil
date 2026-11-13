/*
 * Tetris: telas e fluxo. Porte do main.cpp da dupla (Arduino + TFT_eSPI) para
 * as camadas do projeto:
 *   - desenho: gfx.h (fill_rect / blit) em vez de TFT_eSPI;
 *   - botoes:  input.h (bordas e repeticao ja calculadas pelo launcher);
 *   - tempo:   game_ctx_t (nada de millis()/micros()/esp_random() aqui).
 *
 * Mantido do original: layout (peca de 14 px, painel a direita), cache de
 * celulas (so redesenha o que mudou), cores, nuvens do menu, fluxo de telas.
 * Mudou: cada celula do campo e um unico blit de um tile pre-renderizado
 * (antes eram 4 fillRect), o texto usa a fonte 5x7 do game_api e SELECT sai
 * para o launcher.
 */
#include <stdio.h>
#include <string.h>
#include "gfx.h"
#include "tetris.h"
#include "tetris_game.h"

/* ---------- Layout (tela em pe, 240x320) ---------- */
#define CELL      14
#define BOARD_X   4
#define BOARD_Y   8
#define BOARD_W   (T_COLS * CELL)    /* 140 */
#define BOARD_H   (T_ROWS * CELL)    /* 280 */
#define PAINEL_X  152
#define PAINEL_W  84

#define BOX_PROX_Y    8
#define BOX_PROX_H    64
#define BOX_PONTOS_Y  80
#define BOX_LINHAS_Y  136
#define BOX_NIVEL_Y   192
#define BOX_VALOR_H   48
#define BOX_PAUSA_Y   248
#define BOX_PAUSA_H   40

/* ---------- Cores (RGB565) ---------- */
#define C_FUNDO_MENU  GFX_RGB565(186, 222, 247)
#define C_FUNDO_JOGO  GFX_RGB565(210, 210, 214)
#define C_CAMPO       GFX_RGB565(44, 44, 50)
#define C_VAZIA       GFX_RGB565(60, 60, 67)
#define C_CAIXA       GFX_RGB565(44, 44, 50)
#define C_TEXTO       GFX_RGB565(235, 235, 240)
#define C_TEXTO_LEVE  GFX_RGB565(150, 150, 160)
#define C_AMBAR       GFX_RGB565(150, 125, 30)
#define C_BRANCO      GFX_RGB565(255, 255, 255)
#define C_PRETO       GFX_RGB565(0, 0, 0)
#define C_TITULO_SOMBRA GFX_RGB565(90, 130, 170)
#define C_DICA_MENU   GFX_RGB565(60, 90, 120)

static const uint8_t RGB[8][3] = {
    {   0,   0,   0 },   /* 0 = vazio (nao usado) */
    {  40, 200, 230 },   /* I ciano    */
    { 250, 210,  40 },   /* O amarelo  */
    { 170,  70, 220 },   /* T roxo     */
    {  60, 200,  80 },   /* S verde    */
    { 230,  50,  50 },   /* Z vermelho */
    {  50, 110, 235 },   /* J azul     */
    { 250, 140,  40 },   /* L laranja  */
};

static uint16_t COR[8], COR_ESC[8], COR_CLARA[8];

/* tiles 14x14 pre-renderizados: [0] = celula vazia, [1..7] = blocos */
static uint16_t s_tile[8][CELL * CELL] __attribute__((aligned(4)));

/* ---------- Estado ---------- */
typedef enum { TELA_MENU, TELA_JOGO, TELA_FIM } tela_t;

static struct {
    tela_t         tela;
    Tetris         jogo;
    bool           pausado;
    uint8_t        mostrado[T_ROWS][T_COLS];   /* o que ja esta na tela */
    uint32_t       v_pontos, v_linhas, v_nivel;
    uint8_t        v_proxima;
    input_repeat_t rep_esq, rep_dir, rep_baixo;
} S;

/* ---------- Tiles e blocos ---------- */
static void put_rect(uint16_t *buf, int s, int x, int y, int w, int h, uint16_t c)
{
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            buf[(y + j) * s + (x + i)] = c;
}

static void render_bloco(uint16_t *buf, int s, uint8_t v)
{
    put_rect(buf, s, 0, 0, s, s, COR_ESC[v]);
    put_rect(buf, s, 2, 2, s - 4, s - 4, COR[v]);
    put_rect(buf, s, 2, 2, s / 2 - 2, 2, COR_CLARA[v]);   /* brilho no topo     */
    put_rect(buf, s, 2, 2, 2, s / 2 - 2, COR_CLARA[v]);   /* brilho na esquerda */
}

static void render_vazia(uint16_t *buf, int s)
{
    put_rect(buf, s, 0, 0, s, s, C_CAMPO);
    put_rect(buf, s, 2, 2, s - 4, s - 4, C_VAZIA);
}

static void prepara_tiles(void)
{
    for (int i = 1; i < 8; i++) {
        int r = RGB[i][0], g = RGB[i][1], b = RGB[i][2];
        COR[i]       = GFX_RGB565(r, g, b);
        COR_ESC[i]   = GFX_RGB565(r * 65 / 100, g * 65 / 100, b * 65 / 100);
        COR_CLARA[i] = GFX_RGB565(r + (255 - r) * 55 / 100,
                                  g + (255 - g) * 55 / 100,
                                  b + (255 - b) * 55 / 100);
        render_bloco(s_tile[i], CELL, (uint8_t)i);
    }
    render_vazia(s_tile[0], CELL);
}

/* ---------- Nuvem pixelada (menu) ---------- */
static int dentro_circulo(float x, float y, float cx, float cy, float r)
{
    float dx = x - cx, dy = y - cy;
    return dx * dx + dy * dy <= r * r;
}

static int nuvem_pixel(int gx, int gy, int w, int h)
{
    float x = gx + 0.5f, y = gy + 0.5f, W = (float)w, H = (float)h;
    if (gx < 0 || gy < 0 || gx >= w || gy >= h) return 0;
    if (x >= 0.20f * W && x <= 0.80f * W && y >= 0.55f * H && y <= 0.98f * H) return 1;
    if (dentro_circulo(x, y, 0.20f * W, 0.68f * H, 0.30f * H)) return 1;
    if (dentro_circulo(x, y, 0.38f * W, 0.45f * H, 0.42f * H)) return 1;
    if (dentro_circulo(x, y, 0.62f * W, 0.40f * H, 0.36f * H)) return 1;
    if (dentro_circulo(x, y, 0.80f * W, 0.68f * H, 0.30f * H)) return 1;
    return 0;
}

/* Por faixas horizontais: poucas chamadas a tela, mesmo resultado */
static void desenha_nuvem(int x0, int y0, int w, int h, int p, uint16_t cor)
{
    for (int gy = 0; gy < h; gy++) {
        int gx = 0;
        while (gx < w) {
            if (!nuvem_pixel(gx, gy, w, h)) { gx++; continue; }
            int ini = gx;
            while (gx < w && nuvem_pixel(gx, gy, w, h)) gx++;
            gfx_fill_rect(x0 + ini * p, y0 + gy * p, (gx - ini) * p, p, cor);
        }
    }
}

/* ---------- Telas ---------- */
static void desenha_menu(void)
{
    gfx_fill_rect(0, 0, 240, 320, C_FUNDO_MENU);

    /* nuvens decorativas */
    desenha_nuvem(150,  82, 14, 6, 5, C_BRANCO);
    desenha_nuvem(  6,  92, 12, 5, 5, C_BRANCO);
    desenha_nuvem(  8, 235, 18, 8, 5, C_BRANCO);
    desenha_nuvem(130, 262, 16, 7, 5, C_BRANCO);

    /* titulo com sombra */
    const int tw = gfx_text_width("TETRIS", 4);
    gfx_text_transparent(120 - tw / 2 + 3, 29 + 3, "TETRIS", 4, C_TITULO_SOMBRA);
    gfx_text_transparent(120 - tw / 2,     29,     "TETRIS", 4, C_BRANCO);

    /* nuvem do INICIAR, centralizada: grade 24x11, pixel de 8 -> 192x88 */
    const int w = 24, h = 11, p = 8;
    const int x0 = (240 - w * p) / 2, y0 = (320 - h * p) / 2;
    desenha_nuvem(x0, y0, w, h, p, C_BRANCO);
    gfx_text_transparent(120 - gfx_text_width("INICIAR", 3) / 2, y0 + 39, "INICIAR", 3, C_PRETO);

    gfx_text_centered(120, 214, "APERTE START", 2, C_DICA_MENU, C_FUNDO_MENU);
}

static void caixa(int y, int h)
{
    gfx_fill_round_rect(PAINEL_X, y, PAINEL_W, h, C_CAIXA);
}

static void desenha_tela_jogo(void)
{
    gfx_fill_rect(0, 0, 240, 320, C_FUNDO_JOGO);
    gfx_fill_round_rect(BOARD_X - 3, BOARD_Y - 3, BOARD_W + 6, BOARD_H + 6, C_CAMPO);

    caixa(BOX_PROX_Y,   BOX_PROX_H);
    caixa(BOX_PONTOS_Y, BOX_VALOR_H);
    caixa(BOX_LINHAS_Y, BOX_VALOR_H);
    caixa(BOX_NIVEL_Y,  BOX_VALOR_H);

    const int cx = PAINEL_X + PAINEL_W / 2;
    gfx_text_centered(cx, BOX_PONTOS_Y + 3, "PONTOS", 2, C_TEXTO_LEVE, C_CAIXA);
    gfx_text_centered(cx, BOX_LINHAS_Y + 3, "LINHAS", 2, C_TEXTO_LEVE, C_CAIXA);
    gfx_text_centered(cx, BOX_NIVEL_Y  + 3, "NIVEL",  2, C_TEXTO_LEVE, C_CAIXA);
}

static void invalida_campo(void) { memset(S.mostrado, 0xFF, sizeof(S.mostrado)); }

static void atualiza_campo(const Tetris *t)
{
    for (int y = 0; y < T_ROWS; y++)
        for (int x = 0; x < T_COLS; x++) {
            uint8_t v = tetris_cell(t, x, y);
            if (v == S.mostrado[y][x]) continue;
            S.mostrado[y][x] = v;
            gfx_blit(BOARD_X + x * CELL, BOARD_Y + y * CELL, CELL, CELL, s_tile[v]);
        }
}

static void desenha_proxima(const Tetris *t)
{
    caixa(BOX_PROX_Y, BOX_PROX_H);

    int minc = 4, maxc = -1, minr = 4, maxr = -1;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            if (tetris_next_cell(t, c, r)) {
                if (c < minc) minc = c;
                if (c > maxc) maxc = c;
                if (r < minr) minr = r;
                if (r > maxr) maxr = r;
            }
    enum { PS = 12 };
    uint16_t bloco[PS * PS];
    int larg = (maxc - minc + 1) * PS, alt = (maxr - minr + 1) * PS;
    int ox = PAINEL_X + (PAINEL_W - larg) / 2;
    int oy = BOX_PROX_Y + (BOX_PROX_H - alt) / 2;
    for (int r = minr; r <= maxr; r++)
        for (int c = minc; c <= maxc; c++) {
            uint8_t v = tetris_next_cell(t, c, r);
            if (!v) continue;
            render_bloco(bloco, PS, v);
            gfx_blit(ox + (c - minc) * PS, oy + (r - minr) * PS, PS, PS, bloco);
        }
}

static void escreve_valor(int y_caixa, uint32_t valor)
{
    if (valor > 999999) valor = 999999;
    char buf[8];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)valor);
    const int escala = (strlen(buf) <= 4) ? 3 : 2;
    const int h = GFX_FONT_H * escala;
    gfx_fill_rect(PAINEL_X + 4, y_caixa + 21, PAINEL_W - 8, 24, C_CAIXA);
    gfx_text_centered(PAINEL_X + PAINEL_W / 2, y_caixa + 21 + (24 - h) / 2, buf, escala, C_TEXTO, C_CAIXA);
}

static void escreve_pausa(bool pausado)
{
    const uint16_t cor = pausado ? C_AMBAR : C_CAIXA;
    const int cx = PAINEL_X + PAINEL_W / 2;
    gfx_fill_round_rect(PAINEL_X, BOX_PAUSA_Y, PAINEL_W, BOX_PAUSA_H, cor);
    gfx_text_centered(cx, BOX_PAUSA_Y + 4,  pausado ? "EM" : "START", 2, C_TEXTO, cor);
    gfx_text_centered(cx, BOX_PAUSA_Y + 20, "PAUSA", 2, C_TEXTO, cor);
}

static void faixa_mensagem(const char *l1, const char *l2)
{
    const int cx = BOARD_X + BOARD_W / 2, cy = BOARD_Y + BOARD_H / 2;
    const int w = 132, h = 72, x = cx - w / 2, y = cy - h / 2;
    gfx_fill_round_rect(x, y, w, h, C_TEXTO_LEVE);                 /* borda */
    gfx_fill_round_rect(x + 2, y + 2, w - 4, h - 4, C_CAIXA);
    gfx_text_centered(cx, y + 12, l1, 3, C_TEXTO, C_CAIXA);
    gfx_text_centered(cx, y + 44, l2, 2, C_TEXTO_LEVE, C_CAIXA);
}

static void atualiza_painel(void)
{
    if (S.jogo.proxima != S.v_proxima) { S.v_proxima = S.jogo.proxima; desenha_proxima(&S.jogo); }
    if (S.jogo.pontos  != S.v_pontos)  { S.v_pontos = S.jogo.pontos;   escreve_valor(BOX_PONTOS_Y, S.v_pontos); }
    if (S.jogo.linhas  != S.v_linhas)  { S.v_linhas = S.jogo.linhas;   escreve_valor(BOX_LINHAS_Y, S.v_linhas); }
    if (S.jogo.nivel   != S.v_nivel)   { S.v_nivel  = S.jogo.nivel;    escreve_valor(BOX_NIVEL_Y,  S.v_nivel);  }
}

/* ---------- game_ops_t ---------- */
static void inicia_partida(const game_ctx_t *ctx)
{
    tetris_init(&S.jogo, ctx->now_ms ^ ctx->entropy);
    S.pausado = false;
    memset(&S.rep_esq,   0, sizeof(S.rep_esq));
    memset(&S.rep_dir,   0, sizeof(S.rep_dir));
    memset(&S.rep_baixo, 0, sizeof(S.rep_baixo));
    desenha_tela_jogo();
    invalida_campo();
    S.v_pontos = S.v_linhas = S.v_nivel = 0xFFFFFFFFu;
    S.v_proxima = 0xFF;
    escreve_pausa(false);
    S.tela = TELA_JOGO;
}

static void tetris_game_init(void)
{
    prepara_tiles();
    memset(&S, 0, sizeof(S));
    S.tela = TELA_MENU;
    desenha_menu();
}

static void tetris_game_deinit(void) { }

static bool tetris_game_tick(const game_ctx_t *ctx)
{
    const input_state_t *in = ctx->in;

    switch (S.tela) {
    case TELA_MENU:
        if (in->pressed & BTN_MASK(BTN_SELECT)) return false;
        if (in->pressed & BTN_MASK(BTN_START))  inicia_partida(ctx);
        break;

    case TELA_JOGO:
        if (in->pressed & BTN_MASK(BTN_SELECT)) return false;

        if (in->pressed & BTN_MASK(BTN_START)) {
            S.pausado = !S.pausado;
            escreve_pausa(S.pausado);
            if (S.pausado) faixa_mensagem("PAUSA", "START");
            else           invalida_campo();           /* redesenha por cima da faixa */
        }
        if (S.pausado) break;

        if (input_repeat(&S.rep_esq,   in, BTN_LEFT,  ctx->now_ms, 170, 50)) tetris_move(&S.jogo, -1);
        if (input_repeat(&S.rep_dir,   in, BTN_RIGHT, ctx->now_ms, 170, 50)) tetris_move(&S.jogo, +1);
        if (input_repeat(&S.rep_baixo, in, BTN_DOWN,  ctx->now_ms, 100, 40)) tetris_soft_drop(&S.jogo);
        if (in->pressed & (BTN_MASK(BTN_A) | BTN_MASK(BTN_UP))) tetris_rotate(&S.jogo);
        if (in->pressed & BTN_MASK(BTN_B)) tetris_hard_drop(&S.jogo);

        tetris_update(&S.jogo, ctx->dt_ms > 100 ? 100 : ctx->dt_ms);   /* evita saltos apos travadas */
        atualiza_campo(&S.jogo);
        atualiza_painel();

        if (S.jogo.game_over) {
            char pts[16];
            uint32_t p = S.jogo.pontos > 999999 ? 999999 : S.jogo.pontos;
            snprintf(pts, sizeof(pts), "%lu PTS", (unsigned long)p);
            faixa_mensagem("FIM", pts);
            S.tela = TELA_FIM;
        }
        break;

    case TELA_FIM:
        if (in->pressed & BTN_MASK(BTN_SELECT)) return false;
        if (in->pressed & BTN_MASK(BTN_START)) { S.tela = TELA_MENU; desenha_menu(); }
        break;
    }
    return true;
}

const game_ops_t tetris_game = {
    .name   = "Tetris",
    .init   = tetris_game_init,
    .tick   = tetris_game_tick,
    .deinit = tetris_game_deinit,
};
