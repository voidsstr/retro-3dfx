# V5 6000 per-chip scanout instruments

Two user-mode tools for looking at what each VSA-100 on a Voodoo 5 6000 is
actually programmed with. **Neither needs a driver rebuild.** Built with mingw
on the dev host, run on the box through the retro agent.

```bash
i686-w64-mingw32-gcc -O2 -Wall -o fxscan2.exe fxscan2.c -lgdi32
i686-w64-mingw32-gcc -O2 -Wall -o fxpci.exe   fxpci.c
```

## Why these exist

A CRTC **scanout** defect cannot be photographed from software. Every readback
path — in-engine screenshot, Glide `grLfbReadRegion`, GDI `SCREENSHOT`,
DirectDraw Lock — reads memory, and reads through the master's BAR1 are
**SLI-gathered in hardware** (`CFG_SLI_RD_EN` is set on every chip in every SLI
arm), so the card hands any reader a correctly reassembled image. Per-chip
framebuffer readback does not exist either: `GlideMapSlaveChips` aliases every
slave's FB VA to the master's (`SLIAA.C:1402-1404`).

So per-chip **register** state is the only instrument. See `FINDINGS.md`,
"The V5 6000's SLI band skew", for the dead ends that were each tested rather
than assumed.

## `fxscan2` — per-chip scanout registers (MMIO)

Uses escapes the SHIPPING `3dfxv5d.dll` already answers, chiefly
`HWCEXT_GET_SLAVE_REGS` (0x19, `HWCEXT.H:565`, handled `HWCEXT.C:2732`), which
returns each chip's register windows mapped **read/write** into the caller.

```
fxscan2 dump [--json]      per-chip register table with a must-match verdict
fxscan2 diff               exit 1 if the must-match set diverges
fxscan2 phase [secs]       per-chip CRTC frame period + relative line phase
fxscan2 poke <chip> <off> <val> [holdms]    holdms=0 leaves the value in place
fxscan2 fixtest [holdms]   copy master geometry to slaves + pulse their vidproc
```

**Sequencing trap:** do NOT send `HWCEXT_ALLOCCONTEXT` first.
`hwcGetLinearAddr` (`HWCEXT.C:785-825`) has an "if a GLIDESTATE already exists,
return the old mapping" branch that returns the OLD never-mapped bases and never
fills `glideSlaveRegBase[]` — you silently get four zeros. Order must be
`GETLINEARADDR` (0x03) then `GET_SLAVE_REGS` (0x19). Neither is exclusive-gated,
so both run against a live fullscreen game.

**Read it DURING the failing state.** At the desktop the slaves still hold the
last game's geometry (nothing restores them), which looks like a divergence and
is not one.

`phase` is the one that found the real anomaly: chip2 free-running +148 ppm
against chip0 while chips 1 and 3 held within 0.5 ppm.

## `fxpci` — the 3dfx-private PCI config registers

The inter-chip video timing registers are NOT in MMIO (`SLIAA.H:273-279`):
`0x80 cfgVideoCtrl0` (bit 11 `CFG_VIDPLL_SEL`, bit 25 `CFG_DAC_HSYNC_TRISTATE`),
`0x84/0x88 cfgVideoCtrl1/2` (SLI render/compare masks), `0x8C cfgSliLfbCtrl`,
`0xAC cfgSliAaMisc` (bits 8:0 `vga_vsync_offset` — the only inter-chip
horizontal alignment knob).

**Needs `SeTcbPrivilege`**, which the agent (running as the console
Administrator) does not hold. Running it from a temporary service was tried and
did not produce output; the practical route is to read these from the miniport
instead, or run it with a real LocalSystem launcher.

## `fxscan2 ring` — the GLIDE-path flight recorder

    fxscan2 ring <secs> <interval_ms> <outfile>

The display driver's registry ring cannot see a Glide fullscreen session: the
driver RELEASES the hardware (`DrvAssertMode(DISABLE)`) and Glide programs the
card directly, so nothing in `3dfxv5d.dll` is on the path. This is the equivalent
recorder, sampled from outside through the per-chip register windows.

Logs a line whenever any watched register on any chip changes, plus a per-second
`HB` heartbeat of each chip's `vidCurrentLine` (so a stalled CRTC shows up too).
**Every line is flushed**, so if the box wedges the ring still holds the last
state before the wedge. **Start it BEFORE launching the game** — the mode
transition is the interesting part.

This is what proved the slaves are never given geometry: across the transition
the master gets `vidScreenSize`/`vidDesktopStride`/`vidDesktopStart`/
`lfbMemoryConfig`/`miscInit0` and chips 1-3 get only `vidProcCfg` + `dacMode`.
A static `dump` could not show that, because the slaves were sitting on the
previous session's resolution and therefore *coincidentally matched*.

## `sligrid.c` — Glide test pattern + in-process PCI probe

A fullscreen Glide app that draws a static, bit-exact pattern. It matters because
`HWCEXT_PCI_OP` (0x18) is gated on the caller holding `HWC_EXCLUSIVE`
(`HWCEXT.C:2409`) — **only the process that owns the card in fullscreen Glide can
touch PCI config space**. So the probe has to live inside a Glide app. Supports
`--pci`, `--regs`, `--readback`, and `--poke fn:OFF=VAL`. This is the only route
to `cfgVideoCtrl0` (`CFG_VIDPLL_SEL`, `CFG_DAC_HSYNC_TRISTATE`) without a driver
rebuild.

Build needs the Glide SDK from the sibling repo:

```bash
i686-w64-mingw32-gcc -O2 -Wall -o sligrid.exe sligrid.c \
  -I<retro-agent>/scripts/3dfx/glide-sdk/include \
  <retro-agent>/scripts/3dfx/glide-sdk/lib/libglide3x_retail.dll.a -lgdi32 -luser32
```

## `trials/` — persistent visual trials

A trial that needs the operator's eyes is split in two and **never time-boxed**
(see `tests/README.md`): `*_setup.py` applies the state, leaves the game running
and exits; `trial_teardown.py` is run separately, only after the verdict.
`dudxsweep2.py` is the liveness-gated sweep pattern — poking a live scanout
hard-froze the box once, so every step is bracketed by an agent round-trip and an
uptime check, and the sweep stops on the step that breaks it.
