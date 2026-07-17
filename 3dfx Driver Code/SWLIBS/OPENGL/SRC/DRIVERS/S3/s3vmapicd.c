/*
** Copyright 1997, Silicon Graphics, Inc.
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

#include <stdio.h>
#include <windows.h>
#include <vmm.h>
#include <vxdldr.h>
#include <conio.h>

#include <stdio.h>
#include "render.h"
#include "context.h"

#include "s3vcontext.h"
#include "s3virge.h"

#include "drvesc.h"


/* 
 * The following are initialized per-process, so they're static.
 */

static DWORD S3regBase;    // Address of the register set

GLboolean 
__glS3VMapRegisters(int x)
{
    unsigned base, base1, base2;
    DWORD  s3VirgeRegBase;

    HDC hdc;
    GLMAPREG	lpIn;
    GLMAPREGRET	lpOut;

    _outp(0x3c4, 0x8);
    _outp(0x3c5, 0x6);

    /* allow io ops */
    _outp(0x3c4, 0x9);
    _outp(0x3c5, 0x0);

    /* 
     * get the base address of the video buffers. The registers are 
     * are locates 0x1000000 (16 mbytes) above the buffer base. 
     */

    _outp(0x3d4, 0x59);
    base1 = _inp(0x3d5);
    _outp(0x3d4, 0x5a);
    base2 = _inp(0x3d5);

    base = ((base1<<8)|base2)<<16;
    base += 0x1000000;

    lpIn.och.oglcmd.ulSubEsc = 100;
    lpIn.och.oglcmd.fl = 0;

    lpIn.base = base;
    lpIn.length = 65536;

    hdc = GetDC(NULL);

    if( ExtEscape(hdc, OPENGL_CMD, 
		  sizeof(GLMAPREG), (LPCSTR) &lpIn, 
		  sizeof(GLMAPREGRET), (LPSTR) &lpOut) <= 0) {
	printf("Illegal escape\n");
	return GL_FALSE;
    }

    ReleaseDC(NULL, hdc);

    s3VirgeRegBase = (DWORD) lpOut.addr;

    S3regBase = s3VirgeRegBase; // Address of the register set

    return GL_TRUE;
}

GLboolean
__glS3VUnmapRegisters(int x)
{
    return GL_TRUE;
}

GLuint
__glS3GetRegBase(void)
{
    return S3regBase;
}
