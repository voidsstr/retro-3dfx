/******************************Module*Header*******************************\
* Module Name: GLICD.c
*
* This module contains the functions for the OpenGL ICD.
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

BOOL bGetICDInfo(PDEV*);

#ifdef OPENGL_ICD

///////////////////////////////////////////////////////////////////////
//
//      PIXEL FORMAT DESCRIPTIONS FOR OPENGL ICD
//
///////////////////////////////////////////////////////////////////////

static iNumPixelFormats;

static PIXELFORMATDESCRIPTOR wglPixelFormats_16bit[] =
{
    /*
    ** 16 bit
    */
    {
        sizeof(PIXELFORMATDESCRIPTOR),  // size
        1,                              // version
        PFD_SUPPORT_OPENGL      |       // flags
        PFD_DRAW_TO_WINDOW      |
        PFD_SUPPORT_GDI,
        PFD_TYPE_RGBA,                  // pixel type
        16, 5, 11, 6, 5, 5, 0,          // BBP, color buffer (Rb, Rs, Gb, Gs, Bb, Bs)
        0, 0,                           // alpha buffer (Abits, Ashift)
        64, 16, 16, 16, 16,             // accumulation buffer (Bits, Rbits, Gbits, Bbits)
        16,                             // depth buffer
        8,                              // stencil buffer
        0,                              // aux buffers
        PFD_MAIN_PLANE,                 // layer type
        0,                              // (reserved)
        0, 0, 0,                        // layer mask, visible mask, damage mask
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),  // size
        1,                              // version
        PFD_SUPPORT_OPENGL      |       // flags
        PFD_DRAW_TO_WINDOW      |
        PFD_DOUBLEBUFFER        |
        PFD_SUPPORT_GDI,
        PFD_TYPE_RGBA,                  // pixel type
        16, 5, 11, 6, 5, 5, 0,          // BBP, color buffer (Rb, Rs, Gb, Gs, Bb, Bs)
        0, 0,                           // alpha buffer (Abits, Ashift)
        64, 16, 16, 16, 16,             // accumulation buffer (Bits, Rbits, Gbits, Bbits)
        16,                             // depth buffer
        8,                              // stencil buffer
        0,                              // aux buffers
        PFD_MAIN_PLANE,                 // layer type
        0,                              // (reserved)
        0, 0, 0,                        // layer mask, visible mask, damage mask
    }
};

static int wglNumPixelFormats_16bit = sizeof(wglPixelFormats_16bit) / 
                                      sizeof(wglPixelFormats_16bit[0]);

static PIXELFORMATDESCRIPTOR wglPixelFormats_32bit[] = {
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  /* size */
        1,                                              /* version */
        PFD_SUPPORT_OPENGL |                            /* flags */
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_GDI,
        PFD_TYPE_RGBA,                                  /* pixel type */
        32, 8, 16, 8, 8, 8, 0,                          /* color buffer */
        8, 24,                                          /* alpha buffer */
        64, 16, 16, 16, 16,                             /* accumulation buffer */
        24,                                             /* depth buffer */
        8,                                              /* stencil buffer */
        0,                                              /* aux buffers */
        PFD_MAIN_PLANE,                                 /* layer type */
        0,                                              /* (reserved) */
        0, 0, 0,                                        /* layer masks */
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  /* size */
        1,                                              /* version */
        PFD_SUPPORT_OPENGL |                            /* flags */
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER   |
        PFD_SUPPORT_GDI,
        PFD_TYPE_RGBA,                                  /* pixel type */
        32, 8, 16, 8, 8, 8, 0,                          /* color buffer */
        8, 24,                                          /* alpha buffer */
        64, 16, 16, 16, 16,                             /* accumulation buffer */
        24,                                             /* depth buffer */
        8,                                              /* stencil buffer */
        0,                                              /* aux buffers */
        PFD_MAIN_PLANE,                                 /* layer type */
        0,                                              /* (reserved) */
        0, 0, 0,                                        /* layer masks */
    }
};

static int wglNumPixelFormats_32bit = sizeof(wglPixelFormats_32bit) / 
                                      sizeof(wglPixelFormats_32bit[0]);



/***************************** Local Routines ****************************/


/* ------------------------------------------------------------------- */

static VOID CALLBACK WndObjCallback(WNDOBJ *pwo, FLONG fl)
{
    // This function does not need to do anything for 3DFX. It would normally
    // be used to track window changes, for example, to assist with clipping
    // or driver list management for the window. But, this must be here for
    // the EngCreateWindow in DrvSetPixelFormat below.
}



/***************************** Public Routines ****************************\
*
* BOOL APIENTRY DrvSetPixelFormat
*
* Required ICD function.
*
\**************************************************************************/
BOOL APIENTRY DrvSetPixelFormat( SURFOBJ *pso,
                                 LONG     iPixelFormat,
                                 HWND     hwnd)
{
    WNDOBJ*                 pwo = NULL;
    PIXELFORMATDESCRIPTOR*  pfd;
    PDEV*    ppdev;

    ppdev = (PDEV*) pso->dhpdev;

    // We only render surfaces managed by this device
    if (pso->dhsurf == NULL)
    {
        return FALSE;
    }

    // Validate the pixel format
    if( (iPixelFormat == 0) || (iPixelFormat > iNumPixelFormats) )
    {
        return FALSE;
    }

    if (ppdev->cBitsPerPel == 32)
    {
        pfd = &(wglPixelFormats_32bit[iPixelFormat-1]);
    }
    else
    {
        pfd = &(wglPixelFormats_16bit[iPixelFormat-1]);
    }

    // Make sure the object supports this pixel format
    if (hwnd == NULL)
    {
        if ((pfd->dwFlags & PFD_DRAW_TO_BITMAP) == 0)   // Memory DC
        {
            return FALSE;
        }
    }
    else
    {
        if ((pfd->dwFlags & PFD_DRAW_TO_WINDOW) == 0)   // Display DC
        {
	    return FALSE;
        }
    }

    // We found that this function must be called to have GDI's gdiSwapBuffer
    // return a TRUE value (really helpful for Quake II). This routine also
    // establishes a callback to this display driver which we do not use at
    // present.
    pwo = EngCreateWnd(pso, hwnd, WndObjCallback, WO_RGN_CLIENT, iPixelFormat);
    if ((pwo == NULL) || (pwo == (WNDOBJ *)-1))
    {
        return FALSE;
    }

    return  TRUE;
}



/**************************************************************************\
*
* LONG APIENTRY DrvDescribePixelFormat
*
* Required ICD function.
*
\**************************************************************************/

LONG DrvDescribePixelFormat( DHPDEV                 dhpdev,
                             LONG                   iPixelFormat,
                             ULONG                  cjpfd,
                             PIXELFORMATDESCRIPTOR* ppfd)
{
    PDEV *ppdev = (PDEV *) dhpdev;
    PIXELFORMATDESCRIPTOR *wglPixelFormats;

    //
    // Check for the ICD in the registry, and also for the file in
    // the system32 directory. If both are found, we return the
    // pixel format information and run the ICD.
    //

    if (bGetICDInfo(ppdev) == FALSE)
    {
	    return 0;
    }

    switch(ppdev->cBitsPerPel)  // 8, 15, 16, 24 or 32
    {
        case 16:	/* 16 bpp */
        case 24:	/* 24 bpp */
            iNumPixelFormats = wglNumPixelFormats_16bit;
            wglPixelFormats = wglPixelFormats_16bit;
            break;
        case 32:        /* 32 bpp */
            if (IS_NAPALM) {
                iNumPixelFormats = wglNumPixelFormats_32bit;
                wglPixelFormats = wglPixelFormats_32bit;
            }
            else {
                iNumPixelFormats = wglNumPixelFormats_16bit;
                wglPixelFormats = wglPixelFormats_16bit;
            }
	        break;
        case  8:	/*  8 bpp */
        case 15:	/* 15 bpp */
        default:
            iNumPixelFormats = 0;
            break;
    }

    if ((iPixelFormat > 0) && (iPixelFormat <= iNumPixelFormats ))
    {
        if (ppfd)
        {
            UINT size;

            size = sizeof(PIXELFORMATDESCRIPTOR);
            if (cjpfd < size) size = cjpfd;

            RtlMoveMemory (ppfd, &(wglPixelFormats[iPixelFormat-1]), size);
        }
        return iNumPixelFormats;
    }
    else
    {
        return 0;
    }
}



/**************************************************************************\
*
* BOOL APIENTRY DrvSwapBuffers
*
* Required ICD function.
*
\**************************************************************************/

//  This function is called because we added a call to EngCreateWnd in DrvSetPixelFormat.
//  Otherwise, this would not be called. But, if this is not called, gdiSwapBuffers returns
//  a false value (0). Because Quake II check this return, it won't run. Therefore, we
//  have to include EngCreateWnd and return TRUE from this function. Brian and Miles, 02/10/1999.

BOOL APIENTRY DrvSwapBuffers( SURFOBJ *pso, WNDOBJ *pwo )
{
    return (TRUE);
}

/******************************Public*Routine******************************\
* BOOL bGetICDInfo
*
* Initializes ICD specific information based on data collected from the
* miniport, and the system32 directory.
*
\**************************************************************************/
BOOL bGetICDInfo(
PDEV* ppdev)
{
  TDFX_QUERY_VALUE_INFO   queryValueInfo;
  DWORD                   ReturnedDataLength;


  //
  // We've overloaded the QueryRegisteryValue request for this value
  // so it checks for the presence of the file in the system32 before
  // it goes out for the registry value. If this returns a string, we
  // definitely have an ICD installed.
  //
  // initialize queryValueInfo for nul terminated string data
  queryValueInfo.Type = REG_SZ;
  queryValueInfo.DataLength = TDFX_MAX_DATA_LENGTH;
  if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_3DFX_QUERY_REGISTRY_VALUE,
                         (PVOID)"3Dfx",           // input buffer
                         strlen("3Dfx") + 1,
                         &queryValueInfo,         // output buffer
                         sizeof(queryValueInfo),
                         &ReturnedDataLength))
  {
    DISPDBG((0, "bGetICDInfo - Can't get 3Dfx ICD information!\n\n"));
  }
  else
  {
    //
    // We found the registry entry, so now check for the existence of 3Dfxogl.dll
    //

    DISPDBG((0, "bGetICDInfo - Got 3Dfx ICD information!\n\n"));
    DISPDBG((0, "              Type         = %d", queryValueInfo.Type));
    DISPDBG((0, "              DataLength   = %d", queryValueInfo.DataLength));
    DISPDBG((0, "              ICD Dll Name = %s", queryValueInfo.Data));

    return TRUE;
  }

  return FALSE;
}

#endif // OPENGL_ICD

