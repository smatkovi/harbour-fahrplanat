#!/usr/bin/env python3
"""Sailfish QML -> QtQuick 1.1 + com.nokia.meego.

Called by meego/port-qml.sh; every rule here is one thing QtQuick 1.1 or
Harmattan spells differently. Anything that needs judgement rather than a
substitution lives in meego/qml/hand/ instead.

The rules are the ones the SeaPrint port worked out on the same device; what
is new here is this app's own vocabulary -- the theme icons it asks for and
the pages it keeps by hand.
"""
import os
import re
import shutil
import sys

# Modules that do not exist on Harmattan. The singletons among them
# (Mimer, IppDiscovery, ConvertChecker, SeaPrintSettings, RangeListChecker)
# are root context properties in meego/main.cpp, which the QML reads by the
# same names, so dropping the import is all that is needed.
DROP_IMPORTS = [
    # Die Kartenuebergabe an Pure Maps gibt es hier nicht; maps ist eine
    # Kontext-Eigenschaft und der MeeGo-Ersatz oeffnet eine geo:-Adresse.
    r'^import Nemo\.DBus 2\.0\s*$',
    r'^import Sailfish\.Pickers 1\.0\s*$',
]


# Whole blocks that have no mechanical answer. Each is matched against the
# upstream text exactly, so that a change upstream is noticed here rather than
# silently dropped.
FILE_PATCHES = {
}

SUBST = [
    # --- language -----------------------------------------------------------
    (r'^import QtQuick 2\.[0-9]+\s*$', 'import QtQuick 1.1'),
    (r'\breadonly property ', 'property '),
    (r'\bproperty var\b', 'property variant'),
    # Auch in Signalparametern: QtQuick 1.1 kennt nur "variant", und ein
    # "var" dort ist kein Syntaxfehler, sondern ein "Expected parameter type".
    (r'^(\s*signal\s+\w+\s*\([^)]*)\bvar\b', r'\1variant'),

    # --- the theme ----------------------------------------------------------
    # com.nokia.meego exports a Theme of its own, which beats a context
    # property of the same name; the port's theme object is called AppTheme.
    (r'\bTheme\.', 'AppTheme.'),
    (r'\bScreen\.width\b', 'AppTheme.screenWidth'),
    (r'\bScreen\.height\b', 'AppTheme.screenHeight'),

    # --- Silica types with a plain counterpart ------------------------------
    (r'\bSilicaFlickable\b', 'Flickable'),
    (r'\bSilicaListView\b', 'ListView'),
    (r'\bSilicaGridView\b', 'GridView'),

    # --- pages --------------------------------------------------------------
    # A Silica page lists the orientations it allows; a MeeGo page locks to one
    # or follows the device.
    (r'^\s*allowedOrientations:\s*Orientation\.All\s*$', '    orientationLock: PageOrientation.Automatic'),
    (r'^\s*allowedOrientations:\s*Orientation\.Portrait[^\n]*$', '    orientationLock: PageOrientation.LockPortrait'),
    (r'^\s*allowedOrientations:\s*Orientation\.Landscape[^\n]*$', '    orientationLock: PageOrientation.LockLandscape'),
    (r'^\s*allowedOrientations:\s*defaultAllowedOrientations\s*$\n', ''),
    (r'^\s*defaultAllowedOrientations:[^\n]*$\n', ''),
    # A Silica page can refuse the back gesture; a MeeGo page has no such
    # property -- navigation is the toolbar's, and this port shows none, so
    # the busy page cannot be left by mistake either way.
    (r'^\s*backNavigation:[^\n]*$\n', ''),
    # QtQuick 1.1's PageStack resolves a relative URL against its own
    # directory, not against the file that asked: com.nokia.meego's
    # PageStack.js calls Qt.createComponent(page) itself. Silica resolves it
    # against the caller, so upstream can pass a bare name.
    (r'pageStack\.(push|replace)\("([^"]+\.qml)"', r'pageStack.\1(Qt.resolvedUrl("\2")'),
    # PageStack takes a bool for "do not animate".
    (r'\bPageStackAction\.Immediate\b', 'true'),
    (r'\bPageStackAction\.Animated\b', 'false'),
    # Silica pages know their orientation; here the shape of the page says it.
    (r'\bpage\.isPortrait\b', '(page.height > page.width)'),
    (r'\bisPortrait\b(?!:)', '(height > width)'),

    # --- text ---------------------------------------------------------------
    # Silica fades a truncated line, a QtQuick 1.1 Text elides.
    (r'^(\s*)truncationMode:\s*TruncationMode\.\w+\s*$', r'\1elide: Text.ElideRight'),
    # Padding on a Text or a Column is QtQuick 2.
    (r'^(\s*)leftPadding:\s*([^\n]+)$', r'\1anchors.leftMargin: \2'),
    (r'^(\s*)rightPadding:\s*([^\n]+)$', r'\1anchors.rightMargin: \2'),
    (r'^(\s*)topPadding:\s*([^\n]+)$', r'\1anchors.topMargin: \2'),
    (r'^(\s*)bottomPadding:\s*([^\n]+)$', r'\1anchors.bottomMargin: \2'),

    # --- what a QtQuick 1.1 Flickable needs to scroll at all ----------------
    # A child MouseArea takes the press at once, so the Flickable never gets to
    # flick; pressDelay lets it steal the press back, which is what
    # SilicaFlickable does for itself.
    (r'^(\s*)(Flickable|ListView|GridView) \{\s*$\n(\s*)(anchors\.fill: parent)',
     r'\1\2 {\n\3\4\n\3pressDelay: 150'),

    # Qt 4.7's Qt.application knows only `active` and `layoutDirection`; the
    # version is a root context property from meego/main.cpp.
    (r'\bQt\.application\.version\b', 'appVersion'),

    # (the theme icon names are rewritten in convert(), from the table in
    #  meego/themeicons.cpp)

    # --- Tastatur -----------------------------------------------------------
    # Silicas EnterKey ist eine angehaengte Eigenschaft der virtuellen
    # Tastatur; die gibt es hier nicht. Das Symbol faellt weg, der Griff nach
    # der Eingabetaste wird zu Keys.onReturnPressed -- das meldet die
    # Harmattan-Tastatur genauso.
    (r'^\s*EnterKey\.iconSource:[^\n]*$\n', ''),
    (r'^\s*EnterKey\.enabled:[^\n]*$\n', ''),
    (r'^(\s*)EnterKey\.onClicked:', r'\1Keys.onReturnPressed:'),

    # --- die beiden Waehler ---------------------------------------------------
    # Auf Sailfish schiebt man einen Typnamen auf den Stapel; hier ist es eine
    # Datei (meego/qml/silica/DatePickerDialog.qml), die den Harmattan-Dialog
    # umhuellt.
    (r'pageStack\.push\("Sailfish\.Silica\.(Date|Time)PickerDialog"',
     r'pageStack.push(Qt.resolvedUrl("../silica/\1PickerDialog.qml")'),
    # Die 24-Stunden-Anzeige setzt die Huelle selbst.
    (r'^\s*hourMode:\s*DateTime\.\w+,?\s*$\n', ''),

    # --- Kleinkram der Komponenten -------------------------------------------
    # Silicas BusyIndicator waehlt seine Groesse ueber eine Aufzaehlung, der
    # von com.nokia.meego ueber seine Masse.
    (r'^\s*size:\s*BusyIndicatorSize\.\w+\s*$\n', ''),
    # Silicas Button hat eine Wunschbreite, der hier eine Breite.
    (r'^(\s*)preferredWidth:', r'\1width:'),

    # --- icons --------------------------------------------------------------
    # IconButton takes the source directly here; icon.source is a Silica
    # grouped property.
    (r'\bicon\.source:', 'source:'),
]

# The BusyPage and others call this on Qt.application, which has no state in
# Qt 4.7.
TOLERANT_CONNECTIONS = (
    re.compile(r'^(\s*)(target:\s*Qt\.application\s*)$', re.M),
    r'\1\2\n\1ignoreUnknownSignals: true',
)


def silica_import(relative_depth):
    """The directory import that carries the Silica stand-ins."""
    prefix = "../" * relative_depth
    return 'import com.nokia.meego 1.0\nimport "%ssilica"' % prefix


# Silicas Symbolnamen und wie sie auf diesem Geraet heissen. Uebersetzt wird
# beim Portieren und nicht durch einen eigenen Bildanbieter: ein fuer "theme"
# registrierter Anbieter verdraengt den der Plattform, und dann fehlen den
# MeeGo-Komponenten ihre eigenen Grafiken.
THEME_ICONS = [
    ("icon-cover-refresh",     "icon-m-common-refresh"),
    ("icon-m-add",             "icon-m-toolbar-add-white"),
    ("icon-m-clear",           "icon-m-toolbar-close-white"),
    ("icon-m-enter-close",     "icon-m-common-dialog-close"),
    ("icon-m-enter-next",      "icon-m-common-drilldown-arrow-inverse"),
    ("icon-m-favorite-selected", "icon-m-common-favorite-mark-selected"),
    ("icon-m-location",        "icon-m-common-location-inverse"),
    ("icon-m-refresh",         "icon-m-toolbar-refresh-white"),
    ("icon-m-remove",          "icon-m-toolbar-delete-white"),
    ("icon-m-shuffle",         "icon-m-toolbar-shuffle-white"),
    ("icon-m-time",            "icon-m-common-clock-inverse"),
]


def theme_icons():
    return THEME_ICONS


# Der Streckenverlauf auf der Karte.
#
# Auf Sailfish malt ihn ein QtQuick-2-Canvas; QtQuick 1.1 hat keines. Der
# ganze Canvas-Block wird deshalb durch ein kleines C++-Element ersetzt
# (meego/polylineitem.cpp), das fertige Bildschirmpunkte bekommt -- gerechnet
# wird weiter hier, wo auch die Kachelmathematik steht.
OVERLAY = """        PolylineItem {
            id: overlay
            anchors.fill: parent

            function legLine(leg, emphasised) {
                var pts = page.legPoints(leg)
                if (pts.length < 2) {
                    return null
                }
                var ox = tileLayer.originX
                var oy = tileLayer.originY
                var flach = []
                for (var i = 0; i < pts.length; ++i) {
                    flach.push(page.lonToX(pts[i].lon, page.zoom) - ox)
                    flach.push(page.latToY(pts[i].lat, page.zoom) - oy)
                }
                var blass = (!leg.isWalk && !emphasised && page.focusLeg >= 0) ? 0.45 : 1.0
                return {
                    "points": flach,
                    "color": leg.isWalk ? "#3a3a3a" : leg.product.color,
                    "width": emphasised ? 6 : 4,
                    "casing": emphasised ? 11 : 8,
                    "alpha": blass
                }
            }

            // Heisst wie Silicas Canvas-Methode, damit die Seite unveraendert
            // "overlay.requestPaint()" rufen kann.
            function requestPaint() {
                var neu = []
                for (var i = 0; i < page.legs.length; ++i) {
                    if (page.focusLeg >= 0 && i === page.focusLeg) {
                        continue
                    }
                    var l = legLine(page.legs[i], page.focusLeg < 0)
                    if (l) {
                        neu.push(l)
                    }
                }
                if (page.focusLeg >= 0 && page.focusLeg < page.legs.length) {
                    var f = legLine(page.legs[page.focusLeg], true)
                    if (f) {
                        neu.push(f)
                    }
                }
                lines = neu
            }

            Component.onCompleted: requestPaint()
        }
"""


def patch_map(text):
    """Den Canvas-Block in MapPage.qml gegen das gemalte Element tauschen."""
    zeilen = text.split("\n")
    anfang = None
    for i, zeile in enumerate(zeilen):
        if zeile == "        Canvas {":
            anfang = i
            break
    if anfang is None:
        raise SystemExit("meego/fix-qml.py: der Canvas-Block in MapPage.qml ist "
                         "nicht mehr da -- nachsehen, was sich oben geaendert hat")
    ende = None
    for i in range(anfang + 1, len(zeilen)):
        if zeilen[i] == "        }":
            ende = i
            break
    if ende is None:
        raise SystemExit("meego/fix-qml.py: Ende des Canvas-Blocks nicht gefunden")
    neu = zeilen[:anfang] + OVERLAY.split("\n")[:-1] + zeilen[ende + 1:]
    text = "\n".join(neu)
    # Das Element kommt aus dem eigenen Modul.
    return text.replace("import QtQuick 1.1\n",
                        "import QtQuick 1.1\nimport fahrplanat.polyline 1.0\n", 1)


def convert(text, depth, name=""):
    for old, new in FILE_PATCHES.get(name, []):
        if old not in text:
            raise SystemExit("meego/fix-qml.py: the block it patches in %s is not "
                             "there any more; check what changed upstream" % name)
        text = text.replace(old, new)

    for pattern in DROP_IMPORTS:
        text = re.sub(pattern + r'\n', '', text, flags=re.M)

    # Silica's import becomes two: the MeeGo components, then the stand-ins,
    # which have to come last so that they win where both define a name.
    text = re.sub(r'^import Sailfish\.Silica 1\.0\s*$', silica_import(depth), text, flags=re.M)

    for pattern, replacement in SUBST:
        text = re.sub(pattern, replacement, text, flags=re.M)

    for silica, blanco in theme_icons():
        text = text.replace("image://theme/" + silica + '"', "image://theme/" + blanco + '"')

    text = TOLERANT_CONNECTIONS[0].sub(TOLERANT_CONNECTIONS[1], text)

    if name == "pages/MapPage.qml":
        text = patch_map(text)
    return text



def main():
    src, dst = sys.argv[1], sys.argv[2]

    files = [("harbour-fahrplanat.qml", 0)]
    for folder in ("pages", "components", "cover"):
        path = os.path.join(src, folder)
        if not os.path.isdir(path):
            continue
        for name in sorted(os.listdir(path)):
            if name.endswith(".qml"):
                files.append((folder + "/" + name, 1))

    for name, depth in files:
        with open(os.path.join(src, name)) as f:
            text = f.read()
        out = os.path.join(dst, name)
        os.makedirs(os.path.dirname(out), exist_ok=True)
        with open(out, "w") as f:
            f.write(convert(text, depth, name))

    # Die Bilder, die die Seiten mitbringen (die Verkehrsmittel-Symbole).
    for folder in ("icons", "pages", "components"):
        path = os.path.join(src, folder)
        if not os.path.isdir(path):
            continue
        for name in sorted(os.listdir(path)):
            if name.endswith((".svg", ".png")):
                os.makedirs(os.path.join(dst, folder), exist_ok=True)
                shutil.copy(os.path.join(path, name), os.path.join(dst, folder, name))

    print("%d QML files converted" % len(files))


if __name__ == "__main__":
    main()
