# Diário de bancada — tela do console (2026-11-14)

Registro do diagnóstico da tela, feito na bancada com a placa ESP32-S3, o shield TFT 2,4"
(paralelo, 8 bits) e um multímetro. Serve de base para o relatório e evita refazer testes.

**Conclusão em uma frase:** a placa, o firmware, a alimentação do shield e a fiação estão
bons até os conectores do shield, mas **o painel do shield não responde** (nem a comandos de
escrita, nem a leituras de ID). O shield é o principal suspeito. Contingência: ADR-0013.

## Montagem

- Placa ESP32-S3 N16R8 (clone do DevKitC-1), pad "IN-OUT" soldado para entregar 5 V.
- Shield "2.4\" TFT LCD SHIELD" (ILI9341 segundo o anúncio), alimentado por **5V e GND** da placa.
  O pino **3.3V** do shield não é ligado: é entrada, e o jumper J1 fica aberto.
- Fiação conforme `hardware/pinos.md` (LCD_D5 no GPIO 12, ver abaixo).
- Firmware: `firmware/` (launcher, sem o Doom) e, depois, o programa `tools/lcd_teste_gpio`.

## Resultados, na ordem em que aconteceram

| # | Teste | Resultado | Leitura |
|---|---|---|---|
| 1 | Placa sozinha, USB | 5V = 4,68 V; 3V3 = 3,32 V | Solda do pino de 5 V conduz; regulação da placa ok |
| 2 | Shield só com 5V e GND | Backlight acende; nada esquenta; 5V > 4,5 V; aba do regulador U1 = 3,3 V; pino 3.3V do J4 = 0 V | Regulador do shield ok; o pino 3.3V é entrada (esperado 0 V) |
| 3 | Build do firmware | Erros de CMake e de memória (ver abaixo) | Corrigidos ou registrados |
| 4 | Log serial do firmware | PSRAM 8 MB ok; `lcd_i80: ... pronto`; `launcher iniciado`; sem reinício | O programa roda; **tela toda branca** |
| 5 | Teste por GPIO simples (sem LCD_CAM), cores | Tela continua branca | Não é o driver `lcd_i80` |
| 6 | Continuidade dos 13 fios | Todos apitaram | Jumpers ok (não prova ausência de curto na rede) |
| 7 | Tensões DC dos controles, firmware parado | RD 3,27 V; WR 2,6 V; RS 0 V; CS 3,0 V; RST 3,0 V | Inconclusivo (resistores do shield mantêm os pinos altos mesmo sem acionamento) |
| 8 | Caminhada de pinos, versão 1 (RD baixo) | D0..D7 só chegam a 1,5–2 V | **Artefato do teste**: com RD baixo o buffer inverte o sentido e briga com o ESP32 |
| 9 | Caminhada v2 (RD alto) | Todos ~3,3 V, exceto **LCD_D5 (GPIO 16): 0 V**, mesmo com o fio solto do shield | **GPIO 16 da placa não sobe**: defeito do pino ou da solda do pino |
| 10 | Passo 2 da v2: RD baixo + D0 alto | LCD_D0 = 1,5 V | Confirma: o sentido do buffer de dados segue o RD (o shield aceita leitura) |
| 11 | D4 e D5 do shield × GND, continuidade | ~500 nos dois | Sem curto específico no D5 do shield |
| 12 | LCD_D5 movido para o GPIO 12 | ~3,3 V no shield | Defeito do GPIO 16 contornado |
| 13 | Leitura do ID (comandos 0xD3, 0x04 e registrador de 16 bits) | `10 10 10 10`, depois `40 40 40 40`; mesmo valor em todas as leituras | O painel **não dirige o barramento**: não responde (um ILI9341 devolveria `00 00 93 41`) |
| 14 | Teste de cores por GPIO com D5 no GPIO 12 | Tela não muda | Painel não reage a escrita |

## Erros de build encontrados e como foram resolvidos

- `esp_timer.h: No such file` em `main.c`: no ESP-IDF 6 o `main` precisa declarar o que usa.
  Correção em `firmware/main/CMakeLists.txt`: `PRIV_REQUIRES esp_timer esp_system esp_hw_support log`.
- `-DPORTATIL_SEM_DOOM=1` **não funcionou**: o ESP-IDF lê os `REQUIRES` antes de ver as variáveis do
  CMake, e o Doom continuou no link. Correção em `components/game_registry/CMakeLists.txt`: a opção
  passou a ser a **variável de ambiente** `PORTATIL_SEM_DOOM=1`.
- Com o Doom no link: `region dram0_0_seg overflowed by 31728 bytes`. **Ainda não corrigido.**
  Caminho previsto: mover o `.bss` do Doom para a PSRAM (`firmware/README.md`).
- Aviso `Detected size(16384k) larger than the size in the binary image header(2048k)` no programa de
  teste: inofensivo (o teste usa a configuração padrão de 2 MB de flash).

## O que está provado

- Alimentação da placa e do shield (regulador U1 do shield em 3,3 V; os buffers são alimentados
  pela saída dele, segundo medida de continuidade feita antes).
- Os 13 sinais (D0..D7, WR, RS, CS, RST, RD) saem do ESP32-S3 com o nível correto até os pinos do
  shield, conferidos com multímetro depois do ajuste do D5.
- O buffer de dados do shield está vivo (muda de sentido com o RD).
- O firmware roda na placa até o launcher.

## O que não está provado

- Alimentação e reset **do painel**, depois dos buffers (não há ponto de medida acessível).
- Se o cabo flexível do painel está bem encaixado (conferência visual pendente).
- Se o controlador do painel é mesmo um ILI9341. Um controlador diferente ainda dirigiria o
  barramento numa leitura; como não dirige, o mais provável é módulo ou flexível com defeito.

## Como repetir

- `tools/lcd_teste_gpio/`: projeto ESP-IDF separado que mexe nos pinos com GPIO simples.
  `MODO_CAMINHADA 0` = confere o D5, lê o ID e desenha cores; `1` = um pino por vez para o multímetro.
  Os pinos estão fixos no código (D5 = GPIO 12): mude-os lá se mudar a fiação.
- Para o RD: nos testes de escrita ele fica **alto**; baixo só durante leituras.