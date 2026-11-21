// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <vector>

#include "raylib.h"

namespace nm {

// Ambiente de un nivel: colores y estilo del fondo, y carácter de la música.
struct Theme {
  Vector3 a, b, c;  // paleta del fondo
  int mode;         // 0 aurora, 1 agua, 2 noche estrellada, 3 atardecer
  int mood;         // escala y tempo de la música
  Color mote;       // color de las motas que flotan
};

// Fondo animado a pantalla completa (shader) con motas flotando por encima.
class Background {
 public:
  void init();
  void shutdown();
  void draw(const Theme& theme, double time, float dt);

 private:
  struct Mote {
    Vector2 pos, vel;
    float size, phase, spin;
  };

  Shader shader_ = {};
  int locTime_ = -1, locRes_ = -1, locA_ = -1, locB_ = -1, locC_ = -1, locMode_ = -1;
  std::vector<Mote> motes_;
};

}  // namespace nm
