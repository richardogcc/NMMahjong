// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include "raylib.h"

namespace nm {

// Medidas de una ficha en unidades de diseño (la cara mide 160 x 210).
namespace tile {
constexpr float kFaceW = 160.0f;
constexpr float kFaceH = 210.0f;
constexpr float kDepth = 28.0f;  // grosor visible por la derecha y por abajo
constexpr float kAspect = kFaceH / kFaceW;
constexpr float kDepthRatio = kDepth / kFaceW;
}  // namespace tile

enum class TileSet { Modern = 0, Classic = 1, Contrast = 2, ColorBlind = 3 };

// Lo necesario para dibujar las caras de un juego de fichas.
struct TileStyle {
  TileSet set = TileSet::Modern;
  const Font* font = nullptr;     // números y letras
  const Font* cjk = nullptr;      // caracteres chinos, para el juego clásico
  const char* westLetter = "O";   // inicial de «oeste» en el idioma actual
  Color edge = {46, 150, 118, 255};  // color del canto: jade, salvo en temporada
};

// Dibuja el símbolo de una cara dentro del rectángulo de 160 x 210 que empieza en `origin`.
void drawTileFace(int face, Vector2 origin, const TileStyle& style);

// Todas las fichas se dibujan una sola vez en una textura y luego se pintan
// como imágenes: así escalan y giran con suavidad.
class TileAtlas {
 public:
  void build(const TileStyle& style);
  void unload();

  // `center` es el centro de la cara; `faceWidth` su ancho en pantalla.
  void draw(int face, Vector2 center, float faceWidth, float rotation, Color tint) const;

 private:
  Texture2D texture_ = {};
};

}  // namespace nm
