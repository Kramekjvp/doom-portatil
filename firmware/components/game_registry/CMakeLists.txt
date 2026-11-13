# Lista oficial de jogos do console (ordem do menu). Usada pela placa (main/) e pelo simulador (sim/).
# Sem o Doom (-DPORTATIL_SEM_DOOM=1) o componente doom_game nem entra na compilacao.
set(deps game_api launcher tetris_game)
if(NOT PORTATIL_SEM_DOOM)
    list(APPEND deps doom_game)
endif()
idf_component_register(SRCS "game_registry.c"
                       INCLUDE_DIRS "include"
                       REQUIRES ${deps})
