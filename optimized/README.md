# optimized/ — the .143 Voodoo5 driver-optimization campaign

Everything documenting the path from "leaked H5 source that doesn't build" to the
current **retro3dfx 0.3.4d** stack running Q3/Q2/CS/UT/RTCW on a real Voodoo5 5500
under XP. Start here.

## Read in this order

| doc | what it holds |
|-----|---------------|
| [`CHANGELOG.md`](CHANGELOG.md) | **The master ledger** — every version 0.1.0 → 0.3.4d: change, commit, benchmark, disposition. Plus the quality campaign, exp32, and the overnight verification matrix. |
| [`DEBUG-LOG.md`](DEBUG-LOG.md) | Grep-first debugging facts (newest era at top). One-liners for every root cause + the capture/deploy truths. |
| [`CS-GREEN-WORLD-LOG.md`](CS-GREEN-WORLD-LOG.md) | Full narrative of the hardest bug: the 2PPC stale-state green world — hypotheses, ruled-out table, bisect ledger, failed fixes, final fix. |
| [`PROFILING-FINDINGS.md`](PROFILING-FINDINGS.md) | RDTSC frame profiling: why the workload is present-bound and micro-opts were flat. |
| [`TEXT-GARBLE-FINDINGS.md`](TEXT-GARBLE-FINDINGS.md) / [`TEXT-GARBLE-ROOTCAUSE.md`](TEXT-GARBLE-ROOTCAUSE.md) | The long text-garble hunt (superseded conclusion: it was the 16-byte heap alignment, fixed in 0.3.1). |
| [`RESEARCH-2D-TEXT-vs-MESA.md`](RESEARCH-2D-TEXT-vs-MESA.md) / [`RESEARCH-vsa100-pipeline.md`](RESEARCH-vsa100-pipeline.md) | Reference research (Mesa comparison, VSA-100 pipeline). |
| [`OPTIMIZATION-QUEUE.md`](OPTIMIZATION-QUEUE.md) | The original opt backlog + audit of what's worth doing (most micro-opts: not, present-bound). |
| [`DIAGNOSTICS.md`](DIAGNOSTICS.md) | The driver's built-in diagnostic switches (file markers `C:\icd_*.on`, log channels, fbdump). |
| [`OVERNIGHT-SUMMARY.md`](OVERNIGHT-SUMMARY.md) | Historical snapshot of the first overnight (0.1.0 bring-up). |
| [`DEBUG-LOG.md` §build/deploy] + [`gltest/`](gltest/) | The test tooling: `gloop.py` (deploy→run→fbdump→verify), `gfix.exe` isolated repro cases, GDI-Generic oracle method. |
| [`benchmarks/`](benchmarks/) | Per-run benchmark JSONs + capture PNGs. |
| [`src-snapshots/`](src-snapshots/) | Diffable snapshots of the (gitignored) Wine build tree vs pristine source. |
| [`experimental/`](experimental/) | Not-shipped experiments (0.1.4 multitexture dark build, notes). |

## The short version of the journey

1. **0.1.0** — first render: glide3x `grSstWinOpen` NULL-lostContext crash fixed
   (NT-branch escape fallback). Q3 81 fps @640.
2. **0.1.1–0.1.3** — micro-opts (P6 sched, state dedup, vertex cache): all ~flat →
   profiling (RDTSC) proved the workload is **present/engine-bound**; stopped grinding.
3. **0.1.4-exp** — multitexture single-pass: real +6% but half-brightness
   (overbright); parked, not shipped.
4. **0.2.0–0.2.2** — quality/stability: honor GL texture filters in hw; UT
   `glTexSubImage2D` NULL-cache crash fixed → all 4 games stable.
5. **exp32** — 32-bit framebuffer proven feasible/stable/free, but textures are
   16-bit on VSA-100 → modest payoff, parked.
6. **0.3.1–0.3.4d** — the bug-killing campaign: 16-byte tex-heap alignment (text
   garble), palette stride (CS colors), Glide-context reuse (CS fps 3×), stale-2PPC
   full-state re-issue (CS green world). **All known render bugs closed.**

Machine/deploy specifics live in the memory files + `FINDINGS.md` at the repo root;
the deploy skill is `retro-agent/.claude/skills/deploy-3dfx-driver`.

## Change policy (REQUIRED for every driver change)

1. **Never regress a shipped fix.** Every change is verified against the golden
   gate before it ships: CS de_dust fbdump (green must be 0), Q3 menu (text RED)
   + q3dm1 at 640/800/1024 (world renders, correct res in ICD log), Q3 timedemo
   fps vs current baseline, Q2 timedemo, UT launch (0 criticals). `gloop.py cs`
   + the verify scripts automate this. A change that passes its own test but
   fails the gate does not ship (this caught 0.3.4d and 0.3.6 regressions).
2. **API paths stay isolated.** OpenGL fixes go in the ICD (3dfxogl), Glide fixes
   in glide2x/glide3x, D3D fixes in the display HAL (D7D3D/DD paths of 3dfxv5d).
   Never "fix" one API by changing another's path. The display driver's Glide
   escape path (HWCEXT) is shared by the ICD — changes there need the OpenGL
   gate too.
3. **Risky behavior must be gated, default-safe.** Prefer behavior-keyed gates
   (e.g. the 2PPC hook fires only on real dual-texture frames) over global
   changes; diagnostics behind file markers / registry values (inert unless
   enabled); experimental paths behind env/registry (RETRO3DFX_32BPP pattern).
   Per-game splits, if ever needed, key off measured behavior first, exe name
   (GetModuleFileNameA) only as a last resort — and document them here.
4. **Version every build** (renderer string + VERSION + CHANGELOG row) and keep
   the previous binary as a .pre/.old backup at every deploy site.
5. **Fix ledger** (must remain true after any change):
   ICD: 16-byte tex-heap alignment (0.3.1) · palette stride (0.3.2) · Glide ctx
   reuse + all-DB PFDs (0.3.3) · 2PPC swap hook, dual-tex-gated, NO cache reset
   (0.3.4d/0.3.5/0.3.7) · modern-board res-cap bypass (0.3.6) · UT NULL-cache
   guard (0.2.2) · hw texture filters (0.2.0) · cook OOB fix (0.2.1).
   glide3x: NT lostContext fallback (0.1.0).
   Display: H3MakeRoom spin-breaker + registry-ring flight recorder (2026-07-21)
   · D3TXTR per-LOD mip-download addr restore, commit 08fd889 (2026-07-21) ·
   DdFlip pending-swap spin-breaker · DP2/SLIAA ring instrumentation.
6. **Regression tests gate every deploy** (`tests/` at repo root). Run
   `tests/predeploy.sh` before copying any driver binary to a box (source
   invariants + build-tree sync + built-artifact/stale-obj checks), and
   `tests/run_target_tests.py` + the OpenGL golden gate after deploy+reboot.
   **When a fix is verified on hardware, add its regression test (source
   assertion, and a d3dlab mode/golden or equivalent target test) in the same
   commit as the fix — before the next deploy.** See `tests/README.md`.
