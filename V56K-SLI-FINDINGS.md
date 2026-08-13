# Voodoo 5 6000 — multi-chip / 4-way SLI findings (2026-08-11)

Box: **192.168.1.133 "P3-DUAL"** — dual Pentium III 700, 1 GB, XP SP3, Voodoo 5 6000
(4× VSA-100 behind a HiNT HB1-SE66 bridge, 128 MB BIOS mode = 32 MB/chip).
Stack: `3dfxv5m.sys` + `3dfxv5d.dll` + `glide3x.dll`/`glide2x.dll` + `3dfxogl.dll` (ICD 0.5.0).

---

## 1. Multi-chip IS working — but nothing configures SLI explicitly

Measured with Quake 3 `timedemo four`, 16-bit, quiesced:

| config | 640×480 | 1024×768 | 1280×1024 |
|---|--:|--:|--:|
| default (no SLI env/registry set) | 62.2 | 61.4 | 60.7 |
| `FX_GLIDE_NUM_CHIPS=1` | — | 44.8 | 27.7 |

Forcing a single chip costs **27 %** at 1024×768 and **54 %** at 1280×1024, so by
default more than one chip is rasterising. The flat default curve is CPU/present
bound on the 700 MHz P3, not a fill-rate limit.

**But the configuration surface is empty.** On the live box:

- `HKLM\SYSTEM\CurrentControlSet\Services\3dfxvs\Device0`
  - `SSTH3_SLI_AA_CONFIGURATION` — **ABSENT**
  - `SSTH3_SLI_BAND_HEIGHT`, `DISABLE_SLI`, `ENABLE_AA` — **ABSENT**
  - `…\D3D\QuadChipAASLI`, `…\Glide\QuadChipAASLI` — **ABSENT**
    (the miniport is supposed to write these from `switch(numUnits)`, `H3.C:276-490`,
    `case 4:` at `H3.C:337`)
- The display driver's flight-recorder ring contains **zero**
  `COMPUTE-SLIAA` / `PROMOTE-SLIAA` / `DEMOTE-SLIAA` entries — only
  `DDRAW-ENABLED: units=4`, i.e. detection happened, policy never ran.

So the board is running on whatever the default path gives it, and the documented
4-way analog configuration has never been requested. `09-INCOMPLETE-AND-GAPS.md:89`
is explicit that 4-way SLI "is present but was never productized… the 2-chip paths
are the hardened ones."

### Config gotchas found while tracing this
- The SLI/AA registry values are read as **`REG_SZ`** — a `REG_DWORD` is silently
  ignored (`DDFXNT.C:1675`). Write `"5"`, not `5`.
- `SSTH3_SLI_COMPATIBILITY_SETTINGS` default 0 **demotes to single chip** whenever
  `SST_VIDEO_2X_MODE_EN` is set — precisely the 1280×1024 case (`DDFXNT.C:3642-3660`).
  Set `"2"`.
- The miniport's `H3ZwQueryRegistryValue` searches `<DevNode>\D3D` then `<DevNode>`
  only, so it can **never** see a `Glide\SSTH3_*` value; that subkey is read by Glide.

---

## 2. `FX_GLIDE_NUM_CHIPS=2` hard-wedges the machine (reproducible)

Running Q3 at 1280×1024 with `FX_GLIDE_NUM_CHIPS=2` on this 4-chip board killed the
box outright: no ping, no agent, screen flickering, and it did not come back without
a power cycle. `=1` is safe, `=4`/unset are safe. **Do not use `=2` on this board.**

Mechanism: the env override only narrows *Glide's* view of the board and it does so
**after** the slave chips have been mapped and their PCI config read
(`MINIHWC.C:1534`, and the same pattern at `:1318`/`:1432`). The kernel keeps the
board programmed for 4 units, and 4-way is forced *analog* combining while 2-way is
digital (`MINIHWC.C:4278-4287`) — so the video/SLI path ends up half-programmed.
On the kernel side `SLIAA.C:3480` walks `HwDeviceExtension->numUnits` (4) while the
setup pass only handles `dwChips` (2), leaving units 2-3 touched but unconfigured.

**Planned fix** (not yet applied — kernel side, needs a supervised reboot):
clamp the slave-touch loop to `dwChips`, add an explicit safe-state pass for the
leftover units (clear `SLICTRL`/`AACTRL`, clear the `CFG_INIT_ENABLE` snoop bits,
`SST_POWERDOWN_DAC` so exactly one chip drives video), and — as an interim guard —
refuse the enable when `dwChips != numUnits`, logging `SLIAA-MISMATCH`.
Losing SLI is strictly better than a half-programmed board.

---

## 3. The glide2 machine reset (UT99 Glide) — root cause found

UT99 on its Glide renderer reboots the box under load. glide2 initialises fine
(`grSstOpen`, both TMUs, 16 MB each) and then dies once the demo streams.

**Root cause: glide2 never re-cached the SLAVE MMIO pointers after a remap.**
`GLIDE3/SRC/GSST.C` refreshes `gc->slaveSstRegs[]` / `gc->slaveCRegs[]` alongside the
master registers whenever the board is re-mapped; `GLIDE/SRC/GSST.C` refreshed only
the master set. glide2 sets the slave pointers exactly once (`GPCI.C:1228-1229`), so
after any remap `_grHwFifoPtrSlave()` (`GLIDE/SRC/FIFO.C:960-975`) reads MMIO through
**stale kernel addresses on every make-room** — which resets the machine rather than
faulting. That function also dereferenced the slave pointers with no NULL/range check.

## 4. Latent bug in both Glides: `_grSliCtrl` / `_grEnableSliCtrl`

```c
FxI32 sliChipCountDivisor;                 /* never initialised */
if (gc->chipCount == 2) sliChipCountDivisor = (gc->grPixelSample == 4) ? 2 : 1;
if (gc->chipCount == 4) sliChipCountDivisor = (gc->grPixelSample == 2) ? 2 : 1;
...
while ((0x1UL << log2chipCount) != (gc->chipCount / sliChipCountDivisor))
  log2chipCount++;                         /* never terminates unless power of two */
```
For any chip count other than 2 or 4 the divisor is **stack garbage**; the loop then
spins forever with the command FIFO open (and a divisor of 0 faults). The divisor was
also derived from `grPixelSample` rather than the `sliCount` the buffers were actually
laid out from, so in the 4-chip/2-sample case the hardware is masked 2-way while the
buffers are laid out 4-way and chips render into each other's bands.
(glide2 `GSST.C:3164`; glide3 `GSST.C:3778`, function `_grEnableSliCtrl`.)

---

## 5. What was changed (user-mode only, no reboot to deploy)

All in `3dfx Driver Code/H5/`, built with the reconstructed Wine/VC6 toolchain:

| fix | file | marker |
|---|---|---|
| re-cache slave MMIO pointers after a remap | `GLIDE/SRC/GSST.C` | `V56K-SLAVE-RECACHE` |
| validate slave FIFO pointers before use | `GLIDE/SRC/FIFO.C` | `V56K-SLAVE-GUARD` |
| divisor from `sliCount` + init + power-of-two guard | `GLIDE/SRC/GSST.C`, `GLIDE3/SRC/GSST.C` | `V56K-SLICTRL-GUARD` |
| SLI state diagnostics | both `GSST.C` | `_grSliLog` |

**Diagnostics are opt-in and free when unused:** set `FX_GLIDE_SLI_LOG=C:\sli.log`
to capture `SLAVE-RECACHE`, `SLICTRL`, `SLICTRL-BAIL` and `SLAVE-UNMAPPED` lines.
Unset, each call is a single pointer test.

Artifacts: `glide2x.dll` 258,048 B / 133 exports · `glide3x.dll` 339,968 B / 96
exports — both clean `/WX` builds, ABI unchanged. Rollback is a single file copy from
`optimized/deployed-133-v56k-20260811/`.

---

## 6. Bring-up ladder (next, once the box is back)

1. Deploy the two DLLs (user-mode, no reboot), `FX_GLIDE_SLI_LOG` on.
2. Re-run Q3 at 1280×1024 → confirm no regression, capture `SLICTRL` (this tells us
   the real `chipCount`/`sliCount`/band height for the first time).
3. UT99 Glide under sustained load → the reboot should be gone. If it now *hangs*
   cleanly instead, that is progress: read `SLAVE-UNMAPPED`/`SLICTRL-BAIL`.
4. Only then the kernel side: registry `SSTH3_SLI_AA_CONFIGURATION="5"` (+
   `SSTH3_SLI_COMPATIBILITY_SETTINGS="2"`), the `SLIAA.C` chip-count coherence fix,
   and the `V56K-CLOCK` gate before any 4-way *analog* attempt.

**Hard gate before 4-way analog:** the V5-6000 external clock synthesiser must be
programmed (`SLIAA.C` v56k section, HiNT bridge GPIO). Enabling analog SLI on chips
whose clock was never set is the single highest hardware risk in this plan and is a
credible cause of unattended resets on its own.

---

## 7. D3D "0K vram" — root cause and fix (built, NOT yet deployed)

`DdGetAvailDriverMemory()` (`DDFXNT.C:1569`) initialised `dwTotal = dwFree = 0` and
then returned **`DDHAL_DRIVER_HANDLED` unconditionally**:

- The whole body is gated on `IS_NAPALM && bUseSliExtraLinearHeap`, and
  `bUseSliExtraLinearHeap` comes from the registry value `UseSliExtraLinearHeap`
  (`DDINIT.C:1296`), which defaults to 0 and **is absent on this box** — so the body
  never runs and the function reports 0/0 as authoritative.
- Even inside the body, the `DUAL_CHIP_SLI_2WAY_AA_DISABLED` /
  `QUAD_CHIP_SLI_4WAY_AA_DISABLED` arm has **literally empty branches**
  (`// don't do anything`), and the other arm sets `dwTotal` but has
  `dwFree` **commented out**. Every path yields `dwFree = 0`.

That is exactly what UT99 reports: `D3D Device 0K vram, 0K free`. DirectDraw
therefore believes the board has no video memory and D3D keeps every texture in
system memory — the reason UT-D3D manages ~24 fps on hardware that runs Q3 fine.

**Fix (`V56K-VIDMEM`, `DDFXNT.C`):** only claim the call when there is genuinely
something to hide (the SLI extra linear heap); otherwise return
`DDHAL_DRIVER_NOTHANDLED` and let DirectDraw account the heaps the driver already
gave it. Also sets `dwFree` in the arm where it was commented out, and logs
`DDAVAILMEM: caps=… total=… free=…`. Builds clean (`3dfxvs.dll`, 964,256 B).

### ⚠ Do not deploy the rebuilt display driver yet — it would REGRESS diagnostics

The deployed `3dfxv5d.dll` (969,264 B) was built from the working tree that died with
the OMEN NVMe, and contains **19 `retro3dfx` log strings that our source does not
have** — listed in `optimized/v56k-sli-build-20260811/LOST-INSTRUMENTATION.txt`.
They include the DirectDraw enable-path diagnostics we are actively relying on
(`DDRAW-ENABLED: units=%ld cyMem=%ld slop=%ld ddHeap=%ld` — how we know all four
chips are detected), the full heap dump (`HEAP%ld LIN/RECT`, `HEAPSUM`), the D3D HAL
create-failure paths, and an SLI diagnostic
(`SLICFG: degenerate denominator (units=%ld) -> non-optimal path`).

**RESOLVED 2026-08-11:** that instrumentation has been reconstructed from the deployed
binary's format strings and verified at full 68/68 string parity, so the rebuilt display
driver no longer regresses anything. The vram fix ships with it.
then ship the vram fix together with it. Until then the D3D fix stays source-only;
the Glide fixes (§5) are user-mode and ship independently with no such risk.

---

## 8. HARDWARE-VERIFIED RESULTS (2026-08-12)

Fixes deployed to `.133` (user-mode DLLs, no reboot) and tested.

### 4-way SLI is ALREADY ACTIVE — §1's conclusion was wrong

The first SLI readout ever taken off this board, via the new `FX_GLIDE_SLI_LOG`:

```
Q3 @1024x768 :  SLICTRL chips=4 sli=4 divisor=1 band=5 renderMask=0x60 log2=2
UT @640x480  :  SLICTRL chips=4 sli=4 divisor=1 band=3 renderMask=0x18 log2=2
```

All four VSA-100s are in SLI (`log2=2` ⇒ 4 chips), scanline-band interleaved —
32 lines under Q3, 8 lines under UT. **The absent `SSTH3_SLI_AA_CONFIGURATION` /
`QuadChipAASLI` registry values are optional OVERRIDES, not the configuration**;
the driver computes the config itself. Their absence, and the empty ring, meant
"nobody overrode the default", not "SLI never ran". The `FX_GLIDE_NUM_CHIPS=1`
fps cliff (§1) was the honest signal all along.

### The UT99-Glide machine reset is FIXED

UT99 on the Glide renderer previously took the machine down every time. With the
slave-pointer re-cache deployed it ran **~110 s under sustained load including a
full timedemo, uptime climbing 624 → 839 s, no reboot**, then a second full
benchmark run 897 → 1049 s, also clean. The log caught the mechanism live:

```
SLAVE-RECACHE chips=4 sli=4 s0=0CF40000/0CF20000 s1=0CF80000/0CF60000 s2=0CFC0000/0CFA0000
```

glide2 really does remap mid-run, and all three slave MMIO pointers were
refreshed. Unfixed, those stayed stale and the make-room loop drove MMIO at dead
kernel addresses — which resets the box rather than faulting.

### No performance regression

Q3 @1024×768: **60.9 fps** after the fixes vs **61.4** before (noise).

⚠ **Keep `FX_GLIDE_SLI_LOG` unset for benchmarking.** `_grEnableSliCtrl` runs per
buffer-swap; logging it unconditionally cost two thirds of the frame rate
(61 → 22 fps) and wrote 582 KB. The line is now emitted only when the programmed
config changes (63 bytes/run), but the env var still gates real work.

### UT is CPU-bound, not driver-bound

| UT99 renderer | 640×480 | verdict |
|---|--:|---|
| Glide (native fast path) | **24.4** | renders clean, stable |
| Direct3D | ~24 | renders clean, stable |

Both land on ~24 fps, so the limiter is the 700 MHz Pentium III running
`UTbench.dem` — a 17-player DM-Gothic botmatch — not the driver or the card.
Q3 reaches 61 fps on the same hardware because its engine is far lighter. The
D3D `0K vram` fix (§7) is still correct and worth shipping for texture-heavy D3D
titles, but it will not move UT.

---

## 9. CORRECTION + thermal hypothesis (2026-08-12, later session)

### The UT99-Glide reboot was NOT fixed — §8's claim is withdrawn

§8 reported the UT-Glide machine reset as "fixed and verified" on the strength of
two clean runs. That was over-claimed. Later the same day UT-Glide rebooted the box
on **four consecutive attempts**, including one with the ORIGINAL golden kernel pair
(`optimized/deployed-133-v56k-20260811/`, byte-verified on disk) restored — i.e. the
exact configuration §8 called fixed.

The slave-pointer re-cache (`V56K-SLAVE-RECACHE`) is still a genuine defect fix — the
log proved glide2 remaps mid-run and the pointers really were going stale — but it is
**not sufficient** to make UT-Glide reliable. Treat UT-Glide as INTERMITTENTLY UNSTABLE.

### Then Quake 3 started rebooting the box too — on byte-identical binaries

Q3 had run stably ~15 times across the day at 60-62 fps. Late in the session it began
rebooting the machine within seconds of launch, with the golden display driver,
golden miniport and hardware-verified Glide DLLs all md5-verified on disk. **No driver
change can explain a regression on byte-identical binaries.**

### Leading hypothesis: THERMAL / power, not software

`V56K-PLAN.md` flags exactly this risk for this board: the HiNT HB1-SE66 bridge "runs
hot (needs airflow)", the board draws ~80 W through a 6-pin PCIe input, and the
original design had "known AA/AF bus-corruption instability". The session put dozens
of sustained 3D runs plus ~10 unclean power-cycles through the card over several hours.
A card that is stable when cool and resets under load after prolonged hammering fits
thermal far better than it fits any code path.

**Ranked candidates:**
1. Bridge/VSA-100 temperature after hours of sustained load — needs a physical check
   (heatsink hot to touch, case airflow, fan on the bridge).
2. 6-pin PCIe power delivery under 4-chip load (marginal PSU rail or connector).
3. Filesystem/registry damage accumulated from ~10 unclean shutdowns.

### What was done in response
- All in-flight 256MB changes were neutralised or reverted (see §10) and the box was
  restored to the golden kernel pair + hardware-verified Glides, md5-verified on disk.
- **Hardware testing was STOPPED.** Continuing to hammer a rare board that may be
  overheating is not a reasonable trade.

**Do not flip the 256MB BIOS switch.** 256MB mode raises power and memory-bus load;
attempting it while the board resets under ordinary 128MB load would only confuse a
thermal/power fault with a software one.

## 10. Status of the 256MB change set (all inert on the box)

| change | state | why |
|---|---|---|
| C1 BAR clamp (`H3.C`) | **neutralised, diagnostic kept** | `AccessRanges[MEMBASE_ONE].RangeLength` is not yet valid where it was inserted, so `bar1Len` read 0, the sizing loop left `pow2` at its 4MB floor and the clamp cut each chip 32MB -> 4MB. It must run AFTER the BAR is known, and be observable — `VideoDebugPrint` is compiled out of the free build, which is why it was invisible. Logged as `WOULD-CLAMP`, not applied. |
| C2 AA aperture mask (`SLIAA.C`) | reverted | unverifiable at 32MB/chip; part of the same untested batch |
| C3 glide2 texture munge (`GLIDE/SRC/GTEX.C`) | conditional | `SST_TEXTURE_MUNGE_ADDRESS` is **not** a superset of the mask — it overwrites bit 1 with the relocated bit 25. Now only taken when bit 25 is actually set, i.e. byte-identical to the original mask below 32MB. |
| C4 glide3 texture munge (`GLIDE3/SRC/GTEX.C`) | reverted | same reason; revisit when >32MB addresses can actually be tested |

The C1 analysis remains correct in principle — nothing validates `AdapterMemorySize`
against the BAR that must hold 2x it — and that guard IS required before 256MB. It
simply has to be implemented where the BAR length is known and where its decision can
be read back (the `Retro3dfxSli*` registry pattern works; kernel `VideoDebugPrint` does not).

### Refinement (same day, from the operator): normal gameplay is FINE

Before powering the box down to cool, the operator **played Quake 3 interactively and
it ran fine** — while the automated `timedemo` runs were rebooting the machine.

That is not a contradiction, it is the discriminator:

| load | behaviour |
|---|---|
| interactive gameplay (vsync-limited, ~60-85 fps cap) | **stable** |
| `+set timedemo 1 +demo four`, back-to-back, incl. 1280x1024 | reboots |

A timedemo renders flat out with no frame limiter, and the suite ran three
resolutions back-to-back with no cooldown. Sustained 100% load is exactly what a
marginal thermal/power situation punishes, and it is what this board's own design
notes warn about (HiNT bridge runs hot, ~80W through a 6-pin input).

**Consequences for methodology — the benchmark harness was part of the problem:**
1. Insert a cooldown between runs (>=60s idle at the desktop), never back-to-back.
2. Prefer vsync-limited runs for *stability* testing; keep unlimited timedemos for
   *performance* numbers only, one at a time.
3. Treat a reboot during a timedemo as a THERMAL/POWER datum first and a software
   datum second — the reverse of what this session assumed for hours.
4. Re-establish a cold baseline before attributing any reboot to a code change; a
   drifting baseline invalidated several hours of A/B work here.

**This also re-opens §9's other conclusions.** Several "regressions" attributed to
in-flight driver changes (the 256MB clamp, the texture munge) were measured against a
baseline that was itself degrading. The clamp really was wrong (it cut 32MB->4MB, that
is provable from the code), but the *evidence* used to convict it — "UT rebooted after
deploying it" — was not sound, because the golden baseline rebooted too. Re-test both
from a cold box before drawing conclusions.

---

## 11. Thermal envelope, measured from a cold box (2026-08-12)

After the operator powered the box down to cool and restarted it, the whole matrix
was re-run through `toolchain-3dfx/build/bench-safe.py` (cooldowns enforced, stops on
first reboot). Binaries: golden kernel pair + hardware-verified Glides, md5-verified.

| test | result |
|---|---|
| 1024x768 vsync-limited, single, from cold | **60.0 fps**, no reboot |
| 1024x768 unlimited, single, from cold | **60.8 fps**, no reboot |
| 640x480 then 1024x768 unlimited, 180 s cooldown | **61.1 / 61.6 fps**, no reboot |
| 1280x1024 unlimited, single, rested | **60.3 fps**, no reboot |
| 640/1024/1280 unlimited, 120 s cooldown (3 back-to-back) | **REBOOT** partway |

### Conclusions

1. **The drivers are not the problem.** Every single run passes, at every resolution,
   including the heaviest (1280x1024) and including unlimited/flat-out. Q3 is
   consistently 60-62 fps.
2. **It is not the resolution and not the frame limiter.** 1280x1024 alone from a
   rested card is fine; unlimited from cold is fine.
3. **It is cumulative duty cycle.** Three flat-out runs at a 120 s cooldown reboots the
   box; two runs at 180 s does not. The board tolerates load but not sustained
   back-to-back load without recovery time — consistent with the HiNT bridge heat /
   ~80 W 6-pin power warnings in `V56K-PLAN.md`.

### Practical operating envelope (until cooling/power is improved)

- **Safe:** interactive gameplay (the operator played Q3 with no trouble), and up to
  ~2 flat-out timedemos with >=180 s of idle between them.
- **Unsafe:** 3+ flat-out timedemos in quick succession; anything back-to-back.
- Benchmark with `bench-safe.py` (enforces this) rather than a naive loop. The old
  harness's back-to-back matrix is what produced hours of phantom "regressions".

**This also fully explains §9.** Q3 "regressing" on byte-identical golden binaries was
never a code regression — it was a hot card. Every A/B conclusion drawn during that
window (the 256MB clamp, the texture munge) was measured against a degrading baseline
and must be re-tested cold before being trusted.

---

## 12. 256MB MODE IS WORKING (2026-08-12)

The operator flipped the card's dual-VBIOS switch to 256MB. Measured:

| check | result |
|---|---|
| `HardwareInformation.MemorySize` | **0x10000000 = 256 MB total = 64 MB/chip** |
| `Retro3dfxSliUnits` | **4** — all four VSA-100s still detected |
| display driver | loads normally, 1024x768x16@85, no VGA fallback |
| Q3 in-engine capture (640x480) | **renders perfectly** — no texture corruption or wrapping |
| Q3 timedemo 1024x768 | **61.4 fps**, no reboot |
| Q3 timedemo 640x480 then 1024x768, 180 s cooldown | **61.2 / 61.4 fps**, no reboot |

The feared catastrophic case did not occur: the 256MB VBIOS widened BAR1 correctly,
so the chip is not decoding more than PnP reserved, and `H3DetermineMemorySize` sizes
the 128Mbit parts exactly as §2 of `V56K-256MB-READINESS.md` predicted.

**Performance is unchanged** (61 fps, same as 128MB mode) — expected, because Q3 on
this box is CPU/present-bound at ~61 fps, not memory- or fill-bound. 256MB buys
texture headroom, not frame rate, on this workload.

### The texture munge is REJECTED on hardware evidence

Readiness Changes 3/4 proposed routing texture base addresses through
`SST_TEXTURE_MUNGE_ADDRESS` because a plain mask is 25 bits (32MB). The conditional
form turned out to be the perfect experiment — it only fires above 32MB, so it never
executed at 128MB and ran for the **first time** at 64MB/chip:

- with the munge: box **reboots** during a Q3 timedemo
- with the plain mask, same 256MB mode: Q3 renders perfectly, **61.4 fps**

So `grTexMultibaseAddress` does not carry the same register semantics as the tiled
path at `GTEX.C:2811/:2825`. Both Glides are back to the shipped mask and the
reasoning is recorded at the call sites so it does not get re-applied.

**Twice now a speculative "256MB hardening" made a working driver worse** (the BAR
clamp, then the munge). Neither was needed. The stack was already 64MB/chip-clean.

### ⚠ Operational: do not drive the card from two places at once

The operator reported a Q3 crash at 1024x768 while an automated timedemo sweep was
also running. One fullscreen 3D application at a time is a hard rule on this stack
(see the deploy/bench skills) — a second one contending for the Glide surface is its
own failure mode and will contaminate any stability measurement. Coordinate: either
the operator drives, or the harness does, never both.

---

## 13. The operator's "Quake 3 crash" — root cause: OUR BENCHMARK HARNESS

The operator reported Q3 crashing at 1024x768 while an automated sweep was running.
Investigated properly rather than assumed:

**Evidence gathered**
- No Dr Watson logs, no `.dmp` crash dumps anywhere — Windows never caught a fault.
- Flight recorder + `C:\3dfxvs.log`: **zero** `WEDGE-BREAK`, **zero** `H3MakeRoom STALL`,
  zero `memMgr ALLOC-FAIL`, zero `DP2-*` errors. The accelerator never hung.
- **Not reproducible**: interactive q3dm1 with bots at 1024x768 in 256MB mode ran a
  full 3 minutes, machine stable throughout (uptime 1048 -> 1336 s).

**Root cause: `bench-safe.py` force-killed the operator's live game.** The cleanup
step ran `taskkill /f /im quake3.exe` — `/im` kills EVERY quake3.exe, not the one the
harness started — and there was no pre-flight check for an already-running instance.
So the moment the sweep finished a run, it terminated the operator's session. From the
player's seat that is indistinguishable from Quake 3 crashing.

**Fix (`V56K-NO-STOMP`), verified live on hardware:**
1. Pre-flight — if a `quake3.exe` is already running the harness **ABORTS** with the
   offending PID and touches nothing. Somebody is using the box.
2. Cleanup kills **only the PID the harness itself launched** (`taskkill /f /pid N`).
   `/im` is gone entirely.
3. The `QUIESCE` list is documented as background CPU thieves only — it must never
   contain a game executable, i.e. never terminate something a human started.

Verified: with an operator-style Q3 running as pid 276, the harness printed
`ABORT: quake3.exe already running (pid 276)` and **pid 276 survived**.

**Wider lesson.** Automation that force-kills by image name is unsafe on a shared box,
and it also silently corrupts measurements — a killed session looks like an
instability datum. Several "crash" observations in this session deserve re-reading
with that in mind.

---

## 14. Game benchmark results in 256MB mode (2026-08-12)

All on the golden kernel pair + hardware-verified Glides, 256MB/64MB-per-chip VBIOS,
via `bench-safe.py` (cooldowns enforced, no-stomp pre-flight).

| game | renderer | resolution | result | stability |
|---|---|---|--:|---|
| **Quake 3** (timedemo four) | our ICD `3dfxogl` | 640×480×16 | **61.7 fps** | stable |
| **Quake 3** | our ICD `3dfxogl` | 1024×768×16 | **61.7 fps** | stable |
| **UT99** (UTbench.dem) | **Glide** (our `glide2x`) | 640×480×16 | **39.81 fps** avg (2939 frames / 73.81 s, Min 10.95, Max 60.26) | stable, clean exit |
| **UT99** | Direct3D (our HAL) | 640×480×16 | ~24 fps | stable |
| **RtCW** (wolfbench) | *vintage* `gl/openglv5.dll` | 1024×768×16 | **51.8 / 51.2 / 50.8 fps** | stable ×3 |

### Corrections to earlier numbers

- **UT99 Glide is 39.8 fps, not ~24.** The earlier ~24 figure was read off the HUD
  mid-demo; the complete 2939-frame result from `bench.log` is 39.81 fps avg. **Glide
  is UT's fast path** — roughly 1.7× the D3D number — so the earlier "UT is CPU-bound
  at 24 fps on every renderer" conclusion was wrong. D3D is slow because it falls back
  to software (see §7/§10), not because the CPU caps everything at 24.
- UT-Glide ran a full timedemo and exited cleanly with no reboot, twice in a row here.
  It remains historically intermittent, but it is not reliably broken.

### 256MB mode delivers measurable texture headroom

UT's Glide init reports per-TMU texture space:

```
128MB mode:  Init: Glide tmu 0: tmuRam=2 Space=15874032   (~15.9 MB)
256MB mode:  Init: Glide tmu 0: tmuRam=2 Space=32389104   (~32.4 MB)
```

**Texture memory per TMU has doubled.** That is the concrete payoff of the 256MB
VBIOS: not frame rate (these workloads are CPU/present-bound), but twice the texture
working set before thrashing.

### RtCW is NOT running on our driver

RtCW logs `...assuming 'gl/openglv5.dll' is a standalone driver` /
`GL_VENDOR: METABYTE/WICKED3D` — the vintage 2001 Wicked3D wrapper the game ships,
not our ICD (`3Dfx [retro3dfx 0.5.0]`). Setting `r_glDriver` on the command line, then
persisting it in `wolfconfig_mp.cfg`, then making that file read-only so RtCW could not
revert it, all failed to move it: `r_glDriver` is CVAR_LATCH and this GOG build pins
the wrapper. So the ~51 fps is a valid reference point but **not a measurement of this
stack**. Open item.

### Operator note: a stray system env var was contaminating everything

`FX_GLIDE_SLI_LOG` had been left set **system-wide** from the SLI investigation, so
every Glide application was writing a diagnostic log (the ICD log had reached 1.5 MB of
per-texture `APPLYTEX` lines). Removed. Q3 measured 61.7 fps afterwards versus 60.0-61.4
before — within noise, but the logging was a real per-frame cost and a stability risk.
**Never leave a logging env var set system-wide.**

## 15. Hardware monitoring: what this box can and cannot measure (2026-08-12)

Asked whether the GPU has a temperature sensor we can monitor. Probed it properly
rather than guessing.

**The GPU has no temperature sensor, and no software can invent one.** The VSA-100
(2000) has no on-die thermal diode, and there is no monitor chip anywhere on the
V5 6000 board: SpeedFan's full sweep — ISA probe, PIIX4 SMBus scan of every address
$14–$2C, and the PCI/I2C paths — finds nothing on the card. WMI agrees from the
other side: no `MSAcpi_ThermalZoneTemperature` instances (the 1998 BX BIOS predates
ACPI thermal zones) and no `Win32_TemperatureProbe`. **Do not look for a GPU
temperature again.**

What the Tyan 440BX motherboard *does* have is a real hardware monitor, and it is
enough to characterise the card indirectly:

| Chip | Bus | Address | Channels | Idle reading |
|---|---|---|---|---|
| LM79 | ISA | `$290` | board temp, 3 fan tachs, 7 voltage rails | 24 °C |
| LM75 | Intel SMBus | `$4C` | temp | 36 °C |
| LM75 | Intel SMBus | `$4D` | temp | 32 °C |
| Samsung 860 EVO | AdvSMART | — | drive temp | 25 °C |

Idle rails: COREA/COREB 1.68 V (dual P3), +3.3 V 3.30, +5 V 5.08, **+12 V 12.22**,
−12 V −13.44, −5 V −5.68. Fans 0 / 4441 / 4470 RPM (Fan1 header unpopulated).

**Why +12V is the interesting channel.** The card pulls ~80 W through a 6-pin input.
If the resets in §11 are power rather than heat, a sag here under 4-chip load is the
direct evidence — and it is the one hypothesis §11 could not separate. `thermal.py`
therefore range-checks the rails on every window and prints `RAIL EXCURSION`.

### How it is logged (no port I/O of ours)

SpeedFan 4.49 was already installed; its signed `speedfan.sys` does the ISA/SMBus
access, so we add no kernel risk. Logging is driven entirely from its **plain-text**
configs — `speedfansens.cfg` (`logged=true` per reading, names set to the CSV
headers) and `speedfanparams.cfg` (`LogEnabled=true`). It appends a tab-separated
`SFLog<YYYYMMDD>.csv` every ~3 s, which `toolchain-3dfx/build/thermal.py` downloads:

```
thermal.py ensure          # start + verify it is logging
thermal.py now             # latest sample  (--seconds-only to mark a window)
thermal.py window <t0>     # min/max/delta since a mark, + rail range check
thermal.py stop
```

> **SpeedFan rewrites both configs when it exits.** Kill it before uploading a
> config or the change is silently lost. This cost a cycle to find.

### Two measured caveats — read before trusting this data

1. **It does not perturb benchmarks.** Q3 1024×768 unlimited measured **60.9 fps**
   with SpeedFan sampling throughout, against a 60.8–61.1 fps baseline. And it is
   not starved by an exclusive-fullscreen Glide app: across a full timedemo the log
   had **57 samples, no gap >6 s** at a 3 s cadence.

2. **It is far too coarse to see a single run.** A complete flat-out 1024×768
   timedemo moved *nothing*: board, both LM75s, and all three rails were flat to
   their resolution (1 °C, ~0.01–0.16 V) across the whole window. These sensors sit
   on the motherboard, not on the card, behind 4400 RPM of case airflow.
   **A single benchmark is not a thermal measurement.** The only regime where this
   telemetry can say anything is a long soak — which is exactly the regime where
   §11's reboots occur (three flat-out runs at a 120 s cooldown). Use it that way,
   sampling continuously across a whole session, or not at all.

## 16. Why D3D gets no hardware: the driver is built `DX=7` (2026-08-12)

Chasing "TotalVRAM=0 makes D3D fall back to software". **The premise was wrong and
the real cause is a build-configuration one.** Measured, not reasoned.

### TotalVRAM=0 is a red herring — it came from a different build

The `DDRAW-ENABLED: ... vram=00000000h` line that started this only exists on the
`USE_NT5_DDMEMMGR` arm of DDFXNT.C:794. **The deployed driver takes the `#else`
arm** — it prints `DDRAW-ENABLED: units=4 cyMem=4095 slop=4 ddHeap=3203`, and the
heaps it publishes are healthy:

```
HEAP0 LIN start=0008E400h end=0337FFFFh      (~54 MB linear, 64 MB/chip mode)
HEAPSUM n=9 tiledStart=039C0000h sliTileCmp=03E50000h
DDGDDI-OK: d3dGlobal=E1B889C0h heaps=9       (x16, and ZERO DDGDDI-FAIL)
```

`d3dGlobal` is non-NULL and 9 heaps are handed over, i.e. `D3DHALCreateDriver`
succeeds and the D3D HAL *is* published. Nothing here is zero. Do not go looking
for a vram-reporting bug on this build again.

### What actually fails

`d3dlab` (windowed, `D3DDEVTYPE_HAL`) on the box:

```
CreateDevice failed 8876086a        == D3DERR_NOTAVAILABLE
```

Reproduced at 1024×768×32, 1024×768×16 and 800×600×16 — so it is not a format or
mode issue. The decisive measurement: **`C:\3dfxvs.log` grew by exactly 0 bytes
across the failing call.** The D3D8 runtime rejects the device from cached caps
*without ever entering the driver*, so no amount of driver-side memory reporting
can affect it.

### Root cause

`Displays/H5/SOURCES`:

```
!IFNDEF DX
DX = 7          <-- and nothing in SETENV.BAT, bldw2k.bat or build-w2k.sh sets it
!ENDIF
C_DEFINES = $(C_DEFINES) -DDX=$(DX)
```

and `DDINIT.C:240`, inside the `dwFlags` the driver reports to DirectDraw:

```
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
/* DX8 GetDriverInfo2 callback */   DDHALINFO_GETDRIVERINFO2 |
```

**The driver is compiled as a DirectX 7 driver.** `DDHALINFO_GETDRIVERINFO2` is
compiled out, so the DX8 caps negotiation (`GUID_GetDriverInfo2` → the
`DD_GETDRIVERINFO2DATA` handler at DDFXNT.C:1062) never happens and the D3D8
runtime concludes there is no D3D8-capable HAL. That is precisely
`D3DERR_NOTAVAILABLE` with no driver entry.

Corroborating, from `dxdiag` (a DX9 runtime querying a DX7 driver):
`DDI Version: unknown`, `Display Memory: n/a`, `D3D Status: Not Available`.

**D3D7 is unaffected** — during a forced UT99 D3D run the driver logged
`DrvEnableDirectDraw ENTER` / `EXIT ok`, 16× `DDGDDI-OK` and the fullscreen
`DrvAssertMode` transitions, i.e. the DirectDraw/D3D7 HAL is live and servicing
the app. This matches CS 1.6 D3D benching 33.5 fps. **Only D3D8+ titles are
affected.**

### BLOCKED: we cannot build `DX=8` with the toolchain we have

The DX8 source paths are real and substantial (D3CONTXT, D3INIT, D6DP2, D7DP2,
DDFXNT, DDINIT — the `GetDriverInfo2` handler is fully written), but they need DX8
DDK headers we do not have:

| symbol | defined in our toolchain? |
|---|---|
| `DD_GETDRIVERINFO2DATA` | **no** |
| `GUID_GetDriverInfo2` | **no** |
| `D3DGDI_GET_GDI2_DATA` | **no** |
| `DDHALINFO_GETDRIVERINFO2` | **no** |

`prefix/drive_c/3dfxtools/` contains `dx7ddk` only. Those four symbols appear
nowhere in the tree except the Win9x `*.COD` compiler listings under
`H5/Win9x/DX/DD32/` — proof 3dfx *did* build this path, with a DDK we are missing.
Setting `DX=8` today just fails to compile.

**To unblock:** the DirectX 8.0/8.1 DDK (`ddrawint.h` / `d3dhal.h` of that
vintage) needs to be added to `3dfxtools`. Then build with `DX=8 DXDDKVERSION=8`,
and treat it as a BSOD-risk binary — the whole DX8 arm will be executing for the
first time here, so deploy it with the `.v56kprev` rollback and expect to use it.
