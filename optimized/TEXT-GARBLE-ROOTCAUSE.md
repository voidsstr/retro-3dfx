# 2D text garble — ROOT CAUSE FOUND (2026-07-19)

The Q3 menu (and CS 1.6 HUD/console) "garbled text" — regular **vertical slices
through glyph strokes** on the proportional UI font — is **minification aliasing
of the non-mipmapped font atlas**. Proven by isolating it in a standalone GL
probe (`gltest.exe`, built with our ICD as opengl32) that renders known patterns
and dumps glReadPixels.

## The proof (gltest cases, all read back via glReadPixels on .143)
Using the REAL Q3 `font1_prop` atlas (dumped from pak0 as the app uploads it —
full 8-bit soft alpha), red-modulate + SRC_ALPHA blend, immediate-mode quads:

| case | what | result |
|------|------|--------|
| 22 | real font, **1:1** scale | **CLEAN** solid glyphs |
| 24 | real font, 1:1, + Q3 dropshadow (black@+2 then red) | **CLEAN** (dropshadow exonerated) |
| 21 | real font, **0.74× minified**, GL_LINEAR (bilinear) | **SLICED** |
| 23 | real font, 0.74× minified, **GL_NEAREST** | **SLICED** (≈ identical to 21) |
| 06 | 1px-column checker (hard alpha=255), 0.74× | clean (hard-alpha content doesn't expose it) |

Same code, same font, **only the scale differs between 22 (clean) and 21
(sliced)** ⇒ minification is the trigger. 21≈23 ⇒ **filter-independent**: our
BILINEAR minification behaves like POINT (drops columns) because VSA-100 bilinear
only samples a 2×2 texel neighborhood — insufficient for the minification ratio,
so thin vertical strokes of the soft-alpha glyphs alias out. The real menu draws
the proportional font minified, so it slices; 3D world + fixed HUD bigchars are
drawn ≈1:1 so they're clean (matches the long-standing "only font1_prop" symptom).

## What was ruled OUT this pass (each built + deployed + captured)
- **Texel-center / half-texel convention**: gltest calibration showed our
  sampling is GL-conformant (case 03 fractional 1:1 = [141,111] = exact bilinear;
  a −0.5-texel bias BREAKS 1:1 to 50/50 gray). Removed the earlier speculative
  half-texel hack; biases are 0. NOT the cause.
- **asm vs C vertex path**: a full `GL_NOASM` (uniform C) ICD sliced identically
  (pixel-diff 37k px, same pattern) ⇒ not the multi-path divergence.
- **32-bit / dithering**: it's geometric slicing, not colour banding.
- **Dropshadow / draw order** (case 24 clean), **strip vs VA vs immediate**
  (cases 17/18/19 clean), **vertex texcoords** (traced final GrVertex stream:
  correct positions/texcoords).

## ⚠️ UPDATE (later same session): minification DISPROVEN as the live cause
Implemented the mip fix below (box-filtered chain + GR_MIPMAP_NEAREST_DITHER,
env RETRO3DFX_FONTMIP; confirmed engaged: the 256² ARGB_4444 font gets
mipWords=87381). **The live Q3 menu was UNCHANGED** — still sliced, identical.
That means the hardware samples LOD 0 (the font is NOT minified in the live
menu), so the case-21 0.74× slice was a *different* phenomenon that merely looks
similar. Further isolation:
- case 26: real font, **1:1, via glDrawElements (VA path = what Q3 uses)** = CLEAN
- so immediate (22), VA (26), and dropshadow (24) are ALL clean at 1:1.

**Current standing contradiction:** every isolated render at 1:1 is CLEAN, yet the
live menu (vertex trace says ~1:1: glyph x=222..245=23px from s=230..253=23 texels)
SLICES. The trigger is something in Q3's *actual* per-glyph state not yet
replicated — prime remaining suspects: **fractional screen positions** combined
with **real variable-width glyph sub-rect texcoords** and the full atlas (the exact
`UI_DrawProportionalString` recipe; the case-20 full replication crashed and needs
redoing). NOT filter, NOT mips/minification, NOT dropshadow, NOT VA-vs-immediate,
NOT asm-vs-C, NOT texel-center. Box left on shipped 0.2.2; the env-gated mip code
is harmless (off by default) and kept for the eventual real-minification cases.

## The (candidate, now deprioritized) minification fix — for reference
The font atlas is a single-LOD (non-mipmapped, GL_LINEAR) 256² texture; VSA-100
has nothing to down-filter with at minification. Options, in order of preference:
1. **Driver-side mip generation + LOD blend**: build a box-filtered mip chain for
   2D textures and enable a blending minification mode. NOTE: `GR_MIPMAP_NEAREST`
   at 0.74× still selects LOD 0 (log2(1.35)=0.43 rounds to 0) so it does NOT help
   mild minification — needs **`GR_MIPMAP_NEAREST_DITHER`** (single-TMU trilinear
   approximation, LOD-dithered blend) or true trilinear across 2 TMUs. Our ICD
   currently maps `GL_LINEAR_MIPMAP_LINEAR → GR_MIPMAP_NEAREST` ("XXXshui
   unsupported by sst", SST_TEX.C:666) — wire the dither/trilinear path.
2. This is what MesaFX effectively does (its 2D text is clean), so adopting the
   finished-driver behaviour for the 2D case is legitimate (our H5 source is
   unreleased/in-progress).

Verification harness is ready: `toolchain-3dfx/prefix/drive_c/3dfx/gltest/`
(gltest.c, embeds the real font rows in fontrows.h) — cases 21 (must go clean)
vs 22 (must stay clean) are the regression gate; then re-capture the live Q3 menu.

Box left on shipped ICD 0.2.2 (6b8182dd). Cross-ref: RESEARCH-2D-TEXT-vs-MESA.md
(earlier screen-space-rasterizer hypothesis — now superseded by this isolation).
