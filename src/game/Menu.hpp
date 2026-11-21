// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include "game/Ctx.hpp"
#include "game/Game.hpp"

namespace nm {

struct MenuResult {
  enum class Kind { None, Play, Quit };
  Kind kind = Kind::None;
  GameRequest request;
};

// Pantalla principal: selección de tablero y paneles de opciones,
// estadísticas, logros y tutorial.
class Menu {
 public:
  MenuResult draw(Ctx& ctx);
  // Abre un panel directamente: 1 opciones, 2 estadísticas, 3 logros, 4 tutorial.
  void openPanel(int which) { panel_ = static_cast<Panel>(which); }

 private:
  enum class Panel { None, Options, Stats, Achievements, Tutorial };

  // Menu.cpp
  bool drawLevelCard(const Ctx& ctx, int index, Rectangle card, bool unlocked, bool focused) const;
  bool drawSpecialCard(const Ctx& ctx, int which, Rectangle card, bool enabled, bool focused) const;
  // Panels.cpp
  void drawPanel(Ctx& ctx);
  void drawOptions(Ctx& ctx, Rectangle box);
  void drawStats(const Ctx& ctx, Rectangle box) const;
  void drawAchievements(const Ctx& ctx, Rectangle box) const;
  void drawTutorial(Ctx& ctx, Rectangle box);

  Panel panel_ = Panel::None;
  int tutorialPage_ = 0;
  int focus_ = 0;            // tarjeta señalada con teclado o mando
  bool focusActive_ = false;
  bool firstFrame_ = true;
};

}  // namespace nm
