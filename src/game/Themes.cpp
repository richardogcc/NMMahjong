// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include <algorithm>
#include <iterator>

#include "game/Ctx.hpp"

namespace nm {
namespace {

constexpr Color kPetal = {255, 176, 204, 255};
constexpr Color kGoldDust = {255, 214, 130, 255};
constexpr Color kFoam = {190, 240, 255, 255};
constexpr Color kFirefly = {255, 244, 170, 255};

const Theme kMenu = {{0.05f, 0.07f, 0.16f}, {0.10f, 0.30f, 0.38f}, {0.55f, 0.30f, 0.65f}, 0, 0, kPetal};

const Theme kLevels[] = {
    /* Jardín    */ {{0.10f, 0.26f, 0.22f}, {0.42f, 0.66f, 0.62f}, {1.00f, 0.85f, 0.55f}, 3, 0, kPetal},
    /* Pirámide  */ {{0.36f, 0.16f, 0.14f}, {0.86f, 0.52f, 0.30f}, {1.00f, 0.88f, 0.58f}, 3, 3, kGoldDust},
    /* Puente    */ {{0.02f, 0.16f, 0.28f}, {0.05f, 0.38f, 0.48f}, {0.60f, 0.95f, 1.00f}, 1, 2, kFoam},
    /* Tortuga   */ {{0.02f, 0.20f, 0.16f}, {0.08f, 0.42f, 0.34f}, {0.75f, 1.00f, 0.80f}, 1, 0, kFoam},
    /* Fortaleza */ {{0.03f, 0.03f, 0.10f}, {0.12f, 0.08f, 0.28f}, {0.50f, 0.30f, 0.80f}, 2, 1, kFirefly},
    /* Mariposa  */ {{0.16f, 0.05f, 0.20f}, {0.50f, 0.16f, 0.42f}, {1.00f, 0.62f, 0.45f}, 0, 3, kPetal},
    /* Cruz      */ {{0.02f, 0.08f, 0.14f}, {0.05f, 0.36f, 0.40f}, {0.40f, 0.95f, 0.70f}, 0, 2, kFoam},
    /* Templo    */ {{0.08f, 0.02f, 0.04f}, {0.32f, 0.06f, 0.10f}, {0.95f, 0.55f, 0.25f}, 2, 1, kGoldDust},
};

}  // namespace

const Theme& menuTheme() { return kMenu; }

const Theme& levelTheme(int levelIndex) {
  const int count = static_cast<int>(std::size(kLevels));
  return kLevels[std::clamp(levelIndex, 0, count - 1) % count];
}

}  // namespace nm
