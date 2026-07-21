# CS 1.6 / GoldSrc green-world bug — investigation log

**Target:** .143, Voodoo5 5500 (2× VSA-100), Win XP SP3. Our ICD (leaked 3dfx
H5/Napalm Glide3 + Mesa-derived OpenGL ICD in `SWLIBS/OPENGL/GLIDE3X`).

**Symptom:** Counter-Strike 1.6 (GoldSrc) world/brush surfaces render
GREEN-tinted — pixel value ~`(0, G, 0)` (R and B lost, only the RGB565 green
field survives). Distance/LOD-dependent (near/LOD-0 green, far/small-mip
correct). Quake 3 on the identical driver is perfect.

---

## ✅✅✅ FIXED — SHIPPED (2026-07-20, ICD 0.3.4d)

**Result: green 0/4, de_dust renders perfect tan sandstone** (samples
`(180,149,106) (205,165,123) (197,157,90) (180,153,123)`, screenshot
`/tmp/gloop_cs.png` — correct walls/ground/sky/crates, no green, no garble).

**The fix (SwapBuffers, `sst_export.c`):** once per frame, re-issue the FULL
TMU1 pipeline state so the next world draw forces a complete TMU re-validation
(which re-runs `_grTex2ppc` and clears the stale bit-29):
```c
grTexSource( GR_TMU1, <last-565-world-tex>, GR_MIPMAPLEVELMASK_BOTH, &ti );
grTexCombine( GR_TMU1, LOCAL, NONE, LOCAL, NONE, F, F );
grColorCombine( SCALE_OTHER, ONE, LOCAL_NONE, OTHER_TEXTURE, F );
grAlphaBlendFunction( ONE, ZERO, ONE, ZERO );
```
Gated on `__r3d_blitValid` (the first large 565 TMU1 texture, latched in
`__r3dLogTexSource`, `SST_TEX.C:100`). **No `__glSSTResetCombineCache()`** — that
call (attempts 2 & 3) wipes the ext-combine/overbright state every frame and
reintroduces green (4/6). The grTexSource address is only used to *dirty* the
TMU1 registers; GoldSrc re-binds its own texture/combine per surface before the
world draw, so lighting/texture are preserved. Q3 (single-texture) never latches
`__r3d_blitValid`, so the block is a no-op there — zero regression.

**Bisect ledger (what actually gated it):**
| set re-issued at swap | green |
|---|---|
| grTexSource ALONE | still green |
| combines ALONE (grTexCombine+grColorCombine) | 4/6 |
| combines + `__glSSTResetCombineCache()` (0.3.4c) | 4/6 |
| **grTexSource + combines + grAlphaBlend (0.3.4d)** | **0/4 ✅** |
| full state, no draw (diagnostic 0.3.4) | 0/4 ✅ |

grTexSource invalidates `textureMode`/`texBaseAddr`; the combines invalidate
`combineMode`/`tmuConfig`. Neither alone forces the 2PPC re-eval — both are
needed so validation runs end-to-end on the next world draw.

---

## ✅✅ CONFIRMED ROOT CAUSE + FIX (2026-07-20, ICD 0.3.4)

**2PPC ("2 pixels per clock") is Glide's SINGLE-texture optimization** (both
VSA-100 chips gang up on one pixel; the two TMU register sets are forced
identical and "any write to one TMU is mirrored to both"). It is ON for
single-texture draws (Q3 the whole time; GoldSrc HUD/sprites) and MUST be OFF
for a genuine dual-texture world+lightmap draw. My earlier framing had this
backwards — that inversion hid the bug.

**The bug:** Glide re-evaluates 2PPC only inside TMU *validation*, and the draw
path skips validation when `gc->state.invalid == 0` (`GR_FLUSH_STATE()`,
fxglide.h). GoldSrc alternates single-texture (HUD, 2PPC ON) and dual-texture
(world, 2PPC must be OFF) each frame. Our ICD caches the combine words
(`s_TC0/s_TC1/s_CC/s_AC`); when the world's combine matches the previous
frame's world combine, we **skip `grTexCombine`/`grColorCombine`**, so
`tmuConfig` is never invalidated, so validation is skipped, so **stale 2PPC
bit-29 (`SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK`) leaks into the dual-texture world
draw** → the two TMUs' independent world+lightmap programmings mirror together
→ the `(0,G,0)` "green channel only" world. (Format bits are a separate field,
which is why the format register reads correct.) Q3 never flips topology, so it
never desyncs. Web research pinned it in the H5 Glide source: `_grTex2ppc`
(gtex.c) is the only bit-29 writer and runs only from `_grValidateTMUState`.

**Bisect that proved it:** re-issuing the COMBINE once per frame fixes it
(0/4 green); re-issuing grTexSource ALONE does NOT (grTexSource invalidates
`textureMode` but not `tmuConfig`, so it doesn't force 2PPC re-eval).

**Fix attempt 1 (FAILED):** reset the combine cache on active-TMU *count*
change in `__glSSTLoadCombineFunction`. Still green — GoldSrc keeps both units
enabled and only changes the combine, so the count never flips; the detection
never fired.

**Refined mechanism (why the blit worked):** GoldSrc DOES call the combine
setup every frame (proven: the blit's passthrough combine was overridden by the
game's real combine + lighting the next frame). But our `s_*Word` cache SKIPS
the actual `grColorCombine`/`grTexCombine` Glide call when the words are
unchanged frame-to-frame — so bit-29 is never re-cleared. The blit worked by
*changing* the cached words (passthrough), forcing the game's next combine to
genuinely re-issue.

**Fix attempt 2 (SwapBuffers):** call `__glSSTResetCombineCache()` once per
frame at swap. This clears `s_*Word`, so the app's per-frame combine call
actually re-issues to Glide, invalidating `tmuConfig` → forcing `_grTex2ppc`
re-eval → 2PPC OFF for the world draw. Once-per-frame, cheap, Q3 unaffected.
*(Testing…)*

---

## (superseded framing) stale Glide pipeline state in the 2-TMU (2PPC) path

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

### STRONGEST candidate fix (found in source, GTEX.C `_grTex2ppc`, line 958)
`_grTex2ppc(enable)` toggles 2PPC. It invalidates the TMU texture registers
**only when DISABLING** 2PPC (lines 988-995):
```c
if(!enable) {                                   /* <-- only on EXIT */
    INVALIDATE_TMU(GR_TMU0, textureMode); INVALIDATE_TMU(GR_TMU0, texBaseAddr); ...
    INVALIDATE_TMU(GR_TMU1, textureMode); INVALIDATE_TMU(GR_TMU1, texBaseAddr); ...
}
```
When **ENTERING** 2PPC the TMU texture registers are NOT invalidated/refreshed,
so a chip's world-TMU `textureMode`/`texBaseAddr` keeps a stale value from
before the mode switch → wrong colorpath → `(0,G,0)` green. Q3 never toggles
2PPC (single-texture only), so it's never stale.
**Candidate fix:** invalidate the TMU texture registers on 2PPC **enable** too
(make the `if(!enable)` block run on both transitions), forcing a clean
re-flush of both chips' TMU registers when multitexture begins. Matches: the
per-frame grTexSource re-issue (blit) works by re-dirtying the same registers.

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

## Bisect progress (which of the 4 re-issued calls fixes it)
- Full set (grTexSource+grTexCombine+grColorCombine+grAlphaBlend): **fixes** ✅
- State-only, no draw: **fixes** ✅ (it's the state, not the draw)
- grTexSource ALONE: **does NOT fix** (still green) → not (just) the texture register
- grColorCombine+grTexCombine (combines only): *testing…*
→ Points at the **combine** (color/tex) being the stale 2PPC state, matching
the `(0,G,0)` "green channel only" = a wrong colorpath on one chip.

## Next
1. Confirm combine is the fixer (bisect running); if so, the fix is in the 2PPC
   combine broadcast (`_grTex2ppc` / grColorCombine / grTexCombine two-chip
   programming), not grTexSource.
2. Implement the proper fix in GTEX.C's mode2ppc path (write both chips'
   TMU registers, or force the deferred flush to cover both) — NOT the
   per-frame blit hack.
3. Verify with gloop + the full Q3/Q2/UT/RTCW matrix (no regression).
