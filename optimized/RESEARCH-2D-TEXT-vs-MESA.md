# Research: 2D text quality — our H5-source ICD vs the open-source (MesaFX) driver

**Context.** The user reports the open-source Glide OpenGL driver (MesaFX / Mesa's
`fxdd` on the .124 Voodoo3 lane) renders Q3's 2D menu text noticeably better than
our driver, which shows **regular vertical black slices through the glyph strokes**
(both the enlarged highlighted item and normal-size items; the highlight overlay
also reads as "offset too far left"). Our codebase is 3dfx's **unreleased,
in-progress H5/Napalm source** — so the OSS driver, built on the older but
*finished* Mesa+Glide path, has fixes/design choices our snapshot never got.

This documents what was tested live (build → deploy → glReadPixels capture on
.143), what was ruled out, and concrete recommendations.

## Symptom, precisely characterized
- Regular **vertical black columns** cut through solid glyph strokes.
- Present at BOTH the enlarged highlighted item (~2x) and normal-size items (~1:1)
  → **not** a magnification/minification-only artifact.
- 3D world + weapon + the fixed-width HUD font (bigchars) render **clean**.
- Only the **proportional UI font** (`font1_prop`, tightly-packed variable-width
  glyph atlas) is affected.

## Architectural difference vs Mesa (the root of the divergence)
Our ICD is a **hardware T&L + Glide-vertex** design: GL → SST backend builds
`GrVertex` → `grDrawTriangle`/`grDrawVertexArray` → Glide3 trisetup → TREX. The
2D path shares this 3D pipeline (Q3 batches UI quads through the same tess/vertex
path). Key mechanisms:
- **Texcoord scale via a float-exponent trick** (`sst_vertex.c:114`):
  `sBias = bits(width2f) − bits(1.0)`, added to the texcoord's integer bits to
  multiply the normalized [0,1] coord by the (power-of-2) texture width. It is
  fast but can ONLY scale — it cannot add a constant, so the +0.5 texel-center
  offset is applied *separately* and INCONSISTENTLY (see below).
- **Half-texel** (`__glSSTHalfTexelS/T`, set in `SST_TEX.C:922-947`,
  `0.5/width2` for non-mipmapped 2D atlases) is added in only SOME of the ~15
  `CopyTexCoord*` variants (`S_VARRAY.C:321` `CopyTexCoord2f`, `S_TAPI.C:119`)
  and omitted in the compiled/JIT batch path Q3's world uses.

Mesa's `fxdd` instead does **software T&L into a uniform float `GrVertex`** with a
single, consistent texcoord scaling (`* width`) and a uniform pixel/texel-center
convention applied once — no multi-path exponent-trick, no per-variant half-texel
divergence. That uniformity is why its 2D text is clean.

## Ruled OUT (each built + deployed + captured on .143)
| hypothesis | change tested | result |
|---|---|---|
| hw texture filter wrong | honor GL min/mag → BILINEAR, both TMUs (0.2.0) | 3D minified surfaces improved; **2D slices unchanged** (2D draws ~1:1, point==bilinear) |
| half-texel value/sign | exaggerated `__glSSTHalfTexel` 0.5→4 texels | glyph **content shifted** but the **screen-space slices stayed put** |
| vertex pixel-center | re-enabled the commented `+__glHalf` viewport offset (`S_XFORM.C:557`), `SNAP_BIAS=0` | **no change** to slices |

The exaggerated-texel result is the key discriminator: texture content moved under
the slices while the slices held their **screen** position ⇒ the black columns are
**dropped destination columns in screen space**, i.e. a **triangle-rasterization /
subpixel-DDA** artifact, NOT a texture or texcoord error.

## Leading hypothesis (next to test): subpixel triangle setup on thin 2D quads
The Glide3 trisetup (`H5/GLIDE3/SRC` XDRAW2 asm / `grDrawTriangle`) computes edge
DDAs with a subpixel-bits convention (`__GL_DEFAULT_COORD_SUBPIXEL_BITS`, snap in
`S_PGDRAW.C __glSnapXLeft/Right`). For tightly-packed 2D glyph quads whose edges
fall near pixel boundaries, an off-by-one in the fractional x-coverage rule drops
thin destination columns — exactly the regular vertical slices, at any scale, only
where quads are thin/adjacent (proportional font), never on big 3D triangles.
Mesa's separate fill rule doesn't share this snap. **Candidate sites:**
`GLCORE/S_PGDRAW.C` (`__glSnapXLeft`/`__glSnapXRight`, "snap each y to pixel
center"), `SST/sst_globals.h:113 __GL_SST_SNAP_BIAS (0.0)`,
`H5/GLIDE3/SRC/XDRAW2.INC` (asm trisetup fractional area/cull).

## Recommendations (ranked)
1. **Instrument the trisetup fractional-x path for 2D quads** (oow≈1): log each
   quad's window x0..x1 and the snapped left/right span; a systematic 1-column
   loss vs the intended width confirms the DDA rule. Then adjust the snap
   convention (likely add a +0.5 subpixel bias in the x fill rule, or fix the
   `SNAP_BIAS`) — verify via capture (this IS capturable, unlike the scanout
   issues).
2. **Unify the texcoord half-texel across ALL paths** (correctness even if not the
   slice cause): apply `__glSSTHalfTexel` in the compiled `CompileElementsIndexed`
   texcoord copy too, or better, fold a real +0.5-texel add into the point where
   `sBias` scaling lands (`sst_vertex.c` `t0_sow`), matching Mesa's single uniform
   convention. Removes the multi-path divergence permanently.
3. **Port Mesa's 2D fill rule wholesale** if (1) proves the H5 trisetup snap is the
   culprit: Mesa's `fxdd` uses a simpler, proven pixel-coverage rule. Since our
   source is unreleased/in-progress, adopting the finished OSS convention for the
   2D case is legitimate.
4. Keep **0.2.0's filter fix** regardless — it's a real, free (0 fps) quality win
   on 3D minified surfaces, already shipped.

## What's already fixed this session (for the record)
- **Franken-stack**: .143 was running the in-box 2001 `3dfxvs.dll` display driver
  with our miniport; activated our `3dfxv5d` display driver (full our-stack now).
- **0.2.0 filter fix**: ICD now honors GL texture filters in hw (both TMUs);
  eliminated point-sampling shimmer on minified 3D textures; 76.5/76.0 fps (no
  regression). GL_RENDERER stamped `[retro3dfx 0.2.0]`.

Cross-ref: `DEBUG-LOG.md` (fast facts), `PROFILING-FINDINGS.md` (perf is
present-bound; quality is the lever).
