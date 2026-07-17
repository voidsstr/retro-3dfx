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
** $Date: 10/11/00 7:30:17 PM$ 
**
*/

#include <windows.h>
#include <glide.h>
#include <atutil.h>
#include <atrender.h>
#include <atscene.h>
#include <atinput.h>
#include <ataudio.h>
#include <atdemop.h>
#include <conio.h>
#include <shlobj.h>
#include "resource.h"
#include "afxres.h"
#include "view.h"

#define MAXSEL      25
HWND hWndList ;

BOOL
centerWindow( HWND hWnd ) {
    HWND hWndOwner ;
    RECT rcOwner, rcDlg ;
    int cxOwner, cyOwner, cxDlg, cyDlg ;

    /*   overlapped hWnd */

    hWndOwner = GetParent( hWnd) ;

    GetClientRect( hWndOwner, &rcOwner) ;

    MapWindowPoints( hWndOwner, HWND_DESKTOP, (LPPOINT)&rcOwner, 2) ;

    GetWindowRect( hWnd, &rcDlg) ;

    cxOwner = rcOwner.right - rcOwner.left ;
    cyOwner = rcOwner.bottom - rcOwner.top ;
    cxDlg = rcDlg.right - rcDlg.left ;
    cyDlg = rcDlg.bottom - rcDlg.top ;

    if( ( cxOwner < cxDlg) || ( cyOwner < cyDlg))
        return TRUE ;

    SetWindowPos(   hWnd,
                    HWND_TOP,
                    rcOwner.left + ( cxOwner - cxDlg) / 2,
                    rcOwner.top + ( cyOwner - cyDlg) / 2,
                    0, 0,
                    SWP_NOSIZE) ;

    return TRUE ;
}

static BOOL WINAPI 
AddPathDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    HWND hWndEdit = CTRL(hWnd, IDC_DNAME_EDIT);
    static int modify;
    static int curSel ;
    char buff[80];

    switch( msg) {
        case WM_INITDIALOG:
        {
            centerWindow( hWnd );
 
            modify = lParam;

            if ( lParam ) {
                SetWindowText(hWnd, "Modify Path Element");

                curSel = ListBox_GetCurSel( hWndList) ;
                ListBox_GetText( hWndList, curSel, buff) ;
                Edit_SetText( hWndEdit, buff);
            }
 
            SetFocus(hWndEdit);
        }
            return FALSE ;

        case WM_COMMAND:
        {
            switch( LOWORD( wParam)) {
                case IDOK:
                    Edit_GetText(hWndEdit, buff, sizeof(buff));
                    if ( modify ) {
                        ListBox_DeleteString(hWndList, curSel);
                        ListBox_InsertString(hWndList, curSel, buff);
                    } else {
                        ListBox_AddString(hWndList, buff);
                    }
                    EndDialog( hWnd, TRUE) ;
                    return TRUE ;
                case IDCANCEL:
                    EndDialog( hWnd, TRUE) ;
                    return TRUE ;
                case IDBROWSE:
                    {
                        BROWSEINFO BrInfo ;
                        static LPITEMIDLIST pidlDestination, pidlDesktop;
                        char szPath[ 512] ;

                        /* get the PIDL for the desktop */

                        SHGetSpecialFolderLocation( HWND_DESKTOP, 
                                   CSIDL_DESKTOP, &pidlDesktop) ;

                        ZeroMemory( &BrInfo, sizeof(BrInfo)) ;
                        BrInfo.hwndOwner = hWnd ;
                        BrInfo.ulFlags = BIF_RETURNONLYFSDIRS ;
                        BrInfo.pidlRoot = pidlDesktop;
                        BrInfo.pszDisplayName = szPath;
                        BrInfo.lpszTitle = "Add directory to load path";
                    
                        /* use the shell's folder browser */
                    
                        pidlDestination = SHBrowseForFolder(&BrInfo) ;

                        /* did the user select the cancel button */

                        if (pidlDestination == NULL)
                            return TRUE ;

                        /* transform the PIDL into a pathname */

                        SHGetPathFromIDList( pidlDestination, szPath) ;

                        Edit_SetText( hWndEdit, szPath);

                        return TRUE ;
                    }
            }
        }
        break;
    } 
    return FALSE ;
}

BOOL WINAPI 
LoadPathDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch( msg) {
        case WM_INITDIALOG:
        {
            char *loadPath = atuGetLoadPath();
            char  tmp_path[512];
            char *partial_path;

            hWndList = CTRL(hWnd, IDC_LOADPATH_LIST);

            centerWindow( hWnd );
            if ( loadPath != NULL ) {
                strcpy( tmp_path, loadPath );
                for ( partial_path = strtok( tmp_path, ";" );
                     partial_path != 0 ;
                     partial_path = strtok( NULL, ";" ) ) {
                    ListBox_AddString(hWndList, partial_path);
                }
            }
        }
            return TRUE ;

        case WM_COMMAND:
        {
            switch( LOWORD( wParam))
            {
                case IDOK:
                    {
                        int i, j;
                        char tmp[80];
                        char tmp_path[512] ;

                        tmp_path[0] = 0;
                        i = ListBox_GetCount(hWndList);
                        for ( j = 0; j < i; j++ ) {
                            ListBox_GetText(hWndList, j, tmp);
                            strcat(tmp_path, ";");
                            strcat(tmp_path, tmp);
                        }
                        atuSetLoadPath(tmp_path);
               
                        EndDialog( hWnd, TRUE) ;
                    }
                    return TRUE ;
                case IDCANCEL:
                    EndDialog( hWnd, TRUE) ;
                    return TRUE ;
                case IDC_LOADPATH_LIST:
                    switch ( HIWORD( wParam ) ) {
                    case LBN_DBLCLK:
                        {
                            int i ;

                            i = ListBox_GetCurSel( hWndList) ;
                            if ( i >= 0 ) {
                                DialogBoxParam( _atGlobals.hInstApp, 
                                                MAKEINTRESOURCE(IDD_LOADPATH_ELEMENT), 
                                                hWnd, AddPathDlgProc, 1) ;
                            } else MessageBeep( 0) ;
                        }
                        break;
                     }
                     break;
                case IDC_LOADPATH_ADD:
                    {
                        DialogBoxParam( _atGlobals.hInstApp, 
                                        MAKEINTRESOURCE(IDD_LOADPATH_ELEMENT), 
                                        hWnd, AddPathDlgProc, 0) ;
                    }
                    break;
                case IDC_LOADPATH_DEL:
                    {
                        long lStyle ;

                        lStyle = GetWindowLong( hWndList, GWL_STYLE) ;
                        if( lStyle & LBS_MULTIPLESEL || lStyle & LBS_EXTENDEDSEL) {
                            int nSel[ MAXSEL] ;
                            int wPos ;

                            wPos = (WORD)SendMessage(   hWndList,
                                                        LB_GETSELITEMS,
                                                        MAXSEL, (long)(LPSTR)nSel) ;
                            while( wPos > 0) {
                                ListBox_DeleteString( hWndList, nSel[ --wPos]) ;
                            }
                        } else {
                                int i ;

                                i = ListBox_GetCurSel( hWndList) ;
                                if ( i >= 0 )
                                    ListBox_DeleteString( hWndList, i) ;
                                else MessageBeep( 0) ;
                        }
                   
                    }
                    break ;
                case IDC_LOADPATH_CLEAR:
                    {
                        ListBox_ResetContent( hWndList ) ;
                    }
                    break;
            }
        }
        break ;
    }
    return FALSE ;
}

char *
fileLocate(const char *filename, char *full_path) {
    char buff[128];
    char *result;

    sprintf(buff, "Can't locate file %s: Specify path?", filename);
    switch ( MessageBox( _atGlobals.hInstApp, 
                        buff, "Open Error", MB_YESNO|MB_ICONERROR ) ) {
    case IDYES:
        sprintf(buff, "Specify path for %s", filename);
        result = GetFileName(buff);
        return result;
    case IDNO:
    default:
        return NULL;
    }

}
