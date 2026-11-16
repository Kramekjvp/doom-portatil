# Doom Portátil

Console portátil que roda **Doom** e **Tetris** num ESP32-S3 (Game Boy como extensão
condicional) — projeto da disciplina de Sistemas Embarcados. Apresentação: **18/11/2026**.

## O projeto em 30 segundos

| | |
|---|---|
| Placa | ESP32-S3 N16R8 (16 MB de flash, 8 MB de PSRAM octal) — único microcontrolador ([ADR-0012](docs/adr/0012-esp32-s3-unico.md)) |
| Tela | TFT 2,4" ILI9341, 240×320, paralela de 8 bits (i80) — [ADR-0008](docs/adr/0008-tela-ili9341-paralela-i80.md) |
| Botões | expansor MCP23017 (I²C); em GPIO direto nos testes de bancada |
| Software | ESP-IDF 6.0.3 + FreeRTOS; Doom via doomgeneric |
| Ambiente | Windows, Git/GitHub (`docs/dev-environment.md`) |

## Onde estamos (2026-11-14)

Detalhe e níveis de prova: [`docs/tasks.md`](docs/tasks.md).

- **No PC (simulador e testes):** Tetris, launcher (menu) e Doom funcionam.
- **Na placa:** o firmware único **sem o Doom** compila, grava e roda até o launcher (log serial).
- **Tela:** o shield paralelo não mostra imagem: fiação, alimentação e sinais chegam a ele, mas o
  painel não responde. Diagnóstico completo em [`docs/bancada.md`](docs/bancada.md); contingência
  com módulo SPI na [ADR-0013](docs/adr/0013-tela-spi-de-contingencia.md).
- **Doom na placa:** o link estoura a DRAM em 31.728 bytes (a corrigir).
- **GPIO 16 da placa não sobe:** LCD_D5 usa o GPIO 12 (`hardware/pinos.md`).
- **Sem decisão:** bateria (RF04) e desligamento seguro (RF05); botões por MCP23017 ou GPIO direto; microSD.

## Mapa do repositório

```
doom-portatil/
├── firmware/            projeto ESP-IDF do ESP32-S3 (esta pasta É o projeto)
│   ├── main/            laço de quadros da placa
│   ├── components/      camadas: game_api, game_registry, launcher, tetris_*, doom_game, lcd_i80, input_gpio
│   ├── sim/             simulador no PC (Doom e Tetris sem hardware)
│   └── host_test/       testes no PC
├── third_party/         doomgeneric (submódulo, GPL)
├── tools/               preparar_wad.sh
├── hardware/            bom.md (lista de materiais), pinos.md (pinos e pendências)
├── docs/                requisitos, tarefas, ambiente, histórico e adr/ (decisões)
├── wad/                 doom1.wad local (não vai para o git)
├── CLAUDE.md            guia curto para continuar o projeto (humano ou IA)
└── README.md
```

## Comece por aqui

```bash
git clone --recurse-submodules <url-do-repositório>
cd doom-portatil/firmware/host_test && make       # testes no PC (gcc: MSYS2, WSL ou Linux)
```

Build e gravação na placa, simulador, pinos e riscos da primeira compilação:
[`firmware/README.md`](firmware/README.md).

## Documentação

- [`docs/tasks.md`](docs/tasks.md) — quadro de tarefas (**fonte de verdade** do que está feito)
- [`docs/requirements.md`](docs/requirements.md) — requisitos formais
- [`docs/bancada.md`](docs/bancada.md) — diário de bancada: o diagnóstico da tela, passo a passo
- [`docs/adr/`](docs/adr/README.md) — decisões de arquitetura (índice e situação de cada uma)
- [`docs/historico.md`](docs/historico.md) — as arquiteturas anteriores e o mapa de caminhos antigos → novos
- [`docs/dev-environment.md`](docs/dev-environment.md) — ambiente de desenvolvimento

## Créditos e licenças

- `firmware/components/tetris_core`: regras do Tetris, **autoria da dupla do projeto**.
- `third_party/doomgeneric`: [ozkl/doomgeneric](https://github.com/ozkl/doomgeneric), GPL-2.0
  (código do Doom da id Software).
- Game Boy (se entrar): Peanut-GB e o fork Walnut-CGB — manter a cadeia de autoria na documentação.
- WAD do Doom e ROMs são dados de jogo: **não** são versionados. ROMs só de cartucho próprio.