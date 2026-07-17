/* Copyright (c) Mark J. Kilgard, 1994.  */

/* This program is freely distributable without licensing fees
   and is provided without guarantee or warrantee expressed or
   implied. This program is -not- in the public domain. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <GL/glut.h>
#include "glutint.h"
#include "wgllib.h"
#include "winlib.h"

GLUTwindow *__glutCurrentWindow = NULL;
GLUTwindow **__glutWindowList = NULL;
int __glutWindowListSize = 0;
GLUTstale *__glutStaleWindowList = NULL;

void (*__glutFreeOverlayFunc) (GLUToverlay *);

static void
cleanWindowWorkList(GLUTwindow * window)
{
  GLUTwindow **pEntry = &__glutWindowWorkList;
  GLUTwindow *entry = __glutWindowWorkList;

  /* Tranverse singly-linked window work list look for the
     window. */
  while (entry) {
    if (entry == window) {
      /* Found it; delete it. */
      *pEntry = entry->prevWorkWin;
      return;
    } else {
      pEntry = &entry->prevWorkWin;
      entry = *pEntry;
    }
  }
}

static void
cleanStaleWindowList(GLUTwindow * window)
{
  GLUTstale **pEntry = &__glutStaleWindowList;
  GLUTstale *entry = __glutStaleWindowList;

  /* Tranverse singly-linked stale window list look for the
     window ID. */
  while (entry) {
    if (entry->window == window) {
      /* Found it; delete it. */
      *pEntry = entry->next;
      free(entry);
      return;
    } else {
      pEntry = &entry->next;
      entry = *pEntry;
    }
  }
}

static GLUTwindow *__glutWindowCache = NULL;

GLUTwindow *
__glutGetWindow(HWND hwnd)
{
  GLUTstale *entry;
  int i;

  /* Does win belong to the last window ID looked up? */
  if (__glutWindowCache && (hwnd == __glutWindowCache->hwnd ||
      (__glutWindowCache->overlay && hwnd ==
        __glutWindowCache->overlay->hwnd))) {
    return
      __glutWindowCache;
  }
  /* Otherwise scan the window list looking for the window ID. */
  for (i = 0; i < __glutWindowListSize; i++) {
    if (__glutWindowList[i]) {
      if (hwnd == __glutWindowList[i]->hwnd) {
        __glutWindowCache = __glutWindowList[i];
        return __glutWindowCache;
      }
      if (__glutWindowList[i]->overlay) {
        if (hwnd == __glutWindowList[i]->overlay->hwnd) {
          __glutWindowCache = __glutWindowList[i];
          return __glutWindowCache;
        }
      }
    }
  }
  /* Scan through destroyed overlay window IDs for which no
     DestroyNotify has yet been received. */
  for (entry = __glutStaleWindowList; entry; entry = entry->next) {
    if (entry->hwnd == hwnd)
      return entry->window;
  }
  return NULL;
}

/* CENTRY */
HDC glutGetWindowHDC(void) {
  if (__glutCurrentWindow) {
    return __glutCurrentWindow->hdc;
  } else {
    return 0;
  }
}

int
glutGetWindow(void)
{
  if (__glutCurrentWindow) {
    return __glutCurrentWindow->num + 1;
  } else {
    return 0;
  }
}
/* ENDCENTRY */

void
__glutSetWindow(GLUTwindow * window)
{
  /* It is tempting to try to short-circuit the call to
     glXMakeCurrent if we "know" we are going to make current
     to a window we are already current to.  In fact, this
     assumption breaks when GLUT is expected to integrated with
     other OpenGL windowing APIs that also make current to
     OpenGL contexts.  Since glXMakeCurrent short-circuits the
     "already bound" case, GLUT avoids the temptation to do so
     too. */

	// I'm gonna short-circuit it as follows:  (not sure if this is faster tho...)
	HGLRC hRC = wglGetCurrentContext();
	if(hRC != window->hglrc)
		wglMakeCurrent(window->hdc, window->hglrc);

	__glutCurrentWindow = window;

  /* We should be careful to force a finish between each
     iteration through the GLUT main loop if indirect OpenGL 
     contexts are in use; indirect contexts tend to have  much
     longer latency because lots of OpenGL extension requests
     can queue up in the X protocol stream.  We accomplish this
     by posting GLUT_FINISH_WORK to be done. */
  //if (!__glutCurrentWindow->isDirect)
  //  __glutPutOnWorkList(__glutCurrentWindow, GLUT_FINISH_WORK);

  /* If debugging is enabled, we'll want to check this window
     for any OpenGL errors every iteration through the GLUT
     main loop.  To accomplish this, we post the
     GLUT_DEBUG_WORK to be done on this window. */
  //if (__glutDebug)
  //  __glutPutOnWorkList(__glutCurrentWindow, GLUT_DEBUG_WORK);
}

/* CENTRY */
void
glutSetWindow(int win)
{
  GLUTwindow *window;

  if (win < 1 || win > __glutWindowListSize) {
    __glutWarning("glutWindowSet attempted on bogus window.");
    return;
  }
  window = __glutWindowList[win - 1];
  if (!window) {
    __glutWarning("glutWindowSet attempted on bogus window.");
    return;
  }
  __glutSetWindow(window);
}
/* ENDCENTRY */

static int
getUnusedWindowSlot(void)
{
  int i;

  /* Look for allocated, unused slot. */
  for (i = 0; i < __glutWindowListSize; i++) {
    if (!__glutWindowList[i]) {
      return i;
    }
  }
  /* Allocate a new slot. */
  __glutWindowListSize++;
  if (__glutWindowList) {
    __glutWindowList = (GLUTwindow **)
      realloc(__glutWindowList,
      __glutWindowListSize * sizeof(GLUTwindow *));
  } else {
    /* XXX Some realloc's do not correctly perform a malloc
       when asked to perform a realloc on a NULL pointer,
       though the ANSI C library spec requires this. */
    __glutWindowList = (GLUTwindow **)
      malloc(sizeof(GLUTwindow *));
  }
  if (!__glutWindowList)
    __glutFatalError("out of memory.");
  __glutWindowList[__glutWindowListSize - 1] = NULL;
  return __glutWindowListSize - 1;
}

void
__glutDefaultDisplay(void)
{
  /* XXX Remove the warning after GLUT 3.0. */
	__glutFatalError("DefaultDisplay callback called...this shouldn't happen\n");

	// following moved to glutMainLoop
	__glutWarning("The following is a new check for GLUT 3.0; update your code.");
  __glutFatalError(
    "display needed for window %d, but no display callback.",
    __glutCurrentWindow->num);
}

void
__glutDefaultReshape(int width, int height)
{
  GLUToverlay *overlay;

  /* Adjust the viewport of the window (and overlay if one
     exists). */
  glViewport(0, 0, (GLsizei) width, (GLsizei) height);
  overlay = __glutCurrentWindow->overlay;
  if (overlay) {
    glViewport(0, 0, (GLsizei) width, (GLsizei) height);
  }
  /* Make sure we are current to the current layer (application 
     should be able to count on the current layer not changing
     unless the application explicitly calls glutUseLayer). */
}

static int FlagIsSet(int value, int flag) {
	int ret = value & flag;
	return ret;
}

static int __glutSelectPFD(HDC hdc)
{
    PIXELFORMATDESCRIPTOR pfd;
	
    wlbInitPFD(&pfd);

	// SET THE DEFAULT STATE
    wlbSetSupportOpenGL(&pfd);
    wlbSetSupportGDI(&pfd);
    wlbSetPFDVersion(&pfd,1);
    wlbSetDrawToWindow(&pfd);
	wlbSetSingleBuffer(&pfd);
	wlbSetRGBA(&pfd);
	wlbSetColorDepth(&pfd, 64);

	// SET ANY NON DEFAULT STATE AS SPECIFIED BY __glutDisplayMode
    if(FlagIsSet(__glutDisplayMode,  GLUT_INDEX))
		wlbSetColorIndex(&pfd);
    if(FlagIsSet(__glutDisplayMode,  GLUT_DOUBLE))
		wlbSetDoubleBuffer(&pfd);
    if(FlagIsSet(__glutDisplayMode,  GLUT_ACCUM))
		wlbSetAccumBits(&pfd, 64);
    if(FlagIsSet(__glutDisplayMode,  GLUT_ALPHA))
		wlbSetAlphaBits(&pfd, 64);
    if(FlagIsSet(__glutDisplayMode,  GLUT_DEPTH))
		wlbSetDepth(&pfd, 64);
    if(FlagIsSet(__glutDisplayMode,  GLUT_STENCIL))
		wlbSetStencilBits(&pfd, 64);
    if(FlagIsSet(__glutDisplayMode,  GLUT_MULTISAMPLE))
		__glutWarning("MULTISAMPLE unsupported in Win32 implementation of GLUT");
    if(FlagIsSet(__glutDisplayMode,  GLUT_STEREO))
		wlbSetStereo(&pfd);
    if(FlagIsSet(__glutDisplayMode,  GLUT_LUMINANCE))
		__glutWarning("LUMINANCE unsupported in Win32 implementation of GLUT");
      
    wlbSetDriverPFD(&pfd);
	// first check for a hardware pfd, if not found, check for software
    int pixelformat;
    if(!(pixelformat = ChoosePixelFormat(hdc, &pfd))) {
		wlbSetDriverPFD(&pfd, 0);
		// if this fails, we don't even have software pfd
		if(!(pixelformat = ChoosePixelFormat(hdc, &pfd)))
			return 0;
    }

    if((pixelformat = ChoosePixelFormat(hdc, &pfd)) == 0) {
        __glutFatalError("ChoosePixelFormat failed");
        return 0;
    }

    if (SetPixelFormat(hdc, pixelformat, &pfd)==FALSE) {
        __glutFatalError("SetPixelFormat failed");
        return 0;
    }
	
	__glutCurrentWindow->hPalette = wlbCreateRGBPalette(hdc);

    return pixelformat;
}


GLUTwindow *
__glutCreateWindow(GLUTwindow * parent,
  int x, int y, int width, int height)
{
    GLUTwindow* window;
  int winnum;
  int i;

  winnum = getUnusedWindowSlot();
  window = (GLUTwindow *) malloc(sizeof(GLUTwindow));
  if (!window)
    __glutFatalError("out of memory.");
  window->num = winnum;
  window->width = width;
  window->height = height;
  window->forceReshape = TRUE;

  window->parent = parent;
  if (parent) {
    window->siblings = parent->children;
    parent->children = window;
  } else {
    window->siblings = NULL;
  }
  window->overlay = NULL;
  window->children = NULL;
  window->display = NULL; //__glutDefaultDisplay;
  window->reshape = __glutDefaultReshape;
  window->mouse = NULL;
  window->motion = NULL;
  window->visibility = NULL;
  window->passive = NULL;
  window->entry = NULL;
  window->special = NULL;
  window->buttonBox = NULL;
  window->dials = NULL;
  window->spaceMotion = NULL;
  window->spaceRotate = NULL;
  window->spaceButton = NULL;
  window->tabletMotion = NULL;
  window->tabletButton = NULL;
//  window->tabletPos[0] = -1;
//  window->tabletPos[1] = -1;
  window->keyboard = NULL;
  window->shownState = 0;
  window->fullscreen = 0;		/* new the window isn't full screen*/
  window->pfdSetup = __glutSelectPFD;
  window->visState = !__glutIconic;
  window->entryState = -1;  /* not EnterNotify or LeaveNotify */
  window->damaged = 0;
  window->workMask = GLUT_MAP_WORK;
  window->desiredMapState = SW_SHOW;
  window->desiredConfMask = 0;
  window->buttonUses = 0;
  window->cursor = NULL; //GLUT_CURSOR_INHERIT;
  window->prevWorkWin = __glutWindowWorkList;
  __glutWindowWorkList = window;
  for (i = 0; i < GLUT_MAX_MENUS; i++) {
    window->menu[i] = 0;
  }
  __glutWindowList[winnum] = window;

  __glutSetWindow(window);
#if 0  // moved to WM_CREATE
#if _TODO_FOR_WIN32
  if (window->fakeSingle) {
    glDrawBuffer(GL_FRONT);
    glReadBuffer(GL_FRONT);
  }
#endif
  glDrawBuffer(GL_BACK);
  glReadBuffer(GL_FRONT);
#endif

  return window;
}

GLUTwindow *
__glutToplevelOf(GLUTwindow * window)
{
  while (window->parent) {
    window = window->parent;
  }
  return window;
}

/* CENTRY */
int
glutCreateWindow(char *title)
{
    GLUTwindow *window;

    window = __glutCreateWindow(NULL, __glutInitX, __glutInitY,
				__glutInitWidth, __glutInitHeight);

    // INITIALIZE THE WINDOWS LIBRARY
    wlInitialize(GetModuleHandle(NULL));
    wlWindowInfo WindowInfo(GetModuleHandle(NULL));

    // SET UP A NEW WINDOW IN WHICH TO DISPLAY OGL 
    WNDCLASS * wc = WindowInfo.GetWC();
    wc->lpfnWndProc = glutWindowProc;
    wc->hbrBackground = GetStockObject(BLACK_BRUSH);
    WindowInfo.SetWCName("GLUTwindow"); 
    WindowInfo.SetTitle(title); 
    WindowInfo.SetCreationData((LPVOID)(window));
    WindowInfo.SetWidth(__glutInitWidth);
    WindowInfo.SetHeight(__glutInitHeight);
    WindowInfo.SetX(__glutInitX);
    WindowInfo.SetY(__glutInitY);
    WindowInfo.SetCmdShow(SW_SHOW); //(int)window->desiredMapState);

    // CREATE AND SHOW THE WINDOW
    HWND hwnd = wlCreateWindow(title, &WindowInfo);
    if (!hwnd) {
		__glutFatalError(wlGetLastError());
		return 0;
    }

    return window->num + 1;
}

int
glutCreateSubWindow(int win, int x, int y, int width, int height)
{
    GLUTwindow *window;
	GLUTwindow *parent = __glutWindowList[win - 1];

    window = __glutCreateWindow(parent, x, y, width, height);

    // INITIALIZE THE WINDOWS LIBRARY
    wlInitialize(GetModuleHandle(NULL));
    wlWindowInfo WindowInfo(GetModuleHandle(NULL));

    // SET UP A NEW WINDOW IN WHICH TO DISPLAY OGL 
    WNDCLASS * wc = WindowInfo.GetWC();
    wc->lpfnWndProc = glutWindowProc;
    wc->hbrBackground = GetStockObject(BLACK_BRUSH);
    WindowInfo.SetWCName("GLUTChildWindow"); 
    WindowInfo.SetTitle("GLUTChildWindow");
    WindowInfo.SetCreationData((LPVOID)(window));
    WindowInfo.SetWidth(width);
    WindowInfo.SetHeight(height);
    WindowInfo.SetX(x);
    WindowInfo.SetY(y);
	WindowInfo.SetStyle(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	WindowInfo.SetParent(parent->hwnd);
    WindowInfo.SetCmdShow(SW_SHOW); //(int)window->desiredMapState);

    // CREATE AND SHOW THE WINDOW
    HWND hwnd = wlCreateWindow(NULL, &WindowInfo);
    if (!hwnd) {
		__glutFatalError(wlGetLastError());
		return 0;
    }

    return window->num + 1;
}
/* ENDCENTRY */

static void
destroyWindow(GLUTwindow * window,
  GLUTwindow * initialWindow)
{
  GLUTwindow **prev, *cur, *parent, *siblings;

  /* Recursively destroy any children. */
  cur = window->children;
  while (cur) {
    siblings = cur->siblings;
    destroyWindow(cur, initialWindow);
    cur = siblings;
  }
  /* Remove from parent's children list (only necessary for
     non-initial windows and subwindows!). */
  parent = window->parent;
  if (parent && parent == initialWindow->parent) {
    prev = &parent->children;
    cur = parent->children;
    while (cur) {
      if (cur == window) {
        *prev = cur->siblings;
        break;
      }
      prev = &(cur->siblings);
      cur = cur->siblings;
    }
  }
  /* Unbind if bound to this window. */
  if (window == __glutCurrentWindow) {
//    glXMakeCurrent(__glutDisplay, None, NULL);
    __glutCurrentWindow = NULL;
  }
  /* Begin tearing down window itself. */
  if (window->overlay) {
    __glutFreeOverlayFunc(window->overlay);
  }

//  glXDestroyContext(__glutDisplay, window->ctx);
  if (window->colormap) {
    /* Only color index windows have colormap data structure. */
//    __glutFreeColormap(window->colormap);
  }
  /* NULLing the __glutWindowList helps detect is a window
     instance has been destroyed, given a window number. */
  __glutWindowList[window->num] = NULL;

  /* Cleanup data structures that might contain window. */
  cleanWindowWorkList(window);
  cleanStaleWindowList(window);
  /* Remove window from the "get window cache" if it is there. */
  if (__glutWindowCache == window)
    __glutWindowCache = NULL;

  free(window);
}

/* CENTRY */
void
glutDestroyWindow(int win)
{
  GLUTwindow *window = __glutWindowList[win - 1];

  if (__glutMappedMenu && __glutMenuWindow == window) {
    __glutFatalUsage("destroying menu window not allowed while menus in use");
  }
#if 0
  /* if not a toplevel window... */
  if (window->parent) {
    /* destroying subwindows may change colormap requirements;
       recalculate toplevel window's WM_COLORMAP_WINDOWS
       property */
    __glutPutOnWorkList(__glutToplevelOf(window->parent),
      GLUT_COLORMAP_WORK);
  }
#endif
  destroyWindow(window, window);
}

void
glutSwapBuffers(void)
{
	SwapBuffers(__glutCurrentWindow->hdc);
}
/* ENDCENTRY */

void
glutDisplayFunc(GLUTdisplayCB displayFunc)
{
  /* XXX Remove the warning after GLUT 3.0. */
  if (!displayFunc)
    __glutFatalError("NULL display callback not allowed in GLUT 3.0; update your code.");
  __glutCurrentWindow->display = displayFunc;
}

void
glutKeyboardFunc(GLUTkeyboardCB keyboardFunc)
{
//  __glutChangeWindowEventMask(KeyPressMask,
//    keyboardFunc != NULL || __glutCurrentWindow->special != NULL);
  __glutCurrentWindow->keyboard = keyboardFunc;
}

void
glutSpecialFunc(GLUTspecialCB specialFunc)
{
//  __glutChangeWindowEventMask(KeyPressMask,
//    specialFunc != NULL || __glutCurrentWindow->keyboard != NULL);
  __glutCurrentWindow->special = specialFunc;
}

void
glutMouseFunc(GLUTmouseCB mouseFunc)
{
  if (__glutCurrentWindow->mouse) {
    if (!mouseFunc) {
      /* previous mouseFunc being disabled */
      __glutCurrentWindow->buttonUses--;
//      __glutChangeWindowEventMask(
//        ButtonPressMask | ButtonReleaseMask,
//        __glutCurrentWindow->buttonUses > 0);
    }
  } else {
    if (mouseFunc) {
      /* previously no mouseFunc, new one being installed */
      __glutCurrentWindow->buttonUses++;
//      __glutChangeWindowEventMask(
//        ButtonPressMask | ButtonReleaseMask, True);
    }
  }
  __glutCurrentWindow->mouse = mouseFunc;
}

void
glutMotionFunc(GLUTmotionCB motionFunc)
{
  /* Hack.  Some window managers (4Dwm by default) will mask
     motion events if the client is not selecting for button
     press and release events. So we select for press and
     release events too (being careful to use reference
     counting).  */
  if (__glutCurrentWindow->motion) {
    if (!motionFunc) {
      /* previous mouseFunc being disabled */
      __glutCurrentWindow->buttonUses--;
//      __glutChangeWindowEventMask(
//        ButtonPressMask | ButtonReleaseMask,
//        __glutCurrentWindow->buttonUses > 0);
    }
  } else {
    if (motionFunc) {
      /* previously no mouseFunc, new one being installed */
      __glutCurrentWindow->buttonUses++;
//      __glutChangeWindowEventMask(
//        ButtonPressMask | ButtonReleaseMask, True);
    }
  }
  /* Real work of selecting for passive mouse motion.  */
//  __glutChangeWindowEventMask(
//    Button1MotionMask | Button2MotionMask | Button3MotionMask,
//    motionFunc != NULL);
  __glutCurrentWindow->motion = motionFunc;
}

void
glutPassiveMotionFunc(GLUTpassiveCB passiveMotionFunc)
{
//  __glutChangeWindowEventMask(PointerMotionMask,
//    passiveMotionFunc != NULL);

  /* Passive motion also requires watching enters and leaves so
     that a fake passive motion event can be generated on an
     enter. */
//  __glutChangeWindowEventMask(EnterWindowMask | LeaveWindowMask,
//    __glutCurrentWindow->entry != NULL || passiveMotionFunc != NULL);

  __glutCurrentWindow->passive = passiveMotionFunc;
}

void
glutEntryFunc(GLUTentryCB entryFunc)
{
//  __glutChangeWindowEventMask(EnterWindowMask | LeaveWindowMask,
//    entryFunc != NULL || __glutCurrentWindow->passive);
  __glutCurrentWindow->entry = entryFunc;
  if (!entryFunc) {
    __glutCurrentWindow->entryState = -1;
  }
}

void
glutVisibilityFunc(GLUTvisibilityCB visibilityFunc)
{
//  __glutChangeWindowEventMask(VisibilityChangeMask,
//    visibilityFunc != NULL);
  __glutCurrentWindow->visibility = visibilityFunc;
  if (!visibilityFunc) {
    /* make state invalid */
    __glutCurrentWindow->visState = -1;
  }
}

void
glutReshapeFunc(GLUTreshapeCB reshapeFunc)
{
  if (reshapeFunc) {
    __glutCurrentWindow->reshape = reshapeFunc;
  } else {
    __glutCurrentWindow->reshape = __glutDefaultReshape;
  }

  RECT r;

  int dx = 0, dy = 0;
#if 0

  dx = GetSystemMetrics(SM_CXEDGE) * 2;
  dy = GetSystemMetrics(SM_CYEDGE);
  dy += GetSystemMetrics(SM_CYCAPTION);
  printf("glutReshapeFunc() - %d %d\n", dx, dy);
#endif

  GetClientRect(__glutCurrentWindow->hwnd, &r);
  PostMessage(__glutCurrentWindow->hwnd, WM_SIZE, 0, 
	      MAKELPARAM(r.right - r.left + dx, r.bottom - r.top + dy));
}
