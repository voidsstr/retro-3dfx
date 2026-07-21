# retro-3dfx — Running Findings Log

Living log of important, hard-won findings for the 3dfx Voodoo3 (.124) driver work.
Append new findings as they're uncovered; keep newest-first within each section.
Detailed narratives live in `D3D-DRIVER-PLAN.md`; this file is the quick index of
"things that cost us time and we must not forget."

Target box: **.124** = "ADMIN", XP SP3, Voodoo3 AGP
`PCI\VEN_121A&DEV_0005&SUBSYS_1037121A&REV_01`, active Windows on **D:**, C: is a
Win98 FAT volume. Agent 1.14.0. Autologs in as voidsstr/password.

---

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

## Driver logging (comprehensive-logging effort)

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
