// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "ui/Ui.hpp"

#include <cmath>
#include <string>

namespace nm::ui {

namespace {
bool largeText = false;
}

void setLargeText(bool on) { largeText = on; }

// Solo crecen los textos pequeños: los títulos ya se leen bien y no hay sitio.
float fit(float size) { return largeText && size < 27.0f ? size * 1.17f : size; }

float textWidth(const Font& font, const char* str, float size) {
  return MeasureTextEx(font, str, fit(size), 0.0f).x;
}

void text(const Font& font, const char* str, float x, float y, float size, Color color) {
  const float real = fit(size);
  DrawTextEx(font, str, {x, y - (real - size) * 0.5f}, real, 0.0f, color);
}

void textCentered(const Font& font, const char* str, float cx, float y, float size, Color color) {
  text(font, str, cx - textWidth(font, str, size) * 0.5f, y, size, color);
}

float textWrapped(const Font& font, const char* str, float x, float y, float width, float size, Color color,
                  bool centered) {
  std::string line, word;
  float used = 0.0f;
  auto flush = [&]() {
    if (line.empty()) return;
    const float lineX = centered ? x + (width - textWidth(font, line.c_str(), size)) * 0.5f : x;
    text(font, line.c_str(), lineX, y + used, size, color);
    used += fit(size) * 1.32f;
    line.clear();
  };
  for (const char* c = str;; ++c) {
    if (*c == ' ' || *c == '\0') {
      const std::string candidate = line.empty() ? word : line + " " + word;
      if (!line.empty() && textWidth(font, candidate.c_str(), size) > width) {
        flush();
        line = word;
      } else {
        line = candidate;
      }
      word.clear();
      if (*c == '\0') break;
    } else {
      word.push_back(*c);
    }
  }
  flush();
  return used;
}

void panel(Rectangle rect, float alpha) {
  DrawRectangleRounded(rect, 0.12f, 12, Fade({12, 18, 34, 255}, alpha));
  DrawRectangleRoundedLinesEx(rect, 0.12f, 12, 2.0f, Fade(WHITE, 0.16f));
}

bool hovered(const Ctx& ctx, Rectangle rect) { return CheckCollisionPointRec(ctx.mouse, rect); }

bool button(const Ctx& ctx, Rectangle rect, const char* label, bool enabled, float fontSize) {
  const bool over = enabled && hovered(ctx, rect);
  if (over) rect = {rect.x - 2.0f, rect.y - 2.0f, rect.width + 4.0f, rect.height + 4.0f};
  const Color fill = over ? Color{70, 110, 190, 235} : Color{26, 36, 64, 215};
  const float alpha = enabled ? 1.0f : 0.38f;
  DrawRectangleRounded(rect, 0.4f, 10, Fade(fill, alpha * fill.a / 255.0f));
  DrawRectangleRoundedLinesEx(rect, 0.4f, 10, 2.0f, Fade(WHITE, (over ? 0.7f : 0.25f) * alpha));
  // Si el texto no cabe (idiomas de palabras largas, texto grande), se encoge.
  const float room = rect.width - 14.0f, wide = textWidth(ctx.ui, label, fontSize);
  if (wide > room) fontSize *= room / wide;
  textCentered(ctx.ui, label, rect.x + rect.width * 0.5f,
               rect.y + (rect.height - fontSize) * 0.5f, fontSize, Fade(kText, alpha));
  if (over && ctx.click) {
    ctx.audio->play(Sfx::Click);
    return true;
  }
  return false;
}

void star(float cx, float cy, float radius, Color color) {
  // Diez triángulos desde el centro; raylib exige orden antihorario.
  Vector2 pts[10];
  for (int i = 0; i < 10; ++i) {
    const float a = (-90.0f + i * 36.0f) * DEG2RAD;
    const float r = i % 2 == 0 ? radius : radius * 0.45f;
    pts[i] = {cx + std::cos(a) * r, cy + std::sin(a) * r};
  }
  for (int i = 0; i < 10; ++i) DrawTriangle({cx, cy}, pts[(i + 1) % 10], pts[i], color);
}

}  // namespace nm::ui
