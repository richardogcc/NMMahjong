#!/usr/bin/env bash
# Compila ambas plataformas y deja en dist/ los paquetes listos para publicar:
#   NMMahjong-<versión>-macOS-universal.dmg   (Apple Silicon e Intel)
#   NMMahjong-<versión>-macOS-arm64.dmg       (el mismo, con el nombre que buscan las versiones hasta la 0.4.0)
#   NMMahjong-<versión>-Windows-x64.zip
#   NMMahjong-<versión>-Linux-x64.tar.gz      (solo si Docker está en marcha)
set -euo pipefail
cd "$(dirname "$0")/.."

VERSION="$(sed -n 's/^project(NMMahjong VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt)"
[ -n "$VERSION" ] || { echo "No se pudo leer la versión de CMakeLists.txt" >&2; exit 1; }

scripts/build-mac.sh
scripts/build-windows.sh

rm -rf dist
LINUX=0
if docker info >/dev/null 2>&1; then
  scripts/build-linux.sh
  [ -n "$(ls dist/linux-test 2>/dev/null)" ] || { echo "La versión de Linux no arrancó en la prueba." >&2; exit 1; }
  rm -rf dist/linux-test
  LINUX=1
else
  echo "Docker no está en marcha: no se genera el paquete de Linux." >&2
fi
mkdir -p dist/stage-mac dist/stage-win/NMMahjong

# --- macOS: imagen de disco con la app y los textos legales ---------------
cp -R build/mac/NMMahjong.app dist/stage-mac/
cp LICENSE THIRD-PARTY-NOTICES.md dist/stage-mac/
codesign --force --deep --sign - dist/stage-mac/NMMahjong.app
ln -s /Applications dist/stage-mac/Applications
hdiutil create -quiet -volname "NM Mahjong $VERSION" -srcfolder dist/stage-mac \
  -ov -format UDZO "dist/NMMahjong-$VERSION-macOS-universal.dmg"
# Las versiones hasta la 0.4.0 buscan su actualización con este otro nombre.
cp "dist/NMMahjong-$VERSION-macOS-universal.dmg" "dist/NMMahjong-$VERSION-macOS-arm64.dmg"

# --- Windows: zip con el ejecutable, sus recursos y los textos legales ----
cp build/windows/NMMahjong.exe dist/stage-win/NMMahjong/
cp -R build/windows/assets dist/stage-win/NMMahjong/
cp LICENSE dist/stage-win/NMMahjong/LICENSE.txt
cp THIRD-PARTY-NOTICES.md dist/stage-win/NMMahjong/
(cd dist/stage-win && zip -qr "../NMMahjong-$VERSION-Windows-x64.zip" NMMahjong)

# --- Linux: tar.gz con el ejecutable, sus recursos y los textos legales ---
if [ "$LINUX" = 1 ]; then
  mkdir -p dist/stage-linux/NMMahjong
  cp build/linux/NMMahjong dist/stage-linux/NMMahjong/
  cp -R build/linux/assets dist/stage-linux/NMMahjong/
  cp LICENSE THIRD-PARTY-NOTICES.md dist/stage-linux/NMMahjong/
  COPYFILE_DISABLE=1 tar -czf "dist/NMMahjong-$VERSION-Linux-x64.tar.gz" -C dist/stage-linux NMMahjong
fi

rm -rf dist/stage-mac dist/stage-win dist/stage-linux
echo "Paquetes de la versión $VERSION:"
ls -lh dist
