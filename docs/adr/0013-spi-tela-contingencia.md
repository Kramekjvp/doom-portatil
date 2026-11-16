# ADR-0013: Tela SPI como contingência

## Status
Proposta — 2026-11-14. Depende da compra de um módulo SPI e de sua chegada antes de 18/11.

## Contexto
A ADR-0008 escolheu o shield TFT 2,4" paralelo (i80). Na bancada de 14/11 (`docs/bancada.md`)
a fiação, a alimentação e os sinais chegaram ao shield, mas o **painel não responde** a
escritas nem a leituras de ID. A apresentação é em 18/11/2026.

## Decisão (proposta)
Acrescentar um segundo backend de tela, `lcd_spi`, para um módulo **ILI9341 SPI de 2,8"
(240×320)** com 14 pinos (VCC, GND, CS, RESET, DC, SDI, SCK, LED, SDO, T_*). Ele implementa o
mesmo contrato de `gfx.h` que o `lcd_i80`, reaproveita a sequência de inicialização do
ILI9341 e é escolhido no `menuconfig`. Doom, Tetris e launcher não mudam. O `lcd_i80` e a
ADR-0008 permanecem, para o caso de um shield novo funcionar.

Ligação proposta (pinos nativos do SPI2 do ESP32-S3): CS = GPIO 10, MOSI (SDI) = 11,
SCK = 12, DC = 9, RESET = 14, LED → 3V3, VCC → 5V da placa (J1 aberto), GND → GND.

## Consequências
- Só 5 GPIOs de tela em vez de 13; sobram 8 para botões e outros usos.
- Teto de ~30 quadros/s a 40 MHz (~154 KB por quadro); 80 MHz dobra o teto se o módulo aguentar.
- O Doom roda, com menos folga que no barramento paralelo.
- BOM: acrescenta o módulo SPI; o shield paralelo vira sobra.
- Reduz a pressão sobre o MCP23017 (ADR-0012): com mais GPIOs livres, os botões em GPIO direto
  podem bastar no MVP. A decidir.

## Alternativas consideradas
- **Trocar ou devolver o shield paralelo:** mantém o projeto, mas depende da loja, do prazo e de
  nada garantir que o novo funcione.
- **Apresentar só com o simulador:** funciona hoje (Doom, Tetris e launcher no PC), mas a placa
  não mostraria imagem. Fica como plano de reserva em qualquer caso.

## Pendências
- Confirmar no anúncio: ILI9341, SPI, pinos soldados e entrega até segunda-feira (16/11).
- Escrever e testar o `lcd_spi` (no PC primeiro, depois na placa).
- Conferir o cabo flexível do shield paralelo (conferência visual ainda não feita).