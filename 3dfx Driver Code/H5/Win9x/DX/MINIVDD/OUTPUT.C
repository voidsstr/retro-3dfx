/* -*-c++-*- */
/* $Header: output.c, 2, 10/11/00 8:54:48 PM, Brent$ */
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
** File name:   output.c
**
** Description: 32-bit side of the GDI output functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:48 PM$
**
** $Revision: 2$
** $Date: 10/11/00 8:54:48 PM$
**
** $History: output.c $
** 
** *****************  Version 38  *****************
** User: Michael      Date: 1/11/99    Time: 3:54p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 37  *****************
** User: Andrew       Date: 8/01/98    Time: 2:57p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added some code to hwOsScanline to use the Pen if there is no brush.
** 
** *****************  Version 36  *****************
** User: Michael      Date: 6/12/98    Time: 4:23p
** Updated in $/devel/h3/Win95/dx/minivdd
** HandleBrush was getting called with rop incorrectly positioned.  It
** needed to be right shifted one byte to get correcly handled.  Added fix
** and removed unneeded support code.  Fixes #1881.
** 
** *****************  Version 35  *****************
** User: Andrew       Date: 6/11/98    Time: 11:56a
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed the rectangle in FXENTER in hwOsRect to use blit coordinates
** 
** *****************  Version 34  *****************
** User: Michael      Date: 6/01/98    Time: 10:30a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fred W's (igx) fix for dithering at 8-bpp with solid brush.  Fixes
** #1671 and 1678.
** 
** *****************  Version 33  *****************
** User: Andrew       Date: 6/01/98    Time: 6:50a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added clip rectangles to some Polyscan & Polyline functions
** 
** *****************  Version 32  *****************
** User: Michael      Date: 5/15/98    Time: 4:31p
** Updated in $/devel/h3/Win95/dx/minivdd
** Backed out my previous changes.
** 
** *****************  Version 31  *****************
** User: Michael      Date: 5/15/98    Time: 9:28a
** Updated in $/devel/h3/Win95/dx/minivdd
** Add lpDst to FXPUNT and FXLEAVE_NOHWACCESS.
** 
** *****************  Version 30  *****************
** User: Ken          Date: 5/14/98    Time: 8:31a
** Updated in $/devel/h3/win95/dx/minivdd
** added a PERFNOP case to gdi32.   change the "undef" to a "define"
** in fifomgr.h, and enable in softice (doperfnop = 1).  Replaces every
** write
** to the command register with a "0"
** 
** *****************  Version 29  *****************
** User: Andrew       Date: 5/04/98    Time: 10:59p
** Updated in $/devel/h3/Win95/dx/minivdd
** Removed fixed as Michael was better
** 
** *****************  Version 27  *****************
** User: Michael      Date: 4/29/98    Time: 12:26p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix bug in hwOsRect of nothing being drawn when rectangle collapses
** down to a single line.  Bug seen in Coral draw.
** 
** *****************  Version 26  *****************
** User: Ken          Date: 4/27/98    Time: 5:35p
** Updated in $/devel/h3/win95/dx/minivdd
** complete (well, almost) enter/leave implementation for gdi32
** 
** *****************  Version 25  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#include "h3.h"
#include "thunk32.h"
#include "output.h"
#include "h3g.h"
#include "entrleav.h"

int dbgrect        = 0;
int dbgscanline    = 1;
int dbgbeginscan   = 1;
int dbgpolyline    = 0;
int dbgwindpolygon = 0;
int dbgaltpolygon  = 0;

//   **********************************************************************   
/*  
/*   
   << THIS TABLE, rop2ConvertTable, is FOR LINES ONLY >>
   The rop2ConvertTable converts the Rop2 in Drawmode to rop3.
   that use src argument instead of pattern arguement.
   Rop2 operate on dest and pattern.  However the hardware feeds the
   stipple patterns as source input into the rops.
*/
//   **********************************************************************   
DWORD rop2ConvertTable[17] = {   0x00000000,      // filler
                        0x00000000,      // R2_BLACK
                        0x11000000,      // notmergepen   DSon
                        0x22000000,      // masknotpen    DSna   
                        0x33000000,      // notcopypen    Sn
                        0x44000000,      // maskpennot    SDna
                        0x55000000,      // not           Dn
                        0x66000000,      // xorpen        DSx
                        0x77000000,      // notmaskpen    DSan
                        0x88000000,      // maskpen       DSa
                        0x99000000,      // notxorpen     DSxn
                        0xAA000000,      // nop           D
                        0xBB000000,      // mergenotpen   DSno
                        0xCC000000,      // copy pen      S
                        0xDD000000,      // mergepennot   SDno
                        0xEE000000,      // mergepen      DSo
                        0xFF000000       // white         1
                        };
   
//   **********************************************************************   
/*
   Hardware doesn't decode Rop2. 
   rop2ToRop3Table translates a rop2 to and equivalent 
   rop3 to use with hardware.
*/
//   **********************************************************************   
//                                                 rop 2       rop3 equiv
DWORD rop2ToRop3[17] =       {   0x00000000,      // filler      
                        0x00000000,      // R2_BLACK
                        0x05000000,      // notmergepen   DPon
                        0x0A000000,      // masknotpen    DPna   
                        0x0F000000,      // notcopypen    Pn
                        0x50000000,      // maskpennot    PDna
                        0x55000000,      // not           Dn
                        0x5A000000,      // xorpen        DPx
                        0x5F000000,      // notmaskpen    DPan
                        0xA0000000,      // maskpen       DPa
                        0xA5000000,      // notxorpen     DPxn
                        0xAA000000,      // nop           D
                        0xAF000000,      // mergenotpen   DPno
                        0xF0000000,      // copy pen      P 
                        0xF5000000,      // mergepennot   PDno
                        0xFA000000,      // mergepen      DPo
                        0xFF000000       // white         1
                        };

// cde 021398
//
// To reduce the amount of code needed to process pen styles, this
// array contains the lineStyle, lineStipple, and cmdData values
// needed for each style. We can then look up the values based on
// the given pen style instead of calculating the values each time.
// Note, we use the GDIDEFS.H MaxLineStyle, but we add one because
// the styles start with 0!

PEN_STIPPLE_INFO PenConvert[ MaxLineStyle+1 ] = 
{
    { 0, 0, 0 },                                    // LS_SOLID
    { DASH,       DASH_STYLE,       CMD_STIPPLE },  // LS_DASHED
    { DOT,        DOT_STYLE,        CMD_STIPPLE },  // LS_DOTTED
    { DOTDASH,    DOTDASH_STYLE,    CMD_STIPPLE },  // LS_DOTDASHED
    { DASHDOTDOT, DASHDOTDOT_STYLE, CMD_STIPPLE },  // LS_DASHDOTDOT
    { 0, 0, 0 }                                     // LS_NOLINE
};

#define SETUP_STIPPLE_FROM_PEN(penStyle,lineStipple,lineStyle,cmd) \
    lineStipple = PenConvert[ penStyle ].dwLineStipple;            \
    lineStyle   = PenConvert[ penStyle ].dwLineStyle;              \
    cmd        |= PenConvert[ penStyle ].dwCmdData;


//
// tells the line drawing function hwOsPolyLine whether or not it's being
// called from the top levtl FxOutput() or from a polygon drawing function
//
FxU32 DrawingPolygonBorder = 0;

int FindBoundRect(LPRECT lpRect, LPPOINT lpPoints, int nVerts);
int FindBoundRect2(LPRECT lpRect, LPPOINT lpPoints, int nVerts);
int FindBoundRect3(LPRECT lpRect, LPPOINT lpPoints, int nVerts);


/*----------------------------------------------------------------------
Function name:  FxOutput

Description:    32-bit side of the GDI output function.

Information:    

Return:         LONG    Result of the operation if handled or
                        0 if not handled.
----------------------------------------------------------------------*/
LONG
FxOutput(OutputParams* pParams)
//      DIBENGINE FAR * lpDestDev, 
//      int wStyle, 
//      int wCount, 
//      POINT FAR * lpDstPoints, 
//      DIB_Pen FAR * lppPen, 
//      DIB_Brush8 FAR * lpPBrush, 
//      DRAWMODE FAR * lpDrawMode, 
//      RECT FAR * lpClipRect)
{
    LONG retval;
    LPDIBENGINE lpDst;
    LPRECT      lpClipRect;
    DEBUG_FIX;

    FarToFlat( pParams->lpDestDev, lpDst );
    lpClipRect=NULL;
    retval=0;

    if ( pParams->lpClipRect )
    {
	FarToFlat( pParams->lpClipRect, lpClipRect );
    }

    switch( pParams->wStyle ) 
    {
      case OS_BEGINNSCAN:
	  // need to make sure put in state code for not
	  // reloading  brush or pen for scanlines.
	  // until next endscan
	  // Guide says I can punt.
	  // but faster if I implement.
	  // put in with brush cache.
	  //
	  FXPUNT(FXPUNT_NORMAL);
	  break;
	  
      case OS_SCANLINES:
	  retval = hwOsScanline( pParams, lpDst, lpClipRect);
	  break;
          
      case OS_ENDNSCAN:
	  FXPUNT(FXPUNT_NORMAL);
	  break;
    
      case OS_POLYLINE:
	  retval = hwOsPolyLine( pParams, lpDst, lpClipRect);
	  break;
   
      case OS_RECTANGLE:
	  retval = hwOsRect( pParams, lpDst, lpClipRect);
	  break;
    
      case OS_POLYSCANLINE:
	  // pain in the ass scan structure.
	  // SCAN is variable length
	  // xcnPntX[] is variable length.
	  // scnPntCnt is count of x coordinates NOT PAIRS
	  // wCount is count for number of SCAN STRUCTS.
	  //
	  retval = hwOsPolyScanline(pParams, lpDst, lpClipRect);
	  break;
   
      case OS_ALTPOLYGON:
	  DrawingPolygonBorder = TRUE;
	  retval = hwOsAltPolygon(pParams, lpDst, lpClipRect);
	  DrawingPolygonBorder = FALSE;
	  break;
        
      case OS_WINDPOLYGON:
	  FXPUNT(FXPUNT_NORMAL);
	  break;
   
      default:
	  FXPUNT(FXPUNT_NORMAL);
	  break;
    }

    FXLEAVE_NOHWACCESS("FxOutput", 1);
}


/*----------------------------------------------------------------------
Function name:  hwOsPolyLine

Description:    Use hardware to perform the GDI operation.

Information:    If the points determine a polygon, it does not
                fill the polygon.

Return:         LONG    Result of the operation if handled or
                        0 if not handled.
----------------------------------------------------------------------*/
LONG 
hwOsPolyLine(OutputParams* pParams,
	     LPDIBENGINE   lpDst,
	     LPRECT        lpClipRect)
//      DIBENGINE FAR * lpDst, 
//      int wCount, 
//      POINT FAR * lpDstPoints, 
//      DIB_Pen FAR * lppPen, 
//      DIB_Brush8 FAR * lpPBrush, 
//      DRAWMODE FAR * lpDrawMode,
//      RECT FAR * lpClipRect) 
{
    RECT             OurRect;
    LPDRAWMODE lpDrawMode;
    LPPOINT    lpDstPoints; 
    LPpen      lpPPen;
    LPRECT     lpOurRect; 
    DWORD      dwFP1616;
    DWORD      cmdData;
    DWORD      lineStippleData, lineStyleData;
    DWORD      topX, topY, bottomX, bottomY;
    int i;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;
    
    FarToFlat( pParams->lpDrawMode, lpDrawMode );
    FarToFlat( pParams->lpPoints, lpDstPoints);

    cmdData = 0;
    lineStippleData = 0;
    lineStyleData = 0;

    // hwOsPolyLine can be called in one of two ways -- the regular way,
    // a result of Output(OS_POLYLINE...), or as a helper function to
    // draw the border of a polygon at the end of a Output(OS_POLYGON...)
    // call
    // 
    if (DrawingPolygonBorder)
    {
	// no need to check busy, exclude the cursor, etc., that's already
	// been done in the main polygon routine
	//
	CMDFIFO_SETUP(cf);
    }
    else
    {
	// do all the regular main drawing function chores
	//
	// XXX FIXME
	// TBD, calculate proper s/w exclusion bounding box 
	//
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR)
        {
        lpOurRect = &OurRect;
        STACKFP(lpOurRect, dwFP1616)
        FarToFlat(dwFP1616, lpOurRect);
        FindBoundRect(lpOurRect, lpDstPoints, pParams->wCount);
        }

	FXENTER("hwOsPolyLine", lpDst->deFlags, (hwAll & hwPolyline),
		cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
		FXENTER_RECT, OurRect.left, OurRect.top, OurRect.right, OurRect.bottom,
		FXENTER_NO_RECT, 0, 0, 0, 0,   );
    }
    
    // handle clip rect
    if ( lpClipRect )
    {
	topX = lpClipRect->left;
	topY = lpClipRect->top;
	bottomX = lpClipRect->right;
	bottomY = lpClipRect->bottom;

	CMDFIFO_CHECKROOM(cf, 3);
	SETPH(cf, SSTCP_PKT2 | clip1minBit | clip1maxBit);
	SET(cf, _FF(lpGRegs)->clip1min, R32(topY, topX));
	SET(cf, _FF(lpGRegs)->clip1max, R32(bottomY, bottomX));
	BUMP(3);
   
	cmdData |= SSTG_CLIPSELECT;
    }

    if (lpDrawMode->bkMode & BKMODE_TRANSPARENT)
    {
	cmdData |= SSTG_TRANSPARENT;
    }

    cmdData |= ( rop2ConvertTable[(lpDrawMode->Rop2)] );
    // set stipple regs
    /* The repeat count field of the lineStyle Reg holds n-1 count.
       The pattern from the lineStipple is reproduced from lsb to
       msb with lsb being bit 0.      
       */
      
    if ( pParams->lpPPen)
    {
	// LS_XXXX are values not flags.
	FarToFlat( pParams->lpPPen, lpPPen );

	//cde 030998
	//Changes for polyline style fixes (see above)
	//including using macro for pattern setup

	if ( lpPPen->dpPenStyle == LS_NOLINE ) 
	{
	    goto line_exit;
	}    

	SETUP_STIPPLE_FROM_PEN( lpPPen->dpPenStyle, lineStippleData, 
				lineStyleData, cmdData );

	if ( !( (lpPPen->dpPenStyle == LS_SOLID) ||
	       (lpPPen->dpPenStyle == LS_INSIDEFRAME)) ) 
	{
	    CMDFIFO_CHECKROOM(cf, 3);
	    SETPH(cf,   SSTCP_PKT2 
		  | lineStippleBit
		  | lineStyleBit
		);

	    SET(cf, _FF(lpGRegs)->lineStipple, lineStippleData);
	    SET(cf, _FF(lpGRegs)->lineStyle, lineStyleData);
	    BUMP( 3 );
	}

    }  // if (lpPPen)

    if (pParams->wCount < 3)
    {
	// single line
	//
	cmdData |= SSTG_POLYLINE | SSTG_GO;    //Don't draw endpoint!
	CMDFIFO_CHECKROOM(cf, 6);
	SETPH(cf,   SSTCP_PKT2      
	      |srcXYBit
	      |colorBackBit
	      |colorForeBit
	      |dstXYBit
	      |commandBit);
	SET(cf, _FF(lpGRegs)->srcXY, ( (DWORD) lpDstPoints[0].y)  << 16|
	    ( (WORD) lpDstPoints[0].x) );

	SET(cf, _FF(lpGRegs)->colorBack, lpDrawMode->bkColor);
	SET(cf, _FF(lpGRegs)->colorFore, lpPPen->dpPenColor);
	SET(cf, _FF(lpGRegs)->dstXY, ( (DWORD) lpDstPoints[1].y)  << 16|
	    ( (WORD) lpDstPoints[1].x) );
	SETC(cf, _FF(lpGRegs)->command, cmdData );
	BUMP( 6 );
    }
    else
    {
	// use launch
	cmdData |= SSTG_POLYLINE;

	CMDFIFO_CHECKROOM(cf, 5);
	SETPH(cf,   SSTCP_PKT2|      
	      srcXYBit|
	      colorBackBit|
	      colorForeBit|
	      commandBit);

	SET(cf, _FF(lpGRegs)->srcXY, ( (DWORD) lpDstPoints[0].y  << 16) |
	    ( (WORD) lpDstPoints[0].x) );

	SET(cf, _FF(lpGRegs)->colorBack, lpDrawMode->bkColor);
	SET(cf, _FF(lpGRegs)->colorFore, lpPPen->dpPenColor);
	SETC(cf, _FF(lpGRegs)->command, cmdData );
	BUMP( 5 );
   
	// sync indices for points of the polyline
	(DWORD) lpDstPoints=(DWORD)lpDstPoints + 4;

	CMDFIFO_CHECKROOM(cf,  pParams->wCount );
	SETPH(cf, SSTCP_PKT1
	      | SSTCP_PKT1_2D
	      | LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT
	      | ((DWORD)(pParams->wCount-1)) << SSTCP_PKT1_NWORDS_SHIFT 
            );

	for( i=0; i < pParams->wCount - 1; i++)
	{
	    SET(cf, _FF(lpGRegs)->launch[0], 
		( (DWORD) lpDstPoints[i].y)  << 16|
		( (WORD) lpDstPoints[i].x) );
	}
	BUMP( pParams-> wCount );
    }

line_exit:
    if (DrawingPolygonBorder)
    {
	CMDFIFO_EPILOG(cf);
	return 1;
    }

    FXLEAVE("hwOsPolyLine", cf, lpDst);
}


/*----------------------------------------------------------------------
Function name:  hwOsRect

Description:    Use hardware to perform the GDI operation.

Information:    The first point is upper left, second point is
                lower right.  hwOsRect fills the rectangle using
                the specified brush and draws the border using
                the specified pen.

                Solid border are special cased to use rectfill
                because the h3/banshee's line engine draws pixel
                per clock.

Return:         LONG
----------------------------------------------------------------------*/
LONG 
hwOsRect(OutputParams* pParams,
	 LPDIBENGINE   lpDst,
	 LPRECT        lpClipRect)
    //DIBENGINE FAR * lpDst, 
    //int wCount, 
    //POINT FAR * lpDstPoints, 
    //DIB_Pen FAR * lppPen, 
    //DIB_Brush8 FAR * lpPBrush, 
    //DRAWMODE FAR * lpDrawMode,
    //RECT FAR * lpClipRect) 

{
    LPDRAWMODE lpDrawMode;
    LPPOINT    lpDstPoints; 
    DIB_Brush8  *  lpPBrush;
    LPpen      lpPPen;
    DWORD       cmdData;
    DWORD      lineStippleData, lineStyleData;
    short      top, left, right, bottom;
    short      cliptop, clipleft, clipright, clipbottom;
    WORD  cmdBump;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    cmdData = 0;
    lineStippleData = 0;
    lineStyleData = 0;

    FarToFlat( pParams->lpDrawMode, lpDrawMode );
    FarToFlat( pParams->lpPoints, lpDstPoints);

    /* ************************************************************ 
       GDI will hand the driver negative xy coordinates that need to
       be clipped to the clip rect or the screen.  You have to clip to
       the screen also because GDI will also give you negative clip
       coordinates.  The driver must also check for null intersections.
       and return -1 (0xffff) in the cases where the clip rect is zero
       area (i.e. lpClipRect points to (0,0,0,0) rect) or the clipped 
       lpdstPoints is zero area.
       ************************************************************ */

    // Get rectangle coordinates
    left   = lpDstPoints[0].x;
    top    = lpDstPoints[0].y;
    right  = lpDstPoints[1].x - 1;   //right edge is exclusive
    bottom = lpDstPoints[1].y - 1;   //bottom edge is exclusive

    // Reject bad rectangle
    if ((bottom-top < 0) || (right-left < 0))
    {
      goto ret_bad_rect;
    }

    // Handle clipping rectangle
    if (lpClipRect)
    {
	clipleft   = lpClipRect->left;
	cliptop    = lpClipRect->top;
	clipright  = lpClipRect->right;
	clipbottom = lpClipRect->bottom;

	// Reject if clipping rectangle is bad
	if (((clipright-clipleft) <= 0) || (clipbottom-cliptop <= 0))
	{
	    goto ret_bad_rect;
	}

	// Banshee only handles 13-bits for dstSize and dstXY
	// handle by clamping to that range here
	if (left < -4096)
	{
	    left = -4096;
	}

	if (top < -4096)
	{
	    top = -4096;
	}

	if (right > 4095)
	{
	    right = 4095;
	}

	if (bottom > 4095)
	{
	    bottom = 4095;
	}

	// Clip against screen
	if ((short) lpClipRect->left < 0)
	{
	    clipleft = 0;
	}

	if ((short)lpClipRect->top < 0)
	{
	    cliptop = 0;
	}

	if ((short)lpClipRect->right > lpDriverData->hres)
	{
	    clipright = lpDriverData->hres;
	}

	if ((short)lpClipRect->bottom > lpDriverData->vres)
	{
	    clipbottom = lpDriverData->vres;
	}

	// If rectangle lies completely outside clipping boundaries 
	// then we're done
	if ((right < clipleft) ||     // rectangle is right of clip
	    (left > clipright) ||     // rectangle is left of clip
	    (top > clipbottom) ||     // rectangle is below clip
	    (bottom < cliptop))       // rectangle is above of clip
	{
	    goto ret_from_rect;
	}
    }

    FXENTER("hwOsRect", lpDst->deFlags, (hwAll & hwRect),
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
	    FXENTER_RECT, left, top, right, bottom,
	    FXENTER_NO_RECT, 0, 0, 0, 0,   );
    
    if (lpClipRect)
    {
	//Setup HW clip1 rectangle
	CMDFIFO_CHECKROOM(cf, 3);
	SETPH(cf, SSTCP_PKT2  |
	      clip1minBit |
	      clip1maxBit);
	SET(cf, _FF(lpGRegs)->clip1min,
            (cliptop << 16) | (WORD)clipleft);
	SET(cf, _FF(lpGRegs)->clip1max,
            (clipbottom << 16) | (WORD)clipright);
	BUMP( 3 );
   
	cmdData |= SSTG_CLIPSELECT;
    }

    // If a pointer to a brush exists, fill the interior
    if (pParams->lpPBrush)
    {
	FarToFlat( pParams->lpPBrush, lpPBrush);

	// No interior fill for NULL brush
	if (lpPBrush->dp8BrushStyle == BS_NULL)
	{
	    goto DrawBorderOnly;
	}

	// Do brush handling
	CMDFIFO_SAVE(cf);
	cmdData |= (DWORD)HandleBrush(lpPBrush, 
				      (rop2ToRop3[lpDrawMode->Rop2] >> 8),
				      lpDrawMode);   
	CMDFIFO_RELOAD(cf);

	// Do the interior fill
	CMDFIFO_CHECKROOM(cf, 4);
	SETPH(cf, SSTCP_PKT2 |
	      dstSizeBit |
	      dstXYBit |
	      commandBit);
   
	SET(cf, _FF(lpGRegs)->dstSize , 
	    ((DWORD)(bottom-top) << 16) | (WORD)(right-left));
   
	SET(cf, _FF(lpGRegs)->dstXY , 
	    ((DWORD)top << 16) | (WORD)left) ;
   
	SETC(cf, _FF(lpGRegs)->command,
	    cmdData |
	    SSTG_GO |
	    SSTG_RECTFILL );
	BUMP( 4 );
    } 

    // Draw the border
  DrawBorderOnly:

    // If a pointer to a pen exists, draw the border
    if (pParams->lpPPen)
    {

	FarToFlat( pParams->lpPPen, lpPPen);

	// Nothing to do if sytle is noline
	if (lpPPen->dpPenStyle != LS_NOLINE)
	{

	    cmdData = 0L;

	    // Handle clipping rectangle
	    if (lpClipRect)
            {
		cmdData |= SSTG_CLIPSELECT;
            }

	    // Solid and insideframe pen styles are handled the same
	    // for pen widths of one.
	    if ((lpPPen->dpPenStyle == LS_SOLID) ||
		(lpPPen->dpPenStyle == LS_INSIDEFRAME))
            {
		// convert rop for lines and use launch
		cmdData |= ( rop2ConvertTable[(lpDrawMode->Rop2)] | SSTG_POLYLINE);

		cmdBump = 9;
		CMDFIFO_CHECKROOM(cf, cmdBump);
		SETPH(cf, SSTCP_PKT2  |      
		      srcXYBit |
		      colorForeBit|
		      commandBit);
		SET(cf, _FF(lpGRegs)->srcXY,
		    ((DWORD) top)  << 16 | (WORD) left);
		SET(cf, _FF(lpGRegs)->colorFore, lpPPen->dpPenColor);
		SETC(cf, _FF(lpGRegs)->command, cmdData );
            }
	    else
            {
		// All remaining pen styles get handled here:
		// Dashed, Dotted, Dotdashed, Dashdotdot

		// Styled line may reqire transparency
		if (lpDrawMode->bkMode & BKMODE_TRANSPARENT)
		{
		    cmdData |= SSTG_TRANSPARENT;
		}

		// Setup the rop and command type
		cmdData |= rop2ConvertTable[(lpDrawMode->Rop2)] | SSTG_POLYLINE;


		// Preset the pen sytle and stipple
		SETUP_STIPPLE_FROM_PEN(lpPPen->dpPenStyle, lineStippleData, 
				       lineStyleData, cmdData);
		cmdBump = 12;
		CMDFIFO_CHECKROOM(cf, cmdBump);
		SETPH(cf, SSTCP_PKT2  |
		      lineStippleBit |
		      lineStyleBit   |
		      srcXYBit       |
		      colorBackBit   |
		      colorForeBit   |
		      commandBit);
      
		// Setup the pen sytle and stipple
		SET(cf, _FF(lpGRegs)->lineStipple, lineStippleData);
		SET(cf, _FF(lpGRegs)->lineStyle, lineStyleData);

		// prepare to draw from upper left
		SET(cf, _FF(lpGRegs)->srcXY,
		    ((DWORD)top) << 16 | ((WORD)left));
		SET(cf, _FF(lpGRegs)->colorBack, lpDrawMode->bkColor);
		SET(cf, _FF(lpGRegs)->colorFore, lpPPen->dpPenColor);
		SETC(cf, _FF(lpGRegs)->command, cmdData );
            }         
      
	    // Sync indices for points of the polyline
	    SETPH(cf, SSTCP_PKT1|
		  SSTCP_PKT1_2D|
		  LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT| 
		  4L << SSTCP_PKT1_NWORDS_SHIFT);

	    // Draw border top
	    SET(cf,  _FF(lpGRegs)->launch[0], 
		(((DWORD)top) << 16 | (WORD)right));

	    // Draw border right
	    SET(cf,  _FF(lpGRegs)->launch[0], 
		(((DWORD)bottom) << 16 | (WORD)right));

	    // Draw border bottom
	    SET(cf,  _FF(lpGRegs)->launch[0], 
		(((DWORD)bottom) << 16 | (WORD)left));

	    // Draw border left
	    SET(cf,  _FF(lpGRegs)->launch[0], 
		(((DWORD)top) << 16 | (WORD)left));

	    BUMP(cmdBump);
	}
    }

    FXLEAVE("hwOsRect", cf, lpDst);
    
ret_from_rect:
    // trivially clipped
    return 1;

ret_bad_rect:
    // malformed clip rectangle
    return 0xffff;
}


/*----------------------------------------------------------------------
Function name:  hwOsScanline

Description:    Use hardware to perform the GDI operation.

Information:

Return:         LONG
----------------------------------------------------------------------*/
LONG 
hwOsScanline(OutputParams* lp,
	     LPDIBENGINE   lpDst,
	     LPRECT        lpClipRect)
      //DIBENGINE FAR * lpDst, 
      //int wCount, 
      //POINT FAR * lpDstPoints, 
      //DIB_Pen FAR * lppPen, 
      //DIB_Brush8 FAR * lpPBrush, 
      //DRAWMODE FAR * lpDrawMode,
     //RECT FAR *   lpClipRect)
{
    RECT             OurRect;
    LPRECT     lpOurRect; 
    DWORD      dwFP1616;
    LPDRAWMODE lpDrawMode;
    LPPOINT    lpDstPoints; 
    DWORD       cmdData;
    DWORD      topX, topY, bottomX, bottomY;
    DWORD yVal;
    int j;
    DIB_Brush8  *  lpPBrush;
    DIB_Pen * lpPPen;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;
    
    FarToFlat( lp->lpDrawMode, lpDrawMode );
    FarToFlat( lp->lpPoints, lpDstPoints);

    // XXX FIXME
    // TBD, calculate proper s/w exclusion bounding box 
    //
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR)
        {
        lpOurRect = &OurRect;
        STACKFP(lpOurRect, dwFP1616)
        FarToFlat(dwFP1616, lpOurRect);
        FindBoundRect2(lpOurRect, lpDstPoints, lp->wCount);
        }

    FXENTER("hwOsScanline", lpDst->deFlags, (hwAll & hwScanline),
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
  		 FXENTER_RECT, OurRect.left, OurRect.top, OurRect.right, OurRect.bottom,
	    FXENTER_NO_RECT, 0, 0, 0, 0,   );

    cmdData = 0;


    // handle clip rect
    if (lpClipRect)
    {
	topX = lpClipRect->left;
	topY = lpClipRect->top;
	bottomX = lpClipRect->right;
	bottomY = lpClipRect->bottom;
         
	CMDFIFO_CHECKROOM(cf,  3 );
	SETPH(cf,   SSTCP_PKT2|      
	      clip1minBit|
	      clip1maxBit);
	SET(cf, _FF(lpGRegs)->clip1min, (topY <<16) | (WORD)topX );
   
	SET(cf, _FF(lpGRegs)->clip1max, (bottomY << 16) | (WORD)bottomX );
   
	BUMP( 3 );
   
	cmdData |= SSTG_CLIPSELECT;
    }

    if (lp->lpPBrush)
    {
	FarToFlat( lp->lpPBrush, lpPBrush);

	if ( lpPBrush->dp8BrushStyle == BS_NULL)
	{
	    //   DPF(" lpPBrush is NULL in scanline");
	    //return 1;
	}
	// rop is useless, but sets the mono expand bit if mono brush 
	// solid brush converion is doesn't work with rop2
	CMDFIFO_SAVE(cf);
	cmdData |= (DWORD)HandleBrush(lpPBrush, 
				  (rop2ToRop3[lpDrawMode->Rop2] >> 8),
				  lpDrawMode);   
	CMDFIFO_RELOAD(cf);

    }
    else
        {
        // Hey if No brush then use the Pen ...         
        // FIX_ME <APS> Do I need to worry about other line types then solid fill?
        // Added to FIX PRS 1726
        if (lp->lpPPen)
            { 
            cmdData |= rop2ConvertTable[lpDrawMode->Rop2];
            FarToFlat( lp->lpPPen, lpPPen);
            CMDFIFO_CHECKROOM(cf,3);
            SETPH(cf , SSTCP_PKT2 | colorBackBit | colorForeBit);
            SET(cf, _FF(lpGRegs)->colorBack, lpDrawMode->bkColor);
            SET(cf, _FF(lpGRegs)->colorFore, lpPPen->dpPenColor);
            BUMP( 3 );
            }
        }
   
    // draws  line segments on A SINGLE scan line
    // y member of first point determines Y coordinate of scan line.
    // For each subsequent point  x determines the starting point
    // of line segment and y determines ending point of line segment
   
    if (lpDrawMode->bkMode & BKMODE_TRANSPARENT)
    {
	cmdData |= SSTG_TRANSPARENT;
    }

    // draw line segment
    // semantics in lines below but will implement in blt.
    // scanlineY=lpDstPoints[0].y;
    // line(lpDstPoints[i].x,scanlineY,lpDstPoint[i].y, scanlineY);

    yVal=lpDstPoints[0].y;

    // it takes 4 dwords for each segment of scan

    for (j=1; j < lp->wCount; j++)
    {

	CMDFIFO_CHECKROOM(cf,  4);
	SETPH(cf, SSTCP_PKT2|
	      dstSizeBit|
	      dstXYBit |
	      commandBit);
   
	SET(cf, _FF(lpGRegs)->dstSize , ( 1L << 16) | 
	    (lpDstPoints[j].y - lpDstPoints[j].x) );

	SET(cf , _FF(lpGRegs)->dstXY , (yVal << 16) | lpDstPoints[j].x );
   
	SETC(cf , _FF(lpGRegs)->command,
	    cmdData |
	    SSTG_GO |
	    SSTG_RECTFILL );

	BUMP( 4 );
    }

    FXLEAVE("hwOsScanline", cf, lpDst)
}


/*----------------------------------------------------------------------
Function name:  hwOsPolyScanline

Description:    Use hardware to perform the GDI operation.

Information:

Return:         LONG
----------------------------------------------------------------------*/
LONG 
hwOsPolyScanline(OutputParams* pParams,
		 LPDIBENGINE   lpDst,
		 LPRECT        lpClipRect)
      //DIBENGINE FAR * lpDst, 
      //int wCount, 
      //POINT FAR * lpDstPoints, 
      //DIB_Pen FAR * lppPen, 
      //DIB_Brush8 FAR * lpPBrush, 
      //DRAWMODE FAR * lpDrawMode,
      //RECT FAR * lpClipRect) 
{
    RECT             OurRect;
    LPRECT     lpOurRect; 
    DWORD      dwFP1616;
    LPDRAWMODE lpDrawMode;
    LPPOINT    lpDstPoints; 
    DIB_Brush8  * lpPBrush; 
    LPpen     lpPPen;
    DWORD cmdData ;
    DWORD topX, topY, bottomX, bottomY;
    SCAN  * lpScan;
    SCAN  * lpScanRestart;
    short * lpPntX;
    short * lpPntXRestart;
    DWORD xCount, dwordsNeeded, j;
    int i;
    DWORD yVal;
    DWORD yLoop, yCount;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    cmdData = 0;

    FarToFlat( pParams->lpDrawMode, lpDrawMode );
    FarToFlat( pParams->lpPoints, lpDstPoints);

    // XXX FIXME
    // TBD, calculate proper s/w exclusion bounding box 
    //
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR)
        {
        lpOurRect = &OurRect;
        STACKFP(lpOurRect, dwFP1616)
        FarToFlat(dwFP1616, lpOurRect);
        FindBoundRect3(lpOurRect, lpDstPoints, pParams->wCount);
        }

    FXENTER("hwOsPolyScanline", lpDst->deFlags, (hwAll & hwPolyScanline),
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
       FXENTER_RECT, OurRect.left, OurRect.top, OurRect.right, OurRect.bottom,
	    FXENTER_NO_RECT, 0, 0, 0, 0,   );

    // handle clip rect
    if (lpClipRect)
    {
	topX = lpClipRect->left;
	topY = lpClipRect->top;
	bottomX = lpClipRect->right;
	bottomY = lpClipRect->bottom;

	CMDFIFO_CHECKROOM(cf,  3);
	SETPH(cf,   SSTCP_PKT2|      
	      clip1minBit|
	      clip1maxBit);
	SET(cf, _FF(lpGRegs)->clip1min, R32(topY, topX));
	SET(cf, _FF(lpGRegs)->clip1max, R32(bottomY, bottomX));
	BUMP(3);
   
	cmdData |= SSTG_CLIPSELECT;
    }
   
    if (pParams->lpPBrush)
    {
	FarToFlat( pParams->lpPBrush, lpPBrush);
	if ( lpPBrush->dp8BrushStyle == BS_NULL)
	{
	    //DPF(" lpPBrush is NULL in scanline");
	    //return 1;
	}
	// rop is useless, but sets the mono expand bit if mono brush 
	// solid brush converion is doesn't work with rop2
	CMDFIFO_SAVE(cf);
	cmdData |= (DWORD)HandleBrush(lpPBrush, 
				  (rop2ToRop3[lpDrawMode->Rop2] >> 8),
				  lpDrawMode);   
	CMDFIFO_RELOAD(cf);
    }
    else
	if (pParams->lpPPen)
	{

	    FarToFlat( pParams->lpPPen, lpPPen);

	    // set color for solid fill pen color.   
	    CMDFIFO_CHECKROOM(cf,  2);
	    SETPH(cf,SSTCP_PKT2 | colorForeBit);
	    SET(cf, _FF(lpGRegs)->colorFore, lpPPen->dpPenColor);
	    BUMP( 2 );

	    cmdData |= ( rop2ConvertTable[(lpDrawMode->Rop2)] );
	}

    // get pointer to first scanstructs
    lpScan=(SCAN *)lpDstPoints;
    lpPntX = (short *)(lpScan->scnPntX);

    //cde 031098
    // The DDK spec for this function is incomplete; it does not tell you
    // the you must draw the specified scanline segments at each Y location
    // from scnPntTop to scnPntBottom; so, this code was only drawing
    // at scnPntTop; I added code to repeat scanlines until scnPntBottom 
    // is reached; this fixed filled ellipses and punted polygons

    // outer loop for scans
    for(i=0; i < (short)pParams->wCount; i++)
    {
	// first y coordinate
	yVal = lpScan->scnPntTop;

	// save our current scan and x array pointers so we can use them
	// on the next vertical scanline
	lpScanRestart = lpScan;
	lpPntXRestart = lpPntX;

	yCount = lpScan->scnPntBottom - lpScan->scnPntTop;

	for( yLoop=0; yLoop<yCount; yLoop++ )
	{
	    lpPntX = lpPntXRestart;
	    lpScan = lpScanRestart;

	    //  for pairs of x coordinate that determine segment
	    xCount = lpScan->scnPntCnt >> 1;

	    // inner loop for line segments.
	    dwordsNeeded = DWORDS_PER_SCAN  * xCount;
	    CMDFIFO_CHECKROOM(cf, dwordsNeeded);
	    for (j=0; j < xCount; j++)
	    {
		SETPH(cf, SSTCP_PKT2 |
		      dstSizeBit |
		      dstXYBit   |
		      commandBit );
         
#ifdef DEBUG
		if ( (short)(lpPntX[1] - lpPntX[0]) <= 0 )
		{
		    __asm int 3   
			}
#endif
		SET(cf, _FF(lpGRegs)->dstSize , ( 1L << 16) | 
		    (lpPntX[1] - lpPntX[0]) );
   
#ifdef DEBUG
		if ( ((short)yVal<0) |((short)lpPntX[0]<0))
		{
		    __asm int 3   
			}
#endif
		SET(cf, _FF(lpGRegs)->dstXY , (yVal << 16) | lpPntX[0] );
         
		SETC(cf, _FF(lpGRegs)->command,
		    cmdData |
		    SSTG_GO |
		    SSTG_RECTFILL );

		// set up for next pair
		(DWORD)   lpPntX += 4; 
	    } // end for xCount

	    BUMP(dwordsNeeded);
                     
	    (DWORD)   lpPntX += 2; 
	    //setup for the next scan;
	    lpScan=(SCAN *)lpPntX;
	    lpPntX = (short *)(lpScan->scnPntX);

	    // go to the next vertical scanline
	    yVal++;
	}
    }

    FXLEAVE("hwOsPolyScanline", cf, lpDst);
}
            


