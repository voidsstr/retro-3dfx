# 04 — Hardware Layer: MINIHWC, CINIT, HAL, BIOS, Register Model

The code that owns the board. If you are writing a new driver, this layer is what you port
first — it encodes silicon knowledge (init ordering, PLL constants, memory timing, SLI wiring,
errata) that cannot be re-derived from the databook alone.

## 1. Register model orientation (`H5/INCLUDE/H3REGS.H`)

The chip exposes several register files through the register BAR (offsets are relative bases;
struct layouts in H3REGS.H are authoritative):

| Block | Struct | Contents |
|---|---|---|
| 3D core | `SstRegs` | status, TSU vertex/parameter regs, per-triangle setup regs, fbzMode/fbzColorPath (pixel pipeline config), alphaMode, fogMode/fogTable, lfbMode, clip windows, textureMode/tLOD/tDetail/texBaseAddr per TMU, trexInit, chromaKey/chromaRange, stipple, color0/1, zaColor, swapbufferCmd, ... |
| 2D engine ("WAX") | `SstGRegs` | blt src/dst base+format, rop, colorFore/Back, clip0/1, command, launch area, pattern regs, line/polygon regs |
| Command/AGP | `SstCRegs` | `cmdFifo0/1 { baseAddr, baseSize, bump, readPtr, aMin, aMax, fifoDepth, holeCount }`, agpReqSize/agpHostAddressLow/High/agpGraphicsAddr/agpMoveCMD, fence regs |
| Video | `SstVRegs` | vidProcCfg (overlay/desktop enable, filtering, FSAA sample combine), vidScreenSize, overlay start/end/stride, desktop start/stride, hwCurPat/Loc, CLUT access, vidPLL, dacMode, vidTvOut*, vidMaxRGBDelta, vidDesktopOverlayStride |
| I/O / init | `SstIORegs` | pciInit0/1, sipMonitor, lfbMemoryConfig, miscInit0/1, dramInit0/1 (memory timing/size/type), agpInit, tmuGbeInit, vgaInit0/1, pllCtrl0/1/2, dacRead/Write |

Access macros: `HWC_IO_LOAD/STORE` (MINIHWC), `GR_SET/GR_GET` + `REG_GROUP_*` (Glide via FIFO),
`SET/GET` (CSIM/diags direct). 2D vs 3D register space selected by packet bit 14.

## 2. MINIHWC (`H5/MINIHWC/MINIHWC.C`, 246 KB — the board manager)

### Public API (called by Glide/GL; grep-verified function list)

| Group | Functions |
|---|---|
| Discovery/map | `hwcInit(vID,dID)`, `hwcMapBoard`, `hwcInitRegisters`, `hwcReadConfigRegister`, `hwcUnmapMemory(9x)` |
| Memory/buffers | `hwcAllocBuffers(nCol,nAux)`, `hwcCheckMemSize`, `hwcAllocAuxRenderingBuffer`, `hwcBufferLfbAddr`, `hwcGetSurfaceInfo` |
| FIFO | `hwcInitFifo(enableHoleCounting)`, `hwcInitAGPFifo`, windowed: `hwcAllocWinContext/Fifo`, `hwcLock/Unlock/Free/Execute/Idle WinFifo`, `hwcExecuteStatusWinFifo` |
| Video | `hwcInitVideo(tiled, timing, ...)`, `hwcInitVideoOverlaySurface`, `hwcRestoreVideo`, `hwcResolutionSupported`, `hwcInitLookupRefresh` |
| Gamma | `hwcGammaTable/GetGammaTable/GammaRGB`, `hwcGammaCorrect` |
| Clocks | `hwcSetGrxClock`, `hwcSetMemClock` |
| Context | `hwcQueryContext`, `hwcShareContextData`, `hwcClearContextData` |
| Readback | `hwcReadBuffer565/1555/8888` (SLI/AA-aware de-interleave for screenshots) |
| Misc | `hwcGetenv` (env + registry), `hwcGetErrorString`, `hwcCheckTarget` |

### Board memory layout (established by `hwcAllocBuffers`)

```
offset 0 ──► desktop/2D surfaces & cursor
        ──► tiled color buffer 0..n (tile mark = colBuffStart0[0])
        ──► aux/depth buffer(s)
        ──► [FSAA: secondary color + depth sample buffers (colBuffStart1, lfbBuffAddr0[n])]
        ──► texture memory (remainder; per-TMU apertures on the 3D side)
top    ──► command FIFO (when local; FIFO_END_ADJUST slop below the JMP wrap)
```
Tiling: color/depth are tile-addressed (`bufStrideInTiles`, written to
`vidDesktopOverlayStride`); LFB accesses translate through `hwcBufferLfbAddr`.

### SLI/AA configuration path (Napalm multichip)

`hwcInitVideo` → build `SLI_AA_REQUEST { ChipInfo{aaEn, sliEn, sliAaAnalog, sli_nlines,
CfgSwapAlgorithm, dwChips, aaSampleHigh}, MemInfo{totalMemory, tileMark, secondary buffer
ranges, bpp} }` → deliver to kernel: **W2K/NT** via `ExtEscape(HWCEXT_SLI_AA_REQUEST)`;
**9x** via `DeviceIoControl(SLI_AA_ENABLE)` to the MiniVDD (`MINIHWC.C:4293+`, self-described
"lazy bastard" path). Constraints encoded: 4-way ⇒ analog SLI; 2-way+4xAA ⇒ digital AA with
analog SLI off; the kernel driver forgets `vidScreenSize` on slaves (re-written after request);
MiniVDD "screws with lfbMemoryConfig" (rewritten after). Per-chip PCI config writes go through
`HWCEXT_PCI_OP` with function number = chip index.

### Platform variants in the same directory

- `DOS_MODE.C` — full DOS mode-set path (uses CINIT directly, no kernel).
- `MAC_MODE.C` — MacOS name registry / Open Firmware property path.
- `LINHWC.C` + `LINDRI.H` — Linux: `/dev/3dfx` (FXMEMMAP-equivalent module) or DRM-style DRI
  with SAREA clip rects; the XF86 DRI structs are declared locally.
- `DXDRVR.C` — windowed contexts hosted by DirectDraw surfaces on Win9x.
- `HWCIO.C/HWCIO.H` — raw register I/O + PCI config plumbing per OS.

## 3. CINIT (`H5/CINIT/H3CINIT.C` — chip cold init)

Entry points (all take `regBase`, callable from BIOS-less environments):

| Function | What it does |
|---|---|
| `h3InitGetMemSize` | Probe/size SGRAM/SDRAM via dramInit patterns |
| `h3InitSgram` | Memory controller timing + type setup (defaults to SGRAM if strap undefined) |
| `h3InitPlls` / `h4InitPlls` | Program grx/mem/vid PLLs from `PLLTABLE.H` / `H4PLL.H` (+`H4OEMPLL.H` OEM speed grades). **h4 = Napalm** |
| `h3InitVga`, `h3InitVideoProc` | VGA core init, video processor config |
| `h3InitFindVideoMode` / `h3InitSetVideoMode` | Look up + program full mode timing from `MODETABL.H` (all resolutions/refreshes incl. Q3D specials) |
| `h3InitVideoDesktopSurface` / `h3InitVideoOverlaySurface` | Scanout surface config (stride/tiling/format) |
| `h3InitMeasureSiProcess` | Ring-oscillator silicon speed binning (`sipMonitor`) |
| `h3InitBlockWrite` | SGRAM block-write enable for fast clears |
| `h3InitResetAll` | Ordered full-chip reset |

The **same file is compiled into** the Win9x MiniVDD (`H5/Win9x/DX/MINIVDD/H3CINIT.C`), the
NT/W2K miniports, DOS Glide, and diags — single source of init truth. Port this file first.

## 4. HAL (`H5/HAL/` — diagnostics HAL, *not* the driver stack)

`fxHal*` API used by DIAGS/PERL/CSIM front-ends: `FXHAL.C` (`fxHalInit`,
`fxHalNumBoardsInSystem`, board select, init), `HALIO.C` (register/LFB peek-poke incl. ranges),
`AGPIO.C` (AGP aperture access), `KERNEL.C` (ring-0 access via FXMEMMAP/FXPTL),
`VIDEO.C`, `INFO.C` (board/chip reporting), `GUIM.C`/`GUIV.C`/`GUI.RC` (Windows monitor GUI),
`HALCTRL.ASM`, `MAKEFVXD` (VxD build). Distinct from and older than MINIHWC — keep them
straight: **MINIHWC is what shipping Glide uses; HAL is the diag stack.**
`SWLIBS/HWC` is a third, older context library used by some manufacturing tools.

## 5. BIOS (`H5/BIOS/`)

Real-mode x86 (MASM): `BIOSNEWS.ASM` main POST flow, `BIOSEQU.INC`/`BIOSPARM.INC` equates,
`CLKPLL.INC` PLL programming, `DACDATA.INC`, `DATA.ASM`/`FFF0DATA.ASM` (entry vectors),
`CURS.ASM` text cursor, `ALT.ASM` alternate functions, VBE mode list (`Modelist.doc`),
per-board OEM config tables (`New OemConfig table scheme.xls`), scratch-register conventions
(`Scratch register usage.xls`), `FLASH/` DOS flasher, `buildtools/`. Board-name/build-option
matrix in `Napalm Board BIOS Names and Build opts.doc`.

## 6. Kernel services (SWLIBS side)

| Component | Role |
|---|---|
| `SWLIBS/FXMEMMAP` (`FXMEMMAP.VXD`) | Win9x: map BARs to user space, MTRR write-combining, contig memory for host FIFOs. API in `FXMEMMAP.H` |
| `SWLIBS/FXNTDRVR/FXPTL` | NT: equivalent ring-0 mapping/port service |
| `SWLIBS/FXAGP`, `SWLIBS/FXGART` | AGP aperture setup + GART page-table programming (host FIFO in AGP space, PKT6 AGP MOVE sources, AGP texturing) |
| `SWLIBS/FXPCI`/`NEWPCI` | PCI config access from user mode (diags) |
| `SWLIBS/FXDPMI` | DOS: flat mapping + `P6.ASM` MTRR/WC setup without an OS |

## 7. Device identification

- `IS_NAPALM(deviceID)` = ID within `[SST_DEVICE_ID_L_AP, SST_DEVICE_ID_H_AP]`
  (`MINIHWC.H:513`). Vendor 0x121A (3dfx): Banshee=0x0003, Voodoo3=0x0005, VSA-100
  (Voodoo4/5)=0x0009 (per `fxhal.h`/INF files; the INFs in `*/Inf/Voodoo3|Voodoo5` bind
  specific subsystem IDs).
- `bInfo->pciInfo.numChips` (1/2/4) drives SLI/AA logic; chips are PCI functions 0–3 behind
  the board's bridge on Voodoo5.

## 8. Bring-up order for a new driver (mirrors what this code does)

1. PCI enumerate; match vendor/device; read BARs (`hwcInit`, `hwcMapBoard`).
2. Map register BAR uncached; run `h3InitResetAll` → `h3InitSgram` → `h(3|4)InitPlls` →
   `h3InitGetMemSize`.
3. Program a video mode (`h3InitFindVideoMode`/`SetVideoMode` + desktop surface) — you now
   have scanout.
4. Map LFB write-combined; verify LFB reads/writes (DIAGS `FBI/LFB` tests are the reference).
5. `hwcInitFifo` a small local FIFO with hole counting off; send PKT1 NOP/status writes;
   verify readPtr advances (PERL `CMDFIFO.PL` is the reference sequence).
6. PKT4 state init (copy Glide's `grSstWinOpen` initial state), PKT3 one triangle. Compare
   against CSIM output.
7. Only then: hole counting on, AGP FIFO (Napalm only), SLI/AA (V5), overlay, TV-out.
