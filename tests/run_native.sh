#!/bin/bash
# Native C driver-logic regression tests.
# Compiles + runs every native/test_*.c natively (host gcc, no Wine/hardware).
# Each is a PURE-LOGIC test guarding a shipped driver fix's invariant.
# Exit non-zero if any test binary fails.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
NAT="$HERE/native"
OBJ="$HERE/.obj"; mkdir -p "$OBJ"
# Host-compiler discovery. This WSL host has no gcc/cc on PATH, so for a whole
# session every suite reported "[BUILD FAIL] ... gcc: command not found" and the
# driver-logic invariants silently never ran. A host compiler does ship in the
# mingw bundle at ~/toolchain-mingw/hostbin (gcc/cc fronting x86_64-linux-gnu-gcc-13);
# note that is the HOST compiler -- the i686-w64-mingw32-* cross tools in the same
# tree build PE binaries and are useless here. cc1 in either path needs the
# bundle's own lib dir on LD_LIBRARY_PATH or it dies on libisl.so.23.
# Override with CC=... or RETRO3DFX_HOSTCC=/path/to/toolchain.
if ! command -v "${CC:-gcc}" >/dev/null 2>&1; then
    for cand in "${RETRO3DFX_HOSTCC:-}" "$HOME/toolchain-mingw"; do
        [ -n "$cand" ] && [ -x "$cand/hostbin/gcc" ] || continue
        export PATH="$cand/hostbin:$PATH"
        export LD_LIBRARY_PATH="$cand/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}"
        echo "  [toolchain] using host compiler from $cand/hostbin"
        break
    done
fi
CC="${CC:-gcc}"
if ! command -v "$CC" >/dev/null 2>&1; then
    echo "FAIL  no host C compiler ('$CC') — the native suites did NOT execute."
    echo "      Install one, or set CC=/path/to/gcc or RETRO3DFX_HOSTCC=<toolchain-root>."
    exit 1                     # never let a missing compiler read as a pass
fi
CFLAGS="-std=c11 -O0 -g -Wall -Wextra -I$HERE -I$NAT"
rc=0
found=0

for src in "$NAT"/test_*.c; do
    [ -e "$src" ] || continue
    found=1
    name="$(basename "$src" .c)"
    bin="$OBJ/$name"
    if ! $CC $CFLAGS "$src" -lm -o "$bin" 2>"$OBJ/$name.build.log"; then
        echo "  [BUILD FAIL] $name"; cat "$OBJ/$name.build.log"; rc=1; continue
    fi
    if ! "$bin"; then rc=1; fi
done

[ $found -eq 0 ] && echo "  (no native/test_*.c yet)"
exit $rc
