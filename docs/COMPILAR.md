# Compilar NM Mahjong desde el código

Todo se compila en local; no hay ningún servicio de integración continua.

## En un Mac

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

Al terminar, el juego está en `build/mac/NMMahjong.app`, como binario
universal (Apple Silicon e Intel):

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

## En Linux

Con Debian o Ubuntu (en otras distribuciones los paquetes se llaman parecido):

```bash
sudo apt install build-essential cmake ninja-build git libx11-dev libxrandr-dev \
  libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev libasound2-dev
cmake -S . -B build/linux -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux
ctest --test-dir build/linux
build/linux/NMMahjong
```

Hace falta CMake 3.24 o posterior. Desde un Mac o un PC con Docker,
`scripts/build-linux.sh` hace lo mismo dentro de un contenedor de Ubuntu
22.04, pasa las pruebas y arranca el juego sin pantalla para dejar unas
capturas en `dist/linux-test/`.

## Paquetes y publicación

| Orden | Qué hace |
|---|---|
| `scripts/test-windows.sh` | Arranca el `.exe` con Wine dentro de Docker y deja capturas en `dist/windows-test/`. |
| `scripts/build-linux.sh` | Compila y prueba la versión de Linux dentro de Docker. |
| `scripts/package.sh` | Compila las tres plataformas y deja el `.dmg`, el `.zip` y el `.tar.gz` en `dist/` (la de Linux, solo si Docker está en marcha). |
| `scripts/make-cjk-font.py` | Recorta la fuente china a los caracteres que usa el juego; hay que repetirlo si cambian los textos en chino. |
| `scripts/release.sh --draft` | Publica la versión en GitHub como borrador. |
| `scripts/release.sh` | Publica la versión en GitHub. |

Antes de publicar: sube la versión en `CMakeLists.txt`, escribe las notas en
`docs/releases/v<versión>.md` y deja todos los cambios confirmados y subidos.

### Forzar una actualización

Para que las versiones antiguas dejen de poder jugarse hasta actualizarse,
añade a las notas de la versión (`docs/releases/v<versión>.md`) esta línea,
que no se ve en la página:

```html
<!-- nm-min-version: 0.7.0 -->
```

Toda copia anterior a esa versión mostrará «Actualización necesaria» en lugar
del menú, con el botón para instalarla (o para abrir la página de descargas
si no puede hacerlo sola). Conviene saber que:

- La exigencia se recuerda en el guardado, así que sigue en pie sin conexión
  y aunque versiones posteriores no repitan la línea.
- Para retirarla, publica una versión con una marca más baja, por ejemplo
  `<!-- nm-min-version: 0.0.0 -->`.
- Solo alcanza a las copias que lleven el mecanismo (0.6.2 en adelante) y que
  hayan podido consultar GitHub al menos una vez desde que se publicó.
- Una partida en curso no se interrumpe: el aviso sale al volver al menú.

El juego busca versiones nuevas en las *releases* del repositorio indicado en
`NM_UPDATE_REPO` (en `CMakeLists.txt`). Los nombres de los paquetes que genera
`scripts/package.sh` son los que el actualizador espera; no los cambies.
