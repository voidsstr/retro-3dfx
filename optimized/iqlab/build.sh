#!/usr/bin/env bash
# Build iqlab.exe for Windows XP (Win32 PE) with the mingw cross-toolchain.
#
# The host has no gcc/cc on PATH; the cross toolchain lives in ~/toolchain-mingw
# and its cc1 needs the bundle's own lib dir on LD_LIBRARY_PATH or it dies on
# libisl.so.23. Both are handled here so the build is one command.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
TC="${RETRO3DFX_MINGW:-$HOME/toolchain-mingw}"
CROSS=""
for c in "$TC/usr/bin/i686-w64-mingw32-gcc-win32" "$TC/usr/bin/i686-w64-mingw32-gcc" \
         "$(command -v i686-w64-mingw32-gcc || true)"; do
  [ -n "$c" ] && [ -x "$c" ] && { CROSS="$c"; break; }
done
[ -n "$CROSS" ] || { echo "FATAL: no i686-w64-mingw32-gcc found (set RETRO3DFX_MINGW)"; exit 1; }
export LD_LIBRARY_PATH="$TC/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}"

VER=$(grep -oE '#define IQLAB_VERSION "[^"]+"' "$HERE/iqlab.c" | grep -oE '[0-9]+\.[0-9]+\.[0-9]+')
echo "building iqlab $VER with $(basename "$CROSS")"
"$CROSS" -O2 -Wall -DNDEBUG \
    -o "$HERE/iqlab.exe" "$HERE/iqlab.c" \
    -lopengl32 -lgdi32 -luser32 -lcomctl32 -ladvapi32 -lshell32 -mwindows
"$TC/usr/bin/i686-w64-mingw32-strip" "$HERE/iqlab.exe" 2>/dev/null || true
ls -la "$HERE/iqlab.exe" | awk '{printf "  iqlab.exe  %s bytes\n",$5}'
echo "  version $VER"
