#!/bin/bash
# Native C driver-logic regression tests.
# Compiles + runs every native/test_*.c natively (host gcc, no Wine/hardware).
# Each is a PURE-LOGIC test guarding a shipped driver fix's invariant.
# Exit non-zero if any test binary fails.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
NAT="$HERE/native"
OBJ="$HERE/.obj"; mkdir -p "$OBJ"
CC="${CC:-gcc}"
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
