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
** $Revision: 63$
** $Date: 10/26/00 7:59:28 AM$
**
** $Log:
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
    GETPIXELFORMAT(TXTRHNDL(pRc->DDSHndl)->dwBitCnt >> 3, dstPixelFormat);
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
    dstBaseAddr = GET_HW_OFFSET(pRc->DDSHndl);
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
        dstBaseAddr = GET_AAHW_OFFSET(pRc->DDSHndl);

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
    dstBaseAddr = GET_HW_OFFSET(pRc->DDSZHndl);
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
        dstBaseAddr = GET_AAHW_OFFSET(pRc->DDSZHndl);

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

#ifdef WINNT
    auxBufferAddr = GET_HW_ADDR(pRc->lpDDSZ);
#else
    auxBufferAddr = GET_HW_OFFSET(pRc->DDSZHndl);
#endif

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

  ASSERTDD(NULL != pRc->lpDDS, " NULL render target");
  ASSERTDD(NULL != (void *)pRc->lpDDS->lpGbl->dwReserved1, " NULL texture desc");

  colBufferAddr = GET_HW_OFFSET(pRc->DDSHndl);

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

    colBufferAddr = GET_HW_OFFSET(pRc->DDSHndl);

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
        ULONG AABufferAddr = GET_AAHW_OFFSET(pRc->DDSHndl);

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

    auxBufferAddr = GET_HW_OFFSET(pRc->DDSZHndl);
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
      ULONG AABufferAddr = GET_AAHW_OFFSET(pRc->DDSZHndl);
#ifdef DEBUG
      ULONG lastAABufferAddr = _D3(last).auxAABufferAddr; // to help with debug
#endif

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
    PALHNDL **newlist= (PALHNDL **)DXMALLOCZ(sizeof(PALHNDL *)*newsize);
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
      DXFREE(pHndlList->ppPalHndlList);
      D3DPRINT(NORMAL_DBG_LEVEL,"Freeing pDDLcl=%X's old PaletteList[%X]",
               pRc->pDDLcl, pHndlList->ppPalHndlList);
    }

    pHndlList->ppPalHndlList = newlist;
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
    pPalHndl = (PALHNDL *)DXMALLOCZ(sizeof(PALHNDL));

    if (NULL == pPalHndl)
    {
      D3DPRINT(0, "D3DDP2OP_SETPALETTE out of memory, failed to alloc PALHNDL");
      return DDERR_OUTOFMEMORY;
    }

    // store the PALHNDL in the pHndlList array in the dwPaletteHandle element
    pHndlList->ppPalHndlList[dwPaletteHandle] = pPalHndl;
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
    PALHNDL **newlist= (PALHNDL **)DXMALLOCZ(sizeof(PALHNDL *)*newsize);
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
      DXFREE(pHndlList->ppPalHndlList);
      D3DPRINT(NORMAL_DBG_LEVEL,"Freeing pDDLcl=%X's old PaletteList[%X]",
               pRc->pDDLcl, pHndlList->ppPalHndlList);
    }

    pHndlList->ppPalHndlList = newlist;
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
    pPalHndl = (PALHNDL *)DXMALLOCZ(sizeof(PALHNDL));

    if (NULL == pPalHndl)
    {
      D3DPRINT(0, "D3DDP2OP_SETPALETTE out of memory, failed to alloc PALHNDL");
      return DDERR_OUTOFMEMORY;
    }

    // store the PALHNDL in the pHndlList array in the dwPaletteHandle element
    pHndlList->ppPalHndlList[dwPaletteHandle] = pPalHndl;

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
  pSB = DXMALLOCZ(sizeof(STATEBLOCK));
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
    DXFREE(pSB);
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
        if (IS_STATEBLOCK_RENDERSTATE_FLAG_SET(pSB, i))
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
          if (IS_STATEBLOCK_TEXSTAGESTATE_FLAG_SET(pSB, j, i))
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
                ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESS] = data;
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

	  // Viewport info
	  if( IS_STATEBLOCK_VIEWPORT_FLAG_SET(pSB) )
	  {
        SETUP_PPDEV ( pRc )         // initializes ppdev
		DWORD dwX		= pSB->uc.ViewportInfoState.dwX;
		DWORD dwY		= pSB->uc.ViewportInfoState.dwY;
		DWORD dwWidth	= pSB->uc.ViewportInfoState.dwWidth;
		DWORD dwHeight	= pSB->uc.ViewportInfoState.dwHeight;

	    // viewports change every version of DX
	    D3DPRINT(NORMAL_DBG_LEVEL, "      updating viewport from %lXh", (DWORD)&(pSB->uc.ViewportInfoState) );
	    D3DPRINT(NORMAL_DBG_LEVEL, "X=%d  Y=%d  Width=%d  Height=%d", dwX, dwY, dwWidth, dwHeight);
#ifdef TnL_HAL
        // Update T&L viewport state IF it really changed
		if( (pRc->tl.Viewport.dwX != dwX) ||
            (pRc->tl.Viewport.dwY != dwY) ||
            (pRc->tl.Viewport.dwWidth != dwWidth) ||
            (pRc->tl.Viewport.dwHeight != dwHeight) )
        {
	      pRc->tl.Viewport.dwX = dwX;
	      pRc->tl.Viewport.dwY = dwY;
	      pRc->tl.Viewport.dwWidth = dwWidth;
	      pRc->tl.Viewport.dwHeight = dwHeight;
		  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VIEWRECT;
		}
#endif

        if (IS_NAPALM) 
        {
			if ((dwX+dwWidth)%2==1)     /* Adjust right to even boundary */
				dwWidth += 1;
			if ((dwX+dwWidth) > 0xffe)  /* clipRight1 must <= 0xffe */
				dwWidth = 0xffe-dwX;
			if ((dwY+dwHeight) > 0xfff) /* clipBottom1 must <= 0xfff */
                dwHeight = 0xfff-dwY;

			/* the left must be even - Taken care of by dwX&0xffe below*/
			pRc->sst.clipLeftRight1 = ((dwX&0xffe)<<16) | (((dwX+dwWidth)&0xffe)<<0);      //Left<Right
			pRc->sst.clipBottomTop1 = (((dwY+dwHeight)&0xfff)<<0) |  (((dwY)&0xfff)<<16);//Top<Bottom
			D3DPRINT(255,"NAPALM VIEWPORTINFO %08x %08x %08x %08x", dwX,dwY,dwWidth,dwHeight );
		}
#ifdef NEW_CLIP_FOR_GB
		pRc->sst.clipLeftRight= (((dwX                )&0xfff)<<16) | (((dwX+dwWidth /*+1*/)&0xfff)<<0 ); //Take out +1 so bottom and right are exclusive of drawn pixels (not drawn).  
		pRc->sst.clipBottomTop= (((dwY+dwHeight /*+1*/)&0xfff)<<0 ) | (((dwY               )&0xfff)<<16);
#endif
	  }

#ifdef TnL_HAL
      // Execute any necessary state for lights, materials, transforms,
      // viewport info, z range and clip planes - here -

	  // Lights
	  j = pRc->tl.lighting.dwLightArraySize;
	  if (j > TLMAX_LIGHTS)
	  {
      	D3DPRINT(NORMAL_DBG_LEVEL, "BAD: More lights than we've allocated capture states for!!!");
      	D3DPRINT(NORMAL_DBG_LEVEL, "     Allocated space for %ld, Total lights = %ld", TLMAX_LIGHTS, j);
		j = TLMAX_LIGHTS;
	  }
      D3DPRINT(NORMAL_DBG_LEVEL, "      executing %ld lights, (%ld are active)", j, pRc->tl.lighting.dwNumActiveLights);

	  for(i = 0; i < j; ++i)
	  {
		if (IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pSB, i, 0) )
		{
			// Create a light by building a command buffer
			BYTE cmdString[32];
			LPD3DHAL_DP2COMMAND pCmd = (LPD3DHAL_DP2COMMAND)&(cmdString[0]);
			D3DHAL_DP2CREATELIGHT *pCreateLight = (D3DHAL_DP2CREATELIGHT*)&(cmdString[sizeof(LPD3DHAL_DP2COMMAND)]);
			pCmd->wStateCount = 1;		// create 1 light
			pCreateLight->dwIndex = i;	// index of new light
			hr = DP2TL_CreateLight(pRc, pCmd);
			if (DD_OK != hr)
				return hr;
		}
		if (IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pSB, i, 1) )
		{
			// Enable or Disable the light
			if( pSB->uc.LightStates[i].enabled )
			{
				D3DPRINT(NORMAL_DBG_LEVEL, "      enableing light %d", i );
        		if ( LightEnable(GetIndexedLightPtr(pRc->tl.lighting.pLightArray, i), pRc) )
        			pRc->tl.dwDirtyFlags |= TLPV_DIRTY_SETLIGHT;
			}
			else
			{
				D3DPRINT(NORMAL_DBG_LEVEL, "      disableing light %d", i );
        		LightDisable(GetIndexedLightPtr(pRc->tl.lighting.pLightArray, i), pRc);
			}
		}
		if (IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pSB, i, 2) )
		{
			LPD3DLIGHT7 pD3DLight = (LPD3DLIGHT7)&pSB->uc.LightStates[i].lData;
			TLLIGHT *pTlLight = GetIndexedLightPtr(pRc->tl.lighting.pLightArray, i);
			if( memcmp(pTlLight, pD3DLight, sizeof(LPD3DLIGHT7)) != 0 )	// only update if it really changed
			{
			  // Copy this data into the light
			  D3DPRINT(NORMAL_DBG_LEVEL, "      updating data for light %d from %lXh", i, (DWORD)pD3DLight );
			  hr = SetLight( pRc, pTlLight, pD3DLight );
        	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_SETLIGHT;
			  if (DD_OK != hr)
				return hr;
			}
		}

      }

	  // Material
	  if( IS_STATEBLOCK_MATERIAL_FLAG_SET(pSB) )
      {
	    if(memcmp(&pRc->tl.lighting.Material, &pSB->uc.MaterialState, sizeof(D3DMATERIAL7)) != 0)	// don't load if it didn't change
	    {
		D3DPRINT(NORMAL_DBG_LEVEL, "      updating material from %lXh", (DWORD)&(pSB->uc.MaterialState) );
		pRc->tl.lighting.Material = pSB->uc.MaterialState;
		  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_MATERIAL;
	    }
      }

	  // Transformation matricies
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, TLTRANSFORMSTATE_WORLD) )
      {
	    if(memcmp(&(pRc->tl.xfmWorld[0]), &(pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD]), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
	    {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      updating world xform from %lXh", (DWORD)&(pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD]) );
	    pRc->tl.xfmWorld[0] = pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD];
		pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLDXFM;
      }
      }
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, TLTRANSFORMSTATE_WORLD1) )
      {
	    if(memcmp(&(pRc->tl.xfmWorld[1]), &(pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD1]), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
	    {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      updating world1 xform from %lXh", (DWORD)&(pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD1]) );
	    pRc->tl.xfmWorld[1] = pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD1];
		pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLD1XFM;
		}
      }
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, TLTRANSFORMSTATE_WORLD2) )
      {
	    if(memcmp(&(pRc->tl.xfmWorld[2]), &(pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD2]), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
      {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      updating world2 xform from %lXh", (DWORD)&(pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD2]) );
	    pRc->tl.xfmWorld[2] = pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD2];
		pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLD2XFM;
      }
      }
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, TLTRANSFORMSTATE_WORLD3) )
      {
	    if(memcmp(&(pRc->tl.xfmWorld[3]), &(pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD3]), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
      {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      updating world3 xform from %lXh", (DWORD)&(pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD3]) );
	    pRc->tl.xfmWorld[3] = pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD3];
		pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLD3XFM;
      }
      }
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, TLTRANSFORMSTATE_VIEW) )
      {
	    if(memcmp(&(pRc->tl.xfmView), &(pSB->uc.TransformationStates[TLTRANSFORMSTATE_VIEW]), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
	    {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      updating viewport xform from %lXh", (DWORD)&(pSB->uc.TransformationStates[TLTRANSFORMSTATE_VIEW]) );
	    pRc->tl.xfmView = pSB->uc.TransformationStates[TLTRANSFORMSTATE_VIEW];
		pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VIEWXFM;
		}
      }
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, TLTRANSFORMSTATE_PROJ) )
      {
	    if(memcmp(&(pRc->tl.xfmProj), &(pSB->uc.TransformationStates[TLTRANSFORMSTATE_PROJ]), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
      {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      updating projection xform from %lXh", (DWORD)&(pSB->uc.TransformationStates[TLTRANSFORMSTATE_PROJ]) );
	    pRc->tl.xfmProj = pSB->uc.TransformationStates[TLTRANSFORMSTATE_PROJ];
		pRc->tl.dwDirtyFlags |= TLPV_DIRTY_PROJXFM;
      }
      }
      for(i=0; i<D3DDP_MAXTEXCOORD; ++i)
	  {
        if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, (TLTRANSFORMSTATE_TEX0+i)) )
	    {
	      if(memcmp(&(pRc->tl.xfmTxtr[i]), &(pSB->uc.TransformationStates[(TLTRANSFORMSTATE_TEX0+i)]), sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
	    {
	      D3DPRINT(NORMAL_DBG_LEVEL, "      updating texture %d xform from %lXh", i, (DWORD)&(pSB->uc.TransformationStates[(TLTRANSFORMSTATE_TEX0+i)]) );
	      pRc->tl.xfmTxtr[i] = pSB->uc.TransformationStates[(TLTRANSFORMSTATE_TEX0+i)];
		  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_TXTRXFM;
		  pRc->tl.dwDirtyXfmTxtr |= (1 << i);
	    }
	  }
	  }

	  // Z-Range
	  if( IS_STATEBLOCK_ZRANGE_FLAG_SET(pSB) )
	  {
		if( ( (*(DWORD*)&pRc->tl.Viewport.dvMinZ) != (*(DWORD*)&pSB->uc.ZRangeState.dvMinZ) ) ||	// only update this if it's dirty
		    ( (*(DWORD*)&pRc->tl.Viewport.dvMaxZ) != (*(DWORD*)&pSB->uc.ZRangeState.dvMaxZ) ) )
		{
	    D3DPRINT(NORMAL_DBG_LEVEL, "      updating z-range from %lXh", (DWORD)&( pSB->uc.ZRangeState) );
	    pRc->tl.Viewport.dvMinZ = pSB->uc.ZRangeState.dvMinZ;
	    pRc->tl.Viewport.dvMaxZ = pSB->uc.ZRangeState.dvMaxZ;
		pRc->tl.dwDirtyFlags |= TLPV_DIRTY_ZRANGE;
		}
	  }

	  // User Clip Planes
      for(i=0; i<TLMAX_USER_CLIPPLANES; ++i)
	  {															
        if( IS_STATEBLOCK_CLIPPLANE_FLAG_SET( pSB, i ))
	    {
	      D3DPRINT(NORMAL_DBG_LEVEL, "      updating user clip plane %d at %lXh from %lXh", 
	      		i, (DWORD)&(pRc->tl.UserClipPlanes[i]), (DWORD)&(pSB->uc.ClipPlaneStates[i]) );
	      D3DPRINT(NORMAL_DBG_LEVEL, "        x=%lXh  y=%lXh  z=%lXh  z=%lXh",
	      		pSB->uc.ClipPlaneStates[i].x, pSB->uc.ClipPlaneStates[i].y, 
	      		pSB->uc.ClipPlaneStates[i].z, pSB->uc.ClipPlaneStates[i].w );
	      pRc->tl.UserClipPlanes[i] = pSB->uc.ClipPlaneStates[i];
		  {
	      	pRc->tl.UserClipPlanes[i] = pSB->uc.ClipPlaneStates[i];
		  	pRc->tl.dwDirtyFlags |= TLPV_DIRTY_CLIPPLANES;
		  }
	    }
      }

#endif //TnL_HAL
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
      BYTE	  *pNextData;	// ptr to the next field in the compressed data area

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
              ((LPDWORD)&pRc->textureStage[stage])[D3DTSS_ADDRESS] = data;
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

	  // Initalize the pointer to the data area
      pNextData = (BYTE*) &pSB->cc.pair[pSB->cc.numPairs];

	  // Viewport info
	  if(pSB->cc.dwSizeStoredViewport)
	  {
        SETUP_PPDEV ( pRc )         // initializes ppdev
		D3DHAL_DP2VIEWPORTINFO	*ViewportInfoState = (D3DHAL_DP2VIEWPORTINFO*)pNextData;
		DWORD dwX		= ViewportInfoState->dwX;
		DWORD dwY		= ViewportInfoState->dwY;
		DWORD dwWidth	= ViewportInfoState->dwWidth;
		DWORD dwHeight	= ViewportInfoState->dwHeight;

	    // viewports change every version of DX
	    D3DPRINT(NORMAL_DBG_LEVEL, "  updating viewport from %lXh", (DWORD)ViewportInfoState );
	    D3DPRINT(NORMAL_DBG_LEVEL, "X=%d  Y=%d  Width=%d  Height=%d", dwX, dwY, dwWidth, dwHeight);
#ifdef TnL_HAL
        // Update T&L viewport state IF it really changed
		if( (pRc->tl.Viewport.dwX != dwX) ||
            (pRc->tl.Viewport.dwY != dwY) ||
            (pRc->tl.Viewport.dwWidth != dwWidth) ||
            (pRc->tl.Viewport.dwHeight != dwHeight) )
	    {
	      pRc->tl.Viewport.dwX = dwX;
	      pRc->tl.Viewport.dwY = dwY;
	      pRc->tl.Viewport.dwWidth = dwWidth;
	      pRc->tl.Viewport.dwHeight = dwHeight;
		  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VIEWRECT;
		}
#endif

        if (IS_NAPALM) 
        {
			if ((dwX+dwWidth)%2==1)     /* Adjust right to even boundary */
				dwWidth += 1;
			if ((dwX+dwWidth) > 0xffe)  /* clipRight1 must <= 0xffe */
				dwWidth = 0xffe-dwX;
			if ((dwY+dwHeight) > 0xfff) /* clipBottom1 must <= 0xfff */
                dwHeight = 0xfff-dwY;

			/* the left must be even - Taken care of by dwX&0xffe below*/
			pRc->sst.clipLeftRight1 = ((dwX&0xffe)<<16) | (((dwX+dwWidth)&0xffe)<<0);      //Left<Right
			pRc->sst.clipBottomTop1 = (((dwY+dwHeight)&0xfff)<<0) |  (((dwY)&0xfff)<<16);//Top<Bottom
			D3DPRINT(255,"NAPALM VIEWPORTINFO %08x %08x %08x %08x", dwX,dwY,dwWidth,dwHeight );
		}
#ifdef NEW_CLIP_FOR_GB
		pRc->sst.clipLeftRight= (((dwX                )&0xfff)<<16) | (((dwX+dwWidth /*+1*/)&0xfff)<<0 ); //Take out +1 so bottom and right are exclusive of drawn pixels (not drawn).  
		pRc->sst.clipBottomTop= (((dwY+dwHeight /*+1*/)&0xfff)<<0 ) | (((dwY               )&0xfff)<<16);
#endif
        pNextData += sizeof(D3DHAL_DP2VIEWPORTINFO);
	  }

#ifdef TnL_HAL
      // Execute any necessary state for lights, materials, transforms,
      // viewport info, z range and clip planes

	  // Lights
	  dwFinalState += (pSB->cc.dwNumLightDataStates * 3);
      D3DPRINT(NORMAL_DBG_LEVEL, "  executing %d light stage states", pSB->cc.dwNumLightDataStates);
      for (; i < dwFinalState; i += 3)
      {
		DWORD dwLightIndex = pSB->cc.pair[i+0].dwType;

		// Create a light by building a command buffer
	    if(pSB->cc.pair[i+0].dwValue != -1)
		{	
			BYTE cmdString[32];
			LPD3DHAL_DP2COMMAND pCmd = (LPD3DHAL_DP2COMMAND)&(cmdString[0]);
			D3DHAL_DP2CREATELIGHT *pCreateLight = (D3DHAL_DP2CREATELIGHT*)&(cmdString[sizeof(LPD3DHAL_DP2COMMAND)]);
			pCmd->wStateCount = 1;		// create 1 light
			pCreateLight->dwIndex = dwLightIndex;	// index of new light
            D3DPRINT(NORMAL_DBG_LEVEL, "    creating light %d", dwLightIndex);
			hr = DP2TL_CreateLight(pRc, pCmd);
			if (DD_OK != hr)
				return hr;
		}

		// Enable or Disable the light
	    if(pSB->cc.pair[i+1].dwValue != -1)
		{	
			if( pSB->cc.pair[i+1].dwValue )
			{
				D3DPRINT(NORMAL_DBG_LEVEL, "      enableing light %d", dwLightIndex );
        		if ( LightEnable(GetIndexedLightPtr(pRc->tl.lighting.pLightArray, dwLightIndex), pRc) )
        			pRc->tl.dwDirtyFlags |= TLPV_DIRTY_SETLIGHT;
			}
			else
			{
				D3DPRINT(NORMAL_DBG_LEVEL, "      disableing light %d", dwLightIndex );
        		LightDisable(GetIndexedLightPtr(pRc->tl.lighting.pLightArray, dwLightIndex), pRc);
			}
		}

	    if(pSB->cc.pair[i+2].dwValue != 0)
		{
			// Copy this data into the light
			LPD3DLIGHT7 pD3DLight = (LPD3DLIGHT7)pNextData;
			TLLIGHT *pTlLight = GetIndexedLightPtr(pRc->tl.lighting.pLightArray, dwLightIndex);
			if( memcmp(pTlLight, pD3DLight, sizeof(LPD3DLIGHT7)) != 0 )	// only update if it really changed
			{
			  D3DPRINT(NORMAL_DBG_LEVEL, "      updating data for light %d from %lXh", dwLightIndex, pD3DLight );
			  hr = SetLight( pRc, pTlLight, pD3DLight );
        	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_SETLIGHT;
			  if (DD_OK != hr)
				return hr;
			}
		    pNextData += sizeof(D3DLIGHT7);
		}
	  }

	  // Material
	  if(pSB->cc.dwSizeStoredMAT)
	  {
	    if(memcmp(&pRc->tl.lighting.Material, pNextData, sizeof(D3DMATERIAL7)) != 0)	// don't load if it didn't change
		{
		  D3DPRINT(NORMAL_DBG_LEVEL, "  updating material from %lXh", (DWORD)pNextData );
          memcpy(&pRc->tl.lighting.Material, pNextData, sizeof(D3DMATERIAL7));
		  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_MATERIAL;
		}
	    pNextData += sizeof(D3DMATERIAL7);
	  }

	  // Transformation matricies
	  dwFinalState += pSB->cc.dwNumXforms;
      for (; i < dwFinalState; i++)
	  {
		switch(pSB->cc.pair[i].dwType)
		{
		  case TLTRANSFORMSTATE_WORLD:
		  {
	        if(memcmp(&(pRc->tl.xfmWorld[0]), pNextData, sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
	        {
	          D3DPRINT(NORMAL_DBG_LEVEL, "  updating world xform from %lXh", (DWORD)pNextData );
              memcpy(&(pRc->tl.xfmWorld[0]), pNextData, sizeof(D3DMATRIX));
	    	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLDXFM;
			}
		    break;
		  }
		  case TLTRANSFORMSTATE_WORLD1:
		  {
	        if(memcmp(&(pRc->tl.xfmWorld[1]), pNextData, sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
	        {
	          D3DPRINT(NORMAL_DBG_LEVEL, "  updating world1 xform from %lXh", (DWORD)pNextData );
              memcpy(&(pRc->tl.xfmWorld[1]), pNextData, sizeof(D3DMATRIX));
	    	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLD1XFM;
			}
		    break;
		  }
		  case TLTRANSFORMSTATE_WORLD2:
		  {
	        if(memcmp(&(pRc->tl.xfmWorld[2]), pNextData, sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
	        {
	          D3DPRINT(NORMAL_DBG_LEVEL, "  updating world2 xform from %lXh", (DWORD)pNextData );
              memcpy(&(pRc->tl.xfmWorld[2]), pNextData, sizeof(D3DMATRIX));
	    	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLD2XFM;
			}
		    break;
		  }
		  case TLTRANSFORMSTATE_WORLD3:
		  {
	        if(memcmp(&(pRc->tl.xfmWorld[3]), pNextData, sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
	        {
	          D3DPRINT(NORMAL_DBG_LEVEL, "  updating world3 xform from %lXh", (DWORD)pNextData );
              memcpy(&(pRc->tl.xfmWorld[3]), pNextData, sizeof(D3DMATRIX));
	    	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_WORLD3XFM;
			}
		    break;
		  }
		  case TLTRANSFORMSTATE_VIEW:
		  {
	        if(memcmp(&(pRc->tl.xfmView), pNextData, sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
	        {
	          D3DPRINT(NORMAL_DBG_LEVEL, "  updating view xform from %lXh", (DWORD)pNextData );
              memcpy(&pRc->tl.xfmView, pNextData, sizeof(D3DMATRIX));
	    	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VIEWXFM;
			}
		    break;
		  }
		  case TLTRANSFORMSTATE_PROJ:
		  {
	        if(memcmp(&(pRc->tl.xfmView), pNextData, sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
	        {
	          D3DPRINT(NORMAL_DBG_LEVEL, "  updating projection xform from %lXh", (DWORD)pNextData );
              memcpy(&pRc->tl.xfmProj, pNextData, sizeof(D3DMATRIX));
	    	  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_PROJXFM;
			}
		    break;
		  }
		  case TLTRANSFORMSTATE_TEX0:
		  case TLTRANSFORMSTATE_TEX1:
		  case TLTRANSFORMSTATE_TEX2:
		  case TLTRANSFORMSTATE_TEX3:
		  case TLTRANSFORMSTATE_TEX4:
		  case TLTRANSFORMSTATE_TEX5:
		  case TLTRANSFORMSTATE_TEX6:
		  case TLTRANSFORMSTATE_TEX7:
	      {
		    DWORD xfmTxtrIndex = pSB->cc.pair[i].dwType - TLTRANSFORMSTATE_TEX0;
	        if(memcmp(&(pRc->tl.xfmTxtr[xfmTxtrIndex]), pNextData, sizeof(D3DMATRIX)) != 0)	// don't load if it didn't change
	        {
              D3DPRINT(NORMAL_DBG_LEVEL, "  updating texture %d xform from %lXh", xfmTxtrIndex, (DWORD)pNextData );
              memcpy(&pRc->tl.xfmTxtr[xfmTxtrIndex], pNextData, sizeof(D3DMATRIX));
		      pRc->tl.dwDirtyFlags |= TLPV_DIRTY_TXTRXFM;
		      pRc->tl.dwDirtyXfmTxtr |= (xfmTxtrIndex << i);
			}
		    break;
    	  }

		  default:
		  {
	        D3DPRINT(NORMAL_DBG_LEVEL, "  UNKNOWN xform from %lXh, nothing updated!", pSB->cc.pair[i].dwType );
		    break;
		  }

		}
	    pNextData += sizeof(D3DMATRIX);
	  }

	  // Z-Range
	  if(pSB->cc.dwSizeStoredZRange)
	  {
	    D3DHAL_DP2ZRANGE *ZRangeState = (D3DHAL_DP2ZRANGE*)pNextData;
		if( ( (*(DWORD*)&pRc->tl.Viewport.dvMinZ) != (*(DWORD*)&ZRangeState->dvMinZ) ) ||	// only update this if it's dirty
		    ( (*(DWORD*)&pRc->tl.Viewport.dvMaxZ) != (*(DWORD*)&ZRangeState->dvMaxZ) ) )
		{
	      D3DPRINT(NORMAL_DBG_LEVEL, "      updating z-range from %lXh", (DWORD)ZRangeState );
	      pRc->tl.Viewport.dvMinZ = ZRangeState->dvMinZ;
	      pRc->tl.Viewport.dvMaxZ = ZRangeState->dvMaxZ;
		  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_ZRANGE;
		}
        pNextData += sizeof(D3DHAL_DP2ZRANGE);
	  }

      // User Clip Planes
	  dwFinalState += pSB->cc.dwNumUserClipplanes;
      for (; i < dwFinalState; i++)
	  {
		DWORD clipPlaneIndex = pSB->cc.pair[i].dwType;
		TLVECTOR4 *pClipPlane = (TLVECTOR4*)pNextData;
		if (memcmp( &pRc->tl.UserClipPlanes[clipPlaneIndex], pClipPlane, sizeof(TLVECTOR4)) != 0)	// did they really change?
		{
	      D3DPRINT(NORMAL_DBG_LEVEL, "  updating user clip plane %d at %lXh from %lXh", 
	      		clipPlaneIndex, (DWORD)&(pRc->tl.UserClipPlanes[clipPlaneIndex]), (DWORD)pClipPlane );
          memcpy(&pRc->tl.UserClipPlanes[clipPlaneIndex], pClipPlane, sizeof(TLVECTOR4));
		  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_CLIPPLANES;
		}
        pNextData += sizeof(TLVECTOR4);
	  }

#endif //TnL_HAL
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
      // DX 8 Stream Support (Single stream only)

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
  BYTE		  *pNextData;

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
    FIELDOFFSET(RC, blendEnable),           //   D3DRENDERSTATE_BLENDENABLE            = 27,
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
#ifdef TnL_HAL
    FIELDOFFSET(RC, tl.lighting.fog_range_enable), //   D3DRENDERSTATE_RANGEFOGENABLE  = 48,
#else
    -1,                                     //   D3DRENDERSTATE_RANGEFOGENABLE         = 48,
#endif
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
#ifdef TnL_HAL
    FIELDOFFSET(RC, tl.StateClipping),      //   D3DRENDERSTATE_CLIPPING               = 136,
    FIELDOFFSET(RC, tl.StateLighting),      //   D3DRENDERSTATE_LIGHTING               = 137,
    -1,                                     //   D3DRENDERSTATE_EXTENTS                = 138,
    FIELDOFFSET(RC, tl.lighting.ambient_save), //   D3DRENDERSTATE_AMBIENT             = 139,
    FIELDOFFSET(RC, tl.lighting.fog_mode),  //   D3DRENDERSTATE_FOGVERTEXMODE          = 140,
    FIELDOFFSET(RC, tl.StateColorVertex),   //   D3DRENDERSTATE_COLORVERTEX            = 141,
    FIELDOFFSET(RC, tl.StateLocalViewer),   //   D3DRENDERSTATE_LOCALVIEWER            = 142,
    FIELDOFFSET(RC, tl.StateNormNormals),   //   D3DRENDERSTATE_NORMALIZENORMALS       = 143,
    -1,                                     //   D3DRENDERSTATE_COLORKEYBLENDENABLE    = 144,
    FIELDOFFSET(RC, tl.lighting.DfusMaterialSrc),    //   D3DRENDERSTATE_DIFFUSEMATERIALSOURCE  = 145,
    FIELDOFFSET(RC, tl.lighting.SpecMaterialSrc),    //   D3DRENDERSTATE_SPECULARMATERIALSOURCE = 146,
    FIELDOFFSET(RC, tl.lighting.AmbMaterialSrc),     //   D3DRENDERSTATE_AMBIENTMATERIALSOURCE  = 147,
    FIELDOFFSET(RC, tl.lighting.EmisMaterialSrc),    //   D3DRENDERSTATE_EMISSIVEMATERIALSOURCE = 148,
    -1,                                     //   D3DRENDERSTATE_ALPHASOURCE            = 149,
    -1,                                     //   D3DRENDERSTATE_FOGFACTORSOURCE        = 150,
    FIELDOFFSET(RC, tl.StateVertexBlends),  //   D3DRENDERSTATE_VERTEXBLEND            = 151,
    FIELDOFFSET(RC, tl.clipPlaneEnable),    //   D3DRENDERSTATE_CLIPPLANEENABLE        = 152,
#else
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
#endif
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
        if (IS_STATEBLOCK_RENDERSTATE_FLAG_SET(pSB, i))
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
          if (IS_STATEBLOCK_TEXSTAGESTATE_FLAG_SET(pSB, j, i))
          {
            data = ((LPDWORD)&pRc->textureStage[j])[i];

            pSB->uc.TssStates[j][i] = data;

            D3DPRINT(NORMAL_DBG_LEVEL, "        stage=%ld, state=%ld, data=%ld", j, i, pSB->uc.TssStates[j][i]);
          }
        }
      }

	  // viewport info
	  if( IS_STATEBLOCK_VIEWPORT_FLAG_SET(pSB) )
	  {
#ifdef TnL_HAL
	    // viewports change every version of DX
	    D3DPRINT(NORMAL_DBG_LEVEL, "      capturing viewport from %lXh", (DWORD)&pRc->tl.Viewport );
	    D3DPRINT(NORMAL_DBG_LEVEL, "X=%d  Y=%d  Width=%d  Height=%d", 
	    	pRc->tl.Viewport.dwX, pRc->tl.Viewport.dwY, pRc->tl.Viewport.dwWidth, pRc->tl.Viewport.dwHeight);
	    pSB->uc.ViewportInfoState.dwX = pRc->tl.Viewport.dwX;
	    pSB->uc.ViewportInfoState.dwY = pRc->tl.Viewport.dwY;
	    pSB->uc.ViewportInfoState.dwWidth = pRc->tl.Viewport.dwWidth;
	    pSB->uc.ViewportInfoState.dwHeight = pRc->tl.Viewport.dwHeight;
#endif
	  }

#ifdef TnL_HAL
      // Capture t&l state for lights, materials, transforms,
      // viewport info, z range and clip planes
	  j = pRc->tl.lighting.dwLightArraySize;
	  if (j > TLMAX_LIGHTS)
	  {
      	D3DPRINT(NORMAL_DBG_LEVEL, "BAD: More lights than we've allocated capture states for!!!");
      	D3DPRINT(NORMAL_DBG_LEVEL, "     Allocated space for %ld, Total lights = %ld", TLMAX_LIGHTS, j);
		j = TLMAX_LIGHTS;
	  }
      D3DPRINT(NORMAL_DBG_LEVEL, "      capturing %ld lights, (%ld are active)", j, pRc->tl.lighting.dwNumActiveLights);

	  for(i = 0; i < j; ++i)
	  {
	    TLLIGHT *pTlLight = GetIndexedLightPtr(pRc->tl.lighting.pLightArray, i);
		if (IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pSB, i, 0) )
		  pSB->uc.LightStates[i].create = TRUE;
		if (IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pSB, i, 1) )
		  pSB->uc.LightStates[i].enabled = (pTlLight->dwFlags & TLLIGHT_ENABLED) ? TRUE : FALSE;
		if (IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pSB, i, 2) )
		{
		  // we're copying a TLLIGHT into a D3DLIGHT7 (which is much smaller) so we can't just memcpy
		  LPD3DLIGHT7 pD3DLight = (LPD3DLIGHT7)&pSB->uc.LightStates[i].lData;
		  pD3DLight->dltType		= pTlLight->dltType;
		  pD3DLight->dcvDiffuse		= pTlLight->Ld;
		  pD3DLight->dcvSpecular	= pTlLight->Ls;
		  pD3DLight->dcvAmbient		= pTlLight->La;
		  pD3DLight->dvPosition		= pTlLight->Position;
		  pD3DLight->dvDirection	= pTlLight->Direction;
		  pD3DLight->dvRange		= pTlLight->Range;
		  pD3DLight->dvFalloff		= pTlLight->Falloff;
		  pD3DLight->dvAttenuation0	= pTlLight->Attenuation0;
		  pD3DLight->dvAttenuation1	= pTlLight->Attenuation1;
		  pD3DLight->dvAttenuation2	= pTlLight->Attenuation2;
		  pD3DLight->dvTheta		= pTlLight->Theta;
		  pD3DLight->dvPhi			= pTlLight->Phi;
		}
		if ( IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pSB, i, 0) || IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pSB, i, 1) || IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pSB, i, 2) )
		{
	      D3DPRINT(NORMAL_DBG_LEVEL, "Light %ld at %lXh  Type = %ld   %s", 
    	   	i, (DWORD)pTlLight, pTlLight->dltType, (pTlLight->dwFlags & TLLIGHT_ENABLED) ? "Enabled" : "Disabled" );
		}
      }

	  // Material
	  if( IS_STATEBLOCK_MATERIAL_FLAG_SET(pSB) )
	  {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      capturing material from %lXh", (DWORD)&pRc->tl.lighting.Material );
	    pSB->uc.MaterialState = pRc->tl.lighting.Material;
	  }

	  // Transformation matricies
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, TLTRANSFORMSTATE_WORLD) )
	  {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      capturing world xform from %lXh", (DWORD)&pRc->tl.xfmWorld[0] );
	    pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD] = pRc->tl.xfmWorld[0];
	  }
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, TLTRANSFORMSTATE_WORLD1) )
	  {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      capturing world1 xform from %lXh", (DWORD)&pRc->tl.xfmWorld[1] );
	    pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD1] = pRc->tl.xfmWorld[1];
	  }
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, TLTRANSFORMSTATE_WORLD2) )
	  {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      capturing world2 xform from %lXh", (DWORD)&pRc->tl.xfmWorld[2] );
	    pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD2] = pRc->tl.xfmWorld[2];
	  }
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, TLTRANSFORMSTATE_WORLD3) )
	  {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      capturing world3 xform from %lXh", (DWORD)&pRc->tl.xfmWorld[3] );
	    pSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD3] = pRc->tl.xfmWorld[3];
	  }
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, TLTRANSFORMSTATE_VIEW) )
	  {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      capturing viewport xform from %lXh", (DWORD)&pRc->tl.xfmView );
	    pSB->uc.TransformationStates[TLTRANSFORMSTATE_VIEW] = pRc->tl.xfmView;
	  }
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, TLTRANSFORMSTATE_PROJ) )
	  {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      capturing projection xform from %lXh", (DWORD)&pRc->tl.xfmProj );
	    pSB->uc.TransformationStates[TLTRANSFORMSTATE_PROJ] = pRc->tl.xfmProj;
	  }
      for(i=0; i<D3DDP_MAXTEXCOORD; ++i)
	  {
        if( IS_STATEBLOCK_XFORM_FLAG_SET( pSB, (TLTRANSFORMSTATE_TEX0+i)) )
	    {
	      D3DPRINT(NORMAL_DBG_LEVEL, "      capturing texture %d xform from %lXh", i, (DWORD)&pRc->tl.xfmTxtr[i] );
	      pSB->uc.TransformationStates[(TLTRANSFORMSTATE_TEX0+i)] = pRc->tl.xfmTxtr[i];
	    }
	  }

	  // Z-Range
	  if( IS_STATEBLOCK_ZRANGE_FLAG_SET(pSB) )
	  {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      capturing z-range from viewport at %lXh", (DWORD)&pRc->tl.Viewport );
	    D3DPRINT(NORMAL_DBG_LEVEL, "        MinZ=%lXh  MaxZ=%lXh", pRc->tl.Viewport.dvMinZ, pRc->tl.Viewport.dvMaxZ );
	    pSB->uc.ZRangeState.dvMinZ = pRc->tl.Viewport.dvMinZ;
	    pSB->uc.ZRangeState.dvMaxZ = pRc->tl.Viewport.dvMaxZ;
	  }

	  // User Clip Planes
      for(i=0; i<TLMAX_USER_CLIPPLANES; ++i)
	  {
        if( IS_STATEBLOCK_CLIPPLANE_FLAG_SET( pSB, i ))
	    {
	      D3DPRINT(NORMAL_DBG_LEVEL, "      capturing user clip plane %d from %lXh", 
	      	i, (DWORD)&pRc->tl.UserClipPlanes[i] );
	      D3DPRINT(NORMAL_DBG_LEVEL, "        x=%lXh  y=%lXh  z=%lXh  z=%lXh",
	      		pRc->tl.UserClipPlanes[i].x, pRc->tl.UserClipPlanes[i].y, 
	      		pRc->tl.UserClipPlanes[i].z, pRc->tl.UserClipPlanes[i].w );
	      pSB->uc.ClipPlaneStates[i] = pRc->tl.UserClipPlanes[i];
	    }
      }
#endif //TnL_HAL
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
      // DX 8 Stream Support (Single stream only)

      if (IS_STATEBLOCK_STREAM_FLAG_SET(pSB, 0))
      {
        D3DPRINT(NORMAL_DBG_LEVEL, "      capturing stream source, stream 0");
        D3DPRINT(NORMAL_DBG_LEVEL, "        Stream=%lxh, VBHandle=%lxh, Stride=%lxh", 0, pRc->dwVBHandle, pRc->dwVerticesStride);

        pSB->uc.StreamSource[0].dwStream = 0;
        pSB->uc.StreamSource[0].dwVBHandle = pRc->dwVBHandle;
        pSB->uc.StreamSource[0].dwStride = pRc->dwVerticesStride << 2;
      }
      if (IS_STATEBLOCK_STREAM_FLAG_SET(pSB, 8))
      {
        D3DPRINT(NORMAL_DBG_LEVEL, "      capturing stream indices");
        D3DPRINT(NORMAL_DBG_LEVEL, "        IndexHandle=%lxh, Stride=%lxh", pRc->dwIndexHandle, pRc->dwIndicesStride);

        pSB->uc.StreamIndices.dwVBHandle = pRc->dwIndexHandle;
        pSB->uc.StreamIndices.dwStride = pRc->dwIndicesStride;
      }
      if (IS_STATEBLOCK_STREAM_FLAG_SET(pSB, 9))
      {
        D3DPRINT(NORMAL_DBG_LEVEL, "      capturing vertex shader handle");
        D3DPRINT(NORMAL_DBG_LEVEL, "        ShaderHandle=%lxh", pRc->dwShaderHandle);

        pSB->uc.dwShaderHandle = pRc->dwShaderHandle;
      }
#endif // DX 8
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

	  // Initalize the pointer to the data area
      pNextData = (BYTE*) &pSB->cc.pair[pSB->cc.numPairs];

	  // viewport info
	  if(pSB->cc.dwSizeStoredViewport)
	  {
#ifdef TnL_HAL
	    LPD3DHAL_DP2VIEWPORTINFO pViewPort = (LPD3DHAL_DP2VIEWPORTINFO)pNextData;
	    D3DPRINT(NORMAL_DBG_LEVEL, "      capturing viewport info state");
	    pViewPort->dwX = pRc->tl.Viewport.dwX;
	    pViewPort->dwY = pRc->tl.Viewport.dwY;
	    pViewPort->dwWidth = pRc->tl.Viewport.dwWidth;
	    pViewPort->dwHeight = pRc->tl.Viewport.dwHeight;
		pNextData += sizeof(D3DHAL_DP2VIEWPORTINFO);
#endif
	  }

#ifdef TnL_HAL
      // Capture any necessary state for lights, materials, transforms,
      // viewport info, z range and clip planes - here -

	  dwFinalState += (pSB->cc.dwNumLightDataStates * 3);
      D3DPRINT(NORMAL_DBG_LEVEL, "      capturing %ld compressed lights", pSB->cc.dwNumLightDataStates);

      for (; i < dwFinalState; i += 3)
      {
		DWORD dwIndex = pSB->cc.pair[i+0].dwType;
	    TLLIGHT *pTlLight = GetIndexedLightPtr(pRc->tl.lighting.pLightArray, dwIndex);

		// create light
	    // if(pSB->cc.pair[i+0].dwValue != -1)	
		// Since you can't create the same light more than once, so this state can't updated

		// Enable or Disable the light
	    if(pSB->cc.pair[i+1].dwValue != -1)
		  pSB->cc.pair[i+1].dwValue = (pTlLight->dwFlags & TLLIGHT_ENABLED) ? TRUE : FALSE;

		// Copy this data into the light
	    if(pSB->cc.pair[i+2].dwValue != 0)
		{
			LPD3DLIGHT7 pD3DLight = (LPD3DLIGHT7)pNextData;
			D3DPRINT(NORMAL_DBG_LEVEL, "      capturing data for light %d from %lXh", dwIndex, pTlLight );
			// we're copying a TLLIGHT into a D3DLIGHT7 (which is much smaller) so we can't just memcpy
			pD3DLight->dltType			= pTlLight->dltType;
			pD3DLight->dcvDiffuse		= pTlLight->Ld;
			pD3DLight->dcvSpecular		= pTlLight->Ls;
			pD3DLight->dcvAmbient		= pTlLight->La;
			pD3DLight->dvPosition		= pTlLight->Position;
			pD3DLight->dvDirection		= pTlLight->Direction;
			pD3DLight->dvRange			= pTlLight->Range;
			pD3DLight->dvFalloff		= pTlLight->Falloff;
			pD3DLight->dvAttenuation0	= pTlLight->Attenuation0;
			pD3DLight->dvAttenuation1	= pTlLight->Attenuation1;
			pD3DLight->dvAttenuation2	= pTlLight->Attenuation2;
			pD3DLight->dvTheta			= pTlLight->Theta;
			pD3DLight->dvPhi			= pTlLight->Phi;
		    pNextData += sizeof(D3DLIGHT7);
		}
	  }

	  // Material
	  if(pSB->cc.dwSizeStoredMAT)
	  {
	    LPD3DMATERIAL7 pMat = (LPD3DMATERIAL7)pNextData;
	    D3DPRINT(NORMAL_DBG_LEVEL, "      capturing material from %lXh", (DWORD)&pRc->tl.lighting.Material );
        memcpy(pMat, &pRc->tl.lighting.Material, sizeof(D3DMATERIAL7));
		pNextData += sizeof(D3DMATERIAL7);
	  }

	  // Transformation matricies
	  dwFinalState += pSB->cc.dwNumXforms;
      for (; i < dwFinalState; i++)
	  {
	    DWORD dwXformType = pSB->cc.pair[i].dwType;
	    switch(dwXformType)
		{
		  case TLTRANSFORMSTATE_WORLD:
		  {
			D3DPRINT(NORMAL_DBG_LEVEL, "  capturing world transformation");
			memcpy(pNextData, &pRc->tl.xfmWorld[0], sizeof(D3DMATRIX));
		    break;
		  }
		  case TLTRANSFORMSTATE_WORLD1:
		  {
			D3DPRINT(NORMAL_DBG_LEVEL, "  capturing world1 transformation");
			memcpy(pNextData, &pRc->tl.xfmWorld[1], sizeof(D3DMATRIX));
		    break;
		  }
		  case TLTRANSFORMSTATE_WORLD2:
		  {
			D3DPRINT(NORMAL_DBG_LEVEL, "  capturing world2 transformation");
			memcpy(pNextData, &pRc->tl.xfmWorld[2], sizeof(D3DMATRIX));
		    break;
		  }
		  case TLTRANSFORMSTATE_WORLD3:
		  {
			D3DPRINT(NORMAL_DBG_LEVEL, "  capturing world3 transformation");
			memcpy(pNextData, &pRc->tl.xfmWorld[3], sizeof(D3DMATRIX));
		    break;
		  }
		  case TLTRANSFORMSTATE_VIEW:
		  {
			D3DPRINT(NORMAL_DBG_LEVEL, "  capturing view transformation");
			memcpy(pNextData, &pRc->tl.xfmView, sizeof(D3DMATRIX));
		    break;
		  }
		  case TLTRANSFORMSTATE_PROJ:
		  {
			D3DPRINT(NORMAL_DBG_LEVEL, "  capturing projection transformation");
			memcpy(pNextData, &pRc->tl.xfmProj, sizeof(D3DMATRIX));
		    break;
		  }
		  case TLTRANSFORMSTATE_TEX0:
		  case TLTRANSFORMSTATE_TEX1:
		  case TLTRANSFORMSTATE_TEX2:
		  case TLTRANSFORMSTATE_TEX3:
		  case TLTRANSFORMSTATE_TEX4:
		  case TLTRANSFORMSTATE_TEX5:
		  case TLTRANSFORMSTATE_TEX6:
		  case TLTRANSFORMSTATE_TEX7:
		  {
		    DWORD dwTxXfmIndex = dwXformType - TLTRANSFORMSTATE_TEX0;
			D3DPRINT(NORMAL_DBG_LEVEL, "  capturing texture %d transformation", dwTxXfmIndex);
			memcpy(pNextData, &pRc->tl.xfmTxtr[dwTxXfmIndex], sizeof(D3DMATRIX));
		    break;
		  }
          default:
		    break;
		}

		pNextData += sizeof(D3DMATRIX);
	  }

	  // Z-Range
	  if(pSB->cc.dwSizeStoredZRange)
	  {
	    LPD3DHAL_DP2ZRANGE zRange = (LPD3DHAL_DP2ZRANGE)pNextData;
	    D3DPRINT(NORMAL_DBG_LEVEL, "      capturing z-range");
	    zRange->dvMinZ = pRc->tl.Viewport.dvMinZ;
	    zRange->dvMaxZ = pRc->tl.Viewport.dvMaxZ;
		pNextData += sizeof(D3DHAL_DP2ZRANGE);
	  }

	  // User Clip Planes
	  dwFinalState += pSB->cc.dwNumUserClipplanes;
      for (; i < dwFinalState; i++)
	  {
	    DWORD dwClipPlaneIndex = pSB->cc.pair[i].dwType;
	    D3DPRINT(NORMAL_DBG_LEVEL, "      copying user clip plane %d", dwClipPlaneIndex);
        memcpy(pNextData, &pRc->tl.UserClipPlanes[dwClipPlaneIndex], sizeof(TLVECTOR4));
        pNextData += sizeof(TLVECTOR4);
	  }

#endif  // TnL HAL
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
      // DX 8 Stream Support (Single stream only)

      if(pSB->cc.dwSizeSetStreamSource)
      {
        LPD3DHAL_DP2SETSTREAMSOURCE pSetStreamSource = (LPD3DHAL_DP2SETSTREAMSOURCE) pNextData;

        D3DPRINT(NORMAL_DBG_LEVEL, "      capturing stream source, stream 0");
        D3DPRINT(NORMAL_DBG_LEVEL, "        Stream=%lxh, VBHandle=%lxh, Stride=%lxh", 0, pRc->dwVBHandle, pRc->dwVerticesStride);

        pSetStreamSource->dwStream = 0;
        pSetStreamSource->dwVBHandle = pRc->dwVBHandle;
        pSetStreamSource->dwStride = pRc->dwVerticesStride << 2;

        pNextData += sizeof(D3DHAL_DP2SETSTREAMSOURCE);
      }
      if(pSB->cc.dwSizeSetIndices)
      {
        LPD3DHAL_DP2SETINDICES pSetIndices = (LPD3DHAL_DP2SETINDICES) pNextData;

        D3DPRINT(NORMAL_DBG_LEVEL, "      capturing stream indices");
        D3DPRINT(NORMAL_DBG_LEVEL, "        IndexHandle=%lxh, Stride=%lxh", pRc->dwIndexHandle, pRc->dwIndicesStride);

        pSetIndices->dwVBHandle = pRc->dwIndexHandle;
        pSetIndices->dwStride = pRc->dwIndicesStride;

        pNextData += sizeof(D3DHAL_DP2SETINDICES);
      }
      if(pSB->cc.dwSizeShaderHandle)
      {
        DWORD *pShaderHandle = (DWORD *) pNextData;

        D3DPRINT(NORMAL_DBG_LEVEL, "      capturing vertex shader handle");
        D3DPRINT(NORMAL_DBG_LEVEL, "        ShaderHandle=%lxh", pRc->dwShaderHandle);

        *pShaderHandle = pRc->dwShaderHandle;

        pNextData += sizeof(DWORD);
      }
#endif // DX 8
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
        DXFREE(pSB);
      }
    }

    // free state block table
    D3DPRINT(NORMAL_DBG_LEVEL, "  freeing ppSBTable=%lXh for pRc=%lXh", pRc->ppSBTable, pRc);
    DXFREE(pRc->ppSBTable);
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


  // If the current list is not large enough, we'll have to grow a new one
  if ((NULL == pRc->ppSBTable) || (dwHandle > (DWORD)pRc->ppSBTable[0]))
  {
    // New size of our state block table
    // (round up dwHandle in steps of LISTGROW)
    // we need to account for using index zero as a count of how many elements are in
    // the ppSBTable array, so add one to dwHandle
    dwNewSize = (((dwHandle + 1) + (LISTGROWSIZE - 1)) / LISTGROWSIZE) * LISTGROWSIZE;

    // we have to grow our list
    pNewSBTable = (STATEBLOCK **)DXMALLOCZ(dwNewSize*sizeof(STATEBLOCK *));
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

      // and get rid of it
      DXFREE(pRc->ppSBTable);
      D3DPRINT(NORMAL_DBG_LEVEL,"Freeing pRc=%lXh old ppSBTable[%X]",
               pRc, pRc->ppSBTable);
    }

    // New index table data
    pRc->ppSBTable = pNewSBTable;
    // store size in ppSBTable[0]
    // since we're using element zero to hold the array size
    // we only have dwNewSize-1 entries to store STATEBLOCK * elements in
    (DWORD)pRc->ppSBTable[0] = dwNewSize - 1;
  }

  // Store our state set pointer into our access list
  pRc->ppSBTable[dwHandle] = pSB;

  D3DPRINT(NORMAL_DBG_LEVEL,"Store state block, pRc=%lXh SBHandle=%ld pSB = %lXh",
           pRc, dwHandle, pSB);

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
  DWORD       dwSizeStoredViewport;
  BYTE		  *pNextData;
#ifdef TnL_HAL
  DWORD       dwNumLightDataStates;
  DWORD       dwSizeStoredMAT;
  DWORD       dwNumXforms;
  DWORD       dwSizeStoredZRange;
  DWORD		  dwNumUserClipplanes;
#endif

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
    if (IS_STATEBLOCK_RENDERSTATE_FLAG_SET(pUncompressedSB, i))
      dwCount++;
  }
  D3DPRINT(NORMAL_DBG_LEVEL, "  stored render state count = %ld", dwCount);

  for (j = 0; j < NUMTEXTUREUNITS+1; j++)
  {
    for (i = 0; i <= MAX_TEXTURESTAGESTATES; i++)
    {
      if (IS_STATEBLOCK_TEXSTAGESTATE_FLAG_SET(pUncompressedSB, j, i))
        dwCount++;
    }
    D3DPRINT(NORMAL_DBG_LEVEL, "  stored tss[%ld]+renderstate count = %ld", j, dwCount);
  }
  D3DPRINT(NORMAL_DBG_LEVEL, "  total stored tss + renderstate count = %ld", dwCount);

  if( IS_STATEBLOCK_VIEWPORT_FLAG_SET( pUncompressedSB ) )
    dwSizeStoredViewport = sizeof(D3DHAL_DP2VIEWPORTINFO);
  else
	dwSizeStoredViewport = 0;

#ifdef TnL_HAL
  for (j = 0, dwNumLightDataStates = 0; j < TLMAX_LIGHTS; j++)
  {
	if (IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pUncompressedSB, j, 0) ||	// create light
	    IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pUncompressedSB, j, 1) ||	// Enable or Disable
	    IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pUncompressedSB, j, 2) ) 	// data
	{
	  dwCount += 3;	// we'll store indicies for all three if any is needed
	  if (IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pUncompressedSB, j, 2))
	    dwNumLightDataStates++;
	}
  }

  if( IS_STATEBLOCK_MATERIAL_FLAG_SET(pUncompressedSB) )
	dwSizeStoredMAT = sizeof(D3DMATERIAL7);
  else
	dwSizeStoredMAT = 0;

  dwNumXforms = 0;
  if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, TLTRANSFORMSTATE_WORLD) )	dwNumXforms++;
  if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, TLTRANSFORMSTATE_WORLD1) )	dwNumXforms++;
  if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, TLTRANSFORMSTATE_WORLD2) )	dwNumXforms++;
  if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, TLTRANSFORMSTATE_WORLD3) )	dwNumXforms++;
  if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, TLTRANSFORMSTATE_VIEW) )	dwNumXforms++;
  if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, TLTRANSFORMSTATE_PROJ) )	dwNumXforms++;
  for(j = 0; j < D3DDP_MAXTEXCOORD; ++j)
    if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, (TLTRANSFORMSTATE_TEX0+j)) )	dwNumXforms++;
  dwCount += dwNumXforms;

  if( IS_STATEBLOCK_ZRANGE_FLAG_SET( pUncompressedSB ) )
    dwSizeStoredZRange = sizeof(D3DHAL_DP2ZRANGE);
  else
	dwSizeStoredZRange = 0;

  for(j = 0, dwNumUserClipplanes = 0; j < TLMAX_USER_CLIPPLANES; ++j)
    if( IS_STATEBLOCK_CLIPPLANE_FLAG_SET( pUncompressedSB, j )) 
      dwNumUserClipplanes++;
  dwCount += dwNumUserClipplanes;

  D3DPRINT(NORMAL_DBG_LEVEL, "  stored render state count w/t&l states = %ld", dwCount);
#endif //TnL_HAL

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

  // Calculate the size of the compressed state block
  dwSize = sizeof(pCompressedSB->cc) + (dwCount * sizeof(pCompressedSB->cc.pair[0])) + dwSizeStoredViewport;
#ifdef TnL_HAL
  dwSize += dwNumLightDataStates * sizeof(D3DLIGHT7) +  // data for each light
            dwSizeStoredMAT +                           // 0 if no material state
            dwNumXforms * sizeof(D3DMATRIX) +           // transformation matricies
            dwSizeStoredZRange +                        // 0 if no ZRange state
            dwNumUserClipplanes * sizeof(TLVECTOR4);    // user clip planes
#endif
#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
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

  pCompressedSB = (STATEBLOCK *)DXMALLOCZ(dwSize);
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
      if (IS_STATEBLOCK_RENDERSTATE_FLAG_SET(pUncompressedSB, i))
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
        if (IS_STATEBLOCK_TEXSTAGESTATE_FLAG_SET(pUncompressedSB, j, i))
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

    // Initalize the pointer to the data area
	pCompressedSB->cc.numPairs = dwCount;
	pNextData = (BYTE*) &pCompressedSB->cc.pair[dwCount];

	// viewport info
	pCompressedSB->cc.dwSizeStoredViewport = dwSizeStoredViewport;
	if(dwSizeStoredViewport)
	{
      D3DPRINT(NORMAL_DBG_LEVEL, "  copying viewport info state");
      memcpy(pNextData, &pUncompressedSB->uc.ViewportInfoState, sizeof(D3DHAL_DP2VIEWPORTINFO));
      pNextData += sizeof(D3DHAL_DP2VIEWPORTINFO);
	}

#ifdef TnL_HAL
	// Lights
	pCompressedSB->cc.dwNumLightDataStates = dwNumLightDataStates;
    D3DPRINT(NORMAL_DBG_LEVEL, "  compressing %d light stage states", dwNumLightDataStates);
    for (j = 0; j < TLMAX_LIGHTS; j++)
    {
      if (IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pUncompressedSB, j, 0) ||	// create light
	      IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pUncompressedSB, j, 1) ||	// Enable or Disable
	      IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pUncompressedSB, j, 2) ) 	// data
	  {
        D3DPRINT(NORMAL_DBG_LEVEL, "  compressing light %d", j);
		pCompressedSB->cc.pair[dwIndex+0].dwType = j; 
		if(IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pUncompressedSB, j, 0)) // create light
		  pCompressedSB->cc.pair[dwIndex+0].dwValue = pUncompressedSB->uc.bStoredLS[(j)/FLAGS_PER_DWORD][(0)] & (1 << ((j) % FLAGS_PER_DWORD));
		else
		  pCompressedSB->cc.pair[dwIndex+0].dwValue = -1;

		pCompressedSB->cc.pair[dwIndex+1].dwType = j;
		if(IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pUncompressedSB, j, 1)) // enable/disable
		  pCompressedSB->cc.pair[dwIndex+1].dwValue = pUncompressedSB->uc.bStoredLS[(j)/FLAGS_PER_DWORD][(1)] & (1 << ((j) % FLAGS_PER_DWORD));
		else
		  pCompressedSB->cc.pair[dwIndex+1].dwValue = -1;

		pCompressedSB->cc.pair[dwIndex+2].dwType = j;
		if(IS_STATEBLOCK_LIGHTSTATE_FLAG_SET(pUncompressedSB, j, 2)) // data
		{
		  pCompressedSB->cc.pair[dwIndex+2].dwValue = 1;
		  memcpy(pNextData, &pUncompressedSB->uc.LightStates[j].lData, sizeof(D3DLIGHT7));
		  pNextData += sizeof(D3DLIGHT7);
        }
		else
		{
		  pCompressedSB->cc.pair[dwIndex+2].dwValue = -1;
		}

        dwIndex += 3;
	  }
	}

	// Material
	pCompressedSB->cc.dwSizeStoredMAT = dwSizeStoredMAT;
	if(dwSizeStoredMAT)
	{
      D3DPRINT(NORMAL_DBG_LEVEL, "  copying material");
      memcpy(pNextData, &pUncompressedSB->uc.MaterialState, sizeof(D3DMATERIAL7));
      pNextData += sizeof(D3DMATERIAL7);
	}

	// Transformation matricies
	pCompressedSB->cc.dwNumXforms = dwNumXforms;
    if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, TLTRANSFORMSTATE_WORLD) )
	{
      D3DPRINT(NORMAL_DBG_LEVEL, "  copying world transformation");
      memcpy(pNextData, &pUncompressedSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD], sizeof(D3DMATRIX));
      pNextData += sizeof(D3DMATRIX);
	  pCompressedSB->cc.pair[dwIndex].dwType = TLTRANSFORMSTATE_WORLD; 
	  pCompressedSB->cc.pair[dwIndex].dwValue = 1;
	  dwIndex++;
	}
    if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, TLTRANSFORMSTATE_WORLD1) )
	{
      D3DPRINT(NORMAL_DBG_LEVEL, "  copying world1 transformation");
      memcpy(pNextData, &pUncompressedSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD1], sizeof(D3DMATRIX));
      pNextData += sizeof(D3DMATRIX);
	  pCompressedSB->cc.pair[dwIndex].dwType = TLTRANSFORMSTATE_WORLD1; 
	  pCompressedSB->cc.pair[dwIndex].dwValue = 1;
	  dwIndex++;
	}
    if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, TLTRANSFORMSTATE_WORLD2) )
	{
      D3DPRINT(NORMAL_DBG_LEVEL, "  copying world2 transformation");
      memcpy(pNextData, &pUncompressedSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD2], sizeof(D3DMATRIX));
      pNextData += sizeof(D3DMATRIX);
	  pCompressedSB->cc.pair[dwIndex].dwType = TLTRANSFORMSTATE_WORLD2; 
	  pCompressedSB->cc.pair[dwIndex].dwValue = 1;
	  dwIndex++;
	}
    if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, TLTRANSFORMSTATE_WORLD3) )
	{
      D3DPRINT(NORMAL_DBG_LEVEL, "  copying world3 transformation");
      memcpy(pNextData, &pUncompressedSB->uc.TransformationStates[TLTRANSFORMSTATE_WORLD3], sizeof(D3DMATRIX));
      pNextData += sizeof(D3DMATRIX);
	  pCompressedSB->cc.pair[dwIndex].dwType = TLTRANSFORMSTATE_WORLD3; 
	  pCompressedSB->cc.pair[dwIndex].dwValue = 1;
	  dwIndex++;
	}
    if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, TLTRANSFORMSTATE_VIEW) )
	{
      D3DPRINT(NORMAL_DBG_LEVEL, "  copying view transformation");
      memcpy(pNextData, &pUncompressedSB->uc.TransformationStates[TLTRANSFORMSTATE_VIEW], sizeof(D3DMATRIX));
      pNextData += sizeof(D3DMATRIX);
	  pCompressedSB->cc.pair[dwIndex].dwType = TLTRANSFORMSTATE_VIEW; 
	  pCompressedSB->cc.pair[dwIndex].dwValue = 1;
	  dwIndex++;
	}
    if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, TLTRANSFORMSTATE_PROJ) )
	{
      D3DPRINT(NORMAL_DBG_LEVEL, "  copying projection transformation");
      memcpy(pNextData, &pUncompressedSB->uc.TransformationStates[TLTRANSFORMSTATE_PROJ], sizeof(D3DMATRIX));
      pNextData += sizeof(D3DMATRIX);
	  pCompressedSB->cc.pair[dwIndex].dwType = TLTRANSFORMSTATE_PROJ; 
	  pCompressedSB->cc.pair[dwIndex].dwValue = 1;
	  dwIndex++;
	}
    for(j=0; j<D3DDP_MAXTEXCOORD; ++j)
      if( IS_STATEBLOCK_XFORM_FLAG_SET( pUncompressedSB, (TLTRANSFORMSTATE_TEX0+j)) )
	  {
        D3DPRINT(NORMAL_DBG_LEVEL, "  copying texture %d transformation", j);
        memcpy(pNextData, &pUncompressedSB->uc.TransformationStates[TLTRANSFORMSTATE_TEX0+j], sizeof(D3DMATRIX));
        pNextData += sizeof(D3DMATRIX);
  	    pCompressedSB->cc.pair[dwIndex].dwType = TLTRANSFORMSTATE_TEX0+j; 
	    pCompressedSB->cc.pair[dwIndex].dwValue = 1;
	    dwIndex++;
	  }

	// Z-Range
	pCompressedSB->cc.dwSizeStoredZRange = dwSizeStoredZRange;
	if(dwSizeStoredZRange)
	{
      D3DPRINT(NORMAL_DBG_LEVEL, "  copying Z-Range state");
      memcpy(pNextData, &pUncompressedSB->uc.ZRangeState, sizeof(D3DHAL_DP2ZRANGE));
      pNextData += sizeof(D3DHAL_DP2ZRANGE);
	}

    // User Clip Planes
	pCompressedSB->cc.dwNumUserClipplanes = dwNumUserClipplanes;
    for(j=0; j<TLMAX_USER_CLIPPLANES; ++j)
	{
      if( IS_STATEBLOCK_CLIPPLANE_FLAG_SET( pUncompressedSB, j ))
	  {
	    D3DPRINT(NORMAL_DBG_LEVEL, "      copying user clip plane %d", j);
        memcpy(pNextData, &pUncompressedSB->uc.ClipPlaneStates[j], sizeof(TLVECTOR4));
        pNextData += sizeof(TLVECTOR4);
	    pCompressedSB->cc.pair[dwIndex].dwType = j; 
	    pCompressedSB->cc.pair[dwIndex].dwValue = 1;
	    dwIndex++;
	  }
    }

#endif //TnL_HAL

#if (DIRECT3D_VERSION >= 0x0800) && (DX >= 8)
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
#endif

    // Get rid of the old(uncompressed) one
    D3DPRINT(NORMAL_DBG_LEVEL, "  freeing pUncompressedSB=%lXh", pUncompressedSB);
    DXFREE(pUncompressedSB);
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
#endif //ENABLE_COMPRESSED_STATEBLOCKS
}

#endif //DX7

