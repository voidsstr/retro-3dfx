# Visual QA — Voodoo 5 6000 (.133), 2026-08-12

Method matters here. Three capture paths, only two of which tell the truth:

| capture | what it reads | trustworthy? |
|---|---|---|
| Q3 `+screenshot` (glReadPixels) | the ICD's render target | **yes** |
| ICD **fbdump** (`C:\icd_fbdump.on` → `C:\fbdump_NN.raw`, RGB565) | the ICD's render target, any GL app | **yes** |
| agent `SCREENSHOT` (GDI) | the scanned-out desktop surface | **no** for 3dfx fullscreen |

## Results

- `q3_q3dm1_INENGINE_GOOD.png` — Q3 3D world: correct textures, geometry, HUD.
  Matches the repo golden `quality_q3dm1_0.1.3.png` (mean abs diff 7.3/px).
- `q3_menu_INENGINE_GOOD.png` — Q3 2D/proportional font: crisp, no slicing.
  Matches golden `quality_q3menu_0.1.4_FIXED.png` (5.0/px). The historical
  text-garble bug is NOT present.
- `q2_FBDUMP_RENDER_IS_CLEAN.png` — Quake II via ICD fbdump: **renders correctly**
  (crates, walls, enemy, blood particles, weapon, HUD numerals).
- `q2_GDI_SCANOUT_IS_GARBLED.png` — the SAME Q2 session via GDI: horizontal
  banding with diagonal smearing.

## Conclusion

The user reported Q2 gameplay showing "garbled lines like smeared paint" on the
physical monitor. The fbdump proves **the ICD's rendering is correct**; the
corruption appears between the finished framebuffer and the monitor — i.e. the
**display/scanout path**, not the renderer.

That is the same class as the 2026-07-18 reframe in `../DEBUG-LOG.md`
("the DISPLAY/SCANOUT is broken, not rendering"), and it means the fix belongs in
the display driver / video path (`3dfxv5d.dll`, mode set, tiled-vs-linear scanout,
SLI band combining), NOT in the ICD or Glide.

Forcing a single chip (`FX_GLIDE_NUM_CHIPS=1`) did NOT clear it, so it is not
simply SLI band interleave. Disabling paletted textures did not clear it either.
