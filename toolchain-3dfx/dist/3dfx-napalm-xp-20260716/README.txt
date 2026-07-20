3dfx Voodoo3 / Voodoo4 / Voodoo5 (incl. Voodoo5 6000) driver package
for Windows 2000 / XP
=====================================================================
Package: 3dfx-napalm-xp-20260716   (rebuilt 2026-07-19)

Complete self-built driver stack for the leaked H5/Napalm source, produced
with the Wine + VC6/W2K-DDK toolchain. Covers Voodoo3 (DEV_0005), Voodoo4/5
5500 (DEV_0009 / DEV_000B), and the Voodoo5 6000 (quad VSA-100, DEV_0009).

CONTENTS
--------
  Kernel / display (W2K miniport + XP display driver):
    3dfxv5m.sys   miniport, WFP-safe name — Voodoo4/5 + *6000*. Includes the
                  6000 quad-VSA-100 bring-up: HiNT HB1-SE66 bridge finder,
                  external graphics-clock synthesis over bridge GPIO
                  (V56KFindHintBridge / V56KSetExternalClock / V56KOutputClock),
                  and QuadChipAASLI config. Also serves the 5500 (dwChips==2)
                  and Voodoo3 (IS_VOODOO3) from the same binary via runtime
                  branches — the 6000 paths are dwChips==4 / HiNT-gated, so no
                  regression on 5500 / V3.
    3dfxv5d.dll   XP/2K display driver (WFP-safe name), incl. the D3D HAL.
    3dfxvsm.sys   legacy stock-name miniport (for voodoo3.inf / voodoo5.inf).
    3dfxvs.dll    legacy stock-name display driver.

  Runtimes:
    glide3x.dll   Glide3 runtime (96 exports) — Napalm, minihwc-linked (incl.
                  the 6000 same-bus chip-detection fallback).
    glide2x.dll   Glide2 runtime (133 exports) — most Glide GAMES need this
                  (Unreal/UT'99, NFS, Diablo II, …), not glide3x.
    3dfxogl.dll   OpenGL ICD [retro3dfx 0.3.2] (0.3.1: 16-byte Napalm texture-heap alignment, solves 2D sliced-text garble; 0.3.2: EXT_paletted_texture RGBA table stride fix, solves GoldSrc green-world colors). Honors GL texture filters in
                  hardware; fixes the UT glTexSubImage2D NULL-cache GPF. Stable
                  across Quake III, Quake II, CS 1.6, Unreal Tournament on the
                  Voodoo5. Registered by voodoo5-6k.inf / voodoo5-wfp.inf.
    fxoem2x.dll   3dfx OEM support DLL (not INF-installed; for tools).

  INFs:
    voodoo5-6k.inf   Voodoo5 6000 (quad VSA-100). Ships the WFP-safe driver
                     pair + all three runtimes + registers the OpenGL ICD.
                     MaximumNumberOfDevices=4. Generic PCI\VEN_121A&DEV_0009
                     row (the 6000 shares DEV_0009; chip count is detected at
                     runtime). Fill in the exact Strange God SUBSYS after the
                     card's PnP tree is captured (Phase 1).
    voodoo5-wfp.inf  Voodoo4/5 5500 (WFP-safe 3dfxv5d/3dfxv5m pair) — the
                     verified .143 fleet path.
    voodoo5.inf      legacy stock-name Voodoo4/5 (3dfxvs/3dfxvsm).
    voodoo3.inf      Voodoo3 (DEV_0005), incl. SUBSYS_1037121A&REV_01 (.124).

  Installer helpers:
    updrv.exe     UpdateDriverForPlugAndPlayDevicesA (SetupAPI/PnP; W2K+).
    INSTALL.bat   scripted install (backup + signing policy + updrv).

INSTALL (Voodoo5 6000, XP/2000)
-------------------------------
  1. Capture + archive the vendor's known-good stack and BOTH VBIOS images
     first (Phase 1) — that is the rollback. Never reflash the card BIOS.
  2. Start in the card's 128 MB BIOS mode (32 MB/chip) — the hardened path.
  3. Back up the display-class registry + existing driver files.
  4. Suppress the unsigned-driver prompt (our binaries are unsigned):
       reg add "HKLM\SOFTWARE\Microsoft\Driver Signing" /v Policy /t REG_BINARY /d 00 /f
  5. updrv.exe voodoo5-6k.inf "PCI\VEN_121A&DEV_0009"
  6. Reboot; verify VIDEODIAG shows our driver and a sane resolution (not
     640x480 4-bit VGA — that means the VxD/miniport fell back).

STATE / SUPPORT MATRIX
----------------------
  Voodoo3, Voodoo5 5500          : verified on the fleet (.124 / .143).
  Voodoo5 6000, 128 MB mode      : code-complete (external clock + quad SLI +
                                   detection); awaiting the physical card to
                                   bring up (single-chip -> 2-way -> 4-way).
  Voodoo5 6000, 256 MB mode      : Phase 3 — needs one hwcMapBoard change
                                   (MINIHWC.C:1681, 32 MB -> 64 MB/chip BAR
                                   length) that must be verified on hardware;
                                   NOT applied here to keep the 128 MB path
                                   hardened. See V56K-PLAN.md.

  Full plan + phased bring-up: ../../V56K-PLAN.md
  OpenGL ICD changelog:        ../../optimized/CHANGELOG.md

NOTES
-----
  - Unsigned, sign-free-era install (driver-signing prompt only on W2K/XP).
  - WFP: never raw-copy 3dfxv5d.dll/3dfxv5m.sys into system32 — install via
    the INF/updrv (SetupAPI) so WFP does not silently revert them. The
    WFP-safe names avoid the in-box 3dfxvs.dll/3dfxvsm.sys protection.
  - INFs are trimmed to the files actually shipped (no 3dfxSpl2/3.dll).
