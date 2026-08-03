# VINTAGE-FIXES — catalog of every fix made to the 3dfx H5/Napalm driver source

This repo builds the leaked 3dfx H5 ("Napalm" / VSA-100) Windows-2000/XP driver
stack and runs it on real hardware (Voodoo5 5500 AGP, box .143). This file is
the definitive ledger of every change made to that vintage codebase: which bugs
were **present in 3dfx's original source** (found and fixed by this project,
some latent for 26 years), which changes are robustness hardening, which are
policy/behavior choices, and which fix this project's own additions.

Every fix is locked by a regression test — `tests/test_source_invariants.sh`
(source-level assertions), `tests/native/` (pure-logic tests), and the on-box
gates (`tests/predeploy.sh` before deploy, `tests/run_target_tests.py` +
GL golden after). Policy: a fix without a regression test is not done
(see `optimized/README.md`, `tests/README.md`).

Hardware verification below means: verified on the Voodoo5 5500, XP SP3,
box .143, with the registry-ring flight recorder confirming driver-side state.

---

## 1. Vintage bugs — found in 3dfx's original code, fixed here

These defects shipped in (or were introduced by the final revisions of) 3dfx's
own source. None of them were fixed by 3dfx before the company folded in
December 2000; each was root-caused and fixed by this project.

### 1.1 Mipmapped D3D textures download black/garbage
- **Symptom:** every mipmapped Direct3D texture rendered black or garbage
  (the "black dragon" in 3DMark2001); also manifested as apparent "2-way SLI
  banding" (garbage texture memory scanned through the SLI band interleave).
- **Root cause (VINTAGE):** `D3TXTR.C` **rev 40 — 3dfx's last-ever change to
  the file (10/25/00, "no longer use surface local pointers")** — deleted the
  per-LOD board-address recomputation in `TEXTURELOAD`'s mipmapped path, so
  every LOD downloaded to a stale address. Found by diffing the working Win9x
  tree (`Win9x/DX/D3D/D3TXTR.C` rev 35) against the W2K tree.
- **Fix:** restore the per-LOD address line
  (`addr = psurfDst->mmData[nDstLOD].fpVidMem - _FX(textureHeapStart[tmuCnt]);`).
- **Commit:** `08fd889` · **Files:** `Displays/H5/D3TXTR.C`
- **Verified:** d3dlab 12-mode matrix golden; first correctly-rendered
  3DMark2001 run (1601, later 1633); SLI "banding" gone (2-way SLI 1544 marks,
  correct across all scenes).
- **Regression:** source invariant #1; native logic test
  (`tests/native`, `ed5bfcf`); d3dlab `big512mip`/`mipfar` goldens.

### 1.2 UT2004 (any DXTn app) bugcheck 0x8E — TEXBLT FourCC arity mismatch
- **Symptom:** kernel AV 0xC0000005 / bugcheck 0x1000008E in
  `Blt32_CopyFourCC` on the first compressed-texture upload (UT2004).
- **Root cause (VINTAGE):** `D6DP2.C`'s `D3DDP2OP_TEXBLT` handler cast the
  **5-argument** `__stdcall Blt32_CopyFourCC` to the **7-argument**
  `PTEXBLTFUNC` — `nSrcLOD` (int 0) landed in the `pDDDstSurf` pointer slot
  and was dereferenced. Any DX7 app uploading DXTn/FXT1 via TEXBLT crashed.
- **Fix:** new 7-argument adapter `Blt32_TexBltCopyFourCC` in `DDBLT32.C`
  (rebuilds surface views from `TXTRHNDL` `mmData`, calls with correct arity
  and full-LOD rects); `D6DP2.C` dispatches through it.
- **Commit:** `7ddda02` · **Files:** `Displays/H5/DDBLT32.C`, `D6DP2.C`
- **Verified:** UT2004 launched repeatedly with zero minidumps; d3dlab
  `dxt1up` mode (drives this exact path) renders the correct checker.
- **Regression:** source invariants 5c (adapter defined, D6DP2 uses it, raw
  cast banned); d3dlab `dxt1`/`dxt1up` goldens.

### 1.3 GoldSrc Direct3D bugcheck 0x8E — NULL render context in texture download
- **Symptom:** selecting Direct3D in Counter-Strike 1.6 / Half-Life
  (`EngineD3D=1`) bugchecked the OS on launch
  (0x1000008E, fault `DdBlt+0x32C`).
- **Root cause (VINTAGE):** `DdBlt`'s system→video texture-download path
  resolved texture handles via `TXTRHNDL_PTR(h)` =
  `pRc->pHndlList->ppTxtrHndlList[h]` with `pRc = _D3(lastContext)`.
  `lastContext` is set **only** inside the DrawPrimitives2 draw path and is
  **zeroed by `textureLoad()` itself**, so any app that uploads textures
  before its first draw (GoldSrc's exact behavior) ran this path with
  `pRc == NULL` → `[NULL+0x510]` kernel AV. Proven by minidump + disassembly
  of the deployed binary.
- **Fix:** resolve handles context-independently (see 1.4 for the final
  resolution mechanism).
- **Commit:** `8de09a3` · **Files:** `Displays/H5/DDBLT32.C`
- **Verified:** CS launches and runs in Direct3D with zero bugchecks
  (previously 100% crash); full d3dlab matrix unchanged.
- **Regression:** source invariants 5f.

### 1.4 GoldSrc Direct3D white world — mip-sublevel downloads silently dropped
- **Symptom:** with the crash fixed, the world rendered flat white while sky,
  HUD, menus, and lightmap shading were correct.
- **Root cause (VINTAGE):** GoldSrc blts **every mip level as a separate
  DirectDraw surface**. Only the chain **root** ever gets a `TXTRHNDL`
  (`ddiCreateSurfaceEx`); sublevel surfaces carry their own never-registered
  `dwSurfaceHandle`, so the download path could not resolve them and silently
  dropped mips 1..n of every world texture — the TMU then minified into
  stale-white video memory. (Sky/HUD/lightmaps are mip-less → unaffected.
  Proven with ring instrumentation: `TEXDL-SKIP` lines, then `lod=1/1..3/3`
  downloads after the fix.) Uploads issued during GoldSrc's repeated video
  restarts (no D3D context alive) were dropped the same way.
- **Fix:** in `DdBlt`'s download path — resolve through the **global**
  per-DDLcl handle-list chain `g_pHndlList` (what `GetHndlListPtr` iterates;
  exists independent of any render context — the vintage comment at its
  definition even anticipates this need); for sublevel surfaces walk **up**
  the `lpAttachListFrom` chain to the texture root and recover the LOD index
  by matching the blitted surface's dimensions against the root `TXTRHNDL`'s
  `mmData[]` (the driver's own TEXBLT mip-match idiom); call `TEXTURELOAD`
  with the real `(nSrcLvl, nDstLvl)` instead of hardcoded `(0, 0)`.
- **Commit:** `cf3ab3e` · **Files:** `Displays/H5/DDBLT32.C`
- **Verified:** de_dust fully textured (screenshot); ring shows sublevel
  downloads on 8-level chains with zero skips; fullscreen D3D stable at
  640×480 and 1024×768; **timedemo: D3D 33.5 fps @1024×768 vs OpenGL 30.6 —
  Direct3D is now the fastest GoldSrc renderer on this card**. Side effect:
  the engine's re-upload thrash (~19,700 uploads/40 s — it kept re-uploading
  textures whose mip validation failed) dropped below 8,192.
- **Regression:** source invariants 5g (walk-up, mmData match, real-LOD call).

### 1.5 `TEXTURELOAD` source-height log computed from the wrong LOD index
- **Root cause (VINTAGE, latent):** `D3TXTR.C` computed `tlog` from
  `psurfSrc->mmData[nDstLOD].wHeight` — a typo for `nSrcLOD`, benign only
  while every caller passed `(0, 0)`. Load-bearing the moment 1.4 made real
  LOD indices flow.
- **Fix/Commit:** corrected to `nSrcLOD` · `cf3ab3e` · **Regression:** 5h.

### 1.6 ALPHA_P8 texture format OR'd into the wrong TMU register (×2)
- **Root cause (VINTAGE, latent copy-paste):** `D6MT.C`
  `setupTextureStage0`/`setupTextureStage1` wrote the ALPHA_P8 format bits to
  the single-texture register (`sst.textureMode`) instead of the stage's TMU
  register (`textureModeT1`/`textureModeT0`) — the sibling P8_RGBA lines two
  lines above are correct, exposing the copy-paste. Latent (the A8P8 format
  is not currently exported), fixed for correctness.
- **Fix/Commit:** `cf3ab3e` · **Regression:** invariants 5i (both registers).

### 1.7 TEXBLT "no matching mip level" diagnostic was unreachable dead code
- **Root cause (VINTAGE):** in `D6DP2.C`'s TEXBLT mip-match loop the
  no-match `else` can never execute (its guard is the loop condition), so a
  blt with no matching source level downloaded **zero** levels and returned
  `D3D_OK` with no trace.
- **Fix:** the skip is kept (semantics unchanged) but now emits a bounded
  `TEXBLT-NOMATCH` ring line. · **Commit:** `cf3ab3e`

### 1.8 Recycled texture descriptors kept the prior texture's palette handle
- **Root cause (VINTAGE, latent):** `D3TXTR.H` `ALLOCTEXTUREDESC` re-used a
  freed `TXTRDESC` slot without clearing `dwPaletteHandle` — a new texture on
  that slot could sample through a stale palette.
- **Fix:** cleared on allocation with the other slot state. · **Commit:** `cf3ab3e`

### 1.9 P8 palettized-texture pipeline broken on Napalm (3dfx's own battle)
- **Root cause (VINTAGE, known-broken feature):** the P8 palette never
  reliably reaches the TMU on this path. 3dfx's own history records the
  fight: rev 24 "disable palettized texture", rev 25 "reenable", a
  "Palette failures in DCT 300" note (Microsoft's compatibility tests), a
  **permanently disabled A8P8 export** (`#if ... || TRUE`), and a ready-made
  `DISABLE_PAL8_ON_NAPALM` kill-switch left commented out in `D3INIT.C`.
- **Fix:** enabled 3dfx's own kill-switch — the driver stops exporting
  P8/A8P8 texture formats, so apps negotiate RGB formats, which render
  correctly. (P8 textures previously rendered white/garbage; not exporting a
  broken format is a strict improvement.)
- **Commit:** `cf3ab3e` · **Files:** `Displays/H5/D3INIT.C`

### 1.10 Desktop gamma washout — driver persisted app gamma ramps
- **Symptom:** after running (or crashing out of) Quake 3, the Windows
  desktop stayed washed out; the corrupt ramp survived reboots via the
  registry `GammaTable`.
- **Root cause (VINTAGE design flaw):** `PALETTE.C` treated every
  `DrvIcmSetDeviceGammaRamp` as the new persistent desktop gamma — a 3D
  app's overbright ramp (Q3's `r_overBrightBits`) became the desktop's and
  was persisted.
- **Fix:** app-set ramps route to a transient slot; the desktop ramp is
  re-asserted on every mode return (`vAssertModePalette`), and a degenerate
  persisted ramp self-heals at `bInitializePalette`.
- **Commit:** `07424b8` · **Files:** `Displays/H5/PALETTE.C`
- **Verified:** desktop stays correct across Q3 run/crash/quit cycles.
- **Regression:** source invariants (PALETTE gamma block).

## 2. Vintage robustness hardening — unbounded hardware waits

The vintage driver contained **six** unbounded busy-wait loops on accelerator
state. Any GPU/FIFO wedge turned into a hard OS freeze (NIC dead, physical
power cycle). All are now bounded (~50–100 M iterations) with a `WEDGE-BREAK`
ring line on trip, in `ENABLE_LOG_FILE` builds:

| Wait | File | Commit |
|---|---|---|
| `H3MakeRoom` command-FIFO stall | `CFIFO.C` | `6857906` |
| `DdFlip` pending-swap spin | `DDFLIP.C` | `a280f26` |
| `FXBUSYWAIT` accelerator-busy spin | `DDGLOBAL.H` | `fc8e313` |
| `H3_GP_WAIT` graphics-processor wait | `HW.H` | `fc8e313` |
| `DdLock`/`DdFlip` flip-status waits | `DDSURF.C`/`DDFLIP.C` | `baf3c2a` |
| 2D BitBlt GP spin (`RETRO_GP_SPIN`) | `HW.H`/`BITBLT.C` | `baf3c2a` |

Verified no-regression after each (D3D matrix + GL golden, `2b39149`).
Every accelerator wait in the display driver is now bounded.

**glide2x had its own pair** (found 2026-08-03 via UT99):

- **Symptom:** quitting UT99 (Glide) after a session left the screen
  garbled — `UnrealTournament.exe` pinned at 98% CPU forever, desktop
  never restored (looked like a crash, was actually a spin). Ring
  signature: WinClose escapes + `EXCL-LEAVE 3dCnt=0`, then silence — no
  `HWCRLSEXCLUSIVE`, no mode restore; UT log truncates right after
  `Unbound to Render.dll`.
- **Root cause (VINTAGE):** with the accelerator wedged (`SST_BUSY`
  stuck), `grSstWinClose`'s two user-mode spins never terminate in
  release builds: `grSstIdle`'s status busy-poll (`GSST.C`) and
  `_grCommandTransportMakeRoom`'s read-pointer stall (`FIFO.C` — its
  `checks > 1000` diagnostic is GDBG-only and just logs). So
  `hwcRestoreVideo` never ran and the desktop was never re-asserted.
- **Fix:** both loops bounded at ~4 M no-progress polls (seconds of
  continuous BUSY / frozen read pointer), then shutdown proceeds — same
  WEDGE-BREAK philosophy as the display-driver table above.
- **Commit:** `2b3e832` · **Files:** `H5/GLIDE/SRC/GSST.C`, `FIFO.C`
- **Verified:** UT99 launch → console `exit` → process gone in <3 s, ring
  shows the full close sequence (`EXCL-LEAVE` → `HWCRLSEXCLUSIVE` → mode
  restore 1024×768×32 → `UNMAP_MEMORY`), UT log runs to "Log file
  closed", target D3D matrix still green.
- **Regression:** source invariants #15 (markers + glide2 tree sync) +
  `tests/native/test_glide2_shutdown_spins.c` (healthy/wedged/glacial
  simulated hw). Glide2 build recipe reminder: overlay
  `GLIDE/SRC/{GLIDE,GLIDESYS,GLIDEUTL}.H` into `H5/INCLUDE`, build
  `build_glide2.bat`, restore the Glide3 headers.

## 3. Policy / behavior changes (deliberate, not bug fixes)

- **Vsync ON by default at the driver level** (`FX_GLIDE_SWAPINTERVAL` via
  the INF) — per user request; games that need max fps override per-process
  (e.g. `optimized/gltest/cs_fast.bat` sets `FX_GLIDE_SWAPINTERVAL=0`).
  Commit `07424b8`.
- **3D gamma default 1.3** (registry), part of the gamma work.
- **P8 export disabled** — see 1.9.

## 4. Fixes to this project's own additions

Honest ledger — these bugs were introduced by this project, then found and
fixed the same way:

- **Q3 1024→1280 mode-change bugcheck 0x8E in `DrvBitBlt`** — the
  `ENABLE_LOG_FILE` diagnostic block added by this project dereferenced
  `psoSrc` unconditionally; `psoSrc` is NULL for solid/pattern blts and the
  first solid fill after a mode change faulted. Fixed with NULL guards in
  `BITBLT.C` (`a03a9fe`). Regression: invariants 5d.
- **`g_pContexts`-walk interim fix for 1.3** dropped uploads issued while no
  context existed (GoldSrc's video restarts) — superseded by the
  `g_pHndlList` resolution in `cf3ab3e` (see 1.4).

## 5. Clean-room OpenGL ICD fixes (tracked in this repo's prefix tree)

The OpenGL ICD source of truth lives at
`toolchain-3dfx/prefix/drive_c/3dfx/SWLIBS/OPENGL/GLIDE3X/` (git-tracked):

- **ICD 0.3.8** (`f92d34f`): CS 1.6 green/rainbow world — dual-texture path
  never wrote vertex colors.
- **ICD 0.3.9** (`a03a9fe`): `maxFrame` instrumentation + GoldSrc timedemo
  automation (disproved the texture-streaming-stutter theory).
- **ICD 0.4.0** (`3737586`): Q3 black-screen wedge at 1280×1024 — the ICD's
  resolution table capped below 1280×1024; added `SST_1280x1024` +
  per-resolution highest-safe refresh (85 Hz ≤1024×768, 75 Hz @1280×1024).

(The full 0.1–0.3.x MesaFX/Glide debugging history lives in
`optimized/README.md` and the changelog/debug-log documents it indexes.)

## 6. Instrumentation added to the vintage driver (diagnostic, retained)

- **Registry-ring flight recorder** (`LOGFILE.C` — vintage file, extended;
  ring sink `6857906`): `RLog00..RLog31` + `RLogSeq` under the Device0 key;
  survives reboots and hard freezes; reader pattern `/tmp/ring_read.py`.
- Lifecycle (`DrvEnableSurface`/`AssertMode`, `PRIMARY-TILED/LINEAR`),
  DP2 error localization (`DP2-PARSE-ERR`/`DP2-EXIT-ERR`, failing op + offset),
  SLI/AA (`COMPUTE/PROMOTE/DEMOTE-SLIAA`), memory (`ALLOC-FAIL`, kernel-pool +
  video-surface alloc/free balance, `19d0e26`), device-init abort shims
  (`CreateSurface`/`CreateSurfaceEx`, `faecd8f`/`28bd4e0`), context balance
  (`CTX-CREATE/DESTROY`), texture-download tracing (`TEXDL`/`TEXDL-SKIP` with
  LOD indices) and D3D bind-path tracing (`MT1`/`MT2` gate bits,
  `TSS-BADTEX`) from the GoldSrc-D3D investigation (`cf3ab3e`).

## 7. How this is kept true

`tests/test_source_invariants.sh` asserts the source patterns of every fix
above (both repo tree AND Wine build tree — they must not diverge);
`tests/predeploy.sh` refuses deployment if any invariant fails, the built
DLL is stale, or a fixed `.obj` predates its source; on-box
`tests/run_target_tests.py` replays the d3dlab D3D matrix against goldens and
checks the ring for new errors; the GL golden gate (Q3/CS) covers the shared
modeset/2D core. Every fix listed here passed all gates on hardware before
its commit.
