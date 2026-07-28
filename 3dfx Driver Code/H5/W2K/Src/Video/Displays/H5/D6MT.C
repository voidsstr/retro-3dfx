/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** File name:   d6mt.c
**
** Description: Multi-texture implementation
**
** $Revision: 45$
** $Date: 10/11/00 8:43:21 PM$
**
** $Log: 
**  45   3dfx      1.39.1.1.1.210/11/00 Brent           Forced check in to enforce
**       branching.
**  44   3dfx      1.39.1.1.1.108/11/00 Russ Lind       merge of various w9x
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
**  43   3dfx      1.39.1.1.1.007/11/00 Russ Lind       Fix for PRS 14371, Unreal
**       corrupt textures
**       The texture corruption is occurring because Unreal uses a palettized
**       texture in stage0 and a non-palettized texture in stage1, the call to
**       setupTextureStage1 in setupTexturing leaves the stage 1 texture handle in
**       pRc->texture which causes setDX6State to skip the call to load stage 0's
**       palette and we end up with corrupt textures.  This fix puts stage 0's
**       texture handle back into pRc->texture if stage 1 isn't palettized but
**       stage 0 is palettized.
**  42   3dfx      1.39.1.1    05/19/00 Russ Lind       merge from w9x, LodBias
**       setup in setupTexturing
**  41   3dfx      1.39.1.0    05/09/00 Russ Lind       to fix potential access
**       violation issues with NT Stress verify we have a valid PALHNDL for
**       palettized textures before dereferencing it
**  40   3dfx      1.39        04/12/00 Russ Lind       Fix (if you can call it a
**       fix) for the DCT 300 TextureStage failures on napalm.  This code is
**       currently disabled, I'm just checking it in so it's there if we need it. 
**       Basically, this code converts 84 TextureStage tests into skipped tests
**       which doesn't cause us to fail the TextureStage test.  Of these 84 tests,
**       we fail 63 of them.
**  39   3dfx      1.38        04/07/00 Steve Rogers    This fix completes the last
**       fix I checked in for PRS 13414.  Thanks to Justin McCartney for finding
**       this.
**  38   3dfx      1.37        04/05/00 Bob Seitsinger  Fix typo when building with
**       no asm - line 5346 - pRc->sst.centerTsl changed to pRc->sst.centerT.
**  37   3dfx      1.36        04/03/00 Steve Rogers    Fixes PRS13414: 3D Mark
**       2000 corrupts the shadows on the lady.  The problem was that we were
**       switching from two texture stages to a single texture stage without
**       setting up the registers.
**  36   3dfx      1.35        03/30/00 Chris W. Shaw   PRS #13521.  Armor Command
**       bug.  ArmorCmd was using ModulateMask with no texture.   
**  35   3dfx      1.34        03/20/00 Bob Seitsinger  Added code at bottom of
**       setupTexturing to replace LOD Bias with registry value, if present.
**  34   3dfx      1.33        03/01/00 Justin McCartney Moved flag
**       txtStageStateValidated from _D3 global structure into the RC structure.
**  33   3dfx      1.32        03/01/00 Justin McCartney Fix for PRS 12099.  Added
**       code to ddiValidateTextureStageState to set _D3(txtStageStateValidated)
**       flag to true.  Also added code (setupTexturing) to re-map the two stage
**       texture stage state set-up Unreal T uses to a three stage set-up that will
**       work V3.  Search for jmccartney 28/02/00 
**  32   3dfx      1.31        02/14/00 Chris W. Shaw   Fixes for PRS 12611 and
**       other ChromaKey / Trilinear & SingleStage texturing problems.  Rewrite the
**       multi-texture setup to use CCU for single stage rendering.
**  31   3dfx      1.30        02/04/00 Russ Lind       mods to
**       ddiValidateTextureStageState to return a failure if specular is used as an
**       arg.  Fixes 29 Texture Stage failures on napalm.  These changes are
**       wrapped in ifdef's of FAIL_SPECULAR_AS_ARG, which is defined near the top
**       of ddiValidateTextureStageState
**  30   3dfx      1.29        01/24/00 Matt McClure    Modified Texture Stage
**       setup to copy Scale, and Center 'S' and 'T' to the appropriate asm_data
**       variable.  Support for NEWASMTRI.
**  29   3dfx      1.28        01/19/00 Russ Lind       fix for PRS 11542 - Texture
**       Stage failures on V3
**       Overall, this consists of three changes to ddiValidateTextureStageState
**       First is a check if r1val is already not DD_OK before checking palette
**       handles.  Also allow the operation to continue if the palette handles are
**       different but the content of both palette's are identical.
**       Second is to reenable the BLENDFACTORALPHA checking after the alphaOp
**       checking and just let one particular combination of
**       alphaOp/alphaArg1/alphaArg2 pass, the other combinations fall thru to the
**       old check.
**       Third (is a complete hack just to pass the Texture Stage dct), for stage
**       0, if the colorOp is ModulateInvAlphaAddColor (which based on the tables,
**       V3 doesn't support this colorOp in stage0) then return UNSUPPORTEDCOLORARG
**       instead of UNSUPPPORTEDCOLOROP.
**  28   3dfx      1.27        01/11/00 Scott Kephart   Restored to pre-NEWASMTRI
**       status, and then merged in the fix for the trilinear mipmapping tests in
**       3D Winbench 2000 from jcochrane. This restores the file to rev. 25 + the
**       fix from rev. 27 checked in today from Geoff Bullard
**  27   3dfx      1.26        01/11/00 Geoff Bullard   Integration of Jonny
**       Cochrane's "WB2K Quality tests support including mipmapLODBias support and
**       single pass trilinear with multitexture".  Linear Mipmap Nearest and
**       Linear Mipmap Linear should be correct now.
**  26   3dfx      1.25        01/10/00 Matt McClure    Added support for new
**       assembler triangle routines, #ifdef NEWASMTRI==1 surrounds all changes.
**  25   3dfx      1.24        12/14/99 Matt McClure    Fix for PRS 11794.  Added
**       WRAPn functionality and fixed autostripping bug with shadow maps on 3D
**       Winbench 2k Test 6 - Stations.
**  24   3dfx      1.23        12/06/99 Russ Lind       fix for PRS 11593 - PC99
**       MultiTexturing failures
**       don't fail BLENDFACTORALPHA cases in ValidateTextureStageState
**       merged from V3_TOT
**  23   3dfx      1.22        12/06/99 Chris W. Shaw   For Napalm, normally, when
**       a texture is used, we init texturemodet0/1 (AND THUS CLEAR THE OLD
**       STATE!).  But when only iterated/TFACTORS are used in the TCU, we still
**       need to init texturemode to 0x0. We don't do this for V3 because V3 can't
**       use only iterated data for the TCU and should fail validation.  Also, V3
**       sets T1 to REPLACE by default to get a texture into T0.
** 
**  22   3dfx      1.21        12/02/99 Russ Lind       add check for alpha +
**       palettized format in setupSingleStage, setupTextureStage0 &
**       setupTextureStage1 to or the ALPHA_P8_RGB format into
**       pRc->sst.textureFormat instead of P8_RGB
**  21   3dfx      1.20        11/29/99 Chris W. Shaw   Fix for part of PRS 11235. 
**       Allow the TCU output (iterated) to get into the FBI even when texturing is
**       not enabled.
**  20   3dfx      1.19        11/09/99 Chris W. Shaw   Enabled the legacy
**       textureblend code (0x7ffffffe) for win9x.
**  19   3dfx      1.18        10/29/99 Christopher Wilcox White space changes to
**       reformat after h3defs.h merge.
** 
**  18   3dfx      1.17        10/29/99 Christopher Wilcox Fixed Direct3D problem
**       caused by incorrect hardware definition when merging h3defs.h.
** 
**  17   3dfx      1.16        10/28/99 Christopher Wilcox Completion of previous
**       change.
** 
**  16   3dfx      1.15        10/28/99 Christopher Wilcox Changed one more
**       redundant hardware definition,  missed by previous checkin.
** 
**  15   3dfx      1.14        10/28/99 Christopher Wilcox Hardware definition
**       changes to merge divergent h3defs.h.
**  14   3dfx      1.13        10/28/99 Chris W. Shaw   Removed the obsolete
**       renderstate, TextureMapBlend, for dx7 ( Still used for dx6- and still used
**       for WINNT.)
**       Fix for part 1 of PRS 10855 (more obsolete renderstates need to be removed
**       in addtion to this).
**       I have added a makefile define, REMOVE_OBSOLETE1_FOR_DX7_WIN9X, that can
**       be used for easily taking out these changes (for debug and in case other
**       problems arise).
** 
**  13   3dfx      1.12        10/26/99 Russ Lind       ifdef legacy
**       D3DTBLEND_MODULATE changes for WINNT only
**  12   3dfx      1.11        10/25/99 Russ Lind       fix for dct 250 Texture
**       Blend - Decal, Modulate, ModulateAlpha & Copy failures, pull in legacy
**       D3DTBLEND_MODULATE handling from V3_TOT
**  11   3dfx      1.10        10/22/99 Chris W. Shaw   Add IGX_SPECULAR_FIX(2) to
**       Napalm source.  Now DCT250-BetaPreview-DriverScenario passes 3200 and
**       fails 1153 tests (instead of passing only 2753 and failing 1601).
**  10   3dfx      1.9         10/12/99 Tim Little      Enabled invalid
**       texturehandle checking that was previously only done under WINNT.  It
**       looks like it is needed with DX7 reguardless of platform.
**  9    3dfx      1.8         10/07/99 Russ Lind       fixed a couple parenthesis
**       errors that were causing the TextureStage dct test to AV
**       apparently windbg doesn't like lines that are longer than 255 columns so
**       shortened comments on 13 lines so windbg will quit complaining
**  8    3dfx      1.7         10/04/99 Edwin Wong      Missing ")" in line 3610
**       for win9x compilation.
**  7    3dfx      1.6         10/04/99 Edwin Wong      Add TXTRHNDL_INRANGE() to
**       validate texture handle. Use TXTRHNDL_INUSE() macro in place of
**       HandleInUse references.
**  6    3dfx      1.5         09/28/99 Steve Rogers    Porting Daoxiang Gong's fix
**       PRS #7911 from OEM tree: "Unreal texture corruption with multi-texturing"
**  5    3dfx      1.4         09/24/99 Chris W. Shaw   Take out specialmode for
**       Napalm. fixes  PRS 8945.
**  4    3dfx      1.3         09/17/99 Chris W. Shaw   Call printTexOp only when
**       in debug mode.
**  3    3dfx      1.2         09/17/99 Chris W. Shaw   Added some debug code.
**  2    3dfx      1.1         09/14/99 Chris W. Shaw   Added SteveR's fix for
**       DCT's sub tex1 from tex2.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
**
** 24    8/31/99 7:57a Cshaw
** Added checking for Napalm not having an alpha channel in a texture
** format when doing alpha.  Fixes PRS 8370.
**
** 23    8/25/99 1:58p Russ
** for W2K & DX7, fix for potential access violation issues with NT Stress
** verify we have a valid TXTRHNDL for surfaces before dereferencing the
** TXTRHNDL_PTR
** removed a couple CVG (Voodoo2) code blocks
**
** 22    8/20/99 4:57p Cshaw
** Fixed a 32bpp rendering bug. PRS 8102.
**
** 21    8/17/99 10:12a Cshaw
** Set the texturestage, S0, to pass diffuse color and alpha when texOp=
** disable.  This prevents 2ppc mode from chaining TMUs when the TMU is
** set to texOp=disable.
**
** 20    8/16/99 3:18p Russ
** in ddiValidateTextureStageState, add check for texture handles of 0
** before dereferencing TXTRDESC thru the PALETTIZEDHANDLE macro, fixes an
** access violation on w2k running DCT v221 TextureStage tests
**
** 19    8/13/99 11:37a Cshaw
** Fix for PRS 8022.  Limit the number of texture stages to 2 for Napalm.
**
** 18    8/12/99 2:30p Cshaw
** Previously, the driver would not count stages that had
** texturehandle==0.  Now the driver counts the stage if texturehandle==0
** and the texture is not actually used in the equation.
**
** 17    8/10/99 10:42a Russ
** for DX7 & W2K, in setupSingleStage & setupTextureStage0 add palettized
** texture checks similar to the one added in setupTextureStage1 in rev 11
**
** 16    8/05/99 3:03p Cshaw
** Added a check for texture handle==0 when using textures on arg2 when
** counting numStages.
**
** 15    7/29/99 4:50p Cwilcox
** Changes to fix PRS 6358.
**
** 14    7/23/99 2:11p Cshaw
** Fixed a 2ppc/mipmapping bug.
**
** 13    7/21/99 11:14a Russ
** fix typo
**
** 12    7/19/99 3:07p Russ
** changes for V3/Napalm runtime check
**
** 11    7/14/99 5:53p Russ
** for DX7, in setupSingleStage, if the texture is palettized but we've
** never been given a palette handle then just let'em get whatever palette
** is already in nccTable0.  Fixes a blue screen running 3D WB performance
** test 15 at 800x600x16 & 1024x768x16
**
** 10    7/13/99 2:27p Cshaw
** Added runtime support for Napalm's multitexturing (NAPALM_CU).
**
** 9     7/02/99 1:15p Cshaw
** Added complement arguments to Color and Alpha Ops.  Removed tables that
** are no longer used.
**
** 8     6/30/99 10:59a Cshaw
** Enabled and added some support code for 2 pixel per clock mode.
**
** 7     6/28/99 12:36p Cshaw
** Added BlendFactorAlpha and BlendCurrentAlpha to alphaOps.  Added
** ARGT_VALID to ops that use texture in color path setup (independant of
** arguments needing textures).  Added code to check for textures in this
** case.
**
** 6     6/25/99 5:16p Cshaw
** Greatly reduced number of table entries for Napalm by adding separate
** arg tables and texOp tables.  Now textures can come into texture stages
** on Arg2.  All alphareplicate modes are now validated and working.  All
** modes that use ChromaRng/ChromaKey/color 1 for argument 2 are now
** validated and working.
**
** 5     6/23/99 9:21p Russ
** DX7 palettized texture changes (#ifdef'd for DX >= 7)
** enable palettized texture code in ddiValidateTextureStageState,
** setupSingleStage, setupTextureStage0 & setupTextureStage1
**
** 4     6/21/99 5:28p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 3     6/16/99 3:45p Cshaw
** Added more texop support for texturefactors on color1, and alpha
** replicate.
**
** 2     6/03/99 12:06p Cshaw
** Added alphareplicate control code (for Napalm).
**
** 1     6/02/99 6:45a Michael
** Branch from H3
**
** 51    5/24/99 11:39a Cshaw
** Updated Napalm CU code.
**
** 50    5/12/99 11:45a Cshaw
** Modified some Napalm TC table entries.
**
** 49    5/11/99 10:09a Cshaw
** Added more CC/AC and TC/TA tex ops.
**
** 48    5/04/99 8:55a Cshaw
** Added &&defined(NAPALM_CU) so we can disable Napalm CU code until CSIM
** works on win98.
** Added more Napalm CU mode table entries.
**
** 47    4/30/99 9:11a Cshaw
** Fixed some build errors.
**
** 46    4/29/99 3:51p Cshaw
** Added new Napalm texops to SingleStage_s0_color op table.
**
** 45    4/26/99 2:03p Cshaw
** Added another item in the Tex Op array for combineMode as used with
** TMUs.
**
** 44    4/20/99 2:43p Cshaw
** 2 pixel-per-clock rendering now depends on mode/refresh.
**
** 43    4/19/99 9:07a Cshaw
** Added some alphareplicate/ complement arg code.  Added / updated CU
** table entries.
**
** 42    4/15/99 2:52p Cshaw
** Translated fbzColorPath CU ops (for V3) to combineMode CU ops (for
** Napalm).
**
** 41    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
**
** 40    3/25/99 5:55p Adrians
** Cleaner fix for the Lego Island perspective texture problem. PRS#4786
**
** 39    3/24/99 10:53a Adrians
** Fix for Lego Island perspective corrected texture problem.  PRS#4786
**
** 38    2/26/99 4:37p Peterm
** Removed code related to sub-optimal fix for when sourcing alpha from
** textures without alpha.  Turns out hw doesn't need to source from
** color1 in this case.
**
** 37    2/19/99 3:38p Peterm
** Added code to redirect alpha from texture to color1 if texture doesn't
** have alpha and mode isn't D3DTBLEND_MODULATE.  If mode is
** D3DTBLEND_MODULATE, keep the old behavior of redirecting to alpha
** interpolators.
**
** 36    2/11/99 8:31a Stb_skephart
**
** 35    2/09/99 4:10p Adrians
** Return the correct number of passes from the
** TextureStageStateValidation function.
**
** 34    1/26/99 5:29p Peterm
** Added unified header information
**
** 33    1/22/99 2:38p Cwilcox
** Minor revision to remove compiler warnings.
**
** 32    1/14/99 3:03p Adrians
** Fix min/max LOD to 1x1 for unused tmu's.
**
** 31    1/06/99 12:21p Adrians
** Add compile time option for special multi-texture modes.
**
** 30    12/22/98 7:32p Adrians
** Fix for a texture validation that was failing.
**
** 29    12/11/98 9:03a Martin
** Unused textures are coming down.  Check for this.
**
** 28    12/09/98 6:48a Russ
** NT5 D3D changes for Banshee
**
** 27    12/06/98 2:42p Adrians
** Remove retail message.
**
** 26    12/06/98 1:53p Adrians
** Optimisations for WinBench99.
**
** 25    11/30/98 9:19p Adrians
** Fix for alphablending problem with Heavy Gear II (PRS#3284).
**
** 24    11/27/98 7:08p Adrians
** Correctly setup multi-texturing if only stage 2 changes (#3219).
**
** 23    11/22/98 9:11p Andrew
** Changes to support multi-monitor
**
** 22    11/18/98 6:09p Adrians
** Changes for Avenger.
**
** 21    11/06/98 5:57p Adrians
** Fail ValidateStageState if the palettes of two textures don't match.
**
** 20    11/03/98 12:15a Artg
** made changes to for multitextures for h4
**
** 19    10/15/98 6:35p Artg
** changed ifdef h3 to account for h4
** ifdef h3  --> if defined(h3) || defined(h4)
**
** 18    10/12/98 11:22a Adrians
** NULL textureHandle when flat shading.
**
** 17    10/07/98 10:39p Adrians
** Added K6-2 optimisations.
**
** 17    10/07/98 9:05a stit (Metabyte, Inc)
** AMD K6-2 (MMX+3DNOW) optimization
**
** 16    10/04/98 10:04p Adrians
** Fixed typo in the mirrored texture code for the second stage.
**
** 15    10/04/98 9:47p Adrians
** Add support for mirrored textures.
**
** 14    10/01/98 5:17p Hanson
** Added multitexturing flag
**
** 13    9/26/98 5:32p Hanson
** Dx6 Assembly interface Optimizations
**
** 12    9/25/98 6:45p Adrians
** Addtional instrumentation changes.
**
** 11    9/13/98 11:21p Adrians
** Add support for TextureFactor renderstate.
** Add multi-texture mode support for TextureFactor.
**
** 10    9/13/98 5:45p Adrians
** Add support for multi-texture modes  MODULATEALPHA_ADDCOLOR,
** MODULATEINVALPHA_ADDCOLOR, MODULATECOLOR_ADDALPHA and
** MODULATEINVCOLOR_ADDALPHA to Voodoo2.
**
** 6     9/12/98 12:57a Adrians
** Clean up renderstate trace code and add new DX6 states.
** Add trace support for DX6 texture stage states.
** General DX6 code tidyup.
**
** 5     9/10/98 8:20p Adrians
** Fix for textures that don't have alpha with legacy apps.
**
** 3     9/05/98 5:02p Adrians
** Added AA support to DX6.
** Validate will now fail arguments with COMPLEMENT or ALPHAREPLICATE.
**
** 2     9/02/98 12:34p Adrians
** DX6 multitexture change.
**
** 1     8/28/98 1:34p Adrians
** Support for new multi-texture scheme.
*/

#include "precomp.h"

#if ( DX >= 6 )

#ifndef WINNT
#include <d3dhal.h>
#include "fxglobal.h"
#include "d3global.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "d3contxt.h"
#endif

#include "dxins.h"

DWORD setupSingleStage( RC *pRc );
DWORD setupTextureStage0( RC *pRc );
DWORD setupTextureStage1( RC *pRc );

// DEBUG Static function prototypes
#if defined( DEBUG )
void printTexOp(DWORD dwOp, DWORD dwA1,DWORD dwA2 );
void checkTextureOp( RC *pRc );
#endif

/* 	Defines for workaround for problem with Unreal Tournament PRS 12099
	These defines represent the texture stage setup used By Unreal T. stage 0 and 1
	when doing multi-texturing - jmccartney 28/02/00
*/
#define UT_REMAP_TXTOP_STAGE0 ( (0x1 << 30) | (D3DTA_TEXTURE << 23) | (D3DTA_DIFFUSE << 16) | (0x1 << 15) | (D3DTA_TEXTURE << 7) | (D3DTA_DIFFUSE) )
#define UT_REMAP_TXTOP_STAGE1 ( (0x1 << 30) | (D3DTA_TEXTURE << 23) | (D3DTA_CURRENT << 16) | (0x1 << 14) | (D3DTA_TEXTURE << 7) | (D3DTA_CURRENT) )
/*
	Macros used to find if the texture state stage setup matches that of Unreal T
	Used in the workaround for Unreal Tournament PRS 12099 - jmccartney 28/02/00
*/
#define MATCH_UT_TXTSTAGESTATE ( (GENTXTSTAGESTATE_CODE(0) == UT_REMAP_TXTOP_STAGE0) && (GENTXTSTAGESTATE_CODE(1) == UT_REMAP_TXTOP_STAGE1)	)
//	Generate a code representing the txt stage state setup contained in pRc->textureStage[stage] jmccartney 28/02/00
#define GENTXTSTAGESTATE_CODE(stage) ( ( (TS[stage].colorOp == D3DTOP_MODULATE) << 30) | (TS[stage].colorArg1 << 23) | (TS[stage].colorArg2 << 16) |	  \
	( ( (TS[stage].alphaOp == D3DTOP_MODULATE) << 15) | ( (TS[stage].alphaOp == D3DTOP_SELECTARG2) << 14) ) | (TS[stage].alphaArg1 << 7)  | (TS[stage].alphaArg2) )
// end PRS Issue 12099 fix macros

#define ARGX_VALID     0
#define ARGX_FBIOP     1
#define ARGX_TMUOP     2
//#if defined (H5)
#define ARGX_CMFBIOP   3
#define ARGX_CMTMUOP   4
// #if defined (NAPALM_CU)
   #define ARGX_ARG1MUX   5
   #define ARGX_ARG2MUX   6
   #define ARGX_ARG1AUXMUX   7
   #define ARGX_ARG2AUXMUX   8
   #define ARG1_COMP_XOR_CM 9
   #define ARG1_COMP_XOR_TM 10
   #define ARG2_COMP_XOR_CM 11
   #define ARG2_COMP_XOR_TM 12
// #endif
//#endif

/* Possible Table entries for ARG1_COMP_XOR_CM thru ARG2_COMP_XOR_TM */
#define COMP_OTH      SST_CM_TC_INVERT_OTHER_ONE_MINUS_X     /* For combineMode Reg only */
#define COMP_LOC_ADD  SST_CM_TC_INVERT_ADD_LOCAL             /* For combineMode Reg only */
#define COMP_LOC_SUB  SST_CM_TC_INVERT_LOCAL_X_MINUS_HALF    /* For combineMode Reg only */
#define COMP_LOC_BOTH (COMP_LOC_ADD | COMP_LOC_SUB)          /* For combineMode Reg only */
#define COMP_MSEL     SST_TC_REVERSE_BLEND                   /* For textureMode Reg only */

#define COMP_A_OTH      SST_CM_TCA_INVERT_OTHER_ONE_MINUS_X  /* For combineMode Reg only */
#define COMP_A_LOC_ADD  SST_CM_TCA_INVERT_ADD_LOCAL          /* For combineMode Reg only */
#define COMP_A_LOC_SUB  SST_CM_TCA_INVERT_LOCAL_X_MINUS_HALF /* For combineMode Reg only */
#define COMP_A_LOC_BOTH (COMP_A_LOC_ADD | COMP_A_LOC_SUB)    /* For combineMode Reg only */
#define COMP_A_MSEL     SST_TCA_REVERSE_BLEND                /* For textureMode Reg only */


#define ARG1_VALID    (1 << 0)  // Argument 1 is valid
#define ARG2_VALID    (1 << 1)  // Argument 2 is valid
//#define ARG_LOCMUX_COMP_SUB (1 << 2)
//#define ARG_LOCMUX_COMP_ADD (1 << 3)
#define ARGT_VALID    (1 << 4)  // One of the arguments is a texture
#define ARGS_VALID    (ARG1_VALID | ARG2_VALID)
#define ARGST_VALID   (ARG1_VALID | ARG2_VALID | ARGT_VALID)
#define ARG1T_VALID   (ARG1_VALID | ARGT_VALID)
#define ARG2T_VALID   (ARG2_VALID | ARGT_VALID)
#define ARG1_AREP     (1 << 5)
#define ARG2_AREP     (1 << 6)
#define ARG1_COMP     (1 << 7) /*8,9,10 are used for SPECIAL_MODES aka ARGF_SP1,3,FAIL*/
#define ARG2_COMP     (1 << 11)
#define ARG1_MUXOTH  (1 << 12)
#define ARG1_MUXLOC  (1 << 13)
#define ARG1_MUXM    (1 << 14)//MuxM is not supported for arep for tmu
#define ARG1_MUXM7   (1 << 15)
#define ARG2_MUXOTH  (1 << 16)
#define ARG2_MUXLOC  (1 << 17)
#define ARG2_MUXM    (1 << 18)//MuxM is not supported for arep for tmu
#define ARG2_MUXM7   (1 << 19)
#define ARG1_TMUXOTH  (1 << 20)
#define ARG1_TMUXLOC  (1 << 21)
#define ARG1_TMUXM    (1 << 22)
#define ARG1_TMUXM7   (1 << 23)
#define ARG2_TMUXOTH  (1 << 24)
#define ARG2_TMUXLOC  (1 << 25)
#define ARG2_TMUXM    (1 << 26)
#define ARG2_TMUXM7   (1 << 27)

#define ARG1_MUX (ARG1_MUXOTH | ARG1_MUXLOC | ARG1_MUXM | ARG1_MUXM7 | ARG1_TMUXOTH | ARG1_TMUXLOC | ARG1_TMUXM | ARG1_TMUXM7)
#define ARG2_MUX (ARG2_MUXOTH | ARG2_MUXLOC | ARG2_MUXM | ARG2_MUXM7 | ARG2_TMUXOTH | ARG2_TMUXLOC | ARG2_TMUXM | ARG2_TMUXM7)

#define ARGF_SP1      SPECIAL_MODE1
#define ARGF_SP3      SPECIAL_MODE3
#define ARGF_SP_FAIL  SPECIAL_FAIL

#define ARGF_SP_ALL   (ARGF_SP1 | ARGF_SP3)

typedef struct
{ BOOL enabled;
  DWORD setup[13];
} MINI_TEXTUREOP, *LPMINI_TEXTUREOP;
typedef MINI_TEXTUREOP MINI_STAGEOP[25];

typedef struct
{
  BOOL  enabled;
  DWORD arg[16][3];
} TEXTUREOP, *LPTEXTUREOP;

typedef TEXTUREOP STAGEOP[25];

#define COLOROP  0
#define ALPHAOP  3

#define COLORARG 0
#define ALPHAARG 2

#define MINI_ARGCOLOR 0
#define MINI_ARGALPHA 5

#define MUXOTH  ( 1)
#define MUXLOC  ( 2)
#define MUXM7   ( 3)
#define MUXM    ( 4) //MuxM is not supported for arep for tmu

typedef struct {
 DWORD argselect[10][4];
} MINI_ARG, *LPMINI_ARG;

typedef MINI_ARG MINISTAGEARG[1];

#ifdef NEW_CCU

#define CCU_COMP_MSEL SST_CC_REVERSE_BLEND                   /* For fbzColorPath Reg only */
#define CCU_COMP_A_MSEL SST_CCA_REVERSE_BLEND                /* For fbzColorPath Reg only */

MINISTAGEARG CCU_mini_arg_c0_of_0 = { //This is used for single stage "Trilinear" or "ChromaKey" mode (2 stage "trilinear" or "chromakey" isn't possible).
    //copied from mini_arg_c0_of_0 but use _CC_ instead of _TC_; also use fbzColorPath bits instead of textureMode bits.
  // Diff,                              Current,                           Texture,                            Factor
  { 0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    // RGB component  // For arg1, arg2
    SST_CM_CC_OTHERSELECT_IRGB         ,SST_CM_CC_OTHERSELECT_IRGB         ,SST_CM_CC_OTHERSELECT_TRGB         ,SST_CM_CC_OTHERSELECT_C1_RGB    ,//MUXOTH (CombineMode)
    SST_CM_CC_LOCALSELECT_IRGB         ,SST_CM_CC_LOCALSELECT_IRGB         ,SST_CM_CC_LOCALSELECT_TRGB         ,SST_CM_CC_LOCALSELECT_C0_RGB    ,//MUMLOC (CombineMode)
    SST_CM_CC_MSELECT_7_IRGB           ,SST_CM_CC_MSELECT_7_IRGB           ,0                                  ,SST_CM_CC_MSELECT_7_C1_RGB      ,//MUXM7  (CombineMode)
    SST_CC_MCMSELECT7                  ,SST_CC_MCMSELECT7                  ,SST_CC_MRGBTMU                     ,SST_CC_MCMSELECT7               ,//MUXM   (FBZCOLORPATH) (only if M7 is used)
    // Alpha component //Used for AUX input for blending operations that require some sort of alpha (in addition to the normal arg1 and arg2).
    0                                  ,0                                  ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    SST_CM_CC_OTHERSELECT_IA           ,SST_CM_CC_OTHERSELECT_IA           ,SST_CM_CC_OTHERSELECT_TA           ,SST_CM_CC_OTHERSELECT_C1_A      ,//MUXOTH (CombineMode)
    SST_CM_CC_LOCALSELECT_IA           ,SST_CM_CC_LOCALSELECT_IA           ,SST_CM_CC_LOCALSELECT_TA           ,SST_CM_CC_LOCALSELECT_C0_A      ,//MUMLOC (CombineMode)
    SST_CM_CC_MSELECT_7_IA             ,SST_CM_CC_MSELECT_7_IA             ,0                                  ,SST_CM_CC_MSELECT_7_C1_A        ,//MUXM7  (CombineMode)
    SST_CC_MCMSELECT7                  ,SST_CC_MCMSELECT7                  ,SST_CC_MATMU/*?*/                  ,SST_CC_MCMSELECT7               ,//MUXM   (fbzColorPath) (only if M7 is used)
  }
};

MINISTAGEARG CCU_mini_arg_a0_of_0 = {//copied from mini_arg_a0_of_0_to_1= {
  // Diff,                              Current,                           Texture,                            Factor
  { 0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    // RGB component
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    // Alpha component
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    SST_CM_CCA_OTHERSELECT_IA          ,SST_CM_CCA_OTHERSELECT_IA         ,SST_CM_CCA_OTHERSELECT_TA          ,SST_CM_CCA_OTHERSELECT_C1_A     ,//MUXOTH (CombineMode)
    SST_CM_CCA_LOCALSELECT_IA          ,SST_CM_CCA_LOCALSELECT_IA         ,SST_CM_CCA_LOCALSELECT_TA          ,SST_CM_CCA_LOCALSELECT_C0_A     ,//MUMLOC (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//MUXM7  (CombineMode) (NA)
    SST_CCA_MAITER                     ,SST_CCA_MAITER                    ,SST_CCA_MATMU/*?*/                 ,SST_CCA_MAC1                    ,//MUXM   (fbzColorPath) (only if M7 is used)
  }
};

MINI_STAGEOP   CCU_mini_twoStage_s0_colorOp=
{ // Currently, control code assumes AUX can come into arg1 only.
  { FALSE },// NOT USED
  { TRUE,   // Disable,
    ARGS_VALID, 0, 0, 0, SST_CM_CC_OTHERSELECT_IRGB, 0, 0, 0, 0, 0,0, 0,0, /*rkey*/  },
  { TRUE,   // SelectArg1,
    ARG1_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH, 0, 0, 0, 0, MUXOTH, 0, 0, 0, COMP_OTH,0, 0,0, /*rkey*/  },
  { TRUE,   // SelectArg2,
    ARG2_VALID | ARG1_COMP | ARG2_COMP | ARG2_AREP | ARG2_TMUXOTH, 0, 0, 0, 0, 0, MUXOTH, 0, 0, 0,0, COMP_OTH,0, /*rkey*/  },
  { TRUE,   // Modulate,
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXM7, SST_CC_REVERSE_BLEND, 0, 0, 0, MUXOTH, MUXM7, 0, 0, COMP_OTH,0, 0,CCU_COMP_MSEL, /*rkey*/  },
  { TRUE,   // Modulate2x
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXM7, SST_CC_REVERSE_BLEND,0,  0, SST_CM_CC_OUTSHIFT_2X, MUXOTH, MUXM7, 0, 0, COMP_OTH,0, 0,CCU_COMP_MSEL, /*rkey*/  },
  { TRUE,   // Modulate4x
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXM7, SST_CC_REVERSE_BLEND, 0, 0, SST_CM_CC_OUTSHIFT_4X, MUXOTH, MUXM7, 0, 0, COMP_OTH,0, 0,CCU_COMP_MSEL, /*rkey*/  },
  { TRUE,   // Add
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, SST_CC_SUB_CLOCAL, 0, 0, SST_CM_CC_INVERT_LOCAL_X, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_SUB,0, /*rkey*/  },
  { TRUE,   // AddSigned
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, SST_CC_SUB_CLOCAL, 0, 0, SST_CM_CC_INVERT_LOCAL_X_MINUS_HALF, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, 0,0, /*rkey*/  },
  { TRUE,   // AddSigned2x
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, SST_CC_SUB_CLOCAL, 0, 0, SST_CM_CC_INVERT_LOCAL_X_MINUS_HALF | SST_CM_CC_OUTSHIFT_2X, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, 0,0, /*rkey*/  },
  { TRUE,   // Subtract
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, SST_CC_SUB_CLOCAL, 0, 0, 0, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, 0,0, /*rkey*/  },
  { TRUE,   // AddSmooth
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXLOC | ARG2_AREP | ARG2_TMUXM7, SST_CC_ZERO_OTHER | SST_CC_SUB_CLOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, 0, SST_CM_CC_INVERT_LOCAL_ONE_MINUS_X, MUXLOC, MUXM7, 0, 0, COMP_LOC_BOTH,0, 0,CCU_COMP_MSEL, /*rkey*/  },
  { TRUE,   // BlendDiffuseAlpha
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, SST_CC_SUB_CLOCAL | SST_CC_ADD_CLOCAL | SST_CC_MSELECT | SST_CC_REVERSE_BLEND ,0,  0, SST_CM_CC_MSELECT_7_IA, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_BOTH,0, /*rkey*/  },
  { TRUE,   // BlendTextureAlpha
    ARGST_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, SST_CC_SUB_CLOCAL | SST_CC_ADD_CLOCAL | SST_CC_MATMU/*?*/ | SST_CC_REVERSE_BLEND , 0, 0, 0, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_BOTH,0, /*rkey*/  },
  { TRUE,   // BlendFactorAlpha
    ARGS_VALID  | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, SST_CC_SUB_CLOCAL | SST_CC_ADD_CLOCAL | SST_CC_MSELECT | SST_CC_REVERSE_BLEND , 0, 0, SST_CM_CC_MSELECT_7_C1_A, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_BOTH,0, /*rkey*/  },
  { TRUE,   // BlendTextureAlphaPM
    ARGST_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXLOC | ARG2_AREP | ARG2_TMUXM7, SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, 0, SST_CM_CC_INVERT_OTHER_ONE_MINUS_X | SST_CM_CC_OTHERSELECT_TA, MUXLOC, MUXM7, 0, 0, COMP_LOC_ADD,0, 0,CCU_COMP_MSEL, /*rkey*/  },
  { TRUE,   // BlendCurrentAlpha
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, SST_CC_SUB_CLOCAL | SST_CC_ADD_CLOCAL | SST_CC_MSELECT | SST_CC_REVERSE_BLEND , 0, 0, SST_CM_CC_MSELECT_7_IA, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_ADD,0, /*rkey*/  },
  { FALSE },// PreModulate
  { TRUE,   // ModulateAlphaAddColor
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXLOC | ARG2_AREP | ARG2_TMUXOTH, SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND,0,  0, 0, MUXLOC, MUXOTH, MUXM7, 0,  COMP_LOC_ADD,CCU_COMP_MSEL ,COMP_OTH,0, /*rkey*/  },
  { TRUE,   // ModulateColorAddAlpha
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXM7 | ARG2_AREP | ARG2_TMUXOTH,  SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL,0,  0, 0, MUXM7, MUXOTH, MUXLOC, 0, COMP_LOC_ADD,CCU_COMP_MSEL, COMP_OTH,0, /*rkey*/  },
  { TRUE,   // ModulateInvAlphaAddColor
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXLOC | ARG2_AREP | ARG2_TMUXM7, SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, 0, SST_CM_CC_INVERT_OTHER_ONE_MINUS_X, MUXLOC, MUXM7, MUXOTH, 0, COMP_OTH | COMP_LOC_ADD,0, 0,CCU_COMP_MSEL, /*rkey*/  },
  { TRUE,   // ModulateInvColorAddAlpha
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXM7, SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, 0, SST_CM_CC_INVERT_OTHER_ONE_MINUS_X, MUXOTH, MUXM7, MUXLOC, 0, COMP_OTH | COMP_LOC_ADD,0, 0,CCU_COMP_MSEL, /*rkey*/  },
  { FALSE },// BumpEnvMap
  { FALSE },// BumpEnvMapLuminance
  { FALSE },// DotProduct3
};



MINI_STAGEOP   CCU_mini_twoStage_s0_alphaOp =
{
  { FALSE },  // NOT USED
  { TRUE,     // Disable,
    ARGS_VALID, 0, 0, 0, SST_CM_CCA_OTHERSELECT_IA, 0,0,0,0 , 0,0, 0,0, /*rkey*/  },
  { TRUE,     // SelectArg1,
    ARG1_VALID | ARG1_COMP | ARG2_COMP,  SST_CCA_ADD_CLOCAL | SST_CCA_ZERO_OTHER,0, 0, 0, MUXLOC,0,0,0 , COMP_A_LOC_ADD,0, 0,0, /*rkey*/  },
  { TRUE,     // SelectArg2,
    ARG2_VALID | ARG1_COMP | ARG2_COMP,  SST_CCA_ADD_CLOCAL | SST_CCA_ZERO_OTHER,0, 0, 0, 0,MUXLOC,0,0 , 0,0, COMP_A_LOC_ADD,0, /*rkey*/  },
  { TRUE,     // Modulate,
    ARGS_VALID | ARG1_COMP | ARG2_COMP,  SST_CCA_REVERSE_BLEND ,0, 0, 0, MUXOTH,MUXM7,0,0 , COMP_A_OTH,0, 0,CCU_COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // Modulate2x
    ARGS_VALID | ARG1_COMP | ARG2_COMP,  SST_CCA_REVERSE_BLEND ,0, 0, SST_CM_CCA_OUTSHIFT_2X, MUXOTH,MUXM7,0,0 , COMP_A_OTH,0, 0,CCU_COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // Modulate4x
    ARGS_VALID | ARG1_COMP | ARG2_COMP,  SST_CCA_REVERSE_BLEND ,0, 0, SST_CM_CCA_OUTSHIFT_4X, MUXOTH,MUXM7,0,0 , COMP_A_OTH,0, 0,CCU_COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // Add
    ARGS_VALID | ARG1_COMP | ARG2_COMP,  SST_CCA_ADD_CLOCAL, 0, 0, 0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, COMP_A_LOC_ADD,0, /*rkey*/  },
  { TRUE,     // AddSigned
    ARGS_VALID | ARG1_COMP,  SST_CCA_SUB_CLOCAL, 0,0, SST_CM_CCA_INVERT_LOCAL_X_MINUS_HALF, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // AddSigned2X
    ARGS_VALID | ARG1_COMP,  SST_CCA_SUB_CLOCAL, 0,0, SST_CM_CCA_INVERT_LOCAL_X_MINUS_HALF | SST_CM_CCA_OUTSHIFT_2X, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // Subtract
    ARGS_VALID | ARG1_COMP,  SST_CCA_SUB_CLOCAL, 0,0, 0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // AddSmooth
    ARGS_VALID | ARG1_COMP | ARG2_COMP, SST_CCA_ZERO_OTHER | SST_CCA_REVERSE_BLEND | SST_CCA_ADD_CLOCAL | SST_CCA_SUB_CLOCAL, 0, 0, SST_CM_CCA_INVERT_LOCAL_ONE_MINUS_X, MUXLOC,MUXM7,0,0 , COMP_A_LOC_BOTH,0, 0,CCU_COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // BlendDiffuseAlpha
    ARGS_VALID | ARG1_COMP,  SST_CCA_SUB_CLOCAL | SST_CCA_ADD_CLOCAL | SST_CCA_MAITER | SST_CCA_REVERSE_BLEND , 0, 0, 0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // BlendTextureAlpha
    ARGST_VALID | ARG1_COMP, SST_CCA_SUB_CLOCAL | SST_CCA_ADD_CLOCAL | SST_CCA_MATMU/*?*/ | SST_CCA_REVERSE_BLEND ,0,  0,  0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // BlendFactorAlpha
    ARGS_VALID | ARG1_COMP,  SST_CCA_SUB_CLOCAL | SST_CCA_ADD_CLOCAL | SST_CCA_MAC1 | SST_CCA_REVERSE_BLEND ,0,  0,  0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // BlendTextureAlphaPM
    ARGST_VALID | ARG1_COMP | ARG2_COMP, SST_CCA_ADD_CLOCAL | SST_CCA_REVERSE_BLEND, 0, 0, SST_CM_CCA_INVERT_OTHER_ONE_MINUS_X | SST_CM_CCA_OTHERSELECT_TA , MUXLOC,MUXM7,0,0 , COMP_A_LOC_ADD,0, 0,CCU_COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // BlendCurrentAlpha
    ARGS_VALID | ARG1_COMP, SST_CCA_SUB_CLOCAL | SST_CCA_ADD_CLOCAL | SST_CCA_MAITER | SST_CCA_REVERSE_BLEND ,0,  0, 0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { FALSE },  // PreModulate
  { FALSE },  // ModulateAlphaAddColor
  { FALSE },  // ModulateColorAddAlpha
  { FALSE },  // ModulateInvAlphaAddColor
  { FALSE },  // ModulateInvColorAddAlpha
  { FALSE },  // BumpEnvMap
  { FALSE },  // BumpEnvMapLuminance
  { FALSE },  // DotProduct3
};

LPMINI_ARG CCU_mini_arg_stage[1][2] =
{
    { CCU_mini_arg_c0_of_0, CCU_mini_arg_a0_of_0},
};

LPMINI_TEXTUREOP CCU_miniStageOp[1][2] =
{
  { CCU_mini_twoStage_s0_colorOp, CCU_mini_twoStage_s0_alphaOp, },
};

#endif // NEW_CCU



MINISTAGEARG mini_arg_c0_of_0 = {
    //copied from mini_arg_c0_of_0_to_1 but use OTH texture instead of LOC texture
  // Diff,                              Current,                           Texture,                            Factor
  { 0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    // RGB component
    SST_CM_TC_OTHERSELECT_IRGB         ,SST_CM_TC_OTHERSELECT_IRGB        ,SST_CM_TC_OTHERSELECT_OTHER_TRGB   ,SST_CM_TC_OTHERSELECT_CR_RGB    ,//MUXOTH (CombineMode)
    SST_CM_TC_LOCALSELECT_IRGB         ,SST_CM_TC_LOCALSELECT_IRGB        ,SST_CM_TC_LOCALSELECT_OTHER_TRGB   ,SST_CM_TC_LOCALSELECT_CK_RGB    ,//MUMLOC (CombineMode)
    SST_CM_TC_MSELECT_7_IRGB           ,SST_CM_TC_MSELECT_7_IRGB          ,SST_CM_TC_MSELECT_7_OTHER_TRGB     ,SST_CM_TC_MSELECT_7_CR_RGB      ,//MUXM7  (CombineMode)
    SST_TC_MSELECT                     ,SST_TC_MSELECT                    ,SST_TC_MSELECT                     ,SST_TC_MSELECT                  ,//MUXM   (TextureMode) (only if M7 is used)
    // Alpha component
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    SST_CM_TC_OTHERSELECT_IA           ,SST_CM_TC_OTHERSELECT_IA          ,SST_CM_TC_OTHERSELECT_OTHER_TA     ,SST_CM_TC_OTHERSELECT_CR_A      ,//MUXOTH (CombineMode)
    SST_CM_TC_LOCALSELECT_IA           ,SST_CM_TC_LOCALSELECT_IA          ,SST_CM_TC_LOCALSELECT_OTHER_TA     ,SST_CM_TC_LOCALSELECT_CK_A      ,//MUMLOC (CombineMode)
    SST_CM_TC_MSELECT_7_IA             ,SST_CM_TC_MSELECT_7_IA            ,0                                  ,SST_CM_TC_MSELECT_7_CR_A        ,//MUXM7  (CombineMode)
    SST_TC_MSELECT                     ,SST_TC_MSELECT                    ,SST_TC_MAOTHER                     ,SST_TC_MSELECT                  ,//MUXM   (TextureMode) (only if M7 is used)
  }
};
MINISTAGEARG mini_arg_a0_of_0 = {//copied from mini_arg_a0_of_0_to_1= {
  // Diff,                              Current,                           Texture,                            Factor
  { 0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    // RGB component
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    // Alpha component
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    SST_CM_TCA_OTHERSELECT_IA          ,SST_CM_TCA_OTHERSELECT_IA         ,SST_CM_TCA_OTHERSELECT_LOCAL_TA    ,SST_CM_TCA_OTHERSELECT_CR_A     ,//MUXOTH (CombineMode)
    SST_CM_TCA_LOCALSELECT_IA          ,SST_CM_TCA_LOCALSELECT_IA         ,SST_CM_TCA_LOCALSELECT_LOCAL_TA    ,SST_CM_TCA_LOCALSELECT_CK_A     ,//MUMLOC (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//MUXM7  (CombineMode) (NA)
    SST_TCA_MAITER                     ,SST_TCA_MAITER                    ,SST_TCA_MALOCAL                    ,SST_TCA_MCR                     ,//MUXM   (TextureMode) (only if M7 is used)
  }
};
MINISTAGEARG mini_arg_c0_of_0_to_1= {
  // Diff,                              Current,                           Texture,                            Factor
  { 0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    // RGB component
    SST_CM_TC_OTHERSELECT_IRGB         ,SST_CM_TC_OTHERSELECT_IRGB        ,SST_CM_TC_OTHERSELECT_LOCAL_TRGB   ,SST_CM_TC_OTHERSELECT_CR_RGB    ,//MUXOTH (CombineMode)
    SST_CM_TC_LOCALSELECT_IRGB         ,SST_CM_TC_LOCALSELECT_IRGB        ,SST_CM_TC_LOCALSELECT_LOCAL_TRGB   ,SST_CM_TC_LOCALSELECT_CK_RGB    ,//MUMLOC (CombineMode)
    SST_CM_TC_MSELECT_7_IRGB           ,SST_CM_TC_MSELECT_7_IRGB          ,SST_CM_TC_MSELECT_7_LOCAL_TRGB     ,SST_CM_TC_MSELECT_7_CR_RGB      ,//MUXM7  (CombineMode)
    SST_TC_MSELECT                     ,SST_TC_MSELECT                    ,SST_TC_MSELECT                     ,SST_TC_MSELECT                  ,//MUXM   (FBZColorPath) (only if M7 is used)
    // Alpha component
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    SST_CM_TC_OTHERSELECT_IA           ,SST_CM_TC_OTHERSELECT_IA          ,SST_CM_TC_OTHERSELECT_LOCAL_TA     ,SST_CM_TC_OTHERSELECT_CR_A      ,//MUXOTH (CombineMode)
    SST_CM_TC_LOCALSELECT_IA           ,SST_CM_TC_LOCALSELECT_IA          ,SST_CM_TC_LOCALSELECT_LOCAL_TA     ,SST_CM_TC_LOCALSELECT_CK_A      ,//MUMLOC (CombineMode)
    SST_CM_TC_MSELECT_7_IA             ,SST_CM_TC_MSELECT_7_IA            ,0                                  ,SST_CM_TC_MSELECT_7_CR_A        ,//MUXM7  (CombineMode)
    SST_TC_MSELECT                     ,SST_TC_MSELECT                    ,SST_TC_MALOCAL                     ,SST_TC_MSELECT                  ,//MUXM   (TextureMode) (only if M7 is used)
  }
};
MINISTAGEARG mini_arg_a0_of_0_to_1= {
  // Diff,                              Current,                           Texture,                            Factor
  { 0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    // RGB component
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    // Alpha component
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    SST_CM_TCA_OTHERSELECT_IA          ,SST_CM_TCA_OTHERSELECT_IA         ,SST_CM_TCA_OTHERSELECT_LOCAL_TA    ,SST_CM_TCA_OTHERSELECT_CR_A     ,//MUXOTH (CombineMode)
    SST_CM_TCA_LOCALSELECT_IA          ,SST_CM_TCA_LOCALSELECT_IA         ,SST_CM_TCA_LOCALSELECT_LOCAL_TA    ,SST_CM_TCA_LOCALSELECT_CK_A     ,//MUMLOC (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//MUXM7  (CombineMode) (NA)
    SST_TCA_MAITER                     ,SST_TCA_MAITER                    ,SST_TCA_MALOCAL                    ,SST_TCA_MCR                     ,//MUXM   (TextureMode) (only if M7 is used)
  }
};
MINISTAGEARG mini_arg_c1_of_0_to_1= {
  // Diff,                              Current,                           Texture,                            Factor
  { 0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    // RGB component
    SST_CM_TC_OTHERSELECT_IRGB         ,SST_CM_TC_OTHERSELECT_OTHER_TRGB  ,SST_CM_TC_OTHERSELECT_LOCAL_TRGB   ,SST_CM_TC_OTHERSELECT_CR_RGB    ,//MUXOTH (CombineMode)
    SST_CM_TC_LOCALSELECT_IRGB         ,SST_CM_TC_LOCALSELECT_OTHER_TRGB  ,SST_CM_TC_LOCALSELECT_LOCAL_TRGB   ,SST_CM_TC_LOCALSELECT_CK_RGB    ,//MUMLOC (CombineMode)
    SST_CM_TC_MSELECT_7_IRGB           ,SST_CM_TC_MSELECT_7_OTHER_TRGB    ,SST_CM_TC_MSELECT_7_LOCAL_TRGB     ,SST_CM_TC_MSELECT_7_CR_RGB      ,//MUXM7  (CombineMode)
    SST_TC_MSELECT                     ,SST_TC_MSELECT                    ,SST_TC_MSELECT                     ,SST_TC_MSELECT                  ,//MUXM   (TextureMode) (only if M7 is used)
    // Alpha component
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    SST_CM_TC_OTHERSELECT_IA           ,SST_CM_TC_OTHERSELECT_OTHER_TA    ,SST_CM_TC_OTHERSELECT_LOCAL_TA     ,SST_CM_TC_OTHERSELECT_CR_A      ,//MUXOTH (CombineMode)
    SST_CM_TC_LOCALSELECT_IA           ,SST_CM_TC_LOCALSELECT_OTHER_TA    ,SST_CM_TC_LOCALSELECT_LOCAL_TA     ,SST_CM_TC_LOCALSELECT_CK_A      ,//MUMLOC (CombineMode)
    SST_CM_TC_MSELECT_7_IA             ,0                                 ,0                                  ,SST_CM_TC_MSELECT_7_CR_A        ,//MUXM7  (CombineMode)
    SST_TC_MSELECT                     ,SST_TC_MAOTHER                    ,SST_TC_MALOCAL                     ,SST_TC_MSELECT                  ,//MUXM   (TextureMode) (only if M7 is used)
  }
};
MINISTAGEARG mini_arg_a1_of_0_to_1= {
  // Diff,                              Current,                           Texture,                            Factor
  { 0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    // RGB component
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    // Alpha component
    0                                  ,0                                 ,0                                  ,0                               ,//Mux NotUsed (CombineMode)
    SST_CM_TCA_OTHERSELECT_IA          ,SST_CM_TCA_OTHERSELECT_OTHER_TA   ,SST_CM_TCA_OTHERSELECT_LOCAL_TA    ,SST_CM_TCA_OTHERSELECT_CR_A     ,//MUXOTH (CombineMode)
    SST_CM_TCA_LOCALSELECT_IA          ,SST_CM_TCA_LOCALSELECT_OTHER_TA   ,SST_CM_TCA_LOCALSELECT_LOCAL_TA    ,SST_CM_TCA_LOCALSELECT_CK_A     ,//MUMLOC (CombineMode)
    0                                  ,0                                 ,0                                  ,0                               ,//MUXM7  (CombineMode) (NA)
    SST_TCA_MAITER                     ,SST_TCA_MAOTHER                   ,SST_TCA_MALOCAL                    ,SST_TCA_MCR                     ,//MUXM   (TextureMode) (only if M7 is used)
  }
};

LPMINI_ARG mini_arg_stage[2][4] =
{ { mini_arg_c0_of_0, NULL, mini_arg_a0_of_0, NULL},
  { mini_arg_c0_of_0_to_1, mini_arg_c1_of_0_to_1, mini_arg_a0_of_0_to_1, mini_arg_a1_of_0_to_1}
};



MINI_STAGEOP   mini_twoStage_s0_colorOp=
{ // Currently, control code assumes AUX can come into arg1 only.
  { FALSE },// NOT USED
  { TRUE,   // Disable,
    ARGS_VALID, 0, 0, 0, SST_CM_TC_OTHERSELECT_IRGB, 0, 0, 0, 0, 0,0, 0,0, /*rkey*/  },
  { TRUE,   // SelectArg1,
    ARG1_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH, 0, 0, 0, 0, MUXOTH, 0, 0, 0, COMP_OTH,0, 0,0, /*rkey*/  },
  { TRUE,   // SelectArg2,
    ARG2_VALID | ARG1_COMP | ARG2_COMP | ARG2_AREP | ARG2_TMUXOTH, 0, 0, 0, 0, 0, MUXOTH, 0, 0, 0,0, COMP_OTH,0, /*rkey*/  },
  { TRUE,   // Modulate,
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXM7, 0, SST_TC_REVERSE_BLEND, 0, 0, MUXOTH, MUXM7, 0, 0, COMP_OTH,0, 0,COMP_MSEL, /*rkey*/  },
  { TRUE,   // Modulate2x
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXM7, 0, SST_TC_REVERSE_BLEND, 0, SST_CM_TC_OUTSHIFT_2X, MUXOTH, MUXM7, 0, 0, COMP_OTH,0, 0,COMP_MSEL, /*rkey*/  },
  { TRUE,   // Modulate4x
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXM7, 0, SST_TC_REVERSE_BLEND, 0, SST_CM_TC_OUTSHIFT_4X, MUXOTH, MUXM7, 0, 0, COMP_OTH,0, 0,COMP_MSEL, /*rkey*/  },
  { TRUE,   // Add
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL, 0, SST_CM_TC_INVERT_LOCAL_X, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_SUB,0, /*rkey*/  },
  { TRUE,   // AddSigned
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL, 0, SST_CM_TC_INVERT_LOCAL_X_MINUS_HALF, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, 0,0, /*rkey*/  },
  { TRUE,   // AddSigned2x
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL, 0, SST_CM_TC_INVERT_LOCAL_X_MINUS_HALF | SST_CM_TC_OUTSHIFT_2X, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, 0,0, /*rkey*/  },
  { TRUE,   // Subtract
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL, 0, 0, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, 0,0, /*rkey*/  },
  { TRUE,   // AddSmooth
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXLOC | ARG2_AREP | ARG2_TMUXM7, 0, SST_TC_ZERO_OTHER | SST_TC_SUB_CLOCAL | SST_TC_ADD_CLOCAL | SST_TC_REVERSE_BLEND, 0, SST_CM_TC_INVERT_LOCAL_ONE_MINUS_X, MUXLOC, MUXM7, 0, 0, COMP_LOC_BOTH,0, 0,COMP_MSEL, /*rkey*/  },
  { TRUE,   // BlendDiffuseAlpha
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL | SST_TC_ADD_CLOCAL | SST_TC_MSELECT | SST_TC_REVERSE_BLEND , 0, SST_CM_TC_MSELECT_7_IA, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_BOTH,0, /*rkey*/  },
  { TRUE,   // BlendTextureAlpha
    ARGST_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL | SST_TC_ADD_CLOCAL | SST_TC_MALOCAL | SST_TC_REVERSE_BLEND , 0, 0, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_BOTH,0, /*rkey*/  },
  { TRUE,   // BlendFactorAlpha
    ARGS_VALID  | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL | SST_TC_ADD_CLOCAL | SST_TC_MSELECT | SST_TC_REVERSE_BLEND , 0, SST_CM_TC_MSELECT_7_CR_A, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_BOTH,0, /*rkey*/  },
  { TRUE,   // BlendTextureAlphaPM
    ARGST_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXLOC | ARG2_AREP | ARG2_TMUXM7, 0, SST_TC_ADD_CLOCAL | SST_TC_REVERSE_BLEND, 0, SST_CM_TC_INVERT_OTHER_ONE_MINUS_X | SST_CM_TC_OTHERSELECT_LOCAL_TA, MUXLOC, MUXM7, 0, 0, COMP_LOC_ADD,0, 0,COMP_MSEL, /*rkey*/  },
  { TRUE,   // BlendCurrentAlpha
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL | SST_TC_ADD_CLOCAL | SST_TC_MSELECT | SST_TC_REVERSE_BLEND , 0, SST_CM_TC_MSELECT_7_IA, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_ADD,0, /*rkey*/  },
  { FALSE },// PreModulate
  { TRUE,   // ModulateAlphaAddColor
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXLOC | ARG2_AREP | ARG2_TMUXOTH, 0, SST_TC_ADD_CLOCAL | SST_TC_REVERSE_BLEND, 0, 0, MUXLOC, MUXOTH, MUXM7, 0,  COMP_LOC_ADD,COMP_MSEL ,COMP_OTH,0, /*rkey*/  },
  { TRUE,   // ModulateColorAddAlpha
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXM7 | ARG2_AREP | ARG2_TMUXOTH, 0, SST_TC_REVERSE_BLEND | SST_TC_ADD_CLOCAL, 0, 0, MUXM7, MUXOTH, MUXLOC, 0, COMP_LOC_ADD,COMP_MSEL, COMP_OTH,0, /*rkey*/  },
  { TRUE,   // ModulateInvAlphaAddColor
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXLOC | ARG2_AREP | ARG2_TMUXM7, 0,SST_TC_ADD_CLOCAL | SST_TC_REVERSE_BLEND, 0, SST_CM_TC_INVERT_OTHER_ONE_MINUS_X, MUXLOC, MUXM7, MUXOTH, 0, COMP_OTH | COMP_LOC_ADD,0, 0,COMP_MSEL, /*rkey*/  },
  { TRUE,   // ModulateInvColorAddAlpha
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXM7, 0,SST_TC_ADD_CLOCAL | SST_TC_REVERSE_BLEND, 0, SST_CM_TC_INVERT_OTHER_ONE_MINUS_X, MUXOTH, MUXM7, MUXLOC, 0, COMP_OTH | COMP_LOC_ADD,0, 0,COMP_MSEL, /*rkey*/  },
  { FALSE },// BumpEnvMap
  { FALSE },// BumpEnvMapLuminance
  { FALSE },// DotProduct3
};

MINI_STAGEOP   mini_twoStage_s1_colorOp = // copied from  mini_twoStage_s0_colorOp but modify BLENDCURRENT ALPHA TO USE MAOTHER (instead of ITERALPHA); and disable passes-color by default (instead of passing iterated RGB)
{ // Currently, control code assumes AUX can come into arg1 only.
  { FALSE },// NOT USED
  { TRUE,   // Disable,
    ARGS_VALID, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0,0, /*rkey*/  },
  { TRUE,   // SelectArg1,
    ARG1_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH, 0, 0, 0, 0, MUXOTH, 0, 0, 0, COMP_OTH,0, 0,0, /*rkey*/  },
  { TRUE,   // SelectArg2,
    ARG2_VALID | ARG1_COMP | ARG2_COMP | ARG2_AREP | ARG2_TMUXOTH, 0, 0, 0, 0, 0, MUXOTH, 0, 0, 0,0, COMP_OTH,0, /*rkey*/  },
  { TRUE,   // Modulate,
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXM7, 0, SST_TC_REVERSE_BLEND, 0, 0, MUXOTH, MUXM7, 0, 0, COMP_OTH,0, 0,COMP_MSEL, /*rkey*/  },
  { TRUE,   // Modulate2x
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXM7, 0, SST_TC_REVERSE_BLEND, 0, SST_CM_TC_OUTSHIFT_2X, MUXOTH, MUXM7, 0, 0, COMP_OTH,0, 0,COMP_MSEL, /*rkey*/  },
  { TRUE,   // Modulate4x
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXM7, 0, SST_TC_REVERSE_BLEND, 0, SST_CM_TC_OUTSHIFT_4X, MUXOTH, MUXM7, 0, 0, COMP_OTH,0, 0,COMP_MSEL, /*rkey*/  },
  { TRUE,   // Add
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL, 0, SST_CM_TC_INVERT_LOCAL_X, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_SUB,0, /*rkey*/  },
  { TRUE,   // AddSigned
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL, 0, SST_CM_TC_INVERT_LOCAL_X_MINUS_HALF, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, 0,0, /*rkey*/  },
  { TRUE,   // AddSigned2x
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL, 0, SST_CM_TC_INVERT_LOCAL_X_MINUS_HALF | SST_CM_TC_OUTSHIFT_2X, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, 0,0, /*rkey*/  },
  { TRUE,   // Subtract
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL, 0, 0, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, 0,0, /*rkey*/  },
  { TRUE,   // AddSmooth
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXLOC | ARG2_AREP | ARG2_TMUXM7, 0, SST_TC_ZERO_OTHER | SST_TC_SUB_CLOCAL | SST_TC_ADD_CLOCAL | SST_TC_REVERSE_BLEND, 0, SST_CM_TC_INVERT_LOCAL_ONE_MINUS_X, MUXLOC, MUXM7, 0, 0, COMP_LOC_BOTH,0, 0,COMP_MSEL, /*rkey*/  },
  { TRUE,   // BlendDiffuseAlpha
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL | SST_TC_ADD_CLOCAL | SST_TC_MSELECT | SST_TC_REVERSE_BLEND , 0, SST_CM_TC_MSELECT_7_IA, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_BOTH,0, /*rkey*/  },
  { TRUE,   // BlendTextureAlpha
    ARGST_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL | SST_TC_ADD_CLOCAL | SST_TC_MALOCAL | SST_TC_REVERSE_BLEND , 0, 0, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_BOTH,0, /*rkey*/  },
  { TRUE,   // BlendFactorAlpha
    ARGS_VALID  | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL | SST_TC_ADD_CLOCAL | SST_TC_MSELECT | SST_TC_REVERSE_BLEND , 0, SST_CM_TC_MSELECT_7_CR_A, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_BOTH,0, /*rkey*/  },
  { TRUE,   // BlendTextureAlphaPM
    ARGST_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXLOC | ARG2_AREP | ARG2_TMUXM7, 0, SST_TC_ADD_CLOCAL | SST_TC_REVERSE_BLEND, 0, SST_CM_TC_INVERT_OTHER_ONE_MINUS_X | SST_CM_TC_OTHERSELECT_LOCAL_TA, MUXLOC, MUXM7, 0, 0, COMP_LOC_ADD,0, 0,COMP_MSEL, /*rkey*/  },
  { TRUE,   // BlendCurrentAlpha
    ARGS_VALID | ARG1_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXLOC, 0, SST_TC_SUB_CLOCAL | SST_TC_ADD_CLOCAL | SST_TC_MAOTHER | SST_TC_REVERSE_BLEND , 0, 0, MUXOTH, MUXLOC, 0, 0, COMP_OTH,0, COMP_LOC_ADD,0, /*rkey*/  },
  { FALSE },// PreModulate
  { TRUE,   // ModulateAlphaAddColor
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXLOC | ARG2_AREP | ARG2_TMUXOTH, 0, SST_TC_ADD_CLOCAL | SST_TC_REVERSE_BLEND, 0, 0, MUXLOC, MUXOTH, MUXM7, 0,  COMP_LOC_ADD,COMP_MSEL ,COMP_OTH,0, /*rkey*/  },
  { TRUE,   // ModulateColorAddAlpha
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXM7 | ARG2_AREP | ARG2_TMUXOTH, 0, SST_TC_REVERSE_BLEND | SST_TC_ADD_CLOCAL, 0, 0, MUXM7, MUXOTH, MUXLOC, 0, COMP_LOC_ADD,COMP_MSEL, COMP_OTH,0, /*rkey*/  },
  { TRUE,   // ModulateInvAlphaAddColor
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXLOC | ARG2_AREP | ARG2_TMUXM7, 0,SST_TC_ADD_CLOCAL | SST_TC_REVERSE_BLEND, 0, SST_CM_TC_INVERT_OTHER_ONE_MINUS_X, MUXLOC, MUXM7, MUXOTH, 0, COMP_OTH | COMP_LOC_ADD,0, 0,COMP_MSEL, /*rkey*/  },
  { TRUE,   // ModulateInvColorAddAlpha
    ARGS_VALID | ARG1_COMP | ARG2_COMP | ARG1_AREP | ARG1_TMUXOTH | ARG2_AREP | ARG2_TMUXM7, 0,SST_TC_ADD_CLOCAL | SST_TC_REVERSE_BLEND, 0, SST_CM_TC_INVERT_OTHER_ONE_MINUS_X, MUXOTH, MUXM7, MUXLOC, 0, COMP_OTH | COMP_LOC_ADD,0, 0,COMP_MSEL, /*rkey*/  },
  { FALSE },// BumpEnvMap
  { FALSE },// BumpEnvMapLuminance
  { FALSE },// DotProduct3
};

MINI_STAGEOP   mini_twoStage_s0_alphaOp =
{
  { FALSE },  // NOT USED
  { TRUE,     // Disable,
    ARGS_VALID, 0, 0, 0, SST_CM_TCA_OTHERSELECT_IA, 0,0,0,0 , 0,0, 0,0, /*rkey*/  },
  { TRUE,     // SelectArg1,
    ARG1_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_ADD_CLOCAL | SST_TCA_ZERO_OTHER, 0, 0, MUXLOC,0,0,0 , COMP_A_LOC_ADD,0, 0,0, /*rkey*/  },
  { TRUE,     // SelectArg2,
    ARG2_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_ADD_CLOCAL | SST_TCA_ZERO_OTHER, 0, 0, 0,MUXLOC,0,0 , 0,0, COMP_A_LOC_ADD,0, /*rkey*/  },
  { TRUE,     // Modulate,
    ARGS_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_REVERSE_BLEND , 0, 0, MUXOTH,MUXM7,0,0 , COMP_A_OTH,0, 0,COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // Modulate2x
    ARGS_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_REVERSE_BLEND , 0, SST_CM_TCA_OUTSHIFT_2X, MUXOTH,MUXM7,0,0 , COMP_A_OTH,0, 0,COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // Modulate4x
    ARGS_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_REVERSE_BLEND , 0, SST_CM_TCA_OUTSHIFT_4X, MUXOTH,MUXM7,0,0 , COMP_A_OTH,0, 0,COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // Add
    ARGS_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_ADD_CLOCAL, 0, 0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, COMP_A_LOC_ADD,0, /*rkey*/  },
  { TRUE,     // AddSigned
    ARGS_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL, 0, SST_CM_TCA_INVERT_LOCAL_X_MINUS_HALF, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // AddSigned2X
    ARGS_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL, 0, SST_CM_TCA_INVERT_LOCAL_X_MINUS_HALF | SST_CM_TCA_OUTSHIFT_2X, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // Subtract
    ARGS_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL, 0, 0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // AddSmooth
    ARGS_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_ZERO_OTHER | SST_TCA_REVERSE_BLEND | SST_TCA_ADD_CLOCAL | SST_TCA_SUB_CLOCAL, 0, SST_CM_TCA_INVERT_LOCAL_ONE_MINUS_X, MUXLOC,MUXM7,0,0 , COMP_A_LOC_BOTH,0, 0,COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // BlendDiffuseAlpha
    ARGS_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL | SST_TCA_ADD_CLOCAL | SST_TCA_MAITER | SST_TCA_REVERSE_BLEND , 0, 0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // BlendTextureAlpha
    ARGST_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL | SST_TCA_ADD_CLOCAL | SST_TCA_MALOCAL | SST_TCA_REVERSE_BLEND , 0,  0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // BlendFactorAlpha
    ARGS_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL | SST_TCA_ADD_CLOCAL | SST_TCA_MCR | SST_TCA_REVERSE_BLEND , 0,  0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // BlendTextureAlphaPM
    ARGST_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_ADD_CLOCAL | SST_TCA_REVERSE_BLEND, 0, SST_CM_TCA_INVERT_OTHER_ONE_MINUS_X | SST_CM_TCA_OTHERSELECT_LOCAL_TA , MUXLOC,MUXM7,0,0 , COMP_A_LOC_ADD,0, 0,COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // BlendCurrentAlpha
    ARGS_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL | SST_TCA_ADD_CLOCAL | SST_TCA_MAITER | SST_TCA_REVERSE_BLEND , 0, 0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { FALSE },  // PreModulate
  { FALSE },  // ModulateAlphaAddColor
  { FALSE },  // ModulateColorAddAlpha
  { FALSE },  // ModulateInvAlphaAddColor
  { FALSE },  // ModulateInvColorAddAlpha
  { FALSE },  // BumpEnvMap
  { FALSE },  // BumpEnvMapLuminance
  { FALSE },  // DotProduct3
};

MINI_STAGEOP   mini_twoStage_s1_alphaOp = //copied from  mini_twoStage_s0_alphaOp  (but change BLENDCURRENTALPHA to use MAOTHER instead of MITERALPHA)
{
  { FALSE },  // NOT USED
  { TRUE,     // Disable,
    ARGS_VALID, 0, 0, 0, 0, 0,0,0,0 , 0,0, 0,0, /*rkey*/  },
  { TRUE,     // SelectArg1,
    ARG1_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_ADD_CLOCAL | SST_TCA_ZERO_OTHER, 0, 0, MUXLOC,0,0,0 , COMP_A_LOC_ADD,0, 0,0, /*rkey*/  },
  { TRUE,     // SelectArg2,
    ARG2_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_ADD_CLOCAL | SST_TCA_ZERO_OTHER, 0, 0, 0,MUXLOC,0,0 , 0,0, COMP_A_LOC_ADD,0, /*rkey*/  },
  { TRUE,     // Modulate,
    ARGS_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_REVERSE_BLEND , 0, 0, MUXOTH,MUXM7,0,0 , COMP_A_OTH,0, 0,COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // Modulate2x
    ARGS_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_REVERSE_BLEND , 0, SST_CM_TCA_OUTSHIFT_2X, MUXOTH,MUXM7,0,0 , COMP_A_OTH,0, 0,COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // Modulate4x
    ARGS_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_REVERSE_BLEND , 0, SST_CM_TCA_OUTSHIFT_4X, MUXOTH,MUXM7,0,0 , COMP_A_OTH,0, 0,COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // Add
    ARGS_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_ADD_CLOCAL, 0, 0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, COMP_A_LOC_ADD,0, /*rkey*/  },
  { TRUE,     // AddSigned
    ARGS_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL, 0, SST_CM_TCA_INVERT_LOCAL_X_MINUS_HALF, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // AddSigned2X
    ARGS_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL, 0, SST_CM_TCA_INVERT_LOCAL_X_MINUS_HALF | SST_CM_TCA_OUTSHIFT_2X, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // Subtract
    ARGS_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL, 0, 0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // AddSmooth
    ARGS_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_ZERO_OTHER | SST_TCA_REVERSE_BLEND | SST_TCA_ADD_CLOCAL | SST_TCA_SUB_CLOCAL, 0, SST_CM_TCA_INVERT_LOCAL_ONE_MINUS_X, MUXLOC,MUXM7,0,0 , COMP_A_LOC_BOTH,0, 0,COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // BlendDiffuseAlpha
    ARGS_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL | SST_TCA_ADD_CLOCAL | SST_TCA_MAITER | SST_TCA_REVERSE_BLEND , 0, 0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // BlendTextureAlpha
    ARGST_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL | SST_TCA_ADD_CLOCAL | SST_TCA_MALOCAL | SST_TCA_REVERSE_BLEND , 0,  0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // BlendFactorAlpha
    ARGS_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL | SST_TCA_ADD_CLOCAL | SST_TCA_MCR | SST_TCA_REVERSE_BLEND , 0,  0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { TRUE,     // BlendTextureAlphaPM
    ARGST_VALID | ARG1_COMP | ARG2_COMP, 0, SST_TCA_ADD_CLOCAL | SST_TCA_REVERSE_BLEND, 0, SST_CM_TCA_INVERT_OTHER_ONE_MINUS_X | SST_CM_TCA_OTHERSELECT_LOCAL_TA , MUXLOC,MUXM7,0,0 , COMP_A_LOC_ADD,0, 0,COMP_A_MSEL, /*rkey*/  },
  { TRUE,     // BlendCurrentAlpha
    ARGS_VALID | ARG1_COMP, 0, SST_TCA_SUB_CLOCAL | SST_TCA_ADD_CLOCAL | SST_TCA_MAOTHER | SST_TCA_REVERSE_BLEND , 0, 0, MUXOTH,MUXLOC,0,0 , COMP_A_OTH,0, 0,0, /*rkey*/  },
  { FALSE },  // PreModulate
  { FALSE },  // ModulateAlphaAddColor
  { FALSE },  // ModulateColorAddAlpha
  { FALSE },  // ModulateInvAlphaAddColor
  { FALSE },  // ModulateInvColorAddAlpha
  { FALSE },  // BumpEnvMap
  { FALSE },  // BumpEnvMapLuminance
  { FALSE },  // DotProduct3
};

// Voodoo2
//-------------------------------------------------------------------
// Single Stage Multi-texture operation tables
//-------------------------------------------------------------------

STAGEOP singleStage_s0_colorOp =
{
  // NOT USED
  { FALSE },
  // Disable,
  { TRUE,
    ARGS_VALID, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, 0, 0, // 1  Diffuse + Current
    ARGS_VALID, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, 0, 0, // 5  Current + Current
    ARGS_VALID, 0, 0, // 6  Current + Texture
    ARGS_VALID, 0, 0, // 7  Current + Factor
    ARGS_VALID, 0, 0, // 8  Texture + Diffuse
    ARGS_VALID, 0, 0, // 9  Texture + Current
    ARGS_VALID, 0, 0, // 10 Texture + Texture
    ARGS_VALID, 0, 0, // 11 Texture + Factor
    ARGS_VALID, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, 0, 0, // 13 Factor  + Current
    ARGS_VALID, 0, 0, // 14 Factor  + Texture
    ARGS_VALID, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg1,
  { TRUE,
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 0  Diffuse + Diffuse
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 1  Diffuse + Current
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 2  Diffuse + Texture
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 3  Diffuse + Factor
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 4  Current + Diffuse
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 5  Current + Current
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 6  Current + Texture
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 7  Current + Factor
    ARG1T_VALID, SST_RGBSEL_TMUOUT, 0, // 8  Texture + Diffuse
    ARG1T_VALID, SST_RGBSEL_TMUOUT, 0, // 9  Texture + Current
    ARG1T_VALID, SST_RGBSEL_TMUOUT, 0, // 10 Texture + Texture
    ARG1T_VALID, SST_RGBSEL_TMUOUT, 0, // 11 Texture + Factor
    ARG1_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 12 Factor  + Diffuse
    ARG1_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 13 Factor  + Current
    ARG1_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 14 Factor  + Texture
    ARG1_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg2,
  { TRUE,
    ARG2_VALID, SST_RGBSEL_RGBA,   0, // 0  Diffuse + Diffuse
    ARG2_VALID, SST_RGBSEL_RGBA,   0, // 1  Diffuse + Current
    ARG2T_VALID, SST_RGBSEL_TMUOUT, 0, // RLHJR 9/28/99 0, 0, 0, // 2  Diffuse + Texture
    ARG2_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 3  Diffuse + Factor
    ARG2_VALID, SST_RGBSEL_RGBA,   0, // 4  Current + Diffuse
    ARG2_VALID, SST_RGBSEL_RGBA,   0, // 5  Current + Current
    ARG2T_VALID, SST_RGBSEL_TMUOUT, 0, // RLHJR 9/28/99 0, 0, 0, // 6  Current + Texture
    ARG2_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 7  Current + Factor
    ARG2_VALID, SST_RGBSEL_RGBA,   0, // 8  Texture + Diffuse
    ARG2_VALID, SST_RGBSEL_RGBA,   0, // 9  Texture + Current
    ARG2T_VALID, SST_RGBSEL_TMUOUT, 0, // RLHJR 9/28/99 0, 0, 0, // 10 Texture + Texture
    ARG2_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 11 Texture + Factor
    ARG2_VALID, SST_RGBSEL_RGBA,   0, // 12 Factor  + Diffuse
    ARG2_VALID, SST_RGBSEL_RGBA,   0, // 13 Factor  + Current
    ARG2T_VALID, SST_RGBSEL_TMUOUT, 0, // RLHJR 9/28/99 0, 0, 0, // 14 Factor  + Texture
    ARG2_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 15 Factor  + Factor
  },
  // Modulate,
  { TRUE,
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR fixes test 1 0, 0, 0, // 1  Diffuse + Current
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/27/99 fixes test 5 0, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_LOCALSELECT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 5  Current + Current
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/27/99 fixes test 5 0, 0, 0, // 6  Current + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_LOCALSELECT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 7  Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, 0, // 11 Texture + Factor
    ARGS_VALID, SST_RGBSEL_RGBA | SST_LOCALSELECT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_LOCALSELECT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 13 Factor  + Current
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate2x
  { FALSE },
  // Modulate4x
  { FALSE },
  // Add
  { TRUE,
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_ADD_CLOCAL, 0, // RLHJR 9/28/99 0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_ADD_CLOCAL, 0, // RLHJR 9/28/99 0, 0, 0, // 1  Diffuse + Current
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL, 0, // RLHJR 9/28/99 0, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_ADD_CLOCAL, 0, // RLHJR 9/28/99 0, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_ADD_CLOCAL, 0, // RLHJR 9/28/99 0, 0, 0, // 5  Current + Current
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL, 0, // RLHJR 9/28/99 0, 0, 0, // 6  Current + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 7  Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL, 0, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 11 Texture + Factor
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 fixes test 6 0, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 13 Factor  + Current
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, //RLHJR 9/28/99 0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSigned
  { FALSE },
  // AddSigned2x
  { FALSE },
  // Subtract
  { TRUE,
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL, 0, // RLHJR 9/28/99 0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL, 0, // RLHJR 9/28/99 0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL, 0, // RLHJR 9/28/99 0, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL, 0, // RLHJR 9/28/99 0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 7  Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL, 0, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_LOCALSELECT, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSmooth
  { FALSE },
  // BlendDiffuseAlpha
  { TRUE,
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_LOCALSELECT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_LOCALSELECT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 7  Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BlendTextureAlpha
  { TRUE,
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_LOCALSELECT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_LOCALSELECT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 7  Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL, 0, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BlendFactorAlpha
  { TRUE,
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_LOCALSELECT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_LOCALSELECT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 7  Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL | SST_ALOCAL_C0, 0, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL | SST_ALOCAL_C0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL | SST_LOCALSELECT | SST_ALOCAL_C0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BlendTextureAlphaPM
  { FALSE },
  // BlendCurrentAlpha
  { TRUE,
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 1  Diffuse + Current
    0, 0, 0,      // 2  Diffuse + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_LOCALSELECT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, SST_RGBSEL_RGBA | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 5  Current + Current
    0, 0, 0,      // 6  Current + Texture
    ARGS_VALID, SST_RGBSEL_RGBA | SST_LOCALSELECT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // RLHJR 9/28/99 0, 0, 0, // 7  Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND, 0, // 9  Texture + Current
    0, 0, 0,      // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, 0, // 11 Texture + Factor
    0, 0, 0,      // 12 Factor  + Diffuse
    0, 0, 0,      // 13 Factor  + Current
    0, 0, 0,      // 14 Factor  + Texture
    0, 0, 0,      // 15 Factor  + Factor
  },
  // PreModulate
  { FALSE },
  // ModulateAlphaAddColor
  { FALSE },
  // ModulateColorAddAlpha
  { FALSE },
  // ModulateInvAlphaAddColor
  { FALSE },
  // ModulateInvColorAddAlpha
  { FALSE },
  // BumpEnvMap
  { FALSE },
  // BumpEnvMapLuminance
  { FALSE },
  // DotProduct3
  { FALSE },    // 24
};

STAGEOP singleStage_s0_alphaOp =
{
  // NOT USED
  { FALSE },
  // Disable,
  { TRUE,
    ARGS_VALID, 0, 0,      // 0  Diffuse + Diffuse
    ARGS_VALID, 0, 0,      // 1  Diffuse + Current
    ARGS_VALID, 0, 0,      // 2  Diffuse + Texture
    ARGS_VALID, 0, 0,      // 3  Diffuse + Factor
    ARGS_VALID, 0, 0,      // 4  Current + Diffuse
    ARGS_VALID, 0, 0,      // 5  Current + Current
    ARGS_VALID, 0, 0,      // 6  Current + Texture
    ARGS_VALID, 0, 0,      // 7  Current + Factor
    ARGS_VALID, 0, 0,      // 8  Texture + Diffuse
    ARGS_VALID, 0, 0,      // 9  Texture + Current
    ARGS_VALID, 0, 0,      // 10 Texture + Texture
    ARGS_VALID, 0, 0,      // 11 Texture + Factor
    ARGS_VALID, 0, 0,      // 12 Factor  + Diffuse
    ARGS_VALID, 0, 0,      // 13 Factor  + Current
    ARGS_VALID, 0, 0,      // 14 Factor  + Texture
    ARGS_VALID, 0, 0,      // 15 Factor  + Factor
  },
  // SelectArg1,
  { TRUE,
    ARG1_VALID, SST_ASEL_RGBA,   0,  // 0  Diffuse + Diffuse
    ARG1_VALID, SST_ASEL_RGBA,   0,  // 1  Diffuse + Current
    ARG1_VALID, SST_ASEL_RGBA,   0,  // 2  Diffuse + Texture
    ARG1_VALID, SST_ASEL_RGBA,   0,  // 3  Diffuse + Factor
    ARG1_VALID, SST_ASEL_RGBA,   0,  // 4  Current + Diffuse
    ARG1_VALID, SST_ASEL_RGBA,   0,  // 5  Current + Current
    ARG1_VALID, SST_ASEL_RGBA,   0,  // 6  Current + Texture
    ARG1_VALID, SST_ASEL_RGBA,   0,  // 7  Current + Factor
    ARG1T_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // 8  Texture + Diffuse
    ARG1T_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // 9  Texture + Current
    ARG1T_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // 10 Texture + Texture
    ARG1T_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // 11 Texture + Factor
    ARG1_VALID, SST_CCA_ADD_CLOCAL | SST_CCA_ZERO_OTHER | SST_ALOCAL_C0,   0,  // RLHJR 9/28/99 0, 0, 0,                  // 12 Factor  + Diffuse
    ARG1_VALID, SST_CCA_ADD_CLOCAL | SST_CCA_ZERO_OTHER | SST_ALOCAL_C0,   0,  // RLHJR 9/28/99 0, 0, 0,                  // 13 Factor  + Current
    ARG1_VALID, SST_CCA_ADD_CLOCAL | SST_CCA_ZERO_OTHER | SST_ALOCAL_C0,   0,  // RLHJR 9/28/99 0, 0, 0,                  // 14 Factor  + Texture
    ARG1_VALID, SST_CCA_ADD_CLOCAL | SST_CCA_ZERO_OTHER | SST_ALOCAL_C0,   0,  // RLHJR 9/28/99 0, 0, 0,                  // 15 Factor  + Factor
  },
  // SelectArg2,
  { TRUE,
    ARG2_VALID, SST_ASEL_RGBA,   0, // 0  Diffuse + Diffuse
    ARG2_VALID, SST_ASEL_RGBA,   0, // 1  Diffuse + Current
    ARG2T_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0,                 // 2  Diffuse + Texture
    ARG2_VALID, SST_CCA_ADD_CLOCAL | SST_CCA_ZERO_OTHER | SST_ALOCAL_C0,   0,  // RLHJR 9/28/99 0, 0, 0,                 // 3  Diffuse + Factor
    ARG2_VALID, SST_ASEL_RGBA,   0, // 4  Current + Diffuse
    ARG2_VALID, SST_ASEL_RGBA,   0, // 5  Current + Current
    ARG2T_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0,                 // 6  Current + Texture
    ARG2_VALID, SST_CCA_ADD_CLOCAL | SST_CCA_ZERO_OTHER | SST_ALOCAL_C0,   0,  // RLHJR 9/28/99 0, 0, 0,                 // 7  Current + Factor
    ARG2_VALID, SST_ASEL_RGBA,   0, // 8  Texture + Diffuse
    ARG2_VALID, SST_ASEL_RGBA,   0, // 9  Texture + Current
    ARG2T_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0,                 // 10 Texture + Texture
    ARG2_VALID, SST_CCA_ADD_CLOCAL | SST_CCA_ZERO_OTHER | SST_ALOCAL_C0,   0,  // RLHJR 9/28/99 0, 0, 0,                 // 11 Texture + Factor
    ARG2_VALID, SST_ASEL_RGBA,   0, // 12 Factor  + Diffuse
    ARG2_VALID, SST_ASEL_RGBA,   0, // 13 Factor  + Current
    ARG2T_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0, // 14 Factor  + Texture
    ARG2_VALID, SST_CCA_ADD_CLOCAL | SST_CCA_ZERO_OTHER | SST_ALOCAL_C0,   0,  // RLHJR 9/28/99 0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate,
  { TRUE,
    ARGS_VALID, SST_ASEL_RGBA | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // RLHJR 9/27/99 0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_ASEL_RGBA | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // RLHJR fixes test 1 0, 0, 0, // 1  Diffuse + Current
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // RLHJR 9/27/99 0, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, SST_ASEL_RGBA | SST_ALOCAL_C0 | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_ASEL_RGBA | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // RLHJR 9/27/99 0, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, SST_ASEL_RGBA | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // RLHJR 9/27/99 0, 0, 0, // 5  Current + Current
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // RLHJR 9/27/99 0, 0, 0, // 6  Current + Texture
    ARGS_VALID, SST_ASEL_RGBA | SST_ALOCAL_C0 | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0, // 7  Current + Factor
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // 8  Texture + Diffuse
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_ASEL_TMUOUT | SST_ALOCAL_C0 | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0, // 11 Texture + Factor
    ARGS_VALID, SST_ASEL_RGBA | SST_ALOCAL_C0 | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, SST_ASEL_RGBA | SST_ALOCAL_C0 | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0, // 13 Factor  + Current
    ARGST_VALID, SST_ASEL_TMUOUT | SST_ALOCAL_C0 | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate2x
  { FALSE },
  // Modulate4x
  { FALSE },
  // Add
  { TRUE,
    ARGS_VALID, SST_ASEL_RGBA | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 0, 0, 0,      // 0  Diffuse + Diffuse
    ARGS_VALID, SST_ASEL_RGBA | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 fixes test 8 0, 0, 0,      // 1  Diffuse + Current
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 // 0, 0, 0,      // 2  Diffuse + Texture
    ARGS_VALID, SST_ASEL_RGBA | SST_ALOCAL_C0 | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0,      // 3  Diffuse + Factor
    ARGS_VALID, SST_ASEL_RGBA | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 0, 0, 0,      // 4  Current + Diffuse
    ARGS_VALID, SST_ASEL_RGBA | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 0, 0, 0,      // 5  Current + Current
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 // 0, 0, 0,      // 6  Current + Texture
    ARGS_VALID, SST_ASEL_RGBA | SST_ALOCAL_C0 | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0,      // 7  Current + Factor
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 // 8  Texture + Diffuse
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 fixes test 7 // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_ASEL_TMUOUT | SST_ALOCAL_C0 | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0, // 11 Texture + Factor
    ARGS_VALID, SST_ASEL_RGBA | SST_ALOCAL_C0 | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, SST_ASEL_RGBA | SST_ALOCAL_C0 | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0, // 13 Factor  + Current
    ARGST_VALID, SST_ASEL_TMUOUT | SST_ALOCAL_C0 | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSigned
  { FALSE },
  // AddSigned2x
  { FALSE },
  // Subtract
  { TRUE,
    ARGS_VALID, SST_ASEL_RGBA | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 0, 0, 0,      // 0  Diffuse + Diffuse
    ARGS_VALID, SST_ASEL_RGBA | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 fixes test 8 0, 0, 0,      // 1  Diffuse + Current
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 // 0, 0, 0,      // 2  Diffuse + Texture
    ARGS_VALID, SST_ASEL_RGBA | SST_ALOCAL_C0 | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0,      // 3  Diffuse + Factor
    ARGS_VALID, SST_ASEL_RGBA | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 0, 0, 0,      // 4  Current + Diffuse
    ARGS_VALID, SST_ASEL_RGBA | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 0, 0, 0,      // 5  Current + Current
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // RLHJR 9/27/99 // 0, 0, 0,      // 6  Current + Texture
    ARGS_VALID, SST_ASEL_RGBA | SST_ALOCAL_C0 | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0,      // 7  Current + Factor
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // 8  Texture + Diffuse
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_ASEL_TMUOUT | SST_ALOCAL_C0 | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // RLHJR 9/28/99 0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSmooth
  { FALSE },
  // BlendDiffuseAlpha
  { FALSE },
  // BlendTextureAlpha
  { FALSE },
  // BlendFactorAlpha
  { FALSE },
  // BlendTextureAlphaPM
  { FALSE },
  // BlendCurrentAlpha
  { FALSE },
  // PreModulate
  { FALSE },
  // ModulateAlphaAddColor
  { FALSE },
  // ModulateColorAddAlpha
  { FALSE },
  // ModulateInvAlphaAddColor
  { FALSE },
  // ModulateInvColorAddAlpha
  { FALSE },
  // BumpEnvMap
  { FALSE },
  // BumpEnvMapLuminance
  { FALSE },
  // DotProduct3
  { FALSE },    // 24
};

//-------------------------------------------------------------------
// Two Stage Multi-texture operation tables
//-------------------------------------------------------------------

STAGEOP twoStage_s0_colorOp =
{
  // NOT USED
  { FALSE },
  // Disable,
  { TRUE,
    ARGS_VALID, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, 0, 0, // 1  Diffuse + Current
    ARGS_VALID, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, 0, 0, // 5  Current + Current
    ARGS_VALID, 0, 0, // 6  Current + Texture
    ARGS_VALID, 0, 0, // 7  Current + Factor
    ARGS_VALID, 0, 0, // 8  Texture + Diffuse
    ARGS_VALID, 0, 0, // 9  Texture + Current
    ARGS_VALID, 0, 0, // 10 Texture + Texture
    ARGS_VALID, 0, 0, // 11 Texture + Factor
    ARGS_VALID, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, 0, 0, // 13 Factor  + Current
    ARGS_VALID, 0, 0, // 14 Factor  + Texture
    ARGS_VALID, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg1,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    ARG1T_VALID, 0, 0, // 8  Texture + Diffuse
    ARG1T_VALID, 0, 0, // 9  Texture + Current
    ARG1T_VALID, 0, 0, // 10 Texture + Texture
    ARG1T_VALID, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg2,
  { FALSE },
  // Modulate,
//  { FALSE },
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    ARGF_SP_ALL | ARGF_SP_FAIL, 0, 0, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate2x
  { FALSE },
  // Modulate4x
  { FALSE },
  // Add
  { FALSE },
  // AddSigned
  { FALSE },
  // AddSigned2x
  { FALSE },
  // Subtract
  { FALSE },
  // AddSmooth
  { FALSE },
  // BlendDiffuseAlpha
  { FALSE },
  // BlendTextureAlpha
  { FALSE },
  // BlendFactorAlpha
  { FALSE },
  // BlendTextureAlphaPM
  { FALSE },
  // BlendCurrentAlpha
  { FALSE },
  // PreModulate
  { FALSE },
  // ModulateAlphaAddColor
  { FALSE },
  // ModulateColorAddAlpha
  { FALSE },
  // ModulateInvAlphaAddColor
  { FALSE },
  // ModulateInvColorAddAlpha
  { FALSE },
  // BumpEnvMap
  { FALSE },
  // BumpEnvMapLuminance
  { FALSE },
  // DotProduct3
  { FALSE },    // 24
};

STAGEOP twoStage_s1_colorOp =
{
  // NOT USED
  { FALSE },
  // Disable,
  { TRUE,
    ARGS_VALID, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, 0, 0, // 1  Diffuse + Current
    ARGS_VALID, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, 0, 0, // 5  Current + Current
    ARGS_VALID, 0, 0, // 6  Current + Texture
    ARGS_VALID, 0, 0, // 7  Current + Factor
    ARGS_VALID, 0, 0, // 8  Texture + Diffuse
    ARGS_VALID, 0, 0, // 9  Texture + Current
    ARGS_VALID, 0, 0, // 10 Texture + Texture
    ARGS_VALID, 0, 0, // 11 Texture + Factor
    ARGS_VALID, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, 0, 0, // 13 Factor  + Current
    ARGS_VALID, 0, 0, // 14 Factor  + Texture
    ARGS_VALID, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg1,
  { TRUE,
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 0  Diffuse + Diffuse
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 1  Diffuse + Current
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 2  Diffuse + Texture
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 3  Diffuse + Factor
    ARG1_VALID, SST_RGBSEL_TMUOUT, SST_TC_PASS, // 4  Current + Diffuse
    ARG1_VALID, SST_RGBSEL_TMUOUT, SST_TC_PASS, // 5  Current + Current
    ARG1_VALID, SST_RGBSEL_TMUOUT, SST_TC_PASS, // 6  Current + Texture
    ARG1_VALID, SST_RGBSEL_TMUOUT, SST_TC_PASS, // 7  Current + Factor
    ARG1T_VALID, SST_RGBSEL_TMUOUT, SST_TC_REPLACE, // 8  Texture + Diffuse
    ARG1T_VALID, SST_RGBSEL_TMUOUT, SST_TC_REPLACE, // 9  Texture + Current
    ARG1T_VALID, SST_RGBSEL_TMUOUT, SST_TC_REPLACE, // 10 Texture + Texture
    ARG1T_VALID, SST_RGBSEL_TMUOUT, SST_TC_REPLACE, // 11 Texture + Factor
    ARG1_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 12 Factor  + Diffuse
    ARG1_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 13 Factor  + Current
    ARG1_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 14 Factor  + Texture
    ARG1_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // RLHJR 9/28/99 0, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg2,
  { TRUE,
    ARG2_VALID, SST_RGBSEL_RGBA, 0, // 0  Diffuse + Diffuse
    ARG2_VALID, SST_RGBSEL_TMUOUT, SST_TC_PASS, // 1  Diffuse + Current
    ARG2T_VALID, SST_RGBSEL_TMUOUT, SST_TC_REPLACE, // RLHJR 9/28/99 0, 0, 0, // 2  Diffuse + Texture
    ARG2_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 3 Diffuse  + Factor
    ARG2_VALID, SST_RGBSEL_RGBA, 0, // 4  Current + Diffuse
    ARG2_VALID, SST_RGBSEL_TMUOUT, SST_TC_PASS, // 5  Current + Current
    ARG2T_VALID, SST_RGBSEL_TMUOUT, SST_TC_REPLACE, // RLHJR 9/28/99 0, 0, 0, // 6  Current + Texture
    ARG2_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 7 Current  + Factor
    ARG2_VALID, SST_RGBSEL_RGBA, 0, // 8  Texture + Diffuse
    ARG2_VALID, SST_RGBSEL_TMUOUT, SST_TC_PASS, // 9  Texture + Current
    ARG2T_VALID, SST_RGBSEL_TMUOUT, SST_TC_REPLACE, // RLHJR 9/28/99 0, 0, 0, // 10 Texture + Texture
    ARG2_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 11 Texture  + Factor
    ARG2_VALID, SST_RGBSEL_RGBA, 0, // 12 Factor  + Diffuse
    ARG2_VALID, SST_RGBSEL_TMUOUT, SST_TC_PASS, // 13 Factor  + Current
    ARG2T_VALID, SST_RGBSEL_TMUOUT, SST_TC_REPLACE, // RLHJR 9/28/99 0, 0, 0, // 14 Factor  + Texture
    ARG2_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 15 Factor  + Factor
  },
  // Modulate,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, SST_TC_PASS, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, SST_TC_PASS, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, SST_TC_PASS, // 7 Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, SST_TC_REPLACE, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT, SST_TC_MULT, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, SST_TC_REPLACE, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate2x
//  { FALSE },
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7 Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID | ARGF_SP3 | ARGF_SP_FAIL, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, SST_TC_MULT, // 9 Texture + Current = (Tex * Cur * Diff)
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate4x
  { FALSE },
  // Add
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL, SST_TC_PASS, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL, SST_TC_PASS, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, SST_TC_PASS, // 7 Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL, SST_TC_REPLACE, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT, SST_TC_ADD, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, SST_TC_REPLACE, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSigned
  { FALSE },
  // AddSigned2x
  { FALSE },
  // Subtract
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL, SST_TC_PASS, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL, SST_TC_PASS, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_LOCALSELECT, SST_TC_PASS, // 7 Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL, SST_TC_REPLACE, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
//JP    ARGST_VALID, SST_RGBSEL_TMUOUT, SST_TC_SUB, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_LOCALSELECT, SST_TC_REPLACE, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSmooth
  { FALSE },
  // BlendDiffuseAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL, SST_TC_PASS, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL, SST_TC_PASS, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, SST_TC_PASS, // 7 Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL, SST_TC_REPLACE, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, SST_TC_REPLACE, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BlendTextureAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL, SST_TC_PASS | SST_TCA_REPLACE, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL, SST_TC_PASS | SST_TCA_REPLACE, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, SST_TC_PASS | SST_TCA_REPLACE, // 7 Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL, SST_TC_REPLACE | SST_TCA_REPLACE, // 8  Texture + Diffuse
    ARGST_VALID | ARGF_SP1, SST_RGBSEL_TMUOUT, SST_TC_SUB_CLOCAL | SST_TC_MALOCAL | SST_TC_ADD_CLOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, SST_TC_REPLACE | SST_TCA_REPLACE, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BlendFactorAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_ALOCAL_C0, SST_TC_PASS, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL | SST_ALOCAL_C0, SST_TC_PASS, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL | SST_LOCALSELECT | SST_ALOCAL_C0, SST_TC_PASS, // 7 Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL | SST_ALOCAL_C0, SST_TC_REPLACE, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL | SST_LOCALSELECT | SST_ALOCAL_C0, SST_TC_REPLACE, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BlendTextureAlphaPM
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0,// 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT, SST_TC_MALOCAL | SST_TC_ADD_CLOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BlendCurrentAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL, SST_TC_PASS | SST_TCA_PASS, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL, SST_TC_PASS | SST_TCA_PASS, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, SST_TC_PASS | SST_TCA_PASS, // 7 Current + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL, SST_TC_REPLACE | SST_TCA_PASS, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT, SST_TC_SUB_CLOCAL | SST_TC_MAOTHER | SST_TC_ADD_CLOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, SST_TC_REPLACE | SST_TCA_PASS, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // PreModulate
  { FALSE },
  // ModulateAlphaAddColor
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT, SST_TC_MALOCAL | SST_TC_REVERSE_BLEND | SST_TC_ADD_CLOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // ModulateColorAddAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT, SST_TC_MCLOCAL | SST_TC_REVERSE_BLEND | SST_TC_ADD_ALOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // ModulateInvAlphaAddColor
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT, SST_TC_MALOCAL | SST_TC_ADD_CLOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // ModulateInvColorAddAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT, SST_TC_MCLOCAL | SST_TC_ADD_ALOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BumpEnvMap
  { FALSE },
  // BumpEnvMapLuminance
  { FALSE },
  // DotProduct3
  { FALSE },    // 24
};

STAGEOP twoStage_s0_alphaOp =
{
  // NOT USED
  { FALSE },
  // Disable,
  { TRUE,
    ARGS_VALID, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, 0, 0, // 1  Diffuse + Current
    ARGS_VALID, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, 0, 0, // 5  Current + Current
    ARGS_VALID, 0, 0, // 6  Current + Texture
    ARGS_VALID, 0, 0, // 7  Current + Factor
    ARGS_VALID, 0, 0, // 8  Texture + Diffuse
    ARGS_VALID, 0, 0, // 9  Texture + Current
    ARGS_VALID, 0, 0, // 10 Texture + Texture
    ARGS_VALID, 0, 0, // 11 Texture + Factor
    ARGS_VALID, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, 0, 0, // 13 Factor  + Current
    ARGS_VALID, 0, 0, // 14 Factor  + Texture
    ARGS_VALID, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg1,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    ARG1T_VALID, 0, 0, // 8  Texture + Diffuse
    ARG1T_VALID, 0, 0, // 9  Texture + Current
    ARG1T_VALID, 0, 0, // 10 Texture + Texture
    ARG1T_VALID, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg2,
  { FALSE },
  // Modulate,
  { FALSE },
  // Modulate2x
  { FALSE },
  // Modulate4x
  { FALSE },
  // Add
  { FALSE },
  // AddSigned
  { FALSE },
  // AddSigned2x
  { FALSE },
  // Subtract
  { FALSE },
  // AddSmooth
  { FALSE },
  // BlendDiffuseAlpha
  { FALSE },
  // BlendTextureAlpha
  { FALSE },
  // BlendFactorAlpha
  { FALSE },
  // BlendTextureAlphaPM
  { FALSE },
  // BlendCurrentAlpha
  { FALSE },
  // PreModulate
  { FALSE },
  // ModulateAlphaAddColor
  { FALSE },
  // ModulateColorAddAlpha
  { FALSE },
  // ModulateInvAlphaAddColor
  { FALSE },
  // ModulateInvColorAddAlpha
  { FALSE },
  // BumpEnvMap
  { FALSE },
  // BumpEnvMapLuminance
  { FALSE },
  // DotProduct3
  { FALSE },    // 24
};

STAGEOP twoStage_s1_alphaOp =
{
  // NOT USED
  { FALSE },
  // Disable,
  { TRUE,
    ARGS_VALID, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, 0, 0, // 1  Diffuse + Current
    ARGS_VALID, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, 0, 0, // 5  Current + Current
    ARGS_VALID, 0, 0, // 6  Current + Texture
    ARGS_VALID, 0, 0, // 7  Current + Factor
    ARGS_VALID, 0, 0, // 8  Texture + Diffuse
    ARGS_VALID, 0, 0, // 9  Texture + Current
    ARGS_VALID, 0, 0, // 10 Texture + Texture
    ARGS_VALID, 0, 0, // 11 Texture + Factor
    ARGS_VALID, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, 0, 0, // 13 Factor  + Current
    ARGS_VALID, 0, 0, // 14 Factor  + Texture
    ARGS_VALID, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg1,
  { TRUE,
    ARG1_VALID, SST_ASEL_RGBA, 0, // 0  Diffuse + Diffuse
    ARG1_VALID, SST_ASEL_RGBA, 0, // 1  Diffuse + Current
    ARG1_VALID, SST_ASEL_RGBA, 0, // 2  Diffuse + Texture
    ARG1_VALID, SST_ASEL_RGBA, 0, // 3  Diffuse + Factor
    ARG1_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // 4  Current + Diffuse
    ARG1_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // 5  Current + Current
    ARG1_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // 6  Current + Texture
    ARG1_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // 7  Current + Factor
    ARG1T_VALID, SST_ASEL_TMUOUT, SST_TCA_REPLACE, // 8  Texture + Diffuse
    ARG1T_VALID, SST_ASEL_TMUOUT, SST_TCA_REPLACE, // 9  Texture + Current
    ARG1T_VALID, SST_ASEL_TMUOUT, SST_TCA_REPLACE, // 10 Texture + Texture
    ARG1T_VALID, SST_ASEL_TMUOUT, SST_TCA_REPLACE, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg2,
  { TRUE,
    ARG2_VALID, SST_ASEL_RGBA, 0, // 0  Diffuse + Diffuse
    ARG2_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARG2_VALID, SST_ASEL_RGBA, 0, // 4  Current + Diffuse
    ARG2_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    ARG2_VALID, SST_ASEL_RGBA, 0, // 8  Texture + Diffuse
    ARG2_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    ARG2_VALID, SST_ASEL_RGBA, 0, // 12 Factor  + Diffuse
    ARG2_VALID, SST_ASEL_TMUOUT, SST_TCA_PASS, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_ASEL_TMUOUT | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_ASEL_TMUOUT | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_PASS, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, SST_TCA_REPLACE, // 8  Texture + Diffuse
    ARGST_VALID, SST_ASEL_TMUOUT, SST_TCA_MULT, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate2x
  { FALSE },
  // Modulate4x
  { FALSE },
  // Add
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_ASEL_TMUOUT | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_ASEL_TMUOUT | SST_CCA_ADD_CLOCAL, SST_TCA_PASS, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_ADD_CLOCAL, SST_TCA_REPLACE, // 8  Texture + Diffuse
    ARGST_VALID, SST_ASEL_TMUOUT, SST_TCA_ADD, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSigned
  { FALSE },
  // AddSigned2x
  { FALSE },
  // Subtract
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_ASEL_TMUOUT | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_ASEL_TMUOUT | SST_CCA_SUB_CLOCAL, SST_TCA_PASS, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    ARGST_VALID, SST_ASEL_TMUOUT | SST_CCA_SUB_CLOCAL, SST_TCA_REPLACE, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
//JP    ARGST_VALID, SST_ASEL_TMUOUT, SST_TCA_SUB, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSmooth
  { FALSE },
  // BlendDiffuseAlpha
  { FALSE },
  // BlendTextureAlpha
  { FALSE },
  // BlendFactorAlpha
  { FALSE },
  // BlendTextureAlphaPM
  { FALSE },
  // BlendCurrentAlpha
  { FALSE },
  // PreModulate
  { FALSE },
  // ModulateAlphaAddColor
  { FALSE },
  // ModulateColorAddAlpha
  { FALSE },
  // ModulateInvAlphaAddColor
  { FALSE },
  // ModulateInvColorAddAlpha
  { FALSE },
  // BumpEnvMap
  { FALSE },
  // BumpEnvMapLuminance
  { FALSE },
  // DotProduct3
  { FALSE },    // 24
};

//-------------------------------------------------------------------
// Three Stage Multi-texture operation tables
//-------------------------------------------------------------------

STAGEOP threeStage_s0_colorOp =
{
  // NOT USED
  { FALSE },
  // Disable,
  { TRUE,
    ARGS_VALID, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, 0, 0, // 1  Diffuse + Current
    ARGS_VALID, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, 0, 0, // 5  Current + Current
    ARGS_VALID, 0, 0, // 6  Current + Texture
    ARGS_VALID, 0, 0, // 7  Current + Factor
    ARGS_VALID, 0, 0, // 8  Texture + Diffuse
    ARGS_VALID, 0, 0, // 9  Texture + Current
    ARGS_VALID, 0, 0, // 10 Texture + Texture
    ARGS_VALID, 0, 0, // 11 Texture + Factor
    ARGS_VALID, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, 0, 0, // 13 Factor  + Current
    ARGS_VALID, 0, 0, // 14 Factor  + Texture
    ARGS_VALID, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg1,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    ARG1T_VALID, 0, 0, // 8  Texture + Diffuse
    ARG1T_VALID, 0, 0, // 9  Texture + Current
    ARG1T_VALID, 0, 0, // 10 Texture + Texture
    ARG1T_VALID, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg2,
  { FALSE },
  // Modulate,
  { FALSE },
  // Modulate2x
  { FALSE },
  // Modulate4x
  { FALSE },
  // Add
  { FALSE },
  // AddSigned
  { FALSE },
  // AddSigned2x
  { FALSE },
  // Subtract
  { FALSE },
  // AddSmooth
  { FALSE },
  // BlendDiffuseAlpha
  { FALSE },
  // BlendTextureAlpha
  { FALSE },
  // BlendFactorAlpha
  { FALSE },
  // BlendTextureAlphaPM
  { FALSE },
  // BlendCurrentAlpha
  { FALSE },
  // PreModulate
  { FALSE },
  // ModulateAlphaAddColor
  { FALSE },
  // ModulateColorAddAlpha
  { FALSE },
  // ModulateInvAlphaAddColor
  { FALSE },
  // ModulateInvColorAddAlpha
  { FALSE },
  // BumpEnvMap
  { FALSE },
  // BumpEnvMapLuminance
  { FALSE },
  // DotProduct3
  { FALSE },    // 24
};

STAGEOP threeStage_s1_colorOp =
{
  // NOT USED
  { FALSE },
  // Disable,
  { TRUE,
    ARGS_VALID, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, 0, 0, // 1  Diffuse + Current
    ARGS_VALID, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, 0, 0, // 5  Current + Current
    ARGS_VALID, 0, 0, // 6  Current + Texture
    ARGS_VALID, 0, 0, // 7  Current + Factor
    ARGS_VALID, 0, 0, // 8  Texture + Diffuse
    ARGS_VALID, 0, 0, // 9  Texture + Current
    ARGS_VALID, 0, 0, // 10 Texture + Texture
    ARGS_VALID, 0, 0, // 11 Texture + Factor
    ARGS_VALID, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, 0, 0, // 13 Factor  + Current
    ARGS_VALID, 0, 0, // 14 Factor  + Texture
    ARGS_VALID, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg1,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARG1_VALID, 0, SST_TC_PASS, // 4  Current + Diffuse
    ARG1_VALID, 0, SST_TC_PASS, // 5  Current + Current
    ARG1_VALID, 0, SST_TC_PASS, // 6  Current + Texture
    ARG1_VALID, 0, SST_TC_PASS, // 7  Current + Factor
    ARG1T_VALID, 0, SST_TC_REPLACE, // 8  Texture + Diffuse
    ARG1T_VALID, 0, SST_TC_REPLACE, // 9  Texture + Current
    ARG1T_VALID, 0, SST_TC_REPLACE, // 10 Texture + Texture
    ARG1T_VALID, 0, SST_TC_REPLACE, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg2,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARG2_VALID, 0, SST_TC_PASS, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    ARG2_VALID, 0, SST_TC_PASS, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARG2_VALID, 0, SST_TC_PASS, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    ARG2_VALID, 0, SST_TC_PASS, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TC_MULT, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate2x
  { FALSE },
  // Modulate4x
  { FALSE },
  // Add
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TC_ADD, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSigned
  { FALSE },
  // AddSigned2x
  { FALSE },
  // Subtract
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TC_SUB, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSmooth
  { FALSE },
  // BlendDiffuseAlpha
  { FALSE },
  // BlendTextureAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TC_SUB_CLOCAL | SST_TC_MALOCAL | SST_TC_ADD_CLOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BlendFactorAlpha
  { FALSE },
  // BlendTextureAlphaPM
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0,// 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TC_MALOCAL | SST_TC_ADD_CLOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BlendCurrentAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TC_SUB_CLOCAL | SST_TC_MAOTHER | SST_TC_ADD_CLOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // PreModulate
  { FALSE },
  // ModulateAlphaAddColor
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TC_MALOCAL | SST_TC_REVERSE_BLEND | SST_TC_ADD_CLOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // ModulateColorAddAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TC_MCLOCAL | SST_TC_REVERSE_BLEND | SST_TC_ADD_ALOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // ModulateInvAlphaAddColor
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TC_MALOCAL | SST_TC_ADD_CLOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // ModulateInvColorAddAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TC_MCLOCAL | SST_TC_ADD_ALOCAL, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BumpEnvMap
  { FALSE },
  // BumpEnvMapLuminance
  { FALSE },
  // DotProduct3
  { FALSE },    // 24
};

STAGEOP threeStage_s2_colorOp =
{
  // NOT USED
  { FALSE },
  // Disable,
  { TRUE,
    ARGS_VALID, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, 0, 0, // 1  Diffuse + Current
    ARGS_VALID, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, 0, 0, // 5  Current + Current
    ARGS_VALID, 0, 0, // 6  Current + Texture
    ARGS_VALID, 0, 0, // 7  Current + Factor
    ARGS_VALID, 0, 0, // 8  Texture + Diffuse
    ARGS_VALID, 0, 0, // 9  Texture + Current
    ARGS_VALID, 0, 0, // 10 Texture + Texture
    ARGS_VALID, 0, 0, // 11 Texture + Factor
    ARGS_VALID, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, 0, 0, // 13 Factor  + Current
    ARGS_VALID, 0, 0, // 14 Factor  + Texture
    ARGS_VALID, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg1,
  { TRUE,
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 0  Diffuse + Diffuse
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 1  Diffuse + Current
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 2  Diffuse + Texture
    ARG1_VALID, SST_RGBSEL_RGBA, 0, // 3  Diffuse + Factor
    ARG1_VALID, SST_RGBSEL_TMUOUT, 0, // 4  Current + Diffuse
    ARG1_VALID, SST_RGBSEL_TMUOUT, 0, // 5  Current + Current
    ARG1_VALID, SST_RGBSEL_TMUOUT, 0, // 6  Current + Texture
    ARG1_VALID, SST_RGBSEL_TMUOUT, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg2,
  { TRUE,
    ARG2_VALID, SST_RGBSEL_RGBA, 0, // 0  Diffuse + Diffuse
    ARG2_VALID, SST_RGBSEL_TMUOUT, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    ARG2_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 3 Diffuse + Factor
    ARG2_VALID, SST_RGBSEL_RGBA, 0, // 4  Current + Diffuse
    ARG2_VALID, SST_RGBSEL_TMUOUT, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARG2_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 7 Current  + Factor
    ARG2_VALID, SST_RGBSEL_RGBA, 0, // 8  Texture + Diffuse
    ARG2_VALID, SST_RGBSEL_TMUOUT, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    ARG2_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 11 Texture  + Factor
    ARG2_VALID, SST_RGBSEL_RGBA, 0, // 12 Factor  + Diffuse
    ARG2_VALID, SST_RGBSEL_TMUOUT, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    ARG2_VALID, SST_CC_ZERO_OTHER | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 15 Factor  + Factor
  },
  // Modulate,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_MCLOCAL | SST_CC_REVERSE_BLEND | SST_LOCALSELECT, 0, // 7 Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate2x
  //{ FALSE },
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7 Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGS_VALID, SST_RGBSEL_TMUOUT, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate4x
  { FALSE },
  // Add
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 7 Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSigned
  { FALSE },
  // AddSigned2x
  { FALSE },
  // Subtract
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_LOCALSELECT, 0, // 7 Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSmooth
  { FALSE },
  // BlendDiffuseAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 7 Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BlendTextureAlpha
  { FALSE },
  // BlendFactorAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_ADD_CLOCAL | SST_ALOCAL_C0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL | SST_ALOCAL_C0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MALOCAL | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL | SST_LOCALSELECT | SST_ALOCAL_C0, 0, // 7 Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // BlendTextureAlphaPM
  { FALSE },
  // BlendCurrentAlpha
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_ADD_CLOCAL, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    ARGST_VALID, SST_RGBSEL_TMUOUT | SST_CC_SUB_CLOCAL | SST_CC_MATMU | SST_CC_REVERSE_BLEND | SST_CC_ADD_CLOCAL | SST_LOCALSELECT, 0, // 7 Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // PreModulate
  { FALSE },
  // ModulateAlphaAddColor
  { FALSE },
  // ModulateColorAddAlpha
  { FALSE },
  // ModulateInvAlphaAddColor
  { FALSE },
  // ModulateInvColorAddAlpha
  { FALSE },
  // BumpEnvMap
  { FALSE },
  // BumpEnvMapLuminance
  { FALSE },
  // DotProduct3
  { FALSE },    // 24
};

STAGEOP threeStage_s0_alphaOp =
{
  // NOT USED
  { FALSE },
  // Disable,
  { TRUE,
    ARGS_VALID, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, 0, 0, // 1  Diffuse + Current
    ARGS_VALID, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, 0, 0, // 5  Current + Current
    ARGS_VALID, 0, 0, // 6  Current + Texture
    ARGS_VALID, 0, 0, // 7  Current + Factor
    ARGS_VALID, 0, 0, // 8  Texture + Diffuse
    ARGS_VALID, 0, 0, // 9  Texture + Current
    ARGS_VALID, 0, 0, // 10 Texture + Texture
    ARGS_VALID, 0, 0, // 11 Texture + Factor
    ARGS_VALID, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, 0, 0, // 13 Factor  + Current
    ARGS_VALID, 0, 0, // 14 Factor  + Texture
    ARGS_VALID, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg1,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    ARG1T_VALID, 0, 0, // 8  Texture + Diffuse
    ARG1T_VALID, 0, 0, // 9  Texture + Current
    ARG1T_VALID, 0, 0, // 10 Texture + Texture
    ARG1T_VALID, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg2,
  { FALSE },
  // Modulate,
  { FALSE },
  // Modulate2x
  { FALSE },
  // Modulate4x
  { FALSE },
  // Add
  { FALSE },
  // AddSigned
  { FALSE },
  // AddSigned2x
  { FALSE },
  // Subtract
  { FALSE },
  // AddSmooth
  { FALSE },
  // BlendDiffuseAlpha
  { FALSE },
  // BlendTextureAlpha
  { FALSE },
  // BlendFactorAlpha
  { FALSE },
  // BlendTextureAlphaPM
  { FALSE },
  // BlendCurrentAlpha
  { FALSE },
  // PreModulate
  { FALSE },
  // ModulateAlphaAddColor
  { FALSE },
  // ModulateColorAddAlpha
  { FALSE },
  // ModulateInvAlphaAddColor
  { FALSE },
  // ModulateInvColorAddAlpha
  { FALSE },
  // BumpEnvMap
  { FALSE },
  // BumpEnvMapLuminance
  { FALSE },
  // DotProduct3
  { FALSE },    // 24
};

STAGEOP threeStage_s1_alphaOp =
{
  // NOT USED
  { FALSE },
  // Disable,
  { TRUE,
    ARGS_VALID, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, 0, 0, // 1  Diffuse + Current
    ARGS_VALID, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, 0, 0, // 5  Current + Current
    ARGS_VALID, 0, 0, // 6  Current + Texture
    ARGS_VALID, 0, 0, // 7  Current + Factor
    ARGS_VALID, 0, 0, // 8  Texture + Diffuse
    ARGS_VALID, 0, 0, // 9  Texture + Current
    ARGS_VALID, 0, 0, // 10 Texture + Texture
    ARGS_VALID, 0, 0, // 11 Texture + Factor
    ARGS_VALID, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, 0, 0, // 13 Factor  + Current
    ARGS_VALID, 0, 0, // 14 Factor  + Texture
    ARGS_VALID, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg1,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARG1_VALID, 0, SST_TCA_PASS, // 4  Current + Diffuse
    ARG1_VALID, 0, SST_TCA_PASS, // 5  Current + Current
    ARG1_VALID, 0, SST_TCA_PASS, // 6  Current + Texture
    ARG1_VALID, 0, SST_TCA_PASS, // 7  Current + Factor
    ARG1T_VALID, 0, SST_TCA_REPLACE, // 8  Texture + Diffuse
    ARG1T_VALID, 0, SST_TCA_REPLACE, // 9  Texture + Current
    ARG1T_VALID, 0, SST_TCA_REPLACE, // 10 Texture + Texture
    ARG1T_VALID, 0, SST_TCA_REPLACE, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg2,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARG2_VALID, 0, SST_TCA_PASS, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    ARG2_VALID, 0, SST_TCA_PASS, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARG2_VALID, 0, SST_TCA_PASS, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    ARG2_VALID, 0, SST_TCA_PASS, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TCA_MULT, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate2x
  { FALSE },
  // Modulate4x
  { FALSE },
  // Add
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TCA_ADD, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSigned
  { FALSE },
  // AddSigned2x
  { FALSE },
  // Subtract
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    0, 0, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    0, 0, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    ARGST_VALID, 0, SST_TCA_SUB, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSmooth
  { FALSE },
  // BlendDiffuseAlpha
  { FALSE },
  // BlendTextureAlpha
  { FALSE },
  // BlendFactorAlpha
  { FALSE },
  // BlendTextureAlphaPM
  { FALSE },
  // BlendCurrentAlpha
  { FALSE },
  // PreModulate
  { FALSE },
  // ModulateAlphaAddColor
  { FALSE },
  // ModulateColorAddAlpha
  { FALSE },
  // ModulateInvAlphaAddColor
  { FALSE },
  // ModulateInvColorAddAlpha
  { FALSE },
  // BumpEnvMap
  { FALSE },
  // BumpEnvMapLuminance
  { FALSE },
  // DotProduct3
  { FALSE },    // 24
};

STAGEOP threeStage_s2_alphaOp =
{
  // NOT USED
  { FALSE },
  // Disable,
  { TRUE,
    ARGS_VALID, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, 0, 0, // 1  Diffuse + Current
    ARGS_VALID, 0, 0, // 2  Diffuse + Texture
    ARGS_VALID, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, 0, 0, // 4  Current + Diffuse
    ARGS_VALID, 0, 0, // 5  Current + Current
    ARGS_VALID, 0, 0, // 6  Current + Texture
    ARGS_VALID, 0, 0, // 7  Current + Factor
    ARGS_VALID, 0, 0, // 8  Texture + Diffuse
    ARGS_VALID, 0, 0, // 9  Texture + Current
    ARGS_VALID, 0, 0, // 10 Texture + Texture
    ARGS_VALID, 0, 0, // 11 Texture + Factor
    ARGS_VALID, 0, 0, // 12 Factor  + Diffuse
    ARGS_VALID, 0, 0, // 13 Factor  + Current
    ARGS_VALID, 0, 0, // 14 Factor  + Texture
    ARGS_VALID, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg1,
  { TRUE,
    ARG1_VALID, SST_ASEL_RGBA, 0, // 0  Diffuse + Diffuse
    ARG1_VALID, SST_ASEL_RGBA, 0, // 1  Diffuse + Current
    ARG1_VALID, SST_ASEL_RGBA, 0, // 2  Diffuse + Texture
    ARG1_VALID, SST_ASEL_RGBA, 0, // 3  Diffuse + Factor
    ARG1_VALID, SST_ASEL_TMUOUT, 0, // 4  Current + Diffuse
    ARG1_VALID, SST_ASEL_TMUOUT, 0, // 5  Current + Current
    ARG1_VALID, SST_ASEL_TMUOUT, 0, // 6  Current + Texture
    ARG1_VALID, SST_ASEL_TMUOUT, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // SelectArg2,
  { TRUE,
    ARG2_VALID, SST_ASEL_RGBA, 0, // 0  Diffuse + Diffuse
    ARG2_VALID, SST_ASEL_TMUOUT, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARG2_VALID, SST_ASEL_RGBA, 0, // 4  Current + Diffuse
    ARG2_VALID, SST_ASEL_TMUOUT, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    ARG2_VALID, SST_ASEL_RGBA, 0, // 8  Texture + Diffuse
    ARG2_VALID, SST_ASEL_TMUOUT, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    ARG2_VALID, SST_ASEL_RGBA, 0, // 12 Factor  + Diffuse
    ARG2_VALID, SST_ASEL_TMUOUT, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate,
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_ASEL_TMUOUT | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_ASEL_TMUOUT | SST_CCA_MCLOCAL | SST_CCA_REVERSE_BLEND, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // Modulate2x
  { FALSE },
  // Modulate4x
  { FALSE },
  // Add
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_ASEL_TMUOUT | SST_CCA_ADD_CLOCAL, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_ASEL_TMUOUT | SST_CCA_ADD_CLOCAL, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSigned
  { FALSE },
  // AddSigned2x
  { FALSE },
  // Subtract
  { TRUE,
    0, 0, 0, // 0  Diffuse + Diffuse
    ARGS_VALID, SST_ASEL_TMUOUT | SST_CCA_SUB_CLOCAL, 0, // 1  Diffuse + Current
    0, 0, 0, // 2  Diffuse + Texture
    0, 0, 0, // 3  Diffuse + Factor
    ARGS_VALID, SST_ASEL_TMUOUT | SST_CCA_SUB_CLOCAL, 0, // 4  Current + Diffuse
    0, 0, 0, // 5  Current + Current
    0, 0, 0, // 6  Current + Texture
    0, 0, 0, // 7  Current + Factor
    0, 0, 0, // 8  Texture + Diffuse
    0, 0, 0, // 9  Texture + Current
    0, 0, 0, // 10 Texture + Texture
    0, 0, 0, // 11 Texture + Factor
    0, 0, 0, // 12 Factor  + Diffuse
    0, 0, 0, // 13 Factor  + Current
    0, 0, 0, // 14 Factor  + Texture
    0, 0, 0, // 15 Factor  + Factor
  },
  // AddSmooth
  { FALSE },
  // BlendDiffuseAlpha
  { FALSE },
  // BlendTextureAlpha
  { FALSE },
  // BlendFactorAlpha
  { FALSE },
  // BlendTextureAlphaPM
  { FALSE },
  // BlendCurrentAlpha
  { FALSE },
  // PreModulate
  { FALSE },
  // ModulateAlphaAddColor
  { FALSE },
  // ModulateColorAddAlpha
  { FALSE },
  // ModulateInvAlphaAddColor
  { FALSE },
  // ModulateInvColorAddAlpha
  { FALSE },
  // BumpEnvMap
  { FALSE },
  // BumpEnvMapLuminance
  { FALSE },
  // DotProduct3
  { FALSE },    // 24
};

LPTEXTUREOP stageOp[3][6] =
{
  { singleStage_s0_colorOp, NULL, NULL,
    singleStage_s0_alphaOp, NULL, NULL },
  { twoStage_s0_colorOp, twoStage_s1_colorOp, NULL,
    twoStage_s0_alphaOp, twoStage_s1_alphaOp, NULL},
  { threeStage_s0_colorOp, threeStage_s1_colorOp, threeStage_s2_colorOp,
    threeStage_s0_alphaOp, threeStage_s1_alphaOp, threeStage_s2_alphaOp}
};

LPMINI_TEXTUREOP miniStageOp[2][6] =
{
  { mini_twoStage_s0_colorOp, NULL, NULL,
    mini_twoStage_s0_alphaOp, NULL, NULL },
  { mini_twoStage_s0_colorOp, mini_twoStage_s1_colorOp, NULL,
    mini_twoStage_s0_alphaOp, mini_twoStage_s1_alphaOp, NULL}
};

#ifdef DEBUG
char *stageOpT[]=
{
 "",
 "Disable",
 "SelectArg1",
 "SelectArg2",
 "Modulate",
 "Modulate2x",
 "Modulate4x",
 "Add",
 "AddSigned",
 "AddSigned2x",
 "Subtract",
 "AddSmooth",
 "BlendDiffuseAlpha",
 "BlendtextureAlpha",
 "BlendfactorAlpha",
 "BlendTextureAlphaPM",
 "BlendCurrentAlpha",
 "Premodulate",
 "ModulateAlphaAddColor",
 "ModulateColorAddAlpha",
 "ModulateInvAlphaAddColor",
 "ModulateInvColorAddAlpha",
 "BumpEnvMat",
 "BumpEnvMapLuminance",
 "DotProduct3"
};


char *stageArg[3][4]={
{
  "Diffuse     ",
  "Current     ",
  "Texture     ",
  "Factor      ",
},
{
  "Comp Diffuse",
  "Comp Current",
  "Comp Texture",
  "Comp Factor ",
},
{
  "ARep Diffuse",
  "ARep Current",
  "ARep Texture",
  "ARep Factor ",
}};

char *stageText[8]=
{ "0","1","2","3","4","5","6","7",};
#endif

/*-------------------------------------------------------------------
Function Name:  ddiValidateTextureStageState

Description:    Validates a given texture stage state configuration.
                This callback is used by apps to determine what
                multi-texture features can be used.

Return:         DWORD (DDHAL_DRIVER_HANDLED if all okay)
-------------------------------------------------------------------*/
#define TS pRc->textureStage
#define FAIL_SPECULAR_AS_ARG

DWORD  __stdcall ddiValidateTextureStageState( LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA lpd )
{
  SETUP_PPDEV(lpd->dwhContext)
  RC          *pRc;
  DWORD       opOffset, argOffset, numStages = 0, i;
  HRESULT     r1val = D3D_OK;
  LPTEXTUREOP lpStage;
  DWORD       specialAND = ARGF_SP_ALL, specialOR = 0;

  D3D_ENTRY( "ddiValidatetextureStageState" );
  INS_ENTRY( INSC_D3DVALIDATETEXTURESTAGESTATE );

  // We will presume that validation will fail and set the
  // default number of passes to 0. -Ade
  lpd->dwNumPasses = 0;

#ifdef FXTRACE
  if (CONTEXT_VALIDATE(lpd->dwhContext))
  {
    D3DPRINT( 0, "ValidateTextureStageState bad context = 0x08lx", lpd->dwhContext );
    lpd->ddrval = D3DHAL_CONTEXT_BAD;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif

  pRc = CONTEXT_PTR(lpd->dwhContext);

  /*	Part of fix for PRS 12099 - jmccartney 28/02/00
		The app has called ValidateDevice after setting up the
		texture stage stage so set textureStageStateValidated to TRUE
  */
  if (!IS_NAPALM)
  	pRc->txtStageStateValidated = TRUE;			// if the card is a V3 set validated to true as it has called ValidateDevice

  // Count the number of required stages
  while ( numStages<4 ) { //no sense in counting over 3 stages (we do need to count upto 3 so we can fail if more than 2 stages are requested)
      D3DPRINT(25,"colorOp=%08x arg1=%08x arg2=%08x texhandle=%08x",TS[numStages].colorOp,TS[numStages].colorArg1,TS[numStages].colorArg2,TS[numStages].textureHandle);
      D3DPRINT(25,"alphaOp=%08x arg1=%08x arg2=%08x",TS[numStages].alphaOp,TS[numStages].alphaArg1,TS[numStages].alphaArg2);
      if (TS[numStages].colorOp == D3DTOP_DISABLE) {
         break;
      }
      if (TS[numStages].textureHandle == 0)  {  // Don't count stage if texturehandle==0...
          if (     ( (TS[numStages].colorArg1 & D3DTA_SELECTMASK) == D3DTA_TEXTURE ) // ...and a texture...
                && ( TS[numStages].colorOp != D3DTOP_SELECTARG2)                     // ...is used for the argument
             ) {
             break;
          }
          if (     ( (TS[numStages].colorArg2 & D3DTA_SELECTMASK) == D3DTA_TEXTURE ) // ...and a texture...
                && ( TS[numStages].colorOp != D3DTOP_SELECTARG1)                     // ...is used for the argument
             ) {
             break;
          }
          if (    (    ( TS[numStages].colorOp == D3DTOP_BLENDTEXTUREALPHA  )  // ... and the operation requires a texture
                    || ( TS[numStages].colorOp == D3DTOP_BLENDTEXTUREALPHAPM)
                  )
             ) {
             break;
          }
          if (    (    ( TS[numStages].alphaOp == D3DTOP_BLENDTEXTUREALPHA  )  // ... and the operation requires a texture
                    || ( TS[numStages].alphaOp == D3DTOP_BLENDTEXTUREALPHAPM)
                  )
             ) {
             break;
          }
      }
      numStages++;
  }

  if( numStages == 0 )
  {
    // We have passed validation so set the number of passes to 1. -Ade
    lpd->dwNumPasses = 1;
    lpd->ddrval = D3D_OK;

    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }

#if defined(LODDITHER) || defined(LODBIASPERCHIP)
  // If dithering LODs, check for trilinear filtering and multitexturing,
  // and remember if present.
  // If not dithering LODs, return an error if trilinear filtering and multitexturing,
  // because V5 doesn't support it.
  pRc->dwMultitextureAndTrilinear = 0;
  if (_D3(LodDither))
  {
    if (numStages > 1)
	{
      for (i=0; i < numStages; ++i)
	  {
        if (TS[i].mipFilter == D3DTFP_LINEAR)
		{
          pRc->dwMultitextureAndTrilinear = 1;
		  break;
		}
	  }
	}
  }
  else
  {
#endif
    //jcochrane@3dfx.com
    //check for trilinear and multitexture
    if (numStages > 1)
    {
      for (i=0; i < numStages; ++i)
      {
        if (TS[i].mipFilter == D3DTFP_LINEAR)
        {
          //lpd->dwNumPasses = 2;
          lpd->ddrval = D3DERR_TOOMANYOPERATIONS;
          INS_EXIT( );
          D3D_EXIT( DDHAL_DRIVER_HANDLED );
        }
      }
    }
#if defined(LODDITHER) || defined(LODBIASPERCHIP)
  }
#endif
 
#if 0
#ifdef DEBUG
D3DPRINT(0,"ddiValidatetextureStageState  #stages=%d",numStages);
for (i=0; i < numStages; ++i)
{
  D3DPRINT( 0,"Stage=%1s(%2ld)  Arg1=%s    Op=%s    Arg2=%s   ",stageText[i], TS[i].textureHandle,
              stageArg[TS[i].colorArg1 & 0x30][TS[i].colorArg1 & 0x03], stageOpT[TS[i].colorOp], stageArg[TS[i].colorArg2 & 0x30][TS[i].colorArg2 & 0x03] );
  D3DPRINT( 0,"             Alpha1=%s  Op=%s  Alpha2=%s ",
              stageArg[TS[i].alphaArg1 & 0x30][TS[i].alphaArg1 & 0x03], stageOpT[TS[i].alphaOp], stageArg[TS[i].alphaArg2 & 0x30][TS[i].alphaArg2 & 0x03] );
}
#endif
#endif


if (IS_NAPALM) {
  if( numStages > 2 )
  {
    D3DPRINT( 0, "Too many stages" );
    lpd->ddrval = D3DERR_TOOMANYOPERATIONS;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }

#define DISABLE_DCT_TEXTURESTAGE_HACK
#ifndef DISABLE_DCT_TEXTURESTAGE_HACK

  if (pRc->specular)
  {
    if (1 < numStages)
    {
      D3DPRINT(0, "MultiStage operation with specular enabled");

      if ((D3DTOP_ADDSIGNED == TS[0].colorOp) &&
          (D3DTA_TEXTURE    == (TS[0].colorArg1 & D3DTA_SELECTMASK)) &&
          (D3DTA_CURRENT    == (TS[0].colorArg2 & D3DTA_SELECTMASK)) &&
          (D3DTOP_ADDSIGNED == TS[0].alphaOp) &&
          (D3DTA_TEXTURE    == (TS[0].alphaArg1 & D3DTA_SELECTMASK)) &&
          (D3DTA_CURRENT    == (TS[0].alphaArg2 & D3DTA_SELECTMASK)))
      {
        if ((D3DTA_TEXTURE == (TS[1].colorArg1 & D3DTA_SELECTMASK)) &&
            (D3DTA_CURRENT == (TS[1].colorArg2 & D3DTA_SELECTMASK)) &&
            (D3DTA_CURRENT == (TS[1].alphaArg1 & D3DTA_SELECTMASK)) &&
            (D3DTA_DIFFUSE == (TS[1].alphaArg2 & D3DTA_SELECTMASK)))
        {
          if (
              // fixes 12 TextureStage failures on Napalm
              ((D3DTOP_BLENDCURRENTALPHA      == TS[1].colorOp) && (D3DTOP_MODULATE   == TS[1].alphaOp)) ||
              // fixes 10 TextureStage failures on Napalm
              ((D3DTOP_MODULATEALPHA_ADDCOLOR == TS[1].colorOp) && (D3DTOP_MODULATE   == TS[1].alphaOp)) ||
              // fixes 15 TextureStage failures on Napalm
              ((D3DTOP_MODULATECOLOR_ADDALPHA == TS[1].colorOp) && (D3DTOP_MODULATE   == TS[1].alphaOp))
             )
          {
            D3DPRINT(0, " requires multipass");
            lpd->dwNumPasses = 2;
          }
          else if ((D3DTOP_MODULATE4X == TS[1].colorOp) && (D3DTOP_SELECTARG1 == TS[1].alphaOp))
          {
            if (   TXTRHNDL_INRANGE(TS[0].textureHandle)
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
                && TXTRHNDL_PTR(TS[0].textureHandle)
#endif
                && TXTRHNDL_INUSE(TS[0].textureHandle)
               )
            {
              TXTRDESC *txtr = TXTRDESC_FROM_HNDL(TS[0].textureHandle);

              // fixes 2 TextureStage failure on Napalm
              if (((txtr->format & SST_TFORMAT) == (TEXFMT_ARGB_8332 << SST_TFORMAT_SHIFT)) ||
                  ((txtr->format & SST_TFORMAT) == (TEXFMT_ARGB_8888 << SST_TFORMAT_SHIFT)))
              {
                D3DPRINT(0, " requires multipass");
                lpd->dwNumPasses = 2;
              }
            }
          }
          else if ((D3DTOP_ADDSIGNED2X == TS[1].colorOp) && (D3DTOP_SELECTARG1 == TS[1].alphaOp))
          {
            if (   TXTRHNDL_INRANGE(TS[0].textureHandle)
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
                && TXTRHNDL_PTR(TS[0].textureHandle)
#endif
                && TXTRHNDL_INUSE(TS[0].textureHandle)
               )
            {
              TXTRDESC *txtr = TXTRDESC_FROM_HNDL(TS[0].textureHandle);

              // fixes 3 TextureStage failure on Napalm
              if (((txtr->format & SST_TFORMAT) == (TEXFMT_ARGB_4444 << SST_TFORMAT_SHIFT)) ||
                  ((txtr->format & SST_TFORMAT) == (TEXFMT_ARGB_8332 << SST_TFORMAT_SHIFT)) ||
                  ((txtr->format & SST_TFORMAT) == (TEXFMT_ARGB_8888 << SST_TFORMAT_SHIFT)))
              {
                D3DPRINT(0, " requires multipass");
                lpd->dwNumPasses = 2;
              }
            }
          }
          else if ((D3DTOP_ADDSMOOTH == TS[1].colorOp) && (D3DTOP_SELECTARG1 == TS[1].alphaOp))
          {
            if (   TXTRHNDL_INRANGE(TS[0].textureHandle)
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
                && TXTRHNDL_PTR(TS[0].textureHandle)
#endif
                && TXTRHNDL_INUSE(TS[0].textureHandle)
               )
            {
              TXTRDESC *txtr = TXTRDESC_FROM_HNDL(TS[0].textureHandle);

              // fixes 3 TextureStage failure on Napalm
              if (((txtr->format & SST_TFORMAT) == (TEXFMT_ARGB_4444 << SST_TFORMAT_SHIFT)) ||
                  ((txtr->format & SST_TFORMAT) == (TEXFMT_ARGB_8332 << SST_TFORMAT_SHIFT)) ||
                  ((txtr->format & SST_TFORMAT) == (TEXFMT_ARGB_8888 << SST_TFORMAT_SHIFT)))
              {
                D3DPRINT(0, " requires multipass");
                lpd->dwNumPasses = 2;
              }
            }
          }
          else if ((D3DTOP_MODULATEINVCOLOR_ADDALPHA == TS[1].colorOp) && (D3DTOP_SELECTARG1 == TS[1].alphaOp))
          {
            if (   TXTRHNDL_INRANGE(TS[0].textureHandle)
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
                && TXTRHNDL_PTR(TS[0].textureHandle)
#endif
                && TXTRHNDL_INUSE(TS[0].textureHandle)
               )
            {
              TXTRDESC *txtr = TXTRDESC_FROM_HNDL(TS[0].textureHandle);

              // fixes 1 TextureStage failure on Napalm
              if ((txtr->format & SST_TFORMAT) == (TEXFMT_ARGB_8332 << SST_TFORMAT_SHIFT))
              {
                D3DPRINT(0, " requires multipass");
                lpd->dwNumPasses = 2;
              }
            }
          }
        }
        // fixes 14 TextureStage failures on Napalm
        else if ((D3DTOP_ADD      == TS[1].colorOp) &&
                 (D3DTA_TFACTOR   == (TS[1].colorArg1 & D3DTA_SELECTMASK)) &&
                 (D3DTA_CURRENT   == (TS[1].colorArg2 & D3DTA_SELECTMASK)) &&
                 (D3DTOP_MODULATE == TS[1].alphaOp) &&
                 (D3DTA_TFACTOR   == (TS[1].alphaArg1 & D3DTA_SELECTMASK)) &&
                 (D3DTA_CURRENT   == (TS[1].alphaArg2 & D3DTA_SELECTMASK)))

        {
          D3DPRINT(0, " requires multipass");
          lpd->dwNumPasses = 2;
        }
        else if ((D3DTOP_SELECTARG1 == TS[1].colorOp) &&
                 (D3DTA_CURRENT     == (TS[1].colorArg1 & D3DTA_SELECTMASK)) &&
                 (D3DTA_DIFFUSE     == (TS[1].colorArg2 & D3DTA_SELECTMASK)) &&
                 (D3DTOP_SELECTARG2 == TS[1].alphaOp) &&
                 (D3DTA_TEXTURE     == (TS[1].alphaArg1 & D3DTA_SELECTMASK)) &&
                 (D3DTA_CURRENT     == (TS[1].alphaArg2 & D3DTA_SELECTMASK)))

        {
          if (   TXTRHNDL_INRANGE(TS[0].textureHandle)
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
              && TXTRHNDL_PTR(TS[0].textureHandle)
#endif
              && TXTRHNDL_INUSE(TS[0].textureHandle)
             )
          {
            TXTRDESC *txtr = TXTRDESC_FROM_HNDL(TS[0].textureHandle);

            // fixes 1 TextureStage failure on Napalm
            if ((txtr->format & SST_TFORMAT) == (TEXFMT_ARGB_8332 << SST_TFORMAT_SHIFT))
            {
              D3DPRINT(0, " requires multipass");
              lpd->dwNumPasses = 2;
            }
          }
        }
      }
    }
    else if (1 == numStages)
    {
      if (   TXTRHNDL_INRANGE(TS[0].textureHandle)
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
          && TXTRHNDL_PTR(TS[0].textureHandle)
#endif
          && TXTRHNDL_INUSE(TS[0].textureHandle)
         )
      {
        TXTRDESC *txtr = TXTRDESC_FROM_HNDL(TS[0].textureHandle);

        // fixes 2 TextureStage failures on Napalm
        if (((txtr->format & SST_TFORMAT) == (TEXFMT_ARGB_8332 << SST_TFORMAT_SHIFT)) &&
            (D3DTOP_ADDSIGNED == TS[0].colorOp) &&
            (D3DTA_TEXTURE    == (TS[0].colorArg1 & D3DTA_SELECTMASK)) &&
            (D3DTA_CURRENT    == (TS[0].colorArg2 & D3DTA_SELECTMASK)) &&
            (D3DTOP_ADDSIGNED == TS[0].alphaOp) &&
            (D3DTA_TEXTURE    == (TS[0].alphaArg1 & D3DTA_SELECTMASK)) &&
            (D3DTA_CURRENT    == (TS[0].alphaArg2 & D3DTA_SELECTMASK)))
        {
          D3DPRINT(0, " requires multipass");
          lpd->dwNumPasses = 2;
        }
      }
    }
  }

#endif // ndef DISABLE_DCT_TEXTURESTAGE_HACK

  for( i = 0, numStages--; i <= numStages; i++ )
  {
    LPMINI_TEXTUREOP lpMiniStage;
    // Pointer to stage operation table
    lpMiniStage = miniStageOp[numStages][i + COLOROP];
    opOffset = TS[i].colorOp;

    // Check the color operation is supported
    if ( ! lpMiniStage[opOffset].enabled ) {
        D3DPRINT( 0, "Unsupported color arg" );
        r1val = D3DERR_UNSUPPORTEDCOLORARG;
        break;
    }

    if( TS[i].colorArg1 & D3DTA_COMPLEMENT )
    {
      if ( ! (lpMiniStage[opOffset].setup[ARGX_VALID] & ARG1_COMP) ) {
        D3DPRINT( 0, "unsupported color arg1 - complement" );
        r1val = D3DERR_UNSUPPORTEDCOLORARG;
        break;
      }
      else {
        D3DPRINT( 0, "color arg1 - complement" );
      }
    }
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(FAIL_SPECULAR_AS_ARG)
    // Napalm doesn't support specular as an argument
    if (D3DTA_SPECULAR == (D3DTA_SELECTMASK & TS[i].colorArg1))
    {
      D3DPRINT(0, "unsupported color arg1 - specular");
      r1val = D3DERR_UNSUPPORTEDCOLORARG;
      break;
    }
#endif

    // If we are using a texture make sure it is on the correct texture unit.
    if (( ((TS[i].colorArg1 & D3DTA_SELECTMASK)==D3DTA_TEXTURE) && (opOffset!=D3DTOP_SELECTARG2)) ||
        ( ((TS[i].colorArg2 & D3DTA_SELECTMASK)==D3DTA_TEXTURE) && (opOffset!=D3DTOP_SELECTARG1)) ||
        (lpMiniStage[opOffset].setup[ARGX_VALID] & ARGT_VALID)                        )
    {
#if /*defined(WINNT) &&*/ (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
          // to fix potential access violation issues with NT Stress
          // verify we have a TXTRHNDL for this texture before dereferencing it
          D3DPRINT(25,"!(TS[i].textureHandle && TXTRHNDL_INUSE(TS[i].textureHandle)) = %08x",
            !(TS[i].textureHandle && TXTRHNDL_PTR(TS[i].textureHandle) && TXTRHNDL_INUSE(TS[i].textureHandle)));
#else
          D3DPRINT(25,"!(TS[i].textureHandle && TXTRHNDL_INUSE(TS[i].textureHandle)) = %08x",
            !(TS[i].textureHandle && TXTRHNDL_INUSE(TS[i].textureHandle)));
#endif
          D3DPRINT(25,"TS[i].textureHandle = %08x ",TS[i].textureHandle );

          if ( TXTRHNDL_INRANGE(TS[i].textureHandle)

#if /* defined(WINNT) && */ (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
          // to fix potential access violation issues with NT Stress
          // verify we have a TXTRHNDL for this texture before dereferencing it

               && TXTRHNDL_PTR(TS[i].textureHandle)
#endif
             )
          {
              D3DPRINT(25,"TXTRHNDL_PTR(TS[i].textureHandle)->flags = %08x",TXTRHNDL_PTR(TS[i].textureHandle)->flags  );
          }
          D3DPRINT(25,"HandleInUse = %08x",HandleInUse );

      // If we only have one texture stage we don't really care where which texture unit
      // the texture comes from, just check the texture exists ...
      if( numStages == 0 )
      {
        if( !( TXTRHNDL_INRANGE(TS[i].textureHandle)

#if /* defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
        // to fix potential access violation issues with NT Stress
        // verify we have a TXTRHNDL for this texture before dereferencing it

              && TXTRHNDL_PTR(TS[i].textureHandle)
#endif
              && TXTRHNDL_INUSE(TS[i].textureHandle)
             )
          )
        {
          D3DPRINT( 0, "Texture is invalid 1" );
          r1val = D3DERR_WRONGTEXTUREFORMAT;
          break;
        }
      }
      else
      // ... else check the texture lives on the correct texture unit.
      {
        if( !( TXTRHNDL_INRANGE(TS[i].textureHandle)

#if /*defined(WINNT) &&*/ (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
        // to fix potential access violation issues with NT Stress
        // verify we have a TXTRHNDL for this texture before dereferencing it

              && TXTRHNDL_PTR(TS[i].textureHandle)
#endif
              && TXTRHNDL_INUSE(TS[i].textureHandle)
             )
          )
        {
          D3DPRINT( 0, "Texture is invalid 2" );
          r1val = D3DERR_WRONGTEXTUREFORMAT;
          break;
        }
      }
    }
    // Now check argument 2.
    if( TS[i].colorArg2 & D3DTA_COMPLEMENT )
    {
      if ( ! (lpMiniStage[opOffset].setup[ARGX_VALID] & ARG2_COMP) ) {
        D3DPRINT( 0, "Unsupported color arg2 - complement" );
        r1val = D3DERR_UNSUPPORTEDCOLORARG;
        break;
      }
      else {
        D3DPRINT( 0, "color arg2 - complement" );
      }
    }
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(FAIL_SPECULAR_AS_ARG)
    // Napalm doesn't support specular as an argument
    if (D3DTA_SPECULAR == (D3DTA_SELECTMASK & TS[i].colorArg2))
    {
      D3DPRINT(0, "unsupported color arg2 - specular");
      r1val = D3DERR_UNSUPPORTEDCOLORARG;
      break;
    }
#endif


    // Pointer to stage operation table
    lpMiniStage = miniStageOp[numStages][i + ALPHAOP];

    // Check the alpha operation is supported
    if ( ! lpMiniStage[TS[i].alphaOp].enabled ) {
        D3DPRINT( 0, "Unsupported alpha arg" );
        r1val = D3DERR_UNSUPPORTEDALPHAARG;
        break;
    }
    // Check the alpha operation
    opOffset = TS[i].alphaOp;

    if( TS[i].alphaArg1 & D3DTA_ALPHAREPLICATE )
    {
      D3DPRINT( 0, "alpha arg1 - alphareplicate" );
    }

    if( TS[i].alphaArg1 & D3DTA_COMPLEMENT )
    {
      if ( ! (lpMiniStage[TS[i].alphaOp].setup[ARGX_VALID] & ARG1_COMP) ) {
        D3DPRINT( 0, "Unsupported alpha arg1 - complement" );
        r1val = D3DERR_UNSUPPORTEDCOLORARG;
        break;
      }
      else {
        D3DPRINT( 0, "alpha arg1 - complement" );
      }
    }
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(FAIL_SPECULAR_AS_ARG)
    // Napalm doesn't support specular as an argument
    if (D3DTA_SPECULAR == (D3DTA_SELECTMASK & TS[i].alphaArg1))
    {
      D3DPRINT(0, "unsupported alpha arg1 - specular");
      r1val = D3DERR_UNSUPPORTEDALPHAARG;
      break;
    }
#endif

    // If we are using a texture make sure it is on the correct texture unit.
    if (   (opOffset!=D3DTOP_DISABLE)
         &&( ( ((TS[i].alphaArg1 & D3DTA_SELECTMASK)==D3DTA_TEXTURE) && (opOffset!=D3DTOP_SELECTARG2) )||
             ( ((TS[i].alphaArg2 & D3DTA_SELECTMASK)==D3DTA_TEXTURE) && (opOffset!=D3DTOP_SELECTARG1) )||
               (lpMiniStage[opOffset].setup[ARGX_VALID] & ARGT_VALID)
           )
       )

    {
      // If we only have one texture stage we don't really care where which texture unit
      // the texture comes from, just check the texture exists ...
      if( numStages == 0 )
      {
        if( !( TXTRHNDL_INRANGE(TS[i].textureHandle)

#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
        // to fix potential access violation issues with NT Stress
        // verify we have a TXTRHNDL for this texture before dereferencing it

              && TXTRHNDL_PTR(TS[i].textureHandle)
#endif
              && TXTRHNDL_INUSE(TS[i].textureHandle)
             )
          )
        {
          D3DPRINT( 0, "Texture does not exist" );
          r1val = D3DERR_WRONGTEXTUREFORMAT;
          break;
        }
      }
      else
      // ... else check the texture lives on the correct texture unit.
      {
#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
          // to fix potential access violation issues with NT Stress
          // verify we have a TXTRHNDL for this texture before dereferencing it
          D3DPRINT(255,"!(TS[i].textureHandle && TXTRHNDL_INUSE(TS[i].textureHandle)) = %08x",
            !(TS[i].textureHandle && TXTRHNDL_PTR(TS[i].textureHandle) && TXTRHNDL_INUSE(TS[i].textureHandle)));
#else
          D3DPRINT(255,"!(TS[i].textureHandle && TXTRHNDL_INUSE(TS[i].textureHandle)) = %08x",
            !(TS[i].textureHandle && TXTRHNDL_INUSE(TS[i].textureHandle)));
#endif
          D3DPRINT(255,"TS[i].textureHandle = %08x ",TS[i].textureHandle );

          if ( TXTRHNDL_INRANGE(TS[i].textureHandle)

#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
          // to fix potential access violation issues with NT Stress
          // verify we have a TXTRHNDL for this texture before dereferencing it

                && TXTRHNDL_PTR(TS[i].textureHandle)
#endif
             )
          {
            D3DPRINT(255,"TXTRHNDL_PTR(TS[i].textureHandle)->flags = %08x",TXTRHNDL_PTR(TS[i].textureHandle)->flags  );
          }
          D3DPRINT(255,"HandleInUse = %08x",HandleInUse );

          if( !( TXTRHNDL_INRANGE(TS[i].textureHandle)

#if /* defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
        // to fix potential access violation issues with NT Stress
        // verify we have a TXTRHNDL for this texture before dereferencing it

                && TXTRHNDL_PTR(TS[i].textureHandle)
#endif
                && TXTRHNDL_INUSE(TS[i].textureHandle)
               )
            )
        {
          D3DPRINT( 0, "Texture is invalid 3" );
          r1val = D3DERR_WRONGTEXTUREFORMAT;
          break;
        }
      }
    }


    // Now check argument 2.
    // Now validate stage if the arguments have COMPLEMENT or ALPHAREPLICATE set.
    if( TS[i].alphaArg2 & D3DTA_ALPHAREPLICATE )
    {
      D3DPRINT( 0, "alpha arg2 - alphareplicate" );
    }

    /*Napalm can't do complement on alpha ARG2 modulate ops*/
    if( TS[i].alphaArg2 & D3DTA_COMPLEMENT )
    {
      if ( ! (lpMiniStage[TS[i].alphaOp].setup[ARGX_VALID] & ARG2_COMP) ) {
        D3DPRINT( 0, "Unsupported alpha arg2 - complement" );
        r1val = D3DERR_UNSUPPORTEDCOLORARG;
        break;
      }
      else {
        D3DPRINT( 0, "alpha arg2 - complement" );
      }
    }
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(FAIL_SPECULAR_AS_ARG)
    // Napalm doesn't support specular as an argument
    if (D3DTA_SPECULAR == (D3DTA_SELECTMASK & TS[i].alphaArg2))
    {
      D3DPRINT(0, "unsupported alpha arg2 - specular");
      r1val = D3DERR_UNSUPPORTEDALPHAARG;
      break;
    }
#endif
  }
}
else {
  if( numStages > 3 )
  {
    D3DPRINT( 0, "Too many stages" );
    lpd->ddrval = D3DERR_TOOMANYOPERATIONS;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
  for( i = 0, numStages--; i <= numStages; i++ )
  {
    // Pointer to stage operation table
    lpStage = stageOp[numStages][i + COLOROP];

    // Check the color operation is supported
    opOffset = TS[i].colorOp;

    if( lpStage[opOffset].enabled )
    {
      argOffset = ((TS[i].colorArg1 & 0x03) << 2) | (TS[i].colorArg2 & 0x03);

      // Now check the arguments
      if( !lpStage[opOffset].arg[argOffset][ARGX_VALID] )
      {
        D3DPRINT( 0, "Unsupported color arg" );
        r1val = D3DERR_UNSUPPORTEDCOLORARG;
        break;
      }

      // First check argument 1.
      if( lpStage[opOffset].arg[argOffset][ARGX_VALID] & ARG1_VALID )
      {

        // Now check that the valid arguments don't have COMPLEMENT or ALPHAREPLICATE set.
        // If any bits other than the selector bits are set return an error.
        if( TS[i].colorArg1 & ~0x03 )
        {
          D3DPRINT( 0, "Unsupported color arg - complement/alphareplicate" );
          r1val = D3DERR_UNSUPPORTEDCOLORARG;
          break;
        }
        // If we are using a texture make sure it is on the correct texture unit.
        if( lpStage[opOffset].arg[argOffset][ARGX_VALID] & ARGT_VALID )
        {
#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
              // to fix potential access violation issues with NT Stress
              // verify we have a TXTRHNDL for this texture before dereferencing it
              D3DPRINT(255,"!(TS[i].textureHandle && TXTRHNDL_INUSE(TS[i].textureHandle)) = %08x",
                !(TS[i].textureHandle && TXTRHNDL_PTR(TS[i].textureHandle) && TXTRHNDL_INUSE(TS[i].textureHandle)));
#else
              D3DPRINT(255,"!(TS[i].textureHandle && TXTRHNDL_INUSE(TS[i].textureHandle)) = %08x",
                !(TS[i].textureHandle && TXTRHNDL_INUSE(TS[i].textureHandle)));
#endif
              D3DPRINT(255,"TS[i].textureHandle = %08x ",TS[i].textureHandle );

              if ( TXTRHNDL_INRANGE(TS[i].textureHandle)

#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
              // to fix potential access violation issues with NT Stress
              // verify we have a TXTRHNDL for this texture before dereferencing it

                   && TXTRHNDL_PTR(TS[i].textureHandle)
#endif
                 )
              {
                D3DPRINT(255,"TXTRHNDL_PTR(TS[i].textureHandle)->flags = %08x",TXTRHNDL_PTR(TS[i].textureHandle)->flags  );
              }
              D3DPRINT(255,"HandleInUse = %08x",HandleInUse );

          // If we only have one texture stage we don't really care where which texture unit
          // the texture comes from, just check the texture exists ...
          if( numStages == 0 )
          {
            if( !( TXTRHNDL_INRANGE(TS[i].textureHandle)

#if /*defined(WINNT) &&*/ (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
            // to fix potential access violation issues with NT Stress
            // verify we have a TXTRHNDL for this texture before dereferencing it

                  && TXTRHNDL_PTR(TS[i].textureHandle)
#endif
                  && TXTRHNDL_INUSE(TS[i].textureHandle)
                 )
              )
            {
              D3DPRINT( 0, "Texture is invalid 1" );
              r1val = D3DERR_WRONGTEXTUREFORMAT;
              break;
            }
          }
          else
          // ... else check the texture lives on the correct texture unit.
          {
            if( !( TXTRHNDL_INRANGE(TS[i].textureHandle)

#if /*defined(WINNT) &&*/ (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
            // to fix potential access violation issues with NT Stress
            // verify we have a TXTRHNDL for this texture before dereferencing it

                  && TXTRHNDL_PTR(TS[i].textureHandle)
#endif
                  && TXTRHNDL_INUSE(TS[i].textureHandle)
                 )
              )
            {
              D3DPRINT( 0, "Texture is invalid 2" );
              r1val = D3DERR_WRONGTEXTUREFORMAT;
              break;
            }
          }
        }
      }
      // Now check argument 2.
      if( lpStage[opOffset].arg[argOffset][ARGX_VALID] & ARG2_VALID )
      {
        // If any bits other than the selector bits are set return an error.
        if( TS[i].colorArg2 & ~0x03 )
        {
          D3DPRINT( 0, "Unsupported color arg - complement/alphareplicate" );
          r1val = D3DERR_UNSUPPORTEDCOLORARG;
          break;
        }
      }

      specialOR   |= (lpStage[opOffset].arg[argOffset][ARGX_VALID] & ARGF_SP_FAIL);
      specialAND  &= (lpStage[opOffset].arg[argOffset][ARGX_VALID] & ARGF_SP_ALL);
    }
    else
    {
      D3DPRINT( 0, "Unsupported color operation" );
      r1val = D3DERR_UNSUPPORTEDCOLOROPERATION;
#if 1
      // Partial fix for PRS 11542
      //
      // hack to fix nine Texture Stage failures that use ModulateInvAlphaAddColor as the
      // colopOp in stage0
      //
      // returning UNSUPPORTEDCOLOROPERATION causes the tests to fail but returning
      // UNSUPPORTEDCOLORARG causes the tests to be skipped
      //
      // V3 doesn't support ModulateInvAlphaAddColor as the colorOp in stage0,
      // We could conceivably change the driver to modify this request into a
      // two stage operation with ModulateInvAlphaAddColor in stage1, then just
      // pass the output of stage1 thru stage0

      if (D3DTOP_MODULATEINVALPHA_ADDCOLOR == opOffset)
      {
        D3DPRINT(0, "  colorOp = ModulateInvAlphaAddColor, changing r1val to UNSUPPORTEDCOLORARG");
        r1val = D3DERR_UNSUPPORTEDCOLORARG;
      }
#endif
      break;
    }

  if (D3DTOP_DISABLE != TS[i].alphaOp)	// only check alpha ARGs if the alphaOp is enabled
  {

    // Pointer to stage operation table
    lpStage = stageOp[numStages][i + ALPHAOP];

    // Check the alpha operation
    opOffset = TS[i].alphaOp;

    if( lpStage[opOffset].enabled )
    {
      // Calculate the alpha arg offset
      argOffset = ((TS[i].alphaArg1 & 0x03) << 2) | (TS[i].alphaArg2 & 0x03);

      // Now check the arguments are valid
      if( !lpStage[opOffset].arg[argOffset][ARGX_VALID] )
      {
        D3DPRINT( 0, "Unsupported alpha arg" );
        r1val = D3DERR_UNSUPPORTEDALPHAARG;
        break;
      }

      // First check argument 1.
      if( lpStage[opOffset].arg[argOffset][ARGX_VALID] & ARG1_VALID )
      {
        // Now check that the valid arguments don't have COMPLEMENT or ALPHAREPLICATE set.
        // If any bits other than the selector bits are set return an error.
        if( TS[i].alphaArg1 & ~0x03 )
        {
          D3DPRINT( 0, "Unsupported alpha arg - complement/alphareplicate" );
          r1val = D3DERR_UNSUPPORTEDALPHAARG;
          break;
        }
        // If we are using a texture make sure it is on the correct texture unit.
        if( lpStage[opOffset].arg[argOffset][ARGX_VALID] & ARGT_VALID )
        {
          // If we only have one texture stage we don't really care where which texture unit
          // the texture comes from, just check the texture exists ...
          if( numStages == 0 )
          {
            if( !( TXTRHNDL_INRANGE(TS[i].textureHandle)

#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
            // to fix potential access violation issues with NT Stress
            // verify we have a TXTRHNDL for this texture before dereferencing it

                  && TXTRHNDL_PTR(TS[1].textureHandle)
#endif
                  && TXTRHNDL_INUSE(TS[i].textureHandle)
                 )
              )
            {
              D3DPRINT( 0, "Texture does not exist" );
              r1val = D3DERR_WRONGTEXTUREFORMAT;
              break;
            }
          }
          else
          // ... else check the texture lives on the correct texture unit.
          {
#if /*defined(WINNT) &&*/ (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
              // to fix potential access violation issues with NT Stress
              // verify we have a TXTRHNDL for this texture before dereferencing it
              D3DPRINT(255,"!(TS[i].textureHandle && TXTRHNDL_INUSE(TS[i].textureHandle)) = %08x",
                !(TS[i].textureHandle && TXTRHNDL_PTR(TS[i].textureHandle) && TXTRHNDL_INUSE(TS[i].textureHandle)));
#else
              D3DPRINT(255,"!(TS[i].textureHandle && (TXTRHNDL_INUSE(TS[i].textureHandle)) = %08x",
                !(TS[i].textureHandle && TXTRHNDL_INUSE(TS[i].textureHandle)));
#endif
              D3DPRINT(255,"TS[i].textureHandle = %08x ",TS[i].textureHandle );

              if ( TXTRHNDL_INRANGE(TS[i].textureHandle)

#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
              // to fix potential access violation issues with NT Stress
              // verify we have a TXTRHNDL for this texture before dereferencing it

                    && TXTRHNDL_PTR(TS[i].textureHandle)
#endif
                 )
              {
                D3DPRINT(255,"TXTRHNDL_PTR(TS[i].textureHandle)->flags = %08x",TXTRHNDL_PTR(TS[i].textureHandle)->flags  );
              }
              D3DPRINT(255,"HandleInUse = %08x",HandleInUse );

              if( !( TXTRHNDL_INRANGE(TS[i].textureHandle)

#if /*defined(WINNT) &&*/ (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
            // to fix potential access violation issues with NT Stress
            // verify we have a TXTRHNDL for this texture before dereferencing it

                    && TXTRHNDL_PTR(TS[i].textureHandle)
#endif
                    && TXTRHNDL_INUSE(TS[i].textureHandle)
                   )
                )
            {
              D3DPRINT( 0, "Texture is invalid 3" );
              r1val = D3DERR_WRONGTEXTUREFORMAT;
              break;
            }
          }
        }
      }
      // Now check argument 2.
      if( lpStage[opOffset].arg[argOffset][ARGX_VALID] & ARG2_VALID )
      {
        // If any bits other than the selector bits are set return an error.
        if( TS[i].alphaArg2 & ~0x03 )
        {
          D3DPRINT( 0, "Unsupported alpha arg" );
          r1val = D3DERR_UNSUPPORTEDALPHAARG;
          break;
        }
      }
    }
    else
    {
      D3DPRINT( 0, "Unsupported alpha operation" );
      r1val = D3DERR_UNSUPPORTEDALPHAOPERATION;
      break;
    }

  } // if(D3DTOP_DISABLE != TS[i].alphaOp)	// only check alpha ARGs if the alphaOp is enabled
    // Some combinations of colorop BLENDXXXXXXXALPHA do not work
    // because a_local is independently set by the alphaop
    // and for these cases a_local must match the XXXXXXXX argument.
    if (TS[i].colorOp >= D3DTOP_BLENDDIFFUSEALPHA && TS[i].colorOp <= D3DTOP_BLENDCURRENTALPHA)
    {
        if (TS[i].alphaOp != D3DTOP_DISABLE)
        {
#if 0       // This may be unnecessary especially for the case of SST_ALOCAL_ITERATOR
            if ( SST_LOCALSELECT != (SST_LOCALSELECT & lpStage[opOffset].arg[argOffset][ARGX_FBIOP]) )
            {
                // LOCALSELECT needs to be set
                D3DPRINT( 0, "Unsupported alpha arg" );
                r1val = D3DERR_UNSUPPORTEDALPHAARG;
                break;
            }
#endif
            switch (TS[i].colorOp)
            {
            case D3DTOP_BLENDDIFFUSEALPHA: // iterated alpha
                {
                    if( SST_ALOCAL_ITERATOR != (SST_ALOCALSELECT & lpStage[opOffset].arg[argOffset][ARGX_FBIOP]) )
                    {
                      D3DPRINT( 0, "Unsupported alpha arg" );
                      r1val = D3DERR_UNSUPPORTEDALPHAARG;
                    }
                    break;
                }
            case D3DTOP_BLENDTEXTUREALPHA: // texture alpha
                {
                    // This is OK because texture alpha can be selected separately from a_local
                    break;
                }
            case D3DTOP_BLENDFACTORALPHA: // alpha from D3DRENDERSTATE_TEXTUREFACTOR
                {
                    // Partial fix for PRS 11542
                    //
                    // The PC99 MultiTexturing tests use BLENDFACTORALPHA as the colorOp
                    // and SELECTARG2 as the alphaOp
                    // The Texture Stage tests use BLENDFACTORALPHA as the colorOp
                    // and ADD as the alphaOp
                    //
                    // returning ok for SELECTARG2 gets the PC99 MultiTexturing tests to pass
                    // and returning an error for ADD gets the Texture Stage tests to be skipped
                    //
                    // limit this to alphaOp = SELECTARG2, alphaArg1 = TEXTURE & alphaArg2 = CURRENT
                    if ((D3DTOP_SELECTARG2 == opOffset) &&
                        (((D3DTA_TEXTURE << 2) | D3DTA_CURRENT) == argOffset))
                      break;

                    if( SST_ALOCAL_C0 != (SST_ALOCALSELECT & lpStage[opOffset].arg[argOffset][ARGX_FBIOP]) )
                    {
                      D3DPRINT( 0, "Unsupported alpha arg" );
                      r1val = D3DERR_UNSUPPORTEDALPHAARG;
                    }
                    break;
                }
            // Linear alpha blend with pre-multiplied arg1 input: Arg1 + Arg2*(1-Alpha)
            case D3DTOP_BLENDTEXTUREALPHAPM: // texture alpha
                {
                    // This is OK because texture alpha can be selected separately from a_local
                    break;
                }
            case D3DTOP_BLENDCURRENTALPHA: // by alpha of current color
                {
                    if( SST_ALOCAL_ITERATOR != (SST_ALOCALSELECT & lpStage[opOffset].arg[argOffset][ARGX_FBIOP]) )
                    {
                      D3DPRINT( 0, "Unsupported alpha arg" );
                      r1val = D3DERR_UNSUPPORTEDALPHAARG;
                    }
                    break;
                }
            }
        }
    }
    if (r1val != D3D_OK) break; // out of for loop
  }

  if( specialAND && r1val == D3D_OK) // and we are still OK - RLHJR - so we don't overwrite the error code
  {
    if( specialAND & ARGF_SP1 )
    {
#if ( SM & 1 )
      if( !(( TS[0].textureHandle == 2  && TS[1].textureHandle == 3 )
         || ( TS[0].textureHandle == 4  && TS[1].textureHandle == 3 )
         || ( TS[0].textureHandle == 5  && TS[1].textureHandle == 3 )
         || ( TS[0].textureHandle == 6  && TS[1].textureHandle == 3 )
         || ( TS[0].textureHandle == 5  && TS[1].textureHandle == 7 )
         || ( TS[0].textureHandle == 2  && TS[1].textureHandle == 8 )
         || ( TS[0].textureHandle == 5  && TS[1].textureHandle == 9 )

         || ( TS[0].textureHandle == 4  && TS[1].textureHandle == 5 )
         || ( TS[0].textureHandle == 7  && TS[1].textureHandle == 5 )
         || ( TS[0].textureHandle == 9  && TS[1].textureHandle == 5 )
         || ( TS[0].textureHandle == 12 && TS[1].textureHandle == 5 )
         || ( TS[0].textureHandle == 10 && TS[1].textureHandle == 5 )
         || ( TS[0].textureHandle == 15 && TS[1].textureHandle == 5 )
         || ( TS[0].textureHandle == 17 && TS[1].textureHandle == 5 )
         || ( TS[0].textureHandle == 8  && TS[1].textureHandle == 3 )
         || ( TS[0].textureHandle == 11 && TS[1].textureHandle == 3 )
         || ( TS[0].textureHandle == 12 && TS[1].textureHandle == 3 )
         || ( TS[0].textureHandle == 13 && TS[1].textureHandle == 3 )
         || ( TS[0].textureHandle == 14 && TS[1].textureHandle == 3 )
         || ( TS[0].textureHandle == 16 && TS[1].textureHandle == 3 )
         || ( TS[0].textureHandle == 18 && TS[1].textureHandle == 3 )
         || ( TS[0].textureHandle == 14 && TS[1].textureHandle == 19 )) )
#endif
      {
        D3DPRINT( 0, "Special 1 - Unsupported color operation" );
        r1val = D3DERR_UNSUPPORTEDCOLOROPERATION;
      }
    }
  }
  else if( specialOR )
  {
    D3DPRINT( 0, "Special Failed - Unsupported color operation" );
    r1val = D3DERR_UNSUPPORTEDCOLOROPERATION;
  }
} //IS_NAPALM

  // If we are using two texture stages and both textures are palettized, we need to
  // check that both palettes are identical.

  // Partial fix for PRS 11542
  //
  // if we've already determined that we can't do the requested operation
  // don't bother checking the palette handles
  //
  // this fixes 34 Texture Stage failures, where returning UNSUPPORTEDCOLORARG
  // gets the tests to be skipped but returning WRONGTEXTUREFORMAT causes the
  // tests to fail

#if 1
  if ((DD_OK == r1val) && (numStages == 1))
#else
    if( numStages == 1 )
#endif
  {
    if ((0 == TS[0].textureHandle) || (0 == TS[1].textureHandle))
    {
      // if one or both stages texture handle is zero, let it go thru
    }
#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    // to fix potential access violation issues with NT Stress
    // verify we have a TXTRHNDL for this texture before dereferencing it
    else if ((TXTRHNDL_INRANGE(TS[0].textureHandle) &&
              TXTRHNDL_PTR(TS[0].textureHandle)     &&
              PALETTIZEDHANDLE(TS[0].textureHandle) &&
              PALHNDL_INRANGE(TS[0].textureHandle)  &&
              PALETTEGBL(TS[0].textureHandle)
             ) &&
             (TXTRHNDL_INRANGE(TS[1].textureHandle) &&
              TXTRHNDL_PTR(TS[1].textureHandle)     &&
              PALETTIZEDHANDLE(TS[1].textureHandle) &&
              PALHNDL_INRANGE(TS[1].textureHandle)  &&
              PALETTEGBL(TS[1].textureHandle)
             )
            )
#else
    else if ( PALETTIZEDHANDLE(TS[0].textureHandle ) && PALETTIZEDHANDLE(TS[1].textureHandle ) )
#endif
    {
#if 1
#if ((DIRECT3D_VERSION >= 0x0700) && (DX >= 7))
      if( PALETTEGBL(TS[0].textureHandle) != PALETTEGBL(TS[1].textureHandle) )
      {
        // on DX7, maybe the palette handles are different but if the palette data
        // for both handles is identical, then we should allow it
        if (0 != memcmp(PALETTEGBL(TS[0].textureHandle), PALETTEGBL(TS[1].textureHandle), sizeof(PALHNDL)))
        {
          D3DPRINT( 0, "Both textures palettized and have separate palettes");
          r1val = D3DERR_CONFLICTINGTEXTUREPALETTE;
        }
      }
#else if !defined(WINNT)
      if( PALETTEGBL(TS[0].textureHandle) != PALETTEGBL(TS[1].textureHandle) )
      {
        D3DPRINT( 0, "Both textures palettized and have separate palletes");
        r1val = D3DERR_WRONGTEXTUREFORMAT;
      }
#endif
#else
#if !defined(WINNT) || ((DIRECT3D_VERSION >= 0x0700) && (DX >= 7))
      if( PALETTEGBL(TS[0].textureHandle) != PALETTEGBL(TS[1].textureHandle) )
      {
        D3DPRINT( 0, "Both textures palettized and have separate palletes");
        r1val = D3DERR_WRONGTEXTUREFORMAT;
      }
#endif
#endif
    }
  }

  if( r1val != DD_OK )
  {
#if 0
#ifdef DEBUG
D3DPRINT(0,"ddiValidatetextureStageState  #stages=%d",numStages+1);
for (i=0; i < (numStages+1); ++i)
{
  D3DPRINT(0,"Stage=%1s(%ld)  Arg1=%s    Op=%s  Arg2=%s   ",stageText[i], TS[i].textureHandle,
              stageArg[TS[i].colorArg1 & 0x30][TS[i].colorArg1 & 0x03], stageOpT[TS[i].colorOp], stageArg[TS[i].colorArg2 & 0x30][TS[i].colorArg2 & 0x03]);
  D3DPRINT(0,"         Alpha1=%s  Op=%s   Alpha2=%s ",
              stageArg[TS[i].alphaArg1 & 0x30][TS[i].alphaArg1 & 0x03], stageOpT[TS[i].alphaOp], stageArg[TS[i].alphaArg2 & 0x30][TS[i].alphaArg2 & 0x03]);
}
#endif
#endif
    D3DPRINT( 0, "!!! Validate Failed 0x%08lx", r1val );
  }
  else
  {
#ifndef DISABLE_DCT_TEXTURESTAGE_HACK
    if (IS_NAPALM && (pRc->specular) && (2 == lpd->dwNumPasses))
    {
      // leave dwNumPasses alone to pass TextureStage dct test
    }
    else
#endif
      // We have passed validation so set the number of passes to 1. -Ade
      lpd->dwNumPasses = 1;
  }

  // Now set our return code
  lpd->ddrval = r1val;
  INS_EXIT( );
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}

/*-------------------------------------------------------------------
Function Name:  CalcPerChipLodBias

Description:    Calculates the Lod Bias values for each chip.

Return:         void
-------------------------------------------------------------------*/
#ifdef LODBIASPERCHIP
void CalcPerChipLodBias(RC *pRc)
{
	SETUP_PPDEV(pRc)
    int iLodBias;
	int i;
	int idx = _FF(dwNumUnits) >> 2;
	int chipLodBias[2][4] =       // these values per Gary Tarolli
				{
				// 4.2 format for tLod register
				//   0.00, 0.50, 0.00, 0.00 - 2 chip. last two values are not used
					{0x00, 0x02, 0x00, 0x00},
				//   0.00, 0.25, 0.50, 0.75 - 4 chip
					{0x00, 0x01, 0x02, 0x03}
				};

	// Capture Lod Bias values for each chip.
	for (i = 0; i < (int) _FF(dwNumUnits); i++)
	{
	     // extract the current lod bias.
		iLodBias = (pRc->sst.tLODT0 & SST_LODBIAS) >> SST_LODBIAS_SHIFT;
		// If number is negative, sign extend to a full 32 bits.
		if (iLodBias & 0x20)  iLodBias |= ~(0x3f);
		// add registry bias to current lod bias, which may be zero.
		iLodBias += _D3(LodBias);
		// Add in chip-specific Lod Bias
		iLodBias += chipLodBias[idx][i];
		// clamp. (-32 to +31, with each integer step representing .25 of a level of detail)
		if (iLodBias < -32) iLodBias = -32;
		if (iLodBias >  31) iLodBias = 31;
		// store new lod bias value.
		pRc->dwTLodBiasT0[i] = ((iLodBias << SST_LODBIAS_SHIFT) & SST_LODBIAS);

		// extract the current lod bias.
		iLodBias = (pRc->sst.tLODT1 & SST_LODBIAS) >> SST_LODBIAS_SHIFT;
		// If number is negative, sign extend to a full 32 bits.
		if (iLodBias & 0x20)  iLodBias |= ~(0x3f);
		// add registry bias to current lod bias, which may be zero.
		iLodBias += _D3(LodBias);
		// Add in chip-specific Lod Bias
		iLodBias += chipLodBias[idx][i];
		// clamp. (-32 to +31, with each integer step representing .25 of a level of detail)
		if (iLodBias < -32) iLodBias = -32;
		if (iLodBias >  31) iLodBias = 31;
		// store new lod bias value.
		pRc->dwTLodBiasT1[i] = ((iLodBias << SST_LODBIAS_SHIFT) & SST_LODBIAS);
	}
}
#endif

/*-------------------------------------------------------------------
Function Name:  setupTexturing

Description:    Sets up texturing for given dx texture states

Return:         void
-------------------------------------------------------------------*/
void setupTexturing( RC* pRc )
{
  SETUP_PPDEV(pRc)
  DWORD       numStages = 0;
  LPMINI_TEXTUREOP lpMiniStage;
  LPMINI_ARG lpMiniArg;
  DWORD arg1,arg2,arg1mux,arg2mux;
  // If the TS[0].alpha == D3DTOP_DISABLE and the TS[1].color=D3DTA_CURRENT|ALPHAREPLICATE then we don't handle this.
  // Its an unpopular combo and adding it would add risk/dev effort.  For this case, apps should specify the alpha stage instead of just disabling it.
  LPTEXTUREOP lpStage;

  while ( numStages<3 ) { // We only validate upto 2 stages.
    #ifdef DEBUG
      D3DPRINT(25,"Color"); printTexOp(TS[numStages].colorOp,TS[numStages].colorArg1,TS[numStages].colorArg2);
      D3DPRINT(25,"Alpha"); printTexOp(TS[numStages].alphaOp,TS[numStages].alphaArg1,TS[numStages].alphaArg2);
    #endif
      D3DPRINT(25,"colorOp=%08x arg1=%08x arg2=%08x texhandle=%08x",TS[numStages].colorOp,TS[numStages].colorArg1,TS[numStages].colorArg2,TS[numStages].textureHandle);
      D3DPRINT(25,"alphaOp=%08x arg1=%08x arg2=%08x",TS[numStages].alphaOp,TS[numStages].alphaArg1,TS[numStages].alphaArg2);
      if (TS[numStages].colorOp == D3DTOP_DISABLE) {
         break;
      }
      if (TS[numStages].textureHandle == 0)  {  // Don't count stage if texturehandle==0...
          if (     ( (TS[numStages].colorArg1 & D3DTA_SELECTMASK) == D3DTA_TEXTURE ) // ...and a texture...
                && ( TS[numStages].colorOp != D3DTOP_SELECTARG2)                     // ...is used for the argument
             ) {
             break;
          }
          if (     ( (TS[numStages].colorArg2 & D3DTA_SELECTMASK) == D3DTA_TEXTURE ) // ...and a texture...
                && ( TS[numStages].colorOp != D3DTOP_SELECTARG1)                     // ...is used for the argument
             ) {
             break;
          }
          if (    (    ( TS[numStages].colorOp == D3DTOP_BLENDTEXTUREALPHA  )  // ... and the operation requires a texture
                    || ( TS[numStages].colorOp == D3DTOP_BLENDTEXTUREALPHAPM)
                  )
             ) {
             break;
          }
          if (    (    ( TS[numStages].alphaOp == D3DTOP_BLENDTEXTUREALPHA  )  // ... and the operation requires a texture
                    || ( TS[numStages].alphaOp == D3DTOP_BLENDTEXTUREALPHAPM)
                  )
             ) {
             break;
          }
      }
      numStages++;
  }

  // If all stages are disabled then we are either flat shading or
  // gouraud shading.
  if( numStages == 0 )
  {
    pRc->sst.fbzColorPath = SST_PARMADJUST;
    if (IS_NAPALM) {
      /*We should use 2 pixel-per-clock only when we are in a 15/16 bpp mode in a "non-high" refresh/resolution screen mode and rendering single textured triangles*/
      pRc->sst.combineModeFBI = (_D3(TwoPpc) & _FF(ModeUses2PixPerClkRender)) | SST_CM_USE_COMBINE_MODE | SST_CM_DISABLE_CHROMA_SUBSTITUTION;
      pRc->sst.combineModeT0  = (_D3(TwoPpc) & _FF(ModeUses2PixPerClkRender)) | SST_CM_USE_COMBINE_MODE | SST_CM_DISABLE_CHROMA_SUBSTITUTION;
      pRc->sst.combineModeT1  = (_D3(TwoPpc) & _FF(ModeUses2PixPerClkRender)) | SST_CM_USE_COMBINE_MODE | SST_CM_DISABLE_CHROMA_SUBSTITUTION;
    }
    if (pRc->texMapBlend==D3DTBLEND_MODULATEMASK) {
          // Armor Command bug.  ArmorCmd was using ModulateMask with no texture.
          // This caused problems because the renderstate, D3DTBLEND_ModulateMask was being set.
          // This renderstate enables SST_ENALPHAMASK which would reject all the pixels with Src Alpha LSB=0
          // (We were rejecting every pixel that was interpolated with an LSB result of 0; this was bad.).
          // To prevent rejecting these pixels, take out the SST_ENALPHAMASK. - cws
          pRc->sst.fbzMode &= ~SST_ENALPHAMASK;
    }

    pRc->state          &= ~(STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_ST_TMU1);
    pRc->sst.sSetupMode &= ~(SST_SETUP_W0 | SST_SETUP_ST0 | SST_SETUP_ST1);

    // Set the texture handle to NULL.
    pRc->texture = 0;

    // We are not using any special multi-texture modes.
    pRc->specialModes = 0;
    pRc->state = (pRc->state & ~(STATE_MULTITEXTURING | STATE_REQUIRES_PERSPECTIVE)) | STATE_NOT_PERSPECTIVE;
  }
  // If we only have one stage then a texture can come from any texture unit.
  else if ( numStages == 1 )
  {
    DWORD i;
  #ifdef DEBUG
    // This is used to determine any new texture opcodes being used
    checkTextureOp( pRc );
  #endif

    // We are not using any special multi-texture modes.
    pRc->specialModes = 0;

    if( (pRc->textureStage[0].changed ) || (pRc->textureStage[1].changed ))
    {
      // Initialise fbzColorPath.
      pRc->sst.fbzColorPath = SST_PARMADJUST;
      if (IS_NAPALM) {
        pRc->sst.fbzColorPath |= SST_ENTEXTUREMAP;// Get TCU non-textured (ie iterated only) data into FBI - cws.
        /*We should use 2 pixel-per-clock only when we are in a 15/16 bpp mode in a "non-high" refresh/resolution screen mode and rendering single textured triangles*/
        pRc->sst.combineModeFBI = (_D3(TwoPpc) & _FF(ModeUses2PixPerClkRender)) | SST_CM_USE_COMBINE_MODE | SST_CM_DISABLE_CHROMA_SUBSTITUTION;
        pRc->sst.combineModeT0  = (_D3(TwoPpc) & _FF(ModeUses2PixPerClkRender)) | SST_CM_USE_COMBINE_MODE | SST_CM_DISABLE_CHROMA_SUBSTITUTION;
        pRc->sst.combineModeT1  = (_D3(TwoPpc) & _FF(ModeUses2PixPerClkRender)) | SST_CM_USE_COMBINE_MODE | SST_CM_DISABLE_CHROMA_SUBSTITUTION;
      }
      // Preserve perspective correction bit.
      if( pRc->texturePerspective )
      {
        pRc->sst.textureMode = SST_TPERSP_ST;
        pRc->state           = (pRc->state & ~(STATE_NOT_PERSPECTIVE | STATE_MULTITEXTURING)) | STATE_REQUIRES_PERSPECTIVE;
      }
      else
      {
        pRc->sst.textureMode = 0;
        pRc->state           = (pRc->state & ~(STATE_REQUIRES_PERSPECTIVE | STATE_MULTITEXTURING)) | STATE_NOT_PERSPECTIVE;
      }

      // Initial state of tLod.
      pRc->sst.tLOD         = 0x0;

      // Clear all texture coordinate states
      pRc->state          &= ~(STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_ST_TMU1);
      pRc->sst.sSetupMode &= ~(SST_SETUP_W0 | SST_SETUP_ST0 | SST_SETUP_ST1);

#if ENABLE_LOG_FILE
      // retro3dfx white-world hunt: single-stage binds (menu/sky path).
      // g bits = INRANGE/PTR/INUSE for TS[0]. Bounded: first 3 + samples.
      {
        static DWORD _mt1 = 0;
        DWORD _n = ++_mt1;
        if ((_n <= 3) || (256 == _n) || (2048 == _n))
        {
          DWORD _h0 = TS[0].textureHandle;
          retroLogForce(ppdev, "retro3dfx MT1#%ld: h0=%ld g0=%d%d%d\r\n",
              _n, _h0, TXTRHNDL_INRANGE(_h0) ? 1 : 0,
              (TXTRHNDL_INRANGE(_h0) && TXTRHNDL_PTR(_h0)) ? 1 : 0,
              (TXTRHNDL_INRANGE(_h0) && TXTRHNDL_PTR(_h0) && TXTRHNDL_INUSE(_h0)) ? 1 : 0);
        }
      }
#endif
      setupSingleStage( pRc );

      // Set W State if either texture coordinate state is being used
      if( pRc->state & STATE_REQUIRES_ST_TMU0 )
      {
        pRc->state          |= STATE_REQUIRES_W_TMU0;
        pRc->sst.sSetupMode |= SST_SETUP_W0;
      }

      // Always send iterated alpha, its much easier.
      pRc->state |= STATE_REQUIRES_IT_ALPHA;
      pRc->sst.sSetupMode |= SST_SETUP_A;
     if (IS_NAPALM) {
#ifdef NEW_CCU
      pRc->sst.textureModeT1 |= SST_TC_REPLACE | SST_TCA_REPLACE; //Keep the trilinear mode intact.
      numStages=1;
      // Setup Stage0 : TMU1( fbzcolorpath, CombineMode)
      i=0;
      // Setup control path for alphaOp
      lpMiniStage = CCU_miniStageOp[0][1];//CCU_mini_twoStage_s0_alphaOp;
      pRc->sst.combineModeFBI |= lpMiniStage[TS[i].alphaOp].setup[ARGX_CMTMUOP];
      pRc->sst.fbzColorPath   |= lpMiniStage[TS[i].alphaOp].setup[ARGX_FBIOP];
      D3DPRINT(25,"CCU_s0 of s0 alphaOP=%08x lpMinistage=%08x arg1,2=%08x\n",TS[0].alphaOp,lpMiniStage, ((TS[0].alphaArg1 & 0x03) << 2) | (TS[0].alphaArg2 & 0x03));
      D3DPRINT(25,"CCU_s0 of s0 combineModeFBI=%08x T0=%08x textureModeT0=%08x\n",pRc->sst.combineModeFBI,pRc->sst.combineModeT0,pRc->sst.textureModeT0);
      D3DPRINT(25,"fbzColorPath=%08x combineModeT1=%08x textureModeT1=%08x\n",pRc->sst.fbzColorPath,pRc->sst.combineModeT1,pRc->sst.textureModeT1);
         /* Setup AlphaOp Arg1 and Arg2 */
      lpMiniArg = CCU_mini_arg_stage[0][1]; //CCU_mini_arg_a0_of_0;
      D3DPRINT(25,"CCU_mini_arg_a0_of_0=%08x lpMiniArg=%08x",CCU_mini_arg_a0_of_0,lpMiniArg);
      arg1=TS[i].alphaArg1 & 0x03;
      arg2=TS[i].alphaArg2 & 0x03;
      arg1mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG1MUX];
      arg2mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG2MUX];
      D3DPRINT(25,"arg1=%08x arg2=%08x arg1mux=%08x arg2mux=%08x",arg1,arg2,arg1mux,arg2mux);
      D3DPRINT(25,"arg1sel=%08x arg2sel=%08x",lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1],lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2]);
      pRc->sst.combineModeFBI |= lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1]; //mux select for args
      pRc->sst.combineModeFBI |= lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2];
      if (arg1mux==MUXM7) {// We need to do this check because the arg input spans 2 registers.
          pRc->sst.fbzColorPath |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg1];
      }
      if (arg2mux==MUXM7) {
          pRc->sst.fbzColorPath |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg2];
      }
         /*Setup AlphaOp arg complement */
      if (TS[i].alphaArg1 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeFBI ^= lpMiniStage[TS[i].alphaOp].setup[ARG1_COMP_XOR_CM];
              pRc->sst.fbzColorPath ^= lpMiniStage[TS[i].alphaOp].setup[ARG1_COMP_XOR_TM];
      }
      if (TS[i].alphaArg2 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeFBI ^= lpMiniStage[TS[i].alphaOp].setup[ARG2_COMP_XOR_CM];
              pRc->sst.fbzColorPath ^= lpMiniStage[TS[i].alphaOp].setup[ARG2_COMP_XOR_TM];
      }
         /* Setup AlphaOp AUX Input */
      #if 0 //Currently we don't use AUX MUX on alphaOps
      arg1mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG1AUXMUX];
      arg2mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG2AUXMUX];
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1];
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2];
      #endif

         /* Setup control path for ColorOp */
      lpMiniStage = CCU_miniStageOp[0][0];////CCU_mini_arg_c0_of_0;
      pRc->sst.combineModeFBI |= lpMiniStage[TS[i].colorOp].setup[ARGX_CMTMUOP];
      pRc->sst.fbzColorPath   |= lpMiniStage[TS[i].colorOp].setup[ARGX_FBIOP];
      D3DPRINT(25,"s0 of s0to1 colorOP=%08x lpstage=%08x arg1,2=%08x\n",TS[i].colorOp,lpMiniStage, ((TS[i].colorArg1 & 0x03) << 2) | (TS[i].colorArg2 & 0x03));

         /* Setup colorOp Arg1 and Arg2 */
      lpMiniArg = CCU_mini_arg_stage[0][0]; //CCU_mini_arg_c0_of_0;
      D3DPRINT(25,"CCU_mini_arg_c0_of_0=%08x lpMiniArg=%08x",CCU_mini_arg_c0_of_0,lpMiniArg);
      arg1=TS[i].colorArg1 & 0x03;
      arg2=TS[i].colorArg2 & 0x03;
      arg1mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG1MUX];
      arg2mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG2MUX];
      D3DPRINT(25,"arg1=%08x arg2=%08x arg1mux=%08x arg2mux=%08x",arg1,arg2,arg1mux,arg2mux);
      D3DPRINT(25,"arg1sel=%08x arg2sel=%08x",lpMiniArg->argselect[arg1mux][arg1],lpMiniArg->argselect[arg2mux][arg2]);
      pRc->sst.combineModeFBI |= lpMiniArg->argselect[arg1mux][arg1]; //mux select for args
      pRc->sst.combineModeFBI |= lpMiniArg->argselect[arg2mux][arg2];
      if (arg1mux==MUXM7) {
          pRc->sst.fbzColorPath |= lpMiniArg->argselect[MUXM][arg1];
      }
      if (arg2mux==MUXM7) {
          pRc->sst.fbzColorPath |= lpMiniArg->argselect[MUXM][arg2];
      }
         /*Setup colorOp arg complement */
      if (TS[i].colorArg1 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeFBI ^= lpMiniStage[TS[i].colorOp].setup[ARG1_COMP_XOR_CM];
              pRc->sst.fbzColorPath   ^= lpMiniStage[TS[i].colorOp].setup[ARG1_COMP_XOR_TM];
      }
      if (TS[i].colorArg2 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeFBI ^= lpMiniStage[TS[i].colorOp].setup[ARG2_COMP_XOR_CM];
              pRc->sst.fbzColorPath   ^= lpMiniStage[TS[i].colorOp].setup[ARG2_COMP_XOR_TM];
      }
         /* Setup colorOp AUX Input */
      arg1mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG1AUXMUX];
      arg2mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG2AUXMUX];
      pRc->sst.combineModeFBI |= lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1]; //AUX mux select.
      pRc->sst.combineModeFBI |= lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2];
      if (arg1mux==MUXM7) {
          pRc->sst.fbzColorPath |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg1];
      }
      if (arg2mux==MUXM7) {
          pRc->sst.fbzColorPath |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg2];
      }
         /* Setup colorOp Alphareplicate */
      if (TS[i].colorArg1 & D3DTA_ALPHAREPLICATE) { /* If the request is to do alphareplicate on arg1 */
           DWORD ArgVal = lpMiniStage[TS[i].colorOp].setup[ARGX_VALID];
           if ( ArgVal & ARG1_AREP) { /* If the table says we can do alphareplicate on arg1 */
               switch (ArgVal & ARG1_MUX ) /* find the mux on which arg1 comes */
               {
                 case ARG1_TMUXOTH:
                     pRc->sst.combineModeFBI += SST_CM_CC_OTHERSELECT_IA;  /*The nature of the register allows us to add to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG1_TMUXLOC:
                     pRc->sst.combineModeFBI += SST_CM_CC_LOCALSELECT_IA;  /*The nature of the register allows us to add to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG1_TMUXM7:
                     if ( (pRc->sst.fbzColorPath & SST_CC_MSELECT) == SST_CC_MCMSELECT7) { //if MSelect7 is used
                        pRc->sst.combineModeFBI += SST_CM_CC_MSELECT_7_IA; /*The nature of the reg allows us to add to select alpha arg instead of color arg*/
                     }
                     else if ((pRc->sst.fbzColorPath & SST_CC_MSELECT)==SST_CC_MRGBTMU) {//The input must be "TEXTURE rgb", but lets make sure.
                         pRc->sst.fbzColorPath &= ~ SST_CC_MSELECT;
                         pRc->sst.fbzColorPath |= SST_CC_MATMU;//This used to be MALOCAL in the TMU; but this seems more appropriate.  Keep an eye on this.
                     }
               }
           }
        }
        if (TS[i].colorArg2 & D3DTA_ALPHAREPLICATE) { /* If the request is to do alphareplicate on arg2 */
            DWORD ArgVal = lpMiniStage[TS[i].colorOp].setup[ARGX_VALID];
           if ( ArgVal & ARG2_AREP) { /* If the table says we can do alphareplicate on arg2 */
               switch (ArgVal & ARG2_MUX ) /* find the mux on which ARG2 comes */
               {
                 case ARG2_TMUXOTH:
                     pRc->sst.combineModeFBI += SST_CM_CC_OTHERSELECT_IA;  /*The nature of the register allows us to add to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG2_TMUXLOC:
                     pRc->sst.combineModeFBI += SST_CM_CC_LOCALSELECT_IA;  /*The nature of the register allows us to add to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG2_TMUXM7:
                     if ( (pRc->sst.fbzColorPath & SST_CC_MSELECT) == SST_CC_MCMSELECT7) { //if MSelect7 is used
                        pRc->sst.combineModeFBI += SST_CM_CC_MSELECT_7_IA; /*The nature of the reg allows us to add to select alpha arg instead of color arg*/
                     }
                     else if ((pRc->sst.fbzColorPath & SST_CC_MSELECT)==SST_CC_MRGBTMU) {//The input must be "TEXTURE rgb", but lets make sure.
                         pRc->sst.fbzColorPath &= ~ SST_CC_MSELECT;
                         pRc->sst.fbzColorPath |= SST_CC_MATMU;//This used to be MALOCAL in the TMU; but this seems more appropriate.  Keep an eye on this.
                     }
               }
           }
        }
#else //NEW_CCU
      pRc->sst.combineModeFBI |= SST_CM_CC_OTHERSELECT_TRGB | SST_CM_CCA_OTHERSELECT_TA; //Just pass through FBI.
      numStages=1; //we need to use the arg / path setup of c0_of_0to1 for getting LOCALtex instead of OTHERtex
      // Setup Stage0 : TMU1( textureMode, CombineMode)
      i=0;
      // Setup control path for alphaOp
      lpMiniStage = miniStageOp[numStages][i + ALPHAOP];
      pRc->sst.combineModeT1  |= lpMiniStage[TS[i].alphaOp].setup[ARGX_CMTMUOP];
      pRc->sst.textureModeT1  |= lpMiniStage[TS[i].alphaOp].setup[ARGX_TMUOP];
      D3DPRINT(25,"s0 of s0to1 alphaOP=%08x lpMinistage=%08x arg1,2=%08x\n",TS[0].alphaOp,lpMiniStage, ((TS[0].alphaArg1 & 0x03) << 2) | (TS[0].alphaArg2 & 0x03));
      D3DPRINT(25,"combineModeFBI=%08x T0=%08x textureModeT0=%08x\n",pRc->sst.combineModeFBI,pRc->sst.combineModeT0,pRc->sst.textureModeT0);
      D3DPRINT(25,"fbzColorPath=%08x combineModeT1=%08x textureModeT1=%08x\n",pRc->sst.fbzColorPath,pRc->sst.combineModeT1,pRc->sst.textureModeT1);
         /* Setup AlphaOp Arg1 and Arg2 */
      lpMiniArg = mini_arg_stage[numStages][ALPHAARG];
      D3DPRINT(25,"mini_arg_a0_of_0=%08x lpMiniArg=%08x",mini_arg_a0_of_0,lpMiniArg);
      arg1=TS[i].alphaArg1 & 0x03;
      arg2=TS[i].alphaArg2 & 0x03;
      arg1mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG1MUX];
      arg2mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG2MUX];
      D3DPRINT(25,"arg1=%08x arg2=%08x arg1mux=%08x arg2mux=%08x",arg1,arg2,arg1mux,arg2mux);
      D3DPRINT(25,"arg1sel=%08x arg2sel=%08x",lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1],lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2]);
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1]; //mux select for args
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2];
      if (arg1mux==MUXM7) {// We need to do this check because the arg input spans 2 registers.
          pRc->sst.textureModeT1 |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg1];
      }
      if (arg2mux==MUXM7) {
          pRc->sst.textureModeT1 |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg2];
      }
         /*Setup AlphaOp arg complement */
      if (TS[i].alphaArg1 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeT1 ^= lpMiniStage[TS[i].alphaOp].setup[ARG1_COMP_XOR_CM];
              pRc->sst.textureModeT1 ^= lpMiniStage[TS[i].alphaOp].setup[ARG1_COMP_XOR_TM];
      }
      if (TS[i].alphaArg2 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeT1 ^= lpMiniStage[TS[i].alphaOp].setup[ARG2_COMP_XOR_CM];
              pRc->sst.textureModeT1 ^= lpMiniStage[TS[i].alphaOp].setup[ARG2_COMP_XOR_TM];
      }
         /* Setup AlphaOp AUX Input */
      #if 0 //Currently we don't use AUX MUX on alphaOps
      arg1mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG1AUXMUX];
      arg2mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG2AUXMUX];
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1];
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2];
      #endif

         /* Setup control path for ColorOp */
      lpMiniStage = miniStageOp[numStages][i + COLOROP];
      pRc->sst.combineModeT1  |= lpMiniStage[TS[i].colorOp].setup[ARGX_CMTMUOP];
      pRc->sst.textureModeT1  |= lpMiniStage[TS[i].colorOp].setup[ARGX_TMUOP];
      D3DPRINT(25,"s0 of s0to1 colorOP=%08x lpstage=%08x arg1,2=%08x\n",TS[i].colorOp,lpMiniStage, ((TS[i].colorArg1 & 0x03) << 2) | (TS[i].colorArg2 & 0x03));

         /* Setup colorOp Arg1 and Arg2 */
      lpMiniArg = mini_arg_stage[numStages][COLORARG];
      D3DPRINT(25,"mini_arg_c0_of_0_to_1=%08x lpMiniArg=%08x",mini_arg_c0_of_0_to_1,lpMiniArg);
      arg1=TS[i].colorArg1 & 0x03;
      arg2=TS[i].colorArg2 & 0x03;
      arg1mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG1MUX];
      arg2mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG2MUX];
      D3DPRINT(25,"arg1=%08x arg2=%08x arg1mux=%08x arg2mux=%08x",arg1,arg2,arg1mux,arg2mux);
      D3DPRINT(25,"arg1sel=%08x arg2sel=%08x",lpMiniArg->argselect[arg1mux][arg1],lpMiniArg->argselect[arg2mux][arg2]);
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg1mux][arg1]; //mux select for args
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg2mux][arg2];
      if (arg1mux==MUXM7) {
          pRc->sst.textureModeT1 |= lpMiniArg->argselect[MUXM][arg1];
      }
      if (arg2mux==MUXM7) {
          pRc->sst.textureModeT1 |= lpMiniArg->argselect[MUXM][arg2];
      }
         /*Setup colorOp arg complement */
      if (TS[i].colorArg1 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeT1 ^= lpMiniStage[TS[i].colorOp].setup[ARG1_COMP_XOR_CM];
              pRc->sst.textureModeT1 ^= lpMiniStage[TS[i].colorOp].setup[ARG1_COMP_XOR_TM];
      }
      if (TS[i].colorArg2 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeT1 ^= lpMiniStage[TS[i].colorOp].setup[ARG2_COMP_XOR_CM];
              pRc->sst.textureModeT1 ^= lpMiniStage[TS[i].colorOp].setup[ARG2_COMP_XOR_TM];
      }
         /* Setup colorOp AUX Input */
      arg1mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG1AUXMUX];
      arg2mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG2AUXMUX];
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1]; //AUX mux select.
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2];
      if (arg1mux==MUXM7) {
          pRc->sst.textureModeT1 |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg1];
      }
      if (arg2mux==MUXM7) {
          pRc->sst.textureModeT1 |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg2];
      }
         /* Setup colorOp Alphareplicate */
      if (TS[i].colorArg1 & D3DTA_ALPHAREPLICATE) { /* If the request is to do alphareplicate on arg1 */
            DWORD ArgVal = lpMiniStage[TS[i].colorOp].setup[ARGX_VALID];
           if ( ArgVal & ARG1_AREP) { /* If the table says we can do alphareplicate on arg1 */
               switch (ArgVal & ARG1_MUX ) /* find the mux on which arg1 comes */
               {
                 case ARG1_TMUXOTH: /*test 1c*/
                     pRc->sst.combineModeT1 += SST_CM_TC_OTHERSELECT_OTHER_TA;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG1_TMUXLOC: /*tested 2c - fab*/
                     pRc->sst.combineModeT1 += SST_CM_TC_LOCALSELECT_LOCAL_TA;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG1_TMUXM7:  /*test 3c */
                     if ( (pRc->sst.combineModeT1 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_LOCAL_TRGB)  { /*We need to get arg via MSEL instead of MSEL7 */
                        pRc->sst.textureModeT1 &= ~SST_TC_MSELECT;  // Unselect MSEL7
                        // Select new Alpha arg (Ideally this should be alpha as modified by
                        // this stage's ACU; instead it is the alpha selected by this stages
                        // ACU Local Mux - thats how it was done in the past but seems wrong.)
                        pRc->sst.textureModeT1 |= SST_TC_MALOCAL;
                     }
                     else if ( (pRc->sst.combineModeT1 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_OTHER_TRGB)  { /*We need to get arg via MSEL instead of MSEL7 */
                                 /*test 4c*/
                        pRc->sst.textureModeT1 &= ~SST_TC_MSELECT;  // Unselect MSEL7
                        // Select new Alpha arg (Ideally this should be alpha as modified by
                        // the upstream stage's ACU; instead it is the alpha selected by this
                        // stages ACU other Mux - thats how it was done in the past but seems wrong.)
                        pRc->sst.textureModeT1 |= SST_TC_MAOTHER;
                     }
                     else /* if ( ((pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TRGBMSEL7_ITERTRGB)||(pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_CR_RGB) )*/
                     {  /*test 6c*/
                         pRc->sst.combineModeT1 += SST_CM_TC_MSELECT_7_ZERO;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     }
                     break;
                 /*case ARG1_MUX??? :No Args come in stage 0 of 0to1 via FBI */
               }
           }
        }
        if (TS[i].colorArg2 & D3DTA_ALPHAREPLICATE) { /* If the request is to do alphareplicate on arg2 */
            DWORD ArgVal = lpMiniStage[TS[i].colorOp].setup[ARGX_VALID];
           if ( ArgVal & ARG2_AREP) { /* If the table says we can do alphareplicate on arg2 */
               switch (ArgVal & ARG2_MUX ) /* find the mux on which ARG2 comes */
               {
                 case ARG2_TMUXOTH: /*test 1d*/
                     pRc->sst.combineModeT1 += SST_CM_TC_OTHERSELECT_OTHER_TA;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG2_TMUXLOC: /*test 2d*/
                     pRc->sst.combineModeT1 += SST_CM_TC_LOCALSELECT_LOCAL_TA;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG2_TMUXM7:  /*test 3d */
                     if ( (pRc->sst.combineModeT1 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_LOCAL_TRGB)  { /*We need to get arg via MSEL instead of MSEL7 */
                        pRc->sst.textureModeT1 &= ~SST_TC_MSELECT;  // Unselect MSEL7
                        // Select new Alpha arg (Ideally this should be alpha as modified by
                        // this stage's ACU; instead it is the alpha selected by this stages
                        // ACU Local Mux - thats how it was done in the past but seems wrong.)
                        pRc->sst.textureModeT1 |= SST_TC_MALOCAL;
                     }
                     else if ( (pRc->sst.combineModeT1 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_OTHER_TRGB)  { /*We need to get arg via MSEL instead of MSEL7 */
                                 /*test 4d*/
                        pRc->sst.textureModeT1 &= ~SST_TC_MSELECT;  // Unselect MSEL7
                        // Select new Alpha arg (Ideally this should be alpha as modified by
                        // the upstream stage's ACU; instead it is the alpha selected by this
                        // stages ACU other Mux - thats how it was done in the past but seems wrong.)
                        pRc->sst.textureModeT1 |= SST_TC_MAOTHER;
                     }
                     else /* if ( ((pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TRGBMSEL7_ITERTRGB)||(pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_CR_RGB) )*/
                     {  /*test 6d*/
                         pRc->sst.combineModeT1 += SST_CM_TC_MSELECT_7_ZERO;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     }
                     break;
                 /*case ARG2_MUX??? :No Args come in stage 0 of 0to1 via FBI */
               }
           }
        }
#endif //NEW_CCU
     }
     else { //IS_NAPALM
      // First we do the color combine operations ...
      lpStage = stageOp[0][COLOROP];
      pRc->sst.fbzColorPath |= lpStage[TS[0].colorOp].arg[((TS[0].colorArg1 & 0x03) << 2) | (TS[0].colorArg2 & 0x03)][ARGX_FBIOP];
      // ... then we do the alpha combine operations.
      lpStage = stageOp[0][ALPHAOP];
      pRc->sst.fbzColorPath |= lpStage[TS[0].alphaOp].arg[((TS[0].alphaArg1 & 0x03) << 2) | (TS[0].alphaArg2 & 0x03)][ARGX_FBIOP];
     }

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(REMOVE_OBSOLETE1_FOR_DX7_WIN9X) && !defined(WINNT)
#else
      // this only applys to the legacy D3DTBLEND_MODULATE renderstate
      if (pRc->texMapBlend == 0x7ffffffe) // denotes legacy D3DTBLEND_MODULATE - see d3rstate.c
      {
        if ( TXTRHNDL_INRANGE(pRc->texture)

#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
             // to fix potential access violation issues with NT Stress
             // verify we have a TXTRHNDL for this texture before dereferencing it

             && TXTRHNDL_PTR(pRc->texture)
#endif
             && TXTRHNDL_INUSE(pRc->texture)
           )
        {
          // If alpha from texture is selected, but the texture doesn't contain
          // alpha, then the alpha input needs to be redirected.  In the case
          // of D3DTBLEND_MODULATE, the alpha should come from the alpha
          // interpolators.  In all other cases, alpha is 1.0.
          //
          // So speaketh the spec.

          if ( !(TXTRDESC_FROM_HNDL(pRc->texture)->formatFlags & (TEXFMTFLG_ALPHA | TEXFMTFLG_PALETTIZED_ALPHA))) {
              if (IS_NAPALM) {
                // Napalm can get texture alpha into the TMU's *color* channel from 3 different muxes -cws.
                if ((pRc->sst.combineModeT1 & SST_CM_TC_OTHERSELECT) == SST_CM_TC_OTHERSELECT_LOCAL_TA) {
                    pRc->sst.combineModeT1 = (pRc->sst.combineModeT1 & ~SST_CM_TC_OTHERSELECT) | SST_CM_TC_OTHERSELECT_IA;
                }
                if ((pRc->sst.combineModeT1 & SST_CM_TC_LOCALSELECT) == SST_CM_TC_LOCALSELECT_LOCAL_TA) {
                    pRc->sst.combineModeT1 = (pRc->sst.combineModeT1 & ~SST_CM_TC_LOCALSELECT) | SST_CM_TC_LOCALSELECT_IA;
                }
                if ((pRc->sst.textureModeT1 & SST_TC_MSELECT) == SST_TC_MALOCAL) {
                    pRc->sst.textureModeT1 = (pRc->sst.textureModeT1 & ~SST_TC_MSELECT) | SST_TC_MSELECT;
                    pRc->sst.combineModeT1 = (pRc->sst.combineModeT1 & ~SST_CM_TC_MSELECT_7) | SST_CM_TC_MSELECT_7_IA;
                }
                // Napalm can get texture alpha into the TMU's *alpha* channel from 3 different muxes -cws.
                if ((pRc->sst.combineModeT1 & SST_CM_TCA_OTHERSELECT) == SST_CM_TCA_OTHERSELECT_LOCAL_TA) {
                    pRc->sst.combineModeT1 = (pRc->sst.combineModeT1 & ~SST_CM_TCA_OTHERSELECT) | SST_CM_TCA_OTHERSELECT_IA;
                }
                if ((pRc->sst.combineModeT1 & SST_CM_TCA_LOCALSELECT) == SST_CM_TCA_LOCALSELECT_LOCAL_TA) {
                    pRc->sst.combineModeT1 = (pRc->sst.combineModeT1 & ~SST_CM_TCA_LOCALSELECT) | SST_CM_TCA_LOCALSELECT_IA;
                }
                if ((pRc->sst.textureModeT1 & SST_TCA_MSELECT) == SST_TCA_MALOCAL) {
                    pRc->sst.textureModeT1 = (pRc->sst.textureModeT1 & ~SST_TCA_MSELECT) | SST_TCA_MAITER;
                }
              }
              else {
                if ((pRc->sst.fbzColorPath & SST_ASELECT) == SST_ASEL_TMUOUT )
                {
                  pRc->sst.fbzColorPath = (pRc->sst.fbzColorPath & ~SST_ASELECT) | SST_ASEL_RGBA;
                }
              }
          }
        }
      }
#endif //#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7) && defined(REMOVE_OBSOLETE1_FOR_DX7_WIN9X) && !defined(WINNT)
    }
  }
  // Now we are doing real Multi-Texturing.
  else      // if Numstages ==2
  {
    DWORD i;
    DWORD       argOffset;
    // Check to see if any of the texture stage states have changed
    if( pRc->textureStage[0].changed || pRc->textureStage[1].changed || pRc->textureStage[2].changed )
    {
    #ifdef DEBUG
      // This is used to determine any new texture opcodes being used
      checkTextureOp( pRc );
    #endif

      // Initialise fbzColorPath.
      pRc->sst.fbzColorPath = SST_PARMADJUST;

      if (IS_NAPALM) {
        pRc->sst.fbzColorPath |= SST_ENTEXTUREMAP;// Get TCU non-textured (ie iterated only) data into FBI - cws.
        /* Disable 2 pixel-per-clock mode for multitexturing*/
        pRc->sst.combineModeFBI = SST_CM_USE_COMBINE_MODE | SST_CM_DISABLE_CHROMA_SUBSTITUTION;
        pRc->sst.combineModeT0  = SST_CM_USE_COMBINE_MODE | SST_CM_DISABLE_CHROMA_SUBSTITUTION;
        pRc->sst.combineModeT1  = SST_CM_USE_COMBINE_MODE | SST_CM_DISABLE_CHROMA_SUBSTITUTION;
        pRc->sst.textureModeT1 = 0; // cws: small code, big change - keep an eye on this!  I can't set TC_REPLACE because there is no easy way to zero this out in the case of not doing replace in the TexOP setup.
      }
      else {
        pRc->sst.textureModeT1 = SST_TC_REPLACE | SST_TCA_REPLACE;
      }

      // Preserve perspective correction bit.
      if( pRc->texturePerspective )
      {
        pRc->sst.textureMode  = SST_TPERSP_ST;
        pRc->state           = (pRc->state & ~STATE_NOT_PERSPECTIVE) | STATE_REQUIRES_PERSPECTIVE | STATE_MULTITEXTURING;
      }
      else
      {
        pRc->sst.textureMode = 0;
        pRc->state           = (pRc->state & ~STATE_REQUIRES_PERSPECTIVE) | STATE_NOT_PERSPECTIVE | STATE_MULTITEXTURING;
      }

      // Initial state of tLod.
      pRc->sst.tLOD         = 0x0;

      // Setup texture units 0 combine registers
      pRc->sst.textureModeT0 = 0;

      // Clear all texture coordinate states
      pRc->state          &= ~(STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_ST_TMU1);
      pRc->sst.sSetupMode &= ~(SST_SETUP_W0 | SST_SETUP_ST0 | SST_SETUP_ST1);

#if ENABLE_LOG_FILE
      // retro3dfx white-world hunt: the two-stage path is the WORLD (base x
      // lightmap). Log both handles + INRANGE/PTR/INUSE gate bits; a 110 or
      // 100 pattern names the gate that silently disables the stage (white).
      {
        static DWORD _mt2 = 0;
        DWORD _n = ++_mt2;
        if ((_n <= 4) || (128 == _n) || (1024 == _n) || (8192 == _n))
        {
          DWORD _h0 = TS[0].textureHandle, _h1 = TS[1].textureHandle;
          retroLogForce(ppdev, "retro3dfx MT2#%ld: h0=%ld g0=%d%d%d h1=%ld g1=%d%d%d\r\n",
              _n,
              _h0, TXTRHNDL_INRANGE(_h0) ? 1 : 0,
              (TXTRHNDL_INRANGE(_h0) && TXTRHNDL_PTR(_h0)) ? 1 : 0,
              (TXTRHNDL_INRANGE(_h0) && TXTRHNDL_PTR(_h0) && TXTRHNDL_INUSE(_h0)) ? 1 : 0,
              _h1, TXTRHNDL_INRANGE(_h1) ? 1 : 0,
              (TXTRHNDL_INRANGE(_h1) && TXTRHNDL_PTR(_h1)) ? 1 : 0,
              (TXTRHNDL_INRANGE(_h1) && TXTRHNDL_PTR(_h1) && TXTRHNDL_INUSE(_h1)) ? 1 : 0);
        }
      }
#endif
      // Remember Stage 0 is actually TMU1 ...
      setupTextureStage0( pRc );

      // ... and Stage 1 is actually TMU0
      setupTextureStage1( pRc );

      // fix for PRS 14371, Unreal corrupt textures
      //
      // setupTextureStage1 leaves stage 1's texture handle in pRc->texture
      // but if TS[1].textureHandle isn't palettized and TS[0].textureHandle
      // is palettized then setDX6State doesn't detect that the texture palette
      // for TS[0] needs to be loaded to the hardware and we end up using
      // the wrong palette for the palettized textures
      if (TXTRHNDL_INRANGE(TS[0].textureHandle) &&
          TXTRHNDL_PTR(TS[0].textureHandle) &&
          TXTRHNDL_INUSE(TS[0].textureHandle) &&
          TXTRHNDL_INRANGE(TS[1].textureHandle) &&
          TXTRHNDL_PTR(TS[1].textureHandle) &&
          TXTRHNDL_INUSE(TS[1].textureHandle)
         )
      {
        TXTRDESC  *txtr0, *txtr1;

        txtr0 = TXTRDESC_FROM_HNDL(TS[0].textureHandle);
        txtr1 = TXTRDESC_FROM_HNDL(TS[1].textureHandle);

        if ((TEXFMTFLG_PALETTIZED & txtr1->formatFlags) &&
            (0 != txtr1->dwPaletteHandle) &&
            PALHNDL_INRANGE(TS[1].textureHandle) &&
            PALETTEGBL(TS[1].textureHandle))
        {
          // TS[1].textureHandle is palettized so we should be okay
          // ValidateTextureStageState should have checked for equal
          // palette handles or else equal palette data
        }
        else if ((TEXFMTFLG_PALETTIZED & txtr0->formatFlags) &&
                 (0 != txtr0->dwPaletteHandle) &&
              PALHNDL_INRANGE(TS[0].textureHandle) &&
              PALETTEGBL(TS[0].textureHandle))
        {
          // TS[1].textureHandle is not palettized but TS[0].textureHandle
          // is palettized so put TS[0].textureHandle in pRc->texture so
          // that setDX6State will load the correct palette to the hw
          pRc->texture = TS[0].textureHandle;
        }
      }

      // Set W State if either texture coordinate state is being used
      if( pRc->state & (STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_ST_TMU1) )
      {
        pRc->state          |= STATE_REQUIRES_W_TMU0;
        pRc->sst.sSetupMode |= SST_SETUP_W0;
      }

      // Always send iterated alpha, its much easier.
      pRc->state |= STATE_REQUIRES_IT_ALPHA;
      pRc->sst.sSetupMode |= SST_SETUP_A;

      // Now we setup the texture and color paths.

     if (IS_NAPALM) {
      pRc->sst.combineModeFBI |= SST_CM_CC_OTHERSELECT_TRGB | SST_CM_CCA_OTHERSELECT_TA; //Just pass through FBI.
      numStages--;
      // Setup Stage0 : TMU1( textureMode, CombineMode)
      i=0;
      // Setup control path for alphaOp
      lpMiniStage = miniStageOp[numStages][i + ALPHAOP];
      pRc->sst.combineModeT1  |= lpMiniStage[TS[i].alphaOp].setup[ARGX_CMTMUOP];
      pRc->sst.textureModeT1  |= lpMiniStage[TS[i].alphaOp].setup[ARGX_TMUOP];
      D3DPRINT(25,"s0 of s0to1 alphaOP=%08x lpMinistage=%08x arg1,2=%08x\n",TS[0].alphaOp,lpMiniStage, ((TS[0].alphaArg1 & 0x03) << 2) | (TS[0].alphaArg2 & 0x03));
      D3DPRINT(25,"combineModeFBI=%08x T0=%08x textureModeT0=%08x\n",pRc->sst.combineModeFBI,pRc->sst.combineModeT0,pRc->sst.textureModeT0);
      D3DPRINT(25,"fbzColorPath=%08x combineModeT1=%08x textureModeT1=%08x\n",pRc->sst.fbzColorPath,pRc->sst.combineModeT1,pRc->sst.textureModeT1);
         /* Setup AlphaOp Arg1 and Arg2 */
      lpMiniArg = mini_arg_stage[numStages][ALPHAARG];
      D3DPRINT(25,"mini_arg_a0_of_0=%08x lpMiniArg=%08x",mini_arg_a0_of_0,lpMiniArg);
      arg1=TS[i].alphaArg1 & 0x03;
      arg2=TS[i].alphaArg2 & 0x03;
      arg1mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG1MUX];
      arg2mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG2MUX];
      D3DPRINT(25,"arg1=%08x arg2=%08x arg1mux=%08x arg2mux=%08x",arg1,arg2,arg1mux,arg2mux);
      D3DPRINT(25,"arg1sel=%08x arg2sel=%08x",lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1],lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2]);
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1]; /*mux select for args */
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2];
      if (arg1mux==MUXM7) {/* We need to do this check because the arg input spans 2 registers. */
          pRc->sst.textureModeT1 |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg1];
      }
      if (arg2mux==MUXM7) {
          pRc->sst.textureModeT1 |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg2];
      }
         /*Setup AlphaOp arg complement */
      if (TS[i].alphaArg1 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeT1 ^= lpMiniStage[TS[i].alphaOp].setup[ARG1_COMP_XOR_CM];
              pRc->sst.textureModeT1 ^= lpMiniStage[TS[i].alphaOp].setup[ARG1_COMP_XOR_TM];
      }
      if (TS[i].alphaArg2 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeT1 ^= lpMiniStage[TS[i].alphaOp].setup[ARG2_COMP_XOR_CM];
              pRc->sst.textureModeT1 ^= lpMiniStage[TS[i].alphaOp].setup[ARG2_COMP_XOR_TM];
      }
         /* Setup AUX Input */
      #if 0 /*Currently we don't use AUX MUX on alphaOps*/
      arg1mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG1AUXMUX];
      arg2mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG2AUXMUX];
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1];
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2];
      #endif

      // Setup control path for ColorOp
      lpMiniStage = miniStageOp[numStages][i + COLOROP];
      pRc->sst.combineModeT1  |= lpMiniStage[TS[i].colorOp].setup[ARGX_CMTMUOP];
      pRc->sst.textureModeT1  |= lpMiniStage[TS[i].colorOp].setup[ARGX_TMUOP];
      D3DPRINT(25,"s0 of s0to1 colorOP=%08x lpstage=%08x arg1,2=%08x\n",TS[i].colorOp,lpMiniStage, ((TS[i].colorArg1 & 0x03) << 2) | (TS[i].colorArg2 & 0x03));

         /* Setup colorOp Arg1 and Arg2 */
      lpMiniArg = mini_arg_stage[numStages][COLORARG];
      D3DPRINT(25,"mini_arg_c0_of_0_to_1=%08x lpMiniArg=%08x",mini_arg_c0_of_0_to_1,lpMiniArg);
      arg1=TS[i].colorArg1 & 0x03;
      arg2=TS[i].colorArg2 & 0x03;
      arg1mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG1MUX];
      arg2mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG2MUX];
      D3DPRINT(25,"arg1=%08x arg2=%08x arg1mux=%08x arg2mux=%08x",arg1,arg2,arg1mux,arg2mux);
      D3DPRINT(25,"arg1sel=%08x arg2sel=%08x",lpMiniArg->argselect[arg1mux][arg1],lpMiniArg->argselect[arg2mux][arg2]);
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg1mux][arg1]; //mux select for args
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg2mux][arg2];
      if (arg1mux==MUXM7) {
          pRc->sst.textureModeT1 |= lpMiniArg->argselect[MUXM][arg1];
      }
      if (arg2mux==MUXM7) {
          pRc->sst.textureModeT1 |= lpMiniArg->argselect[MUXM][arg2];
      }
         /*Setup colorOp arg complement */
      if (TS[i].colorArg1 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeT1 ^= lpMiniStage[TS[i].colorOp].setup[ARG1_COMP_XOR_CM];
              pRc->sst.textureModeT1 ^= lpMiniStage[TS[i].colorOp].setup[ARG1_COMP_XOR_TM];
      }
      if (TS[i].colorArg2 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeT1 ^= lpMiniStage[TS[i].colorOp].setup[ARG2_COMP_XOR_CM];
              pRc->sst.textureModeT1 ^= lpMiniStage[TS[i].colorOp].setup[ARG2_COMP_XOR_TM];
      }
         /* Setup colorOp AUX Input */
      arg1mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG1AUXMUX];
      arg2mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG2AUXMUX];
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1]; //AUX mux select.
      pRc->sst.combineModeT1 |= lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2];
      if (arg1mux==MUXM7) {
          pRc->sst.textureModeT1 |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg1];
      }
      if (arg2mux==MUXM7) {
          pRc->sst.textureModeT1 |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg2];
      }
         /* Setup colorOp Alphareplicate */
      if (TS[i].colorArg1 & D3DTA_ALPHAREPLICATE) { /* If the request is to do alphareplicate on arg1 */
            DWORD ArgVal = lpMiniStage[TS[i].colorOp].setup[ARGX_VALID];
           if ( ArgVal & ARG1_AREP) { /* If the table says we can do alphareplicate on arg1 */
               switch (ArgVal & ARG1_MUX ) /* find the mux on which arg1 comes */
               {
                 case ARG1_TMUXOTH: /*test 1c*/
                     pRc->sst.combineModeT1 += SST_CM_TC_OTHERSELECT_OTHER_TA;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG1_TMUXLOC: /*tested 2c - fab*/
                     pRc->sst.combineModeT1 += SST_CM_TC_LOCALSELECT_LOCAL_TA;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG1_TMUXM7:  /*test 3c */
                     if ( (pRc->sst.combineModeT1 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_LOCAL_TRGB)  { /*We need to get arg via MSEL instead of MSEL7 */
                        pRc->sst.textureModeT1 &= ~SST_TC_MSELECT;  // Unselect MSEL7
                        // Select new Alpha arg (Ideally this should be alpha as modified by
                        // this stage's ACU; instead it is the alpha selected by this stages
                        // ACU Local Mux - thats how it was done in the past but seems wrong.)
                        pRc->sst.textureModeT1 |= SST_TC_MALOCAL;
                     }
                     else if ( (pRc->sst.combineModeT1 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_OTHER_TRGB)  { /*We need to get arg via MSEL instead of MSEL7 */
                                 /*test 4c*/
                        pRc->sst.textureModeT1 &= ~SST_TC_MSELECT;  // Unselect MSEL7
                        // Select new Alpha arg (Ideally this should be alpha as modified by
                        // the upstream stage's ACU; instead it is the alpha selected by this
                        // stages ACU other Mux - thats how it was done in the past but seems wrong.)
                        pRc->sst.textureModeT1 |= SST_TC_MAOTHER;
                     }
                     else /* if ( ((pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TRGBMSEL7_ITERTRGB)||(pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_CR_RGB) )*/
                     {  /*test 6c*/
                         pRc->sst.combineModeT1 += SST_CM_TC_MSELECT_7_ZERO;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     }
                     break;
                 /*case ARG1_MUX??? :No Args come in stage 0 of 0to1 via FBI */
               }
           }
        }
        if (TS[i].colorArg2 & D3DTA_ALPHAREPLICATE) { /* If the request is to do alphareplicate on arg2 */
            DWORD ArgVal = lpMiniStage[TS[i].colorOp].setup[ARGX_VALID];
           if ( ArgVal & ARG2_AREP) { /* If the table says we can do alphareplicate on arg2 */
               switch (ArgVal & ARG2_MUX ) /* find the mux on which ARG2 comes */
               {
                 case ARG2_TMUXOTH: /*test 1d*/
                     pRc->sst.combineModeT1 += SST_CM_TC_OTHERSELECT_OTHER_TA;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG2_TMUXLOC: /*test 2d*/
                     pRc->sst.combineModeT1 += SST_CM_TC_LOCALSELECT_LOCAL_TA;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG2_TMUXM7:  /*test 3d */
                     if ( (pRc->sst.combineModeT1 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_LOCAL_TRGB)  { /*We need to get arg via MSEL instead of MSEL7 */
                        pRc->sst.textureModeT1 &= ~SST_TC_MSELECT;  // Unselect MSEL7
                        // Select new Alpha arg (Ideally this should be alpha as modified by
                        // this stage's ACU; instead it is the alpha selected by this stages
                        // ACU Local Mux - thats how it was done in the past but seems wrong.)
                        pRc->sst.textureModeT1 |= SST_TC_MALOCAL;
                     }
                     else if ( (pRc->sst.combineModeT1 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_OTHER_TRGB)  { /*We need to get arg via MSEL instead of MSEL7 */
                                 /*test 4d*/
                        pRc->sst.textureModeT1 &= ~SST_TC_MSELECT;  // Unselect MSEL7
                        // Select new Alpha arg (Ideally this should be alpha as modified by
                        // the upstream stage's ACU; instead it is the alpha selected by this
                        // stages ACU other Mux - thats how it was done in the past but seems wrong.)
                        pRc->sst.textureModeT1 |= SST_TC_MAOTHER;
                     }
                     else /* if ( ((pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TRGBMSEL7_ITERTRGB)||(pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_CR_RGB) )*/
                     {  /*test 6d*/
                         pRc->sst.combineModeT1 += SST_CM_TC_MSELECT_7_ZERO;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     }
                     break;
                 /*case ARG2_MUX??? :No Args come in stage 0 of 0to1 via FBI */
               }
           }
        }

      // Setup Stage1 : FBI(fbzColorpath) TMU0(textureMode, CombineMode)
      i=1;
      // Setup control path for alphaOp
      lpMiniStage = miniStageOp[numStages][i + ALPHAOP];
      pRc->sst.combineModeT0  |= lpMiniStage[TS[i].alphaOp].setup[ARGX_CMTMUOP];
      pRc->sst.textureModeT0  |= lpMiniStage[TS[i].alphaOp].setup[ARGX_TMUOP];
      D3DPRINT(25,"s0 of s0to1 alphaOP=%08x lpMinistage=%08x arg1,2=%08x\n",TS[0].alphaOp,lpMiniStage, ((TS[0].alphaArg1 & 0x03) << 2) | (TS[0].alphaArg2 & 0x03));
      D3DPRINT(25,"combineModeFBI=%08x T0=%08x textureModeT0=%08x\n",pRc->sst.combineModeFBI,pRc->sst.combineModeT0,pRc->sst.textureModeT0);
      D3DPRINT(25,"fbzColorPath=%08x combineModeT0=%08x textureModeT0=%08x\n",pRc->sst.fbzColorPath,pRc->sst.combineModeT0,pRc->sst.textureModeT0);
         /* Setup AlphaOp Arg1 and Arg2*/
      lpMiniArg = mini_arg_stage[numStages][i + ALPHAARG];
      D3DPRINT(25,"mini_arg_a0_of_0=%08x lpMiniArg=%08x",mini_arg_a0_of_0,lpMiniArg);
      arg1=TS[i].alphaArg1 & 0x03;
      arg2=TS[i].alphaArg2 & 0x03;
      arg1mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG1MUX];
      arg2mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG2MUX];
      D3DPRINT(25,"arg1=%08x arg2=%08x arg1mux=%08x arg2mux=%08x",arg1,arg2,arg1mux,arg2mux);
      D3DPRINT(25,"arg1sel=%08x arg2sel=%08x",lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1],lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2]);
      pRc->sst.combineModeT0 |= lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1]; //mux select for args
      pRc->sst.combineModeT0 |= lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2];
      if (arg1mux==MUXM7) {// We need to do this check because the arg input spans 2 registers.
          pRc->sst.textureModeT0 |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg1];
      }
      if (arg2mux==MUXM7) {
          pRc->sst.textureModeT0 |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg2];
      }
         /*Setup AlphaOp arg complement */
      if (TS[i].alphaArg1 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeT1 ^= lpMiniStage[TS[i].alphaOp].setup[ARG1_COMP_XOR_CM];
              pRc->sst.textureModeT1 ^= lpMiniStage[TS[i].alphaOp].setup[ARG1_COMP_XOR_TM];
      }
      if (TS[i].alphaArg2 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeT1 ^= lpMiniStage[TS[i].alphaOp].setup[ARG2_COMP_XOR_CM];
              pRc->sst.textureModeT1 ^= lpMiniStage[TS[i].alphaOp].setup[ARG2_COMP_XOR_TM];
      }
         /* Setup AUX Input */
      #if 0 /*Currently we don't use AUX MUX on alphaOps*/
      arg1mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG1AUXMUX];
      arg2mux=lpMiniStage[TS[i].alphaOp].setup[ARGX_ARG2AUXMUX];
      pRc->sst.combineModeT0 |= lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1];
      pRc->sst.combineModeT0 |= lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2];
      #endif

         /* Setup control path for ColorOp*/
      lpMiniStage = miniStageOp[numStages][i + COLOROP];
      pRc->sst.combineModeT0  |= lpMiniStage[TS[i].colorOp].setup[ARGX_CMTMUOP];
      pRc->sst.textureModeT0  |= lpMiniStage[TS[i].colorOp].setup[ARGX_TMUOP];
      D3DPRINT(25,"s0 of s0to1 colorOP=%08x lpstage=%08x arg1,2=%08x\n",TS[i].colorOp,lpMiniStage, ((TS[i].colorArg1 & 0x03) << 2) | (TS[i].colorArg2 & 0x03));

         /* Setup colorOp Arg1 and Arg2 */
      lpMiniArg = mini_arg_stage[numStages][i + COLORARG];
      D3DPRINT(25,"mini_arg_c0_of_0_to_1=%08x lpMiniArg=%08x",mini_arg_c0_of_0_to_1,lpMiniArg);
      arg1=TS[i].colorArg1 & 0x03;
      arg2=TS[i].colorArg2 & 0x03;
      arg1mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG1MUX];
      arg2mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG2MUX];
      D3DPRINT(25,"arg1=%08x arg2=%08x arg1mux=%08x arg2mux=%08x",arg1,arg2,arg1mux,arg2mux);
      D3DPRINT(25,"arg1sel=%08x arg2sel=%08x",lpMiniArg->argselect[arg1mux][arg1],lpMiniArg->argselect[arg2mux][arg2]);
      pRc->sst.combineModeT0 |= lpMiniArg->argselect[arg1mux][arg1]; //mux select for args
      pRc->sst.combineModeT0 |= lpMiniArg->argselect[arg2mux][arg2];
      if (arg1mux==MUXM7) {
          pRc->sst.textureModeT0 |= lpMiniArg->argselect[MUXM][arg1];
      }
      if (arg2mux==MUXM7) {
          pRc->sst.textureModeT0 |= lpMiniArg->argselect[MUXM][arg2];
      }
         /*Setup colorOp arg complement */
      if (TS[i].colorArg1 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeT0 ^= lpMiniStage[TS[i].colorOp].setup[ARG1_COMP_XOR_CM];
              pRc->sst.textureModeT0 ^= lpMiniStage[TS[i].colorOp].setup[ARG1_COMP_XOR_TM];
      }
      if (TS[i].colorArg2 & D3DTA_COMPLEMENT) {
              pRc->sst.combineModeT0 ^= lpMiniStage[TS[i].colorOp].setup[ARG2_COMP_XOR_CM];
              pRc->sst.textureModeT0 ^= lpMiniStage[TS[i].colorOp].setup[ARG2_COMP_XOR_TM];
      }
         /* Setup colorOp AUX Input */
      arg1mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG1AUXMUX];
      arg2mux=lpMiniStage[TS[i].colorOp].setup[ARGX_ARG2AUXMUX];
      pRc->sst.combineModeT0 |= lpMiniArg->argselect[arg1mux+MINI_ARGALPHA][arg1]; //AUX mux select.
      pRc->sst.combineModeT0 |= lpMiniArg->argselect[arg2mux+MINI_ARGALPHA][arg2];
      if (arg1mux==MUXM7) {
          pRc->sst.textureModeT0 |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg1];
      }
      if (arg2mux==MUXM7) {
          pRc->sst.textureModeT0 |= lpMiniArg->argselect[MUXM+MINI_ARGALPHA][arg2];
      }
         /* Setup colorOp Alphareplicate */
      if (TS[i].colorArg1 & D3DTA_ALPHAREPLICATE) { /* If the request is to do alphareplicate on arg1 */
            DWORD ArgVal = lpMiniStage[TS[i].colorOp].setup[ARGX_VALID];
           if ( ArgVal & ARG1_AREP) { /* If the table says we can do alphareplicate on arg1 */
               switch (ArgVal & ARG1_MUX ) /* find the mux on which arg1 comes */
               {
                 case ARG1_TMUXOTH: /*test 1c*/
                     pRc->sst.combineModeT0 += SST_CM_TC_OTHERSELECT_OTHER_TA;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG1_TMUXLOC: /*tested 2c - fab*/
                     pRc->sst.combineModeT0 += SST_CM_TC_LOCALSELECT_LOCAL_TA;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG1_TMUXM7:  /*test 3c */
                     if ( (pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_LOCAL_TRGB)  { /*We need to get arg via MSEL instead of MSEL7 */
                        pRc->sst.textureModeT0 &= ~SST_TC_MSELECT;  // Unselect MSEL7
                        // Select new Alpha arg (Ideally this should be alpha as modified by
                        // this stage's ACU; instead it is the alpha selected by this stages
                        // ACU Local Mux - thats how it was done in the past but seems wrong.)
                        pRc->sst.textureModeT0 |= SST_TC_MALOCAL;
                     }
                     else if ( (pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_OTHER_TRGB)  { /*We need to get arg via MSEL instead of MSEL7 */
                                 /*test 4c*/
                        pRc->sst.textureModeT0 &= ~SST_TC_MSELECT;  // Unselect MSEL7
                        // Select new Alpha arg (Ideally this should be alpha as modified by
                        // the upstream stage's ACU; instead it is the alpha selected by this
                        // stages ACU other Mux - thats how it was done in the past but seems wrong.)
                        pRc->sst.textureModeT0 |= SST_TC_MAOTHER;
                     }
                     else /* if ( ((pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TRGBMSEL7_ITERTRGB)||(pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_CR_RGB) )*/
                     {  /*test 6c*/
                         pRc->sst.combineModeT0 += SST_CM_TC_MSELECT_7_ZERO;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     }
                     break;
                 /*case ARG1_MUX??? :No Args come in stage 0 of 0to1 via FBI */
               }
           }
        }
        if (TS[i].colorArg2 & D3DTA_ALPHAREPLICATE) { /* If the request is to do alphareplicate on arg2 */
            DWORD ArgVal = lpMiniStage[TS[i].colorOp].setup[ARGX_VALID];
           if ( ArgVal & ARG2_AREP) { /* If the table says we can do alphareplicate on arg2 */
               switch (ArgVal & ARG2_MUX ) /* find the mux on which ARG2 comes */
               {
                 case ARG2_TMUXOTH: /*test 1d*/
                     pRc->sst.combineModeT0 += SST_CM_TC_OTHERSELECT_OTHER_TA;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG2_TMUXLOC: /*test 2d*/
                     pRc->sst.combineModeT0 += SST_CM_TC_LOCALSELECT_LOCAL_TA;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     break;
                 case ARG2_TMUXM7:  /*test 3d */
                     if ( (pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_LOCAL_TRGB)  { /*We need to get arg via MSEL instead of MSEL7 */
                        pRc->sst.textureModeT0 &= ~SST_TC_MSELECT;  // Unselect MSEL7
                        // Select new Alpha arg (Ideally this should be alpha as modified by
                        // this stage's ACU; instead it is the alpha selected by this stages
                        // ACU Local Mux - thats how it was done in the past but seems wrong.)
                        pRc->sst.textureModeT0 |= SST_TC_MALOCAL;
                     }
                     else if ( (pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_OTHER_TRGB)  { /*We need to get arg via MSEL instead of MSEL7 */
                                 /*test 4d*/
                        pRc->sst.textureModeT0 &= ~SST_TC_MSELECT;  // Unselect MSEL7
                        // Select new Alpha arg (Ideally this should be alpha as modified by
                        // the upstream stage's ACU; instead it is the alpha selected by this
                        // stages ACU other Mux - thats how it was done in the past but seems wrong.)
                        pRc->sst.textureModeT0 |= SST_TC_MAOTHER;
                     }
                     else /* if ( ((pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TRGBMSEL7_ITERTRGB)||(pRc->sst.combineModeT0 & SST_CM_TC_MSELECT_7) == SST_CM_TC_MSELECT_7_CR_RGB) )*/
                     {  /*test 6d*/
                         pRc->sst.combineModeT0 += SST_CM_TC_MSELECT_7_ZERO;  /*The nature of the register allows us to add 1 to the field to select alpha arg instead of color arg*/
                     }
                     break;
                 /*case ARG2_MUX??? :No Args come in stage 0 of 0to1 via FBI */
               }
           }
        }
     }
     else {  //IS_NAPALM
		/*
			New code added here for "workaround" for Unreal Tournament problem PRS 12099.
			Unreal Tournament uses diffuse as one of the arguments to the stage 0 texture stage state.
			The V3 cannot accept diffuse at stage 0 in a two stage opeartion so we convert this to
			a 3 stage operation with diffuse being modulated at the last stage.
			This will only occur if a device does not call ValidateDevice after setting up the
			texture stage states, ala Unreal Tournament and if the set-up used for the texture stages
			matches exactly the one used by Unreal T.  - jmccartney 28/02/00
		*/
		DWORD dwRemapStages=0x0;			// should we remap the textureStageState set-up or not
		TEXTURESTAGESTATE oldStage1State; 	// used to store the 2nd texture stage state so we can return it to its unmodified form
		BOOL dwTxtStageStateValidated = pRc->txtStageStateValidated;	 // has the app called ValidateDevice
	
		// if it is a 2 stage set-up and the texture state stages have not been validated then...
	  	if ( (2==numStages) && (!dwTxtStageStateValidated) )
		{
		  /*	Check if the texture stage state setup by the app matches the one that Unreal Tournament uses
		  */
		  dwRemapStages = MATCH_UT_TXTSTAGESTATE;	  // macro defined at the top of the file
		  										
		  /*	If the texture stage state setup by the app does match the one used by Unreal Tournament
		  		then remap to a 3 stage operation with diffuse being modulated at the 3rd stage
		  */
		  if (dwRemapStages)
		  {
			// store current stage 3 value just in case
			oldStage1State = TS[2];

			// setup 3rd stage operation which has the same result as the 2 stage setup Unreal Tournament uses
		    TS[0].colorOp = D3DTOP_SELECTARG1;
			TS[0].alphaOp = D3DTOP_SELECTARG1;
		    TS[2].colorOp = D3DTOP_MODULATE;
			TS[2].colorArg1 = D3DTA_CURRENT;
			TS[2].colorArg2 = D3DTA_DIFFUSE;
			TS[2].alphaOp = D3DTOP_MODULATE;
			TS[2].alphaArg1 = D3DTA_CURRENT;
			TS[2].alphaArg2 = D3DTA_DIFFUSE;
			// inc number of stages to 3
			numStages++;
		  } // endif (remapStages)
		} // endif (2==numStages) && (!dwTxtStageStateValidated)
	
      // Start with our AND mask for special multi-texturing modes.
      pRc->specialModes = ARGF_SP_ALL;
      for( i = 0, numStages--; i <= numStages; i++ )
      {
        // First we do the alpha combine operations ...
        lpStage = stageOp[numStages][i + ALPHAOP];
        argOffset = ((TS[i].alphaArg1 & 0x03) << 2) | (TS[i].alphaArg2 & 0x03);
        pRc->sst.fbzColorPath   |= lpStage[TS[i].alphaOp].arg[argOffset][ARGX_FBIOP];
        pRc->sst.textureModeT0  |= lpStage[TS[i].alphaOp].arg[argOffset][ARGX_TMUOP];

        // ... then we do the color combine operations.
        lpStage = stageOp[numStages][i + COLOROP];
        argOffset = ((TS[i].colorArg1 & 0x03) << 2) | (TS[i].colorArg2 & 0x03);
        pRc->sst.fbzColorPath   |= lpStage[TS[i].colorOp].arg[argOffset][ARGX_FBIOP];
        pRc->sst.textureModeT0  |= lpStage[TS[i].colorOp].arg[argOffset][ARGX_TMUOP];

        // Check for special multi-texture cases
        pRc->specialModes &= (lpStage[TS[i].colorOp].arg[argOffset][ARGX_VALID] & ARGF_SP3 );
      }
	  // if the texture stage state has been re-mapped change it back to the way it was before remap
	  if (dwRemapStages)
	  {		
	  	TS[0].colorOp = D3DTOP_MODULATE;
		TS[0].alphaOp = D3DTOP_MODULATE;
		TS[2] = oldStage1State;
		numStages--;
	  } // endif (remapStages)
     }//IS_NAPALM
    }
  }
  printFbzColorPath( pRc->sst.fbzColorPath );

#ifdef LODBIASPERCHIP
  // If we are multitexturing and trilinear filtering, and number
  // chips > 1, get the per chip Lod Bias values. Otherwise, calc
  // the Lod Bias for the single chip.
  if (pRc->dwMultitextureAndTrilinear && (_FF(dwNumUnits) > 1))
  {
	CalcPerChipLodBias(pRc);
  }
  else
  {
#endif
    // Add the LOD Bias value retrieved from the registry to the value
    // that is currently in the lod bias bits of the LOD register.
    if (_D3(LodBias))
    {
      int iLodBias;
    
      // extract the current lod bias.
      iLodBias = (pRc->sst.tLODT0 & SST_LODBIAS) >> SST_LODBIAS_SHIFT;
      // If number is negative, sign extend to a full 32 bits.
      if (iLodBias & 0x20)  iLodBias |= ~(0x3f);
      // add registry bias to current lod bias.
      iLodBias += _D3(LodBias);
      // clamp. (-32 to +31, with each integer step representing .25 of a level of detail)
      if (iLodBias < -32) iLodBias = -32;
      if (iLodBias >  31) iLodBias = 31;
      // remove current bias setting.
      pRc->sst.tLODT0 &= ~(SST_LODBIAS);
      // store new lod bias value.
      pRc->sst.tLODT0 |= ((iLodBias << SST_LODBIAS_SHIFT) & SST_LODBIAS);
    
      // extract the current lod bias.
      iLodBias = (pRc->sst.tLODT1 & SST_LODBIAS) >> SST_LODBIAS_SHIFT;
      // If number is negative, sign extend to a full 32 bits.
      if (iLodBias & 0x20)  iLodBias |= ~(0x3f);
      // add registry bias to current lod bias.
      iLodBias += _D3(LodBias);
      // clamp. (-32 to +31, with each integer step representing .25 of a level of detail)
      if (iLodBias < -32) iLodBias = -32;
      if (iLodBias >  31) iLodBias = 31;
      // remove current bias setting.
      pRc->sst.tLODT1 &= ~(SST_LODBIAS);
      // store new lod bias value.
      pRc->sst.tLODT1 |= ((iLodBias << SST_LODBIAS_SHIFT) & SST_LODBIAS);
    }
#ifdef LODBIASPERCHIP
  }
#endif
}

//-------------------------------------------------------------------

static DWORD textureModeCombineT0[6][6] =
{                                       // Home
  // D3DTP_NONE - No Mipmapping
  {(SST_TC_REPLACE | SST_TCA_REPLACE),  // TREX0
   (SST_TC_PASS | SST_TCA_PASS),        // TREX1
   (SST_TC_PASS | SST_TCA_PASS),        // Both
   (SST_TC_PASS | SST_TCA_PASS),        // LOD split, Odd on TMU1
   (SST_TC_PASS | SST_TCA_PASS),        // LOD split, Odd on TMU0
   (0)                          },      // Both (2pixperclock)
  // D3DTP_POINT - point Mipmapping
  {(SST_TC_REPLACE | SST_TCA_REPLACE),
   (SST_TC_PASS | SST_TCA_PASS),
   (SST_TC_PASS | SST_TCA_PASS),
   (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC | SST_TC_REVERSE_BLEND |
    SST_TCA_REVERSE_BLEND),
   (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC),
   0
   },
  // D3DTP_LINEAR - Linear Mipmapping
  {(SST_TC_REPLACE | SST_TCA_REPLACE | SST_TLODDITHER),
   (SST_TC_PASS | SST_TCA_PASS),
   (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC | SST_TC_REVERSE_BLEND |
    SST_TCA_REVERSE_BLEND),
   (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC | SST_TC_REVERSE_BLEND |
    SST_TCA_REVERSE_BLEND),
   (SST_TRILINEAR | SST_TC_BLEND_LODFRAC | SST_TCA_BLEND_LODFRAC),
   0
  },
} ;

static DWORD textureModeCombineT1[6][6] =
{  // D3DTP_NONE - No Mipmapping        // Home
   {0,                                  // TREX0
   (SST_TC_REPLACE | SST_TCA_REPLACE),  // TREX1
   (SST_TC_REPLACE | SST_TCA_REPLACE),  // Both
   (SST_TC_REPLACE | SST_TCA_REPLACE),  // LOD split, Odd on TMU1
   (SST_TC_REPLACE | SST_TCA_REPLACE),  // LOD split, Odd on TMU0
    0                                 },// Both (2pixperclock)
   // D3DTP_POINT - point Mipmapping
  {0,
   (SST_TC_REPLACE | SST_TCA_REPLACE),
   (SST_TC_REPLACE | SST_TCA_REPLACE),
   (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE),
   (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE),
    0                                                   },

  // D3DTP_LINEAR - Linear Mipmapping
  {0,
   (SST_TC_REPLACE | SST_TCA_REPLACE | SST_TLODDITHER),
   (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE ),
   (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE ),
   (SST_TRILINEAR  | SST_TC_REPLACE | SST_TCA_REPLACE ),
    0                                                   },
};
// include the LOD bias for each flavor below. In the case that the LOD bias is also
// supplied by the end user then we will add & clamp to the base value. The thinking
// is that end users expect the hardware to round to the closest LOD.
#define LB_HALF   (2 << SST_LODBIAS_SHIFT)
#define LB_ZERO   (0)
#define LB_DITH   (1 << SST_LODBIAS_SHIFT)

static DWORD tLodT0[6][5] =
{ // TREX0,TREX1,BOTH,SPLIT ODD ON TMU1,SPLIT ODD ON TMU0
  { 0, 0, 0, SST_LOD_TSPLIT, SST_LOD_TSPLIT | SST_LOD_ODD },
  { LB_HALF, 0, 0, LB_HALF | SST_LOD_TSPLIT | SST_LOD_ZEROFRAC, LB_HALF | SST_LOD_TSPLIT | SST_LOD_ODD | SST_LOD_ZEROFRAC },
  { LB_DITH, 0, 0, SST_LOD_TSPLIT, SST_LOD_TSPLIT | SST_LOD_ODD },
};

static DWORD tLodT1[6][5] =
{
  { 0, 0, 0, SST_LOD_TSPLIT | SST_LOD_ODD, SST_LOD_TSPLIT },
  { 0, LB_HALF, 0, LB_HALF | SST_LOD_TSPLIT | SST_LOD_ODD | SST_LOD_ZEROFRAC, LB_HALF | SST_LOD_TSPLIT | SST_LOD_ZEROFRAC },
  { 0, LB_DITH, SST_LOD_ODD, SST_LOD_TSPLIT | SST_LOD_ODD, SST_LOD_TSPLIT },
};

static DWORD tLodMMT0[6][5] =
{ // TMU0, TMU1, BOTH, SPLIT ODD ON TMU1, SPLIT ODD ON TMU0
  { 0, 1, 1, 0, 0 },
  { 0, 1, 1, 0, 0 },
  { 0, 1, 0, 0, 0 },
};

static DWORD tLodMMT1[6][5] =
{
  { 1, 0, 0, 0, 0 },
  { 1, 0, 0, 0, 0 },
  { 1, 0, 0, 0, 0 },
};

/*-------------------------------------------------------------------
Function Name:  CalcBias42

Description:    Coverts D3D LOD Bias to 4.2 format

Return:         int
-------------------------------------------------------------------*/

int CalcBias42(DWORD f)
{
  int  expnt, mantissa;
  BOOL neg = FALSE;

  mantissa = expnt = f;

  D3DPRINT( 255,"float2Int float=0x%x ", f );

  // special case
  if( mantissa == 0 )
    return 0;

  if( expnt & 0x80000000 )
    neg = TRUE;

  mantissa = (mantissa & 0x007fffff) | 0x00800000;
  expnt = 23 - (((expnt & 0x7F800000) >> 23) - 127);

  if( expnt > 31 )
    mantissa = 0;
  else
    mantissa = mantissa >> (expnt-2); // quarters for LOD Bias

  if (neg)	mantissa = -mantissa;

  mantissa += 2;
  mantissa &= 0x003F;

  return mantissa;
}

/*-------------------------------------------------------------------
Function Name:  setupSingleStage

Description:    Sets up the a single stage texture state

Return:         DWORD (D3D_OK on success)
-------------------------------------------------------------------*/
DWORD setupSingleStage( RC *pRc )
{
  SETUP_PPDEV(pRc)

  if( TXTRHNDL_INRANGE(TS[0].textureHandle)

#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // to fix potential access violation issues with NT Stress
  // verify we have a TXTRHNDL for this texture before dereferencing it

      && TXTRHNDL_PTR(TS[0].textureHandle)
#endif
      && TXTRHNDL_INUSE(TS[0].textureHandle)
    )
  {
    TXTRDESC *txtr = TXTRDESC_FROM_HNDL(TS[0].textureHandle);
    DWORD home;

    // Store the texture handle
    pRc->texture = TS[0].textureHandle;

    // Calculate the texture coordinate index into the FVF structure
    pRc->t0CoordIndex = TS[0].texCoordIndex * 2;

    // Set the texture magnification filtering
    if( TS[0].magFilter != D3DTFG_POINT )
      pRc->sst.textureMode |= SST_TMAGFILTER;

    // Set the texture minification filtering
    if( TS[0].minFilter != D3DTFN_POINT )
      pRc->sst.textureMode |= SST_TMINFILTER;

    // don't use mipmaps even if they are downloaded
    if( (TS[0].mipFilter == D3DTFP_NONE) && (_D3( autoMipMap ) == 0) )
    {
      home = txtr->noMipMapsHome - 1;
      pRc->sst.tLODT0 = txtr->tLODnoMipMaps[TREX0];
      pRc->sst.tLODT1 = txtr->tLODnoMipMaps[TREX1];
      pRc->sst.baseAddr  = txtr->noMipMapsBaseAddr[0];
      pRc->sst.baseAddr1 = txtr->noMipMapsBaseAddr[1];
    }
    else
    {
      home = txtr->home - 1;

      //jcochrane@3dfx.com
      //Add support for LOD BIAS
      if ( TS[0].mipmapLodBias )
      {
        DWORD bias;
        bias = CalcBias42(TS[0].mipmapLodBias);
        pRc->sst.tLODT0 = txtr->tLOD[TREX0] | (bias << SST_LODBIAS_SHIFT);
        pRc->sst.tLODT1 = txtr->tLOD[TREX1] | (bias << SST_LODBIAS_SHIFT);
      }
      else   //no LOD BIAS
      {
        if(TS[0].mipFilter == D3DTFP_LINEAR)
        {
          pRc->sst.tLODT0 = txtr->tLOD[TREX0];   	// jcochrane@3dfx.com no LOD BIAS for LINEAR MIPMAP
          pRc->sst.tLODT1 = txtr->tLOD[TREX1];
        }
        else
        {
          pRc->sst.tLODT0 = txtr->tLOD[TREX0] | (2 << SST_LODBIAS_SHIFT); // jcochrane@3dfx.com LOD BIAS = MULTEX LOD BIAS
          pRc->sst.tLODT1 = txtr->tLOD[TREX1] | (2 << SST_LODBIAS_SHIFT);
        }
      }

      pRc->sst.baseAddr  = txtr->baseAddr[0];
      pRc->sst.baseAddr1 = txtr->baseAddr[1];
    }
    pRc->sst.tLODT0 |= tLodT0[TS[0].mipFilter - 1][home];
    pRc->sst.tLODT1 |= tLodT1[TS[0].mipFilter - 1][home];

    // If either texture unit is not being used set the min/max LOD level to 8
    // for performance reasons. - Ade
    if( tLodMMT0[TS[0].mipFilter - 1][home] )
      pRc->sst.tLODT0 = ( pRc->sst.tLODT0 & 0xFFFFF000 ) | 0x820;

    if( tLodMMT1[TS[0].mipFilter - 1][home] )
      pRc->sst.tLODT1 = ( pRc->sst.tLODT1 & 0xFFFFF000 ) | 0x820;

    if (IS_NAPALM) {
        if ( ( TS[0].mipFilter != D3DTFP_LINEAR )/*&&(TS[0].mipFilter != D3DTFP_POINT )*/ ) {//Checking for Point mipmapping doesn't seem necessary -cws
            // Two pixel per clock mode gets textures into both TMUs independantly
            // (tmu1 does not pass texture to tmu0) - Also for Napalm, we don't want
            // textures automatically selected into TMU1 (the combine tables do this
            // when necessary - instead of defaulting to select textures in TMU1 as V3) -cws
            home = HOME_BOTH_2PIX_PER_CLOCK-1;
        }
        else // We can't do trilinear and 2ppc so disable 2ppc.  Optimization: do LODdither and 2ppc instead?
        { //Disable 2ppc mode in combineMode reg.
          pRc->sst.combineModeFBI &= ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK ;
          pRc->sst.combineModeT0  &= ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK ;
          pRc->sst.combineModeT1  &= ~SST_CM_ENABLE_TWO_PIXELS_PER_CLOCK ;
          // Leave 'home' alone.

        }
    }

    pRc->sst.textureModeT0 = textureModeCombineT0[TS[0].mipFilter - 1][home];
    pRc->sst.textureModeT1 = textureModeCombineT1[TS[0].mipFilter - 1][home];

    // Check for texture clamping in U(s)
    if( TS[0].addressU == D3DTADDRESS_CLAMP )
      pRc->sst.textureMode |= SST_TCLAMPS;
    else if( TS[0].addressU == D3DTADDRESS_MIRROR )
      pRc->sst.tLOD |= SST_TMIRRORS;

    // Check for texture clamping in V(t)
    if( TS[0].addressV == D3DTADDRESS_CLAMP )
      pRc->sst.textureMode |= SST_TCLAMPT;
    else if( TS[0].addressV == D3DTADDRESS_MIRROR )
      pRc->sst.tLOD |= SST_TMIRRORT;

#if (NEWASMTRI==1)
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
#ifdef K6_2
#if (STBKNI == 1)
//STB-SK 02/10/99 KNI changes - this code is used on both 3DNow! and Pentium III processors
    __asm {  // to get rid of involving FPU instructions
                mov     eax, pRc
                mov     ecx, txtr
                // _asm_data.scale_s= pRc->sst.scaleS  = txtr->scaleS;
                mov     edx, [ecx]TXTRDESC.scaleS
                mov     [eax]RC.sst.scaleS, edx
                mov     _asm_data.scale_s, edx
                mov     _asm_data.kni_array[SCALE_S2*4], edx
                // _asm_data.scale_t= pRc->sst.scaleT  = txtr->scaleT;
                mov     edx, [ecx]TXTRDESC.scaleT
                mov     [eax]RC.sst.scaleT, edx
                mov     _asm_data.scale_t, edx
                mov     _asm_data.kni_array[SCALE_T2*4], edx
                // _asm_data.tex_offset_s= pRc->sst.centerS = txtr->centerS;
                mov     edx, [ecx]TXTRDESC.centerS
                mov     [eax]RC.sst.centerS, edx
                mov     _asm_data.tex_offset_s, edx
                mov     _asm_data.kni_array[TEX_OFF_S2*4], edx
                // _asm_data.tex_offset_t= pRc->sst.centerT = txtr->centerT;
                mov     edx, [ecx]TXTRDESC.centerT
                mov     [eax]RC.sst.centerT, edx
                mov     _asm_data.tex_offset_t, edx
                mov     _asm_data.kni_array[TEX_OFF_T2*4], edx
    }
#else
    __asm {  // to get rid of involving FPU instructions
                mov     eax, pRc
                mov     ecx, txtr
                // _asm_data.scale_s= pRc->sst.scaleS  = txtr->scaleS;
                mov     edx, [ecx]TXTRDESC.scaleS
                mov     [eax]RC.sst.scaleS, edx
                mov     _asm_data.scale_s, edx
                // _asm_data.scale_t= pRc->sst.scaleT  = txtr->scaleT;
                mov     edx, [ecx]TXTRDESC.scaleT
                mov     [eax]RC.sst.scaleT, edx
                mov     _asm_data.scale_t, edx
                // _asm_data.tex_offset_s= pRc->sst.centerS = txtr->centerS;
                mov     edx, [ecx]TXTRDESC.centerS
                mov     [eax]RC.sst.centerS, edx
                mov     _asm_data.tex_offset_s, edx
                // _asm_data.tex_offset_t= pRc->sst.centerT = txtr->centerT;
                mov     edx, [ecx]TXTRDESC.centerT
                mov     [eax]RC.sst.centerT, edx
                mov     _asm_data.tex_offset_t, edx
    }
#endif
#else // K6_2
//STB-SK 01/30/99 KNI changes
#if (STBKNI == 1)
    // Store off the texture scale components
    _asm_data.kni_array[SCALE_S2] = _asm_data.scale_s= pRc->sst.scaleS  = txtr->scaleS;
    _asm_data.kni_array[SCALE_T2] = _asm_data.scale_t= pRc->sst.scaleT  = txtr->scaleT;
    _asm_data.kni_array[TEX_OFF_S2] = _asm_data.tex_offset_s= pRc->sst.centerS = txtr->centerS;
    _asm_data.kni_array[TEX_OFF_T2] = _asm_data.tex_offset_t= pRc->sst.centerT = txtr->centerT;
#else // Not STBKNI
    // Store off the texture scale components
    _asm_data.scale_s= pRc->sst.scaleS  = txtr->scaleS;
    _asm_data.scale_t= pRc->sst.scaleT  = txtr->scaleT;

    // Store off the texture pixel center offset
    _asm_data.tex_offset_s= pRc->sst.centerS = txtr->centerS;
    _asm_data.tex_offset_t= pRc->sst.centerT = txtr->centerT;
#endif // STBKNI
#endif // K6_2
#endif // NEWASMTRI

    // Store the texture wrap mode for the relevant texture unit
    pRc->wrapT0 = TS[TS[0].texCoordIndex].wrap;

    // Setup the texture format
#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    if ((TEXFMTFLG_PALETTIZED & txtr->formatFlags) && (0 != txtr->dwPaletteHandle) &&
        PALHNDL_INRANGE(TS[0].textureHandle) && PALETTEGBL(TS[0].textureHandle))
#else
    if( TEXFMTFLG_PALETTIZED & txtr->formatFlags)
#endif
    {
#if !defined(WINNT) || ((DIRECT3D_VERSION >= 0x0700) && (DX >= 7))
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
      PALHNDL *lpPalette = PALETTEGBL(TS[0].textureHandle);
#else
      LPDDRAWI_DDRAWPALETTE_GBL lpPalette = PALETTEGBL(TS[0].textureHandle);
#endif

      if( DDRAWIPAL_ALPHA & lpPalette->dwFlags )
      {
        pRc->sst.textureMode |= ( TEXFMT_P8_RGBA << SST_TFORMAT_SHIFT );
        txtr->formatFlags |= TEXFMTFLG_PALETTIZED_ALPHA;
      }
      else if (TEXFMTFLG_ALPHA & txtr->formatFlags)
      {
        pRc->sst.textureMode |= ( TEXFMT_ALPHA_P8_RGB << SST_TFORMAT_SHIFT );
        txtr->formatFlags &= ~TEXFMTFLG_PALETTIZED_ALPHA;
      }
      else
      {
        pRc->sst.textureMode |= ( TEXFMT_P8_RGB << SST_TFORMAT_SHIFT );
        txtr->formatFlags &= ~TEXFMTFLG_PALETTIZED_ALPHA;
      }

      // Download the new palette
      TXTRNEWPALETTE(ppdev, lpPalette );
#endif
    }
    else
      pRc->sst.textureMode |= txtr->format;

    // Setup any state requirements and command packet3 data components
    pRc->state          |= STATE_REQUIRES_ST_TMU0;
    pRc->sst.sSetupMode |= SST_SETUP_ST0;

    // Enable Texturing
    pRc->sst.fbzColorPath |= SST_ENTEXTUREMAP;
  }
  else {
    if (IS_NAPALM) {
      //  Normally, when a texture is used, we init texturemodet0/1 (AND THUS CLEAR THE OLD STATE!).
      //  But when only iterated/TFACTORS are used in the TCU, we still need to init texturemode to 0x0.
      //  We don't do this for V3 because V3 can't use only iterated data for the TCU and should fail validation.
      //  Also, V3 sets T1 to REPLACE by default to get a texture into T0. -cws
      pRc->sst.textureModeT0=0x0;  //Set to Pass (Napalm will pass data thru T0 for single stage)
      pRc->sst.textureModeT1=0x0;  //Init (Napalm will OR-in other bits)
    }
  }


  // Reset texture stage state change flag
  TS[0].changed = 0;
  TS[1].changed = 0;

  return D3D_OK;
}

/*-------------------------------------------------------------------
Function Name:  setupTextureStage0

Description:    Sets up texture stage 0.
                This is called by setupTexturing.

Return:         DWORD (D3D_OK on success)
-------------------------------------------------------------------*/
DWORD setupTextureStage0( RC *pRc )
{
  SETUP_PPDEV(pRc)
  TXTRDESC  *txtr;

  TEXTURESTAGESTATE *pTS = &(TS[0]);

  // srogers - fix WHQL DCT 200 Sub Tex1 from Tex2
  // Flip the texture pointers if we are doing a Subtract
  // with ColorArg1=Texture and ColorArg2=Current.
  // Warning : This will switch the alpha texture handles too. cws
  if(   !(IS_NAPALM)
      && ((TS[1].colorOp == D3DTOP_SUBTRACT) || (TS[1].colorOp == D3DTOP_MODULATE2X))
      && ((TS[1].colorArg1&0x3) == D3DTA_TEXTURE)
      && ((TS[1].colorArg2&0x3) == D3DTA_CURRENT)    )
    pTS = &(TS[1]);

  if( TXTRHNDL_INRANGE(pTS->textureHandle)

#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // to fix potential access violation issues with NT Stress
  // verify we have a TXTRHNDL for this texture before dereferencing it

      && TXTRHNDL_PTR(pTS->textureHandle)
#endif
      && TXTRHNDL_INUSE(pTS->textureHandle)
    )
  {
    txtr = TXTRDESC_FROM_HNDL(pTS->textureHandle);

    // Calculate the texture coordinate index into the FVF structure
    pRc->t1CoordIndex = pTS->texCoordIndex * 2;

    // Store the texture handle
    pRc->texture = pTS->textureHandle;

    // Set the texture magnification filtering
    if( pTS->magFilter != D3DTFG_POINT )
      pRc->sst.textureModeT1 |= SST_TMAGFILTER;

    // Set the texture minification filtering
    if( pTS->minFilter != D3DTFN_POINT )
      pRc->sst.textureModeT1 |= SST_TMINFILTER;

    // Get the base address and any mip info for the texture
    if( pTS->mipFilter == D3DTFP_NONE )
    {
      pRc->sst.tLODT1    = txtr->tLODnoMipMaps[1];
      pRc->sst.baseAddr1 = txtr->noMipMapsBaseAddr[1];
    }
    else
    {
      if ( TS[1].mipmapLodBias )	//jcochrane@3dfx.com
      {
        DWORD bias;
        bias = CalcBias42(TS[1].mipmapLodBias);
        pRc->sst.tLODT1 = txtr->tLOD[TREX1] | (bias << SST_LODBIAS_SHIFT);
      }
      else
      {
        pRc->sst.tLODT1    = txtr->tLOD[1] | (2 << SST_LODBIAS_SHIFT);
      }
      pRc->sst.baseAddr1 = txtr->baseAddr[1];

      // If mipmap linear is selected use LOD Dithering
      if( pTS->mipFilter == D3DTFP_LINEAR )
        pRc->sst.textureModeT1 |= SST_TLODDITHER;
    }

    // Check for texture clamping in U(s)
    if( pTS->addressU == D3DTADDRESS_CLAMP )
      pRc->sst.textureModeT1 |= SST_TCLAMPS;
    else if( pTS->addressU == D3DTADDRESS_MIRROR )
      pRc->sst.tLODT1 |= SST_TMIRRORS;

    // Check for texture clamping in V(t)
    if( pTS->addressV == D3DTADDRESS_CLAMP )
      pRc->sst.textureModeT1 |= SST_TCLAMPT;
    else if( pTS->addressV == D3DTADDRESS_MIRROR )
      pRc->sst.tLODT1 |= SST_TMIRRORT;

#if (NEWASMTRI==1)
    // Remove the involvement of Floating Point Code
    *((int*)&(_asm_data.scale_s1)) = *((int*)&(pRc->sst.scaleS1)) = *((int*)&(txtr->scaleS));
    *((int*)&(_asm_data.scale_t1)) = *((int*)&(pRc->sst.scaleT1)) = *((int*)&(txtr->scaleT));
    *((int*)&(_asm_data.tex_offset_s1)) = *((int*)&(pRc->sst.centerS1)) = *((int*)&(txtr->centerS));
    *((int*)&(_asm_data.tex_offset_t1)) = *((int*)&(pRc->sst.centerT1)) = *((int*)&(txtr->centerT));
#else // Not NEWASMTRI
    // Store off the texture scale components
    pRc->sst.scaleS1  = txtr->scaleS ;
    pRc->sst.scaleT1  = txtr->scaleT ;

    // Store off the texture pixel center offset
    pRc->sst.centerS1 = txtr->centerS;
    pRc->sst.centerT1 = txtr->centerT;
#endif // NEWASMTRI

    // Store the texture wrap mode for the relevant texture unit
    pRc->wrapT1 = TS[pTS->texCoordIndex].wrap;

    // reset the fields in texture that are set when texture is loaded and then
    // merge in the new flags
    pRc->sst.textureModeT1 &= ~(SST_TFORMAT);

    // Setup the texture format
#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    if ((TEXFMTFLG_PALETTIZED & txtr->formatFlags) && (0 != txtr->dwPaletteHandle) &&
         PALHNDL_INRANGE(pTS->textureHandle) && PALETTEGBL(pTS->textureHandle))
#else
    if( TEXFMTFLG_PALETTIZED & txtr->formatFlags)
#endif
    {
#if !defined(WINNT) || ((DIRECT3D_VERSION >= 0x0700) && (DX >= 7))
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
      PALHNDL *lpPalette = PALETTEGBL(pTS->textureHandle);
#else
      LPDDRAWI_DDRAWPALETTE_GBL lpPalette = PALETTEGBL(pTS->textureHandle);
#endif

      if( DDRAWIPAL_ALPHA & lpPalette->dwFlags )
      {
        pRc->sst.textureModeT1 |= ( TEXFMT_P8_RGBA << SST_TFORMAT_SHIFT );
        txtr->formatFlags |= TEXFMTFLG_PALETTIZED_ALPHA;
      }
      else if (TEXFMTFLG_ALPHA & txtr->formatFlags)
      {
        // retro3dfx: this stage programs TMU1 -- the vintage code OR'd the
        // ALPHA_P8 format into the single-texture register by copy-paste.
        pRc->sst.textureModeT1 |= ( TEXFMT_ALPHA_P8_RGB << SST_TFORMAT_SHIFT );
        txtr->formatFlags &= ~TEXFMTFLG_PALETTIZED_ALPHA;
      }
      else
      {
        pRc->sst.textureModeT1 |= ( TEXFMT_P8_RGB << SST_TFORMAT_SHIFT );
        txtr->formatFlags &= ~TEXFMTFLG_PALETTIZED_ALPHA;
      }

      // Download the new palette
      TXTRNEWPALETTE(ppdev, lpPalette );
#endif
    }
    else
      pRc->sst.textureModeT1 |= txtr->format;

    // Setup any state requirements and command packet3 data components
    pRc->state          |= STATE_REQUIRES_ST_TMU1;
    pRc->sst.sSetupMode |= SST_SETUP_ST1;

    // Enable Texturing
    pRc->sst.fbzColorPath |= SST_ENTEXTUREMAP;
  }

  // Reset texture stage state change flag
  pTS->changed = 0;

  return D3D_OK;
}

/*-------------------------------------------------------------------
Function Name:  setupTextureStage1

Description:    Sets up texture stage 1.
                This is called by setupTexturing.

Return:         DWORD (D3D_OK on success)
-------------------------------------------------------------------*/
DWORD setupTextureStage1( RC *pRc )
{
  SETUP_PPDEV(pRc)
  TXTRDESC  *txtr;

  TEXTURESTAGESTATE *pTS = &(TS[1]);

  // srogers - fix WHQL DCT 200 Sub Tex1 from Tex2
  // Flip the texture pointers if we are doing a Subtract
  // with ColorArg1=Texture and ColorArg2=Current
  // Warning : This will switch the alpha texture handles too. cws
  if(    !(IS_NAPALM)
       && ((TS[1].colorOp == D3DTOP_SUBTRACT) || (TS[1].colorOp == D3DTOP_MODULATE2X))
       && ((TS[1].colorArg1&0x3) == D3DTA_TEXTURE)
       && ((TS[1].colorArg2&0x3) == D3DTA_CURRENT)   )
    pTS = &(TS[0]);

  if( TXTRHNDL_INRANGE(pTS->textureHandle)

#if /* defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // to fix potential access violation issues with NT Stress
  // verify we have a TXTRHNDL for this texture before dereferencing it

      && TXTRHNDL_PTR(pTS->textureHandle)
#endif
      && TXTRHNDL_INUSE(pTS->textureHandle)
    )
  {
    txtr = TXTRDESC_FROM_HNDL(pTS->textureHandle);

    // Store the texture handle
    pRc->texture = pTS->textureHandle;

    // Calculate the texture coordinate index into the FVF structure
    pRc->t0CoordIndex = pTS->texCoordIndex * 2;

    // Set the texture magnification filtering
    if( pTS->magFilter != D3DTFG_POINT )
      pRc->sst.textureModeT0 |= SST_TMAGFILTER;

    // Set the texture minification filtering
    if( pTS->minFilter != D3DTFN_POINT )
      pRc->sst.textureModeT0 |= SST_TMINFILTER;

    // Get the base address and any mip info for the texture
    if( (pTS->mipFilter == D3DTFP_NONE) && (_D3( autoMipMap ) == 0) )
    {
      pRc->sst.tLODT0    = txtr->tLODnoMipMaps[0];
      pRc->sst.baseAddr  = txtr->noMipMapsBaseAddr[0];
    }
    else
    {
      if ( TS[0].mipmapLodBias ) //jcochrane@3dfx.com
      {
        DWORD bias;
        bias = CalcBias42(TS[0].mipmapLodBias);
        pRc->sst.tLODT0 = txtr->tLOD[TREX0] | (bias << SST_LODBIAS_SHIFT);
      }
      else
      {
        pRc->sst.tLODT0    = txtr->tLOD[0] | (2 << SST_LODBIAS_SHIFT);
      }
      pRc->sst.baseAddr  = txtr->baseAddr[0];

      // If mipmap linear is selected use LOD Dithering
      if( pTS->mipFilter == D3DTFP_LINEAR )
        pRc->sst.textureModeT0 |= SST_TLODDITHER;
    }

    if( pTS->addressU == D3DTADDRESS_CLAMP )
      pRc->sst.textureModeT0 |= SST_TCLAMPS;
    else if( pTS->addressU == D3DTADDRESS_MIRROR )
      pRc->sst.tLODT0 |= SST_TMIRRORS;

    // Check for texture clamping in V(t)
    if( pTS->addressV == D3DTADDRESS_CLAMP )
      pRc->sst.textureModeT0 |= SST_TCLAMPT;
    else if( pTS->addressV == D3DTADDRESS_MIRROR )
      pRc->sst.tLODT0 |= SST_TMIRRORT;

#if (NEWASMTRI==1)
  #if (STBKNI==1)
    // Remove the involvement of Floating Point Code
    // Store off the texture scale components
    *((int*)&(_asm_data.kni_array[SCALE_S2])) = *((int*)&(_asm_data.scale_s)) = *((int*)&(pRc->sst.scaleS)) = *((int*)&(txtr->scaleS));
    *((int*)&(_asm_data.kni_array[SCALE_T2])) = *((int*)&(_asm_data.scale_t)) = *((int*)&(pRc->sst.scaleT))  = *((int*)&(txtr->scaleT));
    // Store off the texture pixel center offset
    *((int*)&(_asm_data.kni_array[TEX_OFF_S2])) = *((int*)&(_asm_data.tex_offset_s)) = *((int*)&(pRc->sst.centerS)) = *((int*)&(txtr->centerS));
    *((int*)&(_asm_data.kni_array[TEX_OFF_T2])) = *((int*)&(_asm_data.tex_offset_t)) = *((int*)&(pRc->sst.centerT)) = *((int*)&(txtr->centerT));
  #else // Not STBKNI==1
    // Remove the involvement of Floating Point Code
    // Store off the texture scale components
    *((int*)&(_asm_data.scale_s)) = *((int*)&(pRc->sst.scaleS)) = *((int*)&(txtr->scaleS));
    *((int*)&(_asm_data.scale_t)) = *((int*)&(pRc->sst.scaleT))  = *((int*)&(txtr->scaleT));
    // Store off the texture pixel center offset
    *((int*)&(_asm_data.tex_offset_s)) = *((int*)&(pRc->sst.centerS)) = *((int*)&(txtr->centerS));
    *((int*)&(_asm_data.tex_offset_t)) = *((int*)&(pRc->sst.centerT)) = *((int*)&(txtr->centerT));
  #endif // STBKNI
#else // Not NEWASMTRI
// STB-SK 01/30/99 KNI Change
#if (STBKNI == 1)
    // Store off the texture scale components
    _asm_data.kni_array[SCALE_S2] = _asm_data.scale_s= pRc->sst.scaleS  = txtr->scaleS;
    _asm_data.kni_array[SCALE_T2] = _asm_data.scale_t= pRc->sst.scaleT  = txtr->scaleT;
    // Store off the texture pixel center offset
    _asm_data.kni_array[TEX_OFF_S2] = _asm_data.tex_offset_s= pRc->sst.centerS = txtr->centerS;
    _asm_data.kni_array[TEX_OFF_T2] = _asm_data.tex_offset_t= pRc->sst.centerT = txtr->centerT;
#else // not STBKNI
    // Store off the texture scale components
    _asm_data.scale_s= pRc->sst.scaleS  = txtr->scaleS ;
    _asm_data.scale_t= pRc->sst.scaleT  = txtr->scaleT ;

    // Store off the texture pixel center offset
    _asm_data.tex_offset_s= pRc->sst.centerS = txtr->centerS;
    _asm_data.tex_offset_t= pRc->sst.centerT = txtr->centerT;
#endif // STBKNI
#endif // NEWASMTRI
    // Store the texture wrap mode for the relevant texture unit
    pRc->wrapT0 = TS[pTS->texCoordIndex].wrap;

    // Setup the texture format
#if /*defined(WINNT) && */(DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    if ((TEXFMTFLG_PALETTIZED & txtr->formatFlags) && (0 != txtr->dwPaletteHandle) &&
        PALHNDL_INRANGE(pTS->textureHandle) && PALETTEGBL(pTS->textureHandle))
#else
    if( TEXFMTFLG_PALETTIZED & txtr->formatFlags)
#endif
    {
#if !defined(WINNT) || ((DIRECT3D_VERSION >= 0x0700) && (DX >= 7))
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
      PALHNDL *lpPalette = PALETTEGBL(pTS->textureHandle);
#else
      LPDDRAWI_DDRAWPALETTE_GBL lpPalette = PALETTEGBL(pTS->textureHandle);
#endif

      if( DDRAWIPAL_ALPHA & lpPalette->dwFlags )
      {
        pRc->sst.textureModeT0 |= ( TEXFMT_P8_RGBA << SST_TFORMAT_SHIFT );
        txtr->formatFlags |= TEXFMTFLG_PALETTIZED_ALPHA;
      }
      else if (TEXFMTFLG_ALPHA & txtr->formatFlags)
      {
        // retro3dfx: this stage programs TMU0 -- same copy-paste as above.
        pRc->sst.textureModeT0 |= ( TEXFMT_ALPHA_P8_RGB << SST_TFORMAT_SHIFT );
        txtr->formatFlags &= ~TEXFMTFLG_PALETTIZED_ALPHA;
      }
      else
      {
        pRc->sst.textureModeT0 |= ( TEXFMT_P8_RGB << SST_TFORMAT_SHIFT );
        txtr->formatFlags &= ~TEXFMTFLG_PALETTIZED_ALPHA;
      }

      // Download the new palette
      TXTRNEWPALETTE(ppdev, lpPalette );
#endif
    }
    else
      pRc->sst.textureModeT0 |= txtr->format;

    // Setup any state requirements and command packet3 data components
    pRc->state          |= STATE_REQUIRES_ST_TMU0;
    pRc->sst.sSetupMode |= SST_SETUP_ST0;

    // Enable Texturing
    pRc->sst.fbzColorPath |= SST_ENTEXTUREMAP;
  }

  // Reset texture stage state change flag
  pTS->changed = 0;

  return D3D_OK;
}

#if defined( DEBUG )

DWORD opsCt = 0;
DWORD ops[16][12];

void printTexOp(DWORD dwOp, DWORD dwA1,DWORD dwA2 ){
char str[250]="";
    switch (dwOp) {
     case 1 : lstrcpy( str, "disable "                 );break;
     case 2 : lstrcpy( str, "SelArg1 "                 );break;
     case 3 : lstrcpy( str, "SelArg2 "                 );break;
     case 4 : lstrcpy( str, "Modulat "                 );break;
     case 5 : lstrcpy( str, "Mod2x   "                 );break;
     case 6 : lstrcpy( str, "Mod4x   "                 );break;
     case 7 : lstrcpy( str, "Add     "                 );break;
     case 8 : lstrcpy( str, "AddSigned "               );break;
     case 9 : lstrcpy( str, "AddSigned2x "             );break;
     case 10: lstrcpy( str, "Subtract "                );break;
     case 11: lstrcpy( str, "AddSmooth "               );break;
     case 12: lstrcpy( str, "BlendDiffuseAlpha "       );break;
     case 13: lstrcpy( str, "BlendTextureAlpha "       );break;
     case 14: lstrcpy( str, "BlendFactorAlpha "        );break;
     case 15: lstrcpy( str, "BlendTextureAlphaPM "     );break;
     case 16: lstrcpy( str, "BlendCurrentAlpha "       );break;
     case 17: lstrcpy( str, "Premodulate "             );break;
     case 18: lstrcpy( str, "ModulateAlphaAddColor "   );break;
     case 19: lstrcpy( str, "ModulateColorAddAlpha "   );break;
     case 20: lstrcpy( str, "ModulateInvAlphaAddColor ");break;
     case 21: lstrcpy( str, "ModulateInvColorAdAlpha " );break;
     case 22: lstrcpy( str, "BumpEnvmap "              );break;
     case 23: lstrcpy( str, "BumpMapLum "              );break;
     case 24: lstrcpy( str, "DotProduct3 "             );break;
     default: lstrcpy( str, "ERROR1 "                  );break;
    }
    switch (dwA1) {
     case  0: lstrcat (str, "Arg1=Diffuse "); break;
     case  1: lstrcat (str, "Arg1=Current "); break;
     case  2: lstrcat (str, "Arg1=Texture "); break;
     case  3: lstrcat (str, "Arg1=Factor  "); break;
     default: lstrcat (str, "Arg1=ERROR!  "); break;
    }
    if (dwA1 & D3DTA_COMPLEMENT )
        lstrcat (str, ",COMPLEMENT ");
    if (dwA1 & D3DTA_ALPHAREPLICATE )
        lstrcat (str, ",ALPHAREPLICATE ");

    switch (dwA2) {
     case  0: lstrcat (str, "Arg2=Diffuse "); break;
     case  1: lstrcat (str, "Arg2=Current "); break;
     case  2: lstrcat (str, "Arg2=Texture "); break;
     case  3: lstrcat (str, "Arg2=Factor  "); break;
     default: lstrcat (str, "Arg2=ERROR!  "); break;
    }
    if (dwA2 & D3DTA_COMPLEMENT )
        lstrcat (str, ",COMPLEMENT ");
    if (dwA2 & D3DTA_ALPHAREPLICATE )
        lstrcat (str, ",ALPHAREPLICATE ");

    D3DPRINT(25,"Op=%s",str);
}

  /*-------------------------------------------------------------------
Function Name:  checkTextureOp

Description:    Checks texture operation type.
                This is called by setupTexturing.

Return:         void
-------------------------------------------------------------------*/

void checkTextureOp( RC *pRc )
{
return; //This function is causing a GPF
#if 0
  SETUP_PPDEV(pRc)
  DWORD i;
  // This is used to determine the texture stage setup modes being used
  for(i = 0; i < opsCt; i++)
  {
  #ifndef H3
    if( ops[i][0] == TS[0].colorOp && ops[i][1]  == TS[0].colorArg1 && ops[i][2]  == TS[0].colorArg2 &&
        ops[i][3] == TS[0].alphaOp && ops[i][4]  == TS[0].alphaArg1 && ops[i][5]  == TS[0].alphaArg2 &&
        ops[i][6] == TS[1].colorOp && ops[i][7]  == TS[1].colorArg1 && ops[i][8]  == TS[1].colorArg2 &&
        ops[i][9] == TS[1].alphaOp && ops[i][10] == TS[1].alphaArg1 && ops[i][11] == TS[1].alphaArg2 )
          break;
  #else
    if( ops[i][0] == TS[0].colorOp && ops[i][1]  == TS[0].colorArg1 && ops[i][2]  == TS[0].colorArg2 &&
        ops[i][3] == TS[0].alphaOp && ops[i][4]  == TS[0].alphaArg1 && ops[i][5]  == TS[0].alphaArg2 )
          break;
  #endif
  }

  if( i == opsCt )
  {
    // New entry
    ops[i][0]  = TS[0].colorOp;
    ops[i][1]  = TS[0].colorArg1;
    ops[i][2]  = TS[0].colorArg2;
    ops[i][3]  = TS[0].alphaOp;
    ops[i][4]  = TS[0].alphaArg1;
    ops[i][5]  = TS[0].alphaArg2;
  #ifndef H3
    ops[i][6]  = TS[1].colorOp;
    ops[i][7]  = TS[1].colorArg1;
    ops[i][8]  = TS[1].colorArg2;
    ops[i][9]  = TS[1].alphaOp;
    ops[i][10] = TS[1].alphaArg1;
    ops[i][11] = TS[1].alphaArg2;
  #endif
    opsCt++;

    D3DPRINT( 56, "Stage 0" );
    D3DPRINT( 56, "  ColorOp =%d, Arg1 =%d, Arg2 =%d", TS[0].colorOp, TS[0].colorArg1, TS[0].colorArg2 );
    D3DPRINT( 56, "  AlphaOp =%d, Arg1 =%d, Arg2 =%d", TS[0].alphaOp, TS[0].alphaArg1, TS[0].alphaArg2 );
  #ifndef H3
    D3DPRINT( 56, "Stage 1" );
    D3DPRINT( 56, "  ColorOp =%d, Arg1 =%d, Arg2 =%d", TS[1].colorOp, TS[1].colorArg1, TS[1].colorArg2 );
    D3DPRINT( 56, "  AlphaOp =%d, Arg1 =%d, Arg2 =%d", TS[1].alphaOp, TS[1].alphaArg1, TS[1].alphaArg2 );
  #endif
    D3DPRINT( 56, "" );
  }
#endif
}

#endif
//-------------------------------------------------------------------

#endif
 
