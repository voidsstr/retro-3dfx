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
H5GLIDE="3dfx Driver Code/H5/GLIDE3/SRC"
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

# 5d. DrvBitBlt/DrvCopyBits NULL-guard (Q3 1024->1280 mode-change bugcheck
#     1000008E @ 3dfxv5d+0x346). The ENABLE_LOG_FILE block in DrvBitBlt used to
#     deref psoSrc unconditionally; psoSrc is NULL for solid/pattern blts and,
#     while a mode change leaves the primary as a DIB, the psoDst branch fails
#     and the next solid fill faulted. The psoSrc branch MUST be null-guarded.
chk "BITBLT DrvBitBlt psoSrc log-block null-guard" \
    "$H5DISP/BITBLT.C" \
    "else if ((psoSrc != NULL) && (psoSrc->dhsurf != NULL) && (! (((DSURF \*)psoSrc->dhsurf)->dt & DT_DIB)))"
chk "BITBLT DrvCopyBits psoDst log-block null-guard" \
    "$H5DISP/BITBLT.C" \
    "if ((psoDst != NULL) && (psoDst->dhsurf != NULL))"

# 5c. TEXBLT FourCC arity fix (UT2004 bugcheck 1000008E): the DP2 TEXBLT
#     handler must call the 7-argument adapter, never cast the 5-argument
#     Blt32_CopyFourCC to PTEXBLTFUNC (that put nSrcLOD in pDDDstSurf and
#     dereferenced it -> kernel AV in session space).
chk "DDBLT32 TEXBLT FourCC 7-arg adapter defined" \
    "$H5DISP/DDBLT32.C" \
    "Blt32_TexBltCopyFourCC(NT9XDEVICEDATA  \*ppdev,"
chk "D6DP2 TEXBLT uses the FourCC adapter" \
    "$H5DISP/D6DP2.C" \
    "pfnTexBlt = (PTEXBLTFUNC)Blt32_TexBltCopyFourCC;"
if grep -q -a -- "pfnTexBlt = (PTEXBLTFUNC)Blt32_CopyFourCC;" "$H5DISP/D6DP2.C"; then
  echo "FAIL  D6DP2 still casts 5-arg Blt32_CopyFourCC to PTEXBLTFUNC"; fail=1
else
  echo "PASS  D6DP2 no raw Blt32_CopyFourCC PTEXBLTFUNC cast"
fi

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

# 8. glide3x unbounded-spin breakers (CS color-depth-change 25s hang -> crash fix).
#    grBufferSwap/grDRIBufferSwap swap-pending spins + grFinish SST_BUSY do-while
#    must be time-bounded so a wedged chip breaks out instead of hanging forever.
chk "GGLIDE bounded-spin macro defined" \
    "$H5GLIDE/GGLIDE.C" \
    "define RETRO_BOUNDED_SPIN"
chk "GGLIDE grBufferSwap swap-pending spin bounded" \
    "$H5GLIDE/GGLIDE.C" \
    "RETRO_BOUNDED_SPIN(_grBufferNumPending() > _GlideRoot.environment.swapPendingCount);"
if grep -q -a -- "	while(_grBufferNumPending() >" "$H5GLIDE/GGLIDE.C"; then
  echo "FAIL  GGLIDE still has an unbounded while(_grBufferNumPending()) spin"; fail=1
else
  echo "PASS  GGLIDE no unbounded _grBufferNumPending spin"
fi
chk "GSST grFinish idle-wait time-bounded" \
    "$H5GLIDE/GSST.C" \
    "while(i < 3 && (GetTickCount() - _rfStart < 3000UL));"

# 9. Miniport refresh-force: use highest monitor-safe refresh per resolution
#    (EDID-validated cap so a GTF rate beyond the CRT is never selected).
chk "H3MODES refresh-force safety caps defined" \
    "$H5MINI/H3MODES.C" \
    "define RETRO_MAX_VREFRESH"
chk "H3MODES refresh-force horizontal-freq cap defined" \
    "$H5MINI/H3MODES.C" \
    "define RETRO_MAX_HFREQ_HZ"
chk "H3MODES mode-set upgrades to highest safe refresh" \
    "$H5MINI/H3MODES.C" \
    "bestEntry = scanEntry;"

# 10. ICD 0.3.8 dual-texture vertex-color fix (CS green/rainbow world root cause):
#     the _B vertex procs and Intersect_B MUST write/carry iterated colors — the
#     vintage "taco - don't bother since no it color" skip modulated GoldSrc's
#     world by stale ring colors (green walls) and uninitialized stack floats
#     (rainbow clip shards) once the 0.1.4 combine used ITERATED x TEXTURE.
ICDSST="toolchain-3dfx/prefix/drive_c/3dfx/SWLIBS/OPENGL/GLIDE3X/SST"
if grep -q -a -- "taco - don" "$ICDSST/sst_vertex.c"; then
  echo "FAIL  ICD sst_vertex.c still contains the vintage no-color skip (taco)"; fail=1
else
  echo "PASS  ICD sst_vertex.c vintage no-color skip removed"
fi
n038=$(grep -a -c "RETRO3DFX 0.3.8" "$ICDSST/sst_vertex.c")
if [ "$n038" -ge 8 ]; then
  echo "PASS  ICD _B color-store fix markers present ($n038 sites)"
else
  echo "FAIL  ICD _B color-store fix markers missing (found $n038, want >=8)"; fail=1
fi

# 11. ICD 0.3.9 worst-single-frame (maxFrame) perf instrumentation: __r3dPerfDump
#     must track and log the worst inter-swap time per window so a periodic
#     hitch (the ~1s GoldSrc walking stutter) is visible in the automated
#     goldsrc_bench even when average fps looks fine.
chk "ICD __r3dPerfDump maxFrame hitch tracking" \
    "$ICDSST/sst_export.c" \
    "maxFrame=%lums"
chk "ICD renderer string bumped to 0.3.9" \
    "toolchain-3dfx/prefix/drive_c/3dfx/SWLIBS/OPENGL/GLIDE3X/GLCORE/S_CONTXT.C" \
    "retro3dfx 0.3.9"
chk "goldsrc_bench timedemo harness present" \
    "optimized/gltest/goldsrc_bench.py" \
    "build_listenserver_cfg"

echo "== repo tree vs build tree sync (fixed files must match) =="
for f in D3TXTR.C DDFLIP.C D6DP2.C DDFXNT.C CFIFO.C LOGFILE.C LOGFILE.H DEBUG.C ENABLE.C DDMEMMGR.C D3CONTXT.C DDGLOBAL.H HW.H D7D3D.C DDINIT.C MEMCHECK.H BITBLT.C DDSURF.C DDOVL32.C DDBLT32.C FNPROTO.H; do
  if cmp -s "$H5DISP/$f" "$PREFIX/Displays/H5/$f"; then
    echo "PASS  sync $f"
  else
    echo "FAIL  sync $f (repo tree != build tree — the build would not contain the repo fix)"; fail=1
  fi
done

exit $fail
