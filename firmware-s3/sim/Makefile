# Simulador do console no PC (Windows/MSYS2 UCRT64, Linux, macOS).
#
#   make                      compila build/portatil_sim (com janela se achar o SDL2)
#   make headless             compila SEM SDL2 (so --headless)
#   make test-headless        roda os jogos sem janela e confere (precisa de WAD=arquivo.wad para o Doom)
#   make clean
#
# Requisitos no MSYS2 (terminal UCRT64):
#   pacman -S mingw-w64-ucrt-x86_64-gcc make mingw-w64-ucrt-x86_64-SDL2
# O doomgeneric vem do submodulo:  git submodule update --init --recursive
#
# O simulador compila os MESMOS fontes da placa: game_api, tetris_*, doom_game e o doomgeneric
# (lista em ../components/doom_game/doomgeneric_sources.txt). So muda a tela e os botoes.

CC      ?= gcc
ROOT     = ..
COMP     = $(ROOT)/components
DG       = $(ROOT)/third_party/doomgeneric/doomgeneric
BUILD    = build
EXE      = $(BUILD)/portatil_sim
HASH     = \#

ifeq ($(OS),Windows_NT)
  EXE   := $(EXE).exe
  WINLIBS = -lwinmm
endif

# SDL2: usado se o sdl2-config existir (ou forcado com SDL=0 / SDL=1)
SDL ?= auto
ifeq ($(SDL),auto)
  SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
  SDL_LIBS   := $(shell sdl2-config --libs 2>/dev/null)
else ifeq ($(SDL),1)
  SDL_CFLAGS := $(shell sdl2-config --cflags)
  SDL_LIBS   := $(shell sdl2-config --libs)
endif
ifneq ($(strip $(SDL_LIBS)),)
  SIM_DEFS = -DSIM_SDL
endif

INC = -I$(COMP)/game_api/include -I$(COMP)/tetris_core/include -I$(COMP)/tetris_game/include \
      -I$(COMP)/doom_game/include -I$(COMP)/launcher/include -I$(DG) -I.
DOOMDEFS = -DCMAP256 -DDOOMGENERIC_RESX=320 -DDOOMGENERIC_RESY=200 -DNORMALUNIX -D_DEFAULT_SOURCE
CFLAGS  ?= -O2 -g
CFLAGS  += -std=gnu11 -Wall -Wextra

DG_NAMES := $(shell sed '/^$(HASH)/d;/^[[:space:]]*$$/d' $(COMP)/doom_game/doomgeneric_sources.txt)
DG_OBJS  := $(patsubst %.c,$(BUILD)/dg/%.o,$(DG_NAMES)) $(BUILD)/dg/w_file.o $(BUILD)/dg/w_file_stdc.o

OUR_SRCS = sim_main.c sim_gfx.c doom_port_pc.c \
           $(COMP)/game_api/input.c $(COMP)/game_api/gfx_util.c \
           $(COMP)/tetris_core/tetris.c $(COMP)/tetris_game/tetris_game.c \
           $(COMP)/doom_game/doom_game.c $(COMP)/doom_game/dg_portatil.c \
           $(COMP)/launcher/launcher.c
OUR_OBJS = $(patsubst %.c,$(BUILD)/our/%.o,$(notdir $(OUR_SRCS)))

# Desvios do linker iguais aos da placa (ver components/doom_game/CMakeLists.txt)
LDFLAGS += -Wl,--wrap=I_Quit -Wl,--wrap=M_FileExists

.PHONY: all headless test-headless clean
all: $(EXE)

headless:
	$(MAKE) SDL=0 BUILD=build_headless

$(EXE): $(OUR_OBJS) $(DG_OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) $(SDL_LIBS) -lm $(WINLIBS)

# codigo de terceiros (GPL, 1993): sem avisos
$(BUILD)/dg/%.o: $(DG)/%.c | $(BUILD)/dg
	$(CC) -O2 -w $(INC) $(DOOMDEFS) -c $< -o $@

$(BUILD)/our/%.o: %.c | $(BUILD)/our
	$(CC) $(CFLAGS) $(INC) $(DOOMDEFS) $(SIM_DEFS) $(SDL_CFLAGS) -c $< -o $@
$(BUILD)/our/%.o: $(COMP)/game_api/%.c | $(BUILD)/our
	$(CC) $(CFLAGS) $(INC) -c $< -o $@
$(BUILD)/our/%.o: $(COMP)/tetris_core/%.c | $(BUILD)/our
	$(CC) $(CFLAGS) $(INC) -c $< -o $@
$(BUILD)/our/%.o: $(COMP)/tetris_game/%.c | $(BUILD)/our
	$(CC) $(CFLAGS) $(INC) -c $< -o $@
$(BUILD)/our/%.o: $(COMP)/launcher/%.c | $(BUILD)/our
	$(CC) $(CFLAGS) $(INC) -c $< -o $@
$(BUILD)/our/%.o: $(COMP)/doom_game/%.c | $(BUILD)/our
	$(CC) $(CFLAGS) $(INC) $(DOOMDEFS) -c $< -o $@

$(BUILD)/dg $(BUILD)/our:
	mkdir -p $@

# --- testes sem janela (tempo virtual, roteiro de botoes) ---
WAD ?=
test-headless: $(EXE)
	$(EXE) --game tetris --headless --frames 120 --script "20:*A 40:*LEFT 60:*B" --shot 119 --shot-dir shots
	$(EXE) --game launcher --headless --frames 200 --script "10:*A 40:*START 100:*SELECT" --shot 5 --shot 30 --shot 90 --shot 150 --shot-dir shots
ifneq ($(WAD),)
	$(EXE) --game doom --wad $(WAD) --headless --frames 400 --shot 399 --shot-dir shots
	$(EXE) --game launcher --wad $(WAD) --headless --frames 400 --script "10:*DOWN 30:*A" --shot 399 --shot-dir shots
endif

clean:
	rm -rf build build_headless shots default.cfg