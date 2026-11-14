# Lista oficial de jogos do console (ordem do menu). Usada pela placa (main/) e pelo simulador (sim/).
# Sem o Doom: defina a variavel de AMBIENTE PORTATIL_SEM_DOOM=1 antes de rodar o idf.py.
# (-D nao serve: o ESP-IDF le os REQUIRES antes de ver as variaveis do CMake; o ambiente ele ve.)
set(deps game_api launcher tetris_game)
if(NOT "$ENV{PORTATIL_SEM_DOOM}" STREQUAL "1")
    list(APPEND deps doom_game)
endif()
idf_component_register(SRCS "game_registry.c"
                       INCLUDE_DIRS "include"
                       REQUIRES ${deps})