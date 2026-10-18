/*
 * Botoes em GPIO direto (ativos em nivel baixo, pull-up interno).
 * Devolve a mascara crua; as bordas e a repeticao ficam em input.h.
 */
#pragma once
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t input_gpio_init(void);
uint16_t  input_gpio_read(void);   /* bit BTN_x = 1 quando apertado */

#ifdef __cplusplus
}
#endif
