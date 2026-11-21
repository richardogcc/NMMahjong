// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include "game/Ctx.hpp"

namespace nm {

struct MenuResult {
  enum class Kind { None, Play, Quit };
  Kind kind = Kind::None;
  int level = 0;
};

// Pantalla principal: selección de nivel y ajustes.
class Menu {
 public:
  MenuResult draw(Ctx& ctx);
};

}  // namespace nm
