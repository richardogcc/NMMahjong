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
  Count
};

void setLang(Lang lang);
Lang lang();
// Idioma del sistema operativo: español si está configurado así, inglés en otro caso.
Lang systemLang();

const char* tr(Str key);
const char* levelName(int levelIndex);

}  // namespace nm
