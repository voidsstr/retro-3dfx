#!/bin/bash
# Source-level regression assertions for the 3dfx driver stack.
# Each check encodes a VERIFIED fix — if one fails, a fix has been reverted
# or clobbered (e.g. by re-syncing vintage sources). Run from repo root or
# anywhere; exits non-zero on any failure.
#
# Policy (see README.md "Driver test suite"): every time a driver fix is
# verified on hardware, add an assertion here (and a target test where
# possible) BEFORE the next deploy.

cd "$(dirname "$0")/.." || exit 2
H5DISP="3dfx Driver Code/H5/W2K/Src/Video/Displays/H5"
H5MINI="3dfx Driver Code/H5/W2K/Src/Video/Miniport/H5"
PREFIX="toolchain-3dfx/prefix/drive_c/3dfx/H5/W2K/Src/Video"
fail=0
chk() { # chk <desc> <file> <pattern>
  if grep -q -a -- "$3" "$2"; then echo "PASS  $1"; else echo "FAIL  $1  [$2 : $3]"; fail=1; fi
}

echo "== source invariants (repo tree) =="

# 1. Mip-download fix (commit 08fd889): the rev-40-deleted per-LOD address line
#    must exist in TEXTURELOAD's mipmapped path.
chk "D3TXTR mip-download addr line (black-texture fix)" \
    "$H5DISP/D3TXTR.C" \
    "addr = psurfDst->mmData\[nDstLOD\].fpVidMem - _FX(textureHeapStart\[tmuCnt\]);"

# 2. DdFlip bounded pending-swap spin (hard-freeze vector).
chk "DDFLIP pending-swap spin-breaker" \
    "$H5DISP/DDFLIP.C" \
    "retro3dfx DdFlip WEDGE-BREAK@50M"

# 3. H3MakeRoom spin-breaker + flight recorder (CFIFO wedge diagnosis).
chk "CFIFO H3MakeRoom wedge-breaker" \
    "$H5DISP/CFIFO.C" \
    "retro3dfx H3MakeRoom WEDGE-BREAK@50M"

# 4. DP2 error flight-recorder (names failing D3D op on DRIVERINTERNALERROR).
chk "D6DP2 parse-error ring logging" \
    "$H5DISP/D6DP2.C" \
    "retro3dfx DP2-PARSE-ERR"

# 5. SLI-AA promote/demote/config ring logging (SLI banding investigation).
chk "DDFXNT promote-SLIAA ring logging" \
    "$H5DISP/DDFXNT.C" \
    "retro3dfx PROMOTE-SLIAA"

# 5b. FXBUSYWAIT / H3_GP_WAIT bounded busy-spins (remaining freeze vectors).
chk "DDGLOBAL FXBUSYWAIT wedge-breaker" \
    "$H5DISP/DDGLOBAL.H" \
    "retro3dfx FXBUSYWAIT WEDGE-BREAK@100M"
chk "HW.H H3_GP_WAIT wedge-breaker" \
    "$H5DISP/HW.H" \
    "retro3dfx H3GpWait WEDGE-BREAK@100M"
chk "HW.H 2D BitBlt spin-breaker (RETRO_GP_SPIN)" \
    "$H5DISP/HW.H" \
    "retro3dfx BitBlt-GPSpin WEDGE-BREAK@100M"
chk "BITBLT.C uses bounded RETRO_GP_SPIN" \
    "$H5DISP/BITBLT.C" \
    "RETRO_GP_SPIN(ppdev)"

# 6. Registry-ring log sink (all of the above depend on it).
chk "LOGFILE registry-ring sink" \
    "$H5DISP/LOGFILE.C" \
    "RLogSeq"

# 7. v56k 6000 external clock port present in miniport build tree and PURELY
#    additive vs the vintage tree (0 vintage lines removed/changed).
chk "v56k external-clock port present (build tree)" \
    "$PREFIX/Miniport/H5/SLIAA.C" \
    "V56KFindHintBridge"
removed=$(diff -w "$H5MINI/SLIAA.C" "$PREFIX/Miniport/H5/SLIAA.C" | grep -c "^<")
if [ "$removed" -eq 0 ]; then
  echo "PASS  v56k SLIAA.C changes purely additive (0 vintage lines touched)"
else
  echo "FAIL  v56k SLIAA.C removed/changed $removed vintage lines"; fail=1
fi

echo "== repo tree vs build tree sync (fixed files must match) =="
for f in D3TXTR.C DDFLIP.C D6DP2.C DDFXNT.C CFIFO.C LOGFILE.C LOGFILE.H DEBUG.C ENABLE.C DDMEMMGR.C D3CONTXT.C DDGLOBAL.H HW.H D7D3D.C DDINIT.C MEMCHECK.H BITBLT.C DDSURF.C DDOVL32.C; do
  if cmp -s "$H5DISP/$f" "$PREFIX/Displays/H5/$f"; then
    echo "PASS  sync $f"
  else
    echo "FAIL  sync $f (repo tree != build tree — the build would not contain the repo fix)"; fail=1
  fi
done

exit $fail
