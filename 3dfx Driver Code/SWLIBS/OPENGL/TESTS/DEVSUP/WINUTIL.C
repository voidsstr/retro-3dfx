#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include <windows.h>
#include <glide.h>
#include <gl/gl.h>
#include <gl/glu.h>

HWND hWndMain;
HDC hDC;

char *sstenv; /* whether to do sst or lfb, based on getenv */
int quit = 0;

static BOOL initApplication( HANDLE hInstance, int nCmdShow );
extern void main( int argc, char **argv);

int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, int nCmdShow )
{
    extern void main(int argc, char **argv);

    if( !initApplication(hInstance, nCmdShow) )
        return FALSE;
    main(0, NULL);
    DestroyWindow(hWndMain);
    return 0;
}

long FAR PASCAL MainWndproc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC         hdc;
    float       width, height;

    switch( message )
    {
    case WM_CREATE:
        break;

    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        return 1;

    case WM_CLOSE:
        break;

    case WM_DESTROY:
        break;

    case WM_MOVE:
        if (sstenv && !grSstControl(GR_CONTROL_MOVE)) {
            PostMessage( hWndMain, WM_CLOSE, 0, 0 );
            return 0;
        }
        break;

    case WM_DISPLAYCHANGE:
    case WM_SIZE:
      {
	RECT    rect;

	GetClientRect(hWndMain, &rect);
	width = (float) rect.right;
	height = (float) rect.bottom;
      }
        if (sstenv && !grSstControl(GR_CONTROL_RESIZE)) {
            PostMessage( hWndMain, WM_CLOSE, 0, 0 );
            return 0;
        }
        break;

    case WM_CHAR:
        if (!isascii(wParam)) break;
	if (tolower(wParam) == 'q') quit = TRUE;
        break;
    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_GENERIC_FORMAT,
	PFD_TYPE_RGBA,					// pixel type
	16, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	0,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    };

static BOOL initApplication( HANDLE hInstance, int nCmdShow )
{
    WNDCLASS    wc;
    BOOL        rc;

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = MainWndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon( NULL, IDI_APPLICATION);    /* generic icon */
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = GetStockObject( BLACK_BRUSH );
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = "WinGlideClass";
    rc = RegisterClass( &wc );
    if( !rc )
    {
        return FALSE;
    }

    hWndMain = CreateWindowEx(
#if 0                         
        WS_EX_APPWINDOW  gives you regular borders?
        WS_EX_TOPMOST    Works as advertised.
#endif
        WS_EX_APPWINDOW,
        "WinGlideClass",
        "Glide Test",
        WS_OVERLAPPED |     
            WS_CAPTION  |     
            WS_THICKFRAME | 
            WS_MAXIMIZEBOX | 
            WS_MINIMIZEBOX | 
            WS_VISIBLE |    /* so we don't have to call ShowWindow */
            WS_POPUP |      /* non-app window */
            WS_SYSMENU,     /* so we get an icon in the tray */
        CW_USEDEFAULT, 
        CW_USEDEFAULT,
        640,                /* GetSystemMetrics(SM_CXSCREEN), */
        480,                /* GetSystemMetrics(SM_CYSCREEN), */
        NULL,
        NULL,
        hInstance,
        NULL );

    if( !hWndMain ) return FALSE;
    ShowWindow( hWndMain, SW_NORMAL);
    UpdateWindow( hWndMain );
    {
    	HGLRC hRC;
	int ipfd;

        hDC = GetDC(hWndMain);
	ipfd = GetPixelFormat(hDC);
	DescribePixelFormat(hDC, ipfd, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	ipfd = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, ipfd, &pfd);

	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRC);
    }
    return TRUE;
}

int getkey( void ) {
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (quit) {
	    quit = 0;
            return 1;
        }
    }
    return 0;
}
