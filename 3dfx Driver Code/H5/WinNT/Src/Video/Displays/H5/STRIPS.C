/******************************Module*Header*******************************\
* Module Name: Strips.c
*
* These are the line rendering routines of last resort, and are called
* by 'bLines' when a line is clipped or otherwise cannot be drawn
* directly by the hardware.
*
* We take advantage of Banshee's point-to-point vector drawing capability
* to draw the strips.  Banshee can automatically exclude the last pixel,
* which makes our routines a little simpler.
*
* Copyright (c) 1992-1996 Microsoft Corporation
* Copyright (c) 1997 3Dfx Interactive, Inc.
\**************************************************************************/

#include "precomp.h"

/******************************Public*Routine******************************\
* VOID vH3SolidHorizontal
*
* Draws left-to-right x-major near-horizontal lines.
*
\**************************************************************************/

VOID vH3SolidHorizontal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjH3Base;
    LONG    x;
    LONG    y;
    LONG    yDir;
    LONG*   pStrips;
    LONG    cStrips;
    LONG    i;

    GWH_DECL;

    GWH_PROLOG;

    pjH3Base = ppdev->pjH3Base;

    x = pStrip->ptlStart.x + ppdev->xOffset;
    y = pStrip->ptlStart.y + ppdev->yOffset;

    yDir    = (pStrip->flFlips & FL_FLIP_V) ? -1 : 1;
    pStrips = pStrip->alStrips;
    cStrips = pStrip->cStrips;

    for (i = cStrips; i != 0; i--)
    {
        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET( 3, SSTCP_PKT2_SRCXY   |
                                SSTCP_PKT2_DSTXY   |
                                SSTCP_PKT2_COMMAND );

        SET(1, pjH3Base, srcXY, H3_PACKXY(x, y));
        x += *pStrips++;
        SET(2, pjH3Base, dstXY, H3_PACKXY(x, y));
        SET(3, pjH3Base, command, (ppdev->ulRop3 << SSTG_ROP0_SHIFT)   |
                                   SSTG_MONO_PATTERN                   |
                                   SSTG_REVERSIBLE                     |
                                   SSTG_GO                             |
                                   SSTG_POLYLINE );
        GWH_END_2D_PACKET( 3 );

        y += yDir;
    }

    pStrip->ptlStart.x = x - ppdev->xOffset;
    pStrip->ptlStart.y = y - ppdev->yOffset;

    GWH_EPILOG;
}


/******************************Public*Routine******************************\
* VOID vH3SolidVertical
*
* Draws left-to-right y-major near-vertical lines.
*
\**************************************************************************/

VOID vH3SolidVertical(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjH3Base;
    LONG    x;
    LONG    y;
    LONG    yDir;
    LONG*   pStrips;
    LONG    cStrips;
    LONG    i;

    GWH_DECL;

    GWH_PROLOG;

    pjH3Base = ppdev->pjH3Base;

    x = pStrip->ptlStart.x + ppdev->xOffset;
    y = pStrip->ptlStart.y + ppdev->yOffset;

    yDir    = (pStrip->flFlips & FL_FLIP_V) ? -1 : 1;
    pStrips = pStrip->alStrips;
    cStrips = pStrip->cStrips;

    for (i = cStrips; i != 0; i--)
    {
        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET( 3, SSTCP_PKT2_SRCXY   |
                                SSTCP_PKT2_DSTXY   |
                                SSTCP_PKT2_COMMAND );

        SET(1, pjH3Base, srcXY, H3_PACKXY(x, y));
        y += (yDir > 0) ? *pStrips : -*pStrips;
        pStrips++;
        SET(2, pjH3Base, dstXY, H3_PACKXY(x, y));
        SET(3, pjH3Base, command, (ppdev->ulRop3 << SSTG_ROP0_SHIFT)   |
                                   SSTG_MONO_PATTERN                   |
                                   SSTG_REVERSIBLE                     |
                                   SSTG_GO                             |
                                   SSTG_POLYLINE );
        GWH_END_2D_PACKET( 3 );

        x++;
    }

    pStrip->ptlStart.x = x - ppdev->xOffset;
    pStrip->ptlStart.y = y - ppdev->yOffset;

    GWH_EPILOG;
}


/******************************Public*Routine******************************\
* VOID vH3SolidDiagonalHorizontal
*
* Draws left-to-right x-major near-diagonal lines.
*
\**************************************************************************/

VOID vH3SolidDiagonalHorizontal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjH3Base;
    LONG    x;
    LONG    y;
    LONG    yDir;
    LONG*   pStrips;
    LONG    cStrips;
    LONG    i;

    GWH_DECL;

    GWH_PROLOG;

    pjH3Base = ppdev->pjH3Base;

    x = pStrip->ptlStart.x + ppdev->xOffset;
    y = pStrip->ptlStart.y + ppdev->yOffset;

    yDir    = (pStrip->flFlips & FL_FLIP_V) ? -1 : 1;
    pStrips = pStrip->alStrips;
    cStrips = pStrip->cStrips;

    for (i = cStrips; i != 0; i--)
    {
        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET( 3, SSTCP_PKT2_SRCXY   |
                                SSTCP_PKT2_DSTXY   |
                                SSTCP_PKT2_COMMAND );

        SET(1, pjH3Base, srcXY, H3_PACKXY(x, y));
        x += *pStrips;
        y += (yDir > 0) ? *pStrips : -*pStrips;
        pStrips++;
        SET(2, pjH3Base, dstXY, H3_PACKXY(x, y));
        SET(3, pjH3Base, command, (ppdev->ulRop3 << SSTG_ROP0_SHIFT)   |
                                   SSTG_MONO_PATTERN                   |
                                   SSTG_REVERSIBLE                     |
                                   SSTG_GO                             |
                                   SSTG_POLYLINE );
        GWH_END_2D_PACKET( 3 );

        y -= yDir;
    }

    pStrip->ptlStart.x = x - ppdev->xOffset;
    pStrip->ptlStart.y = y - ppdev->yOffset;

    GWH_EPILOG;
}


/******************************Public*Routine******************************\
* VOID vH3SolidDiagonalVertical
*
* Draws left-to-right y-major near-diagonal lines.
*
\**************************************************************************/

VOID vH3SolidDiagonalVertical(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjH3Base;
    LONG    x;
    LONG    y;
    LONG    yDir;
    LONG*   pStrips;
    LONG    cStrips;
    LONG    i;

    GWH_DECL;

    GWH_PROLOG;

    pjH3Base = ppdev->pjH3Base;

    x = pStrip->ptlStart.x + ppdev->xOffset;
    y = pStrip->ptlStart.y + ppdev->yOffset;

    yDir    = (pStrip->flFlips & FL_FLIP_V) ? -1 : 1;
    pStrips = pStrip->alStrips;
    cStrips = pStrip->cStrips;

    for (i = cStrips; i != 0; i--)
    {
        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET( 3, SSTCP_PKT2_SRCXY   |
                                SSTCP_PKT2_DSTXY   |
                                SSTCP_PKT2_COMMAND );

        SET(1, pjH3Base, srcXY, H3_PACKXY(x, y));
        x += *pStrips;
        y += (yDir > 0) ? *pStrips : -*pStrips;
        pStrips++;
        SET(2, pjH3Base, dstXY, H3_PACKXY(x, y));
        SET(3, pjH3Base, command, (ppdev->ulRop3 << SSTG_ROP0_SHIFT)   |
                                   SSTG_MONO_PATTERN                   |
                                   SSTG_REVERSIBLE                     |
                                   SSTG_GO                             |
                                   SSTG_POLYLINE );

        GWH_END_2D_PACKET( 3 );

        x--;
    }

    pStrip->ptlStart.x = x - ppdev->xOffset;
    pStrip->ptlStart.y = y - ppdev->yOffset;

    GWH_EPILOG;
}


/******************************Public*Routine******************************\
* VOID vH3StripStyledHorizontal
*
* Takes the list of strips that define the pixels that would be lit for
* a solid line, and breaks them into styling chunks according to the
* styling information that is passed in.
*
* This particular routine handles x-major lines that run left-to-right,
* and are comprised of horizontal strips.  It draws the dashes using
* short-stroke vectors.
*
* The performance of this routine could be improved significantly if
* anyone cared enough about styled lines improve it.
*
\**************************************************************************/

VOID vH3StripStyledHorizontal(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    BYTE*   pjH3Base;
    LONG    x;
    LONG    y;
    LONG    dy;
    LONG*   plStrip;
    LONG    cStrips;
    LONG    cStyle;
    LONG    cStrip;
    LONG    cThis;
    ULONG   bIsGap;

    GWH_DECL;

    GWH_PROLOG;

    pjH3Base = ppdev->pjH3Base;

    if (pstrip->flFlips & FL_FLIP_V)
    {
        // The minor direction of the line is 90 degrees, and the major
        // direction is 0 (it's a left-to-right x-major line going up):

        dy = -1;
    }
    else
    {
        // The minor direction of the line is 270 degrees, and the major
        // direction is 0 (it's a left-to-right x-major line going down):

        dy = 1;
    }

    cStrips = pstrip->cStrips;      // Total number of strips we'll do
    plStrip = pstrip->alStrips;     // Points to current strip
    x       = pstrip->ptlStart.x + ppdev->xOffset;
                                    // x position of start of first strip
    y       = pstrip->ptlStart.y + ppdev->yOffset;
                                    // y position of start of first strip

    cStrip = *plStrip;              // Number of pels in first strip

    cStyle = pls->spRemaining;      // Number of pels in first 'gap' or 'dash'
    bIsGap = pls->ulStyleMask;      // Tells whether in a 'gap' or a 'dash'

    // ulStyleMask is non-zero if we're in the middle of a 'gap',
    // and zero if we're in the middle of a 'dash':

    if (bIsGap)
        goto SkipAGap;
    else
        goto OutputADash;

PrepareToSkipAGap:

    // Advance in the style-state array, so that we can find the next
    // 'dot' that we'll have to display:

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
        pls->psp = pls->pspStart;

    cStyle = *pls->psp;

    // If 'cStrip' is zero, we also need a new strip:

    if (cStrip != 0)
        goto SkipAGap;

    // Here, we're in the middle of a 'gap' where we don't have to
    // display anything.  We simply cycle through all the strips
    // we can, keeping track of the current position, until we run
    // out of 'gap':

    while (TRUE)
    {
        // Each time we loop, we move to a new scan and need a new strip:

        y += dy;

        plStrip++;
        cStrips--;
        if (cStrips == 0)
            goto AllDone;

        cStrip = *plStrip;

    SkipAGap:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

        x += cThis;

        if (cStyle == 0)
            goto PrepareToOutputADash;
    }

PrepareToOutputADash:

    // Advance in the style-state array, so that we can find the next
    // 'dot' that we'll have to display:

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
        pls->psp = pls->pspStart;

    cStyle = *pls->psp;

    // If 'cStrip' is zero, we also need a new strip.

    if (cStrip != 0)
    {
        // There's more to be done in the current strip:

        goto OutputADash;
    }

    // We've finished with the current strip:

    while (TRUE)
    {
        // Each time we loop, we move to a new scan and need a new strip:

        y += dy;

        plStrip++;
        cStrips--;
        if (cStrips == 0)
            goto AllDone;

        cStrip = *plStrip;

    OutputADash:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET( 3, SSTCP_PKT2_SRCXY   |
                                SSTCP_PKT2_DSTXY   |
                                SSTCP_PKT2_COMMAND );

        SET(1, pjH3Base, srcXY, H3_PACKXY(x, y));
        x += cThis;
        SET(2, pjH3Base, dstXY, H3_PACKXY(x, y));
        SET(3, pjH3Base, command, (ppdev->ulRop3 << SSTG_ROP0_SHIFT)   |
                                   SSTG_MONO_PATTERN                   |
                                   SSTG_REVERSIBLE                     |
                                   SSTG_GO                             |
                                   SSTG_POLYLINE );

        GWH_END_2D_PACKET( 3 );

        if (cStyle == 0)
            goto PrepareToSkipAGap;
    }

AllDone:

    // Update our state variables so that the next line can continue
    // where we left off:

    pls->spRemaining   = cStyle;
    pls->ulStyleMask   = bIsGap;
    pstrip->ptlStart.x = x - ppdev->xOffset;
    pstrip->ptlStart.y = y - ppdev->yOffset;

    GWH_EPILOG;
}


/******************************Public*Routine******************************\
* VOID vH3StripStyledVertical
*
* Takes the list of strips that define the pixels that would be lit for
* a solid line, and breaks them into styling chunks according to the
* styling information that is passed in.
*
* This particular routine handles y-major lines that run left-to-right,
* and are comprised of vertical strips.  It draws the dashes using
* short-stroke vectors.
*
* The performance of this routine could be improved significantly if
* anyone cared enough about styled lines improve it.
*
\**************************************************************************/

VOID vH3StripStyledVertical(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    BYTE*   pjH3Base;
    LONG    x;
    LONG    y;
    LONG    dy;
    LONG*   plStrip;
    LONG    cStrips;
    LONG    cStyle;
    LONG    cStrip;
    LONG    cThis;
    ULONG   bIsGap;

    GWH_DECL;

    GWH_PROLOG;

    pjH3Base = ppdev->pjH3Base;

    if (pstrip->flFlips & FL_FLIP_V)
    {
        // The minor direction of the line is 0 degrees, and the major
        // direction is 90 (it's a left-to-right y-major line going up):

        dy = -1;
    }
    else
    {
        // The minor direction of the line is 0 degrees, and the major
        // direction is 270 (it's a left-to-right y-major line going down):

        dy = 1;
    }

    cStrips = pstrip->cStrips;      // Total number of strips we'll do
    plStrip = pstrip->alStrips;     // Points to current strip
    x       = pstrip->ptlStart.x + ppdev->xOffset;
                                    // x position of start of first strip
    y       = pstrip->ptlStart.y + ppdev->yOffset;
                                    // y position of start of first strip

    cStrip = *plStrip;              // Number of pels in first strip

    cStyle = pls->spRemaining;      // Number of pels in first 'gap' or 'dash'
    bIsGap = pls->ulStyleMask;      // Tells whether in a 'gap' or a 'dash'

    // ulStyleMask is non-zero if we're in the middle of a 'gap',
    // and zero if we're in the middle of a 'dash':

    if (bIsGap)
        goto SkipAGap;
    else
        goto OutputADash;

PrepareToSkipAGap:

    // Advance in the style-state array, so that we can find the next
    // 'dot' that we'll have to display:

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
        pls->psp = pls->pspStart;

    cStyle = *pls->psp;

    // If 'cStrip' is zero, we also need a new strip:

    if (cStrip != 0)
        goto SkipAGap;

    // Here, we're in the middle of a 'gap' where we don't have to
    // display anything.  We simply cycle through all the strips
    // we can, keeping track of the current position, until we run
    // out of 'gap':

    while (TRUE)
    {
        // Each time we loop, we move to a new column and need a new strip:

        x++;

        plStrip++;
        cStrips--;
        if (cStrips == 0)
            goto AllDone;

        cStrip = *plStrip;

    SkipAGap:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

        y += (dy > 0) ? cThis : -cThis;

        if (cStyle == 0)
            goto PrepareToOutputADash;
    }

PrepareToOutputADash:

    // Advance in the style-state array, so that we can find the next
    // 'dot' that we'll have to display:

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
        pls->psp = pls->pspStart;

    cStyle = *pls->psp;

    // If 'cStrip' is zero, we also need a new strip.

    if (cStrip != 0)
    {
        // There's more to be done in the current strip:

        goto OutputADash;
    }

    // We've finished with the current strip:

    while (TRUE)
    {
        // Each time we loop, we move to a new column and need a new strip:

        x++;

        plStrip++;
        cStrips--;
        if (cStrips == 0)
            goto AllDone;

        cStrip = *plStrip;

    OutputADash:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET( 3, SSTCP_PKT2_SRCXY   |
                                SSTCP_PKT2_DSTXY   |
                                SSTCP_PKT2_COMMAND );

        SET(1, pjH3Base, srcXY, H3_PACKXY(x, y));
        y += (dy > 0) ? cThis : -cThis;
        SET(2, pjH3Base, dstXY, H3_PACKXY(x, y));
        SET(3, pjH3Base, command, (ppdev->ulRop3 << SSTG_ROP0_SHIFT)   |
                                   SSTG_MONO_PATTERN                   |
                                   SSTG_REVERSIBLE                     |
                                   SSTG_GO                             |
                                   SSTG_POLYLINE );

        GWH_END_2D_PACKET( 3 );

        if (cStyle == 0)
            goto PrepareToSkipAGap;
    }

AllDone:

    // Update our state variables so that the next line can continue
    // where we left off:

    pls->spRemaining   = cStyle;
    pls->ulStyleMask   = bIsGap;
    pstrip->ptlStart.x = x - ppdev->xOffset;
    pstrip->ptlStart.y = y - ppdev->yOffset;

    GWH_EPILOG;
}
