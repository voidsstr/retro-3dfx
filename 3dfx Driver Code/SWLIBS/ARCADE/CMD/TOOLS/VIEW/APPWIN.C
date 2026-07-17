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
** $Date: 10/11/00 7:30:05 PM$ 
**
*/

#include <glide.h>
#include <atutil.h>
#include <atrender.h>
#include <atscene.h>
#include <atinput.h>
#include <ataudio.h>
#include <atdemop.h>
#include <conio.h>
#include "resource.h"
#include "afxres.h"

char *GetFileName(void);
void editFile(char *fileName);
void doAbout(void);
void showTabDialog(void);
extern FxU32 gCullMode ;

/* win95 frontend for demo applications so that they can run in different OS
   environments (WIN95, DOS, MacOS etc. without requiring code changes.
   Assumes a simple model of 1 display surface. Handles input from a 
   single window
 */

/*
 * Tokenize and parse the command line
 */

void tokenizeCmdString( const char *cmdLine ) {
    char *token;
    char *s    = strdup( cmdLine );
    char **argv;
    int argc;

    argc = 1;
    argv = calloc( sizeof( char* ), 64 );

    for( token = strtok( s, " \n\t" ); 
         token; 
         token = strtok( NULL, " \n\t" ) ) {
        argv[argc++] = token;
    }
    argv[0] = "atbapp"; /* TBD: for want of a better name */
    _atdParseCmdLine( argc, argv );
    return;
}

/*
 * Create the window and initialize all objects needed to begin rendering
 */

FxBool
atdWinMain(void) {
}

/****************************************************************************/
/*                            WinMain                                       */
/****************************************************************************/
/*
 * Create a default window, parse the command line for options and call main
 */

int PASCAL
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
        int nCmdShow) {
    WNDCLASS wc;
    int failcount = 0; /* number of times RenderLoop has failed */
    MSG msg;

    /*
     * Initialize the global variables
     */

    _atdInitGlobals();

    /* Tokenize the command line and search for standard options */
    
    tokenizeCmdString( lpCmdLine );

    _atGlobals.hInstApp = hInstance;

    atuErrorSetCallback( atdErrorCallback );

    /*
     * Register the window class
     */
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc =  atWindowProc ;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = _atGlobals.hInstApp;
    wc.hIcon = LoadIcon( _atGlobals.hInstApp, "AppIcon");
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "ATBApp";
    if (!RegisterClass(&wc))
        return FALSE;
    /*
     * Create a window with some default settings that may change
     */

    _atGlobals.hWndMain = CreateWindowEx(
         WS_EX_APPWINDOW,
         "ATBApp",
         "ATB Demo",
         WS_OVERLAPPED | WS_CAPTION |
         WS_THICKFRAME | WS_MINIMIZEBOX,
         CW_USEDEFAULT, CW_USEDEFAULT,
         START_WIN_X, START_WIN_Y,
         NULL,                              /* parent window */
         NULL,                              /* menu handle */
         _atGlobals.hInstApp,               /* program handle */
         NULL);                             /* create parms */  

    if (!_atGlobals.hWndMain){
        atuError(FXTRUE, "CreateWindowEx failed");
        return FALSE;
    }

    ShowWindow(_atGlobals.hWndMain, SW_SHOWNORMAL);

    SetFocus(_atGlobals.hWndMain);

    if (!AppInitGraphics()) {
       atuError(FXTRUE, "Can't initialize graphics\n");
    }

    /* initialize input library */

    if (!atiInit(MessageFunc)) {
       atuError(FXTRUE, "Can't initialize input\n");
    }

    if ( _atGlobals.soundAvailable )
        _atGlobals.soundAvailable = ataInit( (FxU32)_atGlobals.hWndMain );

    /* run the application */

    AppMain(_atGlobals.argc, _atGlobals.argv);

    /* cleanup */

    atiShutdown();
    AppTermGraphics();
    DestroyWindow(_atGlobals.hWndMain);
    return msg.wParam;
}


/****************************************************************************/
/*          Initialization, error reporting and release functions.          */
/****************************************************************************/

void MessageFunc(void) {
    MSG         msg;

    while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK atWindowProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam) {
    int w, h;
    POINT pClientOnPrimary; 
    char *fileToBrowse ;

    switch(msg) {
    case WM_SIZE:
        /*
         * If we have minimzied, take note and call the default window proc
         */
        if (wParam == SIZE_MINIMIZED) {
            _atGlobals.bMinimized = TRUE;
            break;
        }
        /*
         * If we are minimized, this is the un-minimized size message.
         */
        if (_atGlobals.bMinimized) {
            _atGlobals.bMinimized = FALSE;
            break;
        }
        w = LOWORD(lParam);
        h = HIWORD(lParam);
        /*
         * Restore our surfaces and update the dirty rectangle info
         */
        if ( _atGlobals.ctx != NULL )
            atrResize(_atGlobals.ctx, w, h);
        break;
    case WM_MOVE:
        /*
         * Update client window position information
         */
        pClientOnPrimary.x = pClientOnPrimary.y = 0;
        ClientToScreen(hWnd, &pClientOnPrimary);
        break;
    case WM_ACTIVATE:
        break;
    case WM_ACTIVATEAPP:
        _atGlobals.bAppActive = (BOOL)wParam;
        break;
    case WM_SETCURSOR:
        /*
         * Prevent the cursor from being shown in fullscreen
         */
        if (_atGlobals.bFullScreen && !_atGlobals.bPaused) {
            SetCursor(NULL);
            break;
        }
        break;
    case WM_MOVING:
        /*
         * Prevent the window from moving in fullscreen
         */
        if (_atGlobals.bFullScreen) {
            GetWindowRect(hWnd, (LPRECT)lParam);
        }
        break;
    /*
     * Pause and unpause the app when entering/leaving the menu
     */
    case WM_ENTERMENULOOP:
        atdPause(TRUE);
        break;
    case WM_EXITMENULOOP:
        atdPause(FALSE);
        break;
    case WM_INITMENUPOPUP:
        CheckMenuItem((HMENU)wParam, ID_RENDER_FOG, (_atGlobals.fog) ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem((HMENU)wParam, ID_RENDER_BILINEAR, (_atGlobals.bilinear) ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem((HMENU)wParam, ID_RENDER_MIPMAPPING, (_atGlobals.mipMap) ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem((HMENU)wParam, ID_RENDER_WIREFRAME, (_atGlobals.wireframe) ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem((HMENU)wParam, ID_RENDER_TEXTURING, (_atGlobals.texture) ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem((HMENU)wParam, ID_MISC_PLUG, (_atGlobals.plug) ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem((HMENU)wParam, ID_RENDER_CULLING, (gCullMode == ATS_CULL_NODES) ? MF_CHECKED : MF_UNCHECKED);
        break;
    case WM_GETMINMAXINFO:
        /*
         * Some applications don't like being resized, such as those 
         * which use screen coordinates (TLVERTEXs).
         * Prevent resizing through this message
         */
        if (_atGlobals.bResizingDisabled && !_atGlobals.bMinimized) {
            ((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = START_WIN_X;
            ((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = START_WIN_Y;
            ((LPMINMAXINFO)lParam)->ptMinTrackSize.x = START_WIN_X;
            ((LPMINMAXINFO)lParam)->ptMinTrackSize.y = START_WIN_Y;
        } else {
            ((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = _atGlobals.caps.width;
            ((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = _atGlobals.caps.height;
        }

        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case ID_APP_EXIT:
            exit(1);

        case ID_FILE_OPEN:
            if (( fileToBrowse = GetFileName()) != NULL ) {
                editFile(fileToBrowse);
            }
            break;
        case ID_RENDER_FOG:
            _atGlobals.fog = !_atGlobals.fog;
            break;
        case ID_RENDER_BILINEAR:
             _atGlobals.bilinear = !_atGlobals.bilinear;
            break;
        case ID_RENDER_MIPMAPPING:
            _atGlobals.mipMap = !_atGlobals.mipMap;
            break;
        case ID_RENDER_WIREFRAME:
            _atGlobals.wireframe = !_atGlobals.wireframe;
            if ( _atGlobals.wireframe )
              atsRenderMode(ATS_RM_WIREFRAME);
            else
              atsRenderMode(ATS_RM_SOLID);
            break;
        case ID_RENDER_TEXTURING:
            _atGlobals.texture = !_atGlobals.texture;
            break;
        case ID_MISC_PLUG:
            _atGlobals.plug = !_atGlobals.plug;
            break;
        case ID_MISC_JOYCAL:
            atiJoystickCalibrate();
            break;
        case ID_HELP_ABOUT:
            doAbout();
            break;
        case ID_RENDER_CULLING:
            if (gCullMode == ATS_CULL_NODES) 
                 gCullMode = ATS_CULL_NONE;
            else gCullMode = ATS_CULL_NODES;
            break;
        }
        break;
    }
    return (atiWindowProc(hWnd,msg,wParam,lParam));
}

void atdSetName(char *name) {
    SetWindowText(_atGlobals.hWndMain, name);
}

HINSTANCE atdGetInstance(void) {
    return _atGlobals.hInstApp ;
}

/*-------------------------------------------------------------------
  Function: atdErrorCallback
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Demo
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
static void 
atdErrorCallback( FxBool fatal, const char *format, ... ) {
    char buff[256];
    UINT uType = MB_TASKMODAL|MB_OK|MB_SETFOREGROUND;

#if defined(MB_ICONERROR) && defined(MB_ICONWARNING)
    uType |= fatal ? MB_ICONERROR : MB_ICONWARNING;
#endif

    wvsprintf(buff, format, (char *)(&format+1));
    lstrcat(buff, "\r\n");
    atdPause(TRUE);
    if (_atGlobals.bFullScreen)
        SetWindowPos(_atGlobals.hWndMain, HWND_NOTOPMOST, 0, 0, 0, 0,
                     SWP_NOSIZE | SWP_NOMOVE);
    MessageBox( NULL, buff, "ATB Error Message", uType );
    if (_atGlobals.bFullScreen)
        SetWindowPos(_atGlobals.hWndMain, HWND_TOPMOST, 0, 0, 0, 0,
                     SWP_NOSIZE | SWP_NOMOVE);
    if ( fatal ) {
        exit( -1 );
    }
    atdPause(FALSE);
}

/*-------------------------------------------------------------------
  Function: atdPrintf
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Demo
  Description:
    Windows version of general purpose printf function
  Arguments:
    format - printf style format string
    ... - arguments determined by format string
  Return:
    none
  -------------------------------------------------------------------*/
void 
atdPrintf( const char *format, ... ) {
    char buff[256];

    wvsprintf(buff, format, (char *)(&format+1));
    OutputDebugStr(buff);
}

void atdSetFocus(void) {
    ShowWindow(_atGlobals.hWndMain, SW_SHOWNORMAL);

    SetFocus(_atGlobals.hWndMain);
}
