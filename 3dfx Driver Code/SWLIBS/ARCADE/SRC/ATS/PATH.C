
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
** $Date: 10/11/00 7:34:38 PM$ 
**
**
*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <atscenep.h>

#define PATH_INC 10

/*-------------------------------------------------------------------
  Function: atsPathNew
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a new path
  Arguments:
    None
  Return:
    Nothing
  -------------------------------------------------------------------*/

AtsPath *_atsPathNew(char *where, FxU32 line) {
    AtsPath *path;

    path = (AtsPath *)_atuMemRealloc(NULL, sizeof(AtsPath), where, line);

    if ( path == NULL )
        atuError(FXTRUE, "atsPathNew: out of memory\n");

    return path;
}

/*-------------------------------------------------------------------
  Function: atsPathExtend
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    extend a path to make room for a specified number of additional elements
  Arguments:
    path  - the path
    nelem - the number of nodes to make space for
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsPathExtend(AtsPath *path, int nelem ) {
    if ( ( path->cur_size + nelem) >= path->max_size ) {
        path->max_size += nelem+PATH_INC;
        path->nodes = (AtsNode **)atuMemRealloc(path->nodes,
                                              path->max_size*sizeof(AtsNode *));
        if ( path->nodes == NULL )
            atuError(FXTRUE, "atsPathPush: out of memory\n");
    }
}

/*-------------------------------------------------------------------
  Function: atsPathPushNode
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    push a new node onto the list of nodes
  Arguments:
    path - the path
    node - the node
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsPathPushNode(AtsPath *path, const AtsNode *node) {
    atsPathExtend(path, 1);

    path->nodes[path->cur_size++] = (AtsNode *)node;
}

/*-------------------------------------------------------------------
  Function: atsPopNode
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    remove a node from the list of nodes
  Arguments:
    path - the path
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsPathPopNode(AtsPath *path) {
    if ( path->cur_size > 0 )
        path->cur_size--;
    else atuError(FXTRUE, "atsPathPop: tried to pop empty stack\n");
}

/*-------------------------------------------------------------------
  Function: atsPathCopy
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a new path
  Arguments:
    None
  Return:
    Nothing
  -------------------------------------------------------------------*/

AtsPath *atsPathCopy(AtsPath *src) {
    AtsPath *dst;

    dst = atsPathNew();
    atsPathExtend(dst, src->cur_size);
    dst->cur_size = src->cur_size;
    dst->max_size = src->max_size;
    memcpy(dst->nodes, src->nodes, src->cur_size*sizeof(AtsNode *));

    return dst;
}

/*-------------------------------------------------------------------
  Function: atsPathFree
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    free a path
  Arguments:
    path - path to free
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsPathFree(AtsPath *path) {
    atuMemFree(path->nodes);
    atuMemFree(path);
}
