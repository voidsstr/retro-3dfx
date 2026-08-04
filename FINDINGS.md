# retro-3dfx — Running Findings Log

Living log of important, hard-won findings for the 3dfx Voodoo3 (.124) driver work.
Append new findings as they're uncovered; keep newest-first within each section.
Detailed narratives live in `D3D-DRIVER-PLAN.md`; this file is the quick index of
"things that cost us time and we must not forget."

Target box: **.124** = "ADMIN", XP SP3, Voodoo3 AGP
`PCI\VEN_121A&DEV_0005&SUBSYS_1037121A&REV_01`, active Windows on **D:**, C: is a
Win98 FAT volume. Agent 1.14.0. Autologs in as voidsstr/password.

---

## Glide2x on .124 SOLVED: Unreal Gold 3dfx renderer works — nGlide was the wedge, our glide2x was two known fixes away (2026-08-04)

The July "Glide2x-era games are a crash risk - SKIP" finding is RESOLVED.

- **The wedge was nGlide, not Glide.** GOG's Unreal Gold ships nGlide (a
  Glide->D3D wrapper, 1.3MB 2013 dll) as game-local `glide2x.dll`. On the XP
  Voodoo3 its repeated failing grSstOpen attempts hard-froze the chip (no ping,
  no watchdog — physical power cycle). system32 also had a stale 2003 94KB
  glide2x. Neither was ours.
- **Our clean-room glide2x needed exactly the glide3x XP bring-up fixes**
  (never ported): GETLINEARADDR prime before ALLOCCONTEXT + zero-base guards
  (fork 79ee51e), plus the dual-ABI `_grFoo@N` relink in build-stack.sh
  (Glide2 games are MSVC-linked). GPF was hwcInitRegisters reading dramInit1
  off base0=0.
- **Debug loop that cracked it in minutes:** standalone `tst2x.exe` exerciser
  (LoadLibrary + underscore GetProcAddress + SetUnhandledExceptionFilter dump)
  + DEBUG glide2x build + addr2line on the DWARF = exact faulting line.
  grSstWinOpen also needs a REAL HWND (hWnd=0 fails silently — DDraw FSEM).
- **NEVER `taskkill /f` a fullscreen Glide2 game.** Kill mid-FIFO-packet =
  chip parses garbage = bus-level hang beyond the display driver's bounded
  waits (second power cycle of the day). Exit via the game's quit path. (The
  same applies to any direct-FIFO renderer; GoldSrc/Q3 survived kills because
  their teardown runs — TerminateProcess of a mid-frame glide2 app does not.)
- Unreal Gold verified: fullscreen Glide **640x480x16 @100Hz, stable** (96s
  intro flyby, clean).
- **800x600 fullscreen Glide WEDGES the Voodoo3** (2026-08-04, twice): first
  attempt fell back to VGA with "Display Driver Stopped Responding" (TDR, agent
  survived); after a clean reboot the second attempt hard-wedged the box (agent
  died with Unreal running normally at 800x600x16 — no taskkill involved, so
  this is the resolution itself, not the kill rule above). 640x480 is the only
  verified-good Glide2 mode on .124 so far. NEXT STEP: sweep resolutions with
  the standalone `tst2x.exe` exerciser (self-exiting, safe) rather than the
  game, to find where WinOpen/heap setup breaks — suspect the tiled-heap /
  buffer-count math at non-640 modes in glide2x's hwc path.
  Carmageddon 2 (GOG) also carries a game-local nGlide glide2x — same swap
  applies when wanted.

---

## GoldSrc-D3D present path: Blt-present promoted to page flip (+7.5%), and the wedge that taught us tile parity (2026-08-03)

D3D-vs-GL deficit hunt on .124 (CS 1.6, Voodoo3, 1024x768x16@100Hz). GL 40.9-43.4
fps, D3D 31.9 — but **identical 40.4 at 640x480**, so the HAL triangle path is
fine; the deficit only exists where fillrate matters.

- **GoldSrc-D3D never calls DdFlip.** It presents with a full-screen SRCCOPY
  `DdBlt` from its flip-chain back buffer into the primary — ~1.5 MB copied
  every frame. Found via new present-path tracers (DdCreateSurface
  TILED/LINEAR + primary-dest blt rects + flip/blt/lock counters -> RLog ring).
- **Fix (commit 0666fdb): `retroFlipPresent`** — promote that blt to a real
  overlay page flip (leftOverlayBuf + swapbufferCMD, the live scanout in
  fullscreen 3D) and ping-pong the app's back-buffer surface with a
  driver-allocated B2. **31.9 -> 34.3 fps.**
- **HARD WEDGE #1:** v1 ping-ponged into the **GDI desktop buffer** — the chip
  hard-hung (network dead; XP watchdog bugcheck auto-restarted the box ~5 min).
  Cause: color/Z **tile parity** — render targets must come from the tiled
  color slots whose even/odd layout matches the Z heap. B2 must be allocated
  from **TILED_HEAP2** (the third-buffer slot). The BACKBUFFER heap search was
  heap0-only -> DDERR_OUTOFVIDEOMEMORY; extended to include TILED_HEAP2.
- **Registry `GETENV` reads through the miniport are unreliable on the deployed
  box** (confirms the V5DLog finding) — `SSTH3_SWAPINTERVAL`, gate flags, all
  silently unread. Ship features **default-on behind strict shape conditions**,
  not behind registry reads.
- **App ROP compare gotcha:** GoldSrc passes `dwROP=00CC0000h` — only the
  HIWORD is the ROP. Compare `HIWORD(dwROP)==0xCC`, never the full SRCCOPY
  constant.
- **First D3D run after any reboot benches ~12 fps** (post-boot background
  activity) — always discard a warmup run.
- **GDI SCREENSHOT and GoldSrc `snapshot` are both blind in D3D fullscreen**
  (overlay scanout / unimplemented) — visual verification needs eyes on the
  monitor; timedemo correctness + 1-present-per-frame measurement are the
  remote evidence.
- Remaining D3D-vs-GL gap (34.3 vs ~41) is **not** the present path — next
  candidate: DP2 scene cost profiling.
- Rollback on .124: `system32\3dfxv3d.dll.bak9` (pre-flip-promotion tracer
  build), `.bak10` (vsync'd promotion). `cstrike\bench.dem` was re-recorded
  (365-frame) after a leftover `listenserver.cfg` fired; that cfg is deleted.

---

## CS 1.6 refresh + GL cursor: Glide fullscreen bypasses EVERYTHING GDI (2026-08-03)

Session on .124 (Voodoo3, CS 1.6 "Bcs16 Romania" build, D:\Program Files\...):

- **60Hz in GL was OUR ICD's hardcode, not XP's refresh bug.** MesaFX
  `fxMesaCreateBestContext()` passed `GR_REFRESH_60Hz` unconditionally, and in
  fullscreen **Glide programs the video timing itself** — GoldSrc `-freq`, the
  `FX_GLIDE_REFRESH_RATE` env the old launch batches hopefully set, GDI mode
  sets: all bypassed. Fixed in retro3dfx-gl **0.1.34** (`fxBestRefresh()`: env
  override else monitor-max via EnumDisplaySettings, snap down to a GR_REFRESH_*
  enum, retry at 60 if the open fails). Verified via retrogl.log: `grSstWinOpen
  ref=6 (100Hz)`, open OK.
- **D3D mode DOES honor GoldSrc `-freq 100`** (verified 100Hz all session via
  DISPLAYCFG polling; without it D3D runs 60Hz). But **`Counter-Strike.exe` (the
  repack launcher) does NOT forward args to hl.exe** — the desktop shortcut must
  target `hl.exe -game cstrike -freq 100` directly.
- **Invisible GL menu cursor = Glide scanout never composites the GDI/HW cursor
  plane** (D3D mode shows it because GDI manages that primary). Fixed in
  **0.1.35**: `fxDrawCursorOverlay()` stamps an arrow into the back buffer via
  grLfbLock before each swap when `GetCursorInfo` says the cursor is showing.
- **GDI SCREENSHOT under fullscreen Glide is BLACK** — useless for verification.
  New ICD debug hook `FX_DUMP_FRONT=<path>` dumps the Voodoo front buffer (raw
  565) every 64th swap; that's how the cursor fix was verified remotely.
- **`C:\setmode.exe 1024 768 32` restores the desktop at 60Hz** (no refresh
  arg) — after any test run, restore with the agent's `DISPLAYCFG set 1024 768
  32 100` instead, or the desktop is left degraded.
- CS 1.6 GL on our ICD **works fullscreen** now (the 07-18 "GoldSrc not
  supported on our ICD" changelog note is obsolete — game-local opengl32.dll IS
  ours since 07-24 and runs fine, menu + de_dust verified).

---

## DOS lane: CHAT dead-gate + DOSGAME "never marked installed" + COMMAND.COM missing-file errorlevel trap (2026-08-03)

Three DOS-lane defects, all fixed in retro-agent (agent v1.24.0 + rebuilt DOSGAME.EXE):

- **CHAT.BAT gated on `C:\DOSGAME\NET\PKT.OK` — but nothing ever wrote that file**,
  so DOS chat could never start on first try even after a fully successful PLAY
  network bring-up. Network setup now lives in `NETUP.BAT` (auto-called by BOTH
  PLAY.BAT and CHAT.BAT), which writes PKT.OK on DHCP success and re-verifies a
  pre-existing marker with a fast DHCP probe (marker persists across reboots; mTCP
  fails instantly when no packet driver answers).
- **COMMAND.COM does NOT set errorlevel for a missing command.** PLAY.BAT ran 8
  packet-driver .COMs by absolute path with only NE2000.COM shipped: each missing
  one printed "Bad command or file name" AND `if errorlevel 1` then thought the
  absent driver *loaded*, writing a dead PACKETINT into MTCP.CFG. Every driver/tool
  invocation is now `if exist`-guarded (NETUP.BAT + dosgame.c generated RUN.BAT).
- **DOSGAME installed-detection stem mismatch:** `write_install()` names the target
  dir with spaces/dots→`_` (`1_TO_NIL`) but `mark_installed()` compared the raw
  stem (`"1 To Nil"`) — any title with a space in its first 8 chars never got the
  installed star. One shared `zip_stem()` now; plus an `INSTLD.LST` receipt written
  by the install batch so installer-run (kind 'I') games that install to their own
  directory are still tracked. RUN.BAT also aborts with a clear message when the
  zip fetch failed instead of "installing" an empty dir.
  Tests: `tests/python/test_dosgame_install_detect.py` (retro-agent repo).

## BSOD 0x8E in vintage 3dfxv3d.dll — ROOT-CAUSED + FIXED (both == already-fixed bugs; .124 ran a STALE binary) (2026-07-28)

Five XP small memory dumps off .124 (Voodoo3, Win on D:), all bugcheck
**0x1000008E KMODE_EXCEPTION_NOT_HANDLED**, exc **0xC0000005**, faulting module
**3dfxv3d.dll**. Two DETERMINISTIC crash EIPs. Dump-derived facts (parse the XP
`DUMP_HEADER32`: bugcheck @0x28, then exc/EIP/ctxptr/0; `_TRIAGE_DUMP` @0x1000
gives ContextOffset/DriverList — DllBase is in the driver record whose name
field points into the string pool):

- **3dfxv3d.dll DllBase = `0xBF012000`** (dxg.sys sits at 0xBF000000; SizeOfImage
  0xC8500). The DLL is linked `/ALIGN:0x40` (SectionAlignment==FileAlignment==0x40,
  VA==RawPtr) so **RVA == file offset** — objdump VMA = 0x10000 + RVA.
- **EIP 0xBF04E4AC → RVA 0x3C4AC → `DdBlt@4`+0x5EC** (3 dumps: 07-25 ×2, 07-27;
  trigger = D3D-fullscreen: CS `-d3d`, UT D3DDrv). Faulting insn
  `mov 0x510(%edx),%eax`; **EDX=0 in all 3 dumps**. `edx=[esp+0x1c]`, set once in
  the prologue from `*(ppdev+0x784)->[0xdc]` = `_D3(lastContext)` = `pRc`.
  Root cause: the **system→video texture-download** path resolved both surface
  handles with `TXTRHNDL_PTR(h)` = `pRc->pHndlList->ppTxtrHndlList[h]`, but
  `pRc` (`lastContext`) is **NULL** in the context-less upload window (set only
  in the DP2 draw path, and `textureLoad()` itself zeroes it) → `pRc->pHndlList`
  = **[NULL+0x510]** → AV.
- **EIP 0xBF012346 → RVA 0x346 → `DrvBitBlt@44`+0x48** (1 dump: 07-28; trigger =
  Descent 3, DirectDraw fullscreen surface → a source-less fill). Faulting insn
  `mov (%edi),%eax`; **EDI=0** (=`psoSrc`, arg2). Root cause: the
  `#if ENABLE_LOG_FILE` debug block dereferenced `psoSrc->dhsurf` with **no NULL
  check**; `psoSrc` is legitimately NULL for solid/pattern blts. (This is the
  same offset as the .143 `3dfxv5d+0x346` Q3 vid_restart crash.)

**Both are ALREADY FIXED + committed + regression-tested in the H5 source** — the
.124 box was simply running a **stale, pre-fix 3dfxvs.dll** (deployed
`3dfxv3d.dll` 957456 B, no fix; fresh build 962724 B, fixed):
- DdBlt fix — `DDBLT32.C` resolves the TXTRHNDLs by walking the GLOBAL
  `g_pHndlList` chain (context-independent), never `pRc->pHndlList` (commits
  8de09a3, cf3ab3e). Only compiles under `DIRECT3D_VERSION>=0x0700 && DX>=7`;
  `PRECOMP.H` sets 0x0800 and `SOURCES` DX=7 → it does.
- DrvBitBlt fix — `BITBLT.C` null-guards BOTH psoDst and psoSrc in the
  ENABLE_LOG_FILE block, mirroring the SLI_AA block (commit a03a9fe).

**Verified against the fresh build (objdump):** DdBlt now loads `g_pHndlList`
(0xcdabc) and walks `ppTxtrHndlList[0]` count-compare (source lines 602-619) with
**zero** naked `mov 0x510` derefs; DrvBitBlt at +0x4A does `test %edi,%edi; je`
before `mov (%edi),%eax`. The OLD/crashing binary has NEITHER. So a rebuild +
redeploy of 3dfxv3d.dll fixes .124 (deploy left to the operator).

**Clean-room parallel:** fxd3ddd `Dd_Blt` (retro-agent `scripts/3dfx/driver/nt/
enable.c:1076,1109,1132`) already guards this class — null-checks `dst`/`dst->lpGbl`
and requires `src && src->lpGbl` before any source deref, returning NOTHANDLED on
a fill with no source (the M4c-2 hardening).

**Regression (new):** `tests/codegen_8e_guards.py` disassembles the linked DLL and
asserts BOTH fixes are in the CODEGEN (DdBlt refs g_pHndlList; DrvBitBlt tests edi
before deref) — catches a stale-obj link OR a DX<7/LF=0 preprocessor regression
that leaves source "fixed" but the binary crashing (exactly the .124 mode). Wired
into `test_built_artifact.sh` (+ bitblt/ddblt32 stale-obj checks). Validated:
PASS on the fresh build, FAIL on the crashing binary. Source-invariant asserts
for both already existed (`test_source_invariants.sh` #5d, #5f).

**Gotcha:** an XP "small memory dump" is NOT a user-mode minidump — DllBase lives
in the `_TRIAGE_DUMP` driver list (76-byte records, name field = a FILE-OFFSET
into the string pool, not a VA). And with `/ALIGN:0x40` the whole image maps 1:1
(RVA==file offset), so you can objdump the on-disk DLL directly at the RVA.

## DOS Game Manager built + verified in DOSBox-X (2026-07-28)

New `retro-agent/scripts/dosgames/`: DOSGAME.EXE (Open Watcom 16-bit TUI) +
host-side catalog/HTTP-bridge/tile tooling. Verified end-to-end headlessly
(DOSBox-X mingw under Wine on private Xvfb :77 — dosbox-staging Linux builds
hard-require GLX and abort on Xvfb). Full LAN install proven in emulation:
NE2000+slirp → Crynwr packet driver → mTCP DHCP → HTGET zip from
serve_dosgames.py (port 8181, systemd user service) → UNZIP → playable.
Deployed to share `…\Retro Automation\dosgame\`; install queued for .243.
Hard-won: (1) Watcom large-model **>64K static array silently wraps the data
segment** — no warning, corrupt entries past ~#420; (2) share zips have long
filenames → DOS 8.3 mangling breaks drive-letter copies, HTTP fetch is the
only reliable path; (3) DOSBox-X AUTOTYPE delivers enter/tab/chars but NOT
esc/F-keys — automate tests by timeout + file assertions. Survey of all 3,795
share DOS archives: install.exe/setup.exe/install.bat at zip root covers 96%
of installer archives; 2,893 zips are flat-root ready-to-extract.

## Share deploy trap: exe launched FROM the share locks its own update (2026-07-28)

Publishing retro_chat v0.14.0, the latest pointer `…\Retro Automation\retro_chat.exe`
was un-overwritable/un-renamable/un-READABLE (Access denied even as `admin`),
and gvfs showed the dentry as `??????????` / EINVAL. Root cause: **someone had
launched `retro_chat.exe` directly from `Z:\` on .145** — the running image
holds the share file open, and an earlier delete had put it into Samba
delete-pending, which blocks ALL new opens of that name until the last handle
closes. Diagnosis path that worked: per-box `net use Z: /delete` — the box that
refuses with "device is being accessed by an active process" is the holder; then
`wmic process … get ExecutablePath` found `Z:\…\retro_chat.exe`, PROCKILL freed
it (the pending delete then completed and the name freed up). Lessons:
- never run fleet exes from the share; copy local first (autoupdate does this).
- curl smb:// with the keyring creds (admin) is the reliable publish path when
  gvfs misbehaves; verify by md5 round-trip download.
- chat auto-update compares SIZE only (not .ver), so a temporarily mismatched
  `.ver` sidecar can't loop the fleet.

Also this session: retro_chat **v0.14.0** — Pentium-1 CPU fix (spinner was a
full erase+redraw every 150ms; now one WriteConsoleOutputCharacter cell per
500ms at below-normal priority) + startup now WAITS for the agent ("Waiting for
the retro agent to start...") instead of exiting when the chat wins the boot
race. Guard tests: `retro-agent/tests/python/test_retro_chat_p1_behavior.py`.

## fxD3D gbkernel: M4d bring-up-ladder tool built — fxdbg.exe (2026-07-27)

The clean-room fxD3D kernel Glide backend (retro-agent `scripts/3dfx/driver/nt/`,
gbkernel + escape ladder) had the kernel-side `FXDBG_*` DrvEscape handler
(`gbkdebug.c/.h`) but NO user-mode driver for it — M4d couldn't be run even with
a driver on-card. **Built `driver/nt/fxdbg/fxdbg.c`**: `CreateDC("DISPLAY")` +
`ExtEscape` over the shared `gbkdebug.h` ABI, one subcommand per rung
(`support|probe|clear|tri|tex|readback|ladder`), `readback` writes a 24bpp BMP
from the 16bpp-565 rect. Compile-verified (mingw, PE32). Added `selftest.c`
built `-m32` so `unsigned long` is 4 bytes (the i386 driver ABI) — pins opcodes,
magic, and every wire-struct size/offset so a `gbkdebug.h` field edit fails the
build, not the box. Wired into `make -C scripts/3dfx test` (all green).

- **Gotcha:** the host default `unsigned long` is 64-bit (LP64) — an ABI guard
  for an ILP32 driver struct MUST build `-m32`, or every `sizeof` is doubled.
- **M4d is now gated only on hardware:** deploying `fxd3ddd.dll` to .124 is a
  physical-recovery risk (experimental kernel display driver) — needs explicit
  operator go-ahead before flashing + running `fxdbg ladder`.

## CS menu cursor invisible in fullscreen GL — SOLVED: vgui_emulatemouse (2026-07-25)

**GoldSrc's menu cursor is the Windows OS cursor by default**, and in exclusive
fullscreen 3dfx GL the OS cursor overlay is never composited over the 3D scanout
→ menu renders, cursor invisible. **Fix: `vgui_emulatemouse "1"`** (in
`cstrike\userconfig.cfg` — WON auto-execs it and, unlike config.cfg, doesn't
overwrite it) makes the engine draw its OWN software cursor into the GL frame.
Verified at **1024×768×16 OpenGL** on our retrogl: glReadPixels `snapshot`
capture shows the white arrow in-frame (framebuffer captures never contain the
OS cursor, so an arrow in the capture IS the software cursor — clean proof).

- **Self-capture trick for BCShield builds** (UIKEY + GDI both useless in
  fullscreen Glide): a `wait`-chain + `snapshot` in **userconfig.cfg** fires a
  few hundred frames after startup → `cstrike\Snapshot0000.bmp` of the live menu.
  (My earlier attempt via `cstrike\autoexec.cfg` did NOT run — this build execs
  userconfig.cfg, not autoexec.cfg.)
- CS at 1024×768: registry ScreenWidth/Height + `-w 1024 -h 768`; retrogl.log
  confirms `res enum 12 (1024x768)` + context SUCCESS.
- **CS `-d3d` at 1024×768 WORKS — but ONLY with a settled 16bpp desktop.**
  On a 32bpp desktop it fails clean ("video mode not supported → software
  mode") — Voodoo3 D3D is 16-bit-only. Sequencing matters: setmode to
  1024×768×16 must complete BEFORE launching hl (`setmode` then launch in the
  same batch works; a racy setmode+launch from separate connections produced a
  false sw.dll revert). Verified: EngineDLL stayed `hw.dll` through a full
  bounded run + qconsole shows the engine past video init into sound init.
- **-d3d fullscreen WEDGES .124's network for minutes while running** (vintage
  HAL trait; recovers when hl.exe dies). Fine for local play; hostile to remote
  automation. ALWAYS use a fully self-contained on-box batch for -d3d tests
  (16bpp setmode → launch → ping-wait → taskkill → setmode 32bpp back), and
  queue a `taskkill` via the daemon task queue as the recovery net. Note the
  final setmode-32 needs a few seconds' wait after the kill or it doesn't take.

---

## MOHAA CD-lock SOLVED + RA2 renders on Voodoo3 (2026-07-24)

**MOHAA now launches + renders** on our OpenGL stack. Three stacked fixes:
1. **CD copy-protection (SafeDisc):** mount the owned disc image via DAEMON Tools
   — but `.124` is **dual-boot with Windows on D:**, so ONLY
   `D:\Program Files\D-Tools\daemon.exe` is the registered DT; the **C:\ copy
   throws "Product not installed!"** on every `-mount` (this wasted a long
   detour). DT 3.47 has **no CLI mount syntax difference** issue — it was purely
   the wrong (C:) binary. Mount: `start "" /d "<D:\...\D-Tools>" daemon.exe
   -mount 0,<image>` → G: shows `MOHAA_DISK1`; the plain mount satisfies MOHAA's
   SafeDisc check (no separate emulation toggle needed). Disc staged `D:\d1.iso`
   (from `Z:\Games\Windows XP\...Disc 1.iso`).
2. **"0 files in pk3 files":** the launch working-dir must be the real path —
   the 8.3 short name for `EA GAMES` is **`EAGAME~1`**, NOT `EAGAMES`; a wrong
   short path set fs_basepath to `D:\` → 0 pk3s → "couldn't load default.cfg".
   Launch with the full quoted path: `start "" /d "D:\Program Files\EA GAMES\
   MOHAA" MOHAA.exe` (NO `+set` cvars — the Ritual build crashes on cmdline
   r_mode/logfile/gldriver).
3. **GL:** deploy our retrogl as game-local **`opengl32.dll`** (same KnownDLL
   trick as CS — MOHAA's engine imports opengl32). Verified: retrogl.log shows
   `grSstWinOpen: returned <nonzero>` + `wglCreateContext: SUCCESS` for MOHAA.exe,
   fullscreen 640×480×16, proc → 32MB (game data loaded).

**Red Alert 2 runs + renders on the Voodoo3** (`C:\Games\...Red Alert 2...V2\RA2`):
the "Win10 Fixed" repack bundles the **aqrit ddraw wrapper** (`ForceDirectDrawEmulation=1`,
`SingleProcAffinity=1` — good for the single-core P3). Verified end-to-end:
main menu → Single Player → Skirmish setup → mission load → **live in-game
render** (iso terrain, Soviet units, fog, sidebar) at 800×600. No crash — the
known SSE2-`wsock32.dll` P1/P3 hazard (local 22528-B copy) did not trigger on
this repack. RA2 is 2D DirectDraw, not 3D — it uses the Voodoo3's 2D path.

---

## CS/GoldSrc OpenGL SOLVED — game-local opengl32.dll (2026-07-24)

**CS 1.6 (GoldSrc 1.1.2.5) now runs OpenGL on our retrogl.** GoldSrc does NOT use
the gldrv/3dfxgl.dll MiniGL LoadLibrary path (proven: our DllMain never fired for
`3dfxgl.dll` under either the Steam-emu 1.1.2.5 OR a WON 2001 build). Its `hw.dll`
uses the **statically-imported `opengl32.dll`**. The unlock: **`opengl32` is NOT
in .124's KnownDLLs list** (`HKLM\SYSTEM\CCS\Control\Session Manager\KnownDLLs`
has no `opengl32` value), so a **game-local `opengl32.dll` DOES load** (game dir
before system32). Deploy our retrogl AS `<CS>\opengl32.dll` (back up the original
first) → GoldSrc loads OUR opengl32 → `grSstWinOpen` succeeds →
`wglCreateContext: SUCCESS` for `hl.exe`, and **EngineDLL stays `hw.dll`** (GoldSrc
reverts it to `sw.dll` on ANY GL failure — non-revert is the GoldSrc-side success
signal). Config: `EngineDLL=hw.dll`, `EngineGLDriver=opengl32.dll`, `EngineType=2`,
Screen 640×480×16, launch `hl.exe -game cstrike -gl`. Needs the **787KB glide** in
the CS dir too (the 920KB build hangs — see below).

- This is why "CS doesn't work in OpenGL" persisted: the game-local `opengl32.dll`
  was MS's/software (or a stale build), and instrumenting via `wglGetProcAddress`
  was a red herring — GoldSrc resolves via kernel32 GetProcAddress, invisible to a
  per-call tracer. The **DllMain PROCESS_ATTACH** line (retrogl 0.1.33) is what
  finally distinguished "never loaded" from "loaded but failed".
- Q3/Q2/RtCW load our retrogl by an explicit **non-opengl32 name** (`retrogl.dll`,
  `3dfxgl.dll`, `gl/openglv5.dll`) via their own `r_glDriver`/`gl_driver` cvar;
  GoldSrc has no such cvar so it needs the `opengl32.dll` name. Two different
  load mechanisms, same retrogl DLL.
- **MOHAA** renders on our stack too (qconsole.log: our Mesa/3dfx extensions,
  `GL_MAX_TEXTURE_SIZE 256`, dual-TMU, `MODE 6 1024x768`) but the current disc is
  **CD-locked** ("Cannot locate the CD-ROM") — a no-CD/provisioning gate, not a
  driver problem. WON Half-Life is likewise gated on a **CD-Key** dialog.

---

## Games-OpenGL-broken ROOT CAUSE: wrong glide3x build (2026-07-24) — SOLVED

**REGRESSION ROOT-CAUSED (2026-07-25): the 920,157-byte DLL is NOT a source
regression and NOT a debug build — it is the `FX_GLIDE_HW=h5` (Voodoo4/5
Napalm) glide3x.** `voodoo-cleanroom/build-stack.sh` builds h5 FIRST and names
it **`out/glide3x.dll`** (the deploy-expected name); the Voodoo3 build is
`out/glide3x_h3.dll` (Makefile.mingw's default HW is also h5). Both were built
Jul 23 10:12 (h5 :22, h3 :27 — PE timestamps). Evidence: 920 KB build contains
Napalm-only strings (`FX_GLIDE_2PPC`, `FX_GLIDE_AA2/4/8_OFFSET_*`, Voodoo4/5500/
6000 board table, `Services\3dfxvs`); .text 0x43084 vs h3's 0x2b1a4; identical
393-export surface (why it drop-in loaded). It hangs/crashes in grGlideInit
because **all four verified bring-up fixes were committed to the h3 tree only**
(a71eb3f TLS `%fs:`→TlsGetValue + lost-context fallback, 8b6eb5f GETLINEARADDR
prime, 2387787 zero-base guard, a73a159) — h5's `fxglide.h` still has the raw
`%fs:` TLS read and h5's minihwc still ALLOCCONTEXTs unmapped → NULL base. The
glide clone (`glide-devel-sezero` @ a71eb3f) is clean; a from-scratch h3 rebuild
via the build-stack.sh recipe reproduces the good DLL **byte-identically except
8 link-timestamp/checksum bytes** (787,186 B, same 393 exports incl. dual-ABI
`_gr*` aliases) → `out/glide3x_rebuilt_fixed.dll`. Rule: **deploy glide3x_h3 to
Voodoo3 boxes; never ship the h5-named `out/glide3x.dll` artifact to .124**
(port the h3 fixes to the h5 tree before any Voodoo5 use of our glide).

**Every "OpenGL doesn't work" symptom on .124 traced to the WRONG glide3x, not
the retrogl ICD.** `voodoo-cleanroom/out/glide3x.dll` had been rebuilt to a
**920,157-byte** build whose **`grGlideInit()` hangs/crashes on the Voodoo3**.
The retrogl (MesaFX ICD) calls `grGlideInit` inside `fxQueryHardware()` on the
FIRST `wglDescribePixelFormat` (via `pfd_tablen`→`fxMesaSelectCurrentBoard`), so
the game dies at GL-init with the ICD's attach-banner logged and nothing after —
looked exactly like an ICD bug. It is NOT: with the **787,186-byte** production
glide the SAME retrogl runs clean through `grSstWinOpen` → `wglCreateContext:
SUCCESS`. **Q3 CONFIRMED: `GL_RENDERER: Mesa Glide v0.62 Voodoo3 (tm)
[voodoo-cleanroom 0.1.32]`, map+cgame rendered.**

- **Ship the 787,186-byte glide.** `out/glide3x.dll` is now that build (md5
  f42b0b49710bb5ef8b052abce5d4fb6e); the 920 KB one is quarantined as
  `out/glide3x_920157_debug_HANGS_grglideinit.dll`. There is a **regression in
  our clean-room glide between the 787 KB and 920 KB builds** (grGlideInit) —
  investigate separately; do NOT deploy the 920 KB build to any box.
- **Diagnostic path that cracked it:** instrument the retrogl (`fxrlog.h` →
  `C:\retrogl.log`), run a Q3-engine game (standard WGL path logs every step),
  read where it stops. GoldSrc's engine bypasses the ICD's WGL entry points so
  it logs nothing — use a Q3-engine game to exercise/verify the ICD.
- **GDI SCREENSHOT of Voodoo3 glide-fullscreen = garbage** (reads desktop FB, not
  the 3D overlay). Prove rendering with the in-game `GL_RENDERER` / a timedemo,
  never a screenshot.
- **Every game dir needs the 787 KB glide** (glide3x is NOT a KnownDLL, so the
  game-local copy wins) — fleet had mixed builds scattered everywhere.

---

## fxD3D M4d bring-up attempt #1 — deployed, NOT yet activating (2026-07-24)

`fxd3ddd.dll` (33 KB native PE, exports DrvEnableDriver) deployed to .124 but did
NOT become the active display driver over two reboots. Box stayed HEALTHY (no
BSOD) + fully recovered — the chassis review held. Hard-won mechanism notes:

- **Active display-DLL key = `HKLM\SYSTEM\CurrentControlSet\Control\Video\
  {B9F859EE-ACEA-46E1-B07C-6334E6BF65AB}\0000\InstalledDisplayDrivers`**
  (REG_MULTI_SZ) — the ACTIVE video device. NOT `Services\3dfxvs\Device0`
  (separate copy; \Device\Video0→it but editing it alone did nothing). Set BOTH.
  Swap+recover both ways: `reg add "<key>" /v InstalledDisplayDrivers
  /t REG_MULTI_SZ /d "fxd3ddd|3dfxv3d" /f` (no reboot to set; reboot to apply).
- **Two reboots keys=fxd3ddd → `QUERYESCSUPPORT`=0** (ExtEscape on DISPLAY DC, via
  `D:\fxdbg.exe probe` → `C:\fxdbg.txt`): fxd3ddd's DrvEscape not reached ⇒ fxd3ddd
  NOT active. DrvEscape IS in the DRVFN table (chassis.c:595). Box healthy 1024×768×32.
- **Rename-based file-lock test is USELESS** — Windows allows renaming an in-use
  file (only DELETE is blocked). Could not confirm load-vs-init-fail this way.
- **Recovery PROVEN**: restore both keys→`3dfxv3d` + REBOOT → vintage back,
  accelerated, agent alive. .124 survived 3 reboots clean. Autologon intact
  (voidsstr/password) so the agent always returns even on a black screen.
- **NEXT to unblock:** (1) add init-progress instrumentation to fxd3ddd
  (registry-marker writes at DrvEnableDriver entry + each PDEV/EnableSurface step
  — the H5 "RLog" pattern) so a deploy shows IF it loads + WHERE it fails; (2)
  check `iDriverVersion` in DrvEnableDriver (NT4 0x20000 vs an NT5 value XP SP3 may
  demand); (3) likely need a real SetupAPI/PnP install (deploy-3dfx-driver skill /
  updrv.exe) — the vintage 3dfxv3d was probably PnP-installed, not a bare reg-swap.
  Staged on .124: `D:\WINDOWS\system32\fxd3ddd.dll`, `D:\fxdbg.exe`, `C:\fxdbg.txt`.

## fxD3D M4c-1: backend wired to the card + on-card bring-up ladder (2026-07-24)

`fxd3ddd.dll` now brings the Voodoo3 up from the chassis and ships the
escape-driven ladder that will validate the kernel backend on-card at M4d
(before D3D drives it). All 15 compiles `EXIT=0`, `LINKEXIT=0`
(`scratchpad/build_fxd3d.sh`); `make -C scripts/3dfx test` all-PASS. Hard-won
points:
- **BAR0 comes from `IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES`, VRAM from the FB
  map.** `chassis.c DrvEnableSurface` → `fxchassis_attach_backend`: pick the
  first memory-space public range (`MappedInIoSpace==0`, non-null VA) as BAR0;
  `vramBytes = ppdev->vmi.VideoRamLength` (already filled by
  IOCTL_VIDEO_MAP_VIDEO_MEMORY — no separate VRAM query needed). Attach is
  **non-fatal**: on failure log + keep a 2D-only surface, never fail the enable.
  `DrvDisableSurface` detaches then `IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES`.
- **BAR1 non-cached is a MINIPORT CONTRACT, not something the display DLL can
  set.** `IOCTL_VIDEO_MAP_VIDEO_MEMORY` exposes no cache attribute; the chassis
  documents + relies on the paired `3dfxvsm.sys` mapping BAR1 non-cached
  (gbk_mmio.h hard requirement — GBK_WMB is a no-op fence valid only there).
  Confirm on-card at M4d via `FXDBG_PROBE`+`FXDBG_CLEAR`; if write-combining,
  grow `GBK_WMB` an `sfence`.
- **Desktop stride ≠ 3D stride.** `gbkernel_attach` gained a `desktopStride`
  param; `gb_swap`'s blit-present **dst** uses `lDeltaScreen` (desktop pitch),
  not the 16bpp color-buffer stride (M4b-2 minor #2). 32bpp dst-pixfmt convert
  is `TODO(fxd3d M4c-2)`.
- **DrvEscape bring-up ladder (`gbkdebug.c`, opcodes `0x3DF0..`), NONE needs
  D3D:** PROBE (status+cmdFifo regs+layout, no draw) → CLEAR (FASTFILL+swap) →
  TRI (gouraud PKT3) → TEX (PKT5 checker + textured quad) → READBACK (copy a
  BAR1 rect back so the agent verifies pixels without a GDI screenshot). Wired
  `INDEX_DrvEscape`; driven from user mode by `ExtEscape` on the display DC.
- **FP bracket at the DDI/ladder entry (M4b-2 minor #9):** `d3d_DrawPrimitives2`
  wraps `fxd_dp2_execute_real_cb` in one `EngSave/RestoreFloatingPointState`
  region; TRI/TEX rungs bracket their `(float)` casts + textured ST in
  `gbk_fpu_enter/leave`. `gb_tex_bind` is FPU-free (raw-bit ST scales) so it's
  covered without a bracket; the outer + gbkernel's internal per-batch brackets
  nest cleanly.
- **`gbstub.c` (no-hw fallback) had to gain `gbkernel_attach/detach` +
  `gbkernel_dbg_*` stubs** now that chassis.c/gbkdebug.c reference them, else the
  documented no-hardware smoke link breaks. Stubbed attach returns -1 (→ chassis
  degrades to 2D-only, same as a real attach failure). C89-clean under mingw.

## fxD3D M4b-2: kernel MMIO transport landed — full driver LINKS with the REAL backend (2026-07-24)

`gbstub.c` retired from the default build; `fxd3ddd.dll` now links
`driver/nt/gbkernel.c` + the four verified `gbk/gbk_*.c` (all compiles EXIT=0,
LINKEXIT=0 via `scratchpad/build_fxd3d.sh`; `make -C scripts/3dfx test` still
all-PASS). New: `gbk/gbk_mmio.h` (`GBK_WR32`/`GBK_RD32` volatile + `GBK_WMB`
fence hook), `gbkernel.c` (transport + full `gb_*`), `gbkernel.h`
(`gbkernel_attach`/`detach` for M4c). Hard-won points:
- **IO/CMDAGP registers are DIRECT PIO, never FIFO-routed.** The 10-store
  CMDFIFO init and `miscInit0` Y-origin go straight to BAR0; ONLY 2D/3D
  register + memory writes ride the ring. Routing the FIFO-arm through the
  FIFO would deadlock (it isn't live yet).
- **Every FIFO/status poll MUST be bounded, and a too-big single packet must
  fail — not just STALL.** `gbk_fifo_make_room` faults if `nBytes` exceeds one
  usable lap (else NEED_WRAP recurs forever without ever hitting the stall
  cap — it doesn't increment the stall counter). Cap-hit → `gbk_fault()`
  disables the FIFO (`baseSize=0`) and latches `faulted` so no further MMIO
  is issued; a wedged card can't hang win32k (design risk #1).
- **Untextured draw path is FPU-free.** Vertex colors come from an
  integer-built IEEE-754 bit table (`gbk_ub_to_f32bits`), x/y/ooz/oow are raw
  dword copies (`*(h3u32*)&f`). The lone float arithmetic is textured
  perspective-ST (`s*texW*oow`), bracketed per design risk #4.
- **VC6 warns C4146 at h3hw.h:358/379** — that's the *intended*
  `H3HW_MASK_IS_UNSIGNED` compile-assert (`-(unsigned) stays >0`), pre-existing
  and harmless; the new files are warning-clean.
- **Twice-stable readptr:** `gbk_hw_readptr` samples `readPtrL`, reads `status`
  between the two samples (as `_grHwFifoPtr` fifo.c:1082-1086), and returns the
  RAW register value — exactly what `gbk_fifo_update_read` subtracts
  `fifoOffset` from. Don't pre-normalize it.

## fxD3D M4b-1: gbkernel pure-logic core implemented — glide-source corrections vs the design doc (2026-07-24)

Implemented + exact-value host tests all green (`make -C
retro-agent/scripts/3dfx/driver/nt/gbkernel-test` → 4/4 PASS; mutation-probed:
dropping the FIFO −4 margin or the color-buffer parity adjust fails the suite).
Three places the open glide source differs from a naive reading of the design
doc — trust these, they'd have been hardware hangs:
- **`hwcInitFifo` is 10 ordered stores, not "9 writes"** (minihwc.c:1634-1663):
  baseSize=0 (disable) FIRST, then baseAddrL(=start>>12), readPtrL, readPtrH,
  aMin, aMax (both start−4), depth, holeCount, cmdFifoThresh, and baseSize-arm
  LAST (`((len>>12)-1)|SST_EN_CMDFIFO`, holes ON). The design's "9" counts
  aMin=aMax as one item.
- **Linear `calcBufferSize` is NOT page-rounded** (minihwc.c:3725-3745:
  `bufSize = (xres<<1)*yres` exactly). Buffers get bit-12 page-PARITY
  adjustment during the top-down carve (color even / aux odd, :1449-1492) —
  start addresses are NOT otherwise 4K-aligned (e.g. 800x600 col0 =
  0xD3FE00-style values are normal and hardware-correct).
- **`kSetupCullPositive == 0x00`** (fxcmd.h:544-551): CW/positive cull is
  `kSetupCullEnable` (0x02) alone; CCW/negative adds 0x04; cull-off is
  `kSetupPingPongDisable` (0x08) alone (_grUpdateTriPacketHdr
  gglide.c:2717-2726). Also: glide's GLIDE_TRI_CULLING clears hw-cull for
  independent tris (sw cull instead) — deliberately NOT lifted; kernel uses
  hw culling per design §2.
- **FIFO wrap accounting conservation** (why it's exact): at a writer wrap
  `roomToReadPtr -= roomToEnd` over-charges by (C−w), and when the reader
  later takes the same JMP the update credits a full lap C instead of w
  (fifo.c:944) — the two errors cancel exactly. Verified by a 20k-op
  randomized reader-model test (byte-exact overwrite detection).

## fxD3D M4b-1: gbkernel pure-logic scaffold; h3 headers NOT vendorable verbatim (2026-07-24)

The gbkernel design's "vendor h3regs.h/h3defs.h/h3gdefs.h verbatim" idea fails
self-containedness: `h3regs.h` uses glide's `FxU32` (only typedef'd there under
`#ifdef _H2INC`, h3regs.h:42-44) and expresses registers as volatile structs
(no byte offsets), and `h3gdefs.h` does `#include "cmddefs.h"` which exists
only in the glide tree's **h5** dir, not h3. Extracted instead into
`retro-agent/scripts/3dfx/driver/nt/hw/h3hw.h` — self-contained, every define
cited file:line, byte offsets computed from the all-FxU32 struct walks and
cross-checked against the design doc's §0 list (all matched). Host-test
scaffold: `driver/nt/gbk/` (gbk_packet/layout/state/fifo.c + gbk.h) +
`driver/nt/gbkernel-test/` (`make` = build+run, PASS/FAIL per module). Module
objects compile with `-std=c89 -pedantic -Werror -nostdinc` (no-CRT enforced
on host) and cross-compile clean under `i686-w64-mingw32-gcc`. Make gotcha:
a phony target named `build` collides with a `build/` output dir — renamed dir
to `obj/`.

## fxD3D M4a: REAL DX7 DP2 stream translator in-tree, tested, linked (2026-07-24)

`d3dhal/d3dhal_dp2real.c` + `include/fxd3d_dp2.h` (`fxd_dp2_execute_real`) now
parse the runtime's ACTUAL `D3DHAL_DP2COMMAND` stream (4-byte packed headers,
separate FVF vertex buffer) straight into `fxd_set_renderstate/fxd_set_tss/
fxd_draw` — the simplified `fxd_dp2_execute`/`fxd2_hdr` path stays intact for
the legacy tests. Wired into `driver/nt/enable.c` `d3d_DrawPrimitives2` with
the NT-correct surface deref (`lpDDCommands`→`PDD_SURFACE_LOCAL`→`lpGbl->
fpVidMem`; `lpVertices` raw only under `D3DHALDP2_USERMEMVERTICES`), the
`D3DERR_COMMAND_UNPARSED`+`dwErrorOffset` protocol, `lpdwRStates` mirroring
(<768), and a `GUID_D3DParseUnknownCommandCallback` stash in `DdGetDriverInfo`.
Host test `test/test_dp2real.c` covers the happy path (all tri forms + lines +
points, FVF 0x1C4 and a 28-byte 0x144 stride) plus malformed-input hardening
(truncated header/operands, VB overrun, bad index, unknown op err_off, NULL/
zero-len, bad FVF). `make test` green (3 host tests), mingw winobj green, Wine
DDK `clfxd3d.bat` all EXIT=0 + LINKEXIT=0, zero warnings.

- **Comment gotcha that broke the build:** writing `D3DHAL_DP2*/D3DDP2OP_*` in
  a C block comment — the `*/` in the wildcard TERMINATES the comment and the
  rest of the header parses as garbage (cascaded into bogus `FxU8` errors from
  glide.h). Never put `*/` glob patterns inside `/* */` comments.
- DP2 walk safety pattern that satisfied gcc -Wextra AND VC6 /W3 at once:
  byte-composed LE reads (no casts, no alignment assumptions), per-command
  `need > avail` checks, contiguous-run precheck `vrange_ok` for non-indexed
  prims, per-index `vfetch` bound for indexed, walk always advances ≥4 bytes.

## fxD3D NT display driver `fxd3ddd.dll` now LINKS — clean-room chassis written (2026-07-24)

The clean-room fxD3D NT/2000/XP display driver (`scripts/3dfx/`, the open
Glide-GPL+public-DDK experimental tree — NOT the vintage H5 lane) now links a
complete `fxd3ddd.dll` under the Wine W2K+DX7 DDK. Build loop:
`scratchpad/build_fxd3d.sh` → `clfxd3d.bat` (VC6 `cl`/`link`), LINKEXIT=0.

- **`DrvEnableDriver` was the sole reported unresolved** only because the linker
  builds the `.def` exports file FIRST and aborts (LNK1141) before resolving
  body symbols. Providing `DrvEnableDriver` uncovered **21 more** pre-existing
  unresolveds (the d3dhal core's `gb_*` Glide-backend calls + CRT `malloc/calloc/
  free`). A one-unresolved link is not necessarily one-away from linking.
- **New clean-room files (`driver/nt/`):** `chassis.c` = the framebuffer 2D/PDEV/
  modeset half + the `gadrvfn[]` DRVFN table + `DrvEnableDriver/DrvDisableDriver`
  (unaccelerated DDK "framebuf" pattern, hooks nothing → GDI draws into the
  miniport-mapped linear FB). DDraw enable pair + `Dd*` stubs + `DdGetDriverInfo`
  (answers `GUID_D3DCallbacks3` with `DrawPrimitives2`) added to `enable.c`.
- **`/Gz` gotcha:** the DDK build defaults to `__stdcall` (`-Gz`), so backend
  symbols decorate as `_gb_*@N`; CRT names stay `__cdecl` (`_malloc`). Any shim
  must match the decoration (backend stubs compiled with the same flags;
  `crtshim.c` declares `__cdecl` explicitly).
- **CRT shim:** native `-nodefaultlib` driver has no CRT → `crtshim.c` maps
  `malloc/calloc/free` onto `EngAllocMem/EngFreeMem` (declared locally to avoid
  pulling `<windows.h>`'s CRT prototypes and colliding).
- **`ntddvdeo.h` is NOT on the default DDK INCLUDE** — it lives in
  `w2kddk/src/video/inc`; added that dir to `clfxd3d.bat` INCLUDE. It also needs
  `<devioctl.h>` included first (CTL_CODE / FILE_DEVICE_VIDEO / METHOD_BUFFERED).
- **Genuine blocker / next milestone (M4):** the real driver-side Glide backend
  (Glide minihwc/cinit compiled in) and real DDraw/D3D bodies. `gbstub.c` is a
  link-time PLACEHOLDER (all `gb_*` no-ops, `gb_tex_create`→NULL); the existing
  `d3dhal/glidebackend.c` is user-mode-only (opens a GDI window + glide3x) and is
  NOT linkable into a subsystem:native driver. Every stub is `TODO(fxd3d M4)`.
- Host unit tests (`make -C scripts/3dfx test`) stay green — all new code is under
  `#ifdef HAVE_DDK`; no portable-core / `!HAVE_DDK` path touched.

## Direct3D on the Voodoo3 — root cause of GoldSrc "video mode not supported" (2026-07-23)

Diagnosed live on .124 + against the H5 source. The vintage H5 **D3D HAL WORKS**;
the GoldSrc failure is a **hardware ceiling**, not a driver bug.

- **DDraw mode enumeration is COMPLETE.** A live `IDirectDraw::EnumDisplayModes`
  on .124 lists 640×480×16 (RGB **565**, Rmask 0xf800), 800×600×16, and the full
  8/16/32-bpp matrix 320×200 … 1600×1200. The mode GoldSrc wants is present.
- **The D3D HAL device creates fine — at 16-bit.** A DX7 probe
  (`DirectDrawCreateEx` → `SetCooperativeLevel(EXCL|FS)` →
  `SetDisplayMode(640,480,16)` → primary+flip+3DDEVICE surface → QI `IDirect3D7`
  → `CreateDevice(IID_IDirect3DHALDevice)`) returns **DD_OK at every step** and
  creates a HAL device. So the vintage H5 D3D HAL is functional.
- **ROOT CAUSE = Voodoo3 is 16-bit-3D-only.** `BuildD3DCaps` sets
  `dwDeviceRenderBitDepth = DDBD_16` **only** (H5 `D3INIT.C:3088`; `DDBD_32` is
  gated behind `IS_NAPALM`/VSA-100 at `:3155`). XP desktop is 32bpp; GoldSrc's
  D3D renderer matches the 32-bit primary and asks D3D for a **32-bit** render
  target → no matching HAL device → "the specified video mode is not supported"
  → software fallback. The Avenger core physically renders 16-bit (22-bit
  post-filter → 16-bit buffer); there is **no 32-bit 3D path** — adding DDBD_32
  would advertise silicon that doesn't exist. **Not driver-fixable.**
- **FIX = 16-bit desktop.** `ChangeDisplaySettings` to ×16 succeeds live
  (helper: build `setmode.c` with `-luser32 -lgdi32`). Then the D3D device's
  DDBD_16 matches the primary. (No 5:5:5 mode exists — only 565 — so any app
  demanding a 555 primary would also fail; GoldSrc uses 565, fine.)
- **SEPARATE BLOCKER — CS `-d3d` FULLSCREEN WEDGES the box for ~2-3 min** (agent
  unreachable, recovers when hl.exe dies), even at 16-bit. The DX7 probe (create
  device, no frame render, RestoreDisplayMode, exit) did NOT wedge — so the hang
  is in the **fullscreen D3D render/flip loop**, not device creation. Windowed
  `-d3d` on the plain CS surfaced the HL Autorun (that install's hl.exe chains to
  it) — inconclusive; multiple CS installs on the box (`Program Files\Counter-strike`,
  `Bcs16 Romania\Counter-Strike 1.6`, `Sierra\Half-Life`) muddy game-specific tests.
- **Bottom line:** on the Voodoo3, **OpenGL (our clean-room MesaFX+Glide) is the
  correct 3D path** — it works, is fast, and renders CS. D3D is 16-bit-capped and
  its fullscreen path hangs. The clean-room **fxD3D** HAL (`scripts/3dfx/d3dhal/`,
  D3D→Glide) is the way to put D3D in our repo AND likely dodge the H5 fullscreen
  wedge by reusing Glide's working fullscreen path — but it needs the loadable
  display-driver chassis (2D+DDraw+modeset) written + DDK-built (M3), then a
  risky display-driver swap. Core (M1/M2) + DDI glue done + host-tested.

## Benchmark matrix + game gotchas (2026-07-21, our WFP driver, ICD 0.1.31)

Recorded to specpicks (retro_benchmark_runs, machine .124):
| game | 640×480 | 800×600 | 1024×768 |
|---|---|---|---|
| Quake II (`q2-timedemo`) | 96.7 | — | 46.9 |
| Quake III (`q3-timedemo-four`) | 58.4 | 58.1 | 51.0 |
| RtCW (`rtcw-wolfbench`) | ~56 | (fn hardcodes 640) | — |
| Unreal Tournament (`ut-timedemo`) | ~30 | (needs .ini res set) | — |

**UT failure root cause (FIXED):** UT was returning None fps because the per-run
`taskkill /f` of each fullscreen game spawned a Windows Error Reporting
("X has encountered a problem") dialog; across a multi-game sweep these ACCUMULATED
and BLOCKED UT's Recovery-Mode launch dialog → UT never reached the menu → no
timedemo. Fix: **disable WER/Dr Watson** (`PCHealth\ErrorReporting DoReport=0
ShowUI=0`, `AeDebug\Auto=0`) — now baked into `run_bench.py preflight()`. Also
`ut_ensure_binds` now **auto-stages UTbench.dem** (was a silent missing-file fail).

**Sweep timeout gotcha:** a per-game `timeout` that kills run_bench mid-batch loses
ALL of that game's rows (the DB insert is at the end). Run one game+one mode+2 runs
per call (~4 min) so each COMMITS. RtCW's fn hardcodes r_mode 3 (640); UT ignores
`--modes` (resolution is in UnrealTournament.ini) — both need a small code change
for a real resolution sweep.

**MOHAA — CD-mount SOLVED (2026-07-21):** ISO is on the share at
`Z:\Games\Windows XP\Medal of Honor Allied Assault (2002) - Disc 1.iso` (+ Disc 2).
**Automated ISO mount** via `scripts/mount_iso.py <ip> "<space-free-iso>"` (also
`mount_iso()` in run_bench). DaemonTools 3.47 gotchas: (1) two D-Tools installs —
only the one on the ACTIVE Windows volume (**D:**) is registered; the C: daemon.exe
throws "Product not installed!". (2) daemon.exe is resident (tray) — launch DETACHED
(`start "" /d <dtdir> daemon.exe -mount 0,<iso>`); EXECW tree-kills it. (3) the
d347bus virtual-SCSI driver is already installed+running (creates the virtual CD
drives, F:/G:). (4) path must be SPACE-FREE (staged the ISO to `D:\ISO\MOHAA_CD1.iso`;
the share path has spaces). Verified: mounts to G: = MOHAA_DISK1, MOHAA's CD check
passes, MOHAA **renders the game** (7 threads, past the launcher). Only reads the CD
for verification (not during play), so no local copy strictly needed if a space-free
path is used.
**MOHAA fps STILL open:** MOHAA.exe is a launcher front-end (crash-recovery dialog →
"Play in Normal Mode" click, handled in the bench). The game uses **DirectInput**
(injected keys don't reach it) and **`+exec <cfg>` does NOT run** (verified: a
wait+quit cfg didn't quit) — Ritual's build has a non-standard config/console
mechanism. So a timedemo needs its config-exec path figured out OR a `.dm_` demo
obtained (MOHAA ships none; recording needs the console). `--game mohaa` mounts the
CD + launches + validates render; fps is the remaining bounded step.

**Carmageddon 2** uses **nGlide** (Glide→D3D wrapper), not our OpenGL ICD — would
test our D3D HAL path, not the ICD; and it's a racing game with no timedemo. Not a
clean ICD-benchmark add.

## ✅ SOLVED: voodoo3-wfp.inf loads our driver durably AND runs games (2026-07-21)

The **voodoo3-wfp.inf** (rename-files INF, PnP-installable) is the master unblock:
- Built from voodoo3.inf by renaming display→`3dfxv3d.dll`, miniport→`3dfxv3m.sys`
  (names not in any WFP catalog), dropping glide3x + CatalogFile, repointing
  ServiceBinary/InstalledDisplayDrivers. Lives in
  `toolchain-3dfx/dist/3dfx-voodoo3-wfp-20260721/`.
- PnP-install: `updrv.exe voodoo3-wfp.inf "PCI\VEN_121A&DEV_0005"`. Rebuilds the
  class-key config cleanly (InfPath=oem13.inf, Device0.IDD=3dfxv3d,
  ImagePath=3dfxv3m.sys) — NO corruption, NO WFP revert. (Gotcha: PnP defers the
  file copy to "reboot required" but leaves OLD files; copy our new
  3dfxv3d.dll/3dfxv3m.sys over the targets directly before reboot — they're
  rename-named so not WFP-tracked.)
- Result: **our driver loads** (VIDEODIAG drv_ver=`unknown`, not the in-box
  5.1.2001.0) AND **games run**: RtCW timedemo **54.4 fps** on our WFP-installed
  driver. The rename-NAME does NOT break Glide/games (my earlier "rename breaks
  games" was WRONG — those hangs were the in-box driver loaded + clobbered
  game-local ICD files, not the rename).
- ⇒ This is THE deploy method for our driver on a games box. Supersedes the
  "rename breaks games" caveat below.

Corollary (corrected): the earlier Q2/Q3 "hangs at GL context creation" were the
IN-BOX display driver, NOT clobbered game files. On our WFP-installed driver ALL
games work and match the morning numbers:
- Q2 96.6 fps @640 / 47.1 @1024 (exact morning match)
- Q3 58.6 @640 / 50.8 @1024
- RtCW 54.4-55.9 (wolfbench)
- MOHAA renders (needs CD1 ISO mounted via DaemonTools; no-CD patch optional)
⇒ The single root cause of the whole "games broken" saga was: our display driver
was not loaded (in-box was, via the config corruption + WFP). voodoo3-wfp.inf fixes
it. game-local ICD files were fine.

## Deployment & WFP (critical)

- **WFP silently reverts `D:\WINDOWS\system32\3dfxvs.dll` to the 2001 retail driver
  (689,216 B)** within ~15 s of any raw file swap. `SFCDisable=0xFFFFFF9D` +
  `SFCScan=0` do NOT disable WFP on retail XP. dllcache holds the catalog-validated
  retail pair: display `3dfxvs.dll` 689,216 + miniport `3dfxvsm.sys` 148,352.
- **Two reliable deploy methods:**
  1. **Rename method** (WFP-free): ship display as `3dfxv3d.dll` (not catalogued),
     set `HKLM\SYSTEM\CCS\Services\3dfxvs\Device0\InstalledDisplayDrivers=3dfxv3d`;
     miniport as `3dfxv3m.sys`, `Services\3dfxvs\ImagePath=system32\DRIVERS\3dfxv3m.sys`.
     Sticks across reboots. **CAVEAT: breaks the Glide path — games hang at
     `GLW_ChoosePFD`.** Fine for D3D/3DMark, NOT for OpenGL games.
  2. **PnP/SetupAPI install** (deploy-3dfx-driver skill): `updrv.exe voodoo3.inf
     "PCI\VEN_121A&DEV_0005"` — rebuilds the class-key registry consistently. Seed
     dllcache with our files first so WFP keeps them. This is the correct install.
- **WFP beats dllcache seeding** (2026-07-20): seeding `dllcache\3dfxvs.dll` +
  `\3dfxvsm.sys` with OUR versions before a PnP install did NOT stick — after reboot
  WFP restored the RETAIL files (689,216 + 148,352) to both dllcache and system32.
  WFP has a deeper catalog/driver-store source than dllcache alone. ⇒ dllcache
  seeding is NOT a reliable WFP escape here; only the rename method (non-catalogued
  filename) reliably loads our unsigned display driver. But rename breaks Glide/games.
- **The retail pair 3dfxvs.dll 689,216 + 3dfxvsm.sys 148,352 is MIS-configured**
  under the standard binding on this box: boots to 800×600×4 with "primary display
  adapter is not configured properly" + wallpaper corruption. NOT a clean full-res
  state. The Microsoft **in-box** driver (5.1.2001.0 / 3dfxvs2k.inf, in the driver
  store) is the proven full-res + Glide known-good — prefer it as the stable base
  for GAME benchmarking (games use our ICD, not the display driver's D3D).
- **Manual driver-file swaps + registry edits corrupt the display class key**
  `{4D36E968-E325-11CE-BFC1-08002BE10318}\0000`: after ~12 swap-reboots its
  `Driver`/`Service`/`InstalledDisplayDrivers` values went MISSING (MatchingDeviceId
  survived) → "primary display adapter not configured properly", 800×600. Recovery =
  clean PnP reinstall (rebuilds those values). **Prefer PnP installs over raw swaps.**

## ✅ SOLVED: comprehensive driver logging via registry-ring sink (2026-07-21)

Working end-to-end. The log sink is a **registry ring** (NOT the file IOCTL, which
was unreliable): the display driver flushes log text as REG_SZ chunks
`RLog00..RLog31` (32 × ~1000 B) under `HKLM\SYSTEM\CCS\Services\3dfxvs\Device0`,
with `RLogSeq` (total chunks; newest slot = (RLogSeq-1)&31), via the PROVEN
`SetRegSZ` (IOCTL_3DFX_SET_REGISTRY_VALUE, 0xfd6) — the same channel the driver
uses for all its settings. Agent reads via REGREAD (values are REG_BINARY UTF-16LE;
decode with `utf-16-le`). Helper: `scratchpad/rlog.py` `readlog(c)` reassembles the
ring in write order.
- **Coverage (all agent-readable):** V5DLog display lifecycle
  (DrvEnableSurface/DisableSurface/AssertMode with res/bpp + the Glide
  fullscreen-switch hwcExt escapes), CFIFO flight recorder (H3MakeRoom FIRST-CALL
  positive control + STALL>=100K + WEDGE-BREAK@50M), D3D texture-OOM (the 3DMark
  overcommit path). Verified: RtCW run produced RLogSeq=29 with the full
  fullscreen mode-switch trace.
- **V5DLog is UNCONDITIONAL** (infrequent lifecycle events, always captured — the
  registry-read gate `Retro3dfxLog` via ddgetenv proved unreliable, so don't rely
  on it). Per-op verbose `h3printf` stays gated (rarely needed; would flood).
- **Build:** `LF=1` in both Displays/H5 and Miniport/H5 SOURCES. Ships in our
  driver (the WFP-safe `3dfxv3d.dll`/`3dfxv3m.sys`). The miniport just needs its
  normal SET_REGISTRY handler (no LF needed for the registry sink).
- **Why the old file sink failed:** WRITE_LOG_FILE IOCTL (0xfd7) didn't reliably
  reach the miniport (build/videoprt artifact); SET_REGISTRY (0xfd6, adjacent) is
  used everywhere and works. Also EngDebugPrint is a no-op on free XP. So the
  registry ring is the reliable sink.

## Driver logging (comprehensive-logging effort) — historical notes below

- **`EngDebugPrint` is a NO-OP on free/retail XP** — never reaches DebugView (a
  kernel `DbgPrint` from watchdog.sys DID show, so DebugView itself works). A prior
  session's `V5DLog` instrumentation (EngDebugPrint-based) was therefore always
  silently invisible. Do not rely on EngDebugPrint for driver logging here.
- The driver has a **built-in file logger** gated by `ENABLE_LOG_FILE` (SOURCES
  `LF=0→1`): `H3PRINTF`/`h3printf` buffers → `EngDeviceIoControl(hDriver,
  IOCTL_3DFX_WRITE_LOG_FILE)` → miniport `H3StartIO` → `ZwCreateFile` to a .log.
  This is the RIGHT sink (agent-downloadable file). Built: LF=1 both SOURCES, runtime
  gate `Retro3dfxLog` reg value, `retroLogForce`/`retroLogRaw`, V5DLog rerouted to
  file, CFIFO flight-recorder, miniport log path C:→D: fallback.
- **UNSOLVED BLOCKER:** `IOCTL_3DFX_WRITE_LOG_FILE` never reaches the miniport — a
  registry counter in the handler never increments, even from an unconditional probe
  in DrvEnableSurface. videoprt.sys drops it before dispatch. Output-buffer
  hypothesis tested + wrong. No 3dfxvs.log produced yet. (Xed after 9 rebuilds.)
  Miniport log path: the box's C: is Win98 FAT → kernel write to `\DosDevices\C:`
  silently fails; changed to try D: first.

## D3D / 3DMark

- **★ .143 (V5 5500): 3DMark2001 SE HARD-FREEZES the box** (2× on 2026-07-21:
  first run crashed mid-suite ["Safety Precaution" abort dialog after ~15 min],
  second run froze the machine solid — 100% ping loss, NIC dead, physical reboot
  required). PowerStrip autostart was disabled for run 2 ⇒ NOT PowerStrip; it's
  the **D3D HAL path of our 3dfxv5d.dll wedging** (CMD-FIFO spin class). Key gap:
  .143's deployed `3dfxv5d.dll` is the **2026-07-17 build (595,644 B) WITHOUT the
  `H3MakeRoom` CFIFO spin-breaker** that the .124 lane added for exactly this
  wedge (present in the current CFIFO.C). Post-reboot plan: (1) isolate the
  wedge test by running a reduced 3DMark selection; (2) SUPERVISED: rebuild
  3dfxv5d for V5 from current CFIFO.C (mind the other session's in-flight
  DEBUG.C/ENABLE.C logging edits in the same tree) and deploy with user standing
  by. The OpenGL/game stack is unaffected (ICD path, not D3D).

- Our H5 D3D HAL is the display driver `3dfxvs.dll` (D7D3D.C etc. compiled in). Via
  the WFP-free rename path, 32MB texture tests + full 3DMark2000 COMPLETE on our
  clean rebuild, the original dist 595180, AND retail 689216 — **nothing wedges when
  loaded cleanly.** The original 16/32MB wedge was tied to the std `3dfxvs`+WFP load
  path, not our binary. `H3MakeRoom` CMD-FIFO spin-breaker (CFIFO.C) added as a
  safety net (converts a wedge hang→TDR into a recoverable condition).

## Build toolchain (Wine)

- Display build: `bldw2k.bat c:\3dfx\H5\W2K\Src\Video\Displays\H5` (free/objfre by
  default; `checked` arg for objchk). Miniport: `...\Miniport\H5`. Always
  `export COPYCMD=/Y`, `ulimit -f 2000000`, redirect (never pipe) wine output,
  `timeout`. Purge stale `.obj` before a rebuild or nmake silently skips files.
- Display DLL sizes: 595,180 (base dist) → 595,964 (spin-breaker) → 942,200 (LF=1
  logging). Miniport 195,812 (base) → 199,612 (LF=1 + counter).

## GAMES-STACK BLOCKER (2026-07-20 night) — ICD needs OUR display driver, standard-name

**Symptom:** after tonight's display-driver recovery, ALL games hang at GL context
creation — Q2 at "...calling CDS: ok" (then nothing), Q3 at "GLW_ChoosePFD" — and
produce None fps. This morning the SAME games+harness gave real numbers (Q2 96.6 fps
@640, 47.1 @1024, ICD 0.1.31).

**Isolation (what does NOT fix it):**
- Tried in-box display driver (5.1.2001.0) AND our rename-path display driver
  (3dfxv3d = our 595180) — BOTH hang at context creation.
- Tried in-box glide3x (335,872) AND our retail glide3x (348,160, `_grFoo@N`
  decoration matching our ICD's imports), deployed game-local — both hang.
- retrogl version isn't it: the pre-existing system32 retrogl (2,733,493) hung
  BEFORE I touched it; the deployed 0.1.31 (2,742,298 = the morning-working build)
  hangs too. Both are 0.1.31.

**Diagnosis (corrected):** VIDEODIAG shows the loaded display driver is **5.1.2001.0
= the Microsoft IN-BOX driver**. Games hang at GL context creation on the in-box
driver. Games worked this morning on OUR H5 display driver (standard `3dfxvs` name).
So the ICD's Glide fullscreen path needs OUR display driver — the in-box driver's
DDraw/Glide doesn't satisfy it.
**Why I couldn't test our driver tonight:** the manual rename-path edit
(`Device0\InstalledDisplayDrivers=3dfxv3d`) **did NOT take** — the PnP install of
`3dfxvs2k.inf` (in-box, done during display-config recovery) firmly re-established
the in-box driver in the class key + driver store (`class\0000\InfPath=3dfxvs2k.inf`),
which overrides a manual Device0 edit. Result: I set the rename binding + rebooted
but the box still loaded the in-box 5.1.2001.0 driver. Ruled out (all hang on in-box):
glide3x 335872/344064(AmigaMerlin)/348160, retrogl 2733493/2742298, env
FX_GAMMA/FX_DITHER — all correct, all hang → it's the display driver, not these.

**Why we can't just load our driver standard-name:** WFP reverts standard-name
`3dfxvs.dll` to retail (dllcache seed doesn't beat it — see above), and the manual
rename-registry edit corrupts the class-key config. So the morning stack is not
trivially reproducible.

**The real fix (not yet done):** make our unsigned H5 display driver loadable under
the standard `3dfxvs` name durably. Options, best first:
  1. **Catalog-sign** our driver package (build a `.cat`, test-sign, enable
     testsigning / disable integrity checks) so WFP accepts our `3dfxvs.dll` — keeps
     the standard binding the ICD's DDraw/Glide path requires. THE right path.
  2. **Patch `sfc_os.dll`** to truly kill WFP (offline/registry), then raw-install
     our standard-name files. Heavier/riskier.
  3. Confirm exactly how the morning stack got our driver standard-name (check .124
     driver-store / whether a prior deploy-skill run + dllcache seed stuck before a
     WFP scan) and reproduce that.
Until one of these lands, games run only on the morning-style all-ours standard-name
stack, which the current box state does not have.

## Games / ICD

- **★★ ICD NEVER OPENED >640×480 until 0.3.6 (2026-07-21, .143).** MakeCurrent's
  resolution walk consults a Voodoo1/2-era cap table; V3/V5 hardware strings fall
  through to platform=Voodoo1 + mem=2MB (GR_MEMORY_FB byte-scale value matches no
  case) → best db+Z = 640×480 → every 800/1024 request silently opened a 640×480
  Glide context under an 800/1024 game viewport → BLACK WORLD on the monitor.
  Engine-side timedemo fps still measured "fine", so all pre-0.3.6 "800/1024"
  benchmark labels on the 3dfx-optimized stack are really 640×480 — re-baseline.
  Fix: platform=-1 sentinel for modern boards bypasses the cap gate (0.3.6).
  Corollary: the old "Q3 640/800/1024 flat fps ⇒ engine-bound" evidence is
  partially VOID (flat because identical 640 rendering). Re-test the fill-bound
  question at REAL 1024 before trusting the present-bound conclusions at high res.
- **0.3.4d swap-hook regression (fixed in 0.3.5):** the `__r3d_blitValid` latch is
  NOT GoldSrc-specific — on the 2-TMU config the ICD maps GL unit 0 → GR_TMU1, so
  Q3 latches too and the per-frame combine override killed vertex-color modulate
  (WHITE menu text; stock Q3 menu text is RED — check capture COLOR fidelity, not
  just structure). 0.3.5 gates the hook on `__r3d_sawTMU0` (real dual-texture
  frames only) + clears the latch at grSstWinClose + words-only combine-cache
  invalidation (`__glSSTInvalidateCombineWords`, NOT the full reset).
- **★ CS/GoldSrc GREEN WORLD = stale 2PPC — FIXED (.143 ICD 0.3.4d, 2026-07-20).**
  2PPC (2-px/clock, `combineMode` bit-29) is Glide's SINGLE-texture opt; must be OFF
  for dual-texture world+lightmap. Our ICD's combine-word cache skipped the Glide
  re-issue on GoldSrc's single↔dual flips → `tmuConfig` never invalidated →
  `_grTex2ppc` never re-ran → both VSA-100s mirror → `(0,G,0)` world. Fix = re-issue
  the FULL TMU1 state at SwapBuffers (grTexSource+grTexCombine+grColorCombine+
  grAlphaBlend; bisect-proven — no subset works, and `__glSSTResetCombineCache()`
  there REINTRODUCES green by wiping ext/overbright state). Q3 unaffected (68-74fps).
  Details: `optimized/CS-GREEN-WORLD-LOG.md`, commit 71db1e3.
- **GDI SCREENSHOT of fullscreen Glide/D3D = scanline garble, always** — it cannot
  BitBlt the Voodoo surface. A garbled GDI shot of a running game usually means the
  game IS on the hardware path (software-GL would capture clean). TRUE capture =
  the ICD's FBDUMP (`C:\icd_fbdump.on` → grLfbReadRegion). Caveat: fbdump sits after
  the `!doubleBufferMode` early-return, so single-buffered contexts (RTCW) never dump.
- **idTech/Unreal games detect unclean exits (`taskkill /f`) and block the NEXT
  launch on a GUI dialog** (UT: "Recovery Mode" window; RTCW: safe-mode stall,
  process alive but opengl32 never loads). Automation must click through
  (UICLICK Run-button) or the game "runs but never renders". Prefer in-game `quit`
  cfgs over taskkill where possible.
- **MOHAA (retail, `C:\Program Files\EA GAMES\MOHAA`) is SafeDisc-blocked** on .143:
  modal "Cannot locate the CD-ROM" before any rendering. No local disc image found;
  DAEMON Tools is installed — needs a legitimately-owned image staged/mounted before
  MOHAA can join the benchmark matrix. NOT a driver issue.
- **PowerStrip on .143 autostarted from HKLM Run and pops a "Trial Expiration"
  modal that kills fullscreen-exclusive benchmarks** (likely aborted the first
  3DMark2001 640×480 run). Disabled by renaming the Run value to
  `PowerStrip.disabled-for-benchmarks` (same data — trivially reversible).
- RTCW crashed once in glide3x (NULL `[eax]`, Dr Watson 2026-07-21 00:09) during
  repeated kill/relaunch cycling; later launches render the menu fine. Same
  lost-context/teardown fragility class as the known Glide-exit issues.
- Games use our OpenGL ICD `retrogl.dll` (MesaFX-over-Glide) + `glide3x.dll`, loaded
  game-locally or from system32 — INDEPENDENT of the D3D display-driver path. But the
  ICD's Glide init needs the display driver loaded under its STANDARD name (rename
  path breaks it — ChoosePFD hang).
- Game-local `opengl32.dll` staleness trap: LoadLibrary checks the game dir before
  system32 — update EVERY game-local ICD copy, verify by GL_RENDERER string not size.

## Box ops

- **Autologon** fixed + verified: voidsstr had a BLANK password → set to `password`
  to match `DefaultPassword`; `AutoAdminLogon=1`, `DefaultUserName=voidsstr`. Reboots
  now auto-login to desktop. boot.ini default=XP timeout=30 (unattended reboot safe).
- TDR recovery: our D3D HAL wedge → soft VGA fallback, box + agent stay alive (PING
  ok while VIDEODIAG hangs); reboot restores full res. NOT a hard box-down.
- Daemon (retro-chat) claims .124 but connects on-demand — direct RetroConnection
  works fine; keep sessions short.

## Benchmark collection games (2026-07-21) — findings + per-game reality
Downloaded the Internet Archive "Benchmark Collection" (Benchmarks_v1.iso, 695 MB,
Coleslav) — ~20 games with built-in benchmarks + per-resolution shortcuts, Inno
Setup (silent-installable /VERYSILENT). Extracted installers staged at
~/staging/benchmarks-collection/installers, served on 192.168.1.132:8891.
- **Sin** (idTech2): runs on our driver; **our MesaFX ICD (retrogl) is INCOMPATIBLE
  with Sin's demo playback** — `+demomap cole.dm2` sticks at GL init with our ICD,
  but plays with the bundled **3dfx MiniGL**. Via MiniGL: **29.5 fps @640** (1893
  frames/64.2s). Recorded (sin-timedemo). Command: `sin.exe +set logfile 2 +timedemo
  1 +demomap cole.dm2`; fps in base\qconsole.log; demo takes ~65s (wait >=85s).
- **Incoming** (1998, Glide2x): **HARD-CRASHES our driver** (box unreachable ~2 min,
  then TDR-recovered). We provide Glide3x; Glide2x-era games are a crash risk. SKIP.
- **Hexen II** (GLQuake engine, glh2.exe): runs on our driver via its bundled MiniGL
  opengl32.dll (no crash), per-res benchmark shortcuts (`glh2.exe -width W -height H
  -bpp 16 +timedemo coleslav`), but the GLQuake timedemo fps did NOT reach
  qconsole.log even with -condebug + 75s — console-only output, headless capture
  unsolved.
- **Pattern:** each collection game needs per-game reverse-engineering (exact demo
  command from the .lnk unicode args, fps-output format/location, long waits) AND
  carries crash risk (Glide2x). Autonomous completion is impractical without the
  user present to recover a hard crash (box isn't physically accessible). Best done
  supervised, per-game.
- **Driver-quality signal for the ICD campaign:** our MesaFX ICD has compat gaps
  with older idTech2 games (Sin) where the 3dfx reference MiniGL works — a concrete
  target for ICD improvement.

## Benchmark matrix (our driver, ICD 0.1.31) — recorded in specpicks
| game | 640 | 800 | 1024 |
|---|---|---|---|
| Quake II       | 96.6 | 69.2 | 47.0 |
| Quake III      | 58.6 | 58.1 | 50.9 |
| RtCW wolfbench | 55.9 | 48.0 | 31.9 |
| Unreal Tournmt | 30.0 | 31.0 | 27.4 |
| SiN (MiniGL)   | 29.5 |  -   |  -   |
Context: period P3-850 + Voodoo3 3000 did ~60 fps Q3 @800 (ours 58.1) — competitive.

## D3D / 3DMark2001 SE isolation matrix (2026-07-21, .143 V5 5500, instrumented v5d)

First-ever COMPLETED 3DMark2001 run on the self-built XP driver:
**1636 3D marks** @ 640x480, 16-bit color/textures/Z, D3D Software T&L
(recorded in optimized/benchmarks/3dmark2001se-640x480x16-sli2.json).

| config | result | rendering |
|---|---|---|
| 16-bit, SLI_AA_CONFIGURATION default(2=2-way SLI) | full suite completes, score 1636, ring clean (no FIFO stalls) | **band-corrupted**: alternating good/garbage horizontal bands in ALL tests AND 2D loading screens (user CRT photos IMG_2062-64 + GDI grab) |
| 16-bit, SLI_AA_CONFIGURATION=0 (single chip) | renders CORRECTLY (clean mid-run grabs) | dies mid-suite `swapBuffer:Present : D3DERR_DRIVERINTERNALERROR`; timing varies (~45s fresh process, ~3.5min warm) — NOT a same-session state leak |
| 32-bit / compressed (Jul 17-21 history) | hard freeze or instant Present error | (pre-instrumentation) |

**SLI banding analysis:** slave VSA-100's bands are unrendered memory ⇒ slave never
executes the D3D command stream. Miniport SLI programming (H3_SETUP_SLI_AA) is
IDENTICAL code for the working Glide/OpenGL path (HWCEXT_SLI_AA_REQUEST → same
IOCTL; Q3 @1024 = 71.7 fps proves GL 2-way SLI works). v56k SLIAA.C changes are
pure gated additions (diff-verified, 0 vintage lines touched) ⇒ vintage bug in the
never-shipped XP D3D display-side SLI path (promote/CMDFIFO-vidmem/snoop interplay),
NOT our regression. Overlay promotion is REQUIRED for SLI (DDFXNT.C:2463 comment).

**DdFlip freeze vector found:** DDFLIP.C:342 `while (READSWAPCOUNT() > swapsQueued);`
unbounded spin — if hw stops retiring swaps this loops forever at raised IRQL =
the 3DMark hard-freeze signature. Now bounded @50M with ring log (instr2 build).

**Instrumentation added (3dfxv5d instr2, 952,120 B, deployed 2026-07-21 14:15):**
DP2-PARSE-ERR / DP2-EXIT-ERR (hr + failing opcode + offset at every
DrawPrimitives2 error exit), PROMOTE-SLIAA / PROMOTE-SLIAA OK / DEMOTE-SLIAA /
COMPUTE-SLIAA (full multi-chip request + primary hwPtr), DdFlip WEDGE-BREAK@50M.
Next repro of the Present error will name the failing D3D op in RLog.

## Supervised titles round 2 (2026-07-21) — Hexen II + 3DMark2000
- **Hexen II (glh2.exe, GLQuake engine)** — installed as `D:\Games\Heretic2`
  (mislabeled dir; it IS Hexen II). Its bundled 1997 `opengl32.dll` (MiniGL,
  126,464 B) **CRASHES at GL-context creation** under our WFP display driver
  (log stops at `640x480x16`, process exits). **OUR ICD runs it** — stage
  `retrogl.dll`→`opengl32.dll` + `glide3x.dll` (from `C:\Games\Quake2`), and it
  initializes (`GL_RENDERER: Mesa Glide v0.62 Voodoo3 [retro3dfx 0.1.31]`) and
  plays. **Our ICD: 109.9 fps @640 (5414f/49.3s), 48.7 fps @1024 (5414f/111.3s).**
  Recorded to specpicks (hexen2-timedemo). This is a POSITIVE ICD data point that
  contrasts SiN (where our ICD fails but MiniGL works) — our ICD's GLQuake-engine
  compat is title-specific.
  - **fps-capture SOLVED** (was "unsolved" in round 1): `glh2.exe -condebug
    -width W -height H -bpp 16 +timedemo coleslav`; the fps line lands in
    `data1\qconsole.log` as `NNNN frames  S seconds  F fps`. The round-1 "no
    capture" was really the MiniGL crashing before playback. **Wait scales with
    res**: 5414 frames at ~49 fps @1024 = ~111 s wall — poll to ~130 s, don't
    time out at 90 s.
- **3DMark2000 (D3D)** — installed `D:\Program Files\MadOnion.com\3DMark2000`.
  Launches to a clean GUI (detects our card), but **"Run Default Benchmark"
  (1024×768×16 D3D) CRASHES our display driver**: TDR dialog "The 3dfxv3d display
  driver has stopped working normally." Driver auto-recovered (VIDEODIAG still
  ours, desktop intact, no reboot needed). ⇒ **Our WFP build's Direct3D HAL is
  unstable/incomplete** — D3D-path benchmarking (3DMark2000/2001) is NOT viable
  until the D3D HAL is hardened. All our working benchmarks are the OpenGL-ICD
  and Glide paths. (This is why supervised: an unsupervised 3DMark run would have
  left the box in the TDR dialog.)

## ICD compiler-flag opt lane on .124 (2026-07-21) — /Ob2 verified-neutral
`.124` = ~845 MHz P3 + Voodoo3. Q3 timedemo flat ~58 fps at BOTH 640 and 800 ⇒
**CPU-bound at ≤800** on this box (contrast the GPU-bound Voodoo5 .143). A/B of
OGL.MAK item 1 **/Ob2** (add to release `/O2 /G6`; ICD 704512→729088 B, more
inlining): baseline 56.8/58.6/58.3 (avg 57.9, cold first run) → /Ob2
58.6/58.9/58.5 (avg 58.67). = +1.3% raw / ~neutral trimmed, zero regression,
byte-identical codegen semantics. **KEPT** (safe, non-negative). Corroborates the
.143 finding that C-codegen flags (/G6) are ~INERT: even CPU-bound, the hot cost
is the x87 hand-asm T&L + per-triangle Glide submit (`__GL_USE_INTEL_ASM`), not
the C the flag touches. ⇒ pure-flag ceiling is low; real wins are the C/asm
restructures (queue items 4-9, esp. #8 vertex-dedup ~10-25%). Next flag with
actual reach: /QIfist (item 3) — kills __ftol fldcw serialization in hot C
float→int casts.

## ICD flag A/B CORRECTED + 2.7MB-build discrepancy (2026-07-21)
**The earlier "/Ob2 = 58.67" was CONFOUNDED by the game-local deploy trap.** run_bench
launches quake3.exe from `C:\Quake III Arena\Quake3`, which had a game-local
`retrogl.dll` = the **2,742,298-byte** deployed build. Game-local shadows system32,
so my system32-only /Ob2 deploy was never loaded; the run measured the 2.7MB build
(~58 fps). The qconsole "LoadLibrary system32" line I trusted was STALE (from a
manual launch out of `C:\q3home`, which has no game-local copy). LESSON (re-confirmed,
matches [[text-garble-solved-alignment]]): trust NOTHING but a fresh qconsole; when
A/B-ing the ICD, NEUTRALIZE the game-local copy (rename it) so only the build under
test loads — or deploy to BOTH system32 AND every game-local path.

**Clean 3-way A/B (game-local renamed aside, all loaded from system32, confirmed):**
| build | flags | Q3@640 avg (3 runs) | vs base |
|---|---|---|---|
| base   | /O2 /G6 (704,512 B)        | 53.53 | — |
| /Ob2   | +/Ob2 (729,088 B)          | 53.80 | +0.5% (noise, non-neg) |
| /QIfist| +/Ob2 /QIfist (729,088 B)  | 52.73 | **−1.5% → REJECTED** |
/QIfist REJECTED: net regression on this box (the __ftol removal doesn't offset;
and it's a semantic chop→nearest change — reject a regressing semantic change).
/Ob2 kept (safe, marginally positive). Confirms the pure-flag ceiling here is ±1%.

**THE REAL FINDING: the deployed 2.7MB ICD (~58 fps) is ~8% FASTER than a clean
release rebuild from `v56k-6000` source (base = 53.5 fps).** The compiler flags are
±1% noise next to this ~4-fps source-level gap. The fast deployed driver
(3dfxvgl.dll, 2,742,298 B) was built from a config/branch that v56k-6000 does NOT
reproduce. ⇒ before more flag tuning, must identify + rebuild from the source that
produces the fast 2.7MB build (investigation in progress). Optimizing on the slower
v56k-6000 base would ship a regression vs what's already deployed.

## RESOLVED: the fast ICD is a DIFFERENT tree; compiler+math lane already exhausted (2026-07-21)
The 8% gap resolves cleanly — **I was optimizing the wrong source tree.**
- **Fast deployed ICD (2,742,298 B, ~58 fps)** = the **`retro3dfx-gl` GitHub fork**
  (MesaFX 6.2.2), cross-built with **mingw gcc-13** via
  `retro-agent/retro3dfx/build-mesafx-retail.sh`. Its compiler+math flags
  (`Makefile.mgw:74`): `-O2 -ffast-math -march=pentium3 -mtune=pentium3
  -mfpmath=sse -DNDEBUG`. THIS is the driver on .124. Rebuild via that script
  (needs `build-stack.sh` once first).
- **The 704 KB `opengl.dll`** I A/B'd today = the **retro-3dfx SWLIBS MSVC6/Wine**
  tree (`v56k-6000`) — a SEPARATE, slower lineage NOT deployed to .124. My
  /Ob2(+0.5%)//QIfist(−1.5%) results are real but on the wrong tree ⇒ irrelevant
  to the deployed driver. `icd-opt-ob2` branch keeps the /Ob2 experiment; NOT
  merged (inert + wrong tree).
- **The gcc compiler+math lane on the CORRECT fork was ALREADY RUN & EXHAUSTED
  2026-07-17** (retro-agent/retro3dfx/CHANGELOG.md 0.1.7–0.1.11): `0.1.7 opt/lto`
  **-O3 -funroll-loops = 58.7 INERT** ("hot path already SSE; -O can't remove
  algorithmic cost"); SSE cliptest intrinsics **REGRESSED** (Vanderhoof x86 asm
  wins); SSE emit INERT. Only `0.1.11` lod-bias (quality) merged. **Verdict:
  MesaFX/V3 vertex path is near-optimal; it already beats AmigaMerlin + era 3dfx
  ICD.** ⇒ No meaningful compiler+math headroom remains. Today's MSVC-tree A/B
  independently reached the same conclusion (flags ±1%, cost is in the asm).
- **Remaining ICD levers are NOT compiler/math:** quality knobs (lod-bias done;
  gamma/dither done), higher-effort compat/features (texture_env_combine on V3,
  ARB pixelformat, S3TC — 0.1.30 review open items), or accept the V3 hardware
  ceiling (fillrate at high-res + single-TMU). PGO was deferred (needs on-target
  instrumented run) but is low-EV given -O3 was inert.
- **DEPLOY DISCIPLINE (cost me a confounded run today):** .124's Q3 dir
  (`C:\Quake III Arena\Quake3`) has a game-local `retrogl.dll`; run_bench launches
  from there so game-local SHADOWS system32. A/B the ICD by neutralizing the
  game-local copy OR deploying to system32 AND every game-local path.

## ROOT CAUSE FOUND+FIXED: black/garbage mipmapped textures in D3D (2026-07-21)

**Symptom:** all mipmapped D3D content black (3DMark trucks/dragons) or garbage-
striped; un-mipped content (HUD, skybox, 2D) perfect. GL/Glide unaffected.
**Repro:** minimal windowed D3D8 lab (`toolchain scratchpad d3dlab.exe`, staged
C:\RETRO_AGENT\d3dlab.exe): `big512mip` = solid black, `mippoint` = garbage
stripes, any no-mip mode = perfect. Deterministic in 5 s — no benchmark needed.
**Root cause:** W2K `D3TXTR.C` rev 40 (10/25/00, days before 3dfx shut down,
"no longer use surface local pointers") deleted the per-LOD board-offset line in
TEXTURELOAD's mipmapped path, leaving `addr` stale for every mip-level download:
`addr = psurfDst->mmData[nDstLOD].fpVidMem - _FX(textureHeapStart[tmuCnt]);`
(Win9x rev 35 has the line; the dangling "// board address offset" comment
made the deletion visible.) Texels landed at stale offsets; the TMU sampled
unwritten memory.
**Fix:** restored the line (commented). Verified on .143: all d3dlab mip modes
correct (marker-color mips sample at right LODs), mippoint garbage → perfect
checker.

## Warm-rerun D3D degradation (2026-07-21, post-mip-fix) — NEXT INVESTIGATION
3-run warm-rerun loop (same 3DMark process, 640x480x16, single-chip, mip-fix
driver): run0 ended ~4min no score, run1 bounced fullscreen/desktop and ended
~70s no score, run2 produced a score window after only ~44s (tests failing
progressively faster). Fresh-boot runs complete the full ~6min suite cleanly
(1601 marks). Pattern = driver state degrades across repeated D3D device
create/destroy cycles in one boot (leak: texture handles / heap fragmentation /
D3 context state). Matches the historic "errors INSTANTLY after previous
activity" reports. Ring showed no DP2/WEDGE errors -> the failure path returns
clean errors to the runtime or dies in the runtime. Next: instrument
D3CONTXT create/destroy + heap stats into the ring, run N warm reruns, watch
for monotonic drift (fresh boot needed between reliability experiments).

## RESOLVED: warm-rerun "degradation" is 3DMark2001, not the driver (2026-07-21)
Decisive test: 4 COLD-process runs (kill+relaunch 3DMark each time, NO reboot,
same instr7 driver) = **['ok','ok','ok','ok']** — every fresh-process run
completes the full suite. WARM reruns (Benchmark again in the same process)
failed randomly ~2/3. Conclusion: 3DMark2001SE corrupts its own D3D state on
re-benchmark within one process; a fresh process clears it. Corroborated by
the driver being provably clean across every warm-rerun failure (instr4-7 ring:
CTX create/destroy balance returns to 0, dd3DSurfaceCount flat, ZERO
CREATESURF-FAIL/CSEX-FAIL/ALLOC-FAIL/DP2-ERR/WEDGE lines; DP2-FIRST fires =
first frame drawn) and by the random (not monotonic) pass/fail pattern.
**Benchmark protocol: one fresh 3DMark process per run** (dm_freshproc.py /
dm_wrap.exit-wrapper). Real-world launch-and-run is unaffected. NOT a driver bug.

## RESOLVED: D3D "2-way SLI banding" was the mip bug too (2026-07-21 evening)
Re-tested D3D 3DMark2001 on 2-way SLI (SSTH3_SLI_AA_CONFIGURATION default=2 on
5500) with the mip-fix driver: **Car Chase renders perfectly, zero banding**
(sli_midrun.png). Ring confirms SLI genuinely active: PROMOTE-SLIAA
`sliEn=1 aaEn=0 nlines=16 chips=2` + PROMOTE-SLIAA OK. The earlier "alternating
good/garbage horizontal bands" (CRT photos IMG_2062-64) were NOT a slave-chip
command-stream fault — they were the rev-40 mip-download bug's garbage texture
memory being scanned out through the SLI band-interleave, which mimicked SLI
banding. The mip fix (08fd889) closed this open item. D3D 2-way SLI works.
Lesson: don't attribute a display-interleave-shaped artifact to the SLI path
before ruling out texture/framebuffer content corruption.

## Benchmark rerun 2026-07-21 evening (all fixes verified in place)
Predeploy gate PASS (40 checks); on-target D3D suite 17/17; OpenGL golden gate
PASS (Q3 1024 world non-black nb=84 gr=0, CS de_dust 0-green). Q3 timedemo
(2-way SLI, ICD as deployed): 640=68.2, 800=72.9, 1024=70.6 fps (vs prior
72.8/73.0/71.7 — within run variance, no regression). D3D single-chip 3DMark
1601; 2-way SLI run pending.

## OPEN (new, 2026-07-21 night): D3D device-cycle accumulation → display wedge after ~12 cycles/boot
Distinct from the 3DMark warm-rerun app issue (that was one process; this is
fresh processes). Running d3dlab (each a fresh D3D device create/destroy) in
sequence: the first ~12 device cycles per boot render fine, the ~13th wedges
the display (SCREENSHOT hangs, box starves ~60-90s then self-recovers — NOT a
hard freeze, the wedge-breakers hold). Evidence: on-target d3dlab suite passes
12 modes then hangs on the 13th (dxt1); dxt1 run FIRST (standalone) passes, so
it's the cycle COUNT not the mode. Cold 4-process 3DMark passed (4 < 12).
Implies a driver-side per-device resource not fully freed on device/process
teardown (kernel-side display-driver allocation; CTX create/destroy ring
balance returns to 0, so it's NOT the D3D context — suspect a heap/handle-list
or exclusive-mode artifact in DDMEMMGR/DdCreateSurface/HNDLLIST). Real-world
impact: low (needs 12+ D3D app launches without reboot) but a genuine leak.
Mitigation in place: on-target suite runs a curated 9-mode set (under
threshold); full 14-mode matrix via RETRO_D3DLAB_MODES=all on a fresh boot.
NEXT: instrument per-device heap free counts + HNDLLIST alloc/free balance
across N cycles; find the unfreed allocation. Deferred — driver is otherwise
stable and this needs careful measurement, not a blind fix.

**CAVEAT (contention):** during this investigation the agent version changed
under me (1.15.0 → 1.15.6), i.e. a CONCURRENT session was actively working
box .143. The progressively-faster wedging (1-2 cycles late, vs 12 early) is
likely confounded by that session's own D3D/agent activity on the shared box.
The device-cycle accumulation is real (reproduced when I had the box to myself:
12 modes then wedge) but its exact threshold and whether reboots fully clear it
need a DEDICATED, UNCONTENDED session to measure. Not a hard freeze in any case
(wedge-breakers hold; box self-recovers). Box left clean-rebooted + healthy.

## RESOLVED (2026-07-22): "device-cycle accumulation" is NOT a driver leak
Investigated the open item with instr8/9 (kernel-pool + video-surface alloc/
free balance counters in the display driver, read from the persistent registry
ring — immune to agent/network confounds). Controlled, single-session cycling:
  - 16x sel1 (tiny 64x64):            poolLive FLAT 36, no wedge
  - 16x big512mip (512x512 mipmapped): vidSurfLive FLAT 1, nullFree=0, no wedge
  - 16x big512mip WITH mid-render GDI SCREENSHOT each cycle: no wedge
Both kernel pool AND video-memory surfaces are freed cleanly on every D3D
device teardown. There is NO monotonic resource leak. The driver's windowed-
D3D device create/destroy cycle is clean.

**The earlier apparent ~12-cycle "wedges" were a CONCURRENT-SESSION artifact.**
During these tests the agent version churned 1.15.0 -> 1.15.6 -> 1.16.0 — a
second session was actively rebuilding and REDEPLOYING the agent on the shared
box .143 (agent redeploy = agent restart, sometimes reboot). My multi-minute
cycle sequences overlapped those deploys; an agent restart mid-sequence times
out the client connection and looks identical to a display wedge. dxt1 "wedged
on cycle 1" exactly as the agent went 1.15.6->1.16.0. Single-mode runs that
happened to fall between deploys ran 16 clean cycles.

CONCLUSION: no driver fix needed for this item. The D3D device cycle is leak-
free (proven). The on-target suite's curated-mode workaround (added when the
cause was thought to be a leak) is unnecessary but harmless; the full 14-mode
matrix (RETRO_D3DLAB_MODES=all) passes on an UNCONTENDED clean box. For future
driver verification on .143, coordinate with any concurrent session or use a
window when the agent version is stable. Leak-balance instrumentation retained
(instr9) as a permanent diagnostic.

## DEFINITIVE (2026-07-22): the display driver's D3D cycle is CLEAN — "wedge" is agent/box-level
Read the FULL flight recorder after a client-observed dxt1 "wedge" (cycle 13,
no screenshots, agent version stable 1.16.0). EVERY cycle in the ring logged
cleanly: CTX-CREATE (poolLive=34, vidSurfLive=1, nullFree=0) -> DP2-FIRST (drew)
-> CTX-DESTROY (balanced) — including the dxt1 cycle. NO ALLOC-FAIL, NO
WEDGE-BREAK, NO DP2 error anywhere. poolLive and vidSurfLive are ROCK STEADY
across every cycle. So the display driver did NOT wedge or leak — it completed
dxt1's create/draw/destroy with balanced resources.

The client-observed "UNREACHABLE" is therefore the AGENT/box going transiently
network-unresponsive, NOT a display-driver defect. Corroborated by
non-determinism: sel1-cycle-1 went unreachable once (sel1x16 passed cleanly
elsewhere); big512mipx16 never failed; the failure point isn't a fixed mode or
count. This is environmental (a concurrent session was actively rebuilding +
redeploying the agent, 1.15.0->1.16.0, and exercising the shared box .143
throughout).

**RESOLUTION of the open item:** NO display-driver fix is warranted. The D3D
device create/destroy cycle is proven leak-free (pool + video memory balanced
across 16+ cycles) and clean (flight recorder shows no wedge/error even on the
cycle the client called a wedge). The instr8/9 balance instrumentation is
retained as a permanent diagnostic.

**One agent-domain follow-up (not display-driver):** the agent's GDI SCREENSHOT
(DrvCopyBits/DrvBitBlt) during an active D3D present is the operation most
correlated with client stalls. If it recurs on an UNCONTENDED box, audit the
2D GDI blit path for an unbounded accelerator wait (the 4 wedge-breakers cover
the DDraw/3D paths H3MakeRoom/DdFlip/FXBUSYWAIT/H3_GP_WAIT; a pure-2D DrvBitBlt
spin, if any, is not yet bounded). Deferred: needs a clean box to reproduce,
and the driver's own log shows no wedge, so this is a low-priority robustness
audit, not a confirmed bug.

## FOLLOW-UP COMPLETE (2026-07-22): all accelerator waits bounded; screenshot path was already safe
Executed the deferred 2D-path audit. Result: the LIVE GDI screenshot readback
path — DrvCopyBits -> START_DIRECT_ACCESS_H3 (H3G.H) -> H3_GP_WAIT — was ALREADY
bounded by the H3_GP_WAIT wedge-breaker added during stabilization (fc8e313).
That is exactly why the display driver never actually wedged (flight recorder
stayed clean through every "wedge"): the screenshot's own engine wait was
already protected. So the earlier client "unreachable" events were agent/box
transients, not this path.

Completed the coverage anyway (every accelerator wait now bounded):
- LIVE DDraw surface-lock busy spins: DDSURF.C x3, DDOVL32.C x2 -> FXBUSYWAIT
  (already bounded). LIVE flip-status waits: DDSURF.C DdLock + DDFLIP.C DdFlip
  -> new DdLock-FlipWait / DdFlip-FlipWait breakers.
- Dead-code blit spins (BITBLT.C x4 under PERF_COPY_BITS_OPT, DDFXNT.C x1 under
  ENABLE_V3_W2K_GLIDE_CHANGES — both #ifdefs off) bounded defensively.
Deployed instr10, verified NO regression: D3D 5 key modes render correct
(sel1/dxt1/big512mip/tex2/mod2x match goldens), OpenGL golden gate PASS (Q3
1024 world nb=84 gr=0, CS de_dust 0-green). Predeploy gate + source/binary
assertions updated. The stabilization follow-up is closed.

## 2026-07-22 — clean-room glide (Voodoo3) bring-up: TLS accessor crash (getThreadValueFast)

**Symptom:** clean-room `glide3x.dll` (voodoo-cleanroom, MesaFX ICD path) crashed
Q3 at `GLW_ChoosePFD` with `instruction at 0x06f5a22d referenced memory at
0x0000001c` (NULL+0x1c read). Same MesaFX ICD works fine on retail glide.

**Root cause:** `getThreadValueFast()` (glide3/src/fxglide.h) reads the TLS slot
straight out of the TEB via `%fs:` + `_GlideRoot.tlsOffset`. Our mingw/gcc-13
build's inline-asm variant faulted inside `grGetString`'s `GR_DCL_GC` — the first
glide entry to actually *read* TLS (detect/select only *write* via
`setThreadValue`/`TlsSetValue`). Fix: use the ABI-correct `TlsGetValue(tlsIndex)`
instead of the raw TEB read. (tlsIndex measured =18, OSWin95=0, so the classic
high-index/OS-mismatch theories were NOT the cause — the raw `%fs:` read itself
was the problem under our toolchain.)

**Deadly-trap corollary — stale objects on header change:** the era Makefiles do
NOT track header dependencies. Editing `fxglide.h` (where `getThreadValueFast` is
an inline) did NOT recompile `diget.o` et al — the old `%fs:` asm stayed in the
DLL and the crash persisted through a "successful" rebuild. Always
`find glide3 minihwc -name '*.o' -delete` before rebuilding after a HEADER edit.

**Result:** after the fix, `grGetString(GR_HARDWARE)` returns "Voodoo3 (tm)",
Q3 gets "3 PFDs found / hardware acceleration found / PIXELFORMAT 2 selected",
and **grSstWinOpen is reached** (was never reached before). Crash moved deep
into `hwcInitVideo` (next layer). Layers solved so far: base-mapping,
MMIO-read (dramInit1=0x40530031 real HW), detect+bInfo, grGlideInit, TLS.

## 2026-07-22 — clean-room glide RENDERS Q3 (lost-context NULL deref fixed)

After the TLS fix (above), grSstWinOpen reached `hwcShareContextData` →
`*gc->lostContext = FXFALSE` and crashed: gc->lostContext == NULL. Root cause:
`hwcShareContextData` NT/CONTEXT_DWORD_NT branch (minihwc.c) stored the driver's
`dwordOffset` with NO fallback, while the SHARE_CONTEXT_DWORD (non-NT) and linux
branches fall back to `&dummyContextDWORD`. Our display driver maps no
lost-context dword → NULL. Fix: mirror the `&dummyContextDWORD` guard in the NT
branch. **Result: Quake 3 renders correctly** on the clean-room open-source stack
(retro3dfx-glide + MesaFX retrogl ICD) on .124 Voodoo3 — full HUD, textures,
lighting, in-world text, player models (verified via windowed LFB->GDI capture;
fullscreen Glide output isn't GDI-capturable on 3dfx).

Full layer sequence solved this session (all in the clean-room glide bring-up):
base-map (GETLINEARADDR-before-ALLOCCONTEXT) → MMIO read (dramInit1=0x40530031
real HW) → detect+bInfo → grGlideInit → **TLS accessor (TlsGetValue)** →
**lost-context NULL fallback** → hwcInitVideo → render. Fork commit a71eb3f
(voidsstr/retro3dfx-glide @ glide-devel-sezero).

OPEN: intermittent crash under FULLSCREEN (r_fullscreen 1) — windowed renders
clean; fullscreen sometimes stops at GLW_ChoosePFD (possibly a stale crash
dialog stealing the DDraw exclusive mode switch). Next: broader game/res sweep +
fullscreen stability.

### clean-room glide Q3 fullscreen resolution sweep (2026-07-22, .124 Voodoo3)
All 1260-frame timedemos COMPLETED — stable across resolutions, no crashes:
  640×480  46.0 fps   (retail ref 58.8)
  800×600  44.3 fps   (retail ref 58.4)
  1024×768 39.2 fps   (retail ref 51.2)
Clean-room glide is ~78–80% of retail glide speed → optimization target (the
render/FIFO/LFB path, not the vertex path which the ICD campaign already tuned).
Stability is solid. NOTE: a ghost "quake3.exe - Application Error" dialog (no
owning process, survives taskkill + UICLICK) lingers from an earlier crashed run;
cosmetic — does NOT block rendering (sweep ran with it present) — cleared by reboot.

### clean-room glide multi-engine validation (2026-07-22, .124 Voodoo3)
The clean-room glide renders BOTH engines via the full open-source stack
(our glide3x + our retrogl/MesaFX ICD), each completing a full timedemo:
  Q3 (id Tech 3)  640×480  46.0 fps  (retail 58.8)
  Q2 (id Tech 2 / ref_gl)  640×480×16  88.4 fps  (retail 93.6)
     GL_RENDERER: "Mesa Glide v0.62 Voodoo3 (tm) [retro3dfx 0.1.31]", clean ShutdownGame.
Consistent ~78–94% of retail glide speed → the clean-room glide's render/FIFO/LFB
path is the optimization target (vertex/ICD path already tuned in the ICD campaign).
Deploy: OUR glide3x.dll next to each game exe (LoadLibrary search order); Q2 also
needs gl_bitdepth 16 + gl_mode 3.

## 2026-07-23 — CS 1.6 / GoldSrc "display mode not working" on .124 Voodoo3: FIXED (OpenGL), D3D not viable

**Symptom:** CS 1.6 launched from the desktop failed with "display mode not
working" / "Video mode change failure".

**Root causes (two, both fixed):**
1. **PowerStrip** (a display-mode-hooking tuning util) auto-started (HKLM\...\Run
   `PowerStrip=d:\program files\powerstrip\pstrip.exe`), had crashed, and its
   "Safety Precaution" recovery dialog blocked startup AND it hooks
   ChangeDisplaySettings — breaking games' fullscreen mode switches. FIX: removed
   its autostart + killed it. (This is the "Safety Precaution" dialog seen on
   base HL too — it's PowerStrip, not the game.)
2. **Broken GoldSrc video config**: `HKCU\Software\Valve\Half-Life\Settings`
   `EngineDLL=sw.dll` (software) + ScreenWidth/Height **swapped** (480×640). FIX:
   `EngineDLL=hw.dll`, ScreenWidth=640, ScreenHeight=480, ScreenBPP=16.

**OpenGL — WORKS via our clean-room stack (CS menu + de_dust render):**
- Deploy OUR **retrogl ICD AS `gldrv\3dfxgl.dll`** (GoldSrc's `EngineGLDriver=3dfxgl.dll`
  loads it as the "3Dfx OpenGL" driver). Our retrogl uses **Glide-exclusive
  fullscreen** (grSstWinOpen, like Q3) which **bypasses the GDI ChangeDisplaySettings
  mode-switch** that fails with the stock 3dfx MiniGL ("Video mode change failure").
- Pair with OUR **clean-room glide3x** — it exports `grAADrawTriangle@24`, which our
  retrogl imports but the **retail AmigaMerlin glide3x does NOT export** (→ "Entry
  Point Not Found: grAADrawTriangle@24 in glide3x.dll" if paired with retail glide).
- system32\glide3x is **WFP-protected** → seed `dllcache\glide3x.dll` with ours
  FIRST, then system32 (WFP escape), so ours persists.
- `FX_NO_PALETTED_TEXTURE=1` (CS paletted textures).
- **GOTCHA — opengl32 is a KnownDLL:** a game-local `opengl32.dll` (retrogl) is
  IGNORED (Windows loads system32's). Deploy retrogl **as `gldrv\3dfxgl.dll`**, not
  as game-local opengl32.
- **GOTCHA — BCShield** (BC Romania build anti-cheat) kills `hl.exe` if launched
  directly; launch via `Counter-Strike.exe`. (The menu is stable; the earlier
  `+map` "exits" were BCShield, not the driver — de_dust rendered fine first.)

**Direct3D — NOT viable for GoldSrc on the Voodoo3:** `-d3d` → "Video mode change
failure: the specified video mode is not supported → software mode", at every
resolution. D3D on .124 goes through the **vintage H5 D3D HAL (3dfxv3d.dll), NOT
our clean-room stack** (which has no D3D HAL); the H5 HAL doesn't enumerate the
DirectDraw/D3D fullscreen mode GoldSrc's (deprecated) D3D renderer wants. No
Glide-fullscreen bypass exists for D3D (it must use DDraw mode enumeration).
OpenGL is the correct GoldSrc path on 3dfx (as it always was).
