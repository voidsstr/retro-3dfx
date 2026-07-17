/*
** Copyright 1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "shell.h"


static Display *display;
static XVisualInfo *visual;
static Window window;
static GLXContext context;
static Colormap colorMap;
static long screen;

static int attribList1[] = {
    GLX_RGBA,
    GLX_RED_SIZE, 1,
    GLX_GREEN_SIZE, 1,
    GLX_BLUE_SIZE, 1,
    None
};

static int attribList2[] = {
    GLX_DOUBLEBUFFER,
    GLX_RGBA,
    GLX_RED_SIZE, 1,
    GLX_GREEN_SIZE, 1,
    GLX_BLUE_SIZE, 1,
    None
};

static enumTestRec enum_GetConfig[] = {
    GLX_USE_GL, "GLX_USE_GL",
    GLX_BUFFER_SIZE, "GLX_BUFFER_SIZE",
    GLX_LEVEL, "GLX_LEVEL",
    GLX_RGBA, "GLX_RGBA",
    GLX_DOUBLEBUFFER, "GLX_DOUBLEBUFFER",
    GLX_STEREO, "GLX_STEREO",
    GLX_AUX_BUFFERS, "GLX_AUX_BUFFERS",
    GLX_RED_SIZE, "GLX_RED_SIZE",
    GLX_GREEN_SIZE, "GLX_GREEN_SIZE",
    GLX_BLUE_SIZE, "GLX_BLUE_SIZE",
    GLX_ALPHA_SIZE, "GLX_ALPHA_SIZE",
    GLX_DEPTH_SIZE, "GLX_DEPTH_SIZE",
    GLX_STENCIL_SIZE, "GLX_STENCIL_SIZE",
    GLX_ACCUM_RED_SIZE, "GLX_ACCUM_RED_SIZE",
    GLX_ACCUM_GREEN_SIZE, "GLX_ACCUM_GREEN_SIZE",
    GLX_ACCUM_BLUE_SIZE, "GLX_ACCUM_BLUE_SIZE",
    GLX_ACCUM_ALPHA_SIZE, "GLX_ACCUM_ALPHA_SIZE",
    -1, "End of List"
};


void CallPixmap(void)
{
    Pixmap pixMap1;
    GLXPixmap pixMap2;

    pixMap1 = XCreatePixmap(display, window, 10, 10, visual->depth);
    Output("glXCreateGLXPixmap\n");
    pixMap2 = glXCreateGLXPixmap(display, visual, pixMap1);
    Output("glXDestroyGLXPixmap\n");
    glXDestroyGLXPixmap(display, pixMap2);
    XFreePixmap(display, pixMap1);
}

void CallGetConfig(void)
{
    int x, i;

    Output("glXGetConfig\n");
    for (i = 0; enum_GetConfig[i].value != -1; i++) {
	Output("\t%s\n", enum_GetConfig[i].name);
	glXGetConfig(display, visual, (int)enum_GetConfig[i].value, &x);
    }
}

void CallIsDirect(void)
{
    int x;

    Output("glXIsDirect\n");
    x = glXIsDirect(display, context);
}

void CallSwapBuffers(void)
{

    Output("glXSwapBuffers\n");
    glXSwapBuffers(display, window);
}

void CallUseXFont(void)
{
    static char pattern[] = "-*-*-*-*-*-*-*-*-*-*-*-*-*-*";
    XFontStruct *info;
    Font id;
    char **list;
    int first, last, count;
    GLuint base;

    list = XListFonts(display, pattern, 1, (int *)&count);
    info = XLoadQueryFont(display, list[0]);

    id = info->fid;
    first = (int)info->min_char_or_byte2;
    last = (int)info->max_char_or_byte2;

    base = glGenLists(last+1);

    Output("glXUseXFont\n");
    glXUseXFont(id, first, last-first+1, base+first);
}

void CallWaitGL(void)
{

    Output("glXWaitGL\n");
    glXWaitGL();
}

void CallWaitX(void)
{

    Output("glXWaitX\n");
    glXWaitX();
}

static Bool WaitForMapNotify(Display *d, XEvent *e, char *arg)
{

    if (e->type == MapNotify && e->xmap.window == window) {
	return GL_TRUE;
    }
    return GL_FALSE;
}

void DoTest(void)
{
    XSetWindowAttributes wa;
    GLXContext tmpContext;
    XSizeHints sh;
    XEvent e;
    unsigned long mask;
    long tmpWindow, x, y, z;
    const char *ptr;

    display = XOpenDisplay(0);

    Output("glXQueryExtension\n");
    x = glXQueryExtension(display, (int *)&y, (int *)&z);

    Output("glXQueryVersion\n");
    x = glXQueryVersion(display, (int *)&y, (int *)&z);

    screen = DefaultScreen(display);

#ifdef GLX_VERSION_1_1

    if (z >= 1) {
	Output("glXQueryExtensionsString\n");
	ptr = glXQueryExtensionsString(display, screen);

	Output("glXGetClientString(GLX_VENDOR)\n");
	ptr = glXGetClientString(display, GLX_VENDOR);
	Output("glXGetClientString(GLX_VERSION)\n");
	ptr = glXGetClientString(display, GLX_VERSION);
	Output("glXGetClientString(GLX_EXTENSIONS)\n");
	ptr = glXGetClientString(display, GLX_EXTENSIONS);

	Output("glXQueryServerString(GLX_VENDOR)\n");
	ptr = glXQueryServerString(display, screen, GLX_VENDOR);
	Output("glXQueryServerString(GLX_VERSION)\n");
	ptr = glXQueryServerString(display, screen, GLX_VERSION);
	Output("glXQueryServerString(GLX_EXTENSIONS)\n");
	ptr = glXQueryServerString(display, screen, GLX_EXTENSIONS);
    }

#ifdef GLX_VERSION_1_2

    if (y > 1 || z >= 1) {
	Output("glXGetCurrentDrawable()\n");;
	glXGetCurrentDrawable();
    }

#endif

#endif

    Output("glXChooseVisual\n");
    visual = glXChooseVisual(display, screen, attribList1);
    if (!visual) {
	visual = glXChooseVisual(display, screen, attribList2);
    }

    Output("glXCreateContext\n");
    context = glXCreateContext(display, visual, None, GL_FALSE);

    Output("glXGetCurrentContext\n");
    tmpContext = glXGetCurrentContext();

    tmpContext = glXCreateContext(display, visual, None, GL_FALSE);

    Output("glXCopyContext\n");
    glXCopyContext(display, context, tmpContext, 0);

    Output("glXDestroyContext\n");
    glXDestroyContext(display, tmpContext);

    colorMap = XCreateColormap(display, RootWindow(display, screen),
			       visual->visual, AllocNone);
    wa.colormap = colorMap;
    wa.background_pixel = 0xFFFFFFFF;
    wa.border_pixel = 0;
    wa.event_mask = StructureNotifyMask | ExposureMask;

    mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;

    window = XCreateWindow(display, RootWindow(display, screen), 0, 0,
			   100, 100, 0, visual->depth, InputOutput,
			   visual->visual, mask, &wa);

    sh.flags = USPosition;
    sh.x = 10;
    sh.y = 10;
    XSetStandardProperties(display, window, "covglx", "covglx", None,
			   0, 0, &sh);
    
    XMapWindow(display, window);
    XIfEvent(display, &e, WaitForMapNotify, 0);

    XSetWMColormapWindows(display, window, &window, 1);

    Output("glXMakeCurrent\n");
    glXMakeCurrent(display, window, context);

    Output("glXMakeCurrent\n");
    tmpWindow = glXGetCurrentDrawable();

    XFlush(display);

    CallGetConfig();
    CallIsDirect();
    CallUseXFont();
    CallPixmap();
    CallSwapBuffers();
    CallWaitGL();
    CallWaitX();

    XDestroyWindow(display, window);
    glXDestroyContext(display, context);
    XFree((char *)visual);

    Output("\n");
}
