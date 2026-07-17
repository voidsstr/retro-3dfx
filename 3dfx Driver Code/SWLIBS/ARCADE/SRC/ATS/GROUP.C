
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
** $Date: 10/11/00 7:34:33 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

#define GROUP_INC 10

static AtsType group_type;
AtsType *_ats_group_type = &group_type;
static const FxU32 _binaryRevision = 1;

/*-------------------------------------------------------------------
  Function: AtsNodeAddParent
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Add a parent to nodes list of parents
  Arguments:
    child - Node being added to a group
    group - the group node
  Return:
    Nothing
  -------------------------------------------------------------------*/

static void atsNodeAddParent(AtsNode *child, AtsNode *group) {
    AtsNode **n = (AtsNode **)child->parents.data;

    if ( child->parents.cur_size >= child->parents.max_size ) {
        child->parents.max_size += GROUP_INC;
        n = (AtsNode **)atuMemRealloc(n, 
                                   sizeof(AtsNode *)*child->parents.max_size);

        if ( n == NULL ) {
            atuError(FXTRUE, "Could not extend parent list\n");
        }

        child->parents.data = (char *)n;
    }
    n[child->parents.cur_size++] = group;
}

/*-------------------------------------------------------------------
  Function: AtsNodeRemoveParent
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Remove a parent from nodes list of parents
  Arguments:
    child - Node being removed from a group
    group - the group node
  Return:
    Nothing
  -------------------------------------------------------------------*/

static FxBool atsNodeRemoveParent(AtsNode *child, AtsNode *group) {
    AtsNode **n = (AtsNode **)child->parents.data;
    int i, j;

    for ( i = 0; i < child->parents.cur_size; i++ ) {
        if ( n[i] == group ) {
            for ( j = i; j < child->parents.cur_size-1; j++ )
                n[j] = n[j+1];
            child->parents.cur_size--;
            return FXTRUE;
        }
    }

    atuError(FXTRUE, 
             "atsNodeRemoveParent: Internal error could not find parent\n");
    return FXFALSE; 
}

/*-------------------------------------------------------------------
  Function: atsGroupNew
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Allocate a new group node
  Arguments:
    None
  Return:
    pointer to new group node
  -------------------------------------------------------------------*/

AtsNode* _atsGroupNew(char *where, FxU32 line) {
    return _atsNew(_ats_group_type, where, line);
}

/*-------------------------------------------------------------------
  Function: atsGroupGetType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns a pointer to the group's type
  Arguments:
    None
  Return:
    pointer to groups type 
  -------------------------------------------------------------------*/

AtsType* atsGroupGetType(void) {
    return (AtsType *)_ats_group_type;
}

/*-------------------------------------------------------------------
  Function: atsGroupFindChild
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Determine if a node is a child of a group
  Arguments:
    group - the group 
    n     - node to check for
  Return:
    index of child in group, -1 if not found
  -------------------------------------------------------------------*/

int atsGroupFindChild(const AtsNode* g, AtsNode *n) {
    int i;
    AtsGroup *group ;

    VALIDATE_GROUP(group, g, "atsGroupFindChild");

    for ( i = 0; i < group->num_children; i++ )
        if ( group->children[i] == n )
            return i;

    return -1;
}

/*-------------------------------------------------------------------
  Function: atsGroupExtend
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Ensure that their is at least enough space to add n 
    additional children
  Arguments:
    group - the group 
    n     - amount to extend by
  Return:
    Nothing
  -------------------------------------------------------------------*/

static void _atsGroupExtend(AtsGroup *group, int n) {
    if ( group->num_children+n >= group->max_children ) {
        group->max_children += n+GROUP_INC;
        group->children = atuMemRealloc(group->children,
                                  group->max_children*sizeof(AtsNode *));
        assert(group->children);
    } 
}

/*-------------------------------------------------------------------
  Function: atsGroupAddChild
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Add a child to the specified group
  Arguments:
    group - the group 
    child - the child to be added
  Return:
    FXTRUE success
    FXFALSE failure
  -------------------------------------------------------------------*/

FxBool atsGroupAddChild(AtsNode* g, AtsNode *child) {
    AtsGroup *group ;

    VALIDATE_GROUP(group, g, "atsGroupAddChild");

    _atsGroupExtend(group, 1);

    group->children[group->num_children++] = child;

    atsRef(child);

    atsNodeAddParent(child, g);

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsGroupInsertChild
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Insert a child into the specified group at the specified location
  Arguments:
    group - the group 
    index - where to insert child
    child - the child to be added
  Return:
    FXTRUE success
    FXFALSE failure
  -------------------------------------------------------------------*/

FxBool atsGroupInsertChild(AtsNode* g, int index, AtsNode *child) {
    int nodes_to_move;
    AtsNode **n;
    AtsGroup *group ;

    VALIDATE_GROUP(group, g, "atsGroupInsertChild");

    if ( ( index < 0 ) || ( index >= group->num_children )) {
        return FXFALSE;
    }

    _atsGroupExtend(group, 1);

    nodes_to_move = group->num_children-index;
    n = group->children+group->num_children;

    while ( nodes_to_move-- > 0 ) {
        *n = *(n-1);
		n--;
    }

    group->children[index] = child;

    group->num_children++;

    atsRef(child);
    
    atsNodeAddParent(child, g);

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsGroupRemoveChild
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Remove a child from the group
  Arguments:
    group - the group 
    child - the child to remove
  Return:
    FXTRUE success
    FXFALSE failure
  -------------------------------------------------------------------*/

FxBool atsGroupRemoveChild(AtsNode* g, AtsNode *child) {
    AtsGroup *group ;
    int nodes_to_move, pos;
    AtsNode **n;

    VALIDATE_GROUP(group, g, "atsGroupRemoveChild");

    pos = atsGroupFindChild(g, child);

    if ( pos < 0 )
        return FXFALSE;

    atsUnrefDelete(group->children[pos]);

    nodes_to_move = group->num_children-pos-1;
    n = group->children+pos;

    while ( nodes_to_move-- > 0 ) {
        *n = *(n+1);
		n++;
    }

    group->num_children--;

    atsNodeRemoveParent(child, g);
    
    return FXTRUE; 
}

/*-------------------------------------------------------------------
  Function: atsGroupFindChildIndexByName
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    return the index of a child with specified name, else return -1
  Arguments:
    group      - the group 
    child_name - the childs name 
  Return:
    index of child if present else -1
  -------------------------------------------------------------------*/

int atsGroupFindChildIndexByName(AtsNode* g, const char *child_name) {
    AtsGroup *group ;
    AtsNode **children ;
    int i;

    VALIDATE_GROUP(group, g, "atsGroupFindChildIndexByName");

    children = (AtsNode **)(group->children);

    for ( i = 0; i < group->num_children; i++ ) {
        if ( strcmp(children[i]->name, child_name) == 0 )
            return i;
    }

    return -1;
}

/*-------------------------------------------------------------------
  Function: atsGroupReplaceChild
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Replace a specified with a new child
  Arguments:
    group - the group 
    oldn  - child to replace
    newn  - the child to insert
  Return:
    FXTRUE success
    FXFALSE failure
  -------------------------------------------------------------------*/

FxBool atsGroupReplaceChild(AtsNode* g, AtsNode *oldn, AtsNode *newn) {
    int pos;
    AtsGroup *group ;

    VALIDATE_GROUP(group, g, "atsGroupReplaceChild");

    /* if the old and new nodes are the same, do nothing */

    if (oldn == newn )
        return FXTRUE;

    pos = atsGroupFindChild(g, oldn);

    if ( pos < 0 )
       return FXFALSE;

    atsUnrefDelete(group->children[pos]);
  
    atsRef(newn);

    group->children[pos] = newn;

    atsNodeRemoveParent(oldn, g);

    atsNodeAddParent(newn, g);

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsGroupGetChild
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return a specified child in a group
  Arguments:
    group - the group 
    index - which child to return
  Return:
    FXTRUE success
    FXFALSE failure
  -------------------------------------------------------------------*/

AtsNode* atsGroupGetChild(const AtsNode* g, int index) {
    AtsGroup *group ;

    VALIDATE_GROUP(group, g, "atsGroupGetChild");

    if ( ( index < 0 ) || ( index >= group->num_children )) {
        return NULL;
    }

    return group->children[index];
}

/*-------------------------------------------------------------------
  Function: atsGroupGetSize
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the number of children in a group
  Arguments:
    group - the group 
  Return:
    Number of children in group
  -------------------------------------------------------------------*/

int atsGroupGetSize(const AtsNode* g) {
    AtsGroup *group ;

    VALIDATE_GROUP(group, g, "atsGroupGetSize");

    return group->num_children;
}

/*-------------------------------------------------------------------
  Function: atsGroupFlatten
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Remove any unneccesary heirarchy in a group
  Arguments:
    group - the group 
    mode  - how to flatten group
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsGroupFlatten(AtsNode* node, FxU32 mode) {
    FXUNUSED(node);
    FXUNUSED(mode);

    printf("atsGroupFlatten not implemented\n");
}

static void GroupInit(AtsObject *node)
{
    AtsGroup *group ;

    VALIDATE_GROUP(group, node, "GroupInit");

    ATS_PARENT_CALL(_ats_group_type, Init)(node);

    group->num_children = 0;
    group->max_children = 0;
    group->children = NULL;
}

static AtsNode *GroupClone(AtsObject* node, FxU32 mode) {
    AtsGroup *gsrc, *gdst ;
    AtsNode *ndst;
    int i;

    VALIDATE_GROUP(gsrc, node, "GroupClone");

    ndst = ATS_PARENT_CALL(_ats_group_type, Clone)(node, mode);

    assert(ndst);

    gdst = (AtsGroup *)ndst;

    gdst->num_children = 0;
    gdst->max_children = 0;
    gdst->children = NULL;

    _atsGroupExtend(gdst, gsrc->num_children-1);

    switch ( mode ) {
    case ATS_CLONE_OBJECT:    /* clone just this object */
        for ( i = 0; i < gsrc->num_children; i++ )
            atsGroupAddChild(ndst, gsrc->children[i]);
        break;
    case ATS_CLONE_HIERARCHY: /* clone only the hierarchy */
    case ATS_CLONE_ALL:       /* clone the hierarchy and geometry */
        for ( i = 0; i < gsrc->num_children; i++ )
            atsGroupAddChild(ndst, atsClone(gsrc->children[i], mode));
        break;
    }

    return ndst;
}

static FxU32 GroupSetMask(AtsNode *node, FxU32 mode, FxU32 mask) {
    AtsGroup *group ;
    FxU32 m = 0;
    int i;

    VALIDATE_GROUP(group, node, "GroupSetMask");

    ATS_PARENT_CALL(_ats_group_type, SetMask)(node, mode, mask);

    /* set masks on children */

    if ( ( mode & ( ATM_TRAV_DESCEND | ATM_TRAV_DESCEND_SET_FROM_CHILD ) ) ) {
        for ( i = 0; i < group->num_children; i++ )
            m |= ATS_NODE_CALL(group->children[i], SetMask)(group->children[i], 
                               mode, mask);
    }

    if ( mode & ATM_TRAV_DESCEND_SET_FROM_CHILD ) {
        node->mask = atsMaskCombine(node->mask, m, mode);
    }
    
    return node->mask ;
}

static void GroupDelete(AtsObject *node) {
    AtsGroup *group ;
    int i;

    VALIDATE_GROUP(group, node, "GroupDelete");

    /* decrement reference count on each of groups children */

    for ( i = 0; i < group->num_children; i++ )
        atsUnrefDelete(group->children[i]);

    atuMemFree(group->children);

    ATS_PARENT_CALL(_ats_group_type, Delete)(node);
}

static FxBool GroupPrint(const AtsObject* node, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    int i;
    AtsGroup *group ;

    VALIDATE_GROUP(group, node, "GroupPrint");

    ATS_PARENT_CALL(_ats_group_type, Print)(node, stream, indent, verbose);

    for ( i = 0; i < group->num_children; i++ )
        atsPrint(group->children[i], stream, indent+4, verbose);

    return FXTRUE;
}

static FxBool GroupLoad(AtsObject *obj, FILE *stream) {
    AtsGroup *group ;
    FxU16 num_children;
    void  **id_list;
    FxU32 version;

    VALIDATE_GROUP(group, obj, "GroupLoad");

    if (!ATS_PARENT_CALL(_ats_group_type, Load)(obj, stream))
        return FXFALSE;
    
    CHECK( atuRead32( &version, 1, stream ));

    CHECK( version == _binaryRevision );

    CHECK( atuRead16( &num_children, 1, stream ));

    group->num_children = num_children;

    if ( group->num_children > 0 ) {
        if ((id_list = (void **)atuMemMalloc(sizeof(void *)*num_children)) == NULL )
            return FXFALSE;

        CHECK( atuRead32( id_list, num_children, stream ));

        group->children = (AtsNode **)id_list;
    }

    return FXTRUE;
}

static FxBool GroupFixup(AtsObject *obj) {
    AtsGroup *group ;
    FxU16 num_children;
    void **id_list;
    int i;
    AtsNode *child;

    VALIDATE_GROUP(group, obj, "GroupFixup");

    if (!ATS_PARENT_CALL(_ats_group_type, Fixup)(obj))
        return FXFALSE;

    if ( group->num_children == 0 ) 
        return FXTRUE;

    id_list = (void **)group->children;
    group->children = NULL;
    num_children = group->num_children;
    group->num_children = 0;

    for ( i = 0; i < num_children; i++ ) {
        if ((child = atsIDToPointer(id_list[i])) == NULL )
            return FXFALSE;
        if (!atsGroupAddChild(obj, child))
            return FXFALSE;
    }

    atuMemFree(id_list);

    return FXTRUE;
}

static FxBool GroupStore(AtsObject *obj, FILE *stream) {
    AtsGroup *group ;
    void *id;
    int i;

    VALIDATE_GROUP(group, obj, "GroupStore");

    if (!ATS_PARENT_CALL(_ats_group_type, Store)(obj, stream))
        return FXFALSE;
    
    CHECK( atuWrite32( &_binaryRevision, 1, stream ));

    CHECK( atuWrite16( &group->num_children, 1, stream ));

    for ( i = 0; i < group->num_children; i++ ) {
        id = atsPointerToID( group->children[i] );
        CHECK( atuWrite32( &id, 1, stream ));
    }

    return FXTRUE;
}

static void GroupEnumerateReferences(AtsObject *obj) {
    AtsGroup *group ;
    int i;

    VALIDATE_GROUP(group, obj, "GroupStore");

    ATS_PARENT_CALL(_ats_group_type, EnumerateReferences)(obj);

    for ( i = 0; i < group->num_children; i++ ) {
        ATS_NODE_CALL(group->children[i], EnumerateReferences)(group->children[i]);
    }
}

static FxBool GroupIsectSphere(const AtsNode *node, const AtmSphere *bsphere) {
    AtsGroup *group ;
    int i;
    FxBool status;

    VALIDATE_GROUP(group, node, "GroupIsectSphere");

    for ( i = 0; i < group->num_children; i++ ) {
        status = ATS_NODE_CALL(group->children[i], IsectSphere)(
                                     group->children[i], bsphere);

        /* return as soon as we have an intersection */

        if (status)
            return FXTRUE;
    }

    return FXFALSE;
}

static void GroupIsectSeg(const AtsNode *node, AtmIsect *isect) {
    AtsGroup *group ;
    int i;

    VALIDATE_GROUP(group, node, "GroupIsectSeg");

    for ( i = 0; i < group->num_children; i++ ) {
        ATS_NODE_CALL(group->children[i], IsectSeg)(
                                     group->children[i], isect);
    }
}

static FxBool GroupIsectNode(const AtsNode *node, const AtsNode *other) {
    AtsGroup *group ;
    int i;
    FxBool status;

    VALIDATE_GROUP(group, node, "GroupIsectNode");

    for ( i = 0; i < group->num_children; i++ ) {
        status = ATS_NODE_CALL(group->children[i], IsectNode)(
                                      group->children[i], other);
        if ( status )
            return FXTRUE;
    }

    return FXFALSE;
}

static void GroupDraw(AtsObject *node, FxU32 mask) {
    AtsGroup *group ;

    int i;

    VALIDATE_GROUP(group, node, "GroupDraw");
 
    if ( ( group->node.mask & mask ) == 0 )
        return;

    if ( atsNodeVisible(node) == ATM_IS_FALSE )
        return;

    for ( i = 0; i < group->num_children; i++ ) {
        ATS_NODE_CALL(group->children[i], Draw)(group->children[i], mask);
    }
}

static FxU32 GroupTraverseNodes(AtsObject* node, AtsTravFunc func, void *data) {
    AtsGroup *group ;
    int i;
    FxU32 status = 0;

    VALIDATE_GROUP(group, node, "GroupTraverseNodes");

    status = ATS_PARENT_CALL(_ats_group_type, Traverse)(node, func, data);

    switch ( status ) {
    case  ATM_TRAV_PRUNE:
        return ATM_TRAV_CONT;

    case ATM_TRAV_CONT:
        break;

    case ATM_TRAV_TERM:
        return ATM_TRAV_TERM;
    }

    for ( i = 0; i < group->num_children; i++ ) {
        status = ATS_NODE_CALL(group->children[i], Traverse)
                                             (group->children[i], func, data);
        switch ( status ) {
        case  ATM_TRAV_PRUNE:
            return ATM_TRAV_CONT;

        case ATM_TRAV_CONT:
            break;

        case ATM_TRAV_TERM:
            return ATM_TRAV_TERM;
        }
    }

    return status;
}

static void GroupTraverseVertices(AtsObject* node, AtsTravVertexFunc func, 
                              void *data) {
    AtsGroup *group ;
    int i;

    VALIDATE_GROUP(group, node, "GroupTraverseVertices");

    for ( i = 0; i < group->num_children; i++ ) {
        ATS_NODE_CALL(group->children[i], TraverseVertices)(group->children[i], 
                      func, data);
    }
}

void atsGroupNewType(AtsType *gt) {
    atsNodeNewType(gt);

    /* set default methods */
           
    gt->Init = GroupInit;
    gt->Clone = GroupClone;
    gt->SetMask = GroupSetMask;
    gt->Delete = GroupDelete;
    gt->Print = GroupPrint;
    gt->Load = GroupLoad;
    gt->Fixup = GroupFixup;
    gt->Store = GroupStore;
    gt->EnumerateReferences = GroupEnumerateReferences;
    gt->Draw = GroupDraw;
    gt->Traverse = GroupTraverseNodes;
    gt->TraverseVertices = GroupTraverseVertices;
    gt->IsectNode = GroupIsectNode;
    gt->IsectSphere = GroupIsectSphere;
    gt->IsectSeg = GroupIsectSeg;

    return ;
}

void _atsGroupInitClass(void)
{
    atsGroupNewType(_ats_group_type);
    ((AtsType *)_ats_group_type)->name = "Group";
    ((AtsType *)_ats_group_type)->size = sizeof(AtsGroup);
    ((AtsType *)_ats_group_type)->parent = (AtsType *)_ats_node_type;
}
