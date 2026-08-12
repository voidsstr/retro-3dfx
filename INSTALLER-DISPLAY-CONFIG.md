# Known-Good Display Driver Config (.124 Voodoo3 XP) — for the future installer

Captured 2026-07-20 from a **fully working** state: **1024×768×32, status OK, no
"not configured properly" error, games' Glide path works**. This is the reference
the installer must reproduce.

## THE key lesson
The **same** display driver files (`3dfxvs.dll` 689,216 + `3dfxvsm.sys` 148,352 +
`glide3x.dll` 335,872) that booted to a **broken** 800×600×4 "primary display
adapter not configured properly" state ALSO run **perfectly at 1024×768×32** — the
only difference is a **proper PnP install via a valid INF** vs. a hand-edited/
corrupted registry config. ⇒ **The installer must install via SetupAPI/PnP
(`UpdateDriverForPlugAndPlayDevices`, i.e. our `updrv.exe <inf> <hwid>`), NEVER by
raw file copy + manual registry edits.** Raw swaps corrupt the class key and
produce the 800×600×4 failure.

## Working registry config (Microsoft in-box driver, 5.1.2001.0 / 3dfxvs2k.inf)
- `HKLM\SYSTEM\CurrentControlSet\Services\3dfxvs\Device0`
  - `InstalledDisplayDrivers` = `REG_MULTI_SZ` **`3dfxvs`** (the display DLL base
    name, no extension — GDI loads `system32\3dfxvs.dll`)
- `HKLM\SYSTEM\CurrentControlSet\Services\3dfxvs`
  - `ImagePath` = `REG_EXPAND_SZ` **`system32\DRIVERS\3dfxvsm.sys`**  (miniport)
  - (service `3dfxvs`, type 1 kernel, start 1 system, group `Video` — from the INF
    `AddService`)
- `HKLM\SYSTEM\CurrentControlSet\Control\Class\{4D36E968-E325-11CE-BFC1-08002BE10318}\0000`
  - `MatchingDeviceId` = `REG_SZ` `pci\ven_121a&dev_0005`
  - `InfPath` = `REG_SZ` `3dfxvs2k.inf`  (or our INF's oemNN.inf alias)
  - **NOTE:** `Service`, `Driver`, and `InstalledDisplayDrivers` are **absent** in
    the class \0000 key on this working config and it works fine — do NOT treat
    their absence as breakage (earlier misdiagnosis). The binding lives in the
    Services\Device0 + service keys above, not the class key.

## Active files in the working state
| file | path | size | note |
|---|---|---|---|
| display DLL | `D:\WINDOWS\system32\3dfxvs.dll` | 689,216 | MS in-box (WFP-maintained) |
| miniport | `D:\WINDOWS\system32\DRIVERS\3dfxvsm.sys` | 148,352 | MS in-box |
| Glide3 | `D:\WINDOWS\system32\glide3x.dll` | 335,872 | MS in-box |

(Active Windows is on **D:**; the box is XP SP3 dual-boot, C: = Win98 FAT.)

## Installing OUR driver (the real goal) — installer requirements
To ship OUR H5 display driver (595,180) instead of the in-box one, the installer
must both (a) PnP-install via our trimmed `voodoo3.inf` (rebuilds config correctly)
AND (b) defeat WFP so our unsigned files aren't reverted to the in-box ones:
- **WFP finding:** dllcache seeding does NOT work (WFP restores in-box from a deeper
  catalog source). The durable options are:
  1. **Catalog-sign** our driver package (produce a `.cat`, test-sign, enable
     testsigning) so WFP accepts our files under the standard `3dfxvs` name — the
     cleanest, keeps the standard binding that Glide/games need.
  2. **Patch `sfc_os.dll`** to truly disable WFP (offline/registry; heavier, risky).
  3. **Rename method** (ship as `3dfxv3d.dll`, repoint `InstalledDisplayDrivers`) —
     WFP-free and works for the D3D/display path, **but breaks the Glide path
     (games hang at `GLW_ChoosePFD`)** so it's unsuitable for a games machine.
- ⇒ For a driver that must serve BOTH games (Glide) and D3D, the installer should
  go the **catalog-sign** route (option 1). The rename method is only acceptable
  for D3D-only validation.

## Recovery recipe (if the display config gets corrupted again)
`updrv.exe D:\WINDOWS\inf\3dfxvs2k.inf "PCI\VEN_121A&DEV_0005"` then reboot →
restores this known-good in-box config at full res. (The in-box driver stays in the
XP driver store as the permanent rollback.)
