# VINTAGE-FIXES — catalog of every fix made to the 3dfx H5/Napalm driver source

This repo builds the leaked 3dfx H5 ("Napalm" / VSA-100) Windows-2000/XP driver
stack and runs it on real hardware (Voodoo5 5500 AGP, box .143). This file is
the definitive ledger of every change made to that vintage codebase: which bugs
were **present in 3dfx's original source** (found and fixed by this project,
some latent for 26 years), which changes are robustness hardening, which are
policy/behavior choices, and which fix this project's own additions.
**Section 2 is the open half of the ledger** — vintage defects that are
identified and evidenced but not yet fixed, so a candidate is never re-proposed
from scratch and a falsified one is never re-chased.

Every fix is locked by a regression test — `tests/test_source_invariants.sh`
(source-level assertions), `tests/native/` (pure-logic tests), and the on-box
gates (`tests/predeploy.sh` before deploy, `tests/run_target_tests.py` +
GL golden after). Policy: a fix without a regression test is not done
(see `optimized/README.md`, `tests/README.md`).

Hardware verification below means: verified on the Voodoo5 5500, XP SP3,
box .143, with the registry-ring flight recorder confirming driver-side state.
Section 2's open items were measured on the Voodoo5 6000 recreation, box .191,
where the display driver is off the path entirely and the sensors are per-chip
registers plus an eye on the monitor (see that section's preamble).

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

## 2. Vintage defects identified but NOT yet fixed (open)

Same standard of evidence as section 1 — each is traced to 3dfx's own source or
measured against a control on the same hardware — but **no fix is written, so
these carry no commit and no regression test yet**. Each entry ends with the
experiment that would close it.

Hardware for this section is **box `.191`**, the Voodoo5 6000 "Strange God"
recreation: 4× VSA-100 @166 MHz behind a HiNT HB1-SE66 bridge, 128 MB VBIOS
mode (32 MB/chip), AGP, Athlon 1152 MHz / nForce2, XP SP3. The control is
**AmigaMerlin 3.1-R11** — a third-party *retail-lineage* driver (a different
driver lineage from this repo's), installed alongside our stack on the same box
under its own file names (`3dfxvs.dll` / `3dfxvsm.sys` vs our `3dfxv5d.dll` /
`3dfxv5m.sys`); rollback set at `C:\RETRO_AGENT\am-rollback\`.

### 2.1 4-way SLI band skew — PRIME SUSPECT: `vidProcCfg` BIT(28)|BIT(29)

**Status: NOT YET PROVEN.** The defect is ours (a working control exists on the
same board); the named cause is a candidate, not a verdict.

- **Symptom:** in any multi-chip configuration the 6000 renders *correctly into
  memory* and **scans out** wrong — horizontal strips displaced sideways.
  Single-chip is clean. No software can see it: reads through the master's BAR1
  are SLI-gathered in hardware, and `tools/v56k/sligrid.c` read back its own
  static pattern from the front buffer as `0 of 307200 pixels differ (0.0000%)`
  while the monitor was visibly skewed. Only an eye on the monitor and per-chip
  register state are sensors here.
- **The defect is OURS, VERIFIED ON HARDWARE:** AmigaMerlin 3.1-R11, same
  board, same 4-way SLI config, renders with **no skew** (confirmed by the
  operator on the monitor). The board, the HiNT bridge and the analog combine
  are therefore all fine — stop looking for a hardware excuse.
  **This is NOT an A/B.** "Bits clear" has only ever been observed under a
  *different driver*, which moved `pllCtrl0` (the pixel clock / refresh) at the
  same time — a **two-variable comparison**. One board, yes; one variable, no.
  It localizes the defect to software; it does not name the register.
- **The register diff.** Captured live with `fxscan2 dump` in both stacks, all
  four chips. `vidScreenSize`, `vidDesktopStride`, `vidDesktopStart`,
  `vidOvlEndCoord`, `lfbMemoryConfig`, `vidOverlayDudx` and
  `vidOvlDudxOffSrcW` are identical between the stacks; exactly two registers
  differ:

  | register | AmigaMerlin (CORRECT) | ours (SKEWED) |
  |---|---|---|
  | `vidProcCfg` | `03E60101` | `33E60101` |
  | `pllCtrl0` | `0000D137` | `0000B31F` |

  `vidProcCfg` differs by exactly `0x30000000` = BIT(28) | BIT(29), set in ours
  and clear in AmigaMerlin.
- **Root cause candidate (VINTAGE):** `Miniport/H5/h3modeset.c:667-672` sets
  both bits unconditionally on every Napalm board —

  ```c
  // Set Bit 28 and 29 on Napalm boards
  // This fixes a problem we were seeing with high-res modes in a heated environment.
  if ((IS_NAPALM) /*&& (66 == HwDeviceExtension->PciSpeed)*/)
    temp |= (BIT(28) | BIT(29));
  else
    temp &= ~(BIT(28) | BIT(29));
  ```

  By 3dfx's own comment this is an **erratum workaround for high-resolution
  modes in a hot chassis** — almost certainly a scanout prefetch or fetch
  threshold tweak, which is precisely the class of change that shifts *when* a
  chip latches its scanout data. Note the **commented-out `66 == PciSpeed`
  gate**: the workaround was once conditional on a 66 MHz PCI bus and someone at
  3dfx widened it to every Napalm board. BIT(29) is `SST_OVERLAY_EACH_VSYNC`
  (`Displays/H5/H3DEFS.H:1222`); **no copy of `H3DEFS.H` in this
  tree names BIT(28)** — a consistent gap between `SST_CURSOR_EN`
  (BIT(27), line 1221) and BIT(29). An undocumented bit.
- **Why this is NOT YET PROVEN — two live objections, both unresolved:**
  1. **The comparison changed two things at once.** `pllCtrl0` differs as well,
     and decoding it with the VSA-100 PLL formula
     `f = 14.31818 * (N+2) / ((M+2) * 2^K)` (`N = bits[15:8]`, `M = bits[7:2]`,
     `K = bits[1:0]`) says what it was: AmigaMerlin `0000D137` = 209/13/3 =
     **25.176 MHz** (VESA 640×480 = **60 Hz**); ours `0000B31F` = 179/7/3 =
     **35.994 MHz** (**85 Hz**). Agreement to three decimals, so this is
     certain, not inferred — and refresh rate is itself a scanout-timing
     variable: at 85 Hz each chip has ~30% less time per pixel, hence less
     margin for inter-chip skew.
     **Where the two clocks came from — the 60-vs-85 gap is OURS, not the
     drivers'.** The benchmark runs of §2.2 pin refresh on both stacks
     (`tools/v56k/trials/ambench.py` sets `FX_GLIDE_REFRESH=60` **and**
     `+set r_displayRefresh 60`), but the register dumps above were **not**
     taken under it: they came from the trial scripts (`trial_setup.py`,
     `ringrun.py`, `ringtrial.py`), **none of which sets any refresh**, so those
     runs took the ICD's own per-resolution default of 85 Hz. The AmigaMerlin
     dump, by contrast, was captured while `FX_GLIDE_REFRESH` had been forced to
     60 to stop the monitor going out of range. So the 60-vs-85 difference is a
     **configuration difference this session introduced between the two
     captures, not an intrinsic property of either driver** — which makes it
     controllable, and means **any future capture must pin refresh explicitly or
     the comparison is worthless**.
  2. **The 5500 sets the same bits and is clean.** `IS_NAPALM` is
     `(0x06 <= HwDeviceExtension->PCIDeviceID)` (`Miniport/H5/H3.H:320`), and
     both the V5 5500 and the 6000 are `DEV_0009` — so box `.143` takes this
     exact branch and its 2-way SLI has never skewed. **The bits are therefore
     not sufficient on their own.** A scanout-timing perturbation that 2 chips
     absorb and 4 (two of them behind a HiNT bridge) do not is consistent with
     both observations — but consistency is not proof.
- **Deciding experiment — a 2×2, no miniport rebuild required** (which matters:
  the Wine build tree is currently unusable). Both bits are directly pokeable at
  IO offset `0x05C` through `HWCEXT_GET_SLAVE_REGS` (0x19), and the refresh is
  set from the ICD side:

  | | bits 28/29 SET | bits 28/29 CLEAR |
  |---|---|---|
  | **85 Hz** | known: **SKEWED** | ? |
  | **60 Hz** | ? | AmigaMerlin-equivalent — **NOT RUN on our driver** |

  Only the top-left cell has ever been run on our stack. The bottom-right is
  where AmigaMerlin sat, under its own driver — it is *not* a clean result for
  our code and must never be recorded as one. A clean "60 Hz + bits set" cell
  acquits the bits and convicts refresh; a clean "85 Hz + bits clear" cell
  convicts the bits.
- **If the bits are convicted, the fix is 3dfx's own:** restore the
  `66 == PciSpeed` gate they commented out, or condition the workaround on chip
  count so a 4-chip board does not take it.
- **Still-live lever, not yet the answer:** `cfgSliAaMisc[8:0]`
  (`vga_vsync_offset` = pixels[2:0] | chars[5:3] | hxtra[8:6]) is the **only
  inter-chip horizontal alignment knob in the whole stack**. Hardware reads
  `0x800` (0 px) on the master and `0x827` (39 px) on all three slaves; zeroing
  the slaves **visibly moved the bands** (still skewed). `SLIAA.C:2489-2494`'s
  own comment — desired `pixels=7, chars=3`, bumped to `chars=4` because "the
  `vga_crtc_fast` module has a bug in it" — is **CONFIRMED ON SILICON**:
  programming 31 px (`cfgSliAaMisc = 0x81F`) **hard-froze the board** (NIC dead,
  physical power cycle). 3dfx's bump is a genuine, necessary workaround.
  **Never program `pixels=7` with `chars=3`.** 7 / 15 / 23 px and the 39 px
  default all run.
- **FALSIFIED on hardware — do not re-propose** (each tested, not assumed):
  slaves never receiving `vidScreenSize` (all four chips agree in-game; the
  patch was written and reverted); slave Y-origin `miscInit0` divergence
  (forcing the master's `077C0000` onto the slaves **blanks the screen** — the
  divergence is by design); the Win9x 4-chip master `CFG_VIDPLL_SEL` branch
  (`MINIVDD/SLIAA.C:1690`) that W2K dropped (poking bit 11 on the master gives
  **no picture at all** — its `SYNC_CLK_IN`/`SYNC_CLK_FB` are grounded on this
  board; the patch is retained only as evidence); the "chip 2 free-runs at
  +148 ppm" clock theory (re-measured from a clean boot with the skew fully
  reproduced, chips 1-3 against chip 0: +0.000 / +0.093 / +0.047 ppm — all
  locked, and still skewed, so phase is not even a usable proxy metric here);
  the SLI row/column masks and the analog arm's *absent* hsync tristate (both
  match 3dfx's own `H5/DOCS/Video SLI AA Configs.xls`); band height 16 (**resets
  the board** — 8 is correct at 640×480, where 480/32 = 15 exactly);
  `vidOverlayDudx`, swept across eight configurations, all skewed.
- **See** `FINDINGS.md`, 2026-09-04 entries "the skew bits are NAMED IN 3dfx's
  OWN CODE: an undocumented heat erratum", "AmigaMerlin RUNS 4-WAY SLI
  CORRECTLY ON THIS BOARD. The bug is ours.", and "PROOF no software can see
  this bug, and the clock theory is DEAD".

### 2.2 21% slower than AmigaMerlin on a SINGLE chip — a second defect

**Status: MEASURED AND UNEXPLAINED.** Independent of 2.1; may be inherited from
3dfx rather than introduced here.

- **Measured, VERIFIED ON HARDWARE.** Box `.191`, Quake III 1.32c `demo four`,
  640×480×16, vsync off (`FX_GLIDE_SWAPINTERVAL=0` **and** `r_swapInterval 0`),
  Athlon 1152 / nForce2. Automated via `tools/v56k/trials/ambench.py`; the
  pass metric is "the timedemo completes and prints an fps line", which needs no
  eyes on the monitor.

  | configuration | AmigaMerlin 3.1-R11 | ours (H5 + SGL ICD 0.5.0) | gap |
  |---|--:|--:|--:|
  | single chip | 116.5 | 92.5 | **-21%** |
  | 4-way SLI | 151.5 | 105.0 | **-31%** |

  Both rows are the **paired** run — the two drivers measured back-to-back under
  the same harness, which is the only comparison the -31% may be quoted from.
  The separate eight-point resolution sweep measured AmigaMerlin's 4-way
  640×480 at **152.6**; the two agree to 0.7%, well inside the ~3% run-to-run
  noise, but they are different runs — **never mix the two figures silently**.

- **Why this is a separate bug from 2.1.** The single-chip row pins every
  variable except the driver — same box, card, game, demo, resolution, colour
  depth — and we are 21% down **before a second chip is involved**. It cannot
  be an SLI bug, a scanout bug, or a hardware excuse. The deficit being roughly
  the same size in both rows reads as **per-chip submission-path cost**, which
  points at the open FIFO-wedge / submission-pacing items
  (`V56K-SLI-FINDINGS.md` §21–22) rather than at anything new. (Caveat: if the
  scanout defect carries a throughput cost of its own, part of the 4-way half of
  the gap may be a symptom of 2.1. The single-chip 21% is clean.)
- **The gap is recoverable in software — it is not a host ceiling.**
  AnandTech's Oct-2000 Q3 640×480×16 CPU ladder (GeForce2 GTS: Athlon 800 = 128,
  900 = 137, 1.0 GHz = 144, 1.2 GHz = 162, "a 1% performance increase per MHz")
  interpolates to **~158 fps** at 1152 MHz. **AmigaMerlin's 4-way 640×480 is
  ~96% of that ceiling (152.6 on the sweep, 151.5 paired); ours at 105.0 is
  66%.** AmigaMerlin is at the wall and cannot go faster on this host, so the
  wall is not what is holding us.
- **VINTAGE or ours? — the one published datum on our own code lineage
  (HYPOTHESIS).** VoodooExtreme's Dec-2000 review of 3dfx's own V5-6000
  *reference driver* is the only published test of this code lineage anywhere.
  At 1024×768 it reports **88.4 fps at 16-bit vs 86.1 at 32-bit — a 1.03×
  gain** for halving the colour depth, and the reviewer calls it a driver bug
  ("performance in 16-bit color was a bit lackluster... most likely a driver
  issue"), noting the 5500 shows it too. Community drivers on the same silicon
  get a proper **1.20×** (VoodooAlert, 155.1 vs 128.7). So part of our deficit
  may be **inherited from 3dfx** rather than introduced here.
- **Deciding experiment — fully automated, fps only, no visual verdict:** run
  *our* stack at 640×480 in 16-bit and again in 32-bit.
  - ratio ≈ **1.0×** → we have inherited the documented 3dfx 16-bit defect,
    and the hunt has a named target inside the vintage colour-depth path.
  - ratio ≈ **1.20×** → the 16-bit path is healthy and the 21% is ours,
    living elsewhere — the submission path first.

  Prerequisite: our stack reinstalled on `.191` (AmigaMerlin currently holds the
  HWID; see the rollback set named at the top of this section).
- **Calibration to compare against, VERIFIED ON HARDWARE.** Under AmigaMerlin
  the same board is a textbook fill-rate line — single chip at
  19.9 / 19.8 / 19.7 ms per megapixel across the three intervals from 640×480 to
  1600×1200, agreeing to 1%, so one VSA-100 here is fill-bound at every
  resolution — and its 4-way scaling at 1024×768 is **2.66×**, matching
  x86-secret's measurement on the real prototype to the second decimal. Any Q3
  number at or below 1024×768 on this box is a CPU benchmark, not a card
  benchmark (`t = 6.41 ms + 0.50 ms/Mpx`, ceiling ~156 fps).
- **See** `FINDINGS.md`, 2026-09-04 entries "OUR DRIVER IS 21% SLOWER THAN
  AMIGAMERLIN ON ONE CHIP, AND THAT IS A SECOND BUG" and "the 6000 benchmarks
  EXACTLY on published data: 2.66x SLI at 1024x768".

## 3. Vintage robustness hardening — unbounded hardware waits

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

## 4. Policy / behavior changes (deliberate, not bug fixes)

- **Vsync ON by default at the driver level** (`FX_GLIDE_SWAPINTERVAL` via
  the INF) — per user request; games that need max fps override per-process
  (e.g. `optimized/gltest/cs_fast.bat` sets `FX_GLIDE_SWAPINTERVAL=0`).
  Commit `07424b8`.
- **3D gamma default 1.3** (registry), part of the gamma work.
- **P8 export disabled** — see 1.9.

## 5. Fixes to this project's own additions

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

## 6. Clean-room OpenGL ICD fixes (tracked in this repo's prefix tree)

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

## 7. Instrumentation added to the vintage driver (diagnostic, retained)

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

## 8. How this is kept true

`tests/test_source_invariants.sh` asserts the source patterns of every fix
above (both repo tree AND Wine build tree — they must not diverge);
`tests/predeploy.sh` refuses deployment if any invariant fails, the built
DLL is stale, or a fixed `.obj` predates its source; on-box
`tests/run_target_tests.py` replays the d3dlab D3D matrix against goldens and
checks the ring for new errors; the GL golden gate (Q3/CS) covers the shared
modeset/2D core. Every fix listed here passed all gates on hardware before
its commit.

Section 2 is deliberately outside that machinery: an open defect has no fix to
assert, so it is held true by evidence instead — a named source line, a measured
register or fps pair, and the falsified candidates recorded beside it so no pass
re-proposes one. An item leaves section 2 only by moving into section 1 with a
commit and a regression test, in the same change.
