// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <string>
#include <vector>

#include "raylib.h"

namespace nm {

enum class Sfx { Click, Select, Match, Error, Win, Shuffle, Hint, Count };

// Los efectos se sintetizan al arrancar. La música son piezas grabadas que se
// leen de assets/music; si no hay ninguna, suena la música generada.
class Audio {
 public:
  void init(const std::string& assetDir);
  void shutdown();
  // Hay que llamarlo en cada fotograma: alimenta la música y hace los fundidos.
  void update(float dt);

  void play(Sfx sfx, float pitch = 1.0f);
  void setMusicVolume(float volume);
  void setSfxVolume(float volume);
  // Elige la pieza que suena (o el carácter de la música generada).
  void setMood(int mood);

 private:
  Sound sounds_[static_cast<int>(Sfx::Count)] = {};
  AudioStream stream_ = {};
  float sfxVolume_ = 0.8f;
  float musicVolume_ = 0.6f;
  std::vector<Music> tracks_;
  int current_ = -1, wanted_ = 0;
  float fade_ = 0.0f;  // 0 en silencio, 1 a su volumen
  bool ready_ = false;
};

namespace music {
// Rellena `frames` muestras estéreo en coma flotante. Se ejecuta en el hilo de audio.
void fill(void* buffer, unsigned int frames);
void setVolume(float volume);
void setMood(int mood);
}  // namespace music

}  // namespace nm
