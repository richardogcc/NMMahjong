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
uniform int uPhoto;
uniform vec2 uTexSize;
uniform sampler2D texture0;

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

vec3 generated(vec2 uv, vec2 p, float t) {
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
  return col;
}

// Fotografía con movimiento lento de cámara y un efecto vivo según el tema.
vec3 photograph(vec2 uv, vec2 p, float t) {
  vec2 top = vec2(uv.x, 1.0 - uv.y);  // origen arriba a la izquierda, como la imagen
  float screen = uRes.x / uRes.y, image = uTexSize.x / uTexSize.y;
  vec2 fit = screen > image ? vec2(1.0, image / screen) : vec2(screen / image, 1.0);
  float zoom = 1.09 + 0.05 * sin(t * 0.045);
  vec2 pan = vec2(sin(t * 0.031), cos(t * 0.023)) * 0.016;
  vec2 st = (top - 0.5) * fit / zoom + 0.5 + pan;

  float low = smoothstep(0.5, 1.0, top.y);  // mitad inferior: agua, arena, hierba
  if (uMode == 1) {
    float wob = noise(vec2(top.x * 6.0, top.y * 30.0 - t * 0.5));
    st += low * vec2(sin(top.y * 110.0 + t * 1.5 + wob * 6.0) * 0.0022, cos(top.x * 46.0 + t * 1.1) * 0.0012);
  } else if (uMode == 3) {
    st.x += (noise(vec2(top.y * 9.0, t * 0.35)) - 0.5) * 0.0045 * low;  // el viento mece el suelo
  }
  vec3 col = texture(texture0, st).rgb;

  if (uMode == 0) {
    // Aurora: el cielo respira con cortinas de luz que se desplazan.
    float sky = smoothstep(0.8, 0.15, top.y);
    float veil = fbm(vec2(top.x * 3.0 + t * 0.05, top.y * 1.4 - t * 0.07));
    float lum = dot(col, vec3(0.3, 0.59, 0.11));
    col *= 1.0 + sky * (veil - 0.45) * 1.1 * smoothstep(0.08, 0.5, lum);
  } else if (uMode == 1) {
    // Destellos sueltos de sol sobre el agua.
    float grain = noise(vec2(top.x * 150.0 + t * 0.35, top.y * 330.0 - t * 0.9));
    float glint = pow(grain, 10.0) * (0.5 + 0.5 * sin(t * 2.2 + top.x * 37.0 + top.y * 91.0));
    col += uC * glint * low * 0.9 * (0.3 + dot(col, vec3(0.33)));
  } else {
    // Bruma o nubes bajas que cruzan despacio.
    float mist = fbm(vec2(p.x * 1.4 + t * 0.028, p.y * 3.2 - t * 0.006));
    col = mix(col, uC, smoothstep(0.48, 0.9, mist) * 0.24);
  }
  if (uMode == 0 || uMode == 2) {
    vec2 g = gl_FragCoord.xy / (uRes.y / 46.0);
    vec2 cell = floor(g);
    float seed = hash(cell);
    if (seed > 0.9 && top.y < 0.55) {
      float d = length(fract(g) - (vec2(hash(cell + 3.1), hash(cell + 7.7)) * 0.7 + 0.15));
      col += vec3(1.0, 0.97, 0.9) * smoothstep(0.07, 0.0, d) * (0.5 + 0.5 * sin(t * (1.0 + seed * 3.0) + seed * 40.0)) * 0.7;
    }
  }
  // Las fichas tienen que leerse bien: algo menos de color y de luz.
  col = mix(col, vec3(dot(col, vec3(0.3, 0.59, 0.11))), 0.14) * 0.74;
  return col;
}

void main() {
  vec2 uv = gl_FragCoord.xy / uRes;
  vec2 p = (gl_FragCoord.xy - 0.5 * uRes) / uRes.y;
  float t = uTime;
  vec3 col = uPhoto == 1 ? photograph(uv, p, t) : generated(uv, p, t);

  float vig = smoothstep(1.25, 0.25, length(p * vec2(0.85, 1.1)));
  col *= mix(0.5, 1.0, vig);
  col += (hash(gl_FragCoord.xy + t) - 0.5) / 255.0;  // evita el bandeado
  finalColor = vec4(col, 1.0);
}
)GLSL";

float frand() { return static_cast<float>(GetRandomValue(0, 10000)) / 10000.0f; }

}  // namespace

void Background::init(const std::string& assetDir) {
  dir_ = assetDir + "backgrounds/";
  Image white = GenImageColor(4, 4, WHITE);
  blank_ = LoadTextureFromImage(white);
  UnloadImage(white);
  shader_ = LoadShaderFromMemory(nullptr, kFragment);
  locTime_ = GetShaderLocation(shader_, "uTime");
  locRes_ = GetShaderLocation(shader_, "uRes");
  locA_ = GetShaderLocation(shader_, "uA");
  locB_ = GetShaderLocation(shader_, "uB");
  locC_ = GetShaderLocation(shader_, "uC");
  locMode_ = GetShaderLocation(shader_, "uMode");
  locPhoto_ = GetShaderLocation(shader_, "uPhoto");
  locTexSize_ = GetShaderLocation(shader_, "uTexSize");
  for (int i = 0; i < 46; ++i)
    motes_.push_back({{frand(), frand()},
                      {0.012f + frand() * 0.03f, 0.01f + frand() * 0.028f},
                      4.0f + frand() * 7.0f,
                      frand() * 6.28f,
                      20.0f + frand() * 70.0f});
}

void Background::shutdown() {
  for (auto& [name, texture] : photos_)
    if (texture.id != 0) UnloadTexture(texture);
  photos_.clear();
  UnloadTexture(blank_);
  UnloadShader(shader_);
}

const Texture2D& Background::photo(const char* name) {
  const auto found = photos_.find(name);
  if (found != photos_.end()) return found->second;
  Texture2D texture = {};
  const std::string path = dir_ + name + ".jpg";
  if (FileExists(path.c_str())) {
    texture = LoadTexture(path.c_str());
    if (texture.id != 0) {
      SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
      SetTextureWrap(texture, TEXTURE_WRAP_CLAMP);
    }
  }
  return photos_[name] = texture;  // un id 0 recuerda que no está y no se reintenta
}

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
  const Texture2D& image = photo(theme.photo);
  const int hasPhoto = image.id != 0 ? 1 : 0;
  const Texture2D& bound = hasPhoto ? image : blank_;
  const float texSize[2] = {static_cast<float>(bound.width), static_cast<float>(bound.height)};
  SetShaderValue(shader_, locPhoto_, &hasPhoto, SHADER_UNIFORM_INT);
  SetShaderValue(shader_, locTexSize_, texSize, SHADER_UNIFORM_VEC2);
  BeginShaderMode(shader_);
  DrawTexturePro(bound, {0.0f, 0.0f, texSize[0], texSize[1]}, {0.0f, 0.0f, w + 1.0f, h + 1.0f}, {0.0f, 0.0f},
                 0.0f, WHITE);
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
