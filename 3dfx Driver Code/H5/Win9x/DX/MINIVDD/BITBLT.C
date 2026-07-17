/* -*-c++-*- */
/*
** Copyright (c) 1997-1999, 3Dfx Interactive, Inc.
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
** File name:   bitblt.c
**
** Description: GDI BitBlt functions on the 32-bit side.
**
** $Revision: 5$
** $Date: 10/11/00 8:53:50 PM$
**
** $History: bitblt.c $
** 
** *****************  Version 64  *****************
** User: Andrew       Date: 7/16/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed regBase and RegBase from single dword to array to support
** sparse register mapping
** 
** *****************  Version 62  *****************
** User: Andrew       Date: 5/06/99    Time: 4:31p
** Updated in $/devel/h3/Win95/dx/minivdd
** Modified to work with Direct Write
** 
** *****************  Version 61  *****************
** User: Stb_sjohnston Date: 4/07/99    Time: 2:50p
** Updated in $/devel/h3/win95/dx/minivdd
** Cleaner fix for Slashdot/Wax bug (PRS 4457) from Chris Edgington
** 
** *****************  Version 60  *****************
** User: Stb_mmcclure Date: 3/10/99    Time: 2:50p
** Updated in $/devel/h3/win95/dx/minivdd
** For Bob Johnston: Fix for PRS 4976 - Dell Qual: Verdict, raster ops
** failures. We are now punting to the DIB engine when source = mono and
** Pattern style = BS_HATCHED. The accelerator cannot handle that case.
** Masked the Transparent blit bit returned from HandleBrush (). BitBlt
** does not use it.
** 
** *****************  Version 59  *****************
** User: Stb_sjohnston Date: 2/22/99    Time: 6:14p
** Updated in $/devel/h3/win95/dx/minivdd
** Fixes PRS #4457. Tweaked the size of the small blts when host to screen
** blt is followed by a screen to screen blt.
** 
** *****************  Version 58  *****************
** User: Edwin        Date: 2/03/99    Time: 4:34p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix PRS 4191,  need to make room for 2 dwords for 3D nopcmd.
** 
** *****************  Version 57  *****************
** User: Stb_pzheng   Date: 2/03/99    Time: 8:15a
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 56  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 8:02a
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 55  *****************
** User: Edwin        Date: 1/25/99    Time: 1:58p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix 3871, implemented a workaround for a HW bug in HWBitbltMemory().
** 
** *****************  Version 54  *****************
** User: Michael      Date: 1/04/99    Time: 1:19p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** 53    8/21/98 11:29a Michael
** Code didn't make it in during previous checkin.  Try once more.
** 
** 52    8/21/98 11:19a Michael
** Add MarkL's (igx) workaround for WHQL PC98 crash duning VFW AVI.  This
** is a work around (special cased punting to dib engine) that should be
** revisited in the future.  This work around shaves 0-1 Winmarks from
** WB98.   Fixes #1696
** 
** 51    6/18/98 7:02a Michael
** Rewrote HandleSpecialBrush().  Put missing CMDFIFO_SAVE/_RELOAD around
** call to HandleSpecialBrush().  Fixes 1901.
** 
** 50    5/15/98 4:30p Michael
** Backed out my previous changes.
** 
** 49    5/15/98 9:25a Michael
** Add lpDst to FXPUNT.
** 
** 48    5/14/98 10:17a Andrew
** Changed lpSrc to lpDst in FXENTER macro
** 
** 47    5/14/98 8:31a Ken
** added a PERFNOP case to gdi32.   change the "undef" to a "define"
** in fifomgr.h, and enable in softice (doperfnop = 1).  Replaces every
** write
** to the command register with a "0"
** 
** 46    5/13/98 5:35p Edwin
** Use FXPUNT macro instead of setting bPunt directly.
** 
** 44    5/09/98 5:50p Edwin
** Added support in HWBitbltMemory to handle cases where lpSrc->deType !=
** TYPE_DIBENG.
** 
** 43    4/27/98 5:35p Ken
** complete (well, almost) enter/leave implementation for gdi32
** 
** 42    4/24/98 9:43p Ken
** cleaned up some functions, added R32 macros, 
** 
** 41    4/24/98 6:58p Ken
** entire file now updated to enter/leave
** 
** 40    4/24/98 1:33p Ken
** first installment of FXENTER / FXLEAVE display driver drawing function
** work.  gdi32 only, no gdi16 yet.   works, but is slow, don't measure
** performance until all work is done, in about a week.
** also renamed fields in some blit parameter functions to be a bit more
** meaningful
** 
** 39    4/18/98 10:59a Michael
** More work for making gdi work in tiled mode.
** 
** 38    4/16/98 8:13p Michael
** Add code to handle drawing when in tiled mode.  Fix bug of menus
** incorrectly working in full-screen DD/D3D apps.
** 
** 37    4/15/98 6:41p Ken
** added unified header to all files, with revision, etc. info in it
** 
** 36    4/09/98 3:32p Andrew
** Removed bad source clip check and fixed bad src clip
** 
** 35    4/02/98 11:20p Artg
** added comments and usage sample for user level defined dpf
** 
** 34    3/31/98 11:38a Miriam
** remove print statements.
*/
#include "h3.h"
#include "thunk32.h"
 
#include "bitblt.h"
#include "h3g.h"

#include "tiled.h"

#include "entrleav.h"

#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif

extern DWORD RopConvertTable[17];

#ifdef PERFNOP
FxU32 doPerfNop = 0;
#endif // #ifdef PERFNOP

// variables to turn on and off blt info messages.
int PSBBDbg=0;
int HSBBDbg=1;
int SSBBDbg=0;

#define ROWSIZE 8
#define COLSIZE 8

// 3d nop command

#define SSTCP_PKT4_NOPCMD  ((0x48 << SSTCP_REGBASE_SHIFT))

#define CMDFIFO_BUILD_PK4( mask, regName, chip )\
    ( ((mask) << SSTCP_PKT4_MASK_SHIFT) | (chip << 11)\
    | (regName) | SSTCP_PKT4 )


// this macro is used to fix the lockup problems with web browsing,
// search for 3871 for description (see 4457 now)

#ifdef CMDFIFO
#define WAXBUG_3DNOPFIX \
  { \
    CMDFIFO_CHECKROOM(cf, 2); \
    SETPH( cf, CMDFIFO_BUILD_PK4(1, SSTCP_PKT4_NOPCMD, 0) ); \
    SET(cf, ghw3D->nopCMD, 0); \
    BUMP(2); \
    CMDFIFO_EPILOG(cf); \
  }
#else
#define WAXBUG_3DNOPFIX \
  { \
    SstRegs * ghw3D = (SstRegs*)(_FF(regBase[HWINFO_SST_3DREGS_INDEX]));\
    CMDFIFO_CHECKROOM(cf, 2); \
    SETPH( cf, CMDFIFO_BUILD_PK4(1, SSTCP_PKT4_NOPCMD, 0) ); \
    SET(cf, ghw3D->nopCMD, 0); \
    BUMP(2); \
    CMDFIFO_EPILOG(cf); \
  }
#endif

#define FIXW 8
#define FIXH 8
#define FIXOP ||


/*----------------------------------------------------------------------
Function name:  FxBitBlt

Description:    Main 32-bit entry point for GDI BitBlt function
                called from the 16-bit side.
Information:

Return:         BOOL    1 if no work (rejected) or,
                        result of ensuing call to BitBlt function.
----------------------------------------------------------------------*/
BOOL 
FxBitBlt(BitBltParams * lpParams)
//   DIBENGINE FAR  *Dst,
//   WORD          DstX,
//   WORD          DstY,
//   DIBENGINE FAR  *Src,
//   WORD          SrcX,
//   WORD          SrcY,
//   WORD          width,
//   WORD          height,
//   DWORD         Rop,
//   DIB_Brush8 FAR *lpPBrush,
//   DRAWMODE FAR   *lpDrawMode)
{
    LPDIBENGINE  lpDst;
    LPDIBENGINE  lpSrc;
    short newleft, newright, newbottom, newtop;
    LONG retval;

    DEBUG_FIX;

    FarToFlat(lpParams->lpDstDev, lpDst);

    if (!RopUsesSrc(lpParams->Rop >> 16))
    {
	goto SkipSrcClip;
    }

    // src clipping 
    newleft   = (short) lpParams->SrcX;
    newtop    = (short) lpParams->SrcY;
    newright  = (short) lpParams->SrcX + (short)lpParams->width;
    newbottom = (short) lpParams->SrcY + (short)lpParams->height;

    if ((newright <= 0) || (newbottom <= 0))
    {
	return 1;
    }
   
    // clip left   
    if (newleft < 0)
    {
	(short)lpParams->DstX -= newleft;
	(short)lpParams->width = newright;
	(short)lpParams->SrcX = 0;
    }
   
    //clip top 
    if (newtop < 0)
    {
	(short)lpParams->DstY -= newtop;
	(short)lpParams->height = newbottom;
	(short)lpParams->SrcY = 0;
    }

    FarToFlat(lpParams->lpSrcDev, lpSrc);

    // clip right
    if (newright > (short)lpSrc->deWidth)
    {
	newright -= (short)lpSrc->deWidth;
	(short)lpParams->width -= newright;
	if ((short)lpParams->width <= 0)
	{   
	    return 1;
	}
    }

    // clip bottom
    if (newbottom > (short)lpSrc->deHeight)   
    {
	newbottom -= (short)lpSrc->deHeight;
	(short)lpParams->height -= newbottom;
	if ((short)lpParams->height <= 0)
	{   
	    return 1;
	}
    }   
   
SkipSrcClip:

    if (lpSrc)
    {
	if (RopUsesSrc(lpParams->Rop >> 16))  
	{
        // PRS #1696 - check that colordepths match
        if ((lpSrc->deBitsPixel > 4) &&
            (lpSrc->deBitsPixel != lpDst->deBitsPixel)) 
        {
            bPunt = 1;
            return TRUE;
        }
	    if ((lpSrc == lpDst) || (lpSrc->deFlags & VRAM))
        {
            //DPF(DBG_DEBUG, 1, "BltS: w=%lx h=%lx\n", (DWORD)lpParams->width, (DWORD)lpParams->height); 
            retval = HWBitBltScreen(lpParams, lpDst, lpSrc);
        }
	    else   
        {
            //DPF(DBG_DEBUG, 1, "BltM: w=%lx h=%lx\n", (DWORD)lpParams->width, (DWORD)lpParams->height); 
            retval = HWBitBltMemory(lpParams, lpDst, lpSrc);
        }
	}
	else
    {
        //DPF(DBG_DEBUG, 1, "BltP: w=%lx h=%lx\n", (DWORD)lpParams->width, (DWORD)lpParams->height); 
	    retval = HWBitBltPattern(lpParams, lpDst, lpSrc);
    }
    }         
    else
    {
        //DPF(DBG_DEBUG, 1, "BltP: w=%lx h=%lx\n", (DWORD)lpParams->width, (DWORD)lpParams->height); 
        retval = HWBitBltPattern(lpParams, lpDst, lpSrc);
    }

    return retval;
}


/*----------------------------------------------------------------------
Function name:  HWBitBltScreen

Description:    Perform a BitBlt when the source and dest device
                are both the screen.

Information:    Handles following cases:
                    1) screen to screen with color patterns
                    2) screen to screen with not patterns
                    3) Caveats:
                        source is never mono since it's the screen.
                        dest is screen
                        pattern is always color?

Return:         LONG    TRUE is always returned.
----------------------------------------------------------------------*/
LONG 
HWBitBltScreen(BitBltParams * lp,
	       LPDIBENGINE lpDst,
	       LPDIBENGINE lpSrc)
   //   DIBENGINE FAR  *Dst,
   //   WORD          DstX,
   //   WORD          DstY,
   //   DIBENGINE FAR  *Src,
   //   WORD          SrcX,
   //   WORD          SrcY,
   //   WORD          width,
   //   WORD          height,
   //   DWORD         Rop,
   //   DIB_Brush8 FAR *lpPBrush,
   //   DRAWMODE FAR   *lpDrawMode
{
    DIB_Brush8 *lpPBrush;
    LPDRAWMODE  lpDrawMode;
    DWORD lastY, srcY, lastYMinus8;
    DWORD DirBit;
    DWORD dstXYData, srcXYData, cmdData;
    DWORD dstSizeData ;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    dstXYData = 0;
    srcXYData = 0;

    FarToFlat(lp->lpPBrush, lpPBrush);
    FarToFlat(lp->lpDrawMode, lpDrawMode);

    // PRS 4457 - web browser hang - hardware WAX bug 
    // we keep track of the end y for all operations in fxenter,
    // then we see if our src y for the screen to screen blt
    // is between savedY-8 and savedY; if so, inject a 3DNOP
    lastY = _FF(lastY); // save current value before FXENTER saves new value
    if (lastY <= 8)
        lastYMinus8 = 0;
    else
        lastYMinus8 = lastY - 8;

    // PRS 4457 - web browser hang - hardware WAX bug 
    // in order to know inside FXENTER which rect is the destination and
    // which is the source, rearranged the two rects so that the dest
    // is first (since there will always be a destination)
    FXENTER("HWBitBltScreen", lpDst->deFlags, (hwBlt && hwBitBltSS),
	    cf, lpDst, lpSrc, FXENTER_COMPUTE_SRCFORMAT,
	    FXENTER_RECT, lp->DstX, lp->DstY,
	                  (lp->DstX + lp->width), (lp->DstY + lp->height),
	    FXENTER_RECT, lp->SrcX, lp->SrcY,
	                  (lp->SrcX + lp->width), (lp->SrcY + lp->height)   );

    // BS_NULL=BS_HOLLOW=1
    // Note that NULL/HOLLOW brush cannot be trivially
    // rejected.  There still may need to be processing
    // contingent on the rop.

    // determine direction of blt.
    // need to to neg dir blt if 
    //   lp->DstY->lp->SrcY>0
    //   lp->DstY->lp->SrcY=0 and lp->DstX->lp->SrcX<0

    if (lpDst == lpSrc)
      {
      if (lp->DstY <= lp->SrcY)
         {
	      if ((lp->DstY == lp->SrcY) &&
	          (lp->DstX > lp->SrcX))
	         {
	         // setup for X neg dir
	         // srcXY, dstXY  upper right corner
	         // DPF(" DIR BIT = X DIR   UPPER RIGHT"); 
	         DirBit = SSTG_XDIR; 
	         srcXYData = R32(lp->SrcY, lp->SrcX + lp->width - 1);
	         dstXYData = R32(lp->DstY, lp->DstX + lp->width - 1);
	         }
	      else
	         {
	         // DPF(" DIR BIT = 0  UPPER LEFT");
	         // srcXY, dstXY  upper left corner
	         DirBit =  0L;
	         srcXYData = R32(lp->SrcY, lp->SrcX);
	         dstXYData = R32(lp->DstY, lp->DstX);
	         }
         }
      else 
         {
	      // lp->DstY->lp->SrcY>0
	      // srcXY, dstXY  lower left corner
	      // DPF(" DIR BIT = YDIR   LOWER  LEFT");
	      DirBit = SSTG_YDIR;
	      srcXYData = R32(lp->SrcY + lp->height - 1, lp->SrcX);
	      dstXYData = R32(lp->DstY + lp->height - 1, lp->DstX);
         }
      }
   else
      {
	   DirBit =  0L;
	   srcXYData = R32(lp->SrcY, lp->SrcX);
	   dstXYData = R32(lp->DstY, lp->DstX);
      }

    // <<< POSSIBLE ERROR:  Why doesn't this handle lower right corner?? >>>
      
    // PRS 4457 - web browser hang - hardware WAX bug 
    srcY = ((srcXYData >> 16) & 0xFFFF);
    if ( (srcY >= lastYMinus8) &&
         (srcY <= lastY) )
    {
        WAXBUG_3DNOPFIX;
    }

    dstSizeData = R32(lp->height, lp->width);

    // ternary rops. 
    // Src, Dst Pat.
    if (RopUsesPat(lp->Rop >> 16))
    {
	CMDFIFO_CHECKROOM(cf, 2);
	SETPH(cf, SSTCP_PKT2 | srcXYBit);
	SET(cf, _FF(lpGRegs)->srcXY, srcXYData);
	BUMP(2);

	// HandleBrush  calls prolog ...
	CMDFIFO_SAVE(cf);
	//STB-MDM For Bob Johnston, Part of the fix for PRS #4976 
	cmdData = (HandleBrush(lpPBrush, lp->Rop, lpDrawMode) & ~SSTG_TRANSPARENT); //Don't allow Transparent Blts
	CMDFIFO_RELOAD(cf);

	CMDFIFO_CHECKROOM(cf, 5);
	SETPH(cf, (SSTCP_PKT2 | srcFormatBit | dstSizeBit |
			dstXYBit | commandBit));
	SET(cf, _FF(lpGRegs)->srcFormat, _SrcFormat);
	SET(cf, _FF(lpGRegs)->dstSize, dstSizeData);
	SET(cf, _FF(lpGRegs)->dstXY, dstXYData);
	SETC(cf, _FF(lpGRegs)->command, cmdData | DirBit | SSTG_GO | SSTG_BLT);
	BUMP(5);
	CLEAR_COMMAND_EX(cf);
    }
    else    // setup  for ss without pattern.
    {
	lp->Rop = (lp->Rop >> 16) << 24;
	
	// since we don't call HandleBrush packet header and fifo stuff
	// needs to be called here
	CMDFIFO_CHECKROOM(cf, 6);
	SETPH(cf, (SSTCP_PKT2 | srcFormatBit | srcXYBit | dstSizeBit |
			dstXYBit | commandBit));
	SET(cf, _FF(lpGRegs)->srcFormat, _SrcFormat);
	SET(cf, _FF(lpGRegs)->srcXY, srcXYData);
	SET(cf, _FF(lpGRegs)->dstSize, dstSizeData );
	SET(cf, _FF(lpGRegs)->dstXY, dstXYData);
	SETC(cf, _FF(lpGRegs)->command, lp->Rop | DirBit | SSTG_GO | SSTG_BLT);
	BUMP(6);
    }

    if ((lp->width <= FIXW) FIXOP (lp->height <= FIXH ))
    {
        WAXBUG_3DNOPFIX;
    }

    FXLEAVE("HWBitBltScreen", cf, lpDst);
}


/*----------------------------------------------------------------------
Function name:  HWBitBltPattern

Description:    Perform a BitBlt when the source is a pattern and
                dest is the screen.

Information:    Handles following cases:
                    1) pat blt with color pattern
                    2) pat blt with mono pattern
                    3) pat blt without pattern (whiteness,
                       Blackeness, Dn)
                       
                Caveats:
                    1) no src
                    HWbitbltPattern has no src

Return:         LONG    TRUE is always returned.
----------------------------------------------------------------------*/
LONG 
HWBitBltPattern(BitBltParams * lp,
		LPDIBENGINE lpDst,
		LPDIBENGINE lpSrc)
    //   DIBENGINE FAR  *Dst,
    //   WORD          DstX,
    //   WORD          DstY,
    //   DIBENGINE FAR  *Src,
    //   WORD          SrcX,
    //   WORD          SrcY,
    //   WORD          width,
    //   WORD          height,
    //   DWORD         Rop,
    //   DIB_Brush8 FAR *lpPBrush,
    //   DRAWMODE FAR   *lpDrawMode
{
    DWORD  cmdData;
    DIB_Brush8 *lpPBrush;
    LPDRAWMODE  lpDrawMode;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;
         
    //DPF(DBG_BITBLT, 5 , " bitblt pattern %p\r\n",lpDst );
    FarToFlat(lp->lpDrawMode, lpDrawMode);
    FarToFlat(lp->lpPBrush, lpPBrush);

    // PRS #1696 - ensure lpdst colordepth == lpsrc
    if ((lpPBrush->dp8BrushBpp != 1) &&
        ((lpPBrush->dp8BrushBpp >> 3) != (lpDst->deBitsPixel >> 3)))
    {
        bPunt = 1;
        return TRUE;
    }

    // BS_NULL=BS_HOLLOW=1
    // Note that NULL/HOLLOW brush cannot be trivially rejected.
    // There still may need to be processing contingent on the rop.
    // Theoretically, we should never get a hollow brush with no
    // src and a pattern, but we do.  So much for theory...
    if ((lpPBrush->dp8BrushStyle == BS_HOLLOW) &&
	(RopUsesPat(lp->Rop >> 16)))
    {
	return 1;
    }

    FXENTER("HWBitBltPattern", lpDst->deFlags, (hwBlt && hwBitBltPS),
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
	    FXENTER_RECT, lp->DstX, lp->DstY,
	                  lp->DstX + lp->width,
                          lp->DstY + lp->height,
	    FXENTER_NO_RECT, 0, 0, 0, 0   );

#ifndef STB_16BIT_BITBLT
    switch(lp->Rop)
    {
      case(WHITENESS):
	  CMDFIFO_CHECKROOM(cf, 5);
	  SETPH(cf, (SSTCP_PKT2 | colorForeBit | dstSizeBit | dstXYBit |
			  commandBit));
	  SET(cf, _FF(lpGRegs)->colorFore, 0xFFFFFFFF);
	  SET(cf, _FF(lpGRegs)->dstSize, R32(lp->height, lp->width));
	  SET(cf, _FF(lpGRegs)->dstXY, R32(lp->DstY, lp->DstX));
	  SETC(cf, _FF(lpGRegs)->command, (SSTG_ROP_SRCCOPY |
					  SSTG_GO | SSTG_RECTFILL));
	  BUMP(5);
	  break; // whiteness

      case(BLACKNESS):
	  CMDFIFO_CHECKROOM(cf, 5);
	  SETPH(cf, (SSTCP_PKT2 | colorForeBit | dstSizeBit | dstXYBit |
		     commandBit));
	  SET(cf, _FF(lpGRegs)->colorFore, 0x00000000);
	  SET(cf, _FF(lpGRegs)->dstSize, R32(lp->height, lp->width));
	  SET(cf, _FF(lpGRegs)->dstXY, R32(lp->DstY, lp->DstX));
	  SETC(cf, _FF(lpGRegs)->command,  (SSTG_ROP_SRCCOPY |
					   SSTG_GO | SSTG_RECTFILL));
	  BUMP(5);
	  break; //blackness
   
#ifdef STBPERF_SOLIDCLRPATCPY
// PingZ.  Special case for PATCOPY with solid color brush
      case(PATCOPY):

   if((lpPBrush->dp8BrushStyle == BS_SOLID) && (lpPBrush->dp8BrushFlags & COLORSOLID))
   {
   DWORD * lpPattern = (DWORD *)lpPBrush->dp8BrushBits;

	  CMDFIFO_CHECKROOM(cf, 5);
	  SETPH(cf, (SSTCP_PKT2 | colorForeBit | dstSizeBit | dstXYBit |
		     commandBit));
	  SET(cf, _FF(lpGRegs)->colorFore, *lpPattern);
	  SET(cf, _FF(lpGRegs)->dstSize, R32(lp->height, lp->width));
	  SET(cf, _FF(lpGRegs)->dstXY, R32(lp->DstY, lp->DstX));
	  SETC(cf, _FF(lpGRegs)->command,  (SSTG_ROP_SRCCOPY |
					   SSTG_GO | SSTG_RECTFILL));
	  BUMP(5);
	  break; //PATCOPY with solid color brush)
   }

   // Intentional fall through 
#endif

	  // p d dpon dpna pn dn dpx  dpan dpa pdxn d dpno dpo  
      default:
	  //  HandleBrush calls CMDFIFO_CHECKROOM ...
	  CMDFIFO_SAVE(cf);
	  //STB-MDM For Bob Johnston, Part of the fix for PRS #4976 
	  cmdData = (HandleBrush(lpPBrush, lp->Rop, lpDrawMode) & ~SSTG_TRANSPARENT); //Don't allow Transparent Blts
	  CMDFIFO_RELOAD(cf);

	  CMDFIFO_CHECKROOM(cf, 4);
	  SETPH(cf, SSTCP_PKT2 | dstSizeBit | dstXYBit | commandBit);
	  SET(cf, _FF(lpGRegs)->dstSize, R32(lp->height, lp->width));
	  SET(cf, _FF(lpGRegs)->dstXY, R32(lp->DstY, lp->DstX));
	  SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_GO | SSTG_RECTFILL);
	  BUMP(4);

	  CLEAR_COMMAND_EX(cf);
	  break; // default
    }
#else //STB_16BIT_BITBLT
	  //  HandleBrush calls CMDFIFO_CHECKROOM ...
	  CMDFIFO_SAVE(cf);
	  //STB-MDM For Bob Johnston, Part of the fix for PRS #4976 
	  cmdData = (HandleBrush(lpPBrush, lp->Rop, lpDrawMode) & ~SSTG_TRANSPARENT); //Don't allow Transparent Blts
	  CMDFIFO_RELOAD(cf);

	  CMDFIFO_CHECKROOM(cf, 4);
	  SETPH(cf, SSTCP_PKT2 | dstSizeBit | dstXYBit | commandBit);
	  SET(cf, _FF(lpGRegs)->dstSize, R32(lp->height, lp->width));
	  SET(cf, _FF(lpGRegs)->dstXY, R32(lp->DstY, lp->DstX));
	  SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_GO | SSTG_RECTFILL);
	  BUMP(4);

	  CLEAR_COMMAND_EX(cf);
#endif //STB_16BIT_BITBLT

    FXLEAVE("HWBitBltPattern", cf, lpDst);
}


/*----------------------------------------------------------------------
Function name:  HWBitBltMemory

Description:    Perform a BitBlt when the source is host memory
                and dest is the screen.

Information:    Assumptions:
                  - No bpp conversion between src and dest.
                  - The src is always in dst format except when
                    src is mono.
                  - Host to screen will require as many as 3
                    packet headers.
                  - color pattern
                  - srcXY/dstXY/dstSize/Command ...
                  - Src Data.
                HwBitBltMemory should be able to use
                HandleBrush code.

Return:         LONG    TRUE is always returned.
----------------------------------------------------------------------*/
LONG 
HWBitBltMemory(BitBltParams * lp,
	       LPDIBENGINE lpDst,
	       LPDIBENGINE lpSrc)
    //   DIBENGINE FAR  *Dst,
    //   WORD          DstX,
    //   WORD          DstY,
    //   DIBENGINE FAR  *Src,
    //   WORD          SrcX,
    //   WORD          SrcY,
    //   WORD          width,
    //   WORD          height,
    //   DWORD         Rop,
    //   DIB_Brush8 FAR *lpPBrush,
    //   DRAWMODE FAR   *lpDrawMode

{
    DWORD dwtemp, dwordsToBlt;
    DWORD cmdData;
    DWORD dwordsInScanline, dwordsInScanlineG, rightByteAddr;
    DWORD startOffset;
    BITMAP * lpbmp;
    short index;
    DIB_Brush8 *lpPBrush;
    LPDRAWMODE  lpDrawMode;

    DWORD SrcPitch;     // set to deDeltaScan or bmWidthBytes
    WORD  BitsPixel;    // set to lpSrc->deBitsPixel or lpSrc->bmBitsPixel
    WRITECOLORSRCFUNC fpWriteColorSrc;  // function pointer to WriteColorSrc

    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    //STB-MDM For Bob Johnston, Part of the fix for PRS #4976 
	// Moved from below for else statement casing out HATCHED and punting the brush style.
	FarToFlat(lp->lpPBrush, lpPBrush);
    FarToFlat(lp->lpDrawMode, lpDrawMode);

    /*  
    ** Src can point to a DIBENGINE struct or PBITMAP struct.
    ** First word pointed by Src determines the type of struct.
    ** Src points to DIBENGINE structure if first word is 
    ** TYPE_DIBENG (0x5250) and PBITMAP if first word is 0.
    */
    if (TYPE_DIBENG == lpSrc->deType)
    {
        SrcPitch = lpSrc->deDeltaScan;
        BitsPixel = (WORD)lpSrc->deBitsPixel;
        fpWriteColorSrc = (WRITECOLORSRCFUNC) WriteColorSrc;
    }
    else
    {
        lpbmp = (LPBITMAP) lpSrc;
        if (lpbmp->bmSegmentIndex)  // non-zero indicates bitmap > 64K
            FXPUNT(FXPUNT_NORMAL);  // punt to dib engine for now


		// Bob Johnston 03/09/1999 PRS # 4976
		// Cannot acclerate MONO source and HATCHED brush.  The reason
		// is that the forground colors and background colors are in different 
		// structures lpPBrush vs lpDrawMode
		// This is evident in Verdict GDI->BitBltMonochrome->Hatched Brush tests!
		// The sensible thing to do is to punt back to the DIBEngine!
	    if(lpPBrush->dp8BrushStyle == BS_HATCHED)
            FXPUNT(FXPUNT_NORMAL);  // punt to dib engine for now

        SrcPitch = lpbmp->bmWidthBytes;
        BitsPixel = (WORD)lpbmp->bmBitsPixel;
        fpWriteColorSrc = (WRITECOLORSRCFUNC) WriteBMPColorSrc;
    }

//  STB-MDM For Bob Johnston, Part of the fix for PRS #4976
//  Moved above to get pointers when needed
//	FarToFlat(lp->lpPBrush, lpPBrush);
//  FarToFlat(lp->lpDrawMode, lpDrawMode);

    // Note that NULL/HOLLOW brush cannot be trivially
    // rejected.  There still may need to be processing
    // contingent on the rop.

    /*  
	Assume that the brush/src/dest color pixel is independent.
	Brush can be mono or color. Src can be mono or color. Dest must
	be color.  The hardware expands mono args before rops.
   
	H3 can't not do mono src, mono pattern.  In this case, expand 
	mono pattern into color.
      
 
	*/
   
#define ColPat_ColSrc   0
#define ColPat_MonoSrc   1
#define MonoPat_ColSrc   4   
#define MonoPat_MonoSrc   5
   
   
//hwm

    // index can have the following values:
    // 000b color pattern, color src
    // 001b color pattern, mono src
    // 100b mono pattern, color src
    // 101b mono pattern, mono src

#ifdef DEBUG
    if (lpPBrush->dp8BrushBpp == 20) 
    {
	//__asm int 3;
    }
#endif

/*
    DPF(" DstX %x, %d: DstY, %x, %d\r\n", lp->DstX, lp->DstX,
	lp->DstY, lp->DstY); 
    DPF(" SrcX %x, %d: SrcY, %x, %d\r\n", lp->SrcX, lp->SrcX,
	lp->SrcY, lp->SrcY); 
    DPF(" width %x, %d: height, %x, %d\r\n", lp->width, lp->width,
	lp->height, lp->height); 
*/

    FXENTER("HWBitBltMemory", lpDst->deFlags, (hwBlt && hwBitBltHS),
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
	    FXENTER_RECT, lp->DstX, lp->DstY,
	                  lp->DstX + lp->width,
                      lp->DstY + lp->height,
	    FXENTER_NO_RECT, 0, 0, 0, 0   );
    
    index = ( (lpPBrush->dp8BrushFlags & PATTERNMONO) | (BitsPixel & 1) );

    WAXBUG_3DNOPFIX;

    if (BitsPixel == 1) 
    {
	// Mono case will be byte packed.
	// No stride info needed
	// startOffset is byte ptr to first pix of blt

	startOffset = ( (lp->SrcY * SrcPitch) + (lp->SrcX >> 3) );

	dwtemp =  (((lp->width * BitsPixel) - 1) >> 3) ;

	dwordsInScanlineG =
	    ((lp->width + (lp->SrcX & 0x1F) - 1) >> 5) + 1;
         
	dwordsToBlt = dwordsInScanlineG * lp->height;
	//INT_3
	if (RopUsesPat(lp->Rop >> 16))
	{
	    switch(index)
	    {
	      case ColPat_MonoSrc:
		  //DPF("case ColPat_MonoSrc:\r\n");
		  //INT_3
		  // color pattern reg is set in 
		  // packet header in HandleBrush

		  //  Hatched Brush is essentially a mono 
		  //  brush masquarading as a color brush.
		  if (lpPBrush->dp8BrushStyle == BS_HATCHED)
		  { 
		     // Bob Johnston  03/09/1999
			 // Bogus case, we cannot accelerate
			 // We should never get here because of the 
			 // Punt above.
				
				//INT_3

		  }
		  else
		  {
		      // since its color brush no pattern alias reg
		      //   writes or color fore/back
		      CMDFIFO_SAVE(cf);
			  //STB-MDM For Bob Johnston, Part of the fix for PRS #4976 
	          cmdData = (HandleBrush(lpPBrush, lp->Rop, lpDrawMode) & ~SSTG_TRANSPARENT); //Don't allow Transparent Blts
		      CMDFIFO_RELOAD(cf);
   
		  CMDFIFO_CHECKROOM(cf, 8);
		  SETPH(cf, (SSTCP_PKT2 | srcFormatBit | srcXYBit |
			     colorBackBit | colorForeBit | dstSizeBit |
			     dstXYBit | commandBit)); 

		  SET(cf, _FF(lpGRegs)->srcFormat, HandleFormat(lpSrc));
		  SET(cf, _FF(lpGRegs)->srcXY, ((startOffset << 3) |
						(lp->SrcX & 7)) );
    
    		  //STB-MDM For Bob Johnston, Part of the fix for PRS #4976
			  // Punting Hatched case
//			  if (lpPBrush->dp8BrushStyle == BS_HATCHED)
//			  { 
//			      SET(cf, _FF(lpGRegs)->colorBack, lpDrawMode->bkColor);
//			      SET(cf, _FF(lpGRegs)->colorFore, lpDrawMode->TextColor);
//			  }
//			  else   
//			  {
			      SET(cf, _FF(lpGRegs)->colorBack, lpDrawMode->TextColor);
			      SET(cf, _FF(lpGRegs)->colorFore, lpDrawMode->bkColor);
//			  }
		      SET(cf, _FF(lpGRegs)->dstSize, R32(lp->height, lp->width));
		      SET(cf, _FF(lpGRegs)->dstXY, R32(lp->DstY, lp->DstX));
		      SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_HOST_BLT);
		      BUMP(8);
  		      startOffset &= 0xFFFFFFFC;
		      CMDFIFO_SAVE(cf);
		      SendMonoBltData(lp->SrcX, lp->SrcY, lp->width, lp->height,
			     	  (LPBITMAP) lpSrc);

		  }
   
		  CMDFIFO_RELOAD(cf);
		  CLEAR_COMMAND_EX(cf);
		  break;

	      case MonoPat_MonoSrc:
		  //DPF("case MonoPat_MonoSrc::\r\n");
		  //INT_3
		  //goto hs_Use_Dib_Only;
   
		  //  have to manually expand mono brush. hw
		  //  can't handle mono brush, mono src.
   
          CMDFIFO_SAVE(cf);
		  cmdData = HandleSpecialBrush (lpPBrush, 
                                        lp->Rop, 
                                        lpDrawMode);
          CMDFIFO_RELOAD(cf);
		  CMDFIFO_CHECKROOM(cf, 8);
		  SETPH(cf, (SSTCP_PKT2 | srcFormatBit | srcXYBit |
			     colorBackBit | colorForeBit | dstSizeBit |
			     dstXYBit | commandBit) );
		  SET(cf, _FF(lpGRegs)->srcFormat, HandleFormat(lpSrc));
		  SET(cf, _FF(lpGRegs)->srcXY, ((startOffset << 3) |
						     (lp->SrcX & 7)));
		  SET(cf, _FF(lpGRegs)->colorBack, lpDrawMode->TextColor);
		  SET(cf, _FF(lpGRegs)->colorFore, lpDrawMode->bkColor);
		  SET(cf, _FF(lpGRegs)->dstSize, R32(lp->height, lp->width));
		  SET(cf, _FF(lpGRegs)->dstXY, R32(lp->DstY, lp->DstX));
		  SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_HOST_BLT);
		  BUMP(8);

		  startOffset &= 0xFFFFFFFC;
		  CMDFIFO_SAVE(cf);
		  SendMonoBltData(lp->SrcX, lp->SrcY, lp->width, lp->height,
				  (LPBITMAP) lpSrc);
		  CMDFIFO_RELOAD(cf);
   		  break;
   	    }   //switch(index)
	}
	else
	{
	    lp->Rop = (lp->Rop >> 16) << 24;
	    
	    // only mono src, no pat.
	    //DPF( " only mono src, no pat.\r\n");
	    //INT_3
	    CMDFIFO_CHECKROOM(cf, 8);
	    SETPH(cf, (SSTCP_PKT2 | srcFormatBit | srcXYBit |
		       colorBackBit | colorForeBit | dstSizeBit |
		       dstXYBit | commandBit));
	    //DPF("Mono Src Only");
	    // HandleFormat  fill a dword for srcFormat register,
	    // stride, pixel depth, packing.
	    SET(cf, _FF(lpGRegs)->srcFormat, HandleFormat(lpSrc));
	    SET(cf, _FF(lpGRegs)->srcXY, ((startOffset << 3) |
					   (lp->SrcX & 7)) );
	    SET(cf, _FF(lpGRegs)->colorBack, lpDrawMode->TextColor);
	    SET(cf, _FF(lpGRegs)->colorFore, lpDrawMode->bkColor);
	    SET(cf, _FF(lpGRegs)->dstSize, R32(lp->height, lp->width));
	    SET(cf, _FF(lpGRegs)->dstXY, R32(lp->DstY, lp->DstX));
	    SETC(cf, _FF(lpGRegs)->command, lp->Rop | SSTG_HOST_BLT);
	    BUMP(8);

	    startOffset &= 0xFFFFFFFC;
	    CMDFIFO_SAVE(cf);
	    SendMonoBltData(lp->SrcX, lp->SrcY, lp->width, lp->height,
			    (LPBITMAP) lpSrc);
	    CMDFIFO_RELOAD(cf);

        /*
         * fix PRS 3871, see comment in color src path 
         * (ChrisE - modified from EdW original fix to check for a little
         *  large size of blt as a condition of injecting the 3DNOP - 
         *  fixes more of the web browsing lockup problems)
         * **** NOTE THIS HAS BEEN MOVED TO THE FIX FOR 4457
         */
	}
    }
    else    
	//   // color src
    {
	//INT_3
	// startOffset is byte ptr to first pix of blt

	startOffset = ( (lp->SrcY * SrcPitch) + (lp->SrcX * (BitsPixel >> 3)) );
   
	rightByteAddr = startOffset + (lp->width * (BitsPixel >> 3));

	dwordsInScanline =    ( ((rightByteAddr + 3) & 0xFFFFFFFC) -
				(startOffset & 0xFFFFFFFC) ) >> 2; 

//gregs calc for dwords in scanline.
//      dwordsInScanlineG = (((((lp->width * lpSrc->deBitsPixel) - 1) 
//                       >> 3) + (lp->SrcX & 3)) >> 2) + 1;
	// calc dwords in rect to be blted

	// stride in bytes of bitmap in bytes
	// stride will tell the hw how many bytes to skip at end.
	if (RopUsesPat(lp->Rop >> 16))
	{
	    switch(index)
	    {
	      case ColPat_ColSrc:
		  //DPF(" case ColPat_ColSrc:\r\n");
		  //INT_3

		  CMDFIFO_SAVE(cf);
		  //STB-MDM For Bob Johnston, Part of the fix for PRS #4976 
	      cmdData = (HandleBrush(lpPBrush, lp->Rop, lpDrawMode) & ~SSTG_TRANSPARENT); //Don't allow Transparent Blts
		  CMDFIFO_RELOAD(cf);

		  CMDFIFO_CHECKROOM(cf, 8);
		  SETPH(cf, (SSTCP_PKT2 | srcFormatBit | srcXYBit |
		  //STB-MDM For Bob Johnston, Part of the fix for PRS #4976 
		  //	  colorBackBit | colorForeBit | dstSizeBit |
			      dstSizeBit | dstXYBit | commandBit)); 
		  SET(cf, _FF(lpGRegs)->srcFormat, HandleFormat(lpSrc));
		  SET(cf, _FF(lpGRegs)->srcXY, startOffset & 3);

		  //STB-MDM For Bob Johnston, Part of the fix for PRS #4976 
//        SET(cf, _FF(lpGRegs)->colorBack, lpDrawMode->TextColor);
//		  SET(cf, _FF(lpGRegs)->colorFore, lpDrawMode->bkColor);

		  SET(cf, _FF(lpGRegs)->dstSize, R32(lp->height, lp->width));
		  SET(cf, _FF(lpGRegs)->dstXY, R32(lp->DstY, lp->DstX));
		  SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_HOST_BLT);
		  BUMP(8);

		  startOffset &= 0xFFFFFFFC;
		  CMDFIFO_SAVE(cf);
		  fpWriteColorSrc(lpSrc, startOffset, lp->height, 
				dwordsInScanline);
		  CMDFIFO_RELOAD(cf);
		  CLEAR_COMMAND_EX(cf);
   
		  break;
            
	      case MonoPat_ColSrc:
		  //DPF("case MonoPat_ColSrc:\r\n");
		  //INT_3

		  CMDFIFO_CHECKROOM(cf, 3);
		  SETPH(cf, SSTCP_PKT2 | srcFormatBit | srcXYBit);
		  SET(cf, _FF(lpGRegs)->srcFormat, HandleFormat(lpSrc));
		  SET(cf, _FF(lpGRegs)->srcXY, startOffset & 3);
		  BUMP(3);

		  // Handle brush  writes patternalias0, patternalias1,
		  // colorfore, colorback   
		  CMDFIFO_SAVE(cf);
		  //STB-MDM For Bob Johnston, Part of the fix for PRS #4976 
	      cmdData = (HandleBrush(lpPBrush, lp->Rop, lpDrawMode) & ~SSTG_TRANSPARENT); //Don't allow Transparent Blts
		  CMDFIFO_RELOAD(cf);

		  CMDFIFO_CHECKROOM(cf, 4);
		  SETPH(cf, (SSTCP_PKT2 | dstSizeBit | dstXYBit |
			      commandBit)); 
		  SET(cf, _FF(lpGRegs)->dstSize, R32(lp->height, lp->width));
		  SET(cf, _FF(lpGRegs)->dstXY, R32(lp->DstY, lp->DstX));
		  SETC(cf, _FF(lpGRegs)->command, cmdData | SSTG_HOST_BLT);
		  BUMP(4);

		  startOffset &= 0xFFFFFFFC;
		  CMDFIFO_SAVE(cf);
		  fpWriteColorSrc(lpSrc, startOffset, lp->height, 
				dwordsInScanline);
		  CMDFIFO_RELOAD(cf);
		  CLEAR_COMMAND_EX(cf);
		  break;
   
	      default: 
		  __asm int 3;
	    }   //switch(index)
	}
	else
	{
	    lp->Rop = (lp->Rop >> 16) << 24;
	    
	    // only color src case: no pat.
	    //DPF(" > > > > > > > > > >");
	    //DPF("Color Src no Pat\r\n");
	    //DPF(" > > > > > > > > > >");
	    //INT_3
	    CMDFIFO_CHECKROOM(cf, 6);
	    SETPH(cf, (SSTCP_PKT2  | srcFormatBit | srcXYBit |
			    dstSizeBit | dstXYBit | commandBit)); 
	    // HandleFormat  fill a dword for srcFormat register,
	    // stride, pixel depth.
	    SET(cf, _FF(lpGRegs)->srcFormat, HandleFormat(lpSrc));
	    SET(cf, _FF(lpGRegs)->srcXY, startOffset & 3);
	    SET(cf, _FF(lpGRegs)->dstSize, R32(lp->height, lp->width));
	    SET(cf, _FF(lpGRegs)->dstXY, R32(lp->DstY, lp->DstX));
	    SETC(cf, _FF(lpGRegs)->command, lp->Rop | SSTG_HOST_BLT);
	    BUMP(6);

	    startOffset &= 0xFFFFFFFC;
	    CMDFIFO_SAVE(cf);
	    fpWriteColorSrc(lpSrc, startOffset, lp->height, 
			  dwordsInScanline);
	    CMDFIFO_RELOAD(cf);

        /*
         * Fix PRS 3871: http://ocp.agn3d.com/Stuff/ocp_hot_faq.htm hangs in
         * Netscape browser v4.5. Workaround a bug in WAX pipeline for very
         * small host to screen blt, which follows by screen to screen blts.
         * The source of the second screen to screen blt is the destination
         * of the first blt. Thanks to John W.'s solution for the NT driver.
         *
         * (ChrisE - modified from EdW original fix to check for a little
         *  large size of blt as a condition of injecting the 3DNOP - 
         *  fixes more of the web browsing lockup problems)
         * **** NOTE THIS HAS BEEN MOVED TO THE FIX FOR 4457
         */
	}
    }

    WAXBUG_3DNOPFIX;
    FXLEAVE("HWBitBltMemory", cf, lpDst);
}


/*----------------------------------------------------------------------
Function name:  HandleFormat

Description:    Process information to determine the correct DWORD 
                for the srcFormat register.

Information:    - Hardware ignores stride if the src packing bits
                  are set.
                - Color Bitmaps are word aligned Dword packed.

Return:         DWORD   Value for the srcFormat register.
----------------------------------------------------------------------*/
DWORD 
HandleFormat(LPDIBENGINE lpDev)
{
    BITMAP * lpbitmap;
    DWORD retval;

    DEBUG_FIX;

    if (TYPE_DIBENG == lpDev->deType)
    {
        // lpDev points to DIBENGINE structure

        switch (lpDev->deBitsPixel)
        {
        case 8:
            retval = (lpDev->deDeltaScan ) | SSTG_PIXFMT_8BPP;
            break;

        case 16:
            retval = (lpDev->deDeltaScan ) | SSTG_PIXFMT_16BPP;
            break;

        case 24:
            retval = (lpDev->deDeltaScan ) | SSTG_PIXFMT_24BPP ;
            break;

        case 32:
            retval = (lpDev->deDeltaScan) | SSTG_PIXFMT_32BPP; 
            break;
#ifdef DEBUG
        case 1:
            INT_3       // should never get here
            break;
#endif
        default:
            retval = 0;
            break;
        }
    }
    else
    {
        // lpDev points to PBITMAP structure
       
        lpbitmap = (LPBITMAP) lpDev;

        switch (lpbitmap->bmBitsPixel)
        {
        case 1:
            retval = (lpbitmap->bmWidthBytes) | SSTG_PIXFMT_1BPP;
            break;

        case 8:
            retval = (lpbitmap->bmWidthBytes) | SSTG_PIXFMT_8BPP;
            break;

        case 16:
            retval = (lpbitmap->bmWidthBytes) | SSTG_PIXFMT_16BPP;
            break;

        case 24:
            retval = (lpbitmap->bmWidthBytes) | SSTG_PIXFMT_24BPP ;
            break;

        case 32:
            retval = (lpbitmap->bmWidthBytes) | SSTG_PIXFMT_32BPP; 
            break;

        default:
            retval = 0;
            break;
       }
    }
    
    return retval;
}

// ************************************************************************ 
// *********************************************************************** 
// for all blt routines.  Listed below are opimization
/*   1) if convert all rops with no src and solid brush as rectfill 
      color register1 as src.  i.e. conver PDa to SDa with S being 
      color regsister 1

   2) convert whiteness and blackness as rect fill with src copy. 


   assumptions:
      1) only have handle mono stuff with hatched and pattern brush.
         maybe dibpattern.

   Return values
   retval > 0:  bits to set in command register
   retval < 0   Error.
   if no blt is required ie hollow brush, it must be handled before   
   calling HandleBrush.


   Probably 
*/


/*----------------------------------------------------------------------
Function name:  HandleSpecialBrush

Description:    HandleBrush special cases:  MonoSrc, MonoPat.

Information:    
   This routine doesn't buy us much.  It does *exactly* what
   HandleBrush does for pattern and hatch with the exception
   that it doesn't set the color registers.  The color registers
   are set by the caller.  We could consider getting rid of this
   routine as it does not appear to be time critical (not used
   in speedy/winbench).  (michael@3dfx 061898)

Return:         DWORD   Valid dword to program the command register.
----------------------------------------------------------------------*/
DWORD 
HandleSpecialBrush(
    DIB_Brush8 *lpBrush, 
    DWORD Rop,
    LPDRAWMODE lpDrawMode)
{
    DWORD dwtemp, dwpatcount, i;
    int patcount;
    BYTE mono0[4];
    BYTE mono1[4];
    DWORD dwXprntFlg;
    DWORD * lpPattern;

    CMDFIFO_PROLOG(cf);
   
    DEBUG_FIX;
   
    CMDFIFO_SETUP(cf);

    lpPattern = (DWORD *) lpBrush->dp8BrushBits;
    switch(lpBrush->dp8BrushStyle)
        {
        case BS_SOLID:
#ifdef DEBUG
            DPF(DBG_DEBUG, 1, "HandleSpecialBrush: Solid Brush\n"); 
            __asm int 3;
#endif
            break;
   
        case BS_NULL:
            // Note that NULL/HOLLOW brush cannot be trivially
            // rejected.  There still may need to be processing
            // contingent on the rop.
            return  ((Rop & 0xffff0000) << 8);
            break;
   
        case BS_HATCHED:
            // According to documentation, physical background color is
            // stored in dpxBgColor and foreground in dpxFgColor, then
            // draw the bit pattern in dpxBrushmask.
            // Above applies if dib engine realizes the brush.
            // Note that Fg/Bg color regs will get subsequently set by caller.
            for (i=0; i < 4; i++)
                {
                mono0[i] = (lpBrush->dp8BrushMask[4*i]);
                mono1[i] = (lpBrush->dp8BrushMask[4*i+16]);
                }
            CMDFIFO_CHECKROOM(cf,3);
            SETPH(cf, SSTCP_PKT2|   
                pattern0aliasBit|
                pattern1aliasBit);
             
            // load mono pattern
            // DANGER WILL ROBINSON
            // dibengine realizes mono brushes has dword packed
            SET(cf, _FF(lpGRegs)->pattern0alias, *(DWORD*) mono0 );
            SET(cf, _FF(lpGRegs)->pattern1alias, *(DWORD*) mono1 );
            BUMP(3);
            CMDFIFO_EPILOG(cf);
            dwXprntFlg = 0L;

#if 0
		    //PRS #4976 
		    // Setting the SSTG_TRANSPARENT bit here is bad
		    // Causes failure in Verdict tests
		    // It is also believe that bkMode has no 
		    // meaning in BitBlt()
            if (lpDrawMode->bkMode & BKMODE_TRANSPARENT)
                {
                dwXprntFlg = SSTG_TRANSPARENT;
                }
#endif
            return ( SSTG_MONO_PATTERN | dwXprntFlg |
                    ((Rop &0xffff0000) << 8) );
            break;
   
        case BS_PATTERN:
#ifdef DEBUG
            //decodebrushflag(lpBrush);

            // When a CreatePatternBrush is used, a brush with
            // bits MONOVALID and not PATTERN MONO is given to
            // the bitblt call.  There appears to be no way to
            // create a mono brush from without using mono bitmap
            // so it appears that the correct filter is to check
            // PATTERNMONO for monochrome brushes. 
            if ( (lpBrush->dp8BrushFlags & PATTERNMONO) &&
                !(lpBrush->dp8BrushFlags & MONOVALID) )
                {
                DPF (DBG_DEBUG, 1, "HandleSpecialBrush: Pattern PATTERNMONO AND NOT MONOVALID\n"); 
                __asm int 3 
                }
#endif
   
            // color case
            if ( !(lpBrush->dp8BrushFlags & PATTERNMONO) )
                {
#ifdef DEBUG
                if ( !(lpBrush->dp8BrushBpp && 7) )
                    {
                    DPF (DBG_DEBUG, 128, "HandleSpecialBrush: ???\n"); 
                    __asm int 3
                    }
#endif

                // in BYTES
                patcount = (64 * lpBrush->dp8BrushBpp >> 3) ;
                dwpatcount = (DWORD) (patcount >> 2);
                CMDFIFO_CHECKROOM(cf,dwpatcount+1);
                SETPH(cf, SSTCP_PKT1|   
                               SSTCP_PKT1_2D|
                               SSTCP_INC|
                               // register number starts with clipmin0(2)
                               PATTERN_REG_1 << SSTCP_REGBASE_SHIFT|
                               dwpatcount << SSTCP_PKT1_NWORDS_SHIFT);

                for (i=0; i < dwpatcount; i++)
                    {
                    SET(cf,_FF(lpGRegs)->colorPattern[i], lpPattern[i]); 
                    }

                BUMP(dwpatcount+1);
                CMDFIFO_EPILOG(cf);
                return( ((Rop & 0xFFFF0000) << 8) );
                // Hardware aligns pattern to screen
                // no pattern alignment needed.
                }
             else
                {
                // mono case
                // The mono brush is declared as BYTE +BRUSHSIZE*4]
                // which would imply dibeng realizes mono brushes
                // as dword packed.
                for (i=0; i < 4; i++)
                    {
                    mono0[i] = (lpBrush->dp8BrushMono[4*i]);
                    mono1[i] = (lpBrush->dp8BrushMono[4*i+16]);
                    }

                // Note that Fg/Bg color regs will get subsequently set by caller.
                CMDFIFO_CHECKROOM(cf,3);
                SETPH(cf, SSTCP_PKT2|   
                               pattern0aliasBit|
                               pattern1aliasBit);
                SET(cf, _FF(lpGRegs)->pattern0alias, *(DWORD*) mono0 );
                SET(cf, _FF(lpGRegs)->pattern1alias, *(DWORD*) mono1 );
                BUMP(3);

                CMDFIFO_EPILOG(cf);
                dwtemp = ( ((Rop & 0xFFFF0000) << 8) |
                             SSTG_MONO_PATTERN );
                return dwtemp;
                }
            break;
   
        case BS_INDEXED: 
#ifdef DEBUG
            DPF (DBG_DEBUG, 1, "HandleSpecialBrush: BS_INDEXED\n"); 
            __asm int 3
#endif
            break;
   
        case BS_DIBPATTERN:
#ifdef DEBUG
            DPF (DBG_DEBUG, 1, "HandleSpecialBrush: BS_DIBPATTERN\n"); 
            __asm int 3
#endif
            break;

        case BS_DIBPATTERNPT:
#ifdef DEBUG
            DPF (DBG_DEBUG, 1, "HandleSpecialBrush: BS_DIBPATTERNPT\n"); 
            __asm int 3
#endif
            break;

        case BS_PATTERN8X8:
#ifdef DEBUG
            DPF (DBG_DEBUG, 1, "HandleSpecialBrush: BS_PATTERN8X8\n"); 
            __asm int 3
#endif
            break;

        case BS_DIBPATTERN8X8:
#ifdef DEBUG
            DPF (DBG_DEBUG, 1, "HandleSpecialBrush: BS_DIBPATTERN8X8\n"); 
            __asm int 3
#endif
            break;

        }
}


/*----------------------------------------------------------------------
Function name:  ExpandMonoBrush

Description:    Expands the monochrome brush into something
                useful for the hardware.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void 
ExpandMonoBrush(short DstBytePix, 
		DIB_Brush8 FAR *lpPBrush, 
		DRAWMODE FAR *lpDrawMode, 
		BYTE * patbuf)
{
    BYTE monopatrow;
    short  bufinc;
    DWORD fgcol, bkcol;
    short i, j;
    short Colcount;
    BYTE * patternPtr;

    DEBUG_FIX;
   
    Colcount = (DstBytePix * ROWSIZE * COLSIZE) >> 2;

    bufinc = 0;
    fgcol = lpDrawMode->TextColor;
    bkcol = lpDrawMode->bkColor;

    switch (lpPBrush->dp8BrushStyle)
    {
      case BS_HATCHED:
	  patternPtr = lpPBrush->dp8BrushMask;
	  break;   

      case BS_PATTERN:
	  patternPtr = lpPBrush->dp8BrushMono;
	  break;   
    }

    switch(DstBytePix)
    {
      case 1:
	  for (i = 0; i < 8; i++)
	  {
	      // read first byte from mono pattern;
	      monopatrow = patternPtr[4*i];

	      for (j = 0; j < 8; j++)
	      {
		  if ( WORDBIT(j) & monopatrow)
		  {
		      *(patbuf+bufinc) = (BYTE) bkcol;
		      bufinc+=DstBytePix;
		  }
		  else
		  {
		      *(patbuf+bufinc) = (BYTE) fgcol;
		      bufinc+=DstBytePix;
		  }
	      }
	  }
	  break;

      case 2:
	  for (i = 0; i < 8; i++)
	  {
	      monopatrow = (patternPtr[4*i]);
	      for (j=0; j<8; j++)
	      {
		  if (WORDBIT(j) & monopatrow)
		  {
		      *(WORD*)(patbuf+bufinc) = (WORD) fgcol;

		      //* (WORD*)(patbuf + bufinc) = (WORD) bkcol;
		      bufinc+=DstBytePix;
		  }
		  else
		  {
		      //   *(WORD*)(patbuf+bufinc) = (WORD) fgcol;

		      * (WORD*)(patbuf + bufinc) = (WORD) bkcol;
		      bufinc+=DstBytePix;
		  }
	      }
	  }
	  break;
      case 3:
	  for (i = 0; i < 8; i++)
	  {
	      monopatrow = (patternPtr[4*i]);
	      for (j=0; j<8; j++)
	      {
		  if (WORDBIT(j) & monopatrow)
		  {
		      *(DWORD*)(patbuf+bufinc) = (DWORD) fgcol;
		      bufinc+=DstBytePix;

		  }
		  else
		  {

		      * (WORD*)(patbuf + bufinc) = (WORD) bkcol;
		      bufinc+=DstBytePix;
		  }
	      }
	  }
	  // got to handle;
      case 4:    
	  for (i = 0; i < 8; i++) 
	  {
	      monopatrow = (patternPtr[4*i]);
	      for (j=0; j<8; j++)
	      {
		  if (WORDBIT(j) & monopatrow)
		  {
		      * (DWORD*)(patbuf + bufinc) = (DWORD) bkcol;
		      bufinc+=DstBytePix;
		  }
		  else
		  {
		      *(DWORD*)(patbuf+bufinc) = (DWORD) fgcol;
		      bufinc+=DstBytePix;
		  }
	      }
	  }
	  break;
    }

}


/*----------------------------------------------------------------------
Function name:  WriteColorSrc

Description:    Writes a block of data to the hardware.

Information:    The routine will either write the all of the
                data in one packet if it small or it will send
                a scanline packet at a time.

                lpSrc points to a DIBENGINE structure.

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
LONG WriteColorSrc(DIBENGINE FAR * lpSrc,
    DWORD SrcOffset ,
    DWORD NumberOfScans,
    DWORD DwordsInScanline)
{

    DWORD i, j,k,  TotalDwords ;
    DWORD * lpData;
    DWORD lpTmp16;           // tmp 16:16 pointer
    CMDFIFO_PROLOG(cf);
    
    DEBUG_FIX;

    CMDFIFO_SETUP(cf);

    // Dwords in a scanline can not exceed 

    // DIBENGINE structure keeps pointer to bits in a 16:32 address.
    // Selector is in deBitsSelector and the offset is in deBitsOffset.
    // To convert to flat pointer, convert deBitsSelector:0 to flat
    // pointer and add deBitsOffset.

   
    lpTmp16= ((DWORD)(lpSrc->deBitsSelector)) << 16;
    FarToFlat(lpTmp16, lpData)
	(DWORD)lpData = (DWORD)(lpData) + lpSrc->deBitsOffset + SrcOffset;
    TotalDwords = DwordsInScanline*(NumberOfScans);
    if (TotalDwords < MAXCOLORDWORDS)
    {
	// MAXCOLORWORDS is used to special case small bitmaps
	// that will be sent without breaking it up by scanlines.

	//   DPF("WriteColorSrc: entire bitmap ");
	CMDFIFO_CHECKROOM(cf, TotalDwords+1);
	SETPH(cf, (SSTCP_PKT1 |
		   SSTCP_PKT1_2D |
		   (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
		   ((TotalDwords) << SSTCP_PKT1_NWORDS_SHIFT)));
	k = 0;
	for (i = 0; i < NumberOfScans; i++)
	{
	    for (j = 0; j < DwordsInScanline; j++)
	    {
		SET(cf, _FF(lpGRegs)->launch[0], lpData[j]);
		k++;
	    }
	    (DWORD) lpData+= lpSrc->deDeltaScan;
	}
	BUMP(TotalDwords + 1);
    }
    else
    {
	//Offset=SrcOffset;
	//DPF("WriteColorSrc: Scanline ");
	for (i = 0; i < NumberOfScans; i++)
	{
	    CMDFIFO_CHECKROOM(cf,  DwordsInScanline + 1 );
	    SETPH(cf, (SSTCP_PKT1 |
		       SSTCP_PKT1_2D |
		       (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
		       ((DwordsInScanline) << SSTCP_PKT1_NWORDS_SHIFT)));

	    for (j = 0; j < DwordsInScanline; j++)
	    {
		SET(cf, _FF(lpGRegs)->launch[0], lpData[j]);
	    }   

	    (DWORD) lpData += lpSrc->deDeltaScan;
	    BUMP(DwordsInScanline +1);
	}

    }   // else
      
    CMDFIFO_EPILOG(cf);

    return 1;
}


/*----------------------------------------------------------------------
Function name:  WriteColorSrc

Description:    Writes a block of data to the hardware.

Information:    The routine will either write the all of the
                data in one packet if it small or it will send
                a scanline packet at a time.

                lpSrc points to a PBITMAP structure.

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
LONG WriteBMPColorSrc(DIBENGINE FAR * lpSrcParam,
    DWORD SrcOffset ,
    DWORD NumberOfScans,
    DWORD DwordsInScanline)
{

    DWORD i, j,k,  TotalDwords ;
    DWORD * lpData;
    LPBITMAP lpSrc;

    CMDFIFO_PROLOG(cf);
    
    DEBUG_FIX;

    CMDFIFO_SETUP(cf);

    lpSrc = (BITMAP *) lpSrcParam;  // lpSrc points to a PBITMAP structure

    FarToFlat(lpSrc->bmBits, lpData)
	(DWORD)lpData = (DWORD)(lpData) + SrcOffset;

    TotalDwords = DwordsInScanline * NumberOfScans;
    if (TotalDwords < MAXCOLORDWORDS)
    {
	// MAXCOLORWORDS is used to special case small bitmaps
	// that will be sent without breaking it up by scanlines.

	//   DPF("WriteColorSrc: entire bitmap ");
	CMDFIFO_CHECKROOM(cf, TotalDwords+1);
	SETPH(cf, (SSTCP_PKT1 |
		   SSTCP_PKT1_2D |
		   (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
		   ((TotalDwords) << SSTCP_PKT1_NWORDS_SHIFT)));
	k = 0;
	for (i = 0; i < NumberOfScans; i++)
	{
	    for (j = 0; j < DwordsInScanline; j++)
	    {
		SET(cf, _FF(lpGRegs)->launch[0], lpData[j]);
		k++;
	    }
	    (DWORD) lpData+= lpSrc->bmWidthBytes;
	}
	BUMP(TotalDwords + 1);
    }
    else
    {
	//Offset=SrcOffset;
	//DPF("WriteColorSrc: Scanline ");
	for (i = 0; i < NumberOfScans; i++)
	{
	    CMDFIFO_CHECKROOM(cf,  DwordsInScanline + 1 );
	    SETPH(cf, (SSTCP_PKT1 |
		       SSTCP_PKT1_2D |
		       (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
		       ((DwordsInScanline) << SSTCP_PKT1_NWORDS_SHIFT)));

	    for (j = 0; j < DwordsInScanline; j++)
	    {
		SET(cf, _FF(lpGRegs)->launch[0], lpData[j]);
	    }   

	    (DWORD) lpData += lpSrc->bmWidthBytes;
	    BUMP(DwordsInScanline +1);
	}

    }   // else
      
    CMDFIFO_EPILOG(cf);

    return 1;
}


/*----------------------------------------------------------------------
Function name:  WriteColorSrc

Description:    Writes a block of data to the hardware.

Information:

Return:         LONG    1 is always returned.
----------------------------------------------------------------------*/
WriteMonoSrc(LPBITMAP lpSrc,
	     DWORD SrcOffset,
	     DWORD NumberOfScans,
	     DWORD DwordsInScanline)
{
    DWORD TotalDwords;
    int i, j, index;
    DWORD *  lpData;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;
    
    CMDFIFO_SETUP(cf);

    index = 0;
    FarToFlat((DWORD)lpSrc->bmBits, lpData);
    (DWORD) (lpData) += SrcOffset;

    //   DPF("WriteMonoSrc: entire bitmap ");
    TotalDwords = DwordsInScanline * (NumberOfScans);
    CMDFIFO_CHECKROOM(cf, TotalDwords + 1);
    SETPH(cf, (SSTCP_PKT1 |
	       SSTCP_PKT1_2D |
	       (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
	       ((TotalDwords) << SSTCP_PKT1_NWORDS_SHIFT)));

    for (i = 0; i < (int)NumberOfScans; i++)
    {
	for (j = 0; j < (int)DwordsInScanline; j++)
	{
	    SET(cf, _FF(lpGRegs)->launch[0], lpData[j]);
	}
	(DWORD) lpData+= lpSrc->bmWidthBytes;
    }

    BUMP(TotalDwords + 1); 

    CMDFIFO_EPILOG(cf); 

    return 1;
}
