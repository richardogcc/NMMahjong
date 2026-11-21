// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "game/Save.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace nm {
namespace {

namespace fs = std::filesystem;

fs::path dataDir() {
  fs::path dir;
#if defined(_WIN32)
  if (const char* appData = std::getenv("APPDATA")) dir = fs::path(appData) / "NMMahjong";
#elif defined(__APPLE__)
  if (const char* home = std::getenv("HOME"))
    dir = fs::path(home) / "Library" / "Application Support" / "NMMahjong";
#else
  if (const char* home = std::getenv("HOME")) dir = fs::path(home) / ".local" / "share" / "NMMahjong";
#endif
  if (dir.empty()) dir = fs::current_path();
  std::error_code ec;
  fs::create_directories(dir, ec);
  return dir;
}

fs::path saveFile() { return dataDir() / "save.txt"; }
fs::path gameFile() { return dataDir() / "partida.txt"; }
bool gPersist = true;

std::tm localTime(std::time_t t) {
  std::tm out = {};
#if defined(_WIN32)
  localtime_s(&out, &t);
#else
  localtime_r(&t, &out);
#endif
  return out;
}

}  // namespace

void SaveData::load() {
  std::ifstream in(saveFile());
  std::string line;
  std::map<std::string, int*> ints = {
      {"lang", &lang},           {"musicTrack", &musicTrack},   {"tileSet", &tileSet},
      {"difficulty", &difficulty}, {"playStyle", &playStyle},   {"played", &played},
      {"won", &won},             {"winStreak", &winStreak},     {"bestWinStreak", &bestWinStreak},
      {"secondsWon", &secondsWon}, {"pairs", &pairs},           {"maxCombo", &maxCombo},
      {"randomWon", &randomWon}, {"dailyDate", &dailyDate},     {"dailyStreak", &dailyStreak},
      {"bestDailyStreak", &bestDailyStreak}};
  std::map<std::string, bool*> flags = {
      {"animatedBg", &animatedBg}, {"highlightFree", &highlightFree}, {"tutorialDone", &tutorialDone},
      {"largeText", &largeText},   {"reduceFx", &reduceFx},           {"seasonal", &seasonal}};
  while (std::getline(in, line)) {
    std::istringstream row(line);
    std::string key;
    if (!(row >> key)) continue;
    if (key == "music") {
      row >> music;
    } else if (key == "sfx") {
      row >> sfx;
    } else if (key == "level") {
      std::string id;
      LevelRecord rec;
      if (row >> id >> rec.stars >> rec.bestSeconds) {
        row >> rec.bestScore;  // no está en los guardados antiguos
        levels[id] = rec;
      }
    } else if (key == "achievement") {
      std::string id;
      if (row >> id && id.size() <= 32) achievements.insert(id);
    } else if (auto it = ints.find(key); it != ints.end()) {
      row >> *it->second;
    } else if (auto flag = flags.find(key); flag != flags.end()) {
      int value = 0;
      if (row >> value) *flag->second = value != 0;
    }
  }
  // El archivo lo puede haber tocado cualquiera: se acotan los valores.
  music = std::clamp(music, 0.0f, 1.0f);
  sfx = std::clamp(sfx, 0.0f, 1.0f);
  if (lang < -1 || lang > 5) lang = -1;
  musicTrack = std::clamp(musicTrack, -1, 31);
  tileSet = std::clamp(tileSet, 0, 3);
  difficulty = std::clamp(difficulty, 0, 2);
  playStyle = std::clamp(playStyle, 0, 2);
  for (int* counter : {&played, &won, &winStreak, &bestWinStreak, &secondsWon, &pairs, &maxCombo, &randomWon,
                       &dailyDate, &dailyStreak, &bestDailyStreak})
    *counter = std::max(*counter, 0);
  for (auto& [id, rec] : levels) {
    rec.stars = std::clamp(rec.stars, 0, 3);
    rec.bestSeconds = std::max(rec.bestSeconds, 0);
    rec.bestScore = std::clamp(rec.bestScore, 0, 10000000);
  }
  gPersist = persist;
}

void SaveData::store() const {
  gPersist = persist;
  if (!persist) return;
  std::ofstream out(saveFile(), std::ios::trunc);
  out << "music " << music << "\nsfx " << sfx << "\nlang " << lang << "\n";
  out << "animatedBg " << animatedBg << "\nmusicTrack " << musicTrack << "\nhighlightFree " << highlightFree
      << "\ntileSet " << tileSet << "\ndifficulty " << difficulty << "\nplayStyle " << playStyle
      << "\ntutorialDone " << tutorialDone << "\nlargeText " << largeText << "\nreduceFx " << reduceFx
      << "\nseasonal " << seasonal << "\n";
  out << "played " << played << "\nwon " << won << "\nwinStreak " << winStreak << "\nbestWinStreak "
      << bestWinStreak << "\nsecondsWon " << secondsWon << "\npairs " << pairs << "\nmaxCombo " << maxCombo
      << "\nrandomWon " << randomWon << "\n";
  out << "dailyDate " << dailyDate << "\ndailyStreak " << dailyStreak << "\nbestDailyStreak "
      << bestDailyStreak << "\n";
  for (const auto& [id, rec] : levels)
    out << "level " << id << " " << rec.stars << " " << rec.bestSeconds << " " << rec.bestScore << "\n";
  for (const std::string& id : achievements) out << "achievement " << id << "\n";
}

bool loadSavedGame(SavedGame& out) {
  std::ifstream in(gameFile());
  if (!in) return false;
  SavedGame g;
  std::size_t tiles = 0, moves = 0;
  if (!(in >> g.kind >> g.level >> g.layoutSeed >> g.theme >> g.difficulty >> g.playStyle >> g.score >>
        g.hintsUsed >> g.shufflesUsed >> g.elapsed >> g.timeLeft >> tiles >> moves))
    return false;
  if (tiles == 0 || tiles > 144 || moves > 72 || g.kind < 0 || g.kind > 2) return false;
  g.faces.resize(tiles);
  g.removed.resize(tiles);
  for (std::size_t i = 0; i < tiles; ++i)
    if (!(in >> g.faces[i] >> g.removed[i])) return false;
  g.history.resize(moves);
  for (auto& [a, b] : g.history)
    if (!(in >> a >> b)) return false;
  if (!(in >> g.dealSeed)) g.dealSeed = 0;  // añadido después; puede faltar
  g.level = std::clamp(g.level, 0, 63);
  g.theme = std::clamp(g.theme, 0, 63);
  g.difficulty = std::clamp(g.difficulty, 0, 2);
  g.playStyle = std::clamp(g.playStyle, 0, 2);
  g.score = std::clamp(g.score, 0, 10000000);
  g.elapsed = std::clamp(g.elapsed, 0.0f, 360000.0f);
  g.timeLeft = std::clamp(g.timeLeft, 0.0f, 360000.0f);
  out = std::move(g);
  return true;
}

void storeSavedGame(const SavedGame& g) {
  if (!gPersist) return;
  std::ofstream out(gameFile(), std::ios::trunc);
  out << g.kind << " " << g.level << " " << g.layoutSeed << " " << g.theme << " " << g.difficulty << " "
      << g.playStyle << " " << g.score << " " << g.hintsUsed << " " << g.shufflesUsed << " " << g.elapsed << " "
      << g.timeLeft << " " << g.faces.size() << " " << g.history.size() << "\n";
  for (std::size_t i = 0; i < g.faces.size(); ++i) out << g.faces[i] << " " << g.removed[i] << " ";
  out << "\n";
  for (const auto& [a, b] : g.history) out << a << " " << b << " ";
  out << "\n" << g.dealSeed << "\n";
}

void clearSavedGame() {
  if (!gPersist) return;
  std::error_code ec;
  fs::remove(gameFile(), ec);
}

bool hasSavedGame() {
  std::error_code ec;
  return gPersist && fs::exists(gameFile(), ec);
}

int todayDate() {
  const std::tm now = localTime(std::time(nullptr));
  return (now.tm_year + 1900) * 10000 + (now.tm_mon + 1) * 100 + now.tm_mday;
}

int previousDate(int date) {
  std::tm day = {};
  day.tm_year = date / 10000 - 1900;
  day.tm_mon = date / 100 % 100 - 1;
  day.tm_mday = date % 100 - 1;  // mktime normaliza el día 0 al último del mes anterior
  day.tm_hour = 12;
  const std::time_t t = std::mktime(&day);
  const std::tm prev = localTime(t);
  return (prev.tm_year + 1900) * 10000 + (prev.tm_mon + 1) * 100 + prev.tm_mday;
}

}  // namespace nm
