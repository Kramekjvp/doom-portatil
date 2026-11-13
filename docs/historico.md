# Histórico de arquitetura e mapa de caminhos

## As três arquiteturas do projeto

| # | Arquitetura | Decisões | Por que mudou |
|---|---|---|---|
| 1 | Raspberry Pi Zero 2 W (roda o jogo) + microcontrolador de I/O, tela SPI | ADR-0001 a 0005 | Raspberry Pi sem estoque nos revendedores nacionais (ADR-0006) |
| 2 | ESP32-S3 (jogos, tela) + ESP32-C3 (botões, bateria, desligamento), UART entre eles | ADR-0003, 0004, 0006 | S3 passou a assumir todas as funções; botões via MCP23017 (ADR-0012) |
| 3 | **ESP32-S3 único**, tela paralela i80, botões por MCP23017 (atual) | ADR-0008 a 0012 | — |

## Onde está o código antigo

A reorganização de 2026-11-12 removeu `pi-doom/` (protocolo serial em Python, daemon,
testes) e `mcu-firmware/`. Nada foi perdido: a tag `pre-reorganizacao` aponta o estado
anterior.

```bash
git ls-tree -r pre-reorganizacao --name-only | grep pi-doom     # lista o que existia
git show pre-reorganizacao:pi-doom/protocol.py                  # lê um arquivo antigo
```

## Mapa de caminhos: antes → agora

As ADRs 0005 a 0011 citam os caminhos da coluna "antes".

| Antes | Agora |
|---|---|
| `firmware-s3/` | `firmware/` (é o próprio projeto ESP-IDF) |
| `firmware-s3/app/` | raiz de `firmware/` (`firmware/main/`, `firmware/partitions.csv`, `firmware/sdkconfig.defaults`) |
| `firmware-s3/examples/tetris_standalone`, `doom_standalone` | removidos; para validar só tela, botões e Tetris: `idf.py -DPORTATIL_SEM_DOOM=1 build` |
| `firmware-s3/third_party/doomgeneric`, `pi-doom/doomgeneric` | `third_party/doomgeneric` (um único submódulo) |
| `firmware-s3/tools/preparar_wad.sh`, `wad/preparar_wad` | `tools/preparar_wad.sh` |
| `JOGOS[]` em `app/main/main.c` e em `sim/sim_main.c` | `firmware/components/game_registry` |
| `BOM.md` | `hardware/bom.md` |
| `docs/README.md` | `docs/adr/README.md` (ADRs) e `README.md` da raiz |
| `docs/architecture.svg` | removido; será refeito na conclusão (S3 único, tela i80, MCP23017) |
| `pi-doom/`, `mcu-firmware/`, `host/`, `legacy/`, `plataform-host/` | removidos |
