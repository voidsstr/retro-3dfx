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
** $Log: 
**  6    3dfx      1.3.1.1     10/23/00 Johnny Trainor  Updated so we no longer use
**       surface local pointers.
**  5    3dfx      1.3.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  4    3dfx      1.3         01/18/00 Bob Seitsinger  Changes to correctly write
**       to selected chip components (fbi,tmu0,tmu1). Chip component selection
**       occurred correctly for command packet header  writes, but needed to also
**       be done for data packet writes.
**  3    3dfx      1.2         10/28/99 Christopher Wilcox Hardware definition
**       changes to merge divergent h3defs.h.
**  2    3dfx      1.1         09/20/99 Steve Rogers    Porting my fix for PRS 6247
**       from V3 OEM tree:  Reload texture pointers if context is switched.
** 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 8     7/29/99 3:43p Bseitsin
** Remove an #ifdef WINNT - not required, since NT won't call D3D DX5/DX6
** stuff.
** 
** 7     7/29/99 12:54a Bseitsin
** Fix a compile error. Move CMDFIFO_PROLOG to be after TXTRDESC *txtr
** declaration.
** 
** 6     7/28/99 11:58p Bseitsin
** Changes to enable DX7 for W9x.
** 
** 5     7/13/99 2:27p Cshaw
** Added runtime support for Napalm's multitexturing (NAPALM_CU).
** 
** 4     7/09/99 4:33p Bseitsin
** Backwards compatability changes.
** 
** 3     6/14/99 3:17p Bseitsin
** Triangle Iterator Column Band Control support.
** 
** 2     6/04/99 8:33a Cshaw
** Added combineMode regs to the list of regs updated upon state change.
** 
** 1     6/02/99 6:40a Michael
** Branch from H3
** 
** 62    5/30/99 5:36p Edwin
** Remove ifdef MM, multi-monitor support is always enabled.
** 
** 61    5/24/99 5:04p Bseitsin
** Removal of Antialiasing code.
** 
** 60    4/26/99 11:36a Stb_bseitsin
** 32bit rendering mods. New SETSURFACEPIXELDEPTH macro.
** 
** 59    4/23/99 2:06p Stb_bseitsin
** 32-bit rendering mods
** 
** 58    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
** 
** 57    1/20/99 1:49p Adrians
** Changes to DX6 hardware setup.
** 
** 56    1/18/99 2:07p Adrians
** Removed obsolete variables and associated code.
** 
** 55    1/12/99 8:03a Cshaw
** Added runtime switches to the performance analysis code (use SIce to
** modify).
** 
** 54    1/07/99 12:01p Cshaw
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
** 53    12/23/98 1:16p Martin
** Set auxBufferAddr when zWriteEnable is on.  If an app (MFCtex), enables
** writes to the zbuffer, but doesn't enable zbuffer compares, then we
** still need to set up the zbuffer address.
** 
** 52    12/02/98 8:24a Martin
** Turn off anti-aliasing by default and rename some variables for
** super-sampling AA.
** 
** 51    11/30/98 8:13p Adrians
** Removed some compiler warnings.
** 
** 50    11/22/98 8:56p Andrew
** Changes to support multi-monitor
** 
** 49    11/13/98 2:52p Miriam
** Support for DX6 triangle flavor & texture flavor tracing. Just compile
** with debug & fp=1 or tp=1.
** 
** 48    10/16/98 4:33p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
** 
** 47    10/06/98 11:58p Adrians
** Migrate the aaTestFlags from the DX6 driver to the DX5 driver.
** 
** 46    10/04/98 9:44p Adrians
** When flat shading set texturing units lodmin/max to 1x1.
** 
** 45    10/02/98 11:16a Adrians
** Set maximum DX5 renderstate to D3DRENDERSTATE_STIPPLEPATTERN31.  This
** fixes a problem with Zbias not functioning under DX5.
** 
** 44    9/24/98 4:22p Adrians
** Added initial Instrumentation support.
** 
** 43    9/21/98 3:10p Adrians
** Optimisation to Fog HW setup.
** Added ZBIAS state change.
** Fix for multiple context's with different fog color.
** Set lodMin & lodMax to 1x1 when flat shading.
** 
** 42    9/12/98 12:55a Adrians
** Clean up renderstate trace code and add new DX6 states.
** Add trace support for DX6 texture stage states.
** General DX6 code tidyup.
** 
** 41    8/28/98 4:39p Martin
** Copy execute-buffer super-sampling AA implementation into
** drawPrimitive.
** 
** 40    8/28/98 3:20p Martin
** Reload palette when texture handle changes. Old fix caused performance
** degradation. New fix should only download palette at texture handle
** change.
** 
** 39    8/25/98 5:37p Suninn
** typo in sethwstate.  fix part of bug 2201
** 
** 38    8/23/98 12:49a Adrians
** Added support for AA SuperSampling.
** 
** 37    7/31/98 10:33p Miriam
** D3D & Glide cooperation. Allow each API to set/reset state to indicate
** that  the HW state has been changed.
** 
** 36    7/29/98 3:30p Adrians
** DX6 Changes.
** 
** 35    7/24/98 1:37p Hohn
** 
** 34    7/10/98 4:50p Martin
** check if palettized texture and if palette has changed.
** If so then reload the palette.
** 
** 33    7/02/98 12:52p Adrians
** Fix for AA hang.
** 
** 32    7/01/98 3:49p Miriam
** Clip registers are now set for drawing surfaces.
** 
** 31    6/30/98 5:29p Miriam
** Performance optimizations.
** 
** 30    6/01/98 12:20p Adrians
** Added aa to DrawPrimitives.
** 
** 29    5/20/98 2:47p Suninn
** use GET_HW_ADDR to access hw addresses for banshee
** 
** 28    5/18/98 3:20p Adrians
** Add some missing semi colon's.
** 
** 27    5/14/98 6:03p Miriam
** Merge in H3 support.
** 
** 26    5/06/98 6:07p Adrians
** Changes for DX6 into DX5 driver.
** 
** 2     5/01/98 4:11p Adrians
** Compile options for dx5 and dx6.
** Removed redundent returns.
** 
** 1     4/29/98 6:31p Adrians
** Created
** 
** 25    4/08/98 1:23p Adrians
** Fix for Non-Perspective DrawPrimitives.
** 
** 24    3/28/98 2:12p Adrians
** Return DD_OK when clearing nothing.
** 
** 23    3/27/98 5:50p Adrians
** Broadcast palette changes to both TMU's.
** 
** 22    3/27/98 1:44p Adrians
** Code tidyup.
** Removed palette and texture clamp compile options.
** 
** 21    3/27/98 11:15a Adrians
** New DrawPrimitive code.
** 
** 20    2/22/98 5:31p Adrians
** In DEBUG displays warning if Lock active when rendering.
** 
** 19    2/21/98 10:17p Adrians
** Added FRONT_BUFFER_RENDER compile option.
** 
** 18    2/16/98 11:51p Adrians
** No longer clear Zbuffer if it does not exist.
 * 
 * 17    1/15/98 10:23a Adrians
 * Added ZBIAS support.
 * 
 * 16    11/23/97 3:56p Suninn
 * replay _d3Global with _D3 & D3G macros
 * 
 * 15    11/21/97 5:29p Adrians
 * Restore 3D registers after 2D has used them.
 * Added context change code for DrawPrimitives.
 * 
 * 14    11/13/97 7:22p Miriam
 * Fix for Tomb2, same as Voodoo. Process the renderstates before setting
 * hardware state for drawprimitive.
 * 
 * 13    11/09/97 2:49p Adrians
 * Single pk1's use an increment of 0.
 * Code added to find the triangle flavours used by apps.
 * Optimisation of triangle flavours.
 * Support for strips and fans in execute buffers.
 * Bug fix to wrapU and wrapV modes.
 * 
 * 
 * 12    10/27/97 3:10p Adrians
 * Suninn's fix to myClear32.  Now converts 24bit col to 16bit.
 * 
 * 11    10/27/97 2:25p Adrians
 * Fixed 8 bit texture download.
 * 8 bit palettes are now built as default (p8=1).
 * Incorporated software triangle setup. Added build option (default
 * ss=0).
 * Command fifo debug build option added (default fd=0).
 * Texture clamping build option (default tc=1).
 * 
 * 10    10/24/97 7:14p Miriam
 * Support for 2 tmu and trilinear. Will see 2x the memory on a 2 tmu
 * system. Trilinear is 1 pass on 2 tmu & lodDither for 1 tmu. 
 * Fixed command fifo problem when using save/restore.
 * 
 * 9     10/16/97 1:52p Suninn
 * add view port clear
 * 
 * 8     10/14/97 2:31p Suninn
 * prepare for h3
 * 
 * 7     10/14/97 11:22a Adrians
 * Added Miriam's 4M texture support.  Added build environment for H3.
 * 
 * 6     10/10/97 10:42a Adrians
 * Removed all references to DIRECTX5.
 * 
 * 5     10/02/97 8:38p Adrians
 * Include init code into build. Enable Write Combining.  Inline system
 * functions.  Change optimisations.  Some code tidy up.
 * 
 * 4     9/15/97 4:12p Adrians
 * #DEFINE USE_8BIT_PALETTE to enable palettised textures 
 * 
 * 3     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/
#include "precomp.h"
#include <d3dhal.h>
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h" 
#include "fifomgr.h"
#include "d3contxt.h"
#include "d3txtr.h"

#include "dxins.h"

extern DWORD __stdcall ddiDrawOnePrimitive(LPD3DHAL_DRAWONEPRIMITIVEDATA);
extern DWORD __stdcall ddiDrawOneIndexedPrimitive(LPD3DHAL_DRAWONEINDEXEDPRIMITIVEDATA);
extern DWORD __stdcall ddiDrawPrimitives(LPD3DHAL_DRAWPRIMITIVESDATA);
extern DWORD __stdcall ddiClear(LPD3DHAL_CLEARDATA);

extern void  __stdcall fpLineList(RC *, DWORD, LPD3DTLVERTEX); 
extern void  __stdcall fpILineList(RC *, DWORD, WORD *, LPD3DTLVERTEX); 
extern void  __stdcall fpLineStrip(RC *, DWORD, LPD3DTLVERTEX); 
extern void  __stdcall fpILineStrip(RC *, DWORD, WORD *, LPD3DTLVERTEX); 
extern void  __stdcall fpPointList(RC *, DWORD, LPD3DTLVERTEX); 
extern void  __stdcall fpIPointList(RC *, DWORD, WORD *, LPD3DTLVERTEX); 
extern void  __stdcall fpFillIDrawTriangle(RC *, LPD3DHAL_DRAWONEINDEXEDPRIMITIVEDATA);
extern void  __stdcall fpFillDrawTriangle(RC *, LPD3DHAL_DRAWONEPRIMITIVEDATA);

// Fan Functions
extern void __stdcall dpDrawFanAll(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawFanIZ(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawFanIZT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawFanITH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawFanIH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawFanI(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawFanIT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawFanIZTH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);

// Indexed Fan Functions
extern void __stdcall dpDrawIndexedFanAll(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawIndexedFanIZ(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawIndexedFanIZT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawIndexedFanITH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawIndexedFanIH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawIndexedFanI(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawIndexedFanIT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);
extern void __stdcall dpDrawIndexedFanIZTH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices);

// Strip Functions
extern void __stdcall dpDrawStripAll(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawStripIZ(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawStripIZT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawStripIZTH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );

// Indexed Strip Functions
extern void __stdcall dpDrawIndexedStripAll(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawIndexedStripIZ(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawIndexedStripIZT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawIndexedStripIZTH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );

// Triangle Functions
extern void __stdcall dpDrawTriangleAll(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawTriangleIZ(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawTriangleIZT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawTriangleIZTH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );

// Indexed Triangle Functions
extern void __stdcall dpDrawIndexedTriangleAll(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawIndexedTriangleIZ(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawIndexedTriangleIZT(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );
extern void __stdcall dpDrawIndexedTriangleIZTH(RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices );

void __stdcall setHWstate( RC *, DWORD, DWORD, DWORD );

#if defined( FLAVOR_PROFILE )
// Draw primitive NULL renderer.
void __stdcall dpDrawTriangleNULL( RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices )
{
  triangleFlavour( pRc, pRc->state, 4, count );
  _asm { _asm int 1 }
  return;
}
void __stdcall dpDrawIndexedTriangleNULL( RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices )
{
  triangleFlavour( pRc, pRc->state, 7, count );
  _asm { _asm int 1 }
  return;
}
void __stdcall dpDrawFanNULL( RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices )
{
  triangleFlavour( pRc, pRc->state, 6, count );
  _asm { _asm int 1 }
  return;
}
void __stdcall dpDrawIndexedFanNULL( RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices )
{
  triangleFlavour( pRc, pRc->state, 9, count );
  _asm { _asm int 1 }
  return;
}
void __stdcall dpDrawStripNULL( RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices )
{
  triangleFlavour( pRc, pRc->state, 5, count );
  _asm { _asm int 1 }
  return;
}
void __stdcall dpDrawIndexedStripNULL( RC *pRc, DWORD count, WORD *triIndex, LPD3DTLVERTEX vertices )
{
  triangleFlavour( pRc, pRc->state, 8, count );
  _asm { _asm int 1 }
  return;
}
#else
#define dpDrawTriangleNULL          dpDrawTriangleAll
#define dpDrawIndexedTriangleNULL   dpDrawIndexedTriangleAll
#define dpDrawFanNULL               dpDrawFanAll
#define dpDrawIndexedFanNULL        dpDrawIndexedFanAll
#define dpDrawStripNULL             dpDrawStripAll
#define dpDrawIndexedStripNULL      dpDrawIndexedStripAll
#endif

typedef VOID (__stdcall *DPFXN)(RC *, DWORD, WORD *, LPD3DTLVERTEX );

typedef enum {
DPIDX_NULL, DPIDX_ALL, DPIDX_IZ, DPIDX_IZT, DPIDX_IZS, DPIDX_ITH, DPIDX_IH,
DPIDX_I, DPIDX_IT, DPIDX_IZTH, DPIDX_AAALL, DPIDX_MAX
} DPFXNIDX;

DPFXN dpDrawTriangleTable[DPIDX_MAX] = {
  dpDrawTriangleNULL,
  dpDrawTriangleAll,
  dpDrawTriangleIZ,
  dpDrawTriangleIZT,
  dpDrawTriangleNULL,
  dpDrawTriangleNULL,
  dpDrawTriangleNULL,
  dpDrawTriangleNULL,
  dpDrawTriangleNULL,
  dpDrawTriangleIZTH,
  dpDrawTriangleNULL
};

DPFXN dpDrawFanTable[DPIDX_MAX] = {
  dpDrawFanNULL,
  dpDrawFanAll,
  dpDrawFanIZ,
  dpDrawFanIZT,
  dpDrawFanNULL,
  dpDrawFanITH,
  dpDrawFanIH,
  dpDrawFanI,
  dpDrawFanIT,
  dpDrawFanIZTH,
  dpDrawFanNULL
};

DPFXN dpDrawStripTable[DPIDX_MAX] = {
  dpDrawStripNULL,
  dpDrawStripAll,
  dpDrawStripIZ,
  dpDrawStripIZT,
  dpDrawStripNULL,
  dpDrawStripNULL,
  dpDrawStripNULL,
  dpDrawStripNULL,
  dpDrawStripNULL,
  dpDrawStripIZTH,
  dpDrawStripNULL
};

DPFXN dpDrawIndexedTriangleTable[DPIDX_MAX] = {
  dpDrawIndexedTriangleNULL,
  dpDrawIndexedTriangleAll,
  dpDrawIndexedTriangleIZ,
  dpDrawIndexedTriangleIZT,
  dpDrawIndexedTriangleNULL,
  dpDrawIndexedTriangleNULL,
  dpDrawIndexedTriangleNULL,
  dpDrawIndexedTriangleNULL,
  dpDrawIndexedTriangleNULL,
  dpDrawIndexedTriangleIZTH,
  dpDrawIndexedTriangleNULL
};

DPFXN dpDrawIndexedFanTable[DPIDX_MAX] = {
  dpDrawIndexedFanNULL,
  dpDrawIndexedFanAll,
  dpDrawIndexedFanIZ,
  dpDrawIndexedFanIZT,
  dpDrawIndexedFanNULL,
  dpDrawIndexedFanITH,
  dpDrawIndexedFanIH,
  dpDrawIndexedFanI,
  dpDrawIndexedFanIT,
  dpDrawIndexedFanIZTH,
  dpDrawIndexedFanNULL
};

DPFXN dpDrawIndexedStripTable[DPIDX_MAX] = {
  dpDrawIndexedStripNULL,
  dpDrawIndexedStripAll,
  dpDrawIndexedStripIZ,
  dpDrawIndexedStripIZT,
  dpDrawIndexedStripNULL,
  dpDrawIndexedStripNULL,
  dpDrawIndexedStripNULL,
  dpDrawIndexedStripNULL,
  dpDrawIndexedStripNULL,
  dpDrawIndexedStripIZTH,
  dpDrawIndexedStripNULL
};

//-----------------------------------------------------------------------
//
// Process DX 5 Draw Primitives that have non-indexed vertex data 
//
//-----------------------------------------------------------------------
DWORD __stdcall processOnePrimitive(LPD3DHAL_DRAWONEPRIMITIVEDATA lpdopd)
{
  SETUP_PPDEV(lpdopd->dwhContext)
  RC *pRc = CONTEXT_PTR(lpdopd->dwhContext);

#if defined( DEBUG )  
  if( _FX( openLockCount ) )
    D3DPRINT( 0, "WARNING - ProcessOnePrimitive during lock !!" );
#endif

#ifdef DEBUG
  if (!pRc)
  {
    lpdopd->ddrval = D3DHAL_CONTEXT_BAD;
    return (DDHAL_DRIVER_HANDLED);
  }
#endif

 setHWstate( pRc, lpdopd->dwhContext, lpdopd->PrimitiveType, lpdopd->dwNumVertices );
   
  switch (lpdopd->PrimitiveType)
  { 
    case D3DPT_TRIANGLELIST:
      if (pRc->fillMode != D3DFILL_SOLID)
        fpFillDrawTriangle(pRc, lpdopd);
      else
        dpDrawTriangleTable[pRc->drawIndex]( pRc, lpdopd->dwNumVertices, NULL, (LPD3DTLVERTEX)lpdopd->lpvVertices );
      break;
      
    case D3DPT_TRIANGLESTRIP:
      if (pRc->fillMode != D3DFILL_SOLID)
        fpFillDrawTriangle(pRc, lpdopd);
      else
        dpDrawStripTable[pRc->drawIndex]( pRc, lpdopd->dwNumVertices - 2, NULL, (LPD3DTLVERTEX)lpdopd->lpvVertices );
      break;
      
    case D3DPT_TRIANGLEFAN:
      if (pRc->fillMode != D3DFILL_SOLID)
        fpFillDrawTriangle(pRc, lpdopd);
      else
        dpDrawFanTable[pRc->drawIndex]( pRc, lpdopd->dwNumVertices - 2, NULL, (LPD3DTLVERTEX)lpdopd->lpvVertices );
      break;
      
    case D3DPT_POINTLIST:
      fpPointList(pRc, lpdopd->dwNumVertices, (LPD3DTLVERTEX)lpdopd->lpvVertices); 
      break;
      
    case D3DPT_LINELIST:
      fpLineList(pRc, lpdopd->dwNumVertices, (LPD3DTLVERTEX)lpdopd->lpvVertices); 
      break;
      
    case D3DPT_LINESTRIP:
      fpLineStrip(pRc, lpdopd->dwNumVertices-1, (LPD3DTLVERTEX)lpdopd->lpvVertices); 
      break;
      
    default:
      D3DPRINT( 255, "Unknown or unsupported primitive type requested of doDrawOnePrimitive32" );
      return DDHAL_DRIVER_NOTHANDLED;                        
    }
    
    lpdopd->ddrval = DD_OK;
    return DDHAL_DRIVER_HANDLED;
} // doDrawOnePrimitive32

//-------------------------------------------------------------------

DWORD __stdcall ddiDrawOnePrimitive(LPD3DHAL_DRAWONEPRIMITIVEDATA lpdopd)
{
  SETUP_PPDEV(lpdopd->dwhContext)
  RC *pRc = CONTEXT_PTR(lpdopd->dwhContext);
  
  D3D_ENTRY( "ddiDrawOnePrimitive" );
  INS_ENTRY( INSC_D3DDRAWONEPRIMITIVE );

#if defined( NULLDRIVER ) 
  if (!_D3(ondrtTri)) {
	lpdopd->ddrval = DD_OK;
	INS_EXIT( );
	D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif
  D3DPRINT( 255, "DrawPrimitive %08lx %08lx %08lx %08lx %08lx %08lx",
    lpdopd->dwhContext,lpdopd->dwFlags,lpdopd->PrimitiveType,lpdopd->VertexType,lpdopd->dwNumVertices,lpdopd->lpvVertices );

#if defined( DEBUG )  
  if( _FX( openLockCount ) )
    D3DPRINT( 0, "WARNING - DrawOnePrimitive during lock !!" );
#endif
  
#ifdef DEBUG
  if (!pRc)
  {
    lpdopd->ddrval = D3DHAL_CONTEXT_BAD;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif
  
  INS_EXIT( );
  D3D_EXIT( processOnePrimitive(lpdopd) );
}

//-------------------------------------------------------------------
//
// This is the execute buffer equivalent for draw primitive. The driver
// gets a hodgepodge of primitives & renderstates. 
//
//-------------------------------------------------------------------
DWORD __stdcall ddiDrawPrimitives(LPD3DHAL_DRAWPRIMITIVESDATA lpdpd)
{
  SETUP_PPDEV(lpdpd->dwhContext)
  RC                         *pRc = CONTEXT_PTR(lpdpd->dwhContext);
  LPBYTE                      lpData=(LPBYTE)lpdpd->lpvData;
  LPD3DHAL_DRAWPRIMCOUNTS     lpDrawPrimitiveCounts;    
  LPDWORD                     lpStateChange;
  DWORD                       j,StateType,StateValue;
  D3DHAL_DRAWONEPRIMITIVEDATA DrawOnePrimitiveData;
  
  D3D_ENTRY( "ddiDrawPrimitives" );
  INS_ENTRY( INSC_D3DDRAWPRIMITIVES );
  
#if defined( NULLDRIVER ) 
  if (!_D3(ondrtTri)) {
	lpdpd->ddrval = DD_OK;
	INS_EXIT( );
	D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif

#ifdef debug
  if (!pRc)
  {
    lpdpd->ddrval = D3DHAL_CONTEXT_BAD;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif
  
  DrawOnePrimitiveData.dwFlags=lpdpd->dwFlags;
  DrawOnePrimitiveData.dwhContext=lpdpd->dwhContext;
  do
  {
    lpDrawPrimitiveCounts=(LPD3DHAL_DRAWPRIMCOUNTS)(lpData);
    lpData += sizeof(D3DHAL_DRAWPRIMCOUNTS);
    lpStateChange=(LPDWORD)lpData;

    //-------------
    // Renderstates
    //-------------
    for (j=lpDrawPrimitiveCounts->wNumStateChanges;j>0;j--)
    {
      StateType=*lpStateChange;
      lpStateChange++;
      StateValue=*lpStateChange;
      lpStateChange++;
      
      printRenderState(StateType, StateValue);
      
      if ( StateType > D3DRENDERSTATE_STIPPLEPATTERN31 )
        dummy(pRc, StateValue);
      else 
        _renderFuncs[StateType](pRc, StateValue) ;
    }


    //-------------
    // Primitives
    //-------------
    lpData +=(lpDrawPrimitiveCounts->wNumStateChanges*sizeof(DWORD)*2)+31;
    lpData = (LPBYTE)((ULONG)lpData & (~31));
    D3DPRINT( 255, "DrawPrims: lpData=%08lx PrimitiveType=%d wNumVertices=%d",lpData,lpDrawPrimitiveCounts->wPrimitiveType,lpDrawPrimitiveCounts->wNumVertices );
    if (lpDrawPrimitiveCounts->wNumVertices)
    {
      DrawOnePrimitiveData.PrimitiveType=lpDrawPrimitiveCounts->wPrimitiveType;
      DrawOnePrimitiveData.lpvVertices=(LPVOID)lpData;
      DrawOnePrimitiveData.VertexType=lpDrawPrimitiveCounts->wVertexType;
      DrawOnePrimitiveData.dwNumVertices=lpDrawPrimitiveCounts->wNumVertices;
      processOnePrimitive((LPD3DHAL_DRAWONEPRIMITIVEDATA)&DrawOnePrimitiveData); 
      if ((lpdpd->ddrval=DrawOnePrimitiveData.ddrval) != DD_OK)
        return DDHAL_DRIVER_HANDLED;
      lpData+=(lpDrawPrimitiveCounts->wNumVertices*sizeof(D3DTLVERTEX));
    }
  }while(lpDrawPrimitiveCounts->wNumVertices);

  lpdpd->ddrval = DD_OK;
  INS_EXIT( );
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}

//-------------------------------------------------------------------
//
// Process DX 5 Draw Primitives that have indexed vertex data
//
//-------------------------------------------------------------------
DWORD __stdcall ddiDrawOneIndexedPrimitive(LPD3DHAL_DRAWONEINDEXEDPRIMITIVEDATA lpdoipd)
{
  SETUP_PPDEV(lpdoipd->dwhContext)
  RC *pRc = CONTEXT_PTR(lpdoipd->dwhContext);

  D3D_ENTRY( "ddiDrawOneIndexedPrimitive" );
  INS_ENTRY( INSC_D3DDRAWONEINDEXEDPRIMITIVE );

#if defined( NULLDRIVER )  
  if ((!_D3(ondrtTri)) && (!_D3(ondrtHWSU))) {
	lpdoipd->ddrval = DD_OK;
	INS_EXIT( );
	D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif
  D3DPRINT( 255, "DrawIndexedPrimitive %08lx %08lx %08lx %08lx %08lx %08lx %x",
    lpdoipd->dwhContext,lpdoipd->dwFlags,lpdoipd->PrimitiveType,lpdoipd->VertexType,lpdoipd->lpwIndices,lpdoipd->lpvVertices,lpdoipd->lpwIndices );

#if defined( DEBUG )  
  if( _FX( openLockCount ) )
    D3DPRINT( 0, "WARNING - DrawOneIndexedPrimitive during lock !!" );
#endif

 #ifdef DEBUG
  if (!pRc)
  {
    lpdoipd->ddrval = D3DHAL_CONTEXT_BAD;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
 #endif

  setHWstate( pRc, lpdoipd->dwhContext, lpdoipd->PrimitiveType + 3, lpdoipd->dwNumIndices );

#if defined(NULLDRIVER) 
  if (!_D3(ondrtTri)) { 
	lpdoipd->ddrval = DD_OK;
	INS_EXIT( );
	D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif

  switch (lpdoipd->PrimitiveType)
  {
    case D3DPT_TRIANGLELIST:
      if (pRc->fillMode != D3DFILL_SOLID)
        fpFillIDrawTriangle(pRc, lpdoipd);
      else
        dpDrawIndexedTriangleTable[pRc->drawIndex]( pRc, lpdoipd->dwNumIndices, (WORD*)lpdoipd->lpwIndices, (LPD3DTLVERTEX)lpdoipd->lpvVertices );
      break;
     
    case D3DPT_TRIANGLESTRIP:
      if (pRc->fillMode != D3DFILL_SOLID)
        fpFillIDrawTriangle(pRc, lpdoipd);
      else
        dpDrawIndexedStripTable[pRc->drawIndex]( pRc, lpdoipd->dwNumIndices-2, (WORD*)lpdoipd->lpwIndices, (LPD3DTLVERTEX)lpdoipd->lpvVertices );
      break;
      
    case D3DPT_TRIANGLEFAN:
      if (pRc->fillMode != D3DFILL_SOLID)
        fpFillIDrawTriangle(pRc, lpdoipd);
      else
        dpDrawIndexedFanTable[pRc->drawIndex]( pRc, lpdoipd->dwNumIndices - 2, (WORD*)lpdoipd->lpwIndices, (LPD3DTLVERTEX)lpdoipd->lpvVertices );
      break;

    case D3DPT_POINTLIST:
      fpIPointList(pRc, lpdoipd->dwNumIndices, lpdoipd->lpwIndices, (LPD3DTLVERTEX)lpdoipd->lpvVertices); 
      break;
       
    case D3DPT_LINELIST:
      fpILineList(pRc, lpdoipd->dwNumIndices, lpdoipd->lpwIndices, (LPD3DTLVERTEX)lpdoipd->lpvVertices); 
      break;
     
    case D3DPT_LINESTRIP:
      fpILineStrip(pRc, lpdoipd->dwNumIndices-1, lpdoipd->lpwIndices, (LPD3DTLVERTEX)lpdoipd->lpvVertices); 
      break;
           
    default:
      D3DPRINT( 255, "Unknown or unsupported primitive type requested of DrawIndexedPrimitive" );
      INS_EXIT( );
      D3D_EXIT( DDHAL_DRIVER_NOTHANDLED );
  }
  
  lpdoipd->ddrval = DD_OK;
  INS_EXIT( );
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}

void __stdcall setHWstate( RC *pRc, DWORD dwhContext, DWORD primitiveType, DWORD count )
{
   SETUP_PPDEV(pRc)

  if (_D3(last).colBufferAddr != GET_HW_ADDR(pRc->lpDDS))
  {
    CMDFIFO_PROLOG(cmdFifo);

    CMDFIFO_CHECKROOM( cmdFifo, 2 * (PH1_SIZE + 2) );

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, colBufferAddr, 0xf ) );
    if(IS_TILED(GET_HW_ADDR(pRc->lpDDS)))
    { 
      DWORD colBufferStride;
    
      colBufferStride = (_DS(ddTileStride) & 0x3FFFL) | 0x8000L;
      SETPD( cmdFifo, ghw0->colBufferAddr,   (GET_HW_ADDR(pRc->lpDDS) & 0x7FFFFFFFL));
      SETPD( cmdFifo, ghw0->colBufferStride, colBufferStride);
    }
    else
    {
      SETPD( cmdFifo, ghw0->colBufferAddr,   GET_HW_ADDR(pRc->lpDDS));
      SETPD( cmdFifo, ghw0->colBufferStride, pRc->lpDDS->lpGbl->lPitch);
    }

    // Reset the clipping registers
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, clipLeftRight, 0xf ) );
    SETPD( cmdFifo, ghw0->clipLeftRight, (DWORD)pRc->lpDDS->lpGbl->wWidth);
    SETPD( cmdFifo, ghw0->clipBottomTop, (DWORD)pRc->lpDDS->lpGbl->wHeight);

    _D3(last).colBufferAddr = GET_HW_ADDR(pRc->lpDDS);
    CMDFIFO_EPILOG( cmdFifo );  
  }

  if (pRc->zEnable || pRc->zWriteEnable)
  {
    if (_D3(last).auxBufferAddr != GET_HW_ADDR(pRc->lpDDSZ))
    {
      CMDFIFO_PROLOG(cmdFifo);

      CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 2 );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, auxBufferAddr, 0xf ) );
      if(IS_TILED(GET_HW_ADDR(pRc->lpDDSZ)))
      {
        DWORD auxBufferStride;
      
        auxBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;
        SETPD( cmdFifo, ghw->auxBufferAddr,   (GET_HW_ADDR(pRc->lpDDSZ) & 0x7fffffffL));
        SETPD( cmdFifo, ghw->auxBufferStride, auxBufferStride);
      }
      else
      {
        SETPD( cmdFifo, ghw->auxBufferAddr,   GET_HW_ADDR(pRc->lpDDSZ));
        SETPD( cmdFifo, ghw->auxBufferStride, pRc->lpDDSZ->lpGbl->lPitch);
      } 
      _D3(last).auxBufferAddr = GET_HW_ADDR(pRc->lpDDSZ);
      CMDFIFO_EPILOG( cmdFifo );  
    }
  }

  //------------------------------------------------------------
  // Update Context
  // Either we just changed contexts or something in the context
  // changed and we need to update the hardware state. Must update
  // context before process the Z visibility (calls triangle code)
  //------------------------------------------------------------
  // This optimization can not be enabled until the appropriate changes to
  // DDraw have been made.
  if( dwhContext != _D3( lastContext ) )
  {
    UPDATE_HW_STATE( SC_STATE );
    UPDATE_FOG_STATE( SC_FOGALL );

	// srogers 8/17/99 - Reload textures if context is switched
	// Fixes PRS 6247
    pRc->textureStage[0].changed = 1;
    pRc->textureStage[1].changed = 1;
  }
  
  if( HW_STATE_CHANGED )
  {
    CMDFIFO_PROLOG(cmdFifo);

    _D3(lastContext) = dwhContext;

    if (HW_STATE_CHANGED & SC_TRIANGLE_FLAVOUR)
      setDrawTriangle(pRc);

    triangleFlavour( pRc, pRc->state, primitiveType, count );
    
    if( pRc->state & (STATE_REQUIRES_SPECULAR | STATE_REQUIRES_WRAP) )
      pRc->drawIndex = DPIDX_ALL;
    else
    {          
      switch( pRc->state & ~( STATE_REQUIRES_IT_ALPHA | STATE_REQUIRES_VERTS_AREA ) )
      {    
        case ( STATE_REQUIRES_IT_RGB | STATE_REQUIRES_OOZ |
            STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_PERSPECTIVE ) :
          pRc->drawIndex = DPIDX_IZT;
          break;
          
        case ( STATE_REQUIRES_IT_RGB | STATE_REQUIRES_OOZ ) :
          pRc->drawIndex = DPIDX_IZ;
          break;
          
        case ( STATE_REQUIRES_IT_RGB | STATE_REQUIRES_W_TMU0
            | STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_W_FBI | STATE_REQUIRES_HWFOG | STATE_REQUIRES_PERSPECTIVE ) :
          pRc->drawIndex    = DPIDX_ITH;
          break;

        case ( STATE_REQUIRES_IT_RGB | STATE_REQUIRES_OOZ |
             STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_PERSPECTIVE |
             STATE_REQUIRES_W_FBI | STATE_REQUIRES_HWFOG ) :
          pRc->drawIndex    = DPIDX_IZTH;
          break;

        case ( STATE_REQUIRES_IT_RGB | STATE_REQUIRES_W_FBI | STATE_REQUIRES_HWFOG ) :
          pRc->drawIndex    = DPIDX_IH;
          break;

        case ( STATE_REQUIRES_IT_RGB ) :
          pRc->drawIndex    = DPIDX_I;
          break;

        case ( STATE_REQUIRES_IT_RGB | STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_PERSPECTIVE ) :
          pRc->drawIndex    = DPIDX_IT;
          break;
          
        default:
          pRc->drawIndex = DPIDX_NULL;
          break;
      }
    }
    
    pRc->sst.fbzMode &= ~SST_ENCHROMAKEY;
 
    //------------------------------------------------------------------------
    //
    // Set attributes one time for all triangles  
    // NOTE: we are setting attributes here instead of when the attribute is             
    //       set for several reasons: 
    //       1. Direct Draw may change states because DD calls are interleaved
    //       2. Processes have just changed and a process swap occured.
    //       3. May have just swapped back from a glide application.
    //       4. This should be optimized later!!!!!!!!
    //------------------------------------------------------------------------
  
    //----------------
    //
    // Texture mapping
    //
    //----------------
    //if you change from texture mapping to non-texture mapping then must reset texturemode

    if ( 0 != pRc->texture )
    {  
      TXTRDESC *txtr = TXTRDESC_FROM_HNDL(pRc->texture);
      // setup the texture mode, location of texture and tlod register for this texture

      CMDFIFO_CHECKROOM( cmdFifo, 2 * (PH4_SIZE + 3) );
      
      SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R3, textureMode, TMU2CHIP(TREX0)) );
      SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->textureMode, pRc->sst.textureMode | pRc->sst.textureModeT0);
      SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->tLOD, pRc->sst.tLODT0 );
      SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->texBaseAddr, pRc->sst.baseAddr );

      // if palettized texture and it changed then redownload it.
      // why download now instead of when texture id changes - this is the conservative
      // approach and the palette can change for numerous reasons so lets make sure
      // that it is download before each mesh (can optimize later)

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
      if (PALETTIZEDHANDLE(pRc->texture) && (PALETTECHANGED & _D3(flags)))
      {
        CMDFIFO_SAVE( cmdFifo );

        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 | CHIP_TMU1 ),
                            PALETTEGBL(pRc->texture),
                            txtr->formatFlags );

        _D3(currentPalette) = (void *) PALETTEGBL(pRc->texture);
        CMDFIFO_RELOAD( cmdFifo );
      }
#else
      if(PALETTIZEDHANDLE(pRc->texture) && 
          ((_D3(flags) & PALETTECHANGED) || 
          (_D3(prevContentStamp) != PALETTEGBL(pRc->texture)->dwContentsStamp)))
      {
        CMDFIFO_SAVE( cmdFifo );
        
        // save off current value of dwContentStamp of palette associated
        // with texture.  When a palette changes, we don't get notified of
        // this event, so we will compare the previous one with the current
        // one, and reload the palette if different.
        _D3(prevContentStamp) = PALETTEGBL(pRc->texture)->dwContentsStamp;
      
    #if( DX >= 6 )        
        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 ), ((LPDDRAWI_DDRAWSURFACE_LCL)
            (TXTRHNDL_PTR(pRc->texture)->surfLcl))->lpDDPalette->lpLcl->lpGbl->lpColorTable,
            txtr->formatFlags );
    #else
        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 ), ((LPDDRAWI_DDRAWSURFACE_LCL)
            (TXTRHNDL_PTR(pRc->texture)->surfLcl))->lpDDPalette->lpLcl->lpGbl->lpColorTable );
    #endif          
          
        _D3(currentPalette) = (void *) PALETTEGBL(pRc->texture);
        CMDFIFO_RELOAD( cmdFifo );
	  }
#endif

      //----------------
      //
      // Chroma Keying
      //
      //----------------
      // The chroma key value is stored in the texture surface. This is the only attribute
      // that is stored in the surface and not the rendering context. If the source chroma
      // key value is set for the texture surface then use the color. SST-1 chroma key
      // tests after the bi-linear filter and this is not correct. D3D performs the chroma
      // test before the filter. Scott/Gary said they would fix it one day...
      if( (pRc->colorKeyEnable) &&
          (((LPDDRAWI_DDRAWSURFACE_LCL)(TXTRHNDL_PTR(pRc->texture)->surfLcl))->dwFlags & DDRAWISURF_HASCKEYSRCBLT))
      { 
        int r, g, b, xrgb;
        
        r=g=b=((LPDDRAWI_DDRAWSURFACE_LCL)(TXTRHNDL_PTR(pRc->texture)->surfLcl))->ddckCKSrcBlt.dwColorSpaceLowValue ;

        // format of the chroma key value is the format of the texture
        // need to convert from texture format to xRGB-x888
        switch (txtr->format)
        {
          case (TEXFMT_ARGB_4444 << SST_TFORMAT_SHIFT): 
            r = ( r >> 8  ) & 0x0F;
            r = _imgMSBReplicate( r, 4, 0 );
            g = ( g >> 4  ) & 0x0F;
            g = _imgMSBReplicate( g, 4, 0 );
            b = ( b >> 0  ) & 0x0F;
            b = _imgMSBReplicate( b, 4, 0 );
            break;
          case (TEXFMT_ARGB_1555 << SST_TFORMAT_SHIFT):
            r = ( r >> 10 ) & 0x1F;
            r = _imgMSBReplicate( r, 3, 2);
            g = ( g >> 5  ) & 0x1F;
            g = _imgMSBReplicate( g, 3, 2);
            b =   b         & 0x1F;
            b = _imgMSBReplicate( b, 3, 2);
            break;
          default:
          case (TEXFMT_RGB_565 << SST_TFORMAT_SHIFT):
            r = ( r >> 11 ) & 0x1F;
            r = _imgMSBReplicate( r, 3, 2);
            g = ( g >> 5 ) &  0x3F;
            g = _imgMSBReplicate( g, 2, 4); 
            b = ( b >> 0 ) &  0x1F;
            b = _imgMSBReplicate( b, 3, 2 );
            break;
          case (TEXFMT_RGB_332 << SST_TFORMAT_SHIFT):
            r = ( r >> 5 ) & 0x07;
            r = (r  << 5 ) | (r << 2) | (r >> 1) ;   
            g = ( g >> 2 ) & 0x07;
            g = (g  << 5 ) | (g << 2) | (g >> 1) ;   
            b =   b        & 0x03;
            b = (b << 6) | (b << 4) | (b << 2) | b ;
            break;
          case (TEXFMT_P8_RGB << SST_TFORMAT_SHIFT):
          case (TEXFMT_P8_RGBA << SST_TFORMAT_SHIFT):
            {
              LPDDRAWI_DDRAWSURFACE_LCL lcl = (LPDDRAWI_DDRAWSURFACE_LCL)(TXTRHNDL_PTR(pRc->texture)->surfLcl);
              if (lcl->lpDDPalette != NULL)
              {
                g = lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable[r].peGreen;   
                b = lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable[r].peBlue;   
                r = lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable[r].peRed;   
              }
            }
        } 
        xrgb =  (r << 16) | (g << 8) | b;

        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chromaKey, 0xF ) );
        SETPD( cmdFifo, ghw->chromaKey, xrgb );

        _D3(last).chromaKey = xrgb;
        pRc->sst.fbzMode |= SST_ENCHROMAKEY;
      }
      else
      {
        pRc->sst.fbzMode &= ~SST_ENCHROMAKEY;
      }  
    }
    else
    {
      if( HW_STATE_CHANGED & SC_TLOD )
      {
        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, tLOD, 0xF ) );
        SETPD( cmdFifo, ghw->tLOD, (8 << SST_LODMIN_SHIFT) | (8 << SST_LODMAX_SHIFT) );
      }
    }

    // Deal with fog.
    if( pRc->useFog & FOG_STATE_CHANGED )
    {
      if( FOG_STATE_CHANGED & SC_FOGCOLOR )
      {
        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogColor, 0xF ) );
        SETPD( cmdFifo, ghw->fogColor, pRc->fogColor );
      }

      // Hardware table fog
      if( pRc->useFogTable & FOG_STATE_CHANGED )
      {
        CMDFIFO_SAVE( cmdFifo );
        createTableAndLoad(pRc);
        CMDFIFO_RELOAD( cmdFifo );
      }

      RESET_FOG_STATE;
    }

    //----------------------------------------------------------
    // set target surface pixel depth.
    // this macro modifies the renderMode and fbzMode registers.
    //----------------------------------------------------------
    if (IS_NAPALM)
    {
        SETSURFACEPIXELDEPTH(pRc, pRc->lpDDS);

        // Set the triangle iterator column band control.
        // This value is taken from a registry key, or is zero if no key exists.
        pRc->sst.fbzColorPath |= (( _D3(columnBandControl) << SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL_SHIFT ) 
                                  & SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL );

        CMDFIFO_CHECKROOM( cmdFifo, PH4_SIZE + 5 + (PH1_SIZE * 2) + 5 );//NAPALM_CU
    }
    else
    {
        CMDFIFO_CHECKROOM( cmdFifo, PH4_SIZE + 5 + PH1_SIZE + 1 );
    }

    //----------------------  
    //
    // Context switch restore 
    //
    //------------------------
    
    if ((HW_STATE_CHANGED) & SC_NEED_NOP)
    {
      SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3|R7, fbzColorPath, 0 ) );
      SETPD( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath );
      SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode );
      SETPD( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode );
      SETPD( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );
      SETPD( cmdFifo, ghw->nopCMD, 0 );
    }
    else
    {
      SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3, fbzColorPath, 0 ) );
      SETPD( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath );
      SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode );
      SETPD( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode );
      SETPD( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );
    }

    if (IS_NAPALM)
    {
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 1 ) ); //NAPALM_CU
        SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, pRc->sst.combineModeFBI );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 2 ) );
        SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU0)->combineMode, pRc->sst.combineModeT0 );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 4 ) );
        SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU1)->combineMode, pRc->sst.combineModeT1 ); //NAPALM_CU

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 1, renderMode, 0 ) );
        SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode );
    }

    // If Z buffer is enabled then we need to send the current
    // ZBIAS value. - AS
    if( HW_STATE_CHANGED & SC_ZBIAS )
    {
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, zaColor, 0 ) );
      SETPD( cmdFifo, ghw->zaColor, pRc->sst.zaColor );
    }

    _D3(last).fbzColorPath  = pRc->sst.fbzColorPath;
    _D3(last).fogMode       = pRc->sst.fogMode;
    _D3(last).alphaMode     = pRc->sst.alphaMode;
    _D3(last).fbzMode       = pRc->sst.fbzMode;
    _D3(last).zaColor       = pRc->sst.zaColor;

    if (IS_NAPALM)
    {
      _D3(last).combineModeFBI   = pRc->sst.combineModeFBI; //NAPALM_CU
      _D3(last).combineModeT0    = pRc->sst.combineModeT0 ;
      _D3(last).combineModeT1    = pRc->sst.combineModeT1 ;//NAPALM_CU
      _D3(last).renderMode    = pRc->sst.renderMode;
    }

    RESET_HW_STATE;

    CMDFIFO_EPILOG( cmdFifo );
  } // context update
  else // HW_STATE_CHANGED
  {
    if (pRc->texture != 0)
    {
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
      TXTRDESC *txtr = TXTRDESC_FROM_HNDL(pRc->texture);
      CMDFIFO_PROLOG( cmdFifo );

      if (PALETTIZEDHANDLE(pRc->texture) && (PALETTECHANGED & _D3(flags)))
      {
        CMDFIFO_SAVE( cmdFifo );

        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 ),
                            PALETTEGBL(pRc->texture),
                            txtr->formatFlags );

        _D3(currentPalette) = (void *) PALETTEGBL(pRc->texture);
        CMDFIFO_RELOAD( cmdFifo );
      }
#else
      CMDFIFO_PROLOG( cmdFifo );
      if(PALETTIZEDHANDLE(pRc->texture) && 
          ((_D3(flags) & PALETTECHANGED) || 
          (_D3(prevContentStamp) != PALETTEGBL(pRc->texture)->dwContentsStamp)))
      {
        CMDFIFO_SAVE( cmdFifo );
      
        _D3(prevContentStamp) = PALETTEGBL(pRc->texture)->dwContentsStamp;

    #if( DX >= 6 )        
        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 ), ((LPDDRAWI_DDRAWSURFACE_LCL)
            (TXTRHNDL_PTR(pRc->texture)->surfLcl))->lpDDPalette->lpLcl->lpGbl->lpColorTable,
            TXTRDESC_FROM_HNDL(pRc->texture)->formatFlags );
    #else
        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 ), ((LPDDRAWI_DDRAWSURFACE_LCL)
            (TXTRHNDL_PTR(pRc->texture)->surfLcl))->lpDDPalette->lpLcl->lpGbl->lpColorTable );
    #endif          
        
        _D3(currentPalette) = (void *) PALETTEGBL(pRc->texture);
        CMDFIFO_RELOAD( cmdFifo );
      }
#endif
     CMDFIFO_EPILOG( cmdFifo );
    }

    if( _D3(last).changed )
    {
      CMDFIFO_PROLOG( cmdFifo );

      if (IS_NAPALM)
      {
          CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE * 7) + 10 ); //NAPALM_CU
      }
      else
      {
          CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE * 3) + 6 );
      }

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chromaKey, 0xF ) );
      SETPD( cmdFifo, ghw->fbzColorPath, _D3(last).chromaKey );
      
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 4, 1, fbzColorPath, 0xF ) );
      SETPD( cmdFifo, ghw->fbzColorPath, _D3(last).fbzColorPath );
      SETPD( cmdFifo, ghw->fogMode, _D3(last).fogMode );
      SETPD( cmdFifo, ghw->alphaMode, _D3(last).alphaMode );
      SETPD( cmdFifo, ghw->fbzMode, _D3(last).fbzMode );

      if (IS_NAPALM)
      {
         SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 1 ) ); //NAPALM_CU
         SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, _D3(last).combineModeFBI);
         SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 2 ) );
         SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU0)->combineMode, _D3(last).combineModeT0);
         SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 4 ) );
         SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU1)->combineMode, _D3(last).combineModeT1);      //NAPALM_CU
         SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 1, renderMode, 0 ) );
         SETPD( cmdFifo, ghw->renderMode, _D3(last).renderMode );
      }

      // If Z buffer is enabled then we need to send the current
      // ZBIAS value. - AS
      if(pRc->zEnable)
      {
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 1, zaColor, 0xF ) );
        SETPD( cmdFifo, ghw->zaColor, pRc->sst.zaColor );
      }
      
      _D3(last).changed = FALSE;

      CMDFIFO_EPILOG( cmdFifo );
    }

  } // HW_STATE_CHANGED

  return;
} 

