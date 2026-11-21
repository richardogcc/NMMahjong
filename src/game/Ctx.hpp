// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include "audio/Audio.hpp"
#include "game/Save.hpp"
#include "raylib.h"
#include "render/Background.hpp"
#include "render/TileAtlas.hpp"

namespace nm {

// Todo se coloca sobre un lienzo virtual de 1600 x 900 que se escala a la ventana.
constexpr float kViewW = 1600.0f;
constexpr float kViewH = 900.0f;

// Lo que comparten las pantallas del juego en cada fotograma.
struct Ctx {
  Font title = {};
  Font ui = {};
  Audio* audio = nullptr;
  SaveData* save = nullptr;
  const TileAtlas* atlas = nullptr;

  Vector2 mouse = {};  // en coordenadas del lienzo virtual
  bool click = false;
  float dt = 0.0f;
  double time = 0.0;
  bool langChanged = false;  // el menú lo activa al cambiar de idioma
};

const Theme& menuTheme();
const Theme& levelTheme(int levelIndex);

}  // namespace nm
