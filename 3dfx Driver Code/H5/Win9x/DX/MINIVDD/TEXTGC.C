/* -*-c++-*- */
/* $Header: textgc.c, 2, 10/11/00 8:55:13 PM, Brent$ */
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
** File name:   textgc.c
**
** Description: Support for text glyph caching.
**              THIS FILE IS NOT USED!
**
** $Revision: 2$
** $Date: 10/11/00 8:55:13 PM$
**
** $History: textgc.c $
** 
** *****************  Version 4  *****************
** User: Michael      Date: 1/15/99    Time: 7:02a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 3  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/


t/*****************************************************************************
 *                                                                           *
 * textgc.c                                                                  *
 *                                                                           *
 * Written by F. Weigel (Intelligraphics) for 3Dfx                           *
 *                                                                           *
 * Text glyph cache                                                          *
 *                                                                           *
 *****************************************************************************/


#include "h3.h"
#include "thunk32.h"

#include "textout.h"


/* Font caching - max characters in cache */
#define MAX_CHARS_CACHED 8000


/* Dictionary is statically allocated for now. We should do this dynamically
 * at startup.
 */
static char fdict[sizeof(DICTIONARY) + sizeof(DICTIONARY_ENTRY) * MAX_CHARS_CACHED];
static DICTIONARY *font_dictionary = (DICTIONARY *)&fdict;
static int chars_cached;
static DWORD fcache_pos;
static int cache_remaining;


/* Initialize the cache. */
void initialize_cache(void)
    {
    DEBUG_FIX

    initialize_dictionary(font_dictionary, MAX_CHARS_CACHED);
    fcache_pos = lpDriverData->FontCacheAddr;
    cache_remaining = lpDriverData->FontCacheSize;
    chars_cached = 0;
    }


/* Flush the cache. */
void flush_cache(void)
    {
    DEBUG_FIX

    initialize_cache();
    }


/* Look up character in cache. Returns NULL, or fcache_pos */
DWORD lookup_cache(int key)
    {
    TAKE_ADDRESS long val;

    DEBUG_FIX

    if ( member_of_dictionary(font_dictionary, key, &val) )
        return (DWORD)val;
    else
        return 0;
    }


/* Attempt to put character into cache. Return 1 if ok, 0 if fail */
int cache_char(int key, char *glyph, int glyph_bytes, int height, int width)
    {
    DEBUG_FIX

    /* Don't cache if the character glyph would occupy 10% or more of
     * the off-screen memory area.
     */
    if ( glyph_bytes >= (int)(lpDriverData->FontCacheSize / 10) )
        return 0;

    /* If this glyph won't fit, then simply flush the cache (we could
     * maybe do something more sophisticated, but that wouldn't help
     * performance in WinBench!)
     */
    if ( glyph_bytes >= cache_remaining )
        flush_cache();

    /* If the cache loading factor is greater than 50%, flush cache
     * (what we are after is to ditch the cache if the collision
     * statistic becomes too high... but I made that optional, so
     * we assume that if the cache load gets too high, the collisions
     * will be too high. Tune this for WinBench!)
     */
    if ( chars_cached >= (MAX_CHARS_CACHED / 2) )
        flush_cache();

    /* Put the new (key, position) n-tuple into the dictionary */
    insert_into_dictionary(font_dictionary, key, fcache_pos);

    fcache_pos += glyph_bytes;

    return 1;
    }


/* Draw character to offscreen
 *
 * Draw to a linear mono map, with no clipping, given linear address
 * Source is mono, byte packed, opaque.
 */

#if 0

static void draw_to_offscreen(
        int         wDestXOrg,
        int         wDestYOrg,
        RECT       *lpClipRect,
        char       *lpString,
        int         wCount,
        NewFontSeg *lpFont,
        DRAWMODE   *lpDrawMode,
        short      *lpCharWidths,
        int         wOptions)
    {
    char *glyph;
    DWORD l;
#   define FONTSEGT(offset, t) ((t)((char *)lpFont + lpFont->offset))
#   define FONTSEG(offset) ((char *)lpFont + offset)
    int org_x, org_y, width, height;
    int x, y, i, glyph_stride, glyph_dwords;
    int nchars, glyph_index;
    int opaque, max_right, clip_left, clip_right;

    CMDFIFO_PROLOG(cmdFifo)

    DEBUG_FIX

    CMDFIFO_SETUP(cmdFifo);

    /* Program in clip rectangle, foreground and background colors */
    CMDFIFO_CHECKROOM(cmdFifo, 6);
    SETPH(cmdFifo, SSTCP_PKT2 | clip1minBit |
                                clip1maxBit |
                                srcXYBit |
                                colorBackBit |
                                colorForeBit);
    SET(cmdFifo, _FF(lpGRegs)->clip1min, R32(lpClipRect->top, lpClipRect->left));
    SET(cmdFifo, _FF(lpGRegs)->clip1max, R32(lpClipRect->bottom, lpClipRect->right));
    SET(cmdFifo, _FF(lpGRegs)->srcXY, 0L);
    SET(cmdFifo, _FF(lpGRegs)->colorBack, lpDrawMode->bkColor);
    SET(cmdFifo, _FF(lpGRegs)->colorFore, lpDrawMode->TextColor);
    BUMP(6);

    /* extract left and right for fast clipping */
    clip_left = (short)lpClipRect->left;
    clip_right = (short)lpClipRect->right;

    /* maximum right, for detecting overlaps */
    max_right = -32768;

    for ( nchars = 0; nchars < wCount; ++nchars )
        {
        /* Is this character possible in opaque mode? */
        opaque = (lpDrawMode->bkMode == OPAQUE);

        /* fetch the glyph index */
        if ( wOptions & ETO_GLYPH_INDEX )
            glyph_index = ((WORD *)lpString)[nchars];
        else
            glyph_index = ((unsigned char *)lpString)[nchars];

        /* if the glyph offset is larger than the number of glyphs in the
         * font, then skip this character.
         */
        if ( glyph_index >= lpFont->nfNumGlyphs )
            continue;

        /* either word table or dword table of glyph pointers */
        if ( lpFont->nfFormat & NF_LARGE )
            l = FONTSEGT(nfGlyphOffset, DWORD *)[glyph_index];
        else
            l = FONTSEGT(nfGlyphOffset, WORD *)[glyph_index];
        glyph = FONTSEG(l);

        /* extract information from header of glyph. note that pixels
         * is only valid if bit packed font is present.
         *
         * org_x  - distance from glyph origin to left edge of glyph bitmap
         * org_y  - distance from glyph origin to top edge of glyph bitmap
         * width  - width of glyph bitmap in pixels
         * height - height of glyph bitmap in pixels
         */
        if ( lpFont->nfFormat & NF_LARGE )
            {
            org_x  = (signed short)  ((LARGEROWGLYPH *)glyph)->lrgOrgX;
            org_y  = (signed short)  ((LARGEROWGLYPH *)glyph)->lrgOrgY;
            width  = (unsigned short)((LARGEROWGLYPH *)glyph)->lrgWidth;
            height = (unsigned short)((LARGEROWGLYPH *)glyph)->lrgHeight;
            glyph += sizeof(LARGEROWGLYPH);
            ASSERT_AT_COMPILE(sizeof(LARGEROWGLYPH) == 8);
            }
        else
            {
            org_x  = (signed char)  (((SMALLROWGLYPH *)glyph)->srgOrgX);
            org_y  = (signed char)  (((SMALLROWGLYPH *)glyph)->srgOrgY);
            width  = (unsigned char)(((SMALLROWGLYPH *)glyph)->srgWidth);
            height = (unsigned char)(((SMALLROWGLYPH *)glyph)->srgHeight);
            glyph += sizeof(SMALLROWGLYPH);
            ASSERT_AT_COMPILE(sizeof(SMALLROWGLYPH) == 4);
            }

        /* position character */
        x = wDestXOrg;
        y = wDestYOrg;

        /* adjust position wrt glyph */
        x += org_x;
        y -= org_y;

        /* loose opaque if we overlap any previous characters */
        if ( x < max_right )
            opaque = 0;

        /* new right maximum? */
        if ( (x + width) > max_right )
            max_right = x + width;

        /* don't draw an empty glyph, or a glyph completely clipped */
        /* FIXME: WHY DOESN'T (x < clip_right) CASE WORK??? */
        if ( ((height != 0) && (width != 0)) &&
              ((x + width) >= clip_left) /* &&
              (x < clip_right) */ )
            {
            /* compute number of bytes required by glyph
             * row. 0 is not a valid width, so this works.
             */
            glyph_stride = (width + 7) >> 3;

            /* compute number of dwords to transfer. Because 0
             * is not a valid height, at least 1 dword will be
             * transferred.
             */
            glyph_dwords = ((glyph_stride * height) + 3) >> 2;

            /* Set up to transfer bitmap */

            CMDFIFO_CHECKROOM(cmdFifo, 6);
            SETPH(cmdFifo, SSTCP_PKT2 | srcFormatBit |
                                        srcSizeBit |
                                        dstSizeBit |
                                        dstXYBit |
                                        commandBit);
            SET(cmdFifo, _FF(lpGRegs)->srcFormat, SSTG_PIXFMT_1BPP |
                                             SSTG_SRC_PACK_8);
            SET(cmdFifo, _FF(lpGRegs)->srcSize, R32(height, width));
            SET(cmdFifo, _FF(lpGRegs)->dstSize, R32(height, width));
            SET(cmdFifo, _FF(lpGRegs)->dstXY, R32(y, x));
            SET(cmdFifo, _FF(lpGRegs)->command, ((SRCCOPY & 0xffff0000) << 8)  |
                                            SSTG_CLIPSELECT |
                                            (opaque ? 0 : SSTG_TRANSPARENT) |
                                            SSTG_HOST_BLT);
            BUMP(6);

            /* Transfer host data (glyph bitmap) */
            CMDFIFO_CHECKROOM(cmdFifo, glyph_dwords + 1);
            SETPH(cmdFifo, SSTCP_PKT1 | SSTCP_PKT1_2D |
                                        LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT |
                                        glyph_dwords << SSTCP_PKT1_NWORDS_SHIFT);

            for ( i = 1; i <= glyph_dwords; ++i )
                {
                l = *(DWORD *)glyph;
                glyph += 4;
                SET(cmdFifo, _FF(lpGRegs)->launch[0], l);
                }

            BUMP(glyph_dwords + 1);
            }

        /* compute escapement to next glyph */
        if ( lpCharWidths != NULL )
            wDestXOrg += (short)lpCharWidths[nchars];
        else
            wDestXOrg += FONTSEGT(nfAWTable, short *)[glyph_index];
        }

    CMDFIFO_EPILOG(cmdFifo);
    }
#endif
