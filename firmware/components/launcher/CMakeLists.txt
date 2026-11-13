# Menu de jogos. Depende so de contratos (game_api): nao conhece Tetris nem Doom
# (quem monta o firmware passa a lista). Roda tambem no PC (host_test, sim).
idf_component_register(SRCS "launcher.c"
                       INCLUDE_DIRS "include"
                       REQUIRES game_api)