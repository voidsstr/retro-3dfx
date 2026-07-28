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
** File name: d3init.c
**
** Description: define d3d callbacks, capability structure, texture formats.
**              Initialization of the 3D portion of the DDraw HAL.
**
** $Revision: 42$
** $Date: 10/31/00 2:02:21 AM$
**
**
** $Log: 
**  42   3dfx      1.32.2.3.1.410/31/00 Johnny Trainor  Removed  D3DFVFCAPS_PSIZE
**       cap.
**  41   3dfx      1.32.2.3.1.310/25/00 Johnny Trainor  Text formating changes.
**  40   3dfx      1.32.2.3.1.210/20/00 Johnny Trainor  Changed the texture format
**       reporting code. Added two new functions,  BuildDX7TextureFormats() and
**       BuildDX8TextureFormats().
**  39   3dfx      1.32.2.3.1.110/11/00 Brent           Forced check in to enforce
**       branching.
**  38   3dfx      1.32.2.3.1.008/11/00 Russ Lind       merge of various w9x
**       changes into w2k
**       including
**         LODBIASPERCHIP
**         LODDITHER
**         MAXPENDINGBUFFERS
**         TEXTURE_REPEAT_NOT_SCALED
**         COLUMNBANDCONTROL_DEFAULT
**         SLI_ABOVE_1280
**         fix for z buffer problems in Shadow of the Empire
**         flat shaded specular fix
**         part of the Daytona/Napalm2 changes
**  37   3dfx      1.32.2.3    06/14/00 James Hunter    Removed
**       D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT from Voodoo 3 specific init code.
**  36   3dfx      1.32.2.2    06/06/00 Russ Lind       merge from w9x, mods to 4
**       chip support
**  35   3dfx      1.32.2.1    06/02/00 Russ Lind       modified dynamic memory
**       allocation code so MEMCHECK tracking can be enabled
**  34   3dfx      1.32.2.0    05/19/00 Russ Lind       merge from w9x, clamp
**       LodBias setting read from registry
**  33   3dfx      1.32        04/25/00 Sam Hanna       Changed fogGenerateLinear()
**       to point to fogGenerateLinear_Orig() instead of fogGenerateLinear_K620()
**       when adjusting 3dNow pointers. This fixes PRS 13517.
**  32   3dfx      1.31        04/19/00 Matt McClure    Implementation of Z Buffer
**       Access Optimization.  Added code to support setup and reset of
**       optimization based on the amount of flips that have occurred without a Z
**       Buffer Clear.
**  31   3dfx      1.30        04/07/00 Andrew Sobczyk  Init new global variables
**       for 2-chip 2-sample AA
**  30   3dfx      1.29        04/03/00 Edwin Wong      Fix PRS 13607 by removing
**       support for 8bit alpha + 8bit palettized texture format.
**  29   3dfx      1.28        03/23/00 Matt McClure    Modified Column Band
**       Control to 2, and 2PPC Band Height to 2.  This bought us 6 MTexels/s in
**       3DMark 2000's fill rate test Single Textured
**  28   3dfx      1.27        03/20/00 Bob Seitsinger  Added code in
**       D3DHalCreateDriver to capture SSTH3_LOD_BIAS registry value.
**  27   3dfx      1.26        03/17/00 Bob Seitsinger  Added compile and run-time
**       informational messages for the #define's that are set.
**  26   3dfx      1.25        03/13/00 Bob Seitsinger  Added code to handle 2 new
**       registry keys. SSTH3_AAJITTER_FORCEFLAG and SSTH3_DITHMAT_FORCEFLAG.
**  25   3dfx      1.24        03/08/00 Russ Lind       reenable the palettized
**       texture formats for napalm on w2k
**  24   3dfx      1.23        02/25/00 Russ Lind       disable palettized texture
**       formats on napalm for w2k (temporarily, hopefully)
**  23   3dfx      1.22        02/22/00 Leo Galway      Fix for WHQL DCT 300
**       Direct3D Texture Formats - Verify on Voodoo3.
**       Decrements the global number of texture formats, rather than setting
**       format to EmptyFormat,  when culling unsupported  formats in
**       D3DHalCreateDriver(). Ported from Win2K.
**  22   3dfx      1.21        02/04/00 Steve Houston   Added new asm triangle
**       routines by Matt McClure and Allen Hanson to W2K driver.
**       Added NEWASMTRI, STBKNI and K6_2 defines to W2K\build\stbperf.inc to
**       enable this code.
** 
**  21   3dfx      1.20        01/25/00 Russ Lind       fix for Texture Formats -
**       Verify failure on napalm, compressed textures apparently aren't supposed
**       to have a bit depth in the texture format
**  20   3dfx      1.19        01/24/00 Matt McClure    Modified Triangle
**       re-direction pointers to support new 3DNow and KNI triangle rendering
**       routines.  Ifdef'd NEWASMTRI==1
**  19   3dfx      1.18        01/11/00 Scott Kephart   Reverted back to source
**       prior to new triangle assembly code
**  18   3dfx      1.17        01/10/00 Matt McClure    Added support for new
**       assembler triangle routines, #ifdef NEWASMTRI==1 surrounds all changes.
**  17   3dfx      1.16        12/16/99 Russ Lind       reenable
**       D3DPTEXTURECAPS_TRANSPARENCY for napalm, this is a required cap for whql
**  16   3dfx      1.15        12/13/99 Scott Kephart   Big T&L Update:
**       1. Improved T&L profiling code
**       2. Optimizations to scalar transformation and lighting code
**       3. Changes to the vertex buffer code to allow functionality under Windows
**       2000.
**  15   3dfx      1.14        12/02/99 Matt McClure    Fix for PRS 11538,
**       corruption of FPS counter on 3DWinbenc 2k on using Napalm TOT on a Voodoo
**       3.  The "Alpha + Palettized texture - 16Bit AP" format is not supported by
**       the Voodoo 3 hardware, so we now special case it.  Also moved the format
**       descriptor towards the Napalm specific formats in the texture format
**       descriptor table.
**  14   3dfx      1.13        12/01/99 Bob Seitsinger  Changes to apply default
**       jitter values (as defined by Gary T.) appropriately.
**  13   3dfx      1.12        11/15/99 Scott Kephart   Added initial vertex buffer
**       support (more changes to follow)
**       Added support for Intel C Compiler for T&L code
**  12   3dfx      1.11        11/09/99 Bob Seitsinger  Set triangle/line antialias
**       raster cap correctly. If Napalm - sort INdependent, otherwise sort
**       DEpendent. 
**  11   3dfx      1.10        11/04/99 Chris W. Shaw   Fix for PRS 11245.  Take
**       out Tblend caps for triangles (in addition to lines).
**  10   3dfx      1.9         10/28/99 Christopher Wilcox Hardware definition
**       changes to merge divergent h3defs.h.
**  9    3dfx      1.8         10/28/99 Chris W. Shaw   Removed the obsolete
**       renderstate, TextureMapBlend, for dx7 ( Still used for dx6- and still used
**       for WINNT.)
**       Fix for part 1 of PRS 10855 (more obsolete renderstates need to be removed
**       in addtion to this).
**       I have added a makefile define, REMOVE_OBSOLETE1_FOR_DX7_WIN9X, that can
**       be used for easily taking out these changes (for debug and in case other
**       problems arise).
** 
**  8    3dfx      1.7         10/27/99 Russ Lind       fix for dct250 Texture
**       Formats - Verify failure
**       The W2K DDK documenation for DDPIXELFORMAT indicates that for alpha only
**       formats, DDPF_ALPHA should be set rather than DDPF_ALPHAPIXELS
**  7    3dfx      1.6         10/27/99 Bob Seitsinger  Additional rotated dither
**       matrix changes.
**  6    3dfx      1.5         10/26/99 Scott Kephart   Added initial support for
**       software T&L HAL
**  5    3dfx      1.4         10/20/99 Bob Seitsinger  Rotated dither matrix
**       selection fogMode register values clean up.
**  4    3dfx      1.3         09/24/99 Chris W. Shaw   Take out
**       D3DPTEXTURECAPS_TRANSPARENCY for Napalm;  Napalm doesn't support texture
**       chroma key.
**  3    3dfx      1.2         09/17/99 Tim Little      Initial changes for support
**       for FXT1 textures, nothing done to handle > 8:1 aspect textures currently.
**  2    3dfx      1.1         09/16/99 Bob Seitsinger  Dither matrix rotate
**       changes. D3DHalCreateDriver - get DITHER_ROTATE registry value, get dither
**       matrix rotate selection registry values.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 1     11/12/99 4:27p Skephart
** 
** 23    9/09/99 3:28p Cshaw
** D3DPTBLENDCAPS_MODULATEMASK  and D3DPTBLENDCAPS_DECALMASK  removed for
** PRS #8369 - hw does not support this.
** 
** 22    9/01/99 11:43a Cwilcox
** Added 24bpp zbuffer support, when 32bpp rendering is enabled.
** 
** 21    8/20/99 4:53p Cshaw
** Modified the _D3(twoppc) to contain the bitfield for enabling 2ppc.
** 
** 20    8/20/99 3:55p Bseitsin
** Remove #include d3aa.h.
** 
** 19    8/10/99 10:37a Russ
** for DX7, disable call to HNDL_INIT_ARRAY
**
** 18    8/05/99 4:15p Russ
** for RC_LINKED_LIST implementation, disable call to CONTEXT_INIT_ARRAY
**
** 17    7/29/99 5:17p Bseitsin
** Remove SrcAlphaSaturate as a default capability. Include it at run time
** if IS_NAPALM.
**
** 16    7/27/99 4:27p Bseitsin
** Antialiasing changes.
**
** 15    7/27/99 4:02p Tlittle
** Fixes for > 8:1 aspect ratio textures that are large.
**
** 14    7/21/99 3:17p Cshaw
** Added 2ppc tuning via registry.
**
** 13    7/21/99 3:00p Bseitsin
** Enable new alpha blending modes - src*src, dst*dst and [src/dst] *
** dstalpha.
**
** 12    7/19/99 10:26a Bseitsin
** Remove temporary hack of if (1) in D3DHALCreateDriver when reading
** registry values.
**
** 11    7/19/99 10:20a Bseitsin
** Added code to capture Antialiasing Jitter values, and the base code to
** capture Dither Matrix values, from the regsitry.
**
** 10    7/16/99 6:14p Bseitsin
** Changes to capture new registry key values.
**
** 9     7/12/99 1:10p Russ
** modify V3 vs. Napalm texture format reporting policy for W2K (don't
** stomp on Napalm formats when running on V3, just report that there are
** fewer formats in the array)
**
** 8     7/08/99 10:19a Tlittle
** changes for run-time checking of H5
**
** 7     6/24/99 1:51p Russ
** DX7 palettized texture changes (#ifdef'd for DX >= 7)
** enable palettized texture format for DX7 builds on NT
** wrapped DXTC texture formats in #if 1/#endif
**
** 6     6/21/99 5:23p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 5     6/21/99 12:10p Andrew
** Removed HAL_HW
**
** 4     6/21/99 1:54p Tlittle
** Enabled 8bit alpha only textures and 8-8 alpha/palletized textures
**
** 3     6/14/99 3:17p Bseitsin
** Triangle Iterator Column Band Control support.
**
** 2     6/11/99 12:39p Russ
** initial changes for DX7
**
** 1     6/02/99 6:41a Michael
** Branch from H3
**
** 77    5/30/99 5:38p Edwin
** Remove ifdef MM, multi-monitor support is always enabled.
**
** 76    5/26/99 11:00a Cwilcox
** Removed D3DSTATIC_ARRAY code.
**
** 75    5/24/99 5:04p Bseitsin
** Removal of Antialiasing code.
**
** 74    5/12/99 9:27a Tlittle
** Addes DXTn texture exports for H5
**
** 73    5/10/99 2:18p Adrians
** Forgot to add the '|' for the previous checkin.
**
** 72    5/10/99 1:50p Adrians
** Enable the D3DPMISCCAPS_MASKZ caps bit because we already support
** ZWRITEENABLE in the driver.
**
** 71    4/29/99 3:56p Stb_bseitsin
** 32bit rendering and 32bit texture changes.
**
** 70    4/22/99 3:15p Cwilcox
** Removed declaration and all references to fbiMemorySize.
**
** 69    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
**
** 68    4/01/99 1:39p Stb_echilds
** Fix for PRS 5177. Removed the getKatmai function call because it did
** not prevent execution of Katmai instructions in Windows95.
**
** 67    2/11/99 8:30a Stb_skephart
**
** 66    1/29/99 10:48a Cshaw
** Added unified headers.
**
** 65    1/12/99 8:04a Cshaw
** Added runtime switches to the performance analysis code (use SIce to
** modify).
**
** 64    12/16/98 4:03p Sreid
** K6 merge - 12-16-98 drop from contractor
**
** 63    12/09/98 6:38p Sreid
** Merged latest K6 optimization code from Metabyte
**
** 62    12/09/98 7:25a Russ
** NT5 D3D changes for Banshee
**
** 61    12/03/98 6:02p Martin
** Add 3D Studio Max registry variable, and setup texture formats if
** turned on.
**
** 60    12/02/98 8:24a Martin
** Turn off anti-aliasing by default and rename some variables for
** super-sampling AA.
**
** 59    11/24/98 7:25p Martin
** Keep trilinear on for Voodoo2
**
** 58    11/24/98 5:56p Martin
** Disable trilinear mipmapping, and
** Disable unsupported blend modes
**
** 57    11/24/98 8:38a Andrew
** removed a compiler warning from the Voodoo II build
**
** 56    11/22/98 9:00p Andrew
** Changes to support multi-monitor
**
** 55    11/08/98 4:20p Hanson
** Fixes and Optimizations for Winbench 99
**
** 54    11/06/98 11:06a Adrians
** Create a new event handle on every initialisation when running with
** instrumentation.
**
** 53    10/21/98 9:20p Russ
** added include of precomp.h and some NT5 specific changes to
** D3DHALCALLBACKS and D3DHALCreateDriver
**
** 52    10/16/98 4:33p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
**
** 51    10/14/98 11:17a Hanson
** Fixed new DrawIndexedPrimitive stuff so it will run on pentium.
**
** 50    10/07/98 11:42p Adrians
** Make AA_EDGE default for Voodoo2.
**
** 49    10/06/98 2:03p Miriam
** Automipmap support
**
** 48    10/04/98 9:47p Adrians
** Add support for mirrored textures.
**
** 47    10/04/98 5:39p Hanson
** Added check and flags setting for non P6's
**
** 46    10/02/98 11:45a Adrians
** Added support for D3DTBLEND_ADD to DX5.
**
** 45    10/01/98 11:59a Adrians
** Added Event Notification to the instrumentation code.
**
** 43    9/24/98 4:22p Adrians
** Added initial Instrumentation support.
**
** 42    9/21/98 5:36p Adrians
** Add the missing TextureFilter caps bits.
**
** 41    9/10/98 8:20p Adrians
** Remove SEPERATETEXTUREMEMORY cap for Banshee.
**
** 40    9/02/98 12:32p Adrians
** DX6 multitexture change.
**
** 39    9/01/98 5:33p Martin
** Turn on super-sampling AA by default.  Registry variable can override
** default, to either turn it off, or force edge-AA on instead of
** super-sampling.
**
** 38    9/01/98 11:31a Miriam
** Default for LDELAY is off.
**
** 37    8/28/98 2:33p Martin
** Conditionalize banshee compilation.
**
** 36    8/28/98 12:37p Martin
** Do super-sampling AA when the registry key is set to 2.  Do edge AA if
** registry key == 1 OR we are not running at the 2 magical resolutions of
** 640x480 or 800x600.
**
** No more conditional compilation of AA.
**
** 35    8/23/98 12:49a Adrians
** Added support for AA SuperSampling.
**
** 33    8/14/98 8:08p Adrians
** Add new DX6 texture formats.
** Enable WFOG cap bit.
**
** 2     8/12/98 12:03p Adrians
** Enable WFOG caps bit.
**
** 1     8/10/98 6:20p Adrians
** Added new DX6 texture formats.
**
** 32    8/07/98 3:24p Adrians
** Swap 1555 and 565 textures back to fix Forsaken problem.
**
** 31    7/31/98 10:34p Miriam
** D3D & Glide cooperation. Allow each API to set/reset state to indicate
** that  the HW state has been changed.
**
** 30    7/31/98 6:17p Martin
** Add  INDEPENDENTUV to triangle caps
**
** 29    7/30/98 12:57p Miriam
** Automipmap textures with alpha. new implementation of automipmapping.
**
** 28    7/24/98 1:37p Hohn
**
** 27    7/22/98 12:37p Adrians
** Enable Lock Idle by default.
**
** 26    7/22/98 11:53a Adrians
** Added Lock Idle code & preliminary Automipmap code.
**
** 25    7/16/98 5:18p Martin
** Reorder pixelformats to be 3d Studio Max friendly
**
** 24    6/05/98 5:22p Miriam
** Use banshee registry key definitions. SSTH3_
**
** 23    6/03/98 2:25p Adrians
** Fix for Hyperblade.
**
** 22    5/26/98 7:22p Adrians
** Added antialiasing.
**
** 21    5/06/98 6:09p Adrians
** Changes for DX6 into DX5 driver.
**
** 3     5/05/98 3:28p Adrians
** Added new DX6 DEVCAPS.
**
** 2     5/01/98 4:11p Adrians
** Compile options for dx5 and dx6.
** Removed redundent returns.
**
** 1     4/29/98 6:31p Adrians
** Created
**
** 20    3/27/98 1:44p Adrians
** Code tidyup.
** Removed palette and texture clamp compile options.
**
** 19    3/27/98 11:16a Adrians
** Code added for triangle profiling.
**
** 18    3/16/98 1:46p Adrians
** Added dynamic pixel center offset control.
**
** 17    2/21/98 10:19p Adrians
** Added ZBIAS cap.
**
** 16    1/21/98 4:04p Miriam
** Changes that allow D3D to be shared between Voodoo2 & Banshee.
 *
 * 15    11/25/97 4:27p Suninn
 * add some temporary support for H3, the two lines will go away later
 * after we implemente memory allocation for the global data.
 *
 * 14    11/23/97 3:56p Suninn
 * replay _d3Global with _D3 & D3G macros
 *
 * 13    11/23/97 2:11p Suninn
 * replace _ddglobal with _DD macro
 *
 * 12    10/31/97 4:53p Adrians
 * Now reads environment variables from the Registry.
 * Gamma is now read from the environment variables.
 * Fixed ddtof that was very buggy.
 *
 * 11    10/29/97 4:44p Adrians
 * Change to get Software Triangle Setup code to compile.
 *
 * 10    10/24/97 7:14p Miriam
 * Support for 2 tmu and trilinear. Will see 2x the memory on a 2 tmu
 * system. Trilinear is 1 pass on 2 tmu & lodDither for 1 tmu.
 * Fixed command fifo problem when using save/restore.
 *
 * 9     10/10/97 10:42a Adrians
 * Removed all references to DIRECTX5.
 *
 * 8     10/07/97 1:39p Adrians
 * Optimisation to myRenderPrimitive setup.
 * New Caching enable call changes.
 *
 * 7     10/02/97 8:38p Adrians
 * Include init code into build. Enable Write Combining.  Inline system
 * functions.  Change optimisations.  Some code tidy up.
 *
 * 6     9/15/97 3:44p Adrians
 * Tidyup Fan and Strip code.
 * Removed some int to float routines
 * Changed lines and points to send wstz with all vertices
 * Converted tabs to spaces in some files
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
#include <string.h>

#include "sst1init.h"
#include "runtime.h"
#include "hw.h"
#include "header.h"
#include "d3global.h"
#include "ddglobal.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d3contxt.h"
#include "d3txtr.h"
#endif // ifndef WINNT

#include "regkeys.h"

#ifdef K6_2
#include "k6_2.h"
#include "d6global.h"
#endif // K6_2
                              
#ifdef WINNT
extern int __cdecl atoi(const char *);
#define ATOI  atoi

// retro3dfx: stop exporting palettized texture formats (P8 / D3DFMT_P8/A8P8).
// The Napalm P8 palette pipeline never delivers the palette to the TMU -- P8
// textures sample WHITE (GoldSrc Direct3D: white world, textured RGB sky; the
// vintage devs fought the same fight: rev 24 "disable palettized texture",
// rev 25 "reenable", the DCT-300 palette failures note, and the perma-disabled
// A8P8 block below). With P8 unexported, apps negotiate RGB formats instead,
// which render correctly. This is the vintage devs' own kill-switch, enabled.
#define DISABLE_PAL8_ON_NAPALM    1
#endif

extern DWORD __stdcall ddiSceneCapture(LPD3DHAL_SCENECAPTUREDATA psc) ;

void BuildD3DCaps(NT9XDEVICEDATA *ppdev, D3DDEVICEDESC_V1 *pCaps);
void BuildD3DExtendedCaps(NT9XDEVICEDATA *ppdev, D3DHAL_D3DEXTENDEDCAPS *pCaps);
void BuildDX7TextureFormats(NT9XDEVICEDATA *, DDSURFACEDESC *, int *);

DDSURFACEDESC g_DX7TextureFormats[MAX_TEXTURE_FORMATS];
int g_nDX7TextureFormats = 0;

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
void BuildD3DCaps8(NT9XDEVICEDATA *ppdev, D3DCAPS8 *pCaps);
void BuildDX8TextureFormats(NT9XDEVICEDATA *, DDSURFACEDESC *, int *);

DDSURFACEDESC g_DX8TextureFormats[MAX_TEXTURE_FORMATS];
int g_nDX8TextureFormats = 0;
#endif //(DIRECT3D_VERSION >= 0x0800) && (DX >= 8)

//--------------------------------------------------------------------
// Direct3D HAL Table - Indicates which HAL calls this driver supports
// This is the entry points for this driver therefore it is the roadmap
// for understanding when we get control.
//--------------------------------------------------------------------
static D3DHAL_CALLBACKS myD3DHALCallbacks = {
    sizeof(D3DHAL_CALLBACKS),

    // Device context
    ddiContextCreate,            // Required.
    ddiContextDestroy,           // Required.
#ifdef WINNT
    NULL,                        // (NT5) Reserved, must be zero
#else
    ddiContextDestroyAll,        // Required.
#endif

    // Scene capture
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    NULL,
#else
    ddiSceneCapture,             // Optional.
#endif

    // Execution
    NULL,                        // execute buffer is handled by D3D now 2/96
    NULL,                        // myExecuteClipped
#ifdef WINNT
    NULL,                        // (NT5) Reserved, must be zero
    NULL,                        // (NT5) Reserved, must be zero
#else
#if (DIRECT3D_VERSION >= 0x0600) && (DX >= 6)
    NULL,                        // Not required beyond DX 5 runtime
    NULL,                        // Not required beyond DX 5 runtime
#else
    ddiRenderState,              // (DX5) render state handler
    ddiRenderPrimitive,          // (DX5) primitive handler
#endif
#endif
    0L,                          // must be zero

    // Textures
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    NULL,
    NULL,
    NULL,
    NULL,
#else
    ddiHandleCreate,             // If any of these calls are supported,
    ddiHandleDestroy,            // they must all be.
    ddiHandleSwap,               // ditto - but can always fail.
    ddiHandleGetSurf,            // ditto - but can always fail.
#endif

    // Transform - must be supported if lighting is supported.
    NULL, //myMatrixCreate,      // If any of these calls are supported,
    NULL, //myMatrixDestroy,     // they must all be.
    NULL, //myMatrixSetData,     // ditto
    NULL, //myMatrixGetData,     // ditto
    NULL, //mySetViewportData,   // ditto

    // Lighting
    NULL,                        // If any of these calls are supported,
    NULL, //myMaterialCreate,    // they must all be.
    NULL, //myMaterialDestroy,   // ditto
    NULL, //myMaterialSetData,   // ditto
    NULL, //myMaterialGetData,   // ditto
    // Pipeline state
#ifdef WINNT
    0L,                          // (NT5) Reserved, must be zero
#else
    ddiGetState,                 // Required.
#endif

    0L,                          // Reserved, must be zero
    0L,                          // Reserved, must be zero
    0L,                          // Reserved, must be zero
    0L,                          // Reserved, must be zero
    0L,                          // Reserved, must be zero
    0L,                          // Reserved, must be zero
    0L,                          // Reserved, must be zero
    0L,                          // Reserved, must be zero
    0L,                          // Reserved, must be zero
    0L,                          // Reserved, must be zero

};

#define nullPrimCaps {                                          \
    sizeof(D3DPRIMCAPS), 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0     \
}

#if defined( INSTRUMENTATION )
#include "dxins.h"
INSHEADER insHeader = { 0 };  // Set flags to 0
DWORD totalTimeH, totalTimeL;
DWORD counter0H, counter0L, counter1H, counter1L;
DWORD queueTime, queueTimeH, queueTimeL;
DWORD insCategory;
HANDLE insEventHandle;
static BOOL insInitialised = FALSE;
#endif

#ifdef K6_2

/*-------------------------------------------------------------------
Function Name:  ADJUST3DNOWPOINTERS
Description:    Here we do 3DNow-function pointers initialization
Information:    BOOL ADJUST3DNOWPOINTERS(NT9XDEVICEDATA *ppdev)
Return:         FALSE if no 3DNow! feature detected

-------------------------------------------------------------------*/
BOOL ADJUST3DNOWPOINTERS(NT9XDEVICEDATA *ppdev)
{
  static char regDisOptKey[]="DISABLEPSGP";
  BOOL optEnable=TRUE;
  extern BOOL _3DNowAllowed;

  if (!detect3DX()) return FALSE;

  if (GETENV(regDisOptKey))
    optEnable=!(BOOL)atoi(GETENV(regDisOptKey));

  if (optEnable) {
    _3DNowAllowed = TRUE;
    // do here pointers to opt functions initialization
#ifndef WINNT    
#if (DX >= 6)
    DrawTriFanImmAsm     =DrawTriFanImmAsm_K62O;

    DrawTriEdge2Asm      =DrawTriEdge2Asm_K62O;
    DrawTriEdge2GAsm     =DrawTriEdge2GAsm_K62O;
    /* not implemented yet */
    DrawTriEdge2FAsm     =DrawTriEdge2FAsm_P5;

#if (NEWASMTRI==1)
    PrefetchTriIdx2_12_Asm = PrefetchTriIdx2_12_Asm_K6;
    PrefetchTriIdx2_11_Asm = PrefetchTriIdx2_XX_Asm_K6;
    PrefetchTriIdx2_10_Asm = PrefetchTriIdx2_10_Asm_K6;
    PrefetchTriIdx2_9_Asm  = PrefetchTriIdx2_XX_Asm_K6;
    PrefetchTriIdx2_8_Asm  = PrefetchTriIdx2_8_Asm_K6;
    PrefetchTriIdx2_7_Asm  = PrefetchTriIdx2_XX_Asm_K6;
    PrefetchTriIdx2_6T_Asm = PrefetchTriIdx2_XX_Asm_K6;
    PrefetchTriIdx2_6G_Asm = PrefetchTriIdx2_XX_Asm_K6;
    PrefetchTriIdx2_5_Asm  = PrefetchTriIdx2_XX_Asm_K6;
#else // Not NEWASMTRI
    PrefetchTriIdx2_12_Asm = PrefetchTriIdx2_12_Asm_K6;
    PrefetchTriIdx2_11_Asm = PrefetchTriIdx2_11_Asm_K6;
    PrefetchTriIdx2_10_Asm = PrefetchTriIdx2_10_Asm_K6;
    PrefetchTriIdx2_9_Asm  = PrefetchTriIdx2_9_Asm_K6;
    PrefetchTriIdx2_8_Asm  = PrefetchTriIdx2_8_Asm_K6;
    PrefetchTriIdx2_7_Asm  = PrefetchTriIdx2_7_Asm_K6;
    PrefetchTriIdx2_6T_Asm = PrefetchTriIdx2_6T_Asm_K6;
    PrefetchTriIdx2_6G_Asm = PrefetchTriIdx2_6G_Asm_K6;
    PrefetchTriIdx2_5_Asm  = PrefetchTriIdx2_5_Asm_K6;
#endif // NEWASMTRI\

    // C-style DP2 function
    dp2StripAll=dp2StripAll_K62O;
    dp2IdxStripAll=dp2IdxStripAll_K62O;

    dp2FanAll=dp2FanAll_K62O;
    dp2IdxFanAll=dp2IdxFanAll_K62O;

    dp2TriangleAll     = dp2TriangleAll_K62O;
    dp2IdxTriangleAll  = dp2IdxTriangleAll_K62O;
    dp2IdxTriangle2All = dp2IdxTriangle2All_K62O;

#endif // (DX >= 6)
    fogGenerateLinear = fogGenerateLinear_Orig;
    
#else // WINNT    

#if (NEWASMTRI==1)
    PrefetchTriIdx2_12_Asm = PrefetchTriIdx2_12_Asm_K6;
    PrefetchTriIdx2_11_Asm = PrefetchTriIdx2_XX_Asm_K6;
    PrefetchTriIdx2_10_Asm = PrefetchTriIdx2_10_Asm_K6;
    PrefetchTriIdx2_9_Asm  = PrefetchTriIdx2_XX_Asm_K6;
    PrefetchTriIdx2_8_Asm  = PrefetchTriIdx2_8_Asm_K6;
    PrefetchTriIdx2_7_Asm  = PrefetchTriIdx2_XX_Asm_K6;
    PrefetchTriIdx2_6T_Asm = PrefetchTriIdx2_XX_Asm_K6;
    PrefetchTriIdx2_6G_Asm = PrefetchTriIdx2_XX_Asm_K6;
    PrefetchTriIdx2_5_Asm  = PrefetchTriIdx2_XX_Asm_K6;       
#endif // NEWASMTRI
#endif // WINNT

  }
  else {
    _3DNowAllowed = FALSE;
#ifndef WINNT
#if (DX >= 6)
    DrawTriFanImmAsm     =DrawTriFanImmAsm_P5;

    DrawTriEdge2Asm      =DrawTriEdge2Asm_P5;
    DrawTriEdge2GAsm     =DrawTriEdge2GAsm_P5;
    DrawTriEdge2FAsm     =DrawTriEdge2FAsm_P5;

    PrefetchTriIdx2_12_Asm = PrefetchTriIdx2_12_Asm_P5;
    PrefetchTriIdx2_11_Asm = PrefetchTriIdx2_11_Asm_P5;
    PrefetchTriIdx2_10_Asm = PrefetchTriIdx2_10_Asm_P5;
    PrefetchTriIdx2_9_Asm  = PrefetchTriIdx2_9_Asm_P5;
    PrefetchTriIdx2_8_Asm  = PrefetchTriIdx2_8_Asm_P5;
    PrefetchTriIdx2_7_Asm  = PrefetchTriIdx2_7_Asm_P5;
    PrefetchTriIdx2_6T_Asm = PrefetchTriIdx2_6T_Asm_P5;
    PrefetchTriIdx2_6G_Asm = PrefetchTriIdx2_6G_Asm_P5;
    PrefetchTriIdx2_5_Asm  = PrefetchTriIdx2_5_Asm_P5;

    // C-style DP2 function
    dp2StripAll=dp2StripAll_Orig;
    dp2IdxStripAll=dp2IdxStripAll_Orig;

    dp2FanAll=dp2FanAll_Orig;
    dp2IdxFanAll=dp2IdxFanAll_Orig;

    dp2TriangleAll=dp2TriangleAll_Orig;
    dp2IdxTriangleAll=dp2IdxTriangleAll_Orig;
    dp2IdxTriangle2All=dp2IdxTriangle2All_Orig;

#endif // (DX >= 6)
    fogGenerateLinear = fogGenerateLinear_Orig;
    
#else // WINNT

    PrefetchTriIdx2_12_Asm = PrefetchTriIdx2_12_Asm_P5;
    PrefetchTriIdx2_11_Asm = PrefetchTriIdx2_11_Asm_P5;
    PrefetchTriIdx2_10_Asm = PrefetchTriIdx2_10_Asm_P5;
    PrefetchTriIdx2_9_Asm  = PrefetchTriIdx2_9_Asm_P5;
    PrefetchTriIdx2_8_Asm  = PrefetchTriIdx2_8_Asm_P5;
    PrefetchTriIdx2_7_Asm  = PrefetchTriIdx2_7_Asm_P5;
    PrefetchTriIdx2_6T_Asm = PrefetchTriIdx2_6T_Asm_P5;
    PrefetchTriIdx2_6G_Asm = PrefetchTriIdx2_6G_Asm_P5;
    PrefetchTriIdx2_5_Asm  = PrefetchTriIdx2_5_Asm_P5;
    
#endif  // WINNT
  }

  return TRUE;
}
#endif // K6_2

//-------------------------------------------------------------------
// Functions used to instantiate the 3D portion of the DirectDraw HAL
//-------------------------------------------------------------------

/*-------------------------------------------------------------------
Function Name:  D3DHALCREATEDRIVER
Description:    Create the 3D portion of the DDraw HAL
Information:    BOOL __stdcall D3DHALCREATEDRIVER(NT9XDEVICEDATA *ppdev,
                                 LPD3DHAL_GLOBALDRIVERDATA* lplpGlobal,
                                 LPD3DHAL_CALLBACKS* lplpHALCallbacks)
Return:         BOOL    TRUE - upon success
                BOOL    FALSE- upon failure

-------------------------------------------------------------------*/
#ifdef DEBUG
extern void _stdcall InitDebugMsgs();
#endif

#if   defined(TnL_HAL) && defined(VERT_BUFF)
BOOL __stdcall D3DHALCreateDriver(NT9XDEVICEDATA *ppdev,
                                  LPD3DHAL_GLOBALDRIVERDATA* lplpGlobal,
                                  LPD3DHAL_CALLBACKS* lplpHALCallbacks,
                                  DDHAL_DDEXEBUFCALLBACKS   *pExebufCallbacks )
#else
BOOL __stdcall D3DHALCreateDriver(NT9XDEVICEDATA *ppdev,
                                  LPD3DHAL_GLOBALDRIVERDATA* lplpGlobal,
                                  LPD3DHAL_CALLBACKS* lplpHALCallbacks)
#endif
{
//   int i;
  FxU32 cpu;

   cpu=cpu_detect_asm();

#ifdef DEBUG
  InitDebugMsgs();
#endif

   if (cpu!=6)
      _cpu_type=STATE_BAD_CPU;
#if (DX>=6)
   if (cpu==6)
   {
      PrefetchTriIdx2_12_Asm=PrefetchTriIdx2_12_Asm_P6;
      PrefetchTriIdx2_11_Asm=PrefetchTriIdx2_11_Asm_P6;
      PrefetchTriIdx2_10_Asm=PrefetchTriIdx2_10_Asm_P6;
      PrefetchTriIdx2_9_Asm=PrefetchTriIdx2_9_Asm_P6;
      PrefetchTriIdx2_8_Asm=PrefetchTriIdx2_8_Asm_P6;
      PrefetchTriIdx2_7_Asm=PrefetchTriIdx2_7_Asm_P6;
      PrefetchTriIdx2_6T_Asm=PrefetchTriIdx2_6T_Asm_P6;

#ifdef WINNT

// Scott Kephart's old TriIdx2 KNI code isn't included in the Win2K build. 
// Hence, if NEWASMTRI isn't defined we stick with the P6 code. 

#if (STBKNI==1) && (NEWASMTRI==1)

// Fix for PRS 5177.  Removed the getKatmai function call because it did not prevent
// execution of Katmai instructions in Windows95.

      if (CPUTYPE & P6_INTELCPU_WITH_KNI)
      {                      
//      Don't support the cases that are horrible for Katmai because they
//      are unaligned. These will be addressed later, but they aren't
//      important in Winbench 1.1...
        PrefetchTriIdx2_12_Asm=PrefetchTriIdx2_12_Asm_KNI;
//      PrefetchTriIdx2_11_Asm=PrefetchTriIdx2_11_Asm_KNI;
        PrefetchTriIdx2_10_Asm=PrefetchTriIdx2_10_Asm_KNI;
        PrefetchTriIdx2_9_Asm=PrefetchTriIdx2_9_Asm_KNI;
        PrefetchTriIdx2_8_Asm=PrefetchTriIdx2_8_Asm_KNI;
        PrefetchTriIdx2_7_Asm=PrefetchTriIdx2_7_Asm_KNI;
        PrefetchTriIdx2_6T_Asm=PrefetchTriIdx2_6T_Asm_KNI;
		PrefetchTriIdx2_5_Asm=PrefetchTriIdx2_5_Asm_KNI;
      }
#endif //STBKNI && NEWASMTRI
   }
   
#else // NOT (WINNT)
   
//STB-SK 01/30/99 KNI Changes
#if (STBKNI==1)

// Fix for PRS 5177.  Removed the getKatmai function call because it did not prevent
// execution of Katmai instructions in Windows95.

      if (CPUTYPE & P6_INTELCPU_WITH_KNI)
      {                      
//      Don't support the cases that are horrible for Katmai because they
//      are unaligned. These will be addressed later, but they aren't
//      important in Winbench 1.1...
#if (NEWASMTRI == 1)
        PrefetchTriIdx2_12_Asm=PrefetchTriIdx2_12_Asm_KNI;
//      PrefetchTriIdx2_11_Asm=PrefetchTriIdx2_11_Asm_KNI;
        PrefetchTriIdx2_10_Asm=PrefetchTriIdx2_10_Asm_KNI;
        PrefetchTriIdx2_9_Asm=PrefetchTriIdx2_9_Asm_KNI;
        PrefetchTriIdx2_8_Asm=PrefetchTriIdx2_8_Asm_KNI;
        PrefetchTriIdx2_7_Asm=PrefetchTriIdx2_7_Asm_KNI;
        PrefetchTriIdx2_6T_Asm=PrefetchTriIdx2_6T_Asm_KNI;
		PrefetchTriIdx2_5_Asm=PrefetchTriIdx2_5_Asm_KNI;
#else // Not NEWASMTRI
        PrefetchTriIdx2_12_Asm=PrefetchTriIdx2_12_Asm_KNI;
//        PrefetchTriIdx2_11_Asm=PrefetchTriIdx2_11_Asm_KNI;
        PrefetchTriIdx2_10_Asm=PrefetchTriIdx2_10_Asm_KNI;
//        PrefetchTriIdx2_9_Asm=PrefetchTriIdx2_9_Asm_KNI;
        PrefetchTriIdx2_8_Asm=PrefetchTriIdx2_8_Asm_KNI;
//        PrefetchTriIdx2_7_Asm=PrefetchTriIdx2_7_Asm_KNI;
//        PrefetchTriIdx2_6T_Asm=PrefetchTriIdx2_6T_Asm_KNI;
#endif // NEWASMTRI

        DrawTriFanImmAsm = DrawTriFanImmAsm_KNI;
       }
#endif //STBKNI
   }
   
#endif //WINNT      

#if defined(TnL_HAL) && defined(VERT_BUFF)
    pExebufCallbacks->dwSize = sizeof(DDHAL_DDEXEBUFCALLBACKS);
    pExebufCallbacks->CanCreateExecuteBuffer = CanCreateExecuteBuffer32;
    pExebufCallbacks->CreateExecuteBuffer = CreateExecuteBuffer32;
    pExebufCallbacks->DestroyExecuteBuffer = DestroyExecuteBuffer32;
    pExebufCallbacks->LockExecuteBuffer = LockExecuteBuffer32;
    pExebufCallbacks->UnlockExecuteBuffer = UnlockExecuteBuffer32;
    pExebufCallbacks->dwFlags =
        (pExebufCallbacks->CanCreateExecuteBuffer ? DDHAL_EXEBUFCB32_CANCREATEEXEBUF : 0) |
        (pExebufCallbacks->CreateExecuteBuffer    ? DDHAL_EXEBUFCB32_CREATEEXEBUF    : 0) |
        (pExebufCallbacks->DestroyExecuteBuffer   ? DDHAL_EXEBUFCB32_DESTROYEXEBUF   : 0) |
        (pExebufCallbacks->LockExecuteBuffer      ? DDHAL_EXEBUFCB32_LOCKEXEBUF      : 0) |
        (pExebufCallbacks->UnlockExecuteBuffer    ? DDHAL_EXEBUFCB32_UNLOCKEXEBUF    : 0) ;            
#endif

#endif // DX>=6

  if( NULL == GETENV(ANTIALIAS) )
  {
    _D3( enableAntialias ) = AA_NOT_ENABLED;
  }
  else
  {
    _D3( enableAntialias ) = ATOI( GETENV(ANTIALIAS) );
  }

#if defined( H3 )
  if( NULL == GETENV(TRILINEAR) )
  {
    _D3( enableTrilinear ) = 0;
  }
  else
  {
    _D3( enableTrilinear ) = ATOI( GETENV(TRILINEAR) );
  }

#endif

    // Build the texture format array/count
    BuildDX7TextureFormats(ppdev, g_DX7TextureFormats, &g_nDX7TextureFormats);
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
    BuildDX8TextureFormats(ppdev, g_DX8TextureFormats, &g_nDX8TextureFormats);
#endif

    // Declare texture formats
    _D3(d3d).dwNumTextureFormats = g_nDX7TextureFormats;
    _D3(d3d).lpTextureFormats = g_DX7TextureFormats;

  if (IS_NAPALM)
  {
     if( NULL == GETENV(CBC) )
     {
#ifdef COLUMNBANDCONTROL_DEFAULT
       _D3( columnBandControl ) = 0;       // default to column-of-8 for tri iterators
#else
       _D3( columnBandControl ) = 2;       // default to column-of-32 for tri iterators
#endif
     }
     else
     {
       _D3( columnBandControl ) = ATOI( GETENV(CBC) );
     }

	 // LOD Bias
     if( NULL == GETENV(LOD_BIAS) )
     { 
         _D3( LodBias ) = 0;
     }
     else
     {
       _D3(LodBias) = ATOI(GETENV(LOD_BIAS));

       // Clamp to -32 to +31. Bias is a signed 4.2 fixed format.
       // Each integer step represents .25, which really makes the range -8.0 to +7.75.
       if (_D3(LodBias) < -32)
       {
         D3DPRINT(0, "D3DHalCreateDriver: Registry LOD Bias < -32. Clamping.");
         _D3(LodBias) = -32;
       }

       if (_D3(LodBias) > 31)
       {
         D3DPRINT(0, "D3DHalCreateDriver: Registry LOD Bias > 31. Clamping.");
         _D3(LodBias) = 31;
       }
     }

#ifdef MAXPENDINGBUFFERS
     // Maximum pending buffers when flipping.
     if( NULL == GETENV(MAX_PENDING_BUFFERS) )
     { 
        _D3(useMaxPendingBuffers) = 0;
        _D3(maxPendingBuffers) = 0;
     }
     else
     {
        _D3(useMaxPendingBuffers) = 1;
        _D3(maxPendingBuffers) = ATOI(GETENV(MAX_PENDING_BUFFERS));

        // Make sure we don't exceed what would cause the hardware to hang.
        if (_D3(maxPendingBuffers) > MAXPENDINGBUFFERS_TOAVOIDHANG)
        {
           _D3(maxPendingBuffers) = MAXPENDINGBUFFERS_TOAVOIDHANG;
        }
     }
#endif

#if defined(LODDITHER) || defined(LODBIASPERCHIP)
     // Maximum pending buffers when flipping.
     if( NULL == GETENV(LOD_DITHER) )
     { 
        _D3(LodDither) = 0;
     }
     else
     {
        _D3(LodDither) = ATOI(GETENV(LOD_DITHER));
     }
#endif

     // By default, we want dither rotation enabled.
     if( NULL == GETENV(DITHER_ROTATION) )
     {
       _D3( ditherRotation ) = 1;
     }
     else
     {
       _D3( ditherRotation ) = ATOI( GETENV(DITHER_ROTATION) );
     }

     if( NULL == GETENV(THIRTYTWOBPP_RENDERING) )
     {
       // Enable the possibility of 32bpp rendering by default.
       _D3( thirtyTwoBppRendering ) = 1;
     }
     else
     {
       _D3( thirtyTwoBppRendering ) = ATOI( GETENV(THIRTYTWOBPP_RENDERING) );
     }

     if( NULL == GETENV(TWOPPC) )
     {
        // Enable 2 pixels-per-clock by default.
        _D3( TwoPpc ) = SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK; 
     }
     else
     {
       _D3( TwoPpc ) = ATOI( GETENV(TWOPPC) );
       if (_D3(TwoPpc)) { 
          _D3(TwoPpc) = SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK;
       }
       else {
          _D3(TwoPpc) = 0;
       }
     }

     if( NULL == GETENV(TWOPPC_LOG2_BAND_HEIGHT) )
     {
        // Band Height is 1 line by default.
        _D3( TwoPpcLog2BandHeight ) = 2;
     }
     else
     {
       _D3( TwoPpcLog2BandHeight ) = ATOI( GETENV(TWOPPC_LOG2_BAND_HEIGHT) );
       // "log 2 band height" in renderMode used for 2ppc rendering must specify a band height
       // that is less than the band height used to setup the sliCtrl register (and the other sli init regs).
       // I should check this, but the reg entry is titled, "Log2BandHeight" instead of "BandHeight".  These entries are for tweaking
       // and might go away later.  So, until all our assumptions about consistancy in the naming are in sync, I won't check for this.
        // But lets at least bulletproof the range for the bitfield, log2bandheight.
       if ( _D3( TwoPpcLog2BandHeight ) <0 ) {
           _D3( TwoPpcLog2BandHeight )=0;
       }
       if ( _D3( TwoPpcLog2BandHeight ) >7 ) {
           _D3( TwoPpcLog2BandHeight )=7;
       }

     }

     if( NULL == GETENV(GUARDBAND_CLIPPING) )
     {
        // Enable Guardband Clipping by default.
        _D3( GuardbandClipping ) = 1;
     }
     else
     {
       _D3( GuardbandClipping ) = ATOI( GETENV(GUARDBAND_CLIPPING) );
     }

     // D I T H E R   M A T R I X   S E L E C T I O N
     { // Capture the Dither Matrix selection values.
        LPSTR lpStr;
        ULONG DithMatSel;        // fogMode[13:12]
        ULONG DithMatSelABlnd;   // fogMode[15:14]
        ULONG DithMatSelAA;      // fogMode[17:16]
        ULONG DithMatSelABlndAA; // fogMode[19:18]
        ULONG ulForceFlag;

        // Force flag:
        // 0 - Force capability disabled. Follow standard behavior - Use registry entries,
        //     if present, else defaults.
        // 1 - Force usage of the defaults, regardless of entries in Registry.
        // 2 - Force all values to zero, regardless of entries in Registry.
        if( NULL == GETENV(DITHMAT_FORCEFLAG) )
        { 
            ulForceFlag = 0;
        }
        else
        {
            ulForceFlag = ATOI( GETENV(DITHMAT_FORCEFLAG) );
        }

        if (ulForceFlag == 2)
        {
          // Force to zero
          _D3( DithMatSel_2Smpl )      = 0;
          _D3( DithMatSel_2Smpl_Chp0 ) = 0;
          _D3( DithMatSel_2Smpl_Chp1 ) = 0;
          _D3( DithMatSel_4Smpl_Chp0 ) = 0;
          _D3( DithMatSel_4Smpl_Chp1 ) = 0;
        }
        else
        {
	        // 2 Sample Antialiasing - 1 chip
	        if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSEL_2SMPL))) )
	        { DithMatSel = DITHMATSEL_2SMPL_DEF; }
	        else
	        { DithMatSel = ATOI(lpStr) << SST_DITHER_ROTATE_SHIFT; }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLND_2SMPL))) )
	        { DithMatSelABlnd = DITHMATSELABLND_2SMPL_DEF; }
	        else
	        { DithMatSelABlnd = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_SHIFT; }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELAA_2SMPL))) )
	        { DithMatSelAA = DITHMATSELAA_2SMPL_DEF; }
	        else
	        { DithMatSelAA = ATOI(lpStr) << SST_DITHER_ROTATE_AA_SHIFT; }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLNDAA_2SMPL))) )
	        { DithMatSelABlndAA = DITHMATSELABLNDAA_2SMPL_DEF; }
	        else
	        { DithMatSelABlndAA = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_AA_SHIFT; }

	        _D3( DithMatSel_2Smpl ) = DithMatSel        |
	                                  DithMatSelABlnd   |
	                                  DithMatSelAA      |
	                                  DithMatSelABlndAA;

	        _D3( DithMatSel_2Smpl_Chp0 ) = DithMatSel | DithMatSelABlnd;

	        _D3( DithMatSel_2Smpl_Chp1 ) = (DithMatSelAA | DithMatSelABlndAA) >> (SST_DITHER_ROTATE_AA_SHIFT - SST_DITHER_ROTATE_SHIFT);

	        // 4 Sample Antialiasing - 2 chips
	        // Chip 0
	        if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSEL_4SMPL_CHP0))) )
	        { DithMatSel = DITHMATSEL_4SMPL_CHP0_DEF; }
	        else
	        { DithMatSel = ATOI(lpStr) << SST_DITHER_ROTATE_SHIFT; }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLND_4SMPL_CHP0))) )
	        { DithMatSelABlnd = DITHMATSELABLND_4SMPL_CHP0_DEF; }
	        else
	        { DithMatSelABlnd = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_SHIFT; }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELAA_4SMPL_CHP0))) )
	        { DithMatSelAA = DITHMATSELAA_4SMPL_CHP0_DEF; }
	        else
	        { DithMatSelAA = ATOI(lpStr) << SST_DITHER_ROTATE_AA_SHIFT; }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLNDAA_4SMPL_CHP0))) )
	        { DithMatSelABlndAA = DITHMATSELABLNDAA_4SMPL_CHP0_DEF; }
	        else
	        { DithMatSelABlndAA = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_AA_SHIFT; }

          if (4 == _FF(dwNumUnits))
          {
            _D3( DithMatSel_4Smpl_Chp0 ) = DithMatSel      |
                                           DithMatSelABlnd;

            _D3( DithMatSel_4Smpl_Chp1 ) = (DithMatSelAA | DithMatSelABlndAA) >> (SST_DITHER_ROTATE_AA_SHIFT - SST_DITHER_ROTATE_SHIFT);
          }
          else
          {
            _D3( DithMatSel_4Smpl_Chp0 ) = DithMatSel        |
                                           DithMatSelABlnd   |
                                           DithMatSelAA      |
                                           DithMatSelABlndAA;
          }

	        // Chip 1
	        if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSEL_4SMPL_CHP1))) )
	        { DithMatSel = DITHMATSEL_4SMPL_CHP1_DEF; }
	        else
	        { DithMatSel = ATOI(lpStr) << SST_DITHER_ROTATE_SHIFT; }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLND_4SMPL_CHP1))) )
	        { DithMatSelABlnd = DITHMATSELABLND_4SMPL_CHP1_DEF; }
	        else
	        { DithMatSelABlnd = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_SHIFT; }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELAA_4SMPL_CHP1))) )
	        { DithMatSelAA = DITHMATSELAA_4SMPL_CHP1_DEF; }
	        else
	        { DithMatSelAA = ATOI(lpStr) << SST_DITHER_ROTATE_AA_SHIFT; }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLNDAA_4SMPL_CHP1))) )
	        { DithMatSelABlndAA = DITHMATSELABLNDAA_4SMPL_CHP1_DEF; }
	        else
	        { DithMatSelABlndAA = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_AA_SHIFT; }

          if (4 == _FF(dwNumUnits))
          {
            _D3( DithMatSel_4Smpl_Chp2 ) = DithMatSel      |
                                           DithMatSelABlnd;

            _D3( DithMatSel_4Smpl_Chp3 ) = (DithMatSelAA | DithMatSelABlndAA) >> (SST_DITHER_ROTATE_AA_SHIFT - SST_DITHER_ROTATE_SHIFT);
          }
          else
          {
            _D3( DithMatSel_4Smpl_Chp1 ) = DithMatSel        |
                                           DithMatSelABlnd   |
                                           DithMatSelAA      |
                                           DithMatSelABlndAA;
          }

          // 8 Sample 4 Chip
          // Chip 0
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSEL_8SMPL_CHP0))) )
          { DithMatSel = DITHMATSEL_8SMPL_CHP0_DEF; }
          else
          { DithMatSel = ATOI(lpStr) << SST_DITHER_ROTATE_SHIFT; }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLND_8SMPL_CHP0))) )
          { DithMatSelABlnd = DITHMATSELABLND_8SMPL_CHP0_DEF; }
          else
          { DithMatSelABlnd = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_SHIFT; }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELAA_8SMPL_CHP0))) )
          { DithMatSelAA = DITHMATSELAA_8SMPL_CHP0_DEF; }
          else
          { DithMatSelAA = ATOI(lpStr) << SST_DITHER_ROTATE_AA_SHIFT; }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLNDAA_8SMPL_CHP0))) )
          { DithMatSelABlndAA = DITHMATSELABLNDAA_8SMPL_CHP0_DEF; }
          else
          { DithMatSelABlndAA = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_AA_SHIFT; }
          
          _D3( DithMatSel_8Smpl_Chp0 ) = DithMatSel        |
                                         DithMatSelABlnd   |
                                         DithMatSelAA      |
                                         DithMatSelABlndAA;
          
          // Chip 1
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSEL_8SMPL_CHP1))) )
          { DithMatSel = DITHMATSEL_8SMPL_CHP1_DEF; }
          else
          { DithMatSel = ATOI(lpStr) << SST_DITHER_ROTATE_SHIFT; }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLND_8SMPL_CHP1))) )
          { DithMatSelABlnd = DITHMATSELABLND_8SMPL_CHP1_DEF; }
          else
          { DithMatSelABlnd = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_SHIFT; }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELAA_8SMPL_CHP1))) )
          { DithMatSelAA = DITHMATSELAA_8SMPL_CHP1_DEF; }
          else
          { DithMatSelAA = ATOI(lpStr) << SST_DITHER_ROTATE_AA_SHIFT; }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLNDAA_8SMPL_CHP1))) )
          { DithMatSelABlndAA = DITHMATSELABLNDAA_8SMPL_CHP1_DEF; }
          else
          { DithMatSelABlndAA = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_AA_SHIFT; }
          
          _D3( DithMatSel_8Smpl_Chp1 ) = DithMatSel        |
                                         DithMatSelABlnd   |
                                         DithMatSelAA      |
                                         DithMatSelABlndAA;
          
          // Chip 2
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSEL_8SMPL_CHP2))) )
          { DithMatSel = DITHMATSEL_8SMPL_CHP2_DEF; }
          else
          { DithMatSel = ATOI(lpStr) << SST_DITHER_ROTATE_SHIFT; }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLND_8SMPL_CHP2))) )
          { DithMatSelABlnd = DITHMATSELABLND_8SMPL_CHP2_DEF; }
          else
          { DithMatSelABlnd = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_SHIFT; }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELAA_8SMPL_CHP2))) )
          { DithMatSelAA = DITHMATSELAA_8SMPL_CHP2_DEF; }
          else
          { DithMatSelAA = ATOI(lpStr) << SST_DITHER_ROTATE_AA_SHIFT; }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLNDAA_8SMPL_CHP2))) )
          { DithMatSelABlndAA = DITHMATSELABLNDAA_8SMPL_CHP2_DEF; }
          else
          { DithMatSelABlndAA = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_AA_SHIFT; }
          
          _D3( DithMatSel_8Smpl_Chp2 ) = DithMatSel        |
                                         DithMatSelABlnd   |
                                         DithMatSelAA      |
                                         DithMatSelABlndAA;
          
          // Chip 3
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSEL_8SMPL_CHP3))) )
          { DithMatSel = DITHMATSEL_8SMPL_CHP3_DEF; }
          else
          { DithMatSel = ATOI(lpStr) << SST_DITHER_ROTATE_SHIFT; }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLND_8SMPL_CHP3))) )
          { DithMatSelABlnd = DITHMATSELABLND_8SMPL_CHP3_DEF; }
          else
          { DithMatSelABlnd = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_SHIFT; }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELAA_8SMPL_CHP3))) )
          { DithMatSelAA = DITHMATSELAA_8SMPL_CHP3_DEF; }
          else
          { DithMatSelAA = ATOI(lpStr) << SST_DITHER_ROTATE_AA_SHIFT; }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(DITHMATSELABLNDAA_8SMPL_CHP3))) )
          { DithMatSelABlndAA = DITHMATSELABLNDAA_8SMPL_CHP3_DEF; }
          else
          { DithMatSelABlndAA = ATOI(lpStr) << SST_DITHER_ROTATE_BLEND_AA_SHIFT; }
          
          _D3( DithMatSel_8Smpl_Chp3 ) = DithMatSel        |
                                         DithMatSelABlnd   |
                                         DithMatSelAA      |
                                         DithMatSelABlndAA;
          
        }

     }

     // A A  J I T T E R
     { // Capture the Jitter values. Format is 3.4 fixed.
       // Make the values ready to be used.
        LPSTR lpStr;
        ULONG PriBufVertexOffsetX;
        ULONG PriBufVertexOffsetY;
        ULONG SecBufVertexOffsetX;
        ULONG SecBufVertexOffsetY;
        ULONG ulForceFlag;

        // Force flag:
        // 0 - Force capability disabled. Follow standard behavior - Use registry entries,
        //     if present, else defaults.
        // 1 - Force usage of the defaults, regardless of entries in Registry.
        // 2 - Force all values to zero, regardless of entries in Registry.
        if( NULL == GETENV(AAJITTER_FORCEFLAG) )
        { 
            ulForceFlag = 0;
        }
        else
        {
            ulForceFlag = ATOI( GETENV(AAJITTER_FORCEFLAG) );
        }

        if (ulForceFlag == 2)
        {
        	// Force to zero
                _D3( aaJitterValues_2Smpl )      = 0;
                _D3( aaJitterValues_2Smpl_Chp0 ) = 0;
                _D3( aaJitterValues_2Smpl_Chp1 ) = 0;
                _D3( aaJitterValues_4Smpl_Chp0 ) = 0;
                _D3( aaJitterValues_4Smpl_Chp1 ) = 0;
                // 4 chip single sample
                _D3( aaJitterValues_4Smpl_Chp2 ) = 0;
                _D3( aaJitterValues_4Smpl_Chp3 ) = 0;
        
                // 8 chip double sample
                _D3( aaJitterValues_8Smpl_Chp0 ) = 0;
                _D3( aaJitterValues_8Smpl_Chp1 ) = 0;
                _D3( aaJitterValues_8Smpl_Chp2 ) = 0;
                _D3( aaJitterValues_8Smpl_Chp3 ) = 0;
        }
        else
        {
	        // 2 Sample Antialiasing - 1 chip
	        if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_X_2SMPL))) )
	        { 
	            PriBufVertexOffsetX = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFX_2SMPL_DEF)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                                             &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
	        }
	        else
	        {
	            PriBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
	        }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_Y_2SMPL))) )
	        { 
	            PriBufVertexOffsetY = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFY_2SMPL_DEF)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                                             &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
	        }
	        else
	        {
	            PriBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
	        }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_X_2SMPL))) )
	        { 
	            SecBufVertexOffsetX = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFX_2SMPL_DEF)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                                             &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
	        }
	        else
	        {
            SecBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
	        }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_Y_2SMPL))) )
	        { 
	            SecBufVertexOffsetY = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFY_2SMPL_DEF)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                                             &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
	        }
	        else
	        {
	            SecBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
	        }

        
	        _D3( aaJitterValues_2Smpl ) = PriBufVertexOffsetX |
	                                      PriBufVertexOffsetY |
	                                      SecBufVertexOffsetX |
	                                      SecBufVertexOffsetY ;

	        _D3( aaJitterValues_2Smpl_Chp0 ) = PriBufVertexOffsetX |
	                                           PriBufVertexOffsetY;

	        _D3( aaJitterValues_2Smpl_Chp1 ) = (SecBufVertexOffsetX | SecBufVertexOffsetY) >> (SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT - SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT);

	        // 4 Sample Antialiasing - 2 chip
	        // Chip 0
	        if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_X_4SMPL_CHIP0))) )
	        { 
	            PriBufVertexOffsetX = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFX_4SMPL_CHP0_DEF)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
	        }
	        else
	        {
	            PriBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
	        }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_Y_4SMPL_CHIP0))) )
	        { 
	            PriBufVertexOffsetY = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFY_4SMPL_CHP0_DEF)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
	        }
	        else
	        {
	            PriBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
	        }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_X_4SMPL_CHIP0))) )
	        { 
	            SecBufVertexOffsetX = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFX_4SMPL_CHP0_DEF)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
	        }
	        else
	        {
	            SecBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
	        }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_Y_4SMPL_CHIP0))) )
	        { 
	            SecBufVertexOffsetY = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFY_4SMPL_CHP0_DEF)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
	        }
	        else
	        {
	            SecBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
	        }

          if (4 == _FF(dwNumUnits))
          {
            _D3( aaJitterValues_4Smpl_Chp0 ) = PriBufVertexOffsetX |
                                               PriBufVertexOffsetY;
            
            _D3( aaJitterValues_4Smpl_Chp1 ) = (SecBufVertexOffsetX | SecBufVertexOffsetY) >> (SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT - SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT);
          }
          else
          {
            _D3( aaJitterValues_4Smpl_Chp0 ) = PriBufVertexOffsetX |
                                               PriBufVertexOffsetY |
                                               SecBufVertexOffsetX |
                                               SecBufVertexOffsetY ;
          }

	        // Chip 1
	        if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_X_4SMPL_CHIP1))) )
	        { 
	            PriBufVertexOffsetX = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFX_4SMPL_CHP1_DEF)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
	        }
	        else
	        {
	            PriBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
	        }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_Y_4SMPL_CHIP1))) )
	        { 
	            PriBufVertexOffsetY = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFY_4SMPL_CHP1_DEF)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
	        }
	        else
	        {
	            PriBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
	        }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_X_4SMPL_CHIP1))) )
	        { 
	            SecBufVertexOffsetX = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFX_4SMPL_CHP1_DEF)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
	        }
	        else
	        {
	            SecBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
	        }

	        if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_Y_4SMPL_CHIP1))) )
	        { 
	            SecBufVertexOffsetY = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFY_4SMPL_CHP1_DEF)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
	        }
	        else
	        {
	            SecBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
	        }

          if (4 == _FF(dwNumUnits))
          {
            _D3( aaJitterValues_4Smpl_Chp2 ) = PriBufVertexOffsetX |
                                               PriBufVertexOffsetY;

            _D3( aaJitterValues_4Smpl_Chp3 ) = (SecBufVertexOffsetX | SecBufVertexOffsetY) >> (SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT - SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT);
          }
          else
          {
            _D3( aaJitterValues_4Smpl_Chp1 ) = PriBufVertexOffsetX |
                                               PriBufVertexOffsetY |
                                               SecBufVertexOffsetX |
                                               SecBufVertexOffsetY ;
          }

          // 8 Sample Antialiasing - 4 chip
          // Chip 0
          if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_X_8SMPL_CHIP0))) )
          { 
              PriBufVertexOffsetX = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFX_8SMPL_CHP0_DEF)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
          }
          else
          {
              PriBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
          }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_Y_8SMPL_CHIP0))) )
          { 
              PriBufVertexOffsetY = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFY_8SMPL_CHP0_DEF)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
          }
          else
          {
              PriBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
          }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_X_8SMPL_CHIP0))) )
          { 
              SecBufVertexOffsetX = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFX_8SMPL_CHP0_DEF)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
          }
          else
          {
              SecBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
          }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_Y_8SMPL_CHIP0))) )
          { 
              SecBufVertexOffsetY = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFY_8SMPL_CHP0_DEF)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
          }
          else
          {
              SecBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
          }
          
          _D3( aaJitterValues_8Smpl_Chp0 ) = PriBufVertexOffsetX |
                                             PriBufVertexOffsetY |
                                             SecBufVertexOffsetX |
                                             SecBufVertexOffsetY ;
          
          
          // Chip 1
          if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_X_8SMPL_CHIP1))) )
          { 
              PriBufVertexOffsetX = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFX_8SMPL_CHP1_DEF)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
          }
          else
          {
              PriBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
          }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_Y_8SMPL_CHIP1))) )
          { 
              PriBufVertexOffsetY = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFY_8SMPL_CHP1_DEF)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
          }
          else
          {
              PriBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
          }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_X_8SMPL_CHIP1))) )
          { 
              SecBufVertexOffsetX = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFX_8SMPL_CHP1_DEF)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
          }
          else
          {
              SecBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
          }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_Y_8SMPL_CHIP1))) )
          { 
              SecBufVertexOffsetY = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFY_8SMPL_CHP1_DEF)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
          }
          else
          {
              SecBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
          }
          
          _D3( aaJitterValues_8Smpl_Chp1 ) = PriBufVertexOffsetX |
                                             PriBufVertexOffsetY |
                                             SecBufVertexOffsetX |
                                             SecBufVertexOffsetY ;
          
          
          // Chip 2
          if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_X_8SMPL_CHIP2))) )
          { 
              PriBufVertexOffsetX = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFX_8SMPL_CHP2_DEF)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
          }
          else
          {
              PriBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
          }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_Y_8SMPL_CHIP2))) )
          { 
              PriBufVertexOffsetY = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFY_8SMPL_CHP2_DEF)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
          }
          else
          {
              PriBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
          }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_X_8SMPL_CHIP2))) )
          { 
              SecBufVertexOffsetX = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFX_8SMPL_CHP2_DEF)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
          }
          else
          {
              SecBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
          }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_Y_8SMPL_CHIP2))) )
          { 
              SecBufVertexOffsetY = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFY_8SMPL_CHP2_DEF)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
          }
          else
          {
              SecBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
          }
          
          _D3( aaJitterValues_8Smpl_Chp2 ) = PriBufVertexOffsetX |
                                             PriBufVertexOffsetY |
                                             SecBufVertexOffsetX |
                                             SecBufVertexOffsetY ;
          
          
          // Chip 3
          if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_X_8SMPL_CHIP3))) )
          { 
              PriBufVertexOffsetX = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFX_8SMPL_CHP3_DEF)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
          }
          else
          {
              PriBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_X_OFFSET;
          }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(PRI_BUF_VERTEX_OFFSET_Y_8SMPL_CHIP3))) )
          { 
              PriBufVertexOffsetY = 
                 (FLOATTO34(ddatof(PRIBUFVTXOFFY_8SMPL_CHP3_DEF)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
          }
          else
          {
              PriBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_PRIMARY_Y_OFFSET;
          }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_X_8SMPL_CHIP3))) )
          { 
              SecBufVertexOffsetX = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFX_8SMPL_CHP3_DEF)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
          }
          else
          {
              SecBufVertexOffsetX =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_X_OFFSET;
          }
          
          if( ulForceFlag || (NULL == (lpStr = GETENV(SEC_BUF_VERTEX_OFFSET_Y_8SMPL_CHIP3))) )
          { 
              SecBufVertexOffsetY = 
                 (FLOATTO34(ddatof(SECBUFVTXOFFY_8SMPL_CHP3_DEF)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                                                  &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
          }
          else
          {
              SecBufVertexOffsetY =
                 (FLOATTO34(ddatof(lpStr)) << SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT)
                                           &  SST_AA_CONTROL_SECONDARY_Y_OFFSET;
          }
          
          _D3( aaJitterValues_8Smpl_Chp3 ) = PriBufVertexOffsetX |
                                             PriBufVertexOffsetY |
                                             SecBufVertexOffsetX |
                                             SecBufVertexOffsetY ;
        }
      }
  }

  _D3(d3d).dwSize = sizeof(D3DHAL_GLOBALDRIVERDATA);
  BuildD3DCaps(ppdev, &_D3(d3d).hwCaps);
  _D3(d3d).dwNumVertices     = 0;     // let D3D allocate its own local buffer
  _D3(d3d).dwNumClipVertices = 0;
  
//repair texture format list for 3d studio max
  if( NULL == GETENV(STUDIOMAX) )
  {
    _D3( enable3DStudioMax ) = 0;
  }
  else
  {
    _D3( enable3DStudioMax ) = ATOI( GETENV(STUDIOMAX) );
  }

  if ( _D3( enable3DStudioMax ) )
  {
    DWORD         i;
    DDPIXELFORMAT pf1555 = {                                    // ddpixelformat
                              sizeof(DDPIXELFORMAT),            /* ddpfPixelFormat.dwSize */
                              DDPF_RGB | DDPF_ALPHAPIXELS,      /* ddpfPixelFormat.dwFlags */
                              0,                                /* FOURCC code */
                              16,                               /* ddpfPixelFormat.dwRGBBitCount */
                              RGB1555_RMASK,                    // dwRBitMask
                              RGB1555_GMASK,                    // dwGBitMask
                              RGB1555_BMASK,                    // dwBBitMask
                              RGB1555_AMASK                     // alpha channel mask
                           };


    for (i=1; i<_D3(d3d).dwNumTextureFormats; i++)
    {
      if ( memcmp( &pf1555, &_D3(d3d).lpTextureFormats[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT) ) == 0 )
      {
        DDSURFACEDESC tmpDesc;

        // swap format[i] with format[0],  eg for 3DStudio Max, we want format[0] to be 1555

        memcpy( &tmpDesc,                       &_D3(d3d).lpTextureFormats[0],  sizeof(DDSURFACEDESC) );
        memcpy( &_D3(d3d).lpTextureFormats[0],  &_D3(d3d).lpTextureFormats[i],  sizeof(DDSURFACEDESC) );
        memcpy( &_D3(d3d).lpTextureFormats[i],  &tmpDesc,                       sizeof(DDSURFACEDESC) );

        break;
      }
    }

  }

  // Texture management init
  TXTRINIT(ppdev);

#if !RC_LINKED_LIST
  if (NULL == _D3(contexts))
      {
      if (!CONTEXT_INIT_ARRAY(ppdev))
         return FALSE;
      }
#endif
  if (NULL == _D3(txtrDesc))
      {
      if (!DESC_INIT_ARRAY(ppdev))
         return FALSE;
      }
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
#else
  if (NULL == _D3(txtrHndl))
      {
      if (!HNDL_INIT_ARRAY(ppdev))
         return FALSE;
      }
#endif

#if defined( DEBUG )
  // Get the debug level output from the win.ini file
  if( NULL != GETENV(D3DDEBUGLEVEL) )
  {
    char *str;
    str = GETENV(D3DDEBUGLEVEL);
    setDebugLevel( str );
  }
#endif

  if( NULL == GETENV(PIXELCENTER) )
    _D3( pixelCenter ) = 1;
  else
    _D3( pixelCenter ) = ATOI( GETENV(PIXELCENTER) );

  if( NULL == GETENV(AUTOMIPMAP) )
    _D3( autoMipMap ) = 0;
  else
    _D3( autoMipMap ) = ATOI( GETENV(AUTOMIPMAP) );

  // allocate the temp buffer for largest texture supported
  if (_D3( autoMipMap ))
  {
#ifdef WINNT
    if (NULL == _D3(buffer))
    {
#endif
    if (IS_NAPALM)
      (DWORD *)_D3(buffer) = (DWORD*)D3DMALLOC(D3D_BUFFER_SIZE_TBIG, &_D3(buffer));
    else
      (DWORD *)_D3(buffer) = (DWORD*)D3DMALLOC(D3D_BUFFER_SIZE, &_D3(buffer));

    if( NULL == (DWORD *)_D3(buffer) )
    {
      D3DPRINT( 0, "WARNING: Could not allocate autoMipMap buffer." );
      _D3( autoMipMap ) = 0;
    }
#ifdef WINNT
    }
#endif
  }

  if( NULL == GETENV( LDELAY ) )
    _D3( noLockIdle ) = 0;
  else
    _D3( noLockIdle ) = ATOI( GETENV( LDELAY ) );


  _D3( sceneCount ) = 0;
  _FF( pD3context ) = (DWORD) &_D3( lastContext );
  _FF( pD3changed ) = (DWORD) &_D3( last ).changed;
  _FF( pD3colbuff ) = (DWORD) &_D3( last ).colBufferAddr;
  _FF( pD3auxbuff ) = (DWORD) &_D3( last ).auxBufferAddr;

#if defined(NULLDRIVER)
   _D3(ondrtTri) = 0;
   _D3(ondrtState) = 0;
   _D3(ondrtTexD) = 0;
   _D3(ondrtZB) = 0;
   _D3(ondrtHWSU) = 0;

  #if defined(OND_TRI)
   _D3(ondrtTri) = 1;
  #endif
  #if defined(OND_STATE)
   _D3(ondrtState) = 1;
  #endif
  #if defined(OND_ZB)
   _D3(ondrtZB) = 1;
  #endif
  #if defined(OND_HWSU)
   _D3(ondrtHWSU) = 1;
  #endif
  #if defined(OND_TEXD)
   _D3(ondrtTexD) = 1;
  #endif
#endif

#if defined( FLAVOR_PROFILE )
  renderTypes = 0;
#endif

#if defined( INSTRUMENTATION )
  // We have to do this every time because when the driver goes away the
  // handle is automatically deleted and we don't know.
  insEventHandle = CreateEvent( NULL, FALSE, FALSE, INS_EVENT_NAME );

  if( !insInitialised )
  {
    int i, j;

    insHeader.flags = INS_ENABLE;

    for( i = 0; i < INSC_MAX; i++ )
      for( j = 0; j < INSQ_MAX; j++ )
        insHeader.insCategory[i].insQuery[j] = 0;


    insInitialised = TRUE;
  }
#endif

  // Return callbacks and global data area
  *lplpGlobal = (LPD3DHAL_GLOBALDRIVERDATA)&_D3G;
  *lplpHALCallbacks = &myD3DHALCallbacks;

#ifdef K6_2
  ADJUST3DNOWPOINTERS(ppdev);
#endif

// Clear out ddFlipsWithoutZClear so we start fresh
// with every new D3D context.
#ifdef Z_ACCESS_OPT
  _DD(ddFlipsWithoutZClear) = 0;
#endif

  return TRUE;
}

void BuildDX7TextureFormats(NT9XDEVICEDATA *ppdev, DDSURFACEDESC *TextureFormats, int *pTextureFormats)
{
    int l_nIndex = 0;                   /* Texture array index value         */

    /* Initialize the texture format array */

    for (l_nIndex = 0; l_nIndex < MAX_TEXTURE_FORMATS; l_nIndex++)
    {
        TextureFormats[l_nIndex].dwSize = sizeof(DDSURFACEDESC);
        TextureFormats[l_nIndex].dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT;
        TextureFormats[l_nIndex].dwHeight = 0;
        TextureFormats[l_nIndex].dwWidth = 0;
        TextureFormats[l_nIndex].lPitch = 0;
        TextureFormats[l_nIndex].dwBackBufferCount = 0;
        TextureFormats[l_nIndex].dwZBufferBitDepth = 0;
        TextureFormats[l_nIndex].dwReserved = 0;
        TextureFormats[l_nIndex].lpSurface = 0;

        TextureFormats[l_nIndex].ddckCKDestOverlay.dwColorSpaceLowValue = 0;
        TextureFormats[l_nIndex].ddckCKDestOverlay.dwColorSpaceHighValue = 0;
        
        TextureFormats[l_nIndex].ddckCKDestBlt.dwColorSpaceLowValue = 0;
        TextureFormats[l_nIndex].ddckCKDestBlt.dwColorSpaceHighValue = 0;

        TextureFormats[l_nIndex].ddckCKSrcOverlay.dwColorSpaceLowValue = 0;
        TextureFormats[l_nIndex].ddckCKSrcOverlay.dwColorSpaceHighValue = 0;

        TextureFormats[l_nIndex].ddckCKSrcBlt.dwColorSpaceLowValue = 0;
        TextureFormats[l_nIndex].ddckCKSrcBlt.dwColorSpaceHighValue = 0;
        TextureFormats[l_nIndex].ddsCaps.dwCaps = DDSCAPS_TEXTURE;

        ZeroMemory(&TextureFormats[l_nIndex].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    }
    /* Reset the texture format array index */

    l_nIndex = 0;

    /* Define the 16-bit RGB 565 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_RGB;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBBitCount = 16;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwRBitMask = RGB565_RMASK;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwGBitMask = RGB565_GMASK;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwBBitMask = RGB565_BMASK;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBAlphaBitMask = RGB565_AMASK;
    l_nIndex++;

    /* Define the 16-bit ARGB 1555 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBBitCount = 16;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwRBitMask = RGB1555_RMASK;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwGBitMask = RGB1555_GMASK;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwBBitMask = RGB1555_BMASK;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBAlphaBitMask = RGB1555_AMASK;
    l_nIndex++;

    /* Define the 16-bit ARGB 4444 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBBitCount = 16;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwRBitMask = RGB4444_RMASK;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwGBitMask = RGB4444_GMASK;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwBBitMask = RGB4444_BMASK;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBAlphaBitMask = RGB4444_AMASK;
    l_nIndex++;

    /* Define the 8-bit RGB 332 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_RGB;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBBitCount = 8;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwRBitMask = RGB332_RMASK;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwGBitMask = RGB332_GMASK;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwBBitMask = RGB332_BMASK;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBAlphaBitMask = RGB332_AMASK;
    l_nIndex++;

#if defined(WINNT) && defined(DISABLE_PAL8_ON_NAPALM)
#else
#if !defined(WINNT) || ((DIRECT3D_VERSION >= 0x0700) && (DX >= 7))

//    if (IS_DX7)
    {    
        /* Define the 8-bit palettized texture format */

        TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBBitCount = 8;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwRBitMask = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwGBitMask = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwBBitMask = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBAlphaBitMask = 0;
        l_nIndex++;
    }
#endif  // Direct X 7
#endif  // Disable PAL 8 on Napalm

#if ((DIRECT3D_VERSION >= 0x0600) && (DX >= 6))

//    if (IS_DX6) 
    {
        /* Define the 16-bit ARGB 8332 texture format */

        TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBBitCount = 16;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwRBitMask = RGB8332_RMASK;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwGBitMask = RGB8332_GMASK;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwBBitMask = RGB8332_BMASK;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBAlphaBitMask = RGB8332_AMASK;
        l_nIndex++;

        /* Define the 8-bit luminance 8 texture format */

        TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwLuminanceBitCount = 8;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwLuminanceBitMask = L8_LMASK;
        l_nIndex++;

        /* Define the 8-bit alpha 8 texture format */

        TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_ALPHA;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBBitCount = 8;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBAlphaBitMask = A8_AMASK;
        l_nIndex++;

        /* Define the 8-bit alpha luminance 44 texture format */

        TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwLuminanceBitCount = 8;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwLuminanceBitMask = LA8_LMASK;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwLuminanceAlphaBitMask = LA8_AMASK;
        l_nIndex++;

        /* Define the 16-bit alpha luminance 88 texture format */

        TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwLuminanceBitCount = 16;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwLuminanceBitMask = LA16_LMASK;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwLuminanceAlphaBitMask = LA16_AMASK;
        l_nIndex++;

        if (IS_NAPALM)
        {
#if defined(WINNT) && defined(DISABLE_PAL8_ON_NAPALM) || TRUE
#else

            /* Define the 16-bit alpha palettized 88 texture format */

            TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS | DDPF_PALETTEINDEXED8;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBBitCount = 16;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwRBitMask = 0;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwGBitMask = 0;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwBBitMask = 0;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBAlphaBitMask = ALPHA_P8_AMASK;
            l_nIndex++;

#endif  // Disable PAL 8 on Napalm

            /* Define the 32-bit ARGB 8888 texture format */

            TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_RGB| DDPF_ALPHAPIXELS;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBBitCount = 32;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwRBitMask = RGB8888_RMASK;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwGBitMask = RGB8888_GMASK;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwBBitMask = RGB8888_BMASK;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwRGBAlphaBitMask = RGB8888_AMASK;
            l_nIndex++;

            /* Define the DXT1 compressed texture format */

            TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = FOURCC_DXT1;
            l_nIndex++;

            /* Define the DXT2 compressed texture format */

            TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = FOURCC_DXT2;
            l_nIndex++;

            /* Define the DXT3 compressed texture format */

            TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = FOURCC_DXT3;
            l_nIndex++;

            /* Define the DXT4 compressed texture format */

            TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = FOURCC_DXT4;
            l_nIndex++;

            /* Define the DXT5 compressed texture format */

            TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = FOURCC_DXT5;
            l_nIndex++;

            /* Define the FXT1 compressed texture format */

            TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
            TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = FOURCC_FXT1;
            l_nIndex++;
        }
    }

#endif // Direct X 6

    /* Save the number of texture formats declared */

    *pTextureFormats = l_nIndex;

    /* Return control to the caller */

    return;
}

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)

void BuildDX8TextureFormats(NT9XDEVICEDATA *ppdev, DDSURFACEDESC *TextureFormats, int *pTextureFormats)
{
    int l_nIndex = 0;                   /* Texture array index value         */

    /* Initialize the texture format array */

    for (l_nIndex = 0; l_nIndex < MAX_TEXTURE_FORMATS; l_nIndex++)
    {
        TextureFormats[l_nIndex].dwSize = sizeof(DDSURFACEDESC);
        TextureFormats[l_nIndex].dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT;
        TextureFormats[l_nIndex].dwHeight = 0;
        TextureFormats[l_nIndex].dwWidth = 0;
        TextureFormats[l_nIndex].lPitch = 0;
        TextureFormats[l_nIndex].dwBackBufferCount = 0;
        TextureFormats[l_nIndex].dwZBufferBitDepth = 0;
        TextureFormats[l_nIndex].dwReserved = 0;
        TextureFormats[l_nIndex].lpSurface = 0;

        TextureFormats[l_nIndex].ddckCKDestOverlay.dwColorSpaceLowValue = 0;
        TextureFormats[l_nIndex].ddckCKDestOverlay.dwColorSpaceHighValue = 0;
        
        TextureFormats[l_nIndex].ddckCKDestBlt.dwColorSpaceLowValue = 0;
        TextureFormats[l_nIndex].ddckCKDestBlt.dwColorSpaceHighValue = 0;

        TextureFormats[l_nIndex].ddckCKSrcOverlay.dwColorSpaceLowValue = 0;
        TextureFormats[l_nIndex].ddckCKSrcOverlay.dwColorSpaceHighValue = 0;

        TextureFormats[l_nIndex].ddckCKSrcBlt.dwColorSpaceLowValue = 0;
        TextureFormats[l_nIndex].ddckCKSrcBlt.dwColorSpaceHighValue = 0;
        TextureFormats[l_nIndex].ddsCaps.dwCaps = DDSCAPS_TEXTURE;

        ZeroMemory(&TextureFormats[l_nIndex].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    }
    /* Reset the texture format array index */

    l_nIndex = 0;

    /* Define texture and display mode formats */
    /* Define the DX 8 16-bit RGB 565 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_R5G6B5;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_DISPLAYMODE | D3DFORMAT_OP_3DACCELERATION | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the texture only formats (Not displayable) */
    /* Define the DX 8 16-bit XRGB 1555 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_X1R5G5B5;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DX 8 16-bit ARGB 1555 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_A1R5G5B5;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DX 8 16-bit XRGB 4444 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_X4R4G4B4;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DX 8 16-bit ARGB 4444 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_A4R4G4B4;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DX 8 8-bit RGB 332 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_R3G3B2;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DX 8 16-bit ARGB 8332 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_A8R3G3B2;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DX 8 8-bit luminance 8 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_L8;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DX 8 8-bit alpha 8 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_A8;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DX 8 8-bit alpha luminance 44 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_A4L4;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DX 8 16-bit alpha luminance 88 texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_A8L8;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the Z buffer/stencil formats (16-Bit same display mode only) */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_D16_LOCKABLE;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_ZSTENCIL;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the compressed texture formats */
    /* Define the DXT1 compressed texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_DXT1;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DXT2 compressed texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_DXT2;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DXT3 compressed texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_DXT3;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DXT4 compressed texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_DXT4;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the DXT5 compressed texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_DXT5;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Define the FXT1 compressed texture format */

    TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = FOURCC_FXT1;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE;
    TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    l_nIndex++;

    /* Update the format list for Napalm */

    if (IS_NAPALM)
    {
        /* Define Napalm only texture and display mode formats */
        /* Define the DX 8 32-bit XRGB 8888 texture format */
        TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_X8R8G8B8;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_DISPLAYMODE | D3DFORMAT_OP_3DACCELERATION | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
        l_nIndex++;

        /* Define the DX 8 32-bit ARGB 8888 texture format */

        TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_A8R8G8B8;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET | D3DFORMAT_OP_SAME_FORMAT_UP_TO_ALPHA_RENDERTARGET;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
        l_nIndex++;

        /* Define the Napalm only texture only formats (Not displayable) */

#if !defined(DISABLE_PAL8_ON_NAPALM)

        /* Define the DX 8 8-bit palettized texture format */

        TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_P8;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
        l_nIndex++;

        /* Define the DX 8 16-bit alpha palettized 88 texture format */

        TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_A8P8;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_TEXTURE | D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
        l_nIndex++;

#endif  // Disable PAL 8 on Napalm

        /* Define the Napalm only Z buffer/stencil formats */

        TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_D16;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_ZSTENCIL | D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
        l_nIndex++;

        TextureFormats[l_nIndex].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwFourCC = D3DFMT_S8D24;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwOperations = D3DFORMAT_OP_ZSTENCIL;
        TextureFormats[l_nIndex].ddpfPixelFormat.dwPrivateFormatBitCount = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
        TextureFormats[l_nIndex].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
        l_nIndex++;

        /* Define the Napalm only compressed texture formats */




    }
    /* Save the number of texture formats declared */

    *pTextureFormats = l_nIndex;

    /* Return control to the caller */

    return;
}

#endif //(DIRECT3D_VERSION >= 0x0800) && (DX >= 8)

void BuildD3DCaps(NT9XDEVICEDATA *ppdev, D3DDEVICEDESC_V1 *pCaps)
{
    /* Initialize the D3D Capabilities Structure (pCaps) */

    memset(pCaps, 0, sizeof(D3DDEVICEDESC_V1));

    /* dwSize (Structure Size) */

    pCaps->dwSize =             sizeof(D3DDEVICEDESC_V1);

    /* dwFlags (Valid Caps Fields) */

    pCaps->dwFlags =            0                                             |
                                D3DDD_COLORMODEL                              |
                                D3DDD_DEVCAPS                                 |
                                D3DDD_DEVICERENDERBITDEPTH                    |
                                D3DDD_DEVICEZBUFFERBITDEPTH                   |
//                              D3DDD_TRANSFORMCAPS                           |
//                              D3DDD_LIGHTINGCAPS                            |
//                              D3DDD_BCLIPPING                               |
//                              D3DDD_MAXBUFFERSIZE                           |
//                              D3DDD_MAXVERTEXCOUNT                          |
//                              D3DDD_LPD3DDEVSURFACE                         |
                                D3DDD_LINECAPS                                |
                                D3DDD_TRICAPS                                 |
                                0;

    /* dcmColorModel (Color Model) */

    pCaps->dcmColorModel =      D3DCOLOR_RGB;

    /* dwDevCaps (Device Caps) */

    pCaps->dwDevCaps =          0                                             |
                                D3DDEVCAPS_FLOATTLVERTEX                      |
//                              D3DDEVCAPS_SORTINCREASINGZ                    |
//                              D3DDEVCAPS_SORTDECREASINGZ                    |
//                              D3DDEVCAPS_SORTEXACT                          |
                                D3DDEVCAPS_EXECUTESYSTEMMEMORY                |
//                              D3DDEVCAPS_EXECUTEVIDEOMEMORY                 |
//                              D3DDEVCAPS_TLVERTEXVIDEOMEMORY                |
//                              D3DDEVCAPS_TEXTURESYSTEMMEMORY                |
                                D3DDEVCAPS_DRAWPRIMTLVERTEX                   |
                                D3DDEVCAPS_CANRENDERAFTERFLIP                 |
//                              D3DDEVCAPS_TEXTURENONLOCALVIDMEM              |
                                D3DDEVCAPS_TEXTUREVIDEOMEMORY                 |
#if( DX >= 6 )
                                D3DDEVCAPS_DRAWPRIMITIVES2                    |
#if (DIRECT3D_VERSION >=0x0700) && (DX >= 7)
                                D3DDEVCAPS_DRAWPRIMITIVES2EX                  |
//                              D3DDEVCAPS_HWTRANSFORMANDLIGHT                |
#endif
#endif
                                0;

    /* Modify Device Capabilities Based on Software TnL Enabled */

#if defined(TnL_HAL)
#ifndef WINNT
    if(_D3(TransformAndLighting) != TnL_DISABLED)
    {
      // need to turn this on ony for sse or extended 3dnow cpu's (and TnL enabled byt
      if( ((CPUTYPE & CPU_FEATURE_MMX) && (CPUTYPE & CPU_FEATURE_SSE)) ||                                   // p3 or better
          ((CPUTYPE & CPU_FEATURE_MMX) && (CPUTYPE & CPU_FEATURE_AMMX) && (CPUTYPE & CPU_FEATURE_3DNOWX)) ) // athlons or better
        pCaps->dwDevCaps |=     0                                             |
                                D3DDEVCAPS_TLVERTEXVIDEOMEMORY                |
                                D3DDEVCAPS_HWTRANSFORMANDLIGHT                |
                                0;

    }
#endif
#endif

    /* Initialize Transformation Capabilities SubStructure (dtcTransformCaps) */

    memset(&pCaps->dtcTransformCaps, 0, sizeof(D3DTRANSFORMCAPS));

    /* dwSize (Structure Size) */

    pCaps->dtcTransformCaps.dwSize =    sizeof(D3DTRANSFORMCAPS);

    /* dtcTransformCaps (Transformation Caps [None]) */

    pCaps->dtcTransformCaps.dwCaps =    0;

    /* bClipping (3D Clipping [No]) */

    pCaps->bClipping =          FALSE;

    /* Initialize the Lighting Capabilities SubStructure (dlcLightingCaps) */

    memset(&pCaps->dlcLightingCaps, 0, sizeof(D3DLIGHTINGCAPS));

    /* dwSize (Structure Size) */

    pCaps->dlcLightingCaps.dwSize =     sizeof(D3DLIGHTINGCAPS);

    /* dlcLightingCaps (Lighting Caps [None]) */

    pCaps->dlcLightingCaps.dwCaps =     0;

    /* Initialize the Line Capabilities SubStructure (dpcLineCaps) */

    memset(&pCaps->dpcLineCaps, 0, sizeof(D3DPRIMCAPS));

    /* dwSize (Structure Size) */

    pCaps->dpcLineCaps.dwSize =         sizeof(D3DPRIMCAPS);

    /* dwMiscCaps (Capability Flags) */

    pCaps->dpcLineCaps.dwMiscCaps =     0                                     |
//                                      D3DPMISCCAPS_MASKPLANES               |
                                        D3DPMISCCAPS_MASKZ                    |
//                                      D3DPMISCCAPS_LINEPATTERNREP           |
//                                      D3DPMISCCAPS_CONFORMANT               |
                                        D3DPMISCCAPS_CULLNONE                 |
                                        D3DPMISCCAPS_CULLCW                   |
                                        D3DPMISCCAPS_CULLCCW                  |
                                        0;

    /* dwRasterCaps (Rasterization Caps) */

    pCaps->dpcLineCaps.dwRasterCaps =   0                                     |
//                                      D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT |
                                        D3DPRASTERCAPS_DITHER                 |
//                                      D3DPRASTERCAPS_ROP2                   |
//                                      D3DPRASTERCAPS_XOR                    |
//                                      D3DPRASTERCAPS_PAT                    |
//                                      D3DPRASTERCAPS_ZTEST                  |
                                        D3DPRASTERCAPS_SUBPIXEL               |
//                                      D3DPRASTERCAPS_SUBPIXELX              |
                                        D3DPRASTERCAPS_FOGVERTEX              |
                                        D3DPRASTERCAPS_FOGTABLE               |
                                        D3DPRASTERCAPS_ZBIAS                  |
//                                      D3DPRASTERCAPS_STIPPLE                |
#if( DX >= 6 )
                                        D3DPRASTERCAPS_WBUFFER                |
                                        D3DPRASTERCAPS_WFOG                   |
#endif
                                        0;

    /* dwZCmpCaps (Z Compare Caps) */

    pCaps->dpcLineCaps.dwZCmpCaps =     0                                     |
                                        D3DPCMPCAPS_NEVER                     |
                                        D3DPCMPCAPS_LESS                      |
                                        D3DPCMPCAPS_EQUAL                     |
                                        D3DPCMPCAPS_LESSEQUAL                 |
                                        D3DPCMPCAPS_GREATER                   |
                                        D3DPCMPCAPS_NOTEQUAL                  |
                                        D3DPCMPCAPS_GREATEREQUAL              |
                                        D3DPCMPCAPS_ALWAYS                    |
                                        0;

    /* dwSrcBlendCaps (Source Blend Caps) */

    pCaps->dpcLineCaps.dwSrcBlendCaps = 0                                     |
                                        D3DPBLENDCAPS_ZERO                    |
                                        D3DPBLENDCAPS_ONE                     |
//                                      D3DPBLENDCAPS_SRCCOLOR                |
//                                      D3DPBLENDCAPS_INVSRCCOLOR             |
                                        D3DPBLENDCAPS_SRCALPHA                |
                                        D3DPBLENDCAPS_INVSRCALPHA             |
//                                      D3DPBLENDCAPS_DESTALPHA               |
//                                      D3DPBLENDCAPS_INVDESTALPHA            |
                                        D3DPBLENDCAPS_DESTCOLOR               |
                                        D3DPBLENDCAPS_INVDESTCOLOR            |
//                                      D3DPBLENDCAPS_SRCALPHASAT             |
                                        D3DPBLENDCAPS_BOTHSRCALPHA            |
                                        D3DPBLENDCAPS_BOTHINVSRCALPHA         |
                                        0;

    /* dwDestBlend (Destination Blend Caps) */

    pCaps->dpcLineCaps.dwDestBlendCaps = 0                                    |
                                        D3DPBLENDCAPS_ZERO                    |
                                        D3DPBLENDCAPS_ONE                     |
                                        D3DPBLENDCAPS_SRCCOLOR                |
                                        D3DPBLENDCAPS_INVSRCCOLOR             |
                                        D3DPBLENDCAPS_SRCALPHA                |
                                        D3DPBLENDCAPS_INVSRCALPHA             |
//                                      D3DPBLENDCAPS_DESTALPHA               |
//                                      D3DPBLENDCAPS_INVDESTALPHA            |
//                                      D3DPBLENDCAPS_DESTCOLOR               |
//                                      D3DPBLENDCAPS_INVDESTCOLOR            |
//                                      D3DPBLENDCAPS_SRCALPHASAT             |
                                        D3DPBLENDCAPS_BOTHSRCALPHA            |
                                        D3DPBLENDCAPS_BOTHINVSRCALPHA         |
                                        0;

    /* dwAlphaCmpCaps (Alpha Compare Caps) */

    pCaps->dpcLineCaps.dwAlphaCmpCaps = 0                                     |
                                        D3DPCMPCAPS_NEVER                     |
                                        D3DPCMPCAPS_LESS                      |
                                        D3DPCMPCAPS_EQUAL                     |
                                        D3DPCMPCAPS_LESSEQUAL                 |
                                        D3DPCMPCAPS_GREATER                   |
                                        D3DPCMPCAPS_NOTEQUAL                  |
                                        D3DPCMPCAPS_GREATEREQUAL              |
                                        D3DPCMPCAPS_ALWAYS                    |
                                        0;

    /* dwShadeCaps (Shade Caps) */

    pCaps->dpcLineCaps.dwShadeCaps =    0                                     |
//                                      D3DPSHADECAPS_COLORFLATMONO           |
                                        D3DPSHADECAPS_COLORFLATRGB            |
//                                      D3DPSHADECAPS_COLORGOURAUDMONO        |
                                        D3DPSHADECAPS_COLORGOURAUDRGB         |
//                                      D3DPSHADECAPS_COLORPHONGMONO          |
//                                      D3DPSHADECAPS_COLORPHONGRGB           |
                                        D3DPSHADECAPS_SPECULARFLATMONO        |
                                        D3DPSHADECAPS_SPECULARFLATRGB         |
                                        D3DPSHADECAPS_SPECULARGOURAUDMONO     |
                                        D3DPSHADECAPS_SPECULARGOURAUDRGB      |
//                                      D3DPSHADECAPS_SPECULARPHONGMONO       |
//                                      D3DPSHADECAPS_SPECULARPHONGRGB        |
                                        D3DPSHADECAPS_ALPHAFLATBLEND          |
//                                      D3DPSHADECAPS_ALPHAFLATSTIPPLED       |
                                        D3DPSHADECAPS_ALPHAGOURAUDBLEND       |
//                                      D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED    |
//                                      D3DPSHADECAPS_ALPHAPHONGBLEND         |
//                                      D3DPSHADECAPS_ALPHAPHONGSTIPPLED      |
                                        D3DPSHADECAPS_FOGFLAT                 |
//                                      D3DPSHADECAPS_FOGPHONG                |
                                        D3DPSHADECAPS_FOGGOURAUD              |
                                        0;

    /* dwTextureCaps (Texture Caps) */

    pCaps->dpcLineCaps.dwTextureCaps =  0                                     |
#if( DX >= 6 )
                                        D3DPTEXTURECAPS_ALPHAPALETTE          |
#endif
                                        D3DPTEXTURECAPS_PERSPECTIVE           |
                                        D3DPTEXTURECAPS_POW2                  |
                                        D3DPTEXTURECAPS_ALPHA                 |
#ifdef TEXTURE_REPEAT_NOT_SCALED
                                        D3DPTEXTURECAPS_TRANSPARENCY          |
                                        D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE|
#else
                                        D3DPTEXTURECAPS_TRANSPARENCY          |
#endif
//                                      D3DPTEXTURECAPS_BORDER                |
//                                      D3DPTEXTURECAPS_SQUAREONLY            |
                                        0;

    /* dwTextureFilterCaps (Texture Filter Caps) */

    pCaps->dpcLineCaps.dwTextureFilterCaps = 0                                |
                                        D3DPTFILTERCAPS_NEAREST               |
                                        D3DPTFILTERCAPS_LINEAR                |
                                        D3DPTFILTERCAPS_MIPNEAREST            |
                                        D3DPTFILTERCAPS_MIPLINEAR             |
                                        D3DPTFILTERCAPS_LINEARMIPNEAREST      |
                                        D3DPTFILTERCAPS_LINEARMIPLINEAR       |
#if ( DX >= 6 )
                                        D3DPTFILTERCAPS_MINFPOINT             |
                                        D3DPTFILTERCAPS_MINFLINEAR            |
                                        D3DPTFILTERCAPS_MIPFPOINT             |
                                        D3DPTFILTERCAPS_MIPFLINEAR            |
                                        D3DPTFILTERCAPS_MAGFPOINT             |
                                        D3DPTFILTERCAPS_MAGFLINEAR            |
#endif
                                        0;

    /* dwTextureBlendCaps (Texture Blend Caps) */

    pCaps->dpcLineCaps.dwTextureBlendCaps = 0                                 |
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(REMOVE_OBSOLETE1_FOR_DX7_WIN9X) //cws obsolete1 dx7
#else
                                        D3DPTBLENDCAPS_DECAL                  |
                                        D3DPTBLENDCAPS_MODULATE               |
                                        D3DPTBLENDCAPS_DECALALPHA             |
                                        D3DPTBLENDCAPS_MODULATEALPHA          |
//                                      D3DPTBLENDCAPS_DECALMASK              | Removed for PRS #8369 - hw does not support this.
//                                      D3DPTBLENDCAPS_MODULATEMASK           | Removed for PRS #8369 - hw does not support this.
                                        D3DPTBLENDCAPS_COPY                   |
                                        D3DPTBLENDCAPS_ADD                    |
#endif
                                        0;

    /* dwTextureAddressCaps (Texture Address Caps) */

    pCaps->dpcLineCaps.dwTextureAddressCaps = 0                               |
                                        D3DPTADDRESSCAPS_WRAP                 |
#if ( DX >= 6 )
                                        D3DPTADDRESSCAPS_MIRROR               |
#endif
//                                      D3DPTADDRESSCAPS_BORDER               |
                                        D3DPTADDRESSCAPS_INDEPENDENTUV        |
                                        D3DPTADDRESSCAPS_CLAMP                |
                                        0;

    /* dwStippleWidth (Maximum Stipple Width) */

    pCaps->dpcLineCaps.dwStippleWidth = 0;

    /* dwStippleHeight (Maximum Stipple Height) */

    pCaps->dpcLineCaps.dwStippleHeight = 0;

    /* Initialize the Triangle Capabilities SubStructure (dpcTriCaps) */

    memset(&pCaps->dpcTriCaps, 0, sizeof(D3DPRIMCAPS));

    /* dwSize (Structure Size) */

    pCaps->dpcTriCaps.dwSize =          sizeof(D3DPRIMCAPS);

    /* dwMiscCaps (Capability Flags) */

    pCaps->dpcTriCaps.dwMiscCaps =      0                                     |
//                                      D3DPMISCCAPS_MASKPLANES               |
                                        D3DPMISCCAPS_MASKZ                    |
//                                      D3DPMISCCAPS_LINEPATTERNREP           |
//                                      D3DPMISCCAPS_CONFORMANT               |
                                        D3DPMISCCAPS_CULLNONE                 |
                                        D3DPMISCCAPS_CULLCW                   |
                                        D3DPMISCCAPS_CULLCCW                  |
                                        0;

    /* dwRasterCaps (Rasterization Caps) */

    pCaps->dpcTriCaps.dwRasterCaps =    0                                     |
//                                      D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT |
                                        D3DPRASTERCAPS_DITHER                 |
//                                      D3DPRASTERCAPS_ROP2                   |
//                                      D3DPRASTERCAPS_XOR                    |
//                                      D3DPRASTERCAPS_PAT                    |
//                                      D3DPRASTERCAPS_ZTEST                  |
                                        D3DPRASTERCAPS_SUBPIXEL               |
//                                      D3DPRASTERCAPS_SUBPIXELX              |
                                        D3DPRASTERCAPS_FOGVERTEX              |
                                        D3DPRASTERCAPS_FOGTABLE               |
                                        D3DPRASTERCAPS_ZBIAS                  |
//                                      D3DPRASTERCAPS_STIPPLE                |
#if( DX >= 6 )
                                        D3DPRASTERCAPS_WBUFFER                |
                                        D3DPRASTERCAPS_WFOG                   |
#endif
                                        0;

    /* dwZCmpCaps (Z Compare Caps) */

    pCaps->dpcTriCaps.dwZCmpCaps =      0                                     |
                                        D3DPCMPCAPS_NEVER                     |
                                        D3DPCMPCAPS_LESS                      |
                                        D3DPCMPCAPS_EQUAL                     |
                                        D3DPCMPCAPS_LESSEQUAL                 |
                                        D3DPCMPCAPS_GREATER                   |
                                        D3DPCMPCAPS_NOTEQUAL                  |
                                        D3DPCMPCAPS_GREATEREQUAL              |
                                        D3DPCMPCAPS_ALWAYS                    |
                                        0;

    /* dwSrcBlendCaps (Source Blend Caps) */

    pCaps->dpcTriCaps.dwSrcBlendCaps =  0                                     |
                                        D3DPBLENDCAPS_ZERO                    |
                                        D3DPBLENDCAPS_ONE                     |
//                                      D3DPBLENDCAPS_SRCCOLOR                |
//                                      D3DPBLENDCAPS_INVSRCCOLOR             |
                                        D3DPBLENDCAPS_SRCALPHA                |
                                        D3DPBLENDCAPS_INVSRCALPHA             |
//                                      D3DPBLENDCAPS_DESTALPHA               |
//                                      D3DPBLENDCAPS_INVDESTALPHA            |
                                        D3DPBLENDCAPS_DESTCOLOR               |
                                        D3DPBLENDCAPS_INVDESTCOLOR            |
//                                      D3DPBLENDCAPS_SRCALPHASAT             |
                                        D3DPBLENDCAPS_BOTHSRCALPHA            |
                                        D3DPBLENDCAPS_BOTHINVSRCALPHA         |
                                        0;

    /* dwDestBlend (Destination Blend Caps) */

    pCaps->dpcTriCaps.dwDestBlendCaps = 0                                     |
                                        D3DPBLENDCAPS_ZERO                    |
                                        D3DPBLENDCAPS_ONE                     |
                                        D3DPBLENDCAPS_SRCCOLOR                |
                                        D3DPBLENDCAPS_INVSRCCOLOR             |
                                        D3DPBLENDCAPS_SRCALPHA                |
                                        D3DPBLENDCAPS_INVSRCALPHA             |
//                                      D3DPBLENDCAPS_DESTALPHA               |
//                                      D3DPBLENDCAPS_INVDESTALPHA            |
//                                      D3DPBLENDCAPS_DESTCOLOR               |
//                                      D3DPBLENDCAPS_INVDESTCOLOR            |
//                                      D3DPBLENDCAPS_SRCALPHASAT             |
                                        D3DPBLENDCAPS_BOTHSRCALPHA            |
                                        D3DPBLENDCAPS_BOTHINVSRCALPHA         |
                                        0;

    /* dwAlphaCmpCaps (Alpha Compare Caps) */

    pCaps->dpcTriCaps.dwAlphaCmpCaps =  0                                     |
                                        D3DPCMPCAPS_NEVER                     |
                                        D3DPCMPCAPS_LESS                      |
                                        D3DPCMPCAPS_EQUAL                     |
                                        D3DPCMPCAPS_LESSEQUAL                 |
                                        D3DPCMPCAPS_GREATER                   |
                                        D3DPCMPCAPS_NOTEQUAL                  |
                                        D3DPCMPCAPS_GREATEREQUAL              |
                                        D3DPCMPCAPS_ALWAYS                    |
                                        0;

    /* dwShadeCaps (Shade Caps) */

    pCaps->dpcTriCaps.dwShadeCaps =     0                                     |
//                                      D3DPSHADECAPS_COLORFLATMONO           |
                                        D3DPSHADECAPS_COLORFLATRGB            |
//                                      D3DPSHADECAPS_COLORGOURAUDMONO        |
                                        D3DPSHADECAPS_COLORGOURAUDRGB         |
//                                      D3DPSHADECAPS_COLORPHONGMONO          |
//                                      D3DPSHADECAPS_COLORPHONGRGB           |
                                        D3DPSHADECAPS_SPECULARFLATMONO        |
                                        D3DPSHADECAPS_SPECULARFLATRGB         |
                                        D3DPSHADECAPS_SPECULARGOURAUDMONO     |
                                        D3DPSHADECAPS_SPECULARGOURAUDRGB      |
//                                      D3DPSHADECAPS_SPECULARPHONGMONO       |
//                                      D3DPSHADECAPS_SPECULARPHONGRGB        |
                                        D3DPSHADECAPS_ALPHAFLATBLEND          |
//                                      D3DPSHADECAPS_ALPHAFLATSTIPPLED       |
                                        D3DPSHADECAPS_ALPHAGOURAUDBLEND       |
//                                      D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED    |
//                                      D3DPSHADECAPS_ALPHAPHONGBLEND         |
//                                      D3DPSHADECAPS_ALPHAPHONGSTIPPLED      |
                                        D3DPSHADECAPS_FOGFLAT                 |
                                        D3DPSHADECAPS_FOGGOURAUD              |
//                                      D3DPSHADECAPS_FOGPHONG                |
                                        0;

    /* dwTextureCaps (Texture Caps) */

    pCaps->dpcTriCaps.dwTextureCaps =   0                                     |
#if( DX >= 6 )
                                        D3DPTEXTURECAPS_ALPHAPALETTE          |
#endif
                                        D3DPTEXTURECAPS_PERSPECTIVE           |
                                        D3DPTEXTURECAPS_POW2                  |
                                        D3DPTEXTURECAPS_ALPHA                 |
#ifdef TEXTURE_REPEAT_NOT_SCALED
                                        D3DPTEXTURECAPS_TRANSPARENCY          |
                                        D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE|
#else
                                        D3DPTEXTURECAPS_TRANSPARENCY          |
#endif
//                                      D3DPTEXTURECAPS_BORDER                |
//                                      D3DPTEXTURECAPS_SQUAREONLY            |

#if(DIRECT3D_VERSION >= 0x0700) && ( DX >= 7 )
//                                      D3DPTEXTURECAPS_PROJECTED             | /* Device can do D3DTTFF_PROJECTED */
//                                      D3DPTEXTURECAPS_CUBEMAP               | /* Device can do cubemap textures */
//                                      D3DPTEXTURECAPS_COLORKEYBLEND
#endif

                                        0;

    /* dwTextureFilterCaps (Texture Filter Caps) */

    pCaps->dpcTriCaps.dwTextureFilterCaps = 0                                 |
                                        D3DPTFILTERCAPS_NEAREST               |
                                        D3DPTFILTERCAPS_LINEAR                |
                                        D3DPTFILTERCAPS_MIPNEAREST            |
                                        D3DPTFILTERCAPS_MIPLINEAR             |
                                        D3DPTFILTERCAPS_LINEARMIPNEAREST      |
                                        D3DPTFILTERCAPS_LINEARMIPLINEAR       |
#if ( DX >= 6 )
                                        D3DPTFILTERCAPS_MINFPOINT             |
                                        D3DPTFILTERCAPS_MINFLINEAR            |
                                        D3DPTFILTERCAPS_MIPFPOINT             |
                                        D3DPTFILTERCAPS_MIPFLINEAR            |
                                        D3DPTFILTERCAPS_MAGFPOINT             |
                                        D3DPTFILTERCAPS_MAGFLINEAR            |
#endif
                                        0;

    /* dwTextureBlendCaps (Texture Blend Caps) */

    pCaps->dpcTriCaps.dwTextureBlendCaps = 0                                  |
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(REMOVE_OBSOLETE1_FOR_DX7_WIN9X) //cws obsolete1 dx7
#else
                                        D3DPTBLENDCAPS_DECAL                  |
                                        D3DPTBLENDCAPS_MODULATE               |
                                        D3DPTBLENDCAPS_DECALALPHA             |
                                        D3DPTBLENDCAPS_MODULATEALPHA          |
//                                      D3DPTBLENDCAPS_DECALMASK              | Removed for PRS #8369 - hw does not support this.
//                                      D3DPTBLENDCAPS_MODULATEMASK           | Removed for PRS #8369 - hw does not support this.
                                        D3DPTBLENDCAPS_COPY                   |
                                        D3DPTBLENDCAPS_ADD                    |
#endif
                                        0;

    /* dwTextureAddressCaps (Texture Address Caps) */

    pCaps->dpcTriCaps.dwTextureAddressCaps = 0                                |
                                        D3DPTADDRESSCAPS_WRAP                 |
#if ( DX >= 6 )
                                        D3DPTADDRESSCAPS_MIRROR               |
#endif
                                        D3DPTADDRESSCAPS_INDEPENDENTUV        |
                                        D3DPTADDRESSCAPS_CLAMP                |
                                        0;

    /* dwStippleWidth (Maximum Stipple Width) */

    pCaps->dpcTriCaps.dwStippleWidth =  0;

    /* dwStippleHeight (Maximum Stipple Height) */

    pCaps->dpcTriCaps.dwStippleHeight = 0;

    /* dwDeviceRenderBitDepth (Rendering Bit Depths) [16 BPP] */

    pCaps->dwDeviceRenderBitDepth = DDBD_16;

    /* dwDeviceZBufferBitDepth (Z Buffer Bit Depths) [16 BPP] */

    pCaps->dwDeviceZBufferBitDepth = DDBD_16;

    /* dwMaxBufferSize (Maximum Execute Buffer Size) */

    pCaps->dwMaxBufferSize =    0;

    /* dwMaxVertexCount (Maximum Vertex Count) */

    pCaps->dwMaxVertexCount =   0;

    /* Check for Voodoo3 and Disable any Requested Capabilities */

#if defined( H3 )
    if ( _D3( enableTrilinear ) == 0 )
    {
        /* Disable TriLinear Mip Mapping */

        pCaps->dpcLineCaps.dwTextureFilterCaps &= ~(D3DPTFILTERCAPS_LINEARMIPNEAREST |
                                                    D3DPTFILTERCAPS_LINEARMIPLINEAR  );


        pCaps->dpcTriCaps.dwTextureFilterCaps &=  ~(D3DPTFILTERCAPS_LINEARMIPNEAREST |
                                                    D3DPTFILTERCAPS_LINEARMIPLINEAR  );
    }
#endif

    /* Check for Napalm and Enable Napalm Specific Capabilities */

    if (IS_NAPALM)
    {
        /* Enable New Alpha Blending Modes (Src*Src, Dst*Dst, [Src/Dst]*DstAlpha) */

        pCaps->dpcLineCaps.dwSrcBlendCaps |=    D3DPBLENDCAPS_SRCCOLOR        |
                                                D3DPBLENDCAPS_INVSRCCOLOR     |
                                                D3DPBLENDCAPS_DESTALPHA       |
                                                D3DPBLENDCAPS_INVDESTALPHA    |
                                                D3DPBLENDCAPS_SRCALPHASAT;

        pCaps->dpcTriCaps.dwSrcBlendCaps |=     D3DPBLENDCAPS_SRCCOLOR        |
                                                D3DPBLENDCAPS_INVSRCCOLOR     |
                                                D3DPBLENDCAPS_DESTALPHA       |
                                                D3DPBLENDCAPS_INVDESTALPHA    |
                                                D3DPBLENDCAPS_SRCALPHASAT;

        pCaps->dpcLineCaps.dwDestBlendCaps |=   D3DPBLENDCAPS_DESTALPHA       |
                                                D3DPBLENDCAPS_INVDESTALPHA    |
                                                D3DPBLENDCAPS_DESTCOLOR       |
                                                D3DPBLENDCAPS_INVDESTCOLOR    |
                                                D3DPBLENDCAPS_SRCALPHASAT;

        pCaps->dpcTriCaps.dwDestBlendCaps |=    D3DPBLENDCAPS_DESTALPHA       |
                                                D3DPBLENDCAPS_INVDESTALPHA    |
                                                D3DPBLENDCAPS_DESTCOLOR       |
                                                D3DPBLENDCAPS_INVDESTCOLOR    |
                                                D3DPBLENDCAPS_SRCALPHASAT;

        /* Enable Sort Independent Anti-Aliasing (Lines and Triangles) */

        pCaps->dpcLineCaps.dwRasterCaps |=      D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT;
        pCaps->dpcTriCaps.dwRasterCaps |=       D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT;

        /* Enable 32 BPP Rendering/24 BPP Z Buffering if Enabled */

        if (_D3( thirtyTwoBppRendering ))
        {
            pCaps->dwDeviceRenderBitDepth |=    DDBD_32;
            pCaps->dwDeviceZBufferBitDepth |=   DDBD_24;
        }
    }
    /* Completed Building the D3D Capabilities Structure, Return to Caller */

    return;
}

void BuildD3DExtendedCaps(NT9XDEVICEDATA *ppdev, D3DHAL_D3DEXTENDEDCAPS *pCaps)
{
    /* Initialize the D3D Extended Capabilities Structure (pCaps) */

    memset(pCaps, 0, sizeof(D3DHAL_D3DEXTENDEDCAPS));

    /* dwSize (Structure Size) */

    pCaps->dwSize =                 sizeof(D3DHAL_D3DEXTENDEDCAPS);

    /* dwMinTextureWidth, dwMaxTextureWidth, dwMinTextureHeight, dwMaxTextureHeight */

    if (IS_NAPALM)
    {
        pCaps->dwMinTextureWidth =  1;
        pCaps->dwMaxTextureWidth =  2048;
        pCaps->dwMinTextureHeight = 1;
        pCaps->dwMaxTextureHeight = 2048;
    }
    else    /* Non-Napalm */
    {
        pCaps->dwMinTextureWidth =  1;
        pCaps->dwMaxTextureWidth =  256;
        pCaps->dwMinTextureHeight = 1;
        pCaps->dwMaxTextureHeight = 256;
    }
    /* dwMaxTextureRepeat (Maximum Times a Texture may be Repeated [Tiled]) */

#if( DIRECTDRAW_VERSION >= 0x0600 )
#ifdef TEXTURE_REPEAT_NOT_SCALED
    pCaps->dwMaxTextureRepeat =     256;
#else
    pCaps->dwMaxTextureRepeat =     32768;
#endif

    /* dwMaxTextureAspectRatio (Maximum Texture Aspect Ratio) */

    pCaps->dwMaxTextureAspectRatio= 8;

    /* dwMaxAnisotrophy (Maximum Anisotropy Value) */

    pCaps->dwMaxAnisotropy =        1;

    /* dvGuardBandLeft, dvGuardBandTop, dvGuardBandRight, dvGuardBandBottom (Guard Band Limits) */

    if (IS_NAPALM && _D3(GuardbandClipping))   //cws gb
    {
        pCaps->dvGuardBandLeft =    -200.f;
        pCaps->dvGuardBandTop =     -200.f;
        pCaps->dvGuardBandRight =   2048.f;
        pCaps->dvGuardBandBottom =  1536.f;
    }
    else    /* Non-Napalm */
    {
        pCaps->dvGuardBandLeft =    0.f;
        pCaps->dvGuardBandTop =     0.f;
        pCaps->dvGuardBandRight =   0.f;
        pCaps->dvGuardBandBottom =  0.f;
    }
    /* dvExtentsAdjust (Extents Adjustment Value) */

    pCaps->dvExtentsAdjust =        0.f;

    /* dwStencilCaps (Stencil Capabilities) */

    if (IS_NAPALM)
    {
        pCaps->dwStencilCaps =      0                                         |
                                    D3DSTENCILCAPS_DECR                       |
                                    D3DSTENCILCAPS_DECRSAT                    |
                                    D3DSTENCILCAPS_INCR                       |
                                    D3DSTENCILCAPS_INCRSAT                    |
                                    D3DSTENCILCAPS_INVERT                     |
                                    D3DSTENCILCAPS_KEEP                       |
                                    D3DSTENCILCAPS_REPLACE                    |
                                    D3DSTENCILCAPS_ZERO                       |
                                    0; 
    }
    else    /* Non-Napalm */
    {
        pCaps->dwStencilCaps =      0;
    }
    /* dwTextureOpCaps, wMaxTextureBlendStages (Texture Capabilities) */

    if (IS_NAPALM)
    {
        pCaps->dwTextureOpCaps =    0                                         |
                                    D3DTEXOPCAPS_DISABLE                      |
                                    D3DTEXOPCAPS_SELECTARG1                   |
                                    D3DTEXOPCAPS_SELECTARG2                   |
                                    D3DTEXOPCAPS_MODULATE                     |
                                    D3DTEXOPCAPS_MODULATE2X                   |
                                    D3DTEXOPCAPS_ADD                          |
                                    D3DTEXOPCAPS_SUBTRACT                     |
                                    D3DTEXOPCAPS_BLENDDIFFUSEALPHA            |
                                    D3DTEXOPCAPS_BLENDTEXTUREALPHA            |
                                    D3DTEXOPCAPS_BLENDFACTORALPHA             |
                                    D3DTEXOPCAPS_BLENDTEXTUREALPHAPM          |
                                    D3DTEXOPCAPS_BLENDCURRENTALPHA            |
//                                  D3DTEXOPCAPS_PREMODULATE                  |
                                    D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR       |
                                    D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA       |
                                    D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR    |
                                    D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA    |
//                                  D3DTEXOPCAPS_BUMPENVMAP                   |
//                                  D3DTEXOPCAPS_BUMPENVMAPLUMINANCE          |
//                                  D3DTEXOPCAPS_DOTPRODUCT3                  |
                                    D3DTEXOPCAPS_MODULATE4X                   |
                                    D3DTEXOPCAPS_ADDSIGNED                    |
                                    D3DTEXOPCAPS_ADDSIGNED2X                  |
                                    D3DTEXOPCAPS_ADDSMOOTH                    |
                                    0;

        pCaps->wMaxTextureBlendStages = NUMTEXTUREUNITS;
    }
    else    /* Non-Napalm */
    {
        pCaps->dwTextureOpCaps =    0                                         |
                                    D3DTEXOPCAPS_DISABLE                      |
                                    D3DTEXOPCAPS_SELECTARG1                   |
                                    D3DTEXOPCAPS_SELECTARG2                   |
                                    D3DTEXOPCAPS_MODULATE                     |
                                    D3DTEXOPCAPS_MODULATE2X                   |
                                    D3DTEXOPCAPS_ADD                          |
                                    D3DTEXOPCAPS_SUBTRACT                     |
                                    D3DTEXOPCAPS_BLENDDIFFUSEALPHA            |
                                    D3DTEXOPCAPS_BLENDTEXTUREALPHA            |
                                    D3DTEXOPCAPS_BLENDFACTORALPHA             |
                                    D3DTEXOPCAPS_BLENDTEXTUREALPHAPM          |
                                    D3DTEXOPCAPS_BLENDCURRENTALPHA            |
//                                  D3DTEXOPCAPS_PREMODULATE                  |
                                    D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR       |
                                    D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA       |
                                    D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR    |
                                    D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA    |
//                                  D3DTEXOPCAPS_BUMPENVMAP                   |
//                                  D3DTEXOPCAPS_BUMPENVMAPLUMINANCE          |
//                                  D3DTEXOPCAPS_DOTPRODUCT3                  |
                                    0;

        pCaps->wMaxTextureBlendStages = NUMTEXTUREUNITS + 1;
    }
    /* dwMaxSimultaneousTextures (Number of Simultaneous Textures [Multi-Texture]) */

    pCaps->wMaxSimultaneousTextures = NUMTEXTUREUNITS;

    /* dwFVFCaps (Flexible Vertex Format Capabilities) */

    pCaps->dwFVFCaps =              0                                         |
                                    NUMTEXTUREUNITS                           |
                                    D3DFVFCAPS_DONOTSTRIPELEMENTS             |
                                    0;

#endif

    /* dwMaxActiveLights, wMaxUserClipPlanes, wMaxVertexBlendMatrices, dwVertexProcessingCaps (TnL Capabilities) */

#if( DIRECTDRAW_VERSION >= 0x0700 )
#ifdef TnL_HAL
    pCaps->dwMaxActiveLights =      TLMAX_LIGHTS;
    pCaps->wMaxUserClipPlanes =     TLMAX_USER_CLIPPLANES;
    pCaps->wMaxVertexBlendMatrices= TLMAX_WORLD_MATRICES;

    pCaps->dwVertexProcessingCaps = 0                                         |
//                                  D3DVTXPCAPS_TEXGEN                        |
                                    D3DVTXPCAPS_MATERIALSOURCE7               |
                                    D3DVTXPCAPS_VERTEXFOG                     |
                                    D3DVTXPCAPS_DIRECTIONALLIGHTS             |
                                    D3DVTXPCAPS_POSITIONALLIGHTS              |
                                    D3DVTXPCAPS_LOCALVIEWER                   |
                                    0;
#endif
#endif

    /* Completed Building D3D Extended Capabilities Structure, Return to Caller */

    return;
}

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
void BuildD3DCaps8(NT9XDEVICEDATA *ppdev, D3DCAPS8 *pCaps)
{
    D3DDEVICEDESC_V1 D3DCaps;
    D3DHAL_D3DEXTENDEDCAPS D3DExtendedCaps;

    /* Build the D3D and D3D Extended Capabililties Structures */

    BuildD3DCaps(ppdev, &D3DCaps);
    BuildD3DExtendedCaps(ppdev, &D3DExtendedCaps);

    /* Initialize the D3D DX 8 Capabilities Structure (pCaps) */

    memset(pCaps, 0, sizeof(D3DCAPS8));

    /* DeviceType, AdapterOrdinal (Device Type and Adapter Number) [New] */

    pCaps->DeviceType =             D3DDEVTYPE_HAL;
    pCaps->AdapterOrdinal =         0;      // Should this be a real number?

    /* Caps, Caps2, Caps3 (DX 8 Style DirectDraw Capabilities) [New] */


// JT removed for now - need to fix later
//    pCaps->Caps =                   _FF(HALInfo).ddCaps.dwCaps                |
//                                  D3DCAPS_READ_SCANLINE                     |
                                    0;
// JT removed for now - need to fix later
//    pCaps->Caps2 =                  _FF(HALInfo).ddCaps.dwCaps2               |
//                                  D3DCAPS2_NO2DDURING3DSCENE                |
//                                  D3DCAPS2_FULLSCREENGAMMA                  |
                                    D3DCAPS2_CANRENDERWINDOWED                |
//                                  D3DCAPS2_CANCALIBRATEGAMMA                |
                                    0;

    pCaps->Caps3 =                  0;

    /* PresentationIntervals (Presentation Capabilities) [New] */

    pCaps->PresentationIntervals =  0                                         |
                                    D3DPRESENT_INTERVAL_DEFAULT               |
                                    D3DPRESENT_INTERVAL_ONE                   |
                                    D3DPRESENT_INTERVAL_TWO                   |
                                    D3DPRESENT_INTERVAL_THREE                 |
                                    D3DPRESENT_INTERVAL_FOUR                  |
                                    0;

    /* CursorCaps (Cursor Capabilities) [New] */

    pCaps->CursorCaps =             0                                         |
//                                  D3DCURSORCAPS_COLOR                       |
//                                  D3DCURSORCAPS_LORES                       |
                                    0;

    /* DevCaps (3D Device Capabilities) [Copy from DX 7 and Extend] */

    pCaps->DevCaps =                D3DCaps.dwDevCaps                         |
#if defined(TnL_HAL)
//                                  D3DDEVCAPS_PUREDEVICE                     |
#endif
//                                  D3DDEVCAPS_QUINTICRTPATCHES               |
//                                  D3DDEVCAPS_RTPATCHES                      |
//                                  D3DDEVCAPS_RTPATCHHANDLEZERO              |
//                                  D3DDEVCAPS_NPATCHES                       |
                                    0;

    /* PrimitiveMiscCaps (Primitive Capabilities) [Copy from DX 7 and Extend] */

    pCaps->PrimitiveMiscCaps =      D3DCaps.dpcTriCaps.dwMiscCaps             |
//                                  D3DPMISCCAPS_COLORWRITEENABLE             |
//                                  D3DPMISCCAPS_CLIPPLANESCALEDPOINTS        |
//                                  D3DPMISCCAPS_CLIPTLVERTS                  |
//                                  D3DPMISCCAPS_TSSARGTEMP                   |
//                                  D3DPMISCCAPS_BLENDOP                      |
                                    0;

    /* RasterCaps (Rasterization Capabilities) [Copy from DX 7 and Extend] */

    pCaps->RasterCaps =             D3DCaps.dpcTriCaps.dwRasterCaps           |
                                    D3DPRASTERCAPS_COLORPERSPECTIVE           |
//                                  D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE      |
                                    0;

    /* ZCmpCaps (Z Compare Capabilities) [Copy from DX 7 and Extend] */

    pCaps->ZCmpCaps =               D3DCaps.dpcTriCaps.dwZCmpCaps             |
                                    0;

    /* SrcBlendCaps (Source Blend Capabilities) [Copy from DX 7 and Extend] */

    pCaps->SrcBlendCaps =           D3DCaps.dpcTriCaps.dwSrcBlendCaps         |
                                    0;

    /* DestBlendCaps (Destination Blend Capabilities) [Copy from DX 7 and Extend] */

    pCaps->DestBlendCaps =          D3DCaps.dpcTriCaps.dwDestBlendCaps        |
                                    0;

    /* AlphaCmpCaps (Alpha Compare Capabilities) [Copy from DX 7 and Extend] */

    pCaps->AlphaCmpCaps =           D3DCaps.dpcTriCaps.dwAlphaCmpCaps         |
                                    0;

    /* ShadeCaps (Shading Capabilities) [Copy from DX 7 and Extend] */

    pCaps->ShadeCaps =              D3DCaps.dpcTriCaps.dwShadeCaps            |
                                    0;

    /* TextureCaps (Texturing Capabilities) [Copy from DX 7 and Extend] */

    pCaps->TextureCaps =            D3DCaps.dpcTriCaps.dwTextureCaps          |
//                                  D3DPTEXTURECAPS_NONPOW2CONDITIONAL        |
//                                  D3DPTEXTURECAPS_PROJECTED                 |
//                                  D3DPTEXTURECAPS_CUBEMAP                   |
//                                  D3DPTEXTURECAPS_VOLUMEMAP                 |
                                    D3DPTEXTURECAPS_MIPMAP                    |
//                                  D3DPTEXTURECAPS_MIPVOLUMEMAP              |
//                                  D3DPTEXTURECAPS_MIPCUBEMAP                |
//                                  D3DPTEXTURECAPS_CUBEMAP_POW2              |
//                                  D3DPTEXTURECAPS_VOLUMEMAP_POW2            |
                                    0;
    /* TextureFilterCaps (Texture Filtering Capabilities) [Copy from DX 7 and Extend] */

    pCaps->TextureFilterCaps =      D3DCaps.dpcTriCaps.dwTextureFilterCaps    |
                                    0;

    /* CubeTextureFilterCaps (Cubic Texture Filter Capabilities) [New] */

    pCaps->CubeTextureFilterCaps =  0                                         |
//                                  D3DPTFILTERCAPS_NEAREST                   |
//                                  D3DPTFILTERCAPS_LINEAR                    |
//                                  D3DPTFILTERCAPS_MIPNEAREST                |
//                                  D3DPTFILTERCAPS_MIPLINEAR                 |
//                                  D3DPTFILTERCAPS_LINEARMIPNEAREST          |
//                                  D3DPTFILTERCAPS_LINEARMIPLINEAR           |
//                                  D3DPTFILTERCAPS_MINFPOINT                 |
//                                  D3DPTFILTERCAPS_MINFLINEAR                |
//                                  D3DPTFILTERCAPS_MINFANISOTROPIC           |
//                                  D3DPTFILTERCAPS_MIPFPOINT                 |
//                                  D3DPTFILTERCAPS_MIPFLINEAR                |
//                                  D3DPTFILTERCAPS_MAGFPOINT                 |
//                                  D3DPTFILTERCAPS_MAGFLINEAR                |
//                                  D3DPTFILTERCAPS_MAGFANISOTROPIC           |
//                                  D3DPTFILTERCAPS_MAGFAFLATCUBIC            |
//                                  D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC         |
                                    0;

    /* VolumeTextureFilterCaps (Volume Texture Filter Capabilities) [New] */

    pCaps->VolumeTextureFilterCaps = 0                                        |
//                                  D3DPTFILTERCAPS_NEAREST                   |
//                                  D3DPTFILTERCAPS_LINEAR                    |
//                                  D3DPTFILTERCAPS_MIPNEAREST                |
//                                  D3DPTFILTERCAPS_MIPLINEAR                 |
//                                  D3DPTFILTERCAPS_LINEARMIPNEAREST          |
//                                  D3DPTFILTERCAPS_LINEARMIPLINEAR           |
//                                  D3DPTFILTERCAPS_MINFPOINT                 |
//                                  D3DPTFILTERCAPS_MINFLINEAR                |
//                                  D3DPTFILTERCAPS_MINFANISOTROPIC           |
//                                  D3DPTFILTERCAPS_MIPFPOINT                 |
//                                  D3DPTFILTERCAPS_MIPFLINEAR                |
//                                  D3DPTFILTERCAPS_MAGFPOINT                 |
//                                  D3DPTFILTERCAPS_MAGFLINEAR                |
//                                  D3DPTFILTERCAPS_MAGFANISOTROPIC           |
//                                  D3DPTFILTERCAPS_MAGFAFLATCUBIC            |
//                                  D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC         |
                                    0;

    /* TextureAddressCaps (Texture Addressing Capabilities) [Copy from DX 7 and Extend] */

    pCaps->TextureAddressCaps =     D3DCaps.dpcTriCaps.dwTextureAddressCaps   |
//                                  D3DPTADDRESSCAPS_MIRRORONCE               |
                                    0;

    /* VolumeTextureAddressCaps (Volume Texture Addressing Capabilities) [New] */

    pCaps->VolumeTextureAddressCaps = 0                                       |
//                                  D3DPTADDRESSCAPS_WRAP                     |
//                                  D3DPTADDRESSCAPS_MIRROR                   |
//                                  D3DPTADDRESSCAPS_CLAMP                    |
//                                  D3DPTADDRESSCAPS_BORDER                   |
//                                  D3DPTADDRESSCAPS_INDEPENDENTUV            |
//                                  D3DPTADDRESSCAPS_MIRRORONCE               |
                                    0;

    /* LineCaps (Line Capabilities) [New] */

    pCaps->LineCaps =               0                                         |
//                                  D3DLINECAPS_TEXTURE                       |
//                                  D3DLINECAPS_ZTEST                         |
//                                  D3DLINECAPS_BLEND                         |
//                                  D3DLINECAPS_ALPHACMP                      |
//                                  D3DLINECAPS_FOG                           |
                                    0;

    /* MaxTextureWidth, MaxTextureHeight (Maximum Texture Width and Height) [Copy DX 7 Values] */

    pCaps->MaxTextureWidth =        D3DExtendedCaps.dwMaxTextureWidth;
    pCaps->MaxTextureHeight =       D3DExtendedCaps.dwMaxTextureHeight;

    /* MaxVolumeExtent (Maximum Volume Texture Extent) [New] */

    pCaps->MaxVolumeExtent =        0;

    /* MaxTextureRepeat (Maximum Times a Texture may be Repeated [Tiled]) [Copy DX 7 Value] */

    pCaps->MaxTextureRepeat =       D3DExtendedCaps.dwMaxTextureRepeat;

    /* MaxTextureAspectRatio (Maximum Texture Aspect Ratio) [Copy DX 7 Value] */

    pCaps->MaxTextureAspectRatio =  D3DExtendedCaps.dwMaxTextureAspectRatio;

    /* MaxAnisotrophy (Maximum Anisotropy Value) [Copy DX 7 Value] */

    pCaps->MaxAnisotropy =          D3DExtendedCaps.dwMaxAnisotropy;

    /* MaxVertexW (Maximum Vertex W Buffering Value) [New] */

    pCaps->MaxVertexW =             0.f;

    /* GuardBandLeft, GuardBandTop, GuardBandRight, GuardBandBottom (Guard Band Limits) [Copy DX 7 Values] */

    pCaps->GuardBandLeft =          D3DExtendedCaps.dvGuardBandLeft;
    pCaps->GuardBandTop =           D3DExtendedCaps.dvGuardBandTop;
    pCaps->GuardBandRight =         D3DExtendedCaps.dvGuardBandRight;
    pCaps->GuardBandBottom =        D3DExtendedCaps.dvGuardBandBottom;

    /* ExtentsAdjust (Extents Adjustment Value) [Copy DX 7 Value] */

    pCaps->ExtentsAdjust =          D3DExtendedCaps.dvExtentsAdjust;

    /* StencilCaps (Stencil Capabilities) [Copy from DX 7 and Extend] */

    pCaps->StencilCaps =            D3DExtendedCaps.dwStencilCaps             |
                                    0;
    /* FVFCaps (Flexible Vertex Buffer Format Capabilities) [Copy from DX 7 and Extend] */

    pCaps->FVFCaps =                D3DExtendedCaps.dwFVFCaps                 |
//                                  D3DFVFCAPS_PSIZE                          |
                                    0;

    /* TextureOpCaps, MaxTextureBlendStages, MaxSimultaneousTextures (Texture Capabilities) [Copy/Extend from DX 7] */

    pCaps->TextureOpCaps =          D3DExtendedCaps.dwTextureOpCaps           |
//                                  D3DTEXOPCAPS_MULTIPLYADD                  |
//                                  D3DTEXOPCAPS_LERP                         |
                                    0;

    pCaps->MaxTextureBlendStages =  D3DExtendedCaps.wMaxTextureBlendStages;
    pCaps->MaxSimultaneousTextures= D3DExtendedCaps.wMaxSimultaneousTextures;

    /* MaxActiveLights, MaxUserClipPlanes, MaxVertexBlendMatrices, VertexProcessingCaps (TnL Capabilities) [Copy/Extend from DX 7] */

#ifdef TnL_HAL
    pCaps->MaxActiveLights =        D3DExtendedCaps.dwMaxActiveLights;
    pCaps->MaxUserClipPlanes =      D3DExtendedCaps.wMaxUserClipPlanes;
    pCaps->MaxVertexBlendMatrices = D3DExtendedCaps.wMaxVertexBlendMatrices;

    pCaps->VertexProcessingCaps |=  D3DExtendedCaps.dwVertexProcessingCaps    |
//                                  D3DVTXPCAPS_TWEENING                      |
//                                  D3DVTXPCAPS_NO_VSDT_UBYTE4                |
                                    0;
#endif

    /* MaxPointSize (Maximum Point Sprite Size) [New] */

    pCaps->MaxPointSize =           1.f;  // Required for DX 8 support

    /* MaxPrimitiveCount (Maximum Primitive Count) [New] */

    pCaps->MaxPrimitiveCount =      0xffff;

    /* MaxVertexIndex (Maximum Vertex Buffer Index) [New] */

    pCaps->MaxVertexIndex =         0xffff;

    /* MaxStreams, MaxStreamStride (Streams Capabilities) [New] */

    pCaps->MaxStreams =             1;    // Indicates a DX 8 driver
    pCaps->MaxStreamStride =        256;

    /* VertexShaderVersion, MaxVertexShaderConst (Vertex Shader Capabilities) [New] */

    pCaps->VertexShaderVersion =    D3DVS_VERSION(0, 0);
    pCaps->MaxVertexShaderConst =   0;

    /* PixelShaderVersion, MaxPixelValue (Pixel Shader Capabilities) [New] */

    pCaps->PixelShaderVersion =     D3DPS_VERSION(0, 0);
    pCaps->MaxPixelShaderValue =    0;

    /* Completed Building D3D DX 8 Capabilities Structure, Return to Caller */

    return;
}
#endif //(DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
