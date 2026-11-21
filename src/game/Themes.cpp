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

const Theme kMenu = {"menu", {0.05f, 0.07f, 0.16f}, {0.10f, 0.30f, 0.38f}, {0.80f, 0.90f, 1.00f}, 1, 0, 0.5f, MoteKind::Petals, kPetal};

const Theme kLevels[] = {
    {"jardin", {0.10f, 0.26f, 0.22f}, {0.42f, 0.66f, 0.62f}, {0.90f, 1.00f, 0.85f}, 1, 0, 0.5f, MoteKind::Petals, kPetal},
    {"piramide", {0.36f, 0.16f, 0.14f}, {0.86f, 0.52f, 0.30f}, {1.00f, 0.86f, 0.62f}, 3, 3, 0.5f, MoteKind::Dust, kGoldDust},
    {"puente", {0.02f, 0.16f, 0.28f}, {0.05f, 0.38f, 0.48f}, {1.00f, 0.95f, 0.80f}, 1, 2, 0.5f, MoteKind::Dust, kFoam},
    {"tortuga", {0.02f, 0.20f, 0.16f}, {0.08f, 0.42f, 0.34f}, {0.85f, 1.00f, 1.00f}, 1, 0, 0.5f, MoteKind::Dust, kFoam},
    {"fortaleza", {0.03f, 0.03f, 0.10f}, {0.12f, 0.08f, 0.28f}, {0.92f, 0.95f, 1.00f}, 3, 1, 0.5f, MoteKind::Fireflies, kFirefly},
    {"mariposa", {0.16f, 0.05f, 0.20f}, {0.50f, 0.16f, 0.42f}, {0.95f, 0.88f, 1.00f}, 3, 3, 0.5f, MoteKind::Petals, kPetal},
    {"cruz", {0.02f, 0.08f, 0.14f}, {0.05f, 0.36f, 0.40f}, {0.40f, 0.95f, 0.70f}, 0, 2, 0.5f, MoteKind::Dust, kFoam},
    {"templo", {0.08f, 0.02f, 0.04f}, {0.32f, 0.06f, 0.10f}, {1.00f, 0.80f, 0.55f}, 1, 1, 0.5f, MoteKind::Dust, kGoldDust},
};

}  // namespace

const Theme& menuTheme() { return kMenu; }

const Theme& levelTheme(int levelIndex) {
  const int count = static_cast<int>(std::size(kLevels));
  return kLevels[std::clamp(levelIndex, 0, count - 1) % count];
}

}  // namespace nm
