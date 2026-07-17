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

## Deploying to the fleet

Deployment to fleet XP machines is driven from the sibling **retro-agent**
repo via its `deploy-3dfx-driver` skill, which stages the newest
`toolchain-3dfx/dist/3dfx-napalm-xp-*` package from this repo over the retro
agent protocol (preflight HWID gate, backup, install, verify, rollback).
