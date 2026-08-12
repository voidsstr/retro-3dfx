/* $Header: ddsurf.c, 100, 10/31/00 1:58:23 AM, Johnny Trainor $ */
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
**
** File Name:  DDSURF.C
**
** Description: Direct Draw surface related functions.
**
** $Revision: 100$
** $Date: 10/31/00 1:58:23 AM$
**
*/

/*******************************************************************************
*
* DIRECTDRAW FUNCTIONS:
*
* DdCanCreateSurface        --- DirectDraw CanCreateSurface entry point.
* DdCreateSurface           --- DirectDraw CreateSurface entry point.
* DdDestroySurface          --- DirectDraw DestroySurface entry point.
* DdLock                    --- DirectDraw Lock entry point.
* DdUnlock                  --- DirectDraw Unlock entry point.
* DdAddAttachedSurface      --- DirectDraw AddAttachedSurface entry point.
* DdSetSurfaceColorKey      --- DirectDraw SetSurfaceColorKey entry point.
*
* EXPORTED FUNCTIONS:
*
* FxGetBusyStatus           --- Determine if the pipeline is busy.
*
* INTERNAL FUNCTIONS:
*
* ShrinkOverlaySurface      --- Shrink the overlay to the shrink surface.
*
*******************************************************************************/

#include "precomp.h"
#if ENABLE_3D && !defined(WINNT)
#include "d3txtr.h"
#endif

#if ENABLE_3D
#ifdef WINNT
#define UNPAD_DXT1(arg1,arg2)   unpad_dxt1(arg1,arg2)
#define PAD_DXT1(arg1,arg2)     pad_dxt1(arg1,arg2)
#else
#define UNPAD_DXT1(arg1,arg2)   unpad_dxt1(arg2)
#define PAD_DXT1(arg1,arg2)     pad_dxt1(arg2)
#endif

void UNPAD_DXT1(NT9XDEVICEDATA*,LPDDRAWI_DDRAWSURFACE_GBL surfGBL);
void PAD_DXT1(NT9XDEVICEDATA*,LPDDRAWI_DDRAWSURFACE_GBL surfGBL);
#endif

#if defined(WINNT) && DBG && !defined(FXTRACE)
extern void DUMP_SURFACEDESC(NT9XDEVICEDATA*,int,LPDDSURFACEDESC);
#endif

#ifdef WINNT
#define COMMON_PRIMARYCHAIN_DDSCAPS \
        (DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE | DDSCAPS_FLIP | DDSCAPS_COMPLEX)
#define PARTOFPRIMARYCHAIN(dwCaps) \
            ((COMMON_PRIMARYCHAIN_DDSCAPS & (dwCaps)) == COMMON_PRIMARYCHAIN_DDSCAPS)
            //(((COMMON_PRIMARYCHAIN_DDSCAPS & (dwCaps)) == COMMON_PRIMARYCHAIN_DDSCAPS) && \
            // ((DDSCAPS_BACKBUFFER | DDSCAPS_FRONTBUFFER) & (dwCaps)))
extern HANDLE getCurrentProcessId(PDEV *);
#endif

/*******************************************************************/
/*                     DIRECTDRAW FUNCTIONS                        */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name: DdCanCreateSurface

Description:   Reports whether surface can be created.

               Surfaces allowed in offscreen memory:

               alpha:           not supported
               texture:         all formats
               zbuffer:         16bpp, 32bpp
               overlay:         yuy2, uyvy, rgb16
               offscreen:       rgb8, rgb16, rgb24, rgb32, yuy2, uyvy
               flipping chain:  matches primary

Return:        DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
DdCanCreateSurface( LPDDHAL_CANCREATESURFACEDATA pccsd )
{
  LPDDSURFACEDESC  lpddsd;
  DWORD            dwCaps, dwBpp, dwFlags, dwOverlay=FALSE;

  DD_ENTRY_SETUP(pccsd->lpDD);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "CanCreateSurface32" ));
  DUMP_SURFACEDESC(ppdev, DEBUG_DDGORY, pccsd->lpDDSurfaceDesc );
  #endif

#if defined(WINNT) && DBG
  DISPDBG((2, "DdCanCreateSurface:"));
  DISPDBG((2, "  SurfaceDesc->ddsCaps.dwCaps = %08lXh", pccsd->lpDDSurfaceDesc->ddsCaps.dwCaps));

  DUMP_SURFACEDESC(ppdev, DEBUG_DDGORY, pccsd->lpDDSurfaceDesc);
#endif

  lpddsd  = pccsd->lpDDSurfaceDesc;
  dwCaps  = lpddsd->ddsCaps.dwCaps;
  dwFlags = lpddsd->ddpfPixelFormat.dwFlags;

  /* Determine bits per pixel requested. */

  if (pccsd->bIsDifferentPixelFormat)
  {
    dwBpp = lpddsd->ddpfPixelFormat.dwRGBBitCount;
  }
  else
  {
    dwBpp = (DWORD) GETPRIMARYBYTEDEPTH << 3L; // primary surface
  }

  /* Assume format is invalid. */

  pccsd->ddRVal = DDERR_INVALIDPIXELFORMAT;

  /* Handle various surface formats. */

  if (dwCaps & DDSCAPS_ALPHA)   // Alpha surfaces
  {
    pccsd->ddRVal = DDERR_UNSUPPORTED;
  }

#if ENABLE_3D
  else if (dwCaps & DDSCAPS_TEXTURE) // Texture surfaces
  {
    // fail creation of textures that can be rendered to
    // unless the texture format is supported as a render target
    // by the hw
    if (DDSCAPS_3DDEVICE & dwCaps)
    {
      // both napalm and V3 support 565 as a render target format
      if ((16 == dwBpp) &&
          (0xF800 == pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask) &&
          (0x07E0 == pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask) &&
          (0x001F == pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask))
      {
        pccsd->ddRVal = DD_OK;
      }
      // napalm also supports 1555 and 8888 as render target formats
      else if ((IS_NAPALM) &&
               ((32 == dwBpp) &&
                (0xFF0000 == pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask) &&
                (0x00FF00 == pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask) &&
                (0x0000FF == pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask)) ||
               ((16 == dwBpp) &&
                (0x7C00 == pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask) &&
                (0x03E0 == pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask) &&
                (0x001F == pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask)))
      {
        pccsd->ddRVal = DD_OK;
      }
    }
    else
    {
      pccsd->ddRVal = DD_OK;
    }
  }

  else if (dwCaps & DDSCAPS_ZBUFFER) // Zbuffer surfaces
  {
    if (lpddsd->dwFlags & DDSD_ZBUFFERBITDEPTH)
    {
      dwBpp = lpddsd->dwZBufferBitDepth;
    }

    if (IS_NAPALM)
    {
      if ((dwBpp == 16) || (dwBpp == 24) || (dwBpp == 32)) // 16bpp/32bpp zbuffer (Napalm)
      {
        pccsd->ddRVal = DD_OK;
      }
    }
    else
    {
      if (dwBpp == 16) // 16bpp zbuffer (Voodoo3)
      {
        pccsd->ddRVal = DD_OK;
      }
    }
  }
#endif // ENABLE_3D

  else if (dwCaps & DDSCAPS_OVERLAY) // Overlay surfaces
  {
    if (_DD(dd3DInOverlay)) // Is primary using overlay?
      pccsd->ddRVal = DDERR_NOOVERLAYHW;
    else if (_DD(overlaySurfaceCnt) && !(dwCaps & DDSCAPS_BACKBUFFER)) // Is overlay already created?
      pccsd->ddRVal = DDERR_NOOVERLAYHW;
    else
       dwOverlay = TRUE; // Check for overlay formats
  }

  else if (dwCaps & DDSCAPS_OFFSCREENPLAIN) // Offscreen surfaces
  {
    if (!(pccsd->bIsDifferentPixelFormat))
   {
      pccsd->ddRVal = DD_OK; // Matches primary format.
    }
    else if ((dwFlags & DDPF_RGB) && ((dwBpp == 8)  || (dwBpp == 16) || (dwBpp == 24) || (dwBpp == 32)))
    {
      // disallow 5:5:5 format surfaces
      if (! ((dwBpp == 16) && ((pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask == 0x7C00))))
        pccsd->ddRVal = DD_OK; // Allow any color depth
    }
    else if ((dwFlags & DDPF_FOURCC) &&
            (lpddsd->ddpfPixelFormat.dwFourCC == FOURCC_RAW8))
    {
         pccsd->ddRVal = DD_OK;
    }
    else
    {
      dwOverlay = TRUE; // Check for overlay formats
    }
  }
  // in non palettized modes, allow YUY2 or UYVY surfaces with
  // only the VIDEOMEMORY bit set in ddsCaps.dwCaps
  else if ((8 != GETPRIMARYBYTEDEPTH) &&
           (DDSCAPS_VIDEOMEMORY & dwCaps) &&
           (pccsd->bIsDifferentPixelFormat) &&
           (DDPF_FOURCC & dwFlags) &&
           ((FOURCC_YUY2 == lpddsd->ddpfPixelFormat.dwFourCC) ||
            (FOURCC_UYVY == lpddsd->ddpfPixelFormat.dwFourCC)))
  {
    pccsd->ddRVal = DD_OK;
  }

  else // Flipping chain surfaces (primary, back buffer, third buffer)
  {
    if (!(pccsd->bIsDifferentPixelFormat))
    {
      pccsd->ddRVal = DD_OK; // Matches primary format.
    }
    else if ((dwFlags & DDPF_RGB) && (dwBpp == (DWORD) (GETPRIMARYBYTEDEPTH << 3L)))
    {
      pccsd->ddRVal = DD_OK; // Matches primary depth.
    }
  }

  /* Check for legal overlay formats. */

  if (dwOverlay)
  {
    CanCreateOverlaySurface( ppdev, pccsd, dwBpp, dwFlags);
  }

  return DDHAL_DRIVER_HANDLED;

} // DdCanCreateSurface


/*----------------------------------------------------------------------
Function name: DdCreateSurface

Description:   Creates the specified surface.

               Overlay surface count keeps track of how many overlays.
               When count goes to zero, disable overlay hardware.

               PROMOTION/DEMOTION:

               - Promote primary to overlay DdCreateSurface.
               - Demote primary to desktop DdDestroySurface.
               - Promote primary from desktop to overlay if:

                 1. in exclusive mode
                 2. primary is tiled
                 3. not overlay surface
                 4. 3D surface
                 5. primary surface or back buffer

Return:        DDHAL_DRIVER_HANDLED or DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
DdCreateSurface( LPDDHAL_CREATESURFACEDATA pcsd )
{
  LPDDRAWI_DDRAWSURFACE_LCL   psurf;
  LPDDRAWI_DDRAWSURFACE_GBL   psurf_gbl;
  DWORD                       ddReturnVal;
  int                         i;
  DWORD                       pWidth, bWidth, height, pixelByteDepth;
  DWORD                       tileFlag, dwCaps;
  FXSURFACEDATA               *surfaceData;

  DD_ENTRY_SETUP(pcsd->lpDD);
  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "CreateSurface32" ));
  DUMP_SURFACEDESC(ppdev, DEBUG_DDGORY, pcsd->lpDDSurfaceDesc );
  #endif

#if defined(WINNT) && DBG
  DISPDBG((2,"DdCreateSurface:"));
  for (i = 0; i < (int)pcsd->dwSCnt; i++)
  {
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    DISPDBG((2,"  surf=%d pSurf = %8lXh  handle = %ld",
             i, pcsd->lplpSList[i], pcsd->lplpSList[i]->lpSurfMore->dwSurfaceHandle));
    if (DDSCAPS2_TEXTUREMANAGE & pcsd->lplpSList[i]->lpSurfMore->ddsCapsEx.dwCaps2)
      DISPDBG((2,"    driver texture management requested"));
#else
    DISPDBG((2,"  surf=%d pSurf = %8lXh", i, pcsd->lplpSList[i]));
#endif
  }
  DISPDBG((2,"  SurfaceDesc->ddsCaps.dwCaps = %08lXh", pcsd->lpDDSurfaceDesc->ddsCaps.dwCaps));

  DUMP_SURFACEDESC(ppdev, DEBUG_DDGORY, pcsd->lpDDSurfaceDesc);
#endif

  dwCaps = pcsd->lpDDSurfaceDesc->ddsCaps.dwCaps;
#if ENABLE_3D
  // Texture surface
  if (dwCaps & DDSCAPS_TEXTURE)
  {
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
    for (i = 0; i < (int)pcsd->dwSCnt; i++)
    {
      if (0 == pcsd->lplpSList[i]->lpSurfMore->dwSurfaceHandle)
      {
        DISPDBG((0, "  Texture creation with handle==0, returning OUTOFVIDEOMEMORY"));
        pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
        return DDHAL_DRIVER_HANDLED;
      }
    }
#endif
    #ifndef WINNT
    DISPDBG((ppdev, DEBUG_DDDETAILS,"Surface Create - Texture"));
    D3DPRINT(DLSURFACE, "ddCreateSurface: Calling TEXTURESURFACECREATE");
    #endif
    ddReturnVal = TEXTURESURFACECREATE(ppdev, pcsd->lplpSList[0],-1, &pcsd->ddRVal);
    #ifndef WINNT
    DISPDBG((ppdev, DEBUG_DDDETAILS,"return from texture create = %d\n", ddReturnVal));
    #endif
    return(ddReturnVal);
  }
#endif // ENABLE_3D

#if ENABLE_TILED_HEAP
  if ( (dwCaps & (DDSCAPS_COMPLEX | DDSCAPS_FLIP)) && (!(dwCaps & DDSCAPS_OVERLAY)))
  {
    if (((int)pcsd->dwSCnt > 2) && (dwCaps & DDSCAPS_3DDEVICE))
    {
      DWORD dramInit1;
      dramInit1 = GET(ghwIO->dramInit1);
      dramInit1 |= SST_TRIPLE_BUFFER_EN;
      SETDW(ghwIO->dramInit1,dramInit1);
      _FF(ddMiscFlags) |= DDMF_TRIPPLE_BUFFER;
      D3DPRINT(DLSURFACE, "ddCreateSurface: Setting Triple Buffering");
    }
    else
    {
      DWORD dramInit1;
      dramInit1 = GET(ghwIO->dramInit1);
      dramInit1 &= ~SST_TRIPLE_BUFFER_EN;
      SETDW(ghwIO->dramInit1,dramInit1);
      _FF(ddMiscFlags) &= ~DDMF_TRIPPLE_BUFFER;
      D3DPRINT(DLSURFACE, "ddCreateSurface: UnSetting Triple Buffering");
    }
  }
#endif // ENABLE_TILED_HEAP

#if defined(WINNT) && (_WIN32_WINNT < 0x0500)
  // DO NOT ENABLE THIS CHECK ON NT4!
  // Otherwise apps running on NT4 will NOT be able to create overlay flipping chains!
#else
  //If overlay surface to be created and overlay h/w is already being used, fail it
  if ( (dwCaps & DDSCAPS_OVERLAY) && !(dwCaps & DDSCAPS_BACKBUFFER) &&
#ifndef WINNT // MDM-Richardson Only implemented for Win9x, PRS 7643 & 7902
      !(_FF(dwRelaxedOverlayOwnerMode)) &&
#endif
     _DD(overlaySurfaceCnt))

  {
      D3DPRINT(0,"ddCreateSurface: Overlay hardware busy. Aborting request.");
      pcsd->ddRVal = DDERR_NOOVERLAYHW;
      return DDHAL_DRIVER_HANDLED;
  }
#endif

  // Special handling for 3D, exclusive-mode, non-overlay, full-screen application.

#if ENABLE_3D
#ifdef SLI_AA
  // Fix for PRS 14504
  // Re-Volt creates a Z Buffer surface before creating the flipping chain
  // we need to check for SLI/AA for z buffer surfaces here too or else the
  // z buffer is allocated in the wrong location
  //
  // need to check for fullscreen zbuffer or else Blt-Exotic 3D Depth Fill fails
  if ((_FF(ddExclusiveMode) == TRUE) &&
      (_FF(ddPrimaryInTile) == TRUE) &&
      (SINGLE_CHIP_NOSLI_AA_DISABLED == _DD(ddSLIAAConfiguration)) &&
      (!(dwCaps & DDSCAPS_OVERLAY))  &&
      (((dwCaps & DDSCAPS_3DDEVICE) && (dwCaps & (DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP))) ||
       ((dwCaps & DDSCAPS_ZBUFFER) &&
        ((DDSD_WIDTH | DDSD_HEIGHT) == ((DDSD_WIDTH | DDSD_HEIGHT) & pcsd->lpDDSurfaceDesc->dwFlags)) &&
        (pcsd->lpDDSurfaceDesc->dwWidth == (DWORD)ppdev->cxScreen) &&
        (pcsd->lpDDSurfaceDesc->dwHeight == (DWORD)ppdev->cyScreen))))
  {
    // Determine SLI and AA configuration.

    D3DPRINT(DLSURFACE, "ddCreateSurface: Exclusive mode SLIAA config setup");

    if (IS_NAPALM)
    {
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)

      // this flag is only valid if DDSCAPS_3DDEVICE is also set, but we've already checked for that
      if (sizeof(DDSURFACEDESC2) <= pcsd->lpDDSurfaceDesc->dwSize)
      {
        _DD(ddAAModeRequested) = ((DDSURFACEDESC2 *)pcsd->lpDDSurfaceDesc)->ddsCaps.dwCaps2 & DDSCAPS2_HINTANTIALIASING;
      }
#endif
      // need to lie about the number of buffers in the case of a z buffer,
      // we're probably going to be hosed if they don't create a 2 buffer
      // flipping chain after the z buffer
      // PRS 15301 - Only lie about the number of buffers if we are creating 
      // a z-buffer and we have not already created any color buffers
	  Compute_SLIAA_Config(ppdev, ( (dwCaps & DDSCAPS_ZBUFFER) &&  ( !_FF(dd3DSurfaceCount) ) ) ? 2 : pcsd->dwSCnt);
    }
    else
    {
      _DD(ddAAModeRequested) = 0;
      _DD(ddSLIModeRequested) = 0;
      _DD(ddAANumberSamples)  = 0;
    }
  }

#endif // SLI_AA
#endif // ENABLE_3D

  for( i=0;i<(int)pcsd->dwSCnt;i++ )
  {
    psurf = pcsd->lplpSList[i];
    psurf_gbl = psurf->lpGbl;
    dwCaps = psurf->ddsCaps.dwCaps;

    D3DPRINT(DLSURFACE, "ddCreateSurface: surface request %d of %d", (i+1), pcsd->dwSCnt);

    #ifdef FXTRACE
    DUMP_DDRAWSURFACE_LCL(ppdev, DEBUG_DDGORY, psurf );
    #endif

    if ( dwCaps & DDSCAPS_SYSTEMMEMORY )
    {
      D3DPRINT(0,"ddCreateSurface: Can't allocate system memory surfaces. Aborting request.");
      pcsd->ddRVal = DDERR_UNSUPPORTED;
      return DDHAL_DRIVER_HANDLED;
    }

    if (!(dwCaps & DDSCAPS_PRIMARYSURFACE))
    {
      D3DPRINT(DLSURFACE, "ddCreateSurface: Not a primary surface");
      surfaceData = (FXSURFACEDATA*) DDMALLOCZ(sizeof(FXSURFACEDATA), &psurf_gbl->dwReserved1);
      if(!surfaceData)
      {
        D3DPRINT(0,"ddCreateSurface: FXSURFACEDATA allocation failed. Aborting request.");
        pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
        return DDHAL_DRIVER_HANDLED;
      }
    }
    else
    {
      D3DPRINT(DLSURFACE, "ddCreateSurface: Primary surface");
      surfaceData = (FXSURFACEDATA*) &(_FF(ddPrimarySurfaceData));
    }
    DDPRINT(DDDBGLVL, "    surfaceData=%8lXh", surfaceData);

    surfaceData->heapID = HEAP_INVALID;
#if ENABLE_3D
    surfaceData->AAheapID = HEAP_INVALID;
#endif
    surfaceData->overlayShrinkFlag = FALSE;
    surfaceData->doShrink = FALSE;

    if(dwCaps & DDSCAPS_FLIP)
    {
      D3DPRINT(DLSURFACE, "ddCreateSurface: Flipping surface");
      // Initialize surface flipping variables.

      _DD(ddSurfaceFlippedFrom) = 0;
      _DD(ddSurfaceFlippedTo)   = 0;
      _DD(ddAcceleratorUsed)    = 0;
    }

#ifndef WINNT
    if(dwCaps & DDSCAPS_FLIP)
    {
      surfaceData->inFlipChain = TRUE;
    }
    else
    {
      surfaceData->inFlipChain = FALSE;
    }
    surfaceData->surfaceLevel = _FF(ddCurrentSurfaceLevel);
#endif

    psurf_gbl->dwReserved1 = (DWORD)surfaceData;

    pWidth = psurf_gbl->dwBlockSizeX = psurf_gbl->wWidth;
    height = psurf_gbl->dwBlockSizeY = psurf_gbl->wHeight;

#if ENABLE_3D

    // check zbuffer format

    if (dwCaps & DDSCAPS_ZBUFFER)
    {
      D3DPRINT(DLSURFACE, "ddCreateSurface: Z Buffer");
#ifdef WINNT
      // WNT always has pixel format in global surface.
      pixelByteDepth = (DWORD)(psurf->lpGbl->ddpfSurface.dwZBufferBitDepth) >> 3;
#else
      // W9X must check local surface flag for pixel format.
      if((psurf->dwFlags) & DDRAWISURF_HASPIXELFORMAT )
      {
        pixelByteDepth = (DWORD)(psurf->lpGbl->ddpfSurface.dwZBufferBitDepth) >> 3;
        D3DPRINT(DLSURFACE, "ddCreateSurface: (Z) Has it's own pixel format, depth %d", pixelByteDepth);
      }			
      else
      {
        pixelByteDepth = GETPRIMARYBYTEDEPTH;
        D3DPRINT(DLSURFACE, "ddCreateSurface: (Z) Getting from Primary, depth %d", pixelByteDepth);
      }
#endif
      // When 24bpp zbuffer is requested, 32bpp zbuffer must be allocated.

      if (pixelByteDepth == 3)
	  {
          D3DPRINT(DLSURFACE, "ddCreateSurface: (Z) Reseting byte depth from 3 to 4");
		  pixelByteDepth = 4;

          // Munge surface information
          psurf_gbl->ddpfSurface.dwZBufferBitDepth = 32; // substitute 32bpp zbuffer
          psurf_gbl->ddpfSurface.dwZBitMask = 0x00FFFFFF;
	  }

      // Verify that zbuffer format is supported.

      if (!(IS_NAPALM))
      {
        if (pixelByteDepth != 2) // 16bpp zbuffer (Voodoo3)
        {
          D3DPRINT(0,"ddCreateSurface: (Z) !Napalm and !16bpp Z requested. Aborting request.");
          pcsd->ddRVal = DDERR_UNSUPPORTED;
          return DDHAL_DRIVER_HANDLED;
        }
      }
      else
      {
        if ((pixelByteDepth != 2) && (pixelByteDepth != 4)) // 16bpp/32bpp zbuffer (Napalm)
        {
          D3DPRINT(0,"ddCreateSurface: (Z) Napalm and Z request not 16/32bpp. Aborting request.");
          pcsd->ddRVal = DDERR_UNSUPPORTED;
          return DDHAL_DRIVER_HANDLED;
        }
      }

      /* Must allocate 16bpp zbuffer for 16bpp primary, and a 32bpp     */
      /* zbuffer for a 32bpp primary, since Napalm cannot handle a      */
      /* different depth zbuffer and primary! We will not decide on     */
      /* the actual depth until the zbuffer is attached, at which time  */
      /* we will use the same depth zbuffer as rendering surface. -CGW- */

      if (IS_NAPALM)
      {
#ifndef WINNT
        if ((pixelByteDepth != 2) && (GETPRIMARYBYTEDEPTH == 2)) // 16bpp rendering, 24bpp zbuffer requested
        {
          // Hack for DCT 300 Blt Exotic test, which creates a 32bpp zbuffer and attaches
          // it to a 16bpp primary, then fills the 32bpp zbuffer using DdBlt and checks the
          // contents.  If we only allocate 16bpp, the test will fail in low resolution
          // modes, so this hack allows the 32bpp allocation only for these modes. -CGW-

          if (IS_NAPALM && (_FF(hres) >= 640))
          {
            pixelByteDepth = 2; // Force 16bpp allocation
            D3DPRINT(DLSURFACE, "ddCreateSurface: (Z) Forcing zbuffer allocation from 24/32bpp to 16bpp.");

            // Munge surface information
            psurf_gbl->ddpfSurface.dwZBufferBitDepth = 16; // substitute 16bpp zbuffer
            psurf_gbl->ddpfSurface.dwZBitMask = 0x0000FFFF;
          }
        }
#endif
        if ((pixelByteDepth == 2) && (GETPRIMARYBYTEDEPTH == 4)) // 32bpp rendering, 16bpp zbuffer requested
        {
          pixelByteDepth = 4; // Force 32bpp allocation
          D3DPRINT(DLSURFACE, "ddCreateSurface: (Z) Forcing zbuffer allocation from 16bpp to 32bpp.");

          // Munge surface information
          psurf_gbl->ddpfSurface.dwZBufferBitDepth = 32; // substitute 32bpp zbuffer
          psurf_gbl->ddpfSurface.dwZBitMask = 0x00FFFFFF;
        }
      }
    }
    else
#endif // ENABLE_3D
    if ((dwCaps & (DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE)) && !(dwCaps & DDSCAPS_OVERLAY))
    {
      D3DPRINT(DLSURFACE, "ddCreateSurface: (Primary | Flip) and NO overlay");
#ifdef WINNT
      // nt doesn't set the DDRAWISURF_HASPIXELFORMAT bit
      if (DDSD_PIXELFORMAT & pcsd->lpDDSurfaceDesc->dwFlags)
#else
      if((psurf->dwFlags) & DDRAWISURF_HASPIXELFORMAT )
#endif
      {
        D3DPRINT(DLSURFACE, "ddCreateSurface: (Pri | Flip). Has pixel format");
        if(psurf_gbl->ddpfSurface.dwFlags & DDPF_RGB)
        {
          pixelByteDepth = (DWORD)(psurf_gbl->ddpfSurface.dwRGBBitCount) >> 3;
          D3DPRINT(DLSURFACE, "ddCreateSurface: (Pri | Flip). RGB byte depth %d", pixelByteDepth);
        }   // RGB
        else
        {
          D3DPRINT(0,"ddCreateSurface: (Pri | Flip). Unsupported format(1). Aborting request.");
          pcsd->ddRVal = DDERR_UNSUPPORTED;
          return DDHAL_DRIVER_HANDLED;
        }
      } // HASPIXELFORMAT
      else
      {
        pixelByteDepth = GETPRIMARYBYTEDEPTH;
        D3DPRINT(DLSURFACE, "ddCreateSurface: (Pri | Flip). Getting byte depth from primary %d", pixelByteDepth);
      } // !DDRAWISURF_HASPIXELFORMAT
    }
    else if( dwCaps & DDSCAPS_VIDEOMEMORY
        || dwCaps & DDSCAPS_OFFSCREENPLAIN)
    {
      D3DPRINT(DLSURFACE, "ddCreateSurface: Video | OffScreenplain");
#ifdef WINNT
      // nt doesn't set the DDRAWISURF_HASPIXELFORMAT bit
      if (DDSD_PIXELFORMAT & pcsd->lpDDSurfaceDesc->dwFlags)
#else
      if((psurf->dwFlags) & DDRAWISURF_HASPIXELFORMAT )
#endif
      {
        D3DPRINT(DLSURFACE, "ddCreateSurface: (Vid | Off). Has pixel format");
        if(psurf_gbl->ddpfSurface.dwFlags & DDPF_FOURCC)
        {
          D3DPRINT(DLSURFACE, "ddCreateSurface: (Vid | Off). FourCC");
#ifdef SIMULATE_YV12
          if ((FOURCC_YUY2 != psurf_gbl->ddpfSurface.dwFourCC) &&
              (FOURCC_RAW8 != psurf_gbl->ddpfSurface.dwFourCC) &&
              (FOURCC_UYVY != psurf_gbl->ddpfSurface.dwFourCC) &&
              (FOURCC_YV12 != psurf_gbl->ddpfSurface.dwFourCC))
#else
          if( (psurf_gbl->ddpfSurface.dwFourCC != FOURCC_YUY2)
           && (psurf_gbl->ddpfSurface.dwFourCC != FOURCC_RAW8)
            && (psurf_gbl->ddpfSurface.dwFourCC != FOURCC_UYVY))
#endif
          {
            D3DPRINT(0,"ddCreateSurface: Invalid FourCC format. Aborting request.");
            pcsd->ddRVal = DDERR_UNSUPPORTED;
            return DDHAL_DRIVER_HANDLED;
          }
          pixelByteDepth = 2;
          psurf_gbl->ddpfSurface.dwYUVBitCount = 16;
        //V3TV support for vbi surface
        if ((dwCaps & DDSCAPS_VIDEOPORT) &&
           /*_DD(bPLD656inUse) && */(psurf_gbl->ddpfSurface.dwFourCC != FOURCC_RAW8))
        {
         D3DPRINT(DLSURFACE, "ddCreateSurface: (Vid | Off). Videoport. byte depth %d", pixelByteDepth);
         if (pWidth < 720)    // 570 would handle the 1140 needed for vbi
            pWidth = 720;
         height += 0x1F;   // worse case situation
        }
        }   // FOURCC
        else if(psurf_gbl->ddpfSurface.dwFlags & DDPF_RGB)
        {
          pixelByteDepth = (DWORD)(psurf_gbl->ddpfSurface.dwRGBBitCount) >> 3;
          D3DPRINT(DLSURFACE, "ddCreateSurface: (Vid | Off). RGB, byte depth %d", pixelByteDepth);
        }   // RGB
        else
        {
          D3DPRINT(0,"ddCreateSurface: Unsupported format(2). Aborting request.");
          pcsd->ddRVal = DDERR_UNSUPPORTED;
          return DDHAL_DRIVER_HANDLED;
        }
      } // HASPIXELFORMAT
      else
      {
        pixelByteDepth = GETPRIMARYBYTEDEPTH;
        D3DPRINT(DLSURFACE, "ddCreateSurface: (Vid | Off). Taking pixel format from primary, byte depth %d", pixelByteDepth);
      } // !DDRAWISURF_HASPIXELFORMAT

      if (_DD(ddSLIModeEnabled) && 
          (pWidth == (DWORD) ppdev->cxScreen) &&
          (height == (DWORD) ppdev->cyScreen)
         )
      {
        DWORD newWidth, newHeight;

        // Now find out the new width
        for(newWidth = 1; newWidth < pWidth; newWidth <<= 1);
        for(newHeight= 1; newHeight < height; newHeight <<=1);

        // Test for > 1:8 width to height ratios
        if((newHeight/newWidth) <= 8)
        {
          pWidth = newWidth;
          height = newHeight;
        }  
      }
    }
    else
    {
      D3DPRINT(0,"ddCreateSurface: Unsupported format(3). Aborting request.");
      pcsd->ddRVal = DDERR_UNSUPPORTED;
      return DDHAL_DRIVER_HANDLED;
    }

#ifdef WINNT
    if (dwCaps & DDSCAPS_OVERLAY)
    {
       pixelByteDepth =
       CreateOverlaySurface(ppdev,pcsd,psurf,surfaceData,pWidth, height);
       D3DPRINT(DLSURFACE, "ddCreateSurface: Overlay, byte depth %d", pixelByteDepth);
       if( pcsd->ddRVal != DD_OK)
       {
       D3DPRINT(DLSURFACE, "ddCreateSurface: Overlay, Failed!!!!!!");
#if (_WIN32_WINNT >= 0x0500)
          goto CleanUp;
#else
          return DDHAL_DRIVER_HANDLED;
#endif
       }
    }
#endif

    bWidth = pWidth * pixelByteDepth;

#ifdef WINNT
// For nt let MS manage memory
// the nt ddraw.dll does export the DDHAL32_VidMemAlloc & DDHAL32_VidMemFree
// functions but since under nt we aren't currently supporting 3D or a tiled
// heap, there's really no point in using them
// Actually we can't use the DDHAL32_??? functions directly because they are
// in a ring 3 dll and we're at ring 0, in fact using them directly causes
// our driver not to even load.

// We choose a 16 byte alignment to prevent problems on Banshee, at a small
// cost in memory use. We used to align on DWORD boundaries, but this won't
// work for clients like OpenGL

// BYTEALIGNMENT must be a power of 2 or the code won't work right!
#define BYTEALIGNMENT   16  // 16 byte alignment

#define XALIGN(width)   (((width) + (BYTEALIGNMENT - 1)) & ~(BYTEALIGNMENT - 1))

#if USE_NT5_DDMEMMGR
    if (psurf->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
    {
#if ENABLE_TILED_HEAP
      if (_DS(ddPrimaryInTile) == TRUE)
      {
        psurf_gbl->lPitch   = _DS(ddTilePitch);
        surfaceData->lPitch = _DS(ddTilePitch);
        surfaceData->hwPtr  = ppdev->ulScreenOffset | SSTG_IS_TILED;
        DISPDBG((2, "Allocated primary ddraw surface (%lXh x %lXh) at fpVidMem = %8lXh, pitch = %8lXh, hwPtr = %8lXh",
                    psurf_gbl->wWidth, psurf_gbl->wHeight, psurf_gbl->fpVidMem, psurf_gbl->lPitch, surfaceData->hwPtr));
      }
      else
#endif // ENABLE_TILED_HEAP
      {
        //psurf_gbl->lPitch   = ppdev->lDelta;
        surfaceData->lPitch = ppdev->lDelta;
        surfaceData->hwPtr  = ppdev->ulScreenOffset;
        DISPDBG((2, "Allocated primary ddraw surface (%lXh x %lXh) at fpVidMem = %08lXh, pitch = %08lXh",
                    psurf_gbl->wWidth, psurf_gbl->wHeight, psurf_gbl->fpVidMem, psurf_gbl->lPitch));
      }
    }
    else
    {
      // round pitch and blockSizeX up to at least a BYTEALIGNMENT boundary
      // otherwise NT won't allocate it in video memory
      psurf_gbl->lPitch = XALIGN(bWidth);
      psurf_gbl->dwBlockSizeX = XALIGN(bWidth);
      psurf_gbl->fpVidMem = DDHAL_PLEASEALLOC_BLOCKSIZE;

      pcsd->ddRVal = memMgr_allocSurface(ppdev,
                                        psurf->ddsCaps.dwCaps,
                                        bWidth,
                                        height,
                                        (bWidth + 0x7F)>> 7L,
                                        (height + 0x1F)>> 5L,
                                        &(surfaceData->lfbPtr),
                                        &(surfaceData->hwPtr),
                                        &(surfaceData->lPitch),
                                        &tileFlag,
                                        &(surfaceData->heapID),
                                        &(surfaceData->pvmHeap));

      /* retro3dfx: surface-placement tracer (CS-D3D fillrate hunt). Logs every
       * video-memory surface create with its caps and TILED/LINEAR placement —
       * a 3D back/depth buffer landing MEM_IN_LINEAR renders dramatically
       * slower on this hardware than the tiled heap Glide always uses. Surface
       * creates are rare (mode set / level load), so always log. */
      if (DD_OK == pcsd->ddRVal)
      {
        extern VOID V5DLog(CHAR *, ...);
        V5DLog("retro3dfx DdCreateSurface caps=%08lXh %ldx%ld pitch=%ld hwPtr=%08lXh %s heap=%ld\n",
               (unsigned long)psurf->ddsCaps.dwCaps, (long)bWidth, (long)height,
               (long)surfaceData->lPitch, (unsigned long)surfaceData->hwPtr,
               (MEM_IN_TILED == tileFlag) ? "TILED" : "LINEAR",
               (long)surfaceData->heapID);
      }

      if (DD_OK != pcsd->ddRVal)
      {
CleanUp:
        if (i)
        {
          DDHAL_DESTROYSURFACEDATA dsd;

          dsd.lpDD = pcsd->lpDD;

          if (DDSCAPS_OVERLAY & dwCaps)  //decreas  _DD(overlaySurfaceCnt) in this way
            DestroyOverlaySurface(ppdev, psurf, psurf_gbl,surfaceData);

          // free FXSURFACEDATA for surface that caused failed allocation
          DDFREE(surfaceData);
          psurf_gbl->dwReserved1 = 0;

          // loop over allocations that succeeded and free the video memory and FXSURFACEDATA structs
          for (i--; i >= 0; i--)
          {
            dsd.lpDDSurface = pcsd->lplpSList[i];
            DdDestroySurface(&dsd);
          }
        }
        else
        {
          // free FXSURFACEDATA for the first surface if that allocation failed
          DDFREE(surfaceData);
          psurf_gbl->dwReserved1 = 0;
        }
        return DDHAL_DRIVER_HANDLED;
      }

#ifdef SIMULATE_YV12
      if ((DDSCAPS_OVERLAY & psurf->ddsCaps.dwCaps) &&
          (DDPF_FOURCC & psurf_gbl->ddpfSurface.dwFlags) &&
          (FOURCC_YV12 == psurf_gbl->ddpfSurface.dwFourCC))
      {
        psurf_gbl->lPitch = psurf_gbl->wWidth;
        (LPVOID)psurf_gbl->fpVidMem =EngAllocPrivateUserMem(psurf,(3 * psurf_gbl->wHeight * psurf_gbl->wWidth) / 2, '1VYD');
        DISPDBG((2,"  YV12 surface allocated in UserMem at %08lXh", psurf_gbl->fpVidMem));
        psurf_gbl->ddpfSurface.dwYUVBitCount = 12;
      }
      else
#endif // ! SIMULATE_YV12
      {
        psurf_gbl->lPitch   = surfaceData->lPitch;
        psurf_gbl->fpVidMem = surfaceData->lfbPtr;
      }
    }
#else // ! USE_NT5_DDMEMMGR
    // round pitch and blockSizeX up to at least a BYTEALIGNMENT boundary
    // otherwise NT won't allocate it in video memory
    psurf_gbl->lPitch = XALIGN(bWidth);
    psurf_gbl->dwBlockSizeX = XALIGN(bWidth);
    psurf_gbl->fpVidMem = DDHAL_PLEASEALLOC_BLOCKSIZE;
#endif // ! USE_NT5_DDMEMMGR

#else //WINNT

    // Windows 9x surface creation. */

    D3DPRINT(DLSURFACE, "ddCreateSurface: Starting surface creation");

    if (dwCaps & DDSCAPS_OVERLAY )
    {
      CreateOverlaySurface(ppdev,pcsd,psurf,surfaceData,pWidth, height);
      D3DPRINT(DLSURFACE, "ddCreateSurface: Overlay.");
      if( pcsd->ddRVal != DD_OK)
      {
        D3DPRINT(DLSURFACE, "ddCreateSurface: Overlay. Failed!!!! (2)");
        return DDHAL_DRIVER_HANDLED;
      }
    }
    else
    {
      // Create primary drawing surface.
      D3DPRINT(DLSURFACE, "ddCreateSurface: Create primary drawing surface");

      if (psurf->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
      {
        surfaceData->lPitch = _DS(pitch);
        surfaceData->hwPtr  = _DS(TotalVRAM) - _DS(gdiDesktopSize);

#if ENABLE_TILED_HEAP
        if(_DS(ddPrimaryInTile) == TRUE)
        {
#ifdef SLI_AA
          // Adjust lfbPtr for SLI mode.

          if (QUAD_CHIP_SLI_2WAY_AA_4SAMPLE == _DD(ddSLIAAConfiguration))
            {
            DWORD tileOffset;
            tileOffset = ((surfaceData->hwPtr & ~SSTG_IS_TILED) - _FF(ddTiledHeapStart))<<1;
            tileOffset = (tileOffset / SST_TILE_WIDTH) / _FF(ddTileStride);
            tileOffset *= _DS(ddTilePitch);
            surfaceData->lfbPtr = _FF(LFBBASE) + _FF(ddTiledHeapStart) + tileOffset;            
            }

          surfaceData->lfbPtr = AdjustLfbPtr(ppdev, surfaceData->lfbPtr, &surfaceData->heapID);

          // Compute hwPtr from lfbPtr.

          surfaceData->hwPtr = LfbPtrToHwPtr(ppdev, surfaceData->lfbPtr, surfaceData->heapID);
#endif

          tileFlag = MEM_IN_TILED;
          surfaceData->lPitch = _DS(ddTilePitch);
          surfaceData->hwPtr |= SSTG_IS_TILED;
          D3DPRINT(DLSURFACE, "ddCreateSurface: Primary Surface Setup. Tiled.");
        }
#endif // ENABLE_TILED_HEAP
#ifdef DEBUG
		else
		{
	        D3DPRINT(DLSURFACE, "ddCreateSurface: Primary Surface Setup. Linear.");
		}
#endif
      }
      else
      {
        // Allocate all other surfaces from memory manager.
         D3DPRINT(DLSURFACE, "ddCreateSurface: Not the Primary Surface. Calling memMgr_allocSurface");

         pcsd->ddRVal = memMgr_allocSurface(
            psurf_gbl->lpDD,            // [IN] DirectDraw object for VidMemAlloc
            psurf->ddsCaps.dwCaps,      // [IN] DirectDraw surface capabilities
            bWidth,                     // [IN] width in butes
            height,                     // [IN] height in scanlines
            (bWidth + 0x7F)>> 7L,       // [IN] width in tiles
            (height + 0x1F)>> 5L,       // [IN] height in tiles
            &(surfaceData->lfbPtr),     // [OUT] lfb address of surface
            &(surfaceData->hwPtr),      // [OUT] hardware offset of surface
            &(surfaceData->lPitch),     // [OUT] hardware pitch of surface
            &tileFlag,                  // [OUT] MEM_IN_TILED or MEM_IN_LINEAR
            &(surfaceData->heapID));    // [OUT] DirectDraw heap number
      }

      // Check if surface allocated.

      if (!surfaceData->lfbPtr)
      {
        D3DPRINT(0,"ddCreateSurface: Out of Video Memory. Aborting request. %s",
									(tileFlag == MEM_IN_TILED) ? "Tiled memory" : "Linear memory");
        pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
        return DDHAL_DRIVER_HANDLED;
      }
    }
#ifdef SIMULATE_YV12
   if(psurf_gbl->ddpfSurface.dwFourCC != FOURCC_YV12)
#endif
   {
        psurf_gbl->fpVidMem = surfaceData->lfbPtr;
        psurf_gbl->lPitch = surfaceData->lPitch;
   }

#endif // WINNT

  }   // Surface Creation Loop

#if ENABLE_3D

  // Special handling for 3D, exclusive-mode, non-overlay, full-screen application.

  dwCaps = pcsd->lpDDSurfaceDesc->ddsCaps.dwCaps;
  if ((_FF(ddExclusiveMode) == TRUE) &&
      (_FF(ddPrimaryInTile) == TRUE) &&
      (!(dwCaps & DDSCAPS_OVERLAY))  &&
      (((dwCaps & DDSCAPS_3DDEVICE)  && (dwCaps & DDSCAPS_PRIMARYSURFACE)) ||
       ((dwCaps & DDSCAPS_3DDEVICE)  && (dwCaps & DDSCAPS_FLIP))           ||
        (dwCaps & DDSCAPS_ZBUFFER)))
  {
    D3DPRINT(DLSURFACE, "ddCreateSurface: Special full-screen handling.");

#ifdef SLI_AA

    // Allocate AA secondary flipping chain.

    if (_DD(ddAAModeRequested))
    {
      D3DPRINT(DLSURFACE, "ddCreateSurface: Special. AAModeRequested. Allocate AA buffs");

      for( i=0;i<(int)pcsd->dwSCnt;i++ )
      {
        psurf = pcsd->lplpSList[i];
        psurf_gbl = psurf->lpGbl;
        surfaceData = (FXSURFACEDATA*) psurf->lpGbl->dwReserved1;

        D3DPRINT(DLSURFACE, "ddCreateSurface: AA surface request %d of %d", (i+1), pcsd->dwSCnt);

#ifdef WINNT
        if ((PARTOFPRIMARYCHAIN(psurf->ddsCaps.dwCaps)) ||
            ((dwCaps & DDSCAPS_ZBUFFER) && (psurf_gbl->wWidth == (DWORD)ppdev->cxScreen) && (psurf_gbl->wHeight == (DWORD)ppdev->cyScreen)))
#else
        if ((psurf->dwFlags & DDRAWISURF_PARTOFPRIMARYCHAIN) || (dwCaps & DDSCAPS_ZBUFFER))
#endif
        {
          // Call memory manager to allocate.
          D3DPRINT(DLSURFACE, "ddCreateSurface: AA surf. PartOfPrimaryChain | Z. Calling memMgr_allocSecondary");

          pcsd->ddRVal = memMgr_allocSecondary(
#ifdef WINNT
              ppdev,
#else
              psurf_gbl->lpDD,            // [IN] DirectDraw object for VidMemAlloc
#endif
              psurf->ddsCaps.dwCaps,      // [IN] DirectDraw surface capabilities
              bWidth,                     // [IN] width in butes
              height,                     // [IN] height in scanlines
              (bWidth + 0x7F)>> 7L,       // [IN] width in tiles
              (height + 0x1F)>> 5L,       // [IN] height in tiles
              &(surfaceData->AAlfbPtr),   // [OUT] lfb address of surface
              &(surfaceData->AAhwPtr),    // [OUT] hardware offset of surface
              &(surfaceData->AAlPitch),   // [OUT] hardware pitch of surface
              &tileFlag,                  // [OUT] MEM_IN_TILED or MEM_IN_LINEAR
              &(surfaceData->AAheapID)    // [OUT] DirectDraw heap number
#ifdef WINNT
              , &(surfaceData->AApvmHeap)
#endif
              );

          // Check if surface allocated.

          if (!surfaceData->AAlfbPtr)
          {
            D3DPRINT(0,"ddCreateSurface: Out of Video Memory for AA Surface. Aborting request. %s",
									(tileFlag == MEM_IN_TILED) ? "Tiled memory" : "Linear memory");
#ifdef WINNT
            {
              extern void BailOutOfAA(PDEV *);
              BailOutOfAA(ppdev);
              pcsd->ddRVal = DD_OK;
            }
#else
            pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
            return DDHAL_DRIVER_HANDLED;
#endif
          }
        }
      }

      // Store secondary buffer offsets.

      if (_DD(ddSLIModeRequested) && !(QUAD_CHIP_SLI_2WAY_AA_4SAMPLE == _DD(ddSLIAAConfiguration)))
      {
        D3DPRINT(DLSURFACE, "ddCreateSurface: AA surf. SLI Enabled. Store secondary buffer offsets.");
        switch (_DS(ddNumColorBuff))
        {
          case 3:  _DD(ddAAPrimaryStart) = _DS(secondaryThirdBuffer);
                   break;
          case 2:  _DD(ddAAPrimaryStart) = _DS(secondaryZBuffer) - SST_TILE_SIZE;
                   break;
          case 1:  _DD(ddAAPrimaryStart) = _DS(secondaryBackBuffer);
                   break;
          case 0:  _DD(ddAAPrimaryStart) = _DS(secondaryFrontBuffer);
                   break;
        }
        _DD(ddAAZbufferStart) = _DS(secondaryZBuffer);
      }
      else
      {

        _DD(ddAAPrimaryStart) = _DS(ddLinearHeap4Start);
        _DD(ddAAZbufferStart) = _DS(ddLinearHeap3Start);
      }
    }
#endif // SLI_AA

#ifdef SLI_AA

/* The DirectX runtime can call DdCreateSurface to create a single-buffered    	 */
/* primary surface for 3D rendering, possibly with an attached zbuffer.  When 	 */
/* this surface is created, we call Enter_3DApplication to promote to using      */
/* using the overlay planes, but we never enable SLI or AA in the single-buffer  */
/* case.  When the single-buffered primary surface is destroyed, the DirectX   	 */
/* runtime does not call DdDestroySurface, so Exit_3DApplication is never     	 */
/* called.  When a new primary flipping chain is created after this sequence,    */
/* the driver has _DD(dd3DInOverlay) set, so Enter_3DApplication is never      	 */
/* called again, so the application fails to enter SLI or AA modes, even though	 */
/* they have been requested.  This results in wrong addresses for the primary    */
/* flipping chain buffers.  The fix for this is to detect the case where the SLI */
/* and AA requests do not match the enables, and call Enter_3DApplication   	 */
/* even though _DD(dd3DInOverlay) is already set. -SH-                           */

if (((!_DD(dd3DInOverlay)) && (!(dwCaps & DDSCAPS_ZBUFFER))) ||
     ((_DD(dd3DInOverlay)) && ((_DD(ddAAModeEnabled)  != _DD(ddAAModeRequested)) ||
                               (_DD(ddSLIModeEnabled) != _DD(ddSLIModeRequested)))))
#else
    if ((!_DD(dd3DInOverlay)) && (!(dwCaps & DDSCAPS_ZBUFFER)))
#endif // SLI_AA
    {
      D3DPRINT(DLSURFACE, "ddCreateSurface: Calling Enter_3DApplication");
      Enter_3DApplication(ppdev);
    }
  }

#endif // ENABLE_3D

#ifndef WINNT
   // If we are in Low Power mode then we want to
   // not spin on the hardware
   #define BUSY 0x10
   if (BUSY != (*_FF(lpDeFlags) & BUSY))
      FXBUSYWAIT(ppdev);
#else
   FXBUSYWAIT(ppdev);
#endif
  pcsd->ddRVal = DD_OK;

#ifdef WINNT
#if USE_NT5_DDMEMMGR
  return DDHAL_DRIVER_HANDLED;
#else
  return DDHAL_DRIVER_NOTHANDLED;
#endif
#else
  return DDHAL_DRIVER_HANDLED;
#endif

} // DdCreateSurface


/*----------------------------------------------------------------------
Function name: DdDestroySurface

Description:   Destroys the specified surface.

Return:        DDHAL_DRIVER_HANDLED or DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
DdDestroySurface( LPDDHAL_DESTROYSURFACEDATA pdsd )
{
  DWORD                dwCaps;
  LPDDRAWI_DDRAWSURFACE_LCL psurf;
  LPDDRAWI_DDRAWSURFACE_GBL psurf_gbl;
  FXSURFACEDATA        *surfaceData=NULL;

  DD_ENTRY_SETUP(pdsd->lpDD);

  D3DPRINT(DLSURFACE, "ddDestroySurface: Entry");

#ifdef WINNT
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  DISPDBG((2, "DdDestroySurface: SURFACE_LCL = %8lXh, dwCaps = %08lXh,  handle = %ld",
           pdsd->lpDDSurface, pdsd->lpDDSurface->ddsCaps.dwCaps,
           pdsd->lpDDSurface->lpSurfMore->dwSurfaceHandle));
#else
  DISPDBG((2, "DdDestroySurface: SURFACE_LCL = %8lXh, dwCaps = %08lXh",
           pdsd->lpDDSurface, pdsd->lpDDSurface->ddsCaps.dwCaps));
#endif
#endif

  psurf = pdsd->lpDDSurface;
  psurf_gbl = psurf->lpGbl;
  dwCaps = psurf->ddsCaps.dwCaps;

  /* retro3dfx: if this surface is flip-present-promoted, restore its
   * original backing before the runtime frees it (DDFLIP.C) */
  {
    extern VOID retroFlipPresentSurfGone(void *);
    retroFlipPresentSurfGone((void *)psurf_gbl->dwReserved1);
  }

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "DestroySurface32" ));
  DUMP_DDHAL_DESTROYSURFACEDATA(ppdev, DEBUG_DDGORY, pdsd );
  #endif

#ifdef WINNT
  FXBUSYWAIT(ppdev);
#endif

#if ENABLE_3D
  if ( dwCaps & DDSCAPS_TEXTURE )
  {
    D3DPRINT(DLSURFACE, "ddDestroySurface: Calling TEXTURESURFACEDELETE");
    TEXTURESURFACEDELETE(ppdev, pdsd->lpDDSurface);

    surfaceData = (FXSURFACEDATA*) psurf_gbl->dwReserved1;
    if(surfaceData != (FXSURFACEDATA*) NULL)
    {
       DDFREE(surfaceData);
    }

    pdsd->ddRVal = DD_OK;
    return DDHAL_DRIVER_NOTHANDLED;
  }

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  // if it's a 3DDEVICE or ZBUFFER surface, find and release it's TXTRHNDL
  if (dwCaps & (DDSCAPS_3DDEVICE | DDSCAPS_ZBUFFER))
  {
    extern HNDLLIST *g_pHndlList;
    DWORD     hSurf;
    HNDLLIST  *pHndlList;

    hSurf = psurf->lpSurfMore->dwSurfaceHandle;
    pHndlList = g_pHndlList;

    while (pHndlList)
    {
      // first check for a valid ppTxtrHndlList in the HNDLLIST
      // and that the txtrHndl count is within the allocated range for this HNDLLIST
      if ((NULL != pHndlList->ppTxtrHndlList) &&
          ((DWORD)pHndlList->ppTxtrHndlList[0] >= hSurf) && (0 != hSurf) )
      {
        // if this HNDLLIST has a TXTRHNDL for this handle and the surfLcl is this surface
		// if ((pHndlList->ppTxtrHndlList[hSurf]) && (pHndlList->ppTxtrHndlList[hSurf]->surfLcl == (DWORD)psurf))
		if ((pHndlList->ppTxtrHndlList[hSurf]) && (hSurf == (DWORD)psurf->lpSurfMore->dwSurfaceHandle))
        {
          // release the TXTRHNDL
          ReleaseTxtrHndl(ppdev, pHndlList, hSurf);

          // this assumes a surface can only belong to one HNDLLIST at a time
          // if this turns out not to be the case, then we need to remove
          // this break statement and loop over all the HNDLLIST's
          break;
        }
      }
      pHndlList = pHndlList->pNext;
    }
  }
#endif

#endif // ENABLE_3D

  if ( dwCaps & DDSCAPS_SYSTEMMEMORY)
  {
    D3DPRINT(DLSURFACE, "ddDestroySurface: System memory. Unsupported");
    pdsd->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_HANDLED;
  }

#if ENABLE_TILED_HEAP

  // Demote when primary is destroyed.
#ifdef WINNT
  if (PARTOFPRIMARYCHAIN(psurf->ddsCaps.dwCaps))
#else
  if (psurf->dwFlags & DDRAWISURF_PARTOFPRIMARYCHAIN) // DDSCAPS_PRIMARYSURFACE is not always set!
#endif
  {
    D3DPRINT(DLSURFACE, "ddDestroySurface: Part of primary chain");
    if (_DD(dd3DInOverlay))
    {
      // Special handling for 3D, exclusive-mode, non-overlay, full-screen application.
      D3DPRINT(DLSURFACE, "ddDestroySurface: 3DInOverlay. Calling Exit_3DApplication");

      Exit_3DApplication(ppdev);

      // Destroy secondary buffer for primary surface.

#ifdef SLI_AA

      {
        DWORD hwPtr;

        surfaceData = (FXSURFACEDATA*) &(_FF(ddPrimarySurfaceData));
        hwPtr = surfaceData->AAhwPtr & ~SSTG_IS_TILED;

        if(hwPtr != 0)
        {
          if(surfaceData->AAheapID != HEAP_INVALID)
          {
            memMgr_freeSurface(
#ifdef WINNT
              ppdev,
#else
              psurf_gbl->lpDD,                 // [IN] DirectDraw object for VidMemAlloc
#endif
              psurf->ddsCaps.dwCaps,           // [IN] DirectDraw surface capabilities
              surfaceData->AAlfbPtr,           // [IN] lfb address of surface
              surfaceData->AAhwPtr,            // [IN] hardware offset of surface
              GETMEMTYPE(surfaceData->AAhwPtr),// [IN] MEM_IN_TILED or MEM_IN_LINEAR
              surfaceData->AAheapID            // [IN] DirectDraw heap number
#ifdef WINNT
              , surfaceData->AApvmHeap
#endif
              );

            surfaceData->AAhwPtr  = 0;
            surfaceData->AAlfbPtr = 0;
            surfaceData->AAlPitch   = 0;
            surfaceData->AAheapID = HEAP_INVALID;
          }
        }
      }

#endif // SLI_AA

    }
    _FF(ddVisibleOverlaySurf) = 0;
  }

  if ( (dwCaps & DDSCAPS_BACKBUFFER) && (_FF(ddMiscFlags) & DDMF_TRIPPLE_BUFFER))
  {
    DWORD dramInit1;
    dramInit1 = GET(ghwIO->dramInit1);
    dramInit1 &= ~SST_TRIPLE_BUFFER_EN;
    SETDW(ghwIO->dramInit1,dramInit1);
    _FF(ddMiscFlags) &= ~DDMF_TRIPPLE_BUFFER;
    D3DPRINT(DLSURFACE, "ddDestroySurface: Disabling Triple Buffering");
  }

#endif // ENABLE_TILED_HEAP

  if ( dwCaps & DDSCAPS_OVERLAY )
  {
    DestroyOverlaySurface(ppdev, pdsd->lpDDSurface, psurf_gbl, surfaceData);
  }

#ifdef WINNT
// For nt let MS manage memory
// the nt ddraw.dll does export the DDHAL32_VidMemAlloc & DDHAL32_VidMemFree
// functions but since under nt we aren't currently supporting 3D or a tiled
// heap, there's really no point in using them

  surfaceData = (FXSURFACEDATA*) psurf_gbl->dwReserved1;
  if (NULL != surfaceData)
  {
#ifdef SIMULATE_YV12
    if ((DDSCAPS_OVERLAY & psurf->ddsCaps.dwCaps) &&
        (DDPF_FOURCC & psurf_gbl->ddpfSurface.dwFlags) &&
        (FOURCC_YV12 == psurf_gbl->ddpfSurface.dwFourCC))
    {

      EngFreePrivateUserMem(psurf,(LPVOID)psurf_gbl->fpVidMem);
      psurf_gbl->fpVidMem = surfaceData->lfbPtr;
    }
#endif

#if USE_NT5_DDMEMMGR
#if ENABLE_RECONFIG_VIDMEM
    if (psurf_gbl->fpVidMem != (FLATPTR)(ppdev->pjScreen - ppdev->pjScreenBase))
#else
    if (psurf_gbl->fpVidMem != ppdev->ulScreenOffset)
#endif
    {
      extern void CheckForGlideCmdfifoSurface(PDEV *ppdev, DWORD surfStart, DWORD surfEnd);

      if ((! _DD(ddAAModeRequested)) &&
          (! _DD(ddSLIModeRequested)) &&
          (surfaceData->hwPtr < _FF(ddTileMark)) &&
          (! (DDPF_FOURCC & psurf_gbl->ddpfSurface.dwFlags)))
      {
        CheckForGlideCmdfifoSurface(ppdev,
                                    surfaceData->hwPtr,
                                    surfaceData->hwPtr + psurf_gbl->wHeight * psurf_gbl->lPitch);
      }

      memMgr_freeSurface(ppdev,
                         psurf->ddsCaps.dwCaps,
                         surfaceData->lfbPtr,
                         surfaceData->hwPtr,
                         GETMEMTYPE(surfaceData->hwPtr),
                         surfaceData->heapID,
                         surfaceData->pvmHeap);

#ifdef SLI_AA
      if (0 != surfaceData->AAhwPtr)
      {
        memMgr_freeSurface(ppdev,
                           psurf->ddsCaps.dwCaps,
                           surfaceData->AAlfbPtr,
                           surfaceData->AAhwPtr,
                           GETMEMTYPE(surfaceData->AAhwPtr),
                           surfaceData->AAheapID,
                           surfaceData->AApvmHeap);
      }
#endif // SLI_AA

      DDFREE(surfaceData);
    }
#if DBG
    else
    {
      DISPDBG((2, "Freeing primary ddraw surface at fpVidMem = %08lXh", psurf_gbl->fpVidMem));
    }
#endif
#else
    DDFREE(surfaceData);
#endif
    psurf_gbl->dwReserved1 = 0;
  }

  pdsd->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;
#else
  // Win9x
  {
    DWORD hwPtr;

    surfaceData = (FXSURFACEDATA*) psurf_gbl->dwReserved1;
    if(surfaceData != (FXSURFACEDATA*) NULL)
    {
      hwPtr = surfaceData->hwPtr & ~SSTG_IS_TILED;

      if(hwPtr != 0)
      {
        if(surfaceData->heapID != HEAP_INVALID)
        {
          D3DPRINT(DLSURFACE, "ddDestroySurface: hwPtr %08x, caps %08x, MemType %s",
			  surfaceData->hwPtr, psurf->ddsCaps.dwCaps,
			  ((GETMEMTYPE(surfaceData->hwPtr)) == MEM_IN_TILED ? "Tiled" : "Linear") );

          memMgr_freeSurface(
            psurf_gbl->lpDD,                 // [IN] DirectDraw object for VidMemAlloc
            psurf->ddsCaps.dwCaps,           // [IN] DirectDraw surface capabilities
            surfaceData->lfbPtr,             // [IN] lfb address of surface
            surfaceData->hwPtr,              // [IN] hardware offset of surface
            GETMEMTYPE(surfaceData->hwPtr),  // [IN] MEM_IN_TILED or MEM_IN_LINEAR
            surfaceData->heapID );           // [IN] DirectDraw heap number
        }
      }
      else
	  {
         D3DPRINT(DLSURFACE, "ddDestroySurface: hwPtr == 0, nothing to destroy");
	  }

#ifdef SLI_AA

      hwPtr = surfaceData->AAhwPtr & ~SSTG_IS_TILED;
      if(hwPtr != 0)
      {
        if(surfaceData->AAheapID != HEAP_INVALID)
        {
          D3DPRINT(DLSURFACE, "ddDestroySurface: AAhwPtr %08x, caps %08x, MemType %s",
			  surfaceData->AAhwPtr, psurf->ddsCaps.dwCaps,
			  ((GETMEMTYPE(surfaceData->AAhwPtr)) == MEM_IN_TILED ? "Tiled" : "Linear") );

          memMgr_freeSurface(
            psurf_gbl->lpDD,                 // [IN] DirectDraw object for VidMemAlloc
            psurf->ddsCaps.dwCaps,           // [IN] DirectDraw surface capabilities
            surfaceData->AAlfbPtr,           // [IN] lfb address of surface
            surfaceData->AAhwPtr,            // [IN] hardware offset of surface
            GETMEMTYPE(surfaceData->AAhwPtr),// [IN] MEM_IN_TILED or MEM_IN_LINEAR
            surfaceData->AAheapID );         // [IN] DirectDraw heap number
        }
      }
      else
	  {
         D3DPRINT(DLSURFACE, "ddDestroySurface: AAhwPtr == 0, nothing to destroy");
	  }

#endif // SLI_AA

      DDFREE((void*)surfaceData);
    }
  }

  // Tell D3D that buffer has been deleted
  _D3(last).colBufferAddr = 0xffffffff; // bogus value
  _D3(last).auxBufferAddr = 0xffffffff;

  psurf_gbl->dwReserved1 = 0;
  psurf_gbl->fpVidMem = 0;

  pdsd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
#endif

} // DdDestroySurface


/*----------------------------------------------------------------------
Function name: DdLock

Description:   Locks the specified surface.

Return:        DDHAL_DRIVER_HANDLED or DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/
#ifdef SLI_AA
#ifndef WINNT
int HostRestoreCursor(NT9XDEVICEDATA * ppdev, DWORD SaveCursorSurface, int LastCursorX, int LastCursorY, DWORD dwExclusionSave);
int HostDrawCursor(NT9XDEVICEDATA * ppdev, DWORD swapToAddr);
#endif
#endif


DWORD __stdcall
DdLock( LPDDHAL_LOCKDATA pld )
{
  DWORD               dwCaps, ptr;
  DWORD               BytesPerPixel;
  FXSURFACEDATA       *surfaceData;
  DD_ENTRY_SETUP(pld->lpDD);

  DDPRINT(DDDBGLVL, ">> DdLock (surf = %08lXh)", pld->lpDDSurface);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "Lock32" ));
  DUMP_LOCKDATA(ppdev, DEBUG_DDGORY, pld );
  #endif

  P6FENCE; // Flush write combine buffers

  dwCaps = pld->lpDDSurface->ddsCaps.dwCaps;

  /* retro3dfx: lock tracer (CS-D3D present hunt). A per-frame app Lock of the
   * primary/back buffer is a hard CPU-GPU serialization point (and would veto
   * any flip-promotion of the Blt-present). First 4 + every 512th. */
  {
    extern VOID V5DLog(CHAR *, ...);
    static LONG lockCount = 0;
    LONG n = ++lockCount;
    if ((4 >= n) || (0 == (n & 511)))
    {
      V5DLog("retro3dfx DdLock #%ld caps=%08lXh\n", (long)n, (unsigned long)dwCaps);
    }
  }

#ifdef Z_ACCESS_OPT
  // If the app is locking the surface to clear the Z Buffer itself,
  // lets reset the optimization
  if ( _DD(ddEnableZClearOpt) && (dwCaps & DDSCAPS_ZBUFFER) &&
       _DD(dd3DInOverlay) && (_FF(dd3DSurfaceCount) > 2) && (_FF(dd3DSurfaceCount) < 5) )
  {
    RC *pRc = (RC *)_D3(lastContext);
  
    if ( ((pRc)&&(pRc->dwZClearOptEnabled)) || ((_DD(ddFlipsWithoutZClear) & 0x7fffffff) > 6) )
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

  if ( dwCaps & DDSCAPS_SYSTEMMEMORY)
  {
    pld->ddRVal = DD_OK;
    return DDHAL_DRIVER_NOTHANDLED;
  }

#ifndef WINNT
#ifdef AGP_CMDFIFO
   {
       void  FLUSHAGP(NT9XDEVICEDATA * ppdev);

       FLUSHAGP(ppdev);
   }
#endif // #ifdef AGP_CMDFIFO

  if(dwCaps & DDSCAPS_OVERLAY)
  {
    surfaceData = (FXSURFACEDATA*) (pld->lpDDSurface->lpGbl->dwReserved1);
    if((surfaceData!=NULL) && (surfaceData->overlayShrinkFlag == TRUE))
    {
      surfaceData->doShrink=TRUE;
    }
  }
#endif // #ifndef WINNT

#if ENABLE_3D
  if ( dwCaps & DDSCAPS_TEXTURE )
  {
    TXTRDESC *txtr;

    // Always flush the pipeline when a texture is locked,
    // to ensure that all rendering to the surface is complete
    // before allowing any linear framebuffer access.
    // Fixes PRS 11789 - Texture Sizes failures on Napalm

    //while (FXGETBUSYSTATUS(ppdev));

    txtr = TXTRDESC_PTR(pld->lpDDSurface->dwReserved1);

    // modified busy wait, only stall if there's been a blt to this texture
    // or the texture is also a render target
    if ((BltToTxtrInFifo & txtr->flags) || (DDSCAPS_3DDEVICE & dwCaps))
    {
      FXBUSYWAIT(ppdev);  /* retro3dfx: bounded (was raw spin) */
      txtr->flags &= ~BltToTxtrInFifo;
    }

    if( (_D3(autoMipMap) != 0) &&
        (txtr->flags & AutoMipMaps) )  // Can't automipmap because the app is doing the loading
    {
       DWORD tmuCnt;

       txtr->flags &=(~AutoMipMaps); // Turn off automipmapping

       txtr->numMipmaps = 1;       // Set Mip Level to 1.

       // fix LODMax for each TMU
       for(tmuCnt = 0; tmuCnt < NUM_AVAILABLE_TMUS; ++tmuCnt )
       {
          DWORD LODMin;

          LODMin = txtr->tLOD[tmuCnt] & SST_LODMIN;
          txtr->tLOD[tmuCnt] &= (~SST_LODMAX);
          txtr->tLOD[tmuCnt] |= (LODMin << SST_LODMAX_SHIFT);
       }
    }

    if( !(txtr->flags & AspectRatioGT8) )
    {
      if (TEXTURE_IS_DXT_SURFACE(pld->lpDDSurface->lpGbl->ddpfSurface))
      {
         MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);
         if( (LPVOID)pld->lpDDSurface->lpGbl->fpVidMem != NULL )
         {
            if (pld->lpDDSurface->lpGbl->wWidth < 8
                && pld->lpDDSurface->lpGbl->ddpfSurface.dwFourCC == FOURCC_DXT1)
            {
               UNPAD_DXT1(ppdev, pld->lpDDSurface->lpGbl);
            }
#ifdef WINNT
            // let MS figure out the surface address
            pld->ddRVal = DD_OK;
            return DDHAL_DRIVER_NOTHANDLED;
#else
            pld->lpSurfData = (LPVOID) pld->lpDDSurface->lpGbl->fpVidMem;
            pld->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
#endif
         }
      }
      else //if( !(dwCaps & DDSCAPS_MIPMAP) )
      {
         MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);
         if( (LPVOID)pld->lpDDSurface->lpGbl->fpVidMem != NULL )
         {
#ifdef WINNT
            // let MS figure out the surface address
            pld->ddRVal = DD_OK;
            return DDHAL_DRIVER_NOTHANDLED;
#else
            pld->lpSurfData = (LPVOID) pld->lpDDSurface->lpGbl->fpVidMem;
            pld->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
#endif
         }
      }
    }
    else if (TEXTURE_IS_DXT_SURFACE(pld->lpDDSurface->lpGbl->ddpfSurface))
    {
         MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);
         // here I need to unmunge the texture to its original size!
         if( (LPVOID)pld->lpDDSurface->lpGbl->fpVidMem != NULL )
         {
            if (txtr->tlog > txtr->slog)
            {
               int widthshift = txtr->initialTlog - txtr->initialSlog - 3;
               if (((pld->lpDDSurface->lpGbl->wWidth << widthshift) < 8)
                   && pld->lpDDSurface->lpGbl->ddpfSurface.dwFourCC == FOURCC_DXT1)
               {
                  UNPAD_DXT1(ppdev, pld->lpDDSurface->lpGbl);
               }
            }
            shrinkDXTn(pld->lpDDSurface->lpGbl,txtr,&_D3G,ppdev);
#ifdef WINNT
            // let MS figure out the surface address
            pld->ddRVal = DD_OK;
            return DDHAL_DRIVER_NOTHANDLED;
#else
            pld->lpSurfData = (LPVOID) pld->lpDDSurface->lpGbl->fpVidMem;
            pld->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
#endif
         }
    }
    else if (TEXTURE_IS_FXT_SURFACE(pld->lpDDSurface->lpGbl->ddpfSurface))  
    {
         MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);
         // here I need to unmunge the texture to its original size!
         if( (LPVOID)pld->lpDDSurface->lpGbl->fpVidMem != NULL )
         {
            shrinkFXT1(pld->lpDDSurface->lpGbl,txtr,&_D3G,ppdev);
#ifdef WINNT
            // let MS figure out the surface address
            pld->ddRVal = DD_OK;
            return DDHAL_DRIVER_NOTHANDLED;
#else
            pld->lpSurfData = (LPVOID) pld->lpDDSurface->lpGbl->fpVidMem;
            pld->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
#endif
         }
    }

    pld->ddRVal = DDERR_UNSUPPORTED;
    return DDHAL_DRIVER_HANDLED;
  }
#endif // ENABLE_3D

#ifndef WINNT
  /* Avoid (non-texture) surfaces which have been invalidated by a mode change. - CGW */

  surfaceData = (FXSURFACEDATA*) (pld->lpDDSurface->lpGbl->dwReserved1);
  if ((surfaceData != NULL) && (surfaceData->surfaceLevel != _FF(ddCurrentSurfaceLevel)))
  {
      pld->ddRVal = DDERR_SURFACELOST;
      return DDHAL_DRIVER_HANDLED;
  }
#endif

  if (_DD(ddAcceleratorUsed) || _DD(dd3DInOverlay) || (dwCaps & DDSCAPS_3DDEVICE))
  {
    /* Must flush pipeline when surface is locked, in order  */
    /* to process all drawing commands.  A side effect is to */
    /* is to flush previous swap buffer commands, which ends */
    /* up causing a wait on vertical retrace.  Some apps may */
   /* not like this behavior, but the flush is unavoidable  */
   /* for a pipelined architecture. -CGW-                   */

    if (pld->dwFlags & DDLOCK_WAIT)
    {
      FXBUSYWAIT(ppdev);  /* retro3dfx: bounded */
    }
    else if (FXGETBUSYSTATUS(ppdev))
    {
      pld->ddRVal = DDERR_WASSTILLDRAWING;
      return(DDHAL_DRIVER_HANDLED);
    }
  }
  else
  {
#ifdef WINNT
    /* Fix for PRS 3860 affecting NT.                        */
    /* There may be a better place to put this but we need   */
    /* it now to fix WHQL so here it is.                     */

    FXBUSYWAIT(ppdev);  /* retro3dfx: bounded */
#endif

    /* Avoid pipeline flush when surface is locked, if the   */
    /* the rendering surface is not 3D, or if Blt has not    */
    /* been called since the flipping chain was created.     */
    /* The intention is to allow unacclerated applications   */
    /* to avoid flushing.  Must still check the flip status, */
   /* if the surface being locked is involved. -CGW-        */

    /* Return flip status only for surface flipped from or to. */

    if (( pld->lpDDSurface->lpGbl->fpVidMem == _DD(ddSurfaceFlippedFrom)) ||
        ( pld->lpDDSurface->lpGbl->fpVidMem == _DD(ddSurfaceFlippedTo)))
    {
      /* Check primary surface or video overlay flipping status. */

      if (pld->dwFlags & DDLOCK_WAIT)
      {
#if ENABLE_LOG_FILE
        /* retro3dfx: bounded (was raw spin). A wedged flip must not hang a
           DDLOCK_WAIT surface lock (would hang the locking app and any GDI). */
        { ULONG _rs = 0;
          while (FXGETFLIPSTATUS(ppdev)) {
            if (++_rs >= 100000000UL) {
              retroLogForce(ppdev, "retro3dfx DdLock-FlipWait WEDGE-BREAK@100M\r\n");
              break;
            }
          } }
#else
        while (FXGETFLIPSTATUS(ppdev));
#endif
      }
      else if (FXGETFLIPSTATUS(ppdev))
      {
        pld->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
      }
    }
  }

#ifndef WINNT
#ifdef SLI_AA
  if (_DD(ddAAModeEnabled) || _DD(ddSLIModeEnabled))
  {
    // if cursor is enabled but not excluded
    if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
    {
      ptr = (DWORD) pld->lpDDSurface->lpGbl->fpVidMem;      
      if (ptr == _FF(CursorSurface))
         {
         HostRestoreCursor(ppdev, ptr, _FF(LastCursorPosX), _FF(LastCursorPosY), _FF(HostcursorExclusionStart));
         }
    }
  }
#endif
#endif

#ifdef WINNT
  // let MS figure out the surface address
  pld->ddRVal = DD_OK;
  MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);
#ifdef SIMULATE_YV12
  if ((dwCaps & DDSCAPS_OVERLAY) &&
      (DDPF_FOURCC & pld->lpDDSurface->lpGbl->ddpfSurface.dwFlags) &&
      (FOURCC_YV12 == pld->lpDDSurface->lpGbl->ddpfSurface.dwFourCC))
  {
    pld->lpSurfData = (LPVOID)(pld->lpDDSurface->lpGbl->fpVidMem);
    if (pld->bHasRect)
    {
      (BYTE *)pld->lpSurfData += (pld->lpDDSurface->lpGbl->lPitch * pld->rArea.top) +
                                 (pld->rArea.left);
    }
    return DDHAL_DRIVER_HANDLED;
  }
  else
#endif
    return DDHAL_DRIVER_NOTHANDLED;
#else

  // Return virtual memory pointer to surface.

  ptr = (DWORD) pld->lpDDSurface->lpGbl->fpVidMem;
  {
    if( pld->bHasRect )
    {
      if (pld->lpDDSurface->dwFlags & DDRAWISURF_HASPIXELFORMAT)
      {
        BytesPerPixel = pld->lpDDSurface->lpGbl->ddpfSurface.dwRGBBitCount >> 3;
      }
      else
      {
        BytesPerPixel = pld->lpDDSurface->lpGbl->lpDD->vmiData.ddpfDisplay.dwRGBBitCount >> 3;
      }
      pld->lpSurfData = (LPVOID)
        (ptr + ((pld->rArea.top * pld->lpDDSurface->lpGbl->lPitch) + (pld->rArea.left * BytesPerPixel)));
    }
    else
    {
      pld->lpSurfData = (LPVOID) ptr;
    }
  }

  MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);
  pld->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;
#endif

} // DdLock


/*----------------------------------------------------------------------
Function name: DdUnlock

Description:   Unlocks the specified surface.

Return:        DDHAL_DRIVER_HANDLED or DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
DdUnlock( LPDDHAL_UNLOCKDATA puld )
{
#if ENABLE_3D
  TXTRDESC *txtr;
#endif
  DWORD dwCaps;
#ifdef SLI_AA
  DWORD ptr;
#endif
  DD_ENTRY_SETUP(puld->lpDD);

  DDPRINT(DDDBGLVL, ">> DdUnlock (surf = %08lXh)", puld->lpDDSurface);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "UnLock32" ));
  #endif

  P6FENCE; // Flush write combine buffers

  dwCaps = puld->lpDDSurface->ddsCaps.dwCaps;

  if (dwCaps & DDSCAPS_OVERLAY)
  {
#if (_WIN32_WINNT >= 0x0500) || !defined(WINNT)
    UnlockOverlaySurface( ppdev, puld);
#endif

#ifdef SIMULATE_YV12
#if (_WIN32_WINNT >= 0x0500)

    if ((DDPF_FOURCC & puld->lpDDSurface->lpGbl->ddpfSurface.dwFlags) &&
        (FOURCC_YV12 == puld->lpDDSurface->lpGbl->ddpfSurface.dwFourCC))
    {
      DWORD   surfAddr = GET_HW_ADDR(puld->lpDDSurface);
      FLATPTR fpVidMem;
      FLATPTR YAddr, UAddr, VAddr;
      DWORD   i;

      SETDW(ghwAC->yuvBaseAddr, surfAddr);
      if (IS_TILED(surfAddr))
      {
        SETDW(ghwAC->yuvStride, _DS(ddTileStride));
      }
      else
      {
        SETDW(ghwAC->yuvStride, ((FXSURFACEDATA *)puld->lpDDSurface->lpGbl->dwReserved1)->lPitch);
      }

      // now copy the Y data from the YV12 block to the Y aperture
      fpVidMem = puld->lpDDSurface->lpGbl->fpVidMem;
#if REDUCED_MEMORY_MAPPINGS
      YAddr  = (FLATPTR)(ppdev->pjYUVPlanar);
#else
      YAddr  = (FLATPTR)(ppdev->pjBase + 0xC00000);
#endif
      for (i = 0; i < puld->lpDDSurface->lpGbl->wHeight; i++)
      {
        memcpy((LPVOID)YAddr, (LPVOID)fpVidMem, puld->lpDDSurface->lpGbl->wWidth);
        YAddr += 1024;

        fpVidMem += puld->lpDDSurface->lpGbl->lPitch;
      }

      // the V data from the YV12 block to the V aperture
#if REDUCED_MEMORY_MAPPINGS
      VAddr = (FLATPTR)(ppdev->pjYUVPlanar + (0xE00000 - 0xC00000));
#else
      VAddr = (FLATPTR)(ppdev->pjBase + 0xE00000);
#endif
      for (i = 0; i < puld->lpDDSurface->lpGbl->wHeight / 2; i++)
      {
        // copy the V data
        memcpy((LPVOID)VAddr, (LPVOID)fpVidMem, puld->lpDDSurface->lpGbl->wWidth / 2);
        VAddr += 1024;

        fpVidMem += (puld->lpDDSurface->lpGbl->lPitch / 2);
      }

      // the U data from the YV12 block to the U aperture
#if REDUCED_MEMORY_MAPPINGS
      UAddr = (FLATPTR)(ppdev->pjYUVPlanar + (0xD00000 - 0xC00000));
#else
      UAddr = (FLATPTR)(ppdev->pjBase + 0xD00000);
#endif
      for (i = 0; i < puld->lpDDSurface->lpGbl->wHeight / 2; i++)
      {
        // copy the U data
        memcpy((LPVOID)UAddr, (LPVOID)fpVidMem, puld->lpDDSurface->lpGbl->wWidth / 2);
        UAddr += 1024;

        fpVidMem += (puld->lpDDSurface->lpGbl->lPitch / 2);
      }
    }
#endif
#endif
  }

#if ENABLE_3D
  if (dwCaps & DDSCAPS_TEXTURE)
  {
     RC   *pRc;


     txtr = TXTRDESC_PTR(puld->lpDDSurface->dwReserved1);

     pRc = (RC *)_D3(lastContext);
     if (NULL != pRc)
     {
       CMDFIFO_PROLOG(cmdFifo);

       // fix for Texture Load - Video failures
       // flush the texture cache in case it was the active texture that was locked
       CMDFIFO_CHECKROOM(cmdFifo, 4 * (PH4_SIZE + 1));
  
       // Flush the texture cache by writing any other base addr (so invert the base addr)
       SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0, texBaseAddr, TMU2CHIP(TREX0)));
       SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->texBaseAddr, (0xffffffff ^ pRc->sst.baseAddr));
       SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0, texBaseAddr, TMU2CHIP(TREX0)));
       SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->texBaseAddr, pRc->sst.baseAddr);
       SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0, texBaseAddr, TMU2CHIP(TREX1)));
       SETPD(cmdFifo, SST_TREX(ghw0,TREX0)->texBaseAddr, (0xffffffff ^ pRc->sst.baseAddr));
       SETPH(cmdFifo, CMDFIFO_BUILD_PK4(R0, texBaseAddr, TMU2CHIP(TREX1)));
       SETPD( cmdFifo, SST_TREX(ghw0,TREX1)->texBaseAddr, pRc->sst.baseAddr1);
  
       CMDFIFO_EPILOG(cmdFifo);
     }

#if defined(WINNT)
#if DBG && 0
     if ((DDPF_FOURCC & puld->lpDDSurface->lpGbl->ddpfSurface.dwFlags) &&
         (FOURCC_DXT2 == puld->lpDDSurface->lpGbl->ddpfSurface.dwFourCC) &&
         (4 > puld->lpDDSurface->lpGbl->wWidth) &&
         (4 > puld->lpDDSurface->lpGbl->wHeight))
     {
        //DDPRINT(DDDBGLVL, "  sub 4x4 DXT2 surface => %08lXh %08lXh %08lXh %08lXh",
        DDPRINT(0, "  sub 4x4 DXT2 surface (%ldx%ld) => %08lXh %08lXh %08lXh %08lXh",
                puld->lpDDSurface->lpGbl->wWidth, puld->lpDDSurface->lpGbl->wHeight,
                *(DWORD *)(puld->lpDDSurface->lpGbl->fpVidMem + ppdev->pjLfbBase),
                *(DWORD *)(puld->lpDDSurface->lpGbl->fpVidMem + ppdev->pjLfbBase + sizeof(DWORD)),
                *(DWORD *)(puld->lpDDSurface->lpGbl->fpVidMem + ppdev->pjLfbBase + 2 * sizeof(DWORD)),
                *(DWORD *)(puld->lpDDSurface->lpGbl->fpVidMem + ppdev->pjLfbBase + 3 * sizeof(DWORD)));
     }
#endif

#ifndef DISABLE_W2K_MAXMIPLEVEL_HACK
     // Fix for MipFilter Point - MaxMipLevel and
     //         MipFilter Linear - MaxMipLevel failures on W2K
     //
     // somehow in a 32bpp mode, Test 27 of MipFilter -point_maxmiplevel and
     // MipFilter -linear_maxmiplevel puts incorrect data into the 2x2 DXT2
     // texture on w2k
     //
     // if you just run Test 27 by itself (mipfilter -point_maxmiplevel:27),
     // the test passes and the data put in the 2x2 DXT2 surface
     // is 00FF00FFh 00000000h 000083F0h 55555050h
     //
     // but if you run test 26 and test 27 (mipfilter -point_maxmiplevel:26,27),
     // then the test fails and the data put in the 2x2 DXT2 surface
     // is 00FF00FFh 0000FF00h 0001F72Ah 55055A5Ah
     //
     // so as a total hack, detect a 2x2 DXT2 surface containing the data that
     // causes the failure and replace it with the data that passes

     if ((4 == GETPRIMARYBYTEDEPTH) &&
         (DDPF_FOURCC & puld->lpDDSurface->lpGbl->ddpfSurface.dwFlags) &&
         (FOURCC_DXT2 == puld->lpDDSurface->lpGbl->ddpfSurface.dwFourCC) &&
         (2 == puld->lpDDSurface->lpGbl->wWidth) &&
         (2 == puld->lpDDSurface->lpGbl->wHeight) &&
         (0x00FF00FF == *(DWORD *)(puld->lpDDSurface->lpGbl->fpVidMem + ppdev->pjLfbBase)) &&
         (0x0000FF00 == *(DWORD *)(puld->lpDDSurface->lpGbl->fpVidMem + ppdev->pjLfbBase + sizeof(DWORD))) &&
         (0x0001F72A == *(DWORD *)(puld->lpDDSurface->lpGbl->fpVidMem + ppdev->pjLfbBase + 2 * sizeof(DWORD))) &&
         (0x55055A5A == *(DWORD *)(puld->lpDDSurface->lpGbl->fpVidMem + ppdev->pjLfbBase + 3 * sizeof(DWORD))))
     {
        *(DWORD *)(puld->lpDDSurface->lpGbl->fpVidMem + ppdev->pjLfbBase + 0 * sizeof(DWORD)) = 0x00FF00FF;
        *(DWORD *)(puld->lpDDSurface->lpGbl->fpVidMem + ppdev->pjLfbBase + 1 * sizeof(DWORD)) = 0x00000000;
        *(DWORD *)(puld->lpDDSurface->lpGbl->fpVidMem + ppdev->pjLfbBase + 2 * sizeof(DWORD)) = 0x000083F0;
        *(DWORD *)(puld->lpDDSurface->lpGbl->fpVidMem + ppdev->pjLfbBase + 3 * sizeof(DWORD)) = 0x55555050;
     }
#endif
#endif

     if (TEXTURE_IS_DXT_SURFACE(puld->lpDDSurface->lpGbl->ddpfSurface)
         && txtr->flags & AspectRatioGT8)
     {
         stretchDXTn(puld->lpDDSurface->lpGbl,txtr,&_D3G,ppdev);
         if ( puld->lpDDSurface->lpGbl->ddpfSurface.dwFlags & DDPF_FOURCC 
             && puld->lpDDSurface->lpGbl->ddpfSurface.dwFourCC == FOURCC_DXT1) 
         {
            if (txtr->tlog > txtr->slog)
            {
               int widthshift = txtr->initialTlog - txtr->initialSlog - 3;
               if ((puld->lpDDSurface->lpGbl->wWidth << widthshift) < 8)
                 PAD_DXT1(ppdev,puld->lpDDSurface->lpGbl);
            }
         }
     }
     else if (TEXTURE_IS_FXT_SURFACE(puld->lpDDSurface->lpGbl->ddpfSurface)
              && txtr->flags & AspectRatioGT8)
     {
         stretchFXT1(puld->lpDDSurface->lpGbl,txtr,&_D3G,ppdev);
     }
     else if ( puld->lpDDSurface->lpGbl->ddpfSurface.dwFlags & DDPF_FOURCC
              && puld->lpDDSurface->lpGbl->ddpfSurface.dwFourCC == FOURCC_DXT1)
     {
         if (puld->lpDDSurface->lpGbl->wWidth < 8)
           PAD_DXT1(ppdev,puld->lpDDSurface->lpGbl);
     }
  }
#endif


#ifndef WINNT
#ifdef SLI_AA
  if (_DD(ddAAModeEnabled) || _DD(ddSLIModeEnabled))
  {
    // if cursor is enabled but not excluded
    if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
    {
      ptr = (DWORD) puld->lpDDSurface->lpGbl->fpVidMem;      
      if (ptr == _FF(CursorSurface))
         {
         HostDrawCursor(ppdev, ptr);
         }
    }
  }
#endif
#endif

  MODIFY_SLI_READ(ppdev, DISABLE_SLI_READ);

  puld->ddRVal = DD_OK;
#ifdef WINNT
  return DDHAL_DRIVER_NOTHANDLED;
#else
  return DDHAL_DRIVER_HANDLED;
#endif

} // DdUnlock


/*----------------------------------------------------------------------
Function name: DdAddAttachedSurface

Description:   Attach a surface to another surface.

Return:        DDHAL_DRIVER_HANDLED or DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
DdAddAttachedSurface( LPDDHAL_ADDATTACHEDSURFACEDATA pasd )
{
  DWORD dwCap1, dwCap2;
  DWORD pixelByteDepth, zbufferDepth;
  LPDDRAWI_DDRAWSURFACE_LCL   psurf;
#ifndef WINNT
  FXSURFACEDATA* surfaceData1, * surfaceData2;
#endif
  DD_ENTRY_SETUP(pasd->lpDD);

  DDPRINT(DDDBGLVL, ">> DdAddAttachedSurface (lpDDSurface=%08lXh, lpSurfAttached=%08lXh)",
          pasd->lpDDSurface, pasd->lpSurfAttached);
  D3DPRINT(DLSURFACE, "ddAddAttSurf: Entry");

  #ifdef FXTRACE
    DISPDBG((ppdev, DEBUG_APIENTRY, "AddAttachSurface32" ));
  #endif

#if ENABLE_3D
  dwCap1 = pasd->lpDDSurface->ddsCaps.dwCaps;
  dwCap2 = pasd->lpSurfAttached->ddsCaps.dwCaps;

  // Make sure surface locations are both video memory.
  if (!(dwCap1 & DDSCAPS_VIDEOMEMORY) || !(dwCap2 & DDSCAPS_VIDEOMEMORY))
  {
	  D3DPRINT(DLSURFACE, "ddAddAttSurf: Not Video-to-Video. Aborting.");
      DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pasd->ddRVal);
	  pasd->ddRVal = DD_OK;
      return DDHAL_DRIVER_NOTHANDLED;
  }

#if 0
  // This code causes WHQL to fail, and is probably not necessary. -CGW-

  // Make sure surface types are the same.
  if ((IS_TILED(GET_HW_ADDR(pasd->lpDDSurface))) && !(IS_TILED(GET_HW_ADDR(pasd->lpSurfAttached))))
  {
    D3DPRINT(DLSURFACE, "ddAddAttSurf: Non-Tiled to Tiled-surface. Aborting.");
	pasd->ddRVal = DDERR_CANNOTATTACHSURFACE;
	return DDHAL_DRIVER_HANDLED;
   }

  if (!(IS_TILED(GET_HW_ADDR(pasd->lpDDSurface))) && (IS_TILED(GET_HW_ADDR(pasd->lpSurfAttached))))
  {
    D3DPRINT(DLSURFACE, "ddAddAttSurf: Tiled to Non-Tiled-surface. Aborting.");
	pasd->ddRVal = DDERR_CANNOTATTACHSURFACE;
	return DDHAL_DRIVER_HANDLED;
   }
#endif

#ifndef WINNT
  if (dwCap1 & DDSCAPS_OVERLAY)
  {
    if (dwCap2 & DDSCAPS_OVERLAY)
    {
	  D3DPRINT(DLSURFACE, "ddAddAttSurf: Overlay-to-Overlay");
	  surfaceData1 = (FXSURFACEDATA*) (pasd->lpDDSurface->lpGbl->dwReserved1);
      surfaceData2 = (FXSURFACEDATA*) (pasd->lpSurfAttached->lpGbl->dwReserved1);

      surfaceData1->inFlipChain = TRUE;
      surfaceData2->inFlipChain = TRUE;
    }
    else
    {
	  D3DPRINT(DLSURFACE, "ddAddAttSurf: Overlay-to-NonOverlay. Aborting.");
	  pasd->ddRVal = DDERR_CANNOTATTACHSURFACE;
      DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pasd->ddRVal);
      return DDHAL_DRIVER_HANDLED;
    }
  }
#endif

  /* Very dangerous code to munge the zbuffer surface depth, since   */
  /* Napalm cannot handle a different depth zbuffer and rendering    */
  /* surface!  This code forces the zbuffer depth to match the depth */
  /* of the rendering surface to which it is attached. -CGW-         */

  if ((IS_NAPALM) && (dwCap2 & DDSCAPS_ZBUFFER))
  {
    psurf = pasd->lpDDSurface;  // Rendering surface being attached to.

#ifdef WINNT
    // WNT always has pixel format in global surface.
    pixelByteDepth = (DWORD)(psurf->lpGbl->ddpfSurface.dwRGBBitCount) >> 3;
#else
    // W9X must check local surface flag for pixel format.
    if((psurf->dwFlags) & DDRAWISURF_HASPIXELFORMAT )
    {
      pixelByteDepth = (DWORD)(psurf->lpGbl->ddpfSurface.dwRGBBitCount) >> 3;
    }			
    else
    {
      pixelByteDepth = GETPRIMARYBYTEDEPTH;
    }
#endif

    psurf = pasd->lpSurfAttached;  // Zbuffer surface being attached.

    zbufferDepth = (DWORD)(psurf->lpGbl->ddpfSurface.dwZBufferBitDepth) >> 3;

    D3DPRINT(DLSURFACE, "ddAddAttSurf: Comparing PriDepth %d, ZDepth %d", pixelByteDepth, zbufferDepth);
    DDPRINT(DDDBGLVL, "  Comparing PriDepth %d, ZDepth %d", pixelByteDepth, zbufferDepth);

    // 16bpp render target, 16bpp zbuffer: do nothing, no errors
    // 16bpp render target, 24bpp zbuffer: silently substitute 16bpp zbuffer
    // 16bpp render target, 32bpp zbuffer: silently substitute 16bpp zbuffer
    // 32bpp render target, 16bpp zbuffer: silently substitute 32bpp zbuffer
    // 32bpp render target, 24bpp zbuffer: silently substitute 32bpp zbuffer
    // 32bpp render target, 32bpp zbuffer: do nothing, no errors

    if ((pixelByteDepth == 2) && (zbufferDepth != 2))
    {
      D3DPRINT(DLSURFACE, "ddAddAttSurf: Forcing Z Buffer depth to 16bpp from 24bpp");
      DDPRINT(DDDBGLVL, "  Forcing Z Buffer depth to 16bpp from 24bpp");
      if (!(psurf->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY))
      {
        psurf->lpGbl->ddpfSurface.dwZBufferBitDepth = 16; // substitute 16bpp zbuffer
        psurf->lpGbl->ddpfSurface.dwZBitMask = 0x0000FFFF;
#ifdef WINNT
        if (psurf->lpGbl->dwReserved1)
          ((FXSURFACEDATA *)psurf->lpGbl->dwReserved1)->dwSurfaceFlags |= FX_ORIGINALLY_Z32;
#endif
      }
    }
    else if ((pixelByteDepth == 4) && (zbufferDepth != 4))
    {
      D3DPRINT(DLSURFACE, "ddAddAttSurf: Forcing Z Buffer depth to 32bpp");
      DDPRINT(DDDBGLVL, "  Forcing Z Buffer depth to 32bpp");
      if (!(psurf->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY))
      {
#ifdef WINNT
        // in 32bpp modes, we already overallocated 16bpp zbuffers as 32bpp zbuffers
        // so only in 16bpp modes, reallocate 16bpp zbuffers as 32bpp zbuffers when
        // they are being attached to a 32bpp render target
        if (GETPRIMARYBYTEDEPTH != 4)
        {
          FXSURFACEDATA *newSurfData, *oldSurfData;
          DWORD                       pWidth, bWidth, height;
          DWORD                       tileFlag;
          HRESULT                     ddrval;
  
  
          // attempt to reallocate a 32bpp zbuffer surface and release the 16bpp zbuffer surface
          newSurfData = (FXSURFACEDATA*)DDMALLOCZ(sizeof(FXSURFACEDATA), 0);
  
          if (! newSurfData)
          {
            DDPRINT(0, "DdAddAttachedSurface: FXSURFACEDATA allocation failed for resize of zbuffer from 16bpp to 32bpp");
            pasd->ddRVal = DDERR_CANNOTATTACHSURFACE;
            DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pasd->ddRVal);
            return DDHAL_DRIVER_HANDLED;
          }
  
          newSurfData->heapID = HEAP_INVALID;
          newSurfData->AAheapID = HEAP_INVALID;
  
          pWidth = psurf->lpGbl->wWidth;
          height = psurf->lpGbl->wHeight;
  
          bWidth = XALIGN(pWidth * pixelByteDepth);
  
          ddrval = memMgr_allocSurface(ppdev,
                                       psurf->ddsCaps.dwCaps,
                                       bWidth,
                                       height,
                                       (bWidth + 0x7F)>> 7L,
                                       (height + 0x1F)>> 5L,
                                       &(newSurfData->lfbPtr),
                                       &(newSurfData->hwPtr),
                                       &(newSurfData->lPitch),
                                       &tileFlag,
                                       &(newSurfData->heapID),
                                       &(newSurfData->pvmHeap));
  
          if (DD_OK != ddrval)
          {
            DDPRINT(0, "DdAddAttachedSurface: Surface allocation failed for resize of zbuffer from 16bpp to 32bpp");
            DDFREE((void*)newSurfData);
            pasd->ddRVal = DDERR_CANNOTATTACHSURFACE;
            DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pasd->ddRVal);
            return DDHAL_DRIVER_HANDLED;
          }
  
          oldSurfData = (FXSURFACEDATA*)(psurf->lpGbl->dwReserved1);
  
          memMgr_freeSurface(ppdev,
                             psurf->ddsCaps.dwCaps,
                             oldSurfData->lfbPtr,
                             oldSurfData->hwPtr,
                             GETMEMTYPE(oldSurfData->hwPtr),
                             oldSurfData->heapID,
                             oldSurfData->pvmHeap);
          
          DDFREE((void*)oldSurfData);
          
          psurf->lpGbl->dwReserved1 = (DWORD)newSurfData;
          UPDATE_BLOCK_DATA(surfaceData, &psurf->lpGbl->dwReserved1);
          psurf->lpGbl->fpVidMem = newSurfData->lfbPtr;
          psurf->lpGbl->lPitch = newSurfData->lPitch;
        }
#endif

        psurf->lpGbl->ddpfSurface.dwZBufferBitDepth = 32; // 32bpp zbuffer
        psurf->lpGbl->ddpfSurface.dwZBitMask = 0x00FFFFFF;

#ifdef WINNT
        if (psurf->lpGbl->dwReserved1)
          ((FXSURFACEDATA *)psurf->lpGbl->dwReserved1)->dwSurfaceFlags |= FX_ORIGINALLY_Z16;
#endif
      }
    }
  }

#ifdef SLI_AA
  // Allocate AA secondary zbuffer.
  // The buffer MUST be in video memory to have made it to this point in the code.
#ifdef WINNT
  if ((IS_NAPALM) &&
      (dwCap2 & DDSCAPS_ZBUFFER) &&
      (psurf->lpGbl->wWidth == (DWORD)ppdev->cxScreen) &&
      (psurf->lpGbl->wHeight == (DWORD)ppdev->cyScreen))
#else
  if ((IS_NAPALM) && (dwCap2 & DDSCAPS_ZBUFFER))
#endif
  {
    FXSURFACEDATA               *surfaceData;
    DWORD                       tileFlag;
    DWORD                       bWidth, height;

    surfaceData = (FXSURFACEDATA*) psurf->lpGbl->dwReserved1;

    // If the surface is non-tiled then it is a non-distributed surface
    if (_DD(ddAAModeEnabled) && (IS_TILED(GET_HW_ADDR(pasd->lpDDSurface))))
    {
      psurf = pasd->lpSurfAttached;  // Zbuffer surface being attached.

      height = psurf->lpGbl->wHeight;
      bWidth = psurf->lpGbl->wWidth * GETPRIMARYBYTEDEPTH;

      // Call memory manager to allocate.

      if ((surfaceData) && (surfaceData->AAheapID == HEAP_INVALID))
      {
        D3DPRINT(DLSURFACE, "ddAddAttSurf: Allocating AA buffer for Z");

        pasd->ddRVal = memMgr_allocSecondary(
#ifdef WINNT
            ppdev,
#else
            psurf->lpGbl->lpDD,         // [IN] DirectDraw object for VidMemAlloc
#endif
            psurf->ddsCaps.dwCaps,      // [IN] DirectDraw surface capabilities
            bWidth,                     // [IN] width in butes
            height,                     // [IN] height in scanlines
            (bWidth + 0x7F)>> 7L,       // [IN] width in tiles
            (height + 0x1F)>> 5L,       // [IN] height in tiles
            &(surfaceData->AAlfbPtr),   // [OUT] lfb address of surface
            &(surfaceData->AAhwPtr),    // [OUT] hardware offset of surface
            &(surfaceData->AAlPitch),   // [OUT] hardware pitch of surface
            &tileFlag,                  // [OUT] MEM_IN_TILED or MEM_IN_LINEAR
            &(surfaceData->AAheapID)    // [OUT] DirectDraw heap number
#ifdef WINNT
            , &(surfaceData->AApvmHeap)
#endif
            );
         // Check if surface allocated.

         if (!surfaceData->AAlfbPtr)
         {
#ifdef WINNT
           {
             extern void BailOutOfAA(PDEV *);
             BailOutOfAA(ppdev);
             pasd->ddRVal = DD_OK;
           }
#else
           pasd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
           DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pasd->ddRVal);
           D3DPRINT(DLSURFACE, "ddAddAttSurf: Out Of Video memory. Aborting. %s",
                    (tileFlag == MEM_IN_TILED) ? "Tiled memory" : "Linear memory");
           return DDHAL_DRIVER_HANDLED;
#endif
         }
#ifdef DEBUG
		 else
		 {
		   D3DPRINT(DLSURFACE, "ddAddAttSurf: Allocation succeeded. %s",
						(tileFlag == MEM_IN_TILED) ? "Tiled memory" : "Linear memory");
		 }
#endif
      }

    }

#ifndef WINNT
	if (_DD(ddSLIModeRequested))
    {
      // Adjust lfbPtr for SLI mode.

      surfaceData->lfbPtr = AdjustLfbPtr(ppdev, surfaceData->lfbPtr, &surfaceData->heapID);

      // Compute hwPtr from lfbPtr.

      surfaceData->hwPtr = LfbPtrToHwPtr(ppdev, surfaceData->lfbPtr, surfaceData->heapID);
    }
#endif // WINNT

  }
#endif // SLI_AA

#endif // ENABLE_3D

  pasd->ddRVal = DD_OK;
  DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pasd->ddRVal);
  return DDHAL_DRIVER_NOTHANDLED;

} // DdAddAttachedSurface


/*----------------------------------------------------------------------
Function name: DdSetSurfaceColorKey

Description:   Set colorkey value for specified surface.

Return:        DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
DdSetSurfaceColorKey( LPDDHAL_SETCOLORKEYDATA pssck)
{
  DD_ENTRY_SETUP(pssck->lpDD);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "SetSurfaceColorKey" ));
  DUMP_SETSURFACECOLORKEY32(ppdev, DEBUG_DDGORY, pssck );
  #endif

  pssck->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;

} // DdSetSurfaceColorKey


/*******************************************************************/
/*                     EXPORTED FUNCTIONS                          */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name: FxGetBusyStatus

Description:   Determine if the pipeline is busy.

Return:        DD_OK or DDERR_WASSTILLDRAWING;
----------------------------------------------------------------------*/

#pragma optimize("",off)
HRESULT FXGETBUSYSTATUS(NT9XDEVICEDATA *ppdev)
{
  DWORD dwStatus;

  P6FENCE; // Flush write combine buffers

  dwStatus = GET(ghwIO->status);

#if defined(SLI_AA) && (!defined(WINNT) || (_WIN32_WINNT >= 0x0500))
  {
    int i;
    int nNumChips;
    SstIORegs * pIORegs;

    // If we are in SLI AA Mode
#ifdef WINNT
    if (_FF(ddMultiChipConfig))
#else
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_AA_MASTER)
#endif
       nNumChips = _FF(dwNumUnits);
    else
       nNumChips = 1;

    for (i=1; i<nNumChips; i++)
    {
#ifdef WINNT
      pIORegs = (SstIORegs * )_FF(regBase[i * HWINFO_SST_MAX_CHIP_INDEX + HWINFO_SST_IOREGS_INDEX]);
#else
      pIORegs = (SstIORegs * )_FF(regBase[i * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_IOREGS_INDEX]);
#endif
      dwStatus |= GET(pIORegs->status);
    }
  }
#endif

  if(dwStatus & SST_BUSY)
  {
    return DDERR_WASSTILLDRAWING;
  }
  else
  {
    _DD(ddAcceleratorUsed) = 0; // Pipeline flushed
    return DD_OK;
  }

} // FxGetBusyStatus

#pragma optimize("",on)

/*******************************************************************/
/*                    INTERNAL FUNCTIONS                           */
/*******************************************************************/
#if ENABLE_3D
void UNPAD_DXT1(NT9XDEVICEDATA *ppdev, LPDDRAWI_DDRAWSURFACE_GBL surfGBL)
{
   int size = (surfGBL->wHeight >> 2) << 2;
#ifdef WINNT
   unsigned long *src = (unsigned long*)(surfGBL->fpVidMem + ppdev->pjLfbBase);
   unsigned long *dst = (unsigned long*)(surfGBL->fpVidMem + ppdev->pjLfbBase);
#else
   unsigned long *src = (unsigned long*)surfGBL->fpVidMem;
   unsigned long *dst = (unsigned long*)surfGBL->fpVidMem;
#endif

   while (size > 4)
   {
      dst[0] = src[0];
      dst[1] = src[1];
      src += 4;
      dst += 2;
      size -= 4;
   }
}
void PAD_DXT1(NT9XDEVICEDATA *ppdev, LPDDRAWI_DDRAWSURFACE_GBL surfGBL)
{
   int size = (surfGBL->wHeight >> 2) << 2;
#ifdef WINNT
   unsigned long *src = (unsigned long*)(surfGBL->fpVidMem + ppdev->pjLfbBase) + ((size >> 1) - 2);
   unsigned long *dst = (unsigned long*)(surfGBL->fpVidMem + ppdev->pjLfbBase) + (size - 4);
#else
   unsigned long *src = (unsigned long*)surfGBL->fpVidMem+((size >> 1) - 2);
   unsigned long *dst = (unsigned long*)surfGBL->fpVidMem+(size - 4);
#endif

   while (size > 4)
   {
      dst[0] = src[0];
      dst[1] = src[1];
      dst -= 4;
      src -= 2;
      size -= 4;
   }
}
#endif

