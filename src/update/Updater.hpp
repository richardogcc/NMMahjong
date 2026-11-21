// NM Mahjong — © 2026 Richardo G. Cibea M. Todos los derechos reservados.
#pragma once

#include <memory>
#include <string>

namespace nm {

// Busca versiones nuevas en las «releases» de GitHub y las instala. Todo el
// trabajo de red y de disco se hace en un hilo aparte; el juego solo consulta
// el estado en cada fotograma.
class Updater {
 public:
  enum class State {
    Idle,
    Checking,
    UpToDate,     // no hay nada nuevo (o no se pudo comprobar)
    Available,    // hay versión nueva y se puede instalar sola
    Manual,       // hay versión nueva, pero hay que descargarla a mano
    Downloading,
    Ready,        // preparada: falta reiniciar
    Failed,
  };

  Updater();

  void check();
  State state() const;
  std::string version() const;  // versión nueva, cuando la hay
  std::string pageUrl() const;  // página de descargas
  // Versión mínima que exige la última publicada; vacía si sus notas no dicen nada.
  std::string minVersion() const;

  // Descarga y prepara la versión nueva (solo desde Available).
  void install();
  // Lanza el relevo que sustituye el juego y lo vuelve a abrir. Si devuelve
  // true, hay que cerrar el juego de inmediato.
  bool restartIntoUpdate();

 private:
  struct Shared;
  std::shared_ptr<Shared> shared_;
};

}  // namespace nm
