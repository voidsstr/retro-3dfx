# 3dfx-driver-optimized — CHANGELOG

Our self-built 3dfx Voodoo driver stack (miniport `3dfxv5m.sys`, display `3dfxv5d.dll`,
`glide3x.dll`, OpenGL ICD `3dfxogl.dll`), built from the H5/Napalm source under Wine,
deployed to **192.168.1.143** (Voodoo5 5500, XP SP3). Tracked in specpicks DB as
`driver_stack.name = 3dfx-driver-optimized`, machine id=3.

Each optimization is a separate commit + a benchmark JSON under `optimized/benchmarks/`.
Deploy/bench harness: `tools/deploy_bench.py`. Q3 timedemo `four`, r_mode/colorbits per run.

## Iteration ledger

| ver | change | commit | Q3 timedemo (res/fps) | notes |
|-----|--------|--------|-----------------------|-------|
| 0.1.0 | first rendering build: glide3x grSstWinOpen NULL-lostContext fix (NT-branch dummyContextDWORD fallback) | 092ca60 | 640x480: **81.2** | Q3 four 16bpp; GL_RENDERER 3Dfx. Full self-built stack renders on Voodoo5 5500. Beats era V3 refs + .124 MesaFX (58.8). |
| 0.1.1 | /G5->/G6 P6 (PPro/PII/PIII) instruction scheduling, glide3x + ICD, clean rebuild | (this commit) | 640x480: 80.2 (flat vs 81.2) | **INERT** — C-flag scheduling doesn't help: hot paths are hand-asm (__GL_USE_INTEL_ASM). Confirms bottleneck = asm/per-triangle/state, not C codegen. Kept (correct P6 target). |
| 0.1.2 | glide3x state-setter redundancy dedup (11 setters skip _grValidateState on unchanged args; assertDefaultState first-call guard) | (this commit) | 640x480: 79.7 (flat) | Render pixel-identical to 0.1.1 (verified). Flat/within-noise: this demo isn't state-validation-bound. Kept (correctness-verified, reduces per-drawcall CPU). Confirms bottleneck is T&L, not state. |
| 0.1.3 | ICD post-transform vertex cache in CompileElementsIndexed (dedup shared indices within 36-elt batches, verified-hit struct-copy) | (this commit) | 640x480: 80.5 vs 79.9 before (+0.6, +0.75%) | Drift-controlled back-to-back A/B. Render pixel-identical. Marginal — 36-elt batch window limits cross-batch sharing AND the demo isn't strongly T&L-bound. Kept (correct, zero-regression). |
| 0.1.4-exp | GL_ARB_multitexture single-pass world rendering (2 TMU) + DrvGetProcAddress fix (was NULL -> blocked ALL extensions under XP opengl32) | (this commit, EXPERIMENTAL) | 640x480: **84.5 (+5-6%)** but render TOO DARK | **NOT SHIPPED.** Real +6% single-pass win + the DrvGetProcAddress fix are both valuable. But single-pass is ~half brightness: Q3 sends identityLight=0.5 expecting a 2x overbright the 2-pass path got free from the lightmap blend (GL_SRC_COLOR/GL_DST_COLOR). Added 2x via grColorCombineExt shift=1 — log confirms the branch engages (ext_active=1, texEnv 0x2100x2) but the Napalm output-shift does NOT double on hardware. NEXT: apply the 2x by a different mechanism (double the iterated color, or grConstantColorValue factor, or grTexCombine scale) — the ext output-shift is a dead end here. dist/ + .143 kept at correct-rendering 0.1.3. |
