#!/bin/sh
# Valida o doom1.wad e o deixa pronto em wad/doom1.wad (usado pelo simulador e gravado na placa).
#
#   sh tools/preparar_wad.sh  CAMINHO/doom1.wad   (ou um .zip que o contenha)
#
# Confere: cabecalho IWAD/PWAD, tamanho que cabe na particao "wad" (6 MB, ver
# firmware/partitions.csv) e imprime o MD5. Nao baixa nada.
set -e

PART_SIZE=$((0x600000))          # igual ao partitions.csv
PART_OFFSET=0x310000             # idem
ESPERADO_BYTES=4196020           # doom1.wad shareware 1.9
ESPERADO_MD5=f0cefca49926d00903cf57551d901abe   # de memoria: confira, nao e garantia

aqui=$(cd "$(dirname "$0")" && pwd)
destino="$aqui/../wad"
src="$1"
[ -n "$src" ] && [ -f "$src" ] || { echo "uso: sh $0 CAMINHO/doom1.wad (ou .zip)"; exit 2; }

tmp=""
case "$src" in
  *.zip|*.ZIP)
    tmp=$(mktemp -d)
    if command -v bsdtar >/dev/null 2>&1; then bsdtar -xf "$src" -C "$tmp"
    elif command -v unzip >/dev/null 2>&1; then unzip -q "$src" -d "$tmp"
    else echo "erro: nem bsdtar nem unzip (no MSYS2: pacman -S unzip)"; exit 1; fi
    wad=$(find "$tmp" -iname 'doom1.wad' | head -n 1)
    [ -n "$wad" ] || { echo "erro: nao ha doom1.wad dentro de $src"; rm -rf "$tmp"; exit 1; }
    ;;
  *) wad="$src" ;;
esac

tam=$(wc -c < "$wad" | tr -d ' ')
cab=$(head -c 4 "$wad")
if [ "$cab" != "IWAD" ] && [ "$cab" != "PWAD" ]; then
  echo "ERRO: nao e um WAD (cabecalho \"$cab\"). Provavel pagina de erro salva como .wad."
  [ -n "$tmp" ] && rm -rf "$tmp"; exit 1
fi

# tamanho real pelo cabecalho: diretorio + 16 bytes por lump (mesma conta do w_file_esp.c)
set -- $(od -An -tu4 -j4 -N8 "$wad")
lumps=$1; dir=$2
real=$((dir + lumps * 16))

echo "arquivo : $wad"
echo "tamanho : $tam bytes ($((tam / 1024)) KB), lumps: $lumps, pelo cabecalho: $real bytes"
if [ "$real" -gt "$PART_SIZE" ] || [ "$tam" -gt "$PART_SIZE" ]; then
  echo "ERRO: nao cabe na particao wad ($PART_SIZE bytes). Esse e o WAD certo? (freedoom1.wad e do PC, nao da placa)"
  [ -n "$tmp" ] && rm -rf "$tmp"; exit 1
fi
md5=$(md5sum "$wad" | cut -d' ' -f1)
echo "md5     : $md5"
if [ "$tam" = "$ESPERADO_BYTES" ] && [ "$md5" = "$ESPERADO_MD5" ]; then
  echo "versao  : doom1.wad shareware 1.9 (confere)"
else
  echo "versao  : diferente do doom1.wad 1.9 esperado ($ESPERADO_BYTES bytes). Serve se for um IWAD valido, mas confira."
fi

mkdir -p "$destino"
cp "$wad" "$destino/doom1.wad"       # o nome TEM de ser doom1.wad: o Doom identifica o jogo por ele
[ -n "$tmp" ] && rm -rf "$tmp"
dest_abs=$(cd "$destino" && pwd)
echo
echo "ok: $dest_abs/doom1.wad"
echo "simulador: ./build/portatil_sim.exe --game doom --wad $dest_abs/doom1.wad"
echo "placa    : python -m esptool --chip esp32s3 -p COMx write_flash $PART_OFFSET $dest_abs/doom1.wad"
