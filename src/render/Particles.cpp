// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "render/Particles.hpp"

#include <cmath>

namespace nm {
namespace {

float frand(float lo, float hi) {
  return lo + (hi - lo) * static_cast<float>(GetRandomValue(0, 10000)) / 10000.0f;
}

}  // namespace

void Particles::burst(Vector2 at, Color color, int count, float speed) {
  for (int i = 0; i < count; ++i) {
    const float angle = frand(0.0f, 6.2832f);
    const float v = speed * frand(0.25f, 1.0f);
    const bool spark = i % 3 == 0;
    const float life = frand(0.45f, 1.0f);
    items_.push_back({at,
                      {std::cos(angle) * v, std::sin(angle) * v - speed * 0.15f},
                      life,
                      life,
                      spark ? frand(7.0f, 13.0f) : frand(3.0f, 7.0f),
                      frand(0.0f, 360.0f),
                      frand(-240.0f, 240.0f),
                      i % 4 == 0 ? WHITE : color,
                      spark ? Shape::Spark : Shape::Glow});
  }
}

void Particles::confetti(Vector2 at, int count) {
  const Color palette[] = {{255, 99, 132, 255}, {255, 205, 86, 255}, {75, 192, 192, 255},
                           {54, 162, 235, 255}, {186, 104, 200, 255}, {255, 255, 255, 255}};
  for (int i = 0; i < count; ++i) {
    const float angle = frand(-2.6f, -0.55f);
    const float v = frand(280.0f, 760.0f);
    const float life = frand(1.6f, 2.8f);
    items_.push_back({at,
                      {std::cos(angle) * v, std::sin(angle) * v},
                      life,
                      life,
                      frand(7.0f, 12.0f),
                      frand(0.0f, 360.0f),
                      frand(-520.0f, 520.0f),
                      palette[GetRandomValue(0, 5)],
                      Shape::Confetti});
  }
}

void Particles::update(float dt) {
  for (Item& it : items_) {
    it.life -= dt;
    const float gravity = it.shape == Shape::Confetti ? 620.0f : 180.0f;
    const float drag = it.shape == Shape::Confetti ? 0.8f : 2.2f;
    it.vel.y += gravity * dt;
    it.vel.x -= it.vel.x * drag * dt;
    it.vel.y -= it.vel.y * drag * dt * 0.5f;
    it.pos.x += it.vel.x * dt;
    it.pos.y += it.vel.y * dt;
    it.rot += it.spin * dt;
  }
  std::erase_if(items_, [](const Item& it) { return it.life <= 0.0f; });
}

void Particles::draw() const {
  BeginBlendMode(BLEND_ADDITIVE);
  for (const Item& it : items_) {
    if (it.shape == Shape::Confetti) continue;
    const float k = it.life / it.maxLife;
    if (it.shape == Shape::Glow) {
      DrawCircleV(it.pos, it.size * (0.6f + k), Fade(it.color, k * 0.35f));
      DrawCircleV(it.pos, it.size * 0.45f * k, Fade(WHITE, k));
    } else {
      // Destello de cuatro puntas.
      const float r = it.size * (0.4f + k);
      for (int arm = 0; arm < 2; ++arm) {
        const float a = (it.rot + arm * 90.0f) * DEG2RAD;
        const Vector2 d = {std::cos(a) * r, std::sin(a) * r};
        DrawLineEx({it.pos.x - d.x, it.pos.y - d.y}, {it.pos.x + d.x, it.pos.y + d.y},
                   2.2f * k + 0.6f, Fade(it.color, k));
      }
    }
  }
  EndBlendMode();
  for (const Item& it : items_) {
    if (it.shape != Shape::Confetti) continue;
    const float k = std::min(1.0f, it.life / 0.5f);
    const float flip = std::fabs(std::sin(it.rot * DEG2RAD * 1.7f));
    DrawRectanglePro({it.pos.x, it.pos.y, it.size, it.size * (0.25f + 0.5f * flip)},
                     {it.size * 0.5f, it.size * 0.25f}, it.rot, Fade(it.color, k));
  }
}

}  // namespace nm
