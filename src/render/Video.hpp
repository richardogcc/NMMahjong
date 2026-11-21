// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <string>
#include <vector>

#include "raylib.h"

struct plm_t;

namespace nm {

// Vídeo en bucle (MPEG-1, sin sonido) que se va volcando en una textura.
class Video {
 public:
  Video() = default;
  Video(const Video&) = delete;
  Video& operator=(const Video&) = delete;
  ~Video() { close(); }

  bool open(const std::string& path);
  void close();
  // Avanza la reproducción `dt` segundos y actualiza la textura si toca.
  void update(float dt);

  bool ready() const { return plm_ != nullptr && texture_.id != 0; }
  const Texture2D& texture() const { return texture_; }

 // La función que recibe los fotogramas necesita ver este tipo.
  // Último fotograma decodificado, pendiente de subir a la textura.
  struct Frame {
    std::vector<unsigned char> rgb;
    int width = 0, height = 0;
    bool dirty = false;
  };


 private:
  plm_t* plm_ = nullptr;
  Texture2D texture_ = {};
  Frame frame_;
};

}  // namespace nm
