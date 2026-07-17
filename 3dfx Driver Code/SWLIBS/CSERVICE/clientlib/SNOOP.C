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
** File name:   snoop.c
**
** Description: Functions for snooping and getting realtime information
**
** $Log: 
**  11   3dfx      1.7.1.2     10/11/00 Brent           Forced check in to enforce
**       branch.
**  10   3dfx      1.7.1.1     10/11/00 Ryan Bissell    Modifications needed for
**       OGL.
**  9    3dfx      1.7.1.0     07/06/00 Ryan Bissell    Incremental changes, and
**       bug fixes.
**  8    3dfx      1.7         03/19/00 Don Fowler      Added the
**       CSALLOCATIONDESCRIPTOR.u32Locale so that the client can specify
**       CS_LOCALE_FRAMEBUFFER  or CS_LOCALE_AGP separate from tiled or linear
**       memory types.
** 
**       Changed all of the error codes in the API from _ERROR_ to _APIERROR_ so
**       that server codes can have a different base 
** 
**  7    3dfx      1.6         03/09/00 Don Fowler      Modified include file
**       structure to separate out a server-specific file and a client-specific
**       file. 
**  6    3dfx      1.5         03/05/00 Don Fowler      Removed the notion of a
**       device context from the code. Since a graphical context is locked to a
**       window the same as a device context it was a redundant notion. 
**  5    3dfx      1.4         03/03/00 Don Fowler      Incremental development
**  4    3dfx      1.3         03/03/00 Don Fowler      Incremental development
**  3    3dfx      1.2         03/02/00 Don Fowler      Incremental Development
**  2    3dfx      1.1         03/02/00 Don Fowler      Incremental development
**  1    3dfx      1.0         03/02/00 Don Fowler      
** $
**
*/

#include "csclient.h"
#include "csserver.h"

#ifdef WIN32

/*****************************************************************************
**
**  Function        : csSnoopWinowProc
**
**  Parameters      : Standard window types
**
**  Purpose         : Filters window messages to perform realtime actions
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

LRESULT CALLBACK csSnoopWindowProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    CWPSTRUCT * lpCPW = ( CWPSTRUCT * )lParam;
//    PCSGRAPHICALCONTEXT psGraphicalContext;
//    PCSSLLISTNODE       psNode;
    LRESULT lResult;
    PFxSz pszFunctionName = "csSnoopWindowProc";
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( nCode(0x%x), wParam(0x%x), lParam(0x%x) )\n", pszFunctionName, nCode, wParam, lParam );

#if 0
    /* If the flag to disallow processing is set then simply return */
    if( !_sCSClientLibData.u32SnoopHooksEnabled )
    {
        /* Return from snoop function */
       CSDEBUG_RETURN_NOERROR( DEBUG_LEVEL_FUNCTION, 0x00 );
    }

    /* Get the graphical context associated with this window. The graphical contexts are all
       in a linked list off of the main structure for this library */
    psNode = csSLLISTGetNext( &_sCSClientLibData.sRootNodeGraphicalContext );
    while( psNode )
    {   
        psGraphicalContext = ( PCSGRAPHICALCONTEXT )psNode;     
        if( psGraphicalContext->idWindow == lpCPW->hwnd )
        {
            csDEBUGDisplayFormatted( DEBUG_LEVEL_INFO, "%s::Found psGraphicalContext For Window (0x%x)\n", pszFunctionName, lpCPW->hwnd );
            csDEBUGDisplayStruct( DEBUG_LEVEL_STRUCTURE, DEBUG_STRUCT_CSGRAPHICALCONTEXT, psGraphicalContext );
            break;
        }

        /* Get the next graphical context */
        psNode = csSLLISTGetNext( psNode );
    }

    /* Verify that a graphical context was found, if not then call the next hook and return */
    if( !psNode )
    {
        /* Call the next hook proc and return the value it returns */
        CSDEBUG_RETURN_NOERROR( DEBUG_LEVEL_FUNCTION, CallNextHookEx( 
            _sCSClientLibData.hWindowSnoopHook, nCode, wParam, lParam ) );
    }

    /* Filter the messages */
    switch( lpCPW->message )
    {
        case WM_CREATE:
             csDEBUGDisplayFormatted( DEBUG_LEVEL_INFO, "%s::WM_CREATE(0x%x)\n", pszFunctionName, lpCPW->hwnd );
             break;

        case WM_COMMAND:

            switch( LOWORD( lpCPW->wParam ) )
            {
            }  

            break;

        case WM_MOVE:
            break;

        case WM_CLOSE:                          
            csDEBUGDisplayFormatted( DEBUG_LEVEL_INFO, "%s::WM_CLOSE(0x%x)\n", pszFunctionName, lpCPW->hwnd );
            break;
    }
#endif

    /* Call the next hook proc */
    lResult = CallNextHookEx( _sCSClientLibData.hWindowSnoopHook, nCode, wParam, lParam ); 

    /* Return the value from CallNextHookProc */
    CSDEBUG_RETURN_NOERROR( DEBUG_LEVEL_FUNCTION, lResult );
}

/*****************************************************************************
**
**  Function        : csSnoopKeyboardProc
**
**  Parameters      : Standard window types
**
**  Purpose         : Filters keyboard messages to perform realtime actions
**
**  Return Value    : CSRESULT
**
**  Programmer(s)   : Don Fowler
**
*****************************************************************************/

LRESULT CALLBACK csSnoopKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    LRESULT lResult;
    PFxSz pszFunctionName = "csSnoopKeyboardProc";
    static FxU32 u32AltKeyDown = FXFALSE;
    csDEBUGDisplayFormatted( DEBUG_LEVEL_FUNCTION, "Enter - %s( nCode(0x%x), wParam(0x%x), lParam(0x%x) )\n", pszFunctionName, nCode, wParam, lParam );

    /* If the flag to disallow processing is set then simply return */
    if( !_sCSClientLibData.u32SnoopHooksEnabled )
    {
        /* Return from snoop function */
       CSDEBUG_RETURN_NOERROR( DEBUG_LEVEL_FUNCTION, 0x00 );
    }

    /* Validate processing ability */
    if( nCode >= 0 )
    {
        csDEBUGDisplayFormatted( DEBUG_LEVEL_INFO, "%s::VK_(0x%x) (0x%x)\n", pszFunctionName, wParam, lParam );

        /* Filter the messages */
        switch( wParam )
        {
            case VK_MENU:
                if( !( lParam & (1<<31) ) )
                {
                    u32AltKeyDown = FXTRUE;
                }
                else
                {
                    u32AltKeyDown = FXFALSE;
                }
                break;

            case VK_TAB:
                csDEBUGDisplayFormatted( DEBUG_LEVEL_INFO, "%s::VK_TAB(0x%x)\n", pszFunctionName, lParam );

                if( u32AltKeyDown )    
                    csDEBUGDisplayFormatted( DEBUG_LEVEL_INFO, "%s::ALT+TAB Pressed\n", pszFunctionName );

                 break;
        }
    }

    /* Call the next hook proc */
    lResult = CallNextHookEx( _sCSClientLibData.hKeyboardSnoopHook, nCode, wParam, lParam ); 

    /* Return the value from CallNextHookProc */
    CSDEBUG_RETURN_NOERROR( DEBUG_LEVEL_FUNCTION, lResult );
}


#endif /* WIN32 */