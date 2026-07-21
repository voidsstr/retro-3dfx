# retro-3dfx

Building the original 3dfx Voodoo 3/4/5 drivers for Windows XP from the
leaked H5/Napalm source tree, on Linux, under Wine — plus the packaging and
deployment tooling for the retro fleet.

## Layout

- **`3dfx Driver Code/`** — the vintage 3dfx H5/Napalm driver source drop,
  archived byte-complete (including checked-in vintage build artifacts).
  `documentation/` inside it holds our engineering notes on the tree
  (build system, toolchain requirements, gaps).
- **`toolchain-3dfx/`** — Wine-hosted VC6 SP5 + MASM 6.15 + W2K DDK build
  environment, packaging script, and the deployable driver package under
  `dist/`. Heavy toolchain pieces are gitignored; `toolchain-3dfx/README.md`
  documents how to reconstruct them and every Wine gotcha.

## Results (2026-07-16, all adversarially verified as fresh builds)

- `GLIDE3X.DLL` — Glide3 runtime, 96 exports, export-list identical to the
  vintage Nov-2000 DLL
- `3dfxvsm.sys` + `3dfxvs.dll` — W2K/XP video miniport + XPDM display driver
  (incl. D3D HAL), clean `/WX` DDK build, valid PE checksums
- `dist/3dfx-napalm-xp-<version>/` — install package: trimmed INFs, `updrv.exe`
  scripted SetupAPI installer, `INSTALL.bat` with backup + signing-policy
  handling

## Driver test suite (run before and after EVERY deploy)

`tests/` holds the regression suite that encodes every hardware-verified fix:

- `tests/predeploy.sh` — pre-deploy gate (source invariants, repo↔build-tree
  sync, built-DLL fix markers, stale-.obj detection). Non-zero exit = do not
  deploy.
- `tests/run_target_tests.py [host]` — on-box D3D matrix (d3dlab modes vs
  goldens), driver/ring health. Run after deploy+reboot, alongside the OpenGL
  golden gate (Q3/CS).

**Policy: when a driver fix is verified on hardware, update the test suite in
the same commit** — add a source assertion to `test_source_invariants.sh` and
a target test (d3dlab mode + golden, or equivalent) so the fix can never
silently regress. Details in `tests/README.md`; the full change policy lives
in `optimized/README.md`.

## Deploying to the fleet

Deployment to fleet XP machines is driven from the sibling **retro-agent**
repo via its `deploy-3dfx-driver` skill, which stages the newest
`toolchain-3dfx/dist/3dfx-napalm-xp-*` package from this repo over the retro
agent protocol (preflight HWID gate, backup, install, verify, rollback).
Run `tests/predeploy.sh` before any deploy and `tests/run_target_tests.py`
after.
