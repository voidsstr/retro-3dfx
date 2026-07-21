# CS 1.6 / GoldSrc green-world bug — investigation log

**Target:** .143, Voodoo5 5500 (2× VSA-100), Win XP SP3. Our ICD (leaked 3dfx
H5/Napalm Glide3 + Mesa-derived OpenGL ICD in `SWLIBS/OPENGL/GLIDE3X`).

**Symptom:** Counter-Strike 1.6 (GoldSrc) world/brush surfaces render
GREEN-tinted — pixel value ~`(0, G, 0)` (R and B lost, only the RGB565 green
field survives). Distance/LOD-dependent (near/LOD-0 green, far/small-mip
correct). Quake 3 on the identical driver is perfect.

---

## ✅ ROOT CAUSE (found 2026-07-20): stale Glide pipeline state in the 2-TMU (2PPC) path

**The green is NOT a texture data or format bug.** It is a **stale Glide
hardware state** that is wrong for GoldSrc's simultaneous dual-TMU
(multitexture) draws and is corrected by **re-issuing pipeline state once per
frame**.

### The decisive experiment (read-back blit)
A swap-time blit that (a) re-issues `grTexSource`+`grTexCombine`+
`grColorCombine`+`grAlphaBlendFunction` and (b) draws the saved world texture
to a screen corner — **completely fixes the green**: de_dust renders perfect
tan sandstone with correct lighting. The corner read-back showed the world
texture's TMU memory is **correct tan** (not green). Bisect: **state re-issue
alone (no draw) fixes it** → it's the state calls, not the draw.

### Why Q3 is fine, CS is green
- **Q3 = single-texture** (world on one TMU, 2-pass lighting; `t1=-1x-1` in
  every draw). Never enters 2PPC dual-TMU mode.
- **CS = simultaneous multitexture** (world 565 on GR_TMU1, lightmap 4444 on
  GR_TMU0, MODULATE×MODULATE). Triggers Glide `mode2ppc` (the two VSA-100 chips
  act as TMU0+TMU1 for one pixel stream).

### The suspect code (GTEX.C grTexSource, ~line 2916)
```c
if(!gc->state.mode2ppc || (tmu == gc->state.mode2ppcTMU)) {
    ... write textureMode/tLOD/texBaseAddr to hardware for `tmu` ...
    if(gc->state.mode2ppc) { ...copy this tmu's values into 1-tmu's shadow... }
} else {
    INVALIDATE_TMU(tmu, textureMode);   /* deferred write only */
    INVALIDATE_TMU(tmu, texBaseAddr);   /* NOTE: does NOT invalidate tLOD */
}
```
In 2PPC mode grTexSource writes hardware **immediately only for
`mode2ppcTMU`**; the other TMU (where the world texture may live) only gets its
shadow **invalidated** (deferred to the next `_grValidateState` at draw time),
and the else-branch invalidates `textureMode`/`texBaseAddr` but **not `tLOD`**.
Leading hypothesis: the deferred flush leaves one VSA-100 chip's world-TMU
register stale, so that chip samples the 565 world texture through a wrong
colorpath → the `(0,G,0)` green field. Re-issuing `grTexSource` re-dirties and
re-flushes it → fixed.

*(Bisecting whether grTexSource alone is the fix; a web research agent is
studying the exact mode2ppc register-write path for the precise line + fix.)*

---

## What was RULED OUT (each with an isolated on-hardware test)

| Hypothesis | Test | Result |
|---|---|---|
| Texture format register wrong (565 read as 4444) | logged HW format at draw time | **565 correct, 170/170 world draws** — NOT format |
| Texture DATA wrong in memory | read-back blit of world tex | **tan/correct** — memory fine |
| Our RGBA→565 conversion | shadow-buffer dump | correct tan (`0xe5b1`→230,182,139) |
| Mixed-format mip chain (MesaFX bug) | per-level ifmt capture | **0/144 mixed** — uniform 565 |
| Mip chain / LOD | driver nomip + `gl_texturemode GL_LINEAR` | still green |
| Texture size / picmip | `gl_max_size 64`, `gl_picmip 3`, `gl_round_down 0` | all still green |
| Neighbour memory overlap | 4× over-reservation of every texture | still green |
| Eviction under pressure | 300-texture gfix repro | tan/correct |
| Dual-TMU + 4444 lightmap + pressure | 240-texture multitex gfix repro | tan/correct |
| Format clobber by decal on same TMU | last-sourced-format tracking | no clobber |
| MOD² combine / lightmap TMU | driver single-texture force (nomt) | still green (memory, not combine) |

**Not reproducible synthetically** — every gfix isolated case renders correct;
only the live full GoldSrc dual-TMU pipeline shows it. That pointed at live
pipeline *state*, which the read-back experiment confirmed.

---

## Fixes already SHIPPED for CS during this investigation
- **Text garble** — 16-byte Napalm texture-heap alignment (ICD 0.3.1).
- **Palette/green-decal colors** — EXT_paletted_texture RGBA table stride (0.3.2).
- **Low fps** — Glide context reuse + all-double-buffered pixel formats (0.3.3);
  measured steady ~100 fps.

## Tooling built
- `optimized/gltest/gloop.py` — one-command deploy→run→fbdump→verify.
- **FBDUMP** (`C:\icd_fbdump.on`) — reads the real HW framebuffer via
  grLfbReadRegion (self-service capture, no human/monitor needed).
- Diagnostic markers: `icd_texblit.on` (read-back blit), `icd_nomt/nomip/heap8/
  bigalloc.on`, `icd_perf/verbose/prof.on`, `icd_trace`. All gated, inert by
  default. Full map in `optimized/DIAGNOSTICS.md`.
- Research: `optimized/research/` (MesaFX fxddtex.c/fxsetup.c reference).

## Next
1. Narrow to the exact state call (grTexSource vs combine) — bisect running.
2. Implement the proper fix in GTEX.C's mode2ppc path (write both chips'
   TMU registers, or force the deferred flush to cover both) — NOT the
   per-frame blit hack.
3. Verify with gloop + the full Q3/Q2/UT/RTCW matrix (no regression).
