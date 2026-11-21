// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "core/Release.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <optional>

namespace nm {
namespace {

// Valor de cadena de la primera clave `key` que aparezca desde `from`.
// `valueEnd` recibe la posición siguiente al valor.
std::optional<std::string> stringValue(const std::string& json, std::string_view key,
                                       std::size_t from, std::size_t* valueEnd = nullptr,
                                       std::size_t* keyPos = nullptr) {
  const std::string needle = "\"" + std::string(key) + "\"";
  std::size_t pos = json.find(needle, from);
  while (pos != std::string::npos) {
    std::size_t i = pos + needle.size();
    while (i < json.size() && std::isspace(static_cast<unsigned char>(json[i]))) ++i;
    if (i < json.size() && json[i] == ':') {
      ++i;
      while (i < json.size() && std::isspace(static_cast<unsigned char>(json[i]))) ++i;
      if (i < json.size() && json[i] == '"') {
        std::string value;
        for (++i; i < json.size() && json[i] != '"'; ++i) {
          if (json[i] == '\\' && i + 1 < json.size()) ++i;
          value.push_back(json[i]);
        }
        if (valueEnd) *valueEnd = i + 1;
        if (keyPos) *keyPos = pos;
        return value;
      }
      return std::nullopt;  // la clave existe pero su valor no es una cadena (p. ej. null)
    }
    pos = json.find(needle, pos + 1);
  }
  return std::nullopt;
}

bool safeUrl(const std::string& url, const std::string& prefix) {
  if (url.size() > 400 || url.compare(0, prefix.size(), prefix) != 0) return false;
  if (url.find("..") != std::string::npos) return false;
  return std::all_of(url.begin(), url.end(), [](unsigned char c) {
    return std::isalnum(c) || c == '.' || c == '_' || c == '-' || c == '/' || c == ':';
  });
}

bool endsWith(const std::string& text, std::string_view suffix) {
  return text.size() >= suffix.size() &&
         text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool isHex64(const std::string& text) {
  return text.size() == 64 && std::all_of(text.begin(), text.end(), [](unsigned char c) {
           return std::isdigit(c) || (c >= 'a' && c <= 'f');
         });
}

// --- SHA-256 (FIPS 180-4) --------------------------------------------------
constexpr std::uint32_t kK[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

std::uint32_t rotr(std::uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

void compress(std::uint32_t state[8], const unsigned char block[64]) {
  std::uint32_t w[64];
  for (int i = 0; i < 16; ++i)
    w[i] = (std::uint32_t{block[4 * i]} << 24) | (std::uint32_t{block[4 * i + 1]} << 16) |
           (std::uint32_t{block[4 * i + 2]} << 8) | std::uint32_t{block[4 * i + 3]};
  for (int i = 16; i < 64; ++i) {
    const std::uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
    const std::uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
    w[i] = w[i - 16] + s0 + w[i - 7] + s1;
  }
  std::uint32_t v[8];
  std::copy(state, state + 8, v);
  for (int i = 0; i < 64; ++i) {
    const std::uint32_t s1 = rotr(v[4], 6) ^ rotr(v[4], 11) ^ rotr(v[4], 25);
    const std::uint32_t ch = (v[4] & v[5]) ^ (~v[4] & v[6]);
    const std::uint32_t t1 = v[7] + s1 + ch + kK[i] + w[i];
    const std::uint32_t s0 = rotr(v[0], 2) ^ rotr(v[0], 13) ^ rotr(v[0], 22);
    const std::uint32_t maj = (v[0] & v[1]) ^ (v[0] & v[2]) ^ (v[1] & v[2]);
    const std::uint32_t t2 = s0 + maj;
    std::copy_backward(v, v + 7, v + 8);
    v[4] += t1;
    v[0] = t1 + t2;
  }
  for (int i = 0; i < 8; ++i) state[i] += v[i];
}

}  // namespace

bool parseVersion(std::string_view text, std::array<int, 3>& out) {
  out = {0, 0, 0};
  if (!text.empty() && (text[0] == 'v' || text[0] == 'V')) text.remove_prefix(1);
  if (text.empty() || text.size() > 20) return false;
  int part = 0, digits = 0;
  for (char c : text) {
    if (c == '.') {
      if (digits == 0 || ++part > 2) return false;
      digits = 0;
    } else if (c >= '0' && c <= '9') {
      if (++digits > 5) return false;
      out[part] = out[part] * 10 + (c - '0');
    } else {
      return false;
    }
  }
  return digits > 0;
}

bool isNewerVersion(std::string_view remote, std::string_view local) {
  std::array<int, 3> r{}, l{};
  if (!parseVersion(remote, r) || !parseVersion(local, l)) return false;
  return r > l;
}

bool parseRelease(const std::string& json, std::string_view repo, std::string_view assetSuffix,
                  ReleaseInfo& out) {
  out = {};
  const auto tag = stringValue(json, "tag_name", 0);
  std::array<int, 3> version{};
  if (!tag || !parseVersion(*tag, version)) return false;
  out.version = std::to_string(version[0]) + "." + std::to_string(version[1]) + "." +
                std::to_string(version[2]);

  const std::string base = "https://github.com/" + std::string(repo) + "/releases/";
  out.pageUrl = base + "latest";
  if (const auto page = stringValue(json, "html_url", 0); page && safeUrl(*page, base))
    out.pageUrl = *page;

  // Cada paquete publicado trae su huella («digest») antes de su dirección.
  std::size_t from = 0, previousEnd = 0;
  for (;;) {
    std::size_t end = 0, keyPos = 0;
    const auto url = stringValue(json, "browser_download_url", from, &end, &keyPos);
    if (!url) break;
    if (endsWith(*url, assetSuffix) && safeUrl(*url, base + "download/")) {
      out.assetUrl = *url;
      std::size_t digestKey = 0;
      const auto digest = stringValue(json, "digest", previousEnd, nullptr, &digestKey);
      if (digest && digestKey < keyPos && digest->rfind("sha256:", 0) == 0 &&
          isHex64(digest->substr(7)))
        out.sha256 = digest->substr(7);
      return true;
    }
    from = previousEnd = end;
  }
  return false;
}

std::string sha256File(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return "";
  std::uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  unsigned char block[64];
  std::uint64_t total = 0;
  for (;;) {
    in.read(reinterpret_cast<char*>(block), 64);
    const std::size_t got = static_cast<std::size_t>(in.gcount());
    total += got;
    if (got == 64) {
      compress(state, block);
      continue;
    }
    // Último bloque: relleno con 0x80, ceros y la longitud en bits.
    block[got] = 0x80;
    std::fill(block + got + 1, block + 64, static_cast<unsigned char>(0));
    if (got >= 56) {
      compress(state, block);
      std::fill(block, block + 64, static_cast<unsigned char>(0));
    }
    const std::uint64_t bits = total * 8;
    for (int i = 0; i < 8; ++i) block[63 - i] = static_cast<unsigned char>(bits >> (8 * i));
    compress(state, block);
    break;
  }
  char hex[65];
  for (int i = 0; i < 8; ++i) std::snprintf(hex + 8 * i, 9, "%08x", state[i]);
  return std::string(hex, 64);
}

}  // namespace nm
