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
** $Date: 10/11/00 7:30:34 PM$ 
**
*/

#include <math.h>
#include <3dfx.h>
#define  _WIN32_LEAN_AND_MEAN_
#include <windows.h>

#include <atscene.h>
#include <atmath.h>
#include <atinput.h>
#include <atdemo.h>
#include "resource.h"
#include "view.h"

static FxU32 queryTris( AtrTriSet *set ) {
    FxU32 numTris = 0;
    _AtrTriSetNode *node = set->nodes;
    while( node ) {
        FxU32 *triPtr = node->connectivity;
        while( *(triPtr++) ) numTris++;
        node = node->next;
    }
    return numTris;
}

BOOL CALLBACK 
triSetEditProc(HWND hWnd, UINT uMsg, WPARAM  wParam, LPARAM lParam) {

    switch( uMsg )      {
      case WM_INITDIALOG:
        {
            char         buffer[80];
            AtmSphere    bsphere;
            AtrTriSet   *triset ;

            triset = (AtrTriSet *)lParam;
            centerWindow( hWnd );
            SetWindowLong(hWnd, GWL_USERDATA, lParam);
            setInt( hWnd, IDC_TSET_NTRI, queryTris( triset ));
            atsComputeBSphere((AtsObject *)triset, &bsphere);
            setFloat( hWnd, IDC_TSET_RADIUS, bsphere.radius);
            sprintf( buffer, 
                     "(%f, %f, %f)",
                     bsphere.center[0], bsphere.center[1], bsphere.center[2]);
            setText( hWnd, IDC_TSET_CENTER, buffer );
        }
        return TRUE;
      case WM_COMMAND:
        {
            AtrTriSet *triset = (AtrTriSet *)GetWindowLong(hWnd, GWL_USERDATA);

            switch( LOWORD( wParam)) {
                case IDOK:
                    viewTriSet( triset );
                    EndDialog( hWnd, TRUE) ;
                    break;
                case IDCANCEL:
                    EndDialog( hWnd, TRUE) ;
                    break;
            }
        }
        return TRUE;
    }

    return FALSE;
}

void *
triSetEdit( void *object, HWND parent ) {

    DialogBoxParam( _atGlobals.hInstApp, MAKEINTRESOURCE( IDD_TRISET_EDIT ),
                    parent, triSetEditProc, (LPARAM)object );

    return object;
}
