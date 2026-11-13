# Requisitos — Doom Portátil

Atualizados em 2026-11-12, após a ADR-0012 (ESP32-S3 único, sem ESP32-C3). Cada
requisito mantém um critério de aceite verificável.

## Requisitos funcionais

**RF01 — Doom rodando no ESP32-S3**
O ESP32-S3 executa o doomgeneric (ESP-IDF/FreeRTOS) e desenha direto na tela paralela
i80 (ADR-0008).
*Critério de aceite:* o Doom inicia e é jogável na tela física, sem outro dispositivo
além do próprio S3 e dos botões.

**RF02 — Entrada por D-pad e botões de ação**
O sistema reconhece D-pad e 2 botões de ação (A, B), além de START e SELECT. Os botões
são lidos pelo próprio S3, por um expansor MCP23017 (I²C); nos testes de bancada, por
GPIO direto.
*Critério de aceite:* joga-se o Doom inteiro pelos botões físicos.

**RF03 — Launcher com Tetris e Doom**
Um menu escolhe o jogo (ADR-0011). O Doom roda uma vez por boot: ao sair, o chip
reinicia e o menu volta limpo.
*Critério de aceite:* da tela do menu entra-se no Tetris e no Doom, e volta-se ao menu.

**RF04 — Alimentação por bateria — SEM DECISÃO**
Dependia do ESP32-C3 (leitura de tensão por ADC). Falta decidir se entra no MVP e como
(ADC do próprio S3 com divisor resistivo, por exemplo).
*Critério de aceite:* a definir junto com a decisão.

**RF05 — Desligamento seguro — SEM DECISÃO**
Dependia do ESP32-C3 cortando a energia de forma independente do jogo. Sem ele, a
independência se perde (ADR-0012). Falta decidir se entra no MVP e como.
*Critério de aceite:* a definir; o original era 10 ciclos liga/desliga sem corrupção de
dados salvos (quando houver SD).

> O antigo RF03 (protocolo serial C3 → S3) foi removido com a ADR-0012.

## Requisitos não funcionais

**RNF01 — Plataforma**
ESP32-S3 N16R8 (16 MB de flash, 8 MB de PSRAM octal), ESP-IDF 6.0.x com FreeRTOS.

**RNF02 — Portabilidade do source port**
doomgeneric com um backend próprio que escreve na tela i80 (ADR-0002, ADR-0008, ADR-0010).

**RNF03 — Validação no PC antes do hardware**
Lógica, controles e jogos rodam no simulador e no `host_test` sem a placa (ADR-0010),
para que a ida à placa teste só o que é de hardware.

## Fora do MVP (extensão condicional — ver ADR-0005)

Só entra se o mínimo (RF01 a RF03 na placa) estiver pronto antes de 18/11/2026:

- Game Boy (Peanut-GB) e, depois, Game Boy Color (Walnut-CGB); Pokémon só por dump de
  cartucho próprio
- Áudio (I²S + amplificador tipo MAX98357A)
- Armazenamento em cartão microSD (ROMs, saves, configuração)
