# ADR 0003 — Protocolo serial MCU → Pi

## Status
Aceita

## Contexto
MCU (`mcu-firmware/`) e Pi (`pi-doom/`) são desenvolvidos e testados de
forma independente, inclusive antes de existir hardware físico (via porta
serial virtual simulada com socat). Era necessário um protocolo simples de
definir, fácil de depurar num terminal serial cru, e simples de decodificar
no daemon que injeta eventos via uinput.

## Decisão
Mensagens de 3 bytes: 1 byte de tipo de evento + 1 byte de payload + 1 byte
de checksum.

| Byte 1 (tipo)      | Byte 2 (payload)         | Byte 3        |
|---|---|---|
| `0x01` — botão      | código do botão pressionado/solto | checksum |
| `0x02` — power-off solicitado | não usado (`0x00`) | checksum |
| `0x03` — ack de shutdown seguro | não usado (`0x00`) | checksum |

## Consequências
- Protocolo trivial de implementar dos dois lados e de inspecionar
  manualmente durante testes.
- Sem correção de erro além do checksum simples — aceitável dado que a
  distância física e o baud rate envolvidos são pequenos, dentro do
  escopo do MVP.
- Extensível: novos tipos de evento (ex: nível de bateria) podem ser
  adicionados sem quebrar o formato existente.
