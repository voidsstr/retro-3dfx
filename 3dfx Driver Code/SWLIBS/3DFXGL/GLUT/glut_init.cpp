/* Copyright (c) Mark J. Kilgard, 1994. */

/* This program is freely distributable without licensing fees
   and is provided without guarantee or warrantee expressed or
   implied. This program is -not- in the public domain. */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <GL/glut.h>
#include "glutint.h"

/* GLUT inter-file variables */
/* *INDENT-OFF* */
char *__glutProgramName = NULL;
int __glutArgc = 0;
char **__glutArgv = NULL;
char *__glutGeometry = NULL;
void *__glutDisplay = NULL;
int __glutScreen;
HWND __glutRoot;
int __glutScreenHeight;
int __glutScreenWidth;
GLboolean __glutIconic = GL_FALSE;
GLboolean __glutDebug = GL_FALSE;
unsigned int __glutDisplayMode = GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH;
int __glutConnectionFD;
int __glutInitWidth = 300, __glutInitHeight = 300;
int __glutInitX = -1, __glutInitY = -1;
GLboolean __glutForceDirect = GL_FALSE,
  __glutTryDirect = GL_TRUE;
/* *INDENT-ON* */

static BOOL synchronize = FALSE;

#if defined(__vms)
char *
strdup(const char *string)
{
  char *new;

  new = malloc(strlen(string) + 1);
  if (new == NULL)
    return NULL;
  strcpy(new, string);
  return new;
}
#endif

/*
 *  XParseGeometry()
 *
 *    Parses window geometry according to the
 *    standard X Windows geometry specification
 *    (wwwxhhh+xxx+yyy)
 *
 *    This is not the REAL XParseGeometry, just a
 *    simple hack.
 */
#define SCREEN_WIDTH  (1024)
#define SCREEN_HEIGHT ( 768)
int XParseGeometry(char *geometry, int *x, int *y, unsigned int *width, unsigned int *height)
{
    char sign_x, sign_y;
    int status = 0, old_x, old_y;
	
    /* set the default values */
    old_x = *x;
    old_y = *y;
    sign_x = '\0';
    sign_y = '\0';
    status = 0;

    /* scan the string (form: [+-]xxx[+-]yyy) */
    status = sscanf(geometry, "%[+-]%d%[+-]%d", &sign_x, x, &sign_y, y);

    if(status != 4)
    {
	/* if we didn't get 4 elements, try some other form */
	sign_x = '\0';
	sign_y = '\0';

	/* must be no sign on the first number (form: xxx[+-]yyy) */
	status = sscanf(geometry, "%d%[+-]%d", x, &sign_y, y);
	if(status != 3)
	{
	    /* if we didn't get 3 elements, try some other form */
	    sign_x = '\0';
	    sign_y = '\0';

	    /* have to reset x and y here...they were munged above */
	    *x = old_x;
	    *y = old_y;
			
	    /* must be a full geometry specification (wwwxhhh[+-]xxx[+-]yyy) */
	    sscanf(geometry, "%dx%d%[+-]%d%[+-]%d", 
		   width, height, &sign_x, x, &sign_y, y);
	}
    }

    /* check for negative references */
    if(sign_x == '-')
    {
	*x = SCREEN_WIDTH - *width - *x;
    }
    if(sign_y == '-')
    {
	*y = SCREEN_HEIGHT - *height - *y;
    }

    return 0;
}

void
__glutInitTime(struct timeval *beginning)
{
  static int beenhere = 0;
  static struct timeval genesis;

  if (!beenhere) {
    GETTIMEOFDAY(&genesis);
    beenhere = 1;
  }
  *beginning = genesis;
}

static void
removeArgs(int *argcp, char **argv, int numToRemove)
{
  int i, j;

  for (i = 0, j = numToRemove; argv[j]; i++, j++) {
    argv[i] = argv[j];
  }
  argv[i] = NULL;
  *argcp -= numToRemove;
}

void
glutInit(int *argcp, char **argv)
{
  char *display = NULL;
  char *str;
  struct timeval unused;
  int i;

  if (__glutDisplay) {
    __glutWarning("glutInit being called a second time.");
    return;
  }

  // INITIALIZE LIBRARY VARIABLES THAT ARE INDEPENDENT OF CMD LINE
  RECT dr;
  GetWindowRect(GetDesktopWindow(), &dr);
  __glutScreenWidth = dr.right-dr.left;
  __glutScreenHeight = dr.bottom-dr.top;

  /* determine temporary program name */
  str = strrchr(argv[0], '/');
  if (str == NULL) {
    __glutProgramName = argv[0];
  } else {
    __glutProgramName = str + 1;
  }

  /* make private copy of command line arguments */
  __glutArgc = *argcp;
  __glutArgv = (char **) malloc(__glutArgc * sizeof(char *));
  if (!__glutArgv)
    __glutFatalError("out of memory.");
  for (i = 0; i < __glutArgc; i++) {
    __glutArgv[i] = strdup(argv[i]);
    if (!__glutArgv[i])
      __glutFatalError("out of memory.");
  }

  /* determine permanent program name */
  str = strrchr(__glutArgv[0], '/');
  if (str == NULL) {
    __glutProgramName = __glutArgv[0];
  } else {
    __glutProgramName = str + 1;
  }

  /* parse arguments for standard options */
  for (i = 1; i < __glutArgc; i++) {
    if (!strcmp(__glutArgv[i], "-display")) {
      if (++i >= __glutArgc) {
        __glutFatalError(
          "follow -display option with X display name.");
      }
      __glutWarning("-display option unsupported in Win32 implementation of GLUT");
      display = __glutArgv[i];
      removeArgs(argcp, &argv[1], 2);
    } else if (!strcmp(__glutArgv[i], "-geometry")) {
      int flags, x, y, width, height;

      if (++i >= __glutArgc) {
        __glutFatalError(
          "follow -geometry option with geometry parameter.");
      }
      /* Fix bogus "{width|height} may be used before set"
         warning */
      width = 0;
      height = 0;

      flags = XParseGeometry(__glutArgv[i], &x, &y,
        (unsigned int *) &width, (unsigned int *) &height);
        if (width > 0)
          __glutInitWidth = width;
        if (height > 0)
          __glutInitHeight = height;

      glutInitWindowSize(__glutInitWidth, __glutInitHeight);
        if (x >= 0)
          __glutInitX = x;
        if (y >= 0)
          __glutInitY = y;
	  
      glutInitWindowPosition(__glutInitX, __glutInitY);
      removeArgs(argcp, &argv[1], 2);
    } else if (!strcmp(__glutArgv[i], "-direct")) {
	__glutWarning("-direct option unsupported in Win32 implementation of GLUT");
      if (!__glutTryDirect)
        __glutFatalError(
          "cannot force both direct and indirect rendering.");
      __glutForceDirect = GL_TRUE;
      removeArgs(argcp, &argv[1], 1);
    } else if (!strcmp(__glutArgv[i], "-indirect")) {
	__glutWarning("-indirect option unsupported in Win32 implementation of GLUT");
      if (__glutForceDirect)
        __glutFatalError(
          "cannot force both direct and indirect rendering.");
      __glutTryDirect = GL_FALSE;
      removeArgs(argcp, &argv[1], 1);
    } else if (!strcmp(__glutArgv[i], "-iconic")) {
      __glutIconic = GL_TRUE;
      removeArgs(argcp, &argv[1], 1);
    } else if (!strcmp(__glutArgv[i], "-gldebug")) {
      __glutDebug = GL_TRUE;
      removeArgs(argcp, &argv[1], 1);
    } else if (!strcmp(__glutArgv[i], "-sync")) {
	__glutWarning("-sync option unsupported in Win32 implementation of GLUT");
      synchronize = GL_TRUE;
      removeArgs(argcp, &argv[1], 1);
    } else {
      /* once unknown option encountered, stop command line
         processing */
      break;
    }
  }

  __glutInitTime(&unused);
}

/* CENTRY */
void
glutInitWindowPosition(int x, int y)
{
  __glutInitX = x;
  __glutInitY = y;
}

void
glutInitWindowSize(int width, int height)
{
    if(width > 0 && height > 0) {
	__glutInitWidth = width;
	__glutInitHeight = height;
    }
}

void
glutInitDisplayMode(unsigned int mask)
{
  __glutDisplayMode = mask;
}

/* ENDCENTRY */
