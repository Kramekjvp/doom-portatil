/*
 * Driver da tela TFT ILI9341 (240x320) por SPI, usando o esp_lcd (panel IO SPI) com DMA.
 * Modulo 2,8" de 14 pinos (VCC, GND, CS, RESET, DC, SDI, SCK, LED, ...). ADR-0013.
 *
 * Depois de lcd_spi_init(), os jogos usam so a API de gfx.h (gfx_fill_rect / gfx_blit),
 * que este componente implementa no lugar do lcd_i80 (escolha no menuconfig).
 * NAO e thread-safe: desenhe sempre da mesma task.
 */
#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t lcd_spi_init(void);

#ifdef __cplusplus
}
#endif