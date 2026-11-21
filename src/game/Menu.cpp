// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "game/Menu.hpp"

#include <algorithm>
#include <cmath>

#include "core/Layout.hpp"
#include "game/Strings.hpp"
#include "ui/Ui.hpp"

namespace nm {
namespace {

constexpr float kCardW = 262.0f;
constexpr float kCardH = 152.0f;
constexpr float kGapX = 25.0f;
constexpr float kGapY = 16.0f;
constexpr float kTop = 212.0f;
constexpr int kColumns = 5;
constexpr int kSlots = 15;  // 12 niveles, aleatorio, reto diario y continuar

void drawTitle(const Ctx& ctx) {
  const char* title = "NM MAHJONG";
  const float size = 108.0f;
  float x = (kViewW - ui::textWidth(ctx.logo, title, size)) * 0.5f;
  for (int i = 0; title[i] != '\0'; ++i) {
    const char letter[2] = {title[i], '\0'};
    const float bob = ctx.save->reduceFx ? 0.0f : std::sin(static_cast<float>(ctx.time) * 1.6f + i * 0.5f) * 5.0f;
    ui::text(ctx.logo, letter, x + 4.0f, 40.0f + bob + 6.0f, size, Fade(BLACK, 0.35f));
    ui::text(ctx.logo, letter, x, 40.0f + bob, size, i < 2 ? ui::kGold : ui::kText);
    x += ui::textWidth(ctx.logo, letter, size);
  }
  ui::textCentered(ctx.ui, tr(Str::Subtitle), kViewW * 0.5f + 2.0f, 162.0f, 26.0f, Fade(BLACK, 0.6f));
  ui::textCentered(ctx.ui, tr(Str::Subtitle), kViewW * 0.5f, 160.0f, 26.0f, ui::kText);
}

// Miniatura del tablero vista desde arriba.
void drawPreview(const Layout& layout, Rectangle area, float alpha) {
  int maxX = 0, maxY = 0;
  for (const Slot& s : layout.slots) {
    maxX = std::max(maxX, s.x + 2);
    maxY = std::max(maxY, s.y + 2);
  }
  const float u = std::min(area.width / maxX, area.height / maxY);
  const float ox = area.x + (area.width - maxX * u) * 0.5f;
  const float oy = area.y + (area.height - maxY * u) * 0.5f;
  for (int z = 0; z <= 4; ++z) {
    for (const Slot& s : layout.slots) {
      if (s.z != z) continue;
      const float lift = z * u * 0.35f;
      const Rectangle r = {ox + s.x * u - lift, oy + s.y * u - lift, 2.0f * u - 0.8f, 2.0f * u - 0.8f};
      const unsigned char shade = static_cast<unsigned char>(std::min(255, 178 + z * 20));
      DrawRectangleRounded({r.x + 1.2f, r.y + 1.2f, r.width, r.height}, 0.3f, 4, Fade({20, 90, 70, 255}, alpha));
      DrawRectangleRounded(r, 0.3f, 4, Fade({shade, shade, static_cast<unsigned char>(shade - 22), 255}, alpha));
    }
  }
}

void drawLock(float cx, float cy) {
  DrawRing({cx, cy - 12.0f}, 11.0f, 16.0f, 180.0f, 360.0f, 24, ui::kText);
  DrawRectangleRounded({cx - 20.0f, cy - 12.0f, 40.0f, 32.0f}, 0.3f, 8, ui::kText);
  DrawCircleV({cx, cy + 3.0f}, 4.5f, {12, 18, 34, 255});
}

void cardFrame(Rectangle& card, bool over, bool focused, bool golden) {
  if (over || focused) card.y -= 5.0f;
  ui::panel(card, over || focused ? 0.88f : 0.68f);
  if (over || focused) DrawRectangleRoundedLinesEx(card, 0.12f, 12, 3.0f, Fade(ui::kGold, 0.95f));
  else if (golden) DrawRectangleRoundedLinesEx(card, 0.12f, 12, 2.5f, Fade(ui::kGold, 0.4f));
}

}  // namespace

bool Menu::drawLevelCard(const Ctx& ctx, int index, Rectangle card, bool unlocked, bool focused) const {
  const Layout& layout = allLayouts()[index];
  const bool over = unlocked && panel_ == Panel::None && ui::hovered(ctx, card);
  cardFrame(card, over, focused && unlocked, false);

  drawPreview(layout, {card.x + 18.0f, card.y + 10.0f, kCardW - 36.0f, 74.0f}, unlocked ? 1.0f : 0.26f);
  ui::text(ctx.title, TextFormat("%d · %s", index + 1, levelName(index)), card.x + 16.0f, card.y + 90.0f, 26.0f,
           unlocked ? ui::kText : Fade(ui::kText, 0.5f));
  if (!unlocked) {
    drawLock(card.x + kCardW * 0.5f, card.y + 48.0f);
    return false;
  }

  LevelRecord rec;
  if (auto it = ctx.save->levels.find(layout.id); it != ctx.save->levels.end()) rec = it->second;
  for (int s = 0; s < 3; ++s)
    ui::star(card.x + 26.0f + s * 24.0f, card.y + 132.0f, 10.0f, s < rec.stars ? ui::kGold : Fade(WHITE, 0.18f));
  // Mejores marcas del nivel: tiempo y puntos. Sin ellas, el número de fichas.
  const char* info = TextFormat("%d %s", static_cast<int>(layout.slots.size()), tr(Str::Tiles));
  if (rec.bestSeconds > 0 && rec.bestScore > 0)
    info = TextFormat("%02d:%02d · %d", rec.bestSeconds / 60, rec.bestSeconds % 60, rec.bestScore);
  else if (rec.bestSeconds > 0)
    info = TextFormat("%s %02d:%02d", tr(Str::Best), rec.bestSeconds / 60, rec.bestSeconds % 60);
  ui::text(ctx.ui, info, card.x + kCardW - 16.0f - ui::textWidth(ctx.ui, info, 15.0f), card.y + 124.0f, 15.0f,
           ui::kMuted);
  return over && ctx.click;
}

// Tarjetas que no son niveles: 0 aleatorio, 1 reto diario, 2 continuar.
bool Menu::drawSpecialCard(const Ctx& ctx, int which, Rectangle card, bool enabled, bool focused) const {
  const bool over = enabled && panel_ == Panel::None && ui::hovered(ctx, card);
  cardFrame(card, over, focused && enabled, enabled);
  const float alpha = enabled ? 1.0f : 0.35f;
  const float t = static_cast<float>(ctx.time);
  const Vector2 icon = {card.x + 52.0f, card.y + 50.0f};

  const char* title = "";
  const char* hint = "";
  if (which == 0) {
    title = tr(Str::Random);
    hint = tr(Str::RandomHint);
    for (int i = 0; i < 2; ++i) {
      const float sway = ctx.save->reduceFx ? 0.0f : std::sin(t * 1.4f + i * 1.6f) * 3.0f;
      const Rectangle tile = {icon.x - 30.0f + i * 30.0f, icon.y - 28.0f + sway + i * 6.0f, 36.0f, 48.0f};
      DrawRectangleRounded({tile.x + 4.0f, tile.y + 4.0f, tile.width, tile.height}, 0.2f, 6, {30, 110, 86, 255});
      DrawRectangleRounded(tile, 0.2f, 6, {252, 247, 233, 255});
      ui::textCentered(ctx.logo, "?", tile.x + tile.width * 0.5f, tile.y + 8.0f, 34.0f,
                       i == 1 ? Color{204, 44, 54, 255} : Color{28, 52, 110, 255});
    }
  } else if (which == 1) {
    title = tr(Str::Daily);
    const bool done = ctx.save->dailyDate == todayDate();
    hint = done ? tr(Str::DailyDone) : tr(Str::DailyHint);
    // Hoja de calendario con el día de hoy.
    const Rectangle page = {icon.x - 28.0f, icon.y - 30.0f, 56.0f, 60.0f};
    DrawRectangleRounded(page, 0.2f, 6, {252, 247, 233, 255});
    DrawRectangleRounded({page.x, page.y, page.width, 18.0f}, 0.5f, 6, {204, 44, 54, 255});
    ui::textCentered(ctx.logo, TextFormat("%d", todayDate() % 100), icon.x, icon.y - 8.0f, 34.0f, {28, 52, 110, 255});
    if (done) ui::star(page.x + page.width, page.y + 4.0f, 13.0f, ui::kGold);
    if (ctx.save->dailyStreak > 0)
      ui::text(ctx.ui, TextFormat(tr(Str::Streak), ctx.save->dailyStreak), card.x + 16.0f, card.y + 124.0f, 15.0f,
               ui::kGold);
  } else {
    title = tr(Str::Continue);
    DrawCircleV(icon, 30.0f, Fade(ui::kGold, 0.9f * alpha));
    DrawTriangle({icon.x - 9.0f, icon.y - 15.0f}, {icon.x - 9.0f, icon.y + 15.0f}, {icon.x + 16.0f, icon.y},
                 Fade({12, 18, 34, 255}, alpha));
  }
  ui::text(ctx.title, title, card.x + 100.0f, card.y + 26.0f, 28.0f, Fade(which == 2 ? ui::kText : ui::kGold, alpha));
  // La pista puede ocupar dos líneas.
  ui::textWrapped(ctx.ui, hint, card.x + 100.0f, card.y + 62.0f, kCardW - 112.0f, 14.5f, Fade(ui::kText, 0.85f * alpha), false);
  return over && ctx.click;
}

MenuResult Menu::draw(Ctx& ctx) {
  MenuResult result;
  SaveData& save = *ctx.save;
  if (firstFrame_) {
    firstFrame_ = false;
    if (!save.tutorialDone) panel_ = Panel::Tutorial;  // la primera vez se explica el juego
  }
  drawTitle(ctx);

  // Teclado y mando: las flechas recorren las tarjetas y Enter abre.
  const auto& layouts = allLayouts();
  const bool canContinue = hasSavedGame();
  if (panel_ == Panel::None) {
    const Nav& nav = ctx.nav;
    const Vector2 delta = GetMouseDelta();
    if (std::fabs(delta.x) + std::fabs(delta.y) > 0.5f) focusActive_ = false;
    if (nav.left || nav.right || nav.up || nav.down) {
      focusActive_ = true;
      int col = focus_ % kColumns, row = focus_ / kColumns;
      col = std::clamp(col + nav.right - nav.left, 0, kColumns - 1);
      row = std::clamp(row + nav.down - nav.up, 0, kSlots / kColumns - 1);
      focus_ = row * kColumns + col;
    }
  }
  const bool accept = panel_ == Panel::None && focusActive_ && ctx.nav.accept;

  const float x0 = (kViewW - (kColumns * kCardW + (kColumns - 1) * kGapX)) * 0.5f;
  bool previousCleared = true;
  for (int i = 0; i < kSlots; ++i) {
    const Rectangle card = {x0 + (i % kColumns) * (kCardW + kGapX), kTop + (i / kColumns) * (kCardH + kGapY), kCardW,
                            kCardH};
    const bool focused = focusActive_ && focus_ == i;
    bool chosen = false;
    GameRequest request;
    if (i < static_cast<int>(layouts.size())) {
      const bool unlocked = previousCleared;
      chosen = drawLevelCard(ctx, i, card, unlocked, focused) || (focused && accept && unlocked);
      request = {BoardKind::Level, i, false};
      const auto it = save.levels.find(layouts[i].id);
      previousCleared = it != save.levels.end() && it->second.stars > 0;
    } else {
      const int which = i - static_cast<int>(layouts.size());
      const bool enabled = which != 2 || canContinue;
      chosen = drawSpecialCard(ctx, which, card, enabled, focused) || (focused && accept && enabled);
      request = which == 0 ? GameRequest{BoardKind::Random, 0, false}
                           : (which == 1 ? GameRequest{BoardKind::Daily, 0, false} : GameRequest{BoardKind::Level, 0, true});
    }
    if (chosen) {
      ctx.audio->play(Sfx::Select);
      result = {MenuResult::Kind::Play, request};
    }
  }

  // Barra inferior.
  const float y = 742.0f, barW = kViewW - 2.0f * x0;
  ui::panel({x0, y - 12.0f, barW, 70.0f}, 0.6f);
  const Str labels[5] = {Str::Options, Str::Stats, Str::Achievements, Str::HowToPlay, Str::Quit};
  const float w = (barW - 2.0f * 20.0f - 4.0f * 14.0f) / 5.0f;
  for (int i = 0; i < 5; ++i) {
    const Rectangle r = {x0 + 20.0f + i * (w + 14.0f), y, w, 46.0f};
    if (!ui::button(ctx, r, tr(labels[i]), panel_ == Panel::None, 22.0f)) continue;
    if (i == 0) panel_ = Panel::Options;
    if (i == 1) panel_ = Panel::Stats;
    if (i == 2) panel_ = Panel::Achievements;
    if (i == 3) {
      panel_ = Panel::Tutorial;
      tutorialPage_ = 0;
    }
    if (i == 4) result.kind = MenuResult::Kind::Quit;
  }
  ui::textCentered(ctx.ui, "© 2026 Richardo G. Cibea M.  ·  v" NM_VERSION, kViewW * 0.5f, 852.0f, 18.0f,
                   Fade(ui::kMuted, 0.7f));

  drawPanel(ctx);
  return result;
}

}  // namespace nm
