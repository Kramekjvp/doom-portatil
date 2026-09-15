# Requisitos — Doom Portátil

Atualizados após a ADR-0006 (substituição da Raspberry Pi por um segundo
ESP32). Cada requisito mantém um critério de aceite verificável.

## Requisitos funcionais

**RF01 — Doom rodando no ESP32-S3, sem sistema operacional**
O ESP32-S3 deve executar o doomgeneric via FreeRTOS/bare-metal,
desenhando diretamente na tela SPI.
*Critério de aceite:* Doom inicia e é jogável na tela física, sem
depender de nenhum outro dispositivo além do ESP32-C3 para entrada.

**RF02 — Entrada via D-pad e botões de ação**
O sistema deve reconhecer eventos de D-pad e 2 botões de ação, lidos
pelo ESP32-C3 e enviados ao ESP32-S3. Start/Select reservados no
protocolo para quando o Game Boy (ADR-0005) entrar em desenvolvimento.
*Critério de aceite:* joga-se o Doom inteiramente pelos botões físicos.

**RF03 — Protocolo serial ESP32-C3 → ESP32-S3 com detecção de erro**
A comunicação entre os dois chips deve seguir o protocolo definido em
`docs/adr/0003-protocolo-serial.md`, descartando mensagens corrompidas.
*Critério de aceite:* ESP32-C3 envia eventos de botão por UART,
corretamente decodificados pelo ESP32-S3; mensagens com checksum
inválido são descartadas sem travar.

**RF04 — Alimentação por bateria**
Sistema alimentado por bateria, com autonomia mínima testada, leitura de
tensão feita pelo ESP32-C3 via ADC.
*Critério de aceite:* autonomia medida em teste de uso contínuo (valor
mínimo a definir após a escolha da bateria).

**RF05 — Desligamento seguro**
O ESP32-C3 detecta o botão de power e coordena o desligamento com o
ESP32-S3, sem depender do ESP32-S3 estar responsivo para garantir o
corte físico de energia.
*Critério de aceite:* 10 ciclos consecutivos de liga/desliga sem
corrupção de dados salvos (quando houver SD).

## Requisitos não funcionais

**RNF01 — Plataforma**
ESP32-S3 N16R8 (jogos, gráfico, áudio e storage futuros) + ESP32-C3
(I/O), via ESP-IDF/FreeRTOS.

**RNF02 — Portabilidade do source port**
doomgeneric adaptado para um backend de framebuffer bare-metal,
escrevendo direto na tela SPI (ver ADR-0002 e ADR-0004).

**RNF03 — Depurabilidade do protocolo**
Protocolo simples o bastante para inspeção manual via terminal serial
durante o desenvolvimento.

## Fora do escopo do MVP (extensão condicional pós-Doom — ver ADR-0005)

- Estrutura de launcher e abstração de input (entra cedo, junto do Doom,
  como base estrutural — o conteúdo dos jogos abaixo é que é condicional)
- Emulação de Game Boy (Peanut-GB) e, como estágio seguinte, Game Boy
  Color (Walnut-CGB)
- Tetris (implementação própria, sem ROM comercial)
- Áudio (I²S + amplificador tipo MAX98357A)
- Armazenamento em cartão microSD (ROMs, saves, configuração)