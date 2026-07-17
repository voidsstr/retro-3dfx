# 01 — System Architecture

How the entire stack fits together, from a game's API call to photons leaving the monitor.

## 1. The stack

```
                            GAME / APPLICATION
      ┌──────────────┬───────────────┬────────────────┬──────────────┐
      │  Glide 2.x   │   Glide 3.x   │   Direct3D     │   OpenGL     │
      │  H5/GLIDE    │   H5/GLIDE3   │  (DX6/7 HAL)   │  ICD/MiniGL  │
      │              │               │  H5/Win9x/DX/  │  SWLIBS/     │
      │              │               │  D3D, W2K disp │  OPENGL,     │
      │              │               │  driver        │  3DFXGL      │
      └──────┬───────┴──────┬────────┴───────┬────────┴──────┬───────┘
             │              │                │  (GL layers on Glide3)
             │              │                │               │
             ▼              ▼                ▼               ▼
   ═════════════════ COMMAND TRANSPORT (software FIFO) ═════════════════
     CPU writes typed packets into a ring buffer in card memory or AGP
     memory through a WRITE-COMBINED mapping, then "bumps" the chip.
     Implemented in: H5/GLIDE3/SRC/FIFO.C + FXCMD.H (Glide),
     H5/Win9x/DX/D3D/AFIFO.C + W2K AFIFO.C/CFIFO.C (D3D/2D)
   ═════════════════════════════════╤════════════════════════════════════
                                    │
        ┌───────────────────────────┴───────────────────────────┐
        │  BOARD MANAGEMENT (user mode): H5/MINIHWC/MINIHWC.C   │
        │  PCI discovery, BAR mapping, buffer/tile allocation,  │
        │  FIFO init, SLI/AA config, video modes, gamma         │
        └───────────────────────────┬───────────────────────────┘
                                    │ ExtEscape (9x/NT display) /
                                    │ DeviceIoControl (MiniVDD) /
                                    │ ioctl (Linux DRI)
        ┌───────────────────────────┴───────────────────────────┐
        │  KERNEL: MiniVDD (Win9x VxD, H5/Win9x/DX/MINIVDD)     │
        │          Miniport (NT4/W2K, */Src/Video/Miniport)     │
        │          FXMEMMAP.VXD (SWLIBS/FXMEMMAP) — maps the    │
        │          LFB/FIFO apertures user-side and programs    │
        │          MTRRs for WRITE-COMBINING                    │
        └───────────────────────────┬───────────────────────────┘
                                    ▼
   ┌────────────────────────────────────────────────────────────────┐
   │ HARDWARE — Avenger (V3) / VSA-100 (V4/V5)                      │
   │                                                                │
   │  PCI/AGP target ─ CMDFIFO engine (pull) ─┬─ 2D engine ("WAX")  │
   │                                          └─ 3D pipeline:       │
   │     Triangle Setup Unit (slopes/gradients from raw verts)      │
   │       → FBI (edge walk, Z/alpha/dither/blend, LFB access)      │
   │       → TREX (texture: bilinear/trilinear, LOD, FXT1/NCC       │
   │          decompress, 2 TMU-equivalent pipes per VSA-100)       │
   │  Video unit: overlay + desktop scanout, gamma, RAMDAC          │
   │  SLI: 1–3 slave chips render interleaved scanline bands;       │
   │       FSAA: multi-sample buffers combined at scanout           │
   └────────────────────────────────────────────────────────────────┘
```

### Where each API enters

- **Glide** is the native API: thinnest possible shim over the packet FIFO. Games (Quake glide,
  Unreal, Falcon, etc.) call `grDrawTriangle`/`grDrawVertexArray*` and Glide writes packets.
- **OpenGL** ships two ways: the full **ICD** (`SWLIBS/OPENGL`) whose back-end (`SST/` dirs)
  emits Glide3 calls, and the lighter **MiniGL** (`SWLIBS/3DFXGL`) that implements the subset
  id-tech games used, also over Glide.
- **Direct3D** does *not* go through Glide. The D3D HAL (`H5/Win9x/DX/D3D`, and the same code
  compiled into the W2K display driver) builds packets directly with its own assembly
  primitive writers, sharing only the packet encodings and kernel services.
- **DirectDraw/GDI 2D** uses the 2D engine via PKT1/PKT4-style register writes from DD32/DD16
  and the NT display drivers.

## 2. The hardware programming model

All chip work is driven by writes into a **command FIFO** parsed by the chip's pull engine.
Canonical encodings: `H5/INCLUDE/H3GDEFS.H` (mirrored in `INCSRC`, `Win9x/DX/INC`, W2K display).

### Packet types

| Type | Header fields | Purpose |
|---|---|---|
| **PKT0** | func[5:3], addr[28:6] | FIFO control flow: `NOP`, `JSR`, `RET`, `JMP_LOCAL`, `JMP_AGP`. The FIFO is programmable: ring wrap is a planted JMP; windowed rendering splices per-window sub-FIFOs with JSR/RET |
| **PKT1** | regbase[13:3], 2D-space bit 14, inc bit 15, nwords[31:16] | N consecutive writes to one register (or auto-incrementing range) |
| **PKT2** | mask[31:3] | Masked write across a fixed 29-register group (legacy CVG-style) |
| **PKT3** | cmd[5:3] (`BDDBDD`,`BDDDDD`,`DDDDDD`), numVertex[9:6], pMask[21:10], sMode[27:22], packedColor bit 28 | **Hardware triangle setup.** Header + raw vertex floats. cmd encodes strip continuation semantics; pMask says which parameters (x,y,z,w,r,g,b,a,s0,t0,w0,s1,t1,w1) follow per vertex |
| **PKT4** | regbase[13:3], 2D bit 14, mask[28:15] | 14-bit masked register-group write — the workhorse for 3D state updates |
| **PKT5** | space[31:30] (LFB/YUV/3DLFB/TEXPORT), byteN masks, nwords[21:3] | Bulk linear data: LFB image writes, YUV planes, **texture download through the FIFO** |
| **PKT6** | space, nbytes, src base/stride, dst offset/stride | **AGP MOVE** — chip-initiated DMA from host/AGP memory into LFB or texture space (strided 2D) |

`SSTCP_REGBASE` is 10 bits on H3-class parts and 11 bits under `#ifdef H4` (Napalm's grown
register file). Two FIFOs exist (`cmdFifo0/1` in `SstCRegs`); each has `baseSize` flags:
`SST_EN_CMDFIFO`, `SST_CMDFIFO_AGP` (FIFO lives in AGP space), `SST_CMDFIFO_DISABLE_HOLES`.

### Hole counting & fences

With a write-combined mapping, CPU stores can reach the card out of order. The FIFO engine
therefore counts *arrived dwords* ("hole counting") rather than requiring sequential arrival.
Hardware constraint: the hole counter needs a serializing flush at least every 64 KB of writes —
Glide fences via `P6FENCE` (`FXGLIDE.H:2094`, a locked `xchg` on x86; `__sync()` on PPC) inside
`GR_CHECK_FOR_FENCE` (`FXCMD.H:285`), limit tunable by `FX_GLIDE_FENCE_LIMIT` (≤ 0x10000).

### The bump protocol

The chip only knows about bytes the driver has **bumped**: `GR_BUMP_N_GRIND` (`FXCMD.H:258`)
writes `fifoPtr - lastBump` to the `bump` register. Default is auto-bump every
`bumpSize` = 64 KB (0x1000 on PPC); manual mode via `FX_GLIDE_BUMP`/`FX_GLIDE_BUMPSIZE`. The
driver tracks `fifoRoom` in software and only reads the hardware read-pointer when it runs out
(`_grCommandTransportMakeRoom`; the header calls hardware reads "*SLOW*" — see `FXCMD.H:222`).

## 3. Frame lifecycle (fullscreen Glide game)

1. **Context open** — `grSstWinOpen` (`GSST.C`): picks resolution/refresh from mode tables,
   calls MINIHWC to allocate 2–3 color buffers + aux (depth) buffer (tiled memory), initializes
   the FIFO (`hwcInitFifo`/`hwcInitAGPFifo`), writes full initial register state, computes
   `tsuDataList`/`curVertexSize` from the vertex layout.
2. **State changes** — API calls write shadow copies (`gc->state.shadow`) and set an invalid
   flag; nothing is sent yet.
3. **Draw call** — `grDrawTriangle` → `gc->triSetupProc` (pre-selected specialization).
   If state dirty: `GR_FLUSH_STATE` emits one PKT4 masked burst. Then cull test (software,
   sign-bit of 2D area), `GR_SET_EXPECTED_SIZE` (arithmetic FIFO-room check), PKT3 header +
   vertex float copy via `TRI_SETF` loop over `tsuDataList`, `fifoRoom -=`, occasional bump.
4. **Texture download** — `grTexDownloadMipMap` → per-CPU/format proc → PKT5 TEXPORT stream
   (pipelines behind drawing; no idle wait).
5. **Swap** — `grBufferSwap(interval)`: PKT4 write to `swapbufferCmd` with swap-pending count
   check (`FX_GLIDE_SWAPPENDINGCOUNT`, clamp 0–3) so CPU can run ahead ≤ N frames; hardware
   flips at vblank honoring the swap interval. SLI slaves sync via their own mapped registers
   (`FIFO.C` slave-FIFO sync, 01/2000 changelog entries).
6. **Scanout** — video unit merges SLI bands / averages FSAA samples; overlay path is borrowed
   for pipelined flipping in the OS drivers (`DDFXS32.C`).

## 4. Initialization sequence (cold boot → running driver)

1. **BIOS POST** (`H5/BIOS/SRC`): real-mode init — DRAM sizing, PLLs, VGA core, mode tables,
   OEM config from board straps/EEPROM.
2. **OS driver load** — miniport (NT/W2K) or MiniVDD+display driver (9x) binds to PCI IDs
   (INFs in `*/Inf/Voodoo3`, `*/Inf/Voodoo5`).
3. **Chip init library** — `H5/CINIT/H3CINIT.C` (shared by BIOS-less contexts, diags, DOS,
   and the drivers): `h3InitGetMemSize`, `h3InitSgram`, `h3InitPlls`/`h4InitPlls`
   (`PLLTABLE.H`), `h3InitSetVideoMode` (`MODETABL.H`), `h3InitMeasureSiProcess` (reads a
   ring-oscillator to bin silicon speed), `h3InitResetAll`.
4. **Aperture mapping** — kernel maps: register BAR (uncached), LFB BAR (write-combined via
   MTRR — FXMEMMAP.VXD on 9x, miniport on NT, `P6STUFF.ASM`/`K6_2.ASM`/`PENT.ASM` per-CPU).
5. **MINIHWC bring-up** (user side): `hwcInit(vID,dID)` → `hwcMapBoard` → `hwcInitRegisters`
   → `hwcAllocBuffers` (color/aux, tile marks) → `hwcInitFifo` → `hwcInitVideo` (incl. the
   SLI/AA request into the kernel driver for multi-chip Napalm — `HWCEXT_SLI_AA_REQUEST`).
6. **API runtime attach** — Glide `_GlideInitEnvironment` (`GPCI.C:1525+`): CPU detect, proc
   table selection, env-var config, `_grSstDetectResources`.

## 5. Multi-chip (Voodoo5) architecture

- **SLI**: scanline-band interleave, band height 2–128 lines (`h3sliBandHeight`). Geometry is
  broadcast (each chip sees all packets; each renders only its bands). Memory is *not* shared:
  each VSA-100 has its own 16/32 MB — textures are replicated, effective texture memory is one
  chip's worth.
- **Digital vs analog SLI**: 2-way can combine digitally; **4-way (Voodoo5 6000) must use
  analog combining** (`MINIHWC.C:4280`: "For 4-way SLI we must use analog SLI").
- **FSAA**: 2x/4x/8x rotated-grid supersampling; secondary color/depth buffers per sample
  (`colBuffStart1`, secondary depth ranges passed in the SLI/AA request); samples averaged at
  scanout. AA and SLI trade off against buffer memory — hence the promote/demote logic and the
  per-game config table documented in [05-OS-DRIVERS.md](05-OS-DRIVERS.md) §4.
- The kernel driver applies the config to all chips (PCI config writes per function number,
  `HWCEXT_PCI_OP` loop in `MINIHWC.C`).

## 6. Windowed vs fullscreen rendering

Fullscreen gets the big linear FIFO and exclusive state. Windowed rendering (NT/9x desktop):
- Per-window FIFO contexts (`hwcAllocWinContext`, `hwcAllocWinFifo` — host, local, or AGP
  backed: `HWC_WIN_FIFO_{HOST,LOCAL,AGP}`), executed by splicing into the master stream
  (`hwcExecuteWinFifo`), with a "tail chasing fifo" scheme (FIFO.C rev 24) and a lost-context
  dword shared from the kernel (`hwcShareContextData`) checked per begin — cheap detection of
  desktop mode switches without a kernel round trip.

## 7. Key architectural invariants (do not violate when modifying)

1. FIFO writes are 4-byte aligned; never let the write pointer cross `fifoEnd - FIFO_END_ADJUST`
   without planting the wrap JMP (`FIFO_END_ADJUST` reserves 8 dwords; comment: "Its actually a
   little bit bigger just in case someone does not read this comment").
2. Every `GR_SET_EXPECTED_SIZE` must be matched by exactly that many packet bytes
   (`GLIDE_SANITY_SIZE` debug builds assert this — keep it on during development).
3. Fence at least every 64 KB of WC writes (hole-counter hardware limit).
4. PKT3 vertex count ≤ 15; parameter order must match the sMode/pMask encoding.
5. State registers are written only via the shadow → `GR_FLUSH_STATE` path; a direct write that
   bypasses shadows desynchronizes redundancy elimination.
6. On SLI, anything written outside broadcast (per-chip register mappings) must be replicated
   to slaves; several fixes in the changelogs are exactly this class of bug (e.g.
   `vidScreenSize` re-write after the W2K miniport "doesn't copy this value to the slave chips").
