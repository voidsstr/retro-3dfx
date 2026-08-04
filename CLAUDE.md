# retro-3dfx — Claude Code Instructions

**This repo is the VINTAGE-SOURCE 3dfx driver stack** — the leaked H5/Napalm
tree built for Windows XP under Wine. It is NOT the open-source stack (MesaFX
ICD / open Glide3 in `retro3dfx-gl` and `retro-agent/scripts/3dfx/`) that other
sessions work on. When in doubt which stack a file belongs to: everything under
`3dfx Driver Code/` and `toolchain-3dfx/prefix/drive_c/3dfx/` here is the
vintage stack.

## RULE: the Voodoo 5 box runs THIS repo's drivers ONLY

**Never build, deploy, copy, or otherwise use any driver binary from the
`retro-agent` tree (or any other repo) on the Voodoo 5 box (.143).** That
includes `retro-agent/voodoo-cleanroom/build/retro3dfx-gl` (a separate ~2.7 MB
Mesa ICD, versioned `0.1.x`) and the staging copies under
`C:\RETRO_AGENT\3dfx-driver\` on the box. They are a DIFFERENT driver lineage
and mixing them silently regresses the V5.

Every V5 driver binary must come from this repo:

| On the box | Canonical source in THIS repo |
|---|---|
| `system32\3dfxv5d.dll` | `.../Displays/H5/objfre/i386/3dfxvs.dll` |
| `system32\drivers\3dfxv5m.sys` | `.../Miniport/H5/objfre/i386/3dfxvsm.sys` |
| `system32\glide2x.dll` | `H5/GLIDE/SRC/glide2x.dll` |
| `system32\glide3x.dll` | `H5/GLIDE3/SRC/glide3x.dll` |
| `system32\opengl32.dll` AND `3dfxogl.dll` (same file) | `SWLIBS/OPENGL/GLIDE3X/release/opengl.dll` (ICD `retro3dfx 0.4.0`) |

Verify with hashes, never sizes/dates: **all ICD builds 0.3.7–0.4.0 are exactly
704,512 bytes**. Read the embedded `retro3dfx 0.x.y` string. **`fc /b` returns
EMPTY through the agent** (reports identical files as differing) — DOWNLOAD and
md5 instead. Audit method + last sweep: memory `driver-version-audit`.

Game dirs that ship their own `opengl32.dll`/`3dfxogl.dll`/`glide2x.dll`/
`glide3x.dll` SHADOW system32 — re-audit after any game reinstall.

## The driver stack — what ships to the box

| Deployed file (XP box) | Built from | What it is |
|---|---|---|
| `C:\WINDOWS\system32\3dfxv5d.dll` | `Displays/H5` → `objfre/i386/3dfxvs.dll` (renamed; WFP-safe name) | XPDM display driver: 2D/GDI + DirectDraw + **D3D6/7/8 HAL** (D3*/D6*/D7* files) + Glide escape (HWCEXT) |
| `C:\WINDOWS\system32\drivers\3dfxv5m.sys` | `Miniport/H5` → `objfre/i386/3dfxvsm.sys` (renamed) | video miniport: PCI/chip init, modes, **SLI/AA enable IOCTL (SLIAA.C)**, registry access for the display driver |
| `glide2x.dll` / `glide3x.dll` | `H5/GLIDE2` and `H5/GLIDE3` + `MINIHWC` | Glide runtimes (games talk straight to these) |
| `3dfxogl.dll` | (OTHER STACK — MesaFX ICD from `retro3dfx-gl`) | OpenGL ICD; registered via `OpenGLdrivers` registry key |
| `C:\RETRO_AGENT\d3dlab.exe` | `tests/d3dlab/` | our windowed D3D8 regression lab (not vintage) |

## Source layout (two trees — keep them in sync!)

- **Repo tree (source of truth, committed):**
  `3dfx Driver Code/H5/W2K/Src/Video/Displays/H5/` (display driver + D3D HAL)
  `3dfx Driver Code/H5/W2K/Src/Video/Miniport/H5/` (miniport)
  `3dfx Driver Code/H5/GLIDE3|GLIDE2|MINIHWC|SWLIBS/` (Glide + shared HW code)
  `3dfx Driver Code/H5/Win9x/` — the WORKING Win9x driver: the reference
  implementation when the W2K tree misbehaves (the mip-download fix was found
  by diffing `Win9x/DX/D3D/D3TXTR.C` rev 35 vs W2K rev 40).
- **Build tree (what Wine actually compiles, mostly gitignored):**
  `toolchain-3dfx/prefix/drive_c/3dfx/H5/...` — same layout. **Every repo-tree
  edit must be `cp`'d here before building.** `tests/test_source_invariants.sh`
  fails if a fixed file diverges between trees.
- Key display-driver files: `D3TXTR.C` (texture create/download — mip fix),
  `D6DP2.C` (DrawPrimitives2 dispatch), `D3CONTXT.C` (context create/destroy),
  `DDMEMMGR.C`/`HEAP5.C` (video memory heaps), `DDFLIP.C` (flip/present),
  `DDFXNT.C` (Enter/Exit_3DApplication, Promote/Demote SLIAA,
  Compute_SLIAA_Config), `CFIFO.C` (command FIFO + H3MakeRoom),
  `LOGFILE.C` (registry-ring flight recorder), `HWCEXT.C` (Glide escape).
- Miniport: `SLIAA.C` (multi-chip SLI/AA setup + v56k 6000 external clock),
  `H3.C`, `h3registry.c`.

## Build process (Wine-hosted VC6 + W2K DDK)

```bash
cd toolchain-3dfx
# 1. sync edited files repo tree -> prefix tree (cp; see above)
# 2. purge the .obj of every edited file (build -cZ does NOT reliably rebuild):
rm -f prefix/drive_c/3dfx/H5/W2K/Src/Video/Displays/H5/objfre/i386/<file>.obj
# 3. build (display driver shown; use the Miniport path for 3dfxv5m):
export WINEPREFIX=$PWD/prefix PATH=$PWD/wine/bin:$PATH COPYCMD=/Y
ulimit -f 2000000
timeout 560 wine cmd /c 'c:\3dfx\bldw2k.bat c:\3dfx\H5\W2K\Src\Video\Displays\H5'
pkill -9 -x wineserver
# output: prefix/.../Displays/H5/objfre/i386/3dfxvs.dll  (BUILDEXIT=0 = success)
```
Traps: `build -cZ` on the miniport deletes the *other* .sys name (stash);
POSTBLD rebase failure under Wine is cosmetic; grep vintage sources with `-a`
(CRLF/binary chars); `SOURCES` sets `ENABLE_LOG_FILE=1` (LF=1) for the
instrumented builds.

## Test process (REQUIRED order)

1. **Before deploy:** `bash tests/predeploy.sh` — source invariants + native
   pure-logic tests + built-artifact/stale-obj checks. Non-zero exit = DO NOT
   deploy.
2. **Deploy** (display driver, box .143): UPLOAD via retro-agent protocol →
   `move /Y` the loaded `3dfxv5d.dll` aside → `copy` new one in → reboot
   (`shutdown -r -t 5 -f`). Keep the move-aside `.bak`.
3. **After reboot:** `python3 tests/run_target_tests.py` (driver/res/ring +
   12-mode d3dlab D3D matrix vs goldens) AND the OpenGL golden gate
   (Q3/CS — `/tmp/post_instr_verify.py` pattern) — D3D and GL share the
   modeset/2D core, so both gates apply to display-driver changes.
4. **When a fix is verified on hardware:** add its regression test in the SAME
   commit — source assertion (`tests/test_source_invariants.sh`), native logic
   test (`tests/native/test_*.c`), d3dlab mode + golden, binary marker —
   whichever layers apply. A fix without a regression test is not done.
   Full change policy: `optimized/README.md`; suite docs: `tests/README.md`.

## Instrumentation & diagnostics (in the deployed driver)

- **Registry-ring flight recorder**: `RLog00..RLog31` + `RLogSeq` under
  `HKLM\SYSTEM\CCS\Services\3dfxvs\Device0` (REG_BINARY UTF-16LE; newest slot
  = (RLogSeq-1)&31; survives reboots). Reader: `/tmp/ring_read.py` pattern.
  Lines: lifecycle (DrvEnableSurface/AssertMode), `H3MakeRoom STALL/WEDGE-BREAK`,
  `DdFlip WEDGE-BREAK`, `DP2-PARSE-ERR`/`DP2-EXIT-ERR` (failing D3D op),
  `COMPUTE/PROMOTE/DEMOTE-SLIAA`, `memMgr ALLOC-FAIL`, `CTX-CREATE/DESTROY`
  (live-context balance).
- **Registry knobs** (display Device0): `SSTH3_SLI_AA_CONFIGURATION`
  (0=single-chip, 2=2-way SLI default on 5500 — Glide reads it too!),
  `Retro3dfxLog` (verbose gate), `CmdfifoSize`, `SSTH3_SLI_BAND_HEIGHT`.
- **Test box .143**: Athlon 1GHz, V5 5500 AGP, XP SP3, agent v1.15+
  (HIGH priority, `MONITOR` streaming). Hard freeze = NIC dead = physical
  power cycle (ask the user). Reboots need user awareness. GDI screenshots
  garble during exclusive fullscreen (windowed d3dlab is truthful).

## Current state / open bugs

**`VINTAGE-FIXES.md` is the definitive ledger of every fix to the vintage
codebase** (which bugs were 3dfx's, which are ours, commits, regression
tests). `FINDINGS.md` has the investigation matrices; memory file
`d3d-3dmark-state` the session state. Headlines — ALL FIXED: mip downloads
(`08fd889`), UT2004 TEXBLT FourCC arity (`7ddda02`), GoldSrc Direct3D crash
(`8de09a3`) + white world / mip-sublevel downloads (`cf3ab3e` — CS D3D now
renders and benches FASTER than GL: 33.5 vs 30.6 fps @1024), desktop gamma
washout (`07424b8`), all six unbounded accelerator spins bounded. SLI banding
and warm-rerun "degradation" were resolved as non-bugs (the mip bug / 3DMark
itself). V5 6000 (4-chip) prep: branch `v56k-6000`, plan in `V56K-PLAN.md`.
