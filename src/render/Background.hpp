// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "raylib.h"
#include "render/Video.hpp"

namespace nm {

enum class MoteKind { Petals, Leaves, Fireflies, Dust };

// Ambiente de un nivel: colores y estilo del fondo, y carácter de la música.
struct Theme {
  const char* photo;  // nombre del vídeo de assets/video (o foto de assets/backgrounds)
  Vector3 a, b, c;    // paleta: tinte de la bruma y fondo de reserva
  int mode;           // animación: 0 aurora, 1 agua, 2 noche estrellada, 3 bruma
  int mood;           // escala y tempo de la música
  float fog;          // cantidad de niebla en movimiento, de 0 a 1
  MoteKind motes;     // qué flota por delante
  Color mote;         // y de qué color
};

// Fondo animado a pantalla completa: un vídeo en bucle con niebla y motas
// flotando por encima. Si falta el vídeo se usa una fotografía animada por
// shader, y si tampoco hay foto, un fondo generado.
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
  Video* video(const char* name);

  Shader shader_ = {};
  int locTime_ = -1, locRes_ = -1, locA_ = -1, locB_ = -1, locC_ = -1, locMode_ = -1;
  int locPhoto_ = -1, locTexSize_ = -1, locFog_ = -1, locVideo_ = -1;
  std::string dir_, videoDir_;
  std::map<std::string, std::unique_ptr<Video>> videos_;  // nullptr: no existe, no se reintenta
  std::map<std::string, Texture2D> photos_;  // se cargan la primera vez que hacen falta
  Texture2D blank_ = {};
  std::vector<Mote> motes_;
};

}  // namespace nm
