# Voodoo 5 6000 — multi-chip / 4-way SLI findings (2026-08-11)

Box: **192.168.1.133 "P3-DUAL"** — dual Pentium III 700, 1 GB, XP SP3, Voodoo 5 6000
(4× VSA-100 behind a HiNT HB1-SE66 bridge, 128 MB BIOS mode = 32 MB/chip).
Stack: `3dfxv5m.sys` + `3dfxv5d.dll` + `glide3x.dll`/`glide2x.dll` + `3dfxogl.dll` (ICD 0.5.0).

**As of 2026-09-04 the card lives in `192.168.1.191`** — Athlon 1152 MHz /
nForce2, XP SP3, AGP, same board and same 128 MB VBIOS mode. §§1–25 were
measured on `.133`; **§26 is `.191`** and is the current front: AmigaMerlin
3.1-R11 drives 4-way SLI on this board with no skew, so the multi-chip scanout
defect is **ours**, not the hardware's.

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

## 17. DX8 DDK obtained — and how far the DX=8 build actually gets (2026-08-12)

§16 ended blocked on missing DX8 driver headers. **Obtained and archived**, so that
half is closed:

- **Windows XP SP1 DDK** (`en_winxp_sp1_ddk.exe`, 137.8 MB, sha256 `6f3113bd…`) on the
  share at `…\3dfx-build-toolchain\downloads\`, in `SHA256SUMS.txt`, described in the
  share README, and fetched+extracted automatically by `setup-toolchain.sh`.
  Readback-verified byte-for-byte off the NAS after upload.
- It supplies all four blocking symbols; the W2K DDK has none of them:
  `DDHALINFO_GETDRIVERINFO2` (`ddrawint.h:907`), `DD_GETDRIVERINFO2DATA`,
  `GUID_GetDriverInfo2`, `D3DGDI_GET_GDI2_DATA` (`d3dhalex.h`).
- `RETRO3DFX_DX=8 build-w2k.sh display` now builds with
  `DX=8 DXDDKVERSION=8 DXDDK=c:\3dfxtools\dx8ddk`.

### Four traps found the hard way (all now encoded in the scripts)

1. **`.exe` is a WinZip SFX** — needs `7zz x -tzip`; a plain `x` sees only the PE.
2. **CAB members are `<CABNAME>_FILE_<N>`**; real names come from the paired `.INF`.
   `extract_ddk.py` handles it — but the **XP INFs write the subdir with a leading
   backslash** (`49000,\inc\ddk`), and `DEST / sub` with an absolute `sub` silently
   discards `DEST`, so it tried to extract the DDK to `/`. Now stripped.
3. **`DXDDK\inc` is PREPENDED**, so it shadows the W2K DDK *and* 3dfx's own vendored
   `Displays/INC/DX95TYPE.H`. It must stay a **curated** set, never a copy of the XP
   DDK. `d3dhal.h` is deliberately **excluded**: it is the Win9x HAL variant and
   collides with the NT pair (`d3dnthal.h` + `dx95type.h`, which aliases
   `D3DHAL_* -> D3DNTHAL_*`) as `D3DHAL_CALLBACKS: redefinition`.
4. **`DX8DDK_DX7HAL_DEFINES`** — the DX8 DDK hides *every* legacy `D3DHAL_*` alias
   behind this switch. This driver uses the DX7 HAL names throughout, so without it
   `D3DHAL_TSS_MAXSTAGES`, `D3DHAL_DP2SETSTREAMSOURCE` etc. simply vanish at DX=8.

Two `PRECOMP.H` edits carry 3 and 4 (`V56K-DX8-DX7HAL`, `V56K-DX8-INCORDER`; the
latter because at `DIRECT3D_VERSION 0x0800` the DX7 body of `d3d.h` is guarded out,
taking with it the chain that used to reach `dx95type.h` before `driver.h`). **Both
sit inside `#if (DX == 8)` and are inert at DX=7 — verified: the default build is
still green at 968,916 bytes.**

### STILL BLOCKED — but now on 3dfx's source, not on tooling

The header wall is gone; what remains are real gaps in 3dfx's own NT DX8 code, which
**was evidently never compiled** (the `GetDriverInfo2` symbols appear in this tree
only in the *Win9x* `.COD` listings — they shipped DX8 for Win9x, not for W2K/NT):

| file | error |
|---|---|
| `ddfxnt.c:210` | includes `d3dhal.h` unguarded — needs the NT/9x split the other sites have |
| `d6dp2.c:4711` | `'lpCmdStart' : undeclared identifier` |
| `d7dp2.c:1645,1655,1664` | `'bStoredStream' : is not a member of '__unnamed'` (`d7d3d.h:96`) |

These are a porting job on the DP2 command-buffer path, not a configuration fix, and
that path is exactly where a wrong edit bugchecks the box. **Do not deploy a DX=8
binary until these are properly resolved and the d3dlab matrix passes.** The DX=7
driver on the box is unaffected by any of this work.

## 18. The resets are a LOAD ceiling, not heat — and Glide is the worst offender (2026-08-13)

Overnight run with the new rail/temperature telemetry (§15) attached. This
supersedes the "cumulative duty cycle" reading in §11 and in memory
`v56k-thermal-envelope`: **that theory does not survive the measurement.**

### What the sensors say at the moment of a reset

Sampled every 3 s right up to a reset, from a box that had been idle for hours:

```
3679  board 23C  smb4C 35.0  smb4D 31.5  +12V 12.22  +5V 5.08  +3.3V 3.30  Vcore 1.68/1.68
3682  board 23C  smb4C 35.0  smb4D 31.5  +12V 12.22  +5V 5.08  +3.3V 3.30  Vcore 1.68/1.68
3685  board 23C  smb4C 35.5  smb4D 31.5  +12V 12.22  +5V 5.08  +3.3V 3.30  Vcore 1.68/1.68
<reset>
```

Across the whole session `V12` never left 12.22 (min == max) and the board never
left 23-24 C. **No sag, no heat, and the flight-recorder ring is clean** -- no
`H3MakeRoom STALL/WEDGE-BREAK`, no `DdFlip WEDGE-BREAK`, no `DP2-PARSE-ERR`, no
`memMgr ALLOC-FAIL`. The driver never sees a fault; the machine simply goes away,
in one case exactly at the `DrvAssertMode(DISABLE)` full-screen transition.

Two things the telemetry genuinely cannot see, so this is *not* proof of a clean
bill of health: a **fast transient** (3 s sampling cannot resolve a millisecond
droop when four chips spin up) and **die temperature** (the VSA-100 has no sensor
and the board thermistors are nowhere near it).

### What actually predicts a reset

| workload | frame cap | result |
|---|---|---|
| Q3 timedemo 1024x768 | vsync | **59.9 fps, stable** |
| Q3 timedemo 1024x768 | unlimited | 61.3 fps; reset 1 of 3 runs |
| UT99 UTbench, **OpenGL** (our ICD 0.5.0) | vsync | **34.56 fps avg** (min 8.99 max 55.01, 2940 frames / 85.06 s), stable |
| UT99 UTbench, **OpenGL** | unlimited | **RESET** |
| UT99 UTbench, **Glide** | unlimited | **RESET** |
| UT99 UTbench, **Glide** | vsync | **RESET** |

It is a **load ceiling**, not accumulated heat: a cold, freshly-booted box resets
on the first flat-out run, and the same box runs frame-limited work all night.
Every reset recovers on its own in ~45 s, and `chkdsk` is clean afterwards.

**Glide is the outlier and the one clear driver-side lead.** UT-Glide is the only
workload that resets even *frame-limited*, while UT-OpenGL at the same resolution
and cap is stable -- and the ICD reaches the hardware through `glide3x`, so the
finger points at the **`glide2x`** path UT's GlideDrv uses directly. That matches
the standing "UT-Glide is intermittent" note in `v56k-stability-findings`, and it
is where to look next; it is not a thermal story.

**Operational rule for this board: cap the frame rate.** `FX_GLIDE_SWAPINTERVAL=1`
(or the renderer's own vsync) is the difference between a stable session and a
reset, and it costs nothing in these titles -- UT averages 34.6 fps, nowhere near
the 85 Hz cap, so the cap only removes the peaks. Prefer OpenGL over Glide in UT.

## 19. RtCW is on our ICD now — and the ICD never enables multitexturing (2026-08-13)

### RtCW: fixed, and benchmarked

§14 left RtCW running the vintage 2001 Wicked3D wrapper it ships as
`gl\openglv5.dll`, because `r_glDriver` is `CVAR_LATCH` and this GOG build pins
it — command line, `wolfconfig_mp.cfg`, and making that file read-only all failed
to move it. **Stop fighting the cvar and satisfy it instead: put our ICD at the
path it insists on.** `toolchain-3dfx/build/bench-rtcw.py` backs up
`gl\openglv5.dll`, drops `system32\3dfxogl.dll` in its place, and always restores
it. Result:

```
GL_VENDOR:   3Dfx Interactive Inc.
GL_RENDERER: 3Dfx [retro3dfx 0.5.0]        <- ours, finally
...setting mode 6: 1024 768 FS
421 frames, 11.8 seconds: 35.8 fps
```

**RtCW 1024x768x16 on our ICD: 35.8 fps**, against ~51 fps for the Wicked3D
wrapper. We are slower than the wrapper, and the log says exactly why.

### The ICD never enables multitexturing — on a 2-TMU-per-chip card

Both engines report the same thing. Q3 is blunt about it:

```
...GL_ARB_multitexture not found
multitexture: disabled
GL_EXTENSIONS: GL_EXT_paletted_texture GL_EXT_shared_texture_palette GL_SGIS_multitexture
```

`sst_export.c:782` appends **`GL_SGIS_multitexture`** when the chip reports two
TMUs (`gc->grNTexelFx == 2`). But `GL_SGIS_multitexture` and `GL_ARB_multitexture`
are *different APIs*, and Quake 3 / RtCW only ever look for the ARB one. So on a
card with **two TMUs per chip, times four chips**, every lightmapped surface is
drawn in **two passes** instead of one — double the geometry submission and double
the fill.

Worse, grepping the whole ICD tree finds **neither** API's entry points:

| symbol | present in `SWLIBS/OPENGL`? |
|---|---|
| `glActiveTextureARB`, `glMultiTexCoord*ARB`, `GL_MAX_TEXTURE_UNITS_ARB` | no |
| `glSelectTextureSGIS`, SGIS multitexcoord entry points | no |

So the advertised `GL_SGIS_multitexture` is **not backed by an implementation
either** — it is a bare string. Any app that believed it and called the SGIS
entry points would fail to link them.

**This is the single largest performance item on the board**, and it is a
*user-mode* DLL, so unlike the DX8 work it cannot bugcheck the box. Implementing
`GL_ARB_multitexture` (the three entry-point families, `GL_MAX_TEXTURE_UNITS_ARB`
= 2, and the extension string) against the ICD's existing two-TMU state
(`gc->texture.sst.texUnits[0..1]`, already populated for `grNTexelFx == 2`) should
collapse those two passes into one in Q3, RtCW and every other Quake-engine title.
Caveat before anyone promises a number: at 1024x768 on a 700 MHz P3 these titles
are partly CPU-bound (§8 measured UT identical on Glide and D3D), so expect the
win to be real but smaller than the 2x the pass count suggests.

Removing the unbacked `GL_SGIS_multitexture` string is worth doing regardless.

## 20. Multitexture: the ARB entry points are ALREADY SHIPPING, just never advertised

Follow-on to §19, and it changes the effort estimate for queue item 12
(`icd-multitexture`, `optimized/OPTIMIZATION-QUEUE.md`, which budgets it as
"High risk — a feature, not a tweak; largest commit in the queue").

**The deployed `3dfxogl.dll` (retro3dfx 0.5.0, 708,608 bytes) already exports the
ARB entry points.** Checked against the binary itself:

| symbol | in the shipped DLL |
|---|---|
| `glActiveTextureARB` | **present** |
| `glMultiTexCoord2fARB` | **present** |
| `glClientActiveTextureARB` | **present** |
| `glSelectTextureSGIS` | present |
| the string `GL_ARB_multitexture` | **absent** |

So the plumbing exists; only the advertisement is missing. `sst_export.c:782`
appends `GL_SGIS_multitexture` when `gc->grNTexelFx == 2`, and Q3/RtCW only ever
look for the ARB name — which is why Q3 says `multitexture: disabled` on a card
with two TMUs per chip across four chips.

### Tested by patching only the advertised string

Byte-patched the 21-byte literal `"GL_SGIS_multitexture "` to
`"GL_ARB_multitexture  "` in a copy of the DLL and deployed that (user-mode, so
no bugcheck risk, and reversible without a reboot). Q3 changed behaviour exactly
as intended:

```
...using GL_ARB_multitexture
GL_EXTENSIONS: GL_EXT_paletted_texture GL_EXT_shared_texture_palette GL_ARB_multitexture
multitexture: enabled
1260 frames, 19.9 seconds: 63.4 fps      (vsync-limited; 59.9 fps on the stock DLL)
```

**+5.8 % at 1024x768 vsync-limited, and the frame rendered.** A captured frame
showed correct textures, correct lightmap falloff, correct HUD and weapon — none
of the failure signature of the earlier
`optimized/experimental/3dfxogl-0.1.4-multitexture-itercolor-DARK.dll`.

### What is NOT established — do not ship on this evidence

* **Rendering correctness is UNVERIFIED.** The two frames I compared came from
  different points of a demo (different room, different HUD), so the -17.6 %
  mean-luminance delta between them is an artifact of comparing different scenes,
  **not** a measurement of darkening. A controlled same-frame A/B is still owed.
  `toolchain-3dfx/build/icd-ab-shot.py` is a start at one (fixed map + `setviewpos`
  to pin the camera) but does not yet produce a shot — Q3 did not launch under
  `+devmap`, and quoted console commands do not survive the EXECW/cmd/start
  quoting chain, which is why the binds had to move into an `+exec`'d cfg.
* **+5.8 % is one vsync-limited data point**, well short of the queue's 25-50 %
  estimate — expected, since these titles are partly CPU-bound on a 700 MHz P3,
  and a vsync-capped run compresses differences. An unlimited run would measure it
  better but resets this board (§18).
* The string patch is a **probe, not a fix.** The real change is in
  `sst_export.c:782`: advertise `GL_ARB_multitexture`, and drop the
  `GL_SGIS_multitexture` string, which is advertised but has **no** SGIS entry
  points behind it in the ICD source at all.

The box was returned to the stock ICD (`3dfxogl.dll.arbbak` retained on it).

## 21. MOHAA runs on our ICD — and the "stability" we had was accidental throttling

### Retro3dfxLog=1 was holding the board up (A/B/A)

Found `Retro3dfxLog = 0x1` still set under `Services\3dfxvs\Device0`, with the ICD
writing an **800 KB `C:\3dfxogl.log` per session** of per-texture `APPLYTEX` and
per-batch `BEGIN` lines. That is the same class of mistake as the
`FX_GLIDE_SLI_LOG` incident in §14 — and **every benchmark in §18-§20 was measured
with it on.** Turning it off produced something much more interesting than a
speedup:

| `Retro3dfxLog` | Q3 1024x768 vsync |
|---|---|
| 1 (on) | 59.9 / 60.5 / 60.5 fps — **stable, 5 runs** |
| **0 (off)** | **RESET, RESET, RESET** — three consecutive |
| 1 (restored) | 60.5 / 60.5 fps — **stable again** |

A clean A/B/A. The per-frame log I/O was acting as an **unintentional throttle**,
and removing it pushed the card past the load ceiling of §18 immediately. This
reframes the whole night: the board is *less* stable than it looked, and what was
keeping it up was debug logging, not headroom.

**Do not treat this as the fix.** Relying on log I/O to pace the GPU is accidental
and fragile — it makes every benchmark a "safe mode" number and silently taxes
real gameplay. The real work is a proper frame/rate limiter (or the underlying
power/timing cause), with logging off. Left at `1` for now because that is the
configuration the box is stable in; **anyone benchmarking must state which setting
they used**, because it changes both the fps and whether the box survives.

### MOHAA: mounted, running on our ICD, benchmark blocked

`toolchain-3dfx/build/bench-mohaa.py` does the whole prep. MOHAA is SafeDisc
(`drvmgt.dll` + `secdrv.sys`) so it needs the disc:

* Disc 1 ISO was **not** on the fleet share (that path in retro-agent's
  driver-bench skill belongs to a different host — this share has no game ISOs at
  all). It is on the box, on the Administrator's Desktop.
* Staged to `C:\ISO\MOHAA_CD1.iso` — **DAEMON Tools 3.47 will not take a path with
  spaces** — and mounted with `daemon.exe -mount 0,<image>`. `daemon.exe` stays
  resident, so it must be launched **detached** (`start ""`) or EXECW tree-kills it
  and undoes the mount. Mounts as **`E: MOHAA_DISK1`**.
* Our ICD staged as `MOHAA\opengl32.dll` (the game directory wins the DLL search
  order).

Confirmed from MOHAA's *own* log:

```
...setting mode 3: 640 480 FS
GL_VENDOR:   3Dfx Interactive Inc.
GL_RENDERER: 3Dfx [retro3dfx 0.5.0]
GL_VERSION:  1.1.0 3Dfx Beta 3.00
------ Server Initialization Complete ------  6.06 seconds
```

**MOHAA loads, initialises our ICD, and plays a map.** It shipped configured for
`r_colorbits 32`, which reset the box on the first map load; at 640x480x16 with
`r_picmip 1` it runs (config backed up as `unnamedsoldier.cfg.v56kprev`).

It also confirms §20 in a **third** engine: `GL_ARB_multitexture not found` and
`GL_MAX_ACTIVE_TEXTURES_ARB: 0`.

**No fps number yet, and the reason is structural.** MOHAA has no bundled demo,
`record` is not accepted from a startup cfg, and it has no `com_speeds` output —
so a timedemo needs a demo recorded during interactive play. retro-agent's
driver-bench skill hit the same wall ("stage first; DirectInput blocks recording")
and no `mohbench.dm_` exists in `benchmarks/`. Screenshot-based fps reading is out
too: GDI capture of this box garbles during 3D (the same scanout defect already
recorded for Q2). **A MOHAA fps figure needs one demo recorded by hand at the
keyboard; everything else is in place and scripted.**

### 21a. CmdfifoSize does NOT reproduce the logging throttle (negative result)

Tried to replace the §21 logging crutch with a legitimate driver knob.
`ENABLE.C:1235` reads `CmdfifoSize` (default 512 KB, clamped to 64 KB..1 MB,
rounded to 4 KB); a smaller FIFO fills sooner and forces more `H3MakeRoom` waits,
which looked like a clean way to buy the same pacing.

**It does not work.** With `Retro3dfxLog=0` and `CmdfifoSize=65536` (the minimum),
after a reboot to apply it, Q3 1024x768 **vsync reset on the first run** — same as
with the default FIFO. Then the box **hard-froze**: no ping, no route, NIC dead,
requiring a physical power cycle (the NAS on the same switch stayed up, so it was
the box, not the network).

Why the two are not equivalent, and the useful part of the negative result: a
smaller FIFO does not slow submission, it only **blocks** the CPU once the FIFO is
full — the GPU still receives the same tight burst of work. The ICD's per-texture
file logging instead **interleaves CPU time between submissions**, spreading the
work out within the frame. Both give ~60 fps under vsync, so **frame rate is not
the variable — the burstiness of submission inside each frame is.** That is
consistent with the transient hypothesis in §18, and it means the fix has to
*spread* submission, not merely throttle or cap it. No registry knob does that;
it needs actual pacing code in the Glide/ICD submission path.

**Do not leave the box on `Retro3dfxLog=0`.** That configuration reset it 4 times
out of 4 and then froze it hard.

## 22. The resets were never power — every spin-breaker outran the video watchdog

**This supersedes §18, §21 and §21a.** Those blamed a load ceiling / power
transient, with logging as an accidental throttle. The crash dumps say otherwise.

### The evidence

**12 of 12 minidumps on .133 are `0x100000EA THREAD_STUCK_IN_DEVICE_DRIVER`**, and
parameter 4 is `1` — which specifically means *the video port driver detected the
stuck thread*. Windows names the culprit itself, in
`HKLM\SYSTEM\CCS\Control\Watchdog\Display`:

```
DriverName        REG_SZ     3dfxv5d
BreakCount        REG_DWORD  0x18      (24 watchdog events)
ShutdownCount     REG_DWORD  0x14      (20 driver shutdowns)
BugcheckTriggered REG_DWORD  0x1
```

and in the System event log, source `3dfxvs`, **event 108**:

> The driver 3dfxv5d for the display device \Device\Video0 **got stuck in an
> infinite loop**. This usually indicates a problem with the device itself or with
> the device driver programming the hardware incorrectly.

Our own flight recorder had already caught it:

```
H3MakeRoom STALL>=100K:  N=644 fifoSize=524276 curRead=000750c4 room=84
H3MakeRoom WEDGE-BREAK@50M: N=644 fifoSize=524276 curRead=000750c4 room=84
```

`curRead` is **identical** in both lines — the hardware stopped consuming the FIFO.

### The bug

Every accelerator spin *was* bounded — but the bounds were 50M/100M iterations, and
each iteration is an **uncached MMIO read across PCI (~1 us)**. That is **50–100
seconds**. Windows shoots a stuck display thread at **~30 s**. So the watchdog
always won, the recovery paths were **dead code**, and a recoverable FIFO wedge
became a bugcheck every time. The `CFIFO.C` comment even says the 50M value was
chosen so "near-stock wedge behavior stays observable" — a diagnostic decision
that never accounted for the watchdog.

Fixed: one shared `RETRO_WEDGE_BREAK_SPINS = 2000000UL` (~2 s) across **all eight**
spin sites in `CFIFO.C`, `DDFLIP.C` (×3), `DDSURF.C`, `DDGLOBAL.H`, `HW.H` (×2).
Still ~20× beyond the "healthy stalls drain well under 100K" figure, but an order
of magnitude inside the watchdog — so a wedge now costs a frame, not the machine.
`tests/test_source_invariants.sh` guards both halves: no raw 50M/100M literal may
return, and the constant must stay small. 89/89 pass.

**This also explains the `Retro3dfxLog` A/B/A in §21.** Logging never provided
"power headroom" — it perturbed timing enough that the FIFO wedge rarely occurred.
The correlation was real; the mechanism I gave for it was wrong.

**Still open:** *why* the hardware stops consuming the FIFO (`curRead` frozen) is a
separate question this fix does not answer — it only makes the wedge survivable.

### Separately: what actually blocked normal boot

After the hard freeze, normal boot stopped completing — and **not** by bugchecking
(no minidump after 11:25 despite many attempts). Two pieces of PnP damage:

* `Application Popup`: **"There was error [DATABASE OPEN FAILED] processing the
  driver database."**
* `setupapi.log`: `#W104 Device "PCI\VEN_121A&DEV_0009..." required reboot:
  Query remove failed (install) **CfgMgr32 returned: 0x17**` (CR_REMOVE_VETOED) —
  a pending device-install state stuck on the Voodoo 5 6000's PCI node.

Safe Mode does minimal PnP, which is why it boots. Repair applied: `INFCACHE.1`
renamed so XP rebuilds the driver database, watchdog counters cleared, and
`Services\3dfxvs\Start=4` so the next normal boot comes up on VGA with our driver
out of the path. The filesystem was never dirty.

## 23. The Safe-Mode-only boot failure is NOT software (2026-08-13)

After §22 fixed the watchdog bug, the box still would only boot in Safe Mode. It is
worth recording how thoroughly that was eliminated, because the answer is hardware
and the next person will otherwise re-run all of it.

### The machine is capable of booting perfectly

The 13:47:54 session, from its own logs:

```
13:47:54  EventLog service started
13:48:19  retro_agent v1.28.0: main() entered
13:48:39  Hostname=P3-DUAL IP=192.168.1.133  Listening on TCP :9898
13:49:02  retrowall: arranged desktop icons     <- Explorer running, desktop up
13:57:10  USER32 1074 - CLEAN shutdown
```

Kernel → drivers → services → autologon → Explorer → desktop → agent listening, for
nine minutes, then a clean shutdown. **Not a hang, not a crash.**

### What the hang actually looks like

Photographed by the operator: the `/SOS` screen showing only

```
Microsoft (R) Windows (R) Version 5.1 (Build 2600: Service Pack 3)
2 System Processors [1024 MB Memory] MultiProcessor Kernel
Boot Logging Enabled
```

and **no driver lines at all**. `/SOS` prints every driver as it initialises, so the
stall is at the very first step of device init — which is also why `ntbtlog.txt`
never gained a normal-mode session (the log is never flushed).

### Eliminated, each by direct test

| Suspect | How it was excluded |
|---|---|
| 3dfx display driver | **`3dfxv5m.sys` RENAMED away** (cannot load) → identical hang |
| `d347bus`/`d347prt` (DAEMON Tools) | `Start=4` → identical hang |
| `speedfan` (boot-start, raw ISA/SMBus I/O) | `Start=4` → identical hang |
| Video output | the operator's photo proves the display works |
| Filesystem / registry | `fsutil` clean, chkdsk clean, no PendingFileRenameOperations |
| Driver database | `INFCACHE.1` rebuilt after "DATABASE OPEN FAILED" |
| A crash | **zero** bugchecks after 11:25; watchdog `BreakCount`/`EventCount` stayed **0** |
| Boot config | operator's photo shows NTLDR loaded the kernel with our `/sos /bootlog` entry |

**A real, self-inflicted bug was found and fixed along the way:** `net use Z:
\\192.168.1.122\files` had been saved **persistently**, with the username mangled to
the *local* account `P3-DUAL\voidsstr` (`HKCU\Network\Z`, `SaveConnections=yes`).
Every logon stalled in "Attempting to restore your network connections" — a
light-blue-screen hang, and Safe Mode does not restore mappings, which is exactly
the observed asymmetry. Removed, and `SaveConnections=no` set. **Never leave a
persistent `net use` on a fleet box.** It was not the whole story, though.

### Why the conclusion is hardware

The configuration that booted successfully at 13:47 — `3dfxvs`, `d347bus`,
`d347prt`, `speedfan` all disabled, `/basevideo` default — is **the same
configuration that then hung repeatedly**. Same software, different outcome, with
no fault of any kind recorded. That is intermittent hardware, and the timing is
unambiguous: the box ran for **days with the case open**, and broke immediately
after the PSU swap (300→380 W) *and* the lid going on with a new rear fan. Safe
Mode survives because it initialises far fewer devices.

Checks to make, cheapest first: **boot with the lid OFF** (newest variable; a heavy
4-chip card flexes easily when a panel is fitted), reseat the card, verify its ~80 W
auxiliary power connector on the new supply, and check no cable is routed against
the card.

**Box state left ready:** watchdog-fixed `3dfxv5d.dll` (968,932, md5 `dbba17b4949c`)
deployed, `3dfxv5m.sys` restored, all services re-enabled, watchdog counters zeroed
as a fresh instrument. `boot.ini` retains three entries (VGA+logged, normal+logged,
pristine original); the original is backed up at `/tmp/qa256/boot.ini.bak`.

## 24. Full XP boot-path audit — `NVStrap` in "Boot Bus Extender" (2026-08-13)

§23 concluded hardware. A systematic audit of **everything XP loads at boot** found a
far better candidate, and it had been sitting there the whole time.

### The find

`Win32_SystemDriver` enumeration (179 drivers) shows two third-party **boot-start**
(`Start=0`) drivers that have nothing to do with this machine's job:

```
Boot   NVStrap   Stopped   C:\WINDOWS\system32\drivers\NVStrap.sys   (dated 2009)
Boot   giveio    Running   C:\WINDOWS\system32\giveio.sys            (dated 1996)
```

`NVStrap` is **RivaTuner's NVIDIA low-level hardware-hooking driver** — and this box's
GeForce4 Ti 4600 was removed long ago (its `Query remove failed` entries are still all
over `setupapi.log`). `giveio` is a raw I/O-port access shim.

The decisive detail is `NVStrap`'s load group:

```
NVStrap:  Group = "Boot Bus Extender"   Start = 0
ServiceGroupOrder = System Reserved | Boot Bus Extender | System Bus Extender |
                    SCSI miniport | Port | Primary Disk | ...
```

**"Boot Bus Extender" is the SECOND group XP loads — before `PCI`, before storage,
before everything.** A driver that hangs there produces exactly the observed screen:
the `/SOS` kernel banner, and then **nothing** — no driver names, no boot log flushed,
no bugcheck, no event-log entry. It also explains the Safe Mode asymmetry perfectly:
Safe Mode loads only the `SafeBoot\Minimal` set, which does not include it.

It further explains the **intermittency** (§23 leaned on this for "hardware"): a
hardware-probing driver aimed at an absent card will behave differently depending on
what the AGP/PCI bus returns, so it can pass one boot and hang the next.

### What was done

Disabled (`Start=4`): **`NVStrap`**, **`giveio`**, and `speedfan` (also boot-start,
also raw ISA/SMBus port I/O, also unnecessary to boot). `d347bus`/`d347prt` (DAEMON
Tools) and `Si3114r5` (**the boot disk controller — must stay**) left enabled.

Rest of the audit, all clean: every Boot/System-start driver's binary **exists**;
`boot.ini` restored to the pristine original; `BootExecute = autocheck autochk *`;
no `PendingFileRenameOperations`; filesystem not dirty; `Shell`/`Userinit` stock;
autologon intact; **no persistent net mappings** (see §23 — mine was removed).

One more logon-time item worth knowing about, pre-existing and not mine:
`HKLM\...\Run` contains `MapShare = net use \\192.168.1.122\files /user:admin password`,
plus NVIDIA leftovers (`NvCplDaemon`, `nwiz`, `NvMediaCenter`) that `rundll32` into
NVIDIA DLLs on a 3dfx box. These run **after** logon so they cannot block boot, but
they do cost logon time.

### Driver state

Our custom stack is archived to `C:\RETRO3DFX_REMOVED\` (`3dfxv5d.dll`, `3dfxv5m.sys`,
`3dfxogl.dll`, `opengl32.dll`, `glide2x/3x.dll`), the live copies renamed `.removed`,
and `3dfxvs` disabled — so XP boots on plain VGA with **no 3dfx code loaded at all**.
**AmigaMerlin 2.5 SE** (Win2k/XP) is staged at `C:\AMIGAMERLIN\driver2K\`; its INF
lists our exact hardware ID:

```
"Amigamerlin 2.5 SE for Voodoo 4/5" = 3dfxvsV5,PCI\VEN_121A&DEV_0009&SUBSYS_0001121A
```

## 25. RETRACTION — §23 and §24 were not established (2026-08-14)

**§23 ("the boot failure is hardware") and §24 (NVStrap as culprit) are both
withdrawn.** Neither conclusion survives review of its own evidence. The box was
reimaged on 2026-08-14 before the question was settled, so the root cause is
**UNKNOWN** — not "hardware", and not NVStrap. Read this section before trusting
anything in §23–24.

### Why §23's eliminations do not eliminate

§23 lists eight suspects "eliminated, each by direct test". Every one of those
tests was a **single failed boot**. But §23 also documents the machine booting
normally once and running nine minutes. Both cannot be true of a deterministic
fault: **the failure is intermittent**, and against an intermittent fault a
1-sample negative has no discriminating power at all. `/PCILOCK`, `/ONECPU`,
`/BASEVIDEO`, the renamed 3dfx driver, the 17 disabled drivers, the 14 services —
none of them were actually ruled out. The conclusion "everything software is
eliminated, therefore hardware" was residual reasoning over an empty residue.

Three specific inference errors, each of which independently breaks the chain:

1. **"Zero `ntbtlog.txt` lines ⇒ it hung before driver loading" is a non sequitur.**
   `ntbtlog` is buffered and flushed only after the boot volume is mounted. Zero
   lines means "before the flush point" — which covers most of Phase-1 driver
   init, not just the first instant.
2. **"Normal mode now loads the same driver set as Safe Mode" is false.** That
   diff was `Win32_SystemDriver` (`Start=0/1`) against a Safe Mode `ntbtlog`.
   **All 70 `Start=3` PnP function/filter drivers were excluded by construction** —
   USB, HID, audio, chipset/AGP filters. They load in normal mode via PnP and never
   in Safe Mode. That tranche is the largest untested delta and was never touched.
3. **`/BASEVIDEO` is not `/NOGUIBOOT`.** Under `/BASEVIDEO` the kernel still drives
   the card through bootvid/Inbv. The Voodoo 5 6000 was being driven by video code
   in *every single failing boot*; only the 3dfx *function* driver was excluded.
   Video was never eliminated. Likewise `/ONECPU` is not a uniprocessor HAL.

`NVStrap` (§24) was never a valid suspect either: its `Boot Bus Extender` group
**is** present in `SafeBoot\Minimal`, so it loads in Safe Mode too — the mode that
always works.

### The one hard datum this cost us

At the hang (plain normal boot, `/SOS /BOOTLOG`, 2026-08-14): the screen stops on
the light-blue system-information banner, the spinner stops, and **the NumLock LED
does not respond**. Keyboard interrupts are not being serviced — so this is a hard
wedge (high IRQL, interrupts disabled, or a dead interrupt path), not a driver
blocked at PASSIVE_LEVEL waiting on something. *Caveat before over-reading it:* at
that stage the keyboard class driver may not be loaded, in which case a dead LED is
also the healthy behaviour. It was never A/B'd against a known-good boot.

Also established, and still true: all six `boot.ini` entries hang except Safe Mode;
Last Known Good (ControlSet002) hangs as well; `ntbtlog.txt` never contains a
normal-mode session; and the failure persists with the entire 3dfx stack archived
to `C:\RETRO3DFX_REMOVED\` and the `3dfxvs` service disabled — i.e. **with no 3dfx
code loading at all**. That last point is the one genuinely load-bearing negative
here, and it is what makes "our driver caused it" unlikely, though not impossible
via leftover registry/PnP state.

### Do this instead next time

- **Five boots per condition, not one.** An intermittent fault needs a pass/fail
  *rate*. A single hang proves nothing; a single success proves nothing either.
- **Arm the crash dump before you need it**, while the box still boots Safe Mode:
  `CrashOnCtrlScroll=1` under `Services\kbdhid\Parameters` (**`kbdhid`, not
  `i8042prt` — this box has a USB keyboard**), plus `CrashControl` `AutoReboot=0`
  and `CrashDumpEnabled=2`. Then `Ctrl+ScrollLock ScrollLock` at the hang yields a
  bugcheck with a real stack, which ends the guessing outright. This was the
  correct move and it was reached far too late.
- **Bisect the `Start=3` tranche** — it is the untested delta. USB first
  (`usbuhci`/`usbehci`/`usbohci`/`usbhub`/`usbstor`/`hidusb`), and physically
  unplug USB mass storage: a USB flash drive was present at every failing boot and
  USB enumeration is a classic XP early-boot hang.
- **Settle hardware vs software with one non-Windows test** — a Linux live USB or
  memtest86. That answers in a single boot what a dozen Windows reboots did not.
- **Baseline before you build.** On a fresh XP, confirm ~5 clean normal boots
  *before* installing the 3dfx stack, then re-confirm after each component. Without
  a known-good baseline there is nothing to attribute a regression to.

---

## 26. THE 4-WAY SCANOUT SKEW — the board is exonerated, the bug is ours (2026-09-04)

**Board moved.** The Voodoo 5 6000 is now in **192.168.1.191** — Athlon 1152 MHz /
nForce2, XP SP3, AGP, same card (4× VSA-100 @166 MHz behind a HiNT HB1-SE66
bridge, **128 MB VBIOS mode = 32 MB/chip**). Deployment record:
`DEPLOY-191-20260904.md`. Everything in §§1–25 was measured on `.133`; this
section is `.191`.

**The defect.** In any multi-chip config the card **renders** perfectly and
**scans out** wrong: horizontal strips displaced sideways. Single-chip is clean.
Raw log: `FINDINGS.md`, the 2026-09-04 entries.

### 26.1 VERIFIED ON HARDWARE — AmigaMerlin renders 4-way SLI correctly

**AmigaMerlin 3.1-R11 (a third-party retail-lineage driver) was installed on
`.191` alongside our stack and it renders 4-way SLI with NO skew**, confirmed on
the monitor by the operator. Same board, same bridge, same analog combine, same
host. **The hardware is fine and the scanout defect is in OUR driver.** Stop
looking for a hardware excuse; that ends weeks of ambiguity.

Q3 1.32c `demo four`, 640×480×16, vsync off, `.191`:

| driver | single chip | 4-way SLI | SLI scaling |
|---|--:|--:|--:|
| **AmigaMerlin 3.1-R11** (correct picture) | 116.5 | **151.5** | **1.30×** |
| ours (H5 + SGL ICD 0.5.0) (skewed picture) | 92.5 | 105.0 | 1.14× |

**fps convention — do not silently mix the two runs.** The table above is the
**paired** comparison: both stacks benchmarked back-to-back in the same session,
and it is the pair to quote for any driver-vs-driver delta (**−21 %** single
chip, **−31 %** 4-way). The later eight-point resolution sweep separately
measured AmigaMerlin 4-way at **152.6** (→ **1.31×** scaling) — that is the
figure the sweep-derived tables in `FINDINGS.md` and the sibling docs use. The
two agree to 0.7 %, well inside this box's ~3 % run-to-run noise, but quote
**151.5 / 1.30×** for the paired driver comparison and **152.6 / 1.31×** for the
sweep.

AmigaMerlin is faster in **both** configurations. The single-chip 92.5 vs 116.5
(**−21 %**) is a *separate* defect with nothing to do with SLI — it is measured
before any second chip is involved — and is tracked in `FINDINGS.md`,
2026-09-04 entry *OUR DRIVER IS 21% SLOWER THAN AMIGAMERLIN ON ONE CHIP*.

**Pass/fail metric that needs no eyes:** whether the timedemo *completes and
prints an fps line*. A driver that wedges the card never gets there, so both
rows above were produced unattended. `tools/v56k/trials/ambench.py` — which
**pins refresh to 60 Hz on both stacks** (`FX_GLIDE_REFRESH=60` *and*
`+set r_displayRefresh 60`), so these two rows are not subject to the capture
confound of §26.4.

#### Installing AmigaMerlin, for the record

Extract `amigamerlin_3.1_r11.exe` with 7-Zip (the payload is a plain 7z at
offset 78848) and install `driver2k/3dfxvs.inf` headlessly with `tools/drvupd.c`
against `PCI\VEN_121A&DEV_0009&SUBSYS_0001121A` — its INF carries a dedicated
`3dfxvsV6` section, *"AMIGAMERLIN 3.1-R11 For Voodoo 5 6000 AGP"*, for exactly
that HWID. Two dialogs must be clicked through even with the signing policy
relaxed (unsigned-driver warning, per-file Confirm File Replace). Our files
survive alongside it — different names (`3dfxv5d.dll`/`3dfxv5m.sys` vs its
`3dfxvs.dll`/`3dfxvsm.sys`) — and a full rollback set sits at
`C:\RETRO_AGENT\am-rollback\`.

**Trap:** AmigaMerlin's INF writes `FX_GLIDE_REFRESH = 75` into
`…\Device0\Glide`, which overrides the per-resolution refresh for **every** mode
and put the monitor out of range. Delete it (or set 60) before running anything.

### 26.2 The register diff — two registers differ, everything else is identical

Captured live with `fxscan2 dump` under both stacks, 4-way, 640×480:

| register | AmigaMerlin (CORRECT) | ours (SKEWED) |
|---|---|---|
| `vidProcCfg` | `03E60101` | `33E60101` |
| `pllCtrl0`   | `0000D137` | `0000B31F` |

`vidScreenSize`, `vidDesktopStride`, `vidDesktopStart`, `vidOvlEndCoord`,
`lfbMemoryConfig`, `vidOverlayDudx` and `vidOvlDudxOffSrcW` are **identical**
between the two stacks and across all four chips in both. `miscInit0` diverges
the same way in both (master `077C0000`, slaves `0`), which independently
confirms the slave Y-origin divergence is by design (§26.5, row 9).

**`vidProcCfg` differs by exactly `0x30000000` = BIT(28) | BIT(29)** — set in
ours, clear in AmigaMerlin. Decoded against the `vidProcCfg` table at
`Displays/H5/H3DEFS.H:1174-1224`:

| value | bits set | meaning |
|---|---|---|
| AmigaMerlin `03E60101` | 0,8,17,18,21,22,23,24,25 | VIDEO_PROCESSOR_EN, OVERLAY_EN, OVERLAY_FILTER_4X4, DESKTOP_PIXEL_RGB565, OVERLAY_PIXEL_RGB565D, DESKTOP_TILED_EN, OVERLAY_TILED_EN |
| ours `33E60101` | the same **plus 28 and 29** | + BIT(28) (**unnamed**) + `SST_OVERLAY_EACH_VSYNC` |

BIT(29) is `SST_OVERLAY_EACH_VSYNC` (`H3DEFS.H:1222`). **BIT(28) has no name in
any copy of `H3DEFS.H` in this tree** — the vidProcCfg block jumps straight from
`SST_CURSOR_EN` BIT(27) (`:1221`) to BIT(29). It is an undocumented bit.

`vidProcCfg` is directly pokeable at **IO offset 0x05C**, so both bits can be
trialled with **no miniport rebuild** — which matters, because the Wine build
tree is currently unusable (see `FINDINGS.md`, 2026-09-04 entry
*`setup-toolchain.sh` succeeds and produces an unusable Wine*).

### 26.3 Where bits 28/29 come from — 3dfx's own undocumented heat erratum

`Miniport/H5/h3modeset.c:667-672`, verbatim:

```c
  // Set Bit 28 and 29 on Napalm boards
  // This fixes a problem we were seeing with high-res modes in a heated environment.
  if ((IS_NAPALM) /*&& (66 == HwDeviceExtension->PciSpeed)*/)
    temp |= (BIT(28) | BIT(29));
  else
    temp &= ~(BIT(28) | BIT(29));
```

3dfx's own comment: these bits are an **erratum workaround for high-resolution
modes in a hot chassis** — the class of change that shifts when a chip latches
its scanout data. Note the **commented-out `66 == PciSpeed` gate**: the
workaround was once conditional on a 66 MHz PCI bus and someone at 3dfx widened
it to every Napalm board.

**Why the working 5500 does not disprove this.** `IS_NAPALM` is
`(0x06 <= PCIDeviceID)` (`Miniport/H5/H3.H:320`), and both the V5 5500 and the
6000 are `DEV_0009` — so `.143` sets these same bits and its 2-way SLI is clean.
The bits are therefore **not sufficient on their own**.

**And the AmigaMerlin comparison is NOT an A/B.** "Bits clear" was only ever
observed under a *different driver*, which moved `pllCtrl0` at the same time
(§26.4). One board, yes; one variable, no — it is a **two-variable comparison**.
A scanout-timing perturbation that 2 chips absorb and 4 chips (two of them behind
a HiNT bridge) do not would be consistent with every observation here — but so
would the refresh difference, and nothing yet separates them. These bits are the
**prime suspect; they are not a proven cause.**

### 26.4 UNRESOLVED CONFOUND — the comparison changed TWO variables

**Do not treat the erratum bits as the cause yet.** The AmigaMerlin capture also
ran at a different pixel clock. Decoding `pllCtrl0` with the VSA-100 PLL formula
`f = 14.31818 * (N+2) / ((M+2) * 2^K)`, where `N = bits[15:8]`,
`M = bits[7:2]`, `K = bits[1:0]`:

| stack | `pllCtrl0` | N / M / K | pixel clock | VESA 640x480 |
|---|---|---|--:|---|
| AmigaMerlin | `0000D137` | 209 / 13 / 3 | **25.176 MHz** | 25.175 = **60 Hz** |
| ours        | `0000B31F` | 179 /  7 / 3 | **35.994 MHz** | 36.000 = **85 Hz** |

Three-decimal agreement with the VESA clocks, so the refresh rates are certain,
not inferred.

**Where those two clocks came from — the 60-vs-85 gap is OURS, not the
drivers'.** The two captures were taken under different harnesses. The §26.1
benchmark rows ran under `tools/v56k/trials/ambench.py`, which **pins** refresh
(`FX_GLIDE_REFRESH=60` *and* `+set r_displayRefresh 60`). The **register dumps in
the table above were not taken under it**: they came from the trial scripts
(`trial_setup.py`, `ringrun.py`, `ringtrial.py`), **none of which sets any
refresh**, so our dump took the ICD's own per-resolution default of **85 Hz**.
The AmigaMerlin dump, by contrast, was captured while `FX_GLIDE_REFRESH` had been
forced to **60** to stop the monitor going out of range (the INF trap in §26.1).
**So the 60-vs-85 difference is a configuration difference this session
introduced between the two captures — not an intrinsic property of either
driver.** That makes it easy to control for, and it makes one rule mandatory:
**any future capture must pin refresh explicitly, or the comparison is
worthless.**

**Refresh rate is itself a scanout-timing variable** — at 85 Hz each chip has
~30 % less time per pixel, i.e. less margin for inter-chip skew — so in the two
dumps we hold, the erratum bits and the pixel clock are **confounded** and
neither is isolated.

**The decisive experiment is a 2×2**, and every cell is reachable by poking IO
`0x05C` and setting the refresh rate — **no miniport rebuild required**:

| | bits 28/29 SET | bits 28/29 CLEAR |
|---|---|---|
| **85 Hz** | known: **SKEWED** | ? |
| **60 Hz** | ? | AmigaMerlin-equivalent — **NOT RUN on our driver** |

- If **60 Hz + bits set** is clean → refresh is the cause, the bits are innocent.
- If **85 Hz + bits clear** is clean → the bits are the cause.
- If both are clean → either alone suffices.
- If neither is clean → both are needed, or the real cause is a third thing that
  differs between the stacks and was not in the dump.

**All four cells still have to be run on OUR stack, the bottom-right one
included.** AmigaMerlin's clean picture at 60 Hz with the bits clear is *not* a
substitute for that cell: it was produced by a different driver that differs from
ours in every other respect too (§26.1's −21 % single-chip deficit is the proof).
Until we run it, "60 Hz + bits clear" is unknown on our driver, not clean.

Run all four cells with a liveness gate (§26.6) and the operator watching the
monitor; the artefact has no software detector (§26.5, row 3).

### 26.5 ELIMINATED — consolidated. Do not re-propose any of these.

Every row was killed **on hardware on `.191`**, not by reasoning.

| # | hypothesis | what killed it |
|--:|---|---|
| 1 | GDI `SCREENSHOT` during exclusive fullscreen shows the bug | The same capture taken in **single-chip** mode — visually clean on the monitor — comes back just as interlaced. The garble is the CAPTURE. `gdi_fs_single.png` vs `gdi_fs_4way.png` |
| 2 | Windowed 3D + GDI capture instead | The window's client area captures **BLACK**; a Glide fullscreen-exclusive surface never composites into the GDI primary |
| 3 | Any framebuffer / pixel-diff detector | `tools/v56k/sligrid.c` read back its own bit-exact pattern with the monitor visibly skewed: `readback: 0 of 307200 pixels differ (0.0000%)`. Reads through the master's BAR1 are **SLI-gathered in hardware** (`CFG_SLI_RD_EN` set on every chip), so the card reassembles a flawless image for any reader. **Proven, not inferred — do not build another picture-based detector** |
| 4 | Use the display driver's registry flight recorder | The display driver is **not on this path** — a Glide fullscreen app makes it release the hardware. Replaced by `fxscan2 ring` (§26.7) |
| 5 | "The slaves never get `vidScreenSize`" (leading candidate of two research passes; 3dfx complains about it at `MINIHWC.C:4339`) | Read live during a 4-way 640×480 Q3 run: all four chips agree on `vidScreenSize`, `vidDesktopStride`, `vidProcCfg`, `vidDesktopStart`, `vidOvlEndCoord`, `lfbMemoryConfig`. The complaint is real and its workaround is broken, but `H3SetMode` on each slave already leaves the right geometry. **A patch was written and REVERTED — do not re-apply it blind** |
| 6 | SLI band height is wrong | Glide programs `band=3` (8 lines) and the same value reaches the miniport, so both sides agree. Forcing 16 via `FX_GLIDE_FORCE_SLI_BAND_HEIGHT` **rebooted the box**. Arithmetic says why: group = band × chips must divide screen height — at 640×480 4-way, band 8 → group 32 → 480/32 = 15 exact; band 16 → group 64 → 480/64 = **7.5** |
| 7 | The SLI row/column masks are wrong | 3dfx's own `H5/DOCS/Video SLI AA Configs.xls`, "4 chips, analog SLI": `rmask_fetch/rmask_crt = 0x30` on every chip, `cmask_fetch/cmask_crt = 0x00/0x10/0x20/0x30` for chips 0-3, `rmask_aafifo=0x0`, `cmask_aafifo=0xff`, `divide_video=1`. `Miniport/H5/SLIAA.C:2677-2724` computes exactly that shape. **No discrepancy** |
| 8 | The 2/4-way analog arm never tristates hsync (`CFG_DAC_HSYNC_TRISTATE` appears at `SLIAA.C` 1927/2609/2858/2929/2950/3002/3024/3111/3261/3332 — none inside the arm at 2677-2731) | The same spreadsheet's header states the whole table assumes `video_tv_output_en = dac_vsync_float = dac_hsync_float = 0`. The absence is **correct**, not an omission |
| 9 | Slave Y-origin: `miscInit0` master `077C0000` vs slaves `0` is a bug | Forcing chips 1-3 to the master's value **BLANKED THE SCREEN** (needed Ctrl-Alt-Del). By design: the master flips its origin (bottom-left for GL) while the slaves address *compacted band buffers* from 0. **"Slave register ≠ master register" is NOT automatically a bug** — that framing drove two whole research passes and is wrong for this hardware |
| 10 | The chip clocks are not locked / chip2 free-runs at +148 ppm | Re-measured from a clean boot **with the skew fully reproduced**: chip1 +0.000, chip2 +0.093, chip3 +0.047 ppm — all locked, picture still skewed. The +148 ppm was transient and does not reproduce. **Clock lock is not the fault and phase is not a usable proxy metric for it** |
| 11 | Restore Win9x's dropped 4-chip master `CFG_VIDPLL_SEL` branch (`MINIVDD/SLIAA.C:1690`, *"Special Case 4 way where master also needs to sync from slave"*; W2K `SLIAA.C:3421` has the slave branch and no `else`) | The master really is `FREERUN` (`cfgVideoCtrl0 = 00000001`, bit 11 clear) while chips 1-3 read `00000803` LOCKED — but **poking bit 11 on the master produced NO PICTURE AT ALL**. Per Databook 3.4.9 the master's `SYNC_CLK_IN`/`SYNC_CLK_FB` are grounded, so slaving its PLL to an undriven input stops its video clock. The Win9x branch presupposes a board that wires a slave's clock back to the master; this recreation does not. `optimized/v56k-sli-scanout-candidates/02-master-vidpll-sel.patch` is **NOT a fix** and is retained only as evidence |
| 12 | `vga_vsync_offset` (`cfgSliAaMisc[8:0]` = pixels[2:0] \| chars[5:3] \| hxtra[8:6]) has the wrong value — the **only** inter-chip horizontal alignment knob in the stack | Live and real: the slaves run **39 px** ahead of the master (`00000827`) and zeroing them to `0x800` **visibly MOVED the bands**. But no value observed so far removes the skew — **and the sweep never finished**. `tools/v56k/trials/vsyncsweep.py` aborts the moment the box goes unreachable (returns 3), and it did: only **7, 15 and 23 px** actually ran, all still skewed, before the **31 px** step hard-froze the board (§26.6) and stopped the run. With the shipped **39 px** default as the broken control that is **four observed values**, not a completed sweep — **47 px** (Case B, `chars=5`, `SLIAA.C:2489-2515`, a documented candidate but never a run), 55 px and 63 px were **never reached**. `vsyncsweep.py` now skips 31 px permanently. **Those four values are eliminated; the register is not** |
| 13 | The SLI hsync handover column `vidOverlayDudx` is 0 and 0 is wrong (Napalm r1.13 §11.1.21; the DDraw path sets `cxScreen >> 1` at `DDFXNT.C:2606`, the Glide path hardcodes `0UL` at `MINIHWC.C:4041`) | Poking it **visibly changes the artefact**, so it is a live lever — but eight configurations at 640×480 4-way (0/160/320/480/639 uniform; staggered 160/320/480/639; master-only 320; slaves-only 320) were **still skewed in every one**. Those eight values are eliminated; the register is not |

**Three non-levers, corrected from earlier premises in this document:**

- **`SSTH3_SLI_AA_CONFIGURATION` cannot select 2-way on this board.**
  `GPCI.C:1446` has no `case 2` and no `case 5` (both fall to `default:`),
  `GSST.C:1828` forces `sliCount=4` when `chipCount==4`, and `EnableSLIAA`
  refuses any request where `dwChips != numUnits` (`SLIAA.C:3510`). It is 4-way
  or single-chip, nothing between.
- **`FX_GLIDE_ANALOG_SLI=0` does not mean digital.** `GSST.C:2053` ends with an
  unconditional `if (gc->chipCount == 4) gc->bInfo->h3analogSli = 1;` and
  `MINIHWC.C:4277` re-forces it. **Analog was active in every skewed trial.**
- **`SSTH3_VIDEO_REFRESH_OPTIMIZATION` has zero references in the W2K tree** — it
  exists only in the Win9x MiniVDD. Setting it is a no-op.

**One piece of reasoning to retire.** An earlier research pass "eliminated"
`vga_vsync_offset` on the grounds that W2K, Win9x and DOS all program it
identically. **That reasoning is invalid**: identical across ports says nothing
about whether the value is right for a board 3dfx never shipped. Row 12 above
does the work properly — on silicon — but note what it actually eliminates: the
**four values observed**, not the register, which stays open (§26.8 item 4).

### 26.6 HARDWARE HAZARDS — every one of these was hit for real

Every row below was hit for real this session, all from poking a live scanout:
**two hard freezes** (both needing a physical power cycle), one self-recovering
reboot, and two recoverable blank screens.

| poke | consequence | note |
|---|---|---|
| `vga_vsync_offset` **pixels=7, chars=3 = 31 px** (`cfgSliAaMisc = 0x81F`) | **HARD FREEZE.** NIC dead, no route to host, **physical power cycle required** (ask the operator) | This is the `vga_crtc_fast` bug 3dfx documents at `SLIAA.C:2489-2494` — *"the vga_crtc_fast module has a bug in it which causes us to have to bump the vsyncOffsetChars field"* — **confirmed on silicon**. The shipped `chars=4` (39 px) bump is a genuine, necessary workaround. **Never program pixels=7 with chars=3.** `vsyncsweep.py` skips it permanently |
| `vidOverlayDudx` sweep, **first (ungated) version** | **HARD FREEZE.** NIC dead, **physical power cycle required** | A *separate* freeze from the one above. Because the sweep had **no liveness gate**, which of its eight steps did it is **unknown and unrecoverable** — that loss is why rule 1 exists. `tools/v56k/trials/dudxsweep2.py` is the gated rewrite; see §26.5 row 13 |
| `FX_GLIDE_FORCE_SLI_BAND_HEIGHT=16` at 640×480 4-way | box **rebooted** (recovered on its own) | See §26.5 row 6 |
| master `cfgVideoCtrl0` bit 11 (`vidpll_sel`) set | **no picture at all** | §26.5 row 11 |
| slave `miscInit0` forced to the master's `077C0000` | **screen blank**, Ctrl-Alt-Del to recover | §26.5 row 9 |

**Two operational rules that follow:**

1. **Liveness-gate every sweep.** The step that breaks the box **is** the
   finding — an ungated sweep just tells you the box is gone. Both halves of
   that lesson were paid for here: the `vga_vsync_offset` sweep **was** gated and
   its gate named the failing step exactly (31 px), while the earlier
   **ungated** `vidOverlayDudx` sweep froze the box and left no record of which
   step did it. Pattern: `tools/v56k/trials/dudxsweep2.py` (the gated rewrite),
   `tools/v56k/trials/vsyncsweep.py`. Note that a gated sweep **aborts** at the
   wedge, so the steps after it are unrun, not clean.
2. **Pokes persist until reboot and contaminate later runs.** Nothing restores
   per-chip scanout state when an app exits: after a game the slaves keep the
   *game's* geometry (observed: desktop at 1024×768 with the slaves still at
   640×480 from the previous run). A later trial at the same resolution will
   then "agree" by coincidence. **Reboot between conditions**, and treat any
   register reading taken outside the failing app as worthless.

### 26.7 The instruments (the reusable part)

- **`HWCEXT_GET_SLAVE_REGS` (0x19, `HWCEXT.H:565`, handled `HWCEXT.C:2732`)** is
  already answered by the **shipping** `3dfxv5d.dll` — no driver rebuild needed.
  It returns, per chip 0..3, the VAs of four register windows mapped
  **read/write** into the calling process, so per-chip scanout state can be both
  read and poked live, against a running fullscreen game.
  **Sequencing trap:** do **not** send `HWCEXT_ALLOCCONTEXT` first.
  `hwcGetLinearAddr` (`HWCEXT.C:785-825`) has an "if a GLIDESTATE already
  exists, return the old mapping" branch that returns the old, never-mapped
  bases and never fills `glideSlaveRegBase[]` — you get four zeros.
  **Order: `GETLINEARADDR` (0x03), then `GET_SLAVE_REGS` (0x19).**
- **`HWCEXT_PCI_OP` (0x18, `HWCEXT.C:2409`) IS exclusive-gated** — it needs
  `HWC_EXCLUSIVE`, i.e. it only works from inside a Glide app. That is why PCI
  **config**-space work (e.g. `CFG_VIDEO_CTRL0`) needs `tools/v56k/sligrid.c`
  (`--pci`, `--poke fn:OFF=VAL`) and cannot be done with `fxscan2 poke`.
  `GETLINEARADDR` and `GET_SLAVE_REGS` are **not** gated.
- **`fxscan2 dump` / `diff` / `phase` / `poke`** — `tools/v56k/fxscan2.c`, mingw,
  run through the agent.
- **`fxscan2 ring <secs> <interval_ms> <outfile>`** — the Glide-path equivalent
  of the display driver's registry flight recorder. Samples every watched
  register on all four chips, writes a line whenever any changes, plus a
  per-second `HB` heartbeat of each chip's `vidCurrentLine` so a stalled CRTC is
  visible. **Every line is flushed**, so a wedge still leaves the last state on
  disk — the entire point. **Start it before launching the game** so it captures
  the mode transition. A static dump cannot tell *"correctly programmed"* from
  *"stale but coincidentally equal"*; the ring can, and that distinction changed
  the conclusions (§26.5 row 5, §26.6 rule 2).
- **`tools/v56k/sligrid.c`** — draws a static bit-exact pattern from inside
  Glide and reads it back. Must be a Glide app because `HWCEXT_PCI_OP` is
  exclusive-gated. Supports `--poke <chip>:OFF=<val>` (multi-poke).
- **`tools/v56k/trials/ambench.py`** — automated Q3 timedemo harness;
  completion + fps is the pass metric, no eyes required.

**What the ring showed that no dump could:** across the desktop → Glide
transition at 640×480, the **master** receives the full geometry program
(`vidScreenSize`, `vidDesktopStride`, `vidDesktopStart`, `lfbMemoryConfig`,
`miscInit0`) while **chips 1-3 receive only `vidProcCfg` and `dacMode`**.

### 26.8 What is still open

1. **Run the 2×2 of §26.4.** It is the next action, it needs no rebuild, and
   until it is done "the erratum bits cause the skew" is a HYPOTHESIS.
2. **BIT(28) is unnamed anywhere in the tree.** If the 2×2 implicates the bits,
   isolate which of the two matters — they are independently pokeable.
3. **`vidOverlayDudx` is a live lever with an unknown correct value** for four
   chips; eight values are eliminated (§26.5 row 13), the register is not.
4. **`vga_vsync_offset` likewise** — the sweep moved the bands without fixing
   them, and the sweep **aborted at 31 px** after only four observed values
   (7 / 15 / 23 px plus the 39 px default; 47 / 55 / 63 px were never reached —
   §26.5 row 12). A value outside those four, or a *per-chip* pattern rather
   than a uniform one, has not been ruled out.
5. **The −21 % single-chip throughput deficit** (§26.1) is a separate bug and
   should not be conflated with this one.
