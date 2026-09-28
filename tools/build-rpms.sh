#!/bin/sh
# Build the Sailfish RPMs for every architecture on the Arch build machine.
#
# Each target gets its own freshly unpacked tree inside the SDK container.
# That is the whole point of this script: the 0.1.6 packages for armv7hl and
# i486 carried an aarch64 binary, because all three targets had been built one
# after the other in the same source tree and mb2 packaged the object files
# that were already lying there. Nothing warns about it -- the RPM header says
# armv7hl either way, only `file` on the unpacked binary tells the truth, so
# this script checks that before it hands anything over.
#
#   sh tools/build-rpms.sh              # build, verify, copy to ~/ps/rpms/fahrplanat
#   BUILD_HOST=... sh tools/build-rpms.sh
set -e

SRC=$(cd "$(dirname "$0")/.." && pwd)
ARCHS=${ARCHS:-"aarch64 armv7hl i486"}
TARGET=${TARGET:-SailfishOS-5.2.0.15}
CONTAINER=${CONTAINER:-sfossdk52}
OUT=${OUT:-$HOME/ps/rpms/fahrplanat}
REMOTE=/tmp/fahrplanat-build

# On the home LAN the build machine answers directly, which is much faster than
# the tunnel; probe briefly and fall back.
if [ -n "$BUILD_HOST" ]; then
    HOST=$BUILD_HOST
elif ssh -o BatchMode=yes -o ConnectTimeout=4 sebastian@192.168.1.21 true 2>/dev/null; then
    HOST=sebastian@192.168.1.21
else
    HOST=arch
fi
echo "Baurechner: $HOST"

# Uebertragen wird der Arbeitsbaum, nicht ein Zweig: auf dem meego-Zweig stehen
# eine andere .pro und eine andere Spec, das Ergebnis waere stillschweigend die
# alte Version. Deshalb sagen, was gebaut wird, und auf main bestehen.
BRANCH=$(git -C "$SRC" rev-parse --abbrev-ref HEAD)
echo "Zweig: $BRANCH, $(grep -m1 '^Version:' "$SRC/rpm/harbour-fahrplanat.spec")"
if [ "$BRANCH" != main ] && [ -z "$ALLOW_BRANCH" ]; then
    echo "Nicht auf main -- mit ALLOW_BRANCH=1 trotzdem bauen." >&2
    exit 1
fi

ssh "$HOST" "mkdir -p $REMOTE/src"
rsync -a --partial --delete --exclude .git --exclude build --exclude '*.o' \
    "$SRC/" "$HOST:$REMOTE/src/"

# The build itself runs in tmux with its output in a file: an ssh call dies
# with the phone, a tmux session on the build machine does not.
ssh "$HOST" "cat > $REMOTE/run.sh" <<REMOTE_SCRIPT
set -e
cd $REMOTE/src
tar czf $REMOTE/src.tgz --exclude=.git .
docker cp $REMOTE/src.tgz $CONTAINER:/tmp/fahrplanat-src.tgz
docker exec $CONTAINER bash -lc '
set -e
rm -rf ~/fp-out && mkdir -p ~/fp-out
for a in $ARCHS; do
    echo "=== \$a ==="
    rm -rf ~/fpbuild-\$a && mkdir -p ~/fpbuild-\$a
    tar xzf /tmp/fahrplanat-src.tgz -C ~/fpbuild-\$a
    cd ~/fpbuild-\$a
    nice mb2 -t $TARGET-\$a build
    for r in RPMS/*.\$a.rpm; do
        case \$r in *debuginfo*|*debugsource*) continue;; esac
        cp "\$r" ~/fp-out/
    done
    cd ~ && rm -rf ~/fpbuild-\$a
done'
rm -rf $REMOTE/out
docker cp $CONTAINER:/home/mersdk/fp-out $REMOTE/out
echo "FERTIG"
REMOTE_SCRIPT

ssh "$HOST" "tmux kill-session -t fahrplanat 2>/dev/null; \
    tmux new-session -d -s fahrplanat 'sh $REMOTE/run.sh > $REMOTE/build.log 2>&1'"
echo "tmux-Sitzung fahrplanat laeuft, Log $REMOTE/build.log"

while ssh "$HOST" "tmux has-session -t fahrplanat 2>/dev/null"; do
    sleep 30
done
ssh "$HOST" "tail -3 $REMOTE/build.log"
ssh "$HOST" "grep -q FERTIG $REMOTE/build.log" || { echo "Bau fehlgeschlagen"; exit 1; }

# Was ist wirklich im Paket? Der RPM-Kopf luegt nicht, aber er sagt auch nichts
# ueber die Binaerdatei darin.
ssh "$HOST" "cd $REMOTE/out && rm -rf ../check && for r in *.rpm; do
    a=\${r##*-}; a=\${a%.rpm}; a=\${a##*.}
    mkdir -p ../check/\$a && (cd ../check/\$a && rpm2cpio ../../out/\$r | cpio -idm 2>/dev/null)
    printf '%s: ' \"\$r\"; file -b ../check/\$a/usr/bin/harbour-fahrplanat
done"

mkdir -p "$OUT"
rsync -a --partial "$HOST:$REMOTE/out/*.rpm" "$OUT/"
ls -l "$OUT"
