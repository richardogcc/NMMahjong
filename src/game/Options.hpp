// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include "game/Ctx.hpp"

namespace nm {

// Dibuja y atiende las filas de opciones dentro de `box` y guarda los cambios.
// Con `inGame` avisa de que dificultad y modo valen para la próxima partida.
void drawOptionsPanel(Ctx& ctx, Rectangle box, bool inGame);
// Pestaña visible: 0 imagen y sonido, 1 juego, 2 accesibilidad.
void setOptionsTab(int which);

}  // namespace nm
