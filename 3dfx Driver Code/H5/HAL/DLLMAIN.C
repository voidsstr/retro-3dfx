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
** $Revision: 2$
** $Date: 10/11/00 8:30:59 PM$
*/
#if defined(FX_DLL_ENABLE)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gdebug.h>

#ifdef GDBG_INFO_ON
#define LEVEL 0
#else
#define LEVEL 80
#endif

#define DLLNAME "h4hal.dll"

BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID lpvReserved)
{
#ifdef THUNK
	extern BOOL _stdcall h3hal_ThunkConnect32(LPSTR pszDLL16, LPSTR
				pszDLL32, HINSTANCE hInst, DWORD dwReason);
	if(! h3hal_ThunkConnect32("csim16.dll", DLLNAME, hInst, dwReason)) {
	    __asm int 3
	}	
#endif
    if (dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH) {
	char buf[132];
        GDBG_INIT();
	if (GetModuleFileName(GetModuleHandle(DLLNAME),buf,sizeof(buf))) {
	    GDBG_INFO(LEVEL,"DLL path: %s\n",buf);
	} /* silent failure */
    }
    GDBG_INFO(1,"DllMain(%d,%d)\n",hInst,dwReason);
    // Here is our DLL initialization code
    if (dwReason == DLL_PROCESS_ATTACH) {
	// nothing to do
    }

    // Terminate the C run-time after all of our code
    if (dwReason == DLL_THREAD_DETACH) {
	// nothing to do
    }
    if (dwReason == DLL_PROCESS_DETACH) {
	// GMT: fxhal.h just not worth including
	FX_ENTRY void   FX_CALL fxHalShutdownAll(void);
	fxHalShutdownAll();
	GDBG_SHUTDOWN();
    }

    return TRUE;
}
#endif
