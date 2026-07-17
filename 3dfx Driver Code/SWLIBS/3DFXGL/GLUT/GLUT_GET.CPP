/* Copyright (c) Mark J. Kilgard, 1994. */

/* This program is freely distributable without licensing fees
   and is provided without guarantee or warrantee expressed or
   implied. This program is -not- in the public domain. */

#include <GL/glut.h>
#include "glutint.h"
#include "wgllib.h"

RECT __glutRect;

/* CENTRY */
int
glutGet(GLenum param)
{
  HWND win, root;
  HDC hdc = GetDC(GetDesktopWindow());
  PIXELFORMATDESCRIPTOR pfd, requested_pfd;
  int x = 0, y = 0, value;
  unsigned int width, height, border, depth;

  switch (param) {
  case GLUT_INIT_WINDOW_X:
    return __glutInitX;
  case GLUT_INIT_WINDOW_Y:
    return __glutInitY;
  case GLUT_INIT_WINDOW_WIDTH:
    return __glutInitWidth;
  case GLUT_INIT_WINDOW_HEIGHT:
    return __glutInitHeight;
  case GLUT_INIT_DISPLAY_MODE:
    return __glutDisplayMode;
  case GLUT_WINDOW_X:
	  GetWindowRect(__glutCurrentWindow->hwnd, &__glutRect);
	  return __glutRect.left;
  case GLUT_WINDOW_Y:
	  GetWindowRect(__glutCurrentWindow->hwnd, &__glutRect);
	  return __glutRect.top;
  case GLUT_WINDOW_WIDTH:
      x = 0;
      // if there is not a parent window, then decorations exist
      if(!__glutCurrentWindow->parent) {
	  // get the size of the window decorations
	  // the SM_CYFRAME is the width of the 'sizing' handles
	  x = GetSystemMetrics(SM_CXFRAME) * 2;
      }
      // return the size of the window MINUS the size
      // of the decorations (so return the client area)
      GetWindowRect(__glutCurrentWindow->hwnd, &__glutRect);
      return __glutRect.right - __glutRect.left - x;
  case GLUT_WINDOW_HEIGHT:
      y = 0;
      // if there is not a parent window, then decorations exist
      if(!__glutCurrentWindow->parent) {
	  // get the size of the window decorations
	  // the SM_CYFRAME is the lower 'sizing' handle
	  // the SM_CYCAPTION is the title bar
	  y = GetSystemMetrics(SM_CYFRAME);
	  y += GetSystemMetrics(SM_CYCAPTION);
      }
      // return the size of the window MINUS the size
      // of the decorations (so return the client area)
      GetWindowRect(__glutCurrentWindow->hwnd, &__glutRect);
      return __glutRect.bottom - __glutRect.top - y;
  case GLUT_WINDOW_BUFFER_SIZE:
      return (int)wlbGetColorDepth(__glutCurrentWindow->hdc, 
				   __glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_STENCIL_SIZE:
      return (int)wlbGetStencilBits(__glutCurrentWindow->hdc, 
				    __glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_DEPTH_SIZE:
      return (int)wlbGetDepthBits(__glutCurrentWindow->hdc, 
				  __glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_RED_SIZE:
      return (int)wlbGetRedBits(__glutCurrentWindow->hdc, 
				__glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_GREEN_SIZE:
      return (int)wlbGetRedBits(__glutCurrentWindow->hdc, 
				__glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_BLUE_SIZE:
      return (int)wlbGetBlueBits(__glutCurrentWindow->hdc, 
				 __glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_ALPHA_SIZE:
      return (int)wlbGetAlphaBits(__glutCurrentWindow->hdc, 
				  __glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_ACCUM_RED_SIZE:
      return (int)wlbGetAccumRedBits(__glutCurrentWindow->hdc, 
				     __glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_ACCUM_GREEN_SIZE:
      return (int)wlbGetAccumGreenBits(__glutCurrentWindow->hdc, 
				       __glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_ACCUM_BLUE_SIZE:
      return (int)wlbGetAccumBlueBits(__glutCurrentWindow->hdc, 
				      __glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_ACCUM_ALPHA_SIZE:
      return (int)wlbGetAccumAlphaBits(__glutCurrentWindow->hdc, 
				       __glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_DOUBLEBUFFER:
      return (int)wlbIsDoubleBuffered(__glutCurrentWindow->hdc, 
				      __glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_RGBA:
      return (int)wlbIsRGBA(__glutCurrentWindow->hdc, 
			    __glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_COLORMAP_SIZE:
      value = (int)wlbIsRGBA(__glutCurrentWindow->hdc, 
			     __glutCurrentWindow->pfdIndex);
    if (value) {
      return 0;
    } else {
	// not sure if this is exactly right...
	return (int)(1 << wlbGetColorDepth(__glutCurrentWindow->hdc, __glutCurrentWindow->pfdIndex));
    }
  case GLUT_WINDOW_PARENT:
    return __glutCurrentWindow->parent ?
      __glutCurrentWindow->parent->num + 1 : 0;
  case GLUT_WINDOW_NUM_CHILDREN:
    {
      int num = 0;
      GLUTwindow *children = __glutCurrentWindow->children;

      while (children) {
        num++;
        children = children->siblings;
      }
      return num;
    }
  case GLUT_WINDOW_NUM_SAMPLES:
#if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
    if (__glutIsSupportedByGLX("GLX_SGIS_multisample")) {
      GET_CONFIG(GLX_SAMPLES_SGIS);
      return value;
    } else {
      return 0;
    }
#else
    /* Independent of GLX server support, multisampling not
       supported by GLX client-side. */
    return 0;
#endif
  case GLUT_WINDOW_STEREO:
      return (int)wlbIsStereo(__glutCurrentWindow->hdc, 
			      __glutCurrentWindow->pfdIndex);
  case GLUT_WINDOW_CURSOR:
    return (int)__glutCurrentWindow->cursor;
  case GLUT_SCREEN_WIDTH:
    return GetSystemMetrics(SM_CXSCREEN);
  case GLUT_SCREEN_HEIGHT:
    return GetSystemMetrics(SM_CYSCREEN);
  case GLUT_SCREEN_WIDTH_MM:
      __glutWarning("SCREEN_WIDTH_MM unimplemented in Win32 implementation of GLUT");
      return 0;
  case GLUT_SCREEN_HEIGHT_MM:
      __glutWarning("SCREEN_HEIGHT_MM unimplemented in Win32 implementation of GLUT");
      return 0;
  case GLUT_MENU_NUM_ITEMS:
      return __glutCurrentMenu->num;
      return 0;
  case GLUT_DISPLAY_MODE_POSSIBLE:
      // set up a dummy pfd
      wlbInitPFD(&pfd);
      wlbSetSupportOpenGL(&pfd);
      wlbSetPFDVersion(&pfd,1);
      wlbSetDrawToWindow(&pfd);
      if(__glutDisplayMode & GLUT_RGB)
	  wlbSetRGBA(&pfd);
      if(__glutDisplayMode & GLUT_RGBA)
	  wlbSetRGBA(&pfd);
      if(__glutDisplayMode & GLUT_INDEX)
	  wlbSetColorIndex(&pfd);
      if(__glutDisplayMode & GLUT_SINGLE)
	  wlbSetSingleBuffer(&pfd);
      if(__glutDisplayMode & GLUT_DOUBLE)
	  wlbSetDoubleBuffer(&pfd);
      if(__glutDisplayMode & GLUT_ACCUM)
	  wlbSetAccumBits(&pfd, 1);
      if(__glutDisplayMode & GLUT_ALPHA)
	  wlbSetAlphaBits(&pfd, 1);
      if(__glutDisplayMode & GLUT_DEPTH)
	  wlbSetDepth(&pfd, 1);
      if(__glutDisplayMode & GLUT_STENCIL)
	  wlbSetStencilBits(&pfd, 1);
      if(__glutDisplayMode & GLUT_MULTISAMPLE)
	  __glutWarning("MULTISAMPLE unsupported in Win32 implementation of GLUT");
      if(__glutDisplayMode & GLUT_STEREO)
	  wlbSetStereo(&pfd);
      if(__glutDisplayMode & GLUT_LUMINANCE)
	  __glutWarning("LUMINANCE unsupported in Win32 implementation of GLUT");
      
      wlbSetDriverPFD(&pfd);
      requested_pfd = pfd;
      // first check for a hardware pfd, if not found, check for software
      if(!ChoosePixelFormat(hdc, &pfd)) {
	  wlbSetDriverPFD(&pfd, 0);
	  // if this fails, we don't even have software pfd
	  if(!ChoosePixelFormat(hdc, &pfd))
	      return 0;
      }
	  
      // check the flags
      if(__glutDisplayMode & GLUT_RGB)
	  if(!wlbIsRGBA(&pfd)) 
	      return 0;
      if(__glutDisplayMode & GLUT_RGBA)
	  if(!wlbIsRGBA(&pfd)) 
	      return 0;
      if(__glutDisplayMode & GLUT_INDEX)
	  if(!wlbIsColorIndex(&pfd)) 
	      return 0;
      if(__glutDisplayMode & GLUT_SINGLE)
	  if(wlbIsDoubleBuffered(&pfd)) 
	      return 0;
      if(__glutDisplayMode & GLUT_DOUBLE)
	  if(!wlbIsDoubleBuffered(&pfd)) 
	      return 0;
      if(__glutDisplayMode & GLUT_ACCUM)
	  if(!wlbGetAccumBits(&pfd)) 
	      return 0;
      if(__glutDisplayMode & GLUT_ALPHA)
	  if(!wlbGetAlphaBits(&pfd)) 
	      return 0;
      if(__glutDisplayMode & GLUT_DEPTH)
	  if(!wlbGetDepthBits(&pfd)) 
	      return 0;
      if(__glutDisplayMode & GLUT_STENCIL)
	  if(!wlbGetStencilBits(&pfd)) 
	      return 0;
      if(__glutDisplayMode & GLUT_MULTISAMPLE) {
	  return 0;
      }
      if(__glutDisplayMode & GLUT_STEREO)
	  if(!wlbIsStereo(&pfd))
	      return 0;
      if(__glutDisplayMode & GLUT_LUMINANCE) {
	  return 0;
      }
  case GLUT_ELAPSED_TIME:
    {
      struct timeval elapsed, beginning, now;

      __glutInitTime(&beginning);
      GETTIMEOFDAY(&now);
      TIMEDELTA(elapsed, now, beginning);
      /* Return elapsed milliseconds. */
#if defined(__vms)
      return (int) (elapsed.val / TICKS_PER_MILLISECOND);
#else
      return (int) ((elapsed.tv_sec * 1000) + (elapsed.tv_usec / 1000));
#endif
    }
  default:
    __glutWarning("invalid glutGet parameter: %d", param);
    return -1;
  }
}
/* ENDCENTRY */

