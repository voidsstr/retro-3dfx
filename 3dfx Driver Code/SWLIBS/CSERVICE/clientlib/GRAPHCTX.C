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
** File name:   graphctx.c
**
** Description: Graphical context functions for CSCLIENT.LIB
**
** $Log: 
**  23   3dfx      1.11.1.10   10/24/00 Don Fowler      Added CSTIME structure to
**       debug system and CSGRAPHICALCONTEXT structure. De-coupled
**       CSGRAPHICALCONTEXT from CS_BUFFER_TEXTURE_HEAP type in all functions that
**       deal with this memory type. 
**  22   3dfx      1.11.1.9    10/12/00 Don Fowler      Added type CSDCID for OS
**       independent device contexts
**       Changed all ExtEscape calls to happen from the DC of the desktop because
**       no clients currently known will be multi-monitor compatible.
**  21   3dfx      1.11.1.8    10/11/00 Brent           Forced check in to enforce
**       branch.
**  20   3dfx      1.11.1.7    10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  19   3dfx      1.11.1.6    10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  18   3dfx      1.11.1.5    10/09/00 Don Fowler      Removed // DWF left in from
**       previous build 
**  17   3dfx      1.11.1.4    10/09/00 Don Fowler      Changed debug system to get
**       errors from the server when the error code comes from the server. The
**       error code defines are stored in the respecitve servers header files. Call
**       csDEBUGGetErrorString to retrieve the verbose string from the server
**  16   3dfx      1.11.1.3    10/09/00 Don Fowler      Removed #ifdef
**       ENABLE_ESCAPE since we will be working with an ESCAPE enabled library from
**       now on
**  15   3dfx      1.11.1.2    10/09/00 Don Fowler      Added support for the
**       CSSST1 chip type.
**  14   3dfx      1.11.1.1    07/20/00 Ryan Bissell    Let the server assign the
**       context IDs.
**  13   3dfx      1.11.1.0    07/06/00 Ryan Bissell    Incremental changes, and
**       bug fixes.
**  12   3dfx      1.11        03/19/00 Don Fowler      Added the
**       CSALLOCATIONDESCRIPTOR.u32Locale so that the client can specify
**       CS_LOCALE_FRAMEBUFFER  or CS_LOCALE_AGP separate from tiled or linear
**       memory types.
** 
**       Changed all of the error codes in the API from _ERROR_ to _APIERROR_ so
**       that server codes can have a different base 
** 
**  11   3dfx      1.10        03/09/00 Don Fowler      Added stricture error
**       checking on get graphical context
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
**  5    3dfx      1.4         03/02/00 Don Fowler      Incremental development
**  4    3dfx      1.3         03/01/00 Don Fowler      Incremental development
**  3    3dfx      1.2         02/29/00 Don Fowler      Incremental development
**  2    3dfx      1.1         02/28/00 Don Fowler      Incremental development
**  1    3dfx      1.0         02/28/00 Don Fowler      
** $
**
*/

#include "csclient.h"
#include "csserver.h"

/*****************************************************************************
**
**  Function        : csGetGraphicalContext
**
**  Parameters      : CSDCID idDeviceContext - ID of the device to communicate
**                          with.
**                    PCSGRAPHICALCONTEXT psGraphicalContext - Structure to 
**                      fill with the graphical context data
**
**  Purpose         : Creates a CS graphical context for a specific HDC
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csGetGraphicalContext( CSDCID idDeviceContext,
                                        PCSGRAPHICALCONTEXT psGraphicalContext )
{
    PFxSz pszFunctionName = "csGetGraphicalContext";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( idDeviceContext(0x%x), psGraphicalContext(0x%x) )\n", pszFunctionName, idDeviceContext, psGraphicalContext );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( ( !psGraphicalContext ) );
    CSDEBUG_VALIDATE_INITIALIZATION;

    /* Verify that this graphical context structure is not already initialized */
    if( psGraphicalContext->u32Flags & CSGRAPHICALCONTEXT_FLAGS_INITIALIZED )
    {
        /* Context already initialized */
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_INVALIDPARAM::Graphical Context Already Initialized::0x%x\n", pszFunctionName, psGraphicalContext );
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_GRAPHICALCONTEXTINITIALIZED );
    }


#ifdef WIN32
    {
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;

        /* Store the relevant information into the graphical context */
        psGraphicalContext->idProcess = GetCurrentProcessId(); 
        psGraphicalContext->idThread = GetCurrentThreadId();
        psGraphicalContext->idWindow = ( CSWINDOWID )0;  //not used on WIN32 platforms.

        psGraphicalContext->u32ContextID = 0; //server will initialize this to a valid, unique value.

        /* Save the DC of this CS context, to be used for future device-specific calls */
        psGraphicalContext->hDC = idDeviceContext;
        memcpy( &psGraphicalContext->sProcessTimeStamp, &_sCSClientLibData.sProcessTimeStamp, 
            sizeof( CSTIME ) );

        /* Create a direct draw clipper object and associate with this CS context. This will allow
           the code to get a clip list on the csSwapBufferToDisplay call */
        {
            HRESULT hResult;

            /* Create the clipper object for this graphical context( window ) */
            if( ( hResult = _sCSClientLibData.psDirectDraw->lpVtbl->CreateClipper(
                _sCSClientLibData.psDirectDraw, 0, &psGraphicalContext->psDirectDrawClipper, NULL ) 
                ) != DD_OK ) 
            {
                /* Can't create the direct draw clipper object */
                csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSGETGRAPHICALCONTEXT_APIERROR_CANTCREATECLIPPER::Unable To Create DirectDraw Clipper::0x%x\n", pszFunctionName, hResult );
                CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSGETGRAPHICALCONTEXT_APIERROR_CANTCREATECLIPPER );
            }
        }

        /* Setup the structure for the request */
        sServerReq.u32ReqID = CSREQ_GETGRAPHICALCONTEXT;
        memcpy( &sServerReq.unionReqData.sGetGraphicalContextReq.sGraphicalContext, psGraphicalContext, 
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

        /* Copy the information from the result packet into the context structure */
        memcpy( psGraphicalContext, &sServerRes.unionResData.sGetGraphicalContextRes.sGraphicalContext,
            sizeof( CSGRAPHICALCONTEXT ) );
    }    
#endif /* WIN32 */

    /* Verify that all of the fields are filled in correctly */
    if( ( psGraphicalContext->sChipSpecificData.u32ChipType < CSCHIPSPECIFICDATA_CHIPTYPE_SST1 ) ||
        ( psGraphicalContext->sChipSpecificData.u32ChipType > CSCHIPSPECIFICDATA_CHIPTYPE_SST2 ) )
    {
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_INVALIDCHIPTYPE::Invalid Chip Type::0x%x\n", pszFunctionName, psGraphicalContext->sChipSpecificData.u32ChipType );
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_INVALIDCHIPTYPE );
    }

    /* Get the current video mode information from the server and save it in sPrevVideoMode */

    /* Structure is initialized now */
    psGraphicalContext->u32Flags = CSGRAPHICALCONTEXT_FLAGS_INITIALIZED;

    /* Clear the overlay information structure */
    psGraphicalContext->sOverlay.u32Flags = 0x00;

    /* Add this structure to the linked list of graphical contexts stored relative to the
       CS client lib global data structure */
    csSLLISTInsert( ( PCSSLLISTNODE )&_sCSClientLibData.sRootNodeGraphicalContext, 
        ( PCSSLLISTNODE )psGraphicalContext );

    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}

/*****************************************************************************
**
**  Function        : csReleaseGraphicalContext
**
**  Parameters      : PCSGRAPHICALCONTEXT psGraphicalContext - Context to release
**
**  Purpose         : Released a context acquired by csGetGraphicalContext
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csReleaseGraphicalContext( PCSGRAPHICALCONTEXT psGraphicalContext )
{
    PCSSLLISTNODE psAllocationDescriptorNode;

    PFxSz pszFunctionName = "csReleaseGraphicalContext";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psGraphicalContext(0x%x) )\n", pszFunctionName, psGraphicalContext );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( ( !psGraphicalContext ) );
    CSDEBUG_VALIDATE_INITIALIZATION;
    CSDEBUG_VALIDATE_GRAPHICALCONTEXT_PTR( psGraphicalContext );

    /* Traverse the list of allocation descriptors, releasing each one */
    psAllocationDescriptorNode = csSLLISTGetNext( &psGraphicalContext->sRootNodeAllocationDescriptor );
    while( psAllocationDescriptorNode )
    {
        /* Release the graphical context, set the lock count to 0 to ensure it's freed memory */
        ( ( PCSALLOCATIONDESCRIPTOR )psAllocationDescriptorNode )->u32LockCount = 0x00;
        csFree( psGraphicalContext, ( PCSALLOCATIONDESCRIPTOR )psAllocationDescriptorNode );

        /* Get the next graphical context */
        psAllocationDescriptorNode = csSLLISTGetNext( psAllocationDescriptorNode );
    }

#ifdef WIN32
    {
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;

        /* Release the clipper object. Unfortunately, there doesn't seem to be a mechanism 
           to do this without destroying the main DirectDraw object. May need to have a 
           DirectDraw object for each graphical context and destroy it here. */

        /* Setup the structure for the request */
        sServerReq.u32ReqID     = CSREQ_RELEASEGRAPHICALCONTEXT;
        memcpy( &sServerReq.sGraphicalContext, psGraphicalContext, 
            sizeof( CSGRAPHICALCONTEXT ) );

        /* Copy the context info into the request packet */
        memcpy( &sServerReq.unionReqData.sReleaseGraphicalContextReq.sGraphicalContext,
            psGraphicalContext, sizeof( CSGRAPHICALCONTEXT ) );
    
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

    /* Do we need to restore the video mode for each closed graphical context, or do we
       only do this if the owning process is shutting down and the mode wasn't restored */

    /* Structure is no longer intitialized here */ 
    psGraphicalContext->u32Flags = 0x00;

    /* Remove this graphical context structure from the list of structures */
    csSLLISTRemove( ( PCSSLLISTNODE )&_sCSClientLibData.sRootNodeGraphicalContext,
        ( PCSSLLISTNODE )psGraphicalContext );

    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}