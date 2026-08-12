# The build → debug → deploy loop for the V5-6000 / vintage 3dfx stack

One place that ties the whole workflow together. Set up once with
[`setup-toolchain.sh`](setup-toolchain.sh); after that every iteration is
edit → build → deploy → read logs on the box.

The **Voodoo 5 6000 lives in `192.168.1.133` ("P3-DUAL")** — dual Pentium III, XP
SP3. It runs THIS repo's stack (miniport `3dfxv5m.sys` + display `3dfxv5d.dll` +
`glide3x.dll`/`glide2x.dll` + OpenGL ICD `3dfxogl.dll`). Driver debug logs land at
`C:\3dfxvs.log`, `C:\3dfxogl.log`, `C:\glide3x.log`; the flight-recorder ring is in
the registry (below).

## 1. Build

```bash
export RETRO3DFX_TC=$HOME/retro3dfx-toolchain
source toolchain-3dfx/build/env.sh
```

| artifact | script | output (under `$RETRO3DFX_TC/prefix/drive_c/3dfx`) | reboot to deploy? |
|---|---|---|---|
| Glide3 runtime | `build-glide3x.sh` | `H5/BIN/glide3x.dll` (96 exp) | no (user-mode) |
| Glide2 runtime | `build-glide2x.sh` | `H5/BIN/glide2x.dll` (133 exp) | no |
| OpenGL ICD | (build in `SWLIBS/OPENGL/GLIDE3X`, `nmake` → `release/opengl.dll`; rename `3dfxogl.dll`) | `release/opengl.dll` | no |
| miniport | `build-w2k.sh miniport` | `.../Miniport/H5/objfre/i386/3dfxvsm.sys` | **yes** |
| display+D3D HAL | `build-w2k.sh display` | `.../Displays/H5/objfre/i386/3dfxvs.dll` | **yes** |

Source edits go in the build copy under `$RETRO3DFX_TC/prefix/drive_c/3dfx/...`; a
shipped fix must also be committed as a delta under
`toolchain-3dfx/prefix/drive_c/3dfx/...` in the repo (that is what `setup-toolchain.sh`
overlays). The user-mode DLLs are the **safe** lane (a bad DLL just fails to load);
the miniport/display are **BSOD-at-boot risk — build freely, deploy only supervised.**

## 2. Deploy (over the retro agent)

Never raw-copy in-box names into `system32` — XP's WFP reverts them. Ship the
WFP-safe renamed names (`3dfxv5d.dll`/`3dfxv5m.sys`); user-mode DLLs swap with no
reboot. Full procedure = the **`deploy-3dfx-driver`** skill. Quick manual swap:

```python
# client/retro_protocol.py — RetroConnection('192.168.1.133', 9898), secret 'retro-agent-secret'
UPLOAD C:\RETRO_AGENT\stage\glide3x.dll         # then, for a loaded DLL:
EXEC cmd /c copy /Y C:\RETRO_AGENT\stage\glide3x.dll "C:\Games\Quake III Arena\Quake3\glide3x.dll"
EXEC cmd /c copy /Y C:\RETRO_AGENT\stage\glide3x.dll C:\WINDOWS\system32\glide3x.dll
```
Game-local shadows system32 — update EVERY game copy, then verify by the renderer
string (`GL_RENDERER: 3Dfx [retro3dfx x.y.z]`), never by file size. Kill the GL game
first. **Never `taskkill /f` a fullscreen Glide2 game** (mid-FIFO kill hangs the chip).

## 3. Debug

- **Driver logs**: `DOWNLOAD C:\3dfxvs.log` (display/DDraw/D3D), `C:\3dfxogl.log`
  (ICD), `C:\glide3x.log` (glide). `Retro3dfxLog=1` under the service key enables them.
- **Flight-recorder ring** (survives reboots): `REGREAD HKLM
  SYSTEM\CurrentControlSet\Services\3dfxvs\Device0` → `RLog00..RLog31` (REG_BINARY
  UTF-16LE) + `RLogSeq`; newest slot = `(RLogSeq-1)&31`. Signal lines:
  `H3MakeRoom STALL/WEDGE-BREAK`, `DdFlip WEDGE-BREAK`, `COMPUTE/PROMOTE/DEMOTE-SLIAA`,
  `DDRAW-ENABLED units=N` (N = detected VSA-100 chips — the 6000 shows **units=4**).
- **Multi-chip levers** (same service key / env): `SSTH3_SLI_AA_CONFIGURATION`
  (0 single-chip, 2 2-way, 5 4-way SLI, 6/7/8 AA modes), `FX_GLIDE_NUM_CHIPS` (can
  only REDUCE — force 1/2 chips to isolate an SLI fault), `FX_GLIDE_SLI_BAND_HEIGHT`.
- **csim-native** (`../csim-native/`): 3dfx's own VSA-100 C-model built natively on
  Linux (`-m32`) — per-pixel `GDBG_LEVEL` tracing, no hardware. For rasterizer/texcoord
  bugs, reproduce here in seconds instead of the build→deploy→screenshot loop.
- **In-engine screenshots only** for fullscreen Glide/GL — a GDI `SCREENSHOT` of an
  exclusive-fullscreen surface comes back garbled (looks like a driver bug, isn't).

## Reproduce-the-deployed-binary check (2026-08-11)

A from-scratch rebuild on a fresh host matched the binaries actually deployed on
`.133`: `glide2x.dll` 258,048 B / 133 exports and `3dfxvsm.sys` 199,656 B both equal
the on-box size; `glide3x.dll` links to 96 exports. (The deployed OpenGL ICD is
`0.5.0`; the source at that version was lost with the OMEN dev-box NVMe, so the ICD
rebuilds from `0.4.0`/pristine — see `../../optimized/deployed-133-v56k-20260811/`.)
