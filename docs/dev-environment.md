# Ambiente de desenvolvimento

Atualizado em 2026-11-12 (ADR-0012: ESP32-S3 único). Não há mais alvo Linux nem segundo chip.

## Firmware (`firmware/`) — ESP32-S3 N16R8

- **ESP-IDF 6.0.3** no Windows, usado pelo "terminal do ESP-IDF" (`idf.py`).
- Placa N16R8: 16 MB de flash e 8 MB de PSRAM octal (já configurados em
  `firmware/sdkconfig.defaults`).
- Cabo USB para gravar e ver o log.
- doomgeneric como submódulo em `third_party/doomgeneric`:
  `git submodule update --init --recursive`.

## Testes e simulador no PC

Compilam os mesmos fontes da placa, sem hardware (RNF03).

- **MSYS2, terminal UCRT64:**
  `pacman -S mingw-w64-ucrt-x86_64-gcc make mingw-w64-ucrt-x86_64-SDL2`
- `firmware/host_test/`: testes de entrada, regras do Tetris, launcher e fluxo do jogo.
- `firmware/sim/`: simulador com janela (SDL2) ou `make headless` (sem SDL2).
- Comandos: `firmware/README.md` e `firmware/sim/README.md`.

## Git e scripts

- **Git Bash** (vem com o Git for Windows) para scripts `.sh` como `tools/preparar_wad.sh`.
- Fins de linha: o repositório guarda LF (`.gitattributes`); não converta para CRLF.
- Commits: `tipo: descrição curta` (`feat`, `fix`, `docs`, `chore`, `test`).
- `build/`, `sdkconfig`, `managed_components/`, `*.wad` e ROMs **não** vão para o git.

## O que ficou obsoleto

- `socat`, portas seriais virtuais, `uinput` e o daemon Python: eram do modelo Raspberry
  Pi + Linux (ver `docs/historico.md`).
- Toolchain do ESP32-C3, PlatformIO e driver de tela SPI: não fazem parte do projeto atual.
