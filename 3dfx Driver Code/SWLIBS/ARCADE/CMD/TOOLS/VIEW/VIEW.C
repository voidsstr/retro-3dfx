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
** $Date: 10/11/00 7:30:37 PM$ 
**
*/

#include <windows.h>
#include <commctrl.h>
#include <shellAPI.h>
#include <shlguid.h>
#include <shlobj.h>
#include <mapi.h>
#include <isguids.h>
#include <atutil.h>
#include <atrender.h>
#include <atscene.h>
#include <atinput.h>
#include <ataudio.h>
#include <atdemop.h>
#include <conio.h>
#include "resource.h"
#include "afxres.h"
#include "view.h"

/* data types */

typedef ULONG (WINAPI *PFNMAPISENDDOCUMENTS)(HWND, LPSTR, LPSTR, LPSTR, ULONG);

FxBool quit = FXFALSE;

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

FxBool 
atdPause(FxBool flag) {
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

/*
 * Create the window and initialize all objects needed to begin rendering
 */

FxBool
atdWinMain(void) {
    return FXTRUE;
}

void
setStructureRect(void) {
    RECT rc1, rc2;

    GetWindowRect(hWndTree, &rc1);
    GetClientRect(_atGlobals.hWndMain, &rc2);
    MoveWindow(hWndTree, -5, -5, rc1.right-rc1.left,rc2.bottom - rc2.top+10, 
               TRUE);
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

static void exitApp() {
    RECT rc;

    GetWindowRect(_atGlobals.hWndMain, &rc);
    WriteWindowPos( &rc, _atGlobals.hInstApp) ;
    PostMessage( _atGlobals.hWndMain, WM_QUIT, 0, 0L) ;
    _atGlobals.done = quit = FXTRUE;
}

BOOL 
checkExitApp() {
    MSGBOXPARAMS msgbox ;
    char szText[] = "Do you really want to terminate?" ;
    char szCaption[] = "ATB Viewer" ;

    msgbox.cbSize = sizeof( msgbox) ;
    msgbox.hwndOwner = HWND_DESKTOP ;
    msgbox.hInstance = _atGlobals.hInstApp ;
    msgbox.lpszText = szText ;
    msgbox.lpszCaption = szCaption ;
    msgbox.dwStyle = MB_YESNO ;
    msgbox.lpszIcon = NULL;
    msgbox.dwContextHelpId = 1 ;
    msgbox.lpfnMsgBoxCallback = NULL ;
    msgbox.dwLanguageId = MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL) ;

    if( MessageBoxIndirect( &msgbox) == IDNO) {
        return FALSE ;
    }

    exitApp();

    return FXTRUE;
}

char *GetFileName(char *prompt) {
    char   filter[] = "BOF File\0*.bof\0TDS File\0*.3ds\0FLT File\0*.flt\0"
                      "All Files\0*.*\0";
    static char *fileToBrowse = NULL;
    static OPENFILENAME ofn;

    if ( prompt == NULL )
        prompt = "Open File";

    if ( fileToBrowse == NULL ) {
        fileToBrowse = atuMemCalloc( 512, 1 );

        ofn.lStructSize       = sizeof( ofn );
        ofn.hwndOwner         = _atGlobals.hWndMain;
        ofn.hInstance         = _atGlobals.hInstApp;
        ofn.lpstrFilter       = filter;
        ofn.lpstrCustomFilter = 0;
        ofn.nMaxCustFilter    = 0;
        ofn.nFilterIndex      = 0;
        ofn.lpstrFile         = fileToBrowse;
        ofn.nMaxFile          = 512;
        ofn.lpstrFileTitle    = 0;
        ofn.nMaxFileTitle     = 0;
        ofn.lpstrInitialDir   = NULL;
        ofn.lpstrTitle        = prompt;
        ofn.Flags             = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST, 
        ofn.lpstrDefExt       = 0;
    }

    while( !GetOpenFileName( &ofn ) ) {
        switch ( MessageBox( _atGlobals.hInstApp, 
                            "The browser could not open this file.  Try again?", 
                            "Open Error", 
                            MB_YESNO|MB_ICONERROR ) ) {
          case IDYES:
            continue;
          case IDNO:
          default:
            return NULL;
        }
    }
    return fileToBrowse;
}

BOOL
SendDocument(HWND hWnd, char *name) {
    HMODULE hmod ;
    PFNMAPISENDDOCUMENTS lpfnMAPISendDocuments ;
    ULONG iRet;

    /* load the MAPI library */

    if( ( hmod = LoadLibrary( "MAPI32")) < (HANDLE)32)
        return FALSE;

    if( ( lpfnMAPISendDocuments = (PFNMAPISENDDOCUMENTS)GetProcAddress( hmod, "MAPISendDocuments")) == NULL)
        return FALSE ;

    iRet = (*lpfnMAPISendDocuments)( hWnd, ";", "readme.doc", "readme.doc", 0) ;

    if ( iRet != SUCCESS_SUCCESS ) {
        MessageBeep( 0);
        return FALSE ;
    }

    FreeLibrary( hmod) ;
    return TRUE ;
}

LRESULT CALLBACK atWindowProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam) {
    int w, h;
    POINT pClientOnPrimary; 
    char *fileToBrowse ;
    static RECT rc;

    switch(msg) {
    case WM_CREATE:
        {
            LPCREATESTRUCT *pData = ( LPCREATESTRUCT *)lParam;

            if ( ReadWindowPos( &rc, _atGlobals.hInstApp ) ) {
                if( rc.left == rc.right) {
                    rc.left = 10 ;
                    rc.right = 810 ;
                    rc.top = 10 ;
                    rc.bottom = 610 ;
                }
                /* resize and reposition the main window */
                SetWindowPos(   hWnd, HWND_TOP,
                                rc.left, rc.top,
                                rc.right - rc.left,
                                rc.bottom - rc.top,
                                0) ;
            }
            setStructureRect();

        }
        break ;
    case WM_CLOSE:
        if ( !checkExitApp() )
            return FALSE;
        break ;
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
        setStructureRect();
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if ( GetFocus() != _atGlobals.hWndMain )
            SetFocus(_atGlobals.hWndMain);
        break;
    case WM_DROPFILES:
    {
        HDROP hdrop ;
        static char szFileName[ 100] = { '\0'} ;
        int iCnt, i ;

        // handle to the dropped objects
        hdrop = (HDROP)wParam ;

        // query the dropped files
        iCnt = (int)DragQueryFile( hdrop, 0xFFFFFFFF, NULL, 0) ;

        // loop through the dropped files
        for( i = 0; i < iCnt; i++) {
            // get the name of the ith file
            DragQueryFile( hdrop, i, szFileName, sizeof( szFileName)) ;
            editFile(szFileName);
        }
        // terminate
        DragFinish( hdrop) ;
    }
            break ;
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
        break;
    case WM_MOVING:
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
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case ID_MISC_VIEWMODE:
            SetFocus(_atGlobals.hWndMain);
            break;

        case ID_APP_EXIT:
            if ( !checkExitApp() )
                return FALSE;
            break;

        case ID_FILE_OPEN:
            if (( fileToBrowse = GetFileName(NULL)) != NULL ) {
                editFile(fileToBrowse);
            }
            break;
        case ID_FILE_LOADPATH:
            DialogBoxParam( _atGlobals.hInstApp, MAKEINTRESOURCE(IDD_LOADPATH), 
                            _atGlobals.hWndMain, LoadPathDlgProc, 0) ;
            break;
        case ID_FILE_SEND:
            SendDocument(_atGlobals.hWndMain, "foo");
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
            atdJoystickCalibrate();
            break;
        case ID_HELP_ABOUT:
            DialogBoxParam( _atGlobals.hInstApp, MAKEINTRESOURCE(IDD_ABOUT), 
                            _atGlobals.hWndMain, AboutProc, 0) ;
            break;
        case ID_RENDER_CULLING:
            if (gCullMode == ATS_CULL_NODES) 
                 gCullMode = ATS_CULL_NONE;
            else gCullMode = ATS_CULL_NODES;
            break;
        }
        break;

    case WM_SYSCOMMAND:
        switch( wParam) {
            case SC_CLOSE:
                if ( !checkExitApp() )
                    return FALSE;
                break ;

            default:
                break ;
        }
        break ;
        
    case WM_NOTIFY:
        {
            LPNMHDR pnmhdr = (LPNMHDR)lParam;
            switch(pnmhdr->idFrom) {
            case CT_TREEVIEW:
                structureViewProc(hWnd, msg, wParam, lParam);
                switch ( pnmhdr->code ) {
                    case NM_SETFOCUS:
                        SetTitle("Structure View");
                        break;
                    case NM_KILLFOCUS:
                        SetTitle(modeString);
                        break;
                }
                break;
            }
            break;
        }
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
    if (_atGlobals.caps.fullScreen)
        SetWindowPos(_atGlobals.hWndMain, HWND_NOTOPMOST, 0, 0, 0, 0,
                     SWP_NOSIZE | SWP_NOMOVE);
    MessageBox( NULL, buff, "ATB Error Message", uType );
    if (_atGlobals.caps.fullScreen)
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

void
winCloseFunc(void) {
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
    HWND hWndPrev;
    WNDCLASS wc;
    int failcount = 0; /* number of times RenderLoop has failed */
    MSG msg;
    char szMainWindowClass[ 30] ;
    char szMainWindowTitle[ 30] ;

    InitCommonControls();

    /*
     * Initialize the global variables
     */

    _atdInitGlobals();

    /* Tokenize the command line and search for standard options */
    
    tokenizeCmdString( lpCmdLine );

    _atGlobals.hInstApp = hInstance;

    atuErrorSetCallback( atdErrorCallback );

    LoadString( _atGlobals.hInstApp, IDS_TOP_CLASS_NAME, 
                szMainWindowClass, sizeof( szMainWindowClass)) ;

    LoadString( _atGlobals.hInstApp, IDS_APP_TITLE, 
                szMainWindowTitle, sizeof( szMainWindowTitle)) ;

	/* 
     * see if we already have an application instance running 
     */

    
    hWndPrev = FindWindow(szMainWindowClass, NULL );

    if ( hWndPrev != NULL ) {
        atuError(FXFALSE, "The %s program is already running\n", 
                 szMainWindowTitle);
        SetActiveWindow(hWndPrev);
        return FALSE;
    }

    /*
     * Register the top level window class
     */
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc =  atWindowProc ;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = _atGlobals.hInstApp;
    wc.hIcon = LoadIcon( _atGlobals.hInstApp, "AppIcon");
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    wc.lpszClassName = szMainWindowClass;
    if (!RegisterClass(&wc))
        return FALSE;

    /*
     * Create the top level window
     */

    _atGlobals.hWndMain = CreateWindowEx(
         WS_EX_OVERLAPPEDWINDOW | WS_EX_ACCEPTFILES,
         szMainWindowClass,
         szMainWindowTitle,
         WS_OVERLAPPEDWINDOW | WS_VISIBLE |
         WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
         CW_USEDEFAULT, CW_USEDEFAULT,
         800, 600,
         NULL,                              /* parent window */
         NULL,                              /* menu handle */
         _atGlobals.hInstApp,               /* program handle */
         NULL);                             /* create parms */  

    if (!_atGlobals.hWndMain){
        atuError(FXTRUE, "Could not create main window");
        return FALSE;
    }

    SetFocus(_atGlobals.hWndMain);

    /* create the structure view */

    structureViewInit();
    
    if (!AppInitGraphics()) {
       atuError(FXTRUE, "Can't initialize graphics\n");
    }

    /* initialize input library */

    if (!atiInit(MessageFunc)) {
       atuError(FXTRUE, "Can't initialize input\n");
    }

    if ( _atGlobals.soundAvailable )
        _atGlobals.soundAvailable = ataInit( (FxU32)_atGlobals.hWndMain );

    if (!atsInit())
        atuError( FXTRUE, "Couldn't initialize input library.\n" );

    atuSetLoadHook(fileLocate);

    /* Initialize event handling for the graphics window */

    atdEventInit();

    /* set close func so we don't exit */

    atiWinCloseFunc(winCloseFunc);
    
    /* run the application */

    AppMain(_atGlobals.argc, _atGlobals.argv);

    /* cleanup */

    atiShutdown();
    AppTermGraphics();
    DestroyWindow(_atGlobals.hWndMain);
    return msg.wParam;
}

static char szKeyValue[] = "Window size and position" ;
static char szKey[] = "Software\\3dfx\\view" ;

FxBool ReadWindowPos( LPRECT prc, HINSTANCE hInstance) {
    HKEY hkey ;
    DWORD dwType, dwSize = sizeof( RECT) ;
    long lVal ;

    /* open/create the Software\\3dfx\\view key */
    
    lVal = RegOpenKeyEx(    HKEY_CURRENT_USER,
                            szKey,
                            0,
                            KEY_ALL_ACCESS,
                            &hkey) ;

    if( lVal != ERROR_SUCCESS) {
        return FXFALSE ;
    }

    /* read the information */
    if( RegQueryValueEx(    hkey,
                            szKeyValue,
                            NULL,
                            &dwType,
                            (BYTE *)prc,
                            &dwSize) != ERROR_SUCCESS) {
        atuError( FXFALSE, "Error reading the regisry", NULL, MB_OK) ;
        return FALSE ;
    }

    /* close the key */

    RegCloseKey( hkey) ;

    return TRUE ;
}

FxBool WriteWindowPos( LPRECT prc, HINSTANCE hInstance) {
   HKEY hkey ;
   DWORD dwAction ;

    /* open/create the Software\\3dfx\\view key */

    if( RegCreateKeyEx( HKEY_CURRENT_USER,
                        szKey,
                        0L,
                        "View",
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkey,
                        &dwAction) != ERROR_SUCCESS) {
        atuError( FXFALSE, "Error creating/opening the key") ;
        return FXFALSE ;
    }

    /* lets store the data */

    if( RegSetValueEx(  hkey,
                        szKeyValue,
                        0L,
                        REG_BINARY,
                        (BYTE *)prc,
                        sizeof( RECT)) != ERROR_SUCCESS) {
        atuError( FXFALSE, "Error saving location data") ;
        return FXFALSE ;
    }

    /* close the key */

    RegCloseKey( hkey) ;
    return FXTRUE ;
}
