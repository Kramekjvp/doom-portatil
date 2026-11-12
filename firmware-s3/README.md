# firmware-s3 — Tetris + tela paralela (i80)

Componentes ESP-IDF (v6.0.x) para o ESP32-S3 N16R8 com a tela TFT 2,4" ILI9341
em paralelo de 8 bits. Cada camada so conhece a de baixo:

```
tetris_standalone (main)  ──►  tetris_game  ──►  tetris_core   (regras, C puro)
        │                          │
        ▼                          ▼
   lcd_i80 / input_gpio  ◄──  game_api  (gfx.h, input.h, game.h: so contratos)
   (hardware)
```

| Componente    | Papel                                                                  |
|---------------|------------------------------------------------------------------------|
| `game_api`    | Contratos: `game_ops_t` (jogo), `input.h` (botoes), `gfx.h` (tela) + fonte 5x7 |
| `tetris_core` | Regras do Tetris. **Autoria: dupla do projeto.** Unica mudanca: chute vertical no giro |
| `tetris_game` | Menu, partida, pausa e fim de jogo; implementa `game_ops_t`            |
| `launcher`    | Menu de jogos (é um `game_ops_t`); recebe a lista de quem monta o firmware; ADR-0011 |
| `doom_game`   | Doom (doomgeneric em `third_party/`) como `game_ops_t`; ADR-0010        |
| `lcd_i80`     | Driver ILI9341 (esp_lcd i80 + DMA); implementa o backend de `gfx.h`; retrato e paisagem |
| `input_gpio`  | Botoes em GPIO direto (bancada). Trocavel por MCP23017 sem mexer nos jogos |

## Pinos padrao (mude em `idf.py menuconfig`)

Evitam 0, 3, 19, 20 (strapping/USB), 26-37 (flash/PSRAM octal), 38 e 48 (LED RGB
em algumas placas), 43-46.

| Sinal da tela | GPIO | | Botao (GPIO -> GND) | GPIO |
|---|---|---|---|---|
| LCD_D0..D7 | 4, 5, 6, 7, 15, 16, 17, 18 | | BAIXO | 1 |
| LCD_WR | 8 | | ESQUERDA | 2 |
| LCD_RS | 9 | | DIREITA | 42 |
| LCD_CS | 10 | | A (girar) | 41 |
| LCD_RST | 14 | | B (queda) | 40 |
| LCD_RD | 21 (fica em nivel alto; -1 = ligar em 3V3) | | START (pausa) | 39 |

A ordem D0..D7 e livre (o LCD_CAM roteia qualquer GPIO), mas D0..D7 precisam ir
aos pinos LCD_D0..LCD_D7 correspondentes da tela.

## Testes no PC (sem hardware)

```
cd host_test
make                                                      # gcc (Linux, WSL ou MSYS2)
make SAN="-fsanitize=address,undefined"                   # Linux/WSL: acha estouro/UB
```

Rodam: entrada (bordas e repeticao), regras do Tetris (7-bag, giro, pontos, nivel,
fuzz de 1 milhao de acoes) e o fluxo completo do jogo sobre um framebuffer falso.
As capturas saem em `host_test/shots/` (`exemplo_telas.png` mostra o resultado).

## Gravar na placa

```
cd examples/tetris_standalone
idf.py set-target esp32s3
idf.py menuconfig        # confira pinos em "LCD i80" e "Entrada"
idf.py build flash monitor
```

Ao ligar, aparecem 2,5 s de barras vermelha/verde/azul (de cima para baixo) com um
quadrado branco no canto superior esquerdo. Se as cores estiverem trocadas ou a
imagem espelhada, ajuste `LCD_I80_MADCTL` / `LCD_I80_INVERT` no menuconfig.

## Estado da verificacao

- Verificado no PC: regras, entrada, fluxo e desenho do Tetris (testes acima).
- Sintaxe do codigo de hardware (`lcd_i80`, `input_gpio`, `main`) conferida contra os
  headers reais de `esp_lcd` do ESP-IDF v6.0.3. **Ainda nao compilado com o ESP-IDF
  nem testado na placa.**

## Relação com o restante do repositório

- Decisões: `../docs/adr/0008-tela-ili9341-paralela-i80.md` (tela) e
  `../docs/adr/0009-camada-de-jogos-e-reuso-do-tetris.md` (camadas e Tetris).
- Correspondência com as pastas sugeridas na ADR-0005: `graphics/` = `lcd_i80` +
  `gfx.h`; `input/` = `input_gpio` + `input.h`; `games/tetris/` = `tetris_core` +
  `tetris_game`; `launcher/` ainda não existe.
- `CMakeLists.txt`, `partitions.csv` e `sdkconfig.defaults` na raiz de `firmware-s3/`
  estão vazios (reservados para o projeto principal com Doom). O Tetris compila pelo
  projeto em `examples/tetris_standalone/`.

## Doom e simulador no PC

- `components/doom_game` + `examples/doom_standalone` (placa) e `sim/` (PC): ver
  `../docs/adr/0010-doom-no-s3-e-simulador-no-pc.md`, `examples/doom_standalone/README.md` e
  `sim/README.md`.
- `gfx.h` ganhou `gfx_set_orientation()` (retrato 240×320 / paisagem 320×240); o Doom usa paisagem.
- Estado: validado no simulador; **não compilado com o ESP-IDF nem testado na placa**.

## Firmware único (launcher)

- `app/` é o firmware do console (launcher + Tetris + Doom); `examples/` ficam para depurar cada jogo.
  Ver `app/README.md` e `../docs/adr/0011-launcher-e-firmware-unico.md`.
- Estado: validado no simulador e no `host_test`; **não compilado com o ESP-IDF**.