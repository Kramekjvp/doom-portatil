/*
 * Driver da tela TFT 2,4" ILI9341 (240x320) em paralelo 8 bits, usando o
 * periferico LCD_CAM do ESP32-S3 (esp_lcd i80) com DMA.
 *
 * Depois de lcd_i80_init(), os jogos usam so a API de gfx.h
 * (gfx_fill_rect / gfx_blit), que este componente implementa.
 * NAO e thread-safe: desenhe sempre da mesma task.
 */
#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t lcd_i80_init(void);

#ifdef __cplusplus
}
#endif
