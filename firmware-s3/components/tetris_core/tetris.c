#include "tetris.h"
#include <string.h>

/* ordem: I, O, T, S, Z, J, L */
static const uint8_t TAM[7] = { 4, 2, 3, 3, 3, 3, 3 };

static const uint8_t FORMAS[7][4][4] = {
    { {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} },  /* I */
    { {1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} },  /* O */
    { {0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },  /* T */
    { {0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} },  /* S */
    { {1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },  /* Z */
    { {1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },  /* J */
    { {0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },  /* L */
};

static uint32_t aleatorio(Tetris *t)      /* xorshift32: bem leve */
{
    uint32_t x = t->rng;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    t->rng = x;
    return x;
}

static bool colide(const Tetris *t, uint8_t (*s)[4], int n, int px, int py)
{
    for (int r = 0; r < n; r++)
        for (int c = 0; c < n; c++) {
            if (!s[r][c]) continue;
            int x = px + c, y = py + r;
            if (x < 0 || x >= T_COLS || y >= T_ROWS) return true;
            if (y >= 0 && t->board[y][x]) return true;
        }
    return false;
}

static uint8_t sorteia(Tetris *t)
{
    if (t->saco_i >= 7) {
        for (uint8_t i = 0; i < 7; i++) t->saco[i] = i;
        for (int i = 6; i > 0; i--) {
            int k = (int)(aleatorio(t) % (uint32_t)(i + 1));
            uint8_t tmp = t->saco[i]; t->saco[i] = t->saco[k]; t->saco[k] = tmp;
        }
        t->saco_i = 0;
    }
    return t->saco[t->saco_i++];
}

static void nascePeca(Tetris *t)
{
    t->tipo = t->proxima;
    t->proxima = sorteia(t);
    t->n = TAM[t->tipo];
    memcpy(t->shape, FORMAS[t->tipo], sizeof(t->shape));
    t->px = (int8_t)((T_COLS - t->n) / 2);
    t->py = (t->tipo == 0) ? -1 : 0;
    t->acc_ms = 0;
    if (colide(t, t->shape, t->n, t->px, t->py)) t->game_over = true;
}

void tetris_init(Tetris *t, uint32_t semente)
{
    memset(t, 0, sizeof(*t));
    t->rng = semente ? semente : 0x2545F491u;
    t->saco_i = 7;
    t->nivel = 1;
    t->proxima = sorteia(t);
    nascePeca(t);
}

bool tetris_move(Tetris *t, int dx)
{
    if (t->game_over) return false;
    if (colide(t, t->shape, t->n, t->px + dx, t->py)) return false;
    t->px = (int8_t)(t->px + dx);
    return true;
}

void tetris_rotate(Tetris *t)
{
    if (t->game_over || t->tipo == 1) return;      /* O nao gira */
    uint8_t tmp[4][4] = {{0}};
    int n = t->n;
    for (int r = 0; r < n; r++)
        for (int c = 0; c < n; c++)
            tmp[r][c] = t->shape[n - 1 - c][r];

    /* Tentativas {dx, dy}: primeiro so empurra de lado (original da dupla),
     * depois tambem sobe 1 ou 2 linhas. A peca I deitada no chao precisa
     * subir 2 (ela fica vertical, com 4 de altura); as demais, 1. */
    static const int8_t chutes[11][2] = {
        { 0, 0 }, { -1, 0 }, { 1, 0 }, { -2, 0 }, { 2, 0 },
        { 0, -1 }, { -1, -1 }, { 1, -1 },
        { 0, -2 }, { -1, -2 }, { 1, -2 },
    };
    for (int i = 0; i < 11; i++) {
        if (!colide(t, tmp, n, t->px + chutes[i][0], t->py + chutes[i][1])) {
            memcpy(t->shape, tmp, sizeof(tmp));
            t->px = (int8_t)(t->px + chutes[i][0]);
            t->py = (int8_t)(t->py + chutes[i][1]);
            return;
        }
    }
}

static void limpaLinhas(Tetris *t)
{
    static const uint16_t tabela[5] = { 0, 100, 300, 500, 800 };
    int feitas = 0;

    for (int y = T_ROWS - 1; y >= 0; y--) {
        bool cheia = true;
        for (int x = 0; x < T_COLS; x++)
            if (!t->board[y][x]) { cheia = false; break; }
        if (!cheia) continue;

        for (int yy = y; yy > 0; yy--)
            memcpy(t->board[yy], t->board[yy - 1], T_COLS);
        memset(t->board[0], 0, T_COLS);
        feitas++;
        y++;
    }
    if (feitas) {
        t->pontos += (uint32_t)tabela[feitas] * t->nivel;
        t->linhas = (uint16_t)(t->linhas + feitas);
        t->nivel  = (uint8_t)(1 + t->linhas / 10);
    }
}

static void travaPeca(Tetris *t)
{
    for (int r = 0; r < t->n; r++)
        for (int c = 0; c < t->n; c++)
            if (t->shape[r][c]) {
                int y = t->py + r;
                if (y >= 0) t->board[y][t->px + c] = (uint8_t)(t->tipo + 1);
            }
    limpaLinhas(t);
    nascePeca(t);
}

void tetris_soft_drop(Tetris *t)
{
    if (t->game_over) return;
    if (!colide(t, t->shape, t->n, t->px, t->py + 1)) {
        t->py++;
        t->acc_ms = 0;
    }
}

void tetris_hard_drop(Tetris *t)
{
    if (t->game_over) return;
    while (!colide(t, t->shape, t->n, t->px, t->py + 1)) t->py++;
    travaPeca(t);
}

void tetris_update(Tetris *t, uint32_t dt_ms)
{
    if (t->game_over) return;

    int32_t intervalo = 800 - ((int32_t)t->nivel - 1) * 70;
    if (intervalo < 80) intervalo = 80;

    t->acc_ms += dt_ms;
    while (t->acc_ms >= (uint32_t)intervalo && !t->game_over) {
        t->acc_ms -= (uint32_t)intervalo;
        if (!colide(t, t->shape, t->n, t->px, t->py + 1)) t->py++;
        else travaPeca(t);
    }
}

uint8_t tetris_cell(const Tetris *t, int x, int y)
{
    int dx = x - t->px, dy = y - t->py;
    if (!t->game_over && dx >= 0 && dx < t->n && dy >= 0 && dy < t->n && t->shape[dy][dx])
        return (uint8_t)(t->tipo + 1);
    return t->board[y][x];
}

uint8_t tetris_next_cell(const Tetris *t, int x, int y)
{
    return FORMAS[t->proxima][y][x] ? (uint8_t)(t->proxima + 1) : 0;
}
