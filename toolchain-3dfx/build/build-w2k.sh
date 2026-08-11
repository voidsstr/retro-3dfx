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
  ( timeout 560 wine cmd /c "set TEMP=c:\\windows\\temp&& set TMP=c:\\windows\\temp&& set DEVTOOLS=c:\\3dfxtools&& set BASEDIR=c:\\3dfxtools\\w2kddk&& bldw2k.bat ${DIRS[$t]}" ) > "$log" 2>&1
  wclean
  out="$TC/prefix/drive_c/3dfx/${OUT[$t]}"
  if [ -f "$out" ] && grep -q "1 executable built" "$log"; then
    echo "ok    $t -> $(ls -la "$out" | awk '{print $5}') bytes  ${OUT[$t]}"
  else
    echo "FAIL  $t — see $log"; grep -iE 'error|fatal|NMAKE' "$log" | grep -vi 'rebase' | head -8; exit 1
  fi
done
