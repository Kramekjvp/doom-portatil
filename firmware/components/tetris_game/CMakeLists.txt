# Tela e fluxo do Tetris. Depende so de contratos (game_api) e das regras (tetris_core):
# nada de hardware aqui, por isso roda tambem no PC (host_test).
idf_component_register(SRCS "tetris_game.c"
                       INCLUDE_DIRS "include"
                       REQUIRES game_api tetris_core)
