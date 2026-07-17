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
** $Date: 10/11/00 7:33:19 PM$ 
**
*/

#include <glide.h>

#include <atutil.h>
#include <atrender.h>
#include <atinput.h>
#include <ataudio.h>
#include "atdemop.h"
#include <atscene.h>
#include <afxres.h>
#include <conio.h>
#include "atdres.h"


/* win95 frontend for demo applications so that they can run in different OS
   environments (WIN95, DOS, MacOS etc. without requiring code changes.
   Assumes a simple model of 1 display surface. Handles input from a 
   single window
 */

//    Denis. User Defined message loop
//    atUserWindowProc(hWnd,msg,wParam,lParam);
void (*atUserWindowProc)(HWND, UINT, UINT, LONG );
//LRESULT CALLBACK atWindowProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
void  atSetUserWindowProc( void (*user_message_proc)( HWND, UINT, UINT, LONG ) )
{
        atUserWindowProc = user_message_proc;
};


BOOL WINAPI ErrorProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

FxBool atdPause(FxBool flag) {
    if ( _atGlobals.graphicsEnabled ) {
        _atGlobals.bPaused = flag;           
        atrPause(flag);
    }

    /* TBD: This does not currebtly work with glide, need to remove it
       from d3dapp.c
     */

#ifdef notdef
    if ( flag && _atGlobals.fullScreen ) {
         /*
          * Draw the menu and frame
          */
        DrawMenuBar(_atGlobals.hWndMain);
        RedrawWindow(_atGlobals.hWndMain, NULL, NULL, RDW_FRAME);
    }
#endif

    return FXTRUE;
}

void
atdWinHelp(void) {
    WinHelp( _atGlobals.hWndMain, "demo.hlp", HELP_CONTENTS, 0);
    _atGlobals.help = FXFALSE;
}

void
atdExit(FxBool force) {
	exit(1);
}

static void
CheckConsole(void) {
    static consoleAllocated = FXFALSE;

    if (!consoleAllocated) {
        AllocConsole();
        consoleAllocated = FXTRUE;
    }
}

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

static void
SetupMenus(void) {

    _atGlobals.hMainMenu = LoadMenu( _atGlobals.hInstApp,
                                     MAKEINTRESOURCE(IDR_DEMOMENU)) ;

    if ( !_atGlobals.hMainMenu ) {
        atuError( FALSE, "Failed to load popup menu.");
    }

    SetMenu(_atGlobals.hWndMain, _atGlobals.hMainMenu);

}

FxBool
atdWinMain(void) {
    WNDCLASS wc;
    int failcount = 0; /* number of times RenderLoop has failed */
    MSG msg;
    HICON icon;

    if ( _atGlobals.hasConsole )
        CheckConsole();

    atuErrorSetCallback( atdErrorCallback );

    /* see if we have an application icon */

    if ( (icon = LoadIcon( _atGlobals.hInstApp, "AppIcon")) == NULL ) {
        icon = LoadIcon( _atGlobals.hInstApp, MAKEINTRESOURCE(IDR_ATB));
    }

    /*
     * Register the window class
     */
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc =  atWindowProc ;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = _atGlobals.hInstApp;
    wc.hIcon = icon;
    wc.hCursor = _atGlobals.fullScreen ? NULL : LoadCursor( NULL, IDC_ARROW );
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
         WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
         WS_THICKFRAME | WS_MINIMIZEBOX | WS_VISIBLE |
         WS_MAXIMIZEBOX,
         CW_USEDEFAULT, CW_USEDEFAULT,
         _atGlobals.driverInfo.width,
         _atGlobals.driverInfo.height,
         NULL,                              /* parent window */
         NULL,                              /* menu handle */
         _atGlobals.hInstApp,               /* program handle */
         NULL);                             /* create parms */  

    if (!_atGlobals.hWndMain){
        atuError(FXTRUE, "CreateWindowEx failed");
        return FALSE;
    }

    SetupMenus();

    ShowWindow(_atGlobals.hWndMain, SW_SHOWNORMAL);

    SetFocus(_atGlobals.hWndMain);

    if (!AppInitGraphics()) {
       atuError(FXTRUE, "Can't initialize graphics\n");
    }

    /* initialize input library */

    if (!atiInit(MessageFunc)) {
       atuError(FXTRUE, "Can't initialize input\n");
    }

    atuErrorSetCallback( atdErrorCallback );

    if ( _atGlobals.sound )
        _atGlobals.soundAvailable = ataInit( (FxU32)_atGlobals.hWndMain );

    /* run the application */

    atuErrorSetCallback( atdErrorCallback );


    /* Denis. User define message loop */
    atUserWindowProc = NULL;


    AppMain(_atGlobals.argc, _atGlobals.argv);

    /* cleanup */

    atiShutdown();
    AppTermGraphics();
    DestroyWindow(_atGlobals.hWndMain);
    return msg.wParam;
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
    /*
     * Initialize the global variables
     */

    _atdInitGlobals();

    /* Tokenize the command line and search for standard options */
    
    tokenizeCmdString( lpCmdLine );

    _atGlobals.hInstApp = hInstance;

    return atdWinMain();
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

void
InitMenu(HMENU hMenu) { 
    int  cMenuItems = GetMenuItemCount(hMenu); 
    int  nPos; 
    UINT id; 
    UINT fuFlags; 
 
    for (nPos = 0; nPos < cMenuItems; nPos++) { 
        id = GetMenuItemID(hMenu, nPos); 
 
        switch (id) { 
        case ID_RENDER_FOG:
            CheckMenuItem(hMenu, id, 
                          (_atGlobals.fog) ? MF_CHECKED : MF_UNCHECKED);
            break;
        case ID_RENDER_BILINEAR:
            CheckMenuItem(hMenu, id, 
                          (_atGlobals.bilinear) ? MF_CHECKED : MF_UNCHECKED);
            break;
        case ID_RENDER_MIPMAPPING:
            CheckMenuItem(hMenu, id, 
                          (_atGlobals.mipMap) ? MF_CHECKED : MF_UNCHECKED);
            break;
        case ID_RENDER_WIREFRAME:
            CheckMenuItem(hMenu, id, 
                          (_atGlobals.wireframe) ? MF_CHECKED : MF_UNCHECKED);
            break;
        case ID_RENDER_TEXTURING:
            CheckMenuItem(hMenu, id, 
                          (_atGlobals.texture) ? MF_CHECKED : MF_UNCHECKED);
            break;
        case ID_SCREENPRINT:
            CheckMenuItem(hMenu, id, 
                          (_atGlobals.print) ? MF_CHECKED : MF_UNCHECKED);
            break;
        case ID_EVENT_PLAYBACK:
            CheckMenuItem(hMenu, id, 
                          (_atGlobals.eventMode == ATI_EM_PLAYBACK) ? 
                                                   MF_CHECKED : MF_UNCHECKED);
            break;
        case ID_EVENT_RECORD:
            CheckMenuItem(hMenu, id, 
                          (_atGlobals.eventMode == ATI_EM_RECORD) ? 
                                                   MF_CHECKED : MF_UNCHECKED);
            break;
        case ID_MISC_PERFORMANCE:
            CheckMenuItem(hMenu, id, 
                          (_atGlobals.showPerformance == ATD_PERF_FPS) ? 
                                                   MF_CHECKED : MF_UNCHECKED);
            break;
        case ID_MISC_PERFORMANCEFULL:
            CheckMenuItem(hMenu, id, 
                          (_atGlobals.showPerformance == ATD_PERF_ALL) ? 
                                                   MF_CHECKED : MF_UNCHECKED);
            break;
        case ID_MISC_VSYNC:
            CheckMenuItem(hMenu, id, 
                          (_atGlobals.sync) ? MF_CHECKED : MF_UNCHECKED);
            break;
        case ID_MISC_SOUND:
            fuFlags = _atGlobals.soundAvailable ? MF_BYCOMMAND | MF_GRAYED : 
                                                  MF_BYCOMMAND | MF_ENABLED; 
            EnableMenuItem(hMenu, id, fuFlags); 
            CheckMenuItem(hMenu, id, 
                          (_atGlobals.sound) ? MF_CHECKED : MF_UNCHECKED);
            break;
        } 
    } 
}

LRESULT CALLBACK atWindowProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam) {
    int x, y, w, h;
    POINT pClientOnPrimary;

    // Denis. Allow the ATB app to define its own windows message
    // loop. Then we call it here by default, as long as it != NULL
    // and we then proceed with the regular stuff
    if ( atUserWindowProc )
        atUserWindowProc(hWnd,msg,wParam,lParam);


    switch(msg) {
    case WM_KEYDOWN:
        if (_atGlobals.fullScreen && 
		     (( wParam == VK_LWIN ) || ( wParam == VK_RWIN ))) {
			_atGlobals.bAppActive = FXFALSE;
             atdPause(FXTRUE);
			 ShowWindow(hWnd, SW_SHOWMINIMIZED);
		}
        break;
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
            if (!atrResize(_atGlobals.ctx, w, h)) {
                atuError(FXTRUE, "Could not resize window");
            }
        break;
    case WM_MOVE:
        /*
         * Update client window position information
         */
        if (!_atGlobals.fullScreen && ( _atGlobals.ctx != NULL )) {
            x = LOWORD(lParam);
            y = HIWORD(lParam);
            if (! atrMove( _atGlobals.ctx, x, y)) {
                atuError(FXTRUE, "Could not resize window");
            }
        } else {
            pClientOnPrimary.x = pClientOnPrimary.y = 0;
            ClientToScreen(hWnd, &pClientOnPrimary);
        }
        break;
    case WM_ACTIVATE: {
            int fActive = LOWORD(wParam);           // activation flag 
            int fMinimized = (BOOL) HIWORD(wParam); // minimized flag 
            atdPause(( fActive == WA_INACTIVE ) || fMinimized );
        }
        break;
    case WM_ACTIVATEAPP:
        _atGlobals.bAppActive = (BOOL)wParam;
         atdPause(!_atGlobals.bAppActive);
        break;
    case WM_SETCURSOR:
        /*
         * Prevent the cursor from being shown in fullscreen
         */
        if (_atGlobals.fullScreen && !_atGlobals.bPaused) {
            SetCursor(NULL);
            break;
        }
        break;
    case WM_MOVING:
        /*
         * Prevent the window from moving in fullscreen
         */
        if (_atGlobals.fullScreen) {
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
    case WM_INITMENU:
        atdPause(TRUE);
        break;
    case WM_INITMENUPOPUP:
        InitMenu((HMENU)wParam);
        break;
    case WM_GETMINMAXINFO:
        /*
         * Control extent to which window can be resized
         * when not minimized. Ensure window does not shrink to nothing
         * or grow beyond the memory we have available
         */
        if (!_atGlobals.bMinimized) {
            if (_atGlobals.fullScreen) {
                ((LPMINMAXINFO)lParam)->ptMinTrackSize.x = 
                ((LPMINMAXINFO)lParam)->ptMaxTrackSize.x =
                                                   _atGlobals.driverInfo.width;
                ((LPMINMAXINFO)lParam)->ptMinTrackSize.y =
                ((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = 
                                                   _atGlobals.driverInfo.height;
            } else {
                ((LPMINMAXINFO)lParam)->ptMaxSize.x = 640;
                ((LPMINMAXINFO)lParam)->ptMaxSize.y = 480;
                ((LPMINMAXINFO)lParam)->ptMinTrackSize.x = 160;
                ((LPMINMAXINFO)lParam)->ptMinTrackSize.y = 120;
                ((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = 640;
                ((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = 480;
            }
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case ID_APP_EXIT:
            atdExit(FXFALSE);
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
        case ID_JOYCAL:
            atiJoystickCalibrate();
            break;
        case ID_HELP:
            atdWinHelp();
            break;
        case ID_SCREENPRINT:
            _atGlobals.print = !_atGlobals.print;
            break;
        case ID_EVENT_RESET:
            atiEventMode(_atGlobals.eventMode = ATI_EM_DEFAULT);
            atdResetState();
            break;
        case ID_EVENT_PLAYBACK:
            togglePlayback();
            break;
        case ID_EVENT_RECORD:
            toggleRecord();
            break;
        case ID_MISC_PERFORMANCE:
            togglePerformance(FXFALSE);
            break;
        case ID_MISC_PERFORMANCEFULL:
            togglePerformance(FXTRUE);
            break;
        case ID_MISC_VSYNC:
            _atGlobals.sync = !_atGlobals.sync;
            break;
        case ID_MISC_SOUND:
            if ( _atGlobals.soundAvailable )
                _atGlobals.sound = !_atGlobals.sound;
            break;
        case ID_APP_ABOUT:
           DialogBoxParam( _atGlobals.hInstApp, MAKEINTRESOURCE(IDD_ABOUTBOX),
                           _atGlobals.hWndMain, AboutProc, 0) ;
            break;
        }
    }
    return (atiWindowProc(hWnd,msg,wParam,lParam));
}

void atdSetName(char *name) {
    char buff[40];
    HMENU aboutMenu;
    MENUITEMINFO menuItem;

    strncpy(_atGlobals.appName, name, sizeof(_atGlobals.appName));

    sprintf(buff, "3DFX %s", name);
    SetWindowText(_atGlobals.hWndMain, buff);

    aboutMenu = GetSubMenu(_atGlobals.hMainMenu, 4);

    menuItem.cbSize = sizeof(menuItem);  
    menuItem.fMask = MIIM_TYPE; 
    menuItem.fType = MFT_STRING; 
    sprintf(buff, "About %s", name);
    menuItem.dwTypeData = buff; 
    SetMenuItemInfo( aboutMenu, 2, TRUE, &menuItem);
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

static char errBuff[256];
static FxBool errFlag;

BOOL WINAPI 
ErrorProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    HWND hWndErrorIcon = CTRL(hWnd, IDC_ERROR_ICON);
    HWND hWndMessage = CTRL(hWnd, IDC_ERROR_MESSAGE);
    HICON icon;

    switch( msg) {
        case WM_INITDIALOG:
        {
            icon = LoadIcon( NULL, errFlag ? IDI_HAND : IDI_EXCLAMATION);
    
            SendMessage(hWndErrorIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)icon);
            SetWindowText(hWnd, "ATB Error");
            SendMessage( hWndMessage, WM_SETTEXT, (WPARAM)0, (LPARAM)errBuff);

        } 
        return TRUE ;

        case WM_COMMAND:
        {
            switch( LOWORD( wParam)) {
                case ID_END_TASK:
                    EndDialog( hWnd, TRUE) ;
                    atdExit(FXTRUE);
                    return TRUE ;
                case IDOK:
                    EndDialog( hWnd, TRUE) ;
                    return TRUE ;
                default:
                    return FALSE ;
            }
        }
        default:
            return FALSE ;
    } 
    return FALSE ;
}

void 
atdErrorCallback( FxBool fatal, const char *format, ... ) {
    errFlag = fatal;

    wvsprintf(errBuff, format, (char *)(&format+1));
    atdPause(TRUE);
    DialogBoxParam( _atGlobals.hInstApp, MAKEINTRESOURCE(IDD_ERROR),
                     _atGlobals.hWndMain, ErrorProc, 0) ;
    if ( fatal ) {
        atdExit( FXTRUE );
    }
    atdPause(FALSE);
}

void atdSetFocus(void) {
    BringWindowToTop( _atGlobals.hWndMain );
    SetCapture( _atGlobals.hWndMain );
    ReleaseCapture();
    SetFocus(_atGlobals.hWndMain);
}

/*-------------------------------------------------------------------
  Function: atdPrintf
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Demo
  Description:
    Multi platform printf function
  Arguments:
    format - printf style format string
    ... - arguments determined by format string
  Return:
    none
  -------------------------------------------------------------------*/

void
atdPrintf( const char *format, ... ) {
    char buff[256];
    int howMany = 0;
    va_list args;

    va_start( args, format);
    vsprintf( buff, format, args);
    va_end( args );

    OutputDebugStr(buff);
    if ( _atGlobals.hasConsole )
        WriteConsole( GetStdHandle(STD_OUTPUT_HANDLE), buff,
                      strlen(buff), &howMany, NULL);
    if ( _atGlobals.debugFile ) {
        fputs(buff, _atGlobals.debugFile);
        fflush(_atGlobals.debugFile);
    }
}

/*-------------------------------------------------------------------
  Function: atdGetString
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Demo
  Description:
    Multi platform get string function
  Arguments:
    c        - buffer to receieve string
    buffSize - size of buffer
  Return:
    String read, or NULL on error
  -------------------------------------------------------------------*/

char *
atdGetString(char *c, int buffSize) {
    int howMany = 0;

    if ( _atGlobals.hasConsole ) {
        ReadConsole( GetStdHandle(STD_INPUT_HANDLE), c, buffSize, &howMany, 
                     NULL);
         return c;
    } else return NULL;
}

/*-------------------------------------------------------------------
  Function: atdGetChar
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Demo
  Description:
    Multi platform get character function
  Arguments:
    None
  Return:
    Character read
  -------------------------------------------------------------------*/

int
atdGetChar(void) {
    int howMany = 0;
    char c[2];

    if ( _atGlobals.hasConsole ) {
        ReadConsole( GetStdHandle(STD_INPUT_HANDLE), c, 1, &howMany, NULL);
        return c[0];
    } else {
        return -1;
    }
}
