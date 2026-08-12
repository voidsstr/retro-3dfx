# 256MB Readiness Plan — Voodoo 5 6000 (4× VSA‑100, HiNT HB1‑SE66, Windows XP)

Source tree: `/home/voidsstr/retro3dfx-toolchain/prefix/drive_c/3dfx`
Live Windows tree is `H5/W2K/Src/Video/…` (the `H5/WinNT/Src/Video/…` copies are not built for XP).

---

## 1. Verdict

**YES — 256MB mode (64MB/chip) is safe to attempt, but only after change #1 below is in place.**

The stack is fundamentally 64MB/chip-clean: `H3DetermineMemorySize` sizes 128Mbit SGRAM parts correctly, every heap DWORD has headroom, and the hardware address fields (26‑bit buffer base, 26‑bit munged texture base, 27‑bit LFB aperture) land **exactly** at the 64MB/128MB ceiling with zero bits to spare. Nothing overflows at 64MB/chip.

The single blocking defect is that **`AdapterMemorySize` is trusted without ever being checked against the PCI BAR that has to hold `2 × AdapterMemorySize`.** In 256MB mode that value doubles and is fed straight into `VideoPortGetDeviceBase`, into the master's `memBase1` decode field, and into the physical address at which all three slave BAR1s are parked. If the 256MB VBIOS does not also widen BAR1 to 128MB, the chip decodes 128MB out of a 64MB PnP allocation and the slaves claim addresses nobody reserved — overlapping decode with the neighbouring PCI device, i.e. bus hang or hard reset before video comes up.

That is one clamp, in one function, at one call site. With it, the worst outcome of flipping the switch is "behaves like the smaller size", not "board reboots the machine".

Two verified refinements to the plan as drafted:

- **`H3DetermineMemorySize` is called only once on this build.** The second call at `H3.C:744` is inside `#ifndef SLI_AA`, and `SLI_AA` is defined (`Miniport/H5/SOURCES:77`). So a clamp inserted after `H3.C:1611` is not undone later. Confirmed.
- **The clamp must round down to a power of two.** `GetDecodeSize()` (`SLIAA.C:971‑1020`) switches on `dwSize >> 20` and its `default:` arm returns **`CFG_MEMBASE0_128MB_DECODE`** (`SLIAA.C:1014‑1016`). A clamp to a non‑power‑of‑two BAR length would silently program a *128MB* decode — the exact failure the clamp exists to prevent. `MEMBASE1_DECODE_SIZE` and `MEMBASE1_MASTER_TO_SLAVE_SPACING` are both literally `2*HwDeviceExtension->AdapterMemorySize` (`SLIAA.C:92,99`), so this one value drives decode, slave placement, and mapping alike.

`DetectNumUnits` runs at `H3.C:652`, before `H3MapAccessRanges` at `:697`, so `numUnits` is valid inside the new log line.

---

## 2. The change set, in order

All paths below are relative to `/home/voidsstr/retro3dfx-toolchain/prefix/drive_c/3dfx/H5/`.

**Line endings, verified by counting `\r$` per file:**

| file | EOL |
|---|---|
| `W2K/Src/Video/Miniport/H5/H3.C` | CRLF (4419/4419) |
| `W2K/Src/Video/Miniport/H5/SLIAA.C` | CRLF (3702/3702) |
| `W2K/Src/Video/Displays/H5/ENABLE.C` | CRLF (2836/2836) |
| `W2K/Src/Video/Displays/H5/DDFXNT.C` | **LF‑only** (0/3911) |
| `W2K/Src/Video/Displays/H5/DDINIT.C` | CRLF (1741/1741) |
| `MINIHWC/MINIHWC.C` | CRLF (7321/7321) |
| `GLIDE/SRC/GTEX.C` | CRLF (1418/1418) |
| `GLIDE3/SRC/GSST.C` | CRLF (4000/4001) |
| `GLIDE3/SRC/GTEX.C` | **mixed** — only 134 of 3639 lines are CRLF; **lines 2805‑2830 and 3160‑3175 are all LF**, so both edits below are LF |

---

### Change 1 — clamp `AdapterMemorySize` to the real BAR1 length — **MANDATORY**
**Risk class: machine-reset (this change prevents it)**
`W2K/Src/Video/Miniport/H5/H3.C`, insert immediately after line **1616** (the closing `}` of the `H3DetermineMemorySize` error check), before the `// map frame buffer` comment at `:1618`. CRLF.

```c
  //
  // V56K-256MB: everything downstream derives from AdapterMemorySize*2 --
  // the VideoPortGetDeviceBase below, MEMBASE1_DECODE_SIZE and
  // MEMBASE1_MASTER_TO_SLAVE_SPACING in SLIAA.C (both literally
  // 2*AdapterMemorySize).  Nothing validates it against the BAR that PnP
  // actually reserved.  Clamp it, rounded DOWN to a power of two: any other
  // value falls through GetDecodeSize()'s default: arm to a 128MB decode.
  //
  {
    ULONG bar1Len    = HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeLength;
    ULONG maxPerChip = bar1Len >> 1;          // need 2x for the tiled aperture
    ULONG pow2       = 0x00400000;            // 4MB floor

    while ((pow2 << 1) <= maxPerChip)
      pow2 <<= 1;

    if (HwDeviceExtension->AdapterMemorySize > pow2)
    {
      VideoDebugPrint((0, "3dfx MEM: CLAMP perChip=%08lXh -> %08lXh bar1Len=%08lXh\n",
                       HwDeviceExtension->AdapterMemorySize, pow2, bar1Len));
      HwDeviceExtension->AdapterMemorySize = pow2;
    }

    VideoDebugPrint((0, "3dfx MEM: perChip=%08lXh bar1Len=%08lXh need2x=%08lXh units=%ld total=%08lXh\n",
                     HwDeviceExtension->AdapterMemorySize,
                     bar1Len,
                     HwDeviceExtension->AdapterMemorySize * 2,
                     HwDeviceExtension->numUnits,
                     HwDeviceExtension->AdapterMemorySize * HwDeviceExtension->numUnits));
  }
```

This is what the dead `H3.C:2075` line was for (`#if ENABLE_UNATTENDED_INSTALL_CHECK`, which is 0 on W2K/XP per `H3.H:1654‑1655`). It makes the whole stack degrade gracefully to whatever the BAR really is.

Note the registry value written at `H3.C:2062` is computed *inside* `H3DetermineMemorySize`, i.e. **before** this clamp. If the clamp fires, `HardwareInformation.MemorySize` will report the unclamped size while the driver runs clamped. That mismatch is itself a useful tell — see §3. Do not "fix" it by moving the clamp inside `H3DetermineMemorySize`; the `VideoDebugPrint` above is the authority.

---

### Change 2 — mask the AA aperture fields
**Risk class: corruption (hardening; no behaviour change at 64MB)**
`W2K/Src/Video/Miniport/H5/SLIAA.C:2443‑2456`. CRLF.

Replace lines **2443‑2449**:
```c
      PCI_CFG_WR(CFG_AA_LFB_CTRL,
                 (((aaSecondaryBuffersBegin << CFG_AA_BASEADDR_SHIFT) & CFG_AA_BASEADDR) |
                  CFG_AA_LFB_CPU_WR_EN |
                  CFG_AA_LFB_DPTCH_WR_EN |
                  CFG_AA_LFB_RD_EN |
                  dwFormat),
                 h3ChipSetupDeviceNum);
```
and lines **2453‑2456**:
```c
      PCI_CFG_WR(CFG_AA_ZBUFF_APERTURE,
                 ((((aaDepthBufferBegin >> 12) << CFG_AA_DEPTH_BUFFER_BEG_SHIFT) & CFG_AA_DEPTH_BUFFER_BEG) |
                  (((aaDepthBufferEnd   >> 12) << CFG_AA_DEPTH_BUFFER_END_SHIFT) & CFG_AA_DEPTH_BUFFER_END)),
                 h3ChipSetupDeviceNum);
```

Why: `CFG_AA_BASEADDR` is `0x3FFFFFF` (26 bits) and `CFG_AA_LFB_CPU_WR_EN` is `BIT(26)` (`SLIAA.H:417‑419`). An AA secondary base of exactly 64MB would land on the control bit and lose the base entirely. `CFG_AA_DEPTH_BUFFER_BEG` is 15 bits (`0x7FFF`, `SLIAA.H:412`); `64MB>>12 = 0x4000` fits with one bit to spare, so an off‑by‑one in buffer placement spills BEG into END. Both are one‑line masks that change nothing at legal values.

---

### Change 3 — Glide2 texture base wraps at 32MB
**Risk class: corruption**
`GLIDE/SRC/GTEX.C`, CRLF, two sites.

Line **1159**, in `grTexDownloadMipMapLevel`:
```c
  baseAddress = SST_TEXTURE_MUNGE_ADDRESS(baseAddress);
```
Line **1342**, in `grTexMultibaseAddress` — change the tail of the expression:
```c
      const FxU32
        baseAddress = SST_TEXTURE_MUNGE_ADDRESS(gc->tramOffset[tmu] +
                       _grTexCalcBaseAddress(startAddress,
                                             largeLevelLod,
                                             info->aspectRatio,
                                             info->format,
                                             evenOdd));
```

`SST_TEXTURE_ADDRESS` is `(SST_MASK(21)<<4)|BIT(1)` — bits 4..24 plus bit 1, i.e. **25 usable address bits = 32MB**. Address bit 25 is only reachable through `SST_TEXTURE_MUNGE_ADDRESS` (`INCSRC/H3DEFS.H:1039`), which relocates it to bit 1 because bits 25‑31 are `SST_TEXTURE_TILESTRIDE`. With texture RAM spanning roughly 1MB→60MB at 64MB/chip, an unmunged base both wraps the address *and* corrupts the tile stride. `SST_TEXTURE_MUNGE_ADDRESS` is already in scope in this file (same header supplies `SST_TEXTURE_ADDRESS`).

---

### Change 4 — same bug in Glide3 `grTexMultibaseAddress`
**Risk class: corruption**
`GLIDE3/SRC/GTEX.C:3162‑3168`, **LF endings on these lines**:
```c
      const FxU32
        baseAddress = SST_TEXTURE_MUNGE_ADDRESS(memInfo->tramOffset +
                       _grTexCalcBaseAddress(startAddress,
                                             largeLevelLod,
                                             G3_ASPECT_TRANSLATE(info->aspectRatioLog2),
                                             info->format,
                                             evenOdd));
```
This makes it match what `GTEX.C:2811` and `:2825` in the same file already do.

---

### Change 5 — logging (see §3 for the exact lines)
**Risk class: cosmetic**
`ENABLE.C` ×2 (CRLF), `GLIDE3/SRC/GSST.C` ×1 (CRLF).

---

### Change 6 — hygiene, not required for the flip
**Risk class: cosmetic — all dead on Windows; do these separately, after the flip is proven**

| file:line | change |
|---|---|
| `MINIHWC/MINIHWC.C:1682`, `:1713` | `32*1024*1024` → `bInfo->h3Mem << 20`. DOS/Linux `#else` branch only (`:1674`), not compiled on Windows (`HWC_EXT_INIT` is set at `MINIHWC/MAKEFILE:51`) |
| `MINIHWC/MINIHWC.C:1724`, `:1732` | silent `bInfo->pciInfo.numChips = 1` on slave-map failure → return `FXFALSE` with an `errorString`. Never silently discard 4‑way SLI |
| `MINIHWC/MINIHWC.C:7007` | `sfcOffset > 0x2000000` → `> (bInfo->h3Mem << 20)`. Debug builds only (`GDBG_INFO_ON` via `GLIDE3/SRC/MAKEFILE:87`), but it `exit(-1)`s |
| `MINIHWC/MINIHWC.C:1899‑1906` | Napalm `h3Mem > 16 → 16` clamp; dead under `#if !defined(HWC_ACCESS_DDRAW)` at `:1873` |
| `GLIDE3/SRC/GSST.C:2413`, `:2571`; `GLIDE/SRC/GSST.C:1911/1914`, `2059/2062` | hardcoded `bInfo->h3Mem = 32` — CSIM/HAL paths only, dead on real hardware |
| `Miniport/H5/H3.H:376` | `SST_RAW_LFB_ADDR (0x1FFFFFF<<0)` — 25 bits, vs `0x7FFFFFF` in `Displays/H5/H3DEFS.H:1443`. Zero users today; fix or delete before someone uses it |
| `SLIAA.C:1165` vs `:1425` | `:1165` parks all slaves at `master + 2*AdapterMemorySize` (no `* functionNumber`), `:1425` uses `* i`. Pre-existing asymmetry, unchanged by 256MB — noted only so it is not mistaken for a new bug |

**Do not "clean up"** the open-coded 26‑bit munges at `DDOVL32.C:2445`, `DDSLIBLT.C:1975`, `D3TXTR.C:3339‑3343` — they are correct for 64MB. **Do not raise** the `ENABLE.C:2437‑2449` guard (`totalLinearAddrSpace > 2*cjBank`); it is what keeps tiled addressing inside the 27‑bit LFB aperture.

---

## 3. What to log, and the exact expected values in both modes

### 3a. Miniport — `VideoDebugPrint`, kernel debugger / `DbgView`
Emitted by Change 1, `H3.C` after `:1616`:
```
3dfx MEM: perChip=%08lXh bar1Len=%08lXh need2x=%08lXh units=%ld total=%08lXh
```

| field | 128MB switch position | 256MB switch position | wrong = |
|---|---|---|---|
| `perChip` | `02000000h` | `04000000h` | anything else |
| `bar1Len` | `≥ 04000000h` | **`≥ 08000000h`** | `04000000h` in 256MB mode ⇒ **VBIOS did not widen BAR1, abort** |
| `need2x` | `04000000h` | `08000000h` | must be ≤ `bar1Len` |
| `units` | `4` | `4` | `1` ⇒ slave init already failed |
| `total` | `08000000h` | `10000000h` | — |

`3dfx MEM: CLAMP …` must **not** appear in a healthy 256MB boot. If it does, the flip did not take and you are running clamped — safe, but not 256MB.

Also present, from `H3.C:2067`: `3dfx memory size = 08000000h` → **`10000000h`**. This is the *pre-clamp* value; if `CLAMP` fired, this line and `perChip*units` will disagree, which is the tell.

### 3b. Display driver — `retroLogForce`, `C:\3dfxvs.log` (≤200 chars, `%ld`/`%08lXh` only)

**New, `ENABLE.C` immediately after line 1538** (CRLF), the decisive line:
```c
#if ENABLE_LOG_FILE
    retroLogForce(ppdev, "retro3dfx VRAM: cjBank=%08lXh ramLen=%08lXh need=%08lXh\r\n",
                  (DWORD)ppdev->cjBank,
                  (DWORD)VideoMemoryInfo.VideoRamLength,
                  (DWORD)(2 * ppdev->cjBank));
#endif
```
`VideoRamLength` is `MemBase1Length` (`H3.C:2727`); `FrameBufferLength` is `AdapterMemorySize` (`H3.C:2735`). Place it **before** the `MemSizePerChipOverride` block at `:1540` so it reports the hardware value, not the override.

| | 128MB | 256MB |
|---|---|---|
| `cjBank` | `02000000h` | `04000000h` |
| `ramLen` | `≥ 04000000h` | **`≥ 08000000h`** |
| `need` | `04000000h` | `08000000h` |

**If `ramLen < need`, stop.** Everything past that point is undefined — `DDFXNT.C:333` will request a `ViewSize` of `2*cjBank`, `H3.C:3218‑3219` will reject it against `MemBase1Length`, and every `DdMapMemory` fails.

**New, `ENABLE.C` after line 2450** (CRLF), after the `TryItAgain` loop settles:
```c
#if ENABLE_LOG_FILE
    retroLogForce(ppdev, "retro3dfx MEMCFG: lin=%08lXh tiled=%08lXh desk=%08lXh nbuf=%ld tot=%08lXh\r\n",
                  (DWORD)_FF(ddLinearHeapSize), (DWORD)_FF(ddTiledHeapSize),
                  (DWORD)_FF(gdiDesktopStart),  (LONG)_FF(ddNumColorBuff),
                  (DWORD)totalLinearAddrSpace);
#endif
```
Expect `lin` to roughly **quadruple** and `nbuf` **not** to drop. A drop in `nbuf` means the `totalLinearAddrSpace > 2*cjBank` guard fired — which at 64MB/chip means something is asking for more than the 128MB aperture.

**Already present and sufficient:**

| log | source | 128MB | 256MB |
|---|---|---|---|
| `DDRAW-ENABLED: units=%ld cyMem=%ld slop=%ld ddHeap=%ld` | `DDFXNT.C:791` (**LF file**) | `units=4 cyMem=4095 slop=5 ddHeap=3298` | `units=4 cyMem=4095` — **both unchanged**; `cyMemory` is clamped to `0xfff` at `ENABLE.C:1342‑1349`, so the GDI/DFB heap gains nothing. All growth appears in `cyDDMemoryExtra`/`ddLinearHeapSize` |
| `HEAP%ld …` / `HEAPSUM …` | `DDINIT.C:1410‑1424` | linear heap spans ~14MB | linear heap spans **~46MB** |
| `DdMapMemory SHARE-FAIL viewSize=…` | `DDFXNT.C:355` | absent | **must stay absent**; `viewSize=08000000h` if it appears |
| `DDAVAILMEM` | `DDFXNT.C:1681` | — | `GetAvailableVidMem` totals should stop reading 0 once heaps are non-degenerate |

### 3c. Glide — `_grSliLog`, always on
**New, `GLIDE3/SRC/GSST.C`**, inside the existing one-shot `if (v56kSig != v56kLastSig)` block at `:3868‑3873` (CRLF), right after the `SLICTRL` line:
```c
      _grSliLog("MEMCFG h3Mem=%uMB fbOff=0x%lx tramOff=0x%lx tramSize=0x%lx fifo=0x%lx\n",
                gc->bInfo->h3Mem,
                (unsigned long)gc->bInfo->fbOffset,
                (unsigned long)gc->bInfo->tramOffset,
                (unsigned long)gc->bInfo->tramSize,
                (unsigned long)gc->bInfo->fifoInfo.fifoStart);
```
Field names verified: `h3Mem`, `fbOffset`, `tramOffset`, `tramSize` in `hwcBoardInfo` (`MINIHWC/MINIHWC.H:465‑472`); `fifoStart` in `hwcFifoInfo` (`:353`). `gc->bInfo->h3Mem` is already dereferenced this way at `GSST.C:2127`.

| field | 128MB | 256MB | note |
|---|---|---|---|
| `h3Mem` | `32MB` | **`64MB`** | comes from `ExtEscape(HWCEXT_GETDEVICECONFIG)` → `ppdev->cjBank` → `AdapterMemorySize`; **no source change needed** |
| `fifo` | ~`0x18000` | ~`0x18000` | **unchanged** — FIFO stays pinned low. `GSST.C:2723` writes `fifoOffset>>25` into a field that only holds up to 32MB; never relocate the FIFO |
| `tramOff` | ~`0x100000` | ~`0x100000` | unchanged (`MINIHWC.C:2074`, `MAXFIFOSIZE_16MB` + pad) |
| `fbOff` | X | **X + `0x2000000`** | buffers are placed top-down from `h3Mem<<20`; at the same resolution the delta is exactly 32MB |
| `tramSize` | ≈`0x0E00000` (14MB) | **≈`0x2E00000`** (46MB) | `fbOffset − tramOffset`; grows by exactly 32MB |

And unchanged, from `GSST.C:3870`:
```
SLICTRL chips=4 sli=4 divisor=1 band=5 renderMask=0x60 log2=2
```

**Success signature, all four together:** `perChip=04000000h` with no `CLAMP`, `VRAM: cjBank=04000000h ramLen>=08000000h`, `MEMCFG h3Mem=64MB tramSize≈0x2E00000`, `SLICTRL chips=4 sli=4 … log2=2`, and `HardwareInformation.MemorySize = 0x10000000`.

---

## 4. Bring-up procedure

**Before touching the switch**

1. Build and deploy Changes 1–5 **while still in 128MB position**. Confirm the log signature matches the 128MB column exactly and that `CLAMP` does not fire. This proves the new code is inert at the known-good size, which is the only way to attribute a later failure to the switch rather than to the patch.
2. Archive the working driver set and the current `C:\3dfxvs.log`. Keep a second known-good display adapter (or confirm onboard video) physically available.
3. Verify F8 → **Enable VGA Mode** boots this machine, and note the exact keystroke timing. A bad `memBase1` decode can hang before any video output; VGA mode is the only in-band recovery.
4. Pre-stage the registry value so the *display* side behaves identically to today on first 256MB boot:
   `HKLM\System\CurrentControlSet\Services\<3dfx service>\Device0\MemSizePerChipOverride = 32` (DWORD)
   Read by `GetRegDWORD` at `ENABLE.C:1544`; it only ever shrinks `ppdev->cjBank` (`:1547` guards against growing it) and only in the display driver. The **miniport is unaffected**, which is exactly what you want on the first boot: the miniport/BAR/decode path runs at its real 64MB while the display driver behaves like 32MB.

**Flip**

5. Power off, unplug, move the dual‑VBIOS switch to its 256MB position, reseat, power on.
6. **First boot: expect a POST/VGA-only outcome and be ready for it.** Read the miniport `3dfx MEM:` line first. `bar1Len < 08000000h` ⇒ the 256MB VBIOS did not widen BAR1; the clamp saves you, but there is no point continuing — flip back.
7. If `bar1Len ≥ 08000000h` and no `CLAMP`, check `retro3dfx VRAM:` in `C:\3dfxvs.log`. With the override still at 32, `cjBank` will read `02000000h` while `ramLen` reads `08000000h`. That is the pass condition for this step: miniport sees 64MB/chip, display driver is deliberately behaving like 32MB.
8. Remove `MemSizePerChipOverride`, reboot. Now `cjBank=04000000h`, `MEMCFG h3Mem=64MB`, `lin` quadrupled, `nbuf` unchanged.
9. Only then run workloads: a Glide title first (Glide exercises the texture-base munge hardest — Changes 3 and 4 are what keep it from corrupting), then `ddtest.exe` for `GetAvailableVidMem`, then UT99 D3D.

**How to tell quickly that Glide silently fell back to a single chip**

The stack has *many* silent `numUnits = 1` paths (`SLIAA.C:1062, 1079, 1115, 1131, 1183, 1231, 1251, 1268, 1283, 1298, 1314, 1392, 1442, 1482`) and, on the DOS path, `numChips = 1` at `MINIHWC.C:1724/1732`. In descending order of speed:

1. **`C:\3dfxvs.log`, `DDRAW-ENABLED: units=…`** (`DDFXNT.C:791`). `units=1` ⇒ the miniport already gave up on the slaves; the display driver never saw four chips. Fastest single check, no game needed.
2. **Glide log, `SLICTRL chips=4 sli=4 divisor=1 band=5 renderMask=0x60 log2=2`** (`GSST.C:3870`). Anything other than `chips=4 sli=4` — or the line `SLICTRL-BAIL` (`GSST.C:3844`) — means Glide is not running 4‑way. `chips=4 sli=1` is the classic "chips found, SLI not engaged" state.
3. **`MEMCFG h3Mem=`**. If this says `32MB` while the miniport said `perChip=04000000h`, the escape path (`MINIHWC.C:1262‑1276`) or an `FX_GLIDE_FBRAM` override (`:1329‑1331`) is overriding you. Check the environment before blaming the driver.
4. **Registry `HardwareInformation.MemorySize`.** `0x10000000` = four chips at 64MB. `0x04000000` = one chip at 64MB — an unambiguous single-chip fallback.
5. **Frame rate.** A ~4× drop with everything else nominal is 4‑way SLI silently gone; confirm with (2), do not diagnose from framerate alone.
6. Ensure `FX_GLIDE_NUM_CHIPS` is **unset** for these runs (open item #11 — it hard-wedges a 4-chip board), and `FX_GLIDE_FBRAM` unset so `h3Mem` reflects hardware.

---

## 5. Rollback

| step | rollback | reversibility |
|---|---|---|
| Change 1 (`H3.C` clamp) | Delete the inserted block. Restores today's unchecked behaviour. **Do not roll this back while the switch is in the 256MB position** — it is the only thing standing between a too-small BAR and an overlapping PCI decode | trivial; miniport rebuild + reboot |
| Change 2 (`SLIAA.C` masks) | Remove the three `& CFG_AA_*` terms. Provably no behaviour change at legal values, so rollback is a no-op in practice | trivial |
| Changes 3, 4 (texture munge) | Restore `& SST_TEXTURE_ADDRESS`. Reintroduces the 32MB wrap; harmless while `h3Mem == 32`, corrupting at 64 | trivial; Glide DLL swap only, no reboot |
| Change 5 (logging) | Delete the three added blocks. `retroLogForce` sites are guarded by `#if ENABLE_LOG_FILE`; the Glide site is inside the existing one-shot guard so it costs nothing per frame. Safe to leave in permanently | trivial |
| Change 6 (hygiene) | Per-hunk revert. All dead on Windows by construction | trivial |
| `MemSizePerChipOverride = 32` | Delete the registry value, reboot. Only accepts 8/16/32/64 and only shrinks (`ENABLE.C:1547‑1552`), so it cannot make things worse | reboot |
| **Physical switch** | Power off, move the switch back to 128MB, reboot. **This is the master rollback** and it undoes every 256MB-specific code path at once, because every one of them keys off `AdapterMemorySize` | power cycle |
| **Unbootable** (no video after the flip) | Power off → switch back to 128MB → boot. If still dead: F8 → Enable VGA Mode → uninstall the display adapter → reboot → reinstall the archived driver set from step 2 | requires physical access; keep the case open for the duration |

Commit each change separately so any single one can be reverted without disturbing the others, and tag the last known-good 128MB build before the flip.