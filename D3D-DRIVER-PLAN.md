# Plan: a real 3dfx Direct3D driver for 3DMark (and D3D games) on the Voodoo3

**Goal.** Run 3DMark (and D3D titles) on **our own** H5-built Direct3D driver on
.124 (Voodoo3, XP), instead of the retail in-box `3dfxvs.dll` — so the D3D path is
ours to validate and optimize, the same way the OpenGL ICD (MesaFX/`retrogl`) and
Glide (`glide3x`) are.

## Key architecture fact
On Windows 2000/XP there is **no standalone D3D DLL**. The Direct3D HAL is compiled
**into the display driver** `3dfxvs.dll`
(`H5/W2K/Src/Video/Displays/H5/`): `D7D3D.C` (DX7 HAL, 48 KB), `D3CONTXT.C`,
`D3INIT.C`, `D3TXTR.C`, `D3RSTATE.C`, `D3FOG.C`, `D6*.C`/`D7*.C` (DX6/DX7 FVF +
primitive paths), `SOATNL.C`/`SOALIGHT.C`/`TLCLIP.C` (the SoA transform/light/clip
pipeline), plus 3DNow/MMX asm (`AMESH2.ASM`, `AFAN.ASM`, `tlknimath.asm`). So
"build a D3D driver" == build our H5 XP **display driver** and make Windows load it.
That is kernel-adjacent (its miniport `3dfxvsm.sys` is a `.sys`), hence higher risk
than the user-mode ICD/Glide work.

## Status: the driver is ALREADY BUILT
The `.143`-lane toolchain already produced it (does not need rebuilding to start):
- `toolchain-3dfx/dist/3dfx-napalm-xp-20260716/` → `3dfxvs.dll` (display+D3D HAL),
  `3dfxv5d.dll` (WFP-safe renamed copy), `3dfxvsm.sys` (miniport), `glide3x.dll`,
  `fxoem2x.dll`, trimmed `voodoo3.inf`/`voodoo5.inf`, packaged by
  `toolchain-3dfx/package_driver.sh`.
- Build tree objfre outputs also present under
  `…/Displays/H5/objfre/i386/dll/3dfxvs.dll` and `…/Miniport/H5/objfre/i386/…`.

So Phase 2 (build) is mostly **done**; the work is verify-for-Voodoo3 → safe deploy
→ test 3DMark → optimize.

## Phased plan

### Phase 0 — Baseline on the RETAIL D3D driver (do first, zero risk)
Run 3DMark 99 Max (`C:\Program Files\3DMark 99 Max\3dmark.exe`, D3D) on the current
retail in-box `3dfxvs.dll` (date 8-10-2000). Confirm it completes with no crash and
capture the score. This proves 3DMark + the box's D3D work and gives the number our
driver must match. (3DMark is Direct3D-only — it does **not** exercise our OpenGL
ICD; it validates the display driver's D3D HAL.)

### Phase 1 — Verify our built driver targets the Voodoo3
- The dist is named `napalm-xp` (Voodoo4/5). Confirm the built `3dfxvs.dll` +
  miniport actually enumerate/support **Voodoo3** (PCI `VEN_121A&DEV_0005`): check
  `voodoo3.inf` HWIDs (package_driver already injects .124's
  `…&SUBSYS_1037121A&REV_01`), and that the miniport `h3*`/H5 modeset covers
  Avenger. If the H5 W2K source is Napalm-only for some HAL paths, identify the V3
  gaps before deploying.
- Diff our `3dfxvs.dll` D3D HAL vs retail behaviourally on a low-risk surface first.

### Phase 2 — Build (mostly done; re-run only if Phase 1 needs V3 fixes)
- Reuse the existing Wine + W2K DDK toolchain that built the dist. Rebuild with
  `objfre` (retail, optimized) after any V3 source fix; re-`package_driver.sh`.
- Keep the OpenGL ICD registration **out** of the INF (package_driver already
  drops it — a registered-but-missing ICD breaks GL apps).

### Phase 3 — Safe deploy to .124 (HIGH RISK — recoverable install)
The miniport is kernel-mode: a bad one **BSODs at boot** → physical access to
recover. Mitigations (from the `.143` DEBUG-LOG "franken-stack" lessons):
- The **agent still runs on VGA fallback**, so a failed *display DLL* is
  remotely recoverable (flip `InstalledDisplayDrivers` back + reboot). The
  **miniport** is the real BSOD risk — deploy the display DLL first against the
  **proven** in-box miniport if possible, and only swap the miniport when forced.
- Match the pair: `HKLM\…\Video\{GUID}\0000 InstalledDisplayDrivers` must name our
  display driver; the active INF must not re-clobber it on PnP (the stock
  `3dfxvs2k.inf` rewrites it — use our trimmed INF).
- Take a registry backup of the current `InstalledDisplayDrivers` +
  `Services\3dfxvs ImagePath` first; script the rollback.
- **Do this with the user present** (physical reset available), like the `.143`
  display-driver deploys — not overnight-autonomous.
- WFP: use the renamed `3dfxv5d.dll` path so Windows File Protection doesn't
  restore the in-box driver.

### Phase 4 — Test 3DMark on OUR D3D driver
Re-run 3DMark 99 Max; confirm no crash, compare score to the Phase-0 baseline,
screenshot the result. Then run the D3D games in the rotation (and any DX6/7
title). Watch for: T&L/clip garbage (SoA pipeline), texture format issues
(`D3TXTR.C`), fog/alpha correctness, and mode-set/scanout (same class as the
OpenGL-side "capture clean / monitor garbled" postfilter issue).

### Phase 5 — Optimize the D3D HAL (mirrors the OpenGL ICD campaign)
Once stable, apply the same discipline as the MesaFX opt campaign: per-change
branch + A/B via 3DMark score + pixel screenshots. Targets in `D7D3D.C` /
`SOATNL.C` / the asm mesh paths (codegen `/G6`, `/QIfist`, vertex dedup, state
dedup) — same catalogue as `optimized/OPTIMIZATION-QUEUE.md`, which already scoped
these for the Glide/ICD side.

## Risks & coordination
- **Kernel/BSOD** — the miniport is the sharp edge; keep it the proven in-box one
  as long as possible, swap the display DLL first.
- **Lane** — the display driver is the `.143` lane's domain
  ([[3dfx-optimized-driver-143-overnight]]); coordinate — this reuses their
  toolchain + package_driver.sh and the same franken-stack lessons.
- **Provenance** — H5 display-driver source is leaked 3dfx proprietary → stays in
  the PRIVATE `retro-3dfx` repo; never the public `retro-agent` repo.
- **Voodoo3 vs Napalm** — the dist is napalm-xp; the single biggest unknown is
  whether the H5 W2K D3D HAL fully covers Avenger (V3). Phase 1 must settle this
  before any deploy.

## Immediate next step
Phase 0: run 3DMark 99 Max on the retail driver now (no risk), record the score +
"no crash", and that becomes the target for our driver. Everything past Phase 1 is
supervised (kernel risk), so it waits for a user-present session.

## Phase 0 RESULT (2026-07-18) — retail D3D driver CRASHES under 3DMark2000
Ran 3DMark2000 v1.1 (DX7) default benchmark on the **retail** `3dfxvs` display
driver, 1024×768×16. It ran the game/fill/poly scenes for ~5 minutes with no app
crash, then **the display driver TDR'd**: "Windows — Display Driver Stopped
Responding … The 3dfxvs display driver has stopped working normally … reboot".
The desktop dropped to VGA fallback (640×480×4) but **recovered via a mode-set**
(no reboot needed), and the **OpenGL/Glide path was unaffected** — Q3 ran at 58.6
fps immediately after. So: 3DMark2000 itself is fine on XP+Voodoo3; the **retail
D3D HAL is what's unstable under sustained load.** This is the concrete motivation
for Phases 1–5: our own H5-built D3D HAL (`D7D3D.C`) is ours to stabilize/optimize.
(3DMark99 separately can't even launch on XP — needs DX6.1.)

## Correction (2026-07-20): the deployed driver is OURS, and the TDR is OURS
Re-checked .124: `D:\WINDOWS\system32\3dfxvs.dll` = **595,180 bytes dated 07/17**
(our H5 build), miniport `3dfxvsm.sys` = our build "Running OK",
`InstalledDisplayDrivers = 3dfxvs`. So the deploy (Phase 3) is **already done** and
the 3DMark2000 TDR reproduced above is on **OUR** D3D HAL, not the retail one — the
earlier "retail" attribution was wrong. Phase 4 (test) is therefore the live task:
**make our D3D HAL survive 3DMark**.

Also important: the TDR is **soft-recoverable** — the OS + agent stay alive through
it; the box is NOT hard-down. The display just drops to VGA fallback (640×480×4) and
a **reboot** (soft/remote REBOOT works — no physical reset) restores full res. A
bare mode-set does NOT recover it (tried); it needs the reboot.

## Isolation RESULT (2026-07-20) — the TEXTURE RENDERING test wedges our HAL
Used 3DMark2000 → New Custom Benchmark → Select Tests (Custom tab exposes each
sub-test) to run feature tests in isolation (each renders at desktop res
1024×768×16, NOT fullscreen-exclusive, so mid-test GDI screenshots are readable).
Detection: a wedge blocks GDI/display calls (VIDEODIAG hangs) while the agent's
**PING still returns PONG** — i.e. the CPU/agent stay alive; only the display
driver spins. Per-group results:
- **Bump Mapping alone → PASS** (ran to the score screen).
- **Fill Rate (Single-Texturing) alone → PASS.**
- **Fill Rate (Multi-Texturing) alone → PASS.**
- **High Polygon Count (1/4/8 Light) alone → PASS.**
- **Texture Rendering Speed (8/16/32MB) alone → WEDGES.** Agent PING stays alive
  but VIDEODIAG hangs; `PROCKILL 3DMark2000.exe` unwinds the driver to VGA
  fallback (640×480×4). ⇒ **Texture Rendering is the culprit**, NOT Fill Rate. (My
  first call of "Fill Rate" mis-identified the combined-run frame — that rotating
  textured object is the texture-rendering scene, and Fill Rate merely runs earlier
  in the batch.)

Recovery note: because PING stays alive, a wedge is recoverable by `PROCKILL`-ing
the D3D app (drops to VGA fallback), then a soft **REBOOT** restores full res
(boot.ini default=XP, timeout=30 → unattended reboot is safe, no dual-boot-menu
stranding).

### Root-cause chain (from source)
Symptom = long hardware hang → display-driver TDR = a **CMD-FIFO wedge**. In
`CFIFO.C`, `H3MakeRoom()` stalls in `while (roomToReadPtr <= N) { curReadPtr =
GET2(fifo->readPtrL); … }` waiting for the chip's FIFO read pointer to advance. If
the **command processor has wedged** (stopped consuming the FIFO because it was fed
a command/register combo the **Avenger/V3** can't execute), that read pointer never
advances and the driver spins until the OS TDR watchdog resets it — exactly the 78s
hang.
- NOT the known SW bug: the file documents a *second*, relative-calc `H3MakeRoom`
  that infinite-loops on D3D over-requests, but that one is `#else` of
  `#ifdef USE_D3D_CODE`, and `SOURCES:75` defines `-DUSE_D3D_CODE` — so the **safe
  absolute-calc** version is compiled in. The spin here is on a genuinely dead chip,
  not the SW miscalc.
- Structural cause: we ship the **H5/Napalm** display driver
  (`-DNUMTEXTUREUNITS=2`, napalm-xp dist) on **Avenger** hardware. `IS_NAPALM` is a
  *runtime* device-ID check that adapts caps (texture size 256 vs 2048, extra blend
  modes, guardband, 32bpp) — but the Voodoo3-specific *compile-time* paths are
  gated `#if defined( H3 )` and are **absent from the H5 binary**. Something in the
  Fill Rate primitive/multitexture path emits a command valid on Napalm but not on
  Avenger.

### Next diagnostic step — narrow the texture SIZE
3DMark2000's "N MB Texture Rendering Speed" uploads an N-MB working set of textures
and renders. The Voodoo3 on .124 has **16 MB unified** memory, so the **16 MB and
32 MB** sets can't fit in texture memory → forces DDraw texture eviction/re-upload.
Hypothesis: the H5/Napalm HAL's texture memory management wedges the Avenger under
overcommit — either it reports too large a texture budget, mishandles a DDraw heap
eviction/re-upload, or issues a texture-download command the V3 can't complete
(chip stops draining CMD-FIFO → `H3MakeRoom` spins → GDI blocks → TDR).
- **Test 8 MB alone** (fits in 16 MB): if it PASSES, the bug is memory-overcommit
  on 16/32 MB → fix by capping V3 texture-memory reporting / graceful alloc-fail.
  If 8 MB also WEDGES, it's a deeper texture-download/heap-path bug independent of
  size.
- Code to inspect: `D3TXTR.C` — `TXTRMIPMAPALLOC` (heap alloc per TMU),
  `TextureSurfaceDelete`/eviction, `txtrCalcBaseAddress*`, and the texture-download
  path; plus how available texture memory is reported to DDraw for a non-Napalm
  (V3) board (the `if (! IS_NAPALM)` texture branches).

Then register/heap-level fix for V3, rebuild, kernel redeploy (BSOD risk, user
present), retest.

### Size narrowing DONE (2026-07-20) + source trace
- **8 MB Texture Rendering alone → PASS** (ran to score screen). **16 MB + 32 MB
  (combined with 8) → WEDGE.** ⇒ trigger is **texture-memory OVERCOMMIT**: the V3's
  ~8–10 MB free texture heap (16 MB VRAM − framebuffer/desktop) can't hold the
  16/32 MB working set, so DirectDraw thrash-evicts, and something in that path
  wedges the chip.

Source trace (W2K `Displays/H5`), ruling hypotheses in/out:
- `ReportNTDDrawHeaps` (DDINIT.C:1041): DDraw's texture pool is **LINEAR_HEAP0**
  (`VIDMEM_ISLINEAR`, no `DDSCAPS_TEXTURE` restriction); tiled heaps forbid
  textures. Its size = `ddLinearHeapSize`.
- `ddLinearHeapSize` (ENABLE.C:2608 / 2269) = `gdiDesktopStart − ulScreenOffset`,
  derived from **real** `TotalVRAM` — so it is correctly sized to actual V3 VRAM,
  NOT an over-report. (Ruled out: naive memory over-report.)
- CMD FIFO is at `CMDFIFO_START_OFFSET`=0x1000 and `ulScreenOffset` is placed
  *after* it (ENABLE.C:1244 `ulScreenOffset = fifoSize + CMDFIFO_START_OFFSET`;
  AGP-FIFO path puts the FIFO in AGP/system memory). Linear heap starts at
  `ulScreenOffset`, i.e. **above** the FIFO. (Ruled out: texture heap overlapping
  the FIFO.)
- `TextureSurfaceCreate` (D3TXTR.C:2245) returns `DDERR_OUTOFVIDEOMEMORY` cleanly
  when `TXTRMIPMAPALLOC` fails. (Ruled out: broken OOM return.)

**Leading hypothesis — oversized CMD-FIFO request infinite-loops H3MakeRoom.**
The *safe* (USE_D3D_CODE) `H3MakeRoom` stalls in `while (roomToReadPtr <= N)`, and
`roomToReadPtr` saturates at `fifoSize` (empty FIFO ⇒ `roomToReadPtr = fifoSize`).
If a **single request N > fifoSize**, the loop can never be satisfied → infinite
spin → 78 s hang → TDR — exactly our signature. Free build compiles
`ASSERTDD(N <= fifoSize)` to a no-op, so it hangs silently instead of asserting.
The 16/32 MB texture test's large host texture-download packets are the plausible
source of an oversized N (esp. if download batches scale with texture size). This
is consistent with CFIFO.C's own comment about "D3D code requests space for the max
number of entries… causes the sw to get into an infinite loop… system appears to
be hung." `fifoSize` on non-AGP is 512 KB (ENABLE.C:1214) / 1 MB.

**Other live hypotheses (need the debug log to disambiguate):** eviction/re-upload
race (texture heap slot reused / texbase reprogrammed while the chip is still
fetching → bad texel fetch address wedges the TMU); or a texture-download path that
programs a bad base after realloc.

### Definitive next step — instrument + capture the wedge op
The free build has D3DPRINT/DISPDBG compiled out; the exact wedging op is only
visible with logging. Build the **checked** (`objchk`) display driver (D3DPRINT on),
deploy via the **deploy-3dfx-driver** skill, reproduce 16 MB Texture Rendering, and
capture DbgPrint via **DebugView** (or WinDbg) on the XP box. The last log line
before the hang names the op → confirms which hypothesis. Alternatively, add a
guard in `H3MakeRoom` (`if (N >= fifoSize) split/flush`) as a speculative fix and
A/B it. Either way: rebuild → deploy (kernel, user present) → retest (reboot cycle).

### Reframe / scope reality
- Even the **RETAIL** in-box 3dfxvs TDR'd under 3DMark2000 (Phase 0 note) — this
  texture-overcommit-on-16 MB case is one 3dfx's own shipping driver failed too.
- **All real games pass** (period titles don't overcommit texture memory), and
  3DMark2000's Game 1/2, Fill Rate, High Polygon, Bump Mapping all pass on our HAL.
  Only the synthetic 16/32 MB texture-thrash feature test wedges.
- A "graceful fail/skip instead of wedge" outcome (no TDR) is a legitimate win even
  if a 16 MB card can never post a real number on the 32 MB test.

### Static analysis EXHAUSTED — ruled out & tooling reality (2026-07-20)
Ruled out via source: memory over-report, texture-heap/FIFO overlap, broken OOM
return, and oversized single CMD-FIFO request (`Blt32_SystemToVideo`, DDBLT32.C:1320,
chunks host texel data **per scanline** ≈128 dwords ≪ fifoSize — safe). Remaining
live causes (eviction/re-upload race; bad texbase after heap realloc) can't be
disambiguated statically — they need the **runtime debug log**.
- Free build (`objfre`, what ships): `DISPDBG`/`D3DPRINT` are no-ops
  (DDGLOBAL.H:173, D3GLOBAL.H:1991) → no log.
- To capture the wedging op: build the **checked** driver (`objchk` →
  D3DPRINT/DISPDBG route to `DebugPrint`/`DbgPrint`), set the debug level
  (DD_DebugLevel via env/registry), deploy via **deploy-3dfx-driver** skill, run
  DebugView on .124, reproduce 16 MB Texture Rendering, read the last line before
  the hang. NOTE: the DDK build has the known **38 GB-log hazard** — guard it.
- `package_driver.sh` currently packages only `objfre`; a checked build is a new
  toolchain step.

Decision fork recorded for the session: (A) checked-build + DebugView capture (the
definitive way to nail the exact op), (B) speculative eviction-sync fix
(idle/`H3_GP_WAIT` before texture-heap slot reuse) → build → kernel deploy → A/B
retest, or (C) accept current state (all games + 3DMark Game1/2/Fill/Poly/Bump pass;
only the 16/32 MB synthetic texture-thrash wedges, as retail 3dfx also did) and
pivot to the ICD quality push.

## ✅ FIXED (2026-07-20) — H3MakeRoom spin-breaker → 3DMark2000 completes (3196)
Built an **instrumented free display driver** (595828 B) with a CMD-FIFO
wedge-breaker in `CFIFO.C` `H3MakeRoom` (the safe USE_D3D_CODE version): count the
stall-loop iterations; if a single stall exceeds **20,000,000** iterations the
FIFO read-pointer is frozen (chip wedged) or the request can never fit — so dump
state via `EngDebugPrint` once (`retroDbg`) and **force-exit the stall**
(`roomToReadPtr = fifoSize; break`) to unpin the CPU. This turns the fatal hang →
TDR into a survivable, recoverable condition.

Deploy = display-DLL-only swap on .124 (move-aside `3dfxvs.dll` →
`3dfxvs_orig595180.dll`, copy instrumented in, reboot) — recoverable (bad display
DLL → VGA fallback → restore backup).

**Validation (all on the instrumented driver, box stayed fully responsive — VIDEODIAG
answered throughout, no TDR):**
- 16 MB Texture alone → **completes** (stock: part of the wedging set).
- 8+16+32 MB Texture (incl. 32 MB max-overcommit that reliably wedged stock) →
  **completes** (Texture Rendering 3/3 selected).
- **FULL suite (20/22: Game1 3/3, Game2 3/3, CPU, Fill 2/2, HighPoly 3/3, Texture
  4/4, Bump 4/4) → COMPLETES with `3196 3D marks`.** This is the exact default run
  that TDR'd our HAL ~4.5 min in at session start. Goal "iterate until 3DMark2000
  completes" = ACHIEVED.

**Honesty on the fix's nature:** the spin-breaker is a robust **safety net at the
CMD-FIFO-stall level** — it prevents the hang regardless of the underlying reason
the chip stops draining the FIFO. It confirms the wedge *manifests* as the
`H3MakeRoom` spin (hypothesis #1), and eliminates the hang/TDR so the benchmark
completes. It does NOT yet establish *why* the chip stalls under texture
overcommit (N>fifoSize oversized-request vs a genuine chip lockup) — that needs the
`retroDbg` `EngDebugPrint` values, which we could not capture because the current
Sysinternals DebugView requires Vista+ (XP is 5.1 → "not a valid Win32
application"). Next: an XP-compatible DebugView (≤ v4.81) to read N/fifoSize/
curReadPtr and decide whether a cleaner root fix (chunk oversized requests / sync
eviction) is warranted, plus visual-correctness spot-check of the texture scenes.

Source edits live in the prefix build copy; mirror to `3dfx Driver Code/` and
`package_driver.sh` + version bump + `deploy-3dfx-driver` for the official build.
(CFIFO.C fix mirrored to source tree 2026-07-20.)

## ⚠️ NUANCE (2026-07-20) — the spin-breaker may be DORMANT; clean rebuild suspected
Got an **XP-compatible DebugView** (v4.x, PE subsystem 5.0 — modern build is
Vista+ only, "not a valid Win32 application" on XP; fetched the ~2012 build from the
Wayback Machine, 468 KB). Enabled Capture Kernel; it works (captured a live
`watchdog!WdUpdateRecoveryState` kernel line). Reran 16 MB then 32 MB
(max-overcommit) texture tests on a fresh boot (wedge-latch `g_h3mrWedgeReported`
reset) — **both COMPLETED and my `retro3dfx H3MakeRoom WEDGE` print NEVER appeared.**

⇒ The `H3MakeRoom` 20M-iteration spin-breaker does **not** seem to be triggering —
so what actually fixed the wedge is likely the **clean rebuild** (`build -cZ`
recompiled all 75 files), NOT the spin-breaker. The previously-deployed 595180
build (2026-07-16) may have shipped with **stale build objects** (the toolchain
README's explicit hazard: shared copy-timestamps make nmake silently skip
compilation) → a subtly broken binary that wedged on texture overcommit; a clean
recompile of the current source produces a driver that doesn't wedge.

**Caveat — not yet positively confirmed:** the "no WEDGE print" is absence-of-
evidence. Need a **positive control**: add an unconditional `retroDbg` at a
guaranteed-hit point (e.g. first `H3MakeRoom` call — desktop 2D hits it at boot),
rebuild, reboot, and confirm "retro3dfx" appears in DebugView. If it appears →
capture path is trusted → spin-breaker truly dormant → clean rebuild is the fix
(and the root cause of the *old* build's wedge is the stale-object bug, worth
identifying which .obj). If it does NOT appear → display-driver `EngDebugPrint`
isn't captured this way and the absence proves nothing (need another logging path).
This positive control also validates the mechanism for the requested
**comprehensive driver logging**.

Either way the deliverable stands: the rebuilt 595828 driver makes **3DMark2000
complete (3196)**, and it's deployed + backed up on .124.

### Scope note
All actual GAMES run on the OpenGL ICD (`retrogl`/Glide), which is stable+benchmarked
— D3D only matters for 3DMark and D3D titles. Fixing this is deep kernel-driver work
(closed rasterizer, register-level) with BSOD-risk rebuild/redeploy + reboot-cycle
retest per iteration. Weigh against pivoting to the ICD quality push.

## ⚠️⚠️ MAJOR CORRECTION (2026-07-20 pm) — WFP confound: we were testing the RETAIL driver
After building instrumented drivers and never seeing our `EngDebugPrint` output, I
discovered **Windows File Protection (WFP) was silently reverting `3dfxvs.dll` to
the vintage retail driver** (689,216 B, dated 2001, held in
`system32\dllcache` + a catalog). Any copy of our build into
`system32\3dfxvs.dll` was restored to retail by WFP's real-time watcher within
~15 s. `SFCDisable=0xFFFFFF9D` + `SFCScan=0` do **NOT** disable WFP on retail XP
(only on checked builds / with a patched `sfc_os.dll`) — verified: our file was
re-reverted even with those set.

### Consequences (invalidate several earlier conclusions)
- Throughout the earlier session, the driver actually **loaded** as `3dfxvs` was
  frequently the **retail** binary, not ours — so "our D3D HAL wedges on texture
  overcommit" was very likely the **retail** driver wedging (or a WFP-managed
  load-path artifact), NOT our code. The "spin-breaker fixed it / 3196 completes"
  result cannot be cleanly attributed to our build either.
- `EngDebugPrint` from the display driver **never appeared in DebugView** despite
  kernel capture working for a kernel `DbgPrint` (the watchdog line). This is the
  known **free-build limitation**: display-driver `EngDebugPrint` is effectively a
  no-op on retail/free Windows. So the EngDebugPrint→DebugView logging path is
  **not viable** here; comprehensive logging needs a **checked display-driver build**
  (which also activates the 3dfx devs' extensive existing `DISPDBG`/`D3DPRINT`
  instrumentation across all files) or a file-based mechanism.

### Reliable deployment = the RENAME method (WFP-free), NOT file-swap
Deploy our display DLL under a name **not in any WFP catalog** and repoint the
loader:
- Copy our build → `D:\WINDOWS\system32\3dfxv3d.dll`
- `HKLM\SYSTEM\CurrentControlSet\Services\3dfxvs\Device0\InstalledDisplayDrivers`
  = `3dfxv3d` (REG_MULTI_SZ)
- Reboot → GDI loads `3dfxv3d.dll`; WFP ignores it (not catalogued). VERIFIED to
  stick across reboots. This is what the eventual installer should do (or
  catalog-sign our driver so WFP accepts `3dfxvs.dll`).

### Clean A/B via the rename path (WFP-free, both builds truly OURS):
- Our clean rebuild (596016) → **32 MB COMPLETES, responsive.**
- Original dist build **595180 → 32 MB COMPLETES, responsive.** (!)
⇒ **Neither of our binaries wedges when loaded cleanly.** The original 16/32 MB
wedge is **no longer reproducible** with our drivers on the rename path. Strong
evidence the wedge belonged to the WFP-served **retail** driver (or the
`3dfxvs`/catalog load path), and our own H5 D3D HAL is fine on texture overcommit.

### Net for the user's question "why did 16/32 MB fail"
Best-supported answer: **it wasn't our driver.** WFP kept swapping our deployed
`3dfxvs.dll` for the 2001 retail binary, and that retail HAL is what TDR'd under
sustained texture load. Once our own build is actually loaded (rename method),
3DMark2000's texture tests complete. Remaining open item: pin the exact retail-vs-
ours difference (would need a clean retail-via-rename 32 MB run + the checked-build
logging), and decide the production install strategy (rename vs catalog-sign vs
sfc_os patch).

## FINAL (2026-07-20) — nothing wedges via the rename path; wedge tied to std load path
Ran 32 MB via the WFP-free rename path on all three binaries:
- our clean rebuild (596016) → completes
- original dist 595180 → completes
- **retail 689216 → completes too**
⇒ **No binary wedges when loaded via the rename path.** The original, reproducible
16/32 MB wedge (session start) was tied to the **standard `3dfxvs` load path with
WFP active** — NOT to any specific driver binary. Likely the standard install's
class-key / memory-config, or WFP activity during that load, produced the unstable
setup; the rename path uses a cleaner/more-default config that doesn't wedge.
Exact mechanism unpinned (not reproducible now), but the practical result is solid:
**our driver deployed via the rename method completes 3DMark2000 including all
texture tests.** Box left on the clean dist build (595180) via `3dfxv3d.dll`.

### Logging status (comprehensive-logging task)
`EngDebugPrint` from the display driver is a no-op on this free/retail XP — never
captured by DebugView (kernel `DbgPrint` from other drivers WAS). So the free-build
`EngDebugPrint` approach for comprehensive logging does not work here. The viable
paths: (1) **checked (`objchk`) display-driver build** — activates the 3dfx devs'
extensive existing `DISPDBG`/`D3DPRINT` calls across ALL files (real comprehensive
logging), needs the DEBUG.H `RIP` de-fanged (no `EngDebugBreak`) + checked DDK libs
verified; (2) a file/registry-based log written from the miniport. Recommend (1).

## Comprehensive logging (2026-07-20 pm) — infra built; miniport IOCTL sink UNSOLVED
User asked to implement comprehensive driver logging. Findings + what was built:

### KEY: the driver already HAS two logging mechanisms, both currently dead on free XP
1. **`EngDebugPrint`** (used by V5DLog + the old DebugPrint) — VERIFIED a **no-op on
   free/retail XP**: never appears in DebugView even with kernel capture confirmed on
   and the Session-Manager `Debug Print Filter` set. (A kernel `DbgPrint` from
   `watchdog.sys` DID show, so DebugView itself works.) A prior session's `V5DLog`
   instrumentation across ENABLE.C (DrvEnableSurface/DisableSurface/AssertMode) was
   therefore silently invisible too.
2. **`H3PRINTF`/`h3printf` -> `ENABLE_LOG_FILE` file logger** — the driver's OWN
   built-in mechanism: display buffers text -> `EngDeviceIoControl(hDriver,
   IOCTL_3DFX_WRITE_LOG_FILE)` -> miniport `H3StartIO` case -> `ZwCreateFile`/
   `ZwWriteFile` to a .log file. Gated off by `ENABLE_LOG_FILE=0` (SOURCES `LF=0`).
   This is the RIGHT mechanism (a file the agent can DOWNLOAD; no DebugView).

### What was built (all in the prefix build copy + rebuilt binaries)
- `LF=1` in BOTH Displays/H5/SOURCES and Miniport/H5/SOURCES (enables H3PRINTF +
  the miniport `H3WriteLogFile` handler). Display DLL grew 595k->942k (logging in).
- Runtime gate `g_retroLogLevel` (LOGFILE.C) read once via `ddgetenv("Retro3dfxLog")`
  from `HKLM\...\Services\3dfxvs\Device0` (0/absent=off default, 1=on, >=2=verbose)
  so a fully-instrumented driver costs nothing at benchmark time unless enabled.
- `retroLogForce` (always-log+flush, for flight-recorder lines) and `retroLogRaw`
  (gated, for V5DLog) in LOGFILE.C; decls in LOGFILE.H.
- **V5DLog rerouted to the file logger** (DEBUG.C) via a global `g_retroLogPpdev`
  stashed at DrvEnableSurface — so all the existing rich V5DLog call sites (mode
  set / enable / assert / teardown) become file output. This is the bulk of the
  "comprehensive" coverage, for free.
- CFIFO.C flight-recorder rewritten to the file logger: FIRST-CALL positive
  control, STALL>=100K dump, WEDGE-BREAK@50M (the texture-wedge recorder).
- Miniport log path changed `C:` -> try `D:` (active NT system vol) then `C:`
  fallback (the box's C: is the Win98 FAT vol; kernel write to it silently failed).
- Miniport observability: WRITE_LOG_FILE IOCTL handler bumps
  `Retro3dfxLogIoctlCount` + `Retro3dfxLogLastStatus` via `VideoPortSetRegistry-
  Parameters` (IRQL-safe) so usermode can see if the IOCTL arrives + its status.

### BLOCKER (unsolved): the WRITE_LOG_FILE IOCTL never reaches the miniport
Verified the running miniport IS our rebuilt one (3dfxv3m.sys 199612, ImagePath +
driverquery confirm). Yet `Retro3dfxLogIoctlCount` NEVER increments — even from an
**unconditional** direct `EngDeviceIoControl(ppdev->hDriver,
IOCTL_3DFX_WRITE_LOG_FILE,...)` probe placed in DrvEnableSurface (guaranteed to run
every mode set), gate fully bypassed. So no C:/D:\3dfxvs.log is ever produced.
- The miniport `H3StartIO` switch has `case IOCTL_3DFX_WRITE_LOG_FILE` as a plain
  peer of the WORKING `IOCTL_3DFX_GET_AGP_FIFO_INFO` (no pre-filter, both
  FILE_DEVICE_VIDEO, both compiled in — strings confirm). So handling is identical
  to IOCTLs that work; the IOCTL is being dropped BEFORE the miniport, in the
  videoprt.sys forwarding layer, for this specific code/shape.
- Leading hypothesis: `IOCTL_3DFX_WRITE_LOG_FILE` = CTL_CODE(FILE_DEVICE_VIDEO,
  0xfd7, METHOD_BUFFERED, FILE_ANY_ACCESS) with a **NULL/zero OUTPUT buffer**;
  the working private IOCTLs all pass a non-zero output buffer. videoprt may drop a
  private video IOCTL that has no output buffer. NEXT: give WRITE_LOG_FILE a small
  non-zero output buffer on both sides and retest the counter. (Untested — stopped
  here after 8 rebuilds/~10 reboots to avoid more blind kernel iteration.)

### Status
Logging INFRASTRUCTURE is complete and correct end-to-end EXCEPT the videoprt IOCTL
forwarding for the log code. Once that one plumbing issue is solved (likely the
output-buffer hypothesis above, one more rebuild), the whole comprehensive log —
V5DLog mode/enable/teardown trace + CFIFO flight recorder + any H3PRINTF site —
lands in D:\3dfxvs.log, downloadable by the agent, gated by Retro3dfxLog. Box
restored to the clean non-logging 595180 driver via the rename path for benchmarks.

---

## CURRENT STATUS — V5 5500 D3D on .143 (2026-07-21, updated)

The comprehensive-logging BLOCKER above (WRITE_LOG_FILE IOCTL never reaching the
miniport) was SOLVED by switching the log sink to the **registry ring**
(LOGFILE.C: SetRegSZ → RLog00..31 + RLogSeq under the miniport Device0 key,
UTF-16LE, survives reboots). The IOCTL file-log path is left in as a harmless
no-op. The full flight recorder now lands in the registry and is read by
`/tmp/ring_read.py`.

**D3D on the V5 5500 now works** (single-chip AND 2-way SLI), verified end-to-end:

- **Black/garbage textures — FIXED** (commit 08fd889). Root cause: W2K
  D3TXTR.C rev 40 (3dfx's last change, 10/25/00) deleted the per-LOD
  board-offset line in TEXTURELOAD's mipmapped path; every mip level
  downloaded to a stale offset → the TMU sampled unwritten memory. One line
  restored. This was the dominant D3D defect and ALSO the cause of the apparent
  "2-way SLI banding" (garbage texture memory scanned out through the SLI
  band-interleave looked like alternating bands).
- **2-way SLI D3D — WORKS.** 3DMark2001 Car Chase renders fully textured with
  no banding on active SLI (ring: PROMOTE-SLIAA sliEn=1 chips=2 nlines=16).
- **Hard-freeze vectors — CLOSED.** All four unbounded accelerator busy-spins
  (H3MakeRoom, DdFlip pending-swap, FXBUSYWAIT, H3_GP_WAIT) are bounded with
  ring-logged wedge-breakers.
- **"Warm-rerun instability" — NOT a driver bug.** 3DMark2001 corrupts its own
  D3D state when re-benchmarked within one process; 4/4 cold-process runs pass.
- **First correct D3D scores:** single-chip 1601 @640×480×16; 2-way SLI pending.

**Regression-locked:** `retro-3dfx/tests/` (predeploy gate + on-target D3D
matrix via d3dlab). Deployed driver: 3dfxv5d.dll instr7 (~957 KB).

**Remaining D3D work (lower priority, driver is stable):** bisect up to
compressed textures + 32-bit color (single-chip); real D3D game validation;
feed the verified 2-way SLI path into V5 6000 4-way (branch v56k-6000).
