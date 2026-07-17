REM
REM You can specify other arguments on the command line that will get passed
REM down to the tests. For instance 'runtests -i' will run them all in
REM interactive mode and they'll prompt you to hit return after each
REM frame. 'runtests -res 800x600' will run them all in 800x600 resolution,
REM etc.
REM

set MORE_ARGS=%1% %2% %3% %4% %5% %6% %7%

REM 
REM See if grErrorSetCallback() works.
REM
error-callback %MORE_ARGS%

REM
REM Run the LFB tests, call it for each COLOR_FORMAT
REM The tests themselves cycle through LFB write modes, the color and aux 
REM buffers, with and without the pixel pipeline enabled.
REM
lfb -cf ARGB %MORE_ARGS%
REM lfb -mode both %MORE_ARGS%   ## COLORFORMAT no longer used
REM lfb -cf ABGR %MORE_ARGS%   ## COLORFORMAT no longer used
REM lfb -cf BGRA %MORE_ARGS%   ## COLORFORMAT no longer used
REM lfb -cf RGBA %MORE_ARGS%   ## COLORFORMAT no longer used
lfb-region -cf ARGB %MORE_ARGS% 

REM
REM Clear test (clipped and unclipped, color and aux)
REM
simple-clear %MORE_ARGS%
clear %MORE_ARGS%

REM
REM Draw incrementally rotated versions of all prims in constant color.
REM
simple-ras %MORE_ARGS%
simple-ras -aa %MORE_ARGS%
simple-ras -wc %MORE_ARGS%
simple-ras -wc -aa %MORE_ARGS%

REM
REM Draw all prims with interpolated alpha values
REM
simple-interp-alpha %MORE_ARGS%

REM
REM Draw all prims with interpolated depth (W and Z) values, compare
REM against constant-depth lines staggered on each side of the prims.
REM
simple-interp-depth %MORE_ARGS%
simple-interp-depth -z %MORE_ARGS%

REM
REM Draw all prims with interpolated RGB colors
REM
simple-interp-rgb %MORE_ARGS%
simple-interp-rgb -aa %MORE_ARGS%

REM
REM Draw incrementally rotated triangles with a single texture applied, draw
REM fixed orientation triangled with a single rotated texture applied. Apply
REM the texture as a decal texture, and with interpolated RGB. Also, apply
REM the texture with a different depth at one texture vertex.
REM
simple-interp-tex %MORE_ARGS%
simple-interp-tex -aa %MORE_ARGS%
simple-interp-tex -wc %MORE_ARGS%
simple-interp-tex -wc -aa %MORE_ARGS%

REM
REM More rasterization tests
REM
ras-tri %MORE_ARGS%
ras-tri -aa %MORE_ARGS%
ras-tri -cw %MORE_ARGS%
ras-tri -aa -cw %MORE_ARGS%

ras-tri-2 %MORE_ARGS%
ras-tri-2 -aa %MORE_ARGS%
ras-tri-2 -cw %MORE_ARGS%
ras-tri-2 -aa -cw %MORE_ARGS%

ras-tri-2 -wc %MORE_ARGS%
ras-tri-2 -wc -aa %MORE_ARGS%
ras-tri-2 -wc -cw %MORE_ARGS%
ras-tri-2 -wc -aa -cw %MORE_ARGS%

ras-line %MORE_ARGS%
ras-line -aa %MORE_ARGS%
ras-line -cw %MORE_ARGS%
ras-line -cw -aa %MORE_ARGS%

ras-point  %MORE_ARGS%
ras-point -aa %MORE_ARGS%

REM
REM Raster tests of grDrawVertexArray()
REM
ras-va -mode points %MORE_ARGS%
ras-va -mode points -aa %MORE_ARGS%
ras-va -mode points -cw %MORE_ARGS%
ras-va -mode points -aa -cw %MORE_ARGS%

ras-va -mode lines %MORE_ARGS%
ras-va -mode lines -aa %MORE_ARGS%
ras-va -mode lines -cw %MORE_ARGS%
ras-va -mode lines -aa -cw %MORE_ARGS%

ras-va -mode line_strip %MORE_ARGS%
ras-va -mode line_strip -aa %MORE_ARGS%
ras-va -mode line_strip -cw %MORE_ARGS%
ras-va -mode line_strip -aa -cw %MORE_ARGS%

ras-va -mode triangles %MORE_ARGS%
ras-va -mode triangles -aa %MORE_ARGS%
ras-va -mode triangles -cw %MORE_ARGS%
ras-va -mode triangles -aa -cw %MORE_ARGS%

ras-va -mode triangle_strip %MORE_ARGS%
ras-va -mode triangle_strip -aa %MORE_ARGS%
ras-va -mode triangle_strip -cw %MORE_ARGS%
ras-va -mode triangle_strip -aa -cw %MORE_ARGS%

ras-va -mode triangle_strip -wc %MORE_ARGS%
ras-va -mode triangle_strip -wc -aa %MORE_ARGS%
ras-va -mode triangle_strip -wc -cw %MORE_ARGS%
ras-va -mode triangle_strip -wc -aa -cw %MORE_ARGS%

ras-va -mode triangle_fan %MORE_ARGS%
ras-va -mode triangle_fan -aa %MORE_ARGS%
ras-va -mode triangle_fan -cw %MORE_ARGS%
ras-va -mode triangle_fan -aa -cw %MORE_ARGS%

ras-va -mode polygon %MORE_ARGS%
ras-va -mode polygon -aa %MORE_ARGS%
ras-va -mode polygon -cw %MORE_ARGS%
ras-va -mode polygon -aa -cw %MORE_ARGS%

REM
REM Raster tests of grDrawVertexArrayContiguous()
REM (note: the checksums for these and the non-Contiguous tests should match)
REM
ras-va -contig -mode points %MORE_ARGS%
ras-va -contig -mode points -aa %MORE_ARGS%
ras-va -contig -mode points -cw %MORE_ARGS%

ras-va -contig -mode lines %MORE_ARGS%
ras-va -contig -mode lines -aa %MORE_ARGS%
ras-va -contig -mode lines -cw %MORE_ARGS%
ras-va -contig -mode lines -aa -cw %MORE_ARGS%

ras-va -contig -mode line_strip %MORE_ARGS%
ras-va -contig -mode line_strip -aa %MORE_ARGS%
ras-va -contig -mode line_strip -cw %MORE_ARGS%
ras-va -contig -mode line_strip -aa -cw %MORE_ARGS%

ras-va -contig -mode triangles %MORE_ARGS%
ras-va -contig -mode triangles -aa %MORE_ARGS%
ras-va -contig -mode triangles -cw %MORE_ARGS%
ras-va -contig -mode triangles -aa -cw %MORE_ARGS%

ras-va -contig -mode triangle_strip %MORE_ARGS%
ras-va -contig -mode triangle_strip -aa %MORE_ARGS%
ras-va -contig -mode triangle_strip -cw %MORE_ARGS%
ras-va -contig -mode triangle_strip -aa -cw %MORE_ARGS%

ras-va -contig -mode triangle_fan %MORE_ARGS%
ras-va -contig -mode triangle_fan -aa %MORE_ARGS%
ras-va -contig -mode triangle_fan -cw %MORE_ARGS%
ras-va -contig -mode triangle_fan -aa -cw %MORE_ARGS%

ras-va -contig -mode polygon %MORE_ARGS%
ras-va -contig -mode polygon -aa %MORE_ARGS%
ras-va -contig -mode polygon -cw %MORE_ARGS%
ras-va -contig -mode polygon -aa -cw %MORE_ARGS%

REM
REM Raster test for grDrawVertexArray() with "continue" prims
REM
ras-va-cont -mode strip %MORE_ARGS%
ras-va-cont -mode strip -aa %MORE_ARGS%
ras-va-cont -mode strip -cw %MORE_ARGS%
ras-va-cont -mode strip -aa -cw %MORE_ARGS%

ras-va-cont -mode fan %MORE_ARGS%
ras-va-cont -mode fan -aa %MORE_ARGS%
ras-va-cont -mode fan -cw %MORE_ARGS%
ras-va-cont -mode fan -aa -cw %MORE_ARGS%

REM
REM Raster test for grDrawVertexArrayContiguous() with "continue" prims
REM (note: the checksums for these and the non-Contiguous tests should match)
REM
ras-va-cont -contig -mode strip %MORE_ARGS%
ras-va-cont -contig -mode strip -aa %MORE_ARGS%
ras-va-cont -contig -mode strip -cw %MORE_ARGS%
ras-va-cont -contig -mode strip -aa -cw %MORE_ARGS%

ras-va-cont -contig -mode fan %MORE_ARGS%
ras-va-cont -contig -mode fan -aa %MORE_ARGS%
ras-va-cont -contig -mode fan -cw %MORE_ARGS%
ras-va-cont -contig -mode fan -aa -cw %MORE_ARGS%

REM
REM Texture tests
REM
texture-lod  %MORE_ARGS%
texture-bias  %MORE_ARGS%
texture-load-mip %MORE_ARGS%
texture-load-level %MORE_ARGS%
texture-load-partial %MORE_ARGS%
texture-load-format %MORE_ARGS%
texture-load-palette %MORE_ARGS%
texture-load-simple %MORE_ARGS% 
texture-load-multi %MORE_ARGS%
texture-load-ncc.exe %MORE_ARGS%

texture-interp %MORE_ARGS%
texture-interp -atc %MORE_ARGS%
texture-interp-large %MORE_ARGS%
texture-interp-large -atc %MORE_ARGS%
texture-interp-2tmu %MORE_ARGS%
texture-interp-2tmu -atc %MORE_ARGS%
texture-interp-2tmu -cw %MORE_ARGS%
texture-interp-2tmu -cw -atc %MORE_ARGS%
texture-interp-2tmu-large %MORE_ARGS%
texture-interp-2tmu-large -atc %MORE_ARGS%
texture-interp-3df %MORE_ARGS%

REM
REM Backface culling test
REM 
backface-cull %MORE_ARGS%

REM Flexible Vertex Layout tests
flex-coords %MORE_ARGS%
flex-coords -wc %MORE_ARGS%

flex-coords-depth-color %MORE_ARGS%
flex-coords-depth-color -z %MORE_ARGS%
flex-coords-depth-color -wc %MORE_ARGS%
flex-coords-depth-color -wc -z %MORE_ARGS%

flex-coords-texture %MORE_ARGS%
flex-coords-texture -2tmus %MORE_ARGS%
flex-coords-texture -wc %MORE_ARGS%
flex-coords-texture -wc -2tmus %MORE_ARGS%

flex-coords-texture-fog %MORE_ARGS%
flex-coords-texture-fog -2tmus %MORE_ARGS%
flex-coords-texture-fog -wc %MORE_ARGS%
flex-coords-texture-fog -wc -2tmus %MORE_ARGS%

REM Color tests
color-combine %MORE_ARGS%
color-constant %MORE_ARGS%
color-dither %MORE_ARGS%
color-chroma %MORE_ARGS%

REM Fog tests
fog %MORE_ARGS%

REM Depth
depth-wbias  %MORE_ARGS%
depth-wbuffer  %MORE_ARGS%
depth-zbias  %MORE_ARGS%
depth-zbuffer  %MORE_ARGS%

REM Alpha combine
alpha-combine %MORE_ARGS%
alpha-blend  %MORE_ARGS%
alpha-buffer  %MORE_ARGS%
alpha-test  %MORE_ARGS%
alpha-funky  %MORE_ARGS%

REM Texture extensions
ext-chroma  %MORE_ARGS%
ext-fogcoord  %MORE_ARGS%
ext-palette  %MORE_ARGS%
ext-tex-chroma  %MORE_ARGS%
ext-texture-mirror  %MORE_ARGS%

REM Buffer swap
buffer-swap %MORE_ARGS%

REM Query tests
query-res  %MORE_ARGS%
query  %MORE_ARGS%
query-state  %MORE_ARGS%

disable-effects %MORE_ARGS%

