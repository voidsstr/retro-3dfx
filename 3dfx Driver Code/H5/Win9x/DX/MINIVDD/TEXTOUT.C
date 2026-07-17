/* -*-c++-*- */
/* $Header: textout.c, 2, 10/11/00 8:55:14 PM, Brent$ */
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
** File name:   textout.c
**
** Description: the 32-bit GDI ExtTextOut and supporting function.
**
** $Revision: 2$
** $Date: 10/11/00 8:55:14 PM$
**
** $History: textout.c $
** 
** *****************  Version 28  *****************
** User: Stb_mmcclure Date: 3/12/99    Time: 12:16p
** Updated in $/devel/h3/win95/dx/minivdd
** Fix for PRS #4945 signed short is overloading then sign extended to an
** integer. Convert to an integer so 32 bit integer subtraction will
** occur. Removed fixes for PRS 2486, 1929, and 1623. Now fixed with this
** code modification.
** 
** *****************  Version 27  *****************
** User: Michael      Date: 1/15/99    Time: 8:40a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 26  *****************
** User: Michael      Date: 9/11/98    Time: 4:39p
** Updated in $/devel/h3/Win95/dx/minivdd
** FredW's (igx) fix for PRS #2486.  In notepad we have found a case where
** the opqaue rect passed is fff7 0130 7fff 013f. Once we compute the
** width, it is greater than 32767, and this cannot be sent to the
** hardware! (its a negative 16 bit integer). So clamp height and width to
** 32k maximum.
** 
** *****************  Version 25  *****************
** User: Michael      Date: 6/10/98    Time: 3:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** Circumvent HW bug that doesn't allow drawing spans of more than 16k
** bytes at 32-bpp.  Fixes #1623 and #1929.
** 
** *****************  Version 24  *****************
** User: Michael      Date: 5/15/98    Time: 4:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Backed out my previous changes.
** 
** *****************  Version 23  *****************
** User: Michael      Date: 5/15/98    Time: 9:31a
** Updated in $/devel/h3/Win95/dx/minivdd
** Add lpDst to FXPUNT.  Perform FLATTEN_POINTER prior to calling FXPUNT.
** 
** *****************  Version 22  *****************
** User: Ken          Date: 5/14/98    Time: 8:31a
** Updated in $/devel/h3/win95/dx/minivdd
** added a PERFNOP case to gdi32.   change the "undef" to a "define"
** in fifomgr.h, and enable in softice (doperfnop = 1).  Replaces every
** write
** to the command register with a "0"
** 
** *****************  Version 21  *****************
** User: Artg         Date: 5/05/98    Time: 9:09a
** Updated in $/devel/h3/Win95/dx/minivdd
** added opaque rect to the cursor exclude region
** 
** 
** *****************  Version 20  *****************
** User: Artg         Date: 5/01/98    Time: 4:25p
** Updated in $/devel/h3/Win95/dx/minivdd
** sw cursor stuff
** 
** *****************  Version 19  *****************
** User: Ken          Date: 4/27/98    Time: 5:35p
** Updated in $/devel/h3/win95/dx/minivdd
** complete (well, almost) enter/leave implementation for gdi32
** 
** *****************  Version 18  *****************
** User: Michael      Date: 4/18/98    Time: 11:01a
** Updated in $/devel/h3/Win95/dx/minivdd
** More work to make gdi work in tiled mode.
** 
** *****************  Version 17  *****************
** User: Michael      Date: 4/16/98    Time: 8:14p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add code to handle drawing when in tiled mode.  Fix bug of menus
** incorrectly working in full-screen DD/D3D apps.
** 
** *****************  Version 16  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#include "h3.h"
#include "thunk32.h"

#include "h3g.h"
#include "textout.h"

#include "entrleav.h"


/*----------------------------------------------------------------------
Function name:  assert_

Description:    This function is called on an ASSERT() failure.
                It prints out the assertion failure, and then
                traps to the debugger so that the failure can be seen.
                
Information:    Surrounded by #if ASSERT_FLAG

Return:         VOID
----------------------------------------------------------------------*/
#if ASSERT_FLAG
void assert_(char *exp, char *filename, unsigned linenumber)
{
    DEBUG_FIX;

    /* dpf("ASSERT failed: %s (%s:%d)\n", exp, filename, linenumber); */
    DEBUG_BREAK;
}

#endif /* ASSERT_FLAG */


/*----------------------------------------------------------------------
Function name:  draw_text

Description:    Draw text to screen. The only structure that
                draw_text() deals with is NewFontSeg. Everything
                else has been digested already. This is an effort
                to assist the compilers optimizer, because this *is*
                the inner loop for character imaging.
Information:    

Return:         VOID
----------------------------------------------------------------------*/
static void 
draw_text(int         wDestXOrg,
	  int         wDestYOrg,
	  char       *lpString,
	  int         wCount,
	  NewFontSeg *lpFont,
	  short      *lpCharWidths,
	  int         dbcs,
	  int         clip_left,
	  int         clip_right)
{
    char *glyph;
    DWORD l;
#   define FONTSEGT(offset, t) ((t)((char *)lpFont + lpFont->offset))
#   define FONTSEG(offset) ((char *)lpFont + offset)
    int org_x, org_y, width, height;
    int x, y, i, glyph_stride, glyph_dwords;
    int nchars, glyph_index;
    CMDFIFO_PROLOG(cmdFifo);

    DEBUG_FIX;

    CMDFIFO_SETUP(cmdFifo);

    for ( nchars = 0; nchars < wCount; ++nchars )
    {
        /* Fetch the glyph index */
        if ( dbcs )
            glyph_index = ((WORD *)lpString)[nchars];
        else
            glyph_index = ((unsigned char *)lpString)[nchars];

        /* If the glyph offset is larger than the number of glyphs in the font,
         * then skip this character.
         */
        //PARANOID( glyph_index < lpFont->nfNumGlyphs, continue );

        /* Get glyph information */
        if ( lpFont->nfFormat & NF_LARGE )
        {
            l = FONTSEGT(nfGlyphOffset, DWORD *)[glyph_index];
            glyph = FONTSEG(l);
            org_x  = (signed short)  ((LARGEROWGLYPH *)glyph)->lrgOrgX;
            org_y  = (signed short)  ((LARGEROWGLYPH *)glyph)->lrgOrgY;
            width  = (unsigned short)((LARGEROWGLYPH *)glyph)->lrgWidth;
            height = (unsigned short)((LARGEROWGLYPH *)glyph)->lrgHeight;
            glyph += sizeof(LARGEROWGLYPH);
            ASSERT_AT_COMPILE( sizeof(LARGEROWGLYPH) == 8 );
        }
        else
        {
            l = FONTSEGT(nfGlyphOffset, WORD *)[glyph_index];
            glyph = FONTSEG(l);
            org_x  = (signed char)  (((SMALLROWGLYPH *)glyph)->srgOrgX);
            org_y  = (signed char)  (((SMALLROWGLYPH *)glyph)->srgOrgY);
            width  = (unsigned char)(((SMALLROWGLYPH *)glyph)->srgWidth);
            height = (unsigned char)(((SMALLROWGLYPH *)glyph)->srgHeight);
            glyph += sizeof(SMALLROWGLYPH);
            ASSERT_AT_COMPILE( sizeof(SMALLROWGLYPH) == 4 );
        }

        /* Position character */
        x = wDestXOrg + org_x;
        y = wDestYOrg - org_y;

        /* Don't draw an empty glyph, or a completely clipped glyph. Note that
         * this code is conservative. We could speed things up a bit by
         * assuming that if a right clip occurs in a string then the rest of
         * the string can be ignored. But, there is a possibility that a very
         * string TrueType font could have a character that images WAY over to
         * the left.
         */
        if ( ((height != 0) && (width != 0)) &&
             ((x + width) >= clip_left) &&
             (x < clip_right) )
        {
            /* Compute number of bytes required by glyph row. 0 is not a valid
             * width, so this works.
             */
            glyph_stride = (width + 7) >> 3;

            /* Compute number of dwords to transfer. Because 0 is not a valid
             * height, at least 1 dword will be transfered.
             */
            glyph_dwords = ((glyph_stride * height) + 3) >> 2;

            /* Set up to transfer bitmap */
            CMDFIFO_CHECKROOM(cmdFifo, (unsigned)(glyph_dwords + 5));
            SETPH(cmdFifo, SSTCP_PKT2 | dstSizeBit | dstXYBit | commandBit);
            SET(cmdFifo, _FF(lpGRegs)->dstSize, R32(height, width));
            SET(cmdFifo, _FF(lpGRegs)->dstXY, R32(y, x));
            SETC(cmdFifo, _FF(lpGRegs)->command, (SSTG_ROP_SRCCOPY |
						 SSTG_CLIPSELECT |
						 SSTG_TRANSPARENT |
						 SSTG_HOST_BLT));
            /* Transfer host data (glyph bitmap) */
            SETPH(cmdFifo, (SSTCP_PKT1 |
			    SSTCP_PKT1_2D |
			    (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) |
			    (glyph_dwords << SSTCP_PKT1_NWORDS_SHIFT)));
	    
            for ( i = 1; i <= glyph_dwords; ++i )
            {
                l = *(DWORD *)glyph;
                glyph += 4;
                SET(cmdFifo, _FF(lpGRegs)->launch[0], l);
	    }
            BUMP(glyph_dwords + 5);
        }

        /* Compute escapement to next glyph */
        if ( lpCharWidths != NULL )
            wDestXOrg += (short)lpCharWidths[nchars];
        else
            wDestXOrg += FONTSEGT(nfAWTable, short *)[glyph_index];
    }

    CMDFIFO_EPILOG(cmdFifo);
}


#define END_OF_RECT_LIST(x) ((((DWORD *)x)[0] == 0) && \
			     (((DWORD *)x)[1] == 0))


/*----------------------------------------------------------------------
Function name:  FxExtTextOut

Description:    32-bit side of GDI's ExtTextOut.

Information:    This function *really* returns a DWORD, but define
                as BOOL to keep thunk32.c unchanged.

Return:         BOOL
----------------------------------------------------------------------*/
BOOL 
FxExtTextOut(ExtTextOutParams *lp)
/*
  LPPDEVICE       lpDestDev,
  WORD            wDestXOrg,
  WORD            wDestYOrg,
  LPRECT          lpClipRect,
  LPSTR           lpString,
  int             wCount,
  LPFONTINFO      lpFontInfo,
  LPDRAWMODE      lpDrawmode,
  LPTEXTXFORM     lpTextXForm,
  LPSHORT         lpCharWidths,
  LPRECT          lpOpaqueRect,
  WORD            wOptions
  */
{
#   define WIN4X_FONT (ETO_BYTE_PACKED | ETO_BIT_PACKED)
#   define lpNewFontSeg ((NewFontSeg *)lpFontInfo)
#   define lpStringBoundingBox ((RECT *)lpTextXForm)
    
    DEFINE_FLAT_POINTER(DIBENGINE, lpDst);
    DEFINE_FLAT_POINTER(RECT,      lpClipRect);
    DEFINE_FLAT_POINTER(char,      lpString);
    DEFINE_FLAT_POINTER(FONTINFO,  lpFontInfo);
    DEFINE_FLAT_POINTER(RECT,      lpOpaqueRect);
    DEFINE_FLAT_POINTER(RECT,      lpTextXForm);
    DEFINE_FLAT_POINTER(DRAWMODE,  lpDrawMode);
    DEFINE_FLAT_POINTER(short,     lpCharWidths);
    int height, width;
    CMDFIFO_PROLOG(cf);

    DEBUG_FIX;

    if (lp->wCount < 0)
    {
        FXPUNT(FXPUNT_NORMAL);
    }
    else if (lp->wCount > 0)
    {
        /* We only want new fonts, only byte packed, and not antialiased */
        if ( !(lp->wOptions & WIN4X_FONT) ||
	     (lp->wOptions & ETO_BIT_PACKED) ||
	     (lp->wOptions & ETO_LEVEL_MODE) )
        {
            FXPUNT(FXPUNT_EXTTEXTEXT);
        }
    }

    /* Don't convert lpDst until obvious punts are checked. Conversion
     * is expensive, so avoid if we can.
     */
    FLATTEN_POINTER(lpDst);
    FLATTEN_POINTER(lpTextXForm);
    FLATTEN_POINTER(lpClipRect);
    FLATTEN_POINTER(lpOpaqueRect);
    FLATTEN_POINTER(lpDrawMode);

    // XXX FIXME
    // calculate bounding box for cursor exclusion
    //
    FXENTER("FxExtTextOut", lpDst->deFlags, (hwAll && hwText),
	    cf, lpDst, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
	    FXENTER_RECT, lpTextXForm->left,lpTextXForm->top, 
        lpTextXForm->right, lpTextXForm->bottom,
	    FXENTER_RECT, lpOpaqueRect->left, lpOpaqueRect->top, 
        lpOpaqueRect->right, lpOpaqueRect->bottom  );

    /* Set clip rectangle */
    CMDFIFO_CHECKROOM(cf, 3);
    SETPH(cf, (SSTCP_PKT2 |
	       clip1minBit |
	       clip1maxBit));
    SET(cf, _FF(lpGRegs)->clip1min, RECT_TOP_LEFT(lpClipRect));
    SET(cf, _FF(lpGRegs)->clip1max, RECT_BOTTOM_RIGHT(lpClipRect));
    BUMP(3);

    /* Draw opaqueing rectangle list. It is possible to factor out the
     * setting of colorFore, but it isn't advisable, since most all
     * calls are for a single rectangle.
     */


    if ( lpOpaqueRect != NULL )
	for (; !END_OF_RECT_LIST(lpOpaqueRect); ++lpOpaqueRect )
	{
		// STB-MDM
		// PRS #4945 Signed short is overloading then sign extended to an integer.
		// Convert to an integer so 32 bit integer subtraction will occur.
		// Removed fixes for PRS 2486, 1929, and 1623.  Now fixed with this
		// code modification.
	    height = ((int)lpOpaqueRect->bottom) - lpOpaqueRect->top;
	    width = ((int)lpOpaqueRect->right) - lpOpaqueRect->left;

		width = min ( width, 0x0fff );

	    /* PRS 2486 - In notepad we have found a case where
	     * the opqaue rect passed is fff7 0130 7fff 013f. Once
	     * we compute the width, it is greater than 32767, and
	     * this cannot be sent to the hardware! (its a negative
	     * 16 bit integer). So clamp height and width to 32k
	     * maximum.
	     */
/*	    height = min(height, 0x7fff);
	    width = min(width, 0x7fff);

        // Circumvent HW bug that doesn't allow drawing spans
        // of more than 16k bytes at 32-bpp (#1623 and #1929).
        if ((_FF(bpp) == 32) && (width > 0x1000))
            width = 0x1000;
*/
	    if ((height > 0) && (width > 0))
	    {
		CMDFIFO_CHECKROOM(cf, 5);
		SETPH(cf, (SSTCP_PKT2 | colorForeBit | dstSizeBit |
			   dstXYBit | commandBit));
		SET(cf, _FF(lpGRegs)->colorFore, lpDrawMode->bkColor);
		SET(cf, _FF(lpGRegs)->dstSize, R32(height, width));
		SET(cf, _FF(lpGRegs)->dstXY, RECT_TOP_LEFT(lpOpaqueRect));
		SETC(cf, _FF(lpGRegs)->command, (SSTG_ROP_SRCCOPY |
						SSTG_CLIPSELECT |
						SSTG_GO | SSTG_RECTFILL));
		BUMP(5);
	    }
	}

    /* lp->wCount > 0, we have characters to draw */
    if ( lp->wCount > 0 )
    {
	FLATTEN_POINTER(lpString);
	lp->lpFontInfo &= 0xFFFF0000;  /* Strip offset from pointer */
	FLATTEN_POINTER(lpFontInfo);
	FLATTEN_POINTER(lpCharWidths);

	/* Program in foreground color. Because we are always transparent,
	 * we don't need to set background color. We also set the source
	 * format (monochrome), packing and initial rotation (0).
	 */
	CMDFIFO_CHECKROOM(cf, 4);
	SETPH(cf, SSTCP_PKT2 | srcFormatBit | srcXYBit | colorForeBit);
	SET(cf, _FF(lpGRegs)->srcFormat, (SSTG_PIXFMT_1BPP |
					  SSTG_SRC_PACK_8));
	SET(cf, _FF(lpGRegs)->srcXY, R32(0, 0));
	SET(cf, _FF(lpGRegs)->colorFore, lpDrawMode->TextColor);
	BUMP(4);

	/* Unleash the draw */
	CMDFIFO_SAVE(cf);
	draw_text((signed short)lp->wDestXOrg,   /* base of string */
		  (signed short)lp->wDestYOrg,
		  lpString,                         /* string */
		  lp->wCount,
		  lpNewFontSeg,                     /* font */
		  lpCharWidths,                     /* escapements */
		  lp->wOptions & ETO_GLYPH_INDEX,       /* dbcs */
		  (signed short)lpClipRect->left,   /* clip edges */
		  (signed short)lpClipRect->right);
	CMDFIFO_RELOAD(cf);
    }
	
    FXLEAVE("FxExtTextOut", cf, lpDst);
}

