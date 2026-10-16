# Testes no PC (sem hardware). Requer gcc (Linux, WSL ou MSYS2/MinGW no Windows).
#   make            -> compila e roda tudo
#   make SAN=-fsanitize=address,undefined   (Linux/WSL: acha estouro de indice e UB)
# As capturas de tela do Tetris saem na pasta shots/ (arquivos .ppm)
CC     ?= gcc
SAN    ?=
CFLAGS  = -std=c11 -Wall -Wextra -g $(SAN)
COMP    = ../components
INC     = -I$(COMP)/game_api/include -I$(COMP)/tetris_core/include -I$(COMP)/tetris_game/include

.PHONY: all clean test_input test_core test_game
all: test_input test_core test_game

test_input: test_input.c $(COMP)/game_api/input.c
	$(CC) $(CFLAGS) $(INC) $^ -o $@ && ./$@

test_core: test_tetris_core.c $(COMP)/tetris_core/tetris.c
	$(CC) $(CFLAGS) $(INC) $^ -o $@ && ./$@

test_game: test_tetris_game.c fake_gfx.c $(COMP)/game_api/input.c $(COMP)/game_api/gfx_util.c $(COMP)/tetris_core/tetris.c
	$(CC) $(CFLAGS) $(INC) $^ -o $@ && ./$@

clean:
	rm -rf test_input test_core test_game shots
