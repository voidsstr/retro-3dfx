/*
	FILE:	wlglib.cpp

	Implementation of sample wgl functionality.

	Paul Mayfield, 7/3/96
*/

#include <assert.h>
#include <windows.h>
#include "winlib.h"
#include "wgllib.h"
#include <GL/gl.h>

extern "C" {
	int left=0, top=0;
}

HPALETTE ghPalette = NULL;
HPALETTE ghpalOld = NULL;

wlbInfo::wlbInfo() {
	init = NULL;
	display = NULL;
	reshape = NULL;
	mouse = NULL;
	pfdsetup = NULL;
}

wlbInfo::~wlbInfo() {
}

// HERE'S OUR WINDOW PROCEDURE
LONG WINAPI	wlbWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static RECT	rect;
	static RECT	oldrect;
    LONG    lRet = 1;
	wlbInfo * info;

	// RETRIEVE CORROSPONDING INFO OF THIS WINDOW
	info = (wlbInfo*)wlGetWindowParam(hWnd);

    switch (uMsg)
    {
    	case WM_CREATE:
        {
	    	HDC	  hDC;
			HGLRC hRC;

			// RETRIEVE THE POINTER TO THE CORROSPONDING INFO 
			wlbInfo * info;
			info = (wlbInfo*)wlGetCreationData(lParam);
			assert(info);

			// SAVE THE INFO POINTER AS PARAMETER OF THIS WINDOW
			wlAddWindowParam(hWnd, (DWORD)info);

			// SET UP THE PIXEL FORMAT OF THIS WINDOW'S DC
            hDC = GetDC(hWnd);
	    	(*(info->pfdsetup))(hDC);

			// CREATE THE WINDOW CONTEXT
            hRC = wglCreateContext(hDC);
            wglMakeCurrent(hDC, hRC);

			// PERFORM OPENGL INITIALIZATION
			PostMessage(hWnd, WM_INIT, 0, 0);

			return 0;
		}
		break;


    	case WM_PAINT:
        { 
		    HDC	hDC;
		    PAINTSTRUCT	ps;
		    hDC = BeginPaint(hWnd, &ps);
		    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			if (info && info->display)	
				(*(info->display))();
			RECT r; r.top=0; r.bottom =50; r.left=0; r.right=50;
			//FillRect(hDC, &r, GetStockObject(LTGRAY_BRUSH));
		    EndPaint(hWnd, &ps);
        }
		break;

    	case WM_INIT:
	        if (info && info->init)
				(*(info->init))();
	        break;

    	case WM_SIZE:
		    GetClientRect(hWnd, &rect);
			if (info && info->reshape) 
				(*(info->reshape))(LOWORD(lParam), HIWORD(lParam));
		    if((oldrect.right > rect.right) || (oldrect.bottom > rect.bottom))
				PostMessage (hWnd, WM_PAINT, 0, 0L);
			oldrect.right = rect.right;
		    oldrect.bottom = rect.bottom;
			return 0;

        // The WM_QUERYNEWPALETTE message informs a window that it is about to
        // receive input focus. In response, the window receiving focus should
        // realize its palette as a foreground palette and update its client
        // area. If the window realizes its palette, it should return TRUE;
        // otherwise, it should return FALSE.
        case WM_QUERYNEWPALETTE:
        {
            HDC     hDC;
            if(ghPalette)
            {
                hDC = GetDC(hWnd);

                // Select and realize the palette

                ghpalOld = SelectPalette(hDC, ghPalette, FALSE);
                RealizePalette(hDC);

                // Redraw the client area

                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);

                if(ghpalOld)
                    SelectPalette(hDC, ghpalOld, FALSE);

                ReleaseDC(hWnd, hDC);

                return TRUE;
            }

            return FALSE;
        }

        // The WM_PALETTECHANGED message informs all windows that the window
        // with input focus has realized its logical palette, thereby changing 
        // the system palette. This message allows a window without input focus
        // that uses a color palette to realize its logical palettes and update
        // its client area.
        //
        // This message is sent to all windows, including the one that changed
        // the system palette and caused this message to be sent. The wParam of
        // this message contains the handle of the window that caused the system
        // palette to change. To avoid an infinite loop, care must be taken to
        // check that the wParam of this message does not match the window's
        // handle.
        case WM_PALETTECHANGED:
        {
            HDC         hDC; 


            // Before processing this message, make sure we
            // are indeed using a palette

            if (ghPalette)
            {
                // If this application did not change the palette, select
                // and realize this application's palette

                if (wParam != (WPARAM)hWnd)
                {
                    // Need the window's DC for SelectPalette/RealizePalette

                    hDC = GetDC(hWnd);

                    // Select and realize our palette

                    ghpalOld = SelectPalette(hDC, ghPalette, FALSE);
                    RealizePalette(hDC);

                    // WHen updating the colors for an inactive window,
                    // UpdateColors can be called because it is faster than
                    // redrawing the client area (even though the results are
                    // not as good)

                    UpdateColors(hDC);

                    // Clean up

                    if (ghpalOld)
                       SelectPalette(hDC, ghpalOld, FALSE);

                    ReleaseDC(hWnd, hDC);
                }
            }
            break;
        }

		/* 
    	case WM_COMMAND:
            lRet = CommandHandler (hWnd, wParam, lParam);
            break;
		*/
		case WM_KEYDOWN:
			{
				if (wParam == VK_ESCAPE)
					PostQuitMessage(0);
				else if (info && info->keyboard) {
					int key = (int)wParam;
					if (key > 'A' && key < 'Z')
						key = (int)('a' + (key - 'A'));
					(*(info->keyboard))(key, 0,0);
					PostMessage (hWnd, WM_PAINT, 0, 0L);
				}
				break;
			}

   	    case WM_CLOSE:
        {
    	    HGLRC hRC;
    	    HDC	  hDC;

                /* release and free the device context and rendering context */
    	    hRC = wglGetCurrentContext();
    	    hDC = wglGetCurrentDC();

    	    wglMakeCurrent(NULL, NULL);

    	    if (hRC)
    	    	wglDeleteContext(hRC);
    	    if (hDC)
    	        ReleaseDC(hWnd, hDC);

                /* call destroy window to cleanup and go away */
                DestroyWindow (hWnd);
        }
        break;

   	    case WM_DESTROY:
        {
    	    HGLRC hRC;
    	    HDC	  hDC;

                /* release and free the device context and rendering context */
    	    hRC = wglGetCurrentContext();
    	    hDC = wglGetCurrentDC();

    	    wglMakeCurrent(NULL, NULL);

    	    if (hRC)
    	    	wglDeleteContext(hRC);
    	    if (hDC)
    	        ReleaseDC(hWnd, hDC);

                PostQuitMessage (0);
        }
        break;

    	default:
            /* pass all unhandled messages to DefWindowProc */
            lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
        break;
    }

    /* return 1 if handled message, 0 if not */
    return lRet;
}



// ==================================================================
// ==================================================================
// ==================================================================
//		LIBRARY ENTRY POINTS
// ==================================================================
// ==================================================================
// ==================================================================
char * Bool2String (BOOL boolval) {
	if (boolval)
		return "Yes";
	else
		return "No";
}

// OUTPUTS INFORMATION CONCERNING THE PFD
void wlbOutputPFD (FILE * file, HDC hdc, int pfdIndex) {
	// GET THE PIXEL FORMAT DESCRIPTOR
	PIXELFORMATDESCRIPTOR pfd;
	if (! DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
		return;
	
	fprintf(file, "PFD Index: %03d      HDC: %04x\n", pfdIndex, hdc); 
	fprintf(file, "=================================\n", pfdIndex, hdc); 
	fprintf(file, "%-30s%d\n", "PFD Version:", wlbGetPFDVersion(&pfd));
	fprintf(file, "%-30s%s\n", "Can draw to window:", Bool2String(wlbCanDrawToWindow(&pfd)));
	fprintf(file, "%-30s%s\n", "Can draw to bitmap:", Bool2String(wlbCanDrawToBitmap(&pfd)));
	fprintf(file, "%-30s%s\n", "Supports GDI:",		Bool2String(wlbSupportsGDI(&pfd)));
	fprintf(file, "%-30s%s\n", "Supports OpenGL",	Bool2String(wlbSupportsOpenGL(&pfd)));
	fprintf(file, "%-30s%s\n", "Hardware PFD:",		Bool2String(wlbIsDriverPFD(&pfd)));
	fprintf(file, "%-30s%s\n", "Needs Palette",		Bool2String(wlbNeedsPalette(&pfd)));
	fprintf(file, "%-30s%s\n", "Needs System Palette", Bool2String(wlbNeedsSystemPalette(&pfd)));
	fprintf(file, "%-30s%s\n", "Double Buffered:",	Bool2String(wlbIsDoubleBuffered(&pfd)));
	fprintf(file, "%-30s%s\n", "Stereo:",			Bool2String(wlbIsStereo(&pfd)));
	fprintf(file, "%-30s%s\n", "Swaps Inidivid Layers:", Bool2String(wlbCanSwapIndividualLayers(&pfd)));
	fprintf(file, "%-30s%s\n", "Type RGBA:",		Bool2String(wlbIsRGBA(&pfd)));
	fprintf(file, "%-30s%s\n", "Type Color Index:", Bool2String(wlbIsColorIndex(&pfd)));
	fprintf(file, "%-30s%d\n", "Color Depth:", wlbGetColorDepth(&pfd));
	fprintf(file, "%-30s%d\n", "Red Bits:", wlbGetRedBits(&pfd));
	fprintf(file, "%-30s%d\n", "Green Bits:", wlbGetGreenBits(&pfd));
	fprintf(file, "%-30s%d\n", "Blue Bits:", wlbGetBlueBits(&pfd));
	fprintf(file, "%-30s%d\n", "Alpha Bits:", wlbGetAlphaBits(&pfd));
	fprintf(file, "%-30s%d\n", "Accumulation Depth:", wlbGetAccumBits(&pfd));
	fprintf(file, "%-30s%d\n", "Accum Red Bits:", wlbGetAccumRedBits(&pfd));
	fprintf(file, "%-30s%d\n", "Accum Green Bits:", wlbGetAccumGreenBits(&pfd));
	fprintf(file, "%-30s%d\n", "Accum Blue Bits:", wlbGetAccumBlueBits(&pfd));
	fprintf(file, "%-30s%d\n", "Accum Alpha Bits:", wlbGetAccumAlphaBits(&pfd));
	fprintf(file, "%-30s%d\n", "Depth Bits (Z):", wlbGetDepthBits(&pfd));
	fprintf(file, "%-30s%d\n", "Stencil Bits:", wlbGetStencilBits(&pfd));
	fprintf(file, "%-30s%d\n", "Aux Buffers:", wlbGetAuxBufferCount(&pfd));
	fprintf(file, "%-30s%d\n", "Layer Plane Count:", wlbGetLayerPlaneCount(&pfd));
	fprintf(file, "%-30s%x\n", "Underlay Trans. Color:", wlbGetUnderlayTransparentClr(&pfd));
}

void wlbInitPFD	(PIXELFORMATDESCRIPTOR * pfd) {
	if (!pfd)
		return;
	memset(pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
}

// FUNCTIONS TO QUERY STATE OF A PFD
WORD wlbGetPFDVersion (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.nVersion;   
}

BOOL wlbCanDrawToWindow (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
	return (pfd.dwFlags & PFD_DRAW_TO_WINDOW);
}

BOOL wlbCanDrawToBitmap (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
	return (pfd.dwFlags & PFD_DRAW_TO_BITMAP);
}

BOOL wlbSupportsOpenGL (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
	return (pfd.dwFlags & PFD_SUPPORT_OPENGL);
}

BOOL wlbSupportsGDI	(HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
	return (pfd.dwFlags & PFD_SUPPORT_GDI);
}

BOOL wlbIsDriverPFD	(HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
	return (!(pfd.dwFlags & PFD_SUPPORT_OPENGL));
}

BOOL wlbNeedsPalette (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
	return (pfd.dwFlags & PFD_NEED_PALETTE);
}

BOOL wlbNeedsSystemPalette (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
	return (pfd.dwFlags & PFD_NEED_SYSTEM_PALETTE);
}

BOOL wlbIsDoubleBuffered (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
	return (pfd.dwFlags & PFD_DOUBLEBUFFER);
}

BOOL wlbIsStereo (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
	return (pfd.dwFlags & PFD_STEREO);
}

BOOL wlbCanSwapIndividualLayers (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
	//return (pfd.dwFlags & PFD_SWAP_LAYER_BUFFERS);
	return 0;
}

BOOL wlbSwapCopyExtensionSet (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
//	return (pfd.dwFlags & PFD_SWAP_COPY);
	return 0;
}

BOOL wlbSwapExchangeSet (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
//	return (pfd.dwFlags & PFD_SWAP_EXCHANGE);
	return 0;
}

BOOL wlbIsRGBA	(HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
	return (pfd.iPixelType == PFD_TYPE_RGBA);
}

BOOL wlbIsColorIndex (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return -1;
	return (pfd.iPixelType == PFD_TYPE_COLORINDEX);
}

BYTE wlbGetColorDepth (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cColorBits;
}

BYTE wlbGetRedBits (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cRedBits;
}

BYTE wlbGetGreenBits (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cGreenBits;
}

BYTE wlbGetBlueBits (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cBlueBits;
}

BYTE wlbGetAlphaBits (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cAlphaBits;
}

BYTE wlbGetAccumBits (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cAccumBits;
}

BYTE wlbGetAccumRedBits	(HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cAccumRedBits;
}

BYTE wlbGetAccumGreenBits (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cAccumGreenBits;
}

BYTE wlbGetAccumBlueBits (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cAccumBlueBits;
}

BYTE wlbGetAccumAlphaBits (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cAccumAlphaBits;
}

BYTE wlbGetDepthBits (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cDepthBits;
}

BYTE wlbGetStencilBits (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cStencilBits;
}

BYTE wlbGetAuxBufferCount (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.cAuxBuffers;
}

BYTE wlbGetLayerPlaneCount (HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.bReserved;
}

DWORD wlbGetUnderlayTransparentClr(HDC hdc, int pfdIndex) {
	PIXELFORMATDESCRIPTOR pfd;
	int success = DescribePixelFormat(hdc, pfdIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if (!success) 
		return 0;
	return pfd.dwVisibleMask;
}

// VERSIONS OF FUNCTIONS TO QUERY STATE OF PFD THAT TAKE PFD'S AS PARAMS
WORD wlbGetPFDVersion (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->nVersion;   
}

BOOL wlbCanDrawToWindow (PIXELFORMATDESCRIPTOR * pfd) {
	return (pfd->dwFlags & PFD_DRAW_TO_WINDOW);
}

BOOL wlbCanDrawToBitmap (PIXELFORMATDESCRIPTOR * pfd) {
	return (pfd->dwFlags & PFD_DRAW_TO_BITMAP);
}

BOOL wlbSupportsOpenGL (PIXELFORMATDESCRIPTOR * pfd) {
	return (pfd->dwFlags & PFD_SUPPORT_OPENGL);
}

BOOL wlbSupportsGDI	(PIXELFORMATDESCRIPTOR * pfd) {
	return (pfd->dwFlags & PFD_SUPPORT_GDI);
}

BOOL wlbIsDriverPFD	(PIXELFORMATDESCRIPTOR * pfd) {
	return (!(pfd->dwFlags & PFD_SUPPORT_OPENGL));
}

BOOL wlbNeedsPalette (PIXELFORMATDESCRIPTOR * pfd) {
	return (pfd->dwFlags & PFD_NEED_PALETTE);
}

BOOL wlbNeedsSystemPalette (PIXELFORMATDESCRIPTOR * pfd) {
	return (pfd->dwFlags & PFD_NEED_SYSTEM_PALETTE);
}

BOOL wlbIsDoubleBuffered (PIXELFORMATDESCRIPTOR * pfd) {
	return (pfd->dwFlags & PFD_DOUBLEBUFFER);
}

BOOL wlbIsStereo (PIXELFORMATDESCRIPTOR * pfd) {
	return (pfd->dwFlags & PFD_STEREO);
}

BOOL wlbCanSwapIndividualLayers (PIXELFORMATDESCRIPTOR * pfd) {
	//return (pfd->dwFlags & PFD_SWAP_LAYER_BUFFERS);
	return 0;
}

BOOL wlbSwapCopyExtensionSet (PIXELFORMATDESCRIPTOR * pfd) {
//	return (pfd->dwFlags & PFD_SWAP_COPY);
    return 0;
}

BOOL wlbSwapExchangeSet (PIXELFORMATDESCRIPTOR * pfd) {
//	return (pfd->dwFlags & PFD_SWAP_EXCHANGE);
    return 0;
}

BOOL wlbIsRGBA	(PIXELFORMATDESCRIPTOR * pfd) {
	return (pfd->iPixelType == PFD_TYPE_RGBA);
}

BOOL wlbIsColorIndex (PIXELFORMATDESCRIPTOR * pfd) {
	return (pfd->iPixelType == PFD_TYPE_COLORINDEX);
}

BYTE wlbGetColorDepth (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cColorBits;
}

BYTE wlbGetRedBits (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cRedBits;
}

BYTE wlbGetGreenBits (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cGreenBits;
}

BYTE wlbGetBlueBits (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cBlueBits;
}

BYTE wlbGetAlphaBits (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cAlphaBits;
}

BYTE wlbGetAccumBits (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cAccumBits;
}

BYTE wlbGetAccumRedBits	(PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cAccumRedBits;
}

BYTE wlbGetAccumGreenBits (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cAccumGreenBits;
}

BYTE wlbGetAccumBlueBits (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cAccumBlueBits;
}

BYTE wlbGetAccumAlphaBits (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cAccumAlphaBits;
}

BYTE wlbGetDepthBits (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cDepthBits;
}

BYTE wlbGetStencilBits (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cStencilBits;
}

BYTE wlbGetAuxBufferCount (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->cAuxBuffers;
}

BYTE wlbGetLayerPlaneCount (PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->bReserved;
}

DWORD wlbGetUnderlayTransparentClr(PIXELFORMATDESCRIPTOR * pfd) {
	return pfd->dwVisibleMask;
}


// FUNCTIONS TO SET THE STATE OF A PFD
BOOL wlbSetPFDVersion (PIXELFORMATDESCRIPTOR * pfd, WORD version) {
	pfd->nVersion = version;
	return 1;
}

BOOL wlbSetDrawToWindow (PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (state)
		pfd->dwFlags |= PFD_DRAW_TO_WINDOW;
	else
		pfd->dwFlags &= ~PFD_DRAW_TO_WINDOW;
	return 1;
}

BOOL wlbSetDrawToBitmap (PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (state)
		pfd->dwFlags |= PFD_DRAW_TO_BITMAP;
	else
		pfd->dwFlags &= ~PFD_DRAW_TO_BITMAP;
	return 1;
}

BOOL wlbSetSupportGDI (PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (state) {
		pfd->dwFlags |= PFD_SUPPORT_GDI;
		pfd->dwFlags &= ~PFD_DOUBLEBUFFER;
	}
	else
		pfd->dwFlags &= ~PFD_SUPPORT_GDI;
	return 1;
}

BOOL wlbSetSupportOpenGL (PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (state) 
		pfd->dwFlags |= PFD_SUPPORT_OPENGL;
	else
		pfd->dwFlags &= ~PFD_SUPPORT_OPENGL;
	return 1;
}

BOOL wlbSetDriverPFD (PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (state) 
		pfd->dwFlags &= ~PFD_GENERIC_FORMAT;
	else
		pfd->dwFlags |= PFD_GENERIC_FORMAT;
	return 1;
}

BOOL wlbSetNeedsPalette (PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (state) 
		pfd->dwFlags |= PFD_NEED_PALETTE;
	else
		pfd->dwFlags &= ~PFD_NEED_PALETTE;
	return 1;
}

BOOL wlbSetNeedsSystemPalette (PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (state) 
		pfd->dwFlags |= PFD_NEED_SYSTEM_PALETTE;
	else
		pfd->dwFlags &= ~PFD_NEED_SYSTEM_PALETTE;
	return 1;
}

BOOL wlbSetDoubleBuffer	(PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (state) {
		pfd->dwFlags |= PFD_DOUBLEBUFFER;
		pfd->dwFlags &= ~PFD_SUPPORT_GDI;
	}
	else
		pfd->dwFlags &= ~PFD_DOUBLEBUFFER;
	return 1;
}

BOOL wlbSetSingleBuffer	(PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (!state) {
		pfd->dwFlags |= PFD_DOUBLEBUFFER;
		pfd->dwFlags &= ~PFD_SUPPORT_GDI;
	}
	else
		pfd->dwFlags &= ~PFD_DOUBLEBUFFER;
	return 1;
}

BOOL wlbSetStereo (PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (state)
		pfd->dwFlags |= PFD_STEREO;
	else
		pfd->dwFlags &= ~PFD_STEREO;
	return 1;
}

BOOL wlbSetDBDontCare (PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (state)
		pfd->dwFlags |= PFD_DOUBLEBUFFER_DONTCARE;
	else
		pfd->dwFlags &= ~PFD_DOUBLEBUFFER_DONTCARE;
	return 1;
}

BOOL wlbSetStereoDontCare (PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (state)
		pfd->dwFlags |= PFD_STEREO_DONTCARE;
	else
		pfd->dwFlags &= ~PFD_STEREO_DONTCARE;
	return 1;
}

BOOL wlbSetCanSwapIndLayers	(PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	// (state)
	//pfd->dwFlags |= PFD_SWAP_LAYER_BUFFERS;
	//else
	//	pfd->dwFlags &= ~PFD_SWAP_LAYER_BUFFERS;
	return 0;
}

BOOL wlbSetRGBA	(PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (state) {
		pfd->iPixelType = PFD_TYPE_RGBA;
		pfd->dwFlags &= ~PFD_NEED_PALETTE;
	} else {
		pfd->iPixelType = PFD_TYPE_COLORINDEX;
		pfd->dwFlags |= PFD_NEED_PALETTE;
	}
	return 1;
}

BOOL wlbSetColorIndex (PIXELFORMATDESCRIPTOR * pfd, BOOL state) {
	if (!state) {
		pfd->iPixelType = PFD_TYPE_RGBA;
		pfd->dwFlags &= ~PFD_NEED_PALETTE;
	}
	else {
		pfd->iPixelType = PFD_TYPE_COLORINDEX;
		pfd->dwFlags |= PFD_NEED_PALETTE;
	}
	return 1;
}

BOOL wlbSetColorDepth (PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cColorBits = value;
	return 1;
}

BOOL wlbSetRedBits (PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cRedBits = value;
	return 1;
}

BOOL wlbSetGreenBits (PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cGreenBits = value;
	return 1;
}

BOOL wlbSetBlueBits	(PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cBlueBits = value;
	return 1;
}

BOOL wlbSetAlphaBits (PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cAlphaBits = value;
	return 1;
}

BOOL wlbSetAccumBits (PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cAccumBits = value;
	return 1;
}

BOOL wlbSetAccumRedBits	(PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cAccumRedBits = value;
	return 1;
}

BOOL wlbSetAccumGreenBits (PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cAccumGreenBits = value;
	return 1;
}

BOOL wlbSetAccumBlueBits (PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cAccumBlueBits = value;
	return 1;
}

BOOL wlbSetAccumAlphabits (PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cAccumAlphaBits = value;
	return 1;
}

BOOL wlbSetDepth (PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cDepthBits = value;
	return 1;
}

BOOL wlbSetStencilBits (PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cStencilBits = value;
	return 1;
}

BOOL wlbSetAuxBufferCount (PIXELFORMATDESCRIPTOR * pfd, BYTE value) {
	pfd->cAuxBuffers = value;
	return 1;
}

BOOL wlbSetUnderlayTransparentClr (PIXELFORMATDESCRIPTOR * pfd, DWORD value) {
	pfd->dwVisibleMask = value;
	return 1;
}

unsigned char wlbthreeto8[8] = {
    0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
};

unsigned char wlbtwoto8[4] = {
    0, 0x55, 0xaa, 0xff
};

unsigned char wlboneto8[2] = {
    0, 255
};

static int wlbdefaultOverride[13] = {
    0, 3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
};

static PALETTEENTRY wlbdefaultPalEntry[20] = {
    { 0,   0,   0,    0 },
    { 0x80,0,   0,    0 },
    { 0,   0x80,0,    0 },
    { 0x80,0x80,0,    0 },
    { 0,   0,   0x80, 0 },
    { 0x80,0,   0x80, 0 },
    { 0,   0x80,0x80, 0 },
    { 0xC0,0xC0,0xC0, 0 },

    { 192, 220, 192,  0 },
    { 166, 202, 240,  0 },
    { 255, 251, 240,  0 },
    { 160, 160, 164,  0 },

    { 0x80,0x80,0x80, 0 },
    { 0xFF,0,   0,    0 },
    { 0,   0xFF,0,    0 },
    { 0xFF,0xFF,0,    0 },
    { 0,   0,   0xFF, 0 },
    { 0xFF,0,   0xFF, 0 },
    { 0,   0xFF,0xFF, 0 },
    { 0xFF,0xFF,0xFF, 0 }
};

unsigned char wlbComponentFromIndex(int i, UINT nbits, UINT shift) {
    unsigned char val;

    val = (unsigned char) (i >> shift);
    switch (nbits) {
		case 1:
			val &= 0x1;
			return wlboneto8[val];

		case 2:
			val &= 0x3;
			return wlbtwoto8[val];

		case 3:
			val &= 0x7;
			return wlbthreeto8[val];

		default:
			return 0;
    }
}

HPALETTE WINAPI	wlbCreateRGBPalette(HDC hDC) {
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE *pPal;
    int n, i;

    n = GetPixelFormat(hDC);
    DescribePixelFormat(hDC, n, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    if (pfd.dwFlags & PFD_NEED_PALETTE) { //wlbIsColorIndex(&pfd)) {
        n = 1 << pfd.cColorBits;
        pPal = (PLOGPALETTE)LocalAlloc(LMEM_FIXED, sizeof(LOGPALETTE) +
                n * sizeof(PALETTEENTRY));
        pPal->palVersion = 0x300;
        pPal->palNumEntries = n;
        for (i=0; i<n; i++) {
            pPal->palPalEntry[i].peRed =
                    wlbComponentFromIndex(i, pfd.cRedBits, pfd.cRedShift);
            pPal->palPalEntry[i].peGreen =
                    wlbComponentFromIndex(i, pfd.cGreenBits, pfd.cGreenShift);
            pPal->palPalEntry[i].peBlue =
                    wlbComponentFromIndex(i, pfd.cBlueBits, pfd.cBlueShift);
            pPal->palPalEntry[i].peFlags = 0;
        }

        /* fix up the palette to include the default GDI palette */
        if ((pfd.cColorBits == 8)                           &&
            (pfd.cRedBits   == 3) && (pfd.cRedShift   == 0) &&
            (pfd.cGreenBits == 3) && (pfd.cGreenShift == 3) &&
            (pfd.cBlueBits  == 2) && (pfd.cBlueShift  == 6)
           ) {
            for (i = 1 ; i <= 12 ; i++)
                pPal->palPalEntry[wlbdefaultOverride[i]] = wlbdefaultPalEntry[i];
        }

        ghPalette = CreatePalette(pPal);
		DWORD error;
		if(!ghPalette)
			error = GetLastError();
        LocalFree(pPal);

        ghpalOld = SelectPalette(hDC, ghPalette, FALSE);
        n = RealizePalette(hDC);
    }

	return ghPalette;
}

void WINAPI	wlbPfdSetup (HDC hDC) {
    static PIXELFORMATDESCRIPTOR pfd;

	wlbInitPFD(&pfd);
	wlbSetPFDVersion(&pfd,1);
	wlbSetDrawToWindow(&pfd);
	wlbSetSupportOpenGL(&pfd);
	wlbSetDoubleBuffer(&pfd);
	wlbSetRGBA(&pfd);
	wlbSetColorDepth(&pfd, 16);
	wlbSetDepth(&pfd, 8);

    int pixelformat;

    if ((pixelformat=ChoosePixelFormat(hDC, &pfd))==0) {
        MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
        return;
    }

    if (SetPixelFormat(hDC, pixelformat, &pfd)==FALSE) {
        MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
        return;
    }

    wlbCreateRGBPalette(hDC);

    return;
}
