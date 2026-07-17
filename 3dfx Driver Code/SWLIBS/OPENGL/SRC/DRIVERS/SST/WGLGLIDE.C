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
** $Date: 10/11/00 8:03:09 PM$ 
**
*/

#include <glide.h>
#include <ddraw.h>
#include "sstcontext.h"
#include "wgllib.h"
#include "wglDIB.h"
#include "wglmem.h"
#include "gldevice.h"

static int __glIgnoreActivate = 0;
static int refcount = 0;


/*
** per-thread info
*/
typedef struct __GLthreadAreaRec {
    HHOOK messageHook;
} __GLthreadArea;


GLboolean __wglGlideSwapBuffers(__WGLdrawablePrivate *wglPriv) {
	return TRUE;
}

GLint  __wglGlideGetDisplayMasks(GLint *rMask, GLint *gMask, GLint *bMask) {
	*rMask = 0xf800;
	*gMask = 0x07E0;
	*bMask = 0x001f;
	return 16;
}

GLvoid 
__wglGlideUpdateDrawablePalette(__WGLdrawablePrivate *wglPriv)
{
}

GLboolean
__wglGlideOpen(GLvoid)
{
   char *v;
   OSVERSIONINFO osinfo;

   if ( !refcount ) {
      refcount++;

      if (v = getenv("SST_DUALHEAD")) {
          if (atoi(v)) {
             __glIgnoreActivate = 1;
          }
      }
      if (v = getenv("SSTV2_DUALHEAD")) {
         if (atoi(v)) {
             __glIgnoreActivate = 1;
          }
      }

      osinfo.dwOSVersionInfoSize = sizeof(osinfo);
      GetVersionEx(&osinfo);
      if (osinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
          FreeLibrary(GetModuleHandle("GLIDE3X"));
          putenv("SST_DUALHEAD=1");
          putenv("SSTV2_DUALHEAD=1");
          LoadLibrary("GLIDE3X");
      }
      grGlideInit();
      if (!strcmp(grGetString(GR_HARDWARE), "VoodooRush")) {
          __glIgnoreActivate = 2;
      }
      grSstSelect(0);
      return GL_TRUE;
   } else {
      return GL_FALSE;
   }
}

GLboolean
__wglGlideClose(GLvoid)
{
    __GLSSTcontext *hwcx = (__GLSSTcontext *)__wglGetCurrentGC;
    refcount--;

    grSstWinClose(hwcx->glide.state.context);
    return GL_TRUE;
}

/* ----------------------------------------------------------------- */


GLvoid
__wglGlideUpdateClipList(__WGLdrawablePrivate *wglPriv, 
			 LPDIRECTDRAWCLIPPER lpClipper)
{
    __wglDIBUpdateOwnershipBuffer(wglPriv, NULL);
}

static GLboolean
Update(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    __WGLdrawablePrivate *wglPriv;

    wglPriv = (__WGLdrawablePrivate *) glPriv->other;

    __wglUpdateDrawableSize(wglPriv);
    buf->width = wglPriv->width;
    buf->height = wglPriv->height;

    return GL_TRUE;
}

#define	BUF_ALIGN	32	/* x86 cache alignment */
#define BUF_ALIGN_MINUS_1  (BUF_ALIGN - 1)

static GLboolean
UpdateDepth(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    GLuint newSize;
    void *ubase;

    /*
    ** Note: buf->handle points to unaligned base.
    ** buf->base points to aligned base.
    */

    __wglUpdateDrawableSize(wglPriv);

    newSize = wglPriv->width * wglPriv->height * buf->elementSize;
    if (newSize > buf->size) {
	if (buf->handle) {
	    ubase = (*glPriv->realloc)(buf->handle, newSize + BUF_ALIGN_MINUS_1);
	    if (ubase == NULL) {
		__wglError("Mem: Update: Realloc failed");
		return GL_FALSE;
	    }
	} else {
	    ubase = (*glPriv->malloc)(newSize + BUF_ALIGN_MINUS_1);
	    if (ubase == NULL) {
		__wglError("Mem: Update: Malloc failed");
		return GL_FALSE;
	    }
	}
	buf->handle = ubase;
	buf->base = (void *)(((size_t)ubase + BUF_ALIGN_MINUS_1) &
			     ~BUF_ALIGN_MINUS_1);
	assert((size_t)buf->base % BUF_ALIGN == 0);

	buf->size = newSize;
    }

    buf->width = wglPriv->width;
    buf->byteWidth = buf->width * buf->elementSize;
    buf->height = wglPriv->height;

    return GL_TRUE;
}

static GLvoid
Lock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
      GrLfbInfo_t info;
      GrBuffer_t buffer;

      info.size = sizeof( info );
      
      if ( &glPriv->frontBuffer == buf ) {
         buffer = GR_BUFFER_FRONTBUFFER;
      } else {
	 buffer = GR_BUFFER_BACKBUFFER;
      }

      if (buf->readlock) {
	  if (!grLfbLock(GR_LFB_READ_ONLY, buffer, GR_LFBWRITEMODE_ANY,
	        GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
	       return;
	  }
      }

      info.size = sizeof( info );

      if (buf->writelock) {
          if (!grLfbLock(GR_LFB_WRITE_ONLY, buffer,GR_LFBWRITEMODE_565,
	         GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
             return;
          }
      }
      buf->base = info.lfbPtr;
      buf->byteWidth = info.strideInBytes;

}


static GLvoid
LockDepth(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
      GrLfbInfo_t info;

      info.size = sizeof( info );

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

static GLvoid
Unlock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
      GrBuffer_t buffer;

      if ( &glPriv->frontBuffer == buf ) {
         buffer = GR_BUFFER_FRONTBUFFER;
      } else {
	 buffer = GR_BUFFER_BACKBUFFER;
      }

      if (buf->readlock) {
          grLfbUnlock(GR_LFB_READ_ONLY, buffer);
      }
      
      if (buf->writelock) {
          grLfbUnlock(GR_LFB_WRITE_ONLY, buffer);
      }

      buf->base = 0;
      buf->byteWidth = 0;
}

static GLvoid
UnlockDepth(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
      GrLfbInfo_t info;
      info.size = sizeof( info );

      if (grLfbLock(GR_LFB_WRITE_ONLY, GR_BUFFER_AUXBUFFER, 
              GR_LFBWRITEMODE_ZA16, GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {

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

static GLvoid
Fill(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint val,
     GLint x, GLint y, GLint w, GLint h)
{
    __GLcontext *gc = (__GLcontext *)__wglGetCurrentGC;
    grDepthMask(FXFALSE);
    grColorMask(FXTRUE, FXFALSE);

    grBufferClear((GrColor_t) val, 0, 0);

    /* XXXwheeler - hack until Deanna fixes clears */
    grDepthMask(FXTRUE);
    grColorMask(FXTRUE, FXFALSE); 
}

static GLvoid
FillDepth(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint val,
	  GLint x, GLint y, GLint w, GLint h)
{
    __GLcontext *gc = (__GLcontext *)__wglGetCurrentGC;
    grDepthMask(FXTRUE);
    grColorMask(FXFALSE, FXFALSE); 

    grBufferClear(0, 0, val);

    /* XXXwheeler - hack until Deanna fixes clears */
    grDepthMask(FXTRUE);
    grColorMask(FXTRUE, FXFALSE); 
}

static GLvoid
Free(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}


GLvoid
__wglInitGlide(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
	       GLint bits, __GLbufFallbackInitFn back)
{
    buf->depth = bits;
    buf->width = buf->height = 0;	/* not assigned yet */
    buf->handle = buf->base = NULL;	/* not assigned yet */
    buf->size = 0;			/* not assigned yet */
    buf->byteWidth = 0;

    buf->elementSize = ((bits-1) / 8) + 1;

    buf->lockCnt = 0;

    buf->update = Update;
    buf->lock = Lock;
    buf->unlock = Unlock;
    buf->fill = Fill;
    buf->free = Free;
}

GLvoid
__wglInitGlideDepth(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
		       GLint bits, __GLbufFallbackInitFn back)
{
    buf->depth = bits;
    buf->width = buf->height = 0;	/* not assigned yet */
    buf->handle = buf->base = NULL;	/* not assigned yet */
    buf->size = 0;			/* not assigned yet */
    buf->byteWidth = 0;

    buf->elementSize = ((bits-1) / 8) + 1;

    buf->lockCnt = 0;

    buf->update = UpdateDepth;
    buf->lock = LockDepth;
    buf->unlock = UnlockDepth;
    buf->fill = FillDepth;
    buf->free = Free;
}


/*****************************************************************************
**
**  __wglSstMonitorWindowChanges.
**
**  Callback function that parses the message stream looking for messages 
**  of interest to the wgl drawable management code.
**
****************************************************************************/

/* XXXX This is kind of bogus to put this function here, but it is useful
	because we have all this weird pass thru stuff to deal with and
	we use static variable in this file to deal with it.
*/

LRESULT	CALLBACK
__wglSstMonitorWindowChanges(int code, WPARAM wParam, LPARAM lParam)
{
    HHOOK messageHook = ((__GLthreadArea *)TlsGetValue(__wglTLSIndex))->messageHook;
    CWPSTRUCT *callBackMessage = (CWPSTRUCT *) lParam;
    __GLcontext *gc = NULL;
    __WGLcontext *glrc = NULL;

    /*
    ** Examine the message and see if it is a window change type.
    ** Then look to see if is a window associated with an openGL
    ** context.  If it is adjust its buffers, viewport, and scissor
    ** area.  Note that there may be more than one context attached
    ** to a single window.
    */
    switch (callBackMessage->message) {
/* I would argue that we don't need to deal with the CLOSE case, DESTROY is
	enough, but we can figure this out later. */
    case WM_CLOSE:
      wglMakeCurrent( 0, 0 );
      break;
    case WM_ACTIVATE:
	gc = (__GLcontext *)__wglGetCurrentGC;
	glrc = (__WGLcontext *) gc->imports.other;

        if (!__glIgnoreActivate && glrc->isInUse ) {
            if ( LOWORD( callBackMessage->wParam ) == WA_INACTIVE ) {
                grDisable(GR_PASSTHRU);
            } else {
                grEnable(GR_PASSTHRU);
            }
        }
        break;
    case WM_WINDOWPOSCHANGING:
        {
            static int b;
            WINDOWPOS *wp = (WINDOWPOS*)callBackMessage->lParam;
            if ( wp->flags & SWP_HIDEWINDOW && (__glIgnoreActivate == 2 )) {
                /* loathsome hacke for voodoo rush */
                wglMakeCurrent( 0, 0 );
            }
        }
        break;
    case WM_SIZE:
        /* XXX Taco - need some implementation */
        break;
    case WM_WINDOWPOSCHANGED:
    case WM_PALETTECHANGED:
	{
	    __WGLcontext *glrc = NULL;

	    while ((glrc = __wglFindWGLWindow(callBackMessage->hwnd, glrc)) != NULL) {
		/*
		** If the context does not belong to this thread, then return - each
		** thread will look after its own window changes.
		*/
		if (glrc->owner != GetCurrentThreadId()) {
		    continue;
		}

		/*
		** Update the drawable now if the context is current, otherwise
		** just mark it as needing a drawable change.  Which will happen
		** the next time the context is made current.
		*/
		if (glrc->isInUse) {	
		    if (__wglUpdateDrawableBuffers(glrc) == GL_FALSE) {
			/* resize failed */
			/* XXX: What can we possibly do here? */
			;
		    }
		} else {
		    glrc->pendingWindowChange = GL_TRUE;
		}
	    }
	}
	break;
    case WM_DESTROY:
	__wglDestroyDrawable(callBackMessage->hwnd);
	break;
    case WM_DISPLAYCHANGE:
	{
	    static int width, height, depth;
	    int nwidth, nheight, ndepth;
	    ndepth = callBackMessage->wParam;
	    nwidth = LOWORD(callBackMessage->lParam);
	    nheight = LOWORD(callBackMessage->lParam);
	    if (ndepth != depth || nwidth != width || nheight != height) {
		width = nwidth; height = nheight; depth = ndepth;
		(*__glDevice->devInvalidatePixelFormatList)();
	    }
	}
	break;
    default:
	break;
    }	
    return CallNextHookEx(messageHook, code, wParam, lParam);
}

GLvoid
__wglGlideInitDrawable(__WGLdrawablePrivate *wglPriv, __GLcontextModes *modes)
{
    __GLdrawablePrivate *glPriv;
    GLint colorBits;
    GLint accumColorBits;

    glPriv = &wglPriv->glPriv;
    colorBits = modes->rgbBits;
    accumColorBits = modes->accumRedBits + modes->accumGreenBits +
	modes->accumBlueBits + modes->accumAlphaBits;

    /* initialize swap buffers routines first */
    wglPriv->swapBuffers = __wglGlideSwapBuffers;

    /* initialize front/back color buffers */
    if (modes->doubleBufferMode) {
	    __wglInitGlide(&glPriv->frontBuffer, glPriv, modes->indexBits,NULL);
	    __wglInitGlide(&glPriv->backBuffer, glPriv, modes->indexBits,NULL);
    } else {
	    __wglInitGlide(&glPriv->frontBuffer, glPriv, modes->indexBits,NULL);
    }

    glPriv->yInverted = GL_TRUE;

#if __GL_MAX_AUXBUFFERS > 0
    /* initilize the aux color buffers */
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i = 0; i < modes->maxAuxBuffers, i++) {
		__wglInitGlide(&glPriv->auxBuffer[i], glPriv, modes->indexBits,NULL);
	}
    }
#endif /* __GL_MAX_AUXBUFFERS */

    /* initialize the other ancillary buffers */
    if (modes->haveAccumBuffer) {
	__wglInitMem(&glPriv->accumBuffer, glPriv, accumColorBits);
    }
    if (modes->haveDepthBuffer) {
	__wglInitGlideDepth(&glPriv->depthBuffer, glPriv, modes->depthBits, NULL);
    }
    if (modes->haveStencilBuffer) {
	__wglInitDIB(&glPriv->stencilBuffer, glPriv, modes->stencilBits);
    }

    /* owndership buffer is 1 bpp */
    __wglInitDIB(&glPriv->ownershipBuffer, glPriv, 1);
}
