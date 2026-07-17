# 06 — OpenGL, MiniGL, and the Other API Layers

## 1. The OpenGL ICD (`SWLIBS/OPENGL/`)

3dfx's full OpenGL implementation, shipped as an ICD (`3dfxogl.dll` era). Three parallel source
trees exist (historical layering):

| Tree | Back-end | Status |
|---|---|---|
| `SRC/` | `DRIVERS/SST` → **Glide 2** (+ `DRIVERS/DDRAW`, `DRIVERS/S3` experiments) | Original codebase |
| `GLIDE2X/SRC/` | Glide 2 | Productized variant |
| `GLIDE3X/` | **Glide 3** | The current one for H5-era boards — build this |

### Structure (per tree; GLIDE3X shown)

| Dir | Contents |
|---|---|
| `GLCORE/` | API core: dispatch tables (`G_API.C`, `G_LTAB.C`, `g_noptab_icd.c`), immediate-mode geometry (`GEOM_OG.C/.AG`), display lists (`DL_*.C` — heap, blocks, **`DL_OPT.C`/`DL_MOPT.C` display-list optimizer/compiler**, pool lists), `G_VARRAY.C` vertex arrays, pixel paths (`PX_*.C`), eval/feedback/select, `FASTDRAW.C` |
| `RASTER/` (SRC/GLIDE2X) | Rasterization state → Glide translation, span fallbacks |
| `SST/` | The Glide back-end: `sst_contxt.c`, `SST_PRIM.C` (points/lines/tris → `grDraw*`), `SST_TEX.C`+`SST_MTEX.C` (GL texture objects → TMU memory, multitexture), `sst_vertex.c`, `SST_OG.C` (optimized geometry pipe), `sst_clear.c`, `sst_depth.c`, `SST_PICK.C` (proc picking = per-state fast paths), `sst_pgmode.c` |
| `WGL/` | ICD plumbing: `WGLCMDS.C` (dispatch to MS opengl32 vs internal, driver-rename detection "opengl32.dll"), `WGLCOMM.C`, pixel formats, `WGLFONTO.C` outline fonts |
| `MGL/` | MiniGL-compat shim inside the ICD tree |
| `TRACE/` | GL call tracer |
| `GENERATE/` + `TOOLS/AG/` | Code generators — `.AG` files generate the repetitive per-format/ per-state C code (the `GEOM_OG.AG` → `GEOM_OG.C` pattern). Regenerate rather than hand-edit |
| `INCLUDE/GL/`, `INCLUDE/MAKE/` | GL headers + make fragments |
| `ICDDIST/` | ICD installer + VxD glue |

### Support / QA around the ICD

- `GDIBYP/` — **GDI bypass**: hooks the display driver so windowed GL can blit without GDI
  (`ALTGDI.C`, DPMI ring transition) — the trick that made windowed 3dfx GL usable.
- `SYSTRAY/` — the 3dfx tray tool (settings toggle).
- `BENCH/` — `TRISPEED` (tri throughput), `bindspeed` (texture-bind cost), `QUAKE.XLS`
  (Quake timedemo tracking sheet — their real-world metric).
- `CONFORM/` — official OpenGL conformance harness (+ `covgl`, `covglu`, `primtest`).
- `OGTST/` — in-house regression suite, organized by GL area (`FUNC/COORDS`, `FUNC/PRIMS`,
  `FUNC/TEXTURE/MIPMAPS`, `FUNC/framebuffer`, `ENV/` per-environment golden configs...).
- `EXAMPLES/`, `TESTS/`, `Docs/`, `SCRSAVE/` (3D screensavers incl. 3dfx logo).

## 2. MiniGL (`SWLIBS/3DFXGL/`)

The small, fast GL subset created for glQuake and its descendants (`fxgl`): `LIBGL/` implements
the exact entry points id-tech titles used (immediate mode, texture objects, no display lists
worth noting) straight onto Glide — far less overhead than the full ICD. `GLUT/`, `PROGRAMS/`
(demos: atlantis, glutmech, walker…), `STATUE/`, `TD/` sample apps, `microsoft/` headers.
Historically this is the driver Quake/Quake2/Half-Life(GL) users ran; benchmark parity with it
matters for any new driver.

## 3. Direct3D

Documented with the OS drivers ([05-OS-DRIVERS.md](05-OS-DRIVERS.md) §1) since it lives inside
them. Summary: DX6/DX7 HAL with SIMD software T&L (SoA x87 / 3DNow! / SSE), assembly PKT3
submission, FXT1/DXT texture support, DP2 token-stream execution, DX8 entry stub only.

## 4. RAVE (Mac) — `H5/MacOS8/RAVE/`

Apple QuickDraw 3D RAVE driver over the Mac Glide port — the Mac equivalent of a MiniGL
(games: Quake Mac ports, Unreal Mac used RAVE). `SOURCE/`, `HEADERS/`, `resources/`.

## 5. ATB — Arcade ToolBox (`SWLIBS/ARCADE/`)

Quantum3D's middleware for arcade/sim developers: a full **software T&L + clip + scene
pipeline on top of Glide** (matrix stacks, lighting, materials, LOD, morphing, MultiGen `.flt`
import via the `MULTIGEN/API`), with pluggable render back-ends (`SRC/DRIVERS/{GLIDE,D3D,RAVE,
DTRI}`). Modules: `ATC` (core/scene), `ATG` (geometry math), `ATM` (materials), `ATR`
(render), `ATD` (display), `ATI` (input), `ATN` (networking, + `ATBNET` IPX/UDP), `ATS`
(sound), `ATU` (util), `ATA` (audio). Docs: `ATPRGRMG.DOC` programming guide, `ATREFMAN.DOC`
reference, design docs per module. Useful today as a worked example of an optimized fixed-
function T&L pipeline tuned for exactly this hardware's submission model.

## 6. Content & tooling APIs

- **TEXUS/TEXUS2** (`SWLIBS/TEXUS*`): texture pipeline — `.3DF`/`.TXS` formats, mipmap
  generation (box/triangle filters, gamma-aware `CLAMP.C`/`DIFFUSE.C` dithering), palette
  quantization (`PAL256`, `PAL6666`), **NCC** narrow-channel compression (YIQ decomposition,
  `EIGEN.C` PCA, `NCCNNET.C` neural-net trainer), and in TEXUS2 the **FXT1** block codec
  (`CODEC.C`, `BITCODER.C`, four block modes, `SST2FXT1.H` bit layouts). CLI in `CMD/`.
  FXT1 was open-sourced by 3dfx separately; this is the reference encoder.
- **GlideTrap/GlidePlay** (`SWLIBS/GlideTrap`): API-stream capture/replay (see
  [08-DEBUGGING-AND-DIAGNOSTICS.md](08-DEBUGGING-AND-DIAGNOSTICS.md) §4).
- **Splash** (`SWLIBS/3DfxSplash`, `NuSplash`): logo animation data + 3DS MAX export plugin,
  played by Glide at init (`GSPLASH.C`).
- **GAMEGEN**, **3DSR4**, **MRI**: MultiGen model conversion, 3D Studio R4 driver, I2C ref.
