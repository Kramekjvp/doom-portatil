idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES game_api lcd_i80 input_gpio tetris_game esp_timer esp_hw_support)
