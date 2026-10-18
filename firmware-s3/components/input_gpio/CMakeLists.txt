# Backend de entrada com botoes ligados direto em GPIO (bancada / MVP).
# Troque por um backend MCP23017 (I2C) mantendo input_gpio_read() -> mascara.
idf_component_register(SRCS "input_gpio.c"
                       INCLUDE_DIRS "include"
                       REQUIRES game_api
                       PRIV_REQUIRES esp_driver_gpio)
