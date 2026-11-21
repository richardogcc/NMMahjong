// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace nm {

struct LevelRecord {
  int stars = 0;
  int bestSeconds = 0;
  int bestScore = 0;
};

// Progreso, ajustes y estadísticas del jugador, guardados en su carpeta de datos.
struct SaveData {
  std::map<std::string, LevelRecord> levels;

  // Ajustes
  float music = 0.6f;
  float sfx = 0.8f;
  int lang = -1;              // -1: todavía sin elegir, se usa el idioma del sistema
  bool animatedBg = true;
  int musicTrack = -1;        // -1: la pieza que toque según el nivel
  bool highlightFree = true;  // atenuar las fichas que no se pueden usar
  int tileSet = 0;            // 0 modernas, 1 clásicas, 2 alto contraste, 3 daltonismo
  int difficulty = 0;         // 0 fácil, 1 normal, 2 difícil
  int playStyle = 0;          // 0 normal, 1 contrarreloj, 2 zen
  bool tutorialDone = false;
  bool largeText = false;     // textos pequeños algo más grandes
  bool reduceFx = false;      // sin destellos, confeti ni temblores
  bool seasonal = true;       // el menú y las fichas cambian con la época del año

  // Estadísticas
  int played = 0, won = 0, winStreak = 0, bestWinStreak = 0, secondsWon = 0;
  int pairs = 0, maxCombo = 0, randomWon = 0;

  // Reto diario
  int dailyDate = 0;  // último día completado, como AAAAMMDD
  int dailyStreak = 0, bestDailyStreak = 0;

  std::set<std::string> achievements;
  bool persist = true;  // false: no se escribe nada en disco (modo de capturas)

  void load();
  void store() const;
};

// Partida a medias, para poder continuarla al volver a abrir el juego.
struct SavedGame {
  int kind = 0;  // 0 nivel fijo, 1 aleatorio, 2 reto diario
  int level = 0;
  std::uint32_t layoutSeed = 0;
  std::uint32_t dealSeed = 0;  // 0: reparto desconocido (guardados antiguos)
  int theme = 0, difficulty = 0, playStyle = 0;
  int score = 0, hintsUsed = 0, shufflesUsed = 0;
  float elapsed = 0.0f, timeLeft = 0.0f;
  std::vector<int> faces, removed;
  std::vector<std::pair<int, int>> history;
};

bool loadSavedGame(SavedGame& out);
void storeSavedGame(const SavedGame& game);
void clearSavedGame();
bool hasSavedGame();

// Fecha local de hoy como AAAAMMDD, y la del día anterior a una fecha dada.
int todayDate();
int previousDate(int date);

}  // namespace nm
