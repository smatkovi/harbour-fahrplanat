#!/bin/sh
# Packt den ARM-Build als Harmattan-.deb. Laeuft auf dem Build-Rechner nach
# "meego/build.sh arm":
#
#   meego/build-deb.sh                 # -> build/meego/harbour-fahrplanat_<version>_armel.deb
#   VERSION=0.1.6-meego2 meego/build-deb.sh
#
# Ablage auf dem Geraet:
#   /opt/harbour-fahrplanat/bin/harbour-fahrplanat
#   /opt/harbour-fahrplanat/qml
#   /usr/share/applications/harbour-fahrplanat.desktop
#   /usr/share/icons/hicolor/80x80/apps/harbour-fahrplanat.png
#
# Das .deb schreibt mkdeb.py: Harmattans dpkg ist 1.15 und will gzip-Glieder
# und keinen Schraegstrich an den ar-Namen, was GNU ar beides anders macht.
set -e

HERE=$(cd "$(dirname "$0")/.." && pwd)
PKG=$HERE/meego
OUT=$HERE/build/meego
BIN=$OUT/arm/harbour-fahrplanat
XGCC=${XGCC:-/tmp/xgcc-harmattan}
VERSION=${VERSION:-$(sh "$PKG/version.sh")}

[ -x "$BIN" ] || { echo "ARM-Binary fehlt: $BIN (erst meego/build.sh arm)" >&2; exit 1; }
[ -f "$PKG/qml/harbour-fahrplanat.qml" ] || { echo "QML fehlt (erst meego/port-qml.sh)" >&2; exit 1; }

STAGE=$OUT/stage
rm -rf "$STAGE"
mkdir -p "$STAGE/DEBIAN" "$STAGE/opt/harbour-fahrplanat/bin" \
         "$STAGE/usr/share/applications" "$STAGE/usr/share/icons/hicolor/80x80/apps" \
         "$STAGE/usr/share/themes/base/meegotouch/icons"

cp "$BIN" "$STAGE/opt/harbour-fahrplanat/bin/harbour-fahrplanat"
"$XGCC/bin/arm-none-linux-gnueabi-strip" "$STAGE/opt/harbour-fahrplanat/bin/harbour-fahrplanat"
chmod 755 "$STAGE/opt/harbour-fahrplanat/bin/harbour-fahrplanat"

# Das QML, ohne das, was nur zum Erzeugen da ist: hand/ sind die Vorlagen,
# die port-qml.sh ueber die erzeugten Dateien kopiert.
cp -a "$PKG/qml" "$STAGE/opt/harbour-fahrplanat/qml"
rm -rf "$STAGE/opt/harbour-fahrplanat/qml/hand"

# Symbole: 80x80 fuer den Startbildschirm, 64x64 als base64 fuer den
# Programm-Manager. Beide schneidet meego/icons/make-icon.py auf die exakte
# Silhouette der mitgelieferten Apps.
cp "$PKG/icons/icon-80.png" "$STAGE/usr/share/icons/hicolor/80x80/apps/harbour-fahrplanat.png"
cp "$PKG/icons/icon-80.png" "$STAGE/usr/share/themes/base/meegotouch/icons/harbour-fahrplanat-80.png"
cp "$PKG/harbour-fahrplanat.desktop" "$STAGE/usr/share/applications/harbour-fahrplanat.desktop"

# control samt Icon. Die base64-Zeilen brauchen je ein fuehrendes Leerzeichen,
# sonst zeigt der Manager kein Bild.
VERSION="$VERSION" ICON="$PKG/icons/icon-64.png" python3 - "$PKG/control.in" "$STAGE/DEBIAN/control" <<'PY'
import base64, os, sys, textwrap
src, dst = sys.argv[1], sys.argv[2]
ctl = open(src).read()
b64 = base64.b64encode(open(os.environ["ICON"], "rb").read()).decode("ascii")
icon = "\n".join(" " + line for line in textwrap.wrap(b64, 76))
ctl = ctl.replace("@VERSION@", os.environ["VERSION"]).replace("@ICON@", icon)
open(dst, "w").write(ctl)
PY

DEB="$OUT/harbour-fahrplanat_${VERSION}_armel.deb"
python3 "$PKG/mkdeb.py" "$STAGE" "$DEB"
echo "== $DEB"
