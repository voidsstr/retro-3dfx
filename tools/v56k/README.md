# V5 6000 per-chip instrumentation suite

Reference for the user-mode tools that look at what each of the four VSA-100s
on a Voodoo 5 6000 is *actually* programmed with, and for the trial scripts
that drive them on hardware. **Nothing here needs a driver rebuild** — every
tool rides escapes the SHIPPING `3dfxv5d.dll` already answers.

Target: box **`.191`** (`192.168.1.191`, agent port 9898) — the "Strange God"
V5 6000 recreation, 4x VSA-100 @166 MHz behind a HiNT HB1-SE66 bridge, 128 MB
VBIOS mode (32 MB/chip), AGP, Athlon 1152 / nForce2, XP SP3. Deploy dir on the
box is `C:\RETRO_AGENT\v56k-deploy\`.

Raw investigation log: `FINDINGS.md`, the 2026-09-04 entries (referenced by
name throughout). This file is the *how to use it* reference; FINDINGS is the
*what we learned* log. Do not duplicate one into the other.

---

## READ THIS FIRST — six traps, every one paid for on hardware

**1. `HWCEXT_GET_SLAVE_REGS` needs `GETLINEARADDR` first, NOT `ALLOCCONTEXT`.**
`hwcGetLinearAddr` (`HWCEXT.C:785-825`) has an "if a GLIDESTATE already exists,
return the old mapping" branch that hands back the OLD never-mapped bases and
never fills `glideSlaveRegBase[]`. Send `ALLOCCONTEXT` (0x01) first and you get
**four silent zeros** instead of an error. Correct order is
`HWCEXT_GETLINEARADDR` (0x03) then `HWCEXT_GET_SLAVE_REGS` (0x19). Neither is
exclusive-gated, so both run against a live fullscreen game.

**2. `HWCEXT_PCI_OP` (0x18) is gated on `HWC_EXCLUSIVE`** (`HWCEXT.C:2409`).
Only the process that owns the card in fullscreen Glide may touch PCI config
space. That is the entire reason `sligrid` exists as a Glide app rather than a
console tool: `fxscan2` can never reach `cfgVideoCtrl0` or `cfgSliAaMisc`.

**3. NO SOFTWARE CAN SEE THE SCANOUT DEFECT. Stop building detectors.**
VERIFIED ON HARDWARE with a bit-exact static pattern: while the monitor was
visibly skewed and mis-coloured, `sligrid --readback` reported

    readback: 0 of 307200 pixels differ (0.0000%)

**Zero.** Reads through the master's BAR1 are SLI-gathered *in hardware*
(`CFG_SLI_RD_EN` is set on every chip in every SLI arm), so the card
reassembles a flawless image for any reader. Screenshots, `grLfbReadRegion`,
GDI capture, DirectDraw Lock and pixel diffs are all useless for this class of
bug. Per-chip framebuffer readback does not exist either — `GlideMapSlaveChips`
aliases every slave's FB VA to the master's (`SLIAA.C:1402-1404`). **The only
sensors are per-chip REGISTER state and an operator's eye on the monitor.**

**4. `vga_vsync_offset = 31 px` HARD-FREEZES the board.** `cfgSliAaMisc` =
`0x81F` (`pixels=7, chars=3`) killed `.191` outright — NIC dead, physical power
cycle required. This is exactly the `vga_crtc_fast` bug 3dfx's own comment
describes at `SLIAA.C:2489-2494`, confirmed on silicon. **Never program it.**
`trials/vsyncsweep.py` skips it permanently; keep it skipped.

**5. Nothing restores a poke for you.** `fxscan2 poke` with `holdms=0` and every
`sligrid --poke` write are left in place deliberately, so the operator can look
at the monitor. An MMIO poke survives until something re-runs a mode set; a PCI
config-space poke has no restore path in the tool at all. **Treat a reboot as
the only reliable clear** before any measurement that must be trusted, and
never carry a poked box into a benchmark run.

**6. Read the registers DURING the failing state, never at the desktop.** After
a game exits the slaves keep the game's geometry — nothing restores them — so a
desktop dump shows a divergence that is not one. Worse, the reverse also
happens: with the desktop at 1024x768 and the slaves left at 640x480 from a
previous run, a 640x480 dump shows the four chips "agreeing" *by coincidence*.
**A static dump cannot tell "correctly programmed" from "stale but
coincidentally equal".** That is what `ring` is for.

---

## Build

```bash
cd tools/v56k
i686-w64-mingw32-gcc -O2 -Wall -o fxscan2.exe fxscan2.c -lgdi32
i686-w64-mingw32-gcc -O2 -Wall -o fxpci.exe   fxpci.c

# sligrid needs the Glide SDK from the sibling repo:
i686-w64-mingw32-gcc -O2 -Wall -o sligrid.exe sligrid.c \
  -I<retro-agent>/scripts/3dfx/glide-sdk/include \
  <retro-agent>/scripts/3dfx/glide-sdk/lib/libglide3x_retail.dll.a \
  -lgdi32 -luser32
```

**Build gap, current tree:** `sligrid.c` includes `"pattern_gen.h"` and calls
`sligrid_pattern(pat, W, H)`, and that header is **not in the repo** (verified:
absent from `tools/v56k/` and untracked anywhere in the tree). `sligrid.exe` on
the box was built from a scratchpad copy. Anyone rebuilding it must restore or
re-author the header — it need only fill a `unsigned short*` with a static,
bit-exact RGB565 pattern of `W*H` pixels. `--emit <file>` writes that pattern to
disk without touching the card, which is the cheap way to check a replacement.

Binaries are pushed to `C:\RETRO_AGENT\v56k-deploy\` and run through the retro
agent (`EXEC` / `EXECW` / `LAUNCH`).

---

## `fxscan2` — per-chip scanout registers over MMIO

```
fxscan2 dump [--json]                   per-chip table + must-match verdict
fxscan2 diff                            exit 1 if the must-match set diverges
fxscan2 phase [secs]                    per-chip CRTC period + relative line phase
fxscan2 poke <chip> <off> <val> [ms]    ms defaults to 8000; ms=0 = leave it set
fxscan2 fixtest [ms]                    copy master geometry to slaves, pulse vidproc
fxscan2 ring [secs] [interval_ms] [outfile]
```

Uses `HWCEXT_GET_SLAVE_REGS` (0x19, `HWCEXT.H:565`, handled `HWCEXT.C:2732`),
which returns each chip's four register windows mapped **read/write** into the
caller — so it both reads and pokes per-chip scanout state from user mode
against a live fullscreen game. `HWCEXT_MAX_SLAVE_REGS` is 4 and the FB window
is never among them.

Observe trap 1 (sequencing) and trap 6 (read it in the failing state).

### The watched register set

Offsets are byte offsets into `SstIORegs`, derived by counting `FxU32` members
of `H5/INCLUDE/H3REGS.H:121-194`. `ring` samples all of these on all chips;
`dump`/`diff` print them with the must-match verdict.

| off | register | must-match | note |
|---|---|:--:|---|
| `0x000` | `status` | - | |
| `0x00C` | `lfbMemoryConfig` | - | MUX reg; read reflects bits 29:30 select. COPIED by miniport `SLIAA.C:3555` |
| `0x010` | `miscInit0` | yes | **Y-origin**; NOT in the miniport 5-reg copy |
| `0x014` | `miscInit1` | - | bit 8 = `POWERDOWN_DAC` |
| `0x028` | `vgaInit0` | yes | |
| `0x02C` | `vgaInit1` | yes | |
| `0x040` | `pllCtrl0` | yes | pixel clock |
| `0x044` | `pllCtrl1` | yes | video PLL |
| `0x048` | `pllCtrl2` | yes | video PLL |
| `0x04C` | `dacMode` | - | |
| `0x058` | `vidMaxRGBDelta` | yes | DOS copies it, W2K does not |
| `0x05C` | `vidProcCfg` | yes | COPIED by miniport `SLIAA.C:3557` |
| `0x094` | `vidCurrentLine` | - | the `HB` heartbeat source |
| `0x098` | `vidScreenSize` | yes | not copied by W2K (see below) |
| `0x09C` | `vidOvlStartCoords` | yes | DOS copies it, W2K does not |
| `0x0A0` | `vidOvlEndCoord` | yes | DOS copies it, W2K does not |
| `0x0A4` | `vidOverlayDudx` | yes | **SLI hsync handover column** — live lever |
| `0x0A8` | `vidOvlDudxOffSrcW` | yes | COPIED by miniport `SLIAA.C:3558` |
| `0x0E4` | `vidDesktopStart` | yes | COPIED by miniport (DOS refuses to) |
| `0x0E8` | `vidDesktopStride` | yes | COPIED by miniport |
| `0x0B0` | `vgaRegister[0]` | - | MMIO alias of VGA I/O `0x3B0..0x3DF` |

**The miniport's master->slave copy is exactly five registers**, the block at
`Miniport/H5/SLIAA.C:3555-3559`, in this order: `lfbMemoryConfig` (3555),
`vidDesktopStartAddr` (3556), `vidProcCfg` (3557),
`vidOverlayDudxOffsetSrcWidth` (3558), `vidDesktopOverlayStride` (3559). Cite the
block, not a remembered line — the five are one after another and an off-by-one
attributes a copy to the wrong register.

`vgaRegister[0]` is `0x0B0`; `sligrid` reaches the legacy CRTC pair through it
at `0x0B0+0x24` (index, legacy `0x3D4`) and `0x0B0+0x25` (data, `0x3D5`).

**Offset arithmetic, since it is easy to get wrong:** `vidDesktopStartAddr` is
`0x0E4` and `vidDesktopOverlayStride` is the very next `FxU32`, so **`0x0E8`**.
The 12-entry `vgaRegister[]` array occupies `0x0B0..0x0DC` and
`vidOverlayDvdyOffset` sits at `0x0E0`; miscount that array and every offset
past it shifts by 4.

### Reference values, 640x480 4-way SLI on `.191` (VERIFIED ON HARDWARE)

Captured with `dump` under both driver stacks. See FINDINGS.md, 2026-09-04
entry *"AmigaMerlin RUNS 4-WAY SLI CORRECTLY ON THIS BOARD"*.

| register | AmigaMerlin 3.1-R11 (CORRECT) | ours, H5 + SGL ICD (SKEWED) |
|---|---|---|
| `vidProcCfg` | `03E60101` | `33E60101` |
| `pllCtrl0` | `0000D137` (25.176 MHz, 60 Hz) | `0000B31F` (35.994 MHz, 85 Hz) |
| `vidScreenSize` | `001E0280` | `001E0280` |
| `vidDesktopStride` | `0000000A` | `0000000A` |
| `vidDesktopStart` | `01F60000` | `01F60000` |
| `lfbMemoryConfig` | `000A3D58` | `000A3D58` |
| `miscInit0` | master `077C0000`, slaves `0` | master `077C0000`, slaves `0` |

`vidProcCfg` differs by exactly `0x30000000` — bits 28 and 29. Everything else
that differs is a consequence of the refresh rate. The `miscInit0` master/slave
split is **BY DESIGN** and diverges identically in both stacks.

### `fxscan2 ring` — the Glide-path flight recorder

    fxscan2 ring <secs> <interval_ms> <outfile>

The display driver's registry ring (`RLog00..RLog31`) **cannot see a Glide
fullscreen session**: the driver releases the hardware on
`DrvAssertMode(DISABLE)` and Glide programs the card directly, so nothing in
`3dfxv5d.dll` is on the path. `ring` is the equivalent recorder, sampled from
outside through the per-chip register windows.

- Samples **every** watched register on **every** chip each `interval_ms`
  (default 100 ms) and writes a line whenever **any** of them changes.
- Emits an `INITIAL SNAPSHOT` block of all chips x all registers on the first
  pass, so the log is self-contained.
- Emits a per-second `HB` heartbeat of each chip's `vidCurrentLine` (masked to
  11 bits), so a **stalled CRTC** shows up even when no register moves.
- **`fflush` after every line.** If the box wedges, the file on disk still holds
  the last state *before* the wedge. That is the whole point.
- Read-only — it never writes hardware, so it cannot contaminate a later run.

**Start it BEFORE launching the game** (`trials/ringrun.py` does exactly this).
The mode transition is the interesting part, and once the game is up there is
nothing left to see.

Log format:

```
# ms      chip reg                  old      -> new
    6562  0   vidScreenSize        00300400 -> 001E0280
    6562  0   vidDesktopStride     00000010 -> 0000000A
    6703  0   miscInit0            00000000 -> 077C0000
    6703  1   vidProcCfg           33E60100 -> 33E60101
    7003  HB   curline  c0= 312  c1= 311  c2= 313  c3= 312
```

**What it proved that `dump` could not:** across the desktop -> Glide transition
at 640x480 the master receives the full geometry program
(`vidScreenSize`/`vidDesktopStride`/`vidDesktopStart`/`lfbMemoryConfig`/
`miscInit0`) and **chips 1-3 receive only `vidProcCfg` and `dacMode`**. A
snapshot could never have shown that — see trap 6, and FINDINGS.md,
2026-09-04 entry *"A scanout flight recorder for the GLIDE path"*.

### `fxscan2 fixtest`

Copies the master's `vidScreenSize`, `vidOvlStartCoords`, `vidOvlEndCoord`,
`vidOverlayDudx` and `vidMaxRGBDelta` to every slave, then pulses each slave's
`SST_VIDEO_PROCESSOR_EN` bit in `vidProcCfg` — the reset the Napalm spec
(11.1.18) requires after a screen-size change, which the DOS reference performs
(`MINIHWC/DOS_MODE.C:645`) and W2K's `EnableSLIAA` does not. `holdms > 0`
restores the previous values afterwards; `holdms = 0` leaves them.

### `fxscan2 phase` — and why it is NOT a proxy metric

Measures each chip's CRTC frame period and relative line phase against chip 0.
An early run showed chip 2 free-running at **+148 ppm** and a whole hypothesis
was built on it. **FALSIFIED**: re-measured from a clean boot with the skew
fully reproduced, chip1 `+0.000`, chip2 `+0.093`, chip3 `+0.047` ppm — all four
locked, picture still skewed. The +148 ppm was transient and does not
reproduce. Clock lock is not the fault and phase does not track this bug.

---

## `sligrid` — Glide test pattern + in-process PCI probe

```
sligrid --mode 640x480 --hold 45 [--regs] [--pci] [--readback C:\rb.bmp]
sligrid --emit C:\ref.bin --mode 640x480               (no card touched)
sligrid --mode 640x480 --hold 20 --poke 1:AC=0x827     (live PCI config write)
```

A fullscreen Glide app that draws a **static, bit-exact** pattern into both
colour buffers and then stops swapping, because a static scanout is what an
operator can actually judge. Modes: 640x480, 800x600, 1024x768, `GR_REFRESH_60Hz`,
RGB565. A watchdog thread kills the process at `hold + 20` s so a wedged trial
cannot leave the box in fullscreen forever.

It exists as a Glide app for one reason: **trap 2** — `HWCEXT_PCI_OP` only
answers a caller holding `HWC_EXCLUSIVE`, so the PCI probe has to live inside
the process that owns the card. This is the only route to `cfgVideoCtrl0` and
`cfgSliAaMisc` without a driver rebuild.

| flag | effect |
|---|---|
| `--mode WxH` | 640x480 / 800x600 / 1024x768 |
| `--hold N` | seconds to hold the static pattern on screen |
| `--regs` | per-chip MMIO dump from inside exclusive mode |
| `--pci` | per-chip `cfgVideoCtrl0/1/2`, `cfgSliLfbCtrl`, `cfgSliAaMisc` |
| `--phase N` | in-process phase sample |
| `--readback F` | `grLfbReadRegion` the front buffer, diff vs the pattern, write BMP |
| `--emit F` | write the reference pattern to a file, touch no hardware |
| `--poke fn:OFF=VAL` | PCI config write, **repeatable up to 8 times** |

`--poke` parses `%lu:%lx=%lx` — function number, hex config offset, hex value —
and prints `old -> new (readback)` for each. Eight is a hard limit
(`npoke < 8`); extra `--poke` arguments are silently dropped. Writes are **not
restored** (trap 5). `trials/vsyncsweep.py` uses three per step, one for each
slave chip.

Remember trap 3 about `--readback`: it is there to *prove* the framebuffer is
clean, not to detect the bug.

### PCI config registers reachable this way

3dfx-private, documented at `Miniport/H5/SLIAA.H:273-279`. **These are NOT in
MMIO** — `fxscan2 poke` cannot reach any of them.

| off | register | contents |
|---|---|---|
| `0x80` | `cfgVideoCtrl0` | bit 11 `CFG_VIDPLL_SEL`, bits 14:12 `divide_video`, bits 19:16 `vsync_ref`, bit 25 `CFG_DAC_HSYNC_TRISTATE` |
| `0x84` | `cfgVideoCtrl1` | SLI render/compare masks |
| `0x88` | `cfgVideoCtrl2` | SLI render/compare masks |
| `0x8C` | `cfgSliLfbCtrl` | bit 28 `CFG_SLI_RD_EN` (the reason trap 3 exists) |
| `0x94` | `cfgAaLfbCtrl` | |
| `0xAC` | `cfgSliAaMisc` | bits 8:0 `vga_vsync_offset` = `pixels[2:0]` \| `chars[5:3]` \| `hxtra[8:6]` — **the only inter-chip horizontal alignment knob in the whole stack** |

**`cfgSliAaMisc` as shipped, read from hardware in 4-way SLI:**

```
chip0  00000800   [0 0 0 =  0 px]   master
chip1  00000827   [7 4 0 = 39 px]
chip2  00000827   [7 4 0 = 39 px]
chip3  00000827   [7 4 0 = 39 px]
```

Field value to poke = `0x800 | (chars << 3) | pixels`. Documented candidates
are 31 (`chars=3`, **NEVER — trap 4**), 39 (`chars=4`, the shipped default,
`SLIAA.C:2489-2515` Case A) and 47 (`chars=5`, Case B, "run slave 8 clocks
ahead").

**Four values have ever been observed on `.191`, not seven:** 7, 15 and 23 px
ran in the sweep, plus the **39 px** shipped default read straight off the
hardware. `vsyncsweep.py` **aborts the run on a wedge** (it returns 3), and
31 px froze the board, so **47, 55 and 63 px were never reached**. 47 px is a
*documented candidate* in the driver source, never a result — do not read it as
a value that ran.

### `cfgVideoCtrl0` bit 11 — a poke that BLANKS THE SCREEN (FALSIFIED)

Read from inside exclusive mode, the master really does differ from Win9x:

```
chip cfgVideoCtrl0 [vidpll_sel]
  0  00000001   FREERUN     <- master, bit 11 CLEAR
  1  00000803   LOCKED
  2  00000803   LOCKED
  3  00000803   LOCKED
```

That matches the Win9x branch W2K dropped (`MINIVDD/SLIAA.C:1690`). **Poking
bit 11 on the master produced NO PICTURE AT ALL** — per Databook 3.4.9 the
master's `SYNC_CLK_IN`/`SYNC_CLK_FB` are grounded on this board, so slaving its
PLL to an undriven input stops its video clock.
`optimized/v56k-sli-scanout-candidates/02-master-vidpll-sel.patch` is therefore
**not a fix**; it is retained only as evidence. Do not apply it blind.

---

## `fxpci` — PCI config space with no driver at all

```
fxpci scan                                enumerate 121A:0009 functions
fxpci dump                                0x00-0xFF of every function + decode
fxpci get  <fn> <off>
fxpci set  <fn> <off> <val> [restore_ms]
fxpci vidpll <fn> <0|1> [restore_ms]      set/clear CFG_VIDPLL_SEL
fxpci vsyncoff <fn> <pix> <chr> <hxtra> [restore_ms]
```

Mechanism #1 port I/O (`0xCF8`/`0xCFC`) direct from user mode, enabled with
`NtSetInformationProcess(ProcessUserModeIOPL = 34)`. **Needs `SeTcbPrivilege`.**
The agent running as the console Administrator does not hold it, and the tool
reports `NOT_PRIVILEGED` and exits without touching anything rather than
half-working. Running it from a temporary service was tried and produced no
output.

**In practice, use `sligrid --pci` instead.** `fxpci` is the no-driver fallback
(useful when no 3dfx driver is installed at all); the working route on a
configured box is the Glide-hosted probe. The remaining options are reading
these from the miniport or a real LocalSystem launcher.

---

## `trials/` — hardware trial scripts

All of them talk to `.191` through `retro-agent`'s `RetroConnection`
(`sys.path.insert` to `/home/voidsstr/development/retro-agent`), reconnect with
backoff, and are meant to be run one at a time from the dev host.

| script | what it does |
|---|---|
| `ambench.py <sliconfig> <tag> [r_modes]` | Q3 timedemo sweep across resolutions; fps + completion, liveness-gated per run. `r_mode` map 2..9 = 512x384..1600x1200 |
| `amtest.py <sliconfig 0\|5> <label>` | single automated pass/fail: does the timedemo print an fps line |
| `ringrun.py [mode] [secs]` | start `fxscan2 ring`, **then** launch Q3, so the transition is captured |
| `ringtrial.py <mode> <secs> <tag>` | ring-capture a session and LEAVE the game running for an operator verdict |
| `dudxsweep2.py [hold]` | 8-step `vidOverlayDudx` (IO `0x0A4`) sweep, fully liveness-gated |
| `vsyncsweep.py [hold]` | `cfgSliAaMisc` `vga_vsync_offset` sweep via `sligrid --poke`; **31 px permanently skipped** |
| `band16_setup.py [band] [mode]` | force Glide's SLI band height and PROVE it took from Glide's own `SLICTRL` line before anyone looks at the monitor |
| `trial_setup.py <yorigin\|fullgeom\|baseline> [mode]` | apply a per-chip state, leave everything running, exit |
| `trial_teardown.py` | kill Q3, `DISPLAYCFG set 1024 768 16 85`, report uptime |

### The automated pass/fail metric

Whether a Q3 timedemo **completes and prints an fps line** is a perfectly good
health check needing no eyes — a driver that wedges the card never gets there.
Both `amtest.py` and `ambench.py` are built on it, and the whole
AmigaMerlin-vs-ours comparison table in FINDINGS was produced without anyone
watching the screen.

### Persistent visual trials: split setup from teardown, never time-box

A trial that needs the operator's eyes is split in two (see `tests/README.md`):
`*_setup.py` applies the state, leaves the game running and **exits**;
`trial_teardown.py` is a separate run, only after the verdict. The operator's
answer is worthless if the screen changes before they give it.

### The liveness gate — REQUIRED for every sweep that pokes hardware

Poking a live SLI scanout can wedge the video hardware. Three separate incidents
on `.191` in one session — keep them apart, they are not one story:

- **hard freeze** (NIC dead, physical power cycle) from an **ungated** 8-step
  `vidOverlayDudx` poke sweep. Nothing named the step; the run had to be
  reconstructed afterwards. `dudxsweep2.py` is the gated rewrite.
- **hard freeze** (NIC dead, physical power cycle) at the **31 px**
  `vga_vsync_offset` step (trap 4). This one **was** liveness-gated and the gate
  named the step exactly — which is the entire case for the gate, and the reason
  `vsyncsweep.py` now skips 31 px permanently.
- **self-recovering reboot** from SLI band height 16.

Neither freeze came from `ambench.py`: benchmarking pokes no hardware, which is
why the fps sweeps run unattended and the poke sweeps do not.

**The step that breaks the box IS the finding**, so the sweep must name it.
Pattern, from `dudxsweep2.py` and `vsyncsweep.py`:

1. Read a baseline `SYSINFO` `uptime_seconds` before the first step. Unreachable
   at the start = `ABORT`, do not proceed.
2. Before each step, reconnect. No connection = print
   `BOX DIED BEFORE STEP <n>` and **stop**.
3. Apply the step's pokes; any exception = print `POKE FAILED ON STEP <n>` and
   **stop**.
4. Hold for the operator, then reconnect and read `uptime_seconds` again.
   - unreachable -> `BOX WENT UNREACHABLE DURING STEP <n> — that step wedges it`
   - `uptime < baseline` -> `BOX REBOOTED DURING STEP <n>`
   - either way, **stop**; do not carry on into the next step.
5. Only on a clean finish: kill the app and restore the desktop mode.

Steps must be numbered and the number printed with the step, because the
operator reports a step *number*, not a description. Print with `flush=True` —
the run is long and the output is the record.

---

## What these instruments have already settled

Cross-references into `FINDINGS.md` (2026-09-04). Do not re-propose the
falsified items; they cost hardware time each.

**VERIFIED ON HARDWARE**
- AmigaMerlin 3.1-R11 renders 4-way SLI on this board with **no skew**. The
  board, the HiNT bridge and the analog combine are all fine — **the scanout
  defect is ours.** (*"AmigaMerlin RUNS 4-WAY SLI CORRECTLY ON THIS BOARD"*)
- The framebuffer is bit-exact perfect while the monitor is wrong (trap 3).
- Across the mode transition, slaves receive only `vidProcCfg` + `dacMode`.
- `vidOverlayDudx` and `cfgSliAaMisc` are both **live levers**: poking either
  visibly changes the artefact. Neither value found so far fixes it.
- `vidProcCfg` bits 28/29 are 3dfx's own high-res heat erratum workaround
  (`Miniport/H5/h3modeset.c:667-672`); BIT(28) is unnamed in every copy of
  `H3DEFS.H` in this tree. (*"the skew bits are NAMED IN 3dfx's OWN CODE"*)

**FALSIFIED — do not re-propose**
- "Slaves never get `vidScreenSize`" — they do, during a game.
- Slave Y-origin (`miscInit0`) divergence is **by design**; forcing the
  master's `077C0000` onto the slaves **blanked the screen** (Ctrl-Alt-Del to
  recover). "Slave register != master register" is not automatically a bug.
- Master `CFG_VIDPLL_SEL` — blanks the screen (see above).
- Chip-2 clock drift — transient, does not reproduce, all four chips locked.
- SLI render/compare masks — match 3dfx's own `Video SLI AA Configs.xls`.
- Not tristating hsync in the analog arm is **correct** per that sheet.
- Band height 8 is correct at 640x480; forcing 16 **reboots the board**
  (group height must divide screen height: 480/32 = 15, 480/64 = 7.5).
- A sweep of eight `vidOverlayDudx` configurations was skewed in every one.

> **Where the 60 Hz vs 85 Hz difference came from — read this before using the
> register table above as evidence.** The benchmark harness `ambench.py` pins
> refresh (`FX_GLIDE_REFRESH=60` and `+set r_displayRefresh 60`), but the
> register dumps were NOT taken under it. They came from the trial scripts
> (`trial_setup.py`, `ringrun.py`, `ringtrial.py`), none of which set a refresh
> at all, so those runs took the ICD's own per-resolution default of 85 Hz. The
> AmigaMerlin dump was taken while `FX_GLIDE_REFRESH` had been forced to 60 to
> stop the monitor going out of range. **So the 60-vs-85 gap is a configuration
> difference this session introduced between the two captures, not a property of
> either driver.** Any future capture must pin refresh explicitly or the
> comparison is worthless.

**STILL OPEN**
- The 2x2 that separates the erratum bits from the refresh rate: `vidProcCfg`
  bits 28/29 set-vs-clear (poke IO `0x05C`) crossed with 60 Hz vs 85 Hz.
  **On OUR driver exactly one cell has been observed** (85 Hz + bits set =
  skewed). The other three are unrun; AmigaMerlin's clean 60 Hz/bits-clear
  picture is a *different driver* and is not a substitute for the cell.
- The right `vga_vsync_offset` value, if one exists. Only 7, 15 and 23 px have
  actually run, plus the 39 px shipped default; the sweep aborted at 31 px, so
  47, 55 and 63 px remain untried.
- Our driver is **21% slower than AmigaMerlin on a single chip** (92.5 vs
  116.5 fps, 640x480x16, `demo four`) — a second, independent bug that has
  nothing to do with SLI. (*"OUR DRIVER IS 21% SLOWER THAN AMIGAMERLIN ON ONE
  CHIP"*)
