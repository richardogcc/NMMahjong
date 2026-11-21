// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "core/Layout.hpp"

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

}  // namespace nm
