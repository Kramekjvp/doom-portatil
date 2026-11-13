/*
 * Testes do launcher sem hardware: dois jogos falsos, backend grafico em memoria.
 * Inclui o .c para poder inspecionar o estado interno.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../components/launcher/launcher.c"
#include "fake_gfx.h"

static int fails;
#define CHECK(c, msg) do { if (!(c)) { printf("FALHOU: %s\n", msg); fails++; } else printf("ok: %s\n", msg); } while (0)

/* ---- jogos falsos ---- */
typedef struct { int init, deinit, ticks, quit_after; bool landscape; } fake_t;
static fake_t fa = { .quit_after = 3 };                       /* volta ao menu */
static fake_t fb = { .quit_after = 2, .landscape = true };    /* once_per_boot, troca orientacao (como o Doom) */

static void a_init(void)   { fa.init++; fa.ticks = 0; }
static bool a_tick(const game_ctx_t *c) { (void)c; return ++fa.ticks < fa.quit_after; }
static void a_deinit(void) { fa.deinit++; }
static void b_init(void)   { fb.init++; fb.ticks = 0; gfx_set_orientation(GFX_LANDSCAPE); }
static bool b_tick(const game_ctx_t *c) { (void)c; return ++fb.ticks < fb.quit_after; }
static void b_deinit(void) { fb.deinit++; gfx_set_orientation(GFX_PORTRAIT); }

static const game_ops_t jogoA = { "JogoA", a_init, a_tick, a_deinit };
static const game_ops_t jogoB = { "JogoB", b_init, b_tick, b_deinit };
static const launcher_entry_t ENTRADAS[] = { { &jogoA, false }, { &jogoB, true } };

static uint32_t now_ms = 1000;
static input_state_t in;
static bool frame(uint16_t raw)
{
    input_state_update(&in, raw);
    game_ctx_t ctx = { &in, now_ms, 16, 0x1234u };
    bool r = launcher_game.tick(&ctx);
    now_ms += 16;
    return r;
}
static bool tap(input_btn_t b) { bool r = frame(BTN_MASK(b)); frame(0); return r; }

/* a linha i esta destacada? (pixel no meio-topo da linha, longe dos cantos arredondados e do texto) */
static bool linha_destacada(int i) { return fake_gfx_pixel(120, LIST_Y + i * ROW_H + 1) == C_SEL_BG; }

int main(void)
{
    /* ---- menu ---- */
    launcher_set_entries(ENTRADAS, 2);
    launcher_game.init();
    CHECK(s_state == ST_MENU && s_sel == 0, "inicia no menu, jogo 0 escolhido");
    CHECK(fake_gfx_pixel(5, 5) == C_BG, "fundo do menu desenhado");
    CHECK(linha_destacada(0) && !linha_destacada(1), "so a linha 0 destacada");

    tap(BTN_DOWN);
    CHECK(s_sel == 1 && linha_destacada(1) && !linha_destacada(0), "BAIXO escolhe o jogo 1 e move o destaque");
    tap(BTN_DOWN);
    CHECK(s_sel == 0, "BAIXO no fim volta ao primeiro");
    tap(BTN_UP);
    CHECK(s_sel == 1, "CIMA no inicio vai ao ultimo");
    tap(BTN_UP);
    CHECK(s_sel == 0 && fa.init == 0 && fb.init == 0, "navegar nao inicia jogo nenhum");

    /* ---- jogo que volta ao menu ---- */
    tap(BTN_A);
    CHECK(fa.init == 1 && s_state == ST_PLAYING, "A inicia o jogo A (init 1 vez)");
    int t0 = fa.ticks;                     /* o frame(0) do tap() ja foi um tick do jogo */
    CHECK(frame(0) && fa.ticks == t0 + 1, "jogo A recebe tick a cada quadro");
    bool vivo = true;
    for (int i = 0; i < 6 && s_state == ST_PLAYING; i++) vivo = frame(0);
    CHECK(fa.deinit == 1 && s_state == ST_MENU, "jogo A saiu: deinit 1 vez e volta ao menu");
    CHECK(vivo && !launcher_reboot_requested(), "jogo normal NAO pede reinicio");
    CHECK(fake_gfx_pixel(5, 5) == C_BG && linha_destacada(0), "menu redesenhado ao voltar");

    /* A segurado o tempo todo (jogo + volta ao menu) nao relanca; soltar e apertar relanca */
    int antes = fa.init;
    frame(0);
    frame(BTN_MASK(BTN_A));                                   /* borda: inicia */
    CHECK(fa.init == antes + 1 && s_state == ST_PLAYING, "A (borda) inicia o jogo");
    for (int i = 0; i < 12; i++) frame(BTN_MASK(BTN_A));      /* segura ate o jogo sair e depois no menu */
    CHECK(s_state == ST_MENU && fa.init == antes + 1, "A segurado ao voltar ao menu nao relanca o jogo");
    frame(0);
    tap(BTN_A);
    CHECK(fa.init == antes + 2 && s_state == ST_PLAYING, "soltar e apertar de novo relanca");
    while (s_state == ST_PLAYING) frame(0);

    /* START tambem inicia */
    antes = fa.init;
    tap(BTN_START);
    CHECK(fa.init == antes + 1 && s_state == ST_PLAYING, "START tambem inicia o jogo");
    while (s_state == ST_PLAYING) frame(0);

    /* ---- jogo once_per_boot (como o Doom) ---- */
    tap(BTN_DOWN);
    tap(BTN_A);
    CHECK(fb.init == 1 && s_state == ST_PLAYING, "A inicia o jogo B");
    CHECK(gfx_width() == 320 && gfx_height() == 240, "jogo B trocou a tela para paisagem");
    bool r = true;
    for (int i = 0; i < 6 && r; i++) r = frame(0);
    CHECK(!r, "jogo once_per_boot: tick devolve false ao sair (o laco encerra)");
    CHECK(fb.deinit == 1, "deinit do jogo B chamado");
    CHECK(launcher_reboot_requested(), "pede reinicio do chip");
    CHECK(gfx_width() == 240 && gfx_height() == 320, "deinit devolveu a tela para retrato");

    /* depois do reboot: init zera o pedido */
    launcher_game.init();
    CHECK(!launcher_reboot_requested() && s_state == ST_MENU, "novo init (apos reiniciar): menu limpo, sem pedido");

    /* ---- casos de borda ---- */
    launcher_set_entries(NULL, 0);
    launcher_game.init();
    CHECK(frame(BTN_MASK(BTN_A)) && s_state == ST_MENU, "sem jogos: A nao faz nada, nao quebra");
    launcher_set_entries(ENTRADAS, 2);
    s_sel = 1; launcher_set_entries(ENTRADAS, 1);
    CHECK(s_sel == 0, "lista menor que a escolha antiga: escolha volta a 0");
    static launcher_entry_t muitas[8];
    for (int i = 0; i < 8; i++) muitas[i].game = &jogoA;
    launcher_set_entries(muitas, 8);
    CHECK(s_count == LAUNCHER_MAX_ENTRIES, "mais entradas que o limite: so as que cabem na tela");

    /* ---- fuzz: botoes aleatorios, nada fora da tela, estado coerente ---- */
    launcher_set_entries(ENTRADAS, 2);
    fb.quit_after = 1000000; fa.quit_after = 20;       /* B nao sai sozinho: nao vale reiniciar no fuzz */
    launcher_game.init();
    fake_gfx_reset_stats();
    srand(7);
    for (int i = 0; i < 200000; i++) {
        uint16_t raw = (uint16_t)(rand() & 0xFF) & (uint16_t)(rand() & 0xFF);   /* poucos botoes por vez */
        if (s_state == ST_PLAYING && s_cur == &ENTRADAS[1]) raw &= (uint16_t)~BTN_MASK(BTN_A);
        frame(raw);
        if (s_state == ST_PLAYING && s_cur == &ENTRADAS[1] && fb.ticks > 50) { launcher_game.deinit(); launcher_game.init(); }
    }
    CHECK(fake_gfx_stats.oob_pixels == 0, "fuzz: nenhum pixel fora da tela");
    CHECK(fa.init >= fa.deinit && fa.init - fa.deinit <= 1, "fuzz: init/deinit do jogo A balanceados");

    if (fails) { printf("\n%d FALHA(S)\n", fails); return 1; }
    printf("\nTUDO OK (0 falhas)\n");
    return 0;
}
