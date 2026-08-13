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
# The heavy Wine build tree normally lives OUTSIDE the repo (see
# toolchain-3dfx/build/env.sh); $RETRO3DFX_TC points at it. Fall back to the
# historical in-repo location so old checkouts keep working.
BUILDROOT="${RETRO3DFX_TC:+$RETRO3DFX_TC/prefix/drive_c/3dfx}"
[ -n "$BUILDROOT" ] && [ -d "$BUILDROOT" ] || BUILDROOT="toolchain-3dfx/prefix/drive_c/3dfx"
H5DISP="3dfx Driver Code/H5/W2K/Src/Video/Displays/H5"
H5MINI="3dfx Driver Code/H5/W2K/Src/Video/Miniport/H5"
H5GLIDE="3dfx Driver Code/H5/GLIDE3/SRC"
PREFIX="$BUILDROOT/H5/W2K/Src/Video"
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
    "retro3dfx DdFlip WEDGE-BREAK"

# 3. H3MakeRoom spin-breaker + flight recorder (CFIFO wedge diagnosis).
chk "CFIFO H3MakeRoom wedge-breaker" \
    "$H5DISP/CFIFO.C" \
    "retro3dfx H3MakeRoom WEDGE-BREAK"

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
    "retro3dfx FXBUSYWAIT WEDGE-BREAK"
chk "HW.H H3_GP_WAIT wedge-breaker" \
    "$H5DISP/HW.H" \
    "retro3dfx H3GpWait WEDGE-BREAK"
chk "HW.H 2D BitBlt spin-breaker (RETRO_GP_SPIN)" \
    "$H5DISP/HW.H" \
    "retro3dfx BitBlt-GPSpin WEDGE-BREAK"
chk "BITBLT.C uses bounded RETRO_GP_SPIN" \
    "$H5DISP/BITBLT.C" \
    "RETRO_GP_SPIN(ppdev)"

# 5c. V56K-WEDGE-WATCHDOG: every accelerator spin-breaker must use the SHARED bound,
# and that bound must stay inside Windows' ~30s video watchdog. At the original
# 50M/100M iterations (~50-100s of uncached MMIO reads) the watchdog always fired
# first and bugchecked 0xEA THREAD_STUCK_IN_DEVICE_DRIVER, so the breakers below
# were dead code -- that is what took .133 down 12 times. Guard BOTH halves:
# no raw over-long literal may come back, and the constant must stay small.
for f in CFIFO.C DDFLIP.C DDSURF.C DDGLOBAL.H HW.H; do
  chk "$f spin-breakers use RETRO_WEDGE_BREAK_SPINS" "$H5DISP/$f" "RETRO_WEDGE_BREAK_SPINS"
done
if grep -qa "100000000UL\|50000000UL" "$H5DISP"/*.C "$H5DISP"/*.H 2>/dev/null; then
  echo "FAIL  a raw 50M/100M spin bound is back (watchdog fires at ~30s)"; RC=1
else
  echo "PASS  no raw 50M/100M spin bounds remain"
fi
if grep -qa "define RETRO_WEDGE_BREAK_SPINS   2000000UL" "$H5DISP/CFIFO.C"; then
  echo "PASS  RETRO_WEDGE_BREAK_SPINS is watchdog-safe (2M ~= 2s)"
else
  echo "FAIL  RETRO_WEDGE_BREAK_SPINS changed - keep it well under the ~30s watchdog"; RC=1
fi

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

# 5f. DdBlt texture-download context-independent handle resolution (GoldSrc
#     Direct3D bugcheck 1000008E @ 3dfxv5d DdBlt+0x32C, AND garbled textures).
#     The system->video texture-download path used TXTRHNDL_PTR = pRc->pHndlList->
#     ppTxtrHndlList[h] with pRc=_D3(lastContext); GoldSrc uploads textures with NO
#     current context (lastContext is set only in the draw path and zeroed by
#     textureLoad itself) -> pRc==NULL -> [NULL+0x510] kernel AV. On W2K a surface
#     has no owning DD-local, and walking g_pContexts is NOT enough either: GoldSrc
#     restarts video repeatedly at startup and uploads issued between CTX-DESTROY
#     and the next CTX-CREATE were dropped (ring TEXDL-SKIP no-RC -> stale-white
#     textures). Walk the GLOBAL per-DDLcl handle-list chain g_pHndlList (what
#     GetHndlListPtr iterates; populated at CreateSurfaceEx, context-independent)
#     for the list resolving BOTH handles, and TEXTURELOAD through its TXTRHNDLs --
#     removes the AV AND lets uploads succeed in context-less windows.
#     textureLoad() reads no context state, so safe.
chk "DDBLT32 texture-download walks global g_pHndlList (context-independent)" \
    "$H5DISP/DDBLT32.C" \
    "for ( pHL = g_pHndlList; NULL != pHL; pHL = pHL->pNext )"
chk "DDBLT32 texture-download resolves both TXTRHNDLs from the RC handle list" \
    "$H5DISP/DDBLT32.C" \
    "pSrcTxtr    = pHL->ppTxtrHndlList"
# 5g. Mip SUBLEVEL blts (GoldSrc white-world): sublevel surfaces carry their own
#     never-registered dwSurfaceHandle; the download path must walk UP the
#     attach-from chain to the registered chain ROOT and recover the LOD index by
#     matching the blitted surface's dims against the root TXTRHNDL's mmData[]
#     (the DP2 TEXBLT mip-match idiom). Without this, mips 1..n of every world
#     texture silently dropped -> TMU minified into stale-white memory.
chk "DDBLT32 sublevel blts walk up lpAttachListFrom to the texture root" \
    "$H5DISP/DDBLT32.C" \
    "pSrcRoot = pSrcRoot->lpAttachListFrom->lpAttached;"
chk "DDBLT32 sublevel LOD recovered by mmData dimension match" \
    "$H5DISP/DDBLT32.C" \
    "(pSrcTxtr->mmData\[nSrcLvl\].wWidth  == (DWORD)pbd->lpDDSrcSurface->lpGbl->wWidth)"
chk "DDBLT32 texture-download calls TEXTURELOAD with recovered LODs" \
    "$H5DISP/DDBLT32.C" \
    "TEXTURELOAD(ppdev, pSrcTxtr, &pbd->rSrc, nSrcLvl, pDstTxtr, &pbd->rDest, nDstLvl)"
# 5h. TEXTURELOAD per-LOD tlog: source height log must index mmData[nSrcLOD]
#     (vintage typo used nDstLOD; benign only while all callers passed 0,0 --
#     load-bearing now that DdBlt passes real sublevel LODs).
chk "D3TXTR TEXTURELOAD tlog uses source LOD (nSrcLOD)" \
    "$H5DISP/D3TXTR.C" \
    "while (((psurfSrc->mmData\[nSrcLOD\].wHeight - (0x01 << tlog)) != 0)"
# 5i. ALPHA_P8 copy-paste (latent): the stage functions must OR the ALPHA_P8
#     texture format into THEIR TMU's register (T1 for stage0, T0 for stage1),
#     not the single-texture register.
chk "D6MT stage0 ALPHA_P8 goes to textureModeT1" \
    "$H5DISP/D6MT.C" \
    "pRc->sst.textureModeT1 |= ( TEXFMT_ALPHA_P8_RGB << SST_TFORMAT_SHIFT );"
chk "D6MT stage1 ALPHA_P8 goes to textureModeT0" \
    "$H5DISP/D6MT.C" \
    "pRc->sst.textureModeT0 |= ( TEXFMT_ALPHA_P8_RGB << SST_TFORMAT_SHIFT );"

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
ICDSST="$BUILDROOT/SWLIBS/OPENGL/GLIDE3X/SST"
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
chk "ICD renderer string current (>=0.4.0)" \
    "$BUILDROOT/SWLIBS/OPENGL/GLIDE3X/GLCORE/S_CONTXT.C" \
    "retro3dfx 0.4.0"
chk "goldsrc_bench timedemo harness present" \
    "optimized/gltest/goldsrc_bench.py" \
    "build_listenserver_cfg"

# 12. Desktop gamma-persist hardening (washed-out Windows desktop after a game
#     exits/crashes). A full-screen Glide/OpenGL app (Quake3 r_overBrightBits
#     doubles the ramp -> out=min(255,2*in)) sets gamma via DrvIcmSetDeviceGammaRamp;
#     the vintage code wrote EVERY ramp into the persistent desktop GammaTable and
#     saved it to the registry, so a game (or a crash) left the desktop washed out
#     permanently. Three guards must be present in PALETTE.C:
#     (a) DrvIcmSetDeviceGammaRamp routes an app ramp to TRANSIENT (no persist)
#         when a full-screen app owns the hw (!bEnabled) or the ramp is overbright;
#     (b) vAssertModePalette re-applies GAMMA_DESKTOP on return-to-desktop (runs
#         from DrvAssertMode ENABLE, so the desktop recovers even after a crash);
#     (c) bInitializePalette rejects a degenerate persisted GammaTable -> identity.
chk "PALETTE DrvIcm case-A full-screen app -> transient (no desktop persist)" \
    "$H5DISP/PALETTE.C" \
    "if (! ppdev->bEnabled)"
chk "PALETTE DrvIcm case-A routes app ramp to GAMMA_TRANSIENT" \
    "$H5DISP/PALETTE.C" \
    "ppdev->TransientGammaTable\[i\] = ((pGammaRamp->Red\[i\]"
chk "PALETTE DrvIcm case-B rejects overbright desktop push (re-assert GAMMA_DESKTOP)" \
    "$H5DISP/PALETTE.C" \
    "if (((ULONG)(pGammaRamp->Green\[128\] >> 8) & 0xFF) >= 0xE0)"
chk "PALETTE vAssertModePalette restores GAMMA_DESKTOP on desktop re-enable" \
    "$H5DISP/PALETTE.C" \
    "if (bEnable && (ppdev->iBitmapFormat != BMF_8BPP))"
chk "PALETTE bInitializePalette degenerate-gamma self-heal" \
    "$H5DISP/PALETTE.C" \
    "(((ppdev->GammaTable\[128\] >> 8) & 0xFF) >= 0xE0)"

# 13. Vsync default ON (driver-level). The vintage Voodoo5 INF shipped
#     FX_GLIDE_SWAPINTERVAL="0", which overrode the ICD's grBufferSwap(1) request
#     -> no vblank wait (200+fps tearing/beat). The INF must ship "1" (vsync on)
#     and the "Vertical Sync" tweak must default to Enable.
chk "INF ships FX_GLIDE_SWAPINTERVAL=1 (vsync on by default)" \
    "3dfx Driver Code/H5/W2K/Src/Video/Inf/Voodoo5/3DFXVS2K.INF" \
    "HKR,Glide,FX_GLIDE_SWAPINTERVAL,,\"1\""
if grep -q -a -- "HKR,Glide,FX_GLIDE_SWAPINTERVAL,,\"0\"" "3dfx Driver Code/H5/W2K/Src/Video/Inf/Voodoo5/3DFXVS2K.INF"; then
  echo "FAIL  Voodoo5 INF still forces FX_GLIDE_SWAPINTERVAL=0 (vsync off)"; fail=1
else
  echo "PASS  Voodoo5 INF no longer forces vsync off"
fi

# 14. ICD 0.4.0: 1280x1024 support + highest-safe per-resolution refresh. The
#     resolution table must include the 1280x1024 row and carry a refresh
#     column, the "out of resolutions" sentinel must be the new top entry, and
#     grSstWinOpen must pass the per-resolution refresh (NOT a hardcoded 60Hz),
#     else a 1280x1024 window wedges the hardware / the game is stuck at 60Hz.
chk "ICD SST_RESOLUTION enum includes 1280x1024" \
    "$ICDSST/sst_export.c" \
    "SST_1280x1024,"
chk "ICD __sstResTable has the 1280x1024 row (GR_RESOLUTION_1280x1024)" \
    "$ICDSST/sst_export.c" \
    "{ 1280, 1024, GR_RESOLUTION_1280x1024, GR_REFRESH_75Hz }"
chk "ICD __sstResTable carries a refresh column ([4])" \
    "$ICDSST/sst_export.c" \
    "__sstResTable\[SST_RESOLUTIONS\]\[4\]"
chk "ICD mode-walk sentinel moved to SST_1280x1024" \
    "$ICDSST/sst_export.c" \
    "res == SST_1280x1024 )"
chk "ICD grSstWinOpen uses per-resolution refresh (not hardcoded 60Hz)" \
    "$ICDSST/sst_export.c" \
    "resolution, __sstResTable\[res\]\[3\],"
if grep -q -a -- "resolution, GR_REFRESH_60Hz," "$ICDSST/sst_export.c"; then
  echo "FAIL  ICD still calls grSstWinOpen with a hardcoded GR_REFRESH_60Hz"; fail=1
else
  echo "PASS  ICD no hardcoded GR_REFRESH_60Hz in grSstWinOpen"
fi

# 15. glide2x shutdown wedge-breaks (UT99 exit hang: grSstWinClose spun
#     forever at 98% CPU when the accelerator wedged with SST_BUSY stuck,
#     so hwcRestoreVideo never ran and the desktop was never restored).
#     Both user-mode spins on the WinClose path must be bounded.
G2SRC="3dfx Driver Code/H5/GLIDE/SRC"
G2PREFIX="$BUILDROOT/H5/GLIDE/SRC"
chk "glide2 grSstIdle bounded busy-poll (WEDGE-BREAK)" \
    "$G2SRC/GSST.C" \
    "WEDGE-BREAK: hw stuck busy"
chk "glide2 fifo makeroom bounded stall (WEDGE-BREAK)" \
    "$G2SRC/FIFO.C" \
    "WEDGE-BREAK: force room"

# 16. Blt-present -> page-flip promotion (CS-D3D fillrate fix, 2026-08-03):
#     GoldSrc-D3D presents by full-screen Blt (never DdFlip); the promotion
#     queues a real overlay flip and ping-pongs the app back buffer with a
#     B2 from the third tiled slot. First version ping-ponged into the GDI
#     desktop buffer and HARD-WEDGED the chip (color/Z tile parity) - B2
#     MUST come from a tiled color slot, and the BACKBUFFER heap search
#     MUST include TILED_HEAP2 (else B2 alloc fails OUTOFVIDEOMEMORY and
#     the promotion silently disables).
chk "flip-present promotion present in DDFLIP.C" \
    "$H5DISP/DDFLIP.C" \
    "retroFlipPresent ( NT9XDEVICEDATA"
chk "flip-present restores backing on surface destroy" \
    "$H5DISP/DDSURF.C" \
    "retroFlipPresentSurfGone((void \*)psurf_gbl->dwReserved1)"
chk "BACKBUFFER heap search includes the third tiled slot (B2)" \
    "$H5DISP/DDMEMMGR.C" \
    "TILED_HEAP2_ID : LINEAR_HEAP1_ID"
chk "flip-present session generation bumped at 3D enter" \
    "$H5DISP/DDFXNT.C" \
    "g_retroFlipGen++"
chk "promoted swaps do not wait on vsync (33.8 vs 34.3 fps measured)" \
    "$H5DISP/DDFLIP.C" \
    "SETPD(hwPtr, ghw0->swapbufferCMD, 0);"
if grep -q -a "srcData->hwPtr  = dstData->hwPtr" "$H5DISP/DDFLIP.C"; then
  echo "FAIL  flip-present still swaps with the PRIMARY surface (the desktop-buffer parity wedge)"; fail=1
else
  echo "PASS  flip-present never ping-pongs into the GDI desktop buffer"
fi

echo "== glide2 repo tree vs build tree sync =="
for f in GSST.C FIFO.C; do
  if cmp -s "$G2SRC/$f" "$G2PREFIX/$f"; then
    echo "PASS  sync GLIDE/SRC/$f"
  else
    echo "FAIL  sync GLIDE/SRC/$f (repo tree != build tree — the build would not contain the repo fix)"; fail=1
  fi
done

echo "== repo tree vs build tree sync (fixed files must match) =="
for f in D3TXTR.C DDFLIP.C D6DP2.C DDFXNT.C CFIFO.C LOGFILE.C LOGFILE.H DEBUG.C ENABLE.C DDMEMMGR.C D3CONTXT.C DDGLOBAL.H HW.H D7D3D.C DDINIT.C MEMCHECK.H BITBLT.C DDSURF.C DDOVL32.C DDBLT32.C FNPROTO.H PALETTE.C; do
  if cmp -s "$H5DISP/$f" "$PREFIX/Displays/H5/$f"; then
    echo "PASS  sync $f"
  else
    echo "FAIL  sync $f (repo tree != build tree — the build would not contain the repo fix)"; fail=1
  fi
done

exit $fail
