# ADR-0008: Tela ILI9341 em paralelo de 8 bits (i80) no ESP32-S3

## Status
Proposta — aguardando validação na placa (ver "Pendências de validação").

Data: 2026-10-16 (a decisão de seguir com a tela paralela foi tomada em
2026-10-15). Substitui a ADR-0004 quanto à interface da tela. Relacionada
à ADR-0009 (camada de jogos).

## Contexto
O BOM do projeto previa um módulo TFT 2,4" ILI9341 **SPI** (240×320). A tela
comprada e em mãos é um shield "2.4" TFT LCD SHIELD" com interface **paralela de 8
bits** (LCD_D0–D7, LCD_WR, LCD_RD, LCD_RS, LCD_CS, LCD_RST), touch resistivo e slot
microSD, com dois buffers tipo 245 e um regulador. O firmware usa ESP-IDF v6.0.3 com
FreeRTOS no ESP32-S3 N16R8.

O ESP32-S3 tem o periférico LCD_CAM, que gera o barramento Intel 8080 (i80) por DMA,
exposto no ESP-IDF pelo componente `esp_lcd`.

## Alternativas consideradas
| Alternativa | Por que não |
|---|---|
| Comprar o módulo SPI do BOM | Custo e prazo extras (importação/revenda no Brasil); a tela paralela já está em mãos e é, em tese, mais rápida. |
| Manter Arduino + TFT_eSPI (que lista 8-bit paralelo para o S3) | O projeto é ESP-IDF 6.0.3 + FreeRTOS. O Arduino-ESP32 é acoplado a versões específicas do IDF e não foi confirmada compatibilidade com o 6.0.x. |
| Componente gerenciado `esp_lcd_ili9341` | Dependência extra e baixada em tempo de build; a sequência de init é curta e vale mais documentada no próprio repositório. Não foi avaliado a fundo. |
| SPI com o shield | Não existe: o shield expõe só o barramento paralelo. |

## Decisão
Usar a tela em paralelo, sem trocar pelo módulo SPI:

1. O barramento é gerado pelo LCD_CAM via `esp_lcd` (`esp_lcd_new_i80_bus` e
   `esp_lcd_new_panel_io_i80`), com DMA e troca de bytes RGB565 feita em hardware
   (`swap_color_bytes`).
2. O driver é próprio (componente `lcd_i80`) e usa só o "panel IO" do `esp_lcd`. A
   sequência de inicialização do ILI9341 fica escrita no código (a mesma do
   TFT_eSPI, `ILI9341_Init.h`), em vez de depender de um driver de painel pronto.
3. Todo desenho passa por um buffer DMA interno de 4 KB, enviado em faixas de linhas
   inteiras, cada faixa com sua própria janela (CASET/PASET/RAMWR).
4. Pinos, clock de escrita, orientação (MADCTL) e inversão de cor são configuráveis por
   `menuconfig` (Kconfig do componente). A ordem D0–D7 é livre, pois o LCD_CAM roteia
   qualquer GPIO.
5. RD fica em nível alto (só escrevemos na tela).

## Consequências
**Positivas**

- Vazão maior que SPI. O teto teórico em 10 MHz de escrita é ~10 MB/s (SPI a 40 MHz:
  ~5 MB/s). Um quadro cheio (240×320×2 = 153.600 B) leva ~15 ms em 10 MHz e ~10 ms
  em 15 MHz, antes do overhead de faixas.
- DMA libera a CPU durante a transferência.
- A sequência de init fica documentável e ajustável.

**Negativas / custos**

- Usa 13 GPIOs (8 dados, WR, RS, CS, RST, RD) contra 4–5 do SPI. Estimativa de
  orçamento (a confirmar no mapa de pinos): ~25 GPIOs livres no S3 N16R8 depois de
  excluir strapping, USB, flash/PSRAM octal e UART0; tela 13 + microSD 4 + I²C do
  MCP23017 (2–3) + áudio I2S (3) ≈ 22–23. Funciona, mas sem folga grande.
- Fiação com jumpers em 10 MHz exige cuidado de integridade de sinal.
- O driver de tela é código nosso a manter.

## Pendências de validação (não verificado ainda)
- [ ] **Alimentação e nível lógico dos buffers 245 do shield.** Em diagnóstico: o pino 20
      dos buffers mediu 0 V alimentado pelo pino 3V3. Se os buffers forem alimentados
      em 5 V, sinais de 3,3 V podem não ser lidos de forma confiável.
- [ ] Compilar `lcd_i80` no ESP-IDF 6.0.3 (a API foi conferida contra os headers v6.0.3,
      mas o build ainda não foi feito).
- [ ] Barras de cor do exemplo `tetris_standalone` corretas (ordem BGR, orientação).
- [ ] Clock máximo estável com a fiação real (começar em 10 MHz, tentar 15 MHz).
- [ ] Conflitos de pino com o microSD do shield (SD_SS/DI/DO/SCK) e com I²C/I2S futuros.
- [ ] Taxa de quadros medida na placa (relevante para o Doom).

## Referências
- ESP-IDF v6.0.3: `components/esp_lcd/include/esp_lcd_io_i80.h`, `esp_lcd_panel_io.h`.
- TFT_eSPI: `TFT_Drivers/ILI9341_Init.h` (sequência de init).
- Código: `firmware-s3/components/lcd_i80/` (Kconfig, `lcd_i80.c`).
