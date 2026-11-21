// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "game/Save.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace nm {
namespace {

std::filesystem::path saveFile() {
  namespace fs = std::filesystem;
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
  return dir / "save.txt";
}

}  // namespace

void SaveData::load() {
  std::ifstream in(saveFile());
  std::string line;
  while (std::getline(in, line)) {
    std::istringstream row(line);
    std::string key;
    if (!(row >> key)) continue;
    if (key == "music") {
      row >> music;
    } else if (key == "sfx") {
      row >> sfx;
    } else if (key == "lang") {
      row >> lang;
    } else if (key == "level") {
      std::string id;
      LevelRecord rec;
      if (row >> id >> rec.stars >> rec.bestSeconds) levels[id] = rec;
    }
  }
  // El archivo lo puede haber tocado cualquiera: se acotan los valores.
  music = std::clamp(music, 0.0f, 1.0f);
  sfx = std::clamp(sfx, 0.0f, 1.0f);
  if (lang < -1 || lang > 1) lang = -1;
  for (auto& [id, rec] : levels) {
    rec.stars = std::clamp(rec.stars, 0, 3);
    rec.bestSeconds = std::max(rec.bestSeconds, 0);
  }
}

void SaveData::store() const {
  if (!persist) return;
  std::ofstream out(saveFile(), std::ios::trunc);
  out << "music " << music << "\n";
  out << "sfx " << sfx << "\n";
  out << "lang " << lang << "\n";
  for (const auto& [id, rec] : levels)
    out << "level " << id << " " << rec.stars << " " << rec.bestSeconds << "\n";
}

void SaveData::record(const std::string& levelId, int stars, int seconds) {
  LevelRecord& rec = levels[levelId];
  rec.stars = std::max(rec.stars, stars);
  if (rec.bestSeconds == 0 || seconds < rec.bestSeconds) rec.bestSeconds = seconds;
  store();
}

}  // namespace nm
