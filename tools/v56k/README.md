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
