/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
**
** $Revision: 4$ 
** $Date: 10/11/00 7:33:34 PM$ 
**
*/

#include <atinput.h>

#define START_WIN_X 640
#define START_WIN_Y 480

void _atiPutEvent( AtiEvent *ev );

BOOL(*MouseHandler)(UINT, WPARAM, LPARAM);    /* sample's function which traps
                                                 mouse input */

void CreateInputWin(void);
void HandleLossOfFocus(void);

/*-------------------------------------------------------------------
  Function: _atiWinErrorCallback
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Utility
  Description:
    Windows version of general purpose error callback and default.  Just creates
    a message box with the specified message and conditionally exits with 
    the exit API.
  Arguments:
    fatal - if true then error is not recoverable, so exit
    format - printf style format string
    ... - arguments determined by format string
  Return:
    none
  -------------------------------------------------------------------*/
void _atiWinErrorCallback( FxBool fatal, const char *format, ... ) {
    va_list args;
    char buff[128];
    UINT uType = MB_TASKMODAL|MB_OK|MB_SETFOREGROUND;

#ifdef NOTDEF
    uType |= fatal ? MB_ICONERROR : MB_ICONWARNING;
#endif

    va_start( args, format );
    vsprintf( buff, format, args );
    MessageBox( NULL, buff, "ATU Error", uType);
    va_end( args );

    if ( fatal )
      {
        exit( -1 );
      }
    return;
}

LRESULT CALLBACK atiWindowProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam) {
   AtiEvent    e;

   switch(msg) {
      case WM_SETFOCUS:
         if (atiState.focusGain != NULL )
             (*atiState.focusGain)(FXTRUE);
         break;
      case WM_KILLFOCUS:
         if (atiState.focusLost != NULL )
             (*atiState.focusLost)(FXFALSE);
         break;
      case WM_KEYDOWN:
         e.device = ATI_DEV_KEYBOARD;
         e.ev.key.code = (lParam&0x00FF0000)>>16;
         if (lParam&0x01000000)
             e.ev.key.code+=0x80;
         e.ev.key.state = ATI_KEY_PRESS;
         _atiPutEvent(&e);
         break;
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            /*
             * Call the sample's MouseHandler if available
             */
            if (!MouseHandler)
                break;
            if ((MouseHandler)(msg, wParam, lParam))
                return 1;
            break;
      case WM_KEYUP:
         e.device = ATI_DEV_KEYBOARD;
         e.ev.key.code = (lParam&0x00FF0000)>>16;
         if (lParam&0x01000000)
             e.ev.key.code+=0x80;
         e.ev.key.state = ATI_KEY_RELEASE;
         _atiPutEvent(&e);
         break;

      case WM_CLOSE:
         if (atiState.winClose != NULL )
             (*atiState.winClose)();
         break;
        case WM_DESTROY:
            if (atiState.winClose != NULL )
                 (*atiState.winClose)();
            break;
   }
   return (DefWindowProc(hWnd,msg,wParam,lParam));
}

FxBool InitWin32(void) {
   return FXTRUE;
}
