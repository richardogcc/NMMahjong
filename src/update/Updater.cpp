// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#include "update/Updater.hpp"

#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>

#include "core/Release.hpp"
#include "update/Process.hpp"

namespace nm {
namespace {

namespace fs = std::filesystem;

const std::string kRepo = NM_UPDATE_REPO;
#if defined(_WIN32)
const char* const kAssetSuffix = "-Windows-x64.zip";
const char* const kCurl = "curl.exe";
#else
const char* const kAssetSuffix = "-macOS-arm64.dmg";
const char* const kCurl = "/usr/bin/curl";
#endif

// Solo en compilaciones de depuración: NM_UPDATE_FAKE_DIR apunta a una carpeta
// con un latest.json y el paquete, para ensayar la actualización sin red.
// Las rutas viajan como texto UTF-8; así se convierten bien también en Windows.
fs::path p8(const std::string& text) { return fs::path(std::u8string(text.begin(), text.end())); }

std::string fakeDir() {
#ifndef NDEBUG
  if (const char* dir = std::getenv("NM_UPDATE_FAKE_DIR")) return dir;
#endif
  return "";
}

bool download(const std::string& url, const std::string& target, int maxSeconds) {
  std::error_code ec;
  fs::remove(p8(target), ec);
  if (const std::string fake = fakeDir(); !fake.empty()) {
    const std::string name = url.find("api.github.com") != std::string::npos
                                 ? "latest.json"
                                 : url.substr(url.find_last_of('/') + 1);
    fs::copy_file(p8(fake) / name, p8(target), ec);
    return !ec;
  }
  return proc::run({kCurl, "-fsSL", "--proto", "=https", "--max-time", std::to_string(maxSeconds),
                    "-A", "NMMahjong-updater", "-H", "Accept: application/vnd.github+json", "-o",
                    target, url}) == 0;
}

std::string readFile(const std::string& path) {
  std::ifstream in(p8(path), std::ios::binary);
  std::ostringstream out;
  out << in.rdbuf();
  return out.str();
}

// Dónde está instalado el juego y si se puede sustituir sin ayuda.
struct Install {
  std::string target;  // macOS: el .app; Windows: la carpeta del .exe
  bool selfUpdate = false;
};

Install findInstall() {
  Install install;
  const std::string exe = proc::executablePath();
#if defined(_WIN32)
  install.target = exe.substr(0, exe.find_last_of("\\/"));
  // El relevo es un .cmd: se evitan rutas con caracteres que cmd interpreta.
  const bool plain = install.target.find_first_of("%&^!\"") == std::string::npos &&
                     proc::tempDir().find_first_of("%&^!\"") == std::string::npos;
  install.selfUpdate = !install.target.empty() && plain && proc::dirWritable(install.target);
#else
  const std::string marker = ".app/Contents/MacOS/";
  const std::size_t pos = exe.rfind(marker);
  if (pos == std::string::npos) return install;
  install.target = exe.substr(0, pos + 4);
  // Abierta desde la imagen de disco o «translocada» por macOS no se puede sustituir.
  const bool movable = exe.rfind("/Volumes/", 0) != 0 && exe.find("/AppTranslocation/") == std::string::npos;
  install.selfUpdate = movable && proc::dirWritable(install.target.substr(0, install.target.find_last_of('/')));
#endif
  return install;
}

}  // namespace

struct Updater::Shared {
  std::atomic<State> state{State::Idle};
  mutable std::mutex mutex;
  ReleaseInfo info;
  Install install;
  std::string staged;  // copia nueva ya preparada en disco
};

Updater::Updater() : shared_(std::make_shared<Shared>()) {}

Updater::State Updater::state() const { return shared_->state.load(); }

std::string Updater::version() const {
  std::lock_guard lock(shared_->mutex);
  return shared_->info.version;
}

std::string Updater::pageUrl() const {
  std::lock_guard lock(shared_->mutex);
  return shared_->info.pageUrl.empty() ? "https://github.com/" + kRepo + "/releases/latest"
                                       : shared_->info.pageUrl;
}

void Updater::check() {
  State expected = State::Idle;
  if (!shared_->state.compare_exchange_strong(expected, State::Checking)) return;
  std::thread([shared = shared_] {
    const std::string json = proc::tempDir() + "/nm-mahjong-latest.json";
    ReleaseInfo info;
    const bool found =
        download("https://api.github.com/repos/" + kRepo + "/releases/latest", json, 15) &&
        parseRelease(readFile(json), kRepo, kAssetSuffix, info) && isNewerVersion(info.version, NM_VERSION);
    std::error_code ec;
    fs::remove(p8(json), ec);
    if (!found) {
      shared->state = State::UpToDate;
      return;
    }
    const Install install = findInstall();
    {
      std::lock_guard lock(shared->mutex);
      shared->info = info;
      shared->install = install;
    }
    shared->state = install.selfUpdate ? State::Available : State::Manual;
  }).detach();
}

void Updater::install() {
  State expected = State::Available;
  if (!shared_->state.compare_exchange_strong(expected, State::Downloading)) return;
  std::thread([shared = shared_] {
    ReleaseInfo info;
    Install install;
    {
      std::lock_guard lock(shared->mutex);
      info = shared->info;
      install = shared->install;
    }
    const std::string temp = proc::tempDir();
    std::error_code ec;
    std::string staged;
    bool ok = false;
#if defined(_WIN32)
    const std::string package = temp + "\\NMMahjong-update.zip";
    staged = temp + "\\NMMahjong-update";
    ok = download(info.assetUrl, package, 900) && (info.sha256.empty() || sha256File(p8(package)) == info.sha256);
    if (ok) {
      fs::remove_all(p8(staged), ec);
      fs::create_directories(p8(staged), ec);
      // Windows 10 y posteriores traen tar, que sabe abrir archivos zip.
      ok = proc::run({"tar.exe", "-xf", package, "-C", staged}) == 0 &&
           fs::exists(p8(staged) / "NMMahjong" / "NMMahjong.exe", ec);
    }
#else
    const std::string package = temp + "/NMMahjong-update.dmg";
    const std::string mount = temp + "/NMMahjong-update-mount";
    // Se prepara en la carpeta temporal: junto a la app aparecería en Spotlight.
    staged = temp + "/NMMahjong-update.app";
    ok = download(info.assetUrl, package, 900) && (info.sha256.empty() || sha256File(p8(package)) == info.sha256);
    if (ok) {
      fs::create_directories(p8(mount), ec);
      ok = proc::run({"/usr/bin/hdiutil", "attach", package, "-nobrowse", "-readonly", "-quiet",
                      "-mountpoint", mount}) == 0;
      if (ok) {
        const std::string fresh = mount + "/NMMahjong.app";
        fs::remove_all(p8(staged), ec);
        ok = fs::exists(p8(fresh + "/Contents/MacOS/NMMahjong"), ec) &&
             proc::run({"/usr/bin/ditto", fresh, staged}) == 0;
        proc::run({"/usr/bin/hdiutil", "detach", mount, "-quiet"});
      }
      fs::remove(p8(mount), ec);
    }
#endif
    fs::remove(p8(package), ec);
    if (!ok) {
      fs::remove_all(p8(staged), ec);
      shared->state = State::Failed;
      return;
    }
    {
      std::lock_guard lock(shared->mutex);
      shared->staged = staged;
    }
    shared->state = State::Ready;
  }).detach();
}

bool Updater::restartIntoUpdate() {
  if (shared_->state.load() != State::Ready) return false;
  std::string staged, target;
  {
    std::lock_guard lock(shared_->mutex);
    staged = shared_->staged;
    target = shared_->install.target;
  }
  const std::string pidText = std::to_string(proc::pid());
  bool launched = false;
#if defined(_WIN32)
  // El relevo espera a que el juego se cierre, copia los archivos y lo abre.
  const std::string script = proc::tempDir() + "\\nm-mahjong-apply-update.cmd";
  {
    std::ofstream out(p8(script), std::ios::trunc);
    out << "@echo off\r\nchcp 65001 >nul\r\n:wait\r\n"
        << "tasklist /FI \"PID eq " << pidText << "\" 2>nul | find \"" << pidText << "\" >nul\r\n"
        << "if not errorlevel 1 (\r\n  ping -n 2 127.0.0.1 >nul\r\n  goto wait\r\n)\r\n"
        << "xcopy /E /Y /Q \"" << staged << "\\NMMahjong\\*\" \"" << target << "\\\" >nul\r\n"
        << "start \"\" \"" << target << "\\NMMahjong.exe\"\r\n"
        << "rmdir /S /Q \"" << staged << "\"\r\n"
        << "(goto) 2>nul & del \"%~f0\"\r\n";
  }
  launched = proc::spawnDetached({"cmd.exe", "/d", "/c", script});
#else
  const std::string script = proc::tempDir() + "/nm-mahjong-apply-update.sh";
  {
    std::ofstream out(p8(script), std::ios::trunc);
    out << "#!/bin/bash\n"
           "# Espera a que el juego se cierre, pone la versión nueva en su sitio y la abre.\n"
           "PID=\"$1\"; APP=\"$2\"; NEW=\"$3\"; OLD=\"$(dirname \"$3\")/NMMahjong-old.app\"\n"
           "while kill -0 \"$PID\" 2>/dev/null; do sleep 0.2; done\n"
           "[ -d \"$NEW\" ] || exit 1\n"
           "rm -rf \"$OLD\"\n"
           "if mv \"$APP\" \"$OLD\" && mv \"$NEW\" \"$APP\"; then\n"
           "  rm -rf \"$OLD\"\n"
           "else\n"
           "  [ -d \"$APP\" ] || mv \"$OLD\" \"$APP\"\n"
           "fi\n"
           "open \"$APP\"\n"
           "rm -f \"$0\"\n";
  }
  launched = proc::spawnDetached({"/bin/bash", script, pidText, target, staged});
#endif
  if (!launched) shared_->state = State::Failed;
  return launched;
}

}  // namespace nm
