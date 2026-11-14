idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES game_api lcd_i80 input_gpio launcher game_registry
                       PRIV_REQUIRES esp_timer esp_system esp_hw_support log)