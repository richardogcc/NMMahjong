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

// Dibuja el símbolo de una cara dentro del rectángulo de 160 x 210 que empieza en `origin`.
void drawTileFace(int face, Vector2 origin, const Font& font, const char* westLetter);

// Todas las fichas se dibujan una sola vez en una textura y luego se pintan
// como imágenes: así escalan y giran con suavidad.
class TileAtlas {
 public:
  void build(const Font& font, const char* westLetter);
  void unload();

  // `center` es el centro de la cara; `faceWidth` su ancho en pantalla.
  void draw(int face, Vector2 center, float faceWidth, float rotation, Color tint) const;

 private:
  Texture2D texture_ = {};
};

}  // namespace nm
