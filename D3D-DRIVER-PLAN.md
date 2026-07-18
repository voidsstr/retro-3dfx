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
