# 02 — Complete Directory Inventory

Every directory in the tree, what it contains, and why it exists. ~7,100 files / ~344 MB / 535
directories. Paths relative to repo root.

## Top level

| Entry | Type | Purpose |
|---|---|---|
| `MAKEFILE`, `makefile.unix` | build | Master recursive make. `FX_GLIDE_HW` ∈ {SST1, SST96, CVG, H3, H5} selects the hardware subtree; only `h5` + `swlibs` exist in this snapshot |
| `SETENV.BAT` | build | Win9x driver build env (MSVC6 + MSVC1.52 + MASM 6.14 + Win95 DDK + DX7 DDK + Win32 SDK; `DEVTOOLS` root) |
| `setdosenv.bat` | build | DOS build env (Watcom, DOS4GW) |
| `TOOLS.BAT` | build | Resets PATH/LIB/INCLUDE, sets `DEVTOOLS=c:\3dfxtools`, Quantum3D SDK path |
| `W2kEnv.bat`, `Nt4Allen.bat` | build | W2K / NT4 driver build envs (DDK `build.exe` style) |
| `Q3dEnv.cmd`, `Q3dGlideEnv.cmd`, `Q3dDosEnv.cmd`, `Q3dDiagsEnv.cmd` | build | Quantum3D variants; set `FX_GLIDE_HW=H5`, `FX_HW_PROJECTS=glide3`, `FX_TARGET=WIN32`, `FX_COMPILER=MICROSOFT`, `FX_DLL_BUILD=1` |
| `BuildDiags.bat/.cmd`, `pulldiags.bat`, `pulldiags2.bat` | build | Build + package the diagnostics kit (swlibs → incsrc → diags; copies headers/libs into `diags/mftg/3dfx`) |
| `README.TXT`, `MANIFEST.TXT` | doc | Original *Glide2/Glide3 Diagnostics Kit* readme + manifest for the `3Dfx/` binaries |
| `NAPALM.XLS`, `super_2116.xls` | doc | Engineering spreadsheets |
| `opengl1_2_1.ps` | doc | OpenGL 1.2.1 specification |
| `diags.diff`, `diags_done.diff` | log | **Not diffs** — saved MKS/grep search logs from `D:\OfficialBuild\AAlchemy3\MfgTests\src`, i.e. a **Quantum3D AAlchemy** manufacturing-test build server. Establishes the archive's provenance |
| `Disabled` | marker | 0-byte marker file (likely "this tree disabled from auto-build") |
| `3Dfx/` | binaries | Prebuilt SDK kit: `Diags/{Win95,WinNT}/detect.exe,pcirw.exe`; `Sdk/Glide2x/Bin` (+`/DOS`) & `Sdk/Glide3x/Bin` test00–test38 + `.3DF` textures; `Utils/{Win95,WinNT}/pass.exe` |

## `H5/` — Napalm-generation hardware tree

### Headers / contracts

| Dir | Contents |
|---|---|
| `H5/INCLUDE/` | Canonical driver-facing headers: `H3.H` (top include), `H3DEFS.H` (chip constants), **`H3REGS.H`** (register map: `SstRegs`, `SstGRegs` 2D, `SstCRegs` cmd/agp, `SstVRegs` video, `SstIORegs` PLL/DAC), **`H3GDEFS.H`** (packet + 2D-engine encodings), `H3HWC.H`, `H3INFO.H`, `SST1VID.H` (video timing structs), `CMDDEFS.H`, `FXVID.H`, `FXHAL.H`, `GDEBUG.H`, `VECTOR.H` (vtable for kernel services), `VXD.H`, `MINIHWC.H`, `HWCEXT.H` (display-driver escape API), `GLIDE.H`/`GLIDESYS.H`/`GLIDEUTL.H`, `G3EXT.H`, `SETMODE.H`, `OEMINIT.H`, `GENDATE.*` |
| `H5/INCSRC/` | Source-of-truth copies of the above that `nmake` publishes into `INCLUDE` (plus `GENDATE.C` build-date generator) |

### 3D API runtimes

| Dir | Contents |
|---|---|
| `H5/GLIDE/SRC/` | **Glide 2.x** (44 source files): API entry (`GGLIDE.C`, `GDRAW.C`, `GTEX.C`, `GU*.C` utility layer incl. `GUMP.C` mipmap mgmt), device-independent split (`DI*.C`), `FIFO.C`, `GXDRAW.C` C trisetup, `XDRAW2.ASM` + `XTEXDL.ASM` (3DNow!), `CPUDTECT.ASM`, `GMOVIE.C` (frame capture), splash (`GSPLASH.C`, `SPLSHDAT.C`), Watcom/DOS support. Builds `glide2x.dll` / DOS OVL |
| `H5/GLIDE/` (top) | `PRODMSTR.TXT` — production master packing list for the Glide2 SDK/OEM kits |
| `H5/GLIDE3/SRC/` | **Glide 3.x** (107 files) — the primary runtime. See [03-GLIDE-RUNTIME.md](03-GLIDE-RUNTIME.md) for file-by-file docs |
| `H5/GLIDE3/OEM/` | `FXOEM2X.DLL` OEM-branding plug-in (`OEMINIT.C`) + `FXBLDNO.C` build-number stamping tool |
| `H5/GLIDE3/CONFORM/` | Glide conformance suite: `TESTS/{RASTER,STATE,TEXTURE,CONTROL,extensions}`, `Runner/` GUI, `BMPCMP`/`LOGCMP` compare tools, golden templates |

### Hardware bring-up / low level

| Dir | Contents |
|---|---|
| `H5/MINIHWC/` | Board management layer under Glide — see [04-HARDWARE-LAYER.md](04-HARDWARE-LAYER.md). `MINIHWC.C` (246 KB), `DOS_MODE.C`, `MAC_MODE.C`, `LINHWC.C` (Linux, incl. DRI structs `LINDRI.H`), `DXDRVR.C` (DirectDraw-hosted contexts), `HWCIO.C` register I/O, `INITVGA.H`, `QMODES.H` Quantum3D modes, `TEST/` |
| `H5/CINIT/` | Chip cold-init C library: `H3CINIT.C`, `H4PLL.H`/`H4OEMPLL.H` (Napalm PLL tables), `PLLTABLE.H`, `MODETABL.H` (all video modes), `MEMTABLE.H`; `BUILD.BAT`/`BUILDNT.BAT` |
| `H5/cinit-old/` | Previous CINIT revision kept for reference |
| `H5/BIOS/` | Video BIOS: `SRC/*.ASM` (real-mode x86: POST, VGA core, mode set, PLL, DAC, cursor, OEM tables), `FLASH/` flash updater, `buildtools/`, docs (`Modelist.doc`, board BIOS naming, scratch-register usage) |
| `H5/HAL/` | User-mode diagnostics HAL (`fxHal*`): `FXHAL.C` (board enum/init), `HALIO.C` (register/LFB access), `AGPIO.C`, `KERNEL.C` (ring-0 access via VxD/driver), `VIDEO.C`, `INFO.C`, `GUIM.C`/`GUIV.C` monitor GUI, `HALCTRL.ASM`. Used by DIAGS/PERL, **not** by shipping drivers |
| `H5/CSIM/` | C-model chip simulator — see [08-DEBUGGING-AND-DIAGNOSTICS.md](08-DEBUGGING-AND-DIAGNOSTICS.md) §5 |
| `H5/NICKTEST/` | An engineer's scratch dir: private header snapshot + makefile (no unique code of value) |

### Diagnostics & tools

| Dir | Contents |
|---|---|
| `H5/DIAGS/` | Hardware validation. `FBI/` (93 tests: blending, clamping, iterators, LFB, AGP, dither…), `TREX/` (texture unit: formats, mipmaps, combine, cmdfifo stress `cfestress.c`), `VID/` (video: CLUT, bilerp, 411/422), `GUI/` (2D engine), `LIB/` diag framework, `BRINGUP/` scripted board bring-up (+`TOOLS/pciScan`, `runDiags`, `statusClient`), `CSIMTEST/`, `TOOLS/`, `MUSTPASS.BAT` release gate, `MFTG/` manufacturing suite (Lua-scripted test clients, `BIN/UNREAL` recorded-game streams, `BIN/3DF|GLD|LUA`, VGA core tests, `GPLAY` stream player) |
| `H5/PERL/` | Bring-up Perl: `REGTEST.PL`, `LFBR/LFBW.PL`, `CMDFIFO.PL`, `SIP.PL` (serial interface port), `READPLL.PL`, `AGPINFO.PL`/`AGPWALK`, `memdiag80/90.pl`, `boardtest*.pl` master/slave rigs, `fifo_replay.pl`, `3DVIDEO.PL`; `extension/` = Perl XS module giving Perl direct PCI/register access |
| `H5/UTIL/` | Shared test-app helpers: `SSTIMAGE.C` (TGA/3DF image load/save), `UI.C`, `CONTROLS.C`, `SWAP.C` |

### OS drivers (see [05-OS-DRIVERS.md](05-OS-DRIVERS.md))

| Dir | Contents |
|---|---|
| `H5/Win9x/DX/` | Windows 9x driver set: `DD16/` (16-bit GDI/DIBENG display driver), `DD32/` (32-bit DirectDraw HAL, SLI 2D, memory manager, mode/gamma, bad-app system), `D3D/` (Direct3D HAL, 125 files), `MINIVDD/` (kernel VxD: PCI/AGP/GART, modeset via embedded `H3CINIT.C`, I2C, TV encoders BT868/Chrontel, DFP, IRQ, NVRAM), `INC/` shared headers, `INF/Voodoo3|Voodoo5` install files + help, `BUILD/`, `RES/`, `DOC/`, `BIN/{DEBUG,RETAIL}` prebuilt |
| `H5/WinNT/` | NT 4.0: `Src/Video/Miniport/H5` (modeset/I2C/TV), `Src/Video/Displays/H5` (2D + DirectDraw + Glide escape support; **no D3D**), `Inf/`, `BUILD/` |
| `H5/W2K/` | Windows 2000: same layout as WinNT but display driver additionally contains the **full D3D driver** (152 files incl. `D6*/D7*` and the SIMD T&L), `Inf/Voodoo3|Voodoo5` |
| `H5/MacOS8/` | Mac stack: `GDX/` video driver, `GLIDE2.X/`+`GLIDE3.X/` ports, `RAVE/` QD3D driver, `OpenGL/MCD_Glide` (+`_Exp`), `QD_QT_Acceleration/`, `zH3_2D_ACCELERATION_3dfx/`, `ControlPanel/` (EN/FR/JP), `FlashROM/`, `OpenFirmware/`, `Installer/`, `hardware_resource_manager/`, `BuildScripts/` (deliverable specs per product: MacVoodoo, NAPALM, voodoo3_beta), `release_notes/`, `shared_headers/`, `shared_powerplant/` (Metrowerks PowerPlant) |

### Build outputs

| Dir | Contents |
|---|---|
| `H5/BIN/` | Prebuilt `GLIDE3X.DLL`, `FXOEM2X.DLL` (+ makefiles) |
| `H5/LIB/`, `H5/LIBSRC/`, `H5/BINSRC/` | Library/binary staging dirs (makefile plumbing) |
| `H5/DOCS/` | `DATABOOK.PDF`, `SW Architecture Spec.doc`, `avenger_spec.doc`, `PS_Napalm_*` product specs, `Napalm registry keys.doc`, `3dfx Conditional Compile Flags.doc`, validation plans/results, `Napalm Performance Development.mpp`, `Rampage_Q3D_083199.ppt`, `D3D.XLS`, `Video SLI AA Configs.xls`, `TRACKING/` |

## `SWLIBS/` — shared libraries & major clients

### Build/infra

| Dir | Contents |
|---|---|
| `SWLIBS/INCLUDE/` | Public shared headers (`glide.h` family, `3dfx.h`, `sst1*.h`) + **`NMAKE/`** and **`MAKE/`** — the entire make infrastructure (`3dfx.mak`, `3dfx.linux.mak`, per-target rules) |
| `SWLIBS/INCSRC/` | Source copies published into INCLUDE |
| `SWLIBS/BIN/`, `BINSRC/` | Build tools: flex, bison, `WING32.DLL`, packaging batch files |
| `SWLIBS/LIB/`, `LIBSRC/` | Library staging |
| `SWLIBS/DOCS/` | `TEXUS.DOC`, `PCI.DOC`, `INITHAL.DOC`, `DLL_USE.DOC`, `FLATWARE.DOC` |

### Kernel/system services

| Dir | Contents |
|---|---|
| `SWLIBS/FXMEMMAP/` | `FXMEMMAP.VXD` — Win9x mapping service: maps BARs into flat user space, programs **MTRR write-combining**, provides physical/linear translation. The single most important kernel dependency for Glide on 9x |
| `SWLIBS/FXPCI/` + `SWLIBS/NEWPCI/` | PCI config-space access library (`PCILIB`) + `PCITOOLS` (pcirw etc.); NEWPCI is the newer rewrite |
| `SWLIBS/FXAGP/` | AGP master/aperture library |
| `SWLIBS/FXGART/` | GART programming + sample VxD control code |
| `SWLIBS/FXDPMI/` | DOS DPMI helpers (`P6.ASM` = MTRR/WC from DOS) |
| `SWLIBS/FXNTDRVR/` | NT kernel helpers: `FXPTL` (port-talk ring-0 access), `FXGPIO`, `NTRemap` |
| `SWLIBS/FXREMAP/`, `FXMISC/`, `FXMAC/`, `CSERVICE/`, `DOSDLL/`, `ATBNET/` | Address remapper; misc (64-bit math `FX64`, ATSC); Mac PCI shims; client/server IPC service (docs incl. PDF); DOS DLL loader on Watcom runtime; IPX/UDP networking for ATB |
| `SWLIBS/HWC/` | Older hardware-context lib (pre-MINIHWC; used by some diags) |
| `SWLIBS/GDIBYP/` → see OPENGL below | |

### 3D software

| Dir | Contents |
|---|---|
| `SWLIBS/OPENGL/` | The OpenGL effort: `SRC/` (core GL + `DRIVERS/{SST,S3,DDRAW}` + `WGL/` + `GENERATE/` code-gen + `ICDDIST/` installer + `TOOLS/`), `GLIDE2X/` & `GLIDE3X/` (the two shipped ICD variants layered on Glide2/Glide3: `GLCORE`, `RASTER`, `SST`, `MGL`, `WGL`, `TRACE`), `GDIBYP/` (GDI-bypass display hook for windowed GL), `SYSTRAY/` tray applet, `BENCH/` (`trispeed`, `bindspeed`, `QUAKE.XLS`), `CONFORM/` (OpenGL conformance + covgl/covglu/primtest), `OGTST/` (in-house GL regression suite, hundreds of tests by area), `EXAMPLES/` (red-book + demos + screensavers `SCRSAVE/{3DFO,FLWBOX,MAZE,PIPES,TEXT3D}`), `TESTS/`, `Docs/` |
| `SWLIBS/3DFXGL/` | **MiniGL** (`LIBGL/` — the fxgl subset GL for glQuake-era games), `GLUT/`, demos (`PROGRAMS/DEMOS`: atlantis, glutmech…), `STATUE/`, `TD/`, `microsoft/` (MS GL headers) |
| `SWLIBS/ARCADE/` | **ATB Arcade ToolBox** (Quantum3D): optimized T&L/clip pipeline over Glide. `SRC/AT{A,C,D,G,I,M,N,R,S,U}` modules, `DRIVERS/{GLIDE,D3D,RAVE,DTRI}` back-ends, MultiGen/3DStudio importers, `CMD/TOOLS` (BENCH, BROWSE, VIEW, 3DSTOMFF), `DATA/` models+textures, full design docs in `DOCS/` |
| `SWLIBS/TEXUS/` | Texture Utility System v1: `LIB/` (3DF format, mipmap gen, palettes, NCC quantizer, dequant, diffusion dither, TGA/PPM I/O), `CMD/` texus.exe CLI, `EXAMPLES/`, `TESTS/` |
| `SWLIBS/TEXUS2/` | TEXUS v2: adds **FXT1** codec (`CODEC.C`, `BITCODER.C`, `EIGEN.C` PCA, `SST2FXT1.H`, `SST2COMP.H`), neural-net NCC trainer (`NCCNNET.C`), viewer |
| `SWLIBS/GlideTrap/` | Glide call **tracer/replayer**: `TRAP/glidetrap.c` (interposer DLL that records every Glide call + CRC-deduped textures to `.000` streams), `PLAY/glideplay.c`, `UTIL/FILEBUFF.C`, `DESIGN.TXT`, `fileformat.txt` |
| `SWLIBS/3DfxSplash/`, `3DfxSplashOld/`, `NuSplash/` | The rotating-logo splash played at Glide init: 3DS MAX exporter plugin (`splashplug`), scene/map assets, playback lib (`GSPLASH.C` consumes this) |
| `SWLIBS/GAMEGEN/` | MultiGen "GameGen" flight-format (.flt-era OP_* opcodes) model reader/converter |
| `SWLIBS/3DSR4/` | 3D Studio R4 (DOS) render driver + joystick calibration |
| `SWLIBS/MRI/` | I2C reference implementation + docs |
| `SWLIBS/G2G3/` | Glide2→Glide3 API mapping stubs (`GSTUB.C`) |

## `3Dfx/` — prebuilt diagnostics kit (binaries only)

Matches `MANIFEST.TXT`: Glide2x/Glide3x SDK test binaries (`test00.exe`–`test38.exe`, DOS
variants with DOS4GW), sample `.3DF` textures, `detect.exe` (board detect), `pcirw.exe`
(PCI config read/write), `pass.exe` (Voodoo1/2-only pass-through util — **do not run on
V3/V5**, per README.TXT warning).
