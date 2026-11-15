# Quadro de tarefas

Reescrito em 2026-11-14, depois da bancada da tela (`docs/bancada.md`). **Prazo: apresentação em 18/11/2026.**

`[x]` = feito no nível indicado · `[ ]` = pendente. Níveis de prova, do mais fraco ao mais forte:

| Nível | Significa |
|---|---|
| **escrito** | o código existe, sem nenhuma validação |
| **PC** | validado no simulador ou no `host_test`; nunca compilado para a placa |
| **compila** | compilou com o ESP-IDF; nunca rodou na placa |
| **placa** | rodou na placa e o resultado foi conferido |

## Onde estamos em uma frase

O firmware único (sem o Doom) roda na placa até o launcher, mas **o painel do shield paralelo
não responde** (diagnóstico completo em `docs/bancada.md`). Contingência: módulo SPI (ADR-0013)
e, em qualquer caso, demonstração pelo simulador.

## Repositório (fazer primeiro)
- [ ] Commitar as correções já aplicadas na máquina local, que **ainda não estão no git**:
  `firmware/main/CMakeLists.txt` (`PRIV_REQUIRES esp_timer esp_system esp_hw_support log`),
  `firmware/components/game_registry/CMakeLists.txt` (opção por variável de ambiente) e a pasta
  `tools/lcd_teste_gpio/`
- [ ] Mudar o padrão `LCD_I80_PIN_D5` de 16 para 12 em `components/lcd_i80/Kconfig` (o GPIO 16 da
  placa não sobe) e atualizar `hardware/pinos.md` se mudar de novo
- [ ] Merge do branch `chore/reorganizacao` na `master`, se ainda não foi feito (só o Kramek e você usam o repositório)

## Fase 0 — Fundação
- [x] Estrutura do repositório e reorganização de 2026-11-12 (`docs/historico.md`)
- [x] Requisitos (`docs/requirements.md`) e ADRs 0001 a 0013 (`docs/adr/`)
- [x] **placa** — ESP-IDF 6.0.3 instalado; `hello_world` rodou na placa
- [ ] Confirmar com o professor o nível de profundidade esperado e a arquitetura (S3 único)
- [ ] Refazer `docs/architecture.svg` (S3 único, tela, botões) — ao concluir o projeto

## Fase 1 — Bancada (resultados de 13 e 14/11)
- [x] **placa** — firmware único sem o Doom: compila (ESP-IDF 6.0.3), grava e roda até o launcher
  (PSRAM de 8 MB ok, `lcd_i80` "pronto", "launcher iniciado", sem reinício)
- [x] **placa** — alimentação: 5 V da placa = 4,68 V, 3V3 = 3,32 V; regulador do shield em 3,3 V
- [x] **placa** — diagnóstico da tela paralela (`docs/bancada.md`): fiação, alimentação e sinais
  chegam ao shield, **mas o painel não responde** (ID constante, tela sempre branca)
- [x] **placa** — GPIO 16 da placa não sobe: LCD_D5 movido para o GPIO 12
- [ ] **Bloqueado:** ver imagem na tela física (depende da contingência abaixo)
- [ ] Conferir visualmente o cabo flexível do shield paralelo (5 minutos; ainda não feito)
- [ ] Botões em GPIO direto: atribuir GPIO a CIMA (padrão: desligado) e testar menu e Tetris
- [ ] Doom na placa: o link **estoura a DRAM em 31.728 bytes** (build com Doom, 13/11).
  Caminho previsto: `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY` e `.bss` do Doom na PSRAM
- [ ] Gravar o `doom1.wad` na partição `wad` e rodar o Doom
- [ ] Medir na placa: quadros/s, heap livre (zona de 6 MB), tamanho do binário

## Fase 2 — Contingência da tela (ADR-0013)
- [ ] Decidir e, se for o caso, comprar o módulo ILI9341 SPI de 2,8" (conferir: ILI9341, SPI,
  pinos soldados, entrega até segunda-feira 16/11)
- [ ] Componente `lcd_spi` com o contrato de `gfx.h` e escolha no `menuconfig` (reaproveita a
  inicialização do ILI9341)
- [ ] Teste de cores por SPI na placa
- [ ] Launcher, Tetris e Doom na tela nova

## Fase 3 — Botões definitivos
- [ ] Decidir: MCP23017 (ADR-0012) ou botões em GPIO direto. Com a tela SPI sobram 8 GPIOs, e o
  GPIO direto talvez baste no MVP
- [ ] Se for MCP23017: pinos I²C do S3 e endereço (`hardware/pinos.md`); componente
  `input_mcp23017` com a mesma interface de `input_gpio`
- [ ] Jogar o Doom inteiro pelos botões físicos (RF02)

## Fase 4 — Software já escrito (resumo)
- [x] **placa** (boot) — `main/`, `game_registry` e `launcher` compilam e rodam até iniciar o launcher (visto no log; sem imagem na tela)
- [x] **compila** — `lcd_i80`, `input_gpio`, `game_api`, Tetris
- [x] **PC** — regras, fluxo e desenho do Tetris (`host_test`)
- [x] **PC** — Doom (`doom_game`) e simulador (`firmware/sim/`), com WAD real
- [ ] Doom na placa: os objetos compilam, **o link falha por DRAM** (Fase 1)
- [ ] **PC** — `lcd_spi` (Fase 2)

## Fase 5 — Energia (SEM DECISÃO: RF04 e RF05)
- [ ] Decidir se bateria e desligamento seguro entram no MVP e como (ADR-0012)
- [ ] Se entrarem: leitura de tensão, carregamento no circuito real, rotina de desligamento
  e 10 ciclos liga/desliga sem corrupção

## Fase 6 — Montagem física
- [ ] Fiação definitiva / PCB perfurada
- [ ] Case
- [ ] Teste de autonomia real (só se a bateria entrar)

## Fase 7 — Fechamento (18/11)
- [ ] Demonstração pelo **simulador** (Doom, Tetris e launcher) como garantia: gravar também um vídeo
- [ ] Relatório e slides: arquitetura, decisões e o **diagnóstico da bancada** (`docs/bancada.md`)
  como resultado, inclusive o caminho Raspberry Pi → dois ESP32 → S3 único → tela SPI
- [ ] Ensaiar a apresentação
- [ ] Revisar as ADRs 0008 a 0013 ("Proposta" → "Aceita"/"Superada") conforme a validação na placa

### Plano proposto até 18/11 (revisar a cada dia)
- **Sáb 14:** diagnóstico fechado e documentado; decidir a compra do módulo SPI.
- **Dom 15:** commits pendentes; `lcd_spi` e teste de cores SPI (PC); gravar vídeo do simulador.
- **Seg 16:** se o módulo chegou, ligar e ver cores; botões e launcher na tela.
- **Ter 17:** Doom na placa (DRAM) se a tela funcionar; slides e ensaio de qualquer forma.
- **Qua 18:** apresentação.

## Fase 8 — Extensão condicional (ver ADR-0005)
Só avança se as Fases 1 a 3 estiverem prontas antes do prazo.
- [ ] Peanut-GB (Game Boy) integrado ao launcher; ROM só por dump de cartucho próprio
- [ ] Walnut-CGB (Game Boy Color), se sobrar tempo depois
- [ ] Áudio (I²S + MAX98357A)
- [ ] microSD (ainda não comprado; o slot do shield paralelo compartilha pinos com a tela; o módulo
  SPI traz o próprio slot, conferir pinos)