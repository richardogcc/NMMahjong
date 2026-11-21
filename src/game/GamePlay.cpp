// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Reglas de la partida, entrada del jugador y progreso (estadísticas y logros).
#include <algorithm>
#include <cmath>

#include "game/Achievements.hpp"
#include "game/Game.hpp"
#include "game/Strings.hpp"
#include "ui/Ui.hpp"

namespace nm {
namespace {

constexpr float kGhostTime = 0.45f;
constexpr float kComboWindow = 5.0f;
constexpr float kReplayStep = 0.42f;

std::uint32_t randomSeed() { return static_cast<std::uint32_t>(GetRandomValue(1, 2000000000)); }

}  // namespace

void Game::update(Ctx& ctx) {
  const float dt = ctx.dt;
  introTime_ += dt;
  stateTime_ += dt;
  toastTime_ = std::max(0.0f, toastTime_ - dt);
  particles_.update(dt);

  for (Ghost& g : ghosts_) {
    g.t += dt / kGhostTime;
    if (g.t >= 1.0f && g.burst && !ctx.save->reduceFx) {
      particles_.burst(g.to, levelTheme(themeIndex_).mote, 34, 420.0f);
      particles_.burst(g.to, ui::kGold, 14, 260.0f);
    }
  }
  std::erase_if(ghosts_, [](const Ghost& g) { return g.t >= 1.0f; });
  for (Popup& p : popups_) {
    p.t += dt;
    p.pos.y -= 46.0f * dt;
  }
  std::erase_if(popups_, [](const Popup& p) { return p.t >= 1.2f; });

  // El cursor de teclado o mando manda hasta que se mueve el ratón.
  const Nav& nav = ctx.nav;
  const Vector2 delta = GetMouseDelta();
  if (std::fabs(delta.x) + std::fabs(delta.y) > 0.5f || ctx.click) cursorActive_ = false;
  if (state_ == State::Playing && (nav.up || nav.down || nav.left || nav.right)) {
    cursorActive_ = true;
    moveCursor(nav.right - nav.left, nav.down - nav.up);
  }
  if (state_ != State::Playing) {
    hover_ = -1;
  } else {
    hover_ = cursorActive_ ? cursor_ : pick(ctx.mouse);
  }

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

    if (style_ == PlayStyle::Timed) {
      timeLeft_ -= dt;
      if (timeLeft_ <= 0.0f) {
        timeLeft_ = 0.0f;
        state_ = State::TimeUp;
        stateTime_ = 0.0f;
        ctx.save->winStreak = 0;
        ctx.save->store();
        clearSavedGame();
        ctx.audio->play(Sfx::Error, 0.7f);
        return;
      }
    }

    if (nav.hint && hintsLeft() != 0) useHint(ctx);
    if (nav.undo && board_.canUndo()) useUndo(ctx);
    if (nav.shuffle && shufflesLeft() != 0) useShuffle(ctx);
    if (state_ != State::Playing) return;  // barajar puede dejar la partida sin salida
    if (ctx.click && ctx.mouse.y > 84.0f) handleClick(pick(ctx.mouse), ctx);
    else if (nav.accept && cursorActive_ && cursor_ >= 0) handleClick(cursor_, ctx);
  } else if (state_ == State::Replay) {
    const auto& moves = board_.solution();
    replayTimer_ -= dt;
    if (replayTimer_ <= 0.0f && replayStep_ < moves.size()) {
      const auto [a, b] = moves[replayStep_];
      flyPair(a, b, ctx);
      if (board_.removePair(a, b)) ctx.audio->playNote(static_cast<int>(replayStep_ % 5));
      moves_ = board_.availableMoves();
      replayTimer_ = kReplayStep;
      if (++replayStep_ >= moves.size()) stateTime_ = 0.0f;  // al acabar sale el panel final
    }
  } else if (state_ == State::Won) {
    confettiTimer_ -= dt;
    if (confettiTimer_ <= 0.0f && stateTime_ < 5.0f && !ctx.save->reduceFx) {
      confettiTimer_ = 0.45f;
      particles_.confetti({static_cast<float>(GetRandomValue(250, 1350)), 900.0f}, 46);
    }
  }
}

// Mueve el cursor a la ficha libre más cercana en la dirección pedida.
void Game::moveCursor(int dx, int dy) {
  const auto& tiles = board_.tiles();
  const bool valid = cursor_ >= 0 && cursor_ < static_cast<int>(tiles.size()) && !tiles[cursor_].removed;
  const Vector2 from = valid ? faceCenter(tiles[cursor_].pos) : Vector2{kViewW * 0.5f, 480.0f};
  int best = -1;
  float bestScore = 1e9f;
  for (int i = 0; i < static_cast<int>(tiles.size()); ++i) {
    if (i == cursor_ || !board_.isFree(i)) continue;
    const Vector2 c = faceCenter(tiles[i].pos);
    const float ax = (c.x - from.x) * dx + (c.y - from.y) * dy;        // avance en la dirección
    const float side = std::fabs((c.x - from.x) * dy) + std::fabs((c.y - from.y) * dx);  // desvío lateral
    if (valid && ax < 4.0f) continue;
    const float score = (valid ? ax : std::fabs(ax)) + side * 2.2f;
    if (score < bestScore) {
      bestScore = score;
      best = i;
    }
  }
  if (best >= 0) cursor_ = best;
  else if (!valid) cursor_ = -1;
}

void Game::handleClick(int index, Ctx& ctx) {
  if (index < 0) {
    selected_ = -1;
    return;
  }
  if (!board_.isFree(index)) {
    if (!ctx.save->reduceFx) views_[index].shake = 1.0f;
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
  if (!board_.canMatch(a, b)) return;
  flyPair(a, b, ctx);
  board_.removePair(a, b);

  combo_ = comboTimer_ > 0.0f ? std::min(combo_ + 1, 9) : 1;
  comboTimer_ = kComboWindow;
  if (style_ != PlayStyle::Zen) {
    const int points = 100 * combo_;
    score_ += points;
    popups_.push_back({{mid.x, mid.y - 30.0f}, TextFormat("+%d", points), 0.0f, WHITE});
  }
  if (combo_ >= 2)
    popups_.push_back({{mid.x, mid.y - 68.0f}, TextFormat("%s x%d", tr(Str::Combo), combo_), 0.0f, ui::kGold});
  if (style_ == PlayStyle::Timed) timeLeft_ += 1.0f;

  // Cada pareja suena una nota; encadenadas, suben por la escala.
  ctx.audio->playNote(combo_ - 1);
  ctx.audio->play(Sfx::Match, 1.0f + 0.06f * (combo_ - 1));

  SaveData& save = *ctx.save;
  ++save.pairs;
  save.maxCombo = std::max(save.maxCombo, combo_);
  if (combo_ >= 9) unlock("combo9", ctx);

  selected_ = hintA_ = hintB_ = -1;
  if (cursor_ == a || cursor_ == b) cursor_ = -1;
  afterBoardChange(ctx);
}

// Las dos fichas vuelan una hacia la otra antes de desaparecer.
void Game::flyPair(int a, int b, const Ctx& ctx) {
  const auto& tiles = board_.tiles();
  const Vector2 ca = faceCenter(tiles[a].pos), cb = faceCenter(tiles[b].pos);
  const Vector2 mid = {(ca.x + cb.x) * 0.5f, (ca.y + cb.y) * 0.5f};
  ghosts_.push_back({tiles[a].face, ca, mid, 0.0f, true});
  ghosts_.push_back({tiles[b].face, cb, mid, 0.0f, false});
  if (ctx.save->reduceFx) return;
  particles_.burst(ca, WHITE, 8, 160.0f);
  particles_.burst(cb, WHITE, 8, 160.0f);
}

void Game::startReplay(Ctx& ctx) {
  Board fresh;
  if (dealSeed_ == 0 || !fresh.generate(layout(), dealSeed_, dealDifficulty_)) return;
  board_ = fresh;
  views_.assign(board_.tiles().size(), TileView{});
  for (TileView& v : views_) v.appear = 1.0f;
  ghosts_.clear();
  popups_.clear();
  selected_ = hover_ = hintA_ = hintB_ = cursor_ = -1;
  combo_ = 0;
  state_ = State::Replay;
  stateTime_ = 0.0f;
  replayStep_ = 0;
  replayTimer_ = 0.9f;
  moves_ = board_.availableMoves();
  clearSavedGame();
  ctx.save->winStreak = 0;  // ver la solución es rendirse
  ctx.save->store();
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
  if (!hint || hintsLeft() == 0) return;
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
  if (shufflesLeft() == 0) return;
  const bool ok = board_.shuffle(randomSeed());
  ++shufflesUsed_;
  selected_ = hintA_ = hintB_ = cursor_ = -1;
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
  if (style_ != PlayStyle::Zen) score_ += std::max(0, 3000 - static_cast<int>(elapsed_) * 4);
  clearSavedGame();
  ctx.audio->play(Sfx::Win);

  SaveData& save = *ctx.save;
  ++save.won;
  save.winStreak += 1;
  save.bestWinStreak = std::max(save.bestWinStreak, save.winStreak);
  save.secondsWon += static_cast<int>(elapsed_);

  unlock("first", ctx);
  if (hintsUsed_ == 0 && shufflesUsed_ == 0) unlock("clean", ctx);
  if (style_ == PlayStyle::Timed) unlock("timed", ctx);
  if (difficulty_ == Difficulty::Hard && kind_ != BoardKind::Daily) unlock("hard", ctx);

  if (kind_ == BoardKind::Level) {
    LevelRecord& rec = save.levels[allLayouts()[level_].id];
    rec.stars = std::max(rec.stars, stars_);
    // En zen no hay reloj ni puntos: no cuenta para las marcas.
    if (style_ != PlayStyle::Zen) {
      const int seconds = static_cast<int>(elapsed_);
      newRecord_ = (rec.bestSeconds > 0 && seconds < rec.bestSeconds) || (rec.bestScore > 0 && score_ > rec.bestScore);
      if (rec.bestSeconds == 0 || seconds < rec.bestSeconds) rec.bestSeconds = seconds;
      rec.bestScore = std::max(rec.bestScore, score_);
    }
    bool all = true, perfect = true;
    for (const Layout& l : allLayouts()) {
      const auto it = save.levels.find(l.id);
      all = all && it != save.levels.end() && it->second.stars > 0;
      perfect = perfect && it != save.levels.end() && it->second.stars == 3;
    }
    if (all) unlock("all", ctx);
    if (perfect) unlock("stars", ctx);
  } else if (kind_ == BoardKind::Random) {
    if (++save.randomWon >= 10) unlock("random10", ctx);
  } else {
    const int today = todayDate();
    if (save.dailyDate != today) {
      save.dailyStreak = save.dailyDate == previousDate(today) ? save.dailyStreak + 1 : 1;
      save.bestDailyStreak = std::max(save.bestDailyStreak, save.dailyStreak);
      save.dailyDate = today;
    }
    if (save.dailyStreak >= 3) unlock("daily3", ctx);
    if (save.dailyStreak >= 7) unlock("daily7", ctx);
  }
  save.store();
}

void Game::unlock(const char* id, Ctx& ctx) {
  if (!ctx.save->achievements.insert(id).second) return;
  for (const Achievement& a : kAchievements)
    if (std::string(a.id) == id) toast_ = tr(a.title);
  toastTime_ = 4.5f;
  ctx.save->store();
}

bool Game::autoMove(Ctx& ctx) {
  if (state_ != State::Playing) return false;
  const auto hint = board_.findHint();
  if (!hint) return false;
  match(hint->first, hint->second, ctx);
  return true;
}

}  // namespace nm
