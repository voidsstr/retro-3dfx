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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished - 
** rights reserved under the Copyright Laws of the United States. 
** 
** 
** 
*/ 
#include <windows.h>
#include <glide.h>
#include <math.h>
#include <GL/gl.h>
#include "glint.h"

#include <stdio.h>

BOOL APIENTRY DllMain(HANDLE hInst, 
                      ULONG  ul_reason_for_call,
                      LPVOID lpReserved) {
    static int refcount = 0;
    GrHwConfiguration hwconfig;
    
    switch( ul_reason_for_call ) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
        if(!refcount) {
	  grGlideInit();
	  grSstQueryHardware(&hwconfig);
	  grSstSelect(0);
        }
        refcount++;
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
        refcount--;
        if(!refcount) {
            grGlideShutdown();
        }
        break;
    }
    return(GL_TRUE);
}

BOOL APIENTRY DllInitialize( DWORD arg1, DWORD arg2, DWORD arg3 )
{ 
  return(GL_FALSE);
} 
