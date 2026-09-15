# Doom Portátil

Console portátil rodando Doom — e, como extensão futura, jogos de Game
Boy e Tetris — projeto da disciplina de Sistemas Embarcados.

Um **ESP32-S3 N16R8** executa os jogos (motor gráfico, launcher,
storage e áudio, quando entrarem em escopo). Um **ESP32-C3**, dedicado,
cuida de entrada de botões, leitura de bateria e desligamento seguro. Os
dois se comunicam por um protocolo serial simples.

> Arquitetura anterior (Raspberry Pi Zero 2 W + ESP32 genérico para I/O)
> substituída — ver `docs/adr/0006-arquitetura-dois-chips-esp32.md`.

## Estrutura do repositório (proposta)

```
doom-portatil/
├── firmware-s3/     # ESP32-S3: launcher, doomgeneric, jogos futuros, gráfico, áudio
├── firmware-c3/      # ESP32-C3: botões, bateria, desligamento seguro
├── tools/             # protocolo de referência em Python (design/teste, não roda em produção)
│   ├── protocol.py
│   └── tests/
├── docs/              # requisitos, ADRs, diagramas
│   └── adr/
└── hardware/          # BOM, esquemáticos
```

`pi-doom/` e `mcu-firmware/` (nomes antigos) saem de uso — ver ADR-0006
pra migrar o que já existia.

## Decisões já tomadas

- **Arquitetura:** dois chips, ESP32-S3 (jogos) + ESP32-C3 (I/O) —
  `docs/adr/0006-arquitetura-dois-chips-esp32.md` (substitui `0001`)
- **Source port do Doom:** doomgeneric, agora bare-metal —
  `docs/adr/0002-source-port-doomgeneric.md`
- **Protocolo serial ESP32-C3 ↔ ESP32-S3:**
  `docs/adr/0003-protocolo-serial.md`
- **Tela:** SPI, ILI9341/ST7789 — `docs/adr/0004-tela-spi.md`
- **Extensão pós-Doom (Game Boy + Tetris):**
  `docs/adr/0005-multiplos-jogos.md`

## Documentação

- `docs/requirements.md` — requisitos formais
- `docs/adr/` — decisões de arquitetura
- `docs/architecture.svg` — diagrama de blocos
- `docs/dev-environment.md` — ambiente de desenvolvimento
- `docs/tasks.md` — quadro de tarefas por fase

## Status

Pivô de arquitetura em andamento: saindo de Raspberry Pi + ESP32 (I/O)
para ESP32-S3 + ESP32-C3 (dois microcontroladores, sem Linux). O
protocolo serial e sua lógica de parsing/checksum permanecem válidos
como referência de design; a implementação em Python específica de
Linux (`uinput`, daemon) fica obsoleta como código de produção e será
reescrita como firmware C dentro de `firmware-s3/` e `firmware-c3/`.