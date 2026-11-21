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
constexpr float kCardH = 230.0f;
constexpr float kGap = 25.0f;
constexpr int kColumns = 5;

void drawTitle(const Ctx& ctx) {
  const char* title = "NM MAHJONG";
  const float size = 112.0f;
  float x = (kViewW - ui::textWidth(ctx.title, title, size)) * 0.5f;
  for (int i = 0; title[i] != '\0'; ++i) {
    const char letter[2] = {title[i], '\0'};
    const float bob = std::sin(static_cast<float>(ctx.time) * 1.6f + i * 0.5f) * 6.0f;
    ui::text(ctx.title, letter, x + 4.0f, 44.0f + bob + 6.0f, size, Fade(BLACK, 0.35f));
    ui::text(ctx.title, letter, x, 44.0f + bob, size, i < 2 ? ui::kGold : ui::kText);
    x += ui::textWidth(ctx.title, letter, size);
  }
  ui::textCentered(ctx.ui, tr(Str::Subtitle), kViewW * 0.5f + 2.0f, 170.0f, 28.0f, Fade(BLACK, 0.6f));
  ui::textCentered(ctx.ui, tr(Str::Subtitle), kViewW * 0.5f, 168.0f, 28.0f, ui::kText);
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
      const Rectangle r = {ox + s.x * u - lift, oy + s.y * u - lift, 2.0f * u - 1.0f, 2.0f * u - 1.0f};
      const unsigned char shade = static_cast<unsigned char>(std::min(255, 178 + z * 20));
      DrawRectangleRounded({r.x + 1.5f, r.y + 1.5f, r.width, r.height}, 0.3f, 4, Fade({20, 90, 70, 255}, alpha));
      DrawRectangleRounded(r, 0.3f, 4, Fade({shade, shade, static_cast<unsigned char>(shade - 22), 255}, alpha));
    }
  }
}

void drawLock(float cx, float cy) {
  DrawRing({cx, cy - 14.0f}, 13.0f, 19.0f, 180.0f, 360.0f, 24, ui::kText);
  DrawRectangleRounded({cx - 24.0f, cy - 14.0f, 48.0f, 38.0f}, 0.3f, 8, ui::kText);
  DrawCircleV({cx, cy + 4.0f}, 5.0f, {12, 18, 34, 255});
}

// Devuelve true si se ha pulsado una tarjeta desbloqueada.
bool drawCard(const Ctx& ctx, int index, Rectangle card, bool unlocked) {
  const Layout& layout = allLayouts()[index];
  const bool over = unlocked && ui::hovered(ctx, card);
  if (over) card.y -= 6.0f;
  ui::panel(card, over ? 0.86f : 0.66f);
  if (over) DrawRectangleRoundedLinesEx(card, 0.12f, 12, 3.0f, Fade(ui::kGold, 0.9f));

  drawPreview(layout, {card.x + 30.0f, card.y + 16.0f, kCardW - 60.0f, 116.0f}, unlocked ? 1.0f : 0.28f);
  ui::text(ctx.title, TextFormat("%d · %s", index + 1, levelName(index)), card.x + 20.0f,
           card.y + 142.0f, 30.0f, unlocked ? ui::kText : Fade(ui::kText, 0.5f));

  if (!unlocked) {
    drawLock(card.x + kCardW * 0.5f, card.y + 70.0f);
    ui::text(ctx.ui, tr(Str::Locked), card.x + 20.0f, card.y + 192.0f, 15.5f, Fade(ui::kMuted, 0.8f));
    return false;
  }

  LevelRecord rec;
  if (auto it = ctx.save->levels.find(layout.id); it != ctx.save->levels.end()) rec = it->second;
  for (int s = 0; s < 3; ++s)
    ui::star(card.x + 32.0f + s * 28.0f, card.y + 200.0f, 12.0f,
             s < rec.stars ? ui::kGold : Fade(WHITE, 0.18f));
  const char* info = rec.bestSeconds > 0
                         ? TextFormat("%s %02d:%02d", tr(Str::Best), rec.bestSeconds / 60, rec.bestSeconds % 60)
                         : TextFormat("%d %s", static_cast<int>(layout.slots.size()), tr(Str::Tiles));
  ui::text(ctx.ui, info, card.x + kCardW - 18.0f - ui::textWidth(ctx.ui, info, 17.0f),
           card.y + 191.0f, 17.0f, ui::kMuted);
  return over && ctx.click;
}

// Tarjeta del modo aleatorio: ocupa dos columnas. Devuelve true si se pulsa.
bool drawRandomCard(const Ctx& ctx, Rectangle card) {
  const bool over = ui::hovered(ctx, card);
  if (over) card.y -= 6.0f;
  ui::panel(card, over ? 0.86f : 0.66f);
  DrawRectangleRoundedLinesEx(card, 0.12f, 12, 3.0f, Fade(ui::kGold, over ? 0.9f : 0.35f));

  // Tres fichas con interrogante que se balancean.
  for (int i = 0; i < 3; ++i) {
    const float sway = std::sin(static_cast<float>(ctx.time) * 1.4f + i * 1.3f) * 5.0f;
    const Rectangle tile = {card.x + 46.0f + i * 62.0f, card.y + 40.0f + sway, 52.0f, 68.0f};
    DrawRectangleRounded({tile.x + 5.0f, tile.y + 5.0f, tile.width, tile.height}, 0.2f, 6, {30, 110, 86, 255});
    DrawRectangleRounded(tile, 0.2f, 6, {252, 247, 233, 255});
    ui::textCentered(ctx.title, "?", tile.x + tile.width * 0.5f, tile.y + 12.0f, 46.0f,
                     i == 1 ? Color{204, 44, 54, 255} : Color{28, 52, 110, 255});
  }
  ui::text(ctx.title, tr(Str::Random), card.x + 250.0f, card.y + 46.0f, 46.0f, ui::kGold);
  ui::text(ctx.ui, tr(Str::RandomHint), card.x + 252.0f, card.y + 104.0f, 19.0f, ui::kText);
  ui::text(ctx.ui, TextFormat("48 - 144 %s", tr(Str::Tiles)), card.x + 22.0f, card.y + 190.0f, 17.0f, ui::kMuted);
  return over && ctx.click;
}

// Control de volumen: etiqueta, menos, porcentaje y más. Devuelve el nuevo valor.
float drawVolume(const Ctx& ctx, const char* label, float x, float y, float value) {
  ui::text(ctx.ui, label, x, y + 11.0f, 24.0f, ui::kText);
  if (ui::button(ctx, {x + 108.0f, y, 44.0f, 46.0f}, "-")) value -= 0.1f;
  ui::textCentered(ctx.ui, TextFormat("%d", static_cast<int>(std::round(value * 10.0f))), x + 181.0f,
                   y + 11.0f, 24.0f, ui::kGold);
  if (ui::button(ctx, {x + 210.0f, y, 44.0f, 46.0f}, "+")) value += 0.1f;
  return std::clamp(std::round(value * 10.0f) / 10.0f, 0.0f, 1.0f);
}

// Aviso de versión nueva, arriba a la derecha.
void drawUpdate(Ctx& ctx) {
  using State = Updater::State;
  const State state = ctx.updater->state();
  if (state == State::Idle || state == State::Checking || state == State::UpToDate) return;

  const Rectangle box = {1178.0f, 34.0f, 327.0f, 112.0f};
  ui::panel(box, 0.8f);
  const float cx = box.x + box.width * 0.5f;
  const Rectangle action = {box.x + 46.0f, box.y + 54.0f, box.width - 92.0f, 44.0f};
  if (state == State::Available || state == State::Manual) {
    ui::textCentered(ctx.ui, TextFormat(tr(Str::UpdateAvailable), ctx.updater->version().c_str()), cx,
                     box.y + 16.0f, 21.0f, ui::kGold);
    if (state == State::Available) {
      if (ui::button(ctx, action, tr(Str::UpdateNow), true, 21.0f)) ctx.updater->install();
    } else if (ui::button(ctx, action, tr(Str::UpdateOpenPage), true, 21.0f)) {
      OpenURL(ctx.updater->pageUrl().c_str());
    }
  } else if (state == State::Failed) {
    ui::textCentered(ctx.ui, tr(Str::UpdateFailed), cx, box.y + 16.0f, 21.0f, ui::kText);
    if (ui::button(ctx, action, tr(Str::UpdateOpenPage), true, 21.0f)) OpenURL(ctx.updater->pageUrl().c_str());
  } else {
    const bool restarting = state == State::Ready;
    ui::textCentered(ctx.ui, tr(restarting ? Str::UpdateRestarting : Str::UpdateDownloading), cx,
                     box.y + 22.0f, 21.0f, ui::kText);
    // Barra que va y viene mientras se trabaja.
    const float k = 0.5f + 0.5f * std::sin(static_cast<float>(ctx.time) * 3.0f);
    DrawRectangleRounded({box.x + 40.0f, box.y + 72.0f, box.width - 80.0f, 8.0f}, 1.0f, 6, Fade(WHITE, 0.15f));
    DrawRectangleRounded({box.x + 40.0f + k * (box.width - 160.0f), box.y + 72.0f, 80.0f, 8.0f}, 1.0f, 6, ui::kGold);
  }
}

}  // namespace

MenuResult Menu::draw(Ctx& ctx) {
  MenuResult result;
  drawTitle(ctx);
  drawUpdate(ctx);

  const auto& layouts = allLayouts();
  const float x0 = (kViewW - (kColumns * kCardW + (kColumns - 1) * kGap)) * 0.5f;
  bool previousCleared = true;
  for (int i = 0; i < static_cast<int>(layouts.size()) && i < 8; ++i) {
    const Rectangle card = {x0 + (i % kColumns) * (kCardW + kGap), 224.0f + (i / kColumns) * (kCardH + 26.0f), kCardW,
                            kCardH};
    if (drawCard(ctx, i, card, previousCleared)) {
      ctx.audio->play(Sfx::Select);
      result = {MenuResult::Kind::Play, i};
    }
    const auto it = ctx.save->levels.find(layouts[i].id);
    previousCleared = it != ctx.save->levels.end() && it->second.stars > 0;
  }

  // El modo aleatorio ocupa los dos huecos que quedan en la segunda fila.
  if (drawRandomCard(ctx, {x0 + 3.0f * (kCardW + kGap), 224.0f + kCardH + 26.0f, 2.0f * kCardW + kGap, kCardH})) {
    ctx.audio->play(Sfx::Select);
    result = {MenuResult::Kind::Play, -1};
  }

  // Barra de ajustes.
  const float y = 757.0f;
  ui::panel({x0, y - 14.0f, kViewW - 2.0f * x0, 74.0f}, 0.6f);
  SaveData& save = *ctx.save;
  const float music = drawVolume(ctx, tr(Str::Music), x0 + 24.0f, y, save.music);
  const float sfx = drawVolume(ctx, tr(Str::Effects), x0 + 310.0f, y, save.sfx);
  if (music != save.music || sfx != save.sfx) {
    save.music = music;
    save.sfx = sfx;
    ctx.audio->setMusicVolume(music);
    ctx.audio->setSfxVolume(sfx);
    save.store();
  }
  if (ui::button(ctx, {x0 + 600.0f, y, 262.0f, 46.0f},
                 TextFormat("%s: %s", tr(Str::Language), tr(Str::LanguageName)))) {
    setLang(lang() == Lang::Es ? Lang::En : Lang::Es);
    save.lang = static_cast<int>(lang());
    save.store();
    ctx.langChanged = true;
  }
  if (ui::button(ctx, {x0 + 880.0f, y, 290.0f, 46.0f}, tr(Str::Fullscreen))) ToggleBorderlessWindowed();
  if (ui::button(ctx, {x0 + 1188.0f, y, 198.0f, 46.0f}, tr(Str::Quit))) result.kind = MenuResult::Kind::Quit;

  ui::textCentered(ctx.ui, "© 2026 Richardo G. Cibea M.  ·  v" NM_VERSION, kViewW * 0.5f, 852.0f, 18.0f,
                   Fade(ui::kMuted, 0.7f));
  return result;
}

}  // namespace nm
