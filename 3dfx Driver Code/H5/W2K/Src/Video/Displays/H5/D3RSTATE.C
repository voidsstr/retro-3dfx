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
** File name: d3rstate.c
**
** Description: Define state change functions and jump table.
**
** $Revision: 38$
** $Date: 10/31/00 2:00:59 AM$
**
** $Log:
**  3    3dfx      1.2         09/20/99 Steve Rogers    Porting my fix from V3 OEM
**       tree -   Alpha Dither Subtraction is only allowed when dithering is
**       enabled.  Also 2x2 dithering is always enabled.  This fixes PRS 6378.
**  2    3dfx      1.1         09/16/99 Bob Seitsinger  Dither matrix rotate
**       changes. InitNewRC - create a shifted DWORD for dither matrix rotation
**       selection bits that are OR'd into fogMode register -
**       fogModeDitherMatrixSelect (one for each chip).
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator
** $
**
** 23    9/07/99 12:03p Bseitsin
** Properly initialize sst.chipMask in initNewRC.
**
** 22    9/07/99 10:49a Bseitsin
** Stencil buffer clear fix.
**
** 21    9/02/99 11:14a Cwilcox
** Added 24bpp zbuffer support to Direct3D.
**
** 20    8/25/99 1:53p Russ
** fix for DX7 z buffer problems, disable check for NULL pRc->lpDDSZ in
** zEnable and zWriteEnable, delay check until setDX6State
** for W2K & DX7, fix for potential access violation issues with NT Stress
** verify we have a valid TXTRHNDL for surfaces before dereferencing the
** TXTRHNDL_PTR
**
** 19    8/10/99 1:24p Cshaw
** Added regisrty support for disabling guardband and 2ppc.
**
** 18    8/10/99 10:38a Russ
** for DX7, in textureHandle, remove translation from MSHandle to
** TDFXHandle
**
** 17    8/03/99 1:39p Msmith
** Adding DCT perspective fixes from Bob J.
**
** 16    8/02/99 12:31p Russ
** for W2K, restore ASSERTDD that was disabled last week
**
** 15    7/29/99 5:17p Bseitsin
** Remove SrcAlphaSaturate as a default capability. Include it at run time
** if IS_NAPALM.
**
** 14    7/28/99 11:59p Bseitsin
** Changes to enable DX7 for W9x.
**
** 13    7/27/99 8:08p Bseitsin
** #ifdef SLI_AA some new code in initNewRC.
**
** 12    7/27/99 4:27p Bseitsin
** Antialiasing changes.
**
** 11    7/23/99 11:46a Russ
** for DX7, wrap _renderFuncs[153-158] in #if 0/#endif
**
** 10    7/22/99 6:01p Bseitsin
** Support new alpha blend modes src*src, dst*dst, [src/dst]*dstalpha.
**
** 9     7/19/99 3:06p Russ
** changes for V3/Napalm runtime check
**
** 8     7/13/99 3:38p Cshaw
** Added guardband for Napalm.
**
** 7     7/12/99 1:11p Russ
** add eight new entries to _renderFuncs for DX7
**
** 6     7/02/99 12:17p Russ
** add support for DX7's scene capture render state
** in DX7, this replaces the ddiSceneCapture callback
**
** 5     6/21/99 5:24p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 4     6/16/99 3:45p Cshaw
** Added more texop support for texturefactors on color1, and alpha
** replicate.
**
** 3     6/11/99 12:48p Russ
** initial changes for DX7
** fix for access violations in zFunc & zBias when there is no ZBuffer
** surface
**
** 2     6/04/99 7:14p Bseitsin
** 24bit Z/8bit stencil mods.
**
** 1     6/02/99 6:43a Michael
** Branch from H3
**
** 98    5/30/99 5:48p Edwin
** Remove ifdef MM, multi-monitor support is always enabled.
**
** 97    5/24/99 5:05p Bseitsin
** Removal of Antialiasing code.
**
** 96    5/14/99 4:52p Stb_gbullard
** Updated with contractor (NVH, Intelligraphics) fixes
**
** 95    5/12/99 5:13p Stb_gbullard
** Updated with contractor (NVH, Intelligraphics) fixes
**
** 94    5/10/99 2:03p Stb_srogers
** Adding Video Filter and Alpha Dither Subtraction quality updates. Fixes
** PRS 5442
**
** 93    4/23/99 2:06p Stb_bseitsin
** 32-bit rendering mods
**
** 92    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
**
** 91    3/26/99 4:12p Cwilcox
** Major modification to memory allocation and management, fix PRS #4229.
**
** 90    2/18/99 8:24a Russ
** modify D3DPRINT debugPrintLevel setting to use RSTATE_DBG_LVL instead
** of a hardcoded 24 (RSTATE_DBG_LVL is defined as 24 for win9x and 1 for
** NT)
**
** 89    2/11/99 8:30a Stb_skephart
**
** 88    2/03/99 9:43p Russ
** for NT5, added return value from functions in  _renderFuncs table.
** Added error checking in some of the functions.  This is needed in order
** to pass a new DCT test that throws garbage data at DP2
**
** 87    1/29/99 10:49a Cshaw
** Added unified headers.
**
** 86    12/09/98 7:27a Russ
** NT5 D3D changes for Banshee
**
** 85    12/06/98 1:53p Adrians
** Optimisations for WinBench99.
**
** 84    12/04/98 2:27p Martin
** Fix build
**
** 83    12/04/98 9:35a Martin
** new macro to check if a texture is in use.  Fixes bug where they use
** handles for textures that have been deleted.
**
** 82    12/03/98 9:30p Adrians
** Fix for earlier checkin that broke the build.  Undefined macro.
**
** 81    12/03/98 6:02p Martin
** Add 3D Studio Max registry variable, and setup texture formats if
** turned on.
**
** 80    12/02/98 8:24a Martin
** Turn off anti-aliasing by default and rename some variables for
** super-sampling AA.
**
** 79    11/30/98 9:18p Adrians
** Fix for alphablending problem with Heavy Gear II (PRS#3284).
**
** 78    11/22/98 9:02p Andrew
** Changes to support multi-monitor
**
** 77    11/18/98 6:09p Adrians
** Changes for Avenger.
**
** 76    11/08/98 4:20p Hanson
** Fixes and Optimizations for Winbench 99
**
** 75    11/06/98 2:43p Adrians
** Fix for zBias in WinBench99
**
** 74    11/06/98 12:19p Adrians
** Fix for wrapping problem which occurs with  WinBench99.
**
** 73    10/29/98 2:12p Martin
** Modification of cmdFifo macros to support future changes for
** super-sampling AA.
**
** 72    10/16/98 4:33p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
**
** 71    10/13/98 5:53p Hanson
** Dx6 optimizations for DrawIndexedPrimitive
**
** 70    10/08/98 1:47a Hanson
** K6 3D cleanup
**
** 69    10/07/98 10:39p Adrians
** Added K6-2 optimisations.
**
** 67    10/07/98 9:05a stit (Metabyte, Inc)
** AMD K6-2 (MMX+3DNOW) optimization
**
** 66    10/06/98 6:25p Hanson
** Properly punted on the Edge AA case if depth buffering wasn't being
** used.
**
** 65    10/06/98 2:03p Miriam
** Automipmap support
**
** 64    10/04/98 5:39p Hanson
** Added check and flags setting for non P6's
**
** 63    10/02/98 11:45a Adrians
** Added support for D3DTBLEND_ADD to DX5.
**
** 62    10/01/98 10:38p Adrians
** Re-coded Fog which fixes a bug when Wbuffering with Vertex Fog.
**
** 61    10/01/98 5:58p Hanson
** Added Dx6 ifdef around SetDrawTriangle6
**
** 60    10/01/98 5:17p Hanson
** Fix for added flags to the state structure in dealing with the triangle
** flavors
**
** 59    10/01/98 5:13p Hanson
** Dx6 fixes with SetDrawTriangle
**
** 58    10/01/98 5:05p Hanson
** Dx6 changes mostly affecting triangle flavoring and state flags.
**
** 57    9/26/98 5:32p Hanson
** Dx6 Assembly interface Optimizations
**
** 56    9/25/98 6:44p Adrians
** Addtional instrumentation changes.
**
** 55    9/24/98 11:51a Hanson
** Dx6 Optimizations
**
** 54    9/21/98 3:11p Adrians
** Optimisation to Fog HW setup.
** Added ZBIAS state change.
** Fix for multiple context's with different fog color.
** Set lodMin & lodMax to 1x1 when flat shading.
**
** 53    9/17/98 7:09p Adrians
** Fix for texMag renderstate.
**
** 52    9/14/98 12:13a Adrians
** Remove TextureFactor support for the DX5 driver.
**
** 50    9/13/98 12:18p Adrians
** Added a new flag global variable to disable AA when using specular,
** alphablending or chromokeying.
** Iterated Alpha is now always enabled for DX6.
**
** 49    9/12/98 12:56a Adrians
** Clean up renderstate trace code and add new DX6 states.
** Add trace support for DX6 texture stage states.
** General DX6 code tidyup.
**
** 48    9/10/98 8:20p Adrians
** Fix for textures that don't have alpha with legacy apps.
**
** 47    9/02/98 12:33p Adrians
** DX6 multitexture change.
**
** 46    9/01/98 5:33p Martin
** Turn on super-sampling AA by default.  Registry variable can override
** default, to either turn it off, or force edge-AA on instead of
** super-sampling.
**
** 45    8/28/98 3:20p Martin
** Reload palette when texture handle changes. Old fix caused performance
** degradation. New fix should only download palette at texture handle
** change.
**
** 44    8/28/98 3:04p Martin
** Fix typo
**
** 43    8/28/98 2:33p Martin
** Conditionalize banshee compilation.
**
** 42    8/28/98 12:37p Martin
** Do super-sampling AA when the registry key is set to 2.  Do edge AA if
** registry key == 1 OR we are not running at the 2 magical resolutions of
** 640x480 or 800x600.
**
** No more conditional compilation of AA.
**
** 41    8/25/98 4:53p Martin
** add some more checks before doing antialiasing
**
** 40    8/25/98 2:08p Martin
** Copy 2 triangle flavors (IZ & IZT), and make them AA specific for super
** sampling.
**
** 39    8/24/98 5:23p Adrians
** Fix for SGRAM SuperSampling.
**
** 38    8/23/98 12:49a Adrians
** Added support for AA SuperSampling.
**
** 37    8/14/98 8:09p Adrians
** Add texture wrapping support.
** Fix for WBuffering.
**
** 4     8/12/98 11:52a Adrians
** Initialise wscale variables.
**
** 2     8/07/98 3:56p Adrians
** Re-Enable Zbias
**
** 1     8/07/98 3:51p Adrians
**
** 36    8/04/98 10:14a Adrians
** Remove redunant code.
**
** 35    7/24/98 1:37p Hohn
**
** 34    7/23/98 8:15p Hanson
** Added AA into main tree
**
** 33    7/22/98 12:31p Adrians
** Automipmap code.
**
** 32    7/01/98 3:49p Miriam
** Clip registers are now set for drawing surfaces.
**
** 31    6/30/98 5:29p Miriam
** Performance optimizations.
**
** 30    5/26/98 7:24p Adrians
** Added antialiasing.
** Added renderstate debug level.
**
** 29    5/14/98 6:05p Miriam
** Merge in H3 support. Little cleanup.
**
** 28    5/06/98 6:09p Adrians
** Changes for DX6 into DX5 driver.
**
** 2     5/01/98 4:11p Adrians
** Compile options for dx5 and dx6.
** Removed redundent returns.
**
** 1     4/29/98 6:31p Adrians
** Created
**
** 27    4/03/98 6:44p Adrians
** Fix for Rockem3D, removed state optimisation.
**
** 26    3/27/98 4:04p Adrians
** Triangle flavour CMDFIFO_CHECKROOM optimisations.
**
** 25    3/27/98 11:18a Adrians
** New DrawPrimitive code.
**
** 24    3/16/98 1:46p Adrians
** Added dynamic pixel center offset control.
**
** 23    2/16/98 11:52p Adrians
** No longer clear Zbuffer if it does not exist.
**
** 22    1/28/98 4:22p Adrians
** Added 0.5 to every vertex x,y and half a texel to s,t.
 *
 * 21    1/16/98 10:42a Adrians
 * Added option to force Point Sample texturing to BiLinear.
 * Environment variable is SSTV2_FORCE_BILINEAR.
 *
 * 20    1/15/98 10:23a Adrians
 * Added ZBIAS support.
 *
 * 19    1/14/98 11:25a Adrians
 * Added code to enable FORCE_TRILINEAR.
 *
 * 18    11/23/97 3:56p Suninn
 * replay _d3Global with _D3 & D3G macros
 *
 * 17    11/18/97 4:36p Adrians
 * Nows compiles with new DDK R3.
 *
 * 16    11/11/97 9:26p Adrians
 * Added 3 more triangle flavours (I, IT, C).
 *
 * 15    11/09/97 2:51p Adrians
 * Single pk1's use an increment of 0.
 * Code added to find the triangle flavours used by apps.
 * Optimisation of triangle flavours.
 * Support for strips and fans in execute buffers.
 * Bug fix to wrapU and wrapV modes.
 *
 *
 * 14    10/31/97 11:46a Miriam
 * Clean up some debugging junk.
 *
 * 13    10/27/97 2:32p Adrians
 * Fixed 8 bit texture download.
 * 8 bit palettes are now built as default (p8=1).
 * Incorporated software triangle setup. Added build option (default
 * ss=0).
 * Command fifo debug build option added (default fd=0).
 *
 * 12    10/24/97 7:14p Miriam
 * Support for 2 tmu and trilinear. Will see 2x the memory on a 2 tmu
 * system. Trilinear is 1 pass on 2 tmu & lodDither for 1 tmu.
 * Fixed command fifo problem when using save/restore.
 *
 * 11    10/14/97 11:22a Adrians
 * Added Miriam's 4M texture support.  Added build environment for H3.
 *
 * 10    10/10/97 10:42a Adrians
 * Removed all references to DIRECTX5.
 *
 * 9     10/09/97 10:22a Adrians
 * Removed some of the state changed checks.
 *
 * 8     10/07/97 1:39p Adrians
 * Optimisation to myRenderPrimitive setup.
 * New Caching enable call changes.
 *
 * 6     10/02/97 8:38p Adrians
 * Include init code into build. Enable Write Combining.  Inline system
 * functions.  Change optimisations.  Some code tidy up.
 *
 * 5     9/16/97 5:13p Adrians
 * Flat shading now uses RGB iterator rather than c0/c1.
 *
 * 4     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/

#include "precomp.h"

#ifndef WINNT
#include <d3dhal.h>
#include "hw.h"
#include "d3global.h"
#include "ddglobal.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "d3tri.h"

#if ( DX >= 6 )
#include "d6global.h"
#endif
#endif

#include "dxins.h"

#ifdef K6_2
#include "k6_2.h"
#endif
//--------------------------------------------------------------------
//
// Renderstate processing:
//
// Each renderstate is stored into the rendering context. In addition, the
// renderstate is mapped to the SST-1 registers. We have a shadow of each
// SST-1 register and can just set the register when it is time to render
// a primitive.
//
// Currently, the renderstate processing does not set the register here
// because there is no logic to "context switch" when we change contexts
// or when directdraw changes the state of the hardware. The rendering
// routines are responsible for setting the state each execute buffer.
//
//--------------------------------------------------------------------
RENDERFXN_RETVAL __stdcall textureHandle(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall texturePerspective(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall wrapU(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall wrapV(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall zEnable(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall shadeMode(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall zWriteEnable(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall alphaTestEnable(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall texMag(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall texMin(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall srcBlend(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall dstBlend(RC *pRc, ULONG state) ;
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(REMOVE_OBSOLETE1_FOR_DX7_WIN9X) && !defined(WINNT)
#else
RENDERFXN_RETVAL __stdcall texMapBlend(RC *pRc, ULONG state) ;
#endif
RENDERFXN_RETVAL __stdcall cullMode(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall zFunc(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall alphaRef(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall alphaFunc(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall ditherEnable(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall blendEnable(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall fogEnable(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall fogColor(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall fogTableMode(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall fogTableStart(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall fogTableEnd(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall fogDensity(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall setupColorPath(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall subPixel(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall specular(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall textureAddress(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall zVisible(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall fillMode(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall colorKeyEnable(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall alphaBlendEnable(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall textureAddressU(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall textureAddressV(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall zbias(RC *pRc, ULONG state) ;

RENDERFXN_RETVAL __stdcall edgeAntialiasing( RC *pRc, ULONG state );
RENDERFXN_RETVAL __stdcall antialiasing( RC *pRc, ULONG state );

#if ( DX >= 6 )
RENDERFXN_RETVAL __stdcall textureFactor( RC *pRc, ULONG state );
RENDERFXN_RETVAL __stdcall wrap0(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall wrap1(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall wrap2(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall wrap3(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall wrap4(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall wrap5(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall wrap6(RC *pRc, ULONG state) ;
RENDERFXN_RETVAL __stdcall wrap7(RC *pRc, ULONG state) ;
#endif

RENDERFXN_RETVAL __stdcall stencilEnable( RC *pRc, ULONG state );
RENDERFXN_RETVAL __stdcall stencilFail( RC *pRc, ULONG state );
RENDERFXN_RETVAL __stdcall stencilZFail( RC *pRc, ULONG state );
RENDERFXN_RETVAL __stdcall stencilPass( RC *pRc, ULONG state );
RENDERFXN_RETVAL __stdcall stencilFunc( RC *pRc, ULONG state );
RENDERFXN_RETVAL __stdcall stencilRef( RC *pRc, ULONG state );
RENDERFXN_RETVAL __stdcall stencilMask( RC *pRc, ULONG state );
RENDERFXN_RETVAL __stdcall stencilWriteMask( RC *pRc, ULONG state );

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
RENDERFXN_RETVAL __stdcall sceneCapture(RC *pRc, ULONG state);
#endif

//-----------------------------------------------------------------------
// jump table for state changes. if there is a dummy in the table then we
// don't support it yet.
//-----------------------------------------------------------------------
RENDERFXN _renderFuncs[] =
{
  dummy,                // First entry is not used                      =   0
  textureHandle,        // D3DRENDERSTATE_TEXTURE_HANDLE                =   1
  antialiasing,         // D3DRENDERSTATE_ANTIALIAS                     =   2
  textureAddress,       // D3DTEXTUREADDRESS                            =   3
  texturePerspective,   // D3DRENDERSTATE_TEXTURE_PERSPECTIVE           =   4
  wrapU,                // D3DRENDERSTATE_WRAP_U                        =   5
  wrapV,                // D3DRENDERSTATE_WRAP_V                        =   6
  zEnable,              // D3DRENDERSTATE_Z_ENABLE                      =   7
  fillMode,             // D3DRENDERSTATE_FILL_MODE                     =   8
  shadeMode,            // D3DRENDERSTATE_SHADE_MODE                    =   9
  dummy,                // D3DRENDERSTATE_LINE_PATTERN                  =  10
  dummy,                // D3DRENDERSTATE_MONOENABLE (just ignore)      =  11
  dummy,                // D3DRENDERSTATE_ROP2                          =  12
  dummy,                // D3DRENDERSTATE_PLANE_MASK                    =  13
  zWriteEnable,         // D3DRENDERSTATE_Z_WRITE_ENABLE                =  14
  alphaTestEnable,      // D3DRENDERSTATE_ALPHA_TEST_ENABLE             =  15
  dummy,                // D3DRENDERSTATE_LAST_PIXEL                    =  16
  texMag,               // D3DRENDERSTATE_TEX_MAG                       =  17
  texMin,               // D3DRENDERSTATE_TEX_MIN                       =  18
  srcBlend,             // D3DRENDERSTATE_SRC_BLEND                     =  19
  dstBlend,             // D3DRENDERSTATE_DST_BLEND                     =  20
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(REMOVE_OBSOLETE1_FOR_DX7_WIN9X) && !defined(WINNT)
  dummy,                //                                              =  21
#else
  texMapBlend,          // D3DRENDERSTATE_TEX_MAP_BLEND ???             =  21
#endif
  cullMode,             // D3DRENDERSTATE_CULL_MODE                     =  22
  zFunc,                // D3DRENDERSTATE_Z_FUNC                        =  23
  alphaRef,             // D3DRENDERSTATE_ALPHA_REF                     =  24
  alphaFunc,            // D3DRENDERSTATE_ALPHA_FUNC                    =  25
  ditherEnable,         // D3DRENDERSTATE_DITHER_ENABLE                 =  26
  blendEnable,          // D3DRENDERSTATE_BLEND_ENABLE                  =  27
  fogEnable,            // D3DRENDERSTATE_FOG_ENABLE                    =  28
#ifdef H3_A1
  dummy,                // D3DRENDERSTATE_SPECULARENABLE                =  29
#else
  specular,             // D3DRENDERSTATE_SPECULARENABLE                =  29
#endif
  zVisible,             // D3DRENDERSTATE_ZVISIBLE                      =  30
#if (DIRECT3D_VERSION >= 0x0300) && (DX >= 3)
  dummy,                // D3DRENDERSTATE_SUBPIXEL                      =  31  /* SUBPIXEL is permanently on because Direct X 3.0 sdk samples turn off */
#else
  subPixel,             // D3DRENDERSTATE_SUBPIXEL                      =  31
#endif
  dummy,                // D3DRENDERSTATE_SUBPIXELX                     =  32
  dummy,                // D3DRENDERSTATE_STIPPLEDALPHA                 =  33
  fogColor,             // D3DRENDERSTATE_FOGCOLOR                      =  34
  fogTableMode,         // D3DRENDERSTATE_FOGTABLEMODE                  =  35
  fogTableStart,        // D3DRENDERSTATE_FOGTABLESTART                 =  36
  fogTableEnd,          // D3DRENDERSTATE_FOGTABLEEND                   =  37
  fogDensity,           // D3DRENDERSTATE_FOGTABLEDENSITY               =  38
  dummy,                // D3DRENDERSTATE_STIPPLEENABLE                 =  39
  edgeAntialiasing,     // D3DRENDERSTATE_EDGEANTIALIAS                 =  40
  colorKeyEnable,       // D3DRENDERSTATE_COLORKEYENABLE                =  41
  alphaBlendEnable,     // D3DRENDERSTATE_ALPHABLENDENABLE              =  42
  dummy,                // D3DRENDERSTATE_BORDERCOLOR                   =  43
  textureAddressU,      // D3DRENDERSTATE_TEXTUREADDRESSU               =  44
  textureAddressV,      // D3DRENDERSTATE_TEXTUREADDRESSV               =  45
  dummy,                // D3DRENDERSTATE_MIPMAPLODBIAS                 =  46
  zbias,                // D3DRENDERSTATE_ZBIAS                         =  47
#ifdef TnL_HAL
  StateFogRangeEnable,  // D3DRENDERSTATE_RANGEFOGENABLE                =  48  /* Enables range-based fog */
#else
  dummy,                // D3DRENDERSTATE_RANGEFOGENABLE                =  48  /* Enables range-based fog */
#endif
  dummy,                // D3DRENDERSTATE_ANISOTROPY                    =  49  /* Max. anisotropy. 1 = no anisotropy */
  dummy,                // D3DRENDERSTATE_FLUSHBATCH                    =  50  /* Explicit flush for DP batching (DX5 Only) */
  dummy,                // D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT    =  51  /* BOOL enable sort-independent transparency */
  stencilEnable,        // D3DRENDERSTATE_STENCILENABLE                 =  52  /* BOOL enable/disable stenciling */
  stencilFail,          // D3DRENDERSTATE_STENCILFAIL                   =  53  /* D3DSTENCILOP to do if stencil test fails */
  stencilZFail,         // D3DRENDERSTATE_STENCILZFAIL                  =  54  /* D3DSTENCILOP to do if stencil test passes and Z test fails */
  stencilPass,          // D3DRENDERSTATE_STENCILPASS                   =  55  /* D3DSTENCILOP to do if both stencil and Z tests pass */
  stencilFunc,          // D3DRENDERSTATE_STENCILFUNC                   =  56  /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
  stencilRef,           // D3DRENDERSTATE_STENCILREF                    =  57  /* Reference value used in stencil test */
  stencilMask,          // D3DRENDERSTATE_STENCILMASK                   =  58  /* Mask value used in stencil test */
  stencilWriteMask,     // D3DRENDERSTATE_STENCILWRITEMASK              =  59  /* Write mask applied to values written to stencil buffer */
#if ( DX >= 6 )
  textureFactor,        // D3DRENDERSTATE_TEXTUREFACTOR                 =  60  /* D3DCOLOR used for multi-texture blend */
#else
  dummy,                // D3DRENDERSTATE_TEXTUREFACTOR                 =  60  /* D3DCOLOR used for multi-texture blend */
#endif
  dummy,                // Not Used                                     =  61
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  sceneCapture,         // D3DRENDERSTATE_SCENECAPTURE                  =  62
#else
  dummy,                // Not Used                                     =  62
#endif
  dummy,                // Not Used                                     =  63
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN00              =  64  /* Stipple pattern 01...  */
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN01              =  65 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN02              =  66 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN03              =  67 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN04              =  68 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN05              =  69 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN06              =  70 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN07              =  71 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN08              =  72 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN09              =  73 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN10              =  74 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN11              =  75 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN12              =  76 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN13              =  77 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN14              =  78 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN15              =  79 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN16              =  80 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN17              =  81 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN18              =  82 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN19              =  83 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN20              =  84 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN21              =  85 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN22              =  86 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN23              =  87 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN24              =  88 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN25              =  89 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN26              =  90 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN27              =  91 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN28              =  92 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN29              =  93 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN30              =  94 
  dummy,                // D3DRENDERSTATE_STIPPLEPATTERN31              =  95 

  dummy,                // Not used                                     =  96
  dummy,                // Not used                                     =  97
  dummy,                // Not used                                     =  98
  dummy,                // Not used                                     =  99
  dummy,                // Not used                                     = 100
  dummy,                // Not used                                     = 101
  dummy,                // Not used                                     = 102
  dummy,                // Not used                                     = 103
  dummy,                // Not used                                     = 104
  dummy,                // Not used                                     = 105
  dummy,                // Not used                                     = 106
  dummy,                // Not used                                     = 107
  dummy,                // Not used                                     = 108
  dummy,                // Not used                                     = 109
  dummy,                // Not used                                     = 110
  dummy,                // Not used                                     = 111
  dummy,                // Not used                                     = 112
  dummy,                // Not used                                     = 113
  dummy,                // Not used                                     = 114
  dummy,                // Not used                                     = 115
  dummy,                // Not used                                     = 116
  dummy,                // Not used                                     = 117
  dummy,                // Not used                                     = 118
  dummy,                // Not used                                     = 119
  dummy,                // Not used                                     = 120
  dummy,                // Not used                                     = 121
  dummy,                // Not used                                     = 122
  dummy,                // Not used                                     = 123
  dummy,                // Not used                                     = 124
  dummy,                // Not used                                     = 125
  dummy,                // Not used                                     = 126
  dummy,                // Not used                                     = 127

#if ( DX >= 6 )
  wrap0,                // D3DRENDERSTATE_WRAP0                         = 128  /* wrap for 1st texture coord. set */
  wrap1,                // D3DRENDERSTATE_WRAP1                         = 129  /* wrap for 2nd texture coord. set */
  wrap2,                // D3DRENDERSTATE_WRAP2                         = 130  /* wrap for 3rd texture coord. set */
  wrap3,                // D3DRENDERSTATE_WRAP3                         = 131  /* wrap for 4th texture coord. set */
  wrap4,                // D3DRENDERSTATE_WRAP4                         = 132  /* wrap for 5th texture coord. set */
  wrap5,                // D3DRENDERSTATE_WRAP5                         = 133  /* wrap for 6th texture coord. set */
  wrap6,                // D3DRENDERSTATE_WRAP6                         = 134  /* wrap for 7th texture coord. set */
  wrap7,                // D3DRENDERSTATE_WRAP7                         = 135  /* wrap for 8th texture coord. set */
#else
  dummy,                // D3DRENDERSTATE_WRAP0                         = 128  /* wrap for 1st texture coord. set */
  dummy,                // D3DRENDERSTATE_WRAP1                         = 129  /* wrap for 2nd texture coord. set */
  dummy,                // D3DRENDERSTATE_WRAP2                         = 130  /* wrap for 3rd texture coord. set */
  dummy,                // D3DRENDERSTATE_WRAP3                         = 131  /* wrap for 4th texture coord. set */
  dummy,                // D3DRENDERSTATE_WRAP4                         = 132  /* wrap for 5th texture coord. set */
  dummy,                // D3DRENDERSTATE_WRAP5                         = 133  /* wrap for 6th texture coord. set */
  dummy,                // D3DRENDERSTATE_WRAP6                         = 134  /* wrap for 7th texture coord. set */
  dummy,                // D3DRENDERSTATE_WRAP7                         = 135  /* wrap for 8th texture coord. set */
#endif

  // IGX-NVH Added 04.26.99
  #if(DIRECT3D_VERSION >= 0x0700)
#ifdef TnL_HAL
  StateClipping,        // D3DRENDERSTATE_CLIPPING                      = 136 
  StateLighting,        // D3DRENDERSTATE_LIGHTING                      = 137 
  dummy,                // D3DRENDERSTATE_EXTENTS                       = 138 
  StateAmbient,         // D3DRENDERSTATE_AMBIENT                       = 139 
  StateFogVertexMode,   // D3DRENDERSTATE_FOGVERTEXMODE                 = 140 
  StateColorVertex,     // D3DRENDERSTATE_COLORVERTEX                   = 141 
  StateLocalViewer,     // D3DRENDERSTATE_LOCALVIEWER                   = 142 
  StateNormNormals,     // D3DRENDERSTATE_NORMALIZENORMALS              = 143 
  dummy,                // D3DRENDERSTATE_COLORKEYBLENDENABLE           = 144 
  StateDfusMaterialSrc, // D3DRENDERSTATE_DIFFUSEMATERIALSOURCE         = 145 
  StateSpecMaterialSrc, // D3DRENDERSTATE_SPECULARMATERIALSOURCE        = 146 
  StateAmbMaterialSrc,  // D3DRENDERSTATE_AMBIENTMATERIALSOURCE         = 147 
  StateEmisMaterialSrc, // D3DRENDERSTATE_EMISSIVEMATERIALSOURCE        = 148 
  dummy,                // D3DRENDERSTATE_ALPHASOURCE                   = 149 
  dummy,                // D3DRENDERSTATE_FOGFACTORSOURCE               = 150 
  StateVertexBlends,    // D3DRENDERSTATE_VERTEXBLEND                   = 151 
  StateClipPlaneEnable, // D3DRENDERSTATE_CLIPPLANEENABLE               = 152 
#else
  dummy,                // D3DRENDERSTATE_CLIPPING                      = 136 
  dummy,                // D3DRENDERSTATE_LIGHTING                      = 137 
  dummy,                // D3DRENDERSTATE_EXTENTS                       = 138 
  dummy,                // D3DRENDERSTATE_AMBIENT                       = 139 
  dummy,                // D3DRENDERSTATE_FOGVERTEXMODE                 = 140 
  dummy,                // D3DRENDERSTATE_COLORVERTEX                   = 141 
  dummy,                // D3DRENDERSTATE_LOCALVIEWER                   = 142 
  dummy,                // D3DRENDERSTATE_NORMALIZENORMALS              = 143 
  dummy,                // D3DRENDERSTATE_COLORKEYBLENDENABLE           = 144 
  dummy,                // D3DRENDERSTATE_DIFFUSEMATERIALSOURCE         = 145 
  dummy,                // D3DRENDERSTATE_SPECULARMATERIALSOURCE        = 146 
  dummy,                // D3DRENDERSTATE_AMBIENTMATERIALSOURCE         = 147 
  dummy,                // D3DRENDERSTATE_EMISSIVEMATERIALSOURCE        = 148 
  dummy,                // D3DRENDERSTATE_ALPHASOURCE                   = 149 
  dummy,                // D3DRENDERSTATE_FOGFACTORSOURCE               = 150 
  dummy,                // D3DRENDERSTATE_VERTEXBLEND                   = 151 
  dummy,                // D3DRENDERSTATE_CLIPPLANEENABLE               = 152 
#endif
#if 0 // these renderstates disappeared in the 2082 DDK
  dummy,                // D3DRENDERSTATE_POINTSIZE                     = 153  /* D3DVALUE Point size */
  dummy,                // D3DRENDERSTATE_POINTATTENUATION_A            = 154  /* D3DVALUE Point attenuation a value */
  dummy,                // D3DRENDERSTATE_POINTATTENUATION_B            = 155  /* D3DVALUE Point attenuation b value */
  dummy,                // D3DRENDERSTATE_POINTATTENUATION_C            = 156  /* D3DVALUE Point attenuation c value */
  dummy,                // D3DRENDERSTATE_POINTSIZEMIN                  = 157  /* D3DVALUE Point size minimum threshold */
  dummy,                // D3DRENDERSTATE_POINTSPRITE_ENABLE            = 158  /* BOOL if true, render whole texture for point, */
#endif
  #endif /* DIRECT3D_VERSION >= 0x0700 */

  // In D3GLOBAL.H there is a define called MAX_NUM_RSTATES.
  // It keeps us from accessing elements in this array that do not exist.
  // If you add elements, please update MAX_NUM_RSTATES.

  // End NVH

};

/*-------------------------------------------------------------------
Function Name:  dummy
Description:    This attribute is not supported and the application didn't look at the
                capability flags to see that we can't do this. D3D does not support
                punting back for rasterization. So we have to ignore this attribute.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall dummy(RC *pRc, ULONG state)
{
  D3DPRINT( 255, "Warning: This state is unsupported. Check capability flags" );
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  antialiasing
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall antialiasing( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)

  D3DPRINT( RSTATE_DBG_LVL, "Antialiasing Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DANTIALIAS_NONE, D3DANTIALIAS_SORTINDEPENDENT);

  // If we are allowing antialiasing, then set to passed state.
  // Otherwise, force it to be off.
  if ( _D3( enableAntialias ) )
  {
    pRc->antialias = state;
  }
  else
  {
    pRc->antialias = 0;
  }

  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  edgeAntialiasing
Description:    Updates the D3DRENDERSTATE. Unused.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall edgeAntialiasing( RC *pRc, ULONG state )
{
  D3DPRINT( RSTATE_DBG_LVL, "Edge Antialiasing Renderstate %d", state );
  RENDERFXN_OK;
}

//-------------------------
//
// Performance Optimization
//
//-------------------------
#if (DX < 6)
/*-------------------------------------------------------------------
Function Name:  setDrawTriangle
Description:    Sets the function pointer for the correct type of Triangle based upon Renderstate.
Information:    (RC *pRc)
Return:         VOID

-------------------------------------------------------------------*/
void __stdcall setDrawTriangle(RC *pRc)
{
   if (pRc->fillMode != D3DFILL_SOLID)
   {
      if (pRc->fillMode == D3DFILL_POINT)
         pRc->drawTriangle = fpFillTrianglePt;
      else if (pRc->fillMode == D3DFILL_WIREFRAME)
         pRc->drawTriangle = fpFillTriangleLine;
      else
         pRc->drawTriangle = fpDrawTriangleAll;
   }

  // Need to select which drawTriangle routine will be used based
  // on the current set of render states. It turns out that performance
  // is drastically reduced if there are many if/then/elses all throughout
  // the triangle setup code. So, we will have some faster ones.
  else
  {
    switch (pRc->state & 0xFFFF)
    {
      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_RGB | STATE_REQUIRES_WRAP ) :
      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_RGB ) :
        pRc->drawTriangle = fpTriC;
        break;

      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_OOZ |
          STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_PERSPECTIVE |
          STATE_REQUIRES_W_FBI | STATE_REQUIRES_HWFOG ):
      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_IT_ALPHA |
          STATE_REQUIRES_OOZ | STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 |
          STATE_REQUIRES_PERSPECTIVE | STATE_REQUIRES_W_FBI | STATE_REQUIRES_HWFOG ):
        pRc->drawTriangle = fpTriIZTH;
        break;

      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_OOZ |
          STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_PERSPECTIVE ):
      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_IT_ALPHA |
          STATE_REQUIRES_OOZ | STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 |
          STATE_REQUIRES_PERSPECTIVE ):
        pRc->drawTriangle = fpTriIZT;
        break;

      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_OOZ ):
      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_IT_ALPHA |
          STATE_REQUIRES_OOZ ):
      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_OOZ |
          STATE_REQUIRES_PERSPECTIVE ):
      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_IT_ALPHA |
          STATE_REQUIRES_OOZ | STATE_REQUIRES_PERSPECTIVE ):
        pRc->drawTriangle = fpTriIZ;
        break;

      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_OOZ |
          STATE_REQUIRES_SPECULAR ):
      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_IT_ALPHA |
          STATE_REQUIRES_OOZ | STATE_REQUIRES_SPECULAR ):
        pRc->drawTriangle = fpTriIZS;
        break;

      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_OOZ |
          STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_PERSPECTIVE |
          STATE_REQUIRES_SPECULAR ):
      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_IT_ALPHA |
          STATE_REQUIRES_OOZ | STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 |
          STATE_REQUIRES_PERSPECTIVE | STATE_REQUIRES_SPECULAR):
        pRc->drawTriangle = fpTriIZTS;
        break;

      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB ):
      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_IT_ALPHA ):
        pRc->drawTriangle = fpTriI;
        break;

      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB |
          STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_PERSPECTIVE ):
      case ( STATE_REQUIRES_VERTS_AREA | STATE_REQUIRES_IT_RGB | STATE_REQUIRES_IT_ALPHA |
          STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_PERSPECTIVE):
        pRc->drawTriangle = fpTriIT;
        break;

      default:
        pRc->drawTriangle = fpDrawTriangleAll;
        break;
    }
  }
  CLEAR_HW_STATE(SC_TRIANGLE_FLAVOUR);
}
#endif

#if( DX >= 6 )
void __stdcall setDrawTriangle6(RC *pRc)
{
   pRc->DrawMeshAsm=DrawTriEdge2Asm;
   if (!(pRc->state & STATE_REQUIRES_ST_TMU0))
   {
      pRc->DrawMeshAsm=DrawTriEdge2GAsm;
   }
   if (pRc->state & STATE_REQUIRES_VERTEXFOG)
   {
      pRc->DrawMeshAsm=DrawTriEdge2FAsm;
   }

   CLEAR_HW_STATE(SC_TRIANGLE_FLAVOUR);
}
#endif

/*-------------------------------------------------------------------
Function Name:  zFunc
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall zFunc(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "zFunc Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DCMP_NEVER, D3DCMP_ALWAYS);

  pRc->zFunc = state;

  // clear the existing z comparison state
  pRc->sst.fbzMode &= ~(SST_ZFUNC_LT | SST_ZFUNC_EQ | SST_ZFUNC_GT) ;

  switch (state)
  {
    default:
      D3DPRINT( 255,"Warning: trying to set Invalid Z compare value" );
    case D3DCMP_LESS:
      pRc->sst.fbzMode |= SST_ZFUNC_LT;
      break;
    case D3DCMP_NEVER:
      // clearing the field above is compare never
      break;
    case D3DCMP_EQUAL:
      pRc->sst.fbzMode |= SST_ZFUNC_EQ;
      break;
    case D3DCMP_LESSEQUAL:
      pRc->sst.fbzMode |= (SST_ZFUNC_LT | SST_ZFUNC_EQ);
      break;
    case D3DCMP_GREATER:
      pRc->sst.fbzMode |= SST_ZFUNC_GT;
      break;
    case D3DCMP_NOTEQUAL:
      pRc->sst.fbzMode |= (SST_ZFUNC_LT | SST_ZFUNC_GT);
      break;
    case D3DCMP_GREATEREQUAL:
      pRc->sst.fbzMode |= (SST_ZFUNC_GT | SST_ZFUNC_EQ);
      break;
    case D3DCMP_ALWAYS:
      pRc->sst.fbzMode |= (SST_ZFUNC_LT | SST_ZFUNC_EQ | SST_ZFUNC_GT);
      break;
  }

  // If our zFunc is using LESS in the comparison then we need
  // to invert the zBias value to compensate.

  if ((state == D3DCMP_LESS) || (state == D3DCMP_LESSEQUAL))
    pRc->sst.zaColor = -((long) pRc->zBias);
  else
    pRc->sst.zaColor = pRc->zBias;

  if (0 == pRc->DDSZHndl)
  {
    if (GETPRIMARYBYTEDEPTH == 2)
      pRc->sst.zaColor &= 0x0000FFFF;   // 16bpp zbuffer
    else
      pRc->sst.zaColor &= 0x00FFFFFF;   // 24bpp zbuffer
  }
  else
  {
    // Guarantee zaColor contains the proper number of Z buffer bits.
    pRc->sst.zaColor &= TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask;
  }

#ifdef Z_ACCESS_OPT
  // To avoid triangle clears, lets zero out the counter and consider 
  // whatever operations after this to be a possible clear operation.
  if ( _DD(ddEnableZClearOpt) && 
       _DD(dd3DInOverlay) && (_FF(dd3DSurfaceCount) > 2) && (_FF(dd3DSurfaceCount) < 5) )
  {
    if (pRc->dwZClearOptEnabled)
    {
      _DD(ddFlipsWithoutZClear) = 0x80000000;
    }	 
    else
    {
      // Don't overwrite a reset status.
      if ( _DD(ddFlipsWithoutZClear) != 0x80000000 )
        _DD(ddFlipsWithoutZClear) = 0;
    }
  }
#endif

  UPDATE_HW_STATE( SC_SOMETHING | SC_ZBIAS );
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  zEnable
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall zEnable(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "zEnable Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DZB_FALSE, D3DZB_USEW);

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // move this test to setDX6State for DX7
#else
  if(pRc->lpDDSZ == NULL)
    state = 0;
#endif

#if ( DX == 5 )
  if( state > 1 )
    state = 1;
#endif

  // nothing changed
  if(state == pRc->zEnable)
    RENDERFXN_OK;

  pRc->zEnable = state;

#if( DX == 5 )
  switch( state )
  {
    case TRUE :
      pRc->sst.fbzMode |= (SST_ENDEPTHBUFFER);
      break;

    case FALSE :
      pRc->sst.fbzMode &= ~(SST_ENDEPTHBUFFER);
      break;
   }
#else
  switch( state )
  {
    case D3DZB_FALSE :
      // Disable ZBuffering
      pRc->sst.fbzMode &= ~(SST_ENDEPTHBUFFER | SST_DEPTH_FLOAT_SEL | SST_WBUFFER);
      break;
    case D3DZB_TRUE :
      // Use integer Z
      pRc->sst.fbzMode = (pRc->sst.fbzMode & ~SST_WBUFFER) | SST_ENDEPTHBUFFER;
      break;
    case D3DZB_USEW :
      // Use floating point W
      pRc->sst.fbzMode = (pRc->sst.fbzMode & ~SST_DEPTH_FLOAT_SEL) | SST_ENDEPTHBUFFER | SST_WBUFFER;
      break;
  }
#endif

  {
    DWORD offset = getFnZOffset( pRc );
    pRc->state = (pRc->state & ~FNZ_STATE_MASK) | FnZTable[ offset ][ FNZ_STATE ];
    pRc->sst.sSetupMode = (pRc->sst.sSetupMode & ~FNZ_SETUP_MASK) | FnZTable[ offset ][ FNZ_SETUP ];
    pRc->sst.fogMode = (pRc->sst.fogMode & ~FNZ_FOGMODE_MASK) | FnZTable[ offset ][ FNZ_FOGMODE ];
  }

#ifdef Z_ACCESS_OPT
  // Update CachedFBZMode to watch for updates to the ZBuffer state behind
  // our back.
  // Also, reset the FlipsWithoutZClear flag because of Revolt and their
  // ZEnable method of clearing the ZBuffer.
  if ( _DD(ddEnableZClearOpt) )
  {
    if ( _DD(dd3DInOverlay) && (_FF(dd3DSurfaceCount) > 2) && (_FF(dd3DSurfaceCount) < 5) )
    {
      pRc->dwCachedFBZMode = ((pRc->sst.fbzMode) & (SST_ENDEPTHBUFFER | SST_DEPTH_FLOAT_SEL));
    
      if (pRc->dwZClearOptEnabled)
      {
        _DD(ddFlipsWithoutZClear) = 0x80000000;
      }
      else
      {
        // Don't overwrite a reset status.
        if ( _DD(ddFlipsWithoutZClear) != 0x80000000 )
          _DD(ddFlipsWithoutZClear) = 0;
      }
    }
  }
#endif

  UPDATE_HW_STATE(SC_TRIANGLE_FLAVOUR);

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // ABCamp 12-20-99
  // Applications differ on how they use zEnable and zWriteEnable.
  // Some set zWriteEnable to TRUE for the life of the program
  // and toggle the value of zEnable.  Others, toggle both values.
  // And still others do goofy things when trying to do special
  // effects by interleaving Z and W writes.  This is an attempt 
  // to make everyone happy.
  //
  switch( state )
  {
    case D3DZB_FALSE :
      // Disable writes to the Z-Buffer
      //
      pRc->sst.fbzMode &= ~SST_ZAWRMASK ;
      UPDATE_HW_STATE(SC_SOMETHING);
      break;
    case D3DZB_TRUE :
      if ( pRc->zWriteEnable )
      {
        // Enable writes to the Z-Buffer.
        //
        pRc->sst.fbzMode |= SST_ZAWRMASK ;
        UPDATE_HW_STATE(SC_SOMETHING);
      }
      break;
    case D3DZB_USEW :
      if ( pRc->zWriteEnable )
      {
        // Enable writes to the Z-Buffer.
        pRc->sst.fbzMode |= SST_ZAWRMASK ;
        UPDATE_HW_STATE(SC_SOMETHING);
      }
      break;
  }
#endif

  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  zWriteEnable
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall zWriteEnable(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "zWriteEnable Renderstate %d", state );

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // move this test to setDX6State for DX7
#else
  if(pRc->lpDDSZ == NULL)
  {
    state = 0;
    D3DPRINT( 255, "Warning: trying to enable Z writes with no Z buffer" );
  }
#endif

  pRc->zWriteEnable = state;

  // hardware will allow writes to the depth buffer
  if ((state) && (pRc->zEnable))
    pRc->sst.fbzMode |= SST_ZAWRMASK ;
  else
    pRc->sst.fbzMode &= ~SST_ZAWRMASK ;

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  zVisible
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall zVisible(RC *pRc, ULONG state)
{
#ifdef Z_ACCESS_OPT
  SETUP_PPDEV(pRc)
#endif

  D3DPRINT( RSTATE_DBG_LVL, "zVisible Renderstate %d", state );

  pRc->zVisible = state;

#ifdef Z_ACCESS_OPT
  // To avoid triangle clears, lets zero out the counter and consider 
  // whatever operations after this to be a possible clear operation.
  if ( _DD(ddEnableZClearOpt) && 
       _DD(dd3DInOverlay) && (_FF(dd3DSurfaceCount) > 2) && (_FF(dd3DSurfaceCount) < 5) )
  { 
    if (pRc->dwZClearOptEnabled)
    {
      _DD(ddFlipsWithoutZClear) = 0x80000000;
    }
    else
    {
      // Don't overwrite a reset status.
      if ( _DD(ddFlipsWithoutZClear) != 0x80000000 )
        _DD(ddFlipsWithoutZClear) = 0;
    }
  }
#endif

  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  zbias
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall zbias(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "zBias Renderstate %d", state );

  // Direct3D defines zbias as a value in the range 1..16.
  // We scale this value by 1-bit to make the 3D Winbench
  // quality test pass on 16bpp.  At 24bpp, we scale the
  // value by an additional 8-bits, to match the 16bpp zbias
  // amount. -CGW-

  if (GETPRIMARYBYTEDEPTH == 2)
    pRc->zBias = state << 1;
  else
    pRc->zBias = state << 9;

  // If our zFunc is using LESS in the comparison then we need
  // to invert the zBias value to compensate.

  if( (pRc->zFunc == D3DCMP_LESS) || (pRc->zFunc == D3DCMP_LESSEQUAL) )
    pRc->sst.zaColor = -((long) pRc->zBias);
  else
    pRc->sst.zaColor = pRc->zBias;

  if (0 == pRc->DDSZHndl)
  {
    if (GETPRIMARYBYTEDEPTH == 2)
      pRc->sst.zaColor &= 0x0000FFFF;   // 16bpp zbuffer
    else
      pRc->sst.zaColor &= 0x00FFFFFF;   // 24bpp zbuffer
  }
  else
  {
    // Guarantee zaColor contains the proper number of Z buffer bits.
    pRc->sst.zaColor &= TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask;
  }

#ifdef Z_ACCESS_OPT
  // To avoid triangle clears, lets zero out the counter and consider 
  // whatever operations after this to be a possible clear operation.
  if ( _DD(ddEnableZClearOpt) && 
       _DD(dd3DInOverlay) && (_FF(dd3DSurfaceCount) > 2) && (_FF(dd3DSurfaceCount) < 5) )
  {
    if (pRc->dwZClearOptEnabled)
    {
      _DD(ddFlipsWithoutZClear) = 0x80000000;
    }
    else
    {
      // Don't overwrite a reset status.
      if ( _DD(ddFlipsWithoutZClear) != 0x80000000 )
        _DD(ddFlipsWithoutZClear) = 0;
    }
  }
#endif

  UPDATE_HW_STATE( SC_ZBIAS );
  RENDERFXN_OK;
}

//-------------------------------------------------------------------
// STENCIL functions

/*-------------------------------------------------------------------
Function Name:  stencilEnable
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall stencilEnable(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilEnable Renderstate %d", state );
  D3DPRINT( DLSTENCIL, "stencilEnable: Renderstate %d", state );

  if (! IS_NAPALM)
    RENDERFXN_OK;

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // move this test to setDX6State for DX7
#else
  if(pRc->lpDDSZ == NULL)
    state = 0;
#endif

#if ( DX == 5 )
  if( state > 1 )
    state = 1;
#endif

  // nothing changed
  if(state == pRc->stencilEnable)
  {
    D3DPRINT( DLSTENCIL, "stencilEnable: Nothing changed." );
    RENDERFXN_OK;
  }
  else
  {
    D3DPRINT( DLSTENCIL, "stencilEnable: Old %d, New %d", pRc->stencilEnable, state);
  }

  pRc->stencilEnable = state;

  if (state)
     pRc->sst.stencilMode |= SST_STENCIL_ENABLE;
  else
     pRc->sst.stencilMode &= ~SST_STENCIL_ENABLE;

  D3DPRINT( DLSTENCIL, "stencilEnable: stencilMode %08x", pRc->sst.stencilMode );

  UPDATE_HW_STATE(SC_TRIANGLE_FLAVOUR);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  stencilFail
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall stencilFail( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilFail Renderstate %d", state );
  D3DPRINT( DLSTENCIL, "stencilFail: Renderstate %d", state );

  CHECK_STATE_LIMITS(state, 0, D3DSTENCILOP_DECR);

  if (! IS_NAPALM)
    RENDERFXN_OK;

  // nothing changed
  if(state == pRc->stencilFail)
  {
    D3DPRINT( DLSTENCIL, "stencilFail: Nothing changed" );
    RENDERFXN_OK;
  }
  else
  {
    D3DPRINT( DLSTENCIL, "stencilFail: Old %d, New %d", pRc->stencilFail, state);
  }

  pRc->stencilFail = state;

  // clear the existing stencil fail state
  pRc->sst.stencilOp &= ~SST_STENCIL_SFAIL_OP;

  switch (state)
  {
    default:
      D3DPRINT( 0,"stencilFail: Trying to set Invalid stencil Fail value! %d", state );
	  break;
     case D3DSTENCILOP_KEEP:
      // clearing the field is to set it to keep
      D3DPRINT( DLSTENCIL, "stencilFail: Keep" );
      break;
    case D3DSTENCILOP_ZERO:
      pRc->sst.stencilOp |= SST_SOP_ZERO << SST_STENCIL_SFAIL_OP_SHIFT;
      D3DPRINT( DLSTENCIL, "stencilFail: Zero" );
      break;
    case D3DSTENCILOP_REPLACE:
      pRc->sst.stencilOp |= SST_SOP_REPLACE << SST_STENCIL_SFAIL_OP_SHIFT;
      D3DPRINT( DLSTENCIL, "stencilFail: Replace" );
      break;
    case D3DSTENCILOP_INCRSAT:
      pRc->sst.stencilOp |= SST_SOP_INCSAT << SST_STENCIL_SFAIL_OP_SHIFT;
      D3DPRINT( DLSTENCIL, "stencilFail: Increment and Clamp" );
      break;
    case D3DSTENCILOP_DECRSAT:
      pRc->sst.stencilOp |= SST_SOP_DECSAT << SST_STENCIL_SFAIL_OP_SHIFT;
      D3DPRINT( DLSTENCIL, "stencilFail: Decrement and Clamp" );
      break;
    case D3DSTENCILOP_INVERT:
      pRc->sst.stencilOp |= SST_SOP_NEG << SST_STENCIL_SFAIL_OP_SHIFT;
      D3DPRINT( DLSTENCIL, "stencilFail: Invert" );
      break;
    case D3DSTENCILOP_INCR:
      pRc->sst.stencilOp |= SST_SOP_INC << SST_STENCIL_SFAIL_OP_SHIFT;
      D3DPRINT( DLSTENCIL, "stencilFail: Increment and Wrap" );
      break;
    case D3DSTENCILOP_DECR:
      pRc->sst.stencilOp |= SST_SOP_DEC << SST_STENCIL_SFAIL_OP_SHIFT;
      D3DPRINT( DLSTENCIL, "stencilFail: Decrement and Wrap" );
      break;
  }

  D3DPRINT( DLSTENCIL, "stencilFail: stencilOp %08x", pRc->sst.stencilOp );

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  stencilZFail
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall stencilZFail( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilZFail Renderstate %d", state );
  D3DPRINT( DLSTENCIL, "stencilZFail: Renderstate %d", state );

  CHECK_STATE_LIMITS(state, 0, D3DSTENCILOP_DECR);

  if (! IS_NAPALM)
    RENDERFXN_OK;

  // nothing changed
  if(state == pRc->stencilZFail)
  {
    D3DPRINT( DLSTENCIL, "stencilZFail: Nothing changed" );
    RENDERFXN_OK;
  }
  else
  {
    D3DPRINT( DLSTENCIL, "stencilZFail: Old %d, New %d", pRc->stencilZFail, state);
  }

  pRc->stencilZFail = state;

  // clear the existing stencil fail state
  pRc->sst.stencilOp &= ~SST_STENCIL_ZFAIL_OP;

  switch (state)
  {
    default:
      D3DPRINT( 0,"stencilZFail: Trying to set Invalid stencil Z Fail value! %d", state );
	  break;
     case D3DSTENCILOP_KEEP:
      // clearing the field is to set it to keep
      D3DPRINT( DLSTENCIL, "stencilZFail: Keep" );
      break;
    case D3DSTENCILOP_ZERO:
      D3DPRINT( DLSTENCIL, "stencilZFail: Zero" );
      pRc->sst.stencilOp |= SST_SOP_ZERO << SST_STENCIL_ZFAIL_OP_SHIFT;
      break;
    case D3DSTENCILOP_REPLACE:
      D3DPRINT( DLSTENCIL, "stencilZFail: Replace" );
      pRc->sst.stencilOp |= SST_SOP_REPLACE << SST_STENCIL_ZFAIL_OP_SHIFT;
      break;
    case D3DSTENCILOP_INCRSAT:
      D3DPRINT( DLSTENCIL, "stencilZFail: Increment and Clamp" );
      pRc->sst.stencilOp |= SST_SOP_INCSAT << SST_STENCIL_ZFAIL_OP_SHIFT;
      break;
    case D3DSTENCILOP_DECRSAT:
      D3DPRINT( DLSTENCIL, "stencilZFail: Decrement and Clamp" );
      pRc->sst.stencilOp |= SST_SOP_DECSAT << SST_STENCIL_ZFAIL_OP_SHIFT;
      break;
    case D3DSTENCILOP_INVERT:
      D3DPRINT( DLSTENCIL, "stencilZFail: Invert" );
      pRc->sst.stencilOp |= SST_SOP_NEG << SST_STENCIL_ZFAIL_OP_SHIFT;
      break;
    case D3DSTENCILOP_INCR:
      D3DPRINT( DLSTENCIL, "stencilZFail: Increment and Wrap" );
      pRc->sst.stencilOp |= SST_SOP_INC << SST_STENCIL_ZFAIL_OP_SHIFT;
      break;
    case D3DSTENCILOP_DECR:
      D3DPRINT( DLSTENCIL, "stencilZFail: Decrement and Wrap" );
      pRc->sst.stencilOp |= SST_SOP_DEC << SST_STENCIL_ZFAIL_OP_SHIFT;
      break;
  }

  D3DPRINT( DLSTENCIL, "stencilZFail: stencilOp %08x", pRc->sst.stencilOp );

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  stencilPass
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall stencilPass( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilPass Renderstate %d", state );
  D3DPRINT( DLSTENCIL, "stencilPass: Renderstate %d", state );

  CHECK_STATE_LIMITS(state, 0, D3DSTENCILOP_DECR);

  if (! IS_NAPALM)
    RENDERFXN_OK;

  // nothing changed
  if(state == pRc->stencilPass)
  {
    D3DPRINT( DLSTENCIL, "stencilPass: Nothing changed" );
    RENDERFXN_OK;
  }
  else
  {
    D3DPRINT( DLSTENCIL, "stencilPass: Old %d, New %d", pRc->stencilPass, state);
  }

  pRc->stencilPass = state;

  // clear the existing stencil fail state
  pRc->sst.stencilOp &= ~SST_STENCIL_ZPASS_OP;

  switch (state)
  {
    default:
      D3DPRINT( 0,"stencilPass: Trying to set Invalid stencil Z Pass value! %d", state );
	  break;
     case D3DSTENCILOP_KEEP:
      // clearing the field is to set it to keep
      D3DPRINT( DLSTENCIL, "stencilPass: Keep" );
      break;
    case D3DSTENCILOP_ZERO:
      D3DPRINT( DLSTENCIL, "stencilPass: Zero" );
      pRc->sst.stencilOp |= SST_SOP_ZERO << SST_STENCIL_ZPASS_OP_SHIFT;
      break;
    case D3DSTENCILOP_REPLACE:
      D3DPRINT( DLSTENCIL, "stencilPass: Replace" );
      pRc->sst.stencilOp |= SST_SOP_REPLACE << SST_STENCIL_ZPASS_OP_SHIFT;
      break;
    case D3DSTENCILOP_INCRSAT:
      D3DPRINT( DLSTENCIL, "stencilPass: Increment and Clamp" );
      pRc->sst.stencilOp |= SST_SOP_INCSAT << SST_STENCIL_ZPASS_OP_SHIFT;
      break;
    case D3DSTENCILOP_DECRSAT:
      D3DPRINT( DLSTENCIL, "stencilPass: Decrement and Clamp" );
      pRc->sst.stencilOp |= SST_SOP_DECSAT << SST_STENCIL_ZPASS_OP_SHIFT;
      break;
    case D3DSTENCILOP_INVERT:
      D3DPRINT( DLSTENCIL, "stencilPass: Invert" );
      pRc->sst.stencilOp |= SST_SOP_NEG << SST_STENCIL_ZPASS_OP_SHIFT;
      break;
    case D3DSTENCILOP_INCR:
      D3DPRINT( DLSTENCIL, "stencilPass: Increment and Wrap" );
      pRc->sst.stencilOp |= SST_SOP_INC << SST_STENCIL_ZPASS_OP_SHIFT;
      break;
    case D3DSTENCILOP_DECR:
      D3DPRINT( DLSTENCIL, "stencilPass: Decrement and Wrap" );
      pRc->sst.stencilOp |= SST_SOP_DEC << SST_STENCIL_ZPASS_OP_SHIFT;
      break;
  }

  D3DPRINT( DLSTENCIL, "stencilPass: stencilOp %08x", pRc->sst.stencilOp );

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  stencilFunc
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall stencilFunc( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilFunc Renderstate %d", state );
  D3DPRINT( DLSTENCIL, "stencilFunc: Renderstate %d", state );

  CHECK_STATE_LIMITS(state, 0, D3DCMP_ALWAYS);

  if (! IS_NAPALM)
    RENDERFXN_OK;

  // nothing changed
  if(state == pRc->stencilFunc)
  {
    D3DPRINT( DLSTENCIL, "stencilFunc: Nothing changed" );
    RENDERFXN_OK;
  }
  else
  {
    D3DPRINT( DLSTENCIL, "stencilFunc: Old %d, New %d", pRc->stencilFunc, state);
  }

  pRc->stencilFunc = state;

  // clear the existing stencil function state
  pRc->sst.stencilMode &= ~SST_STENCIL_FUNC;

  switch (state)
  {
    default:
      D3DPRINT( 0,"stencilFunc: Trying to set Invalid stencil compare value! %d", state );
	  break;
    case D3DCMP_LESS:
      D3DPRINT( DLSTENCIL, "stencilFunc: Less" );
      pRc->sst.stencilMode |= SST_SFUNC_LT;
      break;
    case D3DCMP_NEVER:
      // clearing the field above is compare never
      D3DPRINT( DLSTENCIL, "stencilFunc: Never" );
      break;
    case D3DCMP_EQUAL:
      D3DPRINT( DLSTENCIL, "stencilFunc: Equal" );
      pRc->sst.stencilMode |= SST_SFUNC_EQ;
      break;
    case D3DCMP_LESSEQUAL:
      D3DPRINT( DLSTENCIL, "stencilFunc: LessEqual" );
      pRc->sst.stencilMode |= SST_SFUNC_LT | SST_SFUNC_EQ;
      break;
    case D3DCMP_GREATER:
      D3DPRINT( DLSTENCIL, "stencilFunc: Greater" );
      pRc->sst.stencilMode |= SST_SFUNC_GT;
      break;
    case D3DCMP_NOTEQUAL:
      D3DPRINT( DLSTENCIL, "stencilFunc: NotEqual" );
      pRc->sst.stencilMode |= SST_SFUNC_LT | SST_SFUNC_GT;
      break;
    case D3DCMP_GREATEREQUAL:
      D3DPRINT( DLSTENCIL, "stencilFunc: GreaterEqual" );
      pRc->sst.stencilMode |= SST_SFUNC_GT | SST_SFUNC_EQ;
      break;
    case D3DCMP_ALWAYS:
      D3DPRINT( DLSTENCIL, "stencilFunc: Always" );
      pRc->sst.stencilMode |= SST_SFUNC_LT | SST_SFUNC_EQ | SST_SFUNC_GT;
      break;
  }

  D3DPRINT( DLSTENCIL, "stencilFunc: stencilMode %08x", pRc->sst.stencilMode );

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  stencilRef
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall stencilRef( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilRef Renderstate %d", state );
  D3DPRINT( DLSTENCIL, "stencilRef: Renderstate %d", state );

  if (! IS_NAPALM)
    RENDERFXN_OK;

  // nothing changed
  if(state == pRc->stencilRef)
  {
    D3DPRINT( DLSTENCIL, "stencilRef: Nothing changed" );
    RENDERFXN_OK;
  }
  else
  {
    D3DPRINT( DLSTENCIL, "stencilRef: Old %d, New %d", pRc->stencilRef, state);
  }

  pRc->stencilRef = state;

  // clear the existing stencil reference state and set the new one
  pRc->sst.stencilMode &= ~SST_STENCIL_REF;
  pRc->sst.stencilMode |= ((state << SST_STENCIL_REF_SHIFT) & SST_STENCIL_REF);

  D3DPRINT( DLSTENCIL, "stencilRef: stencilMode %08x", pRc->sst.stencilMode );

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  stencilMask
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall stencilMask( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilMask Renderstate %d", state );
  D3DPRINT( DLSTENCIL, "stencilMask: Renderstate %08x", state );

  if (! IS_NAPALM)
    RENDERFXN_OK;

  // nothing changed
  if(state == pRc->stencilMask)
  {
    D3DPRINT( DLSTENCIL, "stencilMask: Nothing changed" );
    RENDERFXN_OK;
  }
  else
  {
    D3DPRINT( DLSTENCIL, "stencilMask: Old %08x, New %08x", pRc->stencilMask, state);
  }

  pRc->stencilMask = state;

  // clear the existing stencil mask state and set the new one
  pRc->sst.stencilMode &= ~SST_STENCIL_MASK;
  pRc->sst.stencilMode |= ((state << SST_STENCIL_MASK_SHIFT) & SST_STENCIL_MASK);

  D3DPRINT( DLSTENCIL, "stencilMask: stencilMode %08x", pRc->sst.stencilMode );

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  stencilWriteMask
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall stencilWriteMask( RC *pRc, ULONG state )
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "stencilWriteMask Renderstate %d", state );
  D3DPRINT( DLSTENCIL, "stencilWriteMask: Renderstate %08x", state );

  if (! IS_NAPALM)
    RENDERFXN_OK;

  // nothing changed
  if(state == pRc->stencilWriteMask)
  {
    D3DPRINT( DLSTENCIL, "stencilWriteMask: Nothing changed." );
    RENDERFXN_OK;
  }
  else
  {
    D3DPRINT( DLSTENCIL, "stencilWriteMask: Old %08x, New %08x", pRc->stencilWriteMask, state);
  }

  pRc->stencilWriteMask = state;

  // clear the existing stencil mask state and set the new one
  pRc->sst.stencilMode &= ~SST_STENCIL_WMASK;
  pRc->sst.stencilMode |= ((state << SST_STENCIL_WMASK_SHIFT) & SST_STENCIL_WMASK);

  D3DPRINT( DLSTENCIL, "stencilWriteMask: stencilMode %08x", pRc->sst.stencilMode );

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}

// end of STENCIL functions
//-------------------------------------------------------------------

//-------------------------------------------------------------------
//-----------
//
// Textures
//
//-----------
// textureMode for TREX 0
ULONG textureModeCombineT0[6][5] =
 {

 // D3DFILTER_NEAREST
 {(SST_TC_REPLACE | SST_TCA_REPLACE),
  (SST_TC_PASS | SST_TCA_PASS),
  (SST_TC_PASS | SST_TCA_PASS),
  (SST_TC_PASS | SST_TCA_PASS),
  (SST_TC_PASS | SST_TCA_PASS)},

 // D3DFILTER_LINEAR
 {(SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER),
  (SST_TC_PASS | SST_TCA_PASS | SST_TMINFILTER),
  (SST_TC_PASS | SST_TCA_PASS | SST_TMINFILTER),
  (SST_TC_PASS | SST_TCA_PASS | SST_TMINFILTER),
  (SST_TC_PASS | SST_TCA_PASS | SST_TMINFILTER)},

 // D3DFILTER_MIPNEAREST
 {(SST_TC_REPLACE | SST_TCA_REPLACE),
  (SST_TC_PASS | SST_TCA_PASS),
  (SST_TC_PASS | SST_TCA_PASS),
  (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC | SST_TC_REVERSE_BLEND |
   SST_TCA_REVERSE_BLEND),
  (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC)
  },

 // D3DFILTER_MIPLINEAR
 {(SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER),
  (SST_TC_PASS | SST_TCA_PASS | SST_TMINFILTER),
  (SST_TC_PASS | SST_TCA_PASS | SST_TMINFILTER),
  (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC | SST_TC_REVERSE_BLEND |
   SST_TCA_REVERSE_BLEND | SST_TMINFILTER),
  (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC | SST_TMINFILTER)
  },

 // D3DFILTER_LINEARMIPNEAREST
 {(SST_TC_REPLACE | SST_TCA_REPLACE | SST_TLODDITHER),
  (SST_TC_PASS | SST_TCA_PASS),
  (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC | SST_TC_REVERSE_BLEND |
   SST_TCA_REVERSE_BLEND),
  (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC | SST_TC_REVERSE_BLEND |
   SST_TCA_REVERSE_BLEND),
  (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC)
 },

 // D3DFILTER_LINEARMIPLINEAR
 {(SST_TC_REPLACE | SST_TCA_REPLACE | SST_TLODDITHER | SST_TMINFILTER),
  (SST_TC_PASS | SST_TCA_PASS),
  (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC | SST_TC_REVERSE_BLEND |
   SST_TCA_REVERSE_BLEND | SST_TMINFILTER),
  (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC | SST_TC_REVERSE_BLEND |
   SST_TCA_REVERSE_BLEND | SST_TMINFILTER),
  (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC | SST_TMINFILTER)
 }

} ;

ULONG textureModeCombineT1[6][5] =
 {                                     // Home

 // D3DFILTER_NEAREST
 {0,                                   // TREX0
  (SST_TC_REPLACE | SST_TCA_REPLACE),  // TREX1
  (SST_TC_REPLACE | SST_TCA_REPLACE),  // Both
  (SST_TC_REPLACE | SST_TCA_REPLACE),  // LOD split, Odd on TMU1
  (SST_TC_REPLACE | SST_TCA_REPLACE)}, // LOD split, Odd on TMU0

 // D3DFILTER_LINEAR
 {0,
  (SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER),
  (SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER),
  (SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER),
  (SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER)},

 // D3DFILTER_MIPNEAREST
 {0,
  (SST_TC_REPLACE | SST_TCA_REPLACE),
  (SST_TC_REPLACE | SST_TCA_REPLACE),
  (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE),
  (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE)},

 // D3DFILTER_MIPLINEAR
 {0,
  (SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER),
  (SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER),
  (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER),
  (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER)},

 // D3DFILTER_LINEARMIPNEAREST
 {0,
  (SST_TC_REPLACE | SST_TCA_REPLACE | SST_TLODDITHER),
  (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE ),
  (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE ),
  (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE )
 },

 // D3DFILTER_LINEARMIPLINEAR
 {0,
  (SST_TC_REPLACE | SST_TCA_REPLACE | SST_TLODDITHER | SST_TMINFILTER),
  (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER),
  (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER),
  (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE | SST_TMINFILTER)
 }

} ;
#define LODFLAGS_REMOVE (SST_LOD_TSPLIT | SST_LOD_ODD)

// include the LOD bias for each flavor below. In the case that the LOD bias is also
// supplied by the end user then we will add & clamp to the base value. The thinking
// is that end users expect the hardware to round to the closest LOD.
#define LB_HALF   (2 << SST_LODBIAS_SHIFT)
#define LB_ZERO   (0)
#define LB_DITH   (1 << SST_LODBIAS_SHIFT)

ULONG tLodT0[6][5] =
{ // TREX0,TREX1,BOTH,SPLIT ODD ON TMU1,SPLIT ODD ON TMU0
{ 0,0,0,SST_LOD_TSPLIT, SST_LOD_TSPLIT | SST_LOD_ODD },
{ 0,0,0,SST_LOD_TSPLIT, SST_LOD_TSPLIT | SST_LOD_ODD },
{ LB_HALF,0,0,LB_HALF | SST_LOD_TSPLIT | SST_LOD_ZEROFRAC, LB_HALF | SST_LOD_TSPLIT | SST_LOD_ODD | SST_LOD_ZEROFRAC},
{ LB_HALF,0,0,LB_HALF | SST_LOD_TSPLIT | SST_LOD_ZEROFRAC, LB_HALF | SST_LOD_TSPLIT | SST_LOD_ODD | SST_LOD_ZEROFRAC},
{ LB_DITH,0,0,SST_LOD_TSPLIT, SST_LOD_TSPLIT | SST_LOD_ODD },
{ LB_DITH,0,0,SST_LOD_TSPLIT, SST_LOD_TSPLIT | SST_LOD_ODD },
};

ULONG tLodT1[6][5] =
{
{ 0,0,0,          SST_LOD_TSPLIT | SST_LOD_ODD, SST_LOD_TSPLIT},
{ 0,0,0,          SST_LOD_TSPLIT | SST_LOD_ODD, SST_LOD_TSPLIT},
{ 0,LB_HALF,0, LB_HALF | SST_LOD_TSPLIT | SST_LOD_ODD | SST_LOD_ZEROFRAC, LB_HALF | SST_LOD_TSPLIT | SST_LOD_ZEROFRAC},
{ 0,LB_HALF,0, LB_HALF | SST_LOD_TSPLIT | SST_LOD_ODD | SST_LOD_ZEROFRAC, LB_HALF | SST_LOD_TSPLIT | SST_LOD_ZEROFRAC},
{ 0,LB_DITH,SST_LOD_ODD,SST_LOD_TSPLIT | SST_LOD_ODD, SST_LOD_TSPLIT},
{ 0,LB_DITH,SST_LOD_ODD,SST_LOD_TSPLIT | SST_LOD_ODD, SST_LOD_TSPLIT},
};


/*-------------------------------------------------------------------
Function Name:  textureHandle
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)(RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall textureHandle(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  DWORD oldTexture = pRc->texture;
  DWORD oldVertexColorType = pRc->vertexColorType;

  D3DPRINT( RSTATE_DBG_LVL, "textureHandle Renderstate %d", state );

#ifdef WINNT
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  if (NULL != pRc->pHndlList->ppTxtrHndlList)
  {
    CHECK_STATE_LIMITS(state, 0, (DWORD)pRc->pHndlList->ppTxtrHndlList[0]);
  }
#else
  CHECK_STATE_LIMITS(state, 0, MAXTEXTURECOUNT-1);
#endif  // DX7
#endif  // WINNT

//  if( state == pRc->texture )
//    return;

  if ( _D3( enable3DStudioMax ) )
  {
#if defined(WINNT) && (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    // to fix potential access violation issues with NT Stress
    // verify we have a TXTRHNDL for this texture otherwise
    // set the texture to zero
    if ( !(TXTRHNDL_INRANGE(state) && TXTRHNDL_PTR(state)) )
      state = 0;
    else
#endif
      state = TXTRHNDL_INUSE(state) ? (D3DTEXTUREHANDLE)state : 0;
  }

#if defined(WINNT) && (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // to fix potential access violation issues with NT Stress
  // verify we have a TXTRHNDL for this texture otherwise
  // set the texture to zero
  if ( !(TXTRHNDL_INRANGE(state) && TXTRHNDL_PTR(state)) )
    state = 0;
#endif

  pRc->texture = (D3DTEXTUREHANDLE)state;

#if ( DX >= 6 )
  TS[0].textureHandle = state;
  TS[0].changed = 1;

  // FBI pipe must be flushed when transitioning between non-textured triangles
  // and textured triangles (fixes a hang problem)
  if( ((oldTexture == 0) && (state != 0)) ||
      ((oldTexture != 0) && (state == 0)) )
  {
    // tri routine only changes on a transition
    UPDATE_HW_STATE( SC_TRIANGLE_FLAVOUR | SC_NEED_NOP );
  }
#else
  // when a texture is loaded the tLOD register is setup and so is some of the
  // texture mode
  if (pRc->texture != 0)
  {
    TXTRDESC *txtr;
    ULONG    home;
    txtr     = TXTRDESC_FROM_HNDL(state);

    // don't use mipmaps even if they are downloaded
    if (   (pRc->texMin == D3DFILTER_NEAREST)
        || (pRc->texMin == D3DFILTER_LINEAR))
    {
      pRc->sst.tLODT0 = txtr->tLODnoMipMaps[TREX0] ;
      pRc->sst.tLODT1 = txtr->tLODnoMipMaps[TREX1] ;
      home = txtr->noMipMapsHome; // largest mipmaplevel is on this tmu
      pRc->sst.baseAddr  = txtr->noMipMapsBaseAddr[0] ;
      pRc->sst.baseAddr1 = txtr->noMipMapsBaseAddr[1] ;
    }
    else
    {
      pRc->sst.tLODT0 = txtr->tLOD[TREX0] ;
      pRc->sst.tLODT1 = txtr->tLOD[TREX1] ;
      home = txtr->home;
      pRc->sst.baseAddr  = txtr->baseAddr[0] ;
      pRc->sst.baseAddr1 = txtr->baseAddr[1] ;
    }

    pRc->sst.tLODT0  |= tLodT0[pRc->texMin - 1][home - 1];
    pRc->sst.tLODT1  |= tLodT1[pRc->texMin - 1][home - 1];

    pRc->sst.textureModeT0 = textureModeCombineT0[pRc->texMin - 1][home - 1];
    pRc->sst.textureModeT1 = textureModeCombineT1[pRc->texMin - 1][home - 1];

#if (NEWASMTRI == 1)
  #if (STBKNI==1)
    // Remove the involvement of Floating Point Code
    // Store off the texture scale components
    *((int*)&(_asm_data.kni_array[SCALE_S2])) = *((int*)&(_asm_data.scale_s)) = *((int*)&(pRc->sst.scaleS)) = *((int*)&(txtr->scaleS));
    *((int*)&(_asm_data.kni_array[SCALE_T2])) = *((int*)&(_asm_data.scale_t)) = *((int*)&(pRc->sst.scaleT))  = *((int*)&(txtr->scaleT));
    // Store off the texture pixel center offset
    *((int*)&(_asm_data.kni_array[TEX_OFF_S2])) = *((int*)&(_asm_data.tex_offset_s)) = *((int*)&(pRc->sst.centerS)) = *((int*)&(txtr->centerS));
    *((int*)&(_asm_data.kni_array[TEX_OFF_T2])) = *((int*)&(_asm_data.tex_offset_t)) = *((int*)&(pRc->sst.centerT)) = *((int*)&(txtr->centerT));
  #else // Not STBKNI
    // Remove the involvement of Floating Point Code
    // Store off the texture scale components
    *((int*)&(_asm_data.scale_s)) = *((int*)&(pRc->sst.scaleS)) = *((int*)&(txtr->scaleS));
    *((int*)&(_asm_data.scale_t)) = *((int*)&(pRc->sst.scaleT))  = *((int*)&(txtr->scaleT));
    // Store off the texture pixel center offset
    *((int*)&(_asm_data.tex_offset_s)) = *((int*)&(pRc->sst.centerS)) = *((int*)&(txtr->centerS));
    *((int*)&(_asm_data.tex_offset_t)) = *((int*)&(pRc->sst.centerT)) = *((int*)&(txtr->centerT));
  #endif // STBKNI
#else // Not NEWASMTRI
    pRc->sst.scaleS    = txtr->scaleS ;
    pRc->sst.scaleT    = txtr->scaleT ;

    pRc->sst.centerS   = txtr->centerS;
    pRc->sst.centerT   = txtr->centerT;
#endif // NEWASMTRI

    // reset the fields in texture that are set when texture is loaded and then
    // merge in the new flags
    pRc->sst.textureMode &= ~(SST_TFORMAT) ;

    if( TEXFMTFLG_PALETTIZED & txtr->formatFlags)
    {
    #if( DX >= 6 )
      if( DDRAWIPAL_ALPHA & PALETTEGBL(pRc->texture)->dwFlags )
      {
        pRc->sst.textureMode |= ( TEXFMT_P8_RGBA << SST_TFORMAT_SHIFT );
        txtr->formatFlags |= TEXFMTFLG_PALETTIZED_ALPHA;
      }
      else
    #endif
      {
        pRc->sst.textureMode |= ( TEXFMT_P8_RGB << SST_TFORMAT_SHIFT );
        txtr->formatFlags &= ~TEXFMTFLG_PALETTIZED_ALPHA;
      }
    }
    else
      pRc->sst.textureMode |= txtr->format;

    // the texture blend is dependant on the format of the texture. Textures
    // with alpha are treated differently than textures without alpha. see setupcolorpath.
    if( txtr->formatFlags & (TEXFMTFLG_ALPHA | TEXFMTFLG_PALETTIZED_ALPHA))
      pRc->vertexColorType = RX_VERTEX_COLOR_RGBA;
    else
      pRc->vertexColorType = RX_VERTEX_COLOR_RGB;

    pRc->state |= (STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_W_TMU0);
    pRc->sst.sSetupMode |= (SST_SETUP_W0 | SST_SETUP_ST0);

    if (PALETTIZEDHANDLE(pRc->texture))
    {
      TXTRNEWPALETTE(ppdev,PALETTEGBL(pRc->texture));
      _D3(prevContentStamp) = 0xffffffff;
    }
  }
  else
  {
    pRc->state &= ~(STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_PERSPECTIVE);
    pRc->sst.sSetupMode &= ~(SST_SETUP_W0 | SST_SETUP_ST0);

    UPDATE_HW_STATE( SC_TLOD );
  }

  // FBI pipe must be flushed when transitioning between non-textured triangles
  // and textured triangles (fixes a hang problem)
  if( ((oldTexture == 0) && (state != 0)) ||
      ((oldTexture != 0) && (state == 0)) )
  {
    if ((state != 0) && (pRc->texturePerspective))
      pRc->state |= (STATE_REQUIRES_PERSPECTIVE);

    FXNOPCMD(ppdev);

    setupColorPath(pRc, pRc->texMapBlend) ;

    // tri routine only changes on a transition
    UPDATE_HW_STATE(SC_TRIANGLE_FLAVOUR);
  }
  else if (oldVertexColorType != pRc->vertexColorType)
    setupColorPath(pRc, pRc->texMapBlend) ;
#endif

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}


/*-------------------------------------------------------------------
Function Name:  texMag
Description:    Updates the D3DRENDERSTATE. For filtering when maximizing textures.
Information:    (RC *pRc, ULONG state)(RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall texMag(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "texMag Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DFILTER_NEAREST, D3DFILTER_LINEARMIPLINEAR);

  pRc->texMag = state;

#if( DX >= 6 )

  switch( state )
  {
    case D3DFILTER_NEAREST:
    case D3DFILTER_MIPNEAREST:
    case D3DFILTER_LINEARMIPNEAREST:
      TS[0].magFilter = D3DTFG_POINT;
      break;

    case D3DFILTER_LINEAR:
    case D3DFILTER_MIPLINEAR:
    case D3DFILTER_LINEARMIPLINEAR:
      TS[0].magFilter = D3DTFG_LINEAR;
      break;
  }

  TS[0].changed = 1;
#else
  // clear the current setting
  pRc->sst.textureMode &= ~SST_TMAGFILTER ;

  switch(state)
  {
    // point sampled
    case D3DFILTER_NEAREST :
      // SST_TMAGFILTER = 0
      break;

    // weighted average of four closest texels or bilinear filtering
    case D3DFILTER_LINEAR :
    default:
      pRc->sst.textureMode |= SST_TMAGFILTER ;
      break;
  }
#endif

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}


/*-------------------------------------------------------------------
Function Name:  texMin
Description:    Updates the D3DRENDERSTATE. For filtering when minimizing textures.
Information:    (RC *pRc, ULONG state)(RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall texMin(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)
  D3DPRINT( RSTATE_DBG_LVL, "texMin Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DFILTER_NEAREST, D3DFILTER_LINEARMIPLINEAR);

  if( _FX( flags ) & FORCE_BILINEAR )
  {
    if( D3DFILTER_NEAREST == state )
      state = D3DFILTER_LINEAR;
    else if( D3DFILTER_MIPNEAREST == state )
      state = D3DFILTER_MIPLINEAR;
  }

  if ( _D3(autoMipMap) )
  {
   if (D3DFILTER_NEAREST == state)
     state = D3DFILTER_MIPNEAREST;
   else if (D3DFILTER_LINEAR == state)
     state = D3DFILTER_MIPLINEAR;
  }

  // If the control panel has force trilinear, convert any
  // MIPMAPPING filter state into its corresponding TRILINEAR
  // state. - AS
  else if( _FX( flags ) & FORCE_TRILINEAR )
  {
    if( D3DFILTER_MIPNEAREST == state )
      state = D3DFILTER_LINEARMIPNEAREST;
    else if( D3DFILTER_MIPLINEAR == state )
      state = D3DFILTER_LINEARMIPLINEAR;
  }

  pRc->texMin = state;

#if( DX >= 6 )
  switch( state )
  {
    case D3DFILTER_NEAREST:
      TS[0].minFilter = D3DTFN_POINT;
      TS[0].mipFilter = D3DTFP_NONE;
      break;

    case D3DFILTER_LINEAR:
      TS[0].minFilter = D3DTFN_LINEAR;
      TS[0].mipFilter = D3DTFP_NONE;
      break;

    case D3DFILTER_MIPNEAREST:
      TS[0].minFilter = D3DTFN_POINT;
      TS[0].mipFilter = D3DTFP_POINT;
      break;

    case D3DFILTER_MIPLINEAR:
      TS[0].minFilter = D3DTFN_LINEAR;
      TS[0].mipFilter = D3DTFP_POINT;
      break;

    case D3DFILTER_LINEARMIPNEAREST:
      TS[0].minFilter = D3DTFN_POINT;
      TS[0].mipFilter = D3DTFP_LINEAR;
      break;

    case D3DFILTER_LINEARMIPLINEAR:
      TS[0].minFilter = D3DTFN_LINEAR;
      TS[0].mipFilter = D3DTFP_LINEAR;
      break;
  }

  TS[0].changed = 1;
#else
  // need a real texture that is in use
  if (TXTRHNDL_INRANGE(pRc->texture) && TXTRHNDL_INUSE(pRc->texture))
  {
    TXTRDESC *txtr = TXTRDESC_FROM_HNDL(pRc->texture);
    ULONG     home;

     // don't use mipmaps even if they are downloaded
    if( (pRc->texMin == D3DFILTER_NEAREST) ||
        (pRc->texMin == D3DFILTER_LINEAR) )
    {
      pRc->sst.tLODT0 = txtr->tLODnoMipMaps[TREX0] ;
      pRc->sst.tLODT1 = txtr->tLODnoMipMaps[TREX1] ;
      home = txtr->noMipMapsHome; // largest mipmaplevel is on this tmu
      pRc->sst.baseAddr  = txtr->noMipMapsBaseAddr[0] ;
      pRc->sst.baseAddr1 = txtr->noMipMapsBaseAddr[1] ;
    }
    else
    {
      pRc->sst.tLODT0 = txtr->tLOD[TREX0] ;
      pRc->sst.tLODT1 = txtr->tLOD[TREX1] ;
      home = txtr->home;
      pRc->sst.baseAddr  = txtr->baseAddr[0] ;
      pRc->sst.baseAddr1 = txtr->baseAddr[1] ;
    }

    pRc->sst.tLODT0    |= tLodT0[state - 1][home - 1];
    pRc->sst.tLODT1    |= tLodT1[state - 1][home - 1];
    pRc->sst.textureModeT0 = textureModeCombineT0[state -1][home - 1];
    pRc->sst.textureModeT1 = textureModeCombineT1[state -1][home - 1];

  }
  else return;
#endif

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}


/*-------------------------------------------------------------------
Function Name:  texturePerspective
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)(RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall texturePerspective(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "texturePerspective Renderstate %d", state );

  pRc->texturePerspective = state;

  pRc->state &=~STATE_NOT_PERSPECTIVE;

  if( state )
  {
    pRc->sst.textureMode |= SST_TPERSP_ST;
    pRc->state           |= STATE_REQUIRES_PERSPECTIVE;
  }
  else
  {
    pRc->sst.textureMode &= ~SST_TPERSP_ST;
    pRc->state           &= ~STATE_REQUIRES_PERSPECTIVE;
    pRc->state |= STATE_NOT_PERSPECTIVE;
  }

  UPDATE_HW_STATE(SC_TRIANGLE_FLAVOUR);
  RENDERFXN_OK;
}

#if ( DX >= 6 )
/*-------------------------------------------------------------------
Function Name:  textureFactor
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall textureFactor( RC *pRc, ULONG state )
{
  D3DPRINT( RSTATE_DBG_LVL, "textureFactor Renderstate %d", state );

  pRc->textureFactor = state;

  UPDATE_HW_STATE( SC_TEXTUREFACTOR );
  RENDERFXN_OK;
}


/*-------------------------------------------------------------------
Function Name:  wrap0
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall wrap0(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap0 Renderstate %d", state );

  TS[0].wrap = state;
  TS[0].changed = 1;

  if( state )
    pRc->state |= STATE_REQUIRES_WRAP;
  else
    pRc->state &= ~STATE_REQUIRES_WRAP;
  RENDERFXN_OK;
}


/*-------------------------------------------------------------------
Function Name:  wrap1
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall wrap1(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap1 Renderstate %d", state );

  TS[1].wrap = state;
  TS[1].changed = 1;

  if( state )
    pRc->state |= STATE_REQUIRES_WRAP;
  else
    pRc->state &= ~STATE_REQUIRES_WRAP;
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  wrap2
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall wrap2(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap2 Renderstate %d", state );

  TS[2].wrap = state;
  TS[2].changed = 1;

  if( state )
    pRc->state |= STATE_REQUIRES_WRAP;
  else
    pRc->state &= ~STATE_REQUIRES_WRAP;
  RENDERFXN_OK;
}


/*-------------------------------------------------------------------
Function Name:  wrap3
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall wrap3(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap3 Renderstate %d", state );

  TS[3].wrap = state;
  TS[3].changed = 1;

  if( state )
    pRc->state |= STATE_REQUIRES_WRAP;
  else
    pRc->state &= ~STATE_REQUIRES_WRAP;
  RENDERFXN_OK;
}
/*-------------------------------------------------------------------
Function Name:  wrap4
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall wrap4(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap4 Renderstate %d", state );

  TS[4].wrap = state;
  TS[4].changed = 1;

  if( state )
    pRc->state |= STATE_REQUIRES_WRAP;
  else
    pRc->state &= ~STATE_REQUIRES_WRAP;
  RENDERFXN_OK;
}


/*-------------------------------------------------------------------
Function Name:  wrap5
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall wrap5(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap5 Renderstate %d", state );

  TS[5].wrap = state;
  TS[5].changed = 1;

  if( state )
    pRc->state |= STATE_REQUIRES_WRAP;
  else
    pRc->state &= ~STATE_REQUIRES_WRAP;
  RENDERFXN_OK;
}
/*-------------------------------------------------------------------
Function Name:  wrap6
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall wrap6(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap6 Renderstate %d", state );

  TS[6].wrap = state;
  TS[6].changed = 1;

  if( state )
    pRc->state |= STATE_REQUIRES_WRAP;
  else
    pRc->state &= ~STATE_REQUIRES_WRAP;
  RENDERFXN_OK;
}


/*-------------------------------------------------------------------
Function Name:  wrap7
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall wrap7(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrap7 Renderstate %d", state );

  TS[7].wrap = state;
  TS[7].changed = 1;

  if( state )
    pRc->state |= STATE_REQUIRES_WRAP;
  else
    pRc->state &= ~STATE_REQUIRES_WRAP;
  RENDERFXN_OK;
}

#endif


/*-------------------------------------------------------------------
Function Name:  wrapU
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall wrapU(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrapU Renderstate %d", state );

  pRc->wrapU = state;

  if( state )
  {
    pRc->state |= STATE_REQUIRES_WRAP;

  #if ( DX >= 6 )
    TS[0].wrap |= D3DWRAP_U;
    TS[0].changed = 1;
  #endif
  }
  else if( !pRc->wrapV )
  {
    pRc->state &= ~STATE_REQUIRES_WRAP;

  #if ( DX >= 6 )
    TS[0].wrap &= ~D3DWRAP_U;
    TS[0].changed = 1;
  #endif
  }

  UPDATE_HW_STATE(SC_TRIANGLE_FLAVOUR);
  RENDERFXN_OK;
}


/*-------------------------------------------------------------------
Function Name:  wrapV
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall wrapV(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "wrapV Renderstate %d", state );

  pRc->wrapV = state;

  if( state )
  {
    pRc->state |= STATE_REQUIRES_WRAP;

  #if ( DX >= 6 )
    TS[0].wrap |= D3DWRAP_V;
    TS[0].changed = 1;
  #endif
  }
  else if( !pRc->wrapU )
  {
    pRc->state &= ~STATE_REQUIRES_WRAP;

  #if ( DX >= 6 )
    TS[0].wrap &= ~D3DWRAP_V;
    TS[0].changed = 1;
  #endif
  }

  UPDATE_HW_STATE(SC_TRIANGLE_FLAVOUR);
  RENDERFXN_OK;
}

//-------------------------------------------------------------------
// DirectX 5.0 - users can now set U and V control separately as defined
// below. The problem is that the textureAddress state can be opposite of
// the addressU and addressV and incorrect. Users should be careful mixing
// and matching the three renderstates.
/*-------------------------------------------------------------------
Function Name:  textureAddress
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall textureAddress(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "textureAddress Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DTADDRESS_WRAP, D3DTADDRESS_BORDER);

  pRc->textureAddress = state;

  textureAddressU (pRc, state);
  textureAddressV (pRc, state);
  RENDERFXN_OK;
} // End textureAddress


/*-------------------------------------------------------------------
Function Name:  textureAddressU
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall textureAddressU(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "textureAddressU Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DTADDRESS_WRAP, D3DTADDRESS_BORDER);

  pRc->textureAddressU = state;

#if ( DX >= 6 )
  TS[0].addressU = state;
  TS[0].changed = 1;
#else
  if (state == D3DTADDRESS_CLAMP)
    pRc->sst.textureMode |= SST_TCLAMPS;
  else
    pRc->sst.textureMode &= ~SST_TCLAMPS;
#endif

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
} // End textureAddressU


/*-------------------------------------------------------------------
Function Name:  textureAddressV
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall textureAddressV(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "textureAddressV Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DTADDRESS_WRAP, D3DTADDRESS_BORDER);

  pRc->textureAddressV = state;

#if ( DX >= 6 )
  TS[0].addressV = state;
  TS[0].changed = 1;
#else
  if (state == D3DTADDRESS_CLAMP)
    pRc->sst.textureMode |= SST_TCLAMPT ;
  else
    pRc->sst.textureMode &= ~SST_TCLAMPT ;
#endif

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
} // End textureAddressV


/*-------------------------------------------------------------------
Function Name:  setupColorPath
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall setupColorPath(RC *pRc, ULONG state)
{
  // any case, reset alpha channel write mask mode
  pRc->sst.fbzMode &= ~SST_ENALPHAMASK ;

  // texture mapping is disabled so setup the chip to default to
  // solid/flat/gouraud shading
  if (pRc->texture == 0)
  {
    pRc->sst.fbzColorPath = DEFAULT_FBZCOLORPATH ;

    if (pRc->subPixel == TRUE)
      pRc->sst.fbzColorPath |= SST_PARMADJUST ;
  }
  else
  {
    // start from scratch by initializing the subpixel correction
    if (pRc->subPixel == TRUE)
      pRc->sst.fbzColorPath = SST_PARMADJUST ;
    else
      pRc->sst.fbzColorPath = 0 ;

    // reset the state of texture mapping
    pRc->sst.fbzColorPath |=  SST_ENTEXTUREMAP ;

    switch(state)
    {

    //  Texture Map Blending
    //    KEY: tex = texture pixel for each pixel
    //         pix = polygon's flat, s oid, or smooth shaded color for pixel
    //
    //                             RGB        Alpha      RGBA                    Alpha
    //                             ---                   ----                    -----
    //  D3DTBLEND_DECALALPHA       Ctex        Apix     (1-Atex)Cpix+(Atex)(Ctex)  Apix
    //  D3DTBLEND_MODULATEALPHA   (Ctex)(Cpix) Apix     (Ctex)(Cpix)              (Apix)(Atex)
    //  D3DTBLEND_DECAL            Ctex        Apix      Ctex                      Atex
    //  D3DTBLEND_MODULATE        (Ctex)(Cpix) Apix     (Ctex)(Cpix)               Atex
    //  D3DTBLEND_DECALMASK        Ctex        Apix      Ctex if LSBofAlpha = 1    Apix
    //  D3DTBLEND_MODULATEMASK    (Ctex)(Cpix) Apix     (Ctex)(Cpix) if LSBofA=1   Apix
      default:
        D3DPRINT( 255, "Warning: Texture map blend is invalid." );

      case D3DTBLEND_DECALALPHA :
        if (pRc->vertexColorType == RX_VERTEX_COLOR_RGB)
          pRc->sst.fbzColorPath  |= (  SST_RGBSEL_TREXOUT
                                 | SST_CCA_ZERO_OTHER | SST_CCA_ADD_CLOCAL);
        else
          pRc->sst.fbzColorPath  |= (  SST_RGBSEL_TREXOUT | SST_CC_SUB_CLOCAL
                                 | SST_CC_MATREX      | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND
                                 | SST_CCA_ZERO_OTHER | SST_CCA_ADD_CLOCAL);
        break;
      case D3DTBLEND_MODULATEALPHA :
        if (pRc->vertexColorType == RX_VERTEX_COLOR_RGB)
          pRc->sst.fbzColorPath  |= (  SST_RGBSEL_TREXOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND
                                 | SST_CCA_ZERO_OTHER | SST_CCA_ADD_CLOCAL);
        else
          pRc->sst.fbzColorPath  |= (  SST_RGBSEL_TREXOUT | SST_CC_MCLOCAL  | SST_CC_REVERSE_BLEND
                                 | SST_ASEL_TREXOUT   | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND);
        break;
      case D3DTBLEND_COPY :
      case D3DTBLEND_DECAL :
        // ATS REMOVED : d3d ddk spec says that alpha=1 if no alpha in texture
        if (pRc->vertexColorType == RX_VERTEX_COLOR_RGB)
          pRc->sst.fbzColorPath  |= (  SST_RGBSEL_TREXOUT
                                 | SST_CCA_ZERO_OTHER | SST_CCA_ADD_CLOCAL); //ATS - SST_CCA_INVERT_OUTPUT);
        else
          pRc->sst.fbzColorPath  |= (  SST_RGBSEL_TREXOUT
                                 | SST_ASEL_TREXOUT);
        break;
      case D3DTBLEND_MODULATE :
        // ATS REMOVED : d3d ddk spec says that alpha=1 if no alpha in texture
        if (pRc->vertexColorType == RX_VERTEX_COLOR_RGB)
          pRc->sst.fbzColorPath  |= (  SST_RGBSEL_TREXOUT | SST_CC_MCLOCAL  | SST_CC_REVERSE_BLEND
                                 | SST_CCA_ZERO_OTHER | SST_CCA_ADD_CLOCAL); //ATS - SST_CCA_INVERT_OUTPUT);
        else
          pRc->sst.fbzColorPath  |= (  SST_RGBSEL_TREXOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND
                                 | SST_ASEL_TREXOUT);
        break;
      case D3DTBLEND_DECALMASK :
        if (pRc->vertexColorType == RX_VERTEX_COLOR_RGB)
          pRc->sst.fbzColorPath  |= (  SST_RGBSEL_TREXOUT
                                 | SST_CCA_ZERO_OTHER | SST_CCA_ADD_CLOCAL);
        else
        {
          pRc->sst.fbzMode       |= SST_ENALPHAMASK ;
          pRc->sst.fbzColorPath  |= (  SST_RGBSEL_TREXOUT | SST_ASEL_TREXOUT
                                 | SST_CCA_ZERO_OTHER | SST_CCA_ADD_CLOCAL);

        }
        break;
      case D3DTBLEND_MODULATEMASK :
        if (pRc->vertexColorType == RX_VERTEX_COLOR_RGB)
          pRc->sst.fbzColorPath  |= (  SST_RGBSEL_TREXOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND
                                 | SST_CCA_ZERO_OTHER | SST_CCA_ADD_CLOCAL);
        else
        {
          pRc->sst.fbzMode       |= SST_ENALPHAMASK ;
          pRc->sst.fbzColorPath  |= (  SST_RGBSEL_TREXOUT | SST_CC_MCLOCAL     | SST_CC_REVERSE_BLEND
                                 | SST_ASEL_TREXOUT   | SST_CCA_ZERO_OTHER | SST_CCA_ADD_CLOCAL);
         }
         break;
      case D3DTBLEND_ADD :
        // ATS REMOVED : d3d ddk spec says that alpha=1 if no alpha in texture
        if (pRc->vertexColorType == RX_VERTEX_COLOR_RGB)
          pRc->sst.fbzColorPath  |= (SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL); //ATS - SST_CCA_INVERT_OUTPUT);
        else
          pRc->sst.fbzColorPath  |= (SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL | SST_ASEL_TMUOUT);
        break;
    } // switch
  } // texture map is enabled

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(REMOVE_OBSOLETE1_FOR_DX7_WIN9X) && !defined(WINNT)
#else
/*-------------------------------------------------------------------
Function Name:  texMapBlend
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall texMapBlend(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "texMapBlend Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DTBLEND_DECAL, D3DTBLEND_ADD);

  pRc->texMapBlend = state;

#if ( DX >= 6)
  switch( state )
  {
    case D3DTBLEND_DECALALPHA:

      TS[0].colorOp   = D3DTOP_BLENDTEXTUREALPHA;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].colorArg2 = D3DTA_DIFFUSE;
      TS[0].alphaOp   = D3DTOP_SELECTARG2;
      TS[0].alphaArg2 = D3DTA_DIFFUSE;
      pRc->sst.fbzMode &= ~SST_ENALPHAMASK ;
      break;

    case D3DTBLEND_MODULATEALPHA:

      TS[0].colorOp   = D3DTOP_MODULATE;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].colorArg2 = D3DTA_DIFFUSE;
      TS[0].alphaOp   = D3DTOP_MODULATE;
      TS[0].alphaArg1 = D3DTA_TEXTURE;
      TS[0].alphaArg2 = D3DTA_DIFFUSE;
      pRc->sst.fbzMode &= ~SST_ENALPHAMASK ;
      break;

    case D3DTBLEND_COPY:
    case D3DTBLEND_DECAL:

      TS[0].colorOp   = D3DTOP_SELECTARG1;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].alphaOp   = D3DTOP_SELECTARG1;
      TS[0].alphaArg1 = D3DTA_TEXTURE;
      pRc->sst.fbzMode &= ~SST_ENALPHAMASK;
      break;

    case D3DTBLEND_MODULATE:

      TS[0].colorOp   = D3DTOP_MODULATE;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].colorArg2 = D3DTA_DIFFUSE;
      TS[0].alphaOp   = D3DTOP_SELECTARG1;
      TS[0].alphaArg1 = D3DTA_TEXTURE;
      pRc->sst.fbzMode &= ~SST_ENALPHAMASK;
      pRc->texMapBlend = 0x7ffffffe; // internal legacy setting for D3DTBLEND_MODULATE used in setupTexturing()
      break;

    case D3DTBLEND_DECALMASK:

      TS[0].colorOp   = D3DTOP_SELECTARG1;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].alphaOp   = D3DTOP_SELECTARG1;
      TS[0].alphaArg1 = D3DTA_TEXTURE;
      pRc->sst.fbzMode |= SST_ENALPHAMASK;
      break;

    case D3DTBLEND_MODULATEMASK:

      TS[0].colorOp   = D3DTOP_MODULATE;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].colorArg2 = D3DTA_DIFFUSE;
      TS[0].alphaOp   = D3DTOP_MODULATE;
      TS[0].alphaArg1 = D3DTA_TEXTURE;
      TS[0].alphaArg2 = D3DTA_DIFFUSE;
      pRc->sst.fbzMode |= SST_ENALPHAMASK;
      break;

    case D3DTBLEND_ADD:

      TS[0].colorOp   = D3DTOP_ADD;
      TS[0].colorArg1 = D3DTA_TEXTURE;
      TS[0].colorArg2 = D3DTA_DIFFUSE;
      TS[0].alphaOp   = D3DTOP_SELECTARG2;
      TS[0].alphaArg1 = D3DTA_TEXTURE;
      TS[0].alphaArg2 = D3DTA_DIFFUSE;
      pRc->sst.fbzMode &= ~SST_ENALPHAMASK;
      break;
  }

  TS[0].changed = 1;
#else
  setupColorPath(pRc, state);
#endif
  RENDERFXN_OK;
}
#endif //#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(REMOVE_OBSOLETE1_FOR_DX7_WIN9X) && !defined(WINNT)

//------------------------------
//
// Alpha Comparison and Blending
//
//------------------------------
/*-------------------------------------------------------------------
Function Name:  blendEnable
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall blendEnable(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "blendEnable Renderstate %d", state );

  pRc->blendEnable = state;

  // Treat BlendEnable as alpha enable only
  // for backwards compatiblity with DX 3.0 apps
  alphaBlendEnable (pRc, state);

  RENDERFXN_OK;
} // End blendEnable


/*-------------------------------------------------------------------
Function Name:  alphaBlendEnable
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall alphaBlendEnable(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "alphaBlendEnable Renderstate %d", state );

  pRc->alphaBlendEnable = state;

#if ( DX == 5 )
  // no alpha blend or alpha test (clear it first)
  if (!pRc->alphaTestEnable)
    pRc->state &= ~STATE_REQUIRES_IT_ALPHA;

  if (state)
  {
    pRc->sst.alphaMode |= SST_ENALPHABLEND;
    pRc->state |= STATE_REQUIRES_IT_ALPHA;
    pRc->sst.sSetupMode |= SST_SETUP_A;
  }
  else
  {
    pRc->sst.alphaMode &= ~SST_ENALPHABLEND;

    if(!(pRc->state & (STATE_REQUIRES_IT_RGB | STATE_REQUIRES_IT_ALPHA)))
      pRc->sst.sSetupMode &= ~SST_SETUP_A;
  }
#else
  if ( state )
  {
    pRc->sst.alphaMode |= SST_ENALPHABLEND;
  }
  else
  {
    pRc->sst.alphaMode &= ~SST_ENALPHABLEND;
  }
#endif

  if (pRc->fogEnable)
    setFogMode(pRc);

  UPDATE_HW_STATE(SC_TRIANGLE_FLAVOUR);
  RENDERFXN_OK;
}


// in DX 5.0 colorkey control and alpha blendenable are controlled via
// separate renderstates.
/*-------------------------------------------------------------------
Function Name:  colorKeyEnable
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall colorKeyEnable(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "colorKeyEnable Renderstate %d", state );

  pRc->colorKeyEnable = state;

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}


//---------------------------------------------------------
//  Alpha blend:
//
//  Cout = ( Cpix * srcBlend) + (Cfb * dstBlend)
//  where Cpix is the pixel coming out of the pipeline and
//        Cfb  is the pixel in the frame buffer
//
/*-------------------------------------------------------------------
Function Name:  srcBlend
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall srcBlend(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc);

  D3DPRINT( RSTATE_DBG_LVL, "srcBlend Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DBLEND_ZERO, D3DBLEND_BOTHINVSRCALPHA);

  pRc->srcBlend = state ;

  // clear the existing Blend mode
  pRc->sst.alphaMode &= ~(SST_RGBSRCFACT | SST_ASRCFACT);

  switch (state)
  {
    case  D3DBLEND_ZERO         :  // (0,0,0,0)
      // clearing the existing Blend mode is blend zero
      break;

    default :
      D3DPRINT(1,"Warning: trying to set invalid source blend value") ;

    case  D3DBLEND_ONE          :  // (1,1,1,1)
      pRc->sst.alphaMode |= ( (SST_A_ONE << SST_RGBSRCFACT_SHIFT)  |
                              (SST_A_ONE << SST_ASRCFACT_SHIFT)    );
      break;

    // Voodoo3: THIS is wrong!!! SST will use Rfb,Gfb,Bfb,Afb
    // This is a bogus combination... Doug says that only OpenGL combinations
    // should be supported.
    case  D3DBLEND_SRCCOLOR     :  // (Rpix, Gpix, Bpix, Apix)
      if (IS_NAPALM)
      {
        pRc->sst.alphaMode |= ( (SST_A_SAMECOLOR << SST_RGBSRCFACT_SHIFT) |
                                (SST_A_SRCALPHA  << SST_ASRCFACT_SHIFT)   );
      }
      else
      {
        pRc->sst.alphaMode |= (SST_A_COLOR << SST_RGBSRCFACT_SHIFT);
      }
      break;

    // Voodoo3: THIS is wrong!!! SST will use 1-Rfb,1-Gfb,1-Bfb,1-Afb
    case  D3DBLEND_INVSRCCOLOR  :  // (1-Rpix, 1-Gpix, 1-Bpix, 1-Apix)
      if (IS_NAPALM)
      {
        pRc->sst.alphaMode |= ( (SST_AOM_SAMECOLOR << SST_RGBSRCFACT_SHIFT) |
                                (SST_AOM_SRCALPHA  << SST_ASRCFACT_SHIFT)   );
      }
      else
      {
        pRc->sst.alphaMode |= (SST_AOM_COLOR << SST_RGBSRCFACT_SHIFT);
      }
      break;

    case  D3DBLEND_SRCALPHA     :  // (Apix, Apix, Apix, Apix)
      pRc->sst.alphaMode |= ( (SST_A_SRCALPHA << SST_RGBSRCFACT_SHIFT) |
                              (SST_A_SRCALPHA << SST_ASRCFACT_SHIFT)   );
      break;

    case  D3DBLEND_INVSRCALPHA  :  // (1-Apix, 1-Apix, 1-Apix, 1-Apix)
      pRc->sst.alphaMode |= ( (SST_AOM_SRCALPHA << SST_RGBSRCFACT_SHIFT) |
                              (SST_AOM_SRCALPHA << SST_ASRCFACT_SHIFT)   );
      break;

    case  D3DBLEND_DESTALPHA    : //  (Afb, Afb, Afb, Afb
      if (IS_NAPALM)
      {
        pRc->sst.alphaMode |= ( (SST_A_DSTALPHA << SST_RGBSRCFACT_SHIFT) |
                                (SST_A_DSTALPHA << SST_ASRCFACT_SHIFT)   );
      }
      break;

    case  D3DBLEND_INVDESTALPHA : //  (1-Afb, 1-Afb, 1-Afb, 1-Afb)
      if (IS_NAPALM)
      {
        pRc->sst.alphaMode |= ( (SST_AOM_DSTALPHA << SST_RGBSRCFACT_SHIFT) |
                                (SST_AOM_DSTALPHA << SST_ASRCFACT_SHIFT)   );
      }
      break;

    case  D3DBLEND_DESTCOLOR    : //  (Rfb, Gfb, Bfb, Afb)
      pRc->sst.alphaMode |= ( (SST_A_COLOR    << SST_RGBSRCFACT_SHIFT) |
                              (SST_A_DSTALPHA << SST_ASRCFACT_SHIFT)   );
      break;

    case  D3DBLEND_INVDESTCOLOR : //  (1-Rb, 1-Gfb, 1-Bfb, 1-Afb)
      pRc->sst.alphaMode |= ( (SST_AOM_COLOR    << SST_RGBSRCFACT_SHIFT) |
                              (SST_AOM_DSTALPHA << SST_ASRCFACT_SHIFT)   );
      break;

    case  D3DBLEND_SRCALPHASAT :  //  f=min(Apix,1-Afb) (f, f, f, 1)
      if (IS_NAPALM)
      {
        pRc->sst.alphaMode |= ( (SST_A_SATURATE << SST_RGBSRCFACT_SHIFT) |
                                (SST_A_ONE      << SST_ASRCFACT_SHIFT)   );
      }
      break;

    case  D3DBLEND_BOTHSRCALPHA: // srcBlend = (Apix, Apix, Apix, Apix)
                                 // dstBlend = (1-Apix, 1-Apix, 1-Apix, 1-Apix)
      pRc->sst.alphaMode &= ~(SST_RGBDSTFACT | SST_ADSTFACT);
      pRc->sst.alphaMode |= ( (SST_A_SRCALPHA   << SST_RGBSRCFACT_SHIFT) |
                              (SST_AOM_SRCALPHA << SST_RGBDSTFACT_SHIFT) |
                              (SST_A_SRCALPHA   << SST_ASRCFACT_SHIFT)   |
                              (SST_AOM_SRCALPHA << SST_ADSTFACT_SHIFT)   );
      break;

    case  D3DBLEND_BOTHINVSRCALPHA : // srcBlend = (1-Apix, 1-Apix, 1-Apix, 1-Apix)
                                     // dstblend = (Apix, Apix, Apix, Apix)
      pRc->sst.alphaMode &= ~(SST_RGBDSTFACT | SST_ADSTFACT);
      pRc->sst.alphaMode |= ( (SST_AOM_SRCALPHA << SST_RGBSRCFACT_SHIFT) |
                              (SST_A_SRCALPHA   << SST_RGBDSTFACT_SHIFT) |
                              (SST_AOM_SRCALPHA << SST_ASRCFACT_SHIFT)   |
                              (SST_A_SRCALPHA   << SST_ADSTFACT_SHIFT)   );
      break;
  }

  UPDATE_HW_STATE(SC_ALPHABLEND);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  dstBlend
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall dstBlend(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc);

  D3DPRINT( RSTATE_DBG_LVL, "dstBlend Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DBLEND_ZERO, D3DBLEND_BOTHINVSRCALPHA);

  pRc->dstBlend = state;

  // if application has set SRC blend to BOTHxx then it overrides destination

  // clear the existing Blend mode
  pRc->sst.alphaMode &= ~(SST_RGBDSTFACT | SST_ADSTFACT);

  switch(state)
  {
    default:
      D3DPRINT( 255,"Warning: trying to set invalid destination blend value" );

    case  D3DBLEND_ZERO          :   // (0,0,0,0)
      // clearing the existing Blend mode is blend zero
      break;

    case  D3DBLEND_ONE           :   // (1,1,1,1)
       pRc->sst.alphaMode |= ( (SST_A_ONE << SST_RGBDSTFACT_SHIFT) |
                               (SST_A_ONE << SST_ADSTFACT_SHIFT)   );
       break;

    case  D3DBLEND_SRCCOLOR     :    // (Rpix, Gpix, Bpix, Apix)
       pRc->sst.alphaMode |= ( (SST_A_COLOR    << SST_RGBDSTFACT_SHIFT) |
                               (SST_A_SRCALPHA << SST_ADSTFACT_SHIFT) );
       break;

    case  D3DBLEND_INVSRCCOLOR :     // (1-Rpix, 1-Gpix, 1-Bpix, 1-Apix)
       pRc->sst.alphaMode |= ( (SST_AOM_COLOR    << SST_RGBDSTFACT_SHIFT) |
                               (SST_AOM_SRCALPHA << SST_ADSTFACT_SHIFT)   );
       break;

    case  D3DBLEND_SRCALPHA     :    // (Apix, Apix, Apix, Apix)
       pRc->sst.alphaMode |= ( (SST_A_SRCALPHA << SST_RGBDSTFACT_SHIFT) |
                               (SST_A_SRCALPHA << SST_ADSTFACT_SHIFT)   );
       break;

    case  D3DBLEND_INVSRCALPHA :     // (1-Apix, 1-Apix, 1-Apix, 1-Apix)
       pRc->sst.alphaMode |= ( (SST_AOM_SRCALPHA << SST_RGBDSTFACT_SHIFT) |
                               (SST_AOM_SRCALPHA << SST_ADSTFACT_SHIFT)   );
       break;

    case  D3DBLEND_DESTALPHA     :   // (Afb, Afb, Afb, Afb
       if (IS_NAPALM)
       {
         pRc->sst.alphaMode |= ( (SST_A_DSTALPHA << SST_RGBDSTFACT_SHIFT) |
                                 (SST_A_DSTALPHA << SST_ADSTFACT_SHIFT)   );
       }
       break;

    case  D3DBLEND_INVDESTALPHA :    // (1-Afb, 1-Afb, 1-Afb, 1-Afb)
       if (IS_NAPALM)
       {
         pRc->sst.alphaMode |= ( (SST_AOM_DSTALPHA << SST_RGBDSTFACT_SHIFT) |
                                 (SST_AOM_DSTALPHA << SST_ADSTFACT_SHIFT)   );
       }
       break;

    // Voodoo3: THIS is wrong!!! SST will use Rpix,Gpix,Bpix,Apix
    // This is a bogus combination...
    case  D3DBLEND_DESTCOLOR    :   //  (Rfb, Gfb, Bfb, Afb)
       if (IS_NAPALM)
       {
            pRc->sst.alphaMode |= ( (SST_A_SAMECOLOR << SST_RGBDSTFACT_SHIFT) |
                                    (SST_A_DSTALPHA  << SST_ADSTFACT_SHIFT)   );
       }
       else
       {
            pRc->sst.alphaMode |= (SST_A_COLOR << SST_RGBDSTFACT_SHIFT);
       }
       break;

    // Voodoo3: THIS is wrong!!! SST will use 1-Rpix,1-Gpix,1-Bpix,1-Apix
    case  D3DBLEND_INVDESTCOLOR : //  (1-Rb, 1-Gfb, 1-Bfb, 1-Afb)
       if (IS_NAPALM)
       {
            pRc->sst.alphaMode |= ( (SST_AOM_SAMECOLOR << SST_RGBDSTFACT_SHIFT) |
                                    (SST_AOM_DSTALPHA  << SST_ADSTFACT_SHIFT)   );
       }
       else
       {
            pRc->sst.alphaMode |= (SST_AOM_COLOR << SST_RGBDSTFACT_SHIFT) ;
       }
       break;

    case  D3DBLEND_SRCALPHASAT  :   //  f=min(Apix,1-Afb) (f, f, f, 1)
       if (IS_NAPALM)
       {
         pRc->sst.alphaMode |= ( (SST_A_COLORBEFOREFOG << SST_RGBDSTFACT_SHIFT) |
                               (SST_A_ONE            << SST_ADSTFACT_SHIFT)   );
       }
       break;

    case  D3DBLEND_BOTHSRCALPHA :   // see srcBlend
       pRc->sst.alphaMode &= ~(SST_RGBSRCFACT | SST_ASRCFACT);
       pRc->sst.alphaMode |= ( (SST_A_SRCALPHA   << SST_RGBSRCFACT_SHIFT) |
                               (SST_AOM_SRCALPHA << SST_RGBDSTFACT_SHIFT) |
                               (SST_A_SRCALPHA   << SST_ASRCFACT_SHIFT)   |
                               (SST_AOM_SRCALPHA << SST_ADSTFACT_SHIFT)   );
        break;

    case  D3DBLEND_BOTHINVSRCALPHA : // see srcBlend
       pRc->sst.alphaMode &= ~(SST_RGBSRCFACT | SST_ASRCFACT);
       pRc->sst.alphaMode |= ( (SST_AOM_SRCALPHA << SST_RGBSRCFACT_SHIFT) |
                               (SST_A_SRCALPHA   << SST_RGBDSTFACT_SHIFT) |
                               (SST_AOM_SRCALPHA << SST_ASRCFACT_SHIFT)   |
                               (SST_A_SRCALPHA   << SST_ADSTFACT_SHIFT)   );
       break;
  }

  UPDATE_HW_STATE(SC_ALPHABLEND);
  RENDERFXN_OK;
}


/*-------------------------------------------------------------------
Function Name:  alphaTestEnable
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall alphaTestEnable(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "alphaTestEnable Renderstate %d", state );

  pRc->alphaTestEnable = state;

#if ( DX == 5 )
  if (state)
  {
    pRc->sst.alphaMode |= SST_ENALPHAFUNC;
    pRc->state |= STATE_REQUIRES_IT_ALPHA;
    pRc->sst.sSetupMode |= SST_SETUP_A;
  }
  else
  {
    pRc->sst.alphaMode &= ~SST_ENALPHAFUNC;

    // no alpha blending or alpha test
    if (!pRc->alphaBlendEnable)
    {
      pRc->state &= ~STATE_REQUIRES_IT_ALPHA;
      pRc->sst.sSetupMode &= ~SST_SETUP_A;
    }
  }
#else
  if ( state )
    pRc->sst.alphaMode |= SST_ENALPHAFUNC;
  else
    pRc->sst.alphaMode &= ~SST_ENALPHAFUNC;
#endif

  UPDATE_HW_STATE(SC_TRIANGLE_FLAVOUR);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  alphaRef
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall alphaRef(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "alphaRef Renderstate %d", state );

  // D3D alpha ranges from 0 to 255 which is the same as our hardware
  // but our format is 16.16 so take 8 bits of the integer part
  pRc->alphaRef       = state;
  pRc->sst.alphaMode &= ~SST_ALPHAREF ;
  pRc->sst.alphaMode |= state << SST_ALPHAREF_SHIFT ;

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  alphaFunc
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall alphaFunc(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "alphaFunc Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DCMP_NEVER, D3DCMP_ALWAYS);

  // Nothing changed
  if(state == pRc->alphaFunc)
    RENDERFXN_OK;

  pRc->alphaFunc = state;

  // clear the existing alpha comparison state
  pRc->sst.alphaMode &= ~(SST_ALPHAFUNC_LT | SST_ALPHAFUNC_EQ | SST_ALPHAFUNC_GT) ;

  switch (state)
  {
    default:
      D3DPRINT( 255,"Warning: trying to set invalid alpha comparison value" );
    case D3DCMP_LESS:
      pRc->sst.alphaMode |= SST_ALPHAFUNC_LT;
      break;

    case D3DCMP_NEVER:
      // clearing the field above is compare never
      break;

    case D3DCMP_EQUAL:
      pRc->sst.alphaMode |= SST_ALPHAFUNC_EQ;
      break;

    case D3DCMP_LESSEQUAL:
      pRc->sst.alphaMode |= (SST_ALPHAFUNC_LT | SST_ALPHAFUNC_EQ);
      break;

    case D3DCMP_GREATER:
      pRc->sst.alphaMode |= SST_ALPHAFUNC_GT;
      break;

    case D3DCMP_NOTEQUAL:
      pRc->sst.alphaMode |= (SST_ALPHAFUNC_LT | SST_ALPHAFUNC_GT);
      break;

    case D3DCMP_GREATEREQUAL:
      pRc->sst.alphaMode |= (SST_ALPHAFUNC_GT | SST_ALPHAFUNC_EQ);
      break;

    case D3DCMP_ALWAYS:
      pRc->sst.alphaMode |= (SST_ALPHAFUNC_LT | SST_ALPHAFUNC_EQ | SST_ALPHAFUNC_GT);
      break;
  }

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}



/*-------------------------------------------------------------------
Function Name:  cullMode
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall cullMode(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "cullMode Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DCULL_NONE, D3DCULL_CCW);

  // Nothing has changed
  if (state == pRc->cullMode)
    RENDERFXN_OK;

  pRc->cullMode = state;

  switch(state)
  {
    case D3DCULL_NONE:
      pRc->cullMask = 0xFFFFFFFF;
      pRc->sst.sSetupMode &= ~(SST_SETUP_EN_CULLING | SST_SETUP_CULL_NEGATIVE);
      pRc->strip_mode=0x1000bc53;
      break;

    case D3DCULL_CW:
      pRc->cullMask = 0x80000000;
      pRc->sst.sSetupMode = (pRc->sst.sSetupMode & ~SST_SETUP_CULL_NEGATIVE) | SST_SETUP_EN_CULLING;
      pRc->strip_mode=0x1000bc53 | BIT(23);
      break;

    default:
      pRc->cullMask = 0x00000000;
      pRc->sst.sSetupMode |= SST_SETUP_EN_CULLING | SST_SETUP_CULL_NEGATIVE;
      pRc->strip_mode=0x1000bc53 | BIT(23) | BIT(24);
      break;
  }

#if( DX >= 6 )
   _asm_data.cull_mode = pRc->strip_mode;
   _asm_data.cull_mask=pRc->cullMask ^ 0x80000000;
#endif
   RENDERFXN_OK;
}


/*-------------------------------------------------------------------
Function Name:  ditherEnable
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall ditherEnable(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)

  D3DPRINT( RSTATE_DBG_LVL, "ditherEnable Renderstate %d", state );

  // Nothing has changed
  if (state == pRc->ditherEnable)
    RENDERFXN_OK;

  pRc->ditherEnable = state;

  pRc->sst.fbzMode |= SST_DITHER2x2;

  // D3D won't let you chose the dither matrix size so hardcode something
  if (state)
  {
    // turn on dithering
    pRc->sst.fbzMode |=  (SST_ENDITHER);

    // srogers 4/28/99
    // Alpha Dither Subtraction on or off using registry key
    // called SSTH3_ALPHADITHERMODE.  Using 2nd two bits of the overlayFilter register
    // to hold the alpha dither mode value.  For an explanation
    // of the two bits, look at inc/regkeys.h for the define of ALPHADITHERMODE
    // The registry is read in the file dd32/ddinit.c in the function fxinit()
	// srogers 6/10/99 moved this code to here so that Alpha dither subtraction is only
	// turned on when dithering in on.
    switch((_DD(overlayFilter)>>2)&0x3)
    {
      default:
	  case 0:
	  case 1:
	  case 2:
		pRc->sst.fbzMode &= ~(SST_ENDITHERSUBTRACT);
		break;
	  case 3:
		pRc->sst.fbzMode |= SST_ENDITHERSUBTRACT;
		break;
    }
  }
  else
    pRc->sst.fbzMode &= ~(SST_ENDITHER | SST_ENDITHERSUBTRACT);

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  shadeMode
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall shadeMode(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "shadeMode Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DSHADE_FLAT, D3DSHADE_PHONG);

  pRc->shadeMode = state;

  if(state == D3DSHADE_FLAT)
    pRc->state = (pRc->state & ~STATE_REQUIRES_IT_RGB) | STATE_REQUIRES_RGB;
  else
    pRc->state = (pRc->state & ~STATE_REQUIRES_RGB) | STATE_REQUIRES_IT_RGB;

  UPDATE_HW_STATE(SC_TRIANGLE_FLAVOUR);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  fillMode
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall fillMode(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "fillMode Renderstate %d", state );

  CHECK_STATE_LIMITS(state, D3DFILL_POINT, D3DFILL_SOLID);

  // nothing has changed
  if (state == pRc->fillMode)
    RENDERFXN_OK;

  pRc->fillMode = state ;

  pRc->state &=~STATE_NOT_SOLID_FILL;
  if (pRc->fillMode != D3DFILL_SOLID)
     pRc->state|=STATE_NOT_SOLID_FILL;

  UPDATE_HW_STATE(SC_TRIANGLE_FLAVOUR);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  specular
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall specular(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "specular Renderstate %d", state );

  // nothing has changed
  if (state == pRc->specular)
    RENDERFXN_OK;

  pRc->specular = state;


#ifdef TnL_HAL
  pRc->tl.dwTLState |= (state & 0x1) << TLPV_DOSPECULAR_SHIFT;
#endif  //TnL_HAL

  if (state)
  {
    pRc->state |= STATE_REQUIRES_SPECULAR;
  }
  else
  {
    pRc->state &= ~STATE_REQUIRES_SPECULAR;
  }

  UPDATE_HW_STATE(SC_TRIANGLE_FLAVOUR);
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  subPixel
Description:    Updates the D3DRENDERSTATE.
Information:    (RC *pRc, ULONG state)
Return:         VOID

-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall subPixel(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "subPixel Renderstate %d", state );

  // Nothing has changed
  if (state == pRc->subPixel)
    RENDERFXN_OK;

  pRc->subPixel = state;

  if (pRc->subPixel == TRUE)
    pRc->sst.fbzColorPath |=  SST_PARMADJUST ;
  else
    pRc->sst.fbzColorPath &=  ~SST_PARMADJUST ;

  UPDATE_HW_STATE(SC_SOMETHING);
  RENDERFXN_OK;
}

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
/*-------------------------------------------------------------------
Function Name:  sceneCapture
Description:    This state passes TRUE or FALSE to replace the
                functionality of D3DHALCallbacks->SceneCapture(),
Information:    (RC *pRc, ULONG state)
Return:         RENDERFXN_OK
-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall sceneCapture(RC *pRc, ULONG state)
{
  SETUP_PPDEV(pRc)

  D3DPRINT(RSTATE_DBG_LVL, "sceneCapture Renderstate %d", state);

  if (FALSE == state)
  {
    // Processing an EndScene ...
    D3DPRINT(RSTATE_DBG_LVL, "  EndScene");
    _D3(flags) &= ~IN_RENDER_SCENE;
  }
  else
  {
    // Processing a BeginScene ...
    D3DPRINT(RSTATE_DBG_LVL, "  BeginScene");
    _D3(flags) |= IN_RENDER_SCENE;
  }

  RENDERFXN_OK;
}
#endif

/*-------------------------------------------------------------------
Function Name:  initNewRC
Description:    This is the default setting for the rendering context.
Information:    (RC *pRc )
Return:         VOID

-------------------------------------------------------------------*/
void __stdcall initNewRC(RC *pRc)
{
  SETUP_PPDEV(pRc)

  D3DPRINT( DLSTENCIL, "initNewRC: stencil data initialized to zero");

  //Each individual state above is also set in the shadow
  //SST-1 registers below. So don't change something above
  //without changing the corresponding thing below.
  pRc->shadeMode          = D3DSHADE_GOURAUD;
  pRc->zWriteEnable       = FALSE;
  pRc->zEnable            = FALSE;
  pRc->zFunc              = D3DCMP_NEVER;
  pRc->zBias              = 0;
  pRc->zScale             = 65536.0f;
  pRc->texture            = 0;
  pRc->textureAddress     = D3DTADDRESS_WRAP;
  pRc->textureAddressU    = D3DTADDRESS_WRAP;
  pRc->textureAddressV    = D3DTADDRESS_WRAP;
  pRc->texMag             = D3DFILTER_NEAREST;
  pRc->texMin             = D3DFILTER_NEAREST;
  pRc->texturePerspective = FALSE;
  pRc->blendEnable        = FALSE;
  pRc->colorKeyEnable     = TRUE;
  pRc->alphaBlendEnable   = FALSE;
  pRc->alphaTestEnable    = FALSE;
  pRc->srcBlend           = D3DBLEND_ONE;
  pRc->dstBlend           = D3DBLEND_ZERO;
  pRc->alphaRef           = 0;
  pRc->alphaFunc          = D3DCMP_NEVER;
  pRc->fogEnable          = FALSE;
  pRc->fogColor           = 0;
  pRc->cullMode           = D3DCULL_NONE;
  pRc->cullMask           = 0xFFFFFFFF;       // Cull Mask (CCW)
  pRc->ditherEnable       = FALSE;
  pRc->wrapU              = FALSE;
  pRc->wrapV              = FALSE;
  pRc->texMapBlend        = D3DTBLEND_MODULATE;
  pRc->subPixel           = TRUE;
  pRc->specular           = FALSE;
  pRc->fogTableMode       = D3DFOG_NONE;
  pRc->fogDensity         = 1.0f;
  pRc->fogTableStart      = 0.0f;
  pRc->fogTableEnd        = 1.0f;
  pRc->ditherEnable       = FALSE;
  pRc->antialias          = D3DANTIALIAS_NONE;
  pRc->stencilEnable      = 0;
  pRc->stencilFail        = 0;
  pRc->stencilZFail       = 0;
  pRc->stencilPass        = 0;
  pRc->stencilRef         = 0;
  pRc->stencilFunc        = 0;
  pRc->stencilMask        = 0;
  pRc->stencilWriteMask   = 0;

  pRc->vertexColorType    = RX_VERTEX_COLOR_RGB; // set when texture handle set

  // By default, do not zero the jitter values.
  // This allows the zeroing of the jitter values on a per primitive call basis.
  pRc->dwZeroJitter       = 0;

#if ( DX == 5 )
  pRc->state              = (STATE_REQUIRES_IT_RGB | STATE_REQUIRES_VERTS_AREA);
  pRc->sst.sSetupMode     = SST_SETUP_RGB;
#else
  pRc->state              = STATE_REQUIRES_IT_RGB | STATE_REQUIRES_IT_ALPHA | STATE_REQUIRES_VERTS_AREA;
  pRc->sst.sSetupMode     = SST_SETUP_RGB | SST_SETUP_A;
#endif

  // default write buffer is the front buffer
  // enable the clip rectangle on Voodoo so if any software does
  // not clip properly then the FIFO won't be overwritten
  pRc->sst.fbzMode        = SST_RGBWRMASK | SST_ENRECTCLIP | SST_ENZBIAS; // | SST_DEPTH_FLOAT_SEL | SST_WBUFFER;

  // Turning Dither 2x2 bit on always
  pRc->sst.fbzMode |= SST_DITHER2x2;

#ifdef Z_ACCESS_OPT
  // Lets clear the dwCachedFBZMode on a new RC
  if ( _DD(ddEnableZClearOpt) )
  {
    pRc->dwCachedFBZMode    = 0;
	pRc->dwZClearOptEnabled = 0;
  }
#endif

  // default alpha function and blending is disabled
  // default alpha src blend = blend zero
  // default alpha dst blend = blend never (which is zero)
  pRc->sst.alphaMode      = (SST_A_ONE << SST_RGBSRCFACT_SHIFT) ;
  pRc->sst.fogMode        = 0 ;

  if (IS_NAPALM)
  {
    // Default rendering mode is 16bpp, RGBA channels enabled.
    pRc->sst.renderMode     = SST_RM_EN16BPP_RGBA;
    pRc->sst.stencilMode    = 0;
    pRc->sst.stencilOp      = 0;

#ifdef SLI_AA
    // Capture the chipMask register load value.
    switch (_FF(dwNumUnits))
    {
      case 1:
        pRc->sst.chipMask = 0x1;
        break;
      case 2:
        pRc->sst.chipMask = 0x3;
        break;
      case 4:
        pRc->sst.chipMask = 0xF;
        break;
      default:
        D3DPRINT( 0,"initNewRC: _FF(dwNumUnits) (%d) != 1,2 or 4. Defaulting to 4.", _FF(dwNumUnits));
        pRc->sst.chipMask = 0xF;
        break;
    }

    pRc->sst.aaCtrl = 0;
#endif

    if ( _D3(GuardbandClipping) )  //cws gb
    {
      pRc->sst.renderMode |= SST_RM_ENGUARDBAND;
      pRc->sst.clipLeftRight1 = 0x0000ffe; //Default to largest inner guardband (clips less area)
      pRc->sst.clipBottomTop1 = 0x0000fff;
    }
  } // IS_NAPALM

  // Default value for Zbias is 0 (Zbias range is 0-15)
  pRc->sst.zaColor        = 0 ;

  // default  texMag & texMin       = point sampled
  //          chroma (transparency) = disabled
  //          chroma color          = 0
  //          blend                 = tex_map_decal = (1-Atex)Cpix + (Atex)(Ctex)
  //          perspective           = false
  //
  //          When we do texture mapping we need the texture and alpha to
  //          come from the local texture memory
  //pRc->sst.textureMode    = GET_TEXTURE_FROM_LOCAL_TEXTURE_MEMORY ;

  // default  solid/flat/gouraud shading
  //          sub-pixel correction is on.
  pRc->sst.fbzColorPath   = SST_LOCALSEL_RGBA | SST_RGBSEL_RGBA | SST_PARMADJUST;

  if( _D3( pixelCenter ) )
    pRc->sst.pixelOffset = 0.5f;
  else
    pRc->sst.pixelOffset = 0.0f;

#if( DX >= 6 )
  _asm_data.pixel_center=pRc->sst.pixelOffset;
// STB-SK 01/30/99 KNI changes
#if (STBKNI == 1)
  _asm_data.kni_array[PIXEL_C2] = _asm_data.pixel_center;
  _asm_data.kni_array[PIXEL_C3] = _asm_data.pixel_center;
#endif

  // This really sucks ... although we tell the DX6 runtime that we support 3 texture
  // stages, the runtime will only initialise the number of texture units.  This means
  // we have to default all texture stages to Color and Alpha Operation DISABLED.
  {
    int i;

    for( i = 0; i < D3DHAL_TSS_MAXSTAGES; i++ )
    {
      TS[i].textureHandle = 0;
      TS[i].colorOp = D3DTOP_DISABLE;
      TS[i].alphaOp = D3DTOP_DISABLE;
    }
  }

  // Initialise texture stages
  pRc->textureStage[0].changed = 0;
  pRc->textureStage[1].changed = 0;

  // Initialise texture wrapping
  pRc->textureStage[0].wrap = 0;
  pRc->textureStage[1].wrap = 0;

  // Initialise W parameters just in case we don't get any w info
#ifdef DCT_FIX
// RAJ 8/2/99 Fix for WHQL DCT perspective correction test
  pRc->scaleW = 1.f;

#else
  pRc->aW = 1.f;
  pRc->bW = 0.f;
#endif

  pRc->textureFactor = 0x80FFFFFF;

  // Initial flags for special multi-texture modes.
  pRc->specialModes = 0;

  _asm_data.vertex_type=FVFOT_TLVERTEX;
  _asm_data.vertex_size=32;
  _asm_data.t0_offset=24;
  _asm_data.scale_z=pRc->zScale;
//STB-SK 01/30/99 KNI changes
#if (STBKNI == 1)
  _asm_data.kni_array[SCALEZ_2] = pRc->zScale;
#endif
#endif

  /* set bad cpu flag if appropriate */
  /* _cpu_type is set in d3init.c */
  pRc->state|=_cpu_type;


  // different triangle routines to handle different attributes
  pRc->drawIndex = 0;

  pRc->drawGlobal = 1;

  UPDATE_HW_STATE(SC_SOMETHING|SC_TRIANGLE_FLAVOUR);

  _D3(lastContext) = 0;
}

#ifndef WINNT
/*-------------------------------------------------------------------
Function Name:  ddiGetState
Description:    Application is inquiring the current state of some attribute
Information:    (LPD3DHAL_GETSTATEDATA pgsd)
Return:         pgsd->ddrval = D3DHAL_CONTEXT_BAD - if the context is invalid
                pgsd->ddrval = DD_OK - if the context is valid
                DWORD DDHAL_DRIVER_HANDLED
-------------------------------------------------------------------*/
DWORD __stdcall ddiGetState(LPD3DHAL_GETSTATEDATA pgsd)
{
  SETUP_PPDEV(pgsd->dwhContext)
  RC  *pRc ;

  D3D_ENTRY( "ddiGetState" );
  INS_ENTRY( INSC_D3DGETSTATE );

  /*
   * NOTES:
   *
   * This callback is called when Direct3D requires information about
   * the state of a particular stage in the pipeline. If you only handle
   * rasterisation then you only need to respond to D3DHALSTATE_GET_RENDER
   * calls.
   * The state wanted is in pgsd->ddState.renderStateType.
   * Return the answer in pgsd->ddState.ulArg[0].
   */
  if (CONTEXT_VALIDATE(pgsd->dwhContext))
  {
    D3DPRINT( 255,"GetState, bad context =0x08lx", pgsd->dwhContext );
    pgsd->ddrval = D3DHAL_CONTEXT_BAD;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }

  pRc = CONTEXT_PTR(pgsd->dwhContext);
  D3DPRINT( 255,"GetState, pgsd->dwhContext =%08lx, pgsd->dwWhich =%08lx",
                        pgsd->dwhContext, pgsd->dwWhich );

  if (pgsd->dwWhich != D3DHALSTATE_GET_RENDER)
  {
    // You must be able to do transform/lighting
  }
  else
  {
  #ifdef D3TRACE
    printRenderState(pgsd->ddState.drstRenderStateType,0) ;
    D3DPRINT( 255, "\n" );
  #endif

    // what should I fill in if I don't support one? MIRIAM ???
    switch(pgsd->ddState.drstRenderStateType)
    {
      case D3DRENDERSTATE_TEXTUREHANDLE :
        pgsd->ddState.dwArg[0] = pRc->texture;
        break;

      case D3DRENDERSTATE_ANTIALIAS :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_TEXTUREPERSPECTIVE :
        pgsd->ddState.dwArg[0] = pRc->texturePerspective;
        break;

      case D3DRENDERSTATE_WRAPU :
        pgsd->ddState.dwArg[0] = pRc->wrapU ;
        break;

      case D3DRENDERSTATE_WRAPV :
        pgsd->ddState.dwArg[0] = pRc->wrapV ;
        break;

      case D3DRENDERSTATE_ZENABLE :
        pgsd->ddState.dwArg[0] = pRc->zEnable ;
        break;

      case D3DRENDERSTATE_FILLMODE :
        pgsd->ddState.dwArg[0] = pRc->fillMode ;
        break;

      case D3DRENDERSTATE_SHADEMODE :
        pgsd->ddState.dwArg[0] = pRc->shadeMode ;
        break;

      case D3DRENDERSTATE_LINEPATTERN :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_ROP2 :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_PLANEMASK :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_ZWRITEENABLE :
        pgsd->ddState.dwArg[0] = pRc->zWriteEnable ;
        break;

      case D3DRENDERSTATE_ALPHATESTENABLE :
        pgsd->ddState.dwArg[0] = pRc->alphaTestEnable ;
        break;

      case D3DRENDERSTATE_LASTPIXEL :
        // Not supported
        pgsd->ddState.dwArg[0] = 0;
        break;

      case D3DRENDERSTATE_TEXTUREMAG :
        pgsd->ddState.dwArg[0] = pRc->texMag ;
        break;

      case D3DRENDERSTATE_TEXTUREMIN :
        pgsd->ddState.dwArg[0] = pRc->texMin ;
        break;

      case D3DRENDERSTATE_SRCBLEND :
        pgsd->ddState.dwArg[0] = pRc->srcBlend ;
        break;

      case D3DRENDERSTATE_DESTBLEND :
        pgsd->ddState.dwArg[0] = pRc->dstBlend ;
        break;

      case D3DRENDERSTATE_TEXTUREMAPBLEND :
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(REMOVE_OBSOLETE1_FOR_DX7_WIN9X) && !defined(WINNT)
        pgsd->ddState.dwArg[0] = 0 ; //Seems like we should return DDERR_INVALIDPARAMS but I'll be consistant with our past drivers. cws 
#else
        pgsd->ddState.dwArg[0] = pRc->texMapBlend ;
#endif
        break;

      case D3DRENDERSTATE_CULLMODE :
        pgsd->ddState.dwArg[0] = pRc->cullMode ;
        break;

      case D3DRENDERSTATE_ZFUNC :
        pgsd->ddState.dwArg[0] = pRc->zFunc ;
        break;

      case D3DRENDERSTATE_ALPHAREF :
        pgsd->ddState.dwArg[0] = pRc->alphaRef ;
        break;

      case D3DRENDERSTATE_ALPHAFUNC :
        pgsd->ddState.dwArg[0] = pRc->alphaFunc ;
        break;

      case D3DRENDERSTATE_DITHERENABLE :
        pgsd->ddState.dwArg[0] = pRc->ditherEnable ;
        break;

      case D3DRENDERSTATE_BLENDENABLE :
        pgsd->ddState.dwArg[0] = pRc->blendEnable ;
        break;

      case D3DRENDERSTATE_FOGENABLE :
        pgsd->ddState.dwArg[0] = pRc->fogEnable ;
        break;

      case D3DRENDERSTATE_SPECULARENABLE :
        pgsd->ddState.dwArg[0] = pRc->specular ;
        break;

      case D3DRENDERSTATE_ZVISIBLE :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_SUBPIXEL :
        pgsd->ddState.dwArg[0] = pRc->subPixel ;
        break;

      case D3DRENDERSTATE_SUBPIXELX :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_STIPPLEDALPHA :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_FOGCOLOR :
        pgsd->ddState.dwArg[0] = pRc->fogColor ;
        break;

      case D3DRENDERSTATE_FOGTABLEMODE :
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_FOGTABLESTART :
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_FOGTABLEEND :
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_FOGTABLEDENSITY :
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_STIPPLEENABLE :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_EDGEANTIALIAS :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_COLORKEYENABLE :
        pgsd->ddState.dwArg[0] = pRc->colorKeyEnable ;
        break;

      case D3DRENDERSTATE_BORDERCOLOR :
        // Not supported
        pgsd->ddState.dwArg[0] = 0 ;
        break;

      case D3DRENDERSTATE_TEXTUREADDRESS :
        pgsd->ddState.dwArg[0] = pRc->textureAddress;
        break;

      case D3DRENDERSTATE_TEXTUREADDRESSU :
        pgsd->ddState.dwArg[0] = pRc->textureAddressU ;
        break;

      case D3DRENDERSTATE_TEXTUREADDRESSV :
        pgsd->ddState.dwArg[0] = pRc->textureAddressV ;
        break;

      default :
        // Unknown Render state
        pgsd->ddState.dwArg[0] = 0 ;
        break;
    } // switch
  }

#ifdef D3TRACE
  printRenderState(pgsd->ddState.drstRenderStateType, pgsd->ddState.dwArg[0]) ;
  D3DPRINT( 10, "\n" ) ;
#endif

  pgsd->ddrval = DD_OK;
  INS_EXIT( );
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}
#endif


