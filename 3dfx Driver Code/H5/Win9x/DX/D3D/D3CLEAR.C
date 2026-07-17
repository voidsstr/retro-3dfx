/*
** Copyright (c) 1997-1998, 3Dfx Interactive, Inc.
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
** File name: ddiclear
**
** Description: D3D clear callbacks
**
** $Revision: 12$
** $Date: 10/23/00 2:09:34 AM$
**
**
** $Log: 
**  12   3dfx      1.7.1.3     10/23/00 Johnny Trainor  Updated so we no longer use
**       surface local pointers.
**  11   3dfx      1.7.1.2     10/11/00 Brent           Forced check in to enforce
**       branching.
**  10   3dfx      1.7.1.1     09/22/00 Johnny Trainor  Preparation for DX8 support
**       in the driver. Modifications so that we can build the Win9x driver using
**       the Win98 DDK and the DX8 DDK. 
**  9    3dfx      1.7.1.0     08/10/00 Steve Rogers    Fixing PRS 15142: Shadows
**       of the empire has Z buffer problems.  This was caused by the fact that the
**       Z Access Optimization wasn't being turned off correctly.
**  8    3dfx      1.7         04/24/00 Matt McClure    Modifications to not
**       overwrite a reset state in Z_ACCESS_OPT.
**  7    3dfx      1.6         04/23/00 Matt McClure    Modification to check of
**       coming from the Z_ACCESS_OPT.  If we are clearing and the number of flips
**       is less than 6, then we reset the count to 0, instead of letting it hover
**       around 6.
**  6    3dfx      1.5         04/19/00 Matt McClure    Implementation of Z Buffer
**       Access Optimization.  Added code to support setup and reset of
**       optimization based on the amount of flips that have occurred without a Z
**       Buffer Clear.
**  5    3dfx      1.4         12/09/99 Chris W. Shaw   Changed clipping to use
**       viewport bounds instead of rendertarget bounds for V3 and Napalm.
**  4    3dfx      1.3         10/28/99 Christopher Wilcox Hardware definition
**       changes to merge divergent h3defs.h.
**  3    3dfx      1.2         10/19/99 Bob Seitsinger  Disable dither rotation
**       (renderMode[25]) for fastfill command.
**  2    3dfx      1.1         10/01/99 Christopher Wilcox Removed P6FENCE macros,
**       which are no longer necessary even when !CMDFIFO, since register space is
**       not write combined.
** 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 4     9/07/99 10:49a Bseitsin
** Stencil buffer clear fix.
** 
** 3     7/13/99 2:27p Cshaw
** Added runtime support for Napalm's multitexturing (NAPALM_CU).
** 
** 2     6/16/99 3:45p Cshaw
** Added more texop support for texturefactors on color1, and alpha
** replicate.
** 
** 1     6/02/99 6:40a Michael
** Branch from H3
** 
** 25    5/24/99 5:04p Bseitsin
** Removal of Antialiasing code.
** 
** 24    4/21/99 3:10p Edwin
** Remove rgb16to24() references, no longer use.
** 
** 22    3/06/99 12:58a Brent
** Checked in for Scott Kephart -   STB-SK 03/06/99 part of fix for PRS
** 4645 -- free count was previously off by one.
** 
** 21    1/29/99 10:48a Cshaw
** Added unified headers.
** 
** 20    1/22/99 11:49a Ken
** removed color buffer clear from ddiClearSD that was happening
** incorrectly when the clear-color bit was not set.    Tested 3d winbench
** (improved 55 points!), multi-texture sdk app, rogue squadron, forsaken
** 
** 19    1/18/99 2:07p Adrians
** Removed obsolete variables and associated code.
** 
** 18    1/12/99 8:03a Cshaw
** Added runtime switches to the performance analysis code (use SIce to
** modify).
** 
** 17    1/07/99 12:00p Cshaw
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
** 16    12/02/98 8:24a Martin
** Turn off anti-aliasing by default and rename some variables for
** super-sampling AA.
** 
** 15    11/22/98 8:54p Andrew
** Changes to support multi-monitor
** 
** 14    11/19/98 4:10p Adrians
** Fix for flashing tri's in WinBench99.  We were not restoring zaColor
** after using it to clear the ZBuffer.
** 
** 13    8/28/98 12:37p Martin
** Do super-sampling AA when the registry key is set to 2.  Do edge AA if
** registry key == 1 OR we are not running at the 2 magical resolutions of
** 640x480 or 800x600.
** 
** No more conditional compilation of AA.
** 
** 12    8/24/98 5:23p Adrians
** Fix for SGRAM SuperSampling.
** 
** 11    8/23/98 12:49a Adrians
** Added support for AA SuperSampling.
** 
** 10    8/04/98 4:38p Martin
** ddiClearSG: too much space allocated in CHECKROOM
** ddiClearSD: not enough space allocated in CHECKROOM
** 
** both fcn's should now be allocating the maximum of what they will need.
** 
** 9     7/31/98 10:33p Miriam
** D3D & Glide cooperation. Allow each API to set/reset state to indicate
** that  the HW state has been changed.
** 
** 8     7/31/97 12:52p Miriam
** Install SDRAM or SGRAM clear based on runtime info.
** 
** 7     7/26/98 6:12p Adrians
** Change for SDRAM clears.
** 
** 6     7/01/98 3:49p Miriam
** Clip registers are now set for drawing surfaces.
** 
** 5     6/01/98 1:47p Ken
** added isSdram to globaldata, settable through control1() calls
** (probably should put this in the registry), changed d3d ddiClear to
** look at this to determine whether or not to issue a dithered clear (to
** force block writes on/off)
** 
** 4     5/29/98 10:29a Suninn
** add check for null dwReserved1
** 
** 3     5/26/98 2:49p Suninn
** fix ddiClear
** 
** 2     5/22/98 9:11a Suninn
** use GET_HW_ADDR()
** 
** 1     5/14/98 6:08p Miriam
** Moved from d3drwprm.c.
** 
*/

#include "precomp.h"

// Fix for building with Windows 98 DDK (Must include DDrawI first)
#include "ddrawi.h"
#include "d3dhal.h"
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h" 
#include "fifomgr.h"
#include "d3contxt.h"
#include "d3txtr.h"

// SGRAM clean
/*-------------------------------------------------------------------
Function Name:  ddiClearSG
Description:    Clears Z and Target buffer.
Information:    DWORD __stdcall ddiClearSG( LPD3DHAL_CLEARDATA pcd )

Return:         DWORD DDHAL_DRIVER_HANDLED
                  pcd->ddrval = DD_OK;

-------------------------------------------------------------------*/
DWORD __stdcall ddiClearSG( LPD3DHAL_CLEARDATA pcd )
{
  SETUP_PPDEV(pcd->dwhContext)
  int           cnt;
  FxU32         fbzMode = 0;
  FxU32         bltRgb24;
  RC           *pRc;

  CMDFIFO_PROLOG(cmdFifo);
  D3D_ENTRY( "ddiClear" );

#if defined( NULLDRIVER ) 
  if (!_D3(ondrtZB)) {
	pcd->ddrval = DD_OK;
	D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif
  
  pRc = CONTEXT_PTR(pcd->dwhContext);

  for (cnt = 0; cnt < (int) pcd->dwNumRects; ++cnt)
  {
    DWORD auxBufferAddr;
    DWORD colBufferAddr;
    
    CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE * 5) + 7 + (PH4_SIZE * 1) + 4); 

    // check to see if Z buffer is attached. d3dtest will try and clear Z when
    // there is no Z attached.
    if ((pcd->dwFlags & D3DCLEAR_ZBUFFER) && (pRc->DDSZHndl != 0) && 
    (TXTRHNDL_PTR(pRc->DDSZHndl)->surfData !=0))
    {

#ifdef Z_ACCESS_OPT
       // If we are clearing the Z Buffer, we must reset the Z Optimization state
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
      fbzMode |= SST_ZAWRMASK;
      
      auxBufferAddr = TXTRHNDL_PTR(pRc->DDSZHndl)->surfData->hwPtr;
      // Z is always set to 0xFFFF (max depth field in DX5)

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, auxBufferAddr, 0xf ) );
      if(IS_TILED(auxBufferAddr))
      {
        DWORD auxBufferStride;
          
        auxBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;
        SETPD( cmdFifo, ghw->auxBufferAddr,   (auxBufferAddr & 0x7fffffffL));
        SETPD( cmdFifo, ghw->auxBufferStride, auxBufferStride);
      }
      else
      {
        SETPD( cmdFifo, ghw->auxBufferAddr,   auxBufferAddr);
        SETPD( cmdFifo, ghw->auxBufferStride, TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch);
      }
    }
    
    if( (pcd->dwFlags & D3DCLEAR_TARGET) && (TXTRHNDL_PTR(pRc->DDSHndl)->surfData != NULL) )
    {
      // set dither option here... MIRIAM ???

      // assume fill color is always rgb888 (supposed to be format of destination but
      // this will brake winbench
      fbzMode |= SST_RGBWRMASK;
      bltRgb24 = pcd->dwFillColor;
      
      colBufferAddr = TXTRHNDL_PTR(pRc->DDSHndl)->surfData->hwPtr;

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, colBufferAddr, 0xf ) );
      if(IS_TILED(colBufferAddr))
      {
        DWORD colBufferStride;
          
        colBufferStride = (_DS(ddTileStride) & 0x3FFFL) | 0x8000L;
        SETPD( cmdFifo, ghw0->colBufferAddr, (colBufferAddr & 0x7FFFFFFFL));
        SETPD( cmdFifo, ghw0->colBufferStride, colBufferStride);
      }
      else
      {
        SETPD( cmdFifo, ghw->colBufferAddr,   colBufferAddr);
        SETPD( cmdFifo, ghw0->colBufferStride, TXTRHNDL_PTR(pRc->DDSHndl)->lPitch);
      }
    }

    SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R2|R3|R8, fbzMode, 0x0 ) );
    SETPD( cmdFifo, ghw->fbzMode, fbzMode );

    SETPD( cmdFifo, ghw->clipLeftRight, ((int)(pcd->lpRects->x1) << 16) | ((int)pcd->lpRects->x2));
    SETPD( cmdFifo, ghw->clipBottomTop, ((int)(pcd->lpRects->y1) << 16) | ((int)pcd->lpRects->y2));

    SETPD( cmdFifo, ghw->zaColor, pcd->dwFillDepth );

    // offset too far for previous packet :-(
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, c1, 0xF ) );
    SETPD( cmdFifo, ghw->c1, bltRgb24 );
    if (IS_NAPALM) {//NAPALM_CU
      UPDATE_HW_STATE( SC_TEXTUREFACTOR );//This flag only needs to be set if TMU0/1 color1 reg changes

      // set target surface pixel depth.
      // this macro modifies the render mode bits in the renderMode register.
      SETRENDERMODEPIXELDEPTH(pRc, pRc->DDSHndl);

      // Make sure dither rotation is disabled. Broken for fastfills.
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0xf ) );
      SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode & ~SST_RM_DITHER_ROTATION);
    }

    // TRUE forces Banshee not to dither the color fill 
    // DX5 support dithering the viewport clear???
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fastfillCMD, 0xF ) );
    SETPD( cmdFifo, ghw->fastfillCMD, TRUE );     
  
    ++pcd->lpRects;
  }

  // Restore the clipping registers for current surface
  CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE * 2) + 3 );
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, clipLeftRight, 0xf ) );
#ifdef NEW_CLIP_FOR_GB
  SETPD( cmdFifo, ghw0->clipLeftRight, pRc->sst.clipLeftRight);
  SETPD( cmdFifo, ghw0->clipBottomTop, pRc->sst.clipBottomTop);
#else
  SETPD( cmdFifo, ghw0->clipLeftRight, (DWORD)pRc->lpDDS->lpGbl->wWidth);
  SETPD( cmdFifo, ghw0->clipBottomTop, (DWORD)pRc->lpDDS->lpGbl->wHeight);
#endif

  // Restore zaColor because this is used for ZBias
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, zaColor, 0xF ) );
  SETPD( cmdFifo, ghw->zaColor, pRc->sst.zaColor );
  
  _D3(last).changed = TRUE;
  CMDFIFO_EPILOG( cmdFifo );

  pcd->ddrval = DD_OK;
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}


/*-------------------------------------------------------------------
Function Name:  ddiClearSD
Description:    Clears Z and Target buffer.
Information:    DWORD __stdcall ddiClearSD( LPD3DHAL_CLEARDATA pcd )
Return:         DWORD DDHAL_DRIVER_HANDLED
                  pcd->ddrval = DD_OK;
                
-------------------------------------------------------------------*/
// SDRAM clear
DWORD __stdcall ddiClearSD( LPD3DHAL_CLEARDATA pcd )
{
  SETUP_PPDEV(pcd->dwhContext)
  int           cnt;
  FxU32         fbzMode = 0;
  FxU32         bltRgb24;
  RC           *pRc;

  CMDFIFO_PROLOG(cmdFifo);
  D3D_ENTRY( "ddiClear" );

#if defined( NULLDRIVER ) 
  if (!_D3(ondrtZB)) {
	pcd->ddrval = DD_OK;
	D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif
  
  pRc = CONTEXT_PTR(pcd->dwhContext);

  for (cnt = 0; cnt < (int) pcd->dwNumRects; ++cnt)
  {
    DWORD colBufferAddr;
    
    //STB-SK 03/06/99 part of fix for PRS 4645 -- free count was previously off by one.
    CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE * 9) + 12 + (PH4_SIZE * 2) + 9); 

    // check to see if Z buffer is attached. d3dtest will try and clear Z when
    // there is no Z attached.
    if ((pcd->dwFlags & D3DCLEAR_ZBUFFER) && (pRc->DDSZHndl != 0) && 
    (TXTRHNDL_PTR(pRc->DDSZHndl)->surfData !=0))
    {
#ifdef Z_ACCESS_OPT
        // If we are clearing the Z Buffer, we must reset the Z Optimization state
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

      colBufferAddr = GET_HW_OFFSET(pRc->DDSZHndl);
      // Z is always set to 0xFFFF (max depth field in DX5)

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, colBufferAddr, 0xf ) );
      if(IS_TILED(colBufferAddr))
      {
        DWORD colBufferStride;

        colBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;
        SETPD( cmdFifo, ghw->colBufferAddr,   (colBufferAddr & 0x7fffffffL));
        SETPD( cmdFifo, ghw->colBufferStride, colBufferStride);
      }
      else
      {
        SETPD( cmdFifo, ghw->colBufferAddr,   colBufferAddr);
        SETPD( cmdFifo, ghw->colBufferStride, TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch);
      } 

      SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R2|R3, fbzMode, 0x0 ) );
      SETPD( cmdFifo, ghw->fbzMode, SST_RGBWRMASK );
      SETPD( cmdFifo, ghw->clipLeftRight, ((int)(pcd->lpRects->x1) << 16) | ((int)pcd->lpRects->x2));
      SETPD( cmdFifo, ghw->clipBottomTop, ((int)(pcd->lpRects->y1) << 16) | ((int)pcd->lpRects->y2));
      
      // offset too far for previous packet :-(
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, c1, 0xF ) );
      SETPD( cmdFifo, ghw->c1, pcd->dwFillDepth );
      if (IS_NAPALM) {//NAPALM_CU
        UPDATE_HW_STATE( SC_TEXTUREFACTOR );//This flag only needs to be set if TMU0/1 color1 reg changes

        // set target surface pixel depth.
        // this macro modifies the render mode bits in the renderMode register.
        SETRENDERMODEPIXELDEPTH(pRc, pRc->DDSHndl);

        // Make sure dither rotation is disabled. Broken for fastfills.
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0xf ) );
        SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode & ~SST_RM_DITHER_ROTATION);
      }

      // TRUE forces Banshee not to dither the color fill
      // DX5 support dithering the viewport clear???
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fastfillCMD, 0xF ) );
      SETPD( cmdFifo, ghw->fastfillCMD, ~(SST_FASTFILL_DISABLE_DITHER));

      // Reset Color Buffer
      colBufferAddr = _D3(last).colBufferAddr;
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, colBufferAddr, 0xf ) );
      if(IS_TILED(colBufferAddr))
      {
        DWORD colBufferStride;
        
        colBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;
        SETPD( cmdFifo, ghw->colBufferAddr,   (colBufferAddr & 0x7fffffffL));
        SETPD( cmdFifo, ghw->colBufferStride, colBufferStride);
      }
      else
      {
        SETPD( cmdFifo, ghw->colBufferAddr,   colBufferAddr);
        SETPD( cmdFifo, ghw->colBufferStride, TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch);
      } 
    }
    
    if ((pcd->dwFlags & D3DCLEAR_TARGET) &&
    (TXTRHNDL_PTR(pRc->DDSHndl)->surfData !=0) )
    {
      // set dither option here... MIRIAM ???
      // assume fill color is always rgb888 (supposed to be format of
      // destination but this will brake winbench
      //
      fbzMode |= SST_RGBWRMASK;
      bltRgb24 = pcd->dwFillColor;

      colBufferAddr = GET_HW_OFFSET(pRc->DDSHndl);

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, colBufferAddr, 0xf ) );
	  if (IS_TILED(colBufferAddr))
	  {
		DWORD colBufferStride;
          
		colBufferStride = (_DS(ddTileStride) & 0x3FFFL) | 0x8000L;
		SETPD( cmdFifo, ghw0->colBufferAddr, (colBufferAddr & 0x7FFFFFFFL));
		SETPD( cmdFifo, ghw0->colBufferStride, colBufferStride);
	  }
	  else
	  {
		SETPD( cmdFifo, ghw->colBufferAddr,   colBufferAddr);
		SETPD( cmdFifo, ghw0->colBufferStride, TXTRHNDL_PTR(pRc->DDSHndl)->lPitch);
	  }

      SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R2|R3|R8, fbzMode, 0x0 ) );
      SETPD( cmdFifo, ghw->fbzMode, fbzMode );    

      SETPD( cmdFifo, ghw->clipLeftRight, ((int)(pcd->lpRects->x1) << 16) | ((int)pcd->lpRects->x2));
      SETPD( cmdFifo, ghw->clipBottomTop, ((int)(pcd->lpRects->y1) << 16) | ((int)pcd->lpRects->y2));

      SETPD( cmdFifo, ghw->zaColor, pcd->dwFillDepth );

      // offset too far for previous packet :-(
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, c1, 0xF ) );
      SETPD( cmdFifo, ghw->c1, bltRgb24 );
      if (IS_NAPALM) {//NAPALM_CU
        UPDATE_HW_STATE( SC_TEXTUREFACTOR );//This flag only needs to be set if TMU0/1 color1 reg changes

        // set target surface pixel depth.
        // this macro modifies the render mode bits in the renderMode register.
        SETRENDERMODEPIXELDEPTH(pRc, pRc->DDSHndl);

        // Make sure dither rotation is disabled. Broken for fastfills.
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0xf ) );
        SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode & ~SST_RM_DITHER_ROTATION);
      }

      // TRUE forces Banshee not to dither the color fill 
      // DX5 support dithering the viewport clear???
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fastfillCMD, 0xF ) );
#ifdef SDRAM_SUPPORT
      SETPD( cmdFifo, ghw->fastfillCMD, ~(SST_FASTFILL_DISABLE_DITHER));
#else
      SETPD( cmdFifo, ghw->fastfillCMD, TRUE );     
#endif      
    }
    
    ++pcd->lpRects;
  }

  // Restore the clipping registers for current surface
  CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE * 2) + 3 );
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, clipLeftRight, 0xf ) );
#ifdef NEW_CLIP_FOR_GB
  SETPD( cmdFifo, ghw0->clipLeftRight, pRc->sst.clipLeftRight);
  SETPD( cmdFifo, ghw0->clipBottomTop, pRc->sst.clipBottomTop);
#else
  SETPD( cmdFifo, ghw0->clipLeftRight, (DWORD)pRc->lpDDS->lpGbl->wWidth);
  SETPD( cmdFifo, ghw0->clipBottomTop, (DWORD)pRc->lpDDS->lpGbl->wHeight);
#endif

  // Restore zaColor because this is used for ZBias
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, zaColor, 0xF ) );
  SETPD( cmdFifo, ghw->zaColor, pRc->sst.zaColor );
  
  _D3(last).changed = TRUE;
  CMDFIFO_EPILOG( cmdFifo );

  pcd->ddrval = DD_OK;
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}
