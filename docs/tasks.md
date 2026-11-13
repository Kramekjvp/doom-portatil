# Quadro de tarefas

Reescrito em 2026-11-12, após a ADR-0012 (ESP32-S3 único). **Prazo: apresentação em 18/11/2026.**

`[x]` = feito no nível indicado · `[ ]` = pendente. Níveis de prova, do mais fraco ao mais forte:

| Nível | Significa |
|---|---|
| **escrito** | o código existe, sem nenhuma validação |
| **PC** | validado no simulador ou no `host_test`; nunca compilado para a placa |
| **compila** | compilou com o ESP-IDF; nunca rodou na placa |
| **placa** | rodou na placa e o resultado foi conferido |

## Fase 0 — Fundação
- [x] Estrutura do repositório e reorganização de 2026-11-12 (`docs/historico.md`)
- [x] Requisitos (`docs/requirements.md`) e ADRs 0001 a 0012 (`docs/adr/`)
- [x] **placa** — ESP-IDF 6.0.3 instalado; `hello_world` rodou na placa
- [ ] Confirmar com o professor o nível de profundidade esperado e a arquitetura (S3 único)
- [ ] Refazer `docs/architecture.svg` (S3 único, tela i80, MCP23017) — ao concluir o projeto

## Fase 1 — Bancada e primeira compilação (prioridade até 18/11)
Nada abaixo rodou na placa ainda. A placa do S3 foi soldada para fornecer 5 V; nada foi testado.
- [ ] Compilar o firmware único: primeiro `idf.py -DPORTATIL_SEM_DOOM=1 build` (menu + Tetris);
  depois sem a flag (com Doom). Riscos conhecidos: `firmware/README.md`
- [ ] Tela: barras R/G/B ao ligar (confere fiação, cor e orientação); ajustar `LCD_I80_MADCTL`
- [ ] Conferir alimentação (5 V) e nível lógico dos buffers 245 do shield (`hardware/pinos.md`)
- [ ] Botões em GPIO direto: atribuir GPIO a CIMA (padrão: desligado) e testar o menu e o Tetris
- [ ] Gravar o `doom1.wad` na partição `wad` e ver o Doom na tela física
- [ ] Medir na placa: quadros/s, heap livre (zona de 6 MB), tamanho do binário

## Fase 2 — Botões definitivos (MCP23017)
- [ ] Definir pinos I²C do S3 e endereço do MCP23017 (`hardware/pinos.md`)
- [ ] Componente `input_mcp23017` com a mesma interface de `input_gpio` (`input_gpio_read()` → máscara de 8 botões)
- [ ] Jogar o Doom inteiro pelos botões físicos (RF02)

## Fase 3 — Software já escrito (resumo)
- [x] **compila** — `lcd_i80`, `input_gpio`, `game_api`, Tetris (compilado em 17/10 com o ESP-IDF 6.0.3;
  o driver ganhou orientação paisagem depois: recompilar)
- [x] **PC** — regras, fluxo e desenho do Tetris (`host_test`)
- [x] **PC** — launcher (menu) e `game_registry` (lista única de jogos)
- [x] **PC** — Doom (`doom_game`, backend doomgeneric) e simulador (`firmware/sim/`), com WAD real
- [ ] **compila** — launcher, `game_registry`, Doom e o firmware único (`firmware/main/`) nunca foram compilados

## Fase 4 — Energia (SEM DECISÃO: RF04 e RF05)
- [ ] Decidir se bateria e desligamento seguro entram no MVP e como (ADR-0012)
- [ ] Se entrarem: leitura de tensão, carregamento no circuito real, rotina de desligamento
  e 10 ciclos liga/desliga sem corrupção

## Fase 5 — Montagem física
- [ ] Fiação definitiva / PCB perfurada
- [ ] Case
- [ ] Teste de autonomia real (só se a bateria entrar)

## Fase 6 — Fechamento (18/11)
- [ ] Testes finais ponta a ponta
- [ ] Vídeo de demonstração (reserva, caso a placa falhe na apresentação)
- [ ] Slides: arquitetura e decisões (inclusive o caminho Raspberry Pi → dois ESP32 → S3 único)
- [ ] Ensaiar a apresentação
- [ ] Revisar as ADRs 0008 a 0011 ("Proposta" → "Aceita") conforme a validação na placa

## Fase 7 — Extensão condicional (ver ADR-0005)
Só avança se as Fases 1 e 2 estiverem prontas antes do prazo.
- [ ] Peanut-GB (Game Boy) integrado ao launcher; ROM só por dump de cartucho próprio
- [ ] Walnut-CGB (Game Boy Color), se sobrar tempo depois
- [ ] Áudio (I²S + MAX98357A)
- [ ] microSD (ainda não comprado; o slot do shield da tela compartilha pinos — conferir antes)
