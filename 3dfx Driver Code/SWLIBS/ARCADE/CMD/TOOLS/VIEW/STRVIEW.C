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
** $Date: 10/11/00 7:30:33 PM$ 
**
*/

#include <stdarg.h>
#include <windows.h>
#include <commctrl.h>
#include <shellAPI.h>
#include <shlguid.h>
#include <shlobj.h>
#include <3dfx.h>
#include <atscene.h>
#include <atinput.h>
#include <atdemop.h>
#include "view.h"
#include "afxres.h"
#include "resource.h"

EditFile *fileList = NULL;
static HMENU hmenu ;
HWND hWndTree;

void
editItem(HWND hWnd) {
    HTREEITEM hTreeItem = TreeView_GetSelection( hWndTree);
    if ( hTreeItem ) {
        TV_ITEM tvi;
        AtsObject *object;
        char text[256];
        
        tvi.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM ;
        tvi.cchTextMax = sizeof( text) ;
        tvi.pszText = text;
        tvi.hItem = hTreeItem;
        TreeView_GetItem( hWndTree, &tvi) ;
        object = (AtsObject*)(tvi.lParam);
        if ( atsIsOfType( object, atsGetTypeFromName( "Material" ) ) ) {
            materialEdit( object, hWndTree );
        } else if ( atsIsOfType( object, atsGetTypeFromName( "TriSet" ) ) ) {
            viewTriSet( object );
            triSetEdit( object, hWndTree );
        } else if ( atsIsOfType( object, atsGetTypeFromName( "Node" ) ) ) {
            viewNode( object );
            nodeEdit( object, hWnd );
        } else {
            atuError(FXFALSE,
                 "Sorry, editing/browsing name \"%s\" of type \"%s\" is as yet unimplemented.",
                 text, atsGetTypeName( atsGetType( object ) ) );
        }
    }
}

LRESULT CALLBACK structureViewProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam) {
    LPNM_TREEVIEW pnmtv = (LPNM_TREEVIEW)lParam ;
    LPNMHDR pnmhdr = (LPNMHDR)lParam ;

    switch( pnmhdr -> code) {
    case NM_RCLICK:
        {
            POINT pt ;
            TV_HITTESTINFO tvht ;

            // saving the mouse coordinate
            GetCursorPos( &pt) ;
            tvht.pt = pt ;
            // converting the coordinate in the TreeView window
            ScreenToClient( hWndTree, &tvht.pt) ;
            // search text only
            tvht.flags = TVHT_ONITEMLABEL ;
            // did we select an item?
            TreeView_HitTest( hWndTree, &tvht) ;

            // yes, we selected an item
            if( tvht.hItem)
            {
                MENUITEMINFO mii ;

                // visually select that item
                TreeView_Select( hWndTree, tvht.hItem, TVGN_CARET) ;

                /* show the popup menu */

                TrackPopupMenu( hmenu,
                                TPM_BOTTOMALIGN,
                                pt.x,
                                pt.y,
                                0,
                                hWnd,
                                NULL) ;
            }
        }
        break ;
    case TVN_ENDLABELEDIT:
        {
            TV_DISPINFO *ptvdi = (TV_DISPINFO *)lParam ;

            // check if the string is empty
            if( !ptvdi -> item.pszText)
                break ;
            // prepare the information to be passed to the selected item
            ptvdi -> item.mask = TVIF_TEXT ;
            ptvdi -> item.cchTextMax = sizeof( ptvdi -> item.pszText) ;
            TreeView_SetItem( hWndTree, &(ptvdi -> item)) ;
        }
        break ;
    case NM_RETURN:
    case NM_DBLCLK:
        {
            HTREEITEM hTreeItem ;

            /* edit the selected item */
            
            editItem(hWnd);
        }
    }
    return FXTRUE;
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

static void addNodesToTree( HWND tree, HTREEITEM root, AtsObject *object, char *label ) {
    HTREEITEM item;
    TV_INSERTSTRUCT tvis;

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

    tvis.hParent = root ;
    tvis.hInsertAfter = TVI_LAST ;
    tvis.item.mask = TVIF_TEXT | TVIF_PARAM ;
    tvis.item.lParam = (LPARAM)object ;
    tvis.item.pszText = name;
    tvis.item.cchTextMax = strlen( name ) ;
    item = TreeView_InsertItem( hWndTree, &tvis) ;

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
        int frame;
        AtsTexture *tex = (AtsTexture *)object;
     
        for ( frame = 0; frame < tex->numImages; frame++ ) {
            addNodesToTree( tree, item, tex->img[frame], 0 );
        }
    }
}

void
editFile(char *fileName) {
    EditFile *f = NULL;
    AtsObject *obj;
    AtsType *nodeType = atsGetTypeFromName("Node");
    AtsType *dictType = atsGetTypeFromName("Dict");
    char *q;
    HRESULT hres ;
    LPITEMIDLIST pidl ;
    char filePath[256];

    if (( atuFileLocate(fileName, filePath) == NULL ) ||
        (( obj = atsFileLoad(filePath, NULL)) == NULL )) {
        atuError(FXFALSE, " could not read file %s\n", fileName);
        return;
    }

    f = atuMemMalloc(sizeof(EditFile));
    f->obj = obj;
    f->fileName = strdup(filePath);
    f->next = fileList;
    fileList = f;

    q = fileName+strlen(fileName);
    while ( --q != fileName ) {
        if ( q[-1] == '\\' )
             break;
    }

    addNodesToTree( hWndTree, 0, obj, q );
       
    if (atsIsOfType(obj, nodeType)) {
        viewNode( obj );
    }

    hres = SHGetSpecialFolderLocation( _atGlobals.hWndMain, CSIDL_DESKTOP, &pidl) ;

    if( FAILED( hres))
        return ;

    /* add a pidl to the Documents list */

    SHAddToRecentDocs( SHARD_PIDL, pidl) ;

    /* add a document to the Documents list */

    SHAddToRecentDocs( SHARD_PATH, filePath ) ;
}

FxBool structureViewInit(void) {
    RECT rc;

    GetClientRect(_atGlobals.hWndMain, &rc);

    hWndTree = CreateWindowEx(
         WS_EX_CLIENTEDGE,
         WC_TREEVIEW,
         NULL,
         WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_THICKFRAME |
         TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS,
         -5, -5,
         200, rc.bottom - rc.top+10, 
         _atGlobals.hWndMain,               /* parent window */
         (HMENU)CT_TREEVIEW,                /* menu handle, control id */
         _atGlobals.hInstApp,               /* program handle */
         NULL);                             /* create parms */  

    if (!hWndTree){
        atuError(FXTRUE, "Could not create structure view window");
        return FALSE;
    }

    hmenu = GetSubMenu( LoadMenu( _atGlobals.hInstApp,  
                                  MAKEINTRESOURCE(IDR_MENU3)), 
                        0) ;
    if ( !hmenu ) {
        atuError( FXFALSE, "Failed to load popup menu.");
    }

   return FXTRUE;
}
