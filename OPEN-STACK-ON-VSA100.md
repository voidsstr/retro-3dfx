# The open stack on a Voodoo 5 5500 — first measurements (2026-08-14)

Task: finish what's missing in the clean-room/open driver stack (notably its
kernel display driver) and benchmark it on the **Voodoo 5 5500** at `.143`.

Result in one line: **the open user-mode stack does not work on VSA-100 yet, and
the failing layer is the open Glide, not the ICD.** The kernel driver was *not*
built — §3 explains why that turned out to be blocked on the same defect.

Method note: every candidate was staged **game-local under a distinct DLL name**
in `C:\Games\Quake2` and selected via Q2's `gl_driver` cvar. `system32` was never
written to, and all three system DLLs were hash-verified unchanged afterwards.
Identity comes from the game's own `GL_RENDERER`/`GL_VERSION` plus out-of-band
md5 of each file — deliberately *not* from the bench harness's `--gldriver` flag,
which asserts lineage rather than detecting it (see §5).

> ⚠️ **KERNEL-DRIVER CAVEAT (added 2026-08-14, see [`DEPLOY-143-20260814.md`](DEPLOY-143-20260814.md)).** Every number below was
> measured while `.143` ran the **AmigaMerlin 3.1-R11** kernel display driver
> (`Services\3dfxvs -> 3dfxv5r.sys`), not this repo's. `3dfxv5d.dll` is not
> referenced anywhere in that box's registry. The ICD/Glide comparisons stay valid
> — they varied only user-mode layers over a fixed kernel — but none of them is a
> measurement of this repo's display driver.

---

## 1. Benchmarks — Quake 2 timedemo, `demo1.dm2`, 640×480×16

| # | OpenGL ICD | Glide | fps | `GL_VERSION` | |
|---|---|---|---|---|---|
| A | **vintage `retro3dfx 0.4.0`** (704,512 B) | retail (344,064 B) | **159.5** <br>`159.4 / 157.1 / 159.0 / 162.6` | `1.1.0 3Dfx Beta 3.00` | ✅ baseline |
| B | **Mesa 6.3** community (2,646,009 B) | retail | **93.5** <br>`93.7 / 93.6 / 93.1` | `1.2 Mesa 6.3` | ✅ works, **−41 %** |
| C | **retro3dfx-gl 0.1.31** (MesaFX 6.2.2, 2,742,276 B) | retail | — | — | ❌ won't bind (§2) |
| C′ | retro3dfx-gl 0.1.31 | **open `glide3x_h5`** (920,669 B) | — | — | ❌ hangs at Glide init |
| D | vintage `retro3dfx 0.4.0` | **open `glide3x_h5`** | — | `Direct3D` / `1.1` | ❌ falls back to MS GL |

Baseline noise floor is **±1.7 %** (n=4), so the B result is ~25σ out — real, not
noise. A cold first-run outlier (118.3 fps) appeared before the box warmed; it did
not recur across nine subsequent runs. Baseline re-verified at 157.3 fps after
cleanup.

### The headline

**On VSA-100 our tuned vintage ICD is 71 % faster than the Mesa 6.3 ICD** (159.5 vs
93.5 fps). That is the *opposite* of the Voodoo 3 result, where MesaFX beat the
stock 3dfx ICD by +28 % in this same benchmark.

The two results are not in conflict — they have different baselines. On the V3 the
comparison was MesaFX vs **stock `3dfxgl`**. Here the baseline is
**`retro3dfx 0.4.0`**, this repo's *tuned* build of the 3dfx ICD. The V3 finding
says "MesaFX beats a stock 3dfx ICD"; this one says "our tuned vintage ICD beats
Mesa on VSA-100". Both can hold, and together they say the ICD work in this repo
has been worth more than the Mesa lineage on this silicon.

So **§5 of `DRIVER-STACK-ASSESSMENT.md` is now answered, and the answer is no**:
swapping in a Mesa ICD is not the free +28 % on a Voodoo 5 that it was on a
Voodoo 3. That recommendation is withdrawn.

---

## 2. Why the open stack fails on VSA-100

**C — an ABI mismatch, not a hardware problem.** `retro3dfx-gl` imports its 65
Glide entry points as **`grFoo@N`** (stdcall-decorated, *no* leading underscore).
The retail `glide3x.dll` on `.143` exports 111 symbols, but in the
**`_grFoo@N`** form — so **all 65 imports fail** and `ref_gl` reports
`could not load "retrogl"`, then silently falls back to the vintage ICD. (That
silent fallback is a trap: the run still produces a plausible fps number under the
*wrong* driver. The first C run measured 138 fps and was pure vintage ICD.)

The open `glide3x_h5` exports 392 symbols and satisfies all 65.

**C′ — the real failure.** With the open Glide staged, the ICD loads and Q2 gets
as far as:

```
Initializing OpenGL display
...setting mode 3: 640 480 FS
...attempting fullscreen
...using gl_bitdepth of 16
...calling CDS: ok
```

…and then stops. No fps, no `GL_RENDERER`, no Glide error log, no crash dialog,
no reboot. It dies **between the mode set and Glide bringing the board up**.

**D — the first isolating experiment.** Leaving the open Glide in place but
reverting to the *vintage* ICD also fails, falling through to Microsoft's
`Direct3D` GL. Since the vintage ICD is known-good with retail Glide (159.5 fps,
§1), **the open Glide is broken on VSA-100.**

> ⚠️ **I originally added "the ICD is exonerated" here. That was wrong** — see §7.
> Test D shows the open *Glide* is broken; it does **not** show the ICD is fine.
> Building the missing retail-linked ICD (§6) and testing it against known-good
> retail Glide showed the ICD fails too. **Both open layers are independently
> broken on VSA-100.**

This is consistent with `retro-agent/docs/3dfx-glide-hardware-init.md`: the open
Glide's NT hardware layer (`minihwc`) obtains its board mapping through the 3dfx
**reference** driver's `HWCEXT` escapes, which the retail/vintage `3dfxvs` display
driver does not implement the way it expects. That was solved for **Voodoo 3** on
`.124` (base-map ordering, TLS accessor, lost-context NULL fallback — fork
`a71eb3f`). **None of that work was ever done for VSA-100**, and this is the first
measurement showing it is in fact needed.

---

## 3. Why the kernel driver was not built

The open stack's missing layer is `fxd3ddd.dll` (milestone **M4d**). It was not
built, and the reason is stronger than "out of time":

1. **It is Voodoo 3 hardware by construction.** There is **no chip detection
   anywhere** in the open kernel driver — a single-chip Avenger is assumed.
   Textures are hard-capped at 256×256 (`driver/nt/enable.c:592`). `gbkernel.c:45`
   states outright: *"Clean-room: built only on the design doc, the open 3dfx-GPL
   glide **h3** tree … The proprietary H5 tree was not opened."* The 5500 is
   **2× VSA-100** and nothing in the driver knows what SLI is.
2. **It has never run on any hardware at all**, and its intended bring-up target
   no longer exists — the Voodoo 3 was pulled from `.124` on 2026-08-11.
3. **Its prerequisite is now empirically broken, not merely untested.** `gbkernel`
   is the kernel-mode sibling of exactly the Glide layer that §2 just measured
   failing on VSA-100. Porting it to VSA-100 means solving that same hardware-init
   gap *in kernel mode*, where a mistake is a bugcheck on the only live V5 rather
   than a game that won't start.
4. **Even finished it would be a downgrade.** It is a D3D6/7 HAL over an
   *unaccelerated* 2D framebuffer, no SLI, 256×256 textures — against a vintage
   driver that already does D3D6/7/8, 4-way SLI, T-buffer FSAA and 32-bit colour.

**Recommended order if this is still wanted:** fix the **open Glide on VSA-100
first**. It is user-mode, debuggable, reversible, and cannot bugcheck the box —
and it is a hard prerequisite for the kernel driver regardless. The concrete first
step is to instrument `minihwc`'s NT init on VSA-100 the way `.124`'s was for
Avenger, and find where the board mapping fails.

---

## 4. State of `.143` (and a records problem)

`.143` was **not** running what the records claim. Actual, by md5:

| system32 file | bytes | what it really is |
|---|---|---|
| `3dfxv5d.dll` | 962,724 | vintage H5 display driver |
| `3dfxv5m.sys` | 199,644 | vintage H5 miniport |
| `glide3x.dll` | 344,064 | **retail/AmigaMerlin** |
| `3dfxogl.dll` | 2,646,009 | **Mesa 6.3 community ICD** |
| `opengl32.dll` | 704,512 | vintage ICD `retro3dfx 0.4.0` |

So the box has been running a **mixed** stack, and the two ICDs on it are from
different lineages — `3dfxogl.dll` ≠ `opengl32.dll`.

**This is a rule violation, not a documentation error.** `CLAUDE.md:74` is
*prescriptive* — its table says what must be **on the box** ("`system32\opengl32.dll`
AND `3dfxogl.dll` (same file)"), under the hard rule that *every* V5 driver binary
must come from this repo. A Mesa 6.3 community build sitting at the `3dfxogl.dll`
path in `system32` on a Voodoo 5 is exactly the foreign-lineage binary that rule
exists to prevent. The doc is right; **`.143` is out of compliance**.

Whether to reconcile it is a judgement call worth making deliberately: that Mesa
ICD is *slower* (§1), so nothing is lost by replacing it with the vintage
`retro3dfx 0.4.0` — but it is also load-bearing for anything on the box that
selects `3dfxogl` explicitly, so it should be swapped knowingly rather than
silently. **Not changed here** — this session only ever wrote game-local files.

Left exactly as found: the open `glide3x.dll` I staged was deleted, `retrogl.dll`
and `mesa63.dll` were left in the Q2 directory as inert reference copies (nothing
loads them without an explicit `gl_driver`), and all three system32 hashes were
re-verified unchanged.

---

## 5. The benchmark records are not trustworthy

All 31 `.143` result files record `icd = "3dfxogl (H5-source OpenGL ICD)"`. That
field is **asserted, not detected** — `run_bench.py:867` overwrites the detected
ICD with a lineage inferred from the `--gldriver` flag:

```python
stack["icd"] = "3dfxogl (H5-source OpenGL ICD)" if args.gldriver == "3dfxogl" else stack["icd"]
```

Which is how a Mesa 6.3 build gets filed as the vintage H5 ICD. Tellingly the
harness reports `"unrecognized (689216 B)"` for the display driver and glide —
failing *honestly* — while asserting the ICD with false confidence. (Diagnosis by
the retro-agent DOS-lane session; the same rows are duplicated in
`retro-agent/benchmarks/`.)

**Consequence:** any comparison drawn from the historical `.143` ICD field is
unsafe. The numbers in §1 do not depend on it — they carry the game's own
`GL_RENDERER`/`GL_VERSION` and an out-of-band md5 per file.

**The `.124` Voodoo 3 figures, by contrast, are sound.** They were checked rather
than assumed: those runs recorded `GL_RENDERER: Mesa Glide v0.62 Voodoo3 (tm)
[retro3dfx 0.1.31]` (`FINDINGS.md:638, 1057, 1700, 2033`). A Mesa renderer string
is positive proof the Mesa ICD actually ran, because the silent-fallback failure
mode reports `3Dfx [retro3dfx 0.4.0]` instead. The +28 % Voodoo 3 result stands.

---

## 6. One cell of the matrix is still empty — and why

The untested combination is **`retro3dfx-gl` + retail Glide on VSA-100**. It
matters because it is the only configuration in which the open ICD could run on
`.143` *without* the broken open Glide of §2 — i.e. the one that might still work.

It could not be tested, for a mundane reason:

- `build-mesafx-retail.sh` exists precisely for this (it relinks MesaFX against
  `libglide3x_retail.dll.a` so the ICD imports `_grFoo@N`), and its own header
  documents the identical mismatch found in §2: *"Confirmed on .124: AmigaMerlin
  glide3x.dll exports `_grBufferSwap@4`; our default opengl32.dll imports
  `grBufferSwap@4` → mismatch → LoadLibrary fails."* So §2 is a **reproduction of
  a known defect on new silicon**, not a discovery.
- But **no retail-linked artifact exists on this host.** Both archived copies
  (`vcr-build/retro3dfx-gl/lib/opengl32.dll` and `voodoo-cleanroom/out/opengl32.dll`)
  are byte-identical — md5 `2a90bebeabee5578` — and both import the undecorated
  `grFoo@N`, i.e. both are open-linked.
- And it **cannot be built here**: this host has no C compiler at all. `gcc`,
  `cc`, `clang` and `i686-w64-mingw32-gcc` are all absent — the same gap that
  makes `tests/run_native.sh` fail. The retail import lib and the MesaFX source
  tree are both present, so this is purely a missing toolchain.

**UPDATE — the cell has now been filled.** A cross-toolchain *does* exist on this
host, at `/home/voidsstr/toolchain-mingw/usr/bin/` (found by the retro-agent
DOS-lane session). It needs two fixes to run: the binaries carry a `-win32`
suffix, so `build-mesafx-retail.sh`'s `command -v ${CROSS}gcc` check fails without
a symlink shim, and `cc1` needs `LD_LIBRARY_PATH=$TC/usr/lib/x86_64-linux-gnu`
(else `libisl.so.23: cannot open shared object file`). `make` is also there but not
on `PATH`. With a shim plus the source tree seeded from
`/home/voidsstr/vcr-build/retro3dfx-gl` (no clone needed), the retail-linked ICD
builds cleanly:

```
output: opengl32_retail.dll v0.1.2 (2,749,065 bytes)   md5 4888dac8d2066998
OK: imports _grFoo@N (binds retail/AmigaMerlin glide3x)
```

Independently verified: **all 65** Glide imports are `_grFoo@N`. Result of testing
it on `.143` is §7.

---

## 7. Correction — both open layers are broken, not just the Glide

The retail-linked ICD built in §6 was staged game-local as `retailgl.dll` and run
against `.143`'s **known-good retail Glide** — the exact configuration that works
at 159.5 fps under the vintage ICD and at 93.5 fps under Mesa 6.3.

**It failed at precisely the same point as the open-Glide run:**

```
...calling CDS: ok            <- and nothing further, 3 runs
```

No fps, no `GL_RENDERER`, no reboot. So `retro3dfx-gl` fails on VSA-100
**independent of which Glide it binds**.

Ruled out as explanations, by inspection rather than assumption:

- *Not* an ABI/link problem — this build imports the correct `_grFoo@N` and the
  retail Glide exports all 65.
- *Not* a missing-export problem in test D either: the open `glide3x_h5` exports
  **all 48** symbols the vintage ICD imports (0 missing).
- *Not* a missing dependency — `glide3x_h5` imports only stock XP DLLs
  (`KERNEL32`, `USER32`, `GDI32`, `ADVAPI32`, `ddraw`, `msvcrt`).

**Revised conclusion.** §2's claim that "the open Glide is the broken layer, not
the ICD" was half right and I have struck it above. The correct reading:

| Test | Shows |
|---|---|
| D — vintage ICD (known-good) + open Glide → fails | the **open Glide** is broken on VSA-100 |
| §7 — retail-linked `retro3dfx-gl` + retail Glide (known-good) → fails | the **open ICD** is broken on VSA-100 |

Each test pairs one suspect layer with a component *proven good on this hardware*,
so the two faults are **independent**. Fixing either one alone will not produce a
working open stack on a Voodoo 5 — a point that materially raises the cost
estimate in §3, and that only became visible because the missing build variant
was actually produced rather than reasoned about.
