/* $Header: ddmemmgr.c, 22, 10/11/00 7:52:06 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
*
** File Name:	DDMEMMGR.C
**
** $Revision: 22$
** $Date: 10/11/00 7:52:06 PM$
**
*/

/*******************************************************************************
*
* EXPORTED FUNCTIONS:
*
* memMgr_allocSurface   --- Allocates memory from linear or tiled heaps.
* memMgr_allocSecondary --- Allocates a secondary buffer for antialiasing.
* memMgr_freeSurface    --- Frees memory from linear or tiled heaps.
*
* INTERNAL FUNCTIONS:
*
* HwPtrToLfbPtr         --- Translate a hwPtr in tiled memory to a lfbPtr.
* LfbPtrToHwPtr         --- Translate an lfbPtr in tiled memory to a hwPtr.
* AdjustLfbPtr          --- Adjust lfbPtr in tiled memory for SLI mode.
* AdjustHwPtr           --- Adjust a hwPtr in tiled memory for SLI mode. (obsolete)
*
*******************************************************************************/

#include "precomp.h"
#include "hw.h"
#include "fxglobal.h"

/*******************************************************************/
/*                     EXPORTED FUNCTIONS                          */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name:  memMgr_allocSurface

Description:	Allocates memory from linear and tiled heaps.

Return:         DDERR_OUTOFVIDEOMEMORY, DD_OK
----------------------------------------------------------------------*/

DWORD memMgr_allocSurface(
  LPDDRAWI_DIRECTDRAW_GBL lpDD, // direct draw vidmemalloc need this
  DWORD type,       // type of surface (can use the standard DD surface flags) [IN]
  DWORD width,      // width in byte (linear space) [IN]
  DWORD height,     // height in pixel (linear space) [IN]
  DWORD tWidth,     // width in tiled space [IN]
  DWORD tHeight,    // height in tiled space [IN]
  DWORD *fpVidMem,  // host lfb start address of allocation [OUT]
  DWORD *hwVidMem,  // hw vidmem address
  DWORD *lpPitch,   // pitch [OUT]
  DWORD *tileFlag,  // MEM_IN_TILE or MEM_IN_LINEAR [OUT]
  DWORD *heapID)    // Ddraw heap ID[OUT]
{
  DWORD heap, pitch, hwPtr, lfbPtr;
  DWORD blockSize, tileWidth, tileHeight;
  DWORD useExtraHeap;
  DWORD numberHeaps;
  DWORD searchHeaps[8];

  DD_ENTRY_SETUP(lpDD)

  /* Setup order to search heaps, based on surface type. */

  useExtraHeap = 0;
  if((width <= (DWORD) (GETPRIMARYBYTEDEPTH)) && (height == 1))
  {
    // 1x1 surface allocation - only from Linear space!
    D3DPRINT(DLALLOC,"AllocSurf: 1x1 surface");

	numberHeaps = 5;
	searchHeaps[0] = LINEAR_HEAP0_ID;    // linear only
	searchHeaps[1] = LINEAR_HEAP1_ID;
	searchHeaps[2] = LINEAR_HEAP2_ID;
	searchHeaps[3] = LINEAR_HEAP3_ID;
	searchHeaps[4] = LINEAR_HEAP4_ID;
  }
  else if (type & DDSCAPS_OVERLAY)
  {
    // Overlay Allocation
    D3DPRINT(DLALLOC,"AllocSurf: Overlay");

	numberHeaps = 5;
	searchHeaps[0] = LINEAR_HEAP0_ID;    // linear only
	searchHeaps[1] = LINEAR_HEAP1_ID;
	searchHeaps[2] = LINEAR_HEAP2_ID;
	searchHeaps[3] = LINEAR_HEAP3_ID;
	searchHeaps[4] = LINEAR_HEAP4_ID;
  }
  else if (type & DDSCAPS_TEXTURE)
  {
    // Texture Allocation
    D3DPRINT(DLALLOC, "AllocSurf: Texture");

	numberHeaps = 5;
	searchHeaps[0] = LINEAR_HEAP0_ID;    // linear only
	searchHeaps[1] = LINEAR_HEAP1_ID;
	searchHeaps[2] = LINEAR_HEAP2_ID;
	searchHeaps[3] = LINEAR_HEAP3_ID;
	searchHeaps[4] = LINEAR_HEAP4_ID;

    /* Enable extra heap in SLI/!AA mode. */

    if (_DD(ddSLIModeRequested) && (!_DD(ddAAModeRequested)))
    {
      useExtraHeap = 1;
    }
  }
  else if (type & DDSCAPS_BACKBUFFER )
  {
    // Back Buffer Allocation
    D3DPRINT(DLALLOC, "AllocSurf: Backbuffer");

    numberHeaps = 1;
    searchHeaps[0] = _DS(ddPrimaryInTile) ? TILED_HEAP0_ID : LINEAR_HEAP0_ID;
  }
  else if (type & DDSCAPS_ZBUFFER) 
  {
    // Zbuffer Allocation
    D3DPRINT(DLALLOC, "AllocSurf: ZBuffer");

    numberHeaps = 6;
    searchHeaps[0] = TILED_HEAP1_ID;
    searchHeaps[1] = LINEAR_HEAP0_ID;
    searchHeaps[2] = LINEAR_HEAP1_ID;
    searchHeaps[3] = LINEAR_HEAP2_ID;
    searchHeaps[4] = LINEAR_HEAP3_ID;
    searchHeaps[5] = LINEAR_HEAP4_ID;

    // Disallow fullscreen zbuffer allocation from linear memory,
    // when primary is tiled.

    if ((height == _FF(vres)) && _DS(ddPrimaryInTile))
    {
      // Hack for DCT 300 Blt Exotic test, which creates a 32bpp zbuffer and attaches
      // it to a 16bpp primary, then fills the 32bpp zbuffer using DdBlt and checks the
      // contents.  If we force allocation from tiled space, the test will fail in low
      // resolution modes, so this hack allows linear allocation only for these modes. - CGW-

      if (IS_NAPALM && (_FF(hres) >= 640))
      {
        numberHeaps = 1;
      }
    }
  }
  else if ((type & DDSCAPS_COMPLEX) && (type & DDSCAPS_FLIP))
  {
    // Third Buffer Allocation
    D3DPRINT(DLALLOC, "AllocSurf: Third Buffer");

    numberHeaps = 1;
    searchHeaps[0] = _DS(ddPrimaryInTile) ? TILED_HEAP2_ID : LINEAR_HEAP0_ID;
  }
  else if (type & DDSCAPS_3DDEVICE)
  {
    // 3D Offscreen Allocation
    D3DPRINT(DLALLOC, "AllocSurf: 3DDevice Offscreen");

	// When in AA mode, force non-FS-3DDEVICE-VIDEOMEMORY-OFFSCREENPLAIN surfaces
	// to be allocated in linear memory.
	if ((_DD(ddAAModeRequested)) &&															// Are we in AA mode?
	    ((width != (DWORD) (_FF(hres) * GETPRIMARYBYTEDEPTH)) || (height != _FF(vres)) ) &&	// Is the surface full screen?
		((type & DDSCAPS_VIDEOMEMORY) && (type & DDSCAPS_OFFSCREENPLAIN) ))					// Caps = 6040? - could be taken out to make the case more general !!!
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
  else if (type == DDSCAPS_LIVEVIDEO)
  {
    // Overlay Shrink surface Allocation
    D3DPRINT(DLALLOC, "AllocSurf: Overlay Shrink surface");

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
    // Generic Offscreen Allocation
    D3DPRINT(DLALLOC, "AllocSurf: Generic Offscreen");

    // Don't allow generic allocation from Tiled memory
	// once the 3D Flipping chain has been created.
	// Fixes PRS 13876
    if(_DD(dwFlags) & DDGLOBAL_3DCHAINON)
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

  /* Calculate width and height for tiled allocation, blocksize for linear allocation. */

  pitch = (width + 0xf) & ~0xf;  // 16 byte align
  blockSize = pitch * height; 

  if((DDSCAPS_LIVEVIDEO != type) || !_DS(ddTileStride))
  {
      tileWidth  = tWidth  << SST_TILE_WIDTH_BITS;
      tileHeight = tHeight << SST_TILE_HEIGHT_BITS;
  }
  else
  {
    // Make sure the allocation is continuous.
    tileWidth = _DS(ddTileStride)* SST_TILE_WIDTH;
    tileHeight = ( blockSize + tileWidth -1 ) / tileWidth;
    // Make sure allocation is large enough so there is no overlap
    // with the next tile block if it is used as linear memory.
    tileHeight = (tileHeight  + SST_TILE_HEIGHT -1 ) & ~(SST_TILE_HEIGHT- 1);
  }
  
  D3DPRINT(DLALLOC, "         : Blocksize %d", blockSize);
  D3DPRINT(DLALLOC, "         : ddNumHeap %d", _DS(ddNumHeap));

  /* Search heaps to allocate memory. */

  lfbPtr = 0;
  for (heap = 0; heap < numberHeaps; heap++)
  {
    if (searchHeaps[heap] < _DS(ddNumHeap))
    {
      switch (searchHeaps[heap])
      {
        case TILED_HEAP0_ID:   
        case TILED_HEAP1_ID:   
		case TILED_HEAP2_ID:   

          lfbPtr = DDHAL32_VidMemAlloc(lpDD, searchHeaps[heap], tileWidth, tileHeight);
          *tileFlag = MEM_IN_TILED;
          break;

        default:
        
          lfbPtr = DDHAL32_VidMemAlloc(lpDD, searchHeaps[heap], blockSize, 1);
          *tileFlag = MEM_IN_LINEAR;
          break;
      }

      if (lfbPtr)
      {
        *heapID = searchHeaps[heap];
        break;
      }
    }
  }

  /* Search extra heap, if necessary. */

  if ((useExtraHeap) && (lfbPtr == 0))
  {
    lfbPtr = DDHAL32_VidMemAlloc(lpDD, _FF(ddExtraMemoryHeap), blockSize, 1);
    *tileFlag = MEM_IN_LINEAR;
    *heapID = _FF(ddExtraMemoryHeap);
  }

  /* Could not allocate from any heap. */
  
  if(lfbPtr == 0)
  {
    D3DPRINT(DLALLOC, "         : OutOfVideoMemory, %s", (*tileFlag == MEM_IN_TILED ? "Tiled" : "Linear"));
    return (DWORD) DDERR_OUTOFVIDEOMEMORY;
  }

  if (type & DDSCAPS_3DDEVICE)
  {    
    _FF(dd3DSurfaceCount)++;
  }

  // Calculate hardware pointer.
    
  if (*tileFlag == MEM_IN_LINEAR)
  {
    hwPtr = lfbPtr - _FF(LFBBASE);

    *lpPitch = pitch;
  }
  else
  {
#ifdef SLI_AA
    // Adjust lfbPtr for SLI mode.

    lfbPtr = AdjustLfbPtr(ppdev, lfbPtr, heapID);

#endif
    // Compute hwPtr from lfbPtr.

    hwPtr = LfbPtrToHwPtr(ppdev, lfbPtr, *heapID);

    if(type != DDSCAPS_LIVEVIDEO)
    {
        hwPtr |= SSTG_IS_TILED;
        *lpPitch = _DS(ddTilePitch);
    }
    else 
    {
        hwPtr &= ~SSTG_IS_TILED;
        *lpPitch = pitch;   // Set pitch as linear surface
    }
  }

  *hwVidMem = hwPtr;
  *fpVidMem = lfbPtr;

  D3DPRINT(DLALLOC, "         : hID %d, hw %08x, fp %08x, %s",
		*heapID, *hwVidMem, *fpVidMem, (*tileFlag == MEM_IN_TILED ? "Tiled" : "Linear"));

  return DD_OK;
  
} // memMgr_allocSurface


/*----------------------------------------------------------------------
Function name:  memMgr_allocSecondary

Description:	Allocates memory from linear and tiled heaps.

Return:         DDERR_OUTOFVIDEOMEMORY, DD_OK
----------------------------------------------------------------------*/

#ifdef SLI_AA

DWORD memMgr_allocSecondary(
  LPDDRAWI_DIRECTDRAW_GBL lpDD, // direct draw vidmemalloc need this
  DWORD type,       // type of surface (can use the standard DD surface flags) [IN]
  DWORD width,      // width in byte (linear space) [IN]
  DWORD height,     // height in pixel (linear space) [IN]
  DWORD tWidth,     // width in tiled space [IN]
  DWORD tHeight,    // height in tiled space [IN]
  DWORD *fpVidMem,  // host lfb start address of allocation [OUT]
  DWORD *hwVidMem,  // hw vidmem address
  DWORD *lpPitch,   // pitch [OUT]
  DWORD *tileFlag,  // MEM_IN_TILE or MEM_IN_LINEAR [OUT]
  DWORD *heapID)    // Ddraw heap ID[OUT]
{
  DWORD heap, hwPtr, lfbPtr;
  DWORD blockSize, tileWidth, tileHeight;
  DWORD numberHeaps;
  DWORD searchHeaps[2];

  DD_ENTRY_SETUP(lpDD)

  D3DPRINT(DLALLOC, "AllocSecondary: ");

  if (!_DD(ddSLIModeRequested))
  {
    /* Allocate from secondary heaps in linear memory. */
    D3DPRINT(DLALLOC, "              : Alloc from secondary heaps in linear mem");

    numberHeaps = 1;
    if (type & DDSCAPS_PRIMARYSURFACE)
    {
      D3DPRINT(DLALLOC, "              : Primary");
      searchHeaps[0] = LINEAR_HEAP1_ID; // Primary (AA) Allocation
    }
    else if (type & DDSCAPS_BACKBUFFER)
    {
      D3DPRINT(DLALLOC, "              : BackBuffer");
      searchHeaps[0] = LINEAR_HEAP2_ID; // Back Buffer (AA) Allocation
    }
    else if (type & DDSCAPS_ZBUFFER) 
    {
      D3DPRINT(DLALLOC, "              : Z Buffer");
      searchHeaps[0] = LINEAR_HEAP3_ID; // Zbuffer (AA) Allocation
    }
    else if ((type & DDSCAPS_COMPLEX) && (type & DDSCAPS_FLIP))
    {
      D3DPRINT(DLALLOC, "              : Complex & Flip");
      searchHeaps[0] = LINEAR_HEAP4_ID; // Third Buffer (AA) Allocation
    }
  
    // Ensure correct alignment of buffers, even when SLI is enabled.

    tHeight = (tHeight + 1) & 0xFFFFFFFE; // must be multiple of 2
    tWidth = (tWidth + 1) & 0xFFFFFFFE; // must be multiple of 2

    /* Calculate width and height for tiled allocation, blocksize for linear allocation. */

    tileWidth  = tWidth  << SST_TILE_WIDTH_BITS;
    tileHeight = tHeight << SST_TILE_HEIGHT_BITS;
    blockSize = tileWidth * tileHeight;

    D3DPRINT(DLALLOC, "              : blocksize %d", blockSize);
    D3DPRINT(DLALLOC, "              : ddNumHeap %d", _DS(ddNumHeap));

    /* Search heaps to allocate memory. */

    lfbPtr = 0;
    for (heap = 0; heap < numberHeaps; heap++)
    {
      if (searchHeaps[heap] < _DS(ddNumHeap))
      {
        switch (searchHeaps[heap])
        {
          case LINEAR_HEAP1_ID:  
          case LINEAR_HEAP2_ID:  
          case LINEAR_HEAP3_ID:  
          case LINEAR_HEAP4_ID:
        
            lfbPtr = DDHAL32_VidMemAlloc(lpDD, searchHeaps[heap], blockSize, 1);
            break;
        }

        if (lfbPtr)
        {
          // Calculate hardware pointer.

          hwPtr = lfbPtr - _FF(LFBBASE);

          *heapID = searchHeaps[heap];
          break;
        }
      }
    }
  }
  else
  {
    /* Allocate from fixed addresses in "extra" tiled memory provided by SLI mode. */
    D3DPRINT(DLALLOC, "              : Alloc from fixed addr in extra tiled mem");

    hwPtr = 0;
    if (type & DDSCAPS_PRIMARYSURFACE)
    {
      D3DPRINT(DLALLOC, "              : Primary");
      hwPtr = _DS(secondaryFrontBuffer); // Primary (AA) Allocation
    }
    else if (type & DDSCAPS_BACKBUFFER)
    {
      D3DPRINT(DLALLOC, "              : BackBuffer");
      hwPtr = _DS(secondaryBackBuffer);  // Back Buffer (AA) Allocation
    }
    else if (type & DDSCAPS_ZBUFFER) 
    {
      D3DPRINT(DLALLOC, "              : Z Buffer");
      hwPtr = _DS(secondaryZBuffer);     // Zbuffer (AA) Allocation
    }
    else if ((type & DDSCAPS_COMPLEX) && (type & DDSCAPS_FLIP))
    {
      D3DPRINT(DLALLOC, "              : Complex & Flip");
      hwPtr = _DS(secondaryThirdBuffer); // Third Buffer (AA) Allocation
    }
    *heapID = HEAP_INVALID;

    // Translate hardware offset into linear frame buffer pointer.

    if (hwPtr)
    {
      hwPtr = AdjustHwPtr(ppdev, hwPtr);
      lfbPtr = HwPtrToLfbPtr(ppdev, hwPtr);
    }
  }

  /* Could not allocate from any heap. */
  
  if ((hwPtr == 0) || (lfbPtr == 0))
  {
    D3DPRINT(DLALLOC, "              : OutOfVideoMemory, %s", (*tileFlag == MEM_IN_TILED ? "Tiled" : "Linear"));
    return (DWORD) DDERR_OUTOFVIDEOMEMORY;
  }

  if (type & DDSCAPS_3DDEVICE)
  {    
    _FF(dd3DSurfaceCount)++;
  }

  // Secondary surfaces are always in tiled memory!

  *hwVidMem = hwPtr | SSTG_IS_TILED;
  *fpVidMem = lfbPtr;
  *tileFlag = MEM_IN_TILED;      
  *lpPitch = _DS(ddTilePitch);

  D3DPRINT(DLALLOC, "              : hID %d, hw %08x, fp %08x, %s",
			*heapID, *hwVidMem, *fpVidMem,  (*tileFlag == MEM_IN_TILED ? "Tiled" : "Linear"));

  return DD_OK;
  
} // memMgr_allocSecondary


#endif // SLI_AA

/*----------------------------------------------------------------------
Function name:  memMgr_freeSurface

Description:	Frees memory from linear and tiled heaps.

Return:         NONE
----------------------------------------------------------------------*/
void memMgr_freeSurface(
  LPDDRAWI_DIRECTDRAW_GBL lpDD, // direct draw vidmemfree need this
  DWORD type,                   // type of surface (can use the standard DD surface flags) [IN]
  DWORD fpVidMem,               // host lfb start address of allocation [IN]
  DWORD hwVidMem,               // hw vidmem address
  DWORD tileFlag,               // MEM_IN_TILE or MEM_IN_LINEAR [IN]
  DWORD heapID )                // Ddraw heap ID[IN]
{
  DD_ENTRY_SETUP(lpDD);

  D3DPRINT(DLALLOC, "FreeSurf: hID %d, hw %08x, fp %08x, %s",
	  heapID, hwVidMem, fpVidMem, (tileFlag == MEM_IN_TILED ? "Tiled" : "Linear"));

  if (heapID != HEAP_INVALID)
  {
    if (type & DDSCAPS_3DDEVICE)
    {    
      if(_FF(dd3DSurfaceCount))     //don't allow it go to negative.
      _FF(dd3DSurfaceCount)--;
    }

#ifdef SLI_AA
    // Restore lfbPtr from SLI mode.

    if (heapID & LFBPTR_TRANSLATED)
    {
      fpVidMem -= _FF(ddExtraMemorySize);
    }
#endif

    // Call DirectDraw memory manager.

    if (heapID != HEAP_INVALID)
    {
      DDHAL32_VidMemFree(lpDD, (heapID & HEAP_MASK), fpVidMem);
    }
  }
} // memMgr_freeSurface

/*******************************************************************/
/*                    INTERNAL FUNCTIONS                           */
/*******************************************************************/

//-----------------------------------------------------------------------------
//
// HwPtrToLfbPtr: Translate a hwPtr in tiled memory to a lfbPtr.
//
//-----------------------------------------------------------------------------

DWORD HwPtrToLfbPtr(NT9XDEVICEDATA * ppdev, DWORD hwPtr)
{
  DWORD tileOffset;
  DWORD tileInY;
  DWORD tileInX;
  DWORD yOffset;
  DWORD xOffset;
  DWORD lfbPtr;

  // Compute byte offset from tile mark.

  tileOffset = (hwPtr & ~SSTG_IS_TILED) - _FF(ddTiledHeapStart);

  // Compute tile offset from tile mark.

  tileInY = (tileOffset / SST_TILE_SIZE) / _FF(ddTileStride);
  tileInX = (tileOffset / SST_TILE_SIZE) % _FF(ddTileStride);

  // Compute scanline and byte offsets.

  yOffset = (tileInY * SST_TILE_HEIGHT) * _FF(ddTilePitch);
  xOffset = (tileInX * SST_TILE_WIDTH);

#ifdef SLI_AA
  // Expand lfbPtr in SLI mode.

  if (_DD(ddSLIModeRequested))
  {
    yOffset *= _DD(ddSLINumberWays);
  }
#endif

  // Compute lfb offset from tile mark.

  lfbPtr  = yOffset + xOffset;
  
  // Add tile mark and lfb base.

  lfbPtr += _FF(ddTiledHeapStart);
  lfbPtr += _FF(LFBBASE);

  return lfbPtr;

} // HwPtrToLfbPtr

//-----------------------------------------------------------------------------
//
// LfbPtrToHwPtr: Translate an lfbPtr in tiled memory to a hwPtr.
//
//-----------------------------------------------------------------------------

DWORD LfbPtrToHwPtr(NT9XDEVICEDATA * ppdev, DWORD lfbPtr, DWORD heapID)
{
  DWORD tileInY;
  DWORD tileInX;
  DWORD yScanlines;
  DWORD xBytes;
  DWORD yOffset;
  DWORD xOffset;
  DWORD hwPtr;

  // Subtract lfb base.
  
  lfbPtr -= _FF(LFBBASE);

  // Subtract tile mark.

  lfbPtr -= _FF(ddTiledHeapStart);

#ifdef SLI_AA
  // Restore lfbPtr from SLI mode.

  if (heapID & LFBPTR_TRANSLATED)
  {
    lfbPtr -= _FF(ddExtraMemorySize);
  }
#endif

  // Compute scanline and byte offset from tile mark.

  yScanlines = lfbPtr / _DS(ddTilePitch);
  xBytes     = lfbPtr % _DS(ddTilePitch);

#ifdef SLI_AA
  // Compress hwPtr in SLI mode.

  if (_DD(ddSLIModeRequested))
  {
    yScanlines /= _DD(ddSLINumberWays);
  }
#endif

  // Compute tile offset from tile mark.

  tileInY = yScanlines / SST_TILE_HEIGHT;
  tileInX = xBytes / SST_TILE_WIDTH;

  // Compute offsets within tile.

  yOffset = yScanlines % SST_TILE_HEIGHT;
  xOffset = xBytes % SST_TILE_WIDTH;

  // Calculate hardware offset.

  hwPtr  = ((tileInY * _FF(ddTileStride)) + tileInX) * SST_TILE_SIZE;
  hwPtr += (yOffset * SST_TILE_WIDTH) + xOffset;
  
  // Add tile mark.

  hwPtr += _FF(ddTiledHeapStart);

#ifdef SLI_AA
  // Add extra memory to hardware offset in SLI/!AA mode.

  if (heapID & LFBPTR_TRANSLATED)
  {
    hwPtr += _FF(ddExtraMemorySize);
  }
#endif

  hwPtr |= SSTG_IS_TILED;

  return hwPtr;

} // LfbPtrToHwPtr


//-----------------------------------------------------------------------------
//
// AdjustLfbPtr: Adjust lfbPtr in tiled memory for SLI mode.
//
//-----------------------------------------------------------------------------

#ifdef SLI_AA
DWORD AdjustLfbPtr(NT9XDEVICEDATA * ppdev, DWORD lfbPtr, DWORD *heapID)
{
  // Adjust lfbPtr for SLI mode.

  if (_DD(ddSLIModeRequested) && !_DD(ddAAModeRequested))
  {
    if (!(*heapID & LFBPTR_TRANSLATED))
    {
      *heapID |= LFBPTR_TRANSLATED;
      return (lfbPtr + _FF(ddExtraMemorySize));
    }
  }
  else
  {
    if (*heapID & LFBPTR_TRANSLATED)
    {
      *heapID &= ~LFBPTR_TRANSLATED;
      return (lfbPtr - _FF(ddExtraMemorySize));
    }
  }
  return lfbPtr;

} // AdjustLfbPtrHwPtr


//-----------------------------------------------------------------------------
//
// AdjustHwPtr: Adjust a hwPtr in tiled memory for SLI mode.
//
//-----------------------------------------------------------------------------

DWORD AdjustHwPtr(NT9XDEVICEDATA * ppdev, DWORD hwPtr)
{
  DWORD tileOffset;
  DWORD tileInY;
  DWORD tileInX;
  DWORD yOffset;
  DWORD xOffset;

  // Compute byte offset from tile mark.

  tileOffset = (hwPtr & ~SSTG_IS_TILED) - _FF(ddTiledHeapStart);

  // Compute tile offset from tile mark.

  tileInY = (tileOffset / SST_TILE_SIZE) / _FF(ddTileStride);
  tileInX = (tileOffset / SST_TILE_SIZE) % _FF(ddTileStride);

  // Compute scanline and byte offsets.

  yOffset = (tileInY * _FF(ddTileStride) * SST_TILE_SIZE);
  xOffset = (tileInX * SST_TILE_SIZE);

  // Adjust linear frame buffer pointer for extra tiled memory in SLI mode.

  if (_DD(ddSLIModeRequested))
  {
    // Compress hwPtr in SLI mode.

    yOffset /= _DD(ddSLINumberWays);
  }

  // Compute lfb offset from tile mark.

  hwPtr  = yOffset + xOffset;
  
  // Add tile mark.

  hwPtr += _FF(ddTiledHeapStart);

  // Add extra memory to hardware offset in SLI/!AA mode.

  if (_DD(ddSLIModeRequested) && !_DD(ddAAModeRequested))
  {
    hwPtr += _FF(ddExtraMemorySize);
  }

  hwPtr |= SSTG_IS_TILED;

  return hwPtr;

} // AdjustHwPtr

#endif // SLI_AA
