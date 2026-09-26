#!/bin/sh
# Baut die MeeGo-Harmattan-Ausgabe (Nokia N9 / N950). Laeuft auf dem
# Build-Rechner im synchronisierten Quellbaum:
#
#   meego/build.sh arm     Binary fuers Geraet -> build/meego/arm/harbour-fahrplanat
#   meego/build.sh check   der QML-Pruefer (meego/tests/qml_check.cpp) gegen das
#                          Desktop-Qt 4 des SDK; meego/tests/check-qml.sh ruft ihn
#
# Der ARM-Build braucht die Cross-Toolchain (XGCC) und das MADDE-Sysroot
# (SYSROOT); moc kommt aus dem Qt des Simulators, das dasselbe Qt 4.7.4 ist
# wie auf dem Geraet.
set -e

HERE=$(cd "$(dirname "$0")/.." && pwd)
MODE=${1:-arm}
XGCC=${XGCC:-/tmp/xgcc-harmattan}
SYSROOT=${SYSROOT:-$HOME/QtSDK/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim}
SIMQT=${SIMQT:-$HOME/QtSDK/Simulator/Qt/gcc}
JOBS=${JOBS:-8}
VERSION=${VERSION:-$(sh "$HERE/meego/version.sh")}
OUT=$HERE/build/meego/$MODE
mkdir -p "$OUT"

# src/main.cpp (SailfishApp) wird durch meego/main.cpp ersetzt, und
# src/maplauncher.cpp (Pure Maps ueber D-Bus) durch die Harmattan-Fassung.
APP_SRC="src/appsettings.cpp \
 src/hafasclient.cpp \
 src/hafasparser.cpp \
 src/hafastypes.cpp \
 src/himmodel.cpp \
 src/journeymodel.cpp \
 src/locationmodel.cpp \
 src/networkfactory.cpp \
 src/recentmodel.cpp \
 meego/main.cpp \
 meego/maplauncher_meego.cpp \
 meego/polylineitem.cpp \
 meego/qt4replyhandler.cpp \
 meego/compat/qt4json.cpp"

# Kopfdateien mit einem Q_OBJECT darin.
MOC_HEADERS="src/appsettings.h \
 src/hafasclient.h \
 src/himmodel.h \
 src/journeymodel.h \
 src/locationmodel.h \
 src/maplauncher.h \
 src/networkfactory.h \
 src/recentmodel.h \
 meego/qt4replyhandler.h \
 meego/polylineitem.h"

INCLUDES="-I$HERE -I$HERE/src -I$HERE/meego -I$HERE/meego/compat"

DEFINES="-DAPP_VERSION='\"$VERSION\"' -DQT_NO_DEBUG"

# -Wno-register: die Qt-4.7-Kopfdateien sind aelter als die Sprache, in der
# der Rest uebersetzt wird.
COMMON_FLAGS="-std=gnu++11 -O2 -Wall -Wno-register -Wno-deprecated-declarations \
 -Wno-unused-parameter $DEFINES $INCLUDES -include $HERE/meego/compat/qt4compat.h"

QT4_MODULES="QtCore QtGui QtNetwork QtDeclarative QtDBus"

case "$MODE" in
arm)
    CXX=$XGCC/bin/arm-none-linux-gnueabi-g++
    [ -x "$CXX" ] || { echo "Cross-Compiler fehlt: $CXX" >&2; exit 1; }
    MOC=$SIMQT/bin/moc
    QTINC=$SYSROOT/usr/include/qt4
    CXXFLAGS="--sysroot=$SYSROOT $COMMON_FLAGS -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    # Harmattan ist hard-float, hat aber den alten Loadernamen behalten; die
    # statische libstdc++ von GCC 14 bleibt dem Binary privat, damit Qt auf
    # dem Geraet weiter seine eigene (GCC 4.4) benutzt.
    LDFLAGS="--sysroot=$SYSROOT -static-libstdc++ -static-libgcc -Wl,-O1 -Wl,--as-needed \
 -Wl,--exclude-libs,ALL -Wl,--dynamic-linker=/lib/ld-linux.so.3"
    LIBS="-lQtDeclarative -lQtDBus -lQtGui -lQtNetwork -lQtCore -lpthread"
    ;;
check)
    # Der QML-Pruefer wird gegen das einfache Desktop-Qt des SDK (4.8.1)
    # gebaut: das Qt des Simulators bricht ohne Bildschirm ab, sogar fuer eine
    # Anwendung ohne Oberflaeche. QtDeclarative 1 liest dieselbe Sprache.
    CXX=${CXX:-g++}
    PROBEQT=${PROBEQT:-$HOME/QtSDK/Desktop/Qt/4.8.1/gcc}
    MOC=$PROBEQT/bin/moc
    QTINC=$PROBEQT/include
    APP_SRC=$(echo "$APP_SRC" | sed 's| meego/main.cpp | meego/tests/qml_check.cpp |')
    CXXFLAGS="$COMMON_FLAGS -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    LDFLAGS="-L$PROBEQT/lib -Wl,-rpath,$PROBEQT/lib"
    LIBS="-lQtDeclarative -lQtDBus -lQtGui -lQtNetwork -lQtCore -lpthread"
    ;;
probe)
    # Dieselben Objekte wie die App, nur ohne Oberflaeche: laeuft auf dem
    # Geraet und prueft TLS, HAFAS und den JSON-Ersatz.
    CXX=$XGCC/bin/arm-none-linux-gnueabi-g++
    [ -x "$CXX" ] || { echo "Cross-Compiler fehlt: $CXX" >&2; exit 1; }
    MOC=$SIMQT/bin/moc
    QTINC=$SYSROOT/usr/include/qt4
    CXXFLAGS="--sysroot=$SYSROOT $COMMON_FLAGS -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    LDFLAGS="--sysroot=$SYSROOT -static-libstdc++ -static-libgcc -Wl,-O1 -Wl,--as-needed \
 -Wl,--exclude-libs,ALL -Wl,--dynamic-linker=/lib/ld-linux.so.3"
    LIBS="-lQtNetwork -lQtCore -lpthread"
    mkdir -p "$OUT"
    $MOC "$HERE/meego/tests/hafas_probe.cpp" -o "$OUT/hafas_probe.moc"
    for h in appsettings hafasclient locationmodel recentmodel; do
        $MOC "$HERE/src/$h.h" -o "$OUT/moc_$h.cpp"
    done
    $MOC "$HERE/meego/qt4replyhandler.h" -o "$OUT/moc_qt4replyhandler.cpp"
    $CXX $CXXFLAGS -I"$OUT" $LDFLAGS -o "$OUT/hafas_probe" \
        "$HERE/meego/tests/hafas_probe.cpp" \
        "$HERE/src/appsettings.cpp" "$HERE/src/hafasclient.cpp" \
        "$HERE/src/hafasparser.cpp" "$HERE/src/hafastypes.cpp" \
        "$HERE/src/locationmodel.cpp" "$HERE/src/recentmodel.cpp" \
        "$HERE/meego/qt4replyhandler.cpp" "$HERE/meego/compat/qt4json.cpp" \
        "$OUT/moc_appsettings.cpp" "$OUT/moc_hafasclient.cpp" \
        "$OUT/moc_locationmodel.cpp" "$OUT/moc_recentmodel.cpp" \
        "$OUT/moc_qt4replyhandler.cpp" $LIBS
    echo "== gebaut: $OUT/hafas_probe"
    exit 0 ;;
*)
    echo "Aufruf: $0 arm|check|probe" >&2; exit 2 ;;
esac

MK=$OUT/Makefile
{
    echo "CXX=$CXX"
    echo "MOC=$MOC"
    echo "CXXFLAGS=$CXXFLAGS"
    echo "LDFLAGS=$LDFLAGS"
    echo "LIBS=$LIBS"
    echo "SRC=$HERE"
    echo
    objs=
    for s in $APP_SRC; do
        o=$(echo "$s" | sed 's|/|_|g; s|\.cpp$|.o|'); objs="$objs $o"
        echo "$o: \$(SRC)/$s"; printf '\t$(CXX) $(CXXFLAGS) -I. -c $< -o $@\n'
    done
    for h in $MOC_HEADERS; do
        n=$(basename "$h" .h); objs="$objs moc_$n.o"
        echo "moc_$n.cpp: \$(SRC)/$h"; printf '\t$(MOC) $< -o $@\n'
        echo "moc_$n.o: moc_$n.cpp"; printf '\t$(CXX) $(CXXFLAGS) -c $< -o $@\n'
    done
    echo "OBJS=$objs"
    echo "all: harbour-fahrplanat"
    echo "harbour-fahrplanat: \$(OBJS)"
    printf '\t$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)\n'
} > "$MK"

nice make -C "$OUT" -j"$JOBS" all
echo "== gebaut: $OUT/harbour-fahrplanat"
