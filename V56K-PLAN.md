# Voodoo 5 6000 (Strange God 256MB AGP) — XP driver build-out plan

**Target hardware (being shipped to us):** "Strange God" AGP 256MB by Anthony
ZXCLXIV (zx-c64.com) — a modern recreation of the unreleased Voodoo 5 6000:

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
   total = per-chip × numUnits, uncapped). **Remaining real item:** Glide's
   `hwcMapBoard` maps BARs with a hardcoded 32 MB length on Napalm
   (`H5/MINIHWC/MINIHWC.C:1681`) — must scale for 64 MB/chip in 256 MB BIOS
   mode (Phase 3; file currently carries another session's in-flight ICD
   edits, so deferred deliberately).
4. Glide3 `sliCount = 4; /* doesn't work yet */` branch — `GSST.C:1820`
   (hardware-debug phase).
5. No 6000 HWID in any INF (6000 shares `DEV_0009`; chip count is detected
   at runtime). → `voodoo5-6k.inf` added in Phase 0; exact Strange God
   subsystem ID to be filled in at Phase 1.
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
  (Note: `package_driver.sh` does NOT know about the hand-added
  `3dfxv5m/3dfxv5d/voodoo5-wfp/6k` files — rerunning it wipes them; update
  the script before the next full repackage.)
- **`voodoo5-6k.inf`** added (voodoo5-wfp pattern, `3dfxv5m`/`3dfxv5d` pair,
  `DriverVer=07/18/2026`): generic `PCI\VEN_121A&DEV_0009` row for manual
  updrv install; replace with the exact HWID after Phase 1 capture.

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

## Phase 3 — 256 MB mode (64 MB/chip)

- Flip BIOS switch; verify probe reports 64 MB/chip / 256 MB total; exercise
  the parameterized aperture layout; audit texture/buffer offset fields for
  >32 MB addressing limits in hwcAllocBuffers and Glide texture download.
- Soak + benchmark; if stable, make 256 MB the default documented config.

## Phase 4 — productize

- `dist/3dfx-napalm-xp-*` package: add voodoo5-6k.inf + docs; update the
  retro-agent `deploy-3dfx-driver` skill HWID gate; quality/perf benchmark
  set (Q3 like the 5500); port ICD 0.2.0+ optimizations.

**Safety rails:** the card is rare (~$1500 class) — vendor driver stack and
both BIOS images archived before our first install; signing-policy + driver
backup via existing INSTALL.bat mechanism; watch bridge temperature.
