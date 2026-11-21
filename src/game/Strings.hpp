// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

namespace nm {

enum class Lang { Es = 0, En = 1 };

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
  Count
};

void setLang(Lang lang);
Lang lang();
// Idioma del sistema operativo: español si está configurado así, inglés en otro caso.
Lang systemLang();

const char* tr(Str key);
const char* levelName(int levelIndex);

}  // namespace nm
