# 07 — Toolchain & Build Documentation (Phase 1: building as-is)

Everything needed to reproduce period builds, decoded from the env scripts and makefiles in
this tree. All version requirements below are stated by the scripts themselves.

## 1. Required tools (the `DEVTOOLS` directory)

`TOOLS.BAT` sets `DEVTOOLS=c:\3dfxtools`; `SETENV.BAT` and friends expect this layout:

| Env var | Path under DEVTOOLS | Tool | Used for |
|---|---|---|---|
| `COMP32` | `msvc6_0\vc98` | **Microsoft Visual C++ 6.0** | All 32-bit C/C++ (Glide, D3D, DD32, ICD, tools) |
| `COMP16` | `msvc1_52` | **Microsoft Visual C++ 1.52** | 16-bit code: DD16 display minidriver, VxD pieces |
| `MASM` | `masm614` | **MASM 6.14** | All `.ASM` (Glide 3DNow paths, D3D tri/T&L asm, BIOS, VxD asm) |
| `DDK9X` | `win95ddk` | **Windows 95 DDK** | DD16/MiniVDD |
| `W9XDDK` | `Win98Ddk\1999-07` | **Windows 98 DDK** (July 1999) | Win9x DX driver set |
| `DDKDX` | `dx7ddk` | **DirectX 7 DDK** | D3D/DDraw HAL headers |
| `DIRECTXSDK` | `dx7asdk` | DX7a SDK | D3D user-side |
| `SDK32` | `win32sdk` | Win32 SDK | Resources, tools |
| `W2KDDK` | `w2kddk` | **Windows 2000 DDK** | W2K miniport + display (`build.exe`) |
| (NT4) | NT4 DDK | **NT 4.0 DDK** | WinNT tree (`Nt4Allen.bat` era) |
| `SIW95` | `SIW95` | SoftICE for Win95 | Debug-symbol builds (NMS files; optional) |
| — | Watcom C/C++ (10.x/11) + DOS4GW | `setdosenv.bat`, `WATCOM` var | DOS Glide + SDK samples |
| — | MKS Toolkit (`MKS_PATH=c:\DevTools\mks`) | Unix-ish shell utils in build scripts (from `diags.diff`: Quantum3D layout) | Diags/MFTG builds |
| — | Perl 5 (Win32) | `H5/PERL`, awk scripts in DIAGS | Bring-up tooling |
| — | Metrowerks CodeWarrior + MPW | `H5/MacOS8/BuildScripts` | Mac stack only |
| — | gcc/binutils (Linux) | `makefile.linux` chain | Linux Glide3 |

Version stamping: `SWLIBS/BINSRC` supplies flex/bison; `GENDATE.C`/`FXBLDNO.C` stamp build
dates/numbers into binaries.

## 2. Environment variables that drive the build system

Set by `Q3dEnv.cmd` / manually (nmake reads them in `SWLIBS/INCLUDE/NMAKE/3dfx.mak`):

| Variable | Values | Meaning |
|---|---|---|
| `BUILD_ROOT` | repo root | Everything is relative to this |
| `BUILD_ROOT_SWLIBS` | `%BUILD_ROOT%\swlibs` | Make-fragment + lib/header publish root |
| `FX_GLIDE_HW` | `H5` (this snapshot; `SST1/SST96/CVG/H3` trees absent) | Hardware generation subtree |
| `FX_HW_PROJECTS` | `glide3` (default), `glide` | Which API runtime(s) to build |
| `FX_TARGET` | `WIN32`, `DOS` | Target OS |
| `FX_COMPILER` | `MICROSOFT`, (`WATCOM`) | Compiler family |
| `FX_DLL_BUILD` | `1` | Build glide as DLL |
| `DEBUG` / `NTDEBUG` | set/empty | Debug vs retail (`GLIDE_DEBUG`, `GDBG_INFO_ON`) |
| Glide feature defines (set by makefiles) | `USE_PACKET_FIFO`, `GLIDE_HW_TRI_SETUP`, `GLIDE_PACKET3_TRI_SETUP`, `FX_GLIDE_NAPALM`, `FX_GLIDE_H5_CSIM`, `GL_AMD3D`, `GLIDE_PLATFORM` | See `H5/DOCS/3dfx Conditional Compile Flags.doc` and `makefile.linux` for the full matrix |
| STB perf switches (`*/BUILD/STBPERF.INC`) | `K6_2=1` (3DNow; comment out to disable — **equating to 0 does not work**), `STBKNI=1` (SSE), `NEWASMTRI=1`, `NEW_CCU=1` | Driver assembly-path selection |

## 3. Build walkthroughs

### 3.1 Glide3 for Win32 (the foundation — do this first)

```bat
cd <repo root>
call Q3dEnv.cmd            :: sets FX_GLIDE_HW=H5, FX_HW_PROJECTS=glide3, WIN32/MS, calls tools+setenv
nmake                      :: top MAKEFILE → swlibs → h5 → glide3
:: outputs: H5\GLIDE3\SRC\GLIDE3X.DLL (+ .LIB/.EXP), staged into H5\BIN
```
Order enforced by makefiles: `swlibs` (headers→`INCLUDE`, make fragments, libs) → `h5/incsrc`
→ `h5/minihwc` → `h5/glide3/src`. Prebuilt `.OBJ`/`.LIB` in the tree let partial builds link
even if you lack one sub-tool (e.g. the checked-in `xdraw*_3dnow.obj`).

### 3.2 Glide3 for Linux (recommended modern path)

```sh
export BUILD_ROOT=$PWD BUILD_ROOT_SWLIBS=$PWD/SWLIBS
export FX_GLIDE_HW=h5 FX_HW_PROJECTS=glide3
make -f makefile.linux     # → libglide3x.so; kernel access expects /dev/3dfx or DRI
```
Options (from `H5/GLIDE3/SRC/makefile.linux`): `DEBUG=1`, `FX_GLIDE_DIRECT_WRITE=1`
(register writes instead of packet FIFO — useful during bring-up), `FX_GLIDE_H5_CSIM=1`
(link against the simulator — **no hardware needed**), `FX_GLIDE_ALT_TAB=1`.
Caveat: written for gcc 2.9x/egcs — expect to patch K&R-isms, `varargs`, and asm constraint
syntax on modern gcc/clang; the C paths (`GLIDE_USE_C_TRISETUP`) build most cleanly.

### 3.3 Win9x driver set

```bat
call SETENV.BAT            :: MSVC6 + MSVC1.52 + MASM + 95/98 DDK + DX7 DDK paths
cd H5\Win9x\DX
:: per-component makefiles: DD16, DD32, D3D, MINIVDD (each has MAKEFILE + DEBUG/RETAIL dirs)
nmake                      :: or per-directory nmake; outputs into BIN\DEBUG / BIN\RETAIL
```
Products: `3dfx16*.drv` (DD16), `3dfx32v3/vb/vs.dll` (DD32+D3D), `3dfxv3/vb/vs.vxd` (MiniVDD).
Install via `INF/Voodoo3` or `INF/Voodoo5`. The `.COD` listings checked into `DEBUG/RETAIL`
are compiler-output artifacts from the original builds — useful as reference disassembly.

### 3.4 NT4 / W2K drivers

```bat
:: W2K:
call W2kEnv.bat            :: or: cd H5\W2K\Src\Video && SETENV.BAT — sets DDK build env
:: uses W2KDDK build.exe over Miniport\H5 and Displays\H5 (DIRS files present)
:: NT4: Nt4Allen.bat, H5\WinNT\Src\Video\setenv.bat — same pattern with NT4 DDK
```
Outputs: miniport `.sys` + display `.dll`; INFs under `Src/Video/Inf/Voodoo3|Voodoo5`.
Sign-free era: installs on W2K with driver-signing prompts only.

### 3.5 DOS Glide + SDK tests

```bat
call setdosenv.bat         :: Watcom paths, DOS4GW
call Q3dDosEnv.cmd         :: FX_TARGET=DOS variant
cd H5\GLIDE3 (or GLIDE) && nmake FX_TARGET=DOS
```

### 3.6 Diagnostics kit

```bat
call Q3dDiagsEnv.cmd
BuildDiags.bat build       :: swlibs → incsrc → copies headers/libs → builds DIAGS trees
:: BAT2SH.AWK/TESTSUM.AWK generate shell equivalents + summaries; MUSTPASS.BAT is the gate
```

### 3.7 BIOS

`H5/BIOS/SRC` + `BIOSENV.BAT`/`BLDVERS.BAT` (MASM real-mode). Flash with `FLASH/` tool from
DOS. **Danger**: wrong image bricks the board; keep a hot-flash recovery plan (second card or
ISA VGA to boot headless).

### 3.8 CSIM and CSIM-backed Glide (no hardware required)

`H5/CSIM/BUILD.BAT` / `DOIT.BAT` (MSVC) or `makefile.unix` builds the simulator library;
`WinSIM.mak` builds the Windows harness. Then build Glide3 with `FX_GLIDE_H5_CSIM=1` and run
any Glide test against the software chip — the intended pre-silicon workflow, and your safest
development loop.

## 4. Practical notes for reproducing builds today

- **Host OS**: a Win98SE or W2K VM (VirtualBox/86Box/PCem for real-hw testing) with the
  DEVTOOLS layout above reproduces everything Windows-side. MSVC6/MASM/DDKs are abandonware
  but widely archived. `subst`/`net use` a drive to keep paths short — the 2000-era tools
  have 260-char path limits and dislike spaces (**this repo's folder name contains a space;
  copy or junction it to e.g. `C:\3dfx` before building on Windows**).
- **Line endings**: sources are CRLF; keep them that way for MSVC1.52/MASM (they mis-parse
  bare LF in some cases). The `.unix`/`.linux` makefiles tolerate LF.
- **StarTeam/SourceSafe droppings**: `MSSCCPRJ.SCC`, `VSSVER.SCC`, `.PRF` files are inert;
  don't delete (some makefiles reference timestamps), just ignore.
- **Parallel/verify builds**: `H5/DIAGS/MUSTPASS.BAT` then Glide `CONFORM` then OpenGL
  `CONFORM`/`OGTST` is the original QA ladder — run in that order after any build.
- Case sensitivity: tree mixes `MAKEFILE`/`makefile.*`; on Linux use the provided lowercase
  names explicitly (`make -f makefile.linux`).
