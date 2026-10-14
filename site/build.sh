#!/usr/bin/env bash
# Baut die Website nach _site/ (wird von .github/workflows/pages.yml aufgerufen).
# Animationen und Folien werden aus presentation/ kopiert, damit sie nur einmal im Repo liegen.
# Die Folien werden nicht neu kompiliert: vor dem Pushen lokal `typst compile slides.typ` laufen lassen.
# Lokale Vorschau:  bash site/build.sh && python3 -m http.server -d _site 8000
set -euo pipefail
cd "$(dirname "$0")/.."

rm -rf _site
cp -r site _site
rm _site/build.sh

mkdir -p _site/simulationen
cp presentation/animations/*.html _site/simulationen/
cp presentation/slides.pdf _site/wellen-und-chladni-figuren.pdf
cp presentation/figures/chladni_gallery.png _site/
cp presentation/images/chladni_portrait.png presentation/images/chladni_figures_historic.png _site/

echo "Website gebaut in _site/"
