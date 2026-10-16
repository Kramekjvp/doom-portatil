# Backend grafico da tela ILI9341 em paralelo 8 bits (periferico LCD_CAM do S3, via esp_lcd i80).
idf_component_register(SRCS "lcd_i80.c"
                       INCLUDE_DIRS "include"
                       REQUIRES game_api
                       PRIV_REQUIRES esp_lcd esp_driver_gpio)
