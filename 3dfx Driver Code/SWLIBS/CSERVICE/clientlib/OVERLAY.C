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
** File name:   overlay.c
**
** Description: Functions for acquiring and manipulating the overlay
**
** $Log: 
**  18   3dfx      1.10.1.6    10/12/00 Don Fowler      Added type CSDCID for OS
**       independent device contexts
**       Changed all ExtEscape calls to happen from the DC of the desktop because
**       no clients currently known will be multi-monitor compatible.
**  17   3dfx      1.10.1.5    10/11/00 Brent           Forced check in to enforce
**       branch.
**  16   3dfx      1.10.1.4    10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  15   3dfx      1.10.1.3    10/09/00 Don Fowler      Removed // DWF left in from
**       previous build 
**  14   3dfx      1.10.1.2    10/09/00 Don Fowler      Changed debug system to get
**       errors from the server when the error code comes from the server. The
**       error code defines are stored in the respecitve servers header files. Call
**       csDEBUGGetErrorString to retrieve the verbose string from the server
**  13   3dfx      1.10.1.1    10/09/00 Don Fowler      Removed #ifdef
**       ENABLE_ESCAPE since we will be working with an ESCAPE enabled library from
**       now on
**  12   3dfx      1.10.1.0    07/06/00 Ryan Bissell    Incremental changes, and
**       bug fixes.
**  11   3dfx      1.10        03/19/00 Don Fowler      Added the
**       CSALLOCATIONDESCRIPTOR.u32Locale so that the client can specify
**       CS_LOCALE_FRAMEBUFFER  or CS_LOCALE_AGP separate from tiled or linear
**       memory types.
** 
**       Changed all of the error codes in the API from _ERROR_ to _APIERROR_ so
**       that server codes can have a different base 
** 
**  10   3dfx      1.9         03/09/00 Don Fowler      Modified include file
**       structure to separate out a server-specific file and a client-specific
**       file. 
**  9    3dfx      1.8         03/08/00 Don Fowler      Changed CSWINDOWID from
**       HWND to DWORD because of 16 bit in the server. Changed HDC to CSHDC which
**       is now a DWORD for the same reason.
**  8    3dfx      1.7         03/07/00 Don Fowler      Added more structures to
**       the debug display system. Added CSSERVERRES.u32ReqID so the debug system
**       would know the id of the request packet after it returns. And for future
**       use.
**  7    3dfx      1.6         03/06/00 Don Fowler      Changed the return value
**       from ExtEscape from a bool to an int because it was wrong as a bool
**  6    3dfx      1.5         03/05/00 Don Fowler      Removed the notion of a
**       device context from the code. Since a graphical context is locked to a
**       window the same as a device context it was a redundant notion. 
**  5    3dfx      1.4         03/02/00 Don Fowler      Incremental Development
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
**  Function        : csAcquireOverlay
**
**  Parameters      :  PCSGRAPHICALCONTEXT psGraphicalContext - Graphical context 
**                     PCSOVERLAY          psOverlay - Information for overlay
**
**  Purpose         : Get the overlay for the current context
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csAcquireOverlay( PCSGRAPHICALCONTEXT psGraphicalContext,
                                   PCSOVERLAY          psOverlay )
{
    PFxSz pszFunctionName = "csAcquireOverlay";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psGraphicalContext(0x%x), psOverlay(0x%x) )\n", pszFunctionName, psOverlay  );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( ( !psGraphicalContext || !psOverlay ) );
    CSDEBUG_VALIDATE_INITIALIZATION;
    CSDEBUG_VALIDATE_GRAPHICALCONTEXT_PTR( psGraphicalContext );

    /* Verify that this overlay structure is not already initialized */
    if( psOverlay->u32Flags & CSOVERLAY_FLAGS_INITIALIZED )
    {
        /* Context already initialized */
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_OVERLAYINITIALIZED::Overlay Already Initialized::0x%x\n", pszFunctionName, psOverlay );
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_OVERLAYINITIALIZED );
    }

    /* Attempt to acquire the overlay through the server */
#ifdef WIN32
    {
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;

        /* Setup the structure for the request */
        sServerReq.u32ReqID     = CSREQ_ACQUIREOVERLAY;
        memcpy( &sServerReq.sGraphicalContext, psGraphicalContext, 
            sizeof( CSGRAPHICALCONTEXT ) );

        /* Copy the descriptor structure into the request packet */
        memcpy( &sServerReq.unionReqData.sAcquireOverlayReq.sOverlay, 
            psOverlay, sizeof( CSOVERLAY ) );

        /* Display the information from the data structure */
        csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSSERVERREQ, &sServerReq ); 

        /* Call the server to make the request */
        s32Return = ExtEscape( 
            ( HDC )_sCSClientLibData.hDesktopDC,
            CS_EXTESCAPE, 
            sizeof( CSSERVERREQ ), ( LPSTR )&sServerReq, 
            sizeof( CSSERVERRES ), ( LPSTR )&sServerRes );

        /* Display the information from the data structure */
        sServerRes.u32ReqID = sServerReq.u32ReqID;
        csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSSERVERRES, &sServerRes ); 

        /* Verify the return states of the call */
        if( s32Return <= 0 )
        {
            csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_SYSTEMFAILEDEXTESCAPE::ExtEscape Failed::0x%x\n", pszFunctionName, s32Return );
            CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_SYSTEMFAILEDEXTESCAPE );
        }

        /* Verify that the server did not fail the call */
        if( sServerRes.u32Status )
        {
            csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::Server Returned Error(%x)\n", pszFunctionName, sServerRes.u32Status );
            CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, sServerRes.u32Status );
        }

        /* copy the allocation descriptor out of the request packet into the clients structure */
        /* Copy the descriptor structure into the request packet */
        memcpy( psOverlay, &sServerRes.unionResData.sAcquireOverlayRes.sOverlay, 
            sizeof( CSOVERLAY ) );
    }    
#endif /* WIN32 */    

    /* Set the flag to indicate that the overlay is initialized */
    psOverlay->u32Flags = CSOVERLAY_FLAGS_INITIALIZED;
    
    /* Store the overlay information in the graphical context so that the server
       can access the information, this will now be the master copy */
    memcpy( &psGraphicalContext->sOverlay, psOverlay, sizeof( CSOVERLAY ) );

    /* Display the information from the data structure */
    csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSOVERLAY, &psGraphicalContext->sOverlay ); 

    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}