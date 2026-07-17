# RDTSC frame profiling — the real bottleneck (2026-07-17)

We built the first-ever runtime profiler into the ICD and it overturned the
working assumption. The `3dfx-driver-optimized` lane was believed **engine-CPU-
bound** (Q3/Q2/CS all flat across 320/640/1024). Profiling proves it is actually
**present/vblank-latency bound**.

## The instrumentation
`SST/sst_export.c :: SwapBuffers()` — logging-only, `C:\3dfxprof.log`, every 100
frames. RDTSC (via `_emit 0F 31` for MSVC6) brackets three spans:
- `flush` = `gc->procs.flush(gc)` (deferred Glide FIFO / T&L submission)
- `swap`  = `grBufferSwap(1)` (buffer flip / present)
- `frame` = swap-to-swap wall time
Reported as Kcycles + each span's % of frame. Zero render change.

## The data (Q3 `four` demo, 640x480, 2-pass)
| scene | fps | swapKc | swap% | flush% | frameKc |
|-------|-----|--------|-------|--------|---------|
| default GPU load  | 76.5 | 2594 | **55.3%** | 0.1% | 4689 |
| minimal GPU load* | 80.6 | 2589 | **55.2%** | 0.1% | 4687 |

*minimal = `r_picmip 10 r_dynamiclight 0 r_fastsky 1 r_detailtextures 0 r_lodbias 4`

**Reads:**
1. **`grBufferSwap` is ~55% of every frame** and **`flush` (T&L submission) is
   0.1%** — submission/T&L is NOT the cost.
2. The swap cost is **fixed (~2590 Kcyc) regardless of GPU load** — gutting
   textures/lights/detail changed swap by <0.2% and fps by only +5%. So it is
   **not fillrate/GPU-drain** — it is a fixed per-frame **present/vblank/SLI-flip
   latency**.
3. Therefore the driver micro-opts in `OPTIMIZATION-QUEUE.md` (items 1-11: codegen
   flags, state-dedup, color LUT, vertex-dedup, tri-batch, MMX FIFO …) target the
   ~45% "other" and the 0.1% flush — they **cannot move an fps ceiling set by the
   55% present latency.** This explains why 0.1.1-0.1.3 were all flat.

## Experiments against the present latency
- **`grBufferSwap(0)`** (no vretrace wait): **breaks rendering** — no completed
  timedemo, transient agent drop. The Voodoo5 dual-VSA-100 SLI needs the hard
  swap sync. NOT viable.
- **`GR_REFRESH_60Hz -> 85Hz`**: swap% collapsed to **~0%** (CONFIRMS the swap was
  a vblank wait) — but frame timing went erratic (8k-55k Kcyc) and Q3 didn't
  finish: 85Hz destabilizes the mode on this display. NOT viable as-is.
- **Triple buffering** (`grSstWinOpen` nColBuffers `2 -> 3`, stay 60Hz): swap%
  collapsed to ~0 (flip no longer blocks) but fps **dropped to 55.8** and frame
  time went erratic (15k-73k Kcyc). WORSE. NOT viable.

## Revised conclusion (after swap0 / 85Hz / triple all failed)
Removing the swap-block made things worse or broke them in **every** case. So
`grBufferSwap(1)` is **not idle vblank-waiting** — it is doing **necessary work**
(GPU render-completion + the dual-VSA-100 SLI flip/sync), and **double-buffer +
60Hz + vsync is already the optimal present config.** The ~55% is real,
required, resolution-independent hardware present cost; the ~45% "other" is the
engine + inline GL. The driver's own T&L/submission CPU is ~0.1% (the flush).

**Bottom line for the campaign:** the ~77 fps ceiling on this Voodoo5 5500 +
P3-1GHz is **hardware-present / engine bound, not driver-CPU bound.** No amount of
driver CPU micro-optimization (queue items 1-11) will raise it — profiling proves
they target <1% of the frame. The ONLY driver-side fps lever is **reducing GPU
work submitted** = multitexture single-pass (fewer passes/triangles → less GPU
per frame); that matches the one real gain we ever measured (0.1.4 = +6%). Beyond
that, **quality** (menu font, texture filtering, LOD/gamma, dithering) is the
higher-value direction — it's orthogonal to the present-bound fps ceiling and is
half of the stated goal ("framerate up AND quality up").

## Strategic consequence
The optimization target is **not** the driver's T&L/submission CPU (that's ~0.1%
of frame at the flush, and the "other" 45% is mostly the engine we can't touch).
It is the **present path**: decouple the CPU from the vblank flip (triple buffer),
or find a stable higher refresh, or overlap CPU work with the GPU-present window.
Multitexture single-pass still helps indirectly only if it shortens GPU work
*inside* the swap window — but the load-insensitivity above suggests that window
is vblank, not GPU-drain, so even multitexture's ceiling here is small.
Quality-side wins (menu font, LOD/gamma) are orthogonal and still worth doing.

## Quality corollary — CONFIRMED: high quality is FREE on the Voodoo5
Because the frame is present-bound (not fill-bound), the card has fill headroom.
Quality-sweep on 0.1.3 (Q3 four @640, tracked in specpicks):
| quality | cvars | fps |
|---------|-------|-----|
| default | picmip 1, GL_LINEAR_MIPMAP_NEAREST (bilinear) | 77.0 / 76.8 |
| **high**| **picmip 0, GL_LINEAR_MIPMAP_LINEAR (trilinear), full detail** | **77.1 / 76.9 (FREE)** |
| max     | + subdivisions 2, lodbias -0.5, detail | 73.1 / 72.6 (-5%) |

Full-detail textures + trilinear filtering cost **0 fps**. So the "quality up" half
of the goal is largely free here. Next quality levers (driver-side, all-app):
1. **Force trilinear** (GL_LINEAR_MIPMAP_LINEAR) in the ICD texture-filter path
   even when the app asks bilinear — free quality.
2. **Voodoo5 22-bit postfilter** — the VSA-100 post-dither filter that lifts 16-bit
   output toward 22-bit (the card's signature IQ feature). Controls are in the
   **W2K display driver / miniport** (`vidProcCfg`/FBIINIT regs, H3.H — note
   `SST_OVERLAY_FILTER_*` is the video-OVERLAY filter, not the 3D postfilter).
   Big free quality win IF currently off — but it needs a **display-driver build**
   (3dfxv5d.dll/3dfxv5m.sys), which is black-screen / physical-recovery risk on a
   remote box: do it SUPERVISED, not unsupervised. (The ICD builds are safe by
   contrast — a bad ICD just fails to load and we swap it back.)
3. Menu proportional-font garble (still open); LOD/gamma tuning.
Ship recommendation for Voodoo5 boxes: run games at picmip 0 + trilinear.

Profiler stays in the tree (gated off in shipped builds via a compile flag once
we settle the present fix). Re-run any time by deploying a profiling build and
reading `C:\3dfxprof.log`.
