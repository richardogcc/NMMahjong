// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Pruebas de la lógica del tablero. Sin dependencias: cada fallo se cuenta y
// el proceso devuelve un código distinto de cero si hay alguno.
#include <cstdio>
#include <cstdlib>
#include <set>
#include <tuple>

#include "core/Board.hpp"

namespace {

int failures = 0;

#define CHECK(cond, ...)                         \
  do {                                           \
    if (!(cond)) {                               \
      ++failures;                                \
      std::printf("FALLO %s:%d  ", __FILE__, __LINE__); \
      std::printf(__VA_ARGS__);                  \
      std::printf("\n");                         \
    }                                            \
  } while (0)

bool supported(const nm::Layout& layout, const nm::Slot& s) {
  if (s.z == 0) return true;
  for (const nm::Slot& o : layout.slots)
    if (o.z == s.z - 1 && std::abs(o.x - s.x) < 2 && std::abs(o.y - s.y) < 2) return true;
  return false;
}

void testLayoutShape(const nm::Layout& layout) {
  const char* id = layout.id.c_str();
  CHECK(layout.slots.size() % 2 == 0, "%s: número impar de fichas (%zu)", id, layout.slots.size());
  CHECK(layout.slots.size() <= 144, "%s: más de 144 fichas", id);
  std::set<std::tuple<int, int, int>> seen;
  for (const nm::Slot& s : layout.slots) {
    CHECK(seen.insert({s.x, s.y, s.z}).second, "%s: hueco repetido (%d,%d,%d)", id, s.x, s.y, s.z);
    CHECK(supported(layout, s), "%s: ficha en el aire (%d,%d,%d)", id, s.x, s.y, s.z);
    for (const nm::Slot& o : layout.slots) {
      const bool same = o.x == s.x && o.y == s.y;
      if (o.z == s.z && !same)
        CHECK(std::abs(o.x - s.x) >= 2 || std::abs(o.y - s.y) >= 2,
              "%s: fichas solapadas (%d,%d,%d)", id, s.x, s.y, s.z);
    }
  }
}

void testSolvable(const nm::Layout& layout) {
  const char* id = layout.id.c_str();
  for (std::uint32_t seed = 1; seed <= 25; ++seed) {
    nm::Board board;
    CHECK(board.generate(layout, seed), "%s: no se pudo generar (semilla %u)", id, seed);

    std::multiset<int> groups;
    for (const nm::Tile& t : board.tiles()) groups.insert(nm::matchGroup(t.face));
    for (int g : std::set<int>(groups.begin(), groups.end()))
      CHECK(groups.count(g) % 2 == 0, "%s: grupo %d sin pareja", id, g);

    for (const auto& [a, b] : board.solution())
      CHECK(board.removePair(a, b), "%s: la solución no es válida (semilla %u)", id, seed);
    CHECK(board.cleared(), "%s: quedan fichas tras la solución (semilla %u)", id, seed);
  }
}

void testRules() {
  const nm::Layout& layout = nm::allLayouts().front();
  nm::Board board;
  board.generate(layout, 7);
  const int total = board.remaining();

  // Una ficha tapada o encajonada no se puede usar.
  int blocked = -1;
  for (int i = 0; i < total; ++i)
    if (!board.isFree(i)) blocked = i;
  CHECK(blocked >= 0, "debería haber alguna ficha bloqueada al empezar");
  CHECK(!board.canMatch(blocked, blocked), "una ficha no empareja consigo misma");

  const auto hint = board.findHint();
  CHECK(hint.has_value(), "un tablero recién generado tiene movimientos");
  CHECK(board.availableMoves() > 0, "availableMoves debe ser positivo");
  if (hint) {
    CHECK(board.removePair(hint->first, hint->second), "la pista debe ser un movimiento válido");
    CHECK(board.remaining() == total - 2, "retirar una pareja quita dos fichas");
    CHECK(!board.removePair(hint->first, hint->second), "no se retira dos veces la misma pareja");
    CHECK(board.undo().has_value(), "debe poder deshacerse");
    CHECK(board.remaining() == total, "deshacer devuelve las dos fichas");
    CHECK(!board.undo().has_value(), "sin historial no hay nada que deshacer");
  }

  // Barajar conserva las fichas y deja movimientos.
  for (int i = 0; i < 4; ++i) {
    const auto h = board.findHint();
    if (h) board.removePair(h->first, h->second);
  }
  const int before = board.remaining();
  std::multiset<int> facesBefore, facesAfter;
  for (const nm::Tile& t : board.tiles())
    if (!t.removed) facesBefore.insert(nm::matchGroup(t.face));
  CHECK(board.shuffle(99), "barajar debe dejar movimientos");
  for (const nm::Tile& t : board.tiles())
    if (!t.removed) facesAfter.insert(nm::matchGroup(t.face));
  CHECK(board.remaining() == before, "barajar no cambia el número de fichas");
  CHECK(facesBefore == facesAfter, "barajar conserva las mismas fichas");
  CHECK(board.availableMoves() > 0, "tras barajar hay movimientos");
  CHECK(!board.canUndo(), "barajar vacía el historial");
}

}  // namespace

int main() {
  CHECK(nm::allLayouts().size() >= 8, "se esperan al menos 8 niveles");
  CHECK(nm::matchGroup(34) == nm::matchGroup(37), "las flores emparejan entre sí");
  CHECK(nm::matchGroup(38) == nm::matchGroup(41), "las estaciones emparejan entre sí");
  CHECK(nm::matchGroup(34) != nm::matchGroup(38), "flores y estaciones no se mezclan");
  for (const nm::Layout& layout : nm::allLayouts()) {
    testLayoutShape(layout);
    testSolvable(layout);
    std::printf("%-10s %3zu fichas\n", layout.id.c_str(), layout.slots.size());
  }
  testRules();
  std::printf(failures == 0 ? "Todas las pruebas correctas\n" : "%d fallos\n", failures);
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
