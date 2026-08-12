#!/bin/bash
# Built-artifact regression checks for 3dfxvs.dll (the display driver binary
# that gets deployed as 3dfxv5d.dll). Catches the classic failure mode of this
# vintage build system: STALE OBJECT FILES — the source has the fix but the
# linked DLL does not.
# Usage: test_built_artifact.sh [path-to-3dfxvs.dll]

cd "$(dirname "$0")/.." || exit 2
BUILDROOT="${RETRO3DFX_TC:+$RETRO3DFX_TC/prefix/drive_c/3dfx}"
[ -n "$BUILDROOT" ] && [ -d "$BUILDROOT" ] || BUILDROOT="toolchain-3dfx/prefix/drive_c/3dfx"
DLL="${1:-$BUILDROOT/H5/W2K/Src/Video/Displays/H5/objfre/i386/3dfxvs.dll}"
SRCDIR="3dfx Driver Code/H5/W2K/Src/Video/Displays/H5"
OBJDIR="$BUILDROOT/H5/W2K/Src/Video/Displays/H5/objfre/i386"
fail=0

[ -f "$DLL" ] || { echo "FAIL  DLL not found: $DLL"; exit 1; }
echo "artifact: $DLL ($(stat -c%s "$DLL") bytes, $(date -r "$DLL" '+%F %T'))"

chk() { # chk <desc> <string>
  if strings -a "$DLL" | grep -q -- "$2"; then echo "PASS  $1"; else echo "FAIL  $1 [missing string: $2]"; fail=1; fi
}

echo "== instrumentation/fix strings present in binary =="
chk "H3MakeRoom breaker"        "retro3dfx H3MakeRoom WEDGE-BREAK@50M"
chk "DdFlip breaker"            "retro3dfx DdFlip WEDGE-BREAK@50M"
chk "DP2 error logging"         "retro3dfx DP2-PARSE-ERR"
chk "DP2 exit logging"          "retro3dfx DP2-EXIT-ERR"
chk "SLIAA promote logging"     "retro3dfx PROMOTE-SLIAA"
chk "SLIAA compute logging"     "retro3dfx COMPUTE-SLIAA"
chk "FXBUSYWAIT breaker"        "retro3dfx FXBUSYWAIT WEDGE-BREAK@100M"
chk "H3GpWait breaker"          "retro3dfx H3GpWait WEDGE-BREAK@100M"
chk "DdLock flip-wait breaker"  "retro3dfx DdLock-FlipWait WEDGE-BREAK@100M"
chk "DdFlip flip-wait breaker"  "retro3dfx DdFlip-FlipWait WEDGE-BREAK@100M"

echo "== stale-object check (every fixed source older than its .obj) =="
for src in d3txtr ddflip d6dp2 ddfxnt cfifo logfile bitblt ddblt32; do
  s=$(find "$SRCDIR" -maxdepth 1 -iname "$src.c" -printf '%T@' 2>/dev/null | cut -d. -f1)
  o=$(find "$OBJDIR" -maxdepth 1 -iname "$src.obj" -printf '%T@' 2>/dev/null | cut -d. -f1)
  if [ -z "$o" ]; then echo "FAIL  $src.obj missing"; fail=1; continue; fi
  if [ -n "$s" ] && [ "$s" -gt "$o" ]; then
    echo "FAIL  $src.c newer than $src.obj (stale obj — rebuild before deploy)"; fail=1
  else
    echo "PASS  $src.obj up to date"
  fi
done

# DLL must be newer than every obj it links (paranoia against partial links)
newest_obj=$(find "$OBJDIR" -name '*.obj' -printf '%T@\n' | sort -n | tail -1 | cut -d. -f1)
dll_t=$(date -r "$DLL" +%s)
if [ "$dll_t" -ge "${newest_obj:-0}" ]; then echo "PASS  DLL newer than all objs"; else echo "FAIL  DLL older than newest obj"; fail=1; fi

echo "== codegen guards for the two 0x1000008E crashes (DdBlt / DrvBitBlt) =="
# Disassembles the linked DLL and asserts the two NULL-deref fixes are actually
# in the machine code -- catches a stale-obj link OR a preprocessor/config
# regression (DX<7, LF=0) that leaves the source "fixed" but the binary crashing
# (the exact .124 failure mode). SKIP (rc 2) if objdump is unavailable.
# NB: this script has cd'd to repo root above, so reference tests/ from there.
python3 tests/codegen_8e_guards.py "$DLL"
cg=$?
if [ "$cg" -eq 1 ]; then fail=1; fi

exit $fail
