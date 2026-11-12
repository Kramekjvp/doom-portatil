/*
 * Simulador do console no PC. Faz o papel do main.c da placa (o "launcher"):
 * dono do laco de quadros, le os botoes, monta o game_ctx_t e chama game_ops_t.
 * Roda os MESMOS jogos (Doom, Tetris) sobre game_api; so a tela (sim_gfx.c) e os
 * botoes (teclado ou roteiro) sao trocados.
 *
 * Dois modos:
 *   interativo (SDL2): janela + teclado, tempo real
 *   --headless       : sem janela, tempo VIRTUAL (mais rapido que o real e
 *                      deterministico), botoes por roteiro, capturas .ppm
 *
 * Teclado:  setas = direcional   Z = A (atirar)   X = B (usar)
 *           Enter = START        Shift direito ou Backspace = SELECT   Esc = sair
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir((p), 0755)
#endif

#include "game.h"
#include "gfx.h"
#include "input.h"
#include "doom_game.h"
#include "doom_port.h"
#include "tetris_game.h"
#include "launcher.h"
#include "sim_platform.h"

#ifdef SIM_SDL
#include <SDL.h>
#endif

#define FRAME_MS   16        /* igual ao main.c da placa (~60 quadros/s de teto) */
#define TAP_FRAMES 8         /* duracao de um toque "*" do roteiro */
#define MAX_EVENTS 1024
#define MAX_SHOTS  128

/* ---------------- roteiro de botoes (modo --headless) ---------------- */

typedef struct { unsigned frame; uint16_t mask; bool on; } script_ev_t;

static script_ev_t s_ev[MAX_EVENTS];
static int s_nev;

static const char *BTN_NAMES[BTN_COUNT] = { "UP", "DOWN", "LEFT", "RIGHT", "A", "B", "START", "SELECT" };

static int btn_from_name(const char *s)
{
    for (int b = 0; b < BTN_COUNT; b++) {
        const char *n = BTN_NAMES[b];
        size_t i = 0;
        for (; n[i] && s[i] && toupper((unsigned char)s[i]) == n[i]; i++) {}
        if (!n[i] && !s[i]) return b;
    }
    return -1;
}

static void add_ev(unsigned frame, int btn, bool on)
{
    if (s_nev >= MAX_EVENTS) { fprintf(stderr, "roteiro longo demais (max %d eventos)\n", MAX_EVENTS); exit(2); }
    s_ev[s_nev].frame = frame;
    s_ev[s_nev].mask = BTN_MASK(btn);
    s_ev[s_nev].on = on;
    s_nev++;
}

/* "30:+A 38:-A 60:*START 90:+SELECT 140:-SELECT"   (+ aperta, - solta, * toque) */
static void parse_item(char *tok)
{
    char *colon = strchr(tok, ':');
    if (!colon || !colon[1]) { fprintf(stderr, "roteiro: item invalido \"%s\" (use quadro:+BOTAO)\n", tok); exit(2); }
    *colon = '\0';
    const unsigned frame = (unsigned)strtoul(tok, NULL, 10);
    const char op = colon[1];
    const int b = btn_from_name(colon + 2);
    if ((op != '+' && op != '-' && op != '*') || b < 0) {
        fprintf(stderr, "roteiro: item invalido \"%s:%s\" (botoes: UP DOWN LEFT RIGHT A B START SELECT)\n", tok, colon + 1);
        exit(2);
    }
    if (op == '+') add_ev(frame, b, true);
    else if (op == '-') add_ev(frame, b, false);
    else { add_ev(frame, b, true); add_ev(frame + TAP_FRAMES, b, false); }
}

static void parse_script(const char *text)
{
    char *copy = strdup(text);
    char *p = copy;
    while (*p) {
        while (*p == ' ' || *p == ',' || *p == ';') p++;
        if (!*p) break;
        char *tok = p;
        while (*p && *p != ' ' && *p != ',' && *p != ';') p++;
        if (*p) *p++ = '\0';
        parse_item(tok);
    }
    free(copy);
}

/* mascara de botoes no quadro `frame`: aplica, em ordem, os eventos ja vencidos */
static uint16_t script_mask(unsigned frame)
{
    static uint16_t mask;
    for (;;) {
        int best = -1;
        for (int i = 0; i < s_nev; i++) {
            if (s_ev[i].frame == (unsigned)-1) continue;          /* ja aplicado */
            if (s_ev[i].frame <= frame && (best < 0 || s_ev[i].frame < s_ev[best].frame)) best = i;
        }
        if (best < 0) break;
        if (s_ev[best].on) mask |= s_ev[best].mask; else mask &= (uint16_t)~s_ev[best].mask;
        s_ev[best].frame = (unsigned)-1;
    }
    return mask;
}

/* ---------------- janela e teclado (SDL2) ---------------- */

#ifdef SIM_SDL
static SDL_Window *s_win;
static SDL_Renderer *s_ren;
static SDL_Texture *s_tex;
static int s_tw, s_th, s_scale = 3;
static unsigned s_fps_frames, s_fps_t0;

static void sdl_start(const char *title)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { fprintf(stderr, "SDL_Init: %s\n", SDL_GetError()); exit(1); }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");      /* pixels nitidos, como no painel */
    s_win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             gfx_width() * s_scale, gfx_height() * s_scale, SDL_WINDOW_SHOWN);
    s_ren = s_win ? SDL_CreateRenderer(s_win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) : NULL;
    if (s_win && !s_ren) s_ren = SDL_CreateRenderer(s_win, -1, SDL_RENDERER_SOFTWARE);
    if (!s_ren) { fprintf(stderr, "SDL: %s\n", SDL_GetError()); exit(1); }
}

static void sdl_present(void)
{
    const int w = gfx_width(), h = gfx_height();
    if (!s_tex || w != s_tw || h != s_th) {               /* o jogo trocou a orientacao */
        if (s_tex) SDL_DestroyTexture(s_tex);
        s_tex = SDL_CreateTexture(s_ren, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, w, h);
        s_tw = w; s_th = h;
        SDL_SetWindowSize(s_win, w * s_scale, h * s_scale);
    }
    SDL_UpdateTexture(s_tex, NULL, sim_gfx_framebuffer(), w * (int)sizeof(uint16_t));
    SDL_RenderClear(s_ren);
    SDL_RenderCopy(s_ren, s_tex, NULL, NULL);
    SDL_RenderPresent(s_ren);

    s_fps_frames++;
    const unsigned now = SDL_GetTicks();
    if (now - s_fps_t0 >= 1000) {
        char t[96];
        snprintf(t, sizeof t, "DOOM_PORTATIL sim - %u quadros/s", s_fps_frames * 1000 / (now - s_fps_t0));
        SDL_SetWindowTitle(s_win, t);
        s_fps_frames = 0; s_fps_t0 = now;
    }
}

static uint16_t sdl_buttons(bool *quit)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) *quit = true;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) *quit = true;
    }
    const Uint8 *k = SDL_GetKeyboardState(NULL);
    uint16_t m = 0;
    if (k[SDL_SCANCODE_UP])    m |= BTN_MASK(BTN_UP);
    if (k[SDL_SCANCODE_DOWN])  m |= BTN_MASK(BTN_DOWN);
    if (k[SDL_SCANCODE_LEFT])  m |= BTN_MASK(BTN_LEFT);
    if (k[SDL_SCANCODE_RIGHT]) m |= BTN_MASK(BTN_RIGHT);
    if (k[SDL_SCANCODE_Z])     m |= BTN_MASK(BTN_A);
    if (k[SDL_SCANCODE_X])     m |= BTN_MASK(BTN_B);
    if (k[SDL_SCANCODE_RETURN]) m |= BTN_MASK(BTN_START);
    if (k[SDL_SCANCODE_RSHIFT] || k[SDL_SCANCODE_BACKSPACE]) m |= BTN_MASK(BTN_SELECT);
    return m;
}
#endif

/* ---------------- principal ---------------- */

static void usage(const char *p)
{
    fprintf(stderr,
        "uso: %s [--game launcher|doom|tetris] [--wad ARQUIVO.wad] [opcoes]\n"
        "  (padrao: launcher = o menu da placa, com Tetris e Doom)\n"
        "  --headless            sem janela; tempo virtual; botoes por --script\n"
        "  --frames N            para apos N quadros (padrao: 300 no headless, infinito na janela)\n"
        "  --script \"Q:+A Q:-A Q:*START ...\"  botoes por quadro: + aperta, - solta, * toque (%d quadros)\n"
        "  --shot Q              salva shots/<jogo>_<Q>.ppm no quadro Q (repetivel)\n"
        "  --shot-every N        salva uma captura a cada N quadros\n"
        "  --shot-dir DIR        pasta das capturas (padrao: shots)\n"
        "  --scale N             tamanho da janela (padrao 3)\n"
        "botoes: UP DOWN LEFT RIGHT A B START SELECT\n", p, TAP_FRAMES);
}

int main(int argc, char **argv)
{
    const char *game_name = "launcher", *wad = NULL, *shot_dir = "shots";
    bool headless = false;
    unsigned frames = 0, shot_every = 0, shots[MAX_SHOTS];
    int nshots = 0;

    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        const bool has = i + 1 < argc;
        if (!strcmp(a, "--game") && has) game_name = argv[++i];
        else if (!strcmp(a, "--wad") && has) wad = argv[++i];
        else if (!strcmp(a, "--headless")) headless = true;
        else if (!strcmp(a, "--frames") && has) frames = (unsigned)strtoul(argv[++i], NULL, 10);
        else if (!strcmp(a, "--script") && has) parse_script(argv[++i]);
        else if (!strcmp(a, "--shot") && has) {
            if (nshots < MAX_SHOTS) shots[nshots++] = (unsigned)strtoul(argv[++i], NULL, 10);
        }
        else if (!strcmp(a, "--shot-every") && has) shot_every = (unsigned)strtoul(argv[++i], NULL, 10);
        else if (!strcmp(a, "--shot-dir") && has) shot_dir = argv[++i];
#ifdef SIM_SDL
        else if (!strcmp(a, "--scale") && has) s_scale = atoi(argv[++i]);
#endif
        else { usage(argv[0]); return 2; }
    }

    /* mesma lista do app/main/main.c da placa */
    static const launcher_entry_t JOGOS[] = {
        { &tetris_game, false },
        { &doom_game,   true  },     /* motor nao reentrante: ao sair, "reinicia o chip" */
    };
    doom_game_set_iwad(wad ? wad : "doom1.wad");

    const game_ops_t *g;
    if (!strcmp(game_name, "launcher")) {
        launcher_set_entries(JOGOS, 2);
        g = &launcher_game;
    } else if (!strcmp(game_name, "doom")) {
        g = &doom_game;
    } else if (!strcmp(game_name, "tetris")) {
        g = &tetris_game;
    } else {
        usage(argv[0]);
        return 2;
    }

#ifndef SIM_SDL
    if (!headless) {
        fprintf(stderr, "Este binario foi compilado sem SDL2: so o modo --headless esta disponivel.\n"
                        "No MSYS2 (UCRT64): pacman -S mingw-w64-ucrt-x86_64-SDL2 e rode \"make\" de novo.\n");
        return 2;
    }
#endif
    if (headless && frames == 0) frames = 300;
    if (nshots || shot_every) MKDIR(shot_dir);

    sim_clock_use_virtual(headless);
    srand(1);                                   /* entropia reproduzivel no headless */

    input_state_t in = { 0 };
    g->init();
#ifdef SIM_SDL
    if (!headless) sdl_start("DOOM_PORTATIL sim");
#endif

    uint32_t prev = doom_port_millis(), next = prev;
    const uint32_t t_start = prev;
    unsigned f = 0;
    bool running = true, quit = false;
    while (running && !quit) {
        uint16_t raw;
#ifdef SIM_SDL
        raw = headless ? script_mask(f) : sdl_buttons(&quit);
#else
        raw = script_mask(f);
#endif
        if (quit) break;

        const uint32_t now = doom_port_millis();
        input_state_update(&in, raw);
        const game_ctx_t ctx = { .in = &in, .now_ms = now, .dt_ms = now - prev, .entropy = (uint32_t)rand() };
        prev = now;
        running = g->tick(&ctx);
        sim_gfx_stats.frames_presented++;

        bool shoot = shot_every && f % shot_every == 0;
        for (int i = 0; i < nshots; i++) if (shots[i] == f) shoot = true;
        if (shoot) {
            char path[512];
            snprintf(path, sizeof path, "%s/%s_%05u.ppm", shot_dir, game_name, f);
            if (sim_gfx_dump_ppm(path) != 0) fprintf(stderr, "nao consegui gravar %s\n", path);
            else printf("sim: captura %s (%dx%d)\n", path, gfx_width(), gfx_height());
        }
#ifdef SIM_SDL
        if (!headless) sdl_present();
#endif
        f++;
        if (frames && f >= frames) break;

        /* mesmo ritmo do vTaskDelayUntil(16 ms) da placa: se o quadro estourou, nao espera */
        next += FRAME_MS;
        const uint32_t t = doom_port_millis();
        if ((int32_t)(next - t) > 0) doom_port_sleep_ms(next - t);
        else next = t;
    }
    g->deinit();

    const uint32_t elapsed = doom_port_millis() - t_start;
    const double px_frame = f ? (double)sim_gfx_stats.pixels_sent / f : 0;
    printf("sim: %u quadros, %u ms %s, %.0f pixels/quadro enviados ao LCD (%.0f KB)\n",
           f, elapsed, headless ? "(virtual)" : "(real)", px_frame, px_frame * 2 / 1024);
    if (px_frame > 0) {
        /* barramento 8 bits: 1 byte por pulso de WR, 2 bytes por pixel; ignora comandos/CASET */
        printf("sim: teto do barramento i80: %.0f quadros/s a 10 MHz, %.0f a 20 MHz (so transferencia de pixels)\n",
               10e6 / (px_frame * 2), 20e6 / (px_frame * 2));
    }
    if (launcher_reboot_requested()) {
        printf("sim: o Doom saiu: na placa o chip reiniciaria aqui e o menu voltaria. Rode o simulador de novo.\n");
    }
    if (sim_gfx_stats.oob_pixels) {
        printf("sim: FALHA - %lu pixels desenhados fora da tela\n", sim_gfx_stats.oob_pixels);
        return 1;
    }
    return 0;
}