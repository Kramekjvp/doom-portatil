#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "input.h"
#include "input_gpio.h"

static const char *TAG = "input_gpio";

/* indice = input_btn_t */
static const int8_t PINS[BTN_COUNT] = {
    [BTN_UP]     = CONFIG_INPUT_GPIO_UP,
    [BTN_DOWN]   = CONFIG_INPUT_GPIO_DOWN,
    [BTN_LEFT]   = CONFIG_INPUT_GPIO_LEFT,
    [BTN_RIGHT]  = CONFIG_INPUT_GPIO_RIGHT,
    [BTN_A]      = CONFIG_INPUT_GPIO_A,
    [BTN_B]      = CONFIG_INPUT_GPIO_B,
    [BTN_START]  = CONFIG_INPUT_GPIO_START,
    [BTN_SELECT] = CONFIG_INPUT_GPIO_SELECT,
};

esp_err_t input_gpio_init(void)
{
    for (int b = 0; b < BTN_COUNT; b++) {
        if (PINS[b] < 0) continue;
        const gpio_config_t cfg = {
            .pin_bit_mask = 1ULL << PINS[b],
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
        };
        ESP_RETURN_ON_ERROR(gpio_config(&cfg), TAG, "gpio_config %d", PINS[b]);
    }
    return ESP_OK;
}

uint16_t input_gpio_read(void)
{
    uint16_t raw = 0;
    for (int b = 0; b < BTN_COUNT; b++) {
        if (PINS[b] >= 0 && gpio_get_level((gpio_num_t)PINS[b]) == 0) raw |= BTN_MASK(b);
    }
    return raw;
}
