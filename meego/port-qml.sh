#!/bin/sh
# Macht aus dem Sailfish-QML in qml/ QtQuick 1.1 + com.nokia.meego unter
# meego/qml/. qml/ wird nur gelesen, nie geaendert: der Sailfish-Build bleibt
# bei QtQuick 2.6 und Silica.
#
# Mechanisch Uebersetzbares steht in meego/fix-qml.py. Was Urteil braucht,
# liegt als ganze Datei in meego/qml/hand/ und wird hinterher darueber kopiert:
#
#   harbour-fahrplanat.qml  ApplicationWindow -> PageStackWindow, ohne Cover
#   ThemedPage.qml          Silicas palette gibt es nicht; hier ist es eine
#                           Seite mit schwarzem Grund und den Farben der App
set -e
cd "$(dirname "$0")/.."
OUT=meego/qml

mkdir -p "$OUT/pages" "$OUT/components"

python3 meego/fix-qml.py qml "$OUT"

# Das Cover ist eine Sailfish-Sache; Harmattan hat keine.
rm -rf "$OUT/cover"

for f in meego/qml/hand/*.qml; do
    name=$(basename "$f")
    case "$name" in
        harbour-fahrplanat.qml) cp "$f" "$OUT/$name" ;;
        *Page.qml)
            if [ -f "qml/pages/$name" ]; then cp "$f" "$OUT/pages/$name";
            else cp "$f" "$OUT/components/$name"; fi ;;
        *) cp "$f" "$OUT/components/$name" ;;
    esac
done

echo "== $(ls qml/*.qml qml/pages/*.qml qml/components/*.qml | wc -l) Dateien -> $OUT"
