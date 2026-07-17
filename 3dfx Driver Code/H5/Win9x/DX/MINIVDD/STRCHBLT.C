/* -*-c++-*- */
/* $Header: strchblt.c, 7, 10/11/00 8:55:08 PM, Brent$ */
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
** File name:   strchblt.c
**
** Description: The GDI StretchBlt routines.
**
** $Revision: 7$
** $Date: 10/11/00 8:55:08 PM$
**
** $History: strchblt.c $
** 
** *****************  Version 43  *****************
** User: Andrew       Date: 9/13/99    Time: 5:10p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to fix the Y Direction Blit bug aka PRS 8191
** 
** *****************  Version 42  *****************
** User: Cwilcox      Date: 7/07/99    Time: 2:32p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Punt only for hollow brush and ROP requires pattern.
** 
** *****************  Version 41  *****************
** User: Andrew       Date: 6/15/99    Time: 5:08p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed _HWC_EXCLUSIVE flag to SKIP_FLAG so that path can be used with
** SLI_AA_SLAVE mode.
** 
** *****************  Version 39  *****************
** User: Stb_mimhoff  Date: 3/10/99    Time: 4:48p
** Updated in $/devel/h3/win95/dx/minivdd
** Fix for Verdict StretchBlt Integer Stretch/ROP/Integer Stretch drawing
** failure.
** 
** *****************  Version 38  *****************
** User: Cwilcox      Date: 1/22/99    Time: 2:08p
** Updated in $/devel/h3/Win95/dx/minivdd
** Minor revisions to clean up compiler warnings.
** 
** *****************  Version 37  *****************
** User: Michael      Date: 1/15/99    Time: 6:35a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 36  *****************
** User: Michael      Date: 11/12/98   Time: 10:02a
** Updated in $/devel/h3/Win95/dx/minivdd
** ChrisE's (igx) fix for the STB TV tuner bug.  A deficiency in the
** calculation of the number of dwords per scanline in LoadColorBuffer was
** causing the error.  Modified the calculation to match how BitBlt
** perform the calculation for number of dwords in scanline.  Fixes 2799 &
** 3128.  Note that this fix is different than GLOP.  The fix in GLOP only
** punted around HWStretchBltMemory.
** 
** *****************  Version 35  *****************
** User: Andrew       Date: 7/17/98    Time: 3:20p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Quick exit if HWC Exclusive is Set
** 
** *****************  Version 34  *****************
** User: Ken          Date: 7/16/98    Time: 1:22p
** Updated in $/devel/h3/win95/dx/minivdd
** optimized thunk16.asm, and tweaked dibblt.c and strchblt.c for Mercury
** 
** *****************  Version 33  *****************
** User: Michael      Date: 6/23/98    Time: 3:59p
** Updated in $/devel/h3/Win95/dx/minivdd
** ChrisE (IGX) - Here's the fix for the Magnify bug. The problem was in
** HWStretchBltToMemory. The bresenham loop stretches the current buffer
** then loads the next scanline if the calculation calls for the Y to
** advance. The problem was that if the stretch was the last scanline of
** the source and the error term rolled over, the loop thought it had to
** load another scanline,
** even though we just stretched the last one. So, I added a last scanline
** check before the while loop that loads the scanlines into the offscreen
** stretch buffer. The crash actually happened in LoadColorSrc. The same
** bug existed in the monochrome section of HWStretchBltToMemory, so I
** fixed it there as well.  Fixes 2014.
** 
** 
** *****************  Version 32  *****************
** User: Michael      Date: 5/22/98    Time: 7:21p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix #1557 by punting to dib engine for mirror image bitmaps.
** 
** *****************  Version 31  *****************
** User: Michael      Date: 5/18/98    Time: 10:09a
** Updated in $/devel/h3/Win95/dx/minivdd
** In FxStretchBlt, Modify all "bpunt=1, return 1" occurences to FXPUNTs.
** Add FXLEAVE_NOHWACCESS.
** 
** *****************  Version 30  *****************
** User: Michael      Date: 5/15/98    Time: 4:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Backed out my previous changes.
** 
** *****************  Version 29  *****************
** User: Michael      Date: 5/15/98    Time: 9:29a
** Updated in $/devel/h3/Win95/dx/minivdd
** Add lpDst to FXPUNT and FXLEAVE_NOHWACCESS.
** 
** *****************  Version 28  *****************
** User: Andrew       Date: 5/14/98    Time: 10:18a
** Updated in $/devel/h3/Win95/dx/minivdd
** changed startOffset to be a byte to byte add instead of byte to dword
** 
** *****************  Version 27  *****************
** User: Ken          Date: 5/14/98    Time: 8:31a
** Updated in $/devel/h3/win95/dx/minivdd
** added a PERFNOP case to gdi32.   change the "undef" to a "define"
** in fifomgr.h, and enable in softice (doperfnop = 1).  Replaces every
** write
** to the command register with a "0"
** 
** *****************  Version 26  *****************
** User: Andrew       Date: 5/13/98    Time: 11:25a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed a problem with LoadMonoBuffer which would cause my machine to
** hang when running GDI/User Inspection
** 
** *****************  Version 25  *****************
** User: Edwin        Date: 4/30/98    Time: 3:10p
** Updated in $/devel/h3/Win95/dx/minivdd
** fix defect 1657: src and dst X,Y are the same
** 
** *****************  Version 24  *****************
** User: Ken          Date: 4/27/98    Time: 5:35p
** Updated in $/devel/h3/win95/dx/minivdd
** complete (well, almost) enter/leave implementation for gdi32
** 
** *****************  Version 23  *****************
** User: Ken          Date: 4/24/98    Time: 9:43p
** Updated in $/devel/h3/win95/dx/minivdd
** cleaned up some functions, added R32 macros, 
** 
** *****************  Version 22  *****************
** User: Ken          Date: 4/24/98    Time: 1:33p
** Updated in $/devel/h3/win95/dx/minivdd
** first installment of FXENTER / FXLEAVE display driver drawing function
** work.  gdi32 only, no gdi16 yet.   works, but is slow, don't measure
** performance until all work is done, in about a week.
** also renamed fields in some blit parameter functions to be a bit more
** meaningful
** 
** *****************  Version 21  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
** 
** *****************  Version 20  *****************
** User: Ken          Date: 4/10/98    Time: 3:16p
** Updated in $/devel/h3/win95/dx/minivdd
** ken's fixes to chris e's fixes to stretch blit clipping
** 
** *****************  Version 18  *****************
** User: Ken          Date: 4/07/98    Time: 7:31p
** Updated in $/devel/h3/win95/dx/minivdd
** commented out dpfs so debug build works
** 
** *****************  Version 17  *****************
** User: Ken          Date: 4/07/98    Time: 6:40p
** Updated in $/devel/h3/win95/dx/minivdd
** chris e's stretch position fix and some reformatting, added header,
** still has clipping bugs
**
*/

#include "h3.h"
#include "thunk32.h"
#include "bitblt.h"
#include "strchblt.h"
#include "h3g.h"
#include "entrleav.h"

// variables to turn on and off blt info messages for Stretchblt.
int STPSBBDbg=1;
int STHSBBDbg=1;
int STSSBBDbg=1;

   
int retval;
//int ss=0;
//int test=0;
//int stop =1;

extern GLOBALDATA * lpDriverData;

#define ROWSIZE 8
#define COLSIZE 8

#define RANGE(val, lo, hi) ((lo <= val) && (val <= hi))
#define XDS_INTERSECT 0x0001
#define XDE_INTERSECT 0x0002
#define YDS_INTERSECT 0x0004
#define YDE_INTERSECT 0x0008

int Intersection[] =
   {
   FALSE,            //0x0000 No Intersection
   FALSE,            //0x0001 X Dest Start
   FALSE,            //0x0002 X Dest End
   FALSE,            //0x0003 X Dest Start & X Dest End
   FALSE,            //0x0004 Y Dest Start
   TRUE,             //0x0005 X Dest Start and Y Dest Start
   TRUE,             //0x0006 X Dest End and Y Dest Start
   TRUE,             //0x0007 X Dest Start/End and Y Dest Start
   FALSE,            //0x0008 Y Dest End
   TRUE,             //0x0009 X Dest Start and Y Dest End
   TRUE,             //0x000A X Dest End and Y Dest End
   TRUE,             //0x000B X Dest Start/End and Y Dest End
   FALSE,            //0x000C Y Dest Start/End
   TRUE,             //0x000D X Dest Start and Y Dest Start/End
   TRUE,             //0x000E X Dest End and Y Dest Start/End 
   TRUE,             //0x000F All
   };

/*----------------------------------------------------------------------
Function name:  FxStretchBlt

Description:    32-bit side of the GDI StretchBlt function.

Information:    

Return:         BOOL    1 if punted, -1 if mirrored bitmap.
----------------------------------------------------------------------*/
#pragma optimize("",on) 
BOOL 
FxStretchBlt(StretchBltParams * lp)
    //   DIBENGINE FAR  *Dst,
    //   WORD          DstX,
    //   WORD          DstY,
    //   WORD          dstWidth,
    //   WORD          dstHeight,
    //   DIBENGINE FAR  *Src,
    //   WORD          SrcX,
    //   WORD          SrcY,
    //   WORD          srcWidth,
    //   WORD          srcHeight,
    //   DWORD         Rop,
    //   DIB_Brush8 FAR *lpPBrush,
    //   DRAWMODE FAR   *lpDrawMode,
    //   RECT FAR *lpClipRect
{
    LPDIBENGINE  lpDst;
    LPDIBENGINE  lpSrc;
    WORD newleft, newright, newbottom, newtop;

    DEBUG_FIX;

    lpSrc = NULL;
    FarToFlat(lp->lpDestDev, lpDst);

    // quick exit if in exclusive mode
    if (_FF(gdiFlags) & SKIP_FLAGS)
         return 1;

#ifdef DEBUG
    if (UseDibEng)
    {
    FXPUNT(FXPUNT_NORMAL);
    }
    if (!(hwStretchBlt&hwAll))
    {
    FXPUNT(FXPUNT_NORMAL);
    }

    if ( lpDst->deFlags&BUSY) 
    {
	//   DPF("BUSY: Stretchblt");
	__asm int 3;
    } 

#endif

    if (lp->lpSrcDev)
    {
	FarToFlat(lp->lpSrcDev, lpSrc);
    }

    if (!RopUsesSrc(lp->Rop >> 16))
    {
	goto SkipSrcClip;
    }

    // StretchBlt creates a mirror image of a bitmap if
    // the signs of the srcWidth and dstWidth or srcHeight
    // and dstHeight parameters differ.

    // test for mirrored horizontal bitmap
    if (((short)lp->srcWidth & 0x8000) ^ ((short)lp->dstWidth & 0x8000))
    {
	  retval = -1; //let dib eng do mirroring
	  goto Use_DibEng;
    }

    // test for mirrored vertical bitmap
    if (((short)lp->srcHeight & 0x8000) ^ ((short)lp->dstHeight & 0x8000))
    {
	  retval = -1; //let dib eng do mirroring
	  goto Use_DibEng;
    }

    // Src clipping 
    newleft= lp->SrcX;
    newtop= lp->SrcY;
    newright = lp->SrcX + lp->srcWidth;
    newbottom = lp->SrcY + lp->srcHeight;

    if (( newright <= 0 ) || (newbottom <= 0))
    {
	return 1;
    }
   
    // clip left   
    if (newleft < 0)
    {
	lp->DstX -= newleft;
	lp->srcWidth = newright;
	lp->SrcX =0;
    }

    //clip top 
    if (newtop < 0)
    {
	lp->DstY -= newtop;
	lp->srcHeight = newbottom;
	lp->SrcY = 0;
    }

    // clip right
    if (newright > lpSrc->deWidth)
    {
	newright -= lpSrc->deWidth;
	lp->srcWidth -= newright;
	if (lp->srcWidth <= 0)
	{   
	    return 1;
	}
    }

    // clip bottom
    if (newbottom > lpSrc->deHeight)   
    {
	newbottom-=lpSrc->deHeight;
	lp->srcHeight -= newbottom;
	if (lp->srcHeight <= 0)
	{   
	    return 1;
	}
    }   
   
SkipSrcClip:

    if (lp->lpSrcDev)
    {
	if ((lpSrc==lpDst) || (lpSrc->deFlags & VRAM))
	{
	    retval = HWStretchBltScreen(lp, lpDst, lpSrc);
	}
	else   
	{ 
	    if (RopUsesSrc(lp->Rop >> 16))  
	    {
		retval =  HWStretchBltMemory(lp, lpDst, lpSrc);
	    }
   
	    else
	    {
		retval =  HWStretchBltPattern(lp, lpDst, lpSrc);
	    }
	}         
    }
    else
    {   
	retval =  HWStretchBltPattern(lp,lpDst, lpSrc);
    }

    //needed for FXPUNT to resolve labels
    FXLEAVE_NOHWACCESS("FxStretchBlt", retval);
}


/*----------------------------------------------------------------------
Function name:  HWStretchBltScreen

Description:    Called to do a StretchBlt when the source and
                dest device is the screen
Information:    
    Handles following cases:
        On screen to On screen  only. (For cases involving offscreen
        see HWStretchBltOffScreen). 
        Note Hardware does not support negative X dir bitblt.

Return:         BOOL
----------------------------------------------------------------------*/
BOOL
HWStretchBltScreen(StretchBltParams   * lp,
		   DIBENGINE * lpDst,
		   DIBENGINE * lpSrc)
		   //   DIBENGINE FAR  *Dst,
		   //   WORD          DstX,
		   //   WORD          DstY,
		   //   WORD          dstWidth,
		   //   WORD          dstHeight,
		   //   DIBENGINE FAR  *Src,
		   //   WORD          SrcX,
		   //   WORD          SrcY,
		   //   WORD          srcWidth,
		   //   WORD          srcHeight,
		   //   DWORD         Rop,
		   //   DIB_Brush8 FAR *lpPBrush,
		   //   DRAWMODE FAR   *lpDrawMode,
		   //   RECT FAR *lpClipRect
{ 
    DIB_Brush8 FAR *lpBrush;
    DRAWMODE FAR   *lpDrawMode;
    RECT FAR *lpClipRect;
    DWORD DirBit;
    DWORD dstXYData, srcXYData , cmdData;
    DWORD dstSizeData, srcSizeData ;
    DWORD clipSelect;
    CMDFIFO_PROLOG(cf);
    
    DEBUG_FIX;

    clipSelect = 0;
    lpClipRect = NULL;
    FarToFlat(lp->lpPBrush, lpBrush);
    FarToFlat(lp->lpDrawMode, lpDrawMode);

    dstXYData =0 ;
    srcXYData =0 ;

    if ((lpBrush->dp8BrushStyle == BS_HOLLOW) && (RopUsesPat(lp->Rop >> 16)))
    {
    FXPUNT(FXPUNT_NORMAL);
    }

    FXENTER("HWStretchBltScreen", lpDst->deFlags, (hwBlt && hwStretchBltSS),
	    cf, lpDst, lpSrc, FXENTER_COMPUTE_SRCFORMAT,
	    FXENTER_RECT, lp->SrcX, lp->SrcY,
	                  (lp->SrcX + lp->srcWidth), (lp->SrcY+lp->srcHeight),
	    FXENTER_RECT, lp->DstX, lp->DstY,
	                  (lp->DstX + lp->dstWidth), (lp->DstY+lp->dstHeight));
    
    // if we've got a clip rect, prepare it for flat access
    if ( lp->lpClipRect )
    {
        FarToFlat( lp->lpClipRect, lpClipRect );
        // now tell banshee about the clip rect
	CMDFIFO_CHECKROOM(cf, 3);
        SETPH(cf, SSTCP_PKT2 |
	      clip1minBit |
	      clip1maxBit);
        SET(cf, _FF(lpGRegs)->clip1min, RECT_TOP_LEFT(lpClipRect));
        SET(cf, _FF(lpGRegs)->clip1max, RECT_BOTTOM_RIGHT(lpClipRect));
        BUMP(3);
	clipSelect = SSTG_CLIPSELECT;
    }

    /*
        1 |  2  | 3   if dstX, dstY falls in grid 1,2,3,5,6,7,8
       -------------     copy from top left corner
        4 | src | 5   else 
       -------------     copy from bottom right corner
        6 |  7  | 8

    pseudo code:
    if ( (lp->DstY < lp->SrcY)                   // test for grid 1,2,3
       || (lp->DstY >= lp->SrcY + lp->srcHeight) // test for grid 6,7,8
       || (lp->DstX >= lp->SrcX + lp->srcWidth)  // test for grid 5
       )
       top to bottom blt;
    else
       bottom to top blt;

    To simplify the code, we can perform normal top to bottom blt if dstY is 
    less than srcY, and all other cases use bottom to top blt.
    */
    if (lpSrc == lpDst)
      {
      if (lp->DstY < lp->SrcY)
         {
         DirBit =  0L;
         srcXYData = R32(lp->SrcY, lp->SrcX);
         dstXYData = R32(lp->DstY, lp->DstX);
         }
      else
         {
         int Xss, Xse, Yss, Yse, Xds, Xde, Yds, Yde;
         int Intersect; 
         // Ok time to get serious
         // If intersection punt
         // else do it

         Xss = lp->SrcX;
         Xse = lp->SrcX + lp->srcWidth;
         Yss = lp->SrcY;
         Yse = lp->SrcY + lp->srcHeight;

         Xds = lp->DstX;
         Xde = lp->DstX + lp->dstWidth;
         Yds = lp->DstY;
         Yde = lp->DstY + lp->dstHeight;

         Intersect = 0;
         if (RANGE(Xds, Xss, Xse))
            Intersect |= XDS_INTERSECT;
          if (RANGE(Xde, Xss, Xse))
            Intersect |= XDE_INTERSECT;
          if (RANGE(Yds, Yss, Yse))
            Intersect |= YDS_INTERSECT;
          if (RANGE(Yde, Yss, Yse))
            Intersect |= YDE_INTERSECT;
          
         if (Intersection[Intersect])
            {
            //_asm {int 03h};
            bPunt = TRUE;
            goto Special_Punt;
            }
         else
            {
            DirBit =  0L;
            srcXYData = R32(lp->SrcY, lp->SrcX);
            dstXYData = R32(lp->DstY, lp->DstX);
            }
         }
      }
    else
      {
      DirBit =  0L;
      srcXYData = R32(lp->SrcY, lp->SrcX);
      dstXYData = R32(lp->DstY, lp->DstX);
      }

    srcSizeData = ((((DWORD)lp->srcHeight ) << 16) |
		  ((DWORD)lp->srcWidth)) ;
    dstSizeData = ((((DWORD)lp->dstHeight ) << 16) |
		   ((DWORD)lp->dstWidth)) ;

    // With Stretch blt Negative direction is not needed
    // since all the screen to screen will most likely be done 
    // from offscreen to onscreen or vice versa so overlapping 
    // blts are not an issue.
    // Note any blting with Offscreen is NOT handled in this routine.
      

    // ternary rops. 
    // Src, Dst Pat.
    if (RopUsesPat(lp->Rop >> 16))
    {
	CMDFIFO_CHECKROOM(cf, 4);
	SETPH(cf, (SSTCP_PKT2 |      
			srcFormatBit |
			srcSizeBit |
			srcXYBit));
	SET(cf, _FF(lpGRegs)->srcFormat, _SrcFormat);
	SET(cf, _FF(lpGRegs)->srcSize, srcSizeData);
	SET(cf, _FF(lpGRegs)->srcXY, srcXYData);
	// need to end packet because have not processed brush
	BUMP(4);

	// HandleBrush  calls prolog ...
	CMDFIFO_SAVE(cf);
	cmdData = HandleBrush(lpBrush, lp->Rop, lpDrawMode);
	CMDFIFO_RELOAD(cf);

	CMDFIFO_CHECKROOM(cf, 4);
	SETPH(cf, (SSTCP_PKT2 |
			dstSizeBit |
			dstXYBit |
			commandBit));
	SET(cf, _FF(lpGRegs)->dstSize, dstSizeData );
	SET(cf, _FF(lpGRegs)->dstXY, dstXYData);
	SETC(cf, _FF(lpGRegs)->command, (cmdData |
					     clipSelect |
					     DirBit  |
					     SSTG_GO |
					     SSTG_STRETCH_BLT));
	BUMP(4); 
	CLEAR_COMMAND_EX(cf);
    }
    else    // setup  for ss without pattern.
    {
	// since we don't call HandleBrush packet header and fifo stuff
	// needs to be called here
	CMDFIFO_CHECKROOM(cf, 7);
	SETPH(cf, (SSTCP_PKT2 |
			srcFormatBit |
			srcSizeBit |
			srcXYBit |
			dstSizeBit |
			dstXYBit |
			commandBit));
	SET(cf,  _FF(lpGRegs)->srcFormat, _SrcFormat);
	SET(cf,  _FF(lpGRegs)->srcSize , srcSizeData);
	SET(cf,  _FF(lpGRegs)->srcXY , srcXYData);
	SET(cf,  _FF(lpGRegs)->dstSize , dstSizeData );
	SET(cf,  _FF(lpGRegs)->dstXY , dstXYData);
	SETC(cf,  _FF(lpGRegs)->command,
	    (((lp->Rop & 0xFFFF0000) << 8) |
	     clipSelect |
	     SSTG_GO |
	     DirBit  |
	     SSTG_STRETCH_BLT));
	BUMP(7); 
    }

Special_Punt:
    FXLEAVE("HWStretchBltScreen", cf, lpDst);    
}


/*----------------------------------------------------------------------
Function name:  HWStretchBltPattern

Description:    Called to do a StretchBlt for the following cases:
                1.  pat blt with color pattern
                2.  pat blt with mono pattern
                3.  pat blt without pattern (whiteness, Blackeness, Dn   
Information:    
    Caveats:
    1.  no src  Should same as HWBitBltPattern except uses dstWidth
        and dstHeight

Return:         INT
----------------------------------------------------------------------*/
#pragma optimize("",on)
int
HWStretchBltPattern(StretchBltParams * lp, 
		    DIBENGINE * lpDst, 
		    DIBENGINE * lpSrc)
   //DIBENGINE FAR  *Dst,
   //WORD          DstX,
   //WORD          DstY,
   //WORD          dstWidth,
   //WORD          dstHeight,
   //DIBENGINE FAR  *Src,
   //WORD          SrcX,
   //WORD          SrcY,
   //WORD          srcWidth,
   //WORD          srcHeight,
   //DWORD         Rop,
   //DIB_Brush8 FAR *lpBrush,
   //DRAWMODE FAR   *lpDrawMode,
   //RECT FAR *lpClipRect)
{
    DWORD  cmdData;
    DIB_Brush8 FAR *lpBrush;
    DRAWMODE FAR   *lpDrawMode;
    RECT FAR *lpClipRect;
    DWORD clipSelect;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    clipSelect = 0;
    lpClipRect = NULL;
    FarToFlat(lp->lpPBrush, lpBrush);
    FarToFlat(lp->lpDrawMode, lpDrawMode);

    if ((lpBrush->dp8BrushStyle == BS_HOLLOW) && (RopUsesPat(lp->Rop >> 16)))
    {
    FXPUNT(FXPUNT_NORMAL);
    }

    FXENTER("HWStretchBltPattern", lpDst->deFlags, (hwBlt && hwStretchBltPS),
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
	    FXENTER_RECT, lp->DstX, lp->DstY,
	                  lp->DstX + lp->dstWidth,
                          lp->DstY + lp->dstHeight,
	    FXENTER_NO_RECT, 0, 0, 0, 0   );

    // if we've got a clip rect, prepare it for flat access
    if ( lp->lpClipRect )
    {
        FarToFlat( lp->lpClipRect, lpClipRect );
        // now tell banshee about the clip rect
	CMDFIFO_CHECKROOM(cf, 3);
        SETPH(cf, SSTCP_PKT2 |
	      clip1minBit |
	      clip1maxBit);
        SET(cf, _FF(lpGRegs)->clip1min, RECT_TOP_LEFT(lpClipRect));
        SET(cf, _FF(lpGRegs)->clip1max, RECT_BOTTOM_RIGHT(lpClipRect));
        BUMP(3);
	clipSelect = SSTG_CLIPSELECT;
    }

    switch(lp->Rop)
    {
      case(WHITENESS):
	  // command
	  CMDFIFO_CHECKROOM(cf, 5);
	  SETPH(cf, SSTCP_PKT2|
		colorForeBit|      
		dstSizeBit|
		dstXYBit |
		commandBit);
	  SET(cf, _FF(lpGRegs)->colorFore, 0xFFFFFFFF);
	  SET(cf, _FF(lpGRegs)->dstSize,
	      ((((DWORD) lp->dstHeight) << 16) | 
	       ((DWORD)lp->dstWidth)) ) ;
	  SET(cf, _FF(lpGRegs)->dstXY,
	      ((((DWORD)lp->DstY) << 16) | 
	       ((DWORD)lp->DstX)) ) ;
	  SETC(cf, _FF(lpGRegs)->command, (SSTG_ROP_SRCCOPY | clipSelect |
					  SSTG_GO | SSTG_RECTFILL));
	  BUMP(5);    
	  break; // whiteness

      case(BLACKNESS):
	  CMDFIFO_CHECKROOM(cf, 5);
	  SETPH(cf, SSTCP_PKT2|
		colorForeBit|      
		dstSizeBit|
		dstXYBit |
		commandBit);
	  SET(cf, _FF(lpGRegs)->colorFore, 0x00000000);
	  SET(cf, _FF(lpGRegs)->dstSize , 
	      ((((DWORD) lp->dstHeight) << 16) |
	       ((DWORD)lp->dstWidth)) ) ;
	  SET(cf, _FF(lpGRegs)->dstXY ,
	      ((((DWORD)lp->DstY) << 16) |
	       ((DWORD)lp->DstX)) ) ;
	  SETC(cf, _FF(lpGRegs)->command, (SSTG_ROP_SRCCOPY | clipSelect |
					  SSTG_GO | SSTG_RECTFILL));
	  BUMP(5);    
	  break; //blackness

	  // p d dpon dpna pn dn dpx  dpan dpa pdxn d dpno dpo  
      default:

	  //  HandleBrush calls PROLOG ...
	  CMDFIFO_SAVE(cf);
	  cmdData=HandleBrush(lpBrush, lp->Rop, lpDrawMode);
	  CMDFIFO_RELOAD(cf);

	  CMDFIFO_CHECKROOM(cf, 4);
	  SETPH(cf, SSTCP_PKT2 | dstSizeBit | dstXYBit | commandBit);
	  SET(cf, _FF(lpGRegs)->dstSize, R32(lp->dstHeight, lp->dstWidth));
	  SET(cf, _FF(lpGRegs)->dstXY, R32(lp->DstY, lp->DstX));
	  SETC(cf, _FF(lpGRegs)->command, (cmdData | clipSelect | SSTG_GO |
					  SSTG_RECTFILL));
	  BUMP(4);    
	  CLEAR_COMMAND_EX(cf);
	  break; // default
    }

    FXLEAVE("HWStretchBltPattern", cf, lpDst);    
}


/*----------------------------------------------------------------------
Function name:  HWStretchBltMemory

Description:    Hardware does not support host to screen stretch blt.
                The driver will emulate HS Stretch blt by blting
                portions of the host src bitmap into offscreen memory
                and then blting the data stored in offscreen to the
                frame buffer.
Information:    
    Assumptions:
        No bpp conversion between src and dest.
        The src is always in dst format except when src is mono.

        The offscreen buffer size is MAX_OFFSCREEN_SIZE.
        HWBltOffscreen function will move the portion of the src
        bitmap to the offscreen buffer.

Return:         INT
----------------------------------------------------------------------*/
HWStretchBltMemory(StretchBltParams * lp, 
		   DIBENGINE * lpDst, 
		   DIBENGINE * lpSrc)
    //DIBENGINE FAR  *Dst,
    //WORD          DstX,
    //WORD          DstY,
    //WORD          dstWidth,
    //WORD          dstHeight,
    //DIBENGINE FAR  *Src,
    //WORD          SrcX,
    //WORD          SrcY,
    //WORD          srcWidth,
    //WORD          srcHeight,
    //DWORD         Rop,
    //DIB_Brush8 FAR *lpBrush,
    //DRAWMODE FAR   *lpDrawMode,
    //RECT FAR *lpClipRect)
{
    DIB_Brush8 FAR *lpBrush;
    DRAWMODE FAR   *lpDrawMode;
    RECT FAR *lpClipRect;
    DWORD  dwtemp;
    DWORD dwordsInScanlineG;
    DWORD leftAlignment, stride, startOffset;
    DWORD bufFormatData;
    BITMAP * lpbmp;
    int index, i, d;
    WORD srcScanY, dstScanY; 
    int deltaY, deltaX, twoDeltaX, twoDeltaY, threeDeltaX;
    DWORD clipSelect;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    clipSelect = 0;
    lpClipRect = NULL;
    FarToFlat(lp->lpPBrush, lpBrush);
    FarToFlat(lp->lpDrawMode, lpDrawMode);

    /*  
    ** Src can point to a DIBENGINE struct or PBITMAP struct.
    ** First word pointed by Src determines the type of struct.
    ** Src points to DIBENGINE structure if first word is 
    ** TYPE_DIBENG (0x5250) and PBITMAP if first word is 0.
      
    ** Driver only handles TYPE_DIBENG or mono nonTYPE_DIBENG
      

    if ( (lpSrc->deType == 0) & (lpSrc->deBitsPixel))
    {
    DPF ("COLOR  NON DIBENG TYPE");
    __asm int 3
    FXPUNT(FXPUNT_NORMAL);
    }
      
    */

    if ((lpBrush->dp8BrushStyle == BS_HOLLOW) && (RopUsesPat(lp->Rop >> 16)))
    {
    FXPUNT(FXPUNT_NORMAL);
    }

    if (lp->Rop != SRCCOPY)
    {
    FXPUNT(FXPUNT_NORMAL);
    }

//    if ((lp->srcHeight > lp->dstHeight) || (lp->srcWidth > lp->dstWidth))
    if (lp->srcHeight > lp->dstHeight)
    {
    FXPUNT(FXPUNT_NORMAL);
    }

    FXENTER("HWStretchBltMemory", lpDst->deFlags, (hwBlt && hwStretchBltHS),
	    cf, lpDst, FXENTER_NO_SRC,
	    (FXENTER_NO_SRCFORMAT | FXENTER_COMPUTE_DSTBA_FORMAT),
	    FXENTER_RECT, lp->DstX, lp->DstY,
	                  lp->DstX + lp->dstWidth,
                          lp->DstY + lp->dstHeight,
	    FXENTER_NO_RECT, 0, 0, 0, 0   );

    // if we've got a clip rect, prepare it for flat access
    if ( lp->lpClipRect )
    {
        FarToFlat( lp->lpClipRect, lpClipRect );
        // now tell banshee about the clip rect
	CMDFIFO_CHECKROOM(cf, 3);
        SETPH(cf, SSTCP_PKT2 |
	      clip1minBit |
	      clip1maxBit);
        SET(cf, _FF(lpGRegs)->clip1min, RECT_TOP_LEFT(lpClipRect));
        SET(cf, _FF(lpGRegs)->clip1max, RECT_BOTTOM_RIGHT(lpClipRect));
        BUMP(3);
	clipSelect = SSTG_CLIPSELECT;
    }

    //DPF("HW Stretch  Memory \r\n");
   
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
   

    // index can have the following values:
    // 000b color pattern, color src
    // 001b color pattern, mono src
    // 100b mono pattern, color src
    // 101b mono pattern, mono src

    index =  ((lpBrush->dp8BrushFlags & PATTERNMONO)  ||
	      (lpSrc->deBitsPixel & 1));

    if (lpSrc->deBitsPixel == 1) 
    {
	// Mono case will be byte packed.
	// No stride info needed
	// startOffset is byte ptr to first pix of blt
	lpbmp = (BITMAP *) lpSrc;

	startOffset = ( (lp->SrcY * lpbmp->bmWidthBytes) + 
			(lp->SrcX >> 3));

	dwtemp =  (((lp->srcWidth * lpSrc->deBitsPixel) - 1) >> 3) ;

	//INT_3
	if (RopUsesPat(lp->Rop >> 16))
	{
	    switch(index)
	    {
            
	      case ColPat_MonoSrc:
		  //DPF("case ColPat_MonoSrc:");
		  //INT_3
		  // color pattern reg is set in 
		  // packet header in HandleBrush
		  goto clean;

		  break;

	      case MonoPat_MonoSrc:
		  //DPF("case MonoPat_MonoSrc::");
		  //INT_3
		  goto clean;
		  //  have to manually expand mono brush. hw
		  //  can't handle mono brush, mono src.
		  break;
   
	    }   //switch(index)
	}
	else
	{
      
	    // only mono src, no pat.
	    //DPF( "// only mono src, no pat.");
	    //INT_3
	    //DPF("Mono Src Only");
   
	    // monp

	    // HandleFormat  fill a dword for srcFormat register,
	    // stride, pixel depth, packing.
   
	    // buffer is dest. Expand mono to display pix depth. 
	    bufFormatData = HandleFormat(lpDst);
	    bufFormatData = bufFormatData  &
		0xFFFF0000L;
	    bufFormatData = bufFormatData | 
		(lp->srcWidth * (lpDst->deBitsPixel>>3) );

	    srcScanY=lp->SrcY;  // src
	    dstScanY=lp->DstY;  // dst

	    deltaX = lp->srcHeight   ;
	    deltaY = lp->dstHeight   ;

	    twoDeltaX = lp->srcHeight + lp->srcHeight;
	    threeDeltaX = twoDeltaX + lp->srcHeight;
	    twoDeltaY = lp->dstHeight + lp->dstHeight;
	    d = threeDeltaX - twoDeltaY;

	    CMDFIFO_SAVE(cf);
	    LoadMonoBuffer(lpSrc, lp->SrcX, srcScanY,lp->srcWidth, 
			   lp->srcHeight, bufFormatData, lpDrawMode);
	    CMDFIFO_RELOAD(cf);
	    for ( i =0; i < deltaY; i++)
	    {
		CMDFIFO_SAVE(cf);
		StretchBltBufferToScreen(lpDst, lp->DstX, dstScanY, 
					 lp->dstWidth,
					 lp->srcWidth, 
					 bufFormatData, clipSelect);
		CMDFIFO_RELOAD(cf);

        // Only load more data if we're not on the last time through the loop
        if( (i+1)!=deltaY )
        {
            while( d >= 0)
            {
                srcScanY++;
                // Since we will never decimate, the buffer load
                // will be executed only once.  we will never
                // increment srcScanY more than once 
                CMDFIFO_SAVE(cf);
                LoadMonoBuffer(lpSrc, lp->SrcX, srcScanY,
            	   lp->srcWidth,
            	   lp->srcHeight, bufFormatData,
            	   lpDrawMode);
                CMDFIFO_RELOAD(cf);
                d -= twoDeltaY;
            }   
        }

        dstScanY++;
        d += twoDeltaX;
        }
    }
    }
    else    
	//   // color src
    {
	//INT_3
	// startOffset is byte ptr to first pix of blt

	startOffset = (lp->SrcY * lpSrc->deDeltaScan) ;
	dwtemp =   (lp->SrcX * (lpSrc->deBitsPixel>>3)) ;
	startOffset+=dwtemp;
   
	dwordsInScanlineG = (((((lp->srcWidth * lpSrc->deBitsPixel) - 1)
			       >> 3) +
			      (lp->SrcX & 3)) >> 2) + 1;

	leftAlignment = lp->SrcX & 3; 
	// stride in bytes of bitmap in bytes
	// stride will tell the hw how many bytes to skip at end.
	stride =  lp->srcWidth * (lpSrc->deBitsPixel >>3);
	if (RopUsesPat(lp->Rop >> 16))
	{
	    switch(index)
	    {
	      case ColPat_ColSrc:
		  //DPF(" case ColPat_ColSrc:");
		  //INT_3
		  goto clean;
		  break;
            
	      case MonoPat_ColSrc:
		  //DPF("case MonoPat_ColSrc:");
		  //INT_3
		  goto clean;
		  break;
   
	      default: INT_3;
	    }   //switch(index)
	}
	else
	{
	    //cnop
	    // only color src case: no pat.
	    //DPF(" > > > > > > > > > >");
	    //DPF("Stretch color src no pat");
	    //DPF(" > > > > > > > > > >");
	    //INT_3

	    //Strategy:  
	    //   hostblt scanline to buffer
	    //  use Bressingham alg to determine whether to  
	    //      repeat blting buffer to replicate in Y direction
	    //       or skip to next src scanline.
	    //   NO DECIMATION.  requires sending lots of data over
	    //      pci bus. Use Dibengine if srcWidth<dstWidth or
	    //      srcHeight<dstHeight
	    //   Similar to Bressingham line with slope < 1 and   
	    //      with only positive numbers. X in the line algorithm
	    //      analagous to srcY in stretchBlt,
	    //      y is analagous to dstY. slope is analagous to
	    //      vertical expansion ratio between src and dst,
	    //      i.e. srcHeight/dstHeight.  Don't confuse the Y's. 
	    //      We are expanding vertical pixels through replication.
	    //     dstY = (m)srcY + b;
	    //      We will allways be column major since we never 
	    //         decimate, ie srcHeight>srcWidth

	    //       The error term is initialized to 3srcHeight-2dstHeight
	    //      which is the second iteration of the error term
	    //      The first would be 2srcHeight-dstHeight.
         
	    // configure the buffer as source blt rect
       if (lp->dstWidth >= lp->srcWidth)
         {         
	      bufFormatData = HandleFormat(lpSrc);
	      bufFormatData = bufFormatData & 0xFFFF0000L;
   	   bufFormatData = bufFormatData | (lp->srcWidth * (lpSrc->deBitsPixel>>3) );

   	   srcScanY=lp->SrcY;  // src
	      dstScanY=lp->DstY;  // dst

	      deltaX = lp->srcHeight;
	      deltaY = lp->dstHeight;

   	   twoDeltaX = lp->srcHeight + lp->srcHeight;
	      threeDeltaX = twoDeltaX + lp->srcHeight;
	      twoDeltaY = lp->dstHeight + lp->dstHeight;
	      d = threeDeltaX - twoDeltaY;
	      CMDFIFO_SAVE(cf);
	      LoadColorBuffer(lpSrc, lp->SrcX, srcScanY, lp->srcWidth, lp->srcHeight, bufFormatData);
	      CMDFIFO_RELOAD(cf);
	      for ( i =0; i < deltaY; i++)
	         {
		      CMDFIFO_SAVE(cf);
		      StretchBltBufferToScreen(lpDst, lp->DstX, dstScanY,  lp->dstWidth, lp->srcWidth, bufFormatData, clipSelect);
		      CMDFIFO_RELOAD(cf);

            // Only load more data if we're not on the last time through the loop
            if( (i+1)!=deltaY )
               {
               while (d >= 0)
                  {
                  srcScanY++;
                  // Since we will never decimate, the buffer load
                  // will be executed only once.  we will never
                  // increment srcScanY more than once 
                  CMDFIFO_SAVE(cf);
                  LoadColorBuffer(lpSrc, lp->SrcX, srcScanY, lp->srcWidth, lp->srcHeight, bufFormatData);
                  CMDFIFO_RELOAD(cf);
                  d -= twoDeltaY;
                  }   
               }

		      dstScanY++;
		      d += twoDeltaX;
	         }
         }
      else
         {
	      bufFormatData = HandleFormat(lpSrc);
	      bufFormatData = bufFormatData & 0xFFFF0000L;
   	   bufFormatData = bufFormatData | (lp->dstWidth * (lpSrc->deBitsPixel>>3) );

   	   srcScanY=lp->SrcY;  // src
	      dstScanY=lp->DstY;  // dst

	      deltaX = lp->srcHeight;
	      deltaY = lp->dstHeight;

   	   twoDeltaX = lp->srcHeight + lp->srcHeight;
	      threeDeltaX = twoDeltaX + lp->srcHeight;
	      twoDeltaY = lp->dstHeight + lp->dstHeight;
	      d = threeDeltaX - twoDeltaY;
	      CMDFIFO_SAVE(cf);
	      LoadShrinkBuffer(lpSrc, lp->SrcX, srcScanY, lp->srcWidth, lp->dstWidth, lp->srcHeight, bufFormatData);
	      CMDFIFO_RELOAD(cf);
	      for ( i =0; i < deltaY; i++)
	         {
		      CMDFIFO_SAVE(cf);
		      BltBufferScreenToScreen(lpDst, lp->DstX, dstScanY,  lp->dstWidth, bufFormatData, clipSelect);
		      CMDFIFO_RELOAD(cf);

            // Only load more data if we're not on the last time through the loop
            if( (i+1)!=deltaY )
               {
               while (d >= 0)
                  {
                  srcScanY++;
                  // Since we will never decimate, the buffer load
                  // will be executed only once.  we will never
                  // increment srcScanY more than once 
                  CMDFIFO_SAVE(cf);
                  LoadShrinkBuffer(lpSrc, lp->SrcX, srcScanY, lp->srcWidth, lp->dstWidth, lp->srcHeight, bufFormatData);
                  CMDFIFO_RELOAD(cf);
                  d -= twoDeltaY;
                  }   
               }

		      dstScanY++;
		      d += twoDeltaX;
	         }
         }
	   }
    }

clean:

    // probably unnecessary, but I'll figure this out later
    // make sure we recompute src & dst if LoadColorBuffer LoadMonoBuffer
    // was the last call made.  If StretchBltBufferToScreen was the last
    // call made, we don't need to set these two gdiFlags bits
    //
    _FF(gdiFlags) |= (SDATA_GDIFLAGS_DST_WAS_DEVBIT |
		      SDATA_GDIFLAGS_SRC_WAS_DEVBIT);
    
    FXLEAVE("HWStretchBltMemory", cf, lpDst);
}


/*----------------------------------------------------------------------
Function name:  LoadColorBuffer

Description:    Moves a buffer worth of src data to the offscreen
                buffer use the blt engine.  (For color src data only!)  
Information:    

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
int
LoadColorBuffer(DIBENGINE FAR * lpSrc, 
		WORD          SrcX,
		WORD          SrcY, 
		WORD          srcWidth,
		WORD          srcHeight,
		DWORD         bufFormatData) 
{
    DWORD dwtemp;
    DWORD dstXYData;
    DWORD dstSizeData, srcSizeData;
    DWORD srcFormatData;
    DWORD dwordsInScanlineG;
    DWORD rightByteAddr;
    DWORD startOffset;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    CMDFIFO_SETUP(cf); 

    startOffset = (SrcY * lpSrc->deDeltaScan) ;
    dwtemp =   (SrcX * (lpSrc->deBitsPixel>>3)) ;
    startOffset+=dwtemp;
               
    // calc dword in scanline, use first line
	rightByteAddr = startOffset + (srcWidth * (lpSrc->deBitsPixel >> 3));

	dwordsInScanlineG =    ( ((rightByteAddr + 3) & 0xFFFFFFFC) -
				(startOffset & 0xFFFFFFFC) ) >> 2; 

    CMDFIFO_CHECKROOM(cf, 9);
    SETPH(cf,  SSTCP_PKT2|
	  dstBaseAddrBit|
	  dstFormatBit|
	  srcFormatBit|
	  srcSizeBit|
	  srcXYBit|
	  dstSizeBit|
	  dstXYBit|
	  commandBit); 

    // dst is offscreen buffer.
    // set dstBaseAddr to offscreen memory.
    SET(cf, _FF(lpGRegs)->dstBaseAddr, lpDriverData->stretchBltStart);

    SET(cf, _FF(lpGRegs)->dstFormat, bufFormatData);

    // src is host data   
    // HandleFormat  fill a dword for srcFormat register,
    // stride, pixel depth.
    srcFormatData = HandleFormat(lpSrc);
    SET(cf, _FF(lpGRegs)->srcFormat, srcFormatData);
   
    srcSizeData= ((((DWORD)1L) << 16) | ((DWORD)srcWidth)) ;
    SET(cf, _FF(lpGRegs)->srcSize , srcSizeData);
   
    SET(cf, _FF(lpGRegs)->srcXY , (startOffset & 3) );


    dstSizeData= ((((DWORD)1L) << 16) | ((DWORD)srcWidth)) ;
    SET(cf, _FF(lpGRegs)->dstSize ,dstSizeData );
   
   
    // always blt to the begin of buffer (0,0)
    dstXYData = ((((DWORD)0L)) ) | ((DWORD)0L) ;
    SET(cf, _FF(lpGRegs)->dstXY , dstXYData);
    SETC(cf, _FF(lpGRegs)->command, SSTG_ROP_SRCCOPY | SSTG_HOST_BLT);
    BUMP(9);
    startOffset &= 0xFFFFFFFC;

    CMDFIFO_SAVE(cf);
    WriteColorSrc(lpSrc, startOffset, 1, dwordsInScanlineG);
    CMDFIFO_RELOAD(cf);

    CMDFIFO_EPILOG(cf);

    return 1;
}


/*----------------------------------------------------------------------
Function name:  LoadMonoBuffer

Description:    Moves one scanlines worth of data to buffer so that 
                offscreen to display stretch blt can be used to
                stretch the hostbitmap.
Information:    

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
int
LoadMonoBuffer(DIBENGINE FAR * Src, 
               WORD          SrcX,
               WORD          SrcY, 
               WORD          srcWidth,
               WORD         srcHeight,
               DWORD         bufFormatData,
               DRAWMODE FAR *   lpDrawMode) 
{
    DWORD dstXYData;
    DWORD dstSizeData, srcSizeData;
    DWORD srcFormatData;
    DWORD dwordsInScanlineG;
    DWORD startOffset;
    BITMAP * lpbmp;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    CMDFIFO_SETUP(cf); 

    lpbmp = (BITMAP *) Src;
    startOffset = ( (SrcY * lpbmp->bmWidthBytes) + ((SrcX & 0xFFE0) >> 3));
    dwordsInScanlineG = (((srcWidth - 1) + (SrcX & 0x1F)) >> 5) + 1;

    CMDFIFO_CHECKROOM(cf, 11);
    SETPH(cf,  SSTCP_PKT2|
	  dstBaseAddrBit|
	  dstFormatBit|
	  srcFormatBit|
	  srcSizeBit|
	  srcXYBit|
	  colorBackBit|
	  colorForeBit|
	  dstSizeBit|
	  dstXYBit|
	  commandBit); 

    // dst is offscreen buffer.
    // set dstBaseAddr to offscreen memory.
    SET(cf, _FF(lpGRegs)->dstBaseAddr, lpDriverData->stretchBltStart);
    SET(cf,_FF(lpGRegs)->dstFormat, bufFormatData);

    // src is host data   
    // HandleFormat  fill a dword for srcFormat register,
    // stride, pixel depth.

    srcFormatData = HandleFormat(Src);
    SET(cf, _FF(lpGRegs)->srcFormat, srcFormatData);
   
    srcSizeData= ((((DWORD)1L) << 16) | ((DWORD)srcWidth)) ;
    SET(cf, _FF(lpGRegs)->srcSize , srcSizeData);
   
    SET(cf, _FF(lpGRegs)->srcXY , SrcX & 0x1F);

    SET(cf, _FF(lpGRegs)->colorBack, lpDrawMode->TextColor);
    SET(cf, _FF(lpGRegs)->colorFore, lpDrawMode->bkColor);

    dstSizeData= ((((DWORD)1L) << 16) | ((DWORD)srcWidth)) ;
    SET(cf, _FF(lpGRegs)->dstSize ,dstSizeData );
   
    // always blt to the begin of buffer (0,0)
    dstXYData = ((((DWORD)0L)) ) | ((DWORD)0L) ;
    SET(cf, _FF(lpGRegs)->dstXY , dstXYData);
   
    SETC(cf, _FF(lpGRegs)->command,
	(SSTG_ROP_SRC << SSTG_ROP0_SHIFT)|
	SSTG_HOST_BLT);
    BUMP(  11);

   // this overcomplicates the X start address
//    startOffset &=   0xFFFFFFFC;

    CMDFIFO_SAVE(cf);
    WriteMonoSrc((BITMAP *)Src, startOffset, 1, dwordsInScanlineG);
    CMDFIFO_RELOAD(cf);

    CMDFIFO_EPILOG(cf);

    return 1;
}

FxU32 CurrentDstAddr;
FxU32 CurrentDstFormat;


/*----------------------------------------------------------------------
Function name:  StretchBltBufferToScreen

Description:    Stretch blts the buffer to the screen.

Information:    
    Assumptions:
        Data in buffer is same format as display.
        Buffer contains one scanline
            srcHeight=dstHeight=1
            srcX=srcY=0

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
// the #defines below fixes PRS 12524

#define SSTCP_PKT4_NOPCMD  ((0x48 << SSTCP_REGBASE_SHIFT))
#define CMDFIFO_BUILD_PK4( mask, regName, chip )\
    ( ((mask) << SSTCP_PKT4_MASK_SHIFT) | (chip << 11)\
    | (regName) | SSTCP_PKT4 )

int 
StretchBltBufferToScreen(LPDIBENGINE lpDst,
			 WORD DstX, 
			 WORD DstScanY, 
			 WORD dstWidth,
			 WORD srcWidth,
			 DWORD bufFormatData,
			 DWORD clipSelect)
{
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    CMDFIFO_SETUP(cf); 

    CMDFIFO_CHECKROOM(cf, 12);

    SETPH(cf, (SSTCP_PKT2 | dstBaseAddrBit | dstFormatBit | srcBaseAddrBit |
	       srcFormatBit | srcSizeBit |
	       srcXYBit | dstSizeBit | dstXYBit | commandBit));
    SET(cf, _FF(lpGRegs)->dstBaseAddr, CurrentDstAddr);
    SET(cf, _FF(lpGRegs)->dstFormat, CurrentDstFormat);
    SET(cf, _FF(lpGRegs)->srcBaseAddr, lpDriverData->stretchBltStart);
    SET(cf, _FF(lpGRegs)->srcFormat, bufFormatData);
    SET(cf, _FF(lpGRegs)->srcSize, R32(1L, srcWidth));
    SET(cf, _FF(lpGRegs)->srcXY, 0L);    // beginning of buffer   
    SET(cf, _FF(lpGRegs)->dstSize, R32(1L, dstWidth));
    SET(cf, _FF(lpGRegs)->dstXY, R32(DstScanY, DstX));
    SETC(cf, _FF(lpGRegs)->command, (SSTG_ROP_SRCCOPY | clipSelect | SSTG_GO |
				    SSTG_STRETCH_BLT));

    // insert 3D nop to fix PRS 12524

    SETPH(cf, CMDFIFO_BUILD_PK4(1, SSTCP_PKT4_NOPCMD, 0));
    SET(cf, ghw3D->nopCMD, 0);
    BUMP(12);

    CMDFIFO_EPILOG(cf);

    return 1;
}

/*----------------------------------------------------------------------
Function name:  LoadShrinkBuffer

Description:    Moves a buffer worth of src data to the offscreen
                buffer use the blt engine.  (For color src data only!)  
Information:    

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
int LoadShrinkBuffer(DIBENGINE FAR * lpSrc, WORD SrcX, WORD SrcY, WORD srcWidth,	WORD dstWidth, WORD srcHeight, DWORD bufFormatData) 
{
    DWORD dwtemp;
    DWORD dstXYData;
    DWORD dstSizeData, srcSizeData;
    DWORD dwordsInScanlineG;
    DWORD startOffset;
    BYTE * pInBuffer;
    BYTE * pOutBuffer;
    WORD i;
    DWORD xWhole;
    DWORD xFrac;
    DWORD ff;
    DWORD inPixel;
    DWORD nBPP;
    DWORD xSize;
    DWORD pixels;
    DWORD lpTmp16;           // tmp 16:16 pointer
    DWORD dwFP1616;
    WORD wholeInc;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    CMDFIFO_SETUP(cf); 

    startOffset = (SrcY * lpSrc->deDeltaScan);
    dwtemp =   (SrcX * (lpSrc->deBitsPixel>>3)) ;
    startOffset+=dwtemp;
               
    // calc dword in scanline, use first line
	 dwordsInScanlineG = (((dstWidth * (lpSrc->deBitsPixel >> 3)) + 3) & 0xFFFFFFFC) >> 2; 

    CMDFIFO_CHECKROOM(cf, 9);
    SETPH(cf,  SSTCP_PKT2|
	  dstBaseAddrBit|
	  dstFormatBit|
	  srcFormatBit|
	  srcSizeBit|
	  srcXYBit|
	  dstSizeBit|
	  dstXYBit|
	  commandBit); 

    // dst is offscreen buffer.
    // set dstBaseAddr to offscreen memory.
    SET(cf, _FF(lpGRegs)->dstBaseAddr, lpDriverData->stretchBltStart);

    SET(cf, _FF(lpGRegs)->dstFormat, bufFormatData);

    SET(cf, _FF(lpGRegs)->srcFormat, bufFormatData);
   
    srcSizeData= ((((DWORD)1L) << 16) | ((DWORD)dstWidth)) ;
    SET(cf, _FF(lpGRegs)->srcSize , srcSizeData);
   
    SET(cf, _FF(lpGRegs)->srcXY, 0x0);

    dstSizeData= ((((DWORD)1L) << 16) | ((DWORD)dstWidth)) ;
    SET(cf, _FF(lpGRegs)->dstSize , dstSizeData);
   
    // always blt to the begin of buffer (0,0)
    dstXYData = ((((DWORD)0L)) ) | ((DWORD)0L) ;
    SET(cf, _FF(lpGRegs)->dstXY , dstXYData);
    SETC(cf, _FF(lpGRegs)->command, SSTG_ROP_SRCCOPY | SSTG_HOST_BLT);
    BUMP(9);

    xWhole = srcWidth/dstWidth;
    xFrac = srcWidth%dstWidth;
   
    ff = 0x0;
    inPixel = 0;
    nBPP = lpSrc->deBitsPixel >> 3;
    xSize = 0;
    wholeInc = (WORD)(xWhole * nBPP);

    CMDFIFO_CHECKROOM(cf, dwordsInScanlineG+1);
    SETPH(cf, (SSTCP_PKT1 | SSTCP_PKT1_2D | (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) | ((dwordsInScanlineG) << SSTCP_PKT1_NWORDS_SHIFT)));

    pixels = 0x0;
    inPixel = 0x0;
    // Simple Decimation
    lpTmp16= ((DWORD)(lpSrc->deBitsSelector)) << 16;
    FarToFlat(lpTmp16, pInBuffer)
	 (DWORD)pInBuffer = (DWORD)(pInBuffer) + lpSrc->deBitsOffset + startOffset;
    pOutBuffer = (BYTE *)&pixels;
    STACKFP(pOutBuffer, dwFP1616)
    FarToFlat(dwFP1616, pOutBuffer);
    for (i=0; i<dstWidth; i++)
      {
      if (4 == nBPP)
         pixels = (DWORD)(*(DWORD *)&pInBuffer[inPixel]);
      else if (2 == nBPP)
         (*(WORD *)&pOutBuffer[xSize]) = (WORD)(*(WORD *)&pInBuffer[inPixel]);
      else 
         pOutBuffer[xSize] = pInBuffer[inPixel];

      xSize += nBPP;
      if (4 == xSize)
         {
         // do write
         SET(cf, _FF(lpGRegs)->launch[0], pixels);
         xSize = 0;
         }

      ff += xFrac;
      if (ff >= dstWidth)
         {
         ff -= dstWidth;
         inPixel += nBPP;
         }
      inPixel += wholeInc;     
      }
   
    // Leftovers???
    if (xSize)
      SET(cf, _FF(lpGRegs)->launch[0], pixels);

    BUMP(dwordsInScanlineG + 1);
    CMDFIFO_EPILOG(cf);

    return 1;
}
/*----------------------------------------------------------------------
Function name:  BltBufferScreenToScreen

Description:    Screen to Screen blit of shrinked buffer.

Information:    
    Assumptions:
        Data in buffer is same format as display.
        Buffer contains one scanline
            srcHeight=dstHeight=1
            srcX=srcY=0

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
int BltBufferScreenToScreen(LPDIBENGINE lpDst, WORD DstX, WORD DstScanY, WORD dstWidth, DWORD bufFormatData, DWORD clipSelect)
{
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    CMDFIFO_SETUP(cf); 

    CMDFIFO_CHECKROOM(cf, 12);

    SETPH(cf, (SSTCP_PKT2 | dstBaseAddrBit | dstFormatBit | srcBaseAddrBit |
	       srcFormatBit | srcSizeBit |
	       srcXYBit | dstSizeBit | dstXYBit | commandBit));
    SET(cf, _FF(lpGRegs)->dstBaseAddr, CurrentDstAddr);
    SET(cf, _FF(lpGRegs)->dstFormat, CurrentDstFormat);
    SET(cf, _FF(lpGRegs)->srcBaseAddr, lpDriverData->stretchBltStart);
    SET(cf, _FF(lpGRegs)->srcFormat, bufFormatData);
    SET(cf, _FF(lpGRegs)->srcSize, R32(1L, dstWidth));
    SET(cf, _FF(lpGRegs)->srcXY, 0L);    // beginning of buffer   
    SET(cf, _FF(lpGRegs)->dstSize, R32(1L, dstWidth));
    SET(cf, _FF(lpGRegs)->dstXY, R32(DstScanY, DstX));
    SETC(cf, _FF(lpGRegs)->command, (SSTG_ROP_SRCCOPY | clipSelect | SSTG_GO | SSTG_BLT));

    // insert 3D nop to fix PRS 12524
    SETPH(cf, CMDFIFO_BUILD_PK4(1, SSTCP_PKT4_NOPCMD, 0));
    SET(cf, ghw3D->nopCMD, 0);
    BUMP(12);

    CMDFIFO_EPILOG(cf);

    return 1;
}
