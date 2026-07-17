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
** $Date: 10/11/00 7:30:35 PM$ 
**
*/

#include <atgui.h>
#include <atscene.h>
#include <atinput.h>
#include <atdemo.h>
#include "ui.h"
#include "afxres.h"
#include "resource.h"

#include <stdarg.h>

extern AtsNode *model ;
AtmCMesh *cmesh;
AtmCMesh *buildCMesh(AtsNode *n);

void *materialEdit( void *object, HWND parent );
void *triSetEdit( void *object, HWND parent );
void *nodeEdit( void *object, HWND parent );

void doOkMenu( WPARAM wParam, LPARAM lParam, void *data ) {
    EditFile *f = data;
    FXUNUSED( lParam );
    FXUNUSED( wParam );
    dialogDone( f->dlg, FXTRUE );    
}

void doTree( WPARAM wParam, LPARAM lParam, void *data ) {
    EditFile *f = data;
    switch( LOWORD( wParam ) ) {
      case NM_DBLCLK:
        MessageBox( 0, "dblclk", "message", 0 );
        break;
    }
    return;
}

void viewNode( AtsObject *object );

void doEditMenu( WPARAM wParam, LPARAM lParam, void *data ) {
    EditFile *f = data;
    char buffer[128];
    char text[128];
    TreeItem *item;

    FXUNUSED( lParam );
    FXUNUSED( wParam );

    item = treeGetSelected( f->tree );

    if ( item ) {
        AtsObject *object = (AtsObject*)treeItemGetData( item );
        treeItemGetText( item, text, sizeof( text ) );
        if ( atsIsOfType( object, atsGetTypeFromName( "Material" ) ) ) {
            materialEdit( object, f->dlg->handle );
        } else if ( atsIsOfType( object, atsGetTypeFromName( "TriSet" ) ) ) {
            triSetEdit( object, f->dlg->handle );
        } else if ( atsIsOfType( object, atsGetTypeFromName( "Node" ) ) ) {
            nodeEdit( object, f->dlg->handle );
        } else {
            sprintf( buffer, 
                     "Sorry, editing/browsing name \"%s\" of type \"%s\" is as yet unimplemented.",
                     text, atsGetTypeName( atsGetType( object ) ) );

            MessageBox( f->dlg->handle, buffer, 
                         "Under Construction", MB_ICONINFORMATION );
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
    AtsType *nodeType  = atsGetTypeFromName( "Node" );
    AtsType *groupType = atsGetTypeFromName( "Group" );
    AtsType *dictType = atsGetTypeFromName( "Dict" );
    AtsType *shapeType = atsGetTypeFromName( "Shape" );
    AtsType *materialType = atsGetTypeFromName( "Material" );
    AtsType *textureType = atsGetTypeFromName( "Texture" );
    AtsType *imageType = atsGetTypeFromName( "Image" );

    char name[128];

    sprintf( name, " %s (0x%lx)", atsGetTypeName( type ), object );
    if ( atsIsOfType( object, nodeType ) ) {
        if ( atsNodeGetName( object ) ) {
            strcat( name, ": " );
            strcat( name, atsNodeGetName( object ) );
        }
    } else if ( atsIsOfType( object, imageType ) ) {
        AtrImg *img = (AtrImg *)object;

        if ( img->name ) {
            strcat( name, ": " );
            strcat( name, img->name );
        }
    }

    if ( label ) {
        strcat( name, ": " );
        strcat( name, label );
    }

    item = treeAddItem( tree, root, name, (DWORD)object );

    if ( atsIsOfType( object, groupType ) ) {
        FxU32 nChildren, child;
        nChildren = atsGroupGetSize( (AtsNode*)object );
        for( child = 0; child < nChildren; child++ ) {
            addNodesToTree( tree, item, atsGroupGetChild( (AtsNode*)object, child ), 0 );
        }       
    } else if ( atsIsOfType( object, dictType ) ) {
        FxU32 nChildren, child;
        nChildren = atsDictGetSize( object );
        for( child = 0; child < nChildren; child++ ) {
            addNodesToTree( tree,
                            item, 
                            ((AtsDict*)object)->entries[child].value,
                            ((AtsDict*)object)->entries[child].key );
        }       
    } else if ( atsIsOfType( object, shapeType ) ) {
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
    } else if ( atsIsOfType( object, materialType ) ) {
        AtsMaterial *mat = (AtsMaterial *)object;
        if ( mat->successor )
            addNodesToTree( tree, item, mat->successor, 0 );
        if ( mat->textures[0] )
            addNodesToTree( tree, item, mat->textures[0], 0 );
        if ( mat->textures[1] )
            addNodesToTree( tree, item, mat->textures[1], 0 );
    } else if ( atsIsOfType( object, textureType ) ) {
        FxU32 frame;
        AtsTexture *tex = (AtsTexture *)object;
     
        for ( frame = 0; frame < tex->numImages; frame++ ) {
            addNodesToTree( tree, item, tex->img[frame], 0 );
        }
    }
}

static EditFile *f = NULL;

void
editFile(char *fileName) {
    AtsObject *obj;
    AtsType *nodeType = atsGetTypeFromName("Node");
    AtsType *dictType = atsGetTypeFromName("Dict");
    char *q;

    if (( obj = atsFileLoad(fileName, NULL)) == NULL ) {
        atuError(FXFALSE, " could not read file %s\n", fileName);
        return;
    }

    if (!atsIsOfType(obj, nodeType) && !atsIsOfType(obj, dictType)) {
        atuError(FXFALSE, " invalid object type %s\n", atsGetTypeName(atsGetType(obj)));
        return;
    }

    if ( f == NULL ) {
        f = atuMemMalloc(sizeof(EditFile));
        f->obj = obj;
        f->next = openFiles;
        openFiles = f;

        f->dlg = newDialog( 0, IDD_STRUC_VIEW, _atGlobals.hInstApp, 
                             &_atGlobals.done, f );

        dialogSetPopup( f->dlg, _atGlobals.hInstApp, IDR_MENU3 );

        f->editMenu = newMenuItem( f->dlg, ID_EDITITEM, doEditMenu, f );
        f->tree     = newTree( f->dlg, IDC_TREE, doTree, f );
    
        SetWindowText(f->dlg->handle, "Available Models");

        dialogShow( f->dlg );
    }

    q = fileName+strlen(fileName);
    while ( --q != fileName ) {
        if ( q[-1] == '\\' )
             break;
    }

    addNodesToTree( f->tree, 0, obj, q );
       
    if (atsIsOfType(obj, nodeType)) {
        viewNode( obj );
        ShowWindow(_atGlobals.hWndMain, SW_SHOWNORMAL);
    }
}
