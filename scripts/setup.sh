#!/usr/bin/env bash
# Comprueba que este Mac tiene todo lo necesario para compilar NM Mahjong y lo
# compila. Pensado para un equipo recién clonado.
#
#   scripts/setup.sh              comprueba, compila para macOS y pasa las pruebas
#   scripts/setup.sh --install    además instala con Homebrew lo que falte
#   scripts/setup.sh --windows    compila también la versión de Windows
#   scripts/setup.sh --check      solo comprueba, no compila
set -uo pipefail
cd "$(dirname "$0")/.."

INSTALL=0; WINDOWS=0; CHECK_ONLY=0
for arg in "$@"; do
  case "$arg" in
    --install) INSTALL=1 ;;
    --windows) WINDOWS=1 ;;
    --check)   CHECK_ONLY=1 ;;
    -h|--help) sed -n '2,9p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) echo "Opción desconocida: $arg (usa --help)" >&2; exit 2 ;;
  esac
done

MISSING=0
ok()   { printf '  \033[32m✓\033[0m %s\n' "$1"; }
bad()  { printf '  \033[31m✗\033[0m %s\n' "$1"; MISSING=1; }
note() { printf '  \033[33m·\033[0m %s\n' "$1"; }

# Comprueba una herramienta; si falta y se pidió --install, la instala con brew.
need() {  # need <orden> <fórmula de brew> <para qué>
  if command -v "$1" >/dev/null 2>&1; then ok "$1 — $3"; return; fi
  if [ "$INSTALL" = 1 ] && command -v brew >/dev/null 2>&1; then
    echo "  Instalando $2…"
    if brew install "$2" >/dev/null 2>&1 && command -v "$1" >/dev/null 2>&1; then
      ok "$1 — instalado"; return
    fi
  fi
  bad "$1 — falta ($3). Instálalo con: brew install $2"
}

echo "Comprobando el equipo"
if [ "$(uname -s)" != "Darwin" ]; then
  bad "Este guion es para macOS. En Windows usa scripts\\setup.ps1"
  exit 1
fi
[ "$(uname -m)" = "arm64" ] && ok "Mac con Apple Silicon" || note "Mac con Intel: el juego se compilará igualmente para arm64"

if xcode-select -p >/dev/null 2>&1 && command -v clang++ >/dev/null 2>&1; then
  ok "Herramientas de línea de órdenes de Xcode (compilador)"
else
  bad "Faltan las herramientas de Xcode. Instálalas con: xcode-select --install"
fi

if command -v brew >/dev/null 2>&1; then
  ok "Homebrew"
else
  bad "Falta Homebrew (https://brew.sh); hace falta para instalar el resto"
fi

need git git "descargar el código"
need cmake cmake "preparar la compilación"
need ninja ninja "compilar"

if command -v cmake >/dev/null 2>&1; then
  CMAKE_VERSION="$(cmake --version | sed -n '1s/[^0-9]*\([0-9][0-9.]*\).*/\1/p')"
  if [ "$(printf '%s\n3.24\n' "$CMAKE_VERSION" | sort -V | head -1)" = "3.24" ]; then
    ok "CMake $CMAKE_VERSION (se necesita 3.24 o posterior)"
  else
    bad "CMake $CMAKE_VERSION es antiguo; actualízalo con: brew upgrade cmake"
  fi
fi

if [ "$WINDOWS" = 1 ]; then
  need x86_64-w64-mingw32-g++ mingw-w64 "compilar la versión de Windows"
elif command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
  ok "mingw-w64 — opcional, para la versión de Windows"
else
  note "mingw-w64 no está: solo hace falta para la versión de Windows (brew install mingw-w64)"
fi
command -v gh >/dev/null 2>&1 && ok "gh — opcional, para publicar versiones" \
  || note "gh no está: solo hace falta para publicar versiones (brew install gh)"

if [ -d build/mac/_deps/raylib-src ]; then
  ok "raylib ya descargada"
elif curl -fsI --max-time 10 https://github.com >/dev/null 2>&1; then
  ok "Conexión con GitHub (la primera compilación descarga raylib)"
else
  bad "Sin conexión con GitHub: la primera compilación necesita descargar raylib"
fi

if [ "$MISSING" = 1 ]; then
  echo
  echo "Falta algo de lo anterior. Corrígelo o vuelve a lanzar con --install."
  exit 1
fi
echo "Todo listo."
[ "$CHECK_ONLY" = 1 ] && exit 0

echo
echo "Compilando para macOS"
scripts/build-mac.sh || { echo "La compilación para macOS ha fallado." >&2; exit 1; }
if [ "$WINDOWS" = 1 ]; then
  echo
  echo "Compilando para Windows"
  scripts/build-windows.sh || { echo "La compilación para Windows ha fallado." >&2; exit 1; }
fi

echo
echo "Hecho. Para jugar:  open build/mac/NMMahjong.app"
