// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
// Sin raylib: <windows.h> choca con sus nombres.
#include "update/Process.hpp"

#include <filesystem>
#include <fstream>

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
extern char** environ;
#endif

namespace nm::proc {

#if defined(_WIN32)
namespace {

std::wstring widen(const std::string& text) {
  if (text.empty()) return L"";
  const int size = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
  std::wstring out(size, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), size);
  return out;
}

std::string narrow(const std::wstring& text) {
  if (text.empty()) return "";
  const int size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0,
                                       nullptr, nullptr);
  std::string out(size, '\0');
  WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), size, nullptr,
                      nullptr);
  return out;
}

// Línea de órdenes para CreateProcess: solo se entrecomilla lo que lleva espacios.
std::wstring commandLine(const std::vector<std::string>& args) {
  std::wstring line;
  for (const std::string& arg : args) {
    if (!line.empty()) line += L' ';
    const bool quote = arg.empty() || arg.find_first_of(" \t") != std::string::npos;
    if (quote) line += L'"';
    line += widen(arg);
    if (quote) line += L'"';
  }
  return line;
}

bool launch(const std::vector<std::string>& args, DWORD flags, PROCESS_INFORMATION& info) {
  for (const std::string& arg : args)
    if (arg.find('"') != std::string::npos) return false;
  std::wstring line = commandLine(args);
  STARTUPINFOW startup = {};
  startup.cb = sizeof(startup);
  return CreateProcessW(nullptr, line.data(), nullptr, nullptr, FALSE, flags, nullptr, nullptr, &startup,
                        &info) != 0;
}

}  // namespace

int run(const std::vector<std::string>& args) {
  PROCESS_INFORMATION info = {};
  if (args.empty() || !launch(args, CREATE_NO_WINDOW, info)) return -1;
  WaitForSingleObject(info.hProcess, INFINITE);
  DWORD code = 1;
  GetExitCodeProcess(info.hProcess, &code);
  CloseHandle(info.hProcess);
  CloseHandle(info.hThread);
  return static_cast<int>(code);
}

bool spawnDetached(const std::vector<std::string>& args) {
  PROCESS_INFORMATION info = {};
  if (args.empty() || !launch(args, CREATE_NO_WINDOW | CREATE_NEW_PROCESS_GROUP, info)) return false;
  CloseHandle(info.hProcess);
  CloseHandle(info.hThread);
  return true;
}

std::string tempDir() {
  wchar_t buffer[MAX_PATH + 1];
  const DWORD size = GetTempPathW(MAX_PATH + 1, buffer);
  std::string dir = narrow(std::wstring(buffer, size));
  while (!dir.empty() && (dir.back() == '\\' || dir.back() == '/')) dir.pop_back();
  return dir;
}

std::string executablePath() {
  wchar_t buffer[32768];
  const DWORD size = GetModuleFileNameW(nullptr, buffer, 32768);
  return narrow(std::wstring(buffer, size));
}

long pid() { return static_cast<long>(GetCurrentProcessId()); }

bool dirWritable(const std::string& dir) {
  const std::wstring probe = widen(dir + "\\.nm-write-test");
  const HANDLE file = CreateFileW(probe.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
  if (file == INVALID_HANDLE_VALUE) return false;
  CloseHandle(file);
  return true;
}

#else  // macOS y otros POSIX

namespace {

std::vector<char*> argv(const std::vector<std::string>& args) {
  std::vector<char*> out;
  for (const std::string& arg : args) out.push_back(const_cast<char*>(arg.c_str()));
  out.push_back(nullptr);
  return out;
}

}  // namespace

int run(const std::vector<std::string>& args) {
  if (args.empty()) return -1;
  std::vector<char*> list = argv(args);
  pid_t child = 0;
  if (posix_spawnp(&child, list[0], nullptr, nullptr, list.data(), environ) != 0) return -1;
  int status = 0;
  if (waitpid(child, &status, 0) < 0) return -1;
  return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

bool spawnDetached(const std::vector<std::string>& args) {
  if (args.empty()) return false;
  std::vector<char*> list = argv(args);
  const pid_t child = fork();
  if (child < 0) return false;
  if (child == 0) {
    setsid();  // sesión propia: sobrevive al cierre del juego
    const int null = open("/dev/null", O_RDWR);
    if (null >= 0) {
      dup2(null, 0);
      dup2(null, 1);
      dup2(null, 2);
    }
    execvp(list[0], list.data());
    _exit(127);
  }
  return true;
}

std::string tempDir() {
  std::error_code ec;
  std::string dir = std::filesystem::temp_directory_path(ec).string();
  if (ec || dir.empty()) dir = "/tmp";
  while (dir.size() > 1 && dir.back() == '/') dir.pop_back();
  return dir;
}

std::string executablePath() {
#if defined(__APPLE__)
  char buffer[4096];
  uint32_t size = sizeof(buffer);
  if (_NSGetExecutablePath(buffer, &size) != 0) return "";
  std::error_code ec;
  const auto real = std::filesystem::canonical(buffer, ec);
  return ec ? std::string(buffer) : real.string();
#else
  std::error_code ec;
  const auto real = std::filesystem::canonical("/proc/self/exe", ec);
  return ec ? "" : real.string();
#endif
}

long pid() { return static_cast<long>(getpid()); }

bool dirWritable(const std::string& dir) { return access(dir.c_str(), W_OK) == 0; }

#endif

}  // namespace nm::proc
