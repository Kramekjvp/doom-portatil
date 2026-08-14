# ADR 0001 — Microcontrolador: ESP32

## Status
Aceita

## Contexto
O microcontrolador do console precisa: ler botões (D-pad + 2 de ação) com
debounce confiável, ler a tensão da bateria via ADC, implementar a lógica
de shutdown seguro e se comunicar com a Raspberry Pi por UART. As opções
consideradas foram ESP32 e STM32.

## Alternativas consideradas
- **STM32** — mais próximo de "bare metal" (HAL direto ou registradores),
  mais alinhado ao espírito da disciplina de Sistemas Embarcados, mas com
  ciclo de prototipagem mais lento.
- **ESP32** — framework Arduino/ESP-IDF com bibliotecas prontas para UART,
  ADC e deep sleep, prototipagem mais rápida.

## Decisão
ESP32, usando o framework Arduino via PlatformIO, com possibilidade de
chamar APIs do ESP-IDF diretamente quando algo mais fino for necessário
(ex: configuração de deep sleep no shutdown).

## Consequências
- Ciclo de desenvolvimento mais rápido para o MVP, com menos tempo gasto em
  drivers de baixo nível.
- Wi-Fi/Bluetooth do chip não são usados no MVP, mas ficam disponíveis caso
  se queira adicionar telemetria ou debug remoto depois.
- Menos "bare metal" que a alternativa STM32 — se o critério de avaliação
  da disciplina valorizar isso, vale registrar essa escolha explicitamente
  no relatório final como uma troca consciente entre profundidade e prazo.
