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

> **Atenção — decisões posteriores a este texto.** O ESP32-S3 passou a assumir
> todas as funções e o ESP32-C3 saiu do BOM revisado; a tela é **paralela de
> 8 bits (i80)**, não SPI (`docs/adr/0008-tela-ili9341-paralela-i80.md`); os botões
> devem usar um expansor MCP23017 (I²C); o ambiente é Windows com ESP-IDF 6.0.3.
> As seções abaixo sobre dois chips, protocolo serial e tela SPI são **históricas**
> até existir um ADR registrando o S3 único.

Documentação completa (não duplicar aqui, só apontar):
- `README.md` — visão geral e decisões tomadas
- `docs/requirements.md` — requisitos formais (RF01–RF05 + RNFs)
- `docs/adr/` — decisões de arquitetura; `0001` e `0004` estão marcadas
  como superadas mas não foram apagadas (histórico do projeto). `0008`
  (tela paralela) e `0009` (camada de jogos) são as mais recentes
- `docs/architecture.svg` — diagrama de blocos atual
- `docs/dev-environment.md` — toolchain (ESP-IDF/PlatformIO, sem mais SDL/Linux em produção)
- `docs/tasks.md` — **fonte de verdade** do que já foi feito por fase

## Comandos úteis

Testes da lógica do protocolo (referência de design, Python puro, sem
hardware — a implementação real agora é em C dentro de `firmware-s3/`):
```bash
cd pi-doom/tests && python3 test_protocol.py
```

Testes do Tetris no PC (sem hardware; precisa de gcc — WSL ou MSYS2):
```bash
cd firmware-s3/host_test && make
make SAN="-fsanitize=address,undefined"   # só Linux/WSL: acha estouro de índice
```

Simulador no PC (Doom e Tetris sem hardware; MSYS2 UCRT64 + SDL2 — ver `firmware-s3/sim/README.md`):
```bash
cd firmware-s3/sim && make
./build/portatil_sim.exe --wad ../../wad/doom1.wad                 # launcher (menu) + Tetris + Doom, como na placa
./build/portatil_sim.exe --game doom --wad ../../wad/doom1.wad      # só o Doom (setas, Z=A, X=B, Enter=START)
make headless && ./build_headless/portatil_sim.exe --game doom --wad doom1.wad --headless --frames 300 --shot 299
```

Build e gravação do teste na placa (terminal do ESP-IDF 6.0.3, Windows):
```
cd firmware-s3\examples\tetris_standalone
idf.py set-target esp32s3
idf.py menuconfig        # pinos em "LCD i80" e "Entrada: botoes em GPIO direto"
idf.py build flash monitor
```

## Código do firmware-s3 (camadas)

Cada camada só conhece a de baixo; **jogos nunca leem GPIO nem falam com a
tela** — usam só os contratos de `game_api` (ADR-0009):

```
tetris_standalone (main) → tetris_game → tetris_core   (regras, C puro; autoria da dupla)
doom_standalone   (main) → doom_game   → doomgeneric  (third_party/, GPL; ADR-0010)
app (firmware único) (main) → launcher → { tetris_game, doom_game }  (ADR-0011)
                                │
              game_api  (game.h, input.h, gfx.h — sem hardware)
                                │
              lcd_i80 (tela)   input_gpio (botões)      ← únicos que tocam hardware
```
Detalhes, pinos e o que ainda não foi verificado: `firmware-s3/README.md`.
O simulador (`firmware-s3/sim/`) troca só `lcd_i80`/`input_gpio` por tela em framebuffer e teclado.
Doom: tela em paisagem 320×240 (`gfx_set_orientation`), WAD numa partição de flash (`wad`),
não reentrante (uma vez por boot) — ver ADR-0010 e `examples/doom_standalone/README.md`.

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
- **Tela paralela (shield ILI9341):** alimentação/nível lógico dos buffers 245
  do shield ainda em diagnóstico — conferir antes de culpar o código.
- **ESP-IDF:** `build/`, `sdkconfig` e `managed_components/` não vão para o git;
  `sdkconfig.defaults` só vale quando ainda não existe `sdkconfig` gerado.
- **Pinos do ESP32-S3 N16R8:** evitar 0, 3, 19, 20, 26–37 (flash/PSRAM octal), 43–46;
  38 e 48 podem ser o LED RGB conforme a placa.
- **Walnut-CGB é um fork, não a biblioteca oficial** — se for adotado
  pro Game Boy Color, manter a atribuição correta da cadeia de autoria
  (Peanut-GB → fork CGB → Walnut-CGB) na documentação.
- **Freedoom (28 MB) só roda no simulador**; na placa o WAD é o `doom1.wad` shareware (~4 MB, partição de 6 MB).
- **Doom não é reentrante**: depois de sair, só reiniciando o chip.
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
- Tetris (núcleo da dupla) já portado para as camadas do `firmware-s3` e testado
  no PC; falta validar tela e botões na placa (ADR-0008 e ADR-0009).
- Firmware único (`app/`: launcher + Tetris + Doom) validado no simulador e no host_test; falta compilar para a placa (ADR-0011).
- Doom: port (`doom_game`) e simulador no PC prontos e validados com Freedoom; falta compilar
  para a placa, gravar o WAD e medir (ADR-0010).