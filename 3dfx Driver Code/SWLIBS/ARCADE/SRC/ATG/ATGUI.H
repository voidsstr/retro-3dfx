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
** $Date: 10/11/00 7:33:28 PM$ 
**
*/

#ifndef _ATGUI_H_
#define _ATGUI_H_
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CONTROLS 64

#include <3dfx.h>
#include <windows.h>
#include <commctrl.h>

/*----------------------------------------------------------------------------
  DATA DEFINITIONS
  ----------------------------------------------------------------------------*/
typedef struct {
    HWND  handle;
    FxU32 id;
    void  *data;
} Button;

typedef struct {
    FxU32 id;
    void  *data;
} MenuItem;

typedef struct {
    HWND  handle;
    FxU32 id;
    void  *data;
} Edit;

typedef struct {
    HWND  handle;
    FxU32 id;
    void  *data;
} ListBox;

typedef struct {
    HWND  handle;
    FxU32 id;
    void  *data;
} Spin;

typedef struct {
    HWND  handle;
    FxU32 id;
    void  *data;
} Slider;

typedef struct {
    HWND  handle;
    FxU32 id;
    void  *data;
} Tree;

typedef struct {
    HTREEITEM handle;
    Tree      *tree;
    DWORD     data;
} TreeItem;

typedef struct {
    HWND  handle;
    FxU32 id;
    void  *data;

    FxU16 min;
    FxU16 max;
    FxU16 pos;
} Progress;

typedef void Action( WPARAM wParam, LPARAM lParam, void *data );

typedef struct {
    FxU16  id;
    HWND   handle;
    Action *action;
    void   *data;
} ControlTableEntry;

typedef struct {
    FxU32             numControls;
    ControlTableEntry entries[MAX_CONTROLS];
} ControlTable;

typedef struct {
    HWND  handle;
    FxU32 *done;
    HMENU popup;
    void  *data;

    ControlTable table;
} Dialog;


/*----------------------------------------------------------------------------
  FUNCTION PROTOTYPES 
  ----------------------------------------------------------------------------*/
typedef void SetupFunction( Dialog *dlg );

void     newModalDialog( HWND parent, FxU32 id, HINSTANCE module, 
                         SetupFunction *setup, BOOL *done, void *data ); 

Dialog   *newDialog( HWND parent, FxU32 id, HINSTANCE module, BOOL *done, 
                     void *data );

void     deleteDialog( Dialog *dlg );

Button   *newButton( Dialog *parent, FxU32 id, Action *action, void *data );
Edit     *newEdit( Dialog *parent, FxU32 id, Action *action, void *data );
ListBox  *newListBox( Dialog *parent, FxU32 id, void *data );
Progress *newProgress( Dialog *parent, FxU32 id, void *data );
Spin     *newSpin( Dialog *parent, FxU32 id, void *data );
Slider   *newSlider( Dialog *parent, FxU32 id, Action *action, void *data );
Tree     *newTree( Dialog *parent, FxU32 id, Action *action, void *data );
MenuItem *newMenuItem( Dialog *parent, FxU32 id, Action *action, void *data );

void dialogShow( Dialog *dlg );
void dialogDone( Dialog *dlg, BOOL done );
void dialogSetPopup( Dialog *dlg, HINSTANCE module, FxU32 id );

void editSetText( Edit *edit, const char *text );
void editGetText( Edit *edit, char *text, FxU32 size );

void progressSetRange( Progress *p, FxU16 min, FxU16 max );
void progressSetPos( Progress *p, FxU16 pos );

int  listBoxAddString( ListBox *l, const char *string );
void listBoxRemoveString( ListBox *l, FxU32 index );
void listBoxReset( ListBox *l );
void listBoxPaint( ListBox *l );

TreeItem  *treeAddItem( Tree *tree, TreeItem *parent, char *text, DWORD data );
TreeItem  *treeGetSelected( Tree *tree );
void      treeRemoveItem( Tree *tree, TreeItem *item );

DWORD     treeItemGetData( TreeItem *item );
void      treeItemGetText( TreeItem *item, char *buffer, FxU32 size );

void spinSetRange( Spin *s, FxU16 min, FxU16 max );
void spinSetPos( Spin *s, FxU16 pos );
void spinSetBuddy( Spin *s, HWND buddy );

void  sliderSetRange( Slider *s, FxU16 min, FxU16 max );
void  sliderSetPos( Slider *s, FxU16 pos );
FxU16 sliderGetPos( Slider *s );


void doNothing( WPARAM lParam, LPARAM wParam, void *data );

BOOL openFileDialog( HWND parent, char *filter, DWORD flags, char *startDir, char *file, FxU32 fileLen );

// Plug in Object Editing DLLS
void *editObject( void *object, const char *dllName, HWND parent );

#ifdef __cplusplus
}
#endif
#endif
