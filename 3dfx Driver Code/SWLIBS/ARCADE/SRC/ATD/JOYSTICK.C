/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
** $Revision: 4$ 
** $Date: 10/11/00 7:33:25 PM$ 
**
*/

#include <glide.h>

#include <atutil.h>
#include <atrender.h>
#include <atinput.h>
#include <ataudio.h>
#include <atdemop.h>
#include <conio.h>

FxBool _atiInitJoystick( void );

#ifdef __WIN32__
#include <windows.h>
#include <cpl.h>
FxBool
launchCPL(HWND hWnd, char *name) {
    HMODULE hmod ;
    FARPROC pfn ;
    int nPages, iRet, i ;
    NEWCPLINFO ncpli[ 10] ;
    char buff[80];

    /* load the specified CPL module */

    if (( hmod = LoadLibrary( name ) ) == NULL ) {
        sprintf(buff, "C:\\WINDOWS\\SYSTEM\\%s", name);
        if (( hmod = LoadLibrary( buff ) ) == NULL ) {
            return FXFALSE;
        }
    }

    /* get the CpiApplet address */

    if (( pfn = GetProcAddress( hmod, "CPlApplet") ) == NULL ) {
        return FXFALSE;
    }

    /* initialize it */

    iRet = (*pfn)( hWnd, CPL_INIT, 0, 0) ;

    /* send the CPL_GETCOUNT message */

    nPages = (*pfn)( hWnd, CPL_GETCOUNT, 0, 0) ;

    /* initialize all the dialogs */

    for( i = 0; i < nPages; i++) {
        ncpli[ i].dwSize = sizeof( NEWCPLINFO) ;
        iRet = (*pfn)( hWnd, CPL_NEWINQUIRE, i, ncpli + i) ;
    }

    /* execute all the dialogs */

    for( i = 0; i < nPages; i++) {
        /* start a page */
        iRet = (*pfn)( hWnd, CPL_DBLCLK, i, ncpli[ i].lData) ;
    }

    /* free the library */

    iRet = (*pfn)( hWnd, CPL_EXIT, 0, 0) ;

    FreeLibrary( hmod) ;

    return FXTRUE;
}

void atdJoystickCalibrate(void) {
    if (!atiState.haveJoystick)
        return;
   
    /* run the windows joystick control applet */

    launchCPL(_atGlobals.hWndMain, "joy.cpl");
    
    _atiInitJoystick();
}
#else
void atdJoystickCalibrate(void) {
    atiJoystickCalibrate();
}
#endif
