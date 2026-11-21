// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "render/TileAtlas.hpp"

#include "core/Layout.hpp"

namespace nm {
namespace {

constexpr int kCols = 7;
constexpr int kRows = 6;
constexpr float kPad = 4.0f;
constexpr float kCellW = kPad + tile::kFaceW + tile::kDepth + 2.0f;
constexpr float kCellH = kPad + tile::kFaceH + tile::kDepth + 2.0f;
constexpr float kBake = 2.0f;  // se hornea al doble para que al reducir quede suave
constexpr float kRound = 0.17f;

unsigned char mix(int a, int b, float k) { return static_cast<unsigned char>(a + (b - a) * k); }

void drawBody(Vector2 o, Color edge) {
  // Canto: marfil junto a la cara y color (jade, normalmente) en la base.
  for (int i = static_cast<int>(tile::kDepth); i >= 1; --i) {
    const float k = i / tile::kDepth;
    Color c;
    if (i > 11) {
      c = {mix(edge.r, edge.r * 45 / 100, k), mix(edge.g, edge.g * 61 / 100, k), mix(edge.b, edge.b * 63 / 100, k), 255};
    } else {
      c = {mix(236, 205, k * 2.2f), mix(226, 190, k * 2.2f), mix(200, 160, k * 2.2f), 255};
    }
    DrawRectangleRounded({o.x + i, o.y + i, tile::kFaceW, tile::kFaceH}, kRound, 10, c);
  }
  const Rectangle faceRect = {o.x, o.y, tile::kFaceW, tile::kFaceH};
  DrawRectangleRounded(faceRect, kRound, 10, {252, 247, 233, 255});
  DrawRectangleRoundedLinesEx(faceRect, kRound, 10, 1.6f, {120, 96, 60, 110});
}

}  // namespace

void TileAtlas::build(const TileStyle& style) {
  unload();
  const int width = static_cast<int>(kCellW * kCols * kBake);
  const int height = static_cast<int>(kCellH * kRows * kBake);
  RenderTexture2D target = LoadRenderTexture(width, height);

  Camera2D cam = {};
  cam.zoom = kBake;
  BeginTextureMode(target);
  ClearBackground(BLANK);
  BeginMode2D(cam);
  for (int face = 0; face < kFaceCount; ++face) {
    const Vector2 origin = {(face % kCols) * kCellW + kPad, (face / kCols) * kCellH + kPad};
    drawBody(origin, style.edge);
    drawTileFace(face, origin, style);
  }
  EndMode2D();
  EndTextureMode();

  // La textura de un render target sale invertida: se pasa a una normal.
  Image image = LoadImageFromTexture(target.texture);
  ImageFlipVertical(&image);
  texture_ = LoadTextureFromImage(image);
  GenTextureMipmaps(&texture_);
  SetTextureFilter(texture_, TEXTURE_FILTER_TRILINEAR);
  UnloadImage(image);
  UnloadRenderTexture(target);
}

void TileAtlas::unload() {
  if (texture_.id != 0) UnloadTexture(texture_);
  texture_ = {};
}

void TileAtlas::draw(int face, Vector2 center, float faceWidth, float rotation, Color tint) const {
  const float k = faceWidth / tile::kFaceW;
  const Rectangle src = {(face % kCols) * kCellW * kBake, (face / kCols) * kCellH * kBake,
                         kCellW * kBake, kCellH * kBake};
  const Rectangle dst = {center.x, center.y, kCellW * k, kCellH * k};
  const Vector2 pivot = {(kPad + tile::kFaceW * 0.5f) * k, (kPad + tile::kFaceH * 0.5f) * k};
  DrawTexturePro(texture_, src, dst, pivot, rotation, tint);
}

}  // namespace nm
