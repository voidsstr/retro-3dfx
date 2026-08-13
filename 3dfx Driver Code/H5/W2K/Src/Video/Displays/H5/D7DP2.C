/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** $Revision: 59$
** $Date: 10/31/00 1:58:51 AM$
**
*/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)

#ifndef WINNT
// Everything NT builds need are in precomp.h!
#include "d3txtr.h"
#include "d3contxt.h"
#include "ddglobal.h"
#include "d3global.h"
#endif

/**************************************************************************
* D E F I N E S
***************************************************************************/

#define NORMAL_DBG_LEVEL    2

/**************************************************************************
* F U N C T I O N   P R O T O T Y P E S
***************************************************************************/

static STATEBLOCK *FindStateBlock(RC *, DWORD);
static HRESULT AddStateBlockToTable(RC *, DWORD, STATEBLOCK *);
static STATEBLOCK *CompressStateBlock(RC *, STATEBLOCK *);

extern VOID _D3D_OP_MStream_SetSrc(RC *pRc, DWORD dwStream, DWORD dwVBHandle, DWORD dwStride);
extern VOID _D3D_OP_MStream_SetIndices(RC *pRc, DWORD dwVBHandle, DWORD dwStride);
extern VOID _D3D_OP_VertexShader_Set(RC *pRc, DWORD dwVtxShaderHandle);

/**************************************************************************
* P U B L I C   F U N C T I O N S
***************************************************************************/

/*-------------------------------------------------------------------
Function Name:  SetRenderTarget

Description:

Return:
-------------------------------------------------------------------*/

HRESULT
SetRenderTarget ( RC    *pRc,
                  DWORD hRenderTarg,
                  DWORD hZBuffer )
{
  TXTRHNDL *pDDS;
  TXTRHNDL * pDDSZ;

  SETUP_PPDEV(pRc)

  // lookup local surfaces based on handles
  pDDS  = TXTRHNDL_PTR(hRenderTarg);

  if (hZBuffer)
  {
    pDDSZ = TXTRHNDL_PTR(hZBuffer);
    D3DPRINT(NORMAL_DBG_LEVEL, "SetRenderTarget: New Z buffer %08x", pDDSZ);
  }
  else
  {
    pDDSZ = NULL;
    D3DPRINT(NORMAL_DBG_LEVEL, "SetRenderTarget: Z is NULL");
  }

  // cannot render into system memory
  if (DDSCAPS_SYSTEMMEMORY & pDDS->dwCaps)
  {
    D3DPRINT(0, "SetRenderTarget: Primary in system memory. Can't do.");
    return DDERR_CURRENTLYNOTAVAIL;
  }

  if (pDDSZ)
  {
    // cannot render if z buffer is in system memory
    if (DDSCAPS_SYSTEMMEMORY & pDDSZ->dwCaps)
    {
      D3DPRINT(0, "SetRenderTarget: Z in system memory. Can't do.");
      return DDERR_CURRENTLYNOTAVAIL;
    }

    pRc->DDSZHndl = hZBuffer;

#ifdef WINNT
    // to fix potential access violation issues with NT Stress
    // verify we have a TXTRHNDL for this texture before dereferencing it
    //
    // no additional check for NULL TXTRHNDL needed here since this function
    // won't be called if the TXTRHNDL for DDSZHndl is NULL
#endif
    ASSERTDD((DWORD)pRc->DDSZHndl == TXTRHNDL_PTR(pRc->DDSZHndl)->txtrID, " z buffer handle's don't match!");
  }
  else
  {
    pRc->DDSZHndl = 0;
  }

  pRc->DDSHndl = hRenderTarg;

#ifdef WINNT
  // to fix potential access violation issues with NT Stress
  // verify we have a TXTRHNDL for this texture before dereferencing it
  //
  // no additional check for NULL TXTRHNDL needed here since this function
  // won't be called if the TXTRHNDL for DDSHndl is NULL
#endif
  ASSERTDD((DWORD)pRc->DDSHndl == TXTRHNDL_PTR(pRc->DDSHndl)->txtrID, " render targ surfLcl's don't match!");

  D3DPRINT(NORMAL_DBG_LEVEL, "SetRenderTarget, dwhContext=%08lXh DDSHndl =%08lXh, DDSZHndl =%08lXh",
           pRc,pRc->DDSHndl,pRc->DDSZHndl);

  _D3(lastContext) = 0;
  UPDATE_HW_STATE(SC_SOMETHING|SC_BUFFERS);

  return D3D_OK;
}

// This function is not currently being used, but it may be useful, so keep it.
// Bob S. - 2/18/2000
/*-------------------------------------------------------------------
Function Name:  Clear

Description:    Clear buffers using 2D blt engine.

Return:         D3D_OK
-------------------------------------------------------------------*/
HRESULT
Clear ( RC              *pRc,
        DWORD           dwFlags,
        DWORD           dwFillColor,
        D3DVALUE        dvFillDepth,
        DWORD           dwFillStencil,
        RECT            *pRects,
        DWORD           count )
{
  SETUP_PPDEV(pRc)
  DWORD   i;
  DWORD   dstPixelFormat, dstBaseAddr, dstFormat, dstSize, dstXY;
  DWORD   color;

  CMDFIFO_PROLOG(cmdfifo);

  // clear of render target
  if (D3DCLEAR_TARGET & dwFlags)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "clearing render target: %8lXh with rgb888 %8lXh", pRc->DDSHndl, dwFillColor);

    ASSERTDD(0 != pRc->DDSHndl, " NULL render target");

#ifdef WINNT
    GETPIXELFORMAT(TXTRHNDL_PTR(pRc->DDSHndl)->dwBitCnt >> 3, dstPixelFormat);
#else
    if((TXTRHNDL_PTR(pRc->DDSHndl)->dwFlags) & DDRAWISURF_HASPIXELFORMAT )
    {
      GETPIXELFORMAT(TXTRHNDL_PTR(pRc->DDSHndl)->dwBitCnt >> 3, dstPixelFormat);
    }
    else
    {
      BYTE ByteDepth = GETPRIMARYBYTEDEPTH;
      GETPIXELFORMAT(ByteDepth, dstPixelFormat);
    }
#endif
//#ifdef WINNT
//    dstBaseAddr = GET_HW_ADDR(pRc->lpDDS);
//#else
    dstBaseAddr = GET_HW_OFFSET(pRc->DDSHndl);
//#endif
#if ENABLE_TILED_HEAP
    if (IS_TILED(dstBaseAddr))
      BLTFMT(_FF(ddTileStride), dstPixelFormat, dstFormat);
    else
#endif
      BLTFMT(TXTRHNDL_PTR(pRc->DDSHndl)->lPitch, dstPixelFormat, dstFormat);

    // dwFillColor is RGB888, so convert it to the hw format of the render target
    if (TXTRHNDL_PTR(pRc->DDSHndl)->dwBitCnt == 32)
    {
      color = dwFillColor & 0x00FFFFFF;
    }
    else
    {
      // convert rgb888 to rgb565
      // the macros from wingdi.h assume the color is BGR but we have RGB
      // so use GetBValue to get the red value and use GetRValue to get the blue value
      color = ((GetBValue(dwFillColor) >> 3) << 11) |     // actually this is red
              ((GetGValue(dwFillColor) >> 2) <<  5) |
              ((GetRValue(dwFillColor) >> 3) <<  0);      // actually this is blue
      D3DPRINT(NORMAL_DBG_LEVEL, "  clear with rgb565 %8lXh", color);
    }


    CMDFIFO_CHECKROOM(cmdfifo, 4 + (count * 4));

    SETPH(cmdfifo, CMDFIFO_BUILD_PK2(dstBaseAddrBit |
                                     dstFormatBit   |
                                     colorForeBit));
    SETPD(cmdfifo, ghw2D->dstBaseAddr, dstBaseAddr);
    SETPD(cmdfifo, ghw2D->dstFormat,   dstFormat);
    SETPD(cmdfifo, ghw2D->colorFore,   color);

    //loop over rects to clear
    for (i = 0; i < count; i++)
    {
      BLTXY(pRects[i].left, pRects[i].top, dstXY);
      BLTSIZE(pRects[i].right - pRects[i].left,
              pRects[i].bottom - pRects[i].top,
              dstSize);

      SETPH(cmdfifo, CMDFIFO_BUILD_PK2(dstSizeBit |
                                       dstXYBit   |
                                       commandBit));
      SETPD(cmdfifo, ghw2D->dstSize, dstSize);
      SETPD(cmdfifo, ghw2D->dstXY,   dstXY);
      SETPD(cmdfifo, ghw2D->command, SSTG_RECTFILL | SSTG_GO | (SSTG_ROP_SRC << SSTG_ROP0_SHIFT));
    }

#if 0 // Disable this for now.
//#ifdef SLI_AA
    // Clear Secondary color buffer
    if (_DD(ddAAModeEnabled))
    {
#ifdef WINNT
        dstBaseAddr = GET_AAHW_ADDR(pRc->lpDDS);
#else
        dstBaseAddr = GET_AAHW_OFFSET(pRc->DDSHndl);
#endif

        CMDFIFO_CHECKROOM(cmdfifo, 2 + (count * 4));

        SETPH(cmdfifo, CMDFIFO_BUILD_PK2(dstBaseAddrBit));
        SETPD(cmdfifo, ghw2D->dstBaseAddr, dstBaseAddr);

        //loop over rects to clear
        // !!!! NEED TO FIGURE OUT HOW TO APPLY THE JITTER VALUES !!!!
        for (i = 0; i < count; i++)
        {
          BLTXY(pRects[i].left, pRects[i].top, dstXY);
          BLTSIZE(pRects[i].right - pRects[i].left,
                  pRects[i].bottom - pRects[i].top,
                  dstSize);

          SETPH(cmdfifo, CMDFIFO_BUILD_PK2(dstSizeBit |
                                           dstXYBit   |
                                           commandBit));
          SETPD(cmdfifo, ghw2D->dstSize, dstSize);
          SETPD(cmdfifo, ghw2D->dstXY,   dstXY);
          SETPD(cmdfifo, ghw2D->command, SSTG_RECTFILL | SSTG_GO | (SSTG_ROP_SRC << SSTG_ROP0_SHIFT));
        }
    }
#endif // SLI_AA
  }

  // zbuffer & stencil buffer clear
  if (((D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL) & dwFlags) && (0 != pRc->DDSZHndl))
  {
#ifdef WINNT
    D3DPRINT(NORMAL_DBG_LEVEL, "clearing zbuffer: %8lXh with %8lXh", pRc->DDSZHndl,
             (DWORD)(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));
#else
    D3DPRINT(NORMAL_DBG_LEVEL, "clearing zbuffer: %8lXh with %8lXh", pRc->DDSZHndl,
             (DWORD)(ddftol(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask)));
#endif

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

    GETPIXELFORMAT(TXTRHNDL_PTR(pRc->DDSZHndl)->dwZDepth >> 3, dstPixelFormat);
//#ifdef WINNT
//    dstBaseAddr = GET_HW_ADDR(pRc->lpDDSZ);
//#else
    dstBaseAddr = GET_HW_OFFSET(pRc->DDSZHndl);
//#endif
#if ENABLE_TILED_HEAP
    if (IS_TILED(dstBaseAddr))
      BLTFMT(_FF(ddTileStride), dstPixelFormat, dstFormat);
    else
#endif
      BLTFMT(TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch, dstPixelFormat, dstFormat);


    CMDFIFO_CHECKROOM(cmdfifo, 4 + (count * 4));

    SETPH(cmdfifo, CMDFIFO_BUILD_PK2(dstBaseAddrBit |
                                     dstFormatBit   |
                                     colorForeBit));
    SETPD(cmdfifo, ghw2D->dstBaseAddr, dstBaseAddr);
    SETPD(cmdfifo, ghw2D->dstFormat,   dstFormat);
    if (D3DCLEAR_STENCIL & dwFlags)
    {
#ifdef WINNT
      SETPD(cmdfifo, ghw2D->colorFore,
            (dwFillStencil << 24) | (DWORD)(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));
#else
      SETPD(cmdfifo, ghw2D->colorFore,
            (dwFillStencil << 24) | (DWORD)(ddftol(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask)));
#endif
    }
    else
    {
#ifdef WINNT
      SETPD(cmdfifo, ghw2D->colorFore, (DWORD)(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));
#else
      SETPD(cmdfifo, ghw2D->colorFore, (DWORD)(ddftol(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask)));
#endif
    }

    //loop over rects to clear
    for (i = 0; i < count; i++)
    {
      BLTXY(pRects[i].left, pRects[i].top, dstXY);
      BLTSIZE(pRects[i].right - pRects[i].left,
              pRects[i].bottom - pRects[i].top,
              dstSize);

      SETPH(cmdfifo, CMDFIFO_BUILD_PK2(dstSizeBit |
                                       dstXYBit   |
                                       commandBit));
      SETPD(cmdfifo, ghw2D->dstSize, dstSize);
      SETPD(cmdfifo, ghw2D->dstXY,   dstXY);
      SETPD(cmdfifo, ghw2D->command, SSTG_RECTFILL | SSTG_GO | (SSTG_ROP_SRC << SSTG_ROP0_SHIFT));
    }

#if 0 // Disable this for now.
//#ifdef SLI_AA
    // Clear Secondary Z buffer
    if (_DD(ddAAModeEnabled))
    {
#ifdef WINNT
        dstBaseAddr = GET_AAHW_ADDR(pRc->lpDDSZ);
#else
        dstBaseAddr = GET_AAHW_OFFSET(pRc->DDSZHndl);
#endif

        CMDFIFO_CHECKROOM(cmdfifo, 2 + (count * 4));

        SETPH(cmdfifo, CMDFIFO_BUILD_PK2(dstBaseAddrBit));
        SETPD(cmdfifo, ghw2D->dstBaseAddr, dstBaseAddr);

        //loop over rects to clear
        // !!!! NEED TO FIGURE OUT HOW TO APPLY THE JITTER VALUES !!!!
        for (i = 0; i < count; i++)
        {
          BLTXY(pRects[i].left, pRects[i].top, dstXY);
          BLTSIZE(pRects[i].right - pRects[i].left,
                  pRects[i].bottom - pRects[i].top,
                  dstSize);

          SETPH(cmdfifo, CMDFIFO_BUILD_PK2(dstSizeBit |
                                           dstXYBit   |
                                           commandBit));
          SETPD(cmdfifo, ghw2D->dstSize, dstSize);
          SETPD(cmdfifo, ghw2D->dstXY,   dstXY);
          SETPD(cmdfifo, ghw2D->command, SSTG_RECTFILL | SSTG_GO | (SSTG_ROP_SRC << SSTG_ROP0_SHIFT));
        }
    }
#endif // SLI_AA
  }

  CMDFIFO_EPILOG(cmdfifo);

  return D3D_OK;
} // Clear

/*-------------------------------------------------------------------
Function Name:  FastFill_2pass

Description:    Clear buffers using Fast Fill command. Due to the limitation
                on V3 SDRAM, we clear render target and z buffer with two
                separate calls.

Return:         D3D_OK
-------------------------------------------------------------------*/

HRESULT
FastFill_2pass (RC *pRc, D3DHAL_DP2CLEAR *pClear, DWORD count)
{
  SETUP_PPDEV(pRc)
  DWORD   i;
  DWORD   colBufferAddr;
  DWORD   auxBufferAddr;
  RECT    *pTmpRects;

  DWORD   dwFlags =         pClear->dwFlags;        // Clear flags
  DWORD   dwFillColor =     pClear->dwFillColor;    // Render target fill color
  float   dvFillDepth =     pClear->dvFillDepth;    // Z buffer fill value
  DWORD   dwFillStencil =   pClear->dwFillStencil;  // Stencil buffer fill value
  RECT    *pRects =         (LPRECT)((LPBYTE)pClear + sizeof(D3DHAL_DP2CLEAR) - sizeof(RECT));

#if defined(TnL_HAL)

// [Randy Spurlock] Need to add Pure Device Support Here! (Zero and User Rects)

#endif
  CMDFIFO_PROLOG(cmdfifo);

  // zbuffer & stencil buffer clear

  if ( ((D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL) & dwFlags) &&
       (0 != pRc->DDSZHndl) && (0 != TXTRHNDL_PTR(pRc->DDSZHndl)->surfData)
     )
  {
#ifdef WINNT
    D3DPRINT(NORMAL_DBG_LEVEL, "clearing zbuffer: %8lXh with %8lXh", pRc->DDSZHndl,
      (DWORD)(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));
#else
    D3DPRINT(NORMAL_DBG_LEVEL, "clearing zbuffer: %8lXh with %8lXh", pRc->DDSZHndl,
      (DWORD)(ddftol(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask)));
#endif

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

//#ifdef WINNT
//    auxBufferAddr = GET_HW_ADDR(pRc->lpDDSZ);
//#else
    auxBufferAddr = GET_HW_OFFSET(pRc->DDSZHndl);
//#endif

 	CMDFIFO_CHECKROOM(cmdfifo, (PH1_SIZE) + 2);
    SETPH(cmdfifo, CMDFIFO_BUILD_PK1(2, 1, auxBufferAddr, 0xf));

#if ENABLE_TILED_HEAP

    if (IS_TILED(auxBufferAddr))
    {
      DWORD auxBufferStride;

      auxBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;
      SETPD(cmdfifo, ghw->auxBufferAddr, (auxBufferAddr & 0x7FFFFFFFL));
      SETPD(cmdfifo, ghw->auxBufferStride, auxBufferStride);
    }
    else
#endif
    {
      SETPD(cmdfifo, ghw->auxBufferAddr, auxBufferAddr);
      SETPD(cmdfifo, ghw->auxBufferStride, TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch);
    }

    // Set Z fill value.
    if (dwFlags & D3DCLEAR_ZBUFFER)
    {

	CMDFIFO_CHECKROOM(cmdfifo, (PH1_SIZE * 2) + 2);
  	SETPH(cmdfifo, CMDFIFO_BUILD_PK1(1, 0, fbzMode, 0xF));
 	SETPD(cmdfifo, ghw->fbzMode, SST_ZAWRMASK);	// Enable zbuffer clear

    SETPH( cmdfifo, CMDFIFO_BUILD_PK1( 1, 0, zaColor, 0xF ) );

    // dvFillDepth is 0.0 - 1.0.
    // dwZBitMask should be 0x0000FFFF for 16bpp and 0x00FFFFFF for 32bpp.
    SETPD(cmdfifo, ghw->zaColor, (DWORD)(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));
    }

    // loop over rectangles

    CMDFIFO_CHECKROOM(cmdfifo, (PH4_SIZE + 3 ) * count);

    for (i = 0, pTmpRects = pRects; i < count; i++, pTmpRects++)
    {
      SETPH(cmdfifo, CMDFIFO_BUILD_PK4(R0|R1|R3, clipLeftRight, 0x0));
      SETPD(cmdfifo, ghw->clipLeftRight,
        ((int)(pTmpRects->left) << 16) | ((int)pTmpRects->right));
      SETPD(cmdfifo, ghw->clipBottomTop,
        ((int)(pTmpRects->top) << 16) | ((int)pTmpRects->bottom));

      SETPD(cmdfifo, ghw->fastfillCMD, TRUE);
    } // endfor: clear rectangles

  } // endif: (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL)

  // restore colBuffAddr and prepares it for render target clear

  ASSERTDD(0 != pRc->DDSHndl, " NULL render target");
  ASSERTDD(0 != TXTRHNDL_PTR(pRc->DDSHndl)->surfData, " NULL texture desc");

//#ifdef WINNT
//  colBufferAddr = GET_HW_ADDR(pRc->lpDDS);
//#else
  colBufferAddr = GET_HW_OFFSET(pRc->DDSHndl);
//#endif

  // include room for dwFillColor here

  CMDFIFO_CHECKROOM(cmdfifo, (PH1_SIZE * 3) + 4);
  SETPH(cmdfifo, CMDFIFO_BUILD_PK1(2, 1, colBufferAddr, 0xf));

#if ENABLE_TILED_HEAP
  if (IS_TILED(colBufferAddr))
  {
    DWORD colBufferStride;

    colBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;
    SETPD(cmdfifo, ghw->colBufferAddr, (colBufferAddr & 0x7FFFFFFFL));
    SETPD(cmdfifo, ghw->colBufferStride, colBufferStride);
  }
  else
#endif
  {
    SETPD(cmdfifo, ghw->colBufferAddr, colBufferAddr);
    SETPD(cmdfifo, ghw->colBufferStride, TXTRHNDL_PTR(pRc->DDSHndl)->lPitch);
  }

  // clear render target

  if (D3DCLEAR_TARGET & dwFlags)
  {
    D3DPRINT(NORMAL_DBG_LEVEL,
      "clearing render target: %8lXh with rgb888 %8lXh", pRc->DDSHndl, dwFillColor);

  	SETPH(cmdfifo, CMDFIFO_BUILD_PK1(1, 0, fbzMode, 0xF));
 	SETPD(cmdfifo, ghw->fbzMode, SST_RGBWRMASK); // enable color buffer clear

    SETPH(cmdfifo, CMDFIFO_BUILD_PK1(1, 0, c1, 0xF));
    SETPD(cmdfifo, ghw->c1, dwFillColor);

    // loop over rectangles
    CMDFIFO_CHECKROOM(cmdfifo, (PH4_SIZE + 3 ) * count);

    for (i = 0; i < count; i++, pRects++)
    {
      SETPH(cmdfifo, CMDFIFO_BUILD_PK4(R0|R1|R3, clipLeftRight, 0x0));
      SETPD(cmdfifo, ghw->clipLeftRight,
        ((int)(pRects->left) << 16) | ((int)pRects->right));
      SETPD(cmdfifo, ghw->clipBottomTop,
        ((int)(pRects->top) << 16) | ((int)pRects->bottom));

      SETPD(cmdfifo, ghw->fastfillCMD, TRUE);
    } // endfor: clear rectangles

  } // endif: D3DCLEAR_TARGET

  // Restore the clipping registers for current surface

  CMDFIFO_CHECKROOM(cmdfifo, (PH1_SIZE * 2) + 3);
  SETPH(cmdfifo, CMDFIFO_BUILD_PK1(2, 1, clipLeftRight, 0xf));
#ifdef NEW_CLIP_FOR_GB
  SETPD(cmdfifo, ghw->clipLeftRight, pRc->sst.clipLeftRight);
  SETPD(cmdfifo, ghw->clipBottomTop, pRc->sst.clipBottomTop);
  
  // _D3(last) shadows the register values on the card
  _D3(last).clipLeftRight = pRc->sst.clipLeftRight;
  _D3(last).clipBottomTop = pRc->sst.clipBottomTop;
#else
  SETPD(cmdfifo, ghw->clipLeftRight, (DWORD)pRc->lpDDS->lpGbl->wWidth);
  SETPD(cmdfifo, ghw->clipBottomTop, (DWORD)pRc->lpDDS->lpGbl->wHeight);
#endif

  // Restore zaColor because this is used for ZBias

  if (dwFlags & D3DCLEAR_ZBUFFER)
  {
    SETPH(cmdfifo, CMDFIFO_BUILD_PK1(1, 0, zaColor, 0xF));
    SETPD(cmdfifo, ghw->zaColor, pRc->sst.zaColor);
  }

  _D3(last).changed = TRUE;
  CMDFIFO_EPILOG(cmdfifo);

  return D3D_OK;
} // FastFill_2pass

/*-------------------------------------------------------------------
Function Name:  FastFill

Description:    Clear buffers using Fast Fill command.

Return:         D3D_OK
-------------------------------------------------------------------*/

HRESULT
FastFill (RC *pRc, D3DHAL_DP2CLEAR *pClear, DWORD count)
{
  SETUP_PPDEV(pRc)
  DWORD   i;
  DWORD   colBufferAddr, auxBufferAddr;
  DWORD   fbzMode;
#if ENABLE_TILED_HEAP
  DWORD   isTiled;
#endif
  DWORD   dwFlags =         pClear->dwFlags;        // Clear flags
  DWORD   dwFillColor =     pClear->dwFillColor;    // Render target fill color
  float   dvFillDepth =     pClear->dvFillDepth;    // Z buffer fill value
  DWORD   dwFillStencil =   pClear->dwFillStencil;  // Stencil buffer fill value
  RECT    *pRects =         (LPRECT)((LPBYTE)pClear + sizeof(D3DHAL_DP2CLEAR) - sizeof(RECT));

#if defined(TnL_HAL)

// [Randy Spurlock] Need to add Pure Device Support Here! (Zero and User Rects)

#endif

  CMDFIFO_PROLOG(cmdfifo);

  CMDFIFO_CHECKROOM(cmdfifo, (PH1_SIZE * 9) + 13);

  fbzMode = 0;

#if ENABLE_TILED_HEAP
  isTiled = 0x0;
#endif
  // clear render target

  ASSERTDD(0 != pRc->DDSHndl, " NULL render target");

  if (IS_NAPALM)
  {
    // set target surface pixel depth.
    // this macro modifies the render mode bits in the renderMode register.
    SETRENDERMODEPIXELDEPTH(pRc, pRc->DDSHndl);

    // Make sure dither rotation is disabled. Broken for fastfills.
    SETPH( cmdfifo, CMDFIFO_BUILD_PK1( 1, 0, renderMode, 0xf ) );
    SETPD( cmdfifo, ghw->renderMode, pRc->sst.renderMode & ~SST_RM_DITHER_ROTATION );
  }

  if ( (D3DCLEAR_TARGET & dwFlags) && (0 != TXTRHNDL_PTR(pRc->DDSHndl)->surfData) )
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "clearing render target: %8lXh with rgb888 %8lXh", pRc->DDSHndl, dwFillColor);

    fbzMode |= SST_RGBWRMASK; // enable color buffer clear

//#ifdef WINNT
//    colBufferAddr = GET_HW_ADDR(pRc->lpDDS);
//#else
    colBufferAddr = GET_HW_OFFSET(pRc->DDSHndl);
//#endif

    SETPH(cmdfifo, CMDFIFO_BUILD_PK1(2, 1, colBufferAddr, 0xf));

#if ENABLE_TILED_HEAP
    isTiled |= IS_TILED(colBufferAddr);

    if (IS_TILED(colBufferAddr))
    {
      DWORD colBufferStride;

      colBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;
      SETPD(cmdfifo, ghw->colBufferAddr, (colBufferAddr & 0x7FFFFFFFL));
      SETPD(cmdfifo, ghw->colBufferStride, colBufferStride);
    }
    else
#endif
    {
      SETPD(cmdfifo, ghw->colBufferAddr, colBufferAddr);
      SETPD(cmdfifo, ghw->colBufferStride, TXTRHNDL_PTR(pRc->DDSHndl)->lPitch);
    }

    SETPH(cmdfifo, CMDFIFO_BUILD_PK1(1, 0, c1, 0xF));
    SETPD(cmdfifo, ghw->c1, dwFillColor);

    if (IS_NAPALM)
    {
#ifdef SLI_AA
      // S E C O N D A R Y   C O L O R   B U F F E R
      // If antialiasing is enabled, setup the secondary color buffer
      if (_DD(ddAAModeEnabled))
      {
//#ifdef WINNT
//        ULONG AABufferAddr = GET_AAHW_ADDR(pRc->lpDDS);
//#else
        ULONG AABufferAddr = GET_AAHW_OFFSET(pRc->DDSHndl);
//#endif

        if (AABufferAddr) // Make sure that the AA buffers are allocated. 
        {
          SETPH( cmdfifo, CMDFIFO_BUILD_PK1( 2, 1, colBufferAddr, 0xf ) );

  #if ENABLE_TILED_HEAP
          if(IS_TILED(AABufferAddr))
          {
            DWORD colBufferStride;

            colBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;

            SETPD( cmdfifo, ghw0->colBufferAddr,   ((AABufferAddr & 0x7FFFFFFFL) | SST_BUFFER_BASE_SELECT));
            SETPD( cmdfifo, ghw0->colBufferStride, colBufferStride);
          }
          else
  #endif
          {
            SETPD( cmdfifo, ghw0->colBufferAddr,   (AABufferAddr | SST_BUFFER_BASE_SELECT));
            SETPD( cmdfifo, ghw0->colBufferStride, TXTRHNDL_PTR(pRc->DDSHndl)->lPitch);
          } // Tiled?
      	} // AABufferAddr
      }
#endif // SLI_AA

      //This flag only needs to be set if TMU0/1 color1 reg changes
      UPDATE_HW_STATE( SC_TEXTUREFACTOR );
    }
  } // endif: D3DCLEAR_TARGET

  // zbuffer & stencil buffer clear

  if ( ((D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL) & dwFlags) &&
       (0 != pRc->DDSZHndl) && (0 != TXTRHNDL_PTR(pRc->DDSZHndl)->surfData)
     )
  {
#ifdef WINNT
    D3DPRINT(NORMAL_DBG_LEVEL, "clearing zbuffer: %8lXh with %8lXh", pRc->DDSZHndl,
      (DWORD)(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));
#else
    D3DPRINT(NORMAL_DBG_LEVEL, "clearing zbuffer: %8lXh with %8lXh", pRc->DDSZHndl,
      (DWORD)(ddftol(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask)));
#endif

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

//#ifdef WINNT
//    auxBufferAddr = GET_HW_ADDR(pRc->lpDDSZ);
//#else
    auxBufferAddr = GET_HW_OFFSET(pRc->DDSZHndl);
//#endif
    SETPH(cmdfifo, CMDFIFO_BUILD_PK1(2, 1, auxBufferAddr, 0xf));

#if ENABLE_TILED_HEAP
    isTiled |= IS_TILED(auxBufferAddr);

    if (IS_TILED(auxBufferAddr))
    {
      DWORD auxBufferStride;

      auxBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;
      SETPD(cmdfifo, ghw->auxBufferAddr, (auxBufferAddr & 0x7FFFFFFFL));
      SETPD(cmdfifo, ghw->auxBufferStride, auxBufferStride);
    }
    else
#endif
    {
      SETPD(cmdfifo, ghw->auxBufferAddr, auxBufferAddr);
      SETPD(cmdfifo, ghw->auxBufferStride, TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch);
    }

#ifdef SLI_AA
    // S E C O N D A R Y   Z   B U F F E R
    // If antialiasing is enabled, setup the secondary Z buffer
    if (IS_NAPALM && _DD(ddAAModeEnabled))
    {
//#ifdef WINNT
//      ULONG AABufferAddr = GET_AAHW_ADDR(pRc->lpDDSZ);
//#else
      ULONG AABufferAddr = GET_AAHW_OFFSET(pRc->DDSZHndl);
//#endif
#ifdef DEBUG
      ULONG lastAABufferAddr = _D3(last).auxAABufferAddr; // to help with debug
#endif

      if (0 != AABufferAddr)
      {
        SETPH( cmdfifo, CMDFIFO_BUILD_PK1( 2, 1, auxBufferAddr, 0xf ) );

    #if ENABLE_TILED_HEAP
        if(IS_TILED(AABufferAddr))
        {
          DWORD auxBufferStride;

          auxBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;

          SETPD( cmdfifo, ghw->auxBufferAddr,   ((AABufferAddr & 0x7fffffffL) | SST_BUFFER_BASE_SELECT));
          SETPD( cmdfifo, ghw->auxBufferStride, auxBufferStride);
        }
        else
    #endif
        {
          SETPD( cmdfifo, ghw->auxBufferAddr,   (AABufferAddr | SST_BUFFER_BASE_SELECT));
        SETPD( cmdfifo, ghw->auxBufferStride, TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch);
        } // Tiled?
      }
    } // AA enabled?
#endif // SLI_AA
  } // endif: (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL)

  // Set Z fill value.

  if (dwFlags & D3DCLEAR_ZBUFFER)
  {
    fbzMode |= SST_ZAWRMASK; // Enable zbuffer clear

    SETPH( cmdfifo, CMDFIFO_BUILD_PK1( 1, 0, zaColor, 0xF ) );

    // dvFillDepth is 0.0 - 1.0.
    // dwZBitMask should be 0x0000FFFF for 16bpp and 0x00FFFFFF for 32bpp.
#ifdef WINNT
    SETPD(cmdfifo, ghw->zaColor, (DWORD)(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));
#else
    SETPD(cmdfifo, ghw->zaColor, (float2int(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask)));
#endif
  }

  // Set stencil fill value.
  if (IS_NAPALM && (dwFlags & D3DCLEAR_STENCIL))
  {
    // Setup stencilMode to clear stencil planes to specified value.
    SETPH(cmdfifo, CMDFIFO_BUILD_PK1( 1, 0, stencilMode, 0xF ));
    SETPD(cmdfifo, ghw->stencilMode, SST_STENCIL_MODE_CLEAR | (dwFillStencil & SST_STENCIL_REF));
  }
  else
  {
    // Disable stenciling, to avoid stencil values being cleared.
    SETPH(cmdfifo, CMDFIFO_BUILD_PK1( 1, 0, stencilMode, 0xF ));
    SETPD(cmdfifo, ghw->stencilMode, SST_STENCIL_MODE_DISABLE);
  }

  SETPH(cmdfifo, CMDFIFO_BUILD_PK1(1, 0, fbzMode, 0xF));
  SETPD(cmdfifo, ghw->fbzMode, fbzMode);

  // loop over rectangles

  for (i = 0; i < count; i++, pRects++)
  {
#ifdef SLI_AA
      // If SLI & Tiled Buffer & Full Screen then do in non-SLI mode
      if (_DD(ddSLIModeEnabled) && (isTiled) &&
          (0x0 == pRects->top) && 
#ifdef WINNT
          (ppdev->cyScreen == pRects->bottom))
#else
          (ppdev->bi.biHeight == pRects->bottom))
#endif
         {
         DWORD AAShift;
         DWORD rMask;
         DWORD sMask;
         DWORD cMask;   
         DWORD sliCtrl;
         DWORD sliCtrlBase;
         DWORD i;
         DWORD log2NChips;

         // Determine Size for Disable SLI, FastFill, Enable SLI, and ChipMask
         CMDFIFO_CHECKROOM( cmdfifo, ((PH1_SIZE + 1) * 1) + (PH4_SIZE + 3)
         + ((PH1_SIZE + 1) * 2 * _FF(dwNumUnits)) + ((PH1_SIZE + 1) * 1));

         // Disable 3d SLI
         SETPH( cmdfifo, CMDFIFO_BUILD_PK1( 1, 0, sliCtrl, 0 ) );
         SETPD( cmdfifo, ghw->sliCtrl, 0x0);

         // Do FastFill
         SETPH(cmdfifo, CMDFIFO_BUILD_PK4(R0|R1|R3, clipLeftRight, 0x0));
         SETPD(cmdfifo, ghw->clipLeftRight, 
         ((int)(pRects->left) << 16) | ((int)pRects->right));
         SETPD(cmdfifo, ghw->clipBottomTop,
         ((int)(pRects->top) << 16) | (int)(_DD(dwMaxHeight)));
         SETPD(cmdfifo, ghw->fastfillCMD, TRUE);

         // Enable 3d SLI
         // Special Case when AA is enabled and number of chips != Sli Ness
         log2NChips = _DD(dwlog2NumChips); 
         if ((_DD(ddSLINumberWays) != _FF(dwNumUnits)))
            AAShift = 1;
         else
            AAShift = 0;

         rMask = ((_FF(dwNumUnits) - 1) >> AAShift) << _DD(dwlog2BandHeight);
         sMask = _DD(ddSLINumberScanlines) - 1;
         sliCtrlBase = (rMask << SST_SLI_CONTROL_RENDER_MASK_SHIFT) | (log2NChips << SST_SLI_CONTROL_LOG2_CHIP_COUNT_SHIFT) | (sMask << SST_SLI_CONTROL_SCAN_MASK_SHIFT) | SST_SLI_CONTROL_SLI_ENABLE;
         for (i=0; i<_FF(dwNumUnits); i++)
            {
            cMask = ((i >> AAShift) << _DD(dwlog2BandHeight)) << SST_SLI_CONTROL_COMPARE_MASK_SHIFT;
            sliCtrl = sliCtrlBase | cMask; 
            SETPH( cmdfifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
            SETPD( cmdfifo, ghw->chipMask, BIT(i));
            SETPH( cmdfifo, CMDFIFO_BUILD_PK1( 1, 0, sliCtrl, 0 ) );
            SETPD( cmdfifo, ghw->sliCtrl, sliCtrl);
            }
         SETPH( cmdfifo, CMDFIFO_BUILD_PK1( 1, 0, chipMask, 0 ) );
         SETPD( cmdfifo, ghw->chipMask, pRc->sst.chipMask);
         }
      else
#endif
         {
         CMDFIFO_CHECKROOM(cmdfifo, (PH4_SIZE + 3));
         SETPH(cmdfifo, CMDFIFO_BUILD_PK4(R0|R1|R3, clipLeftRight, 0x0));
         SETPD(cmdfifo, ghw->clipLeftRight, 
         ((int)(pRects->left) << 16) | ((int)pRects->right));
         SETPD(cmdfifo, ghw->clipBottomTop,
         ((int)(pRects->top) << 16) | ((int)pRects->bottom));
         SETPD(cmdfifo, ghw->fastfillCMD, TRUE);
         }

  } // endfor: clear rectangles

  // Restore the clipping registers for current surface

  CMDFIFO_CHECKROOM(cmdfifo, (PH1_SIZE * 2) + 3);
  SETPH(cmdfifo, CMDFIFO_BUILD_PK1(2, 1, clipLeftRight, 0xf));
#ifdef NEW_CLIP_FOR_GB
  SETPD(cmdfifo, ghw->clipLeftRight, pRc->sst.clipLeftRight);
  SETPD(cmdfifo, ghw->clipBottomTop, pRc->sst.clipBottomTop);
  
  // _D3(last) shadows the register values on the card
  _D3(last).clipLeftRight = pRc->sst.clipLeftRight; 
  _D3(last).clipBottomTop = pRc->sst.clipBottomTop;
#else
  SETPD(cmdfifo, ghw->clipLeftRight, (DWORD)pRc->lpDDS->lpGbl->wWidth);
  SETPD(cmdfifo, ghw->clipBottomTop, (DWORD)pRc->lpDDS->lpGbl->wHeight);
#endif

  // Restore zaColor because this is used for ZBias

  if (dwFlags & D3DCLEAR_ZBUFFER)
  {
    SETPH(cmdfifo, CMDFIFO_BUILD_PK1(1, 0, zaColor, 0xF));
    SETPD(cmdfifo, ghw->zaColor, pRc->sst.zaColor);
  }

  _D3(last).changed = TRUE;
  CMDFIFO_EPILOG(cmdfifo);

  return D3D_OK;

} // FastFill

/*-------------------------------------------------------------------
Function Name:  PaletteSet

Description:    Attaches a palette handle to a texture in the given
                context.  The texture is the one associated to the
                given surface handle.

Return:
-------------------------------------------------------------------*/

HRESULT
PaletteSet(RC *pRc, 
           DWORD dwSurfaceHandle, 
           DWORD dwPaletteHandle, 
           DWORD dwPaletteFlags)
{
  SETUP_PPDEV(pRc)
  HNDLLIST                    *pHndlList = pRc->pHndlList;
  PALHNDL                     *pPalHndl;
  DWORD                       dwTxtrHndl;
  TXTRDESC                    *pTxtrDesc;
  int                         txtrID;


  D3DPRINT(10,">> PaletteSet");

  ASSERTDD(0 != dwSurfaceHandle, "dwSurfaceHandle==0 in D3DDP2OP_SETPALETTE");

  D3DPRINT(NORMAL_DBG_LEVEL,"SETPALETTE PalHndl=%ld to SurfHndl=%ld, PaletteFlags=%08lXh",
           dwPaletteHandle, dwSurfaceHandle, dwPaletteFlags);

  dwTxtrHndl = dwSurfaceHandle;
  if (0 == dwTxtrHndl)
  {
    // invalid dwSurfaceHandle, skip it
    D3DPRINT(0,"__PaletteSet:NULL==pTexture Palette=%ld Surface=%ld",
             dwPaletteHandle, dwSurfaceHandle);
    // fix for PRS 12730 from Johnny Cochrane
    // the runtime attaches palettes to system memory surfaces but passes us a texture handle
    // of zero.  By returning an error here, we terminate DP2 command processing, but returning
    // D3D_OK, means we just ignore this palette and continue processing the rest of the
    // command stream.
    return D3D_OK;
  }
  // if the TXTRHNDL_PTR is NULL, then it's probably a handle for a mipmap level
  // smaller than the top level mipmap, we'll just ignore it for now
  if (NULL == pHndlList->ppTxtrHndlList[dwTxtrHndl])
  {
    D3DPRINT(0, "PaletteSet: NULL == pHndlList->ppTxtrHndlList[dwTxtrHndl]");
    return D3D_OK;
  }

  // Fix for Page Fault occuring when running DCT 400 D3D HWTexMan test app under DX7
  if (!TXTRHNDL_INRANGE(dwTxtrHndl))
		return D3D_OK;

  // save palette handle in TXTRHNDL and TXTRDESC structs
  TXTRHNDL_PTR(dwTxtrHndl)->dwPaletteHandle = dwPaletteHandle;
  txtrID = TXTRHNDL_PTR(dwTxtrHndl)->txtrID;
  // only store palette handle in TXTRDESC if txtrID is nonzero
  if ((txtrID >= 1) && (txtrID < (int) MAXTEXTURECOUNT))
  {
    pTxtrDesc = TXTRDESC_PTR(txtrID);
    pTxtrDesc->dwPaletteHandle = dwPaletteHandle;

    D3DPRINT(17, "PaletteSet: STORING paletteHandle.");
    D3DPRINT(17, "   pTxtrDesc[%08x]->dwPaletteHandle = %d", 
                     pTxtrDesc, pTxtrDesc->dwPaletteHandle);
  }
#ifdef DEBUG
  else
  {
    if (0 == txtrID)
      D3DPRINT(17, "PaletteSet: NOT storing paletteHandle!! 0 == txtrID.");
    else
      D3DPRINT(17, "PaletteSet: NOT storing paletteHandle!! txtrID bad [%08x]", txtrID);
  }
#endif

#ifdef WINNT
  if (pRc->texture == dwTxtrHndl)
    _D3(flags) |= PALETTECHANGED;
#else
  // This fixes a palettized texture bug. Bob S.
  // The sea floor colors in DX7/Dolphin were incorrect - yellow, red, orange.
  // Run Billboard, then run Dolphin to get the incorrect results.
  _D3(flags) |= PALETTECHANGED;
#endif

  if (0 == dwPaletteHandle)
  {
    //palette association is OFF
    D3DPRINT(0, "PaletteSet: Incoming dwPaletteHandle == 0 !!!!");
    return D3D_OK;
  }

  // if we don't have a ppPalHndlList or it's not big enough then
  // resize the current one
  if (NULL == pHndlList->ppPalHndlList ||
      dwPaletteHandle > (DWORD)pHndlList->ppPalHndlList[0])
  {
    // we need to account for using index zero as a count of how many elements are in
    // the ppPalHndlList array, so add one to dwPaletteHandle
    DWORD   newsize = (((dwPaletteHandle + 1) + (LISTGROWSIZE - 1)) / LISTGROWSIZE) * LISTGROWSIZE;
    PALHNDL **newlist = (PALHNDL **)D3DMALLOCZ(sizeof(PALHNDL *)*newsize, 0);
    D3DPRINT(NORMAL_DBG_LEVEL,"Growing pDDLcl=%X's PaletteList[%X] size to %08lXh",
             pRc->pDDLcl, newlist, newsize);

    if (NULL == newlist)
    {
      D3DPRINT(0,"D3DDP2OP_SETPALETTE Out of memory failed to grow PALHNDL list");
      return DDERR_OUTOFMEMORY;
    }

    // if we had a valid PALHNDL list,
    // copy it to the newlist and free the memory allocated before
    if (NULL != pHndlList->ppPalHndlList)
    {
      // copy counter in element zero plus all PALHNDL *'s to new array
      memcpy(newlist, pHndlList->ppPalHndlList,
             ((DWORD)pHndlList->ppPalHndlList[0] + 1) * sizeof(PPALHNDL));
#ifdef MEMCHECK
      {
        DWORD i;

        for (i = 1; i < (DWORD)pHndlList->ppPalHndlList[0]; i++)
        {
          if (NULL != pHndlList->ppPalHndlList[i])
            UPDATE_BLOCK_DATA(pHndlList->ppPalHndlList[i], &newlist[i]);
        }
      }
#endif
      D3DFREE(pHndlList->ppPalHndlList);
      D3DPRINT(NORMAL_DBG_LEVEL,"Freeing pDDLcl=%X's old PaletteList[%X]",
               pRc->pDDLcl, pHndlList->ppPalHndlList);
    }

    pHndlList->ppPalHndlList = newlist;
    UPDATE_BLOCK_DATA(newlist, &pHndlList->ppPalHndlList);
    //store size in ppPalHndlList[0]
    // since we're using element zero to hold the array size
    // we only have newsize-1 entries to store PALHNDL * elements in
    (DWORD)pHndlList->ppPalHndlList[0] = newsize - 1;
  }

  // If we don't have a palette hanging from this palette list
  // element we have to create one. The actual palette data will
  // come down in the D3DDP2OP_UPDATEPALETTE command token.
  if (NULL == pHndlList->ppPalHndlList[dwPaletteHandle])
  {
    // now allocate a PALHNDL
    pPalHndl = (PALHNDL *)D3DMALLOCZ(sizeof(PALHNDL), 0);

    if (NULL == pPalHndl)
    {
      D3DPRINT(0, "D3DDP2OP_SETPALETTE out of memory, failed to alloc PALHNDL");
      return DDERR_OUTOFMEMORY;
    }

    // store the PALHNDL in the pHndlList array in the dwPaletteHandle element
    pHndlList->ppPalHndlList[dwPaletteHandle] = pPalHndl;
    UPDATE_BLOCK_DATA(pPalHndl, &pHndlList->ppPalHndlList[dwPaletteHandle]);
  }
  else
    pPalHndl = pHndlList->ppPalHndlList[dwPaletteHandle];

  // driver may store this dwFlags to decide whether
  // ALPHA exists in Palette
  pPalHndl->dwFlags = dwPaletteFlags;

  D3DPRINT(NORMAL_DBG_LEVEL,"Set pDDLcl=%8lXh PaletteHandle=%ld pPalHndl = %8lXh",
           pRc->pDDLcl, dwPaletteHandle, pPalHndl);

  D3DPRINT(10,"<< PaletteSet");

  return D3D_OK;
} // PaletteSet

/*-------------------------------------------------------------------
Function Name:  PaletteUpdate

Description:    Updates the entries of a palette attached to a texture
                in the given context

Return:
-------------------------------------------------------------------*/

HRESULT
PaletteUpdate(RC    *pRc,
              DWORD dwPaletteHandle,
              WORD  wStartIndex,
              WORD  wNumEntries,
              BYTE  *pPaletteData)
{
  SETUP_PPDEV(pRc)
  HNDLLIST  *pHndlList = pRc->pHndlList;
  PALHNDL   *pPalHndl;


  D3DPRINT(10,">> PaletteUpdate");

  D3DPRINT(NORMAL_DBG_LEVEL,"UPDATEPALETTE dwPaletteHandle %ld, StartIndex %ld, NumEntries %ld",
           dwPaletteHandle,
           wStartIndex,
           wNumEntries);

  ASSERTDD(NULL != pHndlList, "NULL pHndlList");
  ASSERTDD(NULL != pHndlList->ppPalHndlList, "NULL ppPalList");

#if 1
  // if we don't have a ppPalHndlList or it's not big enough then
  // resize the current one
  if (NULL == pHndlList->ppPalHndlList ||
      dwPaletteHandle > (DWORD)pHndlList->ppPalHndlList[0])
  {
    // we need to account for using index zero as a count of how many elements are in
    // the ppPalHndlList array, so add one to dwPaletteHandle
    DWORD   newsize = (((dwPaletteHandle + 1) + (LISTGROWSIZE - 1)) / LISTGROWSIZE) * LISTGROWSIZE;
    PALHNDL **newlist = (PALHNDL **)D3DMALLOCZ(sizeof(PALHNDL *)*newsize, 0);
    D3DPRINT(NORMAL_DBG_LEVEL,"Growing pDDLcl=%X's PaletteList[%X] size to %08lXh",
             pRc->pDDLcl, newlist, newsize);

    if (NULL == newlist)
    {
      D3DPRINT(0,"D3DDP2OP_SETPALETTE Out of memory failed to grow PALHNDL list");
      return DDERR_OUTOFMEMORY;
    }

    // if we had a valid PALHNDL list,
    // copy it to the newlist and free the memory allocated before
    if (NULL != pHndlList->ppPalHndlList)
    {
      // copy counter in element zero plus all PALHNDL *'s to new array
      memcpy(newlist, pHndlList->ppPalHndlList,
             ((DWORD)pHndlList->ppPalHndlList[0] + 1) * sizeof(PPALHNDL));
#ifdef MEMCHECK
      {
        DWORD i;

        for (i = 1; i < (DWORD)pHndlList->ppPalHndlList[0]; i++)
        {
          if (NULL != pHndlList->ppPalHndlList[i])
            UPDATE_BLOCK_DATA(pHndlList->ppPalHndlList[i], &newlist[i]);
        }
      }
#endif
      D3DFREE(pHndlList->ppPalHndlList);
      D3DPRINT(NORMAL_DBG_LEVEL,"Freeing pDDLcl=%X's old PaletteList[%X]",
               pRc->pDDLcl, pHndlList->ppPalHndlList);
    }

    pHndlList->ppPalHndlList = newlist;
    UPDATE_BLOCK_DATA(newlist, &pHndlList->ppPalHndlList);
    //store size in ppPalHndlList[0]
    // since we're using element zero to hold the array size
    // we only have newsize-1 entries to store PALHNDL * elements in
    (DWORD)pHndlList->ppPalHndlList[0] = newsize - 1;
  }

  // If we don't have a palette hanging from this palette list
  // element we have to create one. The actual palette data will
  // come down in the D3DDP2OP_UPDATEPALETTE command token.
  if (NULL == pHndlList->ppPalHndlList[dwPaletteHandle])
  {
    // now allocate a PALHNDL
    pPalHndl = (PALHNDL *)D3DMALLOCZ(sizeof(PALHNDL), 0);

    if (NULL == pPalHndl)
    {
      D3DPRINT(0, "D3DDP2OP_SETPALETTE out of memory, failed to alloc PALHNDL");
      return DDERR_OUTOFMEMORY;
    }

    // store the PALHNDL in the pHndlList array in the dwPaletteHandle element
    pHndlList->ppPalHndlList[dwPaletteHandle] = pPalHndl;
    UPDATE_BLOCK_DATA(pPalHndl, &pHndlList->ppPalHndlList[dwPaletteHandle]);

    // If we just had to create a PALHNDL, the we really ought to 
    // initialize its dwFlags to something, but who knows what, so lets 
    // just use Zero for now....tl
    pPalHndl->dwFlags = 0;
  }
  else
    pPalHndl = pHndlList->ppPalHndlList[dwPaletteHandle];
#else
  pPalHndl = pHndlList->ppPalHndlList[dwPaletteHandle];
#endif

  if (NULL != pPalHndl)
  {
    ASSERTDD(256 >= wStartIndex + wNumEntries,
             "wStartIndex+wNumEntries>256 in D3DDP2OP_UPDATEPALETTE");

    // Copy the palette & associated data
    pPalHndl->wStartIndex = wStartIndex;
    pPalHndl->wNumEntries = wNumEntries;

    memcpy((LPVOID)&pPalHndl->ColorTable[wStartIndex],
           (LPVOID)pPaletteData,
           (DWORD)wNumEntries*sizeof(PALETTEENTRY));

#ifdef DEBUG
    {
        int i;
        D3DPRINT(17, "PaletteUpdate - The Palette");
        for (i = wStartIndex; i < (256/4); i+=4)
        {
            D3DPRINT(18, "%08x %08x %08x %08x",
                        pPalHndl->ColorTable[i+0],
                        pPalHndl->ColorTable[i+1],
                        pPalHndl->ColorTable[i+2],
                        pPalHndl->ColorTable[i+3]
                    );
        }
    }
#endif
#if defined(WINNT) && DBG
    {
      int i;
      D3DPRINT(D3DDBGLVL+1, "PaletteUpdate - palette data");
      for (i = wStartIndex; i < wStartIndex+wNumEntries; i+=8)
      {
        D3DPRINT(D3DDBGLVL+1, "  entries %3ld-%3ld  %08lXh %08lXh %08lXh %08lXh  %08lXh %08lXh %08lXh %08lXh",
                 i, i+7,
                 pPalHndl->ColorTable[i+0],
                 pPalHndl->ColorTable[i+1],
                 pPalHndl->ColorTable[i+2],
                 pPalHndl->ColorTable[i+3],
                 pPalHndl->ColorTable[i+4],
                 pPalHndl->ColorTable[i+5],
                 pPalHndl->ColorTable[i+6],
                 pPalHndl->ColorTable[i+7]);
      }
    }
#endif

    // If we are currently texturing and the texture is using the
    // palette we just updated, dirty the texture flag so that
    // it set up with the right (updated) palette
    if (0 != pRc->texture)
    {
      D3DPRINT(17, "PaletteUpdate - pRc->texture %d != 0", pRc->texture);
      // if the TXTRHNDL_PTR is NULL, then it's probably a handle for a mipmap level
      // smaller than the top level mipmap, we'll just ignore it for now
      if (NULL != pHndlList->ppTxtrHndlList[pRc->texture])
      {
        if (TXTRHNDL_PTR(pRc->texture)->dwPaletteHandle == dwPaletteHandle)
        {
          _D3(flags) |= PALETTECHANGED;
          D3DPRINT(17, "PaletteUpdate, after load: PALETTECHANGED flag set");
        }
#ifdef DEBUG
        else
        {
          D3DPRINT(17, "PaletteUpdate, after load: PALETTECHANGED flag NOT set");
        }
#endif
      }
    }
  }
  else
  {
    D3DPRINT(10,"<< PaletteUpdate");
    D3DPRINT(17, "PaletteUpdate: palHndl == 0, nothing done!!!");

    return DDERR_INVALIDPARAMS;
  }

//jcochrane - PRS 12591: set PALETTECHANGED as we have just updated the current palette
  if( _D3(currentPalette)	== pPalHndl) {
    D3DPRINT(17, "PaletteUpdate: just updated current palette, setting PALETTECHANGED");
   	_D3(flags) |= PALETTECHANGED;
  }

  D3DPRINT(10,"<< PaletteUpdate");

  return D3D_OK;
} // PaletteUpdate

/*-------------------------------------------------------------------
Function Name:  BeginStateBlock

Description:    create a new state block identified by dwHandle and
                enable recording states

Return:
-------------------------------------------------------------------*/

HRESULT
BeginStateBlock(RC *pRc, DWORD dwHandle)
{
#ifndef WINNT
  SETUP_PPDEV(pRc)
#endif
  STATEBLOCK  *pSB;


  D3DPRINT(10,">> BeginStateBlock dwHandle=%ld",dwHandle);

  // make sure we're not already in recording mode
  if (TRUE == pRc->bSBRecMode)
  {
    D3DPRINT(0, "Currently recording state block");
    D3DPRINT(10,"<< BeginStateBlock");

#if (DIRECT3D_VERSION >= 0x0800) || (DX >= 8)
	return DDERR_GENERIC;
#else
    return D3DERR_INBEGINSTATEBLOCK;
#endif
  }

  // create a new state block
  pSB = D3DMALLOCZ(sizeof(STATEBLOCK), 0);
  if (NULL == pSB)
  {
    D3DPRINT(0, "Out of memory for additional state block");
    D3DPRINT(10,"<< BeginStateBlock");
    return DDERR_OUTOFMEMORY;
  }

  // save handle to current state block
  pSB->dwHandle = dwHandle;
  pSB->bCompressed = FALSE;

  // set the current state block in the RC
  pRc->pCurrSB = pSB;
  UPDATE_BLOCK_DATA(pSB, &pRc->pCurrSB);

  // start recording mode
  pRc->bSBRecMode = TRUE;

  D3DPRINT(NORMAL_DBG_LEVEL, "  Starting record mode, SBHandle=%ld, pCurrSB=%lXh for pRc=%lXh",
           dwHandle, pSB, pRc);

  D3DPRINT(10,"<< BeginStateBlock");
  return D3D_OK;
}

/*-------------------------------------------------------------------
Function Name:  EndStateBlock

Description:    disable recording states, revert to executing them
                compress recorded state block and store in context

Return:
-------------------------------------------------------------------*/

HRESULT
EndStateBlock(RC *pRc)
{
  DWORD       dwHandle;
  STATEBLOCK  *pSB;
  HRESULT     hr;


  D3DPRINT(10,">> EndStateBlock");

  // make sure we're in recording mode
  if (FALSE == pRc->bSBRecMode)
  {
    D3DPRINT(0, "Currently NOT recording state block");
    D3DPRINT(10,"<< EndStateBlock");
#if (DIRECT3D_VERSION >= 0x0800) || (DX >= 8)
	return DDERR_GENERIC;
#else
    return D3DERR_NOTINBEGINSTATEBLOCK;
#endif
  }

  hr = D3D_OK;

  if (pRc->pCurrSB)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "  Ending record mode, SBHandle=%ld, pCurrSB=%lXh for pRc=%lXh",
             pRc->pCurrSB->dwHandle, pRc->pCurrSB, pRc);

    dwHandle = pRc->pCurrSB->dwHandle;

    // compress state block
    // Note: after being compressed, the uncompressed version is freed
    // so don't access pRc->pCurrSB after this, use pSB instead
    UPDATE_BLOCK_DATA(pRc->pCurrSB, 0);
    pSB = CompressStateBlock(pRc, pRc->pCurrSB);
    D3DPRINT(NORMAL_DBG_LEVEL, "  CompressStateBlock returned pSB=%lXh", pSB);

    // add state block to ppSBTable
    hr = AddStateBlockToTable(pRc, dwHandle, pSB);
  }

  // clear the current state block in the RC
  pRc->pCurrSB = NULL;

  // end recording mode
  pRc->bSBRecMode = FALSE;

  D3DPRINT(10,"<< EndStateBlock");
  return hr;
}

/*-------------------------------------------------------------------
Function Name:  DeleteStateBlock

Description:    delete the recorded state block identified by dwHandle

Return:
-------------------------------------------------------------------*/

HRESULT
DeleteStateBlock(RC *pRc, DWORD dwHandle)
{
#ifndef WINNT
  SETUP_PPDEV(pRc)
#endif
  STATEBLOCK  *pSB;


  D3DPRINT(10,">> DeleteStateBlock dwHandle=%ld",dwHandle);

  pSB = FindStateBlock(pRc, dwHandle);
  if (NULL != pSB)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "  freeing SBHandle=%ld, pSB=%lXh for pRc=%lXh",
             pSB->dwHandle, pSB, pRc);

    pRc->ppSBTable[dwHandle] = NULL;
    D3DFREE(pSB);
  }

  D3DPRINT(10,"<< DeleteStateBlock");
  return D3D_OK;
}

/*-------------------------------------------------------------------
Function Name:  ExecuteStateBlock

Description:

Return:
-------------------------------------------------------------------*/

HRESULT
ExecuteStateBlock(RC *pRc, DWORD dwHandle)
{
  STATEBLOCK       *pSB;
  DWORD             i, j;
  HRESULT           hr;

  D3DPRINT(10,">> ExecuteStateBlock dwHandle=%ld",dwHandle);

  pSB = FindStateBlock(pRc, dwHandle);
  if (NULL != pSB)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "  executing SBHandle=%ld, pSB=%lXh for pRc=%lXh",
             pSB->dwHandle, pSB, pRc);

    if (! pSB->bCompressed)
    {
      // uncompressed state block
      D3DPRINT(NORMAL_DBG_LEVEL, "    uncompressed state block");

      // Execute any necessary render states
      D3DPRINT(NORMAL_DBG_LEVEL, "      executing stored render states");
      for (i = 1; i <= MAX_RENDERSTATES; i++)   // renderstate zero is unused
      {
        if (IS_SB_RS_FLAG_SET(pSB, i))
        {
          D3DRENDERSTATETYPE  renderState;
          DWORD               data;

          renderState = i;
          data = pSB->uc.RenderStates[renderState];

          D3DPRINT(NORMAL_DBG_LEVEL, "        renderState=%ld, data=%ld", renderState, data);

          hr = _renderFuncs[renderState](pRc, data);

          if (DD_OK != hr)
            return hr;
        }
      }

      // Execute any necessary TSS's
      D3DPRINT(NORMAL_DBG_LEVEL, "      executing stored texture stage states");
      for (j = 0; j < NUMTEXTUREUNITS+1; j++)
      {
        for (i = 0; i <= MAX_TEXTURESTAGESTATES; i++)   // TSS zero is the texture handle
        {
          if (IS_SB_TSS_FLAG_SET(pSB, j, i))
          {
            DWORD stage, state, data;

            stage = j;
            state = i;
            data = pSB->uc.TssStates[stage][state];

            D3DPRINT(D3DDBGLVL, "        stage=%ld, state=%ld, data=%ld", stage, state, data);

            switch (state)
            {
              case 0 :  // Texture Handle  fix a bug with DCT 50's, they send down -1 for a texture handle.
                if (data == 0xFFFFFFFF)
                  data = 0x0;

#ifdef WINNT
                // to fix potential access violation issues with NT Stress
                // verify we have a TXTRHNDL for this texture otherwise
                // set the texture to zero
                if (data && !TXTRHNDL_PTR(data))
                  data = 0;
#endif

                ((LPDWORD)&pRc->textureStage[stage])[state] = data;
                break;

#if (DIRECT3D_VERSION < 0x0800) || (DX < 8)
              case D3DTSS_ADDRESS :
                ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSU] = data;
                ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSV] = data;
                break;
#endif 

              case D3DTSS_COLOROP:      // =  1, /* D3DTEXTUREOP - per-stage blending controls for color channels */
              case D3DTSS_COLORARG1:    // =  2, /* D3DTA_* (texture arg) */
              case D3DTSS_COLORARG2:    // =  3, /* D3DTA_* (texture arg) */
              case D3DTSS_ALPHAOP:      // =  4, /* D3DTEXTUREOP - per-stage blending controls for alpha channel */
              case D3DTSS_ALPHAARG1:    // =  5, /* D3DTA_* (texture arg) */
              case D3DTSS_ALPHAARG2:    // =  6, /* D3DTA_* (texture arg) */
                if(pRc->texMapBlend == 0x7ffffffe)
                  pRc->texMapBlend = D3DTBLEND_MODULATE;
		
                ((LPDWORD)&pRc->textureStage[stage])[state] = data;
                break;

			  case D3DTSS_TEXTURETRANSFORMFLAGS:
	            ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSU] = data;
	            ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSV] = data;
                break;


              default :
                ((LPDWORD)&pRc->textureStage[stage])[state] = data;
                break;
            }
			/* When we have updated the texture stage state, set the flag to signify that this
			   has been done.  This is so that the hardware will be updated in setupTexturing().
			   jmccartney 06/04/00  
			*/
			((LPDWORD)&pRc->textureStage[stage])[TSS_CHANGED] = 1;
          }
        }
      }
      UPDATE_HW_STATE(SC_SOMETHING);

      // Execute any necessary state for lights, materials, transforms,
      // viewport info, z range and clip planes - here -
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
      // DX 8 Stream Support (Single stream only)

      if (IS_STATEBLOCK_STREAM_FLAG_SET(pSB, 0))
      {
        D3DPRINT(NORMAL_DBG_LEVEL, "      updating stream source, stream 0");
        D3DPRINT(NORMAL_DBG_LEVEL, "        Stream=%lxh, VBHandle=%lxh, Stride=%lxh", 0, pSB->uc.StreamSource[0].dwVBHandle, pSB->uc.StreamSource[0].dwStride);

        _D3D_OP_MStream_SetSrc(pRc,
                               pSB->uc.StreamSource[0].dwStream,
                               pSB->uc.StreamSource[0].dwVBHandle,
                               pSB->uc.StreamSource[0].dwStride);
      }
      if (IS_STATEBLOCK_STREAM_FLAG_SET(pSB, 8))
      {
        D3DPRINT(NORMAL_DBG_LEVEL, "      updating stream indices");
        D3DPRINT(NORMAL_DBG_LEVEL, "        IndexHandle=%lxh, Stride=%lxh", pSB->uc.StreamIndices.dwVBHandle, pSB->uc.StreamIndices.dwStride);

        _D3D_OP_MStream_SetIndices(pRc,
                                   pSB->uc.StreamIndices.dwVBHandle,
                                   pSB->uc.StreamIndices.dwStride);
      }
      if (IS_STATEBLOCK_STREAM_FLAG_SET(pSB, 9))
      {
        D3DPRINT(NORMAL_DBG_LEVEL, "      updating vertex shader handle");
        D3DPRINT(NORMAL_DBG_LEVEL, "        ShaderHandle=%lxh", pSB->uc.dwShaderHandle);

        _D3D_OP_VertexShader_Set(pRc, pSB->uc.dwShaderHandle);
      }
#endif // DX 8
    }
    else
    {
      DWORD dwFinalState;

      // compressed state set
      D3DPRINT(NORMAL_DBG_LEVEL, "    compressed state block");

      // Execute any necessary render states
      D3DPRINT(NORMAL_DBG_LEVEL, "      executing stored render states");
      for (i = 0; i < pSB->cc.dwNumRS; i++)
      {
        D3DRENDERSTATETYPE  renderState;
        DWORD               data;

        renderState = pSB->cc.pair[i].dwType;
        data = pSB->cc.pair[i].dwValue;

        D3DPRINT(NORMAL_DBG_LEVEL, "        renderState=%ld, data=%ld", renderState, data);

        hr = _renderFuncs[renderState](pRc, data);

        if (DD_OK != hr)
          return hr;
      }

      // Execute any necessary TSS's
      D3DPRINT(NORMAL_DBG_LEVEL, "      executing stored texture stage states");
      dwFinalState = pSB->cc.dwNumRS;
      for (j = 0; j < NUMTEXTUREUNITS+1; j++)
      {
        dwFinalState += pSB->cc.dwNumTSS[j];
        for (; i < dwFinalState; i++)
        {
          DWORD stage, state, data;

          stage = j;
          state = pSB->cc.pair[i].dwType;
          data  = pSB->cc.pair[i].dwValue;

          D3DPRINT(D3DDBGLVL, "        stage=%ld, state=%ld, data=%ld", stage, state, data);

          switch (state)
          {
            case 0 :  // Texture Handle  fix a bug with DCT 50's, they send down -1 for a texture handle.
              if (data == 0xFFFFFFFF)
                data = 0x0;

#ifdef WINNT
                // to fix potential access violation issues with NT Stress
                // verify we have a TXTRHNDL for this texture otherwise
                // set the texture to zero
                if (data && !TXTRHNDL_PTR(data))
                  data = 0;
#endif

              ((LPDWORD)&pRc->textureStage[stage])[state] = data;
              break;

#if (DIRECT3D_VERSION < 0x0800) || (DX < 8)
            case D3DTSS_ADDRESS :
              ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSU] = data;
              ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSV] = data;
              break;
#endif

            case D3DTSS_COLOROP:      // =  1, /* D3DTEXTUREOP - per-stage blending controls for color channels */
            case D3DTSS_COLORARG1:    // =  2, /* D3DTA_* (texture arg) */
            case D3DTSS_COLORARG2:    // =  3, /* D3DTA_* (texture arg) */
            case D3DTSS_ALPHAOP:      // =  4, /* D3DTEXTUREOP - per-stage blending controls for alpha channel */
            case D3DTSS_ALPHAARG1:    // =  5, /* D3DTA_* (texture arg) */
            case D3DTSS_ALPHAARG2:    // =  6, /* D3DTA_* (texture arg) */
              if(pRc->texMapBlend == 0x7ffffffe)
                pRc->texMapBlend = D3DTBLEND_MODULATE;

              ((LPDWORD)&pRc->textureStage[stage])[state] = data;
              break;

			case D3DTSS_TEXTURETRANSFORMFLAGS:
	            ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSU] = data;
	            ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESSV] = data;
                break;


            default :
              ((LPDWORD)&pRc->textureStage[stage])[state] = data;
              break;
          }
		  /* Fix for PRS Issue 12791
			 When we have updated the texture stage state, set the flag to signify that this
			 has been done.  This is so that the hardware will be updated in setupTexturing().
			 Fixes problems with the game Force Commander. jmccartney 06/04/00  
		  */
		  ((LPDWORD)&pRc->textureStage[stage])[TSS_CHANGED] = 1;
        }
      }
      UPDATE_HW_STATE(SC_SOMETHING);

      // Execute any necessary state for lights, materials, transforms,
      // viewport info, z range and clip planes - here -
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
      // DX 8 Stream Support (Single stream only)
      /* V56K-DX8-PORT: pNextData was used here but never declared.  It is the
      ** cursor to the variable-size tail that follows pair[]; after the TSS
      ** loop above, dwFinalState is exactly the number of pairs stored
      ** (dwNumRS + sum of dwNumTSS[]).  C89 (VC6): open a block for it. */
    {
      LPBYTE pNextData = (LPBYTE)&pSB->cc.pair[dwFinalState];

      if (pSB->cc.dwSizeSetStreamSource)
      {
        D3DHAL_DP2SETSTREAMSOURCE *pSetStreamSource = (D3DHAL_DP2SETSTREAMSOURCE *) pNextData;

        D3DPRINT(NORMAL_DBG_LEVEL, "      updating stream source, stream 0");
        D3DPRINT(NORMAL_DBG_LEVEL, "        Stream=%lxh, VBHandle=%lxh, Stride=%lxh", pSetStreamSource->dwStream, pSetStreamSource->dwVBHandle, pSetStreamSource->dwStride);

        _D3D_OP_MStream_SetSrc(pRc,
                               pSetStreamSource->dwStream,
                               pSetStreamSource->dwVBHandle,
                               pSetStreamSource->dwStride);

        pNextData += sizeof(D3DHAL_DP2SETSTREAMSOURCE);
      }
      if (pSB->cc.dwSizeSetIndices)
      {
        D3DHAL_DP2SETINDICES *pSetIndices = (D3DHAL_DP2SETINDICES *) pNextData;

        D3DPRINT(NORMAL_DBG_LEVEL, "      updating stream indices");
        D3DPRINT(NORMAL_DBG_LEVEL, "        IndexHandle=%lxh, Stride=%lxh", pSetIndices->dwVBHandle, pSetIndices->dwStride);

        _D3D_OP_MStream_SetIndices(pRc,
                                   pSetIndices->dwVBHandle,
                                   pSetIndices->dwStride);

        pNextData += sizeof(D3DHAL_DP2SETINDICES);
      }
      if (pSB->cc.dwSizeShaderHandle)
      {
        DWORD dwShaderHandle = *((DWORD *) pNextData);

        D3DPRINT(NORMAL_DBG_LEVEL, "      updating vertex shader handle");
        D3DPRINT(NORMAL_DBG_LEVEL, "        ShaderHandle=%lxh", dwShaderHandle);

        _D3D_OP_VertexShader_Set(pRc, dwShaderHandle);

        pNextData += sizeof(DWORD);
      }
    }   /* V56K-DX8-PORT: close the pNextData block */
#endif // DX 8

      UPDATE_HW_STATE(SC_SOMETHING);
    }
  }

  D3DPRINT(10,"<< ExecuteStateBlock");
  return D3D_OK;
}

/*-------------------------------------------------------------------
Function Name:  CaptureStateBlock

Description:

Return:
-------------------------------------------------------------------*/

HRESULT
CaptureStateBlock(RC *pRc, DWORD dwHandle)
{
  STATEBLOCK  *pSB;
  DWORD       i, j;
  DWORD       dwFinalState;
  DWORD       state;
  DWORD       data;

#ifndef WINNT
  // Pulled from minivdd\devtable.h
#define FIELDOFFSET(type, field) ((DWORD)(&((type *)0)->field))
#endif

  // this array contains the offset in the RC struct where we store the various
  // render states.  Using this table we can grab the dword from the specific offset
  // to capture the current state for that render state
  // render states we do not store values for have a value of -1 in this array
  static const LONG RenderStateOffsetInRC[] =
  {
    -1,                                     // unused
                                            // typedef enum _D3DRENDERSTATETYPE {
    FIELDOFFSET(RC, texture),               //   D3DRENDERSTATE_TEXTUREHANDLE          = 1,
    FIELDOFFSET(RC, antialias),             //   D3DRENDERSTATE_ANTIALIAS              = 2,
    FIELDOFFSET(RC, textureAddress),        //   D3DRENDERSTATE_TEXTUREADDRESS         = 3,
    FIELDOFFSET(RC, texturePerspective),    //   D3DRENDERSTATE_TEXTUREPERSPECTIVE     = 4,
    FIELDOFFSET(RC, wrapU),                 //   D3DRENDERSTATE_WRAPU                  = 5,
    FIELDOFFSET(RC, wrapV),                 //   D3DRENDERSTATE_WRAPV                  = 6,
    FIELDOFFSET(RC, zEnable),               //   D3DRENDERSTATE_ZENABLE                = 7,
    FIELDOFFSET(RC, fillMode),              //   D3DRENDERSTATE_FILLMODE               = 8,
    FIELDOFFSET(RC, shadeMode),             //   D3DRENDERSTATE_SHADEMODE              = 9,
    -1,                                     //   D3DRENDERSTATE_LINEPATTERN            = 10,
    -1,                                     //   D3DRENDERSTATE_MONOENABLE             = 11,
    -1,                                     //   D3DRENDERSTATE_ROP2                   = 12,
    -1,                                     //   D3DRENDERSTATE_PLANEMASK              = 13,
    FIELDOFFSET(RC, zWriteEnable),          //   D3DRENDERSTATE_ZWRITEENABLE           = 14,
    FIELDOFFSET(RC, alphaTestEnable),       //   D3DRENDERSTATE_ALPHATESTENABLE        = 15,
    -1,                                     //   D3DRENDERSTATE_LASTPIXEL              = 16,
    FIELDOFFSET(RC, texMag),                //   D3DRENDERSTATE_TEXTUREMAG             = 17,
    FIELDOFFSET(RC, texMin),                //   D3DRENDERSTATE_TEXTUREMIN             = 18,
    FIELDOFFSET(RC, srcBlend),              //   D3DRENDERSTATE_SRCBLEND               = 19,
    FIELDOFFSET(RC, dstBlend),              //   D3DRENDERSTATE_DESTBLEND              = 20,
#if /*(DIRECT3D_VERSION >= 0x0700) && (DX >= 7) &&*/ defined(REMOVE_OBSOLETE1_FOR_DX7_WIN9X) && !defined(WINNT)
    -1,
#else
    FIELDOFFSET(RC, texMapBlend),           //   D3DRENDERSTATE_TEXTUREMAPBLEND        = 21,
#endif
    FIELDOFFSET(RC, cullMode),              //   D3DRENDERSTATE_CULLMODE               = 22,
    FIELDOFFSET(RC, zFunc),                 //   D3DRENDERSTATE_ZFUNC                  = 23,
    FIELDOFFSET(RC, alphaRef),              //   D3DRENDERSTATE_ALPHAREF               = 24,
    FIELDOFFSET(RC, alphaFunc),             //   D3DRENDERSTATE_ALPHAFUNC              = 25,
    FIELDOFFSET(RC, ditherEnable),          //   D3DRENDERSTATE_DITHERENABLE           = 26,
    FIELDOFFSET(RC, blendEnable),           //   D3DRENDERSTATE_ALPHABLENDENABLE       = 27,
    FIELDOFFSET(RC, fogEnable),             //   D3DRENDERSTATE_FOGENABLE              = 28,
    FIELDOFFSET(RC, specular),              //   D3DRENDERSTATE_SPECULARENABLE         = 29,
    FIELDOFFSET(RC, zVisible),              //   D3DRENDERSTATE_ZVISIBLE               = 30,
    -1,                                     //   D3DRENDERSTATE_SUBPIXEL               = 31,
    -1,                                     //   D3DRENDERSTATE_SUBPIXELX              = 32,
    -1,                                     //   D3DRENDERSTATE_STIPPLEDALPHA          = 33,
    FIELDOFFSET(RC, fogColor),              //   D3DRENDERSTATE_FOGCOLOR               = 34,
    FIELDOFFSET(RC, fogTableMode),          //   D3DRENDERSTATE_FOGTABLEMODE           = 35,
    FIELDOFFSET(RC, fogTableStart),         //   D3DRENDERSTATE_FOGSTART               = 36,
    FIELDOFFSET(RC, fogTableEnd),           //   D3DRENDERSTATE_FOGEND                 = 37,
    FIELDOFFSET(RC, fogDensity),            //   D3DRENDERSTATE_FOGDENSITY             = 38,
    -1,                                     //   D3DRENDERSTATE_STIPPLEENABLE          = 39,
    -1,                                     //   D3DRENDERSTATE_EDGEANTIALIAS          = 40,
    FIELDOFFSET(RC, colorKeyEnable),        //   D3DRENDERSTATE_COLORKEYENABLE         = 41,
    FIELDOFFSET(RC, alphaBlendEnable),      //   D3DRENDERSTATE_ALPHABLENDENABLE       = 42
    -1,                                     //   D3DRENDERSTATE_BORDERCOLOR            = 43,
    FIELDOFFSET(RC, textureAddressU),       //   D3DRENDERSTATE_TEXTUREADDRESSU        = 44,
    FIELDOFFSET(RC, textureAddressV),       //   D3DRENDERSTATE_TEXTUREADDRESSV        = 45,
    -1,                                     //   D3DRENDERSTATE_MIPMAPLODBIAS          = 46,
    FIELDOFFSET(RC, zBias),                 //   D3DRENDERSTATE_ZBIAS                  = 47,
    -1,                                     //   D3DRENDERSTATE_RANGEFOGENABLE         = 48,
    -1,                                     //   D3DRENDERSTATE_ANISOTROPY             = 49,
    -1,                                     //   D3DRENDERSTATE_FLUSHBATCH             = 50,
    -1,                                     //   D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT=51,
    FIELDOFFSET(RC, stencilEnable),         //   D3DRENDERSTATE_STENCILENABLE          = 52,
    FIELDOFFSET(RC, stencilFail),           //   D3DRENDERSTATE_STENCILFAIL            = 53,
    FIELDOFFSET(RC, stencilZFail),          //   D3DRENDERSTATE_STENCILZFAIL           = 54,
    FIELDOFFSET(RC, stencilPass),           //   D3DRENDERSTATE_STENCILPASS            = 55,
    FIELDOFFSET(RC, stencilFunc),           //   D3DRENDERSTATE_STENCILFUNC            = 56,
    FIELDOFFSET(RC, stencilRef),            //   D3DRENDERSTATE_STENCILREF             = 57,
    FIELDOFFSET(RC, stencilMask),           //   D3DRENDERSTATE_STENCILMASK            = 58,
    FIELDOFFSET(RC, stencilWriteMask),      //   D3DRENDERSTATE_STENCILWRITEMASK       = 59,
    FIELDOFFSET(RC, textureFactor),         //   D3DRENDERSTATE_TEXTUREFACTOR          = 60,
    -1,                                     //                                         = 61,
    -1,                                     //   D3DRENDERSTATE_SCENECAPTURE           = 62,
    -1,                                     //                                         = 63,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN00       = 64,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN01       = 65,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN02       = 66,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN03       = 67,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN04       = 68,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN05       = 69,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN06       = 70,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN07       = 71,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN08       = 72,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN09       = 73,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN10       = 74,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN11       = 75,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN12       = 76,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN13       = 77,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN14       = 78,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN15       = 79,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN16       = 80,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN17       = 81,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN18       = 82,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN19       = 83,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN20       = 84,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN21       = 85,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN22       = 86,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN23       = 87,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN24       = 88,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN25       = 89,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN26       = 90,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN27       = 91,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN28       = 92,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN29       = 93,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN30       = 94,
    -1,                                     //   D3DRENDERSTATE_STIPPLEPATTERN31       = 95,
    -1,                                     //                                         = 96,
    -1,                                     //                                         = 97,
    -1,                                     //                                         = 98,
    -1,                                     //                                         = 99,
    -1,                                     //                                         = 100,
    -1,                                     //                                         = 101,
    -1,                                     //                                         = 102,
    -1,                                     //                                         = 103,
    -1,                                     //                                         = 104,
    -1,                                     //                                         = 105,
    -1,                                     //                                         = 106,
    -1,                                     //                                         = 107,
    -1,                                     //                                         = 108,
    -1,                                     //                                         = 109,
    -1,                                     //                                         = 110,
    -1,                                     //                                         = 111,
    -1,                                     //                                         = 112,
    -1,                                     //                                         = 113,
    -1,                                     //                                         = 114,
    -1,                                     //                                         = 115,
    -1,                                     //                                         = 116,
    -1,                                     //                                         = 117,
    -1,                                     //                                         = 118,
    -1,                                     //                                         = 119,
    -1,                                     //                                         = 120,
    -1,                                     //                                         = 121,
    -1,                                     //                                         = 122,
    -1,                                     //                                         = 123,
    -1,                                     //                                         = 124,
    -1,                                     //                                         = 125,
    -1,                                     //                                         = 126,
    -1,                                     //                                         = 127,
    FIELDOFFSET(RC, textureStage[0].wrap),  //   D3DRENDERSTATE_WRAP0                  = 128,
    FIELDOFFSET(RC, textureStage[1].wrap),  //   D3DRENDERSTATE_WRAP1                  = 129,
    -1,                                     //   D3DRENDERSTATE_WRAP2                  = 130,
    -1,                                     //   D3DRENDERSTATE_WRAP3                  = 131,
    -1,                                     //   D3DRENDERSTATE_WRAP4                  = 132,
    -1,                                     //   D3DRENDERSTATE_WRAP5                  = 133,
    -1,                                     //   D3DRENDERSTATE_WRAP6                  = 134,
    -1,                                     //   D3DRENDERSTATE_WRAP7                  = 135,
    -1,                                     //   D3DRENDERSTATE_CLIPPING               = 136,
    -1,                                     //   D3DRENDERSTATE_LIGHTING               = 137,
    -1,                                     //   D3DRENDERSTATE_EXTENTS                = 138,
    -1,                                     //   D3DRENDERSTATE_AMBIENT                = 139,
    -1,                                     //   D3DRENDERSTATE_FOGVERTEXMODE          = 140,
    -1,                                     //   D3DRENDERSTATE_COLORVERTEX            = 141,
    -1,                                     //   D3DRENDERSTATE_LOCALVIEWER            = 142,
    -1,                                     //   D3DRENDERSTATE_NORMALIZENORMALS       = 143,
    -1,                                     //   D3DRENDERSTATE_COLORKEYBLENDENABLE    = 144,
    -1,                                     //   D3DRENDERSTATE_DIFFUSEMATERIALSOURCE  = 145,
    -1,                                     //   D3DRENDERSTATE_SPECULARMATERIALSOURCE = 146,
    -1,                                     //   D3DRENDERSTATE_AMBIENTMATERIALSOURCE  = 147,
    -1,                                     //   D3DRENDERSTATE_EMISSIVEMATERIALSOURCE = 148,
    -1,                                     //   D3DRENDERSTATE_ALPHASOURCE            = 149,
    -1,                                     //   D3DRENDERSTATE_FOGFACTORSOURCE        = 150,
    -1,                                     //   D3DRENDERSTATE_VERTEXBLEND            = 151,
    -1,                                     //   D3DRENDERSTATE_CLIPPLANEENABLE        = 152,
    -1,                                     //   D3DRENDERSTATE_POINTSIZE              = 153,
    -1,                                     //   D3DRENDERSTATE_POINTATTENUATION_A     = 154,
    -1,                                     //   D3DRENDERSTATE_POINTATTENUATION_B     = 155,
    -1,                                     //   D3DRENDERSTATE_POINTATTENUATION_C     = 156,
    -1,                                     //   D3DRENDERSTATE_POINTSIZEMIN           = 157,
    -1,                                     //   D3DRENDERSTATE_POINTSPRITE_ENABLE     = 158,
                                            //   D3DRENDERSTATE_FORCE_DWORD            = 0x7fffffff,
                                            // } D3DRENDERSTATETYPE;
  };


  D3DPRINT(10,">> CaptureStateBlock dwHandle=%ld",dwHandle);

  pSB = FindStateBlock(pRc, dwHandle);
  if (NULL != pSB)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "  capturing SBHandle=%ld, pSB=%lXh for pRc=%lXh",
             pSB->dwHandle, pSB, pRc);

    if (! pSB->bCompressed)
    {
      // uncompressed state set
      D3DPRINT(NORMAL_DBG_LEVEL, "    uncompressed state block");

      // Capture any necessary render states
      D3DPRINT(NORMAL_DBG_LEVEL, "      capturing render states");
      for (i = 1; i <= MAX_RENDERSTATES; i++)     // renderstate zero is unused
      {
        if (IS_SB_RS_FLAG_SET(pSB, i))
        {
          if (-1 == RenderStateOffsetInRC[i])
          {
            // special case for scene capture
            if (D3DRENDERSTATE_SCENECAPTURE == i)
            {
              SETUP_PPDEV(pRc)
              data = (_D3(flags) & IN_RENDER_SCENE ? TRUE : FALSE);
            }
            // otherwise they are render states we don't care about
            // so just return zero
            else
              data = 0;
          }
          else
            data = *(DWORD *)((BYTE *)pRc + RenderStateOffsetInRC[i]);

          pSB->uc.RenderStates[i] = data;

          D3DPRINT(NORMAL_DBG_LEVEL, "        renderState=%ld, data=%ld", i, pSB->uc.RenderStates[i]);
        }
      }

      // Capture any necessary TSS's
      D3DPRINT(NORMAL_DBG_LEVEL, "      capturing texture stage states");
      for (j = 0; j < NUMTEXTUREUNITS+1; j++)
      {
        for (i = 0; i <= MAX_TEXTURESTAGESTATES; i++)   // TSS zero is the texture handle
        {
          if (IS_SB_TSS_FLAG_SET(pSB, j, i))
          {
            data = ((LPDWORD)&pRc->textureStage[j])[i];

            pSB->uc.TssStates[j][i] = data;

            D3DPRINT(NORMAL_DBG_LEVEL, "        stage=%ld, state=%ld, data=%ld", j, i, pSB->uc.TssStates[j][i]);
          }
        }
      }

      // Capture any necessary state for lights, materials, transforms,
      // viewport info, z range and clip planes - here -
    }
    else
    {
      // compressed state set
      D3DPRINT(NORMAL_DBG_LEVEL, "    compressed state block");

      // Capture any necessary render states
      D3DPRINT(NORMAL_DBG_LEVEL, "      capturing render states");
      for (i = 0; i < pSB->cc.dwNumRS; i++)
      {
        state = pSB->cc.pair[i].dwType;
        if (-1 == RenderStateOffsetInRC[state])
          data = 0;
        else
          data = *(DWORD *)((BYTE *)pRc + RenderStateOffsetInRC[state]);

        pSB->cc.pair[i].dwValue = data;

        D3DPRINT(NORMAL_DBG_LEVEL, "        renderState=%ld, data=%ld", state, pSB->cc.pair[i].dwValue);
      }

      // Capture any necessary TSS's
      D3DPRINT(NORMAL_DBG_LEVEL, "      capturing texture stage states");
      dwFinalState = pSB->cc.dwNumRS;
      for (j = 0; j < NUMTEXTUREUNITS+1; j++)
      {
        dwFinalState += pSB->cc.dwNumTSS[j];
        for (; i < dwFinalState; i++)
        {
          DWORD state;
          DWORD data;

          state = pSB->cc.pair[i].dwType;
          data = ((LPDWORD)&pRc->textureStage[j])[state];

          pSB->cc.pair[i].dwValue = data;

          D3DPRINT(NORMAL_DBG_LEVEL, "        stage=%ld, state=%ld, data=%ld", j, state, pSB->cc.pair[i].dwValue);
        }
      }

      // Capture any necessary state for lights, materials, transforms,
      // viewport info, z range and clip planes - here -
    }
  }

  D3DPRINT(10,"<< CaptureStateBlock");
  return D3D_OK;
}

/*-------------------------------------------------------------------
Function Name:  ReleaseContextStateBlocks

Description:    Delete any remaining state blocks for cleanup purposes

Return:
-------------------------------------------------------------------*/

VOID
ReleaseContextStateBlocks(RC *pRc)
{
#ifndef WINNT
  SETUP_PPDEV(pRc)
#endif
  STATEBLOCK  *pSB;
  DWORD       i;


  D3DPRINT(10,">> ReleaseContextStateBlocks");

  if (pRc->ppSBTable)
  {
    // walk over table and free any state blocks still in use
    for (i = 1; i < (DWORD)pRc->ppSBTable[0]; i++)    // entry zero is the array size
    {
      pSB = pRc->ppSBTable[i];
      if (NULL != pSB)
      {
        D3DPRINT(NORMAL_DBG_LEVEL, "  freeing SBHandle=%ld, pSB=%lXh for pRc=%lXh",
                 pSB->dwHandle, pSB, pRc);
        D3DFREE(pSB);
      }
    }

    // free state block table
    D3DPRINT(NORMAL_DBG_LEVEL, "  freeing ppSBTable=%lXh for pRc=%lXh", pRc->ppSBTable, pRc);
    D3DFREE(pRc->ppSBTable);
    pRc->ppSBTable = NULL;
  }

  D3DPRINT(10,"<< ReleaseContextStateBlocks");
}

/**************************************************************************
* S T A T I C   F U N C T I O N S
***************************************************************************/

/*-------------------------------------------------------------------
Function Name:  FindStateBlock

Description:    Find a state block identified by dwHandle
                If not found, returns NULL.

Return:
-------------------------------------------------------------------*/

static STATEBLOCK *
FindStateBlock(RC *pRc, DWORD dwHandle)
{
  if (NULL == pRc->ppSBTable)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "No state blocks yet - %ld not found", dwHandle);
    return NULL;
  }

  if (dwHandle < (DWORD)pRc->ppSBTable[0])
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "found SBHandle=%ld, pSB=%lXh for pRc=%lXh",
             dwHandle, pRc->ppSBTable[dwHandle], pRc);
    return pRc->ppSBTable[dwHandle];
  }
  else
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "State block %ld not found (max = %ld)",
             dwHandle, (DWORD)pRc->ppSBTable[0]);
    return NULL;
  }
}

/*-------------------------------------------------------------------
Function Name:  AddStateBlockToTable

Description:    Add an entry to the state block table.
                If necessary, grow the table.

Return:
-------------------------------------------------------------------*/

static HRESULT
AddStateBlockToTable(RC *pRc, DWORD dwHandle, STATEBLOCK *pSB)
{
#ifndef WINNT
  SETUP_PPDEV(pRc)
#endif
  DWORD       dwNewSize;
  STATEBLOCK  **pNewSBTable;
  DWORD       dwCount, i, j;


  // If the current list is not large enough, we'll have to grow a new one
  if ((NULL == pRc->ppSBTable) || (dwHandle > (DWORD)pRc->ppSBTable[0]))
  {
    // New size of our state block table
    // (round up dwHandle in steps of LISTGROW)
    // we need to account for using index zero as a count of how many elements are in
    // the ppSBTable array, so add one to dwHandle
    dwNewSize = (((dwHandle + 1) + (LISTGROWSIZE - 1)) / LISTGROWSIZE) * LISTGROWSIZE;

    // we have to grow our list
    pNewSBTable = (STATEBLOCK **)D3DMALLOCZ(dwNewSize*sizeof(STATEBLOCK *), 0);
    D3DPRINT(NORMAL_DBG_LEVEL,"Growing pRc=%lX's ppSBTable[%X] size to %lXh",
             pRc, pNewSBTable, dwNewSize);

    if (NULL == pNewSBTable)
    {
      D3DPRINT(0, "AddStateBlockToTable failed to increase SBTable");
      return DDERR_OUTOFMEMORY;
    }

    if (pRc->ppSBTable)
    {
      // if we already had a previous list, we must transfer its data
      // copy counter in element zero plus all STATEBLOCK *'s to new array
      memcpy(pNewSBTable,
             pRc->ppSBTable,
             ((DWORD)pRc->ppSBTable[0] + 1) * sizeof(STATEBLOCK *));

#ifdef MEMCHECK
      {
        DWORD i;

        for (i = 1; i < (DWORD)pRc->ppSBTable[0]; i++)
        {
          if (NULL != pRc->ppSBTable[i])
            UPDATE_BLOCK_DATA(pRc->ppSBTable[i], &pNewSBTable[i]);
        }
      }
#endif

      // and get rid of it
      D3DFREE(pRc->ppSBTable);
      D3DPRINT(NORMAL_DBG_LEVEL,"Freeing pRc=%lXh old ppSBTable[%X]",
               pRc, pRc->ppSBTable);
    }

    // New index table data
    pRc->ppSBTable = pNewSBTable;
    UPDATE_BLOCK_DATA(pNewSBTable, &pRc->ppSBTable);
    // store size in ppSBTable[0]
    // since we're using element zero to hold the array size
    // we only have dwNewSize-1 entries to store STATEBLOCK * elements in
    (DWORD)pRc->ppSBTable[0] = dwNewSize - 1;
  }

  // if there are no saved states in the state block
  // release it and return an error
  dwCount = 0;
  if (pSB->bCompressed)
  {
    dwCount += pSB->cc.dwNumRS;
    for (j = 0; j < NUMTEXTUREUNITS+1; j++)
      dwCount += pSB->cc.dwNumTSS[j];
  }
  else
  {
    // Calculate how large
    for (i = 1; i <= MAX_RENDERSTATES; i++)
    {
      if (IS_SB_RS_FLAG_SET(pSB, i))
        dwCount++;
    }
  
    for (j = 0; j < NUMTEXTUREUNITS+1; j++)
    {
      for (i = 0; i <= MAX_TEXTURESTAGESTATES; i++)
      {
        if (IS_SB_TSS_FLAG_SET(pSB, j, i))
          dwCount++;
      }
    }
  }
  if (0 == dwCount)
  {
    D3DPRINT(NORMAL_DBG_LEVEL,"Releasing state block (no saved states), pRc=%lXh SBHandle=%ld pSB = %lXh",
             pRc, dwHandle, pSB);

    pRc->ppSBTable[dwHandle] = NULL;
    D3DFREE(pSB);
  }
  else
  {
    // Store our state set pointer into our access list
    pRc->ppSBTable[dwHandle] = pSB;
    UPDATE_BLOCK_DATA(pSB, &pRc->ppSBTable[dwHandle]);
  
    D3DPRINT(NORMAL_DBG_LEVEL,"Store state block, pRc=%lXh SBHandle=%ld pSB = %lXh",
             pRc, dwHandle, pSB);
  }

  return DD_OK;
}

/*-------------------------------------------------------------------
Function Name:  CompressStateBlock

Description:    Compress a state set so it uses the minimum necessary
                space. Since we expect some apps to make extensive use
                of state sets we want to keep things tidy.
                Returns address of new structure (ir old, if it wasn't compressed)

Return:
-------------------------------------------------------------------*/

#define ENABLE_COMPRESSED_STATEBLOCKS   1

static STATEBLOCK *
CompressStateBlock(RC *pRc, STATEBLOCK *pUncompressedSB)
{
#if ENABLE_COMPRESSED_STATEBLOCKS
#ifndef WINNT
  SETUP_PPDEV(pRc)
#endif
  STATEBLOCK  *pCompressedSB;
  DWORD       i, j, dwSize, dwIndex, dwCount;

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
  DWORD       dwSizeSetStreamSource;
  DWORD       dwSizeSetIndices;
  DWORD       dwSizeShaderHandle;
#endif

  D3DPRINT(NORMAL_DBG_LEVEL, "attempting to compress pSB=%lXh", pUncompressedSB);

  // Create a new state set of just the right size we need

  // Calculate how large
  dwCount = 0;
  for (i = 1; i <= MAX_RENDERSTATES; i++)
  {
    if (IS_SB_RS_FLAG_SET(pUncompressedSB, i))
      dwCount++;
  }
  D3DPRINT(NORMAL_DBG_LEVEL, "  stored render state count = %ld", dwCount);

  for (j = 0; j < NUMTEXTUREUNITS+1; j++)
  {
    for (i = 0; i <= MAX_TEXTURESTAGESTATES; i++)
    {
      if (IS_SB_TSS_FLAG_SET(pUncompressedSB, j, i))
        dwCount++;
    }
    D3DPRINT(NORMAL_DBG_LEVEL, "  stored tss[%ld]+renderstate count = %ld", j, dwCount);
  }
  D3DPRINT(NORMAL_DBG_LEVEL, "  total stored tss + renderstate count = %ld", dwCount);

  // Create a new state set of just the right size we need
  // ANY CHANGE MADE TO THE STATEBLOCK structure MUST BE REFLECTED HERE!
  dwSize = 2*sizeof(DWORD) +                          // handle, compressed flag
           (1+(NUMTEXTUREUNITS+1))*sizeof(DWORD) +    // # of RS & TSS[NUMTEXTUREUNITS+1]
           2*dwCount*sizeof(DWORD);                   // compressed structure

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
  if (IS_STATEBLOCK_STREAM_FLAG_SET(pUncompressedSB, 0))
    dwSizeSetStreamSource = sizeof(D3DHAL_DP2SETSTREAMSOURCE);
  else
    dwSizeSetStreamSource = 0;

  if (IS_STATEBLOCK_STREAM_FLAG_SET(pUncompressedSB, 8))
    dwSizeSetIndices = sizeof(D3DHAL_DP2SETINDICES);
  else
    dwSizeSetIndices = 0;

  if (IS_STATEBLOCK_STREAM_FLAG_SET(pUncompressedSB, 9))
    dwSizeShaderHandle = sizeof(DWORD);
  else
    dwSizeShaderHandle = 0;

#endif

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
  /* V56K-DX8-SBSIZE: cc gains THREE DWORDs under DX8
  ** (dwSizeSetStreamSource/dwSizeSetIndices/dwSizeShaderHandle) and they sit
  ** ahead of pair[].  The block is allocated from dwSize, so omitting them --
  ** as the original DX8 change did, despite the warning above -- leaves it 12
  ** bytes short and the stream tail written below overruns the allocation. */
  dwSize += 3*sizeof(DWORD);                            // the three cc.dwSize* fields
  dwSize += dwSizeSetStreamSource + dwSizeSetIndices +  // stream source (Single stream only) & stream indices
            dwSizeShaderHandle;                         // vertex shader handle
#endif

  if (dwSize >= sizeof(STATEBLOCK))
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "compressed SB size would be larger than uncompressed SB size");
    D3DPRINT(NORMAL_DBG_LEVEL, "leaving SB uncompressed");

    // it is not efficient to compress, leave uncompressed !
    pUncompressedSB->bCompressed = FALSE;
    return pUncompressedSB;
  }

  pCompressedSB = (STATEBLOCK *)D3DMALLOCZ(dwSize, 0);
  if (NULL != pCompressedSB)
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "pCompressedSB=%lXh", pCompressedSB);

    // adjust data in new compressed state set
    pCompressedSB->bCompressed = TRUE;
    pCompressedSB->dwHandle = pUncompressedSB->dwHandle;

    // Transfer our info to this new state set
    pCompressedSB->cc.dwNumRS = 0;
    for (i = 0; i < NUMTEXTUREUNITS+1; i++)
      pCompressedSB->cc.dwNumTSS[i] = 0;
    dwIndex = 0;

    D3DPRINT(NORMAL_DBG_LEVEL, "  compressing render states");
    for (i = 1; i <= MAX_RENDERSTATES; i++)
    {
      if (IS_SB_RS_FLAG_SET(pUncompressedSB, i))
      {
        pCompressedSB->cc.pair[dwIndex].dwType = i;
        pCompressedSB->cc.pair[dwIndex].dwValue = pUncompressedSB->uc.RenderStates[i];

        D3DPRINT(NORMAL_DBG_LEVEL, "    dwIndex=%ld, state=%ld, data=%ld",
                 dwIndex, pCompressedSB->cc.pair[dwIndex].dwType,
                 pCompressedSB->cc.pair[dwIndex].dwValue);

        pCompressedSB->cc.dwNumRS++;
        dwIndex++;
      }
    }
    D3DPRINT(NORMAL_DBG_LEVEL, "    num compressed render states = %ld", pCompressedSB->cc.dwNumRS);

    D3DPRINT(NORMAL_DBG_LEVEL, "  compressing texture stage states");
    for (j = 0; j < NUMTEXTUREUNITS+1; j++)
    {
      for (i = 0; i <= MAX_TEXTURESTAGESTATES; i++)
      {
        if (IS_SB_TSS_FLAG_SET(pUncompressedSB, j, i))
        {
          pCompressedSB->cc.pair[dwIndex].dwType = i;
          pCompressedSB->cc.pair[dwIndex].dwValue = pUncompressedSB->uc.TssStates[j][i];

          D3DPRINT(NORMAL_DBG_LEVEL, "    dwIndex=%ld, stage=%ld, state=%ld, data=%ld",
                   dwIndex, j, pCompressedSB->cc.pair[dwIndex].dwType,
                   pCompressedSB->cc.pair[dwIndex].dwValue);

          pCompressedSB->cc.dwNumTSS[j]++;
          dwIndex++;
        }
      }
      D3DPRINT(NORMAL_DBG_LEVEL, "    num compressed TSS[%ld] states = %ld", j, pCompressedSB->cc.dwNumTSS[j]);
    }

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
   {
    /* V56K-DX8-PORT: matching cursor for the writer -- dwIndex is the number of
    ** pair[] entries just emitted, so the tail starts right after them. */
    LPBYTE pNextData = (LPBYTE)&pCompressedSB->cc.pair[dwIndex];

    pCompressedSB->cc.dwSizeSetStreamSource = dwSizeSetStreamSource;
    if(dwSizeSetStreamSource)
    {
        D3DPRINT(NORMAL_DBG_LEVEL, "  copying stream source");

        memcpy(pNextData, &pUncompressedSB->uc.StreamSource[0], sizeof(D3DHAL_DP2SETSTREAMSOURCE));

        pNextData += sizeof(D3DHAL_DP2SETSTREAMSOURCE);
    }
    pCompressedSB->cc.dwSizeSetIndices = dwSizeSetIndices;
    if(dwSizeSetIndices)
    {
        D3DPRINT(NORMAL_DBG_LEVEL, "  copying stream indices");

        memcpy(pNextData, &pUncompressedSB->uc.StreamIndices, sizeof(D3DHAL_DP2SETINDICES));

        pNextData += sizeof(D3DHAL_DP2SETINDICES);
    }
    pCompressedSB->cc.dwSizeShaderHandle = dwSizeShaderHandle;
    if(dwSizeShaderHandle)
    {
        D3DPRINT(NORMAL_DBG_LEVEL, "  copying vertex shader handle");

        memcpy(pNextData, &pUncompressedSB->uc.dwShaderHandle, sizeof(DWORD));

        pNextData += sizeof(DWORD);
    }
   }    /* V56K-DX8-PORT: close the writer pNextData block */
#endif

    // Get rid of the old(uncompressed) one
    D3DPRINT(NORMAL_DBG_LEVEL, "  freeing pUncompressedSB=%lXh", pUncompressedSB);
    D3DFREE(pUncompressedSB);
    return pCompressedSB;
  }
  else
  {
    D3DPRINT(NORMAL_DBG_LEVEL, "Not enough memory left to compress state block");
    D3DPRINT(NORMAL_DBG_LEVEL, "leaving SB uncompressed");

    pUncompressedSB->bCompressed = FALSE;
    return pUncompressedSB;
  }
#else
  return pUncompressedSB;
#endif
}

#endif

