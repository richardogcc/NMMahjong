// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <map>
#include <string>

namespace nm {

struct LevelRecord {
  int stars = 0;
  int bestSeconds = 0;
};

// Progreso y ajustes del jugador, guardados en la carpeta de datos del usuario.
struct SaveData {
  std::map<std::string, LevelRecord> levels;
  float music = 0.6f;
  float sfx = 0.8f;
  int lang = -1;  // -1: todavía sin elegir, se usa el idioma del sistema
  bool persist = true;  // false: no se escribe nada en disco (modo de capturas)

  void load();
  void store() const;
  // Guarda el resultado solo si mejora el que había.
  void record(const std::string& levelId, int stars, int seconds);
};

}  // namespace nm
