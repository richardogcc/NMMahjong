// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Panel de opciones, compartido por el menú y la pausa.
#include "game/Options.hpp"

#include <algorithm>
#include <cmath>

#include "game/Strings.hpp"
#include "ui/Ui.hpp"

namespace nm {

void drawOptionsPanel(Ctx& ctx, Rectangle box, bool inGame) {
  SaveData& save = *ctx.save;
  const SaveData before = save;
  float y = box.y + 92.0f;
  const float labelX = box.x + 90.0f, controlW = 400.0f, controlX = box.x + box.width - 90.0f - controlW;

  auto row = [&](const char* label) {
    ui::text(ctx.ui, label, labelX, y + 9.0f, 23.0f, ui::kText);
    const Rectangle r = {controlX, y, controlW, 42.0f};
    y += 53.0f;
    return r;
  };
  auto volume = [&](const char* label, float& value) {
    const Rectangle r = row(label);
    if (ui::button(ctx, {r.x, r.y, 60.0f, r.height}, "-")) value -= 0.1f;
    if (ui::button(ctx, {r.x + r.width - 60.0f, r.y, 60.0f, r.height}, "+")) value += 0.1f;
    value = std::clamp(std::round(value * 10.0f) / 10.0f, 0.0f, 1.0f);
    ui::textCentered(ctx.ui, TextFormat("%d", static_cast<int>(std::round(value * 10.0f))), r.x + r.width * 0.5f,
                     r.y + 9.0f, 24.0f, ui::kGold);
  };
  // Botón que muestra el valor actual y pasa al siguiente al pulsarlo.
  auto cycle = [&](const char* label, const char* value) { return ui::button(ctx, row(label), value, true, 21.0f); };

  volume(tr(Str::Music), save.music);
  volume(tr(Str::Effects), save.sfx);
  if (cycle(tr(Str::Language), tr(Str::LanguageName))) {
    setLang(lang() == Lang::Es ? Lang::En : Lang::Es);
    save.lang = static_cast<int>(lang());
    ctx.langChanged = true;
  }
  if (cycle(tr(Str::Fullscreen), tr(IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE) ? Str::Yes : Str::No)))
    ToggleBorderlessWindowed();
  if (cycle(tr(Str::OptBackground), tr(save.animatedBg ? Str::Animated : Str::Static)))
    save.animatedBg = !save.animatedBg;

  const int tracks = ctx.audio->trackCount();
  const char* trackLabel = save.musicTrack < 0 || save.musicTrack >= tracks
                               ? tr(Str::ByLevel)
                               : TextFormat("%d · %s", save.musicTrack + 1, ctx.audio->trackName(save.musicTrack));
  if (cycle(tr(Str::OptMusicTrack), trackLabel)) save.musicTrack = save.musicTrack + 1 >= tracks ? -1 : save.musicTrack + 1;

  if (cycle(tr(Str::OptHighlight), tr(save.highlightFree ? Str::Yes : Str::No))) save.highlightFree = !save.highlightFree;
  const Str sets[3] = {Str::TilesModern, Str::TilesClassic, Str::TilesContrast};
  if (cycle(tr(Str::OptTiles), tr(sets[save.tileSet]))) {
    save.tileSet = (save.tileSet + 1) % 3;
    ctx.tilesChanged = true;
  }
  const Str levels[3] = {Str::DiffEasy, Str::DiffNormal, Str::DiffHard};
  if (cycle(tr(Str::OptDifficulty), tr(levels[save.difficulty]))) save.difficulty = (save.difficulty + 1) % 3;
  const Str styles[3] = {Str::StyleNormal, Str::StyleTimed, Str::StyleZen};
  if (cycle(tr(Str::OptStyle), tr(styles[save.playStyle]))) save.playStyle = (save.playStyle + 1) % 3;

  ctx.audio->setMusicVolume(save.music);
  ctx.audio->setSfxVolume(save.sfx);
  ctx.audio->setFixedTrack(save.musicTrack);
  const bool changed = before.music != save.music || before.sfx != save.sfx || before.lang != save.lang ||
                       before.animatedBg != save.animatedBg || before.musicTrack != save.musicTrack ||
                       before.highlightFree != save.highlightFree || before.tileSet != save.tileSet ||
                       before.difficulty != save.difficulty || before.playStyle != save.playStyle;
  if (changed) save.store();
  if (inGame)
    ui::textCentered(ctx.ui, tr(Str::OptNextGame), kViewW * 0.5f, y + 2.0f, 18.0f, ui::kMuted);
}

}  // namespace nm
