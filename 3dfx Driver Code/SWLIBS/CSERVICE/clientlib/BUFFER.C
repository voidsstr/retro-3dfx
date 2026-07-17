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
** File name:   buffer.c
**
** Description: Functions for allocating and releasing buffers
**
** $Log: 
**  24   3dfx      1.13.1.9    10/24/00 Don Fowler      Added CSTIME structure to
**       debug system and CSGRAPHICALCONTEXT structure. De-coupled
**       CSGRAPHICALCONTEXT from CS_BUFFER_TEXTURE_HEAP type in all functions that
**       deal with this memory type. 
**  23   3dfx      1.13.1.8    10/12/00 Don Fowler      Added buffer type
**       CS_BUFFER_TEXTUREHEAP to the allocation system. This texture type is not a
**       child of a graphical context and can be allocated any time after csInit is
**       called
**  22   3dfx      1.13.1.7    10/12/00 Don Fowler      Added type CSDCID for OS
**       independent device contexts
**       Changed all ExtEscape calls to happen from the DC of the desktop because
**       no clients currently known will be multi-monitor compatible.
**  21   3dfx      1.13.1.6    10/11/00 Brent           Forced check in to enforce
**       branch.
**  20   3dfx      1.13.1.5    10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  19   3dfx      1.13.1.4    10/09/00 Don Fowler      Removed // DWF left in from
**       previous build 
**  18   3dfx      1.13.1.3    10/09/00 Don Fowler      Changed debug system to get
**       errors from the server when the error code comes from the server. The
**       error code defines are stored in the respecitve servers header files. Call
**       csDEBUGGetErrorString to retrieve the verbose string from the server
**  17   3dfx      1.13.1.2    10/09/00 Don Fowler      Removed #ifdef
**       ENABLE_ESCAPE since we will be working with an ESCAPE enabled library from
**       now on
**  16   3dfx      1.13.1.1    10/09/00 Don Fowler      Fixed a bug with not
**       clearing the locked flag under certain circumstances
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
**  13   3dfx      1.12        03/09/00 Don Fowler      Added more error types and
**       stricter error checking to csAlloc
**  12   3dfx      1.11        03/09/00 Don Fowler      Modified include file
**       structure to separate out a server-specific file and a client-specific
**       file. 
**  11   3dfx      1.10        03/08/00 Don Fowler      Changed CSWINDOWID from
**       HWND to DWORD because of 16 bit in the server. Changed HDC to CSHDC which
**       is now a DWORD for the same reason.
**  10   3dfx      1.9         03/08/00 Don Fowler      Added an FxFAR define for
**       16 bit code. Fixed a minor type check in GetClipListFrom DirectDraw
**  9    3dfx      1.8         03/07/00 Don Fowler      Added more structures to
**       the debug display system. Added CSSERVERRES.u32ReqID so the debug system
**       would know the id of the request packet after it returns. And for future
**       use.
**  8    3dfx      1.7         03/06/00 Don Fowler      Changed the return value
**       from ExtEscape from a bool to an int because it was wrong as a bool
**  7    3dfx      1.6         03/05/00 Don Fowler      Removed the notion of a
**       device context from the code. Since a graphical context is locked to a
**       window the same as a device context it was a redundant notion. 
**  6    3dfx      1.5         03/02/00 Don Fowler      Incremental development
**  5    3dfx      1.4         03/01/00 Don Fowler      Incremental development
**  4    3dfx      1.3         02/29/00 Don Fowler      Incremental development
**  3    3dfx      1.2         02/28/00 Don Fowler      Incremental development
**  2    3dfx      1.1         02/28/00 Don Fowler      Incremental development
**  1    3dfx      1.0         02/28/00 Don Fowler      
** $
**
*/

#include "csclient.h"
#include "csserver.h"

/*****************************************************************************
**
**  Function        : csAlloc
**
**  Parameters      :  PCSGRAPHICALCONTEXT psGraphicalContext - Graphical context 
**                     PCSALLOCATIONDESCRIPTOR psAllocationDescriptor - Descriptor to 
**                      be used for future accesses to this memory
**
**  Purpose         : Allocated a buffer to be used by the client 
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csAlloc( PCSGRAPHICALCONTEXT psGraphicalContext, 
                          PCSALLOCATIONDESCRIPTOR psAllocationDescriptor )
{
    PFxSz pszFunctionName = "csAlloc";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psGraphicalContext(0x%x), psAllocationDescriptor(0x%x) )\n", pszFunctionName, psGraphicalContext, psAllocationDescriptor );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( !psAllocationDescriptor );
    CSDEBUG_VALIDATE_INITIALIZATION;

    /* Check to see if the type of memory being allocated is of CS_MEMORY_TEXTUREHEAP
       if not then the psGraphicalContext parameter must be valid */
    if( psAllocationDescriptor->u32BufferType != CS_BUFFER_TEXTUREHEAP )
    {
        CSDEBUG_VALIDATE_PARAMETERS( !psGraphicalContext );
        CSDEBUG_VALIDATE_GRAPHICALCONTEXT_PTR( psGraphicalContext );
    }
    else
    {
        /* Set the graphical context pointer to 0 to ensure that it is not used
           during in this functions scope */
        psGraphicalContext = ( PCSGRAPHICALCONTEXT )0x00;
    }

    /* Verify that this allocation descriptor structure is not already initialized */
    if( psAllocationDescriptor->u32Flags & CSALLOCATIONDESCRIPTOR_FLAGS_INITIALIZED )
    {
        /* Allocation descriptor already initialized */
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_ALLOCATIONDESCRIPTORINITIALIZED::Graphical Context Already Initialized::0x%x\n", pszFunctionName, psAllocationDescriptor );
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_ALLOCATIONDESCRIPTORINITIALIZED );
    }

    /* Validate the information in the allocation descriptor */
    if( ( psAllocationDescriptor->u32MemType < CS_MEMORY_LINEAR ) || 
        ( psAllocationDescriptor->u32MemType > CS_MEMORY_TILED ) )
    {
        /* Invalid memory type */
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSALLOC_APIERROR_INVALIDMEMORYTYPE::Invalid Memory Type::0x%x\n", pszFunctionName, psAllocationDescriptor->u32MemType );
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSALLOC_APIERROR_INVALIDMEMORYTYPE );
    }
    if( ( psAllocationDescriptor->u32Locale < CS_LOCALE_FRAMEBUFFER ) || 
        ( psAllocationDescriptor->u32Locale > CS_LOCALE_AGP ) )
    {
        /* Invalid memory type */
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSALLOC_APIERROR_INVALIDLOCALE::Invalid Locale::0x%x\n", pszFunctionName, psAllocationDescriptor->u32Locale );
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSALLOC_APIERROR_INVALIDLOCALE );
    }

    if( ( psAllocationDescriptor->u32BufferType < CS_BUFFER_FIFO ) || 
        ( psAllocationDescriptor->u32BufferType > CS_BUFFER_ZBUFFER ) )
    {
        /* Invalid memory type */
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSALLOC_APIERROR_INVALIDBUFFERTYPE::Invalid Buffer Type::0x%x\n", pszFunctionName, psAllocationDescriptor->u32BufferType );
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSALLOC_APIERROR_INVALIDBUFFERTYPE );
    }

    /* Attempt to allocate the descriptor through the server */
#ifdef WIN32
    {
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;

        /* Setup the structure for the request */
        sServerReq.u32ReqID     = CSREQ_ALLOC;
        if( psGraphicalContext != ( PCSGRAPHICALCONTEXT )0x00 )
        {
            memcpy( &sServerReq.sGraphicalContext, psGraphicalContext, 
                sizeof( CSGRAPHICALCONTEXT ) );
        }
        else
        {
            memset( &sServerReq.sGraphicalContext, 0x00, sizeof( CSGRAPHICALCONTEXT ) );

            // store the process handle and time stamp of the current process in the 
            // graphical context so that the memory allocated can be associated with this process
            // and set the flags to zero to ensure that the server knows this 
            // is not a valid graphical context
            sServerReq.sGraphicalContext.idProcess = ( CSPROCESSID )GetCurrentProcess();
            sServerReq.sGraphicalContext.u32Flags = 0x00;
            memcpy( &psGraphicalContext->sProcessTimeStamp, &_sCSClientLibData.sProcessTimeStamp, 
                sizeof( CSTIME ) );
        }

        /* Copy the descriptor structure into the request packet */
        memcpy( &sServerReq.unionReqData.sAllocReq.sAllocationDescriptor, 
            psAllocationDescriptor, sizeof( CSALLOCATIONDESCRIPTOR ) );

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
        memcpy( psAllocationDescriptor, &sServerRes.unionResData.sAllocRes.sAllocationDescriptor, 
            sizeof( CSALLOCATIONDESCRIPTOR ) );
    }    
#endif /* WIN32 */    

    /* Initialize any fields that may be volatile */
    psAllocationDescriptor->u32LockCount = 0x00;

    /* Set the flag to indicate that the allocation descriptor is initialized */
    psAllocationDescriptor->u32Flags = CSALLOCATIONDESCRIPTOR_FLAGS_INITIALIZED;

    /* Add this structure to the linked list of allocation descriptors stored relative to the
       graphical context.. however buffer type CS_BUFFER_TEXTUREHEAP is not to be a child of
       a graphical context and must not be added to this list */
    if( psGraphicalContext != ( PCSGRAPHICALCONTEXT )0x00 )
    {
        csSLLISTInsert( ( PCSSLLISTNODE )&psGraphicalContext->sRootNodeAllocationDescriptor, 
            ( PCSSLLISTNODE )psAllocationDescriptor );
    }

    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}

/*****************************************************************************
**
**  Function        : csFree
**
**  Parameters      :  PCSGRAPHICALCONTEXT psGraphicalContext - Graphical context 
**                     PCSALLOCATIONDESCRIPTOR psAllocationDescriptor - Descriptor 
**                      created by csAlloc for this memory
**
**  Purpose         : Release a buffer allocated by the client
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csFree( PCSGRAPHICALCONTEXT psGraphicalContext, 
                         PCSALLOCATIONDESCRIPTOR psAllocationDescriptor )
{
    PFxSz pszFunctionName = "csFree";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psGraphicalContext(0x%x), psAllocationDescriptor(0x%x) )\n", pszFunctionName, psGraphicalContext, psAllocationDescriptor );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( !psAllocationDescriptor );
    CSDEBUG_VALIDATE_INITIALIZATION;
    CSDEBUG_VALIDATE_ALLOCATIONDESCRIPTOR_PTR( psAllocationDescriptor );

    /* Check to see if the type of memory being allocated is of CS_MEMORY_TEXTUREHEAP
       if not then the psGraphicalContext parameter must be valid */
    if( psAllocationDescriptor->u32BufferType != CS_BUFFER_TEXTUREHEAP )
    {
        CSDEBUG_VALIDATE_PARAMETERS( !psGraphicalContext );
        CSDEBUG_VALIDATE_GRAPHICALCONTEXT_PTR( psGraphicalContext );
    }
    else
    {
        /* Set the graphical context pointer to 0 to ensure that it is not used
           during in this functions scope */
        psGraphicalContext = ( PCSGRAPHICALCONTEXT )0x00;
    }

    /* Verify that the memory is not currently locked, if so then return an error */
    if( psAllocationDescriptor->u32LockCount )
    {
        /* Allocation descriptor still locked */
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSFREE_APIERROR_STILLLOCKED::csFree:Allocation descriptor still locked::0x%x\n", pszFunctionName, psAllocationDescriptor );
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSFREE_APIERROR_STILLLOCKED );
    }

    /* Attempt to free the descriptor through the server */
#ifdef WIN32
    {
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;

        /* Setup the structure for the request */
        sServerReq.u32ReqID     = CSREQ_FREE;
        if( psGraphicalContext != ( PCSGRAPHICALCONTEXT )0x00 )
        {
            memcpy( &sServerReq.sGraphicalContext, psGraphicalContext, 
                sizeof( CSGRAPHICALCONTEXT ) );
        }
        else
        {
            memset( &sServerReq.sGraphicalContext, 0x00, sizeof( CSGRAPHICALCONTEXT ) );

            // store the process handle and time stamp of the current process in the 
            // graphical context so that the memory allocated can be associated with this process
            // and set the flags to zero to ensure that the server knows this 
            // is not a valid graphical context
            sServerReq.sGraphicalContext.idProcess = ( CSPROCESSID )GetCurrentProcess();
            sServerReq.sGraphicalContext.u32Flags = 0x00;
            memcpy( &psGraphicalContext->sProcessTimeStamp, &_sCSClientLibData.sProcessTimeStamp, 
                sizeof( CSTIME ) );
        }

        /* Copy the descriptor structure into the request packet */
        memcpy( &sServerReq.unionReqData.sFreeReq.sAllocationDescriptor, 
            psAllocationDescriptor, sizeof( CSALLOCATIONDESCRIPTOR ) );

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

        /* Copy the allocation descriptor out of the request packet into the clients structure */
        memcpy( psAllocationDescriptor, &sServerRes.unionResData.sFreeRes.sAllocationDescriptor, 
            sizeof( CSALLOCATIONDESCRIPTOR ) );
    }    
#endif /* WIN32 */ 

    /* Reset the initialized flag for this descriptor */
    psAllocationDescriptor->u32Flags &= ~CSALLOCATIONDESCRIPTOR_FLAGS_INITIALIZED;

    /* Remove this allocation descriptor structure from the list of structures 
       This must not be attempted for buffer type CS_BUFFER_TEXTUREHEAP because this
       memory type is not a child of graphical context */
    if( psGraphicalContext != ( PCSGRAPHICALCONTEXT )0x00 )
    {
        csSLLISTRemove( ( PCSSLLISTNODE )&psGraphicalContext->sRootNodeAllocationDescriptor,
            ( PCSSLLISTNODE )psAllocationDescriptor );
    }

    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}

/*****************************************************************************
**
**  Function        : csLock
**
**  Parameters      :  PCSGRAPHICALCONTEXT psGraphicalContext - Graphical context 
**                     PCSALLOCATIONDESCRIPTOR psAllocationDescriptor - Descriptor 
**                      created by csAlloc for this memory
**
**  Purpose         : Lock a buffer to get a pointer to it. This buffer memory may
**                      only be accessed while this lock is held.  
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csLock( PCSGRAPHICALCONTEXT psGraphicalContext, 
                         PCSALLOCATIONDESCRIPTOR psAllocationDescriptor )
{
    PFxSz pszFunctionName = "csLock";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psGraphicalContext(0x%x), psAllocationDescriptor(0x%x) )\n", pszFunctionName, psGraphicalContext, psAllocationDescriptor );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( !psAllocationDescriptor );
    CSDEBUG_VALIDATE_INITIALIZATION;
    CSDEBUG_VALIDATE_ALLOCATIONDESCRIPTOR_PTR( psAllocationDescriptor );

    /* Check to see if the type of memory being locked is of CS_MEMORY_TEXTUREHEAP
       if not then the psGraphicalContext parameter must be valid */
    if( psAllocationDescriptor->u32BufferType != CS_BUFFER_TEXTUREHEAP )
    {
        CSDEBUG_VALIDATE_PARAMETERS( !psGraphicalContext );
        CSDEBUG_VALIDATE_GRAPHICALCONTEXT_PTR( psGraphicalContext );
    }
    else
    {
        /* Set the graphical context pointer to 0 to ensure that it is not used
           during in this functions scope */
        psGraphicalContext = ( PCSGRAPHICALCONTEXT )0x00;
    }

    /* If the memory is already locked then increment the lock count and leave */
    if( psAllocationDescriptor->u32Flags & CSALLOCATIONDESCRIPTOR_FLAGS_LOCKED )
    {
        psAllocationDescriptor->u32LockCount++;
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
    }

    /* Attempt to lock the memory through the server */
#ifdef WIN32
    {
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;

        /* Setup the structure for the request */
        sServerReq.u32ReqID     = CSREQ_LOCK;
        if( psGraphicalContext != ( PCSGRAPHICALCONTEXT )0x00 )
        {
            memcpy( &sServerReq.sGraphicalContext, psGraphicalContext, 
                sizeof( CSGRAPHICALCONTEXT ) );
        }
        else
        {
            memset( &sServerReq.sGraphicalContext, 0x00, sizeof( CSGRAPHICALCONTEXT ) );

            // store the process handle and time stamp of the current process in the 
            // graphical context so that the memory allocated can be associated with this process
            // and set the flags to zero to ensure that the server knows this 
            // is not a valid graphical context
            sServerReq.sGraphicalContext.idProcess = ( CSPROCESSID )GetCurrentProcess();
            sServerReq.sGraphicalContext.u32Flags = 0x00;
            memcpy( &psGraphicalContext->sProcessTimeStamp, &_sCSClientLibData.sProcessTimeStamp, 
                sizeof( CSTIME ) );
        }

        /* Copy the descriptor structure into the request packet */
        memcpy( &sServerReq.unionReqData.sLockReq.sAllocationDescriptor, 
            psAllocationDescriptor, sizeof( CSALLOCATIONDESCRIPTOR ) );

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
        memcpy( psAllocationDescriptor, &sServerRes.unionResData.sLockRes.sAllocationDescriptor, 
            sizeof( CSALLOCATIONDESCRIPTOR ) );
    }    
#endif /* WIN32 */    

    /* Set the flag to indicate that the allocation descriptor is locked */
    psAllocationDescriptor->u32Flags |= CSALLOCATIONDESCRIPTOR_FLAGS_LOCKED;

    /* Increment the lock count for the allocation descriptor */
    psAllocationDescriptor->u32LockCount++;

    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}

/*****************************************************************************
**
**  Function        : csUnlock
**
**  Parameters      :  PCSGRAPHICALCONTEXT psGraphicalContext - Graphical context 
**                     PCSALLOCATIONDESCRIPTOR psAllocationDescriptor - Descriptor 
**                      created by csAlloc for this memory
**
**  Purpose         : Unlock a buffer to release a pointer to it. This buffer may not be
**                      accessed while no lock is held 
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csUnlock( PCSGRAPHICALCONTEXT psGraphicalContext, 
                         PCSALLOCATIONDESCRIPTOR psAllocationDescriptor )
{
    PFxSz pszFunctionName = "csUnLock";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psGraphicalContext(0x%x), psAllocationDescriptor(0x%x) )\n", pszFunctionName, psGraphicalContext, psAllocationDescriptor );

    /* Verify function entry */
    CSDEBUG_VALIDATE_PARAMETERS( !psAllocationDescriptor );
    CSDEBUG_VALIDATE_INITIALIZATION;
    CSDEBUG_VALIDATE_ALLOCATIONDESCRIPTOR_PTR( psAllocationDescriptor );

    /* Check to see if the type of memory being unlocked is of CS_MEMORY_TEXTUREHEAP
       if not then the psGraphicalContext parameter must be valid */
    if( psAllocationDescriptor->u32BufferType != CS_BUFFER_TEXTUREHEAP )
    {
        CSDEBUG_VALIDATE_PARAMETERS( !psGraphicalContext );
        CSDEBUG_VALIDATE_GRAPHICALCONTEXT_PTR( psGraphicalContext );
    }
    else
    {
        /* Set the graphical context pointer to 0 to ensure that it is not used
           during in this functions scope */
        psGraphicalContext = ( PCSGRAPHICALCONTEXT )0x00;
    }

    /* If the memory is not locked then return an error indicating that is not locked */
    if( !( psAllocationDescriptor->u32Flags & CSALLOCATIONDESCRIPTOR_FLAGS_LOCKED ) )
    {
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSUNLOCK_APIERROR_NOTLOCKED\n", pszFunctionName );
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSUNLOCK_APIERROR_NOTLOCKED );
    }

    /* Decrement the lock count, if it doesn't goes to 0 then return */
    if( !psAllocationDescriptor->u32LockCount || --psAllocationDescriptor->u32LockCount )
    {
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
    }

    /* Attempt to unlock the descriptor through the server */
#ifdef WIN32
    {
        int s32Return;
        CSSERVERREQ sServerReq;
        CSSERVERRES sServerRes;

        /* Setup the structure for the request */
        sServerReq.u32ReqID     = CSREQ_UNLOCK;
        if( psGraphicalContext != ( PCSGRAPHICALCONTEXT )0x00 )
        {
            memcpy( &sServerReq.sGraphicalContext, psGraphicalContext, 
                sizeof( CSGRAPHICALCONTEXT ) );
        }
        else
        {
            memset( &sServerReq.sGraphicalContext, 0x00, sizeof( CSGRAPHICALCONTEXT ) );

            // store the process handle and time stamp of the current process in the 
            // graphical context so that the memory allocated can be associated with this process
            // and set the flags to zero to ensure that the server knows this 
            // is not a valid graphical context
            sServerReq.sGraphicalContext.idProcess = ( CSPROCESSID )GetCurrentProcess();
            sServerReq.sGraphicalContext.u32Flags = 0x00;
            memcpy( &psGraphicalContext->sProcessTimeStamp, &_sCSClientLibData.sProcessTimeStamp, 
                sizeof( CSTIME ) );
        }

        /* Copy the descriptor structure into the request packet */
        memcpy( &sServerReq.unionReqData.sUnLockReq.sAllocationDescriptor, 
            psAllocationDescriptor, sizeof( CSALLOCATIONDESCRIPTOR ) );

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

        /* Increment the lock count for these checks to ensure that a failure leaves the 
           descriptor logically locked, if there is not failure then the following memcpy 
           will restore the lock count.. which is 0 for this case */
        psAllocationDescriptor->u32LockCount++;

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

        /* Copy the allocation descriptor out of the request packet into the clients structure */
        memcpy( psAllocationDescriptor, &sServerRes.unionResData.sUnLockRes.sAllocationDescriptor, 
            sizeof( CSALLOCATIONDESCRIPTOR ) );

        // If the lock count has gone to zero then clear the CSALLOCATIONDESCRIPTOR_FLAGS_LOCKED flag
        if( psAllocationDescriptor->u32LockCount == 0x00 )
        {
            psAllocationDescriptor->u32Flags &= ~CSALLOCATIONDESCRIPTOR_FLAGS_LOCKED;    
        }
    }    
#endif /* WIN32 */  

    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}