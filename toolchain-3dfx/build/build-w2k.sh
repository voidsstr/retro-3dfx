#!/usr/bin/env bash
# Build the W2K/XP kernel driver pair with the W2K DDK build.exe under Wine:
#   miniport  H5\W2K\Src\Video\Miniport\H5  -> objfre\i386\3dfxvsm.sys
#   display   H5\W2K\Src\Video\Displays\H5  -> objfre\i386\3dfxvs.dll   (incl. D3D HAL)
# Usage: build-w2k.sh [miniport|display|both]   (default both)
#
# Judge success by the artifact + "1 executable built", NEVER the exit code: BUILD's
# default includes -i, and the POSTBLD 'rebase' step ALWAYS fails under Wine
# (return 0x63) — that is cosmetic, the .sys/.dll is already linked. These are the
# BSOD-risk binaries; deploy only under supervision (see deploy-3dfx-driver skill).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
source "$HERE/env.sh"
what="${1:-both}"
# DX=8 opt-in: SOURCES defaults to DX=7, which compiles out DDHALINFO_GETDRIVERINFO2
# and every other D3D8 cap, so D3D8 titles get no HAL at all (V56K-SLI-FINDINGS S16).
# Building DX=8 needs the DirectX 8 DRIVER headers, which only the XP DDK has --
# devtools/dx8ddk/inc is a CURATED set of them (it is PREPENDED to the include path,
# so anything added there shadows the W2K DDK; keep it to the DDraw/D3D interface).
#   RETRO3DFX_DX=8 build-w2k.sh display
DXLEVEL="${RETRO3DFX_DX:-7}"
if [ "$DXLEVEL" = 8 ]; then
  [ -f "$TC/devtools/dx8ddk/inc/ddrawint.h" ] || {
    echo "FAIL: DX=8 requested but devtools/dx8ddk/inc is missing." >&2
    echo "      Run setup-toolchain.sh (needs downloads/en_winxp_sp1_ddk.exe)." >&2
    exit 1; }
  grep -q DDHALINFO_GETDRIVERINFO2 "$TC/devtools/dx8ddk/inc/ddrawint.h" || {
    echo "FAIL: dx8ddk ddrawint.h has no DDHALINFO_GETDRIVERINFO2 -- wrong DDK." >&2
    exit 1; }
  # single-quoted: the backslashes must reach cmd as-is (NOT doubled -- the
  # surrounding "..." below escapes its own literals, but a var expansion does not)
  DXENV='set DX=8&& set DXDDKVERSION=8&& set DXDDK=c:\3dfxtools\dx8ddk&& '
  echo "== building with DX=8 (XP DDK headers via DXDDK) =="
else
  DXENV=''
fi
declare -A DIRS=(
  [miniport]='c:\3dfx\H5\W2K\Src\Video\Miniport\H5'
  [display]='c:\3dfx\H5\W2K\Src\Video\Displays\H5'
)
declare -A OUT=(
  [miniport]='H5/W2K/Src/Video/Miniport/H5/objfre/i386/3dfxvsm.sys'
  [display]='H5/W2K/Src/Video/Displays/H5/objfre/i386/3dfxvs.dll'
)
targets=(); [ "$what" = both ] && targets=(miniport display) || targets=("$what")
cd "$TC/prefix/drive_c/3dfx"
for t in "${targets[@]}"; do
  log="/tmp/w2k-$t.log"
  ( timeout 560 wine cmd /c "set TEMP=c:\\windows\\temp&& set TMP=c:\\windows\\temp&& set DEVTOOLS=c:\\3dfxtools&& set BASEDIR=c:\\3dfxtools\\w2kddk&& ${DXENV}bldw2k.bat ${DIRS[$t]}" ) > "$log" 2>&1
  wclean
  out="$TC/prefix/drive_c/3dfx/${OUT[$t]}"
  if [ -f "$out" ] && grep -q "1 executable built" "$log"; then
    echo "ok    $t -> $(ls -la "$out" | awk '{print $5}') bytes  ${OUT[$t]}"
  else
    echo "FAIL  $t — see $log"; grep -iE 'error|fatal|NMAKE' "$log" | grep -vi 'rebase' | head -8; exit 1
  fi
done
