# Decisões de arquitetura (ADRs)

Cada decisão de arquitetura vira uma ADR neste diretório. ADR substituída **não é
apagada**: fica marcada como "Superada" e vira histórico (útil para o relatório final).

| ADR | Decisão | Situação |
|---|---|---|
| [0001](0001-microcontrolador-esp32.md) | Usar ESP32 como microcontrolador | Superada (0006, depois 0012) |
| [0002](0002-source-port-doomgeneric.md) | doomgeneric como source port do Doom | Aceita |
| [0003](0003-protocolo-serial.md) | Protocolo serial C3 → S3 | Superada (0012) |
| [0004](0004-tela-spi.md) | Tela SPI | Superada (0008, só a interface) |
| [0005](0005-multiplos-jogos.md) | Múltiplos jogos: launcher + entrada antes, conteúdo depois | Proposta |
| [0006](0006-arquitetura-dois-chips-esp32.md) | Dois chips (S3 + C3), sem Raspberry Pi | Superada (0012, só o C3) |
| 0007 | — número não usado (a numeração pula de 0006 para 0008) | — |
| [0008](0008-tela-ili9341-paralela-i80.md) | Tela ILI9341 paralela de 8 bits (i80) | Proposta (na placa o painel do shield não respondeu: ver `../bancada.md`) |
| [0009](0009-camada-de-jogos-e-reuso-do-tetris.md) | Camada de jogos (contratos) e reuso do Tetris da dupla | Proposta |
| [0010](0010-doom-no-s3-e-simulador-no-pc.md) | Doom no S3 (doomgeneric) e simulador no PC | Proposta |
| [0011](0011-launcher-e-firmware-unico.md) | Launcher e firmware único | Proposta |
| [0012](0012-esp32-s3-unico.md) | ESP32-S3 único (sem ESP32-C3) | Aceita |
| [0013](0013-tela-spi-de-contingencia.md) | Tela SPI como contingência | Proposta (depende da compra) |

"Proposta" = decidida e implementada, mas ainda sem validação na placa.

Os caminhos citados nas ADRs 0005 a 0011 (`firmware-s3/`, `app/`, `examples/`) são do
layout anterior: veja o mapa antigo → novo em [`../historico.md`](../historico.md).

## Antes de escrever uma ADR nova

1. Use o próximo número (0014, ...) e o formato: Status, Contexto, Decisão,
   Consequências, Alternativas consideradas (e Pendências de validação, se houver).
2. Se ela substitui outra, marque a antiga como "Superada pela ADR-00XX" e atualize a
   tabela acima.
3. Atualize `docs/tasks.md` no mesmo commit.