#!/usr/bin/env bash
# Publica una versión en GitHub desde este equipo.
#
# Antes de ejecutarlo:
#   1. Sube la versión en CMakeLists.txt (project(... VERSION x.y.z)).
#   2. Escribe las notas en docs/releases/v<versión>.md.
#   3. Deja todo confirmado y subido (git commit + git push).
#
# Uso: scripts/release.sh            publica
#      scripts/release.sh --draft    la deja como borrador para revisarla
set -euo pipefail
cd "$(dirname "$0")/.."

VERSION="$(sed -n 's/^project(NMMahjong VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt)"
TAG="v$VERSION"
NOTES="docs/releases/$TAG.md"
DRAFT=()
[ "${1:-}" = "--draft" ] && DRAFT=(--draft)

[ -f "$NOTES" ] || { echo "Faltan las notas de la versión: $NOTES" >&2; exit 1; }
[ -z "$(git status --porcelain --untracked-files=no)" ] || { echo "Hay cambios sin confirmar; haz commit antes de publicar." >&2; exit 1; }
git fetch -q origin
[ "$(git rev-parse HEAD)" = "$(git rev-parse '@{u}')" ] || { echo "Hay commits sin subir; haz git push antes de publicar." >&2; exit 1; }
if gh release view "$TAG" >/dev/null 2>&1; then
  echo "La versión $TAG ya está publicada." >&2; exit 1
fi

scripts/package.sh

gh release create "$TAG" \
  --title "NM Mahjong $TAG" \
  --notes-file "$NOTES" \
  --target "$(git rev-parse HEAD)" \
  ${DRAFT[@]+"${DRAFT[@]}"} \
  "dist/NMMahjong-$VERSION-Windows-x64.zip#Windows (64 bits) — NMMahjong-$VERSION-Windows-x64.zip" \
  "dist/NMMahjong-$VERSION-macOS-arm64.dmg#macOS (Apple Silicon) — NMMahjong-$VERSION-macOS-arm64.dmg"

echo "Publicada: $(gh release view "$TAG" --json url --jq .url)"
