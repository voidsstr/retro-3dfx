#!/usr/bin/env bash
# Build glide3x.dll from the H5 source under Wine/VC6/MASM. Per-directory nmake in
# the documented order (NOT top-level recursion). Output: H5\BIN\glide3x.dll (96 exports).
#   toolchain-3dfx/build/build-glide3x.sh
# Deltas overlaid into the build copy (the shipped source fixes) recompile because
# they are newer than their checked-in .obj; everything else links from the drop's
# prebuilt objects. Do NOT blanket-delete *.lib — SWLIBS/LIBSRC publishes prebuilt
# libs it does not compile.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
source "$HERE/env.sh"
LOGD=/tmp/glide3x-build; mkdir -p "$LOGD"

DIRS=(
  SWLIBS/INCSRC SWLIBS/LIBSRC SWLIBS/FXMEMMAP SWLIBS/FXMISC
  SWLIBS/NEWPCI/PCILIB SWLIBS/FXREMAP SWLIBS/FXAGP
  H5/INCSRC H5/CINIT H5/MINIHWC
  H5/GLIDE3/OEM H5/GLIDE3/SRC
)
for d in "${DIRS[@]}"; do
  log="$LOGD/$(echo "$d" | tr '/' '_').log"
  wnmake "$d" "$log"
  if build_failed "$log"; then
    echo "FAIL  $d  — see $log"; grep -iE 'fatal error|U10|error C|error A|error LNK|LNK1' "$log" | head -8; exit 1
  fi
  echo "ok    $d"
done
echo "=== artifact ==="
find "$TC/prefix/drive_c/3dfx/H5" -ipath '*BIN/glide3x.dll' 2>/dev/null | while read f; do ls -la "$f"; done
