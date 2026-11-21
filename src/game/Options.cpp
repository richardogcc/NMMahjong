// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Panel de opciones, compartido por el menú y la pausa.
#include "game/Options.hpp"

#include <algorithm>
#include <cmath>

#include "game/Strings.hpp"
#include "ui/Ui.hpp"

namespace nm {
namespace {

int tab = 0;  // 0 imagen y sonido, 1 juego, 2 accesibilidad

}  // namespace

void setOptionsTab(int which) { tab = std::clamp(which, 0, 2); }

void drawOptionsPanel(Ctx& ctx, Rectangle box, bool inGame) {
  SaveData& save = *ctx.save;
  const SaveData before = save;
  const float labelX = box.x + 110.0f, controlW = 400.0f, controlX = box.x + box.width - 110.0f - controlW;

  // Pestañas.
  const Str tabs[3] = {Str::TabDisplay, Str::TabGame, Str::TabAccess};
  const float tabW = 290.0f, tabGap = 16.0f, tabX = box.x + (box.width - 3.0f * tabW - 2.0f * tabGap) * 0.5f;
  for (int i = 0; i < 3; ++i) {
    const Rectangle r = {tabX + i * (tabW + tabGap), box.y + 92.0f, tabW, 46.0f};
    if (ui::button(ctx, r, tr(tabs[i]), true, 22.0f)) tab = i;
    if (i == tab) DrawRectangleRounded({r.x + 24.0f, r.y + r.height + 5.0f, r.width - 48.0f, 4.0f}, 1.0f, 4, ui::kGold);
  }

  float y = box.y + 176.0f;
  auto row = [&](const char* label) {
    ui::text(ctx.ui, label, labelX, y + 10.0f, 23.0f, ui::kText);
    const Rectangle r = {controlX, y, controlW, 44.0f};
    y += 62.0f;
    return r;
  };
  auto volume = [&](const char* label, float& value) {
    const Rectangle r = row(label);
    if (ui::button(ctx, {r.x, r.y, 60.0f, r.height}, "-")) value -= 0.1f;
    if (ui::button(ctx, {r.x + r.width - 60.0f, r.y, 60.0f, r.height}, "+")) value += 0.1f;
    value = std::clamp(std::round(value * 10.0f) / 10.0f, 0.0f, 1.0f);
    ui::textCentered(ctx.ui, TextFormat("%d", static_cast<int>(std::round(value * 10.0f))), r.x + r.width * 0.5f,
                     r.y + 10.0f, 24.0f, ui::kGold);
  };
  // Botón que muestra el valor actual y pasa al siguiente al pulsarlo.
  auto cycle = [&](const char* label, const char* value) { return ui::button(ctx, row(label), value, true, 21.0f); };
  auto toggle = [&](Str label, bool& value) {
    if (cycle(tr(label), tr(value ? Str::Yes : Str::No))) value = !value;
  };

  if (tab == 0) {
    volume(tr(Str::Music), save.music);
    volume(tr(Str::Effects), save.sfx);
    const int tracks = ctx.audio->trackCount();
    const char* trackLabel = save.musicTrack < 0 || save.musicTrack >= tracks
                                 ? tr(Str::ByLevel)
                                 : TextFormat("%d · %s", save.musicTrack + 1, ctx.audio->trackName(save.musicTrack));
    if (cycle(tr(Str::OptMusicTrack), trackLabel))
      save.musicTrack = save.musicTrack + 1 >= tracks ? -1 : save.musicTrack + 1;
    if (cycle(tr(Str::Fullscreen), tr(IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE) ? Str::Yes : Str::No)))
      ToggleBorderlessWindowed();
    if (cycle(tr(Str::OptBackground), tr(save.animatedBg ? Str::Animated : Str::Static)))
      save.animatedBg = !save.animatedBg;
    toggle(Str::OptSeasonal, save.seasonal);
  } else if (tab == 1) {
    if (cycle(tr(Str::Language), tr(Str::LanguageName))) {
      setLang(static_cast<Lang>((static_cast<int>(lang()) + 1) % kLangCount));
      save.lang = static_cast<int>(lang());
    }
    const Str levels[3] = {Str::DiffEasy, Str::DiffNormal, Str::DiffHard};
    if (cycle(tr(Str::OptDifficulty), tr(levels[save.difficulty]))) save.difficulty = (save.difficulty + 1) % 3;
    const Str styles[3] = {Str::StyleNormal, Str::StyleTimed, Str::StyleZen};
    if (cycle(tr(Str::OptStyle), tr(styles[save.playStyle]))) save.playStyle = (save.playStyle + 1) % 3;
    if (inGame)
      ui::textWrapped(ctx.ui, tr(Str::OptNextGame), box.x + 150.0f, y + 12.0f, box.width - 300.0f, 19.0f, ui::kMuted,
                      true);
  } else {
    const Str sets[4] = {Str::TilesModern, Str::TilesClassic, Str::TilesContrast, Str::TilesColorBlind};
    if (cycle(tr(Str::OptTiles), tr(sets[save.tileSet]))) save.tileSet = (save.tileSet + 1) % 4;
    toggle(Str::OptHighlight, save.highlightFree);
    toggle(Str::OptLargeText, save.largeText);
    toggle(Str::OptReduceFx, save.reduceFx);
  }

  ctx.audio->setMusicVolume(save.music);
  ctx.audio->setSfxVolume(save.sfx);
  ctx.audio->setFixedTrack(save.musicTrack);
  ui::setLargeText(save.largeText);
  // main recarga fuentes y fichas cuando cambia algo que les afecta.
  if (before.lang != save.lang) ctx.langChanged = true;
  if (before.tileSet != save.tileSet || before.seasonal != save.seasonal) ctx.tilesChanged = true;
  const bool changed = before.music != save.music || before.sfx != save.sfx || before.lang != save.lang ||
                       before.animatedBg != save.animatedBg || before.musicTrack != save.musicTrack ||
                       before.highlightFree != save.highlightFree || before.tileSet != save.tileSet ||
                       before.difficulty != save.difficulty || before.playStyle != save.playStyle ||
                       before.largeText != save.largeText || before.reduceFx != save.reduceFx ||
                       before.seasonal != save.seasonal;
  if (changed) save.store();
}

}  // namespace nm
