# Which driver stack for the Voodoo 3 and Voodoo 5 — a grounded comparison

**Date:** 2026-08-14. **Question:** of the three stacks available to this fleet —
AmigaMerlin (retail/community binary), the **vintage H5 source** stack in *this*
repo, and the **clean-room/open** stack in `retro-agent` — which gives the best
quality and performance on the Voodoo 3 and the Voodoo 5?

Everything below is from source and from the recorded benchmark JSON in
`retro-agent/benchmarks/`, not from recollection. Where something is untested,
it says so.

---

## 1. Recommendation

| Card / OS | Kernel display driver + D3D | Glide2/Glide3 | OpenGL ICD |
|---|---|---|---|
| **Voodoo 5 (5500 / 6000), XP** | **vintage H5** (this repo) | **vintage H5** (this repo) | vintage H5 `3dfxogl` today — **try the open MesaFX ICD**, see §5 |
| **Voodoo 3, XP** | **vintage H5** (this repo) | **vintage H5** or retail | **open MesaFX ICD** (`retro3dfx-gl`) |
| **Either card, Win98** | **AmigaMerlin 2.9** | AmigaMerlin | AmigaMerlin / MesaFX |
| **Anything, "just make it work"** | **AmigaMerlin 2.5 SE** (`driver2K\`) | AmigaMerlin | stock |

The short version: **the vintage H5 stack is the only complete stack**, and on
OpenGL the **open MesaFX ICD is the single component worth swapping in**. Do
*not* use the clean-room Glide for performance — it is measurably slower than
the vintage/retail one. Do not expect the open stack's D3D driver to work at all
yet.

---

## 2. What each stack actually is

### Vintage H5 (this repo) — the only complete stack

It is a genuine **unified Voodoo 3 / Banshee / VSA-100 driver**, not a V5 driver
with a V3 INF stapled on. Both `Inf/Voodoo3/Voodoo3.inf` and
`Inf/Voodoo5/3DFXVS2K.INF` install the **identical** binaries under the same
`3dfxvs` service (`3dfxvsm.sys` + `3dfxvs.dll` + `glide2x/3x.dll` + `3dfxOGL.dll`),
and the split is real at runtime — `IS_VOODOO3` / `IS_NAPALM`
(`DDFXNT95.H:89-90`, `H3.H:319-320`, keyed on the PCI device ID) branch at:

| Tree | chip-branch sites |
|---|---|
| `W2K/.../Displays/H5` | 177 |
| `W2K/.../Miniport/H5` | 78 |
| `GLIDE3/SRC` | 70 |
| `GLIDE/SRC` | 44 |
| `MINIHWC` | 27 |

Covers Banshee (`DEV_0003`), Voodoo3 (`0005`), VSA-100 (`0009`/`000B`).
Layers: XPDM display driver, **D3D6/D3D7 HAL (working, proven)**, DirectDraw,
miniport with SLI/AA setup, Glide2, Glide3, and the vintage 3dfx OpenGL ICD.

### Open / clean-room (`retro-agent`) — two mature layers, one missing

| Layer | State |
|---|---|
| **OpenGL ICD** (`retro3dfx-gl`, MesaFX 6.2 fork, ~2.75 MB) | **Mature and fast.** Deployed and benchmarked extensively on Voodoo 3. |
| **Glide3** (fork of `sezero/glide`, 3dfx 2000 GPL) | Works on V3, but **78–94 % of retail speed** (§3). `glide3x_h5.dll` is built (920 KB) but **never validated on a V5**. |
| **Display driver + D3D HAL** (`fxd3ddd.dll`) | **Code-complete, links, never run on hardware.** Milestone M4d — "bringing it up on real Voodoo3 hardware, never yet done." |

So the open stack **cannot currently host a machine on its own** — no working
kernel display driver, no working D3D.

### AmigaMerlin — mature black box

Community-patched 3dfx reference drivers. Covers Banshee/V3/V4/V5.
**Version matters:** the packaged **2.9 is Win9x-only** (`am29win9x.exe`,
`supported_os: ["win9x"]`); the XP-capable one is **2.5 SE**, which carries
`driver2K\` and whose INF lists our exact hardware
(`3dfxvsV5,PCI\VEN_121A&DEV_0009&SUBSYS_0001121A`). Stable and complete, but
binary-only: when a game renders wrong, there is nothing to fix.

---

## 3. Performance evidence

### Voodoo 3 (`.124`, P3-845 MHz, XP, 16 bpp) — recorded runs

With the **open MesaFX ICD** in place, the kernel driver and glide3x choice are
**within noise** on OpenGL:

| Benchmark | AmigaMerlin retail kernel+glide | vintage H5 kernel+glide |
|---|---|---|
| Q3 `timedemo four` @640 | 58.9–59.1 | 59.1 |
| Q3 @1024 | 50.8 | **51.3** |
| Q2 @640 | 96.6 | **96.7** |
| Q2 @1024 | 47.1 | 47.1 |

**The ICD is the lever, not the kernel driver.** The one large, clean delta is
ICD-vs-ICD:

> **Q2 @640×480×16 — open MesaFX ICD 96.7 fps vs stock 3dfx `3dfxgl` 75.7 fps → +28 %.**

RtCW @1024 also moved 25.2 → 31.9 fps across the ICD version campaign (+27 %).

### The clean-room Glide is the one regression

Full-open stack (our glide3x + our ICD), fullscreen sweep 2026-07-22:

| | clean-room glide | retail/vintage glide |
|---|---|---|
| Q3 @640 | 46.0 | **58.8** |
| Q3 @800 | 44.3 | **58.4** |
| Q3 @1024 | 39.2 | **51.2** |
| Q2 @640 | 88.4 | **93.6** |

**78–94 % of retail.** Stable, but a real loss. Its value is *ownership* (it is
the fxD3D HAL's link target and a modifiable Glide), not speed.

### Voodoo 5 5500 (`.143`, Athlon 1 GHz) — vintage stack throughout

Q3 @640 **85.0**, @800 78.4, @1024 78.3 · Q2 @640 **183.3**, @1024 175.7 ·
CS 1.6 @640 68.5. **Every one of these ran on the vintage H5 ICD** — see §5.

### Voodoo 5 6000 (`.133`, 4× VSA-100)

Q3 **61.7 fps** at *both* 640 and 1024 (so refresh/CPU-bound, not fillrate) ·
UT99 Glide **39.8** · UT99 D3D ~24 · CS 1.6 D3D **33.5** · RtCW ~51 (via the
vintage `openglv5.dll` wrapper, not our ICD).

---

## 4. Quality

**Silicon first — these are hardware limits, not driver choices.** `IS_NAPALM`
gates them in the source:

| Feature | Voodoo 3 | Voodoo 5 (VSA-100) |
|---|---|---|
| 32-bit rendering | ✗ (16 bpp + 22-bit postfilter) | ✓ (`DDFXNT.C:2248`, `cjPelSize==4` requires `IS_NAPALM`) |
| T-buffer FSAA (2×/4× rotated grid) | ✗ | ✓ (`D6DP2.C:5080`, `D7DP2.C:809`, `DDFXNT.C:2655`) |
| SLI (multi-chip) | ✗ | ✓ |

So **on quality the Voodoo 5 wins on hardware regardless of driver** — 32-bit
colour and rotated-grid FSAA simply do not exist on Avenger. Any "which driver
looks better" question is secondary to that.

**Where the driver does decide quality: the OpenGL ICD.**

The vintage 3dfx ICD reports:

- `GL_VERSION = "1.1.0 3Dfx Beta 3.00"` (`GLCORE/S_CONTXT.C:396`) — **OpenGL 1.1**
- multitexture advertised only as the legacy **`GL_SGIS_multitexture`**, and only
  when `grNTexelFx == 2` (`SST/sst_export.c:782`, comment: *"XXXTaco This init is a hack"*)
- `GL_MAX_TEXTURE_SIZE` hard-wired to **512** (`__GL_SST_MAX_LOD 9`,
  `sst_machdep.h:18`) with **no chip branch** — so it under-reports the VSA-100

MesaFX 6.2 is an **OpenGL 1.3-class** implementation advertising `GL_ARB_multitexture`
and the ARB env-combine extensions. For anything post-Quake3 that probes for GL
1.2+/ARB extensions, that is the difference between the fast path, a slow fallback,
and refusing to start. This is a **compatibility** advantage, not just a cosmetic one.

Known open item: the vintage ICD **already ships the ARB entry points** and is one
advertised string short of `GL_ARB_multitexture`; a probe gave **+5.8 %** but
correctness was never validated.

---

## 5. The highest-value untested move

> **Every Voodoo 5 benchmark on record used the vintage H5 ICD**
> (`icd = "3dfxogl (H5-source OpenGL ICD)"` / `retro3dfx 0.3.x` in all 31 `.143`
> result files). **The open MesaFX ICD has never been run on a Voodoo 5.**

On the Voodoo 3 that same swap was worth **+28 % in Q2** and lifted the GL level
from 1.1 to 1.3. It is a **single user-mode DLL** — drop it in the game directory,
no kernel driver change, no reboot, trivially reversible. It is by a wide margin
the best ratio of expected gain to risk available right now, and it is the obvious
first experiment once `.133` is rebuilt.

Caveats to check on first run: the MesaFX ICD has never seen multi-chip SLI or the
T-buffer; the vintage ICD is the one that knows about `FX_GLIDE_NUM_CHIPS` and the
SLI/AA registry config. Validate rendering correctness (§the A/B screenshot harness,
`toolchain-3dfx/build/icd-ab-shot.py`) before trusting any fps figure.

---

## 6. What is NOT established

- **No head-to-head on identical hardware.** `.124` (V3) is a P3-845 and `.143`
  (V5) an Athlon 1 GHz — cross-card fps here is not a controlled comparison.
- **AmigaMerlin was never benchmarked as a complete stack.** Every "AmigaMerlin"
  row in the DB is a *hybrid* — AmigaMerlin kernel+glide under **our** ICD. Its
  own OpenGL path is unmeasured, so "AmigaMerlin is slower" is **not** supported.
- **The open Glide has never run on a VSA-100.** `glide3x_h5.dll` builds; that is all.
- **The open D3D driver has never run at all.**
- **Our own D3D8 build has never run** — it compiles (`c34d62f`), and it is the
  DP2 command-buffer path, so treat first deployment as BSOD-risk.
- Voodoo 3 figures predate its removal from `.124` on 2026-08-11 (that box is a
  GeForce2 GTS now), so the V3 is currently shelf hardware with no host.
