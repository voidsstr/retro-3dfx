3dfx Voodoo3/4/5 driver package for Windows 2000 / XP
=======================================================
Package: 3dfx-napalm-xp-20260716

CONTENTS
--------
  3dfxvsm.sys   video miniport driver (kernel), free/retail build
  3dfxvs.dll    XP/2K display driver
  glide3x.dll   Glide3 runtime (96 exports)
  fxoem2x.dll   3dfx OEM support DLL (not INF-installed; for tools that want it)
  voodoo3.inf   install INF for Voodoo3 (PCI\VEN_121A&DEV_0005),
                includes SUBSYS_1037121A&REV_01 (fleet box .124)
  voodoo5.inf   install INF for Voodoo4/5 (PCI\VEN_121A&DEV_0009 / DEV_000B)
  updrv.exe     tiny helper that calls UpdateDriverForPlugAndPlayDevicesA
                (SetupAPI/PnP install; needs Win2000 or later)
  INSTALL.bat   scripted Voodoo3 install (backup + policy + install)

Both INFs are trimmed to the files actually shipped: glide2x.dll,
3dfxSpl2.dll, 3dfxSpl3.dll and 3dfxOGL.dll are NOT included, and the
OpenGL ICD registration was removed (registering a missing 3dfxOGL.dll
would break every OpenGL app). OpenGL apps should use a Glide wrapper or
MesaFX on top of glide3x.dll.

TARGETS
-------
  voodoo3.inf : 3dfx Voodoo3 2000/3000/3500 (PCI\VEN_121A&DEV_0005),
                verified target: box .124, HWID
                PCI\VEN_121A&DEV_0005&SUBSYS_1037121A&REV_01, Windows XP SP3
  voodoo5.inf : Voodoo4 4500 / Voodoo5 5500 (DEV_0009) and Napalm2 (DEV_000B)
  OS          : Windows 2000 / XP (updrv.exe loads newdev.dll at runtime;
                the binary itself starts on Win98 but the API needs Win2K+)

INSTALL (scripted, via retro agent)
-----------------------------------
  1. Copy this whole directory to the machine, e.g. C:\RETRO_AGENT\3dfx-drv\
     (agent UPLOAD, or copy from the SMB share).
  2. Run INSTALL.bat from that directory (agent: EXEC or EXECW - it is a
     console batch, output is captured). It will:
       - back up existing 3dfxvs*.* + glide3x.dll to C:\RETRO_AGENT\3dfx-backup\
         (first run only - reruns keep the original backup)
       - set the XP driver-signing policy to Ignore (this build is unsigned)
       - copy glide3x.dll to system32
       - install via: updrv.exe voodoo3.inf "PCI\VEN_121A&DEV_0005"
  3. Reboot (fleet machines: only with explicit user approval), then verify
     with VIDEODIAG / DISPLAYCFG.
  For a Voodoo4/5 box run updrv.exe manually with voodoo5.inf and the
  matching HWID (e.g. "PCI\VEN_121A&DEV_0009").

INSTALL (manual, Device Manager fallback)
-----------------------------------------
  1. Copy the directory to the machine.
  2. Device Manager -> Display adapters -> the Voodoo device -> Update
     Driver -> "Install from a list or specific location" -> "Don't search,
     I will choose" -> Have Disk -> browse to voodoo3.inf (or voodoo5.inf).
  3. Accept the unsigned-driver warning, let files copy, reboot.

ROLLBACK
--------
  - Preferred: Device Manager -> display adapter -> Properties -> Driver ->
    "Roll Back Driver" (XP keeps the previous driver set).
  - Boot problem: F8 -> "Last Known Good Configuration" undoes the new
    service/driver registration.
  - Manual: restore the files saved in C:\RETRO_AGENT\3dfx-backup\ over
    system32 / system32\drivers (from Safe Mode if needed), then reboot.

PROVENANCE
----------
  Built 20260716 from the 3dfx Interactive H5 (Napalm) source tree,
  W2K free (retail) build, cross-compiled in the repo's toolchain-3dfx/
  Wine+DDK environment. UNSIGNED - no WHQL catalog; XP shows the unsigned
  driver prompt unless the signing policy is set to Ignore (INSTALL.bat
  does this). updrv.exe is our own helper (agent/tools/updrv.c), built
  freestanding with mingw so it runs on any Win98-XP box.
