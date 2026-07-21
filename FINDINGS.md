# retro-3dfx — Running Findings Log

Living log of important, hard-won findings for the 3dfx Voodoo3 (.124) driver work.
Append new findings as they're uncovered; keep newest-first within each section.
Detailed narratives live in `D3D-DRIVER-PLAN.md`; this file is the quick index of
"things that cost us time and we must not forget."

Target box: **.124** = "ADMIN", XP SP3, Voodoo3 AGP
`PCI\VEN_121A&DEV_0005&SUBSYS_1037121A&REV_01`, active Windows on **D:**, C: is a
Win98 FAT volume. Agent 1.14.0. Autologs in as voidsstr/password.

---

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
