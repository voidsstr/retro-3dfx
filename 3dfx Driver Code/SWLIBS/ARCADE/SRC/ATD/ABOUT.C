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
** $Date: 10/11/00 7:33:18 PM$ 
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
    HWND hWndName = CTRL(hWnd, IDC_DRIVER_NAME);
    HWND hWndFBMem = CTRL(hWnd, IDC_FB_MEM);
    HWND hWndTFX0Mem = CTRL(hWnd, IDC_TFX0_MEM);
    HWND hWndTFX1Mem = CTRL(hWnd, IDC_TFX1_MEM);
    HWND hWndTFX1MemL = CTRL(hWnd, IDC_LABEL_TMU1);

    switch( msg) {
        case WM_INITDIALOG:
        {
            char buff[40];

            sprintf(buff, "About %s", _atGlobals.appName);
            SetWindowText(hWnd, buff);
            SendMessage( hWndName, WM_SETTEXT, (WPARAM)0, (LPARAM)
                        (( _atGlobals.driverName == NULL ) ? "Glide" : 
                        _atGlobals.driverName));
            sprintf(buff, "%d", _atGlobals.caps.pfxMem);
            SendMessage( hWndFBMem, WM_SETTEXT, (WPARAM)0, (LPARAM)buff);
            sprintf(buff, "%d", _atGlobals.caps.tfxConfig[0].tfxMem);
            SendMessage( hWndTFX0Mem, WM_SETTEXT, (WPARAM)0, (LPARAM)buff);
            sprintf(buff, "%d", _atGlobals.caps.tfxConfig[1].tfxMem);
            SendMessage( hWndTFX1Mem, WM_SETTEXT, (WPARAM)0, (LPARAM)buff);
            if ( _atGlobals.caps.numTex == 1 ) {
                EnableWindow(hWndTFX1Mem, FALSE);
                EnableWindow(hWndTFX1MemL, FALSE);
            }
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
