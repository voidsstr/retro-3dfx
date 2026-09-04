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
  the box once (NIC dead, physical power cycle). Bracket each step: the agent
  must answer before and after, and uptime must not go backwards; stop on the
  step that breaks it, because which step broke it IS the finding. Pattern:
  `dudxsweep2.py`.
- **One fullscreen 3D app at a time.** If the operator is also driving the box,
  the run is void and the game can crash — that is not a driver fault.

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
