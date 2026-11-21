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
    /* UpdateAvailable   */ {"Versión %s disponible", "Version %s available"},
    /* UpdateNow         */ {"Actualizar", "Update"},
    /* UpdateDownloading */ {"Descargando actualización...", "Downloading update..."},
    /* UpdateRestarting  */ {"Reiniciando...", "Restarting..."},
    /* UpdateFailed      */ {"No se pudo actualizar", "Update failed"},
    /* UpdateOpenPage    */ {"Abrir descargas", "Open downloads"},
    /* Paused            */ {"Pausa", "Paused"},
    /* Resume            */ {"Continuar", "Resume"},
    /* Random            */ {"Aleatorio", "Random"},
    /* RandomHint        */ {"Un tablero distinto cada vez", "A different board every time"},
    {"Opciones", "Options"},
    {"Estadísticas", "Statistics"},
    {"Logros", "Achievements"},
    {"Reto diario", "Daily challenge"},
    {"El mismo tablero para todos, cada día", "The same board for everyone, every day"},
    {"Completado hoy", "Done today"},
    {"Racha: %d", "Streak: %d"},
    {"Continuar", "Continue"},
    {"Cómo se juega", "How to play"},
    {"Cerrar", "Close"},
    {"Fondo", "Background"},
    {"Animado", "Animated"},
    {"Fijo", "Still"},
    {"Pieza musical", "Music track"},
    {"Según el nivel", "By level"},
    {"Resaltar fichas libres", "Highlight free tiles"},
    {"Sí", "Yes"},
    {"No", "No"},
    {"Fichas", "Tiles"},
    {"Modernas", "Modern"},
    {"Clásicas", "Classic"},
    {"Alto contraste", "High contrast"},
    {"Dificultad", "Difficulty"},
    {"Fácil", "Easy"},
    {"Normal", "Normal"},
    {"Difícil", "Hard"},
    {"Modo", "Mode"},
    {"Normal", "Normal"},
    {"Contrarreloj", "Time attack"},
    {"Zen", "Zen"},
    {"¡Tiempo agotado!", "Time is up!"},
    {"El reloj ha llegado a cero. Prueba otra vez.", "The clock ran out. Give it another go."},
    {"Logro desbloqueado", "Achievement unlocked"},
    {"Zen", "Zen"},
    {"Partidas jugadas", "Games played"},
    {"Partidas ganadas", "Games won"},
    {"Porcentaje de victorias", "Win rate"},
    {"Racha actual", "Current streak"},
    {"Mejor racha", "Best streak"},
    {"Tiempo medio al ganar", "Average winning time"},
    {"Parejas retiradas", "Pairs removed"},
    {"Mayor combo", "Highest combo"},
    {"Mejor racha diaria", "Best daily streak"},
    {"Primera victoria", "First win"},
    {"Completa cualquier tablero.", "Clear any board."},
    {"Sin ayuda", "No help needed"},
    {"Gana sin pistas ni barajar.", "Win without hints or shuffles."},
    {"Racha imparable", "On a roll"},
    {"Consigue un combo x9.", "Reach a x9 combo."},
    {"Viaje completo", "The whole journey"},
    {"Completa los doce niveles.", "Clear all twelve levels."},
    {"Perfeccionista", "Perfectionist"},
    {"Tres estrellas en todos los niveles.", "Three stars on every level."},
    {"Constancia", "Consistency"},
    {"Reto diario tres días seguidos.", "Daily challenge three days running."},
    {"Semana redonda", "A full week"},
    {"Reto diario siete días seguidos.", "Daily challenge seven days running."},
    {"Explorador", "Explorer"},
    {"Gana diez tableros aleatorios.", "Win ten random boards."},
    {"Contra el reloj", "Beat the clock"},
    {"Gana una partida contrarreloj.", "Win a time attack game."},
    {"Maestría", "Mastery"},
    {"Gana una partida en difícil.", "Win a game on hard."},
    {"Empareja fichas", "Match the tiles"},
    {"Pulsa una ficha y después otra igual para retirar las dos. Ganas al despejar el tablero.",
     "Click a tile and then a matching one to remove both. Clear the board to win."},
    {"Fichas libres", "Free tiles"},
    {"Solo puedes usar una ficha si no tiene nada encima y su lado izquierdo o derecho está despejado.",
     "You can only use a tile with nothing on top and an open left or right side."},
    {"Flores y estaciones", "Flowers and seasons"},
    {"Las flores emparejan con cualquier flor, y las estaciones con cualquier estación.",
     "Any flower matches any flower, and any season matches any season."},
    {"Ayudas y teclas", "Help and keys"},
    {"Pista (H), deshacer (Z) y barajar (B). Escape pausa. Las flechas mueven el cursor y Enter elige.",
     "Hint (H), undo (Z) and shuffle (B). Escape pauses. Arrows move the cursor and Enter picks."},
};
static_assert(sizeof(kTable) / sizeof(kTable[0]) == static_cast<int>(Str::Count));

constexpr Entry kLevels[] = {
    {"Jardín", "Garden"},       {"Pirámide", "Pyramid"},   {"Puente", "Bridge"},
    {"Tortuga", "Turtle"},      {"Fortaleza", "Fortress"}, {"Mariposa", "Butterfly"},
    {"Cruz", "Cross"},          {"Templo", "Temple"},      {"Dragón", "Dragon"},
    {"Araña", "Spider"},        {"Arco", "Arch"},          {"Corona", "Crown"},
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
