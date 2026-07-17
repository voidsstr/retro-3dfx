All tests make entries into a log file. By default the name of the 
log file is "conform.log" and it lives in the current working directory.
All but a few tests create images (ppm) and data (.dat) files. By default,
these files are also in the current directory.

You can change the name of the log file and the location where things
go with the following environment variables:

set GLIDE_CONFORM_LOGFILE=conform.log-cvg
set GLIDE_CONFORM_OUTPUTDIR=d:\output\

Setting them as above would put the logfile, data, and images in
d:\output\, and the name of the logfile would be conform.log-cvg


                     --- List of Tests ---

lfb.exe
 Tests LFB reads and writes to front/back/aux buffers. Tests each
 writemode, tests clip window, tests pixel pipeline flag, tests
 writing with depth buffering. This test does its own LFB reads,
 so no images are generated.

simple-clear.exe
 Tests full and clipped clears on front/back/aux buffers. This test
 does its own LFB reads so no images are generated.

query-res.exe
 Makes sure SstWinOpen() succeeds on all resolutions returned by
 grQueryResolutions(). Calls grQueryResolutions with specific
 requests and verifies that it returns only matching results.

query-state.exe
 Draws some stuff with the default glide state. Mucks with state
 a few times and verifies that grGlideGetState() and grGlideSetState()
 are working.

query.exe
 Calls grGet() for each of the attributes listed in the manpage. Verifies
 that each returns the expected size. Tests that GR_VIEWPORT, GR_STATS_LINES,
 GR_STATS_TRIANGLES_IN, GR_STATS_TRIANGLES_OUT, GR_PIXELS_IN, and GR_PIXELS_OUT
 gets are working.

ras-edge.exe
 Draws lines around the edge of the framebuffer. This was really created 
 to demonstrate a bug.

The following rasterization tests draw various rotations of prims. They all
support either window or clip coords and anti-aliasing via command line
arguments. All but ras-point support applying a random clip window to each
prim drawn. By default they use the full window, but all pay attention to
the -wid, -hei, and -tile arguments if you want smaller images.

ras-line.exe
 Tests lines.

ras-tri.exe
 Tests a single triangle.

ras-tri-2.exe
 Tests 4 triangles with shared edges.

ras-va.exe
 Tests grDrawVertexArray() and grDrawVertexArrayContiguous(). Prims
 are selectable on the command line. They are points, lines, line_strip,
 triangles, triangle_strip, triangle_fan, and polygon.

ras-va-cont.exe
 As above, but uses the continuation mode of grDrawVertexArray().
  
ras-point.exe
 Draws a pattern of points.

The following flex-coords-* tests generate and use random vertex layouts.
They all support either window or clip coords via the command line.
By default they use the full window, but all pay attention to
the -wid, -hei, and -tile arguments if you want smaller images.

flex-coords.exe
 Draws rotations of each prim type. 

flex-coords-depth-color.exe
 Draws each prim with varied depth (z or w) and color.
   
flex-coords-texture.exe
 Draws rotations of each prim with a texture applied. 1 or 2 tmus.
  
flex-coords-texture-fog.exe
 Draws rotations of each prim with a texture and fog applied.

The simple-* tests  support either window or clip coords, and the
-wid, -hei, -tile args.

simple-interp-depth.exe
 Draws all prims with interpolated depth (z or w), and verifies
 that the depth is linearly increasing by staggering lines (depthwise)
 either side of the prims.

simple-interp-alpha.exe
 Draws all prims with the interpolated alpha across fully opaque lines
 for contrast.

simple-interp-rgb.exe
 Draws rotations of all prims with randomly chosen interpolated colors.

simple-interp-tex.exe
 Draws rotations of all prims with decal textures applied, texture * rgb
 applied, fixed texture coords, rotating texture coords, and texture
 coords with varied depth.

simple-ras.exe
 Draws rotations of all prims in constant color.

buffer-swap.exe
  Tests that grBufferSwap() is working.

clear.exe
  Does buffer clears to the color and aux buffers, with and without
  clip windows. Tests that clears with alpha and depth values work.

backface-cull.exe
  Draws rotations of each prim, verifying that the cull mode is working.

The following 4 texture tests apply textures to various rotations of 
triangles. They support window or clip coords, random clip windows,
a selectable texture lod, and the -wid -hei -tile args.

texture-interp.exe
  Draws rotations of a rectangle with texture applied. Draw rotations
  with texture coords rotated.

texture-interp-2tmu.exe
  Similar to above, but with 2 tmus.

texture-interp-large.exe
  Draws rotations of a rectangle with texture applied. Draw rotations
  with texture coords rotated.  Use large texture coords (random, up
  to 10*256.0)

texture-interp-2tmu-large.exe
  Similar to above, but with 2 tmus.

error-callback.exe
  Verifies that grErrorSetCallback() works, and that the error handler
  is called successfully.


alpha-blend.exe
 Test alpha blending with no destination alpha

alpha-buffer.exe
 Test alpha blending with destination alpha

alpha-combine.exe
 Test color combine unit

alpha-test.exe
 Test alpha test functionality

alpha-funky.exe
 Test grAlphaControlsITRGBLighting

color-chroma.exe
 Test chroma keying

color-combine.exe
 Test color combine unit

color-constant.exe
 Test constant color

color-dither.exe
 Test dithering

depth-wbias.exe
 Test W buffer bias

depth-wbuffer.exe
 Test basic W buffering

depth-zbias.exe
 Test Z buffer bias

depth-zbuffer.exe
 Test basic Z buffering

ext-chroma.exe
 Test chroma range

ext-fogcoord.exe
 Test per vertex fog functionality

ext-palette.exe
 Test palette 6666 format

ext-tex-chroma.exe
 Test texture chroma range

ext-texture-mirror.exe
 Test mirroring of single level textures

fog.exe
 Test fog functionality

texture-bias.exe
 Test MIP mapped texture LOD bias

texture-combine-detail.exe
 Test texture combine with detail blending

texture-combine-lod.exe
 Test texture combine with LOD blending

texture-combine-multi.exe
 Test texture combine with multiple TMUs

texture-combine.exe
 Test texture combine functionality

texture-filter.exe
 Test filtering of single level textures

texture-load-format.exe
 Test texture formats

texture-load-level.exe
 Test loading of MIP mapped texture levels

texture-load-mip.exe
 Test loading of MIP mapped textures

texture-load-multi.exe
 Test loading of Multibase textures

texture-load-ncc.exe
 Test NCC texture formats

texture-load-palette.exe
 Test palettized texture formats

texture-load-partial.exe
 Test loading of MIP mapped texture chunks

texture-load-simple.exe
 Test loading of single level textures

texture-lod.exe
 Test MIP mapped texture level of detail

texture-mode.exe
 Test MIP map disable, nearest and dither

texture-wrap.exe
 Test wrapping of single level textures

