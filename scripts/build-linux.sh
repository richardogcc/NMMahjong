#!/usr/bin/env bash
# Compila NM Mahjong para Linux (x86-64) dentro de un contenedor de Docker,
# pasa las pruebas y arranca el juego sin pantalla para sacar unas capturas
# (dist/linux-test/). Requiere Docker en marcha.
#
# En un Linux de verdad no hace falta Docker: docs/COMPILAR.md explica qué instalar.
set -euo pipefail
cd "$(dirname "$0")/.."

docker info >/dev/null 2>&1 || { echo "Docker no está en marcha: abre Docker Desktop y vuelve a intentarlo." >&2; exit 1; }
docker build -q --platform linux/amd64 -t nmmahjong-linux scripts/linux >/dev/null

rm -rf dist/linux-test
mkdir -p build/linux dist/linux-test
docker run --rm --platform linux/amd64 -v "$PWD:/src" nmmahjong-linux bash -ec '
  cmake -S . -B build/linux -G Ninja -DCMAKE_BUILD_TYPE=Release >/dev/null
  cmake --build build/linux
  ctest --test-dir build/linux --output-on-failure
  cd build/linux
  NM_CAPTURE=/src/dist/linux-test NM_LANG=es NM_CAPTURE_LEVEL=3 \
    timeout 300 xvfb-run -a -s "-screen 0 1280x720x24" ./NMMahjong || echo "(la prueba de arranque falló)"
'
echo "Listo: build/linux/NMMahjong   Capturas: $(ls dist/linux-test | tr '\n' ' ')"
