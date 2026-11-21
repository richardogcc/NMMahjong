#!/usr/bin/env bash
# Compila NM Mahjong para Windows x64 desde macOS (requiere: brew install mingw-w64).
set -euo pipefail
cd "$(dirname "$0")/.."

cmake -S . -B build/windows -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw-w64.cmake
cmake --build build/windows

echo "Listo: build/windows/NMMahjong.exe"
