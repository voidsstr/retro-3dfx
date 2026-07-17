//: drventry.c
//: Copyright (C) alt.drivers inc. 1997
//: Glenn Nissen

#include "altgdi.h"

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NewDrvControl                                                          //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

DWORD FAR PASCAL __loadds NewDrvControl
(
    LPVOID lpDevice,
    WORD   wFunction,
    LPVOID lpInData,
    LPVOID lpOutData
)
{
    FARENTRY_DRVCONTROL lpControl;
    DWORD dwResult;

    lpControl = (FARENTRY_DRVCONTROL) appInfo.piDrvControl.lpTargetFunc;
    UNPATCHFUNC( appInfo.piDrvControl );
    dwResult = lpControl( lpDevice, wFunction, lpInData, lpOutData );
    PATCHFUNC( appInfo.piDrvControl );

    return dwResult;
}

