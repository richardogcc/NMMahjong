// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Este archivo no incluye raylib a propósito: <windows.h> choca con sus nombres.
#include "game/Strings.hpp"

#include <cstdlib>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace nm {
namespace {

Lang current = Lang::Es;

struct Entry {
  const char* es;
  const char* en;
};

constexpr Entry kTable[] = {
    /* Subtitle      */ {"Solitario de fichas", "Tile solitaire"},
    /* Music         */ {"Música", "Music"},
    /* Effects       */ {"Efectos", "Sound"},
    /* Language      */ {"Idioma", "Language"},
    /* LanguageName  */ {"Español", "English"},
    /* Fullscreen    */ {"Pantalla completa", "Fullscreen"},
    /* Quit          */ {"Salir", "Quit"},
    /* Locked        */ {"Completa el nivel anterior", "Clear the previous level"},
    /* Tiles         */ {"fichas", "tiles"},
    /* Best          */ {"Mejor", "Best"},
    /* Menu          */ {"Menú", "Menu"},
    /* Hint          */ {"Pista", "Hint"},
    /* Undo          */ {"Deshacer", "Undo"},
    /* Shuffle       */ {"Barajar", "Shuffle"},
    /* Restart       */ {"Reiniciar", "Restart"},
    /* Pairs         */ {"Parejas", "Pairs"},
    /* Moves         */ {"Movimientos", "Moves"},
    /* Time          */ {"Tiempo", "Time"},
    /* Score         */ {"Puntos", "Score"},
    /* LevelComplete */ {"¡Nivel completado!", "Level complete!"},
    /* GameComplete  */ {"¡Has completado todos los niveles!", "You cleared every level!"},
    /* NoMoves       */ {"Sin movimientos", "No moves left"},
    /* NoMovesBody   */ {"Baraja las fichas o deshaz tu última jugada.",
                         "Shuffle the tiles or undo your last move."},
    /* NoMovesDead   */ {"Este reparto ya no tiene salida. Reinicia el nivel.",
                         "This deal cannot be finished. Restart the level."},
    /* Next          */ {"Siguiente", "Next"},
    /* Retry         */ {"Repetir", "Replay"},
    /* Combo         */ {"Combo", "Combo"},
    /* West          */ {"O", "W"},
};
static_assert(sizeof(kTable) / sizeof(kTable[0]) == static_cast<int>(Str::Count));

constexpr Entry kLevels[] = {
    {"Jardín", "Garden"},       {"Pirámide", "Pyramid"},   {"Puente", "Bridge"},
    {"Tortuga", "Turtle"},      {"Fortaleza", "Fortress"}, {"Mariposa", "Butterfly"},
    {"Cruz", "Cross"},          {"Templo", "Temple"},
};

const char* pick(const Entry& e) { return current == Lang::Es ? e.es : e.en; }

}  // namespace

void setLang(Lang lang) { current = lang; }
Lang lang() { return current; }

Lang systemLang() {
#if defined(_WIN32)
  return (GetUserDefaultUILanguage() & 0x3ff) == LANG_SPANISH ? Lang::Es : Lang::En;
#elif defined(__APPLE__)
  bool spanish = false;
  if (CFArrayRef langs = CFLocaleCopyPreferredLanguages()) {
    if (CFArrayGetCount(langs) > 0) {
      const auto first = static_cast<CFStringRef>(CFArrayGetValueAtIndex(langs, 0));
      spanish = CFStringHasPrefix(first, CFSTR("es"));
    }
    CFRelease(langs);
  }
  return spanish ? Lang::Es : Lang::En;
#else
  const char* env = std::getenv("LANG");
  return env && std::strncmp(env, "es", 2) == 0 ? Lang::Es : Lang::En;
#endif
}

const char* tr(Str key) { return pick(kTable[static_cast<int>(key)]); }

const char* levelName(int levelIndex) {
  const int count = static_cast<int>(sizeof(kLevels) / sizeof(kLevels[0]));
  if (levelIndex < 0 || levelIndex >= count) return "";
  return pick(kLevels[levelIndex]);
}

}  // namespace nm
