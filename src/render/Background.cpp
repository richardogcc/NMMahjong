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
uniform float uFog;
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

// Fotografía viva: la cámara se acerca y se desplaza, la niebla cruza en dos
// capas, entran rayos de luz y cada tema añade su propio movimiento.
vec3 photograph(vec2 uv, vec2 p, float t) {
  vec2 top = vec2(uv.x, 1.0 - uv.y);  // origen arriba a la izquierda, como la imagen
  float screen = uRes.x / uRes.y, image = uTexSize.x / uTexSize.y;
  vec2 fit = screen > image ? vec2(1.0, image / screen) : vec2(screen / image, 1.0);
  // Movimiento de cámara muy lento: se nota al cabo de un rato, no distrae.
  float zoom = 1.07 + 0.03 * sin(t * 0.05);
  vec2 pan = vec2(sin(t * 0.037), cos(t * 0.029)) * 0.012;
  vec2 st = (top - 0.5) * fit / zoom + 0.5 + pan;

  float low = smoothstep(0.5, 1.0, top.y);  // mitad inferior: agua, arena, hierba
  if (uMode == 1) {
    // Ondas que avanzan por el agua.
    float wob = noise(vec2(top.x * 6.0, top.y * 30.0 - t * 0.7));
    st += low * vec2(sin(top.y * 95.0 - t * 2.2 + wob * 6.0) * 0.0016, cos(top.x * 40.0 + t * 1.4) * 0.0008);
  } else if (uMode == 3) {
    // El viento mece la vegetación y la arena.
    float gust = noise(vec2(top.x * 3.0 - t * 0.5, top.y * 7.0));
    st.x += (gust - 0.5) * 0.003 * smoothstep(0.25, 1.0, top.y);
  }
  vec3 col = texture(texture0, st).rgb;
  float lum = dot(col, vec3(0.3, 0.59, 0.11));

  if (uMode == 0) {
    // Aurora: el cielo respira con cortinas de luz que se desplazan.
    float sky = smoothstep(0.8, 0.15, top.y);
    float veil = fbm(vec2(top.x * 3.0 + t * 0.08, top.y * 1.4 - t * 0.1));
    col *= 1.0 + sky * (veil - 0.45) * 1.3 * smoothstep(0.08, 0.5, lum);
  } else if (uMode == 1) {
    // Destellos sueltos de sol sobre el agua.
    float grain = noise(vec2(top.x * 150.0 + t * 0.35, top.y * 330.0 - t * 0.9));
    float glint = pow(grain, 10.0) * (0.5 + 0.5 * sin(t * 2.2 + top.x * 37.0 + top.y * 91.0));
    col += uC * glint * low * 0.9 * (0.3 + lum);
  } else if (uMode == 2) {
    // Noche: las luces cálidas (farolillos, ventanas) titilan.
    float warm = smoothstep(0.45, 0.9, lum) * smoothstep(0.0, 0.25, col.r - col.b);
    col *= 1.0 + warm * 0.22 * sin(t * 3.1 + top.x * 23.0 + top.y * 17.0 + noise(top * 9.0 + t) * 4.0);
  }

  // Rayos de luz que entran en diagonal (solo de día).
  if (uMode == 1 || uMode == 3) {
    float ang = top.x * 1.4 + top.y * 0.55;
    float rays = pow(0.5 + 0.5 * sin(ang * 9.0 + t * 0.3 + fbm(vec2(ang * 3.0, t * 0.12)) * 4.0), 5.0);
    col += uC * rays * smoothstep(0.95, 0.0, top.y) * 0.05;
  }

  // Niebla en dos capas que se cruzan a distinta velocidad.
  float f1 = fbm(vec2(p.x * 1.2 + t * 0.05, p.y * 2.6 + 3.0));
  float f2 = fbm(vec2(p.x * 2.3 - t * 0.08, p.y * 4.0 - t * 0.012));
  float band = smoothstep(0.9, 0.15, abs(top.y - 0.6) * 2.1);
  float fog = (smoothstep(0.45, 0.85, f1) * 0.6 + smoothstep(0.5, 0.9, f2) * 0.5) * uFog * 0.6 * (0.3 + 0.7 * band);
  col = mix(col, uC, clamp(fog, 0.0, 0.62));

  if (uMode == 0 || uMode == 2) {
    vec2 g = gl_FragCoord.xy / (uRes.y / 46.0);
    vec2 cell = floor(g);
    float seed = hash(cell);
    if (seed > 0.9 && top.y < 0.45 && lum < 0.35) {
      float d = length(fract(g) - (vec2(hash(cell + 3.1), hash(cell + 7.7)) * 0.7 + 0.15));
      col += vec3(1.0, 0.97, 0.9) * smoothstep(0.07, 0.0, d) * (0.5 + 0.5 * sin(t * (1.0 + seed * 3.0) + seed * 40.0)) * 0.7;
    }
  }
  // Las fichas tienen que leerse bien: algo menos de color y de luz.
  col = mix(col, vec3(dot(col, vec3(0.3, 0.59, 0.11))), 0.12) * 0.76;
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

// Elipse girada: sirve de pétalo o de hoja según sus proporciones.
void drawPetal(Vector2 center, float rx, float ry, float degrees, Color color) {
  const float c = std::cos(degrees * DEG2RAD), s = std::sin(degrees * DEG2RAD);
  Vector2 points[10];
  points[0] = center;
  for (int i = 0; i <= 8; ++i) {
    const float a = -i * 6.2831853f / 8.0f;  // sentido antihorario, como exige raylib
    const float x = std::cos(a) * rx, y = std::sin(a) * ry;
    points[i + 1] = {center.x + x * c - y * s, center.y + x * s + y * c};
  }
  DrawTriangleFan(points, 10, color);
}

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
  locFog_ = GetShaderLocation(shader_, "uFog");
  for (int i = 0; i < 34; ++i)
    motes_.push_back({{frand(), frand()},
                      {0.02f + frand() * 0.05f, 0.03f + frand() * 0.06f},
                      5.0f + frand() * 8.0f,
                      frand() * 6.28f,
                      40.0f + frand() * 140.0f});
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

void Background::draw(const Theme& theme, double time, float dt, bool animated) {
  const float w = static_cast<float>(GetScreenWidth());
  const float h = static_cast<float>(GetScreenHeight());
  const float t = animated ? static_cast<float>(time) : 0.0f;
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
  SetShaderValue(shader_, locFog_, &theme.fog, SHADER_UNIFORM_FLOAT);
  BeginShaderMode(shader_);
  DrawTexturePro(bound, {0.0f, 0.0f, texSize[0], texSize[1]}, {0.0f, 0.0f, w + 1.0f, h + 1.0f}, {0.0f, 0.0f},
                 0.0f, WHITE);
  EndShaderMode();

  if (!animated) return;

  // Lo que flota por delante: pétalos, hojas, luciérnagas o polvo de luz.
  const float unit = h / 900.0f;
  const bool glow = theme.motes == MoteKind::Fireflies || theme.motes == MoteKind::Dust;
  if (glow) BeginBlendMode(BLEND_ADDITIVE);
  for (Mote& m : motes_) {
    const float sway = std::sin(t * 0.9f + m.phase * 5.0f);
    const Vector2 at = {m.pos.x * w, m.pos.y * h};
    switch (theme.motes) {
      case MoteKind::Petals:
      case MoteKind::Leaves: {
        const bool leaf = theme.motes == MoteKind::Leaves;
        m.pos.x += (m.vel.x * (leaf ? 1.5f : 1.0f) + 0.035f * sway) * dt;
        m.pos.y += m.vel.y * (leaf ? 1.4f : 1.0f) * dt;
        const float tumble = std::fabs(std::cos(t * 1.7f + m.phase * 9.0f));  // gira sobre sí mismo
        drawPetal(at, m.size * unit * (leaf ? 1.7f : 1.0f), m.size * unit * (0.2f + 0.45f * tumble) * (leaf ? 0.5f : 1.0f),
                  t * m.spin * 0.4f + m.phase * 57.0f, Fade(theme.mote, 0.85f));
        break;
      }
      case MoteKind::Fireflies: {
        m.pos.x += 0.02f * std::sin(t * 0.5f + m.phase * 7.0f) * dt;
        m.pos.y += 0.018f * std::cos(t * 0.4f + m.phase * 3.0f) * dt - m.vel.y * 0.08f * dt;
        const float blink = std::pow(0.5f + 0.5f * std::sin(t * 1.6f + m.phase * 11.0f), 3.0f);
        DrawCircleV(at, m.size * unit * 1.6f, Fade(theme.mote, blink * 0.16f));
        DrawCircleV(at, m.size * unit * 0.3f, Fade(theme.mote, blink * 0.95f));
        break;
      }
      case MoteKind::Dust: {
        m.pos.x += (m.vel.x * 0.5f + 0.02f * sway) * dt;
        m.pos.y -= m.vel.y * 0.25f * dt;  // sube despacio, como polvo al sol
        const float flicker = 0.4f + 0.4f * std::sin(t * 1.3f + m.phase * 3.0f);
        DrawCircleV(at, m.size * unit * 0.9f, Fade(theme.mote, flicker * 0.1f));
        DrawCircleV(at, m.size * unit * 0.22f, Fade(theme.mote, flicker * 0.8f));
        break;
      }
      case MoteKind::Snow: {
        m.pos.x += (m.vel.x * 0.3f + 0.03f * sway) * dt;
        m.pos.y += m.vel.y * 0.9f * dt;
        DrawCircleV(at, m.size * unit * 0.34f, Fade(theme.mote, 0.8f));
        break;
      }
    }
    if (m.pos.x > 1.05f) m.pos.x = -0.05f;
    if (m.pos.x < -0.06f) m.pos.x = 1.04f;
    if (m.pos.y > 1.05f) m.pos.y = -0.05f;
    if (m.pos.y < -0.06f) m.pos.y = 1.04f;
  }
  if (glow) EndBlendMode();
}

}  // namespace nm
