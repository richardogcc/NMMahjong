// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Montaje de la partida, geometría del tablero y guardado.
#include "game/Game.hpp"

#include <algorithm>
#include <cmath>

#include "game/Strings.hpp"
#include "ui/Ui.hpp"

namespace nm {
namespace {

constexpr float kBoardTop = 92.0f;
constexpr float kBoardW = 1480.0f;
constexpr float kBoardH = 780.0f;

std::uint32_t randomSeed() { return static_cast<std::uint32_t>(GetRandomValue(1, 2000000000)); }

// El reto diario sale de la fecha: mismo tablero y mismo reparto para todos.
std::uint32_t dailySeed() { return static_cast<std::uint32_t>(todayDate()) * 2654435761u; }

}  // namespace

const Layout& Game::layout() const {
  return kind_ == BoardKind::Level ? allLayouts()[level_] : generated_;
}

void Game::start(const GameRequest& request, Ctx& ctx) {
  if (request.resume && resume(ctx)) return;
  clearSavedGame();

  const int levels = static_cast<int>(allLayouts().size());
  SaveData& save = *ctx.save;
  kind_ = request.kind;
  level_ = std::clamp(request.level, 0, levels - 1);
  difficulty_ = static_cast<Difficulty>(save.difficulty);
  style_ = static_cast<PlayStyle>(save.playStyle);

  if (kind_ == BoardKind::Level) {
    themeIndex_ = level_;
    dealSeed_ = randomSeed();
    dealDifficulty_ = difficulty_;
    board_.generate(allLayouts()[level_], dealSeed_, difficulty_);
  } else if (kind_ == BoardKind::Daily) {
    layoutSeed_ = dailySeed();
    generated_ = randomLayout(layoutSeed_);
    // Reparto fijo, sin depender de la dificultad elegida, para que sea igual para todos.
    dealSeed_ = layoutSeed_ + 1;
    dealDifficulty_ = Difficulty::Easy;
    while (!board_.generate(generated_, dealSeed_)) ++dealSeed_;
    themeIndex_ = static_cast<int>(layoutSeed_ % static_cast<std::uint32_t>(levels));
  } else {
    // Tablero nuevo cada vez; se repite si alguna forma rara no admite reparto.
    do {
      layoutSeed_ = randomSeed();
      generated_ = randomLayout(layoutSeed_);
      dealSeed_ = randomSeed();
      dealDifficulty_ = difficulty_;
    } while (!board_.generate(generated_, dealSeed_, difficulty_));
    themeIndex_ = GetRandomValue(0, levels - 1);
  }

  resetRound(ctx);
  elapsed_ = 0.0f;
  score_ = hintsUsed_ = shufflesUsed_ = 0;
  const float perTile = difficulty_ == Difficulty::Easy ? 6.0f : (difficulty_ == Difficulty::Normal ? 5.0f : 4.0f);
  timeLeft_ = static_cast<float>(board_.tiles().size()) * perTile;
  dealAnimation(0.15f);
  ++save.played;
  save.store();
}

bool Game::resume(Ctx& ctx) {
  SavedGame saved;
  if (!loadSavedGame(saved)) return false;
  const int levels = static_cast<int>(allLayouts().size());
  kind_ = static_cast<BoardKind>(saved.kind);
  level_ = std::clamp(saved.level, 0, levels - 1);
  layoutSeed_ = saved.layoutSeed;
  dealSeed_ = saved.dealSeed;
  if (kind_ != BoardKind::Level) generated_ = randomLayout(layoutSeed_);
  if (!board_.restore(layout(), saved.faces, saved.removed, saved.history) || board_.cleared()) {
    clearSavedGame();
    return false;
  }
  themeIndex_ = saved.theme % levels;
  difficulty_ = static_cast<Difficulty>(saved.difficulty);
  style_ = static_cast<PlayStyle>(saved.playStyle);
  dealDifficulty_ = kind_ == BoardKind::Daily ? Difficulty::Easy : difficulty_;
  resetRound(ctx);
  elapsed_ = saved.elapsed;
  timeLeft_ = saved.timeLeft;
  score_ = saved.score;
  hintsUsed_ = saved.hintsUsed;
  shufflesUsed_ = saved.shufflesUsed;
  dealAnimation(0.0f);
  afterBoardChange(ctx);
  return true;
}

// Lo que se reinicia igual al empezar y al continuar una partida.
void Game::resetRound(Ctx& ctx) {
  views_.assign(board_.tiles().size(), TileView{});
  ghosts_.clear();
  popups_.clear();
  particles_.clear();
  state_ = State::Playing;
  options_ = false;
  deadEnd_ = false;
  stateTime_ = introTime_ = confettiTimer_ = 0.0f;
  selected_ = hover_ = hintA_ = hintB_ = cursor_ = -1;
  cursorActive_ = false;
  hintTimer_ = comboTimer_ = toastTime_ = 0.0f;
  combo_ = stars_ = 0;
  newRecord_ = false;
  toast_.clear();

  const bool free = style_ == PlayStyle::Zen || difficulty_ == Difficulty::Easy;
  hintLimit_ = free ? -1 : (difficulty_ == Difficulty::Normal ? 5 : 2);
  shuffleLimit_ = free ? -1 : (difficulty_ == Difficulty::Normal ? 3 : 1);

  layoutBoard();
  moves_ = board_.availableMoves();
  ctx.audio->setMood(themeIndex_ + 1);  // la pieza 0 es la del menú
}

void Game::suspend() const {
  if (finished() || board_.cleared() || board_.tiles().empty()) return;
  SavedGame g;
  g.kind = static_cast<int>(kind_);
  g.level = level_;
  g.layoutSeed = layoutSeed_;
  g.dealSeed = dealSeed_;
  g.theme = themeIndex_;
  g.difficulty = static_cast<int>(difficulty_);
  g.playStyle = static_cast<int>(style_);
  g.score = score_;
  g.hintsUsed = hintsUsed_;
  g.shufflesUsed = shufflesUsed_;
  g.elapsed = elapsed_;
  g.timeLeft = timeLeft_;
  for (const Tile& t : board_.tiles()) {
    g.faces.push_back(t.face);
    g.removed.push_back(t.removed ? 1 : 0);
  }
  g.history = board_.history();
  storeSavedGame(g);
}

void Game::layoutBoard() {
  const auto& tiles = board_.tiles();
  int maxX = 0, maxY = 0, maxZ = 0;
  minX_ = minY_ = 1000;
  for (const Tile& t : tiles) {
    minX_ = std::min(minX_, t.pos.x);
    minY_ = std::min(minY_, t.pos.y);
    maxX = std::max(maxX, t.pos.x + 2);
    maxY = std::max(maxY, t.pos.y + 2);
    maxZ = std::max(maxZ, t.pos.z);
  }
  // Tamaño del tablero con fichas de 1 de ancho; luego se escala para que quepa.
  const float cols = (maxX - minX_) * 0.5f, rows = (maxY - minY_) * 0.5f;
  const float extra = (maxZ + 1) * tile::kDepthRatio;
  const float unitW = cols + extra, unitH = rows * tile::kAspect + extra;
  tileW_ = std::min({kBoardW / unitW, kBoardH / unitH, 92.0f});
  tileH_ = tileW_ * tile::kAspect;
  depth_ = tileW_ * tile::kDepthRatio;

  const float left = (kViewW - unitW * tileW_) * 0.5f;
  const float top = kBoardTop + (kBoardH - unitH * tileW_) * 0.5f;
  origin_ = {left + maxZ * depth_ - minX_ * tileW_ * 0.5f, top + maxZ * depth_ - minY_ * tileH_ * 0.5f};

  order_.resize(tiles.size());
  for (int i = 0; i < static_cast<int>(order_.size()); ++i) order_[i] = i;
  std::sort(order_.begin(), order_.end(), [&tiles](int a, int b) {
    const Slot& p = tiles[a].pos;
    const Slot& q = tiles[b].pos;
    if (p.z != q.z) return p.z < q.z;
    if (p.y != q.y) return p.y < q.y;
    return p.x < q.x;
  });
}

void Game::dealAnimation(float baseDelay) {
  const auto& tiles = board_.tiles();
  for (int i = 0; i < static_cast<int>(tiles.size()); ++i) {
    const Slot& s = tiles[i].pos;
    views_[i].appear = 0.0f;
    views_[i].delay = baseDelay + s.z * 0.16f + (s.x - minX_ + s.y - minY_) * 0.011f;
  }
  introTime_ = 0.0f;
}

Vector2 Game::faceCenter(const Slot& slot) const {
  return {origin_.x + slot.x * tileW_ * 0.5f - slot.z * depth_ + tileW_ * 0.5f,
          origin_.y + slot.y * tileH_ * 0.5f - slot.z * depth_ + tileH_ * 0.5f};
}

int Game::pick(Vector2 mouse) const {
  const auto& tiles = board_.tiles();
  for (auto it = order_.rbegin(); it != order_.rend(); ++it) {
    const Tile& t = tiles[*it];
    if (t.removed || views_[*it].appear < 0.9f) continue;
    const Vector2 c = faceCenter(t.pos);
    if (std::fabs(mouse.x - c.x) <= tileW_ * 0.5f && std::fabs(mouse.y - c.y) <= tileH_ * 0.5f)
      return *it;
  }
  return -1;
}

void Game::openOptions() {
  if (state_ == State::Playing) togglePause();
  options_ = state_ == State::Paused;
}

void Game::togglePause() {
  if (options_) {
    options_ = false;
    return;
  }
  if (state_ == State::Playing) {
    state_ = State::Paused;
    selected_ = hintA_ = hintB_ = -1;
  } else if (state_ == State::Paused) {
    state_ = State::Playing;
  }
  stateTime_ = 0.0f;
}

}  // namespace nm
