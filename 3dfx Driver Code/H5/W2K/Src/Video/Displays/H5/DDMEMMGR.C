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
** $Revision: 17$
** $Date: 10/11/00 8:43:53 PM$
**
*/

/**************************************************************************
* I N C L U D E S
***************************************************************************/

#include "precomp.h"

#if USE_NT5_DDMEMMGR
#define __NTDDKCOMP__
#include "dmemmgr.h"

/**************************************************************************
* D E F I N E S
***************************************************************************/

#define MIN_TILED_SURF_WIDTH    32
#define MIN_TILED_SURF_HEIGHT   32

/**************************************************************************
*
* FUNCTION:     memMgr_allocSurface
*
* DESCRIPTION:
*
***************************************************************************/

HRESULT
memMgr_allocSurface(PDEV        *ppdev,
                    DWORD       type,
                    DWORD       width,
                    DWORD       height,
                    DWORD       tWidth,
                    DWORD       tHeight,
                    DWORD       *fpVidMem,
                    DWORD       *hwVidMem,
                    DWORD       *lpPitch,
                    DWORD       *tileFlag,
                    DWORD       *heapID,
                    VIDEOMEMORY **ppvmHeap)
{
  ULONG               iHeap;
  VIDEOMEMORY*        pvmHeap;
  FLATPTR             ddPtr;
  LONG                lDelta;
  SURFACEALIGNMENT    Alignment;

  DWORD               numberHeaps;
  DWORD               searchHeaps[8];
  DWORD               tileWidth, tileHeight;
#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
  DWORD               tileMark;
#endif


  ddPtr = 0;

#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
  memMgr_checkHeapStatus(ppdev);
#endif

  if ((0 == _FF(ddTiledHeapSize)) || ((DDSCAPS_OVERLAY | DDSCAPS_TEXTURE) & type) ||
      ((width <= (DWORD)(MIN_TILED_SURF_WIDTH * GETPRIMARYBYTEDEPTH)) && (height <= (DWORD)MIN_TILED_SURF_HEIGHT)))
  {
#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
    if ((_FF(bUseSliExtraLinearHeap)) &&
        (DDSCAPS_TEXTURE & type) &&
        ((DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)) ||
         (QUAD_CHIP_SLI_4WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))))
    {
      numberHeaps = 6;
      searchHeaps[0] = SLI_EXTRA_LINEAR_HEAP_ID;
      searchHeaps[1] = LINEAR_HEAP0_ID;
      searchHeaps[2] = LINEAR_HEAP1_ID;
      searchHeaps[3] = LINEAR_HEAP2_ID;
      searchHeaps[4] = LINEAR_HEAP3_ID;
      searchHeaps[5] = LINEAR_HEAP4_ID;
    }
    else
#endif
    {
      numberHeaps = 5;
      searchHeaps[0] = LINEAR_HEAP0_ID;
      searchHeaps[1] = LINEAR_HEAP1_ID;
      searchHeaps[2] = LINEAR_HEAP2_ID;
      searchHeaps[3] = LINEAR_HEAP3_ID;
      searchHeaps[4] = LINEAR_HEAP4_ID;
    }
  }
  else if (DDSCAPS_BACKBUFFER & type)
  {
#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
    if ((_FF(bUseSliExtraLinearHeap)) &&
        ((DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)) ||
         (QUAD_CHIP_SLI_4WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))))
    {
      numberHeaps = 0;
      tileMark = _FF(ddTileMark);
      _FF(ddTileMark) = _FF(sliTileCtrl);
      ddPtr = SLI_HwPtrToLfbPtr(ppdev, _FF(sliBackBuffer));
      _FF(ddTileMark) = tileMark;
      lDelta = ppdev->lDelta;
      *tileFlag = MEM_IN_TILED;
      pvmHeap = NULL;
    }
    else
#endif
    {
      // backbuffers can only be created in fullscreen exclusive mode
      //
      // for fullscreen exclusive mode, limit allocations to tiled heap 0 when in tiled mode
      numberHeaps = 1;
      searchHeaps[0] = _DS(ddPrimaryInTile) ? TILED_HEAP0_ID : LINEAR_HEAP0_ID;
    }
  }
  else if (DDSCAPS_ZBUFFER & type)
  {
    if (_DS(ddExclusiveMode))
    {
#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
      if ((_FF(bUseSliExtraLinearHeap)) &&
          ((DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)) ||
           (QUAD_CHIP_SLI_4WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))))
      {
        numberHeaps = 0;
        tileMark = _FF(ddTileMark);
        _FF(ddTileMark) = _FF(sliTileCtrl);
        ddPtr = SLI_HwPtrToLfbPtr(ppdev, _FF(sliZBuffer));
        _FF(ddTileMark) = tileMark;
        lDelta = ppdev->lDelta;
        *tileFlag = MEM_IN_TILED;
        pvmHeap = NULL;
      }
      else
#endif
      // When in AA mode, force non-fullscreen ZBuffer
      // surfaces to be allocated from linear memory
      if ((_DD(ddAAModeRequested)) &&
           ((width != (DWORD) (ppdev->cxScreen * GETPRIMARYBYTEDEPTH)) ||
            (height != (DWORD) ppdev->cyScreen))
         )
      {
        numberHeaps = 5;
        searchHeaps[0] = LINEAR_HEAP0_ID;
        searchHeaps[1] = LINEAR_HEAP1_ID;
        searchHeaps[2] = LINEAR_HEAP2_ID;
        searchHeaps[3] = LINEAR_HEAP3_ID;
        searchHeaps[4] = LINEAR_HEAP4_ID;
      }
      else
      {
        // for fullscreen exclusive mode, allow allocation from any heap, except the tiled
        // heaps reserved for the back and third buffers, in the following order
        numberHeaps = 6;
        searchHeaps[0] = TILED_HEAP1_ID;
        searchHeaps[1] = LINEAR_HEAP0_ID;
        searchHeaps[2] = LINEAR_HEAP1_ID;
        searchHeaps[3] = LINEAR_HEAP2_ID;
        searchHeaps[4] = LINEAR_HEAP3_ID;
        searchHeaps[5] = LINEAR_HEAP4_ID;
      }
    }
    else
    {
      // to prevent fragmentation of tiled heaps by the D3DX DX7 sample apps
      // force 1x1 zbuffer surfaces to be allocated from linear memory
      if ((width == (DWORD)(1 * GETPRIMARYBYTEDEPTH)) && (height == (DWORD)1))
      {
        numberHeaps = 5;
        searchHeaps[0] = LINEAR_HEAP0_ID;
        searchHeaps[1] = LINEAR_HEAP1_ID;
        searchHeaps[2] = LINEAR_HEAP2_ID;
        searchHeaps[3] = LINEAR_HEAP3_ID;
        searchHeaps[4] = LINEAR_HEAP4_ID;
      }
      else
      {
        // for windowed mode, allow allocation from any heap in the following order
        numberHeaps = 8;
        searchHeaps[0] = TILED_HEAP1_ID;
        searchHeaps[1] = TILED_HEAP2_ID;
        searchHeaps[2] = TILED_HEAP0_ID;
        searchHeaps[3] = LINEAR_HEAP0_ID;
        searchHeaps[4] = LINEAR_HEAP1_ID;
        searchHeaps[5] = LINEAR_HEAP2_ID;
        searchHeaps[6] = LINEAR_HEAP3_ID;
        searchHeaps[7] = LINEAR_HEAP4_ID;
      }
    }
  }
  else if ((DDSCAPS_COMPLEX & type) && (DDSCAPS_FLIP & type))
  {
#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
    if ((_FF(bUseSliExtraLinearHeap)) &&
        ((DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)) ||
         (QUAD_CHIP_SLI_4WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))))
    {
      numberHeaps = 0;
      tileMark = _FF(ddTileMark);
      _FF(ddTileMark) = _FF(sliTileCtrl);
      ddPtr = SLI_HwPtrToLfbPtr(ppdev, _FF(sliThirdBuffer));
      _FF(ddTileMark) = tileMark;
      lDelta = ppdev->lDelta;
      *tileFlag = MEM_IN_TILED;
      pvmHeap = NULL;
    }
    else
#endif
    {
      // complex flip can only be created in fullscreen exclusive mode
      //
      // for fullscreen exclusive mode, limit allocations to tiled heap 2 when in tiled mode
      numberHeaps = 1;
      searchHeaps[0] = _DS(ddPrimaryInTile) ? TILED_HEAP2_ID : LINEAR_HEAP0_ID;
    }
  }
  else if (DDSCAPS_3DDEVICE & type)
  {
	  // When in AA mode, force non-FS-3DDEVICE-VIDEOMEMORY-OFFSCREENPLAIN 
    // surfaces to be allocated in linear memory.
    if (((_DD(ddAAModeRequested)) &&	// Are we in AA mode?
          ((width != (DWORD) (ppdev->cxScreen * GETPRIMARYBYTEDEPTH)) || 
          (height != (DWORD) ppdev->cyScreen)) &&  // Is the surface full screen?
          ((type & (DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN)))) ||
        // to prevent fragmentation of tiled heaps by the D3DX DX7 sample apps
        // also force 1x1 render target surfaces to be allocated from linear memory
        ((width == (DWORD)(1 * GETPRIMARYBYTEDEPTH)) && (height == (DWORD)1))
       )
    {
      numberHeaps = 5;
      searchHeaps[0] = LINEAR_HEAP0_ID;    // linear only
      searchHeaps[1] = LINEAR_HEAP1_ID;
      searchHeaps[2] = LINEAR_HEAP2_ID;
      searchHeaps[3] = LINEAR_HEAP3_ID;
      searchHeaps[4] = LINEAR_HEAP4_ID;
    }
    else
    {
      numberHeaps = 8;
      searchHeaps[0] = TILED_HEAP2_ID;
      searchHeaps[1] = TILED_HEAP0_ID;
      searchHeaps[2] = TILED_HEAP1_ID;
      searchHeaps[3] = LINEAR_HEAP0_ID;
      searchHeaps[4] = LINEAR_HEAP1_ID;
      searchHeaps[5] = LINEAR_HEAP2_ID;
      searchHeaps[6] = LINEAR_HEAP3_ID;
      searchHeaps[7] = LINEAR_HEAP4_ID;
    }
  }
  else if (DDSCAPS_LIVEVIDEO == type)
  {
    //keep it at the last case
    numberHeaps = 8;
    searchHeaps[0] = TILED_HEAP2_ID;
    searchHeaps[1] = TILED_HEAP1_ID;
    searchHeaps[2] = TILED_HEAP0_ID;
    searchHeaps[3] = LINEAR_HEAP0_ID;
    searchHeaps[4] = LINEAR_HEAP1_ID;
    searchHeaps[5] = LINEAR_HEAP2_ID;
    searchHeaps[6] = LINEAR_HEAP3_ID;
    searchHeaps[7] = LINEAR_HEAP4_ID;
  }
  else
  {
    if (_DD(ddAAModeRequested) || _DD(ddSLIModeRequested))
      numberHeaps = 5;
    else
      numberHeaps = 8;
    searchHeaps[0] = LINEAR_HEAP0_ID;
    searchHeaps[1] = LINEAR_HEAP1_ID;
    searchHeaps[2] = LINEAR_HEAP2_ID;
    searchHeaps[3] = LINEAR_HEAP3_ID;
    searchHeaps[4] = LINEAR_HEAP4_ID;
    searchHeaps[5] = TILED_HEAP2_ID;
    searchHeaps[6] = TILED_HEAP1_ID;
    searchHeaps[7] = TILED_HEAP0_ID;
  }

#if DBG && 0
  // hack, to just allocate all tiled surface from TILED_HEAP0
  // so I can hopefully see if code to kick dfb's out is working
  if (1 < numberHeaps)
  {
    numberHeaps = 2;
    searchHeaps[0] = TILED_HEAP0_ID;
    searchHeaps[1] = LINEAR_HEAP0_ID;
  }
#endif

  if ((DDSCAPS_LIVEVIDEO == type) && _FF(ddTileStride))
  {
      //make sure it is contigues like linear memory
      tileWidth =  _FF(ddTileStride)* SST_TILE_WIDTH;
      tileHeight = (width * height + tileWidth-1) / tileWidth;
      //also make sure it is big enough to not overlap the next tile block
      tileHeight = (tileHeight * _FF(ddTilePitch) + tileWidth -1 ) / tileWidth;
  } 
  else
  {
      tileWidth = tWidth << SST_TILE_WIDTH_BITS;
      tileHeight = tHeight << SST_TILE_HEIGHT_BITS;
  }


  for (iHeap = 0; iHeap < numberHeaps; iHeap++)
  {
#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
    if ((SLI_EXTRA_LINEAR_HEAP_ID == searchHeaps[iHeap]) &&
         _FF(bUseSliExtraLinearHeap))
    {
      pvmHeap = &ppdev->pvmList[ppdev->cHeaps - 1];
    }
    else
#endif
    {
      if (searchHeaps[iHeap] >= ppdev->cHeaps)
        continue;
      
      pvmHeap = &ppdev->pvmList[searchHeaps[iHeap]];
    }

    if (NULL == pvmHeap)
      continue;

    memset(&Alignment, 0, sizeof(Alignment));

    if (VIDMEM_ISRECTANGULAR & pvmHeap->dwFlags)
    {
      // tile alignment
      Alignment.Rectangular.dwXAlignment = SST_TILE_WIDTH;
      Alignment.Rectangular.dwYAlignment = SST_TILE_HEIGHT;
    }
    else
    {
      // 16 byte alignment
      Alignment.Linear.dwStartAlignment = 16;
      Alignment.Linear.dwPitchAlignment = 16;
    }

    switch (searchHeaps[iHeap])
    {
      case TILED_HEAP0_ID:
      case TILED_HEAP1_ID:
      case TILED_HEAP2_ID:
#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
        if ((_FF(bUseSliExtraLinearHeap)) &&
            ((DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)) ||
             (QUAD_CHIP_SLI_4WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))))
        {
          ddPtr = 0;
        }
        else
#endif
        {
          // loop until either the ddraw surface is successfully
          // allocated or we run out of dfb's to kick out of this heap
          do
          {
            ddPtr = HeapVidMemAllocAligned(pvmHeap,
                                           tileWidth,
                                           tileHeight,
                                           &Alignment,
                                           &lDelta);
            // if the allocation failed, kick a dfb out of this heap
            // and try the allocation again
          } while ((0 == ddPtr) && bMoveOffscreenDfbInHeapToDib(ppdev, pvmHeap, tileWidth, tileHeight));
          *tileFlag = MEM_IN_TILED;
        }
        break;

#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
      case SLI_EXTRA_LINEAR_HEAP_ID:
        ASSERTDD(_FF(bUseSliExtraLinearHeap) != 0, "  bUseSliExtraLinearHeap is FALSE!");
        if (! ((DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)) ||
               (QUAD_CHIP_SLI_4WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))))
        {
          break;
        }
        // if we're in sli mode fall thru and attempt allocation from sli extra linear heap
#endif
      case LINEAR_HEAP0_ID:
      case LINEAR_HEAP1_ID:
      case LINEAR_HEAP2_ID:
      case LINEAR_HEAP3_ID:
      case LINEAR_HEAP4_ID:
        // loop until either the ddraw surface is successfully
        // allocated or we run out of dfb's to kick out of this heap
        do
        {
          ddPtr = HeapVidMemAllocAligned(pvmHeap,
                                         width,
                                         height,
                                         &Alignment,
                                         &lDelta);
          // if the allocation failed, kick a dfb out of this heap
          // and try the allocation again
        } while ((0 == ddPtr) && bMoveOffscreenDfbInHeapToDib(ppdev, pvmHeap, width, height));
        *tileFlag = MEM_IN_LINEAR;
        break;
    }

    if (ddPtr)
    {
      *heapID = searchHeaps[iHeap];
      break;
    }
  }

  if (0 == ddPtr)
    return DDERR_OUTOFVIDEOMEMORY;

  if (DDSCAPS_3DDEVICE & type)
  {
    _FF(dd3DSurfaceCount)++;
  }

  if ((MEM_IN_LINEAR == *tileFlag))
  {
    *hwVidMem =
    *fpVidMem = ddPtr;
    *lpPitch = lDelta;
    *ppvmHeap = pvmHeap;

    DISPDBG((2, "Allocated linear ddraw surface (%lXh x %lXh) at fpVidMem = %8lXh, pitch = %8lXh",
             width, height, ddPtr, lDelta));
  }
  else
  {
    *fpVidMem = ddPtr;
#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
    if ((_FF(bUseSliExtraLinearHeap)) &&
        ((DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)) ||
         (QUAD_CHIP_SLI_4WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))))
    {
      tileMark = _FF(ddTileMark);
      _FF(ddTileMark) = _FF(sliTileCtrl);
      *hwVidMem = SLI_LfbPtrToHwPtr(ppdev, ddPtr);
      _FF(ddTileMark) = tileMark;
    }
    else
#endif
    if (_DD(ddSLIModeRequested))
      *hwVidMem = SLI_LfbPtrToHwPtr(ppdev, ddPtr);
    else
      *hwVidMem = LfbPtrToHwPtr(ppdev, ddPtr);

    if (DDSCAPS_LIVEVIDEO != type)
    {
      *hwVidMem |= SSTG_IS_TILED;
      *lpPitch  = lDelta;
    }
    else
    {
      //use linear pitch
      *lpPitch  = (width + 15 ) & ~15;
    }
    *ppvmHeap = pvmHeap;

    DISPDBG((2, "Allocated tiled ddraw surface (%lXh x %lXh) at fpVidMem = %8lXh, pitch = %8lXh, hwPtr = %8lXh",
             width, height, *fpVidMem, *lpPitch, *hwVidMem));
  }

  return DD_OK;
}

#ifdef SLI_AA
/**************************************************************************
*
* FUNCTION:     memMgr_allocSecondary
*
* DESCRIPTION:
*
***************************************************************************/

HRESULT
memMgr_allocSecondary(PDEV        *ppdev,
                      DWORD       type,
                      DWORD       width,
                      DWORD       height,
                      DWORD       tWidth,
                      DWORD       tHeight,
                      DWORD       *fpVidMem,
                      DWORD       *hwVidMem,
                      DWORD       *lpPitch,
                      DWORD       *tileFlag,
                      DWORD       *heapID,
                      VIDEOMEMORY **ppvmHeap)
{
  ULONG               iHeap;
  VIDEOMEMORY*        pvmHeap;
  FLATPTR             ddPtr, hwPtr;
  LONG                lDelta;
  SURFACEALIGNMENT    Alignment;

  DWORD               numberHeaps;
  DWORD               searchHeaps[1];
  DWORD               tileWidth, tileHeight;


  if (! _DD(ddSLIModeRequested))
  {
    // Allocate from secondary heaps in linear memory

    numberHeaps = 1;
    if (DDSCAPS_PRIMARYSURFACE & type)
    {
      // primary AA allocation
      searchHeaps[0] = LINEAR_HEAP1_ID;
    }
    else if (DDSCAPS_BACKBUFFER & type)
    {
      // backbuffer AA allocation
      searchHeaps[0] = LINEAR_HEAP2_ID;
    }
    else if (DDSCAPS_ZBUFFER & type)
    {
      // zbuffer AA allocation
      searchHeaps[0] = LINEAR_HEAP3_ID;
    }
    else if ((DDSCAPS_COMPLEX & type) && (DDSCAPS_FLIP & type))
    {
      // third buffer AA allocation
      searchHeaps[0] = LINEAR_HEAP4_ID;
    }
    
    tileWidth = tWidth << SST_TILE_WIDTH_BITS;
    tileHeight = tHeight << SST_TILE_HEIGHT_BITS;

    // make sure we allocate the whole heap
    // but if tileWidth * tileHeight is > gdiDesktopSize,
    // we want the allocation to fail
    if (tileWidth * tileHeight < _FF(gdiDesktopSize))
    {
      tileWidth = _FF(gdiDesktopSize);
      tileHeight = 1;
    }
    
    ddPtr = 0;
    for (iHeap = 0; iHeap < numberHeaps; iHeap++)
    {
      if (searchHeaps[iHeap] >= ppdev->cHeaps)
        continue;
    
      pvmHeap = &ppdev->pvmList[searchHeaps[iHeap]];
    
      memset(&Alignment, 0, sizeof(Alignment));
    
      // tile alignment
      Alignment.Linear.dwStartAlignment = SST_TILE_WIDTH;
      Alignment.Linear.dwPitchAlignment = SST_TILE_WIDTH;
    
      switch (searchHeaps[iHeap])
      {
        case LINEAR_HEAP1_ID:
        case LINEAR_HEAP2_ID:
        case LINEAR_HEAP3_ID:
        case LINEAR_HEAP4_ID:
          // loop until either the ddraw surface is successfully
          // allocated or we run out of dfb's to kick out of this heap
          do
          {
            ddPtr = HeapVidMemAllocAligned(pvmHeap,
                                           tileWidth,
                                           tileHeight,
                                           &Alignment,
                                           &lDelta);
            // if the allocation failed, kick a dfb out of this heap
            // and try the allocation again
          } while ((0 == ddPtr) && bMoveOffscreenDfbInHeapToDib(ppdev, pvmHeap, width, height));
          break;
      }
    
      if (ddPtr)
      {
        hwPtr = ddPtr | SSTG_IS_TILED;

        *heapID = searchHeaps[iHeap];
        break;
      }
    }
  }
  else
  {
    // Allocate from fixed addresses in "extra" tiled memory provided by SLI mode
    pvmHeap = NULL;
    hwPtr = 0;
    if (type & DDSCAPS_PRIMARYSURFACE)
    {
      hwPtr = _DS(secondaryFrontBuffer); // Primary (AA) Allocation
    }
    else if (type & DDSCAPS_BACKBUFFER)
    {
      hwPtr = _DS(secondaryBackBuffer);  // Back Buffer (AA) Allocation
    }
    else if (type & DDSCAPS_ZBUFFER) 
    {
      hwPtr = _DS(secondaryZBuffer);     // Zbuffer (AA) Allocation
    }
    else if ((type & DDSCAPS_COMPLEX) && (type & DDSCAPS_FLIP))
    {
      hwPtr = _DS(secondaryThirdBuffer); // Third Buffer (AA) Allocation
    }
    hwPtr |= SSTG_IS_TILED;
    *heapID = 0;

    // Translate hardware offset into linear frame buffer pointer.
    if (hwPtr)
    {
      ddPtr = SLI_HwPtrToLfbPtr(ppdev, hwPtr & ~SSTG_IS_TILED);
    }
  }

  if (0 == ddPtr)
    return DDERR_OUTOFVIDEOMEMORY;
  
  if (DDSCAPS_3DDEVICE & type)
  {
    _FF(dd3DSurfaceCount)++;
  }

  *fpVidMem = ddPtr;
  *hwVidMem = hwPtr | SSTG_IS_TILED;
  *tileFlag = MEM_IN_TILED;
  *lpPitch  = _DS(ddTilePitch);
  *ppvmHeap = pvmHeap;
  
  DISPDBG((2, "Allocated pseudo-tiled ddraw surface (%lXh x %lXh) at fpVidMem = %8lXh, pitch = %8lXh, hwPtr = %8lXh",
           width, height, *fpVidMem, lDelta, *hwVidMem));

  return DD_OK;
}
#endif

/**************************************************************************
*
* FUNCTION:     memMgr_freeSurface
*
* DESCRIPTION:
*
***************************************************************************/

void
memMgr_freeSurface(PDEV        *ppdev,
                   DWORD       type,
                   DWORD       fpVidMem,
                   DWORD       hwVidMem,
                   DWORD       tileFlag,
                   DWORD       heapID,
                   VIDEOMEMORY *pvmHeap)
{
  if (DDSCAPS_3DDEVICE & type)
  {
    _FF(dd3DSurfaceCount)--;
  }

  if (pvmHeap != NULL)
  {
    DISPDBG((2, "Freeing ddraw surface at fpVidMem = %08lXh", fpVidMem));

    VidMemFree(pvmHeap->lpHeap, fpVidMem);
  }
}

#if ENABLE_NAPALM_SLI_EXTRA_LINEAR_HEAP
/**************************************************************************
*
* FUNCTION:     memMgr_checkHeapStatus
*
* DESCRIPTION:
*
***************************************************************************/

void
memMgr_checkHeapStatus(PDEV *ppdev)
{
  VIDEOMEMORY*      pvmHeap;
  FLATPTR           ddPtr;
  LONG              lDelta;
  SURFACEALIGNMENT  Alignment;
  DWORD             width;
  DWORD             height;


  if ((IS_NAPALM) && _FF(bUseSliExtraLinearHeap))
  {
    // in sli mode, release the extra sli heap
    if ((DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)) ||
        (QUAD_CHIP_SLI_4WAY_AA_DISABLED == _DD(ddSLIAAConfiguration)))
    {
      if ((NULL != _FF(sliExtraHeap).pvmHeap) && (0 != _FF(sliExtraHeap).ddPtr))
      {
        DISPDBG((0, "memMgr_checkHeapStatus: Freeing extra sli heap at fpVidMem = %08lXh", _FF(sliExtraHeap).ddPtr));

        pvmHeap = _FF(sliExtraHeap).pvmHeap;
        VidMemFree(pvmHeap->lpHeap, _FF(sliExtraHeap).ddPtr);

        // clear these so we'll relock 
        _FF(sliExtraHeap).pvmHeap = NULL;
        _FF(sliExtraHeap).ddPtr   = 0;
      }
    }
    // in non-sli mode, lock the extra sli heap
    else
    {
      // but only if we didn't already lock it
      if (0 == _FF(sliExtraHeap).ddPtr)
      {
        pvmHeap = &ppdev->pvmList[ppdev->cHeaps - 1];
      
        memset(&Alignment, 0, sizeof(Alignment));
      
        // 16 byte alignment
        Alignment.Linear.dwStartAlignment = 16;
        Alignment.Linear.dwPitchAlignment = 16;

        // set width to full size of heap, and height to 1
        width = _FF(sliTileCompare) - _FF(ddTiledHeapStart);
        height = 1;

        // loop until either the ddraw surface is successfully
        // allocated or we run out of dfb's to kick out of this heap
        do
        {
          ddPtr = HeapVidMemAllocAligned(pvmHeap,
                                         width,
                                         height,
                                         &Alignment,
                                         &lDelta);
          // if the allocation failed, kick a dfb out of this heap
          // and try the allocation again
        } while ((0 == ddPtr) && bMoveOffscreenDfbInHeapToDib(ppdev, pvmHeap, width, height));
    
        if (0 != ddPtr)
        {
          DISPDBG((0, "memMgr_checkHeapStatus: Allocated extra sli heap (%lXh x %lXh) at fpVidMem = %8lXh, pitch = %8lXh",
                   width, height, ddPtr, lDelta));
          _FF(sliExtraHeap).ddPtr = ddPtr;
          _FF(sliExtraHeap).pvmHeap = pvmHeap;
        }
      }
    }
  }
}

#if 0
/**************************************************************************
*
* FUNCTION:     memMgr_releaseHeaps
*
* DESCRIPTION:
*
***************************************************************************/

void
memMgr_releaseHeaps(PDEV *ppdev)
{
  VIDEOMEMORY*      pvmHeap;
  FLATPTR           ddPtr;
  LONG              lDelta;
  SURFACEALIGNMENT  Alignment;
  DWORD             width;
  DWORD             height;


  if ((IS_NAPALM) && _FF(bUseSliExtraLinearHeap))
  {
    if ((NULL != _FF(sliExtraHeap).pvmHeap) && (0 != _FF(sliExtraHeap).ddPtr))
    {
      DISPDBG((0, "memMgr_releaseHeaps: Freeing extra sli heap at fpVidMem = %08lXh", _FF(sliExtraHeap).ddPtr));

      pvmHeap = _FF(sliExtraHeap).pvmHeap;
      VidMemFree(pvmHeap->lpHeap, _FF(sliExtraHeap).ddPtr);

      // clear these so we'll relock 
      _FF(sliExtraHeap).pvmHeap = NULL;
      _FF(sliExtraHeap).ddPtr   = 0;
    }
  }
}
#endif
#endif

#endif

