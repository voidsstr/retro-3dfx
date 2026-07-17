/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Revision: 2$
** $Date: 10/11/00 8:52:03 PM$
*/

// This file puts in one place as much of the direct draw initialization 
// code that could be done in 32-bit code, but for the time being only
// seems to work when it is set in 16-bit land.

// Debug levels for trace control
#ifndef __DDINIT_H__
#define __DDINIT_H__

#define DEBUG_APIENTRY       1
#define DEBUG_DDDETAILS      2
#define DEBUG_D3DDETALS      3
#define DEBUG_DDGORY         4
#define DEBUG_D3DGORY        5

/*
 * callbacks from the DIRECTDRAW object
 */
static DDHAL_DDCALLBACKS        cbDDCallbacks =
{
    sizeof( DDHAL_DDCALLBACKS ),
    0,                          // dwFlags
    NULL,                       // DestroyDriver
    NULL,                       // CreateSurface
    NULL,                       // SetColorKey
    NULL,                       // SetMode
    NULL,                       // WaitForVerticalBlank
    NULL,                       // CanCreateSurface
    NULL,                       // CreatePalette
    NULL,                       // lpReserved3
    NULL,                       // SetExclusiveMode
    NULL                        // FlipToGDISurface
};

/*
 * callbacks from the DIRECTDRAWPALETTE object
 */


static DDHAL_DDPALETTECALLBACKS cbDDPaletteCallbacks =
{
    sizeof( DDHAL_DDPALETTECALLBACKS ),
    0,                          // dwFlags
    NULL,                       // DestroyPalette
    NULL,                       // SetEntries
};

/*
 * callbacks from the DIRECTDRAWSURFACE object
 */

static DDHAL_DDSURFACECALLBACKS cbDDSurfaceCallbacks =
{
    sizeof( DDHAL_DDSURFACECALLBACKS ),
    0,                          // dwFlags
    NULL,                       // DestroySurface
    NULL,                       // Flip
    NULL,                       // SetClipList
    NULL,                       // Lock
    NULL,                       // Unlock
    NULL,                       // Blt
    NULL,                       // SetColorKey
    NULL,                       // AddAttachedSurface
    NULL,                       // GetBltStatus
    NULL,                       // GetFlipStatus
    NULL,                       // UpdateOverlay
    NULL,                       // reserved
    NULL,                       // reserved
    NULL                        // SetPalette
};

static DDHAL_DDEXEBUFCALLBACKS   cbDDExeBufCallbacks =
{
    sizeof( DDHAL_DDEXEBUFCALLBACKS ),
    0,                          // dwFlags
    NULL,                       // CanCreateExecuteBuffer
    NULL,                       // CreateExecuteBuffer
    NULL,                       // DestroyExecuteBuffer
    NULL,                       // LockExecuteBuffer
    NULL                        // UnlockExecuteBuffer
};

#endif // __DDINIT_H__
