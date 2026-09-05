# Voodoo 5 6000 — Quake III benchmark reference

The standing table of every Quake III number measured on the Voodoo 5 6000, and
of every published number worth comparing them against. `FINDINGS.md` is a
~9,500-line running log; benchmark results kept only there are unfindable a week
later. **This file is the reference; FINDINGS.md remains the narrative.** Each
section names the FINDINGS entry it distils.

Everything in §1-§3 is **VERIFIED ON HARDWARE** on box `.191` unless a row says
otherwise. §4 is the published literature, which is *evidence*, not measurement —
read §5 before comparing a number here to a number there.

---

## 0. Standing test conditions

All of our own runs share one configuration. Quote it whenever you quote a
number from this file.

| | |
|---|---|
| Box | **`.191`** `NSC-AF6CF7A80BC` — AMD Athlon **1152 MHz** (family 6 model 10), 511 MB, nVidia **nForce2**, Windows XP SP3 |
| Card | "Strange God" V5 6000 recreation: 4x VSA-100 @ **166 MHz** behind a HiNT HB1-SE66 bridge, **128 MB VBIOS mode = 32 MB/chip**, AGP |
| HWID | `PCI\VEN_121A&DEV_0009&SUBSYS_0001121A&REV_01` |
| Game | Quake III 1.32c, `demo four`, `r_fullscreen 1`, `r_colorbits 16` (**16-bit**) |
| Vsync | **OFF** — `FX_GLIDE_SWAPINTERVAL=0` **and** `r_swapInterval 0`. Both: the Glide env var alone does not reach the engine's own swap logic |
| Refresh | `FX_GLIDE_REFRESH=60`, `r_displayRefresh 60` |
| Chip count | `SSTH3_SLI_AA_CONFIGURATION` written as **`REG_SZ`** into *both* `...\3dfxvs\Device0\Glide` and `...\Device0\D3D`: **`5` = 4-way SLI, `0` = single chip**. There is no 2-way on this board (`GPCI.C:1446` has no `case 2`) |
| Harness | `tools/v56k/trials/ambench.py <cfg> "<tag>" <r_mode,list>` |
| `r_mode` map | 2=512x384 3=640x480 4=800x600 5=960x720 6=1024x768 7=1152x864 8=1280x1024 9=1600x1200 |
| Metric | timedemo **completion + the fps line** from `qconsole.log`. A driver that wedges the card never prints one, so this is also the health check — no eyes needed |

**Run-to-run noise is about 3%.** The same AmigaMerlin 640x480 4-way point read
**151.5** on the first pass and **152.6** in the eight-point sweep — 0.7% apart.
Do not read a 2-3% difference between two runs as a change.

---

## 1. The reference line — AmigaMerlin 3.1-R11, 4-way SLI

**VERIFIED ON HARDWARE.** Eight-point sweep, conditions as §0.
See FINDINGS.md, 2026-09-04 entry *"the V5 6000 is CPU-bound to 1024x768, and
the fill wall is gradual"*.

AmigaMerlin 3.1-R11 is a third-party **retail-lineage** driver, not ours. It is
here because it renders 4-way SLI correctly on this board, which makes it the
only valid control for our own stack.

| resolution | Mpx | fps | ms/frame | marginal ms/Mpx |
|---|--:|--:|--:|--:|
| 512x384   | 0.197 | 147.6 | 6.775 | - |
| 640x480   | 0.307 | **152.6** | 6.553 | - |
| 800x600   | 0.480 | 150.0 | 6.667 | 0.66 |
| 960x720   | 0.691 | 147.9 | 6.761 | 0.45 |
| 1024x768  | 0.786 | 147.1 | 6.798 | 0.39 |
| 1152x864  | 0.995 | 138.7 | 7.210 | 1.97 |
| 1280x1024 | 1.311 | 119.4 | 8.375 | 3.70 |
| 1600x1200 | 1.920 | 71.9  | 13.908 | 9.08 |

**512x384 is SLOWER than 640x480.** That is the tell: below 1 Mpx nothing here
measures the card at all. The five points from 512 to 1024 span 147-153 fps,
which is noise on a flat line — treat them as ONE number.

Fitting 640-1024 gives **`t = 6.41 ms + 0.50 ms/Mpx`**, i.e. a **CPU ceiling of
~156 fps** on this Athlon 1152 / nForce2. A 2.6x increase in pixels costs 3.6%
of the frame rate.

**There is no single knee.** Marginal cost climbs monotonically
(0.39 -> 1.97 -> 3.70 -> 9.08 ms/Mpx) and the departure from flat begins at
**1152x864**. An earlier four-point sweep (640/800/1024/1280) made 1280x1024
look like a threshold and suggested the databook's "1280 needs 2x mode" scanout
note; adding 1152x864 and 1600x1200 killed that reading. A hard mode threshold
would show one step and then a constant slope. A rising slope is saturation.

> **Practical consequence: any Q3 number on this card at or below 1024x768 is a
> CPU benchmark, not a card benchmark.** Comparisons against published figures
> are only meaningful at 1280x1024 and 1600x1200 — which is exactly where the
> published record is thinnest. Bench the 6000 at 1600x1200, or measure nothing.

---

## 2. Single chip — the fill-rate calibration line, and SLI scaling

**VERIFIED ON HARDWARE.** Same driver, same conditions, `ambench.py 0`.
See FINDINGS.md, 2026-09-04 entry *"the 6000 benchmarks EXACTLY on published
data: 2.66x SLI at 1024x768"*.

| resolution | Mpx | 1 chip | ms/frame | marginal ms/Mpx | 4-way SLI | scaling |
|---|--:|--:|--:|--:|--:|--:|
| 640x480   | 0.307 | 116.5 |  8.584 | - | 152.6 | 1.31x |
| 1024x768  | 0.786 |  55.2 | 18.116 | **19.9** | 147.1 | **2.66x** |
| 1280x1024 | 1.311 |  35.1 | 28.490 | **19.8** | 119.4 | **3.40x** |
| 1600x1200 | 1.920 |  24.7 | 40.486 | **19.7** |  71.9 | 2.91x |

### The single-chip line is perfectly straight

**19.9 / 19.8 / 19.7 ms per megapixel** across three independent intervals,
agreeing to 1%. One VSA-100 on this board is **fill-rate-bound at every
resolution including 640x480**, and nothing else is interfering. This is the
cleanest calibration of the board we have, and the reference line against which
any future driver regression should be measured.

### The one anomaly worth chasing — UNEXPLAINED

**SLI scaling peaks at 1280x1024 (3.40x) and falls back to 2.91x at 1600x1200.**
Single-chip stays perfectly linear there (19.7 ms/Mpx), so the loss is in the
4-way path specifically, not in the chip.

It is **not VRAM**: in 128 MB mode each chip holds only its own bands, so
1600x1200x16 front+back+depth is ~2.9 MB of the 32 MB per chip. Same direction
as the rising marginal cost in §1. A real 4-way-only ceiling above 1.3 Mpx.

### Why 640x480 scaling is 1.31x and that is CORRECT

Three independent proofs that 640x480 is a CPU wall, not a fill wall:

1. **thedodgegarage** (Celeron 1000, 16-bit, 640x480): V4 4500 (1 chip) **83**,
   V5 5500 (2 chips) **81**, V5 6000 (4 chips) **82**. Scaling is zero — very
   slightly negative. Their author: *"the 1000 mhz Celeron isn't feeding the
   Quake 3 game data to the 3dfx video cards fast enough."*
2. **AnandTech** (Athlon 750, 16-bit, 640x480): V5 5500 (2 chips) 83.0 and
   V4 4500 (1 chip) 83.0 — **exactly 1.00x**.
3. **The fill-rate proof.** x86-secret enabled 2x FSAA at 640x480, doubling
   sampled fill work: the 1-chip 4500 lost **49%**, the 2-chip 5500 lost **31%**,
   the 4-chip 6000 lost **3%** (148.2 -> 143.8). A card that barely notices a
   doubling of fill work is not fill-bound. Spec agrees: 4 x 333 Mpixel/s =
   1.33 Gpixel/s, and 640x480 at 151 fps with 3x overdraw needs 10-20% of it.

x86-secret measured **1.34x** at 640x480 on an Athlon XP 2800+; we measure
**1.31x** on an Athlon 1152. Within 3%. The "only 1.30x" reported earlier in
the session was never a defect — it is the correct answer to a question asked at
the wrong resolution.

---

## 3. Our driver (vintage H5/Napalm + SGL ICD)

**VERIFIED ON HARDWARE**, box `.191`, conditions as §0, our stack
(`3dfxv5d.dll` / `3dfxv5m.sys` + SGL ICD 0.5.0).
See FINDINGS.md, 2026-09-04 entries *"AmigaMerlin RUNS 4-WAY SLI CORRECTLY ON
THIS BOARD. The bug is ours."* and *"OUR DRIVER IS 21% SLOWER THAN AMIGAMERLIN
ON ONE CHIP, AND THAT IS A SECOND BUG"*.

| run | AmigaMerlin 3.1-R11 | ours | gap |
|---|--:|--:|--:|
| 640x480 single chip | 116.5 | **92.5** | **-21%** |
| 640x480 4-way SLI | 151.5 | **105.0** | **-31%** |
| 640x480 SLI scaling | 1.30x | **1.14x** | — |

**This is the PAIRED run**: both drivers measured back to back on the same box
in the same hour, which is what makes the gap a driver measurement. The
eight-point sweep in §1 read the same AmigaMerlin 4-way point at **152.6**
(0.7% apart, inside the ~3% noise, §0). Quote the paired pair for the driver
delta and the sweep for the resolution curve — never mix the two in one row.

**There is no verified 1024x768 figure for our stack — at any chip count.** Our
stack has been measured on this box at 640x480 only. Producing a 1024x768 pair
against AmigaMerlin's 147.1 — the resolution the entire published record uses,
and where §2 puts SLI scaling at 2.66x — is **the single highest-information run
still outstanding.**

### The single-chip number is the important one

Every variable except the driver is pinned — same box, card, game, demo,
resolution, colour depth — and we are **21% down before any second chip is
involved**. That cannot be an SLI bug, a scanout bug, or a hardware excuse. The
deficit being roughly equal in both configurations reads as **per-chip
submission-path cost**, which points at the open FIFO-wedge / submission-pacing
items (`V56K-SLI-FINDINGS.md` §21-22) rather than at anything new.

Caveat held honestly: if the multi-chip scanout defect carries a performance
cost of its own, part of the 4-way half of the gap may be a symptom of it rather
than an independent throughput problem. **The single-chip 21% is clean.**

### The deficit is recoverable — the host is not the limit

AnandTech (Oct 2000) published a Q3 640x480x16 CPU ladder on a GeForce2 GTS:
Athlon 800 = 128, 900 = 137, **1.0 GHz = 144, 1.2 GHz = 162** ("a 1% performance
increase per MHz"). The control sits on the same chart — 1024x768x32 is dead
flat at 83 fps for every CPU from 800 MHz to 1.2 GHz, so the ladder really is
measuring the CPU. Interpolating to our 1152 MHz Athlon gives **~158 fps**;
`demo four` plus XP probably pulls the real figure to 145-155, consistent with
the ~156 fps fit in §1.

**AmigaMerlin's 151.5-152.6 is ~96% of that ceiling. Ours at 105.0 is 66% of
it.**
AmigaMerlin is at the wall and cannot go faster on this host; our driver is
nowhere near it. So the gap is entirely recoverable in software, and our
640x480 result is a legitimate *driver* measurement rather than a host
measurement.

### OPEN MEASUREMENT — the free experiment the literature hands us

**HYPOTHESIS, not yet run.** VoodooExtreme's Dec-2000 review is the only
published test of our driver's actual code lineage (3dfx's own V5-6000 reference
driver). At 1024x768 it reports **88.4 fps at 16-bit vs 86.1 at 32-bit — a
1.03x gain for halving the colour depth** — which the reviewer explicitly calls
a driver bug (*"performance in 16-bit color was a bit lackluster... most likely
a driver issue"*) and says the 5500 shows too. Community drivers on the same
silicon get a proper **1.20x** (VoodooAlert, 155.1 vs 128.7).

So: **run our driver at 640x480 in 16-bit and again in 32-bit.**

- ratio comes back near **1.0x** -> we inherited a documented 3dfx defect and
  have a named target.
- ratio comes back near **1.2x** -> the 21% is ours and lives elsewhere.

Pure fps, no visual verdict, fully automated. It needs our stack reinstalled on
`.191`.

---

## 4. The published record

Context, not measurement. Every caveat in §5 applies to every row.

### x86-secret 2005 — the only tests of a real 6000 prototype

Athlon XP 2800+, **32-bit, max detail**. 4-way.

| resolution | 6000 fps | note |
|---|--:|---|
| 640x480   | **148.2** | 1.34x vs V4 4500's 110.2; 2x FSAA costs only 3% (143.8) |
| 800x600   | **141.8** | |
| 1024x768  | **130.3** | **2.66x** vs V4 4500 |
| 1280x1024 |  **92.1** | |

x86-secret states the CPU bound outright for 640x480 — the 5500 and the 6000
come out level, *"was hier auf eine Limitierung seitens der CPU schliessen
laesst"* — **on an Athlon XP 2800+**. If a 2.1 GHz Barton is CPU-bound at
640x480, a 1.15 GHz Athlon certainly is.

### GamersNexus 2023 — the same board MODEL as ours

AMD Athlon XP 2600+, **32-bit, trilinear, max detail**, **1024x768**.

| driver | 4-way | single chip | scaling | provenance |
|---|--:|--:|--:|---|
| VRG 1.05.04 | **110.6** | **32.0** | **3.46x** | read off the article's chart image; the pair reproduces the 3.46x carried in FINDINGS |
| AmigaMerlin 2.9 | 113.4 | — | — | research sweep, chart-read, **not independently verified** |
| Raziel64 | 111.0 | — | — | research sweep, chart-read, **not independently verified** |

Only the 3.46x ratio is corroborated anywhere else in this repo. Treat the three
absolute figures as chart reads, and see §5 on why only within-article ratios
travel.

Their 3.46x reads higher than our 2.66x at the same resolution because their
heavier settings push 1024x768 further into fill-bound territory. **Our
equivalent point is 1280x1024, where we measure 3.40x against their 3.46x.**
Both are near the 4x theoretical ceiling.

### thedodgegarage — the CPU-wall demonstration

Celeron 1000, **16-bit**, 640x480: V4 4500 (1 chip) **83**, V5 5500 (2 chips)
**81**, V5 6000 (4 chips) **82**. Zero scaling.

### Colour-depth and CPU controls

| source | what it controls for | result |
|---|---|---|
| VoodooExtreme Dec 2000 (3dfx reference driver, **our lineage**) | 16 vs 32-bit @ 1024x768 | 88.4 vs 86.1 = **1.03x** (reviewer calls it a driver bug) |
| VoodooAlert (community driver, same silicon) | 16 vs 32-bit | 155.1 vs 128.7 = **1.20x** |
| AnandTech Oct 2000 (GeForce2 GTS, Q3 640x480x16) | CPU clock | Athlon 800=128, 900=137, 1000=144, 1200=162; control 1024x768x32 flat at 83 across 800-1200 |
| AnandTech (Athlon 750, 16-bit, 640x480) | chip count | 5500 (2 chips) 83.0 = 4500 (1 chip) 83.0 = **1.00x** |

### The scaling-ratio agreement, which is the result that actually travels

| source | config | 1024x768 scaling |
|---|---|--:|
| x86-secret 2005 (Athlon XP 2800+, 32-bit max) | real prototype | **2.66x** |
| **ours** (AmigaMerlin 3.1-R11, Athlon 1152, 16-bit) | Strange God recreation | **2.66x** |
| GamersNexus 2023 (XP 2600+, 32-bit trilinear max) | VRG 1.05.04 | 3.46x |

**In AmigaMerlin's hands this board performs exactly as the published record
says a Voodoo5 6000 should.** Absolute figures beat every published number at
every resolution (152.6 / 150.0 / 147.1 / 119.4 against x86-secret's
148.2 / 141.8 / 130.3 / 92.1) despite roughly half the CPU clock — because we
run 16-bit and they ran 32-bit at max detail, which the controlled pair above
puts at 1.20x.

---

## 5. Conditions and caveats — read before comparing anything

- **Colour depth.** All of our runs are **16-bit**. The published sets are
  mostly **32-bit at max detail**. The controlled 16 -> 32-bit ratio on this
  silicon is **1.20x** (VoodooAlert). Apply it, or compare only ratios.
- **CPU.** The published CPUs do **not** sit on one side of ours: they span
  **0.65-2.4x our clock**. Below us are AnandTech's Athlon 750 (0.65x), the
  dual PIII-800 (0.69x) and thedodgegarage's Celeron 1000 (0.87x); above us are
  the Athlon XP 2600+/2800+ and XP-M 2.4-2.5 GHz sets (~1.8-2.4x). It is only
  the **32-bit max-detail** sets we compare absolutes against — x86-secret and
  GamersNexus — that run 1.8-2.4x our clock. Below 1152x864 the CPU is the whole
  difference (§1), in whichever direction it points.
- **Cross-review variance is ~±20%, and that is measured, not assumed.** For the
  identical card, CPU, demo and resolution, AnandTech reports **79.5** where PC
  Perspective reports **71.3**. **Only within-article ratios are trustworthy** —
  which is exactly why the 2.66x-vs-2.66x agreement is worth more than any
  absolute-fps agreement in this file.
- **Demo.** Ours is always `demo four`. Several published sets use `demo001`,
  and some do not state which. A source that does not state its demo cannot be
  compared on absolutes.
- **Resolution.** Anything at or below 1024x768 on this card measures the host
  (§1). Do not read a driver verdict off a 640x480 pair unless — as in §3 — the
  *control runs on the same box in the same hour*.
- **Bus.** This board is **AGP**. PCI 6000s carry a 16-24% bus penalty and are
  not comparable.
- **Run-to-run noise is ~3%** (§0).

---

## 6. Sources excluded, with cause

Recorded so nobody spends a second research pass rediscovering them.

| source | why it is out |
|---|---|
| **gaming2k / tredfx "Athlon 1 GHz 6000" numbers** | **Fabricated.** Denied at source by 3dfx's Dave Barron (*"We haven't sent any out to anyone"*), never archived. Do not chase — this is the one that looks closest to our box and it is not real. |
| **I/ITSEC 1600x1200 4xFSAA demo** | 3dfx **taped over the fps counter** and banned timedemos. Any fps attributed to it is invented. |
| **Hartware.de** | Its "16-bit" runs use the **Fast** preset while its 32-bit runs use High Quality. Not a like-for-like pair. |
| **PCGH 2017 Rev A-3700** | **The card locked up; zero numbers published.** |
| **HotHardware** | Demo unstated, and its own charts are internally inconsistent. |
| **ModLabs zx-c64 6000** | Q3 1.11 / Demo1, 32bpp, WinME, and a **PCI** card (16-24% bus penalty). Four condition mismatches. |

---

## 7. What has no published comparator at all

State this rather than papering over it. All three were established by a
literature sweep against every published Q3 number for this card.

1. **No published Q3 benchmark of any V5 6000 on an Athlon-class CPU at
   1.0-1.2 GHz.** Every 6000 dataset uses a Celeron 1000, an Athlon XP
   2600+/2800+, a dual PIII-800, an XP-M at 2.4-2.5 GHz, or a Phenom. Our box
   sits in the gap between them.
2. **No published 640x480 figure for any modern 6000 recreation at any colour
   depth.** Our 640x480 pair has no direct comparator anywhere.
3. **Nothing whatsoever for a driver built from the leaked H5/Napalm W2K
   source.** That stack appears in no benchmark literature on earth. **The §3
   numbers are the first of their kind** — and AmigaMerlin on the same box in
   the same hour is the only valid control, which is exactly the design already
   run.

There is also **no period review of the 6000 at all** — it never shipped. Every
6000 number in existence is either the single VoodooExtreme article or a modern
retro test.

---

## 8. Reproducing any row in §1-§3

```
# 4-way, the eight-point sweep
tools/v56k/trials/ambench.py 5 "AM-4way" 2,3,4,5,6,7,8,9

# single chip, the four fill-bound points
tools/v56k/trials/ambench.py 0 "AM-1chip" 3,6,8,9
```

The harness writes `SSTH3_SLI_AA_CONFIGURATION` (arg 1) as `REG_SZ` into both
the `Glide` and `D3D` subkeys of `...\3dfxvs\Device0`, kills any stale
`quake3.exe`, clears `C:\q3home\baseq3\qconsole.log`, launches the timedemo with
the §0 settings, and polls for the fps line. It is **liveness-gated**: it takes
`uptime_seconds` before the sweep and aborts on `UNREACHABLE` or `REBOOTED`.

Keep that gate. **None of this session's hardware incidents came from
`ambench.py`** — all three came from poking a live scanout, and they are why
every sweep on this board is gated:

1. an **ungated** early 8-step `vidOverlayDudx` poke sweep — **hard freeze**, NIC
   dead, physical power cycle. (`tools/v56k/trials/dudxsweep2.py` is the *gated
   rewrite* of that sweep, not the version that froze the box — its own docstring
   records the incident.);
2. `vga_vsync_offset` = **31 px** in `tools/v56k/trials/vsyncsweep.py` — **hard
   freeze**, also needing a power cycle. That sweep *was* liveness-gated and the
   gate named the offending step exactly, which is the whole argument for the
   gate; 31 px is now permanently skipped;
3. `FX_GLIDE_FORCE_SLI_BAND_HEIGHT` = 16 — a reboot the box recovered from on
   its own.

In a sweep **the step that breaks the box IS the finding**.

**Trap:** AmigaMerlin's INF installs `FX_GLIDE_REFRESH = 75` into
`...\Device0\Glide`, which overrides the per-resolution refresh for every mode
and can put the monitor out of range. Delete it (or set 60) before benching.

---

## Cross-references

- `FINDINGS.md`, 2026-09-04 — *"OUR DRIVER IS 21% SLOWER THAN AMIGAMERLIN ON ONE
  CHIP, AND THAT IS A SECOND BUG"* (§3, §5, §6, §7 above)
- `FINDINGS.md`, 2026-09-04 — *"the 6000 benchmarks EXACTLY on published data:
  2.66x SLI at 1024x768"* (§2, §4)
- `FINDINGS.md`, 2026-09-04 — *"the V5 6000 is CPU-bound to 1024x768, and the
  fill wall is gradual"* (§1)
- `FINDINGS.md`, 2026-09-04 — *"AmigaMerlin RUNS 4-WAY SLI CORRECTLY ON THIS
  BOARD. The bug is ours."* (§3, and the AmigaMerlin install method)
- `DEPLOY-191-20260904.md` — how the card and our stack got onto `.191`
- `V56K-SLI-FINDINGS.md` — the older `.133` (dual P3-700) benchmark set. Those
  numbers are **not comparable** to this file: different box, different CPU, and
  a set that **spans both VBIOS modes** — `.133` ran 128 MB mode (32 MB/chip),
  the same as here, with §12 and §14 measured after the switch to 256 MB
  (64 MB/chip).
- `tools/v56k/trials/ambench.py` — the harness
