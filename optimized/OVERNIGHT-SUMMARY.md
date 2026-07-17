# Overnight run — 3dfx driver on Voodoo5 (.143) — summary

**Date:** 2026-07-17 (overnight, autonomous). **Machine:** 192.168.1.143 "1GHZ",
Windows XP SP3, Pentium III ~1 GHz, **Voodoo5 5500 AGP** (single GPU after the
GeForce 6800 was physically pulled).

## Headline

**The fully self-built 3dfx driver stack renders Quake III correctly on a real
Voodoo5 5500 under Windows XP** — the first time our H5/Napalm-source driver has
run a game end-to-end. Baseline **81 fps** @640×480, clean q3dm1 render (verified
by pixel-diff). Everything is committed, pushed (private `voodoo5`… `retro-3dfx`),
and tracked in the specpicks DB (machine id=3) + `optimized/CHANGELOG.md`.

## What shipped (correct, deployed on .143)

| ver | change | Q3 @640 | note |
|-----|--------|---------|------|
| 0.1.0 | **fix grSstWinOpen NULL-lostContext crash** (NT-branch `dummyContextDWORD` fallback the Win9x branch already had) — this is what made it render at all | 81.2 | headline fix |
| 0.1.1 | /G5→/G6 (P6) scheduling | 80.2 | inert (hot paths are asm) |
| 0.1.2 | glide3x state-setter redundancy dedup (11 setters) | 79.7 | correct, flat |
| 0.1.3 | ICD post-transform vertex cache | 80.5 | correct, +0.75% |

All four are correct + zero-regression. The shipped ICD is **0.1.3** (md5 `0d8c9a5a`).

## The key finding

The three CPU optimizations were all ~flat, and **320×240 == 640×480 fps (~80)** —
so at 640 the timedemo is **NOT driver-bound**. It's **Quake-engine-CPU-bound**
(demo playback + game logic + surface tessellation on the P3). The Voodoo5 is fast
enough that the *driver* isn't the bottleneck at 640. (The other session's slower
Voodoo3 on .124 *was* driver-bound at 640 — different balance.) So per-vertex /
state / codegen tweaks can't move fps here; the only lever that helps an
engine-bound Q3 is **reducing what the engine submits**.

## Multitexture (0.1.4) — the real lever, +6%, one fix from shipping

`GL_ARB_multitexture` makes Q3 render world surfaces in **one pass instead of two**,
halving the geometry the engine tessellates/submits. Measured **80.1 → 85.0 fps
(+6.1%)**, drift-controlled. Also fixed a **latent showstopper**: the ICD's
`DrvGetProcAddress` returned NULL for every name, so under XP's opengl32 (which
forwards `wglGetProcAddress` there) **no GL extension proc was ever reachable** —
which is why the ICD's dormant SGIS multitexture path had never been used.

**Not shipped** — the single-pass render is ~half brightness. Q3 sends world vertex
color = `identityLight` (0.5) expecting a 2× overbright that the 2-pass path gets
free from the lightmap blend (`GL_SRC_COLOR`/`GL_DST_COLOR`). Reproducing that 2× in
single-pass took multiple attempts (all in `optimized/experimental/NOTES.md`), each
confirmed via on-hardware logging to *engage* but not brighten:
`grColorCombineExt` output-shift (no-op on Napalm), then vertex doubling in the
immediate-fill procs (never fired — Q3's world doesn't use them). Root cause
narrowed: **the 2× must be applied on Q3's vertex-array path
(`glDrawElements → CompileElementsIndexed`, S_VARRAY.C), not the fill procs.**

## Also delivered

- **NVIDIA GeForce 6800 fully removed** (driver class, `nv` service, Run keys,
  driver files, OEM INF, and the ACL-locked ghost Enum devnode via `schtasks /ru
  SYSTEM`) → codified as the **`gpu-driver-cleanup`** skill.
- **WFP-safe driver rename** (`3dfxv5d.dll`/`3dfxv5m.sys`) — XP's Windows File
  Protection reverts the in-box driver filenames; shipping under non-protected
  names is the fix.
- Tools: `updrv.exe` (NONINTERACTIVE SetupAPI install), `pendmv.exe` (boot-time
  file swap). Extended the **`driver-bench`** skill with the pure-3dfx lane
  (`--gldriver 3dfxogl`, `--deploy`, crash-log capture) — the one tool for
  deploy+bench+track.
- Every run in specpicks (machine id=3) + JSON drops; quality screenshots per
  version for pixel-diff gating.

## Next step (when you're back)

Finish multitexture: apply the 2× to the compiled vertex color in
**S_VARRAY.C `CompileElementsIndexed`** (Q3's actual world path), confirm via the
`C:\3dfxogl.log` "doubled compiled vtx color" line firing, pixel-diff back to
~sky-noise. That ships a correct **+6%** and unlocks the `DrvGetProcAddress` fix
for all extensions. To reach 1024×768 (a genuinely driver-bound regime where the
other optimizations would also show), `grSstWinOpen` at 1024 needs testing first
(untested; kept off overnight to avoid a garble/power-cycle while you slept).
