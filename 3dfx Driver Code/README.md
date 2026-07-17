# 3dfx Interactive — Voodoo Driver Source Tree ("devel", H5/Napalm generation)

**Complete analysis and inventory** — reviewed July 2026.

> **Full documentation set: [`documentation/`](documentation/README.md)** — deep dives on
> architecture, the complete directory inventory, Glide runtime internals, the hardware layer,
> OS drivers, OpenGL/other APIs, **toolchain & build walkthroughs** (for building as-is),
> **debugging/diagnostic tooling**, **incomplete areas** (no XP driver, NT4 lacks D3D, etc.),
> and the **Phase-2 optimization roadmap** for a modernized driver.
> This file is the executive summary.

This is a snapshot of 3dfx Interactive's internal `devel` build tree from **October–November 2000**
(file dates run through 13 Nov 2000, weeks before 3dfx's assets were sold to NVIDIA). It contains
essentially the entire software stack for the last shipping 3dfx hardware generation:

- **Voodoo3** (H3/H4 "Avenger" family — Banshee-derived single-chip 2D/3D)
- **Voodoo4 / Voodoo5** (H5 **"Napalm"**, the **VSA-100** chip, incl. 2-way and 4-way SLI + FSAA)

The tree builds: Glide 2.x and Glide 3.x runtimes (DOS, Win9x, NT, Linux, MacOS 8/9), the Windows 9x
DirectDraw/Direct3D display driver, Windows NT4 and Windows 2000 miniport + display drivers, an
OpenGL ICD and a MiniGL, the video BIOS, chip diagnostics, the C-model chip simulator, texture
tools (TEXUS/FXT1 compression), and a large pile of internal support libraries and test content.

~7,100 files, ~344 MB. Top-level `MAKEFILE` confirms this is the master `devel` tree; hardware
generation is selected with `FX_GLIDE_HW` (`SST1`, `SST96`, `CVG`, `H3`, `H5`) — **this snapshot
contains the `H5` and `swlibs` subtrees** (the SST1/Voodoo1, CVG/Voodoo2, and standalone H3 glide
trees are referenced by the makefile but not included; H3-class boards are still covered by the
H5-era OS drivers below).

---

## 1. Directory inventory

### Top level

| Item | What it is |
|---|---|
| `MAKEFILE` / `makefile.unix` | Master nmake / GNU-make entry; recurses into `swlibs` then `h5` based on `FX_GLIDE_HW` |
| `SETENV.BAT`, `setdosenv.bat`, `TOOLS.BAT`, `W2kEnv.bat`, `Nt4Allen.bat` | Build-environment setup. Documents the exact toolchain: MSVC 6.0 (32-bit), MSVC 1.52 (16-bit code), MASM 6.14, Win95/98 DDK, NT4 DDK, W2K DDK, DX7 DDK, Win32 SDK, Watcom+DOS4GW for DOS |
| `Q3dEnv.cmd`, `Q3dGlideEnv.cmd`, `Q3dDosEnv.cmd`, `Q3dDiagsEnv.cmd` | Quantum3D build variants (Quantum3D used 3dfx chips in arcade/sim boards; sets `FX_GLIDE_HW=H5`, `FX_HW_PROJECTS=glide3`) |
| `BuildDiags.bat/.cmd`, `pulldiags*.bat` | Build and package the diagnostics kit |
| `MANIFEST.TXT`, `README.TXT` | Original Glide2x/Glide3x *Diagnostics Kit* readme + file manifest (the `3Dfx/` directory) |
| `NAPALM.XLS`, `super_2116.xls` | Engineering spreadsheets (Napalm chip data) |
| `opengl1_2_1.ps` | The OpenGL 1.2.1 specification (PostScript) |
| `3Dfx/` | Prebuilt SDK/diagnostic **binaries**: Glide2x/Glide3x test programs (`test00–test38.exe`), `.3DF` texture files, `detect.exe`, `pcirw.exe`, `pass.exe` |

### `H5/` — the Napalm (VSA-100) hardware tree

| Directory | Contents / purpose |
|---|---|
| `INCLUDE/`, `INCSRC/` | **The hardware contract.** `H3REGS.H` (full register map), `H3GDEFS.H` (command-FIFO packet encodings — see §3), `H3HWC.H`, `H3DEFS.H`, `SST1VID.H` (video timing), `CMDDEFS.H`, `FXVID.H`, `VECTOR.H`, `GDEBUG.H`, `H3INFO.H`, `VXD.H` |
| `GLIDE/SRC/` | **Glide 2.x** runtime source (the classic game API; DOS/Win/Mac targets) |
| `GLIDE3/SRC/` | **Glide 3.x** runtime source — the primary 3D path for this generation. See §4 for the performance architecture |
| `GLIDE3/CONFORM/` | Glide conformance test suite (raster/state/texture tests, bitmap compare tools, test runner) |
| `MINIHWC/` | **Hardware context layer under Glide** (`MINIHWC.C`, 246 KB): PCI/AGP board discovery & mapping, buffer/tile allocation, command-FIFO init (local-memory and **AGP FIFO**, `hwcInitAGPFifo`), video init, **SLI/AA configuration** for 2- and 4-chip boards, gamma, windowed-rendering FIFO contexts, `LINHWC.C` + `LINDRI.H` (**Linux DRI** support), `DOS_MODE.C`, `MAC_MODE.C` |
| `CINIT/` | **Chip cold-init library** (`H3CINIT.C`): DRAM/SGRAM init & sizing, PLL programming (`h3InitPlls`, `h4InitPlls`, `PLLTABLE.H`), full VESA-style mode table (`MODETABL.H`), VGA init, silicon-process measurement, block-write config. `cinit-old/` is the prior rev |
| `BIOS/` | **Video BIOS source** (x86 real-mode ASM: `BIOSNEWS.ASM`, `CLKPLL.INC`, `DACDATA.INC`, mode list, OEM config tables) + `FLASH/` flash utility + build tools |
| `HAL/` | Diagnostics HAL (`fxHal*` API): user-mode register/AGP I/O (`HALIO.C`, `AGPIO.C`), kernel access shims, GUI monitor app. Used by DIAGS, not by shipping drivers |
| `CSIM/` | **C-model simulator of the chip**: `FBI.C` (frame-buffer interface), `TREX.C` (texture unit), `CMDFIFO.C`, `ALPHA.C`, `LFB.C`, `LOD.C`, `COMPRESS.C` (texture compression), `YUV.C`, `RECIP.C`, `SETUP.C` (triangle-setup unit model), `CCU.C`. Drivers can be built against CSIM (`FX_GLIDE_H5_CSIM`) to run without hardware |
| `DIAGS/` | Hardware validation: `FBI/`, `TREX/`, `VID/`, `GUI/` per-unit register/functional tests; `BRINGUP/` board bring-up scripts; `MFTG/` manufacturing test suite — includes **recorded Unreal game streams** (`MFTG/BIN/UNREAL/T00000xx.000`) replayed as factory workloads, plus Lua-scripted test clients |
| `PERL/` | Bring-up/diag Perl: PCI walking, register poke/peek, `fifo_replay.pl`, board test masters, memory diags |
| `W2K/`, `WinNT/` | **Windows 2000 / NT4 drivers**: `Miniport/H5` (modeset, I2C, TV-out, DFP, interrupts) + `Displays/H5` (2D GDI accel, DirectDraw, **full D3D driver compiled into the display driver**, `AFIFO.C`/`CFIFO.C` command transport) + `Inf/Voodoo3`, `Inf/Voodoo5` INFs |
| `Win9x/DX/` | **Windows 9x driver set**: `DD16` (16-bit GDI driver), `DD32` (32-bit DirectDraw/DDraw HAL, SLI-aware 2D blits, memory manager), `D3D` (Direct3D HAL — see §5), `MINIVDD` (kernel VxD: PCI config, AGP GART, mode switch, I2C/TV encoders, IRQ), `INF/Voodoo3` + `INF/Voodoo5` |
| `MacOS8/` | Complete Mac driver stack: GDX video driver, Glide 2/3 ports, **RAVE** (QuickDraw 3D) driver, OpenGL MCD over Glide, QuickDraw/QuickTime 2D acceleration, control panel (EN/FR/JP), Open Firmware, flash ROM, installer |
| `UTIL/`, `LIB/`, `LIBSRC/`, `BIN/`, `BINSRC/`, `NICKTEST/` | Image/UI helpers for tests, build output dirs, header snapshots, an engineer's scratch test dir |
| `DOCS/` | **Gold mine**: `DATABOOK.PDF` (chip databook), `SW Architecture Spec.doc`, `avenger_spec.doc` (Voodoo3 chip spec), `PS_Napalm` product specs, `Napalm registry keys.doc` (driver tunables), validation plans, performance schedule (`Napalm Performance Development.mpp`) |

### `SWLIBS/` — shared software libraries and larger clients

| Directory | Contents / purpose |
|---|---|
| `INCLUDE/`, `INCSRC/` | Shared headers: `glide.h`, `sst1*.h`, 3dfx types, nmake/gnu make infrastructure (`include/nmake/3dfx.mak`, `include/make/3dfx.linux.mak`) |
| `FXPCI/` + `NEWPCI/` | PCI access library (`PCILIB`) + `pcirw`-style tools (old and new versions) |
| `FXMEMMAP/` | The `FXMEMMAP.VXD` — Win9x kernel service that maps board memory into user space, provides **P6 MTRR write-combining setup** for the LFB/FIFO apertures |
| `FXAGP/`, `FXGART/` | AGP aperture / GART programming libraries (AGP command FIFO + AGP texturing support) |
| `FXDPMI/` | DOS DPMI services (incl. `P6.ASM` — WC/MTRR from DOS) |
| `FXNTDRVR/` | NT kernel helper drivers (`FXPTL`, `FXGPIO`, `NTRemap`) — NT equivalents of FXMEMMAP |
| `FXMISC/`, `FXREMAP/`, `FXMAC/`, `CSERVICE/`, `DOSDLL/`, `ATBNET/` | 64-bit math helpers, address remap, Mac PCI shims, client/server IPC service, DOS DLL loader (Watcom), IPX networking for the arcade toolkit |
| `HWC/` | Older/diag hardware-context library (predecessor of `H5/MINIHWC`) |
| `OPENGL/` | **The OpenGL ICD**: `SRC/` (GL core, display lists w/ optimizer, SoftPipe), `GLIDE2X/`+`GLIDE3X/` variants layered on Glide, `SST/` Glide back-end (`sst_prim.c`, `sst_tex.c`…), `WGL/` ICD plumbing & dispatch, `GDIBYP/` (**GDI bypass** display driver hook), `SYSTRAY/` tray applet, `CONFORM/` OpenGL conformance, `OGTST/` regression suite, `BENCH/` (`trispeed`, `bindspeed`, **`QUAKE.XLS`** benchmark tracking), `SCRSAVE/` 3D screensavers |
| `3DFXGL/` | The **MiniGL** (`fxgl` — the subset GL games like glQuake used) + GLUT + demos |
| `TEXUS/`, `TEXUS2/` | **Texture Utility System**: format conversion, mipmap generation, quantizers — palettized (`PAL256`, `PAL6666`), **NCC** narrow-channel (YIQ + neural-net trainer `NCCNNET.C`), and TEXUS2 adds the **FXT1** compressed-texture codec (`CODEC.C`, `BITCODER.C`, `EIGEN.C`, `SST2FXT1.H`) — 3dfx's open 4bpp compression answer to S3TC, supported natively by VSA-100 |
| `GlideTrap/` | **Glide API tracer/replayer** (record every Glide call + textures with CRC dedup → `.000` stream files; `PLAY/` replays them). This is what produced the Unreal factory-test streams — 3dfx's game-workload capture system |
| `ARCADE/` | **ATB — Arcade ToolBox**: a full optimized transform/light/clip geometry pipeline on top of Glide for arcade/sim developers (Quantum3D), with D3D/Glide/RAVE back-ends, MultiGen model import, benchmark tools |
| `GAMEGEN/` | MultiGen "GameGen" flight-format model converter (OP_* opcode database walker) |
| `3DfxSplash/`, `3DfxSplashOld/`, `NuSplash/` | The animated 3dfx logo splash shown at Glide startup (3D Studio MAX exporter plugin + playback lib) |
| `3DSR4/`, `MRI/`, `G2G3/` | 3D Studio R4 driver glue, I2C reference, Glide2→Glide3 wrapper stubs |
| `DOCS/` | `TEXUS.DOC`, `PCI.DOC`, `INITHAL.DOC`, `DLL_USE.DOC`, `FLATWARE.DOC` |
| `BIN/`, `BINSRC/`, `LIB/`, `LIBSRC/` | Build tools (flex/bison, WING32) and output dirs |

---

## 2. End-to-end architecture — how a game's frame reaches the screen

```
        Game (Quake, Unreal, D3D title, GL title)
       ┌────────────┬──────────────┬───────────────┬─────────────┐
       │ Glide 2/3  │  MiniGL/ICD  │   Direct3D    │  DirectDraw │
       │ (native)   │  (SWLIBS/    │  (Win9x DX/   │  2D/blits/  │
       │ H5/GLIDE3  │  OPENGL →    │  D3D, or W2K  │  flips      │
       │            │  Glide)      │  display drv) │             │
       └─────┬──────┴──────┬───────┴──────┬────────┴──────┬──────┘
             │             │              │               │
             ▼             ▼              ▼               ▼
   ┌─────────────────────────────────────────────────────────────┐
   │       COMMAND TRANSPORT: software-managed packet FIFO       │
   │  CPU writes packets to a ring buffer in frame-buffer memory │
   │  or AGP memory through a write-combined mapping, then       │
   │  "bumps" the chip. Chip pulls packets asynchronously.       │
   └───────────────────────────┬─────────────────────────────────┘
                               │
             ┌─────────────────┴───────────────┐
             │ MINIHWC / HWC (user-mode board  │
             │ mgmt) ──ExtEscape/DeviceIoCtl──▶│
             │ MiniVDD (9x) / Miniport (NT/2K) │
             │ FXMEMMAP.VXD: map LFB, set WC   │
             └─────────────────┬───────────────┘
                               ▼
   ┌─────────────────────────────────────────────────────────────┐
   │  VSA-100 / Avenger hardware                                 │
   │  CMDFIFO ▶ 2D engine ("WAX") + 3D: Triangle-Setup Unit ▶    │
   │  FBI (raster/depth/alpha/dither) + TREX (TMUs: bilinear/    │
   │  trilinear, LOD, FXT1/NCC decompress) ▶ RAMDAC/video-out    │
   │  (+ 1–3 slave chips via SLI interleave, FSAA sample combine)│
   └─────────────────────────────────────────────────────────────┘
```

Init path: BIOS posts the card → OS loads miniport/MiniVDD → CINIT (`H3CINIT.C`) sizes RAM,
programs PLLs and the mode from `MODETABL.H` → driver maps registers + LFB (write-combined via
FXMEMMAP/miniport) → MINIHWC allocates color/aux/depth buffers and tiles, initializes the command
FIFO (`hwcInitFifo` / `hwcInitAGPFifo`), configures SLI/AA → Glide/D3D/GL stream packets.

---

## 3. The hardware programming model (from `H5/INCLUDE/H3GDEFS.H`, `H3REGS.H`)

Everything the chip does is driven by **eight packet types** written into the command FIFO:

| Packet | Purpose |
|---|---|
| **PKT0** | FIFO control flow: `NOP`, **`JSR`, `RET`, `JMP_LOCAL`, `JMP_AGP`** — the FIFO is a little program; the driver plants a JMP at the ring end to wrap, and windowed rendering uses JSR/RET to splice per-window sub-FIFOs |
| **PKT1** | N register writes to one base address (optionally auto-incrementing; bit 14 selects the 2D register file) |
| **PKT2** | Masked write across a 29-register group |
| **PKT3** | **Hardware triangle setup**: header encodes vertex count (up to 15), parameter mask (which of x,y,z,w,rgb,a,s/t/w per TMU are present), strip/fan continuation (`BDDBDD`/`BDDDDD`/`DDDDDD`), packed-color flag — followed by raw vertex floats. The chip's setup unit computes all edge slopes and parameter gradients |
| **PKT4** | 14-bit-masked register-group write (the workhorse for state changes) |
| **PKT5** | Bulk linear writes: LFB, YUV planes, **texture download through the FIFO** (`3DLFB`/`TEXPORT` spaces) |
| **PKT6** | **AGP MOVE**: chip-initiated DMA from AGP/host memory into LFB/texture space (strided 2D copies) |

Two command FIFOs exist (`cmdFifo0/1`); each can live in local memory or AGP space
(`SST_CMDFIFO_AGP`), with optional "hole counting" (`SST_CMDFIFO_DISABLE_HOLES`) that tolerates
out-of-order arrival of write-combined CPU stores.

---

## 4. How games were made fast — the optimization catalog

This is the answer to "how were the architecture and driver code set up to optimize games."
Each item cites the primary source files.

### 4.1 Decouple the CPU from the GPU: the software command FIFO
(`H5/GLIDE3/SRC/FIFO.C`, `FXCMD.H`, `H5/MINIHWC/MINIHWC.C`)

- The CPU never spins on chip FIFO status in the hot path. Glide keeps a shadow **`fifoRoom`**
  count and a write pointer in the GC (graphics context); `GR_CHECK_FOR_ROOM` is pure arithmetic
  until room actually runs out, only then calling `_grCommandTransportMakeRoom` (which reads the
  hardware read-pointer — commented *"a 'real' hw read which is \*SLOW\*"*).
- Writes go through a **write-combining (USWC) mapping** — set up by FXMEMMAP.VXD / the miniport
  via MTRRs (`P6STUFF.ASM`, `K6_2.ASM`, `PENT.ASM` handle P6/K6/K7 WC enables) — so vertex data
  streams to memory at cacheline burst rates instead of uncached dword writes.
- The chip is notified with a periodic **"bump"** — one register write telling it N more bytes are
  valid (`GR_BUMP_N_GRIND`, auto-bump every `bumpSize`=64 KB by default, tunable via
  `FX_GLIDE_BUMP*`). Amortizes doorbell cost over thousands of commands.
- **Hole counting** lets WC buffers flush out of order; the hardware counts arrived dwords rather
  than requiring in-order writes. Cost: a CPU store **fence every 64 KB** (`GR_CHECK_FOR_FENCE`,
  `P6FENCE` = `xchg` to a dummy — locked, serializing; limit tunable via `FX_GLIDE_FENCE_LIMIT`).
- FIFO wrap is a planted **JMP packet** (no hardware autowrap); windowed contexts splice with
  JSR/RET. Fullscreen games get the big linear ring.
- On Napalm, entering a fullscreen 3D app **promotes the FIFO into AGP memory**
  (`Promote_CmdFifoToAGP`, `DDFXS32.C`) so command traffic stops competing with pixels for local
  memory bandwidth. (Deliberately disabled on Voodoo3: *"Voodoo3 disables this function because of
  hardware problems, but it should be enabled on Napalm."*)

### 4.2 Push per-triangle math into hardware, strip the driver to a copy loop
(`H5/GLIDE3/SRC/GXDRAW.C`, `XDRAW2.ASM`, `XDRAW3.ASM`, `FXCMD.H`)

- VSA-100 has a **triangle/strip setup unit**: the driver emits PKT3 with raw vertex floats;
  slopes/gradients are computed on-chip. The per-triangle CPU work collapses to: cull test,
  FIFO-room check, and a tight float copy (`TRI_STRIP_BEGIN` / `TRI_SETF` / `TRI_END`).
- Vertex layout is preprocessed: when the app changes vertex format, Glide rebuilds
  **`tsuDataList`** — an offset list of only the enabled parameters — so the inner loop copies
  exactly the floats the current mode needs, no per-field branching (`gc->curVertexSize`,
  `gc->curTriSize` are precomputed for the room check).
- Software backface culling uses the **sign bit of the 2D area** directly
  (`j ^ culltest) >= 0` on the float's integer image) — no branches on compares, and zero-area
  triangles are rejected before touching the FIFO (`_grTriCull`).
- Denormal/tiny floats are squashed with an integer exponent test (`FP_FLOAT_CLAMP`) to protect
  the setup unit, and colors are packed float→int with the classic **add-big-bias float trick**
  (`RGBA_COMP`), avoiding `ftol`'s rounding-mode stalls.

### 4.3 Specialize, don't branch: function-pointer dispatch everywhere
(`H5/GLIDE3/SRC/GPCI.C` `_GlideInitEnvironment`, `FXGLIDE.H`)

- Triangle setup is a **3-D dispatch table**: `_triSetupProcs[cpu_arch][cull][state-valid]` —
  eight specialized entry points (`_trisetup_{Default,3DNow}_win_{cull,nocull}_{valid,invalid}`).
  The current pointer `gc->triSetupProc` is re-picked only when cull mode or state validity
  changes (`INVALIDATE` macro), so `grDrawTriangle` itself contains zero mode branches.
- Same pattern for vertex-array walkers (`_grDrawVertexList_*`), whole-array draws
  (`_grDrawTriangles_*`), and texture download (`_texDownloadProcs[arch][format][width]`).
- **CPU detection at init** (`CPUDTECT.ASM`, overridable with `FX_CPU`): AMD/Cyrix/IDT with
  MMX+3DNow! feature bits swap in the **3DNow! assembly** setup/strip/texture paths
  (`XDRAW2.ASM`, `XDRAW3.ASM` — 122 KB of hand-scheduled x86/3DNow!, `XTEXDL.ASM` MMX/3DNow!
  texture download). PowerPC gets its own paths (`GXDRAW_PPC.C`, `PPCDRAW2.S`, cache-line
  `dcbf` flushing for the FIFO).

### 4.4 Minimize register traffic: shadowing + deferred validation
(`H5/GLIDE3/SRC/DISTATE.C`, `FXCMD.H` `REG_GROUP_*`)

- Every chip register the API can touch is **shadowed in `gc->state.shadow`**; API calls only
  update shadows and set a dirty flag. `GR_FLUSH_STATE()` runs at the next draw and emits one
  **PKT4 masked group write** containing only dirty registers in one burst
  (`REG_GROUP_BEGIN/SET/END` build the mask at compile time where possible).
- TMU state is double-shadowed (`tmuShadow` vs committed `shadow.tmuState`) so texture state
  churn between draws collapses into the minimal diff.

### 4.5 SLI + FSAA as a driver-managed resource (Voodoo5)
(`H5/MINIHWC/MINIHWC.C` `hwcInitVideo`, `H5/Win9x/DX/DD32/DDFXS32.C`)

- 2- and 4-chip boards interleave **horizontal scanline bands** (`h3sliBandHeight`, 2–128 lines,
  tunable `FX_GLIDE_SLI_BAND_HEIGHT`); each chip renders only its bands — geometry is broadcast
  through the shared FIFO, fill rate multiplies. 4-way SLI forces **analog** band combining;
  digital combining otherwise; FSAA (2x/4x/8x) allocates secondary color/depth sample buffers and
  the video unit averages samples on scanout.
- The driver **promotes/demotes** configurations at runtime: `Enter_3DApplication` →
  `Promote_DeviceToSLIAA` (enable SLI/AA only for fullscreen 3D, where the buffers fit),
  `Exit_3DApplication` demotes so the 2D desktop keeps full memory. Single-buffered apps get SLI
  and AA disabled automatically.
- The slave chips' FIFOs are kept in sync by Glide (`FIFO.C` slave-FIFO checking, per-chip
  linear register mappings).

### 4.6 Per-game compatibility profiles ("bad app" list)
(`H5/Win9x/DX/DD32/DDFXS32.C` `IsBadApp`, `GetProcessFileName`)

- The DirectDraw driver looks up the running **.exe name** (obfuscated in the binary with a
  byte-wise `255-c` substitution cipher so games couldn't be spotted in a hex dump) against a
  table of *(chip count, SLI mode, AA mode) → forced new config* entries. Shipped entries:
  **"USAF FOR GAMEGAUGE.EXE"** (2-chip SLI → single chip), **SimCity 3000** and **SC3K World
  Edition** (any config → single chip, no AA).
- The D3D driver carries game-specific fixes (e.g. *"Fix for Unreal Tournament"*, `D3GLOBAL.H`
  rev 25, `D6DP2.C`), and registry keys (documented in `H5/DOCS/Napalm registry keys.doc`)
  gate per-title behaviors (`DISABLE_SLI`, `ENABLE_AA`, …).

### 4.7 A full SIMD software T&L pipeline in the D3D driver
(`H5/Win9x/DX/D3D/`)

For DX7 TnL-HAL contracts on CPUs of the era, the driver implements transform/clip/light on the
CPU in three interchangeable back-ends:

- **SOA x87 path** (`SOATNL.C`, `SOAXFORM.H`, `SOALIGHT.C`): vertices swizzled into
  structure-of-arrays groups of 4, with **prefetching of the next SOA group** during transform.
- **3DNow! path** (`K3DTNL.C`, `K3DLIGHT.C`, `tlk3dmath.asm`) for AMD K6-2/K7.
- **SSE/KNI path** (`tlknimath.asm`, `ATRIKNI.ASM`, `AXTRIKNI.ASM`) for Pentium III
  ("Katmai"), selected by `CPU.ASM` CPUID probing (incl. OS XMM-support check).
- Rasterization submission is hand-written assembly per primitive topology and CPU
  (`ATRI.ASM` 95 KB, `AMESH*.ASM`, `AFAN*.ASM`) writing PKT3 vertex streams straight into the
  WC FIFO; `MEMCOPY.ASM`/`FXF2I.C` provide tuned copies and float→int conversion; `AFIFO.C`
  manages the transport. DrawPrimitive2 fast paths exist per DX version (`D6DP2.C`, `D7DP2.C`,
  `D8DP2.C`).

### 4.8 Feed the texture units cheaply
(`H5/GLIDE3/SRC/GTEXDL.C`, `SWLIBS/TEXUS2/`, TREX)

- Texture download goes **through the FIFO** (PKT5 TEXPORT) so it pipelines behind rendering
  instead of stalling; MMX/3DNow! copy loops (`_grTexDownload_3DNow_MMX`) move texel data;
  per-format/width specialized download procs avoid inner-loop switches.
- **FXT1** (TEXUS2 codec, `SST2FXT1.H`) gives 4 bpp compressed textures decoded for free in TREX
  — 4–8× more texture per MB of card RAM and per unit of download bandwidth. Legacy **NCC**
  (narrow-channel YIQ, neural-net trained) and palettized formats serve the older TMU formats.
- Mipmap LOD bias/dither tunables (`FX_GLIDE_LOD_BIAS`, `FX_GLIDE_LOD_DITHER`,
  `FX_GLIDE_PERFORMANCE_TRILINEAR_Q3D`) trade filtering quality for fill rate.

### 4.9 Latency hiding at the frame level

- **Triple buffering** in hardware (`SST_TRIPLE_BUFFER_EN`, `hwcAllocBuffers`) plus a bounded
  **swap-pending queue** (`FX_GLIDE_SWAPPENDINGCOUNT`, clamp 0–3): the CPU may run up to N frames
  ahead of scanout before Glide throttles — smooths frame-time spikes without unbounded lag.
- Fullscreen flips borrow the **video overlay path** (*"Desktop flipping can't be pipelined, so
  the video overlay is borrowed to display the primary flipping chain"* — `DDFXS32.C`), making
  buffer flips a pipelined register write.
- `grFinish`/`grFlush` semantics let games opt out of synchronization entirely; the "lost
  context" check in windowed mode was reduced to reading one shared dword instead of a kernel
  query (FIFO.C rev history: *"should yield an insignificant speedup"* — they measured it).

### 4.10 Measure with real games

- **GlideTrap** records a game's full Glide stream (CRC-deduped textures) to `.000` files;
  `PLAY` replays them deterministically — used for perf regression and as **manufacturing tests**
  (the `H5/DIAGS/MFTG/BIN/UNREAL` streams are recorded Unreal runs executed on every board).
- `SWLIBS/OPENGL/BENCH` holds `trispeed`/`bindspeed` microbenchmarks and `QUAKE.XLS` — Quake
  timedemo tracking. `H5/DOCS/Napalm Performance Development.mpp` is literally a Microsoft
  Project plan for the performance work.

---

## 5. Voodoo3 vs Voodoo5 in this tree

| Aspect | Voodoo3 (Avenger) | Voodoo4/5 (Napalm/VSA-100) |
|---|---|---|
| Covered by | Same OS driver trees (`INF/Voodoo3` + `IS_NAPALM()` runtime branches) | Primary target of `H5/` |
| Register file | H3-style, 0x3FF regbase in packets | Extended (`#ifdef H4`: 0x7FF regbase), FXT1, 32-bpp rendering |
| Command FIFO | Local memory only (AGP FIFO disabled — hardware bug) | Local **or AGP** FIFO, hole counting |
| Multi-chip | n/a | 2/4-chip SLI + 2x/4x/8x FSAA (`SLI_AA_REQUEST` via MiniVDD/miniport) |
| Chip init | `h3InitPlls`, shared `MODETABL.H` | `h4InitPlls`, same CINIT library |
| Device IDs | Checked via `SST_DEVICE_ID_*` ranges in `MINIHWC.H` / `fxhal.h` | `IS_NAPALM(deviceID)` range check |

The Glide3 runtime built with `FX_GLIDE_HW=H5` + `FX_GLIDE_NAPALM=1` targets Napalm; the Win9x/
NT/W2K display drivers detect the board at runtime and serve both V3 and V4/V5 from one codebase.

---

## 6. Can you build Voodoo 3 / Voodoo 5 drivers from this tree?

**Short answer: yes — the tree is complete enough, but only with the period toolchain for the
Windows drivers; the Linux/Glide path is the practical modern route.**

### What's here and buildable
- **Source completeness**: all driver components have full source (Glide2/3, MINIHWC, CINIT,
  D3D/DD/MiniVDD for 9x, NT4/W2K miniport+display, OpenGL ICD, BIOS, diags). Only 11 files are
  empty/truncated, none load-bearing (`Disabled`, three `LOG` placeholders, one Mac `glr` file, two
  `.RES` build outputs, one sample). Prebuilt `.LIB`/`.OBJ`/`.DLL` intermediates are even present.
- **Build system**: complete — top-level `MAKEFILE` → `swlibs` + `h5`, nmake infrastructure in
  `SWLIBS/INCLUDE/NMAKE`, env scripts (`Q3dEnv.cmd` shows the exact invocation), and parallel
  `makefile.linux`/`makefile.unix` chains.

### Per target
| Target | Feasibility | Requirements |
|---|---|---|
| **Glide3/Glide2 runtime, Win32** | High | MSVC 6.0 + MASM 6.14 (`SETENV.BAT` documents `DEVTOOLS` layout). Builds `glide3x.dll` for both V3 and V5 |
| **Glide3 for Linux + DRI hooks** | **Highest — recommended** | gcc; `makefile.linux` chain is present and was maintained through mid-2000 (`LINHWC.C`, `LINDRI.H`). This lineage is the ancestor of the open-source Glide that still exists; diffing/porting against modern kernels is the realistic path for a *new* driver |
| **Win9x driver (DD16/DD32/D3D/MiniVDD)** | Medium | MSVC 6 **and** MSVC 1.52 (16-bit DD16), MASM 6.14, Win95/98 DDK, DX7 DDK. All source present; toolchain is abandonware but obtainable |
| **NT4 / W2K display+miniport** | Medium | NT4 DDK / W2K DDK `build.exe` environment (`W2kEnv.bat`, `BUILD/` dirs present) |
| **Video BIOS** | Medium | MASM-era 16-bit toolchain (`BIOS/buildtools` included); flashing needs care — a bad flash bricks the card |
| **DOS Glide** | High | Watcom C + DOS4GW (`setdosenv.bat`, `DOSDLL/WATCRT`) |
| **MacOS 8/9 stack** | Low-medium | CodeWarrior/MPW projects included, but the host platform is the hard part |
| **Diagnostics + CSIM** | High | CSIM builds as a plain C program; Glide can target it (`FX_GLIDE_H5_CSIM=1`) — you can run the whole Glide stack **without hardware**, which is ideal for studying behavior before touching your cards |

### Practical guidance for a new driver for your V3/V5

Bring-up order the codebase itself follows (mirror it):
1. **PCI probe & BAR mapping** — `SWLIBS/FXPCI`, `H5/MINIHWC/MINIHWC.C` (`hwcInit`, `hwcMapBoard`);
   device-ID ranges in `MINIHWC.H`.
2. **Cold init** — port `H5/CINIT/H3CINIT.C` nearly verbatim: DRAM sizing, PLL programming
   (`PLLTABLE.H`), mode timing (`MODETABL.H`). This file encodes silicon erratum knowledge you
   cannot re-derive.
3. **Mode set / video** — `hwcInitVideo` + `SST1VID.H`; VGA fallback in `INITVGA.H`.
4. **Write-combined mappings** — replicate what FXMEMMAP/miniport do (register aperture UC; LFB
   and FIFO apertures WC).
5. **Command FIFO** — `hwcInitFifo` + packet encodings from `H3GDEFS.H` + the transport logic in
   `GLIDE3/SRC/FIFO.C`/`FXCMD.H` (bump, fences, JMP wrap). Get PKT1/PKT4 register writes working,
   then PKT3 triangles.
6. **3D state** — register semantics from `H3REGS.H` + `DATABOOK.PDF` in `H5/DOCS`; validate
   against **CSIM** and the `H5/DIAGS` per-unit tests (they are, in effect, executable hardware
   documentation).
7. **SLI/AA (V5)** — `MINIHWC.C` lines around `hwcInitVideo`/SLI_AA_REQUEST plus the
   promote/demote logic in `DDFXS32.C`.

### Licensing note
Glide (GLIDE, GLIDE3, MINIHWC and related headers) was released by 3dfx in mid-2000 under the
**3dfx Glide General Public License** — those files carry that header. Most of the rest
(D3D/DD drivers, MiniVDD, BIOS, DIAGS, CSIM, TEXUS, docs) is marked **UNPUBLISHED PROPRIETARY
SOURCE CODE** — rights now trace to NVIDIA. Fine for personal study and driving your own cards;
be careful redistributing anything outside the Glide-GPL subset.

---

## 7. Historical notes found in the tree

- Revision logs run StarTeam/SourceSafe through **11 Oct – 13 Nov 2000**; several files note the
  *"Napalm Glide open source release"* merge (June 2000) with *"cleaned up offensive comments"*.
- `H5/DOCS/Rampage_Q3D_083199.ppt` references **Rampage**, the never-shipped next chip.
- The engineers left personality in the code: *"Since I am a lazy bastard, try to use the minivdd
  to setup SLI/AA"* (`MINIHWC.C`), *"What the *F* is this about?"* (SLI swap algorithm field),
  *"Glide is not an API for kids"* (`FXCMD.H`), and the FIFO doc comment *"Its actually a little
  bit bigger just in case someone does not read this comment."*
