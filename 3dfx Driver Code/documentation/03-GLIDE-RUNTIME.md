# 03 — Glide Runtime Internals (Glide 3.x, with Glide 2.x deltas)

Primary sources: `H5/GLIDE3/SRC/` (107 files). Glide 3 is the runtime everything else builds on
(OpenGL ICD/MiniGL layer on it; the diagnostics and conformance suites exercise it; D3D is a
sibling, not a client). Glide 2 (`H5/GLIDE/SRC/`) is the same architecture one generation back —
differences at the end.

## 1. File-by-file

### API surface & context

| File | Purpose |
|---|---|
| `GLIDE.H`, `GLIDESYS.H`, `GLIDEUTL.H`, `G3EXT.H` | Public API, platform config (`GLIDE_PLATFORM`, `GLIDE_HW_*` capability macros), utilities, extensions (FSAA, texture-buffer, chromarange…) |
| `GSST.C` | `grSstWinOpen/Close`, `grSelectContext`, resolution/refresh negotiation, buffer allocation via MINIHWC, initial register load, SLI/AA context setup, `grGet`/`grSstStatus` |
| `GGLIDE.C` | `grGlideInit/Shutdown`, `grFlush`/`grFinish`, swap behavior (`grBufferSwap`, swap-pending throttle), global control |
| `DIGLIDE.C`, `DISST.C`, `DIGET.C` | "Device-independent" layers of the above (shared logic, board counting via `_grSstDetectResources`) |
| `GERROR.C` | Error callback plumbing (`grErrorSetCallback`) |
| `GBANNER.C`, `GSPLASH.C`, `FXSPLASH.H` | The 3dfx splash animation shown on `grSstWinOpen` (consumes `SWLIBS/*Splash` data; disable env `FX_GLIDE_NO_SPLASH`) |
| `GTHREAD.C` | Thin thread/TLS wrappers for the GC pointer (single-threaded model; critical section for windowed FIFO — `GR_WINFIFO_BEGIN`) |
| `G3DF.C` | `.3DF` texture-file loader (`gu3dfGetInfo/Load`) |
| `GU.C` | Glide utility entry points kept in 3.x |

### State management

| File | Purpose |
|---|---|
| `DISTATE.C` | **The state engine.** All `grAlphaBlendFunction`/`grDepthBufferMode`/`grFogMode`/etc. write shadow registers + dirty masks; `_grValidateState`/`GR_FLUSH_STATE` emits minimal PKT4 group writes. Two-level TMU shadowing (`tmuShadow` → `shadow.tmuState`); clip-window fast path writes `clipLeftRight/clipBottomTop` directly |
| `GSFC.C`, `GSFC.H`, `GSFCTABL.H` | Surface/format tables (color/depth formats, tiling arithmetic) |
| `DITEX.C`, `GTEX.C` | Texture state: `grTexSource`, `grTexCombine`, `grTexFilterMode`, `grTexMipMapMode`, TMU address-space management, LOD ranges, NCC/palette table downloads |
| `GTEXDL.C` | `grTexDownloadMipMap(Level(Partial))` — chooses per-CPU/format/width download proc, emits PKT5 TEXPORT streams; `texDownloads` stats |
| `DIGET.C` | `grGet`/`grReset` queries (num boards, memory, etc.) |

### Drawing

| File | Purpose |
|---|---|
| `GDRAW.C` | Public draw entries: `grDrawPoint/Line/Triangle`, `grDrawVertexArray(Contiguous)`; routes to `_grDrawPoints/_grDrawLineStrip/_grDrawTriangles` or AA equivalents |
| `GXDRAW.C` | **C triangle setup** (`GLIDE_USE_C_TRISETUP`): `_grTriCull` (sign-bit area cull), `internal_trisetup`, the 8 specialization wrappers `_trisetup_Default_{win,clip}_{cull,nocull}_{valid,invalid}` |
| `XDRAW2.ASM` / `XDRAW2.S` | x86 assembly triangle setup, default (non-3DNow!) build — same entry names, `_def.obj` |
| `XDRAW3.ASM` / `XDRAW3.S` (122 KB) | x86 **3DNow!** triangle setup + vertex-list walkers (`_3dnow.obj`); femms/pfmul-scheduled |
| `XTEXDL.ASM`, `xtexdl_def.c` | MMX/3DNow! texture download inner loops (`_grTexDownload_3DNow_MMX`) vs C fallback |
| `GSTRIP.C`, `DISTRIP.C` | `grDrawVertexArray` strip/fan logic — builds continuation PKT3s (`BDDDDD`), split at 15-vertex packet limit, handles cull-mode flips per winding |
| `GAA.C` | Antialiased point/line/triangle path (coverage-based edge AA — distinct from FSAA) |
| `GLFB.C` | `grLfbLock/Unlock/ReadRegion/WriteRegion` — direct linear-frame-buffer access, write-mode conversion, and PKT5 LFB writes when FIFO'd |
| `GXDRAW_PPC.C`, `XDRAWPPC.ASM`, `PPCDRAW2.S`, `gstrip_ppc.c`, `xdraw_ppc.c` (G2) | PowerPC (Mac) setup paths with `dcbf` cacheline flush FIFO variant |
| `GPCI.C` | `_GlideInitEnvironment`: env-var parsing, **CPU detection & proc-table selection** (§3), board detect; `_grSstDetectResources` |
| `CPUDTECT.ASM`, `CPUDTECT.S` | CPUID: vendor + family + MMX/3DNow! feature bits → `_GlideRoot.CPUType` |

### Command transport

| File | Purpose |
|---|---|
| `FIFO.C` | FIFO bring-up per context (`_grCommandTransportInit`), `_grCommandTransportMakeRoom` (wrap JMP planting, hw read-ptr poll with timeout diagnostics), windowed tail-chasing FIFO, SLI slave-FIFO sync, `_grHwFifoPtr`, debug write shims (`_grFifoWriteDebug`), `_grSet32/_grGet32`, fencing helpers, `grFence`/lost-context handling |
| `FXCMD.H` (68 KB) | **The transport macro library**: `GR_CHECK_FOR_ROOM`, `GR_SET_EXPECTED_SIZE`, `GR_CHECK_FOR_FENCE`, `GR_BUMP_N_GRIND`/`CHECK_FOR_BUMP`, `REG_GROUP_BEGIN/SET/END` (PKT4 builders), `TRI_STRIP_BEGIN`/`TRI_BEGIN`/`TRI_SETF(_CLAMP)`/`TRI_END` (PKT3 builders), `SET_FIFO`/`SETF_FIFO` per-platform store primitives, packed-RGB float-bias macros, FP denormal clamp |
| `FXGLIDE.H` (3,000+ lines) | **The GC** (§2), `_GlideRoot`, proc typedefs & dispatch tables, `P6FENCE`, stats counters, `GR_BEGIN_NOFIFOCHECK` entry prologue macros, debug gates |
| `FXINLINE.H`, `FXGASM.C/.H` | Inline helpers; `FXGASM.C` generates assembler offset constants (`fxgasm.h`) for the .ASM files — **must re-run when GC layout changes** |
| `GLIMPORT.ASM` | Import thunks |
| `FXBLDNO.C`, `RCVER.H`, `GLIDE.RC` | Build-number stamping + version resources |
| `QMODES.H`, `TV.H` | Quantum3D special modes, TV-out modes |

## 2. Core data structures (`FXGLIDE.H`)

### `GrGC` (graphics context — one per opened board/window)

Key members (grouped):
- **Transport**: `cmdTransportInfo{ fifoPtr, fifoRoom, fifoStart/End/Size, lastBump, bumpPos,
  bumpSize, lastFence, autoBump, triPacketHdr, cullStripHdr, fifoOffset, ... }`
- **State**: `state{ shadow (every hw register), tmuShadow[], vData (vertex layout),
  cull_mode, paramIndex, fifoFree, ... }`, dirty flags, `curVertexSize`, `curTriSize`,
  `tsuDataList[]` (offset list of enabled vertex params)
- **Dispatch**: `triSetupProc`, `archDispatchProcs{ coorModeTriVector, ... }`,
  `curTriProcs/nullTriProcs` (null procs used while context lost/invisible)
- **Buffers**: `buffers[]`, `curBuffer`, `frontBuffer`, `lockPtrs`, `fbStride`
- **Textures**: per-TMU info `tmu_config`, texture base registers, `textureBuffer` (render-to-
  texture state)
- **Stats** (debug): `stats.trisProcessed/trisDrawn/texDownloads/tsuValClamp...`
- **Windowed**: `contextP`, `windowed`, lost-context dword pointer, `hwcContext`

### `_GlideRoot` (per-process singleton)

`CPUType`, `deviceArchProcs` (chosen proc tables), `environment{ swapPendingCount, bumpSize,
fenceLimit, autoBump, gammaR/G/B, ... every FX_* env var }`, `pool` (float constants incl.
pack biases), `p6Fencer` (the fence dummy), per-board `GrHwInfo`.

### Proc dispatch model

```
_triSetupProcs[arch][cullMode][validState]  → gc->triSetupProc   (re-picked on state change)
_vertexListProcs[arch]                      → strip/fan walkers
_texDownloadProcs[arch][format][width]      → texture download inner loops
arch: 0 = default x87, 1 = 3DNow! (GL_AMD3D builds); Intel stays 0 (KNI Glide path never landed)
null tables swap in when the window is occluded/context lost → draws become no-ops
```
Selection: `GPCI.C:1594` — vendor from CPUID (`kCPUVendorAMD/Cyrix/IDT` + feature bits 0x2 =
MMX+3DNow!) picks arch 1. Override with `FX_CPU`.

## 3. Environment variables (performance/behavior knobs)

Parsed in `GPCI.C` `_GlideInitEnvironment` and `GSST.C` (see also `H5/DOCS/Napalm registry
keys.doc` for the registry-side equivalents used by the OS drivers):

| Variable | Effect |
|---|---|
| `FX_CPU` | Override CPU detection (hex: vendor<<16 \| features) |
| `FX_GLIDE_BUMP` / `FX_GLIDE_BUMPSIZE` | Disable auto-bump / set bump granularity (default 0x10000) |
| `FX_GLIDE_FENCE_LIMIT` | WC fence interval, max 0x10000 |
| `FX_GLIDE_SWAPINTERVAL` / `FX_GLIDE_SWAPPENDINGCOUNT` | Vsync divisor / max queued swaps (0–3) |
| `FX_GLIDE_NUM_CHIPS`, `FX_GLIDE_ANALOG_SLI`, `FX_GLIDE_SLI_BAND_HEIGHT`, `FX_GLIDE_FORCE_SLI_BAND_HEIGHT` | SLI overrides |
| `FX_GLIDE_AA_TOGGLE_KEY`, `FX_GLIDE_FORCE_OLD_AA` | FSAA runtime toggle hotkey / legacy AA |
| `FX_GLIDE_LOD_BIAS`, `FX_GLIDE_LOD_DITHER`, `FX_GLIDE_PERFORMANCE_TRILINEAR_Q3D` | Texture LOD quality/perf |
| `FX_GLIDE_TMU_MEMSIZE`, `FX_GLIDE_TEX_ALIGN` | Texture memory clamp / alignment |
| `FX_GLIDE_ALLOC_COLOR`, `FX_GLIDE_ALLOC_AUX`, `FX_GLIDE_BPP`, `FX_GLIDE_GBC` | Buffer allocation overrides |
| `FX_GLIDE_2PPC`, `FX_GLIDE_2PPC_BAND` | 2-pixels-per-clock mode control (Napalm) |
| `FX_GLIDE_WAX_ON` | Enable 2D-engine (WAX) usage from Glide |
| `FX_GLIDE_SCREENSHOT_KEY`, `FX_SNAPSHOT` | Framebuffer dump hotkey |
| `SSTH3_RGAMMA/GGAMMA/BGAMMA` | Gamma |
| `FX_GLIDE_NO_HW` | Run against no hardware (with CSIM builds) |
| Build-time (makefile.linux): `FX_GLIDE_DIRECT_WRITE` (register writes instead of packet FIFO), `FX_GLIDE_PACKET_FIFO`, `FX_GLIDE_ALT_TAB`, `FX_GLIDE_H5_CSIM` | Transport selection / simulator |

## 4. The draw fast path, annotated

```
grDrawTriangle(a,b,c)                             GDRAW.C:308
 └► TRISETUP(a,b,c) = gc->triSetupProc            (specialized; e.g. 3DNow nocull valid)
     ├─ [invalid state] GR_FLUSH_STATE()          DISTATE.C → PKT4 masked burst
     ├─ [cull on] _grTriCull: area sign test      GXDRAW.C:211 (zero-area reject, xor-sign cull)
     ├─ GR_SET_EXPECTED_SIZE(curTriSize, 1)       FXCMD.H:353 (arith room check; makeRoom rare)
     ├─ TRI_STRIP_BEGIN(kSetupStrip,3,vs,PKT3_BDDBDD)  header dword
     ├─ TRI_SETF(x) TRI_SETF(y) … over tsuDataList     raw float stores to WC fifo
     └─ TRI_END: fifoRoom -= …; CHECK_FOR_BUMP    doorbell every bumpSize bytes
```
Costs to know: the room check is ~3 ALU ops; the fence is amortized to 1 locked op / 16K
vertices at defaults; the only branches are cull and dirty-state, both specialized away when
possible. **This loop is the template for any new driver's submission path.**

## 5. Glide 2.x deltas (`H5/GLIDE/SRC`)

- Same transport architecture (FIFO.C/fxcmd shared lineage), pre-Napalm feature set: no FSAA
  extension, no texture-buffer render-to-texture API, `GuTex*`/`GUMP.C` higher-level texture
  memory manager (moved out in G3), `GUCLIP.C` guardband clip helpers, `GMOVIE.C` capture.
- `AMD3D.H` + `XDRAW2.ASM`/`XTEXDL.ASM` — the original 3DNow! work (Glide3's XDRAW3 is its
  descendant).
- Needed by older titles; `SWLIBS/G2G3` shows the API mapping if you want Glide2-on-Glide3
  instead of building both.

## 6. Threading & reentrancy model

Single "current GC" per process (TLS pointer, `GTHREAD.C`). No internal locking on the
fullscreen path; windowed path wraps FIFO access in one critical section (`GR_WINFIFO_BEGIN`).
NT-side mutexing issues are called out in FIFO.C's changelog ("nt currently has mutexing
problems via ddraw and extescape", 12/98) — treat multi-window + multi-thread as fragile by
design. Any modernization that adds threads must keep packet-emission ordering per context.

## 7. Numeric conventions

- Vertex coords arrive pre-transformed in screen space; hardware guardband is ±2048
  (`kDimThresh` asserts in `GXDRAW.C`), origin configurable upper-left/lower-left.
- W is 1/W-style ("ooz vs z" changelog); depth either 16-bit W-buffer or 16/24 Z.
- `GLIDE_FP_CLAMP`: floats with exponent < 0x20 are flushed to 0 before the TSU (denormal
  protection), counted in `stats.tsuValClamp`.
- Packed color mode (`GLIDE_PACKED_RGB`, PKT3 bit 28): floats bias-packed to bytes via
  add-2^23 trick (`RGBA_COMP`, pool constants `fBiasHi/Lo`).
