/* Plataforma PC do Doom: relogio (real ou virtual) e espera. WAD: arquivo normal. */
#include <stdint.h>
#include "doom_port.h"
#include "sim_platform.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>        /* timeBeginPeriod (-lwinmm) */
#else
#include <time.h>
#include <unistd.h>
#endif

static bool     s_virtual;
static uint32_t s_vclock = 1000;     /* nao comeca em 0, para nao confundir "ainda nao medido" */

void sim_clock_use_virtual(bool on) { s_virtual = on; }
void sim_clock_advance(uint32_t ms) { if (s_virtual) s_vclock += ms; }

#ifdef _WIN32
static uint32_t real_ms(void)
{
    static LARGE_INTEGER freq, t0;
    LARGE_INTEGER now;
    if (!freq.QuadPart) {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&t0);
        timeBeginPeriod(1);                        /* Sleep(1) de ~1 ms, nao ~15 ms */
    }
    QueryPerformanceCounter(&now);
    return (uint32_t)((now.QuadPart - t0.QuadPart) * 1000 / freq.QuadPart) + 1000;
}
static void real_sleep(uint32_t ms) { Sleep(ms); }
#else
static uint32_t real_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000u + ts.tv_nsec / 1000000u);
}
static void real_sleep(uint32_t ms) { usleep(ms * 1000u); }
#endif

uint32_t doom_port_millis(void) { return s_virtual ? s_vclock : real_ms(); }

void doom_port_sleep_ms(uint32_t ms)
{
    if (s_virtual) s_vclock += ms;
    else real_sleep(ms);
}

bool doom_port_wad_is_virtual(const char *name) { (void)name; return false; }   /* PC: arquivo de verdade */
