/* Copyright (c) Mark J. Kilgard, 1994. */

/* This program is freely distributable without licensing fees
   and is provided without guarantee or warrantee expressed or
   implied. This program is -not- in the public domain. */

#include "glutint.h"

void
glutSpaceballMotionFunc(GLUTspaceMotionCB spaceMotionFunc)
{
    __glutWarning("SpaceballMotionFunc unsupported in Win32 implementation of GLUT");
#ifdef TODO_FOR_WIN32
  __glutCurrentWindow->spaceMotion = spaceMotionFunc;
  __glutUpdateInputDeviceMaskFunc = __glutUpdateInputDeviceMask;
  __glutPutOnWorkList(__glutCurrentWindow,
    GLUT_DEVICE_MASK_WORK);
#endif
}

void
glutSpaceballRotateFunc(GLUTspaceRotateCB spaceRotateFunc)
{
    __glutWarning("SpaceballRotateFunc unsupported in Win32 implementation of GLUT");
#ifdef TODO_FOR_WIN32
  __glutCurrentWindow->spaceRotate = spaceRotateFunc;
  __glutUpdateInputDeviceMaskFunc = __glutUpdateInputDeviceMask;
  __glutPutOnWorkList(__glutCurrentWindow,
    GLUT_DEVICE_MASK_WORK);
#endif
}

void
glutSpaceballButtonFunc(GLUTspaceButtonCB spaceButtonFunc)
{
    __glutWarning("SpaceballButtonFunc unsupported in Win32 implementation of GLUT");
#ifdef TODO_FOR_WIN32
  __glutCurrentWindow->spaceButton = spaceButtonFunc;
  __glutUpdateInputDeviceMaskFunc = __glutUpdateInputDeviceMask;
  __glutPutOnWorkList(__glutCurrentWindow,
    GLUT_DEVICE_MASK_WORK);
#endif
}
