// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "render/Background.hpp"

#include <cmath>

namespace nm {
namespace {

const char* kFragment = R"GLSL(
#version 330
out vec4 finalColor;
uniform float uTime;
uniform vec2 uRes;
uniform vec3 uA;
uniform vec3 uB;
uniform vec3 uC;
uniform int uMode;

float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

float noise(vec2 p) {
  vec2 i = floor(p), f = fract(p);
  f = f * f * (3.0 - 2.0 * f);
  return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), f.x),
             mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), f.x), f.y);
}

float fbm(vec2 p) {
  float v = 0.0, a = 0.5;
  for (int i = 0; i < 5; i++) {
    v += a * noise(p);
    p = p * 2.03 + vec2(17.0, 9.0);
    a *= 0.5;
  }
  return v;
}

void main() {
  vec2 uv = gl_FragCoord.xy / uRes;
  vec2 p = (gl_FragCoord.xy - 0.5 * uRes) / uRes.y;
  float t = uTime;
  vec3 col;

  if (uMode == 0) {
    // Aurora: velos de color que se retuercen despacio.
    vec2 q = vec2(fbm(p * 1.4 + vec2(0.0, t * 0.05)), fbm(p * 1.4 + vec2(5.2, -t * 0.04)));
    float f = fbm(p * 1.8 + q * 1.6 + vec2(t * 0.03, 0.0));
    col = mix(uA, uB, smoothstep(0.2, 0.8, f));
    col = mix(col, uC, smoothstep(0.35, 0.9, q.x * f * 2.2));
    float band = sin((p.y + f * 0.6) * 9.0 + t * 0.4) * 0.5 + 0.5;
    col += uC * pow(band, 6.0) * 0.10;
  } else if (uMode == 1) {
    // Agua: reflejos de luz en el fondo de un estanque.
    vec2 w = p * 3.2;
    float c = 0.0;
    for (int i = 0; i < 4; i++) {
      float fi = float(i);
      w += vec2(sin(w.y * 1.3 + t * 0.35 + fi), cos(w.x * 1.1 - t * 0.30 + fi * 2.0)) * 0.42;
      c += pow(0.5 + 0.5 * sin(w.x + w.y), 5.0);
    }
    col = mix(uA, uB, uv.y + 0.25 * fbm(p * 2.0 + t * 0.03));
    col += uC * c * 0.16;
  } else if (uMode == 2) {
    // Noche: nebulosa lenta y estrellas que titilan.
    float n = fbm(p * 1.6 + vec2(t * 0.012, -t * 0.008));
    col = mix(uA, uB, smoothstep(0.25, 0.85, n + uv.y * 0.25));
    col += uC * pow(fbm(p * 3.0 - vec2(t * 0.02, 0.0)), 3.0) * 0.9;
    vec2 g = gl_FragCoord.xy / (uRes.y / 42.0);
    vec2 cell = floor(g);
    float seed = hash(cell);
    if (seed > 0.86) {
      vec2 off = vec2(hash(cell + 3.1), hash(cell + 7.7)) * 0.7 + 0.15;
      float d = length(fract(g) - off);
      float twinkle = 0.55 + 0.45 * sin(t * (1.0 + seed * 3.0) + seed * 40.0);
      col += vec3(1.0, 0.96, 0.88) * smoothstep(0.09, 0.0, d) * twinkle;
    }
  } else {
    // Atardecer: sol bajo y bancos de niebla que cruzan.
    col = mix(uA, uB, smoothstep(0.0, 1.0, uv.y));
    float sun = length(p - vec2(0.0, -0.08));
    col += uC * (smoothstep(0.34, 0.0, sun) * 0.55 + smoothstep(0.9, 0.0, sun) * 0.22);
    for (int i = 0; i < 3; i++) {
      float fi = float(i);
      float mist = fbm(vec2(p.x * (1.2 + fi * 0.5) + t * (0.02 + fi * 0.012), p.y * 4.0 + fi * 9.0));
      float mask = smoothstep(0.42, -0.3, p.y + fi * 0.16);
      col = mix(col, mix(uB, vec3(1.0), 0.35), smoothstep(0.5, 0.85, mist) * mask * 0.35);
    }
  }

  float vig = smoothstep(1.25, 0.25, length(p * vec2(0.85, 1.1)));
  col *= mix(0.5, 1.0, vig);
  col += (hash(gl_FragCoord.xy + t) - 0.5) / 255.0;  // evita el bandeado
  finalColor = vec4(col, 1.0);
}
)GLSL";

float frand() { return static_cast<float>(GetRandomValue(0, 10000)) / 10000.0f; }

}  // namespace

void Background::init() {
  shader_ = LoadShaderFromMemory(nullptr, kFragment);
  locTime_ = GetShaderLocation(shader_, "uTime");
  locRes_ = GetShaderLocation(shader_, "uRes");
  locA_ = GetShaderLocation(shader_, "uA");
  locB_ = GetShaderLocation(shader_, "uB");
  locC_ = GetShaderLocation(shader_, "uC");
  locMode_ = GetShaderLocation(shader_, "uMode");
  for (int i = 0; i < 46; ++i)
    motes_.push_back({{frand(), frand()},
                      {0.012f + frand() * 0.03f, 0.01f + frand() * 0.028f},
                      4.0f + frand() * 7.0f,
                      frand() * 6.28f,
                      20.0f + frand() * 70.0f});
}

void Background::shutdown() { UnloadShader(shader_); }

void Background::draw(const Theme& theme, double time, float dt) {
  const float w = static_cast<float>(GetScreenWidth());
  const float h = static_cast<float>(GetScreenHeight());
  const float t = static_cast<float>(time);
  const float res[2] = {static_cast<float>(GetRenderWidth()), static_cast<float>(GetRenderHeight())};

  SetShaderValue(shader_, locTime_, &t, SHADER_UNIFORM_FLOAT);
  SetShaderValue(shader_, locRes_, res, SHADER_UNIFORM_VEC2);
  SetShaderValue(shader_, locA_, &theme.a, SHADER_UNIFORM_VEC3);
  SetShaderValue(shader_, locB_, &theme.b, SHADER_UNIFORM_VEC3);
  SetShaderValue(shader_, locC_, &theme.c, SHADER_UNIFORM_VEC3);
  SetShaderValue(shader_, locMode_, &theme.mode, SHADER_UNIFORM_INT);
  BeginShaderMode(shader_);
  DrawRectangle(0, 0, static_cast<int>(w) + 1, static_cast<int>(h) + 1, WHITE);
  EndShaderMode();

  // Motas: pétalos, luciérnagas o copos, según el color del tema.
  const float unit = h / 900.0f;
  for (Mote& m : motes_) {
    m.pos.x += (m.vel.x + 0.012f * std::sin(t * 0.6f + m.phase)) * dt;
    m.pos.y += m.vel.y * dt;
    if (m.pos.x > 1.05f) m.pos.x = -0.05f;
    if (m.pos.y > 1.05f) m.pos.y = -0.05f;
    const float flicker = 0.45f + 0.35f * std::sin(t * 1.3f + m.phase * 3.0f);
    const Vector2 at = {m.pos.x * w, m.pos.y * h};
    DrawPoly(at, 5, m.size * unit, t * m.spin + m.phase * 57.0f, Fade(theme.mote, flicker * 0.6f));
    DrawCircleV(at, m.size * unit * 0.45f, Fade(WHITE, flicker * 0.25f));
  }
}

}  // namespace nm
