# CS/GoldSrc green-world bug — research + diagnosis log

## Research finding (web, MesaFX/leaked-Glide lineage)
The MesaFX 3dfx driver (fxddtex.c) has a single per-texture Glide format
field `ti->info.format` that is **overwritten on every glTexImage2D with the
current level's format** (fxddtex.c:1482,1705), while `grTexSource` binds ONE
format for the whole mip chain (fxsetup.c:395,427,863,869). GoldSrc uploads
mips largest-first; if levels disagree in format the chain is misread. The
"565 data sampled as 4444 → green" signature matches.

- GR_TEXFMT enums: P_8=0x5, RGB_565=0xa, ARGB_1555=0xb, ARGB_4444=0xc.
- 565-as-4444 skews green: 565's 6-bit green [10:5] smears into 4444 R/G.
- GoldSrc world textures: paletted (GL_EXT_paletted_texture / P_8) when the
  driver advertises it; else GL_RGB/565. Per-level glTexImage2D, own mipmaps
  (largest->smallest). HUD/sprites/decals = RGBA -> 4444. Sky = separate path.
- cvars: gl_texturemode, gl_round_down, gl_picmip, gl_max_size, paletted toggle.
- Retail ICD kept ONE consistent format per texture's whole chain.

## What we PROVED on .143 (V5 5500)
- Our 565 shadow data is correct tan (0xe5b1 -> RGB 230,182,139).
- Green is per-texture, LOD-0/near (nomip = all green), CS-only.
- Q3 = single-texture (TMU1 only); CS = simultaneous dual multitexture.
- CS mip chains are NOT mixed-format (0/144); world=565, decals/lightmaps=4444.
- World (565) and single-level decals (4444) BOTH use GR_TMU1.
- Ruled out: format-chain-mix, MOD^2 combine, lightmap TMU, mip chain,
  16-byte len alignment, eviction (300 tex), dual-TMU+pressure (240 tex).

## Two live mechanisms, disambiguated by draw-time HW format register:
- register=4444 -> format clobber (565 read as 4444); fix = re-source per draw.
- register=565  -> LOD0 MEMORY holds wrong bytes (e.g. a 4444 decal texel read
  as 565 = also green); fix = allocation overlap (decal BASE vs world STACK on
  GR_TMU1). This cleanly explains LOD-0-only corruption.

## Tooling
- optimized/gltest/gloop.py: one-command deploy+run+fbdump+verify.
- FBDUMP (C:\icd_fbdump.on): reads real HW framebuffer (self-service capture).
- gfix cases A-T; diagnostic markers icd_nomt/nomip/heap8.on.
