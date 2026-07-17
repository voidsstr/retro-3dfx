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
** $Date: 10/11/00 7:33:29 PM$ 
**
*/

#ifdef __WIN32__

#define WIN32_LEAN_AND_MEAN
#include <3dfx.h>
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include "atgui.h"


#include <stdlib.h>
#include <stdio.h>

/*---------------------------------------------------------------
  Dialog Data
  ---------------------------------------------------------------*/
typedef struct _DlgTableNode {
    Dialog               *dlg;
    HWND                 hwnd;
    struct _DlgTableNode *next;
} DlgTableNode;

static DlgTableNode *sDlgTable;

static SetupFunction *modalSetup;
static void          *modalData;
static BOOL          *modalDone;
static Dialog        *modalDlg;


/*---------------------------------------------------------------
  Dialog Implementation
  ---------------------------------------------------------------*/
static FxU32 dialogCommand( Dialog *dlg, WPARAM wParam, LPARAM lParam ) {
    FxU32 counter;
    FxU32 control = LOWORD( wParam );
    for ( counter = 0; counter < dlg->table.numControls; counter++ ) {
        ControlTableEntry *entry = &dlg->table.entries[counter];
        if ( entry->id == control ) {
            (entry->action)(wParam,lParam,entry->data);
            break;
        }
    }
    return 0;
}

static FxU32 dialogHScroll( Dialog *dlg, WPARAM wParam, LPARAM lParam ) {
    FxU32 counter;
    HWND control = (HWND)lParam;
    for ( counter = 0; counter < dlg->table.numControls; counter++ ) {
        ControlTableEntry *entry = &dlg->table.entries[counter];
        if ( entry->handle == control ) {
            (entry->action)(wParam,lParam,entry->data);
            break;
        }
    }
    return 1;
}

static FxU32 dialogVScroll( Dialog *dlg, WPARAM wParam, LPARAM lParam ) {
    FxU32 counter;
    HWND control = (HWND)lParam;
    for ( counter = 0; counter < dlg->table.numControls; counter++ ) {
        ControlTableEntry *entry = &dlg->table.entries[counter];
        if ( entry->handle == control ) {
            (entry->action)(wParam,lParam,entry->data);
            break;
        }
    }
    return 1;
}

static BOOL CALLBACK dlgFuncModeless(HWND    hwndDlg,  // handle to dialog box
                                     UINT    uMsg,     // message
                                     WPARAM  wParam,   // first message parameter
                                     LPARAM  lParam    // second message parameter
                                     ) {
    BOOL   rv = 0;
    Dialog *dlg = 0;
    DlgTableNode *node = sDlgTable;

    while( node ) {
        if ( hwndDlg == node->hwnd ) {
            dlg = node->dlg;
        }
        node = node->next;
    }

    switch( uMsg )      {
      case WM_CONTEXTMENU:
        if ( dlg->popup ) {
             TrackPopupMenu(dlg->popup,
                            0,
                            LOWORD( lParam ),
                            HIWORD( lParam ),
                            0,
                            hwndDlg,
                            0 );
        }
        rv = 1;
        break;
      case WM_CLOSE:
        *(dlg->done) = FXTRUE;
        rv = 1;
        break;
      case WM_COMMAND: // Messages from controls 
        rv =  dialogCommand( dlg, wParam, lParam );
        break;
      case WM_HSCROLL:
        rv = dialogHScroll( dlg, wParam, lParam );
        break;
      case WM_VSCROLL:
        rv = dialogVScroll( dlg, wParam, lParam );
        break;
    }
    return rv;
}

static BOOL CALLBACK dlgFuncModal(HWND    hwndDlg,  // handle to dialog box
                                  UINT    uMsg,     // message
                                  WPARAM  wParam,   // first message parameter
                                  LPARAM  lParam    // second message parameter
                                  ) {
    BOOL   rv = 0;
    Dialog *dlg = modalDlg;

    if ( dlg && *(dlg->done) ) {
        *(dlg->done)=0;
        EndDialog( hwndDlg, 0 );
    }

    switch( uMsg )      {
      case WM_CONTEXTMENU:
        if ( dlg->popup ) {
             TrackPopupMenu(dlg->popup,
                            0,
                            LOWORD( lParam ),
                            HIWORD( lParam ),
                            0,
                            hwndDlg,
                            0 );
        }
        rv = 1;
        break;
      case WM_INITDIALOG:
        modalDlg = calloc( sizeof( Dialog ), 1 );
        modalDlg->done   = modalDone;
        modalDlg->handle = hwndDlg;
        modalDlg->data = modalData;
        modalSetup( modalDlg );
        rv = 1;
        break;
      case WM_COMMAND: // Messages from controls 
        rv =  dialogCommand( dlg, wParam, lParam );
        break;
      case WM_HSCROLL:
        rv = dialogHScroll( dlg, wParam, lParam );
        break;
      case WM_VSCROLL:
        rv = dialogVScroll( dlg, wParam, lParam );
        break;
    }

    return rv;
}


void newModalDialog( HWND parent, FxU32 id, HINSTANCE module, 
                     SetupFunction *setup, BOOL *done, void *data ) {
    InitCommonControls();
    modalSetup = setup;
    modalData = data;
    modalDone  = done;
    modalDlg   = 0;
    DialogBox( module,
               MAKEINTRESOURCE( id ),
               parent,
               dlgFuncModal );
    return;
}

Dialog *newDialog( HWND parent, FxU32 id, HINSTANCE module, BOOL *done,
                   void *data ) {
    Dialog *d = calloc( sizeof( Dialog ), 1 );
    DlgTableNode *node;

    InitCommonControls();

    d->data = data;

    d->handle = CreateDialog(module,
                             MAKEINTRESOURCE( id ),
                             parent,
                             dlgFuncModeless);

    if ( !d->handle ) {
        LPVOID lpMsgBuf;
 
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                      FORMAT_MESSAGE_FROM_SYSTEM,
                      NULL,
                      GetLastError(),
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR) &lpMsgBuf,
                      0,
                      NULL 
                      );

        // Display the string.
        MessageBox( NULL, lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );

        // Free the buffer.
        LocalFree( lpMsgBuf );
        return 0;
    }

    node = sDlgTable;
    if ( !node ) {
        sDlgTable = calloc( sizeof( DlgTableNode ), 1 );
        sDlgTable->hwnd = d->handle;
        sDlgTable->dlg  = d;
    } else {
        while( node->next ) node = node->next;
        node->next = calloc( sizeof( DlgTableNode ), 1 );
        node->next->hwnd = d->handle;
        node->next->dlg  = d;
    }

    d->done = done;

    return d;
}

void deleteDialog( Dialog *dlg ) {
    DestroyWindow( dlg->handle );
}

void dialogShow( Dialog *dlg ) {
    ShowWindow( dlg->handle, SW_SHOW );
}

void dialogDone( Dialog *dlg, BOOL done ) {
    *(dlg->done) = done;
    return;
}

void dialogSetPopup( Dialog *dlg, HINSTANCE module, FxU32 id ) {
    dlg->popup = GetSubMenu( LoadMenu( module, MAKEINTRESOURCE( id ) ), 0 );
    if ( !dlg->popup ) {
        MessageBox( dlg->handle, 
                    "Failed to load popup menu.\n", 
                    "Error", 
                    MB_OK|MB_ICONERROR );
    }
    return;
}

/*---------------------------------------------------------------
  Button Control Implementation
  ---------------------------------------------------------------*/
Button *newButton( Dialog *parent, FxU32 id, Action *action, void *data ) {
    // Fill In Data Structure
    Button *b = calloc( sizeof( Button ), 1 );
    b->handle = GetDlgItem( parent->handle, id );
    b->id     = id;
    b->data   = data;

    if ( b->handle ) {
        // Add To Message Table 
        parent->table.entries[parent->table.numControls].id     = (FxU16)id;
        parent->table.entries[parent->table.numControls].handle = b->handle;
        parent->table.entries[parent->table.numControls].action = action;
        parent->table.entries[parent->table.numControls].data   = data;
        parent->table.numControls++;
        return b;
    } else 
      return 0;
}

/*---------------------------------------------------------------
  Edit Control Implementation
  ---------------------------------------------------------------*/
Edit *newEdit( Dialog *parent, FxU32 id, Action *action, void *data ) {
    Edit *e = calloc( sizeof( Edit ), 1 );
    
    e->handle = GetDlgItem( parent->handle, id );
    e->id     = id;
    e->data   = data;
    if ( e->handle ) {
        // Add To Message Table 
        parent->table.entries[parent->table.numControls].id     = (FxU16)id;
        parent->table.entries[parent->table.numControls].handle = e->handle;
        parent->table.entries[parent->table.numControls].action = action;
        parent->table.entries[parent->table.numControls].data   = data;
        parent->table.numControls++;
        return e;
    } else 
      return 0;
}

void editSetText( Edit *edit, const char *text ) {
    SendMessage( edit->handle, WM_SETTEXT, (WPARAM)0, (LPARAM)text );
}

void editGetText( Edit *edit, char *text, FxU32 size ) {
    SendMessage( edit->handle, WM_GETTEXT, (WPARAM)size, (LPARAM)text );
}

/*---------------------------------------------------------------
  Progress Control Implementation
  ---------------------------------------------------------------*/
Progress *newProgress( Dialog *parent, FxU32 id, void *data ) {
    Progress *p = calloc( sizeof( Progress ), 1 );
    
    p->handle = GetDlgItem( parent->handle, id );
    p->id     = id;
    p->data   = data;
    if ( p->handle ) {
        return p;
    } else 
      return 0;
}

void progressSetRange( Progress *p, FxU16 min, FxU16 max ) {
    SendMessage( p->handle,
                 PBM_SETRANGE,
                 0,
                 MAKELPARAM( min, max ) );
    p->min = min;
    p->max = max;
}

void progressSetPos( Progress *p, FxU16 pos ) {
    if ( p->pos != pos ) {
        SendMessage( p->handle, 
                    PBM_SETPOS,
                    pos,
                    0 );
    }
    p->pos = pos;
}

/*---------------------------------------------------------------
  ListBox Implementation
  ---------------------------------------------------------------*/
ListBox *newListBox( Dialog *parent, FxU32 id, void *data ) {
    ListBox *l = calloc( sizeof( ListBox ), 1 );
    
    l->handle = GetDlgItem( parent->handle, id );
    l->id     = id;
    l->data   = data;
    if ( l->handle ) {
        return l;
    } else 
      return 0;
}

int  listBoxAddString( ListBox *l, const char *string ) {
    return SendMessage(l->handle,
                       LB_ADDSTRING, 
                       0,
                       (LPARAM)string );
}

void listBoxRemoveString( ListBox *l, FxU32 index ) {
    SendMessage( l->handle,
                 LB_DELETESTRING,
                 (WPARAM)index,
                 0 );
}

void listBoxReset( ListBox *l ) {
    SendMessage( l->handle,
                 LB_RESETCONTENT,
                 0,
                 0 );
}

void listBoxPaint( ListBox *l ) {
    SendMessage( l->handle,
                 WM_PAINT,
                 0, 
                 0 );
}
/*---------------------------------------------------------------
  Slider Implementation
  ---------------------------------------------------------------*/
Slider *newSlider( Dialog *parent, FxU32 id, Action *action, void *data ) {
    Slider *s = calloc( sizeof( Slider ), 1 );
    
    s->handle = GetDlgItem( parent->handle, id );
    s->id     = id;
    s->data   = data;
    if ( s->handle ) {
        // Add To Message Table 
        parent->table.entries[parent->table.numControls].id     = (FxU16)id;
        parent->table.entries[parent->table.numControls].handle = s->handle;
        parent->table.entries[parent->table.numControls].action = action;
        parent->table.entries[parent->table.numControls].data   = data;
        parent->table.numControls++;
        return s;
    } else 
      return 0;
}


void sliderSetRange( Slider *s, FxU16 min, FxU16 max ) {
    SendMessage( s->handle,
                 TBM_SETRANGE,
                 1, 
                 MAKELPARAM( min, max ) );
}

void sliderSetPos( Slider *s, FxU16 pos ) {
    SendMessage( s->handle,
                 TBM_SETPOS,
                 1, 
                 pos );
}

FxU16 sliderGetPos( Slider *s ) {
    return (FxU16)SendMessage( s->handle, 
                               TBM_GETPOS,
                               0,
                               0 );
}

/*---------------------------------------------------------------
  Spin Controls
  ---------------------------------------------------------------*/
Spin *newSpin( Dialog *parent, FxU32 id, void *data ) {
    Spin *s = calloc( sizeof( Spin ), 1 );
    
    s->handle = GetDlgItem( parent->handle, id );
    s->id     = id;
    s->data   = data;
    if ( s->handle ) {
        return s;
    } else 
      return 0;
}

void spinSetRange( Spin *s, FxU16 min, FxU16 max ) {
    SendMessage( s->handle,
                 UDM_SETRANGE,
                 0, 
                 MAKELPARAM( max, min ) );
}

void spinSetPos( Spin *s, FxU16 pos ) {
    SendMessage( s->handle,
                 UDM_SETPOS,
                 0,
                 MAKELPARAM( pos, 0 ) );
}

void spinSetBuddy( Spin *s, HWND buddy ) {
    SendMessage( s->handle,
                 UDM_SETBUDDY,
                 (WPARAM)buddy,
                 0 );
}

/*-------------------------------------------------------------------------
  Tree Control Implementation
  -------------------------------------------------------------------------*/
Tree *newTree( Dialog *parent, FxU32 id, Action *action, void *data ) {
    Tree *t = calloc( sizeof( Tree ), 1 );
    
    t->handle = GetDlgItem( parent->handle, id );
    t->id     = id;
    t->data   = data;
    if ( t->handle ) {
        // Add To Message Table 
        parent->table.entries[parent->table.numControls].id     = (FxU16)id;
        parent->table.entries[parent->table.numControls].handle = t->handle;
        parent->table.entries[parent->table.numControls].action = action;
        parent->table.entries[parent->table.numControls].data   = data;
        parent->table.numControls++;
        return t;
    } else 
      return 0;
}

TreeItem *treeAddItem( Tree *tree, TreeItem *parent, char *text, DWORD data ) {
    TV_INSERTSTRUCT item;
    TreeItem *t = calloc( sizeof( TreeItem ), 1 );

    item.hParent      = (parent)?parent->handle:NULL;
    item.hInsertAfter = TVI_LAST;
    item.item.mask    = TVIF_TEXT|TVIF_PARAM;
    item.item.pszText = text;
    item.item.lParam  = (LPARAM)t;
    t->handle = (HTREEITEM)SendMessage( tree->handle,
                                        TVM_INSERTITEM,
                                        (WPARAM)0,
                                        (LPARAM)&item );
    if ( !t->handle ) {
        MessageBox( NULL, "Couldn't insert item.\n", "Error", MB_OK|MB_ICONERROR );
        return 0;
    }
    t->tree = tree;
    t->data = data;
    return t;
}

void treeRemoveItem( Tree *tree, TreeItem *item ) {
    SendMessage( tree->handle,
                 TVM_DELETEITEM,
                 (WPARAM)0,
                 (LPARAM)item->handle );
    free( item );
    return;
}

TreeItem *treeGetSelected( Tree *tree ) {
    TreeItem *i;
    HTREEITEM hTI;

    hTI = (HTREEITEM)SendMessage( tree->handle,
                                  TVM_GETNEXTITEM,
                                  (WPARAM)TVGN_CARET,
                                  (LPARAM)0 );
    if ( hTI ) {
        TV_ITEM item;
        item.mask  = TVIF_PARAM|TVIF_HANDLE;
        item.hItem = hTI;
        SendMessage( tree->handle,
                     TVM_GETITEM,
                     (WPARAM)0,
                     (LPARAM)&item );
        i = (TreeItem*)item.lParam;
    } else {
        i = 0;
    }
    return i;
}

DWORD treeItemGetData( TreeItem *item ) {
    return item->data;
}

void treeItemGetText( TreeItem *i, char *buffer, FxU32 size ) {
    TV_ITEM item;
    item.mask       = TVIF_HANDLE|TVIF_TEXT;
    item.hItem      = i->handle;
    item.pszText    = buffer;
    item.cchTextMax = size;
    SendMessage( i->tree->handle,
                 TVM_GETITEM,
                 (WPARAM)0,
                 (LPARAM)&item );
    return;
}


/*-------------------------------------------------------------------------
  Menu Item Implementation
  -------------------------------------------------------------------------*/
MenuItem *newMenuItem( Dialog *parent, FxU32 id, Action *action, void *data ) {
    // Fill In Data Structure
    MenuItem *m = calloc( sizeof( MenuItem ), 1 );
    m->id       = id;
    m->data     = data;

    // Add To Message Table 
    parent->table.entries[parent->table.numControls].id     = (FxU16)id;
    parent->table.entries[parent->table.numControls].handle = 0;
    parent->table.entries[parent->table.numControls].action = action;
    parent->table.entries[parent->table.numControls].data   = data;
    parent->table.numControls++;
    return m;
}


/*-------------------------------------------------------------------------
  Dummy Message Processor
  -------------------------------------------------------------------------*/
void doNothing( WPARAM wParam, LPARAM lParam, void *data ) {
    FXUNUSED( wParam );
    FXUNUSED( lParam );
    FXUNUSED( data );
    return;
}

/*-------------------------------------------------------------------------
  Misc Useful Windows Commands 
  -------------------------------------------------------------------------*/
BOOL openFileDialog( HWND parent, char *filter, DWORD flags, char *startDir, char *file, FxU32 fileLen ) {
    OPENFILENAME ofn;

    ofn.lStructSize       = sizeof( ofn );
    ofn.hwndOwner         = parent;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = filter;
    ofn.lpstrCustomFilter = 0;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 0;
    file[0]               = 0;
    ofn.lpstrFile         = file;
    ofn.nMaxFile          = fileLen;
    ofn.lpstrFileTitle    = 0;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = startDir;
    ofn.lpstrTitle        = "Open File";
    ofn.Flags             = flags;
    ofn.lpstrDefExt       = 0;

    return GetOpenFileName( &ofn );
}

#endif
