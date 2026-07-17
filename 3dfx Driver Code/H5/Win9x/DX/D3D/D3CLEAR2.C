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
** File name: ddiclear2
**
** Description: D3D clear2 callbacks
**
** $Log: 
**  14   3dfx      1.9.1.3     10/23/00 Johnny Trainor  Updated so we no longer use
**       surface local pointers.
**  13   3dfx      1.9.1.2     10/11/00 Brent           Forced check in to enforce
**       branching.
**  12   3dfx      1.9.1.1     09/22/00 Johnny Trainor  Preparation for DX8 support
**       in the driver. Modifications so that we can build the Win9x driver using
**       the Win98 DDK and the DX8 DDK. 
** 
**  11   3dfx      1.9.1.0     08/10/00 Steve Rogers    Fixing PRS 15142: Shadows
**       of the empire has Z buffer problems.  This was caused by the fact that the
**       Z Access Optimization wasn't being turned off correctly.
**  10   3dfx      1.9         04/24/00 Matt McClure    Modifications to not
**       overwrite a reset state in Z_ACCESS_OPT.
**  9    3dfx      1.8         04/23/00 Matt McClure    Modification to check of
**       coming from the Z_ACCESS_OPT.  If we are clearing and the number of flips
**       is less than 6, then we reset the count to 0, instead of letting it hover
**       around 6.
**  8    3dfx      1.7         04/19/00 Matt McClure    Implementation of Z Buffer
**       Access Optimization.  Added code to support setup and reset of
**       optimization based on the amount of flips that have occurred without a Z
**       Buffer Clear.
**  7    3dfx      1.6         02/23/00 Christopher Wilcox Updated stencil and
**       zbuffer code to workaround various Napalm problems.
**  6    3dfx      1.5         12/16/99 Chris W. Shaw   Fixed a "#if to #ifdef"
**  5    3dfx      1.4         12/09/99 Chris W. Shaw   Changed clipping to use
**       viewport bounds instead of rendertarget bounds for V3 and Napalm.
**  4    3dfx      1.3         10/28/99 Christopher Wilcox Hardware definition
**       changes to merge divergent h3defs.h.
**  3    3dfx      1.2         10/01/99 Christopher Wilcox Removed P6FENCE macros,
**       which are no longer necessary even when !CMDFIFO, since register space is
**       not write combined.
** 
**  2    3dfx      1.1         09/16/99 Bob Seitsinger  Disable dither matrix
**       rotation (renderMode[25]) for fastfill command.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 8     9/07/99 10:49a Bseitsin
** Stencil buffer clear fix.
** 
** 7     8/06/99 11:43a Cshaw
** Fixed a packet header/packet data mismatch for the case when Z buffer
** is not present (ddiClear2sg).
** 
** 6     7/13/99 2:27p Cshaw
** Added runtime support for Napalm's multitexturing (NAPALM_CU).
** 
** 5     7/13/99 10:45a Andrew
** changed unsigned int cast to float2int to fix Invalid Instruction Trap
** 
** 4     7/05/99 11:16a Bseitsin
** Further fixes to ddiClear2* code to prevent invalid dereferencing.
** 
** 3     7/02/99 4:55p Bseitsin
** In ddiClear2SG, check pRc->lpDDSZ before dereferencing when combining
** Zand stencil fill values.
** 
** 2     6/16/99 3:45p Cshaw
** Added more texop support for texturefactors on color1, and alpha
** replicate.
** 
** 1     6/04/99 7:16p Bseitsin
** Clear2 callback routines.
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
Function Name:  ddiClear2SG
Description:    Clears render target and zbuffer.
Return:         DWORD DDHAL_DRIVER_HANDLED, pcd->ddrval = DD_OK;

-------------------------------------------------------------------*/
DWORD __stdcall ddiClear2SG( LPD3DHAL_CLEAR2DATA pcd )
{
  SETUP_PPDEV(pcd->dwhContext)
  int           cnt;
  FxU32         fbzMode = 0;
  FxU32         bltRgb24;
  RC           *pRc;

  CMDFIFO_PROLOG(cmdFifo);
  D3D_ENTRY( "ddiClear2SG" );

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

    CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE * 7) + 9 + (PH4_SIZE * 1) + 3); 

    // set target surface pixel depth.
    // this macro modifies the render mode bits in the renderMode register.
    SETRENDERMODEPIXELDEPTH(pRc, pRc->DDSHndl);

    // Also, make sure dither rotation is disabled. Broken for fastfills.
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0xf ) );
    SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode & ~SST_RM_DITHER_ROTATION);

    if ((pcd->dwFlags & D3DCLEAR_TARGET) && (TXTRHNDL_PTR(pRc->DDSHndl)->surfData !=0))
    {
      // assume fill color is always rgb888 (supposed to be format of destination but
      // this will brake winbench
      fbzMode |= SST_RGBWRMASK;
      bltRgb24 = pcd->dwFillColor;
      
      colBufferAddr = GET_HW_OFFSET(pRc->DDSHndl);

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

    // check to see if Z buffer is attached. d3dtest will try and clear Z when
    // there is no Z attached.
    if ((pcd->dwFlags & D3DCLEAR_ZBUFFER) && (pRc->DDSZHndl != 0) && (TXTRHNDL_PTR(pRc->DDSZHndl)->surfData !=0))
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

      auxBufferAddr = GET_HW_OFFSET(pRc->DDSZHndl);

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

    // Set Z fill value.
    if (pcd->dwFlags & D3DCLEAR_ZBUFFER)
    {
      fbzMode |= SST_ZAWRMASK; // Enable zbuffer clear
      
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, zaColor, 0xF ) );

      // dvFillDepth is 0.0 - 1.0.
      // dwZBitMask should be 0x0000FFFF for 16bpp and 0x00FFFFFF for 32bpp.
#ifdef WINNT
      SETPD(cmdFifo, ghw->zaColor, (DWORD)(pcd->dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));
#else
      SETPD(cmdFifo, ghw->zaColor, (float2int(pcd->dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask)));
#endif
    }

    SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R2|R3, fbzMode, 0x0 ) );
    SETPD( cmdFifo, ghw->fbzMode, fbzMode );
    SETPD( cmdFifo, ghw->clipLeftRight, ((int)(pcd->lpRects->x1) << 16) | ((int)pcd->lpRects->x2));
    SETPD( cmdFifo, ghw->clipBottomTop, ((int)(pcd->lpRects->y1) << 16) | ((int)pcd->lpRects->y2));

    // offset too far for previous packet :-(
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, c1, 0xF ) );
    SETPD( cmdFifo, ghw->c1, bltRgb24 );

    if (IS_NAPALM) {//NAPALM_CU
      UPDATE_HW_STATE( SC_TEXTUREFACTOR );//This flag only needs to be set if TMU0/1 color1 reg changes
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
Function Name:  ddiClear2SD
Description:    Clears render target and zbuffer.
Return:         DWORD DDHAL_DRIVER_HANDLED, pcd->ddrval = DD_OK;
-------------------------------------------------------------------*/

// SDRAM clear
DWORD __stdcall ddiClear2SD( LPD3DHAL_CLEAR2DATA pcd )
{
  SETUP_PPDEV(pcd->dwhContext)
  int           cnt;
  FxU32         fbzMode = 0;
  FxU32         bltRgb24;
  RC           *pRc;

  CMDFIFO_PROLOG(cmdFifo);
  D3D_ENTRY( "ddiClear2SD" );

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
    
    CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE * 10) + 13 + (PH4_SIZE * 2) + 6); 

    // set target surface pixel depth.
    // this macro modifies the render mode bits in the renderMode register.
    SETRENDERMODEPIXELDEPTH(pRc, pRc->DDSHndl);

    // Also, make sure dither rotation is disabled. Broken for fastfills.
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0xf ) );
    SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode & ~SST_RM_DITHER_ROTATION );

    // check to see if Z buffer is attached. d3dtest will try and clear Z when
    // there is no Z attached.
    if ((pcd->dwFlags & D3DCLEAR_ZBUFFER) && (pRc->DDSZHndl != 0) && (TXTRHNDL_PTR(pRc->DDSZHndl)->surfData != 0))
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

      if (IS_NAPALM) {//NAPALM_CU
        UPDATE_HW_STATE( SC_TEXTUREFACTOR );//This flag only needs to be set if TMU0/1 color1 reg changes
      }

      // Set Z fill value.
      if (pcd->dwFlags & D3DCLEAR_ZBUFFER)
      {
        fbzMode |= SST_RGBWRMASK;

        // Use c1 register here instead of zaColor; per documentation.
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, c1, 0xF ) );

        // dvFillDepth is 0.0 - 1.0.
        // dwZBitMask should be 0x0000FFFF for 16bpp and 0x00FFFFFF for 32bpp.
#ifdef WINNT
        SETPD(cmdFifo, ghw->c1, (DWORD)(pcd->dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));
#else
        SETPD(cmdFifo, ghw->c1, (float2int(pcd->dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask)));
#endif
      }

      SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R2|R3, fbzMode, 0x0 ) );
      SETPD( cmdFifo, ghw->fbzMode, fbzMode );
      SETPD( cmdFifo, ghw->clipLeftRight, ((int)(pcd->lpRects->x1) << 16) | ((int)pcd->lpRects->x2));
      SETPD( cmdFifo, ghw->clipBottomTop, ((int)(pcd->lpRects->y1) << 16) | ((int)pcd->lpRects->y2));
      
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
    (TXTRHNDL_PTR(pRc->DDSHndl)->surfData != 0) )
    {
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

      SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R2|R3, fbzMode, 0x0 ) );
      SETPD( cmdFifo, ghw->fbzMode, fbzMode );
      SETPD( cmdFifo, ghw->clipLeftRight, ((int)(pcd->lpRects->x1) << 16) | ((int)pcd->lpRects->x2));
      SETPD( cmdFifo, ghw->clipBottomTop, ((int)(pcd->lpRects->y1) << 16) | ((int)pcd->lpRects->y2));

      // offset too far for previous packet :-(
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, c1, 0xF ) );
      SETPD( cmdFifo, ghw->c1, bltRgb24 );

      if (IS_NAPALM) {//NAPALM_CU
        UPDATE_HW_STATE( SC_TEXTUREFACTOR );//This flag only needs to be set if TMU0/1 color1 reg changes
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

  _D3(last).changed = TRUE;
  CMDFIFO_EPILOG( cmdFifo );

  pcd->ddrval = DD_OK;
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
}
