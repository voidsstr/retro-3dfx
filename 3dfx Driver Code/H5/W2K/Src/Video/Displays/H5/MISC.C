/******************************Module*Header*******************************\
* Module Name: misc.c
*
* Miscellaneous common routines.
*
* Copyright (c) 1992-1995 Microsoft Corporation
*
\**************************************************************************/

#include "precomp.h"

#ifndef USE_DDRAW_CODE
#define SETDW(hwRegister,data)    hwRegister = (data)
#define GET(hwPtr)                hwPtr
#define ghwIO                     ((SstIORegs *)ppdev->pjBase)
#endif

/******************************Public*Data*********************************\
* MIX translation table
*
* Translates a mix 1-16, into an old style Rop 0-255.
*
\**************************************************************************/

BYTE gaRop3FromMix[] =
{
    0xFF,  // R2_WHITE          - Allow rop = gaRop3FromMix[mix & 0x0F]
    0x00,  // R2_BLACK
    0x05,  // R2_NOTMERGEPEN
    0x0A,  // R2_MASKNOTPEN
    0x0F,  // R2_NOTCOPYPEN
    0x50,  // R2_MASKPENNOT
    0x55,  // R2_NOT
    0x5A,  // R2_XORPEN
    0x5F,  // R2_NOTMASKPEN
    0xA0,  // R2_MASKPEN
    0xA5,  // R2_NOTXORPEN
    0xAA,  // R2_NOP
    0xAF,  // R2_MERGENOTPEN
    0xF0,  // R2_COPYPEN
    0xF5,  // R2_MERGEPENNOT
    0xFA,  // R2_MERGEPEN
    0xFF   // R2_WHITE          - Allow rop = gaRop3FromMix[mix & 0xFF]
};

/******************************Public*Routine******************************\
* BOOL bIntersect
*
* If 'prcl1' and 'prcl2' intersect, has a return value of TRUE and returns
* the intersection in 'prclResult'.  If they don't intersect, has a return
* value of FALSE, and 'prclResult' is undefined.
*
\**************************************************************************/

BOOL bIntersect(
RECTL*  prcl1,
RECTL*  prcl2,
RECTL*  prclResult)
{
    prclResult->left  = max(prcl1->left,  prcl2->left);
    prclResult->right = min(prcl1->right, prcl2->right);

    if (prclResult->left < prclResult->right)
    {
        prclResult->top    = max(prcl1->top,    prcl2->top);
        prclResult->bottom = min(prcl1->bottom, prcl2->bottom);

        if (prclResult->top < prclResult->bottom)
        {
            return(TRUE);
        }
    }

    return(FALSE);
}

/******************************Public*Routine******************************\
* LONG cIntersect
*
* This routine takes a list of rectangles from 'prclIn' and clips them
* in-place to the rectangle 'prclClip'.  The input rectangles don't
* have to intersect 'prclClip'; the return value will reflect the
* number of input rectangles that did intersect, and the intersecting
* rectangles will be densely packed.
*
\**************************************************************************/

LONG cIntersect(
RECTL*  prclClip,
RECTL*  prclIn,         // List of rectangles
LONG    c)              // Can be zero
{
    LONG    cIntersections;
    RECTL*  prclOut;

    DISPDBG((9, "cIntersect called--prclClip=0x%x, prclIn=0x%x,c=%ld",
             prclClip, prclIn, c));

    cIntersections = 0;
    prclOut        = prclIn;

    //
    // Validate input parameter
    //
    ASSERTDD( ((prclIn != NULL ) && (prclClip != NULL) && ( c >= 0 )),
              "Wrong input to cIntersect" );

    for (; c != 0; prclIn++, c--)
    {
        prclOut->left  = max(prclIn->left,  prclClip->left);
        prclOut->right = min(prclIn->right, prclClip->right);

        if (prclOut->left < prclOut->right)
        {
            prclOut->top    = max(prclIn->top,    prclClip->top);
            prclOut->bottom = min(prclIn->bottom, prclClip->bottom);

            if (prclOut->top < prclOut->bottom)
            {
                prclOut++;
                cIntersections++;
            }
        }
    }

    DISPDBG((9, "cIntersect found %d intersections", cIntersections));

    return(cIntersections);
}

/******************************Public*Routine******************************\
* VOID vResetClipping
\**************************************************************************/

VOID vResetClipping(
PDEV*   ppdev)
{
	BYTE*	pjH3Base = ppdev->pjH3Base;

    GWH_DECL;
    GWH_PROLOG;

    CHECK_FIFO_ROOM(ppdev, 2);
    GWH_BEGIN_2D_PACKET( 2, SSTCP_PKT2_CLIP0MIN | SSTCP_PKT2_CLIP0MAX );
    SET(1, pjH3Base, clip0min, 0);
    #if ENABLE_LINEAR_DFBS
    SET(2, pjH3Base, clip0max, H3_PACKXY_FAST( 0x0FFF, ppdev->cyMemory));
    #else
    SET(2, pjH3Base, clip0max, H3_PACKXY_FAST(ppdev->cxMemory, ppdev->cyMemory));
    #endif
    GWH_END_2D_PACKET( 2 );

    GWH_EPILOG;
}

/******************************Public*Routine******************************\
* VOID vSetClipping
\**************************************************************************/

VOID vSetClipping(
PDEV*   ppdev,
RECTL*  prclClip)           // In relative coordinates
{
    LONG    xOffset;
    LONG    yOffset;
    BYTE*   pjH3Base;

    GWH_DECL;
    GWH_PROLOG;

    ASSERTDD(prclClip->left + ppdev->xOffset >= 0,
                    "Can't have a negative left!");
    ASSERTDD(prclClip->top + ppdev->yOffset >= 0,
                    "Can't have a negative top!");

    pjH3Base = ppdev->pjH3Base;

    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    CHECK_FIFO_ROOM(ppdev, 2);
    GWH_BEGIN_2D_PACKET( 2, SSTCP_PKT2_CLIP0MIN | SSTCP_PKT2_CLIP0MAX );
    SET(1, pjH3Base, clip0min, ( (prclClip->top    + yOffset) << 16) |
                               prclClip->left   + xOffset );

    SET(2, pjH3Base, clip0max, ( (prclClip->bottom + yOffset) << 16) |
                               prclClip->right  + xOffset );
    GWH_END_2D_PACKET( 2 );

    GWH_EPILOG;
}


/******************************Public*Routine******************************\
* VOID vClearDesktopSurface
\**************************************************************************/

VOID vClearDesktopSurface(
PDEV*   ppdev)
{
	BYTE*	pjH3Base = ppdev->pjH3Base;

    GWH_DECL;
    GWH_PROLOG;

    // Clear desktop surface with rect. fill.

    CHECK_FIFO_ROOM(ppdev, 4);
    GWH_BEGIN_2D_PACKET(4, SSTCP_PKT2_COLORFORE | SSTCP_PKT2_DSTSIZE | SSTCP_PKT2_DSTXY | SSTCP_PKT2_COMMAND);
    SET(1, pjH3Base, colorFore, 0);
    SET(2, pjH3Base, dstSize, H3_PACKXY_FAST( ppdev->cxMemory, ppdev->cyMemory));
    SET(3, pjH3Base, dstXY, H3_PACKXY( 0, 0));
    SET(4, pjH3Base, command, (0xcc << SSTG_ROP0_SHIFT) | SSTG_MONO_PATTERN | SSTG_GO | SSTG_RECTFILL);
    GWH_END_2D_PACKET( 4 );

    GWH_EPILOG;
}


/******************************Public*Routine******************************\
* VOID vAlignedCopy
*
* Copies the given portion of a bitmap, using dword alignment for the
* screen.  Note that this routine has no notion of banking.
*
* Updates ppjDst and ppjSrc to point to the beginning of the next scan.
*
\**************************************************************************/

VOID vAlignedCopy(
PDEV*   ppdev,
BYTE**  ppjDst,
LONG    lDstDelta,
BYTE**  ppjSrc,
LONG    lSrcDelta,
LONG    cjScan,
LONG    cyScan,
BOOL    bDstIsScreen)
{
    BYTE* pjDst;
    BYTE* pjSrc;
    LONG  cjMiddle;
    LONG  culMiddle;
    LONG  cjStartPhase;
    LONG  cjEndPhase;

    pjSrc = *ppjSrc;
    pjDst = *ppjDst;

    cjStartPhase = (0 - ((bDstIsScreen) ? (LONG) pjDst : (LONG) pjSrc)) & 3;
    cjMiddle     = cjScan - cjStartPhase;

    if (cjMiddle < 0)
    {
        cjStartPhase = 0;
        cjMiddle     = cjScan;
    }

    lSrcDelta -= cjScan;
    lDstDelta -= cjScan;            // Account for middle

    cjEndPhase = cjMiddle & 3;
    culMiddle  = cjMiddle >> 2;

    if (DIRECT_ACCESS(ppdev))
    {
        LONG i;

        ///////////////////////////////////////////////////////////////////
        // Portable bus-aligned copy
        //
        // 'memcpy' usually aligns to the destination, so we could call
        // it for that case, but unfortunately we can't be sure.  We
        // always want to align to the frame buffer:

        // Need to wait for GE idle if using SW cursor or garbage appears
        // when dragging stuff across the screen.

        START_DIRECT_ACCESS_H3(ppdev, ppdev->pjH3Base);

        CP_MEMORY_BARRIER();

        if (bDstIsScreen)
        {
            // Align to the destination (implying that the source may be
            // unaligned):

            for (; cyScan > 0; cyScan--)
            {
                for (i = cjStartPhase; i > 0; i--)
                {
                    *pjDst++ = *pjSrc++;
                }

                for (i = culMiddle; i > 0; i--)
                {
                    *((ULONG*) pjDst) = *((ULONG UNALIGNED *) pjSrc);
                    pjSrc += sizeof(ULONG);
                    pjDst += sizeof(ULONG);
                }

                for (i = cjEndPhase; i > 0; i--)
                {
                    *pjDst++ = *pjSrc++;
                }

                pjSrc += lSrcDelta;
                pjDst += lDstDelta;
            }
        }
        else
        {
            // Align to the source (implying that the destination may be
            // unaligned):

            for (; cyScan > 0; cyScan--)
            {
                for (i = cjStartPhase; i > 0; i--)
                {
                    *pjDst++ = *pjSrc++;
                }
                for (i = culMiddle; i > 0; i--)
                {
                    *((ULONG UNALIGNED *) pjDst) = *((ULONG*) (pjSrc));

                    pjSrc += sizeof(ULONG);
                    pjDst += sizeof(ULONG);
                }
                for (i = cjEndPhase; i > 0; i--)
                {
                    *pjDst++ = *pjSrc++;
                }

                pjSrc += lSrcDelta;
                pjDst += lDstDelta;
            }
        }

        *ppjSrc = pjSrc;            // Save the updated pointers
        *ppjDst = pjDst;

        END_DIRECT_ACCESS_H3(ppdev, ppdev->pjH3Base);
    }
    else
    {
        LONG i;

        ///////////////////////////////////////////////////////////////////
        // No direct dword reads bus-aligned copy
        //
        // Because we support the S3 on ancient Jensen Alpha's, we also
        // have to support a sparse view of the frame buffer -- which
        // means using the 'ioaccess.h' macros.
        //
        // We also go through this code path if doing dword reads would
        // crash a non-x86 system.

        MEMORY_BARRIER();

        if (bDstIsScreen)
        {
            // Align to the destination (implying that the source may be
            // unaligned):

            for (; cyScan > 0; cyScan--)
            {
                for (i = cjStartPhase; i > 0; i--)
                {
                    WRITE_REGISTER_UCHAR(pjDst, *pjSrc);
                    pjSrc++;
                    pjDst++;
                }

                for (i = culMiddle; i > 0; i--)
                {
                    WRITE_REGISTER_ULONG(pjDst, *((ULONG UNALIGNED *) pjSrc));
                    pjSrc += sizeof(ULONG);
                    pjDst += sizeof(ULONG);
                }

                for (i = cjEndPhase; i > 0; i--)
                {
                    WRITE_REGISTER_UCHAR(pjDst, *pjSrc);
                    pjSrc++;
                    pjDst++;
                }

                pjSrc += lSrcDelta;
                pjDst += lDstDelta;
            }
        }
        else
        {
            // Align to the source (implying that the destination may be
            // unaligned):

            for (; cyScan > 0; cyScan--)
            {
                for (i = cjStartPhase; i > 0; i--)
                {
                    *pjDst = READ_REGISTER_UCHAR(pjSrc);
                    pjSrc++;
                    pjDst++;
                }

                for (i = culMiddle; i > 0; i--)
                {
                    // There are some board 864/964 boards where we can't
                    // do dword reads from the frame buffer without
                    // crashing the system.

                    *((ULONG UNALIGNED *) pjDst) =
                     ((ULONG) READ_REGISTER_UCHAR(pjSrc + 3) << 24) |
                     ((ULONG) READ_REGISTER_UCHAR(pjSrc + 2) << 16) |
                     ((ULONG) READ_REGISTER_UCHAR(pjSrc + 1) << 8)  |
                     ((ULONG) READ_REGISTER_UCHAR(pjSrc));

                    pjSrc += sizeof(ULONG);
                    pjDst += sizeof(ULONG);
                }

                for (i = cjEndPhase; i > 0; i--)
                {
                    *pjDst = READ_REGISTER_UCHAR(pjSrc);
                    pjSrc++;
                    pjDst++;
                }

                pjSrc += lSrcDelta;
                pjDst += lDstDelta;
            }
        }

        *ppjSrc = pjSrc;            // Save the updated pointers
        *ppjDst = pjDst;
    }
}


/******************************Public*Routine******************************\
* VOID vGetBits
*
* Copies the bits to the given surface from the screen, using the memory
* aperture.  Must be pre-clipped.
*
\**************************************************************************/

VOID vGetBits(
PDEV*       ppdev,
SURFOBJ*    psoDst,
RECTL*      prclDst,        // Absolute coordinates!
POINTL*     pptlSrc)        // Absolute coordinates!
{
    // Note that if Linear device bitmaps are enabled, prclDst and pptlSrc are
    // not absolute.  The are relative to the surface beginning at ppdev->fpVidMemSrc.

    RECTL   rclDraw;
    LONG    cyScan;
    LONG    lDstDelta;
    LONG    lSrcDelta;
    BYTE*   pjDst;
    BYTE*   pjSrc;
    LONG    cjScan;

    DISPDBG((5, "vGetBits -- enter"));

    rclDraw.left   = pptlSrc->x;
    rclDraw.top    = pptlSrc->y;
    rclDraw.right  = rclDraw.left + (prclDst->right  - prclDst->left);
    rclDraw.bottom = rclDraw.top  + (prclDst->bottom - prclDst->top);

    #if !(ENABLE_LINEAR_DFBS)
    // NVH - Assert does not apply to linear DFBs
    ASSERTDD((rclDraw.left   >= 0) &&
             (rclDraw.top    >= 0) &&
             (rclDraw.right  <= ppdev->cxMemory) &&
             (rclDraw.bottom <= ppdev->cyMemory),
             "vGetBitsLinear: rectangle wasn't fully clipped");
    #endif

    // Calculate the pointer to the upper-left corner of both rectangles:

    #if ENABLE_LINEAR_DFBS
        // NVH  Set the source pitch for linear DFB.
        #if ENABLE_TILED_HEAP
        if (ppdev->fpVidMemSrc & SSTG_IS_TILED)
        {
          lSrcDelta = _FF(ddTilePitch);

          // Set the source address.
          // If linear DFBs are enabled, the pptlSrc is not absolute,
          // so add in the starting address of the DFB.
          pjSrc     = ppdev->pjScreenBase                                         // start of framebuffer
                    + HwPtrToLfbPtr(ppdev, ppdev->fpVidMemSrc & ~SSTG_IS_TILED)   // start of tiled DFB
                    + (rclDraw.top  * lSrcDelta)                                  // start of rectangle
                    + (ppdev->cjPelSize * rclDraw.left);
        }
        else
        #endif
        {
          lSrcDelta = ppdev->lDeltaSrc;

          // Set the source address.
          // If linear DFBs are enabled, the pptlSrc is not absolute,
          // so add in the starting address of the DFB.
          pjSrc     = ppdev->pjScreenBase                   // start of framebuffer
                    + (ppdev->fpVidMemSrc & ~SSTG_IS_TILED) // offset to start of linear DFB
                    + (rclDraw.top  * lSrcDelta)            // start of rectangle
                    + (ppdev->cjPelSize * rclDraw.left);
        }

    #else
        lSrcDelta = ppdev->lDelta;
        pjSrc     = ppdev->pjScreen + rclDraw.top  * lSrcDelta
                                    + (ppdev->cjPelSize * rclDraw.left);
    #endif

    lDstDelta = psoDst->lDelta;
    pjDst     = (BYTE*) psoDst->pvScan0 + prclDst->top  * lDstDelta
                                        + (ppdev->cjPelSize * prclDst->left);

    cjScan = ppdev->cjPelSize * (rclDraw.right  - rclDraw.left);
    cyScan = (rclDraw.bottom - rclDraw.top);

	// This routine is used by the software cursor and since we use async
	// cursor calls we can get very frequent activity here when a software
	// cursor is used.  For some reason we need the idle here in addition to
	// the one in vAlignedCopy otherwise there is some SW cursor corruption.

    START_DIRECT_ACCESS_H3(ppdev,ppdev->pjBase);

    vAlignedCopy(ppdev, &pjDst, lDstDelta, &pjSrc, lSrcDelta, cjScan, cyScan,
                 FALSE);            // Screen is the source

    END_DIRECT_ACCESS_H3(ppdev,ppdev->pjBase);

    DISPDBG((5, "vGetBits -- exit"));
}


/******************************Public*Routine******************************\
* VOID vPutBits
*
* Copies the bits from the given surface to the screen, using the memory
* aperture.  Must be pre-clipped.
*
\**************************************************************************/

VOID vPutBits(
PDEV*       ppdev,
SURFOBJ*    psoSrc,
RECTL*      prclDst,            // Absolute coordinates!
POINTL*     pptlSrc)            // Absolute coordinates!
{
    RECTL   rclDraw;		// Not needed for non LFB case.

    DISPDBG((5, "vPutBits -- enter"));

    rclDraw = *prclDst;    // jdw - eliminate if using vXferNative

    #if !(ENABLE_LINEAR_DFBS)
    // NVH - Assert does not apply to linear DFBs
    ASSERTDD((rclDraw.left   >= 0) &&
             (rclDraw.top    >= 0) &&
             (rclDraw.right  <= ppdev->cxMemory) &&
             (rclDraw.bottom <= ppdev->cyMemory),
             "vPutBits: rectangle wasn't fully clipped");
    #endif

//    if (ppdev->ulBoardId == MGA_STORM)
    if( 1 )		// jdw - change this to use vXferNative so we don't idle HW.
    {
        LONG xOffset;
        LONG yOffset;

        // 'vXferNative' takes relative coordinates, but we have absolute
        // coordinates here.  Temporarily adjust our offset variables:

        xOffset = ppdev->xOffset;
        yOffset = ppdev->yOffset;

        ppdev->xOffset = 0;
        ppdev->yOffset = 0;

        // jdw - looks like vMmXferNative (go through a pfn)
        vMmXferNative(ppdev, 1, prclDst, 0xCCCC, psoSrc, pptlSrc, prclDst, NULL);

        ppdev->xOffset = xOffset;
        ppdev->yOffset = yOffset;
    }
#if 0
    else
    {
        LONG    cyScan;
        LONG    lDstDelta;
        LONG    lSrcDelta;
        BYTE*   pjDst;
        BYTE*   pjSrc;
        LONG    cjScan;

        // Use LFB's to do the transfer.

        // Calculate the pointer to the upper-left corner of both rectangles:

        lDstDelta = ppdev->lDelta;
        pjDst     = ppdev->pjScreen + rclDraw.top  * lDstDelta
                                    + (ppdev->cjPelSize * rclDraw.left);

        lSrcDelta = psoSrc->lDelta;
        pjSrc     = (BYTE*) psoSrc->pvScan0 + (pptlSrc->y * lSrcDelta)
                                            + (ppdev->cjPelSize * pptlSrc->x);

        cjScan = ppdev->cjPelSize * (rclDraw.right - rclDraw.left);
        cyScan = (rclDraw.bottom - rclDraw.top);

        vAlignedCopy(ppdev, &pjDst, lDstDelta, &pjSrc, lSrcDelta, cjScan, cyScan,
                 TRUE);            // Screen is the dest
    }
#endif

    DISPDBG((5, "vPutBits -- exit"));
}

/*----------------------------------------------------------------------
Function name:  miscGlideMapMemoryBases

Description:    

Return:         
----------------------------------------------------------------------*/

GLIDESTATE * hwcGetGlideStateStructureForProcess( HANDLE hProcess );

BOOL
miscGlideMapMemoryBases(PDEV *ppdev, HANDLE hProcess)
{
#ifdef SLI_AA
  VIDEO_PUBLIC_ACCESS_RANGES  VideoAccessRange[3 + HWINFO_SST_MAX_NUM_CHIPS * HWINFO_SST_MAX_CHIP_INDEX];
  ULONG                       i, j;
#else
  VIDEO_PUBLIC_ACCESS_RANGES  VideoAccessRange[3];
#endif
  DWORD                       ReturnedDataLength;
  GLIDESTATE                  *psGlideState;
  HANDLE                      hTempProcess = (HANDLE)-1;


  if ((psGlideState = hwcGetGlideStateStructureForProcess(hProcess)) == (GLIDESTATE *)NULL)
  {
    DISPDBG((0, "miscGlideMapMemoryBases - No State Structure Found For Current Process"));
    return( FALSE );	
  }

  if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_QUERY_GLIDE_ACCESS_RANGES,
                         &hTempProcess,             // input buffer
                         sizeof(hTempProcess),
                         &VideoAccessRange,         // output buffer
                         sizeof(VideoAccessRange),
                         &ReturnedDataLength))
  {
    DISPDBG((0, "miscGlideMapMemoryBases - Error mapping Glide bases"));
    goto ReturnFalse;
  }

  // Save the glide information

  //psGlideState->glideProcessHandle = hTempProcess;
  psGlideState->glideRegBase    = (BYTE *) VideoAccessRange[0].VirtualAddress;
  psGlideState->glideLfbBase    = (BYTE *) VideoAccessRange[1].VirtualAddress;
  psGlideState->glideScreenBase = (BYTE *) VideoAccessRange[1].VirtualAddress;
  psGlideState->glideIOBase     = (BYTE *) VideoAccessRange[2].VirtualAddress;

  DISPDBG((2, "miscGlideMapMemoryBases - Map Memory : PID=%8Xh, RegBase=%8lXh, LfbBase=%8lXh",
           hProcess,
           psGlideState->glideRegBase,
           psGlideState->glideLfbBase));

#ifdef SLI_AA
  memset(&psGlideState->glideSlaveRegBase[0], 0, sizeof(BYTE *) * HWINFO_SST_MAX_NUM_CHIPS * HWINFO_SST_MAX_CHIP_INDEX);

  for (i = 0; i < _FF(dwNumUnits); i++)
  {
    for (j = 0; j < HWINFO_SST_MAX_CHIP_INDEX; j++)
    {
      psGlideState->glideSlaveRegBase[i * HWINFO_SST_MAX_CHIP_INDEX + j] = VideoAccessRange[3 + i * HWINFO_SST_MAX_CHIP_INDEX + j].VirtualAddress;
    }
  }
#endif

  return TRUE;

ReturnFalse:
  return FALSE;
}

/*----------------------------------------------------------------------
Function name:  miscGlideUnmapMemoryBases

Description:    

Return:         
----------------------------------------------------------------------*/

BOOL
miscGlideUnmapMemoryBases(PDEV *ppdev, HANDLE hProcess)
{
#ifdef SLI_AA
  ULONG           i, j;
#endif
  DWORD           ReturnedDataLength;
  GLIDE_BASE_INFO glideBaseInfo;
  GLIDESTATE      *psGlideState;

  if( ( psGlideState = hwcGetGlideStateStructureForProcess( hProcess ) ) == ( GLIDESTATE * )NULL )
  {
    DISPDBG((0, "miscGlideUnmapMemoryBases - No State Structure Found For Current Process"));
    return( FALSE );	
  }

  DISPDBG((2, "miscGlideUnmapMemoryBases - Unmap Memory : PID=%8Xh, RegBase=%8lXh, LfbBase=%8lXh",
           psGlideState->glideProcessHandle,
           psGlideState->glideRegBase,
           psGlideState->glideLfbBase));

  glideBaseInfo.VideoMemory[0].RequestedVirtualAddress = psGlideState->glideRegBase;
  glideBaseInfo.VideoMemory[1].RequestedVirtualAddress = psGlideState->glideLfbBase;
  glideBaseInfo.VideoMemory[2].RequestedVirtualAddress = psGlideState->glideIOBase;
  glideBaseInfo.hProcess = (void *)-1;

#ifdef SLI_AA
  for (i = 0; i < _FF(dwNumUnits); i++)
  {
    for (j = 0; j < HWINFO_SST_MAX_CHIP_INDEX; j++)
    {
      glideBaseInfo.VideoMemory[3 + i * HWINFO_SST_MAX_CHIP_INDEX + j].RequestedVirtualAddress =
            psGlideState->glideSlaveRegBase[i * HWINFO_SST_MAX_CHIP_INDEX + j];
    }
  }
#endif

  if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_FREE_GLIDE_ACCESS_RANGES,
                         &glideBaseInfo,
                         sizeof(glideBaseInfo),
                         NULL,
                         0,
                         &ReturnedDataLength))
  {
    DISPDBG((0, "miscGlideUnmapMemoryBases failed IOCTL_VIDEO_FREE_GLIDE_ACCESS_RANGES"));
    return FALSE;
  }

  return TRUE;
}


VOID miscSaveExtState( PDEV *ppdev )
{
#if 0
	// this should be process relative, but this functiondid not get the process handle passed into 
	// it 
    glideState[ 0 ].vidProcCfg = GET(ghwIO->vidProcCfg);
    glideState[ 0 ].vidDesktopOverlayStride = GET(ghwIO->vidDesktopOverlayStride);
#endif

    return;
}

VOID miscRestoreExtState( PDEV *ppdev )
{
	// removed this line because it was causing problems with alot of restoration. It's not
	// needed because an actual mode set is done right before this call.
//    SETDW(ghwIO->vidProcCfg, glideState[ 0 ].vidProcCfg);

#if 0
    // fix for PRS #2514, don't restore saved desktop stride during
    // hwcReleaseContext
    // For MTM2, hwcGetLinearAddr is called while still in the desktop mode
    // so miscSaveExtState saves the desktop stride for that mode, but when
    // MTM2 shows the options screen it's in a 640x480 mode so the saved stride
    // is incorrect and we get a trashed screen if the original stride is
    // written to the hw here
    SETDW(ghwIO->vidDesktopOverlayStride, glideState[ 0 ].vidDesktopOverlayStride);
#endif

    return;
}

#if (_WIN32_WINNT >= 0x0500)
/******************************Public*Routine******************************\
* VOID DrvSynchronize
*
* This routine is called by GDI to synchronize on the accelerator before
* it draws directly to the driver's surface.  This function must be hooked
* by the driver when:
*
*   1. The primary surface is a GDI-managed surface, which simply means
*      that GDI can directly draw on the primary surface.  This happens
*      when DrvEnableSurface returns a handle created by EngCreateBitmap
*      instead of EngCreateDeviceSurface.
*
*   2. A device-bitmap is made into a GDI-managed surface by calling
*      EngModifySurface with a pointer directly to the bits.
*
\**************************************************************************/

VOID DrvSynchronize(
IN DHPDEV dhpdev,
IN RECTL *prcl)
{
    PDEV *ppdev = (PDEV *)dhpdev;

    START_DIRECT_ACCESS_H3(ppdev, ppdev->pjH3Base);
}
#endif

