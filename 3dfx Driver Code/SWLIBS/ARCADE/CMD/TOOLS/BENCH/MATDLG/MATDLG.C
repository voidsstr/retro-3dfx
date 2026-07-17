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
** $Date: 10/11/00 7:29:48 PM$ 
**
*/

#include <3dfx.h>
#define FX_DLL_DEFINITION
#include <fxdll.h>
#define  _WIN32_LEAN_AND_MEAN_
#include <windows.h>
#include <atgui.h>
#include "resource.h"

#include <atrender.h>

/*---------------------------------------------------------------------
  Control Data
  ---------------------------------------------------------------------*/
static BOOL        busy;
static AtrMaterial *material;
static AtrMaterial *refMaterial;

static Dialog *dialog;
static Button *okButton;
static Button *cancelButton;

/*---------------------------------------------------------------------
  Control Implementation
  ---------------------------------------------------------------------*/
static void doOkButton( WPARAM wParam, LPARAM lParam ) {
    FXUNUSED( wParam );
    FXUNUSED( lParam );
//    atrMaterialAssign( refMaterial, material );
    dialogDone( dialog, FXTRUE );
    return;
}

static void doCancelButton( WPARAM wParam, LPARAM lParam ) {
    FXUNUSED( wParam );
    FXUNUSED( lParam );
    dialogDone( dialog, FXTRUE );
    return;
}

/*---------------------------------------------------------------------
  Control Setup Function
  ---------------------------------------------------------------------*/
static void setupControls( Dialog *dlg ) {
    dialog = dlg;

    okButton     = newButton( dlg, IDOK, doOkButton );
    cancelButton = newButton( dlg, IDCANCEL, doCancelButton );
}

/*---------------------------------------------------------------------
  User Entry Point
  ---------------------------------------------------------------------*/
FX_EXPORT void *edit( void *object, HWND parent ) {
    HINSTANCE module;
    FxU32     done = FXFALSE;

    if ( busy ) {
        MessageBox( parent, 
                    "Object edit DLL's are non-reentrant and this one is busy.",
                    "Try Again Later",
                    MB_OK|MB_ICONINFORMATION );
        return 0;
    } 

    busy = TRUE;
//    refMaterial = (AtrMaterial*)object;
//    material = atrMaterialClone( refMaterial );

    module = GetModuleHandle( "matdlg.dll" );
    if ( !module ) {
        MessageBox( parent, 
                    "Couldn't retrieve module handle.\n", 
                    "Error", 
                    MB_OK|MB_ICONERROR );
        return 0;
    }
    newModalDialog(parent, 
                   IDD_DIALOG1, 
                   module,
                   setupControls,
                   &done );
//    object = (void*)refMaterial;
    object = (void*)1;
    busy = FALSE;
    return object;
}

