#!/usr/bin/env python3
"""Recorta LXGW WenKai TC a los caracteres que usa el juego.

Hay que volver a ejecutarlo cuando cambien los textos en chino
(src/game/StringsPtZh.cpp): un carácter que no esté en la fuente se vería
como un hueco. Necesita la fuente completa, que no está en el repositorio
(https://github.com/lxgw/LxgwWenkaiTC/releases), y fonttools:

    pip install fonttools
    scripts/make-cjk-font.py ruta/a/LXGWWenKaiTC-Regular.ttf
"""
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
TILES = "一二三四五六七八九萬東南西北中發梅蘭菊竹春夏秋冬"

if len(sys.argv) != 2:
    sys.exit(__doc__)
source = (ROOT / "src/game/StringsPtZh.cpp").read_text(encoding="utf-8")
chars = {c for c in source if ord(c) > 0x2E00} | set(TILES) | set("·©×") | {chr(c) for c in range(32, 127)}
subprocess.run(
    ["pyftsubset", sys.argv[1], "--text=" + "".join(sorted(chars)), "--layout-features=", "--no-hinting",
     "--desubroutinize", "--output-file=" + str(ROOT / "assets/fonts/LXGWWenKaiTC-subset.ttf")],
    check=True)
print(f"{len(chars)} caracteres")
