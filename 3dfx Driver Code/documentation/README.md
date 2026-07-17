# Documentation Index — 3dfx H5/Napalm Driver Source Tree

This directory contains the complete engineering documentation for the 3dfx `devel` source tree
(October–November 2000 snapshot), produced by a full review of every directory in the archive.

**Project goals this documentation supports:**
1. **Phase 1 — build the drivers as-is** for Voodoo 3 and Voodoo 4/5 hardware.
2. **Phase 2 — build a modernized, optimized driver** that outperforms the community
   Amigamerlin driver packages (which repackaged this same era of 3dfx code with tuning tweaks,
   without restructuring the internals).

## Reading order

| Doc | Contents | Read when |
|---|---|---|
| [01-ARCHITECTURE.md](01-ARCHITECTURE.md) | End-to-end stack architecture, frame lifecycle, the hardware programming model (command FIFO, packet formats, register model), initialization sequence | First — everything else assumes this |
| [02-DIRECTORY-INVENTORY.md](02-DIRECTORY-INVENTORY.md) | Exhaustive inventory of every directory in the tree, with per-module purpose and key files | Reference while navigating |
| [03-GLIDE-RUNTIME.md](03-GLIDE-RUNTIME.md) | Glide 2.x / 3.x runtime internals: file-by-file documentation, core data structures, dispatch system, state shadowing, FIFO transport code, environment variables | Before touching any Glide source |
| [04-HARDWARE-LAYER.md](04-HARDWARE-LAYER.md) | MINIHWC / HWC / HAL / CINIT / BIOS: board discovery, memory layout, chip init, SLI/AA configuration, register map orientation | Before hardware bring-up work |
| [05-OS-DRIVERS.md](05-OS-DRIVERS.md) | Win9x (DD16/DD32/D3D/MiniVDD), NT4, W2K, Mac, DOS, Linux driver structure and entry points; the D3D SIMD T&L pipeline; per-game compatibility system | Before OS-driver builds |
| [06-OPENGL-AND-OTHER-APIS.md](06-OPENGL-AND-OTHER-APIS.md) | OpenGL ICD, MiniGL (3DFXGL), Mac RAVE/MCD, ATB arcade toolkit, splash/tools ecosystem | When working above Glide |
| [07-TOOLCHAIN-AND-BUILD.md](07-TOOLCHAIN-AND-BUILD.md) | **Complete toolchain documentation**: every compiler/DDK/SDK needed per target, environment scripts decoded, DEVTOOLS layout, step-by-step build walkthroughs, CSIM (no-hardware) builds | Phase 1 — building as-is |
| [08-DEBUGGING-AND-DIAGNOSTICS.md](08-DEBUGGING-AND-DIAGNOSTICS.md) | **Debugger & diagnostic tooling**: GDEBUG trace system, debug env vars, GlideTrap record/replay, CSIM debug levels, DIAGS hardware test suites, Perl bring-up tools, prebuilt utilities | Phase 1 & 2 — validation |
| [09-INCOMPLETE-AND-GAPS.md](09-INCOMPLETE-AND-GAPS.md) | **Everything incomplete, absent, or anomalous**: Windows XP status, NT4's missing D3D, absent sibling hardware trees, DX8 stub, archive provenance (Quantum3D), empty files, dead ends | Before planning scope |
| [10-OPTIMIZATION-ROADMAP.md](10-OPTIMIZATION-ROADMAP.md) | **Phase 2 plan**: post-2000 math/CS advances applicable to this code, per-item rationale → files → expected gain → risk → validation, sequenced into phases | Phase 2 — the new driver |

## Scope covered

- Chips: **Avenger (Voodoo3)** and **Napalm / VSA-100 (Voodoo4/5)**, incl. 2-way and 4-way SLI + FSAA.
- OS targets present in tree: DOS, Windows 9x, Windows NT 4.0, Windows 2000, Linux, MacOS 8/9.
- APIs: Glide 2.x, Glide 3.x, Direct3D (DX6/DX7, DX8 stub), OpenGL (ICD + MiniGL), RAVE (Mac),
  DirectDraw, VESA/VGA BIOS.

## Ground rules used in these docs

- Every claim is grounded in the actual source; file paths are given relative to the repo root
  (`H5/…`, `SWLIBS/…`). Line numbers cited were verified against this snapshot.
- Original 3dfx-era docs live in `H5/DOCS/` and `SWLIBS/DOCS/` (chip databook, software
  architecture spec, registry keys, validation plans) — referenced, not duplicated, here.
- The top-level [`README.md`](../README.md) is the executive summary; these docs are the deep dive.
