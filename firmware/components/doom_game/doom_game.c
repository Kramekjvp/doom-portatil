/*
 * Doom como game_ops_t. Duas responsabilidades:
 *   1. subir o doomgeneric (init) e dar um passo por quadro (tick);
 *   2. traduzir os 8 botoes do console em teclas do Doom.
 *
 * Mapeamento (so 8 botoes; ver docs/adr/0010):
 *
 *   botao      no jogo                         em menu / titulo / demo
 *   ---------  ------------------------------  -----------------------
 *   CIMA/BAIXO andar / recuar                  navegar
 *   ESQ/DIR    girar                           ajustar
 *   A          atirar                          confirmar (Enter; 'y' em "tem certeza?")
 *   B          usar / abrir porta              voltar (Esc; 'n' em "tem certeza?")
 *   START      abre o menu do Doom (Esc)       voltar
 *   SELECT     segurado: ESQ/DIR viram strafe  -
 *              toque rapido (sem direcional): proxima arma
 *   START+SELECT segurados por 1 s             sai para o launcher
 *
 * O contexto (jogo x menu) e lido do estado do proprio Doom, e a tecla enviada na
 * pressao fica guardada para que a SOLTURA seja a mesma tecla mesmo se o contexto mudar.
 */
#include <stdio.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include "sdkconfig.h"
#include "esp_log.h"
#else
#define CONFIG_DOOM_ALWAYS_RUN 1
#define CONFIG_DOOM_ZONE_MB    6
#endif

#include "doom_game.h"
#include "doom_port.h"
#include "dg_portatil.h"
#include "gfx.h"

#include "doomgeneric.h"
#include "doomkeys.h"
#include "doomstat.h"      /* menuactive, demoplayback, gamestate */
#include "m_controls.h"    /* key_strafe, key_nextweapon, joybspeed */
#include "i_system.h"
#include "m_misc.h"

/* globais do m_menu.c (nao ha header que as exporte) */
extern int     messageToPrint;
extern boolean messageNeedsInput;

#define SELECT_TAP_MS      300
#define EXIT_COMBO_MS     1000
#define KEY_NEXT_WEAPON    ']'
#define KEY_PREV_WEAPON    '['

static const char *s_iwad = "doom1.wad";
static bool s_started;      /* doomgeneric_Create ja rodou (nao e reentrante)   */
static bool s_quit;         /* "Quit Game" do menu ou combo START+SELECT        */
static bool s_failed;       /* init nao concluiu: nao chamar o motor            */
static unsigned char s_sent[BTN_COUNT];     /* tecla enviada em cada botao (0 = nenhuma) */
static uint32_t s_select_down_ms;
static bool     s_select_used;              /* um direcional foi usado com SELECT segurado */
static uint32_t s_combo_since_ms;           /* 0 = combo inativo */

void doom_game_set_iwad(const char *path) { if (path && *path) s_iwad = path; }

/* ---------------- desvios do linker (--wrap), sem alterar o doomgeneric ---------------- */

/* Doom -> "Quit Game": no doomgeneric I_Quit so roda os atexit e RETORNA. Marcamos a saida. */
void __wrap_I_Quit(void)
{
    s_quit = true;
}

/* Na placa nao ha sistema de arquivos para o WAD: o nome virtual "existe". */
boolean __real_M_FileExists(char *filename);
boolean __wrap_M_FileExists(char *filename)
{
    if (doom_port_wad_is_virtual(filename)) return true;
    return __real_M_FileExists(filename);
}

/* ---------------- botoes -> teclas ---------------- */

static void press(int btn, unsigned char key)
{
    s_sent[btn] = key;
    dg_key_push(1, key);
}

static void release(int btn)
{
    if (s_sent[btn]) dg_key_push(0, s_sent[btn]);
    s_sent[btn] = 0;
}

static void feed_input(const game_ctx_t *ctx)
{
    const input_state_t *in = ctx->in;
    const bool in_menu = menuactive || demoplayback || gamestate == GS_DEMOSCREEN;
    const bool prompt  = in_menu && messageToPrint && messageNeedsInput;   /* "tem certeza? (y/n)" */

    /* soltou: sempre a mesma tecla que foi enviada na pressao */
    for (int b = 0; b < BTN_COUNT; b++) {
        if (!(in->released & BTN_MASK(b))) continue;
        if (b == BTN_SELECT) {
            release(b);                                     /* solta o modificador de strafe */
            const bool tap = (ctx->now_ms - s_select_down_ms) < SELECT_TAP_MS
                             && !s_select_used && !(in->down & BTN_MASK(BTN_START));
            if (tap && !in_menu) {                           /* toque rapido: proxima arma */
                dg_key_push(1, KEY_NEXT_WEAPON);
                dg_key_push(0, KEY_NEXT_WEAPON);
            }
        } else {
            release(b);
        }
    }

    /* apertou */
    for (int b = 0; b < BTN_COUNT; b++) {
        if (!(in->pressed & BTN_MASK(b))) continue;
        switch (b) {
        case BTN_UP:    press(b, KEY_UPARROW);    break;
        case BTN_DOWN:  press(b, KEY_DOWNARROW);  break;
        case BTN_LEFT:  press(b, KEY_LEFTARROW);  break;
        case BTN_RIGHT: press(b, KEY_RIGHTARROW); break;
        case BTN_A:     press(b, prompt ? 'y' : in_menu ? KEY_ENTER  : KEY_FIRE); break;
        case BTN_B:     press(b, prompt ? 'n' : in_menu ? KEY_ESCAPE : KEY_USE);  break;
        case BTN_START: press(b, KEY_ESCAPE);     break;
        case BTN_SELECT:
            s_select_down_ms = ctx->now_ms;
            s_select_used = false;
            press(b, (unsigned char)key_strafe);              /* modificador de strafe */
            break;
        default: break;
        }
        if (b <= BTN_RIGHT && (in->down & BTN_MASK(BTN_SELECT))) s_select_used = true;
    }

    /* START+SELECT segurados: sair */
    if ((in->down & BTN_MASK(BTN_START)) && (in->down & BTN_MASK(BTN_SELECT))) {
        if (s_combo_since_ms == 0) s_combo_since_ms = ctx->now_ms ? ctx->now_ms : 1;
        else if (ctx->now_ms - s_combo_since_ms >= EXIT_COMBO_MS) s_quit = true;
    } else {
        s_combo_since_ms = 0;
    }
}

/* ---------------- game_ops_t ---------------- */

static void doom_init(void)
{
    s_failed = false;
    s_quit = false;
    if (s_started) {
        /* o motor nao e reentrante: so um Doom por boot */
        s_failed = true;
        printf("doom_game: Doom ja foi executado neste boot; reinicie o chip para jogar de novo\n");
        return;
    }

    gfx_set_orientation(GFX_LANDSCAPE);
    gfx_text_centered(gfx_width() / 2, gfx_height() / 2 - 12, "DOOM", 3,
                      GFX_RGB565(255, 0, 0), GFX_RGB565(0, 0, 0));
    gfx_text_centered(gfx_width() / 2, gfx_height() / 2 + 20, "carregando...", 1,
                      GFX_RGB565(160, 160, 160), GFX_RGB565(0, 0, 0));

    memset(s_sent, 0, sizeof s_sent);
    s_combo_since_ms = 0;
    s_select_used = false;

    static char mb[8];
    snprintf(mb, sizeof mb, "%d", CONFIG_DOOM_ZONE_MB);
    static char *argv[] = { "doom", "-iwad", NULL, "-mb", NULL, NULL };
    argv[2] = (char *)s_iwad;
    argv[4] = mb;

    s_started = true;
    doomgeneric_Create(5, argv);       /* D_DoomMain: carrega o WAD e inicia o motor */

    /* ajustes depois de o Doom ler o default.cfg, para valerem sempre */
    key_nextweapon = KEY_NEXT_WEAPON;
    key_prevweapon = KEY_PREV_WEAPON;
#if CONFIG_DOOM_ALWAYS_RUN
    joybspeed = 29;                    /* >= MAX_JOY_BUTTONS: o Doom correr sempre */
#endif
}

static bool doom_tick(const game_ctx_t *ctx)
{
    if (s_failed || s_quit) return false;
    feed_input(ctx);
    doomgeneric_Tick();                /* >= 1 tic do jogo (1/35 s); chama DG_DrawFrame */
    return !s_quit;
}

static void doom_deinit(void)
{
    gfx_set_orientation(GFX_PORTRAIT);
}

const game_ops_t doom_game = {
    .name = "Doom",
    .init = doom_init,
    .tick = doom_tick,
    .deinit = doom_deinit,
};
