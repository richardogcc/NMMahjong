#!/usr/bin/env python3
"""Genera el logo de NM Mahjong (necesita Pillow: pip install pillow).

Uso: python3 scripts/make-icon.py
Escribe assets/icon/icon.png (1024 px), icon.ico (Windows) y, en macOS,
icon.icns mediante las herramientas del sistema sips e iconutil.
"""
import math
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter

SS = 2  # se dibuja al doble y se reduce para suavizar los bordes
SIZE = 1024 * SS
OUT = Path(__file__).resolve().parent.parent / "assets" / "icon"

IVORY, IVORY_EDGE = (252, 247, 233), (226, 214, 186)
JADE, JADE_DARK = (38, 140, 108), (20, 92, 74)
RED, GREEN, NAVY, GOLD = (204, 44, 54), (28, 138, 84), (28, 52, 110), (236, 182, 62)


def s(value):
    return int(round(value * SS))


def background():
    """Fondo rojo laca con degradado, viñeta y un fino marco dorado."""
    img = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    grad = Image.new("RGBA", (SIZE, SIZE))
    px = grad.load()
    for y in range(SIZE):
        for x in range(0, SIZE, 1):
            d = math.hypot(x - SIZE * 0.38, y - SIZE * 0.28) / (SIZE * 0.95)
            k = min(1.0, d)
            px[x, y] = (int(196 - 96 * k), int(34 - 20 * k), int(40 - 18 * k), 255)
    mask = Image.new("L", (SIZE, SIZE), 0)
    inset, radius = s(52), s(212)
    ImageDraw.Draw(mask).rounded_rectangle((inset, inset, SIZE - inset, SIZE - inset), radius, fill=255)
    img.paste(grad, (0, 0), mask)
    draw = ImageDraw.Draw(img)
    draw.rounded_rectangle((inset + s(18), inset + s(18), SIZE - inset - s(18), SIZE - inset - s(18)),
                           radius - s(18), outline=GOLD + (190,), width=s(7))
    return img


def tile(face):
    """Una ficha con su canto de marfil y jade; `face` dibuja el símbolo."""
    w, h, depth, r = s(380), s(500), s(54), s(60)
    img = Image.new("RGBA", (w + depth + s(40), h + depth + s(40)), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    for i in range(depth, 0, -1):
        k = i / depth
        if k > 0.42:
            color = tuple(int(JADE[c] + (JADE_DARK[c] - JADE[c]) * k) for c in range(3))
        else:
            color = tuple(int(IVORY[c] + (IVORY_EDGE[c] - IVORY[c]) * k * 2.2) for c in range(3))
        draw.rounded_rectangle((i, i, w + i, h + i), r, fill=color)
    draw.rounded_rectangle((0, 0, w, h), r, fill=IVORY, outline=(170, 150, 110), width=s(3))
    face(draw, w / 2, h / 2)
    return img


def red_dragon(draw, cx, cy):
    draw.rounded_rectangle((cx - s(92), cy - s(78), cx + s(92), cy + s(78)), s(22), outline=RED, width=s(30))
    draw.rounded_rectangle((cx - s(17), cy - s(190), cx + s(17), cy + s(190)), s(17), fill=RED)


def circles(draw, cx, cy):
    for dx, dy, color in ((-80, -120, NAVY), (80, -120, RED), (0, 0, GREEN), (-80, 120, RED), (80, 120, NAVY)):
        x, y, rad = cx + s(dx), cy + s(dy), s(58)
        draw.ellipse((x - rad, y - rad, x + rad, y + rad), fill=NAVY)
        draw.ellipse((x - rad * 0.82, y - rad * 0.82, x + rad * 0.82, y + rad * 0.82), fill=IVORY)
        draw.ellipse((x - rad * 0.62, y - rad * 0.62, x + rad * 0.62, y + rad * 0.62), fill=color)
        draw.ellipse((x - rad * 0.24, y - rad * 0.24, x + rad * 0.24, y + rad * 0.24), fill=IVORY)


def place(canvas, layer, center, angle):
    """Pega una ficha girada, con su sombra, centrada en `center`."""
    rotated = layer.rotate(angle, resample=Image.BICUBIC, expand=True)
    shadow = Image.new("RGBA", rotated.size, (0, 0, 0, 0))
    shadow.paste((20, 0, 0, 150), (0, 0), rotated.split()[3])
    shadow = shadow.filter(ImageFilter.GaussianBlur(s(20)))
    x, y = int(center[0] - rotated.width / 2), int(center[1] - rotated.height / 2)
    canvas.alpha_composite(shadow, (x + s(18), y + s(26)))
    canvas.alpha_composite(rotated, (x, y))


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    icon = background()
    place(icon, tile(circles), (s(410), s(500)), 13)
    place(icon, tile(red_dragon), (s(620), s(545)), -9)
    icon = icon.resize((1024, 1024), Image.LANCZOS)
    icon.save(OUT / "icon.png")
    icon.save(OUT / "icon.ico", sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])

    if sys.platform == "darwin" and shutil.which("iconutil"):
        with tempfile.TemporaryDirectory() as tmp:
            iconset = Path(tmp) / "icon.iconset"
            iconset.mkdir()
            for size in (16, 32, 128, 256, 512):
                for scale in (1, 2):
                    name = f"icon_{size}x{size}{'@2x' if scale == 2 else ''}.png"
                    icon.resize((size * scale, size * scale), Image.LANCZOS).save(iconset / name)
            subprocess.run(["iconutil", "-c", "icns", str(iconset), "-o", str(OUT / "icon.icns")], check=True)
    print("Logo generado en", OUT)


if __name__ == "__main__":
    main()
