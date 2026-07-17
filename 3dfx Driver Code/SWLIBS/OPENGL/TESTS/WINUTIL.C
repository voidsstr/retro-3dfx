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
int quit = 0;

extern void main( int argc, char **argv);
static char **commandLineToArgv(LPSTR lpCmdLine, int *pArgc);

/*
** Currently not used; we compile with subsystem:console.
*/
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
		    LPSTR lpCmdLine, int nCmdShow )
{
    int argc;
    char **argv;

    argv = commandLineToArgv(lpCmdLine, &argc);
    main(argc, argv);
    DestroyWindow(hWndMain);
    return 0;
}

/*
 * Converts lpCmdLine to WinMain into argc, argv
 */
static char    *fakeName = "WinTest";
static char    *argvbuf[32];
static char    cmdLineBuffer[1024];
static char **
commandLineToArgv(LPSTR lpCmdLine, int *pArgc)
{
    char    *p, *pEnd;
    int     argc = 0;

    argvbuf[argc++] = fakeName;

    if (lpCmdLine == NULL) {
        *pArgc = argc;
         return argvbuf;
    }

    strcpy(cmdLineBuffer, lpCmdLine);
    p = cmdLineBuffer;
    pEnd = p + strlen(cmdLineBuffer);
    if (pEnd >= &cmdLineBuffer[1022]) pEnd = &cmdLineBuffer[1022];

    fflush(stdout);

    while (1) {
        /* skip over white space */
        fflush(stdout);

        while (*p == ' ') p++;
        if (p >= pEnd) break;

        argvbuf[argc++] = p;
        if (argc >= 32) break;

        /* skip till there's a 0 or a white space */
        while (*p && (*p != ' ')) p++;

        if (*p == ' ') *p++ = 0;
    }

    *pArgc = argc;
    return argvbuf;
}

int gotkey;

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
        break;

    case WM_DISPLAYCHANGE:
    case WM_SIZE:
      {
	RECT    rect;

	GetClientRect(hWndMain, &rect);
	width = (float) rect.right;
	height = (float) rect.bottom;
      }
        break;

    case WM_CHAR:
        if (!isascii(wParam)) break;
	gotkey = wParam;
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
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    };

BOOL initApplication( HANDLE hInstance, int nCmdShow, BOOL db )
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

    if (db) {
        pfd.dwFlags |= PFD_DOUBLEBUFFER;
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
	gotkey = 0;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
	return gotkey;
    }
    return 0;
}

void swap(void) {
    SwapBuffers(hDC);
}
