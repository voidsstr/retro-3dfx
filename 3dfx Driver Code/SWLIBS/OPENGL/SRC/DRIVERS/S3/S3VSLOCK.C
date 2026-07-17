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
** $Date: 10/11/00 8:02:55 PM$
*/

#include <windows.h>

#include "s3vcontext.h"

/*
** This file contains the functionality needed to lock/unlock using the
** system lock. 
*/

__GLS3VSystemLock *sLock;

GLboolean
__glS3VInitializeSLock(void)
{
    int ret;
    HDC hdc;

    hdc = GetDC(0);
    ret = ExtEscape(hdc, 0x2890, 0, NULL, 
		    sizeof(__GLS3VSystemLock *), (LPSTR) &sLock);
    ReleaseDC(0, hdc);
    if (ret <= 0) {
	return GL_FALSE;
    }

    return GL_TRUE;
}

/*
** Get the lock information from the system
*/
GLboolean
__glS3VInitSLock(__GLS3Vcontext *hwcx)
{
    hwcx->sLock = sLock;
    hwcx->pid = GetCurrentProcessId();
    return GL_TRUE;
}

/*
** GL_TRUE: nobody touched the lock
** GL_FALSE: somebody touched the lock
*/
GLboolean
__glS3VsLock(__GLS3Vcontext *hwcx)
{
    __GLS3VSystemLock *sLock = hwcx->sLock;

    if (hwcx->gc.buffers.lock.sLockCnt++ != 0) {
	return GL_TRUE;
    }

#ifdef __GL_S3V_SLOCK
    /*
    ** There are several posibilities here:
    ** 1. The lock is set.  The process ID matches.  We return GL_TRUE.
    ** 2. The lock is reset.  We take the lock.
    ** 2a.If the pid matches, nobody touched the lock. Return GL_TRUE.
    ** 2b.If the pid does not match, we release the lock and return GL_FALSE.
    **
    ** During case 2b, we have to lock our surfaces
    */
    if (sLock->lock) {
	return GL_TRUE;
    } else {
	    sLock->lock = 1;
	    if (sLock->pid == hwcx->pid) {
		return GL_TRUE;
	    } else {
		sLock->lock = 0;
		return GL_FALSE;
	    }
    }
#endif

    return GL_FALSE;
}

/*
** GL_TRUE means we don't have to unlock the buffers
** GL_FALSE means we have to perform a buffer unlock
*/
GLboolean
__glS3VsUnlock(__GLS3Vcontext *hwcx)
{
    if (--hwcx->gc.buffers.lock.sLockCnt != 0) {
	return GL_TRUE;
    }

#ifdef __GL_S3V_SLOCK
    __GLS3VSystemLock *sLock = hwcx->sLock;

    /*
    ** 1. The lock is set. We set it. We used it.  Reset it. Return GL_TRUE.
    ** 2. The lock is reset.  We didn't use it. Return GL_FALSE.
    */
    if (sLock->lock) {
	sLock->lock = 0;
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
#else /* __GL_S3V_SLOCK */
    return GL_FALSE;
#endif /* __GL_S3V_SLOCK */
}
