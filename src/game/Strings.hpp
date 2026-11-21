// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <string>

namespace nm {

enum class Lang { Es = 0, En = 1, Fr = 2, De = 3, Pt = 4, Zh = 5 };
constexpr int kLangCount = 6;

enum class Str {
  Subtitle,
  Music,
  Effects,
  Language,
  LanguageName,
  Fullscreen,
  Quit,
  Locked,
  Tiles,
  Best,
  Menu,
  Hint,
  Undo,
  Shuffle,
  Restart,
  Pairs,
  Moves,
  Time,
  Score,
  LevelComplete,
  GameComplete,
  NoMoves,
  NoMovesBody,
  NoMovesDead,
  Next,
  Retry,
  Combo,
  West,
  UpdateAvailable,
  UpdateNow,
  UpdateDownloading,
  UpdateRestarting,
  UpdateFailed,
  UpdateOpenPage,
  Paused,
  Resume,
  Random,
  RandomHint,
  // Menú
  Options, Stats, Achievements, Daily, DailyHint, DailyDone, Streak, Continue, HowToPlay, Close,
  // Opciones
  OptBackground, Animated, Static, OptMusicTrack, ByLevel, OptHighlight, Yes, No, OptTiles, TilesModern,
  TilesClassic, TilesContrast, OptDifficulty, DiffEasy, DiffNormal, DiffHard, OptStyle, StyleNormal,
  StyleTimed, StyleZen,
  // Partida
  TimeUp, TimeUpBody, AchievementUnlocked, Zen,
  // Estadísticas
  StPlayed, StWon, StWinRate, StStreak, StBestStreak, StAvgTime, StPairs, StMaxCombo, StDailyBest,
  // Logros (título y descripción, en el mismo orden que kAchievements)
  AchFirstT, AchFirstD, AchCleanT, AchCleanD, AchComboT, AchComboD, AchAllT, AchAllD, AchStarsT, AchStarsD,
  AchDaily3T, AchDaily3D, AchDaily7T, AchDaily7D, AchRandomT, AchRandomD, AchTimedT, AchTimedD, AchHardT,
  AchHardD,
  // Tutorial
  Tut1T, Tut1B, Tut2T, Tut2B, Tut3T, Tut3B, Tut4T, Tut4B,
  OptNextGame,
  // Récords, accesibilidad y solución
  NewRecord, OptLargeText, OptReduceFx, OptSeasonal, TilesColorBlind, TabDisplay,
  TabGame, TabAccess, ShowSolution, Solution, SolutionBody,
  Count
};

void setLang(Lang lang);
Lang lang();
// Idioma del sistema operativo si es uno de los del juego; inglés en otro caso.
// `langFromCode` hace lo mismo con un código como «fr» o «pt_BR».
Lang langFromCode(const char* code);
Lang systemLang();

const char* tr(Str key);
const char* levelName(int levelIndex);
// Todos los textos de un idioma seguidos: sirve para saber qué letras cargar de la fuente.
std::string allText(Lang lang);

}  // namespace nm
