// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <string>
#include <vector>

namespace nm {

// Número de caras distintas: 0-8 círculos, 9-17 bambúes, 18-26 caracteres,
// 27-30 vientos, 31-33 dragones, 34-37 flores, 38-41 estaciones.
constexpr int kFaceCount = 42;

// Dos fichas hacen pareja si comparten grupo. Las flores emparejan entre sí,
// igual que las estaciones.
constexpr int matchGroup(int face) {
  if (face >= 38) return 35;
  if (face >= 34) return 34;
  return face;
}

// Posición en medias fichas: una ficha ocupa 2x2 unidades en (x, y) y una capa en z.
struct Slot {
  int x = 0;
  int y = 0;
  int z = 0;
};

struct Layout {
  std::string id;  // identificador estable (se usa en la partida guardada)
  std::vector<Slot> slots;
};

const std::vector<Layout>& allLayouts();

}  // namespace nm
