# Compilar NM Mahjong desde el código

Todo se compila en local; no hay ningún servicio de integración continua.

## En un Mac (Apple Silicon)

Clona el repositorio y lanza el guion de preparación. Comprueba que el equipo
tiene lo necesario, instala con Homebrew lo que falte, compila y pasa las
pruebas:

```bash
git clone git@github.com:richardogcc/NMMahjong.git
```

```bash
cd NMMahjong
```

```bash
scripts/setup.sh --install
```

Al terminar, el juego está en `build/mac/NMMahjong.app`:

```bash
open build/mac/NMMahjong.app
```

Otras formas de usar el guion:

| Orden | Qué hace |
|---|---|
| `scripts/setup.sh --check` | Solo comprueba el equipo, sin instalar ni compilar. |
| `scripts/setup.sh` | Comprueba y compila, sin instalar nada. |
| `scripts/setup.sh --install --windows` | Además compila la versión de Windows desde el Mac. |

Lo único que el guion no instala por ti son las herramientas de Xcode
(`xcode-select --install`) y Homebrew (<https://brew.sh>).

### A mano, sin el guion

```bash
brew install cmake ninja
```

```bash
cmake -S . -B build/mac -G Ninja -DCMAKE_BUILD_TYPE=Release
```

```bash
cmake --build build/mac
```

```bash
ctest --test-dir build/mac --output-on-failure
```

La primera compilación descarga raylib, así que necesita conexión.

### Versión de Windows desde el Mac

```bash
brew install mingw-w64
```

```bash
scripts/build-windows.sh
```

El resultado es `build/windows/NMMahjong.exe`, que necesita a su lado la
carpeta `assets`.

## En un PC con Windows

Desde PowerShell, en la carpeta del repositorio clonado:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\setup.ps1 -Install
```

Comprueba e instala con `winget` Git, CMake, Ninja y el compilador MinGW-w64, y
después compila y pasa las pruebas. El juego queda en
`build\windows\NMMahjong.exe`. Si se acaba de instalar alguna herramienta y no
la encuentra, cierra PowerShell, vuelve a abrirlo y repite la orden.

A mano, con CMake, Ninja y `g++` ya instalados:

```powershell
cmake -S . -B build/windows -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
```

```powershell
cmake --build build/windows
```

## Paquetes y publicación

| Orden | Qué hace |
|---|---|
| `scripts/package.sh` | Compila ambas plataformas y deja el `.dmg` y el `.zip` en `dist/`. |
| `scripts/release.sh --draft` | Publica la versión en GitHub como borrador. |
| `scripts/release.sh` | Publica la versión en GitHub. |

Antes de publicar: sube la versión en `CMakeLists.txt`, escribe las notas en
`docs/releases/v<versión>.md` y deja todos los cambios confirmados y subidos.

El juego busca versiones nuevas en las *releases* del repositorio indicado en
`NM_UPDATE_REPO` (en `CMakeLists.txt`). Los nombres de los paquetes que genera
`scripts/package.sh` son los que el actualizador espera; no los cambies.
