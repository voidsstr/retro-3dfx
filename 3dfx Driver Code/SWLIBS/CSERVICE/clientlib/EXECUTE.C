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
** File name:   execute.c
**
** Description: Functions for executing command buffers
**
** $Log: 
**  23   3dfx      1.13.1.8    10/12/00 Don Fowler      Added type CSDCID for OS
**       independent device contexts
**       Changed all ExtEscape calls to happen from the DC of the desktop because
**       no clients currently known will be multi-monitor compatible.
**  22   3dfx      1.13.1.7    10/11/00 Brent           Forced check in to enforce
**       branch.
**  21   3dfx      1.13.1.6    10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  20   3dfx      1.13.1.5    10/09/00 Don Fowler      Removed // DWF left in from
**       previous build 
**  19   3dfx      1.13.1.4    10/09/00 Don Fowler      Changed debug system to get
**       errors from the server when the error code comes from the server. The
**       error code defines are stored in the respecitve servers header files. Call
**       csDEBUGGetErrorString to retrieve the verbose string from the server
**  18   3dfx      1.13.1.3    10/09/00 Don Fowler      Removed #ifdef
**       ENABLE_ESCAPE since we will be working with an ESCAPE enabled library from
**       now on
**  17   3dfx      1.13.1.2    10/09/00 Don Fowler      Added stricter parameter
**       checking.. a bug was occurring with a null pointer in the debug system
**  16   3dfx      1.13.1.1    07/13/00 Don Fowler      Changed csExecuteCommands
**       so that the server could be responsible for adding the sentinel buffer
**       write packets to the end of the command buffer. This required the sentinel
**       serial number and the sentinel buffer allocation descriptor to be passed
**       to the api and then on to the server.
**  15   3dfx      1.13.1.0    07/06/00 Ryan Bissell    Incremental changes, and
**       bug fixes.
**  14   3dfx      1.13        03/19/00 Don Fowler      Added the
**       CSALLOCATIONDESCRIPTOR.u32Locale so that the client can specify
**       CS_LOCALE_FRAMEBUFFER  or CS_LOCALE_AGP separate from tiled or linear
**       memory types.
** 
**       Changed all of the error codes in the API from _ERROR_ to _APIERROR_ so
**       that server codes can have a different base 
** 
**  13   3dfx      1.12        03/14/00 Don Fowler      Minor changes required to
**       get buffer execution JSR and RET working.
**  12   3dfx      1.11        03/10/00 Don Fowler      Re moved csRegisterRead and
**       csRegisterWrite and added csDeviceSpecificCoummunication  to supercede
**       these functions.
** 
**       Added register reading and writing information to CSSST2
**  11   3dfx      1.10        03/09/00 Don Fowler      Modified csAlloc to take
**       u32StateSize and u32CommandSize. These changes went into the debug system
**       and into CSALLOCREQ and CSALLOCRES
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
**  5    3dfx      1.4         03/03/00 Don Fowler      Incremental development
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
**  Function        : csExecuteCommands
**
**  Parameters      : PCSGRAPHICALCONTEXT psGraphicalContext,  
**                    PCSALLOCATIONDESCRIPTOR psStateAllocationDescriptor - State commands
**                    PCSALLOCATIONDESCRIPTOR psCommandAllocationDescriptor - Graphical commands
**                    PCSALLOCATIONDESCRIPTOR psSentinelAllocationDescriptor - Sentinel buffer
**                    FxU32 u32StateOffset - State restoration buffer offset 
**                    FxU32 u32CommandOffset - Command buffer offset
**                    FxU32 u32StateSize  - Size of the state buffer
**                    FxU32 u32CommandSize - Size of the command buffer
**                    FxU32 u32SentinelSerial - Serial number to write to the sentinel buffer
**
**  Purpose         : Set the current rendering state and execute rendering commands
**                    The rendering state will only be restored if it was previously lost
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csExecuteCommands( 
    PCSGRAPHICALCONTEXT psGraphicalContext,  
    PCSALLOCATIONDESCRIPTOR psStateAllocationDescriptor,
    PCSALLOCATIONDESCRIPTOR psCommandAllocationDescriptor,
    PCSALLOCATIONDESCRIPTOR psSentinelAllocationDescriptor,
    FxU32 u32StateOffset,
    FxU32 u32CommandOffset,
    FxU32 u32StateSize,
    FxU32 u32CommandSize,
    FxU32 u32SentinelSerial )
{
    PFxSz pszFunctionName = "csExecuteCommands";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psGraphicalContext(0x%x), psStateAllocationDescriptor(0x%x), psCommandAllocationDescriptor(0x%x), psSentinelAllocationDescriptor(0x%x), u32StateOffset(0x%x), u32CommandOffset(0x%x), u32StateSize(%x), u32CommandSize(%x), u32SentinelSerial(%x) )\n", pszFunctionName, psGraphicalContext, psStateAllocationDescriptor, psCommandAllocationDescriptor, psSentinelAllocationDescriptor, u32StateOffset, u32CommandOffset, u32StateSize, u32CommandSize, u32SentinelSerial );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( ( !psGraphicalContext || !psSentinelAllocationDescriptor ) );
    CSDEBUG_VALIDATE_INITIALIZATION;
    if( psStateAllocationDescriptor )
        CSDEBUG_VALIDATE_ALLOCATIONDESCRIPTOR_PTR( psStateAllocationDescriptor );
    if( psCommandAllocationDescriptor )
        CSDEBUG_VALIDATE_ALLOCATIONDESCRIPTOR_PTR( psCommandAllocationDescriptor );
    if( psSentinelAllocationDescriptor )
        CSDEBUG_VALIDATE_ALLOCATIONDESCRIPTOR_PTR( psSentinelAllocationDescriptor );

    /* Validate that the offset is not beyond the size of the allocation descriptors memory */

#ifdef WIN32
    {
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;

        /* Setup the structure for the request */
        sServerReq.u32ReqID     = CSREQ_EXECUTECOMMANDS;
        memcpy( &sServerReq.sGraphicalContext, psGraphicalContext, 
            sizeof( CSGRAPHICALCONTEXT ) );
    
        if( psCommandAllocationDescriptor )
        {
            memcpy( &sServerReq.unionReqData.sExecuteCommandsReq.
                sCommandAllocationDescriptor, psCommandAllocationDescriptor, 
                sizeof( CSALLOCATIONDESCRIPTOR ) );
            sServerReq.unionReqData.sExecuteCommandsReq.u32CommandOffset = 
                u32CommandOffset;
            sServerReq.unionReqData.sExecuteCommandsReq.u32CommandSize = 
                u32CommandSize;
        }
        else
        {
            sServerReq.unionReqData.sExecuteCommandsReq.u32CommandOffset = 0x00;
            sServerReq.unionReqData.sExecuteCommandsReq.u32CommandSize = 0x00;
        }

        if( psStateAllocationDescriptor )
        {
            memcpy( &sServerReq.unionReqData.sExecuteCommandsReq.
                sStateAllocationDescriptor, psStateAllocationDescriptor, 
                sizeof( CSALLOCATIONDESCRIPTOR ) );
            sServerReq.unionReqData.sExecuteCommandsReq.u32StateOffset = 
                u32StateOffset;
            sServerReq.unionReqData.sExecuteCommandsReq.u32StateSize = 
                u32StateSize;
        }
        else
        {
            sServerReq.unionReqData.sExecuteCommandsReq.u32StateOffset = 0x00;
            sServerReq.unionReqData.sExecuteCommandsReq.u32StateSize = 0x00;
        }

        if( psSentinelAllocationDescriptor )
        {
            memcpy( &sServerReq.unionReqData.sExecuteCommandsReq.
                sSentinelAllocationDescriptor, psSentinelAllocationDescriptor, 
                sizeof( CSALLOCATIONDESCRIPTOR ) );
            sServerReq.unionReqData.sExecuteCommandsReq.u32SentinelSerial = 
                u32SentinelSerial;
        }

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
    }    
#endif /* WIN32 */

    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}