// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <vector>

#include "raylib.h"

namespace nm {

class Particles {
 public:
  // Estallido de chispas luminosas, para una pareja retirada.
  void burst(Vector2 at, Color color, int count, float speed);
  // Confeti que cae, para la pantalla de victoria.
  void confetti(Vector2 at, int count);
  void update(float dt);
  void draw() const;
  void clear() { items_.clear(); }

 private:
  enum class Shape { Glow, Spark, Confetti };
  struct Item {
    Vector2 pos, vel;
    float life, maxLife, size, rot, spin;
    Color color;
    Shape shape;
  };
  std::vector<Item> items_;
};

}  // namespace nm
