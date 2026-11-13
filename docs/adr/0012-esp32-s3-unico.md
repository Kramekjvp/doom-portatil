# ADR-0012: ESP32-S3 único (sem ESP32-C3)

## Status
Aceita — 2026-11-12. Substitui a ADR-0006 e deixa a ADR-0003 sem uso.

O texto abaixo resume o que o repositório sustenta; o grupo pode acrescentar aqui
outras razões da decisão, se houver.

## Contexto
A ADR-0006 dividia o console em dois chips: ESP32-S3 (jogos e tela) e ESP32-C3
(botões, bateria e desligamento seguro), ligados por UART (ADR-0003). Desde então:

- o S3 já executa o launcher, os jogos e o driver da tela (ADR-0008 a ADR-0011);
- os botões passam a ser lidos por um expansor MCP23017 (I²C) ligado ao próprio S3;
- a apresentação do trabalho é em 18/11/2026.

## Decisão
O **ESP32-S3 N16R8 é o único microcontrolador** do console. Cuida de jogos, tela,
botões (via MCP23017) e, se entrarem, áudio e microSD. O ESP32-C3 sai do projeto e do BOM.

## Consequências
- ADR-0003 (protocolo serial C3 → S3) fica **superada**. O `protocol.py`, o
  `serial_daemon.py` e os testes em Python saíram da árvore; continuam no histórico do
  git (tag `pre-reorganizacao`).
- RF03 (protocolo serial) foi removido de `docs/requirements.md`. RF04 (bateria) e RF05
  (desligamento seguro) dependiam do C3 e estão **sem decisão** (ver `docs/tasks.md`).
- A independência do desligamento seguro — funcionar mesmo com o jogo travado, argumento
  central da ADR-0006 — deixa de ser propriedade do projeto. Se o desligamento seguro
  voltar ao escopo, será pelo S3 ou por hardware dedicado (a decidir).
- A parte da ADR-0006 que descarta a Raspberry Pi continua valendo.
- Documentação alinhada na reorganização de 2026-11-12 (`docs/historico.md`).

## Alternativas consideradas
- **Manter S3 + C3 (ADR-0006):** mais hardware e mais firmware para validar em poucos
  dias, sem ganho para o que o MVP exige (Doom, launcher e Tetris na tela).
- **Voltar à Raspberry Pi:** descartada na ADR-0006 (sem estoque nos revendedores nacionais).

## Pendências
- Bateria (RF04) e desligamento seguro (RF05): entram no MVP? Como?
- Pinos do MCP23017 no S3 (SDA, SCL, endereço).
