# Driver-stack diagnostics — every log channel, gate, and where it lands

All hot-path logs use the same **crash-robust pattern**: open → append →
flush → close per line (raw Win32, no CRT), so the last line survives an
instant process death or hard hang. All gates are **file markers** (create
the file to enable) — env vars proved unreliable: getenv works under
gfix.exe but returns nothing under GoldSrc's process, so anything gated
only by env silently never fires in Half-Life/CS.

## OpenGL ICD (3dfxogl.dll / game-local opengl32.dll)

| Channel | Gate | Output | Contents |
|---|---|---|---|
| OGLLOG (lifecycle) | always on | `C:\3dfxogl.log` | Drv* entry points, wglCreateContext + pixelFormat, MakeCurrent (incl. `REUSE Glide ctx` vs `grSstWinOpen`), grSstWinOpen/Close results, wglSwapBuffers exit-path one-shots (WindowFromDC NULL / FindWGLWindow NULL / single-buffer / first OK swap), error paths. Cheap: only lifecycle + first-occurrence lines. |
| OGLLOGV (verbose) | `C:\icd_verbose.on` | `C:\3dfxogl.log` | High-frequency traces (per-bind FILT@ filter changes, etc). ~500 lines/s — never leave enabled for benchmarks. |
| perflog | `C:\icd_perf.on` | `C:\icd_perf.log` | Per-100-frame: `dt`, `fps10` (fps×10), `db` (double-buffered), `texDl` (full texture downloads), `texDlPart` (subimage/lightmap updates), `tableDl` (palette downloads), `alloc` (texture-memory allocations). The thrash/starvation profile. |
| RDTSC profiler | `C:\icd_prof.on` | `C:\3dfxprof.log` | Per-100-frame flush/swap/other %% split (where frame time goes: T&L submit vs present vs engine). |
| tri-trace | `C:\icd_trace` | `C:\icd_tri.log` | First 400 triangles' final GrVertex data (x, y, sow, tow ×1000) at the grDrawTriangle boundary. For geometry/texcoord bugs. |

## FBDUMP — self-service "what is actually on screen" (no human needed)

| Channel | Gate | Output | Contents |
|---|---|---|---|
| FBDUMP | `C:\icd_fbdump.on` | `C:\fbdump_NN.raw` (+ `FBDUMP n -> WxH` line in 3dfxogl.log) | The REAL hardware front buffer, read at swap time via `grLfbReadRegion` in 16-line strips, raw RGB565. Frame 5 then every 100th swap, 10 dumps max per process. |

This is the ground truth for rendered output. GDI `SCREENSHOT` cannot see a
fullscreen Glide surface (interlaced garbage), in-game screenshots need game
cooperation, and phone photos of the monitor need a human. FBDUMP needs none
of those: drop the marker, run the game, pull `C:\fbdump_*.raw`, convert
565→PNG locally (`(v>>11)&31 → R`, `(v>>5)&63 → G`, `v&31 → B`), and inspect
pixels digitally — diff against expected values, sample regions, measure.

Standard loop for any "looks wrong on screen" report:
1. `echo x > C:\icd_fbdump.on`, delete old `C:\fbdump_*.raw`.
2. Run the game ~2 min (the every-100th cadence spans menu into gameplay).
3. `DOWNLOAD` the dumps, convert, view — the LAST dumps are deepest in-game.
4. Delete the marker (dumps cost a 600KB LFB read each — never leave on for
   benchmarks).
Implementation: `__r3dFbDump` in `SST/sst_export.c`, called right after
`grBufferSwap`.

## glide3x.dll

| Channel | Gate | Output | Contents |
|---|---|---|---|
| `_gsstLog` (GSST.C) | always on | `C:\glide3x.log` | grSstWinOpen path tracing (the historical V5/XP open-crash hunt), context lifecycle. Same crash-robust pattern. |

## Display driver (3dfxv5d.dll) + miniport (3dfxv5m.sys)

Kernel side — file logging is not possible from these layers. Current
state and plan:
- Display driver has the **escape channel** (used by disp_hw.c BAR mapper
  tooling); the planned diagnosability upgrade is an **in-memory ring
  buffer** in the display driver, dumped on demand through the escape by a
  small usermode tool (survives app crashes; lost on BSOD).
- Miniport uses `VideoDebugPrint` (visible only under a kernel debugger).
  For BSOD-class failures: `C:\WINDOWS\Minidump\*.dmp` + the agent's
  `SYSFIX`/event-log pull are the post-mortem path.

## Post-crash collection (one command)

The driver-bench skill's `capture_crash_logs` pulls `C:\3dfxogl.log` +
`C:\glide3x.log` automatically on a no-fps run. Manual sweep after any
incident — grab all of:
`C:\3dfxogl.log  C:\glide3x.log  C:\icd_perf.log  C:\3dfxprof.log  C:\icd_tri.log`
plus the game's own console log (`qconsole.log` / GoldSrc `-condebug`).

## Worked examples (this campaign)

- **Text garble**: tri-trace proved vertices/texcoords correct → gfix texel
  ruler + GDI oracle → 16-byte heap alignment (0.3.1).
- **CS green world**: gfix palette probe decoded the exact stride-3 table
  read (0.3.2).
- **CS low fps**: OGLLOG showed MakeCurrent→grSstWinOpen churn (70–600 ms
  per context switch); after the 0.3.3 reuse fix, perflog measured a steady
  99.9 fps with zero texture churn.
