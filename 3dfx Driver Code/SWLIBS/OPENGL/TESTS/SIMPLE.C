#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include <windows.h>
#include <glide.h>
#include <gl/gl.h>
#include <gl/glu.h>

#pragma warning(disable : 4244)

/* Forward declarations */
HWND hWndMain;
HDC hDC;

static int Res = GR_RESOLUTION_640x480;

#define CHKERROR() \
{ \
    int e = glGetError(); \
    if (e) printf("got error %d at line %d\n", e, __LINE__); \
}

static void
init(void) {
    glMatrixMode(GL_PROJECTION);
    glOrtho(-2,2,-2,2,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1,0,0,1);
}

static void
draw_scene(void) {
    static float ang = 0;

    glLoadIdentity();
    glRotatef(ang,0,0,1);
    ang += 1;

    glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT);
    glColor3f(1,1,0);
    glBegin(GL_TRIANGLE_STRIP);
    	glVertex2f(-1,-1); 
    	glVertex2f(-1, 1); 
    	glVertex2f( 1,-1); 
    	glVertex2f( 1, 1); 
    glEnd();
    CHKERROR();
}

int quit = 0;

int getkey( void ) {
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (quit) {
            return 1;
        }
    }
    return 0;
}

void main( int argc, char **argv) {
    int swapinterval = 5;
    int frames;

    init();
    frames = 1000/swapinterval;
    while (frames--) {
	draw_scene();
	SwapBuffers(hDC);
    }
    while (!getkey());
}

/*
 * MainWndproc
 *
 * Callback for all Windows messages
 */

long FAR PASCAL MainWndproc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC         hdc;
    float       width, height;

    switch( message )
    {
    case WM_SETCURSOR:
        if (Res != GR_RESOLUTION_NONE) {
            SetCursor(NULL);
            return 0;
        }
        break;

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
	if (tolower(wParam) == 'q') quit = TRUE;
        break;

    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);

} /* MainWndproc */

static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
#if 1
	PFD_DOUBLEBUFFER |
#endif
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

/*
 * initApplication
 *
 * Do that Windows initialization stuff...
 */
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
        200,                /* GetSystemMetrics(SM_CXSCREEN), */
        200,                /* GetSystemMetrics(SM_CYSCREEN), */
        NULL,
        NULL,
        hInstance,
        NULL );

    if( !hWndMain )
    {
        return FALSE;
    }

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

} /* initApplication */

/*
 * WinMain
 */
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, int nCmdShow )
{
    if( !initApplication(hInstance, nCmdShow) )
        return FALSE;

    {
        int     argc;
        char    **argv;
        extern void main(int argc, char **argv);

        main(argc, argv);
    }

    printf("Exiting winMain()\n");
    fflush(stdout);

    DestroyWindow(hWndMain);
    return 0;

} /* WinMain */
