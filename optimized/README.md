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
