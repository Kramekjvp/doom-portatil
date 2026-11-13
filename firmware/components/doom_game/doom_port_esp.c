/* Plataforma ESP-IDF do Doom: relogio, espera e nome virtual do WAD. */
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "doom_port.h"

/* Nome que o Doom recebe em -iwad (doom_game.c) e que w_file_esp.c mapeia para a particao. */
#define VIRTUAL_WAD_NAME "doom1.wad"

uint32_t doom_port_millis(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

void doom_port_sleep_ms(uint32_t ms)
{
    const TickType_t t = pdMS_TO_TICKS(ms);
    vTaskDelay(t ? t : 1);        /* pelo menos 1 tick: cede a CPU (FREERTOS_HZ=1000 -> 1 ms) */
}

bool doom_port_wad_is_virtual(const char *name)
{
    return name && strcmp(name, VIRTUAL_WAD_NAME) == 0;
}
