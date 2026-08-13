#!/usr/bin/env bash
# Reconstruct the Wine/VC6/MASM/DDK build toolchain from scratch on a Linux host,
# with NO sudo. Everything lands under $RETRO3DFX_TC (default ~/retro3dfx-toolchain)
# on a roomy Linux volume — never on /mnt/c (slow drvfs, often full).
#
# Fast path: if the fleet share holds the backup (see toolchain-3dfx/README.md ->
# "Toolchain backup on the share"), fetch downloads/ from there instead of the
# internet, then run this with RETRO3DFX_FROM_SHARE=1 to skip the network pulls.
#
# Sources (abandonware, archived; used for this retro-computing project):
#   Wine      github.com/Kron4ek/Wine-Builds  11.13 amd64 wow64
#   VC6 SP5   archive.org/details/visual-studio-6-0-sp5-portable
#   MASM 6.15 (ml.exe ships inside VC6 SP5)
#   W2K DDK   archive.org/details/msdn-disc7-february-2000-x05-48786 (1_WIN2KDDK.iso)
#   DX7 DDK   archive.org/details/dx7ddk         (MSI; headers only, optional)
#   XP DDK    archive.org/details/microsoft-windows-xp-service-pack-1-driver-development-kit-ddk-english
#             ^ ONLY source of the DirectX 8 DRIVER headers (ddrawint.h/ddrawi.h/
#               d3dhal.h/d3dhalex.h). The W2K DDK has none of DDHALINFO_GETDRIVERINFO2 /
#               DD_GETDRIVERINFO2DATA / GUID_GetDriverInfo2 / D3DGDI_GET_GDI2_DATA, so
#               without this the display driver can only build DX=7 and every D3D8 title
#               falls back to software. Headers only -- build.exe still comes from w2kddk.
#   Win98 DDK github.com/fapablazacl/win98-ddk-toolchain (minivdd.h/vmm.h/configmg.h)
#   7-Zip     7-zip.org standalone 7zz (handles BCJ2 / CAB / ISO)
set -eu
TC="${RETRO3DFX_TC:-$HOME/retro3dfx-toolchain}"
REPO="$(cd "$(dirname "$0")/../.." && pwd)"       # retro-3dfx checkout root
SRC="$REPO/3dfx Driver Code"
mkdir -p "$TC"/{wine,downloads,devtools,prefix,extract,bin}
cd "$TC/downloads"
Z="$TC/bin/7zz"

fetch() { [ -s "$2" ] || curl -fsSL --retry 3 --max-time 600 -o "$2" "$1"; }

echo "== 7zz =="
[ -x "$Z" ] || { curl -fsSL --max-time 120 -o 7z.tar.xz https://www.7-zip.org/a/7z2301-linux-x64.tar.xz && tar -xf 7z.tar.xz -C "$TC/bin" 7zz; }

echo "== Wine 11.13 =="
fetch "https://github.com/Kron4ek/Wine-Builds/releases/download/11.13/wine-11.13-amd64-wow64.tar.xz" wine-11.13-amd64-wow64.tar.xz
[ -x "$TC/wine/bin/wine" ] || tar -xf wine-11.13-amd64-wow64.tar.xz -C "$TC/wine" --strip-components=1

export WINEPREFIX="$TC/prefix" PATH="$TC/wine/bin:$PATH" WINEDEBUG=-all DISPLAY=
ulimit -f 2000000
[ -d "$TC/prefix/drive_c" ] || { timeout 180 wine wineboot --init >/dev/null 2>&1 || true; pkill -9 -x wineserver 2>/dev/null || true; }

echo "== downloads =="
fetch "https://archive.org/download/visual-studio-6-0-sp5-portable/visual-studio-6-0-sp5-portable.7z" vc6-sp5-portable.7z
fetch "https://archive.org/download/msdn-disc7-february-2000-x05-48786/1_WIN2KDDK.iso" 1_WIN2KDDK.iso
fetch "https://archive.org/download/dx7ddk/dx7ddk.exe" dx7ddk.exe || true
fetch "https://archive.org/download/microsoft-windows-xp-service-pack-1-driver-development-kit-ddk-english/en_winxp_sp1_ddk.exe" en_winxp_sp1_ddk.exe || true
[ -d "$TC/extract/win98-ddk-toolchain" ] || git clone --depth 1 https://github.com/fapablazacl/win98-ddk-toolchain "$TC/extract/win98-ddk-toolchain"

echo "== VC6 (BCJ2 -> needs 7zz, not py7zr) -> devtools/msvc6_0 =="
[ -x "$TC/devtools/msvc6_0/vc98/Bin/CL.EXE" ] || {
  rm -rf "$TC/extract/vc6"; "$Z" x -y -o"$TC/extract/vc6" vc6-sp5-portable.7z >/dev/null
  mkdir -p "$TC/devtools/msvc6_0"
  mv "$TC/extract/vc6/vc_studio/main/VC98"   "$TC/devtools/msvc6_0/vc98"
  mv "$TC/extract/vc6/vc_studio/main/Common" "$TC/devtools/msvc6_0/Common"
}
echo "== MASM 6.15 (ml.exe from VC6 SP5) -> devtools/masm614 =="
mkdir -p "$TC/devtools/masm614/bin"
cp -f "$TC/devtools/msvc6_0/vc98/Bin/ml.exe" "$TC/devtools/masm614/bin/"
cp -f "$TC/devtools/msvc6_0/vc98/Bin/ml.err" "$TC/devtools/masm614/bin/" 2>/dev/null || true

echo "== W2K DDK (ISO -> CABs -> extract_ddk.py) -> devtools/w2kddk =="
[ -x "$TC/devtools/w2kddk/bin/build.exe" ] || {
  "$Z" x -y -o"$TC/extract/iso-w2k" 1_WIN2KDDK.iso "CABS/I386/*" >/dev/null
  mkdir -p "$TC/devtools/w2kddk"
  python3 "$REPO/toolchain-3dfx/extract_ddk.py" "$TC/extract/iso-w2k/CABS/I386" "$TC/devtools/w2kddk" "$Z"
}
echo "== XP SP1 DDK (DirectX 8 driver headers) -> devtools/xpddk =="
# WinZip SFX: the payload is an appended ZIP, so -tzip. We only pull the CAB/INF
# pairs that carry headers -- extract_ddk.py skips any INF whose CAB is absent.
[ -f "$TC/devtools/xpddk/inc/wxp/ddrawint.h" ] || {
  if [ -s "$TC/downloads/en_winxp_sp1_ddk.exe" ]; then
    "$Z" x -tzip -y -o"$TC/extract/xpddk" en_winxp_sp1_ddk.exe "COMMON/*.INF" \
        "COMMON/SDKINCS1.CAB" "COMMON/SDKINCS2.CAB" "COMMON/SDKINCS3.CAB" \
        "COMMON/SDKINC2K1.CAB" "COMMON/SDKINC2K2.CAB" "COMMON/SDKINC2K3.CAB" \
        "COMMON/DDKINCS.CAB" "COMMON/DDKINC2K.CAB" "COMMON/DXDDK.CAB" \
        "COMMON/W2K_INCS.CAB" >/dev/null
    mkdir -p "$TC/devtools/xpddk"
    python3 "$REPO/toolchain-3dfx/extract_ddk.py" "$TC/extract/xpddk/COMMON" "$TC/devtools/xpddk" "$Z"
  else
    echo "   !! en_winxp_sp1_ddk.exe missing -- DX=8 builds will not be possible"
  fi
}

echo "== Win98 DDK headers -> devtools/w9xddk =="
[ -f "$TC/devtools/w9xddk/inc/win98/MINIVDD.H" ] || cp -a "$TC/extract/win98-ddk-toolchain/98DDK" "$TC/devtools/w9xddk"

echo "== map C:\\3dfxtools -> devtools =="
ln -sfn "$TC/devtools" "$TC/prefix/drive_c/3dfxtools"

echo "== build copy: C:\\3dfx = space-free source + tracked fixes + 2 build edits =="
if [ ! -d "$TC/prefix/drive_c/3dfx/H5" ]; then
  cp -a "$SRC/." "$TC/prefix/drive_c/3dfx/"
fi
# overlay the repo's tracked source-fix deltas onto the build copy
( cd "$REPO" && git ls-files toolchain-3dfx/prefix/drive_c/3dfx ) | while read -r rel; do
  sub="${rel#toolchain-3dfx/prefix/drive_c/3dfx/}"
  mkdir -p "$TC/prefix/drive_c/3dfx/$(dirname "$sub")"
  cp -f "$REPO/$rel" "$TC/prefix/drive_c/3dfx/$sub"; touch "$TC/prefix/drive_c/3dfx/$sub"
done
# edit 1: GLIDE3 MAKEFILE SUBDIRS drops the absent 'tests' dir
sed -i 's/^SUBDIRS = oem src tests/SUBDIRS = oem src/' "$TC/prefix/drive_c/3dfx/H5/GLIDE3/MAKEFILE"
# edit 2: bldw2k.bat wrapper (from the tracked helper), CRLF
sed 's/$/\r/' "$REPO/toolchain-3dfx/prefix/drive_c/3dfx/bldw2k_inc.bat" > "$TC/prefix/drive_c/3dfx/bldw2k.bat"

echo
echo "Toolchain ready under $TC"
echo "Verify:  source $REPO/toolchain-3dfx/build/env.sh && bash $REPO/toolchain-3dfx/build/build-glide3x.sh"
