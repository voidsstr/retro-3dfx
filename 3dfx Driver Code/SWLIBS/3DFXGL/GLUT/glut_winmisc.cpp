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

/* CENTRY */
void
glutSetWindowTitle(char *title)
{
  assert(!__glutCurrentWindow->parent);

  SetWindowText(__glutCurrentWindow->hwnd, title);

}

void
glutSetIconTitle(char *title)
{
	glutSetWindowTitle(title);
}

void 
glutPositionWindow(int x, int y)
{
    RECT r;

//  __glutCurrentWindow->desiredX = x;
//  __glutCurrentWindow->desiredY = y;
//  __glutCurrentWindow->desiredConfMask |= CWX | CWY;
//  __glutPutOnWorkList(__glutCurrentWindow, GLUT_CONFIGURE_WORK);
    __glutCurrentWindow->fullscreen = 0;
	
    GetWindowRect(__glutCurrentWindow->hwnd, &r);
    MoveWindow(__glutCurrentWindow->hwnd,
	       x, y, r.right - r.left, r.bottom - r.top, TRUE);
}

void
glutReshapeWindow(int w, int h)
{
    if(w <= 0 || h <= 0) 
	__glutWarning(
	    "glutReshapeWindow: non-positive width or height not allowed");

    __glutCurrentWindow->desiredWidth = w;
    __glutCurrentWindow->desiredHeight = h;
//  __glutCurrentWindow->desiredConfMask |= CWWidth | CWHeight;
//  __glutPutOnWorkList(__glutCurrentWindow, GLUT_CONFIGURE_WORK);
    
    RECT r;
    GetWindowRect(__glutCurrentWindow->hwnd, &r);
    POINT p; p.x = r.left; p.y = r.top;
    int dx = 0, dy = 0;
    
    // if there is a parent window, grab the screen coords for this window
    // if there is not a parent window, the coords are already screen coords
    if(__glutCurrentWindow->parent) {
	ScreenToClient(__glutCurrentWindow->parent->hwnd, &p);
    } else {
	// get the size of the window decorations so we can ADD it to the
	// size of the window that we want which effectively reshapes the 
	// CLIENT area to the size specified.
	dx = GetSystemMetrics(SM_CXFRAME) * 2;  // * 2 = left and right borders
	dy = GetSystemMetrics(SM_CYFRAME);
	dy += GetSystemMetrics(SM_CYCAPTION);
    }

    // add the differences to the size of the window
    MoveWindow(__glutCurrentWindow->hwnd, p.x, p.y, w + dx, h + dy, TRUE);
    
    __glutCurrentWindow->fullscreen = 0;
}

void
glutPopWindow(void)
{
  //__glutCurrentWindow->desiredStack = Above;
  //__glutCurrentWindow->desiredConfMask |= CWStackMode;
  //__glutPutOnWorkList(__glutCurrentWindow, GLUT_CONFIGURE_WORK);
}

void
glutPushWindow(void)
{
  //__glutCurrentWindow->desiredStack = Below;
  //__glutCurrentWindow->desiredConfMask |= CWStackMode;
  //__glutPutOnWorkList(__glutCurrentWindow, GLUT_CONFIGURE_WORK);
}

void
glutIconifyWindow(void)
{
  assert(!__glutCurrentWindow->parent);
  //__glutCurrentWindow->desiredMapState = IconicState;
  //__glutPutOnWorkList(__glutCurrentWindow, GLUT_MAP_WORK);
  ShowWindow(__glutCurrentWindow->hwnd, SW_SHOWMINIMIZED);
}

void
glutShowWindow(void)
{
  //__glutCurrentWindow->desiredMapState = NormalState;
  //__glutPutOnWorkList(__glutCurrentWindow, GLUT_MAP_WORK);
  ShowWindow(__glutCurrentWindow->hwnd, SW_SHOW);
}

void
glutHideWindow(void)
{
  //__glutCurrentWindow->desiredMapState = WithdrawnState;
  //__glutPutOnWorkList(__glutCurrentWindow, GLUT_MAP_WORK);
  ShowWindow(__glutCurrentWindow->hwnd, SW_HIDE);
}

/* ENDCENTRY */
