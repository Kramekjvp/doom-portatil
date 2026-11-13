#include <stdio.h>
#include "input.h"

static int fails;
#define CHECK(c, m) do { if (!(c)) { printf("FALHOU: %s\n", m); fails++; } else printf("ok: %s\n", m); } while (0)

int main(void)
{
    input_state_t s = {0};
    input_state_update(&s, BTN_MASK(BTN_LEFT));
    CHECK(s.pressed == BTN_MASK(BTN_LEFT) && s.down == BTN_MASK(BTN_LEFT) && !s.released, "aperta: pressed+down");
    input_state_update(&s, BTN_MASK(BTN_LEFT));
    CHECK(!s.pressed && s.down, "segurando: sem pressed novo");
    input_state_update(&s, 0);
    CHECK(s.released == BTN_MASK(BTN_LEFT) && !s.down, "solta: released");

    /* repeticao: 1o disparo ao apertar, depois de 170 ms, depois a cada 50 ms */
    input_state_t r = {0}; input_repeat_t rep = {0};
    uint32_t t = 1000; int disparos = 0; uint32_t quando[16];
    for (int i = 0; i < 40; i++, t += 16) {
        input_state_update(&r, BTN_MASK(BTN_RIGHT));
        if (input_repeat(&rep, &r, BTN_RIGHT, t, 170, 50)) { if (disparos < 16) quando[disparos] = t - 1000; disparos++; }
    }
    printf("info: disparos em ms: ");
    for (int i = 0; i < disparos && i < 16; i++) printf("%u ", quando[i]);
    printf("\n");
    CHECK(disparos >= 2 && quando[0] == 0 && quando[1] >= 170 && quando[1] < 200, "repeticao: apertou -> espera ~170 ms");
    CHECK(quando[2] - quando[1] >= 48 && quando[2] - quando[1] <= 66, "repeticao: depois a cada ~50 ms");

    /* soltar e apertar de novo reinicia o ciclo */
    input_state_update(&r, 0);
    input_state_update(&r, BTN_MASK(BTN_RIGHT));
    CHECK(input_repeat(&rep, &r, BTN_RIGHT, t, 170, 50), "novo aperto dispara na hora");

    /* relogio dando a volta (uint32) */
    input_state_t w = {0}; input_repeat_t rw = {0};
    uint32_t tt = 0xFFFFFFF0u;
    input_state_update(&w, BTN_MASK(BTN_DOWN));
    (void)input_repeat(&rw, &w, BTN_DOWN, tt, 100, 40);
    int ok = 0;
    for (int i = 0; i < 30; i++) { tt += 16; input_state_update(&w, BTN_MASK(BTN_DOWN)); if (input_repeat(&rw, &w, BTN_DOWN, tt, 100, 40)) ok = 1; }
    CHECK(ok, "repeticao funciona na virada do contador de 32 bits");

    printf("\n%s\n", fails ? "COM FALHAS" : "TUDO OK");
    return fails != 0;
}
