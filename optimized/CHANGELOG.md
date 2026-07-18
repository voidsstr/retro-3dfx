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
| 0.2.0 | honor GL min/mag texture filters in hardware at every grTexSource bind (was set once at init = POINT_SAMPLED, so every minified texture point-sampled → shimmer) | 131e430 | 640x480: 76.5 (no regression) | `__glSSTApplyGrFilter` at 7 grTexSource sites, both TMUs. User confirmed "somewhat better" on monitor. GL_RENDERER [retro3dfx 0.2.0]. |
| 0.2.1 | GLCORE `__glCookSubTexture` OOB fix: sub-image row origin used `w*y+x` not `lp->width*y+x`, srcSkip missing `*bpp` — walked out of bounds on any partial update. Only affects the generic (non-SST) cook path. | (this commit) | n/a (correctness) | Necessary but NOT the UT crash (the SST hw path is separate — see 0.2.2). Kept: correct for the generic path. |
| **0.2.2** | **UT glTexSubImage2D crash FIX**: `__glsstim_TexSubImage2D` (SST_TEX.C:3427) deref'd `cache->addr` with a NULL hw-cache pointer on non-resident partial texture updates (UT's create-empty-then-subimage lightmap pattern) → GPF loop → Double fault. Guard the partial download with `if (cache)`. + `element_size` default in `__glSSTShadowTexSubImage`. | 2c3b912 | Q3 76.5 / Q2 178-183 / CS 67.7 (all no-regression) | **★ ALL 4 GAMES STABLE.** UT loads CityIntro + runs 0 criticals (was crashing in seconds). Root cause proven by instrumentation: pre-download 6241 vs post-download 5754, Δ=487 == exactly the 487 `cache=0` calls. Additive guard (cache!=NULL path byte-identical) → can't regress Q3/Q2/CS. GL_RENDERER [retro3dfx 0.2.2], md5 6b8182dd. |

## Quality campaign (present-bound ⇒ fill is ~free)

Profiling proved Q3/Q2/CS are present/engine-bound on the V5 5500 + P3 — fps is
capped, so image quality is nearly free. Measured on ICD 0.2.2:

| quality | Q3 640 fps | vs baseline | notes |
|---------|-----------|-------------|-------|
| default (picmip 1, bilinear, 16-bit) | 76.5 | — | |
| **max** (picmip 0, trilinear `GL_LINEAR_MIPMAP_LINEAR`, 16-bit) | **72.0** | **−5.9%** | Full texture detail + trilinear for 6% — in-game render clean+detailed (see benchmarks/quality_q3dm1.png), still >60fps. Menu proportional font readable in capture, minor slicing (postfilter = supervised display-driver fix). |
| **max + 32-bit request** (r_colorbits 32) | 70.9-71.7 | ≈ same (present-bound) | Q3 asked for 32-bit (`GLW_ChoosePFD(32,24,8)`) but the ICD could only offer `PIXELFORMAT 3 = color(16-bits)` → render bit-identical to 16-bit (distinct colors 3633 vs 3637, same mean-lum). **The ICD's PFD table has no 32-bit color entry.** Added `--colorbits 32` to the bench skill (default 16). |

### ★ Next V5 quality lever discovered: true-color (32-bit) rendering

The VSA-100/Napalm renders 32-bit ARGB internally, but our ICD only advertises
16-bit color PFDs, so every app is stuck at 16-bit (RGB565) with dither banding.
The Voodoo3 lane physically can't do 32-bit; the V5 can — this is a V5-specific
quality win that costs ~0 fps (present-bound). **Implementation (scoped, not yet
done):** (1) add 32-bit color entries to the ICD PFD table (WGLCMDS.C ~1249,
GLICD.C ~578); (2) `grSstWinOpen(...GR_COLORFORMAT_ARGB...)` (sst_export.c:734)
defaults to 16-bit RGB565 — a 32-bit PFD must open via `grSstWinOpenExt` with
`GR_PIXFMT_ARGB_8888`; (3) verify the ICD color-buffer read/clear/LFB paths
handle 32bpp. Objective verification metric: q3dm1 capture distinct-color count
jumps from ~3637 (16-bit) toward tens of thousands (true-color), banding in the
sky gradient disappears. Render-changing ⇒ develop as an experimental A/B build,
on-monitor sign-off before shipping (per HARD LESSONS).

#### exp32 build result (0.2.3-exp32, env-gated RETRO3DFX_32BPP) — feasible + stable, marginal gain

Implemented an env-gated true-color path (all changes no-op unless RETRO3DFX_32BPP
set, so the deployed binary == 0.2.2 by default): WGLGLIDE.C `__wglGlideGetDisplayMasks`
→ 8/8/8 masks + return 32; color LFB `GR_LFBWRITEMODE_565` → `_8888`; sst_export.c
`grSstWinOpen` → `grSstWinOpenExt(GR_PIXFMT_ARGB_8888)` resolved via `grGetProcAddress`
(it is NOT a static export — DIGET.C:1097; direct-link fails with LNK2001). Build
gotcha: a stale `sst.lib` retained the old obj — must delete `release/sst.lib` +
`SST/release/sst.lib` for the recompile to take.

RESULT on Q3 (env set, r_colorbits 32): **the hardware opened ARGB_8888 fine** (ICD
log: `grSstWinOpenExt=0x... grSstWinOpen OK`), **stable, clean render, ~same fps
(72-73)**. BUT: (1) q3dm1 distinct-color count barely moved (3637 → 3743) and the
render looks identical — because **VSA-100 TEXTURES are 16-bit** (TMU samples
RGB565/ARGB4444), so texture-dominated scenes stay 16-bit-ish; 32bpp only helps
gradients/alpha/fog/multi-pass accumulation. (2) Q3 still selected a 16-bit PFD
(`PIXELFORMAT 3 = color(16-bits)`) — the getDisplayMasks→32 change did NOT reach the
PFD-selection path, so the ICD software buffer stayed 16bpp = latent hw-32/sw-16
mismatch (rendered fine anyway since in-game paths are GPU-side). **Disposition: 32bpp
is PROVEN feasible + stable + free on the V5, but the visible quality payoff is modest
(texture precision is the real 16-bit limiter, hardware-fixed). Not shipped; box kept
on 0.2.2. To finish: fix PFD selection to actually pick 32-bit (updatePixelFormats/
getDisplayMasks ordering — figure out why the Glide getDisplayMasks isn't authoritative
at ChoosePixelFormat time) so sw matches hw; then verify on-monitor a gradient-heavy
scene (sky/fog) for the real banding-reduction win.**
