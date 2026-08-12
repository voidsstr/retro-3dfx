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
