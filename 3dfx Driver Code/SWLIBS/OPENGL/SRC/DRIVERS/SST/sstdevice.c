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

#include "texmgr.h"
#include "glcore.h"
#include "wgllib.h"
#include "glDevice.h"
#include "wglglide.h"

#include "..\ddraw\ddtexmgr.h"
/* XXXX Hohn-  I took a stab at leaving the functionality that we would
		need, but it is all ifdef'd out.
*/

/*
 * We notice this in order to decide whether to map the registers or 
 * not. We could do this in the per thread init, but this could avoid
 * mapping registers into random (non-rendering) threads. BAC 9/97
 */
DWORD __glSstTLSIndex = TLS_MINIMUM_AVAILABLE;

/*
** allocate/free thread local info
*/

int
__glSstProcessAttach(int threadId)
{
    if (__wglGlideOpen() == FALSE) {
	return FALSE;
    }

    __glSstTLSIndex = TlsAlloc();

    if (__glSstTLSIndex == 0xffffffff) {
	return FALSE;
    }

/* XXX Hohn-   We want to add some call to Glide which "maps" our registers
		or "inits our context"
    if ( Glide init Call == FALSE) {
        return FALSE;
    }
*/
    return TRUE;
}

int
__glSstProcessDetach(int threadId)
{
/* XXXX Hohn-	Add call to Glide which "unmaps" our context
    if (Glide close call == FALSE) {
	return FALSE;
    }
*/
    if (TlsFree(__glSstTLSIndex) == FALSE) {
	return FALSE;
    }

    if (__wglGlideClose() == FALSE) {
	return FALSE;
    }

    return TRUE;
}


#if 0
__GLSstThreadArea *__glSstThreadArea;

int
__glSstThreadAttach(int threadId)
{
    /*
    ** XXX: 
    ** should be allocating based on some malloc export from wgl, but they are
    ** not initialized, since we could potentially be called before a
    ** context has been created.
    */
    __glSstThreadArea = (__GLSstThreadArea *) 
	GlobalAlloc(GPTR, sizeof(__GLSstThreadArea));
    if (__glSstThreadArea == NULL) {
	return FALSE;
    }

    TlsSetValue(__glSstTLSIndex, (LPVOID) __glSstThreadArea);
}

int
__glSstThreadDetach(int threadId)
{
    TlsSetValue(__glSstTLSIndex, (LPVOID) 0);
}


#endif

void __glSSTInitTextureManager(__GLcontext *gc)
{
}

extern void __wglInvalidatePixelFormatList();
extern __GLcontext *__glSSTCreateContext(__GLimports *imports, __GLcontextModes *modes);

struct __glDeviceStruct __glSstDevice = {
    __GL_BACK_BUFFER_MASK | __GL_DEPTH_BUFFER_MASK, /* bufferFlags */
    0,					/* textureFlags */
    __glLookupTextureFormat,		/* lookupTextureFormat */

    __glSstProcessAttach,   		/* devProcessAttach */
    __glSstProcessDetach,   		/* devProcessDetach */
    NULL  /*__glSstThreadAttach*/,   	/* devThreadAttach */
    NULL  /*__glSstThreadDetach*/,   	/* devThreadDetach */
    NULL,                  		/* devSignal */

    __wglGetPixelFormat, 		/* devGetPixelFormat */
    __wglInvalidatePixelFormatList, 	/* devInvalidatePixelFormatList */

    __glInitTextureManager, 
    __glSSTCreateContext,  		/* devCreateContext */

    __wglGlideInitDrawable,   		/* devFBInitDrawable */
    __wglDIBInitDrawable,  		/* devDIBInitDrawable */
    __wglGlideGetDisplayMasks,		/* getDisplayMasks */
    __wglFreeBuffers,			/* freeBuffers */
    __wglGlideUpdateDrawablePalette,	/* updatePalette */
};

struct __glDeviceStruct *__glDevice = &__glSstDevice;
