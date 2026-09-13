# Quadro de tarefas

Baseado nas fases do roteiro de produção do projeto. Formato simples em
markdown, versionado junto com o código.

## Fase 0 — Planejamento e fundação do projeto
- [x] Estrutura do repositório
- [x] Requisitos formais (`docs/requirements.md`)
- [x] ADRs: microcontrolador, source port, protocolo serial
- [x] Diagrama de arquitetura preliminar
- [x] Ambiente de desenvolvimento documentado
- [ ] Confirmar com o professor o nível de profundidade esperado
- [ ] Levantar todos os componentes e prazos de entrega

## Fase 1 — Prova de conceito: Pi rodando Doom sozinha
- [x] Build do doomgeneric localmente (backend SDL, sem hardware — validado no Arch)
- [ ] Instalar sistema mínimo na Pi (sem desktop)
- [ ] Validar saída de vídeo via framebuffer/DRM
- [ ] Validar áudio básico

## Fase 2 — Microcontrolador isolado
- [ ] Leitura de botões com debounce confiável
- [ ] Leitura de tensão da bateria via ADC
- [ ] Envio de eventos de teste via UART seguindo o protocolo (ADR-0003)
- [ ] Lógica de detecção do botão de power

## Fase 3 — Integração MCU + Pi
- [x] Simular o MCU com porta serial virtual (socat) + script Python
- [x] Daemon na Pi lendo a serial e injetando eventos via uinput (validado end-to-end com o simulador, Arch Linux)
- [x] Testar D-pad e botões controlando o Doom de fato (validado no Arch, sem hardware — daemon + simulador)

## Fase 4 — Energia e shutdown seguro
- [ ] Integrar bateria + carregamento no circuito real
- [ ] Rotina de shutdown seguro (MCU → Pi → desmonta FS → corta energia)
- [ ] 10 ciclos de liga/desliga sem corromper o cartão SD

## Fase 5 — Tela e áudio finais
- [x] Fechar decisão de tela (SPI, ver ADR-0004)
- [ ] Trocar backend de teste pela tela definitiva
- [ ] Integrar áudio final, se entrar no escopo

## Fase 6 — Montagem física
- [ ] Fiação definitiva / PCB perfurada
- [ ] Case
- [ ] Teste de autonomia real da bateria com uso contínuo

## Fase 7 — Fechamento
- [ ] Testes finais end-to-end
- [ ] Vídeo de demonstração (backup)
- [ ] Relatório/slides explicando arquitetura e decisões
- [ ] Ensaiar a apresentação

## Fase 8 — Extensão opcional: múltiplos jogos (pós-MVP)
Só começa depois das Fases 0–7 fechadas com folga (ver ADR-0005).
- [ ] Confirmar que sobrou tempo real antes de iniciar
- [ ] Integrar Peanut-GB com um backend de framebuffer próprio
- [ ] Escrever/adaptar um clone de Tetris open-source pro framebuffer
- [ ] Escrever o launcher (menu de seleção de jogo via D-pad)
- [ ] Obter a ROM de Pokémon por dump de cartucho próprio (não redistribuir)
- [ ] Testar os três jogos (Doom, Game Boy, Tetris) com os botões físicos