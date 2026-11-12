/* Menu e repasse ao jogo escolhido. Ver launcher.h. Sem hardware: so gfx.h e input.h. */
#include <stddef.h>
#include "gfx.h"
#include "launcher.h"

#define C_BG      GFX_RGB565(12, 12, 28)
#define C_TITLE   GFX_RGB565(255, 64, 32)
#define C_TEXT    GFX_RGB565(220, 220, 220)
#define C_DIM     GFX_RGB565(130, 130, 150)
#define C_SEL_BG  GFX_RGB565(40, 60, 130)
#define C_SEL_FG  GFX_RGB565(255, 255, 255)

#define LIST_Y    90
#define ROW_H     44
#define ROW_PAD   6        /* respiro entre linhas */

typedef enum { ST_MENU, ST_PLAYING } state_t;

static const launcher_entry_t *s_entries;
static int s_count;
static int s_sel;
static state_t s_state;
static const launcher_entry_t *s_cur;     /* jogo em execucao */
static bool s_reboot;

void launcher_set_entries(const launcher_entry_t *entries, int count)
{
    s_entries = entries;
    s_count = entries ? count : 0;
    if (s_count > LAUNCHER_MAX_ENTRIES) s_count = LAUNCHER_MAX_ENTRIES;
    if (s_count < 0) s_count = 0;
    if (s_sel >= s_count) s_sel = 0;
}

bool launcher_reboot_requested(void) { return s_reboot; }

static void draw_menu(void)
{
    const int w = gfx_width(), h = gfx_height();
    gfx_fill_rect(0, 0, w, h, C_BG);
    gfx_text_centered(w / 2, 28, "DOOM PORTATIL", 2, C_TITLE, C_BG);
    gfx_text_centered(w / 2, 52, "ESP32-S3", 1, C_DIM, C_BG);

    if (s_count == 0) {
        gfx_text_centered(w / 2, LIST_Y, "nenhum jogo", 2, C_DIM, C_BG);
        return;
    }
    for (int i = 0; i < s_count; i++) {
        const int y = LIST_Y + i * ROW_H;
        const bool sel = (i == s_sel);
        const uint16_t bg = sel ? C_SEL_BG : C_BG;
        const bool nota = s_entries[i].once_per_boot;
        if (sel) gfx_fill_round_rect(12, y, w - 24, ROW_H - ROW_PAD, C_SEL_BG);
        gfx_text_centered(w / 2, y + (nota ? 4 : 11), s_entries[i].game->name, 2,
                          sel ? C_SEL_FG : C_TEXT, bg);
        if (nota) gfx_text_centered(w / 2, y + 24, "reinicia ao sair", 1, C_DIM, bg);
    }
    gfx_text_centered(w / 2, h - 28, "CIMA/BAIXO: escolher", 1, C_DIM, C_BG);
    gfx_text_centered(w / 2, h - 16, "A ou START: jogar", 1, C_DIM, C_BG);
}

static void enter_menu(void)
{
    gfx_set_orientation(GFX_PORTRAIT);     /* o Doom troca para paisagem; o menu e em retrato */
    s_state = ST_MENU;
    s_cur = NULL;
    draw_menu();
}

static void launcher_init(void)
{
    s_reboot = false;
    enter_menu();
}

static bool launcher_tick(const game_ctx_t *ctx)
{
    if (s_state == ST_PLAYING) {
        if (s_cur->game->tick(ctx)) return true;
        s_cur->game->deinit();
        if (s_cur->once_per_boot) {          /* motor nao reentrante: quem tem o laco reinicia */
            s_reboot = true;
            return false;
        }
        enter_menu();
        return true;
    }

    if (s_count == 0) return true;
    const uint16_t p = ctx->in->pressed;      /* so bordas: um A segurado nao relanca o jogo */
    if (p & BTN_MASK(BTN_UP))   { s_sel = (s_sel + s_count - 1) % s_count; draw_menu(); }
    if (p & BTN_MASK(BTN_DOWN)) { s_sel = (s_sel + 1) % s_count;           draw_menu(); }
    if (p & (BTN_MASK(BTN_A) | BTN_MASK(BTN_START))) {
        s_cur = &s_entries[s_sel];
        gfx_fill_rect(0, 0, gfx_width(), gfx_height(), 0x0000);   /* nao herdar o menu */
        s_cur->game->init();
        s_state = ST_PLAYING;
    }
    return true;
}

static void launcher_deinit(void)
{
    if (s_state == ST_PLAYING && s_cur) s_cur->game->deinit();
    s_state = ST_MENU;
    s_cur = NULL;
}

const game_ops_t launcher_game = {
    .name = "Launcher",
    .init = launcher_init,
    .tick = launcher_tick,
    .deinit = launcher_deinit,
};
