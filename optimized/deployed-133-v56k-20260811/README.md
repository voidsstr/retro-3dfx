# Deployed-binary snapshot — 192.168.1.133 (P3-DUAL, Voodoo 5 6000) — 2026-08-11

**Why this exists:** the working tree that built these binaries lived on the OMEN 45L
dev box whose 2TB NVMe died in a power outage (recovery in progress as of 2026-08-11).
This repo's committed source ends at 2026-07-18 (ICD 0.2.0 / csim), but the binaries
deployed on .133 are dated **2026-08-07..09** — they embody ~3 weeks of uncommitted
V5-6000 bring-up work. Until the NVMe is recovered, **these files are the only copy**.
Do not overwrite the on-box copies without keeping this snapshot intact.

Pulled off the box via the retro agent (`DOWNLOAD`) on 2026-08-11.

| file | size | on-box date | what |
|---|---|---|---|
| 3dfxv5m.sys | 199,656 | 08/07 14:44 | XP video miniport (our build; system32\drivers) |
| 3dfxv5d.dll | 969,264 | 08/09 00:43 | XPDM display driver + D3D HAL (ACTIVE) |
| 3dfxv5d.dll.bak | 964,236 | 08/07 14:44 | earlier iteration |
| 3dfxv5d.dll.bak2 | 968,388 | 08/07 16:13 | earlier iteration |
| 3dfxv5d.dll.bak3 | 969,208 | 08/08 19:34 | earlier iteration |
| 3dfxogl.dll | 708,608 | 08/08 22:42 | SGL OpenGL ICD (vintage lane, 0.2.x) |
| glide3x.dll | 339,968 | 08/08 23:26 | Glide3 runtime |
| glide2x.dll | 258,048 | 08/07 14:44 | Glide2 runtime |
| oem15.inf | 34,438 | — | active INF for the V5 6000 (PCI\VEN_121A&DEV_0009&SUBSYS_0001121A) |

Box context at snapshot time: XP SP3, dual P3, 1GB; desktop 1024x768x16@85 on the
V5 6000 driving the monitor; display log shows `units=4` (all four VSA-100 chips)
and 9-heap SLI layout; GeForce4 Ti 4600 also present (inactive).
