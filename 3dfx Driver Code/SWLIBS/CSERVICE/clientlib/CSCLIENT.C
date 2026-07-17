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
** File name:   csclient.c
**
** Description: Functions that initialize and uninitialize CSCLIENT.LIB
**
** $Log: 
**  20   3dfx      1.14.1.4    10/24/00 Don Fowler      Added CSTIME structure to
**       debug system and CSGRAPHICALCONTEXT structure. De-coupled
**       CSGRAPHICALCONTEXT from CS_BUFFER_TEXTURE_HEAP type in all functions that
**       deal with this memory type. 
**  19   3dfx      1.14.1.3    10/12/00 Don Fowler      Added type CSDCID for OS
**       independent device contexts
**       Changed all ExtEscape calls to happen from the DC of the desktop because
**       no clients currently known will be multi-monitor compatible.
**  18   3dfx      1.14.1.2    10/11/00 Brent           Forced check in to enforce
**       branch.
**  17   3dfx      1.14.1.1    10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  16   3dfx      1.14.1.0    07/06/00 Ryan Bissell    Incremental changes, and
**       bug fixes.
**  15   3dfx      1.14        03/29/00 Don Fowler      Test APP: Made render
**       buffers load 640x480x24bpp images test1,2.bmp::Moved csInit and csUnInit
**       into the testdll::Added u32Locale to the csAlloc function calls
** 
**       Test DLL: Added csInit and csUnInit to the DETACH and ATTACH portions fo
**       the DLL::Removed csReleaseResourcesForProcess because the DLL is already
**       process-relative. 
** 
**       Client Library : Moved csReleaseResourcesForProcess to static. 
**  14   3dfx      1.13        03/19/00 Don Fowler      Added the
**       CSALLOCATIONDESCRIPTOR.u32Locale so that the client can specify
**       CS_LOCALE_FRAMEBUFFER  or CS_LOCALE_AGP separate from tiled or linear
**       memory types.
** 
**       Changed all of the error codes in the API from _ERROR_ to _APIERROR_ so
**       that server codes can have a different base 
** 
**  13   3dfx      1.12        03/09/00 Don Fowler      Modified include file
**       structure to separate out a server-specific file and a client-specific
**       file. 
**  12   3dfx      1.11        03/05/00 Don Fowler      Removed the notion of a
**       device context from the code. Since a graphical context is locked to a
**       window the same as a device context it was a redundant notion. 
**  11   3dfx      1.10        03/03/00 Don Fowler      Incremental development
**  10   3dfx      1.9         03/03/00 Don Fowler      Incremental development
**  9    3dfx      1.8         03/02/00 Don Fowler      Incremental development
**  8    3dfx      1.7         03/01/00 Don Fowler      Incremental development
**  7    3dfx      1.6         02/29/00 Don Fowler      Incremental development
**  6    3dfx      1.5         02/28/00 Don Fowler      Incremental development
**  5    3dfx      1.4         02/28/00 Don Fowler      Incremental development
**  4    3dfx      1.3         02/27/00 Don Fowler      Incremental development
** 
**  3    3dfx      1.2         02/27/00 Don Fowler      Incremental development
**  2    3dfx      1.1         02/27/00 Don Fowler      Incremental Development
** 
**  1    3dfx      1.0         02/27/00 Don Fowler      
** $
**
*/

#include "csclient.h"
#include "csserver.h"

static CSRESULT FX_CALL csCleanupResources( FxVOID );

/*****************************************************************************
**
**  Function        : csInit
**
**  Parameters      : PCSINIT psInit
**
**  Purpose         : Initialize CSCLIENT.LIB
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csInit( PCSINIT psInit )
{
    PFxSz pszFunctionName = "csInit";

    /* Verify that the system is not already initialized, if so then return an error */
    if( _sCSClientLibData.u32Flags & CSCLIENTLIBDATA_FLAGS_INITIALIZED )
    {
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CSINIT_APIERROR_ALREADYINITIALIZED\n", pszFunctionName );
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CSINIT_APIERROR_ALREADYINITIALIZED );
    }
        
    /* Clear out the system structure */
    memset( &_sCSClientLibData, 0x00, sizeof( CSCLIENTLIBDATA ) );

    /* Store a pointer to the initialization structure */
    _sCSClientLibData.psInit = psInit;

    /* Initialize the debugging system */
    csDEBUGInit();
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( psInit(0x%x) )\n", pszFunctionName, psInit );

    /* Display the information from the global data structure */
    csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSCLIENTLIBDATA, &_sCSClientLibData ); 

#ifdef WIN32
    {
        SYSTEMTIME sSystemTime;

        /* Get the time stamp for the current process */
        GetSystemTime( &sSystemTime );
        _sCSClientLibData.sProcessTimeStamp.u32Year = sSystemTime.wYear;
        _sCSClientLibData.sProcessTimeStamp.u32Month = sSystemTime.wMonth;
        _sCSClientLibData.sProcessTimeStamp.u32DayOfWeek = sSystemTime.wDayOfWeek;
        _sCSClientLibData.sProcessTimeStamp.u32Day = sSystemTime.wDay;
        _sCSClientLibData.sProcessTimeStamp.u32Hour = sSystemTime.wHour;
        _sCSClientLibData.sProcessTimeStamp.u32Minute = sSystemTime.wMinute;
        _sCSClientLibData.sProcessTimeStamp.u32Second = sSystemTime.wSecond;
        _sCSClientLibData.sProcessTimeStamp.u32Milliseconds = sSystemTime.wMilliseconds;
    }     

    /* Get the operating system information */     
    _sCSClientLibData.sOSVersionInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    GetVersionEx( &_sCSClientLibData.sOSVersionInfo );

    /* Get the DC of the desktop to be used for future device-specific calls */
    _sCSClientLibData.hDesktopDC = GetDC( NULL );

    /* Store a pointer to the initialization structure */
    _sCSClientLibData.psInit = psInit;
    
    /* Attempt to initialize Direct Draw.. we will need this for video mode setting
       and clip lists */
    if( DirectDrawCreate( NULL, &_sCSClientLibData.psDirectDraw, NULL )!= DD_OK )
    {
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::ERROR::CS_APIERROR_DIRECTDRAWFAILED\n", pszFunctionName );
        CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_APIERROR_DIRECTDRAWFAILED );
    }

#if 0
    /* Hook the window proc for this DLL. If there is a failure, its probably
       not crucial that the hook is installed, so just display a warning */
    if( ( _sCSClientLibData.hWindowSnoopHook = SetWindowsHookEx( WH_CALLWNDPROC, 
        csSnoopWindowProc, _sCSClientLibData.psInit->hDLLInstance, 0 ) == NULL ) )
    {
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::WARNING::SetWindowsHookEx::Failed::0x%x\n", pszFunctionName, GetLastError() );
    }


    /* Hook the keyboard proc for this DLL. If there is a failure, its probably
       not crucial that the hook is installed, so just display a warning */
    if( ( _sCSClientLibData.hKeyboardSnoopHook = SetWindowsHookEx( WH_KEYBOARD, 
        csSnoopKeyboardProc, _sCSClientLibData.psInit->hDLLInstance, 0 ) == NULL ) )
    {
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::WARNING::SetWindowsHookEx::Failed::0x%x\n", pszFunctionName, GetLastError() );
    }
#endif

    /* Set the flag to allow our filter functions to process. */
    _sCSClientLibData.u32SnoopHooksEnabled = FXTRUE;

#endif /* WIN32 */

    /* Set the CSCLIENTLIBDATA_FLAGS_INITIALIZED flag to indicate that the
       system is initialized */
    _sCSClientLibData.u32Flags |= CSCLIENTLIBDATA_FLAGS_INITIALIZED;

    /* Display the information from the global data structure */
    csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSCLIENTLIBDATA, &_sCSClientLibData ); 

    /* Return success */
    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}

/*****************************************************************************
**
**  Function        : csUnInit
**
**  Parameters      : FxVOID
**
**  Purpose         : UnInitialize CSCLIENT.LIB, performs any cleanup necessary
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

CSRESULT FX_CALL csUnInit( FxVOID )
{
    PFxSz pszFunctionName = "csUnInit";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( FxVOID )\n", pszFunctionName );

    /* Display the information from the global data structure */
    csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSCLIENTLIBDATA, &_sCSClientLibData );

    /* Validate that the system is initialized */
    CSDEBUG_VALIDATE_INITIALIZATION; 

    /* Cleanup surfaces and such that are left around */  
    csCleanupResources();

#ifdef WIN32
    /* Set the flag to prevent our filter functions from processing. This is to ensure that the
       library does not crash */
    _sCSClientLibData.u32SnoopHooksEnabled = FXFALSE;

#if 0    
    /* UnHook the window proc for this DLL. If there is a failure, its probably
       not crucial, so just display a warning */
    if( !UnhookWindowsHookEx( _sCSClientLibData.hWindowSnoopHook ) )
    {
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::WARNING::UnhookWindowsHookEx::Failed::0x%x\n", pszFunctionName, GetLastError() );
    }

    /* UnHook the keybaord proc for this DLL. If there is a failure, its probably
       not crucial, so just display a warning */
    if( !UnhookWindowsHookEx( _sCSClientLibData.hKeyboardSnoopHook ) )
    {
        csDEBUGDisplayFormatted( DEBUG_LEVEL_ERROR, "%s::WARNING::UnhookWindowsHookEx::Failed::0x%x\n", pszFunctionName, GetLastError() );
    }
#endif

    /* Release the DC of the desktop */
    ReleaseDC( NULL, _sCSClientLibData.hDesktopDC );

#endif /* WIN32 */

    /* Clear the CSCLIENTLIBDATA_FLAGS_INITIALIZED flag to indicate that the system
       is no longer initialized */
    _sCSClientLibData.u32Flags &= ~CSCLIENTLIBDATA_FLAGS_INITIALIZED;
    
    /* Return success */
    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}

/*****************************************************************************
**
**  Function        : csCleanupResources
**
**  Parameters      : FxVOID
**
**  Purpose         : Cleanup any surfaces that have not been released by this process
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

static CSRESULT FX_CALL csCleanupResources( FxVOID )
{
    PCSSLLISTNODE psGraphicalContextNode;

    PFxSz pszFunctionName = "csCleanupResources";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( FxVOID )\n", pszFunctionName );

    /* Validate that the system is initialized */
    CSDEBUG_VALIDATE_INITIALIZATION;

    /* Cleanup surfaces and such that are left around */ 
    psGraphicalContextNode = csSLLISTGetNext( &_sCSClientLibData.sRootNodeGraphicalContext );
    while( psGraphicalContextNode )
    {
        /* Release the graphical context */
        csReleaseGraphicalContext( ( PCSGRAPHICALCONTEXT )psGraphicalContextNode );

        /* Get the next graphical context */
        psGraphicalContextNode = csSLLISTGetNext( psGraphicalContextNode );
    }

    /* Return success */
    CSDEBUG_RETURN( DEBUG_LEVEL_FUNCTION, CS_SUCCESS );
}
