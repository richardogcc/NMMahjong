#!/usr/bin/env bash
# Compila ambas plataformas y deja en dist/ los paquetes listos para publicar:
#   NMMahjong-<versión>-macOS-universal.dmg   (Apple Silicon e Intel)
#   NMMahjong-<versión>-macOS-arm64.dmg       (el mismo, con el nombre que buscan las versiones hasta la 0.4.0)
#   NMMahjong-<versión>-Windows-x64.zip
set -euo pipefail
cd "$(dirname "$0")/.."

VERSION="$(sed -n 's/^project(NMMahjong VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt)"
[ -n "$VERSION" ] || { echo "No se pudo leer la versión de CMakeLists.txt" >&2; exit 1; }

scripts/build-mac.sh
scripts/build-windows.sh

rm -rf dist
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

rm -rf dist/stage-mac dist/stage-win
echo "Paquetes de la versión $VERSION:"
ls -lh dist
