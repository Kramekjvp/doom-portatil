# Contratos compartilhados entre launcher, jogos e drivers (entrada, graficos, jogo).
# Sem dependencia de hardware: compila igual no ESP-IDF e no PC (host_test).
idf_component_register(SRCS "input.c" "gfx_util.c"
                       INCLUDE_DIRS "include")
