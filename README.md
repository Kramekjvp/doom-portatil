# Doom Portátil

Console portátil que roda Doom — e, como extensão, Game Boy e Tetris —
projeto da disciplina de Sistemas Embarcados.

## O projeto em 30 segundos

| | |
|---|---|
| Placa | ESP32-S3 N16R8 (16 MB de flash, 8 MB de PSRAM octal) |
| Tela | TFT 2,4" ILI9341, 240×320, **paralela de 8 bits (i80)** — `docs/adr/0008-tela-ili9341-paralela-i80.md` |
| Botões | previstos via expansor MCP23017 (I²C); nos testes de bancada, GPIO direto |
| Software | ESP-IDF 6.0.3 + FreeRTOS; Doom via doomgeneric |
| Ambiente | Windows (PC de mesa), Git/GitHub |

## Onde estamos

- **Feito:** requisitos, ADRs, BOM; ESP-IDF instalado e `hello_world` rodando na
  placa; Tetris (núcleo da dupla) portado para as camadas do projeto e testado no PC.
- **Próximo:** validar tela e botões na placa com `tetris_standalone`; depois
  launcher e Doom.
- Detalhe fase a fase: `docs/tasks.md`.

## Atenção: alguns documentos ainda não acompanham as decisões mais novas

O ESP32-S3 passou a assumir todas as funções (launcher, controles, tela, jogos,
áudio e microSD) e o ESP32-C3 saiu do BOM revisado. Mas `docs/adr/0003` e `0006`,
`docs/requirements.md`, `docs/tasks.md`, `docs/architecture.svg`, `BOM.md` e
`CLAUDE.md` ainda descrevem o modelo de **dois chips**. Enquanto não houver um ADR
registrando essa mudança, trate o modelo de dois chips como **histórico**.

## Decisões (ADRs em `docs/adr/`)

| ADR | Decisão | Situação |
|---|---|---|
| [0001](docs/adr/0001-microcontrolador-esp32.md) | Usar ESP32 como microcontrolador | Superada pela 0006 |
| [0002](docs/adr/0002-source-port-doomgeneric.md) | doomgeneric como source port do Doom | Aceita |
| [0003](docs/adr/0003-protocolo-serial.md) | Protocolo serial C3 → S3 | Aceita (ver aviso acima) |
| [0004](docs/adr/0004-tela-spi.md) | Tela SPI | Superada pela 0008 (interface) |
| [0005](docs/adr/0005-multiplos-jogos.md) | Múltiplos jogos: launcher + input antes, conteúdo depois | Proposta |
| [0006](docs/adr/0006-arquitetura-dois-chips-esp32.md) | Dois chips (S3 + C3), sem Raspberry Pi | Aceita (ver aviso acima) |
| [0008](docs/adr/0008-tela-ili9341-paralela-i80.md) | Tela ILI9341 paralela (i80) | Proposta |
| [0009](docs/adr/0009-camada-de-jogos-e-reuso-do-tetris.md) | Camada de jogos e reuso do Tetris da dupla | Proposta |

Não há ADR 0007 neste repositório (a numeração pula do 0006 para o 0008).

## Mapa do repositório

```
doom-portatil/
├── firmware-s3/                 ATIVO — projeto ESP-IDF do ESP32-S3
│   ├── components/              as camadas do firmware (uma pasta por componente)
│   │   ├── game_api/            contratos: jogo, botões, tela (sem hardware)
│   │   ├── tetris_core/         regras do Tetris (autoria: dupla do projeto)
│   │   ├── tetris_game/         telas e fluxo do Tetris
│   │   ├── lcd_i80/             driver da tela (paralela, DMA)
│   │   └── input_gpio/          botões em GPIO direto (bancada)
│   ├── examples/tetris_standalone/   primeiro teste na placa
│   └── host_test/               testes no PC, sem hardware
├── docs/                        ATIVO — requisitos, ADRs, diagrama, tarefas
├── BOM.md, hardware/            lista de materiais e esquemas
├── pi-doom/                     LEGADO (Raspberry Pi). Guarda `protocol.py`
│                                (referência do protocolo) e o submódulo doomgeneric
└── mcu-firmware/                LEGADO (vazio)
```

- O submódulo **doomgeneric** ainda está em `pi-doom/doomgeneric`; vai para o
  `firmware-s3` quando a Fase 1 (Doom no S3) começar.
- `firmware-c3/` aparece no ADR-0006, mas não foi criado e só será usado se o
  ESP32-C3 voltar ao projeto (ver aviso acima).

## Comandos

```bash
# Testes do Tetris no PC (precisa de gcc: WSL ou MSYS2)
cd firmware-s3/host_test && make

# Testes do protocolo serial (referência em Python)
cd pi-doom/tests && python3 test_protocol.py
```

Build e gravação na placa (no terminal do ESP-IDF 6.0.3):

```
cd firmware-s3\examples\tetris_standalone
idf.py set-target esp32s3
idf.py menuconfig        # confira os pinos
idf.py build flash monitor
```

Mais detalhes (pinos, o que foi e não foi verificado): `firmware-s3/README.md`.

## Documentação

- `docs/README.md` — índice da pasta `docs/`
- `docs/requirements.md` — requisitos formais
- `docs/adr/` — decisões de arquitetura
- `docs/architecture.svg` — diagrama de blocos (ainda com dois chips)
- `docs/dev-environment.md` — ambiente de desenvolvimento
- `docs/tasks.md` — quadro de tarefas por fase
- `CLAUDE.md` — guia rápido para continuar o projeto (humano ou IA)
