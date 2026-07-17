# 08 — Debugging, Tracing & Diagnostic Tooling

Every debugging facility in the tree, how to turn it on, and what it tells you.

## 1. GDEBUG — the universal trace system (`H5/INCLUDE/GDEBUG.H`, `GDEBUG.C` copies per module)

Compile-time gated (`GDBG_INFO_ON`, set in DEBUG builds): `GDBG_INFO(level, fmt, ...)`
messages are filtered by per-level enables; retail builds compile them to nothing
(`0 && (unsigned long)` trick).

- Control at runtime: `GDBG_SET_DEBUGLEVEL`, env/registry-driven level masks, output to
  file (`GDBG_SET_FILE`) or debugger.
- **Level map** (documented in `H5/CSIM/README`, same numbering used across Glide/CSIM):
  100–110 init/memory/buffers, 105 SST commands, 110 texture download, 120 SET/GET register
  macros, 121–122 PCI, 125–130 registers before/after GO + opcode/status, 126–128 triangle
  parameter/setup/vertex info, 131–132 LFB access/data, 133–145+ per-pixel span internals
  (iterators, FBI rgba/zw, texture modulation…), 280 FIFO room checks, 291 cull decisions,
  80 bump/fence traces.
- Glide-specific debug env vars: `FX_GLIDE_NO_SPLASH`, `FX_SNAPSHOT`, `FX_GLIDE_SCREENSHOT_KEY`,
  plus `GLIDE_SANITY_SIZE` (asserts every packet matches its declared size — **keep on while
  developing transport code**), `FIFO_ASSERT_FULL`.

## 2. FIFO/transport debugging (`H5/GLIDE3/SRC/FIFO.C`, `FXCMD.H`)

- Debug FIFO writes route through `_grFifoWriteDebug`/`_grFifoFWriteDebug` (address/value
  logging with consistent base addresses).
- `_grH3FifoDump_TriHdr` / `_grH3FifoDump_Linear` decode packet headers in traces
  (`DEBUGFIFODUMP_*` macros).
- FIFO check timeouts log slave-chip FIFO state in SLI (`fifo check timeout debug level`).
- `FX_GLIDE_FENCE_LIMIT`, `FX_GLIDE_BUMP`/`BUMPSIZE` let you bisect WC/ordering bugs
  (set fence limit low + manual bump to serialize everything).
- `FX_GLIDE_DIRECT_WRITE=1` (Linux build) bypasses the packet FIFO entirely — register-write
  transport for isolating FIFO vs pipeline bugs.
- D3D equivalents: `FIFODBG.C` (both Win9x D3D and W2K display), `CALLDBG.ASM`+`DBG32.C`
  (MiniVDD), `D3TRACE.C`/`DDTRACE.C` API call tracing.

## 3. State/stat introspection

- Debug Glide keeps counters in `gc->stats` (`trisProcessed`, `trisDrawn`, `texDownloads`,
  `tsuValClamp`, points/lines/buffers swapped) — dumped via `grGet`/debug output; CSIM prints
  statistics after every `SwapBuffers` (level 106).
- `GBANNER.C` prints build/version/config at init (build number stamped by `FXBLDNO`).

## 4. GlideTrap / GlidePlay — API stream capture & replay (`SWLIBS/GlideTrap/`)

The most powerful game-level tool in the tree:
- **Record**: `TRAP/glidetrap.c` builds a `glide2x.dll`-shaped interposer that logs every
  Glide call with arguments to a stream file and saves every texture/LFB upload **once**
  (CRC-32 signature + full-compare hash table, up to 999 collision suffixes —
  `DESIGN.TXT`, `fileformat.txt`).
- **Replay**: `PLAY/glideplay.c` re-executes streams deterministically — perf regression,
  driver A/B comparison, and crash repro without the game installed.
- The `H5/DIAGS/MFTG/BIN/UNREAL/T00000xx.000` files are recorded **Unreal** runs replayed as
  manufacturing burn-in; `GPLAY` in `MFTG/CORE` is the factory player. Use the same mechanism
  to benchmark your Phase-2 driver against real game workloads with zero variance.

## 5. CSIM — the software chip (`H5/CSIM/`)

Functional C model of the VSA-100: command FIFO parser (`CMDFIFO.C`), triangle setup
(`SETUP.C`), rasterizer/FBI (`FBI.C`, span iterators, dither, blend `ALPHA.C`), texture
(`TREX.C`, `LOD.C`, `MIPTABLE.C`, format decoders `RGBFMT.C`/`YUV.C`, compression
`COMPRESS.C`), LFB (`LFB.C`), register file (`REGS.C`), reciprocal/divider models
(`RECIP.C`), I/O + Verilog-PCI trace front-end (`CSIMIO.C`, levels 121/122 → can diff against
RTL sim traces), `H3ASM.C` (test assembler), `GO.C`/`GO2.C` command dispatch.
Build standalone (`BUILD.BAT`/`WinSIM.mak`) or link under Glide (`FX_GLIDE_H5_CSIM=1`) or
DirectDraw (`DDCSIM32.C`). **Golden reference for Phase 2**: render via your new code and via
CSIM, diff framebuffers.

## 6. DIAGS — hardware test suites (`H5/DIAGS/`)

- Per-unit directed tests: `FBI/` (93 tests: alpha blend/clamp, iterators, LFB formats,
  dither, depth, chroma, AGP), `TREX/` (texture formats, mipmap chains, combine units,
  `cfestress/cfevstress` command-FIFO stress, `BIGASS.C` large-texture), `VID/` (CLUT,
  bilinear scaler, 4:1:1/4:2:2), `GUI/` (2D engine), each with `.BAT` runners.
- `MUSTPASS.BAT` — the release gate list. `TESTSUM.AWK` summarizes logs; `BAT2SH.AWK`
  converts runners for unix.
- `BRINGUP/` — new-board sequence (`ALL.BAT`, per-unit shells) + `TOOLS/pciScan`,
  `runDiags`, `statusClient` (networked status reporting for board farms).
- `MFTG/` — manufacturing: Lua-driven test clients (`LUA/` embedded Lua 3.x interpreter!),
  VGA core tests, game-stream replay, `BTOOLS/` board tools.
- `CSIMTEST/` — runs the diag suite against CSIM.
- Prebuilt utilities (`3Dfx/` + `H5/DIAGS/TOOLS`): `detect.exe` (board/ID/memory report),
  `pcirw.exe` (PCI config peek/poke), `pass.exe` (**Voodoo1/2 only — damages nothing but
  misbehaves on V3+; don't run it**), SDK `test00–test38` (each exercises one Glide feature —
  `test00` clear, `test04` AA (4-sample on 2 chips), `test12` LFB, etc. — see
  `3Dfx/Sdk/*/Bin`).

## 7. Perl bring-up kit (`H5/PERL/`)

Direct hardware access from Perl via the XS extension (`extension/Pci`): `REGTEST.PL`
(register r/w patterns), `CMDFIFO.PL` (**reference FIFO bring-up sequence**), `LFBR/LFBW.PL`,
`READPLL.PL`, `AGPINFO.PL`/`AGPWALK.BAT`, `SIP.PL`/`sip_short.pl` (silicon process monitor),
`memdiag80/90.pl`, `IN8/OUT8.PL`, `3DVIDEO.PL`, `ATRI.PL` (send one triangle!),
`fifo_replay.pl`, board-farm rigs (`boardtest*.pl`, `run_bt_master.bat`, `run_slave.bat`).
For a new driver, `ATRI.PL` + `CMDFIFO.PL` encode the minimal happy-path you must reproduce.

## 8. Kernel-side debugging

- Win9x: SoftICE integration (`SIW95` in `SETENV.BAT` — builds `.NMS` symbol files);
  MiniVDD `DEBUG.C`/`DBG32.C` trace ports; `H3IRQ.C` has IRQ-storm guards.
- NT/W2K: standard DDK checked-build + WinDbg/KD; display driver asserts route through
  `EngDebugPrint` (see `DEBUG.C` in miniport).
- Registry debug keys documented in `H5/DOCS/Napalm registry keys.doc`.

## 9. Conformance & regression ladders (run after every change)

1. `H5/DIAGS` `MUSTPASS.BAT` (hardware sanity)
2. `H5/GLIDE3/CONFORM` (Glide behavior, golden-image compare via `BMPCMP`)
3. `SWLIBS/OPENGL/CONFORM` + `OGTST` (GL correctness)
4. GlideTrap replays of real games (performance + pixel diff)
5. `SWLIBS/OPENGL/BENCH` trispeed/bindspeed + Quake timedemos (perf tracking — keep your own
   `QUAKE.XLS` equivalent)
