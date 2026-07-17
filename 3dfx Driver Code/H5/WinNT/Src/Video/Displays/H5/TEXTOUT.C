/******************************Module*Header*******************************\
* Module Name: textout.c
*
* On every TextOut, GDI provides an array of 'GLYPHPOS' structures
* for every glyph to be drawn.  Each GLYPHPOS structure contains a
* glyph handle and a pointer to a monochrome bitmap that describes
* the glyph.  (Note that unlike Windows 3.1, which provides a column-
* major glyph bitmap, Windows NT always provides a row-major glyph
* bitmap.)  As such, there are three basic methods for drawing text
* with hardware acceleration:
*
* 1) Glyph caching -- Glyph bitmaps are cached by the accelerator
*       (probably in off-screen memory), and text is drawn by
*       referring the hardware to the cached glyph locations.
*
* 2) Glyph expansion -- Each individual glyph is colour-expanded
*       directly to the screen from the monochrome glyph bitmap
*       supplied by GDI.
*
* 3) Buffer expansion -- The CPU is used to draw all the glyphs into
*       a 1bpp monochrome bitmap, and the hardware is then used
*       to colour-expand the result.
*
* The fastest method depends on a number of variables, such as the
* colour expansion speed, bus speed, CPU speed, average glyph size,
* and average string length.
*
* For Banshee with normal sized glyphs, I've found that caching the
* glyphs in off-screen memory is typically the fastest method.
* Glyph expansion is slightly slower than caching, and buffer
* expansion is the slowest.
*
* Glyph expansion is typically faster than buffer expansion for very
* large glyphs, even on the ISA bus, because less copying by the CPU
* needs to be done.  Unfortunately, large glyphs are pretty rare.
*
* An advantange of the buffer expansion method is that opaque text will
* never flash -- the other two methods typically need to draw the
* opaquing rectangle before laying down the glyphs, which may cause
* a flash if the raster is caught at the wrong time.
*
* This driver implements the first two methods but glyph caching is
* preferred because it has been determined to be the fastest.
*
* Copyright (c) 1992-1996 Microsoft Corporation
* Copyright (c) 1997-1998 3Dfx Interactive, Inc.
*
\**************************************************************************/

#include "precomp.h"

RECTL grclMax = { 0, 0, 0x8000, 0x8000 };
                                // Maximal clip rectangle for trivial clipping

BYTE gajBit[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
                                // Converts bit index to set bit

#define     FIFTEEN_BITS        ((1 << 15)-1)

/******************************Public*Routine******************************\
* VOID vClipSolid
*
* Fills the specified rectangle with the specified colour, honouring
* the requested clipping.
*
\**************************************************************************/

VOID vH3ClipSolid(
PDEV*       ppdev,
RECTL*      prcl,
ULONG       iColor,
CLIPOBJ*    pco)
{
    BOOL            bMore;              // Flag for clip enumeration
    CLIPENUM        ce;                 // Clip enumeration object
    LONG            c;                  // Count of non-empty rectangles
    RBRUSH_COLOR    rbc;                // For passing colour to vFillSolid

    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_RIGHTDOWN, 0);

    // Scan through all the clip rectangles, looking for intersects
    // of fill areas with region rectangles:

    rbc.iSolidColor = iColor;

    do {
        // Get a batch of region rectangles:

        bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (VOID*) &ce);

        c = cIntersect(prcl, ce.arcl, ce.c);

        if (c != 0)
            ppdev->pfnFillSolid(ppdev, c, ce.arcl, 0xf0f0, rbc, NULL);

    } while (bMore);
}

/******************************Public*Routine******************************\
* VOID vExpandGlyph
*
\**************************************************************************/

VOID vExpandGlyph(
PDEV*   ppdev,
BYTE*   pj,             // Can be unaligned
LONG    lSkip,
LONG    cj,
LONG    cy)
{
    BYTE*               pjH3Base = ppdev->pjH3Base;
    LONG                cd;
    ULONG UNALIGNED*    pulSrc;
    LONG                i;
    ULONG               ul;

    GWH_DECL;

    GWH_PROLOG;

    cd     = cj >> 2;
    pulSrc = (ULONG UNALIGNED*) pj;

    // jdw - Can we optimize here?  If we never get a huge glyph we could
    //       pull the fifo size check and pack headers out of the loop.

    switch (cj & 3)
    {
    case 0:
        do {

            CHECK_FIFO_ROOM(ppdev, cd);
            GWH_BEGIN_PKT1_PACKET(cd , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

            for (i = cd; i > 0; i--)
            {
#if ENABLE_LOG_FILE
                SET_PKT1_REG(pjH3Base, launch, *pulSrc);
                pulSrc++;
#else
                SET_PKT1_REG(pjH3Base, launch, *pulSrc++);
#endif
            }

            GWH_END_PKT1_PACKET(cd);

            pulSrc = (ULONG UNALIGNED*) ((BYTE*) pulSrc + lSkip);

        } while (--cy > 0);
        break;

    case 1:
        do {

            CHECK_FIFO_ROOM(ppdev, cd + 1);
            GWH_BEGIN_PKT1_PACKET(cd + 1, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

            for (i = cd; i > 0; i--)
            {
#if ENABLE_LOG_FILE
                SET_PKT1_REG(pjH3Base, launch, *pulSrc);
                pulSrc++;
#else
                SET_PKT1_REG(pjH3Base, launch, *pulSrc++);
#endif
            }

            SET_PKT1_REG(pjH3Base, launch, (ULONG) (*(BYTE*) pulSrc));

            GWH_END_PKT1_PACKET(cd + 1);

            pulSrc = (ULONG UNALIGNED*) ((BYTE*) pulSrc + lSkip + 1);

        } while (--cy > 0);
        break;

    case 2:
        do {

            CHECK_FIFO_ROOM(ppdev, cd + 1);
            GWH_BEGIN_PKT1_PACKET(cd + 1, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

            for (i = cd; i > 0; i--)
            {
#if ENABLE_LOG_FILE
                SET_PKT1_REG(pjH3Base, launch, *pulSrc);
                pulSrc++;
#else
                SET_PKT1_REG(pjH3Base, launch, *pulSrc++);
#endif
            }

            SET_PKT1_REG(pjH3Base, launch, (ULONG) (*(WORD UNALIGNED *) pulSrc));

            GWH_END_PKT1_PACKET(cd + 1);

            pulSrc = (ULONG UNALIGNED*) ((BYTE*) pulSrc + lSkip + 2);

        } while (--cy > 0);
        break;

    case 3:
        do {

            CHECK_FIFO_ROOM(ppdev, cd + 1);
            GWH_BEGIN_PKT1_PACKET(cd + 1, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

            for (i = cd; i > 0; i--)
            {
#if ENABLE_LOG_FILE
                SET_PKT1_REG(pjH3Base, launch, *pulSrc);
                pulSrc++;
#else
                SET_PKT1_REG(pjH3Base, launch, *pulSrc++);
#endif
            }

            ul = *((WORD UNALIGNED *) pulSrc) | (*(((BYTE*) pulSrc) + 2) << 16);
            SET_PKT1_REG(pjH3Base, launch, ul);
            GWH_END_PKT1_PACKET(cd + 1);

            pulSrc = (ULONG UNALIGNED*) ((BYTE*) pulSrc + lSkip + 3);

        } while (--cy > 0);
        break;
    }

    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* VOID vH3GeneralText
*
\**************************************************************************/

VOID vH3GeneralText(
PDEV*     ppdev,
STROBJ*   pstro,
CLIPOBJ*  pco)
{
    BYTE*       pjH3Base = ppdev->pjH3Base;
    LONG        xOffset;
    LONG        yOffset;
    BYTE        iDComplexity;
    BOOL        bMoreGlyphs;
    ULONG       cGlyphOriginal;
    ULONG       cGlyph;
    GLYPHPOS*   pgpOriginal;
    GLYPHPOS*   pgp;
    GLYPHBITS*  pgb;
    POINTL      ptlOrigin;
    BOOL        bMore;
    CLIPENUM    ce;
    RECTL*      prclClip;
    ULONG       ulCharInc;
    LONG        cxGlyph;
    LONG        cyGlyph;
    LONG        cx;
    LONG        cy;
    LONG        xLeft;
    LONG        yTop;
    LONG        xRight;
    LONG        yBottom;
    LONG        lDelta;
    LONG        cj;
    BYTE*       pjGlyph;
    LONG        xAlign;
    BOOL        bClipSet;

    GWH_DECL;

    GWH_PROLOG;

    xOffset      = ppdev->xOffset;
    yOffset      = ppdev->yOffset;
    iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;
    bClipSet     = FALSE;

//    CHECK_FIFO_ROOM( ppdev, 1);
//    SET(1, pjH3Base, srcXY, 0);   // Zero for packed host blts.

    do {
      if (pstro->pgp != NULL)
      {
        // There's only the one batch of glyphs, so save ourselves
        // a call:

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      if (cGlyphOriginal > 0)
      {
        ulCharInc = pstro->ulCharInc;

        if (iDComplexity != DC_COMPLEX)
        {
            // We could call 'cEnumStart' and 'bEnum' when the clipping is
            // DC_RECT, but the last time I checked, those two calls took
            // more than 150 instructions to go through GDI.  Since
            // 'rclBounds' already contains the DC_RECT clip rectangle,
            // and since it's such a common case, we'll special case it:

            bMore = FALSE;
            ce.c  = 1;

            if (iDComplexity == DC_TRIVIAL)
                prclClip = &grclMax;
            else
                prclClip = &pco->rclBounds;

            goto SingleRectangle;
        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {
          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;
            pgb    = pgp->pgdf->pgb;

            ptlOrigin.x = pgb->ptlOrigin.x + pgp->ptl.x;
            ptlOrigin.y = pgb->ptlOrigin.y + pgp->ptl.y;

            // Loop through all the glyphs for this rectangle:

            while (TRUE)
            {
              cxGlyph = pgb->sizlBitmap.cx;
              cyGlyph = pgb->sizlBitmap.cy;
              pjGlyph = pgb->aj;

              if ((prclClip->left   <= ptlOrigin.x) &&
                  (prclClip->top    <= ptlOrigin.y) &&
                  (prclClip->right  >= ptlOrigin.x + cxGlyph) &&
                  (prclClip->bottom >= ptlOrigin.y + cyGlyph))
              {
                //-----------------------------------------------------
                // Unclipped glyph

                if (bClipSet)
                {
                    // A clipped glyph was just drawn.

                    // Clipping was at reset state on entry to this routine,
                    // so restore it to reset state.

                    bClipSet = FALSE;

                    CHECK_FIFO_ROOM(ppdev, 4);
                    GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_CLIP0MIN  |
                                           SSTCP_PKT2_DSTSIZE   |
                                           SSTCP_PKT2_DSTXY     |
                                           SSTCP_PKT2_COMMAND);

                    SET(1, pjH3Base, clip0min, 0);
                    SET(2, pjH3Base, dstSize, H3_PACKXY_FAST( cxGlyph, cyGlyph ));
                    SET(3, pjH3Base, dstXY, H3_PACKXY( ptlOrigin.x + xOffset,
                                                       ptlOrigin.y + yOffset) );
                    SET(4, pjH3Base, command, ( 0xcc << SSTG_ROP0_SHIFT) |
                                                        SSTG_TRANSPARENT |
                                                        SSTG_HOST_BLT);
                    GWH_END_2D_PACKET( 4 );
                }
                else
                {
                    CHECK_FIFO_ROOM(ppdev, 3);
                    GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_DSTSIZE   |
                                           SSTCP_PKT2_DSTXY     |
                                           SSTCP_PKT2_COMMAND);

                    SET(1, pjH3Base, dstSize, H3_PACKXY_FAST( cxGlyph, cyGlyph ));
                    SET(2, pjH3Base, dstXY, H3_PACKXY( ptlOrigin.x + xOffset,
                                                       ptlOrigin.y + yOffset) );
                    SET(3, pjH3Base, command, ( 0xcc << SSTG_ROP0_SHIFT) |
                                                        SSTG_TRANSPARENT |
                                                        SSTG_HOST_BLT);
                    GWH_END_2D_PACKET( 3 );
                }

                GWH_EPILOG;
                vExpandGlyph(ppdev, pjGlyph, 0, (cxGlyph + 7) >> 3, cyGlyph);
                GWH_PROLOG;
              }
              else
              {
                //-----------------------------------------------------
                // Clipped glyph

                // Find the intersection of the glyph rectangle
                // and the clip rectangle:

                xLeft   = max(prclClip->left,   ptlOrigin.x);
                yTop    = max(prclClip->top,    ptlOrigin.y);
                xRight  = min(prclClip->right,  ptlOrigin.x + cxGlyph);
                yBottom = min(prclClip->bottom, ptlOrigin.y + cyGlyph);

                // Check for trivial rejection:

                if (((cx = xRight - xLeft) > 0) &&
                    ((cy = yBottom - yTop) > 0))
                {
                    // We have to set the clipping rectangle.
                    CHECK_FIFO_ROOM(ppdev, 4);
                    GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_CLIP0MIN    |
                                           SSTCP_PKT2_DSTSIZE     |
                                           SSTCP_PKT2_DSTXY       |
                                           SSTCP_PKT2_COMMAND);

                    SET(1,  pjH3Base, clip0min, (xOffset + xLeft) & 0xffff);
                    bClipSet = TRUE;

                    // 'xAlign' is the bit position in the monochrome glyph
                    // bitmap of the first pixel to be lit, relative to
                    // the start of the byte.  That is, if 'xAlign' is 2,
                    // then the first unclipped pixel is represented by bit
                    // 2 of the corresponding bitmap byte.
                    //
                    // Normally, the accelerator expects bit 0 to be the
                    // first lit byte.  We use the scissors so that the
                    // first 'xAlign' bits of the byte will not be displayed.
                    //
                    // (What we're doing is simply aligning the monochrome
                    // blt using the hardware clipping.)

                    xAlign = (xLeft - ptlOrigin.x) & 0x7;
                    xLeft -= xAlign;
                    cx    += xAlign;

                    SET(2,  pjH3Base, dstSize, H3_PACKXY_FAST(cx, cy) );
                    SET(3,  pjH3Base, dstXY, H3_PACKXY( xLeft + xOffset,
                                                        yTop + yOffset) );
//                    SET(pjH3Base, command, ppdev->ulCmd);
                    SET(4, pjH3Base, command, ( 0xcc << SSTG_ROP0_SHIFT) |
                                                        SSTG_TRANSPARENT |
                                                        SSTG_HOST_BLT);

                    GWH_END_2D_PACKET( 4 );

                    // Send the bits to the launch area.
                    lDelta   = (cxGlyph + 7) >> 3;
                    pjGlyph += (yTop - ptlOrigin.y) * lDelta
                            + ((xLeft - ptlOrigin.x) >> 3);
                    cj = (cx + 7) >> 3;

                    GWH_EPILOG;
                    vExpandGlyph(ppdev, pjGlyph, lDelta - cj, cj, cy);
                    GWH_PROLOG;
                }
              }

              if (--cGlyph == 0)
                break;

              // Get ready for next glyph:

              pgp++;
              pgb = pgp->pgdf->pgb;

              if (ulCharInc == 0)
              {
                ptlOrigin.x = pgp->ptl.x + pgb->ptlOrigin.x;
                ptlOrigin.y = pgp->ptl.y + pgb->ptlOrigin.y;
              }
              else
              {
                ptlOrigin.x += ulCharInc;
              }
            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);

    if (bClipSet)
    {
        // Clear the clipping registers.
        CHECK_FIFO_ROOM(ppdev, 1);
        GWH_BEGIN_2D_PACKET( 1, SSTCP_PKT2_CLIP0MIN );
        SET(1,  pjH3Base, clip0min, 0);
        GWH_END_2D_PACKET( 1 );
    }
    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* CACHEDFONT* pcfAllocateCachedFont()
*
* Initializes our font data structure.
*
\**************************************************************************/

CACHEDFONT* pcfAllocateCachedFont(
PDEV*   ppdev)
{
    CACHEDFONT*     pcf;
    CACHEDGLYPH**   ppcg;
    LONG            i;

    pcf = ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(CACHEDFONT), ALLOC_TAG, &ppdev->cfSentinel.pcfNext);

    if (pcf != NULL)
    {
        // Insert this node into the doubly-linked cached-font list hanging
        // off the PDEV:

        pcf->pcfNext              = ppdev->cfSentinel.pcfNext;
#ifdef MEMCHECK
        if (! ((DWORD)ppdev->cfSentinel.pcfNext == (DWORD)&ppdev->cfSentinel.pcfNext))
          UPDATE_BLOCK_DATA(ppdev->cfSentinel.pcfNext, &pcf->pcfNext);
#endif
        pcf->pcfPrev              = &ppdev->cfSentinel;
        ppdev->cfSentinel.pcfNext = pcf;
        pcf->pcfNext->pcfPrev     = pcf;

        // Note that we rely on FL_ZERO_MEMORY to zero 'pgaChain' and
        // 'cjAlloc':

        pcf->cgSentinel.hg = HGLYPH_SENTINEL;

        // Initialize the hash table entries to all point to our sentinel:

        for (ppcg = &pcf->apcg[0], i = GLYPH_HASH_SIZE; i != 0; i--, ppcg++)
        {
            *ppcg = &pcf->cgSentinel;
        }
    }

    return(pcf);
}

/******************************Public*Routine******************************\
* VOID vFreeCachedFont()
*
* Frees all memory associated with the cache we kept for this font.
*
\**************************************************************************/

VOID vFreeCachedFont(
CACHEDFONT* pcf)
{
    GLYPHALLOC* pga;
    GLYPHALLOC* pgaNext;

    // Remove this node from our cached-font linked-list:

    pcf->pcfPrev->pcfNext = pcf->pcfNext;
    pcf->pcfNext->pcfPrev = pcf->pcfPrev;
#ifdef MEMCHECK
    // since we don't have access to the PDEV here, this is a hack
    // to get around the ASSERTs in the memcheck code
    // when we are freeing the next to last CACHEDFONT or the
    // last CACHEDFONT
    // skip the first UPDATE_BLOCK_DATA call
    // just zero out the dwData in the pcf BLOCKINFO struct
    if (pcf->pcfPrev->pcfNext != pcf->pcfPrev->pcfPrev)
    {
      UPDATE_BLOCK_DATA(pcf->pcfNext, &pcf->pcfPrev->pcfNext);
    }
    UPDATE_BLOCK_DATA(pcf, 0);
#endif

    // Free all glyph position allocations associated with this font:

    pga = pcf->pgaChain;
    while (pga != NULL)
    {
        pgaNext = pga->pgaNext;
#ifdef MEMCHECK
        if (NULL != pgaNext)
        {
          pcf->pgaChain = pgaNext;
          UPDATE_BLOCK_DATA(pgaNext, &pcf->pgaChain);
        }
        UPDATE_BLOCK_DATA(pga, 0);
#endif
        ENGFREEMEM(pga);
        pga = pgaNext;
    }

    ENGFREEMEM(pcf);
}

/******************************Public*Routine******************************\
* VOID vBlowGlyphCache()
*
\**************************************************************************/

VOID vBlowGlyphCache(
PDEV*   ppdev)
{
    CACHEDFONT*     pcfSentinel;
    CACHEDFONT*     pcf;
    GLYPHALLOC*     pga;
    GLYPHALLOC*     pgaNext;
    CACHEDGLYPH**   ppcg;
    LONG            i;

    ASSERTDD(ppdev->flStatus & STAT_GLYPH_CACHE, "No glyph cache to be blown");

    // Reset our current glyph variables:

    ppdev->ulGlyphCurrent = ppdev->ulGlyphStart;

    ///////////////////////////////////////////////////////////////////

    // Now invalidate all active cached fonts:

    pcfSentinel = &ppdev->cfSentinel;
    for (pcf = pcfSentinel->pcfNext; pcf != pcfSentinel; pcf = pcf->pcfNext)
    {
        // Reset all the hash table entries to point to the cached-font
        // sentinel.  This effectively resets the cache for this font:

        for (ppcg = &pcf->apcg[0], i = GLYPH_HASH_SIZE; i != 0; i--, ppcg++)
        {
            *ppcg = &pcf->cgSentinel;
        }

        // We may as well free all glyph position allocations for this font:

        pga = pcf->pgaChain;
        while (pga != NULL)
        {
            pgaNext = pga->pgaNext;
            ENGFREEMEM(pga);
            pga = pgaNext;
        }

        pcf->pgaChain = NULL;
        pcf->cjAlloc  = 0;
    }
}

/******************************Public*Routine******************************\
* VOID vTrimAndPackGlyph
*
\**************************************************************************/

VOID vTrimAndPackGlyph(
BYTE*   pjBuf,          // Note: Routine may touch preceding byte!
BYTE*   pjGlyph,
LONG*   pcxGlyph,
LONG*   pcyGlyph,
POINTL* pptlOrigin,
LONG*   pcj)
{
    LONG    cxGlyph;
    LONG    cyGlyph;
    POINTL  ptlOrigin;
    LONG    cAlign;
    LONG    lDelta;
    BYTE*   pj;
    BYTE    jBit;
    LONG    cjSrcWidth;
    LONG    lSrcSkip;
    LONG    lDstSkip;
    LONG    lDstDelta;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    i;
    LONG    j;
    BYTE    jSrc;

    ///////////////////////////////////////////////////////////////
    // Trim the glyph

    cyGlyph   = *pcyGlyph;
    cxGlyph   = *pcxGlyph;
    ptlOrigin = *pptlOrigin;
    cAlign    = 0;

    // let [x] denote the least integer greater than or equal to x
    // Set lDelta to be [cxGlyph/8]. This is the number of bytes occupied
    // by the pixels in the horizontal direction of the monochrome glyph.

    lDelta = (cxGlyph + 7) >> 3;

    // Trim off any zero rows at the bottom of the glyph:

    pj = pjGlyph + cyGlyph * lDelta;    // One past last byte in glyph
    while (cyGlyph > 0)
    {
        i = lDelta;
        do {
            if (*(--pj) != 0)
                goto Done_Bottom_Trim;
        } while (--i != 0);

        // The entire last row has no lit pixels, so simply skip it:

        cyGlyph--;
    }

    ASSERTDD(cyGlyph == 0, "cyGlyph should only be zero here");

    // We found a space character.  Set both dimensions to zero, so
    // that it's easy to special-case later:

    cxGlyph = 0;

Done_Bottom_Trim:

    // If cxGlyph != 0, we know that the glyph has at least one non-zero
    // row and column.  By exploiting this knowledge, we can simplify our
    // end-of-loop tests, because we don't have to check to see if we've
    // decremented either 'cyGlyph' or 'cxGlyph' to zero:

    if (cxGlyph != 0)
    {
        // Trim off any zero rows at the top of the glyph:

        pj = pjGlyph;                       // First byte in glyph
        while (TRUE)
        {
            i = lDelta;
            do {
                if (*(pj++) != 0)
                    goto Done_Top_Trim;
            } while (--i != 0);

            // The entire first row has no lit pixels, so simply skip it:

            cyGlyph--;
            ptlOrigin.y++;
            pjGlyph = pj;
        }

Done_Top_Trim:

        // Trim off any zero columns at the right edge of the glyph:

        while (TRUE)
        {
            j    = cxGlyph - 1;

            pj   = pjGlyph + (j >> 3);      // Last byte in first row of glyph
            jBit = gajBit[j & 0x7];
            i    = cyGlyph;

            do {
                if ((*pj & jBit) != 0)
                    goto Done_Right_Trim;

                pj += lDelta;
            } while (--i != 0);

            // The entire last column has no lit pixels, so simply skip it:

            cxGlyph--;
        }

Done_Right_Trim:

        // Trim off any zero columns at the left edge of the glyph:

        while (TRUE)
        {
            pj   = pjGlyph;                 // First byte in first row of glyph
            jBit = gajBit[cAlign];
            i    = cyGlyph;

            do {
                if ((*pj & jBit) != 0)
                    goto Done_Left_Trim;

                pj += lDelta;
            } while (--i != 0);

            // The entire first column has no lit pixels, so simply skip it:

            ptlOrigin.x++;
            cxGlyph--;
            cAlign++;
            if (cAlign >= 8)
            {
                cAlign = 0;
                pjGlyph++;
            }
        }
    }

Done_Left_Trim:

    ///////////////////////////////////////////////////////////////
    // Pack the glyph

    // byte count of cell size (trimmed width + blank left columns).
    cjSrcWidth  = (cxGlyph + cAlign + 7) >> 3;

    // difference between cell width and trimmed glyph width.
    lSrcSkip    = lDelta - cjSrcWidth;

    // trimed glyph width in bytes.
    lDstDelta   = (cxGlyph + 7) >> 3;   // Byte packed

    // The glyphs are 'byte-packed' (i.e., every scan is byte aligned)

//    if (ppdev->iBitmapFormat == BMF_24BPP)
//        lDstDelta = (lDstDelta + 3) & ~3;
//    else
//        lDstDelta = (lDstDelta + 1) & ~1;

    lDstSkip  = lDstDelta - cjSrcWidth;

    pjSrc     = pjGlyph;    // Start of trimmed glyph, not including empty left columns.
    pjDst     = pjBuf;

    // Zero the first byte of the buffer, because we're going to 'or' stuff
    // into it:

    *pjDst = 0;

    // cAlign used to indicate which bit in the first byte of the unpacked
    // glyph was the first non-zero pixel column.  Now, we flip it to
    // indicate which bit in the packed byte will receive the next non-zero
    // glyph bit:

    cAlign = (-cAlign) & 0x7;
    if (cAlign > 0)
    {
        // It would be bad if our trimming calculations were wrong, because
        // we assume any bits to the left of the 'cAlign' bit will be zero.
        // As a result of this decrement, we will 'or' those zero bits into
        // whatever byte precedes the glyph bits array:

        pjDst--;

        ASSERTDD((*pjSrc >> cAlign) == 0, "Trimmed off too many bits");
    }

    for (i = cyGlyph; i != 0; i--)
    {
        for (j = cjSrcWidth; j != 0; j--)
        {
            // Note that we may modify a byte past the end of our
            // destination buffer, which is why we reserved an
            // extra byte:
            jSrc = *pjSrc;
            *(pjDst)     |= (jSrc >> (cAlign));
            *(pjDst + 1)  = (jSrc << (8 - cAlign));
            pjSrc++;
            pjDst++;

        }
        pjSrc += lSrcSkip;
        pjDst += lDstSkip;
    }

    ///////////////////////////////////////////////////////////////
    // Return results

    *pcxGlyph   = cxGlyph;
    *pcyGlyph   = cyGlyph;
    *pptlOrigin = ptlOrigin;
    *pcj        = lDstDelta * cyGlyph;
}

/******************************Public*Routine******************************\
* BOOL bPutGlyphInCache
*
* Figures out where to be a glyph in off-screen memory, copies it
* there, and fills in any other data we'll need to display the glyph.
*
* This routine is rather device-specific, and will have to be extensively
* modified for other display adapters.
*
* Returns TRUE if successful; FALSE if there wasn't enough room in
* off-screen memory.
*
\**************************************************************************/

BOOL bPutGlyphInCache(
PDEV*           ppdev,
CACHEDGLYPH*    pcg,
GLYPHBITS*      pgb)
{
    BYTE*   pjH3Base = ppdev->pjH3Base;
    BYTE*   pjGlyph;
    LONG    cxGlyph;
    LONG    cyGlyph;
    POINTL  ptlOrigin;
    ULONG*  pulSrc;
    ULONG*  pulDst;
    LONG    i;
    LONG    cPels;
    ULONG   ulGlyphThis;
    ULONG   ulGlyphNext;
    ULONG   ul;
    ULONG   ulStart;
    ULONG   ulXyStart;
    LONG    cj;
    volatile ULONG   flush;
    BYTE    ajBuf[MAX_GLYPH_SIZE + 4];  // Leave room at end for scratch space
    GWH_DECL;

    pjGlyph   = pgb->aj;
    cyGlyph   = pgb->sizlBitmap.cy;
    cxGlyph   = pgb->sizlBitmap.cx;
    ptlOrigin = pgb->ptlOrigin;

    vTrimAndPackGlyph(&ajBuf[0], pjGlyph, &cxGlyph, &cyGlyph, &ptlOrigin, &cj);

    ASSERTDD(cj <= sizeof(ajBuf), "Overran end of temporary glyph storage");

    ///////////////////////////////////////////////////////////////
    // Find spot for glyph in off-screen memory

//    cPels       = cyGlyph * cxGlyph;            // Note that this may be zero
    ulGlyphThis = ppdev->ulGlyphCurrent;
    ulGlyphNext = ulGlyphThis + ((cj + 3) & ~3);   // Dword aligned

    if (ulGlyphNext >= ppdev->ulGlyphEnd)
    {
        // There's isn't enough free room in the off-screen cache for another
        // glyph.  Let the caller know that it should call 'vBlowGlyphCache'
        // to free up space.
        //
        // First, make sure that this glyph will fit in the cache when it's
        // empty, too:

        ASSERTDD(ppdev->ulGlyphStart + cj < ppdev->ulGlyphEnd,
            "Glyph can't fit in empty cache -- where's the higher-level check?");

        return(FALSE);
    }

    // Remember where the next glyph goes:

    ppdev->ulGlyphCurrent = ulGlyphNext;

    ///////////////////////////////////////////////////////////////
    // Initialize the glyph fields

    // Note that cxLessOne will be invalid for a 'space' character,
    // so the rendering routine had better watch for a height of zero:

    pcg->ptlOrigin     = ptlOrigin;
    pcg->cx            = cxGlyph;
    pcg->cy            = cyGlyph;
    pcg->cxcy          = H3_PACKXY_FAST(cxGlyph, cyGlyph);
    pcg->cd          = (cj + 3) >> 2;
    pcg->ulLinearStart = ulGlyphThis;

    ///////////////////////////////////////////////////////////////
    // Download the glyph

#ifdef H3_FIFO
    if ( pcg->cd > 0 )
    {
        // Copy the glyph to off-screen:

        GWH_PROLOG;

        pulSrc  = (ULONG *) ajBuf;

        // The glyph cache must be in linear memory or addition factors must be considered.

        CHECK_FIFO_ROOM( ppdev, pcg->cd + 1);    // Pkt5 takes 1 more header word.
        GWH_BEGIN_PKT5_PACKET(pcg->cd, ulGlyphThis );

        for (i = pcg->cd; i != 0; i--)
        {
#if ENABLE_LOG_FILE
            SET_ABSOLUTE( UNUSED, *pulSrc );
            pulSrc++;
#else
            SET_ABSOLUTE( UNUSED, *pulSrc++ );
#endif
        }

        GWH_END_PKT5_PACKET( pcg->cd );

        // Wait for LFB's before sending Blt.
        //    Fifo jump seems to always work. H3_GP_WAIT doesn't seem to always work.

#ifndef CSIM
        GWH_FIFO_JMP_PLUS4;     // Flush the LFB's out
#endif

        GWH_EPILOG;
  }
#else
    // Copy the glyph to off-screen:

    pulDst = (ULONG*) (ppdev->pjScreenBase + ulGlyphThis);

    pulSrc  = (ULONG *) ajBuf;

    START_DIRECT_ACCESS_H3(ppdev, pjH3Base);

    for (i = pcg->cd; i != 0; i--)
    {
        *pulDst++ = *pulSrc++;
    }

    END_DIRECT_ACCESS_H3(ppdev, pjH3Base);

    P6_FENCE;   // They don't get out in time without a flush.

    flush = *--pulDst;  // HW needs a read to stall pipline until writes are complete
#endif

    return(TRUE);
}

/******************************Public*Routine******************************\
* CACHEDGLYPH* pcgNew()
*
* Creates a new CACHEDGLYPH structure for keeping track of the glyph in
* off-screen memory.  bPutGlyphInCache is called to actually put the glyph
* in off-screen memory.
*
* This routine should be reasonably device-independent, as bPutGlyphInCache
* will contain most of the code that will have to be modified for other
* display adapters.
*
\**************************************************************************/

CACHEDGLYPH* pcgNew(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp)
{
    GLYPHALLOC*     pga;
    CACHEDGLYPH*    pcg;
    LONG            cjCachedGlyph;
    HGLYPH          hg;
    LONG            iHash;
    CACHEDGLYPH*    pcgFind;

Restart:

    // First, calculate the amount of storage we'll need for this glyph:

    cjCachedGlyph = sizeof(CACHEDGLYPH);

    if (cjCachedGlyph > pcf->cjAlloc)
    {
        // Have to allocate a new glyph allocation structure:

        pga = ENGALLOCMEM(FL_ZERO_MEMORY, GLYPH_ALLOC_SIZE, ALLOC_TAG, &pcf->pgaChain);
        if (pga == NULL)
        {
            // It's safe to return at this time because we haven't
            // fatally altered any of our data structures:

            return(NULL);
        }

        // Add this allocation to the front of the allocation linked list,
        // so that we can free it later:

        pga->pgaNext  = pcf->pgaChain;
#ifdef MEMCHECK
        if (NULL != pcf->pgaChain)
          UPDATE_BLOCK_DATA(pcf->pgaChain, &pga->pgaNext);
#endif
        pcf->pgaChain = pga;

        // Now we've got a chunk of memory where we can store our cached
        // glyphs:

        pcf->pcgNew  = &pga->acg[0];
        pcf->cjAlloc = GLYPH_ALLOC_SIZE - (sizeof(*pga) - sizeof(pga->acg[0]));
    }

    pcg = pcf->pcgNew;

    // We only need to ensure 'dword' alignment of the next structure:

    pcf->pcgNew   = (CACHEDGLYPH*) ((BYTE*) pcg + cjCachedGlyph);
    pcf->cjAlloc -= cjCachedGlyph;

    ///////////////////////////////////////////////////////////////
    // Insert the glyph, in-order, into the list hanging off our hash
    // bucket:

    hg = pgp->hg;

    pcg->hg = hg;
    iHash   = GLYPH_HASH_FUNC(hg);
    pcgFind = pcf->apcg[iHash];

    if (pcgFind->hg > hg)
    {
        pcf->apcg[iHash] = pcg;
        pcg->pcgNext     = pcgFind;
    }
    else
    {
        // The sentinel will ensure that we never fall off the end of
        // this list:

        while (pcgFind->pcgNext->hg < hg)
            pcgFind = pcgFind->pcgNext;

        // 'pcgFind' now points to the entry to the entry after which
        // we want to insert our new node:

        pcg->pcgNext     = pcgFind->pcgNext;
        pcgFind->pcgNext = pcg;
    }

    ///////////////////////////////////////////////////////////////
    // Download the glyph into off-screen memory:

    if (!bPutGlyphInCache(ppdev, pcg, pgp->pgdf->pgb))
    {
        // If there was no more room in off-screen memory, blow the
        // glyph cache and start over.  Note that this assumes that
        // the glyph will fit in the cache when the cache is completely
        // empty.

        vBlowGlyphCache(ppdev);
        goto Restart;
    }

    return(pcg);
}

/******************************Public*Routine******************************\
* BOOL bCachedFixedText
*
* Draws fixed spaced glyphs via glyph caching.
*
\**************************************************************************/

BOOL bCachedFixedText(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp,
LONG        cGlyph,
ULONG       ulCharInc)
{
    BYTE*           pjH3Base = ppdev->pjH3Base;
    LONG            xGlyph;
    LONG            yGlyph;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            x;
    LONG            y;

    GWH_DECL;

    GWH_PROLOG;

    xGlyph = ppdev->xOffset + pgp->ptl.x;
    yGlyph = ppdev->yOffset + pgp->ptl.y;

    do {
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg)
            pcg = pcg->pcgNext;         // Traverse collision list, if any

        if (pcg->hg > hg)
        {
            // This will hopefully not be the common case (that is,
            // we will have a high cache hit rate), so if I were
            // writing this in Asm I would have this out-of-line
            // to avoid the jump around for the common case.
            // But the Pentium has branch prediction, so what the
            // heck.

            GWH_EPILOG;
            pcg = pcgNew(ppdev, pcf, pgp);
            GWH_PROLOG;
            if (pcg == NULL)
                goto ReturnFalse;

        }

        // Space glyphs are trimmed to a height of zero, and we don't
        // even have to touch the hardware for them:

        if (pcg->cy > 0)
        {
            x = xGlyph + pcg->ptlOrigin.x;
            y = yGlyph + pcg->ptlOrigin.y;

            CHECK_FIFO_ROOM( ppdev, 4);
            GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_SRCBASEADDR |
//                                   SSTCP_PKT2_SRCXY       |
                                   SSTCP_PKT2_DSTSIZE     |
                                   SSTCP_PKT2_DSTXY       |
                                   SSTCP_PKT2_COMMAND);
            SET(1, pjH3Base, srcBaseAddr, pcg->ulLinearStart );
//            SET( pjH3Base, srcXY, 0 );      // Start at 0 off src base.
            SET(2, pjH3Base, dstSize, pcg->cxcy );
            SET(3, pjH3Base, dstXY, H3_PACKXY(x, y) );
//            SET( pjH3Base, command, ppdev->ulCmd );
            SET(4, pjH3Base, command, ( 0xcc << SSTG_ROP0_SHIFT) |
                                                SSTG_TRANSPARENT |
                                                SSTG_GO          |
                                                SSTG_BLT);
            GWH_END_2D_PACKET( 4 );
        }

        xGlyph += ulCharInc;

    } while (pgp++, --cGlyph != 0);

    GWH_EPILOG;
    return(TRUE);

ReturnFalse:
    GWH_EPILOG;
    return(FALSE);
}

/******************************Public*Routine******************************\
* BOOL bCachedProportionalText
*
* Draws proportionally spaced glyphs via glyph caching.
*
\**************************************************************************/

BOOL bCachedProportionalText(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp,
LONG        cGlyph)
{
    BYTE*           pjH3Base = ppdev->pjH3Base;
    LONG            xOffset;
    LONG            yOffset;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            x;
    LONG            y;

    GWH_DECL;

    GWH_PROLOG;

    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;

    do {
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg)
            pcg = pcg->pcgNext;         // Traverse collision list, if any

        if (pcg->hg > hg)
        {
            // This will hopefully not be the common case (that is,
            // we will have a high cache hit rate), so if I were
            // writing this in Asm I would have this out-of-line
            // to avoid the jump around for the common case.
            // But the Pentium has branch prediction, so what the
            // heck.

            GWH_EPILOG;
            pcg = pcgNew(ppdev, pcf, pgp);
            GWH_PROLOG;
            if (pcg == NULL)
                goto ReturnFalse;
        }

        // Space glyphs are trimmed to a height of zero, and we don't
        // even have to touch the hardware for them:

        if (pcg->cy > 0)
        {
            x = pgp->ptl.x + pcg->ptlOrigin.x + xOffset;
            y = pgp->ptl.y + pcg->ptlOrigin.y + yOffset;

            CHECK_FIFO_ROOM( ppdev, 4);
            GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_SRCBASEADDR |
//                                   SSTCP_PKT2_SRCXY       |
                                   SSTCP_PKT2_DSTSIZE     |
                                   SSTCP_PKT2_DSTXY       |
                                   SSTCP_PKT2_COMMAND);
            SET(1, pjH3Base, srcBaseAddr, pcg->ulLinearStart );
//            SET( pjH3Base, srcXY, 0 );      // Start at 0 off src base.
            SET(2, pjH3Base, dstSize, pcg->cxcy );
            SET(3, pjH3Base, dstXY, H3_PACKXY(x, y) );
//            SET( pjH3Base, command, ppdev->ulCmd );
            SET(4, pjH3Base, command, ( 0xcc << SSTG_ROP0_SHIFT) |
                                             SSTG_TRANSPARENT |
                                             SSTG_GO          |
                                             SSTG_BLT);
            GWH_END_2D_PACKET( 4 );
        }
    } while (pgp++, --cGlyph != 0);

    GWH_EPILOG;
    return(TRUE);

ReturnFalse:
    GWH_EPILOG;
    return(FALSE);
}

/******************************Public*Routine******************************\
* BOOL bCachedClippedText
*
* Draws clipped text via glyph caching.
*
\**************************************************************************/

BOOL bCachedClippedText(
PDEV*       ppdev,
CACHEDFONT* pcf,
STROBJ*     pstro,
CLIPOBJ*    pco)
{
    BOOL            bRet;
    BYTE*           pjH3Base = ppdev->pjH3Base;
    LONG            xOffset;
    LONG            yOffset;
    BOOL            bMoreGlyphs;
    ULONG           cGlyphOriginal;
    ULONG           cGlyph;
    BOOL            bClipSet;
    GLYPHPOS*       pgpOriginal;
    GLYPHPOS*       pgp;
    LONG            xGlyph;
    LONG            yGlyph;
    LONG            x;
    LONG            y;
    LONG            xRight;
    LONG            cy;
    BOOL            bMore;
    CLIPENUM        ce;
    RECTL*          prclClip;
    ULONG           ulCharInc;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;

    GWH_DECL;

    GWH_PROLOG;

	// The original sample code handled trivial clipping, but we
	// added a fixed text routine to take care of the last trivial
	// case so now there should not be any trivial clipping here.

    ASSERTDD((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL),
             "Don't expect trivial clipping in this function");

    bRet      = TRUE;

    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    ulCharInc = pstro->ulCharInc;

    do {
      if (pstro->pgp != NULL)
      {
        // There's only the one batch of glyphs, so save ourselves
        // a call:

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      if (cGlyphOriginal > 0)
      {
        if (pco->iDComplexity == DC_RECT)
        {
          // We could call 'cEnumStart' and 'bEnum' when the clipping is
          // DC_RECT, but the last time I checked, those two calls took
          // more than 150 instructions to go through GDI.  Since
          // 'rclBounds' already contains the DC_RECT clip rectangle,
          // and since it's such a common case, we'll special case it:

          bMore    = FALSE;
          ce.c     = 1;
          prclClip = &pco->rclBounds;

          goto SingleRectangle;
        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {
          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

            // We don't always simply set the clipping rectangle here
            // because it may actually end up that no text intersects
            // this clip rectangle, so it would be for naught.  This
            // actually happens a lot when using NT's analog clock set
            // to always-on-top, with a round shape:

            bClipSet = FALSE;

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;

            // We can't yet convert to absolute coordinates by adding
            // in 'xOffset' or 'yOffset' here because we have yet to
            // compare the coordinates to 'prclClip':

            xGlyph = pgp->ptl.x;
            yGlyph = pgp->ptl.y;

            // Loop through all the glyphs for this rectangle:

            while (TRUE)
            {
              hg  = pgp->hg;
              pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

              while (pcg->hg < hg)
                pcg = pcg->pcgNext;

              if (pcg->hg > hg)
              {
                // This will hopefully not be the common case (that is,
                // we will have a high cache hit rate), so if I were
                // writing this in Asm I would have this out-of-line
                // to avoid the jump around for the common case.
                // But the Pentium has branch prediction, so what the
                // heck.

                GWH_EPILOG;
                pcg = pcgNew(ppdev, pcf, pgp);
                GWH_PROLOG;
                if (pcg == NULL)
                {
                  bRet = FALSE;
                  goto AllDone;
                }
              }

              // Space glyphs are trimmed to a height of zero, and we don't
              // even have to touch the hardware for them:

              cy = pcg->cy;
              if (cy > 0)
              {
                y      = pcg->ptlOrigin.y + yGlyph;
                x      = pcg->ptlOrigin.x + xGlyph;
                xRight = pcg->cx + x;

                // Do trivial rejection:

                if ((prclClip->right  > x) &&
                    (prclClip->bottom > y) &&
                    (prclClip->left   < xRight) &&
                    (prclClip->top    < y + cy))
                {
                  // Lazily set the hardware clipping:

                  if (!bClipSet)
                  {
                    bClipSet = TRUE;
//                    vSetClipping(ppdev, prclClip);
                    CHECK_FIFO_ROOM(ppdev, 6);
                    GWH_BEGIN_2D_PACKET(6, SSTCP_PKT2_CLIP0MIN    |
                                           SSTCP_PKT2_CLIP0MAX    |
                                           SSTCP_PKT2_SRCBASEADDR |
//                                           SSTCP_PKT2_SRCXY       |
                                           SSTCP_PKT2_DSTSIZE     |
                                           SSTCP_PKT2_DSTXY       |
                                           SSTCP_PKT2_COMMAND);

                    SET(1, pjH3Base, clip0min,
                         ((prclClip->top    + yOffset) << 16) |
                           prclClip->left   + xOffset);

                    SET(2, pjH3Base, clip0max,
                         ((prclClip->bottom + yOffset) << 16) |
                           prclClip->right  + xOffset);

                    SET(3, pjH3Base, srcBaseAddr, pcg->ulLinearStart );
//                    SET( pjH3Base, srcXY, 0 );      // Start at 0 off src base.
                    SET(4, pjH3Base, dstSize, pcg->cxcy );
                    SET(5, pjH3Base, dstXY, H3_PACKXY(x + xOffset, y + yOffset) );
//                   SET( pjH3Base, command, ppdev->ulCmd );
                    SET(6, pjH3Base, command, ( 0xcc << SSTG_ROP0_SHIFT) |
                                                        SSTG_TRANSPARENT |
                                                        SSTG_GO          |
                                                        SSTG_BLT);

                    GWH_END_2D_PACKET( 6 );
                  }
                  else
                  {
                    CHECK_FIFO_ROOM( ppdev, 4);
                    GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_SRCBASEADDR |
//                                           SSTCP_PKT2_SRCXY       |
                                           SSTCP_PKT2_DSTSIZE     |
                                           SSTCP_PKT2_DSTXY       |
                                           SSTCP_PKT2_COMMAND);

                    SET(1, pjH3Base, srcBaseAddr, pcg->ulLinearStart );
//                    SET( pjH3Base, srcXY, 0 );      // Start at 0 off src base.
                    SET(2, pjH3Base, dstSize, pcg->cxcy );
                    SET(3, pjH3Base, dstXY, H3_PACKXY(x + xOffset, y + yOffset) );
//                  SET( pjH3Base, command, ppdev->ulCmd );
                    SET(4, pjH3Base, command, ( 0xcc << SSTG_ROP0_SHIFT) |
                                                        SSTG_TRANSPARENT |
                                                        SSTG_GO          |
                                                        SSTG_BLT);
                    GWH_END_2D_PACKET( 4 );
                  }
                }
              }

              if (--cGlyph == 0)
                break;

              // Get ready for next glyph:

              pgp++;

              if (ulCharInc == 0)
              {
                xGlyph = pgp->ptl.x;
                yGlyph = pgp->ptl.y;
              }
              else
              {
                xGlyph += ulCharInc;
              }
            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);

AllDone:

    //  Put vResetClipping inline to speed things up.
    CHECK_FIFO_ROOM(ppdev, 2);
    GWH_BEGIN_2D_PACKET( 2, SSTCP_PKT2_CLIP0MIN | SSTCP_PKT2_CLIP0MAX );
    SET(1, pjH3Base, clip0min, 0);
    SET(2, pjH3Base, clip0max, H3_PACKXY_FAST(ppdev->cxMemory, ppdev->cyMemory));
    GWH_END_2D_PACKET( 2 );

    GWH_EPILOG;

    return(bRet);
}

/******************************Public*Routine******************************\
* BOOL bH3TextOut
*
* Outputs text using the 'buffer expansion' method.  The CPU draws to a
* 1bpp buffer, and the result is colour-expanded to the screen using the
* hardware.
*
\**************************************************************************/

BOOL bH3TextOut(
SURFOBJ*  pso,
STROBJ*   pstro,
FONTOBJ*  pfo,
CLIPOBJ*  pco,
RECTL*    prclOpaque,
BRUSHOBJ* pboFore,
BRUSHOBJ* pboOpaque)
{
    PDEV*           ppdev;
    DSURF*          pdsurf;
    BYTE*           pjH3Base;
    BOOL            bGlyphExpand;
    BOOL            bTextPerfectFit;
    ULONG           cGlyph;
    BOOL            bMoreGlyphs;
    GLYPHPOS*       pgp;
    GLYPHBITS*      pgb;
    BYTE*           pjGlyph;
    LONG            cyGlyph;
    POINTL          ptlOrigin;
    LONG            ulCharInc;
    BYTE            iDComplexity;
    LONG            lDelta;
    LONG            cw;
    RECTL           rclOpaque;
    CACHEDFONT*     pcf;

    GWH_DECL;

    ppdev = (PDEV*) pso->dhpdev;
    pjH3Base = ppdev->pjH3Base;

    GWH_PROLOG;

    iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if (prclOpaque != NULL)
    {
      ////////////////////////////////////////////////////////////
      // Opaque Initialization
      ////////////////////////////////////////////////////////////

      if (iDComplexity == DC_TRIVIAL)
      {

      DrawOpaqueRect:

        CHECK_FIFO_ROOM(ppdev, 4);
        GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_COLORFORE |
                               SSTCP_PKT2_DSTSIZE   |
                               SSTCP_PKT2_DSTXY     |
                               SSTCP_PKT2_COMMAND);

        SET(1, pjH3Base, colorFore, pboOpaque->iSolidColor);

        SET(2, pjH3Base, dstSize, H3_PACKXY_FAST( prclOpaque->right - prclOpaque->left,
                                                  prclOpaque->bottom - prclOpaque->top ));

        SET(3, pjH3Base, dstXY, H3_PACKXY( prclOpaque->left + ppdev->xOffset,
                                           prclOpaque->top + ppdev->yOffset ));

        SET(4, pjH3Base, command, (0xcc << SSTG_ROP0_SHIFT)  |
                                           SSTG_GO           |
                                           SSTG_RECTFILL );

        GWH_END_2D_PACKET( 4 );
      }
      else if (iDComplexity == DC_RECT)
      {
        if (bIntersect(prclOpaque, &pco->rclBounds, &rclOpaque))
        {
          prclOpaque = &rclOpaque;
          goto DrawOpaqueRect;
        }
      }
      else
      {
        GWH_EPILOG;
        vH3ClipSolid(ppdev, prclOpaque, pboOpaque->iSolidColor, pco);
        GWH_PROLOG;
      }
    }

    ////////////////////////////////////////////////////////////
    // Transparent Initialization
    ////////////////////////////////////////////////////////////

    // Initialize the hardware for transparent text:

//    CHECK_FIFO_ROOM(ppdev, 1);
//    SET(pjH3Base, srcFormat, SSTG_PIXFMT_1BPP | SSTG_SRC_PACK_32);
//    SET(pjH3Base, srcXY, 0);   // Zero for packed host blts.
//    jdw - colorFore set moved below 10/97.
//    SET(1, pjH3Base, colorFore, pboFore->iSolidColor);

//    // Save command for later.
//    ppdev->ulCmd = ( 0xcc << SSTG_ROP0_SHIFT) |
//                             SSTG_TRANSPARENT |
//                             SSTG_HOST_BLT;

//  SkipTransparentInitialization:

#define USE_TEXT_CACHE

#ifdef USE_TEXT_CACHE

    if ((pfo->cxMax <= GLYPH_CACHE_CX) &&
        ((pstro->rclBkGround.bottom - pstro->rclBkGround.top) <= GLYPH_CACHE_CY) &&
        (ppdev->flStatus & STAT_GLYPH_CACHE))
    {
      CHECK_FIFO_ROOM(ppdev, 3);
      GWH_BEGIN_2D_PACKET( 3, SSTCP_PKT2_SRCFORMAT |
                              SSTCP_PKT2_SRCXY     |
                              SSTCP_PKT2_COLORFORE );

      SET(1, pjH3Base, srcFormat, SSTG_PIXFMT_1BPP | SSTG_SRC_PACK_8);
      SET(2, pjH3Base, srcXY, 0 );      // Start at 0 off src base.
      SET(3, pjH3Base, colorFore, pboFore->iSolidColor);

      GWH_END_2D_PACKET( 3 );

      pcf = (CACHEDFONT*) pfo->pvConsumer;

      GWH_EPILOG;   // Needed here in case of multiple returns below.

      if (pcf == NULL)
      {
        pcf = pcfAllocateCachedFont(ppdev);
        if (pcf == NULL)
          return(FALSE);

        pfo->pvConsumer = pcf;
      }

      // Use our glyph cache:

      if (iDComplexity == DC_TRIVIAL)
      {
        do {
          if (pstro->pgp != NULL)
          {
            // There's only the one batch of glyphs, so save ourselves
            // a call:

            pgp         = pstro->pgp;
            cGlyph      = pstro->cGlyphs;
            bMoreGlyphs = FALSE;
          }
          else
          {
            bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyph, &pgp);
          }

          if (cGlyph > 0)
          {
            if (pstro->ulCharInc == 0)
            {
              if (!bCachedProportionalText(ppdev, pcf, pgp, cGlyph))
                return(FALSE);
            }
            else
            {
              if (!bCachedFixedText(ppdev, pcf, pgp, cGlyph, pstro->ulCharInc))
                return(FALSE);
            }
          }
        } while (bMoreGlyphs);
      }
      else
      {
      	// Trivial clipping alread handled.
      	
        if (!bCachedClippedText(ppdev, pcf, pstro, pco))
          return(FALSE);
      }
      GWH_PROLOG;
    }
    else
#endif
    {
#ifdef USE_TEXT_CACHE
      DISPDBG((4, "Text too big to cache: %li x %li",
          pfo->cxMax, pstro->rclBkGround.bottom - pstro->rclBkGround.top));
#endif

      CHECK_FIFO_ROOM(ppdev, 3);
      GWH_BEGIN_2D_PACKET( 3, SSTCP_PKT2_SRCFORMAT |
                              SSTCP_PKT2_SRCXY     |
                              SSTCP_PKT2_COLORFORE );

      SET(1, pjH3Base, srcFormat, SSTG_PIXFMT_1BPP | SSTG_SRC_PACK_32);
      SET(2, pjH3Base, srcXY, 0 );      // Start at 0 off src base.
      SET(3, pjH3Base, colorFore, pboFore->iSolidColor);

      GWH_END_2D_PACKET( 3 );

      GWH_EPILOG;
      vH3GeneralText(ppdev, pstro, pco);
      GWH_PROLOG;
    }

    CHECK_FIFO_ROOM(ppdev, 1);
    GWH_BEGIN_2D_PACKET( 1, SSTCP_PKT2_SRCBASEADDR );
    SET(1, pjH3Base, srcBaseAddr, ppdev->ulScreenOffset);	// Reset srcBaseAddr
    GWH_END_2D_PACKET( 1 );

    GWH_EPILOG;

    return(TRUE);
}

/******************************Public*Routine******************************\
* BOOL DrvTextOut
*
* Calls the appropriate text drawing routine.
*
\**************************************************************************/

BOOL DrvTextOut(
SURFOBJ*  pso,
STROBJ*   pstro,
FONTOBJ*  pfo,
CLIPOBJ*  pco,
RECTL*    prclExtra,    // If we had set GCAPS_HORIZSTRIKE, we would have
                        //   to fill these extra rectangles (it is used
                        //   largely for underlines).  It's not a big
                        //   performance win (GDI will call our DrvBitBlt
                        //   to draw the extra rectangles).
RECTL*    prclOpaque,
BRUSHOBJ* pboFore,
BRUSHOBJ* pboOpaque,
POINTL*   pptlBrush,    // Always unused, unless GCAPS_ARBRUSHOPAQUE set
MIX       mix)          // Always a copy mix -- 0x0d0d
{
    PDEV*           ppdev;
    DSURF*          pdsurf;
    OH*             poh;
    SURFOBJ*        psoPunt;

    GLIDE_EXCLUSION(glideState[ 0 ]);

    pdsurf = (DSURF*) pso->dhsurf;
    ppdev  = (PDEV*) pso->dhpdev;

    if (pdsurf->dt != DT_DIB)
    {
      H3PRINTF((ppdev, "DrvTextOut\r\n"));

      poh            = pdsurf->poh;
      ppdev->xOffset = poh->x;
      ppdev->yOffset = poh->y;

      // The DDI spec says we'll only ever get foreground and background
      // mixes of R2_COPYPEN:

      ASSERTDD(mix == 0x0d0d, "GDI should only give us a copy mix");

#if defined(DBG) || defined(PUNT_OPTION)
      if( UseCSIM )

         return(bH3TextOut(pso, pstro, pfo, pco, prclOpaque, pboFore,
               pboOpaque));

      else
      {
      // Setup the punt surface and call GDI to handle it:

          psoPunt          = ppdev->psoPunt;
          psoPunt->pvScan0 = poh->pvScan0;
          psoPunt->lDelta  = ppdev->lDelta;

          START_DIRECT_ACCESS_H3(ppdev, ppdev->pjH3Base);

          return(EngTextOut(psoPunt, pstro, pfo, pco, prclExtra, prclOpaque,
                            pboFore, pboOpaque, pptlBrush, mix));

          END_DIRECT_ACCESS_H3(ppdev, ppdev->pjH3Base);
      }
#else
      return(bH3TextOut(pso, pstro, pfo, pco, prclOpaque, pboFore, pboOpaque));
#endif
    }
    else
    {
      // We're drawing to a DFB we've converted to a DIB, so just call GDI
      // to handle it:

      return(EngTextOut(pdsurf->pso, pstro, pfo, pco, prclExtra, prclOpaque,
                        pboFore, pboOpaque, pptlBrush, mix));
    }

    return(TRUE);
}

/******************************Public*Routine******************************\
* BOOL bEnableText
*
* Performs the necessary setup for the text drawing subcomponent.
*
\**************************************************************************/

BOOL bEnableText(
PDEV*   ppdev)
{
    OH*         poh;
    CACHEDFONT* pcfSentinel;
    LONG        cShift;
    LONG        cFactor;


    poh = pohAllocate(ppdev,
                      NULL,
                      ppdev->cxMemory,
                      GLYPH_CACHE_HEIGHT / ppdev->cjPelSize,
                      FLOH_MAKE_PERMANENT);
    if (poh != NULL)
    {
        // Since we allocated a chunk of memory that is the width of
        // memory, we can expect the following:

        ASSERTDD(poh->x == 0, "Expected allocation to start at left");

        ppdev->flStatus |= STAT_GLYPH_CACHE;

        // Initialize our doubly-linked cached font list:

        pcfSentinel = &ppdev->cfSentinel;
        pcfSentinel->pcfNext = pcfSentinel;
        pcfSentinel->pcfPrev = pcfSentinel;

        // Setup the display adapter specific glyph data.
        //
        // The linear addresses are computed as byte addresses:

        cFactor = ppdev->cjPelSize;

        ppdev->ulGlyphStart
            = (poh->y * ppdev->cxMemory * cFactor) + ppdev->ulScreenOffset;

        ppdev->ulGlyphCurrent = ppdev->ulGlyphStart;

        ppdev->ulGlyphEnd
            = (((poh->y + poh->cy) * ppdev->cxMemory) * cFactor) + ppdev->ulScreenOffset;
    }

    return(TRUE);
}

/******************************Public*Routine******************************\
* VOID vDisableText
*
* Performs the necessary clean-up for the text drawing subcomponent.
*
\**************************************************************************/

VOID vDisableText(PDEV* ppdev)
{
    // Here we free any stuff allocated in 'bEnableText'.
}

/******************************Public*Routine******************************\
* VOID vAssertModeText
*
* Disables or re-enables the text drawing subcomponent in preparation for
* full-screen entry/exit.
*
\**************************************************************************/

VOID vAssertModeText(
PDEV*   ppdev,
BOOL    bEnable)
{
    // Our off-screen glyph cache will get destroyed when we switch to
    // full-screen:

    if (!bEnable)
    {
        if (ppdev->flStatus & STAT_GLYPH_CACHE)
        {
            vBlowGlyphCache(ppdev);
        }
    }
}

/******************************Public*Routine******************************\
* VOID DrvDestroyFont
*
* Note: Don't forget to export this call in 'enable.c', otherwise you'll
*       get some pretty big memory leaks!
*
* We're being notified that the given font is being deallocated; clean up
* anything we've stashed in the 'pvConsumer' field of the 'pfo'.
*
\**************************************************************************/

VOID DrvDestroyFont(
FONTOBJ*    pfo)
{
    CACHEDFONT* pcf;

	if ( glideState[ 0 ].glideGDIFlags & GLDATA_GDIFLAGS_HWC_EXCLUSIVE ) return;

    pcf = pfo->pvConsumer;
    if (pcf != NULL)
    {
        vFreeCachedFont(pcf);
        pfo->pvConsumer = NULL;
    }
}
