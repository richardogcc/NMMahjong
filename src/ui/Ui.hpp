// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include "game/Ctx.hpp"

namespace nm::ui {

constexpr Color kText = {245, 245, 250, 255};
constexpr Color kMuted = {190, 200, 220, 255};
constexpr Color kGold = {255, 205, 86, 255};

float textWidth(const Font& font, const char* text, float size);
void text(const Font& font, const char* str, float x, float y, float size, Color color);
void textCentered(const Font& font, const char* str, float cx, float y, float size, Color color);

// Texto con salto de línea automático. Devuelve la altura que ha ocupado.
float textWrapped(const Font& font, const char* str, float x, float y, float width, float size, Color color,
                  bool centered);

void panel(Rectangle rect, float alpha = 0.72f);
bool hovered(const Ctx& ctx, Rectangle rect);
// Botón inmediato: lo dibuja y devuelve true si se ha pulsado en este fotograma.
bool button(const Ctx& ctx, Rectangle rect, const char* label, bool enabled = true,
            float fontSize = 24.0f);
void star(float cx, float cy, float radius, Color color);

}  // namespace nm::ui
