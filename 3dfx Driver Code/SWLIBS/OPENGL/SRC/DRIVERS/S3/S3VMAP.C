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



#define CVXD_APIFUNC_1 1
#define CVXD_APIFUNC_2 2

/* The following are initialized per-process, so they're static.
 */

static DWORD S3regBase;    // Address of the register set

static HANDLE hCVxD;     // Handle of the driver used to get kernel services

GLboolean 
__glS3VMapRegisters(int x)
{
    DWORD       cbBytesReturned;
    DWORD       dwErrorCode;
    DWORD       parmsout[2];
    DWORD       parmsin[2];
    unsigned base, base1, base2;
    DWORD  s3VirgeRegBase;

    // Dynamically load and prepare to call CVXDSAMP
    // The CREATE_NEW flag is not necessary

    if (!hCVxD) {
        hCVxD = CreateFile("\\\\.\\S3OGL.VXD", 0,0,0,
			   CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE, 0);

        if ( hCVxD == INVALID_HANDLE_VALUE ) {
            dwErrorCode = GetLastError();
            if ( dwErrorCode == ERROR_NOT_SUPPORTED ) {
                printf("Unable to open VxD, \n device does not support DeviceIOCTL\n");
            } else {
                printf("Unable to open VxD, Error code: %lx\n", dwErrorCode);
            }
            return GL_FALSE;
        } else {

            // Make 1st VxD call here

            _outp(0x3c4, 0x8);
            _outp(0x3c5, 0x6);

            /* allow io ops */
            _outp(0x3c4, 0x9);
            _outp(0x3c5, 0x0);

            /* get the base address of the video buffers. The registers are 
             * are locates 0x1000000 (16 mbytes) above the buffer base. 
             */

            _outp(0x3d4, 0x59);
            base1 = _inp(0x3d5);
            _outp(0x3d4, 0x5a);
            base2 = _inp(0x3d5);

            base = ((base1<<8)|base2)<<16;
            base += 0x1000000;

	    parmsin[0]=base;
	    parmsin[1]=65536; // The Virge register set is 64k long.

            if ( DeviceIoControl(hCVxD, CVXD_APIFUNC_1,
                    (LPVOID)parmsin, sizeof(parmsin),
                    (LPVOID)parmsout, sizeof(parmsout),
                    &cbBytesReturned, NULL) ) {
		s3VirgeRegBase=parmsout[0];
            } else {
                printf("Device does not support the requested API\n");
                return GL_FALSE;
            }
        }

	S3regBase = s3VirgeRegBase; // Address of the register set
    }

    return GL_TRUE;
}

GLboolean
__glS3VUnmapRegisters(int x)
{
    CloseHandle(hCVxD);
    return GL_TRUE;
}

DWORD
__glS3GetRegBase(void)
{
    return S3regBase;
}
