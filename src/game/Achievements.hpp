// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include "game/Strings.hpp"

namespace nm {

struct Achievement {
  const char* id;  // identificador estable con el que se guarda
  Str title, description;
};

inline constexpr Achievement kAchievements[] = {
    {"first", Str::AchFirstT, Str::AchFirstD},    {"clean", Str::AchCleanT, Str::AchCleanD},
    {"combo9", Str::AchComboT, Str::AchComboD},   {"all", Str::AchAllT, Str::AchAllD},
    {"stars", Str::AchStarsT, Str::AchStarsD},    {"daily3", Str::AchDaily3T, Str::AchDaily3D},
    {"daily7", Str::AchDaily7T, Str::AchDaily7D}, {"random10", Str::AchRandomT, Str::AchRandomD},
    {"timed", Str::AchTimedT, Str::AchTimedD},    {"hard", Str::AchHardT, Str::AchHardD},
};

}  // namespace nm
