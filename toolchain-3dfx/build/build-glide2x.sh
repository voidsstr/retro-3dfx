#!/usr/bin/env bash
# Build glide2x.dll from the H5 source under Wine/VC6/MASM. Glide2 and Glide3 stage
# DIFFERENT glide.h/glidesys.h/glideutl.h into the shared H5\include, so this backs
# up the glide3 headers, overlays the glide2 ones, builds with FX_HW_PROJECTS=glide,
# then restores glide3 (leaving the include dir usable for glide3 again).
# Output: H5\BIN\glide2x.dll.  Run build-glide3x.sh first (needs swlibs + minihwc libs).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
source "$HERE/env.sh"
export FX_HW_PROJECTS=glide
BC="$TC/prefix/drive_c/3dfx"
INC="$BC/H5/INCLUDE"; G2="$BC/H5/GLIDE/SRC"
LOGD=/tmp/glide2x-build; mkdir -p "$LOGD"

restore_glide3_headers() {
  for h in GLIDE.H GLIDESYS.H GLIDEUTL.H; do
    [ -f "$INC/.g3bak/$h" ] && cp -f "$INC/.g3bak/$h" "$INC/$h"
  done
}
trap restore_glide3_headers EXIT

# back up glide3 headers, overlay glide2's
mkdir -p "$INC/.g3bak"
for h in GLIDE.H GLIDESYS.H GLIDEUTL.H; do
  [ -f "$INC/$h" ] && cp -f "$INC/$h" "$INC/.g3bak/$h"
  cp -f "$G2/$h" "$INC/$h"
done

# swlibs + minihwc + cinit already built by build-glide3x.sh; build glide2 incsrc then src
for d in H5/INCSRC H5/GLIDE/SRC; do
  log="$LOGD/$(echo "$d" | tr '/' '_').log"
  wnmake "$d" "$log" "set FX_HW_PROJECTS=glide&& "
  if build_failed "$log"; then
    echo "FAIL  $d — see $log"; grep -iE 'fatal error|U10|error C|error A|error LNK|LNK1' "$log" | head -8; exit 1
  fi
  echo "ok    $d"
done
echo "=== artifact ==="
find "$BC/H5" -ipath '*BIN/glide2x.dll' 2>/dev/null | while read f; do ls -la "$f"; done
