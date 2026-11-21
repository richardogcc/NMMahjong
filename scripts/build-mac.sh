#!/usr/bin/env bash
# Compila NM Mahjong para macOS como binario universal (Apple Silicon e Intel)
# y pasa las pruebas.
set -euo pipefail
cd "$(dirname "$0")/.."

cmake -S . -B build/mac -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build/mac
ctest --test-dir build/mac --output-on-failure

echo "Listo: build/mac/NMMahjong.app"
