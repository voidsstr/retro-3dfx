/* $Header: ddovl32.c, 36, 10/11/00 8:43:56 PM, Brent$ */
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
** File Name: 	DDOVL32.C
**
** Description: DirectDraw overlay support.
**
** $Revision: 36$
** $Date: 10/11/00 8:43:56 PM$
**
*/

/*******************************************************************************
*
* DIRECTDRAW FUNCTIONS:
*
* SetOverlayPosition32      --- DirectDraw SetOverlayPosition entry point.
* UpdateOverlay32           --- DirectDraw UpdateOverlay entry point.
*
* EXPORT FUNCTIONS:
*
* CanCreateOverlaySurface   --- Check overlay surface pixel format
* CreateOveralaySurface     --- Allocate overlay and its shrink surfaces
* DestroyOveralaySurface    --- Free overlay shrink surface
* UnlockOverlaySurface      --- Shrink overlay if needed
*
* PRIVATE FUNCTIONS:
*
* SetBltParams              --- Sets up Blt parameters.
* Get_ScalingFactor         --- Calculates scale factor.
* ShrinkOverlaySurface      --- Shrink blt overlay surface
*
*******************************************************************************/

#include "precomp.h"

#ifndef WINNT
#include "ddvpe32.h"
#endif

#ifndef WINNT
#define VBIHeight   MM_DD(dwVBIHeight)
#endif  // ifdef WINNT

#if (defined(WINNT) && (_WIN32_WINNT >= 0x0500))
#define VBIHeight   _DD(dwVBIHeight)
#endif

/*******************************************************************/
/*                     DIRECTDRAW FUNCTIONS                        */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name:  SetOverlayPosition32

Description:    DirectDraw SetOverlayPosition entry point.

                Changes the display coordinates of an overlay surface

Return:         DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/
DWORD __stdcall SetOverlayPosition32( LPDDHAL_SETOVERLAYPOSITIONDATA psopd )
{
  WORD  wDesktopW, wDesktopH;
  WORD  wOvlScrX, wOvlScrY;
  WORD  wOvlScrEndX, wOvlScrEndY;
  DWORD vidOverlayStartAddr, vidOverlayEndAddr;
  DD_ENTRY_SETUP(psopd->lpDD);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "SetOverlayPosition32" ));
  #endif

  //Primary surface/desktop width and height
  // nt declares wWidth & wHeight as DWORDs and win9x declares them as WORDs
  // added casts to clean up nt compile and should have no effect on win9x
  wDesktopW = (WORD)psopd->lpDDDestSurface->lpGbl->wWidth;
  wDesktopH = (WORD)psopd->lpDDDestSurface->lpGbl->wHeight;

  //Overlay screen top, left
#ifndef WINNT
  wOvlScrX = min(psopd->lXPos, wDesktopW);
  wOvlScrY = min(psopd->lYPos, wDesktopH);
#else
  wOvlScrX = (WORD)min(psopd->lXPos, wDesktopW);
  wOvlScrY = (WORD)min(psopd->lYPos, wDesktopH);
#endif

  //Save the new overlay coordinates in video destination rectangle struct
  rOvlDst.right = wOvlScrX + rOvlDst.right - rOvlDst.left;
  rOvlDst.bottom= wOvlScrY + rOvlDst.bottom - rOvlDst.top;
  rOvlDst.left  = wOvlScrX;
  rOvlDst.top   = wOvlScrY;

  // Fix for PRS 12592, we dec the overlay height by 1 move the bottom up by 1 line
  // rVidDst.bottom-1;  jmccartney 09/06/00
  //Overlay screen bottom, right
#ifndef WINNT
  wOvlScrEndX = min(rOvlDst.right, wDesktopW-1);
  wOvlScrEndY = min(rOvlDst.bottom-1, wDesktopH-1);
#else
  wOvlScrEndX = (WORD)min(rOvlDst.right, wDesktopW-1);
  wOvlScrEndY = (WORD)min(rOvlDst.bottom-1, wDesktopH-1);
#endif

  //Set the overlay start and end x,y coordinates on the desktop
  GETOVERLAYADDR(wOvlScrX, wOvlScrY, vidOverlayStartAddr );
  GETOVERLAYADDR(wOvlScrEndX, wOvlScrEndY, vidOverlayEndAddr );

  SETDW(ghwIO->vidOverlayStartCoords, vidOverlayStartAddr);
  SETDW(ghwIO->vidOverlayEndScreenCoord, vidOverlayEndAddr);

  psopd->ddRVal = DD_OK;
  return DDHAL_DRIVER_HANDLED;

} // SetOverlayPosition32


/*----------------------------------------------------------------------
Function name: 	UpdateOverlay32

Description:    DirectDraw UpdateOverlay entry point.

                Repositions or modifies the visual attributes of an
                overlay surface

Return:         DDHAL_DRIVER_HANDLED
				DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

DWORD __stdcall UpdateOverlay32( LPDDHAL_UPDATEOVERLAYDATA puod )
{
  // can't use PDEV_DECL here since CMDFIFO_PROLOG needs ppdev initialized
#ifdef WINNT
    PDEV  *ppdev = puod->lpDD->dhpdev;
#else
    NT9XDEVICEDATA *ppdev = (NT9XDEVICEDATA *)puod->lpDD->dwReserved3;
#endif
    WORD  wDesktopW, wDesktopH;
    WORD  wOvlScrX, wOvlScrY;
    WORD  wOvlScrEndX, wOvlScrEndY;
    RECT  rVidSrcRgn, rVidDst;
    DWORD vidOverlayStartAddr, vidOverlayEndAddr, vidDesktopOverlayStride;
    DWORD vidOverlayDudx, vidOverlayDudxOffsetSrcWidth, vidOverlayDvdy, vidOverlayDvdyOffset;
    DWORD vidChroma, vidProcCfg;
    DWORD overlayAddr, overlayBaseAddr, pixelFormat;
    DWORD dwDstW, dwDstH, dwSrcW, dwSrcH;
    FXSURFACEDATA* surfaceData;
    FXSURFACEDATA* shrinkSurfData;
    DWORD lPitch;
#if ENABLE_VIDEOPORT
    KMVTBUFF *lpKMBuff = (KMVTBUFF *)_FF(KMVTBuff);
#endif


#ifndef WINNT
   WORD * pFlags = (WORD *)_FF(lpDeFlags);
   WORD SaveBusy = *pFlags;
   WORD TrashBusy = Set_Busy(pFlags);
#endif
    CMDFIFO_PROLOG(hwPtr);

#if !defined(WINNT) && !defined(GBLDATA_IN_PDEV)
    if (LOW_POWER_MODE(SaveBusy))
      {
      puod->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;
      }
    ppdev->gdiFlags |= SDATA_GDIFLAGS_2D_DIRTY;
#endif

    #ifdef FXTRACE
    DISPDBG((ppdev, DEBUG_APIENTRY, "UpdateOverlay32" ));
    #endif

    overlayBaseAddr = GET_HW_ADDR(puod->lpDDSrcSurface);
    surfaceData = (FXSURFACEDATA*)puod->lpDDSrcSurface->lpGbl->dwReserved1;
#ifndef WINNT
   _FF(ovlCurAddr) = overlayBaseAddr;
#endif
   VSYNC_IRQ_DISABLE;		// as quick as possible for freeze
   _DS(dwOvlOffset) = 0;
//#endif

    //Hide overlay
    if(puod->dwFlags & DDOVER_HIDE)
    {
      if( _FF(ddVisibleOverlaySurf) == overlayBaseAddr)
      {
        OVERLAYDISABLE;
        _FF(ddVisibleOverlaySurf) = 0;
        _FF(lastOverlayAddress) = INVALID_ADDRESS;
        _DS(dwOvlOffset) = 0;
        VSYNC_IRQ_DISABLE;

        puod->ddRVal = DD_OK;
#ifndef WINNT
        _FF(ovlYScale) = 0;
        _FF(ovlXScale) = 0;
        RESTORE_BUSY(pFlags,SaveBusy);
#endif
        return DDHAL_DRIVER_HANDLED;
      }
      puod->ddRVal = DD_OK;
#ifndef WINNT
      RESTORE_BUSY(pFlags,SaveBusy);
#endif
      return DDHAL_DRIVER_NOTHANDLED;
    }
    else if ((DDOVER_SHOW & puod->dwFlags) ||
             (_FF(ddVisibleOverlaySurf) == overlayBaseAddr))
    {
      //Show overlay
      //

#ifndef WINNT
	  // MDM-Richardson - Compaq SoftDVD and GraphEdit try to do an UpdateOverlay32 with a SHOW
	  // while we are in a dos box.  This is not valid.  So we return a SURFACELOST
	  // Global flag for _FF(DosActive) is only in the 9x driver
	  if( _FF(DosActive) )
      {
	     puod->ddRVal = DDERR_SURFACELOST;

          RESTORE_BUSY(pFlags,SaveBusy);

          return DDHAL_DRIVER_HANDLED;
      }

      // MDM-Richardson flag needed for Overlay management with a dos
      // box. PRS 7643 & 7902
      // When we get a show command, reset the RelaxedOverlayOwnerMode
      // to zero.  This means that we have gotten to a point where
      // we have recreated all the lost surfaces that going to a
      // fullscreen dos caused.
      _FF(dwRelaxedOverlayOwnerMode) = 0;
#endif

      if( _FF(ddVisibleOverlaySurf) == 0 )
      {
        _FF(ddVisibleOverlaySurf) = overlayBaseAddr;
      }
      else if( _FF(ddVisibleOverlaySurf) != overlayBaseAddr)
      {
        puod->ddRVal = DDERR_OUTOFCAPS;
#ifndef WINNT
        RESTORE_BUSY(pFlags,SaveBusy);
#endif
        return DDHAL_DRIVER_HANDLED;
      }

      //This rectangle represents a region on the source surface to be overlaid on destination
      rVidSrcRgn.left   = (WORD)puod->rSrc.left;
      rVidSrcRgn.top    = (WORD)puod->rSrc.top;
      rVidSrcRgn.right  = (WORD)puod->rSrc.right;
      rVidSrcRgn.bottom = (WORD)puod->rSrc.bottom;

      //This rectangle represents a region on the destination surface where overlay is mapped on
      rVidDst.left   = (WORD)puod->rDest.left;
      rVidDst.top    = (WORD)puod->rDest.top;
      rVidDst.right  = (WORD)puod->rDest.right;
      rVidDst.bottom = (WORD)puod->rDest.bottom;

#if ENABLE_VIDEOPORT
      //If autoflipping then VideoPort is in use
      if( puod->dwFlags & DDOVER_AUTOFLIP )
      {
        RECT rNewSrcRgn;
        _DD(fUpdateOverlay) = TRUE;

        surfaceData->overlayShrinkFlag = FALSE;

        VidSrcData.rVidSrc.left   = VidSrcData.rVidSrc.top = 0;
        VidSrcData.rVidSrc.right  = (DWORD)puod->lpDDSrcSurface->lpGbl->wWidth;
        VidSrcData.rVidSrc.bottom = (DWORD)puod->lpDDSrcSurface->lpGbl->wHeight;

        if ( (puod->dwFlags & DDOVER_BOB) &&
             (puod->dwFlags & DDOVER_INTERLEAVED))
        {
           if(!(VidInData.WeaveDeinterlacing & BOB_INTERLEAVED))
           {
               DWORD dwReg;
               //change vidoe port setting
  		        dwReg = GET(ghwIO->vidInFormat) & (~H3_VMI_DEINTERLACE_WEAVE);
                SETDW(ghwIO->vidInFormat, dwReg);
				lpKMBuff->dwVidInFormat = dwReg;
				VBIHeight = lpKMBuff->dwVBILines = lpKMBuff->dwVBILinesOrig;
				lpKMBuff->dwVideoLines = lpKMBuff->dwVideoLinesOrig;
                if(!(VidInData.WeaveDeinterlacing & CAN_SIMULATE))
                    VidInData.WeaveDeinterlacing |= BOB_INTERLEAVED;
          }
		  else if(VidInData.WeaveDeinterlacing & CAN_SIMULATE)		// this deinterlace turned off later in routine
		  {
				 VBIHeight = lpKMBuff->dwVBILines = lpKMBuff->dwVBILinesOrig;
				 lpKMBuff->dwVideoLines = lpKMBuff->dwVideoLinesOrig;
		  }

        }
        else
        {
          if(!(VidInData.WeaveDeinterlacing & CAN_SIMULATE))
          {
            if((VidInData.WeaveDeinterlacing & BOB_INTERLEAVED))
            {
                if(VidInData.WeaveDeinterlacing & INTERLEAVED_VIDEO)
                {
                DWORD dwReg;
                //change back to interleaved mode
    		    dwReg = GET(ghwIO->vidInFormat) | H3_VMI_DEINTERLACE_WEAVE;
                SETDW(ghwIO->vidInFormat, dwReg);
				lpKMBuff->dwVidInFormat = dwReg;
				VBIHeight = lpKMBuff->dwVBILines = (lpKMBuff->dwVBILinesOrig << 1);
				lpKMBuff->dwVideoLines = (lpKMBuff->dwVideoLinesOrig << 1);
                }
            }
          }
          VidInData.WeaveDeinterlacing &= ~BOB_INTERLEAVED;
        }

        VidSrcData.rVidSrcRgn = rVidSrcRgn;

        H3_VMI_SetDecimation (ppdev, &rVidDst, &rNewSrcRgn);

        dwSrcW = rNewSrcRgn.right - rNewSrcRgn.left;
        dwSrcH = rNewSrcRgn.bottom - rNewSrcRgn.top;
        dwDstW = VidDstData.rVidDst.right-VidDstData.rVidDst.left;
        dwDstH = VidDstData.rVidDst.bottom-VidDstData.rVidDst.top;

        if(IS_TILED(overlayBaseAddr))
            _DS(dwOvlOffset) = rNewSrcRgn.left * 2 + rNewSrcRgn.top * (DWORD)_DS(ddTileStride);
        else
            _DS(dwOvlOffset) = rNewSrcRgn.left * 2 + rNewSrcRgn.top *
#ifndef WINNT
                    (DWORD)surfaceData->lPitch;
#else
                    (DWORD)puod->lpDDSrcSurface->lpGbl->lPitch;
#endif
      }
      else
#endif //ENABLE_VIDEOPORT
      {
#if ENABLE_VIDEOPORT
       if((puod->lpDDSrcSurface->ddsCaps.dwCaps & DDSCAPS_VIDEOPORT) &&
            (VidInData.dwVPEFlags & VPORT_STOP))
       {
            rVidSrcRgn.right = (rVidSrcRgn.right * VidInData.dwXScale) >> 16;
            rVidSrcRgn.left = (rVidSrcRgn.left  * VidInData.dwXScale) >> 16;
            rVidSrcRgn.top = (rVidSrcRgn.top * VidInData.dwYScale) >> 16;
            rVidSrcRgn.bottom = (rVidSrcRgn.bottom  * VidInData.dwYScale) >> 16;
			//V3TV
			if (_DD(bVideoPortActive))
			{
				rVidSrcRgn = _DD(rWDMSrc);
			}
			if (_DD(fWDMVXDActive) && _DD(bUseWDMScaling) && _DD(bVideoPortActive))
			{
				DWORD dwReg;

				//turn h decimation off if wdm scaling in use
	   			dwReg = GET(ghwIO->vidInFormat);
				if _DD(bScaleWDMinUse)
					dwReg &= ~(H3_VMI_HDECIMATION_MASK);
				else
					dwReg |= (H3_VMI_HDECIMATION_MASK);
				dwReg |= (H3_VMI_VDECIMATION_MASK);
       			SETDW(ghwIO->vidInFormat, dwReg);	
			} //end V3TV

       }
#endif
       VidInData.dwBpp = 2;
#ifndef WINNT
      // Note that unlike Windows 95, Windows NT always guarantees that
      // there will be a valid 'ddpfSurface' structure
       if(puod->lpDDSrcSurface->dwFlags & DDRAWISURF_HASPIXELFORMAT)
       {
#endif
        if ((puod->lpDDSrcSurface->lpGbl->ddpfSurface.dwFlags & DDPF_RGB)
             && IS_NAPALM
             && (puod->lpDDSrcSurface->lpGbl->ddpfSurface.dwRGBBitCount == 32))
                VidInData.dwBpp = 4;
#ifndef WINNT
        }
        else  if((GETPRIMARYBYTEDEPTH == 4) && IS_NAPALM)
                VidInData.dwBpp = 4;
#endif

        VidSrcData.rVidSrc = VidSrcData.rMaxVidSrc = rVidSrcRgn;
        dwSrcW = rVidSrcRgn.right - rVidSrcRgn.left;
        dwSrcH = rVidSrcRgn.bottom - rVidSrcRgn.top;

        rOvlDst = rVidDst;       //Save the overlay destination rectangle
        dwDstW = rVidDst.right - rVidDst.left;
        dwDstH = rVidDst.bottom - rVidDst.top;

        _DS(dwOvlOffset) = 0;

        if ( surfaceData == NULL )
        {
          puod->ddRVal = DD_OK;
    #ifndef WINNT
          RESTORE_BUSY(pFlags,SaveBusy);
    #endif
          return DDHAL_DRIVER_NOTHANDLED;
        }

#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
        //Native size or stretching
        if ( (dwDstW >= dwSrcW) && (dwDstH >= dwSrcH) )
        {
          surfaceData->overlayShrinkFlag = FALSE;
        }
        else //Shrinking
        {
          surfaceData->overlayShrinkFlag = TRUE;
          surfaceData->doShrink = TRUE;

          if(_DD(overlaySurfaceCnt) >= 2)
          {
            (WORD)(_FF(dwCurrentSKSurf)) +=1;       //change current surface
          }

          if(_FF(dwCurrentSKSurf) & 1 )
             shrinkSurfData = _DD(shrinkSurface2);
          else
             shrinkSurfData = _DD(shrinkSurface1);

          overlayBaseAddr = shrinkSurfData->hwPtr;

          if ( dwDstW < dwSrcW )
            surfaceData->shrinkWidth = dwDstW+1;  //Overfly shrink fix
          else
            surfaceData->shrinkWidth = dwSrcW;

          if ( dwDstH < dwSrcH )
              surfaceData->shrinkHeight = dwDstH;
          else
              surfaceData->shrinkHeight = dwSrcH;

          surfaceData->shrinkSrcWidth  = dwSrcW;
          surfaceData->shrinkSrcHeight = dwSrcH;
          surfaceData->shrinkSrcX = rVidSrcRgn.left;
          surfaceData->shrinkSrcY = rVidSrcRgn.top;
        }
#endif

        if ( dwSrcW ) --dwSrcW;
        if ( dwSrcH ) --dwSrcH;
      }

#ifndef WINNT
      //update scale factors for TVPCI
      if( dwSrcH & dwSrcW)
      {
        _FF(ovlYScale) = (dwDstH << 16) / dwSrcH;
        _FF(ovlXScale) = (dwDstW << 16) / dwSrcW;
      }
      else
      {
          _FF(ovlYScale) =  0;
          _FF(ovlXScale) =  0;
      }
#endif
      //Primary surface/desktop width and height
      // nt declares wWidth & wHeight as DWORDs and win9x declares them as WORDs
      // added casts to clean up nt compile and should have no effect on win9x
      wDesktopW = (WORD)puod->lpDDDestSurface->lpGbl->wWidth;
      wDesktopH = (WORD)puod->lpDDDestSurface->lpGbl->wHeight;

      //Overlay screen top, left
#ifndef WINNT
      wOvlScrX = min(rVidDst.left, wDesktopW);
#else
      wOvlScrX = (WORD)min(rVidDst.left, wDesktopW);
#endif

     if(!IS_NAPALM)     //NAPALM seems don't have this problem
      //Patch to fix 2 pixel column garbage appears on right side when overlay is full screen
      if ( !wOvlScrX && (dwDstW >= (DWORD)(wDesktopW-1)) )
      {
#ifndef WINNT
         if ( (8 != _FF(bpp)) || !(puod->dwFlags & (DDOVER_KEYDEST|DDOVER_KEYDESTOVERRIDE)) )
#else
         if ( (8 != ppdev->cBitsPerPel) || !(puod->dwFlags & (DDOVER_KEYDEST|DDOVER_KEYDESTOVERRIDE)) )
#endif
            wOvlScrX = 1;
      }

#ifndef WINNT
      wOvlScrY = min(rVidDst.top, wDesktopH);
#else
      wOvlScrY = (WORD)min(rVidDst.top, wDesktopH);
#endif
      GETOVERLAYADDR( wOvlScrX, wOvlScrY, vidOverlayStartAddr);


	  // Fix for PRS 12592, we dec the overlay height by 1 move the bottom up by 1 line
	  // rVidDst.bottom-1;  jmccartney 09/06/00
      //Overlay screen bottom, right
#ifndef WINNT
      wOvlScrEndX = min(rVidDst.right, wDesktopW-1);
      wOvlScrEndY = min(rVidDst.bottom-1, wDesktopH-1);
#else
      wOvlScrEndX = (WORD)min(rVidDst.right, wDesktopW-1);
      wOvlScrEndY = (WORD)min(rVidDst.bottom-1, wDesktopH-1);
#endif
      GETOVERLAYADDR( wOvlScrEndX, wOvlScrEndY, vidOverlayEndAddr);

      //Set scaling register in x direction, 0 Dudx offset, SrcWidth
      vidOverlayDudx = Get_ScalingFactor(dwSrcW, dwDstW, 20);

      //Set the right source width to fetch
#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
      if ( !(puod->dwFlags & DDOVER_AUTOFLIP) && (surfaceData->overlayShrinkFlag == TRUE) )
         dwSrcW = surfaceData->shrinkWidth;
#endif
       vidOverlayDudxOffsetSrcWidth = dwSrcW;		//set it in the first step
 #if (USE_NT5_DDMEMMGR) || !defined(WINNT)
      //If not using videoport, interleaved BOB and not shrinking: SrcH is halved
      if ( (puod->dwFlags & DDOVER_BOB) && (puod->dwFlags & DDOVER_INTERLEAVED))
      {

         if( (puod->dwFlags & DDOVER_AUTOFLIP) ||
            (surfaceData->overlayShrinkFlag != TRUE))
         {

            dwSrcH >>= 1;
            if ( dwSrcH > 0 )
               --dwSrcH;

         }
      }
#endif

      //Set scaling register in y direction, 0 Dvdy offset
	  // take away 1 from dwDstH to fix a problem were our overlays
	  // are not the same height as software ones on competitors cards
	  // related to PRS Issue 12592 - jmccartney 07/07/00
      vidOverlayDvdy = Get_ScalingFactor(dwSrcH, dwDstH-1, 20);

      vidOverlayDvdyOffset = 0;

      //Set video processor config register and turn on video processor
      vidProcCfg = GET(ghwIO->vidProcCfg);                             //Video processor config reg
      vidProcCfg &= ~( SST_OVERLAY_STEREO_EN    |  // Disable stereo
                  SST_USE_ALPHA_BIT             |  // No Alpha bit (was interlaced_en in h3)
                  SST_CHROMA_EN                 |  // Enable chromakeying  need this??? -SS
                  SST_CHROMA_INVERT             |  // Disable chromakey result inversion need this??? -SS
                  SST_OVERLAY_EN                |  // Fetch overlay
                  SST_VIDEOIN_AS_OVERLAY        |  // Autoflipping overlay enable
                  SST_OVERLAY_CLUT_BYPASS       |  // No overlay clut bypass
               //   SST_OVERLAY_CLUT_SELECT       |  // Overlay lower clut select
                  SST_OVERLAY_HORIZ_SCALE_EN    |  // Enable horizontal scaling
                  SST_OVERLAY_VERT_SCALE_EN     |  // Enable vertical scaling
                  SST_OVERLAY_FILTER_MODE       |  // Use bilinear scaling
                  SST_OVERLAY_PIXEL_FORMAT      |  // Select UYVY422
                  SST_OVERLAY_TILED_EN          |  // Select overlay linear space
                  SST_OVERLAY_DEINTERLACE_EN    |  // Disable backend deinterlace
                  0 );
      vidProcCfg |=
                  SST_OVERLAY_EN                |
                  //SST_VIDEOIN_AS_OVERLAY        |
                  SST_OVERLAY_CLUT_SELECT       |  //always use overlay clut
                  SST_OVERLAY_HORIZ_SCALE_EN    |
                  SST_OVERLAY_VERT_SCALE_EN     |
                  SST_OVERLAY_FILTER_BILINEAR   |    //Bilinear scaling
                  0;

      // fix for bugs #1753, 1793 & 1808
      // bilinear filtering is not supported when 2x mode enabled
      if (SST_VIDEO_2X_MODE_EN & vidProcCfg)
      {
        // revert to point sample filtering in 2x mode
        vidProcCfg &= ~SST_OVERLAY_FILTER_MODE;   // clear filter mode bits
        vidProcCfg |= SST_OVERLAY_FILTER_POINT;   // set point filter mode
      }

      // in 8bpp modes, use the upper 256 clut entries
      // fixes bugs #1507 & #1573
     // if (1 == GETPRIMARYBYTEDEPTH)
     //   vidProcCfg |= SST_OVERLAY_CLUT_SELECT;   overlay clut is always used

      //////////////////Overlay HW address to use/////////////////
      overlayAddr = overlayBaseAddr;

      //Set desktop and overlay stride register (overlay stride must be the same w/ vidInStride)
#if ENABLE_TILED_HEAP
      if(IS_TILED(overlayBaseAddr))
      {
        DWORD inX, inY, tileInX, tileInY, resX, resY;
        vidProcCfg |= SST_OVERLAY_TILED_EN;

        vidDesktopOverlayStride = GET(ghwIO->vidDesktopOverlayStride);
        vidDesktopOverlayStride &= ~SST_OVERLAY_LINEAR_STRIDE;
        vidDesktopOverlayStride |= (_DS(ddTileStride) << SST_OVERLAY_STRIDE_SHIFT) & SST_OVERLAY_TILE_STRIDE;

        if(surfaceData->overlayShrinkFlag != TRUE)
        {
          inY = rVidSrcRgn.top;
          inX = rVidSrcRgn.left * 2;

          tileInY = inY >> 5L;
          tileInX = inX >> 7L;

          resY = inY - (tileInY << 5L);
          resX = inX - (tileInX << 7L);

          overlayAddr = overlayBaseAddr & ~SSTG_IS_TILED;
          overlayAddr += ( (tileInY * _DS(ddTileStride) + tileInX) << 12L ) + (resY << 7L) + resX;
          overlayAddr |= SSTG_IS_TILED;
        }
      }
      else
#endif
      {
        vidDesktopOverlayStride = GET(ghwIO->vidDesktopOverlayStride);
        vidDesktopOverlayStride &= ~SST_OVERLAY_LINEAR_STRIDE;

        //If not shrinking and not autoflipping, check for interleaved BOB:

#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
        if(surfaceData->overlayShrinkFlag != TRUE)
            lPitch = surfaceData->lPitch;
        else
          lPitch =  shrinkSurfData->lPitch;
#else
        lPitch = puod->lpDDSrcSurface->lpGbl->lPitch;       //NT4
#endif


#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
        if ( (surfaceData->overlayShrinkFlag != TRUE) && !(puod->dwFlags & DDOVER_AUTOFLIP) &&
             (puod->dwFlags & DDOVER_BOB) && (puod->dwFlags & DDOVER_INTERLEAVED) ) //Interleaved BOB: stride * 2
        {
          vidDesktopOverlayStride |= (lPitch << (SST_OVERLAY_STRIDE_SHIFT+1)) & SST_OVERLAY_LINEAR_STRIDE;
        }
        else
#endif
        {
          vidDesktopOverlayStride |= (lPitch << SST_OVERLAY_STRIDE_SHIFT) & SST_OVERLAY_LINEAR_STRIDE;
        }

        if(surfaceData->overlayShrinkFlag != TRUE)
        {
          overlayAddr = overlayBaseAddr + rVidSrcRgn.left * VidInData.dwBpp + rVidSrcRgn.top * lPitch;
        }
      }
#ifndef WINNT
      overlayAddr &= 0xfffffff8L;
#endif

      ///////////////////Save up overlay HW address to use////////////////////
      _FF(lastOverlayAddress)  =  overlayAddr & (~SSTG_IS_TILED);
      surfaceData->overlayAddr =  overlayAddr;

      //Set the colorkey register if KEYDEST or KEYDESTOVERRIDE is set
      if( puod->dwFlags & DDOVER_KEYDEST )
      {
         vidProcCfg |= SST_CHROMA_EN;
         vidChroma = puod->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;
      }
      else if( puod->dwFlags & DDOVER_KEYDESTOVERRIDE )
      {
         vidProcCfg |= SST_CHROMA_EN;
         vidChroma = puod->overlayFX.dckDestColorkey.dwColorSpaceLowValue;
      }
      //Overlay pixel format
#ifndef WINNT
      // Note that unlike Windows 95, Windows NT always guarantees that
      // there will be a valid 'ddpfSurface' structure
      if(puod->lpDDSrcSurface->dwFlags & DDRAWISURF_HASPIXELFORMAT)
      {
#endif
        if (puod->lpDDSrcSurface->lpGbl->ddpfSurface.dwFlags & DDPF_RGB)
        {
//#ifdef WINNT
            // if it's not 565, fail it
            if ((0xF800 != puod->lpDDSrcSurface->lpGbl->ddpfSurface.dwRBitMask) &&
            //for napalm check RGB888
                (!IS_NAPALM ||
                 (puod->lpDDSrcSurface->lpGbl->ddpfSurface.dwRGBBitCount != 32)))
            {
              puod->ddRVal = DDERR_UNSUPPORTED;
              return DDHAL_DRIVER_HANDLED;
            }
//#endif
        if (puod->lpDDSrcSurface->lpGbl->ddpfSurface.dwRGBBitCount == 16)
             pixelFormat =  SST_OVERLAY_PIXEL_RGB565U;
        else if(IS_NAPALM &&
               (puod->lpDDSrcSurface->lpGbl->ddpfSurface.dwRGBBitCount == 32))
		{
                 pixelFormat =  SST_OVERLAY_PIXEL_RGB32U;

		}
        else
        {
              puod->ddRVal = DDERR_UNSUPPORTED;
              return DDHAL_DRIVER_HANDLED;
        }

        }
        else if (puod->lpDDSrcSurface->lpGbl->ddpfSurface.dwFlags & DDPF_FOURCC)
        {
//#ifdef WINNT
            // Adjust YUV 422 formats to eliminate garbage on right edge

            if ( dwSrcW > 0 )       //PRS 13032
                vidOverlayDudx = Get_ScalingFactor(dwSrcW-1, dwDstW, 20);
//#endif
            switch( puod->lpDDSrcSurface->lpGbl->ddpfSurface.dwFourCC )
            {
              case FOURCC_YUY2:
#ifdef SIMULATE_YV12
              case FOURCC_YV12:
#endif
                  pixelFormat = SST_OVERLAY_PIXEL_YUYV422;
#ifdef WINNT
                  // fix for PRS #1811, force overlayAddr to start on a dword
                  // (or equivalently a YUV pixel pair) boundary of the src
                  // we really need the offset from the start of the surface
                  // to be an integer multiple of dwords but since the surface
                  // start address is dword aligned on NT this is sufficient
                  overlayAddr &= -4;
                  _FF(lastOverlayAddress) =  overlayAddr;
                  surfaceData->overlayAddr =  overlayAddr;
#endif
              break;
              case FOURCC_UYVY:
                  pixelFormat = SST_OVERLAY_PIXEL_UYVY422;
#ifdef WINNT
                  // fix for PRS #1811, force overlayAddr to start on a dword
                  // (or equivalently a YUV pixel pair) boundary of the src
                  // we really need the offset from the start of the surface
                  // to be an integer multiple of dwords but since the surface
                  // start address is dword aligned on NT this is sufficient
                  overlayAddr &= -4;
                  _FF(lastOverlayAddress) =  overlayAddr;
                  surfaceData->overlayAddr =  overlayAddr;
#endif
              break;
              default:
                  puod->ddRVal = DDERR_UNSUPPORTED;
#ifndef WINNT
                 RESTORE_BUSY(pFlags,SaveBusy);
#endif
                  return DDHAL_DRIVER_NOTHANDLED;
              break;
            }
        }
        else
        {
            puod->ddRVal = DDERR_UNSUPPORTED;
            return DDHAL_DRIVER_NOTHANDLED;
        }
#ifndef WINNT
      }
      else
      {
        if((GETPRIMARYBYTEDEPTH == 4) && IS_NAPALM)
        {
          pixelFormat =  SST_OVERLAY_PIXEL_RGB32U;
        }
        else
          pixelFormat =  SST_OVERLAY_PIXEL_RGB565D;
      }
#endif

      surfaceData->pixelFormat = pixelFormat;

      if(surfaceData->overlayShrinkFlag == TRUE)
      {
#if ((_WIN32_WINNT >= 0x0500)) || !defined(WINNT)
        if(IS_NAPALM && _DD(f32ShrinkOvl))
		{
         vidProcCfg |= SST_OVERLAY_PIXEL_RGB32U;
		 VidInData.dwBpp = 4;
		}
        else
#endif
		{
         vidProcCfg |= SST_OVERLAY_PIXEL_RGB565D;
		}

#ifdef KMVP
         SetBltParams( ppdev, puod->lpDDSrcSurface );
         _FF(ovlBltParams.wShrinkFlags) |= OVL_SHRINK;
#endif
      }
      else
      {
         vidProcCfg |= pixelFormat;
#ifdef KMVP
         _FF(ovlBltParams.wShrinkFlags) &= ~OVL_SHRINK;
#endif
      }
	   //set it in the second setp
      vidOverlayDudxOffsetSrcWidth = ((vidOverlayDudxOffsetSrcWidth * VidInData.dwBpp) << 19)|(DWORD)0; //Dudx offset = 0

#if ENABLE_VIDEOPORT
      //If autoflipping, then enable autoflipping
      if( puod->dwFlags & DDOVER_AUTOFLIP )
      {
         KMVTBUFF *lpKMBuff = (KMVTBUFF *)_FF(KMVTBuff);

         //If in BOB mode: enable back-end deinterlace
         if(puod->dwFlags & DDOVER_BOB)
         {
            vidProcCfg |= SST_OVERLAY_DEINTERLACE_EN;
            vidOverlayDvdyOffset = 0x20000;	         //Adjust vidOverlayDvdyOffset to .25
            if(puod->dwFlags & DDOVER_INTERLEAVED)
            {
              if(VidInData.WeaveDeinterlacing & CAN_SIMULATE)
              {
                 DWORD dwReg;
                 lpKMBuff->dwStatus |= (INTERLEAVE_ON | BOB_ON);
                 //double pitch
                 vidDesktopOverlayStride &= ~SST_OVERLAY_LINEAR_STRIDE;
                 vidDesktopOverlayStride |= (lPitch << (SST_OVERLAY_STRIDE_SHIFT+1)) & SST_OVERLAY_LINEAR_STRIDE;
                 SETDW(ghwIO->vidInStride, VidSrcData.dwVidSrcStride << 1);
             	 dwReg = GET(ghwIO->vidInFormat) &
                    ~(H3_VMI_BUFFER_MODE_MASK |H3_VMI_DEINTERLACE_MASK);
                 //set double buffer
                 SETDW(ghwIO->vidInFormat, dwReg | H3_VMI_DOUBLE_BUFFER);

				 lpKMBuff->dwVidInFormat = (dwReg | H3_VMI_DOUBLE_BUFFER);
				 VidSrcData.wNumbufs = 2;
				 lpKMBuff->dwStatus &= ~(VP_BUFF_MASK );
				 lpKMBuff->dwStatus  |= VidSrcData.wNumbufs << VP_BUFF_SHIFT;

                 //and flip on IRQ
                 _DS(dwOvlOffset) |= 0x80000000;

              }
              else
                lpKMBuff->dwStatus &= ~(INTERLEAVE_ON | BOB_ON);

            }
            else
                lpKMBuff->dwStatus &= ~(INTERLEAVE_ON | BOB_ON);
         }
         else
         {
            lpKMBuff->dwStatus &= ~BOB_ON;
         }

         if ( !_DS(dwOvlOffset) )
         {
            vidProcCfg |= SST_VIDEOIN_AS_OVERLAY;
            VSYNC_IRQ_DISABLE;
         }
         else
         {
            VSYNC_IRQ_ENABLE;
         }

#ifdef WINNT

        lpKMBuff->dwOVLOffset = _DS(dwOvlOffset);   //pass it to miniport
#endif

      }
      else
#endif
      {
#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
         //If in BOB mode: enable backend Bob deinterlacing
         if( puod->dwFlags & DDOVER_BOB )
         {
            if (puod->dwFlags & DDOVER_INTERLEAVED)   //If interleaved and if not shrinking
            {
               if (surfaceData->overlayShrinkFlag != TRUE)
               {
                  vidProcCfg |= SST_OVERLAY_DEINTERLACE_EN;
                  vidOverlayDvdyOffset = 0x20000;	   //Adjust vidOverlayDvdyOffset to .25
               }
            }
            else //Non-interleaved
            {
               vidProcCfg |= SST_OVERLAY_DEINTERLACE_EN;
               vidOverlayDvdyOffset = 0x20000;	      //Adjust vidOverlayDvdyOffset to .25
            }
         }

        if(surfaceData->overlayShrinkFlag == TRUE && surfaceData->doShrink == TRUE)
        {
          CMDFIFO_SAVE(hwPtr);
          ShrinkOverlaySurface(ppdev, puod->lpDDSrcSurface);
          CMDFIFO_RELOAD(hwPtr);
        }
#else
        CMDFIFO_CHECKROOM(hwPtr, 4);
        SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, leftOverlayBuf, 0xF ));
        SETPD(hwPtr, ghw0->leftOverlayBuf, (overlayAddr & ~SSTG_IS_TILED));
        SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, swapbufferCMD, 0xF ));
        SETPD(hwPtr, ghw0->swapbufferCMD,_DD(WaitOnVsync));
        BUMP(4);
#endif
      }

      WAIT_ON_VSYNC();      //NT need this call too.
      SETDW(ghwIO->vidOverlayStartCoords, vidOverlayStartAddr);
      SETDW(ghwIO->vidOverlayEndScreenCoord, vidOverlayEndAddr);
      SETDW(ghwIO->vidDesktopOverlayStride, vidDesktopOverlayStride);
      SETDW(ghwIO->vidOverlayDudx, vidOverlayDudx);
      SETDW(ghwIO->vidOverlayDudxOffsetSrcWidth, vidOverlayDudxOffsetSrcWidth);
      SETDW(ghwIO->vidOverlayDvdy, vidOverlayDvdy);
      SETDW(ghwIO->vidOverlayDvdyOffset, vidOverlayDvdyOffset);
      SETDW(ghwIO->vidChromaMin, vidChroma);
      SETDW(ghwIO->vidChromaMax, vidChroma);
#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
#if ENABLE_VIDEOPORT
     if(!(puod->dwFlags & DDOVER_AUTOFLIP))
#endif
      {
       CMDFIFO_CHECKROOM(hwPtr, 4);
       SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, leftOverlayBuf, 0xF ));
       SETPD(hwPtr, ghw0->leftOverlayBuf, (overlayAddr & ~SSTG_IS_TILED));
       SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, swapbufferCMD, 0xF ));
       //flip immediately to sync with overlay color format PRS5353
       SETPD(hwPtr, ghw0->swapbufferCMD, 0);
       BUMP(4);
      }

#endif
      SETDW(ghwIO->vidProcCfg, vidProcCfg);
      CMDFIFO_EPILOG(hwPtr);

    }

    puod->ddRVal = DD_OK;
#ifndef WINNT
    RESTORE_BUSY(pFlags,SaveBusy);
#endif
    return DDHAL_DRIVER_HANDLED;

} // UpdateOverlay32

/*******************************************************************/
/*                     EXPORTED FUNCTIONS                          */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name:  FlipOverlaySurface

Description:    flips overlay from surface to another.
                lpFlipData->lpSurfCurr is the
                surface we were at, lpFlipData->lpSurfTarg is the one we are
                flipping to.


Return:         DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

VOID  FlipOverlaySurface( NT9XDEVICEDATA *ppdev,LPDDHAL_FLIPDATA lpFlipData,
 FXSURFACEDATA *curSurfaceData, FXSURFACEDATA *tarSurfaceData,DWORD *swapToAddr)
{
    if (GET_HW_ADDR(lpFlipData->lpSurfCurr) == _FF(ddVisibleOverlaySurf))
    {
#ifdef WINNT
      tarSurfaceData->overlayAddr =
        curSurfaceData->overlayAddr - GET_HW_ADDR(lpFlipData->lpSurfCurr) + GET_HW_ADDR(lpFlipData->lpSurfTarg);
      *swapToAddr = tarSurfaceData->overlayAddr & ~SSTG_IS_TILED;
#endif
      _FF(ddVisibleOverlaySurf) = GET_HW_ADDR(lpFlipData->lpSurfTarg);
    }
#ifdef WINNT
    // fix for PRS bug #1670
    // ActiveMovie 2.0 (which is installed by IE4) calls Flip
    // before it calls UpdateOverlay.  When the overlay is already
    // in use by another app then we were falling thru and flipping
    // to one of the current app's overlay surfaces.  This is why
    // we were seeing frames from the non-hardware accelerated video
    // being flashed in the hardware accelerated window.
    // For some reason, this same problem doesn't seem to be happening on win95
    // so I made this fix NT specific
    else if (_FF(lastOverlayAddress) != INVALID_ADDRESS)
    {
      lpFlipData->ddRVal = DDERR_CURRENTLYNOTAVAIL;
    }
#endif

#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
    if((curSurfaceData->overlayShrinkFlag != tarSurfaceData->overlayShrinkFlag) )
    {
      tarSurfaceData->overlayShrinkFlag = curSurfaceData->overlayShrinkFlag;
    }

    if(tarSurfaceData->overlayShrinkFlag == TRUE)
    {
      tarSurfaceData->doShrink = TRUE;
      tarSurfaceData->shrinkWidth = curSurfaceData->shrinkWidth;
      tarSurfaceData->shrinkHeight = curSurfaceData->shrinkHeight;
      tarSurfaceData->pixelFormat = curSurfaceData->pixelFormat;
      tarSurfaceData->shrinkSrcWidth = curSurfaceData->shrinkSrcWidth;
      tarSurfaceData->shrinkSrcHeight = curSurfaceData->shrinkSrcHeight;
      tarSurfaceData->shrinkSrcX = curSurfaceData->shrinkSrcX;
      tarSurfaceData->shrinkSrcY = curSurfaceData->shrinkSrcY;
      if(_DD(overlaySurfaceCnt) >= 2)
      {
         (WORD)(_FF(dwCurrentSKSurf)) +=1;       //change current surface
      }

      if(_FF(dwCurrentSKSurf) & 1 )
         tarSurfaceData->overlayAddr = (_DD(shrinkSurface2))->hwPtr;
      else
          tarSurfaceData->overlayAddr = (_DD(shrinkSurface1))->hwPtr;

    }
    else
    {
      tarSurfaceData->doShrink = FALSE;
      tarSurfaceData->overlayAddr =
        curSurfaceData->overlayAddr - GET_HW_ADDR(lpFlipData->lpSurfCurr) + GET_HW_ADDR(lpFlipData->lpSurfTarg);
    }

    //DX-6: AGUS Flip ODD & EVEN field support
    //Check if target surface is interleaved
    if ( (lpFlipData->lpSurfTarg->lpSurfMore->dwOverlayFlags & DDOVER_INTERLEAVED) )
    {
        if ( lpFlipData->dwFlags & DDFLIP_ODD )
        {
            if ( tarSurfaceData->doShrink != TRUE ) //If not shrinking: start on the 2nd line
               *swapToAddr = (tarSurfaceData->overlayAddr + tarSurfaceData->lPitch) & ~SSTG_IS_TILED;
            else
               *swapToAddr = tarSurfaceData->overlayAddr & ~SSTG_IS_TILED;
        }
        else if ( lpFlipData->dwFlags & DDFLIP_EVEN )
        {
            *swapToAddr = tarSurfaceData->overlayAddr | LEFT_OVERLAYBUF_EVENFIELD;
        }
        else
        {
            *swapToAddr = tarSurfaceData->overlayAddr & ~SSTG_IS_TILED;
        }
    }
    else //Target surface is non-interleaved
    {
        //Set bit(31) on leftOverlayBuf reg for flip on EVEN field
        if ( (lpFlipData->dwFlags & DDFLIP_EVEN) )
        {
            *swapToAddr = tarSurfaceData->overlayAddr | LEFT_OVERLAYBUF_EVENFIELD;
        }
        else
        {
            *swapToAddr = tarSurfaceData->overlayAddr & ~SSTG_IS_TILED;
        }
    }

    if(tarSurfaceData->doShrink == TRUE)
    {
      ShrinkOverlaySurface(ppdev, lpFlipData->lpSurfTarg);
      tarSurfaceData->doShrink = FALSE;
    }
#endif

      lpFlipData->ddRVal = DD_OK;
}


/*----------------------------------------------------------------------
Function name: CanCreateOverlaySurface

Description:   Reports whether surface can be created.

               Surfaces allowed in offscreen memory:
			   overlay:         yuy2, uyvy, rgb16, rgb32

Return:        if color foramt is support pccsd->ddRVal= DD_OK
----------------------------------------------------------------------*/
VOID CanCreateOverlaySurface(NT9XDEVICEDATA *ppdev,
            LPDDHAL_CANCREATESURFACEDATA pccsd, DWORD dwBpp, DWORD dwFlags)
{

    if (!pccsd->bIsDifferentPixelFormat)
    {
       if ((dwBpp == 16) || (IS_NAPALM  && (dwBpp == 32)))
          pccsd->ddRVal = DD_OK;
    }
    else
    {
      if ((dwFlags & DDPF_RGB) && !(dwFlags & DDPF_FOURCC))
      {
        if ((dwBpp == 16) && (pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask == 0xf800))  // Check for 5:6:5 format
        {
          pccsd->ddRVal = DD_OK;
        }
        if (IS_NAPALM  && (dwBpp == 32)) // Napalm supports 32bpp overlay
        {
          pccsd->ddRVal = DD_OK;
        }
      }
      else if (dwFlags & DDPF_FOURCC)
      {
        if ((pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC == FOURCC_YUY2) ||
            (pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC == FOURCC_UYVY))
        {
          pccsd->ddRVal = DD_OK;
        }
#ifdef SIMULATE_YV12
        if (FOURCC_YV12 == pccsd->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC)
        {
          if ((DDSD_WIDTH & dwFlags) && (1024 > pccsd->lpDDSurfaceDesc->dwWidth))
            pccsd->ddRVal = DD_OK;
          else
            pccsd->ddRVal = DDERR_TOOBIGWIDTH;
        }
#endif
      }
    }
}



/*----------------------------------------------------------------------
Function name: CreateOverlaySurface

Description:   Attempts to allocate overlay surface and its
               shrink surface

Return:        if success return pixel byte depth
----------------------------------------------------------------------*/
DWORD CreateOverlaySurface(
                NT9XDEVICEDATA             *ppdev,
                LPDDHAL_CREATESURFACEDATA pcsd,
                LPDDRAWI_DDRAWSURFACE_LCL  psurf,
                FXSURFACEDATA              *surfaceData,
                DWORD                      pWidth,
                DWORD                      height
                )
{
  LPDDRAWI_DDRAWSURFACE_GBL   psurf_gbl = psurf->lpGbl;
  DWORD pixelByteDepth;
#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
  FXSURFACEDATA* shrinkSurfaceData;
  DWORD  bWidth, tileFlag;
#endif

#ifdef WINNT
      // nt doesn't set the DDRAWISURF_HASPIXELFORMAT bit
      if (DDSD_PIXELFORMAT & pcsd->lpDDSurfaceDesc->dwFlags)
#else
      if((psurf->dwFlags) & DDRAWISURF_HASPIXELFORMAT )
#endif
      {
        if(psurf_gbl->ddpfSurface.dwFlags & DDPF_FOURCC)
        {
#ifdef SIMULATE_YV12
          if ((FOURCC_YUY2 != psurf_gbl->ddpfSurface.dwFourCC) &&
              (FOURCC_UYVY != psurf_gbl->ddpfSurface.dwFourCC) &&
              (FOURCC_YV12 != psurf_gbl->ddpfSurface.dwFourCC))
#else
          if( (psurf_gbl->ddpfSurface.dwFourCC != FOURCC_YUY2)
            && (psurf_gbl->ddpfSurface.dwFourCC != FOURCC_UYVY))
#endif
          {
            pcsd->ddRVal = DDERR_UNSUPPORTED;
            return 0;
          }

#if defined(SIMULATE_YV12) && !defined (WINNT)
         if(FOURCC_YV12 == psurf_gbl->ddpfSurface.dwFourCC)
         {
             //allocate system buffer for YV12
             psurf_gbl->fpVidMem =
               (DWORD)DXMALLOC(pWidth * ( height + (height +1) /2));
             if( psurf_gbl->fpVidMem == 0)
             {
                pcsd->ddRVal = DDERR_OUTOFMEMORY;
                return 0;
             }

              psurf_gbl->lPitch = pWidth;

              psurf_gbl->ddpfSurface.dwYUVBitCount = 12;
          }
          else
#endif
          psurf_gbl->ddpfSurface.dwYUVBitCount = 16;
          pixelByteDepth = 2;
        }   // FOURCC
        else if(psurf_gbl->ddpfSurface.dwFlags & DDPF_RGB)
        {
          pixelByteDepth = (DWORD)(psurf_gbl->ddpfSurface.dwRGBBitCount) >> 3;
        }   // RGB
        else
        {
          pcsd->ddRVal = DDERR_UNSUPPORTED;
          return 0;
        }
      } // HASPIXELFORMAT
      else
      {
        pixelByteDepth = GETPRIMARYBYTEDEPTH;
      } // !DDRAWISURF_HASPIXELFORMAT

      if( (pixelByteDepth != 2) &&
           ( !IS_NAPALM  || (pixelByteDepth != 4))     //count for 32 bit RGB
      )
      {
          pcsd->ddRVal = DDERR_UNSUPPORTED;
          return 0;
      }
#if (_WIN32_WINNT >= 0x0500) || !defined(WINNT)
      bWidth = pWidth * pixelByteDepth;


      if(!_DD(overlaySurfaceCnt)&& IS_NAPALM )
      {
   
        if(GETPRIMARYBYTEDEPTH==4)     //only check for 32bit desktop
        {
            RECT rVidSrc, rMaxVidSrcAllowed;
            //First check BW
            rVidSrc.left   = 0;
            rVidSrc.top    = 0;
            rVidSrc.right  = 720;
            rVidSrc.bottom = 480;

            H3_InitBandwidth(ppdev);

            _DD(H3BandWidth).OverlayBPP  = 4; //Overlay bytes per pixel
            if(H3_Calculate_Bandwidth(ppdev, &rVidSrc, &rMaxVidSrcAllowed,TRUE) )
            {
           
            _DD(f32ShrinkOvl) = TRUE;
            }
            else
            {
            _DD(f32ShrinkOvl) = FALSE;
            }
        }
        else
           _DD(f32ShrinkOvl) = TRUE;

      }


 #if USE_NT5_DDMEMMGR
      if(_DD(overlaySurfaceCnt) <= 1)      //only create two shrink surfaces
      {
        FXSURFACEDATA* txtrSurfaceData;

            shrinkSurfaceData = (FXSURFACEDATA*) DDMALLOCZ(sizeof(FXSURFACEDATA), 0);

            if(!shrinkSurfaceData)
            {
                pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
                return DDHAL_DRIVER_HANDLED;
            }


           if(IS_NAPALM && _DD(f32ShrinkOvl)) 
             pcsd->ddRVal = memMgr_allocSurface(ppdev,
                                       // psurf->ddsCaps.dwCaps,
                                        DDSCAPS_LIVEVIDEO,
                                        bWidth << 1,
                                        height,
                                        ((bWidth <<1)+ 0x7F)>> 7L,
                                        (height + 0x1F)>> 5L,
                                        &(shrinkSurfaceData->lfbPtr),
                                        &(shrinkSurfaceData->hwPtr),
                                        &(shrinkSurfaceData->lPitch),
                                        &tileFlag,
                                        &(shrinkSurfaceData->heapID),
                                        &(shrinkSurfaceData->pvmHeap));
          else
             pcsd->ddRVal = memMgr_allocSurface(ppdev,
                                        //psurf->ddsCaps.dwCaps,
                                        DDSCAPS_LIVEVIDEO,
                                        bWidth,
                                        height,
                                        (bWidth + 0x7F)>> 7L,
                                        (height + 0x1F)>> 5L,
                                        &(shrinkSurfaceData->lfbPtr),
                                        &(shrinkSurfaceData->hwPtr),
                                        &(shrinkSurfaceData->lPitch),
                                        &tileFlag,
                                        &(shrinkSurfaceData->heapID),
                                        &(shrinkSurfaceData->pvmHeap));

          if (DD_OK != pcsd->ddRVal)
          {
            DDFREE(shrinkSurfaceData);
            return 0;
          }
            //Allocate a texture surface
          if(IS_NAPALM && (_DD(overlaySurfaceCnt) == 0)&& _DD(f32ShrinkOvl))
          {
                DWORD dwNewWidth, dwNewHeight;

                txtrSurfaceData = (FXSURFACEDATA*) DDMALLOCZ(sizeof(FXSURFACEDATA), 0);
                if(!txtrSurfaceData)
                {

                   memMgr_freeSurface(ppdev,
                         //psurf->ddsCaps.dwCaps,
                         DDSCAPS_LIVEVIDEO,
                         shrinkSurfaceData->lfbPtr,
                         shrinkSurfaceData->hwPtr,
                         GETMEMTYPE(shrinkSurfaceData->hwPtr),
                         shrinkSurfaceData->heapID,
                         shrinkSurfaceData->pvmHeap);

                    DDFREE(shrinkSurfaceData);
                    pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
                    return DDHAL_DRIVER_HANDLED;
                }

                //dwNewWidth and dwNewHeight must be power of 2 numbers

                for( dwNewWidth = 1; dwNewWidth < pWidth ; dwNewWidth <<= 1)
                    ;

                for( dwNewHeight = 1; dwNewHeight < height ; dwNewHeight <<=1 )
                    ;

                pcsd->ddRVal = memMgr_allocSurface(ppdev,
                                        //psurf->ddsCaps.dwCaps,
                                        DDSCAPS_LIVEVIDEO,
                                        dwNewWidth * 4 + 2*bWidth,
                                        height,     //first try to including second shrink surface
                                        (dwNewWidth * 4 + 2*bWidth+ 0x7F)>> 7L,
                                        (height + 0x1F)>> 5L,
                                        &(txtrSurfaceData->lfbPtr),
                                        &(txtrSurfaceData->hwPtr),
                                        &(txtrSurfaceData->lPitch),
                                        &tileFlag,
                                        &(txtrSurfaceData->heapID),
                                        &(txtrSurfaceData->pvmHeap));

                 if (DD_OK != pcsd->ddRVal)
                 {
                    DDFREE(txtrSurfaceData);
                    txtrSurfaceData = 0;        //no texture surface

                 }
                 else
                 {
                     //free it
                      memMgr_freeSurface(ppdev,
                         DDSCAPS_LIVEVIDEO,
                        // psurf->ddsCaps.dwCaps,
                         txtrSurfaceData->lfbPtr,
                         txtrSurfaceData->hwPtr,
                         GETMEMTYPE(txtrSurfaceData->hwPtr),
                         txtrSurfaceData->heapID,
                         txtrSurfaceData->pvmHeap);

                     //then try to allocate the right amount memory
                     pcsd->ddRVal = memMgr_allocSurface(ppdev,
                                       // psurf->ddsCaps.dwCaps,
                                        DDSCAPS_LIVEVIDEO,
                                        dwNewWidth * 4,
                                        height,
                                        (dwNewWidth * 4 + 0x7F)>> 7L,
                                        (height + 0x1F)>> 5L,
                                        &(txtrSurfaceData->lfbPtr),
                                        &(txtrSurfaceData->hwPtr),
                                        &(txtrSurfaceData->lPitch),
                                        &tileFlag,
                                        &(txtrSurfaceData->heapID),
                                        &(txtrSurfaceData->pvmHeap));

 #else  //USE_NT5_DDMEMMGR
      pcsd->ddRVal = memMgr_allocSurface(
        psurf_gbl->lpDD,          // direct draw vidmemalloc need this
        psurf->ddsCaps.dwCaps,    // type of surface (can use the standard DD surface flags) [IN]
        bWidth,                   // width in byte (linear space) [IN]
        height,                   // height in linear space [IN]
        (bWidth + 0x7F)>> 7L,     // width in tiled space [IN]
        (height + 0x1F)>> 5L,     // width in tiled space [IN]
        &(surfaceData->lfbPtr),   // host lfb start address of allocation [OUT]
        &(surfaceData->hwPtr),    // hw vidmem address
        &(surfaceData->lPitch),   // pitch [OUT]  
        &tileFlag,                // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
        &(surfaceData->heapID));  // Ddraw heap ID[OUT]

        if(!surfaceData->lfbPtr)
        {
            pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
            return 0;
        }
#if defined(SIMULATE_YV12) && !defined (WINNT)
        if(psurf_gbl->ddpfSurface.dwFourCC != FOURCC_YV12)
#endif
            psurf_gbl->lPitch = surfaceData->lPitch;

     // extra buffer in case overlay is not a flipping chain and shrink need a buffer -SS

      //Init surface data's overlayAddr to point to the same hw addr of the surface
        surfaceData->overlayAddr = surfaceData->hwPtr;

       if(_DD(overlaySurfaceCnt) <= 1)      //only create two shrink surfaces
       {
            FXSURFACEDATA* txtrSurfaceData;

            shrinkSurfaceData = (FXSURFACEDATA*) DXMALLOCZ(sizeof(FXSURFACEDATA));
            if(!shrinkSurfaceData)
            {
                pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
                return DDHAL_DRIVER_HANDLED;
            }

            if(IS_NAPALM && _DD(f32ShrinkOvl))
                memMgr_allocSurface(
                    psurf_gbl->lpDD,                // direct draw vidmemalloc need this
                    DDSCAPS_LIVEVIDEO,
                   // psurf->ddsCaps.dwCaps,          // type of surface (can use the standard DD surface flags) [IN]
                    bWidth << 1,                    // width in byte (linear space) [IN]
                                                    // for 32 bit RGB shrink buffer
                    height,                         // height in linear space [IN]
                    ((bWidth<< 1) + 0x7F)>> 7L,     // width in tiled space [IN]
                    (height + 0x1F)>> 5L,           // width in tiled space [IN]
                    &(shrinkSurfaceData->lfbPtr),   // host lfb start address of allocation [OUT]
                    &(shrinkSurfaceData->hwPtr),    // hw vidmem address
                    &(shrinkSurfaceData->lPitch),   // pitch [OUT]
                    &tileFlag,                      // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
                    &(shrinkSurfaceData->heapID));  // Ddraw heap ID[OUT]
                else
                memMgr_allocSurface(
                    psurf_gbl->lpDD,                // direct draw vidmemalloc need this
                   // psurf->ddsCaps.dwCaps,          // type of surface (can use the standard DD surface flags) [IN]
                    DDSCAPS_LIVEVIDEO,
                    bWidth,                         // width in byte (linear space) [IN]
                    height,                         // height in linear space [IN]
                    (bWidth + 0x7F)>> 7L,           // width in tiled space [IN]
                    (height + 0x1F)>> 5L,           // width in tiled space [IN]
                    &(shrinkSurfaceData->lfbPtr),   // host lfb start address of allocation [OUT]
                    &(shrinkSurfaceData->hwPtr),    // hw vidmem address
                    &(shrinkSurfaceData->lPitch),   // pitch [OUT]
                    &tileFlag,                      // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
                    &(shrinkSurfaceData->heapID));  // Ddraw heap ID[OUT]

            if(!shrinkSurfaceData->lfbPtr)
            {
                pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
                return 0;
            }
            //Allocate a texture surface
            if(IS_NAPALM && (_DD(overlaySurfaceCnt) == 0)&& _DD(f32ShrinkOvl))
            {
                DWORD dwNewWidth, dwNewHeight;

                txtrSurfaceData = (FXSURFACEDATA*) DXMALLOCZ(sizeof(FXSURFACEDATA));
                if(!txtrSurfaceData)
                {
                    pcsd->ddRVal = DDERR_OUTOFVIDEOMEMORY;
                    return DDHAL_DRIVER_HANDLED;
                }

                //dwNewWidth and dwNewHeight must be power of 2 numbers

                for( dwNewWidth = 1; dwNewWidth < pWidth ; dwNewWidth <<= 1)
                    ;

                for( dwNewHeight = 1; dwNewHeight < height ; dwNewHeight <<=1 )
                    ;

                memMgr_allocSurface(
                    psurf_gbl->lpDD,                // direct draw vidmemalloc need this
                    //psurf->ddsCaps.dwCaps,          // type of surface (can use the standard DD surface flags) [IN]
                    DDSCAPS_LIVEVIDEO,
                    dwNewWidth * 4,               // width in byte (linear space) [IN]
                    height,                       // height in linear space [IN]
                    (dwNewWidth* 4 + 0x7F )>> 7L,      // width in tiled space [IN]
                    (height + 0x1F)>> 5L,           // width in tiled space [IN]
                    &(txtrSurfaceData->lfbPtr),     // host lfb start address of allocation [OUT]
                    &(txtrSurfaceData->hwPtr),      // hw vidmem address
                    &(txtrSurfaceData->lPitch),     // pitch [OUT]
                    &tileFlag,                      // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
                    &(txtrSurfaceData->heapID));    // Ddraw heap ID[OUT]

                if(!txtrSurfaceData->lfbPtr)
                {

                    DXFREE((void*)txtrSurfaceData);
                    txtrSurfaceData = 0;
                }
                else
                {
                    FXSURFACEDATA* extraSurfaceData;

                    extraSurfaceData = (FXSURFACEDATA*) DXMALLOCZ(sizeof(FXSURFACEDATA));

                    if(!extraSurfaceData)
                    {
                        pcsd->ddRVal = DDERR_OUTOFMEMORY;
                        return DDHAL_DRIVER_HANDLED;
                    }

                    //see we can allocate at least another
                    //shrink surface

                    memMgr_allocSurface(
                    psurf_gbl->lpDD,                // direct draw vidmemalloc need this
                    DDSCAPS_LIVEVIDEO,
                   // psurf->ddsCaps.dwCaps,          // type of surface (can use the standard DD surface flags) [IN]
                    bWidth << 1,                    // width in byte (linear space) [IN]
                                                    // for 32 bit RGB shrink buffer
                    height,                         // height in linear space [IN]
                    ((bWidth<< 1) + 0x7F)>> 7L,     // width in tiled space [IN]
                    (height + 0x1F)>> 5L,           // width in tiled space [IN]
                    &(extraSurfaceData->lfbPtr),   // host lfb start address of allocation [OUT]
                    &(extraSurfaceData->hwPtr),    // hw vidmem address
                    &(extraSurfaceData->lPitch),   // pitch [OUT]
                    &tileFlag,                      // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
                    &(extraSurfaceData->heapID));  // Ddraw heap ID[OUT]


                    if(!extraSurfaceData->lfbPtr)
                    {
                      //free the textrue surface

                       memMgr_freeSurface(
                        psurf_gbl->lpDD,                        // direct draw vidmemalloc need this
                        DDSCAPS_LIVEVIDEO,                        // type of surface (can use the standard DD surface flags) [IN]
                        txtrSurfaceData->lfbPtr,              // host lfb start address of allocation [OUT]
                        txtrSurfaceData->hwPtr,               // hw start address of allocation [OUT]
                        GETMEMTYPE(txtrSurfaceData->hwPtr),   // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
                        txtrSurfaceData->heapID);             // Ddraw heap ID[OUT]

                        DXFREE((void*)txtrSurfaceData);
                        txtrSurfaceData = 0;


                   }
                   else
                   {


                      //free the extra surface
                       memMgr_freeSurface(
                        psurf_gbl->lpDD,                        // direct draw vidmemalloc need this
                        DDSCAPS_LIVEVIDEO,                        // type of surface (can use the standard DD surface flags) [IN]
                        extraSurfaceData->lfbPtr,              // host lfb start address of allocation [OUT]
                        extraSurfaceData->hwPtr,               // hw start address of allocation [OUT]
                        GETMEMTYPE(txtrSurfaceData->hwPtr),   // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
                        extraSurfaceData->heapID);             // Ddraw heap ID[OUT]


                   }

                   DXFREE((void*)extraSurfaceData);

#endif  //USE_NT5_DDMEMMGR
                  txtrSurfaceData->shrinkSrcWidth = dwNewWidth;
                  txtrSurfaceData->shrinkSrcHeight = dwNewHeight;
                }


            }
            if(_DD(overlaySurfaceCnt) == 0)
            {
                _DD(shrinkSurface1) =  shrinkSurfaceData;
                UPDATE_BLOCK_DATA(shrinkSurfaceData, &_DD(shrinkSurface1));
                _FF(dwShrinkSurfAddr1) = shrinkSurfaceData->hwPtr;

                if(IS_NAPALM && _DD(f32ShrinkOvl))
                {
                   _DD(txtrSurface1)  =  txtrSurfaceData;
                   UPDATE_BLOCK_DATA(txtrSurfaceData, &_DD(txtrSurface1));
                   if(txtrSurfaceData)
                     _FF(dwTxtrSurfAddr1) = txtrSurfaceData->hwPtr;
                   else
                     _FF(dwTxtrSurfAddr1) = 0;

                }
                else
                  _FF(dwTxtrSurfAddr1) = 0;
                _FF(dwCurrentSKSurf) = 0;
            }
            else
            {
                _DD(shrinkSurface2) =  shrinkSurfaceData;
                UPDATE_BLOCK_DATA(shrinkSurfaceData, &_DD(shrinkSurface2));
                _FF(dwShrinkSurfAddr2) = shrinkSurfaceData->hwPtr;
                _FF(dwCurrentSKSurf) = 0x10000;  //high word has counter
            }

       }
#endif  //USE_NT5_DDMEMMGR !WINNT
       pcsd->ddRVal = DD_OK;
       _DD(overlaySurfaceCnt) += 1;
       return pixelByteDepth;
}

/*----------------------------------------------------------------------
Function name:DestroyOverlaySurface

Description:  Frees overlay shrink surface

Return
----------------------------------------------------------------------*/
VOID DestroyOverlaySurface(NT9XDEVICEDATA *ppdev,
                           LPDDRAWI_DDRAWSURFACE_LCL  pDDSurf,
                           LPDDRAWI_DDRAWSURFACE_GBL psurf_gbl,
                           FXSURFACEDATA* surfaceData)
{
   DWORD                vidProcCfg;
    surfaceData = (FXSURFACEDATA*) psurf_gbl->dwReserved1;
    if(surfaceData != NULL)
    {
       _FF(lastOverlayAddress) = INVALID_ADDRESS;  //Reset this flag when overlay surf destroyed

       if(_FF(ddVisibleOverlaySurf) == (surfaceData->hwPtr & ~SSTG_IS_TILED))
             _FF(ddVisibleOverlaySurf) = 0;

       if( _DD(overlaySurfaceCnt) == 1)
       {
         OVERLAYDISABLE;
       }

       if( _DD(overlaySurfaceCnt) != 0)
          _DD(overlaySurfaceCnt) -= 1;

#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
       {
#if defined(SIMULATE_YV12) && !defined(WINNT)
         if(psurf_gbl->ddpfSurface.dwFourCC == FOURCC_YV12)
         {
           
                DXFREE(psurf_gbl->fpVidMem);
         }
#endif

         if(_FF(ddVisibleOverlaySurf) == GET_HW_ADDR(pDDSurf))
            _FF(ddVisibleOverlaySurf) = 0;

         if(_DD(overlaySurfaceCnt) <= 1 )
         {
            FXSURFACEDATA* shrinkSurfaceData,*txtrSurfaceData;

            if(_DD(overlaySurfaceCnt) == 1)
            {
                    shrinkSurfaceData = _DD(shrinkSurface2);
                    _DD(shrinkSurface2) = NULL;
            }
            else
            {
                    shrinkSurfaceData = _DD(shrinkSurface1);
                    _DD(shrinkSurface1) = NULL;
                    if(IS_NAPALM)
                    {
                      txtrSurfaceData   = _DD(txtrSurface1);
                      _DD(txtrSurface1) = NULL;
                    }
            }

            _FF(dwCurrentSKSurf) = 0;

            if(shrinkSurfaceData !=NULL)
            {
            DWORD hwPtr;
            hwPtr = shrinkSurfaceData->hwPtr & ~SSTG_IS_TILED;

            if((shrinkSurfaceData->heapID != HEAP_INVALID) && (hwPtr != (DWORD)NULL))
        #if USE_NT5_DDMEMMGR
            {
                memMgr_freeSurface(ppdev,
                                    DDSCAPS_LIVEVIDEO,
                                    shrinkSurfaceData->lfbPtr,
                                    shrinkSurfaceData->hwPtr,
                                    GETMEMTYPE(shrinkSurfaceData->hwPtr),
                                    shrinkSurfaceData->heapID,
                                    shrinkSurfaceData->pvmHeap);
                DDFREE(shrinkSurfaceData);

                if((IS_NAPALM) && (_DD(overlaySurfaceCnt) == 0)
                        &&(txtrSurfaceData !=NULL))
                {
                    memMgr_freeSurface(ppdev,
                                    DDSCAPS_LIVEVIDEO,
                                    txtrSurfaceData->lfbPtr,
                                    txtrSurfaceData->hwPtr,
                                    GETMEMTYPE(txtrSurfaceData->hwPtr),
                                    txtrSurfaceData->heapID,
                                    txtrSurfaceData->pvmHeap);
                   DDFREE(txtrSurfaceData);
                }
            }
        #else
            {
                memMgr_freeSurface(
                psurf_gbl->lpDD,                        // direct draw vidmemalloc need this
                DDSCAPS_LIVEVIDEO,                        // type of surface (can use the standard DD surface flags) [IN]
                shrinkSurfaceData->lfbPtr,              // host lfb start address of allocation [OUT]
                shrinkSurfaceData->hwPtr,               // hw start address of allocation [OUT]
                GETMEMTYPE(shrinkSurfaceData->hwPtr),   // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
                shrinkSurfaceData->heapID);             // Ddraw heap ID[OUT]

                if((IS_NAPALM) &&(_DD(overlaySurfaceCnt) == 0)&&
                    (txtrSurfaceData !=NULL))
                {
                    memMgr_freeSurface(
                        psurf_gbl->lpDD,                        // direct draw vidmemalloc need this
                        DDSCAPS_LIVEVIDEO,                        // type of surface (can use the standard DD surface flags) [IN]
                        txtrSurfaceData->lfbPtr,              // host lfb start address of allocation [OUT]
                        txtrSurfaceData->hwPtr,               // hw start address of allocation [OUT]
                        GETMEMTYPE(txtrSurfaceData->hwPtr),   // MEM_IN_TILED or MEM_IN_LINEAR [OUT]
                        txtrSurfaceData->heapID);             // Ddraw heap ID[OUT]
                        DXFREE((void*)txtrSurfaceData);
                }

            }

            DXFREE((void*)shrinkSurfaceData);
         #endif

            }
         }
       }
#endif
    }
}

#ifndef WINNT
/*----------------------------------------------------------------------
Function name: ConvertYV12

Description:   Convert YV12 to YUYV using current HW
Return:
----------------------------------------------------------------------*/
void ConvertYV12(NT9XDEVICEDATA *ppdev,
  LPDDRAWI_DDRAWSURFACE_GBL psurf_gbl,
  FXSURFACEDATA       *surfaceData)
{

WORD i,wWidth, wHeight;
BYTE *lpMem;
BYTE  *lpDstYAddr,*lpDstUAddr, *lpDstVAddr;

   wWidth  = psurf_gbl->wWidth;
   wHeight = psurf_gbl->wHeight; 
   lpMem = (BYTE *)(psurf_gbl->fpVidMem);

   lpDstYAddr = (BYTE*)(_FF(regBase[HWINFO_SST_IOREGS_INDEX]) + SST_YUV_OFFSET);
   lpDstUAddr = (BYTE*)lpDstYAddr + 0x100000 ;           //1 MG offset
   lpDstVAddr = (BYTE*)lpDstUAddr + 0x100000 ;
     
   SETDW(ghwAC->yuvBaseAddr, surfaceData->hwPtr); //Point planar YUV apperture to the surface

   SETDW(ghwAC->yuvStride, (surfaceData->lPitch) & 0x00003FFF);  //Set the linear stride


   for( i = 0; i < wHeight; i++)
   {
        memcpy(lpDstYAddr, lpMem, wWidth);
        lpDstYAddr += 1024;
        lpMem += psurf_gbl->lPitch;

   }

   wWidth  >>=1;
   wHeight >>=1;
   for( i = 0; i < wHeight; i++)
   {
        memcpy(lpDstVAddr, lpMem, wWidth);
        lpDstVAddr += 1024;
        lpMem += psurf_gbl->lPitch /2;

   }

   for( i = 0; i < wHeight; i++)
   {
        memcpy(lpDstUAddr, lpMem, wWidth);
        lpDstUAddr += 1024;
        lpMem += psurf_gbl->lPitch /2;

   }

}


#endif

#if (_WIN32_WINNT >= 0x0500) || !defined(WINNT)
/*----------------------------------------------------------------------
Function name: UnlockOverlaySurface

Description:   For shrink overlay surface
               Starts a shrink blt
Return:
----------------------------------------------------------------------*/
VOID UnlockOverlaySurface(NT9XDEVICEDATA *ppdev,LPDDHAL_UNLOCKDATA puld)
{
    FXSURFACEDATA * surfaceData;

    surfaceData = (FXSURFACEDATA*) (puld->lpDDSurface->lpGbl->dwReserved1);
    // Shrink overlay to shrink surface.
    if(surfaceData)
    {
#ifndef WINNT
#ifdef SIMULATE_YV12
        if(puld->lpDDSurface->lpGbl->ddpfSurface.dwFourCC == FOURCC_YV12)
        {   
 
             ConvertYV12( ppdev, puld->lpDDSurface->lpGbl, surfaceData);
        }
#endif
#endif
       if(((surfaceData->doShrink == TRUE) ||(_DD(overlaySurfaceCnt) == 1))
          &&(surfaceData->overlayShrinkFlag == TRUE)
          &&(GET_HW_ADDR(puld->lpDDSurface) == _FF(ddVisibleOverlaySurf)))
        {
        FXBUSYWAIT(ppdev);  /* retro3dfx: bounded */
        ShrinkOverlaySurface(ppdev, puld->lpDDSurface);
        surfaceData->doShrink = FALSE;
        }
    }
}


#endif
/*******************************************************************/
/*                     PRIVATE FUNCTIONS                           */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name:  SetBltParams

Description:    Sets up Blt parameters for overlay shrinking.

				This function should only be called in UpdateOverlay.

Return:         none
----------------------------------------------------------------------*/

#ifdef KMVP
void SetBltParams( NT9XDEVICEDATA *ppdev, LPDDRAWI_DDRAWSURFACE_LCL psurf)
{
  DWORD bltDstBaseAddr;
  DWORD bltSrcBaseAddr;

  DWORD dstLeft, dstTop, dstRight, dstBottom, dstWidth, dstHeight, dstPitch;
  DWORD srcLeft, srcTop, srcRight, srcBottom, srcWidth, srcHeight, srcPitch;
  DWORD srcX, srcY, dstX, dstY;
  DWORD pixelFormat;
  OVLBLTPARAMS * lpBltParams;

  FXSURFACEDATA *srcSurfaceData, *dstSurfaceData;

  srcSurfaceData = (FXSURFACEDATA*)(psurf->lpGbl->dwReserved1);
  dstSurfaceData = _DD(shrinkSurface1);

  lpBltParams = &_FF(ovlBltParams);

  switch(srcSurfaceData->pixelFormat)
  {
    case SST_OVERLAY_PIXEL_YUYV422:
      pixelFormat = SSTG_PIXFMT_422YUV;
      break;
    case SST_OVERLAY_PIXEL_UYVY422:
      pixelFormat = SSTG_PIXFMT_422UYV;
      break;
    case SST_OVERLAY_PIXEL_RGB32U:
      pixelFormat = SSTG_PIXFMT_32BPP;
      break;
    default:
      pixelFormat = SSTG_PIXFMT_16BPP;
      break;
  }

  // get rectangle
  dstTop = 0;
  dstRight = srcSurfaceData->shrinkWidth + 1;
  dstBottom = srcSurfaceData->shrinkHeight + 1;
  dstLeft = 0;

  srcTop = srcSurfaceData->shrinkSrcY;
  srcLeft = srcSurfaceData->shrinkSrcX;
  srcRight = srcLeft + srcSurfaceData->shrinkSrcWidth;
  srcBottom = srcTop + srcSurfaceData->shrinkSrcHeight;

  srcWidth = srcRight - srcLeft;
  srcHeight = srcBottom - srcTop;

  if ( dstRight > srcWidth )
   dstRight = srcWidth;

  if (dstBottom > srcHeight )
   dstBottom = srcHeight;

  dstWidth = dstRight - dstLeft;
  dstHeight = dstBottom - dstTop;

  // get base address
  bltSrcBaseAddr = GET_HW_ADDR(psurf);
  bltDstBaseAddr = dstSurfaceData->hwPtr;

#if ENABLE_TILED_HEAP
  if(IS_TILED(bltSrcBaseAddr))
  {
    srcPitch = _DS(ddTileStride);
  }
  else
#endif
  {
    srcPitch = srcSurfaceData->lPitch;
  }


#if ENABLE_TILED_HEAP
  if(IS_TILED(bltDstBaseAddr))
  {
    dstPitch = _DS(ddTileStride);
  }
  else
#endif
  {
    dstPitch = dstSurfaceData->lPitch;
  }



  //if destination is above and left of src, we starts srccopy at the upper left corner
  // otherwise:
  srcX = srcLeft;
  srcY = srcTop;
  dstX = dstLeft;
  dstY = dstTop;

  BLTCLIP(dstLeft, dstTop, lpBltParams->clip1min);
  BLTCLIP(dstRight, dstBottom, lpBltParams->clip1max);

  // now stuff them in the hardware format
  if(IS_NAPALM && _DD(f32ShrinkOvl))
      BLTFMT(dstPitch, SSTG_PIXFMT_32BPP, lpBltParams->bltDstFormat);
  else
      BLTFMT(dstPitch, SSTG_PIXFMT_16BPP, lpBltParams->bltDstFormat);
  BLTSIZE(dstWidth, dstHeight, lpBltParams->bltDstSize);
  BLTXY(dstX, dstY, lpBltParams->bltDstXY);

  BLTFMT(srcPitch, pixelFormat, lpBltParams->bltSrcFormat);  // 16 bpp for now
  BLTSIZE(srcWidth, srcHeight, lpBltParams->bltSrcSize);
  BLTXY(srcX, srcY, lpBltParams->bltSrcXY);

   dstSurfaceData = _DD(txtrSurface1);
  if(IS_NAPALM && dstSurfaceData)
  {
    DWORD lodmax, dwSrcSize, aspectRatio, tLOD;
    int  addrOffset;
    DWORD srcSurfWidth, srcSurfHeight;
    //find parameters for texture shrink
    //lpBltParams->dstWidth  = dstWidth;
   // lpBltParams->dstHeight = dstHeight;
    lpBltParams->dstPitch  = dstPitch;
	lpBltParams->max_x = (float)dstWidth;
	lpBltParams->max_y = (float)dstHeight;

    dstRight = srcSurfaceData->shrinkSrcWidth;
    dstBottom = srcSurfaceData->shrinkSrcHeight;
    dstWidth = dstRight - dstLeft;
    dstHeight = dstBottom - dstTop;

    bltDstBaseAddr = dstSurfaceData->hwPtr;

    #if ENABLE_TILED_HEAP
    if(IS_TILED(bltDstBaseAddr))
    {
        dstPitch = _DS(ddTileStride);
    }
    else
    #endif
    {
         dstPitch = dstSurfaceData->lPitch;
    }

    BLTCLIP(dstRight, dstBottom, lpBltParams->txtrclip1max);
    BLTFMT(dstPitch, SSTG_PIXFMT_32BPP, lpBltParams->txtrbltDstFormat);
    BLTSIZE(dstWidth, dstHeight, lpBltParams->txtrbltDstSize);

    srcSurfWidth  = dstSurfaceData->shrinkSrcWidth;
    srcSurfHeight = dstSurfaceData->shrinkSrcHeight;
    //find the tLOD
    tLOD = 0;

    if( srcSurfWidth > srcSurfHeight)
    {

        lpBltParams->max_s  = (float)(srcWidth * 256 ) / (float)srcSurfWidth;
        lpBltParams->max_t =  (float)(srcHeight * 256) / (float)srcSurfWidth;

        for( lodmax = 0; srcSurfWidth > (DWORD)(1 << lodmax); lodmax++)
            ;
        tLOD |=SST_LOD_S_IS_WIDER;

        for( aspectRatio= 0; (DWORD)(1 << aspectRatio ) < (srcSurfWidth / srcSurfHeight);
                    aspectRatio++)
            ;
        tLOD |= (aspectRatio) << SST_LOD_ASPECT_SHIFT;

    }
    else
    {
        lpBltParams->max_s  = (float)(srcWidth * 256 ) / (float)srcSurfHeight;
        lpBltParams->max_t =  (float)(srcHeight * 256) / (float)srcSurfHeight;

        for( lodmax = 0; srcSurfHeight >(DWORD)(1 << lodmax); lodmax++)
            ;
        for( aspectRatio= 0; (DWORD)(1 << aspectRatio) < (srcSurfHeight / srcSurfWidth);
                    aspectRatio++)
            ;
        tLOD |= (aspectRatio) << SST_LOD_ASPECT_SHIFT;
    }

    //big texture case

    if(lodmax > 8 )
    {
        dwSrcSize = srcSurfWidth * srcSurfHeight * 4;  //in byte
        lodmax = 11 - lodmax;
        tLOD |= SST_TBIG;
        tLOD |= (lodmax << (SST_LODMIN_SHIFT +2 )) |
                (lodmax << (SST_LODMAX_SHIFT +2 ));

        //changes it so that if max LOD = 10 ( 2K texture) lodmax = 2;
        lodmax = 3 - lodmax;

        for( addrOffset = 0; lodmax > 0 ; lodmax--)
        {
            addrOffset += dwSrcSize >>  (2 * (lodmax-1));
        }
    }
    else
    {
        // find the size of lod 3 ( 0 for H4)
        dwSrcSize = 256 * ( 256 >> aspectRatio) * 4;  //in byte
        lodmax = 8 - lodmax;
        tLOD |= ( lodmax << (SST_LODMIN_SHIFT +2 )) |
                ( lodmax << (SST_LODMAX_SHIFT +2 ));

        for( addrOffset = 0; lodmax > 0; lodmax--)
        {
            addrOffset -= dwSrcSize >> (2 *(lodmax-1));
        }
    }

    lpBltParams->tLOD      = tLOD;
    lpBltParams->srcAddrOff= addrOffset;

  }
}
#endif


/*----------------------------------------------------------------------
Function name:  Get_ScalingFactor

Description:    Calculates scale factor.

Return:         DWORD fpiScale
----------------------------------------------------------------------*/
DWORD Get_ScalingFactor(DWORD dwSrc, DWORD dwDst, int fracbits)
{
    float fScale;
    float fVal_bitmask;
    DWORD fpiScale = 0;
    DWORD bitmask = 1 << (fracbits-1);
    int i;

    if ( dwSrc >= dwDst )
       return(0x000fffff); //No scale-up

    if ( dwDst )
    {
       dwDst = dwDst;  //Stretch adjust

       fScale = (float)dwSrc/(float)dwDst;

       //val = (DWORD)(fScale * (float)0x000fffff);
       for ( i = 0; i < fracbits; i++ )
       {
            fVal_bitmask = (float)1.0/(float)(1 << (i+1));
            if ( fScale >= fVal_bitmask )
            {
                 fpiScale |= bitmask;
                 fScale -= fVal_bitmask;
            }
            if ( fScale == 0.0 )
                 break;
            bitmask >>= 1;
       }
       return(fpiScale);
    }
    else
    {
       return(0x000fffff); //No scale-up
    }
}

#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
/*----------------------------------------------------------------------
Function name: ShrinkOverlaySurface

Description:   Shrink the overlay to the shrink surface.

Return:        DD_OK
----------------------------------------------------------------------*/
DWORD ShrinkOverlaySurface(NT9XDEVICEDATA * ppdev, LPDDRAWI_DDRAWSURFACE_LCL psurf)
{
  DWORD bltDstBaseAddr, bltDstFormat, bltDstSize, bltDstXY ;
  DWORD bltRop, bltCommand;
  DWORD bltSrcBaseAddr, bltSrcFormat, bltSrcSize, bltSrcXY;

  DWORD dstLeft, dstTop, dstRight, dstBottom, dstWidth, dstHeight, dstPitch;
  DWORD srcLeft, srcTop, srcRight, srcBottom, srcWidth, srcHeight, srcPitch;
  DWORD srcX, srcY, dstX, dstY, clip1min, clip1max;
  DWORD packetHeader = 0;
  DWORD bumpNum = 13;
  DWORD pixelFormat;
  BOOL  fTextureShrink;
  FXSURFACEDATA *srcSurfaceData, *dstSurfaceData;
  FXSURFACEDATA  *dstSurfaceData2;
#ifndef WINNT
  WORD * pFlags = (WORD *)_FF(lpDeFlags);
  WORD SaveBusy = *pFlags;
  WORD tmpBusy=Set_Busy(pFlags);
#endif
  CMDFIFO_PROLOG(hwPtr);

#ifndef WINNT
  if (LOW_POWER_MODE(SaveBusy))
      {
      return DD_OK;
      }
  ppdev->gdiFlags |= SDATA_GDIFLAGS_2D_DIRTY;
#endif


  bltCommand = SSTG_ROP_SRC << SSTG_ROP0_SHIFT;
  srcSurfaceData = (FXSURFACEDATA*)(psurf->lpGbl->dwReserved1);

  dstSurfaceData = _DD(txtrSurface1);

  if(!_FF(dd3DSurfaceCount) && IS_NAPALM && dstSurfaceData )
  {

    fTextureShrink = TRUE;

    if(_FF(dwCurrentSKSurf) & 1)
    {
            dstSurfaceData2 = _DD(shrinkSurface2);
    }
    else
    {
            dstSurfaceData2 = _DD(shrinkSurface1);
    }
  }
  else
  {
    fTextureShrink = FALSE;

    if(_FF(dwCurrentSKSurf) & 1 )
    {
            dstSurfaceData = _DD(shrinkSurface2);
    }
    else
    {
            dstSurfaceData = _DD(shrinkSurface1);
    }
  }
  switch(srcSurfaceData->pixelFormat)
  {
    case SST_OVERLAY_PIXEL_YUYV422:
      pixelFormat = SSTG_PIXFMT_422YUV;
      break;
    case SST_OVERLAY_PIXEL_UYVY422:
      pixelFormat = SSTG_PIXFMT_422UYV;
      break;
    case SST_OVERLAY_PIXEL_RGB32U:
      pixelFormat = SSTG_PIXFMT_32BPP;
      break;
    default:
      pixelFormat = SSTG_PIXFMT_16BPP;
      break;
  }

  // get rectangle
  dstTop = 0;
  if(fTextureShrink)
  {
     //and Napalm
     dstRight = srcSurfaceData->shrinkSrcWidth;
     dstBottom = srcSurfaceData->shrinkSrcHeight;
     // copy
     bltCommand |= SSTG_BLT | SSTG_GO | SSTG_CLIPSELECT;
  }
  else
  {
     dstRight = srcSurfaceData->shrinkWidth;       //fix PRS4411
     dstBottom = srcSurfaceData->shrinkHeight;	   // fix for PRS 12592; removed +1 addition to height jmccartney 09/06/00
     // SHRINK
     bltCommand |= SSTG_STRETCH_BLT | SSTG_GO | SSTG_CLIPSELECT;

  }
  dstLeft = 0;

  srcTop = srcSurfaceData->shrinkSrcY;
  srcLeft = srcSurfaceData->shrinkSrcX;
  srcRight = srcLeft + srcSurfaceData->shrinkSrcWidth;
  srcBottom = srcTop + srcSurfaceData->shrinkSrcHeight;

  srcWidth = srcRight - srcLeft;
  srcHeight = srcBottom - srcTop;

  if ( dstRight > srcWidth )
   dstRight = srcWidth;

  if (dstBottom > srcHeight )
   dstBottom = srcHeight;

  dstWidth = dstRight - dstLeft;
  dstHeight = dstBottom - dstTop;

  // get base address
  bltSrcBaseAddr = GET_HW_ADDR(psurf);
  bltDstBaseAddr = dstSurfaceData->hwPtr;

//  bltDstBaseAddr = _FF(ddPrimarySurfaceData).hwPtr;
#if ENABLE_TILED_HEAP
  if(IS_TILED(bltSrcBaseAddr))
  {
    srcPitch = _DS(ddTileStride);
  }
  else
#endif
  {
    srcPitch =srcSurfaceData->lPitch;
  }

#if ENABLE_TILED_HEAP
  if(IS_TILED(bltDstBaseAddr))
  {
    dstPitch = _DS(ddTileStride);
  }
  else
#endif
  {
    dstPitch = dstSurfaceData->lPitch;
  }


#ifdef WINNT
  // make space to restore nt invariant regs
  bumpNum += 6;
#endif

  //if destination is above and left of src, we starts srccopy at the upper left corner
  // otherwise:
  srcX = srcLeft;
  srcY = srcTop;
  dstX = dstLeft;
  dstY = dstTop;

  BLTCLIP(dstLeft, dstTop, clip1min);
  BLTCLIP(dstRight, dstBottom, clip1max);

  // now stuff them in the hardware format
  if(IS_NAPALM && _DD(f32ShrinkOvl))
      BLTFMT(dstPitch, SSTG_PIXFMT_32BPP, bltDstFormat);
  else
     BLTFMT(dstPitch, SSTG_PIXFMT_16BPP, bltDstFormat);
  BLTSIZE(dstWidth, dstHeight, bltDstSize);
  BLTXY(dstX, dstY, bltDstXY);

  BLTFMT(srcPitch, pixelFormat, bltSrcFormat);  // 16 bpp for now
  BLTSIZE(srcWidth, srcHeight, bltSrcSize);
  BLTXY(srcX, srcY, bltSrcXY);

  bltRop = (SSTG_ROP_SRC << 16 )| (SSTG_ROP_SRC << 8 ) | SSTG_ROP_SRC;

  // write to hw
  packetHeader |= dstBaseAddrBit
                  | dstFormatBit
                  | ropBit
                  | srcBaseAddrBit
                  | clip1minBit
                  | clip1maxBit
                  | srcFormatBit
                  | srcSizeBit
                  | srcXYBit
                  | dstSizeBit
                  | dstXYBit
                  | commandBit;

  CMDFIFO_CHECKROOM(hwPtr, bumpNum);

  SETPH( hwPtr, CMDFIFO_BUILD_PK2( packetHeader ) );
  SETPD(hwPtr, ghw2D->dstBaseAddr, bltDstBaseAddr);
  SETPD(hwPtr, ghw2D->dstFormat, bltDstFormat);
  SETPD(hwPtr, ghw2D->rop, bltRop );
  SETPD(hwPtr, ghw2D->srcBaseAddr, bltSrcBaseAddr);
  SETPD(hwPtr, ghw2D->clip1min, clip1min);
  SETPD(hwPtr, ghw2D->clip1max, clip1max);
  SETPD(hwPtr, ghw2D->srcFormat, bltSrcFormat);
  SETPD(hwPtr, ghw2D->srcSize, bltSrcSize);
  SETPD(hwPtr, ghw2D->srcXY,bltSrcXY);
  SETPD(hwPtr, ghw2D->dstSize, bltDstSize);
  SETPD(hwPtr, ghw2D->dstXY,bltDstXY);
  SETPD(hwPtr, ghw2D->command, bltCommand);

// need to come back to this later -SS
#ifdef WINNT
  // restore nt invariant regs
  SETPH(hwPtr, CMDFIFO_BUILD_PK2(dstBaseAddrBit |
                                 dstFormatBit   |
                                 srcBaseAddrBit |
                                 commandExBit   |
                                 srcFormatBit));
  SETPD(hwPtr, ghw2D->dstBaseAddr, ppdev->ulScreenOffset);
  SETPD(hwPtr, ghw2D->dstFormat, ppdev->ulScreenFormat);
  SETPD(hwPtr, ghw2D->srcBaseAddr, ppdev->ulScreenOffset);
  SETPD(hwPtr, ghw2D->commandEx, 0);
  SETPD(hwPtr, ghw2D->srcFormat, ppdev->ulScreenFormat);
#endif
  BUMP(bumpNum)

  if(fTextureShrink)
  {
     CMDFIFO_SAVE(hwPtr);
     shrinkTexture( ppdev, bltDstBaseAddr, dstSurfaceData2->hwPtr,
        dstSurfaceData2->lPitch,
        dstSurfaceData->shrinkSrcWidth,  dstSurfaceData->shrinkSrcHeight,
        srcSurfaceData->shrinkSrcWidth,  srcSurfaceData->shrinkSrcHeight,
        srcSurfaceData->shrinkWidth,     srcSurfaceData->shrinkHeight + 1);
     CMDFIFO_RELOAD(hwPtr);
  }

  CMDFIFO_EPILOG(hwPtr);
#ifndef WINNT
  RESTORE_BUSY(pFlags,SaveBusy);
#endif
  return DD_OK;

} // ShrinkOverlaySurface


/**************************************************************
* shrink the soure texture at srcAddr into dstAddr
*    Entry:
*       srcAddr       source surface address
*       dstAddr       destination surface address
*       dstPitch      pitch of destination surface
*       srcSurfWidth  source surface width in pixel
*       srcSurfHeiht  source surface height
*       srcWidth      source image width
*       srcHeight     source image height
*       dstWidth      destnation image width
*       dstHeight     destnation image height
*
* Note:  srcSurfWidth and srcSurfHeight are power of 2 numbers.
**************************************************************/

#pragma  optimize ("",off)
void shrinkTexture(NT9XDEVICEDATA  *ppdev, DWORD srcAddr, DWORD dstAddr,
        DWORD dstPitch,DWORD srcSurfWidth, DWORD srcSurfHeight,
        DWORD srcWidth, DWORD srcHeight,DWORD dstWidth, DWORD dstHeight)
{
  DWORD tLOD,lodmax, dwSrcSize, aspectRatio;
  int addrOffset;
  float max_s, max_t;
  CMDFIFO_PROLOG(cmdFifo);
  tLOD = 0;
  lodmax = 0;
  //find the tLOD
  if( srcSurfWidth > srcSurfHeight)
  {
    //for some reason the first time max_s could be zero for WINDVD 1.299
    max_s  = (float)(srcWidth * 256) / (float)srcSurfWidth;  
    max_t =  (float)(srcHeight * 256) / (float)srcSurfWidth;
    max_s  = (float)(srcWidth * 256 ) / (float)srcSurfWidth;

    for( lodmax = 0; srcSurfWidth > (DWORD)(1 << lodmax); lodmax++)
        ;
    tLOD |=SST_LOD_S_IS_WIDER;

    for( aspectRatio= 0; (DWORD)(1 << aspectRatio ) < (srcSurfWidth / srcSurfHeight);
                aspectRatio++)
        ;
    tLOD |= (aspectRatio) << SST_LOD_ASPECT_SHIFT;

  }
  else
  {
    //for some reason the first time max_s could be zero for WINDVD 1.299
    max_s  = (float)(srcWidth * 256 ) / (float)srcSurfHeight;
    max_t =  (float)(srcHeight * 256) / (float)srcSurfHeight;
    max_s  = (float)(srcWidth * 256 ) / (float)srcSurfHeight;

    for( lodmax = 0; srcSurfHeight >(DWORD)(1 << lodmax); lodmax++)
       ;
    for( aspectRatio= 0; (DWORD)(1 << aspectRatio) < (srcSurfHeight / srcSurfWidth);
                aspectRatio++)
        ;
    tLOD |= (aspectRatio) << SST_LOD_ASPECT_SHIFT;
  }

  //big texture case

  if(lodmax > 8 )
  {
     dwSrcSize = srcSurfWidth * srcSurfHeight * 4;  //in byte
     lodmax = 11 - lodmax;
     tLOD |= SST_TBIG;
     tLOD |= (lodmax << (SST_LODMIN_SHIFT +2 )) |
             (lodmax << (SST_LODMAX_SHIFT +2 ));

     //changes it so that if max LOD = 10 ( 2K texture) lodmax = 2;
     lodmax = 3 - lodmax;

     for( addrOffset = 0; lodmax > 0 ; lodmax--)
     {
        addrOffset += dwSrcSize >>  (2 * (lodmax-1));
     }
  }
  else
  {
    // find the size of lod 3 ( 0 for H4)
     dwSrcSize = 256 * ( 256 >> aspectRatio) * 4;  //in byte

     lodmax = 8 - lodmax;
     tLOD |= ( lodmax << (SST_LODMIN_SHIFT +2 )) |
             ( lodmax << (SST_LODMAX_SHIFT +2 ));

     for( addrOffset = 0; lodmax > 0; lodmax--)
     {
        addrOffset -= dwSrcSize >> (2 *(lodmax-1));
     }
  }

  srcAddr += addrOffset;
  srcAddr = (srcAddr & 0x1FFFFFF) + ((srcAddr & 0x2000000) >> 24);

  FXBUSYWAIT(ppdev);  /* retro3dfx: bounded */   //wait for blt done

  CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + 20 + 7 * PH1_SIZE + 12 + 3 * ( PH4_SIZE +3));
  // Reset the clipping registers
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, clipLeftRight, 0xf ) );
  SETPD( cmdFifo, ghw0->clipLeftRight, dstWidth);
  SETPD( cmdFifo, ghw0->clipBottomTop, dstHeight);

  SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, colBufferAddr, 0x0 ) );
  SETPD( cmdFifo, ghw0->colBufferAddr,   dstAddr);
  SETPD( cmdFifo, ghw0->colBufferStride, dstPitch);

  //set textureMode for TMU1
  //don't care
/*
  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R3, textureMode, TMU2CHIP(TREX1)) );
  SETPD( cmdFifo, SST_TREX(ghw0,TREX1)->textureMode, SST_TCLAMPS|
                                                     SST_TCLAMPT|
                         (TEXFMT_ARGB_8888 << SST_TFORMAT_SHIFT));
  SETPD( cmdFifo, SST_TREX(ghw0,TREX1)->tLOD, tLOD );
  SETPD( cmdFifo, SST_TREX(ghw0,TREX1)->texBaseAddr, srcAddr);
*/
  //set textureMode for TMU0
  //loads texture and enbales bi-linear filter
  SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R3, textureMode, TMU2CHIP(TREX0)));
  SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->textureMode,
                                                     SST_TCLAMPS|
                                                     SST_TCLAMPT|
                         (TEXFMT_ARGB_8888 << SST_TFORMAT_SHIFT)|
                        SST_TC_REPLACE | SST_TCA_REPLACE |
                        SST_TMINFILTER | SST_TMAGFILTER);
  SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->tLOD, tLOD );
  SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->texBaseAddr, srcAddr);

  // filtered, no fog, no alpha blending, no z buffering
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 4, 1, fbzColorPath, 0xf ) );
  SETPD( cmdFifo, ghw->fbzColorPath, SST_ENTEXTUREMAP | SST_PARMADJUST |
                            SST_RGBAZ_CLAMP );
  SETPD( cmdFifo, ghw->fogMode, 0 );
  SETPD( cmdFifo, ghw->alphaMode, 0 );
  SETPD( cmdFifo, ghw->fbzMode, SST_RGBWRMASK );
  //need a NOP
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1(1, 0, nopCMD, 0xF));
  SETPD( cmdFifo, ghw->nopCMD,0);
  //combineMode for LFB
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 1 ) );
  SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, SST_CM_CC_OTHERSELECT_TRGB |
                                    SST_CM_USE_COMBINE_MODE );
  //combineMode for TMU0
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 2 ) );
  SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU0)->combineMode, SST_CM_TC_LOCALSELECT_LOCAL_TRGB |
                                    SST_CM_USE_COMBINE_MODE );
  //combineMode for TMU1
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 4 ) );
  SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU1)->combineMode,  SST_CM_TC_LOCALSELECT_LOCAL_TRGB |
                                     SST_CM_USE_COMBINE_MODE );

  SETPH( cmdFifo, CMDFIFO_BUILD_PK4( R0|R1|R2, renderMode, 0 ) );
  SETPD( cmdFifo, ghw->renderMode, SST_RM_32BPP       |
								   SST_RM_YORIGIN_SELECT|	
                                   SST_RM_RED_WMASK   |
                                   SST_RM_GREEN_WMASK |
                                   SST_RM_BLUE_WMASK );
  SETPD( cmdFifo, ghw->stencilMode, SST_STENCIL_MODE_DISABLE);
  SETPD( cmdFifo, ghw->stencilOp, 0); // D3DSTENCILOP_KEEP

  SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_START, 4, (SST_SETUP_ST0 | SST_SETUP_W0 |SST_SETUP_FAN), 1) );
  SETFPD( cmdFifo, ghw0->sVx, 0.0f );
  SETFPD( cmdFifo, ghw0->sVy, 0.0f );
  SETFPD( cmdFifo, ghw0->sOow0, 1.0f );
  SETFPD( cmdFifo, ghw0->sSow0, 0.0f );
  SETFPD( cmdFifo, ghw0->sTow0, 0.5f );

  SETFPD( cmdFifo, ghw0->sVx, 0.0f            );
  SETFPD( cmdFifo, ghw0->sVy, (float) dstHeight);
  SETFPD( cmdFifo, ghw0->sOow0, 1.f );
  SETFPD( cmdFifo, ghw0->sSow0, 0.0f );
  SETFPD( cmdFifo, ghw0->sTow0, max_t);

  SETFPD( cmdFifo, ghw0->sVx,  (float) dstWidth);
  SETFPD( cmdFifo, ghw0->sVy,  (float) dstHeight);
  SETFPD( cmdFifo, ghw0->sOow0, 1.f );
  SETFPD( cmdFifo, ghw0->sSow0, max_s);
  SETFPD( cmdFifo, ghw0->sTow0, max_t);

  SETFPD( cmdFifo, ghw0->sVx, (float) dstWidth );
  SETFPD( cmdFifo, ghw0->sVy,   0.0f          );
  SETFPD( cmdFifo, ghw0->sOow0, 1.f );
  SETFPD( cmdFifo, ghw0->sSow0, max_s);
  SETFPD( cmdFifo, ghw0->sTow0, 0.5f);

  CMDFIFO_EPILOG( cmdFifo );
}
#endif
