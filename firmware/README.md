# firmware — console (ESP32-S3 N16R8)

Esta pasta **é** o projeto ESP-IDF (6.0.x): `main/` e `components/` são achados
automaticamente. Pinos: `../hardware/pinos.md`. Decisões: `../docs/adr/`.

## Camadas

Cada camada só conhece a de baixo. **Jogos nunca leem GPIO nem falam com a tela**: usam
só os contratos de `game_api` (ADR-0009).

```
main/ (laço de quadros na placa)        sim/sim_main.c (laço no PC)
        └──────────► game_registry ◄──────────┘     (lista de jogos: Tetris, Doom)
                           │
              launcher ──► tetris_game ──► tetris_core    (regras em C puro, autoria da dupla)
                    └────► doom_game ───► ../third_party/doomgeneric   (GPL, ADR-0010)
                           │
              game_api  (game.h, input.h, gfx.h: só contratos, sem hardware)
                           │
              lcd_i80 (tela)      input_gpio (botões)     ← únicos que tocam hardware
```

| Componente | Papel |
|---|---|
| `game_api` | Contratos: `game_ops_t` (jogo), `input.h` (botões), `gfx.h` (tela, retrato/paisagem) e fonte 5×7 |
| `tetris_core` | Regras do Tetris. **Autoria: dupla do projeto.** Única mudança: chute vertical no giro |
| `tetris_game` | Menu, partida, pausa e fim de jogo; implementa `game_ops_t` |
| `doom_game` | Doom (doomgeneric) como `game_ops_t`; WAD numa partição de flash (ADR-0010) |
| `launcher` | Menu de jogos (é um `game_ops_t`); recebe a lista de quem monta o firmware (ADR-0011) |
| `game_registry` | **Única** lista de jogos e da ordem do menu; usada pela placa e pelo simulador |
| `lcd_i80` | Driver ILI9341 (esp_lcd i80 + DMA); implementa o backend de `gfx.h` |
| `input_gpio` | Botões em GPIO direto (bancada). Será trocado pelo MCP23017 sem mexer nos jogos |

Jogo novo: criar o componente (`game_ops_t`) e acrescentar **uma linha** em
`components/game_registry/game_registry.c`.

## Gravar na placa (terminal do ESP-IDF 6.0.3)

```
cd firmware
idf.py set-target esp32s3
idf.py menuconfig        # "LCD i80", "Entrada: botoes em GPIO direto", "Doom (doomgeneric)"
idf.py build flash
python -m esptool --chip esp32s3 -p COMx write_flash 0x310000 ..\wad\doom1.wad
idf.py monitor
```

- **Primeira compilação, sem o Doom** (menu + Tetris): `idf.py -DPORTATIL_SEM_DOOM=1 build`.
  Para voltar ao normal: `-DPORTATIL_SEM_DOOM=0`. (Esta flag não foi testada no ESP-IDF.)
- Doom: `git submodule update --init --recursive` e `sh tools/preparar_wad.sh CAMINHO/doom1.wad`
  (valida o WAD e o copia para `wad/doom1.wad`). O WAD **não** vai para o git.
  O `doom1.wad` usado nos testes (4.196.020 bytes) é detectado pelo motor como v1.8: roda,
  só imprime um aviso.
- Partições (`partitions.csv`): o WAD fica na partição `wad` (0x310000, 6 MB). Se a tabela
  mudar, ou se a placa veio de outro projeto, rode `idf.py erase-flash` uma vez e regrave tudo.
- Ao ligar, 2,5 s de barras vermelha/verde/azul (de cima para baixo) com um quadrado branco no
  canto superior esquerdo. Cores trocadas ou imagem espelhada: ajuste `LCD_I80_MADCTL` /
  `LCD_I80_INVERT` no `menuconfig`.
- Menu: CIMA/BAIXO escolhe, A ou START joga. Tetris: SELECT volta ao menu. Doom: START+SELECT
  por 1 s sai; o chip reinicia e o menu volta (o motor não é reentrante).

## Primeira execução do Doom: o que olhar no log

- `w_file_esp: WAD mapeado da flash: N lumps` → partição e WAD ok. Se aparecer "não contém um
  WAD", o `write_flash` não foi feito (ou o offset difere do `partitions.csv`).
- `zone memory: ..., 600000 allocated` → 6 MB da PSRAM. Se der erro de memória, reduza
  `Doom → Memória do Doom` para 5 ou 4.
- Tela em paisagem; imagem de cabeça para baixo ou espelhada → `LCD_I80_MADCTL_LANDSCAPE`
  (0x28 → 0xE8, 0x68 ou 0xA8).

## Riscos conhecidos da primeira compilação do Doom (nada disto foi verificado)

- **DRAM insuficiente no link** (`region dram0_0_seg overflowed`): as tabelas estáticas do Doom
  (`.bss`) são grandes. Veja `idf.py size-components`; saída usual: habilitar
  `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY` e marcar os maiores arrays com `EXT_RAM_BSS_ATTR`.
- **Símbolos de libc ausentes** (`system`, `mkdir` em `i_system.c`/`m_misc.c`): são rotas de
  erro/config do Doom; um stub de duas linhas resolve.
- **Nomes de `CONFIG_*` do `sdkconfig.defaults`** (cache de dados 64 KB / linha 64 B) podem ter
  mudado no IDF 6: confira no `menuconfig`; nome errado é ignorado em silêncio. O
  `sdkconfig.defaults` só vale enquanto não existe um `sdkconfig` gerado (apague-o para reaplicar).
- **Desempenho:** sem medida. Em Options do Doom há "Low detail". Barramento: ~150 KB/quadro →
  teto de ~65 quadros/s a 10 MHz.

## Testes e simulador no PC (sem hardware)

Requer gcc (MSYS2 UCRT64, WSL ou Linux).

```bash
cd firmware/host_test && make                       # entrada, regras do Tetris, launcher e fluxo do jogo
make SAN="-fsanitize=address,undefined"             # só Linux/WSL: acha estouro de índice e UB

cd firmware/sim && make                             # simulador com janela (precisa do SDL2)
./build/portatil_sim.exe --wad ../../wad/doom1.wad  # menu + Tetris + Doom, como na placa
make test-headless WAD=../../wad/doom1.wad       # sem janela: roda Tetris, launcher e Doom e confere
```

Teclado, roteiros e capturas: `sim/README.md`. As capturas do `host_test` saem em
`host_test/shots/` (`exemplo_telas.png` mostra o resultado).

## Estado da verificação (2026-11-12)

| O quê | Nível (ver `docs/tasks.md`) |
|---|---|
| Regras, fluxo e desenho do Tetris; entrada; launcher | PC |
| Doom e launcher → Doom, com WAD real | PC (simulador) |
| `lcd_i80`, `input_gpio`, `game_api`, Tetris | compila (ESP-IDF 6.0.3, 17/10; recompilar) |
| `game_registry`, `launcher`, `doom_game`, `main/` | **nunca compilados** |
| Qualquer coisa na placa, além do `hello_world` | **nada** |
