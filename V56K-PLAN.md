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

**Known code gaps for 4-chip (file:line, from the 2026-07-18 deep survey):**

1. W2K miniport `DetectNumUnits` uses the single-device function-space walk;
   the multi-bus walk needed behind the HiNT bridge is `#if 0`-disabled —
   `H5/W2K/Src/Video/Miniport/H5/SLIAA.C:480-578`.
2. No HiNT-specific handling anywhere; bridges only generically classified
   (`SWLIBS/NEWPCI/PCILIB/FXINFO.C:82`).
3. 32 MB-per-chip aperture constants hardcoded: `H5/MINIHWC/MINIHWC.C:1650,
   1681`, `H5/MINIHWC/DOS_MODE.C:256,282` — blocks 256 MB (64 MB/chip) mode.
   Memory *probe* itself is fine (`H5/CINIT/H3CINIT.C:344` handles up to
   128 Mbit parts; `H3.C:2058` total = per-chip × numUnits, uncapped).
4. External clock (required when `dwChips==4`) confirmed only in the Win9x
   MiniVDD — verify/port into the W2K miniport.
5. Glide3 `sliCount = 4; /* doesn't work yet */` branch — `GSST.C:1820`.
6. No 6000 HWID in any INF (6000 has no unique PCI ID; it's `DEV_0009` +
   chip-count detection; subsystem ID of the Strange God TBD on arrival).
7. 3dfx Tools don't run on XP → AA/SLI config via registry
   `SSTH3_SLI_AA_CONFIGURATION` tweak values 0,5,6,7,8 (enum at
   `GPCI.C:1420` — 5=4-way SLI, 6=4-way+2xAA, 7=2-way+4xAA, 8=8xAA).

## Phase 0 — before the card arrives (no hardware needed)

- **CSIM 4-chip harness:** Glide's CSIM detect path already fakes a 4-chip
  board via `FX_GLIDE_NUM_CHIPS=4` (`GPCI.C:786-840`). Extend csim-native to
  instantiate multi-chip SLI so band-interleave / AA-sample logic can be
  exercised on Linux before touching the real card.
- **Desk-fix the miniport:** enable + correct the multi-bus `DetectNumUnits`
  path; add master/slave instance policy (bind display stack to master chip
  only); audit W2K SLIAA for the external-clock equivalent and port from
  `GPIO.C:352` if absent.
- **Parameterize the 32 MB/chip constants** off the probed per-chip size.
- **INF skeleton** `voodoo5-6k.inf` on the voodoo5-wfp.inf pattern
  (3dfxv5m/3dfxv5d pair), HWID placeholder pending the real subsystem ID.
- Bench prep: verify host box has 3.3 V or 1.5 V AGP slot + spare 6-pin PCIe
  power + airflow over the bridge heatsink.

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
