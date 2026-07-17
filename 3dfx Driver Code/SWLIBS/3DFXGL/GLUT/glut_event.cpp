/* Copyright (c) Mark J. Kilgard, 1994, 1995, 1996. */

/* This program is freely distributable without licensing fees
   and is provided without guarantee or warrantee expressed or
   implied. This program is -not- in the public domain. */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <GL/glut.h>
#include "glutint.h"
#include "winlib.h"
#include "wgllib.h"

#define WM_GLUTIDLE (WM_USER + 0x0002)

static GLUTtimer *freeTimerList = NULL;
static int mappedMenuButton;

GLUTidleCB __glutIdleFunc = NULL;
GLUTtimer *__glutTimerList = NULL;
GLUTwindow *__glutWindowWorkList = NULL;
void (*__glutUpdateInputDeviceMaskFunc) (GLUTwindow *);
unsigned int __glutModifierMask = ~0;
int __glutWindowDamaged = 0;
HPALETTE hPalOld;
extern HCURSOR __glutCurrentCursor;

void
glutIdleFunc(GLUTidleCB idleFunc)
{
    __glutIdleFunc = idleFunc;
}

void
glutTimerFunc(unsigned int interval, GLUTtimerCB timerFunc, int value)
{
        if (!timerFunc) return;
        if (!__glutCurrentWindow)
                __glutFatalError("WIN32 - No window to associate timer with!");

        GLUTtimer* timer = new GLUTtimer;
        if (!timer)
                __glutFatalError("out of memory.");

        timer->value = value;
        timer->next = __glutTimerList;
        timer->func = timerFunc;
        __glutTimerList = timer;

        if (::SetTimer(__glutCurrentWindow->hwnd, value, interval, NULL) == 0)
        {
                // clean up the queue
                __glutTimerList = __glutTimerList->next;
                delete timer;

                LPVOID lpMsgBuf;
                FormatMessage(  FORMAT_MESSAGE_FROM_SYSTEM|
                                FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                NULL, GetLastError(),
                                MAKELANGID(LANG_NEUTRAL,
                                SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf,
                                0, NULL);
                MessageBox(NULL, (char*) lpMsgBuf, "GetLastError",
                                MB_ICONSTOP|MB_OK);

                // Maybe this should not be a fatal error???
                __glutFatalError("WIN32 -- Can't create timer.");
        }
}

static void
handleTimeouts(void)
{
    struct timeval now;
    GLUTtimer *timer;

    if (__glutTimerList) {
	GETTIMEOFDAY(&now);
	while (IS_AT_OR_AFTER(__glutTimerList->timeout, now)) {
	    timer = __glutTimerList;
	    timer->func(timer->value);
	    __glutTimerList = timer->next;
	    timer->next = freeTimerList;
	    freeTimerList = timer;
	    if (!__glutTimerList)
		break;
	}
    }
}

void
__glutPutOnWorkList(GLUTwindow * window, int workMask)
{
    if (window->workMask) {
	/* Already on list; just OR in new workMask. */
	window->workMask |= workMask;
    } else {
	/* Update work mask and add to window work list. */
	window->workMask = workMask;
	window->prevWorkWin = __glutWindowWorkList;
	__glutWindowWorkList = window;
    }
}

void
__glutPostRedisplay(GLUTwindow * window, int layerMask)
{
    window->workMask = GLUT_REDISPLAY_WORK;
//    PostMessage(window->hwnd, WM_PAINT, 0, 0L);
#if 0
    int shown = (layerMask == GLUT_REDISPLAY_WORK) ? window->shownState : window->overlay->shownState;

    /* Post a redisplay if the window is visible (or the
       visibility of the window is unknown, ie. window->visState
       == -1) _and_ the layer is known to be shown. */
    if (window->visState != 0 && shown)
	__glutPutOnWorkList(window, layerMask);
#endif
}

/* CENTRY */
void
glutPostRedisplay(void)
{
    __glutPostRedisplay(__glutCurrentWindow, GLUT_REDISPLAY_WORK);
}

/* ENDCENTRY */

#if TODO_FOR_WIN32
static void
updateWindowVisibility(GLUTwindow * window, int visState)
{
    if (window->shownState && visState != window->visState) {
	if (window->visibility) {
	    window->visState = visState;
	    __glutSetWindow(window);
	    window->visibility(visState ?
			       GLUT_VISIBLE : GLUT_NOT_VISIBLE);
	}
	/* An unmap is only reported on a single window; its
	   descendents need to know they are no longer visible. */
	if (!visState) {
	    GLUTwindow *child = window->children;

	    while (child) {
		updateWindowVisibility(child, visState);
		child = child->siblings;
	    }
	}
    }
}
#endif

static void
purgeStaleWindow(HWND hwnd)
{
    GLUTstale **pEntry = &__glutStaleWindowList;
    GLUTstale *entry = __glutStaleWindowList;

    /* Tranverse singly-linked stale window list look for the
       window ID. */
    while (entry) {
	if (entry->hwnd == hwnd) {
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

static void
processEvents(void)
{
#if 0
    XEvent event, ahead;
    GLUTwindow *window;
    int width, height;
    GLUTeventParser *parser;

    do {
	XNextEvent(__glutDisplay, &event);
	switch (event.type) {
	case MappingNotify:
	    XRefreshKeyboardMapping((XMappingEvent *) & event);
	    break;
	case ConfigureNotify:
	    window = __glutGetWindow(event.xconfigure.window);
	    if (window) {
		if (window->win != event.xconfigure.window) {
		    /* Ignore ConfigureNotify sent to the overlay planes.
		       GLUT could get here because overlays select for
		       StructureNotify events to receive DestroyNotify. */
		    break;
		}
		width = event.xconfigure.width;
		height = event.xconfigure.height;
		if (width != window->width || height != window->height) {
		    if (window->overlay) {
			XResizeWindow(__glutDisplay, window->overlay->win, width, height);
		    }
		    window->width = width;
		    window->height = height;
		    __glutSetWindow(window);
		    /* Do not execute OpenGL out of sequence with respect
		       to the XResizeWindow request! */
		    glXWaitX();
		    window->reshape(width, height);
		    window->forceReshape = False;
		}
	    }
	    break;
	case Expose:
	    /* compress expose events */
	    while (XEventsQueued(__glutDisplay, QueuedAfterReading)
		   > 0) {
		XPeekEvent(__glutDisplay, &ahead);
		if (ahead.type != Expose ||
		    ahead.xexpose.window != event.xexpose.window)
		    break;
		XNextEvent(__glutDisplay, &event);
	    }
	    if (event.xexpose.count == 0) {
		GLUTmenu *menu;

		if (__glutMappedMenu &&
		    (menu = __glutGetMenu(event.xexpose.window))) {
		    __glutPaintMenu(menu);
		} else {
		    window = __glutGetWindow(event.xexpose.window);
		    if (window) {
			if (window->win == event.xexpose.window) {
			    window->damaged = 1;
			    __glutPostRedisplay(window, GLUT_REDISPLAY_WORK);
			} else if (window->overlay && window->overlay->win == event.xexpose.window) {
			    __glutPostRedisplay(window, GLUT_OVERLAY_REDISPLAY_WORK);
			    window->overlay->damaged = 1;
			}
		    }
		}
	    } else {
		/* there are more exposes to read; wait to redisplay */
	    }
	    break;
	case ButtonPress:
	case ButtonRelease:
	    if (__glutMappedMenu && event.type == ButtonRelease
		&& mappedMenuButton == event.xbutton.button) {
		/* Menu is currently popped up and its button is
		   released. */
		__glutFinishMenu(event.xbutton.window, event.xbutton.x, event.xbutton.y);
	    } else {
		window = __glutGetWindow(event.xbutton.window);
		if (window) {
		    GLUTmenu *menu;

		    menu = __glutGetMenuByNum(
			window->menu[event.xbutton.button - 1]);
		    if (menu) {
			if (event.type == ButtonPress && !__glutMappedMenu) {
			    __glutStartMenu(menu, window,
					    event.xbutton.x_root, event.xbutton.y_root,
					    event.xbutton.x, event.xbutton.y);
			    mappedMenuButton = event.xbutton.button;
			} else {
			    /* Ignore a release of a button with a menu
			       attatched to it when no menu is popped up, or
			       ignore a press when another menu is already
			       popped up. */
			}
		    } else if (window->mouse) {
			__glutSetWindow(window);
			__glutModifierMask = event.xbutton.state;
			window->mouse(event.xbutton.button - 1,
				      event.type == ButtonRelease ?
				      GLUT_UP : GLUT_DOWN,
				      event.xbutton.x, event.xbutton.y);
			__glutModifierMask = ~0;
		    } else {
			/* Stray mouse events.  Ignore. */
		    }
		} else {
		    /* Window might have been destroyed and all the 
		       events for the window may not yet be received. */
		}
	    }
	    break;
	case MotionNotify:
	    if (!__glutMappedMenu) {
		window = __glutGetWindow(event.xmotion.window);
		if (window) {
		    /* If motion function registered _and_ buttons held *
		       down, call motion function...  */
		    if (window->motion && event.xmotion.state &
			(Button1Mask | Button2Mask | Button3Mask)) {
			__glutSetWindow(window);
			window->motion(event.xmotion.x, event.xmotion.y);
		    }
		    /* If passive motion function registered _and_
		       buttons not held down, call passive motion
		       function...  */
		    else if (window->passive &&
			     ((event.xmotion.state &
			       (Button1Mask | Button2Mask | Button3Mask)) ==
			      0)) {
			__glutSetWindow(window);
			window->passive(event.xmotion.x,
					event.xmotion.y);
		    }
		}
	    } else {
		/* Motion events are thrown away when a pop up menu is
		   active. */
	    }
	    break;
	case KeyPress:
	    window = __glutGetWindow(event.xkey.window);
	    if (!window) {
		break;
	    }
	    if (window->keyboard) {
		char tmp[1];
		int rc;

		rc = XLookupString(&event.xkey, tmp, sizeof(tmp),
				   NULL, NULL);
		if (rc) {
		    __glutSetWindow(window);
		    __glutModifierMask = event.xkey.state;
		    window->keyboard(tmp[0],
				     event.xkey.x, event.xkey.y);
		    __glutModifierMask = ~0;
		    break;
		}
	    }
	    if (window->special) {
		KeySym ks;
		int key;

		ks = XLookupKeysym((XKeyEvent *) & event, 0);
		/* XXX Verbose, but makes no assumptions about keysym
		   layout. */
		switch (ks) {
/* *INDENT-OFF* */
		    /* function keys */
		case XK_F1:    key = GLUT_KEY_F1; break;
		case XK_F2:    key = GLUT_KEY_F2; break;
		case XK_F3:    key = GLUT_KEY_F3; break;
		case XK_F4:    key = GLUT_KEY_F4; break;
		case XK_F5:    key = GLUT_KEY_F5; break;
		case XK_F6:    key = GLUT_KEY_F6; break;
		case XK_F7:    key = GLUT_KEY_F7; break;
		case XK_F8:    key = GLUT_KEY_F8; break;
		case XK_F9:    key = GLUT_KEY_F9; break;
		case XK_F10:   key = GLUT_KEY_F10; break;
		case XK_F11:   key = GLUT_KEY_F11; break;
		case XK_F12:   key = GLUT_KEY_F12; break;
		    /* directional keys */
		case XK_Left:  key = GLUT_KEY_LEFT; break;
		case XK_Up:    key = GLUT_KEY_UP; break;
		case XK_Right: key = GLUT_KEY_RIGHT; break;
		case XK_Down:  key = GLUT_KEY_DOWN; break;
/* *INDENT-ON* */

		case XK_Prior:
		    /* XK_Prior same as X11R6's XK_Page_Up */
		    key = GLUT_KEY_PAGE_UP;
		    break;
		case XK_Next:
		    /* XK_Next same as X11R6's XK_Page_Down */
		    key = GLUT_KEY_PAGE_DOWN;
		    break;
		case XK_Home:
		    key = GLUT_KEY_HOME;
		    break;
		case XK_End:
		    key = GLUT_KEY_END;
		    break;
		case XK_Insert:
		    key = GLUT_KEY_INSERT;
		    break;
		default:
		    goto skip;
		}
		__glutSetWindow(window);
		__glutModifierMask = event.xkey.state;
		window->special(key, event.xkey.x, event.xkey.y);
		__glutModifierMask = ~0;
	    skip:;
	    }
	    break;
	case EnterNotify:
	case LeaveNotify:
	    if (event.xcrossing.mode != NotifyNormal ||
		event.xcrossing.detail == NotifyNonlinearVirtual ||
		event.xcrossing.detail == NotifyVirtual) {

		/* Careful to ignore Enter/LeaveNotify events that come
		   from the pop-up menu pointer grab and ungrab.  Also,
		   ignore "virtual" Enter/LeaveNotify events since they
		   represent the pointer passing through the window
		   hierarchy without actually entering or leaving the
		   actual real estate of a window.  */

		break;
	    }
	    if (__glutMappedMenu) {
		GLUTmenuItem *item;
		int num;

		item = __glutGetMenuItem(__glutMappedMenu,
					 event.xcrossing.window, &num);
		if (item) {
		    __glutMenuItemEnterOrLeave(item, num, event.type);
		    break;
		}
	    }
	    window = __glutGetWindow(event.xcrossing.window);
	    if (window) {
		if (window->entry) {
		    if (event.type == EnterNotify) {

			/* With overlays established, X can report two
			   enter events for both the overlay and normal
			   plane window. Do not generate a second enter
			   callback if we reported one without an
			   intervening leave. */

			if (window->entryState != EnterNotify) {
			    int num = window->num;
			    Window xid = window->win;

			    window->entryState = EnterNotify;
			    __glutSetWindow(window);
			    window->entry(GLUT_ENTERED);

			    if (__glutMappedMenu) {

				/* Do not generate any passive motion events
				   when menus are in use. */

			    } else {

				/* An EnterNotify event can result in a
				   "compound" callback if a passive motion
				   callback is also registered. In this case,
				   be a little paranoid about the possibility
				   the window could have been destroyed in the
				   entry callback. */

				window = __glutWindowList[num];
				if (window && window->passive && window->win == xid) {
				    __glutSetWindow(window);
				    window->passive(event.xcrossing.x, event.xcrossing.y);
				}
			    }
			}
		    } else {
			if (window->entryState != LeaveNotify) {

			    /* When an overlay is established for a window
			       already mapped and with the pointer in it, the
			       X server will generate a leave/enter event pair
			       as the pointer leaves (without moving) from the
			       normal plane X window to the newly mapped
			       overlay  X window (or vice versa). This
			       enter/leave pair should not be reported to the
			       GLUT program since the pair is a consequence of
			       creating (or destroying) the overlay, not an
			       actual leave from the GLUT window. */

			    if (XEventsQueued(__glutDisplay, QueuedAfterReading)) {
				XPeekEvent(__glutDisplay, &ahead);
				if (ahead.type == EnterNotify &&
				    __glutGetWindow(ahead.xcrossing.window) == window) {
				    XNextEvent(__glutDisplay, &event);
				    break;
				}
			    }
			    window->entryState = LeaveNotify;
			    __glutSetWindow(window);
			    window->entry(GLUT_LEFT);
			}
		    }
		} else if (window->passive) {
		    __glutSetWindow(window);
		    window->passive(event.xcrossing.x, event.xcrossing.y);
		}
	    }
	    break;
	case UnmapNotify:
	    /* MapNotify events are not needed to maintain visibility
	       state since VisibilityNotify events will be delivered
	       when a window becomes visible from mapping.  However,
	       VisibilityNotify events are not delivered when a window
	       is unmapped (for the window or its children). */
	    window = __glutGetWindow(event.xunmap.window);
	    if (window) {
		if (window->win != event.xconfigure.window) {
		    /* Ignore UnmapNotify sent to the overlay planes.
		       GLUT could get here because overlays select for
		       StructureNotify events to receive DestroyNotify. */
		    break;
		}
		updateWindowVisibility(window, 0);
	    }
	    break;
	case VisibilityNotify:
	    window = __glutGetWindow(event.xvisibility.window);
	    if (window) {
		int visState = (event.xvisibility.state != VisibilityFullyObscured);

		if (visState != window->visState) {
		    if (window->visibility) {
			window->visState = visState;
			__glutSetWindow(window);
			window->visibility(visState ? GLUT_VISIBLE : GLUT_NOT_VISIBLE);
		    }
		}
	    }
	    break;
	case ClientMessage:
	    if (event.xclient.data.l[0] == __glutWMDeleteWindow)
		exit(0);
	    break;
	case DestroyNotify:
	    purgeStaleWindow(event.xdestroywindow.window);
	    break;
	case CirculateNotify:
	case CreateNotify:
	case GravityNotify:
	case ReparentNotify:
	    /* Uninteresting to GLUT (but possible for GLUT to
	       receive). */
	    break;
	default:
	    /* Pass events not directly handled by the GLUT main
	       event loop to any event parsers that have been
	       registered.  In this way, X Input extension events are
	       passed to the correct handler without forcing all GLUT
	       programs to support X Input event handling. */
	    parser = eventParserList;
	    while (parser) {
		if (parser->func(&event))
		    break;
		parser = parser->next;
	    }
	    break;
	}
    }
    while (XPending(__glutDisplay));
#endif
}

LONG WINAPI glutWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    static RECT	rect;
    static POINT point;
    static int x, y;
    LONG    lRet = 1;
    GLUTwindow* info;
    static HWND saveHwnd;
    static BOOL capture;

    // RETRIEVE CORROSPONDING INFO OF THIS WINDOW
    info = (GLUTwindow*)wlGetWindowParam(hWnd);

    switch (uMsg) {
    case WM_CREATE:
    {
	// RETRIEVE THE POINTER TO THE CORROSPONDING INFO 
	GLUTwindow * info;
	info = (GLUTwindow*)wlGetCreationData(lParam);
		
	// SAVE THE INFO POINTER AS PARAMETER OF THIS WINDOW
	wlAddWindowParam(hWnd, (DWORD)info);
		
	// SET UP THE PIXEL FORMAT OF THIS WINDOW'S DC
	HDC hDC = GetDC(hWnd);
	__glutCurrentWindow = info;
	info->pfdIndex = (*info->pfdSetup)(hDC);
			
	HGLRC hRC = wglCreateContext(hDC);

	// CREATE THE WINDOW CONTEXT
	BOOL ret = wglMakeCurrent(hDC, hRC);

	// Add this info to the GLUTwindow struct
	info->hdc = hDC;
	info->hglrc = hRC;
	info->renderHwnd = hWnd;
	info->hwnd = hWnd;
		
	// set the cursor
	glutSetCursor(GLUT_CURSOR_LEFT_ARROW);

	if(wlbIsDoubleBuffered(info->hdc, info->pfdIndex))
	{
	    glDrawBuffer(GL_BACK);
	    glReadBuffer(GL_FRONT);
	} else {
	    glDrawBuffer(GL_FRONT);
	    glReadBuffer(GL_FRONT);
	}
	return 0;
    }
    break;

    case WM_SYSCOMMAND:  // visibility  TODO - if window is fully obscured by another window...
    {
	switch(wParam) {
	case SC_MAXIMIZE:
	case SC_RESTORE:
	    __glutSetWindow(info);
	    if(info->visibility)
		info->visibility(GL_TRUE);
	    break;
	case SC_MINIMIZE:
	    __glutSetWindow(info);
	    if(info->visibility)
		info->visibility(GL_FALSE);
	    break;
	}

	lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
    }
    break;

    case WM_SHOWWINDOW:
    {
	// initial visibility
	__glutSetWindow(info);
	info->visState = (BOOL)wParam;
	if(info->visibility)
	    info->visibility((BOOL)wParam);

	lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
    }
    break;

    case WM_PAINT:
    { 
	if(info) {
	    PAINTSTRUCT	ps;
	    HDC	hDC = BeginPaint(hWnd, &ps);
	    __glutSetWindow(info);
	    if(info->display)
		(*info->display)();
	    EndPaint(hWnd, &ps);
	    return 0;
	}
    }
    break;

    case WM_NCCALCSIZE:  // intercept this so that we can draw a big window
    {
	if(info && info->fullscreen)
	    return 0;
	else
	    lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
    }
    break;

    case WM_GLUTIDLE:
    {
	if(__glutIdleFunc) {
	    __glutSetWindow(info);
	    __glutIdleFunc();
	}
	return 0;
    }
    break;

    case WM_SIZE:
    {
	// probably better to do visibility here...See WM_SIZE in help
		
	static RECT	oldrect;
	GetClientRect(hWnd, &rect);
	if(info && info->reshape) {
	    __glutSetWindow(info);
	    info->width = LOWORD(lParam);
	    info->height = HIWORD(lParam);
	    (*(info->reshape))(LOWORD(lParam), HIWORD(lParam));
	}
	if((oldrect.right > rect.right) || (oldrect.bottom > rect.bottom))
	    PostMessage (hWnd, WM_PAINT, 0, 0L);
	oldrect.right = rect.right;
	oldrect.bottom = rect.bottom;
	return 0;
    }
    break;

    // The WM_QUERYNEWPALETTE message informs a window that it is about to
    // receive input focus. In response, the window receiving focus should
    // realize its palette as a foreground palette and update its client
    // area. If the window realizes its palette, it should return TRUE;
    // otherwise, it should return FALSE.
    case WM_QUERYNEWPALETTE:
    {
	HDC     hDC;
		
	if(info->hPalette)
	{
	    hDC = GetDC(hWnd);
			
	    // Select and realize the palette
			
	    hPalOld = SelectPalette(hDC, info->hPalette, FALSE);
	    RealizePalette(hDC);
			
	    // Redraw the client area
			
	    InvalidateRect(hWnd, NULL, TRUE);
	    UpdateWindow(hWnd);
			
	    if(hPalOld)
		SelectPalette(hDC, hPalOld, FALSE);

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
	HPALETTE	hPalOld;

	// Before processing this message, make sure we
	// are indeed using a palette
	
	if (info->hPalette)
	{	
	    // If this application did not change the palette, select
	    // and realize this application's palette

	    if (wParam != (WPARAM)hWnd)
	    {
		// Need the window's DC for SelectPalette/RealizePalette

		hDC = GetDC(hWnd);
				
		// Select and realize our palette
				
		hPalOld = SelectPalette(hDC, info->hPalette, FALSE);
		RealizePalette(hDC);
				
		// WHen updating the colors for an inactive window,
		// UpdateColors can be called because it is faster than
		// redrawing the client area (even though the results are
		// not as good)
				
		UpdateColors(hDC);
				
		// Clean up
				
		if (hPalOld)
		    SelectPalette(hDC, hPalOld, FALSE);
				
		ReleaseDC(hWnd, hDC);
	    }
	}
	break;
    }

    case WM_KEYDOWN:
    {
	// get any modifiers
	__glutModifierMask = 0;
	if(GetKeyState(VK_CONTROL) & 0x8000)
	    __glutModifierMask |= ControlMask;
	if(GetKeyState(VK_SHIFT) & 0x8000)
	    __glutModifierMask |= ShiftMask;
	if(GetKeyState(VK_MENU) & 0x8000)
	    __glutModifierMask |= AltMask;

	// get the key
	int key;
	switch (wParam) {
	case VK_F1:     key = GLUT_KEY_F1;  break;
	case VK_F2:     key = GLUT_KEY_F2;  break;
	case VK_F3:     key = GLUT_KEY_F3;  break;
	case VK_F4:     key = GLUT_KEY_F4;  break;
	case VK_F5:     key = GLUT_KEY_F5;  break;
	case VK_F6:     key = GLUT_KEY_F6;  break;
	case VK_F7:     key = GLUT_KEY_F7;  break;
	case VK_F8:     key = GLUT_KEY_F8;  break;
	case VK_F9:     key = GLUT_KEY_F9;  break;
	case VK_F10:    key = GLUT_KEY_F10; break;
	case VK_F11:    key = GLUT_KEY_F11; break;
	case VK_F12:    key = GLUT_KEY_F12; break;
	case VK_LEFT:   key = GLUT_KEY_LEFT;  break;
	case VK_UP:     key = GLUT_KEY_UP;    break;
	case VK_RIGHT:  key = GLUT_KEY_RIGHT; break;
	case VK_DOWN:   key = GLUT_KEY_DOWN;  break;
	case VK_PRIOR:  key = GLUT_KEY_PAGE_UP;   break;
	case VK_NEXT:   key = GLUT_KEY_PAGE_DOWN; break;
	case VK_HOME:   key = GLUT_KEY_HOME;      break;
	case VK_END:    key = GLUT_KEY_END;       break;
	case VK_INSERT: key = GLUT_KEY_INSERT;   break;
	case VK_CONTROL: __glutModifierMask |= ControlMask; break;
	case VK_SHIFT: __glutModifierMask |= ShiftMask; break;
//	case VK_SYSMENU: __glutModifierMask |= AltMask; break;
	default: key = FALSE; break;
	}

	// handle the keypress if user defined a callback (& a valid special key)
        if(key && info->special) {
	    GetWindowRect(hWnd, &rect);
	    GetCursorPos(&point);
	    __glutSetWindow(info);
	    (*info->special)(key, point.x - rect.left, point.y - rect.top);
        }

	__glutModifierMask = ~0;

    }
    break;

    case WM_CHAR:
    {
	// get any modifiers
	__glutModifierMask = 0;
	if(GetKeyState(VK_CONTROL) & 0x8000)
	    __glutModifierMask |= ControlMask;
	if(GetKeyState(VK_SHIFT) & 0x8000)
	    __glutModifierMask |= ShiftMask;
	if(GetKeyState(VK_MENU) & 0x8000)
	    __glutModifierMask |= AltMask;

	// get the right window!
	if(info->children) {
	    GetCursorPos(&point);
	    ScreenToClient(hWnd, &point);
	    HWND child = ChildWindowFromPoint(hWnd, point);
	    if(child && child != hWnd)
		SendMessage(child, WM_CHAR, wParam, lParam);
	    else {
		// if the user defined a keyboard callback, call it!
		if (info->keyboard) {
		    GetCursorPos(&point);
		    ScreenToClient(hWnd, &point);
		    __glutSetWindow(info);
		    (*(info->keyboard))((int)wParam, (int)point.x, (int)point.y);
		}
	    }
	    return 0;
	} else {
	    // if the user defined a keyboard callback, call it!
	    if (info->keyboard) {
		GetCursorPos(&point);
		ScreenToClient(hWnd, &point);
		__glutSetWindow(info);
		(*(info->keyboard))((int)wParam, (int)point.x, (int)point.y);
	    }
	}

	__glutModifierMask = ~0;
    }
    break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
	SetCapture(hWnd);
		
	// find exactly which button was pressed
	int button;
	switch(uMsg) {
	case WM_LBUTTONDOWN:
#if 0
	    if(wParam & MK_RBUTTON)  // emulate a middle mouse button
		button = GLUT_MIDDLE_BUTTON;
	    else
#endif
		button = GLUT_LEFT_BUTTON;
	    break;
	case WM_MBUTTONDOWN:
	    button = GLUT_MIDDLE_BUTTON;
	    break;
	case WM_RBUTTONDOWN:
#if 0
	    if(wParam & MK_LBUTTON)  // emulate a middle mouse button
		button = GLUT_MIDDLE_BUTTON;
	    else
#endif
		button = GLUT_RIGHT_BUTTON;
	    break;
	}

	// position of button press (window)
	x = LOWORD(lParam);
	y = HIWORD(lParam);

	// handle inconsistencies in glut/win32
	// roll the GLUT/WIN32 0..2^16 pointer co-ord range to 0 +/- 2^15
	if(x & 1 << 15) x -= (1 << 16);
	if(y & 1 << 15) y -= (1 << 16);

	// get any modifiers
	__glutModifierMask = 0;
	if(wParam & MK_CONTROL)
	    __glutModifierMask |= ControlMask;
	if(wParam & MK_SHIFT)
	    __glutModifierMask |= ShiftMask;
	if(GetKeyState(VK_MENU) & 0x8000)  // high order bit set if pressed!
	    __glutModifierMask |= AltMask;

	GLUTmenu* menu = __glutGetMenuByNum(info->menu[button]);
	if(menu && !__glutMappedMenu) {
	    point.x = x; point.y = y;
	    ClientToScreen(hWnd, &point);
	    __glutStartMenu(menu, info, point.x, point.y, x, y);
	}
	else if(info->mouse) { // call the mouse button callback
	    if(__glutMappedMenu) {
		// must've pressed mouse button outside of the menu
		__glutItemSelected = NULL;
		GetWindowRect(hWnd, &rect);
		__glutFinishMenu(0, x - rect.left, y - rect.top);
	    } else 
		(*info->mouse)(button, GLUT_DOWN, x, y);
	}

	// reset the modifier key mask
	__glutModifierMask = ~0;

	return 1;
    }
    break;
		
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
	ReleaseCapture();

	// find exactly which button was pressed
	int button;
	switch(uMsg) {
	case WM_LBUTTONUP:
#if 0
	    if(wParam & MK_RBUTTON)  // emulate a middle mouse button
		button = GLUT_MIDDLE_BUTTON;
	    else
#endif
		button = GLUT_LEFT_BUTTON;
	    break;
	case WM_MBUTTONUP:
	    button = GLUT_MIDDLE_BUTTON;
	    break;
	case WM_RBUTTONUP:
#if 0
	    if(wParam & MK_LBUTTON)  // emulate a middle mouse button
		button = GLUT_MIDDLE_BUTTON;
	    else	
#endif
		button = GLUT_RIGHT_BUTTON;
	    break;
	}
		          
	// position of button press (window)
	x = LOWORD(lParam);
	y = HIWORD(lParam);

	// handle inconsistencies in glut/win32
	// roll the GLUT/WIN32 0..2^16 pointer co-ord range to 0 +/- 2^15
	if(x & 1 << 15) x -= (1 << 16);
	if(y & 1 << 15) y -= (1 << 16);

	// get any modifiers
	__glutModifierMask = 0;
	if(wParam & MK_CONTROL)
	    __glutModifierMask |= ControlMask;
	if(wParam & MK_SHIFT)
	    __glutModifierMask |= ShiftMask;
	if(GetKeyState(VK_MENU) & 0x8000)  // high order bit set if pressed!
	    __glutModifierMask |= AltMask;

	if(!__glutMappedMenu) {
	    __glutSetWindow(info);
	    capture = FALSE;
	    if(info->mouse) (*info->mouse)(button, GLUT_UP, x, y);
	} else {
	    // must've pressed mouse button outside of the menu
	    __glutItemSelected = NULL;
	    GetWindowRect(hWnd, &rect);
	    __glutFinishMenu(0, x - rect.left, y - rect.top);
	}

	// reset the modifier key mask
	__glutModifierMask = ~0;

	return 1;
    }
    break;

    case WM_MOUSEMOVE:
    {
	// set the cursor here.  I don't know if this is 
	// really the best way to do this, but Win32 seems kinda
	// dumb sometimes, so I do it this way.
	SetCursor(__glutCurrentCursor);

	x = LOWORD(lParam);
	y = HIWORD(lParam);

	// handle inconsistencies in glut/win32
	// roll the GLUT/WIN32 0..2^16 pointer co-ord range to 0 +/- 2^15
	if(x & 1 << 15) x -= (1 << 16);
	if(y & 1 << 15) y -= (1 << 16);

	if(wParam & MK_LBUTTON || wParam & MK_MBUTTON || wParam & MK_RBUTTON) {
	    if(info->motion) {
		__glutSetWindow(info);
		(*info->motion)(x, y);
	    }
	}
	else if(info->passive) {
	    __glutSetWindow(info);
	    (*info->passive)(x, y);
	}

	return 0;
    }
    break;

    case WM_SETFOCUS:
    {
	if(info->entry)
	    (*info->entry)(GLUT_ENTERED);
	return 0;
    }

    case WM_KILLFOCUS:
    {
	if(info->entry)
	    (*info->entry)(GLUT_LEFT);

	// kill the cursor since we're out of the window
	SetCursor(NULL);
	return 0;
    }

#if 0
	x = LOWORD(lParam);
	y = HIWORD(lParam);

	// handle inconsistencies in glut/win32
	// roll the GLUT/WIN32 0..2^16 pointer co-ord range to 0 +/- 2^15
	if(x & 1 << 15) x -= (1 << 16);
	if(y & 1 << 15) y -= (1 << 16);

	if(info->entry) {
	    // entry function
	    __glutSetWindow(info);
	    if(x < 0 || y < 0)
		(*info->entry)(GLUT_LEFT);
	    else {
		GetClientRect(info->hwnd, &rect);
		if(x > rect.right || y > rect.bottom)
		    (*info->entry)(GLUT_LEFT);
		else
		    (*info->entry)(GLUT_ENTERED);
	    }
	}
	return 0;
    }
#endif

    // HERE'S WHERE WE GET THE VALUE OF THE MENU ITEM SELECTED
    case WM_COMMAND: 
    {   // TODO_FOR_WIN32 -- known bug in menu code -- if the user presses ESC to dismiss 
	// the menu, it takes two clicks to get the menu back...
	int value = LOWORD(wParam);
		
	// get position of cursor (relative to window, not screen)
	GetWindowRect(hWnd, &rect);
	ScreenToClient(hWnd, &point);
		
	__glutItemSelected = __glutGetMenuItem(__glutMappedMenu, NULL, &value);
	__glutFinishMenu(0, point.x, point.y);
    }
    break;
	
    case WM_INITMENUPOPUP:
    {
	__glutMappedMenu = __glutGetMenu((HMENU)wParam);
	return 0;
    }
    break;
    
    case WM_TIMER:
    {
	GLUTtimer *timer = __glutTimerList;
	GLUTtimer *theTimer = NULL;
	GLUTtimer *prev  = NULL;
	
	while(timer) {
	    if(timer->value == (int)wParam) {
		KillTimer(hWnd, timer->value);
		theTimer = timer;
		if(prev)
		    prev->next = timer->next;
		else
		    __glutTimerList = timer->next;
		break;
	    }
	    prev = timer;
	    timer = timer->next;
	}
	
	/* If we found a timer for this wParam, call the callback */
	if (theTimer) {
	    timer->func(timer->value);
	    delete timer;
	}
    }
    break;
    
    case WM_CLOSE:
    {
	/* release and free the device context and rendering context */
	HGLRC hRC = wglGetCurrentContext();
	HDC	  hDC = wglGetCurrentDC();
		
	wglMakeCurrent(NULL, NULL);

	if(hRC) wglDeleteContext(hRC);
	if(hDC)	ReleaseDC(hWnd, hDC);

	/* call destroy window to cleanup and go away */
	DestroyWindow (hWnd);
    }
    break;

    case WM_DESTROY:
    {
	/* release and free the device context and rendering context */
	HGLRC hRC = wglGetCurrentContext();
	HDC	  hDC = wglGetCurrentDC();
		
	wglMakeCurrent(NULL, NULL);

	if(hRC) wglDeleteContext(hRC);
	if(hDC)	ReleaseDC(hWnd, hDC);
	
	PostQuitMessage (0);
    }
    break;

    default:
    {
	/* pass all unhandled messages to DefWindowProc */
	lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
    }
    break;

    }

    /* return 1 if handled message, 0 if not */
    return lRet;
}

static void
idleWait(void)
{
// shouldn't need this...with windows SetTimer stuff
//  if (__glutTimerList)
//    handleTimeouts();

    /* Make sure idle func still exists! */
    if (__glutIdleFunc)
	__glutIdleFunc();
}

static GLUTwindow **beforeEnd;

static GLUTwindow *
processWindowWorkList(GLUTwindow * window)
{
#if 0
    int workMask;

    if (window->prevWorkWin)
	window->prevWorkWin = processWindowWorkList(window->prevWorkWin);
    else
	beforeEnd = &window->prevWorkWin;

    /* Capture work mask for work that needs to be done to this
       window, then clear the window's work mask (excepting the
       dummy work bit, see below).  Then, process the captured
       work mask.  This allows callbacks in the processing the
       captured work mask to set the window's work mask for
       subsequent processing. */

    workMask = window->workMask;
    assert((workMask & GLUT_DUMMY_WORK) == 0);

    /* Set the dummy work bit, clearing all other bits, to
       indicate that the window is currently on the window work
       list _and_ that the window's work mask is currently being
       processed.  This convinces __glutPutOnWorkList that this
       window is on the work list still. */
    window->workMask = GLUT_DUMMY_WORK;

    /* Optimization: most of the time, the work to do is a
       redisplay and not these other types of work.  Check for
       the following cases as a group to before checking each one 
       individually one by one. This saves about 25 MIPS
       instructions in the common redisplay only case. */
    if (workMask & (GLUT_EVENT_MASK_WORK | GLUT_DEVICE_MASK_WORK |
		    GLUT_CONFIGURE_WORK | GLUT_COLORMAP_WORK | GLUT_MAP_WORK)) {
	/* Be sure to set event mask *BEFORE* map window is done. */
	if (workMask & GLUT_EVENT_MASK_WORK) {
	    long eventMask;

	    /* Make sure children are not propogating events this
	       window is selecting for.  Be sure to do this before
	       enabling events on the children's parent. */
	    if (window->children) {
		GLUTwindow *child = window->children;
		unsigned long attribMask = CWDontPropagate;
		XSetWindowAttributes wa;

		wa.do_not_propagate_mask = window->eventMask & GLUT_DONT_PROPAGATE_FILTER_MASK;
		if (window->eventMask & GLUT_HACK_STOP_PROPAGATE_MASK) {
		    wa.event_mask = child->eventMask | (window->eventMask & GLUT_HACK_STOP_PROPAGATE_MASK);
		    attribMask |= CWEventMask;
		}
		do {
		    XChangeWindowAttributes(__glutDisplay, child->win,
					    attribMask, &wa);
		    child = child->siblings;
		} while (child);
	    }
	    eventMask = window->eventMask;
	    if (window->parent && window->parent->eventMask & GLUT_HACK_STOP_PROPAGATE_MASK)
		eventMask |= (window->parent->eventMask & GLUT_HACK_STOP_PROPAGATE_MASK);
	    XSelectInput(__glutDisplay, window->win, eventMask);

	    if (window->overlay)
		XSelectInput(__glutDisplay, window->overlay->win,
			     window->eventMask & GLUT_OVERLAY_EVENT_FILTER_MASK);
	}
	/* Be sure to set device mask *BEFORE* map window is done. */
	if (workMask & GLUT_DEVICE_MASK_WORK) {
	    __glutUpdateInputDeviceMaskFunc(window);
	}
	/* Be sure to configure window *BEFORE* map window is done. 
	 */
	if (workMask & GLUT_CONFIGURE_WORK) {
	    XWindowChanges changes;

	    changes.x = window->desiredX;
	    changes.y = window->desiredY;
	    if (window->desiredConfMask & (CWWidth | CWHeight)) {
		changes.width = window->desiredWidth;
		changes.height = window->desiredHeight;
		if (window->overlay)
		    XResizeWindow(__glutDisplay, window->overlay->win,
				  window->desiredWidth, window->desiredHeight);
		if (__glutMotifHints != None) {
		    if (workMask & GLUT_FULL_SCREEN_WORK) {
			MotifWmHints hints;

			hints.flags = MWM_HINTS_DECORATIONS;
			hints.decorations = 0;  /* Absolutely no
						   decorations. */
			XChangeProperty(__glutDisplay, window->win,
					__glutMotifHints, __glutMotifHints, 32,
					PropModeReplace, (unsigned char *) &hints, 4);
		    } else {
			XDeleteProperty(__glutDisplay, window->win, __glutMotifHints);
		    }
		}
	    }
	    if (window->desiredConfMask & CWStackMode) {
		changes.stack_mode = window->desiredStack;
		/* Do not let glutPushWindow push window beneath the
		   underlay. */
		if (window->parent && window->parent->overlay && window->desiredStack == Below) {
		    changes.stack_mode = Above;
		    changes.sibling = window->parent->overlay->win;
		    window->desiredConfMask |= CWSibling;
		}
	    }
	    XConfigureWindow(__glutDisplay, window->win,
			     window->desiredConfMask, &changes);
	    window->desiredConfMask = 0;
	}
	/* Be sure to establish the colormaps *BEFORE* map window
	   is done. */
	if (workMask & GLUT_COLORMAP_WORK) {
	    __glutEstablishColormapsProperty(window);
	}
	if (workMask & GLUT_MAP_WORK) {
	    switch (window->desiredMapState) {
	    case WithdrawnState:
		if (window->parent) {
		    XUnmapWindow(__glutDisplay, window->win);
		} else {
		    XWithdrawWindow(__glutDisplay, window->win,
				    __glutScreen);
		}
		window->shownState = 0;
		break;
	    case NormalState:
		XMapWindow(__glutDisplay, window->win);
		window->shownState = 1;
		break;
	    case IconicState:
		XIconifyWindow(__glutDisplay, window->win, __glutScreen);
		window->shownState = 0;
		break;
	    }
	}
    }
    if (workMask & (GLUT_REDISPLAY_WORK | GLUT_OVERLAY_REDISPLAY_WORK)) {
	if (window->forceReshape) {
	    /* Guarantee that before a display callback is generated
	       for a window, a reshape callback must be generated. */
	    __glutSetWindow(window);
	    window->reshape(window->width, window->height);
	    window->forceReshape = False;
	}
	/* The code below is more involved than otherwise necessary
	   because it is paranoid about the overlay or entire window
	   being removed or destroyed in the course of the callbacks.
	   Notice how the global __glutWindowDamaged is used to
	   record the layers' damage status.  See the code in
	   glutLayerGet for how __glutWindowDamaged is used. The 
	   point is to not have to update the "damaged" field after 
	   the callback since the window (or overlay) may be
	   destroyed (or removed) when the callback returns. */

	if (window->overlay && window->overlay->display) {
	    int num = window->num;
	    Window xid = window->overlay ? window->overlay->win : None;

	    /* If an overlay display callback is registered, we
	       differentiate between a redisplay needed for the
	       overlay and/or normal plane.  If there is no overlay
	       display callback registered, we simply use the
	       standard display callback. */

	    if (workMask & GLUT_REDISPLAY_WORK) {

		/* Render to normal plane. */
		window->renderWin = window->win;
		window->renderCtx = window->ctx;
		__glutWindowDamaged = window->damaged;
		window->damaged = 0;
		__glutSetWindow(window);
		window->display();
		__glutWindowDamaged = 0;
	    }
	    if (workMask & GLUT_OVERLAY_REDISPLAY_WORK) {
		window = __glutWindowList[num];
		if (window && window->overlay &&
		    window->overlay->win == xid && window->overlay->display) {

		    /* Render to overlay. */
		    window->renderWin = window->overlay->win;
		    window->renderCtx = window->overlay->ctx;
		    __glutWindowDamaged = window->overlay->damaged;
		    window->overlay->damaged = 0;
		    __glutSetWindow(window);
		    window->overlay->display();
		    __glutWindowDamaged = 0;
		} else {
		    /* Overlay may have since been destroyed or the
		       overlay callback may have been disabled during
		       normal display callback. */
		}
	    }
	} else {
	    __glutWindowDamaged = window->damaged;
	    window->damaged = 0;
	    if (window->overlay) {
		__glutWindowDamaged |= window->overlay->damaged;
		window->overlay->damaged = 0;
	    }
	    __glutSetWindow(window);
	    window->display();
	    __glutWindowDamaged = 0;
	}
    }
    /* Combine workMask with window->workMask to determine what
       finish and debug work there is. */
    workMask |= window->workMask;

    if (workMask & GLUT_FINISH_WORK) {
	__glutSetWindow(window);
	glFinish();
    }
    if (workMask & GLUT_DEBUG_WORK) {
	GLenum error;

	__glutSetWindow(window);
	while ((error = glGetError()) != GL_NO_ERROR)
	    __glutWarning("GL error: %s", gluErrorString(error));
    }
    /* Strip out dummy, finish, and debug work bits. */
    window->workMask &= ~(GLUT_DUMMY_WORK | GLUT_FINISH_WORK | GLUT_DEBUG_WORK);
    if (window->workMask) {
	/* Leave on work list. */
	return window;
    } else {
	/* Remove current window from work list. */
	return window->prevWorkWin;
    }
#endif
    // _TODO_FOR_WIN32 - take this ret 0 out.
    return 0;
}

/* CENTRY */
void
glutMainLoop(void)
{
    GLUTwindow* window;
    MSG msg;
    int i;

    if (!__glutWindowListSize)
	__glutFatalUsage("main loop entered with no windows created.");
    
    if(!__glutCurrentWindow->display) {
	__glutWarning(
	    "The following is a new check for GLUT 3.0; update your code.");
	__glutFatalError(
	    "display needed for window %d, but no display callback.",
	    __glutCurrentWindow->num);
    } else {
	// get the toplevel window so we can make sure it gets an initial
	// reshape and an initial display call to the respective callbacks

	// get the toplevel window
	while(__glutCurrentWindow->parent)
	    __glutCurrentWindow = __glutCurrentWindow->parent;

	// take care of the entry function (focus in WIN32)
	if(__glutCurrentWindow->entry)
	    (*__glutCurrentWindow->entry)(GLUT_ENTERED);

	// take care of visibility function
	if(__glutCurrentWindow->visibility)
	    (*__glutCurrentWindow->visibility)(__glutCurrentWindow->visState);

	// call the windows reshape and display functions
	// GLUT spec says that we must GUARANTEE that the window
	// gets a refresh and a display before main loop starts
	(*__glutCurrentWindow->reshape)(__glutCurrentWindow->width,
					__glutCurrentWindow->height);
	(*__glutCurrentWindow->display)();
    }
  
    while(1) {

	// grab a message off the queue if one exists
	if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
	    // check for a quit message, and bail if found
	    if(msg.message == WM_QUIT)
		exit(0);

	    // translate and dispatch the message
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	    
	    // check for OpenGL errors if need be
	    if(__glutDebug) {
		GLenum error;
		while ((error = glGetError()) != GL_NO_ERROR)
		    __glutWarning("GL error: %s", gluErrorString(error));
	    }
	} else {
	    // post an idle message (if there is an idle func) to EVERY window
	    // we have to post it to every window, otherwise only the "active"
	    // window will ever see any events (WIN32 is dumb this way)
	    // also combine all redisplays here
	    for(i = 0; i < __glutWindowListSize; i++) {
		// this code effectively combines multiple redisplays
		window = __glutWindowList[i];
		if(window->workMask & GLUT_REDISPLAY_WORK) {
		    PostMessage(window->hwnd, WM_PAINT, 0, 0L);
		    window->workMask &= ~GLUT_REDISPLAY_WORK;
		}
		// if an idle func exists, better call it
		if(__glutIdleFunc)
		    PostMessage(window->hwnd, WM_GLUTIDLE, 0, 0);
	    }
	}
    }
}
/* ENDCENTRY */
