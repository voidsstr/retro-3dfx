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
| **0.3.1** | **★ 2D TEXT GARBLE SOLVED (Q3 menu font1_prop + CS VGUI)** — Napalm 16-byte texture-heap alignment. `__glSSTInitTextureManager` started the TMU0 heap at `grTexMinAddress + 8` (SST1 granularity); VSA-100's texBaseAddr drops bits [3:0] (16-byte `SST_TEXTURE_MUNGE_ADDRESS`) → every texture sampled 8 B = 4 texels below its download address → +4-texel S-shift → per-glyph sub-rect quads clipped the shifted strokes = "sliced" text. Fix: heap start +16, cdrs ramp at addr−16, all alloc lengths rounded to 16. + optional `RETRO3DFX_NODITHER`. | 09ef1e1 | n/a (quality) | gfix case A: **0.0% diff vs GDI-Generic oracle** (was 36.3%); live Q3 menu clean on monitor. Ends the multi-session garble hunt — it was never texcoords/filtering/postfilter; it was the heap base. Also exposed the **game-local stale-DLL deploy trap**: games load `<gamedir>\opengl32.dll` before system32 — every deploy must sweep ALL game-local copies. |
| **0.3.2** | **CS palette colors fixed** — `sst_ctable.c __glSSTColorTableEXT` read the GL_RGBA palette with stride 3 → `GR_TEXTABLE_PALETTE` entries shifted 1 byte/index. Only GoldSrc uses EXT_paletted_texture (Q3 doesn't) — why only CS was wrong. | (post-09ef1e1) | n/a (quality) | CS world went black→rendered (the "black world" had been the stale game-local ICD). gfix case I = palette regression gate; cases G/H archived. |
| **0.3.3** | **CS fps 3× fix** — GoldSrc re-MakeCurrents constantly; each LoseCurrent did `grSstWinClose` + full `grSstWinOpen` on the next MakeCurrent (70-600 ms fullscreen mode-set per switch). Fix: defer the close, reuse the Glide context on same hwnd+res (close only on window change/DestroyContext); made all 4 exposed PFDs double-buffered (single-buffer pick made wglSwapBuffers early-return → front-buffer rendering). | (committed 2026-07-20) | CS de_dust: ~33 → **99.9 fps steady** | Zero texture churn after. The all-double-buffered PFD change is also what makes fbdump work for every game. |
| **0.3.4d** | **★ CS GREEN WORLD SOLVED** — stale **2PPC** (2-px/clock, Glide's SINGLE-texture opt; `combineMode` bit-29 `SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK`). GoldSrc alternates single-tex (HUD, 2PPC ON) and dual-tex (world+lightmap, 2PPC must be OFF) per frame; our combine-word cache skipped the Glide combine re-issue when words were unchanged → `tmuConfig` never invalidated → `_grValidateTMUState`/`_grTex2ppc` never re-ran → both VSA-100 chips mirrored one TMU program → only the RGB565 green field survived: `(0,G,0)` world, distance-dependent. Fix (sst_export.c SwapBuffers): once/frame re-issue the FULL TMU1 state — grTexSource(latched first large-565 TMU1 texture) + grTexCombine + grColorCombine + grAlphaBlendFunction — forcing complete TMU re-validation on the next world draw. | 71db1e3 | CS de_dust **green 0/4**, perfect render; Q3 68.5-73.6 (no regression) | Bisect ledger: grTexSource alone=green; combines alone=4/6; combines+`__glSSTResetCombineCache()`=4/6 (the reset WIPES ext/overbright state — do NOT use at swap); all four together=**0/4 ✅**. Q3 never latches the dual-tex capture → no-op there. Failed attempts (documented in CS-GREEN-WORLD-LOG.md): topology-count cache reset (GoldSrc never flips the enabled-unit count), swap-time cache reset alone. |

| **0.3.5** | **Fix the 0.3.4d Q3 regression** (user-caught on monitor: white menu text, green-tinted logo, black world at 1024×768; 0.3.4d's swap hook fired in Q3 because Q3's single texture unit maps to **GR_TMU1** on the 2-TMU config — the `__r3d_blitValid` latch is NOT GoldSrc-specific — and the per-frame combine override killed vertex-color modulate; the latched texture address also went stale across mode changes). Changes: (1) hook now gated on `__r3d_sawTMU0` — GR_TMU0 (lightmap unit) sourced THIS frame = genuine dual-texture; Q3 never sources TMU0 → hook provably inert; flag cleared every swap. (2) `__r3d_blitValid` reset at both grSstWinClose sites (context-lifetime latch). (3) After the four Glide calls, `__glSSTInvalidateCombineWords()` — a words-only cache invalidation so the app's next combine re-issues and restores its own state (NOT the full ResetCombineCache that wipes ext/overbright — the attempts-2/3 failure). | (this commit) | Q3 menu text red again / 1024 world renders / CS green still 0 (verifying) | LESSON: "verified" fbdump captures must be checked for COLOR fidelity, not just structure — my 0.3.4d Q3 menu capture showed white text and I called it correct; stock Q3 menu text is red. TRUST THE USER'S EYES. |

## Overnight verification matrix (2026-07-20/21, ICD 0.3.4d)

Captured via **FBDUMP** (`C:\icd_fbdump.on` → grLfbReadRegion), the only truthful
capture — GDI SCREENSHOT of fullscreen Glide is always scanline-garbled (cannot
BitBlt the Voodoo surface; garbled GDI ⇒ game IS on the hardware path).

| game | result | evidence |
|------|--------|----------|
| Quake 3 | ✅ 68.5 fps timedemo four, menu crisp | /tmp/overnight/q3.png |
| Quake 2 | ✅ 135.6 fps demo1, console crisp, GL_RENDERER 0.3.4d visible | /tmp/overnight/q2.png |
| CS 1.6 | ✅ de_dust perfect tan — **green gone** | /tmp/overnight/cs.png |
| UT GOTY | ✅ renders via OpenGLDrv=our ICD (city intro correct) | /tmp/overnight/ut.png |
| RTCW | ✅ menu renders on hw (single-buffered ctx ⇒ no fbdump — capture limitation only) | rtcw_final_gdi.png |
| MOHAA | ❌ SafeDisc "Cannot locate the CD-ROM" modal pre-render — DRM, not driver; needs owned disc image | mohaa_gdi.png |

Operational gotchas hit: UT/RTCW detect unclean exits (`taskkill /f`) and block the
next launch (UT "Recovery Mode" dialog — click through; RTCW stalls pre-GL); RTCW
crashed once in glide3x (NULL deref) under kill/relaunch cycling; **PowerStrip
autostart popped a "Trial Expiration" modal that aborted the first 3DMark2001
fullscreen run** — Run-key value renamed to `PowerStrip.disabled-for-benchmarks`.

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
