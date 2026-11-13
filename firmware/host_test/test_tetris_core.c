/* Testes do nucleo (regras) do Tetris no PC. */
#include "tetris.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails;
#define CHECK(c, msg) do { if (!(c)) { printf("FALHOU: %s\n", msg); fails++; } else printf("ok: %s\n", msg); } while (0)

static int count_cells(const Tetris *t) {
    int n = 0;
    for (int y = 0; y < T_ROWS; y++) for (int x = 0; x < T_COLS; x++) n += t->board[y][x] != 0;
    return n;
}
static void forca_I_vertical_na_coluna0(Tetris *t) {
    t->tipo = 0; t->n = 4; memset(t->shape, 0, sizeof t->shape);
    for (int r = 0; r < 4; r++) t->shape[r][0] = 1;
    t->px = 0; t->py = 0;
}

int main(void) {
    /* 7-bag: cada bloco de 7 pecas e uma permutacao (limpa o campo para nunca dar game over) */
    {
        int ruins = 0;
        for (uint32_t seed = 1; seed <= 200; seed++) {
            Tetris t; tetris_init(&t, seed);
            int seq[70];
            for (int i = 0; i < 70; i++) { seq[i] = t.tipo; memset(t.board, 0, sizeof t.board); tetris_hard_drop(&t); }
            for (int b = 0; b < 10; b++) {
                int seen[7] = {0}; for (int i = 0; i < 7; i++) seen[seq[b*7+i]]++;
                for (int k = 0; k < 7; k++) if (seen[k] != 1) { ruins++; break; }
            }
        }
        CHECK(ruins == 0, "saco de 7: 2000 sacos, todos permutacoes");
    }
    /* 4 giros voltam ao formato original no meio do campo */
    {
        int ok = 1;
        for (int seed = 1; seed < 300 && ok; seed++) {
            Tetris t; tetris_init(&t, seed); t.py = 5; t.px = 3;
            uint8_t orig[4][4]; memcpy(orig, t.shape, sizeof orig); int8_t px0 = t.px;
            for (int i = 0; i < 4; i++) tetris_rotate(&t);
            if (memcmp(orig, t.shape, sizeof orig) || t.px != px0) ok = 0;
        }
        CHECK(ok, "4 giros no meio do campo voltam ao formato original");
    }
    /* 4 linhas de uma vez = 800 pontos no nivel 1 */
    {
        Tetris t; tetris_init(&t, 42);
        for (int y = 16; y < 20; y++) for (int x = 1; x < 10; x++) t.board[y][x] = 1;
        forca_I_vertical_na_coluna0(&t); tetris_hard_drop(&t);
        CHECK(t.linhas == 4 && t.pontos == 800 && count_cells(&t) == 0, "tetris (4 linhas): 800 pontos e campo vazio");
    }
    /* nivel sobe a cada 10 linhas */
    {
        Tetris t; tetris_init(&t, 5); t.linhas = 9;
        for (int x = 1; x < 10; x++) t.board[19][x] = 1;
        forca_I_vertical_na_coluna0(&t); tetris_hard_drop(&t);
        CHECK(t.nivel == 2, "nivel 2 ao chegar a 10 linhas");
    }
    /* ALTERACAO NESTE PORTE: peca I deitada no chao agora gira */
    {
        Tetris t; tetris_init(&t, 1);
        t.tipo = 0; t.n = 4; memset(t.shape, 0, sizeof t.shape);
        for (int c = 0; c < 4; c++) t.shape[1][c] = 1;
        t.px = 3; t.py = 18;
        uint8_t antes[4][4]; memcpy(antes, t.shape, sizeof antes);
        tetris_rotate(&t);
        CHECK(memcmp(antes, t.shape, sizeof antes) != 0, "peca I deitada no chao consegue girar (chute vertical)");
        int dentro = 1;
        for (int r = 0; r < 4; r++) for (int c = 0; c < 4; c++) if (t.shape[r][c] && (t.py + r < 0 || t.py + r >= T_ROWS)) dentro = 0;
        CHECK(dentro, "... e fica dentro do campo");
    }
    /* o chute vertical nao deixa girar para dentro de blocos */
    {
        Tetris t; tetris_init(&t, 1);
        t.tipo = 0; t.n = 4; memset(t.shape, 0, sizeof t.shape);
        for (int c = 0; c < 4; c++) t.shape[1][c] = 1;
        t.px = 3; t.py = 18;
        for (int x = 0; x < 10; x++) { t.board[16][x] = 1; t.board[17][x] = 1; } /* teto em cima */
        for (int x = 0; x < 3; x++) t.board[19][x] = 0;
        uint8_t antes[4][4]; memcpy(antes, t.shape, sizeof antes);
        tetris_rotate(&t);
        CHECK(memcmp(antes, t.shape, sizeof antes) == 0, "sem espaco acima: o giro continua negado");
    }
    /* Fuzz: 3000 partidas com entradas aleatorias */
    {
        int viol = 0; uint64_t jogadas = 0; srand(7);
        for (int g = 0; g < 3000 && !viol; g++) {
            Tetris t; tetris_init(&t, (uint32_t)rand() | 1);
            for (int passos = 0; !t.game_over && passos < 20000 && !viol; passos++, jogadas++) {
                switch (rand() % 8) {
                    case 0: tetris_move(&t, -1); break;
                    case 1: tetris_move(&t, +1); break;
                    case 2: tetris_rotate(&t); break;
                    case 3: tetris_soft_drop(&t); break;
                    case 4: if (rand() % 3 == 0) tetris_hard_drop(&t); break;
                    default: tetris_update(&t, (uint32_t)(rand() % 120)); break;
                }
                for (int y = 0; y < T_ROWS && !viol; y++) {
                    int full = 1;
                    for (int x = 0; x < T_COLS; x++) { if (t.board[y][x] > 7) viol = 1; if (!t.board[y][x]) full = 0; }
                    if (full) viol = 1;
                }
                if (!t.game_over) {
                    int s = 0;
                    for (int r = 0; r < t.n; r++) for (int c = 0; c < t.n; c++) if (t.shape[r][c]) {
                        int x = t.px + c, y = t.py + r; s++;
                        if (x < 0 || x >= T_COLS || y >= T_ROWS || (y >= 0 && t.board[y][x])) viol = 1;
                    }
                    if (s != 4) viol = 1;
                }
                for (int y = 0; y < T_ROWS; y++) for (int x = 0; x < T_COLS; x++) (void)tetris_cell(&t, x, y);
            }
        }
        printf("info: %llu acoes aleatorias\n", (unsigned long long)jogadas);
        CHECK(!viol, "fuzz: sem sobreposicao, sem linha cheia sobrando, sem estouro");
    }
    /* game over e gravidade */
    {
        Tetris t; tetris_init(&t, 3); int n = 0;
        while (!t.game_over && n < 1000) { tetris_hard_drop(&t); n++; }
        CHECK(t.game_over, "empilhar tudo no meio causa game over");
        Tetris g; tetris_init(&g, 11); int8_t y0 = g.py;
        tetris_update(&g, 799); int a = g.py; tetris_update(&g, 1);
        CHECK(a == y0 && g.py == y0 + 1, "gravidade nivel 1: 1 linha a cada 800 ms");
    }
    printf("\n%s (%d falhas)\n", fails ? "COM FALHAS" : "TUDO OK", fails);
    return fails != 0;
}
