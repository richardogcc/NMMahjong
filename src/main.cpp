// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <initializer_list>
#include <string>

#include "core/Layout.hpp"
#include "game/Ctx.hpp"
#include "game/Game.hpp"
#include "game/Menu.hpp"
#include "game/Options.hpp"
#include "game/Strings.hpp"
#include "raylib.h"
#include "rlgl.h"
#include "ui/Ui.hpp"

namespace {

using namespace nm;

enum class Scene { Menu, Play };

std::string assetDir() {
  const std::string base = GetApplicationDirectory();
  for (const std::string& dir : {base + "assets/", base + "../Resources/assets/", std::string("assets/")})
    if (DirectoryExists(dir.c_str())) return dir;
  return "assets/";
}

Font loadFont(const std::string& path, int size, const char* glyphs) {
  int count = 0;
  int* codepoints = LoadCodepoints(glyphs, &count);
  Font font = LoadFontEx(path.c_str(), size, codepoints, count);
  UnloadCodepoints(codepoints);
  if (font.texture.id != 0) {
    GenTextureMipmaps(&font.texture);
    SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
  }
  return font;
}

const char* const kLatin =
    " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
    "áéíóúüñÁÉÍÓÚÜÑ¿¡·©×àâäãçèêëîïôöõùûÀÂÄÃÇÈÊËÎÏÔÖÕÙÛß«»’";
const char* const kChinese = "一二三四五六七八九萬東南西北中發梅蘭菊竹春夏秋冬";

void saveFrame(const std::string& path) {
  rlDrawRenderBatchActive();
  Image image = LoadImageFromScreen();
  ExportImage(image, path.c_str());
  UnloadImage(image);
}

// Modo de capturas para la documentación y las pruebas: NM_CAPTURE=<carpeta>
// recorre el menú y una partida, guarda las imágenes y cierra.
//   NM_CAPTURE_LEVEL   nivel (0-11), -1 aleatorio, -2 reto diario, -3 continuar la guardada
//   NM_CAPTURE_PERSIST usa y escribe el guardado (para probar con HOME apuntando a una carpeta de prueba)
//   NM_CAPTURE_SUSPEND tras unas jugadas guarda la partida y cierra
//   NM_CAPTURE_PANEL   1 opciones, 2 estadísticas, 3 logros, 4 tutorial (NM_CAPTURE_TAB: pestaña de opciones)
//   NM_CAPTURE_SOLUTION  a media partida pide ver la solución y la captura
//   NM_SEASON          fuerza la temporada (0 ninguna, 1-4 estaciones, 5 Año Nuevo chino)
//   NM_LARGE_TEXT, NM_REDUCE_FX  activan esas opciones
//   NM_CAPTURE_PAUSE   captura también la pantalla de pausa (2: con las opciones abiertas)
//   NM_CAPTURE_FRAMES  fotogramas seguidos para montar un vídeo
//   NM_TILES, NM_STYLE, NM_DIFFICULTY  fuerzan esos ajustes
struct Capture {
  std::string dir;
  int level = 3;
  int stage = 0;
  float timer = 0.0f;
  int movesDone = 0;
  int frames = 0;
  int framesSaved = 0;
  bool active() const { return !dir.empty(); }
  GameRequest request() const {
    if (level == -1) return {BoardKind::Random, 0, false};
    if (level == -2) return {BoardKind::Daily, 0, false};
    if (level == -3) return {BoardKind::Level, 0, true};
    return {BoardKind::Level, level, false};
  }
};

int envInt(const char* name, int fallback) {
  const char* value = std::getenv(name);
  return value ? std::atoi(value) : fallback;
}

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

  Capture capture;
  if (const char* dir = std::getenv("NM_CAPTURE")) capture.dir = dir;
  capture.level = envInt("NM_CAPTURE_LEVEL", capture.level);
  capture.frames = envInt("NM_CAPTURE_FRAMES", 0);
  if (capture.active() && !std::getenv("NM_CAPTURE_PERSIST")) {
    save = SaveData{};  // las capturas no leen ni tocan el progreso real
    save.persist = false;
    save.tutorialDone = true;
    save.tileSet = std::clamp(envInt("NM_TILES", 0), 0, 3);
    save.playStyle = std::clamp(envInt("NM_STYLE", 0), 0, 2);
    save.difficulty = std::clamp(envInt("NM_DIFFICULTY", 0), 0, 2);
    save.largeText = envInt("NM_LARGE_TEXT", 0) != 0;
    save.reduceFx = envInt("NM_REDUCE_FX", 0) != 0;
    save.store();  // deja anotado que no se escribe en disco
  }
  setLang(save.lang < 0 ? systemLang() : static_cast<Lang>(save.lang));
  if (const char* forced = std::getenv("NM_LANG")) setLang(langFromCode(forced));
  ui::setLargeText(save.largeText);
  // La temporada sale de la fecha, salvo que se desactive en las opciones.
  auto applySeason = [&]() {
    Season now = save.seasonal ? seasonOf(todayDate()) : Season::None;
    if (capture.active()) now = static_cast<Season>(std::clamp(envInt("NM_SEASON", 0), 0, 5));
    setSeason(now);
  };
  applySeason();
  setOptionsTab(envInt("NM_CAPTURE_TAB", 0));

  const std::string assets = assetDir();
  Audio audio;
  audio.init(assets);
  audio.setMusicVolume(capture.active() ? 0.0f : save.music);
  audio.setSfxVolume(capture.active() ? 0.0f : save.sfx);
  audio.setFixedTrack(save.musicTrack);

  Background background;
  background.init(assets);
  TileAtlas atlas;

  Ctx ctx;
  ctx.logo = loadFont(assets + "fonts/LilitaOne-Regular.ttf", 112, kLatin);
  const Font latinUi = loadFont(assets + "fonts/Poppins-SemiBold.ttf", 64, kLatin);
  Font cjk = loadFont(assets + "fonts/LXGWWenKaiTC-subset.ttf", 128, kChinese);
  Font cjkUi = {};  // toda la interfaz en chino; se carga solo si hace falta
  // En chino la interfaz usa la fuente china; en los demás idiomas, las latinas.
  auto pickFonts = [&]() {
    ctx.title = ctx.logo;
    ctx.ui = latinUi;
    if (lang() != Lang::Zh) return;
    if (cjkUi.texture.id == 0)
      cjkUi = loadFont(assets + "fonts/LXGWWenKaiTC-subset.ttf", 72, (std::string(kLatin) + allText(Lang::Zh)).c_str());
    if (cjkUi.texture.id != 0) ctx.title = ctx.ui = cjkUi;
  };
  pickFonts();
  ctx.audio = &audio;
  ctx.save = &save;
  ctx.atlas = &atlas;
  Updater updater;
  ctx.updater = &updater;
  // En las capturas no se consulta la red, salvo en el ensayo con NM_UPDATE_FAKE_DIR.
  if (!capture.active() || std::getenv("NM_UPDATE_FAKE_DIR")) updater.check();

  auto buildTiles = [&]() {
    TileStyle style;
    style.set = static_cast<TileSet>(save.tileSet);
    style.font = &ctx.logo;
    style.cjk = &cjk;
    style.edge = tileEdge();
    style.westLetter = tr(Str::West);
    atlas.build(style);
  };
  buildTiles();

  Menu menu;
  Game game;
  Scene scene = Scene::Menu;
  Scene nextScene = Scene::Menu;
  GameRequest nextRequest;
  bool switching = false, quit = false;
  float fade = 1.0f;  // 1 = negro, 0 = visible
  Vector2 pointer = {kViewW * 0.5f, kViewH * 0.5f};  // puntero que mueve la palanca del mando
  bool pointerActive = false;
  audio.setMood(menuTheme().mood);

  while (!WindowShouldClose() && !quit) {
    ctx.dt = std::min(GetFrameTime(), 0.05f);
    ctx.time = GetTime();
    ctx.nav = readNav();

    const float sw = static_cast<float>(GetScreenWidth()), sh = static_cast<float>(GetScreenHeight());
    Camera2D camera = {};
    camera.zoom = std::min(sw / kViewW, sh / kViewH);
    camera.offset = {(sw - kViewW * camera.zoom) * 0.5f, (sh - kViewH * camera.zoom) * 0.5f};

    // Con el mando, la palanca mueve un puntero y el botón A hace clic.
    const Vector2 mouseDelta = GetMouseDelta();
    if (std::fabs(mouseDelta.x) + std::fabs(mouseDelta.y) > 0.5f) pointerActive = false;
    if (ctx.nav.stick.x != 0.0f || ctx.nav.stick.y != 0.0f) {
      if (!pointerActive) pointer = GetScreenToWorld2D(GetMousePosition(), camera);
      pointerActive = true;
      pointer.x = std::clamp(pointer.x + ctx.nav.stick.x * 900.0f * ctx.dt, 0.0f, kViewW);
      pointer.y = std::clamp(pointer.y + ctx.nav.stick.y * 900.0f * ctx.dt, 0.0f, kViewH);
    }
    if (ctx.nav.up || ctx.nav.down || ctx.nav.left || ctx.nav.right) pointerActive = false;
    ctx.mouse = pointerActive ? pointer : GetScreenToWorld2D(GetMousePosition(), camera);
    const bool ready = !switching && fade < 0.5f;
    ctx.click = ready && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || (pointerActive && ctx.nav.accept));
    if (pointerActive) ctx.nav.accept = false;  // ese A ya ha servido de clic
    if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

    // Fundido entre pantallas.
    if (switching) {
      fade = std::min(1.0f, fade + ctx.dt * 4.0f);
      if (fade >= 1.0f) {
        scene = nextScene;
        if (scene == Scene::Play) game.start(nextRequest, ctx);
        else audio.setMood(menuTheme().mood);
        switching = false;
      }
    } else {
      fade = std::max(0.0f, fade - ctx.dt * 3.0f);
    }
    auto go = [&](Scene target, const GameRequest& request) {
      nextScene = target;
      nextRequest = request;
      switching = true;
    };

    audio.update(ctx.dt);
    if (scene == Scene::Play) {
      if (ready && ctx.nav.pause) game.togglePause();
      game.update(ctx);
    }

    BeginDrawing();
    ClearBackground(BLACK);
    background.draw(scene == Scene::Menu ? menuTheme() : levelTheme(game.themeIndex()), ctx.time, ctx.dt,
                    save.animatedBg && !save.reduceFx);
    BeginMode2D(camera);
    if (scene == Scene::Menu) {
      const MenuResult result = menu.draw(ctx);
      if (result.kind == MenuResult::Kind::Play) go(Scene::Play, result.request);
      if (result.kind == MenuResult::Kind::Quit) quit = true;
    } else {
      const GameAction action = game.draw(ctx);
      if (action == GameAction::Menu) {
        game.suspend();  // se podrá continuar desde el menú
        save.store();
        go(Scene::Menu, {});
      }
      if (action == GameAction::Restart) go(Scene::Play, game.sameRequest());
      if (action == GameAction::Next) go(Scene::Play, game.nextRequest());
    }
    if (pointerActive) {
      DrawCircleV(pointer, 11.0f, Fade(BLACK, 0.5f));
      DrawCircleV(pointer, 8.0f, {255, 205, 86, 255});
    }
    EndMode2D();
    if (fade > 0.0f) DrawRectangle(0, 0, static_cast<int>(sw) + 1, static_cast<int>(sh) + 1, Fade(BLACK, fade));

    if (capture.active()) {
      capture.timer += ctx.dt;
      if (capture.stage == 0 && capture.timer > 1.5f) {
        saveFrame(capture.dir + "/menu.png");
        if (const int panel = envInt("NM_CAPTURE_PANEL", 0); panel > 0) {
          menu.openPanel(panel);
          capture.stage = 30;
        } else {
          go(Scene::Play, capture.request());
          capture.stage = 1;
        }
        capture.timer = 0.0f;
      } else if (capture.stage == 30 && capture.timer > 0.5f) {
        saveFrame(capture.dir + "/panel.png");
        quit = true;
      } else if (capture.stage == 1 && capture.timer > 4.0f) {
        saveFrame(capture.dir + "/game.png");
        capture.stage = capture.frames > 0 ? 10 : 2;
        capture.timer = 0.0f;
        if (std::getenv("NM_CAPTURE_PAUSE")) {
          if (std::atoi(std::getenv("NM_CAPTURE_PAUSE")) == 2) game.openOptions();
          else game.togglePause();
          capture.stage = 20;
        }
      } else if (capture.stage == 20 && capture.timer > 0.4f) {
        saveFrame(capture.dir + "/pause.png");
        quit = true;
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
          if (std::getenv("NM_CAPTURE_SOLUTION")) {
            game.showSolution(ctx);
            capture.stage = 40;
          }
          if (std::getenv("NM_CAPTURE_SUSPEND")) {
            game.suspend();
            quit = true;
          }
        }
      } else if (capture.stage == 3 && capture.timer > 0.03f) {
        capture.timer = 0.0f;
        if (!game.autoMove(ctx)) capture.stage = 4;  // victoria o sin movimientos
      } else if (capture.stage == 40 && capture.timer > 5.0f) {
        saveFrame(capture.dir + "/solution.png");
        quit = true;
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
    if (updater.state() == Updater::State::Ready) {
      if (scene == Scene::Play) game.suspend();
      if (updater.restartIntoUpdate()) quit = true;
    }

    if (ctx.langChanged || ctx.tilesChanged) {
      pickFonts();
      applySeason();
      buildTiles();
      ctx.langChanged = ctx.tilesChanged = false;
    }
  }

  // Al cerrar a mitad de partida, se guarda para continuarla.
  if (scene == Scene::Play && !capture.active()) game.suspend();
  save.store();

  atlas.unload();
  UnloadFont(ctx.logo);
  UnloadFont(latinUi);
  if (cjk.texture.id != 0) UnloadFont(cjk);
  if (cjkUi.texture.id != 0) UnloadFont(cjkUi);
  background.shutdown();
  audio.shutdown();
  CloseWindow();
  return 0;
}
