// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Dibujo de las 42 caras con formas simples: no se usa ninguna imagen externa.
#include <initializer_list>
#include <cmath>
#include <cstdio>

#include "render/TileAtlas.hpp"

namespace nm {
namespace {

constexpr Color kNavy = {28, 52, 110, 255};
constexpr Color kRed = {204, 44, 54, 255};
constexpr Color kGreen = {28, 138, 84, 255};
constexpr Color kBlue = {40, 112, 204, 255};
constexpr Color kGold = {236, 172, 40, 255};
constexpr Color kCream = {252, 247, 233, 255};

// Reparto de símbolos por filas para los números del 1 al 9.
constexpr int kRowsDots[9][3] = {{1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {2, 2, 0}, {2, 1, 2},
                                 {2, 2, 2}, {2, 3, 2}, {3, 2, 3}, {3, 3, 3}};
constexpr int kRowsBamboo[9][3] = {{1, 0, 0}, {1, 1, 0}, {1, 2, 0}, {2, 2, 0}, {2, 1, 2},
                                   {3, 3, 0}, {1, 3, 3}, {3, 2, 3}, {3, 3, 3}};

int rowCount(const int rows[3]) { return (rows[0] > 0) + (rows[1] > 0) + (rows[2] > 0); }

void centeredText(const Font& font, const char* text, float cx, float cy, float size, Color color) {
  const Vector2 m = MeasureTextEx(font, text, size, 0.0f);
  DrawTextEx(font, text, {cx - m.x * 0.5f, cy - m.y * 0.5f}, size, 0.0f, color);
}

void dot(float x, float y, float r, Color color) {
  DrawCircleV({x, y}, r, kNavy);
  DrawCircleV({x, y}, r * 0.84f, kCream);
  DrawCircleV({x, y}, r * 0.66f, color);
  DrawCircleV({x, y}, r * 0.28f, kCream);
}

void dots(int n, float cx, float cy) {
  if (n == 1) {
    dot(cx, cy, 54.0f, kRed);
    DrawRing({cx, cy}, 16.0f, 22.0f, 0.0f, 360.0f, 40, kGreen);
    return;
  }
  const int* rows = kRowsDots[n - 1];
  const int count = rowCount(rows);
  const float r = count == 3 ? 23.0f : (n == 2 ? 34.0f : 27.0f);
  const float stepY = count == 3 ? 60.0f : 82.0f;
  const Color colors[3] = {kBlue, kRed, kGreen};
  for (int row = 0; row < count; ++row) {
    const float y = cy + (row - (count - 1) * 0.5f) * stepY;
    for (int i = 0; i < rows[row]; ++i) {
      float x = cx + (i - (rows[row] - 1) * 0.5f) * (rows[row] == 3 ? 46.0f : 62.0f);
      if (n == 3) x = cx + (row - 1) * 40.0f;  // el tres va en diagonal
      dot(x, y, rows[row] == 3 ? 21.0f : r, colors[(row + i) % 3]);
    }
  }
}

void stick(float x, float y, float w, float h, Color color) {
  DrawRectangleRounded({x - w * 0.5f, y - h * 0.5f, w, h}, 0.9f, 8, color);
  DrawRectangleRounded({x - w * 0.5f + 3, y - h * 0.5f + 4, w * 0.28f, h - 8}, 0.9f, 6,
                       Fade(WHITE, 0.45f));
  for (float k : {-0.22f, 0.22f})
    DrawRectangleRec({x - w * 0.5f, y + h * k - 1.5f, w, 3.0f}, Fade(kCream, 0.9f));
}

void bamboo(int n, float cx, float cy) {
  if (n == 1) {
    stick(cx, cy + 6.0f, 30.0f, 138.0f, kGreen);
    DrawCircleV({cx, cy - 78.0f}, 13.0f, kRed);
    DrawEllipse(static_cast<int>(cx - 30), static_cast<int>(cy - 34), 20.0f, 8.0f, kGreen);
    DrawEllipse(static_cast<int>(cx + 30), static_cast<int>(cy + 10), 20.0f, 8.0f, kGreen);
    return;
  }
  const int* rows = kRowsBamboo[n - 1];
  const int count = rowCount(rows);
  const float h = count == 3 ? 50.0f : 74.0f;
  const float stepY = count == 3 ? 60.0f : 86.0f;
  for (int row = 0; row < count; ++row) {
    const float y = cy + (row - (count - 1) * 0.5f) * stepY;
    for (int i = 0; i < rows[row]; ++i) {
      const float x = cx + (i - (rows[row] - 1) * 0.5f) * 40.0f;
      const bool accent = rows[row] == 1 && count > 1;
      stick(x, y, 19.0f, h, accent ? kRed : (row == 1 ? kBlue : kGreen));
    }
  }
}

void character(int n, float cx, float cy, const Font& font) {
  char label[4];
  std::snprintf(label, sizeof(label), "%d", n);
  centeredText(font, label, cx, cy - 36.0f, 118.0f, kNavy);
  DrawRectangleRounded({cx - 36.0f, cy + 38.0f, 72.0f, 50.0f}, 0.35f, 8, kRed);
  DrawPoly({cx, cy + 63.0f}, 4, 15.0f, 0.0f, kCream);
  DrawPoly({cx, cy + 63.0f}, 4, 7.0f, 0.0f, kRed);
}

void wind(int index, float cx, float cy, const Font& font, const char* westLetter) {
  const char* letters[4] = {"N", "S", "E", westLetter};
  const float angles[4] = {-90.0f, 90.0f, 0.0f, 180.0f};  // hacia dónde apunta la flecha
  const Vector2 compass = {cx, cy - 56.0f};
  DrawRing(compass, 24.0f, 29.0f, 0.0f, 360.0f, 40, kBlue);
  DrawPoly(compass, 3, 17.0f, angles[index], kRed);
  centeredText(font, letters[index], cx, cy + 34.0f, 122.0f, kNavy);
}

void dragon(int index, float cx, float cy) {
  if (index == 0) {  // rojo
    DrawRectangleRoundedLinesEx({cx - 36.0f, cy - 30.0f, 72.0f, 60.0f}, 0.2f, 8, 11.0f, kRed);
    DrawRectangleRounded({cx - 7.0f, cy - 78.0f, 14.0f, 156.0f}, 0.9f, 8, kRed);
  } else if (index == 1) {  // verde
    for (int i = 0; i < 3; ++i) {
      const float y = cy - 46.0f + i * 40.0f;
      DrawLineEx({cx - 44.0f, y + 24.0f}, {cx, y - 10.0f}, 13.0f, kGreen);
      DrawLineEx({cx + 44.0f, y + 24.0f}, {cx, y - 10.0f}, 13.0f, kGreen);
      DrawCircleV({cx, y - 10.0f}, 6.5f, kGreen);
    }
    DrawCircleV({cx, cy + 72.0f}, 9.0f, kGold);
  } else {  // blanco
    DrawRectangleRoundedLinesEx({cx - 50.0f, cy - 72.0f, 100.0f, 144.0f}, 0.14f, 8, 9.0f, kBlue);
    DrawRectangleRoundedLinesEx({cx - 31.0f, cy - 53.0f, 62.0f, 106.0f}, 0.12f, 8, 3.0f, kBlue);
    for (float sx : {-1.0f, 1.0f})
      for (float sy : {-1.0f, 1.0f}) DrawCircleV({cx + sx * 50.0f, cy + sy * 72.0f}, 8.0f, kNavy);
  }
}

void cornerNumber(int n, Vector2 o, const Font& font, Color color) {
  char label[4];
  std::snprintf(label, sizeof(label), "%d", n);
  centeredText(font, label, o.x + 132.0f, o.y + 178.0f, 42.0f, color);
}

void flower(int index, float cx, float cy) {
  const Color petals[4] = {{240, 98, 146, 255}, {245, 150, 50, 255}, {156, 96, 208, 255}, kRed};
  DrawLineEx({cx, cy + 4.0f}, {cx - 10.0f, cy + 82.0f}, 6.0f, kGreen);
  DrawEllipse(static_cast<int>(cx - 30), static_cast<int>(cy + 60), 18.0f, 8.0f, kGreen);
  for (int i = 0; i < 5; ++i) {
    const float a = (i * 72.0f - 90.0f) * DEG2RAD;
    DrawCircleV({cx + std::cos(a) * 30.0f, cy - 16.0f + std::sin(a) * 30.0f}, 24.0f, petals[index]);
  }
  DrawCircleV({cx, cy - 16.0f}, 16.0f, kGold);
  DrawCircleV({cx, cy - 16.0f}, 7.0f, kCream);
}

void season(int index, float cx, float cy) {
  const float y = cy - 14.0f;
  if (index == 0) {  // primavera: brote
    DrawLineEx({cx, y + 70.0f}, {cx, y - 20.0f}, 8.0f, kGreen);
    DrawEllipse(static_cast<int>(cx - 26), static_cast<int>(y - 22), 28.0f, 13.0f, kGreen);
    DrawEllipse(static_cast<int>(cx + 28), static_cast<int>(y + 14), 28.0f, 13.0f, {96, 180, 80, 255});
    DrawCircleV({cx, y - 44.0f}, 12.0f, {240, 98, 146, 255});
  } else if (index == 1) {  // verano: sol
    for (int i = 0; i < 12; ++i) {
      const float a = i * 30.0f * DEG2RAD;
      DrawLineEx({cx + std::cos(a) * 40.0f, y + std::sin(a) * 40.0f},
                 {cx + std::cos(a) * 62.0f, y + std::sin(a) * 62.0f}, 7.0f, kGold);
    }
    DrawCircleV({cx, y}, 33.0f, {245, 150, 50, 255});
    DrawCircleV({cx, y}, 24.0f, kGold);
  } else if (index == 2) {  // otoño: hoja
    DrawLineEx({cx, y + 14.0f}, {cx + 8.0f, y + 78.0f}, 6.0f, {130, 80, 40, 255});
    DrawPoly({cx, y - 6.0f}, 5, 52.0f, -90.0f, {214, 92, 36, 255});
    DrawPoly({cx, y - 6.0f}, 5, 30.0f, -90.0f, {245, 150, 50, 255});
    DrawLineEx({cx, y - 50.0f}, {cx, y + 30.0f}, 3.0f, {130, 80, 40, 255});
  } else {  // invierno: copo
    for (int i = 0; i < 3; ++i) {
      const float a = (i * 60.0f + 30.0f) * DEG2RAD;
      const Vector2 d = {std::cos(a) * 60.0f, std::sin(a) * 60.0f};
      DrawLineEx({cx - d.x, y - d.y}, {cx + d.x, y + d.y}, 8.0f, kBlue);
      for (float s : {-1.0f, 1.0f}) DrawCircleV({cx + d.x * s, y + d.y * s}, 8.0f, kBlue);
    }
    DrawPoly({cx, y}, 6, 17.0f, 0.0f, kCream);
    DrawPoly({cx, y}, 6, 10.0f, 0.0f, kBlue);
  }
}

// --- Juego clásico: caracteres chinos de verdad ---------------------------
const char* const kNumerals[9] = {"一", "二", "三", "四", "五", "六", "七", "八", "九"};
const char* const kWindsCjk[4] = {"北", "南", "東", "西"};
const char* const kFlowersCjk[4] = {"梅", "蘭", "菊", "竹"};
const char* const kSeasonsCjk[4] = {"春", "夏", "秋", "冬"};
constexpr Color kInk = {24, 24, 28, 255};

bool classicFace(int face, Vector2 o, float cx, float cy, const TileStyle& style) {
  if (!style.cjk || style.cjk->texture.id == 0) return false;
  const Font& cjk = *style.cjk;
  if (face >= 18 && face < 27) {
    centeredText(cjk, kNumerals[face - 18], cx, cy - 46.0f, 96.0f, kInk);
    centeredText(cjk, "萬", cx, cy + 50.0f, 96.0f, kRed);
  } else if (face >= 27 && face < 31) {
    centeredText(cjk, kWindsCjk[face - 27], cx, cy, 136.0f, kInk);
  } else if (face == 31) {
    centeredText(cjk, "中", cx, cy, 140.0f, kRed);
  } else if (face == 32) {
    centeredText(cjk, "發", cx, cy, 136.0f, kGreen);
  } else if (face >= 34 && face < 38) {
    centeredText(cjk, kFlowersCjk[face - 34], cx, cy - 8.0f, 118.0f, kRed);
    cornerNumber(face - 33, o, *style.font, kNavy);
  } else if (face >= 38) {
    centeredText(cjk, kSeasonsCjk[face - 38], cx, cy - 8.0f, 118.0f, kGreen);
    cornerNumber(face - 37, o, *style.font, kNavy);
  } else {
    return false;  // círculos, bambúes y dragón blanco se dibujan igual que en el moderno
  }
  return true;
}

// --- Alto contraste: cifras enormes y un color por palo -------------------
void contrastFace(int face, float cx, float cy, const TileStyle& style) {
  const Font& font = *style.font;
  constexpr Color kDots = {0, 84, 200, 255}, kBams = {0, 128, 60, 255}, kChars = {200, 20, 30, 255};
  char label[4];
  if (face < 27) {
    const int suit = face / 9, number = face % 9 + 1;
    const Color color = suit == 0 ? kDots : (suit == 1 ? kBams : kChars);
    std::snprintf(label, sizeof(label), "%d", number);
    centeredText(font, label, cx, cy - 34.0f, 150.0f, color);
    // Marca del palo: círculo, barra o rombo.
    if (suit == 0) DrawCircleV({cx, cy + 66.0f}, 26.0f, color);
    if (suit == 1) DrawRectangleRounded({cx - 12.0f, cy + 34.0f, 24.0f, 64.0f}, 0.8f, 8, color);
    if (suit == 2) DrawPoly({cx, cy + 66.0f}, 4, 32.0f, 0.0f, color);
  } else if (face < 31) {
    const char* letters[4] = {"N", "S", "E", style.westLetter};
    centeredText(font, letters[face - 27], cx, cy, 170.0f, BLACK);
  } else if (face < 34) {
    const Color fills[3] = {kChars, kBams, WHITE};
    DrawRectangleRounded({cx - 56.0f, cy - 76.0f, 112.0f, 152.0f}, 0.2f, 8, face == 33 ? kDots : fills[face - 31]);
    if (face == 33) DrawRectangleRounded({cx - 40.0f, cy - 60.0f, 80.0f, 120.0f}, 0.2f, 8, WHITE);
  } else {
    const bool flower = face < 38;
    const Color color = flower ? Color{190, 0, 140, 255} : Color{220, 110, 0, 255};
    DrawCircleV({cx, cy - 44.0f}, 44.0f, color);
    if (!flower) DrawCircleV({cx, cy - 44.0f}, 22.0f, WHITE);
    std::snprintf(label, sizeof(label), "%d", face - (flower ? 33 : 37));
    centeredText(font, label, cx, cy + 52.0f, 96.0f, color);
  }
}

}  // namespace

void drawTileFace(int face, Vector2 origin, const TileStyle& style) {
  const float cx = origin.x + tile::kFaceW * 0.5f;
  const float cy = origin.y + tile::kFaceH * 0.5f;
  const Font& font = *style.font;
  if (style.set == TileSet::Contrast) {
    contrastFace(face, cx, cy, style);
    return;
  }
  if (style.set == TileSet::Classic && classicFace(face, origin, cx, cy, style)) return;

  if (face < 9) {
    dots(face + 1, cx, cy);
  } else if (face < 18) {
    bamboo(face - 8, cx, cy);
  } else if (face < 27) {
    character(face - 17, cx, cy, font);
  } else if (face < 31) {
    wind(face - 27, cx, cy, font, style.westLetter);
  } else if (face < 34) {
    dragon(face - 31, cx, cy);
  } else if (face < 38) {
    flower(face - 34, cx, cy);
    cornerNumber(face - 33, origin, font, kRed);
  } else {
    season(face - 38, cx, cy);
    cornerNumber(face - 37, origin, font, kGreen);
  }
}

}  // namespace nm
