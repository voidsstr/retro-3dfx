# Voodoo 5 6000 (Strange God 256MB AGP) — XP driver build-out plan

> **STATUS (2026-09-04). The card is in box `.191`, not `.133`.** Everything in
> this repo that says "the 6000 lives in `.133` P3-DUAL" is stale — it moved on
> 2026-08-31 (`.133` holds a GeForce4 Ti 4600 now). `.191` =
> `NSC-AF6CF7A80BC`, **Athlon 1152 MHz / nForce2, 511 MB, XP SP3**, card
> `PCI\VEN_121A&DEV_0009&SUBSYS_0001121A&REV_01` behind the HiNT
> `PCI\VEN_3388&DEV_0021`, in **128 MB VBIOS mode (32 MB/chip)**. Our stack is
> deployed and all four chips come up (`DDRAW-ENABLED: units=4`,
> `InfSection=3dfxvsV6`, `GL_RENDERER: 3Dfx [retro3dfx 0.5.0]`) —
> `DEPLOY-191-20260904.md`.
>
> **Phases 0–3 COMPLETE** (on `.133`, 2026-08-11/12): 256 MB mode (64 MB/chip)
> + 4-way SLI verified — Q3 61.4 fps, UT99 Glide 39.8 fps, per-TMU texture
> space 15.9→32.4 MB. Results: `V56K-SLI-FINDINGS.md` (§8, §12, §14); 256 MB
> outcome banner atop `V56K-256MB-READINESS.md`; deployed set:
> `optimized/deployed-133-v56k-20260811/`.
>
> **CLOSED 2026-09-04**
> - **"Can this board do 4-way SLI at all?" — YES. VERIFIED ON HARDWARE.**
>   AmigaMerlin 3.1-R11, installed on `.191` alongside our stack, renders
>   4-way SLI with **no skew** (operator-verified on the monitor). The board,
>   the HiNT bridge and the analog combine are all fine. **The scanout defect
>   is OURS.** Stop looking for a hardware excuse.
> - **A benchmark baseline now exists** — a same-box control driver, a 4-point
>   resolution ladder, and a dead-straight single-chip fill-rate line that
>   matches the published record to two decimals. See "Reference line" below.
> - **Exact 6000 subsystem HWID captured:** `SUBSYS_0001121A`, and the repo INF
>   updated to match — both closed 2026-09-04 (`bc41558`); see code-gap item 5.
>
> **OPEN, in priority order**
> 1. **The 4-way scanout skew, now known to be our bug.** Prime suspect:
>    `vidProcCfg` bits 28/29, set by `Miniport/H5/h3modeset.c:667-672`.
>    Confounded with an unresolved refresh-rate difference; the separating
>    2x2 is below and needs no rebuild.
> 2. **A SECOND, INDEPENDENT DEFECT: we are 21% slower than AmigaMerlin in
>    SINGLE-CHIP mode** on the identical box (92.5 vs 116.5 fps @ 640x480x16).
>    No SLI, no scanout and no hardware excuse can explain that one.
> 3. **The 16-bit/32-bit ratio experiment** that decides whether the deficit is
>    inherited from 3dfx's own driver or is ours.
> 4. FIFO-wedge root cause + submission pacing (`V56K-SLI-FINDINGS.md`
>    §21–22) — now the leading candidate for (2) as well.
> 5. ~~`voodoo5-6k.inf` in the repo is STALE and would install this card as a
>    2-way 5500~~ — **CLOSED 2026-09-04** (`bc41558`): the repo INF binds
>    `SUBSYS_0001121A` to a real `[3dfxvsV6]` section with the quad defaults.
>    Residue only: the generic catch-all row (`voodoo5-6k.inf:41`) still points
>    at the 5500 section, and the INF's own comment at `:39-40` still tells the
>    reader to "replace it with the exact HWID captured from Device Manager",
>    which has already been done.
> 6. DX=8 build (`V56K-SLI-FINDINGS.md` §17).
> 7. ICD ARB multitexture (`V56K-SLI-FINDINGS.md` §19–20).

**Target hardware** (arrived; installed in `.133` 2026-08-11, **moved to `.191`
2026-08-31**): "Strange God" AGP 256MB by Anthony ZXCLXIV (zx-c64.com) — a
modern recreation of the unreleased Voodoo 5 6000:

- 4× 3dfx VSA-100 ("Napalm") @ 166 MHz, double-SLI (2-way analog combining of
  two 2-way digital SLI pairs — the mandatory 4-way config, `MINIHWC.C:4280`)
- 256 MB total as 16× 16 MiB SDR SDRAM → **64 MB per chip**
- **Dual VBIOS switch: 128 MB mode (32 MB/chip, stock-driver compatible) vs
  256 MB mode (64 MB/chip — needs driver work, see Phase 3)**
- AGP 1.0 @ 66 MHz, adds 1.5 V signaling tolerance (original was 3.3 V-only);
  6-pin PCIe power input; ~80 W board
- Faithful to the original rev 3700A design incl. the HiNT HB1-SE66 PCI-PCI
  bridge behavior ("same bugs"): chips sit behind the bridge as separate PCI
  devices; the bridge runs hot (needs airflow); original boards had known
  AA/AF bus-corruption instability
- Vendor ships an install manual + CD (known-working driver stack) — capture
  both on arrival

**What we already have (verified 2026-07-18 survey):** the leaked H5/Napalm
tree has real, pervasive 4-chip support (Glide3 `chipCount==4` sample matrix
`GSST.C:1792`, quad SLIAA register programming in all three kernel trees,
6000 external-clock routine `Win9x/DX/MINIVDD/GPIO.C:352`, quad INF UI), and
our Wine/VC6-DDK toolchain provably ships working XP drivers
(`3dfxv5m.sys`/`3dfxv5d.dll`/`glide3x.dll`/`3dfxogl.dll`) on the 5500 fleet
box.

**Code-gap list — REVISED 2026-07-18 after deep verification** (items struck
were disproved by reading the detection paths; see Phase 0 log below):

1. ~~`DetectNumUnits` multi-bus walk needed~~ — WRONG. On real 5500/6000
   boards the slave VSA-100s answer as PCI *functions 1–3 of the master's
   device* (hidden from the OS enumerator — Glide proves it: the "Evilness"
   probe at `H5/MINIHWC/MINIHWC.C:1503-1522` reads chip 1 then chip 3
   vendor/device config regs to detect 2- vs 4-chip boards). The existing
   function-space walk (`W2K/.../SLIAA.C DetectNumUnits`) should therefore
   work as-is on the 6000. A defensive HiNT-gated same-bus device sweep was
   added anyway (see Phase 0 log) in case the Strange God straps differently.
2. **External clock — REAL GAP, NOW PORTED.** The 6000's graphics clock
   comes from an external serial synthesizer bit-banged through GPIO pins
   on the HiNT HB1-SE66 bridge (PCI id 3388h:0021h), config reg C4h —
   Win9x-only code (`Win9x/DX/MINIVDD/GPIO.C`, called at `SLIAA.C:1760-1762`
   when `dwChips==4`). Ported to the W2K miniport in Phase 0.
3. ~~32 MB aperture constants block 256 MB mode~~ — PARTLY WRONG. The
   `MEMBASE0` 32 MB spacing/decodes (`W2K/.../SLIAA.C:91-96`) are the fixed
   per-chip *register* aperture — correct for any memory size. The frame
   buffer BAR (`MEMBASE1`) decode is already `2*AdapterMemorySize`, and the
   probe handles 64 MB/chip (`H3.C:1953-1995`: 4 parts × 128 Mbit = 64 MB;
   total = per-chip × numUnits, uncapped). ~~Remaining real item: Glide's
   `hwcMapBoard` hardcoded 32 MB BAR length (`H5/MINIHWC/MINIHWC.C:1681`)~~ —
   RESOLVED 2026-08-12: that literal is in the DOS/Linux `#else` branch, not
   compiled on Windows (`READINESS` §Change 6), and 256 MB mode is verified
   working on hardware including Glide titles (`V56K-SLI-FINDINGS.md` §12, §14).
4. ~~Glide3 `sliCount = 4; /* doesn't work yet */` branch — `GSST.C:1820`~~ —
   RESOLVED 2026-08-12: 4-way SLI verified live on hardware
   (`SLICTRL chips=4 sli=4 divisor=1 log2=2`, `V56K-SLI-FINDINGS.md` §8) after
   the `V56K-SLICTRL-GUARD` divisor fixes in both Glides
   (`V56K-SLI-FINDINGS.md` §4–5).
5. ~~No 6000 HWID in any INF~~ (6000 shares `DEV_0009`; chip count is detected
   at runtime) — **CLOSED 2026-09-04, capture AND repo work.** The card reports
   `PCI\VEN_121A&DEV_0009&SUBSYS_0001121A&REV_01`, and `bc41558` folded the
   deployed-and-verified INF back into the repo:
   `toolchain-3dfx/dist/.../voodoo5-6k.inf` now binds that exact SUBSYS to
   `3dfxvsV6` (`:28`) and carries a real `[3dfxvsV6]` section with the quad
   defaults (`MaximumDeviceMemoryConfiguration=260`,
   `SSTH3_SLI_AA_CONFIGURATION=5`, `FX_GLIDE_ANALOG_SLI=1`). **Two residues,
   both minor:** (a) the generic catch-all row at `:41` ("3dfx Voodoo5 6000
   (quad VSA-100)" = `3dfxvsV5`, `PCI\VEN_121A&DEV_0009`) still routes any
   DEV_0009 board that misses the SUBSYS match to the 5500 section; (b) the
   Phase 0 note below still says "replace with the exact HWID after Phase 1
   capture", which the capture has made stale. Deployment context:
   `DEPLOY-191-20260904.md` §5.
6. 3dfx Tools don't run on XP → AA/SLI config via registry
   `SSTH3_SLI_AA_CONFIGURATION` tweak values 0,5,6,7,8 (enum at
   `GPCI.C:1420` — 5=4-way SLI, 6=4-way+2xAA, 7=2-way+4xAA, 8=8xAA).
7. Good news found on the way: the W2K miniport build already defines
   `GRAPHICS_CLOCK=166` and builds with `AFIFO=0` (AGP command FIFO not
   compiled in), removing the AGP-FIFO-behind-a-bridge concern.

## Phase 0 — before the card arrives (no hardware needed)

### Done 2026-07-18 (branch `v56k-6000`)

- **External clock ported to the W2K/XP miniport** (`W2K/.../SLIAA.C`,
  `v56k` section): HiNT-bridge finder (3388h:0021h matched by secondary bus
  number), GPIO bit-bang over bridge config reg C4h, and the ICS synthesizer
  PLL search re-implemented in integer 64-bit math (NT kernel code must not
  touch the FPU bare; ntoskrnl's _allmul/_aulldiv are linked). Called from
  `EnableSLIAA` when `dwChips==4`, mirroring the Win9x driver. Failure path
  logs and leaves the clock alone instead of the Win9x `int 3`.
- **Defensive chip-detection fallback** in `DetectNumUnits`: if fewer than
  4 chips found by the stock function walk AND the bus hangs off a HiNT
  bridge, sweep the rest of the secondary bus for VSA-100s (guards against
  the replica strapping chips as separate devices; can never trigger on a
  5500/single-board system).
- **Verified no-regression for the 5500 and Voodoo3 paths:** the clock call
  is `dwChips==4`-gated (5500 SLI enables with dwChips==2 → skipped), and
  the detection sweep requires `IS_NAPALM` AND a HiNT bridge AND fewer than
  4 chips found — no-op on the 5500 (no bridge) and impossible on a Voodoo3
  (IS_NAPALM false, added after realizing the vintage slave-BAR loop is not
  Napalm-gated). One unified miniport/display binary serves all three
  boards via runtime `IS_NAPALM`/`IS_VOODOO3` branches (`H3.H:319-320`);
  V3 boxes install via `voodoo3.inf` (3dfxvs/3dfxvsm names, fleet .124
  flow), V5 boxes via `voodoo5-wfp.inf`/`voodoo5-6k.inf` (3dfxv5d/3dfxv5m
  names) — same code, different service identity, so a future package
  carries 6000 support to every board from one build.
- **Built clean** through the Wine W2K-DDK flow (`/W3 /WX`, build.err empty,
  PE checksums valid): `3dfxvsm.sys` and `3dfxv5m.sys` (198 544 bytes,
  +2.6 KB over stock). Dist package updated: new `3dfxv5m.sys` +
  `voodoo5-6k.inf` staged into `dist/3dfx-napalm-xp-20260716/` and the zip.
  (Note — RESOLVED 2026-08-25: `package_driver.sh` now regenerates
  `3dfxv5m.sys`/`3dfxv5d.dll` from the fresh build and carries over
  `voodoo5-wfp.inf`/`voodoo5-6k.inf`/`glide2x.dll`/`3dfxogl.dll`/
  `DEPLOYMENT.txt`, failing loudly if any is missing; the package
  validation also checks their CopyFiles targets.)
- **`voodoo5-6k.inf`** added (voodoo5-wfp pattern, `3dfxv5m`/`3dfxv5d` pair,
  `DriverVer=07/18/2026`): generic `PCI\VEN_121A&DEV_0009` row for manual
  updrv install; ~~replace with the exact HWID after Phase 1 capture~~ — the
  HWID was captured 2026-09-04 and the INF now carries
  `SUBSYS_0001121A` → `[3dfxvsV6]` (`bc41558`). Only the generic row is left,
  and it still points at the 5500 section.

### Remaining Phase 0 (optional, pre-arrival)

- **CSIM 4-chip harness:** Glide's CSIM detect path fakes a 4-chip board via
  `FX_GLIDE_NUM_CHIPS=4` (`GPCI.C:786-840`). Extend csim-native to
  instantiate multi-chip SLI so band-interleave / AA-sample logic can be
  exercised on Linux first. (csim-native just reached full-triangle render,
  so this is now plausible.)
- Bench prep: verify host box has 3.3 V or 1.5 V AGP slot + spare 6-pin PCIe
  power + airflow over the bridge heatsink (HB1-SE66 runs hot).

## Phase 1 — arrival: characterize before changing anything

- Install per Anthony's manual/CD first (his known-good stack, 128 MB BIOS
  mode) → proves card + host are healthy; that stack is our rollback.
- Capture: full PnP tree (bridge vendor/device ID, the four VSA-100
  instances, subsystem IDs), BAR sizes in **both** BIOS modes, both VBIOS
  images dumped and archived, Device Manager + registry state of the working
  vendor stack. A Linux live-boot `lspci -vvvxxx -tv` dump is the gold copy.
- Never reflash the card's BIOS; the dual-BIOS switch is our only mode lever.

## Phase 2 — our stack in 128 MB mode (32 MB/chip = hardened path)

1. INF with real HWID; install via updrv.exe with existing backup/rollback.
2. Bring-up ladder: single-chip (chip 0 only, acts like ¼ of a 5500) →
   2-way digital SLI → 4-way (analog combine + external clock). 2D desktop
   first, then Glide, then the ICD (Q3 pipeline as on the 5500), then D3D.
3. AA ladder via registry tweak: 0 → 5 → 6 → 7 → 8. Expect the hardware's
   own AA/AF instability; treat 8x as stretch goal.
4. Debug `GSST.C:1820` and any scanout/franken-stack issues (check
   `InstalledDisplayDrivers=3dfxv5d` — same trap as DEBUG-LOG.md).

## Phase 3 — 256 MB mode (64 MB/chip) — **DONE 2026-08-12**

- ~~Flip BIOS switch; verify probe reports 64 MB/chip / 256 MB total~~ —
  done: `HardwareInformation.MemorySize=0x10000000`, `Retro3dfxSliUnits=4`,
  Q3 61.4 fps, no corruption (`V56K-SLI-FINDINGS.md` §12). The >32 MB
  texture-addressing audit concluded: the shipped plain mask is CORRECT and
  the speculative munge hardening caused reboots — see the outcome banner atop
  `V56K-256MB-READINESS.md` before re-opening any of it.
- 256 MB is now the default documented config (benchmarks in
  `V56K-SLI-FINDINGS.md` §14). Stability envelope: cooldowns between flat-out
  timedemos, one fullscreen 3D app at a time, `bench-safe.py` only
  (`V56K-SLI-FINDINGS.md` §11–§13, §18, §22).

## Open on `.191` (2026-09-04) — TWO independent driver defects

Both were measured in one session on one box with **AmigaMerlin 3.1-R11
installed alongside our stack as the control**, so every variable except the
driver is pinned: same card, host, game, demo, resolution and colour depth.
They are separate bugs and neither explains the other. Raw log: FINDINGS.md,
the 2026-09-04 entries named at the end of each part below.

### Reference line — what this board does in a working driver (VERIFIED)

Q3 1.32c `demo four`, 16-bit, vsync off (`FX_GLIDE_SWAPINTERVAL=0` **and**
`r_swapInterval 0` — the Glide env var alone does not reach the engine's own
swap logic), AmigaMerlin 3.1-R11, box `.191`.
Driver: `tools/v56k/trials/ambench.py`.

| resolution | 1 chip | 4-way SLI | SLI scaling |
|---|--:|--:|--:|
| 640x480   | 116.5 | 152.6 | 1.31x |
| 1024x768  |  55.2 | 147.1 | **2.66x** |
| 1280x1024 |  35.1 | 119.4 | **3.40x** |
| 1600x1200 |  24.7 |  71.9 | 2.91x |

- **Single-chip is a perfectly straight fill-rate line** — 19.9 / 19.8 /
  19.7 ms per megapixel across the 640→1024→1280→1600 intervals. Three
  independent intervals agreeing to 1%. This is the cleanest calibration of
  this board we have, and the line any future regression is measured against.
- **It matches the published record exactly.** x86-secret (2005, real
  prototype, Athlon XP 2800+, 32-bit) measured 2.66x at 1024x768; we measure
  2.66x. GamersNexus (2023, same board model) measured 3.46x at their heavier
  settings; our equivalent fill-bound point, 1280x1024, gives 3.40x.
- **Any Q3 number at or below 1024x768 on this host is a CPU benchmark, not a
  card benchmark.** The 4-way ladder is flat at 147–153 fps from 512x384 to
  1024x768 (512x384 is actually *slower* than 640x480, 147.6 vs 152.6); fitting
  640→1024 gives `t = 6.41 ms + 0.50 ms/Mpx`, a host ceiling of **~156 fps**.
  Bench the 6000 at 1280x1024/1600x1200, or measure nothing.
- **OPEN, low priority:** 4-way scaling *peaks* at 1280x1024 (3.40x) and falls
  back to 2.91x at 1600x1200 while single-chip stays linear (19.7 ms/Mpx). Not
  VRAM — 1600x1200x16 front+back+depth is ~2.9 MB of the 32 MB per chip. A real
  4-way-only ceiling above ~1.3 Mpx, unexplained.

FINDINGS.md, 2026-09-04 entries "the 6000 benchmarks EXACTLY on published data"
and "the V5 6000 is CPU-bound to 1024x768".

### Defect 1 — the 4-way scanout skew. It is OUR bug.

Renders perfectly, **scans out** wrong: horizontal strips displaced sideways in
any multi-chip config. Single-chip is clean.

**No software can see this bug — proven, not inferred.** `tools/v56k/sligrid.c`
read back its own front buffer while the monitor was visibly skewed and
reported `0 of 307200 pixels differ (0.0000%)`. Reads through the master's BAR1
are SLI-gathered in hardware (`CFG_SLI_RD_EN` is set on every chip), so the card
reassembles a flawless image for any reader. **Do not build another
screenshot / LFB / pixel-diff detector.** The only sensors are per-chip REGISTER
state and an eye on the monitor.

**The complete register diff, working vs broken, same board, same 4-way config**
(`fxscan2 dump` in both stacks):

| register | AmigaMerlin (CORRECT) | ours (SKEWED) |
|---|---|---|
| `vidProcCfg` | `03E60101` | `33E60101` |
| `pllCtrl0`   | `0000D137` | `0000B31F` |

`vidScreenSize`, `vidDesktopStride`, `vidDesktopStart`, `vidOvlEndCoord`,
`lfbMemoryConfig`, `vidOverlayDudx` and `vidOvlDudxOffSrcW` are identical
between the stacks and across all four chips in both.

**Prime suspect: `vidProcCfg` bits 28 and 29** (the diff is exactly
`0x30000000`), decoded against the table at `Displays/H5/H3DEFS.H:1174-1226`:

- BIT(29) is `SST_OVERLAY_EACH_VSYNC`. **No copy of `H3DEFS.H` in this tree
  names BIT(28)** — a consistent gap between `SST_CURSOR_EN` (27) and 29.
  An undocumented bit.
- They are set by *3dfx's own code*, `Miniport/H5/h3modeset.c:667-672`:
  `// Set Bit 28 and 29 on Napalm boards / // This fixes a problem we were
  seeing with high-res modes in a heated environment.` — an **erratum
  workaround**, i.e. exactly the class of change that shifts when a chip
  latches its scanout data. Note the commented-out `66 == PciSpeed` gate:
  this was once conditional on a 66 MHz PCI bus and someone widened it to
  every Napalm board.
- **Not sufficient on their own.** `IS_NAPALM` is `(0x06 <= PCIDeviceID)`
  (`Miniport/H5/H3.H:320`) and the 5500 is the same `DEV_0009`, so `.143` sets
  these bits too and its 2-way SLI is clean. A scanout-timing perturbation that
  2 chips absorb and 4 (two behind a HiNT bridge) do not is consistent with
  both observations.
- Directly pokeable at **IO offset `0x05C`** — no miniport rebuild needed,
  which matters (see "Operational rules" below).

**Unresolved confound — refresh rate.** `pllCtrl0` decodes with the VSA-100 PLL
formula `f = 14.31818 * (N+2) / ((M+2) * 2^K)`, `N=[15:8] M=[7:2] K=[1:0]`:

| | `pllCtrl0` | N / M / K | pixel clock | VESA 640x480 |
|---|---|---|--:|---|
| AmigaMerlin | `0000D137` | 209 / 13 / 3 | 25.176 MHz | 25.175 = **60 Hz** |
| ours        | `0000B31F` | 179 /  7 / 3 | 35.994 MHz | 36.000 = **85 Hz** |

Three-decimal agreement, so this is certain, not inferred.

**Where the 60 and the 85 came from — the gap is OURS, not the drivers'.** The
benchmark harness `tools/v56k/trials/ambench.py` pins refresh
(`FX_GLIDE_REFRESH=60` **and** `+set r_displayRefresh 60`), but the register
dumps were NOT taken under it: they came from the trial scripts
(`trial_setup.py`, `ringrun.py`, `ringtrial.py`), none of which sets a refresh
at all, so those runs took the ICD's own per-resolution default of 85 Hz. The
AmigaMerlin dump was taken while `FX_GLIDE_REFRESH` had been forced to 60 to
stop the monitor going out of range. **So 60-vs-85 is a configuration
difference this session introduced between the two captures, not an intrinsic
property of either driver.** Any future capture must pin refresh explicitly or
the comparison is worthless.

**Which makes this a two-variable comparison, not an A/B.** The erratum bits
*and* the pixel clock (by 1.43x) both moved, and "bits clear" was only ever
observed under a *different driver*. Refresh is itself a scanout-timing
variable: at 85 Hz each chip has 30% less time per pixel, so less margin for
inter-chip skew. Neither cause is isolated.

**NEXT STEP — the 2x2. Every cell is reachable by poking IO `0x05C` and setting
the refresh; no miniport rebuild:**

| | bits 28/29 SET | bits 28/29 CLEAR |
|---|---|---|
| **85 Hz** | known: **SKEWED** | ? |
| **60 Hz** | ? | AmigaMerlin-equivalent — **NOT RUN on our driver** |

If "60 Hz + bits set" is clean, refresh is the cause and the erratum bits are
innocent. If "85 Hz + bits clear" is clean, the bits are the cause.

**Second live lever, value still unknown:** `cfgSliAaMisc[8:0]`
(`vga_vsync_offset` = pixels[2:0] | chars[5:3] | hxtra[8:6]) is the **only
inter-chip horizontal alignment knob in the whole stack**. Hardware reads
master `0x800` (0 px), slaves `0x827` (**39 px ahead**); zeroing the slaves
visibly MOVED the bands, so the register is live and only its value is in
question. Documented candidates are 31 / 39 / 47 px — `SLIAA.C:2489-2515`
Case A (which our config takes) admits its own number is a fudge, and the
Case B else-branch ("Run slave 8 clocks ahead") uses `chars = 5` = 47 px.
Sweep with `sligrid --poke <chip>:AC=<val>`.

> **DANGER — `pixels=7, chars=3` (31 px, `cfgSliAaMisc = 0x81F`) HARD-FREEZES
> the board.** NIC dead, no route to host, physical power cycle. That is the
> `vga_crtc_fast` bug 3dfx's own comment at `SLIAA.C:2489-2494` describes,
> confirmed on silicon — their `chars=4` bump is a genuine, necessary
> workaround, not a mistake. **Never program pixels=7 with chars=3.**
> `tools/v56k/trials/vsyncsweep.py` skips it permanently. Values that ran
> without wedging: 7, 15, 23 px and the 39 px default.

**Falsified on hardware — do NOT re-propose.** Each of these cost at least one
research pass, and several were written as patches before being killed:

| candidate | verdict |
|---|---|
| "the slaves never get `vidScreenSize`" (`MINIHWC.C:4339`) | **FALSIFIED** — in-game all four chips agree; patch written then REVERTED. (The complaint in that comment is real and its workaround is broken, but it is not this bug.) |
| slave Y-origin (`miscInit0`) diverges from master | **FALSIFIED — BY DESIGN.** Forcing chips 1-3 to the master's `077C0000` BLANKS THE SCREEN. "Slave register != master register" is not automatically a bug on this hardware. |
| master `CFG_VIDPLL_SEL`, the 4-way branch W2K dropped (`MINIVDD/SLIAA.C:1690`) | **FALSIFIED** — poking bit 11 gives NO PICTURE AT ALL: per Databook 3.4.9 the master's `SYNC_CLK_IN`/`SYNC_CLK_FB` are grounded. `optimized/v56k-sli-scanout-candidates/02-master-vidpll-sel.patch` is retained as EVIDENCE, not as a fix. |
| "chip2 free-runs at +148 ppm" clock theory | **DEAD** — re-measured from a clean boot with the skew fully reproduced: chips 1/2/3 at +0.000/+0.093/+0.047 ppm, all locked, still skewed. Phase is not a usable proxy metric. |
| SLI row/column masks are wrong | **FALSIFIED** — the driver computes exactly the shape in 3dfx's own `H5/DOCS/Video SLI AA Configs.xls` (`SLIAA.C:2677-2724`). |
| the analog arm never tristates hsync | **FALSIFIED** — that sheet's header assumes `dac_hsync_float = 0`, so the absence of the write is correct. |
| SLI band height should be 16 | **FALSIFIED — 16 REBOOTS THE BOX.** 8 is correct at 640x480: group = band x chips must divide the screen height (480/32 = 15 exact; 480/64 = 7.5). This also explains Win9x's 4-chip band halving (`DDFXS32.C:1272`). |
| `vidOverlayDudx` hsync handover (Napalm r1.13 s11.1.21) | **live lever, not the fix** — poking it visibly changes the artefact, but eight configurations (0/160/320/480/639 uniform, staggered, master-only, slaves-only) at 640x480 4-way were ALL still skewed. |
| `SSTH3_SLI_AA_CONFIGURATION` can select 2-way on this board | **FALSE** — no `case 2`/`case 5` at `GPCI.C:1446`, `sliCount` forced to 4 at `GSST.C:1828`, and `EnableSLIAA` refuses `dwChips != numUnits` (`SLIAA.C:3510`). 4-way or single-chip, nothing else. |
| `FX_GLIDE_ANALOG_SLI=0` selects digital | **FALSE** — re-forced unconditionally (`GSST.C:2053`, `MINIHWC.C:4277`). Analog was active in every skewed trial. |
| `SSTH3_VIDEO_REFRESH_OPTIMIZATION` | **no-op** — zero references in the W2K tree; Win9x MiniVDD only. |

FINDINGS.md, 2026-09-04 entries "the skew bits are NAMED IN 3dfx's OWN CODE",
"PROOF no software can see this bug, and the clock theory is DEAD", "A scanout
flight recorder for the GLIDE path", and "how to SEE per-chip state".

### Defect 2 — we are 21% slower than AmigaMerlin ON ONE CHIP

Q3 1.32c `demo four`, 640x480x16, vsync off, box `.191`, ours = H5 +
SGL ICD `retro3dfx 0.5.0`:

| config | AmigaMerlin 3.1-R11 | ours | gap |
|---|--:|--:|--:|
| single chip | 116.5 | 92.5 | **-21%** |
| 4-way SLI | 151.5 | 105.0 | **-31%** |
| SLI scaling | 1.30x | 1.14x | — |

*This is the **paired** comparison run — both columns measured back-to-back in
one session. The separate 8-point resolution sweep (Reference line, above)
measured AmigaMerlin 4-way at **152.6**; the two agree to 0.7%, inside the ~3%
run-to-run noise. Quote the paired pair for the driver delta; never mix the two.*

**The single-chip row is the finding.** It cannot be a SLI bug, a scanout bug
or a hardware excuse — one chip, one driver difference, 21% down. The deficit
being roughly equal in both configurations reads as **per-chip submission-path
cost**, which points straight at the open FIFO-wedge / submission-pacing items
(`V56K-SLI-FINDINGS.md` §21–22) rather than at anything new. *Caveat:* if the
scanout defect carries a performance cost of its own, part of the 4-way -31%
may be a symptom of it. The single-chip -21% is clean.

**And it is entirely recoverable in software.** AnandTech's Oct-2000 Q3
640x480x16 CPU ladder (Athlon 800 = 128, 900 = 137, 1.0 GHz = 144, 1.2 GHz =
162 fps, "1% per MHz"; control: 1024x768x32 flat at 83 fps for every CPU)
interpolates to ~158 fps at our 1152 MHz, and our own fit gives ~156 fps.
**AmigaMerlin's 152.6 (sweep) is ~96% of that ceiling; ours at 105.0 is 66% of
it.** AmigaMerlin is at the wall and cannot go faster on this host — we are
nowhere near it.

Automated pass/fail needs no eyes: whether the timedemo **completes and prints
an fps line** is a sufficient health check, since a driver that wedges the card
never gets there. Both rows above were produced with nobody watching the screen.

FINDINGS.md, 2026-09-04 entry "OUR DRIVER IS 21% SLOWER THAN AMIGAMERLIN ON ONE
CHIP, AND THAT IS A SECOND BUG".

### The experiment that says whose bug the 21% is — RUN THIS NEXT

VoodooExtreme's Dec-2000 review is the **only published test of our driver's
actual code lineage** (3dfx's own V5-6000 reference driver). At 1024x768 it
reports **88.4 fps at 16-bit vs 86.1 at 32-bit — a 1.03x gain for halving the
colour depth** — and the reviewer calls it out as a driver bug ("performance in
16-bit color was a bit lackluster... most likely a driver issue"), noting the
5500 shows it too. Community drivers on the same silicon get a proper **1.20x**
(VoodooAlert, 155.1 vs 128.7).

**So: run our driver at 16-bit and again at 32-bit and take the ratio.**

| result | conclusion |
|---|---|
| ratio ≈ **1.0x** | we have INHERITED a documented 3dfx defect — named target, and the 21% has a lineage |
| ratio ≈ **1.2x** | the 21% is OURS and lives somewhere else (submission path) |

Pure fps, no visual verdict, fully automated — but it needs our stack
reinstalled on `.191` (AmigaMerlin is currently the installed display driver;
rollback set at `C:\RETRO_AGENT\am-rollback\`). **HYPOTHESIS to hold while
running it:** FINDINGS records 640x480 as the intended resolution, but this
host is CPU-bound below 1152x864, so a depth-ratio measured at 640x480 may be
flattened by the CPU wall — run 1280x1024 as well, where the card is the limit
and VoodooExtreme's own comparison point (1024x768) sits closer.

### Instrumentation and operational rules (all learned the hard way)

- **`tools/v56k/fxscan2.c`** — `dump` / `diff` / `phase` / `poke` / `ring`.
  Per-chip registers from user mode with **no driver rebuild**, via
  `HWCEXT_GET_SLAVE_REGS` (0x19, `HWCEXT.C:2732`), which the SHIPPING
  `3dfxv5d.dll` already answers; it returns four register windows mapped
  **read/write** into the calling process.
  **Sequencing trap:** send `GETLINEARADDR` (0x03) FIRST, then
  `GET_SLAVE_REGS` (0x19). Sending `HWCEXT_ALLOCCONTEXT` first takes the
  "GLIDESTATE already exists" branch (`HWCEXT.C:785-825`) and you get four
  zeros. Neither call is exclusive-gated, so this runs against a live
  fullscreen game.
- **`fxscan2 ring <secs> <interval_ms> <outfile>`** — the Glide-path equivalent
  of the display driver's registry flight recorder (the display driver is not
  on this path: a Glide fullscreen app makes it release the hardware). Logs
  every watched register on all four chips on change, plus a per-second `HB`
  heartbeat of `vidCurrentLine`. **Every line is flushed**, so a wedge still
  leaves the last state on disk. Start it BEFORE launching the game.
  **A static dump cannot tell "correctly programmed" from "stale but
  coincidentally equal"** — the ring caught the slaves sitting at the previous
  game's geometry, which is what made the earlier snapshot misleading.
- **`tools/v56k/sligrid.c`** — bit-exact pattern + front-buffer readback +
  `--pci` / `--poke fn:OFF=VAL`. Must be a Glide app: `HWCEXT_PCI_OP` is gated
  on the caller holding `HWC_EXCLUSIVE` (`HWCEXT.C:2409`).
- **`tools/v56k/trials/ambench.py`** — the automated timedemo harness behind
  every number above; `vsyncsweep.py`, `dudxsweep2.py` are the gated sweeps.
- **LIVENESS-GATE EVERY HARDWARE SWEEP.** THREE incidents in one session, all
  from poking a live scanout, and they are separate events — do not conflate
  them:
  1. an **ungated** 8-step `vidOverlayDudx` poke sweep → **hard freeze**
     (NIC dead, physical power cycle);
  2. the 31 px `vga_vsync_offset` step → **hard freeze** — but this one WAS
     liveness-gated and the gate named the failing step exactly, which is how
     the `vga_crtc_fast` erratum got confirmed on silicon;
  3. SLI band height 16 → **self-recovering reboot**.

  **The step that breaks the gate IS the finding** — that is the whole argument
  for the gate. None of the three came from `ambench.py`.
- **Prefer experiments that need no miniport rebuild.** The toolchain restored
  on this dev host reported `Toolchain ready` and exit 0 yet produced an
  unusable Wine (`could not load kernel32.dll, c0000135`) because `$HOME`'s
  default ACL clamped execute off the whole tree; fix and the two follow-on
  traps are in FINDINGS.md, 2026-09-04 entry "`setup-toolchain.sh` succeeds and
  produces an unusable Wine".
- **Installing the AmigaMerlin control:** extract `amigamerlin_3.1_r11.exe`
  (7-Zip; the payload is a plain 7z at offset 78848) and install
  `driver2k/3dfxvs.inf` headlessly with `tools/drvupd.c` against
  `PCI\VEN_121A&DEV_0009&SUBSYS_0001121A` — its INF has a dedicated `3dfxvsV6`
  section for exactly that HWID. Two dialogs must be clicked through even with
  the signing policy relaxed. Our files coexist (different names:
  `3dfxv5d.dll`/`3dfxv5m.sys` vs its `3dfxvs.dll`/`3dfxvsm.sys`); full rollback
  set at `C:\RETRO_AGENT\am-rollback\`.
  **Trap:** its INF writes `FX_GLIDE_REFRESH = 75` into `...\Device0\Glide`,
  which overrides the per-resolution refresh for EVERY mode and put the monitor
  out of range. Delete it (or set 60) before running anything.
- **Benchmark hygiene for this card:** cross-review variance in the published
  literature is a measured ±20% (AnandTech 79.5 vs PC Perspective 71.3 for the
  identical card/CPU/demo/resolution), so only **within-article ratios** are
  trustworthy. Excluded at source, with cause: the gaming2k/tredfx "Athlon
  1 GHz 6000" numbers (denied by 3dfx's Dave Barron, never archived —
  **fabricated, do not chase**), the I/ITSEC 1600x1200 demo (fps counter taped
  over), Hartware.de (Fast vs High Quality presets, not like-for-like),
  HotHardware (demo unstated, charts internally inconsistent), PCGH 2017
  (card locked up, no numbers), ModLabs (four condition mismatches incl. a PCI
  card). **Nothing built from the leaked H5/Napalm W2K source appears in any
  benchmark literature anywhere — our numbers are the first of their kind, and
  AmigaMerlin on the same box is the only valid control.**

## Benchmark-stack readiness (added 2026-07-18, works on 5500 today)

Goal: benchmark OpenGL games, Glide games, D3D games, and 3DMark. Per-API
state after the glide2x build:

| API | Component | State |
|---|---|---|
| OpenGL | `3dfxogl.dll` ICD (ours, 0.2.0 then; **`retro3dfx 0.5.0` is what is deployed on `.191` as of 2026-09-04**) | Built, benchmarked (Q3). Now INF-registered: `voodoo5-6k.inf` ships it + writes the vintage `OpenGLdrivers\3dfx` registry keys. |
| Glide 3.x | `glide3x.dll` | Built, verified (96 exports), shipped. |
| Glide 2.x | `glide2x.dll` | **NEW: built** from `H5/GLIDE/SRC` (133 exports, Napalm packet-FIFO config, minihwc-linked). Most Glide games (Unreal/UT, NFS, Diablo II…) need this, not glide3x. Shipped + INF CopyFiles. |
| Direct3D | D3D HAL inside `3dfxv5d.dll` | Compiled in (d3/d6/d7 HAL + SIMD T&L asm objs verified in objfre). DX6/DX7-class caps; DX8 DDI negotiates but reports DX7 caps (hardware has no shaders). **Since exercised hard on real hardware** — CS
Direct3D, UT2004 and 3DMark all run on `.143`; the bugs that found are logged in
`VINTAGE-FIXES.md`. |
| DirectDraw | `3dfxv5d.dll` | Working (it's the desktop path on .143). |

3DMark expectations: 3DMark99/2000 (DX6/DX7) should run fully; 3DMark2001
runs the non-shader subset (no HW T&L, no pixel/vertex shader tests —
that's authentic VSA-100 behavior, same as period reviews). Nature test
will be absent. Glide benchmarking: UT '99 Glide mode or NFS:Porsche;
`FX_GLIDE_*` env vars work for AA/SLI experiments.

Build notes (learned): Glide2 and Glide3 stage *different* `glide.h` into
`H5\include` — the include dir is per-API state. To rebuild glide2x:
overlay `GLIDE/SRC/{GLIDE,GLIDESYS,GLIDEUTL}.H` into `H5/INCLUDE`, nmake
in `GLIDE/SRC` with the glide3 env but `FX_HW_PROJECTS=glide`, then
restore the Glide3 headers (backup pattern in the 2026-07-18 session).
Splash: glide2x is built `GLIDE_SPLASH` but we don't ship `3dfxSpl2.dll`;
Glide2 LoadLibrary's it and continues without — verify no-splash startup
on hardware.

Remaining to verify on .143 (5500) — **2 and 3 are DONE**, struck but kept for
the record:
1. Install the updated package; confirm ICD registry keys land and
   GL_RENDERER reports our ICD after reboot.
2. ~~Run a D3D title / 3DMark2000 — first real exercise of the D3D HAL.~~ —
   DONE. The D3D HAL is heavily exercised: CS Direct3D (which now benches
   FASTER than GL), UT2004 and 3DMark, with the resulting fixes logged in
   `VINTAGE-FIXES.md`.
3. ~~Run a Glide2 game or `test05`-style diag against glide2x.dll.~~ — DONE.
   UT99 on the Glide renderer runs on our `glide2x` — 39.8 fps at 640x480x16
   on the 6000 (`V56K-SLI-FINDINGS.md` §14).

## Phase 4 — productize

- `dist/3dfx-napalm-xp-*` package: add voodoo5-6k.inf + docs; update the
  retro-agent `deploy-3dfx-driver` skill HWID gate; quality/perf benchmark
  set (Q3 like the 5500); port ICD 0.2.0+ optimizations.

**Safety rails:** the card is rare (~$1500 class) — vendor driver stack and
both BIOS images archived before our first install; signing-policy + driver
backup via existing INSTALL.bat mechanism; watch bridge temperature.
Added 2026-09-04, both learned by breaking the box: **liveness-gate every
register sweep** (two hard freezes needing a physical power cycle, plus one
self-recovering reboot, in a single session), and **never program
`vga_vsync_offset` pixels=7 with chars=3** — it wedges the board hard.
