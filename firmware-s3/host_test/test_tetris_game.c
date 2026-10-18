/*
 * Testes do fluxo e das telas do Tetris, sem hardware: o backend grafico e um
 * framebuffer em memoria. Inclui o .c para poder inspecionar/forcar o estado.
 * Gera capturas de tela na pasta shots/ (arquivos .ppm).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../components/tetris_game/tetris_game.c"
#include "fake_gfx.h"

static int fails;
#define CHECK(c, msg) do { if (!(c)) { printf("FALHOU: %s\n", msg); fails++; } else printf("ok: %s\n", msg); } while (0)

static uint32_t now_ms = 5000;
static input_state_t in;

static bool frame(uint16_t raw)
{
    input_state_update(&in, raw);
    game_ctx_t ctx = { &in, now_ms, 16, 0xA5A5A5A5u ^ (now_ms * 2654435761u) };
    bool r = tetris_game.tick(&ctx);
    now_ms += 16;
    return r;
}
static bool tap(input_btn_t b) { bool r = frame(BTN_MASK(b)); frame(0); return r; }

static uint32_t hash_fb(void)
{
    uint32_t h = 2166136261u;
    for (int y = 0; y < 320; y++) for (int x = 0; x < 240; x++) { h ^= fake_gfx_pixel(x, y); h *= 16777619u; }
    return h;
}
static void shot(const char *nome)
{
    char p[128]; snprintf(p, sizeof p, "shots/%s.ppm", nome);
    if (fake_gfx_dump_ppm(p) != 0) printf("aviso: nao salvou %s\n", p);
}

int main(void)
{
    system("mkdir -p shots");

    /* ---- menu ---- */
    tetris_game.init();
    shot("1_menu");
    uint32_t h_menu = hash_fb();
    CHECK(S.tela == TELA_MENU, "inicia no menu");
    CHECK(fake_gfx_pixel(120, 4) == C_FUNDO_MENU, "fundo do menu desenhado");

    /* ---- START inicia a partida ---- */
    tap(BTN_START);
    CHECK(S.tela == TELA_JOGO, "START no menu inicia a partida");
    CHECK(fake_gfx_pixel(4 + 7, 8 + 7) != C_FUNDO_JOGO, "campo desenhado");

    /* ---- gravidade: 800 ms ---- */
    int8_t y0 = S.jogo.py; uint32_t falta = 800 - S.jogo.acc_ms, t0 = now_ms;   /* nivel 1 = 800 ms */
    while (now_ms - t0 + 16 < falta - 20) frame(0);
    CHECK(S.jogo.py == y0, "gravidade: ainda nao desceu um pouco antes dos 800 ms");
    while (now_ms - t0 < falta + 40) frame(0);
    CHECK(S.jogo.py == y0 + 1, "gravidade: desceu 1 linha logo depois dos 800 ms");

    /* ---- movimentos ---- */
    int8_t px0 = S.jogo.px;
    tap(BTN_LEFT);
    CHECK(S.jogo.px == px0 - 1, "ESQUERDA move 1 celula");
    tap(BTN_RIGHT); tap(BTN_RIGHT);
    CHECK(S.jogo.px == px0 + 1, "DIREITA move 1 celula");
    int8_t pxh = S.jogo.px; uint32_t th = now_ms;
    while (now_ms - th < 600) frame(BTN_MASK(BTN_LEFT));
    frame(0);
    CHECK(S.jogo.px < pxh - 3, "segurar ESQUERDA repete (auto-repeat)");

    /* ---- queda (B) trava e nasce outra ---- */
    uint8_t prox = S.jogo.proxima; uint32_t cont = 0;
    for (int y = 0; y < T_ROWS; y++) for (int x = 0; x < T_COLS; x++) cont += S.jogo.board[y][x] != 0;
    tap(BTN_B);
    uint32_t cont2 = 0;
    for (int y = 0; y < T_ROWS; y++) for (int x = 0; x < T_COLS; x++) cont2 += S.jogo.board[y][x] != 0;
    CHECK(cont2 == cont + 4 && S.jogo.tipo == prox, "B derruba, trava (+4 celulas) e a proxima entra");

    /* ---- pausa congela o jogo ---- */
    tap(BTN_START);
    CHECK(S.pausado, "START pausa");
    shot("3_pausa");
    uint32_t h_pausa = hash_fb(); int8_t pyp = S.jogo.py;
    for (int i = 0; i < 200; i++) frame(0);               /* ~3 s */
    CHECK(S.jogo.py == pyp && hash_fb() == h_pausa, "pausado: nada se move e a tela nao muda");
    tap(BTN_START);
    CHECK(!S.pausado, "START de novo retoma");

    /* ---- vitrine: campo cheio pela metade + placar grande ---- */
    srand(3);
    for (int y = 11; y < T_ROWS; y++) for (int x = 0; x < T_COLS; x++) S.jogo.board[y][x] = (rand() % 5 == 0) ? 0 : (uint8_t)(1 + rand() % 7);
    S.jogo.pontos = 123456; S.jogo.linhas = 87; S.jogo.nivel = 9;
    S.jogo.py = 3; S.jogo.px = 4;
    frame(0);
    shot("2_jogo");
    CHECK(S.v_pontos == 123456 && S.v_nivel == 9, "painel atualiza pontos/nivel");

    /* ---- fim de jogo ---- */
    S.jogo.pontos = 4800;
    for (int i = 0; i < 400 && S.tela == TELA_JOGO; i++) { tap(BTN_B); }
    CHECK(S.tela == TELA_FIM && S.jogo.game_over, "campo cheio leva ao fim de jogo");
    shot("4_fim");

    /* START no fim volta ao menu, identico ao primeiro */
    tap(BTN_START);
    CHECK(S.tela == TELA_MENU && hash_fb() == h_menu, "START no fim volta ao menu (tela igual a inicial)");

    /* ---- SELECT sai (em qualquer tela) ---- */
    tap(BTN_START);
    CHECK(S.tela == TELA_JOGO, "nova partida");
    CHECK(tap(BTN_SELECT) == false, "SELECT na partida devolve false (volta ao launcher)");
    tetris_game.deinit();

    /* ---- nenhum desenho fora da tela ate aqui ---- */
    CHECK(fake_gfx_stats.oob_pixels == 0, "nenhum pixel desenhado fora da tela (fluxo guiado)");

    /* ---- fuzz de interface: 100 partidas com botoes aleatorios ---- */
    fake_gfx_reset_stats();
    srand(11);
    unsigned long frames = 0, max_calls = 0, soma_calls = 0;
    for (int g = 0; g < 100; g++) {
        tetris_game.init();
        memset(&in, 0, sizeof in);
        for (int f = 0; f < 6000; f++, frames++) {
            uint16_t raw = 0;
            if (rand() % 3 == 0) raw = (uint16_t)(1u << (rand() % (BTN_COUNT - 1)));   /* sem SELECT */
            if (rand() % 400 == 0) raw |= BTN_MASK(BTN_START);
            unsigned long antes = fake_gfx_stats.fill_calls + fake_gfx_stats.blit_calls;
            bool ok = frame(raw);
            unsigned long calls = fake_gfx_stats.fill_calls + fake_gfx_stats.blit_calls - antes;
            if (S.tela == TELA_JOGO || S.tela == TELA_FIM) { if (calls > max_calls) max_calls = calls; soma_calls += calls; }
            if (!ok) { printf("saida inesperada\n"); fails++; break; }
        }
        tetris_game.deinit();
    }
    printf("info: %lu quadros; pior quadro = %lu chamadas de desenho; media = %.2f\n", frames, max_calls, (double)soma_calls / frames);
    CHECK(fake_gfx_stats.oob_pixels == 0, "fuzz: nenhum pixel fora da tela");

    printf("\n%s (%d falhas)\n", fails ? "COM FALHAS" : "TUDO OK", fails);
    return fails != 0;
}
