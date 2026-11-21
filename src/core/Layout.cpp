// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "core/Layout.hpp"

#include <algorithm>
#include <random>

namespace nm {
namespace {

using Slots = std::vector<Slot>;

void rect(Slots& s, int x0, int y0, int cols, int rows, int z) {
  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < cols; ++c) s.push_back({x0 + 2 * c, y0 + 2 * r, z});
}

void ring(Slots& s, int cols, int rows, int z) {
  for (int r = 0; r < rows; ++r)
    for (int c = 0; c < cols; ++c)
      if (r == 0 || c == 0 || r == rows - 1 || c == cols - 1) s.push_back({2 * c, 2 * r, z});
}

Slots jardin() {
  Slots s;
  rect(s, 0, 0, 6, 4, 0);
  rect(s, 2, 2, 4, 2, 1);
  return s;
}

Slots piramide() {
  Slots s;
  rect(s, 0, 0, 8, 6, 0);
  rect(s, 2, 2, 6, 4, 1);
  rect(s, 4, 4, 4, 2, 2);
  rect(s, 6, 5, 2, 1, 3);
  return s;
}

Slots puente() {
  Slots s;
  for (int x0 : {0, 18}) {
    rect(s, x0, 0, 3, 6, 0);
    rect(s, x0 + 1, 1, 2, 5, 1);
    rect(s, x0 + 2, 2, 1, 4, 2);
  }
  rect(s, 6, 4, 6, 2, 0);
  rect(s, 6, 5, 6, 1, 1);
  return s;
}

Slots tortuga() {
  Slots s;
  const int first[8] = {1, 3, 2, 1, 1, 2, 3, 1};
  const int last[8] = {12, 10, 11, 12, 12, 11, 10, 12};
  for (int r = 0; r < 8; ++r)
    for (int c = first[r]; c <= last[r]; ++c) s.push_back({2 * c, 2 * r, 0});
  s.push_back({0, 7, 0});
  s.push_back({26, 7, 0});
  s.push_back({28, 7, 0});
  rect(s, 8, 2, 6, 6, 1);
  rect(s, 10, 4, 4, 4, 2);
  rect(s, 12, 6, 2, 2, 3);
  s.push_back({13, 7, 4});
  return s;
}

Slots fortaleza() {
  Slots s;
  ring(s, 12, 8, 0);
  ring(s, 12, 8, 1);
  rect(s, 8, 4, 4, 4, 0);
  rect(s, 8, 4, 4, 4, 1);
  rect(s, 10, 6, 2, 2, 2);
  for (int x : {0, 22})
    for (int y : {0, 14}) s.push_back({x, y, 2});
  return s;
}

Slots mariposa() {
  Slots s;
  for (int x0 : {0, 12}) {
    rect(s, x0, 0, 4, 6, 0);
    rect(s, x0 + 1, 1, 3, 5, 1);
    rect(s, x0 + 2, 2, 2, 4, 2);
  }
  rect(s, 8, 1, 2, 5, 0);
  rect(s, 9, 2, 1, 4, 1);
  return s;
}

Slots cruz() {
  Slots s;
  rect(s, 0, 6, 14, 3, 0);
  rect(s, 10, 0, 4, 3, 0);
  rect(s, 10, 12, 4, 3, 0);
  rect(s, 2, 7, 12, 2, 1);
  rect(s, 11, 1, 3, 2, 1);
  rect(s, 11, 13, 3, 2, 1);
  rect(s, 4, 8, 10, 1, 2);
  rect(s, 12, 8, 2, 1, 3);
  return s;
}

Slots templo() {
  Slots s;
  rect(s, 0, 0, 12, 7, 0);
  rect(s, 2, 2, 10, 5, 1);
  rect(s, 8, 5, 4, 2, 2);
  rect(s, 10, 6, 2, 1, 3);
  return s;
}

}  // namespace

const std::vector<Layout>& allLayouts() {
  static const std::vector<Layout> layouts = {
      {"jardin", jardin()},       {"piramide", piramide()}, {"puente", puente()},
      {"tortuga", tortuga()},     {"fortaleza", fortaleza()}, {"mariposa", mariposa()},
      {"cruz", cruz()},           {"templo", templo()},
  };
  return layouts;
}

Layout randomLayout(std::uint32_t seed) {
  std::mt19937 rng(seed);
  auto pick = [&rng](int lo, int hi) { return lo + static_cast<int>(rng() % static_cast<unsigned>(hi - lo + 1)); };
  constexpr int kMaxCols = 13, kMaxRows = 7, kLayers = 5;

  for (int attempt = 0; attempt < 400; ++attempt) {
    const int cols = pick(7, kMaxCols), rows = pick(4, kMaxRows);
    bool cell[kLayers][kMaxRows][kMaxCols] = {};

    // La planta es la unión de varios rectángulos centrados; cada uno sube
    // unas cuantas capas, estrechándose como una pirámide escalonada.
    const int blocks = pick(2, 4);
    for (int b = 0; b < blocks; ++b) {
      const int inX = pick(0, (cols - 1) / 2), inY = pick(0, (rows - 1) / 2);
      const int shiftY = pick(0, 2) == 0 ? pick(-1, 1) : 0;  // a veces se rompe la simetría vertical
      const int height = pick(2, 4);
      for (int z = 0; z < height; ++z) {
        const int c0 = inX + z, c1 = cols - 1 - inX - z;
        const int r0 = inY + z + shiftY, r1 = rows - 1 - inY - z + shiftY;
        for (int r = std::max(r0, 0); r <= std::min(r1, rows - 1); ++r)
          for (int c = c0; c <= c1; ++c) cell[z][r][c] = true;
      }
    }
    // Ninguna ficha puede quedar en el aire.
    for (int z = 1; z < kLayers; ++z)
      for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) cell[z][r][c] = cell[z][r][c] && cell[z - 1][r][c];

    Layout layout;
    layout.id = "aleatorio";
    for (int z = 0; z < kLayers; ++z)
      for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
          if (cell[z][r][c]) layout.slots.push_back({2 * c, 2 * r, z});
    if (layout.slots.size() % 2 != 0) layout.slots.pop_back();  // la última es de la capa más alta
    if (layout.slots.size() >= 48 && layout.slots.size() <= 144) return layout;
  }
  return allLayouts().front();  // no debería ocurrir; un tablero conocido como reserva
}

}  // namespace nm
