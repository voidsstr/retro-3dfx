#!/usr/bin/env bash
# Build the 3dfx VSA-100 (Voodoo4/5) C-model simulator NATIVELY on Linux x86-64.
# Source: 3dfx H5 leaked tree "3dfx Driver Code/H5/CSIM" (unreleased in-progress).
# Produces libcsim.a — link a harness against it to drive triangles and read the
# LFB, so ICD/glide texcoord+rasterizer fixes iterate LOCALLY in seconds (no .143,
# no Wine). CSIM has full per-pixel GDBG tracing (see H5/CSIM/README):
#   level 127/128 = triangle setup + vertex info; 134 = pixel x,y+count;
#   150 = put_pixel[x,y]; 149 = rgba after dither; 170 = texture u,v; 177 = bilinear.
# Set GDBG_LEVEL to dump exactly which destination columns the rasterizer writes
# -> directly shows the 2D-text column-drop bug at the hardware-model level.
set -e
SRC="$(cd "$(dirname "$0")/../../3dfx Driver Code/H5" && pwd)"
D="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$D/inc" "$D/src" "$D/obj"
# lowercase symlink headers (Linux is case-sensitive; the 2000 code #includes lowercase)
for dir in "$SRC/INCLUDE" "$SRC/INCSRC" "$SRC/CSIM" "$SRC/../SWLIBS/INCLUDE" "$SRC/GLIDE3/SRC"; do
  [ -d "$dir" ] || continue
  for f in "$dir"/*.[Hh]; do [ -e "$f" ] || continue
    b="$(basename "$f" | tr 'A-Z' 'a-z')"; [ -e "$D/inc/$b" ] || ln -s "$f" "$D/inc/$b"; done
done
# editable header/source copies for the 3 portability fixes:
# (1) PRIVATE macros: cast-lvalue -> proper lvalue (modern gcc)
rm -f "$D/inc/csim.h"
cp "$SRC/CSIM/CSIM.H" "$D/inc/csim.h"
sed -i 's#((CsimPrivate \*)((sst)->reservedD\[1\]))#(*(CsimPrivate **)(void*)\&((sst)->reservedD[1]))#' "$D/inc/csim.h"
sed -i 's#((CsimPrivate \*)((sstg)->unused0))#(*(CsimPrivate **)(void*)\&((sstg)->unused0))#' "$D/inc/csim.h"
sed -i 's#((TmuData \*)((sst)->reservedD\[0\]))#(*(TmuData **)(void*)\&((sst)->reservedD[0]))#' "$D/inc/csim.h"
# (2) setup.c static round() clashes with math.h round() -> csim_round
cp "$SRC/CSIM/SETUP.C" "$D/src/setup.c"; sed -i 's/\bround\b/csim_round/g' "$D/src/setup.c"
# (3) lowercase-symlink the remaining .C sources
for f in "$SRC/CSIM"/*.C; do b="$(basename "$f" .C | tr 'A-Z' 'a-z')"; [ "$b" = setup ] && continue; ln -sf "$f" "$D/src/$b.c"; done
# generate h3asm.h (a host tool emits a hex table header)
gcc -w -I"$D/inc" -o "$D/obj/h3asm" "$D/src/h3asm.c"; "$D/obj/h3asm" -hex > "$D/inc/h3asm.h"
# compile the sim (CSIM only, not HSIM -> no tstbench.h / no PC-hw deps)
CFLAGS="-c -O1 -DH4 -DBUILD_HAL -DHAL_CSIM -DGDBG_INFO_ON -w -I$D/inc"
objs=""
for f in "$D"/src/*.c; do b="$(basename "$f" .c)"; [ "$b" = h3asm ] && continue
  gcc $CFLAGS "$f" -o "$D/obj/$b.o"; objs="$objs $D/obj/$b.o"; done
ar rcs "$D/libcsim.a" $objs
echo "OK: libcsim.a ($(ar t "$D/libcsim.a" | wc -l) objects). Link a harness (see harness/ TODO) to drive triangles + dump LFB."
