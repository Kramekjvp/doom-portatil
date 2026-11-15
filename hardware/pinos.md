# Pinos e pendências de hardware

Placa: ESP32-S3 N16R8 DevKitC. Tela: shield TFT 2,4" ILI9341, 240×320, **paralelo de
8 bits (i80)**, com touch resistivo e slot microSD (ADR-0008). Os pinos abaixo são os
**padrões do código** e podem ser mudados em `idf.py menuconfig` (menus "LCD i80" e
"Entrada: botões em GPIO direto").

Evite os pinos 0, 3, 19, 20 (strapping/USB), 26–37 (flash/PSRAM octal), 43–46; os 38 e 48
podem ser o LED RGB, conforme a placa.

## Tela (`lcd_i80`)

| Sinal | GPIO | Sinal | GPIO |
|---|---|---|---|
| LCD_D0 | 4 | LCD_WR | 8 |
| LCD_D1 | 5 | LCD_RS (DC) | 9 |
| LCD_D2 | 6 | LCD_CS | 10 |
| LCD_D3 | 7 | LCD_RST | 14 |
| LCD_D4 | 15 | LCD_RD | 21 (fica em nível alto; `-1` = ligar em 3V3) |
| LCD_D5 | **12** (o padrão do código ainda é 16) | Clock | 10 MHz |
| LCD_D6 | 17 | MADCTL retrato / paisagem | 0x48 / 0x28 |
| LCD_D7 | 18 | | |

A ordem D0..D7 é livre (o LCD_CAM roteia qualquer GPIO), mas cada Dn do chip tem de ir ao
Dn do shield.

**LCD_D5 no GPIO 12:** o GPIO 16 desta placa não sobe a nível alto (0 V na ponta do fio, mesmo
sem o shield). O D5 foi movido para o GPIO 12 na fiação e no programa `tools/lcd_teste_gpio`.
**Pendente:** `LCD_I80_PIN_D5` no `components/lcd_i80/Kconfig` ainda diz 16; ajuste no `menuconfig`
ou mude o padrão.

## Botões em GPIO direto (`input_gpio`, bancada)

| Botão | GPIO | Botão | GPIO |
|---|---|---|---|
| BAIXO | 1 | A (girar / atirar) | 41 |
| ESQUERDA | 2 | B (queda / usar) | 40 |
| DIREITA | 42 | START (pausa) | 39 |
| **CIMA** | **-1 (desligado)** | **SELECT** | **-1 (desligado)** |

CIMA e SELECT vêm **desligados** por padrão. No Doom, CIMA é andar para a frente; SELECT
sai do Tetris e faz strafe/troca de arma no Doom. Para testar na placa é preciso atribuir
um GPIO a eles no `menuconfig` (ou esperar o MCP23017).

## Shield paralelo: o que a bancada mostrou (2026-11-14, ver `docs/bancada.md`)

- Alimentação por **5V e GND** funciona (backlight acende, regulador U1 em 3,3 V). O pino **3.3V** do
  J4 é **entrada** e mede 0 V com o jumper J1 aberto: não ligue o 3V3 da placa nele; deixe o J1 aberto.
- **RD controla o sentido dos buffers de dados.** Com RD alto o ESP32 escreve; com RD baixo o buffer
  devolve dados e, se o ESP32 estiver dirigindo o mesmo fio, a tensão cai a ~1,5 V. Mantenha o RD
  **alto** em qualquer teste de escrita.
- Os 13 sinais chegam aos pinos do shield com o nível certo (D5 já no GPIO 12), mas **o painel não
  responde**: leitura de ID constante e tela sempre branca. O shield é o principal suspeito.
- Detalhes dos testes e do que não foi provado: `docs/bancada.md`.

## Alternativa: módulo SPI ILI9341 2,8" (ADR-0013, proposta; não comprado)

| Pino do módulo | Liga em | Observação |
|---|---|---|
| VCC | 5V da placa | J1 do módulo aberto (o regulador do módulo gera 3,3 V); confirmar no anúncio |
| GND | GND | |
| CS | GPIO 10 | pino nativo do SPI2 |
| SDI (MOSI) | GPIO 11 | pino nativo do SPI2 |
| SCK | GPIO 12 | pino nativo do SPI2 |
| DC | GPIO 9 | |
| RESET | GPIO 14 | |
| LED (backlight) | 3V3 | |
| SDO (MISO), T_* (touch), SD_* | não usados | |

Com a tela SPI, os GPIOs 4–8, 15–18 e 21 ficam livres (os botões em GPIO direto cabem sem o MCP23017).

## Pendências

- **5 V:** a placa do S3 entrega 4,68 V no pino de 5 V (medido em 14/11); suficiente para o shield.
- **Cabo flexível do shield paralelo:** conferência visual ainda não feita.
- **MCP23017:** pinos I²C (SDA, SCL) e endereço no S3 a definir (ou dispensar, ver `docs/tasks.md`).
- **microSD:** o slot do shield paralelo compartilha pinos com a tela; o módulo SPI traz o próprio
  slot. O cartão ainda não foi comprado.
- **Áudio (I²S):** costuma conflitar com pinos da tela em projetos parecidos; só entra por último.