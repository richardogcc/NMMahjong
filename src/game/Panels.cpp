// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Paneles del menú: opciones, estadísticas, logros y tutorial.
#include <algorithm>
#include <cmath>

#include "game/Achievements.hpp"
#include "game/Menu.hpp"
#include "game/Options.hpp"
#include "game/Strings.hpp"
#include "ui/Ui.hpp"

namespace nm {

void Menu::drawPanel(Ctx& ctx) {
  // El clic que abre un panel no debe pulsar además algo de dentro.
  static Panel shown = Panel::None;
  const bool fresh = panel_ != shown;
  shown = panel_;
  if (panel_ == Panel::None) return;
  const bool click = ctx.click;
  if (fresh) ctx.click = false;

  DrawRectangle(-3000, -3000, 9000, 9000, Fade({4, 6, 14, 255}, 0.72f));
  const Rectangle box = {250.0f, 92.0f, 1100.0f, 716.0f};
  ui::panel(box, 0.95f);
  const Str titles[] = {Str::Options, Str::Options, Str::Stats, Str::Achievements, Str::HowToPlay};
  ui::textCentered(ctx.title, tr(titles[static_cast<int>(panel_)]), kViewW * 0.5f, box.y + 22.0f, 46.0f, ui::kGold);

  const Panel current = panel_;
  if (current == Panel::Options) drawOptions(ctx, box);
  if (current == Panel::Stats) drawStats(ctx, box);
  if (current == Panel::Achievements) drawAchievements(ctx, box);
  if (current == Panel::Tutorial) drawTutorial(ctx, box);

  if (current != Panel::Tutorial) {
    const Rectangle close = {kViewW * 0.5f - 110.0f, box.y + box.height - 66.0f, 220.0f, 48.0f};
    if (ui::button(ctx, close, tr(Str::Close)) || ctx.nav.back) panel_ = Panel::None;
  }
  ctx.click = click;
}

void Menu::drawOptions(Ctx& ctx, Rectangle box) { drawOptionsPanel(ctx, box, false); }

void Menu::drawStats(const Ctx& ctx, Rectangle box) const {
  const SaveData& s = *ctx.save;
  const int rate = s.played > 0 ? static_cast<int>(std::round(100.0f * s.won / s.played)) : 0;
  const int average = s.won > 0 ? s.secondsWon / s.won : 0;
  const struct {
    Str label;
    const char* value;
  } rows[] = {
      {Str::StPlayed, TextFormat("%d", s.played)},
      {Str::StWon, TextFormat("%d", s.won)},
      {Str::StWinRate, TextFormat("%d %%", rate)},
      {Str::StStreak, TextFormat("%d", s.winStreak)},
  };
  // TextFormat reutiliza cuatro memorias: se dibuja en dos tandas.
  float y = box.y + 110.0f;
  auto line = [&](Str label, const char* value) {
    DrawRectangleRounded({box.x + 150.0f, y - 6.0f, box.width - 300.0f, 46.0f}, 0.3f, 6, Fade(WHITE, 0.05f));
    ui::text(ctx.ui, tr(label), box.x + 180.0f, y + 4.0f, 24.0f, ui::kText);
    ui::text(ctx.title, value, box.x + box.width - 180.0f - ui::textWidth(ctx.title, value, 30.0f), y, 30.0f, ui::kGold);
    y += 56.0f;
  };
  for (const auto& r : rows) line(r.label, r.value);
  line(Str::StBestStreak, TextFormat("%d", s.bestWinStreak));
  line(Str::StAvgTime, TextFormat("%02d:%02d", average / 60, average % 60));
  line(Str::StPairs, TextFormat("%d", s.pairs));
  line(Str::StMaxCombo, TextFormat("x%d", s.maxCombo));
  line(Str::StDailyBest, TextFormat("%d", s.bestDailyStreak));
}

void Menu::drawAchievements(const Ctx& ctx, Rectangle box) const {
  int i = 0;
  for (const Achievement& a : kAchievements) {
    const bool got = ctx.save->achievements.count(a.id) > 0;
    const Rectangle cell = {box.x + 50.0f + (i % 2) * 505.0f, box.y + 96.0f + (i / 2) * 106.0f, 495.0f, 94.0f};
    DrawRectangleRounded(cell, 0.25f, 8, Fade(got ? ui::kGold : WHITE, got ? 0.12f : 0.05f));
    ui::star(cell.x + 48.0f, cell.y + 47.0f, 26.0f, got ? ui::kGold : Fade(WHITE, 0.16f));
    ui::text(ctx.title, tr(a.title), cell.x + 92.0f, cell.y + 14.0f, 28.0f, got ? ui::kText : Fade(ui::kText, 0.55f));
    ui::text(ctx.ui, tr(a.description), cell.x + 92.0f, cell.y + 52.0f, 17.0f, got ? ui::kMuted : Fade(ui::kMuted, 0.6f));
    ++i;
  }
}

void Menu::drawTutorial(Ctx& ctx, Rectangle box) {
  const Str pageTitle[4] = {Str::Tut1T, Str::Tut2T, Str::Tut3T, Str::Tut4T};
  const Str pageBody[4] = {Str::Tut1B, Str::Tut2B, Str::Tut3B, Str::Tut4B};
  const int page = std::clamp(tutorialPage_, 0, 3);
  const float cx = kViewW * 0.5f, cy = box.y + 260.0f, w = 96.0f;

  // Una ilustración sencilla con fichas de verdad.
  auto tile = [&](int face, float x, bool dimmed) {
    ctx.atlas->draw(face, {x, cy}, w, 0.0f, dimmed ? Color{150, 150, 160, 255} : WHITE);
  };
  if (page == 0) {
    tile(31, cx - 90.0f, false);
    tile(31, cx + 90.0f, false);
    ui::textCentered(ctx.title, "=", cx, cy - 34.0f, 64.0f, ui::kGold);
  } else if (page == 1) {
    tile(4, cx - w, false);
    tile(22, cx, true);
    tile(13, cx + w, false);
    ui::textCentered(ctx.title, "x", cx, cy - 150.0f, 48.0f, {255, 120, 120, 255});
  } else if (page == 2) {
    tile(34, cx - 230.0f, false);
    tile(36, cx - 120.0f, false);
    tile(38, cx + 120.0f, false);
    tile(41, cx + 230.0f, false);
  } else {
    const char* keys[4] = {"H", "Z", "B", "Esc"};
    for (int i = 0; i < 4; ++i) {
      const Rectangle key = {cx - 250.0f + i * 130.0f, cy - 44.0f, 110.0f, 88.0f};
      DrawRectangleRounded(key, 0.25f, 8, Fade(WHITE, 0.12f));
      DrawRectangleRoundedLinesEx(key, 0.25f, 8, 2.5f, Fade(WHITE, 0.5f));
      ui::textCentered(ctx.title, keys[i], key.x + key.width * 0.5f, key.y + 22.0f, 42.0f, ui::kText);
    }
  }

  ui::textCentered(ctx.title, tr(pageTitle[page]), cx, box.y + 400.0f, 40.0f, ui::kText);
  ui::textWrapped(ctx.ui, tr(pageBody[page]), box.x + 170.0f, box.y + 462.0f, box.width - 340.0f, 24.0f, ui::kMuted, true);
  for (int i = 0; i < 4; ++i)
    DrawCircleV({cx - 36.0f + i * 24.0f, box.y + box.height - 96.0f}, 6.0f, i == page ? ui::kGold : Fade(WHITE, 0.25f));

  const bool last = page == 3;
  const Rectangle next = {cx - 110.0f, box.y + box.height - 66.0f, 220.0f, 48.0f};
  if (ui::button(ctx, next, tr(last ? Str::Close : Str::Next)) || ctx.nav.accept) {
    if (!last) {
      ++tutorialPage_;
    } else {
      panel_ = Panel::None;
      tutorialPage_ = 0;
      ctx.save->tutorialDone = true;
      ctx.save->store();
    }
  } else if (ctx.nav.back) {
    panel_ = Panel::None;
    ctx.save->tutorialDone = true;
    ctx.save->store();
  }
}

}  // namespace nm
