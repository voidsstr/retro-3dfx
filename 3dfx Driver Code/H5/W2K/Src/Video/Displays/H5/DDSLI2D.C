/* -*-c++-*- */
/* $Header: ddsli2d.c, 19, 10/25/00 4:58:22 AM, Johnny Trainor $ */
/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** File Name: 	ddsli2d.c
**
** Description: This file has all of the 2D routines to work on distributed
**  surfaces aka Sli surfaces.
**
** $Revision: 19$
** $Date: 10/25/00 4:58:22 AM$
**
** $History: $
**
*/

#include "precomp.h"

#ifndef WINNT
#include "fifomgr.h"
#include "regkeys.h"
#endif

#ifdef SLI_AA
#include <ddsli2d.h>
#endif

#define YMAX()          (_DD(dwMaxHeight))
#define YLINEARMAX()          (_DD(dwMaxLinearHeight))

#define GETFORMATBYTE(format, byte)                  \
              switch(format & SSTG_DST_FORMAT)       \
              {                                      \
                case SSTG_PIXFMT_8BPP:               \
                  (byte) = 1;                        \
                break;                               \
                case SSTG_PIXFMT_15BPP:              \
                case SSTG_PIXFMT_16BPP:              \
                case SSTG_PIXFMT_422YUV:             \
                case SSTG_PIXFMT_422UYV:             \
                  (byte) = 2;                        \
                break;                               \
                case SSTG_PIXFMT_24BPP:              \
                  (byte) = 3;                        \
                break;                               \
                case SSTG_PIXFMT_32BPP:              \
                  (byte) = 4;                        \
                break;                               \
                default:                             \
                  (byte) = INVALID_PIXELFORMAT;    \
                break;                               \
              }

#define BLIT_WAS_SW (0)
#define BLIT_WAS_HW (1)

#ifndef WINNT
int HostRestoreCursor(NT9XDEVICEDATA * ppdev, DWORD SaveCursorSurface, int LastCursorX, int LastCursorY, DWORD dwExclusionSave);
int HostDrawCursor(NT9XDEVICEDATA * ppdev, DWORD swapToAddr);
#endif

/*----------------------------------------------------------------------
Function name: SliClear

Description:   This is the high level routine to clear Sli surfaces.

Notes:         Make sure to add clean stuff around this call....
Return:        None  

----------------------------------------------------------------------*/
HRESULT SliClear(
  RC * pRc, DWORD dwFlags, DWORD dwFillColor, D3DVALUE dvFillDepth, 
  DWORD dwFillStencil, RECT *pRects, DWORD count)
{
   SETUP_PPDEV(pRc)
   DWORD i;
   DWORD dstPixelFormat;
   long  dstPitch;
   DWORD fillData1;
   DWORD fillData2;
   DWORD dwReturned;

   SOLIDCOLORPARAMS SolidColorParams; 
   FXSURFACEDATA * pSurfaceData;

   for (i = 0; i < count; ++i)
      {
      SolidColorParams.dstLeft = pRects[i].left;
      SolidColorParams.dstTop = pRects[i].top;
      SolidColorParams.dstRight = pRects[i].right;
      SolidColorParams.dstBottom = pRects[i].bottom;

      if (((dwFlags & D3DCLEAR_ZBUFFER) | (dwFlags & D3DCLEAR_STENCIL)) && (pRc->DDSZHndl != 0))
         {
//#ifdef WINNT
//         pSurfaceData = (FXSURFACEDATA*)(((DD_SURFACE_LOCAL *)pRc->lpDDSZ)->lpGbl->dwReserved1);
//#else
         pSurfaceData = (FXSURFACEDATA*)((TXTRHNDL_PTR(pRc->DDSZHndl))->surfData);
//#endif

         SolidColorParams.isTiled = IS_TILED(pSurfaceData->hwPtr);
         if (SolidColorParams.isTiled && _DD(ddSLIModeEnabled))
            SolidColorParams.dwSliSurface = TRUE;
         else
            SolidColorParams.dwSliSurface = FALSE;

//#ifdef WINNT
//         SolidColorParams.bltDstBaseAddr = GET_HW_ADDR(pRc->lpDDSZ);
//#else
         SolidColorParams.bltDstBaseAddr = GET_HW_OFFSET(pRc->DDSZHndl);
//#endif

         SolidColorParams.BytesPerPel = 
          ((TXTRHNDL_PTR(pRc->DDSZHndl))->dwZDepth) >> 3;

         GETPIXELFORMAT(SolidColorParams.BytesPerPel, dstPixelFormat);

         SolidColorParams.fpVidMem = pSurfaceData->lfbPtr;
#ifdef WINNT
         // assume the zbuffer is in video memory
         SolidColorParams.fpVidMem += (DWORD)ppdev->pjLfbBase;
#endif
         SolidColorParams.linearPitch = TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch;
         SolidColorParams.isZClear = TRUE;
#if ENABLE_TILED_HEAP
         if (SolidColorParams.isTiled)
            {
            dstPitch = _DS(ddTileStride);
            SolidColorParams.Pitch = _DS(ddTilePitch);
            }
         else
#endif         
            {
            dstPitch = TXTRHNDL_PTR(pRc->DDSZHndl)->lPitch;
            SolidColorParams.Pitch = dstPitch;
            }

         BLTFMT(dstPitch, dstPixelFormat, SolidColorParams.bltDstFormat);

         if ((pRc->stencilEnable) && ((D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL) != (dwFlags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL))))
            {
            // This is a two step operation
            // we are going to do a AND and then a OR
            // the AND mask will be either NOT STENCIL or NOT Z-Buffer and the
            // OR mask will be STENCIL or Z-BUFFER Data
            if (dwFlags & D3DCLEAR_ZBUFFER)
               {
               fillData1 = ((0xFF << 24) & TXTRHNDL_PTR(pRc->DDSZHndl)->dwStencil);
#ifdef WINNT
               fillData2 = ((DWORD)(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));
#else
               fillData2 = (DWORD)(ddftol(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));
#endif
               }
            else
               {
               fillData1 = ((0x00FFFFFF) & TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask);
               fillData2 = ((dwFillStencil << 24) & TXTRHNDL_PTR(pRc->DDSZHndl)->dwStencil);
               }

            // Clear the Stencil or Z-Buffer
            SolidColorParams.fillData =  fillData1;
            SolidColorParams.bltRop = (SSTG_ROP_AND << 16) | (SSTG_ROP_AND << 8) | (SSTG_ROP_AND);
            dwReturned = DdSli2DSolidColor(ppdev, &SolidColorParams);
      
            // Fill the Stencil or Z-Buffer
            SolidColorParams.fillData =  fillData2;
            SolidColorParams.bltRop = (SSTG_ROP_OR << 16) | (SSTG_ROP_OR << 8) | (SSTG_ROP_OR);
            dwReturned = DdSli2DSolidColor(ppdev, &SolidColorParams);
            // If AA Mode then Do Every Thing Twice unless we punted then once is enough
            if ( (BLIT_WAS_HW == dwReturned) &&
                 SolidColorParams.isTiled && _DD(ddAAModeEnabled)
               )
               {
//#ifdef WINNT
//               SolidColorParams.bltDstBaseAddr = GET_AAHW_ADDR(pRc->lpDDSZ);
//#else
               SolidColorParams.bltDstBaseAddr = GET_AAHW_OFFSET(pRc->DDSZHndl);
//#endif
               SolidColorParams.fpVidMem = pSurfaceData->AAlfbPtr;
#ifdef WINNT
               // assume the aa zbuffer is in video memory
               SolidColorParams.fpVidMem += (DWORD)ppdev->pjLfbBase;
#endif
               SolidColorParams.Pitch = pSurfaceData->AAlPitch;

               // Clear the Stencil or Z-Buffer
               SolidColorParams.fillData =  fillData1;
               SolidColorParams.bltRop = (SSTG_ROP_AND << 16) | (SSTG_ROP_AND << 8) | (SSTG_ROP_AND);
               DdSli2DSolidColor(ppdev, &SolidColorParams);
      
               // Fill the Stencil or Z-Buffer
               SolidColorParams.fillData =  fillData2;
               SolidColorParams.bltRop = (SSTG_ROP_OR << 16) | (SSTG_ROP_OR << 8) | (SSTG_ROP_OR);
               DdSli2DSolidColor(ppdev, &SolidColorParams);
               }
            }
         else
            {
            SolidColorParams.bltRop = (SSTG_ROP_SRC << 16 )| (SSTG_ROP_SRC << 8 ) | (SSTG_ROP_SRC );
#ifdef WINNT
            SolidColorParams.fillData =  ((dwFillStencil << 24) & TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask) |   // Stencil value
                         ((DWORD)(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));                     // Z value
#else
            SolidColorParams.fillData =  ((dwFillStencil << 24) & TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask) |   // Stencil value
                         ((DWORD)ddftol(dvFillDepth * TXTRHNDL_PTR(pRc->DDSZHndl)->dwZMask));               // Z value
#endif

            // Doit the operation
            dwReturned = DdSli2DSolidColor(ppdev, &SolidColorParams);
            // If AA Mode then Do Every Thing Twice unless we punted then once is enough
            if ( (BLIT_WAS_HW == dwReturned) &&
                 SolidColorParams.isTiled && _DD(ddAAModeEnabled)
               )
               {
//#ifdef WINNT
//               SolidColorParams.bltDstBaseAddr = GET_AAHW_ADDR(pRc->lpDDSZ);
//#else
               SolidColorParams.bltDstBaseAddr = GET_AAHW_OFFSET(pRc->DDSZHndl);
//#endif
               SolidColorParams.fpVidMem = pSurfaceData->AAlfbPtr;
#ifdef WINNT
               // assume the aa zbuffer is in video memory
               SolidColorParams.fpVidMem += (DWORD)ppdev->pjLfbBase;
#endif
               SolidColorParams.Pitch = pSurfaceData->AAlPitch;
               DdSli2DSolidColor(ppdev, &SolidColorParams);
               }
            }    
         }

      if( (dwFlags & D3DCLEAR_TARGET) && (pRc->DDSHndl != 0))
         {
//#ifdef WINNT
//         pSurfaceData = (FXSURFACEDATA*)(((DD_SURFACE_LOCAL *)pRc->lpDDS)->lpGbl->dwReserved1);
//#else
         pSurfaceData = (FXSURFACEDATA*)((TXTRHNDL_PTR(pRc->DDSHndl))->surfData);
//#endif
         SolidColorParams.isTiled = IS_TILED(pSurfaceData->hwPtr);
         if (SolidColorParams.isTiled && _DD(ddSLIModeEnabled))
            SolidColorParams.dwSliSurface = TRUE;
         else
            SolidColorParams.dwSliSurface = FALSE;

//#ifdef WINNT
//         SolidColorParams.bltDstBaseAddr = GET_HW_ADDR(pRc->lpDDS);
//#else
         SolidColorParams.bltDstBaseAddr = GET_HW_OFFSET(pRc->DDSHndl);
//#endif

         SolidColorParams.BytesPerPel = GETPRIMARYBYTEDEPTH;
         if (SolidColorParams.BytesPerPel == 2)            // if 16 bpp
            {
            SolidColorParams.fillData = dwFillColor;  // Assume fill color is rgb888 always
		      SolidColorParams.fillData = (((SolidColorParams.fillData & 0xF80000) >> 8) | ((SolidColorParams.fillData & 0xFC00) >> 5) | ((SolidColorParams.fillData & 0xF8) >> 3));
            }
         else if (SolidColorParams.BytesPerPel == 4)       // if 32 bpp
            {
            SolidColorParams.fillData = dwFillColor & 0x00FFFFFF;
            }
         else                                // An error exists - non supported color format
            {
            return D3DERR_UNSUPPORTEDCOLOROPERATION;
            }

         GETPIXELFORMAT(SolidColorParams.BytesPerPel, dstPixelFormat);

         SolidColorParams.fpVidMem = pSurfaceData->lfbPtr;
#ifdef WINNT
         // assume the render target is in video memory
         SolidColorParams.fpVidMem += (DWORD)ppdev->pjLfbBase;
#endif
         SolidColorParams.linearPitch = TXTRHNDL_PTR(pRc->DDSHndl)->lPitch;
         SolidColorParams.isZClear = FALSE;
#if ENABLE_TILED_HEAP
         if (SolidColorParams.isTiled)
            {
            dstPitch = _DS(ddTileStride);
            SolidColorParams.Pitch = _DS(ddTilePitch);
            }
         else
#endif       
            {  
            dstPitch = TXTRHNDL_PTR(pRc->DDSHndl)->lPitch;
            SolidColorParams.Pitch = dstPitch;
            }

         BLTFMT(dstPitch, dstPixelFormat, SolidColorParams.bltDstFormat);
         // Doit the operation
         SolidColorParams.bltRop = (SSTG_ROP_SRC << 16) | (SSTG_ROP_SRC << 8) | (SSTG_ROP_SRC);
         dwReturned = DdSli2DSolidColor(ppdev, &SolidColorParams);

         // If AA Mode then Do Every Thing Twice unless we punted then once is enough
         if ( (BLIT_WAS_HW == dwReturned) &&
              SolidColorParams.isTiled && _DD(ddAAModeEnabled)
            )
            {
//#ifdef WINNT
//            SolidColorParams.bltDstBaseAddr = GET_AAHW_ADDR(pRc->lpDDS);
//#else
            SolidColorParams.bltDstBaseAddr = GET_AAHW_OFFSET(pRc->DDSHndl);
//#endif
            SolidColorParams.fpVidMem = pSurfaceData->AAlfbPtr;
#ifdef WINNT
            // assume the aa render target is in video memory
            SolidColorParams.fpVidMem += (DWORD)ppdev->pjLfbBase;
#endif
            SolidColorParams.Pitch = pSurfaceData->AAlPitch;
            DdSli2DSolidColor(ppdev, &SolidColorParams);
            }
         }

      }

   return D3D_OK;
}


/*----------------------------------------------------------------------
Function name: DdSliColorFill

Description:   This is the high level routine to colorfill Sli surfaces.

Return:        None  

----------------------------------------------------------------------*/
DWORD DdSliColorFill(
  NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD dstPixelFormat, 
  DWORD dwSurfCaps)
{
   long      dstPitch;
   SOLIDCOLORPARAMS SolidColorParams;   
   FXSURFACEDATA * pDstSurfaceData;
   DWORD dwReturn;
   DWORD dwReturned;

#ifdef WINNT
    ASSERTDD(DDSCAPS_VIDEOMEMORY == (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY),
             "DdSliColorFill called with non video memory destination");
#endif

   pDstSurfaceData = (FXSURFACEDATA*) (pbd->lpDDDestSurface->lpGbl->dwReserved1);
   if (IS_TILED(pDstSurfaceData->hwPtr))
      {
      SolidColorParams.dstTop = pbd->rDest.top;
      SolidColorParams.dstRight = pbd->rDest.right;
      SolidColorParams.dstBottom = pbd->rDest.bottom;
      SolidColorParams.dstLeft = pbd->rDest.left;
   
      SolidColorParams.bltDstBaseAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
     
      SolidColorParams.fillData = pbd->bltFX.dwFillColor;

      // This seems kinda of pointless
#if ENABLE_TILED_HEAP
      if (IS_TILED(SolidColorParams.bltDstBaseAddr))
            {
            dstPitch = _DS(ddTileStride);
            SolidColorParams.Pitch = _DS(ddTilePitch);
            }
      else
#endif         
            {
            dstPitch = pbd->lpDDDestSurface->lpGbl->lPitch;         
            SolidColorParams.Pitch = dstPitch;
            }
      BLTFMT(dstPitch, dstPixelFormat, SolidColorParams.bltDstFormat);
      SolidColorParams.bltRop = (SSTG_ROP_SRC << 16 )| (SSTG_ROP_SRC << 8 ) | (SSTG_ROP_SRC );

      if (_DD(ddSLIModeEnabled))
         SolidColorParams.dwSliSurface = TRUE;
      else
         SolidColorParams.dwSliSurface = FALSE;
   
      SolidColorParams.fpVidMem = pDstSurfaceData->lfbPtr;
#ifdef WINNT
      // lpDstSurf must be in video memory
      SolidColorParams.fpVidMem += (DWORD)ppdev->pjLfbBase;
#endif
      SolidColorParams.linearPitch = pbd->lpDDDestSurface->lpGbl->lPitch;
      SolidColorParams.isTiled = TRUE;
      SolidColorParams.isZClear= (dwSurfCaps & DDSCAPS_ZBUFFER)? TRUE : FALSE;

      GETFORMATBYTE(dstPixelFormat, SolidColorParams.BytesPerPel);
      dwReturned = DdSli2DSolidColor(ppdev, &SolidColorParams);

      // If AA Mode then do it for each buffer unless we punted then once is enough   
      if (_DD(ddAAModeEnabled) && (BLIT_WAS_HW == dwReturned))
         {
         SolidColorParams.bltDstBaseAddr = GET_AAHW_ADDR(pbd->lpDDDestSurface);
         SolidColorParams.fpVidMem = pDstSurfaceData->AAlfbPtr;
#ifdef WINNT
         // lpDstSurf must be in video memory
         SolidColorParams.fpVidMem += (DWORD)ppdev->pjLfbBase;
#endif
         SolidColorParams.Pitch = pDstSurfaceData->AAlPitch;
         DdSli2DSolidColor(ppdev, &SolidColorParams);
         }
   
      pbd->ddRVal = DD_OK;
      dwReturn = DDHAL_DRIVER_HANDLED;
      }
   else
      {
      dwReturn = Blt32_ColorFill(ppdev, pbd, dstPixelFormat);
      }
   
   return dwReturn;
}

#ifndef WINNT
/*----------------------------------------------------------------------
Function name:  DoIntersect

Description:    Evaluate whether or not two rectangles intersect.
                One rectange is always the cursor rect.  Routine
                used bitmask to evalulate all possibilities at once.
Information:    

Return:         DWORD   TRUE if interect, FALSE if no intersect.
----------------------------------------------------------------------*/
int IsIntersect[] = {  // y2 x2 y1 x1
   FALSE,            // 0  0  0  0
   FALSE,            // 0  0  0  1
   FALSE,            // 0  0  1  0
   TRUE,             // 0  0  1  1
   FALSE,            // 0  1  0  0
   FALSE,            // 0  1  0  1
   TRUE,             // 0  1  1  0
   TRUE,             // 0  1  1  1
   FALSE,            // 1  0  0  0
   TRUE,             // 1  0  0  1
   FALSE,            // 1  0  1  0
   TRUE,             // 1  0  1  1
   TRUE,             // 1  1  0  0
   TRUE,             // 1  1  0  1
   TRUE,             // 1  1  1  0
   TRUE,             // 1  1  1  1
   };
#define RANGE(val,lo,hi) ((lo) <= (val) && (val) <= (hi))
int DoIntersect (NT9XDEVICEDATA *ppdev, int left, int top, int right, int bottom)
{
   int bIntersect;
   int cl, cr, ct, cb;

   cl = _FF(LastCursorPosX);
   cr = cl + SWCURSOR_WIDTH;
   ct = _FF(LastCursorPosY);
   cb = ct + SWCURSOR_HEIGHT;

   if (right - left <= SWCURSOR_WIDTH)
      {
      bIntersect = RANGE(left, cl, cr);
      bIntersect |= (RANGE(right, cl, cr) << 2);
      }
   else
      {
      bIntersect = RANGE(cl, left, right);
      bIntersect |= (RANGE(cr, left, right) << 2);
      }

   if (bottom - top <= SWCURSOR_WIDTH)
      {
      bIntersect |= (RANGE(top, ct, cb) << 1);
      bIntersect |= (RANGE(bottom, ct, cb) << 3);
      }
   else
      {
      bIntersect |= (RANGE(ct, top, bottom) << 1);
      bIntersect |= (RANGE(cb, top, bottom) << 3);
      }

   return IsIntersect[bIntersect];
}
#endif

/*----------------------------------------------------------------------
Function name: DdSli2DSolidColor

Description:   This is a low level routine to fill a Distributed
surface using solid color fill.

There are two cases:

1.) Easy Case --  Rectangle to fill is the whole surface.  In this case
we just need to crunch Y to account for Sli.
2.) Hard Case -- Rectangle to fill is part of the whole surface.  This
case breaks down to the following:

   a.) Fill Partial Start <a partial Fill is where the Height of Blit
   is less then BandHeight>.  There can be only one of these on one chip.
   b.) Fill Wholes.  There can be several of these and if conditions are right
   more then one chip can work in parallel.
   c.) Fill Partial End <only one on one chip>.

Return:   Whether HW or LFB did the Operation  

----------------------------------------------------------------------*/
int DdSli2DSolidColor(NT9XDEVICEDATA * ppdev, PSOLIDCOLORPARAMS pParams)
{
   DWORD clip1min;
   DWORD clip1max;
   DWORD packetHeader;
   long dstWidth;
   long dstHeight;
   DWORD bltDstSize;
   DWORD bltDstXY;
   DWORD dwReturn = BLIT_WAS_HW;
   DWORD bltDstFormat;
   DWORD dstRight;
   DWORD bltDstBaseAddr;
#ifndef WINNT
   DWORD dwDrawCursor=FALSE;
#endif
   CMDFIFO_PROLOG(cmdFifo);

   //Calculate some loop invariants
   dstWidth = pParams->dstRight - pParams->dstLeft;

#ifndef WINNT
   if (_FF(CursorSurface) == pParams->fpVidMem)
      {
      if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
         {
         if (DoIntersect(ppdev, pParams->dstLeft, pParams->dstTop, pParams->dstRight, pParams->dstBottom))
            {
            HostRestoreCursor(ppdev, pParams->fpVidMem, _FF(LastCursorPosX), _FF(LastCursorPosY), _FF(HostcursorExclusionStart));
            dwDrawCursor = TRUE;
            }
         }
      }
#endif

   if (pParams->dwSliSurface)
      {
      // Start with the most common and easist case
      // This is the case where the Blt is Full Screen so we just need to munge the
      // Height of the Blt
      if ((0x0 == pParams->dstTop) &&
#ifdef WINNT
          (ppdev->cyScreen == pParams->dstBottom))
#else
          (ppdev->bi.biHeight == pParams->dstBottom))
#endif
         {
#ifndef WINNT
         ppdev->gdiFlags |= SDATA_GDIFLAGS_2D_DIRTY; 
#endif

         // If screen is equal to screen width then 
         // a futher optimization is to blit linearly instead of tiled
#if 0
         if ((0x0 == pParams->dstLeft) &&
#ifndef WINNT
             (ppdev->bi.biWidth == pParams->dstRight))
#else

             (ppdev->cxScreen == pParams->dstRight))
#endif
            {
            bltDstFormat = pParams->bltDstFormat & SSTG_DST_TILE_STRIDE;
            bltDstFormat <<= SST_TILE_WIDTH_BITS;
#ifndef WINNT
            dstRight = bltDstFormat >> (_FF(bpp)>>4);
#else
            dstRight = bltDstFormat >> (ppdev->cBitsPerPel>>4);
#endif
            bltDstFormat |= (pParams->bltDstFormat & ~SSTG_DST_TILE_STRIDE);
            bltDstBaseAddr = pParams->bltDstBaseAddr & ~SSTG_IS_TILED;
            dstWidth = dstRight;
            dstHeight = YLINEARMAX();
            }
         else
#endif
            {
            bltDstFormat = pParams->bltDstFormat;
            dstRight = pParams->dstRight;
            bltDstBaseAddr = pParams->bltDstBaseAddr;
            dstHeight = YMAX();
            }

   	   CMDFIFO_CHECKROOM( cmdFifo, 10);

         // write to hw
         packetHeader =  dstBaseAddrBit | dstFormatBit | ropBit | clip1minBit | clip1maxBit | colorForeBit | dstSizeBit | dstXYBit | commandBit;

         BLTCLIP(pParams->dstLeft, pParams->dstTop, clip1min);
         BLTCLIP(dstRight, dstHeight, clip1max);
         BLTSIZE(dstWidth, dstHeight, bltDstSize);
         BLTXY(pParams->dstLeft, pParams->dstTop, bltDstXY);
         SETPH(cmdFifo, CMDFIFO_BUILD_PK2(packetHeader));
         SETPD(cmdFifo, ghw2D->dstBaseAddr, bltDstBaseAddr);
         SETPD(cmdFifo, ghw2D->dstFormat, bltDstFormat);
         SETPD(cmdFifo, ghw2D->rop,          pParams->bltRop );
         SETPD(cmdFifo, ghw2D->clip1min,     clip1min);
         SETPD(cmdFifo, ghw2D->clip1max,     clip1max);
         SETPD(cmdFifo, ghw2D->colorFore,    pParams->fillData );
         SETPD(cmdFifo, ghw2D->dstSize,      bltDstSize);
         SETPD(cmdFifo, ghw2D->dstXY,        bltDstXY);
         SETPD(cmdFifo, ghw2D->command,      SSTG_RECTFILL | SSTG_GO
                  | ((pParams->bltRop & 0xFF) << SSTG_ROP0_SHIFT) | SSTG_CLIPSELECT);

         BUMP(10);
         }
      else
         {  // Punt to SW 
         if (pParams->isZClear)
          sli_PuntColorFillZ(ppdev, pParams);
         else
          sli_PuntColorFill(ppdev, pParams);
         dwReturn = BLIT_WAS_SW;
         }
      }
   else
      {
#ifndef WINNT
      ppdev->gdiFlags |= SDATA_GDIFLAGS_2D_DIRTY; 
#endif
      // Punt to HW non-SLI case
      CMDFIFO_CHECKROOM( cmdFifo, 10);

      // write to hw
      packetHeader =  dstBaseAddrBit | dstFormatBit | ropBit | clip1minBit | clip1maxBit | colorForeBit | dstSizeBit | dstXYBit | commandBit;

      BLTCLIP(pParams->dstLeft, pParams->dstTop, clip1min);
      dstHeight = pParams->dstBottom - pParams->dstTop;
      BLTCLIP(pParams->dstRight, pParams->dstBottom, clip1max);
      BLTSIZE(dstWidth, dstHeight, bltDstSize);
      BLTXY(pParams->dstLeft, pParams->dstTop, bltDstXY);
      SETPH(cmdFifo, CMDFIFO_BUILD_PK2(packetHeader));
      SETPD(cmdFifo, ghw2D->dstBaseAddr,  pParams->bltDstBaseAddr);
      SETPD(cmdFifo, ghw2D->dstFormat,    pParams->bltDstFormat);
      SETPD(cmdFifo, ghw2D->rop,          pParams->bltRop );
      SETPD(cmdFifo, ghw2D->clip1min,     clip1min);
      SETPD(cmdFifo, ghw2D->clip1max,     clip1max);
      SETPD(cmdFifo, ghw2D->colorFore,    pParams->fillData );
      SETPD(cmdFifo, ghw2D->dstSize,      bltDstSize);
      SETPD(cmdFifo, ghw2D->dstXY,        bltDstXY);
      SETPD(cmdFifo, ghw2D->command,      SSTG_RECTFILL | SSTG_GO
                  | ((pParams->bltRop & 0xFF) << SSTG_ROP0_SHIFT) | SSTG_CLIPSELECT);

      BUMP(10);
      }

   CMDFIFO_EPILOG( cmdFifo );

#ifndef WINNT
   if (dwDrawCursor)
      HostDrawCursor(ppdev, pParams->fpVidMem);
#endif

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name: DdSliBltNoSP

Description:   This is a low level routine to work on a Distributed
surface.

There are two cases:

1.) Easy Case --  Rectangle to fill is the whole surface.  In this case
we just need to crunch Y to account for Sli.
2.) Hard Case -- Rectangle to fill is part of the whole surface.  This
case breaks down to the following:

   a.) Fill Partial Start <a partial Fill is where the Height of Blit
   is less then BandHeight>.  There can be only one of these on one chip.
   b.) Fill Wholes.  There can be several of these and if conditions are right
   more then one chip can work in parallel.
   c.) Fill Partial End <only one on one chip>.

Return:        None  

----------------------------------------------------------------------*/
int DdSliBltNoSP(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD dstPixelFormat)
{
   LONG bottom;
   DWORD dwReturn;
   DWORD hwAddr;
   FXSURFACEDATA * pDstSurfaceData;
   DWORD dstPixelByte;
#ifndef WINNT
   DWORD dwDrawCursor=FALSE;
#endif
   CMDFIFO_PROLOG(cmdFifo);

#ifdef WINNT
   ASSERTDD(DDSCAPS_VIDEOMEMORY == (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY),
            "DdSliBltNoSP called with non video memory destination");
#endif

   pDstSurfaceData = (FXSURFACEDATA*) (pbd->lpDDDestSurface->lpGbl->dwReserved1);

#ifndef WINNT
   if (_FF(CursorSurface) == pDstSurfaceData->lfbPtr)
      {
      if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
         {
         if (DoIntersect(ppdev, pbd->rDest.left, pbd->rDest.top, pbd->rDest.right, pbd->rDest.bottom))
            {
            HostRestoreCursor(ppdev, pDstSurfaceData->lfbPtr, _FF(LastCursorPosX), _FF(LastCursorPosY), _FF(HostcursorExclusionStart));
            dwDrawCursor = TRUE;
            }
         }
      }
#endif

   if (IS_TILED(pDstSurfaceData->hwPtr))
      {
      // Start with the most common and easist case
      // This is the case where the Blt is Full Screen so we just need to munge the
      // Height of the Blt
      bottom = pbd->rDest.bottom;
      if ((0x0 == pbd->rDest.top) &&
#ifdef WINNT
          (ppdev->cyScreen == pbd->rDest.bottom))
#else
          (ppdev->bi.biHeight == pbd->rDest.bottom))
#endif
         {
         // If SLI Mode is enabled then reduce Y
         if (_DD(ddSLIModeEnabled))
            pbd->rDest.bottom = YMAX();
         dwReturn = Blt32_DoBltNoSP(ppdev, pbd, rop3, dstPixelFormat);

         // One more time for AA
         if (_DD(ddAAModeEnabled))
            {
            // Swap the Address so that we use the AA Addr
            hwAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
            SET_HW_ADDR(pbd->lpDDDestSurface, GET_AAHW_ADDR(pbd->lpDDDestSurface));
            dwReturn = Blt32_DoBltNoSP(ppdev, pbd, rop3, dstPixelFormat);
            SET_HW_ADDR(pbd->lpDDDestSurface, hwAddr);
            }

         pbd->rDest.bottom = bottom;
         }
      else
         {
         // If just AA Mode then we can handle it
         if (!_DD(ddSLIModeEnabled) && _DD(ddAAModeEnabled))
            {
            // We can accelerate ALL AA case w/o SLI
            dwReturn = Blt32_DoBltNoSP(ppdev, pbd, rop3, dstPixelFormat);

            // One more time for AA
            // Swap the Address so that we use the AA Addr
            hwAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
            SET_HW_ADDR(pbd->lpDDDestSurface, GET_AAHW_ADDR(pbd->lpDDDestSurface));
            dwReturn = Blt32_DoBltNoSP(ppdev, pbd, rop3, dstPixelFormat);
            SET_HW_ADDR(pbd->lpDDDestSurface, hwAddr);
            }
         else
            {
            // Punt to SW case
            GETFORMATBYTE(dstPixelFormat, dstPixelByte);
            dwReturn = sli_DoBltNoSP(ppdev, pbd, rop3, dstPixelByte);
            }
         }
      }
   else
      dwReturn = Blt32_DoBltNoSP(ppdev, pbd, rop3, dstPixelFormat);

#ifndef WINNT
   if (dwDrawCursor)
      HostDrawCursor(ppdev, pDstSurfaceData->lfbPtr);
#endif

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name: DdSli2DScn2Scn

Description:   This is a low level routine to perform Screen to Screen
Blits when in SLI distributed mode.

There are 3 main cases with many subcases

1.) Distributed to Distributed
   a.) Same Surface
   b.) Different Surfaces 
2.) Non-Distributed to Distributed
Assumptions:  Each Device has the same image which must be
placed in the correct Device Distributed Surface
3.) Distributed to Non-Distributed
Assumptions: Each Device has part of the image which must be combined
in a off screen surface

Return:        None  

----------------------------------------------------------------------*/
DWORD DdSli2DScn2Scn(NT9XDEVICEDATA * ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD srcPixelFormat, DWORD dstPixelFormat)
{
   FXSURFACEDATA       *pSrcSurfaceData;
   FXSURFACEDATA       *pDstSurfaceData;
   long dstHeight;
   long srcHeight;
   long dstWidth;
   long srcWidth;
   LONG bottom;
   DWORD dwReturn;
   DWORD hwDstAddr;
   DWORD hwSrcAddr;
#ifndef WINNT
   DWORD dwDrawCursor=FALSE;
#endif
   CMDFIFO_PROLOG(cmdFifo);

#ifdef WINNT
   ASSERTDD(DDSCAPS_VIDEOMEMORY == (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY),
            "DdSli2DScn2Scn called with non video memory destination");
#endif

   pSrcSurfaceData = (FXSURFACEDATA*) (pbd->lpDDSrcSurface->lpGbl->dwReserved1);
   pDstSurfaceData = (FXSURFACEDATA*) (pbd->lpDDDestSurface->lpGbl->dwReserved1);

#ifndef WINNT
   if (_FF(CursorSurface) == pDstSurfaceData->lfbPtr)
      {
      if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
         {
         if (DoIntersect(ppdev, pbd->rDest.left, pbd->rDest.top, pbd->rDest.right, pbd->rDest.bottom))
            {
            HostRestoreCursor(ppdev, pDstSurfaceData->lfbPtr, _FF(LastCursorPosX), _FF(LastCursorPosY), _FF(HostcursorExclusionStart));
            dwDrawCursor = TRUE;
            }
         }
      }
#endif

   // If in Flip Chain assume that it is a Sli Surface
   if ((IS_TILED(pSrcSurfaceData->hwPtr)) == (IS_TILED(pDstSurfaceData->hwPtr)))
      {
      if (IS_TILED(pSrcSurfaceData->hwPtr))
         {
         // if this is full screen not a stretch then
         // crunch Y and do it
         dstHeight = pbd->rDest.bottom - pbd->rDest.top;
         srcHeight = pbd->rSrc.bottom - pbd->rSrc.top;
         dstWidth = pbd->rDest.right - pbd->rDest.left;
         srcWidth = pbd->rSrc.right - pbd->rSrc.left;
         if ((dstHeight == srcHeight) && 
             (dstWidth == srcWidth) &&
             (0x0 == pbd->rDest.top) &&
             (0x0 == pbd->rSrc.top) &&
#ifdef WINNT
             (ppdev->cyScreen == pbd->rDest.bottom))
#else
             (ppdev->bi.biHeight == pbd->rDest.bottom))
#endif
            {
            // Save crunched bottom
            bottom = pbd->rDest.bottom;

            // If SLI Mode is enabled then Just Crunch Y
            if (_DD(ddSLIModeEnabled))
               pbd->rDest.bottom = pbd->rSrc.bottom = YMAX();

            // If we Don't wait here Half-Life Looks bad
            if (pbd->dwFlags & DDBLT_WAIT)
               FXBUSYWAIT(ppdev);

            dwReturn = Blt32_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);

            // One more time for AA
            if (_DD(ddAAModeEnabled))
               {
               // Swap the Address so that we use the AA Addr
               hwDstAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
               hwSrcAddr = GET_HW_ADDR(pbd->lpDDSrcSurface);
               SET_HW_ADDR(pbd->lpDDDestSurface, GET_AAHW_ADDR(pbd->lpDDDestSurface));
               SET_HW_ADDR(pbd->lpDDSrcSurface, GET_AAHW_ADDR(pbd->lpDDSrcSurface));
               dwReturn = Blt32_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);
               SET_HW_ADDR(pbd->lpDDDestSurface, hwDstAddr);
               SET_HW_ADDR(pbd->lpDDSrcSurface, hwSrcAddr);
               }

            // Restore crunched bottom
            pbd->rDest.bottom = pbd->rSrc.bottom = bottom;
            }
         else
            {
            // If this is AA then we can go fast so do it
            if (!_DD(ddSLIModeEnabled) && _DD(ddAAModeEnabled))
               {
               if (pbd->dwFlags & DDBLT_WAIT)
                  FXBUSYWAIT(ppdev);

               dwReturn = Blt32_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);

               // Swap the Address so that we use the AA Addr
               hwDstAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
               hwSrcAddr = GET_HW_ADDR(pbd->lpDDSrcSurface);
               SET_HW_ADDR(pbd->lpDDDestSurface, GET_AAHW_ADDR(pbd->lpDDDestSurface));
               SET_HW_ADDR(pbd->lpDDSrcSurface, GET_AAHW_ADDR(pbd->lpDDSrcSurface));
               dwReturn = Blt32_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);
               SET_HW_ADDR(pbd->lpDDDestSurface, hwDstAddr);
               SET_HW_ADDR(pbd->lpDDSrcSurface, hwSrcAddr);
               }
            else
               {
               dwReturn = sli_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat, vid2vid);
               }
            }
         }
      else
         { 
         // ND to ND .... nothing to do
         dwReturn = Blt32_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);
         }
      }
   else if (IS_TILED(pSrcSurfaceData->hwPtr))
      { // DD to ND
      dwReturn = sli_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat, vid2vid);
      }
   else
      { // ND to DD 

      if (!_DD(ddSLIModeEnabled) && _DD(ddAAModeEnabled))
         {
         if (pbd->dwFlags & DDBLT_WAIT)
            FXBUSYWAIT(ppdev);

         dwReturn = Blt32_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);

         // Swap the Address so that we use the AA Addr
         // But only for the destination!!!!!
         hwDstAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
         SET_HW_ADDR(pbd->lpDDDestSurface, GET_AAHW_ADDR(pbd->lpDDDestSurface));
         dwReturn = Blt32_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);
         SET_HW_ADDR(pbd->lpDDDestSurface, hwDstAddr);
         }
      else
         {
            // Using the 3D engine to perform Blts!
            // First we should test whether or not to use the 3D SW path
            if((rop3 != HIWORD(SRCCOPY)) || (pbd->dwFlags & DDBLT_KEYDESTOVERRIDE) ||
               ((srcPixelFormat != SSTG_PIXFMT_16BPP) && (srcPixelFormat != SSTG_PIXFMT_32BPP)))
              dwReturn = sli_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat, vid2vid);
            else
            {
              // Using the Pitch since the surface might already be promoted
              dstWidth = (DWORD) pbd->lpDDSrcSurface->lpGbl->lPitch;
              // We need to see if the surface already is a power of two
              if( (dstWidth & (dstWidth-1)) )
              {
                // Promote the surface
                dwReturn = PromoteSrcToPower2Width(ppdev, pbd);
                if(pbd->ddRVal != DD_OK)	
                  dwReturn = sli_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat, vid2vid);
                else
                  dwReturn = sli_3DDoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);
              }
              else
                dwReturn = sli_3DDoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);
            }
         }
      }   

#ifndef WINNT
   if (dwDrawCursor)
      HostDrawCursor(ppdev, pDstSurfaceData->lfbPtr);
#endif

  return dwReturn;
}

/*----------------------------------------------------------------------
Function name: DdSli2DSystem2Scn

Description:   This is a low level routine to perform Screen to Screen
Blits when in SLI distributed mode.

There are 3 main cases with many subcases

1.) Distributed to Distributed
   a.) Same Surface
   b.) Different Surfaces 
2.) Non-Distributed to Distributed
Assumptions:  Each Device has the same image which must be
placed in the correct Device Distributed Surface
3.) Distributed to Non-Distributed
Assumptions: Each Device has part of the image which must be combined
in a off screen surface

Return:        None  

----------------------------------------------------------------------*/
DWORD DdSli2DSystem2Scn(NT9XDEVICEDATA * ppdev, LPDDHAL_BLTDATA pbd, DWORD rop, DWORD srcPixelFormat, DWORD srcBytePerPixel, DWORD dstPixelFormat)
{
   DWORD dwReturn;
   FXSURFACEDATA * pDstSurfaceData;
   DWORD hwAddr;
#ifndef WINNT
   DWORD dwDrawCursor=FALSE;
#endif
   CMDFIFO_PROLOG(cmdFifo);

#ifdef WINNT
    ASSERTDD(DDSCAPS_VIDEOMEMORY == (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY),
             "DdSli2DSystem2Scn called with non video memory destination");
#endif

   pDstSurfaceData = (FXSURFACEDATA*) (pbd->lpDDDestSurface->lpGbl->dwReserved1);

#ifndef WINNT
   if (_FF(CursorSurface) == pDstSurfaceData->lfbPtr)
      {
      if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
         {
         if (DoIntersect(ppdev, pbd->rDest.left, pbd->rDest.top, pbd->rDest.right, pbd->rDest.bottom))
            {
            HostRestoreCursor(ppdev, pDstSurfaceData->lfbPtr, _FF(LastCursorPosX), _FF(LastCursorPosY), _FF(HostcursorExclusionStart));
            dwDrawCursor = TRUE;
            }
         }
      }
#endif

   if (IS_TILED(pDstSurfaceData->hwPtr))
      {
      // ND to DD
      // If Just AA then we can doit
      // If this is a AA surface then do it twice
      if (!_DD(ddSLIModeEnabled) && _DD(ddAAModeEnabled))
         {
         dwReturn = Blt32_SystemToVideo(ppdev, pbd, rop, srcPixelFormat, srcBytePerPixel, dstPixelFormat);
         hwAddr = GET_HW_ADDR(pbd->lpDDDestSurface);
         SET_HW_ADDR(pbd->lpDDDestSurface, GET_AAHW_ADDR(pbd->lpDDDestSurface));
         dwReturn = Blt32_SystemToVideo(ppdev, pbd, rop, srcPixelFormat, srcBytePerPixel, dstPixelFormat);
         SET_HW_ADDR(pbd->lpDDDestSurface, hwAddr);
         }
      else
         {
         dwReturn = sli_DoBltS(ppdev, pbd, rop, srcPixelFormat, dstPixelFormat, mem2vid);
         }
      }
   else
      {
      // ND to ND
      dwReturn = Blt32_SystemToVideo(ppdev, pbd, rop, srcPixelFormat, srcBytePerPixel, dstPixelFormat);
      }
      
#ifndef WINNT
   if (dwDrawCursor)
      HostDrawCursor(ppdev, pDstSurfaceData->lfbPtr);
#endif

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name: Dd2SampleColorFill

Description:   Wrapper to handle cursor and then call colorfill

Return:        None  

----------------------------------------------------------------------*/
DWORD Dd2SampleColorFill(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD dstPixelFormat)
{
   FXSURFACEDATA * pDstSurfaceData;
   DWORD dwReturn;
#ifndef WINNT
   DWORD dwDrawCursor=FALSE;
#endif

#ifdef WINNT
    ASSERTDD(DDSCAPS_VIDEOMEMORY == (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY),
             "Dd2SampleColorFill called with non video memory destination");
#endif

   pDstSurfaceData = (FXSURFACEDATA*) (pbd->lpDDDestSurface->lpGbl->dwReserved1);

#ifndef WINNT
   if (_FF(CursorSurface) == pDstSurfaceData->lfbPtr)
      {
      if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
         {
         if (DoIntersect(ppdev, pbd->rDest.left, pbd->rDest.top, pbd->rDest.right, pbd->rDest.bottom))
            {
            HostRestoreCursor(ppdev, pDstSurfaceData->lfbPtr, _FF(LastCursorPosX), _FF(LastCursorPosY), _FF(HostcursorExclusionStart));
            dwDrawCursor = TRUE;
            }
         }
      }
#endif

   dwReturn = Blt32_ColorFill(ppdev, pbd, dstPixelFormat);

#ifndef WINNT
   if (dwDrawCursor)
      HostDrawCursor(ppdev, pDstSurfaceData->lfbPtr);
#endif

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name: Dd2SampleBltNoSP

Description:   Wrapper to handle cursor and then call BltNoSP

Return:        None  

----------------------------------------------------------------------*/
DWORD Dd2SampleBltNoSP(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD dstPixelFormat)
{
   FXSURFACEDATA * pDstSurfaceData;
   DWORD dwReturn;
#ifndef WINNT
   DWORD dwDrawCursor=FALSE;
#endif

#ifdef WINNT
    ASSERTDD(DDSCAPS_VIDEOMEMORY == (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY),
             "Dd2SampleBltNoSP called with non video memory destination");
#endif

   pDstSurfaceData = (FXSURFACEDATA*) (pbd->lpDDDestSurface->lpGbl->dwReserved1);

#ifndef WINNT
   if (_FF(CursorSurface) == pDstSurfaceData->lfbPtr)
      {
      if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
         {
         if (DoIntersect(ppdev, pbd->rDest.left, pbd->rDest.top, pbd->rDest.right, pbd->rDest.bottom))
            {
            HostRestoreCursor(ppdev, pDstSurfaceData->lfbPtr, _FF(LastCursorPosX), _FF(LastCursorPosY), _FF(HostcursorExclusionStart));
            dwDrawCursor = TRUE;
            }
         }
      }
#endif

   dwReturn = Blt32_DoBltNoSP(ppdev, pbd, rop3, dstPixelFormat);

#ifndef WINNT
   if (dwDrawCursor)
      HostDrawCursor(ppdev, pDstSurfaceData->lfbPtr);
#endif

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name: Dd2SampleBltS

Description:   Wrapper to handle cursor and then call BltS

Return:        None  

----------------------------------------------------------------------*/
DWORD Dd2SampleBltS(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD srcPixelFormat, DWORD dstPixelFormat)
{
   FXSURFACEDATA * pDstSurfaceData;
   DWORD dwReturn;
#ifndef WINNT
   DWORD dwDrawCursor=FALSE;
#endif

#ifdef WINNT
    ASSERTDD(DDSCAPS_VIDEOMEMORY == (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY),
             "Dd2SampleBltS called with non video memory destination");
#endif

   pDstSurfaceData = (FXSURFACEDATA*) (pbd->lpDDDestSurface->lpGbl->dwReserved1);

#ifndef WINNT
   if (_FF(CursorSurface) == pDstSurfaceData->lfbPtr)
      {
      if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
         {
         if (DoIntersect(ppdev, pbd->rDest.left, pbd->rDest.top, pbd->rDest.right, pbd->rDest.bottom))
            {
            HostRestoreCursor(ppdev, pDstSurfaceData->lfbPtr, _FF(LastCursorPosX), _FF(LastCursorPosY), _FF(HostcursorExclusionStart));
            dwDrawCursor = TRUE;
            }
         }
      }
#endif

   dwReturn = Blt32_DoBltS(ppdev, pbd, rop3, srcPixelFormat, dstPixelFormat);

#ifndef WINNT
   if (dwDrawCursor)
      HostDrawCursor(ppdev, pDstSurfaceData->lfbPtr);
#endif

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name: Dd2SampleSystemToVideo

Description:   Wrapper to handle cursor and then call SystemToVideo

Return:        None  

----------------------------------------------------------------------*/
DWORD Dd2SampleSystemToVideo(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD srcPixelFormat, DWORD pixelByteDepth, DWORD dstPixelFormat)
{
   FXSURFACEDATA * pDstSurfaceData;
   DWORD dwReturn;
#ifndef WINNT
   DWORD dwDrawCursor=FALSE;
#endif

#ifdef WINNT
    ASSERTDD(DDSCAPS_VIDEOMEMORY == (pbd->lpDDDestSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY),
             "Dd2SampleSystemToVideo called with non video memory destination");
#endif

   pDstSurfaceData = (FXSURFACEDATA*) (pbd->lpDDDestSurface->lpGbl->dwReserved1);

#ifndef WINNT
   if (_FF(CursorSurface) == pDstSurfaceData->lfbPtr)
      {
      if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
         {
         if (DoIntersect(ppdev, pbd->rDest.left, pbd->rDest.top, pbd->rDest.right, pbd->rDest.bottom))
            {
            HostRestoreCursor(ppdev, pDstSurfaceData->lfbPtr, _FF(LastCursorPosX), _FF(LastCursorPosY), _FF(HostcursorExclusionStart));
            dwDrawCursor = TRUE;
            }
         }
      }
#endif

   dwReturn = Blt32_SystemToVideo(ppdev, pbd, rop3, srcPixelFormat, pixelByteDepth, dstPixelFormat);

#ifndef WINNT
   if (dwDrawCursor)
      HostDrawCursor(ppdev, pDstSurfaceData->lfbPtr);
#endif

   return dwReturn;
}


