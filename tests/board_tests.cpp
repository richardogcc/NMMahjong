// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Pruebas de la lógica del tablero. Sin dependencias: cada fallo se cuenta y
// el proceso devuelve un código distinto de cero si hay alguno.
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <set>
#include <tuple>

#include "core/Board.hpp"
#include "core/Release.hpp"
#include "core/Season.hpp"

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

void testRandomLayouts() {
  std::set<std::size_t> sizes;
  for (std::uint32_t seed = 1; seed <= 300; ++seed) {
    const nm::Layout layout = nm::randomLayout(seed);
    CHECK(layout.slots.size() >= 48, "aleatorio %u: muy pocas fichas (%zu)", seed, layout.slots.size());
    testLayoutShape(layout);
    nm::Board board;
    CHECK(board.generate(layout, seed), "aleatorio %u: no se pudo repartir", seed);
    sizes.insert(layout.slots.size());
  }
  CHECK(sizes.size() >= 15, "los tableros aleatorios deberían variar más (%zu tamaños)", sizes.size());
}

void testDeterminismAndRestore() {
  // Mismo tablero y semilla: mismo reparto siempre (y en cualquier sistema).
  const nm::Layout& turtle = nm::allLayouts()[3];
  nm::Board a, b;
  a.generate(turtle, 20261121u);
  b.generate(turtle, 20261121u);
  bool same = true;
  unsigned long long sum = 0;
  for (std::size_t i = 0; i < a.tiles().size(); ++i) {
    same = same && a.tiles()[i].face == b.tiles()[i].face;
    sum = sum * 31 + static_cast<unsigned long long>(a.tiles()[i].face);
  }
  CHECK(same, "dos repartos con la misma semilla deben coincidir");
  CHECK(sum == 8511652726526134804ull, "el reparto de referencia ha cambiado: %llu", sum);

  // Difícil sigue teniendo solución.
  for (std::uint32_t seed = 1; seed <= 20; ++seed) {
    nm::Board hard;
    CHECK(hard.generate(turtle, seed, nm::Difficulty::Hard), "difícil: no se pudo repartir");
    for (const auto& [x, y] : hard.solution()) CHECK(hard.removePair(x, y), "difícil: solución inválida");
    CHECK(hard.cleared(), "difícil: quedan fichas");
  }

  // Guardar y recuperar una partida a medias.
  for (int i = 0; i < 6; ++i) {
    const auto h = a.findHint();
    if (h) a.removePair(h->first, h->second);
  }
  std::vector<int> faces, removed;
  for (const nm::Tile& t : a.tiles()) {
    faces.push_back(t.face);
    removed.push_back(t.removed ? 1 : 0);
  }
  nm::Board c;
  CHECK(c.restore(turtle, faces, removed, a.history()), "debe recuperar una partida válida");
  CHECK(c.remaining() == a.remaining() && c.availableMoves() == a.availableMoves(), "la partida recuperada coincide");
  CHECK(c.undo().has_value() && c.remaining() == a.remaining() + 2, "la partida recuperada conserva el deshacer");
  faces[0] = 99;
  CHECK(!c.restore(turtle, faces, removed, {}), "rechaza caras fuera de rango");
  faces[0] = a.tiles()[0].face;
  removed[0] = removed[0] ? 0 : 1;
  CHECK(!c.restore(turtle, faces, removed, {}), "rechaza fichas sin pareja");
  CHECK(!c.restore(turtle, {1, 2}, {0, 0}, {}), "rechaza tamaños que no cuadran");
}

void testVersions() {
  std::array<int, 3> v{};
  CHECK(nm::parseVersion("v1.2.3", v) && v == (std::array<int, 3>{1, 2, 3}), "v1.2.3");
  CHECK(nm::parseVersion("2", v) && v == (std::array<int, 3>{2, 0, 0}), "versión de una cifra");
  for (const char* bad : {"", "v", "1..2", "1.2.3.4", "1.2a", "1.2.", "$(rm -rf)", "999999"})
    CHECK(!nm::parseVersion(bad, v), "debería rechazar la versión '%s'", bad);
  CHECK(nm::isNewerVersion("0.2.0", "0.1.9"), "0.2.0 > 0.1.9");
  CHECK(nm::isNewerVersion("v1.0", "0.9.9"), "1.0 > 0.9.9");
  CHECK(nm::isNewerVersion("0.10.0", "0.9.0"), "se compara por números, no por texto");
  CHECK(!nm::isNewerVersion("0.1.0", "0.1.0"), "la misma versión no es más nueva");
  CHECK(!nm::isNewerVersion("0.0.9", "0.1.0"), "una versión anterior no es más nueva");
}

void testReleaseParsing() {
  const std::string sum(64, 'a');
  const std::string json =
      R"({"url":"https://api.github.com/x","html_url":"https://github.com/me/game/releases/tag/v0.2.0",)"
      R"("tag_name":"v0.2.0","assets":[)"
      R"({"name":"g.zip","uploader":{"html_url":"https://github.com/me"},"digest":"sha256:)" + std::string(64, 'b') +
      R"(","browser_download_url":"https://github.com/me/game/releases/download/v0.2.0/G-0.2.0-Windows-x64.zip"},)"
      R"({"name":"g.dmg","digest":"sha256:)" + sum +
      R"(","browser_download_url":"https://github.com/me/game/releases/download/v0.2.0/G-0.2.0-macOS-arm64.dmg"}]})";
  nm::ReleaseInfo info;
  CHECK(nm::parseRelease(json, "me/game", "-macOS-arm64.dmg", info), "debe encontrar el paquete de macOS");
  CHECK(info.version == "0.2.0", "versión leída: %s", info.version.c_str());
  CHECK(info.sha256 == sum, "debe tomar la huella del paquete correcto");
  CHECK(info.pageUrl == "https://github.com/me/game/releases/tag/v0.2.0", "página: %s", info.pageUrl.c_str());
  CHECK(nm::parseRelease(json, "me/game", "-Windows-x64.zip", info) && info.sha256 == std::string(64, 'b'),
        "debe encontrar el paquete de Windows con su huella");
  CHECK(!nm::parseRelease(json, "otro/repo", "-macOS-arm64.dmg", info), "no acepta descargas de otro repositorio");
  CHECK(!nm::parseRelease(json, "me/game", "-Linux.tar.gz", info), "sin paquete para la plataforma no hay versión");
  CHECK(!nm::parseRelease("{\"message\":\"Not Found\"}", "me/game", ".dmg", info), "respuesta de error");
  const std::string evil =
      R"({"tag_name":"v9.0.0","assets":[{"browser_download_url":"https://github.com/me/game/releases/download/v9/x\";rm -rf ~;\".dmg"}]})";
  CHECK(!nm::parseRelease(evil, "me/game", ".dmg", info), "no acepta direcciones con caracteres raros");
  const std::string noDigest =
      R"({"tag_name":"v1.0.0","assets":[{"digest":null,"browser_download_url":"https://github.com/me/game/releases/download/v1.0.0/a.dmg"}]})";
  CHECK(nm::parseRelease(noDigest, "me/game", ".dmg", info) && info.sha256.empty(), "huella ausente");
}

void testSha256() {
  const auto path = std::filesystem::temp_directory_path() / "nm-mahjong-sha-test.bin";
  { std::ofstream(path, std::ios::binary) << "abc"; }
  CHECK(nm::sha256File(path) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "SHA-256 de abc");
  { std::ofstream(path, std::ios::binary) << std::string(1000, 'a'); }
  CHECK(nm::sha256File(path) == "41edece42d63e8d9bf515a9ba6932e1c20cbc9f5a5d134645adb5db1b9737ea3", "SHA-256 de 1000 aes");
  std::filesystem::remove(path);
  CHECK(nm::sha256File(path).empty(), "un archivo que no existe no tiene huella");
}

}  // namespace

void testSeasons() {
  using nm::Season;
  CHECK(nm::seasonOf(20260415) == Season::Spring, "abril es primavera");
  CHECK(nm::seasonOf(20260701) == Season::Summer, "julio es verano");
  CHECK(nm::seasonOf(20261121) == Season::Autumn, "noviembre es otoño");
  CHECK(nm::seasonOf(20261225) == Season::Winter, "diciembre es invierno");
  CHECK(nm::seasonOf(20260213) == Season::Winter, "cuatro días antes del Año Nuevo aún es invierno");
  CHECK(nm::seasonOf(20260214) == Season::NewYear, "tres días antes ya es Año Nuevo");
  CHECK(nm::seasonOf(20260304) == Season::NewYear, "hasta la Fiesta de los Faroles, cruzando de mes");
  CHECK(nm::seasonOf(20260305) == Season::Spring, "después vuelve la estación");
  CHECK(nm::seasonOf(20280126) == Season::NewYear, "Año Nuevo de 2028");
}

// La solución que acompaña a un reparto tiene que vaciar el tablero: es la que se enseña al rendirse.
void testSolutionReplay() {
  for (const nm::Layout& layout : nm::allLayouts()) {
    for (nm::Difficulty d : {nm::Difficulty::Easy, nm::Difficulty::Hard}) {
      nm::Board first, again;
      CHECK(first.generate(layout, 4242, d), "%s: reparto", layout.id.c_str());
      CHECK(again.generate(layout, 4242, d), "%s: mismo reparto", layout.id.c_str());
      bool ok = true;
      for (const auto& [a, b] : first.solution()) ok = ok && again.removePair(a, b);
      CHECK(ok && again.cleared(), "%s: la solución guardada resuelve el reparto regenerado", layout.id.c_str());
    }
  }
}

int main() {
  testSeasons();
  testSolutionReplay();
  testVersions();
  testReleaseParsing();
  testSha256();
  CHECK(nm::allLayouts().size() == 12, "se esperan 12 niveles");
  CHECK(nm::matchGroup(34) == nm::matchGroup(37), "las flores emparejan entre sí");
  CHECK(nm::matchGroup(38) == nm::matchGroup(41), "las estaciones emparejan entre sí");
  CHECK(nm::matchGroup(34) != nm::matchGroup(38), "flores y estaciones no se mezclan");
  for (const nm::Layout& layout : nm::allLayouts()) {
    testLayoutShape(layout);
    testSolvable(layout);
    std::printf("%-10s %3zu fichas\n", layout.id.c_str(), layout.slots.size());
  }
  testRules();
  testRandomLayouts();
  testDeterminismAndRestore();
  std::printf(failures == 0 ? "Todas las pruebas correctas\n" : "%d fallos\n", failures);
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
