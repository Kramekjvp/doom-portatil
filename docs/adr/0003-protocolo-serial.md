# ADR-0003: Protocolo serial ESP32-C3 → ESP32-S3

## Status
Aceita. Terminologia atualizada após a ADR-0006 — o "Pi" das versões
anteriores desta ADR é o ESP32-S3; o "MCU" é o ESP32-C3. O formato da
mensagem não muda.

## Contexto
O ESP32-C3 precisa informar o ESP32-S3 sobre eventos de botão
(pressionar/soltar) e sinal de power-off, de forma simples de depurar
manualmente com um terminal serial comum.

## Alternativas consideradas
Protocolo binário compacto (menor overhead, mais difícil de depurar sem
ferramentas) vs. protocolo texto tipo linha (fácil de ler no terminal,
mais overhead, parsing mais simples).

## Decisão
Protocolo binário simples de 3 bytes: `[cabeçalho][payload][checksum]`,
equilibrando simplicidade de parsing com possibilidade de depuração
manual (valores em hexadecimal ainda são legíveis).

- Byte 1 — tipo de evento: `0x01` botão, `0x02` power-off solicitado,
  `0x03` ack de shutdown seguro.
- Byte 2 — payload: pra botão, `(pressed << 7) | button_id`; pros
  demais tipos, `0x00`.
- Byte 3 — checksum: XOR dos dois bytes anteriores. Mensagens com
  checksum inválido são descartadas pelo receptor.

Botões: `0`=UP, `1`=DOWN, `2`=LEFT, `3`=RIGHT, `4`=AÇÃO A, `5`=AÇÃO B —
com `6`=START e `7`=SELECT reservados pra quando o Game Boy (ADR-0005)
entrar em desenvolvimento.

## Consequências
- Overhead pequeno (3 bytes por evento), adequado à baixa frequência de
  eventos de botão.
- Precisa de lógica de resync caso a UART perca sincronismo — o receptor
  deve procurar pelo byte de cabeçalho conhecido antes de interpretar o
  restante da mensagem.
- Lógica de parsing/checksum já validada em Python (`protocol.py`, 8/8
  testes) — serve como referência de design ao reimplementar em C nos
  dois firmwares (ver ADR-0006).