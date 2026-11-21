// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "render/Video.hpp"

#include <algorithm>

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

namespace nm {
bool Video::open(const std::string& path) {
  close();
  plm_ = plm_create_with_filename(path.c_str());
  if (!plm_) return false;
  plm_set_audio_enabled(plm_, 0);
  plm_set_loop(plm_, 1);
  frame_.width = plm_get_width(plm_);
  frame_.height = plm_get_height(plm_);
  // Se acotan las medidas: el archivo podría estar dañado o no ser un vídeo.
  if (frame_.width < 16 || frame_.height < 16 || frame_.width > 4096 || frame_.height > 4096) {
    close();
    return false;
  }
  frame_.rgb.assign(static_cast<std::size_t>(frame_.width) * frame_.height * 3, 0);
  plm_set_video_decode_callback(
      plm_,
      [](plm_t*, plm_frame_t* decoded, void* user) {
        auto* frame = static_cast<Frame*>(user);
        if (static_cast<int>(decoded->width) != frame->width || static_cast<int>(decoded->height) != frame->height)
          return;
        plm_frame_to_rgb(decoded, frame->rgb.data(), frame->width * 3);
        frame->dirty = true;
      },
      &frame_);

  Image blank = {frame_.rgb.data(), frame_.width, frame_.height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8};
  texture_ = LoadTextureFromImage(blank);
  if (texture_.id == 0) {
    close();
    return false;
  }
  SetTextureFilter(texture_, TEXTURE_FILTER_BILINEAR);
  SetTextureWrap(texture_, TEXTURE_WRAP_CLAMP);
  update(0.05f);  // primer fotograma, para no enseñar un cuadro negro
  return true;
}

void Video::close() {
  if (plm_) plm_destroy(plm_);
  plm_ = nullptr;
  if (texture_.id != 0) UnloadTexture(texture_);
  texture_ = {};
  frame_ = {};
}

void Video::update(float dt) {
  if (!plm_) return;
  plm_decode(plm_, std::clamp(dt, 0.0f, 0.1f));
  if (frame_.dirty && texture_.id != 0) {
    UpdateTexture(texture_, frame_.rgb.data());
    frame_.dirty = false;
  }
}

}  // namespace nm
