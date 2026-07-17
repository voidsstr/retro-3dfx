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
** $Revision: 2$
** $Date: 10/11/00 8:41:31 PM$
*/

#include <windows.h>
#include <commctrl.h>

#include <3dfx.h>
#include <gdebug.h>
#include <ui.h>

//---------------------------------------------------------------------------
// The MAIN ui LOOP
//---------------------------------------------------------------------------
void FX_EXPORT FX_CSTYLE
uiMain( const char *szAppName,			// window title
	FxU32 w, FxU32 h,			// initial window width, height
	WORD IconID, WORD MenuID,		// icon and menu IDs
	WNDPROC MainWndProc,			// the main WNDPROC
	BOOL (*checkMessage)(HWND,MSG *),	// acclerator and dialog checker
	void (*Reshape)(FxU32 w, FxU32 h),	// reshape proc
	void (*doFrame)(HWND hwnd)		// frame proc
	)
{
    int xb,yb, cap;
    MSG msg;
    HWND hWnd;
    WNDCLASS wc;
    HANDLE hInstance;
    RECT rect;
    static CHAR szClassName[] = "DemoClass";

#if !defined(FX_DLL_ENABLE)
    GDBG_INIT();
#endif
    hInstance	     = GetModuleHandle(NULL);
    wc.style	     = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon	     = LoadIcon(hInstance,MAKEINTRESOURCE(IconID));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(GRAY_BRUSH);
    wc.lpszMenuName  = MAKEINTRESOURCE(MenuID);
    wc.lpszClassName = szClassName;
    if (!RegisterClass(&wc)) {
	GDBG_ERROR("RegisterClass", "error = %d\n", GetLastError());
	return;
    }
    InitCommonControls();

    xb = GetSystemMetrics(SM_CXFRAME);
    yb = GetSystemMetrics(SM_CYFRAME);
    cap = GetSystemMetrics(SM_CYCAPTION);

    hWnd = CreateWindow(szClassName,
			szAppName,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			w > 0 ? w + xb + xb : CW_USEDEFAULT,
			h > 0 ? h + yb + yb + cap + cap - 2 : CW_USEDEFAULT,
			NULL,
			NULL,
			hInstance,
			NULL);
    GDBG_INFO(55,"uiMain(%s); hWnd=%d(0x%x)\n",szAppName,hWnd,hWnd);

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    if (!GetClientRect(hWnd, &rect)) {
	GDBG_ERROR("uiMain", "GetClientRect failed, GetLastError() => %d\n",
			GetLastError());
    }
    GDBG_INFO(55,"\twindow size is %d x %d\n",rect.right,rect.bottom);
    if (Reshape) Reshape(rect.right,rect.bottom);

    InvalidateRgn(hWnd,NULL,FALSE);		// force a WM_PAINT message
//    myHelpMsg = RegisterWindowMessage(HELPMSGSTRING);

    //-----------------------------------------------------------------------
    // The main (generic infinite animation) message loop for the application
    //-----------------------------------------------------------------------
    do {
        if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {	// if there's a message
	    if (!checkMessage(hWnd,&msg)) {	// give caller a chance first
		TranslateMessage(&msg);		// else translate and dispatch
		DispatchMessage(&msg);
	    }
        } else {
	    if (IsIconic(hWnd))			// if iconic
		WaitMessage();			// then just wait for a message
	    else {				// otherwise draw another frame.
		doFrame(hWnd);			// process the graphics for 1 frame
	    }
	}
    } while (msg.message != WM_QUIT);

    DestroyWindow(hWnd);			// destroy parent window (and children)
}
