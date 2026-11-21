// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include <algorithm>
#include <cmath>

#include "game/Game.hpp"
#include "game/Options.hpp"
#include "game/Strings.hpp"
#include "ui/Ui.hpp"

namespace nm {
namespace {

constexpr float kRound = 0.17f;

float easeOut(float k) { return 1.0f - std::pow(1.0f - k, 3.0f); }
float smooth(float k) { return k * k * (3.0f - 2.0f * k); }

void dim() { DrawRectangle(-3000, -3000, 9000, 9000, Fade({4, 6, 14, 255}, 0.6f)); }

// Reparte `count` botones centrados en una fila y devuelve el rectángulo del i-ésimo.
Rectangle rowButton(int i, int count, float y) {
  const float w = 176.0f, gap = 16.0f;
  const float x0 = (kViewW - (count * w + (count - 1) * gap)) * 0.5f;
  return {x0 + i * (w + gap), y, w, 54.0f};
}

void stat(const Ctx& ctx, const char* label, const char* value, float cx) {
  ui::textCentered(ctx.ui, label, cx, 10.0f, 16.0f, ui::kMuted);
  ui::textCentered(ctx.ui, value, cx, 30.0f, 30.0f, ui::kText);
}

}  // namespace

GameAction Game::draw(Ctx& ctx) {
  drawBoard(ctx);
  drawGhosts(ctx);
  particles_.draw();
  for (const Popup& p : popups_) {
    const float alpha = 1.0f - smooth(std::clamp((p.t - 0.5f) / 0.7f, 0.0f, 1.0f));
    const float size = 34.0f + 10.0f * easeOut(std::min(1.0f, p.t * 5.0f));
    ui::textCentered(ctx.title, p.text.c_str(), p.pos.x + 2.0f, p.pos.y + 2.0f, size, Fade(BLACK, alpha * 0.5f));
    ui::textCentered(ctx.title, p.text.c_str(), p.pos.x, p.pos.y, size, Fade(p.color, alpha));
  }

  GameAction action = drawHud(ctx);
  if (state_ == State::Won && stateTime_ > 0.7f) {
    const GameAction a = drawWon(ctx);
    if (a != GameAction::None) action = a;
  } else if (state_ == State::Stuck && stateTime_ > 0.5f) {
    const GameAction a = drawStuck(ctx);
    if (a != GameAction::None) action = a;
  } else if (state_ == State::Paused) {
    const GameAction a = drawPaused(ctx);
    if (a != GameAction::None) action = a;
  } else if (state_ == State::TimeUp) {
    const GameAction a = drawTimeUp(ctx);
    if (a != GameAction::None) action = a;
  } else if (state_ == State::Replay) {
    const GameAction a = drawReplay(ctx);
    if (a != GameAction::None) action = a;
  }
  drawToast(ctx);
  // Abandonar una partida a medias corta la racha de victorias.
  if (action == GameAction::Restart && !finished()) {
    ctx.save->winStreak = 0;
    ctx.save->store();
  }
  return action;
}

void Game::drawBoard(const Ctx& ctx) const {
  const auto& tiles = board_.tiles();
  const float pulse = ctx.save->reduceFx ? 0.6f : 0.5f + 0.5f * std::sin(static_cast<float>(ctx.time) * 7.0f);

  for (int i : order_) {
    const Tile& t = tiles[i];
    const TileView& v = views_[i];
    if (t.removed || v.appear <= 0.0f) continue;

    const float e = easeOut(v.appear);
    Vector2 c = faceCenter(t.pos);
    c.y -= (1.0f - e) * 140.0f;
    c.x += std::sin(v.shake * 38.0f) * 5.0f * v.shake - v.lift * 1.5f;
    c.y -= v.lift * 3.5f;
    const float width = tileW_ * (1.0f + 0.2f * std::sin(v.pop * 3.1416f));
    const Rectangle face = {c.x - tileW_ * 0.5f, c.y - tileH_ * 0.5f, tileW_, tileH_};

    // Sombra sobre la mesa o sobre las fichas de debajo.
    const float shadow = depth_ + 5.0f + v.lift * 4.0f;
    DrawRectangleRounded({face.x + shadow, face.y + shadow, tileW_, tileH_}, kRound, 6, Fade(BLACK, 0.2f * e));

    const bool usable = board_.isFree(i);
    const bool dimmed = !usable && ctx.save->highlightFree;
    const unsigned char g = dimmed ? 214 : 255;
    ctx.atlas->draw(t.face, c, width, 0.0f, {g, g, static_cast<unsigned char>(dimmed ? 224 : 255),
                                             static_cast<unsigned char>(255 * std::min(1.0f, e * 1.4f))});

    if (i == selected_) {
      DrawRectangleRounded(face, kRound, 8, Fade(ui::kGold, 0.2f + 0.12f * pulse));
      DrawRectangleRoundedLinesEx(face, kRound, 8, 4.0f, Fade(ui::kGold, 0.85f + 0.15f * pulse));
    } else if (i == hintA_ || i == hintB_) {
      DrawRectangleRounded(face, kRound, 8, Fade({120, 230, 255, 255}, 0.16f + 0.2f * pulse));
      DrawRectangleRoundedLinesEx(face, kRound, 8, 4.0f + 2.0f * pulse, Fade({120, 230, 255, 255}, 0.9f));
    } else if (i == hover_ && usable) {
      DrawRectangleRoundedLinesEx(face, kRound, 8, cursorActive_ ? 5.0f : 2.5f, Fade(WHITE, cursorActive_ ? 0.95f : 0.75f));
    }
  }
}

void Game::drawGhosts(const Ctx& ctx) const {
  for (const Ghost& g : ghosts_) {
    const float k = smooth(std::clamp(g.t, 0.0f, 1.0f));
    const float arc = std::sin(g.t * 3.1416f);
    const Vector2 c = {g.from.x + (g.to.x - g.from.x) * k, g.from.y + (g.to.y - g.from.y) * k - arc * 46.0f};
    const float alpha = g.t < 0.75f ? 1.0f : 1.0f - (g.t - 0.75f) / 0.25f;
    const bool calm = ctx.save->reduceFx;
    const float spin = calm ? 0.0f : (g.burst ? -1.0f : 1.0f) * 14.0f * k;
    if (!calm) {
      BeginBlendMode(BLEND_ADDITIVE);
      DrawCircleV(c, tileW_ * (0.7f + 0.5f * arc), Fade(ui::kGold, 0.18f * alpha));
      EndBlendMode();
    }
    ctx.atlas->draw(g.face, c, tileW_ * (1.0f + 0.28f * arc), spin,
                    Fade(WHITE, std::clamp(alpha, 0.0f, 1.0f)));
  }
}

GameAction Game::drawHud(Ctx& ctx) {
  GameAction action = GameAction::None;
  DrawRectangle(-3000, -3000, 9000, 3076, Fade({8, 12, 26, 255}, 0.62f));
  DrawRectangle(-3000, 76, 9000, 2, Fade(WHITE, 0.14f));

  const bool playing = state_ == State::Playing;
  if (ui::button(ctx, {20.0f, 14.0f, 128.0f, 48.0f}, tr(Str::Menu))) action = GameAction::Menu;
  ui::text(ctx.title, title(), 172.0f, 19.0f, 38.0f, ui::kText);

  stat(ctx, tr(Str::Pairs), TextFormat("%d", board_.remaining() / 2), 560.0f);
  stat(ctx, tr(Str::Moves), TextFormat("%d", moves_), 700.0f);
  if (style_ == PlayStyle::Zen) {
    stat(ctx, tr(Str::OptStyle), tr(Str::Zen), 910.0f);
  } else {
    // En contrarreloj el reloj va hacia atrás y avisa en rojo al final.
    const bool timed = style_ == PlayStyle::Timed;
    const int seconds = static_cast<int>(timed ? std::ceil(timeLeft_) : elapsed_);
    ui::textCentered(ctx.ui, tr(Str::Time), 840.0f, 10.0f, 16.0f, ui::kMuted);
    ui::textCentered(ctx.ui, TextFormat("%02d:%02d", seconds / 60, seconds % 60), 840.0f, 30.0f, 30.0f,
                     timed && timeLeft_ < 20.0f ? Color{255, 110, 110, 255} : ui::kText);
    stat(ctx, tr(Str::Score), TextFormat("%d", score_), 980.0f);
  }

  // Con dificultad normal o difícil las ayudas son limitadas y se ve cuántas quedan.
  const char* hintLabel = hintLimit_ < 0 ? tr(Str::Hint) : TextFormat("%s (%d)", tr(Str::Hint), hintsLeft());
  if (ui::button(ctx, {1098.0f, 14.0f, 150.0f, 48.0f}, hintLabel, playing && hintsLeft() != 0)) useHint(ctx);
  if (ui::button(ctx, {1260.0f, 14.0f, 156.0f, 48.0f}, tr(Str::Undo), playing && board_.canUndo())) useUndo(ctx);
  const char* shuffleLabel =
      shuffleLimit_ < 0 ? tr(Str::Shuffle) : TextFormat("%s (%d)", tr(Str::Shuffle), shufflesLeft());
  if (ui::button(ctx, {1428.0f, 14.0f, 152.0f, 48.0f}, shuffleLabel, playing && shufflesLeft() != 0))
    useShuffle(ctx);

  if (combo_ >= 2 && playing) {
    const float k = std::clamp(comboTimer_ / 5.0f, 0.0f, 1.0f);
    ui::textCentered(ctx.title, TextFormat("%s x%d", tr(Str::Combo), combo_), kViewW * 0.5f, 86.0f, 30.0f,
                     Fade(ui::kGold, 0.5f + 0.5f * k));
    DrawRectangleRounded({kViewW * 0.5f - 70.0f * k, 122.0f, 140.0f * k, 5.0f}, 1.0f, 4, Fade(ui::kGold, 0.8f));
  }
  return action;
}

GameAction Game::drawWon(Ctx& ctx) {
  GameAction action = GameAction::None;
  // En aleatorio siempre hay «siguiente» (otro tablero); el reto diario es uno al día.
  const bool last = kind_ == BoardKind::Daily ||
                    (kind_ == BoardKind::Level && level_ + 1 >= static_cast<int>(allLayouts().size()));
  dim();
  particles_.draw();  // el confeti queda por encima del velo
  ui::panel({470.0f, 200.0f, 660.0f, 470.0f}, 0.9f);
  ui::textCentered(ctx.title, tr(Str::LevelComplete), kViewW * 0.5f, 232.0f, 56.0f, ui::kGold);

  for (int s = 0; s < 3; ++s) {
    const float k = easeOut(std::clamp((stateTime_ - 0.9f - s * 0.28f) / 0.35f, 0.0f, 1.0f));
    const float cx = kViewW * 0.5f + (s - 1) * 120.0f;
    ui::star(cx, 380.0f, 50.0f, Fade(WHITE, 0.12f));
    if (s < stars_ && k > 0.0f) ui::star(cx, 380.0f, 50.0f * (1.6f - 0.6f * k), Fade(ui::kGold, k));
  }

  const int seconds = static_cast<int>(elapsed_);
  if (style_ != PlayStyle::Zen)
    ui::textCentered(ctx.ui,
                     TextFormat("%s %02d:%02d     %s %d", tr(Str::Time), seconds / 60, seconds % 60,
                                tr(Str::Score), score_),
                     kViewW * 0.5f, 468.0f, 28.0f, ui::kText);
  if (kind_ == BoardKind::Daily)
    ui::textCentered(ctx.ui, TextFormat(tr(Str::Streak), ctx.save->dailyStreak), kViewW * 0.5f, 516.0f, 24.0f, ui::kGold);
  else if (newRecord_)
    ui::textCentered(ctx.title, tr(Str::NewRecord), kViewW * 0.5f, 510.0f, 34.0f, ui::kGold);
  else if (last)
    ui::textCentered(ctx.ui, tr(Str::GameComplete), kViewW * 0.5f, 516.0f, 22.0f, ui::kMuted);

  const int count = last ? 2 : 3;
  if (ui::button(ctx, rowButton(0, count, 580.0f), tr(Str::Menu))) action = GameAction::Menu;
  if (ui::button(ctx, rowButton(1, count, 580.0f), tr(Str::Retry))) action = GameAction::Restart;
  if (!last && ui::button(ctx, rowButton(2, count, 580.0f), tr(Str::Next))) action = GameAction::Next;
  return action;
}

GameAction Game::drawStuck(Ctx& ctx) {
  GameAction action = GameAction::None;
  dim();
  ui::panel({300.0f, 270.0f, 1000.0f, 330.0f}, 0.9f);
  ui::textCentered(ctx.title, tr(Str::NoMoves), kViewW * 0.5f, 304.0f, 54.0f, ui::kText);
  ui::textCentered(ctx.ui, tr(deadEnd_ ? Str::NoMovesDead : Str::NoMovesBody), kViewW * 0.5f, 392.0f, 24.0f,
                   ui::kMuted);

  const float y = 486.0f;
  if (ui::button(ctx, rowButton(0, 5, y), tr(Str::Shuffle), !deadEnd_ && shufflesLeft() != 0)) useShuffle(ctx);
  if (ui::button(ctx, rowButton(1, 5, y), tr(Str::Undo), board_.canUndo())) useUndo(ctx);
  if (ui::button(ctx, rowButton(2, 5, y), tr(Str::ShowSolution), dealSeed_ != 0)) startReplay(ctx);
  if (ui::button(ctx, rowButton(3, 5, y), tr(Str::Restart))) action = GameAction::Restart;
  if (ui::button(ctx, rowButton(4, 5, y), tr(Str::Menu))) action = GameAction::Menu;
  return action;
}

GameAction Game::drawPaused(Ctx& ctx) {
  GameAction action = GameAction::None;
  // Velo casi opaco: con el reloj parado no se puede estudiar el tablero.
  DrawRectangle(-3000, 78, 9000, 9000, Fade({4, 6, 14, 255}, 0.93f));
  if (options_) {
    const Rectangle box = {250.0f, 92.0f, 1100.0f, 716.0f};
    ui::panel(box, 0.95f);
    ui::textCentered(ctx.title, tr(Str::Options), kViewW * 0.5f, box.y + 22.0f, 46.0f, ui::kGold);
    drawOptionsPanel(ctx, box, true);
    const Rectangle close = {kViewW * 0.5f - 110.0f, box.y + box.height - 66.0f, 220.0f, 48.0f};
    if (ui::button(ctx, close, tr(Str::Close))) options_ = false;
    return action;
  }
  ui::panel({380.0f, 290.0f, 840.0f, 290.0f}, 0.9f);
  ui::textCentered(ctx.title, tr(Str::Paused), kViewW * 0.5f, 322.0f, 60.0f, ui::kText);
  ui::textCentered(ctx.ui, title(), kViewW * 0.5f, 404.0f, 24.0f, ui::kMuted);
  const float y = 472.0f;
  if (ui::button(ctx, rowButton(0, 4, y), tr(Str::Resume))) togglePause();
  if (ui::button(ctx, rowButton(1, 4, y), tr(Str::Options))) options_ = true;
  if (ui::button(ctx, rowButton(2, 4, y), tr(Str::Restart))) action = GameAction::Restart;
  if (ui::button(ctx, rowButton(3, 4, y), tr(Str::Menu))) action = GameAction::Menu;
  return action;
}

GameAction Game::drawTimeUp(Ctx& ctx) {
  GameAction action = GameAction::None;
  dim();
  ui::panel({470.0f, 280.0f, 660.0f, 300.0f}, 0.9f);
  ui::textCentered(ctx.title, tr(Str::TimeUp), kViewW * 0.5f, 312.0f, 56.0f, {255, 130, 130, 255});
  ui::textCentered(ctx.ui, tr(Str::TimeUpBody), kViewW * 0.5f, 400.0f, 24.0f, ui::kMuted);
  if (ui::button(ctx, rowButton(0, 3, 476.0f), tr(Str::Retry))) action = GameAction::Restart;
  if (ui::button(ctx, rowButton(1, 3, 476.0f), tr(Str::ShowSolution), dealSeed_ != 0)) startReplay(ctx);
  if (ui::button(ctx, rowButton(2, 3, 476.0f), tr(Str::Menu))) action = GameAction::Menu;
  return action;
}

GameAction Game::drawReplay(Ctx& ctx) {
  GameAction action = GameAction::None;
  if (replayStep_ < board_.solution().size() || stateTime_ < 1.0f) {
    // Mientras se resuelve solo, un rótulo discreto bajo el marcador.
    ui::textCentered(ctx.title, tr(Str::Solution), kViewW * 0.5f + 2.0f, 88.0f, 32.0f, Fade(BLACK, 0.5f));
    ui::textCentered(ctx.title, tr(Str::Solution), kViewW * 0.5f, 86.0f, 32.0f, ui::kGold);
    return action;
  }
  dim();
  ui::panel({470.0f, 300.0f, 660.0f, 260.0f}, 0.9f);
  ui::textCentered(ctx.title, tr(Str::Solution), kViewW * 0.5f, 330.0f, 54.0f, ui::kGold);
  ui::textCentered(ctx.ui, tr(Str::SolutionBody), kViewW * 0.5f, 408.0f, 24.0f, ui::kMuted);
  if (ui::button(ctx, rowButton(0, 2, 466.0f), tr(Str::Retry))) action = GameAction::Restart;
  if (ui::button(ctx, rowButton(1, 2, 466.0f), tr(Str::Menu))) action = GameAction::Menu;
  return action;
}

void Game::drawToast(const Ctx& ctx) const {
  if (toastTime_ <= 0.0f || toast_.empty()) return;
  const float k = std::min({1.0f, toastTime_ * 2.0f, (4.5f - toastTime_) * 4.0f});
  const Rectangle box = {kViewW * 0.5f - 250.0f, 96.0f - (1.0f - k) * 30.0f, 500.0f, 78.0f};
  DrawRectangleRounded(box, 0.3f, 10, Fade({24, 34, 60, 255}, 0.95f * k));
  DrawRectangleRoundedLinesEx(box, 0.3f, 10, 2.5f, Fade(ui::kGold, k));
  ui::star(box.x + 44.0f, box.y + 39.0f, 20.0f, Fade(ui::kGold, k));
  ui::text(ctx.ui, tr(Str::AchievementUnlocked), box.x + 80.0f, box.y + 12.0f, 17.0f, Fade(ui::kMuted, k));
  ui::text(ctx.title, toast_.c_str(), box.x + 80.0f, box.y + 33.0f, 30.0f, Fade(ui::kText, k));
}

const char* Game::title() const {
  if (kind_ == BoardKind::Random) return tr(Str::Random);
  if (kind_ == BoardKind::Daily) return tr(Str::Daily);
  return TextFormat("%d · %s", level_ + 1, levelName(level_));
}

}  // namespace nm
