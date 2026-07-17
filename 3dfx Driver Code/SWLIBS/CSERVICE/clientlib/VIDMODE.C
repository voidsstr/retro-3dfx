/*
** Copyright (c) 1996-2000, 3Dfx Interactive, Inc.
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
** File name:   vidmode.c
**
** Description: Functions for allocating and releasing buffers
**
** $Log: 
**  10   3dfx      1.6.1.2     10/11/00 Brent           Forced check in to enforce
**       branch.
**  9    3dfx      1.6.1.1     10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  8    3dfx      1.6.1.0     07/06/00 Ryan Bissell    Incremental changes, and
**       bug fixes.
**  7    3dfx      1.6         03/09/00 Don Fowler      Modified include file
**       structure to separate out a server-specific file and a client-specific
**       file. 
**  6    3dfx      1.5         03/08/00 Don Fowler      Changed CSWINDOWID from
**       HWND to DWORD because of 16 bit in the server. Changed HDC to CSHDC which
**       is now a DWORD for the same reason.
**  5    3dfx      1.4         03/05/00 Don Fowler      Removed the notion of a
**       device context from the code. Since a graphical context is locked to a
**       window the same as a device context it was a redundant notion. 
**  4    3dfx      1.3         03/02/00 Don Fowler      Incremental development
**  3    3dfx      1.2         03/01/00 Don Fowler      Incremental development
**  2    3dfx      1.1         02/29/00 Don Fowler      Incremental development
**  1    3dfx      1.0         02/28/00 Don Fowler      
** $
**
*/

#include "csclient.h"
#include "csserver.h"

/*****************************************************************************
**
**  Function        : csSetVideoMode
**
**  Parameters      :  PCSGRAPHICALCONTEXT psGraphicalContext - Graphical context 
**                     PCSVIDEOMODE psVideoMode - Information needed to set the video
**                      mode.
**
**  Purpose         : Set the video mode using the information in psVideoMode 
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csSetVideoMode( PCSGRAPHICALCONTEXT psGraphicalContext,  
                                 PCSVIDEOMODE        psVideoMode,
                                 CSWINDOWID          idWindow )
{
    PFxSz pszFunctionName = "csSetVideoMode";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psGraphicalContext(0x%x), psVideoMode(0x%x) )\n", pszFunctionName, psVideoMode  );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( ( !psGraphicalContext || !psVideoMode  || !idWindow || !IsWindow( ( HWND )idWindow) ) );
    CSDEBUG_VALIDATE_INITIALIZATION;
    CSDEBUG_VALIDATE_GRAPHICALCONTEXT_PTR( psGraphicalContext );

#ifdef WIN32
    {
        FxU32 hresult;

        // set cooperation level to windowed mode normal
        hresult = _sCSClientLibData.psDirectDraw->lpVtbl->SetCooperativeLevel(_sCSClientLibData.psDirectDraw, ( HWND )idWindow,
                      DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE );

        // set the display mode
        _sCSClientLibData.psDirectDraw->lpVtbl->SetDisplayMode(_sCSClientLibData.psDirectDraw,640,480,16);
    }
#endif /* WIN32 */

    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}