// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string>

#include "core/Layout.hpp"
#include "game/Ctx.hpp"
#include "game/Game.hpp"
#include "game/Menu.hpp"
#include "game/Strings.hpp"
#include "raylib.h"
#include "rlgl.h"

namespace {

using namespace nm;

enum class Scene { Menu, Play };

std::string assetDir() {
  const std::string base = GetApplicationDirectory();
  for (const std::string& dir : {base + "assets/", base + "../Resources/assets/", std::string("assets/")})
    if (DirectoryExists(dir.c_str())) return dir;
  return "assets/";
}

Font loadFont(const std::string& path, int size) {
  const char* glyphs =
      " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
      "áéíóúüñÁÉÍÓÚÜÑ¿¡·©×";
  int count = 0;
  int* codepoints = LoadCodepoints(glyphs, &count);
  Font font = LoadFontEx(path.c_str(), size, codepoints, count);
  UnloadCodepoints(codepoints);
  GenTextureMipmaps(&font.texture);
  SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
  return font;
}

void saveFrame(const std::string& path) {
  rlDrawRenderBatchActive();
  Image image = LoadImageFromScreen();
  ExportImage(image, path.c_str());
  UnloadImage(image);
}

// Modo de capturas para la documentación: NM_CAPTURE=<carpeta> recorre el menú
// y una partida, guarda las imágenes y cierra. NM_CAPTURE_LEVEL elige el nivel.
struct Capture {
  std::string dir;
  int level = 3;
  int stage = 0;
  float timer = 0.0f;
  int movesDone = 0;
  int frames = 0;      // NM_CAPTURE_FRAMES: fotogramas seguidos para montar un vídeo
  int framesSaved = 0;
  bool active() const { return !dir.empty(); }
};

}  // namespace

int main() {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
  SetTraceLogLevel(LOG_WARNING);
  InitWindow(1280, 720, "NM Mahjong");
  SetWindowMinSize(960, 540);
  SetExitKey(KEY_NULL);
  SetRandomSeed(static_cast<unsigned int>(std::time(nullptr)));

  SaveData save;
  save.load();
  setLang(save.lang < 0 ? systemLang() : static_cast<Lang>(save.lang));

  Capture capture;
  if (const char* dir = std::getenv("NM_CAPTURE")) capture.dir = dir;
  if (capture.active()) {
    save = SaveData{};  // las capturas no leen ni tocan el progreso real
    save.persist = false;
  }
  if (const char* level = std::getenv("NM_CAPTURE_LEVEL")) capture.level = std::atoi(level);
  if (const char* frames = std::getenv("NM_CAPTURE_FRAMES")) capture.frames = std::atoi(frames);
  if (const char* forced = std::getenv("NM_LANG")) setLang(forced[0] == 'e' && forced[1] == 'n' ? Lang::En : Lang::Es);

  const std::string assets = assetDir();
  Audio audio;
  audio.init(assets);
  audio.setMusicVolume(capture.active() ? 0.0f : save.music);
  audio.setSfxVolume(capture.active() ? 0.0f : save.sfx);

  Background background;
  background.init(assets);
  TileAtlas atlas;

  Ctx ctx;
  ctx.title = loadFont(assets + "fonts/LilitaOne-Regular.ttf", 112);
  ctx.ui = loadFont(assets + "fonts/Poppins-SemiBold.ttf", 64);
  ctx.audio = &audio;
  ctx.save = &save;
  ctx.atlas = &atlas;
  Updater updater;
  ctx.updater = &updater;
  if (!capture.active()) updater.check();
  atlas.build(ctx.title, tr(Str::West));

  Menu menu;
  Game game;
  Scene scene = Scene::Menu;
  Scene nextScene = Scene::Menu;
  int nextLevel = 0;
  bool switching = false, quit = false;
  float fade = 1.0f;  // 1 = negro, 0 = visible
  audio.setMood(menuTheme().mood);

  while (!WindowShouldClose() && !quit) {
    ctx.dt = std::min(GetFrameTime(), 0.05f);
    ctx.time = GetTime();

    const float sw = static_cast<float>(GetScreenWidth()), sh = static_cast<float>(GetScreenHeight());
    Camera2D camera = {};
    camera.zoom = std::min(sw / kViewW, sh / kViewH);
    camera.offset = {(sw - kViewW * camera.zoom) * 0.5f, (sh - kViewH * camera.zoom) * 0.5f};
    ctx.mouse = GetScreenToWorld2D(GetMousePosition(), camera);
    ctx.click = !switching && fade < 0.5f && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

    // Fundido entre pantallas.
    if (switching) {
      fade = std::min(1.0f, fade + ctx.dt * 4.0f);
      if (fade >= 1.0f) {
        scene = nextScene;
        if (scene == Scene::Play) game.start(nextLevel, ctx);
        else audio.setMood(menuTheme().mood);
        switching = false;
      }
    } else {
      fade = std::max(0.0f, fade - ctx.dt * 3.0f);
    }
    auto go = [&](Scene target, int level) {
      nextScene = target;
      nextLevel = level;
      switching = true;
    };

    audio.update(ctx.dt);
    if (scene == Scene::Play) game.update(ctx);

    BeginDrawing();
    ClearBackground(BLACK);
    background.draw(scene == Scene::Menu ? menuTheme() : levelTheme(game.level()), ctx.time, ctx.dt);
    BeginMode2D(camera);
    if (scene == Scene::Menu) {
      const MenuResult result = menu.draw(ctx);
      if (result.kind == MenuResult::Kind::Play) go(Scene::Play, result.level);
      if (result.kind == MenuResult::Kind::Quit) quit = true;
    } else {
      const GameAction action = game.draw(ctx);
      if (action == GameAction::Menu) go(Scene::Menu, 0);
      if (action == GameAction::Restart) go(Scene::Play, game.level());
      if (action == GameAction::Next) go(Scene::Play, game.level() + 1);
      if (IsKeyPressed(KEY_ESCAPE)) go(Scene::Menu, 0);
    }
    EndMode2D();
    if (fade > 0.0f) DrawRectangle(0, 0, static_cast<int>(sw) + 1, static_cast<int>(sh) + 1, Fade(BLACK, fade));

    if (capture.active()) {
      capture.timer += ctx.dt;
      if (capture.stage == 0 && capture.timer > 1.5f) {
        saveFrame(capture.dir + "/menu.png");
        go(Scene::Play, capture.level);
        capture.stage = 1;
        capture.timer = 0.0f;
      } else if (capture.stage == 1 && capture.timer > 4.0f) {
        saveFrame(capture.dir + "/game.png");
        capture.stage = capture.frames > 0 ? 10 : 2;
        capture.timer = 0.0f;
      } else if (capture.stage == 10 && capture.timer > 0.1f) {
        capture.timer = 0.0f;
        saveFrame(capture.dir + TextFormat("/f%03d.png", capture.framesSaved));
        if (++capture.framesSaved >= capture.frames) quit = true;
      } else if (capture.stage == 2 && capture.timer > 0.34f) {
        capture.timer = 0.0f;
        if (capture.movesDone < 6) {
          game.autoMove(ctx);
          ++capture.movesDone;
        } else {
          saveFrame(capture.dir + "/effects.png");
          capture.stage = 3;
        }
      } else if (capture.stage == 3 && capture.timer > 0.03f) {
        capture.timer = 0.0f;
        if (!game.autoMove(ctx)) capture.stage = 4;  // victoria o sin movimientos
      } else if (capture.stage == 4 && capture.timer > 2.8f) {
        saveFrame(capture.dir + "/end.png");
        quit = true;
      }
    }
    EndDrawing();

#ifndef NDEBUG
    // Ensayo de la actualización sin ratón (solo en compilaciones de depuración).
    if (std::getenv("NM_UPDATE_AUTO") && updater.state() == Updater::State::Available) updater.install();
#endif
    // La versión nueva ya está preparada: se cede el paso al relevo y se cierra.
    if (updater.state() == Updater::State::Ready && updater.restartIntoUpdate()) quit = true;

    if (ctx.langChanged) {
      atlas.build(ctx.title, tr(Str::West));
      ctx.langChanged = false;
    }
  }

  atlas.unload();
  UnloadFont(ctx.title);
  UnloadFont(ctx.ui);
  background.shutdown();
  audio.shutdown();
  CloseWindow();
  return 0;
}
