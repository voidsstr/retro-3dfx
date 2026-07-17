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
** File name: D3global.h
**
** Description: Defines global data structions and triangle rendering functions.
**
** $Revision: 59$
** $Date: 10/27/00 3:00:26 AM$
**
**
** $Log: 
**  59   3dfx      1.38.2.3.1.1510/27/00 Johnny Trainor  Added new comments.
**       Removed obselete comments.
**  58   3dfx      1.38.2.3.1.1410/26/00 Johnny Trainor  Added some new
**       TextureStageStates. Added new elements to our global RC structure.
**  57   3dfx      1.38.2.3.1.1310/23/00 Johnny Trainor  Added extra elements to
**       our TXTRHNDL structure. These are used to store all surface information
**       during creation. Added #defines for TEXTURELOAD() TEXTURESURFACECREATE(),
**       textureSurfaceCreate(), TEXTURESURFACEDELETE() and textureSurfaceDelete().
**       Added prototypes for TEXTURELOAD(), TEXTURESURFACECREATE(), and
**       TEXTURESURFACEDELETE().
**  56   3dfx      1.38.2.3.1.1210/11/00 Brent           Forced check in to enforce
**       branching.
**  55   3dfx      1.38.2.3.1.1110/11/00 Johnny Trainor  Recent check ins caused
**       the hard coded offsets in aglobal.inc to become invalid. Moved lpVertices,
**       dwVertexType and the recently added DX8 elments to the end of the global
**       RC structure to fix this problem. 
**  54   3dfx      1.38.2.3.1.1010/10/00 Johnny Trainor  Added lpVertices and
**       dwVertexType to the global RC structure.
**  53   3dfx      1.38.2.3.1.909/29/00 Johnny Trainor  Initialize DX 8 context
**       values. Some minor text formating.
** 
** 
**  52   3dfx      1.38.2.3.1.809/22/00 Johnny Trainor  DX8 Support. Added
**       dwDXInterface element to global structure.
**  51   3dfx      1.38.2.3.1.709/03/00 Allen Hansen    added texXformFlags to
**       TEXTURESTAGESTATE, which changes the hard-coded offsets in the assembly
**       code, this is required for DX7 (because of texgen)
**  50   3dfx      1.38.2.3.1.608/21/00 Allen Hansen    added constants used by
**       WRAP_3DNow macro
**  49   3dfx      1.38.2.3.1.507/24/00 Allen Hansen    Rewrote cpu detection code,
**       cpuType is now defined in cpu.h, all new code should be protected by
**       #ifdef WINNT
**  48   3dfx      1.38.2.3.1.407/06/00 Bob Seitsinger  (1) Added
**       dwMultitextureAndTrilinear, dwTLodBiasT0 and dwTLodBiasT1 to to the RC
**       structure, if LODBIASPERCHIP is defined. (2) Modified ifdef for
**       _D3(LodDither) to include ifdef LODBIASPERCHIP.
** 
**  47   3dfx      1.38.2.3.1.307/05/00 Allen Hansen    Changed "TextureLighting"
**       to "TransformAndLighting"
**  46   3dfx      1.38.2.3.1.207/05/00 Reid Campbell   Added d3Global entry
**       TextureLighting which controls the enabling of T&L
**  45   3dfx      1.38.2.3.1.107/03/00 Bob Seitsinger  (1) Added LodDither to
**       d3global structure, if LODDITHER is defined. (2) Moved
**       MAXPENDINGBUFFERS_TOAVOIDHANG from ddFlip.
** 
**  44   3dfx      1.38.2.3.1.006/30/00 Bob Seitsinger  Added useMaxPendingBuffers
**       and maxPendingBuffers to d3global structure, if MAXPENDINGBUFFERS is
**       defined.
** 
**  43   3dfx      1.38.2.3    06/05/00 Andrew Sobczyk  New code and variables for
**       4 units AA modes
**  42   3dfx      1.38.2.2    05/19/00 Bob Seitsinger  Changed d3global.LodBias
**       from DWORD to long, to allow for negative values.
**  41   3dfx      1.38.2.1    05/16/00 Bob Johnston    Consolodating VERT_BUFF and
**       TnL_HAL defines to just use TnL_HAL
**  40   3dfx      1.38.2.0    05/10/00 Bob Seitsinger  Add prototype and NULL
**       macro for D3DPRINTTEXCOORD.
**  39   3dfx      1.38        04/25/00 Bob Seitsinger  Added dwZeroJitter to RC
**       structure. This allows for per-primitive zeroing of the AA jitter values.
**  38   3dfx      1.37        04/24/00 Matt McClure    Added dwZClearOptEnabled to
**       pRC
**  37   3dfx      1.36        04/20/00 Matt McClure    Added check for ZEnable so
**       we don't cause state changes we don't want
**  36   3dfx      1.35        04/19/00 Matt McClure    Modification to
**       HW_STATE_CHANGED define to not allow the z buffer optimization to run if
**       it is not enabled.
**  35   3dfx      1.34        04/19/00 Matt McClure    Implementation of Z Buffer
**       Access Optimization.  Added code to support setup and reset of
**       optimization based on the amount of flips that have occurred without a Z
**       Buffer Clear.
**  34   3dfx      1.33        04/07/00 Andrew Sobczyk  New Globals for 2-chip
**       2-sample AA
**  33   3dfx      1.32        04/06/00 Christopher Wilcox Removed error code for
**       zbuffer and render targer depth mismatch.
**  32   3dfx      1.31        03/20/00 Bob Seitsinger  Added #define for FLOATTO42
**       and LodBias element to d3Global.
**  31   3dfx      1.30        03/17/00 Bob Seitsinger  Added compile and run-time
**       informational messages for the #define's that are set.
**  30   3dfx      1.29        03/14/00 Russ Lind       add
**       dwRenderTargZBuffMismatchCnt to d3Global structure
**  29   3dfx      1.28        03/12/00 Bob Seitsinger  Change D3DDBGLVL for w98 to
**       8.
**  28   3dfx      1.27        03/11/00 Bob Seitsinger  Debug statements to assist
**       in buffer allocation/deallocation debugging.
**  27   3dfx      1.26        03/01/00 Justin McCartney Moved flag
**       txtStageStateValidated from _D3 global structure into the RC structure.
**  26   3dfx      1.25        03/01/00 Bob Seitsinger  Added DLSTENCIL for Winnt
**       builds.
**  25   3dfx      1.24        03/01/00 Justin McCartney Fix for Unreal Tournament
**       D3D PRS 12099, added flag var txtStageStateValidated to _D3 global data. 
**       Search jmccartney 28/02/00
**  24   3dfx      1.23        02/29/00 Bob Seitsinger  #define DLSTENCIL for
**       debugging.
**  23   3dfx      1.22        02/04/00 Steve Houston   Added new asm triangle
**       routines by Matt McClure and Allen Hanson to W2K driver.
**       Added NEWASMTRI, STBKNI and K6_2 defines to W2K\build\stbperf.inc to
**       enable this code.
** 
**  22   3dfx      1.21        01/24/00 Matt McClure    Added new function headers
**       and expanded asmfifodata to support new 3DNow and KNI Triangle rendering
**       routines.  Ifdef'd NEWASMTRI==1
**  21   3dfx      1.20        01/17/00 Christopher Wilcox Fixed problem where
**       assembly code was using a 16bpp zscale value instead of 32bpp.
** 
**  20   3dfx      1.19        01/11/00 Scott Kephart   Reverted back to source
**       prior to new triangle assembly code
**  19   3dfx      1.18        01/10/00 Matt McClure    Added support for new
**       assembler triangle routines, #ifdef NEWASMTRI==1 surrounds all changes.
**  18   3dfx      1.17        12/16/99 Chris W. Shaw   Define new registers for
**       all makefiles. 
**  17   3dfx      1.16        12/15/99 Russ Lind       mod to include stbperf.inc
**       from the right location for WINNT builds if INCSTBPERF is defined
**  16   3dfx      1.15        12/14/99 Russ Lind       bump D3DDBGLVL &
**       RSTATE_DBG_LVL to 2 for w2k
**  15   3dfx      1.14        12/13/99 Scott Kephart   Big T&L Update:
**       1. Improved T&L profiling code
**       2. Optimizations to scalar transformation and lighting code
**       3. Changes to the vertex buffer code to allow functionality under Windows
**       2000.
**  14   3dfx      1.13        12/10/99 Bob Seitsinger  Move the RENDERFXN function
**       pointer typedef to below the RC structure definition. This resolves DX6
**       build problems.
**  13   3dfx      1.12        12/09/99 Chris W. Shaw   Changed clipping to use
**       viewport bounds instead of rendertarget bounds for V3 and Napalm.
**  12   3dfx      1.11        12/03/99 Christopher Wilcox Updated surface access
**       method for DirectX 7.
** 
**  11   3dfx      1.10        12/01/99 Bob Seitsinger  Changes to apply default
**       jitter values (as defined by Gary T.) appropriately.
**  10   3dfx      1.9         10/27/99 Bob Seitsinger  Additional rotated dither
**       matrix changes.
**  9    3dfx      1.8         10/26/99 Scott Kephart   Merged in preliminary
**       Software T&L HAL source
**  8    3dfx      1.7         10/20/99 Bob Seitsinger  Rotated dither matrix
**       selection fogMode register values clean up.
**  7    3dfx      1.6         10/18/99 Bob Seitsinger  PRS 8373 fix -
**       conditionally modify the source and/or destination alpha blend mode.
**  6    3dfx      1.5         10/01/99 Christopher Wilcox Remove P6FENCE macro
**       definition, not required by Direct3D code.
** 
**  5    3dfx      1.4         09/30/99 Christopher Wilcox Fixes for DirectX 7
**       multi-buffering problem (PRS #8782)
** 
**  4    3dfx      1.3         09/17/99 Tim Little      Initial changes for support
**       for FXT1 textures, nothing done to handle > 8:1 aspect textures currently.
**  3    3dfx      1.2         09/16/99 Bob Seitsinger  Dither matrix rotate
**       selection changes. D3Global\DitherMatrix* name changes and some new
**       #defines.
**  2    3dfx      1.1         09/16/99 Christopher Wilcox Fixed link errors in
**       DirectX 7 build.
** 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 37    9/07/99 2:33p Bseitsin
** Resolve lingering _renderFuncs problem for w9x - set return value for
** 'dummy' function to RENDERFXN_RETVAL instead of void.
** 
** 36    9/07/99 12:02p Bseitsin
** Add chipMask to sst structure.
** 
** 35    9/07/99 10:49a Bseitsin
** Stencil buffer clear fix.
** 
** 34    9/02/99 11:14a Cwilcox
** Added 24bpp zbuffer support to Direct3D.
** 
** 33    8/23/99 9:29a Msmith
** Added a lookup table to do 8 bit integer to float conversions.
** Speeds up 3dmark test.
** 
** 32    8/20/99 4:01p Bseitsin
** Moved AA_NOT_ENABLED and AA_ENABLED from d3aa.h to here.
** 
** 31    8/20/99 12:56p Russ
** in HL_LINKED_LIST decls, ifdef out pHndlList from d3global for W2K
**
** 30    8/17/99 9:52a Tlittle
** Undid last checkin, it was un-needed.
**
** 29    8/17/99 9:48a Tlittle
** Made the temporary buffer even larger :( as before it didn't have room
** for the mip-maps.  This will change for the better real soon.
**
** 28    8/13/99 4:33p Msmith
** These should be the final changes for the DCT perspective fix from Bob
** J.
**
** 27    8/12/99 1:33p Cwilcox
** Changed FLOATTO34 macro rounding method.
**
** 26    8/10/99 10:37a Russ
** add _TXTRHNDL to TXTRHNDL typedef
** for DX7, removed ddsCaps from TXTRHNDL struct
**
** 25    8/05/99 4:14p Russ
** for RC_LINKED_LIST implementation, added pNext to RC struct
** for DX7 HL_LINKED_LIST implementation, removed numHndlLists from
** d3global
** for DX7, added ddsCaps to TXTRHNDL
**
** 24    8/03/99 2:05p Msmith
** Perspective correct DCT fixes from Bob J.
**
** 23    7/28/99 11:59p Bseitsin
** Changes to enable DX7 for W9x.
**
** 22    7/28/99 9:05a Cshaw
** Added SW workaround for HW "feature": 2ppc to 1ppc transition requires
** writting 12 NOPs.
**
** 21    7/27/99 4:01p Tlittle
** Large textures with > 8:1 aspect ratios
**
** 20    7/27/99 3:53p Bseitsin
** Antialiasing changes.
**
** 19    7/23/99 11:45a Russ
** for DX7, change MAX_NUM_RSTATES to 152
**
** 18    7/19/99 3:10p Russ
** for W2K csim builds replaced HAL_CSIM with CSIM around FLAGS_BAD_???
** defines
**
** 17    7/16/99 6:14p Bseitsin
** Changes to capture new registry key values.
**
** 16    7/13/99 3:38p Cshaw
** Added guardband for Napalm.
**
** 15    7/13/99 2:52p Tlittle
** Additional defines.
**
** 14    7/12/99 1:07p Russ
** for DX7, add state block elements to RC struct
** bump MAX_NUM_RSTATES to 158 for DX7, leave it at 135 for DX6
**
** 13    7/12/99 11:46a Bseitsin
** Remove unnecessary CVG code.
**
** 12    7/08/99 10:18a Tlittle
** Run-Time checking for H5 and BIG_TEXTURES
**
** 11    6/30/99 10:59a Cshaw
** Enabled and added some support code for 2 pixel per clock mode.
**
** 10    6/23/99 9:07p Russ
** DX7 palettized texture changes (#ifdef'd for DX >= 7)
** added dwPaletteHandle to TXTRDESC struct
** replaced surfInterface with dwPaletteHandle in TXTRHNDL struct
** modified function prototypes for TXTRDOWNLOADPALETTE, TXTRNEWPALETTE &
** TXTRNEWPALETTE
**
** 9     6/21/99 5:21p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 8     6/21/99 2:33p Tlittle
** More preliminary changes for run-time h4-h5 support.
**
** 7     6/16/99 10:48a Russ
** added _RC to typedef of RC struct
** added NT versions of D3D_ENTRY and D3D_EXIT
**
** 6     6/14/99 3:17p Bseitsin
** Triangle Iterator Column Band Control support.
**
** 5     6/11/99 12:38p Russ
** initial changes for DX7
**
** 4     6/04/99 7:14p Bseitsin
** 24bit Z/8bit stencil mods.
**
** 3     6/03/99 8:41a Tlittle
** Minor change to H5 checking, added BIG_TEXTURES build flag checking.
** It is not ever defined yet however.
**
** 2     6/02/99 4:30p Edwin
** Clean up "if defined(MM) || defined(WINNT) && defined(_WIN32_WINNT >=
** 0x0500)", multi-monitor support is always on. Note that this file is
** not used by NT4.
**
** 1     6/02/99 6:40a Michael
** Branch from H3
**
** 119   6/01/99 9:44a Russ
** wrapped include of precomp.h in #ifndef WINNT
**
** 118   5/30/99 5:47p Edwin
** Remove ifdef MM, multi-monitor support is always enabled.
**
** 117   5/24/99 5:27p Tlittle
** Modified constants for DXTn textures.
**
** 116   5/24/99 5:04p Bseitsin
** Removal of Antialiasing code.
**
** 115   5/17/99 2:59p Tlittle
** Disabled > 256 textures and modified CREATETEXTURE to use bitsPerTexel
** rather than bytesPerTexel since DXT1 is a 4bit per texel format.
**
** 114   5/14/99 4:50p Stb_gbullard
** Updated with contractor (NVH, Intelligraphics) fixes
**
** 113   5/12/99 9:33a Tlittle
** Added a macro to test a texture surface to see if it is DXTn
** compressed.
**
** 112   5/12/99 9:26a Tlittle
** Addes DXTn texture exports for H5
**
** 111   4/29/99 3:56p Stb_bseitsin
** 32bit rendering and 32bit texture changes.
**
** 109   4/26/99 1:49p Cshaw
** combineMode mods.
**
** 108   4/26/99 11:36a Stb_bseitsin
** 32bit rendering mods. New SETSURFACEPIXELDEPTH macro.
**
** 107   4/26/99 10:21a Russ
** added NT version of GETSURFBPP
**
** 106   4/26/99 10:46a Cshaw
** Added combineMode shadows for FBI and TMUs.
**
** 105   4/23/99 2:06p Stb_bseitsin
** 32-bit rendering mods
**
** 104   4/22/99 3:12p Cwilcox
** Removed declaration and all references to fbiMemorySize.
**
** 103   4/13/99 9:05a Russ
** modify DumpCmdFifoEntries define for NT direct write builds
**
** 102   4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
**
** 101   3/27/99 2:05p Russ
** additional debug output
**
** 100   3/26/99 4:10p Cwilcox
** Major modification to memory allocation and management, fix PRS #4229.
**
** 99    3/18/99 12:44p Stb_bburton
** Added deferred texture download support.
**
** 98    3/17/99 2:41p Andrew
** removed swaphang workaround by reverting to old version
**
** 96    2/22/99 8:59a Russ
** removed NT_D3DPRINT, replaced usage of NT_D3DPRINT with D3DPRINT
**
** 95    2/18/99 8:22a Russ
** enable D3DPRINT on NT5 debug builds, added D3DDBGLVL, RSTATE_DBG_LVL
** and NT_D3DPRINT defines, added some other debug output stuff for NT5
**
** 94    2/11/99 8:29a Stb_skephart
**
** 93    2/03/99 9:37p Russ
** for NT5, modify RENDERFXN typedef to return an HRESULT and modify dummy
** prototype to return an HRESULT.  This is needed in order to pass a new
** DCT test that throws garbage data at DP2
**
** 92    2/03/99 3:38p Sreid
** K6/K7 WC enable from contractor
**
** 91    1/29/99 10:48a Cshaw
** Added unified headers.
**
** 90    1/20/99 1:49p Adrians
** Changes to DX6 hardware setup.
**
** 89    1/12/99 8:04a Cshaw
** Added runtime switches to the performance analysis code (use SIce to
** modify).
**
** 88    1/07/99 5:39p Russ
** for NT, if'd out TXTRCHANGEDPALETTE and TXTRNEWPALETTE function
** prototypes
** for NT, modified ASM_SET_RC3, ASM_SET_RC, ASM_SET and ASM_SYNC macros
**
** 87    12/22/98 7:32p Adrians
** Fix for a texture validation that was failing.
**
** 86    12/16/98 4:03p Sreid
** K6 merge - 12-16-98 drop from contractor
**
** 85    12/09/98 7:08p Adrians
** Fix for triple buffering in WinBench99.
**
** 84    12/09/98 6:38p Sreid
** Merged latest K6 optimization code from Metabyte
**
** 83    12/09/98 7:23a Russ
** NT5 D3D changes for Banshee
**
** 82    12/06/98 1:53p Adrians
** Optimisations for WinBench99.
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
** 78    11/24/98 10:04p Andrew
** Moved the position of SETUP_PPDEV so that it was not in the DX=6 only
** ifdef
**
** 77    11/24/98 5:56p Martin
** Disable trilinear mipmapping, and
** Disable unsupported blend modes
**
** 76    11/22/98 8:59p Andrew
** Changes to support multi-monitor
**
** 75    11/13/98 2:52p Miriam
** Support for DX6 triangle flavor & texture flavor tracing. Just compile
** with debug & fp=1 or tp=1.
**
** 74    11/08/98 3:22p Hanson
** Improvements for Winbench 99
**
** 73    11/02/98 4:31p Martin
** Fix Voodoo2 build with DX=6
**
** 72    11/02/98 10:14a Martin
** Fix compile error of DX=6, and new AA fifo macros
**
** 71    10/29/98 2:12p Martin
** Modification of cmdFifo macros to support future changes for
** super-sampling AA.
**
** 70    10/17/98 10:57a Russ
** include d3dnthal.h for NT instead of d3dhal.h
** added NT versions of _D3 and _D3G macros
**
** 69    10/15/98 6:35p Artg
** changed ifdef h3 to account for h4
** ifdef h3  --> if defined(h3) || defined(h4)
**
** 68    10/14/98 7:10p Artg
** added #if to force the C triangle routines for csim build.
** the asm routines lack csim pointer space support.
**
** 67    10/14/98 11:17a Hanson
** Fixed new DrawIndexedPrimitive stuff so it will run on pentium.
**
** 66    10/13/98 5:53p Hanson
** Dx6 optimizations for DrawIndexedPrimitive
**
** 65    10/13/98 3:03p Hanson
** Oops.
**
** 64    10/13/98 1:53p Hanson
** Should fix build so non cmdfifo routines are not run if using DW
**
** 63    10/08/98 1:47a Hanson
** K6 3D cleanup
**
** 62    10/07/98 10:39p Adrians
** Added K6-2 optimisations.
**
** 61    10/07/98 9:05a stit (Metabyte, Inc)
** AMD K6-2 (MMX+3DNOW) optimization
**
** 60    10/06/98 6:24p Hanson
** Fixed texture offset for assembly triangle routines.
**
** 59    10/06/98 1:52p Miriam
** Support for automipmap for Voodoo2.
**
** 58    10/04/98 5:39p Hanson
** Added check and flags setting for non P6's
**
** 57    10/01/98 10:38p Adrians
** Re-coded Fog which fixes a bug when Wbuffering with Vertex Fog.
**
** 56    10/01/98 5:13p Hanson
** Dx6 fixes with SetDrawTriangle
**
** 55    10/01/98 5:05p Hanson
** Dx6 changes mostly affecting triangle flavoring and state flags.
**
** 54    9/26/98 5:32p Hanson
** Dx6 Assembly interface Optimizations
**
** 53    9/26/98 4:06p Hanson
** Dx6 fixes for Billboard and some multitexturing samples.
**
** 52    9/24/98 11:51a Hanson
** Dx6 Optimizations
**
** 51    9/21/98 3:11p Adrians
** Optimisation to Fog HW setup.
** Added ZBIAS state change.
** Fix for multiple context's with different fog color.
** Set lodMin & lodMax to 1x1 when flat shading.
**
** 50    9/13/98 11:18p Adrians
** Add support for TextureFactor renderstate.
**
** 49    9/13/98 12:17p Adrians
** Added a new flag global variable to disable AA when using specular,
** alphablending or chromokeying.
** Iterated Alpha is now always enabled for DX6.
**
** 48    9/12/98 12:56a Adrians
** Clean up renderstate trace code and add new DX6 states.
** Add trace support for DX6 texture stage states.
** General DX6 code tidyup.
**
** 47    9/10/98 8:20p Adrians
** Fix for textures that don't have alpha with legacy apps.
**
** 46    9/02/98 12:32p Adrians
** DX6 multitexture change.
**
** 45    8/28/98 2:33p Martin
** Conditionalize banshee compilation.
**
** 44    8/28/98 12:37p Martin
** Do super-sampling AA when the registry key is set to 2.  Do edge AA if
** registry key == 1 OR we are not running at the 2 magical resolutions of
** 640x480 or 800x600.
**
** No more conditional compilation of AA.
**
** 43    8/25/98 2:08p Martin
** Copy 2 triangle flavors (IZ & IZT), and make them AA specific for super
** sampling.
**
** 42    8/23/98 1:12a Adrians
** Fix for Voodoo2 build after AA supersample checkin.
**
** 40    8/14/98 8:07p Adrians
** Add support for wrapping renderstates.
**
** 1     8/10/98 3:20p Adrians
**
** 39    7/31/98 10:34p Miriam
** D3D & Glide cooperation. Allow each API to set/reset state to indicate
** that  the HW state has been changed.
**
** 38    7/30/98 12:57p Miriam
** Automipmap textures with alpha. new implementation of automipmapping.
**
** 37    7/29/98 3:30p Adrians
** DX6 Changes.
**
** 36    7/24/98 1:37p Hohn
**
** 35    7/22/98 11:53a Adrians
** Added Lock Idle code & preliminary Automipmap code.
**
** 34    7/10/98 4:50p Martin
** check if palettized texture and if palette has changed.
** If so then reload the palette.
**
** 33    6/30/98 5:29p Miriam
** Performance optimizations.
**
** 32    6/03/98 2:25p Adrians
** Fix for Hyperblade.
**
** 31    5/26/98 7:22p Adrians
** Added antialiasing.
**
** 30    5/18/98 1:43p Adrians
** Wbuffering now uses the full wbuffer range.
**
** 29    5/07/98 6:42p Adrians
** Added performance test code.
**
** 2     5/01/98 4:11p Adrians
** Compile options for dx5 and dx6.
** Removed redundent returns.
**
** 1     4/29/98 6:31p Adrians
** Created
**
** 27    4/15/98 2:38p Miriam
** Front/Back buffer needs to be checked every time we render.
**
** 26    3/27/98 5:50p Adrians
** Broadcast palette changes to both TMU's.
**
** 24    3/27/98 1:44p Adrians
** Code tidyup.
** Removed palette and texture clamp compile options.
**
** 23    3/27/98 11:16a Adrians
** Code added for triangle profiling.
**
** 22    3/16/98 1:46p Adrians
** Added dynamic pixel center offset control.
**
** 21    1/28/98 4:22p Adrians
** Added 0.5 to every vertex x,y and half a texel to s,t.
**
** 20    1/21/98 4:04p Miriam
** Changes that allow D3D to be shared between Voodoo2 & Banshee.
 *
 * 19    1/15/98 10:23a Adrians
 * Added ZBIAS support.
 *
 * 18    11/25/97 4:26p Suninn
 * add D3() for H3
 *
 * 17    11/23/97 3:56p Suninn
 * replay _d3Global with _D3 & D3G macros
 *
 * 16    11/21/97 5:29p Adrians
 * Restore 3D registers after 2D has used them.
 * Added context change code for DrawPrimitives.
 *
 * 15    11/11/97 9:26p Adrians
 * Added 3 more triangle flavours (I, IT, C).
 *
 * 14    11/09/97 2:50p Adrians
 * Single pk1's use an increment of 0.
 * Code added to find the triangle flavours used by apps.
 * Optimisation of triangle flavours.
 * Support for strips and fans in execute buffers.
 * Bug fix to wrapU and wrapV modes.
 *
 * 13    10/29/97 4:44p Adrians
 * Change to get Software Triangle Setup code to compile.
 *
 * 12    10/27/97 5:39p Adrians
 * Added packet 3 to DrawPrimitives, Fans, Strips & Tri's.
 * Added packet 3 to lines and points.
 * Added texture clamping to stw (build option tc=1).
 * Fix to DrawPrimitive fog.
 *
 * 11    10/24/97 7:14p Miriam
 * Support for 2 tmu and trilinear. Will see 2x the memory on a 2 tmu
 * system. Trilinear is 1 pass on 2 tmu & lodDither for 1 tmu.
 * Fixed command fifo problem when using save/restore.
 *
 * 10    10/14/97 11:22a Adrians
 * Added Miriam's 4M texture support.  Added build environment for H3.
 *
 * 9     10/07/97 1:41p Adrians
 * Optimisation to myRenderPrimitive setup.
 *
 * 7     9/16/97 5:18p Adrians
 * Added some new cvgdefs (Should really go into cvgdefs.h>
 *
 * 4     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/
//------------------------------------------------------------------------
// Module Name: fxglobal.h
//
// All the global data for this driver is kept here. The global data is
// information that we store on a board level. All this info may be stored
// in the PDEV (physical device control block) later but only because
// Windows 95 supports 1 PDEV. Windows NT supports multiple...
//
//------------------------------------------------------------------------
#ifndef __D3GLOBAL_H__
#define __D3GLOBAL_H__

#ifndef WINNT
#include "precomp.h"
#endif

#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
#include "d3dnthal.h"
#else
#include "d3dhal.h"
#include "hw.h"
#endif

#include "d3mem.h"

#include "ddraw.h"

// include #defines for performance optimizations
#ifdef INCSTBPERF
#ifdef WINNT
#include "..\..\..\..\build\stbperf.inc"
#else
#include "..\build\stbperf.inc"
#endif
#endif

// 10/26/99 -- skephart -- moved these definitions to the top of the file -- the TNL stuff needs them
typedef HRESULT   RENDERFXN_RETVAL;
#define RENDERFXN_OK  return DD_OK

#ifdef TnL_HAL
#include "tlglobal.h"
#endif
// Defines used for performance testing
#define PCI_NOP   1
#define PCI_NULL  2
#define TRI_NULL  3

// Define the maximum number of texture formats
#define MAX_TEXTURE_FORMATS 25

// Define the maximum number of mipmap levels
#define MAX_MIPMAP_LEVELS 9

// Define the number of chips we're currently supporting.
// This defines the size of a number of arrays. E.g. aaCtrl, etc.
#define MAX_NUMBER_CHIPS_SUPPORTED  4

#define FOURCC_FXT1 (MAKEFOURCC('F','X','T','1'))

// Maximum number of pending buffers for Voodoo5. Anything above this causes a hang.
#define MAXPENDINGBUFFERS_TOAVOIDHANG  6

// Define some macros for timing using the Pentium Clock
#if 1 //defined( DEBUG )
// Opcodes to read the Pentium clock
#define rdtsc _asm _emit 0x0f _asm _emit 0x31

// Read the initial clock value
#define START_TIMER( hiTimer, loTimer ) \
_asm { \
       rdtsc \
  _asm mov   loTimer, eax \
  _asm mov   hiTimer, edx \
}

// Calculate the clock difference
#define STOP_TIMER( hiTimer, loTimer ) \
_asm { \
       rdtsc \
  _asm sub   eax, loTimer \
  _asm sbb   edx, hiTimer \
  _asm mov   loTimer, eax \
  _asm mov   hiTimer, edx \
}

#define PRINT_TIMER( hiTimer, loTimer ) \
{ \
  char buffer[128]; \
  sprintf( buffer, "Execution Time - %ld clks\n", hiTimer ? -1 : loTimer ); \
  OutputDebugString( buffer ); \
}

#else // DEBUG
#define START_TIMER( hiTimer, loTimer )
#define STOP_TIMER( hiTimer, loTimer )
#define PRINT_TIMER( hiTimer, loTimer )
#endif // DEBUG

#define CMD_TRI   0
#define CMD_START 1
#define CMD_CONT  2

// Defines for _D3( enableAntialias )
#define AA_NOT_ENABLED            0
#define AA_ENABLED                1

// #ifdef H5
// Retrieve the pixel depth for a surface.
//
// A pixel format structure is not established for surfaces
// with pixel depths the same as the primary surface.
// If the surface has a pixel format structure, then pull the
// pixel depth from it; otherwise, get the pixel depth from the
// primary surface.
//
#ifdef WINNT
#define GETSURFBPP(surfHndl) \
    ( \
           ((TXTRHNDL_PTR(surfHndl)->dwFlags) & DDRAWISURF_HASPIXELFORMAT) \
           ? (TXTRHNDL_PTR(surfHndl)->dwBitCnt) : (ppdev->cBitsPerPel) \
    )
#else
#define GETSURFBPP(surfHndl) \
    ( \
           ((TXTRHNDL_PTR(surfHndl)->dwFlags) & DDRAWISURF_HASPIXELFORMAT) \
           ? (TXTRHNDL_PTR(surfHndl)->dwBitCnt) : (_FF(bpp)) \
    )
#endif

//
// Set the target surface pixel depth.
//
#define SETSURFACEPIXELDEPTH(pRc, surfHndl)              \
    pRc->sst.renderMode &= SST_RM_RESETMODEANDCHANNELS;  \
    switch (GETSURFBPP((surfHndl)))                      \
    {                                                    \
    case 15:                                             \
        pRc->sst.renderMode |= SST_RM_EN15BPP_RGBA;      \
        pRc->sst.fbzMode    &= ~SST_ENALPHABUFFER;       \
        pRc->sst.fbzMode    |= SST_RGBWRMASK;            \
        pRc->zScale = 65536.0f;                          \
        break;                                           \
    case 16:                                             \
        pRc->sst.renderMode |= SST_RM_EN16BPP_RGBA;      \
        pRc->sst.fbzMode    |= SST_RGBWRMASK;            \
        pRc->zScale = 65536.0f;                          \
        break;                                           \
    case 32:                                             \
        pRc->sst.renderMode |= SST_RM_EN32BPP_RGBA;      \
        pRc->sst.fbzMode    &= ~SST_ENALPHABUFFER;       \
        pRc->sst.fbzMode    |= SST_RGBWRMASK;            \
        pRc->zScale = 16777216.0f;                       \
        break;                                           \
    default:                                             \
        pRc->sst.renderMode |= SST_RM_EN16BPP_RGBA;      \
        pRc->sst.fbzMode    |= SST_RGBWRMASK;            \
        pRc->zScale = 65536.0f;                          \
        break;                                           \
    }

// This macro is used when we want to set the render mode only!
#define SETRENDERMODEPIXELDEPTH(pRc, surfHndl)              \
    pRc->sst.renderMode &= SST_RM_RESETMODEANDCHANNELS;     \
    switch (GETSURFBPP((surfHndl)))                         \
    {                                                       \
    case 15:                                                \
        pRc->sst.renderMode |= SST_RM_EN15BPP_RGBA;         \
        break;                                              \
    case 16:                                                \
        pRc->sst.renderMode |= SST_RM_EN16BPP_RGBA;         \
        break;                                              \
    case 32:                                                \
        pRc->sst.renderMode |= SST_RM_EN32BPP_RGBA;         \
        break;                                              \
    default:                                                \
        pRc->sst.renderMode |= SST_RM_EN16BPP_RGBA;         \
        break;                                              \
    }
/// #endif // H5

// Macro to convert a float to a fixed point 3.4 format, with rounding.
#define FLOATTO34(floatValue)   ((unsigned long)((floatValue * 16.0) + 0.5))
// Macro to convert a float to a fixed point 4.2 format, with rounding.
#define FLOATTO42(floatValue)   ((unsigned long)((floatValue * 4.0) + 0.5))

// Check a texture surface to see if it is DXTn compressed
#define TEXTURE_IS_DXT_SURFACE(surface)                  \
    ((surface.dwFlags & DDPF_FOURCC)   &&                \
    ((surface.dwFourCC == FOURCC_DXT1) ||                \
     (surface.dwFourCC == FOURCC_DXT2) ||                \
     (surface.dwFourCC == FOURCC_DXT3) ||                \
     (surface.dwFourCC == FOURCC_DXT4) ||                \
     (surface.dwFourCC == FOURCC_DXT5)))

#define TEXTURE_IS_FXT_SURFACE(surface)                  \
   ((surface.dwFlags & DDPF_FOURCC) &&                   \
   (surface.dwFourCC == FOURCC_FXT1))

// OS dependant macro to get CPU type   
#ifdef WINNT  
#define CPUTYPE ppdev->cpuType
#else
#define CPUTYPE _FF(cpuType)
#endif 
   
// Define some constants for textures.
# define MAX_BIGTEXTURE_SIZE_U     2048
# define MAX_BIGTEXTURE_SIZE_V     2048
# define MAX_BIGTEXTURE_SIZE       MAX_BIGTEXTURE_SIZE_U * MAX_BIGTEXTURE_SIZE_V

# define MAX_BIGTEXTURE_LOD        11
# define MAX_BIGTEXTURE_LOG        11

# define MAX_TEXTURE_SIZE_U        256
# define MAX_TEXTURE_SIZE_V        256
# define MAX_TEXTURE_SIZE          MAX_TEXTURE_SIZE_U * MAX_TEXTURE_SIZE_V

# define MAX_TEXTURE_LOD           8
# define MAX_TEXTURE_LOG           8

// Some missing cvgdefs
#define SST_LOCALSEL_RGBA        (0<<SST_LOCALSELECT_SHIFT)
#define SST_LOCALSEL_C0          (1<<SST_LOCALSELECT_SHIFT)

// this driver currently supports 1 card and 1 trex on each card
//#define MAX_NUM_CARDS       1
#define MAX_NUM_FREE_BLOCKS 10
#define MAX_NUM_FREE_HEAPS  1
#ifndef D3D_MAX_NUM_TMUS
#define D3D_MAX_NUM_TMUS 2
#endif
#define TREX0               0
#define TREX1               1
#define CHIP_FBI    1
#define CHIP_TMU0   2
#define CHIP_TMU1   4
#define CHIP_TMU2   8
//#define CARD0               0
#define BYTES_PER_16BPP     2
#define TMU2CHIP(tmu)       (0x2 << (tmu))

// bit masks for our texture formats
#define RGB565_RMASK   0xf800
#define RGB565_GMASK   0x07e0
#define RGB565_BMASK   0x001f
#define RGB565_AMASK   0x0000

#define RGB4444_AMASK  0xF000
#define RGB4444_RMASK  0x0F00
#define RGB4444_GMASK  0x00F0
#define RGB4444_BMASK  0x000F

#define RGB332_RMASK   0x00e0
#define RGB332_GMASK   0x001c
#define RGB332_BMASK   0x0003
#define RGB332_AMASK   0x0000

#define RGB1555_AMASK  0x8000
#define RGB1555_RMASK  0x7c00
#define RGB1555_GMASK  0x03e0
#define RGB1555_BMASK  0x001f

#define RGB8332_AMASK   0xFF00
#define RGB8332_RMASK   0x00e0
#define RGB8332_GMASK   0x001c
#define RGB8332_BMASK   0x0003

// #ifdef H5
#define RGB8888_AMASK   0xFF000000
#define RGB8888_RMASK   0x00FF0000
#define RGB8888_GMASK   0x0000FF00
#define RGB8888_BMASK   0x000000FF
// #endif // H5

#define A8_AMASK        0xFF
#define L8_LMASK        0xFF

#define LA8_LMASK       0x0F
#define LA8_AMASK       0xF0

#define LA16_LMASK      0x00FF
#define LA16_AMASK      0xFF00

#define ALPHA_P8_AMASK  0xFF00

// sst1 texture formats
#define TEXFMT_RGB_332            0
#define TEXFMT_YIQ_422            1
#define TEXFMT_ALPHA_8            2          /* (0..0xFF) alpha     */
#define TEXFMT_INTENSITY_8        3          /* (0..0xFF) intensity */
#define TEXFMT_ALPHA_INTENSITY_44 4
#define TEXFMT_P8_RGB             5
#define TEXFMT_P8_RGBA            6
#define TEXFMT_RSVD1              7
#define TEXFMT_ARGB_8332          8
#define TEXFMT_AYIQ_8422          9
#define TEXFMT_RGB_565            10
#define TEXFMT_ARGB_1555          11
#define TEXFMT_ARGB_4444          12
#define TEXFMT_ALPHA_INTENSITY_88 13
#define TEXFMT_ALPHA_P8_RGB       14

#define TEXFMT_ARGB_8888          15
#define TEXFMT_FXT1               0
#define TEXFMT_DXT1               1
#define TEXFMT_DXT2               2
#define TEXFMT_DXT3               2
#define TEXFMT_DXT4               3
#define TEXFMT_DXT5               3
#define TEXFMT_RSVD2              15

// #ifdef H5
#define TEXFMT_MAX                16
// #else  // H5
// #define TEXFMT_MAX                15
// #endif // h5

#define TEXFMTFLG_RGB               0x00000001L
#define TEXFMTFLG_ALPHA             0x00000002L
#define TEXFMTFLG_PALETTIZED        0x00000004L
#define TEXFMTFLG_INTENSITY         0x00000008L
#define TEXFMTFLG_PALETTIZED_ALPHA  0x00000010L

// TMU and FBI defaults
#define  GET_TEXTURE_FROM_LOCAL_TEXTURE_MEMORY (SST_TC_REPLACE | SST_TCA_REPLACE)
#define  DEFAULT_FBZCOLORPATH (SST_RGBSEL_RGBA)

#define MAX_STATE       D3DSTATE_OVERRIDE_BIAS
#define DWORD_BITS      32
#define DWORD_SHIFT     5

typedef struct _D3DSAMPStateSet {
    DWORD               bits[MAX_STATE >> DWORD_SHIFT];
} D3DSAMPStateSet;

// DX6 stuff
#if( DX >= 6 )

#define TEXTURESTAGE0   0
#define TEXTURESTAGE1   1

typedef struct TEXTURESTAGESTATE {
    DWORD textureHandle;    // Texture Handle
    DWORD colorOp;          // D3DTSS_COLOROP        =  1, /* D3DTEXTUREOP - per-stage blending controls for color channels */
    DWORD colorArg1;        // D3DTSS_COLORARG1      =  2, /* D3DTA_* (texture arg) */
    DWORD colorArg2;        // D3DTSS_COLORARG2      =  3, /* D3DTA_* (texture arg) */
    DWORD alphaOp;          // D3DTSS_ALPHAOP        =  4, /* D3DTEXTUREOP - per-stage blending controls for alpha channel */
    DWORD alphaArg1;        // D3DTSS_ALPHAARG1      =  5, /* D3DTA_* (texture arg) */
    DWORD alphaArg2;        // D3DTSS_ALPHAARG2      =  6, /* D3DTA_* (texture arg) */
    float bumpEnvMat00;     // D3DTSS_BUMPENVMAT00   =  7, /* D3DVALUE (bump mapping matrix) */
    float bumpEnvMat01;     // D3DTSS_BUMPENVMAT01   =  8, /* D3DVALUE (bump mapping matrix) */
    float bumpEnvMat10;     // D3DTSS_BUMPENVMAT10   =  9, /* D3DVALUE (bump mapping matrix) */
    float bumpEnvMat11;     // D3DTSS_BUMPENVMAT11   = 10, /* D3DVALUE (bump mapping matrix) */
    DWORD texCoordIndex;    // D3DTSS_TEXCOORDINDEX  = 11, /* identifies which set of texture coordinates index this texture */
    DWORD address;          // D3DTSS_ADDRESS        = 12, /* D3DTEXTUREADDRESS for both coordinates */
    DWORD addressU;         // D3DTSS_ADDRESSU       = 13, /* D3DTEXTUREADDRESS for U coordinate */
    DWORD addressV;         // D3DTSS_ADDRESSV       = 14, /* D3DTEXTUREADDRESS for V coordinate */
    DWORD borderColor;      // D3DTSS_BORDERCOLOR    = 15, /* D3DCOLOR */
    DWORD magFilter;        // D3DTSS_MAGFILTER      = 16, /* D3DTEXTUREMAGFILTER filter to use for magnification */
    DWORD minFilter;        // D3DTSS_MINFILTER      = 17, /* D3DTEXTUREMINFILTER filter to use for minification */
    DWORD mipFilter;        // D3DTSS_MIPFILTER      = 18, /* D3DTEXTUREMIPFILTER filter to use between mipmaps during minification */
    DWORD mipmapLodBias;    // D3DTSS_MIPMAPLODBIAS  = 19, /* D3DVALUE Mipmap LOD bias */
    DWORD maxMipLevel;      // D3DTSS_MAXMIPLEVEL    = 20, /* DWORD 0..(n-1) LOD index of largest map to use (0 == largest) */
    DWORD maxAnisotropy;    // D3DTSS_MAXANISOTROPY  = 21, /* DWORD maximum anisotropy */
    DWORD bumpEnvlScale;    // D3DTSS_BUMPENVLSCALE  = 22, /* D3DVALUE scale for bump map luminance */
    DWORD bumpEnvlOffset;   // D3DTSS_BUMPENVLOFFSET = 23, /* D3DVALUE offset for bump map luminance */
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    DWORD texXformFlags;    // D3DTSS_TEXTURETRANSFORMFLAGS = 24, /* DWORD = # of texture coordinates, 256 means projected texture */
#endif
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
    DWORD addressw;         // D3DTSS_ADDRESSW       = 25, /* Volume texture wrapping mode */
    DWORD colorarg0;        // D3DTSS_COLORARG0      = 26, /* third arg for triadic ops */
    DWORD alphaarg0;        // D3DTSS_ALPHAARG0      = 27, /* third arg for triadic ops */
    DWORD resultarg;        // D3DTSS_RESULTARG      = 28, /* arg for result */
#endif
    DWORD wrap;
    DWORD changed;          // We use this to signify a change was made to stage state.
} TEXTURESTAGESTATE, *LPTEXTURESTAGESTATE;

//#define TSS_CHANGED  25	
#define TSS_CHANGED  ((sizeof(TEXTURESTAGESTATE) / sizeof(DWORD))-1)	// un-hard-coded this, sortof

#endif // DX >= 6

//------------------
// Rendering context
//------------------

//--------------------------------------------------------------------------------------
// WARNING!!!!!!!! The assembly rendering code hard codes the some of the offsets of RC
// in aglobal.inc. All new RC structure elements should be added after the sst structure. 
//--------------------------------------------------------------------------------------

typedef struct _RC {
#if RC_LINKED_LIST
  struct _RC        *pNext;     // this must be the first element in struct!
#endif

#ifdef WINNT
  PDEV              *ppdev;     // Pointer to PDEV *
#else
  // Well let's start with our Device Pointer
  LPDDRAWI_DIRECTDRAW_GBL lpDDGbl;  // Direct Draw GlobalData Pointer
  GLOBALDATA        *ppdev;     // Pointer to GLOBALDATA *
#endif

  BOOL              drawGlobal; // Debug Variable

  //----------------------------------------
  // Curent rendering state for this context
  //----------------------------------------
  ULONG             antialias;
  ULONG             shadeMode;
  ULONG             flatShadeReg;
  ULONG             fillMode;
  ULONG             zWriteEnable;
  ULONG             zEnable;
  ULONG             zFunc;
  float             zScale;
  D3DTEXTUREHANDLE  texture;
  ULONG             texMag;
  ULONG             texMin;
  ULONG             texturePerspective;
  ULONG             textureAddress;
  ULONG             textureAddressU;
  ULONG             textureAddressV;
  ULONG             blendEnable;
  ULONG             colorKeyEnable;
  ULONG             alphaBlendEnable;
  ULONG             alphaTestEnable;
  ULONG             srcBlend;
  ULONG             dstBlend;
  ULONG             alphaRef;
  ULONG             alphaFunc;
  ULONG             fogEnable;
  D3DCOLOR          fogColor;
  ULONG             fogTableMode;
  float             fogTableStart;
  float             fogTableEnd;
  float             fogDensity;
  ULONG             cullMode;
  ULONG             cullMask;           // optimization for cullMode
  ULONG             ditherEnable;
  ULONG             wrapU;
  ULONG             wrapV;
  ULONG             texMapBlend;
  ULONG             subPixel;
  ULONG             specular;
  ULONG             zVisible;
  ULONG             zBias;
  ULONG             state;
  ULONG             triSize;
  ULONG             stencilEnable;
  ULONG             stencilFail;
  ULONG             stencilZFail;
  ULONG             stencilPass;
  ULONG             stencilFunc;
  ULONG             stencilRef;
  ULONG             stencilMask;
  ULONG             stencilWriteMask;

  ULONG             hwStateChanged;
// Watch for Z Buffer state changes within drawprimitive command streams
#ifdef Z_ACCESS_OPT
#define HW_STATE_CHANGED              (_DD(ddEnableZClearOpt) && _DD(dd3DInOverlay) && (_FF(dd3DSurfaceCount) > 2) && (_FF(dd3DSurfaceCount) < 5) && (_DD(ddFlipsWithoutZClear) == 0x80000000) && pRc->zEnable) || (pRc->hwStateChanged)
#else
#define HW_STATE_CHANGED              (pRc->hwStateChanged)
#endif

#define UPDATE_HW_STATE(whatChanged)  (pRc->hwStateChanged |= whatChanged)
#define CLEAR_HW_STATE(whatChanged)   (pRc->hwStateChanged &= ~whatChanged)
#define CHECK_HW_STATE(whatChanged)   (pRc->hwStateChanged & whatChanged)
#define RESET_HW_STATE                (pRc->hwStateChanged = 0)
  #define SC_SOMETHING        (1 << 0)
  #define SC_TRIANGLE_FLAVOUR (1 << 1)
  #define SC_NEED_NOP         (1 << 2)
  #define SC_TEXTUREFACTOR    (1 << 3)
  #define SC_ZBIAS            (1 << 4)
  #define SC_FOG              (1 << 5)
  #define SC_TLOD             (1 << 6)
  #define SC_BUFFERS          (1 << 7)
  #define SC_ALPHABLEND       (1 << 8)
  #define SC_STATE            ( SC_SOMETHING | SC_NEED_NOP | SC_TEXTUREFACTOR | SC_ZBIAS | SC_FOG | SC_ALPHABLEND)

  DWORD             fogStateChanged;
#define FOG_STATE_CHANGED             (pRc->fogStateChanged)
#define UPDATE_FOG_STATE(whatChanged) (pRc->fogStateChanged |= whatChanged)
#define CLEAR_FOG_STATE(whatChanged)  (pRc->fogStateChanged &= ~whatChanged)
#define RESET_FOG_STATE               (pRc->fogStateChanged = 0)
#define SC_FOGCOLOR         (1 << 0)
#define SC_FOGTABLE         (1 << 1)
#define SC_FOGALL           (SC_FOGCOLOR | SC_FOGTABLE)

  DWORD             useFogTable;
#define FOGTABLE_ENABLED  SC_FOGTABLE
#define FOGTABLE_DISABLED 0x00000000

  DWORD             useFog;
#define FOG_ENABLED   (SC_FOGCOLOR | SC_FOGTABLE)
#define FOG_DISABLED  0x00000000

#if( DX >= 6 )

  // Used to detect special multi-texturing modes.
#define SPECIAL_MODE1 (1 << 8)
#define SPECIAL_MODE3 (1 << 9)
#define SPECIAL_FAIL  (1 << 10)
  DWORD             specialModes;

  DWORD             textureFactor;

  TEXTURESTAGESTATE textureStage[D3DHAL_TSS_MAXSTAGES];

  DWORD             t0CoordIndex;
  DWORD             t1CoordIndex;

  DWORD             wrapT0;
  DWORD             wrapT1;

#ifdef DCT_FIX
// RAJ 8/2/99 Fix for WHQL DCT perspective correction test
#define WMIN  0.03125f
#define WMAX  (65520.f / (64.f*256.f))
  float             minW, maxW, scaleW;
#else
#define WMIN 1.f
#define WMAX 65520.f
#define WSCALE(a) ((a * pRc->aW) + pRc->bW)
  float             minW, maxW, aW, bW;
#endif // dct_fix

#endif // DX >= 6

  ULONG             vertexColorType;
#define RX_VERTEX_COLOR_RGB     1
#define RX_VERTEX_COLOR_RGBA    2

  // Multiple draw triangle setup to maximize performance
  void (__stdcall *drawTriangle)(VOID *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
  void (__cdecl *DrawMeshAsm)(FxU32 count,FxU32 *idx,FxU32 *verts);
  FxU32 (__cdecl *DrawTriIdx2)(FxU32 count,FxU32 *idx,FxU32 *verts);
  ULONG             drawIndex;

  //
  // 3Dfx SST-1 specific data. The renderstate above is translated into register states
  // that are stored below
  //
  struct {

    // frame buffer organization
    ULONG           bytesPerPixel;          // rgb (not z)

    // SST state
    ULONG           fbzMode;
    ULONG           alphaMode;
    ULONG           fogMode;
    ULONG           fogColor;              // (8) 888-RGB
    ULONG           textureMode;
    ULONG           textureModeT0;           // texture mode specific to TREX0
    ULONG           textureModeT1;           // texture mode specific to TREX1
    ULONG           fbzColorPath;
    ULONG           tLOD;
    ULONG           tLODT0;
    ULONG           tLODT1;
    ULONG           baseAddr;
    ULONG           baseAddr1;
    ULONG           lfbMode;
    float           scaleS, scaleT;
    float           centerS, centerT;
#if ( DX >= 6 )
    float           scaleS1, scaleT1;
    float           centerS1, centerT1;
#endif
    ULONG           sSetupMode;
    ULONG           zaColor;
    ULONG           oldFbzMode;
    ULONG           oldFbzColorPath;
    ULONG           oldTextureMode;

    float           pixelOffset;
    ULONG           combineModeFBI;   // combineMode specific to FBI
    ULONG           combineModeT0;    // combineMode specific to TREX0
    ULONG           combineModeT1;    // combineMode specific to TREX1
    ULONG           renderMode;
    ULONG           stencilMode;
    ULONG           stencilOp;
    ULONG           sliCtrl;
    ULONG           aaCtrl;
    ULONG           clipLeftRight1;//cws gb
    ULONG           clipBottomTop1;//cws gb
    ULONG           clipLeftRight;
    ULONG           clipBottomTop;
#ifdef SLI_AA
    ULONG           chipMask;
#endif // SLI_AA
  } sst;
  FxU32             strip_mode;

  // DDRAW surface associated with this rendering context
#ifndef WINNT
  LPDDRAWI_DDRAWSURFACE_INT interfaceDDS;
#endif

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  DWORD             DDSHndl;            // surface handle (rendering target)
  DWORD             DDSZHndl;           // surface handle (zbuffer)
  HNDLLIST          *pHndlList;
  LPVOID            pDDLcl;

  // state block info
  BOOL              bSBRecMode;         // TRUE if in recording mode, FALSE if not
  STATEBLOCK        *pCurrSB;
  STATEBLOCK        **ppSBTable;
#else
  LPDDRAWI_DIRECTDRAWGBL    lpDDGbl;
  LPDDRAWI_DDRAWSURFACE_INT interfaceDDS;
  LPDDRAWI_DDRAWSURFACE_LCL lpDDS;
  LPDDRAWI_DDRAWSURFACE_LCL lpDDSZ;
#endif

  // Override states in effect
  D3DSAMPStateSet   overrides;
#ifdef TnL_HAL
  TL_RENDERCONTEXT_DATA tl;             // geometry and lighting related data...
#endif  // end of TnL_HAL block

  BOOL              txtStageStateValidated; // has app validated the texture stage state it set-up NOT whether validate has succeeded or not
                                            // jmccartney 28/02/00 part of fix for PRS 12099

  // process id associated with this rendering context
  DWORD             pid;

// Define dwCachedFBZMode for holding the previous FBZMode state before
// Z Access OPT is enabled.
#ifdef Z_ACCESS_OPT  
  DWORD             dwCachedFBZMode;
  DWORD             dwZClearOptEnabled;
#endif

  // Flag to indicate whether to set AA jitter values to zero or not.
  DWORD             dwZeroJitter;

#ifdef LODBIASPERCHIP
  DWORD             dwMultitextureAndTrilinear;
  DWORD             dwTLodBiasT0[4];
  DWORD             dwTLodBiasT1[4];
#endif

  LPDWORD       	lpVertices;
  DWORD         	dwVertexType;

  // Vertices stride use for DX 7 & DX 8 (So drawing code can be shared)
  DWORD             dwVerticesStride;

  // DX 8 Context Entries
#if( DX >= 8 )
  // Direct X Interface for this Context (0 = DX3, 1 = DX5, 2 = DX6, 3 = DX7, 4 = DX8)
  DWORD             dwDXInterface;
  // Direct X 8 Stream Entries for this Context
  LPDWORD           lpIndices;
  DWORD             dwIndicesStride;
  DWORD             dwNumVertices;
  DWORD             dwVBSizeInBytes;
  DWORD             dwVBHandle;
  DWORD             dwIndexHandle;
  DWORD             dwShaderHandle;
#endif // DX >= 8
} RC;

typedef RENDERFXN_RETVAL (__stdcall *RENDERFXN)(RC *, ULONG) ;

// characteristics of texture surface
typedef struct
{

  long              bytesPerTexel;
  long              slog, tlog;
  float             scaleS, scaleT;
  float             centerS, centerT;
  long              aspectRatio;                // 2 ** X of the largest mipmap
  long              flags;
  #define           AspectRatioGT8      0x01    // aspect ratio larger than 8
  #define           InUse               0x02    // this texture handle is allocated
  #define           VideoTexture        0x04
  #define           AutoMipMaps         0x08
  #define           TextureIsBig        0x10
  long              initialSlog, initialTlog;
  long              numMipmaps;                 // number of mipmap levels
  long              format, formatFlags;
  long              start[D3D_MAX_NUM_TMUS];
  long              baseAddr[D3D_MAX_NUM_TMUS];
  long              noMipMapsBaseAddr[D3D_MAX_NUM_TMUS];
  long              tLOD[D3D_MAX_NUM_TMUS];
  long              tLODnoMipMaps[D3D_MAX_NUM_TMUS];
  ULONG             heapID[D3D_MAX_NUM_TMUS];
  ULONG             tmuID[D3D_MAX_NUM_TMUS];
  ULONG             mipmapsOnTmu[D3D_MAX_NUM_TMUS];
  #define           LODMASK_ALL         0x1ff   // LOD 1x1 to 256x256 (9 levels)
  #define           LODMASK_ODD         0xaa    // odd  LOD levels 1,3,5,7
  #define           LODMASK_EVEN        0x155   // even LOD levels 0,2,4,6,8
  #define           TBIG_LODMASK_ALL    0xfff   // LOD 1x1 to 256x256 (9 levels)
  #define           TBIG_LODMASK_ODD    0xaaa   // odd  LOD levels 1,3,5,7
  #define           TBIG_LODMASK_EVEN   0x555   // even LOD levels 0,2,4,6,8
  ULONG             home;                       // texture lives here
  ULONG             noMipMapsHome;              // largest LOD lives here
  #define           HOME_TREX0                  1
  #define           HOME_TREX1                  2
  #define           HOME_BOTH                   3
  #define           HOME_SPLIT_ODD_TMU1         4
  #define           HOME_SPLIT_ODD_TMU0         5
  #define           HOME_BOTH_2PIX_PER_CLOCK    6
  ULONG             textureLength[D3D_MAX_NUM_TMUS];

#ifdef WINNT
  VIDEOMEMORY       *pvmHeap;
#endif

#ifdef DEFER_TXTR_DNLD
    ULONG           checksum[MAX_MIPMAP_LEVELS];// Checksum for each LOD mipmap. Set at txtrload.
    WORD            reload:MAX_MIPMAP_LEVELS;   // T/F if LOD #n (bit n) needs loading
#endif // DEFER_TXTR_DNLD

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    DWORD           dwPaletteHandle;
#endif
} TXTRDESC;

// mipmap data
typedef struct _MIPMAPDATA
{
  WORD              wWidth;                     // mipmap width value
  WORD              wHeight;                    // mipmap height value
  LONG              lPitch;                     // mipmap surface pitch
  unsigned long     fpVidMem;                   // pointer to texture memory
} MIPMAPDATA;

// texture handle
typedef struct _TXTRHNDL
{
  long              flags;                      // texture handle flags
#define             HandleInUse     0x02        // this texture handle is allocated
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  DWORD             dwPaletteHandle;            // palette handle
#else
  DWORD             surfInterface;              // surface interface
#endif
  DWORD             txtrID;                     // texture descriptor ID
  DWORD             dwFlags;                    // surface pixel format flags
  DWORD             dwFourCC;                   // surface four CC code
  DWORD             dwCaps;                     // surface caps value
  DWORD             wWidth;                     // surface width
  DWORD             wHeight;                    // surface height
  LONG              lPitch;                     // surface pitch
  DWORD             dwColorSpaceLowValue;       // chroma key color space low value
  DWORD             dwColorSpaceHighValue;      // chroma key color space high value
  DWORD             dwZMask;                    // Z buffer bit mask value
  DWORD             dwZDepth;                   // Z buffer bit depth
  DWORD             dwStencil;                  // Stencil mask value
  DWORD             dwBitCnt;                   // RGB bit count value
  unsigned long     fpVidMem;                   // pointer to surface memory
#ifndef WINNT
  DWORD             processId;                  // process id that created this handle
#endif
  DWORD             contextId;                  // context that created this handle
  FXSURFACEDATA     *surfData;                  // surface data
  int               nLevels;                    // number of mipmap levels
  MIPMAPDATA        mmData[MAX_MIPMAP_LEVELS];  // mipmap data
} TXTRHNDL;

//------------------
// global misc. data
//------------------
typedef struct _D3GLOBAL
{
  D3DHAL_GLOBALDRIVERDATA d3d;

#if defined(NULLDRIVER)
  DWORD   ondrtTri; // ondrt=OverrideNullDriverviaRunTime
  DWORD   ondrtState;
  DWORD   ondrtTexD;
  DWORD   ondrtHWSU;
  DWORD   ondrtZB;
#endif

  DWORD   enable3DStudioMax;
  DWORD   sceneCount;
  DWORD   noLockIdle;
  DWORD   autoMipMap;
  #define AM_TRUE             1
  #define AM_LODMIN_MINUS_ONE 2

  DWORD   enableAntialias;

  // rest is for our private use
  long           D3D_DebugLevel;
  unsigned long  cpuType;
#ifdef WINNT	// OLD CPUID CODE
  #ifndef  cpuP6
  #define  cpuP6 6
  #endif
#endif

  // optimizations for rendering context changes
  ULONG    lastContext;

  // buffer pointer to an area to create texture maps
  #define D3D_BUFFER_SIZE ((MAX_TEXTURE_SIZE_U + 1) * MAX_TEXTURE_SIZE_V * 4)
  // This is a rediculously large amount of memory to allocate.....tl
  #define D3D_BUFFER_SIZE_TBIG ((1 << MAX_BIGTEXTURE_LOG) * (1 << (MAX_BIGTEXTURE_LOG - 3)) * 4)
  unsigned long *buffer ;

  long           flags;
  #define        PALETTECHANGED       (1 << 0)
  #define        IN_RENDER_SCENE      (1 << 1)

  // save off current value of dwContentStamp of texture palette.
  DWORD          prevContentStamp;

  void          *currentPalette;

  // texture memory
  long           startTREXMem;
  long           totalSizeTREXMem;
  long           totalFreeTREXMem;

  // rendering contexts & texture description definitions
  CONTEXT_DEFINE()
  TXTRDESC_DEFINE()
  TXTRHNDL_DEFINE()
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
#if HL_LINKED_LIST
#ifndef WINNT
  HNDLLIST  *pHndlList;
#endif
#else
  HNDLLIST  *pHndlList;
  DWORD     numHndlLists;
#endif  // HL_LINKED_LIST
#endif  // DX7

  struct {
    ULONG   changed;
    ULONG   fbzColorPath;
    ULONG   fogMode;
    ULONG   alphaMode;
    ULONG   fbzMode;
    ULONG   chromaKey;
    ULONG   zaColor;
    ULONG   colBufferAddr;
    ULONG   colAABufferAddr;
    ULONG   auxBufferAddr;
    ULONG   auxAABufferAddr;
// #ifdef H5
        ULONG  combineModeFBI;
        ULONG  combineModeT0;
        ULONG  combineModeT1;
        ULONG  renderMode;
        ULONG  stencilMode;
        ULONG  stencilOp;
        ULONG  sliCtrl;
        ULONG  aaCtrl;
        ULONG  clipLeftRight1; //cws gb
        ULONG  clipBottomTop1; //cws gb
// #ifdef NEW_CLIP_FOR_GB
        ULONG  clipLeftRight;
        ULONG  clipBottomTop;
// #endif
// #endif // H5
  } last;

  FxU32   pixelCenter;

#ifdef WINNT
  VIDEOMEMORY *zBuffer_pvmHeap;
#endif

#if defined(LEGACY_APPS)
#define LEGACYAPP1      (1 << 0)
        DWORD   legacyApp;
#endif

        ULONG   columnBandControl;          // Feeds CBC bits in fbzColorPath register.
        ULONG   thirtyTwoBppRendering;      // Enable/Disable 32bpp rendering
        ULONG   TwoPpc;                     // Enable/Disable 2pixels-per-clock rendering
        ULONG   TwoPpcLog2BandHeight;       // Band height of 2ppc in log2
        ULONG   GuardbandClipping;          // Enable/Disable guardband clipping
        ULONG   ditherRotation;             // Enable/Disable dither rotation

        // Dither rotation matrix selections.
        // 1 chip - 2 samples
        ULONG   DithMatSel_2Smpl;
        // 2 chip - 2 samples
        ULONG   DithMatSel_2Smpl_Chp0;
        ULONG   DithMatSel_2Smpl_Chp1;

        // 2 chips - 4 samples
        ULONG   DithMatSel_4Smpl_Chp0;
        ULONG   DithMatSel_4Smpl_Chp1;

        // 4 chips - 4 samples
        ULONG   DithMatSel_4Smpl_Chp2;
        ULONG   DithMatSel_4Smpl_Chp3;

        // 4 chips - 8 samples
        ULONG   DithMatSel_8Smpl_Chp0;
        ULONG   DithMatSel_8Smpl_Chp1;
        ULONG   DithMatSel_8Smpl_Chp2;
        ULONG   DithMatSel_8Smpl_Chp3;

        // Antialiasing jitter values. 3.4 fixed format.
        ULONG   aaJitterValues_2Smpl;
        // 2 chip - 2 samples
        ULONG   aaJitterValues_2Smpl_Chp0;
        ULONG   aaJitterValues_2Smpl_Chp1;
        ULONG   aaJitterValues_4Smpl_Chp0;
        ULONG   aaJitterValues_4Smpl_Chp1;

        // 4 chip - 4 samples
        ULONG   aaJitterValues_4Smpl_Chp2;
        ULONG   aaJitterValues_4Smpl_Chp3;
        // 4 chip - 8 samples
        ULONG   aaJitterValues_8Smpl_Chp0;
        ULONG   aaJitterValues_8Smpl_Chp1;
        ULONG   aaJitterValues_8Smpl_Chp2;
        ULONG   aaJitterValues_8Smpl_Chp3;

        ULONG   NapalmIsIn2ppc; /* For Tracking the falling edge of 2ppc mode (going from 2ppc to 1ppc)*/

		// Texture Lod Bias.
		long    LodBias;

#ifdef MAXPENDINGBUFFERS
		// Maximum number of pending buffers when you go to flip.
		ULONG   useMaxPendingBuffers;
		ULONG   maxPendingBuffers;
#endif

#if defined(LODDITHER) || defined(LODBIASPERCHIP)
		ULONG   LodDither;
#endif

#ifdef TnL_HAL
       // Controls the enabling of T&L							   
       ULONG    TransformAndLighting;
#endif


} d3Global;                     // buffer to create a texture map with aspect ratio = 8:1


// Use jump table for consecutive functions that set the rendering state
#ifdef WINNT
#define CHECK_STATE_LIMITS(state, min, max)       \
    if (((state) < (min)) || ((state) > (max)))   \
    {                                             \
      return D3D_OK;                              \
    }
#else
#define CHECK_STATE_LIMITS      1 ? (void)0 : (void)
#endif

extern  RENDERFXN   _renderFuncs[] ;               // defined in d3rstate.c

#if (DIRECT3D_VERSION >= 0x0700)
#define MAX_NUM_RSTATES D3DRENDERSTATE_CLIPPLANEENABLE      // = 152
#else
#define MAX_NUM_RSTATES D3DRENDERSTATE_WRAP7                // = 135
#endif


#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
#define MAX_NUM_TSTATES D3DTSS_RESULTARG                    // = 28
#else
#define MAX_NUM_TSTATES D3DTSS_TEXTURETRANSFORMFLAGS        // = 24
#endif


#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
 #define _D3(Field)  (((d3Global*)(ppdev->DriverData.pd3global))->Field)
 #define _D3G        ((*(d3Global*)ppdev->DriverData.pd3global))
#else
 #define _D3(Field) (((d3Global*)(ppdev->pd3global))->Field)
 #define _D3G ((*(d3Global*)ppdev->pd3global))
#endif

#define FNZ_STATE_MASK    (STATE_REQUIRES_ZBUFFER | STATE_REQUIRES_WBUFFER | STATE_REQUIRES_OOZ | STATE_REQUIRES_W_FBI | STATE_REQUIRES_VERTEXFOG | STATE_REQUIRES_HWFOG)
#define FNZ_SETUP_MASK    (SST_SETUP_Z | SST_SETUP_Wfbi)
#define FNZ_FOGMODE_MASK  (SST_ENFOGGING | SST_FOG_Z | SST_FOG_ALPHA | SST_FOG_DITHER)

#define FNZ_STATE   0
#define FNZ_SETUP   1
#define FNZ_FOGMODE 2

extern DWORD FnZTable[9][3];
extern DWORD getFnZOffset( RC *pRc );

#define TXTRFREEHANDLESFORCONTEXT(arg1,arg2) txtrFreeHandlesForContext(arg1,arg2)
#define TXTRFREEHANDLESFORPROCESS(arg1,arg2) txtrFreeHandlesForProcess(arg1,arg2)
#define TXTRINIT(arg1) txtrInit(arg1)
#if (DX >= 6)
#define TXTRDOWNLOADPALETTE(arg1,arg2,arg3,arg4) txtrDownLoadPalette(arg1,arg2,arg3,arg4)
#else
#define TXTRDOWNLOADPALETTE(arg1,arg2,arg3) txtrDownLoadPalette(arg1,arg2,arg3)
#endif
#define TXTRNEWPALETTE(arg1,arg2) txtrNewPalette(arg1,arg2)
#define TXTRCHANGEDPALETTE(arg1,arg2) txtrChangedPalette(arg1,arg2)
#define FXNOPCMD(arg1) fxNopCmd(arg1)

extern DWORD __stdcall ddiContextCreate(LPD3DHAL_CONTEXTCREATEDATA pccd) ;
extern DWORD __stdcall ddiContextDestroy(LPD3DHAL_CONTEXTDESTROYDATA pcdd) ;
extern DWORD __stdcall ddiContextDestroyAll(LPD3DHAL_CONTEXTDESTROYALLDATA pcdd) ;

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
#else
extern DWORD __stdcall ddiHandleCreate(LPD3DHAL_TEXTURECREATEDATA ptcd) ;
extern DWORD __stdcall ddiHandleDestroy(LPD3DHAL_TEXTUREDESTROYDATA ptcd) ;
extern DWORD __stdcall ddiHandleSwap(LPD3DHAL_TEXTURESWAPDATA ptsd) ;
extern DWORD __stdcall ddiHandleGetSurf(LPD3DHAL_TEXTUREGETSURFDATA ptgd) ;
#endif

#ifndef WINNT
#if (DIRECT3D_VERSION < 0x0600) && (DX < 6)
extern DWORD __stdcall ddiRenderState(LPD3DHAL_RENDERSTATEDATA prd) ;
extern DWORD __stdcall ddiRenderPrimitive(LPD3DHAL_RENDERPRIMITIVEDATA prd) ;
#endif
extern DWORD __stdcall ddiGetState(LPD3DHAL_GETSTATEDATA pgsd) ;
#endif
#if( DX >= 6 )
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  extern void  __stdcall TXTRDOWNLOADPALETTE(NT9XDEVICEDATA * ppdev, unsigned int, PPALHNDL, DWORD ) ;
#else
  extern void  __stdcall TXTRDOWNLOADPALETTE(NT9XDEVICEDATA * ppdev, unsigned int, LPPALETTEENTRY, DWORD ) ;
#endif
#else
extern void  __stdcall TXTRDOWNLOADPALETTE(NT9XDEVICEDATA * ppdev, unsigned int, LPPALETTEENTRY) ;
#endif
#if !defined(WINNT) && (DX <= 6)
extern void  __stdcall TXTRNEWPALETTE(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWPALETTE_GBL) ;
extern void  __stdcall TXTRCHANGEDPALETTE(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWPALETTE_GBL) ;
#endif
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
extern void  __stdcall TXTRNEWPALETTE(NT9XDEVICEDATA * ppdev, PALHNDL *) ;
extern void  __stdcall TXTRCHANGEDPALETTE(NT9XDEVICEDATA * ppdev, PALHNDL *) ;
#endif
extern void  __stdcall TXTRFREEHANDLESFORPROCESS(NT9XDEVICEDATA * ppdev, DWORD pid) ;
extern void  __stdcall TXTRFREEHANDLESFORCONTEXT(NT9XDEVICEDATA * ppdev, DWORD context) ;
extern void  __stdcall initNewRC(RC *pRc) ;
extern void  __stdcall fpTriABCC(RC *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
extern void  __stdcall fpTriABCCSTW(RC *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
extern void  __stdcall fpTriABCCSTWA(RC *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
extern void  __stdcall fpTriABCRGBSTWZ(RC *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
extern void  __stdcall fpTriABCRGBSTW(RC *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
extern void  __stdcall fpTriABCRGBSTWZA(RC *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
extern void  __stdcall fpTriABCRGBSTWA(RC *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
extern void  __stdcall fpDrawTriangle(RC *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
extern void  __stdcall fpDrawTriangleAll(RC *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
extern void  __stdcall fpDrawTriangleAllSW(RC *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
extern void  __stdcall fpFillTrianglePt(RC *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
extern void  __stdcall fpFillTriangleLine(RC *, WORD, LPD3DTRIANGLE, LPD3DTLVERTEX) ;
extern void  __stdcall fpPoint(RC *, WORD, LPD3DPOINT, LPD3DTLVERTEX) ;
extern void  __stdcall fpLine(RC *, WORD, LPD3DLINE, LPD3DTLVERTEX) ;
extern void  __stdcall fpDrawPoint(double, double, D3DTLVERTEX *, RC *, D3DCOLOR) ;
extern void  __stdcall fpDrawLine(D3DTLVERTEX *, D3DTLVERTEX *, RC *, D3DCOLOR) ;
extern void  __stdcall TXTRINIT(NT9XDEVICEDATA * ppdev) ;
extern void  __stdcall d3triInit( void ) ;
extern void  __stdcall setDrawTriangle(RC *pRc);
extern void  __stdcall setDrawTriangle6(RC *pRc);
#ifdef WINNT
extern HRESULT __stdcall dummy(RC *pRc, ULONG state) ;
#else
extern RENDERFXN_RETVAL  __stdcall dummy(RC *pRc, ULONG state) ;
#endif
extern void  createTableAndLoad(RC *pRc) ;
extern void  __stdcall checkForHWIdle() ;
extern int             float2int(float f);
extern void  setFogMode(RC *pRc);
extern void            FXNOPCMD(NT9XDEVICEDATA *ppdev);

FxU32 __cdecl DrawTriFanImmAsm_P5(FxI32 count,FxU32 *verts,FxU32 vb_offset);
void __cdecl DrawTriEdge2Asm_P5(FxU32 count,FxU32 *idx,FxU32 *verts);
void __cdecl DrawTriEdge2GAsm_P5(FxU32 count,FxU32 *idx,FxU32 *verts);
void __cdecl DrawTriEdge2FAsm_P5(FxU32 count,FxU32 *idx,FxU32 *verts);

#define TEXTURELOAD(arg1,arg2,arg3,arg4,arg5,arg6,arg7) textureLoad(arg1,arg2,arg3,arg4,arg5,arg6,arg7)
#define TEXTURESURFACECREATE(arg1,arg2,arg3,arg4) textureSurfaceCreate(arg1,arg2,arg3,arg4)
#define TEXTURESURFACEDELETE(arg1,arg2) textureSurfaceDelete(arg1,arg2)

DWORD __stdcall TEXTURELOAD(NT9XDEVICEDATA * ppdev, TXTRHNDL *, RECTL *, int, TXTRHNDL *, RECTL *, int);
extern DWORD __stdcall TEXTURESURFACECREATE(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWSURFACE_LCL surfLCL, int textureStage,  HRESULT *retCode);
extern void  __stdcall TEXTURESURFACEDELETE(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWSURFACE_LCL surfLCL);

// BobJ - 12/07/1999
#if defined(TnL_HAL) 
DWORD __stdcall CanCreateExecuteBuffer32( LPDDHAL_CANCREATESURFACEDATA lpd);
DWORD __stdcall CreateExecuteBuffer32( LPDDHAL_CREATESURFACEDATA  pcsd);
DWORD __stdcall DestroyExecuteBuffer32( LPDDHAL_DESTROYSURFACEDATA lpd);
DWORD __stdcall LockExecuteBuffer32( LPDDHAL_LOCKDATA lpd);
DWORD __stdcall UnlockExecuteBuffer32( LPDDHAL_UNLOCKDATA lpd);
#endif

#ifdef WINNT

#if (STBKNI==1) && (NEWASMTRI==1)
FxU32 __cdecl PrefetchTriIdx2_12_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_10_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_9_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_8_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_7_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_6T_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_5_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
#endif   // STBKNI && NEWASMTRI

#else    // NOT (WINNT)

#if (STBKNI==1)
#if (NEWASMTRI==1)
FxU32 __cdecl PrefetchTriIdx2_12_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_10_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_9_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_8_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_7_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_6T_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_5_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl DrawTriFanImmAsm_KNI(FxI32 count,FxU32 *verts,FxU32 vb_offset);
#else // Not NEWASMTRI
//STB-SK 01/30/99 KNI changes
FxU32 __cdecl PrefetchTriIdx2_12_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_11_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_10_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_9_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_8_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_7_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_6T_Asm_KNI(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl DrawTriFanImmAsm_KNI(FxI32 count,FxU32 *verts,FxU32 vb_offset);
#endif // NEWASMTRI
#endif // STBKNI

#endif // WINNT

FxU32 __cdecl PrefetchTriIdx2_12_Asm_P6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_11_Asm_P6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_10_Asm_P6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_9_Asm_P6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_8_Asm_P6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_7_Asm_P6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_6T_Asm_P6(FxU32 count,FxU32 *idx,FxU32 *verts);

FxU32 __cdecl PrefetchTriIdx2_12_Asm_P5(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_11_Asm_P5(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_10_Asm_P5(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_9_Asm_P5(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_8_Asm_P5(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_7_Asm_P5(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_6T_Asm_P5(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_6G_Asm_P5(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_5_Asm_P5(FxU32 count,FxU32 *idx,FxU32 *verts);

#ifdef K6_2
FxU32 __cdecl DrawTriFanImmAsm_K62O(FxI32 count,FxU32 *verts,FxU32 vb_offset);
void __cdecl DrawTriEdge2Asm_K62O(FxU32 count,FxU32 *idx,FxU32 *verts);
void __cdecl DrawTriEdge2GAsm_K62O(FxU32 count,FxU32 *idx,FxU32 *verts);

#if (NEWASMTRI == 1)
FxU32 __cdecl PrefetchTriIdx2_12_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_10_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_8_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_XX_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
#else // Not NEWASMTRI
FxU32 __cdecl PrefetchTriIdx2_12_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_11_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_10_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_9_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_8_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_7_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_6T_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_6G_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
FxU32 __cdecl PrefetchTriIdx2_5_Asm_K6(FxU32 count,FxU32 *idx,FxU32 *verts);
#endif // NEWASMTRI
#endif // K6_2


extern  void  __stdcall fpTriC( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

extern  void  __stdcall fpTriI( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

#if (DX >= 6)
typedef FxU32 __cdecl DrawTriFanImm_Type(FxI32 count,FxU32 *verts,FxU32 vb_offset);

extern DrawTriFanImm_Type*  DrawTriFanImmAsm;

typedef void __cdecl DrawTriEdge_Type(FxU32 count,FxU32 *idx,FxU32 *verts);

extern DrawTriEdge_Type*    DrawTriEdge2Asm ;
extern DrawTriEdge_Type*    DrawTriEdge2GAsm;
extern DrawTriEdge_Type*    DrawTriEdge2FAsm;

typedef FxU32 __cdecl Prefetch_TriIdx2_Type(FxU32 count,FxU32 *idx,FxU32 *verts);

extern Prefetch_TriIdx2_Type*   PrefetchTriIdx2_12_Asm;
extern Prefetch_TriIdx2_Type*   PrefetchTriIdx2_11_Asm;
extern Prefetch_TriIdx2_Type*   PrefetchTriIdx2_10_Asm;
extern Prefetch_TriIdx2_Type*   PrefetchTriIdx2_9_Asm;
extern Prefetch_TriIdx2_Type*   PrefetchTriIdx2_8_Asm;
extern Prefetch_TriIdx2_Type*   PrefetchTriIdx2_7_Asm;
extern Prefetch_TriIdx2_Type*   PrefetchTriIdx2_6T_Asm;
extern Prefetch_TriIdx2_Type*   PrefetchTriIdx2_6G_Asm;
extern Prefetch_TriIdx2_Type*   PrefetchTriIdx2_5_Asm;
#endif /* (DX >= 6) */

#ifdef _3DNOWDPFUNCS

extern  void  __stdcall fpTriIT_Orig( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
extern  void  __stdcall fpTriIZ_Orig( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
extern  void  __stdcall fpTriIZT_Orig( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
extern  void  __stdcall fpTriIZTH_Orig( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
extern  void  __stdcall fpTriIZS_Orig( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
extern  void  __stdcall fpTriIZTS_Orig( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

#else

extern  void  __stdcall fpTriIT( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
extern  void  __stdcall fpTriIZ( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
extern  void  __stdcall fpTriIZT( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
extern  void  __stdcall fpTriIZTH( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
extern  void  __stdcall fpTriIZS( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );
extern  void  __stdcall fpTriIZTS( RC* , WORD , LPD3DTRIANGLE , LPD3DTLVERTEX );

#endif // _3DNOWDPFUNCS

#ifdef WINNT	// OLD CPUID CODE
FxU32 __cdecl cpu_detect_asm(void);
FxU32 __cdecl getCPUInfo(void);
#if (STBKNI == 1)
FxU32 __cdecl getKatmai(void);
#endif
// defines for returned information about CPU
#define tcpuNoCPUID     0         // CPUID not supported
#define tcpuIntelCPU    1         // Intel  ("GenuineIntel")
#define tcpuAMDCPU      2         // AMD    ("AuthenticAMD")
#define tcpuCyrixCPU    3         // Cyrix  ("CyrixInstead")
#define tcpuIDTCPU      4         // IDT    ("CentaurHauls")
#define tcpuUnknownCPU -1         // What the hell is it ???

extern FxU32 _cpu_type;
#endif //WINNT

// Debugging print functions

#if defined( FLAVOR_PROFILE )
  extern  void  __stdcall triangleFlavour( RC *pRc, DWORD state, DWORD type, DWORD count );
  extern  int renderTypes;
#else
  #define triangleFlavour 1 ? (void)0 : (void)
#endif

#ifdef TXTR_PROFILE
  extern void __stdcall txtrFlavour( DWORD format );
#else
  #define txtrFlavour 1 ? (void)0 : (void)
#endif


#ifdef FXTRACE
  extern  void __cdecl    D3DPRINT(int debugPrintLevel, LPSTR szFormat, ... );
  extern  void            printRenderState(ULONG state, ULONG stateData);
  extern  void            printFbzColorPath( DWORD value );
  extern  void  __stdcall printCommand(LPD3DINSTRUCTION, LPBYTE);
  extern  char*           float2String( float value );
  extern  void            D3DPRINTTEXCOORD(unsigned long debuglevel, char *prefix, float s, float t);

  void setDebugLevel( char* str );

  extern  char dbgEntryStr[128];

  #define D3D_ENTRY(str)   lstrcpy( dbgEntryStr, str ); D3DPRINT( 1, "%s -> Entry", dbgEntryStr );
  #define D3D_EXIT(a)      D3DPRINT( 2, "%s <- Return <%s>\n", dbgEntryStr, a != DDHAL_DRIVER_HANDLED ? "RETURN ERROR" : "OK" ); return a

#else
  #define FXPRINT(arg)
  #define D3DPRINTTEXCOORD(arg1, arg2, arg3, arg4)
#if defined(WINNT) && DBG
  #define D3DPRINT          DebugPrint
#else
  #define D3DPRINT          1 ? (void)0 : (void)
#endif
  #define printRenderState  1 ? (void)0 : (void)
  #define printFbzColorPath 1 ? (void)0 : (void)

#if defined(WINNT) && DBG
  #define D3D_ENTRY(a)   D3DPRINT(1, ">> %s", a);
  #define D3D_EXIT(a)    D3DPRINT(1, "<< (retval = %08lXh)", a); return a
#else
  #define D3D_ENTRY(a)
  #define D3D_EXIT(a)    return a
#endif

#endif

#if defined(WINNT) && DBG
#define D3DDBGLVL           2
#define RSTATE_DBG_LVL      2
#define DLSTENCIL           15
#define DLSURFACE           16
#define DLALLOC             17
#define DLINFOMSGS          18

#define ENABLE_D3D_CMDFIFO_DUMP   0
//#define ENABLE_D3D_CMDFIFO_DUMP   1

#if defined(CMDFIFO) && ENABLE_D3D_CMDFIFO_DUMP
static __inline VOID
DumpCmdFifoEntries ( const char *funcName, PDEV *ppdev, ULONG *cmdFifoEnd )
{
  ULONG *i;
  ULONG *cmdFifoStart = ppdev->fifoData.fifoPtr;

  if (cmdFifoStart == cmdFifoEnd)
    return;

  D3DPRINT(D3DDBGLVL+1, "%s", funcName);
  D3DPRINT(D3DDBGLVL+1, "  starting cmdFifo = %08lX", cmdFifoStart);
  D3DPRINT(D3DDBGLVL+1, "  ending   cmdFifo = %08lX", cmdFifoEnd);
  for (i = cmdFifoStart; i < cmdFifoEnd; i+=4)
  {
    D3DPRINT(D3DDBGLVL+1, "%08lX : %08lX %08lX %08lX %08lX",
             i, *i, *(i+1), *(i+2), *(i+3));
  }
}
#else
#ifdef CMDFIFO
#define DumpCmdFifoEntries      1 ? (void)0 : (void)
#else
#define DumpCmdFifoEntries(funcName,ppdev,cmdFifoEnd)
#endif
#endif

#else
#define D3DDBGLVL           8
#define RSTATE_DBG_LVL      24
#define DLSTENCIL           15
#define DLSURFACE           16
#define DLALLOC             17
#define DLINFOMSGS          18

#define DumpCmdFifoEntries  1 ? (void)0 : (void)
#endif

// Override renderstate management - this is taken straight out of the
// d3dmini.c example in d3d ddk.
#define IS_OVERRIDE(type)       ((DWORD)(type) > D3DSTATE_OVERRIDE_BIAS)
#define GET_OVERRIDE(type)      ((DWORD)(type) - D3DSTATE_OVERRIDE_BIAS)

#define MAX_STATE       D3DSTATE_OVERRIDE_BIAS
#define DWORD_BITS      32
#define DWORD_SHIFT     5

#define STATESET_MASK(set, state)       \
        (set).bits[((state) - 1) >> DWORD_SHIFT]

#define STATESET_BIT(state)     (1 << (((state) - 1) & (DWORD_BITS - 1)))

#define STATESET_ISSET(set, state) \
        STATESET_MASK(set, state) & STATESET_BIT(state)

#define STATESET_SET(set, state) \
        STATESET_MASK(set, state) |= STATESET_BIT(state)

#define STATESET_CLEAR(set, state) \
        STATESET_MASK(set, state) &= ~STATESET_BIT(state)

#define STATESET_INIT(set)      memset(&(set), 0, sizeof(set))

#define STATE_REQUIRES_VERTS_AREA   (1 << 0)
#define STATE_REQUIRES_RGB          (1 << 1)
#define STATE_REQUIRES_IT_RGB       (1 << 2)
#define STATE_REQUIRES_IT_ALPHA     (1 << 3)
#define STATE_REQUIRES_OOZ          (1 << 4)
#define STATE_REQUIRES_W_FBI        (1 << 5)
#define STATE_REQUIRES_W_TMU0       (1 << 6) // currently broadcast to FBI & all TMUs
#define STATE_REQUIRES_ST_TMU0      (1 << 7) // currently broadcast to FBI & all TMUs
#define STATE_REQUIRES_W_TMU1       (1 << 8) // currently broadcast to FBI & all TMUs
#define STATE_REQUIRES_ST_TMU1      (1 << 9) // currently broadcast to FBI & all TMUs
#define STATE_REQUIRES_C0           (1 << 10)
#define STATE_REQUIRES_VERTEXFOG    (1 << 11)
#define STATE_REQUIRES_SPECULAR     (1 << 12)
#define STATE_REQUIRES_HWFOG        (1 << 13)
#define STATE_REQUIRES_WRAP         (1 << 14)
#define STATE_REQUIRES_PERSPECTIVE  (1 << 15)
#define STATE_REQUIRES_ZBUFFER      (1 << 16)
#define STATE_REQUIRES_WBUFFER      (1 << 17)

#define STATE_REQUIRES_AA           (1 << 20)

#define STATE_BAD_CPU               (1 << 24)
#define STATE_NOT_PERSPECTIVE       (1 << 25)
#define STATE_NOT_SOLID_FILL        (1 << 26)
#define STATE_MULTITEXTURING        (1 << 27)
#define STATE_VERTEX_FOG_WITH_Z     (1 << 28)

#if (DX >= 6)
#define FLAGS_FVF_IDX2                 (1 << 30)
#define FLAGS_FVF_EXECUTE              (1 << 31)

#define FLAGS_FVF                   (FLAGS_FVF_EXECUTE|FLAGS_FVF_IDX2)

#if !defined(CMDFIFO)  || defined(CSIM)

#define FLAGS_BAD_EXECUTE_CASE      (0xFFFF)

#define FLAGS_BAD_FAN_IMM_CASE      (0xFFFF)

#define FLAGS_BAD_TRI_IDX2_CASE     (0xFFFF)
#else // !defined(CMDFIFO)  || defined(CSIM)
#define FLAGS_BAD_EXECUTE_CASE      (STATE_REQUIRES_SPECULAR |  \
                                     STATE_VERTEX_FOG_WITH_Z |  \
                                     STATE_REQUIRES_WRAP |      \
                                     STATE_REQUIRES_RGB |       \
                                     FLAGS_FVF_EXECUTE |        \
                                     STATE_MULTITEXTURING |     \
                                     STATE_REQUIRES_AA |        \
                                     STATE_NOT_SOLID_FILL |     \
                                     STATE_NOT_PERSPECTIVE)

#define FLAGS_BAD_FAN_IMM_CASE      (STATE_REQUIRES_SPECULAR |  \
                                     STATE_REQUIRES_VERTEXFOG | \
                                     STATE_REQUIRES_WRAP |      \
                                     STATE_REQUIRES_RGB |       \
                                     STATE_MULTITEXTURING |     \
                                     STATE_REQUIRES_AA |        \
                                     STATE_NOT_SOLID_FILL |     \
                                     STATE_NOT_PERSPECTIVE)

#define FLAGS_BAD_TRI_IDX2_CASE     (STATE_REQUIRES_SPECULAR |  \
                                     STATE_REQUIRES_VERTEXFOG | \
                                     STATE_REQUIRES_WRAP |      \
                                     STATE_REQUIRES_RGB |       \
                                     FLAGS_FVF_IDX2 |           \
                                     STATE_MULTITEXTURING |     \
                                     STATE_REQUIRES_AA |        \
                                     STATE_NOT_SOLID_FILL |     \
                                     STATE_NOT_PERSPECTIVE)

#endif // !defined(CMDFIFO)  || defined(CSIM)
#endif

#define ZSCALE(zval) ((zval) * pRc->zScale)

#define _MM(field) (pRc->field)

#define PIXEL_OFFSET    pRc->sst.pixelOffset
#define TEXEL_SOFFSET   pRc->sst.centerS
#define TEXEL_TOFFSET   pRc->sst.centerT
#if( DX >= 6 )
#define TEXEL_S1OFFSET  pRc->sst.centerS1
#define TEXEL_T1OFFSET  pRc->sst.centerT1
#endif

#if( DX >= 6 )
typedef struct
{
   FxU32    *fifo_ptr;
   FxI32    fifo_room;
   FxU32    vertex_type;
   FxU32    vertex_size;
   FxU32    cull_mask;
   FxU32    t0_offset;
   float    scale_s;
   float    scale_t;
   float    tex_offset_s;
   float    tex_offset_t;
   float    scale_z;
   float    pixel_center;
   FxU32    cull_mode;
   FxU32    saved_ret_addr;
   FxI32    saved_count;
   RC *     pRc;

//STB-SK 01/30/99 -- KNI changes
#if (STBKNI == 1)
 #ifdef DCT_FIX
   float kni_array[24];  //RAJ 8/10/99 Need more room in the KNI array for DCT fix
 #else
   float kni_array[16];
 #endif
#endif

#if (NEWASMTRI == 1)
   FxU32    FVF_Size;
   FxU32    CMD_Start;
   FxU32    CMD_Cont;
   FxU32    CMD_Tri;
   FxU32    SetupFlag;
   FxU32    Specular_Offset;
   FxU32    t1_offset;
   FxU32    rhw_offset;
   FxU32    z_offset;
   FxU32    color_offset;
   float    scale_s1;
   float    scale_t1;
   float    tex_offset_s1;
   float    tex_offset_t1;
   float    scale_w;
   FxU32    RenderState;
   float    Scale_W_AW;
   float    Scale_W_BW;
   FxU32    CMD_ALPHA_BLEND_SPECULAR;
   FxU32    CMD_NO_ALPHA_BLEND_SPECULAR;
   FxU32    CMD_CONT_ALPHA_BLEND_SPECULAR;
   FxU32    CMD_CONT_NO_ALPHA_BLEND_SPECULAR;
#endif // NEWASMTRI
}ASMFifoData;

#if (STBKNI == 1)
#define ONE_1     0
#define ONE_2     1
#define SCALE_S2  2
#define SCALE_T2  3
#define PIXEL_C2  4
#define PIXEL_C3  5
#define ZERO_1    6
#define ZERO_2    7
#define ZERO_3    8
#define ZERO_4    9
#define TEX_OFF_S2   10
#define TEX_OFF_T2   11
#define SCALEZ_2  12
#define ONE_3     13
#define SCALEW_1  17 //RAJ New defines for DCT premult fix
#define SCALEW_2  18 //RAJ New defines for DCT premult fix
#define SCALEW_3  19 //RAJ New defines for DCT premult fix

#endif
extern ASMFifoData _asm_data;

#ifdef PERF_USE_255_LOOKUP_TABLE
// this table will convert 8 bit integer values to fp
// it is backwards to eliminate a subtraction from code lines like this: 
//      w1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR]));
static float f_255_reverse_lookup[256] = 
{
   255.0, 254.0, 253.0, 252.0, 251.0, 
   250.0, 249.0, 248.0, 247.0, 246.0, 245.0, 244.0, 243.0, 242.0, 241.0,
   240.0, 239.0, 238.0, 237.0, 236.0, 235.0, 234.0, 233.0, 232.0, 231.0,
   230.0, 229.0, 228.0, 227.0, 226.0, 225.0, 224.0, 223.0, 222.0, 221.0,
   220.0, 219.0, 218.0, 217.0, 216.0, 215.0, 214.0, 213.0, 212.0, 211.0,
   210.0, 209.0, 208.0, 207.0, 206.0, 205.0, 204.0, 203.0, 202.0, 201.0,
   200.0, 199.0, 198.0, 197.0, 196.0, 195.0, 194.0, 193.0, 192.0, 191.0,
   190.0, 189.0, 188.0, 187.0, 186.0, 185.0, 184.0, 183.0, 182.0, 181.0,
   180.0, 179.0, 178.0, 177.0, 176.0, 175.0, 174.0, 173.0, 172.0, 171.0,
   170.0, 169.0, 168.0, 167.0, 166.0, 165.0, 164.0, 163.0, 162.0, 161.0,
   160.0, 159.0, 158.0, 157.0, 156.0, 155.0, 154.0, 153.0, 152.0, 151.0,
   150.0, 149.0, 148.0, 147.0, 146.0, 145.0, 144.0, 143.0, 142.0, 141.0,
   140.0, 139.0, 138.0, 137.0, 136.0, 135.0, 134.0, 133.0, 132.0, 131.0,
   130.0, 129.0, 128.0, 127.0, 126.0, 125.0, 124.0, 123.0, 122.0, 121.0,
   120.0, 119.0, 118.0, 117.0, 116.0, 115.0, 114.0, 113.0, 112.0, 111.0,
   110.0, 109.0, 108.0, 107.0, 106.0, 105.0, 104.0, 103.0, 102.0, 101.0,
   100.0,  99.0,  98.0,  97.0,  96.0,  95.0,  94.0,  93.0,  92.0,  91.0,
   90.0,   89.0,  88.0,  87.0,  86.0,  85.0,  84.0,  83.0,  82.0,  81.0,
   80.0,   79.0,  78.0,  77.0,  76.0,  75.0,  74.0,  73.0,  72.0,  71.0,
   70.0,   69.0,  68.0,  67.0,  66.0,  65.0,  64.0,  63.0,  62.0,  61.0,
   60.0,   59.0,  58.0,  57.0,  56.0,  55.0,  54.0,  53.0,  52.0,  51.0,
   50.0,   49.0,  48.0,  47.0,  46.0,  45.0,  44.0,  43.0,  42.0,  41.0,
   40.0,   39.0,  38.0,  37.0,  36.0,  35.0,  34.0,  33.0,  32.0,  31.0,
   30.0,   29.0,  28.0,  27.0,  26.0,  25.0,  24.0,  23.0,  22.0,  21.0,
   20.0,   19.0,  18.0,  17.0,  16.0,  15.0,  14.0,  13.0,  12.0,  11.0,
   10.0,   09.0,  08.0,  07.0,  06.0,  05.0,  04.0,  03.0,  02.0,  01.0,  
   0.0
};
#endif // PERF_USE_255_LOOKUP_TABLE

#ifdef WINNT
#define AGP_FLUSH

// _asm_data.fifo_room is a dword count of available space in the fifo
// ppdev->fifoData.roomToEnd and roomToReadPtr are byte counts of available space
// so make the appropriate adjustments here

#define ASM_SET_RC(pRc)                                           \
  _asm_data.fifo_ptr  = (FxU32 *)ppdev->fifoData.fifoPtr;         \
  _asm_data.fifo_room = (FxI32)ppdev->fifoData.roomToEnd;         \
  if (ppdev->fifoData.roomToReadPtr < ppdev->fifoData.roomToEnd)  \
    _asm_data.fifo_room = (FxI32)ppdev->fifoData.roomToReadPtr;   \
  _asm_data.fifo_room /= sizeof(DWORD);                           \
  _asm_data.pRc = pRc;                                            \
  _asm_data.t0_offset=(fvfOffsetTable[pRc->dwVertexType].tu + pRc->t0CoordIndex)<< 2;

#define ASM_SET()                                                 \
  _asm_data.fifo_ptr = (FxU32 *)ppdev->fifoData.fifoPtr;          \
  _asm_data.fifo_room = (FxI32)ppdev->fifoData.roomToEnd;         \
  if (ppdev->fifoData.roomToReadPtr < ppdev->fifoData.roomToEnd)  \
    _asm_data.fifo_room = (FxI32)ppdev->fifoData.roomToReadPtr;   \
  _asm_data.fifo_room /= sizeof(DWORD);                           \
  _asm_data.pRc = pRc;

// set roomToReadPtr to zero to force next call to H3MakeRoom to update
// roomToReadPtr
#define ASM_SYNC()                                        \
  DumpCmdFifoEntries("ASM_SYNC", ppdev, _asm_data.fifo_ptr); \
  ppdev->fifoData.fifoPtr = (ULONG *)_asm_data.fifo_ptr;  \
  ppdev->fifoData.roomToEnd = CMDFIFO_ROOMTOEND;          \
  ppdev->fifoData.roomToReadPtr = 0;                      \
  AGP_FLUSH;

#else
#define ASM_SET_RC(pRc)                                  \
  _asm_data.fifo_ptr=(FxU32 *)(CMDFIFOPTR);         \
  _asm_data.fifo_room=(FxI32)(CMDFIFOSPACE);        \
  _asm_data.pRc = pRc; \
  _asm_data.t0_offset=(fvfOffsetTable[pRc->dwVertexType].tu + pRc->t0CoordIndex)<< 2;

#define ASM_SET()                                        \
  _asm_data.fifo_ptr=(FxU32 *)(CMDFIFOPTR);         \
  _asm_data.pRc = pRc; \
  _asm_data.fifo_room=(CMDFIFOSPACE);

#define ASM_SYNC()                                       \
  ((FxU32 *)CMDFIFOPTR)=(FxU32 *)_asm_data.fifo_ptr;     \
  CMDFIFOSPACE=_asm_data.fifo_room;                      \
  AGP_FLUSH;

#endif // WINNT

//STB-SK 01/30/99 -- KNI Changes
#if (STBKNI == 1)
#ifdef DCT_FIX
#define ASM_UPDATE_RC(pRc)                               \
  _asm_data.cull_mask=pRc->cullMask ^ 0x80000000;        \
  _asm_data.t0_offset=(fvfOffsetTable[pRc->dwVertexType].tu + pRc->t0CoordIndex)<< 2;\
  _asm_data.scale_z=pRc->zScale;                         \
  _asm_data.scale_s=pRc->sst.scaleS;                     \
  _asm_data.scale_t=pRc->sst.scaleT;                     \
  _asm_data.tex_offset_s=pRc->sst.centerS;               \
  _asm_data.tex_offset_t=pRc->sst.centerT;               \
  _asm_data.pixel_center=pRc->sst.pixelOffset;           \
  _asm_data.kni_array[SCALEZ_2] = pRc->zScale;           \
  _asm_data.kni_array[SCALE_S2] = pRc->sst.scaleS;       \
  _asm_data.kni_array[SCALE_T2] = pRc->sst.scaleT;       \
  _asm_data.kni_array[TEX_OFF_S2] = pRc->sst.centerS;    \
  _asm_data.kni_array[TEX_OFF_T2] = pRc->sst.centerT;    \
  _asm_data.kni_array[PIXEL_C2] = pRc->sst.pixelOffset;  \
  _asm_data.kni_array[PIXEL_C3] = pRc->sst.pixelOffset;  \
  _asm_data.kni_array[SCALEW_1] = pRc->scaleW;           \
  _asm_data.kni_array[SCALEW_2] = pRc->scaleW;           \
  _asm_data.kni_array[SCALEW_3] = pRc->scaleW;           \
  _asm_data.cull_mode=pRc->strip_mode;
#else
#define ASM_UPDATE_RC(pRc)                               \
  _asm_data.cull_mask=pRc->cullMask ^ 0x80000000;        \
  _asm_data.t0_offset=(fvfOffsetTable[pRc->dwVertexType].tu + pRc->t0CoordIndex)<< 2;\
  _asm_data.scale_z=pRc->zScale;                         \
  _asm_data.scale_s=pRc->sst.scaleS;                     \
  _asm_data.scale_t=pRc->sst.scaleT;                     \
  _asm_data.tex_offset_s=pRc->sst.centerS;               \
  _asm_data.tex_offset_t=pRc->sst.centerT;               \
  _asm_data.pixel_center=pRc->sst.pixelOffset;           \
  _asm_data.kni_array[SCALEZ_2] = pRc->zScale;           \
  _asm_data.kni_array[SCALE_S2] = pRc->sst.scaleS;       \
  _asm_data.kni_array[SCALE_T2] = pRc->sst.scaleT;       \
  _asm_data.kni_array[TEX_OFF_S2] = pRc->sst.centerS;    \
  _asm_data.kni_array[TEX_OFF_T2] = pRc->sst.centerT;    \
  _asm_data.kni_array[PIXEL_C2] = pRc->sst.pixelOffset;  \
  _asm_data.kni_array[PIXEL_C3] = pRc->sst.pixelOffset;  \
  _asm_data.cull_mode=pRc->strip_mode;
#endif
#else // STBKNI
#define ASM_UPDATE_RC(pRc)                               \
  _asm_data.cull_mask=pRc->cullMask ^ 0x80000000;        \
  _asm_data.t0_offset=(fvfOffsetTable[pRc->dwVertexType].tu + pRc->t0CoordIndex)<< 2;\
  _asm_data.scale_z=pRc->zScale;                         \
  _asm_data.scale_s=pRc->sst.scaleS;                     \
  _asm_data.scale_t=pRc->sst.scaleT;                     \
  _asm_data.tex_offset_s=pRc->sst.centerS;               \
  _asm_data.tex_offset_t=pRc->sst.centerT;               \
  _asm_data.pixel_center=pRc->sst.pixelOffset;           \
  _asm_data.cull_mode=pRc->strip_mode;
#endif // STBKNI

#endif

#define SETUP_PPDEV(dwhContext) \
   NT9XDEVICEDATA * ppdev = CONTEXT_PTR(dwhContext)->ppdev;

#define IS_DX7_APP(pRc)             ((pRc)->dwDXInterface == 3)
#define IS_DX7_OR_EARLIER_APP(pRc)  ((pRc)->dwDXInterface <= 3)

// These get used by the WRAP_3DNow macro
extern const float const_m1;
extern const float const_5;
extern const float const_1;

#endif //__D3GLOBAL_H__
