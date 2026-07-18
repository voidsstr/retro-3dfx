# Garbled text (Q3 menu / CS) — 2026-07-18 investigation

User report: menu/text looks garbled in Q3 **and** CS 1.6; CS also crashed.
Systemic across games ⇒ a driver issue, not per-game. Investigated with a
now-working glReadPixels capture pipeline (Q3 `+screenshot` → TGA → download).

## What was ruled OUT (with evidence)
- **Extreme-aspect / NPOT `default: return` abort** in `__glSSTTexImage2D`
  (SST_TEX.C ~3014): instrumented it — **0 hits** on the Q3 menu. Not the cause.
- **sScale/tScale wrong for sub-256 textures**: real latent bug (a 128×16 font
  strip gets sScale=256/tScale=32 = 2× the actual dims; proven by the 256×32 case
  yielding tScale=32 = actual). Built a `major/256` fix — but the **Q3 menu font
  is 256-major**, so the fix changed nothing there. Reverted (unverified for 3D,
  doesn't fix the target). Worth a supervised same-viewpoint A/B later — it may be
  a genuine correctness win for sub-256 world textures.
- **Filtering**: `r_textureMode GL_NEAREST` capture is pixel-identical to the
  default (bilinear) — not a filter/bleed issue.
- **Mipmapping**: GL_NEAREST (no mip) still shows it — not mip selection.

## What was ISOLATED
- **In-game 3D world = clean; in-game HUD text (bigchars, fixed-width) = clean;
  only the MENU proportional font (font1_prop) garbles.** Captures:
  `/tmp/fix_q3dm1.png` (3D + HUD clean), `/tmp/q3menu_0.1.3.png` (menu sliced).
- Pixel analysis of the menu text: **readable blocky letters, NOT a regular
  ordered-dither pattern and not severe texcoord corruption** — the framebuffer
  render is basically OK (rough, as expected for a proportional font at 640×480
  **16-bit**).

## Leading conclusion → the fix is the 22-bit POSTFILTER (supervised)
The framebuffer (what glReadPixels captures) is readable; the user sees worse
**on the monitor** (matches the earlier hard-lesson: "capture crisp, monitor
garbled"). That discrepancy means the extra garble is added **after** the
framebuffer, at **scanout** — i.e. the VSA-100 **22-bit postfilter** that smooths
16-bit dithered output (esp. dark-red alpha-blended text on black, which dithers
heavily) is likely **off**. Enabling it would clean the monitor text with no
framebuffer change (which is exactly why a capture can't verify it).

**Why this is SUPERVISED, not overnight-autonomous:**
- The postfilter is NOT registry-controllable (miniport reads only GammaTable,
  dramInit, clocking… — no filter knob), so it needs a **display-driver build**
  (`3dfxv5d.dll`/`3dfxv5m.sys`).
- A bad display driver can **BSOD at boot** → agent never starts → physical
  access required. And the effect **can't be verified via glReadPixels**.
- So it must be done with the user watching the monitor and able to recover.

Register lead: `vidProcCfg` / FBIINIT in `H5/W2K/Src/Video/Displays/H5/DDINIT.C`
+ `Miniport/H5/H3.H` (`SST_OVERLAY_FILTER_*` is the *video-overlay* filter, not
the 3D postfilter — find the 3D framebuffer post-filter/dither bits).

## Also confirmed
- CS 1.6 crash: closed it; box restored to desktop + clean 0.1.3 ICD.
- The ICD build/deploy/capture loop is fast and safe (bad ICD just fails to load).
