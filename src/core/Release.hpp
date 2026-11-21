// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <string_view>

namespace nm {

// Datos de una versión publicada que le interesan al actualizador.
struct ReleaseInfo {
  std::string version;   // «1.2.3», sin la «v» de la etiqueta
  std::string assetUrl;  // descarga del paquete de esta plataforma
  std::string sha256;    // huella del paquete en hexadecimal; vacía si GitHub no la da
  std::string pageUrl;   // página de la versión, para descargarla a mano
  std::string minVersion;  // versión mínima que puede seguir jugando; vacía si no se exige ninguna
};

// Acepta «1.2.3», «v1.2.3», «1.2» o «1». Rechaza cualquier otra cosa.
bool parseVersion(std::string_view text, std::array<int, 3>& out);
bool isNewerVersion(std::string_view remote, std::string_view local);

// Interpreta la respuesta de la API de GitHub «releases/latest». Solo acepta
// direcciones que pertenezcan a `repo` («usuario/repositorio») y un paquete
// cuyo nombre termine en `assetSuffix`. Todo lo demás se descarta: la
// respuesta viene de la red y no se confía en ella.
//
// Si las notas de la versión contienen «nm-min-version: X.Y.Z», esa es la
// versión mínima admitida: las anteriores tienen que actualizarse para jugar.
bool parseRelease(const std::string& json, std::string_view repo, std::string_view assetSuffix,
                  ReleaseInfo& out);

// SHA-256 de un archivo en hexadecimal y minúsculas; cadena vacía si no se puede leer.
std::string sha256File(const std::filesystem::path& path);

}  // namespace nm
