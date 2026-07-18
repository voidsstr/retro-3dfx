# Level-set research: VSA-100/Napalm stack & the path to mature drivers

Research (codebase + web) to ground the strategy for maturing the H5/Napalm 3dfx
drivers for XP/2000 on the Voodoo5 6000 (4× VSA-100, 256 MB).

## The hardware (web research)
- **VSA-100 "Napalm"** = 3dfx's final chip (2000). Voodoo4 4500 = 1 chip, Voodoo5
  5500 = 2, **Voodoo5 6000 = 4**, all @166 MHz. Each chip: 2 pixel pipelines, **1
  TMU each**, 32-bit datapath (vs 16-bit prior), larger texture cache.
- Multi-chip = **SLI by scanline bands**; the driver broadcasts the SSTCP packet
  stream to all chips (a register layout mismatch corrupts all chips identically).
- Glide texture note from the wild: "grTexDownloadMipMap: mipmap cannot span 2
  Mbyte boundary" — texture memory is banked in 2 MB regions; the driver's texture
  allocator must respect that (relevant to the 256 MB 6000).

## The texture pipeline (codebase — the exact register contract)
Hand-driving a textured triangle needs ALL of, in a valid+consistent set:
- **fbzColorPath**: `SST_RGBSEL_TREXOUT` (== TMUOUT, =1) to take color from the
  TMU, **plus `SST_ENTEXTUREMAP` (BIT 27)** — without the enable bit the sim
  literally errors "Using texture data with texturing disabled" (FBI.C:231).
- **textureMode**: format (`SST_RGB565`=10<<8) + the texture color-combine bits.
  Decal/replace (out = texel) = `SST_TC_ZERO_OTHER | SST_TC_ADD_CLOCAL`
  (== `SST_TC_REPLACE`, H3DEFS.H:364). The CCU (CSIM/CCU.C) computes
  out = (ZERO_OTHER?0:other)*MSELECT + (SUB_CLOCAL?local:0) + (ADD_CLOCAL?clocal:0).
- **tLOD**: `SST_TLOD_MINMAX_INT(lodmin,lodmax)` (4.2 fixed, LODMIN sh0 / LODMAX
  sh6) + aspect (`SST_LOD_ASPECT`, sh21) — the "ugly" encoding the ICD warns about.
- **texBaseAddr** (munged via `SST_TEXTURE_UNMUNGE_ADDRESS`; tiled bit BIT0),
  **trexInit0/1** (TREX hw enable — likely the remaining missing init),
  and the setup unit needs `SST_SETUP_ST0 | SST_SETUP_W0` with sSow0/sTow0/sOow0.
- Texture upload: write the TEX0 aperture (`SST_TEX0_OFFSET`=0x600000) via the
  store intercept `SET()` (32-bit = 2 RGB565 texels); direct pointer stores segfault.
- Texture mem and color buffer share board RAM — must not overlap (set colBufferAddr
  vs texBaseAddr to different offsets).

## What works in the sim now (durable)
Flat/Gouraud triangles render pixel-perfect (build->render->readback, seconds,
native -m32). Texture UPLOADS correctly. Untextured geometry renders. See
`csim-native/README.md`.

## ★ Strategic conclusion (the level-set)
**Hand-driving CSIM's texture registers is the wrong path** — the encoding is a
deep, interacting web (I hit upload-ordering, ENTEXTUREMAP, combine, TREX-init,
LOD/aspect munging, buffer overlap — each a separate debug). And it tests the
CHIP, not the DRIVER where the 2D-text column-drop bug actually lives.

**Correct path = glide-on-sim.** Build glide3x with `GLIDE_SST_SIM | HAL_CSIM`
linked to `libcsim.a` (the hooks already exist: `DISTRIP.C:180` `#if GLIDE_SST_SIM
#include csim.h`). Then a tiny Glide program calls `grTexDownloadMipMap` +
`grTexSource` + `grColorCombine` + `grDrawTriangle` — glide sets up ALL the texture
registers CORRECTLY, and it exercises the EXACT driver→chip path. That both (a)
avoids the register-archaeology rabbit hole and (b) reproduces the driver's real
behavior for the column-drop bug. This is the next major build step.

**Driver-maturation implications found along the way** (for the V5 6000 session):
- The `SST_FBI_BUSY`-never-clears-after-triangle behavior (idle spins) — verify the
  real driver's wait-for-idle doesn't hang on the 4-chip 6000.
- chipMask must equal the real chip count (6000 = 4 = 0xf).
- Texture allocator must respect the 2 MB texture-bank boundary on 256 MB.
