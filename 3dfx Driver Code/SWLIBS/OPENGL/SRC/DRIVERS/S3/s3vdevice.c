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
#include <ddraw.h>

#include "wgllib.h"
#include "wglddraw.h"
#include "glDevice.h"
#include "s3vpxlfmt.h"

extern GLboolean __glS3VMapRegisters(int x);

extern GLboolean __glS3VUnmapRegisters(int x);
/*
 * We notice this in order to decide whether to map the registers or 
 * not. We could do this in the per thread init, but this could avoid
 * mapping registers into random (non-rendering) threads. BAC 9/97
 */
DWORD __glS3TLSIndex = TLS_MINIMUM_AVAILABLE;

/*
** allocate/free thread local info
*/

GLboolean __glS3VInitializeSLock(void);

int
__glS3ProcessAttach(int threadId)
{
    if (__wglDDrawOpen() == FALSE) {
	return FALSE;
    }

    __glS3TLSIndex = TlsAlloc();

    if (__glS3TLSIndex == 0xffffffff) {
	return FALSE;
    }

    if (__glS3VMapRegisters(0) == FALSE) {
	return FALSE;
    }

#ifdef __GL_S3V_SLOCK
    if (__glS3VInitializeSLock() == FALSE) {
	return FALSE;
    }
#endif /* __GL_S3V_SLOCK */

    return TRUE;
}

int
__glS3ProcessDetach(int threadId)
{
    if (__glS3VUnmapRegisters(0) == FALSE) {
	return FALSE;
    }

    if (TlsFree(__glS3TLSIndex) == FALSE) {
	return FALSE;
    }

    if (__wglDDrawClose() == FALSE) {
	return FALSE;
    }

    return TRUE;
}


#if 0 /* XXX: Add if you want thread local information */
__GLS3threadArea *__glS3threadArea;

int
__glS3ThreadAttach(int threadId)
{
    /*
    ** XXX: 
    ** should be allocating based on some malloc export from wgl, but they are
    ** not initialized, since we could potentially be called before a
    ** context has been created.
    */
    __glS3threadArea = (__GLS3threadArea *) 
	GlobalAlloc(GPTR, sizeof(__GLS3threadArea));
    if (__glS3threadArea == NULL) {
	return FALSE;
    }

    TlsSetValue(__glS3TLSIndex, (LPVOID) __glS3threadArea);
}

int
__glS3ThreadDetach(int threadId)
{
    TlsSetValue(__glS3TLSIndex, (LPVOID) 0);
}
#endif


extern void __wglInvalidatePixelFormatList();
extern int  __wglS3GetPixelFormat(PIXELFORMATDESCRIPTOR *ppfd, int iPixelFormat);
extern __GLcontext *__glS3VCreateContext(__GLimports *imports, __GLcontextModes *modes);

extern void __glS3VDDrawInitTextureManager(__GLcontext *gc);
extern struct __GLtextureFormatRec *__glLookupTextureFormat(GLenum components, GLenum *baseFormat);

struct __glDeviceStruct __glS3VDevice = {
    __GL_BACK_BUFFER_MASK | __GL_DEPTH_BUFFER_MASK, /* bufferFlags */
    __GL_ALLOCATE_HW_TEXTURE,	/* textureFlags */
    __glLookupTextureFormat,	/* lookupTextureFormat */

    __glS3ProcessAttach,   /* devProcessAttach */
    __glS3ProcessDetach,   /* devProcessDetach */
    NULL /*__glS3ThreadAttach*/,    /* devThreadAttach */
    NULL /*__glS3ThreadDetach*/,    /* devThreadDetach */
    NULL,                  /* devSignal */

    __wglS3GetPixelFormat, /* devGetPixelFormat */
    __wglS3InvalidatePixelFormatList, /* devInvalidatePixelFormatList */

    __glS3VDDrawInitTextureManager, 
    __glS3VCreateContext,  /* devCreateContext */

    __wglDDrawInitDrawable,   /* devFBInitDrawable */
    __wglDIBInitDrawable,  /* devDIBInitDrawable */
    __wglDDrawGetDisplayMasks,/* getDisplayMasks */
    __wglFreeBuffers,			/* freeBuffers */
    __wglDDrawUpdateDrawablePalette,	/* updatePalette */
};

struct __glDeviceStruct *__glDevice = &__glS3VDevice;
