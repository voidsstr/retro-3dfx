/* $Header: ddfxs32.c, 66, 10/11/00 8:52:01 PM, Brent$ */
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
** File Name: 	DDFXS32.C
**
** $Revision: 66$
** $Date: 10/11/00 8:52:01 PM$
**
*/

/*******************************************************************************
*
* DIRECTDRAW FUNCTIONS:
*
* DdDestroyDriver           --- Destroy the DirectDraw driver.
* DdSetExclusiveMode        --- Enter or leave DirectDraw exclusive mode.
* DdSetDrvColorKey          --- Sets color key value for specified surface.
* DdSetPalette              --- Attach palette to the specific surface.
* DdCreatePalette           --- Creates the specified DirectDraw palette.
* DdDestroyPalette          --- Destroys the specified DirectDraw palette.
* DdSetEntries              --- Changes entries in the specified palette.
*
* EXPORTED FUNCTIONS:
*
* Enter_3DApplication       --- Entering a 3D, exclusive, fullscreen application.
* Exit_3DApplication        --- Exiting a 3D, exclusive, fullscreen application.
* Promote_PrimaryToOverlay  --- Switch primary from desktop to overlay.
* Demote_PrimaryFromOverlay --- Switch primary from overlay to desktop.
* Promote_DeviceToSLIAA     --- Enable SLI or AA mode.
* Demote_DeviceFromSLIAA    --- Disable SLI or AA mode.
* Compute_SLIAA_Config      --- Determine SLI and AA configuration.
* MoveTileMark              --- Move tile mark to provide extra memory in SLI mode.
* TwoPpcInit                --- Function to init log2 Two Pixel Per Clock Band Height
*
* INTERNAL FUNCTIONS:
*
* LoadGamma                 --- Load gamma ramp for Direct3D.
* RestoreGamma              --- Restore gamma ramp for desktop.
* SetPalette                --- Write hardware palette.
* Exponential               --- Exponential function.
* Logarithm                 --- Logarithm function.
* RetrieveFromRegistry      --- Read value from registry.
* SwitchToHostCursor        --- Switch to using software cursor.
* GetProcessFileName        --- Function to get and Encode File Name 
* IsBadApp                  --- Is Application in Bad App List?
*
*******************************************************************************/

#include "precomp.h"
#include "regkeys.h"

#if defined(SLI_AA)
#include <ddsli2d.h>
#endif

#ifdef RD_ABORT_ERROR
#pragma message("...Read Abort enabled. Only for A0!...")
#else
#pragma message("....Read Abort Disabled....")
#endif

/*
 * NOTE:  All routines are called with the Win16 lock taken.   This is
 * to prevent anyone from calling the display driver and doing something
 * that could confict with this 32-bit driver.  This means that shared
 * 16-32 memory is safe to use at any time inside either driver.
 */

/*******************************************************************/
/*                     DIRECTDRAW FUNCTIONS                        */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name: DdDestroyDriver

Description:   Destroy the DirectDraw driver.

Return:        DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall DdDestroyDriver( LPDDHAL_DESTROYDRIVERDATA pcsd )
{
  DD_ENTRY_SETUP(pcsd->lpDD);

  return DDHAL_DRIVER_HANDLED;

} // DdDestroyDriver


/*----------------------------------------------------------------------
Function name: DdSetExclusiveMode

Description:   Enter or leave DirectDraw exclusive mode.

Return:        DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall DdSetExclusiveMode( LPDDHAL_SETEXCLUSIVEMODEDATA psemd )
{
    DD_ENTRY_SETUP(psemd->lpDD);

    if( psemd->dwEnterExcl )
    {
      #ifdef FXTRACE
      Msg(ppdev, DEBUG_APIENTRY, "SetExclusiveMode32 (entering)" );
      #endif
      _DS(ddExclusiveMode) = TRUE;
    }
    else
    {
      #ifdef FXTRACE
      Msg(ppdev, DEBUG_APIENTRY, "SetExclusiveMode32 (leaving)" );
      #endif

	  /* In case we reached this point and the primary surface has not been demoted from */
	  /* overlay, we need to call Exit_3DApplication() in order to the neccessary demotions. */

	  if (_DD(dd3DInOverlay))
	      Exit_3DApplication(ppdev);

      _DS(ddExclusiveMode) = FALSE;
    }

    psemd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;

} // DdSetExclusiveMode


/*----------------------------------------------------------------------
Function name: DdSetDrvColorKey

Description:   Sets color key value for specified surface.

Return:        DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/
DWORD __stdcall DdSetDrvColorKey( LPDDHAL_DRVSETCOLORKEYDATA psdck )
{
  psdck->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;  // Runtime will set colorkey?

} //DdSetDrvColorKey


/*----------------------------------------------------------------------
Function name: DdSetPalette

Description:   Attach palette to the specific surface.

Return:        DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall DdSetPalette( LPDDHAL_SETPALETTEDATA psp )
{
  DD_ENTRY_SETUP(psp->lpDD);

  #ifdef FXTRACE
  Msg(ppdev, DEBUG_APIENTRY, "SetPalette32" );
  #endif

  // Palette is being attached, probably to a texture surface,
  // so notify Direct3D of new palette.
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  TXTRNEWPALETTE(ppdev, (PALHNDL *) psp->lpDDPalette);
#else
  TXTRNEWPALETTE(ppdev, psp->lpDDPalette);
#endif

  psp->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} // DdSetPalette


/*----------------------------------------------------------------------
Function name: DdCreatePalette

Description:   Creates the specified DirectDraw palette.

Return:        DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall DdCreatePalette( LPDDHAL_CREATEPALETTEDATA pcp )
{
  DD_ENTRY_SETUP(pcp->lpDD);

  #ifdef FXTRACE
  Msg(ppdev, DEBUG_APIENTRY, "CreatePalette32" );
  DUMP_CREATEPALETTEDATA(ppdev, DEBUG_DDGORY, pcp );
  #endif

  pcp->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} // DdCreatePalette


/*----------------------------------------------------------------------
Function name: DdDestroyPalette

Description:   Destroys the specified DirectDraw palette.

Return:        DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall DdDestroyPalette( LPDDHAL_DESTROYPALETTEDATA pdp )
{
  DD_ENTRY_SETUP(pdp->lpDD);

  #ifdef FXTRACE
  Msg(ppdev, DEBUG_APIENTRY, "DestroyPalette32" );
  #endif

  // Palette is being destroyed, possibly attached to a texture surface,
  // so notify Direct3D of changed palette.
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  TXTRCHANGEDPALETTE(ppdev, (PALHNDL *) pdp->lpDDPalette);
#else
  TXTRCHANGEDPALETTE(ppdev, pdp->lpDDPalette);
#endif

  pdp->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} // DdDestroyPalette


/*----------------------------------------------------------------------
Function name: DdSetEntries

Description:   Changes entries in the specified palette.

Return:        DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall DdSetEntries( LPDDHAL_SETENTRIESDATA pse )
{
  DD_ENTRY_SETUP(pse->lpDD);

  #ifdef FXTRACE
  Msg(ppdev, DEBUG_APIENTRY, "SetEntries32" );
  #endif

  // Palette is being changed, possibly attached to a texture surface,
  // so notify Direct3D of changed palette.
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
  TXTRCHANGEDPALETTE(ppdev, (PALHNDL *) pse->lpDDPalette);
#else
  TXTRCHANGEDPALETTE(ppdev, pse->lpDDPalette);
#endif

  pse->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} // DdSetEntries


/*******************************************************************/
/*                     EXPORTED FUNCTIONS                          */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name: Enter_3DApplication

Description:   Entering a 3D, exclusive-mode, full-screen application.
               
Return:        DD_OK
----------------------------------------------------------------------*/
DWORD Enter_3DApplication(NT9XDEVICEDATA * ppdev)
{
  WORD * pFlags = (WORD *)_FF(lpDeFlags);
  WORD SaveBusy = *pFlags;

  // Set busy flag.

  Set_Busy(pFlags);

  // Check power mode.

  if (LOW_POWER_MODE(SaveBusy))
  {
    return DD_OK;
  }

  // Make note that we have created the 3D flipping chain
  _DD(dwFlags) |= DDGLOBAL_3DCHAINON;

  /* Load gamma for Direct3D. */

  LOADGAMMA(ppdev);

  /* Disable video display. */

  if (IS_NAPALM)
  {
	DWORD vidProcCfg = GET(ghwIO->vidProcCfg);
	vidProcCfg &= ~SST_VIDEO_PROCESSOR_EN;
	SETDW(ghwIO->vidProcCfg, vidProcCfg);
  }

  /* Enable SLI if multiple chips are present, and DISABLE_SLI is not  */
  /* set in the registry.  Enable AA if the application requests by    */
  /* setting the anti-aliasing hint during surface creation, or if     */
  /* ENABLE_AA is set in the registry.  In both cases, AA will only be */
  /* enabled if enough buffers can be allocated.  -CGW-                */
  
#ifdef SLI_AA
  if (IS_NAPALM)
    Promote_DeviceToSLIAA(ppdev);
#endif // SLI_AA

  /* Napalm needs to clear the secondary AA buffers, when enabling AA  */
  /* rendering, and the primary surface when enabling SLI mode. -CGW-  */

#ifdef SLI_AA
  if (IS_NAPALM)
    Clear_SLIAA_Buffers (ppdev);
#endif // SLI_AA

  /* Voodoo3/Napalm have separate mechanisms for flipping the desktop  */
  /* and video overlay surfaces.  Desktop flipping can't be pipelined, */
  /* so the video overlay is borrowed to display the primary flipping  */
  /* chain for 3D, exclusive-mode, full-screen applications.  -CGW-    */

#ifdef ENABLE_TILED_HEAP
  Promote_PrimaryToOverlay(ppdev);
#endif // ENABLE_TILED_HEAP

  /* Voodoo3/Napalm promote to using the AGP command FIFO, to improve  */
  /* the performance of 3D, exclusive-mode, full-screen applications.  */
  /* Voodoo3 disables this function because of hardware problems, but  */
  /* it should be enabled on Napalm.  -CGW-                            */

#ifdef AGP_CMDFIFO
  if (IS_NAPALM)
    Promote_CmdFifoToAGP(ppdev);
#endif // AGP_CMDFIFO

  // Restore busy flag.

  RESTORE_BUSY(pFlags,SaveBusy);

  // Dirty the D3D context, to force D3D to setup registers
  // for both chips, since we may have entered SLI mode

  _D3(lastContext) = 0;

  return DD_OK;
}

/*----------------------------------------------------------------------
Function name: Exit_3DApplication

Description:   Exiting a 3D, exclusive-mode, full-screen application.
               
Return:        DD_OK
----------------------------------------------------------------------*/

DWORD Exit_3DApplication(NT9XDEVICEDATA * ppdev)
{
  WORD * pFlags = (WORD *)_FF(lpDeFlags);
  WORD SaveBusy = *pFlags;

  // Set busy flag.

  Set_Busy(pFlags);

  // Check power mode.

  if (LOW_POWER_MODE(SaveBusy))
  {
    return DD_OK;
  }

  // Make note that we have destroyed the 3D flipping chain
  _DD(dwFlags) &= ~(DDGLOBAL_3DCHAINON);

  /* Disable video display. */

  if (IS_NAPALM)
  {
	DWORD vidProcCfg = GET(ghwIO->vidProcCfg);
	vidProcCfg &= ~SST_VIDEO_PROCESSOR_EN;
	SETDW(ghwIO->vidProcCfg, vidProcCfg);
  }

  /* Disable SLI and AA setup. */

#ifdef SLI_AA
  if (IS_NAPALM)
      Demote_DeviceFromSLIAA(ppdev);
#endif // SLI_AA

  /* Demote from video overlay. */

#ifdef ENABLE_TILED_HEAP
  Demote_PrimaryFromOverlay(ppdev);
#endif // ENABLE_TILED_HEAP

  /* Disable AGP command FIFO. */

#ifdef AGP_CMDFIFO
  if (IS_NAPALM)
    Demote_CmdFifoFromAGP(ppdev);
#endif

  // Restore busy flag.

  RESTORE_BUSY(pFlags,SaveBusy);

  _D3(lastContext) = 0;

  /* Restore gamma for GDI. */

  RESTOREGAMMA(ppdev);

  return DD_OK;
}

/*----------------------------------------------------------------------
Function name: Promote_PrimaryToOverlay

Description:   Display primary using overlay.
               
Return:        DD_OK
----------------------------------------------------------------------*/

DWORD Promote_PrimaryToOverlay(NT9XDEVICEDATA * ppdev)
{
#ifdef WIN_CSIM
   #undef ghwIO
   SstIORegs * ghwIO = (SstIORegs *)_FF(regRealBase);
   SstRegs * l3dRegs = (SstRegs *)(_FF(regRealBase) + SST_3D_OFFSET);
#endif
  DWORD vidProcCfg, vidOverlayStartAddr, vidOverlayEndAddr, vidDesktopOverlayStride;
  DWORD vidScreenSize;
  FXSURFACEDATA *surfaceData;

#ifdef WIN_CSIM
  if (_FF(bpp) == 32) // Cannot promote to 32bpp overlay on Voodoo3.
    return DD_OK;
#endif

  // vidProcCfg

  vidProcCfg = GET(ghwIO->vidProcCfg);
  vidProcCfg &= ~(SST_DESKTOP_EN                |  // Disable desktop
                  SST_OVERLAY_STEREO_EN         |  // Disable stereo
                  SST_INTERLACED_EN             |  // No interlace output
                  SST_CHROMA_EN                 |  // Disable chromakey
                  SST_CHROMA_INVERT             |  // Disable chromakey invert
                  SST_VIDEOIN_AS_OVERLAY        |  // Disable autoflipping overlay
                  SST_OVERLAY_CLUT_BYPASS       |  // No overlay clut bypass
                  SST_OVERLAY_CLUT_SELECT       |  // No overlay clut select
                  SST_OVERLAY_HORIZ_SCALE_EN    |  // Disable horizontal scaling
                  SST_OVERLAY_VERT_SCALE_EN     |  // Disable vertical scaling
                  SST_OVERLAY_FILTER_MODE       |  // Disable bilinear scaling
                  SST_OVERLAY_PIXEL_FORMAT      |  // Clear pixel format
                  SST_OVERLAY_TILED_EN          |  // Select overlay linear space
                  SST_OVERLAY_DEINTERLACE_EN    |  // Disable backend deinterlace
                  0 );
	  
  vidProcCfg |=  (SST_OVERLAY_EN                |  // Enable overlay
                  SST_OVERLAY_FILTER_2X2);         // Enable overlay filtering       

  // From Scott Sellers
  // vidProcCfg[23:21] should be set to RGB565U for AA modes
  // since the desktop and the overlay need to have the same mode
  if (_FF(bpp) == 32)
    vidProcCfg |=  SST_OVERLAY_PIXEL_RGB32U;       // Select 888 format
  else if (_DD(ddAAModeRequested))
    vidProcCfg |=  SST_OVERLAY_PIXEL_RGB565U;      // Select 565U format
  else
    vidProcCfg |=  SST_OVERLAY_PIXEL_RGB565D;      // Select 565 format
  vidProcCfg |= SST_OVERLAY_TILED_EN;


#ifdef STEREO
	if(_FF(ddStereoHeapFactor)) vidProcCfg |=  SST_OVERLAY_STEREO_EN;
#endif
  // Use video scaling for low resolution modes.

  if (SST_HALF_MODE & vidProcCfg)
  {
    #define STRETCH_BY_TWO  0x00080000

    vidProcCfg |= SST_OVERLAY_VERT_SCALE_EN;
    SETDW(ghwIO->vidOverlayDvdy, STRETCH_BY_TWO);
    SETDW(ghwIO->vidOverlayDvdyOffset, 0);
  }

  // Antialiasing specific code
  if (IS_NAPALM)
  {
    if (_DD(ddAAModeRequested))
    {
      vidProcCfg |= (SST_DESKTOP_EN | SST_CHROMA_EN);
      vidScreenSize = GET(ghwIO->vidScreenSize);
      vidScreenSize |= SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE;
      SETDW(ghwIO->vidScreenSize, vidScreenSize);
    }

    // In Enhanced Video Mode AA or SLI then we need to
    // set this register to have hsync in the middle of the line
    // Scott says to set this to the max!!!!!
    if (_DD(ddAAModeRequested) || _DD(ddSLIModeRequested) || (_DD(ddAANumberSamples) && (_FF(dwNumUnits) > 1)))
      SETDW(ghwIO->vidOverlayDudx, _FF(hres)>>1);
//      SETDW(ghwIO->vidOverlayDudx, 0x7FF);
  }

  vidProcCfg |= SST_VIDEO_PROCESSOR_EN; // Enable video
  SETDW(ghwIO->vidProcCfg,vidProcCfg);

  // vidDesktopOverlayStride

  vidDesktopOverlayStride = GET(ghwIO->vidDesktopOverlayStride);
  vidDesktopOverlayStride &= ~SST_OVERLAY_LINEAR_STRIDE;
  vidDesktopOverlayStride |= _DS(ddTileStride) << SST_OVERLAY_STRIDE_SHIFT;
  vidDesktopOverlayStride &= ~SST_OVERLAY_OVERFLOW_DUDX_WIDTH;
  vidDesktopOverlayStride |= (((_FF(bpp) >> 3) * _FF(hres)) & SST_OVERLAY_OVERFLOW_DUDX_BIT) << SST_OVERLAY_OVERFLOW_RELATIVE_SHIFT;
  SETDW(ghwIO->vidDesktopOverlayStride, vidDesktopOverlayStride);

  // vidOverlayStartAddr,vidOverlayEndAddr

  GETOVERLAYADDR( 0, 0, vidOverlayStartAddr);
  SETDW(ghwIO->vidOverlayStartCoords, vidOverlayStartAddr);
  if (SST_HALF_MODE & vidProcCfg)
  {
    GETOVERLAYADDR( (_FF(hres) - 1), ((2 * _FF(vres)) - 1), vidOverlayEndAddr);
  }
  else
  {
    GETOVERLAYADDR( (_FF(hres) - 1), (_FF(vres) - 1), vidOverlayEndAddr);
  }
  SETDW(ghwIO->vidOverlayEndScreenCoord, vidOverlayEndAddr);

  // vidOverlayDudxOffsetSrcWidth 

  SETDW(ghwIO->vidOverlayDudxOffsetSrcWidth, (((_FF(bpp) >> 3) * _FF(hres)) << SST_OVERLAY_FETCH_SIZE_SHIFT));

  _FF(ddVisibleOverlaySurf) = GETPRIMARY - _DS(LFBBASE);
  _FF(lastOverlayAddress) = INVALID_ADDRESS;

  // vidMaxRGBDelta

  SETDW(ghwIO->vidMaxRGBDelta,0x1F0F1F);

  // Setup overlay filter mode.

  vidProcCfg = GET(ghwIO->vidProcCfg) & ~SST_OVERLAY_FILTER_MODE;   // clear filter mode bits
  switch (_DD(overlayFilter) & 0x3)
  {
    default:
    case 0:	// Registry key not set or set to zero, use optimal
    case 1:	// Registry key optimal, use 2X2 filter if < 1024x768, else use 4x4 filter
      if(_FF(hres) < 1024)
        vidProcCfg |= SST_OVERLAY_FILTER_2X2;     // set 2x2 mode
      else    	
        vidProcCfg |= SST_OVERLAY_FILTER_4X4;     // set 4x4 mode
      break;
    case 2:	// Registry key normal - Use 4x4 video filter always
      vidProcCfg   |= SST_OVERLAY_FILTER_4X4;     // set 4x4 mode
      break;
    case 3: // Registry key high, use 2X2 video filter always, except when pixel doubled
      if(SST_VIDEO_2X_MODE_EN & vidProcCfg)
        vidProcCfg |= SST_OVERLAY_FILTER_4X4;     // set 4x4 mode
      else    	
        vidProcCfg |= SST_OVERLAY_FILTER_2X2;     // set 2x2 mode
      break;
  }

  // From Scott Sellers
  // vidProcCfg[17:16] should be set to 0 for AA Modes
  // and 0x0 or 0x2 for SLI modes
  if (IS_NAPALM)
      {
      // If AA Mode then we can do no filtering      
      if (_DD(ddAAModeRequested))
         {
         vidProcCfg = vidProcCfg & ~(SST_OVERLAY_FILTER_MODE);
         }

      // If SLI Mode then Filter Can be 4x4 Dither or Point
      if (_DD(ddSLIModeRequested) || (_DD(ddAANumberSamples) && (_FF(dwNumUnits) > 1)))
         {
         DWORD filterMode;
         filterMode = vidProcCfg & (SST_OVERLAY_FILTER_MODE);
         if ((SST_OVERLAY_FILTER_2X2 == filterMode) || (SST_OVERLAY_FILTER_BILINEAR == filterMode))
            {
            vidProcCfg = vidProcCfg & ~(SST_OVERLAY_FILTER_MODE);            
            }            
         }
      }

  SETDW(ghwIO->vidProcCfg,vidProcCfg);

  // Swap to overlay

  surfaceData = (FXSURFACEDATA*) &(_FF(ddPrimarySurfaceData));

  // When AA is enabled, display from both buffers.

  if ((IS_NAPALM) && (_DD(ddAAModeRequested)))
  {
    SETDW(ghw0->leftOverlayBuf,  (surfaceData->hwPtr & ~SSTG_IS_TILED));
    SETDW(ghw0->leftDesktopBuf,  (surfaceData->AAhwPtr & ~SSTG_IS_TILED));
    SETDW(ghw0->swapbufferCMD, SST_SWAP_DESKTOP_EN);
  }
  else
  {
    SETDW(ghw0->leftOverlayBuf,  (surfaceData->hwPtr & ~SSTG_IS_TILED));
    SETDW(ghw0->swapbufferCMD, 0x0);
  }

#ifdef WIN_CSIM

  // When AA is enabled, display from both buffers.

  if ((IS_NAPALM) && (_DD(ddAAModeRequested)))
  {
    SETDW(l3dRegs->leftOverlayBuf,  (surfaceData->hwPtr & ~SSTG_IS_TILED));
    SETDW(l3dRegs->leftDesktopBuf,  (surfaceData->AAhwPtr & ~SSTG_IS_TILED));
    SETDW(l3dRegs->swapbufferCMD, SST_SWAP_DESKTOP_EN);
  }
  else
  {
    SETDW(l3dRegs->leftOverlayBuf,  (surfaceData->hwPtr & ~SSTG_IS_TILED));
    SETDW(l3dRegs->swapbufferCMD, 0x0);
  }

#endif

  _DD(dd3DInOverlay) = 1;

  return DD_OK;

} // Promote_PrimaryToOverlay


/*----------------------------------------------------------------------
Function name: Demote_PrimaryFromOverlay

Description:   Switch primary from overlay to desktop.

Return:        DD_OK
----------------------------------------------------------------------*/

DWORD Demote_PrimaryFromOverlay(NT9XDEVICEDATA * ppdev)
{
#ifdef WIN_CSIM
   #undef ghwIO
   SstIORegs * ghwIO = (SstIORegs *)_FF(regRealBase);
#endif
  DWORD vidProcCfg;
  DWORD vidScreenSize;

  // vidDesktopStartAddr

#ifdef STEREO
  	if(!_FF(ddStereoHeapFactor)) FXBUSYWAIT(ppdev);
#else
	FXBUSYWAIT(ppdev);
#endif

  SETDW(ghwIO->vidDesktopStartAddr, _DS(gdiDesktopStart));

  // vidProcCfg

  vidProcCfg = GET(ghwIO->vidProcCfg);
  vidProcCfg &= ~(SST_OVERLAY_EN                |  // Disable overlay
                  SST_OVERLAY_STEREO_EN         |  // Disable stereo
                  SST_INTERLACED_EN             |  // No interlace output
                  SST_CHROMA_EN                 |  // Disable chromakey
                  SST_CHROMA_INVERT             |  // Disable chromakey invert
                  SST_VIDEOIN_AS_OVERLAY        |  // Disable autoflipping overlay
                  SST_OVERLAY_CLUT_BYPASS       |  // No overlay clut bypass
                  SST_OVERLAY_CLUT_SELECT       |  // No overlay clut select
                  SST_OVERLAY_HORIZ_SCALE_EN    |  // Disable horizontal scaling
                  SST_OVERLAY_VERT_SCALE_EN     |  // Disable vertical scaling
                  SST_OVERLAY_FILTER_MODE       |  // Disable bilinear scaling
                  SST_OVERLAY_PIXEL_FORMAT      |  // Clear pixel format
                  SST_OVERLAY_TILED_EN          |  // Select overlay linear space
                  SST_OVERLAY_DEINTERLACE_EN    |  // Disable backend deinterlace
                  0 );
	
  vidProcCfg |= SST_DESKTOP_EN | 
                SST_VIDEO_PROCESSOR_EN; // Enable video

  SETDW(ghwIO->vidProcCfg,vidProcCfg);

  // vidScreenSize

  vidScreenSize = GET(ghwIO->vidScreenSize);
  vidScreenSize &= ~SST_VIDEO_SCREEN_DESKTOPADDR_FIFO_ENABLE;
  SETDW(ghwIO->vidScreenSize, vidScreenSize);
 
  if(_FF(ddVisibleOverlaySurf) == (GETPRIMARY - _DS(LFBBASE)))
  {
    _FF(ddVisibleOverlaySurf) = 0;
  }
 
  _DD(dd3DInOverlay) = 0;

  return DD_OK;

} // Demote_PrimaryFromOverlay


/*******************************************************************/
/*                    INTERNAL FUNCTIONS                           */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name: Clear_SLIAA_Buffers

Description:   Clears primary/secondary surfaces for SLI/AA transitions.

Return:        DD_OK
----------------------------------------------------------------------*/

void Clear_SLIAA_Buffers (NT9XDEVICEDATA * ppdev)
{
  SOLIDCOLORPARAMS SolidColorParams; 
  DWORD dstPixelFormat;

  SolidColorParams.fillData       = 0;
  SolidColorParams.fpVidMem       = 0;
  SolidColorParams.Pitch          = 0;
  SolidColorParams.linearPitch    = 0;
  SolidColorParams.dstLeft        = 0;
  SolidColorParams.dstTop         = 0;
  SolidColorParams.dstRight       = _DS(hres);
  SolidColorParams.dstBottom      = _DS(vres);
  SolidColorParams.BytesPerPel    = GETPRIMARYBYTEDEPTH;
  SolidColorParams.isTiled        = TRUE;
  SolidColorParams.isZClear       = FALSE;
  SolidColorParams.bltRop         = (SSTG_ROP_SRC << 16) | (SSTG_ROP_SRC << 8) | (SSTG_ROP_SRC);

  GETPIXELFORMAT(SolidColorParams.BytesPerPel, dstPixelFormat);
  BLTFMT(_DS(ddTileStride), dstPixelFormat, SolidColorParams.bltDstFormat);

  // Clear the Primary if SLI is Enabled
  if (_DD(ddSLIModeEnabled))
  {
    SolidColorParams.dwSliSurface = TRUE;

    SolidColorParams.bltDstBaseAddr = _FF(gdiDesktopStart) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);
  }

  // Clear all the back buffers
  if(_DS(dd3DSurfaceCount))
  {
    if (_DD(ddSLIModeEnabled))
    {
      SolidColorParams.dwSliSurface = TRUE;

      switch(_FF(ddNumColorBuff))
	  {
	  case 3:
	    SolidColorParams.bltDstBaseAddr = (_FF(ddTiledHeap2Start) -_FF(ddTiledHeapStart))/_FF(dwNumUnits);
		SolidColorParams.bltDstBaseAddr += _FF(ddTiledHeapStart);
        if (!_DD(ddAAModeEnabled))
        {
          SolidColorParams.bltDstBaseAddr += _FF(ddExtraMemorySize);
        }
		SolidColorParams.bltDstBaseAddr |= SSTG_IS_TILED;
	    DdSli2DSolidColor(ppdev, &SolidColorParams);

      case 2:
	    SolidColorParams.bltDstBaseAddr = (_FF(ddTiledHeap1Start) - SST_TILE_SIZE -_FF(ddTiledHeapStart))/_FF(dwNumUnits);
		SolidColorParams.bltDstBaseAddr += _FF(ddTiledHeapStart) + SST_TILE_SIZE;
        if (!_DD(ddAAModeEnabled))
        {
          SolidColorParams.bltDstBaseAddr += _FF(ddExtraMemorySize);
        }
		SolidColorParams.bltDstBaseAddr |= SSTG_IS_TILED;
	    DdSli2DSolidColor(ppdev, &SolidColorParams);

      case 1:
	    SolidColorParams.bltDstBaseAddr = (_FF(ddTiledHeap0Start) -_FF(ddTiledHeapStart))/_FF(dwNumUnits);
		SolidColorParams.bltDstBaseAddr += _FF(ddTiledHeapStart);
        if (!_DD(ddAAModeEnabled))
        {
          SolidColorParams.bltDstBaseAddr += _FF(ddExtraMemorySize);
        }
		SolidColorParams.bltDstBaseAddr |= SSTG_IS_TILED;
	    DdSli2DSolidColor(ppdev, &SolidColorParams);

	  default:
	    break;
      }
    }
    else
    {
      SolidColorParams.dwSliSurface = FALSE;

      switch(_FF(ddNumColorBuff))
	  {
	  case 3:
	    SolidColorParams.bltDstBaseAddr = _FF(ddTiledHeap2Start) | SSTG_IS_TILED;
	    DdSli2DSolidColor(ppdev, &SolidColorParams);

      case 2:
	    SolidColorParams.bltDstBaseAddr = _FF(ddTiledHeap1Start) | SSTG_IS_TILED;
	    DdSli2DSolidColor(ppdev, &SolidColorParams);

      case 1:
    	  SolidColorParams.bltDstBaseAddr = _FF(ddTiledHeap0Start) | SSTG_IS_TILED;
	    DdSli2DSolidColor(ppdev, &SolidColorParams);

	  default:
	    break;
      }
	}
  }

  // Clear secondary buffers, if AA enabled.
  if (_DD(ddAAModeEnabled))
  {
    if (_DD(ddSLIModeEnabled))
    {
      SolidColorParams.dwSliSurface = TRUE;

      SolidColorParams.bltDstBaseAddr = AdjustHwPtr(ppdev, _FF(secondaryThirdBuffer)) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);
 
      SolidColorParams.bltDstBaseAddr = AdjustHwPtr(ppdev, _FF(secondaryZBuffer)) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);

      SolidColorParams.bltDstBaseAddr = AdjustHwPtr(ppdev, _FF(secondaryBackBuffer)) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);

      SolidColorParams.bltDstBaseAddr = AdjustHwPtr(ppdev, _FF(secondaryFrontBuffer)) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);
    }
    else
    {
      SolidColorParams.dwSliSurface = FALSE;

      SolidColorParams.bltDstBaseAddr = _FF(ddLinearHeap4Start) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);
 
      SolidColorParams.bltDstBaseAddr = _FF(ddLinearHeap3Start) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);

      SolidColorParams.bltDstBaseAddr = _FF(ddLinearHeap2Start) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);

      SolidColorParams.bltDstBaseAddr = _FF(ddLinearHeap1Start) | SSTG_IS_TILED;
      DdSli2DSolidColor(ppdev, &SolidColorParams);
    }
  }

} // Clear_SLIAA_Buffers

/*----------------------------------------------------------------------
Function name: LoadGamma

Description:   Load gamma ramp for Direct3D, >= 16bpp only.
----------------------------------------------------------------------*/

void LOADGAMMA(NT9XDEVICEDATA *ppdev)
{
  DWORD Gamma[256];
  double rgamma,ggamma,bgamma;
  double rg1, gg1, bg1;
  double fval;
  LPSTR lpStr;
  int ir, ig, ib;
  int i;

   
  if (_FF(bpp) >= 16)
  {
    lpStr = GETENV(RED_NAME);
    if (NULL != lpStr)
      rgamma = ddatof(lpStr);
    else
      rgamma = 1.0;

    if (rgamma < 0.43)
      rgamma = 0.43;

    lpStr = GETENV(GREEN_NAME);
    if (NULL != lpStr)
      ggamma = ddatof(lpStr);
    else
      ggamma = 1.0;

    if (ggamma < 0.43)
       ggamma = 0.43;

    lpStr = GETENV(BLUE_NAME);
    if (NULL != lpStr)
       bgamma = ddatof(lpStr);
    else
       bgamma = 1.0;

    if (bgamma < 0.43)
       bgamma = 0.43;

    rg1 = 1.0/rgamma;
    gg1 = 1.0/ggamma;
    bg1 = 1.0/bgamma;

    for (i=0; i<256; i++)
    {
      fval = (double)i/(double)255.0;
      ir = (int)ddftol((float)(Exponential(fval, rg1) * 255.0 + .5));
      ig = (int)ddftol((float)(Exponential(fval, gg1) * 255.0 + .5));
      ib = (int)ddftol((float)(Exponential(fval, bg1) * 255.0 + .5));
      Gamma[i] = ((ir << 8) | ig) << 8 | ib;
    }               

    SETPALETTE(ppdev, Gamma, 0, 256);
  }

} // LoadGamma


/*----------------------------------------------------------------------
Function name: RestoreGamma

Description:   Restore gamma ramp for desktop, >= 16bpp only.
----------------------------------------------------------------------*/

void RESTOREGAMMA(NT9XDEVICEDATA * ppdev)
{
  DWORD Gamma[256];
  int i;

  // Must be >= 16bpp
  if (_FF(bpp) >= 16)
  {
    for (i=0; i<256; i++)
    {
      // Note that gamma_ramp table has data in most significant byte, not least!

      Gamma[i]  = (_FF(gamma_ramp).blue[i]  & 0xff00) << 8;
      Gamma[i] |= (_FF(gamma_ramp).green[i] & 0xff00);
      Gamma[i] |= (_FF(gamma_ramp).red[i]   & 0xff00) >> 8;
    }

    SETPALETTE(ppdev, Gamma, 0, 256);
  }

} // RestoreGamma


/*----------------------------------------------------------------------
Function name: SetPalette

Description:   Write hardware palette.

----------------------------------------------------------------------*/

#define MAX_RETRYS (10)
void SETPALETTE(NT9XDEVICEDATA * ppdev, DWORD FAR *pGamma, int nStart, int nSize)
{
  SstIORegs * pIORegs;
  DWORD dacMode;
  int i;
  int j;
  int k;
  int nTimeOut;
  FxU32 foo;
  int nNumChips;   


  // Use the real DAC on both CSIM and !CSIM
#ifdef WIN_CSIM
  pIORegs = ((SstIORegs *)_FF(regRealBase));
#else
  pIORegs = (SstIORegs *)ppdev->regBase[HWINFO_SST_IOREGS_INDEX];
#endif 

#ifdef SLI_AA
  if (_FF(gdiFlags) & SDATA_GDIFLAGS_SLI_AA_MASTER)
      nNumChips = _FF(dwNumUnits);
   else 
      nNumChips = 1;
#else
   nNumChips = 1;
#endif   

   // Make sure Hsync and Vsync are toggling before we check them
   // We disable the DAC on the slaves so we cannot do this....

   dacMode = GET(pIORegs->dacMode) & 0xFFFF;
   if (!(dacMode & (SST_DAC_DPMS_ON_VSYNC|SST_DAC_FORCE_VSYNC|SST_DAC_DPMS_ON_HSYNC|SST_DAC_FORCE_HSYNC)))
      {
      nTimeOut = 0;
      while ((!((GET(pIORegs->status) & SST_VRETRACE) ^ (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK))) && (nTimeOut < 10000))
	      {
	      nTimeOut++;
	      }
      }


  for (k=0; k<nNumChips; k++)
      { 
      for (i = 0; i<256; i++)
         {
         SETDW(pIORegs->dacAddr, i);
         foo = GET(pIORegs->dacAddr);
         for (j=0; j<MAX_RETRYS; j++)
            {
            SETDW(pIORegs->dacData, pGamma[i]);
            foo = GET(pIORegs->dacData);
            if (pGamma[i] == foo)
	            {
	            j=MAX_RETRYS;
	            }
            }
         }
         if (k + 1 < nNumChips)
            pIORegs = (SstIORegs *)ppdev->regBase[(k + 1) * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_IOREGS_INDEX];
      }


} // SetPalette


/*----------------------------------------------------------------------
Function name: Exponential

Description:   This functions computes a^x by using the series
               exponential a^x = 1 + x * ln a + (x * ln a)^2/2!
               + (x * ln a) ^ 3/3! + ...

Return:        double dRet is value of a to the x power
----------------------------------------------------------------------*/

#define EPSILON (0.00001)

double Exponential(double a, double x)
{
  double dRet;
  double dOldRet;
  double xlogea;
  double xFac;
  double xfac;
  int n;

  if ((-EPSILON <= a) && (a <= EPSILON))
    return (double)0.0;

  xlogea = x * Logarithm(a);   
  n = 1;
  xFac = 1.0;
  xfac = 1.0;
  dRet = 1.0;
  dOldRet = 2.0;
  while (fabs(dRet - dOldRet) > EPSILON)
  {
    dOldRet = dRet;
    xFac = xFac * n; 
    xfac = xfac * xlogea;                  
    n += 1;
    dRet = dRet + xfac/xFac;      
  }
   
  return dRet;

} // Exponential


/*----------------------------------------------------------------------
Function name: Logarithm

Description:   This functions computes ln(x).  We use the formula
			   ln (1+x)/(1-x) = 2(x + x^3/3 + x^5/5 + x^7/7 + ...)
			   where we set y=(1+x)/(1-x) and solve for x giving
               x=(y-1)/y+1)

Return:        double dRet is ln(x)
----------------------------------------------------------------------*/

double Logarithm(double x)
{
  double xfac;
  double xPart;
  double xsqrd;
  double xnum;
  double dRet;
  double dOldRet;

  xfac = (x - 1.0)/(x + 1.0);
  xsqrd = xfac * xfac;   

  dRet = xfac;
  dOldRet = 2.0;
  xPart = xfac;
  xnum = 3.0;
  while (fabs(dRet - dOldRet) > EPSILON)
  {
    dOldRet = dRet;
    xPart = xPart * xsqrd;
    dRet = dRet + xPart/xnum;
    xnum += 2.0; 
  }

  dRet = dRet * 2.0;

  return dRet;

} // Logarithm

#ifdef SLI_AA

/*----------------------------------------------------------------------
Function name: RetrieveFromRegistry

Description:   Common code to read a integer value from the registry

Return:        default, Min, max or registry value
----------------------------------------------------------------------*/

DWORD RetrieveFromRegistry(NT9XDEVICEDATA * ppdev, const char * pStr, DWORD dwDefault, DWORD dwMin, DWORD dwMax)
{
  DWORD dwReturn = dwDefault;
  LPSTR lpStr;

  lpStr = GETENV(pStr);
  if (NULL != lpStr)
  {
    dwReturn = ddatoi(lpStr);
    if (dwReturn < dwMin)
      dwReturn = dwMin;
    if (dwReturn > dwMax)
      dwReturn = dwMax;
  }
   
   return dwReturn;
}

/*----------------------------------------------------------------------
Function name: GetLog2

Description:   Function to Log 2

Return:        1
----------------------------------------------------------------------*/
DWORD GetLog2(DWORD dwNum)
{
   DWORD dwReturn;

   for (dwReturn=0, dwNum>>=1; dwNum > 0; dwNum>>=1, dwReturn++)
      ;

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name:  Promote_DeviceToSLIAA

Description:    Enable SLI or AA mode.
----------------------------------------------------------------------*/
// A few Key board defines
#define SS_SHIFT (0x0001)
#define SS_LSHIFT (0x0002)
#define SS_RSHIFT (0x0200)

#define SS_CTRL   (0x0080)
#define SS_LCTRL   (0x0004)
#define SS_RCTRL   (0x0400)

#define SS_ALT (0x0100)
#define SS_LALT (0x0008)
#define SS_RALT (0x0800)

#define SS_CAPLOCK (0x0040)
#define SS_NUMLOCK (0x0020)
#define SS_SCRLLOCK (0x0010)

#define SS_CAPLOCK_DN (0x4000)
#define SS_NUMLOCK_DN (0x2000)
#define SS_SCRLLOCK_DN (0x1000)
#define SS_WIN       (0x8000)

HOTKEY_DATA  HotKey_Data[Number_HotKeys] ;

DWORD Promote_DeviceToSLIAA(NT9XDEVICEDATA * ppdev)
{
  DIOC_DATA DIOC_Data;
  SLI_AA_REQUEST Sli_AA_Request;
  HANDLE hDevice;
  DWORD vidProcCfg;
  DWORD dwLog2GroupHeight;
  DWORD dwOverRide;
  DWORD dwSliBandHeight;
  DWORD dwVirtualKey;
  LPSTR lpStr;

#ifdef WIN_CSIM
   #undef ghwIO
   SstIORegs * ghwIO = (SstIORegs *)_FF(regRealBase);
#endif

  // If SLI or AA requested, and this is a Napalm
  if (_DD(ddAAModeRequested) || _DD(ddSLIModeRequested) || _DD(ddAANumberSamples))
  {
    _DD(ddAAModeEnabled)    = _DD(ddAAModeRequested);
    _DD(ddSLIModeEnabled)   = _DD(ddSLIModeRequested);

    // Move the tile mark and adjust the desktop address.

    MoveTileMark(ppdev);

    // Disable hardware cursors.

    _FF(gdiFlags) |= SDATA_GDIFLAGS_SLI_AA_MASTER;
    SwitchToHostCursor(ppdev);

    // Switch to a Host Centered Cursor
    // Save FB Offsets
    _DD(ddOldHostcursorAndStart) = _FF(HostcursorAndStart);
    _DD(ddOldHostcursorXorStart) = _FF(HostcursorXorStart);
    _DD(ddOldHostcursorSrcStart) = _FF(HostcursorSrcStart);
    _DD(ddOldHostcursorExclusionStart) = _FF(HostcursorExclusionStart);

    //Copy Data
    memcpy((void *)_DD(ddHostcursorAndStart), (void *)_FF(HostcursorAndStart), 4096);
    memcpy((void *)_DD(ddHostcursorXorStart), (void *)_FF(HostcursorXorStart), 4096);
    memcpy((void *)_DD(ddHostcursorSrcStart), (void *)_FF(HostcursorSrcStart), 4096);
	// set exclusion start to zero since we are clearing the buffers
    memset((void *)_DD(ddHostcursorExclusionStart), 0x0, 4096);

    // Get Host Based Ones
    _FF(HostcursorAndStart) = _DD(ddHostcursorAndStart);
    _FF(HostcursorXorStart) = _DD(ddHostcursorXorStart);
    _FF(HostcursorSrcStart) = _DD(ddHostcursorSrcStart);
    _FF(HostcursorExclusionStart) = _DD(ddHostcursorExclusionStart);

    // Disable overlay filtering.

    vidProcCfg = GET(ghwIO->vidProcCfg);
    
#ifdef STEREO
	if(_FF(ddStereoHeapFactor)) vidProcCfg |=  SST_OVERLAY_STEREO_EN;
#endif
    
    SETDW(ghwIO->vidProcCfg, vidProcCfg & ~SST_OVERLAY_FILTER_MODE);

    // Request SLI or AA enable
  
    Sli_AA_Request.ChipInfo.dwaaEn             = _DD(ddAAModeEnabled);
    Sli_AA_Request.ChipInfo.dwsliEn            = _DD(ddSLIModeEnabled);

    // Read the following from registry.

    // Entry Is Opposite of Our Logic
    Sli_AA_Request.ChipInfo.dwsliAaAnalog      = _DD(ddSLIAAAnalog);

	// For everything below 768, lets use the SLI Band Height of 16
	// For everything above or equal to 768, lets use the SLI Band Height of 32
	dwSliBandHeight = ( _FF(vres) >= 768 ) ? 32 : 16;

   // Scott's Sellers suggestion for 4-chip optimization
   if (4 == _FF(dwNumUnits))
      dwSliBandHeight >>= 1;

	Sli_AA_Request.ChipInfo.dwsli_nlines       = RetrieveFromRegistry(ppdev, SLI_BAND_HEIGHT_NAME, dwSliBandHeight, 2, 128);
    if (Sli_AA_Request.ChipInfo.dwsli_nlines > 32)
      {
      dwOverRide = RetrieveFromRegistry(ppdev, SLI_BAND_HEIGHT_OVERRIDE_NAME, 0, 0, 1);
      if (!dwOverRide)
         Sli_AA_Request.ChipInfo.dwsli_nlines = 32;
      }

    Sli_AA_Request.ChipInfo.dwCfgSwapAlgorithm = RetrieveFromRegistry(ppdev, SWAPBUFFER_ALGO_NAME, 1, 0, 1);

    _DD(ddSLINumberScanlines) = Sli_AA_Request.ChipInfo.dwsli_nlines;
    _DD(dwlog2BandHeight) = GetLog2(_DD(ddSLINumberScanlines));
    _DD(dwlog2NumChips) = GetLog2(_DD(ddSLINumberWays));  
    dwLog2GroupHeight = _DD(dwlog2BandHeight) + _DD(dwlog2NumChips);

    _DD(dwMaxHeight) = ((ppdev->bi.biHeight + (1 << dwLog2GroupHeight) - 1) >> dwLog2GroupHeight) << _DD(dwlog2BandHeight);
    // Tile Align Max Linear Height
    _DD(dwMaxLinearHeight) = (_DD(dwMaxHeight) + SST_TILE_HEIGHT - 1) & ~(SST_TILE_HEIGHT - 1);

    // Fill out the memory configuration.

    Sli_AA_Request.MemInfo.dwTotalMemory       = _FF(TotalVRAM);
    Sli_AA_Request.MemInfo.dwTileMark          = _FF(ddTileMark);
    Sli_AA_Request.MemInfo.dwTileCmpMark       = _FF(ddTileMark);

    // Fill out SLI mode information.

    Sli_AA_Request.ChipInfo.dwChips            = _FF(dwNumUnits);

    // Fill out AA mode information.

    Sli_AA_Request.ChipInfo.dwaaSampleHigh = _DD(ddAANumberSamples) >> 2; 

    // If AA Sample is non-zero then tell backend to do AA
    if (_DD(ddAANumberSamples))
      {
       Sli_AA_Request.ChipInfo.dwaaEn = 0x1;
      }

    Sli_AA_Request.MemInfo.dwaaSecondaryColorBufBegin  = _DD(ddAAPrimaryStart);
    Sli_AA_Request.MemInfo.dwaaSecondaryDepthBufBegin  = _DD(ddAAZbufferStart);
    Sli_AA_Request.MemInfo.dwaaSecondaryDepthBufEnd    = _DD(ddAAZbufferStart) + _FF(gdiDesktopSize) - 1;
    Sli_AA_Request.MemInfo.dwBpp = (DWORD)_FF(bpp);

    /* Call out miniVDD to add in the AA Toggle */

    hDevice = CreateFile(FILENAME, 0, 0, NULL, 0, 0, NULL);
    if (INVALID_HANDLE_VALUE != hDevice)
    {
      DIOC_Data.dwDevNode = _FF(DevNode);
      DIOC_Data.dwSpare = (DWORD)&Sli_AA_Request;
	  DIOC_Data.dwOffset = AA_HOTKEY ;
      DeviceIoControl(hDevice, SLI_AA_ENABLE, &DIOC_Data, sizeof(DIOC_Data), NULL, 0x0, NULL, NULL);
      dwVirtualKey = RetrieveFromRegistry(ppdev, AAJITTER_TOGGLE_KEY, 0xFFFFFFFF, 0x0, 0xFF);
      HotKey_Data[AA_HOTKEY].pHotKeyData = NULL;
      if (0xFFFFFFFF != dwVirtualKey)
         {      
         DIOC_Data.dwSpare = MapVirtualKey((WORD)dwVirtualKey, (WORD)0x0);
         lpStr = GETENV(AAJITTER_TOGGLE_MODIFIER);
         if (NULL != lpStr)
            ddsscanf(lpStr,"%i", &DIOC_Data.dwModifier);
         else
            DIOC_Data.dwModifier = (~(SS_SHIFT|SS_LSHIFT|SS_RSHIFT|SS_CAPLOCK|SS_NUMLOCK|SS_SCRLLOCK))<<16;

         if (0xFFFFFFFF != DIOC_Data.dwSpare)
            {
            DeviceIoControl(hDevice, HOT_KEY_FLAG, &DIOC_Data, sizeof(DIOC_Data), NULL, 0x0, NULL, NULL);
            HotKey_Data[AA_HOTKEY].pHotKeyData = (DWORD *)DIOC_Data.dwSpare;
            if (NULL != HotKey_Data[AA_HOTKEY].pHotKeyData)
               *HotKey_Data[AA_HOTKEY].pHotKeyData = 0x0;
            }

         }
    }
    CloseHandle(hDevice);

	/* Call the miniVDD to add in the Screen capture Toggle */
	hDevice = CreateFile(FILENAME, 0, 0, NULL, 0, 0, NULL);
    if (INVALID_HANDLE_VALUE != hDevice)
    {
      DIOC_Data.dwDevNode = _FF(DevNode);
	  DIOC_Data.dwOffset = ScreenShot_HOTKEY ;
      dwVirtualKey = RetrieveFromRegistry(ppdev, SCREENSHOT_TOGGLE_KEY, 0xFFFFFFFF, 0x0, 0xFF);
      HotKey_Data[ScreenShot_HOTKEY].pHotKeyData = NULL;
      if (0xFFFFFFFF != dwVirtualKey)
         {      
         DIOC_Data.dwSpare = MapVirtualKey((WORD)dwVirtualKey, (WORD)0x0);
         lpStr = GETENV(AAJITTER_TOGGLE_MODIFIER);
         if (NULL != lpStr)
            ddsscanf(lpStr,"%i", &DIOC_Data.dwModifier);
         else
            DIOC_Data.dwModifier = (~(SS_SHIFT|SS_LSHIFT|SS_RSHIFT|SS_CAPLOCK|SS_NUMLOCK|SS_SCRLLOCK))<<16;

         if (0xFFFFFFFF != DIOC_Data.dwSpare)
            {
            DeviceIoControl(hDevice, HOT_KEY_FLAG, &DIOC_Data, sizeof(DIOC_Data), NULL, 0x0, NULL, NULL);
            HotKey_Data[ScreenShot_HOTKEY].pHotKeyData = (DWORD *)DIOC_Data.dwSpare;
            if (NULL != HotKey_Data[ScreenShot_HOTKEY].pHotKeyData)
               *HotKey_Data[ScreenShot_HOTKEY].pHotKeyData = 0x0;
            }
         }
    }
    CloseHandle(hDevice);
  }

#ifdef RD_ABORT_ERROR
  if (_DD(ddSLIModeRequested))
      _FF(dwSLIMode) = H3VDD_DISABLE_SLI_READ;
#endif

  return 1;

} // Promote_DeviceToSLIAA


/*----------------------------------------------------------------------
Function name:  Demote_DeviceFromSLIAA

Description:    Disable SLI or AA mode.
----------------------------------------------------------------------*/

DWORD Demote_DeviceFromSLIAA(NT9XDEVICEDATA * ppdev)
{
  DIOC_DATA DIOC_Data;
  SLI_AA_REQUEST Sli_AA_Request;
  HANDLE hDevice;
#ifdef STEREO
  DWORD vidProcCfg;
#endif

  if (_DD(ddAAModeEnabled) || _DD(ddSLIModeEnabled) || _DD(ddAANumberSamples))
  {
#ifdef RD_ABORT_ERROR
    _FF(dwSLIMode) = H3VDD_SLI_READ_NOT_IN_USE;
#endif
    // Use identical enable values to promotion.

#ifdef STEREO
	vidProcCfg = GET(ghwIO->vidProcCfg);
  	vidProcCfg &= ~(SST_OVERLAY_EN                |  // Disable overlay
                  SST_OVERLAY_STEREO_EN         |  // Disable stereo
                  SST_INTERLACED_EN             |  // No interlace output
                  SST_CHROMA_EN                 |  // Disable chromakey
                  SST_CHROMA_INVERT             |  // Disable chromakey invert
                  SST_VIDEOIN_AS_OVERLAY        |  // Disable autoflipping overlay
                  SST_OVERLAY_CLUT_BYPASS       |  // No overlay clut bypass
                  SST_OVERLAY_CLUT_SELECT       |  // No overlay clut select
                  SST_OVERLAY_HORIZ_SCALE_EN    |  // Disable horizontal scaling
                  SST_OVERLAY_VERT_SCALE_EN     |  // Disable vertical scaling
                  SST_OVERLAY_FILTER_MODE       |  // Disable bilinear scaling
                  SST_OVERLAY_PIXEL_FORMAT      |  // Clear pixel format
                  SST_OVERLAY_TILED_EN          |  // Select overlay linear space
                  SST_OVERLAY_DEINTERLACE_EN    |  // Disable backend deinterlace
                  0 );
	
  vidProcCfg |= SST_DESKTOP_EN | 
                SST_VIDEO_PROCESSOR_EN; // Enable video

  SETDW(ghwIO->vidProcCfg,vidProcCfg);
#endif



    Sli_AA_Request.ChipInfo.dwaaEn       = _DD(ddAAModeEnabled);
    Sli_AA_Request.ChipInfo.dwsliEn      = _DD(ddSLIModeEnabled);

    // Use number of scanlines stored during promotion.

    Sli_AA_Request.ChipInfo.dwsli_nlines = _DD(ddSLINumberScanlines);

    // Fill out SLI mode information.

    Sli_AA_Request.ChipInfo.dwChips      = _FF(dwNumUnits);
 
    // Call the miniVDD.
 
    hDevice = CreateFile(FILENAME, 0, 0, NULL, 0, 0, NULL);
    if (INVALID_HANDLE_VALUE != hDevice)
    {
      DIOC_Data.dwDevNode = _FF(DevNode);
      DIOC_Data.dwSpare = (DWORD)&Sli_AA_Request;
      DeviceIoControl(hDevice, SLI_AA_DISABLE, &DIOC_Data, sizeof(DIOC_Data), NULL, 0x0, NULL, NULL);
      DIOC_Data.dwSpare = 0x0;
	  DIOC_Data.dwOffset = AA_HOTKEY ;		// Its the AA hotkey toggle we want to disable
      DeviceIoControl(hDevice, HOT_KEY_OFF_FLAG, &DIOC_Data, sizeof(DIOC_Data), NULL, 0x0, NULL, NULL);
      DIOC_Data.dwSpare = 0x0;
	  DIOC_Data.dwOffset = ScreenShot_HOTKEY ;		// Its the ScreenShot hotkey toggle we want to disable
      DeviceIoControl(hDevice, HOT_KEY_OFF_FLAG, &DIOC_Data, sizeof(DIOC_Data), NULL, 0x0, NULL, NULL);
    }
    CloseHandle(hDevice);

    // Restore Old Pointers
    _FF(HostcursorAndStart) = _DD(ddOldHostcursorAndStart);
    _FF(HostcursorXorStart) = _DD(ddOldHostcursorXorStart);
    _FF(HostcursorSrcStart) = _DD(ddOldHostcursorSrcStart);
    _FF(HostcursorExclusionStart) = _DD(ddOldHostcursorExclusionStart);

    // Restore Old Buffers
    memcpy((void *)_FF(HostcursorAndStart), (void *)_DD(ddHostcursorAndStart), 4096);
    memcpy((void *)_FF(HostcursorXorStart), (void *)_DD(ddHostcursorXorStart), 4096);
    memcpy((void *)_FF(HostcursorSrcStart), (void *)_DD(ddHostcursorSrcStart), 4096);
    memset((void *)_FF(HostcursorExclusionStart), 0x0, 4096);

    _FF(gdiFlags) &= ~SDATA_GDIFLAGS_SLI_AA_MASTER;
    SwitchToHostCursor(ppdev);
  }

  // Disable SLI and AA modes.

  _DD(ddSLIModeEnabled)   = FALSE;
  _DD(ddAAModeEnabled)   = FALSE;
  _DD(ddAANumberSamples) = 0x0;

  _DD(ddSLIModeRequested) = FALSE;
  _DD(ddAAModeRequested) = FALSE;

  // Sometime the order of the calls is Enable1, Demote
  // instead of Demote, Enable1.  In the latter case, 8 bpp
  // is not setup right to call the functions in MoveTileMark
  // so let's not do it.  We can tell since this is the case where the
  // TileMark is all of Memory
  if (_FF(ddTileMark) != _FF(TotalVRAM))  
      {
      // Move the tile mark and adjust the desktop address.
      MoveTileMark(ppdev);
      }

  return 1;

} // Demote_DeviceFromSLIAA

typedef struct lookup
{
  DWORD ConfigID ;
  BYTE	Chip_Count ;		// Number of chips this config has (see table below)
  BYTE  SLI_Mode ;			// SLI Mode of this config (see table below)
  BYTE  AA_Mode ;			// AA Mode of this config (see table below)
} LOOKUP;

typedef struct badapp
{
  PSTR  pszProcFileName;    // Bad Application Name                                       __
  BYTE	Chip_Count ; 		// Chip count that triggers the new config (see table below)    |
  BYTE  SLI_Mode ;			// SLI Mode that triggers the new config (see table below)      +- All 3 must be matched..
  BYTE  AA_Mode ;			// AA Mode that triggers the new config (see table below)     __|
  DWORD newConfig ;			// New Configuration
} BADAPP;

   
// This string is used encripted using a simple substition cypher
                       //   'U'   'S'   'A'   'F'   ' '   'F'   'O'   'R'   ' '   'G'   'A'   'M'   'E'   'G'   'A'   'U'   'G'   'E'   '.'   'E'   'X'   'E'  
unsigned char pUSAF[] = { 0xaa, 0xac, 0xbe, 0xb9, 0xdf, 0xb9, 0xb0, 0xad, 0xdf, 0xb8, 0xbe, 0xb2, 0xba, 0xb8, 0xbe, 0xaa, 0xb8, 0xba, 0xd1, 0xba, 0xa7, 0xba, 0x00};

                            // 'S'   'C'   '3'   '.'   'E'   'X'   'E'                           
unsigned char pSimCity3k[] = {0xac, 0xbc, 0xcc, 0xd1, 0xba, 0xa7, 0xba, 0x00};

								      //   'S'   'C'   '3'   'U'   '.'   'I'   'C'   'D'   
unsigned char pSimCity3kWorldEdition[] = {0xac, 0xbc, 0xcc, 0xaa, 0xd1, 0xb6, 0xbc, 0xbb, 0x00 };

/*

  Detecting Bad Applications (Running 1+ - eg. DUAL_CHIP_SLI_2WAY_AA_DISABLED or QUAD_CHIP_SLI_4WAY_AA_DISABLED)

  This system allows us to find a bad application and modify the config of the card, based on its current configuration.
  For example, we can detected SimCity 3000 (e.g.) running on a Quad VSA-100 boards with 2x and 8x FSAA and convert it
  to 4x FSAA >OR< disable all FSAA for SimCity 3000 on all cards.
    
  How
  ---
  
  The routine logically AND's the current config (via a lookup) against the games "bad" config values, if we return a 
  match on all fields we force the new config. The values in BadApps are made up using the following tables, an example
  of "0x7, 0x3, 0x5" shows ALL BOARDS (0x7), ANY SLI (0x3) and 8x and 2x FSAA (0x5).


  Bit Patterns 
  ------------

             >> NUMBER OF CHIPS  <<	                        >> SLI MODE <<					   >> AA MODES <<

  Bit Position  =  2 | 1 | 0			       Bit Position  =  1 | 0	           Bit Position  =  2 | 1 | 0	
                  ---+---+---			 	                   ---+---                             ---+---+---							
				   0 | 0 | 0  - Invalid                         0 | 0  - No SLI                     0 | 0 | 0  - None
				   0 | 0 | 1  - S                               0 | 1  - 2 WAY                      0 | 0 | 1  - 2x
				   0 | 1 | 0  - D                               1 | 0  - 4 WAY                      0 | 1 | 0  - 4x    
				   0 | 1 | 1  - S + D                           1 | 1  - Any WAY                    0 | 1 | 1  - 2x + 4x    
				   1 | 0 | 0  - Q                                                                   1 | 0 | 0  - 8x    
				   1 | 0 | 1  - Q + S                                                               1 | 0 | 1  - 2x + 8x    
				   1 | 1 | 0  - Q + D                                                               1 | 1 | 0  - 4x + 8x    
				   1 | 1 | 1  - All Boards                                                          1 | 1 | 1  - Any AA Mode
		
  - JHunter (19/09/00)
  */

LOOKUP LookupValues[] =
{
  // Config Name               No Chips|SLI|AA Mode
  SINGLE_CHIP_NOSLI_AA_DISABLED  , 0x1, 0x0, 0x0, 
  SINGLE_CHIP_NOSLI_AA_2SAMPLE   , 0x1, 0x0, 0x1,
  DUAL_CHIP_SLI_2WAY_AA_DISABLED , 0x2, 0x1, 0x0,
  DUAL_CHIP_SLI_2WAY_AA_2SAMPLE  , 0x2, 0x1, 0x1,   
  DUAL_CHIP_NOSLI_AA_4SAMPLE     , 0x2, 0x0, 0x2,      
  QUAD_CHIP_SLI_4WAY_AA_DISABLED , 0x4, 0x2, 0x0,  
  QUAD_CHIP_SLI_4WAY_AA_2SAMPLE  , 0x4, 0x2, 0x1,   
  QUAD_CHIP_SLI_2WAY_AA_4SAMPLE  , 0x4, 0x1, 0x2,  
  QUAD_CHIP_NOSLI_AA_8SAMPLE     , 0x4, 0x0, 0x4,   
};

BADAPP BadApps[] =
{
   {
    pUSAF,
    0x2, 0x1, 0x0,						// DUAL_CHIP_SLI_2WAY_AA_DISABLED - Reads : Dual Chip with 2WAY SLI and no AA enabled.
    SINGLE_CHIP_NOSLI_AA_DISABLED,
   },

   {
    pSimCity3k,   
	0x7, 0x3, 0x7,						// Any Board, Any SLI, Any AA mode...           
    SINGLE_CHIP_NOSLI_AA_DISABLED,
   },

   {
    pSimCity3kWorldEdition,
	0x7, 0x3, 0x7,						// Any Board, Any SLI, Any AA mode...
    SINGLE_CHIP_NOSLI_AA_DISABLED,
   },
};

#define ENCODE(ch) (255-(unsigned char)ch)
/*----------------------------------------------------------------------
Function name:  GetProcessFileName

Description:    Function to Determine Process File Name
----------------------------------------------------------------------*/
VOID GetProcessFileName(LPSTR lpBuf, DWORD dwSize)
{
  HANDLE hFile;
  DWORD  i;

  // GetModuleFileName returns length of filename (exclude null terminated
  // char) if successful, else returns zero

  hFile = GetModuleHandle(NULL);
  if ( (i = GetModuleFileName(hFile, lpBuf, dwSize)) )
  {
    // locate first char of filename, excluding pathname

    i--;                    // index starts from zero, points at last char
    while ( (i >=0) && (lpBuf[i] != '\\') )
    {
        i--;
    }
    i++;                    // lpBuf[0] points to first char of filename
    strcpy(lpBuf, &lpBuf[i]);
    for (i=0; i<strlen(lpBuf); i++)
      lpBuf[i]=ENCODE(lpBuf[i]);
  }
  else {
    lpBuf[0] = '\0';        // indicate we did not get the process name
  }
}


/*----------------------------------------------------------------------
Function name:  IsBadApp

Description:    Looks up a list of bad applications and gfx card modes.

Note : See the description (above LOOKUP structure define) for a 
       description of this routines working..			   
----------------------------------------------------------------------*/
DWORD IsBadApp(DWORD dwConfig)
{
  CHAR  szBuffer[MAX_PATH];
  BADAPP *pBadApp;
  LOOKUP *pLookup ;

  GetProcessFileName(szBuffer, sizeof(szBuffer));
  if ('\0' == szBuffer[0])
    return dwConfig;

  // Loop for every bad application that we have listed.
  for (pBadApp = &BadApps[0]; pBadApp < &BadApps[sizeof(BadApps)/sizeof(BadApps[0])]; pBadApp++)
  {
    if (0 == strcmp(pBadApp->pszProcFileName, szBuffer)) 
	{
	// We have the correct application.. Make sure the card is in a mode we want to change
	 pLookup = &LookupValues[dwConfig] ;
     if (((pLookup->Chip_Count & pBadApp->Chip_Count) != 0) &&
	    ((pLookup->SLI_Mode & pBadApp->SLI_Mode) != 0) &&
	    ((pLookup->AA_Mode & pBadApp->AA_Mode) != 0))
		// Success, force a new Chip/SLI/AA mode..
	    return pBadApp->newConfig;
	}
  }

  // if we get here, there no bad applications detected (or the driver isn`t in a chip/SLI/AA mode we want to change)
  return dwConfig;
}

#ifdef RD_ABORT_ERROR
/*----------------------------------------------------------------------
Function name:  Modify_SLI_Read

Description:    
----------------------------------------------------------------------*/
void Modify_SLI_Read(NT9XDEVICEDATA * ppdev, DWORD dwRequest)
{
   HANDLE hDevice;
   DIOC_DATA DIOC_Data;

    hDevice = CreateFile(FILENAME, 0, 0, NULL, 0, 0, NULL);
   if (INVALID_HANDLE_VALUE != hDevice)
      {
      DIOC_Data.dwDevNode = _FF(DevNode);
      DeviceIoControl(hDevice, dwRequest, &DIOC_Data, sizeof(DIOC_Data), NULL, 0x0, NULL, NULL);
      }
    CloseHandle(hDevice);
}
#endif

/*----------------------------------------------------------------------
Function name:  Compute_SLIAA_Config

Description:    Determine SLI and AA configuration.
----------------------------------------------------------------------*/
void Compute_SLIAA_Config(NT9XDEVICEDATA * ppdev, FxU32 numBuffers)
{
   SstIORegs * pIORegs;
   DWORD vidProcCfg;
   DWORD m;
   DWORD n;
   DWORD k;
   DWORD pclock;
   DWORD i;
   DWORD kpow;
   DWORD pixelclock;

#ifndef SLI_ABOVE_1280
   DWORD dwKillAnalog = (4 != _FF(dwNumUnits)) ? 1 : 0;
#else 
	// Settings for analog and digital SLI operation
	// dwSLICompatibilitySettings = 0 - force single chip mode above 1280x1024.
	// dwSLICompatibilitySettings = 1 - (don't kill) allow analog mode at all resolutions.
	// dwSLICompatibilitySettings = 2 - use digital SLI at and above 1280x1024.
	
	// We may need to change the default value of dwSLICompatibilitySettings for different chip configurations.
	DWORD dwSLICompatibilitySettings;

   // Change the default for the 6000 to use Analog
   if (4 != _FF(dwNumUnits)) 
      dwSLICompatibilitySettings = RetrieveFromRegistry(ppdev, SLI_COMPATIBILITY_SETTINGS, 0, 0, 2);
   else
      dwSLICompatibilitySettings = RetrieveFromRegistry(ppdev, SLI_COMPATIBILITY_SETTINGS, 1, 0, 2);
#endif // SLI_ABOVE_1280


#ifdef WIN_CSIM
  #undef ghwIO
   SstIORegs * ghwIO = (SstIORegs *)_FF(regRealBase);
#endif

  // Read SLI and AA configuration from registry.

  switch (_FF(dwNumUnits))
  {
    // Setup default value based on number of chips.  We default to
    // maximum performance if registry value is missing.

    case 4:  // default is 4-way SLI, AA disabled for quad-chip config
             _DD(ddSLIAAConfiguration) =  RetrieveFromRegistry(ppdev, SLI_AA_CONFIG_NAME, 5, 0, 8);
             break;
    case 2:  // default is 2-way SLI, AA disabled for dual-chip config
             _DD(ddSLIAAConfiguration) =  RetrieveFromRegistry(ppdev, SLI_AA_CONFIG_NAME, 2, 0, 8);
             break;
    default: // default is SLI disabled, AA disabled for single-chip config
             _DD(ddSLIAAConfiguration) =  RetrieveFromRegistry(ppdev, SLI_AA_CONFIG_NAME, 0, 0, 8);
             break;
  }

#ifdef SLI_ABOVE_1280
	if(ANALOG_SLI_ABOVE_1280 == dwSLICompatibilitySettings)
		_DD(ddSLIAAAnalog) = !RetrieveFromRegistry(ppdev, DIGITAL_SLI_AA_NAME, 1, 0, 1);
	else
		_DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_DIGITAL;
#else 
   // Allow analog on 6000 or 4 chip board
   if (dwKillAnalog)
     _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_DIGITAL;
   else
     _DD(ddSLIAAAnalog) = !RetrieveFromRegistry(ppdev, DIGITAL_SLI_AA_NAME, 1, 0, 1);
#endif //SLI_ABOVE_1280


  // Sanity testing of config stored in registry.

  switch (_FF(dwNumUnits))
  {
    case 4:  if ((_DD(ddSLIAAConfiguration) == DUAL_CHIP_SLI_2WAY_AA_DISABLED) ||
                 (_DD(ddSLIAAConfiguration) == DUAL_CHIP_SLI_2WAY_AA_2SAMPLE)  ||            
                 (_DD(ddSLIAAConfiguration) == DUAL_CHIP_NOSLI_AA_4SAMPLE) ||
                 (SINGLE_CHIP_NOSLI_AA_2SAMPLE == _DD(ddSLIAAConfiguration)))
               {
               _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
               }
             break;

    case 2:  if ((_DD(ddSLIAAConfiguration) == QUAD_CHIP_SLI_4WAY_AA_DISABLED) ||
                 (_DD(ddSLIAAConfiguration) == QUAD_CHIP_SLI_4WAY_AA_2SAMPLE)  ||            
                 (_DD(ddSLIAAConfiguration) == QUAD_CHIP_SLI_2WAY_AA_4SAMPLE) ||
                 (_DD(ddSLIAAConfiguration) == QUAD_CHIP_NOSLI_AA_8SAMPLE) ||
                 (SINGLE_CHIP_NOSLI_AA_2SAMPLE == _DD(ddSLIAAConfiguration)))
               {
               _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
               }
             break;

    default: if ((_DD(ddSLIAAConfiguration) != SINGLE_CHIP_NOSLI_AA_DISABLED)  &&
                 (_DD(ddSLIAAConfiguration) != SINGLE_CHIP_NOSLI_AA_2SAMPLE))
             {
               // Registry specifies value for dual-chip or quad-chip, instead of single-chip.

               _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
             }
             break;
  }

  // Handle AA requested by application.

  if (DDSCAPS2_HINTANTIALIASING == _DD(ddAAModeRequested))
  {
    // Give the application the best AA we can based on the number of chips.

    switch (_FF(dwNumUnits))
    {
      case 1:
        // best 1 chip AA is 2 sample AA
        _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_2SAMPLE;
        break;
      case 2:
        // best 2 chip AA is 4 sample AA with SLI disabled
        _DD(ddSLIAAConfiguration) = DUAL_CHIP_NOSLI_AA_4SAMPLE;
        break;
      case 4:
        // best 4 chip AA is 4 sample AA with 2-way SLI
        _DD(ddSLIAAConfiguration) = QUAD_CHIP_NOSLI_AA_8SAMPLE;
        break;
    }
  }

  // Disallow SLI and AA when scanline doubling.

  vidProcCfg = GET(ghwIO->vidProcCfg);
  if (vidProcCfg & SST_HALF_MODE)
  {
    _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
  }    

  // If 2X mode then switch to from digital to analog
  // since digital cannot handle 2X mode
  if (vidProcCfg & SST_VIDEO_2X_MODE_EN)
      {
#ifdef SLI_ABOVE_1280
      // There should be no limit on the 6000.
      if (4 == _FF(dwNumUnits))
         {
         // Roll Down to 2 sample if Clock Doubling as 4 or 8 does not look right
         if ((QUAD_CHIP_NOSLI_AA_8SAMPLE == _DD(ddSLIAAConfiguration)) ||
             (QUAD_CHIP_SLI_2WAY_AA_4SAMPLE == _DD(ddSLIAAConfiguration)))
              _DD(ddSLIAAConfiguration) = QUAD_CHIP_SLI_4WAY_AA_2SAMPLE;         
         }
      else
         {
         switch (dwSLICompatibilitySettings)
            {
            case SINGLE_CHIP__ABOVE_1280:
				   if (2 == _FF(dwNumUnits))
				      _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
	   			break;

   			case ANALOG_SLI_ABOVE_1280:
               _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_ANALOG;
   				break;

   			case DIGITAL_SLI_ABOVE_1280:
	   			_DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_DIGITAL;
		   		break;
            }
		   }		
#else
      // Don't roll down to Analog as this is bad
      if (dwKillAnalog)
         {
				_DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
         }
      else
         {
         // if we only have one unit then there is no choice between analog and digital
         if (1 == _FF(dwNumUnits))
            _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
         else
            _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_ANALOG;
         }
#endif //SLI_ABOVE_1280
      }   

  
  // Determine the pixel clock
  pIORegs = (SstIORegs *)ppdev->regBase[HWINFO_SST_IOREGS_INDEX];
  pixelclock = pIORegs->pllCtrl0;
  n = ((pixelclock & 0xFF00) >> 8)+2;
  m = ((pixelclock & 0xFC) >> 2)+2;
  k = pixelclock & 0x03;

  kpow = 1;
  for (i=0; i<k; i++)
     kpow<<=1;

  // This should work to 4 GigaHertz
  pclock = 14318180*n/m/kpow;                    

  // This will need to be revisited when we do 4-way cards
  if (2 == _FF(dwNumUnits))
      {

      // Since Digital is less then analog fall back to analog
      if (SLI_AA_VIDEO_FORMAT_DIGITAL == _DD(ddSLIAAAnalog))
         {
         if (DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))
            {
            if (pclock > DUAL_CHIP_DIGITAL_SLI_CUTOFF)
               {
#ifdef SLI_ABOVE_1280
					switch (dwSLICompatibilitySettings)
					{
						case SINGLE_CHIP__ABOVE_1280:
							_DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
							break;
						case ANALOG_SLI_ABOVE_1280:
							_DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_ANALOG;
							break;
						case DIGITAL_SLI_ABOVE_1280:
							_DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_DIGITAL;
							break;
					}	
#else
               if (dwKillAnalog)
        				_DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
               else
                  _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_ANALOG;
#endif //SLI_ABOVE_1280
               }
            }
         else 
            {
            if (pclock > DUAL_CHIP_DIGITAL_AA_CUTOFF)
               {
#ifdef SLI_ABOVE_1280
					switch (dwSLICompatibilitySettings)
					{
						case SINGLE_CHIP__ABOVE_1280:
							_DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
							break;
						case ANALOG_SLI_ABOVE_1280:
							_DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_ANALOG;
							break;
						case DIGITAL_SLI_ABOVE_1280:
							_DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_DIGITAL;
							break;
					}	
#else
               if (dwKillAnalog)
         			_DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
               else
                  _DD(ddSLIAAAnalog) = SLI_AA_VIDEO_FORMAT_ANALOG;
#endif //SLI_ABOVE_1280
               }
				}
         }

      // Analog does have a limit and let's make sure we don't exceed it
      if (SLI_AA_VIDEO_FORMAT_ANALOG == _DD(ddSLIAAAnalog))
         {
         if (DUAL_CHIP_SLI_2WAY_AA_DISABLED == _DD(ddSLIAAConfiguration))
            {
            if (pclock > DUAL_CHIP_ANALOG_SLI_CUTOFF)
               _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
            }
         else 
            {
            if (pclock > DUAL_CHIP_ANALOG_AA_CUTOFF)
               _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
            }
         }
      }

  // Disable SLI and AA for single-buffered applications.
  if (numBuffers < 2)
  {
    _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
  }

  // Silently fail AA when no secondary buffers are available in !SLI modes.

  if (!RetrieveFromRegistry(ppdev, AA_ENABLE_OUTOFMEMORY_NAME, 0, 0, 1))
  {
    if ((_DD(ddSLIAAConfiguration) == SINGLE_CHIP_NOSLI_AA_2SAMPLE) && (_FF(ddSecondaryHeapSize) == 0))
    {
      _DD(ddSLIAAConfiguration) = SINGLE_CHIP_NOSLI_AA_DISABLED;
    }
    if ((_DD(ddSLIAAConfiguration) == DUAL_CHIP_NOSLI_AA_4SAMPLE) && (_FF(ddSecondaryHeapSize) == 0))
    {
      _DD(ddSLIAAConfiguration) = DUAL_CHIP_SLI_2WAY_AA_2SAMPLE;
    }
    if ((_DD(ddSLIAAConfiguration) == QUAD_CHIP_NOSLI_AA_8SAMPLE) && (_FF(ddSecondaryHeapSize) == 0))
    {
      _DD(ddSLIAAConfiguration) = QUAD_CHIP_SLI_2WAY_AA_4SAMPLE;
    }
  }

  // USAF for GameGauge does not work right in SLI mode so don't let it happen
  _DD(ddSLIAAConfiguration) = IsBadApp(_DD(ddSLIAAConfiguration));

#ifdef STEREO
	
	//default to 2x AA if 4x AA is requested on 2 chip config
	if( _FF(ddStereoHeapFactor) && (_DD(ddSLIAAConfiguration) == DUAL_CHIP_NOSLI_AA_4SAMPLE) && (_FF(dwNumUnits) == 2) )
		_DD(ddSLIAAConfiguration) = DUAL_CHIP_SLI_2WAY_AA_2SAMPLE;
	
	//default to 4x AA if 8x AA is requested on 4 chip config
	if( _FF(ddStereoHeapFactor) && (_DD(ddSLIAAConfiguration) == QUAD_CHIP_NOSLI_AA_8SAMPLE) && (_FF(dwNumUnits) == 4) )
		_DD(ddSLIAAConfiguration) = QUAD_CHIP_SLI_2WAY_AA_4SAMPLE;

#endif




  // Setup SLI and AA configuration.
  switch (_DD(ddSLIAAConfiguration))
  {
    case SINGLE_CHIP_NOSLI_AA_DISABLED:  _DD(ddAAModeRequested)  = 0;
                                         _DD(ddAANumberSamples)  = 0;
                                         _DD(ddSLIModeRequested) = 0;
                                         _DD(ddSLINumberWays)    = 0;
                                         break;
    case SINGLE_CHIP_NOSLI_AA_2SAMPLE:   _DD(ddAAModeRequested)  = 1;
                                         _DD(ddAANumberSamples)  = 2;
                                         _DD(ddSLIModeRequested) = 0;
                                         _DD(ddSLINumberWays)    = 0;
                                         break;
    case DUAL_CHIP_SLI_2WAY_AA_DISABLED: _DD(ddAAModeRequested)  = 0;
                                         _DD(ddAANumberSamples)  = 0;
                                         _DD(ddSLIModeRequested) = 1;
                                         _DD(ddSLINumberWays)    = 2;
                                         break;
    case DUAL_CHIP_SLI_2WAY_AA_2SAMPLE:  _DD(ddAAModeRequested)  = 0;
                                         _DD(ddAANumberSamples)  = 2;
                                         _DD(ddSLIModeRequested) = 0;
                                         _DD(ddSLINumberWays)    = 0;
                                         break;
    case DUAL_CHIP_NOSLI_AA_4SAMPLE:     _DD(ddAAModeRequested)  = 1;
                                         _DD(ddAANumberSamples)  = 4;
                                         _DD(ddSLIModeRequested) = 0;
                                         _DD(ddSLINumberWays)    = 0;
                                         break;
    case QUAD_CHIP_SLI_4WAY_AA_DISABLED: _DD(ddAAModeRequested)  = 0;
                                         _DD(ddAANumberSamples)  = 0;
                                         _DD(ddSLIModeRequested) = 1;
                                         _DD(ddSLINumberWays)    = 4;
                                         _DD(ddSLIAAAnalog) = 1;
                                         break;
    case QUAD_CHIP_SLI_4WAY_AA_2SAMPLE:  _DD(ddAAModeRequested)  = 0;
                                         _DD(ddAANumberSamples)  = 2;
                                         _DD(ddSLIModeRequested) = 1;
                                         _DD(ddSLINumberWays)    = 2;
                                         _DD(ddSLIAAAnalog) = 1;
                                         break;
    case QUAD_CHIP_SLI_2WAY_AA_4SAMPLE:  _DD(ddAAModeRequested)  = 0;
                                         _DD(ddAANumberSamples)  = 4;
                                         _DD(ddSLIModeRequested) = 0;
                                         _DD(ddSLINumberWays)    = 0;
                                         _DD(ddSLIAAAnalog) = 1;
                                         break;
    case QUAD_CHIP_NOSLI_AA_8SAMPLE:     _DD(ddAAModeRequested)  = 1;
                                         _DD(ddAANumberSamples)  = 8;
                                         _DD(ddSLIModeRequested) = 0;
                                         _DD(ddSLINumberWays)    = 0;
                                         _DD(ddSLIAAAnalog) = 1;
                                         break;
  }

   TwoPpcInit(ppdev);

} // Compute_SLIAA_Config


/*----------------------------------------------------------------------
Function name: SwitchToHostCursor

Description:   Calls 16-bit cursor SwitchToHostCursor to switch to Host Based Cursor

Return:        FALSE - failure
               TRUE  - success
----------------------------------------------------------------------*/
DWORD SwitchToHostCursor(NT9XDEVICEDATA * ppdev)
{
   // QT_Thunk is documented in Windows 95 System Programming
   //   Secrets, Pietrek.  pp. 191-195, 208-211.
   // NOTE that QT_Thunk will die without fixing up EBP.  I
   //   don't know exactly how big the stack should be but
   //   it appears the more the 16:16 uses, the larger it must be!

   DWORD Addr1616;
   DWORD dwReturn;

   // Get 16:16 function address from shared data
   Addr1616 = _FF(dwSwitchCursor);

   // QT_Thunk will do all the fixups and pass control to
   //   the 16:16 in edx.
   _asm
      {
      mov   edx,Addr1616     ; QT_Thunk expects 16:16 proc addr in EDX
      sub   ebp,200h         ; QT_Thunk builds convoluted 16:16 stack frame
      call  QT_Thunk
      add   ebp,200h         ; QT_Thunk stack stuff
      and   eax,0ffffh       ; mask off high word
      mov   dwReturn, eax
      }

    return dwReturn;                  // return code in eax
}

/*----------------------------------------------------------------------
Function name:  MoveTileMark

Description:	Move tile mark to provide extra memory in SLI mode.
----------------------------------------------------------------------*/

void MoveTileMark(NT9XDEVICEDATA * ppdev)
{
  FXSURFACEDATA *surfaceData;
  DWORD         lfbMemoryConfig;

  // Move tiled mark to start of tiled heap.

  _FF(ddTileMark) = _FF(ddTiledHeapStart);

  if (!(_DD(ddAAModeEnabled)) && _DD(ddSLIModeEnabled))
  {
    // Move tile mark past extra memory.

    _FF(ddTileMark) += _FF(ddExtraMemorySize);
  }

  // Initialize lfbMemoryTileCtrl for tiled modes.

  lfbMemoryConfig = GET(ghwIO->lfbMemoryConfig);
  lfbMemoryConfig &= ~0x01801FFFL;
  lfbMemoryConfig |= ((_FF(ddTileMark) >> 12L) & 0x1FFFL);
  lfbMemoryConfig |= ((_FF(ddTileMark) >> 25L) & 0x0003L) << 23;
  SETDW(ghwIO->lfbMemoryConfig, lfbMemoryConfig );

  // Get surface data for primary.

  surfaceData = (FXSURFACEDATA*) &(_FF(ddPrimarySurfaceData));

  // Adjust lfbPtr for SLI mode.

  surfaceData->lfbPtr = AdjustLfbPtr(ppdev, surfaceData->lfbPtr, &surfaceData->heapID);

  // Compute hwPtr from lfbPtr.

  surfaceData->hwPtr = LfbPtrToHwPtr(ppdev, surfaceData->lfbPtr, surfaceData->heapID);

  // Update desktop start.

  _FF(gdiDesktopStart) = surfaceData->hwPtr;

  // Update DIB engine pointer.

  ((DIBENGINE FAR *)_FF(lpPDevice32))->deBitsOffset = surfaceData->lfbPtr;
}

#endif // SLI_AA


/*----------------------------------------------------------------------
Function name: TwoPpcInit

Description:   Function to init Two Ppc BandHeight

Return:        
               log 2 Band Height
Note:
     This was added due to problems that I was having with the NMS
 file.
----------------------------------------------------------------------*/
DWORD TwoPpcInit(NT9XDEVICEDATA * ppdev)
{
   DWORD dwBandHeight;
   if( NULL == GETENV(TWOPPC_LOG2_BAND_HEIGHT) )
     {
        // Band Height is 1 line by default.
        // Another Scott Seller Optimization
        if ((4 == _FF(dwNumUnits)) && ( _FF(vres) >= 768 ))
            dwBandHeight=3;
        else
            dwBandHeight=2;
     }
   else
     {
       dwBandHeight = RetrieveFromRegistry(ppdev, TWOPPC_LOG2_BAND_HEIGHT, 0, 0, 7);
     }
   return dwBandHeight;
}
