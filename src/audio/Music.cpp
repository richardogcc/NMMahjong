// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Música ambiental generativa: un colchón de notas largas, un bajo pausado y
// una melodía que pasea por una escala pentatónica, con eco estéreo.
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>

#include "audio/Audio.hpp"

namespace nm::music {
namespace {

constexpr float kRate = 44100.0f;
constexpr float kTau = 6.28318530718f;
constexpr int kVoices = 20;
constexpr int kDelay = 18522;  // 0,42 s
constexpr int kMoods = 4;

constexpr int kScales[kMoods][5] = {
    {0, 2, 4, 7, 9},   // pentatónica mayor
    {0, 2, 3, 7, 8},   // hirajoshi
    {0, 3, 5, 7, 10},  // pentatónica menor
    {0, 2, 5, 7, 9},   // yo
};
constexpr float kRoots[kMoods] = {261.63f, 220.0f, 293.66f, 196.0f};
constexpr float kStepSeconds[kMoods] = {0.34f, 0.44f, 0.38f, 0.36f};
constexpr int kDrone[4] = {0, 3, 1, 4};

struct Voice {
  float phase = 0.0f, inc = 0.0f, amp = 0.0f, decay = 0.0f, pan = 0.5f;
  int age = 0;
};

std::atomic<float> gVolume{0.6f};
std::atomic<int> gMood{0};

// Estado que solo toca el hilo de audio.
Voice voices[kVoices];
float delayL[kDelay], delayR[kDelay];
int delayPos = 0;
float padPhase[3], padFreq[3] = {130.0f, 196.0f, 261.0f}, padTarget[3] = {130.0f, 196.0f, 261.0f};
float lfo = 0.0f, volume = 0.0f, lowL = 0.0f, lowR = 0.0f;
int samplesToStep = 0, step = 0, melody = 4;
std::uint32_t rng = 0x9E3779B9u;

float rnd() {
  rng ^= rng << 13;
  rng ^= rng >> 17;
  rng ^= rng << 5;
  return static_cast<float>(rng & 0xFFFFFF) / 16777216.0f;
}

float semitones(float base, int n) { return base * std::pow(2.0f, n / 12.0f); }

void trigger(float freq, float amp, float seconds, float pan) {
  Voice* quietest = &voices[0];
  for (Voice& v : voices)
    if (v.amp < quietest->amp) quietest = &v;
  *quietest = {0.0f, kTau * freq / kRate, amp, std::exp(-1.0f / (seconds * kRate)), pan, 0};
}

void advanceStep() {
  const int mood = std::clamp(gMood.load(), 0, kMoods - 1);
  const int* scale = kScales[mood];
  const float root = kRoots[mood];
  samplesToStep = static_cast<int>(kStepSeconds[mood] * kRate);

  const int degree = kDrone[(step / 16) % 4];
  if (step % 16 == 0) {
    const float base = semitones(root, scale[degree]) * 0.5f;
    padTarget[0] = base;
    padTarget[1] = base * 1.5f;
    padTarget[2] = base * 2.0f;
  }
  if (step % 8 == 0) trigger(semitones(root, scale[degree]) * 0.25f, 0.2f, 2.4f, 0.5f);

  if (rnd() < 0.56f) {
    melody = std::clamp(melody + static_cast<int>(rnd() * 5.0f) - 2, 0, 9);
    const float freq = semitones(root, scale[melody % 5] + 12 * (melody / 5));
    trigger(freq, 0.1f + rnd() * 0.07f, 1.1f + rnd() * 0.8f, 0.2f + rnd() * 0.6f);
    if (rnd() < 0.18f) {
      const int high = std::min(melody + 2, 11);
      trigger(semitones(root, scale[high % 5] + 12 * (high / 5)), 0.06f, 1.6f, rnd());
    }
  }
  ++step;
}

}  // namespace

void setVolume(float value) { gVolume.store(value); }
void setMood(int mood) { gMood.store(mood); }

void fill(void* buffer, unsigned int frames) {
  auto* out = static_cast<float*>(buffer);
  const float target = gVolume.load();
  for (unsigned int i = 0; i < frames; ++i) {
    if (--samplesToStep <= 0) advanceStep();
    volume += (target - volume) * 0.0005f;

    float left = 0.0f, right = 0.0f;
    for (Voice& v : voices) {
      if (v.amp < 0.0004f) continue;
      const float attack = std::min(1.0f, v.age / 220.0f);
      const float s = (std::sin(v.phase) + 0.32f * std::sin(2.0f * v.phase) +
                       0.1f * std::sin(3.0f * v.phase)) * v.amp * attack;
      left += s * (1.0f - v.pan);
      right += s * v.pan;
      v.phase += v.inc;
      if (v.phase > kTau) v.phase -= kTau;
      v.amp *= v.decay;
      ++v.age;
    }

    lfo += kTau * 0.07f / kRate;
    if (lfo > kTau) lfo -= kTau;
    for (int p = 0; p < 3; ++p) {
      padFreq[p] += (padTarget[p] - padFreq[p]) * 0.00004f;
      padPhase[p] += kTau * padFreq[p] / kRate;
      if (padPhase[p] > kTau) padPhase[p] -= kTau;
      const float s = std::sin(padPhase[p]) * 0.034f * (0.7f + 0.3f * std::sin(lfo + p * 2.1f));
      left += s * (p == 1 ? 0.4f : 1.0f);
      right += s * (p == 2 ? 0.4f : 1.0f);
    }

    // Eco cruzado: lo que sale por un lado vuelve por el otro.
    const float echoL = delayL[delayPos], echoR = delayR[delayPos];
    delayL[delayPos] = left + echoR * 0.42f;
    delayR[delayPos] = right + echoL * 0.42f;
    delayPos = (delayPos + 1) % kDelay;
    left += echoL * 0.38f;
    right += echoR * 0.38f;

    lowL += (left - lowL) * 0.35f;
    lowR += (right - lowR) * 0.35f;
    out[2 * i] = std::tanh(lowL * 1.4f) * volume;
    out[2 * i + 1] = std::tanh(lowR * 1.4f) * volume;
  }
}

}  // namespace nm::music
