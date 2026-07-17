# toolchain-3dfx — Wine-hosted VC6/DDK build environment for the 3dfx H5 driver source

Builds the 3dfx H5/Napalm driver source (`../3dfx Driver Code/`, archived
verbatim in this repo) into working Voodoo 3/4/5 binaries for Windows
2000/XP, entirely on Linux under Wine. Produced and verified 2026-07-16:

- `GLIDE3X.DLL` (Glide3 runtime, 96 exports — export list identical to the vintage Nov-2000 DLL)
- `3dfxvsm.sys` (W2K/XP video miniport) + `3dfxvs.dll` (XPDM display driver incl. D3D HAL)
- `FXOEM2X.DLL`, `glide3x.lib`, `minihwc.lib`, `h3cinit.lib`

## What is tracked vs. ignored

Tracked: `extract_ddk.py`, `package_driver.sh`, `dist/` (deployable driver
package + zip), this README. Everything else is gitignored because it is
large and reconstructable:

| dir | contents | how to reconstruct |
|---|---|---|
| `wine/` | Kron4ek portable Wine 11.13 (wow64) | github.com/Kron4ek/Wine-Builds releases |
| `downloads/` | archive.org fetches: `visual-studio-6-0-sp5-portable`, `vcpp5`, `vs6sp5`, `msdn-disc7-february-2000-x05-48786` (W2K DDK ISO), dx7ddk | archive.org, same item names |
| `devtools/` | extracted toolchain = `C:\3dfxtools`: `msvc6_0/` (VC6 SP5: CL 12.00.8804), `masm614/`, `w2kddk/` (rebuilt from ISO CABs via `extract_ddk.py`), `dx7ddk/`, `w9xddk/` (Win98 DDK headers from github.com/fapablazacl/win98-ddk-toolchain — needed by MINIHWC for minivdd.h/vmm.h/configmg.h) | re-extract from `downloads/` |
| `prefix/` | Wine prefix; `drive_c/3dfx` = space-free build copy of the source | recopy source + apply the two edits below |
| `bin/`, `extract/` | 7-zip + ISO extraction staging | trivial |

## Build-copy edits (the only source changes)

1. `H5/GLIDE3/MAKEFILE:25` — `SUBDIRS = oem src tests` → `oem src` (the
   `tests` dir is absent from the drop; `updown.bat`'s bare `exit` kills the
   parent cmd under Wine).
2. `drive_c/3dfx/bldw2k.bat` (CRLF) — W2K DDK build wrapper: sets
   `DEVTOOLS=c:\3dfxtools`, calls `H5\W2K\Src\Video\SETENV.BAT`, sets
   `COFFBASE_TXT_FILE`, `COD_FILES=0`, then `build -cZ` in the dir given as
   `%1`. Judge success by `build.err` + artifact existence — never the exit
   code (`BUILD_DEFAULT` includes `-i`).

## Critical Wine gotchas (learned the hard way)

- **`COPYCMD=/Y` must be exported** — the tree's `INSTALL` rule is `xcopy`
  without `/Y`; Wine's xcopy re-prints the overwrite prompt forever on EOF
  stdin (this once wrote a 38 GB log and filled the disk).
- **Never pipe wine output** (`wine … | tail` hangs — services.exe inherits
  the pipe FD). Redirect to a file, then tail the file. Always
  `ulimit -f 2000000; timeout 240 …` on every wine call.
- **Purge vintage build artifacts before building** — all checked-in
  `.OBJ/.LIB/.DLL` and generated headers share the copy timestamp, so nmake
  silently skips real compilation.
- Cleanup: `pkill -9 -x wineserver` (NOT `-f`, which kills your own shell),
  `pkill -9 -f 'winedevice\.exe'`, as a separate shell call.
- Glide3 env (exported from bash; Wine imports Linux env): `BUILD_ROOT=C:\3dfx`,
  `BUILD_ROOT_SWLIBS=C:\3dfx\swlibs`, `FX_GLIDE_HW=H5`, `FX_HW_PROJECTS=glide3`,
  `FX_TARGET=WIN32`, `FX_COMPILER=MICROSOFT`, `FX_DLL_BUILD=1`,
  `DIRECTXSDK=C:\3dfxtools\msvc6_0\vc98`, `W9XDDK=C:\3dfxtools\w9xddk`,
  `WINEPATH` = vc98\Bin;msdev98\Bin;masm614\bin, `INCLUDE`/`LIB` = vc98.
  Bypass the tree's `Q3dEnv.cmd`/`TOOLS.BAT`/`SETENV.BAT` wrappers (broken paths).
- Glide3 build order: per-directory `nmake` (swlibs incsrc/libsrc/fxmemmap/
  fxmisc/newpci\pcilib/fxremap/fxagp → h5 incsrc/cinit/minihwc → glide3 oem/src),
  not top-level recursion.

## Deployment

`package_driver.sh [version]` assembles `dist/3dfx-napalm-xp-<version>/`
(binaries + trimmed INFs + `updrv.exe` + `INSTALL.bat`). Deploy to a fleet XP
box with the `deploy-3dfx-driver` skill (`.claude/skills/deploy-3dfx-driver/`).
