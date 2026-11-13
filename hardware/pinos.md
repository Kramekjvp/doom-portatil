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
| LCD_D5 | 16 | Clock | 10 MHz |
| LCD_D6 | 17 | MADCTL retrato / paisagem | 0x48 / 0x28 |
| LCD_D7 | 18 | | |

A ordem D0..D7 é livre (o LCD_CAM roteia qualquer GPIO), mas cada Dn do chip tem de ir ao
Dn do shield.

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

## Pendências (nada abaixo foi verificado)

- **5 V:** a placa do S3 foi soldada para fornecer 5 V; nada foi testado depois.
- **Nível lógico:** o shield tem dois buffers tipo 245 e um regulador (ADR-0008), e os GPIOs
  do S3 são 3,3 V. Alimentação e nível lógico ainda em diagnóstico: confira antes de culpar o código.
- **MCP23017:** pinos I²C (SDA, SCL) e endereço no S3 a definir.
- **microSD:** o slot do shield compartilha pinos com a tela paralela; não foi checado. O
  cartão ainda não foi comprado.
- **Áudio (I²S):** costuma conflitar com pinos da tela em projetos parecidos; só entra por último.
