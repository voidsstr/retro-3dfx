/* $Header: devcomm.c, 9, 10/12/00 1:17:27 PM, Don Fowler$ */
/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** File name:   cstypes.h
**
** Description: Types for the entire CS system
**
** $Log: 
**  9    3dfx      1.1.1.6     10/12/00 Don Fowler      Added type CSDCID for OS
**       independent device contexts
**       Changed all ExtEscape calls to happen from the DC of the desktop because
**       no clients currently known will be multi-monitor compatible.
**  8    3dfx      1.1.1.5     10/11/00 Brent           Forced check in to enforce
**       branch.
**  7    3dfx      1.1.1.4     10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  6    3dfx      1.1.1.3     10/09/00 Don Fowler      Removed // DWF left in from
**       previous build 
**  5    3dfx      1.1.1.2     10/09/00 Don Fowler      Changed debug system to get
**       errors from the server when the error code comes from the server. The
**       error code defines are stored in the respecitve servers header files. Call
**       csDEBUGGetErrorString to retrieve the verbose string from the server
**  4    3dfx      1.1.1.1     10/09/00 Don Fowler      Removed #ifdef
**       ENABLE_ESCAPE since we will be working with an ESCAPE enabled library from
**       now on
**  3    3dfx      1.1.1.0     07/06/00 Ryan Bissell    Incremental changes, and
**       bug fixes.
**  2    3dfx      1.1         03/19/00 Don Fowler      Added the
**       CSALLOCATIONDESCRIPTOR.u32Locale so that the client can specify
**       CS_LOCALE_FRAMEBUFFER  or CS_LOCALE_AGP separate from tiled or linear
**       memory types.
** 
**       Changed all of the error codes in the API from _ERROR_ to _APIERROR_ so
**       that server codes can have a different base 
** 
**  1    3dfx      1.0         03/13/00 Don Fowler      
** $
**
*/

#include "csclient.h"
#include "csserver.h"


/*****************************************************************************
**
**  Function        : csDeviceSpecificCommunication
**
**  Parameters      : PCSGRAPHICALCONTEXT psGraphicalContext - Graphical context   
**                    FxU32 u32MessageID - ID of the device specific message      
**
**  Purpose         : Send a device specific message
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csDeviceSpecificCommunication( PCSGRAPHICALCONTEXT psGraphicalContext,
    FxU32 u32MessageID )
{
    PFxSz pszFunctionName = "csDeviceSpecificCommunication";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psGraphicalContext(0x%x), u32MessageID(0x%x) )\n", pszFunctionName, psGraphicalContext, u32MessageID );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( ( !psGraphicalContext ) );
    CSDEBUG_VALIDATE_INITIALIZATION;
    CSDEBUG_VALIDATE_GRAPHICALCONTEXT_PTR( psGraphicalContext );

#ifdef WIN32
    {
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;

        /* Setup the structure for the request */
        sServerReq.u32ReqID     = u32MessageID;
        memcpy( &sServerReq.sGraphicalContext, psGraphicalContext, 
            sizeof( CSGRAPHICALCONTEXT ) );

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

        /* Copy the graphical context structure out of the request packet */
        memcpy( psGraphicalContext, &sServerRes.sGraphicalContext, sizeof( CSGRAPHICALCONTEXT) );
    }    
#endif /* WIN32 */   

    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}
