/* -*-c++-*- */
/* $Header: entrleav.c, 2, 10/11/00 8:54:16 PM, Brent$ */
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
** File name:   entrleav.c
**
** Description: Prolog/Epilog routines for 32-bit GDI functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:16 PM$
**
** $History: entrleav.c $
** 
** *****************  Version 8  *****************
** User: Michael      Date: 1/05/99    Time: 7:51a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 7  *****************
** User: Ken          Date: 5/13/98    Time: 8:08a
** Updated in $/devel/h3/win95/dx/minivdd
** fast enter/leave turned on, working with SSB.
** also, bumped up the max # of device bitmap allocations by 4x
** 
** *****************  Version 6  *****************
** User: Artg         Date: 5/01/98    Time: 4:25p
** Updated in $/devel/h3/Win95/dx/minivdd
** sw cursor stuff
** 
** *****************  Version 5  *****************
** User: Ken          Date: 4/30/98    Time: 11:26a
** Updated in $/devel/h3/win95/dx/minivdd
** updated enter/leave to always check/set the busy bit on the 
** screen's pdevice, not the destination bitmap's pdevice ('cuz it might
** be a device bitmap.
** 
** *****************  Version 4  *****************
** User: Ken          Date: 4/27/98    Time: 5:35p
** Updated in $/devel/h3/win95/dx/minivdd
** complete (well, almost) enter/leave implementation for gdi32
** 
** *****************  Version 3  *****************
** User: Ken          Date: 4/24/98    Time: 1:33p
** Updated in $/devel/h3/win95/dx/minivdd
** first installment of FXENTER / FXLEAVE display driver drawing function
** work.  gdi32 only, no gdi16 yet.   works, but is slow, don't measure
** performance until all work is done, in about a week.
** also renamed fields in some blit parameter functions to be a bit more
** meaningful
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/23/98    Time: 5:44p
** Updated in $/devel/h3/win95/dx/minivdd
** working now, minus s/w cursor code
** 
** *****************  Version 1  *****************
** User: Ken          Date: 4/23/98    Time: 11:10a
** Created in $/devel/h3/win95/dx/minivdd
** common rendering function stuff
**
*/

#include "h3.h"
#include "thunk32.h"
#include "h3g.h"
#include "entrleav.h"

#ifdef FXENTER_EXTERNAL_FUNCTION


/*----------------------------------------------------------------------
Function name:  myFxEnter

Description:    Function prolog.

Information:

Return:         FxU32   srcFormat if processed or,
                        0xdeadbeef if punted back to dib engine.
----------------------------------------------------------------------*/
FxU32
myFxEnter(FxU16 deFlags,
	  FxU32 debugPuntExpr,
	  LPDIBENGINE lpDst,
	  LPDIBENGINE lpSrc,
	  FxU32 flags,
	  int rect1_valid,
	  int r1_left, int r1_top, int r1_right, int r1_bottom,
	  int rect2_valid,
	  int r2_left, int r2_top, int r2_right, int r2_bottom)
{
    FxU32 srcFormat, srcAddr, dstFormat, dstAddr;
    FxU32 srcIsDevBit, dstIsDevBit;
    FxU32 doSecondRect;
    FxU32 *pDeFlags;
    CMDFIFO_PROLOG(cmdFifo);
    
    DEBUG_FIX;

    // get a pointer to the screen's main pdevice
    //
    pDeFlags = (FxU32 *)&(((LPDIBENGINE)(lpDriverData->lpPDevice32))->deFlags);
    
#ifdef DEBUG
    // yes, that's a single "|", to remove the early exit check
    // the compiler must generate when "||" is used
    // 
    bPunt = UseDibEng | !(debugPuntExpr);
#endif // #ifdef DEBUG

    // check main pdev for busy & palette_xlat
    // 
    bPunt |= *pDeFlags & (BUSY | PALETTE_XLAT);

    // check destination bitmap for vram
    //
    bPunt |= ! (deFlags & VRAM);

    if (bPunt)
	goto Use_DibEng;
    
    //
    // test and set busy, punt if busy is alreay set, or if punt
    // condition is true
    //
    __asm mov eax, pDeFlags;
    __asm bts WORD PTR [eax], BUSY_BIT;
    __asm jc Use_DibEng;
    
    // we've already tested lpDst for VRAM above, no need to test it again,
    // now test for offscreen or not
    //
    dstIsDevBit = deFlags & OFFSCREEN;

    // the source is a device bitmap only if both VRAM and OFFSCREEN are set
    //
    srcIsDevBit = lpSrc &&
	( ! ((lpSrc->deFlags & (VRAM | OFFSCREEN)) ^ (VRAM | OFFSCREEN)));
    
    if (_FF(gdiFlags) != 0)
    {
	// if direct draw changed some 2d registers, or changed the desktop
	// from being linear to tiled or vice-versa,
	// we need to reset the invariant 2d regs and the stride of the
	// screen's pDevice so that dib engine rendering will be correct
	//
	if (_FF(gdiFlags) & SDATA_GDIFLAGS_2D_DIRTY)
	{
	    ResetInvariantState(RESET_DST | RESET_SRC | RESET_PDEV);
           _FF(gdiFlags) &= ~ (SDATA_GDIFLAGS_2D_DIRTY |
			       SDATA_GDIFLAGS_DST_WAS_DEVBIT |
			       SDATA_GDIFLAGS_SRC_WAS_DEVBIT);
	}

	// if the previous drawing operation was to a device bitmap,
	// the invariants were clobbered, so we need to reset them,
	// but there's no need to do this if we're not going to clobber
	// them again anyway, unless we need to save away the current
	// dstBaseAddr and dstFormat (e.g., StretchBltBufferToScreen),
	// so we'd need to put them to known states anyway
	// 
	if (_FF(gdiFlags & SDATA_GDIFLAGS_DST_WAS_DEVBIT))
	{
	    if (!dstIsDevBit ||	(flags & FXENTER_COMPUTE_DSTBA_FORMAT))
	    {
		ResetInvariantState(RESET_DST);
		_FF(gdiFlags) &= ~ SDATA_GDIFLAGS_DST_WAS_DEVBIT;
	    }
	}

	// if the previous drawing operation used a device bitmap as a
	// source, and the current operation doesn't use a device bitmap
	// as a source, reset the source 2d invariants
	//
	if (_FF(gdiFlags & SDATA_GDIFLAGS_SRC_WAS_DEVBIT) && !srcIsDevBit)
	{
	    ResetInvariantState(RESET_SRC);
	    _FF(gdiFlags) &= ~ SDATA_GDIFLAGS_SRC_WAS_DEVBIT;
	}
	
	// if a s/w cursor is active and unexcluded, exclude it if
	// it intersects with one of the given rectangles
	//
   	if (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR)
	{
       doSecondRect = 1;
       if (rect1_valid)
       {
          if ( ((r1_left <= CURSOR_LEFT) && (CURSOR_LEFT <= r1_right)) ||
               ((r1_left <= CURSOR_RIGHT) && (CURSOR_RIGHT <= r1_right)) ||
               ((r1_top <= CURSOR_TOP) && (CURSOR_TOP <= r1_bottom)) ||
               ((r1_top <= CURSOR_BOTTOM) && (CURSOR_BOTTOM <= r1_bottom)) 
             )
          {
              FxBeginAccess32(CURSOREXCLUDE);
              doSecondRect = 0;
          }
       }
       if(rect2_valid && doSecondRect)
       {
          if ( ((r2_left <= CURSOR_LEFT) && (CURSOR_LEFT <= r2_right)) ||
               ((r2_left <= CURSOR_RIGHT) && (CURSOR_RIGHT <= r2_right)) ||
               ((r2_top <= CURSOR_TOP) && (CURSOR_TOP <= r2_bottom)) ||
               ((r2_top <= CURSOR_BOTTOM) && (CURSOR_BOTTOM <= r2_bottom)) 
             )
          {
              FxBeginAccess32(CURSOREXCLUDE);
          }
       }
	}
  
    }

    //
    // finally, we are ready to party on the hardware
    // 
    
    CMDFIFO_SETUP(cmdFifo);

    if (srcIsDevBit)
    {
	// Set up invariant src 2d regs for device bitmap source
	//
	// We know the screen format is always stride packed,
	// and since we're not allowing device bitmaps to be a different pixel
	// type from the desktop, we know the pixel format's also the same.
	// The only thing we have to change is the stride
	// 
	srcFormat = _FF(screenFormat) & ~(SSTG_DST_LINEAR_STRIDE |
					  SSTG_DST_TILE_STRIDE);
	srcFormat |= lpSrc->deDeltaScan & SSTG_DST_LINEAR_STRIDE;
	srcAddr = lpSrc->deBitsOffset - lpDriverData->lfbBase;

	CMDFIFO_CHECKROOM(cmdFifo, 3);
	SETPH(cmdFifo, SSTCP_PKT2 | srcBaseAddrBit | srcFormatBit);
	SET(cmdFifo, _FF(lpGRegs)->srcBaseAddr, srcAddr);
	SET(cmdFifo, _FF(lpGRegs)->srcFormat, srcFormat);
	BUMP(3);

	_FF(gdiFlags) |= SDATA_GDIFLAGS_SRC_WAS_DEVBIT;
    }
    else if (flags & FXENTER_COMPUTE_SRCFORMAT)
    {
	srcFormat = _FF(screenFormat);
    }

    if (dstIsDevBit)
    {
	// Set up invariant dst 2d regs for device bitmap destination
	//
	dstAddr = lpDst->deBitsOffset - lpDriverData->lfbBase;
	dstFormat = _FF(screenFormat) & SSTG_DST_FORMAT;
	dstFormat |= SSTG_SRC_PACK_SRC;
	dstFormat |= lpDst->deDeltaScan & SSTG_SRC_LINEAR_STRIDE;
	
	CMDFIFO_CHECKROOM(cmdFifo, 5);
	SETPH(cmdFifo, (SSTCP_PKT2 | clip0minBit | clip0maxBit |
			dstBaseAddrBit | dstFormatBit));
	SET(cmdFifo, _FF(lpGRegs)->clip0min, 0);
	SET(cmdFifo, _FF(lpGRegs)->clip0max, R32(lpDst->deHeight,
						 lpDst->deWidth));
	SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr, dstAddr);
	SET(cmdFifo, _FF(lpGRegs)->dstFormat, dstFormat);
	BUMP(5);
	
	_FF(gdiFlags) |= SDATA_GDIFLAGS_DST_WAS_DEVBIT;

	if (flags & FXENTER_COMPUTE_DSTBA_FORMAT)
	{
	    CurrentDstAddr = dstAddr;
	    CurrentDstFormat = dstFormat;
	}
    }
    else
    {
	if (flags & FXENTER_COMPUTE_DSTBA_FORMAT)
	{
	    CurrentDstAddr = _FF(gdiDesktopStart);
	    CurrentDstFormat = _FF(screenFormat);
	}
    }

    CMDFIFO_EPILOG(cmdFifo);
    return srcFormat;

Use_DibEng:
    bPunt = TRUE;
    return 0xdeadbeef;
}

#endif // #ifdef FXENTER_EXTERNAL_FUNCTION
