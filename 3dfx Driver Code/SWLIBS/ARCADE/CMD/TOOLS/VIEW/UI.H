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
** $Date: 10/11/00 7:30:36 PM$ 
**
*/


#ifndef _UI_H_
#define _UI_H_


#ifdef __cplusplus
extern "C" {
#endif

void createControls( Dialog *dlg, char *filename );

typedef struct _editFile EditFile;

typedef struct _editFile {
    char *fileName;
    Dialog *dlg;
    AtsObject *obj;
    MenuItem *okMenu;
    MenuItem *editMenu;
    MenuItem *viewMenu;
    Tree     *tree;
    EditFile *next;
} EditFile;

extern EditFile *openFiles ;
#ifdef __cplusplus
}
#endif
#endif
