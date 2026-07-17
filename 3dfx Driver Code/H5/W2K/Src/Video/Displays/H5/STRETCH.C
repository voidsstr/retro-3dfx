
/******************************Module*Header*******************************\
* Module Name: stretch.c
*
* Copyright (c) 1993-1996 Microsoft Corporation
* Copyright (c) 1997 3Dfx Interactive, Inc.
\**************************************************************************/
													
#include "precomp.h"

#define STRETCH_MAX_EXTENT 32767

typedef DWORDLONG ULONGLONG;

#if defined(DBG) || defined(PUNT_OPTION)

    BOOL gbPuntStretchBlt     = FALSE;

#else

    #define gbPuntStretchBlt  FALSE

#endif

/******************************Public*Routine******************************\
*
* Routine Name
*
*   vDirectStretch8Narrow
*
* Routine Description:
*
*   Stretch blt 8->8 when the width is 7 or less
*
* Arguments:
*
*   pStrBlt - contains all params for blt
*
* Return Value:
*
*   VOID
*
\**************************************************************************/

VOID vDirectStretch8Narrow(
STR_BLT* pStrBlt)
{
    BYTE*   pjSrc;
    BYTE*   pjDstEnd;
    ULONG   ulDst;
    ULONG   xAccum;
    ULONG   xTmp;
    ULONG   yTmp;

    LONG    xDst        = pStrBlt->XDstStart;
    LONG    xSrc        = pStrBlt->XSrcStart;
    BYTE*   pjSrcScan   = pStrBlt->pjSrcScan + xSrc;
    BYTE*   pjDst       = pStrBlt->pjDstScan + xDst;
    LONG    yCount      = pStrBlt->YDstCount;
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    LONG    lDstStride  = pStrBlt->lDeltaDst - WidthX;
    ULONG   yInt        = 0;
    PDEV*   ppdev       = pStrBlt->ppdev;

    yInt = pStrBlt->lDeltaSrc * pStrBlt->ulYDstToSrcIntCeil;

    //
    // Narrow blt
    //

    START_DIRECT_ACCESS_H3(ppdev, ppdev->pjH3Base);

    do {

        ULONG  yTmp = yAccum + yFrac;
        BYTE   jSrc0;
        BYTE*  pjDstEndNarrow = pjDst + WidthX;

        pjSrc   = pjSrcScan;
        xAccum  = pStrBlt->ulXFracAccumulator;

        do {
            jSrc0    = *pjSrc;
            xTmp     = xAccum + xFrac;
            pjSrc    = pjSrc + xInt;
            if (xTmp < xAccum)
                pjSrc++;

            *pjDst++ = jSrc0;
            xAccum   = xTmp;
        } while (pjDst != pjDstEndNarrow);

        pjSrcScan += yInt;

        if (yTmp < yAccum)
        {
            pjSrcScan += pStrBlt->lDeltaSrc;
        }

        yAccum = yTmp;
        pjDst += lDstStride;

    } while (--yCount);

    END_DIRECT_ACCESS_H3(ppdev, ppdev->pjH3Base);
}

/******************************Public*Routine******************************\
* VOID vDirectStretch8
*
* Hardware assisted stretchblt at 8bpp when the width is 8 or more,
* for Banshees.
*
\**************************************************************************/

VOID vDirectStretch8(
STR_BLT* pStrBlt)
{
    BYTE*   pjSrc;
    ULONG   ulDst;
    ULONG   xAccum;
    ULONG   xTmp;
    ULONG   yTmp;
    BYTE*   pjOldScan;
    LONG    cyDuplicate;
    ULONG   yInt;

    PDEV*   ppdev       = pStrBlt->ppdev;
    BYTE*   pjH3Base;
    LONG    cxMemory    = ppdev->cxMemory;
    LONG    xDst        = pStrBlt->XDstStart;
    LONG    xSrc        = pStrBlt->XSrcStart;
    BYTE*   pjSrcScan   = pStrBlt->pjSrcScan + xSrc;
    LONG    yDst        = pStrBlt->YDstStart + ppdev->yOffset;
    LONG    yCount      = pStrBlt->YDstCount;
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    LONG    lDeltaSrc   = pStrBlt->lDeltaSrc;
    LONG    i;
#ifdef H3_FIFO
    ULONG   ulPktSize;
#endif

    #if ENABLE_LINEAR_DFBS
    DWORD bltDstFormat, dstPixelFormat;
    DWORD bltSrcFormat, srcPixelFormat;
    #endif

    GWH_DECL;

    GWH_PROLOG;

    pjH3Base = ppdev->pjH3Base;

    yInt = pStrBlt->lDeltaSrc * pStrBlt->ulYDstToSrcIntCeil;

    xDst += ppdev->xOffset;

    #if ENABLE_LINEAR_DFBS
    {
        //
        // Stretching is a two part operation.  In the first step, we
        // x-stretch one scan line from the source surface onto the
        // destination surface.  In the second step, we y-stretch that
        // scan line by replicating it in the destination surface
        // as many times as needed.  We then repeat for the next scan line.
        //
        // Here we set the destination pixel format and base address.
        // This will remain unchanged throughout the operation.
        //
        // We also set the initial src format and base address.
        // The source format and base address will change back and forth
        // between the source surface (while stretching a scan line)
        // and the destination surface (while replicating the scan line).
        // We start by setting the source address to the source surface
        // in preparation for strething the first scan line.
        //
        //
        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        GETPIXELFORMAT(ppdev->cjPelSize, srcPixelFormat);
        BLTFMT(ppdev->lDeltaSrc, srcPixelFormat, bltSrcFormat);
        bltSrcFormat |= SSTG_PIXFMT_8BPP | SSTG_SRC_PACK_32;

        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_DSTBASEADDR      |
                               SSTCP_PKT2_DSTFORMAT        |
                               SSTCP_PKT2_SRCBASEADDR      );

        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        SET(3, pjH3Base, srcBaseAddr, ppdev->fpVidMemSrc);
        GWH_END_2D_PACKET( 3 );
    }
    #endif

    // Loop stretching each scan line

    do {
        BYTE    jSrc0,jSrc1,jSrc2,jSrc3;
        ULONG   yTmp;

        pjSrc   = pjSrcScan;
        xAccum  = pStrBlt->ulXFracAccumulator;

        CHECK_FIFO_ROOM( ppdev, 5);
        GWH_BEGIN_2D_PACKET( 5, SSTCP_PKT2_SRCFORMAT |
                                SSTCP_PKT2_SRCXY     |
                                SSTCP_PKT2_DSTSIZE   |
                                SSTCP_PKT2_DSTXY     |
                                SSTCP_PKT2_COMMAND );
        #if ENABLE_LINEAR_DFBS
        SET(1, pjH3Base, srcFormat, bltSrcFormat);
        #else
        SET(1, pjH3Base, srcFormat, SSTG_PIXFMT_8BPP | SSTG_SRC_PACK_32);
        #endif
        SET(2, pjH3Base, srcXY, 0);     // Zero for packed host blts.
        SET(3, pjH3Base, dstSize, H3_PACKXY_FAST( WidthX, 1 ) );
        SET(4, pjH3Base, dstXY, H3_PACKXY( xDst, yDst ) );
        SET(5, pjH3Base, command, ((ULONG) 0xcc << SSTG_ROP0_SHIFT) | SSTG_HOST_BLT );
        GWH_END_2D_PACKET( 5 );


#ifdef H3_FIFO
        ulPktSize = (WidthX / 4) + ((WidthX & 0x3) ? 1 : 0);
#endif
        // Reserve space for data. Make sure we have enough for all SET's.
        CHECK_FIFO_ROOM( ppdev, ulPktSize );
        GWH_BEGIN_PKT1_PACKET(ulPktSize , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

        for ( i = WidthX / 4; i > 0; i-- )
        {
            jSrc0 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt;
            if (xTmp < xAccum)
                pjSrc++;

            jSrc1 = *pjSrc;
            xAccum = xTmp + xFrac;
            pjSrc = pjSrc + xInt;
            if (xAccum < xTmp)
                pjSrc++;

            jSrc2 = *pjSrc;
            xTmp = xAccum + xFrac;
            pjSrc = pjSrc + xInt;
            if (xTmp < xAccum)
                pjSrc++;

            jSrc3 = *pjSrc;
            xAccum = xTmp + xFrac;
            pjSrc = pjSrc + xInt;
            if (xAccum < xTmp)
                pjSrc++;

            ulDst = (jSrc3 << 24) | (jSrc2 << 16) | (jSrc1 << 8) | jSrc0;

            SET_PKT1_REG(pjH3Base, launch, ulDst);
        }

        if ( WidthX & 0x3 )
        {
            ulDst = 0;
            switch (WidthX & 0x3) {
            case 3:
                ulDst = (ULONG) *pjSrc;
                xTmp = xAccum + xFrac;
                pjSrc = pjSrc + xInt;
                if (xTmp < xAccum)
                    pjSrc++;

                xAccum = xTmp;

                ulDst |= (ULONG) *pjSrc << 8;
                xTmp = xAccum + xFrac;
                pjSrc = pjSrc + xInt;
                if (xTmp < xAccum)
                    pjSrc++;

                ulDst |= (ULONG) *pjSrc << 16;

                break;

            case 2:
                ulDst = (ULONG) *pjSrc;
                xTmp = xAccum + xFrac;
                pjSrc = pjSrc + xInt;
                if (xTmp < xAccum)
                    pjSrc++;

                ulDst |= (ULONG) *pjSrc << 8;
                break;

            case 1:
                ulDst = (ULONG) *pjSrc;
                break;
            }

            SET_PKT1_REG(pjH3Base, launch, ulDst);
        }

        GWH_END_PKT1_PACKET(ulPktSize);

        // Now count the number of duplicate scans:

        yDst++;
        cyDuplicate = -1;
        pjOldScan = pjSrcScan;
        do {
            cyDuplicate++;
            pjSrcScan += yInt;

            yTmp = yAccum + yFrac;
            if (yTmp < yAccum)
            {
                pjSrcScan += lDeltaSrc;
            }
            yAccum = yTmp;
            yCount--;

        } while ((yCount != 0) && (pjSrcScan == pjOldScan));

        // Duplicate the scan 'cyDuplicate' times with one blt:

        if (cyDuplicate != 0)
        {
            // It's an expanding stretch in 'y'; the scan we just laid down
            // will be copied at least once using the hardware:

            #if ENABLE_LINEAR_DFBS
            {
                // When we duplicate scan lines we will do screen-to-screen
                // operations on the destination surface, so set the
                // source pointers to point to the destination surface.

                CHECK_FIFO_ROOM(ppdev, 1);
                GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_SRCBASEADDR);
                SET(1, pjH3Base, srcBaseAddr, ppdev->fpVidMemDst);
                GWH_END_2D_PACKET( 1 );
            }
            #endif

            CHECK_FIFO_ROOM( ppdev, 5);
            GWH_BEGIN_2D_PACKET(5, SSTCP_PKT2_SRCFORMAT |
                                   SSTCP_PKT2_SRCXY     |
                                   SSTCP_PKT2_DSTSIZE   |
                                   SSTCP_PKT2_DSTXY     |
                                   SSTCP_PKT2_COMMAND);
            #if ENABLE_LINEAR_DFBS
            // Set src format to the destination bitmap format.
            SET(1, pjH3Base, srcFormat, bltDstFormat);
            #else
            SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat);
            #endif
            SET(2, pjH3Base, srcXY, H3_PACKXY(xDst, yDst - 1) );
//            SET( pjH3Base, srcSize, H3_PACKXY( WidthX, 1 );
            SET(3, pjH3Base, dstSize, H3_PACKXY(WidthX, cyDuplicate) );
            SET(4, pjH3Base, dstXY, H3_PACKXY( xDst, yDst ) );
            SET(5, pjH3Base, command, (0xcc << SSTG_ROP0_SHIFT)  |
                                               SSTG_GO           |
                                               SSTG_STRETCH_BLT );
            GWH_END_2D_PACKET( 5 );

            yDst += cyDuplicate;

            #if ENABLE_LINEAR_DFBS
            {
                // Aim the source pointers back at the source surface
                // in preparation to stretch the next scan line.

                CHECK_FIFO_ROOM(ppdev, 1);
                GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_SRCBASEADDR);
                SET(1, pjH3Base, srcBaseAddr, ppdev->fpVidMemSrc);
                GWH_END_2D_PACKET( 1 );
            }
            #endif
        }
    } while (yCount != 0);

    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* VOID vDirectStretch16
*
* Hardware assisted stretchblt at 16bpp, for Banshees.
*
\**************************************************************************/

VOID vDirectStretch16(
STR_BLT* pStrBlt)
{
    BYTE*   pjOldScan;
    USHORT* pusSrc;
    USHORT* pusDstEnd;
    ULONG   ulDst;
    ULONG   xAccum;
    ULONG   xTmp;
    ULONG   yTmp;
    LONG    cyDuplicate;
    ULONG   yInt;

    PDEV*   ppdev       = pStrBlt->ppdev;
    BYTE*   pjH3Base;
    LONG    xDst        = pStrBlt->XDstStart;
    LONG    xSrc        = pStrBlt->XSrcStart;
    BYTE*   pjSrcScan   = (pStrBlt->pjSrcScan) + xSrc * 2;
    LONG    yDst        = pStrBlt->YDstStart + ppdev->yOffset;       //  (Note: ASM - sync base)
    LONG    yCount      = pStrBlt->YDstCount;
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    LONG    lDeltaSrc   = pStrBlt->lDeltaSrc;
    LONG    i;
#ifdef H3_FIFO
    ULONG   ulPktSize;
#endif

    #if ENABLE_LINEAR_DFBS
    DWORD bltDstFormat, dstPixelFormat;
    DWORD bltSrcFormat, srcPixelFormat;
    #endif

    GWH_DECL;

    GWH_PROLOG;

    pjH3Base = ppdev->pjH3Base;

    yInt = pStrBlt->lDeltaSrc * pStrBlt->ulYDstToSrcIntCeil;

    xDst += ppdev->xOffset;      //  (Note: ASM - sync base)

    #if ENABLE_LINEAR_DFBS
    {
        //
        // Stretching is a two part operation.  In the first step, we
        // x-stretch one scan line from the source surface onto the
        // destination surface.  In the second step, we y-stretch that
        // scan line by replicating it in the destination surface
        // as many times as needed.  We then repeat for the next scan line.
        //
        // Here we set the destination pixel format and base address.
        // This will remain unchanged throughout the operation.
        //
        // We also set the initial src format and base address.
        // The source format and base address will change back and forth
        // between the source surface (while stretching a scan line)
        // and the destination surface (while replicating the scan line).
        // We start by setting the source address to the source surface
        // in preparation for strething the first scan line.
        //
        //
        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        GETPIXELFORMAT(ppdev->cjPelSize, srcPixelFormat);
        BLTFMT(ppdev->lDeltaSrc, srcPixelFormat, bltSrcFormat);
        bltSrcFormat |= SSTG_PIXFMT_16BPP | SSTG_SRC_PACK_32;

        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_DSTBASEADDR      |
                               SSTCP_PKT2_DSTFORMAT        |
                               SSTCP_PKT2_SRCBASEADDR      );

        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        SET(3, pjH3Base, srcBaseAddr, ppdev->fpVidMemSrc);
        GWH_END_2D_PACKET( 3 );
    }
    #endif

    // Loop stretching each scan line

    do {
        USHORT  usSrc0,usSrc1;
        ULONG   yTmp;

        CHECK_FIFO_ROOM( ppdev, 5);
        GWH_BEGIN_2D_PACKET( 5, SSTCP_PKT2_SRCFORMAT |
                                SSTCP_PKT2_SRCXY     |
                                SSTCP_PKT2_DSTSIZE   |
                                SSTCP_PKT2_DSTXY     |
                                SSTCP_PKT2_COMMAND );
        #if ENABLE_LINEAR_DFBS
        SET(1, pjH3Base, srcFormat, bltSrcFormat);
        #else
        SET(1, pjH3Base, srcFormat, SSTG_PIXFMT_16BPP | SSTG_SRC_PACK_32);  // Have to clip w/o this
        #endif
        SET(2, pjH3Base, srcXY, 0);   // Zero for packed host blts.
        SET(3, pjH3Base, dstSize, H3_PACKXY_FAST( WidthX, 1 ) );
        SET(4, pjH3Base, dstXY, H3_PACKXY( xDst, yDst ) );
        SET(5, pjH3Base, command, ((ULONG) 0xcc << SSTG_ROP0_SHIFT) | SSTG_HOST_BLT );
        GWH_END_2D_PACKET( 5 );

        pusSrc  = (USHORT*) pjSrcScan;
        xAccum  = pStrBlt->ulXFracAccumulator;

        // A single source scan line is being written:

#ifdef H3_FIFO
        ulPktSize = (WidthX >> 1) + ((WidthX & 0x1) ? 1 : 0);
#endif
        // Reserve space for data. Make sure we have enough for all SET's.
        CHECK_FIFO_ROOM( ppdev, ulPktSize );
        GWH_BEGIN_PKT1_PACKET(ulPktSize , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

        for ( i = (WidthX >> 1); i > 0; i-- )
        {
            usSrc0 = *pusSrc;
            xTmp   = xAccum + xFrac;
            pusSrc = pusSrc + xInt;
            if (xTmp < xAccum)
                pusSrc++;

            usSrc1 = *pusSrc;
            xAccum = xTmp + xFrac;
            pusSrc = pusSrc + xInt;
            if (xAccum < xTmp)
                pusSrc++;

            ulDst = (ULONG)((usSrc1 << 16) | usSrc0);

            SET_PKT1_REG(pjH3Base, launch, ulDst);
        }

        if (WidthX & 0x1)
        {
            SET_PKT1_REG(pjH3Base, launch, (ULONG) *pusSrc);
        }

        GWH_END_PKT1_PACKET(ulPktSize);

        // Now count the number of duplicate scans:

        yDst++;     //  (Note: ASM - sync base)
        cyDuplicate = -1;
        pjOldScan = pjSrcScan;
        do {
            cyDuplicate++;
            pjSrcScan += yInt;

            yTmp = yAccum + yFrac;
            if (yTmp < yAccum)
            {
                pjSrcScan += lDeltaSrc;
            }
            yAccum = yTmp;
            yCount--;

        } while ((yCount != 0) && (pjSrcScan == pjOldScan));

        // Duplicate the scan 'cyDuplicate' times with one blt:

        if (cyDuplicate != 0)
        {
            #if ENABLE_LINEAR_DFBS
            {
                // When we duplicate scan lines we will do screen-to-screen
                // operations on the destination surface, so set the
                // source pointers to point to the destination surface.

                CHECK_FIFO_ROOM(ppdev, 1);
                GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_SRCBASEADDR);
                SET(1, pjH3Base, srcBaseAddr, ppdev->fpVidMemDst);
                GWH_END_2D_PACKET( 1 );
            }
            #endif

            CHECK_FIFO_ROOM( ppdev, 5);
            GWH_BEGIN_2D_PACKET(5, SSTCP_PKT2_SRCFORMAT |
                                   SSTCP_PKT2_SRCXY     |
                                   SSTCP_PKT2_DSTSIZE   |
                                   SSTCP_PKT2_DSTXY     |
                                   SSTCP_PKT2_COMMAND);
            #if ENABLE_LINEAR_DFBS
            // Set src format to the destination bitmap format.
            SET(1, pjH3Base, srcFormat, bltDstFormat);
            #else
            SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat);
            #endif
            SET(2, pjH3Base, srcXY, H3_PACKXY(xDst, yDst - 1) );
//            SET( pjH3Base, srcSize, H3_PACKXY( WidthX, 1 );
            SET(3, pjH3Base, dstSize, H3_PACKXY(WidthX, cyDuplicate) );
            SET(4, pjH3Base, dstXY, H3_PACKXY( xDst, yDst ) );
            SET(5, pjH3Base, command, (0xcc << SSTG_ROP0_SHIFT)  |
                                               SSTG_GO           |
                                               SSTG_STRETCH_BLT );
            GWH_END_2D_PACKET( 5 );

            yDst += cyDuplicate;    //  (Note: ASM - sync base)

            #if ENABLE_LINEAR_DFBS
            {
                // Aim the source pointers back at the source surface
                // in preparation to stretch the next scan line.

                CHECK_FIFO_ROOM(ppdev, 1);
                GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_SRCBASEADDR);
                SET(1, pjH3Base, srcBaseAddr, ppdev->fpVidMemSrc);
                GWH_END_2D_PACKET( 1 );
            }
            #endif
        }
    } while (yCount != 0);

    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* VOID vDirectStretch24
*
* Hardware assisted stretchblt at 24bpp, for Banshees.
*
* We use the launch register so that we don't have to worry about
* funky alignments.
*
\**************************************************************************/

VOID vDirectStretch24(
STR_BLT* pStrBlt)
{
    BYTE*   pjSrc;
    LONG    lAddress;
    ULONG   ulDst;
    ULONG   xAccum;
    ULONG   xTmp;
    ULONG   yTmp;
    LONG    i;
    LONG    cyDuplicate;
    LONG    yDst;
    BYTE*   pjOldScan;
    LONG    xDstLeft;
    LONG    xDstRight;
    LONG    yDstTop;

    PDEV*   ppdev       = pStrBlt->ppdev;
    BYTE*   pjH3Base;
    LONG    cxMemory    = ppdev->cxMemory;
    BYTE*   pjSrcScan   = pStrBlt->pjSrcScan + pStrBlt->XSrcStart * 3;
    LONG    yCount      = pStrBlt->YDstCount;
    LONG    WidthX      = pStrBlt->XDstEnd - pStrBlt->XDstStart;
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil * 3;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    LONG    lDeltaSrc   = pStrBlt->lDeltaSrc;
    ULONG   yInt        = pStrBlt->lDeltaSrc * pStrBlt->ulYDstToSrcIntCeil;

    ULONG   ulXFracAccumulator = pStrBlt->ulXFracAccumulator;
#ifdef H3_FIFO
    ULONG   ulPktSize;
#endif

    #if ENABLE_LINEAR_DFBS
    DWORD bltDstFormat, dstPixelFormat;
    DWORD bltSrcFormat, srcPixelFormat;
    #endif



    GWH_DECL;

    GWH_PROLOG;

    pjH3Base = ppdev->pjH3Base;

    yDstTop       = ppdev->yOffset + pStrBlt->YDstStart;
    xDstLeft      = ppdev->xOffset + pStrBlt->XDstStart;
    xDstRight     = ppdev->xOffset + pStrBlt->XDstEnd;

    #if ENABLE_LINEAR_DFBS
    {
        //
        // Stretching is a two part operation.  In the first step, we
        // x-stretch one scan line from the source surface onto the
        // destination surface.  In the second step, we y-stretch that
        // scan line by replicating it in the destination surface
        // as many times as needed.  We then repeat for the next scan line.
        //
        // Here we set the destination pixel format and base address.
        // This will remain unchanged throughout the operation.
        //
        // We also set the initial src format and base address.
        // The source format and base address will change back and forth
        // between the source surface (while stretching a scan line)
        // and the destination surface (while replicating the scan line).
        // We start by setting the source address to the source surface
        // in preparation for strething the first scan line.
        //
        //
        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        GETPIXELFORMAT(ppdev->cjPelSize, srcPixelFormat);
        BLTFMT(ppdev->lDeltaSrc, srcPixelFormat, bltSrcFormat);
        bltSrcFormat |= SSTG_PIXFMT_24BPP | SSTG_SRC_PACK_32;

        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_DSTBASEADDR      |
                               SSTCP_PKT2_DSTFORMAT        |
                               SSTCP_PKT2_SRCBASEADDR      );

        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        SET(3, pjH3Base, srcBaseAddr, ppdev->fpVidMemSrc);
        GWH_END_2D_PACKET( 3 );
    }
    #endif

    CHECK_FIFO_ROOM(ppdev, 2);     // Include space for commands at top of do loop
    GWH_BEGIN_2D_PACKET( 2, SSTCP_PKT2_CLIP0MIN | SSTCP_PKT2_CLIP0MAX );
    SET(1, pjH3Base, clip0min, H3_PACKXY_FAST(xDstLeft, 0));   // Optimize - could eliminate left clip.
    SET(2, pjH3Base, clip0max, H3_PACKXY_FAST(xDstRight, ppdev->cyMemory));
    GWH_END_2D_PACKET( 2 );

    yDst = yDstTop;

    do {
        CHECK_FIFO_ROOM( ppdev, 5);
        GWH_BEGIN_2D_PACKET( 5, SSTCP_PKT2_SRCFORMAT |
                                SSTCP_PKT2_SRCXY     |
                                SSTCP_PKT2_DSTSIZE   |
                                SSTCP_PKT2_DSTXY     |
                                SSTCP_PKT2_COMMAND );
        #if ENABLE_LINEAR_DFBS
        SET(1, pjH3Base, srcFormat, bltSrcFormat);
        #else
        SET(1, pjH3Base, srcFormat, SSTG_PIXFMT_24BPP | SSTG_SRC_PACK_32);  // Have to clip w/o this
        #endif
        SET(2, pjH3Base, srcXY, 0);   // Zero for packed host blts.
        SET(3, pjH3Base, dstSize, H3_PACKXY_FAST( WidthX, 1 ) );
        SET(4, pjH3Base, dstXY, H3_PACKXY( xDstLeft, yDst ) );
        SET(5, pjH3Base, command, ((ULONG) 0xcc << SSTG_ROP0_SHIFT) | SSTG_HOST_BLT );
        GWH_END_2D_PACKET( 5 );

        pjSrc  = pjSrcScan;
        xAccum = ulXFracAccumulator;

#ifdef H3_FIFO
        ulPktSize = WidthX - (WidthX >> 2);   // WidthX - cdSrc
#endif
        // Reserve space for data. Make sure we have enough for all SET's.
        CHECK_FIFO_ROOM( ppdev, ulPktSize );
        GWH_BEGIN_PKT1_PACKET(ulPktSize , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

        i = WidthX;
        while (TRUE)
        {
            ulDst  = *(pjSrc);              // Pixel 0
            ulDst |= *(pjSrc + 1) << 8;
            ulDst |= *(pjSrc + 2) << 16;
            if (--i == 0)
                break;
            pjSrc += xInt;
            xTmp   = xAccum + xFrac;
            if (xTmp < xAccum)
                pjSrc += 3;

            ulDst |= *(pjSrc) << 24;        // Pixel 1
            SET_PKT1_REG(pjH3Base, launch, ulDst);
            ulDst  = *(pjSrc + 1);
            ulDst |= *(pjSrc + 2) << 8;
            if (--i == 0)
                break;
            pjSrc += xInt;
            xAccum = xTmp + xFrac;
            if (xAccum < xTmp)
                pjSrc += 3;

            ulDst |= *(pjSrc) << 16;        // Pixel 2
            ulDst |= *(pjSrc + 1) << 24;
            SET_PKT1_REG(pjH3Base, launch, ulDst);
            ulDst  = *(pjSrc + 2);
            if (--i == 0)
                break;
            pjSrc += xInt;
            xTmp   = xAccum + xFrac;
            if (xTmp < xAccum)
                pjSrc += 3;

            ulDst |= *(pjSrc) << 8;         // Pixel 3
            ulDst |= *(pjSrc + 1) << 16;
            ulDst |= *(pjSrc + 2) << 24;
            if (--i == 0)
                break;
            SET_PKT1_REG(pjH3Base, launch, ulDst);
            pjSrc += xInt;
            xAccum = xTmp + xFrac;
            if (xAccum < xTmp)
                pjSrc += 3;
        }

        // Write out the remainder of the scan:

        SET_PKT1_REG(pjH3Base, launch, ulDst);

        GWH_END_PKT1_PACKET(ulPktSize);

        // Now count the number of duplicate scans:

        yDst++;
        cyDuplicate = -1;
        pjOldScan = pjSrcScan;
        do {
            cyDuplicate++;
            pjSrcScan += yInt;

            yTmp = yAccum + yFrac;
            if (yTmp < yAccum)
            {
                pjSrcScan += lDeltaSrc;
            }
            yAccum = yTmp;
            yCount--;

        } while ((yCount != 0) && (pjSrcScan == pjOldScan));

        // Duplicate the scan 'cyDuplicate' times with one blt:

        if (cyDuplicate != 0)
        {
            #if ENABLE_LINEAR_DFBS
            {
                // When we duplicate scan lines we will do screen-to-screen
                // operations on the destination surface, so set the
                // source pointers to point to the destination surface.

                CHECK_FIFO_ROOM(ppdev, 1);
                GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_SRCBASEADDR);
                SET(1, pjH3Base, srcBaseAddr, ppdev->fpVidMemDst);
                GWH_END_2D_PACKET( 1 );
            }
            #endif

            CHECK_FIFO_ROOM(ppdev, 5);
            GWH_BEGIN_2D_PACKET(5, SSTCP_PKT2_SRCFORMAT  |
                                   SSTCP_PKT2_SRCXY      |
                                   SSTCP_PKT2_DSTSIZE    |
                                   SSTCP_PKT2_DSTXY      |
                                   SSTCP_PKT2_COMMAND);
            #if ENABLE_LINEAR_DFBS
            // Set src format to the destination bitmap format.
            SET(1, pjH3Base, srcFormat, bltDstFormat);
            #else
            SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat);
            #endif

            SET(2,  pjH3Base, srcXY, H3_PACKXY(xDstLeft, yDst - 1) );
//            SET( pjH3Base, srcSize, H3_PACKXY( WidthX, 1 );
            SET(3,  pjH3Base, dstSize, H3_PACKXY(WidthX, cyDuplicate) );
            SET(4,  pjH3Base, dstXY, H3_PACKXY(xDstLeft, yDst) );
            SET(5, pjH3Base, command, (0xcc << SSTG_ROP0_SHIFT)  |
                                               SSTG_GO           |
                                               SSTG_STRETCH_BLT );
            GWH_END_2D_PACKET( 5 );

            yDst += cyDuplicate;

            #if ENABLE_LINEAR_DFBS
            {
                // Aim the source pointers back at the source surface
                // in preparation to stretch the next scan line.

                CHECK_FIFO_ROOM(ppdev, 1);
                GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_SRCBASEADDR);
                SET(1, pjH3Base, srcBaseAddr, ppdev->fpVidMemSrc);
                GWH_END_2D_PACKET( 1 );
            }
            #endif

        }
    } while (yCount != 0);

    GWH_EPILOG;             // Needs to precede vResetClipping call.

    vResetClipping(ppdev);  // Optimize - If left clip is eliminated just reset clip0max
}

/******************************Public*Routine******************************\
* VOID vDirectStretch32
*
* Hardware assisted stretchblt at 32bpp, for Banshees.
*
\**************************************************************************/

VOID vDirectStretch32(
STR_BLT* pStrBlt)
{
    BYTE*   pjOldScan;
    ULONG*  pulSrc;
    ULONG   xAccum;
    ULONG   xTmp;
    ULONG   yTmp;
    LONG    i;
    LONG    cyDuplicate;
    ULONG   yInt;

    PDEV*   ppdev       = pStrBlt->ppdev;
    BYTE*   pjH3Base;
    LONG    cxMemory    = ppdev->cxMemory;
    LONG    xDst        = pStrBlt->XDstStart;
    LONG    xSrc        = pStrBlt->XSrcStart;
    BYTE*   pjSrcScan   = pStrBlt->pjSrcScan + xSrc * 4;
    LONG    yDst        = pStrBlt->YDstStart + ppdev->yOffset;
    LONG    yCount      = pStrBlt->YDstCount;
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    LONG    lDeltaSrc   = pStrBlt->lDeltaSrc;

    #ifdef H3_FIFO
    ULONG   ulPktSize;
    #endif

    #if ENABLE_LINEAR_DFBS
    DWORD bltDstFormat, dstPixelFormat;
    DWORD bltSrcFormat, srcPixelFormat;
    #endif



    GWH_DECL;

    GWH_PROLOG;

    yInt = pStrBlt->lDeltaSrc * pStrBlt->ulYDstToSrcIntCeil;

    xDst += ppdev->xOffset;

    pjH3Base = ppdev->pjH3Base;


    #if ENABLE_LINEAR_DFBS
    {
        //
        // Stretching is a two part operation.  In the first step, we
        // x-stretch one scan line from the source surface onto the
        // destination surface.  In the second step, we y-stretch that
        // scan line by replicating it in the destination surface
        // as many times as needed.  We then repeat for the next scan line.
        //
        // Here we set the destination pixel format and base address.
        // This will remain unchanged throughout the operation.
        //
        // We also set the initial src format and base address.
        // The source format and base address will change back and forth
        // between the source surface (while stretching a scan line)
        // and the destination surface (while replicating the scan line).
        // We start by setting the source address to the source surface
        // in preparation for strething the first scan line.
        //
        //
        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
        BLTFMT(ppdev->lDeltaDst, dstPixelFormat, bltDstFormat);

        GETPIXELFORMAT(ppdev->cjPelSize, srcPixelFormat);
        BLTFMT(ppdev->lDeltaSrc, srcPixelFormat, bltSrcFormat);
        bltSrcFormat |= SSTG_PIXFMT_32BPP | SSTG_SRC_PACK_32;

        CHECK_FIFO_ROOM(ppdev, 3);
        GWH_BEGIN_2D_PACKET(3, SSTCP_PKT2_DSTBASEADDR      |
                               SSTCP_PKT2_DSTFORMAT        |
                               SSTCP_PKT2_SRCBASEADDR      );

        SET(1, pjH3Base, dstBaseAddr, ppdev->fpVidMemDst);
        SET(2, pjH3Base, dstFormat, bltDstFormat);
        SET(3, pjH3Base, srcBaseAddr, ppdev->fpVidMemSrc);
        GWH_END_2D_PACKET( 3 );
    }
    #endif



    do {
        ULONG   ulSrc;
        ULONG   yTmp;


        CHECK_FIFO_ROOM( ppdev, 5);
        GWH_BEGIN_2D_PACKET( 5, SSTCP_PKT2_SRCFORMAT |
                                SSTCP_PKT2_SRCXY     |
                                SSTCP_PKT2_DSTSIZE   |
                                SSTCP_PKT2_DSTXY     |
                                SSTCP_PKT2_COMMAND );

        #if ENABLE_LINEAR_DFBS
        SET(1, pjH3Base, srcFormat, bltSrcFormat);
        #else
        SET(1, pjH3Base, srcFormat, SSTG_PIXFMT_32BPP | SSTG_SRC_PACK_32);
        #endif
        SET(2, pjH3Base, srcXY, 0);     // Zero for packed host blts.
        SET(3, pjH3Base, dstSize, H3_PACKXY_FAST( WidthX, 1 ) );
        SET(4, pjH3Base, dstXY, H3_PACKXY( xDst, yDst ) );
        SET(5, pjH3Base, command, ((ULONG) 0xcc << SSTG_ROP0_SHIFT) | SSTG_HOST_BLT );
        GWH_END_2D_PACKET( 5 );

        pulSrc  = (ULONG*) pjSrcScan;
        xAccum  = pStrBlt->ulXFracAccumulator;

#ifdef H3_FIFO
        ulPktSize = WidthX;
#endif
        CHECK_FIFO_ROOM( ppdev, ulPktSize );
        GWH_BEGIN_PKT1_PACKET(ulPktSize , SSTCP_PKT1_LAUNCH, SSTCP_PKT1_NOINC);

        i = WidthX;
        while (i-- > 0)
        {
            ulSrc  = *pulSrc;
            xTmp   = xAccum + xFrac;
            pulSrc = pulSrc + xInt;
            if (xTmp < xAccum)
                pulSrc++;

            SET_PKT1_REG(pjH3Base, launch, ulSrc);
            xAccum = xTmp;
        }

        GWH_END_PKT1_PACKET(ulPktSize);

        // Now count the number of duplicate scans:

        yDst++;
        cyDuplicate = -1;
        pjOldScan = pjSrcScan;
        do {
            cyDuplicate++;
            pjSrcScan += yInt;

            yTmp = yAccum + yFrac;
            if (yTmp < yAccum)
            {
                pjSrcScan += lDeltaSrc;
            }
            yAccum = yTmp;
            yCount--;

        } while ((yCount != 0) && (pjSrcScan == pjOldScan));

        // Duplicate the scan 'cyDuplicate' times with one blt:

        if (cyDuplicate != 0)
        {

            #if ENABLE_LINEAR_DFBS
            {
                // When we duplicate scan lines we will do screen-to-screen
                // operations on the destination surface, so set the
                // source pointers to point to the destination surface.

                CHECK_FIFO_ROOM(ppdev, 1);
                GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_SRCBASEADDR);
                SET(1, pjH3Base, srcBaseAddr, ppdev->fpVidMemDst);
                GWH_END_2D_PACKET( 1 );
            }
            #endif


            CHECK_FIFO_ROOM(ppdev, 5);
            GWH_BEGIN_2D_PACKET(5, SSTCP_PKT2_SRCFORMAT  |
                                   SSTCP_PKT2_SRCXY      |
                                   SSTCP_PKT2_DSTSIZE    |
                                   SSTCP_PKT2_DSTXY      |
                                   SSTCP_PKT2_COMMAND);

            #if ENABLE_LINEAR_DFBS
            // Set src format to the destination bitmap format.
            SET(1, pjH3Base, srcFormat, bltDstFormat);
            #else
            SET(1, pjH3Base, srcFormat, ppdev->ulScreenFormat);
            #endif

            SET(2,  pjH3Base, srcXY, H3_PACKXY(xDst, yDst - 1) );
//            SET( pjH3Base, srcSize, H3_PACKXY( WidthX, 1 );
            SET(3,  pjH3Base, dstSize, H3_PACKXY(WidthX, cyDuplicate) );
            SET(4,  pjH3Base, dstXY, H3_PACKXY(xDst, yDst) );
            SET(5, pjH3Base, command, (0xcc << SSTG_ROP0_SHIFT)  |
                                               SSTG_GO           |
                                               SSTG_STRETCH_BLT );
            GWH_END_2D_PACKET( 5 );

            yDst += cyDuplicate;

            #if ENABLE_LINEAR_DFBS
            {
                // Aim the source pointers back at the source surface
                // in preparation to stretch the next scan line.

                CHECK_FIFO_ROOM(ppdev, 1);
                GWH_BEGIN_2D_PACKET(1, SSTCP_PKT2_SRCBASEADDR);
                SET(1, pjH3Base, srcBaseAddr, ppdev->fpVidMemSrc);
                GWH_END_2D_PACKET( 1 );
            }
            #endif

        }
    } while (yCount != 0);

    GWH_EPILOG;
}

/******************************Public*Routine******************************\
*
* Routine Description:
*
*   StretchBlt using integer math. Must be from one surface to another
*   surface of the same format.
*
* Arguments:
*
*   ppdev           -   PDEV for device
*   prclDst         -   Pointer to rectangle of Dst extents
*   pvSrc           -   Pointer to start of Src bitmap
*   lDeltaSrc       -   Bytes from start of Src scan line to start of next
*   prclSrc         -   Pointer to rectangle of Src extents
*   prclClip        -   Clip Dest to this rect
*
* Return Value:
*
*   Status
*
\**************************************************************************/

/******************************Public*Routine******************************\
*
* Routine Description:
*
*   StretchBlt using integer math. Must be from one surface to another
*   surface of the same format.
*
* Arguments:
*
*   ppdev           -   PDEV for device
*   prclDst         -   Pointer to rectangle of Dst extents
*   pvSrc           -   Pointer to start of Src bitmap
*   lDeltaSrc       -   Bytes from start of Src scan line to start of next
*   prclSrc         -   Pointer to rectangle of Src extents
*   prclClip        -   Clip Dest to this rect
*
* Return Value:
*
*   Status
*
\**************************************************************************/

VOID vStretchDIB(
PDEV*   ppdev,
RECTL*  prclDst,
VOID*   pvSrc,
LONG    lDeltaSrc,
RECTL*  prclSrc,
RECTL*  prclClip)
{
    STR_BLT StrBlt;
    ULONG   XSrcToDstIntFloor;
    ULONG   XSrcToDstFracFloor;
    ULONG   ulXDstToSrcIntCeil;
    ULONG   ulXDstToSrcFracCeil;
    ULONG   YSrcToDstIntFloor;
    ULONG   YSrcToDstFracFloor;
    ULONG   ulYDstToSrcIntCeil;
    ULONG   ulYDstToSrcFracCeil;
    LONG    SrcIntScan;
    LONG    DstDeltaScanEnd;
    ULONG   ulXFracAccumulator;
    ULONG   ulYFracAccumulator;
    LONG    LeftClipDistance;
    LONG    TopClipDistance;
    BOOL    bStretch;

    union {
        LARGE_INTEGER   large;
        ULONGLONG       li;
    } liInit;

    PFN_DIRSTRETCH      pfnStr;

    //
    // Calculate exclusive start and end points:
    //

    LONG    WidthDst  = prclDst->right  - prclDst->left;
    LONG    HeightDst = prclDst->bottom - prclDst->top;
    LONG    WidthSrc  = prclSrc->right  - prclSrc->left;
    LONG    HeightSrc = prclSrc->bottom - prclSrc->top;

    LONG    XSrcStart = prclSrc->left;
    LONG    XSrcEnd   = prclSrc->right;
    LONG    XDstStart = prclDst->left;
    LONG    XDstEnd   = prclDst->right;
    LONG    YSrcStart = prclSrc->top;
    LONG    YSrcEnd   = prclSrc->bottom;
    LONG    YDstStart = prclDst->top;
    LONG    YDstEnd   = prclDst->bottom;

    //
    // Validate parameters:
    //

    ASSERTDD(pvSrc != (VOID*)NULL, "Bad source bitmap pointer");
    ASSERTDD(prclDst != (RECTL*)NULL, "Bad destination rectangle");
    ASSERTDD(prclSrc != (RECTL*)NULL, "Bad source rectangle");
    ASSERTDD((WidthDst > 0) && (HeightDst > 0) &&
             (WidthSrc > 0) && (HeightSrc > 0),
             "Can't do mirroring or empty rectangles here");
    ASSERTDD((WidthDst  <= STRETCH_MAX_EXTENT) &&
             (HeightDst <= STRETCH_MAX_EXTENT) &&
             (WidthSrc  <= STRETCH_MAX_EXTENT) &&
             (HeightSrc <= STRETCH_MAX_EXTENT), "Stretch exceeds limits");
    ASSERTDD(prclClip != NULL, "Bad clip rectangle");

    //
    // Calculate X Dst to Src mapping
    //
    //
    // dst->src =  ( CEIL( (2k*WidthSrc)/WidthDst) ) / 2k
    //
    //          =  ( FLOOR(  (2k*WidthSrc -1) / WidthDst) + 1) / 2k
    //
    // where 2k = 2 ^ 32
    //

    {
        ULONGLONG   liWidthSrc;
        ULONGLONG   liQuo;
        ULONG       ulTemp;

        //
        // Work around a compiler bug dealing with the assignment
        // 'liHeightSrc = (((LONGLONG)HeightSrc) << 32) - 1':
        //

        liInit.large.LowPart = (ULONG) -1;
        liInit.large.HighPart = WidthSrc - 1;
        liWidthSrc = liInit.li;

        liQuo = liWidthSrc / (ULONGLONG) WidthDst;

        ulXDstToSrcIntCeil  = (ULONG)(liQuo >> 32);
        ulXDstToSrcFracCeil = (ULONG)liQuo;

        //
        // Now add 1, use fake carry:
        //

        ulTemp = ulXDstToSrcFracCeil + 1;

        ulXDstToSrcIntCeil += (ulTemp < ulXDstToSrcFracCeil);
        ulXDstToSrcFracCeil = ulTemp;
    }

    //
    // Calculate Y Dst to Src mapping
    //
    //
    // dst->src =  ( CEIL( (2k*HeightSrc)/HeightDst) ) / 2k
    //
    //          =  ( FLOOR(  (2k*HeightSrc -1) / HeightDst) + 1) / 2k
    //
    // where 2k = 2 ^ 32
    //

    {
        ULONGLONG   liHeightSrc;
        ULONGLONG   liQuo;
        ULONG       ulTemp;

        //
        // Work around a compiler bug dealing with the assignment
        // 'liHeightSrc = (((LONGLONG)HeightSrc) << 32) - 1':
        //

        liInit.large.LowPart = (ULONG) -1;
        liInit.large.HighPart = HeightSrc - 1;
        liHeightSrc = liInit.li;

        liQuo = liHeightSrc / (ULONGLONG) HeightDst;

        ulYDstToSrcIntCeil  = (ULONG)(liQuo >> 32);
        ulYDstToSrcFracCeil = (ULONG)liQuo;

        //
        // Now add 1, use fake carry:
        //

        ulTemp = ulYDstToSrcFracCeil + 1;

        ulYDstToSrcIntCeil += (ulTemp < ulYDstToSrcFracCeil);
        ulYDstToSrcFracCeil = ulTemp;
    }

    //
    // Now clip Dst in X, and/or calc src clipping effect on dst
    //
    // adjust left and right edges if needed, record
    // distance adjusted for fixing the src
    //

    if (XDstStart < prclClip->left)
    {
        XDstStart = prclClip->left;
    }

    if (XDstEnd > prclClip->right)
    {
        XDstEnd = prclClip->right;
    }

    //
    // Check for totally clipped out destination:
    //

    if (XDstEnd <= XDstStart)
    {
        return;
    }

    LeftClipDistance = XDstStart - prclDst->left;

    {
        ULONG   ulTempInt;
        ULONG   ulTempFrac;

        //
        // Calculate displacement for .5 in destination and add:
        //

        ulTempFrac = (ulXDstToSrcFracCeil >> 1) | (ulXDstToSrcIntCeil << 31);
        ulTempInt  = (ulXDstToSrcIntCeil >> 1);

        XSrcStart += ulTempInt;
        ulXFracAccumulator = ulTempFrac;

        if (LeftClipDistance != 0)
        {
            ULONGLONG ullFraction;
            ULONG     ulTmp;

            ullFraction = UInt32x32To64(ulXDstToSrcFracCeil, LeftClipDistance);

            ulTmp = ulXFracAccumulator;
            ulXFracAccumulator += (ULONG) (ullFraction);
            if (ulXFracAccumulator < ulTmp)
                XSrcStart++;

            XSrcStart += (ulXDstToSrcIntCeil * LeftClipDistance)
                       + (ULONG) (ullFraction >> 32);
        }
    }

    //
    // Now clip Dst in Y, and/or calc src clipping effect on dst
    //
    // adjust top and bottom edges if needed, record
    // distance adjusted for fixing the src
    //

    if (YDstStart < prclClip->top)
    {
        YDstStart = prclClip->top;
    }

    if (YDstEnd > prclClip->bottom)
    {
        YDstEnd = prclClip->bottom;
    }

    //
    // Check for totally clipped out destination:
    //

    if (YDstEnd <= YDstStart)
    {
        return;
    }

    TopClipDistance = YDstStart - prclDst->top;

    {
        ULONG   ulTempInt;
        ULONG   ulTempFrac;

        //
        // Calculate displacement for .5 in destination and add:
        //

        ulTempFrac = (ulYDstToSrcFracCeil >> 1) | (ulYDstToSrcIntCeil << 31);
        ulTempInt  = ulYDstToSrcIntCeil >> 1;

        YSrcStart += (LONG)ulTempInt;
        ulYFracAccumulator = ulTempFrac;

        if (TopClipDistance != 0)
        {
            ULONGLONG ullFraction;
            ULONG     ulTmp;

            ullFraction = UInt32x32To64(ulYDstToSrcFracCeil, TopClipDistance);

            ulTmp = ulYFracAccumulator;
            ulYFracAccumulator += (ULONG) (ullFraction);
            if (ulYFracAccumulator < ulTmp)
                YSrcStart++;

            YSrcStart += (ulYDstToSrcIntCeil * TopClipDistance)
                       + (ULONG) (ullFraction >> 32);
        }
    }

    //
    // Warm up the hardware if doing an expanding stretch in 'y':
    //

    bStretch = (HeightDst > HeightSrc);
    if (bStretch)
    {
        LONG xOffset = ppdev->xOffset;
        BYTE* pjH3Base;

        GWH_DECL;

        GWH_PROLOG;

        pjH3Base = ppdev->pjH3Base;

        CHECK_FIFO_ROOM( ppdev, 2);
        GWH_BEGIN_2D_PACKET(2, SSTCP_PKT2_SRCFORMAT | SSTCP_PKT2_SRCSIZE );
        SET(1,  pjH3Base, srcFormat, ppdev->ulScreenFormat );
        SET(2,  pjH3Base, srcSize, H3_PACKXY(XDstEnd - XDstStart, 1) );
        GWH_END_2D_PACKET( 2 );

        GWH_EPILOG;
    }

    //
    // Fill out blt structure, then call format-specific stretch code
    //

    StrBlt.ppdev     = ppdev;
    StrBlt.XDstEnd   = XDstEnd;
    StrBlt.YDstStart = YDstStart;
    StrBlt.YDstCount = YDstEnd - YDstStart;

    if (StrBlt.YDstCount > 0)
    {
        //
        // Caclulate starting scan line address.  Since the inner loop
        // routines are format dependent, they must add XDstStart/XSrcStart
        // to pjDstScan/pjSrcScan to get the actual starting pixel address.
        //

        StrBlt.pjSrcScan = (BYTE*) pvSrc + (YSrcStart * lDeltaSrc);
        #if ENABLE_LINEAR_DFBS
        // Set the source address.
        // If linear DFBs are enabled, the pptlSrc is not absolute,
        // so add in the starting address of the DFB.
        #if ENABLE_TILED_HEAP
        if (ppdev->fpVidMemDst & SSTG_IS_TILED)
        {
          StrBlt.pjDstScan = ppdev->pjScreenBase                                          // start of framebuffer
                            + HwPtrToLfbPtr(ppdev, ppdev->fpVidMemDst & ~SSTG_IS_TILED)   // start of tiled DFB
                            + (YDstStart * _FF(ddTilePitch));                             // start of first scan line
          StrBlt.lDeltaDst = _FF(ddTilePitch);
        }
        else
        #endif
        {
          StrBlt.pjDstScan = ppdev->pjScreenBase              // start of framebuffer
                           + ppdev->fpVidMemDst               // offset to start of linear DFB
                           + (YDstStart * ppdev->lDeltaDst);  // start of first scan line

          StrBlt.lDeltaDst = ppdev->lDeltaDst;
        }
        #else
        StrBlt.pjDstScan = ppdev->pjScreen
                         + (ppdev->yOffset + YDstStart) * ppdev->lDelta
                         + ppdev->xOffset * ppdev->cjPelSize;
        StrBlt.lDeltaDst = ppdev->lDelta;
        #endif

        StrBlt.lDeltaSrc           = lDeltaSrc;
        StrBlt.XSrcStart           = XSrcStart;
        StrBlt.XDstStart           = XDstStart;
        StrBlt.ulXDstToSrcIntCeil  = ulXDstToSrcIntCeil;
        StrBlt.ulXDstToSrcFracCeil = ulXDstToSrcFracCeil;
        StrBlt.ulYDstToSrcIntCeil  = ulYDstToSrcIntCeil;
        StrBlt.ulYDstToSrcFracCeil = ulYDstToSrcFracCeil;
        StrBlt.ulXFracAccumulator  = ulXFracAccumulator;
        StrBlt.ulYFracAccumulator  = ulYFracAccumulator;

        if (ppdev->iBitmapFormat == BMF_8BPP)
        {
            if ((XDstEnd - XDstStart) < 7)
                pfnStr = vDirectStretch8Narrow;
            else
                pfnStr = vDirectStretch8;
        }
        else if (ppdev->iBitmapFormat == BMF_16BPP)
        {
            pfnStr = vDirectStretch16;
        }
        else if (ppdev->iBitmapFormat == BMF_24BPP)
        {
            pfnStr = vDirectStretch24;
        }
        else
        {
            ASSERTDD(ppdev->iBitmapFormat == BMF_32BPP, "Expected 32bpp");

            pfnStr = vDirectStretch32;
        }

        (*pfnStr)(&StrBlt);
    }
}

/******************************Public*Routine******************************\
* BOOL DrvStretchBlt
*
\**************************************************************************/

BOOL DrvStretchBlt(
SURFOBJ*            psoDst,
SURFOBJ*            psoSrc,
SURFOBJ*            psoMsk,
CLIPOBJ*            pco,
XLATEOBJ*           pxlo,
COLORADJUSTMENT*    pca,
POINTL*             pptlHTOrg,
RECTL*              prclDst,
RECTL*              prclSrc,
POINTL*             pptlMsk,
ULONG               iMode)
{
    DSURF*   pdsurfSrc;
    DSURF*   pdsurfDst;
    PDEV*    ppdev;
#if !USE_NT5_DDMEMMGR
    OH*      poh;
#endif
    SURFOBJ* psoDstNew;
    SURFOBJ* psoSrcNew;
    BOOL     bPunt = FALSE;
    BOOL     bRet;
    ULONG	 ulCommand;
    BYTE*   pjH3Base;

	GLIDE_EXCLUSION(glideState[ 0 ]);

    // GDI guarantees us that for a StretchBlt the destination surface
    // will always be a device surface, and not a DIB:

    ppdev = (PDEV*) psoDst->dhpdev;
    pdsurfDst = (DSURF*) psoDst->dhsurf;
    pdsurfSrc = (DSURF*) psoSrc->dhsurf;

    pjH3Base = ppdev->pjH3Base;
  
#if USE_NT5_DDMEMMGR
    ASSERTDD(!(pdsurfDst->dt & DT_DIB), "Didn't expect DT_DIB");
#ifdef SLI_AA
    if (_FF(ddMultiChipConfig))
      bPunt = TRUE;
#endif

#if !ENABLE_RECONFIG_VIDMEM
    ppdev->xOffset  = pdsurfDst->x;
    ppdev->yOffset  = pdsurfDst->y;
#endif
#else
    poh             = pdsurfDst->poh;
    ppdev->xOffset  = poh->x;
    ppdev->yOffset  = poh->y;
#endif

    // It's quicker for GDI to do a StretchBlt when the source surface
    // is not a device-managed surface, because then it can directly
    // read the source bits without having to allocate a temporary
    // buffer and call DrvCopyBits to get a copy that it can use.

    // In case we need to punt, set up the engine managed framebuffer view.

#if USE_NT5_DDMEMMGR
    psoDstNew = psoDst;
#else
    psoDstNew          = ppdev->psoPunt;
    psoDstNew->pvScan0 = pdsurfDst->poh->pvScan0;
    psoDstNew->lDelta  = ppdev->lDelta;
#endif

#if USE_NT5_DDMEMMGR
    psoSrcNew = psoSrc;
#else
    if (psoSrc->iType != STYPE_BITMAP)
    {
        pdsurfSrc = (DSURF*) psoSrc->dhsurf;
        if (pdsurfSrc->dt == DT_SCREEN)
        {
            // The source is a device bitmap that is currently stored
            // in device memory.  Set up the engine managed framebuffer view.

            psoSrcNew          = ppdev->psoPunt2;
            psoSrcNew->pvScan0 = pdsurfSrc->poh->pvScan0;
            psoSrcNew->lDelta  = ppdev->lDelta;
        }
        else
        {
            ASSERTDD(pdsurfSrc->dt == DT_DIB, "Can only handle DIB DFBs here");

            // The source was a device bitmap that we just converted
            // to a DIB:

            psoSrcNew = pdsurfSrc->pso;
        }
    }
    else
    {
        psoSrcNew = psoSrc;
    }

    if (pdsurfDst->dt == DT_DIB)
    {
        // The destination was a device bitmap that we just converted
        // to a DIB:

        psoDstNew = pdsurfDst->pso;
        bPunt = TRUE;
    }
#endif

    #if ENABLE_LINEAR_DFBS
        // Set the destination pitch and address.
        ppdev->fpVidMemDst = pdsurfDst->fpVidMem;
        #if ENABLE_TILED_HEAP
        if (ppdev->fpVidMemDst & SSTG_IS_TILED)
          ppdev->lDeltaDst = _FF(ddTileStride);
        else
        #endif
          ppdev->lDeltaDst = pdsurfDst->lDelta;

        ppdev->fpVidMemSrc = pdsurfSrc ? pdsurfSrc->fpVidMem : ppdev->ulScreenOffset;
        #if ENABLE_TILED_HEAP
        if (ppdev->fpVidMemSrc & SSTG_IS_TILED)
          ppdev->lDeltaSrc = _FF(ddTileStride);
        else
        #endif
          ppdev->lDeltaSrc = pdsurfSrc ? pdsurfSrc->lDelta : psoSrc->lDelta;

        ppdev->xOffset = 0;
        ppdev->yOffset = 0;
    #endif


#if defined(DBG) || defined(PUNT_OPTION)
  if ( !gbPuntStretchBlt )
#endif
    if (!bPunt)
    {
        RECTL       rclClip;
        RECTL*      prclClip;
        ULONG       cxDst;
        ULONG       cyDst;
        ULONG       cxSrc;
        ULONG       cySrc;
        BOOL        bMore;
        CLIPENUM    ce;
        LONG        c;
        LONG        i;

        if ((psoSrcNew->iType == STYPE_BITMAP) &&
            (psoMsk == NULL) &&
            ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)) &&
            ((psoSrcNew->iBitmapFormat == ppdev->iBitmapFormat)))
        {
            cxDst = prclDst->right - prclDst->left;
            cyDst = prclDst->bottom - prclDst->top;
            cxSrc = prclSrc->right - prclSrc->left;
            cySrc = prclSrc->bottom - prclSrc->top;

            // Our 'vStretchDIB' routine requires that the stretch be
            // non-inverting, within a certain size, to have no source
            // clipping, and to have no empty rectangles (the latter is the
            // reason for the '- 1' on the unsigned compare here):

            if (((cxSrc - 1) < STRETCH_MAX_EXTENT)         &&
                ((cySrc - 1) < STRETCH_MAX_EXTENT)         &&
                ((cxDst - 1) < STRETCH_MAX_EXTENT)         &&
                ((cyDst - 1) < STRETCH_MAX_EXTENT)         &&
                (prclSrc->left   >= 0)                     &&
                (prclSrc->top    >= 0)                     &&
                (prclSrc->right  <= psoSrcNew->sizlBitmap.cx) &&
                (prclSrc->bottom <= psoSrcNew->sizlBitmap.cy))
            {
                // Our snazzy routine only does COLORONCOLOR.  But for
                // stretching blts, BLACKONWHITE and WHITEONBLACK are also
                // equivalent to COLORONCOLOR:

                if ((iMode == COLORONCOLOR) ||
                    ((iMode < COLORONCOLOR) && (cxSrc <= cxDst) && (cySrc <= cyDst)))
                {
                    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
                    {
                        rclClip.left   = 0;
                        rclClip.top    = 0;
#if USE_NT5_DDMEMMGR
                        rclClip.right  = pdsurfDst->cx;
                        rclClip.bottom = pdsurfDst->cy;   // Extents of surface
#else
                        rclClip.right  = poh->cx;
                        rclClip.bottom = poh->cy;   // Extents of surface
#endif
                        prclClip = &rclClip;

                    StretchSingleClipRect:

#if ENABLE_LINEAR_DFBS
                      // for pete's sake
                      // if it's a screen to screen stretch
                      // let the hw do it
                      if ((pdsurfSrc) && !(pdsurfSrc->dt & DT_DIB))
                      {
                        DWORD bltDstFormat, dstPixelFormat;
                        DWORD bltSrcFormat, srcPixelFormat;

                        GWH_DECL;
                        
                        GWH_PROLOG;
                        
                        GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
                        #if ENABLE_TILED_HEAP
                        if (pdsurfDst->fpVidMem & SSTG_IS_TILED)
                          BLTFMT(_FF(ddTileStride), dstPixelFormat, bltDstFormat);
                        else
                        #endif
                          BLTFMT(pdsurfDst->lDelta, dstPixelFormat, bltDstFormat);

                        GETPIXELFORMAT(ppdev->cjPelSize, srcPixelFormat);
                        #if ENABLE_TILED_HEAP
                        if (pdsurfSrc->fpVidMem & SSTG_IS_TILED)
                          BLTFMT(_FF(ddTileStride), srcPixelFormat, bltSrcFormat);
                        else
                        #endif
                          BLTFMT(pdsurfSrc->lDelta, srcPixelFormat, bltSrcFormat);

                        ulCommand = (0xcc << SSTG_ROP0_SHIFT) |
                                    SSTG_GO                   |
                                    SSTG_STRETCH_BLT;

                        // Shrinks seem to match the GDI convention better
                        // with the reversible bit set.

                        if ( (cxSrc >= cxDst) && (cySrc >= cyDst) )
                            ulCommand |= SSTG_REVERSIBLE;

                        CHECK_FIFO_ROOM(ppdev, 11);
                        GWH_BEGIN_2D_PACKET(11, 
                                               SSTCP_PKT2_CLIP0MIN    |
                                               SSTCP_PKT2_CLIP0MAX    |
                                               SSTCP_PKT2_DSTBASEADDR |
                                               SSTCP_PKT2_DSTFORMAT   |
                                               SSTCP_PKT2_SRCBASEADDR |
                                               SSTCP_PKT2_SRCFORMAT   |
                                               SSTCP_PKT2_SRCSIZE     |
                                               SSTCP_PKT2_SRCXY       |
                                               SSTCP_PKT2_DSTSIZE     |
                                               SSTCP_PKT2_DSTXY       |
                                               SSTCP_PKT2_COMMAND);


                        // NVH-IGX 07.01.99 -  Gotta clip it!
                        SET(1, pjH3Base, clip0min, ((prclClip->top    & 0xFFF) << 16) | (prclClip->left  & 0xFFF));
                        SET(2, pjH3Base, clip0max, ((prclClip->bottom & 0xFFF) << 16) | (prclClip->right & 0xFFF));

                        SET(3, pjH3Base, dstBaseAddr, pdsurfDst->fpVidMem);
                        SET(4, pjH3Base, dstFormat,   bltDstFormat);
                        SET(5, pjH3Base, srcBaseAddr, pdsurfSrc->fpVidMem);
                        SET(6, pjH3Base, srcFormat,   bltSrcFormat);
                        SET(7, pjH3Base, srcSize,     H3_PACKXY(cxSrc,cySrc));
                        SET(8, pjH3Base, srcXY,       H3_PACKXY(prclSrc->left,prclSrc->top));
                        SET(9, pjH3Base, dstSize,     H3_PACKXY(cxDst,cyDst));
                        SET(10, pjH3Base, dstXY,       H3_PACKXY(prclDst->left,prclDst->top));
                        SET(11, pjH3Base, command,     ulCommand );
                        GWH_END_2D_PACKET(11);

                        GWH_EPILOG;
                        
                      }
                      else
                        // it's a host to screen stretch
#endif
                        vStretchDIB(ppdev,
                                    prclDst,
                                    psoSrcNew->pvScan0,
                                    psoSrcNew->lDelta,
                                    prclSrc,
                                    prclClip);

                      vResetClipping(ppdev); // NVH-IGX 07.01.99 - OK.  Done clipping.
                      return(TRUE);
                    }
                    else if (pco->iDComplexity == DC_RECT)
                    {
                        prclClip = &pco->rclBounds;
                        goto StretchSingleClipRect;
                    }
                    else
                    {
                        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

                        do {
                            bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

                            c = cIntersect(prclDst, ce.arcl, ce.c);

                            if (c != 0)
                            {
                                for (i = 0; i < c; i++)
                                {
#if ENABLE_LINEAR_DFBS
                                  // for pete's sake
                                  // if it's a screen to screen stretch
                                  // let the hw do it
                                  if ((pdsurfSrc) && !(pdsurfSrc->dt & DT_DIB))
                                  {
                                    DWORD bltDstFormat, dstPixelFormat;
                                    DWORD bltSrcFormat, srcPixelFormat;

                                    GWH_DECL;

                                    vSetClipping(ppdev, &ce.arcl[i] ); // NVH-IGX 07.01.99 - Gotta clip it!

                                    GWH_PROLOG;

                                    GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
                                    #if ENABLE_TILED_HEAP
                                    if (pdsurfDst->fpVidMem & SSTG_IS_TILED)
                                      BLTFMT(_FF(ddTileStride), dstPixelFormat, bltDstFormat);
                                    else
                                    #endif
                                      BLTFMT(pdsurfDst->lDelta, dstPixelFormat, bltDstFormat);

                                    GETPIXELFORMAT(ppdev->cjPelSize, srcPixelFormat);
                                    #if ENABLE_TILED_HEAP
                                    if (pdsurfSrc->fpVidMem & SSTG_IS_TILED)
                                      BLTFMT(_FF(ddTileStride), srcPixelFormat, bltSrcFormat);
                                    else
                                    #endif
                                      BLTFMT(pdsurfSrc->lDelta, srcPixelFormat, bltSrcFormat);

                                    ulCommand = (0xcc << SSTG_ROP0_SHIFT) |
                                                SSTG_GO                   |
                                                SSTG_STRETCH_BLT;

                                    // Shrinks seem to match the GDI convention better
                                    // with the reversible bit set.

                                    if ( (cxSrc >= cxDst) && (cySrc >= cyDst) )
                                        ulCommand |= SSTG_REVERSIBLE;

                                    CHECK_FIFO_ROOM(ppdev, 11);
                                    GWH_BEGIN_2D_PACKET(11, SSTCP_PKT2_DSTBASEADDR |
                                                            SSTCP_PKT2_DSTFORMAT   |
                                                            SSTCP_PKT2_SRCBASEADDR |
                                                            SSTCP_PKT2_CLIP1MIN    |
                                                            SSTCP_PKT2_CLIP1MAX    |
                                                            SSTCP_PKT2_SRCFORMAT   |
                                                            SSTCP_PKT2_SRCSIZE     |
                                                            SSTCP_PKT2_SRCXY       |
                                                            SSTCP_PKT2_DSTSIZE     |
                                                            SSTCP_PKT2_DSTXY       |
                                                            SSTCP_PKT2_COMMAND);

                                    SET(1, pjH3Base, dstBaseAddr, pdsurfDst->fpVidMem);
                                    SET(2, pjH3Base, dstFormat,   bltDstFormat);
                                    SET(3, pjH3Base, srcBaseAddr, pdsurfSrc->fpVidMem);

                                    SET(4, pjH3Base, clip1min, ((ce.arcl[i].top    & 0xFFF) << 16) | (ce.arcl[i].left  & 0xFFF));
                                    SET(5, pjH3Base, clip1max, ((ce.arcl[i].bottom & 0xFFF) << 16) | (ce.arcl[i].right & 0xFFF));

                                    SET(6,  pjH3Base, srcFormat,   bltSrcFormat);
                                    SET(7,  pjH3Base, srcSize,     H3_PACKXY(cxSrc,cySrc));
                                    SET(8,  pjH3Base, srcXY,       H3_PACKXY(prclSrc->left,prclSrc->top));
                                    SET(9,  pjH3Base, dstSize,     H3_PACKXY(cxDst,cyDst));
                                    SET(10, pjH3Base, dstXY,       H3_PACKXY(prclDst->left,prclDst->top));
                                    SET(11, pjH3Base, command,     ulCommand );
                                    GWH_END_2D_PACKET(11);

                                    GWH_EPILOG;
                                    
                                    
                                  }
                                  else
                                    // it's a host to screen stretch
#endif                            
                                    vStretchDIB(ppdev,
                                                prclDst,
                                                psoSrcNew->pvScan0,
                                                psoSrcNew->lDelta,
                                                prclSrc,
                                                &ce.arcl[i]);
                                }
                            }

                        } while (bMore);

                        vResetClipping(ppdev); // NVH-IGX 07.01.99 - OK.  Done clipping.
                        return(TRUE);
                    }
                }
            }
        }
    }

    // GDI is nice enough to handle the cases where 'psoDst' and/or 'psoSrc'
    // are device-managed surfaces, but it ain't gonna be fast...

    START_DIRECT_ACCESS_H3(ppdev, ppdev->pjH3Base);

    bRet = EngStretchBlt(psoDstNew, psoSrcNew, psoMsk, pco, pxlo, pca, pptlHTOrg,
                         prclDst, prclSrc, pptlMsk, iMode);

    END_DIRECT_ACCESS_H3(ppdev, ppdev->pjH3Base);

    return(bRet);
}

