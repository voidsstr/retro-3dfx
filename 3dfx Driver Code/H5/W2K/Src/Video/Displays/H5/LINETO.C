/******************************Module*Header*******************************\
* Module Name: Lineto.c
*
* DrvLineTo for S3 driver
*
* Copyright (c) 1995-1996 Microsoft Corporation
\**************************************************************************/

#include "precomp.h"

/******************************Public*Routine******************************\
* VOID vLineToTrivial
*
* Draws a single solid integer-only unclipped cosmetic line.
*
\**************************************************************************/

VOID vLineToTrivial(
PDEV*       ppdev,
LONG        x,              // Passed in x1
LONG        y,              // Passed in y1
LONG        x2,             // Passed in x2
LONG        y2,             // Passed in y2
ULONG       iSolidColor,    // -1 means hardware is already set up
MIX         mix)
{
    BYTE*          pjH3Base = ppdev->pjH3Base;
    FLONG          flQuadrant;

    GWH_DECL;

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


    // May want to do optimization:  Make new rop table to use source
    // instead of pattern.  We won't have to load the pattern registers
    // and Greg says this should be faster in 24 and 32bpp modes
    // (uses 1 clock per pixel insead of two ).  Note: This won't work
    // for all cases.

    // DrvLineTo should always call us with a solid color
    //   DrvStrokePath will call use repeatedly and use the -1
    if (iSolidColor != (ULONG) -1)
    {
        ppdev->ulRop3 = gaRop3FromMix[mix & 0xf];

        CHECK_FIFO_ROOM(ppdev, 6);

        GWH_BEGIN_2D_PACKET( 6, SSTCP_PKT2_PATTERN0ALIAS |
                                SSTCP_PKT2_PATTERN1ALIAS |
                                SSTCP_PKT2_SRCXY         |
                                SSTCP_PKT2_COLORFORE     |
                                SSTCP_PKT2_DSTXY         |
                                SSTCP_PKT2_COMMAND);

        SET(1, pjH3Base, pattern0alias, 0xffffffff);
        SET(2, pjH3Base, pattern1alias, 0xffffffff);
        SET(3,  pjH3Base, srcXY, H3_PACKXY(x,y) );
        SET(4, pjH3Base, colorFore, iSolidColor);
        SET(5,  pjH3Base, dstXY, H3_PACKXY(x2,y2) );
        SET(6,  pjH3Base, command, (ppdev->ulRop3 << SSTG_ROP0_SHIFT)   |
                                                     SSTG_MONO_PATTERN  |
                                                     SSTG_REVERSIBLE    |
                                                     SSTG_GO            |
                                                     SSTG_POLYLINE );

        GWH_END_2D_PACKET( 6 );
    }
    else
    {
        CHECK_FIFO_ROOM(ppdev, 3);

        GWH_BEGIN_2D_PACKET( 3, SSTCP_PKT2_SRCXY   |
                                SSTCP_PKT2_DSTXY   |
                                SSTCP_PKT2_COMMAND );

        SET(1,  pjH3Base, srcXY, H3_PACKXY(x,y) );

        SET(2,  pjH3Base, dstXY, H3_PACKXY(x2,y2) );

        SET(3,  pjH3Base, command, (ppdev->ulRop3 << SSTG_ROP0_SHIFT)   |
                                SSTG_MONO_PATTERN           |
                                SSTG_REVERSIBLE             |
                                SSTG_GO                     |
                                SSTG_POLYLINE );

        GWH_END_2D_PACKET( 3 );
    }

    GWH_EPILOG;
}


/******************************Public*Routine******************************\
* VOID vLineToClipped
*
* Draws a single solid integer-only clipped cosmetic line
*
\**************************************************************************/

VOID vLineToClipped(
PDEV*       ppdev,
LONG        x1,
LONG        y1,
LONG        x2,
LONG        y2,
ULONG       iSolidColor,
MIX         mix,
RECTL*      prclClip)
{
    BYTE*   pjH3Base = ppdev->pjH3Base;
    LONG    xOffset;
    LONG    yOffset;

    GWH_DECL;

    GWH_PROLOG;

    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    // Watch caching through subroutine.
    CHECK_FIFO_ROOM(ppdev, 2);

    GWH_BEGIN_2D_PACKET( 2, SSTCP_PKT2_CLIP0MIN   |
                            SSTCP_PKT2_CLIP0MAX   );

    SET(1, pjH3Base, clip0min, H3_PACKXY( prclClip->left + xOffset,
                                          prclClip->top + yOffset) );

    SET(2, pjH3Base, clip0max, H3_PACKXY( prclClip->right + xOffset,
                                          prclClip->bottom + yOffset ));

    GWH_END_2D_PACKET( 2 );

    GWH_EPILOG;
    vLineToTrivial(ppdev, x1, y1, x2, y2, iSolidColor, mix);
    GWH_PROLOG;

    // Reset clipping.
    CHECK_FIFO_ROOM(ppdev, 2);
    GWH_BEGIN_2D_PACKET( 2, SSTCP_PKT2_CLIP0MIN   |
                            SSTCP_PKT2_CLIP0MAX   );

    SET(1, pjH3Base, clip0min, 0);
    #if ENABLE_LINEAR_DFBS
    SET(2, pjH3Base, clip0max, H3_PACKXY_FAST(0x0FFF,ppdev->cyMemory));
    #else
    SET(2, pjH3Base, clip0max, H3_PACKXY_FAST(ppdev->cxMemory,ppdev->cyMemory));
    #endif
    GWH_END_2D_PACKET( 2 );

    GWH_EPILOG;
}


/******************************Public*Routine******************************\
* BOOL DrvLineTo(pso, pco, pbo, x1, y1, x2, y2, prclBounds, mix)
*
* Draws a single solid integer-only cosmetic line.
*
\**************************************************************************/

BOOL DrvLineTo(
SURFOBJ*    pso,
CLIPOBJ*    pco,
BRUSHOBJ*   pbo,
LONG        x1,
LONG        y1,
LONG        x2,
LONG        y2,
RECTL*      prclBounds,
MIX         mix)
{
    PDEV*   ppdev;
    DSURF*  pdsurf;
#if !USE_NT5_DDMEMMGR
    OH*     poh;
#endif
    LONG    xOffset;
    LONG    yOffset;
    BOOL    bRet;

	GLIDE_EXCLUSION(glideState[ 0 ]);

    // Pass the surface off to GDI if it's a device bitmap that we've
    // converted to a DIB:

    pdsurf = (DSURF*) pso->dhsurf;

#if USE_NT5_DDMEMMGR
    ASSERTDD(!(pdsurf->dt & DT_DIB), "Didn't expect DT_DIB");
#else
    if (pdsurf->dt == DT_DIB)
    {
        return(EngLineTo(pdsurf->pso, pco, pbo, x1, y1, x2, y2, prclBounds, mix));
    }
#endif

    // We'll be drawing to the screen or an off-screen DFB; copy the surface's
    // offset now so that we won't need to refer to the DSURF again:

#if !USE_NT5_DDMEMMGR
    poh   = pdsurf->poh;
#endif
    ppdev = (PDEV*) pso->dhpdev;

#ifdef SLI_AA
    if (_FF(ddMultiChipConfig))
    {
      START_DIRECT_ACCESS_H3(ppdev, ppdev->pjH3Base);
      bRet = EngLineTo(pso, pco, pbo, x1, y1, x2, y2, prclBounds, mix);
      END_DIRECT_ACCESS_H3(ppdev, ppdev->pjH3Base);
      return bRet;
    }
#endif

#if USE_NT5_DDMEMMGR
    #if ENABLE_LINEAR_DFBS
        // Set the destination pitch and address.
        ppdev->fpVidMemDst = pdsurf->fpVidMem;
        #if ENABLE_TILED_HEAP
        if (ppdev->fpVidMemDst & SSTG_IS_TILED)
          ppdev->lDeltaDst = _FF(ddTileStride);
        else
        #endif
          ppdev->lDeltaDst = pdsurf->lDelta;

        ppdev->xOffset = 0;
        ppdev->yOffset = 0;

        xOffset = 0;
        yOffset = 0;
    #else
        xOffset = pdsurf->x;
        yOffset = pdsurf->y;
    #endif
#else
    xOffset = poh->x;
    yOffset = poh->y;
#endif

    x1 += xOffset;
    x2 += xOffset;
    y1 += yOffset;
    y2 += yOffset;

    bRet = TRUE;

    // S3_IO_GP_WAIT(ppdev);       // Chuck when S3 code goes away.

    if (pco == NULL)
    {
        // TGL -- ppdev->pfnLineToTrivial
        vLineToTrivial(ppdev, x1, y1, x2, y2, pbo->iSolidColor, mix);
    }
    else if ((pco->iDComplexity <= DC_RECT) &&
             (prclBounds->left >= H3_MIN_INTEGER_BOUND) &&
             (prclBounds->top    >= H3_MIN_INTEGER_BOUND) &&
             (prclBounds->right  <= H3_MAX_INTEGER_BOUND) &&
             (prclBounds->bottom <= H3_MAX_INTEGER_BOUND))
    {
        ppdev->xOffset = xOffset;
        ppdev->yOffset = yOffset;

        // TGL -- ppdev->pfnLineToClipped
        vLineToClipped(ppdev, x1, y1, x2, y2, pbo->iSolidColor, mix,
                                  &pco->rclBounds);
    }
    else
    {
        bRet = FALSE;
    }

    return(bRet);
}
