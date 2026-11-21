// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Música ambiental china generada en tiempo real. Los instrumentos se
// sintetizan, no son grabaciones:
//   · guzheng (cítara): cuerda pulsada por modelado físico (Karplus-Strong),
//     con la nota «doblada» hacia arriba, vibrato y glissandos de escala;
//   · dizi (flauta de bambú): frases largas con vibrato, soplo y adornos;
//   · gong grave y bordón suave que sostienen el ambiente.
// Las melodías usan los modos pentatónicos chinos (gong, yu, shang y zhi).
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>

#include "audio/Audio.hpp"

namespace nm::music {
namespace {

constexpr float kRate = 44100.0f;
constexpr float kTau = 6.28318530718f;
constexpr int kMoods = 4;
constexpr int kStrings = 12;
constexpr int kStringBuffer = 1024;
constexpr int kTones = 8;
constexpr int kQueue = 40;
constexpr int kDelay = 19845;  // 0,45 s

constexpr int kScales[kMoods][5] = {
    {0, 2, 4, 7, 9},   // gong (宫)
    {0, 3, 5, 7, 10},  // yu (羽)
    {0, 2, 5, 7, 10},  // shang (商)
    {0, 2, 5, 7, 9},   // zhi (徵)
};
constexpr float kRoots[kMoods] = {146.83f, 110.0f, 130.81f, 98.0f};  // re, la, do, sol
constexpr float kStepSeconds[kMoods] = {0.40f, 0.50f, 0.44f, 0.46f};

struct String {  // cuerda de guzheng
  float buf[kStringBuffer] = {};
  int write = 0, age = 0;
  float delay = 100.0f, target = 100.0f, damp = 0.0f, last = 0.0f, pan = 0.5f, vibrato = 0.0f;
  bool active = false;
};
struct Tone {  // parcial de gong o bordón
  float phase = 0.0f, inc = 0.0f, amp = 0.0f, decay = 0.0f;
};
struct Pending {  // nota de guzheng programada (glissandos, octavas)
  int wait = 0;
  float freq = 0.0f, amp = 0.0f, pan = 0.5f, bend = 0.0f;
  bool used = false;
};

std::atomic<float> gVolume{0.6f};
std::atomic<int> gMood{0};

// Estado que solo toca el hilo de audio.
String strings[kStrings];
Tone tones[kTones];
Pending queue[kQueue];
float delayL[kDelay], delayR[kDelay];
int delayPos = 0;
float volume = 0.0f, lowL = 0.0f, lowR = 0.0f;
int samplesToStep = 0, step = 0, degree = 5, restSteps = 0;
std::uint32_t rng = 0x9E3779B9u;

struct Flute {
  float phase = 0.0f, freq = 440.0f, target = 440.0f, env = 0.0f, vibPhase = 0.0f, vibDepth = 0.0f;
  float breath = 0.0f;
  bool gate = false;
  int notesLeft = 0, stepsLeft = 0, degree = 7;
} flute;
float dronePhase[2], droneFreq = 73.0f, droneTarget = 73.0f, droneLfo = 0.0f;

float rnd() {
  rng ^= rng << 13;
  rng ^= rng >> 17;
  rng ^= rng << 5;
  return static_cast<float>(rng & 0xFFFFFF) / 16777216.0f;
}

// Frecuencia del grado `index` de la escala, contando octavas hacia arriba.
float note(int mood, int index) {
  const int octave = index >= 0 ? index / 5 : -((4 - index) / 5);
  const int within = index - octave * 5;
  return kRoots[mood] * std::pow(2.0f, (kScales[mood][within] + 12 * octave) / 12.0f);
}

void pluck(float freq, float amp, float pan, float bendSemitones) {
  String* s = &strings[0];
  for (String& candidate : strings) {
    if (!candidate.active) {
      s = &candidate;
      break;
    }
    if (candidate.age > s->age) s = &candidate;
  }
  const float length = std::clamp(kRate / freq, 8.0f, kStringBuffer - 4.0f);
  // Excitación: ruido suavizado; más brillante cuanto más fuerte se pulsa.
  float smooth = 0.0f;
  const float bright = 0.35f + 0.5f * amp * 4.0f;
  const int count = static_cast<int>(length);
  for (int i = 0; i < kStringBuffer; ++i) s->buf[i] = 0.0f;
  for (int i = 0; i < count; ++i) {
    smooth += ((rnd() * 2.0f - 1.0f) - smooth) * std::min(bright, 0.95f);
    s->buf[(kStringBuffer - count + i) % kStringBuffer] = smooth * amp;
  }
  s->write = 0;
  s->delay = length;
  s->target = length / std::pow(2.0f, bendSemitones / 12.0f);
  s->damp = freq < 200.0f ? 0.9992f : (freq < 500.0f ? 0.9985f : 0.9972f);
  s->last = 0.0f;
  s->pan = pan;
  s->vibrato = rnd() * kTau;
  s->age = 0;
  s->active = true;
}

void schedule(int wait, float freq, float amp, float pan, float bend = 0.0f) {
  for (Pending& p : queue) {
    if (p.used) continue;
    p = {wait, freq, amp, pan, bend, true};
    return;
  }
}

void ring(float freq, float amp, float seconds) {
  Tone* t = &tones[0];
  for (Tone& candidate : tones)
    if (candidate.amp < t->amp) t = &candidate;
  *t = {0.0f, kTau * freq / kRate, amp, std::exp(-1.0f / (seconds * kRate))};
}

void gong(float base) {
  const float ratios[5] = {1.0f, 1.48f, 2.02f, 2.67f, 3.41f};
  for (int i = 0; i < 5; ++i) ring(base * ratios[i], 0.05f / (1.0f + i * 0.7f), 5.0f - i * 0.7f);
}

void advanceStep() {
  const int mood = std::clamp(gMood.load(), 0, kMoods - 1);
  const int stepSamples = static_cast<int>(kStepSeconds[mood] * kRate);
  samplesToStep = stepSamples + static_cast<int>((rnd() - 0.5f) * 0.06f * stepSamples);  // rubato

  // Cada 16 pasos: nota grave del guzheng con su octava; el bordón la sigue.
  if (step % 16 == 0) {
    const int rootDegree = (step / 16) % 4 == 2 ? 3 : ((step / 16) % 4 == 3 ? 2 : 0);
    const float low = note(mood, rootDegree - 5);
    pluck(low, 0.2f, 0.45f, 0.0f);
    schedule(static_cast<int>(0.09f * kRate), low * 2.0f, 0.13f, 0.55f);
    droneTarget = low * 0.5f;
    if (step % 64 == 0) gong(low * 0.5f);
  }

  // Glissando: una cascada de notas de la escala, hacia arriba o hacia abajo.
  if (step % 32 == 24 && rnd() < 0.8f) {
    const bool up = rnd() < 0.6f;
    const int count = 9 + static_cast<int>(rnd() * 5.0f);
    const int start = up ? 2 : 2 + count;
    for (int i = 0; i < count; ++i)
      schedule(static_cast<int>(i * 0.055f * kRate), note(mood, start + (up ? i : -i)),
               0.07f + 0.004f * i, 0.2f + 0.6f * i / count);
    restSteps = 3;
  }

  // Melodía del guzheng: pasos pequeños por la escala, con silencios.
  if (restSteps > 0) {
    --restSteps;
  } else if (rnd() < 0.5f) {
    degree = std::clamp(degree + static_cast<int>(rnd() * 5.0f) - 2, 2, 12);
    const bool longNote = rnd() < 0.3f;
    // La nota larga se «dobla»: se pulsa un grado por debajo y sube a su sitio.
    const float bend = longNote ? 12.0f * std::log2(note(mood, degree) / note(mood, degree - 1)) : 0.0f;
    pluck(note(mood, longNote ? degree - 1 : degree), 0.15f + rnd() * 0.07f, 0.25f + rnd() * 0.5f, bend);
    if (rnd() < 0.22f) schedule(static_cast<int>(0.05f * kRate), note(mood, degree + 5), 0.08f, 0.7f);
    if (longNote) restSteps = 1 + static_cast<int>(rnd() * 2.0f);
  }

  // Frases de dizi: unas pocas notas largas y luego un buen silencio.
  if (flute.notesLeft == 0 && step % 8 == 4 && rnd() < 0.3f) {
    flute.notesLeft = 3 + static_cast<int>(rnd() * 4.0f);
    flute.stepsLeft = 0;
    flute.degree = 7 + static_cast<int>(rnd() * 4.0f);
  }
  if (flute.notesLeft > 0 && --flute.stepsLeft <= 0) {
    if (--flute.notesLeft == 0) {
      flute.gate = false;
    } else {
      flute.degree = std::clamp(flute.degree + static_cast<int>(rnd() * 5.0f) - 2, 5, 14);
      flute.target = note(mood, flute.degree);
      if (!flute.gate) flute.freq = flute.target * 0.94f;  // entra desde un poco más abajo
      flute.gate = true;
      flute.vibDepth = 0.0f;
      flute.stepsLeft = 2 + static_cast<int>(rnd() * 4.0f);
    }
  }
  ++step;
}

float fluteSample() {
  flute.env += ((flute.gate ? 1.0f : 0.0f) - flute.env) * (flute.gate ? 0.0006f : 0.00025f);
  if (flute.env < 0.0005f) return 0.0f;
  flute.freq += (flute.target - flute.freq) * 0.0009f;  // ligado entre notas
  flute.vibDepth += (0.0065f - flute.vibDepth) * 0.00004f;  // el vibrato llega tarde
  flute.vibPhase += kTau * 5.2f / kRate;
  if (flute.vibPhase > kTau) flute.vibPhase -= kTau;
  flute.phase += kTau * flute.freq * (1.0f + std::sin(flute.vibPhase) * flute.vibDepth) / kRate;
  if (flute.phase > kTau) flute.phase -= kTau;
  const float p = flute.phase;
  // La membrana del dizi añade un zumbido: armónicos altos y algo de saturación.
  float tone = std::sin(p) + 0.5f * std::sin(2.0f * p) + 0.3f * std::sin(3.0f * p) +
               0.16f * std::sin(4.0f * p) + 0.1f * std::sin(5.0f * p) + 0.05f * std::sin(7.0f * p);
  tone = std::tanh(tone * 1.3f);
  flute.breath += ((rnd() * 2.0f - 1.0f) - flute.breath) * 0.22f;
  return (tone * 0.5f + flute.breath * 0.16f) * flute.env * 0.06f;
}

}  // namespace

void setVolume(float value) { gVolume.store(value); }
void setMood(int mood) { gMood.store(mood); }

void fill(void* buffer, unsigned int frames) {
  auto* out = static_cast<float*>(buffer);
  const float target = gVolume.load();
  for (unsigned int i = 0; i < frames; ++i) {
    if (--samplesToStep <= 0) advanceStep();
    for (Pending& p : queue)
      if (p.used && --p.wait <= 0) {
        pluck(p.freq, p.amp, p.pan, p.bend);
        p.used = false;
      }
    volume += (target - volume) * 0.0005f;

    float left = 0.0f, right = 0.0f;
    for (String& s : strings) {
      if (!s.active) continue;
      // Tras el ataque la cuerda sube al tono doblado y vibra ligeramente.
      if (s.age > 2600) s.delay += (s.target - s.delay) * 0.00035f;
      s.vibrato += kTau * 5.5f / kRate;
      const float wobble = s.age > 9000 ? std::sin(s.vibrato) * 0.0022f * s.delay : 0.0f;
      float read = static_cast<float>(s.write) - (s.delay + wobble);
      while (read < 0.0f) read += kStringBuffer;
      const int i0 = static_cast<int>(read) % kStringBuffer;
      const float frac = read - std::floor(read);
      const float y = s.buf[i0] + (s.buf[(i0 + 1) % kStringBuffer] - s.buf[i0]) * frac;
      const float filtered = 0.5f * (y + s.last);
      s.last = y;
      s.buf[s.write] = filtered * s.damp;
      s.write = (s.write + 1) % kStringBuffer;
      left += y * (1.0f - s.pan) * 1.9f;
      right += y * s.pan * 1.9f;
      if (++s.age > static_cast<int>(kRate * 7.0f)) s.active = false;
    }

    float center = fluteSample();
    for (Tone& t : tones) {
      if (t.amp < 0.0003f) continue;
      center += std::sin(t.phase) * t.amp;
      t.phase += t.inc;
      if (t.phase > kTau) t.phase -= kTau;
      t.amp *= t.decay;
    }
    droneFreq += (droneTarget - droneFreq) * 0.00003f;
    droneLfo += kTau * 0.06f / kRate;
    if (droneLfo > kTau) droneLfo -= kTau;
    for (int d = 0; d < 2; ++d) {
      dronePhase[d] += kTau * droneFreq * (d == 0 ? 1.0f : 1.5f) / kRate;
      if (dronePhase[d] > kTau) dronePhase[d] -= kTau;
      center += std::sin(dronePhase[d]) * 0.016f * (0.7f + 0.3f * std::sin(droneLfo + d * 2.0f));
    }
    left += center;
    right += center;

    // Eco cruzado que da sensación de sala amplia.
    const float echoL = delayL[delayPos], echoR = delayR[delayPos];
    delayL[delayPos] = left + echoR * 0.36f;
    delayR[delayPos] = right + echoL * 0.36f;
    delayPos = (delayPos + 1) % kDelay;
    left += echoL * 0.32f;
    right += echoR * 0.32f;

    lowL += (left - lowL) * 0.55f;
    lowR += (right - lowR) * 0.55f;
    out[2 * i] = std::tanh(lowL * 2.4f) * volume;
    out[2 * i + 1] = std::tanh(lowR * 2.4f) * volume;
  }
}

}  // namespace nm::music
