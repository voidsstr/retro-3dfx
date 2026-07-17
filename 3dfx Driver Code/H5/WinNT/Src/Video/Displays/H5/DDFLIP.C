/* $Header: ddflip.c, 25, 10/11/00 8:57:03 PM, Brent$ */
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
** File Name: 	DDFLIP.C
**
** Description: DirectDraw flipping related functions.
**
** $Revision: 25$
** $Date: 10/11/00 8:57:03 PM$
**
*/

/*******************************************************************************
*
* DIRECTDRAW FUNCTIONS:
*
* DdFlip                    --- DirectDraw Flip entry point.
* DdGetFlipStatus           --- DirectDraw GetFlipStatus entry point.
* DdWaitForVerticalBlank    --- DirectDraw WaitForVerticalBlank entry point.
* DdGetScanLine             --- DirectDraw GetScanLine entry point.
* DdFlipToGDISurface        --- DirectDraw FlipToGDISurface entry point.
*
* EXPORTED FUNCTIONS:
*
* FxgetFlipStatus           --- Checks if the most recent flip has completed.
* FxInVerticalBlank         --- Checks if the chip is in vertical blank.
* FxGetScanLine             --- Returns vertical scanline position.
* Set_Busy                  --- Set the DIB engine busy bit.
*
* INTERNAL FUNCTIONS:
*
* Promote_CmdFifoToAGP      --- Switch to using the AGP command fifo.
* Demote_CmdFifoFromAGP     --- Switch to using the video command fifo.
* DoConfig                  --- Call miniVDD to enable AGP.
* UpdateIMask               --- Call miniVDD to update IMASK.
*
*******************************************************************************/

/***************************************************************************
* I N C L U D E S
****************************************************************************/

#include "precomp.h"

// STB Begin Changes
#ifdef INCSTBPERF
#ifdef WINNT
#if (_WIN32_WINNT < 0x0500)
#include "..\..\..\..\build\stbperf.inc"
#endif
#else
#include "..\build\stbperf.inc"
#endif
#endif
// STB End Changes

/**************************************************************************
* D E F I N E S
***************************************************************************/

// Increment swap buffer pending field in status register (Avenger/Napalm only)

#define INCSWAPCOUNT()    SETDW(ghw0->swapBufferPend, 0);
#define READSWAPCOUNT()   ((GET(ghwIO->status) & SST_SWAPBUFPENDING) >> SST_SWAPBUFPENDING_SHIFT)

/*******************************************************************/
/*                     DIRECTDRAW FUNCTIONS                        */
/*******************************************************************/

/*----------------------------------------------------------------------
Function name:  DdFlip

Description:    DirectDraw Flip entry point.

                This callback is invoked whenever we are about to flip to
                from one surface to another.   pfd->lpSurfCurr is the
                surface we were at, pfd->lpSurfTarg is the one we are
                flipping to.

                You should point the hardware registers at the new surface,
                and also keep track of the surface that was flipped away
                from, so that if the user tries to lock it, you can be sure
                that it is done being displayed

Return:         DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

#ifdef SLI_AA
#ifndef WINNT
int HostRestoreCursor(NT9XDEVICEDATA * ppdev, DWORD SaveCursorSurface, int LastCursorX, int LastCursorY, DWORD dwExclusionSave);
int HostDrawCursor(NT9XDEVICEDATA * ppdev, DWORD swapToAddr);
#endif
#endif
DWORD __stdcall
DdFlip( LPDDHAL_FLIPDATA pfd )
{
// can't use DD_ENTRY_SETUP because CMDFIFO need pdev
  #ifdef WINNT
    PDEV  *ppdev = pfd->lpDD->dhpdev;
  #else
    NT9XDEVICEDATA  *ppdev = (NT9XDEVICEDATA *)pfd->lpDD->dwReserved3;
  #endif

  DWORD swapToAddr;
  DWORD waitOnVsync;
#ifndef WINNT
   WORD * pFlags = (WORD *)_FF(lpDeFlags);
   WORD SaveBusy = *pFlags;
   WORD TrashBusy = Set_Busy(pFlags);
#endif
#ifdef SLI_AA
#ifndef WINNT
   DWORD SaveCursorSurface;
   int LastCursorX;
   int LastCursorY;
#endif
#endif
  CMDFIFO_PROLOG(hwPtr);

#if !defined(WINNT) && !defined(GBLDATA_IN_PDEV)
  if (LOW_POWER_MODE(SaveBusy))
      {
      pfd->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;
      }
  ppdev->gdiFlags |= SDATA_GDIFLAGS_2D_DIRTY;
#endif

  DDPRINT(DDDBGLVL, ">> DdFlip (lpSurfCurr=%08lXh, lpSurfTarg=%08lXh", pfd->lpSurfCurr, pfd->lpSurfTarg);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "Flip32" ));
  DUMP_DDHAL_FLIPDATA(ppdev, DEBUG_DDGORY, pfd );
  #endif

  // If waitOnVsync is set to 1, flip will wait for VSYNC.
  // If waitOnVsync is set to 0, flip will happen immediately.

  waitOnVsync = _DD(WaitOnVsync);  // wait on VSYNC, unless registry overrides.

#ifdef STBPERF_USE_FLIPNOVSYNC
  if( pfd->dwFlags & DDFLIP_NOVSYNC )
  {
    waitOnVsync = 0; // application wants to ignore VSYNC
  }
#endif

  /* Store addresses of current (flipped from) and target (flipped to) surfaces. */

  _DD(ddSurfaceFlippedFrom) = pfd->lpSurfCurr->lpGbl->fpVidMem;
  _DD(ddSurfaceFlippedTo)   = pfd->lpSurfTarg->lpGbl->fpVidMem;

#ifdef Z_ACCESS_OPT
  if ( _DD(ddEnableZClearOpt) )
  {
    // We need at least 2 buffers and a Z
    // We also do not want to have this optimization on if there are more than one 3D
    // surface active.  So 3 buffers + 1 Z Buffer = 4 Surfaces
    if ( _DD(dd3DInOverlay) && (_FF(dd3DSurfaceCount) > 2) && (_FF(dd3DSurfaceCount) < 5) )
    { 
	  if ( (pfd->lpSurfTarg->ddsCaps.dwCaps & DDSCAPS_ZBUFFER) )
      {
        // Steve Rogers pointed out that some games flip the ZBuffer instead
        // of clearing it, we don't want this
	    if ( ((_DD(ddFlipsWithoutZClear) & 0x7fffffff) > 6) )
	    {
          _DD(ddFlipsWithoutZClear) = 0x80000000;
        }  
	    else 
	    {
	      // We are not coming back from the Optimization, so we do not need to
          // reset
	      _DD(ddFlipsWithoutZClear) = 0;
        } 
	  }
	  else // We are not touching the ZBuffer in any way, so lets increment the counter
	  {
        _DD(ddFlipsWithoutZClear)++;
	  }
    }
    else
    {
	  // We are not in fullscreen triple or double buffered mode so lets null the optimization
      _DD(ddFlipsWithoutZClear) = 0;
    }

  }
#endif

  /* Wait on previous flip, if not 3D rendering surface. */

  if (!_DD(dd3DInOverlay))
  {
    if (pfd->dwFlags & DDFLIP_WAIT)
    {
      while (FXGETFLIPSTATUS(ppdev));
    }
    else if (FXGETFLIPSTATUS(ppdev))
    {
#ifndef WINNT
      RESTORE_BUSY(pFlags,SaveBusy);
#endif
      pfd->ddRVal = DDERR_WASSTILLDRAWING;
      DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pfd->ddRVal);
      return DDHAL_DRIVER_HANDLED;
    }
  }

  /* Flip the desktop surface. */

  ppdev->flip_pending = 1;

  if(!(pfd->lpSurfTarg->ddsCaps.dwCaps & DDSCAPS_OVERLAY) && (!_DD(dd3DInOverlay)))
  {
    /* Banshee cannot pipeline writes to the desktop  */
	/* start address, so pipeline must be flushed.    */
	/* If a flip is already in progress, the flush    */
	/* will end up waiting on vertical retrace. -CGW- */

    FXBUSYWAIT(ppdev);
    SETDW(ghwIO->vidDesktopStartAddr, GET_HW_ADDR(pfd->lpSurfTarg));
#ifdef WIN_CSIM
    SETDW(((SstIORegs *)_FF(regRealBase))->vidDesktopStartAddr, GET_HW_ADDR(pfd->lpSurfTarg));
#endif
#if defined(WINNT) && defined(CSIM)
    // write start addr to real V3 hardware
    ((SstIORegs *)ppdev->CSIM_pjBase)->vidDesktopStartAddr = GET_HW_ADDR(pfd->lpSurfTarg);
#endif

    INCSWAPCOUNT();
    CMDFIFO_CHECKROOM(hwPtr, 2);
    SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, swapbufferCMD, 0xF) );
    SETPD(hwPtr, ghw0->swapbufferCMD, waitOnVsync);
    BUMP(2);
    CMDFIFO_EPILOG(hwPtr);

#ifndef WINNT
#ifdef AGP_CMDFIFO
    {
      void FLUSHAGP(NT9XDEVICEDATA * ppdev);

      if (_FF(doAgpCF))
        FLUSHAGP(ppdev);
    }
#endif // #ifdef AGP_CMDFIFO
#endif // #ifndef WINNT

#ifndef WINNT
    RESTORE_BUSY(pFlags,SaveBusy);
#endif
    pfd->ddRVal = DD_OK;
    DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pfd->ddRVal);
    return DDHAL_DRIVER_HANDLED;
  }
  swapToAddr = GET_HW_ADDR(pfd->lpSurfTarg) & ~SSTG_IS_TILED;

  if(pfd->lpSurfTarg->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
  {
    FXSURFACEDATA *surSurfaceData, *tarSurfaceData;
    surSurfaceData = (FXSURFACEDATA*) pfd->lpSurfCurr->lpGbl->dwReserved1;
    tarSurfaceData = (FXSURFACEDATA*) pfd->lpSurfTarg->lpGbl->dwReserved1;
#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
    if(surSurfaceData->overlayShrinkFlag == TRUE)
    {
      CMDFIFO_SAVE( hwPtr);
    }
#endif
    FlipOverlaySurface(ppdev, pfd,surSurfaceData, tarSurfaceData,&swapToAddr);
#if (USE_NT5_DDMEMMGR) || !defined(WINNT)
    if(surSurfaceData->overlayShrinkFlag == TRUE)
    {
      CMDFIFO_RELOAD(hwPtr);

    }
#endif
    if( pfd->ddRVal != DD_OK)
    {
#ifndef WINNT
        RESTORE_BUSY(pFlags,SaveBusy);
#endif
        DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pfd->ddRVal);
        return DDHAL_DRIVER_HANDLED;
    }
  }

#if defined(CMDFIFO)
  {
    DWORD swapsQueued;

    if (waitOnVsync)
    {
       // VSYNC enabled, limit number of swap buffers for performance reasons.
       swapsQueued = _DD(WaitOnVsync) + 1; // allow 3 swap buffers
    }
    else
    {
       // VSYNC disabled, limit number of swap buffers to avoid hardware hang.
       swapsQueued = 6;  // allow 7 swap buffers
    }
    while (READSWAPCOUNT() > swapsQueued);
  }
#endif // #if defined(CMDFIFO) || defined(H3_FIFO)

  /* Flip the overlay surface. */

  INCSWAPCOUNT();

#ifdef SLI_AA
#if defined(WINNT) && (_WIN32_WINNT < 0x0500)
#else
// this seems to break the windows simulator.  readd when it does not
#ifndef WIN_CSIM
  if (_DD(ddAAModeEnabled))
  {
    DWORD SecondaryAddr;

    SecondaryAddr = ((FXSURFACEDATA*)(pfd->lpSurfTarg->lpGbl->dwReserved1))->AAhwPtr & ~SSTG_IS_TILED;

    CMDFIFO_CHECKROOM(hwPtr, 2);
    SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, leftDesktopBuf, 0xF ));
    SETPD(hwPtr, ghw0->leftDesktopBuf, SecondaryAddr);
    BUMP(2);

    // This causes tearing if DDFLIP_NOVSYNC, since the desktop does
    // not flip at the same time as the overlay in A0 Napalm hardware ;-(

    waitOnVsync |= SST_SWAP_DESKTOP_EN; // Swap desktop and overlay buffers.
  }
#endif
#endif
#endif // SLI_AA

#ifdef SLI_AA
#ifndef WINNT
   _FF(CurrentSurface) = _DD(ddSurfaceFlippedTo);
   _FF(FlipCount)++;
   if (_DD(ddAAModeEnabled) || _DD(ddSLIModeEnabled) || ((2 == _DD(ddAANumberSamples)) && (2 == _FF(dwNumUnits))))
   {
     // if cursor is enabled but not excluded
     if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
     {
       FXBUSYWAIT(ppdev);
       memcpy((void *)_DD(ddHostcursorExclusionSave), (void *)_FF(HostcursorExclusionStart), 4096);
       SaveCursorSurface = _FF(CursorSurface);
       LastCursorX = _FF(LastCursorPosX);
       LastCursorY = _FF(LastCursorPosY);
       HostDrawCursor(ppdev, _DD(ddSurfaceFlippedTo));
     }
   }
#endif
#endif

  CMDFIFO_CHECKROOM(hwPtr, 4);
  SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, leftOverlayBuf, 0xF ));
  SETPD(hwPtr, ghw0->leftOverlayBuf, swapToAddr);
  SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, swapbufferCMD, 0xF) );
  SETPD(hwPtr, ghw0->swapbufferCMD, waitOnVsync);
  BUMP(4);
  CMDFIFO_EPILOG(hwPtr);

#ifdef SLI_AA
#ifndef WINNT
  if (_DD(ddAAModeEnabled) || _DD(ddSLIModeEnabled) || ((2 == _DD(ddAANumberSamples)) && (2 == _FF(dwNumUnits))))
  {
    // if cursor is enabled but not excluded
    if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
    {
      // Wait for Flip!!!!!
      FXBUSYWAIT(ppdev);
      HostRestoreCursor(ppdev, SaveCursorSurface, LastCursorX, LastCursorY, _DD(ddHostcursorExclusionSave));
    }
  }
#endif
#endif

#ifdef WIN_CSIM
  {
    SstRegs FAR * l3dRegs = (SstRegs *)(_FF(regRealBase) + SST_3D_OFFSET);
    SETDW(l3dRegs->leftOverlayBuf, swapToAddr);
    SETDW(l3dRegs->swapbufferCMD, waitOnVsync);
  }
#endif

  _FF(lastOverlayAddress) = swapToAddr & 0x00FFFFFF;

#ifndef WINNT
#ifdef AGP_CMDFIFO
  {
    void  FLUSHAGP(NT9XDEVICEDATA * ppdev);
    FLUSHAGP(ppdev);
  }
#endif // #ifdef AGP_CMDFIFO

   RESTORE_BUSY(pFlags,SaveBusy);
#endif

  pfd->ddRVal = DD_OK;
  DDPRINT(DDDBGLVL, "<< (retval = %08lXh)", pfd->ddRVal);
  return DDHAL_DRIVER_HANDLED;

} // DdFlip

/*----------------------------------------------------------------------
Function name:  DdGetFlipStatus

Description:    DirectDraw GetFlipStatus entry point.

                If the display has went through one refresh cycle since the
                flp occurred we return DD_OK.  If it has not went through
                one refresh cycle we return DDERR_WASSTILLDRAWING to
                indicate that this surface is still busy "drawing" the
                flipped page.   We also return DDERR_WASSTILLDRAWING if the
                bltter is busy and the caller wanted to know if they could
                flip yet

Return:        	DWORD DDRAW result

                DDHAL_DRIVER_HANDLED
----------------------------------------------------------------------*/

DWORD __stdcall
DdGetFlipStatus( LPDDHAL_GETFLIPSTATUSDATA pgd )
{
  DD_ENTRY_SETUP(pgd->lpDD);
  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "GetFlipStatus32" ));
  DUMP_GETFLIPSTATUSDATA(ppdev, DEBUG_DDGORY, pgd );
  #endif

  pgd->ddRVal = DD_OK;

  if (pgd->dwFlags == DDGFS_ISFLIPDONE)
  {
    // Only check flip status if flip is pending.

    if (ppdev->flip_pending)
    {
      // Ignore device status and return DD_OK for 3D applications,
      // otherwise return DDERR_WASSTILLDRAWING if flip in progress.

      if (!_DD(dd3DInOverlay))
      {
        /* Return flip status only for surface flipped from or to. */
	    if (( pgd->lpDDSurface->lpGbl->fpVidMem == _DD(ddSurfaceFlippedFrom)) ||
            ( pgd->lpDDSurface->lpGbl->fpVidMem == _DD(ddSurfaceFlippedTo)))
		{
          /* Check primary surface or video overlay flipping status. */

          if ( FXGETFLIPSTATUS(ppdev) )
		  {
            pgd->ddRVal = DDERR_WASSTILLDRAWING;
		  }
		  else
          {
            ppdev->flip_pending = 0;
          }
		}
	  }
	}
  }
  else if (pgd->dwFlags == DDGFS_CANFLIP)
  {
    // Only check flip status if flip is pending.

    if (ppdev->flip_pending)
	{
      // Ignore device status and return DD_OK for 3D applications,
      // otherwise return DDERR_WASSTILLDRAWING if device is busy.

      if (!_DD(dd3DInOverlay))
      {
	    if ( FXGETBUSYSTATUS(ppdev) )
		{
          pgd->ddRVal = DDERR_WASSTILLDRAWING;
		}
        else
        {
          ppdev->flip_pending = 0;
        }
	  }
	}
  }
  else
  {
    pgd->ddRVal = DDERR_INVALIDPARAMS;
  }
  return DDHAL_DRIVER_HANDLED;

} // DdGetFlipStatus


/*----------------------------------------------------------------------
Function name: DdWaitForVerticalBlank

Description:   DirectDraw WaitForVerticalBlank entry point.

               Three requests:

               DDWAIT_I_TESTVB     - vblank status request
               DDWAITVB_BLOCKBEGIN - wait until vblank begins (and display ends)
               DDWAUTVB_BLOCKEND   - wait until vblank ends (and display begins)

Return:        DWORD DDRAW result

               DDHAL_DRIVER_HANDLED    - waited for vblank period end
               DDHAL_DRIVER_NOTHANDLED - invalid request
----------------------------------------------------------------------*/

DWORD __stdcall
DdWaitForVerticalBlank(LPDDHAL_WAITFORVERTICALBLANKDATA pwvb)
{
  DD_ENTRY_SETUP(pwvb->lpDD);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "WaitForVerticalBlank32" ));
  DUMP_WAITFORVERTICALBLANKDATA(ppdev, DEBUG_DDGORY, pwvb );
  #endif

  switch( pwvb->dwFlags )
  {
    case DDWAITVB_I_TESTVB:

      /* vblank status request */

      pwvb->bIsInVB = FXINVERTICALBLANK(ppdev);

      pwvb->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;

    case DDWAITVB_BLOCKBEGIN:

      /* wait until vblank begins (and display ends) */

      while (FXINVERTICALBLANK(ppdev));
      while (!FXINVERTICALBLANK(ppdev));

      pwvb->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;

    case DDWAITVB_BLOCKEND:

      /* wait until vblank ends (and display begins) */

      if (FXINVERTICALBLANK(ppdev))
      {
		while (FXINVERTICALBLANK(ppdev));
      }
	  else
      {
	    while (! FXINVERTICALBLANK(ppdev));
        while (FXINVERTICALBLANK(ppdev));
      }

      pwvb->ddRVal = DD_OK;
      return DDHAL_DRIVER_HANDLED;
  }

  return DDHAL_DRIVER_NOTHANDLED;

} // DdWaitForVerticalBlank


/*----------------------------------------------------------------------
Function name:  DdGetScanLine

Description:    DirectDraw GetScanLine entry point.

                If a vertical blank is in progress the scan line is in
			    indeterminant, and we return DDERR_VERTICALBLANKINPROGRESS.
			    Otherwise we return the scan line and a success code
			
Return:         DWORD DDRAW result

                DDERR_VERTICALBLANKINPROGRESS - scanline is in progress
			    DDHAL_DRIVER_HANDLED          - scanline is returned in pgsl->dwScanLine
----------------------------------------------------------------------*/

DWORD __stdcall
DdGetScanLine( LPDDHAL_GETSCANLINEDATA pgsl )
{
  DD_ENTRY_SETUP(pgsl->lpDD);

  #ifdef FXTRACE
  DISPDBG((ppdev, DEBUG_APIENTRY, "GetScanLine32" ));
  DUMP_GETSCANLINE(ppdev, DEBUG_DDGORY, pgsl );
  #endif

  if (FXINVERTICALBLANK(ppdev))
  {
    pgsl->dwScanLine = 0;
    pgsl->ddRVal = DDERR_VERTICALBLANKINPROGRESS;
  }
  else
  {
    pgsl->dwScanLine = FXGETSCANLINE(ppdev);
    pgsl->ddRVal = DD_OK;
  }
  return DDHAL_DRIVER_HANDLED;

} // DdGetScanLine


/*----------------------------------------------------------------------
Function name:  DdFlipToGDISurface

Description:    DirectDraw FlipToGDISurface entry point.

Return:         DWORD DDRAW result

                DDHAL_DRIVER_NOTHANDLED
----------------------------------------------------------------------*/

#ifndef WINNT

DWORD __stdcall
DdFlipToGDISurface( LPDDHAL_FLIPTOGDISURFACEDATA pftgs )
{
  NT9XDEVICEDATA *ppdev = (NT9XDEVICEDATA *)(pftgs->lpDD->dwReserved3);
  WORD * pFlags = (WORD *)_FF(lpDeFlags);
  WORD SaveBusy = *pFlags;
  WORD TrashBusy = Set_Busy(pFlags);
#ifdef SLI_AA
#ifndef WINNT
  DWORD SaveCursorSurface;
  int LastCursorX;
  int LastCursorY;
#endif
#endif

  if (LOW_POWER_MODE(SaveBusy))
  {
    pftgs->ddRVal = DD_OK;
    return DDHAL_DRIVER_NOTHANDLED;
  }
  ppdev->gdiFlags |= SDATA_GDIFLAGS_2D_DIRTY;

  if( pftgs->dwToGDI )
  {
    CMDFIFO_PROLOG(hwPtr);

    if(!_DD(dd3DInOverlay))
    {
      FXBUSYWAIT(ppdev);
      SETDW(ghwIO->vidDesktopStartAddr, _DS(gdiDesktopStart));
      CMDFIFO_CHECKROOM(hwPtr, 2);
      SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, swapbufferCMD, 0xF) );
      SETPD(hwPtr, ghw0->swapbufferCMD, 0);
      BUMP(2);
    }
    else
    {
#ifdef SLI_AA
#ifndef WINNT
      _FF(CurrentSurface) = _FF(ddPrimarySurfaceData).lfbPtr;
      _FF(FlipCount)++;
      if (_DD(ddAAModeEnabled) || _DD(ddSLIModeEnabled) || ((2 == _DD(ddAANumberSamples)) && (2 == _FF(dwNumUnits))))
         {
         // if cursor is enabled but not excluded
         if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
            {
            FXBUSYWAIT(ppdev);
            memcpy((void *)_DD(ddHostcursorExclusionSave), (void *)_FF(HostcursorExclusionStart), 4096);
            SaveCursorSurface = _FF(CursorSurface);
            LastCursorX = _FF(LastCursorPosX);
            LastCursorY = _FF(LastCursorPosY);
            HostDrawCursor(ppdev, _FF(ddPrimarySurfaceData).lfbPtr);
            }
         }
#endif
#endif
      CMDFIFO_CHECKROOM(hwPtr, 4);
      // guarantees that vidCurrOverlayStartAddress (readable reg) is updated
      SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, leftOverlayBuf, 0xF ));
      SETPD(hwPtr, ghw0->leftOverlayBuf,_DS(gdiDesktopStart));
      SETPH(hwPtr, CMDFIFO_BUILD_PK1(1, 0, swapbufferCMD, 0xF) );

      // If AA Make sure both surfaces are swapped
      if (_DD(ddAAModeEnabled))
      {
        SETPD(hwPtr, ghw0->swapbufferCMD, SST_SWAP_DESKTOP_EN);
      }
      else
      {
        SETPD(hwPtr, ghw0->swapbufferCMD, 0);
      }

#ifdef SLI_AA
#ifndef WINNT
      if (_DD(ddAAModeEnabled) || _DD(ddSLIModeEnabled) || ((2 == _DD(ddAANumberSamples)) && (2 == _FF(dwNumUnits))))
         {
         // if cursor is enabled but not excluded
         if (SDATA_GDIFLAGS_CURSOR_ENABLED == (_FF(gdiFlags) & (SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_ENABLED)))
            {
            // Wait for Flip!!!!!
            FXBUSYWAIT(ppdev);
            HostRestoreCursor(ppdev, SaveCursorSurface, LastCursorX, LastCursorY, _DD(ddHostcursorExclusionSave));
            }
         }
#endif
#endif
      BUMP(4);
      _FF(lastOverlayAddress) = INVALID_ADDRESS;
    }

    Msg(ppdev, DEBUG_APIENTRY, "FlipToGDISurface32 (to GDI)" );
    CMDFIFO_EPILOG(hwPtr);
  }
  else
  {
    Msg(ppdev, DEBUG_APIENTRY, "FlipToGDISurface32 (from GDI)" );
  }

  RESTORE_BUSY(pFlags,SaveBusy);

  pftgs->ddRVal = DD_OK;
  return DDHAL_DRIVER_NOTHANDLED;

} // DdFlipToGDISurface

#endif // ifndef WINNT


/*******************************************************************/
/*                     EXPORTED FUNCTIONS                          */
/*******************************************************************/


/*----------------------------------------------------------------------
Function name: FxGetFlipStatus

Description:   Checks if the most recent flip has completed.

Return:        BOOL

               TRUE  - Flip in progress
			   FALSE - Flip completed
----------------------------------------------------------------------*/

BOOL
FXGETFLIPSTATUS(NT9XDEVICEDATA *ppdev)
{
  if (READSWAPCOUNT())
  {
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }

} // FxgetFlipStatus


/*----------------------------------------------------------------------
Function name: FxInVerticalBlank

Description:   Checks if the chip is in vertical blank.

Return:        BOOL

               TRUE  - Vertical Retrace Active
			   FALSE - Display Active
----------------------------------------------------------------------*/

BOOL
FXINVERTICALBLANK(NT9XDEVICEDATA *ppdev)
{
// Remove defined(WIN_CSIM) once the w9x csim toggles the bit for us.
// CSIM is for NT, WIN_CSIM is for w9x.
#if defined(CSIM) || defined(WIN_CSIM)
  if(_FF(fakeVerticalBlank))
  {
     _FF(fakeVerticalBlank)=0;
     return TRUE;
  }
  else
  {
     _FF(fakeVerticalBlank)=1;
     return FALSE;
  }
#else
  DWORD status;
  static DWORD toggle = 0;

  // The VSYNC status	does not change when in DPMS, or when TVOUT has disabled the monitor,
  // so let's avoid a hang by returning fake toggle conditions.
  if  (!(GET(ghwIO->dacMode) & (SST_DAC_DPMS_ON_VSYNC|SST_DAC_FORCE_VSYNC|SST_DAC_DPMS_ON_HSYNC|SST_DAC_FORCE_HSYNC)))
  {
     status = GET(ghwIO->status);
     if ((status & SST_VRETRACE) ^ (_FF(ddMiscFlags) & DDMF_VSYNC_POLARITY_MASK))
        return TRUE;
     else
        return FALSE;
  }
  else // Provide a fake toggling result
  {
     toggle = ~toggle; 
     if ( toggle ) 
        return TRUE;
     else
        return FALSE;
  }
#endif

} // FxInVerticalBlank


/*----------------------------------------------------------------------
Function name: FxGetScanLine

Description:   Returns vertical scanline position.

Return:        DWORD position - vertical scanline position
----------------------------------------------------------------------*/

#ifdef WINNT
DWORD
FXGETSCANLINE(NT9XDEVICEDATA *ppdev)
{
  LONG  position;

  if (FXINVERTICALBLANK(ppdev))
    return 0;

  // read the current scanline, only 11 bits are valid
  position = GET(ghwIO->vidCurrentLine) & 0x7FF;

  // if scanline doubling is enabled, divide current scanline by 2
  // aparently bit 4 in vidProcCfg indicates scanline doubling
  // The H3 doc says this bit is reserved, but the miniport sets
  // this bit if scanline doubling is enabled in the current mode
#define SST_HALF_MODE   BIT(4)
  if (SST_HALF_MODE & GET(ghwIO->vidProcCfg))
    position /= 2;

  // if the current scanline is past the end of the visible screen return 0
  if (position >= ppdev->cyScreen)
    return 0;
  else
    return position;
}
#else
DWORD
FXGETSCANLINE(NT9XDEVICEDATA *ppdev)
{
    DWORD position;

    position = GET(ghwIO->vidCurrentLine) & 0x07FFL;
    return position;

} // FxGetScanLine
#endif

#define BUSY_BIT        0x0004  // bit number to test for BUSY
/*----------------------------------------------------------------------
Function name:  Set_Busy

Description:   	Set the DIB engine busy bit.

                Keeps the software cursor from trying to use
                the command fifo at the same time as DirectDraw.
				Must be called before CMDFIFO_PROLOG

Return:         WORD *pFlags
----------------------------------------------------------------------*/

WORD  Set_Busy(WORD * pFlags)
{
      __asm mov   ebx, pFlags
      __asm bts   WORD PTR [ebx], BUSY_BIT

      return *pFlags;

} // SetBusy


/*******************************************************************/
/*                     PRIVATE FUNCTIONS                           */
/*******************************************************************/

#ifndef WINNT
#ifdef AGP_CMDFIFO

/*----------------------------------------------------------------------
Function name:  Promote_CmdFifoToAGP

Description:   	Switch to using the AGP command fifo.

Return:         NONE
----------------------------------------------------------------------*/

VOID Promote_CmdFifoToAGP(NT9XDEVICEDATA * ppdev)
{
  FxU32  startVirtualAddr;
  DWORD fifoLength;

#ifdef AGP_CMDFIFO
#pragma message("AGP Cmdfifo Enabled")
#endif

  // Multiple chip configurations cannot use AGP command FIFO.
  // if AA or SLI is enabled
#ifdef SLI_AA
  if ((_FF(dwNumUnits) > 1) && (_DD(ddAAModeEnabled) || _DD(ddSLIModeEnabled)))
    return;
#endif

  if (!_FF(enableAGPCF))
    _FF(doAgpCF) = 0;
  else
  {
    _FF(doAgpCF) = 1;

    if (CMDFIFOUNBUMPEDWORDS)
    {
      BUMPAGP(ppdev, CMDFIFOUNBUMPEDWORDS );

      while (GET(_FF(lpIOregs)->status) & SST_PCIFIFO_BUSY);

      while ((_FF(lpCRegs)->cmdFifo0.depth) > 0);
    }

    // can not be any commands left in video fifo
    FXBUSYWAIT(ppdev);

#ifndef WINNT
    // Do a config causes this is a good thing
    DoConfig(ppdev);
#endif

	  SETDW(_FF(lpCRegs)->agpReqSize, 0);
	  SETDW(_FF(lpCRegs)->hostAddrLow, 0);
	  SETDW(_FF(lpCRegs)->hostAddrHigh, 0);
	  SETDW(_FF(lpCRegs)->graphicsAddr, 0);
	  SETDW(_FF(lpCRegs)->graphicsStride, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo0.baseSize, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo0.baseAddrL, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo0.bump, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo0.readPtrL, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo0.readPtrH, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo0.aMin, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo0.aMax, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo0.depth, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo0.holeCount, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo1.baseSize, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo1.baseAddrL, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo1.bump, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo1.readPtrL, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo1.readPtrH, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo1.aMin, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo1.aMax, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo1.depth, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo1.holeCount, 0);
	  SETDW(_FF(lpCRegs)->cmdFifoThresh, 0);


    //
    // h/w bug workaround for Swap Hang
    //
      SETDW(_FF(lpCRegs)->cmdFifoThresh,      (15 << 5) | 8 );
	  SETDW(_FF(lpCRegs)->cmdFifo0.baseAddrL, (_FF(agpMain.physAddr) >> 12) );
	  SETDW(_FF(lpCRegs)->cmdFifo0.readPtrL,  _FF(agpMain.physAddr) );
	  SETDW(_FF(lpCRegs)->cmdFifo0.readPtrH,  0);
	  SETDW(_FF(lpCRegs)->cmdFifo0.aMin,      _FF(agpMain.physAddr) - 4);
	  SETDW(_FF(lpCRegs)->cmdFifo0.aMax,      _FF(agpMain.physAddr) - 4);
	  SETDW(_FF(lpCRegs)->cmdFifo0.depth,     0);
	  SETDW(_FF(lpCRegs)->cmdFifo0.holeCount, 0);
	  SETDW(_FF(lpCRegs)->cmdFifo0.baseSize,  SST_EN_CMDFIFO  |
                                            SST_CMDFIFO_AGP |
                                            SST_CMDFIFO_DISABLE_HOLES);

    startVirtualAddr     = _FF(agpMain.linAddr );
	  fifoLength           = _FF(agpMain.sizeInB );

    if (fifoLength > (4 * 1024L * 1024L))
	      fifoLength = 4 * 1024L * 1024L;

    _FF(cmdFifoBasePtr)  = (FxU32)&_FF(mainFifo.base);

	  CMDFIFOPTR           = startVirtualAddr ;
	  CMDFIFOSTART         = startVirtualAddr;
	  CMDFIFOSPACE         = (fifoLength / 4) - 3;
	  CMDFIFOEND           = startVirtualAddr + fifoLength - 12;
	  CMDFIFOOFFSET        = startVirtualAddr - _FF(agpMain.physAddr);
	  CMDFIFOJMP           = SSTCP_PKT0_JMP_AGP | (((_FF(agpMain.physAddr) & 0x00FFFFFF) >> 2) << 6);
	  CMDFIFOJMP2          = (_FF(agpMain.physAddr) >> 25) & ((1L << 12) - 1);

	  CMDFIFOEPILOGPTR     = CMDFIFOSTART;
	  CMDFIFOUNBUMPEDWORDS = 0;

	  _FF(InPacket)          = 0;
	  _FF(WordsLeftInPacket) = 0;
	  _FF(Wrapping)          = 0;
  }

} // Promote_CmdFifoToAGP

/*----------------------------------------------------------------------
Function name:  Demote_CmdFifoFromAGP

Description:    Switch to using the video command fifo.

Return:         NONE
----------------------------------------------------------------------*/

VOID Demote_CmdFifoFromAGP(NT9XDEVICEDATA * ppdev)
{
#ifdef CMDFIFO
  FxU32  startVirtualAddr;
  DWORD  physAddr;
#endif // #ifdef CMDFIFO

// STB Begin Changes
#ifdef STBPERF_2DAGPCMDFIFO
    if (_FF(doAgpCF) || _FF(enableAGPCF))
	  return;
#endif
// STB End Changes

  if (_FF(doAgpCF) && CMDFIFOUNBUMPEDWORDS)
  {
    BUMPAGP(ppdev, CMDFIFOUNBUMPEDWORDS );

    while (GET(_FF(lpIOregs)->status) & SST_PCIFIFO_BUSY);

    while (GET(_FF(lpCRegs)->cmdFifo0.depth) > 0);
  }

  if (_FF(doAgpCF))
  {
    DWORD   baseSize = (_FF(fifoSize) + 4095) & ~4095;

    FXBUSYWAIT(ppdev);

    _FF(mainFifo.base)  = (_FF(fifoStart) + 4095) & ~4095;
    _FF(mainFifo.start) = _FF(LFBBASE) + _FF(mainFifo.base);

    // disable command fifo 0
    //
    SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.baseSize,  0 );

#ifdef CMDFIFO
    //
    // h/w bug workaround for Swap hang
    //
    SETDW( _FF(lpCRegs)->cmdFifoThresh,             (15 << 5) | 8 );
    SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.readPtrL,  _FF(mainFifo.base) );
    SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.readPtrH,  0 );
    SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.aMin,      _FF(mainFifo.base) - 4 );
    SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.aMax,      _FF(mainFifo.base) - 4 );
    SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.depth,     0 );
    SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.holeCount, 0 );

    SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.baseAddrL, _FF(mainFifo.base) >> 12 );
    SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.baseSize,  ((baseSize >> 12) - 1) | SST_EN_CMDFIFO );

    startVirtualAddr    = _FF(mainFifo.start);
    physAddr            =  ((GET(_FF(lpCRegs)->PRIMARY_CMDFIFO.baseAddrL) & 0x3FF) << 12);

    _FF(cmdFifoBasePtr) = (FxU32)&_FF(mainFifo.base);

    CMDFIFOPTR          = startVirtualAddr;
    CMDFIFOSTART        = startVirtualAddr;
    CMDFIFOSPACE        = (_FF(fifoSize) / 4) - 2;
    CMDFIFOEND          = startVirtualAddr + _FF(fifoSize) - 8;
    CMDFIFOOFFSET       = startVirtualAddr - physAddr;
    CMDFIFOJMP          = SSTCP_PKT0_JMP_LOCAL | (( physAddr >> 2) << 6);

    _FF(InPacket)         = 0;
    _FF(WordsLeftInPacket)= 0;
    _FF(Wrapping)         = 0;
#endif
  }

  _FF(doAgpCF) = 0;

} // Demote_CmdFifoFromAGP

#endif // ifdef AGP_CMDFIFO

/*----------------------------------------------------------------------
Function name:  DoConfig

Description:    Call miniVDD to enable AGP.

Return:         int
----------------------------------------------------------------------*/

#ifdef H5
# define FILENAME "\\\\.\\H4VDD"
#else
#ifdef H4
# define FILENAME "\\\\.\\H4VDD"
#endif // H4
#endif // H5

int DoConfig(NT9XDEVICEDATA * ppdev)
{
   DIOC_DATA DIOC_Data;
   HANDLE hDevice;

   hDevice = CreateFile(FILENAME, 0, 0, NULL, 0, 0, NULL);
   if (INVALID_HANDLE_VALUE != hDevice)
      {
      DIOC_Data.dwDevNode = _FF(DevNode);
      DeviceIoControl(hDevice, AGP_WARMUP, &DIOC_Data, sizeof(DIOC_Data), NULL, 0x0, NULL, NULL);
      }
   CloseHandle(hDevice);

   return 0;

} // DoConfig

#endif // ifndef WINNT

#ifdef SLI_AA
#ifndef WINNT
//*----------------------------------------------------------------------
//Function name:  DoBitCopy
//
//Description:    Moves cursor from a Source to Destination via
//                SW (REP MOVS).
//Information:
//
//Return:         VOID
//----------------------------------------------------------------------*
void DoBitCopy(DWORD SrcAddr, DWORD DestAddr, DWORD dwSrcAdj, DWORD dwDestAdj, WORD wWidth, WORD wHeight)
{
        _asm pushad
        _asm cld                                     // Clear Flag
        _asm mov     esi, SrcAddr                    // ds:esi = Source
        _asm mov     edi, DestAddr                   // es:edi = DestStart
        _asm mov     eax, dwSrcAdj                   // Source Adjust
        _asm mov     edx, dwDestAdj                  // Dest Adjust
        _asm mov     bx, wHeight                     // Number of Times
        _asm shl     ebx, 16                         // Height in Upper word of ebx
        _asm xor     ecx, ecx                        // ecx=0
DoLoop:
        _asm mov     bx, wWidth                      // Get Width
        _asm shr     bx, 1                           // Number of Words
        _asm adc     cx, 0                           // Carry has # of bytes
        _asm rep     movsb                           // Move a byte
        _asm shr     bx, 1                           // Number of Dwords
        _asm mov     cx, bx                          // cx=#Dwords
        _asm rep     movsd                           // Move it
        _asm adc     cx, 0                           // Carry has #words
        _asm rep     movsw                           // Move a word
        _asm add     esi, eax                        // Start of Next Source Line
        _asm add     edi, edx                        // Start of Next Dest Line
        _asm xor     bx, bx                          // bx=0
        _asm sub     ebx, 10000h                     // Height=Height - 1
        _asm jnz     DoLoop
        _asm popad

        return;
}

//*----------------------------------------------------------------------
//Function name:  DoBitAndCopy
//
//Description:    Moves And Mask on Top of Image
//Information:
//
//Return:         VOID
//----------------------------------------------------------------------*
void DoBitAndCopy(DWORD SrcAddr, DWORD DestAddr, DWORD dwMaskAdj, WORD wWidth, WORD wHeight)
{
        _asm pushad
        _asm cld                                     // Clear Flag
        _asm mov     esi, SrcAddr                    // ds:esi = Source
        _asm mov     edi, DestAddr                   // es:edi = DestStart
        _asm mov     edx, dwMaskAdj                  // Mask Adjust
        _asm mov     bx, wHeight                     // Number of Times
        _asm shl     ebx, 16                         // Height in Upper word of ebx
        _asm xor     ecx, ecx                        // ecx=0
DoAndLoop:
        _asm mov     bx, wWidth                      // Get Width
        _asm shr     bx, 1                           // Number of Words
        _asm adc     cx, 0                           // Carry has # of bytes
        _asm or      cx, cx                          // Is cx == 0
        _asm jz      SkipAndByte
        _asm lodsb                                   // Move a byte
        _asm and     al, BYTE PTR [edi]              // And Data
        _asm stosb
SkipAndByte:
        _asm xor     cx, cx                          // cx=0
        _asm shr     bx, 1                           // Number of Dwords
        _asm adc     cx, 0                           // Carry has #words
        _asm or      cx, cx                          // Is cx == 0
        _asm jz      SkipAndWord
        _asm lodsw                                   // Move a byte
        _asm and     ax, WORD PTR [edi]              // And Data
        _asm stosw

SkipAndWord:
        _asm mov     cx, bx                          // cx=#Dwords
        _asm or      cx, cx                          // Is cx == 0
        _asm jz      SkipAndDword
DwordAnd:
        _asm lodsd                                   // Move a byte
        _asm and     eax, DWORD PTR [edi]            // And Data
        _asm stosd
        _asm loop    DwordAnd

SkipAndDword:
        _asm add     esi, edx                        // Start of Next Source Line
        _asm add     edi, edx                        // Start of Next Dest Line
        _asm xor     bx, bx                          // bx=0
        _asm sub     ebx, 10000h                     // Height=Height - 1
        _asm jnz     DoAndLoop
        _asm popad

        return;
}


//*----------------------------------------------------------------------
//Function name:  DoBitXorCopy
//
//Description:    Moves Xor Mask on Top of Image
//Information:
//
//Return:         VOID
//----------------------------------------------------------------------*
void DoBitXorCopy(DWORD SrcAddr, DWORD DestAddr, DWORD dwMaskAdj, WORD wWidth, WORD wHeight)
{
        _asm    pushad
        _asm    cld                                     // Clear Flag
        _asm    mov     esi, SrcAddr                    // ds:esi = Source
        _asm    mov     edi, DestAddr                   // es:edi = DestStart
        _asm    mov     edx, dwMaskAdj                  // Mask Adjust
        _asm    mov     bx, wHeight                     // Number of Times
        _asm    shl     ebx, 16                         // Height in Upper word of ebx
        _asm    xor     ecx, ecx                        // ecx=0
DoXorLoop:
        _asm    mov     bx, wWidth                      // Get Width
        _asm    shr     bx, 1                           // Number of Words
        _asm    adc     cx, 0                           // Carry has # of bytes
        _asm    or      cx, cx                          // Is cx == 0
        _asm    jz      SkipXorByte
        _asm    lodsb                                   // Move a byte
        _asm    xor     al, BYTE PTR [edi]              // And Data
        _asm    stosb
SkipXorByte:
        _asm    xor     cx, cx                          // cx=0
        _asm    shr     bx, 1                           // Number of Dwords
        _asm    adc     cx, 0                           // Carry has #words
        _asm    or      cx, cx                          // Is cx == 0
        _asm    jz      SkipXorWord
        _asm    lodsw                                   // Move a byte
        _asm    xor     ax, WORD PTR [edi]              // And Data
        _asm    stosw

SkipXorWord:
        _asm    mov     cx, bx                          // cx=#Dwords
        _asm    or      cx, cx                          // Is cx == 0
        _asm    jz      SkipXorDword
DwordXor:
        _asm    lodsd                                   // Move a byte
        _asm    xor     eax, DWORD PTR [edi]            // And Data
        _asm    stosd
        _asm    loop    DwordXor

SkipXorDword:
        _asm    add     esi, edx                        // Start of Next Source Line
        _asm    add     edi, edx                        // Start of Next Dest Line
        _asm    xor     bx, bx                          // bx=0
        _asm    sub     ebx, 10000h                     // Height=Height - 1
        _asm    jnz     DoXorLoop
        _asm    popad

        return;
}

/*----------------------------------------------------------------------
Function name:  HostRestoreCursor

Description:    Restore the area of the display where the cursor was
                drawn.

Information:

Return:         int     TRUE = Always returned
----------------------------------------------------------------------*/
int HostRestoreCursor(NT9XDEVICEDATA * ppdev, DWORD SaveCursorSurface, int LastCursorX, int LastCursorY, DWORD dwExclusionSave)
{
   DWORD dwDestLoc;
   DWORD dwSrcLoc;
   DWORD dwPitch;
   SHORT xSize;
   SHORT ySize;
   SHORT wBytesPerPixel;
   SHORT xLen;
   SHORT wMaskAdj;
   SHORT wScreenAdj;
   int X;
   int Y;

    MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);

    X = LastCursorX;
    Y = LastCursorY;

    xSize = SWCURSOR_WIDTH;
    if (X < 0)
      {
      xSize += X;
      X = 0;
      }

   if (X + SWCURSOR_WIDTH > _FF(hres))
      {
      xSize = _FF(hres) - X;
      }

    ySize = SWCURSOR_HEIGHT;
    if (Y < 0)
      {
      ySize += Y;
      Y = 0;
      }

   if (Y + SWCURSOR_HEIGHT > _FF(vres))
      {
      ySize = _FF(vres) - Y;
      }

    //Have we been clipped?
    if ((xSize > 0) && (ySize > 0))
      {
      wBytesPerPixel = (_FF(bpp) + 7) >> 3;;
      dwPitch = _FF(ddTilePitch);
      xLen = xSize * wBytesPerPixel;

      dwSrcLoc = dwExclusionSave;
      dwDestLoc = SaveCursorSurface + Y * dwPitch + X * (DWORD)wBytesPerPixel;

      wScreenAdj = (WORD)dwPitch - xLen;
      wMaskAdj = 128 - xLen;

      // Copy Image to Offscreen
      DoBitCopy(dwSrcLoc, dwDestLoc, wMaskAdj, wScreenAdj, xLen, ySize);
      }

    MODIFY_SLI_READ(ppdev, DISABLE_SLI_READ);

    return(TRUE);
}

int HostDrawCursor(NT9XDEVICEDATA * ppdev, DWORD swapToAddr)
{
   DWORD Offset;
   DWORD dwDestLoc;
   DWORD dwSrcLoc;
   DWORD dwMaskLoc;
   DWORD dwPitch;
   SHORT xSize;
   SHORT ySize;
   SHORT xCur;
   SHORT yCur;
   SHORT wBytesPerPixel;
   SHORT xLen;
   SHORT wMaskAdj;
   SHORT wScreenAdj;
   int X;
   int Y;
   int X1;
   int Y1;


   MODIFY_SLI_READ(ppdev, ENABLE_SLI_READ);

   X1 = X = _FF(CursorPosX) - _FF(HotspotX);
   Y1 = Y = _FF(CursorPosY) - _FF(HotspotY);

   // if X is less then zero then fix it up
   xSize = SWCURSOR_WIDTH;
   xCur = 0;
   if (X < 0)
      {
      xSize = xSize + X;
      xCur = (SHORT)SWCURSOR_WIDTH - xSize;
      X = 0;
      }

   if (X + SWCURSOR_WIDTH > _FF(hres))
         {
         xSize = _FF(hres) - X;
         }

    // Ditto with Y
    ySize = SWCURSOR_HEIGHT;
    yCur = 0;
    if (Y < 0)
       {
       ySize = ySize + Y;
       yCur = (SHORT)SWCURSOR_HEIGHT - ySize;
       Y = 0;
       }

    if (Y + SWCURSOR_HEIGHT > _FF(vres))
       {
       ySize = _FF(vres) - Y;
       }

    // if the cursor is clipped no need to draw it
    if ((xSize < 1) || (ySize < 1))
        return 0;

    wBytesPerPixel = (_FF(bpp) + 7) >> 3;
    dwPitch = _FF(ddTilePitch);

    xLen = xSize * wBytesPerPixel;

    // Offset = yCur * Pitch (128) + xCur * Bytes Per Pixels
    Offset = (yCur<<7) + xCur * wBytesPerPixel;
    dwSrcLoc = swapToAddr + Y * dwPitch + X * (DWORD)wBytesPerPixel;

    wScreenAdj = (WORD)dwPitch - xLen;
    wMaskAdj = 128 - xLen;

    // Again for Save Under Buffer
    _FF(CursorFlipCount) = _FF(FlipCount);
    _FF(CursorSurface) = _FF(CurrentSurface);
    dwDestLoc = _FF(HostcursorExclusionStart);
    DoBitCopy(dwSrcLoc, dwDestLoc, wScreenAdj, wMaskAdj, xLen, ySize);

    // Copy Image to Offscreen
    dwDestLoc = Offset + _FF(HostcursorSrcStart);
    DoBitCopy(dwSrcLoc, dwDestLoc, wScreenAdj, wMaskAdj, xLen, ySize);

    // Do And Mask
    dwMaskLoc = _FF(HostcursorAndStart) + Offset;
    DoBitAndCopy(dwMaskLoc, dwDestLoc, wMaskAdj, xLen, ySize);

    // Do Xor Mask
    dwMaskLoc = _FF(HostcursorXorStart) + Offset;
    DoBitXorCopy(dwMaskLoc, dwDestLoc, wMaskAdj, xLen, ySize);

    // Back to the screen
    DoBitCopy(dwDestLoc, dwSrcLoc, wMaskAdj, wScreenAdj, xLen, ySize);
    FXBUSYWAIT(ppdev);

   _FF(LastCursorPosX) = X1;
   _FF(LastCursorPosY) = Y1;

    MODIFY_SLI_READ(ppdev, DISABLE_SLI_READ);

    return 0;
}
#endif
#endif

#ifndef WINNT
#define DEBUG_FIX	// as nothing
#define MYCMDFIFO	_FF(lpCRegs)->PRIMARY_CMDFIFO
#include "..\minivdd\agpcf.c"
#endif

