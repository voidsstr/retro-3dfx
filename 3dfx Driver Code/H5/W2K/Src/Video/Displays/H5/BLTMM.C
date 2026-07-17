/******************************Module*Header*******************************\
* Module Name: bltmm.c
*
* Contains the low-level memory-mapped IO blt functions.  This module
* mirrors 'bltio.c'.
*
* Hopefully, if you're basing your display driver on this code, to
* support all of DrvBitBlt and DrvCopyBits, you'll only have to implement
* the following routines.  You shouldn't have to modify much in
* 'bitblt.c'.  I've tried to make these routines as few, modular, simple,
* and efficient as I could, while still accelerating as many calls as
* possible that would be cost-effective in terms of performance wins
* versus size and effort.
*
* Note: In the following, 'relative' coordinates refers to coordinates
*       that haven't yet had the offscreen bitmap (DFB) offset applied.
*       'Absolute' coordinates have had the offset applied.  For example,
*       we may be told to blt to (1, 1) of the bitmap, but the bitmap may
*       be sitting in offscreen memory starting at coordinate (0, 768) --
*       (1, 1) would be the 'relative' start coordinate, and (1, 769)
*       would be the 'absolute' start coordinate'.
*
* Copyright (c) 1992-1996 Microsoft Corporation.  All rights reserved.
* Copyright (c) 1997-1999 3Dfx Interactive, Inc.  All rights reserved.
*
\**************************************************************************/

#include "precomp.h"

#ifdef INCSTBPERF
//        H3\DISPLAYS\VIDEO\DRV\BUILD\stbperf.inc
#ifndef MS_VIEW
#include "..\..\..\..\build\stbperf.inc"
#else
#include "stbperf.inc"
#endif
#endif


/******************************Public*Routine******************************\
* VOID vH3ImageTransferMm32
*
* Low-level routine for transferring a bitmap image via the launch register
* using 32 bit writes and entirely memory-mapped I/O.
*
* Command fifo is a pain here because we can't garantee that the data for
* the whole blt will fit in one command packet so it's going to cost us
* some overhead.
*
\**************************************************************************/

VOID vH3ImageTransferMm32(  // Type FNIMAGETRANSFER
PDEV*   ppdev,
BYTE*   pjSrc,              // Source pointer
LONG    lDelta,             // Delta from start of scan to start of next
LONG    cjSrc,              // Number of bytes to be output on every scan
LONG    cScans,             // Number of scans
ULONG   ulCmd)              // Accelerator command - shouldn't include bus size
{
    BYTE*   pjH3Base = ppdev->pjH3Base;
    LONG    cdSrc;
    LONG    cjEnd;
    ULONG   d;
#ifdef H3_FIFO
    ULONG   ulPktSize;
#endif

    GWH_DECL;

    ASSERTDD(cScans > 0, "Can't handle non-positive count of scans");

    GWH_PROLOG;

    cdSrc = cjSrc >> 2;
    cjEnd = cdSrc << 2;

#ifdef USE_ALTERNATE_TEXT  // Alternate text routine was pulled out earlier.
    CHECK_FIFO_ROOM( ppdev, 2 );
    GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCXY | SSTCP_PKT2_COMMAND);
    SET(1, pjH3Base, srcXY,  (ULONG) pjSrc );
    SET(2, pjH3Base, command, ulCmd );
    GWH_END_2D_PACKET( 2 );
#else
    /* If we're not supporting the alternate text routine we could move this
     * back into the calling routines to elimate a fifo write.
     */

    CHECK_FIFO_ROOM( ppdev, 1 );
    GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_COMMAND);
    SET(1, pjH3Base, command, ulCmd );
    GWH_END_2D_PACKET( 1 );
#endif

#ifdef H3_FIFO
    ulPktSize = cScans * (cdSrc + ((cjSrc & 3)? 1 : 0));
    // Subtract one extra for header word.
    if( ulPktSize < ((HW_CMDFIFO_TOTAL_SIZE / 4) - H3_FIFO_END_ADJUST - 1) )
    {
#endif
        CHECK_FIFO_ROOM( ppdev, ulPktSize );
        GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

        switch (cjSrc & 3)
        {
        case 3:
            do {
                if (cdSrc > 0)
                    H3_TRANSFER_DWORD(ppdev, pjH3Base, pjSrc, cdSrc);

                d = (ULONG) (*(pjSrc + cjEnd))          |
                            (*(pjSrc + cjEnd + 1) << 8) |
                            (*(pjSrc + cjEnd + 2) << 16);
                H3_TRANSFER_DWORD(ppdev, pjH3Base, &d, 1);
                pjSrc += lDelta;

            } while (--cScans != 0);
            break;

        case 2:
            do {
                if (cdSrc > 0)
                    H3_TRANSFER_DWORD(ppdev, pjH3Base, pjSrc, cdSrc);

                d = (ULONG) (*(pjSrc + cjEnd))          |
                            (*(pjSrc + cjEnd + 1) << 8);
                H3_TRANSFER_DWORD(ppdev, pjH3Base, &d, 1);
                pjSrc += lDelta;

            } while (--cScans != 0);
            break;

        case 1:
            do {
                if (cdSrc > 0)
                    H3_TRANSFER_DWORD(ppdev, pjH3Base, pjSrc, cdSrc);

                d = (ULONG) (*(pjSrc + cjEnd));
                H3_TRANSFER_DWORD(ppdev, pjH3Base, &d, 1);
                pjSrc += lDelta;

            } while (--cScans != 0);
            break;

        case 0:
            do {
                H3_TRANSFER_DWORD(ppdev, pjH3Base, pjSrc, cdSrc);
                pjSrc += lDelta;

            } while (--cScans != 0);
            break;
        }
        GWH_END_PKT1_PACKET(ulPktSize);

#ifdef H3_FIFO
    }
    else
    {
        ulPktSize = cdSrc + ( (cjSrc & 3) ? 1 : 0 );  // Closely tied to switch
        switch (cjSrc & 3)
        {
        case 3:
            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);
                // Because of the large packet size we know we don't have to check cdSrc > 0 here.
                H3_TRANSFER_DWORD(ppdev, pjH3Base, pjSrc, cdSrc);

                d = (ULONG) (*(pjSrc + cjEnd))          |
                            (*(pjSrc + cjEnd + 1) << 8) |
                            (*(pjSrc + cjEnd + 2) << 16);
                H3_TRANSFER_DWORD(ppdev, pjH3Base, &d, 1);
                GWH_END_PKT1_PACKET(ulPktSize);
                pjSrc += lDelta;

            } while (--cScans != 0);
            break;

        case 2:
            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);
                // Because of the large packet size we know we don't have to check cdSrc > 0 here.
                H3_TRANSFER_DWORD(ppdev, pjH3Base, pjSrc, cdSrc);

                d = (ULONG) (*(pjSrc + cjEnd))          |
                            (*(pjSrc + cjEnd + 1) << 8);
                H3_TRANSFER_DWORD(ppdev, pjH3Base, &d, 1);
                GWH_END_PKT1_PACKET(ulPktSize);
                pjSrc += lDelta;

            } while (--cScans != 0);
            break;

        case 1:
            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);
                // Because of the large packet size we know we don't have to check cdSrc > 0 here.
                H3_TRANSFER_DWORD(ppdev, pjH3Base, pjSrc, cdSrc);

                d = (ULONG) (*(pjSrc + cjEnd));
                H3_TRANSFER_DWORD(ppdev, pjH3Base, &d, 1);
                GWH_END_PKT1_PACKET(ulPktSize);
                pjSrc += lDelta;

            } while (--cScans != 0);
            break;

        case 0:
            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);
                H3_TRANSFER_DWORD(ppdev, pjH3Base, pjSrc, cdSrc);
                GWH_END_PKT1_PACKET(ulPktSize);
                pjSrc += lDelta;

            } while (--cScans != 0);
            break;
        }
    }
#endif

	//    CHECK_DATA_COMPLETE(ppdev);
    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* VOID vMmFillSolid
*
* Fills a list of rectangles with a solid colour.
*
\**************************************************************************/

VOID vMmFillSolid(              // Type FNFILL
PDEV*           ppdev,
LONG            c,              // Can't be zero
RECTL*          prcl,           // List of rectangles to be filled, in relative
                                //   coordinates
ULONG           rop4,           // rop4
RBRUSH_COLOR    rbc,            // Drawing colour is rbc.iSolidColor
POINTL*         pptlBrush)      // Not used
{
	// This routine also fills to offscreen rectangles so we must be
	// sure clipping is set correctly.
	
    BYTE*   pjH3Base = ppdev->pjH3Base;
    BYTE    rop3 = (BYTE) rop4;

    GWH_DECL;

    GWH_PROLOG;

    ASSERTDD(c > 0, "Can't handle zero rectangles");

    // It's quite likely that we've just been called from GDI, so it's
    // even more likely that the accelerator's graphics engine has been
    // sitting around idle.  Rather than doing a FIFO_WAIT(3) here and
    // then a FIFO_WAIT(3) before outputing the actual rectangle,
    // we can avoid an 'in' (which can be quite expensive, depending on
    // the card) by doing a single FIFO_WAIT(6) right off the bat:

	// We use a mono pattern consisting of all one bits to fill the
	// rectangles with the solid color in the colorFore register.

	if( c == 1 )  // Special case to save a packet
	{
        #if ENABLE_LINEAR_DFBS
        {
            DWORD bltDstFormat, dstPixelFormat;

            GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
            BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

            CHECK_FIFO_ROOM(ppdev, 2);
            GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     );
            SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
            SET(2, pjH3Base, dstFormat, bltDstFormat);
            GWH_END_2D_PACKET( 2 );
        }
        #endif

        CHECK_FIFO_ROOM(ppdev, 6);
        GWH_BEGIN_2D_PACKET(6, SSTCP_PKT2_PATTERN0ALIAS |
                               SSTCP_PKT2_PATTERN1ALIAS |
                               SSTCP_PKT2_COLORFORE     |
                               SSTCP_PKT2_DSTSIZE       |
                               SSTCP_PKT2_DSTXY         |
                               SSTCP_PKT2_COMMAND);

        SET(1, pjH3Base, pattern0alias, 0xffffffff );
        SET(2, pjH3Base, pattern1alias, 0xffffffff );
        SET(3, pjH3Base, colorFore, rbc.iSolidColor);
        SET(4, pjH3Base, dstSize, H3_PACKXY_FAST( prcl->right - prcl->left,
                                                  prcl->bottom - prcl->top ));
   	    SET(5, pjH3Base, dstXY, H3_PACKXY( prcl->left + ppdev->xOffset,
                                           prcl->top + ppdev->yOffset ));
        SET(6, pjH3Base, command, (rop3 << SSTG_ROP0_SHIFT)  |
                                           SSTG_MONO_PATTERN |
                                           SSTG_GO           |
                                           SSTG_RECTFILL );
        GWH_END_2D_PACKET( 6 );
	}
    else
    {
        #if ENABLE_LINEAR_DFBS
        {
            DWORD bltDstFormat, dstPixelFormat;

            GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
            BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

            CHECK_FIFO_ROOM(ppdev, 2);
            GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     );
            SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
            SET(2, pjH3Base, dstFormat, bltDstFormat);
            GWH_END_2D_PACKET( 2 );
        }
        #endif

        CHECK_FIFO_ROOM(ppdev, 4);
        GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_PATTERN0ALIAS |
                               SSTCP_PKT2_PATTERN1ALIAS |
                               SSTCP_PKT2_COLORFORE     |
                               SSTCP_PKT2_COMMAND);
        SET(1, pjH3Base, pattern0alias, 0xffffffff );
        SET(2, pjH3Base, pattern1alias, 0xffffffff );
        SET(3, pjH3Base, colorFore, rbc.iSolidColor);
        SET(4, pjH3Base, command, (rop3 << SSTG_ROP0_SHIFT)  |
                                        SSTG_MONO_PATTERN |
                                        SSTG_RECTFILL );
        GWH_END_2D_PACKET( 4 );

        while(TRUE)
        {
			WAXFIX_NOP_CODE

            CHECK_FIFO_ROOM(ppdev, 2);
            GWH_BEGIN_PKT4_PACKET(2, SSTCP_PKT4_DSTSIZE, 0x041);
            SET(1, pjH3Base, dstSize, H3_PACKXY_FAST( prcl->right - prcl->left,
                                                      prcl->bottom - prcl->top ));
            SET(2, pjH3Base, launch, H3_PACKXY( prcl->left + ppdev->xOffset,
                                                prcl->top + ppdev->yOffset ));
            GWH_END_PKT4_PACKET( 2 );

            if (--c == 0)
                break;

            prcl++;
        }
    }

    GWH_EPILOG;
}


#ifdef PATTERN_JUNK
/******************************Public*Routine******************************\
* VOID vMmFastPatRealize
*
* This routine transfers an 8x8 pattern to off-screen display memory,
* so that it can be used by the pattern hardware.
*
\**************************************************************************/

VOID vMmFastPatRealize(         // Type FNFASTPATREALIZE
PDEV*   ppdev,
RBRUSH* prb,                    // Points to brush realization structure
POINTL* pptlBrush,              // Brush origin for aligning realization
BOOL    bTransparent)           // FALSE for normal patterns; TRUE for
                                //   patterns with a mask when the background
                                //   mix is LEAVE_ALONE.
{
    BRUSHENTRY* pbe;
    LONG        iBrushCache;
    LONG        x;
    LONG        y;
    LONG        i;
    LONG        xShift;
    LONG        yShift;
    BYTE*       pjSrc;
    BYTE*       pjDst;
    LONG        cjLeft;
    LONG        cjRight;
    BYTE*       pjPattern;
    LONG        cwPattern;

    ULONG       aulBrush[TOTAL_BRUSH_SIZE];
                    // Temporary buffer for aligning brush.  Declared
                    //   as an array of ULONGs to get proper dword
                    //   alignment.  Also leaves room for brushes that
                    //   are up to 32bpp.  Note: this takes up 1/4k!

//    BYTE*       pjH3Base = ppdev->pjH3Base;

    pbe = prb->apbe[IBOARD(ppdev)];
    if ((pbe == NULL) || (pbe->prbVerify != prb))
    {
        // We have to allocate a new off-screen cache brush entry for
        // the brush:

        iBrushCache = ppdev->iBrushCache;
        pbe         = &ppdev->abe[iBrushCache];

        iBrushCache++;
        if (iBrushCache >= ppdev->cBrushCache)
            iBrushCache = 0;

        ppdev->iBrushCache = iBrushCache;

        // Update our links:

        pbe->prbVerify           = prb;
        prb->apbe[IBOARD(ppdev)] = pbe;
    }

    // Load some variables onto the stack, so that we don't have to keep
    // dereferencing their pointers:

    x = pbe->x;
    y = pbe->y;

    // Because we handle only 8x8 brushes, it is easy to compute the
    // number of pels by which we have to rotate the brush pattern
    // right and down.  Note that if we were to handle arbitrary sized
    // patterns, this calculation would require a modulus operation.
    //
    // The brush is aligned in absolute coordinates, so we have to add
    // in the surface offset:

    xShift = pptlBrush->x + ppdev->xOffset;
    yShift = pptlBrush->y + ppdev->yOffset;

    prb->ptlBrushOrg.x = xShift;    // We have to remember the alignment
    prb->ptlBrushOrg.y = yShift;    //   that we used for caching (we check
                                    //   this when we go to see if a brush's
                                    //   cache entry is still valid)

    xShift &= 7;                    // Rotate pattern 'xShift' pels right
    yShift &= 7;                    // Rotate pattern 'yShift' pels down

    prb->bTransparent = bTransparent;

    // I considered doing the colour expansion for 1bpp brushes in
    // software, but by letting the hardware do it, we don't have
    // to do as many OUTs to transfer the pattern.

    if (prb->fl & RBRUSH_2COLOR)
    {
        // We're going to do a colour-expansion ('across the plane')
        // bitblt of the 1bpp 8x8 pattern to the screen.  But first
        // we'll align it properly by copying it to a temporary buffer
        // (which we'll conveniently pack word aligned so that we can do a
        // REP OUTSW...)

        pjSrc = (BYTE*) &prb->aulPattern[0];    // Copy from the start of the
                                                //   brush buffer
        pjDst = (BYTE*) &aulBrush[0];           // Copy to our temp buffer
        pjDst += yShift * sizeof(WORD);         //   starting yShift rows down
        i = 8 - yShift;                         //   for 8 - yShift rows

        do {
            *pjDst = (*pjSrc >> xShift) | (*pjSrc << (8 - xShift));
            pjDst += sizeof(WORD);  // Destination is word packed
            pjSrc += sizeof(WORD);  // Source is word aligned too

        } while (--i != 0);

        pjDst -= 8 * sizeof(WORD);  // Move to the beginning of the source

        ASSERTDD(pjDst == (BYTE*) &aulBrush[0], "pjDst not back at start");

        for (; yShift != 0; yShift--)
        {
            *pjDst = (*pjSrc >> xShift) | (*pjSrc << (8 - xShift));
            pjDst += sizeof(WORD);  // Destination is word packed
            pjSrc += sizeof(WORD);  // Source is word aligned too
        }

        if (bTransparent)
        {
            IO_FIFO_WAIT(ppdev, 3);

            MM_PIX_CNTL(ppdev, pjMmBase, CPU_DATA);
            MM_FRGD_MIX(ppdev, pjMmBase, LOGICAL_1);
            MM_BKGD_MIX(ppdev, pjMmBase, LOGICAL_0);
        }
        else
        {
            IO_FIFO_WAIT(ppdev, 5);

            MM_PIX_CNTL(ppdev, pjMmBase, CPU_DATA);
            MM_FRGD_MIX(ppdev, pjMmBase, FOREGROUND_COLOR | OVERPAINT);
            MM_BKGD_MIX(ppdev, pjMmBase, BACKGROUND_COLOR | OVERPAINT);
            MM_FRGD_COLOR(ppdev, pjMmBase, prb->ulForeColor);
            MM_BKGD_COLOR(ppdev, pjMmBase, prb->ulBackColor);
        }

        IO_FIFO_WAIT(ppdev, 4);

        MM_ABS_CUR_X(ppdev, pjMmBase, x);
        MM_ABS_CUR_Y(ppdev, pjMmBase, y);
        MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, 7); // Brush is 8 wide
        MM_MIN_AXIS_PCNT(ppdev, pjMmBase, 7); // Brush is 8 high

        IO_GP_WAIT(ppdev);

        MM_CMD(ppdev, pjMmBase, RECTANGLE_FILL     | BUS_SIZE_16 | WAIT          |
                                DRAWING_DIR_TBLRXM | DRAW        | LAST_PIXEL_ON |
                                MULTIPLE_PIXELS    | WRITE       | BYTE_SWAP);

        CHECK_DATA_READY(ppdev);

        pjPattern = (BYTE*) &aulBrush[0];
        MM_TRANSFER_WORD_ALIGNED(ppdev, pjMmBase, pjPattern, 8);
                                                // Each word transferred
                                                //   comprises one row of the
                                                //   pattern, and there are
                                                //   8 rows in the pattern

        CHECK_DATA_COMPLETE(ppdev);
    }
    else
    {
        ASSERTDD(!bTransparent,
            "Shouldn't have been asked for transparency with a non-1bpp brush");

        // We're going to do a straight ('through the plane') bitblt
        // of the Xbpp 8x8 pattern to the screen.  But first we'll align
        // it properly by copying it to a temporary buffer:

        cjLeft  = CONVERT_TO_BYTES(xShift, ppdev);  // Number of bytes pattern
                                                    //   is shifted to the right
        cjRight = CONVERT_TO_BYTES(8, ppdev) -      // Number of bytes pattern
                  cjLeft;                           // is shifted to the left

        pjSrc = (BYTE*) &prb->aulPattern[0];        // Copy from brush buffer
        pjDst = (BYTE*) &aulBrush[0];               // Copy to our temp buffer
        pjDst += yShift * CONVERT_TO_BYTES(8, ppdev); //  starting yShift rows
        i = 8 - yShift;                             //  down for 8 - yShift rows

        do {
            RtlCopyMemory(pjDst + cjLeft, pjSrc,           cjRight);
            RtlCopyMemory(pjDst,          pjSrc + cjRight, cjLeft);

            pjDst += cjLeft + cjRight;
            pjSrc += cjLeft + cjRight;

        } while (--i != 0);

        pjDst = (BYTE*) &aulBrush[0];   // Move to the beginning of destination

        for (; yShift != 0; yShift--)
        {
            RtlCopyMemory(pjDst + cjLeft, pjSrc,           cjRight);
            RtlCopyMemory(pjDst,          pjSrc + cjRight, cjLeft);

            pjDst += cjLeft + cjRight;
            pjSrc += cjLeft + cjRight;

        }

        IO_FIFO_WAIT(ppdev, 6);

        MM_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
        MM_FRGD_MIX(ppdev, pjMmBase, SRC_CPU_DATA | OVERPAINT);

        MM_ABS_CUR_X(ppdev, pjMmBase, x);
        MM_ABS_CUR_Y(ppdev, pjMmBase, y);
        MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, 7);     // Brush is 8 wide
        MM_MIN_AXIS_PCNT(ppdev, pjMmBase, 7);     // Brush is 8 high

        IO_GP_WAIT(ppdev);

        MM_CMD(ppdev, pjMmBase, RECTANGLE_FILL     | BUS_SIZE_16| WAIT          |
                                DRAWING_DIR_TBLRXM | DRAW       | LAST_PIXEL_ON |
                                SINGLE_PIXEL       | WRITE      | BYTE_SWAP);

        CHECK_DATA_READY(ppdev);

        pjPattern = (BYTE*) &aulBrush[0];
        cwPattern = CONVERT_TO_BYTES((TOTAL_BRUSH_SIZE / 2), ppdev);
        MM_TRANSFER_WORD_ALIGNED(ppdev, pjMmBase, pjPattern, cwPattern);

        CHECK_DATA_COMPLETE(ppdev);
    }
}
#endif


/******************************Public*Routine******************************\
* VOID ulFlipPattern
*
* Translates monochrome format pattern to form that is easier to process
* in the pattern load routine.
*
\**************************************************************************/

#if defined( _MSC_VER )             // On Microsoft Compiler
#pragma warning ( disable : 4033 )  // Disable return value warning
#endif

__inline ULONG ulFlipPattern(
ULONG   ulPattern)
{
    __asm
    {
        mov     ebx, ulPattern
        mov     ecx,32

    a_loop:
        shl     ebx, 1
        rcr     eax, 1
        loop    a_loop
        bswap   eax
    }

    return;
}

#if defined( _MSC_VER )
#pragma warning ( default : 4033 )  // Enable return value warning
#endif


/******************************Public*Routine******************************\
* VOID vH3LoadPatternRegisters
*
* This routine loads an 8x8 pattern into the H3 pattern registers so that
* it can be used by the H3 pattern hardware.
*
\**************************************************************************/

VOID vH3LoadPatternRegisters(   // Type
PDEV*   ppdev,
RBRUSH* prb,                    // Points to brush realization structure
BOOL    bExpandMonoPattern)    // Needed for mono source cases
{
    ULONG*      pulBrush;
    ULONG*      pulDst;
    ULONG       culColorRegs;
    ULONG       ulPattern0;
    ULONG       ulPattern1;
    ULONG       i;
    LONG        j;
    BYTE*       pjSrc;
    ULONG       ulForeColor = prb->ulForeColor;
    ULONG       ulBackColor = prb->ulBackColor;
    ULONG       ulTmpColor;
    BYTE*       pjH3Base = ppdev->pjH3Base;

    GWH_DECL;

    GWH_PROLOG;

    pulDst = (ULONG *) ( (ULONG) pjH3Base + H3_2DREG_OFFSET(colorPattern) );

    if (prb->fl & RBRUSH_2COLOR)
    {
        if( !bExpandMonoPattern )
        {
            pulBrush = prb->aulPattern;             // Point to the pattern bits.
            ulPattern0 =  *pulBrush++;
            ulPattern1 =  *pulBrush;

            CHECK_FIFO_ROOM( ppdev, 4);
            GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_PATTERN0ALIAS |
                                   SSTCP_PKT2_PATTERN1ALIAS |
                                   SSTCP_PKT2_COLORBACK     |
                                   SSTCP_PKT2_COLORFORE);

            SET(1, pjH3Base, pattern0alias, ulPattern0 );
            SET(2, pjH3Base, pattern1alias, ulPattern1 );
            SET(3, pjH3Base, colorBack, ulBackColor);
            SET(4, pjH3Base, colorFore, ulForeColor);

            GWH_END_2D_PACKET( 4 );
        }
        else
        {
            pulBrush = prb->aulPattern;             // Point to the pattern bits.

#ifdef H3_FIFO
            // Calculate the number of color registers needed to hold the
            // complete pattern.  8 x 8 pattern so we need ( 8*8 ) * pelsize / 4.
            culColorRegs = (ppdev->cjPelSize << 4);
#endif

            CHECK_FIFO_ROOM( ppdev, culColorRegs);
            GWH_BEGIN_PKT1_PACKET(culColorRegs, SSTCP_PKT1_COLORPATTERN, SSTCP_PKT1_INC);

            switch( ppdev->cjPelSize )
            {
            case 4:
                for( j = 2; j > 0; j-- )
                {
                    ulPattern0 =  *pulBrush++;
                    ulPattern0 = ulFlipPattern( ulPattern0 );
                    for( i = 0; i < 32; i++ )
                    {
                        if( ulPattern0 & 0x1 )
                        {
                            SET_ABSOLUTE( pulDst++, ulForeColor );
                        }
                        else
                        {
                            SET_ABSOLUTE( pulDst++, ulBackColor );
                        }

                        ulPattern0 >>= 1;
                    }
                }
                break;

            case 3:
                for( j = 2; j > 0; j-- )
                {
                    ulPattern0 =  *pulBrush++;
                    ulPattern0 = ulFlipPattern( ulPattern0 );
                    for( i = 8; i > 0; i-- )
                    {
                        switch( ulPattern0 & 0x3 )
                        {
                        case 0:
                            SET_ABSOLUTE( pulDst++, (ulBackColor << 24) | ulBackColor);
                            break;
                        case 1:
                            SET_ABSOLUTE( pulDst++, (ulBackColor << 24) | ulForeColor);
                            break;
                        case 2:
                            SET_ABSOLUTE( pulDst++, (ulForeColor << 24) | ulBackColor);
                            break;
                        case 3:
                            SET_ABSOLUTE( pulDst++, (ulForeColor << 24) | ulForeColor);
                            break;
                        }

                        switch( ulPattern0 & 0x6 )
                        {
                        case 0:
                            SET_ABSOLUTE( pulDst++, (ulBackColor << 16) | (ulBackColor >> 8) );
                            break;
                        case 2:
                            SET_ABSOLUTE( pulDst++, (ulBackColor << 16) | (ulForeColor >> 8) );
                            break;
                        case 4:
                            SET_ABSOLUTE( pulDst++, (ulForeColor << 16) | (ulBackColor >> 8) );
                            break;
                        case 6:
                            SET_ABSOLUTE( pulDst++, (ulForeColor << 16) | (ulForeColor >> 8) );
                            break;
                        }

                        switch( ulPattern0 & 0xc )
                        {
                        case 0:
                            SET_ABSOLUTE( pulDst++, (ulBackColor << 8) | (ulBackColor >> 16) );
                            break;
                        case 4:
                            SET_ABSOLUTE( pulDst++, (ulBackColor << 8) | (ulForeColor >> 16) );
                            break;
                        case 8:
                            SET_ABSOLUTE( pulDst++, (ulForeColor << 8) | (ulBackColor >> 16) );
                            break;
                        case 0xc:
                            SET_ABSOLUTE( pulDst++, (ulForeColor << 8) | (ulForeColor >> 16) );
                            break;
                        }

                        ulPattern0 >>= 4;
                    }
                }
                break;

            case 2:
                for( j = 2; j > 0; j-- )
                {
                    ulPattern0 =  *pulBrush++;
                    ulPattern0 = ulFlipPattern( ulPattern0 );
                    for( i = 0; i < 16; i++ )
                    {
                        if( ulPattern0 & 0x2 )
                            ulTmpColor = ulForeColor << 16;
                        else
                            ulTmpColor = ulBackColor << 16;

                        if( ulPattern0 & 0x1 )
                        {
                            SET_ABSOLUTE( pulDst++, ulTmpColor | ulForeColor);
                        }
                        else
                        {
                            SET_ABSOLUTE( pulDst++, ulTmpColor | ulBackColor);
                        }

                        ulPattern0 >>= 2;
                    }
                }
                break;

            case 1:
                for( j = 2; j > 0; j-- )
                {
                    ulPattern0 =  *pulBrush++;
                    ulPattern0 = ulFlipPattern( ulPattern0 );
                    for( i = 0; i < 8; i++ )
                    {
                        if( ulPattern0 & 0x8 )
                            ulTmpColor = ulForeColor << 24;
                        else
                            ulTmpColor = ulBackColor << 24;

                        if( ulPattern0 & 0x4 )
                            ulTmpColor |= ulForeColor << 16;
                        else
                            ulTmpColor |= ulBackColor << 16;

                        if( ulPattern0 & 0x2 )
                            ulTmpColor |= ulForeColor << 8;
                        else
                            ulTmpColor |= ulBackColor << 8;

                        if( ulPattern0 & 0x1 )
                        {
                            SET_ABSOLUTE( pulDst++, ulTmpColor | ulForeColor);
                        }
                        else
                        {
                            SET_ABSOLUTE( pulDst++, ulTmpColor | ulBackColor);
                        }

                        ulPattern0 >>= 4;
                    }
                }
                break;
            }

            GWH_END_PKT1_PACKET(culColorRegs);
        }
    }
    else // Color brush - 8,16,24,32 BPP
    {
        pulBrush = prb->aulPattern;             // Point to the pattern bits.

        // Calculate the number of color registers needed to hold the
        // complete pattern.  8 x 8 pattern so we need ( 8*8 ) * pelsize / 4.
        culColorRegs = (ppdev->cjPelSize << 4);

        CHECK_FIFO_ROOM( ppdev, culColorRegs);
        GWH_BEGIN_PKT1_PACKET(culColorRegs, SSTCP_PKT1_COLORPATTERN, SSTCP_PKT1_INC);
        for (i = 0; i < culColorRegs ; i++)
        {
#if ENABLE_LOG_FILE
          SET_ABSOLUTE( pulDst++, *pulBrush );  // jdw - optimize for P5
          pulBrush++;
#else
            SET_ABSOLUTE( pulDst++, *pulBrush++ );  // jdw - optimize for P5
#endif
        }
        GWH_END_PKT1_PACKET(culColorRegs);
    }

    GWH_EPILOG;
}


/******************************Public*Routine******************************\
* VOID vH3LoadPatternSolid
*
* This routine loads a solid pattern into the H3 pattern registers so that
* it can be used by the H3 pattern hardware.
*
\**************************************************************************/

VOID vH3LoadPatternSolid(
PDEV*   ppdev,
ULONG   iSolidColor,        // Points to brush realization structure
BOOL    bColorExpandPattern)    // Needed for mono source cases
{
    LONG        i;
    ULONG*      pulDst;
    ULONG       ulRegValue1;
    ULONG       ulRegValue2;
    ULONG       ulRegValue3;
    BYTE*       pjH3Base = ppdev->pjH3Base;
    ULONG       culColorRegs;

    GWH_DECL;

    GWH_PROLOG;

    if( !bColorExpandPattern )
    {
        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_PATTERN0ALIAS |
                               SSTCP_PKT2_PATTERN1ALIAS |
                               SSTCP_PKT2_COLORFORE);
        SET(1, pjH3Base, pattern0alias, 0xffffffff );
        SET(2, pjH3Base, pattern1alias, 0xffffffff );
        SET(3, pjH3Base, colorFore, iSolidColor);
        GWH_END_2D_PACKET( 3 );
    }
    else
    {
#ifdef H3_FIFO
        // Calculate the number of color registers needed to hold the
        // complete pattern.  8 x 8 pattern so we need ( 8*8 ) * pelsize / 4.
        culColorRegs = (ppdev->cjPelSize << 4);
#else
        pulDst=(ULONG *) ( (ULONG) pjH3Base + H3_2DREG_OFFSET(colorPattern) );
#endif

        switch( ppdev->cjPelSize )
        {
        case 1:
            ulRegValue1 = (iSolidColor << 24) |
                          (iSolidColor << 16) |
                          (iSolidColor <<  8) |
                           iSolidColor;
            break;

        case 2:
            ulRegValue1 = (iSolidColor<<16) | iSolidColor;
            break;

        case 3:
            ulRegValue1 = (iSolidColor << 24) | iSolidColor;
            ulRegValue2 = (iSolidColor << 16) | (iSolidColor >>  8);
            ulRegValue3 = (iSolidColor <<  8) | (iSolidColor >> 16);
            break;

        case 4:
            ulRegValue1 = iSolidColor;
            break;
        }

        CHECK_FIFO_ROOM( ppdev, culColorRegs);
        GWH_BEGIN_PKT1_PACKET(culColorRegs, SSTCP_PKT1_COLORPATTERN, SSTCP_PKT1_INC);
        if( ppdev->cjPelSize != 3 )
        {
            for( i =  ppdev->cjPelSize << 4; i > 0 ; i-- )
            {
                SET_ABSOLUTE( pulDst++, ulRegValue1);
            }
        }
        else    // cjPelSize == 3
        {
            for( i =  16; i > 0 ; i-- )     // Load 48 color registers.
            {
                SET_ABSOLUTE( pulDst++, ulRegValue1);
                SET_ABSOLUTE( pulDst++, ulRegValue2);
                SET_ABSOLUTE( pulDst++, ulRegValue3);
            }
        }
        GWH_END_PKT1_PACKET(culColorRegs);

    }
    GWH_EPILOG;
}


/******************************Public*Routine******************************\
* VOID vMmFillPatFast
*
* This routine uses the H3 pattern hardware to draw a patterned list of
* rectangles.
*
\**************************************************************************/

VOID vMmFillPatFast(            // Type FNFILL
PDEV*           ppdev,
LONG            c,              // Can't be zero
RECTL*          prcl,           // List of rectangles to be filled, in relative
                                //   coordinates
ULONG           rop4,           // rop4
RBRUSH_COLOR    rbc,            // rbc.prb points to brush realization structure
POINTL*         pptlBrush)      // Pattern alignment
{
    BOOL        bTransparent;
    ULONG       h3cmd;
    LONG        xShift;
    LONG        yShift;
    BYTE        rop3 = (BYTE) rop4;
//    BRUSHENTRY* pbe;        // Pointer to brush entry data, which is used
//                            //   for keeping track of the location and status
//                            //   of the pattern bits cached in off-screen
//                            //   memory
    BYTE*       pjH3Base = ppdev->pjH3Base;

    GWH_DECL;

    // gwh_prolog delayed until pattern load complete

    ASSERTDD(c > 0, "Can't handle zero rectangles");

    bTransparent = (((rop4 >> 8) & 0xff) != (rop4 & 0xff));

    // Note we will not send transparent patterns to this routine
    // because H3 doesn't support transparent patterns used with a
    // rop (it does support transparent sources with rops).
    //
    ASSERTDD( bTransparent == 0,
             "Rops not supported with transparent patterns");

    // It doesn't make sense for us to maintain an off-screen brush
    // cache because it doesn't help our performance (maybe in AGP?),
    // so we can eliminate this if statement and simplify this routine.
    // vMmFastPatRealize won't be necessary for caching because the H3
    // hardware uses the same brush format as Windows(R).  All we will
    // need to do is load the brush.

//    if ((rbc.prb->ptlBrushOrg.x != pptlBrush->x + ppdev->xOffset) ||
//        (rbc.prb->ptlBrushOrg.y != pptlBrush->y + ppdev->yOffset) ||
//        (rbc.prb->apbe[IBOARD(ppdev)]->prbVerify != rbc.prb)      ||
//        (rbc.prb->bTransparent != bTransparent))
//    {
//        vMmFastPatRealize(ppdev, rbc.prb, pptlBrush, bTransparent);
//    }

    // jdw - Optimize.  Should add quick check to see if the pattern
    // currently loaded is the same as the one to be loaded.  This
    // would save a reload.

    vH3LoadPatternRegisters( ppdev, rbc.prb, FALSE );

//   Left this comment in to remind us of the restricted Rops here.
//     ulHwForeMix = gaulHwMixFromRop2[(rop4 >> 2) & 0xf]

    if ( rbc.prb->fl & RBRUSH_2COLOR )
        h3cmd = (rop3 << SSTG_ROP0_SHIFT)  |
                         SSTG_MONO_PATTERN |
                         SSTG_RECTFILL;
    else
        h3cmd = (rop3 << SSTG_ROP0_SHIFT)  |
                         SSTG_RECTFILL;

    // Because we handle only 8x8 brushes, it is easy to compute the
    // number of pels by which we have to rotate the brush pattern
    // left and up.  Note that if we were to handle arbitrary sized
    // patterns, this calculation would require a modulus operation.
    //
    // The brush is aligned in absolute coordinates, so we have to add
    // in the surface offset:

    xShift = -(pptlBrush->x + ppdev->xOffset);
    yShift = -(pptlBrush->y + ppdev->yOffset);

    xShift &= 7;                    // Rotate pattern 'xShift' pels left
    yShift &= 7;                    // Rotate pattern 'yShift' pels up


    h3cmd |= ( yShift << SSTG_Y_PATOFFSET_SHIFT )  |
             ( xShift << SSTG_X_PATOFFSET_SHIFT );

    GWH_PROLOG;

    #if ENABLE_LINEAR_DFBS
    {
        DWORD bltDstFormat, dstPixelFormat;

        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        CHECK_FIFO_ROOM(ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     );
        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        GWH_END_2D_PACKET( 2 );
    }
    #endif


    CHECK_FIFO_ROOM( ppdev, 3);
    GWH_BEGIN_PKT4_PACKET(3, SSTCP_PKT4_DSTSIZE, 0x045 );
    SET(1, pjH3Base, dstSize, H3_PACKXY( prcl->right - prcl->left,
                                         prcl->bottom - prcl->top ) );
    SET(2, pjH3Base, command, h3cmd );
    SET(3, pjH3Base, launch, H3_PACKXY( prcl->left + ppdev->xOffset,
                                        prcl->top + ppdev->yOffset ) );
    GWH_END_PKT4_PACKET( 3 );

    while (--c > 0)
    {
        prcl++;

		WAXFIX_NOP_CODE

        CHECK_FIFO_ROOM( ppdev, 2);
        GWH_BEGIN_PKT4_PACKET(2, SSTCP_PKT4_DSTSIZE, 0x041);
        SET(1, pjH3Base, dstSize, H3_PACKXY( prcl->right - prcl->left,
                                             prcl->bottom - prcl->top ) );
        SET(2, pjH3Base, launch, H3_PACKXY( prcl->left + ppdev->xOffset,
                                            prcl->top + ppdev->yOffset ) );
        GWH_END_PKT4_PACKET( 2 );
    }

    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* VOID vMmXfer1bpp
*
* This routine colour expands a monochrome bitmap, possibly with different
* Rop2's for the foreground and background.  It will be called in the
* following cases:
*
* 1) To colour-expand the monochrome text buffer for the vFastText routine.
* 2) To blt a 1bpp source with a simple Rop2 between the source and
*    destination.
* 3) To blt a true Rop3 when the source is a 1bpp bitmap that expands to
*    white and black, and the pattern is a solid colour.
* 4) To handle a true Rop4 that works out to be two Rop2's between the
*    pattern and destination.
*
* Needless to say, making this routine fast can leverage a lot of
* performance.
*
\**************************************************************************/

VOID vMmXfer1bpp(       // Type FNXFER
PDEV*       ppdev,
LONG        c,          // Count of rectangles, can't be zero
RECTL*      prcl,       // List of destination rectangles, in relative
                        //   coordinates
ROP4        rop4,       // rop4
SURFOBJ*    psoSrc,     // Source surface
POINTL*     pptlSrc,    // Original unclipped source point
RECTL*      prclDst,    // Original unclipped destination rectangle
XLATEOBJ*   pxlo)       // Translate that provides colour-expansion information
{
    LONG    dxSrc;
    LONG    dySrc;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    LONG    cjSrc;
    LONG    xLeft;
    LONG    yTop;
    LONG    xBias;
    ULONG   h3cmd;
    BYTE    rop3 = (BYTE) rop4;
    BYTE*   pjH3Base = ppdev->pjH3Base;

    GWH_DECL;

    GWH_PROLOG;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(pptlSrc != NULL && psoSrc != NULL, "Can't have NULL sources");
    ASSERTDD(((((rop4 & 0xff00) >> 8) == (rop4 & 0xff)) || (rop4 == 0xaacc)),
             "Expect weird rops only when opaquing");

    // Note that only our text routine calls us with a '0xaacc' rop:

    #if ENABLE_LINEAR_DFBS
    {
        DWORD bltDstFormat, dstPixelFormat;

        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        CHECK_FIFO_ROOM(ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     );
        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        GWH_END_2D_PACKET( 2 );
    }
    #endif

    CHECK_FIFO_ROOM(ppdev, 4);
    GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_SRCFORMAT     |
                           SSTCP_PKT2_SRCXY         |
                           SSTCP_PKT2_COLORBACK     |
                           SSTCP_PKT2_COLORFORE);
    SET(1, pjH3Base, srcFormat, SSTG_PIXFMT_1BPP | SSTG_SRC_PACK_32);
    SET(2, pjH3Base, srcXY, 0);				// We always align the source.
    SET(3, pjH3Base, colorBack, pxlo->pulXlate[0]);
    SET(4, pjH3Base, colorFore, pxlo->pulXlate[1]);
    GWH_END_2D_PACKET( 4 );

    dxSrc = pptlSrc->x - prclDst->left;
    dySrc = pptlSrc->y - prclDst->top;      // Add to destination to get source

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    do {
        // We'll byte align to the source, but do dword transfers
        // (implying that we may be doing unaligned reads from the
        // source).  We do this because it may reduce the total
        // number of dword outs/writes that we'll have to do to the
        // display:

		WAXFIX_NOP_CODE

        yTop  = prcl->top;
        xLeft = prcl->left;

        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_CLIP0MIN      |
                               SSTCP_PKT2_DSTSIZE       |
                               SSTCP_PKT2_DSTXY);

        xBias = (xLeft + dxSrc) & 7;        // This is the byte-align bias
        if (xBias != 0)
        {
            // We could either align in software or use the hardware to do
            // it.  We'll use the hardware; the cost we pay is the time spent
            // setting and resetting one clip register:

            SET(1, pjH3Base, clip0min, (xLeft + ppdev->xOffset) & 0xffff);
            xLeft -= xBias;
        }
        else
        {
            // Reset it here so we can move the bottom SET out of the loop
            SET(1, pjH3Base, clip0min, 0 );
        }


        cx = prcl->right  - xLeft;
        cy = prcl->bottom - yTop;

        SET(2, pjH3Base, dstSize, H3_PACKXY_FAST( cx, cy ) );
        SET(3, pjH3Base, dstXY, H3_PACKXY( xLeft + ppdev->xOffset,
                                           yTop  + ppdev->yOffset ));
        GWH_END_2D_PACKET( 3 );

        cjSrc = (cx + 7) >> 3;              // # bytes to transfer
        pjSrc = pjSrcScan0 + (yTop  + dySrc) * lSrcDelta
                           + ((xLeft + dxSrc) >> 3);
                                            // Start is byte aligned (note
                                            //   that we don't have to add
                                            //   xBias)

        h3cmd = ( rop3 << SSTG_ROP0_SHIFT) | SSTG_HOST_BLT;

        if(rop4 == 0xaacc)
            h3cmd |= SSTG_TRANSPARENT;

        GWH_EPILOG;
        vH3ImageTransferMm32(ppdev, pjSrc, lSrcDelta, cjSrc, cy, h3cmd );
        GWH_PROLOG;

        prcl++;
    } while (--c != 0);

    if (xBias != 0)
    {
        // Clipping is always at reset state on entry to this routine,
        // so restore clip0min to reset state.

        CHECK_FIFO_ROOM(ppdev, 1);
        GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_CLIP0MIN );
        SET(1, pjH3Base, clip0min, 0 );
        GWH_END_2D_PACKET( 1 );
    }

    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* VOID vXfer4bpp
*
* Does a 4bpp transfer from a bitmap to the screen.
*
* The reason we implement this is that a lot of resources are kept as 4bpp,
* and used to initialize DFBs, some of which we of course keep off-screen.
*
\**************************************************************************/

VOID vXfer4bpp(         // Type FNXFER
PDEV*       ppdev,
LONG        c,          // Count of rectangles, can't be zero
RECTL*      prcl,       // List of destination rectangles, in relative
                        //   coordinates
ULONG       rop4,       // Rop4
SURFOBJ*    psoSrc,     // Source surface
POINTL*     pptlSrc,    // Original unclipped source point
RECTL*      prclDst,    // Original unclipped destination rectangle
XLATEOBJ*   pxlo)       // Translate that provides colour-expansion information
{
    BYTE*   pjH3Base = ppdev->pjH3Base;
    LONG    xOffset;
    LONG    yOffset;
    LONG    cjPelSize;
    LONG    dx;
    LONG    dy;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    xSrc;
    LONG    iLoop;
    BYTE    jSrc;
    ULONG*  pulXlate;
    LONG    i;
    LONG    xAbsLeft;
    LONG    cjSrc;
    LONG    cwSrc;
    LONG    lSrcSkip;
    LONG    cxRem;
    ULONG   ul;
    ULONG   ul0;
    ULONG   ul1;
    ULONG   ulPktSize;
    BYTE    rop3 = (BYTE) rop4;

    GWH_DECL;

    GWH_PROLOG;

    ASSERTDD(psoSrc->iBitmapFormat == BMF_4BPP, "Source must be 4bpp");
    ASSERTDD(c > 0, "Can't handle zero rectangles");

    #if ENABLE_LINEAR_DFBS
    {
        DWORD bltDstFormat, dstPixelFormat;

        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        CHECK_FIFO_ROOM(ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     );
        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        GWH_END_2D_PACKET( 2 );
    }
    #endif

    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    cjPelSize = ppdev->cjPelSize;
    pulXlate  = pxlo->pulXlate;

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;     // Add to destination to get source

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    CHECK_FIFO_ROOM( ppdev, 2 );
    GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_SRCXY );
    SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat | SSTG_SRC_PACK_32);
    SET(2, pjH3Base, srcXY, 0);   // Zero for packed host blts.
    GWH_END_2D_PACKET( 2 );

    while(TRUE)
    {
        cx = prcl->right  - prcl->left;
        cy = prcl->bottom - prcl->top;

        xSrc     =  prcl->left + dx;
        pjSrc    =  pjSrcScan0 + (prcl->top + dy) * lSrcDelta + (xSrc >> 1);

        xAbsLeft = (xOffset + prcl->left);

		WAXFIX_NOP_CODE

        CHECK_FIFO_ROOM( ppdev, 4 );
        GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_CLIP0MIN      |
                               SSTCP_PKT2_DSTSIZE       |
                               SSTCP_PKT2_DSTXY         |
                               SSTCP_PKT2_COMMAND);

        SET(1, pjH3Base, clip0min, H3_PACKXY( xAbsLeft, 0 ) );

        xAbsLeft -= (xSrc & 1);         // Align to start of first source byte
        cx       += (xSrc & 1);

        SET(2, pjH3Base, dstSize, H3_PACKXY_FAST( cx, cy ) );
        SET(3, pjH3Base, dstXY, H3_PACKXY( xAbsLeft, yOffset + prcl->top) );
        SET(4, pjH3Base, command, ((ULONG) rop3 << SSTG_ROP0_SHIFT) |
                                                    SSTG_HOST_BLT );
        GWH_END_2D_PACKET( 4 );

        cjSrc    = (cx + 1) >> 1;   // Number of source bytes touched
        lSrcSkip = lSrcDelta - cjSrc;

        if (cjPelSize == 1)
        {
            // This part handles 8bpp output:

            cwSrc = (cjSrc >> 1);    // Number of whole source words

#ifdef H3_FIFO
            ulPktSize = cwSrc + (cjSrc & 1);
#endif
            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize);
                GWH_BEGIN_PKT1_PACKET(ulPktSize , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                for (i = cwSrc; i != 0; i--)
                {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    ul  |= (pulXlate[jSrc & 0xf] << 8);
                    jSrc = *pjSrc++;
                    ul  |= (pulXlate[jSrc >> 4] << 16);
                    ul  |= (pulXlate[jSrc & 0xf] << 24);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                }

                // Handle an odd end byte, if there is one:

                if (cjSrc & 1)
                {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    ul  |= (pulXlate[jSrc & 0xf] << 8);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                }

                pjSrc += lSrcSkip;

                GWH_END_PKT1_PACKET(ulPktSize);

            } while (--cy > 0);
        }
        else if (cjPelSize == 2)
        {
            // This part handles 16bpp output:

#ifdef H3_FIFO
            ulPktSize = cjSrc;
#endif
            do {
                i = cjSrc;

                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                do {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    ul  |= (pulXlate[jSrc & 0xf] << 16);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                } while (--i > 0);

                pjSrc += lSrcSkip;
                GWH_END_PKT1_PACKET(ulPktSize);

            } while (--cy > 0);
        }
        else if (cjPelSize == 4)
        {
            cjSrc    = cx >> 1;   // Number of whole source bytes touched
            cxRem    = cx & 1;

            // This part handles 32bpp output:

            do {
                i = cjSrc;

                CHECK_FIFO_ROOM( ppdev, cx );
                GWH_BEGIN_PKT1_PACKET(cx , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                while (i--)     // may be 0
                {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                    ul   = (pulXlate[jSrc & 0xf]);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                }
                if (cxRem)
                {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                }

                pjSrc += lSrcSkip;
                GWH_END_PKT1_PACKET(cx);
            } while (--cy > 0);
        }
        else
        {
            // This part handles packed 24bpp output:

            cwSrc = (cx >> 2);      // Number of whole source words
            cxRem = (cx & 3);
#ifdef H3_FIFO
            ulPktSize = cx - cwSrc;    // 3/4 * cx
#endif
            if (cxRem == 3)
            {
                // Merge this case into the whole word case:

                cwSrc++;
                cxRem = 0;
            }

            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                for (i = cwSrc; i != 0; i--)
                {
                    jSrc = *pjSrc++;
                    ul0  = (pulXlate[jSrc >> 4]);
                    ul1  = (pulXlate[jSrc & 0xf]);
                    ul   = ul0 | (ul1 << 24);
                    SET_PKT1_REG(pjH3Base, launch, ul);

                    jSrc = *pjSrc++;
                    ul0  = (pulXlate[jSrc >> 4]);
                    ul   = (ul1 >> 8) | (ul0 << 16);
                    SET_PKT1_REG(pjH3Base, launch, ul);

                    ul1  = (pulXlate[jSrc & 0xf]);
                    ul   = (ul1 << 8) | (ul0 >> 16);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                }

                if (cxRem > 0)
                {
                    jSrc = *pjSrc++;
                    ul0  = (pulXlate[jSrc >> 4]);
                    ul1  = (pulXlate[jSrc & 0xf]);
                    ul   = ul0 | (ul1 << 24);
                    SET_PKT1_REG(pjH3Base, launch, ul);

                    if (cxRem > 1)
                    {
                        ul = (ul1 >> 8);
                        SET_PKT1_REG(pjH3Base, launch, ul);
                    }
                }

                pjSrc += lSrcSkip;
                GWH_END_PKT1_PACKET(ulPktSize);

            } while (--cy > 0);
        }

        if (--c == 0)
        {
            // Restore the clipping:

            CHECK_FIFO_ROOM(ppdev, 1);
            GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_CLIP0MIN );
            SET(1, pjH3Base, clip0min, 0 );
            GWH_END_2D_PACKET( 1 );

            break;
        }

        prcl++;
    }
    GWH_EPILOG;
}

#pragma optimize( "a", on )

//#ifdef MS_VIEW
#pragma warning ( push )
#pragma warning ( disable : 4731 )
//#endif 

/******************************Public*Routine******************************\
* VOID vXfer8bpp
*
* Does a 8bpp transfer from a bitmap to the screen.
*
* The reason we implement this is that a lot of resources are kept as 8bpp,
* and used to initialize DFBs, some of which we of course keep off-screen.
*
\**************************************************************************/

VOID vXfer8bpp(         // Type FNXFER
PDEV*       ppdev,
LONG        c,          // Count of rectangles, can't be zero
RECTL*      prcl,       // List of destination rectangles, in relative
                        //   coordinates
ULONG       rop4,       // Rop4
SURFOBJ*    psoSrc,     // Source surface
POINTL*     pptlSrc,    // Original unclipped source point
RECTL*      prclDst,    // Original unclipped destination rectangle
XLATEOBJ*   pxlo)       // Translate that provides colour-expansion information
{
    BYTE*   pjH3Base = ppdev->pjH3Base;
    LONG    xOffset;
    LONG    yOffset;
    LONG    cjPelSize;
    LONG    dx;
    LONG    dy;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    xSrc;
    LONG    iLoop;
    ULONG*  pulXlate;
    LONG    i;
    LONG    xAbsLeft;
    LONG    cwSrc;
    LONG    cdSrc;
    LONG    lSrcSkip;
    LONG    cxRem;
    ULONG   ul;
    ULONG   ul0;
    ULONG   ul1;
    ULONG   ulPktSize;
    BYTE    rop3 = (BYTE) rop4;

    GWH_DECL;

    GWH_PROLOG;

    ASSERTDD(psoSrc->iBitmapFormat == BMF_8BPP, "Source must be 8bpp");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(pxlo->pulXlate != NULL, "Must be a translate");

    #if ENABLE_LINEAR_DFBS
    {
        DWORD bltDstFormat, dstPixelFormat;

        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        CHECK_FIFO_ROOM(ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     );
        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        GWH_END_2D_PACKET( 2 );
    }
    #endif

    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    cjPelSize = ppdev->cjPelSize;
    pulXlate  = pxlo->pulXlate;

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;     // Add to destination to get source

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    CHECK_FIFO_ROOM( ppdev, 2 );
    GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_SRCXY );
    SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat | SSTG_SRC_PACK_32);
    SET(2, pjH3Base, srcXY, 0);   // Zero for packed host blts.
    GWH_END_2D_PACKET( 2 );

    while(TRUE)
    {
        cx = prcl->right  - prcl->left;
        cy = prcl->bottom - prcl->top;

        xSrc     =  prcl->left + dx;
        pjSrc    =  pjSrcScan0 + (prcl->top + dy) * lSrcDelta + xSrc;
        xAbsLeft = (xOffset + prcl->left);

        CHECK_FIFO_ROOM( ppdev, 3 );
        GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_DSTSIZE       |
                               SSTCP_PKT2_DSTXY         |
                               SSTCP_PKT2_COMMAND);
        SET(1, pjH3Base, dstSize, H3_PACKXY_FAST( cx, cy ) );
        SET(2, pjH3Base, dstXY, H3_PACKXY( xAbsLeft, yOffset + prcl->top) );
        SET(3, pjH3Base, command, ((ULONG) rop3 << SSTG_ROP0_SHIFT) |
                                                   SSTG_HOST_BLT );
        GWH_END_2D_PACKET( 3 );

        lSrcSkip = lSrcDelta - cx;

        // We do some ASM here because Highend Winbench passes so much data
        // through this routine.

        if (cjPelSize == 2)
        {
            // This part handles 16bpp output:

            cdSrc = (cx >> 2);
            cxRem = (cx & 3);

#ifdef H3_FIFO
            ulPktSize = (cdSrc << 1)+ (cxRem+1)/2;
#endif
            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

#if defined( H3_FIFO ) && defined( _X86_ ) && !defined( CSIM )
                __asm
                {
                    mov eax, cdSrc
                    mov ebx, pjSrc
                    mov ecx, fifoPtr
                    test eax, eax
                    jz Do_Remainder_Pelsize2

                    push   ebp
                    push   edi
                    push   esi

                    mov edi, eax
                    mov ebp, pulXlate

#ifdef PERF_ASM_XLATE //STB_MLS
// stb code		trying to minimize memory accesses (no stack operations)
Data_Loop_Pelsize2:
					mov			eax, DWORD PTR [ebx]		// get 4 bytes
					mov			edx, eax					// save a copy
					add			ebx, 4						// point ebx to next 4 bytes
					and			eax, 0xff					// mask off low byte
					shr			edx, 8						// slide edx down 1 byte
					mov			esi, DWORD PTR [ebp+eax*4]	// translate first byte

					mov			eax, edx					// get original dword back (whats left of it)
					and			eax, 0xff					// mask off byte 2
				    mov			eax, DWORD PTR [ebp+eax*4]  // translate 2nd byte
				    shl			eax, 16						// move this word to upper half of dword
				    or			esi, eax					// first 2 words are done
					shr			edx, 8						// get 3rd byte ready

					mov			DWORD PTR [ecx], esi        // save first dword result
					
					mov			eax, edx					// get bytes 3 and 4
					shr			edx, 8						// set up byte 4 in edx
					and			eax, 0xff					// mask off 3rd byte
					mov			esi, DWORD PTR [ebp+eax*4]  // translate byte 3
					
					mov			edx, DWORD PTR [ebp+edx*4]  // translate byte 4
					shl			edx, 16						// move word 4 to upper half of dword
					or			esi, edx					// combine words 3 and 4

					mov			DWORD PTR [ecx+4], esi		// save 2nd dword result
					
					add			ecx, 8					    // move fifo ptr

				    dec			edi							// are we done yet?
					jnz			short Data_Loop_Pelsize2
				    	
#else

// original code - has a lot of memory accesses
Data_Loop_Pelsize2:
                    xor eax, eax
                    xor edx, edx
                    mov       al, BYTE PTR [ebx+1]
                    mov       dl, BYTE PTR [ebx]
                    lea       esi, DWORD PTR [ecx+4]
                    mov       eax, DWORD PTR [ebp+eax*4]
                    mov       edx, DWORD PTR [ebp+edx*4]
                    shl       eax, 16
                    or        eax, edx
                    xor       edx, edx
                    mov       dl, BYTE PTR [ebx+3]
                    mov       edx, DWORD PTR [ebp+edx*4]
                    shl       edx, 16
                    push      edx                  ; Paired w/ pop eax below
                    xor       edx, edx
                    mov       dl, BYTE PTR [ebx+2]
                    add       ebx, 4
                    mov       edx, DWORD PTR [ebp+edx*4]
                    mov       DWORD PTR [ecx], eax
                    add       ecx, 8
                    pop       eax                  ; Paired w/ push edx above
                    or        eax, edx
                    mov       DWORD PTR [esi], eax
                    dec       edi
                    jnz       short Data_Loop_Pelsize2

#endif  //PERF_ASM_XLATE //STB_MLS

                    pop   esi
                    pop   edi
                    pop   ebp

                    mov pjSrc, ebx
                    mov fifoPtr, ecx
Do_Remainder_Pelsize2:  ;
                }
#  ifdef DBG
                // Adjust dwords written.  Normally would be done with a bunch of GWH_INC_WSH's.
                ppdev->fifoData.writesSinceHeader += (cdSrc<<1);
#  endif
#else
                for (i = cdSrc; i != 0; i--)
                {
                    ul0 = (pulXlate[pjSrc[1]] << 16) | (pulXlate[pjSrc[0]]);
                    ul1 = (pulXlate[pjSrc[3]] << 16) | (pulXlate[pjSrc[2]]);
                    SET_PKT1_REG(pjH3Base, launch, ul0 );
                    SET_PKT1_REG(pjH3Base, launch, ul1 );
                    pjSrc += 4;
                }
#endif

                if (cxRem > 0)
                {
                    ul = pulXlate[*pjSrc++];
                    if (cxRem > 1)
                    {
                    	ul |= ( pulXlate[*pjSrc++] << 16);
                    }

                    SET_PKT1_REG( pjH3Base, launch, ul );

                    if (cxRem > 2)
                    {
#if ENABLE_LOG_FILE
                        SET_PKT1_REG( pjH3Base, launch, pulXlate[*pjSrc] );
                        pjSrc++;
#else
                        SET_PKT1_REG( pjH3Base, launch, pulXlate[*pjSrc++] );
#endif
                    }
                }

                GWH_END_PKT1_PACKET(ulPktSize);
                pjSrc += lSrcSkip;
            } while (--cy > 0);
        }
        else if (cjPelSize == 4)
        {
            // This part handles 32bpp output:

#ifdef H3_FIFO
            ulPktSize = cx;
#endif
            cdSrc = cx >> 2;
            cxRem = cx & 0x3;

            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

#if defined( H3_FIFO ) && defined( _X86_ ) && !defined( CSIM )
                __asm
                {
                    mov     eax, cdSrc
                    mov     i, eax
                    test    eax, eax
                    je      Do_Remainder_Pelsize4

#ifndef PERF_COPY_BITS_OPT	//STB_MLS
// don't really need these
                    push    esi
                    push    edi
#endif  //ndef PERF_COPY_BITS_OPT	//STB_MLS

                    mov     esi, pjSrc
                    mov     ebx, pulXlate
                    mov     edi, fifoPtr

#ifndef PERF_COPY_BITS_OPT	//STB_MLS
// 15 memory accesses in this loop	(original code)
Data_Loop_Pelsize4:
                    mov     eax, DWORD PTR [esi]
                    mov     ecx, 0xff
                    add     esi, 4
                    and     ecx, eax
                    mov     edx, [ebx+ecx*4]
                    mov     ecx, eax
                    mov     ul, edx
                    shr     ecx, 8
                    and     ecx, 0xff
                    mov     edx, [ebx+ecx*4]
                    mov     ecx, eax
                    shr     ecx, 16
                    and     ecx, 0xff
                    rol     eax, 8
                    and     eax, 0xff
                    mov     ecx, [ebx+ecx*4]
                    mov     eax, [ebx+eax*4]
                    push    eax
                    mov     eax, ul
                    mov     [edi], eax              // SETs
                    pop     eax
                    mov     [edi+4], edx
                    mov     [edi+8], ecx
                    mov     [edi+0xc], eax
                    add     edi, 0x10
                    mov     eax, i
                    dec     eax
                    mov     i, eax
                    jnz     short Data_Loop_Pelsize4

#else
// only 9 memory accesses in this loop
					mov		ecx, i						// get counter
Loop_Pelsize4:

					mov		eax, DWORD PTR [esi]		// get 4 bytes
					mov		edx, eax					// save a copy
					add		esi, 4						// point to next 4 bytes

					and		eax, 0xff					// mask off byte 1
					mov		eax, [ebx+eax*4]			// translate byte 1
					mov		[edi], eax					// save dword 1
					
					shr		edx, 8						
					mov		eax, edx					// get original dword again
					and 	eax, 0xff					// mask off byte 2
					mov		eax, [ebx+eax*4]			// translate byte 2
					mov		[edi+4], eax				// save dword 2

					shr		edx, 8
					mov		eax, edx					// get original dword again
					and 	eax, 0xff					// mask off byte 3
					mov		eax, [ebx+eax*4]			// translate byte 3
					mov		[edi+8], eax				// save dword 3

					shr		edx, 8
					mov		eax, [ebx+edx*4]			// translate byte 4
					mov		[edi+12], eax				// save dword 4

					add		edi, 0x10

					dec		ecx
                    jnz     short Loop_Pelsize4
#endif //PERF_COPY_BITS_OPT	//STB_MLS

                    mov     pjSrc, esi
                    mov     fifoPtr, edi

#ifndef PERF_COPY_BITS_OPT	//STB_MLS
// don't really need these
                    pop     edi
                    pop     esi
#endif //ndef PERF_COPY_BITS_OPT	//STB_MLS

Do_Remainder_Pelsize4:  ;
                }
#  ifdef DBG
                // Adjust dwords written.  Normally would be done with a bunch of GWH_INC_WSH's.
                ppdev->fifoData.writesSinceHeader += (cdSrc << 2);
#  endif
#else
                for (i = cdSrc; i != 0; i--)
                {
                    ul = *(ULONG *) pjSrc;
                    pjSrc += 4;

                    SET_PKT1_REG(pjH3Base, launch, pulXlate[ul & 0xff]);
                    SET_PKT1_REG(pjH3Base, launch, pulXlate[(ul >> 8) & 0xff]);
                    SET_PKT1_REG(pjH3Base, launch, pulXlate[(ul >> 16) & 0xff]);
                    SET_PKT1_REG(pjH3Base, launch, pulXlate[(ul >> 24) & 0xff]);
                }
#endif

                for (i = cxRem; i != 0; i--)
                {
#if ENABLE_LOG_FILE
                    SET_PKT1_REG(pjH3Base, launch, pulXlate[*pjSrc]);
                    pjSrc++;
#else
                    SET_PKT1_REG(pjH3Base, launch, pulXlate[*pjSrc++]);
#endif
                }

                pjSrc += lSrcSkip;
                GWH_END_PKT1_PACKET(ulPktSize);
            } while (--cy > 0);
        }
        else if (cjPelSize == 3)
        {
            // This part handles packed 24bpp output:

            cdSrc = (cx >> 2);
            cxRem = (cx & 3);

#ifdef H3_FIFO
            ulPktSize = cx - cdSrc;
#endif
            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

#if defined( H3_FIFO ) && defined( _X86_ ) && !defined( CSIM )
                __asm
                {
                    mov     eax, cdSrc
                    mov     i, eax
                    test    eax, eax
                    je      Do_Remainder_Pelsize3

#ifndef PERF_COPY_BITS_OPT	//STB_MLS
// don't really need these
                    push    esi
                    push    edi
#endif //ndef PERF_COPY_BITS_OPT	//STB_MLS

                    mov     esi, pjSrc
                    mov     ebx, pulXlate
                    mov     edi, fifoPtr

Data_Loop_Pelsize3:
                    mov     eax, DWORD PTR [esi]
                    mov     ecx, 0xff
                    mov     edx, eax
                    and     ecx, eax
                    shr     edx, 8
                    add     esi, 4
                    and     edx, 0xff
                    mov     ecx, [ebx+ecx*4]
                    mov     edx, [ebx+edx*4]
                    push    edx
                    shl     edx, 24
                    or      ecx, edx
                    mov     [edi], ecx
                    mov     ecx, eax
                    shr     ecx, 16
                    pop     edx
                    and     ecx, 0xff
                    shr     edx, 8
                    mov     ecx, [ebx+ecx*4]
                    push    ecx
                    shl     ecx, 16
                    or      edx, ecx
                    pop     ecx
                    rol     eax, 8
                    mov     [edi+4], edx
                    and     eax, 0xff
                    shr     ecx, 16
                    mov     edx, [ebx+eax*4]
                    shl     edx, 8
                    or      ecx, edx
                    mov     eax, i
                    mov     [edi+8], ecx
                    add     edi, 0x0c
                    dec     eax
                    mov     i, eax
                    jnz     short Data_Loop_Pelsize3

                    mov     pjSrc, esi
                    mov     fifoPtr, edi

#ifndef PERF_COPY_BITS_OPT	//STB_MLS
// don't need these
                    pop     edi
                    pop     esi
#endif //ndef PERF_COPY_BITS_OPT	//STB_MLS

Do_Remainder_Pelsize3:  ;
                }
#  ifdef DBG
                // Adjust dwords written.  Normally would be done with a bunch of GWH_INC_WSH's.
                ppdev->fifoData.writesSinceHeader += (cdSrc * 3);
#  endif
#else
                for (i = cdSrc; i != 0; i--)
                {
                    ul0  = (pulXlate[*pjSrc++]);
                    ul1  = (pulXlate[*pjSrc++]);
                    ul   = ul0 | (ul1 << 24);
                    SET_PKT1_REG(pjH3Base, launch, ul );

                    ul0  = (pulXlate[*pjSrc++]);
                    ul   = (ul1 >> 8) | (ul0 << 16);
                    SET_PKT1_REG(pjH3Base, launch, ul );

                    ul1  = (pulXlate[*pjSrc++]);
                    ul   = (ul1 << 8) | (ul0 >> 16);
                    SET_PKT1_REG(pjH3Base, launch, ul );
                }
#endif

                if (cxRem > 0)
                {
                    ul0 = (pulXlate[*pjSrc++]);
                    ul  = ul0;
                    if (cxRem > 1)
                    {
                        ul1 = (pulXlate[*pjSrc++]);
                        ul |= (ul1 << 24);
                        SET_PKT1_REG(pjH3Base, launch, ul );

                        ul = (ul1 >> 8);
                        if (cxRem > 2)
                        {
                            ul0 = (pulXlate[*pjSrc++]);
                            ul |= (ul0 << 16);
                            SET_PKT1_REG(pjH3Base, launch, ul );
                            ul = (ul0 >> 16);
                        }
                    }
                    SET_PKT1_REG(pjH3Base, launch, ul );
                }

                pjSrc += lSrcSkip;
                GWH_END_PKT1_PACKET(ulPktSize);
            } while (--cy > 0);
        }
        else // if (cjPelSize == 1)
        {
            // This part handles 8bpp output:

            cdSrc = (cx >> 2);
            cxRem = (cx & 3);

#ifdef H3_FIFO
            ulPktSize = cdSrc + ( cxRem > 0 ? 1 : 0);
#endif
            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                for (i = cdSrc; i != 0; i--)
                {
                    ul  = (pulXlate[*pjSrc++]);
                    ul |= (pulXlate[*pjSrc++] << 8);
                    ul |= (pulXlate[*pjSrc++] << 16);
                    ul |= (pulXlate[*pjSrc++] << 24);
                    SET_PKT1_REG(pjH3Base, launch, ul );
                }

                if (cxRem > 0)
                {
                    ul = (pulXlate[*pjSrc++]);
                    if (cxRem > 1)
                    {
                        ul |= (pulXlate[*pjSrc++] << 8);
                        if (cxRem > 2)
                        {
                            ul |= (pulXlate[*pjSrc++] << 16);
                        }
                    }
                    SET_PKT1_REG(pjH3Base, launch, ul );
                }

                pjSrc += lSrcSkip;
                GWH_END_PKT1_PACKET(ulPktSize);

            } while (--cy > 0);
        }

        if (--c == 0)
            break;

        prcl++;
    }
    GWH_EPILOG;
}

//#ifdef MS_VIEW
#pragma warning ( pop )
//#endif 
#pragma optimize( "", on )


/******************************Public*Routine******************************\
* VOID vXfer162432bpp
*
* Does either a 16, 24, or 32bpp transfer from a bitmap to the screen.
*
\**************************************************************************/

VOID vXfer162432bpp(         // Type FNXFER
PDEV*       ppdev,
LONG        c,          // Count of rectangles, can't be zero
RECTL*      prcl,       // List of destination rectangles, in relative
                        //   coordinates
ULONG       rop4,       // Rop4
SURFOBJ*    psoSrc,     // Source surface
POINTL*     pptlSrc,    // Original unclipped source point
RECTL*      prclDst,    // Original unclipped destination rectangle
XLATEOBJ*   pxlo)       // Translate that provides colour-expansion information
{
    BYTE*   pjH3Base = ppdev->pjH3Base;
    LONG    xOffset;
    LONG    yOffset;
    LONG    dx;
    LONG    dy;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    ULONG*  pulSrc;
    BYTE*   pjDst;
    LONG    xSrc;
    LONG    iLoop;
    LONG    i;
    LONG    xAbsLeft;
    LONG    cwSrc;
    LONG    cdSrc;
    LONG    lSrcSkip;
    LONG    cxRem;
    ULONG   ulPktSize;
    ULONG   ulSrcPelSize;
    ULONG   ulSrcFormat;
    ULONG   ulWastedBytesOnFront;
    BYTE    rop3 = (BYTE) rop4;

    GWH_DECL;

    GWH_PROLOG;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(pxlo->pulXlate == NULL, "Must not be a translate");

    #if ENABLE_LINEAR_DFBS
    {
        DWORD bltDstFormat, dstPixelFormat;

        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        CHECK_FIFO_ROOM(ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     );
        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        GWH_END_2D_PACKET( 2 );
    }
    #endif

    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;     // Add to destination to get source

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    switch( psoSrc->iBitmapFormat )
    {
    case BMF_24BPP:
        ulSrcFormat = SSTG_PIXFMT_24BPP | SSTG_SRC_PACK_SRC;
        ulSrcPelSize = 3;
        break;

    case BMF_32BPP:
        ulSrcFormat = SSTG_PIXFMT_32BPP | SSTG_SRC_PACK_SRC;
        ulSrcPelSize = 4;
        break;

    case BMF_16BPP:
    default:
        ulSrcFormat = SSTG_PIXFMT_16BPP | SSTG_SRC_PACK_SRC;
        ulSrcPelSize = 2;
    }
	
    while(TRUE)
    {
        cx = prcl->right  - prcl->left;
        cy = prcl->bottom - prcl->top;

        xSrc     =  prcl->left + dx;
        pulSrc   =  (ULONG *) (pjSrcScan0 + (prcl->top + dy) * lSrcDelta + xSrc * ulSrcPelSize);
        ulWastedBytesOnFront = (ULONG) pulSrc & 0x3;
        pulSrc   =  (ULONG *) ((ULONG) pulSrc & ~0x3);
        xAbsLeft = (xOffset + prcl->left);

        ulPktSize = (cx * ulSrcPelSize + ulWastedBytesOnFront + 3) >> 2;
        lSrcSkip = lSrcDelta / 4 - ulPktSize;

        CHECK_FIFO_ROOM( ppdev, 5 );
        GWH_BEGIN_2D_PACKET(5, SSTCP_PKT2_SRCFORMAT |
                               SSTCP_PKT2_SRCXY     |
                               SSTCP_PKT2_DSTSIZE   |
                               SSTCP_PKT2_DSTXY     |
                               SSTCP_PKT2_COMMAND);

        SET(1, pjH3Base, srcFormat, ulSrcFormat | (ulPktSize*4));
        SET(2, pjH3Base, srcXY, ulWastedBytesOnFront);
        SET(3, pjH3Base, dstSize, H3_PACKXY_FAST( cx, cy ) );
        SET(4, pjH3Base, dstXY, H3_PACKXY( xAbsLeft, yOffset + prcl->top) );
        SET(5, pjH3Base, command, ((ULONG) rop3 << SSTG_ROP0_SHIFT) |
                                                   SSTG_HOST_BLT );
        GWH_END_2D_PACKET( 5 );

        do {
            CHECK_FIFO_ROOM( ppdev, ulPktSize );
            GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

            for (i = ulPktSize; i != 0; i--)
            {
#if ENABLE_LOG_FILE
                SET_PKT1_REG(pjH3Base, launch, *pulSrc );
                pulSrc++;
#else
                SET_PKT1_REG(pjH3Base, launch, *pulSrc++ );
#endif
            }

            pulSrc += lSrcSkip;
            GWH_END_PKT1_PACKET(ulPktSize);
        } while (--cy > 0);


        if (--c == 0)
            break;

        prcl++;
    }
    GWH_EPILOG;
}


/******************************Public*Routine******************************\
* VOID vMmXferNative
*
* Transfers a bitmap that is the same colour depth as the display to
* the screen via the launch register, with no translation.
*
\**************************************************************************/

VOID vMmXferNative(     // Type FNXFER
PDEV*       ppdev,
LONG        c,          // Count of rectangles, can't be zero
RECTL*      prcl,       // Array of relative coordinates destination rectangles
ROP4        rop4,       // rop4
SURFOBJ*    psoSrc,     // Source surface
POINTL*     pptlSrc,    // Original unclipped source point
RECTL*      prclDst,    // Original unclipped destination rectangle
XLATEOBJ*   pxlo)       // Not used
{
    LONG    dx;
    LONG    dy;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    LONG    cjSrc;
    ULONG   h3cmd;
    BYTE    rop3 = (BYTE) rop4;
    BYTE*   pjH3Base = ppdev->pjH3Base;

    GWH_DECL;

    GWH_PROLOG;

    ASSERTDD((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL),
            "Can handle trivial xlate only");
    ASSERTDD(psoSrc->iBitmapFormat == ppdev->iBitmapFormat,
            "Source must be same colour depth as screen");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;     // Add to destination to get source

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    #if ENABLE_LINEAR_DFBS
    {
        DWORD bltDstFormat, dstPixelFormat;

        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        CHECK_FIFO_ROOM(ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     );
        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        GWH_END_2D_PACKET( 2 );
    }
    #endif

    // Use DWORD packing for source.  Also set srcXY later.
    CHECK_FIFO_ROOM(ppdev, 2);
    GWH_BEGIN_2D_PACKET( 2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_SRCXY );
    SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat | SSTG_SRC_PACK_32);
    SET(2, pjH3Base, srcXY, 0);       // We always align the source.
    GWH_END_2D_PACKET( 2 );

    while(TRUE)
    {
// DWF
		WAXFIX_NOP_CODE
// DWFE
        cx = prcl->right  - prcl->left;
        cy = prcl->bottom - prcl->top;

        CHECK_FIFO_ROOM(ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTSIZE | SSTCP_PKT2_DSTXY);
        SET(1, pjH3Base, dstSize, H3_PACKXY_FAST( cx, cy ) );
        SET(2, pjH3Base, dstXY, H3_PACKXY( prcl->left + ppdev->xOffset,
                                           prcl->top  + ppdev->yOffset ) );
        GWH_END_2D_PACKET( 2 );

        cjSrc = CONVERT_TO_BYTES(cx, ppdev);
        pjSrc = pjSrcScan0 + (prcl->top  + dy) * lSrcDelta
                + CONVERT_TO_BYTES((prcl->left + dx), ppdev);

        h3cmd = ( rop3 << SSTG_ROP0_SHIFT ) | SSTG_HOST_BLT;
        GWH_EPILOG;
        vH3ImageTransferMm32(ppdev, pjSrc, lSrcDelta, cjSrc, cy, h3cmd );
        GWH_PROLOG;

        if (--c == 0)
            break;

        prcl++;
    }

    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* VOID vMmCopyBlt
*
* Does a screen-to-screen blt of a list of rectangles.
*
\**************************************************************************/

VOID vMmCopyBlt(    // Type FNCOPY
PDEV*   ppdev,
LONG    c,          // Can't be zero
RECTL*  prcl,       // Array of relative coordinates destination rectangles
ULONG   rop4,       // rop4
POINTL* pptlSrc,    // Original unclipped source point
RECTL*  prclDst)    // Original unclipped destination rectangle
{
    LONG        dx;
    LONG        dy;     // Add delta to destination to get source
    LONG        cx;
    LONG        cy;     // Size of current rectangle - 1
    // Really only a subset of rop3 (ab, a=b).  No Pattern required.
    BYTE        rop3 = (BYTE) rop4;
    BYTE*       pjH3Base = ppdev->pjH3Base;
    #if ENABLE_LINEAR_DFBS
        DWORD bltDstFormat, dstPixelFormat;
        DWORD bltSrcFormat, srcPixelFormat;
    #endif

    GWH_DECL;

    GWH_PROLOG;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

	WAXFIX_NOP_CODE

    #if ENABLE_LINEAR_DFBS

        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        GETPIXELFORMAT(ppdev->cjPelSize, srcPixelFormat);

        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);
        BLTFMT(ppdev->lDeltaSrc, srcPixelFormat, bltSrcFormat);

        CHECK_FIFO_ROOM(ppdev, 4);
        GWH_BEGIN_2D_PACKET(4,
                            SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     |
                            SSTCP_PKT2_SRCBASEADDR   |
                            SSTCP_PKT2_SRCFORMAT     );

        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        SET(3, pjH3Base, srcBaseAddr, ppdev->fpVidMemSrc);
        SET(4, pjH3Base, srcFormat, bltSrcFormat);
        GWH_END_2D_PACKET( 4 );
    #endif


    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;

    // We have to use the proper blt alignment.

    #if ENABLE_LINEAR_DFBS
        // If the source and destination are different surfaces,
        // then they do not overlap.
        if (ppdev->fpVidMemDst !=  ppdev->fpVidMemSrc)
            goto Top_Down_Left_To_Right;
    #endif

    if (!OVERLAP(prclDst, pptlSrc))
        goto Top_Down_Left_To_Right;

    if (prclDst->top <= pptlSrc->y)
    {
        if (prclDst->left <= pptlSrc->x)
        {

Top_Down_Left_To_Right:

            CHECK_FIFO_ROOM(ppdev, 2);
            GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_COMMAND);
            #if ENABLE_LINEAR_DFBS
            SET(1, pjH3Base, srcFormat, bltSrcFormat );
            #else
            SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat );
            #endif
            SET(2, pjH3Base, command, (rop3 << SSTG_ROP0_SHIFT) | SSTG_BLT );
            GWH_END_2D_PACKET( 2 );

            do {
//                cx = prcl->right - prcl->left;
//                cy = prcl->bottom - prcl->top;

				WAXFIX_NOP_CODE

                CHECK_FIFO_ROOM(ppdev, 3);
                GWH_BEGIN_PKT4_PACKET(3, SSTCP_PKT4_DSTSIZE, 0x043 );

                SET(1, pjH3Base, dstSize, ( (prcl->bottom - prcl->top) << 16 ) |
                                            (prcl->right - prcl->left) );

                SET(2, pjH3Base, dstXY, H3_PACKXY( prcl->left + ppdev->xOffset,
                                                   prcl->top + ppdev->yOffset ));

                SET(3, pjH3Base, launch, ( (prcl->top + dy + ppdev->yOffset) << 16 ) |
                                           (prcl->left + dx + ppdev->xOffset) );

                GWH_END_PKT4_PACKET( 3 );

                prcl++;

            } while (--c > 0);
        }
        else
        {
        	// Top down, right to left.


            CHECK_FIFO_ROOM(ppdev, 2);
            GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_COMMAND);
            #if ENABLE_LINEAR_DFBS
            SET(1, pjH3Base, srcFormat, bltSrcFormat );
            #else
            SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat );
            #endif
            SET(2, pjH3Base, command, (rop3 << SSTG_ROP0_SHIFT)  |
                                               SSTG_XDIR         |
                                               SSTG_BLT );
            GWH_END_2D_PACKET( 2 );

            do {

				WAXFIX_NOP_CODE

                cx = prcl->right - prcl->left;
                cy = prcl->bottom - prcl->top;

                CHECK_FIFO_ROOM(ppdev, 3);
                GWH_BEGIN_PKT4_PACKET(3, SSTCP_PKT4_DSTSIZE, 0x043 );

                SET(1, pjH3Base, dstSize, H3_PACKXY_FAST(cx, cy) );

                SET(2, pjH3Base, dstXY, H3_PACKXY( prcl->right - 1 + ppdev->xOffset,
                                                   prcl->top       + ppdev->yOffset ));

                SET(3, pjH3Base, launch, ( (prcl->top + dy       + ppdev->yOffset) << 16 ) |
                                           (prcl->right - 1 + dx + ppdev->xOffset) );

                GWH_END_PKT4_PACKET( 3 );

                prcl++;

            } while (--c > 0);
        }
    }
    else
    {
    	// Bottom up.
        if (prclDst->left <= pptlSrc->x)
        {
        	// Bottom up, left to right.

            CHECK_FIFO_ROOM(ppdev, 2);
            GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_COMMAND);
            #if ENABLE_LINEAR_DFBS
            SET(1, pjH3Base, srcFormat, bltSrcFormat );
            #else
            SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat );
            #endif
            SET(2, pjH3Base, command, (rop3 << SSTG_ROP0_SHIFT)  |
                                               SSTG_YDIR         |
                                               SSTG_BLT );
            GWH_END_2D_PACKET( 2 );

            do {
                cx = prcl->right - prcl->left;
                cy = prcl->bottom - prcl->top;

				WAXFIX_NOP_CODE

				CHECK_FIFO_ROOM(ppdev, 3);
                GWH_BEGIN_PKT4_PACKET(3, SSTCP_PKT4_DSTSIZE, 0x043 );

                SET(1, pjH3Base, dstSize, H3_PACKXY_FAST(cx, cy) );

                SET(2, pjH3Base, dstXY, H3_PACKXY( prcl->left       + ppdev->xOffset,
                                                   prcl->bottom - 1 + ppdev->yOffset ));

                SET(3, pjH3Base, launch, ( (prcl->bottom - 1 + dy + ppdev->yOffset) << 16 ) |
                                           (prcl->left + dx       + ppdev->xOffset) );

                GWH_END_PKT4_PACKET( 3 );

                prcl++;

            } while (--c > 0);
        }
        else
        {
        	// Bottom up, right to left.

            CHECK_FIFO_ROOM(ppdev, 2);
            GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_COMMAND);
            #if ENABLE_LINEAR_DFBS
            SET(1, pjH3Base, srcFormat, bltSrcFormat );
            #else
            SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat );
            #endif
            SET(2, pjH3Base, command, (rop3 << SSTG_ROP0_SHIFT)  |
                                               SSTG_XDIR         |
                                               SSTG_YDIR         |
                                               SSTG_BLT );
            GWH_END_2D_PACKET( 2 );

            do {
                cx = prcl->right - prcl->left;
                cy = prcl->bottom - prcl->top;
				WAXFIX_NOP_CODE
                CHECK_FIFO_ROOM(ppdev, 3);
                GWH_BEGIN_PKT4_PACKET(3, SSTCP_PKT4_DSTSIZE, 0x043 );

                SET(1, pjH3Base, dstSize, H3_PACKXY_FAST(cx, cy) );

                SET(2, pjH3Base, dstXY, H3_PACKXY( prcl->right  - 1 + ppdev->xOffset,
                                                   prcl->bottom - 1 + ppdev->yOffset ));

                SET(3, pjH3Base, launch, ( (prcl->bottom - 1 + dy + ppdev->yOffset) << 16 ) |
                                           (prcl->right - 1 + dx + ppdev->xOffset) );

                GWH_END_PKT4_PACKET( 3 );

                prcl++;

            } while (--c > 0);
        }
    }
    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* VOID vPatCopyBlt
*
* Does a screen-to-screen blt of a list of rectangles using the pattern
* currently loaded in the pattern registers.
*
\**************************************************************************/

VOID vPatCopyBlt(    // Type FNCOPY
PDEV*           ppdev,
LONG            c,          // Can't be zero
RECTL*          prcl,       // Array of relative coordinates destination rectangles
ULONG           rop4,       // rop4
POINTL*         pptlSrc,    // Original unclipped source point
RECTL*          prclDst,    // Original unclipped destination rectangle
RBRUSH_COLOR    rbc,
POINTL*         pptlBrush,  // Pattern alignment
BOOL            bSolidColor)// Is this a solid pattern?
{
    LONG        dx;
    LONG        dy;     // Add delta to destination to get source
    LONG        cx;
    LONG        cy;     // Size of current rectangle - 1
    ULONG       h3cmd;
    LONG        xShift;
    LONG        yShift;
    BYTE*       pjH3Base = ppdev->pjH3Base;
    BYTE        rop3 = (BYTE) rop4;
    #if ENABLE_LINEAR_DFBS
        DWORD bltDstFormat, dstPixelFormat;
        DWORD bltSrcFormat, srcPixelFormat;
    #endif

    GWH_DECL;

    // GWH_PROLOG delayed until after pattern load.

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop3");
    ASSERTDD((rop3 >> 4) != (rop3 & 0xf), "Expected ROP with pattern");

    #if ENABLE_LINEAR_DFBS

        GWH_PROLOG;

        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        GETPIXELFORMAT(ppdev->cjPelSize, srcPixelFormat);

        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);
        BLTFMT(ppdev->lDeltaSrc, srcPixelFormat, bltSrcFormat);

        CHECK_FIFO_ROOM(ppdev, 4);
        GWH_BEGIN_2D_PACKET(4,
                            SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     |
                            SSTCP_PKT2_SRCBASEADDR   |
                            SSTCP_PKT2_SRCFORMAT     );

        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        SET(3, pjH3Base, srcBaseAddr, ppdev->fpVidMemSrc);
        SET(4, pjH3Base, srcFormat, bltSrcFormat);
        GWH_END_2D_PACKET( 4 );
        GWH_EPILOG;

    #endif

//    if ((rbc.prb->ptlBrushOrg.x != pptlBrush->x + ppdev->xOffset) ||
//        (rbc.prb->ptlBrushOrg.y != pptlBrush->y + ppdev->yOffset) ||
//        (rbc.prb->apbe[IBOARD(ppdev)]->prbVerify != rbc.prb)      ||
//        (rbc.prb->bTransparent != bTransparent))
//    {
//        vMmFastPatRealize(ppdev, rbc.prb, pptlBrush, bTransparent);
//    }

    // jdw - Optimize.  Should add quick check to see if the pattern
    // currently loaded is the same as the one to be loaded.  This
    // would save a reload.

//    vH3LoadPatternRegisters( ppdev, rbc.prb );

    // Because we handle only 8x8 brushes, it is easy to compute the
    // number of pels by which we have to rotate the brush pattern
    // left and up.  Note that if we were to handle arbitrary sized
    // patterns, this calculation would require a modulus operation.
    //
    // The brush is aligned in absolute coordinates, so we have to add
    // in the surface offset:

    xShift = -(pptlBrush->x + ppdev->xOffset);
    yShift = -(pptlBrush->y + ppdev->yOffset);

    xShift &= 7;                    // Rotate pattern 'xShift' pels left
    yShift &= 7;                    // Rotate pattern 'yShift' pels up


    if ( bSolidColor )
        vH3LoadPatternSolid( ppdev, rbc.iSolidColor, FALSE );
    else
        vH3LoadPatternRegisters( ppdev, rbc.prb, FALSE );

    if ( bSolidColor || (rbc.prb->fl & RBRUSH_2COLOR) )
        h3cmd = ( rop3   << SSTG_ROP0_SHIFT)          |
                ( yShift << SSTG_Y_PATOFFSET_SHIFT )  |
                ( xShift << SSTG_X_PATOFFSET_SHIFT )  |
                            SSTG_MONO_PATTERN         |
                            SSTG_BLT;
    else
        h3cmd = ( rop3   << SSTG_ROP0_SHIFT)          |
                ( yShift << SSTG_Y_PATOFFSET_SHIFT )  |
                ( xShift << SSTG_X_PATOFFSET_SHIFT )  |
                            SSTG_BLT;

    GWH_PROLOG;

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;

    // We have to use the proper blt alignment.

    #if ENABLE_LINEAR_DFBS
        // If the source and destination are different surfaces,
        // then they do not overlap.
        if (ppdev->fpVidMemDst !=  ppdev->fpVidMemSrc)
            goto Top_Down_Left_To_Right;
    #endif

    if (!OVERLAP(prclDst, pptlSrc))
        goto Top_Down_Left_To_Right;

    if (prclDst->top <= pptlSrc->y)
    {
        if (prclDst->left <= pptlSrc->x)
        {

Top_Down_Left_To_Right:

            CHECK_FIFO_ROOM(ppdev, 2);
            GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_COMMAND);
            #if ENABLE_LINEAR_DFBS
            SET(1, pjH3Base, srcFormat, bltSrcFormat );
            #else
            SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat );
            #endif
            SET(2, pjH3Base, command, h3cmd);
            GWH_END_2D_PACKET( 2 );

            do {
//                cx = prcl->right - prcl->left;
//                cy = prcl->bottom - prcl->top;

				WAXFIX_NOP_CODE

                CHECK_FIFO_ROOM(ppdev, 3);

                GWH_BEGIN_PKT4_PACKET(3, SSTCP_PKT4_DSTSIZE, 0x043 );

                SET(1, pjH3Base, dstSize, ( (prcl->bottom - prcl->top) << 16 ) |
                                            (prcl->right - prcl->left) );

                SET(2, pjH3Base, dstXY, H3_PACKXY( prcl->left + ppdev->xOffset,
                                                   prcl->top  + ppdev->yOffset ));

                SET(3, pjH3Base, launch, ( (prcl->top + dy + ppdev->yOffset) << 16 ) |
                                           (prcl->left + dx + ppdev->xOffset) );

                GWH_END_PKT4_PACKET( 3 );

                prcl++;

            } while (--c != 0);
        }
        else
        {
        	// Top down, right to left.

            CHECK_FIFO_ROOM(ppdev, 2);
            GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_COMMAND);
            #if ENABLE_LINEAR_DFBS
            SET(1, pjH3Base, srcFormat, bltSrcFormat );
            #else
            SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat );
            #endif
            SET(2, pjH3Base, command, h3cmd | SSTG_XDIR);
            GWH_END_2D_PACKET( 2 );

            do {
                cx = prcl->right - prcl->left;
                cy = prcl->bottom - prcl->top;
				WAXFIX_NOP_CODE
                CHECK_FIFO_ROOM(ppdev, 3);

                GWH_BEGIN_PKT4_PACKET(3, SSTCP_PKT4_DSTSIZE, 0x043 );

                SET(1, pjH3Base, dstSize, H3_PACKXY_FAST(cx, cy) );

                SET(2, pjH3Base, dstXY, H3_PACKXY( prcl->right - 1 + ppdev->xOffset,
                                                    prcl->top       + ppdev->yOffset ));

                SET(3, pjH3Base, launch, ( (prcl->top + dy       + ppdev->yOffset) << 16 ) |
                                          (prcl->right - 1 + dx + ppdev->xOffset) );

                GWH_END_PKT4_PACKET( 3 );

                prcl++;

            } while (--c != 0);
        }
    }
    else
    {
    	// Bottom up.
        if (prclDst->left <= pptlSrc->x)
        {
        	// Bottom up, left to right.

            CHECK_FIFO_ROOM(ppdev, 2);
            GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_COMMAND);
            #if ENABLE_LINEAR_DFBS
            SET(1, pjH3Base, srcFormat, bltSrcFormat );
            #else
            SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat );
            #endif
            SET(2, pjH3Base, command, h3cmd | SSTG_YDIR);
            GWH_END_2D_PACKET( 2 );

            do {
                cx = prcl->right - prcl->left;
                cy = prcl->bottom - prcl->top;
				WAXFIX_NOP_CODE
                CHECK_FIFO_ROOM(ppdev, 3);

                GWH_BEGIN_PKT4_PACKET(3, SSTCP_PKT4_DSTSIZE, 0x043 );

                SET(1, pjH3Base, dstSize, H3_PACKXY_FAST(cx, cy) );

                SET(2, pjH3Base, dstXY, H3_PACKXY( prcl->left       + ppdev->xOffset,
                                                   prcl->bottom - 1 + ppdev->yOffset ));

                SET(3, pjH3Base, launch, ( (prcl->bottom - 1 + dy + ppdev->yOffset) << 16 ) |
                                           (prcl->left + dx       + ppdev->xOffset) );

                GWH_END_PKT4_PACKET( 3 );

                prcl++;

            } while (--c != 0);
        }
        else
        {
        	// Bottom up, right to left.

            CHECK_FIFO_ROOM(ppdev, 2);
            GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_COMMAND);
            #if ENABLE_LINEAR_DFBS
            SET(1, pjH3Base, srcFormat, bltSrcFormat );
            #else
            SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat );
            #endif
            SET(2, pjH3Base, command, h3cmd | SSTG_XDIR | SSTG_YDIR );
            GWH_END_2D_PACKET( 2 );

            do {
                cx = prcl->right - prcl->left;
                cy = prcl->bottom - prcl->top;
				WAXFIX_NOP_CODE
                CHECK_FIFO_ROOM(ppdev, 3);

                GWH_BEGIN_PKT4_PACKET(3, SSTCP_PKT4_DSTSIZE, 0x043 );

                SET(1, pjH3Base, dstSize, H3_PACKXY_FAST(cx, cy) );

                SET(2, pjH3Base, dstXY, H3_PACKXY( prcl->right  - 1 + ppdev->xOffset,
                                                   prcl->bottom - 1 + ppdev->yOffset ));

                SET(3, pjH3Base, launch, ( (prcl->bottom - 1 + dy + ppdev->yOffset) << 16 ) |
                                           (prcl->right -1 + dx   + ppdev->xOffset) );

                GWH_END_PKT4_PACKET( 3 );

                prcl++;

            } while (--c != 0);
        }
    }
    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* VOID vPatXfer1bpp
*
* This routine colour expands a monochrome bitmap, using the same
* Rops for the foreground and background.
*
* This is the pattern version of vMmXfer1bpp
*
\**************************************************************************/

VOID vPatXfer1bpp(       // Type FNXFER
PDEV*       ppdev,
LONG        c,          // Count of rectangles, can't be zero
RECTL*      prcl,       // List of destination rectangles, in relative
                        //   coordinates
ROP4        rop4,       // rop4
SURFOBJ*    psoSrc,     // Source surface
POINTL*     pptlSrc,    // Original unclipped source point
RECTL*      prclDst,    // Original unclipped destination rectangle
XLATEOBJ*   pxlo,       // Translate that provides colour-expansion information
RBRUSH_COLOR    rbc,
POINTL*     pptlBrush,  // Pattern alignment
BOOL        bSolidColor)// Is this a solid pattern?
{
    LONG    dxSrc;
    LONG    dySrc;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    LONG    cjSrc;
    LONG    xLeft;
    LONG    yTop;
    LONG    xBias;
    ULONG   h3cmd;
    LONG    xShift;
    LONG    yShift;
    BYTE*   pjH3Base = ppdev->pjH3Base;
    BYTE    rop3 = (BYTE) rop4;

    GWH_DECL;

    // GWH_PROLOG delayed until after pattern load.

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(pptlSrc != NULL && psoSrc != NULL, "Can't have NULL sources");
    ASSERTDD( (((rop4 & 0xff00) >> 8) == (rop4 & 0xff)),
             "Only handle rop3s");

//    vH3LoadPatternRegisters( ppdev, rbc.prb );

    // Because we handle only 8x8 brushes, it is easy to compute the
    // number of pels by which we have to rotate the brush pattern
    // left and up.  Note that if we were to handle arbitrary sized
    // patterns, this calculation would require a modulus operation.
    //
    // The brush is aligned in absolute coordinates, so we have to add
    // in the surface offset:

    // We have to color expand a mono pattern or our colorFore and
    // colorBack for the pattern will conflict with the mono source
    // colors.
    if ( bSolidColor )
        vH3LoadPatternSolid( ppdev, rbc.iSolidColor, TRUE );
    else
        vH3LoadPatternRegisters( ppdev, rbc.prb, TRUE );

    xShift = -(pptlBrush->x + ppdev->xOffset);
    yShift = -(pptlBrush->y + ppdev->yOffset);

    xShift &= 7;                    // Rotate pattern 'xShift' pels left
    yShift &= 7;                    // Rotate pattern 'yShift' pels up

    GWH_PROLOG;

    #if ENABLE_LINEAR_DFBS
    {
        DWORD bltDstFormat, dstPixelFormat;

        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        CHECK_FIFO_ROOM(ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     );
        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        GWH_END_2D_PACKET( 2 );
    }
    #endif

    CHECK_FIFO_ROOM(ppdev, 4);
    GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_SRCFORMAT     |
                           SSTCP_PKT2_SRCXY         |
                           SSTCP_PKT2_COLORBACK     |
                           SSTCP_PKT2_COLORFORE);
    SET(1, pjH3Base, srcFormat, SSTG_PIXFMT_1BPP | SSTG_SRC_PACK_32);
    SET(2, pjH3Base, srcXY, 0);				// We always align the source.
    SET(3, pjH3Base, colorBack, pxlo->pulXlate[0]);
    SET(4, pjH3Base, colorFore, pxlo->pulXlate[1]);
    GWH_END_2D_PACKET( 4 );

    dxSrc = pptlSrc->x - prclDst->left;
    dySrc = pptlSrc->y - prclDst->top;      // Add to destination to get source

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    do {
        // We'll byte align to the source, but do dword transfers
        // (implying that we may be doing unaligned reads from the
        // source).  We do this because it may reduce the total
        // number of dword outs/writes that we'll have to do to the
        // display:
		WAXFIX_NOP_CODE
        yTop  = prcl->top;
        xLeft = prcl->left;

        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_CLIP0MIN      |
                               SSTCP_PKT2_DSTSIZE       |
                               SSTCP_PKT2_DSTXY);

        xBias = (xLeft + dxSrc) & 7;        // This is the byte-align bias
        if (xBias != 0)
        {
            // We could either align in software or use the hardware to do
            // it.  We'll use the hardware; the cost we pay is the time spent
            // setting and resetting one clip register:

            SET(1, pjH3Base, clip0min, (xLeft + ppdev->xOffset) & 0xffff);
            xLeft -= xBias;
        }
        else
        {
            // Reset it here so we can move the bottom SET out of the loop
            SET(1, pjH3Base, clip0min, 0 );
        }


        cx = prcl->right  - xLeft;
        cy = prcl->bottom - yTop;

        SET(2, pjH3Base, dstSize, H3_PACKXY_FAST(cx, cy) );
        SET(3, pjH3Base, dstXY, H3_PACKXY( xLeft + ppdev->xOffset,
                                           yTop  + ppdev->yOffset ));

        GWH_END_2D_PACKET( 3 );

        cjSrc = (cx + 7) >> 3;              // # bytes to transfer
        pjSrc = pjSrcScan0 + (yTop  + dySrc) * lSrcDelta
                           + ((xLeft + dxSrc) >> 3);
                                            // Start is byte aligned (note
                                            //   that we don't have to add
                                            //   xBias)

//        h3cmd = ( rop3 << SSTG_ROP0_SHIFT) | SSTG_HOST_BLT;

// We never use mono patterns here (we color expand them) because their
// colors would conflict with the mono source colors.
//    if ( bSolidColor || (rbc.prb->fl & RBRUSH_2COLOR) )
//        h3cmd = ( rop3   << SSTG_ROP0_SHIFT)          |
//                ( yShift << SSTG_Y_PATOFFSET_SHIFT )  |
//                ( xShift << SSTG_X_PATOFFSET_SHIFT )  |
//                            SSTG_MONO_PATTERN         |
//                            SSTG_HOST_BLT;
//    else
        h3cmd = ( rop3   << SSTG_ROP0_SHIFT)          |
                ( yShift << SSTG_Y_PATOFFSET_SHIFT )  |
                ( xShift << SSTG_X_PATOFFSET_SHIFT )  |
                            SSTG_HOST_BLT;

        GWH_EPILOG;
        vH3ImageTransferMm32(ppdev, pjSrc, lSrcDelta, cjSrc, cy, h3cmd );
        GWH_PROLOG;

        prcl++;
    } while (--c != 0);

    if (xBias != 0)
    {
        // Clipping is always at reset state on entry to this routine,
        // so restore clip0min to reset state.

        CHECK_FIFO_ROOM(ppdev, 1);
        GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_CLIP0MIN );
        SET(1, pjH3Base, clip0min, 0 );
        GWH_END_2D_PACKET( 1 );
    }

    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* VOID vPatXferNative
*
* Transfers a bitmap that is the same colour depth as the display to
* the screen via the launch register, with no translation.  Uses the
* pattern loaded in the pattern registers.
*
* This is the pattern version of vMmXferNative.
*
\**************************************************************************/

VOID vPatXferNative(     // Type FNXFER
PDEV*       ppdev,
LONG        c,          // Count of rectangles, can't be zero
RECTL*      prcl,       // Array of relative coordinates destination rectangles
ROP4        rop4,       // rop4
SURFOBJ*    psoSrc,     // Source surface
POINTL*     pptlSrc,    // Original unclipped source point
RECTL*      prclDst,    // Original unclipped destination rectangle
XLATEOBJ*   pxlo,       // Not used
RBRUSH_COLOR  rbc,
POINTL*     pptlBrush,  // Pattern alignment
BOOL        bSolidColor)// Is this a solid pattern?
{
    LONG    dx;
    LONG    dy;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    LONG    cjSrc;
    ULONG   h3cmd;
    LONG    xShift;
    LONG    yShift;
    BYTE*   pjH3Base = ppdev->pjH3Base;
    BYTE    rop3 = (BYTE) rop4;

    GWH_DECL;

    // GWH_PROLOG delayed until after pattern load.

    ASSERTDD((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL),
            "Can handle trivial xlate only");
    ASSERTDD(psoSrc->iBitmapFormat == ppdev->iBitmapFormat,
            "Source must be same colour depth as screen");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop3");

    #if ENABLE_LINEAR_DFBS
    {
        DWORD bltDstFormat, dstPixelFormat;

        GWH_PROLOG;
        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        CHECK_FIFO_ROOM(ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     );
        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        GWH_END_2D_PACKET( 2 );
        GWH_EPILOG;
    }
    #endif


    // Because we handle only 8x8 brushes, it is easy to compute the
    // number of pels by which we have to rotate the brush pattern
    // left and up.  Note that if we were to handle arbitrary sized
    // patterns, this calculation would require a modulus operation.
    //
    // The brush is aligned in absolute coordinates, so we have to add
    // in the surface offset:

    xShift = -(pptlBrush->x + ppdev->xOffset);
    yShift = -(pptlBrush->y + ppdev->yOffset);

    xShift &= 7;                    // Rotate pattern 'xShift' pels left
    yShift &= 7;                    // Rotate pattern 'yShift' pels up

    // We use bSolidColor because we can't tell from rbc whether it is a
    // solid color brush or not.

    if ( bSolidColor )
        vH3LoadPatternSolid( ppdev, rbc.iSolidColor, FALSE );
    else
        vH3LoadPatternRegisters( ppdev, rbc.prb, FALSE );

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;     // Add to destination to get source

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    if ( bSolidColor || (rbc.prb->fl & RBRUSH_2COLOR) )
        h3cmd = ( rop3   << SSTG_ROP0_SHIFT)          |
                ( yShift << SSTG_Y_PATOFFSET_SHIFT )  |
                ( xShift << SSTG_X_PATOFFSET_SHIFT )  |
                            SSTG_MONO_PATTERN         |
                            SSTG_HOST_BLT;
    else
        h3cmd = ( rop3   << SSTG_ROP0_SHIFT)          |
                ( yShift << SSTG_Y_PATOFFSET_SHIFT )  |
                ( xShift << SSTG_X_PATOFFSET_SHIFT )  |
                            SSTG_HOST_BLT;

    GWH_PROLOG;

	// Use DWORD packing for source.  Also set srcXY later.
    CHECK_FIFO_ROOM(ppdev, 2);
    GWH_BEGIN_2D_PACKET( 2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_SRCXY );
    SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat | SSTG_SRC_PACK_32);
    SET(2, pjH3Base, srcXY, 0);       // We always align the source.
    GWH_END_2D_PACKET( 2 );

    while(TRUE)
    {
        cx = prcl->right  - prcl->left;
        cy = prcl->bottom - prcl->top;
		WAXFIX_NOP_CODE
        CHECK_FIFO_ROOM(ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTSIZE | SSTCP_PKT2_DSTXY);

		SET(1, pjH3Base, dstSize, H3_PACKXY_FAST( cx, cy ) );

		SET(2, pjH3Base, dstXY, H3_PACKXY( prcl->left + ppdev->xOffset,
                                           prcl->top  + ppdev->yOffset ) );
        GWH_END_2D_PACKET( 2 );

        cjSrc = CONVERT_TO_BYTES(cx, ppdev);
        pjSrc = pjSrcScan0 + (prcl->top  + dy) * lSrcDelta
                + CONVERT_TO_BYTES((prcl->left + dx), ppdev);

        GWH_EPILOG;
        vH3ImageTransferMm32(ppdev, pjSrc, lSrcDelta, cjSrc, cy, h3cmd );
        GWH_PROLOG;

        if (--c == 0)   // To save a couple of cycles, move prolog below
            break;      // here, change this to return and eliminate
                        // epilog from end of function.  Is it worth it?
        prcl++;
    }

    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* VOID vPatXfer4bpp
*
* Does a 4bpp transfer from a bitmap to the screen using a pattern.
*
* The reason we implement this is that a lot of resources are kept as 4bpp,
* and used to initialize DFBs, some of which we of course keep off-screen.
*
\**************************************************************************/

VOID vPatXfer4bpp(         // Type FNXFER
PDEV*       ppdev,
LONG        c,          // Count of rectangles, can't be zero
RECTL*      prcl,       // List of destination rectangles, in relative
                        //   coordinates
ULONG       rop4,       // Rop4
SURFOBJ*    psoSrc,     // Source surface
POINTL*     pptlSrc,    // Original unclipped source point
RECTL*      prclDst,    // Original unclipped destination rectangle
XLATEOBJ*   pxlo,       // Translate that provides colour-expansion information
RBRUSH_COLOR  rbc,
POINTL*     pptlBrush,  // Pattern alignment
BOOL        bSolidColor)// Is this a solid pattern?
{
    BYTE*   pjH3Base = ppdev->pjH3Base;
    LONG    xOffset;
    LONG    yOffset;
    LONG    cjPelSize;
    LONG    dx;
    LONG    dy;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    xSrc;
    LONG    iLoop;
    BYTE    jSrc;
    ULONG*  pulXlate;
    LONG    i;
    LONG    xAbsLeft;
    LONG    cjSrc;
    LONG    cwSrc;
    LONG    lSrcSkip;
    LONG    cxRem;
    ULONG   ul;
    ULONG   ul0;
    ULONG   ul1;
    ULONG   h3cmd;
    LONG    xShift;
    LONG    yShift;
    ULONG   ulPktSize;
    BYTE    rop3 = (BYTE) rop4;

    GWH_DECL;

    // GWH_PROLOG delayed until after pattern load.

    ASSERTDD(psoSrc->iBitmapFormat == BMF_4BPP, "Source must be 4bpp");
    ASSERTDD(c > 0, "Can't handle zero rectangles");

    #if ENABLE_LINEAR_DFBS
    {
        DWORD bltDstFormat, dstPixelFormat;

        GWH_PROLOG;

        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        CHECK_FIFO_ROOM(ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     );
        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        GWH_END_2D_PACKET( 2 );
        GWH_EPILOG;
    }
    #endif

    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    cjPelSize = ppdev->cjPelSize;
    pulXlate  = pxlo->pulXlate;

    // Because we handle only 8x8 brushes, it is easy to compute the
    // number of pels by which we have to rotate the brush pattern
    // left and up.  Note that if we were to handle arbitrary sized
    // patterns, this calculation would require a modulus operation.
    //
    // The brush is aligned in absolute coordinates, so we have to add
    // in the surface offset:

    xShift = -(pptlBrush->x + ppdev->xOffset);
    yShift = -(pptlBrush->y + ppdev->yOffset);

    xShift &= 7;                    // Rotate pattern 'xShift' pels left
    yShift &= 7;                    // Rotate pattern 'yShift' pels up

    // We use bSolidColor because we can't tell from rbc whether it is a
    // solid color brush or not.

    if ( bSolidColor )
        vH3LoadPatternSolid( ppdev, rbc.iSolidColor, FALSE );
    else
        vH3LoadPatternRegisters( ppdev, rbc.prb, FALSE );

    if ( bSolidColor || (rbc.prb->fl & RBRUSH_2COLOR) )
        h3cmd = ( rop3   << SSTG_ROP0_SHIFT)          |
                ( yShift << SSTG_Y_PATOFFSET_SHIFT )  |
                ( xShift << SSTG_X_PATOFFSET_SHIFT )  |
                            SSTG_MONO_PATTERN         |
                            SSTG_HOST_BLT;
    else
        h3cmd = ( rop3   << SSTG_ROP0_SHIFT)          |
                ( yShift << SSTG_Y_PATOFFSET_SHIFT )  |
                ( xShift << SSTG_X_PATOFFSET_SHIFT )  |
                            SSTG_HOST_BLT;

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;     // Add to destination to get source

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    GWH_PROLOG;

    CHECK_FIFO_ROOM( ppdev, 2 );
    GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_SRCXY );
    SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat | SSTG_SRC_PACK_32);
    SET(2, pjH3Base, srcXY, 0);   // Zero for packed host blts.
    GWH_END_2D_PACKET( 2 );

    while(TRUE)
    {
		WAXFIX_NOP_CODE
        cx = prcl->right  - prcl->left;
        cy = prcl->bottom - prcl->top;

        xSrc     =  prcl->left + dx;
        pjSrc    =  pjSrcScan0 + (prcl->top + dy) * lSrcDelta + (xSrc >> 1);

        xAbsLeft = (xOffset + prcl->left);

        CHECK_FIFO_ROOM( ppdev, 4 );
        GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_CLIP0MIN      |
                               SSTCP_PKT2_DSTSIZE       |
                               SSTCP_PKT2_DSTXY         |
                               SSTCP_PKT2_COMMAND);

        SET(1, pjH3Base, clip0min, H3_PACKXY( xAbsLeft, 0 ) );

        xAbsLeft -= (xSrc & 1);         // Align to start of first source byte
        cx       += (xSrc & 1);

        SET(2, pjH3Base, dstSize, H3_PACKXY_FAST( cx, cy ) );
        SET(3, pjH3Base, dstXY, H3_PACKXY( xAbsLeft, yOffset + prcl->top) );
        SET(4, pjH3Base, command, h3cmd );

        GWH_END_2D_PACKET( 4 );

        cjSrc    = (cx + 1) >> 1;   // Number of source bytes touched
        lSrcSkip = lSrcDelta - cjSrc;

        if (cjPelSize == 1)
        {
            // This part handles 8bpp output:

            cwSrc = (cjSrc >> 1);    // Number of whole source words

#ifdef H3_FIFO
            ulPktSize = cwSrc + (cjSrc & 1);
#endif
            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                for (i = cwSrc; i != 0; i--)
                {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    ul  |= (pulXlate[jSrc & 0xf] << 8);
                    jSrc = *pjSrc++;
                    ul  |= (pulXlate[jSrc >> 4] << 16);
                    ul  |= (pulXlate[jSrc & 0xf] << 24);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                }

                // Handle an odd end byte, if there is one:

                if (cjSrc & 1)
                {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    ul  |= (pulXlate[jSrc & 0xf] << 8);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                }

                pjSrc += lSrcSkip;

                GWH_END_PKT1_PACKET(ulPktSize);

            } while (--cy > 0);
        }
        else if (cjPelSize == 2)
        {
            // This part handles 16bpp output:

            do {
                i = cjSrc;
                CHECK_FIFO_ROOM( ppdev, cjSrc );
                GWH_BEGIN_PKT1_PACKET(cjSrc , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                do {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    ul  |= (pulXlate[jSrc & 0xf] << 16);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                } while (--i > 0);

                pjSrc += lSrcSkip;

                GWH_END_PKT1_PACKET(cjSrc);

            } while (--cy > 0);
        }
        else if (cjPelSize == 4)
        {
            cjSrc    = cx >> 1;   // Number of whole source bytes touched
            cxRem    = cx & 1;

            // This part handles 32bpp output:

            do {
                i = cjSrc;

                CHECK_FIFO_ROOM( ppdev, cx );
                GWH_BEGIN_PKT1_PACKET(cx , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                while (i--)     // may be 0
                {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                    ul   = (pulXlate[jSrc & 0xf]);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                }
                if (cxRem)
                {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                }

                pjSrc += lSrcSkip;
                GWH_END_PKT1_PACKET(cx);
            } while (--cy > 0);
        }
        else
        {
            // This part handles packed 24bpp output:

            cwSrc = (cx >> 2);      // Number of whole source words
            cxRem = (cx & 3);
#ifdef H3_FIFO
            ulPktSize = cx - cwSrc;    // 3/4 * cx
#endif
            if (cxRem == 3)
            {
                // Merge this case into the whole word case:

                cwSrc++;
                cxRem = 0;
            }

            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                for (i = cwSrc; i != 0; i--)
                {
                    jSrc = *pjSrc++;
                    ul0  = (pulXlate[jSrc >> 4]);
                    ul1  = (pulXlate[jSrc & 0xf]);
                    ul   = ul0 | (ul1 << 24);
                    SET_PKT1_REG(pjH3Base, launch, ul);

                    jSrc = *pjSrc++;
                    ul0  = (pulXlate[jSrc >> 4]);
                    ul   = (ul1 >> 8) | (ul0 << 16);
                    SET_PKT1_REG(pjH3Base, launch, ul);

                    ul1  = (pulXlate[jSrc & 0xf]);
                    ul   = (ul1 << 8) | (ul0 >> 16);
                    SET_PKT1_REG(pjH3Base, launch, ul);
                }

                if (cxRem > 0)
                {
                    jSrc = *pjSrc++;
                    ul0  = (pulXlate[jSrc >> 4]);
                    ul1  = (pulXlate[jSrc & 0xf]);
                    ul   = ul0 | (ul1 << 24);
                    SET_PKT1_REG(pjH3Base, launch, ul);

                    if (cxRem > 1)
                    {
                        ul = (ul1 >> 8);
                        SET_PKT1_REG(pjH3Base, launch, ul);
                    }
                }

                pjSrc += lSrcSkip;
                GWH_END_PKT1_PACKET(ulPktSize);

            } while (--cy > 0);
        }

        if (--c == 0)
        {
            // Restore the clipping:

            CHECK_FIFO_ROOM(ppdev, 1);
            GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_CLIP0MIN );
            SET(1, pjH3Base, clip0min, 0 );
            GWH_END_2D_PACKET( 1 );

            break;
        }

        prcl++;
    }
    GWH_EPILOG;
}

//#ifdef MS_VIEW
#pragma warning ( push )
#pragma warning ( disable : 4731 )
//#endif 

/******************************Public*Routine******************************\
* VOID vPatXfer8bpp
*
* Does a 8bpp transfer from a bitmap to the screen using a pattern.
*
* The reason we implement this is that a lot of resources are kept as 8bpp,
* and used to initialize DFBs, some of which we of course keep off-screen.
*
\**************************************************************************/

VOID vPatXfer8bpp(         // Type FNXFER
PDEV*       ppdev,
LONG        c,          // Count of rectangles, can't be zero
RECTL*      prcl,       // List of destination rectangles, in relative
                        //   coordinates
ULONG       rop4,       // Rop4
SURFOBJ*    psoSrc,     // Source surface
POINTL*     pptlSrc,    // Original unclipped source point
RECTL*      prclDst,    // Original unclipped destination rectangle
XLATEOBJ*   pxlo,       // Translate that provides colour-expansion information
RBRUSH_COLOR  rbc,
POINTL*     pptlBrush,  // Pattern alignment
BOOL        bSolidColor)// Is this a solid pattern?
{
    BYTE*   pjH3Base = ppdev->pjH3Base;
    LONG    xOffset;
    LONG    yOffset;
    LONG    cjPelSize;
    LONG    dx;
    LONG    dy;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    xSrc;
    LONG    iLoop;
    ULONG*  pulXlate;
    LONG    i;
    LONG    xAbsLeft;
    LONG    cwSrc;
    LONG    cdSrc;
    LONG    lSrcSkip;
    LONG    cxRem;
    ULONG   ul;
    ULONG   ul0;
    ULONG   ul1;
    ULONG   h3cmd;
    LONG    xShift;
    LONG    yShift;
    ULONG   ulPktSize;
    BYTE    rop3 = (BYTE) rop4;

    GWH_DECL;


    ASSERTDD(psoSrc->iBitmapFormat == BMF_8BPP, "Source must be 8bpp");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(pxlo->pulXlate != NULL, "Must be a translate");

    #if ENABLE_LINEAR_DFBS
    {
        DWORD bltDstFormat, dstPixelFormat;

        GWH_PROLOG;
        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        CHECK_FIFO_ROOM(ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_DSTBASEADDR   |
                            SSTCP_PKT2_DSTFORMAT     );
        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        GWH_END_2D_PACKET( 2 );
        GWH_EPILOG;
    }
    #endif

    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    cjPelSize = ppdev->cjPelSize;
    pulXlate  = pxlo->pulXlate;

    // Because we handle only 8x8 brushes, it is easy to compute the
    // number of pels by which we have to rotate the brush pattern
    // left and up.  Note that if we were to handle arbitrary sized
    // patterns, this calculation would require a modulus operation.
    //
    // The brush is aligned in absolute coordinates, so we have to add
    // in the surface offset:

    xShift = -(pptlBrush->x + ppdev->xOffset);
    yShift = -(pptlBrush->y + ppdev->yOffset);

    xShift &= 7;                    // Rotate pattern 'xShift' pels left
    yShift &= 7;                    // Rotate pattern 'yShift' pels up

    // We use bSolidColor because we can't tell from rbc whether it is a
    // solid color brush or not.

    if ( bSolidColor )
        vH3LoadPatternSolid( ppdev, rbc.iSolidColor, FALSE );
    else
        vH3LoadPatternRegisters( ppdev, rbc.prb, FALSE );

    if ( bSolidColor || (rbc.prb->fl & RBRUSH_2COLOR) )
        h3cmd = ( rop3   << SSTG_ROP0_SHIFT)          |
                ( yShift << SSTG_Y_PATOFFSET_SHIFT )  |
                ( xShift << SSTG_X_PATOFFSET_SHIFT )  |
                            SSTG_MONO_PATTERN         |
                            SSTG_HOST_BLT;
    else
        h3cmd = ( rop3   << SSTG_ROP0_SHIFT)          |
                ( yShift << SSTG_Y_PATOFFSET_SHIFT )  |
                ( xShift << SSTG_X_PATOFFSET_SHIFT )  |
                            SSTG_HOST_BLT;

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;     // Add to destination to get source

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    GWH_PROLOG;

    CHECK_FIFO_ROOM( ppdev, 2 );
    GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_SRCXY );
    SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat | SSTG_SRC_PACK_32);
    SET(2, pjH3Base, srcXY, 0);   // Zero for packed host blts.
    GWH_END_2D_PACKET( 2 );

    while(TRUE)
    {
        cx = prcl->right  - prcl->left;
        cy = prcl->bottom - prcl->top;

        xSrc     =  prcl->left + dx;
        pjSrc    =  pjSrcScan0 + (prcl->top + dy) * lSrcDelta + xSrc;
        xAbsLeft = (xOffset + prcl->left);

        CHECK_FIFO_ROOM( ppdev, 3 );
        GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_DSTSIZE       |
                               SSTCP_PKT2_DSTXY         |
                               SSTCP_PKT2_COMMAND);

        SET(1,  pjH3Base, dstSize, H3_PACKXY_FAST( cx, cy ) );

        SET(2,  pjH3Base, dstXY, H3_PACKXY( xAbsLeft, yOffset + prcl->top) );

        SET(3,  pjH3Base, command, h3cmd );

        GWH_END_2D_PACKET( 3 );

        lSrcSkip = lSrcDelta - cx;

        if (cjPelSize == 2)
        {
            // This part handles 16bpp output:

            cdSrc = (cx >> 2);
            cxRem = (cx & 3);

#ifdef H3_FIFO
            ulPktSize = (cdSrc << 1)+ (cxRem+1)/2;
#endif
            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

#if defined( H3_FIFO ) && defined( _X86_ ) && !defined( CSIM )
                __asm
                {
                    mov eax, cdSrc
                    mov ebx, pjSrc
                    mov ecx, fifoPtr
                    test eax, eax
                    jz Do_Remainder_Pelsize2

                    push   ebp
                    push   edi
                    push   esi

                    mov edi, eax
                    mov ebp, pulXlate
Data_Loop_Pelsize2:
                    xor eax, eax
                    xor edx, edx
                    mov       al, BYTE PTR [ebx+1]
                    mov       dl, BYTE PTR [ebx]
                    lea       esi, DWORD PTR [ecx+4]
                    mov       eax, DWORD PTR [ebp+eax*4]
                    mov       edx, DWORD PTR [ebp+edx*4]
                    shl       eax, 16
                    or        eax, edx
                    xor       edx, edx
                    mov       dl, BYTE PTR [ebx+3]
                    mov       edx, DWORD PTR [ebp+edx*4]
                    shl       edx, 16
                    push      edx                  ; Paired w/ pop eax below
                    xor       edx, edx
                    mov       dl, BYTE PTR [ebx+2]
                    add       ebx, 4
                    mov       edx, DWORD PTR [ebp+edx*4]
                    mov       DWORD PTR [ecx], eax
                    add       ecx, 8
                    pop       eax                  ; Paired w/ push edx above
                    or        eax, edx
                    mov       DWORD PTR [esi], eax
                    dec       edi
                    jnz       Data_Loop_Pelsize2

                    pop   esi
                    pop   edi
                    pop   ebp

                    mov pjSrc, ebx
                    mov fifoPtr, ecx
Do_Remainder_Pelsize2:  ;
                }
#  ifdef DBG
                // Adjust dwords written.  Normally would be done with a bunch of GWH_INC_WSH's.
                ppdev->fifoData.writesSinceHeader += (cdSrc<<1);
#  endif
#else
                for (i = cdSrc; i != 0; i--)
                {
                    ul0 = (pulXlate[pjSrc[1]] << 16) | (pulXlate[pjSrc[0]]);
                    ul1 = (pulXlate[pjSrc[3]] << 16) | (pulXlate[pjSrc[2]]);
                    SET_PKT1_REG(pjH3Base, launch, ul0 );
                    SET_PKT1_REG(pjH3Base, launch, ul1 );
                    pjSrc += 4;
                }
#endif

                if (cxRem > 0)
                {
                    ul = pulXlate[*pjSrc++];
                    if (cxRem > 1)
                    {
                    	ul |= ( pulXlate[*pjSrc++] << 16);
                    }

                    SET_PKT1_REG( pjH3Base, launch, ul );

                    if (cxRem > 2)
                    {
#if ENABLE_LOG_FILE
                        SET_PKT1_REG( pjH3Base, launch, pulXlate[*pjSrc] );
                        pjSrc++;
#else
                        SET_PKT1_REG( pjH3Base, launch, pulXlate[*pjSrc++] );
#endif
                    }
                }

                GWH_END_PKT1_PACKET(ulPktSize);
                pjSrc += lSrcSkip;
            } while (--cy > 0);
        }
        else if (cjPelSize == 4)
        {
            // This part handles 32bpp output:

            cdSrc = cx;

            do {
                CHECK_FIFO_ROOM( ppdev, cdSrc );
                GWH_BEGIN_PKT1_PACKET(cdSrc, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                for (i = cdSrc; i != 0; i--)
                {
                    ul  = (pulXlate[*pjSrc++]);
                    SET_PKT1_REG(pjH3Base, launch, ul );
                }

                pjSrc += lSrcSkip;
                GWH_END_PKT1_PACKET(cdSrc);
            } while (--cy > 0);
        }
        else if (cjPelSize == 3)
        {
            // This part handles packed 24bpp output:

            cdSrc = (cx >> 2);
            cxRem = (cx & 3);

#ifdef H3_FIFO
            ulPktSize = cx - cdSrc;
#endif
            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                for (i = cdSrc; i != 0; i--)
                {
                    ul0  = (pulXlate[*pjSrc++]);
                    ul1  = (pulXlate[*pjSrc++]);
                    ul   = ul0 | (ul1 << 24);
                    SET_PKT1_REG(pjH3Base, launch, ul );

                    ul0  = (pulXlate[*pjSrc++]);
                    ul   = (ul1 >> 8) | (ul0 << 16);
                    SET_PKT1_REG(pjH3Base, launch, ul );

                    ul1  = (pulXlate[*pjSrc++]);
                    ul   = (ul1 << 8) | (ul0 >> 16);
                    SET_PKT1_REG(pjH3Base, launch, ul );
                }

                if (cxRem > 0)
                {
                    ul0 = (pulXlate[*pjSrc++]);
                    ul  = ul0;
                    if (cxRem > 1)
                    {
                        ul1 = (pulXlate[*pjSrc++]);
                        ul |= (ul1 << 24);
                        SET_PKT1_REG(pjH3Base, launch, ul );

                        ul = (ul1 >> 8);
                        if (cxRem > 2)
                        {
                            ul0 = (pulXlate[*pjSrc++]);
                            ul |= (ul0 << 16);
                            SET_PKT1_REG(pjH3Base, launch, ul );
                            ul = (ul0 >> 16);
                        }
                    }
                    SET_PKT1_REG(pjH3Base, launch, ul );
                }

                pjSrc += lSrcSkip;

                GWH_END_PKT1_PACKET(ulPktSize);

            } while (--cy > 0);
        }
        else // if (cjPelSize == 1)
        {
            // This part handles 8bpp output:

            cdSrc = (cx >> 2);
            cxRem = (cx & 3);

#ifdef H3_FIFO
            ulPktSize = cdSrc + (cxRem > 0 ? 1 : 0);
#endif
            do {
                CHECK_FIFO_ROOM( ppdev, ulPktSize );
                GWH_BEGIN_PKT1_PACKET(ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                for (i = cdSrc; i != 0; i--)
                {
                    ul  = (pulXlate[*pjSrc++]);
                    ul |= (pulXlate[*pjSrc++] << 8);
                    ul |= (pulXlate[*pjSrc++] << 16);
                    ul |= (pulXlate[*pjSrc++] << 24);
                    SET_PKT1_REG(pjH3Base, launch, ul );
                }

                if (cxRem > 0)
                {
                    ul = (pulXlate[*pjSrc++]);
                    if (cxRem > 1)
                    {
                        ul |= (pulXlate[*pjSrc++] << 8);
                        if (cxRem > 2)
                        {
                            ul |= (pulXlate[*pjSrc++] << 16);
                        }
                    }
                    SET_PKT1_REG(pjH3Base, launch, ul );
                }

                pjSrc += lSrcSkip;
                GWH_END_PKT1_PACKET(ulPktSize);

            } while (--cy > 0);
        }

        if (--c == 0)
            break;

        prcl++;
    }
    GWH_EPILOG;
}

//#ifdef MS_VIEW
#pragma warning ( pop )
//#endif

#if (_WIN32_WINNT >= 0x0500)

#define PEL32_FROM_RGB( r, g, b ) \
            (   ((r)         & 0x00ff0000) | \
                (((g) >> 8)  & 0x0000ff00) | \
                (((b) >> 16) & 0x000000ff)    )


#define PEL24_FROM_RGB( r, g, b ) \
            (   ((r)         & 0x00ff0000) | \
                (((g) >> 8)  & 0x0000ff00) | \
                (((b) >> 16) & 0x000000ff)    )


#define PEL16_FROM_RGB( r, g, b, dither ) \
            (   ((((r) + dither) >> 8)       & 0xf800) | \
                ((((g) + (dither>>1)) >> 13) & 0x07e0) | \
                ((((b) + dither) >> 19)      & 0x001f)      )


#define PEL16_FROM_RGB_NODITHER( r, g, b ) \
            (   (((r) >> 8)  & 0xf800) |   \
                (((g) >> 13) & 0x07e0) |   \
                (((b) >> 19) & 0x001f)      )


static ULONG gaulDitherTable16[4][4] =
{
        { 0x00000, 0x40000, 0x10000, 0x50000 },
        { 0x60000, 0x20000, 0x70000, 0x30000 },
        { 0x18000, 0x58000, 0x08000, 0x48000 },
        { 0x78000, 0x38000, 0x68000, 0x28000 }
};

//-----------------------------------------------------------------------------
//
// void vGradientFillRect(GFNPB * ppb)
//
// Shades the specified primitives.
//
// Argumentes needed from function block (GFNPB)
//
//  ppdev-------PPDev
//  pdsurfDst----Destination surface
//  lNumRects---Number of rectangles to fill
//  pRects------Pointer to a list of rectangles information which needed to be
//              filled
//  ulMode------Specifies the current drawing mode and how to interpret the
//              array to which pMesh points
//  ptvrt-------Points to an array of TRIVERTEX structures, with each entry
//              containing position and color information.
//  ulNumTvrt---Specifies the number of TRIVERTEX structures in the array to
//              which pVertex points
//  pvMesh------Points to an array of structures that define the connectivity
//              of the TRIVERTEX elements to which ptvrt points
//  ulNumMesh---Specifies the number of elements in the array to which pvMesh
//              points
//
//-----------------------------------------------------------------------------
VOID
vGradientFillRect(GFNPB * ppb)
{
    DSURF*          pdsurfDst = ppb->pdsurfDst;
    RECTL*          prcl = ppb->pRects;
    LONG            c = ppb->lNumRects;
    PDEV*           ppdev = pdsurfDst->ppdev;
    // DWORD           windowBase = pdsurfDst->ulPixOffset;
    TRIVERTEX       *ptvrt = ppb->ptvrt;
    GRADIENT_RECT   *pgr;
    GRADIENT_RECT   *pgrSentinel = ((GRADIENT_RECT *) ppb->pvMesh)
                                 + ppb->ulNumMesh;
    LONG            xShift;
    LONG            yShift;
    ULONG           ulPktSize;
#if ENABLE_LINEAR_DFBS
    DWORD bltDstFormat, dstPixelFormat;
    DWORD bltSrcFormat, srcPixelFormat;
#endif
#ifndef H3_FIFO
    BYTE*           pjH3Base = ppdev->pjH3Base;
#endif

    GWH_DECL;

    GWH_PROLOG;

    DISPDBG((10, "vGradientFillRect"));

    // VALIDATE_GDI_CONTEXT;

    // setup loop invariant state

    ppdev->fpVidMemDst = pdsurfDst->fpVidMem;   // We know we have a dst at this point

#if ENABLE_LINEAR_DFBS

    #if ENABLE_TILED_HEAP
        if (ppdev->fpVidMemDst & SSTG_IS_TILED)
            ppdev->lDeltaDst = _FF(ddTileStride);
        else
    #endif
            ppdev->lDeltaDst = pdsurfDst->lDelta;

    GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
    BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

    CHECK_FIFO_ROOM(ppdev, 3);
    GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_DSTBASEADDR   |
                           SSTCP_PKT2_DSTFORMAT     |
                           SSTCP_PKT2_SRCBASEADDR  );
    SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
    SET(2, pjH3Base, dstFormat, bltDstFormat);
    SET(3, pjH3Base, srcBaseAddr, ppdev->fpVidMemDst);  // srcBase == dstBase
    GWH_END_2D_PACKET( 3 );

    // We'll set the srcFormat later
#endif

    while(c--)
    {

        pgr = (GRADIENT_RECT *) ppb->pvMesh;

        while(pgr < pgrSentinel)
        {
            TRIVERTEX   *ptrvtLr = ptvrt + pgr->LowerRight;
            TRIVERTEX   *ptrvtUl = ptvrt + pgr->UpperLeft;
            LONG        rd;
            LONG        gd;
            LONG        bd;
            LONG        dx;
            LONG        dy;
            RECTL       rect;
            LONG        rdx;
            LONG        rdy;
            LONG        gdx;
            LONG        gdy;
            LONG        bdx;
            LONG        bdy;
            LONG        rs;
            LONG        gs;
            LONG        bs;
            LONG        lTemp;
            BOOL        bReverseH = FALSE;
            BOOL        bReverseV = FALSE;

            rect.left = ptrvtUl->x;
            rect.right = ptrvtLr->x;
            rect.top = ptrvtUl->y;
            rect.bottom = ptrvtLr->y;

            if ( rect.left > rect.right )
            {
                //
                // The fill is from right to left. So we need to swap
                // the rectangle coordinates
                //
                lTemp = rect.left;
                rect.left = rect.right;
                rect.right = lTemp;

                bReverseH = TRUE;
            }

            if ( rect.top > rect.bottom )
            {
                //
                // The coordinate is from bottom to top. So we need to swap
                // the rectangle coordinates
                //
                lTemp = rect.top;
                rect.top = rect.bottom;
                rect.bottom = lTemp;

                bReverseV = TRUE;
            }

            // quick clipping reject
            if(prcl->left >= rect.right ||
               prcl->right <= rect.left ||
               prcl->top >= rect.bottom ||
               prcl->bottom <= rect.top)
                goto nextPgr;

            //
            // We need to set start color and color delta according to the
            // rectangle drawing direction
            //
            if( (ppb->ulMode == GRADIENT_FILL_RECT_H) && (bReverseH == TRUE)
              ||(ppb->ulMode == GRADIENT_FILL_RECT_V) && (bReverseV == TRUE) )
            {
                rd = (ptrvtUl->Red - ptrvtLr->Red) << 8;
                gd = (ptrvtUl->Green - ptrvtLr->Green) << 8;
                bd = (ptrvtUl->Blue - ptrvtLr->Blue) << 8;

                rs = ptrvtLr->Red << 8;
                gs = ptrvtLr->Green << 8;
                bs = ptrvtLr->Blue << 8;
            }
            else
            {
                rd = (ptrvtLr->Red - ptrvtUl->Red) << 8;
                gd = (ptrvtLr->Green - ptrvtUl->Green) << 8;
                bd = (ptrvtLr->Blue - ptrvtUl->Blue) << 8;

                rs = ptrvtUl->Red << 8;
                gs = ptrvtUl->Green << 8;
                bs = ptrvtUl->Blue << 8;
            }

            dx = rect.right - rect.left;
            dy = rect.bottom - rect.top;

            if(ppb->ulMode == GRADIENT_FILL_RECT_H)
            {
                rdx = rd / dx;
                gdx = gd / dx;
                bdx = bd / dx;

                rdy = 0;
                gdy = 0;
                bdy = 0;
            }
            else
            {
                rdy = rd / dy;
                gdy = gd / dy;
                bdy = bd / dy;

                rdx = 0;
                gdx = 0;
                bdx = 0;
            }

            // now perform some clipping adjusting start values as necessary
            xShift = prcl->left - rect.left;
            if(xShift > 0)
            {
                rs = rs + (rdx * xShift);
                gs = gs + (gdx * xShift);
                bs = bs + (bdx * xShift);
                rect.left = prcl->left;
            }

            yShift = prcl->top - rect.top;
            if(yShift > 0)
            {
                rs = rs + (rdy * yShift);
                gs = gs + (gdy * yShift);
                bs = bs + (bdy * yShift);
                rect.top = prcl->top;
            }

            // just move up the bottom right as necessary
            if(prcl->right < rect.right)
                rect.right = prcl->right;

            if(prcl->bottom < rect.bottom)
                rect.bottom = prcl->bottom;

            if(ppb->ulMode == GRADIENT_FILL_RECT_H)
            {
                // Algorithm: Fill one line with a host blt. Replicate it till done.
                //  Unless we are at 16bpp then dither 4 lines and replicate.

                ULONG       WidthY = rect.bottom - rect.top;
                ULONG       WidthX = rect.right - rect.left;
                LONG        i;
                ULONG       j;
                ULONG       k;
                ULONG       pel;
                ULONG       dither;
                ULONG       ditherAlignmentX;
                ULONG       ditherAlignmentY;
                ULONG       *ditherTable;
                LONG        rs2, gs2, bs2;
                ULONG       ul, ul0, ul1;
                LONG        cwSrc;
                LONG        cxRem;
                ULONG       ulCommand;
                ULONG       ulFormat;

                ulCommand = ((ULONG) 0xcc << SSTG_ROP0_SHIFT) | SSTG_HOST_BLT;

                switch( ppdev->cjPelSize )
                {
                    case 4:     // 32 bpp

                        ulFormat = SSTG_PIXFMT_32BPP | SSTG_SRC_PACK_32;
                        break;

                    case 3:     // 24 bpp

                        ulFormat = SSTG_PIXFMT_24BPP | SSTG_SRC_PACK_32;
                        break;

                    case 2:     // 16 bpp

                        ulFormat = SSTG_PIXFMT_16BPP | SSTG_SRC_PACK_32;
                        ulCommand |= SSTG_UPDATE_DSTY;
                        break;

                    default:

                        RIP( "vGradientFillRect - Invalid pixel format" );
                }

                // Set up host blt command

                CHECK_FIFO_ROOM( ppdev, 5);
                GWH_BEGIN_2D_PACKET( 5, SSTCP_PKT2_SRCFORMAT |
                                        SSTCP_PKT2_SRCXY     |
                                        SSTCP_PKT2_DSTSIZE   |
                                        SSTCP_PKT2_DSTXY     |
                                        SSTCP_PKT2_COMMAND );

                SET(1, pjH3Base, srcFormat, ulFormat);
                SET(2, pjH3Base, srcXY, 0);     // Zero for packed host blts.
                SET(3, pjH3Base, dstSize, H3_PACKXY_FAST( WidthX, 1 ) );
                SET(4, pjH3Base, dstXY, H3_PACKXY( rect.left, rect.top ) );
                SET(5, pjH3Base, command, ulCommand);
                GWH_END_2D_PACKET( 5 );

                switch( ppdev->cjPelSize )
                {
                    case 4:     // 32 bpp

                        // Send the host blt data

                        CHECK_FIFO_ROOM( ppdev, WidthX );
                        GWH_BEGIN_PKT1_PACKET(WidthX , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                        i = WidthX;
                        while (i-- > 0)
                        {
                            pel = PEL32_FROM_RGB( rs, gs, bs);
                            rs += rdx;
                            gs += gdx;
                            bs += bdx;

                            SET_PKT1_REG(pjH3Base, launch, pel);
                        }

                        GWH_END_PKT1_PACKET(WidthX);

                        j = 1;      // 1 Line complete

                        break;


                    case 3:     // 24 bpp

                        // Send the host blt data

                        cwSrc = (WidthX >> 2);      // Number of whole source words
                        cxRem = (WidthX & 3);
                        ulPktSize = WidthX - cwSrc;    // 3/4 * WidthX

                        CHECK_FIFO_ROOM( ppdev, ulPktSize );
                        GWH_BEGIN_PKT1_PACKET( ulPktSize, SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                        if (cxRem == 3)
                        {
                            // Merge this case into the whole word case:

                            cwSrc++;
                            cxRem = 0;
                        }

                        for (i = cwSrc; i != 0; i--)
                        {
                            ul0 = PEL24_FROM_RGB( rs, gs, bs);
                            rs += rdx;
                            gs += gdx;
                            bs += bdx;

                            ul1 = PEL24_FROM_RGB( rs, gs, bs);
                            rs += rdx;
                            gs += gdx;
                            bs += bdx;

                            ul   = ul0 | (ul1 << 24);
                            SET_PKT1_REG(pjH3Base, launch, ul);

                            ul0 = PEL24_FROM_RGB( rs, gs, bs);
                            rs += rdx;
                            gs += gdx;
                            bs += bdx;

                            ul   = (ul1 >> 8) | (ul0 << 16);
                            SET_PKT1_REG(pjH3Base, launch, ul);

                            ul1 = PEL24_FROM_RGB( rs, gs, bs);
                            rs += rdx;
                            gs += gdx;
                            bs += bdx;

                            ul   = (ul1 << 8) | (ul0 >> 16);
                            SET_PKT1_REG(pjH3Base, launch, ul);
                        }

                        if (cxRem > 0)
                        {
                            ul0 = PEL24_FROM_RGB( rs, gs, bs);
                            rs += rdx;
                            gs += gdx;
                            bs += bdx;

                            ul1 = PEL24_FROM_RGB( rs, gs, bs);
                            rs += rdx;
                            gs += gdx;
                            bs += bdx;

                            ul   = ul0 | (ul1 << 24);
                            SET_PKT1_REG(pjH3Base, launch, ul);

                            if (cxRem > 1)
                            {
                                ul = (ul1 >> 8);
                                SET_PKT1_REG(pjH3Base, launch, ul);
                            }
                        }

                        GWH_END_PKT1_PACKET(ulPktSize);

                        j = 1;      // 1 Line complete

                        break;


                    case 2:     // 16 bpp


                        ditherAlignmentY = (ppb->pptlDitherOrg->y + rect.top) & 0x3;
                        j = 0;
                        do {

                            if( j > 0 )
                            {
                                CHECK_FIFO_ROOM( ppdev, 1);
                                GWH_BEGIN_2D_PACKET( 1, SSTCP_PKT2_COMMAND );
                                SET(1, pjH3Base, command, ((ULONG) 0xcc << SSTG_ROP0_SHIFT) |
                                                                           SSTG_HOST_BLT    |
                                                                           SSTG_UPDATE_DSTY );
                                GWH_END_2D_PACKET( 1 );
                            }

                            rs2 = rs;
                            gs2 = gs;
                            bs2 = bs;

                            ditherAlignmentX = (ppb->pptlDitherOrg->x + rect.left) & 0x3;
                            ditherTable = gaulDitherTable16[ditherAlignmentY];

                            // Send the host blt data

                            ulPktSize = (WidthX + 1) >> 1;
                            CHECK_FIFO_ROOM( ppdev, ulPktSize );
                            GWH_BEGIN_PKT1_PACKET(ulPktSize , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

                            i = WidthX >> 1;
                            while (i-- > 0)
                            {
                                dither = ditherTable[ditherAlignmentX++ & 3];
                                pel = PEL16_FROM_RGB( rs2, gs2, bs2, dither);
                                rs2 += rdx;
                                gs2 += gdx;
                                bs2 += bdx;

                                dither = ditherTable[ditherAlignmentX++ & 3];
                                pel |= (PEL16_FROM_RGB( rs2, gs2, bs2, dither) << 16);
                                rs2 += rdx;
                                gs2 += gdx;
                                bs2 += bdx;

                                SET_PKT1_REG(pjH3Base, launch, pel);
                            }

                            if ( WidthX & 0x1)
                            {
                                dither = ditherTable[ditherAlignmentX & 3];
                                pel = PEL16_FROM_RGB( rs2, gs2, bs2, dither);
                                SET_PKT1_REG(pjH3Base, launch, pel);
                            }

                            GWH_END_PKT1_PACKET(ulPktSize);

                            j++;
                            ditherAlignmentY = (ditherAlignmentY + 1) & 0x3;

                        } while ( (j < WidthY) && ( j < 4 )  );

                        break;


                    default:

                        RIP( "vGradientFillRect - Invalid pixel format" );
                }

                // Now we have one line in place.  Copy it to all lines.

                if ( WidthX > 192 )
                {
                    if ( WidthY > j )
                    {
                        // We are going to use a trick of the HW to replicate
                        // all the lines at once.

                        CHECK_FIFO_ROOM(ppdev, 5);
                        GWH_BEGIN_2D_PACKET(5, SSTCP_PKT2_SRCFORMAT |
                                               SSTCP_PKT2_SRCXY     |
                                               SSTCP_PKT2_DSTSIZE   |
                                               SSTCP_PKT2_DSTXY     |
                                               SSTCP_PKT2_COMMAND);

                        SET(1, pjH3Base, srcFormat,   bltDstFormat);  // srcFormat == dstFormat
                        SET(2, pjH3Base, srcXY,       H3_PACKXY(rect.left,rect.top));
                        SET(3, pjH3Base, dstSize,     H3_PACKXY_FAST(WidthX,WidthY-j));
                        SET(4, pjH3Base, dstXY,       H3_PACKXY(rect.left,rect.top+j));
                        SET(5, pjH3Base, command,     (0xcc << SSTG_ROP0_SHIFT) |
                                                               SSTG_GO          |
                                                               SSTG_BLT );
                        GWH_END_2D_PACKET(5);
                    }
                }
                else
                {
                    // We are always going to set the srcFormat even if we're
                    // done here because the odds are we'll rarely be done
                    // here so we save a conditional for 99.999% of the calls.

                    CHECK_FIFO_ROOM(ppdev, 2);
                    GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_SRCXY);
                    SET(1, pjH3Base, srcFormat,   bltDstFormat);  // srcFormat == dstFormat
                    SET(2, pjH3Base, srcXY,       H3_PACKXY(rect.left,rect.top));
                    GWH_END_2D_PACKET(2);

                    // j == Lines done so far

                    k = WidthY-j;       // Lines left to do
                    while ( k >= j)
                    {
                        // Double the number of lines with a blt
                        // until we can't double any more

                        CHECK_FIFO_ROOM(ppdev, 3);
                        GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_DSTSIZE   |
                                               SSTCP_PKT2_DSTXY     |
                                               SSTCP_PKT2_COMMAND);

                        SET(1, pjH3Base, dstSize,     H3_PACKXY_FAST(WidthX,j));
                        SET(2, pjH3Base, dstXY,       H3_PACKXY(rect.left,rect.top+j));
                        SET(3, pjH3Base, command,     (0xcc << SSTG_ROP0_SHIFT) |
                                                               SSTG_GO          |
                                                               SSTG_BLT );
                        GWH_END_2D_PACKET(3);

                        k -= j;
                        j += j;
                    }

                    if ( k > 0 )
                    {
                        // Blt any remaing lines that are to small to be doubled

                        CHECK_FIFO_ROOM(ppdev, 3);
                        GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_DSTSIZE   |
                                               SSTCP_PKT2_DSTXY     |
                                               SSTCP_PKT2_COMMAND);

                        SET(1, pjH3Base, dstSize,     H3_PACKXY_FAST(WidthX,k));
                        SET(2, pjH3Base, dstXY,       H3_PACKXY(rect.left,rect.top+j));
                        SET(3, pjH3Base, command,     (0xcc << SSTG_ROP0_SHIFT) |
                                                               SSTG_GO          |
                                                               SSTG_BLT );
                        GWH_END_2D_PACKET(3);
                    }
                }

            }
            else  // GRADIENT_FILL_RECT_V
            {
                ULONG       WidthY = rect.bottom - rect.top;
                ULONG       WidthX = rect.right - rect.left;
                ULONG       i;
                ULONG       pel;

                // Do a solid color fill for each line in the rect

                for ( i = 0; i < WidthY; i++)
                {
                    switch( ppdev->cjPelSize )
                    {
                        case 4:
                            pel = PEL32_FROM_RGB( rs, gs, bs);
                            break;

                        case 3:
                            pel = PEL24_FROM_RGB( rs, gs, bs);
                            break;

                        case 2:
                            pel = PEL16_FROM_RGB_NODITHER( rs, gs, bs);
                            break;

                        default:
                            RIP( "vGradientFillRect - Invalid pixel format" );
                            break;
                    }

                    rs += rdy;
                    gs += gdy;
                    bs += bdy;

                    CHECK_FIFO_ROOM(ppdev, 4);
                    GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_COLORFORE     |
                                           SSTCP_PKT2_DSTSIZE       |
                                           SSTCP_PKT2_DSTXY         |
                                           SSTCP_PKT2_COMMAND);

                    SET(1, pjH3Base, colorFore, pel);
                    SET(2, pjH3Base, dstSize, H3_PACKXY_FAST( WidthX, 1 ));
                    SET(3, pjH3Base, dstXY, H3_PACKXY( rect.left, rect.top + i ));
                    SET(4, pjH3Base, command, (0xcc << SSTG_ROP0_SHIFT)  |
                                                       SSTG_MONO_PATTERN |
                                                       SSTG_GO           |
                                                       SSTG_RECTFILL );
                    GWH_END_2D_PACKET( 4 );
                }

            }

        nextPgr:

            pgr++;

        }

        prcl++;

    }

    GWH_EPILOG;

}// vGradientFillRect()

#endif
