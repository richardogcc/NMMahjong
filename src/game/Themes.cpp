// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include <algorithm>
#include <iterator>

#include "game/Ctx.hpp"

namespace nm {
namespace {

constexpr Color kPetal = {255, 176, 204, 255};
constexpr Color kGoldDust = {255, 214, 130, 255};
constexpr Color kFoam = {190, 240, 255, 255};

constexpr Color kBamboo = {176, 214, 120, 255};
constexpr Color kRiceGold = {240, 214, 120, 255};

// foto, paleta (a, b: fondo de reserva; c: tinte de niebla y luz), animación,
// música, niebla, qué flota y de qué color.
const Theme kMenu = {"menu", {0.05f, 0.07f, 0.16f}, {0.10f, 0.30f, 0.38f}, {0.92f, 0.95f, 1.00f}, 3, 0,
                     0.95f, MoteKind::Petals, kPetal};

const Theme kLevels[] = {
    /* Jardín: jardín de Suzhou        */ {"jardin", {0.10f, 0.26f, 0.22f}, {0.42f, 0.66f, 0.62f}, {0.92f, 1.00f, 0.90f}, 1, 0, 0.35f, MoteKind::Petals, kPetal},
    /* Pirámide: Templo del Cielo      */ {"piramide", {0.36f, 0.16f, 0.14f}, {0.86f, 0.52f, 0.30f}, {1.00f, 0.92f, 0.78f}, 3, 3, 0.30f, MoteKind::Dust, kGoldDust},
    /* Puente: Zhouzhuang              */ {"puente", {0.02f, 0.16f, 0.28f}, {0.05f, 0.38f, 0.48f}, {1.00f, 0.96f, 0.86f}, 1, 2, 0.30f, MoteKind::Petals, kPetal},
    /* Tortuga: lago de Jiuzhaigou     */ {"tortuga", {0.02f, 0.20f, 0.16f}, {0.08f, 0.42f, 0.34f}, {0.88f, 1.00f, 1.00f}, 1, 0, 0.50f, MoteKind::Dust, kFoam},
    /* Fortaleza: Gran Muralla         */ {"fortaleza", {0.03f, 0.03f, 0.10f}, {0.12f, 0.08f, 0.28f}, {1.00f, 0.95f, 0.85f}, 3, 1, 0.60f, MoteKind::Leaves, kBamboo},
    /* Mariposa: estanque de lotos     */ {"mariposa", {0.16f, 0.05f, 0.20f}, {0.50f, 0.16f, 0.42f}, {1.00f, 0.95f, 0.98f}, 3, 3, 0.25f, MoteKind::Petals, kPetal},
    /* Cruz: terrazas de arroz         */ {"cruz", {0.02f, 0.08f, 0.14f}, {0.05f, 0.36f, 0.40f}, {1.00f, 0.97f, 0.85f}, 3, 2, 0.75f, MoteKind::Leaves, kRiceGold},
    /* Templo: Ciudad Prohibida        */ {"templo", {0.08f, 0.02f, 0.04f}, {0.32f, 0.06f, 0.10f}, {1.00f, 0.90f, 0.75f}, 3, 1, 0.30f, MoteKind::Dust, kGoldDust},
};

}  // namespace

const Theme& menuTheme() { return kMenu; }

const Theme& levelTheme(int levelIndex) {
  const int count = static_cast<int>(std::size(kLevels));
  return kLevels[std::clamp(levelIndex, 0, count - 1) % count];
}

}  // namespace nm
