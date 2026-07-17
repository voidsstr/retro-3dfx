#ifndef _wglddraw_h_
#define _wglddraw_h_

/*
** Copyright 1991-1997, Silicon Graphics, Inc.
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
**
** $Revision: 2$
** $Date: 10/11/00 8:07:01 PM$
*/

LPDIRECTDRAW __wglAllocateDDrawObject(GLvoid);
void __wglFreeDDrawObject(LPDIRECTDRAW);
LPDIRECTDRAW __wglGetDDrawObject(void);
GLint __wglDDrawGetDisplayMasks(GLint *rMask, GLint *gMask, GLint *bMask);
GLvoid __wglDDrawUpdateDrawablePalette(__WGLdrawablePrivate *wglPriv);
GLboolean __wglDDrawDDrawSwapBuffers(__WGLdrawablePrivate *wglPriv);
GLboolean __wglDDrawMemSwapBuffers(__WGLdrawablePrivate *wglPriv);
GLvoid __wglDDrawRestoreSurface(__GLdrawableBuffer *buf,
				__GLdrawablePrivate *glPriv);
GLvoid __wglDDrawUpdateClipList(__WGLdrawablePrivate *wglPriv,
				LPDIRECTDRAWCLIPPER lpClipper);
GLvoid *__wglDDrawGetPrimaryBase(GLvoid);
GLboolean __wglDDrawOpen(GLvoid);
GLboolean __wglDDrawClose(GLvoid);
GLvoid __wglInitDDraw(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
		      GLint bits, __GLbufFallbackInitFn back);
GLvoid __wglInitDDrawDepth(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
			   GLint bits, __GLbufFallbackInitFn back);
GLvoid __wglInitDDrawPrimary(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);

#endif /* _wglddraw_h_ */

