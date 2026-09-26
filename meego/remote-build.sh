#!/bin/sh
# Baut das N9-Paket vom Telefon aus: schiebt diesen Baum auf den Build-Rechner
# (tools/buildhost.sh von nfsshift-sfos waehlt LAN oder Tunnel), stellt dort
# die Cross-Toolchain wieder her, falls ein Neustart /tmp geleert hat, baut,
# packt und holt das .deb nach ~/ps/rpms/fahrplanat/.
#
#   meego/remote-build.sh            # bauen + packen
#   meego/remote-build.sh build      # nur das ARM-Binary
#   meego/remote-build.sh check      # nur den QML-Pruefer laufen lassen
set -e
HERE=$(cd "$(dirname "$0")/.." && pwd)
HOST=$(sh "$HERE/../nfsshift-sfos/tools/buildhost.sh")
REMOTE=/tmp/fahrplanat/src
TOOLCHAIN_TAR=${TOOLCHAIN_TAR:-$HOME/ps/toolchains/xgcc-harmattan-gcc14-hardfp.tar.gz}
MODE=${1:-package}

echo "== Build-Rechner: $HOST"
ssh "$HOST" "mkdir -p $REMOTE"
rsync -a --partial --delete --exclude .git --exclude build "$HERE/" "$HOST:$REMOTE/"

if ! ssh "$HOST" test -x /tmp/xgcc-harmattan/bin/arm-none-linux-gnueabi-g++; then
    [ -f "$TOOLCHAIN_TAR" ] || { echo "Toolchain-Tarball fehlt: $TOOLCHAIN_TAR" >&2; exit 1; }
    echo "== Cross-Toolchain aus $TOOLCHAIN_TAR"
    rsync -a --partial "$TOOLCHAIN_TAR" "$HOST:/tmp/xgcc-harmattan.tar.gz"
    ssh "$HOST" "tar xzf /tmp/xgcc-harmattan.tar.gz -C /tmp && rm /tmp/xgcc-harmattan.tar.gz"
fi

case "$MODE" in
build)
    ssh "$HOST" "cd $REMOTE && sh meego/build.sh arm"
    exit 0 ;;
check)
    ssh "$HOST" "cd $REMOTE && sh meego/tests/check-qml.sh"
    exit 0 ;;
esac

ssh "$HOST" "cd $REMOTE && sh meego/build.sh arm && sh meego/build-deb.sh"
mkdir -p "$HOME/ps/rpms/fahrplanat"
rsync -a --partial "$HOST:$REMOTE/build/meego/harbour-fahrplanat_*_armel.deb" "$HOME/ps/rpms/fahrplanat/"
ls -la "$HOME/ps/rpms/fahrplanat/"
