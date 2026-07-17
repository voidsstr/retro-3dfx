/* -*-c++-*- */
/* $Header: polygon.c, 2, 10/11/00 8:54:57 PM, Brent$ */
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
** File name:   polygon.c
**
** Description: The GDI polygon routines.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:57 PM$
**
** $History: polygon.c $
** 
** *****************  Version 21  *****************
** User: Michael      Date: 1/12/99    Time: 11:00a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 20  *****************
** User: Michael      Date: 9/28/98    Time: 7:00p
** Updated in $/devel/h3/Win95/dx/minivdd
** Modify the local variable "POINT breakpoint" in ComplexQuad() to be a
** static variable.  This was previously incorrect and was causing a hang
** in Winstone 98 CorelDraw.  Fixes PRS #2826.
** 
** *****************  Version 19  *****************
** User: Andrew       Date: 9/13/98    Time: 7:57a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed PRS 2252.  Look at all Y's in FindBoundRect3 and not just the
** first.
** 
** *****************  Version 18  *****************
** User: Michael      Date: 6/26/98    Time: 5:22p
** Updated in $/devel/h3/Win95/dx/minivdd
** MarkL (IGX) - Remove duplicate points from verticie data passed into
** AltPolygon.  Fixes 2022.
** 
** *****************  Version 17  *****************
** User: Michael      Date: 6/12/98    Time: 4:24p
** Updated in $/devel/h3/Win95/dx/minivdd
** HandleBrush was getting called with rop incorrectly positioned.  It
** needed to be right shifted one byte to get correcly handled.  Added fix
** and removed unneeded support code.  Fixes #1881.
** 
** *****************  Version 16  *****************
** User: Michael      Date: 6/01/98    Time: 10:30a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fred W's (igx) fix for dithering at 8-bpp with solid brush.  Fixes
** #1671 and 1678.
** 
** *****************  Version 15  *****************
** User: Andrew       Date: 6/01/98    Time: 6:52a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added clip rectangles to FXENTER for polygon functions
** 
** *****************  Version 14  *****************
** User: Michael      Date: 5/15/98    Time: 4:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Backed out my previous changes.
** 
** *****************  Version 13  *****************
** User: Michael      Date: 5/15/98    Time: 9:24a
** Updated in $/devel/h3/Win95/dx/minivdd
** Add lpDst to all FXPUNT and FXLEAVE_NOHWACCESS.  Modify lpDestDev to
** lpDst.
** 
** *****************  Version 12  *****************
** User: Ken          Date: 5/14/98    Time: 8:31a
** Updated in $/devel/h3/win95/dx/minivdd
** added a PERFNOP case to gdi32.   change the "undef" to a "define"
** in fifomgr.h, and enable in softice (doperfnop = 1).  Replaces every
** write
** to the command register with a "0"
** 
** *****************  Version 11  *****************
** User: Ken          Date: 4/27/98    Time: 5:35p
** Updated in $/devel/h3/win95/dx/minivdd
** complete (well, almost) enter/leave implementation for gdi32
** 
** *****************  Version 10  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

/****************************************************************************
 *
 *  POLYGON.C
 *
 *      This module contains the routines necessary to accelerate selected
 *      GDI Output(AltPolygon) calls using the Banshee hardware.  It is 
 *      optimized for the SPEEDY benchmark as well as WinBench 98.
 *
 *
 ****************************************************************************/

#include "h3.h"
#include "thunk32.h"
#include "polygon.h"
#include "h3g.h"
#include "entrleav.h"

extern DWORD rop2ToRop3[17];
int FindBoundRect(LPRECT lpRect, LPPOINT lpPoints, int nVerts);


/*----------------------------------------------------------------------
Function name:  hwOsAltPolygon

Description:    This is the main AltPolygon dispatch routine
                called by the AltPolygon Case code in OUTPUT.
Information:    

Return:         LONG
----------------------------------------------------------------------*/
LONG 
hwOsAltPolygon(OutputParams   * pParams,
	       LPDIBENGINE      lpDestDev,
	       LPRECT           lpClipRect)
//  DIBENGINE FAR  * lpDestDev, 
//  int              wCount, 
//  POINT FAR      * lpDstPoints, 
//  DIB_Pen FAR    * lppPen, 
//  DIB_Brush8 FAR * lpPBrush, 
//  DRAWMODE FAR   * lpDrawMode,
//  RECT FAR       * lpClipRect) 
{
    LPPOINT          lpDstPoints; 
    LONG             retval;
    LONG             direction;
    int              NumVertices;
    int              i;
    int              j;
    
    DEBUG_FIX;

	// Check for closed polygon, adjust wCount if found
	NumVertices = pParams->wCount;
    FarToFlat( pParams->lpPoints, lpDstPoints);

    if ((lpDstPoints[0].x == lpDstPoints[NumVertices-1].x)  &&
        (lpDstPoints[0].y == lpDstPoints[NumVertices-1].y))
    {
        NumVertices -= 1;
    }

    // Here we look for duplicate points.  This will cause us to fail back
    // to GDI, and GDI doesn't rasterize polygons that well...
    for (i = 1; i < NumVertices-1; i++) 
    {
        if ((lpDstPoints[i].x == lpDstPoints[i+1].x)  &&
            (lpDstPoints[i].y == lpDstPoints[i+1].y))
        {
            for (j=i; j < NumVertices-1; j++)
            {
                lpDstPoints[j] = lpDstPoints[j+1];
            }
            NumVertices -= 1;
        }
    }    


    if (NumVertices == 4)
    {
        retval = SpeedyPGon(pParams,lpDestDev,lpClipRect,lpDstPoints);
    }
    else
    {
    
        direction = SimpleConvexCheck(lpDstPoints,NumVertices);
        
        if ( direction != NOTSIMPLECONVEX)
	{
            retval = SimpleConvexPolygon(pParams,
                                         lpDestDev,
                                         lpClipRect,
                                         lpDstPoints,
                                         NumVertices,
                                         direction);
        }
	else
	{
            retval = -1;    
        }
    }

    if (retval == -1)
    {
	FXPUNT(FXPUNT_NORMAL);
    }

    FXLEAVE_NOHWACCESS("hwOsAltPolygon", 1);
}


/*----------------------------------------------------------------------
Function name:  SpeedyPGon

Description:    This routine will special case polygons with
                4 unique vertices.
Information:    
      It will divide these polygons into three specific cases:        

         1)  Simple Convex quadrilaterals                       
         2)  Simple Concave quadrilaterals 
         3)  Complex quadrilaterals 

      It will use cross products to detemine which cases apply, and
      then will dispatch to special case routines for each.

Return:         LONG  Value returned from one of the three specific
                      polygon routines called within.
----------------------------------------------------------------------*/
LONG
SpeedyPGon(
    OutputParams * pParams,
    LPDIBENGINE    lpDestDev,
    LPRECT         lpClipRect,
    LPPOINT        lpDstPoints
)
{
    LONG        retval;
    LONG        xp1;
    LONG        xp2;
    LPPOINT     p0;    
    LPPOINT     p1;    
    LPPOINT     p2;    
    LPPOINT     p3;    

    DEBUG_FIX
    

    p0 = &lpDstPoints[0];
    p1 = &lpDstPoints[1];
    p2 = &lpDstPoints[2];
    p3 = &lpDstPoints[3];
    
    
    xp1 = XProduct(p0,p1,p2);
    xp2 = XProduct(p0,p1,p3);
    if (xp1 == COLLINEAR || xp2 == COLLINEAR) {
        return -1;
    }    
        
    if ( xp1 == xp2) {                                  // Line zero SAME
    
        xp1 = XProduct(p1,p2,p0);
        xp2 = XProduct(p1,p2,p3);
        if (xp1 == COLLINEAR || xp2 == COLLINEAR) {
            return -1;
        }    
        if (xp1 == xp2) {                               // Line one SAME
        
            xp1 = XProduct(p2,p3,p0);
            xp2 = XProduct(p2,p3,p1);
            if (xp1 == COLLINEAR || xp2 == COLLINEAR) {
                return -1;
            }    
            if (xp1 == xp2) {                           // Line two SAME        
            
                retval = SimpleConvexPolygon(pParams,
                                            lpDestDev,
                                            lpClipRect,
                                            lpDstPoints,
                                            (int) 4,
                                            xp1);
            } else {                                    // Line two OPP
                // lpDstPoints[3] is 'inside' vertex
                retval = SimpleConcaveQuad(pParams,             
                                           lpDestDev,
                                           lpClipRect,
                                           lpDstPoints,
                                           (DWORD)3);
            }        
        } else {                                        // Line one OPP
        
            xp1 = XProduct(p2,p3,p0);
            xp2 = XProduct(p2,p3,p1);
            if (xp1 == COLLINEAR || xp2 == COLLINEAR) {
                return -1;
            }    
            if (xp1 == xp2) {  // Line two SAME        
                retval = ComplexQuad(pParams,
                                     lpDestDev,
                                     lpClipRect,
                                     lpDstPoints);
            } else {                                    // Line two OPP
                // lpDstPoints[2] is 'inside' vertex
                retval = SimpleConcaveQuad(pParams,
                                           lpDestDev,
                                           lpClipRect,
                                           lpDstPoints,
                                           (DWORD)2);
            }        
        }
        
    } else {                                            // Line zero OPPOSITE
                                        
        xp1 = XProduct(p1,p2,p0);
        xp2 = XProduct(p1,p2,p3);
        if (xp1 == COLLINEAR || xp2 == COLLINEAR) {
            return -1;
        }    
        if (xp1 == xp2) {                               // Line one SAME
        
            xp1 = XProduct(p2,p3,p0);
            xp2 = XProduct(p2,p3,p1);
            if (xp1 == COLLINEAR || xp2 == COLLINEAR) {
                return -1;
            }    
            if (xp1 == xp2) {  // Line two SAME    
                // lpDstPoints[0] is 'inside' vertex    
                retval = SimpleConcaveQuad(pParams,
                                           lpDestDev,
                                           lpClipRect,
                                           lpDstPoints,
                                           (DWORD)0);
            } else {                                    // Line two OPP
                retval = ComplexQuad(pParams,
                                     lpDestDev,
                                     lpClipRect,
                                     lpDstPoints);
            }        
    
        } else {                                        // Line one OPP
            // lpDstPoints[1] is 'inside' vertex
            retval = SimpleConcaveQuad(pParams,
                                       lpDestDev,
                                       lpClipRect,
                                       lpDstPoints,
                                       (DWORD)1);
        }
    }                                                            
    return retval;
}


/*----------------------------------------------------------------------
Function name:  SimpleConvexPolygon  

Description:    This routine is responsible for using the Banshee
                polygon fill hardware where applicable.  It will
                determine if the hardware supports the polygon
                parameters and if so will send the correct 
                information to the FIFO.
Information:    
  Banshee HW polygon algorithm. (Following comments from the NT driver)

    1. Start at top

    2. Move top pointer left one if the y value of the point to the left
       of the topmost point equals the y value of the topmost point.
       (Check wrap in point array).

       This is so that if there is more than one point that shares the
       lowest Y coordinate we will always send the vertices to the HW
       starting with the leftmost point.  We only have to check one
       point to the left because our test for hardware drawing rejected
       collinear points.

    3. Send pptfxTop (the topmost, leftmost point) to srcXY.

    4. If more than one point shares the lowest y value (topmost) send
       the rightmost top point to dstXY, otherwise send pptfxTop to
       dstXY.

    5. Initiate command.

    6. Keep track of the last y value sent for the left and right sides.
       If the y value for the last vertex sent for the left side is
       <= the last y value sent for the right side, the next vertex on
       the left side should be written to the launch area.  Otherwise,
       the next vertex for the right side should be written to the
       launch area.

    7. When we send the last vertex to the launch area:

           // Compare y values of last left and right vertices sent.

           if ( lvertex.y < rvertex.y )
              send rvertex again;
           else if ( lvertex.y > rvertex.y )
              send lvertex again;
           else  // lvertex.y == rvertex.y
              do nothing -- done;
                   

Return:         LONG
----------------------------------------------------------------------*/
LONG 
SimpleConvexPolygon(OutputParams   * pParams,
		    LPDIBENGINE      lpDst,
		    LPRECT           lpClipRect,
		    LPPOINT          lpDstPoints,
		    int              NumVerts,
		    LONG             direction)
{
    RECT             OurRect;
    LPDRAWMODE       lpDrawMode;
    DIB_Brush8     * lpPBrush;
    LPRECT           lpOurRect; 
    DWORD            dwFP1616;
    DWORD            topX;
    DWORD            topY;
    DWORD            bottomX;
    DWORD            bottomY;
    DWORD            cmdData;
    DWORD            VtxsToWrite;
    BOOL             FlatTop;
    DWORD            i;
    int              j;
    int              TopIndex;
    int              CurLeftIndex;
    int              CurRiteIndex;
    int              Left;
    int              Right;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;
    
    FarToFlat(pParams->lpDrawMode, lpDrawMode);

    cmdData = 0;
    FlatTop = FALSE;

    // XXX FIXME
    // TBD, calculate proper s/w exclusion bounding box 
    //
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR)
        {
        lpOurRect = &OurRect;
        STACKFP(lpOurRect, dwFP1616)
        FarToFlat(dwFP1616, lpOurRect);
        FindBoundRect(lpOurRect, lpDstPoints, NumVerts);
        }
    FXENTER("hwOsAltPolygon/SimpleConvexPolygon", lpDst->deFlags,
	    (hwAll & hwAltPolygon),
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
	    FXENTER_RECT, OurRect.left, OurRect.top, OurRect.right, OurRect.bottom,
	    FXENTER_NO_RECT, 0, 0, 0, 0,   );

    // Handle clip rect
    if ( lpClipRect )
    {
        topX    = lpClipRect->left;
        topY    = lpClipRect->top;
        bottomX = lpClipRect->right;
        bottomY = lpClipRect->bottom;

        CMDFIFO_CHECKROOM(cf,  3 );
        SETPH(cf,  SSTCP_PKT2  |   
	      clip1minBit |
	      clip1maxBit);
        SET(cf, _FF(lpGRegs)->clip1min, (topY <<16) | topX );
        SET(cf, _FF(lpGRegs)->clip1max, (bottomY << 16) | bottomX );
        BUMP(3);
    
        cmdData |= SSTG_CLIPSELECT;
    }    

    // Handle color/pattern setup for HW
    if (pParams->lpPBrush)
    {
	FarToFlat( pParams->lpPBrush, lpPBrush);

	// Need to save return val just in case its mono pattern
	// Handle brush loads brush or colorFore for solid brush
	if ( lpPBrush->dp8BrushStyle == BS_NULL)
	{
	    goto SCPDrawBorderOnly;
	}

	CMDFIFO_SAVE(cf);
	//cde 030698
	// Changed to OR in HandleBrush results so SSTG_CLIPSELECT bit
	// which might already be set is not clobbered!
	cmdData |=  HandleBrush(lpPBrush, 
				(rop2ToRop3[lpDrawMode->Rop2] >> 8),
				lpDrawMode);    
	CMDFIFO_RELOAD(cf);

    } 
    
    // Find Top Point (Min Y value, if two min Y's, find left one.)
    TopIndex = 0;
    
    for (j=0; j < NumVerts; j++) {
        if (lpDstPoints[j].y < lpDstPoints[TopIndex].y) {
            TopIndex = j;
        }
    }
    
    // We need to be able to move 'left' and 'right' on the polygon, so
    // we need to determine the values needed to index to the next and
    // previous vertices in the array.  Wrapping is handled by using
    // MOD on the indices -- performance might be improved by doing
    // literal checks for overflow or underflow, but what a hassle!
    if (direction < 0) {            // Polygon specified in CCW order
        Left  = 1;                  // Moving 'left'  = increasing index
        Right = NumVerts-1;         // Moving 'right' = decreasing index
    } else {                        // Polygon is spec'd in CW order
        Left  = NumVerts-1;         // Moving 'left'  = decreasing index
        Right = 1;                  // Moving 'right' = increasing index
    }
    
    // TopIndex has index of smallest Y coord, but check for another 
    // minimum Y to 'left'.  If there, update TopIndex to point to it.
    if (lpDstPoints[(TopIndex+Left)%NumVerts].y == lpDstPoints[TopIndex].y) {
        TopIndex = (TopIndex+Left)%NumVerts;  // Top-left on horz top edge
        FlatTop = TRUE;
    }
    

    // Set up BANSHEE for Polygon Fill  
    cmdData |= SSTG_POLYFILL;

    CMDFIFO_CHECKROOM(cf, 4);
    SETPH(cf,  SSTCP_PKT2  |       
	  srcXYBit    |
	  dstXYBit    |
	  commandBit);

    SET(cf, _FF(lpGRegs)->srcXY, ((DWORD)lpDstPoints[TopIndex].y << 16) |
	((WORD)lpDstPoints[TopIndex].x      )  );

    CurLeftIndex = CurRiteIndex = TopIndex;
    
    if (FlatTop) {              // Set DstXY to right point
        SET(cf, 
            _FF(lpGRegs)->dstXY, 
            (((DWORD)lpDstPoints[(TopIndex+Right)%NumVerts].y << 16) |
             ((WORD)lpDstPoints[(TopIndex+Right)%NumVerts].x)        )) ;
        CurRiteIndex = (TopIndex + Right) % NumVerts;     
        VtxsToWrite = NumVerts - 2;
    } else {                    // Set DstXY to SrcXY point
        SET(cf, 
            _FF(lpGRegs)->dstXY, 
            (((DWORD)lpDstPoints[TopIndex].y << 16) |
             ((WORD)lpDstPoints[TopIndex].x)        ));
        VtxsToWrite = NumVerts - 1;
    }
    
    SETC(cf, _FF(lpGRegs)->command, cmdData );
    BUMP( 4 );
    

    CMDFIFO_CHECKROOM(cf, VtxsToWrite + 1);
    SETPH(cf, SSTCP_PKT1                            |
	  SSTCP_PKT1_2D                         |
	  LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT   |
	  ((DWORD)VtxsToWrite) << SSTCP_PKT1_NWORDS_SHIFT); 

    for (i=0; i < VtxsToWrite; i++) {

        if (lpDstPoints[CurLeftIndex].y <= lpDstPoints[CurRiteIndex].y) {
            
            // Write next left vertex
            CurLeftIndex = (CurLeftIndex + Left) % NumVerts;    
            SET(cf, 
                _FF(lpGRegs)->launch[0], 
                ((DWORD)lpDstPoints[CurLeftIndex].y)  << 16|
                ((WORD)lpDstPoints[CurLeftIndex].x) );
            
        } else {
        
            // Write next right vertex
            CurRiteIndex = (CurRiteIndex + Right) % NumVerts;   
            SET(cf, 
                _FF(lpGRegs)->launch[0], 
                ((DWORD)lpDstPoints[CurRiteIndex].y)  << 16|
                ((WORD)lpDstPoints[CurRiteIndex].x) );
            
        }
    }

    BUMP(VtxsToWrite + 1);
    
    
    if (lpDstPoints[CurLeftIndex].y < lpDstPoints[CurRiteIndex].y ) {
        // Resend Right Vtx 
        CMDFIFO_CHECKROOM(cf, 2);
        SETPH(cf, SSTCP_PKT1                          |
	      SSTCP_PKT1_2D                       |
	      LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT |
	      ((DWORD)1) << SSTCP_PKT1_NWORDS_SHIFT); 
                    
        SET(cf, 
            _FF(lpGRegs)->launch[0], 
            ((DWORD)lpDstPoints[CurRiteIndex].y)  << 16|
            ((WORD)lpDstPoints[CurRiteIndex].x) );
                
                 
        BUMP(2);
        
    } else if (lpDstPoints[CurLeftIndex].y > lpDstPoints[CurRiteIndex].y ) {
        // Resend Left Vtx 
        CMDFIFO_CHECKROOM(cf, 2);
        SETPH(cf, SSTCP_PKT1                          |
	      SSTCP_PKT1_2D                       |
	      LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT |
	      ((DWORD)1) << SSTCP_PKT1_NWORDS_SHIFT); 
                    
        SET(cf, 
            _FF(lpGRegs)->launch[0], 
            ((DWORD)lpDstPoints[CurLeftIndex].y)  << 16  |
            ((WORD)lpDstPoints[CurLeftIndex].x) );
                
                
        BUMP(2);
        
    } else {
        // All Done, horizontal edge bottom for polygon
    }

  SCPDrawBorderOnly:

    if (pParams->lpPPen)
    {
	CMDFIFO_SAVE(cf);
        hwOsPolyLine(pParams,lpDst,lpClipRect);
	CMDFIFO_RELOAD(cf);
    }   

    FXLEAVE("hwOsAltPolygon/SimpleConvexPolygon", cf, lpDst);
}


/*----------------------------------------------------------------------
Function name:  SimpleConcaveQuad

Description:    This routine will break a simple convex quad into
                two triangles and call the low level rendering
                routine to draw them.  It calls FindQuadHandle
                to determine where the breakpoint should be (it 
                actually returns an index to one of the 'boomerang
                handles' of the quad, and we can tell the rest from
                that).  
Information:    
  We have a quadrilateral -- need to know which two vertices to 
  'cut' between to break the quad into two triangles.  The cross
  products in the calling routine find the 'inside' vertex for us
  and we use it here to split the quad into two triangles.

   i.e.,
                            VTX A
                          /| 
                         / |
                        /  |
                       /   |
                      /    |
                     /     |
                    /      | 'INSIDE' vtx -- inside bounding area of
                   /        \                polygon vertices.
                  /          \ 
                 /            \
                /              \
               /                \
              /__________________\
             
           VTX B                VTX C


  Quadrilateral will be split between the 'inside' vertex and vertex B.

Return:         LONG    Result of the call to RenderTrianglePair.
----------------------------------------------------------------------*/
LONG
SimpleConcaveQuad(
    OutputParams * pParams,
    LPDIBENGINE    lpDst,
    LPRECT         lpClipRect,
    LPPOINT        lpDstPoints,
    DWORD          breakvertex
)
{
    
    LPPOINT        p0;
    LPPOINT        p1;
    LPPOINT        p2;
    LPPOINT        p3;
    LONG           retval;

    DEBUG_FIX;

    p0 = &lpDstPoints[breakvertex];
    p1 = &lpDstPoints[(breakvertex + 2) % 4];
    p2 = &lpDstPoints[(breakvertex + 1) % 4];
    p3 = &lpDstPoints[(breakvertex + 3) % 4];
    
    retval = RenderTrianglePair(pParams,
                                lpDst,
                                lpClipRect,
                                p0, p1, p2,   // SubTriangle One
                                p0, p1, p3);  // SubTriangle Two

    return retval;

}    


/*----------------------------------------------------------------------
Function name:  ComplexQuad

Description:    This routine will break a simple convex quad into
                two triangles and call the low level rendering
                routine to draw them.  It calls Intersect on each
                pair of line segments until it finds an intersection.
                It then calls the special HW polygon routine that
                will render the two resulting triangles.
Information:    
  i.e.,
             VTX D        VTX A
                 \-------/
                  \     /
                   \   /
                    \ / 
                     X 'breakpoint'-- quad is split at this vertex into
                    / \               two triangles.
                   /   \ 
                  /     \                
                 /       \ 
                /         \
               /           \
              /             \
             /_______________\
            
          VTX B                VTX C

Return:         LONG    Result of the call to RenderTrianglePair.
----------------------------------------------------------------------*/
LONG
ComplexQuad(
    OutputParams* pParams,
    LPDIBENGINE   lpDst,
    LPRECT        lpClipRect,
    LPPOINT       lpDstPoints
)
{
    LPPOINT p0;
    LPPOINT p1;
    LPPOINT p2;
    LPPOINT p3;
    static POINT   breakpoint;
    long    status;
    long    retval;

    DEBUG_FIX;

    p0 = &lpDstPoints[0];
    p1 = &lpDstPoints[1];
    p2 = &lpDstPoints[2];
    p3 = &lpDstPoints[3];
    
    // We have a quadrilateral -- need to know which two line segments
    // cross over each other and what the intersection point is.
    // We have a 50-50 chance to guess right the first time. 

    status = Intersect(p0,p1, p2,p3, &breakpoint);
                       
    if (status == DO_INTERSECT) {  // If this pair intersects, render.
                                                          
        retval = RenderTrianglePair(pParams,
                                    lpDst,
                                    lpClipRect,
                                    p1, p2, &breakpoint,   // SubTriangle One
                                    p3, p0, &breakpoint);  // SubTriangle Two
                                    
                                    
        
    } else {            // We KNOW these two cross, we just need the coords.
    
        status = Intersect(p1,p2, p3,p0, &breakpoint);
        retval = RenderTrianglePair(pParams,
                                    lpDst,
                                    lpClipRect,
                                    p0, p1, &breakpoint,   // SubTriangle One
                                    p3, p2, &breakpoint);  // SubTriangle Two
    }   
    
    return retval;                        

}    


/*----------------------------------------------------------------------
Function name:  RenderTrianglePair  

Description:    This routine is responsible for rendering a pair
                of triangles resulting from the decomposition of
                a quadrilateral that could not be rendered by the
                simpleconvex routine.  It is basically the same
                but is special cased for the two triangles.
Information:    

Return:         LONG    
----------------------------------------------------------------------*/
LONG 
RenderTrianglePair(OutputParams     * pParams,
		   LPDIBENGINE        lpDst,
		   LPRECT             lpClipRect,
		   LPPOINT            t1p1,
		   LPPOINT            t1p2,
		   LPPOINT            t1p3,
		   LPPOINT            t2p1,
		   LPPOINT            t2p2,
		   LPPOINT            t2p3)
{
    RECT               OurRect;
    POINT              DstPoints[6];
    LPRECT             lpOurRect;
    LPPOINT            lpDstPoints;
    DWORD              dwFP1616;
    LPDRAWMODE         lpDrawMode;
    DIB_Brush8       * lpPBrush; 
    LPPOINT            Temp;
    LPPOINT            p1;
    LPPOINT            p2;
    LPPOINT            p3;
    DWORD              topX;
    DWORD              topY;
    DWORD              bottomX;
    DWORD              bottomY;
    DWORD              SrcXY;
    DWORD              DstXY;
    TAKE_ADDRESS DWORD FifoData[5];
    DWORD              cmdData;
    DWORD              FifoDatas;
    DWORD              i;
    int                triangle;
    CMDFIFO_PROLOG(cf);

    //
    // Banshee HW polygon algorithm.  (See SimpleConvexPolygon)
    //
     
    DEBUG_FIX;

    FarToFlat( pParams->lpDrawMode, lpDrawMode );

    cmdData = 0;


    // XXX FIXME
    // TBD, calculate proper s/w exclusion bounding box 
    //
    if (_FF(gdiFlags) & SDATA_GDIFLAGS_SW_CURSOR)
       {
       DstPoints[0] = *t1p1;
       DstPoints[1] = *t1p2;
       DstPoints[2] = *t1p3;
       DstPoints[3] = *t2p1;
       DstPoints[4] = *t2p2;
       DstPoints[5] = *t2p3;
       lpOurRect = &OurRect;
       STACKFP(lpOurRect, dwFP1616)
       FarToFlat(dwFP1616, lpOurRect);
       lpDstPoints = DstPoints;
       STACKFP(lpDstPoints, dwFP1616)
       FarToFlat(dwFP1616, lpDstPoints);
       FindBoundRect(lpOurRect, lpDstPoints, 6);
       }
    FXENTER("hwOsAltPolygon/RenderTrianglePair", lpDst->deFlags,
	    (hwAll & hwAltPolygon),
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
	    FXENTER_RECT, OurRect.left, OurRect.top, OurRect.right, OurRect.bottom,
	    FXENTER_NO_RECT, 0, 0, 0, 0,   );

    // Handle clip rect

    if ( lpClipRect )
    {
        topX = lpClipRect->left;
        topY = lpClipRect->top;
        bottomX = lpClipRect->right;
        bottomY = lpClipRect->bottom;

        CMDFIFO_CHECKROOM(cf,  3 );
        SETPH(cf,  SSTCP_PKT2  |   
	      clip1minBit |
	      clip1maxBit);
                        
        SET(cf, _FF(lpGRegs)->clip1min, (topY <<16) | topX );
    
        SET(cf, _FF(lpGRegs)->clip1max, (bottomY << 16) | bottomX );
    
        BUMP(3);
    
        cmdData |= SSTG_CLIPSELECT;
    }    

    // Handle color/pattern setup for HW
    if (pParams->lpPBrush)
    {
        FarToFlat( pParams->lpPBrush, lpPBrush);

        // Need to save return val just in case its mono pattern
        // Handle brush loads brush or colorFore for solid brush
        if ( lpPBrush->dp8BrushStyle == BS_NULL) {
            goto RTPDrawBorderOnly;
        }

        CMDFIFO_SAVE(cf);
        //cde 030698
        // Changed to OR in HandleBrush results so SSTG_CLIPSELECT bit
        // which might already be set is not clobbered!
        cmdData |=  HandleBrush(lpPBrush, 
                                (rop2ToRop3[lpDrawMode->Rop2] >> 8),
                                lpDrawMode);    
        CMDFIFO_RELOAD(cf);
    } 
    
    // Draw each triangle
    for (triangle = 0; triangle < 2; triangle++)
    {
        if (triangle == 0) {    // We'll draw two triangles, sharing code.
            p1 = t1p1;
            p2 = t1p2;
            p3 = t1p3;
        } else {
            p1 = t2p1;
            p2 = t2p2;
            p3 = t2p3;
        }
    
        // Sort triangle vertices by increasing Y.
        // This will simplify the CW/CCW cases below.
        SORT_TRI(p1,p2,p3,Temp);
    
        // By sorting Y and knowing CW/CCW, (and special casing horizontal
        // edges) we will know ahead of time which order to send the 
        // vertices to the launch area.  Flat tops have to be checked for
        // anyway to set DstXY, and the only extra check is for Tri's with
        // a horizontal bottom edge.
        // p1 has index of smallest Y coord, but check for another 
        // minimum Y to left (our pts can be in CW or CCW order, so check 
        // both adjacent pts, looking at X.
        if (XProduct(p1,p2,p3) == CLOCKWISE) {      // ClockWise
    
            // Check pt to left of p1 (p3) to see if it is equal in Y.
            // Left-top should be p1.
            if (p1->y == p3->y) {                   // Flat top tri.  
            
                
                // Two points with equal Y vals.  Sort these by X value.
                if (p1->x > p3->x) {
                    SWAP_PT(p1,p3,Temp);            // Set leftmost Y as p1.
                }
                // Horizontal Top edge -- p1 is Srcxy, p2 is dstXY.
                SrcXY       = ((DWORD)p1->y << 16) | ((WORD)p1->x);    
                DstXY       = ((DWORD)p3->y << 16) | ((WORD)p3->x);
                FifoData[0] = ((DWORD)p2->y << 16) | ((WORD)p2->x);
                FifoData[1] = FifoData[0];
                FifoDatas   = 2;
                
            } else {                                // Not Flat top -- normal
            
                SrcXY = ((DWORD)p1->y << 16) | ((WORD)p1->x);    
                DstXY = SrcXY;
                                      
                if (p2->y == p3->y) {               // Flat Bottom -- 2 Writes   
                
                    FifoData[0] = ((DWORD)p3->y << 16) | ((WORD)p3->x);     
                    FifoData[1] = ((DWORD)p2->y << 16) | ((WORD)p2->x);
                    FifoDatas   = 2;
                    
                } else {                            // Regular Triangle (CW)
                
                    FifoData[0] = ((DWORD)p3->y << 16) | ((WORD)p3->x);     
                    FifoData[1] = ((DWORD)p2->y << 16) | ((WORD)p2->x);
                    FifoData[2] = FifoData[0];
                    FifoDatas   = 3;
                }    
            }
        } else {                                    // Counter Clockwise
                                                    
            if (p1->y == p2->y) {                   // Flat top tri.  Check X.
            
                if (p1->x > p2->x) {
                    SWAP_PT(p1,p2,Temp);            // Set leftmost Y as p1.
                }
                SrcXY       = ((DWORD)p1->y << 16) | ((WORD)p1->x);    
                DstXY       = ((DWORD)p2->y << 16) | ((WORD)p2->x);
                FifoData[0] = ((DWORD)p3->y << 16) | ((WORD)p3->x);
                FifoData[1] = FifoData[0];
                FifoDatas   = 2;
                
            } else {                                // Not Flat top -- normal
            
                SrcXY = ((DWORD)p1->y << 16) | ((WORD)p1->x);    
                DstXY = SrcXY;
                                      
                if (p2->y == p3->y) {               // Flat Bottom -- 2 Writes   
                
                    FifoData[0] = ((DWORD)p2->y << 16) | ((WORD)p2->x);     
                    FifoData[1] = ((DWORD)p3->y << 16) | ((WORD)p3->x);
                    FifoDatas   = 2;
                    
                } else {                            // Regular Triangle (CW)
                
                    FifoData[0] = ((DWORD)p2->y << 16) | ((WORD)p2->x);     
                    FifoData[1] = ((DWORD)p3->y << 16) | ((WORD)p3->x);
                    FifoData[2] = FifoData[1];
                    FifoDatas   = 3;
                }    
            }
        }    
                        
        // Set up BANSHEE for Polygon Fill  
        cmdData |= SSTG_POLYFILL;

        CMDFIFO_CHECKROOM(cf, 4);
        SETPH(cf, SSTCP_PKT2  |        
	      srcXYBit    |
	      dstXYBit    |
	      commandBit);
        SET(cf, _FF(lpGRegs)->srcXY, SrcXY);
        SET(cf, _FF(lpGRegs)->dstXY, DstXY);
        SETC(cf, _FF(lpGRegs)->command, cmdData );
        BUMP( 4 );
    
        CMDFIFO_CHECKROOM(cf, FifoDatas + 1);
        SETPH(cf, SSTCP_PKT1                          |
	      SSTCP_PKT1_2D                       | 
	      LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT | 
	      (FifoDatas << SSTCP_PKT1_NWORDS_SHIFT)); 

        for (i=0; i < FifoDatas; i++)
	{
            SET(cf, _FF(lpGRegs)->launch[0], FifoData[i]); 
        }

        BUMP(FifoDatas + 1);
    }                                       // Draw next Tri.
    
  RTPDrawBorderOnly:

    // Draw border, if there is one.
    if (pParams->lpPPen)
    {
	CMDFIFO_SAVE(cf);
	hwOsPolyLine(pParams, lpDst, lpClipRect);
	CMDFIFO_RELOAD(cf);
    }   

    FXLEAVE("hwOsAltPolygon/RenderTrianglePair", cf, lpDst);
}


/*----------------------------------------------------------------------
Function name:  XProduct

Description:    Cross Product is ((vector1.x * vector2.y) -
                (vector2.x * vector1.y)).  We get three points as
                args -- p0, p1, p2.  p0 and p1 make vector1, p1
                and p2 make vector 2.  

Information:    This is an INLINE function.

Return:         LONG    Either CLOCKWISE, COUNTERCLOCKWISE,
                        or COLLINEAR.
----------------------------------------------------------------------*/
INLINE LONG 
XProduct(LPPOINT p0, LPPOINT p1, LPPOINT p2)
{
    int xp;
    
    DEBUG_FIX
    
    xp = ((p1->x - p0->x) * (p2->y - p1->y) - 
          (p2->x - p1->x) * (p1->y - p0->y)  );
    
    if (xp < 0) {
        return COUNTERCLOCKWISE;
    } else if (xp > 0) {
        return CLOCKWISE;
    } else {
        return COLLINEAR;
    }
}                                    


/*----------------------------------------------------------------------
Function name:  Intersect

Description:    This function computes whether two line segments,
                respectively joining the input points (x1,y1) -- (x2,y2)
                and the input points (x3,y3) -- (x4,y4) intersect.
                If the lines intersect, the output variables x, y are
                set to coordinates of the point of intersection.

Information:    Borrowed from Graphics Gems II
                All values are in integers.  The returned value is
                rounded to the nearest integer point.

Return:         LONG    DONT_INTERSECT, DO_INTERSECT, or COLLINEAR.
----------------------------------------------------------------------*/
#define SAME_SIGNS( a, b )  \
        (((long) ((unsigned long) a ^ (unsigned long) b)) >= 0 )

long                       
Intersect
(
    LPPOINT p1,                 // Line Segment 1, point 0
    LPPOINT p2,                 // Line Segment 1, point 1
    LPPOINT p3,                 // Line Segment 2, point 0
    LPPOINT p4,                 // Line Segment 2, point 1
    LPPOINT intersection        // Point of intersection
)
{
    long a1, a2, b1, b2, c1, c2; // Coefficients of line eqns. 
    long r1, r2, r3, r4;         // 'Sign' values 
    long denom, offset, num;     // Intermediate values 
    
    DEBUG_FIX

    // Compute a1, b1, c1, where line joining points 1 and 2
    // is "a1 x  +  b1 y  +  c1  =  0".
    a1 = p2->y - p1->y;
    b1 = p1->x - p2->x;
    c1 = p2->x * p1->y - p1->x * p2->y;

    // Compute r3 and r4.
    r3 = a1 * p3->x + b1 * p3->y + c1;
    r4 = a1 * p4->x + b1 * p4->y + c1;

    // Check signs of r3 and r4.  If both point 3 and point 4 lie on
    // same side of line 1, the line segments do not intersect.
    if ( r3 != 0 && r4 != 0 && SAME_SIGNS( r3, r4 )) {
        return ( DONT_INTERSECT );
    }    

    // Compute a2, b2, c2 

    a2 = p4->y - p3->y;
    b2 = p3->x - p4->x;
    c2 = p4->x * p3->y - p3->x * p4->y;

    // Compute r1 and r2 

    r1 = a2 * p1->x + b2 * p1->y + c2;
    r2 = a2 * p2->x + b2 * p2->y + c2;

    // Check signs of r1 and r2.  If both point 1 and point 2 lie
    // on same side of second line segment, the line segments do
    // not intersect.
    if ( r1 != 0 && r2 != 0 && SAME_SIGNS( r1, r2 )) {
        return ( DONT_INTERSECT );
    }
    
    // Line segments intersect: compute intersection point. 
    denom = a1 * b2 - a2 * b1;
    
    if ( denom == 0 ) {
        return ( COLLINEAR );
    }    
    offset = denom < 0 ? - denom / 2 : denom / 2;

    // The denom/2 is to get rounding instead of truncating.  It
    // is added or subtracted to the numerator, depending upon the
    // sign of the numerator.
    num = b1 * c2 - b2 * c1;
    intersection->x = (short)(( num < 0 ? num - offset : num + offset ) / denom);

    num = a2 * c1 - a1 * c2;
    intersection->y = (short)(( num < 0 ? num - offset : num + offset ) / denom);

    return ( DO_INTERSECT );

} 


/*----------------------------------------------------------------------
Function name:  SimpleConvexCheck

Description:    This routine will process an array of point
                structures and determine if the polygon they
                describe is simple and convex.  If it is, the
                orientation of the vertices will be returned.
                If it is complex or concave, it will return zero.   
Information:    
  We must compare the cross products of each of the 
  consecutive pairs of line segments -- for a convex polygon they
  should all have the same sign.                      
  We must also check to see if the polygon is complex.  It can be
  convex and still be self-crossing, as below:
                                                                                  
                            /\     /\  
                           /  \   /  \ 
                          /    \ /    \
                         /      X      \
                        /      / \      \
                       /      /   \      /
                      /      /     /    /
                     /       \    /    /   
                    /         \  /    /    
                    \          \/    /      
                     \              /        
                      \____________/
                        
                      
  Our concavity test checks to make sure that the polygon, at all 
  vertices, makes ALL right turns or ALL left turns.  Here, all 
  turns are in the same direction, but the polygon is self crossing
  and therefore complex.  We can check for changes in the sign of
  dX and make sure that it does not change more than two times.
  if it does, the polygon is complex and we identify it as such.

Return:         LONG    Result of XProduct call or NOTSIMPLECONVEX.
----------------------------------------------------------------------*/
LONG
SimpleConvexCheck 
(
    LPPOINT       lpDstPoints,
    int           NumVtxs
)
{    
    int     i;
    LONG    direction;
    LONG    DirectionChanges;

    DEBUG_FIX

    // Set up for concavity check
    // Get cross product of first two line segments of polygon
    direction = XProduct((LPPOINT)(&lpDstPoints[0]),
                         (LPPOINT)(&lpDstPoints[1]),
                         (LPPOINT)(&lpDstPoints[2]));
                         
    // Set up for self-crossing check
    if (SAME_SIGNS((lpDstPoints[1].x-lpDstPoints[0].x),
                   (lpDstPoints[2].x-lpDstPoints[1].x))) {
        DirectionChanges = 0;
    } else {
        DirectionChanges = 1;
    }
    
          
    
    
    for (i=1; i < NumVtxs; i++) {
    
        if (direction != (XProduct((LPPOINT)(&lpDstPoints[i]),
                                   (LPPOINT)(&lpDstPoints[(i+1)%NumVtxs]),
                                   (LPPOINT)(&lpDstPoints[(i+2)%NumVtxs])))) {
            return(NOTSIMPLECONVEX);                // Fail
        }
        
        if (!SAME_SIGNS((lpDstPoints[(i+1)%NumVtxs].x-lpDstPoints[i].x),
               (lpDstPoints[(i+2)%NumVtxs].x-lpDstPoints[(i+1)%NumVtxs].x))) {
           
            DirectionChanges++;
        }
        if (DirectionChanges > 2) {                 // Simple pgon has only two
            return(NOTSIMPLECONVEX);                // No use checking rest...
        }    
    }
    
    // Pathologically, all points could be collinear, but it just so happens
    // that if this comes up, direction will be zero, which is the failure
    // case.  Cool, huh?
    
    return(direction);
                             
}        


/*----------------------------------------------------------------------
Function name:  FindBoundRect

Description:    This routine is used by many to find bounding
                rectangle to exclude a sw cursor.  lpPoints
                is nVerts deep so we need to find the MIN, MAX
                x's and y's.
Information:    

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
#define MIN(A,B) (((A) < (B)) ? (A) : (B))
#define MAX(A,B) (((A) > (B)) ? (A) : (B))

int FindBoundRect(LPRECT lpRect, LPPOINT lpPoints, int nVerts)
{
   int i;
   
   DEBUG_FIX;
 
   lpRect->right = lpRect->left = lpPoints->x;
   lpRect->top = lpRect->bottom = lpPoints->y;

   for (i=1; i<nVerts; i++)
      {
      lpRect->left = MIN(lpRect->left, lpPoints[i].x);
      lpRect->right = MAX(lpRect->right, lpPoints[i].x);
      lpRect->top = MIN(lpRect->top, lpPoints[i].y);
      lpRect->bottom = MAX(lpRect->bottom, lpPoints[i].y);
      }

   return 1;
}


/*----------------------------------------------------------------------
Function name:  FindBoundRect2

Description:    This routine is used by hwOsScanline to find
                bounding rectangle to exclude a sw cursor.
                lpPoints is nVerts-1 of x coordinates and
                1 y coordinate so we need to find the MIN/MAX x's.
Information:    

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
int FindBoundRect2(LPRECT lpRect, LPPOINT lpPoints, int nVerts)
{
   int i;
   
   DEBUG_FIX;
 
   lpRect->right = lpRect->left = lpPoints[1].x;
   lpRect->top = lpRect->bottom = lpPoints->y;

   for (i=1; i<nVerts; i++)
      {
      lpRect->left = MIN(lpRect->left, lpPoints[i].x);
      lpRect->left = MIN(lpRect->left, lpPoints[i].y);
      lpRect->right = MAX(lpRect->right, lpPoints[i].x);
      lpRect->right = MAX(lpRect->right, lpPoints[i].y);
      }

   return 1;
}


/*----------------------------------------------------------------------
Function name:  FindBoundRect3

Description:    This routine is used by hwOsPolyScanline to find
                bounding rectangle to exclude a sw cursor.
                lpPoints is nVerts of lpScan so we need to find
                the MIN, MAX x's and y's.
Information:    

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
int FindBoundRect3(LPRECT lpRect, LPPOINT lpPoints, int nVerts)
{
   LPSCAN lpScan;
   int i;
   int j;
   SHORT * lpPntX;
   SHORT xVerts;
   
   DEBUG_FIX;
 
   lpScan = (SCAN *)lpPoints;
   lpRect->top = lpScan->scnPntTop;
   lpRect->bottom = lpScan->scnPntBottom - 1;

   lpRect->right = 0;
   lpRect->left = 0x7FFF;
   for (i=0; i<nVerts; i++)
      {
      lpRect->top = MIN(lpScan->scnPntTop, lpRect->top);
      lpRect->bottom = MAX(lpScan->scnPntBottom - 1, lpRect->bottom);
      lpPntX = (SHORT *)lpScan->scnPntX;
      xVerts = lpScan->scnPntCnt;
      for (j=0; j<xVerts; j++)
         {
         lpRect->left = MIN(lpRect->left, *lpPntX);
         lpRect->right = MAX(lpRect->right, *lpPntX);
         lpPntX++;
         }
      lpPntX++;
      lpScan = (SCAN *)lpPntX;
      }

   return 1;
}
