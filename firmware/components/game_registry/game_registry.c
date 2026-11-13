#include "game_registry.h"

#include <stddef.h>

#include "tetris_game.h"
#if __has_include("doom_game.h")
#include "doom_game.h"
#define REGISTRY_TEM_DOOM 1
#endif

static const launcher_entry_t ENTRIES[] = {
    { &tetris_game, false },
#ifdef REGISTRY_TEM_DOOM
    { &doom_game,   true  },     /* motor nao reentrante: ao sair, o chip reinicia */
#endif
};

const launcher_entry_t *game_registry_entries(int *count)
{
    if (count) *count = (int)(sizeof ENTRIES / sizeof ENTRIES[0]);
    return ENTRIES;
}

static char minuscula(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }

static int mesmo_nome(const char *a, const char *b)
{
    for (; *a && *b; a++, b++)
        if (minuscula(*a) != minuscula(*b)) return 0;
    return *a == *b;
}

const game_ops_t *game_registry_find(const char *name)
{
    if (!name) return NULL;
    for (size_t i = 0; i < sizeof ENTRIES / sizeof ENTRIES[0]; i++)
        if (mesmo_nome(ENTRIES[i].game->name, name)) return ENTRIES[i].game;
    return NULL;
}
