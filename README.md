# Doom Portátil

Console portátil rodando Doom, projeto da disciplina de Sistemas Embarcados.

A Raspberry Pi executa o motor do jogo (vídeo e áudio). Um microcontrolador
cuida de entrada de botões, gerenciamento de bateria e desligamento seguro.
Os dois se comunicam por um protocolo serial simples.

O objetivo não é só "Doom rodando numa Pi" — o projeto existe para demonstrar
conceitos reais de sistemas embarcados: comunicação entre dispositivos,
tempo real em I/O e gerenciamento de energia.

## Estrutura do repositório

```
doom-portatil/
├── pi-doom/        # source port do Doom + daemon que lê a serial e injeta eventos
├── mcu-firmware/   # firmware do microcontrolador (botões, bateria, shutdown)
├── docs/           # requisitos, ADRs (decisões de arquitetura) e diagramas
│   └── adr/        # registro das decisões de arquitetura, uma por arquivo
└── hardware/       # BOM, esquemáticos e arquivos de case (quando existirem)
```

## Decisões já tomadas

- **Microcontrolador:** ESP32 (`docs/adr/0001-microcontrolador-esp32.md`)
- **Source port do Doom:** doomgeneric (`docs/adr/0002-source-port-doomgeneric.md`)
- **Protocolo serial MCU → Pi:** `docs/adr/0003-protocolo-serial.md`

## Decisões em aberto

- Tela: SPI vs. HDMI pequena

## Documentação

- `docs/requirements.md` — requisitos formais do MVP
- `docs/adr/` — decisões de arquitetura
- `docs/architecture.svg` — diagrama de arquitetura preliminar
- `docs/dev-environment.md` — ambiente de desenvolvimento
- `docs/tasks.md` — quadro de tarefas por fase

## Status

Projeto em planejamento formalizado. Ainda não há hardware físico — o
desenvolvimento começou pelo código (protocolo serial, backend de
renderização, firmware do ESP32 isolado) para reduzir o risco de atraso na
entrega de componentes.
