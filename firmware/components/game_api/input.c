#include "input.h"

void input_state_update(input_state_t *s, uint16_t raw)
{
    s->pressed  = (uint16_t)(raw & ~s->down);
    s->released = (uint16_t)(s->down & ~raw);
    s->down     = raw;
}

bool input_repeat(input_repeat_t *r, const input_state_t *s, input_btn_t b,
                  uint32_t now_ms, uint32_t first_ms, uint32_t every_ms)
{
    uint16_t m = BTN_MASK(b);
    if (s->pressed & m) {
        r->next_ms = now_ms + first_ms;
        return true;
    }
    if ((s->down & m) && (int32_t)(now_ms - r->next_ms) >= 0) {
        r->next_ms = now_ms + every_ms;
        return true;
    }
    return false;
}
