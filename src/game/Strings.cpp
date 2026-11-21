// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Este archivo no incluye raylib a propósito: <windows.h> choca con sus nombres.
#include "game/Strings.hpp"

#include <cstdlib>
#include <cstring>
#include <initializer_list>

#include "game/StringsData.hpp"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace nm {
namespace {

Lang current = Lang::Es;

const char* const* const kTables[kLangCount] = {kStrEs, kStrEn, kStrFr, kStrDe, kStrPt, kStrZh};
const char* const* const kLevels[kLangCount] = {kLevelEs, kLevelEn, kLevelFr, kLevelDe, kLevelPt, kLevelZh};
const char* const kCodes[kLangCount] = {"es", "en", "fr", "de", "pt", "zh"};

}  // namespace

void setLang(Lang lang) { current = lang; }
Lang lang() { return current; }

Lang langFromCode(const char* code) {
  if (code)
    for (int i = 0; i < kLangCount; ++i)
      if (std::strncmp(code, kCodes[i], 2) == 0) return static_cast<Lang>(i);
  return Lang::En;
}

Lang systemLang() {
#if defined(_WIN32)
  switch (GetUserDefaultUILanguage() & 0x3ff) {
    case LANG_SPANISH: return Lang::Es;
    case LANG_FRENCH: return Lang::Fr;
    case LANG_GERMAN: return Lang::De;
    case LANG_PORTUGUESE: return Lang::Pt;
    case LANG_CHINESE: return Lang::Zh;
    default: return Lang::En;
  }
#elif defined(__APPLE__)
  Lang found = Lang::En;
  if (CFArrayRef langs = CFLocaleCopyPreferredLanguages()) {
    char code[16] = "";
    if (CFArrayGetCount(langs) > 0) {
      const auto first = static_cast<CFStringRef>(CFArrayGetValueAtIndex(langs, 0));
      if (CFStringGetCString(first, code, sizeof(code), kCFStringEncodingUTF8)) found = langFromCode(code);
    }
    CFRelease(langs);
  }
  return found;
#else
  for (const char* name : {"LC_ALL", "LC_MESSAGES", "LANG"})
    if (const char* env = std::getenv(name); env && env[0] != '\0') return langFromCode(env);
  return Lang::En;
#endif
}

const char* tr(Str key) { return kTables[static_cast<int>(current)][static_cast<int>(key)]; }

const char* levelName(int levelIndex) {
  if (levelIndex < 0 || levelIndex >= kLevelNames) return "";
  return kLevels[static_cast<int>(current)][levelIndex];
}

std::string allText(Lang lang) {
  std::string out;
  for (int i = 0; i < static_cast<int>(Str::Count); ++i) out += kTables[static_cast<int>(lang)][i];
  for (int i = 0; i < kLevelNames; ++i) out += kLevels[static_cast<int>(lang)][i];
  return out;
}

}  // namespace nm
