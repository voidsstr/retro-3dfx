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
** $Date: 10/11/00 7:30:38 PM$ 
**
*/


#ifndef _VIEW_H_
#define _VIEW_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CT_TREEVIEW 10
#define CT_GRAPHICSVIEW 20

#define     HINST(x)    (HINSTANCE)GetWindowLong(x, GWL_HINSTANCE)
#define     PAPA( x)    GetParent( x)
#define     MENU( x)    GetMenu( x)
#define     CTRL( x, y) GetDlgItem( x, y)
#define     SYS( x)     GetSystemMetrics( x)

typedef struct _editFile EditFile;

typedef struct _editFile {
    char *fileName;
    AtsObject *obj;
    EditFile *next;
} EditFile;

extern EditFile *fileList;

FxBool structureViewInit(void);

AtmCMesh *buildCMesh(AtsNode *n);
void *materialEdit( void *object, HWND parent );
void *triSetEdit( void *object, HWND parent );
void *nodeEdit( void *object, HWND parent );
char *GetFileName(char *prompt);
void editFile(char *fileName);
BOOL WINAPI AboutProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK structureViewProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam);

void editFile(char *fileName);

FxBool ReadWindowPos( LPRECT prc, HINSTANCE hInstance);

FxBool WriteWindowPos( LPRECT prc, HINSTANCE hInstance);

BOOL WINAPI LoadPathDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void editItem(HWND hWnd);

void viewNode( AtsObject *object );

void viewTriSet( AtsObject *object );

char * fileLocate(const char *filename, char *full_path);

BOOL checkExitApp();

typedef struct {
    FxU32 val;
    char *name;
} EnumEntry;

void setEnum( HWND dlg, FxU32 id, EnumEntry *edef, FxU32 cur);
void setBool( HWND dlg, FxU32 id, FxBool cur);
void setText( HWND dlg, FxU32 id, const char *txt );
void setFloat( HWND dlg, FxU32 id, float val );
void setFxU32( HWND dlg, FxU32 id, FxU32 val );
void setInt( HWND dlg, FxU32 id, int val );
char *enumFindString(FxU32 val, EnumEntry *edef);

extern char *modeString ;
void SetTitle(char *mode);

BOOL centerWindow( HWND hWnd );

extern HWND hWndTree;

extern FxU32 gCullMode ;

extern AtsNode *model ;

extern AtmCMesh *cmesh;
 
extern FxBool quit;

void spawnAndWait(char *appName);

HBITMAP imageToDIB(HWND hWnd, AtrImg *img);

#ifdef __cplusplus
}
#endif
#endif
