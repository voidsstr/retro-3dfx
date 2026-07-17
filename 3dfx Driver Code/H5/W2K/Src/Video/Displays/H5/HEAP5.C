/******************************Module*Header*******************************\
*
*                           *******************
*                           * GDI SAMPLE CODE *
*                           *******************
*
* Module Name: heap.c
*
* This module contains the routines for an off-screen video heap manager.
* It is used primarily for allocating space for device-format-bitmaps in
* off-screen memory.
*
* Off-screen bitmaps are a big deal on NT because:
*
*    1) It reduces the working set.  Any bitmap stored in off-screen
*       memory is a bitmap that isn't taking up space in main memory.
*
*    2) There is a speed win by using the accelerator hardware for
*       drawing, in place of NT's GDI code.  NT's GDI is written entirely
*       in 'C++' and perhaps isn't as fast as it could be.
*
*    3) It raises your Winbench score.
*
* Copyright (c) 1993-1998 Microsoft Corporation
* Copyright (c) 1998 3Dfx Interactive, Inc.
*
\**************************************************************************/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

//#include "precomp.h"  // already included by heap.c
#define __NTDDKCOMP__
#include "dmemmgr.h"

//#define DFB_INSTRUMENTATION
#ifdef DFB_INSTRUMENTATION
#include "dfbinfo.h"
#endif

/**************************************************************************
* D E F I N E S
***************************************************************************/

//#define PAINT_RECTS
//#define CHECK_FOR_OVERLAP

#ifdef PAINT_RECTS
#define PAINT_DFB_RECT(a,b,c,d,e,f)   PaintDFBRect((a),(b),(c),(d),(e),(f))
#else
#define PAINT_DFB_RECT(a,b,c,d,e,f)
#endif

#ifdef CHECK_FOR_OVERLAP
#define CHECK_OVERLAP(a,b)            CheckOverlap((a),(b))
#else
#define CHECK_OVERLAP(a,b)
#endif

/**************************************************************************
* G L O B A L   V A R S
***************************************************************************/

#ifdef PAINT_RECTS
DWORD InitFillColor;
DWORD AllocFillColor;
DWORD FreeFillColor;
#endif

#ifdef DFB_INSTRUMENTATION
DFBINFO DfbInfo;
#endif

/**************************************************************************
* P U B L I C   F U N C T I O N S
***************************************************************************/

#ifdef PAINT_RECTS
/**************************************************************************
*
* FUNCTION:     PaintDFBRect
*
* DESCRIPTION:
*
***************************************************************************/

void
PaintDFBRect ( PDEV   *ppdev,
               DWORD  dstBaseAddr,
               DWORD  dstXY,
               DWORD  dstSize,
               DWORD  dstPitch,
               DWORD  color )
{
  DWORD bltDstFormat, dstPixelFormat;
  CMDFIFO_PROLOG(hwPtr);


  GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
#if ENABLE_TILED_HEAP
  if (dstBaseAddr & SSTG_IS_TILED)
    BLTFMT(_FF(ddTileStride), dstPixelFormat, bltDstFormat);
  else
#endif
    BLTFMT(dstPitch, dstPixelFormat, bltDstFormat);

  CMDFIFO_CHECKROOM(hwPtr, 7);
  SETPH(hwPtr, CMDFIFO_BUILD_PK2(  dstBaseAddrBit
                                 | dstFormatBit
                                 | colorForeBit
                                 | dstSizeBit
                                 | dstXYBit
                                 | commandBit ));
  SETPD(hwPtr, ghw2D->dstBaseAddr, dstBaseAddr);
  SETPD(hwPtr, ghw2D->dstFormat,   bltDstFormat);
  SETPD(hwPtr, ghw2D->colorFore,   color );
  SETPD(hwPtr, ghw2D->dstSize,     dstSize);
  SETPD(hwPtr, ghw2D->dstXY,       dstXY);
  SETPD(hwPtr, ghw2D->command,     SSTG_RECTFILL | SSTG_GO
                                 | (SSTG_ROP_SRC << SSTG_ROP0_SHIFT));

  BUMP(7);

  CMDFIFO_EPILOG(hwPtr);
}
#endif


#ifdef CHECK_FOR_OVERLAP
/**************************************************************************
*
* FUNCTION:     CheckOverlap
*
* DESCRIPTION:
*
***************************************************************************/

void
CheckOverlap ( PDEV *ppdev, DSURF *pdsurf)
{
  if (NULL != ppdev->pdsurfDiscardableList)
  {
    DSURF *pdsurfExisting;

    pdsurfExisting = ppdev->pdsurfDiscardableList;
    while (pdsurfExisting)
    {
      long xDelta, yDelta;
      long overlap = 0;

      xDelta = pdsurf->x - pdsurfExisting->x;
      yDelta = pdsurf->y - pdsurfExisting->y;

      if (xDelta >= 0)
      {
        if (xDelta < pdsurfExisting->cx)
          overlap |= 1;
      }
      else
      {
        if (-xDelta < pdsurf->cx)
          overlap |= 1;
      }

      if (yDelta >= 0)
      {
        if (yDelta < pdsurfExisting->cy)
          overlap |= 2;
      }
      else
      {
        if (-yDelta < pdsurf->cy)
          overlap |= 2;
      }

      if (3 == overlap)
      {
        DISPDBG((0, "Overlapping rectangles!"));
        DISPDBG((0, "  new  rect => (x,y)=%lX,%lX (cx,cy)=%lX,%lX",
                 pdsurf->x, pdsurf->y, pdsurf->cx, pdsurf->cy));
        DISPDBG((0, "  prev rect => (x,y)=%lX,%lX (cx,cy)=%lX,%lX",
                 pdsurfExisting->x,  pdsurfExisting->y,
                 pdsurfExisting->cx, pdsurfExisting->cy));
        ASSERTDD(overlap != 3, "");
      }

      pdsurfExisting = pdsurfExisting->pdsurfDiscardableNext;
    }
  }
}
#endif

/******************************Public*Routine******************************\
* DSURF* pVidMemAllocate
*
\**************************************************************************/

DSURF* pVidMemAllocate(
PDEV*       ppdev,
LONG        cx,
LONG        cy,
DSURF*      pdsurfIn)
{
    ULONG               iHeap;
    VIDEOMEMORY*        pvmHeap;
    FLATPTR             fpVidMem;
    DSURF*              pdsurf;
    LONG                lDelta;
    SURFACEALIGNMENT    Alignment;
#ifdef DFB_INSTRUMENTATION
    LONGLONG  startTick, endTick;
#endif

    for (iHeap = 0; iHeap < ppdev->cHeaps; iHeap++)
    {
        pvmHeap = &ppdev->pvmList[iHeap];

        // NULL pointer check.
        if (pvmHeap == NULL)
        {
            DISPDBG((0, "Warning: pVidMemAllocate, pvmHeap was NULL!"));
            return(NULL);
        }
        memset(&Alignment, 0, sizeof(Alignment));

        if (VIDMEM_ISRECTANGULAR & pvmHeap->dwFlags)
        {
          // tile alignment
          Alignment.Rectangular.dwXAlignment = SST_TILE_WIDTH;
          Alignment.Rectangular.dwYAlignment = SST_TILE_HEIGHT;
        }
        else
        {
          // dword alignment
          Alignment.Linear.dwStartAlignment = 4;
          Alignment.Linear.dwPitchAlignment = 4;
        }

        // AGP memory could be potentially used for device-bitmaps, with
        // two very large caveats:
        //
        // 1. No kernel-mode view is made for the AGP memory (would take
        //    up too many PTEs and too much virtual address space).
        //    No user-mode view is made either unless a DirectDraw
        //    application happens to be running.  Consequently, neither
        //    GDI nor the driver can use the CPU to directly access the
        //    bits.  (It can be done through the accelerator, however.)
        //
        // 2. AGP heaps never shrink their committed allocations.  The
        //    only time AGP memory gets de-committed is when the entire
        //    heap is empty.  And don't forget that committed AGP memory
        //    is non-pageable.  Consequently, if you were to enable a
        //    50 MB AGP heap for DirectDraw, and were sharing that heap
        //    for device bitmap allocations, after running a D3D game
        //    the system would never be able to free that 50 MB of non-
        //    pageable memory until every single device bitmap was deleted!
        //    Just watch your Winstone scores plummet if someone plays
        //    a D3D game first.

#if ENABLE_LINEAR_DFBS
        //if (!(pvmHeap->dwFlags & (VIDMEM_ISNONLOCAL | VIDMEM_ISLINEAR)))      // use this if you only want dfb's in tiled memory
        //if (!(pvmHeap->dwFlags & (VIDMEM_ISNONLOCAL | VIDMEM_ISRECTANGULAR))) // use this if you only want dfb's in linear memory
        if (!(pvmHeap->dwFlags & (VIDMEM_ISNONLOCAL)))                        // use this if you want to allow dfb's in both tiled and linear memory
#else
        if (!(pvmHeap->dwFlags & (VIDMEM_ISNONLOCAL | VIDMEM_ISLINEAR)))
#endif
        {
#ifdef DFB_INSTRUMENTATION
            EngQueryPerformanceCounter(&startTick);
#endif
            fpVidMem = HeapVidMemAllocAligned(pvmHeap,
                                              cx * ppdev->cjPelSize,
                                              cy,
                                              &Alignment,
                                              &lDelta);
#ifdef DFB_INSTRUMENTATION
            EngQueryPerformanceCounter(&endTick);
#endif
            if (fpVidMem != 0)
            {
                if ( pdsurfIn == NULL )
                {
                    pdsurf = EngAllocMem(FL_ZERO_MEMORY, sizeof(DSURF), ALLOC_TAG);
                }
                else
                {
                    pdsurf = pdsurfIn;
                }

                if (pdsurf != NULL)
                {
                    pdsurf->dt       = DT_SCREEN;
                    pdsurf->ppdev    = ppdev;
#if !ENABLE_RECONFIG_VIDMEM
                    pdsurf->x = (LONG)((fpVidMem - ppdev->ulScreenOffset) % ppdev->lDelta)
                              / ppdev->cjPelSize;
                    pdsurf->y = (LONG)((fpVidMem - ppdev->ulScreenOffset) / ppdev->lDelta);
#endif
                    pdsurf->cx       = cx;
                    pdsurf->cy       = cy;
#if ENABLE_RECONFIG_VIDMEM
                    if (fpVidMem >= _FF(ddTileMark))
                    {
                      pdsurf->fpVidMem = LfbPtrToHwPtr(ppdev, fpVidMem);
                      pdsurf->fpVidMem |= SSTG_IS_TILED;
                    }
                    else
#endif
                      pdsurf->fpVidMem = fpVidMem;
                    pdsurf->lDelta   = lDelta;
                    pdsurf->pvmHeap  = pvmHeap;

#if !ENABLE_RECONFIG_VIDMEM
                    DISPDBG((2, "Allocated dfb (%lXh x %lXh) at (%lXh, %lXh), fpVidMem(hwPtr)=%8lXh, pitch=%8lXh, lfbPtr=%8lXh, pdsurf=%8lXh",
                                cx, cy, pdsurf->x, pdsurf->y, pdsurf->fpVidMem, lDelta, fpVidMem, pdsurf));
#else
                    DISPDBG((2, "Allocated dfb (%lXh x %lXh) at fpVidMem(hwPtr)=%8lXh, pitch=%8lXh, lfbPtr=%8lXh, pdsurf=%8lXh",
                                cx, cy, pdsurf->fpVidMem, lDelta, fpVidMem, pdsurf));
#endif

                    #if !(ENABLE_LINEAR_DFBS)
                        // This assert does not apply to linear DFBs.
                        // They are allowed to wrap around "past" the right
                        // edge of memory.
                        ASSERTDD(pdsurf->x + pdsurf->cx <= ppdev->cxMemory,
                                "surface past right edge of memory");
                    #endif

#ifdef DFB_INSTRUMENTATION
                    DfbInfo.createCount++;
                    DfbInfo.totalCreateTicks += (endTick - startTick);
#endif

                    PAINT_DFB_RECT(ppdev,
                                   pdsurf->fpVidMem,
                                   0,
                                   H3_PACKXY_FAST(pdsurf->cx,pdsurf->cy),
                                   pdsurf->lDelta,
                                   AllocFillColor);
                    CHECK_OVERLAP(ppdev, pdsurf);

                    return(pdsurf);
                }

                VidMemFree(pvmHeap->lpHeap, fpVidMem);
            }
        }
    }

    return(NULL);
}

/******************************Public*Routine******************************\
* VOID vVidMemFree
*
\**************************************************************************/

VOID vVidMemFree(
DSURF*  pdsurf)
{
    DSURF*  pTmp;
#ifdef DFB_INSTRUMENTATION
    LONGLONG  startTick, endTick;
#endif

    if (pdsurf == NULL)
        return;

    if (!(pdsurf->dt & DT_DIRECTDRAW))
    {
        if (pdsurf->dt & DT_DIB)
        {
            EngFreeMem(pdsurf->pvScan0);
        }
        else
        {
            DWORD pvScan0;

            if (pdsurf->fpVidMem & SSTG_IS_TILED)
              pvScan0 = HwPtrToLfbPtr(pdsurf->ppdev, pdsurf->fpVidMem & ~SSTG_IS_TILED);
            else
              pvScan0 = pdsurf->fpVidMem;

            // Update the uniqueness to show that space has been freed, so
            // that we may decide to see if some DIBs can be moved back into
            // off-screen memory:

            pdsurf->ppdev->iHeapUniq++;

#if !ENABLE_RECONFIG_VIDMEM
            DISPDBG((2, "Freeing dfb %lXh x %lXh at (%lXh, %lXh), fpVidMem(hwPtr)=%8lXh, lfbPtr=%8lXh, pdsurf=%8lXh",
                     pdsurf->cx, pdsurf->cy, pdsurf->x, pdsurf->y, pdsurf->fpVidMem, pvScan0, pdsurf));
#else
            DISPDBG((2, "Freeing dfb %lXh x %lXh at fpVidMem(hwPtr)=%8lXh, lfbPtr=%8lXh, pdsurf=%8lXh",
                     pdsurf->cx, pdsurf->cy, pdsurf->fpVidMem, pvScan0, pdsurf));
#endif

#ifdef DFB_INSTRUMENTATION
            DfbInfo.deleteCount++;
            EngQueryPerformanceCounter(&startTick);
#endif

            VidMemFree(pdsurf->pvmHeap->lpHeap, pvScan0);

#ifdef DFB_INSTRUMENTATION
            EngQueryPerformanceCounter(&endTick);

            DfbInfo.totalDeleteTicks += (endTick - startTick);
#endif

            PAINT_DFB_RECT(pdsurf->ppdev,
                           pdsurf->fpVidMem,
                           0,
                           H3_PACKXY_FAST(pdsurf->cx,pdsurf->cy),
                           pdsurf->lDelta,
                           FreeFillColor);
        }
    }

    EngFreeMem(pdsurf);
}

#if 1
/******************************Public*Routine******************************\
* BOOL bMoveOffscreenDfbToDib
*
\**************************************************************************/

BOOL bMoveOffscreenDfbToDib(PDEV *ppdev, DSURF *pdsurf)
{
    LONG        lDelta;
    VOID*       pvScan0;
    RECTL       rclDst;
    POINTL      ptlSrc;
    SURFOBJ     soTmp;

    if (pdsurf != NULL)
    {
        // Make the system-memory scans quadword aligned:

        lDelta = (pdsurf->cx * ppdev->cjPelSize + 7) & ~7;

        // Note that there's no point in zero-initializing this memory:

        pvScan0 = EngAllocMem(0, lDelta * pdsurf->cy, ALLOC_TAG);

        if (pvScan0 != NULL)
        {
            // The following 'EngModifySurface' call tells GDI to
            // modify the surface to point to system-memory for
            // the bits, and changes what Drv calls we want to
            // hook for the surface.
            //
            // By specifying the surface address, GDI will convert the
            // surface to an STYPE_BITMAP surface (if necessary) and
            // point the bits to the memory we just allocated.  The
            // next time we see it in a DrvBitBlt call, the 'dhsurf'
            // field will still point to our 'pdsurf' structure.
            //
            // Note that we hook only CopyBits and BitBlt when we
            // convert the device-bitmap to a system-memory surface.
            // This is so that we don't have to worry about getting
            // DrvTextOut, DrvLineTo, etc. calls on bitmaps that
            // we've converted to system-memory -- GDI will just
            // automatically do the drawing for us.
            //
            // However, we are still interested in seeing DrvCopyBits
            // and DrvBitBlt calls involving this surface, because
            // in those calls we take the opportunity to see if it's
            // worth putting the device-bitmap back into video memory
            // (if some room has been freed up).

            if (EngModifySurface(pdsurf->hsurf,
                                 ppdev->hdevEng,
                                 HOOK_COPYBITS | HOOK_BITBLT,
                                 0,         // It's system-memory
                                 (DHSURF) pdsurf,
                                 pvScan0,
                                 lDelta,
                                 NULL))
            {
                DISPDBG((1, "Moving pdsurf %08lXh from DFB to DIB", pdsurf));
#if !ENABLE_RECONFIG_VIDMEM
                DISPDBG((1, "Throwing out %lXh x %lXh at (%lXh, %lXh), fpVidMem = %08lXh",
                         pdsurf->cx, pdsurf->cy, pdsurf->x, pdsurf->y, pdsurf->fpVidMem));
#else
                DISPDBG((1, "Throwing out %lXh x %lXh at fpVidMem = %08lXh",
                         pdsurf->cx, pdsurf->cy, pdsurf->fpVidMem));
#endif

                // First, copy the bits from off-screen memory to the DIB:

                rclDst.left   = 0;
                rclDst.top    = 0;
                rclDst.right  = pdsurf->cx;
                rclDst.bottom = pdsurf->cy;


                #if ENABLE_LINEAR_DFBS

                    // Tell vGetBits what the pitch of the two bitmaps are.
                    ppdev->lDeltaSrc = pdsurf->lDelta; // Src is linear DFB
                    ppdev->fpVidMemSrc = pdsurf->fpVidMem;

                    ppdev->lDeltaDst = lDelta; // Dst is host bitmap.
                    ppdev->fpVidMemDst = 0;

                    // If linear device bitmaps are enabled, use relative
                    // coordinates, not absolute.  All addresses are
                    // calculated based on ppdev->fpVidMemSrc.
                    ppdev->xOffset = 0;
                    ppdev->yOffset = 0;

                    ptlSrc.x = 0;
                    ptlSrc.y = 0;
                #else
                    ptlSrc.x = pdsurf->x;
                    ptlSrc.y = pdsurf->y;
                #endif

                soTmp.lDelta  = lDelta;
                soTmp.pvScan0 = pvScan0;

                vGetBits(ppdev, &soTmp, &rclDst, &ptlSrc);

                // Don't even bother checking to see if this DIB should
                // be put back into off-screen memory until the next
                // heap 'free' occurs:

                pdsurf->iUniq = pdsurf->ppdev->iHeapUniq;
                pdsurf->ppdev->iHeapUniq++;
                pdsurf->cBlt  = 0;

                // Now free the off-screen memory:

                if (pdsurf->fpVidMem & SSTG_IS_TILED)
                {
                  DISPDBG((1, "  tiled dfb - hwPtr=%8lXh, lfbPtr=%8lXh",
                           pdsurf->fpVidMem,
                           HwPtrToLfbPtr(pdsurf->ppdev, pdsurf->fpVidMem & ~SSTG_IS_TILED)));
                  VidMemFree(pdsurf->pvmHeap->lpHeap,
                             HwPtrToLfbPtr(pdsurf->ppdev, pdsurf->fpVidMem & ~SSTG_IS_TILED));
                }
                else
                {
                  DISPDBG((1, "  linear dfb - lfbPtr=%8lXh", pdsurf->fpVidMem));
                  VidMemFree(pdsurf->pvmHeap->lpHeap, pdsurf->fpVidMem);
                }

                PAINT_DFB_RECT(ppdev,
                               pdsurf->fpVidMem,
                               0,
                               H3_PACKXY_FAST(pdsurf->cx,pdsurf->cy),
                               pdsurf->lDelta,
                               FreeFillColor);

                // Remove this node from the discardable list:

                if (pdsurf == ppdev->pdsurfDiscardableList)
                {
                  // remove head of list
                  ppdev->pdsurfDiscardableList  = pdsurf->pdsurfDiscardableNext;
                }
                else
                {
                  DSURF *pdsurf2;

                  pdsurf2 = ppdev->pdsurfDiscardableList;

                  // search list for the pdsurf being moved to the host
                  // we already know the pdsurf being moved to the host
                  // isn't at the head of the list
                  // This loop assumes we will find the pdsurf before
                  // hitting the end of the list (which would cause us to
                  // dereference a NULL pointer and cause an access violation)
                  while (pdsurf2->pdsurfDiscardableNext != pdsurf)
                    pdsurf2 = pdsurf2->pdsurfDiscardableNext;

                  // remove pdsurf from list
                  pdsurf2->pdsurfDiscardableNext = pdsurf->pdsurfDiscardableNext;
                }

                pdsurf->pdsurfDiscardableNext = NULL;
                pdsurf->dt                    = DT_DIB;
                pdsurf->pvScan0               = pvScan0;
                pdsurf->lDelta                = lDelta;

                return(TRUE);
            }

            EngFreeMem(pvScan0);
        }
    }

    return(FALSE);
}

/******************************Public*Routine******************************\
* BOOL bMoveOldestOffscreenDfbToDib
*
\**************************************************************************/

BOOL bMoveOldestOffscreenDfbToDib(PDEV *ppdev)
{
  return bMoveOffscreenDfbToDib(ppdev, ppdev->pdsurfDiscardableList);
}

/******************************Public*Routine******************************\
* BOOL bMoveOldestOffscreenDfbInHeapToDib
*
* Move dfb from a particular heap to system memory
\**************************************************************************/

BOOL bMoveOffscreenDfbInHeapToDib(PDEV        *ppdev,
                                  VIDEOMEMORY *pvmHeap,
                                  DWORD       width,
                                  DWORD       height)
{
  DSURF *pdsurf;
  DSURF *pdsurfOldest;
  DSURF *pdsurfBestFit;


  pdsurf        = ppdev->pdsurfDiscardableList;
  pdsurfOldest  = NULL;
  pdsurfBestFit = NULL;

  // loop over dfb's in this heap and find the smallest dfb that
  // fits the requested size and the oldest dfb
  while (NULL != pdsurf)
  {
    if (pdsurf->pvmHeap == pvmHeap)
    {
      // this dfb is in the requested heap

      // keep track of the oldest dfb in this heap
      if (NULL == pdsurfOldest)
      {
        DISPDBG((2, "  found oldest dfb in heap, pdsurf=%8lXh, size=%lXhx%8lXh",
                 pdsurf, pdsurf->cx, pdsurf->cy));
        pdsurfOldest = pdsurf;
      }

      // keep track of the dfb with the closest size to the requested size
      if ((pdsurf->cx >= (LONG)width) && (pdsurf->cy >= (LONG)height))
      {
        // if we don't have a best fit pdsurf yet, then use this one
        if (NULL == pdsurfBestFit)
        {
          DISPDBG((2, "  found best fit dfb in heap, pdsurf=%8lXh, size=%lXhx%8lXh",
                   pdsurf, pdsurf->cx, pdsurf->cy));
          pdsurfBestFit = pdsurf;
        }
        else
        {
          // if we already have a best fit, then see if this one's size
          // more closely matches the requested size
          if ((pdsurf->cx < pdsurfBestFit->cx) || (pdsurf->cy < pdsurfBestFit->cy))
          {
            DISPDBG((2, "  found new best fit dfb in heap, pdsurf=%8lXh, size=%lXhx%8lXh",
                     pdsurf, pdsurf->cx, pdsurf->cy));
            pdsurfBestFit = pdsurf;
          }
        }
      }
    }
    // step to next dfb
    pdsurf = pdsurf->pdsurfDiscardableNext;
  }

  // if we found a best fit pdsurf, then kick it out
  if (pdsurfBestFit)
    return bMoveOffscreenDfbToDib(ppdev, pdsurfBestFit);
  // if we didn't find a best fit pdsurf, then kick out the oldest dfb
  else if (pdsurfOldest)
    return bMoveOffscreenDfbToDib(ppdev, pdsurfOldest);
  // we didn't find any dfb's in this heap!
  else
    return FALSE;
}
#else
/******************************Public*Routine******************************\
* BOOL bMoveOldestOffscreenDfbToDib
*
\**************************************************************************/

BOOL bMoveOldestOffscreenDfbToDib(
PDEV*   ppdev)
{
    DSURF*      pdsurf;
    LONG        lDelta;
    VOID*       pvScan0;
    RECTL       rclDst;
    POINTL      ptlSrc;
    SURFOBJ     soTmp;

    pdsurf = ppdev->pdsurfDiscardableList;
    if (pdsurf != NULL)
    {
        // Make the system-memory scans quadword aligned:

        lDelta = (pdsurf->cx * ppdev->cjPelSize + 7) & ~7;

        // Note that there's no point in zero-initializing this memory:

        pvScan0 = EngAllocMem(0, lDelta * pdsurf->cy, ALLOC_TAG);

        if (pvScan0 != NULL)
        {
            // The following 'EngModifySurface' call tells GDI to
            // modify the surface to point to system-memory for
            // the bits, and changes what Drv calls we want to
            // hook for the surface.
            //
            // By specifying the surface address, GDI will convert the
            // surface to an STYPE_BITMAP surface (if necessary) and
            // point the bits to the memory we just allocated.  The
            // next time we see it in a DrvBitBlt call, the 'dhsurf'
            // field will still point to our 'pdsurf' structure.
            //
            // Note that we hook only CopyBits and BitBlt when we
            // convert the device-bitmap to a system-memory surface.
            // This is so that we don't have to worry about getting
            // DrvTextOut, DrvLineTo, etc. calls on bitmaps that
            // we've converted to system-memory -- GDI will just
            // automatically do the drawing for us.
            //
            // However, we are still interested in seeing DrvCopyBits
            // and DrvBitBlt calls involving this surface, because
            // in those calls we take the opportunity to see if it's
            // worth putting the device-bitmap back into video memory
            // (if some room has been freed up).

            if (EngModifySurface(pdsurf->hsurf,
                                 ppdev->hdevEng,
                                 HOOK_COPYBITS | HOOK_BITBLT,
                                 0,         // It's system-memory
                                 (DHSURF) pdsurf,
                                 pvScan0,
                                 lDelta,
                                 NULL))
            {
                DISPDBG((1, "Moving pdsurf %08lXh from DFB to DIB", pdsurf));
#if !ENABLE_RECONFIG_VIDMEM
                DISPDBG((1, "Throwing out %lXh x %lXh at (%lXh, %lXh), fpVidMem = %08lXh",
                         pdsurf->cx, pdsurf->cy, pdsurf->x, pdsurf->y, pdsurf->fpVidMem));
#else
                DISPDBG((1, "Throwing out %lXh x %lXh at fpVidMem = %08lXh",
                         pdsurf->cx, pdsurf->cy, pdsurf->fpVidMem));
#endif

                // First, copy the bits from off-screen memory to the DIB:

                rclDst.left   = 0;
                rclDst.top    = 0;
                rclDst.right  = pdsurf->cx;
                rclDst.bottom = pdsurf->cy;


                #if ENABLE_LINEAR_DFBS

                    // Tell vGetBits what the pitch of the two bitmaps are.
                    ppdev->lDeltaSrc = pdsurf->lDelta; // Src is linear DFB
                    ppdev->fpVidMemSrc = pdsurf->fpVidMem;

                    ppdev->lDeltaDst = lDelta; // Dst is host bitmap.
                    ppdev->fpVidMemDst = 0;

                    // If linear device bitmaps are enabled, use relative
                    // coordinates, not absolute.  All addresses are
                    // calculated based on ppdev->fpVidMemSrc.
                    ppdev->xOffset = 0;
                    ppdev->yOffset = 0;

                    ptlSrc.x = 0;
                    ptlSrc.y = 0;
                #else
                    ptlSrc.x = pdsurf->x;
                    ptlSrc.y = pdsurf->y;
                #endif

                soTmp.lDelta  = lDelta;
                soTmp.pvScan0 = pvScan0;

                vGetBits(ppdev, &soTmp, &rclDst, &ptlSrc);

                // Don't even bother checking to see if this DIB should
                // be put back into off-screen memory until the next
                // heap 'free' occurs:

                pdsurf->iUniq = pdsurf->ppdev->iHeapUniq;
                pdsurf->ppdev->iHeapUniq++;
                pdsurf->cBlt  = 0;

                // Now free the off-screen memory:

                VidMemFree(pdsurf->pvmHeap->lpHeap, pdsurf->fpVidMem);

                PAINT_DFB_RECT(ppdev,
                               pdsurf->fpVidMem,
                               0,
                               H3_PACKXY_FAST(pdsurf->cx,pdsurf->cy),
                               pdsurf->lDelta,
                               FreeFillColor);

                // Remove this node from the discardable list:

                ASSERTDD(ppdev->pdsurfDiscardableList == pdsurf,
                    "Expected node to be head of the list");

                ppdev->pdsurfDiscardableList  = pdsurf->pdsurfDiscardableNext;

                pdsurf->pdsurfDiscardableNext = NULL;
                pdsurf->dt                    = DT_DIB;
                pdsurf->pvScan0               = pvScan0;
//                #if ENABLE_LINEAR_DFBS
                    pdsurf->lDelta            = lDelta;
//                #endif

                return(TRUE);
            }

            EngFreeMem(pvScan0);
        }
    }

    return(FALSE);
}
#endif

/******************************Public*Routine******************************\
* BOOL bMoveEverythingFromOffscreenToDibs
*
* This function is used when we're about to enter full-screen mode, which
* would wipe all our off-screen bitmaps.  GDI can ask us to draw on
* device bitmaps even when we're in full-screen mode, and we do NOT have
* the option of stalling the call until we switch out of full-screen.
* We have no choice but to move all the off-screen DFBs to DIBs.
*
* Returns TRUE if all DSURFs have been successfully moved.
*
\**************************************************************************/

BOOL bMoveAllDfbsFromOffscreenToDibs(
PDEV*   ppdev)
{
    do {} while (bMoveOldestOffscreenDfbToDib(ppdev));

    return(ppdev->pdsurfDiscardableList == NULL);
}

/******************************Public*Routine******************************\
* BOOL bMoveDibToOffscreenDfbIfRoom
*
\**************************************************************************/

BOOL bMoveDibToOffscreenDfbIfRoom(
PDEV*   ppdev,
DSURF*  pdsurf)
{
#if 1		// We'll assume it works for now.
    DSURF*  pdsurfVMem;
    DSURF*  pTmp;
    RECTL   rclDst;
    POINTL  ptlSrc;
    SURFOBJ soTmp;
    DSURF   dsTmp;
    DWORD   pvScan0;

    ASSERTDD(pdsurf->dt == DT_DIB,
             "Can't move a bitmap off-screen when it's already off-screen");

    // If we're in full-screen mode, we can't move anything to off-screen
    // memory:

    if (!ppdev->bEnabled)
        return(FALSE);

    // In case Windows can't handle the modify surface call we save the surf
    // so we can recover.  It's probably faster for us to do this than to let
    // MS alloc and free memory elsewhere.

    dsTmp = *pdsurf;

    pdsurfVMem = pVidMemAllocate(ppdev, pdsurf->cx, pdsurf->cy, pdsurf);

    if (pdsurfVMem == NULL)
    {
        // There wasn't any free room.

        return(FALSE);
    }

    if (pdsurfVMem->fpVidMem & SSTG_IS_TILED)
      pvScan0 = HwPtrToLfbPtr(ppdev, pdsurfVMem->fpVidMem & ~SSTG_IS_TILED);
    else
      pvScan0 = pdsurfVMem->fpVidMem;

    // If we're running on a card that can map all of off-screen
    // video-memory, give a pointer to the bits to GDI so that
    // it can draw directly on the bits when it wants to.
    //
    // Note that this requires that we hook DrvSynchronize and
    // set HOOK_SYNCHRONIZE.

    if (!EngModifySurface(pdsurf->hsurf,
                         ppdev->hdevEng,
                         ppdev->flHooks |
                         HOOK_SYNCHRONIZE,
                         MS_NOTSYSTEMMEMORY,    // It's video-memory
                         (DHSURF) pdsurfVMem,
                         ppdev->pjScreenBase + pvScan0,
                 #if ENABLE_LINEAR_DFBS
                         pdsurfVMem->lDelta,
                 #else
                         ppdev->lDelta,
                 #endif
                         NULL))
    {
        VidMemFree(pdsurfVMem->pvmHeap->lpHeap, pdsurfVMem->fpVidMem);
        *pdsurf = dsTmp;
        return (FALSE);
    }

#if ENABLE_LINEAR_DFBS
#if ENABLE_TILED_HEAP
    if (pdsurfVMem->fpVidMem & SSTG_IS_TILED)
      ppdev->lDeltaDst = _FF(ddTileStride);
    else
#endif
      ppdev->lDeltaDst = pdsurfVMem->lDelta;
    ppdev->fpVidMemDst = pdsurfVMem->fpVidMem;
#endif

#if !ENABLE_RECONFIG_VIDMEM
    // Copy the DIB to the offscreen DFB

    ptlSrc.x      = dsTmp.x;
    ptlSrc.y      = dsTmp.y;

    rclDst.left   = dsTmp.x;
    rclDst.top    = dsTmp.y;
    rclDst.right  = rclDst.left + dsTmp.cx;
    rclDst.bottom = rclDst.top + dsTmp.cy;
#else
    // Copy the DIB to the offscreen

    ptlSrc.x      = 0;
    ptlSrc.y      = 0;

    rclDst.left   = 0;
    rclDst.top    = 0;
    rclDst.right  = dsTmp.cx;
    rclDst.bottom = dsTmp.cy;
#endif

    soTmp.lDelta  = dsTmp.lDelta;
    soTmp.pvScan0 = dsTmp.pvScan0;
#ifdef DBG
    soTmp.iBitmapFormat = ppdev->iBitmapFormat;
#endif

    vPutBits(ppdev, &soTmp, &rclDst, &ptlSrc);

    // Add this to the tail of the discardable surface list:

    if (ppdev->pdsurfDiscardableList == NULL)
        ppdev->pdsurfDiscardableList = pdsurfVMem;
    else
    {
        for (pTmp = ppdev->pdsurfDiscardableList;
             pTmp->pdsurfDiscardableNext != NULL;
             pTmp = pTmp->pdsurfDiscardableNext)
           ;

        pTmp->pdsurfDiscardableNext = pdsurfVMem;
    }

    // Now free the DIB (but keep the surf).

    if (!(pdsurf->dt & DT_DIRECTDRAW))
    {
        EngFreeMem(dsTmp.pvScan0);
    }

    return(TRUE);
#else
    return(FALSE);
#endif
}

/******************************Public*Routine******************************\
* HBITMAP DrvCreateDeviceBitmap
*
* Function called by GDI to create a device-format-bitmap (DFB).  We will
* always try to allocate the bitmap in off-screen; if we can't, we simply
* fail the call and GDI will create and manage the bitmap itself.
*
* Note: We do not have to zero the bitmap bits.  GDI will automatically
*       call us via DrvBitBlt to zero the bits (which is a security
*       consideration).
*
\**************************************************************************/

HBITMAP DrvCreateDeviceBitmap(
DHPDEV  dhpdev,
SIZEL   sizl,
ULONG   iFormat)
{
    PDEV*   ppdev;
    DSURF*  pdsurf;
    HBITMAP hbmDevice;
    BYTE*   pjSurface;
    LONG    lDelta;
    FLONG   flHooks;
    DSURF*  pTmp;

    // We don't need a Glide exclusive test because the DDraw test below should catch it.

    ppdev = (PDEV*) dhpdev;

    // If we're in full-screen mode, we hardly have any off-screen memory
    // in which to allocate a DFB.

    if (!ppdev->bEnabled)
        return(0);

    // if we didn't enable ddraw, then we can't get dfb's this way!
    if (! (ppdev->flStatus & STAT_DIRECTDRAW))
      return 0;

    //
    // Check If we're in DirectDraw exclusive mode
    //
    if ( _DS(ddExclusiveMode) == TRUE )
    {
        DISPDBG((2, "DrvCreateDeviceBitmap(): return 0, DirectDraw exclusive mode"));

        return (0);
    }

    // We only support device bitmaps that are the same colour depth
    // as our display.
    //
    // Actually, those are the only kind GDI will ever call us with,
    // but we may as well check.  Note that this implies you'll never
    // get a crack at 1bpp bitmaps.

    if (iFormat != ppdev->iBitmapFormat)
        return(0);

    // We don't want anything 8x8 or smaller -- they're typically brush
    // patterns which we don't particularly want to stash in off-screen
    // memory.
    //
    // Note if you're tempted to extend this philosophy to surfaces
    // larger than 8x8: in NT5, software cursors will use device-bitmaps
    // when possible, which is a big win when they're in video-memory
    // because we avoid the horrendous reads from video memory whenever
    // the cursor has to be redrawn.  But the problem is that thos suckers
    // are small!  (Typically 16x16 to 32x32.)

    if ((sizl.cx <= 8) && (sizl.cy <= 8))
        return(0);

    // banshee, v3 & napalm clip registers only have 12 bits so fail
    // requests for dfb's with a width or height larger than 4kB
    if ((4095 < sizl.cx) || (4095 < sizl.cy))
        return 0;

    do {
        pdsurf = pVidMemAllocate(ppdev, sizl.cx, sizl.cy, NULL);
        if (pdsurf != NULL)
        {
            hbmDevice = EngCreateDeviceBitmap((DHSURF) pdsurf, sizl, iFormat);
            if (hbmDevice != NULL)
            {
                DWORD pvScan0;

                if (pdsurf->fpVidMem & SSTG_IS_TILED)
                  pvScan0 = HwPtrToLfbPtr(ppdev, pdsurf->fpVidMem & ~SSTG_IS_TILED);
                else
                  pvScan0 = pdsurf->fpVidMem;

                // If we're running on a card that can map all of off-screen
                // video-memory, give a pointer to the bits to GDI so that
                // it can draw directly on the bits when it wants to.
                //
                // Note that this requires that we hook DrvSynchronize and
                // set HOOK_SYNCHRONIZE.

                if (EngModifySurface((HSURF) hbmDevice,
                                     ppdev->hdevEng,
                                     ppdev->flHooks |
                                     HOOK_SYNCHRONIZE,
                                     MS_NOTSYSTEMMEMORY,    // It's video-memory
                                     (DHSURF) pdsurf,
                                     ppdev->pjScreenBase + pvScan0,
                                     #if ENABLE_LINEAR_DFBS
                                     pdsurf->lDelta,
                                     #else
                                     ppdev->lDelta,
                                     #endif
                                     NULL))
                {
                    pdsurf->hsurf = (HSURF) hbmDevice;

                    // Add this to the tail of the discardable surface list:

                    if (ppdev->pdsurfDiscardableList == NULL)
                        ppdev->pdsurfDiscardableList = pdsurf;
                    else
                    {
                        for (pTmp = ppdev->pdsurfDiscardableList;
                             pTmp->pdsurfDiscardableNext != NULL;
                             pTmp = pTmp->pdsurfDiscardableNext)
                            ;

                        pTmp->pdsurfDiscardableNext = pdsurf;
                    }

                    return(hbmDevice);
                }

                EngDeleteSurface((HSURF) hbmDevice);
            }

            vVidMemFree(pdsurf);

            return(0);
        }
    } while (bMoveOldestOffscreenDfbToDib(ppdev));

    return(0);
}

/******************************Public*Routine******************************\
* HBITMAP DrvDeriveSurface
*
* This function is new to NT5, and allows the driver to accelerate any
* GDI drawing to a DirectDraw surface.
*
* Note the similarity of this function to DrvCreateDeviceBitmap.
*
\**************************************************************************/

HBITMAP DrvDeriveSurface(
DD_DIRECTDRAW_GLOBAL*   lpDirectDraw,
DD_SURFACE_LOCAL*       lpLocal)
{
    PDEV*               ppdev;
    DSURF*              pdsurf;
    HBITMAP             hbmDevice;
    DD_SURFACE_GLOBAL*  lpSurface;
    SIZEL               sizl;
    VIDEOMEMORY*        pvmHeap;
    FXSURFACEDATA       *pSurfaceData;

    DISPDBG((5, ">> DrvDeriveSurface"));

    ppdev = (PDEV*) lpDirectDraw->dhpdev;

    lpSurface = lpLocal->lpGbl;

    // GDI should never call us for a non-RGB surface, but let's assert just
    // to make sure they're doing their job properly.

    ASSERTDD(!(lpSurface->ddpfSurface.dwFlags & DDPF_FOURCC),
        "GDI called us with a non-RGB surface!");

#if !ENABLE_LINEAR_DFBS
    // hack to not allow gdi to access ddraw surfaces
    // in the linear heap

    if (ppdev->ulScreenOffset != lpSurface->fpVidMem)
    {
      pSurfaceData = (FXSURFACEDATA *)lpSurface->dwReserved1;
      ASSERTDD(pSurfaceData != NULL, "pSurfaceData == NULL");
      pvmHeap = (VIDEOMEMORY *)pSurfaceData->pvmHeap;
      if (VIDMEM_ISLINEAR & pvmHeap->dwFlags)
      {
        DISPDBG((6, "<< DrvDeriveSurface"));
        return 0;
      }
    }
#endif

    // The rest of our driver expects GDI calls to come in with the same
    // format as the primary surface.  So we'd better not wrap a device
    // bitmap around an RGB format that the rest of our driver doesn't
    // understand.

    if (lpSurface->ddpfSurface.dwRGBBitCount == (DWORD) ppdev->cjPelSize * 8)
    {
        pdsurf = EngAllocMem(FL_ZERO_MEMORY, sizeof(DSURF), ALLOC_TAG);
        if (pdsurf != NULL)
        {
            sizl.cx = lpSurface->wWidth;
            sizl.cy = lpSurface->wHeight;

            hbmDevice = EngCreateDeviceBitmap((DHSURF) pdsurf,
                                              sizl,
                                              ppdev->iBitmapFormat);
            if (hbmDevice != NULL)
            {
                // Note that HOOK_SYNCHRONIZE must always be hooked when
                // we give GDI a pointer to the bitmap bits.

                if (EngModifySurface((HSURF) hbmDevice,
                                     ppdev->hdevEng,
                                     ppdev->flHooks | HOOK_SYNCHRONIZE,
                                     MS_NOTSYSTEMMEMORY,    // It's video-memory
                                     (DHSURF) pdsurf,
                                     ppdev->pjScreenBase + lpSurface->fpVidMem,
                                     lpSurface->lPitch,
                                     NULL))
                {
                    pdsurf->dt       = DT_DIRECTDRAW;
                    pdsurf->ppdev    = ppdev;
#if !ENABLE_RECONFIG_VIDMEM
                    pdsurf->x        = lpSurface->xHint;
                    pdsurf->y        = lpSurface->yHint;
#endif
                    pdsurf->cx       = lpSurface->wWidth;
                    pdsurf->cy       = lpSurface->wHeight;
#if ENABLE_RECONFIG_VIDMEM
                    if (lpSurface->fpVidMem >= _FF(ddTileMark))
                    {
                      if (_DD(ddSLIModeEnabled))
                        pdsurf->fpVidMem = SLI_LfbPtrToHwPtr(ppdev, lpSurface->fpVidMem);
                      else
                        pdsurf->fpVidMem = LfbPtrToHwPtr(ppdev, lpSurface->fpVidMem);
                      pdsurf->fpVidMem |= SSTG_IS_TILED;
                    }
                    else
#endif
                      pdsurf->fpVidMem = lpSurface->fpVidMem;
                    pdsurf->lDelta   = lpSurface->lPitch;

                    DISPDBG((2, "Derive dfb from ddraw surface (%lXh x %lXh) at fpVidMem = %08lXh, pitch = %08lXh, pdsurf = %08lXh",
                                pdsurf->cx, pdsurf->cy, pdsurf->fpVidMem, pdsurf->lDelta, pdsurf));
                    DISPDBG((6, "<< DrvDeriveSurface"));
                    return(hbmDevice);
                }

                EngDeleteSurface((HSURF) hbmDevice);
            }

            EngFreeMem(pdsurf);
        }
    }

    DISPDBG((6, "<< DrvDeriveSurface"));
    return(0);
}

/******************************Public*Routine******************************\
* VOID DrvDeleteDeviceBitmap
*
* Deletes a DFB.
*
\**************************************************************************/

VOID DrvDeleteDeviceBitmap(
DHSURF  dhsurf)
{
    DSURF*  pdsurf;
    PDEV*   ppdev;
    DSURF*  pTmp;

    pdsurf = (DSURF*) dhsurf;

    ppdev = pdsurf->ppdev;

    if ((pdsurf->dt & (DT_DIB | DT_DIRECTDRAW)) == 0)
    {
        // It's a surface stashed in video memory, so we have to remove
        // it from the discardable surface list:

        if (ppdev->pdsurfDiscardableList == pdsurf)
            ppdev->pdsurfDiscardableList = pdsurf->pdsurfDiscardableNext;
        else
        {
            for (pTmp = ppdev->pdsurfDiscardableList;
                 pTmp->pdsurfDiscardableNext != pdsurf;
                 pTmp = pTmp->pdsurfDiscardableNext)
                ;

            pTmp->pdsurfDiscardableNext = pdsurf->pdsurfDiscardableNext;
        }
    }

    vVidMemFree(pdsurf);
}

/******************************Public*Routine******************************\
* BOOL bAssertModeOffscreenHeap
*
* This function is called whenever we switch in or out of full-screen
* mode.  We have to convert all the off-screen bitmaps to DIBs when
* we switch to full-screen (because we may be asked to draw on them even
* when in full-screen, and the mode switch would probably nuke the video
* memory contents anyway).
*
\**************************************************************************/

BOOL bAssertModeOffscreenHeap(
PDEV*   ppdev,
BOOL    bEnable)
{
    BOOL b;

    b = TRUE;

    if (!bEnable)
    {
        b = bMoveAllDfbsFromOffscreenToDibs(ppdev);
    }

    return(b);
}

/******************************Public*Routine******************************\
* VOID vDisableOffscreenHeap
*
* Frees any resources allocated by the off-screen heap.
*
\**************************************************************************/

VOID vDisableOffscreenHeap(
PDEV*   ppdev)
{
    SURFOBJ* psoPunt;
    HSURF    hsurf;

    psoPunt = ppdev->psoPunt;
    if (psoPunt != NULL)
    {
        hsurf = psoPunt->hsurf;
        EngUnlockSurface(psoPunt);
        EngDeleteSurface(hsurf);
    }

    psoPunt = ppdev->psoPunt2;
    if (psoPunt != NULL)
    {
        hsurf = psoPunt->hsurf;
        EngUnlockSurface(psoPunt);
        EngDeleteSurface(hsurf);
    }
}

/******************************Public*Routine******************************\
* BOOL bEnableOffscreenHeap
*
* Initializes the off-screen heap using all available video memory,
* accounting for the portion taken by the visible screen.
*
\**************************************************************************/

BOOL bEnableOffscreenHeap(
PDEV*   ppdev)
{
    SIZEL   sizl;
    HSURF   hsurf;

    // Allocate a 'punt' SURFOBJ we'll use when the device-bitmap is in
    // off-screen memory, but we want GDI to draw to it directly as an
    // engine-managed surface:

    sizl.cx = ppdev->cxMemory;
    sizl.cy = ppdev->cyMemory;

    // We want to create it with exactly the same hooks and capabilities
    // as our primary surface.  We will override the 'lDelta' and 'pvScan0'
    // fields later:

    hsurf = (HSURF) EngCreateBitmap(sizl,
                                    0xbadf00d,
                                    ppdev->iBitmapFormat,
                                    BMF_TOPDOWN,
                                    (VOID*) 0xbadf00d);

    // We don't want GDI to turn around and call any of our Drv drawing
    // functions when drawing to these surfaces, so always set the hooks
    // to '0':

    if ((hsurf == 0)                                     ||
        (!EngAssociateSurface(hsurf, ppdev->hdevEng, 0)) ||
        (!(ppdev->psoPunt = EngLockSurface(hsurf))))
    {
        DISPDBG((1, "Failed punt surface creation"));

        EngDeleteSurface(hsurf);
        goto ReturnFalse;
    }

    // We don't want GDI to turn around and call any of our Drv drawing
    // functions when drawing to these surfaces, so always set the hooks
    // to '0':

    hsurf = (HSURF) EngCreateBitmap(sizl,
                                    0xbadf00d,
                                    ppdev->iBitmapFormat,
                                    BMF_TOPDOWN,
                                    (VOID*) 0xbadf00d);

    // We don't want GDI to call us back when drawing to these surfaces,
    // so always set the hooks to '0':

    if ((hsurf == 0)                                     ||
        (!EngAssociateSurface(hsurf, ppdev->hdevEng, 0)) ||
        (!(ppdev->psoPunt2 = EngLockSurface(hsurf))))
    {
        DISPDBG((1, "Failed punt surface creation"));

        EngDeleteSurface(hsurf);
        goto ReturnFalse;
    }

    DISPDBG((5, "Passed bEnableOffscreenHeap"));

#ifdef PAINT_RECTS
    switch (ppdev->cjPelSize)
    {
      case 1:
        InitFillColor  = 0x03;
        AllocFillColor = 0x0C;
        FreeFillColor  = 0x06;
        break;
      case 2:
        InitFillColor  = 0x87E0;    // light green
        AllocFillColor = 0x041F;    // light blue
        FreeFillColor  = 0xF810;    // pink
        break;
      case 3:
      case 4:
        InitFillColor  = 0x80FF00;  // light green
        AllocFillColor = 0x0080FF;  // light blue
        FreeFillColor  = 0xFF0080;  // pink
        break;
      default:
        ASSERTDD(1 == 0, "Invalid colordepth");
        break;
    }
#endif

#ifdef DFB_INSTRUMENTATION
    EngQueryPerformanceFrequency(&DfbInfo.ticksPerSecond);
#endif

    return(TRUE);

ReturnFalse:

    DISPDBG((1, "Failed bEnableOffscreenHeap"));

    return(FALSE);
}
