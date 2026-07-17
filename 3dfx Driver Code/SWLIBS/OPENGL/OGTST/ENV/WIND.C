/**************************************************************************
 *									  *
 * 	      Copyright (C) 1990-1997, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "ogtst.h"
#include "env.h"

HINSTANCE hGlobalInst;
static char ogtstClassName[] = "OpenGL Test";

extern opts_t opts;
extern wist_t windata;
extern Test *ctst;
const char *gtst_renderer;
static int hasPbuffers;
static int glversion;

#define MAX_VISUALS 256

/*
 * All information that distinguishes two visuals such that one may want to
 * run tests on both visuals should be put above the vi field in the record.
 * The code the check whether two visuals are distinguishable relies on this.
 */
typedef struct {
    int buffer_size;
    int level;
    /* When a dual personality fbconfig is used, this is set dynamically */
    int rgba;
    int double_buffer;
    int stereo;
    int aux_buffers;
    int red_size;
    int green_size;
    int blue_size;
    int alpha_size;
    int depth_size;
    int stencil_size;
    int acc_red_size;
    int acc_green_size;
    int acc_blue_size;
    int acc_alpha_size;
    /*
     * All information that distinguishes two visuals from each other should be
     * put above this comment!!
     * Hack memcmp code relies on this.
     */
    int pixelFormat;
    DWORD dwFlags;
    int transparent;
    HGLRC cx;
    HWND window;
    HDC hDC;
    HPALETTE hPalette;
}  visfoRec, *visfoRecP;

visfoRec visfo[MAX_VISUALS];

#ifndef WIN32
struct pixmapRec {
    GLXPixmap glxpixmap;
    Pixmap pixmap;
} pixmap[MAX_VISUALS];
#endif

static short dfltcmap[][3] = {
    {0, 0, 0},
    {255, 0, 0},
    {0, 255, 0},
    {255, 255, 0},
    {0, 0, 255},
    {255, 0, 255},
    {0, 255, 255},
    {255, 255, 255},

    {85, 85, 85},
    {198, 113, 113},
    {113, 198, 113},
    {142, 142, 56},
    {113, 113, 198},
    {142, 56, 142},
    {56, 142, 142},
    {170, 170, 170},
    {170, 170, 170},
    {85, 85, 85},
    {170, 170, 170},
    {85, 85, 85},
    {170, 170, 170},
    {85, 85, 85},
    {170, 170, 170},
    {85, 85, 85},
    {170, 170, 170},
    {85, 85, 85},
    {170, 170, 170},
    {85, 85, 85},
    {170, 170, 170},
    {85, 85, 85},
    {170, 170, 170},
    {85, 85, 85},
    {10, 10, 10},
    {20, 20, 20},
    {30, 30, 30},
    {40, 40, 40},
    {51, 51, 51},
    {61, 61, 61},
    {71, 71, 71},
    {81, 81, 81},
    {91, 91, 91},
    {102, 102, 102},
    {112, 112, 112},
    {122, 122, 122},
    {132, 132, 132},
    {142, 142, 142},
    {153, 153, 153},
    {163, 163, 163},
    {173, 173, 173},
    {183, 183, 183},
    {193, 193, 193},
    {204, 204, 204},
    {214, 214, 214},
    {224, 224, 224},
    {234, 234, 234},
    {244, 244, 244},
    {-1, -1, -1}
};

static void	createWindow(char *, int, int, int, int, visfoRecP);
static void	getVisuals(HDC hDC);
static int	queryVisual(int,int);
static void	SetupDC(visfoRec *v, HDC hDC);
static void	SetupPalette(visfoRec *v);

/****************************************************************************
* ogEnvInitVisual()
****************************************************************************/
int
ogEnvInitVisual(int vindex)
{
    visfoRecP v = visfo + vindex;

    windata.cur_visual = vindex;

    if (v->window) {
        if (opts.debugged & 8) {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(v->cx);
            v->cx = wglCreateContext(v->hDC);
            if (v->cx == NULL)
                /* This will exit(1) */
                ogEnvLog(OG_LINTERNALERROR, "Could not create context\n");
        }

        if (!opts.doingPbuffer) {
	    BringWindowToTop(v->window);
            if (!wglMakeCurrent(v->hDC, v->cx))
                /* This will exit(1) */
                ogEnvLog(OG_LINTERNALERROR, "ogEnvInitVisual(): Can't make "
			 "%s current to context for pixelFormat #%x\n",
			 (opts.flags & FLAG_PIXMAP) ? "pixmap" : "window",
			 v->pixelFormat);
        }
    } else {
        createWindow("", windata.wx0, windata.wx1, windata.wy0, windata.wy1, v);
    }

    if (opts.doingAuxBuffer) {
        glDrawBuffer(GL_AUX0);
        glReadBuffer(GL_AUX0);
    }

    ogEnvLog(1, "Visual ID:\t%s0x%x\n", opts.doingPbuffer ? "Pb " : "",
             v->pixelFormat);
    if (opts.plvl > 1) {
        char visDesc[256];
        char strInd = 0;

        if (v->rgba)
            strInd += sprintf(&visDesc[strInd], "rgba(%d,%d,%d,%d),",
                              v->red_size, v->green_size,
                              v->blue_size, v->alpha_size);
        else
            sprintf(&visDesc[strInd], "idx=%d,", v->buffer_size);
        strInd += sprintf(&visDesc[strInd], "lvl=%d,%s%s,dpth=%d,stncl=%d",
                          v->level, v->double_buffer ? "double" : "single",
                          v->stereo ? ",stereo" : "", v->depth_size,
			  v->stencil_size);
        if (v->acc_red_size || v->acc_green_size || v->acc_blue_size ||
            v->acc_alpha_size)
            strInd += sprintf(&visDesc[strInd], ",accum(%d,%d,%d,%d)",
                              v->acc_red_size, v->acc_green_size,
                              v->acc_blue_size, v->acc_alpha_size);
        if (v->aux_buffers)
            strInd += sprintf(&visDesc[strInd], ",aux=%d", v->aux_buffers);
        ogEnvLog(2, "%s\n", visDesc);
    }
    return 1;
}

void
ogEnvUnMapWindow(int vindex) {
    visfoRecP v = visfo + vindex;

    if (v->window) {
	ShowWindow(v->window, SW_HIDE);
    }
}
/****************************************************************************
* ogEnvPostVisual() -
****************************************************************************/
void
ogEnvPostVisual(int vindex)
{
    visfoRecP v = visfo + vindex;

    glFlush();
    if ((opts.flags & FLAG_HOLD) ||
        (opts.errcount && (opts.flags & FLAG_ERRHOLD))) {
        if ((opts.flags & FLAG_PIXMAP) || opts.doingAuxBuffer) {
            static GLuint *buf;

            if (!buf)
                buf = (GLuint *)
                    malloc(sizeof(GLuint) * windata.xsize * windata.ysize);
            ogLibReadPixels(0, 0, windata.xsize - 1, windata.ysize - 1, buf);
            wglMakeCurrent(v->hDC, v->cx);
            if (opts.doingAuxBuffer) 
                glDrawBuffer(ogEnvCurVisualInfo(GLX_DOUBLEBUFFER) ?
                             GL_BACK : GL_FRONT);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, windata.xsize, 0, windata.ysize, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glRasterPos2i(0, 0);
            glDrawPixels(windata.xsize, windata.ysize,
                         v->rgba ? GL_RGBA : GL_COLOR_INDEX,
                         v->rgba ? GL_UNSIGNED_BYTE : GL_UNSIGNED_INT, buf);
            /* Restore default not to upset state checker */
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            ogLibSetDefaultRasterPos();
        }
        /* make back buffer visible */
        if (ogEnvGLXVisualInfo(vindex, GLX_DOUBLEBUFFER))
	    ogEnvSwapBuffers();
    }
    if (opts.doingAuxBuffer) {
        GLenum defBuf = ogEnvCurVisualInfo(GLX_DOUBLEBUFFER) ?
            GL_BACK : GL_FRONT;
        glReadBuffer(defBuf);
        glDrawBuffer(defBuf);
    }
}

/****************************************************************************
* createWindow() -
****************************************************************************/
static void
createWindow(char *name, int x0, int x1, int y0, int y1, visfoRecP v)
{
    RECT rect;

    v->window =
	CreateWindow(ogtstClassName,
		     name,
		     WS_POPUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
		     x0, y0,
		     x1 - x0 + 1,
		     y1 - y0 + 1,
		     NULL,			/* Parent window's handle */
		     NULL,			/* Menu handle */
		     hGlobalInst,		/* Instance handle */
		     NULL);			/* No additional data */

    GetClientRect(v->window, &rect);
    assert(windata.xsize == rect.right - rect.left);
    assert(windata.ysize == rect.bottom - rect.top);

    SetWindowLong(v->window, 0, (LONG) v);

    v->hDC = GetDC(v->window);
    SetupDC(v, v->hDC);
    SetupPalette(v);

    v->cx  = wglCreateContext(v->hDC);

    /* ogEnvLog(OG_LINTERNALERROR,...) does exit(1) */
    if (v->cx == NULL) {
	ogEnvLog(OG_LINTERNALERROR, "Could not create context\n");
    }
        
    if(!(getenv("OGTST_DONT_MAPWINDOW") ||
         (opts.doingPbuffer && !(opts.flags & FLAG_PB_WINDOWS_MAPPED)))) {
	ShowWindow(v->window, SW_SHOWNORMAL);
	UpdateWindow(v->window);
    }

    if (!wglMakeCurrent(v->hDC, v->cx)) {
	ogEnvLog(OG_LINTERNALERROR, "Can't make window current to context\n");
    }
}

/****************************************************************************
* ogEnvXScreenSize() -
****************************************************************************/
int
ogEnvXScreenSize(void)
{
    return GetSystemMetrics(SM_CXSCREEN);
}

/****************************************************************************
* ogEnvYScreenSize() -
****************************************************************************/
int
ogEnvYScreenSize(void)
{
    return GetSystemMetrics(SM_CYSCREEN);
}

/************************
* ogEnvWindow
******************/
HWND
ogEnvWindow(void)
{
    return visfo[windata.cur_visual].window; 
}

/************************
* ogEnvColormap
******************/
HPALETTE
ogEnvColormap(void)
{
    return visfo[0].hPalette;
}

#ifndef WIN32
/*
 * ogEnvVisual - return XID of visual with index i
 */
Visual *
ogEnvVisual(int i)
{
     return visfo[i].vi->visual;
}

/*
 * ogEnvCurVisualVisual - return XID of current visual
 */
Visual *
ogEnvCurVisualVisual(void) {
     return visfo[windata.cur_visual].vi->visual;
}
#endif

/****************************************************************************
* ogEnvWInit() -
****************************************************************************/

#define NUM_COLORS (sizeof(colors) / sizeof(colors[0]))
struct colorIndexState {
    GLfloat amb[3];	/* ambient color / bottom of ramp */
    GLfloat diff[3];	/* diffuse color / middle of ramp */
    GLfloat spec[3];	/* specular color / top of ramp */
    GLfloat ratio;	/* ratio of diffuse to specular in ramp */
    GLint indexes[3];	/* where ramp was placed in palette */
};
struct colorIndexState colors[] = {
    {
        { 0.0F, 0.0F, 0.0F },
        { 1.0F, 0.0F, 0.0F },
        { 1.0F, 1.0F, 1.0F },
        0.75F, { 0, 0, 0 },
    },
    {
        { 0.0F, 0.05F, 0.05F },
        { 0.9F, 0.0F, 1.0F },
        { 1.0F, 1.0F, 1.0F },
        1.0F, { 0, 0, 0 },
    },
    {
        { 0.0F, 0.0F, 0.0F },
        { 1.0F, 0.9F, 0.1F },
        { 1.0F, 1.0F, 1.0F },
        0.75F, { 0, 0, 0 },
    },
    {
        { 0.0F, 0.0F, 0.0F },
        { 0.1F, 1.0F, 0.9F },
        { 1.0F, 1.0F, 1.0F },
        0.75F, { 0, 0, 0 },
    },
};

static void SetupDC(visfoRec *v, HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd;
    BOOL retVal;

    DescribePixelFormat(hDC, v->pixelFormat, sizeof(pfd), &pfd);
    retVal = SetPixelFormat(hDC, v->pixelFormat, &pfd);
    if (retVal != TRUE) {
	MessageBox(WindowFromDC(hDC), "Failed to set pixel format.",
		   "OpenGL application error", MB_ICONERROR|MB_OK);
	exit(1);
    }
}

/****************************************************************************
* SetupPalette() -
****************************************************************************/
static void SetupPalette(visfoRec *v)
{
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE* pPal;
    int pixelFormat = GetPixelFormat(v->hDC);
    int paletteSize;

    DescribePixelFormat(v->hDC, pixelFormat, sizeof(pfd), &pfd);

    if (!(pfd.dwFlags & PFD_NEED_PALETTE ||
        pfd.iPixelType == PFD_TYPE_COLORINDEX)) {
	return;
    }

    paletteSize = 1 << pfd.cColorBits;
    pPal = (LOGPALETTE *)malloc(sizeof(LOGPALETTE)+
                                paletteSize*sizeof(PALETTEENTRY));
    pPal->palVersion = 0x300;
    pPal->palNumEntries = paletteSize;

    GetSystemPaletteEntries(v->hDC, 0, paletteSize, &pPal->palPalEntry[0]);

    if (pfd.iPixelType == PFD_TYPE_RGBA) {
	int redMask = (1 << pfd.cRedBits) - 1;
	int greenMask = (1 << pfd.cGreenBits) - 1;
	int blueMask = (1 << pfd.cBlueBits) - 1;
	int i;

	for (i = 0; i < paletteSize; ++i) {
	    pPal->palPalEntry[i].peRed = (((i >> pfd.cRedShift) & redMask) *
					 255) / redMask;
	    pPal->palPalEntry[i].peGreen = (((i >> pfd.cGreenShift) &
					   greenMask) * 255) / greenMask;
	    pPal->palPalEntry[i].peBlue = (((i >> pfd.cBlueShift) & blueMask) *
					  255) / blueMask;
	    pPal->palPalEntry[i].peFlags = 0;
	}
    } else {
	int numRamps = NUM_COLORS;
	int rampSize = (paletteSize - 20) / numRamps;
	int extra = (paletteSize - 20) - (numRamps * rampSize);
	int i, r;

	for (r = 0; r < numRamps; ++r) {
	    int rampBase = r * rampSize + 10;
	    PALETTEENTRY *pe = &pPal->palPalEntry[rampBase];
	    int diffSize = (int) (rampSize * colors[r].ratio);
	    int specSize = rampSize - diffSize;

	    for (i = 0; i < rampSize; ++i) {
		GLfloat *c0, *c1;
		GLint a;

		if (i < diffSize) {
		    c0 = colors[r].amb;
		    c1 = colors[r].diff;
		    a = (i * 255) / (diffSize - 1);
		} else {
		    c0 = colors[r].diff;
		    c1 = colors[r].spec;
		    a = ((i - diffSize) * 255) / (specSize - 1);
		}

		pe[i].peRed = (BYTE)(a * (c1[0] - c0[0]) + 255 * c0[0]);
		pe[i].peGreen = (BYTE)(a * (c1[1] - c0[1]) + 255 * c0[1]);
		pe[i].peBlue = (BYTE)(a * (c1[2] - c0[2]) + 255 * c0[2]);
		pe[i].peFlags = PC_NOCOLLAPSE;
	    }

	    colors[r].indexes[0] = rampBase;
	    colors[r].indexes[1] = rampBase + (diffSize-1);
	    colors[r].indexes[2] = rampBase + (rampSize-1);
	}

	for (i = 0; i < extra; ++i) {
	    int index = numRamps * rampSize + 10 + i;
	    PALETTEENTRY *pe = &pPal->palPalEntry[index];

	    pe->peRed = (BYTE)0;
	    pe->peGreen = (BYTE)0;
	    pe->peBlue = (BYTE)0;
	    pe->peFlags = PC_NOCOLLAPSE;
	}
    }

    v->hPalette = CreatePalette(pPal);
    free(pPal);

    if (v->hPalette) {
	SelectPalette(v->hDC, v->hPalette, FALSE);
	RealizePalette(v->hDC);
    }
}

LRESULT APIENTRY WinProc(HWND hWin, UINT message, WPARAM wParam, LPARAM lParam)
{
    visfoRec *vis;

    switch (message) {
	case WM_CREATE:
	    SetWindowText(hWin, "ogtst");
	    return 0;
	case WM_DESTROY:
	    vis = (visfoRec *) GetWindowLong(hWin, 0);
	    if (vis) {
		if (vis->cx) {
		    wglMakeCurrent(NULL, NULL);
		    wglDeleteContext(vis->cx);
		    vis->cx = NULL;
		}
		if (vis->hPalette) {
		    UnrealizeObject(vis->hPalette);
		    DeleteObject(vis->hPalette);
		    vis->hPalette = NULL;
		}
		if (vis->hDC) {
		    ReleaseDC(hWin, vis->hDC);
		    vis->hDC = NULL;
		}
	    }
	    return 0;
	case WM_PALETTECHANGED:
	    vis = (visfoRec *) GetWindowLong(hWin, 0);
	    if (vis && vis->hDC &&
		vis->hPalette != NULL && (HWND)wParam != hWin) {
		UnrealizeObject(vis->hPalette);
		SelectPalette(vis->hDC, vis->hPalette, GL_FALSE);
		RealizePalette(vis->hDC);
		return 0;
	    }
	    break;
	case WM_QUERYNEWPALETTE:
	    vis = (visfoRec *) GetWindowLong(hWin, 0);
	    if (vis && vis->hDC && vis->hPalette != NULL) {
		UnrealizeObject(vis->hPalette);
		SelectPalette(vis->hDC, vis->hPalette, GL_FALSE);
		RealizePalette(vis->hDC);
		return GL_TRUE;
	    }
	    break;
	case WM_PAINT:
	{
	    PAINTSTRUCT ps;
	    BeginPaint(hWin, &ps);
	    EndPaint(hWin, &ps);
	    return 0;
	}
	    break;
	default:
	    break;
    }

    return DefWindowProc(hWin, message, wParam, lParam);
}

void
ogEnvWInit(void)
{
    HWND hWin;
    HDC hDC;
    HGLRC hGLRC;
    char *winName = "ogtst";
    WNDCLASS winClass;
    const char *renderer, *version;

    hGlobalInst = GetModuleHandle(NULL);

    /* Define and register the window class */
    winClass.style = CS_HREDRAW | CS_VREDRAW;
    winClass.lpfnWndProc = WinProc;
    winClass.cbClsExtra = 0;
    winClass.cbWndExtra = 4;
    winClass.hInstance = hGlobalInst;
    winClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    winClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    winClass.hbrBackground = GetStockObject(WHITE_BRUSH);
    winClass.lpszMenuName = NULL;
    winClass.lpszClassName = ogtstClassName;

    if (!RegisterClass(&winClass))
	ogEnvLog(OG_LINTERNALERROR, "Failed to register window class \"%s\"\n",
		 ogtstClassName);

    /* create and bind a context so that we can query the renderer string */
    hWin = CreateWindow(ogtstClassName,
			winName,
			WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
			0, 0, 10, 10,
			NULL,			/* Parent window's handle */
			NULL,			/* Menu handle */
			hGlobalInst,		/* Instance handle */
			NULL);			/* No additional data */

    hDC = GetDC(hWin);

    getVisuals(hDC);

    SetupDC(visfo, hDC);

    hGLRC  = wglCreateContext(hDC);

    if (hGLRC == NULL) {
	ogEnvLog(OG_LINTERNALERROR, "Could not create context\n");
    }
        
    if (!wglMakeCurrent(hDC, hGLRC)) {
	ogEnvLog(OG_LINTERNALERROR, "Can't make window current to context\n");
    }

    renderer = glGetString(GL_RENDERER);
#if 1
    {
	char *env = getenv("OGTST_FORCE_RENDERER");
	if (env) renderer = env;
    }
#endif
    gtst_renderer = (const char *) malloc(strlen(renderer)+1);
    strcpy((char *)gtst_renderer, renderer);

    version = glGetString(GL_VERSION);
    /* version string is of the form "1.x Irix m.n" */
    glversion = atoi(version+2);

    ogEnvInitStateChecker();

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hGLRC);
    ReleaseDC(hWin, hDC);

    DestroyWindow(hWin);
}

/****************************************************************************
* getVisuals() -
****************************************************************************/
static int
getVisConfigs(HDC hDC)
{
    visfoRecP v = visfo;
    int found, ii, nOgl = 0;

    found = DescribePixelFormat(hDC, 0, 0, NULL);

    for (ii = 1; ii <= found; ii++) {
	PIXELFORMATDESCRIPTOR pfd;

	if (DescribePixelFormat(hDC, ii, sizeof(pfd), &pfd)) {
	    if (pfd.dwFlags & PFD_SUPPORT_OPENGL) {

		v->buffer_size = pfd.cColorBits;
		switch (pfd.iLayerType) {
		case PFD_MAIN_PLANE:
		    v->level = 0;
		    break;
		case PFD_OVERLAY_PLANE:
		    v->level = 1;
		    break;
		case PFD_UNDERLAY_PLANE:
		    v->level = -1;
		    break;
		}
		v->rgba = (pfd.iPixelType == PFD_TYPE_RGBA);
		v->double_buffer = (pfd.dwFlags & PFD_DOUBLEBUFFER) != 0;
		v->stereo = (pfd.dwFlags & PFD_STEREO) != 0;
		v->aux_buffers = pfd.cAuxBuffers;
		v->red_size = pfd.cRedBits;
		v->green_size = pfd.cGreenBits;
		v->blue_size = pfd.cBlueBits;
		v->alpha_size = pfd.cAlphaBits;
		v->depth_size = pfd.cDepthBits;
		v->stencil_size = pfd.cStencilBits;
		v->acc_red_size = pfd.cAccumRedBits;
		v->acc_green_size = pfd.cAccumGreenBits;
		v->acc_blue_size = pfd.cAccumBlueBits;
		v->acc_alpha_size = pfd.cAccumAlphaBits;
		v->pixelFormat = ii;
		v->dwFlags = pfd.dwFlags;

		v++;
		nOgl++;
	    }
	}
    }
    return nOgl;
}

static void
getVisuals(HDC hDC)
{
    int total, i, j;
    visfoRecP v, vT;

    windata.num_visual = 0;
    total = getVisConfigs(hDC);
    
    for(i = 0, v = visfo; i < total; i++, v++) {
        /* Skip redundant configurations */
        for (j = i, vT = v; j < total && vT->buffer_size == -1; j++, vT++)
            ;
        if (j < total && j != i) {
            /*
             * config[i] was redundant and there is at least one valid
             * configuration past i.  So copy that config over to i
             */
            memcpy(v, vT, sizeof(visfoRec));
        }
        if (v->buffer_size == -1)
            /* No more non redundant configurations */
            break;
        windata.num_visual++;
        v->window = NULL;
        /* Find all configurations that are redundant wrt to i */
        for (j = i + 1, vT = v + 1; j < total; j++, vT++) {
            if (vT->buffer_size != -1 &&
                !memcmp(vT, v, offsetof(visfoRec, pixelFormat))) {
                vT->buffer_size = -1;
#if 1
                printf("nuking visual id# %X, same as %x\n",
                       vT->pixelFormat, v->pixelFormat);
#endif
            }
        }
    }
//    resolveOverlayInfo(hDC);
}
    
/*
 * Remove all but the the visuals with visual/fbconfig ids in the
 * list.
 */
void
ogEnvSelectVisuals(int *idList, int n, GLboolean visIds)
{
    int i, j, nFound;
    visfoRecP v, vT;
    
    for (nFound = j = 0; j < n; j++)
        for (i = 0, v = visfo; i < windata.num_visual; i++, v++)
            if ((idList[j] == v->pixelFormat) && v->window == NULL) {
                v->window = (HWND) 1;
                nFound++;
            }
    if (!nFound) {
        char str[512];

        for (j = i = 0; j < n; j++)
            i += sprintf(&str[i], "0x%x,", idList[j]);
        str[i - 1] = '\0';
        ogEnvLog(OG_LUSERERROR,
                 "No matching visual for visual list (%s) of -U/-V option\n",
                 str);
    }
    for(i = 0, v = vT = visfo; i < windata.num_visual; i++, v++) {
        /* Skip configurations that were not selected */
        for (; i < windata.num_visual && v->window == NULL; i++, v++)
            ;
        if (i < windata.num_visual) {
            /*
             * At least one selected configuration found.
             * Copy it over.
             */
            memcpy(vT, v, sizeof(visfoRec));
            vT->window = NULL;
            vT++;
        }
    }
    windata.num_visual = nFound;
}

int
ogEnvHasFBConfigs(void) {
    return GL_FALSE;
}

int
ogEnvHasPbuffers(void) {
    return hasPbuffers;
}
/****************************************************************************
* ogEnvLegalVisual() -
****************************************************************************/
int
ogEnvLegalVisual(int vindex, int constraint)
{
    int i;

    if (opts.flags & FLAG_PIXMAP) {
	if (!(visfo[vindex].dwFlags & PFD_DRAW_TO_BITMAP))
	    return FALSE;
    } else {
	if (!(visfo[vindex].dwFlags & PFD_DRAW_TO_WINDOW))
	    return FALSE;
    }

    constraint = constraint | opts.vmask;

    if (/* Doing aux buffer phase and no aux or test is not marked for aux */
        (opts.doingAuxBuffer &&
         !(visfo[vindex].aux_buffers && (constraint & OG_AUXBUF_TOO))) ||
        /* Doing pbuffer phase and no pbuffer */
        (visfo[vindex].rgba &&
          /* RGBA overlay */
          (visfo[vindex].level != 0 && !(constraint & OG_RGB_OVERLAY_OK))))
        return 0;

    constraint &= ~(OG_AUXBUF_TOO | OG_LUMINANCE_OK | OG_RGB_OVERLAY_OK);
    for (i = 0; i < OG_VIS_CONSTRAINS; i++) 
	if (!queryVisual(vindex, constraint & (1<<i)))
	    return 0;

    return(1);
}

/****************************************************************************
* ogEnvVisualString() -
****************************************************************************/
char *
ogEnvVisualString(int vindex)
{
    visfoRecP v = visfo + vindex;
    static char buf[256];
    int nChars;

    nChars = sprintf(buf, "%s#%x", opts.doingPbuffer ? "PB": "  ",
                     v->pixelFormat);
    if (opts.doingAuxBuffer)
        nChars += sprintf(&buf[nChars], "-Aux");
    for (; nChars < 12; nChars++)
        buf[nChars] = ' ';
    buf[nChars] = '\0';
    return buf;
}

char *
ogEnvCurVisualString(void)
{
    return ogEnvVisualString(windata.cur_visual);
}

/****************************************************************************
* ogEnvGLXVisualInfo() -
***************************************************************************/
int
ogEnvGLXVisualInfo(int vindex, int attr)
{
    visfoRecP v = visfo + vindex;

    switch (attr) {
      case GLX_BUFFER_SIZE:
	return v->buffer_size;
      case GLX_LEVEL:
	return v->level;
      case GLX_RGBA:
	return v->rgba;
      case GLX_DOUBLEBUFFER:
	return v->double_buffer;
      case GLX_STEREO:
	return v->stereo;
      case GLX_AUX_BUFFERS:
	return v->aux_buffers;
      case GLX_RED_SIZE:
	return v->red_size;
      case GLX_GREEN_SIZE:
	return v->green_size;
      case GLX_BLUE_SIZE:
	return v->blue_size;
      case GLX_ALPHA_SIZE:
	return v->alpha_size;
      case GLX_DEPTH_SIZE:
	return v->depth_size;
      case GLX_STENCIL_SIZE:
	return v->stencil_size;
      case GLX_ACCUM_RED_SIZE:
	return v->acc_red_size;
      case GLX_ACCUM_GREEN_SIZE:
	return v->acc_green_size;
      case GLX_ACCUM_BLUE_SIZE:
	return v->acc_blue_size;
      case GLX_ACCUM_ALPHA_SIZE:
	return v->acc_alpha_size;
      default:
	ogEnvLog(OG_LINTERNALERROR,"ogEnvGLXVisualInfo(0x%x,0x%x): invalid attribute\n",vindex,attr);
	break;
    }
    return 0;
}

/*
 * Return the value of the current visual corresponding glx attribute
 */
int
ogEnvCurVisualInfo(int attr)
{
    return ogEnvGLXVisualInfo(windata.cur_visual, attr);
}


/*
 * ogEnvIsMultiSampled - Is the current visual multisampling capble
 */
int
ogEnvIsMultiSampled(void)
{
    return 0;
}

int
ogEnvIsDoubleBuffered(void) {
    return visfo[windata.cur_visual].double_buffer;
}

int
ogEnvIsStereo(void) {
    return visfo[windata.cur_visual].stereo;
}

int
ogEnvIsTransparent(void) {
    return visfo[windata.cur_visual].transparent;
}

int
ogEnvIsDualPersonality(void) {
	return 0;
}

/*
 * ogEnvMultiSamplingMode
 *  Turn multisampling on and off and record it in the visual info.
 */
void
ogEnvMultiSamplingState(int mode)
{
}

/*
 * ogEnvSetRgbaMode
 *  Set the current visual current personality: rgba or ci.
 *  This used only by fbconfigs on visuals that have dual personality.
 */
void
ogEnvSetRgbaMode(int mode)
{
}

/*
 * ogEnvToggleRenderMode
 *  Toggle the current visual current personality: rgba <-> ci.
 *  This used only by fbconfigs on visuals that have dual personality.
 * Returns - visual has dual personality and fbconfigs are supported.
 */
GLboolean
ogEnvToggleRenderMode(int vindex)
{
    return GL_FALSE;
}

/****************************************************************************
* ogEnvIsCIMode() -
****************************************************************************/
int
ogEnvIsCIMode(void)
{
    return !visfo[windata.cur_visual].rgba;
}

/****************************************************************************
* ogEnvColorBits() -
****************************************************************************/
void
ogEnvColorBits(int *rBits, int *gBits, int *bBits, int *aBits, int *ciBits)
{
    if ( visfo[windata.cur_visual].rgba) {
        *rBits = visfo[windata.cur_visual].red_size;
        *gBits = visfo[windata.cur_visual].green_size;
        *bBits = visfo[windata.cur_visual].blue_size;
        *aBits = visfo[windata.cur_visual].alpha_size;
        *ciBits = 0;
    } else {
        *rBits = *gBits = *bBits = *aBits = 0;
        *ciBits = visfo[windata.cur_visual].buffer_size;
    }
}

/*
 * queryVisual() -
 *  Does visual with vindex meet the required constraint
 */
static int
queryVisual(int vindex, int constraint)
{
    visfoRecP v = visfo + vindex;

    switch (constraint) {
      case 0:
	return 1;

      case OG_RGBA:
        if (v->alpha_size == 0)
            return 0;
        /* FALL THRU */
      case OG_RGB:
	return v->rgba && v->red_size > 0 && v->green_size > 0 &&
            v->blue_size > 0;

      case OG_RGB_ORL_A:
        if (v->alpha_size == 0)
            return 0;
        /* FALL THRU */
      case OG_RGB_ORL:
	return v->rgba && v->red_size > 0;

      case OG_LUMINANCE_A:
        if (v->alpha_size == 0)
            return 0;
        /* FALL THRU */
      case OG_LUMINANCE:
        return v->rgba && v->red_size > 0 && v->green_size == 0 &&
            v->blue_size == 0;

      case OG_INDEX:
	return !v->rgba;
      case OG_SINGLEBUF:
	return !v->double_buffer;
      case OG_DOUBLEBUF:
	return v->double_buffer;
      case OG_STEREO:
	return v->stereo;
      case OG_AUXBUF:
	return v->aux_buffers > 0;
      case OG_DEPTH:
	return v->depth_size > 0;
      case OG_STENCIL:
	return v->stencil_size > 0;
      case OG_ACCUM:
	return v->rgba && v->acc_red_size > 0 &&
            (v->acc_green_size > 0 || v->green_size == 0) &&
            (v->acc_blue_size > 0 || v->blue_size == 0) &&
            (v->acc_alpha_size > 0 || v->alpha_size == 0);
      case OG_MULTISAMPLE:
	return 0;
      case OG_OVERLAY:
	return v->level > 0;
      default:
	ogEnvLog(OG_LINTERNALERROR,"queryVisual: invalid property 0x%x\n", constraint);
    }
    return 0;
}

void ogEnvSwapBuffers(void)
{
   SwapBuffers(visfo[windata.cur_visual].hDC);
}

void ogEnvSetWindowText(const char *text)
{
    HWND hWin = visfo[windata.cur_visual].window;
    SetWindowText(hWin, text);
    UpdateWindow(hWin);
}

#ifndef WIN32
/****************************************************************************
* ogEnvInitFont() -
****************************************************************************/
void
ogEnvInitFont(char *fn, int *fontbase, int *fontlength)
{
   XFontStruct *fontInfo;
   unsigned int firt, last;

   if (!(fontInfo = XLoadQueryFont(gtst_dpy,fn)))
     return;

   firt = fontInfo->min_char_or_byte2;
   last = fontInfo->max_char_or_byte2;
   *fontlength = last+1;

   if (!(*fontbase = glGenLists(*fontlength)))
     return;

   glXUseXFont(fontInfo->fid, firt, last-firt+1, *fontbase+firt);

   XFreeFont(gtst_dpy,fontInfo);
}
#endif

int
ogEnvVersion(void) {
    return glversion;
}

double rint(double x)
{
    return (double)((int)(x + 0.5));
}

int ffs(unsigned int x)
{
    int i;

    for (i = 1; i <= sizeof(unsigned int); i++) {
        if (x & 0x0001) {
	    return i;
	}
	x >> 1;
    }
    return 0;
}
