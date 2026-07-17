/* Copyright (c) Mark J. Kilgard, 1995. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <stdio.h>
#include "glutint.h"

/* CENTRY */
void
glutFullScreen(void)
{
  HWND hwnd = __glutCurrentWindow->hwnd;
  __glutCurrentWindow->fullscreen = 1;

  int x = __glutCurrentWindow->desiredX = 0;
  int y = __glutCurrentWindow->desiredY = 0;
  int width = __glutCurrentWindow->desiredWidth = __glutScreenWidth;
  int height = __glutCurrentWindow->desiredHeight = __glutScreenHeight;
  MoveWindow(hwnd, x, y, width, height, TRUE);

  RECT r; 
  GetClientRect(hwnd, &r);
  POINT p1; p1.x = r.left; p1.y=r.top;
  POINT p2; p2.x = r.right; p2.y=r.bottom;
  ClientToScreen(hwnd, &p1);
  ClientToScreen(hwnd, &p2);

  int deltaX = p1.x;
  int deltaY = p1.y;
  int deltaBottom = __glutScreenHeight - p2.y;
  int newwidth = width + 2*deltaX;
  int newheight = deltaY*2 + deltaBottom + height;
  //MoveWindow(hwnd, -deltaX, -deltaY, newwidth, newheight, TRUE);
  SetWindowPos(hwnd, HWND_TOP, -deltaX, -deltaY, newwidth, newheight, 0);

  PostMessage(hwnd, WM_SIZE, 0, MAKELPARAM(newwidth, newheight));
  PostMessage(hwnd, WM_PAINT, 0,0);

  GetClientRect(hwnd, &r);

  
}

/* ENDCENTRY */
