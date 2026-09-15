# ADR-0006: Arquitetura de dois chips ESP32-S3 + ESP32-C3 (substitui a Raspberry Pi)

## Status
Aceita — substitui ADR-0001.

## Contexto
A Raspberry Pi Zero 2 W, prevista originalmente pra rodar o motor do jogo,
está sem estoque nos principais revendedores nacionais, com preço e prazo
de entrega incertos — o maior risco identificado na Fase 0. Buscou-se uma
alternativa que eliminasse essa dependência sem abrir mão de dois
objetivos do projeto: (a) demonstrar comunicação entre dispositivos
embarcados, e (b) manter uma rotina de desligamento seguro que não
dependa do chip que roda o jogo continuar responsivo.

## Alternativas consideradas

**SBCs alternativos ao Pi Zero 2W** (Radxa Zero 3W, Pi 3A+, Orange Pi
Zero 2) — mantêm a arquitetura original (Linux + MCU separado), mas ainda
dependem de um SBC rodando Linux, com os mesmos riscos de disponibilidade
em menor grau, e menos suporte de driver testado pra tela SPI do que a
Raspberry Pi oficial.

**ESP32-S3 N16R8 sozinho**, fazendo tudo (jogo + I/O + energia) — elimina
o Pi, mas também elimina a comunicação entre dispositivos (objetivo
original do projeto) e a garantia de segurança de um chip dedicado ao
desligamento: se a task do jogo travar, a task de energia depende do
mesmo FreeRTOS/CPU pra continuar sendo escalonada.

**ESP32-S3 N16R8 (jogo) + ESP32-C3 (I/O)**, comunicando-se por UART —
elimina o Pi, mantém dois dispositivos físicos distintos conversando por
protocolo serial, e mantém o ESP32-C3 como um chip independente capaz de
forçar o corte de energia mesmo que o ESP32-S3 trave.

Existe precedente real de Doom — inclusive o doomgeneric, o mesmo source
port já escolhido na ADR-0002 — rodando em ESP32-S3 com tela SPI
ILI9341, e a própria Espressif publica um port de referência do PrBoom
pra ESP32 com PSRAM.

## Decisão
Dois chips:
- **ESP32-S3 N16R8** assume o papel que era da Raspberry Pi: executa o
  doomgeneric e, futuramente, outros jogos (ver ADR-0005); desenha na
  tela SPI; áudio e armazenamento em SD entram como extensões.
- **ESP32-C3** assume o papel que já estava previsto pro "ESP32" desde o
  início do projeto (botões, leitura de bateria, lógica de desligamento
  seguro), agora nomeado especificamente.

## Consequências
- RF01 muda de "Pi via framebuffer/DRM, sem desktop" para "ESP32-S3 via
  FreeRTOS/bare-metal, desenho direto na tela SPI".
- ADR-0001 fica superada — a escolha de usar um ESP32 pro papel de I/O se
  mantém, só passa a nomear o ESP32-C3 especificamente.
- O protocolo serial (ADR-0003) e a decisão de tela (ADR-0004) permanecem
  válidos sem alteração de fundo — só troca terminologia ("Pi" vira
  "ESP32-S3").
- Todo o código Python que dependia de Linux (`serial_daemon.py`, uso de
  `uinput`) fica obsoleto como peça de produção — a lógica equivalente
  (interpretar o protocolo e traduzir em ação de jogo) passa a ser
  reimplementada como firmware C dentro de `firmware-s3/`. `protocol.py`
  e seus testes continuam válidos como referência de design.
- Elimina o item de maior risco de custo/disponibilidade do projeto (a
  Raspberry Pi) por um componente historicamente mais fácil de achar e
  mais barato.
- Elimina a necessidade de imagem Linux, cartão SD pra sistema
  operacional (o SD, se usado, passa a ser só armazenamento de dados/
  ROMs) e todo o processo de "instalar sistema mínimo sem desktop" da
  Fase 1 original.
- Estrutura de pastas do repositório muda de `pi-doom/` +
  `mcu-firmware/` para `firmware-s3/` + `firmware-c3/` (ver README).