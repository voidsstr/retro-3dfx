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
** $Date: 10/11/00 7:33:30 PM$ 
**
*/

#ifdef __WIN32__

#define  _WIN32_LEAN_AND_MEAN_
#include <windows.h>
#include "atgui.h"

#include <stdio.h>

typedef void *(Func)(void*,HWND);

void *editObject( void *object, const char *dllName, HWND parent ) {
    HINSTANCE hDll;
    Func      *entry;
    void      *newObject;
    hDll = LoadLibrary( dllName );
    if ( !hDll ) {
        char message[256];
        sprintf( message, "Couldn't find required dll: %s\n", dllName );
        MessageBox( NULL, message, "Error", MB_OK|MB_ICONERROR );
        return 0;
    }

    entry = (Func*)GetProcAddress( hDll, "edit" );
    if ( !entry ) {
        char message[256];
        sprintf( message, "Invalid or corrupt dll: %s\n", dllName );
        MessageBox( NULL, message, "Error", MB_OK|MB_ICONERROR );
        return 0;
    }
    newObject = entry( object, parent );
    FreeLibrary( hDll );
    return newObject;
}


#endif
