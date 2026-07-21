/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name: d3txtr.c
**
** Description: Texture allocation / destruction / creation functions.
**
** $Revision: 40$
** $Date: 10/25/00 4:58:50 AM$
**
** $Log: 
**  40   3dfx      1.28.1.0.1.3.1.510/25/00 Johnny Trainor  Updated so we no longer
**       use surface local pointers.
**  39   3dfx      1.28.1.0.1.3.1.410/11/00 Brent           Forced check in to
**       enforce branching.
**  38   3dfx      1.28.1.0.1.3.1.309/08/00 Russ Lind       in TEXTURELOAD,
**       invalidate _D3(lastContext) by setting it to 0 rather than 0xADE
**  37   3dfx      1.28.1.0.1.3.1.209/05/00 Russ Lind       in TEXTURELOAD, set
**       flag in TXTRDESC to track blt to texture
**  36   3dfx      1.28.1.0.1.3.1.108/10/00 Russ Lind       When a texture is
**       destroyed call ReleaseTxtrHndl to free the TXTRHNDL rather than just
**       memsetting the TXTRHNDL to 0.  This reduces the amount of memory consumed
**       by the driver when 3DMark2000's Texture Rendering Speed tests are run in
**       looping mode.  After about 15 hours, these tests have created over 4
**       million textures!  Somehow these 3DMark2000 tests seem to cause the DX7
**       runtime to not reuse texture handles.
**  35   3dfx      1.28.1.0.1.3.1.006/22/00 Russ Lind       fix for DCT 350 Texture
**       Handle failures
**       need to munge the hwPtr and lfbPtr in FXSURFACEDATA for 1x1 textures
**  34   3dfx      1.28.1.0.1.306/08/00 Russ Lind       merge from w9x, just in
**       case we ever get KNI Texture Download enabled on w2k
**  33   3dfx      1.28.1.0.1.206/08/00 Leo Galway      Updated fixes from MS (for
**       issues found when running NT Stress tests) and added Tim Little's aspect
**       ratio check and source and destination suface check (in TEXTURELOAD) which
**       fixes an AV when running Quake III with MS's D3D OGL wrapper and NT Stress
**       Test issues respectively.
**  32   3dfx      1.28.1.0.1.106/02/00 Russ Lind       modified dynamic memory
**       allocation code so MEMCHECK tracking can be enabled
**       plugged memory leaks in TEXTURESURFACECREATE
**  31   3dfx      1.28.1.0.1.005/31/00 Russ Lind       fix for PRS 14268, don't
**       clear texture handle(s) in TEXTURESURFACEDELETE
**       See TEXTURESURFACEDELETE for additional comments
**  30   Napalm R1 1.28.1.0    04/27/00 Russ Lind       mod from MS, added two
**       asserts in CREATENEWTEXTURE (ifdef'd for WINNT)
**  29   3dfx      1.28        03/23/00 Tim Little      Fixed a problem with
**       partial texture download for WinBech2000 Qaulity test #60
**  28   3dfx      1.27        03/21/00 Russ Lind       correction for w2k to
**       surface addr calc in stretchDXTn, shrinkDXTn, stretchFXT1 & shrinkFXT1, on
**       nt4/w2k fpVidMem is the offset from the start of the frame buffer
**  27   3dfx      1.26        03/20/00 Christopher Wilcox Removed AGP texture
**       download support.
** 
**  26   3dfx      1.25        03/17/00 Tim Little      Changed address
**       calculations for compressed textures.  Fixes PRSs 13041, 13042.
**  25   3dfx      1.24        03/13/00 Bob Seitsinger  Changed debug level for
**       'tlog=...' message in TEXTURESURFACECREATE from 0 to 6. Too many messages
**       in SoftIce when always on.
**  24   3dfx      1.23        02/29/00 Bob Seitsinger  Added debug messages.
**  23   3dfx      1.22        02/04/00 Steve Houston   Inserted #undef K6_2 under
**       WinNT builds to remove 3DNow optimizations that are not supported under
**       NT. Related to new triangle asm port from W9X.
**  22   3dfx      1.21        02/01/00 Tim Little      Never download more than
**       64k of texture data at a time so we don't overflow the hole counting logic
**       on the chip.
**  21   3dfx      1.20        01/27/00 Tim Little      Fixed a problem with the
**       formatFlags not being initialized for FXT1 textures.
** 
**  20   3dfx      1.19        01/13/00 Bob Seitsinger  Constant name error
**       SST_TEX_ADDRESS64MB should be SST_TEX2_ADDRESS.
**  19   3dfx      1.18        12/22/99 Mark McMahon    Fix for PRS 11099, check
**       hSurf != 0 before clearing texture handles.  
**  18   3dfx      1.17        12/21/99 Tim Little      Finished FXT1 > 8:1 aspect
**       ratio support
**  17   3dfx      1.16        12/16/99 Russ Lind       don't bother including
**       stbperf.inc for WINNT, it's already in the precompiled header courtesty of
**       d3global.h
**       removed some obsolete WINNT specific DXTn code
**       added some debug output in TXTRDOWNLOADPALETTE
**  16   3dfx      1.15        12/15/99 Tim Little      Some stuff for the FXT1 >
**       8:1 aspect texture stuff.  It is disabled for now.
**  15   3dfx      1.14        11/29/99 Tim Little      Fixed an un-initialized
**       variable.
**  14   3dfx      1.13        11/23/99 Tim Little      Fixed several addressing
**       problems with DXTn textures.
**  13   3dfx      1.12        11/17/99 Russ Lind       partial fix for PC99
**       TextureSizes failures
**       1x1 textures need to be aligned 8 bytes from vaddr0
**  12   3dfx      1.11        11/11/99 Russ Lind       tweaks to DXTn surface size
**       calculation and address computation for w2k (maybe this applies to win9x
**       as well, I have no idea)
**       slightly too much memory was being allocated for DXT1 mipmap chains
**       slightly too little memory was being allocated for DXT5 mipmap chains
**       and the 1x1 address was overlapping the 2x2 space (according to the
**       documentation, sub 4x4 DXTn blocks still require a 4x4 block of memory)
**  11   3dfx      1.10        10/29/99 Christopher Wilcox Completed merge of
**       h3defs.h.
** 
**  10   3dfx      1.9         10/18/99 Russ Lind       in initMipMapChain don't
**       let slg or tlg go negative on NT
**       in TEXTURELOAD, if the src is in video memory add ppdev->pjLfbBase to
**       texels on NT
**  9    3dfx      1.8         10/18/99 Tim Little      Fixed base address
**       computations for DXT1/FXT1 textures.
**  8    3dfx      1.7         09/30/99 Chris W. Shaw   Missed an occurance of
**       taking out 16 byte alignment for Big Textures.
**  7    3dfx      1.6         09/30/99 Chris W. Shaw   Took out 16 byte alignment.
**       Fixes PRS 8811 and some DCT250 mip filter tests. 
**  6    3dfx      1.5         09/24/99 Russ Lind       for DX7, in
**       TEXTURESURFACEDELETE, clear TXTRHNDL for surface being destroyed
**  5    3dfx      1.4         09/17/99 Tim Little      Initial changes for support
**       for FXT1 textures, nothing done to handle > 8:1 aspect textures currently.
**  4    3dfx      1.3         09/14/99 Russ Lind       additional checking in
**       TEXTUREDELETESURFACE for DX7 when clearing texture handle from RC
**       fixes blue screen running 3D WB at 1280x1024 and 1600x1200
**  3    3dfx      1.2         09/14/99 Chris W. Shaw   Changed TextureLoad's dx6
**       code to check for dstHandle<MAXTEXTURECOUNT.  PRS 8387
**  2    3dfx      1.1         09/14/99 Tim Little      Support for > 8:1 aspect
**       ratio DXTn textures and fix for PRS 8669
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
**
** 24    8/30/99 10:16a Russ
** in TEXTUREDELETESURFACE for DX7, clear texture handles in renderstate
** and texture stage state if the surface being destroyed is in one of
** those (fixes access violations in ddiValidateTextureStageState running
** PC99 #29 MultiTexturing test of DCT 250.3 on w2k)
** also in TEXTUREDELETESURFACE for W2K, use memMgr_freeSurface instead of
** directly using VidMemFree
** add some debug output in initMipMapChain
** for 16 byte alignment to vaddr1 in txtrComputeVAddr
**
** 23    8/16/99 3:15p Russ
** modify TEXTURELOAD to return a ddraw or d3d error code rather than
** driver handled for driver nothandled from Bob Holden of IGX
**
** 22    8/13/99 3:24p Russ
** enforce 16 byte alignment of mip levels in txtrComputeVAddr &
** txtrCalcMemRequired
**
** 21    8/12/99 2:36p Russ
** change debug output message in TEXTURELOAD for W2K DX7
**
** 20    8/11/99 9:45a Msmith
** added include for stbperf.inc so we can turn on and off performance
** opts.
**
** 19    8/10/99 10:39a Russ
** for DX7, in TEXTURELOAD, modiify check of dwReserved1 to use
** MAXTEXTURECOUNT instead of MAXTEXTUREHANDLE
**               in TXTRINIT, disable loop to clear TXTRHNDL flags
**
** 18    8/02/99 12:32p Russ
** change compile time H4 ifdef to a runtime check
**
** 17    7/30/99 4:34p Msmith
** changed in #if to #ifdef
**
** 16    7/30/99 3:53p Msmith
** Added a faster texture download loop for non-KNI machines
**
** 15    7/27/99 4:02p Tlittle
** Fixes for > 8:1 aspect ratio textures that are large.
**
** 14    7/22/99 4:14p Tlittle
** Now MipMapping is working and large textures < 4meg work.  Textures >
** 4meg fail allocation.  Lg. textures > 8:1 aspect can fail currently.
**
** 13    7/21/99 1:27p Tlittle
** Ok, large textures are working with the following exceptions.
** Textures that are larger than 4meg don't render, and some > 8:1 aspect
** textures will hang.
**
** 12    7/20/99 4:02p Tlittle
** > 256 textures working for some cases, not mip-mapped yet.  Just want a
** fallback rev.  Shouldn't cause problems in current state.
**
** 11    7/08/99 10:24a Tlittle
** More changes for run-time checking of H5
**
** 10    7/06/99 3:35p Cwilcox
** Change to free texture surfaces by calling memmgr_freeSurface function.
**
** 9     7/02/99 4:29p Cwilcox
** Restructured to support texture Blts.
**
** 8     6/30/99 10:26a Tlittle
** Completed DXTn texture issues, including proper memory managment for
** DXT1 < 8 texel wide textures, and mip-mapping for all DXTn textures.
**
** 7     6/23/99 9:12p Russ
** DX7 palettized texture changes (#ifdef'd for DX >= 7)
** modified args passed to TXTRDOWNLOADPALETTE, TXTRNEWPALETTE &
** TXTRNEWPALETTE
**
** 6     6/23/99 11:02a Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
** disable ddiHandleCreate, ddiHandleDestroy, ddiHandleSwap &
** ddiHandleGetSurf for DX >= 7
**
** 5     6/23/99 9:52a Tlittle
** Broke DXTn MipMapped textures, but fixed < 8 pixel wide compressed
** textures.
**
** 4     6/18/99 12:24p Tlittle
** Lots of changes for BIG_TEXTURES that are not currently used, and a
** small change to the txtrCalcMemRequired for compressed textures, it
** forces a small amount of over-allocation for texture 8x8 and smaller,
** needed for padding DXT1 textures.
**
** 2     6/02/99 3:36p Tlittle
** Minor cleanup before I start changing a lot of stuff.  Removed all CVG
** stuff, and #ifdefs for H4 || H5.
**
** 1     6/02/99 6:44a Michael
** Branch from H3
**
** 71    5/30/99 5:39p Edwin
** Remove ifdef MM, multi-monitor support is always enabled.
**
** 70    5/26/99 1:22p Cwilcox
** Added AGP texture download back into TOT.
**
** 69    5/25/99 10:47a Tlittle
**
** 68    5/25/99 10:40a Tlittle
** Forgot to wrap something up in an H5 define check.
**
** 67    5/24/99 4:35p Cwilcox
** Removed AGP texture download.
**
** 66    5/24/99 5:28p Tlittle
** Now supports DXTn textures on H5 hardware.  There are still a few
** problems, but mostly working.
**
** 65    5/17/99 2:59p Tlittle
** Disabled > 256 textures and modified CREATETEXTURE to use bitsPerTexel
** rather than bytesPerTexel since DXT1 is a 4bit per texel format.
**
** 64    4/29/99 3:56p Stb_bseitsin
** 32bit rendering and 32bit texture changes.
**
** 62    4/22/99 3:17p Cwilcox
** Removed declaration and all references to fbiMemorySize.
**
** 61    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
**
** 60    3/26/99 4:14p Cwilcox
** Major modification to memory allocation and management, fix PRS #4229.
**
** 59    3/18/99 12:45p Stb_bburton
** Added deferred texture download support.
**
** 58    3/10/99 1:12p Sreid
** Fix for PRS 3833, 4994, 4996 - Verdict Texture Size test corruption.
** Added code to flush texture cache on texture download.
** Added code to correctly handle non-standard srcPitch for Banshee
** and Voodoo3 (code was corrected in Voodoo2 case earlier.)
** Added code to correctly offset srcSurface data ptr when doing partial
** texture download.
**
** 57    3/05/99 11:06a Cwilcox
** Fix PRS #4851, texture download overruns linear heap.
**
** 56    3/03/99 1:05p Martin
** Use srcpitch when copying from system memory to agp memory in agp
** texture download section.
**
** 55    3/03/99 12:46p Martin
** Further protect agp texture download to occur when afifo=1.
**
** 54    2/28/99 12:40p Martin
** Remove spin-wait.  If agp2x is busted, can cause a hang waiting for 2nd
** apg move to happen.
**
** 53    2/25/99 7:21a Stb_bburton
** Fixed PRS 4487.  K6-enabled driver doesnt kill performance on PIII.
**
** 52    2/20/99 5:17p Martin
** New agp texture download
**
** 51    2/18/99 8:26a Russ
** in ddiHandleDestroy, moved D3DPRINT of handle being destroyed down
** below context validation to prevent access violations
**
** 50    2/08/99 2:10p Ken
** added cpu/OS detection of pentium III (katmai) processors, and added
** katmai-optimized d3d texture download
**
** 49    2/03/99 9:47p Russ
** for NT5, fix for access violation caused when switching from D3D mode
** to DD mode in Kings Quest 7.  ddiHandleDestroy is called with a NULL
** context but SETUP_PPDEV dereferences the context before the context is
** checked for NULL.
**
** 48    1/29/99 10:49a Cshaw
** Added unified headers.
**
** 47    1/12/99 2:17p Cshaw
** Added runtime performance analysis code.
**
** 46    1/12/99 11:28a Sreid
** Fixed broken macro...  My bad.
**
** 45    1/12/99 10:17a Sreid
** Fixed create 8:1 texture pitch problems and AB -> ABAB replication
** bug.  Also fixed special case texture downloads for tall narrow
** textures.
** Partial texture download is still broken.
**
** 44    1/07/99 5:45p Russ
** for NT, if'd out TXTRCHANGEDPALETTE and TXTRNEWPALETTE functions
**
** 43    1/07/99 12:01p Cshaw
** Added #defs for doing wb99 performance analysis.
** To disable areas of the driver use the following environment vars:
** nd=1 (manditory - enables NULLDRIVER)
** OND_TEXD=1 (optional - OverridesNullDriver by executing
** TextureDownloads)
** OND_STATE=1 (optional - OverridesNullDriver by executing
** D3DStateChanges)
** OND_HWSU=1 (optional - OverridesNullDriver by executing Hardware setup)
** OND_ZB=1 (optional - OverridesNullDriver by executing ZBuffer clears)
** OND_TRI=1 (optional - OverridesNullDriver by executing Triangle
** Rendering)
**
** 42    12/23/98 9:36a Sreid
** Fixed min pitch problem broken when partial texture downloads where
** added.
**
** 41    12/21/98 4:59p Sreid
** Yanked partial texture download for Voodoo2 - Monster Truck Madness
** problems...
**
** 40    12/21/98 10:52a Sreid
** Fixed mipmapping corruption/hang problem
**
** 39    12/09/98 6:38p Sreid
** Merged latest K6 optimization code from Metabyte
**
** 38    12/09/98 6:41a Russ
** NT5 D3D changes for Banshee
**
** 37    12/08/98 1:32p Martin
** unsigned -> signed variables.
** Turn off some debug messages.
**
** 36    12/03/98 4:52p Miriam
** For null driver, don't download textures or allocate textures. Trace
** for texture handle flavor.
**
** 35    11/24/98 1:23p Martin
** Partial texture download
**
** 34    11/22/98 9:07p Andrew
** Changes to support multi-monitor
**
** 33    11/18/98 6:09p Adrians
** Changes for Avenger.
**
** 32    11/06/98 1:36p Adrians
** Updated K6_2 changes.
**
** 31    10/27/98 10:33a Miriam
** fix for alt-tab while running redline racer.
**
** 30    10/19/98 3:12p Artg
** typo h3 not H3
**
** 29    10/19/98 12:24p Artg
** The real change from ifdef h3 to if defined(h3) || defined(h4)
**
** 27    10/07/98 10:39p Adrians
** Added K6-2 optimisations.
**
** 27    10/07/98 9:05a stit (Metabyte, Inc)
** AMD K6-2 (MMX+3DNOW) optimization
**
** 26    10/06/98 2:04p Miriam
** automipmap support for Voodoo2 & merge file with Voodoo2.
**
** 25    9/23/98 6:46p Miriam
** Support for rendering to texture surfaces.
**
** 24    9/21/98 5:17p Miriam
** For future reference. Flush the cache if necessary.
**
** 23    9/21/98 3:15p Adrians
** Remove legacy fog global variable.
**
** 22    9/11/98 2:55p Miriam
** Return error if application creates invalid texture format with fourcc
** code. Fixes G-Police problem where it thinks that we are a PowerVR
** card.
**
** 21    8/19/98 6:40p Suninn
** redline racer fix
**
** 20    8/14/98 3:16p Suninn
** texture lock
**
** 19    7/30/98 12:57p Miriam
** Automipmap textures with alpha. new implementation of automipmapping.
**
** 17    7/22/98 5:18p Miriam
** fix for texture corruption-fail on create texture not load
**
** 16    7/22/98 12:31p Adrians
** Automipmap code.
**
** 15    7/09/98 6:39p Miriam
** Build option for B0 silicon.
**
** 14    7/01/98 11:27a Miriam
** Return error at load time for allocOnLoad textures.
**
** 13    6/30/98 5:29p Miriam
** Performance optimizations.
**
** 12    5/20/98 2:47p Suninn
** use GET_HW_ADDR to access hw addresses for banshee
**
** 11    5/06/98 7:01p Suninn
** for dx6 name changes and texture type
**
** 10    4/09/98 11:50a Miriam
** 1xN & 2xN mipmap download fixed.
**
** 9     3/24/98 2:44p Miriam
** merge change from V2. pixel center is now a variable and not a
** constant.
**
** 8     3/12/98 4:52a Miriam
** Fix winbench problems:
** Pitch on source texture download for 1x1 mipmaps was incorrect
** Clear color with command fifo not set because the offset in packet to
** far.
**
** 7     3/05/98 7:16p Suninn
** add tile
**
** 6     2/18/98 1:32p Miriam
** merge in changes from Voodoo 2 for texel center.
**
** 5     2/13/98 3:39p Miriam
** wouldn't compile with command fifo.
**
** 4     2/13/98 1:18p Miriam
** Enable Z, viewport clear & disable alphablending.
**
** 3     1/29/98 10:59a Suninn
** add pitch field to memMgr_surfaceAlloc()
**
**
** 2     1/27/98 8:51a Miriam
** Texture download baseaddr with command fifo is now correctly sent in
** packet.
**
** 1     1/21/98 4:22p Miriam
** After sharing files with Voodoo2 D3D, these files need to be separate
** for the time being.
 *
 * 2     1/16/98 6:23p Miriam
 * Support linear & tiled textures, 1 TMU and other misc. changes for
 * banshee.
 *
 * 1     12/18/97 5:18p Miriam
 *
 * 22    12/02/97 12:06p Miriam
 * Fix to fix regression of last change. The texture base address uses the
 * adjusted lodmin/max for split textures but the lod uses the
 * non-adjusted values now.
 *
 * 21    11/26/97 10:15a Miriam
 * When textures are split across 2 TMUS the lodmin & max are the same as
 * if they were not split.
 *
 * 20    11/23/97 3:56p Suninn
 * replay _d3Global with _D3 & D3G macros
 *
 * 19    11/23/97 2:11p Suninn
 * replace _ddglobal with _DD macro
 *
 * 18    11/21/97 3:52p Adrians
 * Fix for Final Reality. Actually fixes an app problem NOT driver.
 *
 * 17    11/21/97 3:35p Suninn
 * use _FX() instead of _fxGlobal for easier code sharing
 *
 * 16    11/13/97 1:26p Suninn
 * to make h3 compile - h3 do not have SST_ sequencial 8 bit down load
 * flag anymore.
 *
 * 15    11/09/97 1:24p Adrians
 * Single pk1's use increment of 0.
 * Palette download uses grouped writes.
 *
 * 14    10/27/97 2:25p Adrians
 * Fixed 8 bit texture download.
 * 8 bit palettes are now built as default (p8=1).
 * Incorporated software triangle setup. Added build option (default
 * ss=0).
 * Command fifo debug build option added (default fd=0).
 * Texture clamping build option (default tc=1).
 *
 * 13    10/24/97 7:14p Miriam
 * Support for 2 tmu and trilinear. Will see 2x the memory on a 2 tmu
 * system. Trilinear is 1 pass on 2 tmu & lodDither for 1 tmu.
 * Fixed command fifo problem when using save/restore.
 *
 * 12    10/14/97 4:36p Adrians
 * Hellbender texture fix.
 *
 * 11    10/14/97 11:22a Adrians
 * Added Miriam's 4M texture support.  Added build environment for H3.
 *
 * 10    10/09/97 10:23a Adrians
 * Now have a single SETPH macro.  Tidy-up of macro code.
 *
 * 9     10/08/97 1:36p Adrians
 * Bug fix to texture download of special cases.
 *
 * 8     10/02/97 8:38p Adrians
 * Include init code into build. Enable Write Combining.  Inline system
 * functions.  Change optimisations.  Some code tidy up.
 *
 * 7     9/26/97 11:45a Adrians
 * Now supports proper cmdfifo packet 3 in these modules.
 *
 * 6     9/19/97 12:52p Adrians
 * Fixed problem with TRI_DEFINE macro.
 * Added fifo Pk5 to texture download.
 *
 * 5     9/12/97 3:46p Adrians
 * Multiple register pointers implemented for different chip fields.
 *
 * 4     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/

#include "precomp.h"

#ifndef WINNT
#include <windows.h>
#include <stddef.h>
#include <d3dhal.h>
#include "d3global.h"
#include "ddrawi.h"
#include "ddglobal.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d3contxt.h"
#define  FX_DEFINE_MACROS
#include "d3txtr.h"
#include "ddfxnt95.h"
#else // ifdef WINNT
// shouston 1-29-00 : The K6-2 optimizations in this file are not yet
// implemented under WinNT.
#undef K6_2
#endif

// include #defines for performance optimizations
#ifndef WINNT
#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif
#endif


#ifdef WINNT
#define __NTDDKCOMP__
#include "dmemmgr.h"
#endif

#include "tcutils.h"

#pragma warning (disable : 4799 )

//-----------------------------------------------------------------------
//
// Texture memory management including download.
// This management is a bit of a mess and needs some explanation. I will
// add some when I get some time... Miriam
//
//-----------------------------------------------------------------------

// the size of each mipmap level in texels

static unsigned long txtMipMapSize[4][16] = {
   {  // 1:1 aspect ratio
      0x010000, //  0 :  256x256
      0x004000, //  1 :  128x128
      0x001000, //  2 :   64x64
      0x000400, //  3 :   32x32
      0x000100, //  4 :   16x16
      0x000040, //  5 :    8x8
      0x000010, //  6 :    4x4
      0x000004, //  7 :    2x2
      0x000001, //  8 :    1x1
    },
    { // 2:1 aspect ratio
      0x008000, // 0 :  256x128
      0x002000, // 1 :  128x64
      0x000800, // 2 :   64x32
      0x000200, // 3 :   32x16
      0x000080, // 4 :   16x8
      0x000020, // 5 :    8x4
      0x000008, // 6 :    4x2
      0x000002, // 7 :    2x1
      0x000001, // 8 :    1x1
    },
    { // 4:1 aspect ratio
      0x004000, // 0 :  256x64
      0x001000, // 1 :  128x32
      0x000400, // 2 :   64x16
      0x000100, // 3 :   32x8
      0x000040, // 4 :   16x4
      0x000010, // 5 :    8x2
      0x000004, // 6 :    4x1
      0x000002, // 7 :    2x1
      0x000001, // 8 :    1x1
    },
    {// 8:1 aspect ratio
      0x002000, // 0 :  256x32
      0x000800, // 1 :  128x16
      0x000200, // 2 :   64x8
      0x000080, // 3 :   32x4
      0x000020, // 4 :   16x2
      0x000008, // 5 :    8x1
      0x000004, // 6 :    4x1
      0x000002, // 7 :    2x1
      0x000001, // 8 :    1x1
    },
};


static unsigned long txtBigMipMapSize[4][16] = {
   {  // 1:1 aspect ratio
      0x400000, //  0 : 2048x2048
      0x100000, //  1 : 1024x1024
      0x040000, //  2 :  512x512
      0x010000, //  3 :  256x256
      0x004000, //  4 :  128x128
      0x001000, //  5 :   64x64
      0x000400, //  6 :   32x32
      0x000100, //  7 :   16x16
      0x000040, //  8 :    8x8
      0x000010, //  9 :    4x4
      0x000004, // 10 :    2x2
      0x000001, // 11 :    1x1
    },
    { // 2:1 aspect ratio
      0x200000, //  0 : 2048x1024
      0x080000, //  1 : 1024x512
      0x020000, //  2 :  512x256
      0x008000, //  3 :  256x128
      0x002000, //  4 :  128x64
      0x000800, //  5 :   64x32
      0x000200, //  6 :   32x16
      0x000080, //  7 :   16x8
      0x000020, //  8 :    8x4
      0x000008, //  9 :    4x2
      0x000002, // 10 :    2x1
      0x000001, // 11 :    1x1
    },
    { // 4:1 aspect ratio
      0x100000, //  0 : 2048x512
      0x040000, //  1 : 1024x256
      0x010000, //  2 :  512x128
      0x004000, //  3 :  256x64
      0x001000, //  4 :  128x32
      0x000400, //  5 :   64x16
      0x000100, //  6 :   32x8
      0x000040, //  7 :   16x4
      0x000010, //  8 :    8x2
      0x000004, //  9 :    4x1
      0x000002, // 10 :    2x1
      0x000001, // 11 :    1x1
    },
    {// 8:1 aspect ratio
      0x080000, //  0 : 2048x256
      0x020000, //  1 : 1024x128
      0x008000, //  2 :  512x64
      0x002000, //  3 :  256x32
      0x000800, //  4 :  128x16
      0x000200, //  5 :   64x8
      0x000080, //  6 :   32x4
      0x000020, //  7 :   16x2
      0x000008, //  8 :    8x1
      0x000004, //  9 :    4x1
      0x000002, // 10 :    2x1
      0x000001, // 11 :    1x1
    },
};



// the offset from mipmap level 0 of each mipmap level in texels
static long txtMipMapOffset[4][16];
static long txtMipMapOffsetTsplit[4][16];

static long txtBigMipMapOffset[4][16];
static long txtBigMipMapOffsetTsplit[4][16];


 //
 // Level of detail
 //
 // Our texture width/height must be a power of 2. The max is 256x256.
 // So,  LOD level
 //      ---
 //       0     (256 * X) or (X * 256)
 //       1     (128 * X) or (X * 256)
 //       2      64 ...
 //       3      32 ...
 //       4      16
 //       5       8
 //       6       4
 //       7       2
 //       8       1
 //
# define TBIG_LOD_2048  0
# define TBIG_LOD_1024  1
# define TBIG_LOD_512   2
# define TBIG_LOD_256   3
# define TBIG_LOD_128   4
# define TBIG_LOD_64    5
# define TBIG_LOD_32    6
# define TBIG_LOD_16    7
# define TBIG_LOD_8     8
# define TBIG_LOD_4     9
# define TBIG_LOD_2     10
# define TBIG_LOD_1     11

# define LOD_256        0
# define LOD_128        1
# define LOD_64         2
# define LOD_32         3
# define LOD_16         4
# define LOD_8          5
# define LOD_4          6
# define LOD_2          7
# define LOD_1          8

// translate log2 to lod, for example for Voodoo3, if log2 = 8 (2 ** 8 = 256) then LOD = 0
#define LOG2LOD(log2) (MAX_TEXTURE_LOG - log2)
#define LOG2LODTBIG(log2) (MAX_BIGTEXTURE_LOG - log2)

#define  NO_LUCK -1
signed   int    __stdcall ALLOCTEXTUREDESC(NT9XDEVICEDATA * ppdev, TXTRDESC *txtrList);
         void   __stdcall FREETEXTUREDESC(NT9XDEVICEDATA * ppdev, int);
signed   int    __stdcall ALLOCTEXTUREHANDLE(NT9XDEVICEDATA * ppdev, TXTRHNDL *txtrList);
         void   __stdcall FREETEXTUREHANDLE(NT9XDEVICEDATA * ppdev, int);
         long   __stdcall txtrCalcBaseAddress(long start, long lod_min, long ar, long bitsPerTexel, ULONG home,long tbig);
         long   __stdcall txtrCalcBaseAddressDXT1(long tmu, long lodmax, TXTRDESC *txtr);
         long   __stdcall txtrCalcMemRequired(long lodmin, long lodmax, long ar, long bitsPerTexel,
                                              unsigned long mipmapsOnTmu,int TxtrIsBig);
long __stdcall txtrCalcMemRequiredDXT1(long lodmin, long lodmax, long ar, long bitsPerTexel,
                                       unsigned long mipmapsOnTmu, long slog, long tlog,int TxtrIsBig);
unsigned long * __stdcall CREATENEWTEXTURE(NT9XDEVICEDATA   * ppdev,
                                           unsigned long    *texels,
                                           long             slog,
                                           long             tlog,
                                           long             srcPitch,
                                           long             bytesPerTexel,
                                           d3Global         *g) ;

unsigned long __stdcall  txtrDownLoadTexture(NT9XDEVICEDATA * ppdev,
                                             int            trex,
                                             unsigned long  addr,
                                             int            tiled,
                                             int            tStride,
                                             int            ar,
                                             int            slog,
                                             int            tlog,
                                             int            bpt,
                                             int            srcPitch,
                                             int            dstPitch,
                                             int            srcStartOffset,
                                             int            dstStartOffset,
                                             int            width,
                                             int            height,
                                             unsigned long  *data,
                                             unsigned long  home,
                                             unsigned long  flags,
                                             unsigned long  format,
                                             unsigned long  txtrIsBig) ;
DWORD txtrComputeVAddr( DWORD maxST, TXTRDESC *txtr, LPDWORD vaddr0, LPDWORD vaddr1);

static float textureCenter[12] = {
 1.f/2.f, 1.f/4.f, 1.f/8.f, 1.f/16.f,
 1.f/32.f, 1.f/64.f, 1.f/128.f, 1.f/256.f,
 1.f/512.f,1.f/1024.f,1.f/2048.f,1.f/4096.f
};


/*-------------------------------------------------------------------
Function Name: txtrComputeCompSize
Description:   calculates the size of a compressed texture.
--------------------------------------------------------------------*/
static DWORD txtrComputeCompSize(DWORD maxST,TXTRDESC *txtr)
{
   long  slg,tlg;
   DWORD size;

   // Compute the slog and tlog
   if (txtr->slog >= txtr->tlog)
   {
      slg = maxST;
      tlg = (long)maxST - (long)txtr->aspectRatio;
   }
   else
   {
      tlg = maxST;
      slg = (long)maxST - (long)txtr->aspectRatio;
   }

   if (((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_DXT1 | SST_COMPRESSED_TEXTURES)) ||
      ((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_FXT1 | SST_COMPRESSED_TEXTURES)))
   {  // DXT1 & FXT1 both have a minimum width of 8
      if (slg < 3)
         slg = 3;
   }
   else
   {  // All others have a minimum width of 4
      if (slg < 2)
         slg = 2;
   }
   if (tlg < 2)   // All compressed formats have a minimum height of 4
      tlg = 2;

   size = 1 << (slg + tlg); // the size = (width * height) * 1byte per texel.
   if (((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_DXT1 | SST_COMPRESSED_TEXTURES)) ||
      ((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_FXT1 | SST_COMPRESSED_TEXTURES)))
   {  // These formats are only 4bpt not 8bpt.
      size = size >> 1;
   }
   return size;
}

/*-------------------------------------------------------------------
Function Name:  initMipMapChain
Description:    sets up pointers for each LOD in a mipmapped chain.
Information:

Return:          nothing
-------------------------------------------------------------------*/
static void initMipMapChain(LPDDRAWI_DDRAWSURFACE_LCL surf, TXTRDESC *txtr,
                            NT9XDEVICEDATA *ppdev, long slog, long tlog,
                            int bitsPerTexel,int txtrID)
{
   // Must put a ptr in fpVidMem or DDRAW will implicitly allocate a memory from the
   // texture heap and this must be done for every mipmap level.
   LPDDRAWI_DDRAWSURFACE_LCL next;
   DWORD slg,tlg,vaddr0,vaddr1;
   next = surf;
   slg = slog;
   tlg = tlog;
   vaddr0 = txtr->start[TREX0] + _FX(textureHeapStart[TREX0]);
   vaddr1 = txtr->start[TREX1] + _FX(textureHeapStart[TREX1]);
   next->lpGbl->fpVidMem = txtrComputeVAddr(max(slg,tlg),txtr,&vaddr0,&vaddr1);

   // partial fix for PC99 TextureSizes failures
   // 1x1 textures need to be aligned 8 bytes from vaddr0 (a hw feature I guess)
   if ((1 == next->lpGbl->wWidth) && (1 == next->lpGbl->wHeight))
   {
     next->lpGbl->fpVidMem += 8;
     // fix for TextureHandle failures in DCT350
     if (NULL != (FXSURFACEDATA *)next->lpGbl->dwReserved1)
     {
       FXSURFACEDATA *surfData = (FXSURFACEDATA *)next->lpGbl->dwReserved1;
       surfData->hwPtr += 8;
       surfData->lfbPtr += 8;
     }
   }

   D3DPRINT(D3DDBGLVL, "  initMipMapChain");

   if ((txtr->format == (SST_DXT1 | SST_COMPRESSED_TEXTURES))
       || (txtr->format == (SST_DXT2 | SST_COMPRESSED_TEXTURES))
       || (txtr->format == (SST_DXT3 | SST_COMPRESSED_TEXTURES))
       || (txtr->format == (SST_DXT4 | SST_COMPRESSED_TEXTURES))
       || (txtr->format == (SST_DXT5 | SST_COMPRESSED_TEXTURES))
       || (txtr->format == (SST_FXT1 | SST_COMPRESSED_TEXTURES)))
   {
      next->lpGbl->dwLinearSize = txtrComputeCompSize(max(slg,tlg),txtr);
      D3DPRINT(D3DDBGLVL, "    surfLcl=%8lXh, fpVidMem=%8lXh, dwLinearSize=%8lXh, width=%8lXh, height=%8lXh",
               next, next->lpGbl->fpVidMem, next->lpGbl->dwLinearSize, next->lpGbl->wWidth, next->lpGbl->wHeight);
   }
   else
   {
      next->lpGbl->lPitch = ((1<<slg) * bitsPerTexel) >> 3;
      D3DPRINT(D3DDBGLVL, "    surfLcl=%8lXh, fpVidMem=%8lXh, pitch=%8lXh, width=%8lXh, height=%8lXh",
               next, next->lpGbl->fpVidMem, next->lpGbl->lPitch, next->lpGbl->wWidth, next->lpGbl->wHeight);
   }

   do
   {
      if (next->lpAttachList != NULL)
      {
         int maxST;
         next = next->lpAttachList->lpAttached;
         // fix for Unreal Tournament Demo not running on w2k
         if (slg)
            --slg;
         if (tlg)
           --tlg;

         maxST = max(slg,tlg);
#ifdef DEFER_TXTR_DNLD
         // Initialize deferral stuff
         txtr->checksum[LOG2LOD(maxST)] = 0;
         txtr->reload |= (1 << LOG2LOD(maxST));
#endif
         next->lpGbl->fpVidMem = txtrComputeVAddr(maxST,txtr,&vaddr0,&vaddr1);
         next->dwReserved1 = txtrID;

         if ((txtr->format == (SST_DXT1 | SST_COMPRESSED_TEXTURES))
             || (txtr->format == (SST_DXT2 | SST_COMPRESSED_TEXTURES))
             || (txtr->format == (SST_DXT3 | SST_COMPRESSED_TEXTURES))
             || (txtr->format == (SST_DXT4 | SST_COMPRESSED_TEXTURES))
             || (txtr->format == (SST_DXT5 | SST_COMPRESSED_TEXTURES))
             || (txtr->format == (SST_FXT1 | SST_COMPRESSED_TEXTURES)))
         {
            next->lpGbl->dwLinearSize = txtrComputeCompSize(maxST,txtr);
            D3DPRINT(D3DDBGLVL, "    surfLcl=%8lXh, fpVidMem=%8lXh, dwLinearSize=%8lXh, width=%8lXh, height=%8lXh",
                     next, next->lpGbl->fpVidMem, next->lpGbl->dwLinearSize, next->lpGbl->wWidth, next->lpGbl->wHeight);
         }
         else
         {
            next->lpGbl->lPitch = ((1<<slg) * bitsPerTexel) >> 3;
            D3DPRINT(D3DDBGLVL, "    surfLcl=%8lXh, fpVidMem=%8lXh, pitch=%8lXh, width=%8lXh, height=%8lXh",
                     next, next->lpGbl->fpVidMem, next->lpGbl->lPitch, next->lpGbl->wWidth, next->lpGbl->wHeight);
         }
      }
      else
      {
         next = NULL;
      }
   } while (next != NULL);
}


#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
// ddiHandleCreate and ddiHandleDestroy are not reported in DX7
#else
/*-------------------------------------------------------------------
Function Name:  ddiHandleCreate
Description:    d3d callback to create a texture handle.
Information:    DWORD __stdcall ddiHandleCreate(LPD3DHAL_TEXTURECREATEDATA ptcd)

                 Notes on texture handle allocation:
                                                  Video Memory   System Memory
                 DD  Texture surface create       allocHandle       -
                 DD  Texture surface delete       deleteHandle      -
                 D3D Texture handle create            -          allocHandle
                 D3D Texture handle destroy           -          deleteHandle

Return:
                DWORD  DDHAL_DRIVER_HANDLED
                ptcd->ddrval =    DDERR_OUTOFVIDEOMEMORY
                          or  D3DHAL_CONTEXT_BAD
                          or  DDERR_CANTLOCKSURFACE
                          or  DD_OK
-------------------------------------------------------------------*/
DWORD __stdcall ddiHandleCreate(LPD3DHAL_TEXTURECREATEDATA ptcd)
{
   SETUP_PPDEV(ptcd->dwhContext)
#ifdef WINNT
   HANDLE  surfInterface;
#else
   LPDDRAWI_DDRAWSURFACE_INT surfInterface;
#endif
   LPDDRAWI_DDRAWSURFACE_LCL surfLCL;

   D3D_ENTRY( "ddiHandleCreate" );

   // This callback is invoked when a Handle is to be created from a
   // DirectDrawSurface.
   if (CONTEXT_VALIDATE(ptcd->dwhContext))
   {
      D3DPRINT( 255, "ddiHandleCreate, bad context =0x08lx", ptcd->dwhContext );
      ptcd->ddrval = D3DHAL_CONTEXT_BAD;
      D3D_EXIT( DDHAL_DRIVER_HANDLED );
   }

#ifdef WINNT

   surfInterface = ptcd->hDDS;
   if (NULL == (surfLCL = EngLockDirectDrawSurface(surfInterface)))
   {
      ptcd->ddrval = DDERR_CANTLOCKSURFACE;
      return DDHAL_DRIVER_HANDLED;
   }
#else
   surfInterface = (LPDDRAWI_DDRAWSURFACE_INT) ptcd->lpDDS ;
   surfLCL       = surfInterface->lpLcl ;
#endif


   ptcd->dwHandle = ALLOCTEXTUREHANDLE(ppdev, TXTRHNDL_PTR(0));
   if (ptcd->dwHandle <= 0)
   {
#ifdef WINNT
      EngUnlockDirectDrawSurface(surfLCL);
#endif
      // MIRIAM: hmmm something fishy here. there is no good return code defined
      D3DPRINT( 255, "Warning: out of texture memory handles" );
      ptcd->ddrval = DDERR_OUTOFVIDEOMEMORY;
      D3D_EXIT( DDHAL_DRIVER_HANDLED );
   }
   TXTRHNDL_PTR(ptcd->dwHandle)->surfInterface = (DWORD)surfInterface;
   TXTRHNDL_PTR(ptcd->dwHandle)->surfLcl       = (DWORD)surfLCL;
#ifndef WINNT
   TXTRHNDL_PTR(ptcd->dwHandle)->processId     = surfLCL->dwProcessId;
#endif
   TXTRHNDL_PTR(ptcd->dwHandle)->contextId     = ptcd->dwhContext;
#ifdef WINNT
   D3DPRINT( 255, "ddiHandleAllocate hDDS =%x handle=%d\n", ptcd->hDDS, ptcd->dwHandle );
#else
   D3DPRINT( 255, "ddiHandleAllocate lpDDS =%x handle=%d\n", ptcd->lpDDS, ptcd->dwHandle );
#endif

#ifdef WINNT
   EngUnlockDirectDrawSurface(surfLCL);
#endif

   ptcd->ddrval   = DD_OK;
   D3D_EXIT( DDHAL_DRIVER_HANDLED );

}

/*-------------------------------------------------------------------
Function Name:  ddiHandleDestroy
Description:    d3d callback to destroy a texture handle.
Information:    DWORD __stdcall ddiHandleDestroy(LPD3DHAL_TEXTUREDESTROYDATA ptcd)
Return:         DWORD DDHAL_DRIVER_HANDLED
            ptcd->ddrval =  DD_OK - upon success.
                            D3DHAL_CONTEXT_BAD upon invalid context.

-------------------------------------------------------------------*/
DWORD __stdcall ddiHandleDestroy(LPD3DHAL_TEXTUREDESTROYDATA ptcd)
{
#ifdef WINNT
   NT9XDEVICEDATA  *ppdev;
#else
   SETUP_PPDEV(ptcd->dwhContext)
#endif
   D3D_ENTRY( "ddiHandleDestroy" );

   if (CONTEXT_VALIDATE(ptcd->dwhContext))
   {
      D3DPRINT( 255, "ddiHandleDestroy, bad context =0x08lx", ptcd->dwhContext );
      ptcd->ddrval = D3DHAL_CONTEXT_BAD;
      D3D_EXIT( DDHAL_DRIVER_HANDLED );
   }
#ifdef WINNT
   ppdev = ((RC *)ptcd->dwhContext)->ppdev;
#endif
   D3DPRINT( 255, "ddiHandleDestroy lpDDS=0x%x handle=%d", TXTRHNDL_PTR(ptcd->dwHandle)->surfInterface,
            ptcd->dwHandle );

   if (TXTRHNDL_PTR(ptcd->dwHandle)->flags & HandleInUse)
   {
      D3DPRINT( 255, "ddiHandleDestroy(System) lpDDS=0x%x handle=%d",
               TXTRHNDL_PTR(ptcd->dwHandle)->surfInterface, ptcd->dwHandle );

      // add this texture handle to free list
      FREETEXTUREHANDLE(ppdev, ptcd->dwHandle );
   }

   ptcd->ddrval = DD_OK;
   D3D_EXIT( DDHAL_DRIVER_HANDLED );

}
#endif // (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)


/*-------------------------------------------------------------------
Function Name:  TextureSurfaceDelete
Description:    Deletes a Texture surface.  Called by Direct Draw.
Information:    void __stdcall TEXTURESURFACEDELETE(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWSURFACE_LCL surfLCL)
Return:         void



-------------------------------------------------------------------*/
void __stdcall TEXTURESURFACEDELETE(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWSURFACE_LCL surfLCL)
{
   TXTRDESC  *txtr;
   DWORD      tmuCnt;
   FXSURFACEDATA *surfaceData;


#if 0//defined( NULLDRIVER )
   return;
#endif

   // video texture surface
   if (!(surfLCL->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) && (surfLCL->dwReserved1 > 0) && (surfLCL->dwReserved1 < MAXTEXTURECOUNT))
   {
      txtr = TXTRDESC_PTR(surfLCL->dwReserved1);

      for (tmuCnt= 0; tmuCnt < NUM_AVAILABLE_TMUS; ++tmuCnt)
      {
         // any miploads to download to this tmu?
         if (txtr->mipmapsOnTmu[tmuCnt])
         {
            // free the video memory
            surfaceData = (FXSURFACEDATA *)surfLCL->lpGbl->dwReserved1;
            if (surfaceData)
            {
              if ((1 == surfLCL->lpGbl->wWidth) && (1 == surfLCL->lpGbl->wHeight))
              {
                surfLCL->lpGbl->fpVidMem -= 8;
                // fix for TextureHandle failures in DCT350
                if (NULL != (FXSURFACEDATA *)surfLCL->lpGbl->dwReserved1)
                {
                  FXSURFACEDATA *surfData = (FXSURFACEDATA *)surfLCL->lpGbl->dwReserved1;
                  surfData->hwPtr -= 8;
                  surfData->lfbPtr -= 8;
                }
              }

              memMgr_freeSurface(
#ifdef WINNT
                                 ppdev,
#else
                                 surfLCL->lpGbl->lpDD,             // direct draw vidmemalloc need this
#endif
                                 surfLCL->ddsCaps.dwCaps,          // type of surface (can use the standard DD surface flags)
                                 surfaceData->lfbPtr,              // host lfb start address of allocation
                                 surfaceData->hwPtr,               // hw start address of allocation
                                 GETMEMTYPE(surfaceData->hwPtr),   // MEM_IN_TILED or MEM_IN_LINEAR
                                 surfaceData->heapID
#ifdef WINNT
                                 , surfaceData->pvmHeap
#endif
                                 );            // Ddraw heap ID
            }
            TXTR_HEAP_SIZE(tmuCnt) += txtr->textureLength[tmuCnt];
            D3DPRINT( 255, "TextureSurfaceDelete(Video) lpDDS=0x%x handle=%d start=0x%x tmu=%d heap=%d heapsize=0x%x",surfLCL, surfLCL->dwReserved1,
            TXTRDESC_PTR(surfLCL->dwReserved1)->start[tmuCnt] + _FX(textureHeapStart[txtr->tmuID[tmuCnt]]),TXTR_HEAP_SIZE(tmuCnt));

         } // some mipmap on the tmu
      } // each tmu

      // add this texture handle to free list
      FREETEXTUREDESC(ppdev, surfLCL->dwReserved1);

      //
      // Zero out the texture ID so that we know that this texture and all its mipmaps are deleted.
      // This is needed so that we don't get confused when the texture is restored.
      //
      surfLCL->dwReserved1 = 0L;
      {
         LPDDRAWI_DDRAWSURFACE_LCL next;
         next = surfLCL;
         do
         {
            if (next->lpAttachList != NULL)
            {
               next = next->lpAttachList->lpAttached ;
               next->dwReserved1 = 0L;
            }
            else
               next = NULL;

         } while (next != NULL);
      }

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
      // The PC99 #29 Multi-Texturing test in the v221 DCT's creates
      // a texture, sets one of the texture stages to the texture, calls
      // ValidateTextureStageState, then destroys the texture and calls
      // ValidateTextureStageState again (without removing the destroyed
      // texture from the texture stage)  Needless to say, this
      // occasionally caused us to AV in ValidateTextureStageState.  oops!
      //
      // According to MS, it is acceptable for the app to do this and it
      // is the driver's responsibility to remove deleted textures from
      // the texture stage states.
      //
      // Noel VanHook of IGX discovered this problem on 8.24.99, we discussed
      // possible solutions
      //
      // This is a slightly modified implementation of what Noel checked into
      // the w2k code base being provided to MS

      {
        #define TS pRc->textureStage

        DWORD hSurf;
        RC    *pRc;

        hSurf = surfLCL->lpSurfMore->dwSurfaceHandle;
        pRc = g_pContexts;

        while (pRc)
        {
          // first check the RC for a valid pHndlList, a valid ppTxtrHndlList in the HNDLLIST
          // and that the txtrHndl count is within the allocated range for this RC
          if ((NULL != pRc->pHndlList) && (NULL != pRc->pHndlList->ppTxtrHndlList) &&
              ((DWORD)pRc->pHndlList->ppTxtrHndlList[0] >= hSurf) && (0 != hSurf) )
          {
            // if this RC has a TXTRHNDL for this handle and the surfLcl is this surface
            // then we need to check if we need to clear the texture handles in the
            // render state or texture stage states
            if (TXTRHNDL_PTR(hSurf) && (hSurf == surfLCL->lpSurfMore->dwSurfaceHandle))
            {
#if 0
// Fix for PRS 14268, D3D Globe restores it's surfaces after the mode change but it doesn't
// bother telling us the texture handle to use again so don't clear the texture handle(s)
// stored in the D3D context or texture stages here, just clear the TXTRHNDL and then we'll
// be relying on our TXTRHNDL error handling in ValidateTextureStageState and DrawPrimitives2
// to skip the use of these handles if they haven't been restored.
              if (hSurf == pRc->texture)
                pRc->texture = 0;
              if (hSurf == TS[0].textureHandle)
                TS[0].textureHandle = 0;
              if (hSurf == TS[1].textureHandle)
                TS[1].textureHandle = 0;
              // V3 uses up to three texture stages, but napalm only uses two
              if (! IS_NAPALM)
              {
                if (hSurf == TS[2].textureHandle)
                  TS[2].textureHandle = 0;
              }
#endif

              // clear the TXTRHNDL
              memset(TXTRHNDL_PTR(hSurf), 0, sizeof(TXTRHNDL));

              // this assumes a surface can only belong to one RC at a time
              // if this turns out not to be the case, then we need to remove
              // this break statement and loop over all the RC's
              break;
            }
          }
          pRc = pRc->pNext;
        }
      }
#endif
   }
}

//----------------------------------------
//
//
//
//----------------------------------------
/*-------------------------------------------------------------------
Function Name:  TXTRMIPMAPALLOC
Description:    Allocate memory for texture from a heap
Information:    long TXTRMIPMAPALLOC(NT9XDEVICEDATA *ppdev, TXTRDESC  *txtr, DWORD tmu, long memRequired, long lodmin, long lodmax, long lodminAll, long lodmaxAll, long lodmask, long aspectRatioMask,
                     unsigned long aspectRatio, unsigned long s_wider_t, long bitsPerTexel, LPDDRAWI_DDRAWSURFACE_LCL surfLCL, unsigned long flags,
                     unsigned long *pitch, unsigned long *tileFlag)
Return:         long - host lfb start address of allocation

-------------------------------------------------------------------*/
long TXTRMIPMAPALLOC(NT9XDEVICEDATA *ppdev, TXTRDESC  *txtr, DWORD tmu,
                     long memRequired, long lodmin, long lodmax, long lodminAll,
                     long lodmaxAll, long lodmask, long aspectRatioMask,
                     unsigned long aspectRatio, unsigned long s_wider_t,
                     long bitsPerTexel, LPDDRAWI_DDRAWSURFACE_LCL surfLCL,
                     unsigned long flags, unsigned long *pitch, unsigned long *tileFlag)
{
   long            lfbstart = 0;
   unsigned long   heapIndex = 0;
#ifdef WINNT
   VIDEOMEMORY *pvmHeap;
#endif

   long hwStart;

   memMgr_allocSurface(
#ifdef WINNT
                        ppdev,
#else
                        surfLCL->lpGbl->lpDD,          // direct draw vidmemalloc need this
#endif
                        surfLCL->ddsCaps.dwCaps,       // type of surface (can use the standard DD surface flags) [IN]
                        memRequired,                   // width in linear space [IN]
                        1,                             // height in linear space [IN]
                        0,                             // width in tiled space [IN]
                        0,                             // width in tiled space [IN]
                        &lfbstart,                     // host lfb start address of allocation [OUT]
                        &hwStart,                      // hw start address
                        pitch,                         // surface pitch
                        tileFlag,                      // MEM_IN_TILE or MEM_IN_LINEAR [OUT]
                        &txtr->heapID[tmu]             // Ddraw heap ID[OUT]
#ifdef WINNT
                        , &pvmHeap
#endif
                        );


   if (lfbstart != 0)
   {
      if(surfLCL->lpGbl->dwReserved1 != (DWORD)NULL)
      {
         FXSURFACEDATA *surfaceData;
         surfaceData = (FXSURFACEDATA *)surfLCL->lpGbl->dwReserved1;

         surfaceData->lfbPtr = lfbstart;
         surfaceData->hwPtr  = hwStart;
         surfaceData->lPitch = *pitch;
#ifdef WINNT
         surfaceData->pvmHeap = pvmHeap;
#else
         surfaceData->heapID = txtr->heapID[tmu];
#endif
      }
#ifdef WINNT
      txtr->pvmHeap = pvmHeap;
#endif

      txtr->tmuID[tmu] = tmu;
      txtr->start[tmu] = lfbstart - _FX(textureHeapStart[tmu]);
      txtr->mipmapsOnTmu[tmu] = lodmask;     // even LOD levels are on TMU 0
      txtr->textureLength[tmu] = memRequired;

      if (lodmask == LODMASK_ALL)
      {
         if (tmu == TREX0)
            txtr->home = txtr->noMipMapsHome = HOME_TREX0;
         else
            txtr->home = txtr->noMipMapsHome = HOME_TREX1;
      }
      else if (lodmask == LODMASK_ODD)
      {
         if (tmu == TREX0)
         {
            txtr->home = HOME_SPLIT_ODD_TMU0;
            txtr->noMipMapsHome = (lodminAll & 1) ? HOME_TREX0 : HOME_TREX1;
         }
         else
         {
            txtr->home = HOME_SPLIT_ODD_TMU1;
            txtr->noMipMapsHome = (lodminAll & 1) ? HOME_TREX1 : HOME_TREX0;
         }
      }
      else if (lodmask == LODMASK_EVEN)
      {
         if (tmu == TREX0)
         {
            txtr->home = HOME_SPLIT_ODD_TMU1;
            txtr->noMipMapsHome = (lodminAll & 1) ? HOME_TREX1 : HOME_TREX0;
         }
         else
         {
            txtr->home = HOME_SPLIT_ODD_TMU0;
            txtr->noMipMapsHome = (lodminAll & 1) ? HOME_TREX0 : HOME_TREX1;
         }
      }

      if ((_D3(autoMipMap) == 0) || !(flags & AutoMipMaps))
      {
         long base_addr;
         if (bitsPerTexel > 4)
            base_addr = txtrCalcBaseAddress(txtr->start[tmu], lodmin, aspectRatio, bitsPerTexel, txtr->home,txtr->flags & TextureIsBig);
         else
            base_addr = txtrCalcBaseAddressDXT1(tmu, lodmin, txtr);

         // This should only ever happen on H5!
         if (base_addr & BIT(25))
         {
            base_addr &= 0x1fffff0;
            base_addr |= BIT(1);
         }
         txtr->baseAddr[tmu]=base_addr;

         txtr->noMipMapsBaseAddr[tmu]=txtrCalcBaseAddress(txtr->start[tmu], lodmin, aspectRatio, bitsPerTexel, txtr->noMipMapsHome,txtr->flags & TextureIsBig) ;
         txtr->tLOD[tmu]  = (s_wider_t)
                           | (aspectRatioMask)
                           | ((lodmaxAll << 2) << SST_LODMAX_SHIFT)
                           |  (lodminAll << 2) ;

         if (txtr->flags & TextureIsBig)
            txtr->tLOD[tmu] |= SST_TBIG;

         txtr->tLODnoMipMaps[tmu]  = (s_wider_t)
                                    | (aspectRatioMask)
                                    | ((lodminAll << 2) << SST_LODMAX_SHIFT)
                                    |  (lodminAll << 2) ;

         if (txtr->flags & TextureIsBig)
            txtr->tLODnoMipMaps[tmu] |= SST_TBIG;

      }

      // automipmap from lodMin to 0
      else if (_D3(autoMipMap) & AM_TRUE)
      {
         long base_addr;
         if (bitsPerTexel > 4)
            base_addr = txtrCalcBaseAddress(txtr->start[tmu], lodmin, aspectRatio, bitsPerTexel, txtr->home,txtr->flags & TextureIsBig);
         else
            base_addr = txtrCalcBaseAddressDXT1(tmu, lodmin, txtr);

         // This should only ever happen on H5!
         if (base_addr & BIT(25))
         {
            base_addr &= 0x1fffff0;
            base_addr |= BIT(1);
         }

         txtr->baseAddr[tmu]=base_addr;

         txtr->noMipMapsBaseAddr[tmu]=txtrCalcBaseAddress(txtr->start[tmu], lodmin, aspectRatio, bitsPerTexel, txtr->noMipMapsHome,txtr->flags & TextureIsBig) ;
         txtr->tLOD[tmu]  = (s_wider_t)
                           | (aspectRatioMask)
                           | ((LOD_32 << 2) << SST_LODMAX_SHIFT)
                           |  (lodminAll << 2) ;

         if (txtr->flags & TextureIsBig)
            txtr->tLOD[tmu] |= SST_TBIG;

         txtr->tLODnoMipMaps[tmu]  = (s_wider_t)
                                    | (aspectRatioMask)
                                    | ((lodminAll << 2) << SST_LODMAX_SHIFT)
                                    |  (lodminAll << 2) ;

         if (txtr->flags & TextureIsBig)
            txtr->tLODnoMipMaps[tmu] |= SST_TBIG;
      }
      // automipmap from lodMin + 1 to 8
      else
      {
         // baseaddr/tLOD is for LODMIN - 1  whereas nomipmaps is for LODMIN
         long base_addr;
         if (bitsPerTexel > 4)
            base_addr = txtrCalcBaseAddress(txtr->start[tmu], lodmin, aspectRatio, bitsPerTexel, txtr->home,txtr->flags & TextureIsBig);
         else
            base_addr = txtrCalcBaseAddressDXT1(tmu, lodmin, txtr);

         // This should only ever happen on H5!
         if (base_addr & BIT(25))
         {
            base_addr &= 0x1fffff0;
            base_addr |= BIT(1);
         }
         txtr->baseAddr[tmu]=base_addr;
         txtr->noMipMapsBaseAddr[tmu]=txtrCalcBaseAddress(txtr->start[tmu], lodmin, aspectRatio, bitsPerTexel, txtr->noMipMapsHome,txtr->flags & TextureIsBig) ;
         txtr->tLOD[tmu]  =   (s_wider_t)
                              | (aspectRatioMask)
                              | ((LOD_32 << 2) << SST_LODMAX_SHIFT)
                              |  ((lodminAll + 1) << 2) ;

         if (txtr->flags & TextureIsBig)
            txtr->tLOD[tmu] |= SST_TBIG;

         // leave largest mipmap to original size because need to download largest LOD
         txtr->tLODnoMipMaps[tmu]  = (s_wider_t)
                                    | (aspectRatioMask)
                                    | ((lodminAll << 2) << SST_LODMAX_SHIFT)
                                    | ((lodminAll) << 2) ;
         if (txtr->flags & TextureIsBig)
            txtr->tLODnoMipMaps[tmu] |= SST_TBIG;
      }
   }
   return lfbstart;
}

//-----------------------------------------------------------------------------------------------
//
// TextureComputeVaddr - used to
//
//-----------------------------------------------------------------------------------------------
/*-------------------------------------------------------------------
Function Name:  txtrComputeVAddr
Description:    Computes the virtual address for any mipmap within a mipmap chain.
Information:    DWORD txtrComputeVAddr( DWORD maxST, TXTRDESC *txtr, LPDWORD vaddr0, LPDWORD vaddr1)
Return:         DWORD the virtual address for the mipmap within the chain.



-------------------------------------------------------------------*/
DWORD txtrComputeVAddr( DWORD maxST, TXTRDESC *txtr, LPDWORD vaddr0, LPDWORD vaddr1)
{
   // increment past first mipmap (whichever the trex)
   DWORD VirtAddr;
   DWORD bytesPerTexel = txtr->bytesPerTexel;
   DWORD aspectRatio   = txtr->aspectRatio;

   if (txtr->mipmapsOnTmu[TREX0] & (1<<maxST))
   {
      VirtAddr = *vaddr0;
      if (((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_DXT1 | SST_COMPRESSED_TEXTURES)) ||
          ((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_FXT1 | SST_COMPRESSED_TEXTURES)) ||
          ((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_DXT2 | SST_COMPRESSED_TEXTURES)) ||
          ((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_DXT3 | SST_COMPRESSED_TEXTURES)) ||
          ((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_DXT4 | SST_COMPRESSED_TEXTURES)) ||
          ((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_DXT5 | SST_COMPRESSED_TEXTURES)))
      {
         *vaddr0 += txtrComputeCompSize(maxST,txtr);
      }
      else
      {
         if (txtr->flags & TextureIsBig)
            *vaddr0 += (txtBigMipMapSize[aspectRatio][LOG2LODTBIG(maxST)] * bytesPerTexel);
         else
            *vaddr0 += (txtMipMapSize[aspectRatio][LOG2LOD(maxST)] * bytesPerTexel);
      }
   }
   else
   {
      VirtAddr = *vaddr1;
      if (((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_DXT1 | SST_COMPRESSED_TEXTURES)) ||
          ((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_FXT1 | SST_COMPRESSED_TEXTURES)) ||
          ((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_DXT2 | SST_COMPRESSED_TEXTURES)) ||
          ((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_DXT3 | SST_COMPRESSED_TEXTURES)) ||
          ((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_DXT4 | SST_COMPRESSED_TEXTURES)) ||
          ((txtr->format & (SST_TFORMAT | SST_COMPRESSED_TEXTURES)) == (SST_DXT5 | SST_COMPRESSED_TEXTURES)))
      {
         *vaddr1 += txtrComputeCompSize(maxST,txtr);
      }
      else
      {
         if (txtr->flags & TextureIsBig)
            *vaddr1 += (txtBigMipMapSize[aspectRatio][LOG2LODTBIG(maxST)] * bytesPerTexel);
         else
            *vaddr1 += (txtMipMapSize[aspectRatio][LOG2LOD(maxST)] * bytesPerTexel);
      }
   }

   return(VirtAddr);
}

/*-------------------------------------------------------------------
Function Name:  TEXTURESURFACECREATE
Description:    Creates a texture surface.  Called by Direct Draw.
Information:    DWORD __stdcall TEXTURESURFACECREATE(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWSURFACE_LCL surfLCL, int textureStage, HRESULT *retCode )

Return:         DWORD DDHAL_DRIVER_HANDLED
                *retCode =     DDERR_OUTOFVIDEOMEMORY
                            or DDERR_UNSUPPORTEDFORMAT
                            or DDERR_TOOBIGWIDTH
                            or DDERR_TOOBIGHEIGHT
                            or DD_OK
-------------------------------------------------------------------*/
DWORD __stdcall TEXTURESURFACECREATE(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWSURFACE_LCL surfLCL, int textureStage, HRESULT *retCode )
{
   LPDDRAWI_DDRAWSURFACE_GBL surfGCL;
   long           slog = 0;
   long           tlog = 0;
   long           lodmin = 0;
   long           lodmax = 0;
   float          scaleS, scaleT;
   long           s_wider_t = 0;
   long           bitsPerTexel, memRequired ;
   long           aspectRatio, aspectRatioMask, format, formatFlags;
   long           flags, initialSlog, initialTlog ;
   TXTRDESC       *txtr;
   DWORD          numMipmaps, virtualAddr ;
   signed int     txtrID;
   DWORD          heapIndex = 0;
   long           start;
   unsigned long  pitch;
   unsigned long  tileFlag;
   unsigned long  llodmask;

   //----------------------------------------------------------------------
   // Find a block of texture memory that will fit this texture or mipmap.
   // Since we have computed the characteristics of this texture then we
   // might as well save the information in order to download it and use it.
   //----------------------------------------------------------------------
   flags        = initialSlog = initialTlog = 0 ;
   surfGCL      = surfLCL->lpGbl ;

#if 0//defined( NULLDRIVER )

   {
      LPDDRAWI_DDRAWSURFACE_LCL next;
      next = surfLCL;
      next->lpGbl->fpVidMem = 0x12000000 ;

      do
      {
         if (next->lpAttachList != NULL)
         {
            next = next->lpAttachList->lpAttached ;
            next->lpGbl->fpVidMem =  0x12000000;
            next->dwReserved1 = 1;
         }
         else
            next = NULL;

      } while (next != NULL);
   }

   *retCode = DD_OK;
   surfLCL->dwReserved1 = 1;
   return (DDHAL_DRIVER_HANDLED);
#endif

   if ((! IS_NAPALM) && (TEXTURE_IS_DXT_SURFACE(surfGCL->ddpfSurface)
                         || TEXTURE_IS_FXT_SURFACE(surfGCL->ddpfSurface)))
   {
       D3DPRINT(0, "TEXTURESURFACECREATE: !Napalm and asking for DXT or FXT format. Unsupported.");
       *retCode = DDERR_UNSUPPORTEDFORMAT;
       return (DDHAL_DRIVER_HANDLED);
   }

   // Allocate the surface data structure always! - CGW
   {
      FXSURFACEDATA *surfaceData = (FXSURFACEDATA *)NULL;

      surfaceData = (FXSURFACEDATA *)D3DMALLOCZ(sizeof(FXSURFACEDATA), 0);
      if(!surfaceData)
      {
        D3DPRINT(0, "TEXTURESURFACECREATE: Insufficient memory to allocate FXSURFACEDATA.");
        *retCode = DDERR_OUTOFVIDEOMEMORY;
        return DDHAL_DRIVER_HANDLED;
      }
      surfGCL->dwReserved1 = (DWORD)surfaceData;
      UPDATE_BLOCK_DATA(surfaceData, &surfGCL->dwReserved1);
   }

   // by default we have one mipmap
   if (surfLCL->lpSurfMore->dwMipMapCount)
      numMipmaps = surfLCL->lpSurfMore->dwMipMapCount;
   else
      numMipmaps = 1;

   D3DPRINT( 255, "TextureSurfaceCreate: surfLCL=0x%x #mipmaps=%d w=%d h=%d",
            surfLCL, numMipmaps, surfGCL->wWidth, surfGCL->wHeight);

#ifndef WINNT
   // Don't do anything for textures in system memory
   if (surfGCL->dwGlobalFlags & DDRAWISURFGBL_SYSMEMREQUESTED)
      return (DDHAL_DRIVER_NOTHANDLED);
      // HW does not support fourcc textures. Games will use this
      // to verify if this is a PowerVR card and do some screwy stuff.
   else
#endif
   if ((surfGCL->ddpfSurface.dwFlags & DDPF_FOURCC)
      && !(TEXTURE_IS_DXT_SURFACE(surfGCL->ddpfSurface)
           || TEXTURE_IS_FXT_SURFACE(surfGCL->ddpfSurface)))
   {
#if defined(WINNT)

       D3DFREE(surfGCL->dwReserved1);
       surfGCL->dwReserved1 = 0;
#endif
       D3DPRINT(0, "TEXTURESURFACECREATE: FourCC and !(DXT or FXT). Unsupported.");
       *retCode = DDERR_UNSUPPORTEDFORMAT;
       return (DDHAL_DRIVER_HANDLED);
   }
   else
   {

      if (TEXTURE_IS_DXT_SURFACE(surfGCL->ddpfSurface) || TEXTURE_IS_FXT_SURFACE(surfGCL->ddpfSurface))
      {
         if ((surfGCL->wWidth & 0x3) ||
             (surfGCL->wHeight & 0x3))
         {
#if defined(WINNT)
            D3DFREE(surfGCL->dwReserved1);
            surfGCL->dwReserved1 = 0;
#endif
            D3DPRINT(0,"TEXTURESURFACECREATE: DXT surface not a multiple of 4 texels width by height");
            *retCode = DDERR_HEIGHTALIGN;
            return(DDHAL_DRIVER_HANDLED);
         }
      }
      // Banshee and Voodoo3 do not support textures larger than 256x256.
      // Napalm does not support textures larger than 2048x2048.
      if (surfGCL->wWidth > MAX_TEXTURE_SIZE_U)
      {
         if (IS_NAPALM && (surfGCL->wWidth <= MAX_BIGTEXTURE_SIZE_U))
         {
            flags |= TextureIsBig;
         }
         else
         {
#if defined(WINNT)

            D3DFREE(surfGCL->dwReserved1);
            surfGCL->dwReserved1 = 0;
#endif
            D3DPRINT(0, "TEXTURESURFACECREATE: Texture is too Wide. Unsupported.");
            *retCode = DDERR_TOOBIGWIDTH;
            surfLCL->dwReserved1 = 0;
            surfGCL->fpVidMem = (DWORD) NULL;
            return    (DDHAL_DRIVER_HANDLED);
         }

      }

      if (surfGCL->wHeight > MAX_TEXTURE_SIZE_V)
      {
         if (IS_NAPALM && surfGCL->wHeight <= MAX_BIGTEXTURE_SIZE_V)
         {
            flags |= TextureIsBig;
         }
         else
         {
#if defined(WINNT)
            D3DFREE(surfGCL->dwReserved1);
            surfGCL->dwReserved1 = 0;
#endif
            D3DPRINT(0, "TEXTURESURFACECREATE: Texture is too Tall. Unsupported.");
            *retCode = DDERR_TOOBIGHEIGHT;
            surfLCL->dwReserved1 = 0;
            surfGCL->fpVidMem = (DWORD) NULL;
            return    (DDHAL_DRIVER_HANDLED);
         }
      }

     // Ensure that the texture height and width are powers of 2.
     //  Even though we have set D3DPTEXTURECAPS_POW2 some idiot
     //  may call us with invalid parameters.
     if ( ( (surfGCL->wWidth  & ~(surfGCL->wWidth - 1)) != surfGCL->wWidth) ||
        ( (surfGCL->wHeight & ~(surfGCL->wHeight - 1)) != surfGCL->wHeight) )
     {
#if defined(WINNT)
         D3DFREE(surfGCL->dwReserved1);
         surfGCL->dwReserved1 = 0;
#endif
         *retCode = DDERR_INVALIDPARAMS;
         return    (DDHAL_DRIVER_HANDLED);
     } 
    
      // compute the log2 for width and height
      while (((surfGCL->wWidth - (0x01 << slog)) != 0)
            &&  (slog < MAX_BIGTEXTURE_LOG) )
         ++slog ;

      while (((surfGCL->wHeight - (0x01 << tlog)) != 0)
            &&  (tlog < MAX_BIGTEXTURE_LOG) )
         ++tlog ;


      // This is a workaround for ALT-TAB while running Redline racer. Either the game or DD are
      // performing a completely bogus operation. Normally,
      // a mipmap chain is created once but in this case this routine is called one time for each level but we
      // can't create them separately so we recognize it has already been created and just fill in the
      // virtual address which is needed for the download.
      if( (surfLCL->ddsCaps.dwCaps & DDSCAPS_MIPMAP) &&
         (surfLCL->dwReserved1 != 0) )
      {
         // This surface is part of a MipMap that has already
         // been restored, but we need to restore fpVidMem because
         // DD zero's it out.

         DWORD    vaddr0, vaddr1;
         int      maxST, maxSTOrig;
         int      STcount;
         TXTRDESC *miptxtr;

         miptxtr = (TXTRDESC *) TXTRDESC_PTR(surfLCL->dwReserved1);

         vaddr0 = miptxtr->start[TREX0] + _FX(textureHeapStart[TREX0]);
         vaddr1 = miptxtr->start[TREX1] + _FX(textureHeapStart[TREX1]);

         maxST     = max(slog,tlog);
         maxSTOrig = max(miptxtr->slog, miptxtr->tlog);

         for(STcount = maxSTOrig; STcount >= maxST; STcount--)
         {
            surfGCL->fpVidMem = txtrComputeVAddr( (DWORD)STcount, miptxtr, &vaddr0, &vaddr1);
#ifdef DEFER_TXTR_DNLD
            miptxtr->checksum[LOG2LOD(STcount)] = 0;
            miptxtr->reload |= (1 << LOG2LOD(STcount));
#endif // DEFER_TXTR_DNLD
         }

         *retCode = DD_OK;
         return(DDHAL_DRIVER_HANDLED);
      }

      // compute the aspect ratio so it is always greater than 1 because
      // the hardware supports both 256 * X and X * 256 which have the same
      // lod level.
      if (surfGCL->wWidth > surfGCL->wHeight)
      {
         // Our hardware only supports aspect ratios up to 8:1 so we will
         // have to create a texture with an aspect ratio of 8:1 or 1:8
         if ((slog - tlog) > 3)
         {
#if 0
            if (TEXTURE_IS_DXT_SURFACE(surfGCL->ddpfSurface))
            {
#if defined(WINNT)
               D3DFREE(surfGCL->dwReserved1);
               surfGCL->dwReserved1 = 0;
#endif
               D3DPRINT( 0, "TEXTURESURFACECREATE: AllocTexture(1): DXTn aspect ratio is larger than 8:1");
               *retCode = DDERR_UNSUPPORTED;
               return (DDHAL_DRIVER_HANDLED);
            }
#endif
            D3DPRINT( 255, "AllocTexture: aspect ratio is larger than 8:1 width=%d height=%d",
            surfGCL->wWidth, surfGCL->wHeight );
            // save the initial dimensions
            flags |= AspectRatioGT8 ;
            initialSlog = slog ;
            initialTlog = tlog ;

            // (width / 8) to find the size of texture to have an 8:1 ratio
            tlog = slog - 3 ;
         }

         s_wider_t   = SST_LOD_S_IS_WIDER ;
         aspectRatio = slog - tlog ;        // 2 ** X of largest mipmap

         // lodmin is the smallest mipmap level ie the largest mipmap size
         if (flags & TextureIsBig)
            lodmin = LOG2LODTBIG(slog);
         else
            lodmin = LOG2LOD(slog) ;

         // numTextures > 1 if there is mipmapping.
         numMipmaps == 1 ? (lodmax = lodmin) : (lodmax = lodmin + numMipmaps - 1);

         //
         // Rectangular Textures:
         //    TREX uses a virtual coordinate system to map from texture space
         //    to screen space. For square textures, the texture space is from
         //    256x256. For rectangular textures, the texture space scale
         //    accordingly. So a texture with ratio 2:1 has a texture space of
         //    256x128. So if D3D always assume the virtual coordinates of
         //    1x1 then scale the parameters.  D3D texture coordinates are normalized.
         //
         //    NOTE: application specifies 1.0 that is in essence the right side
         //          of the last texel which is the start of the next texel. So,
         //          1.0 produces the same texel as 0.0.
#if 0
         if (flags & TextureIsBig)
         {
            scaleS = (float)MAX_BIGTEXTURE_SIZE_U;              // larger side
            scaleT = (float)MAX_BIGTEXTURE_SIZE_V / (float)(1 << aspectRatio);
         }
         else
#endif
         {
            scaleS = (float)MAX_TEXTURE_SIZE_U;              // larger side
            scaleT = (float)MAX_TEXTURE_SIZE_V / (float)(1 << aspectRatio);
         }
      }

      else
      {
         // Our hardware only supports aspect ratios up to 8:1 so we will
         // have to create a texture with an aspect ratio of 8:1
         if ((tlog - slog) > 3)
         {
#if 0
            if (TEXTURE_IS_DXT_SURFACE(surfGCL->ddpfSurface))
            {
#if defined(WINNT)
               D3DFREE(surfGCL->dwReserved1);
               surfGCL->dwReserved1 = 0;
#endif
               D3DPRINT( 0, "TEXTURESURFACECREATE: AllocTexture(2): DXTn aspect ratio is larger than 8:1");
               *retCode = DDERR_UNSUPPORTED;
               return (DDHAL_DRIVER_HANDLED);
            }
#endif
            D3DPRINT(255, "AllocTexture: aspect ratio is larger than 8:1");
            // save the initial dimensions
            flags |= AspectRatioGT8 ;
            initialSlog = slog ;
            initialTlog = tlog ;
            // (height / 8) to find the size of texture to have an 8:1 ratio
            slog = tlog - 3 ;
         }

         aspectRatio = tlog - slog;
         if (flags & TextureIsBig)
            lodmin = LOG2LODTBIG(tlog);
         else
            lodmin = LOG2LOD(tlog);

         numMipmaps == 1 ? (lodmax = lodmin) : (lodmax = lodmin + numMipmaps - 1);

#if 0
         if (flags & TextureIsBig)
         {
            scaleT = (float)MAX_BIGTEXTURE_SIZE_V;
            if (aspectRatio == 0)
               scaleS = (float)MAX_BIGTEXTURE_SIZE_U;
            else
               scaleS = (float)MAX_BIGTEXTURE_SIZE_U / (float)(1 << aspectRatio);
         }
         else
#endif
         {
            scaleT = (float)MAX_TEXTURE_SIZE_V;
            if (aspectRatio == 0)
               scaleS = (float)MAX_TEXTURE_SIZE_U;
            else
               scaleS = (float)MAX_TEXTURE_SIZE_U / (float)(1 << aspectRatio);
         }
      }

      D3DPRINT( 255, "lodmax = %d lodmin = %d",lodmax,lodmin );

      // need to set the aspect ratio in a hardware register so create a mask
      aspectRatioMask = aspectRatio << SST_LOD_ASPECT_SHIFT;

      // texel format
      if (TEXTURE_IS_DXT_SURFACE(surfGCL->ddpfSurface))
      {
         formatFlags = TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;

         switch(surfGCL->ddpfSurface.dwFourCC)
         {
            case FOURCC_DXT1:
               format = SST_DXT1 | SST_COMPRESSED_TEXTURES;
               bitsPerTexel = 4;
               break;
            case FOURCC_DXT2:
               format = SST_DXT2 | SST_COMPRESSED_TEXTURES;
               bitsPerTexel = 8;
               break;
            case FOURCC_DXT3:
               format = SST_DXT3 | SST_COMPRESSED_TEXTURES;
               bitsPerTexel = 8;
               break;
            case FOURCC_DXT4:
               format = SST_DXT4 | SST_COMPRESSED_TEXTURES;
               bitsPerTexel = 8;
               break;
            case FOURCC_DXT5:
               format = SST_DXT5 | SST_COMPRESSED_TEXTURES;
               bitsPerTexel = 8;
               break;
            default:
               D3DPRINT( 0,"TEXTURESURFACECREATE: Error unknown fourcc format %08x",
                                                                        surfGCL->ddpfSurface.dwFourCC);
#if defined(WINNT)
               D3DFREE(surfGCL->dwReserved1);
               surfGCL->dwReserved1 = 0;
#endif
               *retCode = DDERR_INVALIDPIXELFORMAT;
               return (DDHAL_DRIVER_HANDLED);
         }
      }
      else if (TEXTURE_IS_FXT_SURFACE(surfGCL->ddpfSurface))
      {
         formatFlags = TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;
         format = SST_FXT1 | SST_COMPRESSED_TEXTURES;
         bitsPerTexel = 4;
      }
      else if (surfLCL->dwFlags & DDRAWISURF_HASPIXELFORMAT)
      {
#if ( DX >= 6 )
         if (surfGCL->ddpfSurface.dwFlags & DDPF_PALETTEINDEXED8)
         {
            if( surfGCL->ddpfSurface.dwFlags & DDPF_ALPHAPIXELS )
            {
               D3DPRINT( 255, "Creating an 8bit ALPHA+PALETTIZED texture surface,");
               format = TEXFMT_ALPHA_P8_RGB << SST_TFORMAT_SHIFT;
               formatFlags = TEXFMTFLG_PALETTIZED | TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;
               bitsPerTexel = 16;
            }
            else
            {
               D3DPRINT( 255, "Creating an 8bit PALETTIZED texture surface,");
               format = TEXFMT_P8_RGB << SST_TFORMAT_SHIFT;
               formatFlags = TEXFMTFLG_PALETTIZED | TEXFMTFLG_RGB;
               bitsPerTexel = 8;
            }
         }
         // Intensity textures (Luminance)
         else if( surfGCL->ddpfSurface.dwFlags & DDPF_LUMINANCE )
         {
            if( surfGCL->ddpfSurface.dwFlags & DDPF_ALPHAPIXELS )
            {
               if( 16 == surfGCL->ddpfSurface.dwLuminanceBitCount )
               {
                  // Texture mode 16bit - alpha(8), intensity(8)
                  D3DPRINT( 255, "Creating an 16bit ALPHA+LUMINANCE texture surface,");
                  format = TEXFMT_ALPHA_INTENSITY_88 << SST_TFORMAT_SHIFT;
                  formatFlags = TEXFMTFLG_INTENSITY | TEXFMTFLG_ALPHA;
                  bitsPerTexel = 16;
               }
               else
               {
                  // Texture mode 8bit - alpha(4), intensity(4)
                  D3DPRINT( 255, "Creating an 8bit ALPHA+LUMINANCE texture surface,");
                  format = TEXFMT_ALPHA_INTENSITY_44 << SST_TFORMAT_SHIFT;
                  formatFlags = TEXFMTFLG_INTENSITY | TEXFMTFLG_ALPHA;
                  bitsPerTexel = 8;
               }
            }
            else
            {
               // Texture mode 8bit - intensity(8)
               D3DPRINT( 255, "Creating an 8bit LUMINANCE texture surface,");
               format = TEXFMT_INTENSITY_8 << SST_TFORMAT_SHIFT;
               formatFlags = TEXFMTFLG_INTENSITY;
               bitsPerTexel = 8;
            }
         }
         else if( surfGCL->ddpfSurface.dwFlags & DDPF_ALPHAPIXELS
                  && surfGCL->ddpfSurface.dwRBitMask == 0)
         {
            D3DPRINT(255, "Creating an 8bit ALPHA only texture surface,");
            format = TEXFMT_ALPHA_8 << SST_TFORMAT_SHIFT;
            formatFlags = TEXFMTFLG_ALPHA;
            bitsPerTexel = 8;

         }
#else
         // palettized texture (we only support an 8 bit)
         if (surfGCL->ddpfSurface.dwFlags & DDPF_PALETTEINDEXED8)
         {
            D3DPRINT( 255, "Creating an 8bit PALLETTIZED texture surface,");
            format = TEXFMT_P8_RGB << SST_TFORMAT_SHIFT;
            formatFlags = TEXFMTFLG_PALETTIZED | TEXFMTFLG_RGB;
            bitsPerTexel = 8;
         }

#endif
         // rbg texture
         else switch (surfGCL->ddpfSurface.dwRBitMask)
         {
            case RGB8888_RMASK:
               D3DPRINT( 255, "Creating a 32bit 8888 texture surface,");
               format = TEXFMT_ARGB_8888 << SST_TFORMAT_SHIFT;
               formatFlags = TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;
               bitsPerTexel = 32;
               break;

            case RGB4444_RMASK:
               D3DPRINT( 255, "Creating a 16bit 4444 texture surface,");
               format = TEXFMT_ARGB_4444 << SST_TFORMAT_SHIFT;
               formatFlags = TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;
               bitsPerTexel = 16;
               break;

            case RGB1555_RMASK:
               D3DPRINT( 255, "Creating a 16bit 1555 texture surface,");
               format = TEXFMT_ARGB_1555 << SST_TFORMAT_SHIFT;
               formatFlags = TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;
               bitsPerTexel = 16;
               break;

            case RGB565_RMASK:
               D3DPRINT( 255, "Creating a 16bit 565 texture surface,");
               format = TEXFMT_RGB_565 << SST_TFORMAT_SHIFT;
               formatFlags = TEXFMTFLG_RGB;
               bitsPerTexel = 16;
               break;

            case RGB332_RMASK:
               if( surfGCL->ddpfSurface.dwRGBAlphaBitMask == RGB8332_AMASK )
               {
                  D3DPRINT( 255, "Creating a 16bit 4444 texture surface,");
                  format = TEXFMT_ARGB_8332 << SST_TFORMAT_SHIFT;
                  formatFlags = TEXFMTFLG_RGB | TEXFMTFLG_ALPHA;
                  bitsPerTexel = 16;
               }
               else
               {
                  D3DPRINT( 255, "Creating a 16bit 4444 texture surface,");
                  format = TEXFMT_RGB_332 << SST_TFORMAT_SHIFT;
                  formatFlags = TEXFMTFLG_RGB;
                  bitsPerTexel = 8;
               }
               break;

            default:
               D3DPRINT( 255, "No texture format - Creating a 16bit 565 texture surface" );
               format = TEXFMT_RGB_565 << SST_TFORMAT_SHIFT;
               formatFlags = TEXFMTFLG_RGB;
               bitsPerTexel = 16;
               break;
         }
      }
      else
      {
         // this is not exactly a clean design but direct draw will not
         // set the pixelformat information if the format is identical to
         // the primary surface so setup for the same as the primary surface.

         bitsPerTexel = (DWORD) GETPRIMARYBYTEDEPTH << 3L; // primary surface
         formatFlags = TEXFMTFLG_RGB;
         if (bitsPerTexel == 32)
         {
            D3DPRINT( 255, "No texture format - Creating a 32bit 8888 texture surface,");
            format = TEXFMT_ARGB_8888 << SST_TFORMAT_SHIFT;
            formatFlags |= TEXFMTFLG_ALPHA;
         }
         else
         {
            D3DPRINT( 255, "No texture format - Creating a 16bit 565 texture surface" );
            format = TEXFMT_RGB_565 << SST_TFORMAT_SHIFT;
         }
      }

      D3DPRINT( 6,"  slog=%d tlog=%d lodmin=%d lodmax=%d ar=%d        bpt=%d        format=%x",
                             slog,   tlog,   lodmin,   lodmax,   aspectRatio, bitsPerTexel, format );

      txtrFlavour(format);

      // automatically generate mipmaps for all textures EXCEPT
      //  palletized, aspect ratio > 8, 1xN, Nx1, compressed
      // ** NOTE ** not sure if this 'if' needs to be changed for Napalm. (Bob S.)
      if  (   _D3(autoMipMap) && !(formatFlags & TEXFMTFLG_PALETTIZED)
         && !(flags & AspectRatioGT8) && (numMipmaps == 1)
         && (slog > 2) && !(format & SST_COMPRESSED_TEXTURES)
         )
      {
         if ((flags & TextureIsBig && (lodmin < TBIG_LOD_32)) ||
            (!(flags & TextureIsBig) && (lodmin < LOD_32)))
         {
            flags |= AutoMipMaps;
            lodmax = LOD_32;
            numMipmaps = lodmax - lodmin + 1;
         }
      }

      // compute amount of memory we need for this mipmap
      if ((format == (SST_COMPRESSED_TEXTURES | SST_DXT1)) ||
          (format == (SST_COMPRESSED_TEXTURES | SST_FXT1)) ||
          (format == (SST_COMPRESSED_TEXTURES | SST_DXT2)) ||
          (format == (SST_COMPRESSED_TEXTURES | SST_DXT3)) ||
          (format == (SST_COMPRESSED_TEXTURES | SST_DXT4)) ||
          (format == (SST_COMPRESSED_TEXTURES | SST_DXT5)))
      {
         memRequired = txtrCalcMemRequiredDXT1( lodmin, lodmax, aspectRatio, bitsPerTexel, LODMASK_ALL, slog, tlog,(flags & TextureIsBig));
      }
      else
      {
         memRequired = txtrCalcMemRequired( lodmin, lodmax, aspectRatio, bitsPerTexel, LODMASK_ALL,(flags & TextureIsBig));
      }

      // ok, allocate a texture handle for this puppy.
      txtrID = ALLOCTEXTUREDESC(ppdev, TXTRDESC_PTR(0));
      if (txtrID < 0)
      {
         // MIRIAM: hmmm something fishy here. there is no good return code defined
         D3DPRINT( 0, "TEXTURESURFACECREATE: Warning: out of texture memory handles" );

#if defined(WINNT)
         D3DFREE(surfGCL->dwReserved1);
         surfGCL->dwReserved1 = 0;
#endif
         surfLCL->lpGbl->fpVidMem = (DWORD) NULL;
         *retCode = DDERR_OUTOFVIDEOMEMORY;
         return (DDHAL_DRIVER_HANDLED);
      }
      txtr = TXTRDESC_PTR(txtrID) ;

      for (heapIndex=0; heapIndex < NUM_AVAILABLE_TMUS; ++heapIndex)
         txtr->mipmapsOnTmu[heapIndex] = 0;

      {  // These fields are now used by TXTRMIPMAPALLOC.
         txtr->slog   = slog;
         txtr->tlog   = tlog;
         txtr->flags &= ~(AspectRatioGT8) ;
         txtr->flags |= (flags | VideoTexture);
         txtr->flags |= (flags & TextureIsBig);
      }
      // try to allocate from texture unit
      if (flags & TextureIsBig)
         llodmask = TBIG_LODMASK_ALL;
      else
         llodmask = LODMASK_ALL;
      start = TXTRMIPMAPALLOC(ppdev, txtr, TREX0, memRequired, lodmin, lodmax, lodmin, lodmax,
                              llodmask,aspectRatioMask, aspectRatio, s_wider_t,
                              bitsPerTexel, surfLCL, flags, &pitch, &tileFlag);
      if (start != 0)
      {
         TXTR_HEAP_SIZE( TREX0 ) -= memRequired;
         D3DPRINT( 4, "4 - Allocating %ld at %d:0x%08lx (Free 0x%08lx)", memRequired, TREX0, start, TXTR_HEAP_SIZE( TREX0 ) );
         D3DPRINT( 255, "  1TMU: txtrDesc=%d alloc=0x%x start=0x%x size=0x%x heapID=%d tmu=%d base=0x%x heapSize=0x%x",
                   txtrID,start,txtr->start[ TREX0 ],memRequired,txtr->heapID[ TREX0 ],
                   TREX0, txtr->baseAddr[ TREX0 ],TXTR_HEAP_SIZE( TREX0 ) );

         txtr->home = txtr->noMipMapsHome = HOME_BOTH;
         txtr->tLOD[1] = txtr->tLOD[0];
         txtr->baseAddr[1] = txtr->baseAddr[0];
         txtr->tLODnoMipMaps[1] = txtr->tLODnoMipMaps[0];
         txtr->noMipMapsBaseAddr[1] = txtr->noMipMapsBaseAddr[0];
      }

      // start == 0 will return an error

      virtualAddr = start;

      // allocation of texture suceeded
      if (start != 0)
      {
//         long tlod = 0;
         txtr->slog   = slog;
         txtr->tlog   = tlog;
         txtr->scaleS = scaleS;
         txtr->scaleT = scaleT;

         if( _D3( pixelCenter ) )
         {
            txtr->centerS = textureCenter[slog];
            txtr->centerT = textureCenter[tlog];
         }
         else
         {
            txtr->centerS = 0.0f;
            txtr->centerT = 0.0f;
         }
         txtr->bytesPerTexel = (bitsPerTexel >> 3);
         txtr->aspectRatio   = aspectRatio;
         txtr->flags &= ~(AspectRatioGT8) ;
         txtr->flags |= (flags | VideoTexture);
         txtr->initialSlog = initialSlog ;
         txtr->initialTlog = initialTlog ;
         txtr->numMipmaps  = numMipmaps;

         // This texture information is information specific to this texture that
         // is not set via an individual attribute like texture perspective
         txtr->format = format;
         txtr->formatFlags = formatFlags;

         initMipMapChain(surfLCL,txtr,ppdev,slog,tlog,bitsPerTexel,txtrID);
#ifdef DEBUG
         D3DPRINT( 4, "TMU0 Free - 0x%08lx, TMU1 Free - 0x%08lx", TXTR_HEAP_SIZE(TREX0), TXTR_HEAP_SIZE(TREX1) );
         if( TXTR_HEAP_SIZE(TREX0) & 0x80000000 )
         {
            D3DPRINT( 4, "  ** TMU0 - Empty ** (length - 0x%08lx)", TXTR_HEAP_SIZE(TREX0) );
         }
         if( TXTR_HEAP_SIZE(TREX1) & 0x80000000 )
         {
            D3DPRINT( 4, "  ** TMU1 - Empty ** (length - 0x%08lx)", TXTR_HEAP_SIZE(TREX1) );
         }
#endif
      } // enough memory
      else  // out of memory
      {
         D3DPRINT( 0, "TEXTURESURFACECREATE: Warning: VidMemAlloc Failed:out of texture memory" );
         V5DLog("D3D TextureSurfaceCreate OUT-OF-TEXTURE-MEMORY memReq=%ld (texture overcommit -> DDERR_OUTOFVIDEOMEMORY; the 3DMark 16/32MB-texture path)\n", (long)memRequired);
         FREETEXTUREDESC(ppdev, txtrID);
#if defined(WINNT)
         D3DFREE(surfGCL->dwReserved1);
         surfGCL->dwReserved1 = 0;
#endif
         surfLCL->lpGbl->fpVidMem = (DWORD) NULL;
         *retCode = DDERR_OUTOFVIDEOMEMORY;
         return (DDHAL_DRIVER_HANDLED);
      }

   } // video memory

   *retCode = DD_OK;
   surfLCL->dwReserved1 = txtrID;
   return (DDHAL_DRIVER_HANDLED);

}

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
// ddiHandleSwap is not reported in DX7
#else
/*-------------------------------------------------------------------
Function Name:  ddiHandleSwap
Description:    D3D Callback for swapping texture handles.
Information:    DWORD __stdcall ddiHandleSwap(LPD3DHAL_TEXTURESWAPDATA ptsd)
Return:         DWORD DDHAL_DRIVER_HANDLED
                ptsd->ddrval =    D3DHAL_CONTEXT_BAD
                               or DDERR_CANTLOCKSURFACE
                               or DD_OK
-------------------------------------------------------------------*/
DWORD __stdcall ddiHandleSwap(LPD3DHAL_TEXTURESWAPDATA ptsd)
{
   SETUP_PPDEV(ptsd->dwhContext)
   TXTRHNDL  tmp;
   TXTRHNDL *tex1 = TXTRHNDL_PTR(ptsd->dwHandle1);
   TXTRHNDL *tex2 = TXTRHNDL_PTR(ptsd->dwHandle2);
   LPDDRAWI_DDRAWSURFACE_LCL DDS1, DDS2;

   D3D_ENTRY( "ddiHandleSwap" );

   // This callback is invoked when two texture handles are to be swapped.
   // I.e. the data refered to by the two handles is to be swapped.
   if (CONTEXT_VALIDATE(ptsd->dwhContext))
   {
      ptsd->ddrval = D3DHAL_CONTEXT_BAD;
      D3D_EXIT( DDHAL_DRIVER_HANDLED );
   }

   D3DPRINT( 255, "ddiHandleSwap, ptsd->dwhContext =%08lx ptsd->wHandle1 = %d, ptsd->wHandle2 = %d",
   ptsd->dwhContext, ptsd->dwHandle1, ptsd->dwHandle2 );

   tmp   = *tex1;
   *tex1 = *tex2;
   *tex2 =  tmp;
#ifdef WINNT

   if (NULL == (DDS1 = EngLockDirectDrawSurface((HANDLE)tex1->surfInterface)))
   {
      ptsd->ddrval = DDERR_CANTLOCKSURFACE;
      return DDHAL_DRIVER_HANDLED;
   }
   if (NULL == (DDS2 = EngLockDirectDrawSurface((HANDLE)tex2->surfInterface)))
   {
      EngUnlockDirectDrawSurface(DDS1);
      ptsd->ddrval = DDERR_CANTLOCKSURFACE;
      return DDHAL_DRIVER_HANDLED;
   }
#else
   DDS1  = ((LPDDRAWI_DDRAWSURFACE_INT)(tex1->surfInterface))->lpLcl;
   DDS2  = ((LPDDRAWI_DDRAWSURFACE_INT)(tex2->surfInterface))->lpLcl;
#endif

   //
   // Update texture handles in surfaces if video memory
   //
#ifdef WINNT
   #pragma message(__FILELINE__ "ddiHandleSwap: how do we handle palette swaps?")
#else
   if ( !(DDS1->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY))
   {
      // Either one of the palletized textures are downloaded then need to redownload
      if (PALETTIZED(DDS1->dwReserved1))
         TXTRNEWPALETTE(ppdev,PALETTEGBL((ULONG)ptsd->dwHandle2));
   }

   if ( !(DDS2->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY))
   {
      if (PALETTIZED(DDS2->dwReserved1))
         TXTRNEWPALETTE(ppdev,PALETTEGBL((ULONG)ptsd->dwHandle1));
   }
#endif

   {
      RC *pRc = CONTEXT_PTR(ptsd->dwhContext);
      UPDATE_HW_STATE(SC_SOMETHING);
   }

#ifdef WINNT

   EngUnlockDirectDrawSurface(DDS1);
   EngUnlockDirectDrawSurface(DDS2);
#endif
   ptsd->ddrval = DD_OK;
   D3D_EXIT( DDHAL_DRIVER_HANDLED );
}
#endif // (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)

#ifdef DEFER_TXTR_DNLD
/*----------------------------------------------------------------
Function Name: dtdCreateChecksum
Description: Using the texture data, generate a 32-bit checksum
             of the image.  Scan alternate lines of the texture
             to speed it up a bit.  Larger textures also skip
             every other DWORD of texture data per scanned line.
Returns: 32-bit checksum.
----------------------------------------------------------------*/
static DWORD dtdCreateChecksum (LPDDRAWI_DDRAWSURFACE_LCL srcSurf, // in
                                TXTRDESC *desttxtr                 // in
                                )
{
   WORD w = srcSurf->lpGbl->wWidth, h = srcSurf->lpGbl->wHeight;
   DWORD pitch = srcSurf->lpGbl->lPitch,
                  rowst, rowend, rowlen,           // row/column starts and ends
                  colst, colend,
                  checksum = 0,
                  colskip = sizeof(DWORD);

   rowst = (DWORD)srcSurf->lpGbl->fpVidMem;
   rowend = rowst + h * pitch;
   rowlen = w * desttxtr->bytesPerTexel;
   pitch *= 2;                         // scan alternate lines

   if (h > 32 && w > 32)
   {
      colskip *= 2;                   // skip every other dword on larger textures
   }

   while (rowst < rowend)
   {
      colst = rowst;
      colend = colst + rowlen;
      while (colst < colend)
      {
         checksum += *(DWORD*)colst;
         colst += colskip;
      }
      rowst += pitch;
   }
   return checksum;
}
/*----------------------------------------------------------------
Function Name: dtdCheck4Reload
Description: Determine if this texture needs to be
             downloaded to the hardware.
Returns: nada
----------------------------------------------------------------*/
static void dtdCheck4Reload (TXTRHNDL *psrcSurf,                // in
                             TXTRDESC *desttxtr                 // in
                             )
{
   WORD theLOD, LODmask, slog = 0, tlog = 0;
   DWORD checksum = 0;

   // figure out the LOD for this texture.
   if (desttxtr->numMipmaps > 1)
   {
      while ((psrcSurf->wWidth != (1 << slog)) && slog < MAX_TEXTURE_LOG)
         ++slog;
      while ((psrcSurf->wHeight != (1 << tlog)) && tlog < MAX_TEXTURE_LOG)
         ++tlog;
   }
   else if (desttxtr->numMipmaps <= 1 || desttxtr->flags & AutoMipMaps)
   {
      slog = (WORD)desttxtr->slog;
      tlog = (WORD)desttxtr->tlog;
   }
   theLOD = LOG2LOD (max (slog,tlog));
   LODmask = 1 << theLOD;

   // Calc the new checksum over the texels.
   checksum = dtdCreateChecksum (srcSurf, desttxtr);

   // Set texture to load if the texture is already scheduled for a
   // load, or the new checksum and old checksum differ (because
   // texture changed or hasn't been loaded yet).
   if (checksum != desttxtr->checksum[theLOD])
   {
      desttxtr->reload |= LODmask;
      desttxtr->checksum[theLOD] = checksum;
   }
   return;
}

#endif // DEFER_TXTR_DNLD

/*-------------------------------------------------------------------
Function Name:  TEXTURELOAD
Description:    Downloads a texture and generates mipmaps if appropriate.
Information:    DWORD __stdcall TEXTURELOAD(
                                       NT9XDEVICEDATA           *ppdev,
                                       TXTRHNDL                 *psurfSrc,
                                       RECTL                    *prSrc,
                                       int                      nSrcLOD,
                                       TXTRHNDL                 *psurfDst,
                                       RECTL                    *prDest,
                                       int                      nDstLOD)

Return:         DWORD    DDHAL_DRIVER_NOTHANDLED - upon failure to load
                      or DDHAL_DRIVER_HANDLED - successful load or texture handle out of range.
-------------------------------------------------------------------*/
DWORD __stdcall TEXTURELOAD(
   NT9XDEVICEDATA           *ppdev,
   TXTRHNDL                 *psurfSrc,
   RECTL                    *prSrc,
   int                      nSrcLOD,
   TXTRHNDL                 *psurfDst,
   RECTL                    *prDest,
   int                      nDstLOD
)
{
   TXTRDESC       *desttxtr;
   unsigned long   addr, flags;
   long            cnt = 0;
   DWORD           tmuCnt;
   long            slog, tlog, maxST;
   long            LoadCnt;
   unsigned long  *texels ;
   extern ULONG tLodT1[6][5];
   extern ULONG tLodT0[6][5];
   DWORD  dstHandle;
   DWORD  srcStartOffset, dstStartOffset;
   DWORD  srcPitch, dstPitch;
   DWORD  width,height;

   CMDFIFO_PROLOG(cmdFifo);

   dstHandle = psurfDst->txtrID;

#if defined( NULLDRIVER )

   if (!_D3(ondrtTexD))
   {
      D3D_EXIT( D3D_OK );
   }
#endif

   if (psurfSrc == NULL)
      return DDERR_INVALIDPARAMS;

   if ((dstHandle <= 0) || (dstHandle >= MAXTEXTURECOUNT))
      return DDERR_INVALIDPARAMS;

   // Verify that the source and dest have the same formats.
   if ((psurfSrc->dwFlags & DDPF_RGB) &&
       (psurfDst->dwFlags & DDPF_RGB))
   {
      if (psurfSrc->dwBitCnt != psurfDst->dwBitCnt)
         return DDERR_INVALIDPARAMS;
   }

   D3DPRINT(1, "textureLoad, srcSurf=%8lXh dstSurf=%8lXh, dstTextureDesc=%ld",
            psurfSrc, psurfDst, dstHandle);

   desttxtr = (TXTRDESC *) TXTRDESC_PTR(dstHandle);

#ifdef DEFER_TXTR_DNLD
   dtdCheck4Reload (psurfSrc, nSrcLOD, desttxtr);
#endif // DEFER_TXTR_DNLD

   // each tmu
   for (tmuCnt= 0; tmuCnt < NUM_AVAILABLE_TMUS; ++tmuCnt)
   {
      ULONG ltLOD;

      // any miploads to download to this tmu?
      if (desttxtr->mipmapsOnTmu[tmuCnt]
#ifdef DEFER_TXTR_DNLD
         && desttxtr->reload         // and any mipmaps need reloading...
#endif // DEFER_TXTR_DNLD
         )
      {

         // CSR - Verdict texture sizes test show a corruption in tall skinny textures
         // if you don't flush the cache before download.  Perf issues seem minor, I
         // measured a < .5% hit on WinBench3D.

         // Flush the texture cache by writing any other base addr (so invert the base addr)
         CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
         SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, texBaseAddr, TMU2CHIP(tmuCnt) ) );
         SETPD( cmdFifo, SST_TREX(ghw0,tmuCnt)->texBaseAddr, (0xffffffff ^ desttxtr->baseAddr[tmuCnt]) );

         CMDFIFO_CHECKROOM( cmdFifo, PH4_SIZE + 2 );
         SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0 | R1, textureMode, TMU2CHIP(desttxtr->tmuID[tmuCnt])));

         // auto generating mipmaps - will only download the first mipmap
         if (desttxtr->flags & AutoMipMaps)
         {
            if (desttxtr->tmuID[tmuCnt] == TREX0)
               ltLOD = desttxtr->tLODnoMipMaps[tmuCnt] | tLodT0[0][desttxtr->home - 1];
            else
               ltLOD = desttxtr->tLODnoMipMaps[tmuCnt] | tLodT1[0][desttxtr->home - 1];
         }
         else
         {
            if (desttxtr->tmuID[tmuCnt] == TREX0)
               ltLOD = desttxtr->tLOD[tmuCnt] | tLodT0[0][desttxtr->home - 1];
            else
               ltLOD = desttxtr->tLOD[tmuCnt] | tLodT1[0][desttxtr->home - 1];
         }

         SETPD( cmdFifo, SST_TREX(ghw0, desttxtr->tmuID[tmuCnt])->textureMode, desttxtr->format); // only need format set
         SETPD( cmdFifo, SST_TREX(ghw0, desttxtr->tmuID[tmuCnt])->tLOD, ltLOD );

         D3DPRINT(7, "  textureMode=0x%x tLOD=0x%x", desttxtr->format, ltLOD);

         flags = 0;

         if( desttxtr->flags & AutoMipMaps )
         {
            // We are creating the mip levels so just pass it through normally
            slog = desttxtr->slog;
            tlog = desttxtr->tlog;
            addr = desttxtr->start[tmuCnt];
            LoadCnt = desttxtr->numMipmaps;
         }
         else
         {
            LoadCnt = 1;

            if(desttxtr->numMipmaps > 1)
            {
               //
               // part of a mipmapped texture load.  We need to compute the
               // slog, tlog and start address before performing the download
               //

               // compute slog, tlog, and the lod for the current mip
               // level
               slog = 0;
               while (((psurfSrc->mmData[nSrcLOD].wWidth - (0x01 << slog)) != 0) &&
                      (slog < MAX_BIGTEXTURE_LOG))
                  ++slog;

               tlog = 0;
               while (((psurfSrc->mmData[nDstLOD].wHeight - (0x01 << tlog)) != 0) &&
                      (tlog < MAX_BIGTEXTURE_LOG))
                  ++tlog;

               // board address offset
               // retro3dfx: restore the line rev 40 (10/25/00 "no longer use
               // surface local pointers") dropped — without it `addr` is stale
               // for every per-LOD download of a mipmapped texture, so mip
               // texels land at the wrong board offset and sampling reads
               // unwritten memory (black/garbage textures in all mipmapped
               // D3D content; Win9x rev 35 has this line and works).
               addr = psurfDst->mmData[nDstLOD].fpVidMem - _FX(textureHeapStart[tmuCnt]);
            }
            else
            {
               // just a regular non-mipped texture
               slog = desttxtr->slog;
               tlog = desttxtr->tlog;
               addr = desttxtr->start[tmuCnt];
            }
         }

         // each mipmap level
         for (cnt = 0; cnt < LoadCnt; ++cnt)
         {

            // if this mipmap gets downloaded to this tmu
            (slog > tlog) ? (maxST = slog) : (maxST = tlog);
            if (desttxtr->mipmapsOnTmu[tmuCnt] & (1<<maxST)
#ifdef DEFER_TXTR_DNLD
               // AND this texture needs reloading
               && (desttxtr->reload & (1 << LOG2LOD(maxST)))
#endif // DEFER_TXTR_DNLD
               )
            {
#ifdef DEFER_TXTR_DNLD
               // This LOD is loaded, clear its bit.
               desttxtr->reload &= ~(1 << LOG2LOD(maxST));
#endif // DEFER_TXTR_DNLD
               texels = (unsigned long *) psurfSrc->mmData[nSrcLOD].fpVidMem;

#ifdef WINNT
               if (DDSCAPS_VIDEOMEMORY & psurfSrc->dwCaps)
                 texels = (PULONG)((DWORD)texels + (DWORD)ppdev->pjLfbBase);
#endif

               srcPitch = psurfSrc->mmData[nSrcLOD].lPitch;

               // aspect ratio > 8 so build a new one
               if (desttxtr->flags & AspectRatioGT8 && (maxST-3-cnt) >= 0)
               {
                  // CreateNewTexture wants the actual, uncorrected size
                  // of the bad-aspect ratio texture.  For the numMipmaps
                  // <= 1 case, we calculated that during the surface creation
                  // and stashed it in desttxtr->initialSlog, etc.  slog
                  // and tlog where recomputed in the numMipmaps > 1 case
                  // based on the surface size in the code above.

                  if(desttxtr->numMipmaps <= 1)
                  {
                     slog = desttxtr->initialSlog;
                     tlog = desttxtr->initialTlog;
                  }

                  texels = CREATENEWTEXTURE(ppdev,
                                             texels,
                                             slog - cnt,
                                             tlog - cnt,
                                             srcPitch,
                                             desttxtr->bytesPerTexel,
                                             &_D3G) ;

                  if (!texels)
                     return DDERR_OUTOFMEMORY;

                  // Here's the magic...  When we actually download the
                  // texture, slog and tlog have to represent it's
                  // actual size. For numMipmaps<=1, slog and tlog
                  // become desttxtr->slog and tlog - the corrected
                  // size.  For numMipmaps > 1, desttxtr->initialSlog -
                  // slog is the LOD for this mipmap and we subtract that
                  // from desttxtr->slog, the corrected slog of the level
                  // 0 texture.  (CSR - 18 Dec 98 - with Miriam)

                  slog = desttxtr->slog - (desttxtr->initialSlog - slog);
                  tlog = desttxtr->tlog - (desttxtr->initialTlog - tlog);

                  // The newly generated texture has a standard pitch.  This
                  // should probably be returned from the function...
                  srcPitch = (1<<slog) * desttxtr->bytesPerTexel;
               }

               // Yanking partial texture download on Voodoo2.  Monster Truck
               // Madness 2 completely horks when this is in...

               if( desttxtr->flags & AutoMipMaps )
               {
                  // We can either do partial blts or automipmaps here.  I chose to let the
                  // auto mipmaps to work and disallow partial blts. - JP

                  width  = 1<<slog;
                  height = 1<<tlog;

                  srcStartOffset = 0L;
                  dstStartOffset = 0L;

                  dstPitch = width * desttxtr->bytesPerTexel;
               }
               else if (desttxtr->flags & AspectRatioGT8)
               {
                  // Don't do a partial blt if the aspect ratio is gt 8, just download the whole thing.
                  width  = 1<<slog;
                  height = 1<<tlog;

                  srcStartOffset = 0L;
                  dstStartOffset = 0L;

                  dstPitch = width * desttxtr->bytesPerTexel;
               }
               else
               {
                  // try to adjust the columns to DWORD Boundries. If that cannot be done
                  // then just download the whole texture.
                  //

                  DWORD  leftAdjust = 0L, rightAdjust = 0L;

                  while( (((prDest->left - leftAdjust) * desttxtr->bytesPerTexel) & 3) &&
                        (leftAdjust < MAX_TEXTURE_SIZE_U) )
                  {
                     leftAdjust++;
                  }

                  while( (((prDest->right + rightAdjust) * desttxtr->bytesPerTexel) & 3) &&
                        (rightAdjust < MAX_TEXTURE_SIZE_U) )
                  {
                     rightAdjust++;
                  }


                  if (((int)(prDest->left  - leftAdjust)  < 0) ||
                      ((int)(prDest->right + rightAdjust) > psurfDst->mmData[nDstLOD].wWidth))
                  {
                     width  = 1<<slog;
                     height = 1<<tlog;

                     srcStartOffset = 0L;
                     dstStartOffset = 0L;

                     dstPitch = width * desttxtr->bytesPerTexel;
                  }
                  else
                  {
                     width = (prSrc->right + rightAdjust) - (prSrc->left - leftAdjust);
                     height = prSrc->bottom - prSrc->top;


                     srcStartOffset = (prSrc->top * psurfSrc->mmData[nSrcLOD].lPitch) +
                                      ((prSrc->left - leftAdjust) * desttxtr->bytesPerTexel);

                     dstStartOffset = (prDest->top * psurfDst->mmData[nDstLOD].lPitch) +
                                       ((prDest->left - leftAdjust) * desttxtr->bytesPerTexel);

                     dstPitch = psurfDst->mmData[nDstLOD].lPitch;
                  }
               }

               CMDFIFO_SAVE( cmdFifo );

               // automatically generate mipmaps
               if (desttxtr->flags & AutoMipMaps)
               {
#define FORMAT8888      1
#define FORMAT_ORIGINAL 2
                  if (cnt == 0)
                     flags = FORMAT8888;
                  else
                     flags = FORMAT_ORIGINAL;
               }

               // Running into some bad data here from stress:
               //  Adding a check for data that doesn't make
               //  good sense.
               if ((width * desttxtr->bytesPerTexel) > srcPitch)
               {
                 return DDERR_INVALIDPARAMS;
               }
               
               // download
               addr = txtrDownLoadTexture(ppdev,
                                          desttxtr->tmuID[tmuCnt],   // which TREX chip to load [0,3]
                                          addr,                      // base address of lod 0
                                          desttxtr->aspectRatio,     // aspect ratio
                                          0,                         // tiled (assume linear for now)
                                          0,                         // tile stride
                                          slog,                      // log2 of S size
                                          tlog,                      // log2 of T size
                                          desttxtr->bytesPerTexel,   // bytes per texel
                                          srcPitch,
                                          dstPitch,
                                          srcStartOffset,
                                          dstStartOffset,
                                          width,
                                          height,
                                          texels,                    // the texture data
                                          desttxtr->home,            // location of texture
                                          flags,
                                          desttxtr->format >> SST_TFORMAT_SHIFT,
                                          desttxtr->flags & TextureIsBig);

               CMDFIFO_RELOAD( cmdFifo );

            } // download this mipmap to this TMU

            // next mipmap level
            if (slog > 0)
               --slog;
            if (tlog > 0)
               --tlog;

            //
            // Partial blts are not allowed with auto-mipmapped textures.
            //

            // next mipmap is attached to this surface
//            if (surfLCLnext->lpAttachList != NULL)
//               surfLCLnext = surfLCLnext->lpAttachList->lpAttached ;

         } // each mipmap
      } // any mipmaps on this tmu?
   } // each tmu

   CMDFIFO_EPILOG( cmdFifo );

   desttxtr->flags |= BltToTxtrInFifo;

   _D3( lastContext ) = 0;

   return (D3D_OK);

} /* textureLoad */


#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
// ddiHandleGetSurf is not reported in DX7
#else
/*
 *
 */
/*-------------------------------------------------------------------
Function Name:  ddiHandleGetSurf
Description:    This callback is invoked when the d3d needs to obtain the surface
                 refered to by a handle.
Information:    DWORD __stdcall ddiHandleGetSurf(LPD3DHAL_TEXTUREGETSURFDATA ptgd)
Return:         DWORD DDHAL_DRIVER_HANDLED
                ptgd->ddrval = D3DHAL_CONTEXT_BAD
                               or DD_OK
-------------------------------------------------------------------*/
DWORD __stdcall ddiHandleGetSurf(LPD3DHAL_TEXTUREGETSURFDATA ptgd)
{
   SETUP_PPDEV(ptgd->dwhContext)
   D3D_ENTRY( "ddiHandleGetSurf" );

   // This callback is invoked when the d3d needs to obtain the surface
   // refered to by a handle.
   if (CONTEXT_VALIDATE(ptgd->dwhContext))
   {
      D3DPRINT( 255, "ddiHandleGetSurf, bad context =0x08lx", ptgd->dwhContext );
      ptgd->ddrval = D3DHAL_CONTEXT_BAD;
      D3D_EXIT( DDHAL_DRIVER_HANDLED );
   }

   D3DPRINT( 255, "TextureGetSurf, ptgd->dwhContext =%08lx ptgd->wHandle = %d",
            ptgd->dwhContext, ptgd->dwHandle );

#ifdef WINNT
   ptgd->hDDS  = (HANDLE)TXTRHNDL_PTR(ptgd->dwHandle)->surfInterface;
#else
   ptgd->lpDDS  = TXTRHNDL_PTR(ptgd->dwHandle)->surfInterface;
#endif
   ptgd->ddrval = DD_OK;
   D3D_EXIT( DDHAL_DRIVER_HANDLED );
}
#endif // (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)


void txtMipMapInit(void)
{
   int ar,lod;

   for (ar=0; ar<4; ar++)              // for each aspect ratio
   {
      // Start at the LOD that contains a 256 size to it.
      // Larger textures are offset negatively from there.
      txtBigMipMapOffset[ar][3] = 0;     // start off with offset=0
      for (lod=4; lod<=MAX_TEXTURE_LOD; lod++)
      {    // for each lod, add in previous size
         txtBigMipMapOffset[ar][lod] = txtBigMipMapOffset[ar][lod-1] +
                                    txtBigMipMapSize[ar][lod-1];
      }
      for (lod=2; lod >=0;lod--)
      {
         txtBigMipMapOffset[ar][lod] = txtBigMipMapOffset[ar][lod+1] - txtBigMipMapSize[ar][lod];
      }

      txtBigMipMapOffsetTsplit[ar][0] = 0;
      txtBigMipMapOffsetTsplit[ar][1] = 0;
      for (lod=2; lod<=MAX_TEXTURE_LOD; lod++)
      {    // for each lod, add in previous size
         txtBigMipMapOffsetTsplit[ar][lod] = txtBigMipMapOffsetTsplit[ar][lod-2] +
                                             txtBigMipMapSize[ar][lod-2];
      }


      txtMipMapOffset[ar][0] = 0;     // start off with offset=0
      for (lod=1; lod<=MAX_TEXTURE_LOD; lod++)
      {    // for each lod, add in previous size
         txtMipMapOffset[ar][lod] = txtMipMapOffset[ar][lod-1] +
                                    txtMipMapSize[ar][lod-1];
      }

      txtMipMapOffsetTsplit[ar][0] = 0;
      txtMipMapOffsetTsplit[ar][1] = 0;
      for (lod=2; lod<=MAX_TEXTURE_LOD; lod++)
      {    // for each lod, add in previous size
         txtMipMapOffsetTsplit[ar][lod] = txtMipMapOffsetTsplit[ar][lod-2] +
                                          txtMipMapSize[ar][lod-2];
      }
   }
}

struct textureFormatData
{
   unsigned int aMask;
   unsigned int aShift;
   unsigned int rMask;
   unsigned int rShift;
   unsigned int gMask;
   unsigned int gShift;
   unsigned int bMask;
   unsigned int bShift;
} tfd[TEXFMT_MAX]=
{
   {RGB332_AMASK,0,RGB332_RMASK,5,RGB332_GMASK,2,RGB332_BMASK,0},      //RGB_332            0
   {0,0,0,0,0,0,0,0},                                                  //YIQ_422            1
   {A8_AMASK,0,0,0,0,0,0,0},                                           //ALPHA_8            2
   {L8_LMASK,0,0,0,0,0,0,0},                                           //INTENSITY_8        3
   {LA8_AMASK,4,LA8_LMASK,0,0,0,0,0},                                  //ALPHA_INTENSITY_44 4
   {0,0,0,0,0,0,0,0},                                                  //P8_RGB             5
   {0,0,0,0,0,0,0,0},                                                  //P8_RGBA            6
   {0,0,0,0,0,0,0,0},                                                  //RSVD1              7
   {RGB8332_AMASK,8,RGB8332_RMASK,5,RGB8332_GMASK,2,RGB8332_BMASK,0},  //ARGB_8332          8
   {0,0,0,0,0,0,0,0},                                                  //AYIQ_8422          9
   {RGB565_AMASK,0,RGB565_RMASK,11,RGB565_GMASK,5,RGB565_BMASK,0},     //RGB_565            10
   {RGB1555_AMASK,15,RGB1555_RMASK,10,RGB1555_GMASK,5,RGB1555_BMASK,0},//ARGB_1555          11
   {RGB4444_AMASK,12,RGB4444_RMASK,8,RGB4444_GMASK,4,RGB4444_BMASK,0}, //ARGB_4444          12
   {LA16_AMASK,8,LA16_LMASK,0,0,0,0,0},                                //ALPHA_INTENSITY_88 13
   {0,0,0,0,0,0,0,0},                                                  //ALPHA_P8_RGB       14
   {RGB8888_AMASK,24,RGB8888_RMASK,16,RGB8888_GMASK,8,RGB8888_BMASK,0} //ARGB_8888          15
};

//
/*-------------------------------------------------------------------
Function Name:  txtrformatTo8888
Description:    format one row of a texture into a texture with format argb8888
Information:    VOID txtrformatTo8888(FxU32 *src, FxU32 *trg,
                 unsigned int bpt, unsigned int width, unsigned int format)

                    format - Texture Format Data index
                    bpt - bytes per texel of src
                    trg - address of target
                    src - address of source
                    width - width of row

Return:         VOID
            The address pointed to by trg is filled with new formatted texture.
-------------------------------------------------------------------*/
VOID
txtrformatTo8888(FxU32 *src, FxU32 *trg,
                 unsigned int bpt, unsigned int width, unsigned int format)
{
   unsigned int j;
   unsigned int sum;
   struct textureFormatData *tf = &tfd[format];

   if (bpt == 2)
   {
      FxU16 *data16 = (FxU16 *)src;
      for (j=0; j < width; ++j)
      {
         sum  =  ((*data16 & tf->aMask) >> tf->aShift) << 24;
         sum |=  ((*data16 & tf->rMask) >> tf->rShift) << 16;
         sum |=  ((*data16 & tf->gMask) >> tf->gShift) << 8;
         sum |=  ((*data16 & tf->bMask) >> tf->bShift) ;
         trg[j] = sum;
         ++data16;
      }
   } // 2 bytes per texel

   else
   {
      FxU8 *data8 = (FxU8 *)src;
      for (j=0; j < width; ++j)
      {
         sum  =  ((*data8 & tf->aMask) >> tf->aShift) << 24;
         sum |=  ((*data8 & tf->rMask) >> tf->rShift) << 16;
         sum |=  ((*data8 & tf->gMask) >> tf->gShift) << 8;
         sum |=  ((*data8 & tf->bMask) >> tf->bShift) ;
         trg[j] = sum;
         ++data8;
      }
   }
}

#define B0(x)   ((x>>24)&0xFF)
#define B1(x)   ((x>>16)&0xFF)
#define B2(x)   ((x>>8)&0xFF)
#define B3(x)   ((x>>0)&0xFF)

/*-------------------------------------------------------------------
Function Name:  txtrFormatToOriginal
Description:    format one row of a texture from argb8888 back to the original texture format
Information:    VOID txtrFormatToOriginal(FxU32 *src, FxU32 *trg,
                     unsigned int bpt, unsigned int width, unsigned int format)

                    format - Texture Format Data index
                    bpt - bytes per texel of src
                    trg - address of target
                    src - address of source
                    width - width of row

Return:         VOID
                The address pointed to by trg is filled with new formatted texture.
-------------------------------------------------------------------*/
VOID
txtrFormatToOriginal(FxU32 *src, FxU32 *trg,
                     unsigned int bpt, unsigned int width, unsigned int format)
{
   FxU16 *data16 = (FxU16 *)trg;
   FxU8  *data8  = (FxU8  *)trg;
   unsigned int j, sum;
   struct textureFormatData *tf = &tfd[format];

   for (j=0; j < width; ++j)
   {
      sum  = B0(src[j]) << tf->aShift;
      sum |= B1(src[j]) << tf->rShift;
      sum |= B2(src[j]) << tf->gShift;
      sum |= B3(src[j]) ;
      if (bpt == 2)
      {
         *data16 = (FxU16)sum;
         ++data16;
      }
      else
      {
         *data8 = (FxU8)sum;
         ++data8;
      }
   }
}


/*-------------------------------------------------------------------
Function Name:  txtrImgHalve
Description:    Filter down the incoming texture.
Information:    static void
                txtrImgHalve(long *outdata, int width, int height, long *indata)
Return:         VOID
                outdata - address of newly filtered txtr.
-------------------------------------------------------------------*/
static void
txtrImgHalve(long *outdata, int width, int height, long *indata)
{
   unsigned int i,j,k;
   unsigned int w,h, *p,sum,*q;

   if ((outdata == NULL)       ||
      (width   <= 0   )       ||
      (height  <= 0   )       ||
      (width & (width-1)) ||
      (height & (height-1)) ||
      ((width == 1) && (height==1)))
   {
      return;
   }

   w = width>>1;
   h = height>>1;
   p = (unsigned int *) outdata;
   q = (unsigned int *) indata;

   if ((w == 0) || (h == 0))
   {
      // Input and output are a single span each (row or column)
      for (j=0; j<w; j++)
      {
         sum = B0(q[0]) + B0(q[1]); sum = (sum + 1) >> 1; k = sum;
         sum = B1(q[0]) + B1(q[1]); sum = (sum + 1) >> 1; k = (k << 8) + sum;
         sum = B2(q[0]) + B2(q[1]); sum = (sum + 1) >> 1; k = (k << 8) + sum;
         sum = B3(q[0]) + B3(q[1]); sum = (sum + 1) >> 1; k = (k << 8) + sum;
         *p++ = k;
         q += 2;
      }
      return;
   }

   for (i=0; i<h; i++)
   {
      for (j=0; j<w; j++)
      {
         sum = B0(q[0]) + B0(q[1]) + B0(q[width]) + B0(q[width+1]);
         sum = (sum + 2) >> 2;   // add 2 to round, then divide by 4
         k = sum;
         sum = B1(q[0]) + B1(q[1]) + B1(q[width]) + B1(q[width+1]);
         sum = (sum + 2) >> 2;   // add 2 to round, then divide by 4
         k = (k<<8) + sum;
         sum = B2(q[0]) + B2(q[1]) + B2(q[width]) + B2(q[width+1]);
         sum = (sum + 2) >> 2;   // add 2 to round, then divide by 4
         k = (k<<8) + sum;
         sum = B3(q[0]) + B3(q[1]) + B3(q[width]) + B3(q[width+1]);
         sum = (sum + 2) >> 2;   // add 2 to round, then divide by 4
         k = (k<<8) + sum;
         *p++ = k;
         q += 2;
      }
      q += width;
   }
}

//------------------------------------------------
//  DOWNLOAD!
//
//
//------------------------------------------------

#ifdef K6_2

/*----------------------------------------------------------------
Function Name: txtrDownloadK6
Description:   Helper function for txtrDownLoadTexture to use K6
               inline assembly without nuking txtrDownLoadTexture's
               optimization.
Information:   Below.
Return:        Updated cmdFifo and hwIndex for caller.
----------------------------------------------------------------*/
static void __stdcall txtrDownloadK6(NT9XDEVICEDATA * ppdev,  // in
                                     int height,              // in
                                     int width,               // in
                                     int bpt,                 // in
                                     unsigned long flags,     // in
                                     FxU32 *data32,           // in
                                     long *vaddr,             // in
                                     unsigned long format,    // in
                                     DWORD srcPitchAdjust,    // in
                                     DWORD dstPitchAdjust,    // in
                                     FxU32 **pcmdFifo,        // in/out
                                     FxU32 *phwIndex          // in/out
                                     )
{
   FxU32 *cmdFifo = *pcmdFifo;
   FxU32 hwIndex = *phwIndex;
   int cnt = (width*bpt)/4;
   DWORD cmd = CMDFIFO_BUILD_PK5((FxU32)cnt, 0, 0,(FxU32)SSTCP_PKT5_TEXPORT );
   DWORD dt;
   int t;


   // Texture downloading optimization:
   // We do 8-bytes moves for downloading a texture into the board
   // via MMX registers in aggressively scheduled loop.
   // Currently it shows about 2.4 times improvement for K62CXT-350
   // (Final Reality test - 3D transfer rate)
   // Actually that can be done for regular Intel CPU with MMX as well,
   // but do we need to do this favour for the AMD competitor ?

   __asm   __emit 0xF __asm __emit 0xE // femms

   for (t = 0; t < height; t++)
   {
      dt = ((FxU32)&vaddr[0]);
      if (IS_NAPALM)
      {  // Use the larger texture download port if on Napalm because offsets
         // can be very large for mip-maps etc.
         dt = (dt + 0x3a00000) & 0x7ffffff;
      }

      // first mipmap is formatted into argb8888
      if (flags & FORMAT8888)
         txtrformatTo8888(data32, &_D3(buffer)[t*width], bpt, width, format);
      else if (flags & FORMAT_ORIGINAL)
      {
         // put the final data into the last row
         data32 = (FxU32 *)_D3(buffer)+ (MAX_TEXTURE_SIZE);

         // One row at a time filter image down into the same buffer
         txtrImgHalve(&_D3(buffer)[t*width], width+width,
                        ((height+height)>1?2:1),
                        &_D3(buffer)[t*2*(width+width)]);

         // Reformat texture into original format
         txtrFormatToOriginal(&_D3(buffer)[t*width], data32, bpt, width, format);
      }

      // add 1 for alignment dword
      CMDFIFO_CHECKROOM( cmdFifo, PH5_SIZE + (FxU32)cnt + 1);

      __asm   mov   eax, cmdFifo
      // hwIndex should be 0 due to check room above
      __asm   test  eax, 4
      __asm   mov   ecx, cnt
      __asm   movd  mm0, cmd
      __asm   jz    aligned
      // align to 8-bytes boundary
      __asm   mov   dword ptr [eax], 0
      __asm   inc   hwIndex
      __asm   add   eax, 4
      aligned:
      __asm   punpckldq mm0, dt
      __asm   shr   ecx, 1       // cnt / 2 (unrolling the loop below)
      __asm   mov   edx, data32
      __asm   movq  [eax], mm0
      __asm   jz    no_cycle
      __asm   pushf

      // here is the texture downloading tight loop
      // that we should make agressively optimized
      do_cycle:
      __asm   movq  mm1, [edx]
      __asm   add   eax, 8

      __asm   add   edx, 8
      __asm   dec   ecx

      __asm   movq  [eax], mm1
      __asm   jnz   do_cycle

      __asm   popf
      no_cycle:
      __asm   jnc   all_done
      // make one more 4-bytes move
      __asm   mov   ecx, [edx]
      __asm   mov   [eax+8], ecx

      all_done:
      hwIndex += cnt + 2;
      data32 += cnt;
      vaddr += cnt;

      (char *)data32 += srcPitchAdjust;
      (char *)vaddr  += dstPitchAdjust;
   }

   __asm  __emit 0xF __asm __emit 0xE // femms

   // Update pointers for caller
   *pcmdFifo = cmdFifo;
   *phwIndex = hwIndex;
}
#endif // K6_2

/*-------------------------------------------------------------------
Function Name:  txtrDownLoadTexture
Description:    Download a square texture to sst (no splitting)
                and return the next available texture addr
Information:    <See function header for I/O>
Return:         unsigned long - next available texture memory location.
-------------------------------------------------------------------*/
unsigned long __stdcall txtrDownLoadTexture(  NT9XDEVICEDATA * ppdev,
                                              int trex,              // which TREX chip to load [0,3]
                                              unsigned long addr,    // base address of the lod with 256 in it.
                                              int ar,                // log2 of S - log2 of T of
                                              int tiled,
                                              int tStride,
                                              //   the max mipmap level
                                              int slog,              // log2 of S size for this mipmap
                                              int tlog,              // log2 of T size for this mipmap
                                              int bpt,               // bytes per texel
                                              int srcPitch,
                                              int dstPitch,
                                              int srcStartOffset,
                                              int dstStartOffset,
                                              int width,
                                              int height,
                                              unsigned long *data,   // texture data
                                              unsigned long home,    // location of texture on tmus
                                              unsigned long flags,
                                              unsigned long format,
                                              unsigned long txtrIsBig)
{
   long          s,t, lodmax, retval;
   DWORD         srcPitchAdjust, dstPitchAdjust;
   unsigned long baddr;

   CMDFIFO_PROLOG(cmdFifo);

   // NOTE: ar is not necessarily slog-tlog, eg. its possible to have a texture
   // that is 1x2 with an ar=3, figure that out!

   if (slog >= tlog)
   {
      if (txtrIsBig)
      {
         lodmax = LOG2LODTBIG(slog);
      }
      else
      {
         lodmax = LOG2LOD(slog);
      }
   }
   else
   {
      if (txtrIsBig)
         lodmax = LOG2LODTBIG(tlog);
      else
         lodmax = LOG2LOD(tlog);
   }

   if (txtrIsBig)
      retval = addr+txtBigMipMapSize[ar][lodmax]*bpt;
   else
      retval = addr+txtMipMapSize[ar][lodmax]*bpt;// compute next avail tex memory

   // CSR - Handle partial texture download case.  Offset data by srcStartOffset
   // if they are loading from somewhere other than the origin.  srcStartOffset
   // is forced to zero by the caller when no PTDL is being done.  Note: width
   // and height must be correct or you will go trapsing off the end of the
   // data.  The caller is responsible for ensuring that offsets, widths and
   // heights are valid for the surface involved.

   (char *)data += srcStartOffset;

   if (trex > 0)
      D3DPRINT(5,"sstDownLoadTexture","TMU %d is invalid, only 0 is valid\n",trex);

   if (txtrIsBig && lodmax==TBIG_LOD_1)                          // special case 1x1
      retval = (retval+15) & ~15;               // round up to next 16 bytes
   else if (!txtrIsBig && lodmax == LOD_1)
      retval = (retval+15) & ~15;

   baddr = txtrCalcBaseAddress(addr, lodmax, ar, (bpt << 3), home,txtrIsBig) ;

   // This should only ever happen on H5!
   if (baddr & BIT(25))
   {
      baddr &= 0x1fffff0;
      baddr |= BIT(1);
   }

   D3DPRINT(5,"sstDownLoadTexture(trex=%d,addr=0x%x,baddr=0x%x log=%d,%d,%s,0x%x) ==> 0x%x",
            trex,addr,baddr,slog,tlog, bpt==1?"8b":"16b",data,retval);
   addr=baddr;

   if ( addr & 0xF )                           // check for 16-byte alignment
      D3DPRINT(0, "sstDownLoadTexture","invalid (unaligned-16) texture base address=0x%x\n",addr);



   CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
   SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, texBaseAddr, TMU2CHIP(trex) ) );
   SETPD( cmdFifo, SST_TREX(ghw0,trex)->texBaseAddr, addr );
   D3DPRINT(6,"texBaseAddr 0x%x\n",addr);



// Banshee texture download
   s=0;

   srcPitchAdjust = srcPitch - (width*bpt);
   dstPitchAdjust = dstPitch - (width*bpt);

   // linear memory texture download
   {
      long  *vaddr;
      FxU32 *data32 = (FxU32 *) data;

      if (txtrIsBig)
      {
         addr = ((-1*txtBigMipMapOffset[ar][0]*bpt) - (-1*txtBigMipMapOffset[ar][lodmax]*bpt)) + dstStartOffset;
      }
      else
      {
         addr = txtMipMapOffset[ar][lodmax]*bpt + dstStartOffset;
      }
#ifdef CMDFIFO
      addr &= 0x7ffffff;   // Mask to bits [26:0].
      vaddr = (long *)(addr);

#else
      if (IS_NAPALM)
      {
         vaddr = (long *)(addr + SST_TEX2_ADDRESS(ghw0));// add in texture address space
      }
      else
      {
         vaddr = (long *)(addr + SST_TEX_ADDRESS(ghw0));// add in texture address space
      }
#endif

      // 1 by N : send 1 byte per row
      if (width==1 && bpt==1)
      {
         int shift = 0;
         int mask = 1;

         for (t = 0; t < height; t++)
         {
            CMDFIFO_CHECKROOM( cmdFifo, PH5_SIZE + 1 );
            SETPH( cmdFifo, CMDFIFO_BUILD_PK5( (FxU32)1, ((~mask) & 0xF), 0, (FxU32)SSTCP_PKT5_TEXPORT ) );
            SETPH( cmdFifo, ((FxU32)&vaddr[0]) );
            SETPD( cmdFifo, vaddr[0], ((*data32 & 0xFF) << shift) );   // 1 byte
            shift += 8;
            mask <<= 1;
            if (shift > 24)
            {
               shift =  0;
               mask  =  1;
               vaddr += 1;
            }
            (char *)data32 += 1;
            (char *)data32 += srcPitchAdjust;
            (char *)vaddr  += dstPitchAdjust;
         }
      }
      // 1 by N 16-bit  or 2 by N 8-bit : send 2 bytes each row (dword)
      else if ((width==1 && bpt==2) || (width==2 && bpt==1))
      {
         for (t = 0; t < height; t++)
         {
            CMDFIFO_CHECKROOM( cmdFifo, PH5_SIZE + 1 );
            if (t & 1)
            {
               SETPH( cmdFifo, CMDFIFO_BUILD_PK5( (FxU32)1, 0x3, 0, (FxU32)SSTCP_PKT5_TEXPORT ) );
               SETPH( cmdFifo, (  (FxU32)&vaddr[0]) );
               SETPD( cmdFifo, vaddr[0], *data32 << 16);// upper 2 bytes
               vaddr  += 1;
            }
            else
            {
               SETPH( cmdFifo, CMDFIFO_BUILD_PK5( (FxU32)1, 0xc, 0, (FxU32)SSTCP_PKT5_TEXPORT ) );
               SETPH( cmdFifo, (  (FxU32)&vaddr[0]) );
               SETPD( cmdFifo, vaddr[0], *data32 );    // lower 2 bytes
            }
            (char *)data32 += 2;
            (char *)data32 += srcPitchAdjust;
            (char *)vaddr  += dstPitchAdjust;
         }
      }
      else
      {

#if defined(CMDFIFO) && !defined(NOKNITEXTUREDOWNLOAD) && !defined(WINNT)

         //
         // # of dwords of data that constitute a "large" enough texture download
         // to use an optimzied copy function
         // TDL = texture download
         //
#define TDL_LARGETEXTURE_DWORDS     0x28

         // for large (>= TDL_LARGETEXTURE_DWORDS) texture downloads, align the
         // command fifo to a TDL_CF_DW_ALIGN-dword boundary
         // CF = "command fifo", DW = "dwords"
         //
#define TDL_CF_DW_ALIGN    8

         // byte mask used to compute dword alignment of cmdfifo
         //
#define TDL_CF_ALIGN_MASK  ((TDL_CF_DW_ALIGN * 4) - 1)

         // test for fast path texture download.   For now, don't handle
         // non-zero srcPitchAdjust.   3dwb99 has only dstPitchAdjust == 0,
         // and has about 10% non-zero srcPitchAdjust downloads (and in
         // those cases it's always == 4)
         // assumes that the hidden cmdfifo counter "hwIndex" is used!, so
         // only compiles in when using the command fifo
         //

         if ((((flags & FORMAT8888) | (flags & FORMAT_ORIGINAL)) == 0) &&
            ((srcPitchAdjust + dstPitchAdjust) == 0))
         {
            extern void KNI_movq_memcpy(FxU32 *srcPtr, FxU32 *dstPtr, FxU32 nDwords);
            FxU32 i;
            FxU32 tmp;     // temp in calculating cf alignment
            FxU32 nTextureDwords;   // # dwords of texture data to copy
            FxU32 nTextureDwordsR;
            FxU32 nAlignDwords;  // # of nop cmd packets needed to
            // align the cmdfifo for fast d/l

            // calculate # of dwords of data to copy into the fifo
            //
            nTextureDwords = height * width * bpt / 4;


            nTextureDwordsR = nTextureDwords;
            while (nTextureDwordsR > 0)
            {
               P6FENCE;
               // Never try to download a texture that uses more than 64k of the fifo
               // at one time, and we need to fence to keep the hole counting working.
               // Note:  nTextureDwordsR is in dwords.
               if (nTextureDwordsR > 0x4000)
               {
                  nTextureDwords = 0x4000;
                  nTextureDwordsR -= 0x4000;
               }
               else
               {
                  nTextureDwords = nTextureDwordsR;
                  nTextureDwordsR = 0;
               }

               // check for room for entire d/l packet, including largest
               // possible # of alignment dwords that may be needed.
               // We have to do this before calculating the actual # of
               // alignment dwords because we may have to wrap.
               // We may need to break this up into multiple packets for
               // avenger+ large texture downloads
               //
               CMDFIFO_CHECKROOM(cmdFifo, (PH5_SIZE + nTextureDwords + (TDL_CF_DW_ALIGN - 1)));

               // if we're using the katmai d/l routine, look for a large
               // enough # of dwords for it to be a win, otherwise just use
               // the simple load/store loop.    Also, the kni copy routine
               // only works in the AGP command fifo because it doesn't do
               // a pure linear copy
               //
               if ((nTextureDwords < TDL_LARGETEXTURE_DWORDS) ||
                  !(_FF(cpuType) & P6_INTELCPU_WITH_KNI) || !_FF(doAgpCF))
               {
                  // fifo is now has room for full download. Insert nop packets
                  // if necessary to bring the cmdfifo pointer to
                  // TDL_CF_DW_ALIGN-dword alignment after the 2
                  // command packet header words have been written
                  //
                  // tmp = (((((FxU32)cmdFifo) + 8)
                  //  ^^  get the value that the cmdfifo pointer will be after
                  //     the 2 packet header words,
                  //                               & TDL_CF_ALIGN_MASK) >> 2))
                  //  ^^ find which dword the above ptr is, inside of the
                  //     desired dword-alignment
                  //  (TDL_CF_DW_ALIGN - t) & (TDL_CF_DW_ALIGN - 1)
                  //  ^^ compute # of dwords to add to bring the above dword
                  //     to the desired alignment
                  //
                  tmp = (((((FxU32)cmdFifo) + 8) & TDL_CF_ALIGN_MASK) >> 2);
                  nAlignDwords = (TDL_CF_DW_ALIGN - tmp) & (TDL_CF_DW_ALIGN-1);

                  for (i = 0; i < nAlignDwords; i++)
                  {
                     SETPH(cmdFifo, SSTCP_PKT0_NOP);
                  }

                  SETPH(cmdFifo, CMDFIFO_BUILD_PK5((FxU32)nTextureDwords, 0, 0,
                        (FxU32)SSTCP_PKT5_TEXPORT));

                  if (IS_NAPALM)
                  {

                     SETPH(cmdFifo, (((FxU32)&vaddr[0])+0x3a00000) & 0x7ffffff);
                  }
                  else
                  {
                     SETPH(cmdFifo, ((FxU32)&vaddr[0]));
                  }

#ifdef STBPERF_FAST_TEXTURE_DOWNLOAD
                   __asm
                  {
                     // The C loop here does NOT get compiled to a rep movsd by the current compiler.
                     // Doing it explicity is the only way to be sure.
                     // I tried using a "memcpy" for this and it hung the fifo sometimes - I'm not sure why
                     // Anyway we gain 8-10 pts on 3D WB on an AMD K6 300 doing it this way.
                     mov  ecx, nTextureDwords   // get number of dwords to move
                     mov  esi, data32           // get src data ptr
                     mov  edi, cmdFifo          // get ptr to dest (fifo)
                     mov  ebx, hwIndex          // get hardware index
                     shl  ebx, 2
                     add  edi, ebx              // adjust dest ptr

                     rep  movsd                 // move the data
                  }
                  hwIndex += nTextureDwords;
                  data32 += nTextureDwords;
                  vaddr += nTextureDwords;   // Needs updated for large textures
                                             // being downloaded in several steps.

#else
#pragma message("note - NOT compiling with performance opts for 3dnow texture download")
                  for (i = 0; i < nTextureDwords; i++)
                  {
                     SETPD(cmdFifo, vaddr[0], *data32);
                     ++vaddr;                // Needs updated for large textures   
                     ++data32;               // being downloaded in several steps. 
                  }
#endif

               }
               else
               {
                  // If we get here, we're all set to use the faster KNI-based
                  // copy routine
                  // texture download packet header
                  //
                  SETPH(cmdFifo, CMDFIFO_BUILD_PK5((FxU32)nTextureDwords, 0, 0,
                        (FxU32)SSTCP_PKT5_TEXPORT));

                  if (!IS_NAPALM)
                  {
                     SETPH(cmdFifo, ((FxU32)&vaddr[0]));
                  }
                  else
                  {
                     SETPH(cmdFifo, (((FxU32)&vaddr[0])+0x3a00000) & 0x7ffffff);
                  }

                  // move 'em out! (down, over, whatever) to the cmdfifo
                  //
                  KNI_movq_memcpy(data32, cmdFifo + hwIndex, nTextureDwords);

                  // keep the cmdfifo macros up to date by updating the hidden
                  // counter "hwIndex" with the # of dwords that have been
                  // stored to the cmdfifo in the fast copy routine
                  //
                  hwIndex += nTextureDwords;
                  data32 += nTextureDwords;
                  vaddr += nTextureDwords;  // Needs updated for large textures
                                            // being downloaded in several steps.

               }

               if (nTextureDwordsR > 0)
               {
                  // We are looping,so update the setup for the download.
                  CMDFIFO_CHECKROOM(cmdFifo,PH1_SIZE+1);
                  baddr += (nTextureDwords << 2);
                  SETPH(cmdFifo, CMDFIFO_BUILD_PK1(1,0,texBaseAddr,TMU2CHIP(trex)));
                  SETPD(cmdFifo, SST_TREX(ghw0,trex)->texBaseAddr, baddr);
                  vaddr -= nTextureDwords;
               }

               // This causes a series of extra Bump's in the event of
               // AGP Winbench Test 58 where we have a 2MB Dword dwords
               // dowdload but only a 1MB Dword cmdfifo
               // The fifo_Wrap code assumes that we have had at least one
               // bump <rdPtr != CMDFIFOSTART> and this will cause it to happen 
               CMDFIFO_EPILOG(cmdFifo);
            }

            // all done
            //
            CMDFIFO_EPILOG(cmdFifo);
            return retval;               // return the next available loc

         } // end optimized d/l

#endif // defined(CMDFIFO) && !defined(NOKNITEXTUREDOWNLOAD) && !defined(WINNT)

     // if we get here, we either have some funny flags set or we
     // have an unhandled PitchAdjust (src or dst), or we're compiled
     // for direct writes, or we're compiling to only use the old copy
     // loop, or we're on NT and don't have the CPU detection in yet
     //
#if defined(K6_2) && defined(CMDFIFO)
         {
            extern BOOL _3DNowAllowed;
            if (!_3DNowAllowed)
            {
#endif
               // bytes per row is a multiple of 4
               for (t = 0; t < height; t++)
               {
                  // first mipmap is formatted into argb8888
                  if (flags & FORMAT8888)
                     txtrformatTo8888(data32, &_D3(buffer)[t*width], bpt,
                                       width, format);
                  else if (flags & FORMAT_ORIGINAL)
                  {
                     // put the final data into the last row
                     data32 = (FxU32 *)_D3(buffer)+ (MAX_TEXTURE_SIZE);

                     // One row at a time filter image down into the same buffer
                     txtrImgHalve(&_D3(buffer)[t*width], width+width,
                                 ((height+height)>1?2:1),
                                 &_D3(buffer)[t*2*(width+width)]);

                     // Reformat texture into original format
                     txtrFormatToOriginal(&_D3(buffer)[t*width], data32,
                                          bpt, width, format);
                  }

                  CMDFIFO_CHECKROOM( cmdFifo, PH5_SIZE + (FxU32)(width*bpt)/4);
                  SETPH( cmdFifo, CMDFIFO_BUILD_PK5((FxU32)(width*bpt)/4, 0, 0,
                        (FxU32)SSTCP_PKT5_TEXPORT));

                  if (!IS_NAPALM)
                  {
                     SETPH( cmdFifo, ((FxU32)&vaddr[0]) );
                  }
                  else
                  {
                     SETPH(cmdFifo, (((FxU32)&vaddr[0])+0x3a00000) & 0x7ffffff);
                  }

                  for (s = 0; s < (width*bpt)/4; ++s)
                  {
                     SETPD( cmdFifo, vaddr[0], *data32 );  // dword
                     ++vaddr;
                     ++data32;
                  }

                  (char *)data32 += srcPitchAdjust;
                  (char *)vaddr  += dstPitchAdjust;

               } // for (t = 0; t < height; t++)
#if defined(K6_2) && defined(CMDFIFO)
            }
            else
            { // 3DNowAllowed
               txtrDownloadK6 (ppdev, height, width, bpt, flags, data32, vaddr,
                                 format, srcPitchAdjust, dstPitchAdjust,
                                 &cmdFifo, &hwIndex);
            } // if 3DNowAllowed
         } // new K6 block
#endif
      } // 4 byte download

   } // linear

   CMDFIFO_EPILOG( cmdFifo );
   return retval;                            // return the next available loc
}

/*-------------------------------------------------------------------
Function Name:  txtrCalcBaseAddress
Description:    Finds the base address of the texture
Return:         long - base address of the texture
-------------------------------------------------------------------*/
long __stdcall txtrCalcBaseAddress(long start, long lodmax, long ar,
                                   long bitsPerTexel, ULONG home,long tbig)
{
   long sum_of_lod_sizes;
   long base_address;

   // Fetch size from table

   if (home == HOME_SPLIT_ODD_TMU1 || home == HOME_SPLIT_ODD_TMU0)
   {
      if (tbig)
         sum_of_lod_sizes = (txtBigMipMapOffsetTsplit[ar][lodmax]*bitsPerTexel) >> 3;
      else
         sum_of_lod_sizes = (txtMipMapOffsetTsplit[ar][lodmax]*bitsPerTexel) >> 3;
   }
   else
   {
      if (tbig)
         sum_of_lod_sizes = (txtBigMipMapOffset[ar][lodmax]*bitsPerTexel) >> 3;
      else
         sum_of_lod_sizes = (txtMipMapOffset[ar][lodmax]*bitsPerTexel) >> 3;
   }

   // Compute base address

   base_address = start - sum_of_lod_sizes;

   // Align base address

   base_address = (base_address + 0xf) & ~0xf;

   return (base_address);
}


/*-------------------------------------------------------------------
Function Name:  txtrCalcBaseAddressDXT1
Description:    Finds the base address of the DXT1 texture
Return:         long - base address of the texture
-------------------------------------------------------------------*/
long __stdcall txtrCalcBaseAddressDXT1(long tmu, long lodmax, TXTRDESC *txtr)
{
   long sum_of_lod_sizes=0;
   long base_address;
   long tlg = txtr->tlog;
   long slg = txtr->slog;
   long maxlg;
   long locallmax;

   // Fetch size from table

   if (tlg > slg)
   {
      maxlg = tlg;
      slg = 8-(tlg-slg);
      tlg = 8;
   }
   else
   {
      maxlg = slg;
      tlg = 8 - (slg-tlg);
      slg = 8;
   }

   if (txtr->home == HOME_SPLIT_ODD_TMU1 || txtr->home == HOME_SPLIT_ODD_TMU0)
   {
//      sum_of_lod_sizes = (txtMipMapOffset[ar][lodmax]*bitsPerTexel) >> 3;
   }
   else
   {
      if (maxlg > 8)
      {
         locallmax = 3-lodmax;
         // we have a "big ass texture"
         while (locallmax-- > 0)
         {
            slg++;
            tlg++;
            sum_of_lod_sizes -= ((1 << slg) * (1 << tlg) >> 1);
         }
      }
      else
      {
         while (lodmax-- > 0)
         {
            // If the DXT1 texture is not at least 8 pixels wide, we need
            // extra space so it can be padded.
            if (slg < 3)
               slg = 3;
            // DXTn textures require a minimum size of 4x4 so no tlg < 2.
            if (tlg < 2)
               tlg = 2;

            sum_of_lod_sizes += ((1 << slg) * (1 << tlg) >> 1);
            tlg--;
            slg--;
         }
      }
   }

   // Compute base address

   base_address = txtr->start[tmu] - sum_of_lod_sizes;

   // Align base address

   base_address = (base_address + 0xf) & ~0xf;

   return (base_address);
}


/*-------------------------------------------------------------------
Function Name:  txtrCalcMemRequired
Description:    Calculates memory requirement for mipmap texture.
Information:    long __stdcall txtrCalcMemRequired(long lodmin, long lodmax, long ar, long bitsPerTexel, unsigned long mipmapsOnTmu)
Return:         Long - Memory requirement



-------------------------------------------------------------------*/
long __stdcall txtrCalcMemRequired(long lodmin, long lodmax, long ar, long bitsPerTexel,
                                   unsigned long mipmapsOnTmu, int TxtrIsBig)
{
   long  sum_of_lod_sizes = 0;
   long   lod;

   for ( lod = lodmin; lod <= lodmax; lod++ )
   {
      if (TxtrIsBig)
      {  // This is the BIG_TEXTURE case.
         if (mipmapsOnTmu & (1 << lod))
         {
            sum_of_lod_sizes += txtBigMipMapSize[ar][lod];
         }

      }
      else if (mipmapsOnTmu & (1 << lod))
      {
         sum_of_lod_sizes += txtMipMapSize[ar][lod];
      }
   }
   sum_of_lod_sizes = (sum_of_lod_sizes * bitsPerTexel) >> 3;
   sum_of_lod_sizes = (sum_of_lod_sizes+15) & ~15; // round up to next 16 bytes
   return sum_of_lod_sizes;
}


/*-------------------------------------------------------------------
Function Name:  txtrCalcMemRequired
Description:    Calculates memory requirement for mipmap texture.
Information:    long __stdcall txtrCalcMemRequired(long lodmin, long lodmax, long ar, long bitsPerTexel, unsigned long mipmapsOnTmu)
Return:         Long - Memory requirement



-------------------------------------------------------------------*/
long __stdcall txtrCalcMemRequiredDXT1(long lodmin, long lodmax, long ar, long bitsPerTexel,
                                       unsigned long mipmapsOnTmu, long slog, long tlog, int TxtrIsBig)
{
   long  sum_of_lod_sizes = 0;
   long   lod;

   for ( lod = lodmin; lod <= lodmax; lod++ )
   {
      if (TxtrIsBig)
      {  // This is the BIG_TEXTURE case.
         if (slog > 2)
         {
            if (tlog > 1)
               sum_of_lod_sizes += txtBigMipMapSize[ar][lod];
            else
               sum_of_lod_sizes += ((1 << slog) * 4);
         }
         else if (tlog > 1)
         {
            if (bitsPerTexel == 4)
               sum_of_lod_sizes += (8 * (1 << tlog));
            else
               sum_of_lod_sizes += (4 * (1 << tlog));
         }
         else
         {
            if (bitsPerTexel == 4)
               sum_of_lod_sizes += (8 * 4);
            else
               sum_of_lod_sizes += (4 * 4);
         }
         slog--;
         tlog--;
      }
      else if (mipmapsOnTmu & (1 << lod))
      {
         if (slog > 2)
         {
            if (tlog > 1)
               sum_of_lod_sizes += txtMipMapSize[ar][lod];
            else
               sum_of_lod_sizes += ((1 << slog) * 4);
         }
         else if (tlog > 1)
         {
            if (bitsPerTexel == 4)
               sum_of_lod_sizes += (8 * (1 << tlog));
            else
               sum_of_lod_sizes += (4 * (1 << tlog));
         }
         else
         {
            if (bitsPerTexel == 4)
               sum_of_lod_sizes += (8 * 4);
            else
               sum_of_lod_sizes += (4 * 4);
         }
         slog--;
         tlog--;
      }
   }

   sum_of_lod_sizes = (sum_of_lod_sizes * bitsPerTexel) >> 3;

   sum_of_lod_sizes = (sum_of_lod_sizes+15) & ~15; // round up to next 16 bytes

   return sum_of_lod_sizes;
}



//------------------------
//
//------------------------
//
/*-------------------------------------------------------------------
Function Name:  TXTRINIT
Description:    Init texture management. This is called from d3init()
Information:    void  __stdcall TXTRINIT(NT9XDEVICEDATA * ppdev)
Return:         void



-------------------------------------------------------------------*/
void  __stdcall TXTRINIT(NT9XDEVICEDATA * ppdev)
{
   int cnt;

   // init texture mip map offset table
   txtMipMapInit();

#ifndef WINNT
   // D3DHalCreateDriver is called twice on NT,
   // if _D3(autoMipMap) is nonzero, we will have allocated
   // this on the first call and to avoid a memory leak
   // we don't want to overwrite the pointer here

   // no buffer yet
   _D3(buffer) = NULL ;
#endif

   // none of the textures are in use
   for (cnt = 0; cnt < (int)MAXTEXTURECOUNT; ++cnt)
      TXTRDESC_PTR(cnt)->flags = 0 ;

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
#else
   // none of the texture handles are in use
   for (cnt = 0; cnt < (int)MAXTEXTUREHANDLE; ++cnt)
      TXTRHNDL_PTR(cnt)->flags = 0 ;
#endif

}


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------
/*-------------------------------------------------------------------
Function Name:  CREATENEWTEXTURE
Description:    Create a new texture that has an aspect ratio no greater than 8
Information:    <See function header>
Return:         unsigned long - largest buffer needed (if successful)
                                NULL - upon failure.
-------------------------------------------------------------------*/
unsigned long * __stdcall CREATENEWTEXTURE(NT9XDEVICEDATA * ppdev,
                                unsigned long *texels,
                                long           slog,
                                long           tlog,
                                long           srcPitch,
                                long           bytesPerTexel,
                                d3Global      *g)
{
   int            src, last, trg, duplicate, cnt, pixel, max_pixels, widthBytes;
   unsigned long *buffer;

   // we need a buffer to create the new texture. Simple, the first
   // time we need a buffer then we allocate one and never release it
   // because the chances are that we will need it again.
   buffer = g->buffer;
   if (! buffer)
   {
      // largest buffer we will ever need for Voodoo3 = worst case = 256x1 -> 256x32
      // g->buffer = buffer = LocalAlloc(0, 256*32*BYTES_PER_16BPP);
      if (IS_NAPALM)
         g->buffer = buffer = (DWORD*)D3DMALLOC(D3D_BUFFER_SIZE_TBIG, &g->buffer);
      else
         g->buffer = buffer = (DWORD*)D3DMALLOC(D3D_BUFFER_SIZE, &g->buffer);
   }
   if (!buffer)
      return NULL;

   // width > height
   if (slog > tlog)
   {
      // each row in src texture
      trg = 0;
      widthBytes = (1 << slog) * bytesPerTexel;
      last =       1 << tlog;
      duplicate = (slog - 3) - tlog ;     // (width/8)/height = NewAspectRatio/old hgt
#ifdef WINNT
      ASSERTDD(duplicate >= 0, "CREATENEWTEXTURE, duplicate < 0, AV likely");
#endif
      duplicate = 1 << duplicate ;

      for (src=0; src < last; ++src)
      {
         // duplicate the row
         for (cnt=0; cnt < duplicate; ++cnt)
         {
            // each pixel
            max_pixels = (widthBytes >> 2); // width / 4 = number DWORDS
            for (pixel = 0; pixel < max_pixels; ++pixel)
            {
               buffer[trg] = texels[(src * (widthBytes >> 2)) + pixel] ;
               ++trg;
            } // each pixel
         } // duplicate
      } // each row
   } // width > height
   else  // height is greater than width
   {
      trg = 0;
      widthBytes = (1 << slog) * bytesPerTexel;
      last =       1 << tlog;
      duplicate = (tlog - 3) - slog ;     // (height/8)/width = NewAspectRatio/old hgt
#ifdef WINNT
      ASSERTDD(duplicate >= 0, "CREATENEWTEXTURE, duplicate < 0, AV likely");
#endif
      duplicate = 1 << duplicate ;
      max_pixels = (1 << slog); // number of texels in width

      // The old code took ABAB textures and made ABABABAB textures instead of
      // AABBAABB textures...  CSR and BCW.

      if (bytesPerTexel == 1)
      {
         char *c_buffer = (char *)buffer ;
         char *c_texels = (char *)texels ;

         for (src=0; src < last; ++src)
         {
            // each pixel in row
            for (cnt=0; cnt < max_pixels; ++cnt)
            {
               // duplicate pixel
               for (pixel = 0; pixel < duplicate; ++pixel)
               {
                  c_buffer[trg] = c_texels[(src  * (srcPitch)) + cnt] ;
                  ++trg;
               } // each pixel
            } // duplicate
         }
      }
      else if (bytesPerTexel==2)
      {
         short *s_buffer = (short *)buffer ;
         short *s_texels = (short *)texels ;

         for (src=0; src < last; ++src)
         {
            // each pixel in row
            for (cnt=0; cnt < max_pixels; ++cnt)
            {
               // duplicate pixel
               for (pixel = 0; pixel < duplicate; ++pixel)
               {
                  s_buffer[trg] = s_texels[(src  * (srcPitch >> 1)) + cnt] ;
                  ++trg;
               } // each pixel
            } // duplicate
         }
      }
      else // 4 byte per texel
      {
         for (src=0; src < last; ++src)
         {
            // each pixel in src
            for (cnt=0; cnt < max_pixels; ++cnt)
            {
               // duplicate pixel
               for (pixel = 0; pixel < duplicate; ++pixel)
               {
                  buffer[trg] = texels[(src  * (srcPitch >> 2)) + cnt] ;
                  ++trg;
               } // each pixel
            } // duplicate
         } // each row
      }
   }

   return(buffer) ;
}

/*-------------------------------------------------------------------
Function Name:  TXTRDOWNLOADPALETTE
Description:    Downloads a texture palette.
Information:    void __stdcall TXTRDOWNLOADPALETTE(NT9XDEVICEDATA * ppdev, unsigned int chip,
                                        LPPALETTEENTRY pal, DWORD type )
                    pal - address of PaletteEntry
                    type - palette type
Return:         VOID
-------------------------------------------------------------------*/
#if( DX >= 6)
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
void __stdcall TXTRDOWNLOADPALETTE(NT9XDEVICEDATA * ppdev, unsigned int chip, PPALHNDL pPalHndl, DWORD type )
#else
void __stdcall TXTRDOWNLOADPALETTE(NT9XDEVICEDATA * ppdev, unsigned int chip, LPPALETTEENTRY pal, DWORD type )
#endif
#else
void __stdcall TXTRDOWNLOADPALETTE(NT9XDEVICEDATA * ppdev, unsigned int chip, LPPALETTEENTRY pal )
#endif
{
   FxU32 i, entry;
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  LPPALETTEENTRY  pal = pPalHndl->ColorTable;
#endif
   CMDFIFO_PROLOG( cmdFifo );

   CMDFIFO_CHECKROOM( cmdFifo, 32 * (PH1_SIZE + 8) );

   // try and burst 8 entry tables at a time (256/8=32)

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
#pragma message(__FILELINE__ "modify TXTRDOWNLOADPALETTE to download partial array")
#endif

#if( DX >= 6 )
   if( type & TEXFMTFLG_PALETTIZED_ALPHA )
   {
      D3DPRINT(D3DDBGLVL+1, "TXTRDOWNLOADPALETTE - palettized alpha texture");
      for( i = 0x80000000, entry = 0; i != 0x00000000; i += 0x04000000, entry += 8 )
      {
         SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 8, 1, nccTable0[4], chip ) );

         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[4],  i | (0 << 24) |
               ((pal[entry + 0].peFlags & 0xFC) << 16) | ((pal[entry + 0].peRed & 0xFC) << 10) |
               ((pal[entry + 0].peGreen & 0xFC) << 4)  | ((pal[entry + 0].peBlue & 0xFC) >> 2) );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[5],  i | (0 << 24) |
               ((pal[entry + 1].peFlags & 0xFC) << 16) | ((pal[entry + 1].peRed & 0xFC) << 10) |
               ((pal[entry + 1].peGreen & 0xFC) << 4)  | ((pal[entry + 1].peBlue & 0xFC) >> 2) );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[6],  i | (1 << 24) |
               ((pal[entry + 2].peFlags & 0xFC) << 16) | ((pal[entry + 2].peRed & 0xFC) << 10) |
               ((pal[entry + 2].peGreen & 0xFC) << 4)  | ((pal[entry + 2].peBlue & 0xFC) >> 2) );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[7],  i | (1 << 24) |
               ((pal[entry + 3].peFlags & 0xFC) << 16) | ((pal[entry + 3].peRed & 0xFC) << 10) |
               ((pal[entry + 3].peGreen & 0xFC) << 4)  | ((pal[entry + 3].peBlue & 0xFC) >> 2) );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[8],  i | (2 << 24) |
               ((pal[entry + 4].peFlags & 0xFC) << 16) | ((pal[entry + 4].peRed & 0xFC) << 10) |
               ((pal[entry + 4].peGreen & 0xFC) << 4)  | ((pal[entry + 4].peBlue & 0xFC) >> 2) );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[9],  i | (2 << 24) |
               ((pal[entry + 5].peFlags & 0xFC) << 16) | ((pal[entry + 5].peRed & 0xFC) << 10) |
               ((pal[entry + 5].peGreen & 0xFC) << 4)  | ((pal[entry + 5].peBlue & 0xFC) >> 2) );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[10], i | (3 << 24) |
               ((pal[entry + 6].peFlags & 0xFC) << 16) | ((pal[entry + 6].peRed & 0xFC) << 10) |
               ((pal[entry + 6].peGreen & 0xFC) << 4)  | ((pal[entry + 6].peBlue & 0xFC) >> 2) );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[11], i | (3 << 24) |
               ((pal[entry + 7].peFlags & 0xFC) << 16) | ((pal[entry + 7].peRed & 0xFC) << 10) |
               ((pal[entry + 7].peGreen & 0xFC) << 4)  | ((pal[entry + 7].peBlue & 0xFC) >> 2) );

         D3DPRINT(D3DDBGLVL+1, "entries %3ld-%3ld  %08lXh  %08lXh  %08lXh  %08lXh  %08lXh  %08lXh  %08lXh  %08lXh",
                  entry, entry+7,
                  (i | (0 << 24) |
                   ((pal[entry + 0].peFlags & 0xFC) << 16) |
                   ((pal[entry + 0].peRed   & 0xFC) << 10) |
                   ((pal[entry + 0].peGreen & 0xFC) <<  4) |
                   ((pal[entry + 0].peBlue  & 0xFC) >>  2)),
                  (i | (0 << 24) |
                   ((pal[entry + 1].peFlags & 0xFC) << 16) |
                   ((pal[entry + 1].peRed   & 0xFC) << 10) |
                   ((pal[entry + 1].peGreen & 0xFC) <<  4) |
                   ((pal[entry + 1].peBlue  & 0xFC) >>  2)),
                  (i | (1 << 24) |
                   ((pal[entry + 2].peFlags & 0xFC) << 16) |
                   ((pal[entry + 2].peRed   & 0xFC) << 10) |
                   ((pal[entry + 2].peGreen & 0xFC) <<  4) |
                   ((pal[entry + 2].peBlue  & 0xFC) >>  2)),
                  (i | (1 << 24) |
                   ((pal[entry + 3].peFlags & 0xFC) << 16) |
                   ((pal[entry + 3].peRed   & 0xFC) << 10) |
                   ((pal[entry + 3].peGreen & 0xFC) <<  4) |
                   ((pal[entry + 3].peBlue  & 0xFC) >>  2)),
                  (i | (2 << 24) |
                   ((pal[entry + 4].peFlags & 0xFC) << 16) |
                   ((pal[entry + 4].peRed   & 0xFC) << 10) |
                   ((pal[entry + 4].peGreen & 0xFC) <<  4) |
                   ((pal[entry + 4].peBlue  & 0xFC) >>  2)),
                  (i | (2 << 24) |
                   ((pal[entry + 5].peFlags & 0xFC) << 16) |
                   ((pal[entry + 5].peRed   & 0xFC) << 10) |
                   ((pal[entry + 5].peGreen & 0xFC) <<  4) |
                   ((pal[entry + 5].peBlue  & 0xFC) >>  2)),
                  (i | (3 << 24) |
                   ((pal[entry + 6].peFlags & 0xFC) << 16) |
                   ((pal[entry + 6].peRed   & 0xFC) << 10) |
                   ((pal[entry + 6].peGreen & 0xFC) <<  4) |
                   ((pal[entry + 6].peBlue  & 0xFC) >>  2)),
                  (i | (3 << 24) |
                   ((pal[entry + 7].peFlags & 0xFC) << 16) |
                   ((pal[entry + 7].peRed   & 0xFC) << 10) |
                   ((pal[entry + 7].peGreen & 0xFC) <<  4) |
                   ((pal[entry + 7].peBlue  & 0xFC) >>  2)));
      }
   }
   else
#endif
   {
      D3DPRINT(D3DDBGLVL+1, "TXTRDOWNLOADPALETTE - palettized texture");
      for( i = 0x80000000, entry = 0; i != 0x00000000; i += 0x04000000, entry += 8 )
      {
         SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 8, 1, nccTable0[4], chip ) );

         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[4],  i | (0 << 24) |
               (pal[entry + 0].peRed << 16) | (pal[entry + 0].peGreen << 8) | pal[entry + 0].peBlue );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[5],  i | (0 << 24) |
               (pal[entry + 1].peRed << 16) | (pal[entry + 1].peGreen << 8) | pal[entry + 1].peBlue );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[6],  i | (1 << 24) |
               (pal[entry + 2].peRed << 16) | (pal[entry + 2].peGreen << 8) | pal[entry + 2].peBlue );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[7],  i | (1 << 24) |
               (pal[entry + 3].peRed << 16) | (pal[entry + 3].peGreen << 8) | pal[entry + 3].peBlue );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[8],  i | (2 << 24) |
               (pal[entry + 4].peRed << 16) | (pal[entry + 4].peGreen << 8) | pal[entry + 4].peBlue );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[9],  i | (2 << 24) |
               (pal[entry + 5].peRed << 16) | (pal[entry + 5].peGreen << 8) | pal[entry + 5].peBlue );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[10], i | (3 << 24) |
               (pal[entry + 6].peRed << 16) | (pal[entry + 6].peGreen << 8) | pal[entry + 6].peBlue );
         SETPD( cmdFifo, SST_CHIP( ghw0, chip )->nccTable0[11], i | (3 << 24) |
               (pal[entry + 7].peRed << 16) | (pal[entry + 7].peGreen << 8) | pal[entry + 7].peBlue );

         D3DPRINT(D3DDBGLVL+1, "entries %3ld-%3ld  %08lXh  %08lXh  %08lXh  %08lXh  %08lXh  %08lXh  %08lXh  %08lXh",
                  entry, entry+7,
                  (i | (0 << 24) |
                   (pal[entry + 0].peRed   << 16) |
                   (pal[entry + 0].peGreen <<  8) |
                   (pal[entry + 0].peBlue       )),
                  (i | (0 << 24) |
                   (pal[entry + 1].peRed   << 16) |
                   (pal[entry + 1].peGreen <<  8) |
                   (pal[entry + 1].peBlue       )),
                  (i | (1 << 24) |
                   (pal[entry + 2].peRed   << 16) |
                   (pal[entry + 2].peGreen <<  8) |
                   (pal[entry + 2].peBlue       )),
                  (i | (1 << 24) |
                   (pal[entry + 3].peRed   << 16) |
                   (pal[entry + 3].peGreen <<  8) |
                   (pal[entry + 3].peBlue       )),
                  (i | (2 << 24) |
                   (pal[entry + 4].peRed   << 16) |
                   (pal[entry + 4].peGreen <<  8) |
                   (pal[entry + 4].peBlue       )),
                  (i | (2 << 24) |
                   (pal[entry + 5].peRed   << 16) |
                   (pal[entry + 5].peGreen <<  8) |
                   (pal[entry + 5].peBlue       )),
                  (i | (3 << 24) |
                   (pal[entry + 6].peRed   << 16) |
                   (pal[entry + 6].peGreen <<  8) |
                   (pal[entry + 6].peBlue       )),
                  (i | (3 << 24) |
                   (pal[entry + 7].peRed   << 16) |
                   (pal[entry + 7].peGreen <<  8) |
                   (pal[entry + 7].peBlue       )));
      }
   }

   // palette is downloaded
   _D3(flags) &= ~PALETTECHANGED;

   CMDFIFO_EPILOG( cmdFifo );
} /* _grTexDownloadPalette */

/*-------------------------------------------------------------------
Function Name:  TXTRCHANGEDPALETTE
Description:    If the current palette that is downloaded into the hardware
                has changed then set a flag to re-download it
Information:    void __stdcall
                TXTRCHANGEDPALETTE(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWPALETTE_GBL paletteGBL)

Return:         VOID



-------------------------------------------------------------------*/
#if !defined(WINNT) || ((DIRECT3D_VERSION >= 0x0700) && (DX >= 7))
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
void __stdcall
TXTRCHANGEDPALETTE(NT9XDEVICEDATA * ppdev, PALHNDL *paletteGBL)
#else
void __stdcall
TXTRCHANGEDPALETTE(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWPALETTE_GBL paletteGBL)
#endif
{
   if ( (void *)paletteGBL == _D3(currentPalette) )
      _D3(flags) |= PALETTECHANGED;
}

/*-------------------------------------------------------------------
Function Name:  TXTRNEWPALETTE
Description:    If the new palette is not downloaded then set a flag download it.
Information:    void __stdcall
                TXTRNEWPALETTE(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWPALETTE_GBL paletteGBL)

Return:         VOID
-------------------------------------------------------------------*/
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
void __stdcall
TXTRNEWPALETTE(NT9XDEVICEDATA * ppdev, PALHNDL *paletteGBL)
#else
void __stdcall
TXTRNEWPALETTE(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWPALETTE_GBL paletteGBL)
#endif
{
   // if the new palette is not downloaded then download it
   if ( (void *)paletteGBL != _D3(currentPalette) )
      _D3(flags) |= PALETTECHANGED;
}
#endif
void stretchDXTn(LPDDRAWI_DDRAWSURFACE_GBL surfGBL, TXTRDESC *txtr, d3Global *g, NT9XDEVICEDATA *ppdev)
{
   DWORD *buffer = (DWORD *)g->buffer;
#ifdef WINNT
   DWORD src = (DWORD)(surfGBL->fpVidMem + ppdev->pjLfbBase);
   DWORD dst = (DWORD)(surfGBL->fpVidMem + ppdev->pjLfbBase);
#else
   DWORD src = (DWORD)surfGBL->fpVidMem;
   DWORD dst = (DWORD)surfGBL->fpVidMem;
#endif
   int   ssize,swidth,dwidth,sheight,dheight;
   int   i,j,k,l,m;
   int   cslg=0,ctlg=0,level;
   int   moved;
   int   stretchby;
   DXT_COLOR   *dbuffer,*sbuffer;
   void  (*DecodeBlock) (void *,void *);
   void  (*EncodeBlock) (void *,void *);


   // compute the log2 for width and height
   while (((surfGBL->wWidth - (0x01 << cslg)) != 0)
         &&  (cslg < MAX_BIGTEXTURE_LOG) )
      ++cslg ;

   while (((surfGBL->wHeight - (0x01 << ctlg)) != 0)
         &&  (ctlg < MAX_BIGTEXTURE_LOG) )
      ++ctlg ;


   if(!buffer)
   {
      if (IS_NAPALM)
         g->buffer = buffer = (DWORD*)D3DMALLOC(D3D_BUFFER_SIZE_TBIG, &g->buffer);
      else
         g->buffer = buffer = (DWORD*)D3DMALLOC(D3D_BUFFER_SIZE, &g->buffer);

   }

   if (!buffer)
      return;

   // Setup function pointers for the type of compression to be used.
   if (surfGBL->ddpfSurface.dwFourCC == FOURCC_DXT1)
   {
      ssize = sizeof(DXTBlockRGB);
      EncodeBlock = EncodeBlockRGB;
      DecodeBlock = DecodeBlockRGB;
   }
   else if (surfGBL->ddpfSurface.dwFourCC == FOURCC_DXT2 || surfGBL->ddpfSurface.dwFourCC == FOURCC_DXT3)
   {
      ssize = sizeof(DXTBlockAlpha4);
      EncodeBlock = EncodeBlockAlpha4;
      DecodeBlock = DecodeBlockAlpha4;
   }
   else if(surfGBL->ddpfSurface.dwFourCC == FOURCC_DXT4 || surfGBL->ddpfSurface.dwFourCC == FOURCC_DXT5)
   {
      ssize = sizeof(DXTBlockAlpha3);
      EncodeBlock = EncodeBlockAlpha3;
      DecodeBlock = DecodeBlockAlpha3;
   }

   level = txtr->initialSlog - cslg;

   swidth = 1 << (txtr->initialSlog - level);
   dwidth = 1 << (txtr->slog - level);
   sheight = 1 << (txtr->initialTlog - level);
   dheight = 1 << (txtr->tlog - level);

   // Compute the pointers to the compressed texture.
   dst = dst + (((dheight >> 2) * (dwidth >> 2)) * ssize);
   src = src + (((sheight >> 2) * (swidth >> 2)) * ssize);

   if (txtr->initialSlog < txtr->slog)
   {  // We are stretching in S.

      stretchby = (1 << (txtr->slog - txtr->initialSlog));
      for (l=0; l < ((swidth>>2) * (sheight>>2));l++)
      {
         dst -= (stretchby * ssize);
         src -= ssize;

         DecodeBlock((void*)src,buffer);
         for (k=0; k < 4; k++)
         {
            sbuffer = &((DXT_COLOR*)buffer)[(4*k)];
            dbuffer = &((DXT_COLOR*)buffer)[DXT_BLOCK_PIXELS+(4*k)];
            moved=0;
            for (j=0; j < 4; j++)
            {
               for (i=0;i < stretchby; i++)
               {
                  if (moved == 0x4) // Move to the next tile if needed.
                  {
                     dbuffer += DXT_BLOCK_PIXELS;
                     moved = 0;
                  }
                  dbuffer[moved] = sbuffer[j];
                  moved++;
               }
            }
         }
         sbuffer = &((DXT_COLOR*)buffer)[DXT_BLOCK_PIXELS];
         for (i=0;i < stretchby;i++)
         {
            EncodeBlock(&sbuffer[DXT_BLOCK_PIXELS*i],(void*)(dst+(ssize*i)));
         }
      }
   }
   else if (txtr->initialTlog < txtr->tlog)
   {  // We are stretching in T.
      stretchby = (1 << (txtr->tlog - txtr->initialTlog));

      dst = dst - (((dwidth >> 2) * (stretchby-1)) * ssize);
      for (m=0; m < (sheight >> 2); m++)
      {
         for (l=0; l < (swidth >> 2);l++)
         {
            dst -= ssize;
            src -= ssize;

            DecodeBlock((void*)src,buffer);
            for (k=0; k < 4; k++)
            {
               sbuffer = &((DXT_COLOR*)buffer)[k];
               dbuffer = &((DXT_COLOR*)buffer)[DXT_BLOCK_PIXELS+k];
               moved=0;
               for (j=0; j < 4; j++)
               {
                  for (i=0;i < stretchby; i++)
                  {
                     if (moved == 0x4) // Move to the next tile if needed.
                     {
                        dbuffer += DXT_BLOCK_PIXELS;
                        moved = 0;
                     }
                     dbuffer[moved*4] = sbuffer[(j*4)];
                     moved++;
                  }
               }
            }
            sbuffer = &((DXT_COLOR*)buffer)[DXT_BLOCK_PIXELS];
            for (i=0;i < stretchby;i++)
            {
               EncodeBlock(&sbuffer[DXT_BLOCK_PIXELS*i],(void*)(dst+(ssize*i*(dwidth >> 2))));
            }
         }
         // Compute the pointers to the compressed texture.
         dst = dst - (((dwidth >> 2) * (stretchby-1)) * ssize);
      }
   }
}

void shrinkDXTn(LPDDRAWI_DDRAWSURFACE_GBL surfGBL, TXTRDESC *txtr, d3Global *g, NT9XDEVICEDATA * ppdev)
{
   DWORD *buffer = (DWORD *)g->buffer;
#ifdef WINNT
   DWORD *src = (DWORD*)(surfGBL->fpVidMem + ppdev->pjLfbBase);
   DWORD *dst = (DWORD*)(surfGBL->fpVidMem + ppdev->pjLfbBase);
#else
   DWORD *src = (DWORD*)surfGBL->fpVidMem;
   DWORD *dst = (DWORD*)surfGBL->fpVidMem;
#endif
   int   ssize,swidth,dwidth,sheight,dheight,dcnt;
   int   i,j,k,m;
   int   shrinkby;
   int   cslg=0,ctlg=0,level;
   void  (*DecodeBlock) (void *,void *);
   void  (*EncodeBlock) (void *,void *);
   DXT_COLOR   *dbuffer,*sbuffer;

   // compute the log2 for width and height
   while (((surfGBL->wWidth - (0x01 << cslg)) != 0)
         &&  (cslg < MAX_BIGTEXTURE_LOG) )
      ++cslg ;

   while (((surfGBL->wHeight - (0x01 << ctlg)) != 0)
         &&  (ctlg < MAX_BIGTEXTURE_LOG) )
      ++ctlg ;

   if(!buffer)
   {
      if (IS_NAPALM)
         g->buffer = buffer = (DWORD*)D3DMALLOC(D3D_BUFFER_SIZE_TBIG, &g->buffer);
      else
         g->buffer = buffer = (DWORD*)D3DMALLOC(D3D_BUFFER_SIZE, &g->buffer);

   }

   if (!buffer)
      return;

   // Setup function pointers for the type of compression to be used.
   if (surfGBL->ddpfSurface.dwFourCC == FOURCC_DXT1)
   {
      ssize = sizeof(DXTBlockRGB);
      EncodeBlock = EncodeBlockRGB;
      DecodeBlock = DecodeBlockRGB;
   }
   else if (surfGBL->ddpfSurface.dwFourCC == FOURCC_DXT2 || surfGBL->ddpfSurface.dwFourCC == FOURCC_DXT3)
   {
      ssize = sizeof(DXTBlockAlpha4);
      EncodeBlock = EncodeBlockAlpha4;
      DecodeBlock = DecodeBlockAlpha4;
   }
   else if(surfGBL->ddpfSurface.dwFourCC == FOURCC_DXT4 || surfGBL->ddpfSurface.dwFourCC == FOURCC_DXT5)
   {
      ssize = sizeof(DXTBlockAlpha3);
      EncodeBlock = EncodeBlockAlpha3;
      DecodeBlock = DecodeBlockAlpha3;
   }

   level = txtr->initialSlog - cslg;

   dwidth = 1 << (txtr->initialSlog - level);
   swidth = 1 << (txtr->slog - level);
   dheight = 1 << (txtr->initialTlog - level);
   sheight = 1 << (txtr->tlog - level);
   dcnt = (dwidth >> 2) * (dheight >> 2);

   if (txtr->initialSlog < txtr->slog)
   {  // We are shrinking in S.

      shrinkby = (1 << (txtr->slog - txtr->initialSlog));

      for (j=0;j < dcnt;j++)
      {
         sbuffer = &((DXT_COLOR*)buffer)[DXT_BLOCK_PIXELS];
         dbuffer = (DXT_COLOR*)buffer;
         for (i=0;i < shrinkby; i++)
         {
            DecodeBlock(src,&sbuffer[DXT_BLOCK_PIXELS*i]);
            src += ssize;
         }
         for(i=0;i < 4;i++)
         {
            m = (((i*shrinkby) >> 2) * DXT_BLOCK_PIXELS) + i;
            dbuffer[(4*0)+i] = sbuffer[(4*0)+m];
            dbuffer[(4*1)+i] = sbuffer[(4*1)+m];
            dbuffer[(4*2)+i] = sbuffer[(4*2)+m];
            dbuffer[(4*3)+i] = sbuffer[(4*3)+m];
         }
         EncodeBlock(dbuffer,dst);
         dst += ssize;
      }
   }
   else if (txtr->initialTlog < txtr->tlog)
   {  // We are shrinking in T.
      shrinkby = (1 << (txtr->tlog - txtr->initialTlog));

      for (j=0;j < (dheight >> 2);j++)
      {
         for (k=0;k < (dwidth >> 2);k++)
         {
            sbuffer = &((DXT_COLOR*)buffer)[DXT_BLOCK_PIXELS];
            dbuffer = (DXT_COLOR*)buffer;
            for (i=0;i < shrinkby;i++)
            {
               DecodeBlock(src+(i*ssize*(swidth>>2)),&sbuffer[DXT_BLOCK_PIXELS*i]);
            }
            for (i=0;i < 4;i++)
            {
               m = (((i*shrinkby) >> 2) * DXT_BLOCK_PIXELS) + (i*4);
               dbuffer[(i*4)+0] = sbuffer[m+0];
               dbuffer[(i*4)+1] = sbuffer[m+1];
               dbuffer[(i*4)+2] = sbuffer[m+2];
               dbuffer[(i*4)+3] = sbuffer[m+3];
            }
            EncodeBlock(dbuffer,dst);
            dst += ssize;
         }
         src+= (ssize*(shrinkby-1)*(swidth>>2));
      }

   }
}

#define FXT_BLOCK_PIXELS 32
void stretchFXT1(LPDDRAWI_DDRAWSURFACE_GBL surfGBL, TXTRDESC *txtr, d3Global *g, NT9XDEVICEDATA *ppdev)
{
   DWORD       *buffer = (DWORD *)g->buffer;
#ifdef WINNT
   DWORD       src = (DWORD)(surfGBL->fpVidMem + ppdev->pjLfbBase);
   DWORD       dst = (DWORD)(surfGBL->fpVidMem + ppdev->pjLfbBase);
#else
   DWORD       src = (DWORD)surfGBL->fpVidMem;
   DWORD       dst = (DWORD)surfGBL->fpVidMem;
#endif
   int         swidth,dwidth,sheight,dheight;
   int         i,j,k,l,m;
   int         cslg=0,ctlg=0,level;
   int         moved;
   int         stretchby;
   DXT_COLOR   *dbuffer,*sbuffer;


   // compute the log2 for width and height
   while (((surfGBL->wWidth - (0x01 << cslg)) != 0)
         &&  (cslg < MAX_BIGTEXTURE_LOG) )
      ++cslg ;

   while (((surfGBL->wHeight - (0x01 << ctlg)) != 0)
         &&  (ctlg < MAX_BIGTEXTURE_LOG) )
      ++ctlg ;


   if(!buffer)
   {
      if (IS_NAPALM)
         g->buffer = buffer = (DWORD*)D3DMALLOC(D3D_BUFFER_SIZE_TBIG, &g->buffer);
      else
         g->buffer = buffer = (DWORD*)D3DMALLOC(D3D_BUFFER_SIZE, &g->buffer);

   }

   if (!buffer)
      return;

   level = txtr->initialSlog - cslg;

   swidth = 1 << (txtr->initialSlog - level);
   dwidth = 1 << (txtr->slog - level);
   sheight = 1 << (txtr->initialTlog - level);
   dheight = 1 << (txtr->tlog - level);

   // Compute the pointers to the compressed texture.
   dst = dst + (((dheight >> 2) * (dwidth >> 3)) * 16);
   src = src + (((sheight >> 2) * (swidth >> 3)) * 16);

   if (txtr->initialSlog < txtr->slog)
   {  // We are stretching in S.

      stretchby = (1 << (txtr->slog - txtr->initialSlog));
      for (l=0; l < ((swidth>>3) * (sheight>>2));l++)
      {
         dst -= (stretchby * 16);
         src -= 16;

         decode4bpp_block((void*)src,
                          &buffer[0],
                          &buffer[8],
                          &buffer[16],
                          &buffer[24]);
         for (k=0; k < 4; k++) // Y counter
         {
            sbuffer = &((DXT_COLOR*)buffer)[(8*k)];
            dbuffer = &((DXT_COLOR*)buffer)[FXT_BLOCK_PIXELS+(8*k)];
            moved=0;
            for (j=0; j < 8; j++) // X counter
            {
               for (i=0;i < stretchby; i++)
               {
                  if (moved == 0x8) // Move to the next tile if needed.
                  {
                     dbuffer += FXT_BLOCK_PIXELS;
                     moved = 0;
                  }
                  dbuffer[moved] = sbuffer[j];
                  moved++;
               }
            }
         }
         dbuffer = &((DXT_COLOR*)buffer)[FXT_BLOCK_PIXELS];
         for (i=0;i < stretchby;i++)
         {
            encode4bpp_block((int*)&dbuffer[FXT_BLOCK_PIXELS*i],
                             (int*)&dbuffer[FXT_BLOCK_PIXELS*i+8],
                             (int*)&dbuffer[FXT_BLOCK_PIXELS*i+16],
                             (int*)&dbuffer[FXT_BLOCK_PIXELS*i+24],
                             (void*)(dst+(i*16)));
         }
      }
   }
   else if (txtr->initialTlog < txtr->tlog)
   {  // We are stretching in T.
      stretchby = (1 << (txtr->tlog - txtr->initialTlog));

      dst = dst - (((dwidth >> 3) * (stretchby-1)) * 16);
      for (m=0; m < (sheight >> 2); m++)
      {
         for (l=0; l < (swidth >> 3);l++)
         {
            dst -= 16;
            src -= 16;

            decode4bpp_block((void*)src,
                             (int*)&buffer[0],
                             (int*)&buffer[8],
                             (int*)&buffer[16],
                             (int*)&buffer[24]);
            for (k=0; k < 8; k++) // X axis
            {
               sbuffer = &((DXT_COLOR*)buffer)[k];
               dbuffer = &((DXT_COLOR*)buffer)[FXT_BLOCK_PIXELS+k];
               moved=0;
               for (j=0; j < 4; j++)  // Y axis
               {
                  for (i=0;i < stretchby; i++)
                  {
                     if (moved == 0x4) // Move to the next tile if needed.
                     {
                        dbuffer += FXT_BLOCK_PIXELS;
                        moved = 0;
                     }
                     dbuffer[moved*8] = sbuffer[(j*8)];
                     moved++;
                  }
               }
            }
            sbuffer = &((DXT_COLOR*)buffer)[FXT_BLOCK_PIXELS];
            for (i=0;i < stretchby;i++)
            {
               encode4bpp_block((int*)&sbuffer[FXT_BLOCK_PIXELS*i],
                                (int*)&sbuffer[FXT_BLOCK_PIXELS*i+8],
                                (int*)&sbuffer[FXT_BLOCK_PIXELS*i+16],
                                (int*)&sbuffer[FXT_BLOCK_PIXELS*i+24],
                                (void*)(dst+(16*i*(dwidth >> 3))));
            }
         }
         // Compute the pointers to the compressed texture.
         dst = dst - (((dwidth >> 3) * (stretchby-1)) * 16);
      }
   }
}

void shrinkFXT1(LPDDRAWI_DDRAWSURFACE_GBL surfGBL, TXTRDESC *txtr, d3Global *g, NT9XDEVICEDATA *ppdev)
{
   DXT_COLOR   *dbuffer,*sbuffer;
   DWORD       *buffer = (DWORD *)g->buffer;
#ifdef WINNT
   int         *src = (DWORD*)(surfGBL->fpVidMem + ppdev->pjLfbBase);
   int         *dst = (DWORD*)(surfGBL->fpVidMem + ppdev->pjLfbBase);
#else
   int         *src = (DWORD*)surfGBL->fpVidMem;
   int         *dst = (DWORD*)surfGBL->fpVidMem;
#endif
   int         swidth,dwidth,sheight,dheight,dcnt;
   int         i,j,k,m;
   int         shrinkby;
   int         cslg=0,ctlg=0,level;

   // compute the log2 for width and height
   while (((surfGBL->wWidth - (0x01 << cslg)) != 0)
         &&  (cslg < MAX_BIGTEXTURE_LOG) )
      ++cslg ;

   while (((surfGBL->wHeight - (0x01 << ctlg)) != 0)
         &&  (ctlg < MAX_BIGTEXTURE_LOG) )
      ++ctlg ;

   if(!buffer)
   {
      if (IS_NAPALM)
         g->buffer = buffer = (DWORD*)D3DMALLOC(D3D_BUFFER_SIZE_TBIG, &g->buffer);
      else
         g->buffer = buffer = (DWORD*)D3DMALLOC(D3D_BUFFER_SIZE, &g->buffer);

   }

   if (!buffer)
      return;


   level = txtr->initialSlog - cslg;

   dwidth = 1 << (txtr->initialSlog - level);
   swidth = 1 << (txtr->slog - level);
   dheight = 1 << (txtr->initialTlog - level);
   sheight = 1 << (txtr->tlog - level);
   dcnt = (dwidth >> 3) * (dheight >> 2);

   if (txtr->initialSlog < txtr->slog)
   {  // We are shrinking in S.
      shrinkby = (1 << (txtr->slog - txtr->initialSlog));
      for (j=0;j < dcnt;j++)
      {
         sbuffer = &((DXT_COLOR*)buffer)[FXT_BLOCK_PIXELS];
         dbuffer = (DXT_COLOR*)buffer;
         for (i=0;i < shrinkby; i++)
         {
            decode4bpp_block(src,(int*)&sbuffer[FXT_BLOCK_PIXELS*i],
                             (int*)&sbuffer[FXT_BLOCK_PIXELS*i+8],
                             (int*)&sbuffer[FXT_BLOCK_PIXELS*i+16],
                             (int*)&sbuffer[FXT_BLOCK_PIXELS*i+24]);
            src +=(FXT_BLOCK_PIXELS >> 1);
         }
         for(i=0;i < 8;i++)
         {
            m = (((i*shrinkby) >> 2) * FXT_BLOCK_PIXELS) + i;
            dbuffer[(8*0)+i] = sbuffer[(8*0)+m];
            dbuffer[(8*1)+i] = sbuffer[(8*1)+m];
            dbuffer[(8*2)+i] = sbuffer[(8*2)+m];
            dbuffer[(8*3)+i] = sbuffer[(8*3)+m];
         }
         encode4bpp_block((int*)dbuffer,
                          (int*)&dbuffer[8],
                          (int*)&dbuffer[16],
                          (int*)&dbuffer[24],
                          dst);
         dst += (FXT_BLOCK_PIXELS >> 1);
      }
   }
   else if (txtr->initialTlog < txtr->tlog)
   {  // We are shrinking in T.
      shrinkby = (1 << (txtr->tlog - txtr->initialTlog));
      for (j=0;j < (dheight >> 2);j++)
      {
         for (k=0;k < (dwidth >> 3);k++)
         {
            sbuffer = &((DXT_COLOR*)buffer)[FXT_BLOCK_PIXELS];
            dbuffer = (DXT_COLOR*)buffer;
            for (i=0;i < shrinkby;i++)
            {
               decode4bpp_block(src+(i*16*(swidth>>3)),
                                (int*)&sbuffer[FXT_BLOCK_PIXELS*i],
                                (int*)&sbuffer[FXT_BLOCK_PIXELS*i+8],
                                (int*)&sbuffer[FXT_BLOCK_PIXELS*i+16],
                                (int*)&sbuffer[FXT_BLOCK_PIXELS*i+24]);
            }
            for (i=0;i < 4;i++)
            {
               m = (((i*shrinkby) >> 2) * FXT_BLOCK_PIXELS) + (i*8);
               dbuffer[(i*8)+0] = sbuffer[m+0];
               dbuffer[(i*8)+1] = sbuffer[m+1];
               dbuffer[(i*8)+2] = sbuffer[m+2];
               dbuffer[(i*8)+3] = sbuffer[m+3];
               dbuffer[(i*8)+4] = sbuffer[m+4];
               dbuffer[(i*8)+5] = sbuffer[m+5];
               dbuffer[(i*8)+6] = sbuffer[m+6];
               dbuffer[(i*8)+7] = sbuffer[m+7];
            }
            encode4bpp_block((int*)&dbuffer[0],
                             (int*)&dbuffer[8],
                             (int*)&dbuffer[16],
                             (int*)&dbuffer[24],
                             dst);
            dst += (FXT_BLOCK_PIXELS >> 1);
         }
         src+= ((FXT_BLOCK_PIXELS >> 1)*(shrinkby-1)*(swidth>>3));
      }
   }
}

