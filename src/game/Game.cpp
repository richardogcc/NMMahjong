// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
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
constexpr float kGhostTime = 0.45f;
constexpr float kComboWindow = 5.0f;

std::uint32_t randomSeed() { return static_cast<std::uint32_t>(GetRandomValue(1, 2000000000)); }

}  // namespace

void Game::start(int levelIndex, Ctx& ctx) {
  const auto& layouts = allLayouts();
  level_ = std::clamp(levelIndex, 0, static_cast<int>(layouts.size()) - 1);
  board_.generate(layouts[level_], randomSeed());

  views_.assign(board_.tiles().size(), TileView{});
  ghosts_.clear();
  popups_.clear();
  particles_.clear();
  state_ = State::Playing;
  deadEnd_ = false;
  stateTime_ = introTime_ = confettiTimer_ = 0.0f;
  selected_ = hover_ = hintA_ = hintB_ = -1;
  hintTimer_ = elapsed_ = comboTimer_ = 0.0f;
  score_ = combo_ = stars_ = hintsUsed_ = shufflesUsed_ = 0;

  layoutBoard();
  dealAnimation(0.15f);
  moves_ = board_.availableMoves();
  ctx.audio->setMood(levelTheme(level_).mood);
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

void Game::update(Ctx& ctx) {
  const float dt = ctx.dt;
  introTime_ += dt;
  stateTime_ += dt;
  particles_.update(dt);

  for (Ghost& g : ghosts_) {
    g.t += dt / kGhostTime;
    if (g.t >= 1.0f && g.burst) {
      particles_.burst(g.to, levelTheme(level_).mote, 34, 420.0f);
      particles_.burst(g.to, ui::kGold, 14, 260.0f);
    }
  }
  std::erase_if(ghosts_, [](const Ghost& g) { return g.t >= 1.0f; });
  for (Popup& p : popups_) {
    p.t += dt;
    p.pos.y -= 46.0f * dt;
  }
  std::erase_if(popups_, [](const Popup& p) { return p.t >= 1.2f; });

  hover_ = state_ == State::Playing ? pick(ctx.mouse) : -1;
  for (int i = 0; i < static_cast<int>(views_.size()); ++i) {
    TileView& v = views_[i];
    if (introTime_ >= v.delay) v.appear = std::min(1.0f, v.appear + dt * 3.4f);
    const bool raised = i == selected_ || (i == hover_ && board_.isFree(i));
    v.lift += ((raised ? 1.0f : 0.0f) - v.lift) * std::min(1.0f, dt * 14.0f);
    v.shake = std::max(0.0f, v.shake - dt * 2.6f);
    v.pop = std::max(0.0f, v.pop - dt * 3.0f);
  }

  if (state_ == State::Playing) {
    elapsed_ += dt;
    hintTimer_ = std::max(0.0f, hintTimer_ - dt);
    if (hintTimer_ <= 0.0f) hintA_ = hintB_ = -1;
    comboTimer_ -= dt;
    if (comboTimer_ <= 0.0f) combo_ = 0;
    if (ctx.click && ctx.mouse.y > 84.0f) handleClick(hover_, ctx);
  } else if (state_ == State::Won) {
    confettiTimer_ -= dt;
    if (confettiTimer_ <= 0.0f && stateTime_ < 5.0f) {
      confettiTimer_ = 0.45f;
      particles_.confetti({static_cast<float>(GetRandomValue(250, 1350)), 900.0f}, 46);
    }
  }
}

void Game::handleClick(int index, Ctx& ctx) {
  if (index < 0) {
    selected_ = -1;
    return;
  }
  if (!board_.isFree(index)) {
    views_[index].shake = 1.0f;
    ctx.audio->play(Sfx::Error);
    return;
  }
  if (index == selected_) {
    selected_ = -1;
    ctx.audio->play(Sfx::Click);
  } else if (selected_ >= 0 && board_.canMatch(selected_, index)) {
    match(selected_, index, ctx);
  } else {
    selected_ = index;
    ctx.audio->play(Sfx::Select);
  }
}

void Game::match(int a, int b, Ctx& ctx) {
  const auto& tiles = board_.tiles();
  const Vector2 ca = faceCenter(tiles[a].pos), cb = faceCenter(tiles[b].pos);
  const Vector2 mid = {(ca.x + cb.x) * 0.5f, (ca.y + cb.y) * 0.5f};
  const int faceA = tiles[a].face, faceB = tiles[b].face;
  if (!board_.removePair(a, b)) return;

  ghosts_.push_back({faceA, ca, mid, 0.0f, true});
  ghosts_.push_back({faceB, cb, mid, 0.0f, false});
  particles_.burst(ca, WHITE, 8, 160.0f);
  particles_.burst(cb, WHITE, 8, 160.0f);

  combo_ = comboTimer_ > 0.0f ? std::min(combo_ + 1, 9) : 1;
  comboTimer_ = kComboWindow;
  const int points = 100 * combo_;
  score_ += points;
  popups_.push_back({{mid.x, mid.y - 30.0f}, TextFormat("+%d", points), 0.0f, WHITE});
  if (combo_ >= 2)
    popups_.push_back({{mid.x, mid.y - 68.0f}, TextFormat("%s x%d", tr(Str::Combo), combo_), 0.0f, ui::kGold});
  ctx.audio->play(Sfx::Match, 1.0f + 0.06f * (combo_ - 1));

  selected_ = hintA_ = hintB_ = -1;
  afterBoardChange(ctx);
}

void Game::afterBoardChange(Ctx& ctx) {
  moves_ = board_.availableMoves();
  if (board_.cleared()) {
    win(ctx);
  } else if (moves_ == 0) {
    state_ = State::Stuck;
    stateTime_ = 0.0f;
  } else {
    state_ = State::Playing;
  }
}

void Game::useHint(Ctx& ctx) {
  const auto hint = board_.findHint();
  if (!hint) return;
  hintA_ = hint->first;
  hintB_ = hint->second;
  hintTimer_ = 2.6f;
  selected_ = -1;
  ++hintsUsed_;
  score_ = std::max(0, score_ - 50);
  ctx.audio->play(Sfx::Hint);
}

void Game::useUndo(Ctx& ctx) {
  const auto pair = board_.undo();
  if (!pair) return;
  for (int i : {pair->first, pair->second}) {
    views_[i].appear = 1.0f;
    views_[i].pop = 1.0f;
  }
  selected_ = hintA_ = hintB_ = -1;
  combo_ = 0;
  score_ = std::max(0, score_ - 100);
  ctx.audio->play(Sfx::Select, 0.8f);
  afterBoardChange(ctx);
}

void Game::useShuffle(Ctx& ctx) {
  const bool ok = board_.shuffle(randomSeed());
  ++shufflesUsed_;
  selected_ = hintA_ = hintB_ = -1;
  score_ = std::max(0, score_ - 200);
  dealAnimation(0.0f);
  ctx.audio->play(Sfx::Shuffle);
  deadEnd_ = !ok;
  afterBoardChange(ctx);
}

void Game::win(Ctx& ctx) {
  state_ = State::Won;
  stateTime_ = 0.0f;
  confettiTimer_ = 0.0f;
  stars_ = 1;
  if (shufflesUsed_ <= 1 && hintsUsed_ <= 5) stars_ = 2;
  if (shufflesUsed_ == 0 && hintsUsed_ <= 1) stars_ = 3;
  score_ += std::max(0, 3000 - static_cast<int>(elapsed_) * 4);
  ctx.save->record(allLayouts()[level_].id, stars_, static_cast<int>(elapsed_));
  ctx.audio->play(Sfx::Win);
}

bool Game::autoMove(Ctx& ctx) {
  if (state_ != State::Playing) return false;
  const auto hint = board_.findHint();
  if (!hint) return false;
  match(hint->first, hint->second, ctx);
  return true;
}

}  // namespace nm
