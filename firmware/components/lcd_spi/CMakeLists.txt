# Backend grafico da tela ILI9341 por SPI (modulo 2,8" de 14 pinos), via esp_lcd (ADR-0013).
# Compila vazio se a tela escolhida no menuconfig for o shield paralelo (lcd_i80).
idf_component_register(SRCS "lcd_spi.c"
                       INCLUDE_DIRS "include"
                       REQUIRES game_api
                       PRIV_REQUIRES esp_lcd esp_driver_gpio esp_driver_spi)