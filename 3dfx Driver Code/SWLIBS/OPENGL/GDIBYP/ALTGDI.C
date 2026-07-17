
#include "altgdi.h"

#define COLOR_WINDOW 5
int  WINAPI Escape(HDC, int, int, LPCSTR, void FAR*);
HDC  WINAPI CreateCompatibleDC(HDC);
BOOL WINAPI DeleteDC(HDC);
WORD WINAPI GetClipBox(HDC,RECT FAR *);
WORD WINAPI GetMapMode(HDC);


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  Globals                                                                //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

APPINFO  appInfo;

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  InitFuncPatch                                                          //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

BOOL InitFuncPatch ( HINSTANCE  hInstance,
                     LPCSTR     lpFuncName,
                     LPVOID     lpFunc,
                     PATCHINFO *lpPatchInfo ) 
{
    lpPatchInfo->lpTargetFunc = (VOID FAR *) GetProcAddress( hInstance, lpFuncName );
    if( !lpPatchInfo->lpTargetFunc )
    {
        MessageBox( NULL, "Could not get address to function", lpFuncName, MB_OK );
        return FALSE;
    }

    lpPatchInfo->wSel = SelectorAlloc( SELECTOROF(lpPatchInfo->lpTargetFunc) );
    if( !lpPatchInfo->wSel )
    {
        MessageBox( NULL, "Could not get selector to function", lpFuncName, MB_OK );
        return FALSE;
    }

    lpPatchInfo->lpTargetCode = (BYTE FAR *) MK_FP( lpPatchInfo->wSel, OFFSETOF(lpPatchInfo->lpTargetFunc) );
    lpPatchInfo->lpNewFunc = lpFunc;

    SAVEFUNC( *lpPatchInfo );
    PATCHFUNC( *lpPatchInfo );

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  InitPatch                                                              //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

BOOL InitPatch ( VOID )
{
    HINSTANCE hGdi;
    HINSTANCE hDisplay;

    //-----------------------------------------------------------------------
    //  Patch GDI functions
    //-----------------------------------------------------------------------
    hGdi = GetModuleHandle( "gdi" );
    if( hGdi == NULL )
    {
        MessageBox( NULL, "Could not get handle to GDI", "Error", MB_OK );
        goto fail_0;
    }

    if ( !InitFuncPatch( hGdi, "Escape", NewEscape, &appInfo.piEscape ) ) {
        MessageBox( NULL, "Could not init patch to GDI Escape", "Error", MB_OK );
        goto fail_0;
    }

    return TRUE;

fail_0:
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  FreeFuncPatch                                                          //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

VOID FreeFuncPatch ( PATCHINFO *lpPatchInfo )
{
    UNPATCHFUNC( *lpPatchInfo );

    if( lpPatchInfo->wSel )
    {
        SelectorFree( lpPatchInfo->wSel );
    }
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  FreePatch                                                              //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

VOID FreePatch ( VOID ) {
    DWORD i;
    FreeFuncPatch( &appInfo.piEscape );
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  WindowProc                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

LONG __export FAR PASCAL WindowProc ( HWND     hwnd, 
                                      unsigned msg,
                                      UINT     wparam, 
                                      LONG     lparam )
{
    switch( msg ) 
    {
    case WM_COMMAND:
            break;
    case WM_DESTROY:
        FreePatch();
        PostQuitMessage( 0 );
        break;
    default:
        return( DefWindowProc( hwnd, msg, wparam, lparam ) );
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  WinMain                                                                //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

int PASCAL WinMain( HANDLE hInstance, 
                    HANDLE hPrevious, 
                    LPSTR  cmdline,
                    int    cmdshow ) {
    WNDCLASS wc;
    HWND     hwnd;
    MSG      msg;
    HDC      hdc;
    WORD     wData;

    if( hPrevious )
    {
        return 0;
    }

    memset( &appInfo, 0, sizeof( appInfo ) );

    if( !InitPatch() )
    {
        return 0;
    }

    wc.style         = 0;
    wc.lpfnWndProc   = (LPVOID) WindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "ALTGDI";

    if( !RegisterClass( &wc ) )
    {
        return 0;
    }

    hwnd = CreateWindow( "ALTGDI", 
                         "ALTGDI", 
                         WS_OVERLAPPEDWINDOW,
                         0, 0,
                         100, 100, 
                         NULL, 
                         NULL, 
                         hInstance, 
                         NULL  );

    if( !hwnd ) 
    {
        return 0;
    }

#if 0
    ShowWindow( hwnd, SW_SHOW );
    UpdateWindow( hwnd );
#endif

    while( GetMessage( &msg, NULL, NULL, NULL ) ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
        WaitMessage();
    }

    return msg.wParam;
}



