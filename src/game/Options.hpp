// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include "game/Ctx.hpp"

namespace nm {

// Dibuja y atiende las filas de opciones dentro de `box` y guarda los cambios.
// Con `inGame` avisa de que dificultad y modo valen para la próxima partida.
void drawOptionsPanel(Ctx& ctx, Rectangle box, bool inGame);

}  // namespace nm
