/******************************Module*Header*******************************\
* Module Name: pointer.c
*
* This module contains the hardware pointer support for the display
* driver.
*
* Copyright (c) 1992-1996 Microsoft Corporation
* Copyright (c) 1997 3Dfx Interactive, Inc.
\**************************************************************************/

#include "precomp.h"

/******************************Public*Routine******************************\
* VOID vShowPointerH3
*
* Show or hide the H3 hardware pointer.
*
* We hide the pointer by making it only one row high (we always reserve
* the bottom scan of the pointer shape to be invisible).  We do it this
* way because we ran into problems doing it with any other method:
*
*   1. Disabling the hardware pointer via register CR45 will hang
*      80x/928/864 chips if it is done at exactly the wrong time during
*      the horizontal retrace.  It's is not safe to wait for vertical
*      blank and do it then, because we're a user mode process and
*      could get context switched after doing the wait but before setting
*      the bit.
*
*   2. Simply changing the pointer position to move it off-screen works,
*      but is not a good solution because the pointer position is latched
*      by the hardware, and it usually takes a couple of frames for the
*      new position to take effect (which causes the pointer to jump even
*      more than it currently does).
*
*   3. Using registers CR4C and CR4D to switch to a pre-defined 'invisible'
*      pointer also worked, but still caused machines to crash with the
*      same symptoms as from solution 1 (although it was somewhat more
*      rare).
*
\**************************************************************************/

VOID vShowPointerH3(
PDEV*   ppdev,
BOOL    bShow)      // If TRUE, show the pointer.  If FALSE, hide the pointer.
{
    LONG    xy;     // Combined x and y cursor position

    if (bShow)
    {
        // Make the hardware pointer visible:

        xy = ppdev->xyPointer;
    }
    else
    {
        // Move the hardware pointer off-screen so that it doesn't flash
        // in the old position when we finally turn it back on:

        xy = H3_PACKXY( 2047, 2047 );
    }

    SET_ABSOLUTE_DW( ppdev->pjBase + FIELDOFFSET(SstIORegs,hwCurLoc), xy );
}

/******************************Public*Routine******************************\
* VOID vMovePointerH3
*
* Move the H3 hardware pointer.
*
\**************************************************************************/

VOID vMovePointerH3(
PDEV*   ppdev,
LONG    x,
LONG    y)
{
//    BYTE*   pjBase = ppdev->pjBase;

    // I'll count on Windows to pass only valid cursor positions so we don't
    // need to check for out of range values here.  Just to CYA we'll assert
    // some valid positions.  1800 is arbitrary, we support 1600x1200 max
    // so 1600+64 should be enough, but let's be safe.

    ASSERTDD( ( x >= 0 ) && ( y >= 0 ) && ( x < ppdev->cxScreen ) && ( y < ppdev->cyScreen ),
              "Windows passed bogus pointer address -- add driver check");

    x -= ppdev->xPointerHot;
    y -= ppdev->yPointerHot;

    x += 63;    // Banshee cursor position specified at lower right corner.
    y += 63;

    ppdev->xyPointer = H3_PACKXY(x, y);
    SET_ABSOLUTE_DW( ppdev->pjBase + FIELDOFFSET(SstIORegs,hwCurLoc), ppdev->xyPointer);
}

/******************************Public*Routine******************************\
* VOID vSetPointerShapeH3
*
\**************************************************************************/

VOID vSetPointerShapeH3(
SURFOBJ*    pso,
LONG        x,              // Relative coordinates
LONG        y,              // Relative coordinates
LONG        xHot,
LONG        yHot,
BYTE*       pjShape,
FLONG       fl)
{
    BYTE*   pjBase;
    PDEV*   ppdev;
    ULONG*  pulSrc;
    ULONG*  pulDst;
    LONG    i;

    ppdev    = (PDEV*) pso->dhpdev;
    pjBase = ppdev->pjBase;

    // 1. Hide the current pointer.

    if (!(fl & SPS_ANIMATEUPDATE))
    {
        // Hide the pointer to try and lessen the jumpiness when the
        // new shape has a different hot spot.  We don't hide the
        // pointer while animating, because that definitely causes
        // flashing:

        SET_ABSOLUTE_DW( ppdev->pjBase + FIELDOFFSET(SstIORegs,hwCurLoc), H3_PACKXY( 2047, 2047 ));
    }

    // 2. Set the new pointer position.
    // --

    ppdev->xPointerHot = xHot;
    ppdev->yPointerHot = yHot;

    DrvMovePointer(pso, x, y, NULL);    // Note: Must pass relative coordinates!

    // 3. Download the new pointer shape.

    pulSrc = (ULONG*) pjShape;
    pulDst = (ULONG*) ppdev->pvPointerShape;

    for (i = HW_POINTER_TOTAL_SIZE / sizeof(ULONG); i != 0; i--)
    {
        *pulDst++ = *pulSrc++;
    }
}

/******************************Public*Routine******************************\
* VOID DrvMovePointer
*
* NOTE: Because we have set GCAPS_ASYNCMOVE, this call may occur at any
*       time, even while we're executing another drawing call!
*
*       Consequently, we have to explicitly synchronize any shared
*       resources.  In our case, since we touch the CRTC register here
*       and in the banking code, we synchronize access using a critical
*       section.
*
\**************************************************************************/

VOID DrvMovePointer(
SURFOBJ*    pso,
LONG        x,
LONG        y,
RECTL*      prcl)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) pso->dhpdev;

//    ACQUIRE_CRTC_CRITICAL_SECTION(ppdev);

    if (x != -1)
    {
        // Convert the pointer's position from relative to absolute
        // coordinates (this is only significant for multiple board
        // support):

        #if MULTI_BOARDS
        {
            OH* poh;

            poh = ((DSURF*) pso->dhsurf)->poh;
            x += poh->x;
            y += poh->y;
        }
        #endif

        vMovePointerH3(ppdev, x, y);

        if (!ppdev->bHwPointerActive)
        {
            // We have to make the pointer visible:

            ppdev->bHwPointerActive = TRUE;

            vShowPointerH3(ppdev, TRUE);
        }
    }
    else
    {
        if (ppdev->bHwPointerActive)
        {
            // The pointer is visible, and we've been asked to hide it:

            ppdev->bHwPointerActive = FALSE;

            vShowPointerH3(ppdev, FALSE);
        }
    }

//    RELEASE_CRTC_CRITICAL_SECTION(ppdev);

    // Note that we don't have to modify 'prcl', since we have a
    // NOEXCLUDE pointer...
}

/******************************Public*Routine******************************\
* VOID DrvSetPointerShape
*
* Sets the new pointer shape.
*
\**************************************************************************/

ULONG DrvSetPointerShape(
SURFOBJ*    pso,
SURFOBJ*    psoMsk,
SURFOBJ*    psoColor,
XLATEOBJ*   pxlo,
LONG        xHot,
LONG        yHot,
LONG        x,
LONG        y,
RECTL*      prcl,
FLONG       fl)
{
    PDEV*   ppdev;
    DWORD*  pul;
    ULONG   cx;
    ULONG   cy;
    LONG    i;
    LONG    j;
    BYTE*   pjSrcScan;
    BYTE*   pjDstScan;
    LONG    lSrcDelta;
    LONG    lDstDelta;
    WORD*   pwSrc;
    WORD*   pwDst;
    LONG    cwWhole;
    DWORD*  pulSrc;
    DWORD*  pulDst;
    LONG    culWhole;
    BYTE    ajBuf[HW_POINTER_TOTAL_SIZE];

    ppdev = (PDEV*) pso->dhpdev;

#ifdef CSIM
    return(SPS_DECLINE);
#endif

    // When CAPS_SW_POINTER is set, we have no hardware pointer available,
    // so we always ask GDI to simulate the pointer for us, using
    // DrvCopyBits calls:

    if (ppdev->flCaps & CAPS_SW_POINTER)
        return(SPS_DECLINE);

    // We're not going to handle any colour pointers, pointers that
    // are larger than our hardware allows, or flags that we don't
    // understand.
    //
    // (Note that the spec says we should decline any flags we don't
    // understand, but we'll actually be declining if we don't see
    // the only flag we *do* understand...)
    //
    // Our old documentation says that 'psoMsk' may be NULL, which means
    // that the pointer is transparent.  Well, trust me, that's wrong.
    // I've checked GDI's code, and it will never pass us a NULL psoMsk:

    cx = psoMsk->sizlBitmap.cx;         // Note that 'sizlBitmap.cy' accounts
    cy = psoMsk->sizlBitmap.cy >> 1;    //   for the double height due to the
                                        //   inclusion of both the AND masks
                                        //   and the XOR masks.  For now, we're
                                        //   only interested in the true
                                        //   pointer dimensions, so we divide
                                        //   by 2.

    if ((cx > HW_POINTER_DIMENSION)   ||
        (cy > HW_POINTER_DIMENSION)   ||
        (psoColor != NULL)            ||
        !(fl & SPS_CHANGE)            ||
        (cx & 0x7))     // make sure cx is a multiple of 8 (byte aligned).
    {
        goto HideAndDecline;
    }

    ASSERTDD(psoMsk != NULL, "GDI gave us a NULL psoMsk.  It can't do that!");
#if (_WIN32_WINNT < 0x0500)
    ASSERTDD(pso->iType == STYPE_DEVICE, "GDI gave us a weird surface");
#endif

    pul = (ULONG*) &ajBuf[0];
    for (i = HW_POINTER_TOTAL_SIZE / ( sizeof(ULONG) * 4 ); i != 0; i--)
    {
        // Here we initialize the entire pointer work buffer to be
        // transparent (Banshee has no means of specifying a pointer size
        // other than 64 x 64 -- so if we're asked to draw a 32 x 32
        // pointer, we want the unused portion to be transparent).
        //
        // Banshee's hardware pointer is defined by an interleaved pattern
        // of AND words and XOR words, 64 bits per word.  So a totally
        // transparent pointer starts off with the word 0xffffffffffffffff,
        // followed by the word 0x0000000000000000, followed by 0xffffffffffffffff,
        // etc..
        //
        // We try to leverage the Pentium's addressing modes here.

        *pul     = 0xffffffff;
        *(pul+1) = 0xffffffff;
        *(pul+2) = 0x00000000;
        *(pul+3) = 0x00000000;

        pul += 4;
    }

    // Now we're going to take the requested pointer AND masks and XOR
    // masks and combine them into our work buffer, being careful of
    // the edges so that we don't disturb the transparency when the
    // requested pointer size is not a multiple of 16.
    //
    // 'psoMsk' is actually cy * 2 scans high; the first 'cy' scans
    // define the AND mask.  So we start with that:

    pjSrcScan    = psoMsk->pvScan0;
    lSrcDelta    = psoMsk->lDelta;
    pjDstScan    = &ajBuf[0];               // Start with first AND word
    lDstDelta    = HW_POINTER_DIMENSION / 4;// Every 8 pels is one AND/XOR word


    if( (cx & 0x1f) == 0 )	// Use dwords if cx is a multiple of 32 bits
    {
        culWhole      = cx / 32;                 // Each dword accounts for 32 pels

        for (i = cy; i != 0; i--)
        {
            pulSrc = (DWORD*) pjSrcScan;
            pulDst = (DWORD*) pjDstScan;

            for (j = 0; j < culWhole; j++)
            {
                *pulDst = *pulSrc;
                pulSrc += 1;             // Go to next dword in source mask
                if(  j & 0x1 )
                    pulDst += 3;             // Skip over the XOR bits in the dest mask
                else
                    pulDst += 1;             // Point to the next AND dword in the dest mask
            }

            pjSrcScan += lSrcDelta;
            pjDstScan += lDstDelta;
        }

        // Now handle the XOR mask:

        pjDstScan = &ajBuf[8];          // Start with first XOR word
        for (i = cy; i != 0; i--)
        {
            pulSrc = (DWORD*) pjSrcScan;
            pulDst = (DWORD*) pjDstScan;

            for (j = 0; j < culWhole; j++)
            {
                *pulDst = *pulSrc;
                pulSrc += 1;             // Go to next word in source mask
                if(  j & 0x1 )
                    pulDst += 3;             // Skip over the AND dword in the dest mask
                else
                    pulDst += 1;             // Point to the next XOR dword in the dest mask
            }

            pjSrcScan += lSrcDelta;
            pjDstScan += lDstDelta;
        }
    } // cx
    else
    {
        // Looks like there could be problems here with odd bit sized cx
        // but MS uses it this way so what the heck, we will too.

        cwWhole      = cx / 16;                 // Each word accounts for 16 pels

        for (i = cy; i != 0; i--)
        {
            pwSrc = (WORD*) pjSrcScan;
            pwDst = (WORD*) pjDstScan;

            for (j = 0; j < cwWhole ; j++)
            {
                *pwDst = *pwSrc;
                pwSrc += 1;             // Go to next word in source mask
                if(  j & 0x3 )
                    pwDst += 5;         // Skip over the XOR bits in the dest mask
                else
                    pwDst += 1;         // Point to the next AND word in the dest mask
            }

            pjSrcScan += lSrcDelta;
            pjDstScan += lDstDelta;
        }

        // Now handle the XOR mask:

        pjDstScan = &ajBuf[8];          // Start with first XOR word
        for (i = cy; i != 0; i--)
        {
            pwSrc = (WORD*) pjSrcScan;
            pwDst = (WORD*) pjDstScan;

            for (j = 0; j < cwWhole ; j++)
            {
                *pwDst = *pwSrc;
                pwSrc += 1;             // Go to next word in source mask
                if(  j & 0x3 )
                    pwDst += 5;         // Skip over the AND bits in the dest mask
                else
                    pwDst += 1;         // Point to the next XOR word in the dest mask
            }

            pjSrcScan += lSrcDelta;
            pjDstScan += lDstDelta;
        }
    }

    // Convert the pointer's position from relative to absolute
    // coordinates (this is only significant for multiple board
    // support):

    #if MULTI_BOARDS
    {
        OH*  poh;
        if (x != -1)
        {
            poh = ((DSURF*) pso->dhsurf)->poh;
            x += poh->x;
            y += poh->y;
        }
    }
    #endif

    vSetPointerShapeH3(pso, x, y, xHot, yHot, &ajBuf[0], fl);

    // Since it's a hardware pointer, GDI doesn't have to worry about
    // overwriting the pointer on drawing operations (meaning that it
    // doesn't have to exclude the pointer), so we return 'NOEXCLUDE'.
    // Since we're returning 'NOEXCLUDE', we also don't have to update
    // the 'prcl' that GDI passed us.

    return(SPS_ACCEPT_NOEXCLUDE);

HideAndDecline:

    // Since we're declining the new pointer, GDI will simulate it via
    // DrvCopyBits calls.  So we should really hide the old hardware
    // pointer if it's visible.  We can get DrvMovePointer to do this
    // for us:

    DrvMovePointer(pso, -1, -1, NULL);

    return(SPS_DECLINE);
}

/******************************Public*Routine******************************\
* VOID vDisablePointer
*
\**************************************************************************/

VOID vDisablePointer(
PDEV*   ppdev)
{
    // Nothing to do, really
}

/******************************Public*Routine******************************\
* VOID vAssertModePointer
*
\**************************************************************************/

VOID vAssertModePointer(
PDEV*   ppdev,
BOOL    bEnable)
{
    ULONG*  pulDst;
    LONG    i;
    ULONG    ulReg;

    // We will turn any hardware pointer -- either in the S3 or in the
    // DAC -- off to begin with:

    ppdev->bHwPointerActive = FALSE;

#ifndef CSIM
    if (ppdev->flCaps & CAPS_SW_POINTER)
    {
        // With a software pointer, we don't have to do anything.
    }
    else
    {
        // We're using the built-in hardware pointer:

        if (bEnable)
        {
            // We download an invisible pointer shape because we're about
            // to enable the hardware pointer, but we still want the
            // pointer hidden until we get the first DrvSetPointerShape
            // call:

            pulDst = (ULONG*) ppdev->pvPointerShape;
            for (i = HW_POINTER_TOTAL_SIZE / ( sizeof(ULONG) * 4 ); i != 0; i--)
            {
                *pulDst     = 0xffffffff;
                *(pulDst+1) = 0xffffffff;
                *(pulDst+2) = 0x00000000;
                *(pulDst+3) = 0x00000000;

                pulDst += 4;
            }

            // Point Banshee to where we're storing the pointer shape.
            // The location should be 16 byte aligned.

            ASSERTDD( ppdev->cjPointerOffset == (ppdev->cjPointerOffset & ~0xf),
                  "Cursor pattern not on 16 byte boundary" );

            SET_ABSOLUTE_DW( ppdev->pjBase + FIELDOFFSET(SstIORegs,hwCurPatAddr), ppdev->cjPointerOffset);

            // Now hide it by moving it off-screen:

            vShowPointerH3(ppdev, FALSE);

			// Set the pointer colors
			// rml - don't use the flipped colors suitable for A0 boards - long live A1!
			
            SET_ABSOLUTE_DW( ppdev->pjBase + FIELDOFFSET(SstIORegs,hwCurC0), 0x0 );
            SET_ABSOLUTE_DW( ppdev->pjBase + FIELDOFFSET(SstIORegs,hwCurC1), 0xffffff );

            // Enable the hardware pointer.

            ulReg = GET_ABSOLUTE( ppdev->pjBase + FIELDOFFSET(SstIORegs,vidProcCfg));

            SET_ABSOLUTE_DW( ppdev->pjBase + FIELDOFFSET(SstIORegs,vidProcCfg), ulReg | SST_CURSOR_EN );
        }
    }
#endif
}

/******************************Public*Routine******************************\
* BOOL bEnablePointer
*
\**************************************************************************/

BOOL bEnablePointer(
PDEV*   ppdev)
{
    LONG        cjOffset;

#ifndef CSIM
    if (ppdev->flCaps & CAPS_SW_POINTER)
    {
        // With a software pointer, we don't have to do anything.
    }
    else
    {
        // Compute address of pointer shape.

        ppdev->pvPointerShape = ppdev->pjScreenBase + ppdev->cjPointerOffset;
    }
#endif

    // Actually turn on the pointer:

    vAssertModePointer(ppdev, TRUE);

    DISPDBG((5, "Passed bEnablePointer"));

    return(TRUE);
}
