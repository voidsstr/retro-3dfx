# Hardware tests that need the operator's eyes — the persistent-trial pattern

**A visual trial STAYS UP until the operator answers. Never time-box it.**

Some defects on this stack are only visible on the physical monitor — a CRTC
scanout error is the standing example: every software readback (in-engine
screenshot, Glide LFB, GDI capture, the display-driver ring) reads memory or is
SLI-gathered by the hardware, so all of them show a correct image while the
screen is wrong. See `FINDINGS.md`, "The V5 6000's SLI band skew", for the three
instruments that were each *tested* and eliminated.

When a trial needs a human verdict, split it in two:

| script | does | must NOT do |
|---|---|---|
| `*_setup.py`  | launch the game, apply the state, print what is on screen, **exit leaving it running** | close the game, restore the mode, revert the poke |
| `*_teardown.py` | kill the game, revert pokes, restore the desktop mode | run before the operator has answered |

Rules learned the hard way:

- **Do not hold with `sleep` and then tear down.** If the trial ends before the
  reply lands, the answer describes a screen that is already gone and the
  operator has to sit through it again. This wasted several rounds.
- **Say in the question exactly what is on screen and that it will stay.**
- **Liveness-gate every hardware sweep.** Poking a live SLI scanout hard-froze
  the box **twice** (NIC dead, physical power cycle both times), plus one
  self-recovering reboot from forcing SLI band height 16. The two freezes were
  an *ungated* early version of the `vidOverlayDudx` sweep, and the 31 px
  `vga_vsync_offset` step in `vsyncsweep.py` — that second one WAS gated, and
  the gate named the guilty step exactly, which is the whole argument for
  gating. Bracket each step: the agent must answer before and after, and uptime
  must not go backwards; stop on the step that breaks it, because which step
  broke it IS the finding. Pattern: `tools/v56k/trials/dudxsweep2.py` — note
  that this file is the *gated rewrite*, not the version that froze the box.
- **One fullscreen 3D app at a time.** If the operator is also driving the box,
  the run is void and the game can crash — that is not a driver fault.

# Benchmarking methodology — Quake III timedemos on the V5 6000

**THE CARDINAL RULE: on box `.191` (Athlon 1152 MHz / nForce2, V5 6000, 128 MB
VBIOS mode), a Quake III number at or below 1024x768 measures the HOST CPU, not
the card.** Any SLI-scaling figure or driver-vs-driver comparison intended to
say something about the *hardware* must be run at **1280x1024 or 1600x1200**.

This rule cost this project real time. A 4-way scaling figure of 1.30x at
640x480 was carried for a session as a suspected defect; it is in fact the
correct answer to a question asked at the wrong resolution (x86-secret measured
1.34x at the same point on an Athlon XP 2800+). See `FINDINGS.md`, 2026-09-04
entry "the V5 6000 is CPU-bound to 1024x768, and the fill wall is gradual".

## The evidence (VERIFIED ON HARDWARE, `.191`, AmigaMerlin 3.1-R11, 4-way SLI)

Eight-point sweep, Q3 1.32c `demo four`, 16-bit, vsync off:

| r_mode | resolution | Mpx | fps | ms/frame | marginal ms/Mpx |
|--:|---|--:|--:|--:|--:|
| 2 | 512x384   | 0.197 | 147.6 |  6.775 | - |
| 3 | 640x480   | 0.307 | **152.6** |  6.553 | - |
| 4 | 800x600   | 0.480 | 150.0 |  6.667 | 0.66 |
| 5 | 960x720   | 0.691 | 147.9 |  6.761 | 0.45 |
| 6 | 1024x768  | 0.786 | 147.1 |  6.798 | 0.39 |
| 7 | 1152x864  | 0.995 | 138.7 |  7.210 | 1.97 |
| 8 | 1280x1024 | 1.311 | 119.4 |  8.375 | 3.70 |
| 9 | 1600x1200 | 1.920 |  71.9 | 13.908 | 9.08 |

Three independent reasons this is a host wall and not a card wall:

1. **512x384 benchmarks SLOWER than 640x480 (147.6 vs 152.6).** Below ~1 Mpx
   the card is not in the measurement at all; 512-1024 spans 147-153 fps, which
   is run-to-run noise (~3%) on a flat line. Treat those five points as ONE
   number.
2. **The fit says so.** Fitting 640-1024 gives `t = 6.41 ms + 0.50 ms/Mpx` —
   a fixed per-frame cost of 6.41 ms, i.e. a **CPU ceiling of ~156 fps**. A
   2.6x increase in pixels costs 3.6% of the frame rate.
3. **The period CPU ladder agrees.** AnandTech (Oct 2000) published a Q3
   640x480x16 CPU ladder: Athlon 800 = 128, 900 = 137, 1.0 GHz = 144,
   1.2 GHz = 162 fps ("a 1% performance increase per MHz"), with 1024x768x32
   dead flat at 83 fps for every CPU on the same chart. Interpolating to
   1152 MHz gives **~158 fps** — within 1.5% of our measured ceiling.

Corroborating published data, all 640x480x16, all showing zero multi-chip
scaling because the host is the limit:

| source | CPU | 1 chip | 2 chips | 4 chips |
|---|---|--:|--:|--:|
| thedodgegarage | Celeron 1000 | 83 (V4 4500) | 81 (V5 5500) | 82 (V5 6000) |
| AnandTech | Athlon 750 | 83.0 (V4 4500) | 83.0 (V5 5500) | - |

x86-secret state it outright for 640x480 on an **Athlon XP 2800+** — the 5500
and the 6000 come out level, *"was hier auf eine Limitierung seitens der CPU
schliessen laesst"*. If a 2.1 GHz Barton is CPU-bound at 640x480, a 1.15 GHz
Athlon certainly is.

## Where the card IS the limit — scaling by resolution

Same box and settings; single-chip half of the sweep run separately
(`ambench.py 0`):

| resolution | 1 chip | 4-way SLI | scaling |
|---|--:|--:|--:|
| 640x480   | 116.5 | 152.6 | 1.31x  (CPU-bound — meaningless) |
| 1024x768  |  55.2 | 147.1 | **2.66x** |
| 1280x1024 |  35.1 | 119.4 | **3.40x** |
| 1600x1200 |  24.7 |  71.9 | 2.91x |

1024x768 reproduces x86-secret's published 2.66x for a real 6000 prototype
exactly; 1280x1024 (3.40x) matches GamersNexus' 3.46x at their heavier
settings. **Bench the 6000 at 1280x1024 and 1600x1200, or measure nothing.**

**Known anomaly, not a regression:** scaling PEAKS at 1280x1024 and falls back
to 2.91x at 1600x1200 while single-chip stays perfectly linear there. That is a
4-way-path ceiling above ~1.3 Mpx and it is UNEXPLAINED. Do not read it as a
fresh defect when it reappears.

## The single-chip reference line — the regression detector

Single-chip cost on this board is a **straight fill-rate line**: the marginal
cost between successive sweep points is **19.9 / 19.8 / 19.7 ms per megapixel**
across 640 -> 1024 -> 1280 -> 1600. Three independent intervals agreeing to 1%
means one VSA-100 here is fill-rate-bound at *every* resolution including
640x480, with nothing else interfering.

**Use `19.8 ms/Mpx` single-chip as the calibration constant of this board.** Any
future driver build that departs from it has a throughput regression, and the
departure is measurable without a second chip, without SLI, and without eyes.

Corollary worth knowing: because a single chip is fill-bound even at 640x480, a
**single-chip A/B at 640x480 is still a legitimate driver measurement** even
though the 4-way number at that resolution is not. That is how the current
deficit was established (`FINDINGS.md`, 2026-09-04 "OUR DRIVER IS 21% SLOWER
THAN AMIGAMERLIN ON ONE CHIP"):

| 640x480x16, `demo four`, `.191` | AmigaMerlin 3.1-R11 | ours (H5 + SGL ICD 0.5.0) | gap |
|---|--:|--:|--:|
| single chip | 116.5 | 92.5 | **-21%** |
| 4-way SLI | 151.5 | 105.0 | -31% |

Those are the **paired** run — both stacks measured back to back in one
sitting. The eight-point sweep above separately measured AmigaMerlin's 4-way
640x480 point at 152.6; the two agree to 0.7%, inside the ~3% run-to-run noise.
Quote one or the other, never a mix of both.

## The harness — `tools/v56k/trials/ambench.py`

    python3 tools/v56k/trials/ambench.py <sli_cfg> "<tag>" <r_mode,list>

- `<sli_cfg>` is written to `SSTH3_SLI_AA_CONFIGURATION` under **both**
  `...\Services\3dfxvs\Device0\Glide` and `...\Device0\D3D`. **`0` =
  single-chip, `5` = 4-way.** The two values get there by different routes, and
  the switch really does list neither: in `GLIDE3/SRC/GPCI.C:1446`, `case 0`
  sets `forceSingleChip = 1`, while **`5` has no case at all** and falls through
  to `default:` — no force, no AA — so it takes whatever SLI the board already
  has, which on a 6000 is 4-way. 3dfx's own comment table above that switch
  (`GPCI.C:1420-1431`) documents exactly that meaning: `5` = "4-way SLI enabled,
  AA disabled", valid for 4-chip boards only. There is likewise no `case 2`
  (2-way), and the miniport's `EnableSLIAA` refuses any request whose
  `dwChips != numUnits`, so on this board it is 4-way or single-chip and nothing
  else.
- `<r_mode,list>` is a comma-separated Q3 `r_mode` list; default `3,4,6,8`.
  `2`=512x384 `3`=640x480 `4`=800x600 `5`=960x720 `6`=1024x768 `7`=1152x864
  `8`=1280x1024 `9`=1600x1200.
- Per run it kills any stale `quake3.exe`, deletes the Q3 console log,
  launches the timedemo, then polls that log for up to 200 s.
- It restores the desktop to 1024x768x16@75 when the sweep ends.

Examples actually used:

    ambench.py 5 "AM-4way"   2,3,4,5,6,7,8,9    # full 4-way resolution sweep
    ambench.py 0 "AM-1chip"  3,6,8,9            # single-chip reference line

**Liveness-gated, like every hardware sweep here.** It takes an uptime baseline
from `SYSINFO` and aborts the sweep on `UNREACHABLE` (agent gone) or `REBOOTED`
(uptime went backwards). The step that breaks liveness IS the finding — see the
persistent-trial rules at the top of this file.

## The pass metric needs no eyes

**Whether the timedemo COMPLETES and PRINTS AN FPS LINE is a sufficient
automated health check.** A driver that wedges the card never gets there, so a
benchmark sweep doubles as a stability gate with no operator in the loop.

The harness sets `logfile 2` so Q3 mirrors the console to
`C:\q3home\baseq3\qconsole.log`, and matches:

    (\d+) frames, ([\d.]+) seconds: ([\d.]+) fps

No fps line inside 200 s = FAIL for that cell. This is how the whole
AmigaMerlin-vs-ours comparison was produced without anyone watching the screen.
(It does NOT detect the SLI band skew — that defect is invisible to every
software readback; see the persistent-trial section and `FINDINGS.md`, "The
V5 6000's SLI band skew".)

## Settings that MUST be pinned

| setting | value | why |
|---|---|---|
| `r_swapInterval` | `0` | engine-side vsync off |
| `FX_GLIDE_SWAPINTERVAL` | `0` (env) | Glide-side vsync off |
| `r_colorbits` | `16` | 16 vs 32 changes the answer by ~1.2x on this silicon; never leave it to the config |
| `r_mode` / `r_fullscreen` | swept / `1` | resolution is the whole point |
| `r_glDriver` | `3dfxogl` | otherwise the ICD is not what is being measured |
| `r_displayRefresh` + `FX_GLIDE_REFRESH` | `60` | pins scanout timing; see the AmigaMerlin trap below |
| `fs_homepath` | `C:\q3home` | keeps `qconsole.log` where the harness reads it |
| `s_initsound` | `0` | removes a host-CPU consumer from a CPU-bound test |
| `timedemo 1` + `demo four` + `nextdemo quit` | - | run and exit, so the sweep can continue unattended |

**Both vsync knobs are required. The Glide env var alone does not reach the
engine's own swap logic**, and `r_swapInterval` alone does not reach Glide. Set
both or the number is capped by the refresh rate.

**AmigaMerlin trap:** its INF installs `FX_GLIDE_REFRESH = 75` into
`...\Device0\Glide`, which overrides the per-resolution refresh for EVERY mode
and put the monitor out of range. Delete it (or set 60) before benchmarking.

**Refresh provenance — never compare a benchmark run against a register dump.**
`ambench.py` is the only harness here that pins refresh (`FX_GLIDE_REFRESH=60`
*and* `+set r_displayRefresh 60`). The trial scripts that produced the register
dumps — `trial_setup.py`, `ringrun.py`, `ringtrial.py` — set **no refresh at
all**, so those captures ran at the ICD's own per-resolution default of **85
Hz** while every benchmark ran at 60 Hz. (The AmigaMerlin dump is a further
exception: `FX_GLIDE_REFRESH` had been forced to 60 for that one, to stop the
monitor going out of range.) The 60-vs-85 gap is therefore a **configuration
difference this session introduced between the two captures**, not a property of
either driver — do not read it as one when the skew documents and this file are
read side by side. **Any future capture must pin refresh explicitly or the
comparison is worthless.**

### The 16-bit / 32-bit ratio is itself a diagnostic

VoodooExtreme's Dec-2000 review is the only published test of our driver's own
code lineage (3dfx's V5-6000 reference driver) and reports **88.4 fps at 16-bit
vs 86.1 at 32-bit = 1.03x**, which its author calls out as a driver bug.
Community drivers on the same silicon get **1.20x** (VoodooAlert, 155.1 vs
128.7). So running our stack at both colour depths and reading the ratio
distinguishes an inherited 3dfx defect (~1.0x) from a deficit of our own
(~1.2x). Fully automated, no visual verdict. HYPOTHESIS — not yet run.

## Comparing against published benchmarks

**Only WITHIN-article ratios are trustworthy. Cross-review variance is ±20%,
and that is measured, not assumed:** for the identical card, CPU, demo and
resolution, AnandTech reports 79.5 fps where PC Perspective reports 71.3.

Never quote our absolute fps against another site's absolute fps as evidence of
anything. Quote scaling ratios, colour-depth ratios, or chip-count ratios taken
from a single article — which is exactly why the 2.66x-vs-2.66x agreement at
1024x768 is worth more than any absolute-fps agreement.

Sources excluded from this comparison, with cause (do not re-chase — see
`FINDINGS.md`, 2026-09-04 "OUR DRIVER IS 21% SLOWER..."):

| source | cause |
|---|---|
| gaming2k / tredfx "Athlon 1 GHz 6000" | FABRICATED — denied at source by 3dfx, never archived |
| I/ITSEC 1600x1200 4xFSAA demo | 3dfx taped over the fps counter and banned timedemos |
| Hartware.de | 16-bit runs use *Fast* preset, 32-bit runs High Quality — not like-for-like |
| HotHardware | demo unstated, charts internally inconsistent |
| PCGH 2017 Rev A-3700 | card locked up, zero numbers published |
| ModLabs zx-c64 6000 | Q3 1.11/Demo1, 32bpp, WinME, **PCI** card — four condition mismatches |

Also record what has never been measured rather than papering over it: **no
published Q3 benchmark of any V5 6000 on an Athlon-class CPU at 1.0-1.2 GHz
exists, no published 640x480 figure for any modern 6000 recreation exists, and
nothing at all exists for a driver built from the leaked H5/Napalm source.**
Our numbers are the first of their kind, and AmigaMerlin on the same box is the
only valid control.

## Operational

- **One fullscreen 3D app at a time.** If the operator is also driving the box
  the run is void and the game can crash — that is not a driver fault.
- **Cooldowns between flat-out timedemos** (`V56K-SLI-FINDINGS.md` §11-§13).
- A hard freeze means the NIC is dead and the box needs a physical power cycle;
  ask the user.

# Driver test suite

Regression tests for the self-built 3dfx XP driver stack (display driver,
miniport, D3D HAL, instrumentation). **Every verified fix gets a test here
before the next deploy** — see the policy section in the repo `README.md`.

## Layers

| Layer | Script | When |
|---|---|---|
| Source invariants | `test_source_invariants.sh` | pre-deploy (host, instant) — grep-based presence checks for each display-driver fix |
| **Native logic tests** | **`run_native.sh`** | **pre-deploy (host, instant) — executable pure-logic tests of ICD/HAL fix invariants (`native/test_*.c`, via `munit.h`)** |
| Built artifact | `test_built_artifact.sh [dll]` | pre-deploy (host, instant) — instrumentation strings, stale-obj checks, **+ `codegen_8e_guards.py` codegen asserts** |
| Codegen 0x8E guards | `codegen_8e_guards.py [dll]` | pre-deploy (host, instant) — objdump-asserts the two `0x1000008E` NULL-deref fixes (DdBlt g_pHndlList, DrvBitBlt psoSrc) are in the linked machine code, not just the source |
| Pre-deploy gate | `predeploy.sh [dll]` | runs the three above; non-zero exit = do NOT deploy |
| On-target D3D matrix | `run_target_tests.py [host]` | after deploy+reboot (~2 min on .143) |
| OpenGL golden gate | `/tmp/post_instr_verify.py` (session tool; Q3 render + CS 0-green) | after any display-driver deploy |

The whole stack (these + the Python client tests) also runs in one shot from the
sibling repo: `bash ../retro-agent/tests/run_all.sh`.

## Native logic tests (`native/`, executable invariants)

Where `test_source_invariants.sh` greps that a fix's *line* is present,
`native/test_*.c` compile the fix's *arithmetic* natively and assert the
invariant (and the old buggy value, as executable bug documentation). No Wine,
no hardware. These cover the ICD (MesaFX) pure-logic fixes the source-invariant
greps can't reach (the ICD lives in the external `retro3dfx-gl` fork):

- `test_texheap_align.c` — 0.3.1 garble: 16-byte texture-heap base + alloc rounding
- `test_mip_download_addr.c` — 08fd889 D3D black textures: per-LOD download
  offset must match the chain layout for every mip level (rev-40 stale-addr
  bug asserted as the counterexample)

## What the tests encode (fix ledger)

- **Mip-download fix** (`08fd889`): D3TXTR.C per-LOD board-offset line —
  source assertion + `d3dlab big512mip / mippoint / miplinear` goldens.
  Regression signature: big512mip → black quad, mippoint → garbage stripes.
- **DdFlip / H3MakeRoom spin-breakers** (hard-freeze vectors): source +
  binary-string assertions.
- **DP2 / SLIAA ring instrumentation**: source + binary-string assertions +
  on-target ring positive control.
- **v56k 6000 miniport additions purely additive** (5500 bit-identical):
  diff-count assertion against the vintage tree.
- **Repo-tree ↔ build-tree sync** for every fixed file (the wine build
  consumes `toolchain-3dfx/prefix/...`, not the repo tree).
- **Stale-object detection**: the vintage build silently links old `.obj`
  files; the artifact test compares source vs obj vs DLL timestamps.

## d3dlab

`d3dlab/d3dlab.c` — minimal **windowed** D3D8 app (windowed ⇒ GDI screenshots
are truthful). One texture/stage configuration per invocation, 5 s runtime.
Modes: `sel1 mod modgray mod2x spec tex2 tex2sel mippoint miplinear mipfar
big512 big512mip`. Build with `d3dlab/build.sh` (MinGW). Goldens in
`golden/d3dlab_golden.json` were measured on .143 after the mip fix was
visually verified; tolerance ±12 per channel. If you change d3dlab, re-measure
the goldens on a KNOWN-GOOD driver build and update the JSON in the same
commit.
