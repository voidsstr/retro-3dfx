# HANDOFF — 3DMark2001 / Direct3D stabilization on .143 (Voodoo5 5500)

**Written 2026-07-21 ~13:30 for session resume. Goal (user): make Direct3D games
work on these drivers — finish the 3DMark/D3D fixing.** OpenGL work is DONE and
must not be regressed (see `README.md` Change policy + fix ledger).

## Where things stand RIGHT NOW

**Box .143 state (clean, verified):**
- Display driver: **INSTRUMENTED build deployed + verified** —
  `C:\WINDOWS\system32\3dfxv5d.dll` = 942,668 B (Jul 21 12:46), built from the
  shared tree `Displays/H5` objfre with: `H3MakeRoom` 50M spin-breaker,
  CFIFO flight recorder, **registry-ring log sink** (RLog00..31 + RLogSeq under
  `HKLM\SYSTEM\CCS\Services\3dfxvs\Device0`), V5DLog lifecycle logging.
  `Retro3dfxLog=1` is SET. Ring is ALIVE (RLog00 = "DrvEnableSurface ENTER…").
  Backups: `system32\3dfxv5d-old.dll` + `C:\RETRO_AGENT\3dfx-driver\backup\3dfxv5d-pre-instr.dll`.
- Miniport: unchanged `3dfxv5m.sys` (198,544, Jul 18). `AFifo` ABSENT ⇒ CMD-FIFO
  in **video memory, not AGP** (important: VIA CPU-to-AGP controller on this
  Athlon board, AGP 2x — the AGP FIFO is the classic hard-lock vector, keep OFF).
- OpenGL ICD **0.3.7** everywhere (12 deploy sites incl. RTCW/MOHAA dirs) —
  golden gate passed ON the instrumented display driver: CS de_dust 0/49 green,
  Q3 real 1024×768 renders, Q2/UT fine. Baselines: Q3 640/800/1024 =
  72.8/73.0/71.7 fps (benchmarks/…0.3.7-realres.json).
- PowerStrip autostart REMOVED (parked at `HKLM\Software\RetroAgent\Parked`).
  DAEMON Tools has a **UT2004 ISO mounted as E:** whose autorun pops
  "Unreal Tournament 2004 Options" — unmount or ignore; swap to MOHAA CD1 image
  when doing the MOHAA lane.

## The D3D story so far (evidence)

1. Full-suite 3DMark2001 @640×480 **32-bit color + compressed textures** on the
   OLD (Jul 17, no-instrumentation) display driver: ran ~15 min → "Safety
   Precaution" abort once, then a **hard freeze** (no ping, physical reboot) on
   the second attempt.
2. After reboot, same config errors INSTANTLY and cleanly:
   `swapBuffer:Present : D3DERR_DRIVERINTERNALERROR` (3DMark error.log, saved at
   /tmp/overnight/3dmark_error.log). So the 32bpp D3D path fails at first
   Present now.
3. Source research (full report in the session transcript; key bits):
   - Compressed textures (DXT1-5/FXT1) are advertised **only when IS_NAPALM**
     (D3INIT.C:2158-2228) — a V3 never runs that path, the V5 does. 3DMark was
     set to "compressed" ⇒ **prime freeze suspect** (download/detile path
     DDSURF.C:1601-2020, size math D3TXTR.C:769-937).
   - 2-chip SLI FIFO stall loop (CFIFO.C:259-275); default
     `SSTH3_SLI_AA_CONFIGURATION=2` (2-way) on a 5500. Set `0` to force
     single-chip as an isolation lever (display Device0 REG_SZ).
   - 32bpp D3D path is Napalm-gated (ENABLE.C:1232/1283) — where the
     DRIVERINTERNALERROR now lives.
   - Knob catalog: `CmdfifoSize` (vidmem FIFO size, default 512K),
     `SSTH3_MAX_PENDING_BUFFERS`, `SSTH3_32BPP_RENDERING`, etc. — full table in
     the research report (session 5f44f51f transcript) + FINDINGS.md.

## EXACT next steps (in order)

1. **Run 3DMark at the V3-class config: 640×480 + 16-bit color + 16-bit Z +
   16-bit textures.** This isolates "is the D3D HAL fundamentally OK on V5 when
   the Napalm-only paths (32bpp, compressed) are avoided?"
   - 3DMark settings do NOT persist in registry/files — set via GUI each launch.
   - GUI automation coords (1024×768 desktop, window at default pos —
     VERIFY with a screenshot first, window pos shifts slightly!):
     main "Change..." (Display settings) ≈ (646,367); resolution combo ≈
     (556,318 arrow / 440,318 text); Texture Format combo ≈ (373,452);
     Z-Buffer combo ≈ (508,453); OK ≈ (356,569); "Benchmark" ≈ (575,517).
   - **Combo technique:** click the combo TEXT (not arrow), then `UIKEY home`
     selects the first item ("640x480 16 bit" for res; "16 bit" for
     texture/Z). My last attempt clicked while the Change dialog wasn't open
     and hit a Pro-nag — ALWAYS screenshot-verify the dialog is open before
     clicking combos, and screenshot-verify the summary line
     ("640x480, 16 bit color, 16 bit textures") before clicking Benchmark.
   - Monitor loop: reconnect every 45s (agent unreachable while fullscreen
     test runs = NORMAL); watch WINLIST for main window return (result) or
     modal (`error|safety|precaution|watson|trial`). Suite ≈ 13-18 min.
2. **If 16-bit completes** → capture score screenshot (UI capture is clean when
   3DMark exits exclusive mode), record via /tmp/dm_record.py pattern +
   benchmarks/ JSON. Then bisect UP: 16-bit + compressed textures (isolate
   compression); then 32-bit color (isolate 32bpp). One variable at a time; read
   the **registry ring** (`RLog*`) after ANY freeze/error — that's why the
   instrumented driver is there. `python3` decode: REG_BINARY values are
   UTF-16LE.
3. **If it freezes even at 16-bit** → after reboot read the ring (RLogSeq &
   RLog slots survive!). Expect `H3MakeRoom STALL>=100K` / `WEDGE-BREAK@50M`
   lines identifying the wedge. Then A/B: `SSTH3_SLI_AA_CONFIGURATION=0`
   (single-chip) in Device0, retest.
4. **32bpp DRIVERINTERNALERROR fix**: with the ring alive, reproduce the
   instant error at 32-bit color and read V5DLog/D3D lines to find the failing
   op; fix in the D3D HAL (Displays/H5, D3D-only path per Change policy),
   rebuild via `bldw2k.bat c:\3dfx\H5\W2K\Src\Video\Displays\H5` under Wine
   (COPYCMD=/Y, ulimit -f 2000000, purge stale .obj), redeploy via move-aside
   (loaded DLL: `move /Y` then `copy`), reboot (USER APPROVAL required).
5. **Then D3D GAMES**: after 3DMark is stable, validate a real D3D title.
6. **Queued after D3D** (user requests): Glide lanes (GLQuake staging —
   C:\Games\Quake1 has only DOS/WinQuake; Descent 3 `-timetest` with
   C:\Games\Descent3\demo\Secret2.dem; UT GlideDrv lane); fold ALL benchmark
   capability (MOHAA, Glide lanes, 3DMark, fbdump quality checks) into the
   **driver-bench skill**; update **deploy-3dfx-driver skill** for the full V5
   set (dist/3dfx-napalm-xp-20260716 has 0.3.7 ICD + updated DEPLOYMENT.txt) so
   another session can deploy to a second V5 box.

## Tools inventory (all working, on this dev host)
- `/tmp/post_instr_verify.py` — golden gate + ring check (run after any driver swap)
- `/tmp/gloop.py` (+ committed optimized/gltest/gloop.py) — deploy ICD everywhere + CS green check
- `/tmp/verify036.py`/`verify037.py` — Q3 1024/800 + CS gate template
- `/tmp/q3_rebaseline.py` — Q3 timedemo 640/800/1024 + benchmarks JSON
- `/tmp/dm_run.py`, `/tmp/dm_gui.py`, `/tmp/dm_record.py` — 3DMark automation
- `/tmp/glide_lanes.py`, `/tmp/glide_inventory.py` — Glide game lanes (phase 1)
- fbdump: `echo x > C:\icd_fbdump.on` → C:\fbdump_NN.raw (640-wide RGB565 rows,
  height=len/1280) — the ONLY truthful fullscreen capture.
- Ring read: `reg query .../Device0` RLog00..31 (REG_BINARY UTF-16LE), newest
  slot = (RLogSeq-1)&31.

## Hard-won rules (do not relearn)
- GDI SCREENSHOT garbles while any DDraw/Glide exclusive surface is up (incl.
  3DMark holding a leaked lock after a D3D error) — garble ≠ render bug.
- Fleet rule: REBOOT only with explicit user approval. Freeze recovery on .143
  requires physical power cycle (hard freezes kill the NIC).
- Never `git add -A` in retro-3dfx (shared tree with the .124/D3D session).
- 3DMark "Batch Run"/some UI = Pro-only nag; stick to the Benchmark button.
- The prefix/ build tree is gitignored — findings/docs carry the knowledge.
