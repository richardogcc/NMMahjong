// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include "raylib.h"

namespace nm {

enum class Sfx { Click, Select, Match, Error, Win, Shuffle, Hint, Count };

// Todo el sonido del juego se sintetiza al arrancar: no hay archivos de audio.
class Audio {
 public:
  void init();
  void shutdown();

  void play(Sfx sfx, float pitch = 1.0f);
  void setMusicVolume(float volume);
  void setSfxVolume(float volume);
  // Cambia la escala, el tono y el tempo de la música ambiental.
  void setMood(int mood);

 private:
  Sound sounds_[static_cast<int>(Sfx::Count)] = {};
  AudioStream stream_ = {};
  float sfxVolume_ = 0.8f;
  bool ready_ = false;
};

namespace music {
// Rellena `frames` muestras estéreo en coma flotante. Se ejecuta en el hilo de audio.
void fill(void* buffer, unsigned int frames);
void setVolume(float volume);
void setMood(int mood);
}  // namespace music

}  // namespace nm
