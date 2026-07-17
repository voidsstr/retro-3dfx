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
** $Date: 10/11/00 7:29:41 PM$ 
**
*/

#include <windows.h>
#include <commctrl.h>
#include <atscene.h>
#include <atinput.h>
#include "atdemop.h"
#include "atdres.h"

BOOL WINAPI 
AboutProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch( msg) {
        case WM_INITDIALOG:
        {
            char buff[40];

            sprintf(buff, "About %s", _atGlobals.appName);
            SetWindowText(hWnd, buff);
        }
        return TRUE ;

        case WM_COMMAND:
        {
            switch( LOWORD( wParam)) {
                case IDOK:
                    EndDialog( hWnd, TRUE) ;
                    return TRUE ;
                default:
                    return FALSE ;
            }
        }
        default:
            return FALSE ;
    } 
    return FALSE ;
}
