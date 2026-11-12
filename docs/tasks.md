# Quadro de tarefas

Reestruturado após a ADR-0006 (substituição da Raspberry Pi). Itens já
validados sob a arquitetura antiga foram reclassificados honestamente:
o que continua válido como lógica/design permanece marcado; o que
dependia especificamente de Linux/Raspberry Pi volta a ficar pendente,
porque precisa ser refeito no novo alvo.

## Fase 0 — Planejamento e fundação do projeto
- [x] Estrutura do repositório
- [x] Requisitos formais (`docs/requirements.md`)
- [x] ADRs (0001–0006, incluindo a arquitetura de dois chips)
- [x] Diagrama de arquitetura (atualizado para ESP32-S3 + ESP32-C3)
- [x] Ambiente de desenvolvimento documentado (atualizado para ESP-IDF)
- [ ] Confirmar com o professor o nível de profundidade esperado — e a
  mudança de Raspberry Pi para dois ESP32
- [ ] Levantar componentes e prazos de entrega (lista de compra atualizada)

## Fase 1 — Prova de conceito: doomgeneric no ESP32-S3
- [x] Configurar toolchain ESP-IDF para o N16R8 (ESP-IDF 6.0.3 no Windows; hello_world rodou na placa)
- [x] Driver da tela (paralela i80, `lcd_i80`) escrito — falta validar na tela física
- [x] Backend do doomgeneric (`components/doom_game`: DG_*, paleta→RGB565, 320×200→320×240,
  controles com 8 botões) — validado no simulador do PC; ainda NÃO compilado para a placa (ADR-0010)
- [x] Simulador no PC (`firmware-s3/sim/`): Doom e Tetris sem hardware, modo janela (SDL2) e `--headless`
- [x] Launcher + firmware único (`components/launcher`, `app/`) — validado no simulador e no host_test; ainda NÃO compilado para a placa (ADR-0011)
- [ ] Compilar `examples/doom_standalone` com o ESP-IDF e gravar o `doom1.wad` na partição `wad`
- [ ] Doom rodando na tela física, com entrada de teste (sem o ESP32-C3 ainda)
- [ ] Medir na placa: quadros/s, heap livre (zona de 6 MB), tamanho do binário

## Fase 2 — ESP32-C3 isolado
- [ ] Leitura de botões com debounce confiável
- [ ] Leitura de tensão da bateria via ADC (conferir calibração na doc atual da Espressif)
- [ ] Envio de eventos via UART seguindo o protocolo (ADR-0003)
- [ ] Lógica de detecção do botão de power

## Fase 3 — Integração ESP32-C3 + ESP32-S3
- [x] Lógica do protocolo (parsing/checksum) validada — 8/8 testes em
  Python, mantida como referência de design em `tools/`
- [ ] Porta da lógica do protocolo para C, rodando de verdade no ESP32-S3
- [ ] Testar D-pad e botões controlando o Doom no hardware real

## Fase 4 — Energia e shutdown seguro
- [ ] Integrar bateria + carregamento no circuito real
- [ ] Rotina de shutdown seguro (ESP32-C3 → ESP32-S3 → confirma → corta energia)
- [ ] 10 ciclos de liga/desliga sem corrupção de dados

## Fase 5 — Launcher e estrutura para múltiplos jogos
- [ ] Escrever o launcher (menu de seleção via D-pad)
- [ ] Escrever a camada de abstração de input (protocolo → ação virtual → jogo)
- [ ] Doom rodando através do launcher, não mais standalone

## Fase 6 — Montagem física
- [ ] Fiação definitiva / PCB perfurada
- [ ] Case
- [ ] Teste de autonomia real da bateria com uso contínuo

## Fase 7 — Fechamento
- [ ] Testes finais end-to-end
- [ ] Vídeo de demonstração (backup)
- [ ] Relatório/slides explicando arquitetura e decisões (inclusive o
  pivô Raspberry Pi → dois ESP32)
- [ ] Ensaiar a apresentação

## Fase 8 — Extensão condicional: mais jogos (ver ADR-0005)
Só avança se Doom + launcher + input estiverem funcionando de verdade no
hardware, com folga de prazo.
- [ ] Ponto de checagem: confirmar que sobrou tempo real antes de seguir
- [ ] Peanut-GB (Game Boy DMG) integrado ao launcher
- [ ] Obter a ROM de Pokémon por dump de cartucho próprio (não redistribuir)
- [ ] Tetris (implementação própria) integrado ao launcher
- [ ] Se sobrar tempo depois disso: trocar Peanut-GB por Walnut-CGB (Game Boy Color)
- [ ] Áudio (I²S + MAX98357A) — só depois do resto estável
- [ ] Armazenamento em microSD (ROMs, saves, configuração)