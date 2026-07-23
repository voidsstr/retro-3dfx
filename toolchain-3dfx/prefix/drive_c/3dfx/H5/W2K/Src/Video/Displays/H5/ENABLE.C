/******************************Module*Header*******************************\
* Module Name: enable.c
*
* This module contains the functions that enable and disable the
* driver, the pdev, and the surface.
*
* Copyright (c) 1992-1996 Microsoft Corporation
\**************************************************************************/

/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
*/

#include "precomp.h"
#define TVOUT_SUPPORTED 1
#ifdef TVOUT_SUPPORTED
#include "edgeesc.h"
#endif //def TVOUT_SUPPORTED


#if ENABLE_RECONFIG_VIDMEM
void reconfigureVideoMemory(PDEV *ppdev);
#endif

extern void ClearGlideStateFlags(PDEV *ppdev, LONG state);
extern void SetGlideStateFlags(PDEV *ppdev, LONG state);

/******************************Public*Structure****************************\
* GDIINFO ggdiDefault
*
* This contains the default GDIINFO fields that are passed back to GDI
* during DrvEnablePDEV.
*
* NOTE: This structure defaults to values for an 8bpp palette device.
*       Some fields are overwritten for different colour depths.
\**************************************************************************/

GDIINFO ggdiDefault = {
    GDI_DRIVER_VERSION,
    DT_RASDISPLAY,          // ulTechnology
    0,                      // ulHorzSize (filled in later)
    0,                      // ulVertSize (filled in later)
    0,                      // ulHorzRes (filled in later)
    0,                      // ulVertRes (filled in later)
    0,                      // cBitsPixel (filled in later)
    0,                      // cPlanes (filled in later)
    20,                     // ulNumColors (palette managed)
    0,                      // flRaster (DDI reserved field)

    0,                      // ulLogPixelsX (filled in later)
    0,                      // ulLogPixelsY (filled in later)

    TC_RA_ABLE,             // flTextCaps -- If we had wanted console windows
                            //   to scroll by repainting the entire window,
                            //   instead of doing a screen-to-screen blt, we
                            //   would have set TC_SCROLLBLT (yes, the flag is
                            //   bass-ackwards).

    0,                      // ulDACRed (filled in later)
    0,                      // ulDACGreen (filled in later)
    0,                      // ulDACBlue (filled in later)

    0x0024,                 // ulAspectX
    0x0024,                 // ulAspectY
    0x0033,                 // ulAspectXY (one-to-one aspect ratio)

    1,                      // xStyleStep
    1,                      // yStyleSte;
    3,                      // denStyleStep -- Styles have a one-to-one aspect
                            //   ratio, and every 'dot' is 3 pixels long

    { 0, 0 },               // ptlPhysOffset
    { 0, 0 },               // szlPhysSize

    256,                    // ulNumPalReg

    // These fields are for halftone initialization.  The actual values are
    // a bit magic, but seem to work well on our display.

    {                       // ciDevice
       { 6700, 3300, 0 },   //      Red
       { 2100, 7100, 0 },   //      Green
       { 1400,  800, 0 },   //      Blue
       { 1750, 3950, 0 },   //      Cyan
       { 4050, 2050, 0 },   //      Magenta
       { 4400, 5200, 0 },   //      Yellow
       { 3127, 3290, 0 },   //      AlignmentWhite
       20000,               //      RedGamma
       20000,               //      GreenGamma
       20000,               //      BlueGamma
       0, 0, 0, 0, 0, 0     //      No dye correction for raster displays
    },

    0,                       // ulDevicePelsDPI (for printers only)
    PRIMARY_ORDER_CBA,       // ulPrimaryOrder
    HT_PATSIZE_4x4_M,        // ulHTPatternSize
    HT_FORMAT_8BPP,          // ulHTOutputFormat
    HT_FLAG_ADDITIVE_PRIMS,  // flHTFlags
    0,                       // ulVRefresh
    0,                       // ulPanningHorzRes
    0,                       // ulPanningVertRes
    0,                       // ulBltAlignment
};

/******************************Public*Structure****************************\
* DEVINFO gdevinfoDefault
*
* This contains the default DEVINFO fields that are passed back to GDI
* during DrvEnablePDEV.
*
* NOTE: This structure defaults to values for an 8bpp palette device.
*       Some fields are overwritten for different colour depths.
\**************************************************************************/

#define SYSTM_LOGFONT {16,7,0,0,700,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
                       CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,\
                       VARIABLE_PITCH | FF_DONTCARE,L"System"}
#define HELVE_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
                       CLIP_STROKE_PRECIS,PROOF_QUALITY,\
                       VARIABLE_PITCH | FF_DONTCARE,L"MS Sans Serif"}
#define COURI_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
                       CLIP_STROKE_PRECIS,PROOF_QUALITY,\
                       FIXED_PITCH | FF_DONTCARE, L"Courier"}

DEVINFO gdevinfoDefault = {
    (GCAPS_OPAQUERECT       |
     GCAPS_DITHERONREALIZE  |
     GCAPS_PALMANAGED       |
     GCAPS_ALTERNATEFILL    |
     GCAPS_WINDINGFILL      |
     GCAPS_MONO_DITHER      |
     GCAPS_COLOR_DITHER     |
#ifdef USE_DDRAW_CODE
     GCAPS_DIRECTDRAW       |
#endif
     GCAPS_ASYNCMOVE),          // NOTE: Only enable ASYNCMOVE if your code
                                //   and hardware can handle DrvMovePointer
                                //   calls at any time, even while another
                                //   thread is in the middle of a drawing
                                //   call such as DrvBitBlt.

                                                // flGraphicsFlags
    SYSTM_LOGFONT,                              // lfDefaultFont
    HELVE_LOGFONT,                              // lfAnsiVarFont
    COURI_LOGFONT,                              // lfAnsiFixFont
    0,                                          // cFonts
    BMF_8BPP,                                   // iDitherFormat
    8,                                          // cxDither
    8,                                          // cyDither
    0                                           // hpalDefault (filled in later)
#if (_WIN32_WINNT >= 0x0500)
    , (GCAPS2_CHANGEGAMMARAMP)                  // flGraphicsCaps2
#endif
};

/******************************Public*Structure****************************\
* DFVFN gadrvfn[]
*
* Build the driver function table gadrvfn with function index/address
* pairs.  This table tells GDI which DDI calls we support, and their
* location (GDI does an indirect call through this table to call us).
*
* Why haven't we implemented DrvSaveScreenBits?  To save code.
*
* When the driver doesn't hook DrvSaveScreenBits, USER simulates on-
* the-fly by creating a temporary device-format-bitmap, and explicitly
* calling DrvCopyBits to save/restore the bits.  Since we already hook
* DrvCreateDeviceBitmap, we'll end up using off-screen memory to store
* the bits anyway (which would have been the main reason for implementing
* DrvSaveScreenBits).  So we may as well save some working set.
\**************************************************************************/

#if MULTI_BOARDS

// Multi-board support has its own thunks...

DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) MulEnablePDEV         },
    {   INDEX_DrvCompletePDEV,          (PFN) MulCompletePDEV       },
    {   INDEX_DrvDisablePDEV,           (PFN) MulDisablePDEV        },
    {   INDEX_DrvResetPDEV,             (PFN) DrvResetPDEV          },
    {   INDEX_DrvEnableSurface,         (PFN) MulEnableSurface      },
    {   INDEX_DrvDisableSurface,        (PFN) MulDisableSurface     },
    {   INDEX_DrvAssertMode,            (PFN) MulAssertMode         },
    {   INDEX_DrvMovePointer,           (PFN) MulMovePointer        },
    {   INDEX_DrvSetPointerShape,       (PFN) MulSetPointerShape    },
    {   INDEX_DrvDitherColor,           (PFN) MulDitherColor        },
    {   INDEX_DrvSetPalette,            (PFN) MulSetPalette         },
    {   INDEX_DrvCopyBits,              (PFN) MulCopyBits           },
    {   INDEX_DrvBitBlt,                (PFN) MulBitBlt             },
    {   INDEX_DrvTextOut,               (PFN) MulTextOut            },
    {   INDEX_DrvGetModes,              (PFN) MulGetModes           },
    {   INDEX_DrvStrokePath,            (PFN) MulStrokePath         },
    {   INDEX_DrvFillPath,              (PFN) MulFillPath           },
    {   INDEX_DrvPaint,                 (PFN) MulPaint              },
    {   INDEX_DrvRealizeBrush,          (PFN) MulRealizeBrush       },
    {   INDEX_DrvDestroyFont,           (PFN) MulDestroyFont        },
    // Note that we don't support DrvCreateDeviceBitmap for multi-boards
    // Note that we don't support DrvDeleteDeviceBitmap for multi-boards
    // Note that we don't support DrvStretchBlt for multi-boards
    // Note that we don't support DrvLineTo for multi-boards
    // Note that we don't support DrvEscape for multi-boards
    // Note that we don't support DrvDirectDraw functions for multi-boards
};

#elif DBG

// On Checked builds, thunk everything through Dbg calls...

DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) DbgEnablePDEV         },
    {   INDEX_DrvCompletePDEV,          (PFN) DbgCompletePDEV       },
    {   INDEX_DrvDisablePDEV,           (PFN) DbgDisablePDEV        },
    {   INDEX_DrvResetPDEV,             (PFN) DbgResetPDEV          },
    {   INDEX_DrvEnableSurface,         (PFN) DbgEnableSurface      },
    {   INDEX_DrvDisableSurface,        (PFN) DbgDisableSurface     },
    {   INDEX_DrvAssertMode,            (PFN) DbgAssertMode         },
    {   INDEX_DrvMovePointer,           (PFN) DbgMovePointer        },
    {   INDEX_DrvSetPointerShape,       (PFN) DbgSetPointerShape    },
    {   INDEX_DrvDitherColor,           (PFN) DbgDitherColor        },
    {   INDEX_DrvSetPalette,            (PFN) DbgSetPalette         },
    {   INDEX_DrvCopyBits,              (PFN) DbgCopyBits           },
    {   INDEX_DrvBitBlt,                (PFN) DbgBitBlt             },
    {   INDEX_DrvTextOut,               (PFN) DbgTextOut            },
    {   INDEX_DrvGetModes,              (PFN) DbgGetModes           },
    {   INDEX_DrvLineTo,                (PFN) DbgLineTo             },
    {   INDEX_DrvStrokePath,            (PFN) DbgStrokePath         },
    {   INDEX_DrvFillPath,              (PFN) DbgFillPath           },
#if (_WIN32_WINNT == 0x0400)
    {   INDEX_DrvPaint,                 (PFN) DbgPaint              },
#endif
    {   INDEX_DrvRealizeBrush,          (PFN) DbgRealizeBrush       },
#if (USE_NT5_DDMEMMGR == 0) || ((USE_NT5_DDMEMMGR == 1) && (ENABLE_LINEAR_DFBS))
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DbgCreateDeviceBitmap },
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DbgDeleteDeviceBitmap },
#endif
    {   INDEX_DrvStretchBlt,            (PFN) DbgStretchBlt         },
    {   INDEX_DrvDestroyFont,           (PFN) DbgDestroyFont        },
#ifdef USE_DDRAW_CODE
    {   INDEX_DrvGetDirectDrawInfo,     (PFN) DbgGetDirectDrawInfo  },
    {   INDEX_DrvEnableDirectDraw,      (PFN) DbgEnableDirectDraw   },
    {   INDEX_DrvDisableDirectDraw,     (PFN) DbgDisableDirectDraw  },
#endif
#if (_WIN32_WINNT >= 0x0500)
    {   INDEX_DrvSynchronize,           (PFN) DbgSynchronize        },
    {   INDEX_DrvGradientFill,          (PFN) DbgGradientFill       },
    //    {   INDEX_DrvTransparentBlt,        (PFN) DrvTransparentBlt        },
#if USE_NT5_DDMEMMGR
#if ENABLE_LINEAR_DFBS
    // fix for MS RAID bug 280840
    // without device bitmap support, D3D in a window runs slow
    // when a sw cursor is in use and we hook this function
    {   INDEX_DrvDeriveSurface,         (PFN) DrvDeriveSurface         },
#endif
#endif
    {   INDEX_DrvIcmSetDeviceGammaRamp, (PFN) DbgIcmSetDeviceGammaRamp },
#endif
    {   INDEX_DrvEscape,                (PFN) DrvEscape             },
#ifdef CSIM 			//  TAKE_THIS_OUT_AFTER_TEST   // jdw
    {   INDEX_DrvDrawEscape,            (PFN) DrvDrawEscape         },
#endif
#ifdef OPENGL_ICD
    {   INDEX_DrvDescribePixelFormat,   (PFN) DrvDescribePixelFormat},
    {   INDEX_DrvSetPixelFormat,        (PFN) DrvSetPixelFormat},
    {   INDEX_DrvSwapBuffers,           (PFN) DrvSwapBuffers}
#endif // OPENGL_ICD

};

#else

// On Free builds, directly call the appropriate functions...

DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) DrvEnablePDEV         },
    {   INDEX_DrvCompletePDEV,          (PFN) DrvCompletePDEV       },
    {   INDEX_DrvDisablePDEV,           (PFN) DrvDisablePDEV        },
    {   INDEX_DrvResetPDEV,             (PFN) DrvResetPDEV          },
    {   INDEX_DrvEnableSurface,         (PFN) DrvEnableSurface      },
    {   INDEX_DrvDisableSurface,        (PFN) DrvDisableSurface     },
    {   INDEX_DrvAssertMode,            (PFN) DrvAssertMode         },
    {   INDEX_DrvMovePointer,           (PFN) DrvMovePointer        },
    {   INDEX_DrvSetPointerShape,       (PFN) DrvSetPointerShape    },
    {   INDEX_DrvDitherColor,           (PFN) DrvDitherColor        },
    {   INDEX_DrvSetPalette,            (PFN) DrvSetPalette         },
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits           },
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt             },
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut            },
    {   INDEX_DrvGetModes,              (PFN) DrvGetModes           },
    {   INDEX_DrvLineTo,                (PFN) DrvLineTo             },
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath         },
    {   INDEX_DrvFillPath,              (PFN) DrvFillPath           },
#if (_WIN32_WINNT == 0x0400)
    {   INDEX_DrvPaint,                 (PFN) DrvPaint              },
#endif
    {   INDEX_DrvRealizeBrush,          (PFN) DrvRealizeBrush       },
#if (USE_NT5_DDMEMMGR == 0) || ((USE_NT5_DDMEMMGR == 1) && (ENABLE_LINEAR_DFBS))
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DrvCreateDeviceBitmap },
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DrvDeleteDeviceBitmap },
#endif
    {   INDEX_DrvStretchBlt,            (PFN) DrvStretchBlt         },
    {   INDEX_DrvDestroyFont,           (PFN) DrvDestroyFont        },
#ifdef USE_DDRAW_CODE
    {   INDEX_DrvGetDirectDrawInfo,     (PFN) DrvGetDirectDrawInfo  },
    {   INDEX_DrvEnableDirectDraw,      (PFN) DrvEnableDirectDraw   },
    {   INDEX_DrvDisableDirectDraw,     (PFN) DrvDisableDirectDraw  },
#endif
#if (_WIN32_WINNT >= 0x0500)
    {   INDEX_DrvSynchronize,           (PFN) DrvSynchronize        },
    {   INDEX_DrvGradientFill,          (PFN) DrvGradientFill       },
//    {   INDEX_DrvTransparentBlt,        (PFN) DrvTransparentBlt        },
#if USE_NT5_DDMEMMGR
#if ENABLE_LINEAR_DFBS
    // fix for MS RAID bug 280840
    // without device bitmap support, D3D in a window runs slow
    // when a sw cursor is in use and we hook this function
    {   INDEX_DrvDeriveSurface,         (PFN) DrvDeriveSurface         },
#endif
#endif
    {   INDEX_DrvIcmSetDeviceGammaRamp, (PFN) DrvIcmSetDeviceGammaRamp },
#endif
    {   INDEX_DrvEscape,                (PFN) DrvEscape             },
#ifdef CSIM		// TAKE_THIS_OUT_AFTER_TEST   // jdw
    {   INDEX_DrvDrawEscape,            (PFN) DrvDrawEscape         },
#endif
#ifdef OPENGL_ICD
    {   INDEX_DrvDescribePixelFormat,   (PFN) DrvDescribePixelFormat},
    {   INDEX_DrvSetPixelFormat,        (PFN) DrvSetPixelFormat},
    {   INDEX_DrvSwapBuffers,           (PFN) DrvSwapBuffers}
#endif // OPENGL_ICD
};

#endif

ULONG gcdrvfn = sizeof(gadrvfn) / sizeof(DRVFN);

/******************************Public*Routine******************************\
* BOOL DrvEnableDriver
*
* Enables the driver by retrieving the drivers function table and version.
*
\**************************************************************************/

BOOL DrvEnableDriver(
ULONG          iEngineVersion,
ULONG          cj,
DRVENABLEDATA* pded)
{
    // Engine Version is passed down so future drivers can support previous
    // engine versions.  A next generation driver can support both the old
    // and new engine conventions if told what version of engine it is
    // working with.  For the first version the driver does nothing with it.

    // Fill in as much as we can.

    if (cj >= sizeof(DRVENABLEDATA))
        pded->pdrvfn = gadrvfn;

    if (cj >= (sizeof(ULONG) * 2))
        pded->c = gcdrvfn;

    // DDI version this driver was targeted for is passed back to engine.
    // Future graphic's engine may break calls down to old driver format.

    if (cj >= sizeof(ULONG))
#ifndef MS_VIEW
        pded->iDriverVersion = DDI_DRIVER_VERSION;
#else
		// Report new DDI_DRIVER_VERSION for Whistler
		pded->iDriverVersion = DDI_DRIVER_VERSION_NT5_01;
#endif

    return(TRUE);
}

/******************************Public*Routine******************************\
* VOID DrvDisableDriver
*
* Tells the driver it is being disabled. Release any resources allocated in
* DrvEnableDriver.
*
\**************************************************************************/

VOID DrvDisableDriver(VOID)
{
    return;
}

/******************************Public*Routine******************************\
* DHPDEV DrvEnablePDEV
*
* Initializes a bunch of fields for GDI, based on the mode we've been asked
* to do.  This is the first thing called after DrvEnableDriver, when GDI
* wants to get some information about us.
*
* (This function mostly returns back information; DrvEnableSurface is used
* for initializing the hardware and driver components.)
*
\**************************************************************************/

DHPDEV DrvEnablePDEV(
DEVMODEW*   pdm,            // Contains data pertaining to requested mode
PWSTR       pwszLogAddr,    // Logical address
ULONG       cPat,           // Count of standard patterns
HSURF*      phsurfPatterns, // Buffer for standard patterns
ULONG       cjCaps,         // Size of buffer for device caps 'pdevcaps'
ULONG*      pdevcaps,       // Buffer for device caps, also known as 'gdiinfo'
ULONG       cjDevInfo,      // Number of bytes in device info 'pdi'
DEVINFO*    pdi,            // Device information
HDEV        hdev,           // HDEV, used for callbacks
PWSTR       pwszDeviceName, // Device name
HANDLE      hDriver)        // Kernel driver handle
{
    PDEV*   ppdev;

    // Future versions of NT had better supply 'devcaps' and 'devinfo'
    // structures that are the same size or larger than the current
    // structures:

    if ((cjCaps < sizeof(GDIINFO)) || (cjDevInfo < sizeof(DEVINFO)))
    {
        DISPDBG((0, "DrvEnablePDEV - Buffer size too small"));
        goto ReturnFailure0;
    }

    // Allocate a physical device structure.  Note that we definitely
    // rely on the zero initialization:

    ppdev = ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(PDEV), ALLOC_TAG, 0);
    if (ppdev == NULL)
    {
        DISPDBG((0, "DrvEnablePDEV - Failed EngAllocMem"));
        goto ReturnFailure0;
    }

    ppdev->hDriver = hDriver;

    H3PRINTF((ppdev, "DrvEnablePDEV\r\n"));

	// Get the device information
	if (!bGetDeviceInfo(ppdev))
	{
        DISPDBG((0, "DrvEnablePDEV - Failed bGetDeviceInfo"));
        goto ReturnFailure1;
	}

    // Get the current screen mode information.  Set up device caps and
    // devinfo:

    if (!bInitializeModeFields(ppdev, (GDIINFO*) pdevcaps, pdi, pdm))
    {
        DISPDBG((0, "DrvEnablePDEV - Failed bInitializeModeFields"));
        goto ReturnFailure1;
    }

    // Initialize palette information.

    if (!bInitializePalette(ppdev, pdi))
    {
        DISPDBG((0, "DrvEnablePDEV - Failed bInitializePalette"));
        goto ReturnFailure1;
    }

    DISPDBG((1, "DrvEnablePDEV - ppdev = %8lXh, Mode : %4ld x %4ld x %2ld",
             ppdev, ppdev->cxScreen, ppdev->cyScreen, ppdev->cjPelSize * 8));

#ifdef TVOUT_SUPPORTED
    EdgeInit(ppdev);
#endif //def TVOUT_SUPPORTED

    return((DHPDEV) ppdev);

ReturnFailure1:
    DrvDisablePDEV((DHPDEV) ppdev);

ReturnFailure0:
    DISPDBG((0, "Failed DrvEnablePDEV"));

    return(0);
}

/******************************Public*Routine******************************\
* DrvDisablePDEV
*
* Release the resources allocated in DrvEnablePDEV.  If a surface has been
* enabled DrvDisableSurface will have already been called.
*
* Note that this function will be called when previewing modes in the
* Display Applet, but not at system shutdown.  If you need to reset the
* hardware at shutdown, you can do it in the miniport by providing a
* 'HwResetHw' entry point in the VIDEO_HW_INITIALIZATION_DATA structure.
*
* Note: In an error, we may call this before DrvEnablePDEV is done.
*
\**************************************************************************/

VOID DrvDisablePDEV(
DHPDEV  dhpdev)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) dhpdev;
    H3PRINTF((ppdev, "DrvDisablePDEV\r\n"));

    DISPDBG((1, "DrvDisablePDEV - ppdev = %8lXh", ppdev));

    vUninitializePalette(ppdev);
    ENGFREEMEM(ppdev);
}

/******************************Public*Routine******************************\
* VOID DrvCompletePDEV
*
* Store the HPDEV, the engines handle for this PDEV, in the DHPDEV.
*
\**************************************************************************/

VOID DrvCompletePDEV(
DHPDEV dhpdev,
HDEV   hdev)
{
    ((PDEV*) dhpdev)->hdevEng = hdev;
}


/******************************Public*Routine******************************\
* HSURF DrvEnableSurface
*
* Creates the drawing surface, initializes the hardware, and initializes
* driver components.  This function is called after DrvEnablePDEV, and
* performs the final device initialization.
*
\**************************************************************************/

HSURF DrvEnableSurface(
DHPDEV dhpdev)
{
    PDEV*   ppdev;
    HSURF   hsurf;
    SIZEL   sizl;
    DSURF*  pdsurf;
    VOID*   pvTmpBuffer;


    ppdev = (PDEV*) dhpdev;
#if ENABLE_LOG_FILE
    /* retro3dfx: ppdev (and ppdev->hDriver) are valid here — stash it so V5DLog
       and other ppdev-less loggers can reach the file sink. */
    g_retroLogPpdev = ppdev;
    /* retro3dfx UNCONDITIONAL positive control: fire the WRITE_LOG_FILE IOCTL
       directly from here (no gate, no buffering) so the miniport's IOCTL counter
       increments iff the display->miniport IOCTL channel works at all. */
    {
        char probe[] = "retro3dfx DrvEnableSurface probe\r\n";
        ULONG probeOut = 0;
        DWORD nb;
        EngDeviceIoControl(ppdev->hDriver, IOCTL_3DFX_WRITE_LOG_FILE,
                           probe, sizeof(probe) - 1, &probeOut, sizeof(probeOut), &nb);
    }
#endif
    H3PRINTF((ppdev, "DrvEnableSurface\r\n"));
    DISPDBG((1, "DrvEnableSurface - ppdev = %8lXh", ppdev));

    V5DLog("DrvEnableSurface ENTER ppdev=%08lXh cxScreen=%ld cyScreen=%ld bpp=%ld (GDI creating desktop surface)\n",
           (unsigned long)ppdev, (long)ppdev->cxScreen, (long)ppdev->cyScreen,
           (long)ppdev->cBitsPerPel);

	glideState[ 0 ].glideGDIFlags &= ~GLDATA_GDIFLAGS_HWC_EXCLUSIVE;
	hwcSetContextDWORD();

    /////////////////////////////////////////////////////////////////////
    // First enable all the subcomponents.
    //
    // Note that the order in which these 'Enable' functions are called
    // may be significant in low off-screen memory conditions, because
    // the off-screen heap manager may fail some of the later
    // allocations...

    if (!bEnableHardware(ppdev))
        goto ReturnFailure;

    if (!bEnableOffscreenHeap(ppdev))
        goto ReturnFailure;

    if (!bEnablePointer(ppdev))
        goto ReturnFailure;

    if (!bEnableText(ppdev))
        goto ReturnFailure;

#ifdef PATTERN_JUNK
    if (!bEnableBrushCache(ppdev))
        goto ReturnFailure;
#endif

    if (!bEnablePalette(ppdev))
        goto ReturnFailure;

#ifdef USE_DDRAW_CODE
    if (!bEnableDirectDraw(ppdev))
        goto ReturnFailure;
#endif

#if ENABLE_RECONFIG_VIDMEM
    reconfigureVideoMemory(ppdev);
#endif

    /////////////////////////////////////////////////////////////////////
    // Now create our private surface structure.
    //
    // Whenever we get a call to draw directly to the screen, we'll get
    // passed a pointer to a SURFOBJ whose 'dhpdev' field will point
    // to our PDEV structure, and whose 'dhsurf' field will point to the
    // following DSURF structure.
    //
    // Every device bitmap we create in DrvCreateDeviceBitmap will also
    // have its own unique DSURF structure allocated (but will share the
    // same PDEV).  To make our code more polymorphic for handling drawing
    // to either the screen or an off-screen bitmap, we have the same
    // structure for both.

    pdsurf = ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(DSURF), ALLOC_TAG, &ppdev->pdsurfScreen);
    if (pdsurf == NULL)
    {
        DISPDBG((0, "DrvEnableSurface - Failed pdsurf EngAllocMem"));
        goto ReturnFailure;
    }

    ppdev->pdsurfScreen = pdsurf;           // Remember it for clean-up
#if USE_NT5_DDMEMMGR
#if !ENABLE_RECONFIG_VIDMEM
    DISPDBG((1, "Desktop gets %lXh x %lXh surface at (%lXh, %lXh)",
             ppdev->cxScreen, ppdev->cyScreen, 0 , 0));
#endif

    pdsurf->dt       = DT_SCREEN;
#if !ENABLE_RECONFIG_VIDMEM
    pdsurf->x        = 0;
    pdsurf->y        = 0;
#endif
    pdsurf->cx       = ppdev->cxScreen;
    pdsurf->cy       = ppdev->cyScreen;
    pdsurf->fpVidMem = ppdev->ulScreenOffset;
#else
    pdsurf->poh      = ppdev->pohScreen;     // The screen is a surface, too
    pdsurf->dt       = DT_SCREEN;            // Not to be confused with a DIB
    pdsurf->sizl.cx  = ppdev->cxScreen;
    pdsurf->sizl.cy  = ppdev->cyScreen;
#endif
    pdsurf->ppdev    = ppdev;
    #if ENABLE_LINEAR_DFBS
        // NVH -  Store the surface pitch.
        pdsurf->lDelta = ppdev->lDelta;
    #endif

    /////////////////////////////////////////////////////////////////////
    // Next, have GDI create the actual SURFOBJ.
    //
    // Our drawing surface is going to be 'device-managed', meaning that
    // GDI cannot draw on the framebuffer bits directly, and as such we
    // create the surface via EngCreateDeviceSurface.  By doing this, we ensure
    // that GDI will only ever access the bitmaps bits via the Drv calls
    // that we've HOOKed.
    //
    // If we could map the entire framebuffer linearly into main memory
    // (i.e., we didn't have to go through a 64k aperture), it would be
    // beneficial to create the surface via EngCreateBitmap, giving GDI a
    // pointer to the framebuffer bits.  When we pass a call on to GDI
    // where it can't directly read/write to the surface bits because the
    // surface is device managed, it has to create a temporary bitmap and
    // call our DrvCopyBits routine to get/set a copy of the affected bits.
    // Fer example, the OpenGL component prefers to be able to write on the
    // framebuffer bits directly.

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

#if (_WIN32_WINNT >= 0x0500)
    // Create the primary surface.  This defaults to a 'device-managed'
    // surface, but EngModifySurface can change that.
#endif

    hsurf = EngCreateDeviceSurface((DHSURF) pdsurf, sizl, ppdev->iBitmapFormat);
    if (hsurf == 0)
    {
        DISPDBG((0, "DrvEnableSurface - Failed EngCreateDeviceSurface"));
        goto ReturnFailure;
    }

#if (_WIN32_WINNT >= 0x0500)
    // On all cards where we linearly map the frame buffer, create our
    // drawing surface as a GDI-managed surface, meaning that we give
    // GDI a pointer to the framebuffer and GDI can draw on the bits
    // directly.  This will allow us good performance with drawing such
    // as GradientFills, even though our hardware can't accelerate the
    // drawing and so we don't hook DrvGradientFill.  This way GDI can
    // do write-combined writes directly to the framebuffer and still be
    // very fast.
    //
    // Note that this requires that we hook DrvSynchronize and
    // set HOOK_SYNCHRONIZE.

    // Note that this call is new to NT5, and takes the place of
    // EngAssociateSurface.

    if (!EngModifySurface(hsurf,
                          ppdev->hdevEng,
                          ppdev->flHooks | HOOK_SYNCHRONIZE,
                          MS_NOTSYSTEMMEMORY,    // It's in video memory
                          (DHSURF) pdsurf,
                          ppdev->pjScreen,
                          ppdev->lDelta,
                          NULL))
    {
        DISPDBG((0, "DrvEnableSurface - Failed EngModifySurface"));
        goto ReturnFailure;
    }
#endif

    ppdev->hsurfScreen = hsurf;             // Remember it for clean-up
    ppdev->bEnabled = TRUE;                 // We'll soon be in graphics mode

#if (_WIN32_WINNT == 0x0400)
    /////////////////////////////////////////////////////////////////////
    // Now associate the surface and the PDEV.
    //
    // We have to associate the surface we just created with our physical
    // device so that GDI can get information related to the PDEV when
    // it's drawing to the surface (such as, for example, the length of
    // styles on the device when simulating styled lines).
    //

    if (!EngAssociateSurface(hsurf, ppdev->hdevEng, ppdev->flHooks))
    {
        DISPDBG((0, "DrvEnableSurface - Failed EngAssociateSurface"));
        goto ReturnFailure;
    }
#endif

    // Create our generic temporary buffer, which may be used by any
    // component.

    pvTmpBuffer = ENGALLOCMEM(0, TMP_BUFFER_SIZE, ALLOC_TAG, &ppdev->pvTmpBuffer);
    if (pvTmpBuffer == NULL)
    {
        DISPDBG((0, "DrvEnableSurface - Failed VirtualAlloc"));
        goto ReturnFailure;
    }

    ppdev->pvTmpBuffer = pvTmpBuffer;

    DISPDBG((5, "Passed DrvEnableSurface"));

    return(hsurf);

ReturnFailure:
    DrvDisableSurface((DHPDEV) ppdev);

    DISPDBG((0, "Failed DrvEnableSurface"));

    return(0);
}

/******************************Public*Routine******************************\
* VOID DrvDisableSurface
*
* Free resources allocated by DrvEnableSurface.  Release the surface.
*
* Note that this function will be called when previewing modes in the
* Display Applet, but not at system shutdown.  If you need to reset the
* hardware at shutdown, you can do it in the miniport by providing a
* 'HwResetHw' entry point in the VIDEO_HW_INITIALIZATION_DATA structure.
*
* Note: In an error case, we may call this before DrvEnableSurface is
*       completely done.
*
\**************************************************************************/

VOID DrvDisableSurface(
DHPDEV dhpdev)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) dhpdev;
    H3PRINTF((ppdev, "DrvDisableSurface\r\n"));
    DISPDBG((1, "DrvDisableSurface - ppdev = %8lXh", ppdev));

    V5DLog("DrvDisableSurface ENTER ppdev=%08lXh (GDI tearing down desktop surface; disabling hw/pointer/heap)\n",
           (unsigned long)ppdev);

    // Note: In an error case, some of the following relies on the
    //       fact that the PDEV is zero-initialized, so fields like
    //       'hsurfScreen' will be zero unless the surface has been
    //       sucessfully initialized, and makes the assumption that
    //       EngDeleteSurface can take '0' as a parameter.

    SetGlideStateFlags(ppdev, GLDATA_GDIFLAGS_PPDEV_DESTROYED);

#ifdef USE_DDRAW_CODE
    vDisableDirectDraw(ppdev);
#endif
    vDisablePalette(ppdev);
#ifdef PATTERN_JUNK
    vDisableBrushCache(ppdev);
#endif
    vDisableText(ppdev);
    vDisablePointer(ppdev);
    vDisableOffscreenHeap(ppdev);
    vDisableHardware(ppdev);

    ENGFREEMEM(ppdev->pvTmpBuffer);
    EngDeleteSurface(ppdev->hsurfScreen);
    ENGFREEMEM(ppdev->pdsurfScreen);
}

/******************************Public*Routine******************************\
* VOID DrvAssertMode
*
* This asks the device to reset itself to the mode of the pdev passed in.
*
\**************************************************************************/

BOOL DrvAssertMode(
DHPDEV  dhpdev,
BOOL    bEnable)
{
    PDEV* ppdev;

    ppdev = (PDEV*) dhpdev;
    H3PRINTF((ppdev, "DrvAssertMode\r\n"));

    V5DLog("DrvAssertMode ENTER ppdev=%08lXh bEnable=%ld mode=%ldx%ldx%ld (bEnable=0 -> release hw to full-screen; bEnable=1 -> reprogram hw back to GDI desktop mode)\n",
           (unsigned long)ppdev, (long)bEnable,
           (long)ppdev->cxScreen, (long)ppdev->cyScreen, (long)ppdev->cBitsPerPel);

    if (!bEnable)
    {
        DISPDBG((1, "DrvAssertMode(disable) - ppdev = %8lXh", ppdev));
        V5DLog("DrvAssertMode(DISABLE) ppdev=%08lXh: relinquishing hw (full-screen switch)\n",
               (unsigned long)ppdev);

        SetGlideStateFlags(ppdev, GLDATA_GDIFLAGS_PPDEV_DISABLED);

        //////////////////////////////////////////////////////////////
        // Disable - Switch to full-screen mode

#ifdef USE_DDRAW_CODE
        vAssertModeDirectDraw(ppdev, FALSE);
#endif
        vAssertModePalette(ppdev, FALSE);

#ifdef PATTERN_JUNK
        vAssertModeBrushCache(ppdev, FALSE);
#endif

        vAssertModeText(ppdev, FALSE);

        vAssertModePointer(ppdev, FALSE);

        if (bAssertModeOffscreenHeap(ppdev, FALSE))
        {
            if (bAssertModeHardware(ppdev, FALSE))
            {
                ppdev->bEnabled = FALSE;

                V5DLog("DrvAssertMode(DISABLE) ppdev=%08lXh: bAssertModeHardware(FALSE) OK -> hw released, bEnabled=0, returning TRUE\n",
                       (unsigned long)ppdev);
                return(TRUE);
            }
            V5DLog("DrvAssertMode(DISABLE) ppdev=%08lXh: bAssertModeHardware(FALSE) FAILED -> undoing subcomponent disables\n",
                   (unsigned long)ppdev);

            //////////////////////////////////////////////////////////
            // We failed to switch to full-screen.  So undo everything:

            bAssertModeOffscreenHeap(ppdev, TRUE);  // We don't need to check
        }                                           //   return code with TRUE

        vAssertModePointer(ppdev, TRUE);

        vAssertModeText(ppdev, TRUE);

#ifdef PATTERN_JUNK
        vAssertModeBrushCache(ppdev, TRUE);
#endif

        vAssertModePalette(ppdev, TRUE);

#ifdef USE_DDRAW_CODE
        vAssertModeDirectDraw(ppdev, TRUE);
#endif
    }
    else
    {
        DISPDBG((1, "DrvAssertMode(enable) - ppdev = %8lXh", ppdev));
        V5DLog("DrvAssertMode(ENABLE) ppdev=%08lXh: reprogramming hw back to GDI desktop mode %ldx%ldx%ld (this is the desktop-restore path after a Glide app exits)\n",
               (unsigned long)ppdev, (long)ppdev->cxScreen, (long)ppdev->cyScreen,
               (long)ppdev->cBitsPerPel);

        //////////////////////////////////////////////////////////////
        // Enable - Switch back to graphics mode

        // We have to enable every subcomponent in the reverse order
        // in which it was disabled:

        if (bAssertModeHardware(ppdev, TRUE))
        {
            bAssertModeOffscreenHeap(ppdev, TRUE);  // We don't need to check
                                                    //   return code with TRUE
            vAssertModePointer(ppdev, TRUE);

            vAssertModeText(ppdev, TRUE);

#ifdef PATTERN_JUNK
            vAssertModeBrushCache(ppdev, TRUE);
#endif

            vAssertModePalette(ppdev, TRUE);

#ifdef USE_DDRAW_CODE
            vAssertModeDirectDraw(ppdev, TRUE);
#endif

#if ENABLE_RECONFIG_VIDMEM
            reconfigureVideoMemory(ppdev);
#endif

            ClearGlideStateFlags(ppdev, GLDATA_GDIFLAGS_PPDEV_DISABLED);

            ppdev->bEnabled = TRUE;

            V5DLog("DrvAssertMode(ENABLE) ppdev=%08lXh: bAssertModeHardware(TRUE) OK -> desktop mode reprogrammed, bEnabled=1, returning TRUE\n",
                   (unsigned long)ppdev);
            return(TRUE);
        }
        V5DLog("DrvAssertMode(ENABLE) ppdev=%08lXh: bAssertModeHardware(TRUE) FAILED -> desktop mode NOT restored, returning FALSE (garble risk!)\n",
               (unsigned long)ppdev);
    }

    V5DLog("DrvAssertMode EXIT ppdev=%08lXh returning FALSE (mode transition did not complete)\n",
           (unsigned long)ppdev);
    return(FALSE);
}

/******************************Public*Routine******************************\
* ULONG DrvGetModes
*
* Returns the list of available modes for the device.
*
\**************************************************************************/

ULONG DrvGetModes(
HANDLE      hDriver,
ULONG       cjSize,
DEVMODEW*   pdm)
{
    DWORD cModes;
    DWORD cbOutputSize;
    PVIDEO_MODE_INFORMATION pVideoModeInformation;
    PVIDEO_MODE_INFORMATION pVideoTemp;
    DWORD cOutputModes = cjSize / (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);
    DWORD cbModeSize;

    cModes = getAvailableModes(hDriver,
                            (PVIDEO_MODE_INFORMATION *) &pVideoModeInformation,
                            &cbModeSize);
    if (cModes == 0)
    {
        DISPDBG((0, "DrvGetModes failed to get mode information"));
        return(0);
    }

    if (pdm == NULL)
    {
        cbOutputSize = cModes * (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);
    }
    else
    {
        //
        // Now copy the information for the supported modes back into the
        // output buffer
        //

        cbOutputSize = 0;

        pVideoTemp = pVideoModeInformation;

        do
        {
            if (pVideoTemp->Length != 0)
            {
                if (cOutputModes == 0)
                {
                    break;
                }

                //
                // Zero the entire structure to start off with.
                //

                memset(pdm, 0, sizeof(DEVMODEW));

                //
                // Set the name of the device to the name of the DLL.
                //

                memcpy(pdm->dmDeviceName, DLL_NAME, sizeof(DLL_NAME));

                pdm->dmSpecVersion      = DM_SPECVERSION;
                pdm->dmDriverVersion    = DM_SPECVERSION;
                pdm->dmSize             = sizeof(DEVMODEW);
                pdm->dmDriverExtra      = DRIVER_EXTRA_SIZE;

                pdm->dmBitsPerPel       = pVideoTemp->NumberOfPlanes *
                                          pVideoTemp->BitsPerPlane;
                pdm->dmPelsWidth        = pVideoTemp->VisScreenWidth;
                pdm->dmPelsHeight       = pVideoTemp->VisScreenHeight;
                pdm->dmDisplayFrequency = pVideoTemp->Frequency;
                pdm->dmDisplayFlags     = 0;

                pdm->dmFields           = DM_BITSPERPEL       |
                                          DM_PELSWIDTH        |
                                          DM_PELSHEIGHT       |
                                          DM_DISPLAYFREQUENCY |
                                          DM_DISPLAYFLAGS     ;

                //
                // Go to the next DEVMODE entry in the buffer.
                //

                cOutputModes--;

                pdm = (LPDEVMODEW) ( ((ULONG)pdm) + sizeof(DEVMODEW) +
                                                   DRIVER_EXTRA_SIZE);

                cbOutputSize += (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);

            }

            pVideoTemp = (PVIDEO_MODE_INFORMATION)
                (((PUCHAR)pVideoTemp) + cbModeSize);


        } while (--cModes);
    }

    ENGFREEMEM(pVideoModeInformation);

    return(cbOutputSize);
}

/******************************Public*Routine******************************\
* BOOL bAssertModeHardware
*
* Sets the appropriate hardware state for graphics mode or full-screen.
*
\**************************************************************************/

#if (_WIN32_WINNT >= 0x0500)
#define CMDFIFO_START_OFFSET  0
#else
#define CMDFIFO_START_OFFSET  0x1000
#endif

BOOL bAssertModeHardware(
PDEV* ppdev,
BOOL  bEnable)
{
    BYTE*                   pjIoBase;
    BYTE*                   pjMmBase;
    DWORD                   ReturnedDataLength;
    ULONG                   ulReturn;
    VIDEO_MODE_INFORMATION  VideoModeInfo;
    LONG                    cjEndOfFrameBuffer;
    LONG                    cjPointerOffset;
    LONG                    cjFifoOffset;
    LONG                    lDelta;
#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
    BOOL                    doAGPFifo;
    H3_AGP_INFO             h3AgpInfo;
#endif

    pjIoBase = ppdev->pjIoBase;
    pjMmBase = ppdev->pjMmBase;

    if (bEnable)
    {
        // Call the miniport via an IOCTL to set the graphics mode.

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_SET_CURRENT_MODE,
                               &ppdev->ulMode,  // input buffer
                               sizeof(DWORD),
                               NULL,
                               0,
                               &ReturnedDataLength))
        {
            DISPDBG((0, "bAssertModeHardware - Failed VIDEO_SET_CURRENT_MODE"));
            goto ReturnFalse;
        }

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_QUERY_CURRENT_MODE,
                               NULL,
                               0,
                               &VideoModeInfo,
                               sizeof(VideoModeInfo),
                               &ReturnedDataLength))
        {
            DISPDBG((0, "bAssertModeHardware - failed VIDEO_QUERY_CURRENT_MODE"));
            goto ReturnFalse;
        }

        #if DEBUG_HEAP
            VideoModeInfo.VideoMemoryBitmapWidth  = VideoModeInfo.VisScreenWidth;
            VideoModeInfo.VideoMemoryBitmapHeight = VideoModeInfo.VisScreenHeight;
        #endif

        // The following variables are determined only after the initial
        // modeset:

        ppdev->lDelta   = VideoModeInfo.ScreenStride;
        ppdev->flCaps   = VideoModeInfo.DriverSpecificAttributeFlags;
        ppdev->cxMemory = VideoModeInfo.VideoMemoryBitmapWidth;
        ppdev->cyMemory = VideoModeInfo.VideoMemoryBitmapHeight;
#if ENABLE_RECONFIG_VIDMEM
        DISPDBG((1, "from miniport: cxMemory x cyMemory = %lXh x %lXh => %lXh bytes",
                 ppdev->cxMemory,ppdev->cyMemory,ppdev->cyMemory*ppdev->cxMemory*ppdev->cjPelSize));
#else
        DISPDBG((1, "from miniport: cxMemory x cyMemory = %ld x %ld",
                 ppdev->cxMemory, ppdev->cyMemory));
#endif

        lDelta = ppdev->lDelta;

        SetInvariantReg(ppdev);    // Initialize HW registers.

#ifdef H3_FIFO
#if (_WIN32_WINNT >= 0x0500) && defined (AGP_CMDFIFO)
    doAGPFifo = (ppdev->flCaps & CAPS_AGP_FIFO) ? TRUE : FALSE;
    ppdev->doAGPFifo = doAGPFifo;

    if( doAGPFifo == TRUE )
    {
        ppdev->ulScreenOffset = CMDFIFO_START_OFFSET;

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_3DFX_GET_AGP_FIFO_INFO,
                               NULL,
                               0,
                               &h3AgpInfo,
                               sizeof(h3AgpInfo),
                               &ReturnedDataLength))
        {
            DISPDBG((0, "bAssertModeHardware - failed GET_AGP_FIFO_INFO"));
            goto ReturnFalse;
        }

        ppdev->fifoData.physBaseL = h3AgpInfo.physAddr.LowPart;
        ppdev->fifoData.physBaseH = (ULONG) h3AgpInfo.physAddr.HighPart;

        ppdev->fifoData.fifoOffset = 0;
        ppdev->fifoData.fifoStart = h3AgpInfo.virtualAddr;
        ppdev->fifoData.fifoPtr = (ULONG *) ppdev->fifoData.fifoStart;
        ppdev->fifoData.fifoLastRead = (ULONG) ppdev->fifoData.physBaseL;
        ppdev->fifoData.fifoSize = h3AgpInfo.agpFifoSizeInB;
        ppdev->fifoData.fifoEnd = ppdev->fifoData.fifoStart + ppdev->fifoData.fifoSize;
        ppdev->fifoData.ulCmdFifoBump = 0; // igx-nvh - 01.07.00


        /* Adjust room values.
         * RoomToEnd needs enough room for the jmp packet since we
         * never allow the hw to auto-wrap. RoomToRead needs to be
         * adjusted so that we never acutally write onto the read ptr.
         *
         * fifoRoom is generally the min of roomToEnd and roomToRead,
         * but we 'know' here that roomToRead < roomToEnd.
         */
        ppdev->fifoData.roomToEnd     = ppdev->fifoData.fifoSize - H3_FIFO_END_ADJUST;
        ppdev->fifoData.fifoRoom      =
        ppdev->fifoData.roomToReadPtr = ppdev->fifoData.roomToEnd;

        // Pre-compute the packet to return us back to the start.

        ppdev->fifoData.fifoJmpHdr = (SSTCP_PKT0_JMP_AGP  | ((ppdev->fifoData.physBaseL << (SSTCP_PKT0_ADDR_SHIFT - 2)) & SSTCP_PKT0_ADDR));
        ppdev->fifoData.fifoJmpHdr2 = (ppdev->fifoData.physBaseL >> 25);

        CmdFifo0Init( ppdev,
                      ppdev->fifoData.fifoOffset,
                      ppdev->fifoData.fifoSize,
                      0,
                      doAGPFifo );

        ppdev->fifoData.fifoSize -= H3_FIFO_END_ADJUST;
    }
    else
# endif // AGP_CMDFIFO
    {
#if USE_D3D_CODE && (_WIN32_WINNT >= 0x0500)
        ULONG   fifoSize;
#endif

        // Put the fifo at the beginning of memory.
        cjFifoOffset = CMDFIFO_START_OFFSET;
        ppdev->fifoData.fifoOffset = cjFifoOffset;
#if CSIM
        ppdev->fifoData.fifoStart = (BYTE *) CP_BEGIN(cjFifoOffset);
#else
        ppdev->fifoData.fifoStart = ppdev->pjScreenBase + cjFifoOffset;
#endif
        /* Set initial fifo state. hw read and sw write pointers at
         * start of the fifo.
         */
        ppdev->fifoData.fifoPtr = (ULONG *) ppdev->fifoData.fifoStart;
        ppdev->fifoData.fifoLastRead = (ULONG) ppdev->fifoData.fifoOffset;

#if USE_D3D_CODE && (_WIN32_WINNT >= 0x0500)
        if ((2 == ppdev->cjPelSize) || ((IS_NAPALM) && (4 == ppdev->cjPelSize)))
        {
#if 1
          if (GetRegDWORD(ppdev, "CmdfifoSize", &fifoSize))
          {
            // round up to nearest 4kB size
            fifoSize = ((fifoSize + (4*1024 - 1)) / (4*1024)) * (4*1024);

            // limit minimum fifoSize to 64kB and maximum fifoSize to 1MB
            if (HW_CMDFIFO_TOTAL_SIZE > fifoSize)
            {
              fifoSize = HW_CMDFIFO_TOTAL_SIZE;
            }
            else if (1*1024*1024 < fifoSize)
            {
              fifoSize = 1*1024*1024;
            }
          }
          else
#endif
          {
            // default to 512kB cmdfifo
            fifoSize = 512*1024;
          }

          ppdev->fifoData.fifoSize = fifoSize;
          DISPDBG((0, "  cmdfifoSize = %8lXh", ppdev->fifoData.fifoSize));
        }
        else
#endif
          ppdev->fifoData.fifoSize = HW_CMDFIFO_TOTAL_SIZE;
        ppdev->fifoData.fifoEnd = ppdev->fifoData.fifoStart + ppdev->fifoData.fifoSize;

        /* Adjust room values.
         * RoomToEnd needs enough room for the jmp packet since we
         * never allow the hw to auto-wrap. RoomToRead needs to be
         * adjusted so that we never acutally write onto the read ptr.
         *
         * fifoRoom is generally the min of roomToEnd and roomToRead,
         * but we 'know' here that roomToRead < roomToEnd.
         */
        ppdev->fifoData.roomToEnd     = ppdev->fifoData.fifoSize - H3_FIFO_END_ADJUST;
        ppdev->fifoData.fifoRoom      =
        ppdev->fifoData.roomToReadPtr = ppdev->fifoData.roomToEnd;

        // Pre-compute the packet to return us back to the start.

        ppdev->fifoData.fifoJmpHdr = (SSTCP_PKT0_JMP_LOCAL  | (CMDFIFO_START_OFFSET << (SSTCP_PKT0_ADDR_SHIFT - 2)));

        // Adjust display start in PDEV and video unit to after fifo.
#if USE_D3D_CODE && (_WIN32_WINNT >= 0x0500)
        if ((2 == ppdev->cjPelSize) || ((IS_NAPALM) && (4 == ppdev->cjPelSize)))
          ppdev->ulScreenOffset = fifoSize + CMDFIFO_START_OFFSET;
        else
#endif
          ppdev->ulScreenOffset = HW_CMDFIFO_TOTAL_SIZE + CMDFIFO_START_OFFSET;
        DISPDBG((1, "adjust ulScreenOffset for cmdfifo: %ld", ppdev->ulScreenOffset));

        // Enable command fifo here.

        CmdFifo0Init(ppdev,
                     ppdev->fifoData.fifoOffset,
                     ppdev->fifoData.fifoSize,
                     0,
                     0);
// START Alt-Tab changes
        ppdev->ulCmdFifoDisabled = FALSE;
// END Alt-Tab changes

        ppdev->fifoData.fifoSize -= H3_FIFO_END_ADJUST;
    }
#else
        ppdev->ulScreenOffset = CMDFIFO_START_OFFSET;
#endif	// H3_FIFO

#ifndef CSIM
        // If we're using the hardware pointer, reserve 1k of
        // the frame buffer to store the pointer shape:

        if (!(ppdev->flCaps & CAPS_SW_POINTER))
        {
            // We'll reserve the area of off-screen memory right after
            // the fifo for the hardware pointer shape.  We'll store
            // the shape on a 1K multiple.

            // Figure out the coordinate where the pointer shape starts:

            ppdev->cjPointerOffset = ppdev->ulScreenOffset;
            ppdev->ulScreenOffset += HW_POINTER_TOTAL_SIZE;
            DISPDBG((1, "adjust ulScreenOffset for hw cursor: %ld", ppdev->ulScreenOffset));
        }
#endif

        SET2( ((SstIORegs *)(ppdev->pjBase))->vidDesktopStartAddr, ppdev->ulScreenOffset );
        H3PRINTF((ppdev, "  vidDesktopStartAddr = %08lX\r\n", ppdev->ulScreenOffset));
        SET_DW( ppdev->pjH3Base, srcBaseAddr, ppdev->ulScreenOffset );
        SET_DW( ppdev->pjH3Base, dstBaseAddr, ppdev->ulScreenOffset );

#if defined(H3_FIFO) || !defined(CSIM)
        // Subtract fifo yMemory size from heap.
        ppdev->cyMemory -= ( ppdev->ulScreenOffset + lDelta - 1 ) / lDelta;
        DISPDBG((1, "adjust cyMemory: cyMemory=%ld, ulScreenOffset=%ld, lDelta=%ld",
                 ppdev->cyMemory, ppdev->ulScreenOffset, ppdev->lDelta));
        ppdev->pjScreen = ppdev->pjScreenBase + ppdev->ulScreenOffset;
#endif

        // Banshee hardware clip registers only use 12 bits of precision
        // so we have to limit our cyMemory value or we will be hosed
        // when we set clipping.

        if( ppdev->cyMemory > 0xfff )
        {
          // gdi get scanlines 0 thru 0xffe
#if USE_DDRAW_CODE
          // ddraw can use the extra scanlines 0xfff thru ppdev->cyMemory
          ppdev->cyDDMemoryExtra = ppdev->cyMemory - 0xfff;
#endif
          ppdev->cyMemory = 0xfff;

#if USE_DDRAW_CODE
          DISPDBG((1, "DirectDraw gets 2nd heap: %li x %li surface at (%li, %li)",
                  ppdev->cxMemory, ppdev->cyDDMemoryExtra, 0, ppdev->cyMemory));
#endif
        }
#if USE_DDRAW_CODE
        else
        {
          DISPDBG((1, "DirectDraw gets only one heap"));
          ppdev->cyDDMemoryExtra = 0;
        }
#endif

        // Do some parameter checking on the values that the miniport
        // returned to us:

        ASSERTDD(ppdev->cxMemory >= ppdev->cxScreen, "Invalid cxMemory");
        ASSERTDD(ppdev->cyMemory >= ppdev->cyScreen, "Invalid cyMemory");

        // First thing we do is set the default registers and clear the desktop.

        vResetClipping(ppdev);
        vClearDesktopSurface(ppdev);
	    hwcSetContextDWORD();
    }
    else
    {
		hwcSetContextDWORD();
#ifdef H3_FIFO
        // Either we turn off command fifo here or miniport does it.
        H3_GP_WAIT(ppdev, ppdev->pjBase);
        CmdFifo0Disable(ppdev);
// START Alt-Tab changes
		ppdev->ulCmdFifoDisabled = TRUE;
// END Alt-Tab changes
#endif

        // Call the kernel driver to reset the device to a known state.
        // NTVDM will take things from there:

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_RESET_DEVICE,
                               NULL,
                               0,
                               NULL,
                               0,
                               &ulReturn))
        {
            DISPDBG((0, "bAssertModeHardware - Failed reset IOCTL"));
            goto ReturnFalse;
        }
    }

    DISPDBG((5, "Passed bAssertModeHardware"));

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bAssertModeHardware"));

    return(FALSE);
}

/******************************Public*Routine******************************\
* BOOL bEnableHardware
*
* Puts the hardware in the requested mode and initializes it.
*
* Note: Should be called before any access is done to the hardware from
*       the display driver.
*
\**************************************************************************/

BOOL bEnableHardware(
PDEV*   ppdev)
{
    BYTE*                       pjBase;
#if REDUCED_MEMORY_MAPPINGS
#if (_MAP_FRAMEBUFFER_ONCE == 1)
    VIDEO_PUBLIC_ACCESS_RANGES  VideoAccessRange[6];
#else
    VIDEO_PUBLIC_ACCESS_RANGES  VideoAccessRange[7];
#endif
#else
#if (_MAP_FRAMEBUFFER_ONCE == 1)
    VIDEO_PUBLIC_ACCESS_RANGES  VideoAccessRange[2];
#else
    VIDEO_PUBLIC_ACCESS_RANGES  VideoAccessRange[3];
#endif
#endif
    VIDEO_MEMORY                VideoMemory;
    VIDEO_MEMORY_INFORMATION    VideoMemoryInfo;
    DWORD                       ReturnedDataLength;
    UCHAR*                      pj;
    USHORT*                     pw;
    ULONG*                      pd;
    ULONG                       i;

    ppdev->csCrtc = EngCreateSemaphore();
    if (ppdev->csCrtc == 0)
    {
        DISPDBG((0, "bEnableHardware - Error creating CRTC semaphore"));
        goto ReturnFalse;
    }

    // Map io ports into virtual memory:

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES,
                           NULL,                      // input buffer
                           0,
                           &VideoAccessRange,         // output buffer
                           sizeof(VideoAccessRange),
                           &ReturnedDataLength))
    {
        DISPDBG((0, "bEnableHardware - Initialization error mapping IO port base"));
        goto ReturnFalse;
    }

#if REDUCED_MEMORY_MAPPINGS
    ppdev->pjBase       = (BYTE *)VideoAccessRange[0].VirtualAddress;
    ppdev->pjIoBase     = (BYTE *)VideoAccessRange[1].VirtualAddress;
    ppdev->pjCmdAgpBase = (BYTE *)VideoAccessRange[2].VirtualAddress;
    ppdev->pjMmBase     =
    ppdev->pjH3Base     = (BYTE *)VideoAccessRange[3].VirtualAddress;
    ppdev->p3DBase      = (SstRegs *)VideoAccessRange[4].VirtualAddress;
    ppdev->pjYUVPlanar  = (BYTE *)VideoAccessRange[5].VirtualAddress;

    ppdev->pjFifoBase   = ppdev->pjCmdAgpBase + FIELDOFFSET(SstCRegs, cmdFifo0);

    pjBase = ppdev->pjBase;
#else
#ifdef CSIM
    ppdev->pjBase = (BYTE *) 0x10000000L;
    ppdev->p3DBase = (SstRegs *)(ppdev->pjBase + SST_3D_OFFSET);
    ppdev->CSIM_pjBase = VideoAccessRange[0].VirtualAddress;
#else
    // Miniport passes MemBase0, MemBase1, IoBase

    ppdev->pjBase = (BYTE *) VideoAccessRange[0].VirtualAddress;
#if (_MAP_FRAMEBUFFER_ONCE == 1)
    ppdev->pjIoBase = (BYTE *) VideoAccessRange[1].VirtualAddress;
#else
	ppdev->pjLfbBase = (BYTE *) VideoAccessRange[1].VirtualAddress;
    ppdev->pjIoBase = (BYTE *) VideoAccessRange[2].VirtualAddress;
#endif

    ppdev->pjMmBase = ppdev->pjBase + 0x100000;	// 2D Base
    ppdev->p3DBase = (SstRegs *) (ppdev->pjBase + SST_3D_OFFSET);
#endif

    pjBase = ppdev->pjBase;

    // Set all the register addresses.

    ppdev->pjH3Base     = pjBase + SST_2D_OFFSET;
    ppdev->pjCmdAgpBase = pjBase + SST_CMDAGP_OFFSET;
    ppdev->pjFifoBase   = ppdev->pjCmdAgpBase + FIELDOFFSET(SstCRegs, cmdFifo0);
#endif

    // Get the linear memory address range.

    VideoMemory.RequestedVirtualAddress = NULL;

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_MAP_VIDEO_MEMORY,
                           &VideoMemory,      // input buffer
                           sizeof(VIDEO_MEMORY),
                           &VideoMemoryInfo,  // output buffer
                           sizeof(VideoMemoryInfo),
                           &ReturnedDataLength))
    {
        DISPDBG((0, "bEnableHardware - Error mapping buffer address"));
        goto ReturnFalse;
    }

    // jdw - Miniport may need adjustment for tiled memory & non-zero FB base.

    // Record the Frame Buffer Linear Address.

    // pjScreen will be different from pjScreenBase when using command fifo.
    ppdev->pjScreenBase = (BYTE*) VideoMemoryInfo.FrameBufferBase;
#if (_MAP_FRAMEBUFFER_ONCE == 1)
	ppdev->pjLfbBase= (BYTE *) ppdev->pjScreenBase;
#endif
    ppdev->pjScreen = (BYTE*) VideoMemoryInfo.FrameBufferBase;
    ppdev->cjBank   =         VideoMemoryInfo.FrameBufferLength;

    {
      ULONG MemSizePerChipOverride;
  
  
      if (GetRegDWORD(ppdev, "MemSizePerChipOverride", &MemSizePerChipOverride))
      {
        // only allow sizes less than what's actually on the board
        if (MemSizePerChipOverride < (ULONG)(ppdev->cjBank / (1024 * 1024)))
        {
          if (( 8 == MemSizePerChipOverride) ||
              (16 == MemSizePerChipOverride) ||
              (32 == MemSizePerChipOverride) ||
              (64 == MemSizePerChipOverride))
          {
            DISPDBG((0, "  Overriding default MemSizePerChip with %4ld MB", MemSizePerChipOverride));
            ppdev->cjBank = MemSizePerChipOverride * 1024  * 1024;
          }
        }
      }
    }

    DISPDBG((1, "pjScreen: %lx  pjMmBase: %lx", ppdev->pjScreen, ppdev->pjMmBase));

    // Now we can set the mode, unlock the accelerator, and reset the
    // clipping:

#ifdef CSIM
    if (!bEnableCSIM(ppdev))
        goto ReturnFalse;
#endif

    if (!bAssertModeHardware(ppdev, TRUE))
        goto ReturnFalse;

#if defined(CSIM)
    ((SstIORegs *)ppdev->CSIM_pjBase)->vidDesktopStartAddr = ppdev->ulScreenOffset;
#endif
    // Can do memory-mapped IO:

    ppdev->pfnFillSolid         = vMmFillSolid;
    ppdev->pfnFillPat           = vMmFillPatFast;
    ppdev->pfnXfer1bpp          = vMmXfer1bpp;
    ppdev->pfnXfer4bpp          = vXfer4bpp;
    ppdev->pfnXferNative        = vMmXferNative;
    ppdev->pfnCopyBlt           = vMmCopyBlt;
//        ppdev->pfnFastPatRealize    = vMmFastPatRealize;

    #if DBG
    {
        DISPDBG((0, "Bank: %lx Width: %li Height: %li Stride: %li Flags: %08lx",
                ppdev->cjBank, ppdev->cxMemory, ppdev->cyMemory,
                ppdev->lDelta, ppdev->flCaps));
    }
    #endif

    DISPDBG((5, "Passed bEnableHardware"));

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bEnableHardware"));

    return(FALSE);
}

/******************************Public*Routine******************************\
* VOID vDisableHardware
*
* Undoes anything done in bEnableHardware.
*
* Note: In an error case, we may call this before bEnableHardware is
*       completely done.
*
\**************************************************************************/

VOID vDisableHardware(
PDEV*   ppdev)
{
    DWORD        ReturnedDataLength;
#if REDUCED_MEMORY_MAPPINGS
#if (_MAP_FRAMEBUFFER_ONCE == 1)
    VIDEO_MEMORY VideoMemory[6];
#else
    VIDEO_MEMORY VideoMemory[7];
#endif
#else
#if (_MAP_FRAMEBUFFER_ONCE == 1)
    VIDEO_MEMORY VideoMemory[2];
#else
    VIDEO_MEMORY VideoMemory[3];
#endif
#endif


    VideoMemory[0].RequestedVirtualAddress = ppdev->pjScreenBase;

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_UNMAP_VIDEO_MEMORY,
                           VideoMemory,
                           sizeof(VideoMemory[0]),
                           NULL,
                           0,
                           &ReturnedDataLength))
    {
        DISPDBG((0, "vDisableHardware failed IOCTL_VIDEO_UNMAP_VIDEO"));
    }

    VideoMemory[0].RequestedVirtualAddress = ppdev->pjBase;
#if REDUCED_MEMORY_MAPPINGS
    VideoMemory[1].RequestedVirtualAddress = ppdev->pjIoBase;
    VideoMemory[2].RequestedVirtualAddress = ppdev->pjCmdAgpBase;
    VideoMemory[3].RequestedVirtualAddress = ppdev->pjH3Base;
    VideoMemory[4].RequestedVirtualAddress = (BYTE *)ppdev->p3DBase;
    VideoMemory[5].RequestedVirtualAddress = ppdev->pjYUVPlanar;
#else
#if (_MAP_FRAMEBUFFER_ONCE == 1)
    VideoMemory[1].RequestedVirtualAddress = ppdev->pjLfbBase;
#else
    VideoMemory[1].RequestedVirtualAddress = ppdev->pjLfbBase;
    VideoMemory[2].RequestedVirtualAddress = ppdev->pjIoBase;
#endif
#endif

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES,
                           VideoMemory,
                           sizeof(VideoMemory),
                           NULL,
                           0,
                           &ReturnedDataLength))
    {
        DISPDBG((0, "vDisableHardware failed IOCTL_VIDEO_FREE_PUBLIC_ACCESS"));
    }

    EngDeleteSemaphore(ppdev->csCrtc);
}

/******************************Public*Routine******************************\
* BOOL bGetDeviceInfo
*
* Initializes device-specific information based on data collected from the
* miniport
*
\**************************************************************************/
BOOL bGetDeviceInfo(
PDEV* ppdev)
{
  TDFX_IDENTITY_INFO  DeviceInformation;
  DWORD               ReturnedDataLength;


  // Collect the vendor and device ID
  if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_3DFX_IDENTITY_INFO,
                         NULL,                      // input buffer
                         0,
                         &DeviceInformation,        // output buffer
                         sizeof(DeviceInformation),
                         &ReturnedDataLength))
  {
    DISPDBG((0, "bGetDeviceInfo - Can't get 3Dfx identity information!"));
    return(FALSE);
  }
  DISPDBG((0, "bGetDeviceInfo - found VendorID=%04Xh DeviceID=%04Xh ChipRev=%04lXh",
           DeviceInformation.VendorID, DeviceInformation.DeviceID, DeviceInformation.ChipRevision));


  ppdev->usVendorID = DeviceInformation.VendorID;
#if defined(CSIM)
  {
    ULONG DeviceIDOverride;


    if (GetRegDWORD(ppdev, "DeviceIDOverride", &DeviceIDOverride))
    {
      // if the override deviceID isn't Napalm or V3 then forget it!
      if ((TDFX_NAPALM_ID  == DeviceIDOverride) || (TDFX_AVENGER_ID == DeviceIDOverride))
      {
        ppdev->usDeviceID = (USHORT)DeviceIDOverride;
        DISPDBG((0, "  Overriding default DeviceID with %04lXh", ppdev->usDeviceID));
      }
      else
      {
        // default to indicate we're running on a Napalm
        ppdev->usDeviceID = TDFX_NAPALM_ID;
        DISPDBG((0, "  Defaulting to DeviceID %04lXh", ppdev->usDeviceID));
      }
    }
    else
    {
      // default to indicate we're running on a Napalm
      ppdev->usDeviceID = TDFX_NAPALM_ID;
      DISPDBG((0, "  Defaulting to DeviceID %04lXh", ppdev->usDeviceID));
    }
  }
#else
  ppdev->usDeviceID = DeviceInformation.DeviceID;
#endif
  ppdev->usChipRev  = DeviceInformation.ChipRevision;
  ppdev->cpuType    = DeviceInformation.CpuType;
  ppdev->ulCustomerNumber    = DeviceInformation.CustomerNumber;
  ppdev->usSubSystemID = DeviceInformation.usSubSystemID;

  return(TRUE);
}

/******************************Public*Routine******************************\
* BOOL bInitializeModeFields
*
* Initializes a bunch of fields in the pdev, devcaps (aka gdiinfo), and
* devinfo based on the requested mode.
*
\**************************************************************************/

BOOL bInitializeModeFields(
PDEV*     ppdev,
GDIINFO*  pgdi,
DEVINFO*  pdi,
DEVMODEW* pdm)
{
    ULONG                   cModes;
    PVIDEO_MODE_INFORMATION pVideoBuffer;
    PVIDEO_MODE_INFORMATION pVideoModeSelected;
    PVIDEO_MODE_INFORMATION pVideoTemp;
    BOOL                    bSelectDefault;
    VIDEO_MODE_INFORMATION  VideoModeInformation;
    ULONG                   cbModeSize;

    // Call the miniport to get mode information

    cModes = getAvailableModes(ppdev->hDriver, &pVideoBuffer, &cbModeSize);
    if (cModes == 0)
        goto ReturnFalse;

    // Now see if the requested mode has a match in that table.

    pVideoModeSelected = NULL;
    pVideoTemp = pVideoBuffer;

    if ((pdm->dmPelsWidth        == 0) &&
        (pdm->dmPelsHeight       == 0) &&
        (pdm->dmBitsPerPel       == 0) &&
        (pdm->dmDisplayFrequency == 0))
    {
        DISPDBG((1, "Default mode requested"));
        bSelectDefault = TRUE;
    }
    else
    {
        DISPDBG((1, "Requested mode..."));
        DISPDBG((1, "   Screen width  -- %li", pdm->dmPelsWidth));
        DISPDBG((1, "   Screen height -- %li", pdm->dmPelsHeight));
        DISPDBG((1, "   Bits per pel  -- %li", pdm->dmBitsPerPel));
        DISPDBG((1, "   Frequency     -- %li", pdm->dmDisplayFrequency));

        bSelectDefault = FALSE;
    }

    while (cModes--)
    {
        if (pVideoTemp->Length != 0)
        {
            DISPDBG((8, "   Checking against miniport mode:"));
            DISPDBG((8, "      Screen width  -- %li", pVideoTemp->VisScreenWidth));
            DISPDBG((8, "      Screen height -- %li", pVideoTemp->VisScreenHeight));
            DISPDBG((8, "      Bits per pel  -- %li", pVideoTemp->BitsPerPlane *
                                                      pVideoTemp->NumberOfPlanes));
            DISPDBG((8, "      Frequency     -- %li", pVideoTemp->Frequency));

            if (bSelectDefault ||
                ((pVideoTemp->VisScreenWidth  == pdm->dmPelsWidth) &&
                 (pVideoTemp->VisScreenHeight == pdm->dmPelsHeight) &&
                 (pVideoTemp->BitsPerPlane *
                  pVideoTemp->NumberOfPlanes  == pdm->dmBitsPerPel) &&
                 (pVideoTemp->Frequency       == pdm->dmDisplayFrequency)))
            {
                pVideoModeSelected = pVideoTemp;
                DISPDBG((1, "...Found a mode match!"));
                break;
            }
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + cbModeSize);

    }

    // If no mode has been found, return an error

    if (pVideoModeSelected == NULL)
    {
        DISPDBG((1, "...Couldn't find a mode match!"));
        ENGFREEMEM(pVideoBuffer);
        goto ReturnFalse;
    }

    H3PRINTF((ppdev, "  Setting mode %ldx%ldx%ld @ %ld Hz\r\n",
              pVideoModeSelected->VisScreenWidth,
              pVideoModeSelected->VisScreenHeight,
              pVideoModeSelected->BitsPerPlane * pVideoModeSelected->NumberOfPlanes,
              pVideoModeSelected->Frequency));

    // We have chosen the one we want.  Save it in a stack buffer and
    // get rid of allocated memory before we forget to free it.

    VideoModeInformation = *pVideoModeSelected;
    ENGFREEMEM(pVideoBuffer);

    #if DEBUG_HEAP
        VideoModeInformation.VisScreenWidth  = 640;
        VideoModeInformation.VisScreenHeight = 480;
    #endif

    // Set up screen information from the mini-port:

    ppdev->ulMode           = VideoModeInformation.ModeIndex;
    ppdev->cxScreen         = VideoModeInformation.VisScreenWidth;
    ppdev->cyScreen         = VideoModeInformation.VisScreenHeight;
    ppdev->cBitsPerPel      = VideoModeInformation.BitsPerPlane;
    ppdev->ulFrequency      = VideoModeInformation.Frequency;

    _FF(refresh)            = VideoModeInformation.Frequency;

    DISPDBG((1, "ScreenStride: %lx", VideoModeInformation.ScreenStride));

    ppdev->flHooks          = (
                               HOOK_BITBLT     |
                               HOOK_TEXTOUT    |
                               HOOK_FILLPATH   |
                               HOOK_COPYBITS   |
                               HOOK_STROKEPATH |
                               HOOK_LINETO     |
#if (_WIN32_WINNT == 0x0400)
                               HOOK_PAINT      |
#endif
                               HOOK_STRETCHBLT |
#if (_WIN32_WINNT >= 0x0500)
                               HOOK_GRADIENTFILL |
#endif
                               0);

    // Fill in the GDIINFO data structure with the default 8bpp values:

    *pgdi = ggdiDefault;

    // Now overwrite the defaults with the relevant information returned
    // from the kernel driver:

    pgdi->ulHorzSize        = VideoModeInformation.XMillimeter;
    pgdi->ulVertSize        = VideoModeInformation.YMillimeter;

    pgdi->ulHorzRes         = VideoModeInformation.VisScreenWidth;
    pgdi->ulVertRes         = VideoModeInformation.VisScreenHeight;
    pgdi->ulPanningHorzRes  = VideoModeInformation.VisScreenWidth;
    pgdi->ulPanningVertRes  = VideoModeInformation.VisScreenHeight;

    pgdi->cBitsPixel        = VideoModeInformation.BitsPerPlane;
    pgdi->cPlanes           = VideoModeInformation.NumberOfPlanes;
    pgdi->ulVRefresh        = VideoModeInformation.Frequency;

    pgdi->ulDACRed          = VideoModeInformation.NumberRedBits;
    pgdi->ulDACGreen        = VideoModeInformation.NumberGreenBits;
    pgdi->ulDACBlue         = VideoModeInformation.NumberBlueBits;

    pgdi->ulLogPixelsX      = pdm->dmLogPixels;
    pgdi->ulLogPixelsY      = pdm->dmLogPixels;

    // Fill in the devinfo structure with the default 8bpp values:

    *pdi = gdevinfoDefault;

    if (VideoModeInformation.BitsPerPlane == 8)
    {
        ppdev->cjPelSize       = 1;
        ppdev->iBitmapFormat   = BMF_8BPP;
        ppdev->ulWhite         = 0xff;

        // Assuming palette is orthogonal - all colors are same size.

        ppdev->cPaletteShift   = 8 - pgdi->ulDACRed;
        DISPDBG((3, "palette shift = %d\n", ppdev->cPaletteShift));
    }
    else if ((VideoModeInformation.BitsPerPlane == 16) ||
             (VideoModeInformation.BitsPerPlane == 15))
    {
        ppdev->cjPelSize       = 2;
        ppdev->iBitmapFormat   = BMF_16BPP;
        ppdev->ulWhite         = 0xffff;
        ppdev->flRed           = VideoModeInformation.RedMask;
        ppdev->flGreen         = VideoModeInformation.GreenMask;
        ppdev->flBlue          = VideoModeInformation.BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_16BPP;

        pdi->iDitherFormat     = BMF_16BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
    }
    else if (VideoModeInformation.BitsPerPlane == 24)//24bpp
    {
        ppdev->cjPelSize       = 3;
        ppdev->iBitmapFormat   = BMF_24BPP;
        ppdev->ulWhite         = 0xffffff;
        ppdev->flRed           = VideoModeInformation.RedMask;
        ppdev->flGreen         = VideoModeInformation.GreenMask;
        ppdev->flBlue          = VideoModeInformation.BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_24BPP;

        pdi->iDitherFormat     = BMF_24BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
    }
    else
    {
        ASSERTDD(VideoModeInformation.BitsPerPlane == 32,
         "This driver supports only 8, 16, 24 and 32bpp");

        ppdev->cjPelSize       = 4;
        ppdev->iBitmapFormat   = BMF_32BPP;
        ppdev->ulWhite         = 0xffffffff;
        ppdev->flRed           = VideoModeInformation.RedMask;
        ppdev->flGreen         = VideoModeInformation.GreenMask;
        ppdev->flBlue          = VideoModeInformation.BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_32BPP;

        pdi->iDitherFormat     = BMF_32BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
    }

    DISPDBG((5, "Passed bInitializeModeFields"));

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bInitializeModeFields"));

    return(FALSE);
}

/******************************Public*Routine******************************\
* DWORD getAvailableModes
*
* Calls the miniport to get the list of modes supported by the kernel driver,
* and returns the list of modes supported by the diplay driver among those
*
* returns the number of entries in the videomode buffer.
* 0 means no modes are supported by the miniport or that an error occured.
*
* NOTE: the buffer must be freed up by the caller.
*
\**************************************************************************/

DWORD getAvailableModes(
HANDLE                   hDriver,
PVIDEO_MODE_INFORMATION* modeInformation,       // Must be freed by caller
DWORD*                   cbModeSize)
{
    ULONG                   ulTemp;
    VIDEO_NUM_MODES         modes;
    PVIDEO_MODE_INFORMATION pVideoTemp;

    //
    // Get the number of modes supported by the mini-port
    //

    if (EngDeviceIoControl(hDriver,
                           IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES,
                           NULL,
                           0,
                           &modes,
                           sizeof(VIDEO_NUM_MODES),
                           &ulTemp))
    {
        DISPDBG((0, "getAvailableModes - Failed VIDEO_QUERY_NUM_AVAIL_MODES"));
        return(0);
    }

    *cbModeSize = modes.ModeInformationLength;

    //
    // Allocate the buffer for the mini-port to write the modes in.
    //

    *modeInformation = ENGALLOCMEM(FL_ZERO_MEMORY,
                                   modes.NumModes * modes.ModeInformationLength,
                                   ALLOC_TAG,
                                   modeInformation);

    if (*modeInformation == (PVIDEO_MODE_INFORMATION) NULL)
    {
        DISPDBG((0, "getAvailableModes - Failed EngAllocMem"));
        return 0;
    }

    //
    // Ask the mini-port to fill in the available modes.
    //

    if (EngDeviceIoControl(hDriver,
                           IOCTL_VIDEO_QUERY_AVAIL_MODES,
                           NULL,
                           0,
                           *modeInformation,
                           modes.NumModes * modes.ModeInformationLength,
                           &ulTemp))
    {

        DISPDBG((0, "getAvailableModes - Failed VIDEO_QUERY_AVAIL_MODES"));

        ENGFREEMEM(*modeInformation);
        *modeInformation = (PVIDEO_MODE_INFORMATION) NULL;

        return(0);
    }

    //
    // Now see which of these modes are supported by the display driver.
    // As an internal mechanism, set the length to 0 for the modes we
    // DO NOT support.
    //

    ulTemp = modes.NumModes;
    pVideoTemp = *modeInformation;

    //
    // Mode is rejected if it is not one plane, or not graphics, or is not
    // one of 8, 15, 16, 24 or 32 bits per pel.
    //

    while (ulTemp--)
    {
        if ((pVideoTemp->NumberOfPlanes != 1 ) ||
            !(pVideoTemp->AttributeFlags & VIDEO_MODE_GRAPHICS) ||
            ((pVideoTemp->BitsPerPlane != 8) &&
             (pVideoTemp->BitsPerPlane != 15) &&
             (pVideoTemp->BitsPerPlane != 16) &&
             (pVideoTemp->BitsPerPlane != 24) && //24bpp
             (pVideoTemp->BitsPerPlane != 32)))
        {
            DISPDBG((2, "Rejecting miniport mode:"));
            DISPDBG((2, "   Screen width  -- %li", pVideoTemp->VisScreenWidth));
            DISPDBG((2, "   Screen height -- %li", pVideoTemp->VisScreenHeight));
            DISPDBG((2, "   Bits per pel  -- %li", pVideoTemp->BitsPerPlane *
                                                   pVideoTemp->NumberOfPlanes));
            DISPDBG((2, "   Frequency     -- %li", pVideoTemp->Frequency));

            pVideoTemp->Length = 0;
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + modes.ModeInformationLength);
    }

    return(modes.NumModes);
}

#if ENABLE_RECONFIG_VIDMEM

//-----------------------------------------------------------------------------
//
// reconfigureVideoMemory
//
// modifies configuration of video memory to more closely match the
// win9x layout
//
//-----------------------------------------------------------------------------

#define MEMCONFIG_DBGLVL    0

void
reconfigureVideoMemory ( PDEV *ppdev )
{
  BYTE*	pjH3Base = ppdev->pjH3Base;
  DWORD bltDstFormat, dstPixelFormat;
  DWORD memoryAvailable;
  DWORD minimumLinear;

  GWH_DECL;


  // Reorganize frame buffer as follows:

  // --------------------------------
  //              cmdfifo
  // --------------------------------
  //             hw cursor
  // --------------------------------
  //             font cache
  // --------------------------------
  //      ddraw stretch blt slop
  // --------------------------------
  //            linear heap
  // --------------------------------
  //             tiled heap
  // --------------------------------
  //          desktop/primary
  // --------------------------------


  _FF(TotalVRAM) = ppdev->cjBank;

  memoryAvailable = _FF(TotalVRAM) - ppdev->ulScreenOffset;

  DISPDBG((MEMCONFIG_DBGLVL, "Mode : %4ld x %4ld x %2ld", ppdev->cxScreen, ppdev->cyScreen, ppdev->cjPelSize * 8));
  DISPDBG((MEMCONFIG_DBGLVL, "  TotalVRAM            = %8lXh", _FF(TotalVRAM)));
  DISPDBG((MEMCONFIG_DBGLVL, "  memoryAvailable      = %8lXh", memoryAvailable));

  DISPDBG((MEMCONFIG_DBGLVL, "                   offset      length"));

  // the cmdfifo location has already been setup and the hw configured
  // in bEnableHardware
  DISPDBG((MEMCONFIG_DBGLVL, "Cmdfifo       : %8lXh   %8lXh",
           ppdev->fifoData.fifoStart - ppdev->pjScreenBase,
           ppdev->fifoData.fifoSize + H3_FIFO_END_ADJUST));

  // the hw cursor location has already been setup in bEnableHardware
  // and the hw has been configured in bEnablePointer

  // ppdev->ulScreenOffset currently points to the end of the hw cursor
  // location, so we'll use ulScreenOffset as a variable to keep track
  // of our current position in the frame buffer
#if DBG
  if (ppdev->cjPointerOffset)
  {
    DISPDBG((MEMCONFIG_DBGLVL, "HW Cursor     : %8lXh   %8lXh",
             ppdev->cjPointerOffset, HW_POINTER_TOTAL_SIZE));
  }
#endif

  // re-init font cache location by overriding location already setup
  // in bEnableText
  ppdev->ulGlyphStart = ppdev->ulScreenOffset;
  ppdev->ulScreenOffset += ppdev->cxMemory * ppdev->cjPelSize * ppdev->cyText;
  ppdev->ulGlyphEnd   = ppdev->ulScreenOffset;
  DISPDBG((MEMCONFIG_DBGLVL, "Font cache    : %8lXh   %8lXh",
           ppdev->ulGlyphStart, ppdev->ulGlyphEnd - ppdev->ulGlyphStart));
  memoryAvailable -= (ppdev->ulGlyphEnd - ppdev->ulGlyphStart);

  // re-init ddraw stretch blt slop location by overriding location
  // already setup in bEnableDirectDraw
  _DS(stretchBltStart) = ppdev->ulScreenOffset;
  _DS(stretchBltSize)  = 0x2000;
  ppdev->ulScreenOffset += _DS(stretchBltSize);
  DISPDBG((MEMCONFIG_DBGLVL, "DD Slop       : %8lXh   %8lXh",
           _DS(stretchBltStart), _DS(stretchBltSize)));
  memoryAvailable -= _DS(stretchBltSize);

#if ENABLE_TILED_HEAP && !defined(CSIM)
  if ((2 == ppdev->cjPelSize) || ((IS_NAPALM) && (4 == ppdev->cjPelSize)))
  {
    DWORD screenSize;
    DWORD tileInX, tileInY;
    LONG  numTiledColorBuffers;
    DWORD lfbMemoryConfig;
    DWORD enableTiledMode;
    DWORD memoryWastedPerZBufferHeap;
    DWORD totalLinearAddrSpace;
    LONG  lPitch;


    // default to enableTiledMode enabled
    // but allow registry key to override
    if (GetRegDWORD(ppdev, "TiledMode", &enableTiledMode))
    {
      if (0 == enableTiledMode)
      {
        DISPDBG((0, "  TiledMode disabled in registry, going to linear setup"));
        goto LinearSetup;
      }
    }
#if 0
    else
    {
      DISPDBG((0, "  defaulting to TiledMode disabled, going to linear setup"));
      goto LinearSetup;
    }
#endif

    // determine num tiles in x
    tileInX = (((ppdev->cxScreen * ppdev->cjPelSize) + SST_TILE_WIDTH - 1) / SST_TILE_WIDTH);
    // always make tileInX even
    tileInX = (tileInX + 1) & ~1;
    // determine num tiles in y
    tileInY = ((ppdev->cyScreen + SST_TILE_HEIGHT - 1) / SST_TILE_HEIGHT);
    // always make tileInY even
    tileInY = (tileInY + 1) & ~1;
#ifdef SLI_AA
    if (2 < _FF(dwNumUnits))
    {
      // make tileInY an even multiple of the number of chips
      tileInY = (tileInY + (_FF(dwNumUnits) - 1)) & ~(_FF(dwNumUnits) - 1);
    }
#endif

    // determine size of screen in tiled mode
    screenSize = tileInX * tileInY * SST_TILE_SIZE;

    // this should give minimumLinear = 6MB on 64MB boards
    //                  minimumLinear = 3MB on 32MB boards
    //              and minimumLinear = 1.5MB on 16MB boards
    minimumLinear = (_FF(TotalVRAM) / (16*1024*1024)) * 0x180000;

    // compute how much memory will be wasted for z buffer alignment
    // so that this amount of memory can be used in the determination of how many colorBuffers we
    // can support while still maintaining a linear heap of size minimumLinear or greater
    memoryWastedPerZBufferHeap = tileInX * SST_TILE_SIZE * _FF(dwNumUnits);

    // determine number of fullscreen buffers we can support in this mode
    //numTiledColorBuffers = max(1, min(4, (memoryAvailable - minimumLinear) / screenSize)) - 1;
    numTiledColorBuffers = max(1, min(4, ((memoryAvailable - memoryWastedPerZBufferHeap) - minimumLinear) / screenSize)) - 1;

TryItAgain:
    // if the number of fullscreen buffers we can support is zero
    // then we might as well just go linear
    if (0 == numTiledColorBuffers)
    {
      DISPDBG((0, "  numTiledColorBuffers = 0, going to linear setup"));
      goto LinearSetup;
    }

    if (IS_NAPALM &&
        (3 == numTiledColorBuffers) &&
        ((memoryAvailable - (2 * memoryWastedPerZBufferHeap)) >= (minimumLinear + (screenSize * 8))))
    {
      _FF(ddSecondaryHeapSize) = (4 * screenSize) + memoryWastedPerZBufferHeap;
    }
    else
    {
      _FF(ddSecondaryHeapSize) = 0;
    }

    // add any color buffer limitations here
    // such as only allowing 2 colorbuffers in 1024x768x16 on 8MB cards
#if 0
    if ()
    {
      numTiledColorBuffers--;
    }
#endif

    _FF(ddNumColorBuff) = numTiledColorBuffers;

    // the desktop is at the end of the framebuffer
    _FF(gdiDesktopStart) = _FF(TotalVRAM) - screenSize;
    _FF(gdiDesktopSize)  = screenSize;

    // the tiled heap comes before the desktop
    _FF(ddTiledHeapSize) = _FF(ddNumColorBuff) * screenSize;
    if (1 < _FF(ddNumColorBuff))
    {
      // note that this little trick which is needed for even/odd alignment of z & back buffers
      // will potentially leave less than MINIMUM_LINEAR_HEAP for ddLinearHeapSize
      _FF(ddTiledHeapSize) += memoryWastedPerZBufferHeap;
    }
    _FF(ddTiledHeapStart) = _FF(gdiDesktopStart) - _FF(ddTiledHeapSize);

    // secondary heaps for aa buffers
    _FF(ddSecondaryHeapStart) = _FF(ddTiledHeapStart) - _FF(ddSecondaryHeapSize);

    // the linear heap comes before the tiled heap
    _FF(ddLinearHeapStart) = ppdev->ulScreenOffset;
    _FF(ddLinearHeapSize)  = _FF(ddSecondaryHeapStart) - _FF(ddLinearHeapStart);

#if DBG
    DISPDBG((MEMCONFIG_DBGLVL, "Linear Heap   : %8lXh   %8lXh",
             _FF(ddLinearHeapStart), _FF(ddLinearHeapSize)));
    if (_FF(ddSecondaryHeapSize))
    {
      DISPDBG((MEMCONFIG_DBGLVL, "Secondary Heap: %8lXh   %8lXh",
               _FF(ddSecondaryHeapStart), _FF(ddSecondaryHeapSize)));
    }
    DISPDBG((MEMCONFIG_DBGLVL, "Tiled Heap    : %8lXh   %8lXh",
             _FF(ddTiledHeapStart), _FF(ddTiledHeapSize)));
    DISPDBG((MEMCONFIG_DBGLVL, "Desktop       : %8lXh   %8lXh",
             _FF(gdiDesktopStart), _FF(gdiDesktopSize)));
#endif

#if 0
    DISPDBG((1, "  just putting gdi desktop in tiled mode for now"));
    DISPDBG((1, "  combining tiled heap into linear heap"));
    // for now, set ddTiledHeapSize to 0, so a ddraw heap isn't reported
    // and set the ddTiledHeapStart to gdiDesktopStart so we just get the desktop
    // in tiled mode and might as well toss the tiled heap into linear heap
    _FF(ddLinearHeapSize) += _FF(ddTiledHeapSize);
    _FF(ddTiledHeapSize) = 0;
    _FF(ddTiledHeapStart) = _FF(gdiDesktopStart);
    _FF(ddNumColorBuff) = 0;
#endif

    DISPDBG((MEMCONFIG_DBGLVL, "  tileInX              = %8lXh", tileInX));
    DISPDBG((MEMCONFIG_DBGLVL, "  tileInY              = %8lXh", tileInY));
    DISPDBG((MEMCONFIG_DBGLVL, "  tiled screenSize     = %8lXh", screenSize));
    DISPDBG((MEMCONFIG_DBGLVL, "  numTiledColorBuffers = %8lXh", numTiledColorBuffers));
    DISPDBG((MEMCONFIG_DBGLVL, "  memoryWastedPerZBufferHeap = %08lXh", memoryWastedPerZBufferHeap));

    _FF(ddTileMark) = _FF(ddTiledHeapStart) & ~SSTG_IS_TILED;

#if defined(CSIM)
    lfbMemoryConfig = ((SstIORegs *)ppdev->CSIM_pjBase)->lfbMemoryConfig;
#else
    lfbMemoryConfig = GET(ghwIO->lfbMemoryConfig);
#endif
    DISPDBG((MEMCONFIG_DBGLVL, "  org lfbMemoryConfig  = %8lXh", lfbMemoryConfig));
    lfbMemoryConfig &= ~SST_RAW_LFB_TILE_BEGIN_PAGE;
    lfbMemoryConfig |= SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE(_FF(ddTileMark) >> 12L);
    lfbMemoryConfig &= ~SST_RAW_LFB_ADDR_STRIDE;

    // In tiled mode, 2048x1536x16 and 1920x1080x32 are failing the GuiJr Fonts test on
    // the Win2000 GDI tab of DCT 300
    // so just go to the linear setup for these modes
    if (((2048 == ppdev->cxScreen) && (1536 == ppdev->cyScreen) && (2 == ppdev->cjPelSize)) ||
        ((1920 == ppdev->cxScreen) && (1080 == ppdev->cyScreen) && (4 == ppdev->cjPelSize)))
    {
      goto LinearSetup;
    }
    // don't allow the tile pitch and linear pitch to be identical on V3
    else if (IS_VOODOO3)
    {
      if (1024 > ppdev->lDelta)
      {
        lfbMemoryConfig |= SST_RAW_LFB_ADDR_STRIDE_1K;
        lPitch = 1024;
      }
      else if (2048 > ppdev->lDelta)
      {
        lfbMemoryConfig |= SST_RAW_LFB_ADDR_STRIDE_2K;
        lPitch = 2048;
      }
      else if (4096 > ppdev->lDelta)
      {
        lfbMemoryConfig |= SST_RAW_LFB_ADDR_STRIDE_4K;
        lPitch = 4096;
      }
      else if (8192 > ppdev->lDelta)
      {
        lfbMemoryConfig |= SST_RAW_LFB_ADDR_STRIDE_8K;
        lPitch = 8192;
      }
      else if (16384 > ppdev->lDelta)
      {
        DISPDBG((0, "  pitch=%ld (which requires 16K stride), going to linear setup", ppdev->lDelta));
        goto LinearSetup;
      }
      else
      {
        DISPDBG((0, "  pitch=%ld (which requires > 16K stride), going to linear setup", ppdev->lDelta));
        goto LinearSetup;
      }
    }
    // allow the tile pitch and linear pitch to be identical on Napalm
    else
    {
      if (1024 >= ppdev->lDelta)
      {
        lfbMemoryConfig |= SST_RAW_LFB_ADDR_STRIDE_1K;
        lPitch = 1024;
      }
      else if (2048 >= ppdev->lDelta)
      {
        lfbMemoryConfig |= SST_RAW_LFB_ADDR_STRIDE_2K;
        lPitch = 2048;
      }
      else if (4096 >= ppdev->lDelta)
      {
        lfbMemoryConfig |= SST_RAW_LFB_ADDR_STRIDE_4K;
        lPitch = 4096;
      }
      else if (8192 >= ppdev->lDelta)
      {
        lfbMemoryConfig |= SST_RAW_LFB_ADDR_STRIDE_8K;
        lPitch = 8192;
      }
      else if (16384 >= ppdev->lDelta)
      {
        lfbMemoryConfig |= SST_RAW_LFB_ADDR_STRIDE_16K;
        lPitch = 16384;
      }
      else
      {
        DISPDBG((0, "  pitch=%ld (which requires > 16K stride), going to linear setup", ppdev->lDelta));
        goto LinearSetup;
      }
    }

    // compute linear address space this configuration uses in sli mode
    // if it's more than 2 * ppdev->cjBank, decrease the numTiledColorBuffers and try again
    totalLinearAddrSpace = _FF(ddTiledHeapStart);   // the linear region
    totalLinearAddrSpace += _FF(dwNumUnits) * tileInY * (numTiledColorBuffers + 1) * SST_TILE_HEIGHT * lPitch; // the tiled region including sli mode
    DISPDBG((MEMCONFIG_DBGLVL, "  totalLinearAddrSpace=%08lXh, lPitch=%8lXh", totalLinearAddrSpace, lPitch));
    if (totalLinearAddrSpace > (DWORD)(2 * ppdev->cjBank))
    {
      DISPDBG((MEMCONFIG_DBGLVL, "  totalLinearAddrSpace exceeds 2*ppdev->cjBank", 2 * ppdev->cjBank));

      while ((numTiledColorBuffers) && (totalLinearAddrSpace > (DWORD)(2 * ppdev->cjBank)))
      {
        numTiledColorBuffers--;
        totalLinearAddrSpace -= _FF(dwNumUnits) * tileInY * SST_TILE_HEIGHT * lPitch;
      }

      DISPDBG((MEMCONFIG_DBGLVL, "  decremented numTiledColorBuffers to %ld (totalLinearAddrSpace=%08lXh)",
               numTiledColorBuffers, totalLinearAddrSpace));
      goto TryItAgain;
    }

    ppdev->lDelta = lPitch;

    _FF(ddTileStride) = tileInX;
    _FF(ddTilePitch)  = ppdev->lDelta;
    _FF(ddTileHeight) = tileInY;

    lfbMemoryConfig &= ~SST_RAW_LFB_TILE_STRIDE;
    lfbMemoryConfig |= (tileInX << SST_RAW_LFB_TILE_STRIDE_SHIFT);
    SETDW(ghwIO->lfbMemoryConfig, lfbMemoryConfig);
    DISPDBG((MEMCONFIG_DBGLVL, "  new lfbMemoryConfig  = %8lXh (hw has %8lXh)", lfbMemoryConfig, GET(ghwIO->lfbMemoryConfig)));
#if defined(CSIM)
    ((SstIORegs *)ppdev->CSIM_pjBase)->lfbMemoryConfig = lfbMemoryConfig;
#endif

#if 0
#if DBG
    // this line is intended to display the mode info similar to the spreadsheet in docs
    switch (_FF(ddNumColorBuff))
    {
      case 1:
        DISPDBG((MEMCONFIG_DBGLVL, "%4ld %4ld %2ld %6lXh %6lXh %6lXh %6lXh %6lXh %2ld %6lXh %6lXh %6lXh %6lXh %6s %6s %6s %6s %6s %6s %d %d %d",
                 ppdev->cxScreen,
                 ppdev->cyScreen,
                 ppdev->cjPelSize * 8,
                 ppdev->ulGlyphEnd - ppdev->ulGlyphStart,             // Font Cache
                 ppdev->cjPointerOffset ? HW_POINTER_TOTAL_SIZE : 0,  // HW Cursor
                 _FF(gdiDesktopSize),                                 // PRIMARY
                 _FF(ddTiledHeapSize),                                // TILED
                 _FF(ddLinearHeapSize) +_FF(ddSecondaryHeapSize),     // LINEAR
                 _FF(ddNumColorBuff),                                 // Number Buffers
                 _FF(ddTileMark),                                     // Tile Mark
                 _FF(ddTiledHeapSize) + _FF(gdiDesktopSize),          // Tile Size
                 _FF(gdiDesktopStart),                                // FB
                 _FF(gdiDesktopStart) - _FF(gdiDesktopSize),          // BB
                 "n/a", //_FF(gdiDesktopStart) - 2 * _FF(gdiDesktopSize) - memoryWastedPerZBufferHeap + SST_TILE_SIZE, // ZB
                 "n/a", //_FF(gdiDesktopStart) - 3 * _FF(gdiDesktopSize) - memoryWastedPerZBufferHeap,                 // TB
                 "n/a",
                 "n/a",
                 "n/a",
                 "n/a",
                 (_FF(ddLinearHeapSize) + _FF(ddSecondaryHeapSize) + _FF(ddTiledHeapSize) - minimumLinear)/_FF(gdiDesktopSize),
                 _FF(ddTileStride),
                 _FF(ddTileHeight)
               ));
        break;
      case 2:
        DISPDBG((MEMCONFIG_DBGLVL, "%4ld %4ld %2ld %6lXh %6lXh %6lXh %6lXh %6lXh %2ld %6lXh %6lXh %6lXh %6lXh %6lXh %6s %6s %6s %6s %6s %d %d %d",
                 ppdev->cxScreen,
                 ppdev->cyScreen,
                 ppdev->cjPelSize * 8,
                 ppdev->ulGlyphEnd - ppdev->ulGlyphStart,             // Font Cache
                 ppdev->cjPointerOffset ? HW_POINTER_TOTAL_SIZE : 0,  // HW Cursor
                 _FF(gdiDesktopSize),                                 // PRIMARY
                 _FF(ddTiledHeapSize),                                // TILED
                 _FF(ddLinearHeapSize) +_FF(ddSecondaryHeapSize),     // LINEAR
                 _FF(ddNumColorBuff),                                 // Number Buffers
                 _FF(ddTileMark),                                     // Tile Mark
                 _FF(ddTiledHeapSize) + _FF(gdiDesktopSize),          // Tile Size
                 _FF(gdiDesktopStart),                                // FB
                 _FF(gdiDesktopStart) - _FF(gdiDesktopSize),          // BB
                 _FF(gdiDesktopStart) - 2 * _FF(gdiDesktopSize) - memoryWastedPerZBufferHeap + SST_TILE_SIZE, // ZB
                 "n/a", //_FF(gdiDesktopStart) - 3 * _FF(gdiDesktopSize) - memoryWastedPerZBufferHeap,                 // TB
                 "n/a",
                 "n/a",
                 "n/a",
                 "n/a",
                 (_FF(ddLinearHeapSize) + _FF(ddSecondaryHeapSize) + _FF(ddTiledHeapSize) - minimumLinear)/_FF(gdiDesktopSize),
                 _FF(ddTileStride),
                 _FF(ddTileHeight)
               ));
        break;
      case 3:
        if (0 == _FF(ddSecondaryHeapSize))
        {
          DISPDBG((MEMCONFIG_DBGLVL, "%4ld %4ld %2ld %6lXh %6lXh %6lXh %6lXh %6lXh %2ld %6lXh %6lXh %6lXh %6lXh %6lXh %6lXh %6s %6s %6s %6s %d %d %d",
                   ppdev->cxScreen,
                   ppdev->cyScreen,
                   ppdev->cjPelSize * 8,
                   ppdev->ulGlyphEnd - ppdev->ulGlyphStart,             // Font Cache
                   ppdev->cjPointerOffset ? HW_POINTER_TOTAL_SIZE : 0,  // HW Cursor
                   _FF(gdiDesktopSize),                                 // PRIMARY
                   _FF(ddTiledHeapSize),                                // TILED
                   _FF(ddLinearHeapSize) +_FF(ddSecondaryHeapSize),     // LINEAR
                   _FF(ddNumColorBuff),                                 // Number Buffers
                   _FF(ddTileMark),                                     // Tile Mark
                   _FF(ddTiledHeapSize) + _FF(gdiDesktopSize),          // Tile Size
                   _FF(gdiDesktopStart),                                // FB
                   _FF(gdiDesktopStart) - _FF(gdiDesktopSize),          // BB
                   _FF(gdiDesktopStart) - 2 * _FF(gdiDesktopSize) - memoryWastedPerZBufferHeap + SST_TILE_SIZE, // ZB
                   _FF(gdiDesktopStart) - 3 * _FF(gdiDesktopSize) - memoryWastedPerZBufferHeap,                 // TB
                   "n/a",
                   "n/a",
                   "n/a",
                   "n/a",
                   (_FF(ddLinearHeapSize) + _FF(ddSecondaryHeapSize) + _FF(ddTiledHeapSize) - minimumLinear)/_FF(gdiDesktopSize),
                   _FF(ddTileStride),
                   _FF(ddTileHeight)
                 ));
        }
        else
        {
          DISPDBG((MEMCONFIG_DBGLVL, "%4ld %4ld %2ld %6lXh %6lXh %6lXh %6lXh %6lXh %2ld %6lXh %6lXh %6lXh %6lXh %6lXh %6lXh %6lXh %6lXh %6lXh %6lXh %d %d %d",
                   ppdev->cxScreen,
                   ppdev->cyScreen,
                   ppdev->cjPelSize * 8,
                   ppdev->ulGlyphEnd - ppdev->ulGlyphStart,             // Font Cache
                   ppdev->cjPointerOffset ? HW_POINTER_TOTAL_SIZE : 0,  // HW Cursor
                   _FF(gdiDesktopSize),                                 // PRIMARY
                   _FF(ddTiledHeapSize),                                // TILED
                   _FF(ddLinearHeapSize) +_FF(ddSecondaryHeapSize),     // LINEAR
                   _FF(ddNumColorBuff),                                 // Number Buffers
                   _FF(ddTileMark),                                     // Tile Mark
                   _FF(ddTiledHeapSize) + _FF(gdiDesktopSize),          // Tile Size
                   _FF(gdiDesktopStart),                                // FB
                   _FF(gdiDesktopStart) - _FF(gdiDesktopSize),          // BB
                   _FF(gdiDesktopStart) - 2 * _FF(gdiDesktopSize) - memoryWastedPerZBufferHeap + SST_TILE_SIZE,   // ZB
                   _FF(gdiDesktopStart) - 3 * _FF(gdiDesktopSize) - memoryWastedPerZBufferHeap,                   // TB
                   _FF(ddTiledHeapStart) - _FF(gdiDesktopSize),         // FB (AA)
                   _FF(ddTiledHeapStart) - 2 * _FF(gdiDesktopSize),     // BB (AA)
                   _FF(ddTiledHeapStart) - 3 * _FF(gdiDesktopSize) - memoryWastedPerZBufferHeap + SST_TILE_SIZE,  // ZB (AA)
                   _FF(ddTiledHeapStart) - 4 * _FF(gdiDesktopSize) - memoryWastedPerZBufferHeap,                  // TB (AA)
                   (_FF(ddLinearHeapSize) + _FF(ddSecondaryHeapSize) + _FF(ddTiledHeapSize) - minimumLinear)/_FF(gdiDesktopSize), // total buffers
                   _FF(ddTileStride),                                   // tile width
                   _FF(ddTileHeight)                                    // tile height
                 ));
        }
        break;
    }
#endif
#endif

    ppdev->ulScreenOffset = _FF(gdiDesktopStart) | SSTG_IS_TILED;
    DISPDBG((MEMCONFIG_DBGLVL, "  ppdev->ulScreenOffset = %8lXh", ppdev->ulScreenOffset));

    ppdev->pjScreen = ppdev->pjScreenBase + HwPtrToLfbPtr(ppdev, _FF(gdiDesktopStart));
    DISPDBG((MEMCONFIG_DBGLVL, "  ppdev->pjScreen       = %8lXh", ppdev->pjScreen));

    SETDW(ghwIO->vidDesktopStartAddr, (ppdev->ulScreenOffset & ~SSTG_IS_TILED));
    DISPDBG((MEMCONFIG_DBGLVL, "  vidDesktopStartAddr   = %8lXh", GET(ghwIO->vidDesktopStartAddr)));
    H3PRINTF((ppdev, "  vidDesktopStartAddr = %08lX\r\n", ppdev->ulScreenOffset));
#if defined(CSIM)
    ((SstIORegs *)ppdev->CSIM_pjBase)->vidDesktopStartAddr = (ppdev->ulScreenOffset & ~SSTG_IS_TILED);
#endif

#if defined(CSIM)
    SETDW(ghwIO->vidProcCfg, ((SstIORegs *)ppdev->CSIM_pjBase)->vidProcCfg | SST_DESKTOP_TILED_EN);
    ((SstIORegs *)ppdev->CSIM_pjBase)->vidProcCfg = ((SstIORegs *)ppdev->CSIM_pjBase)->vidProcCfg | SST_DESKTOP_TILED_EN;
#else
    SETDW(ghwIO->vidProcCfg, GET(ghwIO->vidProcCfg) | SST_DESKTOP_TILED_EN);
#endif
    DISPDBG((MEMCONFIG_DBGLVL, "  vidProcCfg            = %8lXh", GET(ghwIO->vidProcCfg)));

#if defined(CSIM)
    SETDW(ghwIO->vidDesktopOverlayStride, (((SstIORegs *)ppdev->CSIM_pjBase)->vidDesktopOverlayStride & ~SST_DESKTOP_LINEAR_STRIDE) | tileInX);
    ((SstIORegs *)ppdev->CSIM_pjBase)->vidDesktopOverlayStride = (((SstIORegs *)ppdev->CSIM_pjBase)->vidDesktopOverlayStride & ~SST_DESKTOP_LINEAR_STRIDE) | tileInX;
#else
    SETDW(ghwIO->vidDesktopOverlayStride, (GET(ghwIO->vidDesktopOverlayStride) & ~SST_DESKTOP_LINEAR_STRIDE) | tileInX);
#endif
    DISPDBG((MEMCONFIG_DBGLVL, "  vidDesktopOverlayStride= %8lXh", GET(ghwIO->vidDesktopOverlayStride)));

    SETDW(ghw2D->srcBaseAddr, ppdev->ulScreenOffset);
    SETDW(ghw2D->dstBaseAddr, ppdev->ulScreenOffset);

    // flag the primary is in tiled mode
    _FF(ddPrimaryInTile) = TRUE;
#if ENABLE_LOG_FILE
    retroLogForce(ppdev, "retro3dfx PRIMARY-TILED: scrOff=%08lXh %ldx%ldx%ld\r\n",
                  ppdev->ulScreenOffset, (LONG)ppdev->cxScreen,
                  (LONG)ppdev->cyScreen, (LONG)(ppdev->cjPelSize * 8));
#endif
  }
  else
#endif
  {
    DWORD screenSize;
    DWORD lfbMemoryConfig;


#if ENABLE_TILED_HEAP && !defined(CSIM)
LinearSetup:
#endif

    screenSize = ppdev->cxScreen * ppdev->cjPelSize * ppdev->cyScreen;

    // no color buffers
    _FF(ddNumColorBuff) = 0;

    // the desktop is at the end of the framebuffer
    _FF(gdiDesktopStart)   = _FF(TotalVRAM) - screenSize;
    _FF(gdiDesktopSize)    = screenSize;

    // no secondary heaps for aa
    _FF(ddSecondaryHeapStart) = _FF(TotalVRAM);
    _FF(ddSecondaryHeapSize)  = 0;

    // no tiled heap
    _FF(ddTiledHeapStart)  = _FF(TotalVRAM);
    _FF(ddTiledHeapSize)   = 0;

    // the linear heap comes before the desktop
    _FF(ddLinearHeapStart) = ppdev->ulScreenOffset;
    _FF(ddLinearHeapSize)  = _FF(gdiDesktopStart) - _FF(ddLinearHeapStart);

    DISPDBG((MEMCONFIG_DBGLVL, "Linear Heap   : %8lXh   %8lXh",
             _FF(ddLinearHeapStart), _FF(ddLinearHeapSize)));
    //DISPDBG((MEMCONFIG_DBGLVL, "Tiled Heap    : %8lXh   %8lXh",
    //         _FF(ddTiledHeapStart), _FF(ddTiledHeapSize)));
    DISPDBG((MEMCONFIG_DBGLVL, "Desktop       : %8lXh   %8lXh",
             _FF(gdiDesktopStart), _FF(gdiDesktopSize)));

    DISPDBG((MEMCONFIG_DBGLVL, "  linear screenSize    = %8lXh", screenSize));

    _FF(ddTileMark) = _FF(ddTiledHeapStart) & ~SSTG_IS_TILED;

    lfbMemoryConfig = SST_RAW_LFB_ADDR_STRIDE_2K | SST_RAW_LFB_TILE_BEGIN_PAGE;

    DISPDBG((MEMCONFIG_DBGLVL, "  org lfbMemoryConfig  = %8lXh", GET(ghwIO->lfbMemoryConfig)));
    SETDW(ghwIO->lfbMemoryConfig, lfbMemoryConfig);
    DISPDBG((MEMCONFIG_DBGLVL, "  new lfbMemoryConfig  = %8lXh", lfbMemoryConfig));
#if defined(CSIM)
    ((SstIORegs *)ppdev->CSIM_pjBase)->lfbMemoryConfig = lfbMemoryConfig;
#endif

#if 0
    // this line is intended to display the mode info similar to memalloc.xls in the win9x branch
    DISPDBG((1, "%8lXh  %8ld  %8ld  %8ld  %8lXh  %8lXh  %8lXh  %8lXh  %8ld",
             _FF(TotalVRAM),
             ppdev->cxScreen,
             ppdev->cyScreen,
             ppdev->cjPelSize * 8,
             _FF(ddLinearHeapSize),
             _FF(gdiDesktopSize),
             _FF(ddTiledHeapSize),
             ppdev->fifoData.fifoSize + H3_FIFO_END_ADJUST,
             _FF(ddNumColorBuff)));
#endif

    ppdev->ulScreenOffset = _FF(gdiDesktopStart);
    DISPDBG((MEMCONFIG_DBGLVL, "  ppdev->ulScreenOffset = %8lXh", ppdev->ulScreenOffset));

    ppdev->pjScreen = ppdev->pjScreenBase+ppdev->ulScreenOffset;
    DISPDBG((MEMCONFIG_DBGLVL, "  ppdev->pjScreen       = %8lXh", ppdev->pjScreen));

    SETDW(ghwIO->vidDesktopStartAddr, ppdev->ulScreenOffset);
    DISPDBG((MEMCONFIG_DBGLVL, "  vidDesktopStartAddr   = %8lXh", GET(ghwIO->vidDesktopStartAddr)));
    H3PRINTF((ppdev, "  vidDesktopStartAddr = %08lX\r\n", ppdev->ulScreenOffset));
#if defined(CSIM)
    ((SstIORegs *)ppdev->CSIM_pjBase)->vidDesktopStartAddr = ppdev->ulScreenOffset;
    SETDW(ghwIO->vidProcCfg, ((SstIORegs *)ppdev->CSIM_pjBase)->vidProcCfg);
    SETDW(ghwIO->vidDesktopOverlayStride, ((SstIORegs *)ppdev->CSIM_pjBase)->vidDesktopOverlayStride);
#endif

    SETDW(ghw2D->srcBaseAddr, ppdev->ulScreenOffset);
    SETDW(ghw2D->dstBaseAddr, ppdev->ulScreenOffset);

    // flag the primary is in linear mode
    _FF(ddPrimaryInTile) = FALSE;
#if ENABLE_LOG_FILE
    /* retro3dfx: a LINEAR primary in a 3D-capable mode disables the whole
       exclusive-fullscreen 3D setup path (DDSURF.C ~414) — if this fires on
       warm reruns where the first run was TILED, that's the degradation. */
    retroLogForce(ppdev, "retro3dfx PRIMARY-LINEAR: scrOff=%08lXh %ldx%ldx%ld\r\n",
                  ppdev->ulScreenOffset, (LONG)ppdev->cxScreen,
                  (LONG)ppdev->cyScreen, (LONG)(ppdev->cjPelSize * 8));
#endif
  }

  // Clear desktop surface with rect. fill.

  GWH_PROLOG;

  GETPIXELFORMAT(ppdev->cjPelSize, dstPixelFormat);
#if ENABLE_TILED_HEAP
  if (ppdev->ulScreenOffset & SSTG_IS_TILED)
    BLTFMT(_FF(ddTileStride), dstPixelFormat, bltDstFormat);
  else
#endif
    BLTFMT(ppdev->lDelta, dstPixelFormat, bltDstFormat);

  CHECK_FIFO_ROOM(ppdev, 6);
  GWH_BEGIN_2D_PACKET(6, SSTCP_PKT2_DSTBASEADDR |
                         SSTCP_PKT2_DSTFORMAT   |
                         SSTCP_PKT2_COLORFORE   |
                         SSTCP_PKT2_DSTSIZE     |
                         SSTCP_PKT2_DSTXY       |
                         SSTCP_PKT2_COMMAND);
  SET(1, pjH3Base, dstBaseAddr, ppdev->ulScreenOffset);
  SET(2, pjH3Base, dstFormat,   bltDstFormat);
  SET(3, pjH3Base, colorFore,   0);
  SET(4, pjH3Base, dstSize,     H3_PACKXY_FAST( ppdev->cxScreen, ppdev->cyScreen));
  SET(5, pjH3Base, dstXY,       H3_PACKXY( 0, 0));
  SET(6, pjH3Base, command,     (0xcc << SSTG_ROP0_SHIFT) | SSTG_MONO_PATTERN | SSTG_GO | SSTG_RECTFILL);
  GWH_END_2D_PACKET( 6 );


  GWH_EPILOG;

//#define PAINT_RECTS
#ifdef PAINT_RECTS
  {
    extern void PaintDFBRect(PDEV *, DWORD, DWORD, DWORD, DWORD, DWORD);
    extern DWORD InitFillColor;

    if (_FF(ddLinearHeapStart))
    {
      DWORD pitch = ppdev->cxMemory * ppdev->cjPelSize;
      DWORD cx = ppdev->cxMemory;
      DWORD cy = _FF(ddLinearHeapSize) / pitch;

      PaintDFBRect(ppdev,
                   _FF(ddLinearHeapStart),
                   0,
                   H3_PACKXY_FAST(cx, cy),
                   pitch,
                   InitFillColor);
    }
    if (_FF(ddTiledHeapStart))
    {
      DWORD hwPitch = ppdev->cxMemory * ppdev->cjPelSize;
      DWORD cx = ppdev->cxMemory;
      DWORD cy = _FF(ddTiledHeapSize) / hwPitch;

      PaintDFBRect(ppdev,
                   _FF(ddTiledHeapStart) | SSTG_IS_TILED,
                   0,
                   H3_PACKXY_FAST(cx,cy),
                   ppdev->lDelta,
                   InitFillColor);
    }
  }
#endif
}
#endif


//-------------------------------Public*Routine--------------------------------
//
// DrvResetPDEV
//
// This function is used by GDI to allow a driver to pass state information
// from one driver instance to the next.
//
// Parameters
//  dhpdevOld---Pointer to the PDEV that describes the physical device to be
//              disabled. This value is the handle returned by DrvEnablePDEV.
//  dhpdevNew---Pointer to the PDEV that describes the physical device to be
//              enabled. This value is the handle returned by DrvEnablePDEV.
//
//
// Return Value
//  TRUE if successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------

BOOL
DrvResetPDEV(DHPDEV  dhpdevOld,
             DHPDEV  dhpdevNew)
{
#if (_WIN32_WINNT >= 0x0500)
    PDEV *   ppdevOld = (PDEV *)dhpdevOld;
    PDEV *   ppdevNew = (PDEV *)dhpdevNew;


    DISPDBG((1, "DrvResetPDEV - ppdevOld = %8lXh, ppdevNew = %8lXh", ppdevOld, ppdevNew));

    // pass state information here:

    // pass information if DirectDraw is in exclusive mode
    // to next active PDEV
    ppdevNew->DriverData.ddExclusiveMode = ppdevOld->DriverData.ddExclusiveMode;

    // this piece of code was disabling the fifo at an inopportune time
    // for unrealtournament.
	// pass information of ulCmdFifoDisabled
//	ppdevNew->ulCmdFifoDisabled = ppdevOld->ulCmdFifoDisabled;

    // sometimes the new ppdev has already some DeviceBitmaps assigned...
    if (ppdevNew->DriverData.ddExclusiveMode)
    {
        bMoveAllDfbsFromOffscreenToDibs(ppdevNew);
    }
#endif

    return TRUE;
}// DrvResetPDEV()
