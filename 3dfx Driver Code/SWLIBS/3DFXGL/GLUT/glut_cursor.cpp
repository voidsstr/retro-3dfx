/* Copyright (c) Mark J. Kilgard, 1995. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <GL/glut.h>
#include "glutint.h"

HCURSOR __glutCurrentCursor;

typedef struct _CursorTable {
  LPCTSTR lpCursorName;
  HCURSOR hCursor;
} CursorTable;
/* *INDENT-OFF* */

// 1, 3, 4, 5, 6, 7, 14, 15, 16, 17, 18, 19, 20 need some work (not exact cursor )
static CursorTable cursorTable[] = {
  {IDC_SIZEALL, NULL},		  /* GLUT_CURSOR_RIGHT_ARROW */ 
  {IDC_ARROW, NULL},	          /* GLUT_CURSOR_LEFT_ARROW */
  {IDC_SIZEALL, NULL},		  /* GLUT_CURSOR_INFO */
  {IDC_NO, NULL},		  /* GLUT_CURSOR_DESTROY */
  {IDC_SIZEALL, NULL},	  /* GLUT_CURSOR_HELP */
  {IDC_NO, NULL},		  /* GLUT_CURSOR_CYCLE */
  {IDC_SIZEALL, NULL},		  /* GLUT_CURSOR_SPRAY */
  {IDC_WAIT, NULL},		  /* GLUT_CURSOR_WAIT */
  {IDC_IBEAM, NULL},		  /* GLUT_CURSOR_TEXT */
  {IDC_CROSS, NULL},		  /* GLUT_CURSOR_CROSSHAIR */
  {IDC_SIZENS, NULL},	  /* GLUT_CURSOR_UP_DOWN */
  {IDC_SIZEWE, NULL},	  /* GLUT_CURSOR_LEFT_RIGHT */
  {IDC_UPARROW, NULL},		  /* GLUT_CURSOR_TOP_SIDE */
  {IDC_SIZENS, NULL},	  /* GLUT_CURSOR_BOTTOM_SIDE */
  {IDC_SIZEWE, NULL},		  /* GLUT_CURSOR_LEFT_SIDE */
  {IDC_SIZEWE, NULL},	  /* GLUT_CURSOR_RIGHT_SIDE */
  {IDC_SIZENWSE, NULL},	  /* GLUT_CURSOR_TOP_LEFT_CORNER */
  {IDC_SIZENESW, NULL},	  /* GLUT_CURSOR_TOP_RIGHT_CORNER */
  {IDC_SIZENWSE, NULL}, /* GLUT_CURSOR_BOTTOM_RIGHT_CORNER */
  {IDC_SIZENESW, NULL},  /* GLUT_CURSOR_BOTTOM_LEFT_CORNER */
};

/* *INDENT-ON* */

/* CENTRY */
void
glutSetCursor(int cursor)
{
    HCURSOR hCursor;

    if (cursor >= 0 && 
	cursor < sizeof(cursorTable) / sizeof(cursorTable[0])) {
	hCursor = LoadCursor(NULL, cursorTable[cursor].lpCursorName);
    } else {
	/* Special cases. */
	switch (cursor) {
	case GLUT_CURSOR_INHERIT:
	    hCursor = NULL;
	    break;
	case GLUT_CURSOR_NONE:
	    hCursor = LoadCursor(NULL, IDC_ICON);
	    break;
	case GLUT_CURSOR_FULL_CROSSHAIR:
	    __glutWarning("FullCorsshairCursor unsupported in Win32 implementation of GLUT");
	    hCursor = NULL;
	    break;
	}
    }

    __glutCurrentCursor = hCursor;
    SetCursor(hCursor);
}
/* ENDCENTRY */
