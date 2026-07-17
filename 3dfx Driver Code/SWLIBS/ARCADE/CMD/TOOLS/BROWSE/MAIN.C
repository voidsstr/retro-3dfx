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
** $Date: 10/11/00 7:29:51 PM$ 
**
*/

#define _WIN32_LEAN_AND_MEAN_
#include <atgui.h>

#include "resource.h"
#include "ui.h"

static void tokenizeCmdString( int *argc, char ***argv, const char *cmdLine ) {
    char *token;
    char *s    = strdup( cmdLine );

    *argc = 1;
    *argv = calloc( sizeof( char* ), 64 );

    for( token = strtok( s, " \n\t" ); 
         token; 
         token = strtok( NULL, " \n\t" ) ) {
        (*argv)[*argc] = token;
        (*argc)++;
    }
    return;
}

static FxBool sDone;

/*-------------------------------------------------------------
  Main Program
  -------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst,
                   HINSTANCE hPrevInst,
                   LPSTR lpCmdLine,
                   int nCmdShow) {
    int  argc;
    char **argv;
    MSG  msg;
    Dialog *mainDlg;
    char   *fileToBrowse;
    char   filter[] = "BOF File\0*.bof\0TDS File\0*.3ds\0All Files\0*.*\0";

    FXUNUSED( hPrevInst );
    FXUNUSED( nCmdShow );

    /*-----------------------------------------------------------------
      Command Line
      -----------------------------------------------------------------*/
    tokenizeCmdString( &argc, &argv, lpCmdLine );

    mainDlg = newDialog( 0, IDD_DIALOG1, hInst, &sDone );
    dialogSetPopup( mainDlg, hInst, IDR_MENU1 );

    if ( argc == 2 ) {
        fileToBrowse = argv[1];
    } else {
        fileToBrowse = calloc( 512, 1 );
        while( !sDone && 
               !openFileDialog( mainDlg->handle, 
                                filter, 
                                OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST, 
                                0, 
                                fileToBrowse, 
                                512 ) ) {
            switch ( MessageBox( mainDlg->handle, 
                                "The browser could not open this file.  Try again?", 
                                "Open Error", 
                                MB_YESNO|MB_ICONERROR ) ) {
              case IDYES:
                continue;
              case IDNO:
              default:
                sDone = FXTRUE;
                break;
            }
        }
    }
        
    if ( !sDone ) {
        createControls( mainDlg, fileToBrowse );
        dialogShow( mainDlg );
    }

    /*-----------------------------------------------------------------
      Message Loop
      -----------------------------------------------------------------*/
    while( GetMessage( &msg,0,0,0 ) && !sDone ) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    deleteDialog( mainDlg );
    
    return 0;
}
