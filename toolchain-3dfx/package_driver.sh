#!/usr/bin/env bash
# package_driver.sh — assemble the 3dfx Napalm/H5 W2K/XP driver package.
#
# Usage: ./package_driver.sh [version]     (version defaults to YYYYMMDD)
#
# Consumes the already-built binaries (does NOT rebuild anything):
#   - 3dfxvsm.sys  (miniport,  objfre)
#   - 3dfxvs.dll   (display driver, objfre)
#   - glide3x.dll  (Glide3 runtime)
#   - fxoem2x.dll  (OEM support DLL)
#   - updrv.exe    (UpdateDriverForPlugAndPlayDevicesA helper, agent/tools)
#
# Generates trimmed INFs (voodoo3.inf / voodoo5.inf) from the build-tree
# INFs: only files we actually ship stay in CopyFiles/SourceDisksFiles, the
# OpenGL ICD registration is dropped (we do not ship 3dfxOGL.dll — a
# registered-but-missing ICD breaks OpenGL apps), and the .124 fleet box HWID
# (PCI\VEN_121A&DEV_0005&SUBSYS_1037121A&REV_01) is added to voodoo3.inf.
# CRLF line endings are preserved byte-for-byte outside the edited lines.
#
# Output: toolchain-3dfx/dist/3dfx-napalm-xp-<version>/  +  matching .zip
# Idempotent: reruns wipe and rebuild the package dir + zip.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

VERSION="${1:-$(date +%Y%m%d)}"
DIST_DIR="$SCRIPT_DIR/dist"
PKG_NAME="3dfx-napalm-xp-$VERSION"
PKG_DIR="$DIST_DIR/$PKG_NAME"
ZIP_PATH="$DIST_DIR/$PKG_NAME.zip"

H5="$SCRIPT_DIR/prefix/drive_c/3dfx/H5"
MINIPORT="$H5/W2K/Src/Video/Miniport/H5/objfre/i386/3dfxvsm.sys"
DISPLAY_DLL="$H5/W2K/Src/Video/Displays/H5/objfre/i386/3dfxvs.dll"
GLIDE3="$H5/BIN/glide3x.dll"
FXOEM="$H5/BIN/fxoem2x.dll"
UPDRV="$REPO_ROOT/agent/tools/updrv.exe"
INF_V3="$H5/W2K/Src/Video/Inf/Voodoo3/Voodoo3.inf"
INF_V5="$H5/W2K/Src/Video/Inf/Voodoo5/3DFXVS2K.INF"

# ---------------------------------------------------------------- inputs
fail() { echo "ERROR: $*" >&2; exit 1; }
for f in "$MINIPORT" "$DISPLAY_DLL" "$GLIDE3" "$FXOEM" "$UPDRV" "$INF_V3" "$INF_V5"; do
    [ -f "$f" ] || fail "required input missing: $f"
done

echo "== Packaging $PKG_NAME =="
rm -rf "$PKG_DIR"
rm -f "$ZIP_PATH"
mkdir -p "$PKG_DIR"

cp "$MINIPORT"    "$PKG_DIR/3dfxvsm.sys"
cp "$DISPLAY_DLL" "$PKG_DIR/3dfxvs.dll"
cp "$GLIDE3"      "$PKG_DIR/glide3x.dll"
cp "$FXOEM"       "$PKG_DIR/fxoem2x.dll"
cp "$UPDRV"       "$PKG_DIR/updrv.exe"

# ------------------------------------------------------- INF generation
# Byte-level edits in python3 so CRLF endings survive untouched.
python3 - "$INF_V3" "$INF_V5" "$PKG_DIR" <<'PYEOF'
import sys

inf_v3, inf_v5, pkg = sys.argv[1], sys.argv[2], sys.argv[3]
CRLF = b'\r\n'
REMOVED = (b'glide2x.dll', b'3dfxSpl2.dll', b'3dfxSpl3.dll', b'3dfxOGL.dll')

def read_lines(path):
    data = open(path, 'rb').read()
    # split on CRLF only, so any stray bare LF stays embedded byte-identical
    return data.split(CRLF)

def strip_file_lines(lines, section, names):
    """Remove lines that are exactly one of `names` inside [section]."""
    out, in_sec, removed = [], False, 0
    for ln in lines:
        s = ln.strip()
        if s.startswith(b'['):
            in_sec = s.lower() == b'[' + section.lower() + b']'
        elif in_sec and s.split(b';')[0].strip().split(b'=')[0].strip() in names:
            removed += 1
            continue
        out.append(ln)
    if removed != len(names):
        raise SystemExit('expected to remove %d lines from [%s], removed %d'
                         % (len(names), section.decode(), removed))
    return out

def drop_line(lines, exact_stripped):
    out = [ln for ln in lines if ln.strip() != exact_stripped]
    if len(out) != len(lines) - 1:
        raise SystemExit('expected exactly one line %r, found %d'
                         % (exact_stripped, len(lines) - len(out)))
    return out

def drop_section(lines, section):
    """Remove [section] header + body (up to next section header)."""
    out, skipping, dropped = [], False, False
    for ln in lines:
        s = ln.strip()
        if s.startswith(b'['):
            skipping = s.lower() == b'[' + section.lower() + b']'
            if skipping:
                dropped = True
                continue
        if skipping:
            continue
        out.append(ln)
    if not dropped:
        raise SystemExit('section [%s] not found' % section.decode())
    return out

# ---------------- voodoo3.inf ----------------
v3 = read_lines(inf_v3)

# 1. add .124's HWID to the [3dfx.Mfg] continuation list
tail = b'  PCI\\VEN_121A&DEV_0005&SUBSYS_0036121A&REV_01'
idx = [i for i, ln in enumerate(v3) if ln == tail]
if len(idx) != 1:
    raise SystemExit('Voodoo3.inf model-list tail line not found exactly once')
i = idx[0]
v3[i] = tail + b', \\'
v3.insert(i + 1, b'  PCI\\VEN_121A&DEV_0005&SUBSYS_1037121A&REV_01')

# 2. ship-list trim: CopyFiles + SourceDisksFiles
v3 = strip_file_lines(v3, b'3dfxvs.Display', REMOVED)
v3 = strip_file_lines(v3, b'SourceDisksFiles',
                      (b'glide2x.dll', b'3dfxSpl2.dll', b'3dfxSpl3.dll', b'3dfxOGL.dll'))

# 3. no 3dfxOGL.dll shipped -> do not register the OpenGL ICD
v3 = drop_line(v3, b'AddReg=OpenGL.Regs')
v3 = drop_section(v3, b'OpenGL.Regs')

open(pkg + '/voodoo3.inf', 'wb').write(CRLF.join(v3))

# ---------------- voodoo5.inf ----------------
v5 = read_lines(inf_v5)
v5 = strip_file_lines(v5, b'3dfxvs.Display', REMOVED)
v5 = strip_file_lines(v5, b'SourceDisksFiles',
                      (b'glide2x.dll', b'3dfxOGL.dll', b'3dfxSpl2.dll', b'3dfxSpl3.dll'))
v5 = drop_line(v5, b'AddReg=OpenGL.AddRegs')
v5 = drop_section(v5, b'OpenGL.AddRegs')
open(pkg + '/voodoo5.inf', 'wb').write(CRLF.join(v5))

print('generated voodoo3.inf and voodoo5.inf')
PYEOF

# --------------------------------------------------------- INSTALL.bat
python3 - "$PKG_DIR" <<'PYEOF'
import sys
pkg = sys.argv[1]
lines = [
 r'@echo off',
 r'rem ------------------------------------------------------------------',
 r'rem  3dfx Voodoo3 driver install (self-built H5/Napalm, Windows XP/2K)',
 r'rem  Safe to run twice: backup is only taken once, all copies use /Y.',
 r'rem ------------------------------------------------------------------',
 r'setlocal',
 r'set PKGDIR=%~dp0',
 r'set BACKUP=C:\RETRO_AGENT\3dfx-backup',
 r'',
 r'echo [1/4] Backing up current 3dfx driver files to %BACKUP% ...',
 r'if not exist "%BACKUP%" mkdir "%BACKUP%"',
 r'if exist "%BACKUP%\backup-done.txt" goto skipbackup',
 r'if exist "%SystemRoot%\system32\drivers\3dfxvsm.sys" copy /Y "%SystemRoot%\system32\drivers\3dfxvsm.sys" "%BACKUP%\" >nul',
 r'if exist "%SystemRoot%\system32\3dfxvs*.*" copy /Y "%SystemRoot%\system32\3dfxvs*.*" "%BACKUP%\" >nul',
 r'if exist "%SystemRoot%\system32\glide3x.dll" copy /Y "%SystemRoot%\system32\glide3x.dll" "%BACKUP%\" >nul',
 r'echo backup taken by INSTALL.bat> "%BACKUP%\backup-done.txt"',
 r'goto backupdone',
 r':skipbackup',
 r'echo        (backup already exists, keeping the original copies)',
 r':backupdone',
 r'',
 r'echo [2/4] Setting driver-signing policy to Ignore ...',
 r'reg add "HKLM\Software\Microsoft\Driver Signing" /v Policy /t REG_BINARY /d 00 /f >nul',
 r'',
 r'echo [3/4] Copying glide3x.dll to system32 ...',
 r'copy /Y "%PKGDIR%glide3x.dll" "%SystemRoot%\system32\glide3x.dll"',
 r'',
 r'echo [4/4] Installing driver (updrv + voodoo3.inf) ...',
 r'"%PKGDIR%updrv.exe" "%PKGDIR%voodoo3.inf" "PCI\VEN_121A&DEV_0005"',
 r'set RC=%ERRORLEVEL%',
 r'echo.',
 r'if "%RC%"=="0" goto ok',
 r'if "%RC%"=="2" goto okreboot',
 r'echo RESULT: FAILED (updrv exit code %RC%). Driver was NOT installed.',
 r'echo Backup of the previous driver is in %BACKUP%.',
 r'endlocal',
 r'exit /b 1',
 r':ok',
 r'echo RESULT: OK - driver installed.',
 r'goto done',
 r':okreboot',
 r'echo RESULT: OK - driver installed, Windows says a REBOOT IS REQUIRED.',
 r':done',
 r'echo.',
 r'echo REMINDER: reboot the machine to activate the new display driver.',
 r'echo (Do NOT reboot a fleet box without explicit user approval.)',
 r'endlocal',
 r'exit /b 0',
]
open(pkg + '/INSTALL.bat', 'wb').write(('\r\n'.join(lines) + '\r\n').encode('ascii'))
print('generated INSTALL.bat')
PYEOF

# ----------------------------------------------------------- README.txt
python3 - "$PKG_DIR" "$VERSION" <<'PYEOF'
import sys
pkg, version = sys.argv[1], sys.argv[2]
text = """3dfx Voodoo3/4/5 driver package for Windows 2000 / XP
=======================================================
Package: 3dfx-napalm-xp-%s

CONTENTS
--------
  3dfxvsm.sys   video miniport driver (kernel), free/retail build
  3dfxvs.dll    XP/2K display driver
  glide3x.dll   Glide3 runtime (96 exports)
  fxoem2x.dll   3dfx OEM support DLL (not INF-installed; for tools that want it)
  voodoo3.inf   install INF for Voodoo3 (PCI\\VEN_121A&DEV_0005),
                includes SUBSYS_1037121A&REV_01 (fleet box .124)
  voodoo5.inf   install INF for Voodoo4/5 (PCI\\VEN_121A&DEV_0009 / DEV_000B)
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
  voodoo3.inf : 3dfx Voodoo3 2000/3000/3500 (PCI\\VEN_121A&DEV_0005),
                verified target: box .124, HWID
                PCI\\VEN_121A&DEV_0005&SUBSYS_1037121A&REV_01, Windows XP SP3
  voodoo5.inf : Voodoo4 4500 / Voodoo5 5500 (DEV_0009) and Napalm2 (DEV_000B)
  OS          : Windows 2000 / XP (updrv.exe loads newdev.dll at runtime;
                the binary itself starts on Win98 but the API needs Win2K+)

INSTALL (scripted, via retro agent)
-----------------------------------
  1. Copy this whole directory to the machine, e.g. C:\\RETRO_AGENT\\3dfx-drv\\
     (agent UPLOAD, or copy from the SMB share).
  2. Run INSTALL.bat from that directory (agent: EXEC or EXECW - it is a
     console batch, output is captured). It will:
       - back up existing 3dfxvs*.* + glide3x.dll to C:\\RETRO_AGENT\\3dfx-backup\\
         (first run only - reruns keep the original backup)
       - set the XP driver-signing policy to Ignore (this build is unsigned)
       - copy glide3x.dll to system32
       - install via: updrv.exe voodoo3.inf "PCI\\VEN_121A&DEV_0005"
  3. Reboot (fleet machines: only with explicit user approval), then verify
     with VIDEODIAG / DISPLAYCFG.
  For a Voodoo4/5 box run updrv.exe manually with voodoo5.inf and the
  matching HWID (e.g. "PCI\\VEN_121A&DEV_0009").

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
  - Manual: restore the files saved in C:\\RETRO_AGENT\\3dfx-backup\\ over
    system32 / system32\\drivers (from Safe Mode if needed), then reboot.

PROVENANCE
----------
  Built %s from the 3dfx Interactive H5 (Napalm) source tree,
  W2K free (retail) build, cross-compiled in the repo's toolchain-3dfx/
  Wine+DDK environment. UNSIGNED - no WHQL catalog; XP shows the unsigned
  driver prompt unless the signing policy is set to Ignore (INSTALL.bat
  does this). updrv.exe is our own helper (agent/tools/updrv.c), built
  freestanding with mingw so it runs on any Win98-XP box.
""" % (version, version)
open(pkg + '/README.txt', 'wb').write(text.replace('\n', '\r\n').encode('ascii'))
print('generated README.txt')
PYEOF

# ------------------------------------------------------------ validation
python3 - "$PKG_DIR" <<'PYEOF'
import os, sys
pkg = sys.argv[1]
shipped = set(os.listdir(pkg))
removed = ('glide2x.dll', '3dfxspl2.dll', '3dfxspl3.dll', '3dfxogl.dll')
problems = []

def parse(path):
    sections, cur = {}, None
    for raw in open(path, 'rb').read().decode('ascii').split('\r\n'):
        line = raw.split(';')[0].strip()  # inline comments off
        if not line:
            continue
        if line.startswith('['):
            cur = line.strip('[]').lower()
            sections.setdefault(cur, [])
        elif cur is not None:
            sections[cur].append(line)
    return sections

for inf in ('voodoo3.inf', 'voodoo5.inf'):
    path = os.path.join(pkg, inf)
    sec = parse(path)

    # every CopyFiles-referenced file must exist in the package
    copy_secs = set()
    for lines in sec.values():
        for ln in lines:
            k, _, v = ln.partition('=')
            if k.strip().lower() == 'copyfiles':
                copy_secs.update(s.strip().lower() for s in v.split(','))
    if not copy_secs:
        problems.append('%s: no CopyFiles directives found' % inf)
    for cs in sorted(copy_secs):
        for ln in sec.get(cs, ['<missing section %s>' % cs]):
            fname = ln.split(',')[0].strip()
            if fname not in shipped:
                problems.append('%s: [%s] wants %s (not in package)' % (inf, cs, fname))

    # no active reference to any removed file anywhere
    data = open(path, 'rb').read().decode('ascii')
    for n, raw in enumerate(data.split('\r\n'), 1):
        active = raw.split(';')[0].lower()
        for r in removed:
            if r in active:
                problems.append('%s:%d active reference to %s: %r' % (inf, n, r, raw.strip()))

    # CRLF integrity: no bare LF outside the one inherited from the source
    crlf = data.count('\r\n'); bare = data.count('\n') - crlf
    limit = 1 if inf == 'voodoo3.inf' else 0   # source Voodoo3.inf has 1 legacy bare LF
    if bare > limit:
        problems.append('%s: %d unexpected bare LFs' % (inf, bare - limit))

# target HWID present in voodoo3.inf model list
v3 = open(os.path.join(pkg, 'voodoo3.inf'), 'rb').read().decode('ascii')
if 'PCI\\VEN_121A&DEV_0005&SUBSYS_1037121A&REV_01' not in v3:
    problems.append('voodoo3.inf: SUBSYS_1037121A HWID missing')

if problems:
    print('VALIDATION FAILED:')
    for p in problems: print('  -', p)
    sys.exit(1)
print('validation OK: CopyFiles targets all shipped, HWID present, no stale refs, CRLF intact')
PYEOF

# ------------------------------------------------------------------ zip
( cd "$DIST_DIR" && zip -qr "$PKG_NAME.zip" "$PKG_NAME" )

echo
echo "Package : $PKG_DIR"
echo "Zip     : $ZIP_PATH"
ls -la "$PKG_DIR"
