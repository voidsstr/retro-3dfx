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
** File name:   register.c
**
** Description: Functions for manipulating hardware registers
**
** $Log: 
**  16   Napalm R1.51.8.1.6     10/12/00 Don Fowler      Added type CSDCID for OS
**       independent device contexts
**       Changed all ExtEscape calls to happen from the DC of the desktop because
**       no clients currently known will be multi-monitor compatible.
**  15   Napalm R1.51.8.1.5     10/11/00 Brent           Forced check in to enforce
**       branch.
**  14   Napalm R1.51.8.1.4     10/09/00 Don Fowler      Removed // DWF left in
**       from previous build 
**  13   Napalm R1.51.8.1.3     10/09/00 Don Fowler      Changed debug system to
**       get errors from the server when the error code comes from the server. The
**       error code defines are stored in the respecitve servers header files. Call
**       csDEBUGGetErrorString to retrieve the verbose string from the server
**  12   Napalm R1.51.8.1.2     10/09/00 Don Fowler      Removed #ifdef
**       ENABLE_ESCAPE since we will be working with an ESCAPE enabled library from
**       now on
**  11   Napalm R1.51.8.1.1     07/20/00 Andrew  Bell    Rolling back all swlibs
**       files to 2nd latest version due to an incorrect checkin.  This should
**       bring things back to normal.
**  10   Napalm R1.51.8.1.0     07/19/00 Dinesh Raja Savari Amirtharaj CSIM for the
**       RTL release_3_2
**  9    3dfx      1.8         03/09/00 Don Fowler      Modified include file
**       structure to separate out a server-specific file and a client-specific
**       file. 
**  8    3dfx      1.7         03/08/00 Don Fowler      Changed CSWINDOWID from
**       HWND to DWORD because of 16 bit in the server. Changed HDC to CSHDC which
**       is now a DWORD for the same reason.
**  7    3dfx      1.6         03/07/00 Don Fowler      Added more structures to
**       the debug display system. Added CSSERVERRES.u32ReqID so the debug system
**       would know the id of the request packet after it returns. And for future
**       use.
**  6    3dfx      1.5         03/06/00 Don Fowler      Changed the return value
**       from ExtEscape from a bool to an int because it was wrong as a bool
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
**  Function        : csRegisterRead
**
**  Parameters      : PCSGRAPHICALCONTEXT psGraphicalContext - Graphical context  
**                    FxU32 u32RegisterSpace - Register space 
**                      CS_REGSPACE_PCI	PCI configuration registers
**                      CS_REGSPACE_IO	Registers accessible via Port I/O
**                      CS_REGSPACE_2D	2D registers
**                      CS_REGSPACE_3D	3D registers
**                      CS_REGSPACE_CMD	Command Transport/AGP registers
**                    FxU32 u32RegisterOffset - Offset of register
**                    FxU32 u32IOSize - Size of the access( in bytes )
**                    FxU32* pu32RegisterValue - Variable to store the result into
**
**  Purpose         : Read a value from a hardware register
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csRegisterRead(
    PCSGRAPHICALCONTEXT psGraphicalContext,  
    FxU32 u32RegisterSpace, 
    FxU32 u32RegisterOffset,
    FxU32 u32IOSize,
    FxU32* pu32RegisterValue )
{
    PFxSz pszFunctionName = "csRegisterRead";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psGraphicalContext(0x%x), u32RegisterSpace(0x%x), u32RegisterOffset(0x%x), u32IOSize(0x%x), pu32RegisterValue(0x%x) )\n", pszFunctionName, psGraphicalContext, u32RegisterSpace, u32RegisterOffset, u32IOSize, pu32RegisterValue );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( ( !psGraphicalContext || !pu32RegisterValue || ( u32IOSize > 4 ) ) );
    CSDEBUG_VALIDATE_INITIALIZATION;
    CSDEBUG_VALIDATE_GRAPHICALCONTEXT_PTR( psGraphicalContext );
 
#ifdef WIN32
    {
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;

        /* Setup the structure for the request */
        sServerReq.u32ReqID     = CSREQ_REGISTERREAD;
        memcpy( &sServerReq.sGraphicalContext, psGraphicalContext, 
            sizeof( CSGRAPHICALCONTEXT ) );

        /* Copy the context info into the request packet */
        sServerReq.unionReqData.sRegisterReadReq.u32RegisterSpace = u32RegisterSpace;
        sServerReq.unionReqData.sRegisterReadReq.u32RegisterOffset = u32RegisterOffset;
        sServerReq.unionReqData.sRegisterReadReq.u32IOSize = u32IOSize;
    
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
            csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_ERROR_SYSTEMFAILEDEXTESCAPE::ExtEscape Failed::0x%x\n", pszFunctionName, s32Return );
            CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_ERROR_SYSTEMFAILEDEXTESCAPE );
        }

        /* Verify that the server did not fail the call */
        if( sServerRes.u32Status )
        {
            csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::Server Returned Error(%x)\n", pszFunctionName, sServerRes.u32Status );
            CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, sServerRes.u32Status );
        }

        /* Return the information from the server */
        *pu32RegisterValue = sServerRes.unionResData.sRegisterReadRes.u32RegisterValue;
    }    
#endif /* WIN32 */

    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}

/*****************************************************************************
**
**  Function        : csRegisterWrite
**
**  Parameters      : PCSGRAPHICALCONTEXT psGraphicalContext - Graphical context  
**                    FxU32 u32RegisterSpace - Register space 
**                      CS_REGSPACE_PCI	PCI configuration registers
**                      CS_REGSPACE_IO	Registers accessible via Port I/O
**                      CS_REGSPACE_2D	2D registers
**                      CS_REGSPACE_3D	3D registers
**                      CS_REGSPACE_CMD	Command Transport/AGP registers
**                    FxU32 u32RegisterOffset - Offset of register
**                    FxU32 u32IOSize - Size of the access( in bytes )
**                    FxU32 u32RegisterValue - Variable to store to the register
**
**  Purpose         : Read a value from a hardware register
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csRegisterWrite(
    PCSGRAPHICALCONTEXT psGraphicalContext,  
    FxU32 u32RegisterSpace, 
    FxU32 u32RegisterOffset,
    FxU32 u32IOSize,
    FxU32 u32RegisterValue )
{
    PFxSz pszFunctionName = "csRegisterWrite";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psGraphicalContext(0x%x), u32RegisterSpace(0x%x), u32RegisterOffset(0x%x), u32IOSize(0x%x), u32RegisterValue(0x%x) )\n", pszFunctionName, psGraphicalContext, u32RegisterSpace, u32RegisterOffset, u32IOSize, u32RegisterValue );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( ( !psGraphicalContext || ( u32IOSize > 4 ) ) );
    CSDEBUG_VALIDATE_INITIALIZATION;
    CSDEBUG_VALIDATE_GRAPHICALCONTEXT_PTR( psGraphicalContext );
 
#ifdef WIN32
    {
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;

        /* Setup the structure for the request */
        sServerReq.u32ReqID     = CSREQ_REGISTERWRITE;
        memcpy( &sServerReq.sGraphicalContext, psGraphicalContext, 
            sizeof( CSGRAPHICALCONTEXT ) );

        /* Copy the context info into the request packet */
        sServerReq.unionReqData.sRegisterWriteReq.u32RegisterSpace = u32RegisterSpace;
        sServerReq.unionReqData.sRegisterWriteReq.u32RegisterOffset = u32RegisterOffset;
        sServerReq.unionReqData.sRegisterWriteReq.u32IOSize = u32IOSize;
        sServerReq.unionReqData.sRegisterWriteReq.u32Value = u32RegisterValue;
    
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
            csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_ERROR_SYSTEMFAILEDEXTESCAPE::ExtEscape Failed::0x%x\n", pszFunctionName, s32Return );
            CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_ERROR_SYSTEMFAILEDEXTESCAPE );
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