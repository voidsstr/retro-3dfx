#!/bin/bash
# Build d3dlab.exe (windowed D3D8 texture/stage lab used by the target tests).
cd "$(dirname "$0")"
i686-w64-mingw32-gcc -O2 -o d3dlab.exe d3dlab.c -ld3d8 -lgdi32 -luser32 || exit 1
echo "built d3dlab.exe ($(stat -c%s d3dlab.exe) bytes)"
