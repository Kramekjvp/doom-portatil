# doom_standalone — Doom direto na placa (sem launcher)

Projeto de teste do ESP32-S3 N16R8: `lcd_i80` + `input_gpio` + `doom_game`.
Decisões: `docs/adr/0010-doom-no-s3-e-simulador-no-pc.md`.
**Status: escrito e conferido contra o simulador, mas AINDA NÃO compilado com o ESP-IDF.**

## 1. Pré-requisitos
```
git submodule add https://github.com/ozkl/doomgeneric.git firmware-s3/third_party/doomgeneric
git submodule update --init --recursive
```
(Se o submódulo antigo `pi-doom/doomgeneric` ainda existir, ele pode continuar lá; este
projeto usa `third_party/`.) Baixe o `doom1.wad` (shareware, ~4 MB; pode ser o `.zip`) e valide/prepare com o script
(confere cabeçalho e tamanho, copia para `wad/doom1.wad` na raiz e imprime os comandos):
```
sh firmware-s3/tools/preparar_wad.sh /c/Users/SEU_USUARIO/Downloads/doom1.wad
```
**Não** versione o WAD (`*.wad` já está no `.gitignore`).

## 2. Compilar e gravar (terminal do ESP-IDF 6.0.3)
```
cd firmware-s3\examples\doom_standalone
idf.py set-target esp32s3
idf.py menuconfig        # "LCD i80", "Entrada: botoes em GPIO direto", "Doom (doomgeneric)"
idf.py build flash
```
Grave o WAD na partição `wad` (offset 0x310000, ver `partitions.csv`):
```
python -m esptool --chip esp32s3 -p COMx write_flash 0x310000 caminho\doom1.wad
idf.py monitor
```
Se mudar `partitions.csv`, rode `idf.py erase-flash` uma vez e regrave tudo.

## 3. Primeira execução: o que olhar no log
- `w_file_esp: WAD mapeado da flash: N lumps` → partição e WAD ok. Se aparecer "não contém
  um WAD", o `write_flash` não foi feito (ou o offset difere do `partitions.csv`).
- `zone memory: ..., 600000 allocated` → 6 MB da PSRAM. Se der erro de memória, reduza
  `Doom → Memória do Doom` para 5 ou 4.
- Tela: barras R/G/B (2,5 s, retrato) e depois o Doom em paisagem. Imagem de cabeça para
  baixo ou espelhada → `LCD_I80_MADCTL_LANDSCAPE` (0x28 → 0xE8, 0x68 ou 0xA8).
- Botões: START abre o menu; A confirma; use os GPIOs do `menuconfig`. SELECT precisa de
  GPIO (padrão -1 = ausente) para strafe/troca de arma.

## 4. Riscos conhecidos da primeira compilação (não verificados)
- **DRAM insuficiente no link** (`region dram0_0_seg overflowed`): as tabelas estáticas do
  Doom (.bss) são grandes. Veja `idf.py size-components`; saída usual: habilitar
  `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY` e marcar os maiores arrays com `EXT_RAM_BSS_ATTR`.
- **Símbolos de libc** ausentes no link (`system`, `mkdir` em `i_system.c`/`m_misc.c`): são
  rotas de erro/config do Doom; um stub de duas linhas resolve.
- **Nomes de `CONFIG_*` do `sdkconfig.defaults`** (cache de dados 64 KB / linha 64 B)
  podem ter mudado no IDF 6: confira no `menuconfig` — nome errado é ignorado em silêncio.
- **Desempenho**: ainda sem medida. Em Options do Doom há "Low detail" se for preciso ganhar
  quadros. Quanto ao barramento: ~150 KB/quadro → teto de ~65 quadros/s a 10 MHz.