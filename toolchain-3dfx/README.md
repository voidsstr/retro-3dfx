# toolchain-3dfx — Wine-hosted VC6/DDK build environment for the 3dfx H5 driver source

Builds the 3dfx H5/Napalm driver source (`../3dfx Driver Code/`, archived
verbatim in this repo) into working Voodoo 3/4/5 binaries for Windows
2000/XP, entirely on Linux under Wine. Produced and verified 2026-07-16:

- `GLIDE3X.DLL` (Glide3 runtime, 96 exports — export list identical to the vintage Nov-2000 DLL)
- `3dfxvsm.sys` (W2K/XP video miniport) + `3dfxvs.dll` (XPDM display driver incl. D3D HAL)
- `FXOEM2X.DLL`, `glide3x.lib`, `minihwc.lib`, `h3cinit.lib`

## One-shot reconstruction (no sudo required) — START HERE

The whole toolchain rebuilds on any Linux/WSL host with **no root**, driven by the
scripts under [`build/`](build/):

```bash
export RETRO3DFX_TC=$HOME/retro3dfx-toolchain      # heavy content lives HERE, not in the repo
bash   toolchain-3dfx/build/setup-toolchain.sh     # download + extract + assemble everything
source toolchain-3dfx/build/env.sh                 # canonical build env (all the gotchas baked in)
bash   toolchain-3dfx/build/build-glide3x.sh       # verify: builds a 96-export GLIDE3X.DLL
```

Then: `build-glide2x.sh` (glide2 runtime), `build-w2k.sh [miniport|display|both]`
(kernel driver pair). Rebuilt-vs-deployed parity, verified 2026-08-11 on a fresh
host: `glide3x.dll` 96 exports · `glide2x.dll` 258,048 B / 133 exports (== the
binary deployed on the V5-6000 box) · `3dfxvsm.sys` 199,656 B (== deployed).

- **Heavy content is on a roomy Linux volume, not `/mnt/c`.** WSL's `/mnt/c` is slow
  drvfs and often near-full; put `$RETRO3DFX_TC` on the Linux ext4 volume. The five
  gitignored dirs (`wine/ downloads/ devtools/ prefix/ extract/`) may be **symlinks**
  into `$RETRO3DFX_TC` (add them to `.git/info/exclude` so they don't show as
  untracked). `env.sh` reads `$RETRO3DFX_TC` (default `~/retro3dfx-toolchain`).
- **The build Wine prefix** is `$RETRO3DFX_TC/prefix`; `drive_c/3dfx` is a space-free
  copy of `../3dfx Driver Code/` with the repo's tracked source-fix deltas
  (`prefix/drive_c/3dfx/**` in git) overlaid on top. `setup-toolchain.sh` does the
  copy + overlay + the two build-copy edits automatically.

## Toolchain backup on the share (fast offline restore)

The exact reproducible inputs are archived on the fleet share so a rebuild needs no
internet and pins the versions:

```
\\192.168.1.122\files\Utility\Retro Automation\3dfx-build-toolchain\
   README.txt            manifest: versions, source URLs, usage
   downloads\            wine-11.13, vc6-sp5-portable.7z, 1_WIN2KDDK.iso, dx7ddk.exe,
                         vcpp5.exe, win98-ddk-toolchain.tar.gz, 7zz, SHA256SUMS.txt
   build\                copies of env.sh / setup-toolchain.sh / build-glide3x.sh
```

To restore from the share instead of the internet: copy `downloads/` into
`$RETRO3DFX_TC/downloads/`, `tar -xzf downloads/win98-ddk-toolchain.tar.gz -C
$RETRO3DFX_TC/extract/`, then run `setup-toolchain.sh` (it skips any download that is
already present). From Linux the Buffalo NAS needs SMB **signing disabled**
(`smbprotocol` `require_signing=False`) and writes ≤ 64 KB/chunk; creds `admin`/`password`.

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

All of these are already handled inside [`build/env.sh`](build/env.sh) and the
`wnmake` helper — this list is why they are there.

- **`cl.exe` needs `Common\MSDev98\Bin` on the Windows PATH**, not just `vc98\Bin`
  — that is where `mspdb60.dll` lives; without it `cl` dies with status 53
  (`import_dll mspdb60.dll not found`) before compiling anything.
- **`COMSPEC` must point at Wine's `cmd.exe`** (`c:\windows\system32\cmd.exe`) or
  nmake fails `U1056: cannot find command processor` the moment a rule shells out.
- **Put `c:\windows\system32;c:\windows` on `WINEPATH`** or the makefiles' `xcopy`
  install rules fail (`'xcopy' is not recognized`) — a bare toolchain PATH hides
  Wine's own utilities.
- **`LINK` needs `TEMP`/`TMP` set INSIDE the cmd session**, e.g.
  `wine cmd /c "set TEMP=c:\windows\temp&& set TMP=c:\windows\temp&& nmake"`. Wine
  does **not** import the Unix `TMP` into the Windows environment, and VC6's linker
  spills a long object list to a response file via `GetTempPath`; with `TEMP` unset
  it dies `LNK1104: cannot open file "TEMPFILE"`. Short links (few objects) mask this.
- **`COPYCMD=/Y` must be exported** — the tree's `INSTALL` rule is `xcopy` without
  `/Y`; Wine's xcopy re-prints the overwrite prompt forever on EOF stdin (this once
  wrote a 38 GB log and filled the disk).
- **Never pipe wine output** (`wine … | tail` hangs — services.exe inherits the pipe
  FD). Redirect to a file, then tail the file. Always `ulimit -f 2000000; timeout …`.
- **Do NOT blanket-delete `*.lib` to "force a clean build."** `SWLIBS/LIBSRC`
  *publishes* prebuilt libs it does not compile — deleting them gives
  `U1073: don't know how to make '*.lib'`. Instead rely on the overlaid delta `.c`
  files being newer than their checked-in `.obj` (they recompile; the rest links
  from the drop's objects). `build-glide3x.sh` does exactly this.
- **Extraction:** the VC6 SP5 `.7z` uses a **BCJ2** filter that `py7zr` cannot
  decode — use the bundled `7zz` (7-Zip 23.01 standalone) for it, the W2K DDK CABs,
  and the ISO alike. `extract_ddk.py` takes the path to `7zz` as its third argument.
- Cleanup: `pkill -9 -x wineserver` (NOT `-f`, which kills your own shell),
  `pkill -9 -f 'winedevice\.exe'`, as a separate shell call.
- The full env (`BUILD_ROOT`, `FX_GLIDE_HW=H5`, `FX_HW_PROJECTS`, `INCLUDE`/`LIB`,
  `WINEPATH`, …) is in `build/env.sh`. Bypass the tree's `Q3dEnv.cmd`/`TOOLS.BAT`/
  `SETENV.BAT` wrappers (broken paths).
- Glide3 build order: per-directory `nmake` (swlibs incsrc/libsrc/fxmemmap/
  fxmisc/newpci\pcilib/fxremap/fxagp → h5 incsrc/cinit/minihwc → glide3 oem/src),
  not top-level recursion. `glide2` reuses swlibs+minihwc but stages its OWN
  `glide.h/glidesys.h/glideutl.h` into `H5\include` (see `build-glide2x.sh`).
- **W2K DDK `build.exe`:** judge success by the artifact + `1 executable built`,
  never the exit code — `BUILD`'s default has `-i`, and the POSTBLD `rebase` step
  **always** fails cosmetically under Wine (`return code 0x63`); the `.sys`/`.dll`
  is already linked at that point.

## Deployment

`package_driver.sh [version]` assembles `dist/3dfx-napalm-xp-<version>/`
(binaries + trimmed INFs + `updrv.exe` + `INSTALL.bat`). Deploy to a fleet XP
box with the `deploy-3dfx-driver` skill (`.claude/skills/deploy-3dfx-driver/`).
