// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "audio/Audio.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace nm {
namespace {

constexpr int kRate = 44100;
constexpr float kTau = 6.28318530718f;

float bell(float t, float freq, float decay) {
  if (t < 0.0f) return 0.0f;
  return std::sin(kTau * freq * t) * std::exp(-t / decay);
}

template <typename Fn>
Sound synth(float seconds, Fn sample) {
  const int count = static_cast<int>(seconds * kRate);
  std::vector<float> data(count);
  for (int i = 0; i < count; ++i) {
    const float t = static_cast<float>(i) / kRate;
    const float fadeIn = std::min(1.0f, i / 64.0f);
    const float fadeOut = std::min(1.0f, (count - i) / 400.0f);
    data[i] = std::clamp(sample(t), -1.0f, 1.0f) * fadeIn * fadeOut;
  }
  Wave wave = {};
  wave.frameCount = static_cast<unsigned int>(count);
  wave.sampleRate = kRate;
  wave.sampleSize = 32;
  wave.channels = 1;
  wave.data = data.data();
  return LoadSoundFromWave(wave);  // copia las muestras
}

float noise(std::uint32_t& state) {
  state = state * 1664525u + 1013904223u;
  return static_cast<float>(state >> 8) / 8388608.0f - 1.0f;
}

}  // namespace

void Audio::init() {
  InitAudioDevice();
  if (!IsAudioDeviceReady()) return;

  auto set = [this](Sfx id, Sound sound) { sounds_[static_cast<int>(id)] = sound; };
  set(Sfx::Click, synth(0.08f, [](float t) { return bell(t, 1150.0f, 0.014f) * 0.5f; }));
  set(Sfx::Select, synth(0.25f, [](float t) {
        return bell(t, 740.0f, 0.06f) * 0.45f + bell(t, 1480.0f, 0.035f) * 0.2f;
      }));
  set(Sfx::Match, synth(0.9f, [](float t) {
        return bell(t, 1318.5f, 0.16f) * 0.34f + bell(t - 0.07f, 1975.5f, 0.2f) * 0.3f +
               bell(t - 0.14f, 2637.0f, 0.24f) * 0.22f;
      }));
  set(Sfx::Error, synth(0.22f, [](float t) {
        return std::sin(kTau * (165.0f - 180.0f * t) * t) * std::exp(-t / 0.07f) * 0.6f;
      }));
  set(Sfx::Win, synth(2.2f, [](float t) {
        const float notes[] = {523.25f, 659.25f, 783.99f, 1046.5f, 1318.5f, 1568.0f};
        float out = 0.0f;
        for (int i = 0; i < 6; ++i) out += bell(t - 0.12f * i, notes[i], 0.55f) * 0.22f;
        return out;
      }));
  set(Sfx::Shuffle, synth(0.55f, [state = std::uint32_t{7}](float t) mutable {
        const float env = std::exp(-t / 0.2f) * (0.6f + 0.4f * std::sin(t * 70.0f));
        return noise(state) * env * 0.28f + bell(t, 320.0f + 900.0f * t, 0.2f) * 0.15f;
      }));
  set(Sfx::Hint, synth(0.7f, [](float t) {
        return bell(t, 880.0f, 0.2f) * 0.3f + bell(t - 0.09f, 1320.0f, 0.26f) * 0.24f;
      }));

  SetAudioStreamBufferSizeDefault(2048);
  stream_ = LoadAudioStream(kRate, 32, 2);
  SetAudioStreamCallback(stream_, music::fill);
  PlayAudioStream(stream_);
  ready_ = true;
}

void Audio::shutdown() {
  if (ready_) {
    StopAudioStream(stream_);
    UnloadAudioStream(stream_);
    for (Sound& sound : sounds_) UnloadSound(sound);
  }
  CloseAudioDevice();
  ready_ = false;
}

void Audio::play(Sfx sfx, float pitch) {
  if (!ready_ || sfxVolume_ <= 0.0f) return;
  Sound& sound = sounds_[static_cast<int>(sfx)];
  SetSoundPitch(sound, pitch);
  SetSoundVolume(sound, sfxVolume_);
  PlaySound(sound);
}

void Audio::setMusicVolume(float volume) { music::setVolume(std::clamp(volume, 0.0f, 1.0f)); }
void Audio::setSfxVolume(float volume) { sfxVolume_ = std::clamp(volume, 0.0f, 1.0f); }
void Audio::setMood(int mood) { music::setMood(mood); }

}  // namespace nm
