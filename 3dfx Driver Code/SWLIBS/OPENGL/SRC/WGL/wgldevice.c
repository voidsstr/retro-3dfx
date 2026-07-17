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
*/
#include "wgllib.h"
#include "glDevice.h"

#include <ddraw.h>

#include "wglddraw.h"
#include "wgldci.h"

extern void __wglInvalidatePixelFormatList();
extern PIXELFORMATDESCRIPTOR **__wglGetPixelFormatList(int *numFormats);
extern void __glInitTextureManager(__GLcontext *gc);
extern const struct __GLtextureFormatRec *
__glLookupTextureFormat(GLenum, GLenum*);

int __wglProcessAttach(int threadIdx);

struct __glDeviceStruct __glGenericDevice = {
    0,                     	/* bufferMask -- malloc all buffers */
#if defined(NO_VRAM_TEXTURES)
    0,                     	/* textureMask -- malloc all textures */
#else
    __GL_ALLOCATE_HW_TEXTURE, 	/* textureMask -- vram textures */
#endif
    __glLookupTextureFormat,	/* lookupTextureFormat */
    __wglProcessAttach,    	/* devProcessAttach */
    NULL,			/* devProcessDetach */
    NULL,                  	/* devThreadAttach */
    NULL,                  	/* devThreadDetach */
    NULL,                  	/* devSignal */

    __wglGetPixelFormat,	/* devGetPixelFormat */
    __wglInvalidatePixelFormatList, /* devInvalidatePixelFormatList */

    __glInitTextureManager,     /*devInitTextureManager */

    __glCoreCreateContext, 	/* devCreateContext */

    NULL,   			/* devFBInitDrawable */
    __wglDIBInitDrawable,  	/* devDIBInitDrawable */
    NULL,			/* getDisplayMasks */
    __wglFreeBuffers,		/* freeBuffers */
    NULL, 			/* updatePalette */
};

struct __glDeviceStruct *__glDevice = &__glGenericDevice;



int
__wglProcessDetachDDraw(int threadIdx)
{
    return __wglDDrawClose();
}

int
__wglProcessDetachDCI(int threadIdx)
{
    return __wglDCIClose();
}

int
__wglProcessAttach(int threadIdx)
{
    if (__wglDDrawOpen() == GL_FALSE) {
	/* DDraw failed..  Try DCI */
	if (__wglDCIOpen() == GL_FALSE) {
	    /* DCI failed too!.  Give up */
	    return GL_FALSE;
	} else {
	    /* aha.  DCI works */
	    __glGenericDevice.devProcessDetach = __wglProcessDetachDCI;
	    __glGenericDevice.devGetDisplayMasks = __wglDCIGetDisplayMasks;
	    __glGenericDevice.devFBInitDrawable = __wglDCIInitDrawable;
	    __glGenericDevice.updatePalette = __wglDCIUpdateDrawablePalette;
	}
    } else {
	/* aha..  DDraw works */
	__glGenericDevice.devProcessDetach = __wglProcessDetachDDraw;
	__glGenericDevice.devGetDisplayMasks = __wglDDrawGetDisplayMasks;
	__glGenericDevice.devFBInitDrawable = __wglDDrawInitDrawable;
	__glGenericDevice.updatePalette = __wglDDrawUpdateDrawablePalette;
    }

    return GL_TRUE;
}
