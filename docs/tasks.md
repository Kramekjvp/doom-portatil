# Quadro de tarefas

Baseado nas fases do roteiro de produção do projeto. Formato simples em
markdown, versionado junto com o código — se quiser espelhar em
Issues/Projects do GitHub mais adiante, os itens já saem daqui prontos.

## Fase 0 — Planejamento e fundação do projeto
- [x] Estrutura do repositório
- [x] Requisitos formais (`docs/requirements.md`)
- [x] ADRs: microcontrolador, source port, protocolo serial
- [x] Diagrama de arquitetura preliminar
- [x] Ambiente de desenvolvimento documentado
- [ ] Confirmar com o professor o nível de profundidade esperado
- [ ] Levantar todos os componentes e prazos de entrega

## Fase 1 — Prova de conceito: Pi rodando Doom sozinha
- [ ] Build do doomgeneric localmente (backend SDL, sem hardware)
- [ ] Instalar sistema mínimo na Pi (sem desktop)
- [ ] Validar saída de vídeo via framebuffer/DRM
- [ ] Validar áudio básico

## Fase 2 — Microcontrolador isolado
- [ ] Leitura de botões com debounce confiável
- [ ] Leitura de tensão da bateria via ADC
- [ ] Envio de eventos de teste via UART seguindo o protocolo (ADR-0003)
- [ ] Lógica de detecção do botão de power

## Fase 3 — Integração MCU + Pi
- [ ] Simular o MCU com porta serial virtual (socat) + script Python
- [ ] Daemon na Pi lendo a serial e injetando eventos via uinput
- [ ] Testar D-pad e botões controlando o Doom de fato

## Fase 4 — Energia e shutdown seguro
- [ ] Integrar bateria + carregamento no circuito real
- [ ] Rotina de shutdown seguro (MCU → Pi → desmonta FS → corta energia)
- [ ] 10 ciclos de liga/desliga sem corromper o cartão SD

## Fase 5 — Tela e áudio finais
- [ ] Fechar decisão de tela (SPI vs. HDMI)
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
