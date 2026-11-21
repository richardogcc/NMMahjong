// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <map>
#include <string>
#include <vector>

#include "raylib.h"

namespace nm {

// Ambiente de un nivel: colores y estilo del fondo, y carácter de la música.
struct Theme {
  const char* photo;  // fotografía de assets/backgrounds, sin extensión
  Vector3 a, b, c;    // paleta: tinte de la bruma y fondo de reserva
  int mode;           // animación: 0 aurora, 1 agua, 2 noche estrellada, 3 bruma
  int mood;           // escala y tempo de la música
  Color mote;         // color de las motas que flotan
};

// Fondo animado a pantalla completa: una fotografía con movimiento lento de
// cámara y efectos por shader (reflejos en el agua, bruma, destellos), con
// motas flotando por encima. Si falta la foto se usa un fondo generado.
class Background {
 public:
  void init(const std::string& assetDir);
  void shutdown();
  void draw(const Theme& theme, double time, float dt);

 private:
  struct Mote {
    Vector2 pos, vel;
    float size, phase, spin;
  };

  const Texture2D& photo(const char* name);

  Shader shader_ = {};
  int locTime_ = -1, locRes_ = -1, locA_ = -1, locB_ = -1, locC_ = -1, locMode_ = -1;
  int locPhoto_ = -1, locTexSize_ = -1;
  std::string dir_;
  std::map<std::string, Texture2D> photos_;  // se cargan la primera vez que hacen falta
  Texture2D blank_ = {};
  std::vector<Mote> motes_;
};

}  // namespace nm
