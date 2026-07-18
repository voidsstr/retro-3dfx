# retro-3dfx debug knowledge (fast reference)

Running log of hard-won debugging facts for the Voodoo5/.143 driver. Newest
insight at top of each section. Grep this before re-investigating anything.

## ★★ BIGGEST REFRAME (2026-07-18): the DISPLAY/SCANOUT is broken, not rendering
**Symptom:** user launches Q3 → the monitor switches to 640×480 but shows the
**DESKTOP, not the game** ("game is not even visible"); earlier the games showed
but with "garbled" text.
**Yet:** glReadPixels captures of the same runs are clean-ish, and timedemos
produce correct fps. So **the ICD renders correctly into the Glide framebuffer;
the failure is getting that framebuffer onto the monitor (video scanout).**
**⇒ The bug is in the DISPLAY path (glide3x mode-set / miniport video processor /
display driver scanout), NOT the ICD texture/font path.** All the ICD font/aspect/
sScale/filtering investigation (below) was on the wrong layer — the framebuffer
was basically fine the whole time.
**Consequence:** "garbled text" the user saw = a scanout/video-output artifact
(and/or the 22-bit postfilter), not ICD glyph rendering. glReadPixels CANNOT
verify the fix (it reads the framebuffer, pre-scanout) — must verify on the
monitor (user present).
**Next:** investigate the fullscreen-Glide video setup: `grSstWinOpen` video
config in glide3x, the miniport `vidProcCfg`/FBIINIT scanout, and why the Glide
surface doesn't take over the monitor (DirectDraw exclusive / mode handoff).
Likely files: H5/GLIDE3 mode-set, H5/W2K/.../Displays/H5/DDINIT.C + DDFLIP.C,
Miniport/H5/h3modeset.c.

## ★★★ ROOT CAUSE FOUND (2026-07-18): FRANKEN-STACK — in-box display driver + our miniport
Registry audit on .143: `Services\3dfxvs ImagePath = 3dfxv5m.sys` (OURS, loaded, desktop
1024x768x32@85 works on it) BUT `Device0 InstalledDisplayDrivers = "3dfxvs"` = the IN-BOX
XP 2001 3dfxvs.dll (689,216, 08/17/2001). Our 3dfxv5d.dll (595,644, Jul 17) sits in system32
UNUSED. Cause: active INF oem10.inf is the STOCK 3dfxvs2k.inf — its
[3dfxvs_SoftwareDeviceSettings] writes InstalledDisplayDrivers=3dfxvs on every PnP (re)install,
clobbering the WFP-rename plan. So every 'display driver deployed' conclusion before now ran
with the display DLL NEVER ACTIVE (consistent: no miniport LastMode reg values).
EXPLAINS: HWCEXT escape returning NULL dwordOffset (2001 display DLL doesn't speak our H5
escape protocol -> 0.1.0 dummyContextDWORD bandaid), scanout/mode weirdness, monitor garble,
and why glide3x video-register changes (postfilter) had no visible effect.
FIX: set InstalledDisplayDrivers=3dfxv5d (matched pair with our miniport) + reboot.
RISK: LOW-ish — miniport (the BSOD risk) is unchanged & already proven; a failing display DLL
=> VGA fallback, recoverable remotely by flipping the registry back (agent still runs).
ROLLBACK: REGWRITE InstalledDisplayDrivers back to 3dfxvs + reboot.

## ★★ ROOT-CAUSE #2 (2026-07-18): ICD never honored GL texture filters -> 0.2.0
The ICD called grTexFilterMode(GR_TMU0, POINT_SAMPLED, BILINEAR) ONCE at context
init (sst_export.c:794) and NEVER updated it from glTexParameter. Result: every
MINIFIED texture (minFilter != GL_LINEAR) was POINT-sampled in hw -> grainy
'software-renderer' shimmer on distant/minified surfaces + chunky scaled 2D text.
Also the init only set TMU0, but single-texture binds land on TMU1 (proven by the
FILT dump: tmu=1). FIX (0.2.0): __glSSTApplyGrFilter(tex) at all 7 grTexSource
bind sites, maps GL min/mag -> Glide POINT/BILINEAR, applied to BOTH TMUs.
Verified via instrumentation: menu font 256x32/128x16 now bind min=mag=BILINEAR.
Framebuffer CAPTURE of the menu is ~unchanged (2D glyphs draw ~1:1 texel:pixel so
point==bilinear there) -> the *capture* slicing is NOT a filter issue; but 3D
minified surfaces DO improve. Monitor verification pending (user).

## Efficient A/B loop (the process, now proven this session)
Per driver change (ICD, the SAFE lane): edit source -> build_ogl.bat (guards) ->
release/opengl.dll -> stage to scratchpad -> deploy (UPLOAD + copy to game dir +
system32, no reboot) -> capture menu+q3dm1 via +screenshot -> compare PNGs (+ zoom
crops) + run_bench for fps. Each build ~1-2 min. Instrument-first for any
'why' question (renderTriangle/DrawElements/filter/texsetup dumps via OGLLOG ->
C:dfxogl.log), then fix at the confirmed site, then revert instrumentation for
ship. Monitor-only bugs (scanout/dither) need the user; capture-visible bugs
(filtering, texcoord, color) are fully autonomous.

## Postfilter test (2026-07-18) — glide3x vidMaxRGBDelta has NO visible effect
Widened the video postfilter in glide3x MINIHWC (vidMaxRGBDelta 0x100810 -> 0x303030,
all 4 MINIHWC.C sites), rebuilt glide3x (build_glide_g6.bat, user-mode SAFE), deployed,
launched Q3 q3dm1. USER on monitor: **NO CHANGE AT ALL**. => the fullscreen scanout
filter is NOT controlled by glide3x's vidMaxRGBDelta here (minivdd/miniport likely owns
+resets it), OR the 'pixelated/software-like' look is not dither. glide3x build works:
env in build cmd; output H5/BIN/glide3x.dll (348160). Box backup: glide3x-backup-box.dll.
NEXT: get precise nature of the pixelation (grain vs blocky vs jagged vs upscaled) before
more builds; consider LCD-upscaling-of-640x480 and running at native res.

## Display / mode / garble facts
- After a Glide app exits abnormally (crash/taskkill) the board is left in 640×480
  Glide mode; forcing a GDI mode change (setmode) then garbles until GDI repaints.
  `setmode.exe <w h bpp [hz]` (C:\RETRO_AGENT\3dfx-driver\) restores the desktop;
  the bench harness runs it right after killing the game (kill_wait).
- grSstWinOpen(res=7=640×480, 60Hz) succeeds and renders; but the monitor may keep
  showing the desktop → the Glide surface isn't being scanned out (see reframe).
- 85Hz mode via grSstWinOpen destabilizes; triple-buffer (3,1) drops fps to 55 and
  goes erratic; grBufferSwap(0) breaks rendering. Double-buffer/60Hz is the only
  stable config. (These were fps experiments — see PROFILING-FINDINGS.md.)

## Performance (see PROFILING-FINDINGS.md for full data)
- Workload is PRESENT-bound: grBufferSwap ≈ 55% of frame, FIXED regardless of GPU
  load; T&L submission flush = 0.1%. ⇒ driver CPU micro-opts can't raise fps.
- Q3 ~77 / Q2 ~176 / CS ~68 fps @640, flat across resolution.
- High quality is FREE (picmip 0 + trilinear = same fps as default) — fill headroom.

## ICD font/text investigation (2026-07-18) — mostly MOOT given the reframe
Ruled out as the garble cause (all via glReadPixels capture, which we now know
shows the framebuffer not the monitor): aspect `default:return` abort (0 hits),
sub-256 sScale/tScale mismatch (real latent bug but menu font is 256-major),
filtering (GL_NEAREST identical), mipmaps, dither (already GR_DITHER_4x4).
Framebuffer capture shows readable-but-sliced proportional-font text; 3D + HUD
bigchars clean. If any real ICD font bug remains it is minor vs the scanout issue.

## Build / deploy / capture loop (all WORKING)
- ICD build (SAFE — bad ICD just fails to load, swap back): `cd toolchain-3dfx;
  WINEPREFIX=$PWD/prefix PATH=$PWD/wine/bin:$PATH COPYCMD=/Y; ulimit -f 2000000;
  timeout 560 wine cmd /c 'c:\3dfx\SWLIBS\OPENGL\GLIDE3X\build_ogl.bat'`; output =
  `release/opengl.dll` (rename → 3dfxogl.dll); `pkill -9 -x wineserver` after.
- Deploy ICD (no reboot): UPLOAD → copy to game dir\3dfxogl.dll + system32.
- Quality capture: Q3 `+wait 200 +screenshot +wait 60 +quit` → TGA in
  C:\q3home\baseq3\screenshots → DOWNLOAD. Menu scene = 2D font; +devmap q3dm1 = 3D.
  NOTE: 1024×768 capture came back green-garbled (separate readback issue); use 640.
- Display-driver build (RISKY — BSOD-at-boot → physical recovery; do SUPERVISED):
  W2K DDK via bldw2k.bat; produces 3dfxv5d.dll/3dfxv5m.sys (WFP-safe renamed).
- Good ICD = 0.1.3 md5 0d8c9a5ae79b700959e2e7b8795db234 (staged in session scratchpad).
- specpicks DSN now via scripts/specpicks_dsn.resolve_dsn (other session refactored).
