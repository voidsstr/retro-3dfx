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
** $Date: 10/11/00 7:29:59 PM$ 
**
*/


#include <math.h>
#include <3dfx.h>
#define FX_DLL_DEFINITION
#include <fxdll.h>
#define  _WIN32_LEAN_AND_MEAN_
#include <windows.h>
#include <atgui.h>
#include "resource.h"

#include <atscene.h>
#ifdef GLIDE3
#include <glide3.h>
#else
#include <glide.h>
#endif

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

static float getBSphereRad( AtrTriSet *set ) {
    float mx = 0.0f, my = 0.0f, mz = 0.0f;
    _AtrTriSetNode *node = set->nodes;
    while( node ) {
        FxU32 vertex;
        for( vertex = 0; 
             vertex < node->numVertices;
             vertex++ ) {
            mx = ( node->vertices[vertex].x > mx )?node->vertices[vertex].x:mx;
            my = ( node->vertices[vertex].y > my )?node->vertices[vertex].y:my;
            mz = ( node->vertices[vertex].z > mz )?node->vertices[vertex].z:mz;
        }
        node = node->next;
    }
    return (float)sqrt( mx * mx + my * my + mz + mz );
}

/*---------------------------------------------------------------------
  Control Data
  ---------------------------------------------------------------------*/
static BOOL        busy;
static AtrTriSet   *triset;
static BOOL        threadDone;

static Dialog *dialog;
static Edit   *testEdit;
static Button *okButton;
static Button *cancelButton;

/*---------------------------------------------------------------------
  Control Implementation
  ---------------------------------------------------------------------*/
static void doOkButton( WPARAM wParam, LPARAM lParam, void *data ) {
    FXUNUSED( wParam );
    FXUNUSED( lParam );
    dialogDone( dialog, FXTRUE );
    return;
}

static void doCancelButton( WPARAM wParam, LPARAM lParam, void *data ) {
    FXUNUSED( wParam );
    FXUNUSED( lParam );
    dialogDone( dialog, FXTRUE );
    return;
}

/*---------------------------------------------------------------------
  Control Setup Function
  ---------------------------------------------------------------------*/
static void setupControls( Dialog *dlg ) {
    char buffer[64];
    FxU32 nTris;

    dialog = dlg;

    testEdit     = newEdit( dlg, IDC_NUMTRIS, doNothing, NULL );
    nTris = queryTris( triset );
    sprintf( buffer, 
             "%d Triangles",
             nTris );
    editSetText( testEdit, buffer );

    okButton     = newButton( dlg, IDOK, doOkButton, NULL );
    cancelButton = newButton( dlg, IDCANCEL, doCancelButton, NULL );

    return;
}

/*---------------------------------------------------------------------
  Thread Body
  ---------------------------------------------------------------------*/
static DWORD ThreadFunc( DWORD lParam ) {
    AtrMaterial mat;
    AtrCamera   camera;
    AtrXform    rotation;
    AtrLight    light;
    float       rot = 0.0f;

    atrInit( NULL, NULL );

    atrLightDefault( &light );
    atrAddLight( &light, 0 );

    atrMaterialDefault( &mat );
    atrMaterialSetup( &mat, ATR_MAT_GSHADE );
    mat.emissive.r = 1.0f;
    mat.emissive.g = 1.0f;
    mat.emissive.b = 1.0f;
    atrPushMaterial( &mat );

    atrCameraDefault( &camera );
    atrXformSetTranslation( &(camera.lcsToWCS), 0.0f, 0.0f, -getBSphereRad( triset ) * 4.0f );
    atrSelectCamera( &camera );

    atrXformSetIdentity( &rotation );

    while( !threadDone ) {
        atrPushXform( &rotation );
        atrClearCanvas( 0.0f, 0.0f, 1.0f, 0 );
        atrRenderTriSetWF( triset );
        atrSwapBuffer( 1 );
        // inc rotation
        rot += ATM_DEGREE;
        if ( rot > 360.0f * ATM_DEGREE ) rot = 0.0f;
        atrXformSetYRotation( &rotation, rot );
        atrPopXform( 0 );
        grSstIdle();
    }
    atrShutdown();
    return 0;
}

/*---------------------------------------------------------------------
  User Entry Point
  ---------------------------------------------------------------------*/
FX_EXPORT void *edit( void *object, HWND parent ) {
    HINSTANCE module;
    FxU32     done = FXFALSE;
    HANDLE      hThread;
    DWORD       threadID;
    DWORD       threadBusy;

    if ( busy ) {
        MessageBox( parent, 
                    "Object edit DLL's are non-reentrant and this one is busy.",
                    "Try Again Later",
                    MB_OK|MB_ICONINFORMATION );
        return 0;
    } 

    busy = TRUE;
    triset = (AtrTriSet*)object;

    /* Spin off render thread */
    hThread=CreateThread(NULL,
                         10000,
                         (LPTHREAD_START_ROUTINE)ThreadFunc,
                         NULL,
                         0,
                         &threadID);

    /* Create Input Dialog */
    module = GetModuleHandle( "triset.dll" );
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
                   &done, NULL );
    threadDone = FXTRUE;
    
    do {
        GetExitCodeThread( hThread, &threadBusy );
    } while ( threadBusy == STILL_ACTIVE );

    busy = FALSE;
    return object;
}

