# ADR-0010: Doom no ESP32-S3 (doomgeneric) e simulador no PC

## Status
Proposta — validada no simulador do PC; falta compilar para a placa e medir.

Atualização 2026-11-12: caminhos citados abaixo (`firmware-s3/`, `examples/`,
`app/`) são do layout anterior; ver o mapa em `docs/historico.md`.

## Contexto
O Tetris já roda no ESP32-S3 pelas camadas do `firmware-s3` (ADR-0009): jogos só
conhecem `game_api` (`game.h`, `input.h`, `gfx.h`). Falta o Doom, que é o objetivo
central do projeto. A tela ainda não está soldada, então a primeira validação do
Doom precisa acontecer no PC, sem depender de hardware (princípio já adotado:
validar de ponta a ponta na máquina de desenvolvimento antes de ir ao hardware).

## Decisão

**1. Motor: doomgeneric** (ADR-0002), como submódulo em `firmware-s3/third_party/doomgeneric`.
Sem alterar o código dele: o port é um componente novo, `components/doom_game`, que
implementa `game_ops_t` e as funções `DG_*` do doomgeneric. Dois ajustes de
comportamento são feitos pelo linker (`--wrap`), sem editar o motor:
`I_Quit` (o "Quit Game" do menu vira "sair para o launcher") e `M_FileExists`
(o WAD virtual "existe" na placa).

**2. Vídeo.** Build `CMAP256` (framebuffer de 8 bits, 320×200). `DG_DrawFrame` converte
a paleta para RGB565 por uma tabela de 256 entradas e envia por `gfx_blit` em faixas
de 20 linhas. A tela passa a **paisagem 320×240** (`gfx_set_orientation`, novo em
`gfx.h`; no `lcd_i80` troca o MADCTL, valor em `LCD_I80_MADCTL_LANDSCAPE`). As 200
linhas são esticadas para 240 repetindo 1 a cada 5 (proporção 4:3 do VGA original,
`CONFIG_DOOM_STRETCH_4_3`). O buffer de DMA do `lcd_i80` sobe de 2048 para 6400 pixels
(faixas de 20 linhas em paisagem).

**3. WAD na flash, não em arquivo.** Partição `wad` (data/0x40, 6 MB) mapeada com
`esp_partition_mmap`; o Doom já sabe usar WAD mapeado (`wad_file_t.mapped`), então os
lumps são lidos direto da flash. Só o shareware `doom1.wad` (~4 MB) cabe com folga;
o `freedoom1.wad` (28 MB) não cabe nos 16 MB. WAD é dado, não código: fora do git.
microSD fica para a Fase 8.

**4. Controles com 8 botões** (tabela completa em `doom_game.c`): direcional anda/gira;
A atira; B usa; START abre o menu; SELECT segurado transforma ESQ/DIR em strafe e, em
toque rápido, troca de arma; START+SELECT por 1 s sai. Em menu/título/demo, A=Enter e
B=Esc (e `y`/`n` nas perguntas de confirmação). Corrida automática ligada. Se o hardware
ganhar mais botões (o MCP23017 permite até 16), a tabela é um único `switch`.

**5. Simulador no PC** (`firmware-s3/sim/`). Compila os **mesmos** fontes da placa
(`game_api`, `tetris_*`, `doom_game`, doomgeneric — lista única em
`doomgeneric_sources.txt`) e troca só o que toca hardware: tela (`sim_gfx.c`, backend de
`gfx.h` em framebuffer) e botões (teclado via SDL2, ou roteiro de botões por quadro no
modo `--headless`). O modo headless usa relógio virtual: determinístico e mais rápido
que o tempo real, com capturas `.ppm`.

## Consequências
- O Doom não é reentrante: depois de sair, só roda de novo reiniciando o chip. O
  launcher deve tratá-lo como "uma vez por boot" (`esp_restart()`).
- Sem som (`FEATURE_SOUND` desligado; áudio é a última camada, ADR-0005) e sem
  saves/config (não há sistema de arquivos ainda).
- O simulador valida lógica, mapeamento de botões, paleta/cores, proporção e quantidade
  de pixels enviados por quadro. **Não** valida desempenho nem memória do S3.
- O código do Doom é GPL-2: um binário do console contém código GPL. Mantenha o
  código-fonte disponível se distribuir e preserve a atribuição (id Software →
  Chocolate Doom → doomgeneric).

## Verificação (o que foi e o que não foi feito)
- Feito no PC (Linux, gcc): Doom (Freedoom 0.13 como WAD de teste) sobe, menu navegável só
  com os 8 botões, partida inicia, andar/girar/atirar, troca de arma, strafe, "Quit
  Game" com `y`/`n`, combo de saída; Tetris no mesmo simulador; `host_test` do Tetris
  continua com 0 falhas. Build com SDL2 e cross-build para Windows (mingw-w64) compilam;
  não foram executados no Windows.
- **Não compilado com o ESP-IDF** nem testado na placa: `doom_port_esp.c`, `w_file_esp.c`,
  `lcd_i80.c` (orientação), `CMakeLists.txt`, `examples/doom_standalone`. A lista de
  riscos da primeira compilação está em `examples/doom_standalone/README.md`.
- A medir na placa: quadros/s, heap livre com a zona de 6 MB, tamanho do binário.