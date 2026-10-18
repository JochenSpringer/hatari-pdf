#!/bin/bash
# Baut Hatari 2.6.1 mit PDF-Drucker unter Windows (MSYS2, "MSYS2 MinGW 64-bit").
#
# Einmalig in der MSYS2-MinGW64-Shell installieren:
#   pacman -S --needed make git mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc \
#       mingw-w64-x86_64-SDL2 mingw-w64-x86_64-zlib mingw-w64-x86_64-libpng \
#       mingw-w64-x86_64-portmidi
#
# Aufruf:  ./build-windows.sh /pfad/zum/hatari-quelltext
set -e
HATARI="${1:?Pfad zum Hatari-Quelltext angeben (Ordner mit CMakeLists.txt)}"
HERE="$(cd "$(dirname "$0")" && pwd)"
OUT="$HERE/dist"

echo "== Overlay in den Hatari-Quelltext kopieren"
cp -rv "$HERE/src/." "$HATARI/src/"

echo "== Konfigurieren und bauen"
mkdir -p "$HATARI/build" && cd "$HATARI/build"
cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make -j"$(nproc)"

echo "== Auslieferungsordner $OUT zusammenstellen"
rm -rf "$OUT"; mkdir -p "$OUT"
cp src/hatari.exe "$OUT/"
cp "$HATARI/gpl.txt" "$HATARI/readme.txt" "$OUT/"
cp -r "$HATARI/doc" "$OUT/doc"
# alle benoetigten MinGW-DLLs (SDL2, zlib, libpng, winpthread ...) mitnehmen
ldd src/hatari.exe | awk '/mingw64/ {print $3}' | sort -u | while read -r dll; do
    cp -v "$dll" "$OUT/"
done
echo
echo "Fertig: $OUT/hatari.exe"
echo "Hinweis: eine TOS-Datei (tos.img, z.B. EmuTOS) gehoert in denselben Ordner,"
echo "oder Hatari fragt beim ersten Start danach."
