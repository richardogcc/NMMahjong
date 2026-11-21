#!/usr/bin/env bash
# Prueba el ejecutable de Windows dentro de un contenedor con Wine: lo arranca
# en modo de capturas, recorre el menú y una partida y deja las imágenes en
# dist/windows-test/. No sustituye a probarlo en un Windows de verdad (Wine no
# es Windows, y aquí no hay sonido ni tarjeta gráfica), pero detecta fallos de
# arranque, de carga de recursos y de dibujado.
#
# Requiere Docker en marcha. Uso: scripts/test-windows.sh
set -euo pipefail
cd "$(dirname "$0")/.."

docker info >/dev/null 2>&1 || { echo "Docker no está en marcha: abre Docker Desktop y vuelve a intentarlo." >&2; exit 1; }
[ -f build/windows/NMMahjong.exe ] || scripts/build-windows.sh

OUT="dist/windows-test"
rm -rf "$OUT"
mkdir -p "$OUT"
docker build -q --platform linux/amd64 -t nmmahjong-wine scripts/wine >/dev/null

run() {  # run <subcarpeta> [VAR=valor...]
  local name="$1"; shift
  mkdir -p "$OUT/$name"
  local envs=()
  for pair in "$@"; do envs+=(-e "$pair"); done
  docker run --rm --platform linux/amd64 \
    -v "$PWD/build/windows:/game:ro" -v "$PWD/$OUT/$name:/out" \
    -e NM_CAPTURE='Z:\out' -e NM_LANG=es ${envs[@]+"${envs[@]}"} \
    nmmahjong-wine timeout 600 xvfb-run -a -s "-screen 0 1280x720x24" wine NMMahjong.exe \
    || echo "  ($name terminó con error o se agotó el tiempo)"
  echo "$name: $(ls "$OUT/$name" | tr '\n' ' ')"
}

run nivel NM_CAPTURE_LEVEL=3
run clasicas NM_CAPTURE_LEVEL=9 NM_TILES=1 NM_CAPTURE_FRAMES=1
run opciones NM_CAPTURE_PANEL=1

echo "Capturas en $OUT"
