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
** $Date: 10/11/00 7:29:55 PM$ 
**
*/

#include <atscene.h>
#define  _WIN32_LEAN_AND_MEAN_
#include <windows.h>
#include <atgui.h>
#ifdef GLIDE3
#include <glide3.h>
#else
#include <glide.h>
#endif
#include <texus.h>
#include "resource.h"

#define FX_DLL_DEFINITION
#include <3dfx.h>
#include <fxdll.h>


/*---------------------------------------------------------------------
  Control Data
  ---------------------------------------------------------------------*/
static BOOL        busy;
static AtsMaterial *material;
static HBITMAP     texture0;

static Dialog *dialog;
static Edit   *testEdit;
static Button *okButton;
static Button *cancelButton;


/*---------------------------------------------------------------------
  Control Implementation
  ---------------------------------------------------------------------*/
static void doOkButton( WPARAM wParam, LPARAM lParam ) {
    FXUNUSED( wParam );
    FXUNUSED( lParam );
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
    char buffer[64];

    dialog = dlg;

    testEdit     = newEdit( dlg, IDC_TESTEDIT, doNothing );
    sprintf( buffer, 
             "%1.3f %1.3f %1.3f", 
             material->material.diffuse.r,
             material->material.diffuse.g,
             material->material.diffuse.b );
    editSetText( testEdit, buffer );

    okButton     = newButton( dlg, IDOK, doOkButton );
    cancelButton = newButton( dlg, IDCANCEL, doCancelButton );

    // Create Bitmap For Texture[0]
    if ( material->textures[0] ) {
        HDC        hdc;
        BITMAPINFO bmi;
        AtrImg     *img = ((AtsTexture*)material->textures[0])->img[0];
        char       *pBits;
        Gu3dfInfo  tInfo;
        int x, y;
        FxU8  *srcBits;
        FxU8  *dstBits;
        FxU32      size;

        hdc = GetDC( dlg->handle );

        bmi.bmiHeader.biSize        = sizeof( bmi.bmiHeader );
        bmi.bmiHeader.biWidth       = img->width;
        bmi.bmiHeader.biHeight      = -(int)img->height; // Top Down Bitmap
        bmi.bmiHeader.biPlanes      = 1; // MS Says
        bmi.bmiHeader.biBitCount    = 24;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage   = img->width * img->height * 3;
        bmi.bmiHeader.biXPelsPerMeter = 0; // ?       
        bmi.bmiHeader.biYPelsPerMeter = 0; // ?       
        bmi.bmiHeader.biClrUsed       = 0; // Means use max color
        bmi.bmiHeader.biClrImportant  = 0; // All colors are important

        texture0 = CreateDIBSection( hdc,
                                     &bmi,
                                     DIB_RGB_COLORS,
                                     &pBits,
                                     0,
                                     0 );
        ReleaseDC( dlg->handle, hdc );

        if ( !texture0 ) {
            MessageBox( 0, "Failed to create bitmap.\n", "Error", 0 );
        }
        
        size = txInit3dfInfo( &tInfo, GR_TEXFMT_ARGB_8888, &img->width, &img->height, 1, 0 );
        tInfo.data = malloc( size );

        txConvert( &tInfo, img->format, img->width, img->height, img->data, 0, img->table );

        // Fill in Bitmap Bits
        srcBits = tInfo.data;
        dstBits = pBits;
        for( y = 0; y < (int)img->height; y++ ) {
            for( x = 0; x < (int)img->width; x++ ) {
                FxU8 r, g, b;
                b = *srcBits++;
                g = *srcBits++;
                r = *srcBits++;
                srcBits++;
                *dstBits++ = b;
                *dstBits++ = g;
                *dstBits++ = r;
            }
        }

        free( tInfo.data );

        // Display Bitmap
        SendMessage( GetDlgItem( dlg->handle, IDC_DISPLAY ),
                     STM_SETIMAGE,
                     (WPARAM)IMAGE_BITMAP,
                     (LPARAM)texture0 );
    }
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
    material = (AtsMaterial*)object;

    module = GetModuleHandle( "material.dll" );
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

    if ( texture0 ) DeleteObject( texture0 );
    busy = FALSE;
    return object;
}

