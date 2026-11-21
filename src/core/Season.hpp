// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

namespace nm {

enum class Season { None = 0, Spring = 1, Summer = 2, Autumn = 3, Winter = 4, NewYear = 5 };

// Época del año de una fecha AAAAMMDD (hemisferio norte). El Año Nuevo chino
// manda sobre el invierno desde tres días antes hasta la Fiesta de los Faroles.
Season seasonOf(int date);

}  // namespace nm
