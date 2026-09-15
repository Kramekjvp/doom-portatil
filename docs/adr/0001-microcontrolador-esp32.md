# ADR-0001: Uso do ESP32 como microcontrolador

## Status
Superada pela ADR-0006. A escolha de usar um ESP32 para o papel de I/O
se mantém — a ADR-0006 apenas passa a nomear o chip especificamente
(ESP32-C3) e adiciona um segundo ESP32 (S3) para o papel que antes era
da Raspberry Pi.

## Contexto
Precisava de um MCU para leitura de botões, ADC de bateria, lógica de
shutdown e comunicação serial com a Raspberry Pi (arquitetura original).
Alternativas consideradas: ESP32 ou STM32.

## Alternativas consideradas
STM32 (mais bare-metal, mais alinhado à disciplina, porém prototipagem
mais lenta) vs. ESP32 (bibliotecas prontas para UART/ADC, prototipagem
mais rápida).

## Decisão
ESP32, priorizando velocidade de prototipagem dado o cronograma do
projeto.

## Consequências
- Firmware em C/C++ via PlatformIO ou ESP-IDF.
- Essa decisão permanece válida mesmo após a ADR-0006 — o que muda é a
  arquitetura ao redor dela, não a escolha do chip em si para o papel de
  I/O.