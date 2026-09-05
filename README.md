# retro-3dfx

Building — and **fixing** — the original 3dfx Voodoo 3/4/5 drivers for
Windows XP from the leaked H5/Napalm source tree, on Linux, under Wine, and
running them on real hardware. This repo turned the abandoned December-2000
3dfx driver source into a stable daily-driver stack for a Voodoo5 5500 — and
now drives a 4x VSA-100 Voodoo5 6000 as well (see
[Voodoo 5 6000](#voodoo-5-6000)) — with a regression suite that locks in every
hardware-verified fix.

**This is the vintage-source stack.** The clean-room stack (MesaFX ICD /
open Glide) lives in `retro-agent` (`voodoo-cleanroom/`, `scripts/3dfx/`) —
don't mix them up; how far that stack gets on real VSA-100 silicon (user-mode
only, and the failing layer is its Glide, not its ICD) is measured in
[`OPEN-STACK-ON-VSA100.md`](OPEN-STACK-ON-VSA100.md). The full working map
(deployed files ↔ source locations, build commands, deploy flow, test order,
instrumentation, box facts) is in [`CLAUDE.md`](CLAUDE.md); read it before
touching driver code.

## Stack at a glance

| Deployed on XP box | What it is | Source | Build output |
|---|---|---|---|
| `3dfxv5d.dll` | XPDM display driver: 2D/GDI, DirectDraw, **Direct3D 6/7/8 HAL**, Glide escape | `3dfx Driver Code/H5/W2K/Src/Video/Displays/H5/` | `objfre/i386/3dfxvs.dll` (renamed; WFP-safe name) |
| `3dfxv5m.sys` | video miniport: PCI/chip init, modes, SLI/AA, V5 6000 external clock | `.../Miniport/H5/` | `objfre/i386/3dfxvsm.sys` (renamed) |
| `glide2x.dll` / `glide3x.dll` | Glide 2.x / 3.x runtimes (games talk straight to these) | `H5/GLIDE2`, `H5/GLIDE3`, `MINIHWC` | per Glide build recipes |
| game-local `opengl32.dll` / `3dfxgl.dll` | OpenGL ICD built from the vintage `SWLIBS/OPENGL` tree ("3Dfx [retro3dfx x.y]") | `toolchain-3dfx/prefix/drive_c/3dfx/SWLIBS/OPENGL/GLIDE3X/` | `release/opengl.dll` |
| `d3dlab.exe` | our windowed D3D8 regression lab (not vintage) | `tests/d3dlab/` | MinGW |

Builds run under Wine (`toolchain-3dfx/`, VC6 SP5 + MASM 6.15 + W2K DDK) and
compile the **`toolchain-3dfx/prefix/drive_c/3dfx/` build tree** — repo-tree
edits must be copied there first (the test suite enforces sync).
`3dfx Driver Code/H5/Win9x/` is the working-reference Win9x driver used for
differential debugging (it found the mip-download regression).

## What works now (verified on real hardware — .143: V5 5500 AGP, Athlon 1 GHz, XP SP3)

The stack is a daily driver across **all four APIs**:

- **Direct3D** — the headline capability. The D3D HAL now correctly runs:
  3DMark2001 SE (1601–1633 marks single-chip, 1544 on 2-way SLI, correct
  rendering incl. compressed DXTn and mipmapped textures), UT2004, and —
  believed a first for this hardware/source — **Counter-Strike 1.6 / GoldSrc
  in Direct3D mode**, where it is now the *fastest* renderer (timedemo 33.5
  fps vs OpenGL's 30.6 @1024×768).
- **Glide** — UT99 GOTY on native GlideDrv (~55–58 fps, 60 Hz-vsync-capped at
  every resolution up to 1024×768), glide2x + glide3x both built and shipped.
- **OpenGL** — Quake III (timedemo *four* ~70 fps vsync-off / ~52 vsync-on),
  CS 1.6, Quake2, via the vintage-source ICD.
- **DirectDraw/2D** — desktop at full res, Red Alert 2 / Yuri's Revenge on
  native DirectDraw (after retiring the repack's "Win10" ddraw wrapper).
- **2-way SLI** works across GL/Glide/D3D. Every accelerator wait in the
  display driver is bounded — a wedged GPU degrades instead of hard-freezing
  the OS.

## The fixes (why this source now works when it didn't in 2000)

**[`VINTAGE-FIXES.md`](VINTAGE-FIXES.md) is the definitive ledger** — every
fix with symptom, root cause, commit, hardware verification, and regression
test. Headlines:

- **Ten bugs found in 3dfx's original code**, including: the mip-download
  regression introduced by *3dfx's last-ever change* to `D3TXTR.C` (all
  mipmapped D3D textures black); the TEXBLT FourCC arity mismatch that
  bugchecked any DXTn app (UT2004); the GoldSrc-D3D NULL-context crash and
  the mip-sublevel download drop that rendered its world white; a LOD
  indexing typo latent since 1999; the P8 palettized-texture pipeline 3dfx
  themselves fought and lost (their own kill-switch, now engaged); and the
  desktop-gamma-persist design flaw (games washed out the desktop).
- **Six unbounded hardware busy-waits bounded** — the vintage driver's
  hard-freeze vectors (command FIFO, flips, accelerator waits), each now
  capped with a flight-recorder breadcrumb.
- **An honest ledger of our own bugs** found and fixed along the way.
- **Instrumentation** built into the driver: a registry-ring flight recorder
  (32 slots, survives reboots and hard freezes) logging lifecycle, D3D
  errors with the failing op, SLI transitions, allocation failures, context
  balance, and texture-download tracing. Plus `3dfxctl.exe` (in retro-agent),
  a control panel for every persistent driver knob (clock, refresh,
  vsync, gamma, SLI/AA).

## Build

```bash
cd toolchain-3dfx
# 1. sync any edited files repo tree -> prefix tree (cp)
# 2. purge the .obj of every edited file (build -cZ does NOT reliably rebuild)
rm -f prefix/drive_c/3dfx/H5/W2K/Src/Video/Displays/H5/objfre/i386/<file>.obj
# 3. build (display driver shown; use the Miniport path for 3dfxv5m)
export WINEPREFIX=$PWD/prefix PATH=$PWD/wine/bin:$PATH COPYCMD=/Y
ulimit -f 2000000
timeout 560 wine cmd /c 'c:\3dfx\bldw2k.bat c:\3dfx\H5\W2K\Src\Video\Displays\H5'
pkill -9 -x wineserver
# BUILDEXIT=0 = success; output: .../objfre/i386/3dfxvs.dll
```

Glide and ICD build recipes, and every Wine gotcha, are in `CLAUDE.md` and
`toolchain-3dfx/README.md`.

## Test + deploy (REQUIRED order)

1. **`bash tests/predeploy.sh`** — pre-deploy gate: ~90 source invariants
   (every hardware-verified fix, both trees — repo and build tree must not
   diverge), native pure-logic tests, built-DLL fix markers, stale-`.obj`
   detection. Non-zero exit = **do not deploy**.
2. **Deploy.**
   - *Fresh install* (box on the in-box driver): the retro-agent
     `deploy-3dfx-driver` skill — stages the newest
     `toolchain-3dfx/dist/3dfx-napalm-xp-*` package (trimmed INFs,
     `updrv.exe` SetupAPI installer, backup + signing-policy handling),
     preflight HWID gate, verify, rollback path.
   - *Upgrade in place* (box already on our driver): the byte-verified
     direct swap — UPLOAD, readback-md5 verify, `move /Y` the live DLL to a
     dated `.bak`, `copy /Y` the new one in, re-verify, reboot.
   - *Per-game completion*: the retro-agent **`driver-install`** skill's
     `game_sweep.py` — scans every fixed drive for game-local
     `opengl32/3dfxgl/3dfxogl/glide2x/glide3x/ddraw.dll`, md5-classifies
     against the canonical artifacts, installs the ICD into GL games,
     retires wrapper DLLs (nGlide, ddraw shims), and verifies every write.
     Windows loads game-local DLLs before system32 — **an install isn't done
     until the sweep is clean.**
3. **After reboot:** `python3 tests/run_target_tests.py` (on-box d3dlab D3D
   matrix vs goldens + driver/ring health) **and** the OpenGL golden gate
   (Q3/CS) — D3D and GL share the modeset/2D core, so both gates apply.
   For fullscreen rendering questions use the game's *own* screenshot
   (Q3 `screenshot`, UT `SHOT`) — GDI captures garble in exclusive mode.

**Policy: a fix verified on hardware gets its regression test in the same
commit** — source assertion, native test, d3dlab mode + golden, whichever
layers apply. A fix without a regression test is not done. Details in
`tests/README.md`; change policy in `optimized/README.md`.

## Voodoo 5 6000

The 4x VSA-100 "Strange God" recreation (modern rebuild of the unreleased
6000; four chips behind a HiNT HB1-SE66 bridge) runs our stack. There is one
card and it has moved once — older docs that place it in `.133` are stale:

- **`.133` "P3-DUAL"** (dual P3-700), until 2026-08-31 — verified 2026-08-12
  in **256 MB mode (64 MB/chip) with 4-way SLI**: Q3 61.4 fps, UT99 Glide
  39.8 fps.
- **`.191`** (Athlon 1152 / nForce2, XP SP3), now — **128 MB VBIOS mode**
  (32 MB/chip); this is the box every 2026-09-04 number below was taken on
  ([`DEPLOY-191-20260904.md`](DEPLOY-191-20260904.md) covers the move).

**Headline (2026-09-04): the board is proven good, and the two defects left
are ours.** AmigaMerlin 3.1-R11 — a third-party, retail-lineage driver —
was installed on `.191` next to our stack and renders 4-way SLI **with no
skew**, faster than us in both configurations. Silicon, bridge and analog
combine are therefore fine, and against that control our vintage stack has
two open and independent defects:

| Q3 1.32c `demo four`, 640x480x16, vsync off, `.191` | AmigaMerlin | ours | gap |
|---|--:|--:|--:|
| single chip | 116.5 fps | 92.5 fps | **-21%** |
| 4-way SLI | 151.5 fps | 105.0 fps | **-31%** |

Both rows are a **paired run** — the two drivers benchmarked back to back in the
same session, so the gap is a like-for-like comparison. A later eight-point
resolution sweep measured AmigaMerlin's 4-way 640x480 point separately at
**152.6 fps**; the two agree to 0.7%, well inside this box's ~3% run-to-run
noise. Quote one convention or the other, never a mix of both.

1. **A 4-way scanout skew** — horizontal bands displaced sideways on the
   monitor while the framebuffer is *bit-exact correct* (0 of 307200 pixels
   differ). No screenshot, LFB read or pixel diff can ever see it; per-chip
   register state plus an eye on the monitor are the only sensors.
2. **A ~21% single-chip performance deficit** — measured before any second
   chip is involved, so it is not a symptom of the skew or of SLI.

Where the 6000 material lives:

- [`V56K-BENCHMARKS.md`](V56K-BENCHMARKS.md) — the standing benchmark
  reference: our numbers, the AmigaMerlin control, the published-literature
  comparisons, and which runs are valid at all (on this box any Q3 result at
  or below 1024x768 is a CPU benchmark, not a card benchmark).
- [`V56K-SLI-FINDINGS.md`](V56K-SLI-FINDINGS.md) — the multi-chip/SLI
  investigation, incl. the scanout skew, the candidates killed on hardware,
  and the sweep-safety rules (some register values hard-freeze the board).
- [`V56K-PLAN.md`](V56K-PLAN.md) — the build-out plan; its status header is
  the list of what is still open. Two of its pre-arrival verification items
  are stale — the 5500 work already satisfied them: the D3D HAL is now
  heavily exercised (CS-D3D / UT2004 / 3DMark2001), and Glide2 is proven in a
  real game (UT99 GOTY on `glide2x.dll`).
- [`tools/v56k/`](tools/v56k/) — per-chip instrumentation that needs **no
  driver rebuild**: `fxscan2` (per-chip scanout registers: `dump`, `diff`,
  `phase`, `poke`, `ring`), `fxpci`, `sligrid` (bit-exact Glide pattern with
  read-back, live pokes), and `trials/` (automated bench + sweep harnesses).
- [`V56K-256MB-READINESS.md`](V56K-256MB-READINESS.md) — the 256 MB-mode
  audit; read its outcome banner first (Changes 1-4 were falsified on
  hardware — do not re-apply them).
- [`FINDINGS.md`](FINDINGS.md), the 2026-09-03/04 entries — the raw log
  behind all of the above.

## Layout

- **`3dfx Driver Code/`** — the vintage 3dfx H5/Napalm source drop, archived
  byte-complete; `documentation/` holds our engineering notes on the tree.
- **`toolchain-3dfx/`** — Wine build environment, packaging, `dist/`
  deployable packages. Heavy pieces gitignored; its README documents
  reconstruction.
- **`tests/`** — the regression suite (see above).
- **`optimized/`** — driver-journey documentation, per-game launchers
  (`gltest/cs_fast.bat`), profiling tools.
- **`tools/v56k/`** — per-chip V5 6000 instrumentation and trial harnesses
  (see above); has its own README.
- **`VINTAGE-FIXES.md`** — the fix ledger. **`FINDINGS.md`** —
  investigation matrices (the running log; newest first).
  **`CLAUDE.md`** — the working map.
  6000: **`V56K-PLAN.md`** (plan + open items), **`V56K-BENCHMARKS.md`**
  (numbers), **`V56K-SLI-FINDINGS.md`** (multi-chip/scanout),
  **`V56K-256MB-READINESS.md`** (256 MB mode).
  **`OPEN-STACK-ON-VSA100.md`** — the clean-room stack measured on VSA-100.
