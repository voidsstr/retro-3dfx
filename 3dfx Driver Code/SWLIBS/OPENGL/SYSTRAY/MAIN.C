/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Revision: 2$ 
** $Date: 10/11/00 8:07:11 PM$ 
**
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <process.h>

#include "resource.h"

#define OPENGL_CMD      4352        /* for OpenGL ExtEscape */
#define OPENGL_GETINFO  4353        /* for OpenGL ExtEscape */
#define OPENGL_GETINFO_DRVNAME  0

typedef struct {
  ULONG ulSubEsc;
} OglInInfo;

typedef struct {
  ULONG ulVersion;
  ULONG ulDriverVersion;
  WCHAR awch[MAX_PATH+1];
} OglOutInfo;

static HWND           mainHwnd;

static NOTIFYICONDATA nid;
static OglInInfo      inInfo;
static OglOutInfo     outInfo;

static BOOL           done;
static BOOL           loaded;
static BOOL           accelerated;


LONG WINAPI MainWndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam ) {

    HMENU   menu;
    POINT   point;

    LONG    lRet = 1;
	
    switch (uMsg)
    {
	case WM_USER:
		switch( lParam ) {
		case WM_LBUTTONDBLCLK:
            if ( accelerated ) {
                PostMessage( hWnd, WM_COMMAND, ID_OPENGL_DISABLE, 0 );
            } else {
                PostMessage( hWnd, WM_COMMAND, ID_OPENGL_ENABLE, 0 );
            }
			break;
        case WM_RBUTTONDOWN:
            menu = GetMenu( hWnd );
            menu = GetSubMenu( menu, 1 );
            GetCursorPos( &point );
            TrackPopupMenuEx( menu,
                              TPM_RIGHTALIGN | TPM_RIGHTBUTTON,
                              point.x,
                              point.y,
                              hWnd,
                              0 );
            break;
		}
		break;
	case WM_COMMAND:
		switch( LOWORD(wParam) ) {
		case ID_ACTION_ADDICON:
			if ( !loaded ) {
				nid.cbSize           = sizeof( nid );
				nid.hWnd             = mainHwnd;
				nid.uID              = 0;
				nid.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP ;
				nid.uCallbackMessage = WM_USER;
				nid.hIcon            = 
					LoadIcon( GetModuleHandle( 0 ), MAKEINTRESOURCE( IDI_ICON3 ) );
				strcpy( nid.szTip, "Default OpenGL Acceleration" );
				Shell_NotifyIcon( NIM_ADD, &nid );
				loaded      = TRUE;
                accelerated = FALSE;

                menu = GetMenu( hWnd );
                menu = GetSubMenu( menu, 1 );
                EnableMenuItem( menu, ID_OPENGL_ENABLE,  MF_ENABLED );
                EnableMenuItem( menu, ID_OPENGL_DISABLE, MF_ENABLED );
	            CheckMenuRadioItem( menu,
                                    ID_OPENGL_ENABLE,
                                    ID_OPENGL_DISABLE,
                                    ID_OPENGL_DISABLE,
                                    MF_BYCOMMAND );
			}
			break;
		case ID_ACTION_REMOVEICON:
			if ( loaded ) {
				nid.cbSize           = sizeof( nid );
				nid.hWnd             = mainHwnd;
				nid.uID              = 0;
				Shell_NotifyIcon( NIM_DELETE, &nid );
				loaded = FALSE;
                accelerated = FALSE;

                menu = GetMenu( hWnd );
                menu = GetSubMenu( menu, 1 );
                EnableMenuItem( menu, ID_OPENGL_ENABLE,  MF_GRAYED );
                EnableMenuItem( menu, ID_OPENGL_DISABLE, MF_GRAYED );
			}
			break;
        case ID_OPENGL_ENABLE:
            if ( loaded ) {
				nid.cbSize           = sizeof( nid );
				nid.hWnd             = mainHwnd;
				nid.uID              = 0;
				nid.uFlags           = NIF_ICON | NIF_TIP;
				nid.hIcon            = 
					LoadIcon( GetModuleHandle( 0 ), MAKEINTRESOURCE( IDI_ICON1 ) );
				strcpy( nid.szTip, "3Dfx OpenGL Acceleration" );
				Shell_NotifyIcon( NIM_MODIFY, &nid );
				loaded      = TRUE;
                accelerated = TRUE;
				inInfo.ulSubEsc = 0xbeef;
				ExtEscape( GetDC( hWnd ),        // HDC 
                           OPENGL_GETINFO,       // Escape Index
                           sizeof( OglInInfo ),  // input buffer size
                           (LPSTR)&inInfo,       // input buffer
                           sizeof( OglOutInfo ), // output buffer size
                           (LPSTR)&outInfo       // output bffer
                          );

                menu = GetMenu( hWnd );
                menu = GetSubMenu( menu, 1 );
                CheckMenuRadioItem( menu,
                                    ID_OPENGL_ENABLE,
                                    ID_OPENGL_DISABLE,
                                    ID_OPENGL_ENABLE,
                                    MF_BYCOMMAND );
            }
            break;
        case ID_OPENGL_DISABLE:
            if ( loaded ) {
				nid.cbSize           = sizeof( nid );
				nid.hWnd             = mainHwnd;
				nid.uID              = 0;
				nid.uFlags           = NIF_ICON | NIF_TIP;
				nid.hIcon            = 
					LoadIcon( GetModuleHandle( 0 ), MAKEINTRESOURCE( IDI_ICON3 ) );
				strcpy( nid.szTip, "Default OpenGL Acceleration" );
				Shell_NotifyIcon( NIM_MODIFY, &nid );
				loaded      = TRUE;
                accelerated = FALSE;
				inInfo.ulSubEsc = 0xfade;
				ExtEscape( GetDC( hWnd ),                   // HDC 
                           OPENGL_GETINFO,       // Escape Index
                           sizeof( OglInInfo ),  // input buffer size
                           (LPSTR)&inInfo,       // input buffer
                           sizeof( OglOutInfo ), // output buffer size
                           (LPSTR)&outInfo       // output bffer
                          );

                menu = GetMenu( hWnd );
                menu = GetSubMenu( menu, 1 );
                CheckMenuRadioItem( menu,
                                    ID_OPENGL_ENABLE,
                                    ID_OPENGL_DISABLE,
                                    ID_OPENGL_DISABLE,
                                    MF_BYCOMMAND );
            }
            break;
        case ID_OPENGL_REMOVE:
            PostMessage( hWnd, WM_CLOSE, 0, 0 );
            break;
		}
		break;
    case WM_CREATE:
        menu = GetMenu( hWnd );
        menu = GetSubMenu( menu, 1 );
        EnableMenuItem( menu, ID_OPENGL_ENABLE,  MF_GRAYED );
        EnableMenuItem( menu, ID_OPENGL_DISABLE, MF_GRAYED );
        break;
    case WM_SIZE:
        break;
    case WM_CLOSE:
		if ( loaded ) {
			HWND helperWnd;
			nid.cbSize           = sizeof( nid );
			nid.hWnd             = mainHwnd;
			nid.uID              = 0;
			Shell_NotifyIcon( NIM_DELETE, &nid );
			loaded = FALSE;

			helperWnd = FindWindow( "ALTGDI","ALTGDI" );
			if ( helperWnd ) 
				PostMessage( helperWnd, WM_CLOSE, 0, 0 );
			else
				MessageBox( 0, "Unable to find helper window", "error", 0 );

		}
        DestroyWindow( hWnd );
        break;
    case WM_DESTROY:
        PostQuitMessage (0);
        done = TRUE;
        break;
    default:
        /* pass all unhandled messages to DefWindowProc */
        lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
        break;
    }
    /* return 1 if handled message, 0 if not */
    return lRet;
}


int WINAPI WinMain (HINSTANCE hInstance, 
                    HINSTANCE hPrevInstance, 
                    LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS   wc;
    MSG msg;

	CreateMutex( 0, TRUE, "3dfxogl.exe" );

	if ( GetLastError() == ERROR_ALREADY_EXISTS ) {
		MessageBox( 0, "Can't load 3dfxogl.exe twice.", "error", 0 );
		return -1;
	}

	{ /* find helper app and load it */
		const char *filename;
		char buffer[1024];
		char *retName;
		DWORD rv;

		filename  = "altgdi.exe";

		rv = SearchPath( 0,                // Search the exe path
	  				     filename,         // filename for which to search
					     0,                // extension is in filename
					     sizeof( buffer ), // size of return buffer
						 buffer,           // return buffer
					     &retName );       // points to file part
		if ( !rv ) {
			MessageBox( 0, "Can't find 16-bit helper app.", "error", 0 );
			return -1;
		} else {	
			if ( _spawnl( _P_NOWAIT, buffer, "altgdi.exe", 0 ) == -1 ) {
				MessageBox( 0, "Can't load 16-bit helper app.", "error", 0 );
				return -1;
			}
		}
	}
	
    wc.style         = CS_DBLCLKS;
    wc.lpfnWndProc   = (WNDPROC)MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
    wc.lpszClassName = "SystemTrayTest";
    done = FALSE;

    loaded      = FALSE;
    accelerated = FALSE;

    if (!RegisterClass (&wc) ) {
        MessageBox( 0, "Couldn't Register Window Class",
                    "Error", MB_ICONERROR );
    }

	mainHwnd = CreateWindowEx( WS_EX_TOOLWINDOW,
							   "SystemTrayTest",
							   "System Tray Test",
                               WS_OVERLAPPEDWINDOW | 
                               WS_CLIPSIBLINGS | 
                               WS_CLIPCHILDREN,
                               0, 0, 300, 300,
                               NULL,
                               NULL,
                               hInstance,
                               NULL);						   

    if (!mainHwnd) {
        MessageBox( 0, "Couldn't Create Window",
                    "Systray Test", MB_ICONERROR );
    }

    PostMessage( mainHwnd, WM_COMMAND, ID_ACTION_ADDICON, 0 );
    PostMessage( mainHwnd, WM_COMMAND, ID_OPENGL_ENABLE, 0 );

    while( !done ) {
		WaitMessage(); // Sleep if no messages
        while( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) ) {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }
    
    return 0;
}
