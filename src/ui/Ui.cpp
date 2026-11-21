// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "ui/Ui.hpp"

#include <cmath>

namespace nm::ui {

float textWidth(const Font& font, const char* str, float size) {
  return MeasureTextEx(font, str, size, 0.0f).x;
}

void text(const Font& font, const char* str, float x, float y, float size, Color color) {
  DrawTextEx(font, str, {x, y}, size, 0.0f, color);
}

void textCentered(const Font& font, const char* str, float cx, float y, float size, Color color) {
  text(font, str, cx - textWidth(font, str, size) * 0.5f, y, size, color);
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
