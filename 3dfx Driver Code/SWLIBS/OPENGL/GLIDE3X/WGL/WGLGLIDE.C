/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 2$ 
** $Date: 10/11/00 7:58:36 PM$ 
**
*/

#if defined ( __WGL_USE_GLIDE )
#include "wgllib.h"
#include <glide.h>

BOOL __wglGlideInit(void) {
        return TRUE;
}

BOOL __wglGlideFinish(void) {
        return TRUE;
}

BOOL __wglGlideSwapBuffers(__WGLdrawablePrivate *wglPriv) {
        return TRUE;
}

int  __wglGlideGetDisplayMasks(int *rMask, int *gMask, int *bMask) {
        *rMask = 0xf800;
        *gMask = 0x07E0;
        *bMask = 0x001f;
        return 16;
}

void __wglGlideUpdateBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv) {
}

void __wglGlideFreeBuffer(__GLdrawableBuffer *buf, __WGLdrawablePrivate *wglPriv) {
}

void __wglGlideLockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv) {
      GrLfbInfo_t info;
      GLboolean readmode = glPriv->readmode;
      GrLock_t locktype = GR_LFB_WRITE_ONLY;
      GrLfbWriteMode_t mode = GR_LFBWRITEMODE_565;

      info.size = sizeof( info );

      if (buf->type != GR_BUFFER_AUXBUFFER) {
          if (readmode) {
              mode = GR_LFBWRITEMODE_ANY;
              locktype = GR_LFB_READ_ONLY;
          }

          if (!grLfbLock(locktype, buf->type, mode,
                     GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
            return;
          }
          buf->base = info.lfbPtr;
          buf->byteWidth = info.strideInBytes;
      } else {
	  if (grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_AUXBUFFER, GR_LFBWRITEMODE_ZA16, 
			    GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
		  FxU16 *first;
		  FxU16 *out, *p;
		  int i, j;

		  first = info.lfbPtr;
		  out = buf->base;

		  for (j = 0; j < buf->height; j++) {
			  p = (FxU16 *)first;
			  for (i = 0; i < buf->width; i++) {
				  *out++ = *p++;
			  }
			  first += info.strideInBytes>>1;
		  }
		  grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_AUXBUFFER);
	  }
      }
}

void __wglGlideUnlockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv) {
      GLboolean readmode = glPriv->readmode;
      GrLock_t locktype = GR_LFB_WRITE_ONLY;

      if (buf->type != GR_BUFFER_AUXBUFFER) {
          if (readmode) {
              locktype = GR_LFB_READ_ONLY;
          }
      
          grLfbUnlock(locktype, buf->type);

          buf->base = 0;
          buf->byteWidth = 0;
      } else {
          GrLfbInfo_t info;
          info.size = sizeof( info );
	  if (grLfbLock(GR_LFB_WRITE_ONLY, GR_BUFFER_AUXBUFFER, 
              GR_LFBWRITEMODE_ZA16, 
	      GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
		  FxU16 *first;
		  FxU16 *in, *p;
		  int i, j;

		  first = info.lfbPtr;
		  in = buf->base;

		  for (j = 0; j < buf->height; j++) {
			  p = (FxU16 *)first;
			  for (i = 0; i < buf->width; i++) {
                              *p++ = *in++;
                          }
			  first += info.strideInBytes>>1;
                  }
                  grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_AUXBUFFER);
           }
      }
}

#endif /* __WGL_USE_GLIDE */


