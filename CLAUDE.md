# CLAUDE.md

Guia rápido para continuar este projeto — humano ou IA.

## Contexto

Console portátil rodando Doom (e, em fases futuras, Game Boy e Tetris) —
disciplina de Sistemas Embarcados. Arquitetura atual: dois ESP32, sem
Raspberry Pi.

- **ESP32-S3 N16R8** — roda FreeRTOS, executa o `doomgeneric` bare-metal
  e desenha na tela SPI. Futuramente hospeda o launcher e os outros jogos.
- **ESP32-C3** — botões, leitura de bateria (ADC), desligamento seguro.
  Independente do S3 — se o jogo travar, o C3 ainda corta a energia.

Os dois se comunicam por um protocolo serial de 3 bytes (ADR-0003),
inalterado desde a versão anterior do projeto.

**Isso substitui uma arquitetura anterior baseada em Raspberry Pi + ESP32
rodando Linux/Python.** Se você está vendo referências a `pi-doom/`,
`serial_daemon.py`, `uinput` ou `socat` em algum lugar do histórico do
git, isso é código da versão antiga — ver `docs/adr/0006-arquitetura-dois-chips-esp32.md`
pro motivo da mudança e o que foi aproveitado.

Documentação completa (não duplicar aqui, só apontar):
- `README.md` — visão geral e decisões tomadas
- `docs/requirements.md` — requisitos formais (RF01–RF05 + RNFs)
- `docs/adr/` — decisões de arquitetura; `0001` e `0005` estão marcadas
  como superadas mas não foram apagadas (histórico do projeto)
- `docs/architecture.svg` — diagrama de blocos atual
- `docs/dev-environment.md` — toolchain (ESP-IDF/PlatformIO, sem mais SDL/Linux em produção)
- `docs/tasks.md` — **fonte de verdade** do que já foi feito por fase

## Comandos úteis

Testes da lógica do protocolo (referência de design, Python puro, sem
hardware — a implementação real agora é em C dentro de `firmware-s3/`):
```bash
cd tools/tests && python3 test_protocol.py
```

Build dos firmwares (ESP-IDF ou PlatformIO, alvo `esp32s3` e `esp32c3`
respectivamente) — comandos exatos a confirmar assim que o ambiente for
montado; ver `docs/dev-environment.md`.

## Protocolo (resumo rápido — detalhe completo na ADR-0003)

Mensagens de 3 bytes: `[tipo][payload][checksum]`, checksum = tipo XOR payload.
- `0x01` botão — payload = `(pressed << 7) | button_id`
- `0x02` power-off solicitado
- `0x03` ack de shutdown seguro

Botões: `0`=UP, `1`=DOWN, `2`=LEFT, `3`=RIGHT, `4`=AÇÃO A, `5`=AÇÃO B,
`6`=START, `7`=SELECT (os dois últimos reservados pro Game Boy).

## Pegadinhas já conhecidas (algumas herdadas da versão anterior, outras novas)

- **Sem Linux no caminho de produção** — `socat`, `uinput`, o daemon
  Python: tudo isso foi específico da arquitetura antiga (Pi + Linux) e
  não se aplica mais. Não perder tempo tentando reaproveitar.
- **ADC do ESP32-C3**: há relatos de imprecisão de calibração em algumas
  revisões/versões do SDK — conferir a documentação atual da Espressif
  antes de confiar na leitura de bateria sem calibração manual.
- **Áudio (I2S) costuma conflitar com pinos da tela SPI** em projetos
  parecidos rodando doomgeneric em ESP32-S3 — por isso o áudio é a
  última camada a entrar (ADR-0005), depois de tudo o resto validado.
- **Walnut-CGB é um fork, não a biblioteca oficial** — se for adotado
  pro Game Boy Color, manter a atribuição correta da cadeia de autoria
  (Peanut-GB → fork CGB → Walnut-CGB) na documentação.
- **WAD (Doom) e ROMs (Game Boy) são dados de jogo, não código** — não
  versionar no git. ROMs comerciais só de cartucho próprio (dump legal).

## Convenções

- Commits: `tipo: descrição curta` (`feat`, `fix`, `docs`, `chore`, `test`).
- Decisão de arquitetura nova → vira ADR em `docs/adr/`. Se substituir uma
  ADR anterior, a antiga fica marcada como "Superada por ADR-000X", não é
  apagada — é histórico real de engenharia, útil pro relatório final.
- Item concluído ou item novo → atualizar `docs/tasks.md` no mesmo commit.

## Status atual (resumo — detalhe em docs/tasks.md)

- Pivô de arquitetura concluído na documentação (Raspberry Pi → dois
  ESP32). Fases 0 está com a maior parte da fundação redocumentada.
- Fases 1–7: pendentes de reimplementação/validação no novo alvo — nada
  do código Python da arquitetura anterior roda em produção agora.
- Fase 8 (múltiplos jogos): estrutura (launcher + input) entra cedo,
  conteúdo dos jogos é condicional a sobrar tempo (ver ADR-0005).