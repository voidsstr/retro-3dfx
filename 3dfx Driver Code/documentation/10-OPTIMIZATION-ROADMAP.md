# 10 — Optimization Roadmap (Phase 2: the modernized driver)

Goal: a Voodoo 3 / Voodoo 5 driver measurably better than the Amigamerlin-era community
packages, by applying math/CS/engineering advances from 2001→present to this codebase.
Amigamerlin repackaged this same code with configuration tuning; it did not restructure the
internals below — that is the available headroom.

## 0. Where the performance actually is

The silicon is fixed. Every win comes from one of:

| Budget | Constraint | Attack |
|---|---|---|
| **Bus bandwidth** | PCI ≈ 132 MB/s (V3 PCI), AGP 2x/4x (V3 AGP / V5) — vertices + textures + register writes all share it | Send fewer bytes per triangle; compress textures; batch state |
| **Fill rate** | V3: 1 pipe ×166 MHz; V5: 2 chips × 2 pipes ×166 MHz; killed by overdraw | Don't draw what's hidden; front-to-back where legal |
| **CPU time** | Effectively free on any modern host; was the 2000-era obsession | Reinvest it: cull, compress, reorder, predict |
| **Latency/pacing** | Swap queue, bump cadence, fence stalls | Modern pacing + cheaper fences |

Measure first: GlideTrap replays + Quake/Unreal timedemos are the harness
(see [08-DEBUGGING-AND-DIAGNOSTICS.md](08-DEBUGGING-AND-DIAGNOSTICS.md) §4, §9).

## Phase 2a — free wins (days each, low risk)

### A1. Modern compiler + PGO/LTO
Rebuild with Clang/modern MSVC, profile-guided optimization on GlideTrap replays, LTO,
hot/cold splitting. Expected: 10–30% CPU-side. Files: build system only.
Risk: K&R-isms and asm interop; keep `GLIDE_USE_C_TRISETUP=1` first.

### A2. Retire 2000-era micro-hacks that are now pessimizations
- `P6FENCE` locked `xchg` (`FXGLIDE.H:2094`) → `sfence` (~2 cycles vs ~20–100).
- `FP_FLOAT_CLAMP` per-value denormal test (`FXCMD.H`) → set **DAZ/FTZ** in MXCSR once at init.
- `RGBA_COMP` float-bias packing and the integer-image sign-bit cull → SSE2
  `cvttps2dq`/`movmskps` (exact, faster, clearer).
- LUT-heavy paths (gamma, format conversion) → recompute; modern CPUs beat L2-miss LUTs.
Files: `FXCMD.H`, `FXGLIDE.H`, `GXDRAW.C`, `GTEXDL.C`. Validate with CSIM pixel-diff.

### A3. AVX2 the copy loops
`TRI_SETF` vertex streaming, `_grTexDownload_*`, LFB writes → 32-byte `vmovntps` streaming
stores (WC-friendly, self-fencing semantics still require the 64K rule — verify hole-counter
behavior on real HW with `FX_GLIDE_FENCE_LIMIT` bisect). Replaces XDRAW2/3 3DNow! and XTEXDL
MMX entirely (also un-breaks the disabled-K7 case noted in `GPCI.C`).

## Phase 2b — bus bandwidth (the biggest lever)

### B1. Stripification & vertex-cache-aware reordering of indexed draws
Modern meshopt/Forsyth-class algorithms (2006–2017) on D3D indexed primitives and Glide
vertex arrays at submit (cache keyed on IB/VB identity for static buffers): convert lists to
long PKT3 `BDDDDD` continuations. A strip triangle ≈ 1 vertex of FIFO bytes vs 3 for a list
triangle → up to ~3× less per-triangle bus traffic. Files: new module + `DISTRIP.C`,
`D6DP2.C`/`AMESH*` call sites. Expected: largest single FPS win on bus-limited scenes
(PCI Voodoo3 especially). Risk: must preserve exact rasterization order semantics where the
API requires it (alpha-blended geometry: reorder only within provoking constraints).

### B2. Runtime code generation for vertex emission
Replace the interpreted `tsuDataList` copy loop and the 8-entry `_trisetup_*` matrix with a
tiny **copy-and-patch JIT** (2021-era technique; what Mesa's draw/llvmpipe do): emit the exact
mov sequence for the current vertex layout + cull mode. Kills per-float loop overhead and
per-triangle indirect-call misprediction. Files: `GXDRAW.C`, `FXGLIDE.H` dispatch,
`FXGASM.C` offsets. Expected: 1.5–3× on the submission inner loop.

### B3. Transparent FXT1 compression at texture-download time (V5 only)
TEXUS2's encoder (`CODEC.C`, PCA in `EIGEN.C`) was offline-only. Modern cluster-fit +
least-squares endpoint refinement (squish/ISPC-texcomp lineage, AVX2) compresses in
real time: hook `grTexDownload*`/D3D texture create, encode RGB565/8888 → FXT1 (4 bpp) unless
the app opts out. 4× effective texture memory + 4× less download traffic → fewer texture
thrash stalls. Files: `GTEXDL.C`, `D3TXTR.C`, new encoder lib. Risk: quality on UI/lightmap
textures — add per-format heuristics + registry override. (V3 has no FXT1 decode: use
NCC/palettized equivalents there, smaller win.)

### B4. State-change dedup above the shadow layer
The PKT4 masked writes are already minimal per-draw; add cross-draw redundancy elimination
(hash last-written group values; skip no-op flushes) and sort-by-texture within
non-overlapping batches in the D3D DP2 stream. Files: `DISTATE.C`, `D6DP2.C`.

## Phase 2c — fill rate

### C1. CPU masked occlusion culling (Intel MOC, 2016)
Maintain a hierarchical software depth buffer from the D3D T&L stream (and optionally Glide
depth-hinted draws); test batch AABBs before submission. A modern core does this in <1 ms/
frame at 2000-era scene complexity; the card skips all occluded fill. Expected: large wins in
corridor/city scenes (exactly where V5 FSAA chokes). Files: new module in the T&L path
(`TLREND.C`/`PROCPRIM.C`). Not applicable to pre-transformed Glide games without depth info —
D3D/OpenGL paths first.

### C2. Adaptive SLI band height (V5)
Static `h3sliBandHeight` → per-frame feedback controller (measure per-chip busy via status
reads at swap; nudge band height / rebalance). Files: `MINIHWC.C`, swap path in `GSST.C`.
Expected: smooths the worst-case chip imbalance; helps 4x FSAA modes most.

### C3. Smarter texture-memory management
Replace the first-fit TMU allocator with LRU + ghost-list (ARC-style) eviction and
prioritized async uploads (PKT5 already pipelines — schedule by next-use prediction from the
replay/frame history). Files: `GTEX.C`/`GTEXDL.C`, `D3TXTR.C`/`TCUTILS.C`.

## Phase 2d — structural

### D1. Threaded command building
Move T&L/cull/compress onto worker threads; game thread only records; a submission thread owns
FIFO order (lock-free SPSC ring feeding the WC ring; the existing JSR/RET mechanism can splice
independently built segments). Modern equivalent of what `GTHREAD.C` never became. Highest
complexity; do last, after A/B/C prove out. Keep invariant §7 of
[01-ARCHITECTURE.md](01-ARCHITECTURE.md): per-context packet order.

### D2. Frame pacing & latency
Replace the fixed 0–3 `swapPendingCount` with measured-frame-time pacing (target scanout,
sleep-until-slot) — smoother frametimes at identical FPS; players perceive this more than
averages. Files: `GGLIDE.C` swap, `DDFLIP.C`.

### D3. Autotuned per-game profiles
Replace the hand-coded cipher table in `DDFXS32.C` with a data-driven profile store +
offline autotuner (search over band height, swap depth, LOD bias, AA mode against GlideTrap
replays — OpenTuner-style). Ship best-known configs; learn new ones locally.

### D4. Correctness modernization (what "better than Amigamerlin" also means)
- Differential testing vs **CSIM** (pixel-exact) on every change; fuzz packet builders
  (libFuzzer) against CSIM's parser.
- ASan/UBSan on the user-mode stack; fix the known-fragile areas first
  (windowed alt-tab, NT mutexing — [09-INCOMPLETE-AND-GAPS.md](09-INCOMPLETE-AND-GAPS.md) §8).
- 64-bit-clean the user-mode stack (the code is full of `FxU32` pointer casts — required for
  any modern-OS port).
- CI harness: DIAGS mustpass → Glide CONFORM → GL CONFORM/OGTST → GlideTrap replays with
  golden-image + frametime budgets (the original QA ladder, automated).

## Sequencing summary

```
A1 A2 A3      (weeks 1–3: rebuild, de-hack, vectorize — establishes baseline + harness)
B1 B3         (the two big bandwidth wins)
C1            (fill-rate win for D3D/GL titles)
B2 B4 C2 C3   (second wave)
D2 D3         (feel + per-game polish)
D1 D4-ongoing (structural; correctness runs the whole time)
```

## Non-goals (documented so nobody burns time)

- Shaders/HW T&L emulation: no silicon support; anything shader-like means CPU rasterization,
  which loses to the card.
- Re-enabling AGP FIFO on Voodoo3 (hardware bug — §8 of doc 09).
- Beating the card's peak fill/texture rates: not physically possible; the ceiling is "the
  hardware, fed perfectly." The roadmap above is about *feeding it perfectly*.
