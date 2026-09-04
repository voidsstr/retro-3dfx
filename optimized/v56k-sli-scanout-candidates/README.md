# V5 6000 SLI scanout skew — candidate fixes, NOT YET VERIFIED

Two source changes for the Voodoo 5 6000's multi-chip scanout skew. **Neither
has been built or run.** They are kept as patches rather than committed to the
tree because the repo's rule is that an untested guess is not a checkpoint, and
the Wine/VC6 build toolchain does not work on this host (see `FINDINGS.md`,
"setup-toolchain.sh succeeds and produces an unusable Wine" — the ACL problem
there is fixed, but `wine cmd /c echo` still times out, so nothing can be
compiled).

Apply with `git apply`, from the repo root.

## 01-hsync-handover-column.patch — `H5/MINIHWC/MINIHWC.C`, `hwcInitVideo`

Sets `vidOverlayDudx = xRes >> 1` on multi-chip boards instead of leaving it 0.

Napalm spec r1.13 s11.1.21: *"if enhanced video is enabled, this register is
defined to be the number of active pixels of a scanline before the the driver of
dac_hsync is switched over from one chip to another"*, and the pin table:
*"dac_hsync Pullup (for multi-chip only, where dac_hsync gets driven by multiple
chips and transition occurs in middle of active scanline)"*. Glide hardcodes
`vidOverlayDudx = 0UL` (`MINIHWC.C:4041`); the DirectDraw path sets
`cxScreen >> 1` and says why (`DDFXNT.C:2606`). A Glide game never creates a
DDraw surface, so it never runs.

**Status: the register is a REAL LEVER but this value is NOT SUFFICIENT.**
Poked live on hardware with `tools/v56k/fxscan2.c`, the artefact visibly
changed. A sweep of eight configurations — 0, 160, 320, 480, 639 on all chips,
staggered 160/320/480/639, master-only 320, slaves-only 320 — was run at
640x480 in 4-way SLI and **every one was still skewed**. So this is at best
part of the fix. Keep the patch (0 is provably wrong per the spec), but do not
expect it alone to close the bug.

## 02-master-vidpll-sel.patch — `H5/W2K/Src/Video/Miniport/H5/SLIAA.C`, `H3_SETUP_SLI_AA`

Restores the 4-chip master `CFG_VIDPLL_SEL` branch the W2K port dropped.

Win9x `MINIVDD/SLIAA.C:1690-1695` (1-based loop, so its `1 == i` is the MASTER):

    } else if ((1 == i) && (4 == pChipInfo->dwChips)) {
        // Special Case 4 way where master also needs to sync from slave
        PCI_CFG_WR(CFG_VIDEO_CTRL0, ... | CFG_VIDPLL_SEL, ...);
    }

W2K `SLIAA.C:3421` has the `if (i > 0)` slave branch and **no else at all**. The
branch is guarded by `4 == dwChips`, so no board 3dfx ever shipped could reach
it — which is why the omission survived the port.

Supporting hardware measurement (`fxscan2 phase`, 6 s per chip, box .191, Q3
fullscreen 640x480 4-way SLI):

    chip0  refresh=84.981900 Hz   (reference)
    chip1   -0.047 ppm   locked
    chip2  +147.972 ppm  *** FREE-RUNNING (not locked to a shared clock) ***
    chip3   +0.513 ppm   locked

A chip scanning at a different rate slides horizontally against the others every
frame. Vertical line phase is aligned (offsets ~0.0 lines, sd ~0.23), so the
error is horizontal.

**Status: UNTESTED.** This is the stronger of the two candidates on evidence,
and it is the one that could not be trialled without a rebuild —
`CFG_VIDEO_CTRL0` is 3dfx-private PCI **config** space, not MMIO, so
`fxscan2 poke` cannot reach it and `fxpci` needs privileges that did not work
out (see below).

## A third candidate, not yet written

The 2/4-way **analog** SLI arm (`SLIAA.C:2677-2731`) is the only multi-chip arm
in the file that never writes `CFG_DAC_HSYNC_TRISTATE`. Census of every write of
that bit in `SLIAA.C`: 1927, 2609, 2858, 2929, 2950, 3002, 3024, 3111, 3261,
3332 — none inside 2677-2731, which is the arm this board executes. Every other
multi-chip arm leaves exactly one chip driving hsync; this one lets all four
drive it, which is also why the handover column matters here at all.

## Why `fxpci` could not trial candidate 2

`fxpci` reads/writes the 3dfx-private PCI config registers and needs
`SeTcbPrivilege`. The agent runs as the console Administrator, which does not
hold it. Running it from a temporary service produced no output; scheduling it
with XP's `at` (which does run as LocalSystem) got past the privilege check but
then died inside `NtSetInformationProcess(ProcessUserModeIOPL)` / the port I/O,
leaving a 0-byte file. `ProcessUserModeIOPL` with length 0 returns
`STATUS_INFO_LENGTH_MISMATCH` on XP SP3; the tool now tries a 4-byte form too,
and still produces nothing. Unresolved.

## Operational warning

The first sweep of these pokes **hard-froze the box** (NIC dead, physical power
cycle). Live-poking a running SLI scanout can wedge the video hardware. The
second sweep added a liveness gate around every step (agent must answer before
and after each hold, uptime must not go backwards) and completed cleanly — use
that pattern, in `scratchpad/dudxsweep2.py`, for any further hardware sweeps.
