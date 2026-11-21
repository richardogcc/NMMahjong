// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include "audio/Audio.hpp"
#include "core/Season.hpp"
#include "game/Save.hpp"
#include "raylib.h"
#include "render/Background.hpp"
#include "render/TileAtlas.hpp"
#include "ui/Input.hpp"
#include "update/Updater.hpp"

namespace nm {

// Todo se coloca sobre un lienzo virtual de 1600 x 900 que se escala a la ventana.
constexpr float kViewW = 1600.0f;
constexpr float kViewH = 900.0f;

// Lo que comparten las pantallas del juego en cada fotograma.
struct Ctx {
  Font title = {};  // rótulos grandes
  Font ui = {};     // texto corriente
  Font logo = {};   // siempre latina: el título del juego y las cifras de las fichas
  Audio* audio = nullptr;
  SaveData* save = nullptr;
  const TileAtlas* atlas = nullptr;
  Updater* updater = nullptr;

  Vector2 mouse = {};  // en coordenadas del lienzo virtual
  bool click = false;
  float dt = 0.0f;
  double time = 0.0;
  Nav nav;                   // teclado y mando
  bool langChanged = false;  // el menú lo activa al cambiar de idioma
  bool tilesChanged = false; // o el juego de fichas
};

// Con una temporada activa cambian el fondo del menú y el canto de las fichas.
void setSeason(Season season);
Season season();
Color tileEdge();
const Theme& menuTheme();
const Theme& levelTheme(int levelIndex);

}  // namespace nm
