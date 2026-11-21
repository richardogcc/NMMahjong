// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "core/Season.hpp"

namespace nm {
namespace {

// Días desde una fecha de referencia, para poder restar fechas.
int dayNumber(int date) {
  int y = date / 10000, m = date / 100 % 100;
  const int d = date % 100;
  if (m <= 2) {
    --y;
    m += 12;
  }
  return 365 * y + y / 4 - y / 100 + y / 400 + (153 * (m - 3) + 2) / 5 + d;
}

// Primer día del año lunar. Depende de la luna: no hay fórmula sencilla.
constexpr int kNewYears[] = {20260217, 20270206, 20280126, 20290213, 20300203, 20310123, 20320211,
                             20330131, 20340219, 20350208, 20360128, 20370215, 20380204, 20390124,
                             20400212, 20410201, 20420122, 20430210, 20440130, 20450217};

}  // namespace

Season seasonOf(int date) {
  const int today = dayNumber(date);
  for (int start : kNewYears) {
    const int offset = today - dayNumber(start);
    if (offset >= -3 && offset <= 15) return Season::NewYear;
  }
  const int month = date / 100 % 100;
  if (month >= 3 && month <= 5) return Season::Spring;
  if (month >= 6 && month <= 8) return Season::Summer;
  if (month >= 9 && month <= 11) return Season::Autumn;
  return Season::Winter;
}

}  // namespace nm
