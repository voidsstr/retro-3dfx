# 05 — OS Drivers: Win9x, NT4, W2K, Mac, DOS, Linux

## 1. Windows 9x driver set (`H5/Win9x/DX/`)

Four cooperating binaries (classic 9x display architecture):

```
GDI (16-bit)          DirectDraw/D3D (32-bit)         Kernel
┌───────────┐   ┌──────────────────────────────┐   ┌─────────────────┐
│ DD16      │   │ DD32 (DDraw HAL) + D3D (HAL) │   │ MINIVDD (VxD)   │
│ display   │◄─►│ 3dfx32v3/vb/vs.dll           │◄─►│ 3dfxv3/vb/vs.vxd│
│ minidriver│   │                              │   │ + FXMEMMAP.VXD  │
└───────────┘   └──────────────────────────────┘   └─────────────────┘
   v3 = Voodoo3, vb = Banshee, vs = Voodoo4/5 ("VS"/Napalm) name variants
```

### DD16 (`DD16/`) — 16-bit GDI display minidriver
DIBENG-based 2D driver: cursor, palette, GDI acceleration handoff, mode-set requests to the
MiniVDD. MSVC 1.52-built. Mostly boilerplate; the real 2D work happens in DD32/the 2D engine.

### DD32 (`DD32/`) — DirectDraw HAL + board policy
- `DDINIT(32).C`, `DDFX95.C`: HAL callbacks, capability tables, surface management.
- `MEMMGR32.C`/`DDMEMMGR.C`: video-memory heap incl. **tiled heap** for 3D surfaces.
- `DDFLIP.C`, `DDBLT32.C`, `DDSURF.C`, `DDOVL32.C`, `DDVPE32.C`: flip/blt/overlay/video-port.
- `DDSLI2D.C`, `DDSLIBLT.C`: 2D correctness when the desktop spans SLI bands.
- **`DDFXS32.C` — the policy brain** (documented in §4): fullscreen 3D enter/exit, SLI/AA
  promote/demote, bad-app list, AGP-FIFO promotion, gamma.
- `K6_2.ASM`, `PENT.ASM`, `CPU.ASM`(D3D): CPU detect + K6/K7/P6 write-combine (MTRR/PAT-era)
  enables; STB-inherited (`STBKNI` flags).
- `I2CFX.C`, `DDCSIM32.C`: I2C + a CSIM-backed DDraw path for driver testing without hardware.

### D3D (`D3D/`) — Direct3D HAL (DX6/DX7 + DX8 stub)
Geometry/state:
- `D3CONTXT.C` (contexts; also queries process exe name), `D3RSTATE.C` (renderstate →
  register shadow translation), `D3GLOBAL.C/H`, `D3TXTR.C` (+`FXT.C/H` FXT1/DXT support,
  `TCUTILS.C` texture cache), `D3CLEAR(2).C`, `D3FOG.C`, `LIGHT.C`.
- DrawPrimitive paths per DX rev: `D3EXECUT.C` (DX3/5 execute buffers), `D6DP2.C` (**DX6 DP2**,
  the main path; contains an Unreal Tournament compatibility fix per changelog), `D7DP2.C`,
  `D8DP2.C` (DX8 stub — see [09-INCOMPLETE-AND-GAPS.md](09-INCOMPLETE-AND-GAPS.md)),
  `D6FVF.C`/`D7FVFEXT.C` (flexible vertex formats), `D6MT.C` (multitexture),
  `PROCPRIM.C`/`TLREND.C` (primitive processing/render loop), `TLCLIP.C` (clipping).
- **Software T&L for DX7 TnL-HAL** in three SIMD flavors (selected by `CPU.ASM` CPUID probe +
  `STBPERF.INC` build flags `K6_2`, `STBKNI`, `NEWASMTRI`):
  - x87 SoA: `SOATNL.C`, `SOAXFORM.H`, `SOALIGHT.C`/`SOALITFN.C`, `SOAGEOM.C`, `SOACLIPC.C`,
    `SOADSWIZ.C`/`ASOASWIZ.ASM` (AoS↔SoA swizzle), groups of 4 verts, **prefetches next group**.
  - 3DNow!: `K3DTNL.C`, `K3DLIGHT.C`/`K3DLITFN.C`, `tlk3dmath.asm`, `K3DDEF.H`.
  - SSE (KNI): `tlknimath.asm`, `IAXMM.INC` (XMM macros; OS support probed via `_getKatmai`).
- **Assembly rasterizer submission** (writes PKT3 streams into WC FIFO): `ATRI.ASM` (95 KB,
  default), `atriK.asm`/`ATRIKNI.ASM` (K6/KNI), `axtriK.asm`/`AXTRIKNI.ASM` (xformed),
  `AMESH*.ASM` (indexed meshes), `AFAN*.ASM` (fans), `ATLREND.ASM`, `MEMCOPY.ASM`,
  `FXF2I.C` (fast float→int), `AFIFO.C` (transport), `FIFODBG.C`.
- `D3TRISW.C` (software-rasterized fallback tris), `D3TRACE.C` (API trace), `FXFDBG.C`.

### MINIVDD (`MINIVDD/`) — the kernel VxD
`H3VDD.C` core; embedded `H3CINIT.C` for modeset; `PCI.C`/`AGP.C`/`AGPCF.C` (bus + GART),
`H3IRQ.C` (vsync/fifo IRQs), `LOCK.ASM/C`, `P6STUFF.ASM` (MTRR WC), `PENT.ASM`, `CPU.C`;
display topology: `DDC.C`/`DDC2B` (monitor detect), `DFP.C` (digital flat panel), TV encoders
`BT868.C`, `CHRONTEL.C`, `KMTV.C`, `NULTVOUT.C`; `I2C.C` + `I2C/`; `NVRAM.C`; `DEVTABLE.C`
(multi-device dispatch); `DIBBLT.C`/`BITBLT.C`/`POLYGON.C` (VDD screen ops); `DBG32.C`,
`CALLDBG.ASM`. Services DD32/D3D via DeviceIoControl (`SLI_AA_ENABLE`=6/7 etc.) and
Glide/MINIHWC via the same channel.

### INF/installers
`INF/Voodoo3/Voodoo3.inf`, `INF/Voodoo5/3DFXVS.INF` (+ `3DFXDRV.INI`, help files) — registry
keys they install are the runtime tunables (see `H5/DOCS/Napalm registry keys.doc`).

## 2. Windows NT 4.0 (`H5/WinNT/`)

- `Src/Video/Miniport/H5`: kernel miniport — PCI resources, modeset (CINIT again), I2C/DDC,
  TV-out, DFP, interrupt; IOCTL surface for the display driver + Glide (HWCEXT protocol,
  `FXIOCTL.H`).
- `Src/Video/Displays/H5` (86 files): 2D GDI + DirectDraw + Glide/OpenGL support via
  ExtEscape. **No Direct3D on NT4** (86 vs W2K's 152 files — the `D3*/D6*` sources are absent;
  DX on NT4 capped at DDraw). Games on NT4 use Glide or OpenGL-over-Glide only.
- `BUILD/STBCUST.INC`, `STBPERF.INC` — customer/perf build switches.
- Built with NT4 DDK `build.exe` (see `Nt4Allen.bat`, `Src/Video/setenv.bat`).

## 3. Windows 2000 (`H5/W2K/`)

Same miniport/display split as NT4, but the display driver (152 files) **includes the full D3D
HAL** (same `D3*/D6*/D7*` sources as Win9x, plus `AFIFO.C`/`CFIFO.C` transports and NT-specific
`D3STUBS.C`) — this is the DX7-class driver for W2K. Miniport adds `POWER.C` (ACPI),
AGP (`AGP.C`), `MULTIMON.H`. Notable comments: NT5 build-1959 kernel-PTE exhaustion workaround
(`H3.C:1775`), "Must be NULL for NT5!" (`H3.C:249`) — evidence of active Win2K bring-up right
up to the end. INFs for Voodoo3 + Voodoo5 under `Src/Video/Inf/`.
**There is no Windows XP driver in this tree** — W2K's is the closest ancestor (XP-era
community drivers like Amigamerlin/SFFT derive from exactly this code).

## 4. The fullscreen-3D policy engine (`DD32/DDFXS32.C`) — read this file

- `Enter_3DApplication` / `Exit_3DApplication`: gamma load, video-processor toggle,
  **`Promote_DeviceToSLIAA`/demote**, `Clear_SLIAA_Buffers`, **`Promote_PrimaryToOverlay`**
  (desktop flips can't pipeline → borrow overlay for the flip chain),
  **`Promote_CmdFifoToAGP`** (Napalm only; V3 has an AGP-FIFO hardware bug), busy-flag
  discipline, D3D context dirtying after SLI transitions.
- `Compute_SLIAA_Config`, `Demote_DeviceFromSLIAA`, `MoveTileMark`: config state machine over
  `{chips} × {SLI mode} × {AA mode}` (`LOOKUP LookupValues[]`).
- **Bad-app system** (`IsBadApp`, `GetProcessFileName`, `BADAPP BadApps[]`): process exe name
  (obfuscated with byte cipher `ENCODE(ch)=255-ch` so the strings aren't greppable in the
  binary) matched against per-game entries; on match **and** current-config match, forces a
  safer config. Shipped list: `USAF FOR GAMEGAUGE.EXE` (2-way SLI no-AA → single chip),
  `SC3.EXE` and `SC3U.ICD` (SimCity 3000 (+World Edition): any config → single chip, no AA).
  Single-buffered apps additionally get SLI+AA disabled wholesale.
- Registry knobs honored here: `DISABLE_SLI`, `ENABLE_AA`, AA sample count, plus the D3D keys
  in `Napalm registry keys.doc` (LOD bias, mipmap dither, gamma, "3D filter quality",
  triple-buffer, VSync ...).

## 5. MacOS 8/9 (`H5/MacOS8/`)

- `GDX/` — the graphics driver (per-Apple "GDX" model), `hardware_resource_manager/` (HRM owns
  the board across clients; exports `hrmSLIAA` extension used by MAC_MODE.C), `OpenFirmware/`
  FCode, `FlashROM/`, `ControlPanel/` (EN/FR/JP), `Installer/`.
- APIs: `GLIDE2.X/`+`GLIDE3.X/` (PPC Glide with `PPCDRAW2.S` paths), `RAVE/` (QuickDraw 3D
  RAVE driver), `OpenGL/MCD_Glide(_Exp)` (Apple GL plugin over Glide),
  `QD_QT_Acceleration/` + `zH3_2D_ACCELERATION_3dfx/` (QuickDraw/QuickTime 2D + YUV).
- `BuildScripts/delivrable_specs/{MacVoodoo,NAPALM,napalm_alpha,voodoo3_beta}` — CodeWarrior/
  MPW build recipes per product; `release_notes/` mark Napalm Mac support as **alpha**.

## 6. DOS & Linux

- **DOS**: Glide2/Glide3 build with Watcom + DOS4GW (`setdosenv.bat`, `Q3dDosEnv.cmd`;
  `DOS_MODE.C` modeset; `SWLIBS/DOSDLL` provides DLL loading; `FXDPMI` maps the board and
  enables WC from real DOS). This is how the SDK `Bin/DOS/test*.exe` were produced.
- **Linux**: `makefile.linux` chain (gcc; `SWLIBS/INCLUDE/MAKE/3dfx.linux.mak`); MINIHWC's
  `LINHWC.C` supports `/dev/3dfx` (fxmemmap-style module) and **DRI** (SAREA cliprects in
  `LINDRI.H`) for windowed GL; Glide3 builds as `libglide3x.so`. This lineage became the
  public open-source Glide; it's the most practical modern build target.

## 7. 2D engine notes (all OS drivers)

The 2D core ("WAX"; `SstGRegs`) is programmed via PKT4_2D/PKT1_2D or direct MMIO: ROP blits
(SRCCOPY fast path pre-shifted `SSTG_ROP_SRCCOPY`), mono pattern, lines, polygon fill,
host-to-screen through the launch area. 2D and 3D share the memory controller — the drivers
serialize via FIFO ordering, and `FX_GLIDE_WAX_ON` lets Glide use 2D clears. SLI complicates
2D (bands live on different chips): see `DDSLI2D.C`/`DDSLIBLT.C` for the read-around logic.
