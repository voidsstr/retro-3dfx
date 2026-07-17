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
** $Date: 10/11/00 7:29:53 PM$ 
**
*/

#include <atgui.h>
#include <atscene.h>
#include "ui.h"
#include "resource.h"

#include <stdarg.h>

static Dialog   *dlg;
static MenuItem *okMenu;
static MenuItem *editMenu;
static Tree     *tree;

void doOkMenu( WPARAM wParam,
               LPARAM lParam ) {
    FXUNUSED( lParam );
    FXUNUSED( wParam );
    dialogDone( dlg, FXTRUE );    
}

void doTree( WPARAM wParam,
             LPARAM lParam ) {
    switch( LOWORD( wParam ) ) {
      case NM_DBLCLK:
        MessageBox( 0, "dblclk", "message", 0 );
        break;
    }
    return;
}

void doEditMenu( WPARAM wParam,
                 LPARAM lParam ) {
    char buffer[128];
    TreeItem *item;

    FXUNUSED( lParam );
    FXUNUSED( wParam );

    item = treeGetSelected( tree );

    if ( item ) {
        AtsObject *object = (AtsObject*)treeItemGetData( item );
        treeItemGetText( item, buffer, sizeof( buffer ) );
        if ( atsIsOfType( object, atsGetTypeFromName( "Material" ) ) ) {
            editObject( object, "material\\material.dll", dlg->handle );
        } else if ( atsIsOfType( object, atsGetTypeFromName( "TriSet" ) ) ) {
            editObject( object, "triset\\triset.dll", dlg->handle );
        } else {
            sprintf( buffer, 
                     "Sorry, editing/browsing of type \"%s\" is as yet unimplemented.",
                     atsGetTypeName( atsGetType( object ) ) );

            MessageBox( dlg->handle, buffer, "Under Construction", MB_ICONINFORMATION );
        }
    }

}

void winErrorCallback( FxBool fatal, const char *format, ... ) {
    va_list args;
    char buff[128];
    UINT uType = MB_TASKMODAL|MB_OK|MB_SETFOREGROUND;

#ifdef NOTDEF
    uType |= fatal ? MB_ICONERROR : MB_ICONWARNING;
#endif

    va_start( args, format );
    vsprintf( buff, format, args );
    MessageBox( NULL, buff, "ATU Error", uType);
    va_end( args );

    if ( fatal )
      {
        exit( -1 );
      }
    return;
}

void addNodesToTree( Tree *tree, TreeItem *root, AtsObject *object, char *label ) {
    TreeItem *item;

    AtsType *type = atsGetType( object );
    AtsType *node  = atsGetTypeFromName( "Node" );
    AtsType *group = atsGetTypeFromName( "Group" );
    AtsType *dict = atsGetTypeFromName( "Dict" );
    AtsType *shape = atsGetTypeFromName( "Shape" );

    char name[128];

    strcpy( name, atsGetTypeName( type ) );
    if ( atsIsOfType( object, node ) ) {
        if ( atsNodeGetName( object ) ) {
            strcat( name, ": " );
            strcat( name, atsNodeGetName( object ) );
        }
    }

    if ( label ) {
        strcat( name, ": " );
        strcat( name, label );
    }

    item = treeAddItem( tree, root, name, (DWORD)object );

    if ( atsIsOfType( object, group ) ) {
        FxU32 nChildren, child;
        nChildren = atsGroupGetSize( (AtsNode*)object );
        for( child = 0; child < nChildren; child++ ) {
            addNodesToTree( tree, item, atsGroupGetChild( (AtsNode*)object, child ), 0 );
        }       
    } else if ( atsIsOfType( object, dict ) ) {
        FxU32 nChildren, child;
        nChildren = atsDictGetSize( object );
        for( child = 0; child < nChildren; child++ ) {
            addNodesToTree( tree,
                            item, 
                            ((AtsDict*)object)->entries[child].value,
                            ((AtsDict*)object)->entries[child].key );
        }       
    } else if ( atsIsOfType( object, shape ) ) {
        FxU32 nChildren, child;
        nChildren = atsShapeGetNumParts( (AtsNode*)object );
        for( child = 0; child < nChildren; child++ ) {
            AtsObject *geometry;
            AtsObject *material;
            atsShapeGetPart( (AtsNode*)object, child, &geometry, &material );
            addNodesToTree( tree,
                            item, 
                            material,
                            0 );
            addNodesToTree( tree,
                            item, 
                            geometry,
                            0 );
        }       
    }
}


void createControls( Dialog *dlg_, char *filename ) {
    AtsNode  *top_level;
    dlg = dlg_;

    okMenu   = newMenuItem( dlg, IDOK, doOkMenu );
    editMenu = newMenuItem( dlg, ID_EDITITEM, doEditMenu );
    tree     = newTree( dlg, IDC_TREE, doTree );

    atsInit();
    atuErrorSetCallback( winErrorCallback );

    if ( (top_level = atsFileLoad( filename, NULL )) == NULL ) {
        char message[512];
        sprintf( message, "Couldn't Load File: %s\n", filename );
        MessageBox( dlg->handle, message, "ATS Load Error", MB_ICONERROR );
        dialogDone( dlg, FXTRUE );
    } else {
        AtsNode  *node;
        node = top_level;
        addNodesToTree( tree, 0, node, 0 );
    }

    return;
}
