/* -*-c++-*- */
/* $Header: dddrv16.c, 29, 10/19/00 2:59:16 AM, Jonny Cochrane$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** Portions Copyright (C) 1995 Microsoft Corporation.  All Rights Reserved.
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
** File Name:	dddrv16.c
**
** $Revision: 29$
** $Date: 10/19/00 2:59:16 AM$
**
*/

/*******************************************************************************
*
* EXPORTED FUNCTIONS:
*
* DDCreateDriverObject  --- Create DirectDraw driver object.
* DDDestroyDriverObject --- Destroy DirectDraw driver object.
* D3notify              --- Notify Direct3d that hardware state has changed,
*
* PRIVATE FUNCTIONS:
*
* buildDDHALInfo16      --- Initialize HALINFO structure.
* DDHalModeInfo         --- Allocate/Lock and initialize the DDHALMODEINFO structure.
* DDHalModeFree         --- Unlocks/Free the DDHALMODEINFO structure and globals.
* EnableTiledHeap       --- Enables tiled heap and moves tiled mark.
* DisableTiledHeap      --- Disables tiled heap and moves tiled mark.
* HwPtrToLfbPtr         --- Translate a hwPtr in tiled memory to a lfbPtr.
*
*******************************************************************************/

#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif

#include "header.h"
#include "modelist.h"

extern void FXWAITFORIDLE();

BOOL buildDDHALInfo16(void);
DDHALMODEINFO FAR * DDHalModeInfo(int nNumModes);
void DDHalModeFree(void);
void EnableTiledHeap();
void DisableTiledHeap();
DWORD HwPtrToLfbPtr(DWORD);

#define FOURCC_YUY2   0x32595559
#define FOURCC_YUV2   0x32565559
#define FOURCC_Y211   0x31313259
#define FOURCC_UYVY   0x59565955

DWORD fourCC[] = { FOURCC_YUY2, FOURCC_UYVY };

// DirectDraw Heaps
//
// BB = back buffer
// ZB = zbuffer
// TB = third buffer
//                                Heap usage      Heap usage
//								  !Anti-aliasing  Anti-aliasing

#define LINEAR_HEAP0_ID   0L  //  linear heap     linear heap
#define TILED_HEAP0_ID    1L  //  tiled BB        tiled BB
#define TILED_HEAP1_ID    2L  //  tiled ZB        tiled ZB
#define TILED_HEAP2_ID    3L  //  tiled TB        tiled TB
#define LINEAR_HEAP1_ID   4L  //  linear heap     tiled FB (AA)
#define LINEAR_HEAP2_ID   5L  //  linear heap     tiled BB (AA)
#define LINEAR_HEAP3_ID   6L  //  linear heap     tiled ZB (AA)
#define LINEAR_HEAP4_ID   7L  //  linear heap     tiled TB (AA)

#define HEAP_INVALID       0x0000FFFFL
#define HEAP_MASK          0x0000FFFFL

static VIDMEM vidMem[] =
{
    { 0, 0, 0, 0, 0 }, // linear heap 0
    { 0, 0, 0, 0, 0 }, // tiled heap 0
    { 0, 0, 0, 0, 0 }, // tiled heap 1
    { 0, 0, 0, 0, 0 }, // tiled heap 2
    { 0, 0, 0, 0, 0 }, // linear heap 1
    { 0, 0, 0, 0, 0 }, // linear heap 2
    { 0, 0, 0, 0, 0 }, // linear heap 3
    { 0, 0, 0, 0, 0 }, // linear heap 4
    { 0, 0, 0, 0, 0 }, // extra linear heap!
};

#ifdef SSB
extern void DiscardAllSSB(void);      // in ssb.c
extern BOOL saveScreenBitmapAllowed;  // in ssb.c
extern BOOL saveScreenBitmapDisabled; // in ssb.c
#endif

/*******************************************************************/
/*                     EXPORTED FUNCTIONS                          */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name:  DDCreateDriverObject

Description:    Create DirectDraw driver object.

                Called from Control1 when DirectDraw issues a GDI Escape
                call asking us to create a driver object and register it
                with DirectDraw (bReset == 0).

                Called from Enable1 every time the display mode changes,
                to re-register the mode information and driver capabilities
                with DirectDraw (bReset == 1).

                In both cases we need to call DirectDraw to register our
                driver object.  The function we need to call to register
                with DirectDraw is given to us with the DDNEWCALLBACKFNS
                escape.  If we have not received this escape we should not
                register.

Return:         BOOL (Failure or Success)
----------------------------------------------------------------------*/

BOOL DDCreateDriverObject(BOOL bReset)
{
  DPF(DBGLVL_NORMAL, "DDCreateDriverObject(%d)", bReset);

  // Check that function exists to register with DirectDraw.

  if (_FF(HALCallbacks).lpSetInfo == NULL)
  {
    DPF(DBGLVL_NORMAL, "DDCreateDriverObject: failing because lpSetInfo == NULL");
    return FALSE;
  }

  // Check 32-bit portion of DDHALINFO.

  if (_FF(HALInfo).dwSize != sizeof(DDHALINFO))
  {
    DPF(DBGLVL_NORMAL, "DDCreateDriverObject: failing because 32 driver did not fill in DDHALINFOf");
    return FALSE;
  }

  // Disable bitmap caching.

  {
    extern void DisableDeviceBitmaps(void);

#ifdef PERF_NEWMM
    extern void CacheDepopulate();
    CacheDepopulate();
#else  
    extern void DoAllHost(void);
    DoAllHost();
#endif // PERF_NEWMM    
    DisableDeviceBitmaps();
  }

  // Disable save screen bitmap.
#ifdef SSB
	DiscardAllSSB();
    saveScreenBitmapDisabled = TRUE;
#endif

  // Setup 16-bit portion of DDHALINFO.

  buildDDHALInfo16();

  // Enable tiled heap usage.

  if ((!_FF(ddTiledHeapActive)) && _FF(ddPrimaryInTile))
  {
    EnableTiledHeap ();
  }

  // Set the flag for measuring the vertical refresh time.

  if(bReset)
  {
    _FF(fReset) = TRUE;
  }
      
  // Call DirectDraw to register our driver object.

  return _FF(HALCallbacks).lpSetInfo(&_FF(HALInfo), bReset);

} /* DDCreateDriverObject */


/*----------------------------------------------------------------------
Function name:  DDDestroyDriverObject

Description:    Destroy DirectDraw driver object.

Return:         DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __loadds CALLBACK DDDestroyDriverObject (LPDDHAL_DESTROYDRIVERDATA p)
{
  DPF(DBGLVL_NORMAL, "DDDestroyDriverObject");

  _FF(HALCallbacks).lpSetInfo = 0;

  // Disable tiled heap usage.

  if (_FF(ddTiledHeapActive) && _FF(ddPrimaryInTile))
  {
    DisableTiledHeap ();
  }

  // Enable bitmap caching.

  {
    extern void EnableDeviceBitmaps(void);

    EnableDeviceBitmaps();
  }

  // Enable save screen bitmap.
#ifdef SSB
    saveScreenBitmapDisabled = FALSE;
#endif

  p->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} /* DDDestroyDriverObject */

/*----------------------------------------------------------------------
Function name:  D3notify

Description:    Notify Direct3d that hardware state has changed,

Return:         VOID
----------------------------------------------------------------------*/
void D3notify(void)
{
  // if d3d is loaded
  if (_FF(pD3context) != 0xffffffff)
  {
    h3WRITE(0, (DWORD *)_FF(pD3context), 0);
    h3WRITE(0, (DWORD *)_FF(pD3changed), 1);
    h3WRITE(0, (DWORD *)_FF(pD3colbuff), 0xffffffff);
    h3WRITE(0, (DWORD *)_FF(pD3auxbuff), 0xffffffff); 
  } 
}

/*******************************************************************/
/*                     PRIVATE FUNCTIONS                           */
/*******************************************************************/


/*----------------------------------------------------------------------
Function name:  IsValidDDrawMode

Description:    Determines if passed in mode is a valid DDraw mode.

Information:

Return:         BOOL    TRUE  = Valid DDraw mode
                        FALSE = Invalid mode
----------------------------------------------------------------------*/
BOOL IsValidDDrawMode ( DWORD hres, DWORD vres )
{
  int i;


 	for ( i = 0; ModeList[ i ].dwWidth != 0; i++ )
  	{
    	if ( ModeList[ i ].dwWidth  == hres &&
          	 ModeList[ i ].dwHeight == vres &&
             ModeList[ i ].dwFlags & IS_DDRAW_MODE )
      		return TRUE;
   	}  

  return FALSE;
}

/*----------------------------------------------------------------------
Function name:  buildDDHALInfo16

Description:    Initialize HALINFO structure.

                Most of HALINFO is initialized by 32-bit DirectDraw driver.
                This functions initializes 16-bit specific HALINFO fields,
                including information which can change on every mode switch.
Information:

Return:         BOOL    TRUE  = Success
                        FALSE = Failure
----------------------------------------------------------------------*/
BOOL buildDDHALInfo16()
{
  int                 i;
  LPDDHALMODEINFO     lpMode;
  DWORD               heapAddress;
#ifndef REAL_NET
  DWORD dwWidth, dwHeight, dwBPP;
#endif
  int nCount;         // STBNW JAC 3-4-99 Moved outside of #ifdef NO_REFRESH


  /*
   * modify the structures in our shared window with the 32bit driver
   */
  #define cbDDCallbacks        _FF(DDCallbacks)
  #define cbDDSurfaceCallbacks _FF(DDSurfaceCallbacks)
  #define cbDDPaletteCallbacks _FF(DDPaletteCallbacks)
  #define ddHALInfo            _FF(HALInfo)
  #define vmiData              ddHALInfo.vmiData

  /*
   * count the number of modes in the mode table, the table lives in
   * setmode.c not this file.
   */
#ifndef REAL_NET
  dwWidth = 0x0;
  dwHeight = 0x0;
  dwBPP = 0x0;
  nCount = 0;
  for (i=0; ModeList[i].dwWidth != 0; i++)
      if ( (ModeList[i].dwFlags & IS_VALID_MODE) &&
           IsValidDDrawMode( ModeList[i].dwWidth, ModeList[i].dwHeight ) )  // STBNW JAC 3-4-99
         {
         if ((dwWidth != ModeList[i].dwWidth) ||
             (dwHeight != ModeList[i].dwHeight) ||
             (dwBPP != ModeList[i].dwBPP))
            {
            nCount++;
            dwWidth = ModeList[i].dwWidth;
            dwHeight = ModeList[i].dwHeight;
            dwBPP = ModeList[i].dwBPP;
            }
         }

  nCount++;
  ddHALInfo.dwNumModes = nCount;
  ddHALInfo.lpModeInfo = DDHalModeInfo(nCount);
#else
  nCount = 0;
  for (i=0; ModeList[i].dwWidth != 0; i++)
  {
      if ( IsValidDDrawMode( ModeList[i].dwWidth, ModeList[i].dwHeight ) )
        nCount++;
  }
  i = nCount;

  ddHALInfo.dwNumModes = i;
  ddHALInfo.lpModeInfo = DDHalModeInfo(i);
#endif
  if (NULL == ddHALInfo.lpModeInfo)
      {
      DPF(DBGLVL_NORMAL, "DDHalModeInfo called failed\n");
      return FALSE;
      }

  /*
   * current video mode
   */
  // Translate Mode number into compressed mode list for DirectDraw
#ifndef REAL_NET
  dwWidth = ModeList[_FF(ModeNumber)].dwWidth;
  dwHeight = ModeList[_FF(ModeNumber)].dwHeight;
  dwBPP = ModeList[_FF(ModeNumber)].dwBPP;
  for (i=0; i<nCount; i++)
      if ((dwWidth == ddHALInfo.lpModeInfo[i].dwWidth) &&
          (dwHeight == ddHALInfo.lpModeInfo[i].dwHeight) &&
          (dwBPP == ddHALInfo.lpModeInfo[i].dwBPP))
         break;         

  ddHALInfo.dwModeIndex = nCount - 1;
  lpMode = &ddHALInfo.lpModeInfo[ddHALInfo.dwModeIndex];
  *lpMode = ddHALInfo.lpModeInfo[i];
  // Make sure that the current rate has the right value
  lpMode->wRefreshRate = ModeList[_FF(ModeNumber)].wVert;
//  lpMode->wFlags &= ~DDMODEINFO_MAXREFRESH;
#else
  ddHALInfo.dwModeIndex = _FF(ModeNumber);
  lpMode = &ddHALInfo.lpModeInfo[ddHALInfo.dwModeIndex];
  lpMode->wFlags |= DDMODEINFO_MAXREFRESH;
#endif
  /*
   * current primary surface attributes
   */

  if (_FF(ddPrimaryInTile))
  {
    _FF(ddPrimarySurfaceData).lfbPtr = HwPtrToLfbPtr(_FF(gdiDesktopStart));
  }
  else
  {
    _FF(ddPrimarySurfaceData).lfbPtr = _FF(ScreenAddress);
  }

  _FF(ddPrimarySurfaceData).hwPtr  = _FF(gdiDesktopStart); 
  _FF(ddPrimarySurfaceData).lPitch = lpMode->lPitch;
  _FF(ddPrimarySurfaceData).heapID = HEAP_INVALID;

  vmiData.fpPrimary = _FF(ddPrimarySurfaceData).lfbPtr;

  /*
   * fill in the pixel format
   */
  vmiData.ddpfDisplay.dwSize  = sizeof (DDPIXELFORMAT);
  vmiData.ddpfDisplay.dwFlags = DDPF_RGB;
  vmiData.ddpfDisplay.dwRGBBitCount = lpMode->dwBPP;

  if (lpMode->wFlags & DDMODEINFO_PALETTIZED )
  {
      vmiData.ddpfDisplay.dwFlags |= DDPF_PALETTEINDEXED8;
  }


#ifdef STEREO
//jcochrane - stereo glasses support
	if (lpMode->wFlags & DDMODEINFO_STEREO )
   		_FF(StereoMode) = TRUE;
	else
   		_FF(StereoMode) = FALSE;
#endif

  // Banshee does not support FOURCC Blts to 8bpp destination.

  if (vmiData.ddpfDisplay.dwRGBBitCount == 8)
  {
    ddHALInfo.ddCaps.dwCaps &= ~DDCAPS_BLTFOURCC;
  }
  else
  {
    ddHALInfo.ddCaps.dwCaps |= DDCAPS_BLTFOURCC;
  }

  vmiData.ddpfDisplay.dwRBitMask = lpMode->dwRBitMask;
  vmiData.ddpfDisplay.dwGBitMask = lpMode->dwGBitMask;
  vmiData.ddpfDisplay.dwBBitMask = lpMode->dwBBitMask;
  vmiData.ddpfDisplay.dwRGBAlphaBitMask = lpMode->dwAlphaBitMask;

  ddHALInfo.lpdwFourCC = fourCC;
  ddHALInfo.ddCaps.dwNumFourCCCodes = sizeof( fourCC ) / sizeof( fourCC[0] );

  vmiData.dwOffscreenAlign = 32;
  vmiData.dwOverlayAlign = 32;
  vmiData.dwTextureAlign = 32;
  vmiData.dwAlphaAlign = 32;
  vmiData.dwZBufferAlign = 32;

  vmiData.dwNumHeaps = 1;
  vmiData.pvmList = vidMem;
  vmiData.dwDisplayHeight =  lpMode->dwHeight;
  vmiData.dwDisplayWidth = lpMode->dwWidth;
  vmiData.lDisplayPitch = lpMode->lPitch;

#ifndef NOLOWRESFIX
  {
    extern FxU32 lowreshack, lowresheight;

    if (lowreshack)
    {
      vmiData.dwDisplayHeight = lowresheight;
      vmiData.dwDisplayWidth = 320;
      vmiData.lDisplayPitch /= 2;
    }
  }
#endif // #ifndef NOLOWRESFIX

  // Setup DirectDraw heaps.
 // Initialize secondary buffers.

  heapAddress = _FF(ddSecondaryHeapStart);
  if (_FF(ddSecondaryHeapSize))
  {
    // EVEN alignment
    _FF(ddLinearHeap4Start) = heapAddress;
    vidMem[LINEAR_HEAP4_ID].dwFlags = VIDMEM_ISLINEAR;
    vidMem[LINEAR_HEAP4_ID].fpStart = _FF(ddLinearHeap4Start) + _FF(LFBBASE);
    vidMem[LINEAR_HEAP4_ID].fpEnd   = _FF(gdiDesktopSize) + vidMem[LINEAR_HEAP4_ID].fpStart - 1;
    heapAddress += _FF(gdiDesktopSize);
	

    // ODD alignment
    _FF(ddLinearHeap3Start) = heapAddress + SST_TILE_SIZE;
    vidMem[LINEAR_HEAP3_ID].dwFlags = VIDMEM_ISLINEAR;
    vidMem[LINEAR_HEAP3_ID].fpStart = _FF(ddLinearHeap3Start) + _FF(LFBBASE);
	vidMem[LINEAR_HEAP3_ID].fpEnd   = _FF(gdiDesktopSize) + vidMem[LINEAR_HEAP3_ID].fpStart - 1;
    heapAddress += _FF(gdiDesktopSize) + (_FF(ddTileStride) * SST_TILE_SIZE * _FF(dwNumUnits)); // Align zbuffer (AA)
	

    // EVEN alignment
    _FF(ddLinearHeap2Start) = heapAddress;
    vidMem[LINEAR_HEAP2_ID].dwFlags = VIDMEM_ISLINEAR;
    vidMem[LINEAR_HEAP2_ID].fpStart = _FF(ddLinearHeap2Start) + _FF(LFBBASE);
    vidMem[LINEAR_HEAP2_ID].fpEnd   = _FF(gdiDesktopSize) + vidMem[LINEAR_HEAP2_ID].fpStart - 1;
    heapAddress += _FF(gdiDesktopSize);

    // EVEN alignment
    _FF(ddLinearHeap1Start) = heapAddress;
    vidMem[LINEAR_HEAP1_ID].dwFlags = VIDMEM_ISLINEAR;
    vidMem[LINEAR_HEAP1_ID].fpStart = _FF(ddLinearHeap1Start) + _FF(LFBBASE);
    vidMem[LINEAR_HEAP1_ID].fpEnd   = _FF(gdiDesktopSize) + vidMem[LINEAR_HEAP1_ID].fpStart - 1;
  }

  // Initialize primary buffers.

  heapAddress = _FF(ddTiledHeapStart);
  switch (_FF(ddNumColorBuff))
  {
    case 3:

      // EVEN alignment
      _FF(ddTiledHeap2Start) = heapAddress;
      vidMem[TILED_HEAP2_ID].dwFlags  = VIDMEM_ISRECTANGULAR;
      vidMem[TILED_HEAP2_ID].dwWidth  = _FF(ddTileStride) * SST_TILE_WIDTH;

#ifdef STEREO
	  	if( _FF(ddStereoHeapFactor) )
	    	vidMem[TILED_HEAP2_ID].dwHeight = _FF(ddTileHeight) * SST_TILE_HEIGHT * _FF(ddStereoHeapFactor);
		else
	  		vidMem[TILED_HEAP2_ID].dwHeight = _FF(ddTileHeight) * SST_TILE_HEIGHT;
#else
      vidMem[TILED_HEAP2_ID].dwHeight = _FF(ddTileHeight) * SST_TILE_HEIGHT;
#endif

      vidMem[TILED_HEAP2_ID].fpStart  = HwPtrToLfbPtr(_FF(ddTiledHeap2Start)); // calculate LFB pointer

#ifdef STEREO
	  if( _FF(ddStereoHeapFactor) )
      	heapAddress += ( _FF(gdiDesktopSize) * _FF(ddStereoHeapFactor) );
	  else
	    heapAddress += _FF(gdiDesktopSize);
#else
      heapAddress += _FF(gdiDesktopSize);

#endif

    case 2:
      // ODD alignment
      _FF(ddTiledHeap1Start) = heapAddress + SST_TILE_SIZE;
      vidMem[TILED_HEAP1_ID].dwFlags  = VIDMEM_ISRECTANGULAR;
      vidMem[TILED_HEAP1_ID].dwWidth  = _FF(ddTileStride) * SST_TILE_WIDTH;

#ifdef STEREO
	  if( _FF(ddStereoHeapFactor))
      	vidMem[TILED_HEAP1_ID].dwHeight = _FF(ddTileHeight) * SST_TILE_HEIGHT  * 2;
	  else
		vidMem[TILED_HEAP1_ID].dwHeight = _FF(ddTileHeight) * SST_TILE_HEIGHT;
#else
      vidMem[TILED_HEAP1_ID].dwHeight = _FF(ddTileHeight) * SST_TILE_HEIGHT;
#endif
      vidMem[TILED_HEAP1_ID].fpStart  = HwPtrToLfbPtr(_FF(ddTiledHeap1Start)); // calculate LFB pointer

#ifdef STEREO
	  if( _FF(ddStereoHeapFactor))
      	heapAddress += _FF(gdiDesktopSize) * 2;
	  else {
	    heapAddress += _FF(gdiDesktopSize);
	    heapAddress += (_FF(ddTileStride) * SST_TILE_SIZE * _FF(dwNumUnits));
	   }
#else
      heapAddress += _FF(gdiDesktopSize);
      // Add extra row(s) of tiles to align zbuffer.
      heapAddress += (_FF(ddTileStride) * SST_TILE_SIZE * _FF(dwNumUnits));
#endif
      
     

    case 1:

      // EVEN alignment

      _FF(ddTiledHeap0Start) = heapAddress;
      vidMem[TILED_HEAP0_ID].dwFlags  = VIDMEM_ISRECTANGULAR;
      vidMem[TILED_HEAP0_ID].dwWidth  = _FF(ddTileStride) * SST_TILE_WIDTH;
      vidMem[TILED_HEAP0_ID].dwHeight = _FF(ddTileHeight) * SST_TILE_HEIGHT;
      vidMem[TILED_HEAP0_ID].fpStart  = HwPtrToLfbPtr(_FF(ddTiledHeap0Start)); // calculate LFB pointer
  }

  // Initialize number of heaps.

  vmiData.dwNumHeaps = _FF(ddNumColorBuff) + 1; // linear heap + extra heap + tiled heaps
  if (_FF(ddSecondaryHeapSize))
  {
    vmiData.dwNumHeaps += 4; // secondary linear memory heaps
  }

  // Store total number of heaps

  _FF(ddNumHeap) = vmiData.dwNumHeaps; // Not including extra heap!

  _FF(ddExtraMemorySize) = 0;
  if ((IS_NAPALM) && (_FF(dwNumUnits) > 1))
  {
    // Setup linear heap for extra memory supplied by SLI mode.

    _FF(ddExtraMemorySize) = (_FF(ddTiledHeapSize) + _FF(gdiDesktopSize)) / _FF(dwNumUnits);
    _FF(ddExtraMemoryHeap) = vmiData.dwNumHeaps;

    vmiData.dwNumHeaps += 1; // add extra linear heap

    vidMem[_FF(ddExtraMemoryHeap)].dwFlags = VIDMEM_ISLINEAR;
    vidMem[_FF(ddExtraMemoryHeap)].fpStart = _FF(ddTiledHeapStart) + _FF(LFBBASE);
    vidMem[_FF(ddExtraMemoryHeap)].fpEnd   =  vidMem[_FF(ddExtraMemoryHeap)].fpStart + _FF(ddExtraMemorySize) - 1;
    vidMem[_FF(ddExtraMemoryHeap)].ddsCaps.dwCaps    = (DWORD) ~(DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM);
    vidMem[_FF(ddExtraMemoryHeap)].ddsCapsAlt.dwCaps = (DWORD) ~(DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM);

    // Compute addresses of secondary buffers for SLI mode.

    switch (_FF(ddNumColorBuff))
    {
      case 3: _FF(secondaryThirdBuffer)  = _FF(ddTiledHeap2Start);                     // third buffer hwptr
              _FF(secondaryThirdBuffer) += _FF(ddTiledHeapSize) + _FF(gdiDesktopSize); // add tiled memory size
      case 2: _FF(secondaryZBuffer)      = _FF(ddTiledHeap1Start);                     // zbuffer hwptr
              _FF(secondaryZBuffer)     += _FF(ddTiledHeapSize) + _FF(gdiDesktopSize); // add tiled memory size
      case 1: _FF(secondaryBackBuffer)   = _FF(ddTiledHeap0Start);                     // back buffer hwptr
              _FF(secondaryBackBuffer)  += _FF(ddTiledHeapSize) + _FF(gdiDesktopSize); // add tiled memory size
      case 0: _FF(secondaryFrontBuffer)  = _FF(gdiDesktopStart) & ~SSTG_IS_TILED;      // front buffer hwptr
              _FF(secondaryFrontBuffer) += _FF(ddTiledHeapSize) + _FF(gdiDesktopSize); // add tiled memory size
    }
  }

#ifdef DEBUG
  if ((_FF(ddTiledHeap2Start) & SST_TILE_SIZE)  ||
    (!(_FF(ddTiledHeap1Start) & SST_TILE_SIZE)) ||
      (_FF(ddTiledHeap0Start) & SST_TILE_SIZE)  ||
      (_FF(gdiDesktopStart)   & SST_TILE_SIZE))
  {
    DPF(DBGLVL_NORMAL, "Tiled buffers are not aligned optimally for performance.");
    _asm int 3;
  }
#endif

  // Exclude textures from tiled memory.

  vidMem[TILED_HEAP0_ID].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
  vidMem[TILED_HEAP0_ID].ddsCapsAlt.dwCaps = DDSCAPS_TEXTURE;
  vidMem[TILED_HEAP1_ID].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
  vidMem[TILED_HEAP1_ID].ddsCapsAlt.dwCaps = DDSCAPS_TEXTURE;
  vidMem[TILED_HEAP2_ID].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
  vidMem[TILED_HEAP2_ID].ddsCapsAlt.dwCaps = DDSCAPS_TEXTURE;

  // Setup linear memory heap.

  vidMem[LINEAR_HEAP0_ID].dwFlags = VIDMEM_ISLINEAR;
  vidMem[LINEAR_HEAP0_ID].fpStart = _FF(ddLinearHeapStart) + _FF(LFBBASE);
  vidMem[LINEAR_HEAP0_ID].fpEnd   = vidMem[LINEAR_HEAP0_ID].fpStart + _FF(ddLinearHeapSize) - 1;


  DPF(DBGLVL_NORMAL, "TotalVRAM     = %08lX", _FF(TotalVRAM));
  DPF(DBGLVL_NORMAL, "ScreenAddress = %08lX", _FF(ScreenAddress));
  DPF(DBGLVL_NORMAL, "ScreenSize    = %08lX", lpMode->dwHeight * lpMode->lPitch);

  /*
   * callback functions (give DDRAW 16:16 pointers)
   */
  
  ddHALInfo.lpDDCallbacks        = &cbDDCallbacks;
  ddHALInfo.lpDDSurfaceCallbacks = &cbDDSurfaceCallbacks;
  ddHALInfo.lpDDPaletteCallbacks = &cbDDPaletteCallbacks;

#ifdef TnL_HAL
  // The D3D HAL wants to hook the Vertex Buffer DDI calls
  if (_FF(DDExebufCallbacks.dwFlags))
  {
    ddHALInfo.lpDDExeBufCallbacks  = &_FF(DDExebufCallbacks);
    ddHALInfo.ddCaps.ddsCaps.dwCaps  |= DDSCAPS_EXECUTEBUFFER;
  }
  else
    ddHALInfo.lpDDExeBufCallbacks = NULL;
#endif  

  /*
   * Only 16-bit callback, all others are undefined or 32-bit.
   */
  cbDDCallbacks.DestroyDriver = DDDestroyDriverObject;

  return TRUE;

} /* buildDDHALInfo16 */

/*----------------------------------------------------------------------
Function name:  DDHalModeInfo

Description:    Allocate/Lock and initialize the DDHALMODEINFO structure.

Return:         DDHALMODEINFO pointer if success, NULL if failure
----------------------------------------------------------------------*/

HGLOBAL hglbDDModeInfo = 0x0;
DDHALMODEINFO FAR * lpHalModeInfo = NULL;
int nOldModes = 0;

DDHALMODEINFO FAR * DDHalModeInfo(int nNumModes)
{
  DWORD dwWidth=0;
  DWORD dwHeight=0;
  DWORD dwBPP=0;
#ifdef STEREO
  DWORD dwTotalScanLines;
#endif
  int i,j;
#ifndef REAL_NET
  int k;
  int l;
#endif
  // Check to see if the number of modes has changed.
  // if it has then we need to reallocated our Mode Information

  if (NULL != lpHalModeInfo)
  {
    if (nNumModes == nOldModes)
      GlobalUnlock(hglbDDModeInfo);
    else
      DDHalModeFree();
  }
   
  nOldModes = nNumModes;
  if (NULL == lpHalModeInfo)
  {
    if (0 == (hglbDDModeInfo = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE, nNumModes * sizeof(DDHALMODEINFO))))
    {
      DPF(DBGLVL_NORMAL, "  GlobalAlloc for DDModeList failed");
      return NULL;
    }
  }

  if (NULL == (lpHalModeInfo = (DDHALMODEINFO FAR *)GlobalLock(hglbDDModeInfo)))
  {
    GlobalFree(hglbDDModeInfo);
    DPF(DBGLVL_NORMAL, "  GlobalLock for DDModeList failed");
    return NULL;
  }

  // Fill in the Mode Table

  j = 0;
  for (i=0; ModeList[i].dwWidth != 0; i++)
  {

#ifndef REAL_NET

    // Will current monitor support mode?
	if (!(ModeList[i].dwFlags & IS_VALID_MODE))
      continue;

    // Same mode, different refresh?
    if ((dwWidth == ModeList[i].dwWidth) && (dwHeight == ModeList[i].dwHeight) && (dwBPP == ModeList[i].dwBPP))
      continue;

#endif

    // Is it a valid DirectDraw mode?
    if (!IsValidDDrawMode(ModeList[i].dwWidth, ModeList[i].dwHeight))
      continue;

    dwWidth  = ModeList[i].dwWidth;
    dwHeight = ModeList[i].dwHeight;
    dwBPP    = ModeList[i].dwBPP;

    lpHalModeInfo[j].dwWidth  = ModeList[i].dwWidth;
    lpHalModeInfo[j].dwHeight = ModeList[i].dwHeight;
    lpHalModeInfo[j].dwBPP    = ModeList[i].dwBPP;

    /* Report correct tiled pitch for enumeration of modes. - CGW. */

#ifndef LINEAR_ONLY
    if ((16 == ModeList[i].dwBPP) || ((32 == ModeList[i].dwBPP) && (IS_NAPALM)))
    {
      if      (ModeList[i].lPitch <= 1024) lpHalModeInfo[j].lPitch = 1024L;
      else if (ModeList[i].lPitch <= 2048) lpHalModeInfo[j].lPitch = 2048L;
      else if (ModeList[i].lPitch <= 4096) lpHalModeInfo[j].lPitch = 4096L;
      else if (ModeList[i].lPitch <= 8192) lpHalModeInfo[j].lPitch = 8192L;
      else                                 lpHalModeInfo[j].lPitch = 16384L;
    }
    else
#endif
    {
      lpHalModeInfo[j].lPitch = ModeList[i].lPitch;
    }

    // Tell DirectDraw latter what the max refresh will be
    // simple find max
#ifndef REAL_NET
    l=i;
    for (k=i; ModeList[k].dwWidth != 0; k++)
       {
       if ((dwWidth != ModeList[k].dwWidth) || (dwHeight != ModeList[k].dwHeight) || (dwBPP != ModeList[k].dwBPP))
         break;
       l=k;
       }
    lpHalModeInfo[j].wRefreshRate = ModeList[l].wVert;
#else
    lpHalModeInfo[j].wRefreshRate = ModeList[i].wVert;
#endif

    if (8 == ModeList[i].dwBPP)
    {
      lpHalModeInfo[j].wFlags = DDMODEINFO_PALETTIZED;
      lpHalModeInfo[j].dwRBitMask = 0x00000000L; 
      lpHalModeInfo[j].dwGBitMask = 0x00000000L;
      lpHalModeInfo[j].dwBBitMask = 0x00000000L;
      lpHalModeInfo[j].dwAlphaBitMask = 0x00000000L; 
    }
    else if (16 == ModeList[i].dwBPP)
    {
      lpHalModeInfo[j].wFlags = 0x0;
      lpHalModeInfo[j].dwRBitMask = 0x0000F800L; 
      lpHalModeInfo[j].dwGBitMask = 0x000007E0L;
      lpHalModeInfo[j].dwBBitMask = 0x0000001FL;
      lpHalModeInfo[j].dwAlphaBitMask = 0x00000000L; 
    }
    else if (24 == ModeList[i].dwBPP)
    {
      lpHalModeInfo[j].wFlags = 0x0;
      lpHalModeInfo[j].dwRBitMask = 0x00FF0000L; 
      lpHalModeInfo[j].dwGBitMask = 0x0000FF00L;
      lpHalModeInfo[j].dwBBitMask = 0x000000FFL;
      lpHalModeInfo[j].dwAlphaBitMask = 0x00000000L; 
    }
    else 
    {
      lpHalModeInfo[j].wFlags = 0x0;
      lpHalModeInfo[j].dwRBitMask = 0x00FF0000L; 
      lpHalModeInfo[j].dwGBitMask = 0x0000FF00L;
      lpHalModeInfo[j].dwBBitMask = 0x000000FFL;
      lpHalModeInfo[j].dwAlphaBitMask = 0x00000000L; 
    }

#ifndef REAL_NET
   lpHalModeInfo[j].wFlags |= DDMODEINFO_MAXREFRESH;
#endif

#ifdef STEREO
//jcochrane - stereo glasses support
//is mode capable of doing stereo ?
	if( (dwWidth >= 320) && (dwHeight >= 240))
	{

		dwTotalScanLines = 	 _FF(TotalVRAM) / lpHalModeInfo[j].lPitch;
		
		if( dwTotalScanLines > (dwHeight * 4) )
		{
			lpHalModeInfo[j].wFlags |= (_FF(ddStereoWrapperLoaded) ? DDMODEINFO_STEREO : 0) ;
		}
	
	
	}
#endif
    j++;
  }

  return lpHalModeInfo;
}


/*----------------------------------------------------------------------
Function name:  DDHalModeFree

Description:    Unlocks/frees memory associated with Direct Draw HAL.

Return:         VOID
----------------------------------------------------------------------*/
void DDHalModeFree(void)
{
  // Unlock Pointer

  GlobalUnlock(hglbDDModeInfo);

  // Pointer is no longer valid

  lpHalModeInfo = NULL;  

  // Handle is no longer valid

  if (GlobalFree(hglbDDModeInfo))
  {
    DPF(DBGLVL_NORMAL, "GlobalFree Failed\n");
  }  
  hglbDDModeInfo = 0x0;
}


/*----------------------------------------------------------------------
Function name:  EnableTiledHeap

Description:	Enables tiled heap and moves tiled mark.

Return:         NONE
----------------------------------------------------------------------*/

void EnableTiledHeap()
{
  DWORD lfbMemoryConfig;

  // Set tiled heap active flag.

  _FF(ddTiledHeapActive) = TRUE;

  // Move tiled mark to start of tiled heaps.

  _FF(ddTileMark) = _FF(ddTiledHeapStart) & ~SSTG_IS_TILED;

  // Initialize lfbMemoryTileCtrl for tiled modes.

  lfbMemoryConfig = GET(lph3IORegs->lfbMemoryConfig);
  lfbMemoryConfig &= ~0x01801FFFL;
  lfbMemoryConfig |= ((_FF(ddTileMark) >> 12L) & 0x1FFFL);
  lfbMemoryConfig |= ((_FF(ddTileMark) >> 25L) & 0x0003L) << 23;
  SETDW(lph3IORegs->lfbMemoryConfig, lfbMemoryConfig );

  // Update DIB engine pointer, since tile mark moved.
  
  _FF(lpPDevice)->deBitsOffset = _FF(ddPrimarySurfaceData).lfbPtr;
}

/*----------------------------------------------------------------------
Function name:  DisableTiledHeap

Description:	Disables tiled heap and moves tiled mark.

Return:         NONE
----------------------------------------------------------------------*/
void DisableTiledHeap()
{
  DWORD lfbMemoryConfig;

  // Clear tiled heap active flag.

  _FF(ddTiledHeapActive) = FALSE;

  // Move tiled mark to start of desktop.

  _FF(ddTileMark) = _FF(gdiDesktopStart) & ~SSTG_IS_TILED;

  // Initialize lfbMemoryTileCtrl for desktop.

  lfbMemoryConfig = GET(lph3IORegs->lfbMemoryConfig);
  lfbMemoryConfig &= ~0x01801FFFL;
  lfbMemoryConfig |= ((_FF(ddTileMark) >> 12L) & 0x1FFFL);
  lfbMemoryConfig |= ((_FF(ddTileMark) >> 25L) & 0x0003L) << 23;
  SETDW(lph3IORegs->lfbMemoryConfig, lfbMemoryConfig );

  // Update DIB engine pointer, since tile mark moved.
  
  _FF(lpPDevice)->deBitsOffset = _FF(ScreenAddress);
}

//-----------------------------------------------------------------------------
//
// HwPtrToLfbPtr
//
// Translate a hwPtr in tiled memory to a lfbPtr
//
//-----------------------------------------------------------------------------

DWORD HwPtrToLfbPtr(DWORD hwPtr)
{
  DWORD tileOffset;
  DWORD tileInY;
  DWORD tileInX;
  DWORD lfbPtr;

  // Compute byte offset from tile mark.

  tileOffset = (hwPtr & ~SSTG_IS_TILED) - _FF(ddTiledHeapStart);

  // Compute tile offset from tile mark.

  tileInY = (tileOffset / SST_TILE_SIZE) / _FF(ddTileStride);
  tileInX = (tileOffset / SST_TILE_SIZE) % _FF(ddTileStride);

  // Compute lfb offset from tile mark.

  lfbPtr = ((tileInY * SST_TILE_HEIGHT) * _FF(ddTilePitch)) + (tileInX * SST_TILE_WIDTH);
  
  // Add tile mark and lfb base.

  lfbPtr += _FF(ddTiledHeapStart);
  lfbPtr += _FF(LFBBASE);

  return lfbPtr;

} // HwPtrToLfbPtr
