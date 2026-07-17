/* Copyright (c) Mark J. Kilgard, 1994. */

/* This program is freely distributable without licensing fees
   and is provided without guarantee or warrantee expressed or
   implied. This program is -not- in the public domain. */

#include "glutint.h"

void
glutButtonBoxFunc(GLUTbuttonBoxCB buttonBoxFunc)
{
    __glutWarning("ButtonBoxFunc unsupported in Win32 implementation of GLUT");
#ifdef TODO_FOR_WIN32
  __glutCurrentWindow->buttonBox = buttonBoxFunc;
  __glutUpdateInputDeviceMaskFunc = __glutUpdateInputDeviceMask;
  __glutPutOnWorkList(__glutCurrentWindow,
    GLUT_DEVICE_MASK_WORK);
#endif
}

void
glutDialsFunc(GLUTdialsCB dialsFunc)
{
    __glutWarning("DialsFunc unsupported in Win32 implementation of GLUT");
#ifdef TODO_FOR_WIN32
  __glutCurrentWindow->dials = dialsFunc;
  __glutUpdateInputDeviceMaskFunc = __glutUpdateInputDeviceMask;
  __glutPutOnWorkList(__glutCurrentWindow,
    GLUT_DEVICE_MASK_WORK);
#endif
}
