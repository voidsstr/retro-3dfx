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
** $Date: 10/11/00 7:34:36 PM$ 
**
*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

AtsType node_type;
AtsType *_ats_node_type = &node_type;

static AtsPath *current_path = NULL;

/*-------------------------------------------------------------------
  Function: atsNodeName
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Specify the name of a node
  Arguments:
    node - Node to be named
    name - new name of this node
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsNodeName(AtsNode* n, const char *name) {
    AtsNode *node = (AtsNode *)n;
    int len = strlen(name);

    VALIDATE_NODE(node, n, "atsNodeName");

    if ( node->name )
        atuMemFree(node->name);

    node->name = atuMemMalloc(len+1);
    assert(node->name);
    strcpy(node->name, name);
}

/*-------------------------------------------------------------------
  Function: atsNodeGetName
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the name of a node
  Arguments:
    node  - Node whose name is being inquired
  Return:
    pointer to nodes name
  -------------------------------------------------------------------*/

const char* atsNodeGetName(const AtsNode* n) {
    AtsNode *node ;

    VALIDATE_NODE(node, n, "atsNodeGetName");

    return node->name;
}

/*-------------------------------------------------------------------
  Function: atsNodeGetNumParents
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the number of parents a node has
  Arguments:
    node  - Node whose parents are being queried
  Return:
    Number of parents a node has
  -------------------------------------------------------------------*/

int atsNodeGetNumParents(const AtsNode* n) {
    AtsNode *node ;

    VALIDATE_NODE(node, n, "atsNodeGetNumParents");

    return node->parents.cur_size;
}

/*-------------------------------------------------------------------
  Function: atsNodeGetParent
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return one of the nodes parents
  Arguments:
    node  - Node whose parents are being queried
    i     - which parent to return
  Return:
    nodes parent if it exists else NULL
  -------------------------------------------------------------------*/

AtsNode *atsNodeGetParent(const AtsNode* n, FxU32 i) {
    AtsNode *node, **p ;

    VALIDATE_NODE(node, n, "atsNodeGetParent");

    p = (AtsNode **)node->parents.data;

    if ( i < node->parents.cur_size )
        return p[i];
    else return NULL;
}

/*-------------------------------------------------------------------
  Function: atsNodeMask
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Specify the mask for this node
  Arguments:
    node  - Node to be named
    mode  - how to set mask
    mask  - new mask for this node
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsNodeMask(AtsNode* n, FxU32 mode, FxU32 mask) {
    AtsNode *node ;

    VALIDATE_NODE(node, n, "atsNodeMask");

    ATS_NODE_CALL(node, SetMask)(node, mode, mask);
}

/*-------------------------------------------------------------------
  Function: atsNodeGetMask
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the nodes mask
  Arguments:
    node  - Node whose mask is being inquired
  Return:
    nodes mask
  -------------------------------------------------------------------*/

FxU32 atsNodeGetMask(AtsNode *n) {
    AtsNode *node ;

    VALIDATE_NODE(node, n, "atsNodeGetMask");

    return node->mask;
}

/*-------------------------------------------------------------------
  Function: atsNodeBSphere
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Specify the bounding volume for this node
  Arguments:
    node    - Node to be named
    bsphere - new bounding volume for this node
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsNodeBSphere(AtsNode* n, AtmSphere *bsphere) {
    AtsNode *node ;

    VALIDATE_NODE(node, n, "atsNodeBSphere");

    node->bsphere = *bsphere;
}

/*-------------------------------------------------------------------
  Function: atsNodeGetBSphere
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the nodes bounding volume
  Arguments:
    node    - Node whose mask is being inquired
    bsphere - bounding volume for this node
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsNodeGetBSphere(AtsNode *n, AtmSphere *bsphere) {
    AtsNode *node ;

    VALIDATE_NODE(node, n, "NodeGetBSphere");

    *bsphere = node->bsphere;
}

/*-------------------------------------------------------------------
  Function: AtsNodeCollide
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Determine if two nodes collide
  Arguments:
    node1 - Node being checked for collision
    node2 - Node being checked against
  Return:
    Intersection status
  TBD: need to test against child spheres
       need to use nodes get bounding sphere function in case its not
       currently correct (due to caching)
  -------------------------------------------------------------------*/

FxBool atsNodeCollide(AtsNode* node1, AtsNode *node2) {
    AtrXform t;
    FxBool status;

    atrXformSetIdentity( &t );
    atsPushXform( &t );

    status = ATS_NODE_CALL(node1, IsectNode)(node1, node2);

    atsPopXform();

    return status;
}

/*-------------------------------------------------------------------
  Function: atsBoxContainsNode
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Determine if a box and a node intersect
  Arguments:
    box   - Box being checked for intersection
    node  - Node being checked against
    mask  - Only check against nodes which match specified mask
  Return:
    Intersection status
  -------------------------------------------------------------------*/

FxU32 atsBoxContainsNode(const AtmBox *box, const AtsNode *node) {
    return atmSphereContainsBox(&node->bsphere, box);
}

/*-------------------------------------------------------------------
  Function: atsSphereContainsNode
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Determine if sphere and a node interect
  Arguments:
    sphere - Sphere being checked for intersection
    node   - Node being checked against
  Return:
    Intersection status
  -------------------------------------------------------------------*/

FxBool atsSphereContainsNode(const AtmSphere *sphere, const AtsNode *node) {
    AtrXform identity;
    FxBool status;

    atsPushXform( &identity );
    status = ATS_NODE_CALL(node, IsectSphere)(node, sphere);
    atsPopXform();
    return status;
}

/*-------------------------------------------------------------------
  Function: atsNodeIsectSeg
  Date: 6/2/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Determine if a line segment intersects a node
  Arguments:
    node   - being tested
    isect  - intersection control
  Return:
    FXTRUE if segment intersects node
  -------------------------------------------------------------------*/

FxBool atsNodeIsectSeg(const AtsNode *node, AtmIsect *isect)
{
    AtrXform t;

    isect->hit_seg = isect->seg;

    atrXformSetIdentity( &t );
    atsPushXform( &t );

    ATS_NODE_CALL(node, IsectSeg)(node, isect);

    atsPopXform();

    return (( isect->hit_status & ATM_HIT_VALID ) != 0 );
}

static void NodeInit(AtsObject *n)
{
    AtsNode *node ;

    VALIDATE_NODE(node, n, "NodeInit");

    ATS_PARENT_CALL(_ats_node_type, Init)(node);

    node->name = NULL;
    node->flags = 0;
    node->mask = 0xffffffff;
    node->bsphere.radius = 0.0f;

    /* to avoid circular reference counts we don't want the elements in
       the parent list reference counted so give atsArrayInit an element
       size of 4
    */

    atsArrayInit(&node->parents, 0, sizeof(AtsNode *));
}

static AtsNode *NodeClone(AtsObject* n, FxU32 mode) {
    AtsNode *nsrc, *ndst ;

    VALIDATE_NODE(nsrc, n, "NodeClone");

    ndst = ATS_PARENT_CALL(_ats_node_type, Clone)(n, mode);

    /* Clone function copies all basic data just need to fixup pointers */

    if ( nsrc->name )
        ndst->name = strdup(nsrc->name);

    /* each clone gets a new set of parents */

    atsArrayInit(&ndst->parents, 0, sizeof(AtsNode *));

    return ndst;
}

static FxBool NodeIsectSphere(const AtsNode *node, const AtmSphere *bsphere) {
    AtmSphere dst;
    AtrXform t;

    atsQueryXform( &t );
    atmSphereOrthoXform(&dst, &node->bsphere, t.data);
    return atmSphereContainsSphere(bsphere, &dst);
}

static FxBool NodeIsectNode(const AtsNode *node, const AtsNode *other) {
    AtmSphere dst;
    AtrXform t;

    atsQueryXform( &t );
    atmSphereOrthoXform(&dst, &node->bsphere, t.data);
    return atsSphereContainsNode(&dst, other);
}

FxU32 atsMaskCombine(FxU32 v1, FxU32 v2, FxU32 mode)
{
    switch ( mode & ATM_TRAV_OP ) {
    case ATM_TRAV_OR: 
        return v1 | v2;
    case ATM_TRAV_AND:
        return v1 & v2;
    default:
    case ATM_TRAV_SET:
        return v2;
    }
}

static FxU32 
NodeSetMask(AtsNode *n, FxU32 mode, FxU32 mask) {
    AtsNode *node;

    VALIDATE_NODE(node, n, "NodeSetMask");

    node->mask = atsMaskCombine(node->mask, mask, mode);
    
    return node->mask;
}

static void 
NodeDelete(AtsObject *n) {
    AtsNode *node ;

    VALIDATE_NODE(node, n, "NodeDelete");

    if ( node->name )
       atuMemFree(node->name);

    ATS_PARENT_CALL(_ats_node_type, Delete)(node);
}

static FxBool 
NodePrint(const AtsObject* n, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    AtsNode *node ;

    FXUNUSED(verbose);

    VALIDATE_NODE(node, n, "NodePrint");

    if ( node->name )
        fprintf( stream, "%*sname \"%s\"\n", indent+4, " ", node->name);

    fprintf( stream, "%*smask 0x%lx\n", indent+4, " ", node->mask);

    fprintf( stream, "%*sbounds {\n", indent+4, " ");
    fprintf( stream, "%*smin {%f %f %f}\n", indent+8, " ",
            node->bsphere.center[0], node->bsphere.center[1],
            node->bsphere.center[2]);
    fprintf( stream, "%*sradius %f\n", indent+8, " ", node->bsphere.radius);
    fprintf( stream, "%*s}\n", indent+4, " ");
    
    return FXTRUE;
}

static FxBool NodeLoad(AtsObject *obj, FILE *stream) {
    AtsNode *node ;

    VALIDATE_NODE(node, obj, "NodeLoad");

    CHECK( atuRead32( &node->flags, 1, stream ));

    CHECK( atuRead32( &node->mask, 1, stream ));

    CHECK(atsLoadString(&node->name, stream));

    CHECK( atuRead32( &node->bsphere, sizeof( node->bsphere )>>2, stream ));

    return FXTRUE;
}

static FxBool NodeStore(AtsObject *obj, FILE *stream) {
    AtsNode *node ;

    VALIDATE_NODE(node, obj, "NodeStore");

    CHECK( atuWrite32( &node->flags, 1, stream ));

    CHECK( atuWrite32( &node->mask, 1, stream ));

    CHECK(atsStoreString(node->name, stream));

    CHECK( atuWrite32( &node->bsphere, sizeof( node->bsphere )>>2, stream ));

    return FXTRUE;
}

static void NodeIsectSeg(const AtsNode *node, AtmIsect *isect) {
    float tnear, tfar;
    FxU32 status;
    AtmSphere dst;
    AtrXform t;

    atsPathPushNode(current_path, node);

    atsQueryXform( &t );
    atmSphereOrthoXform(&dst, &node->bsphere, t.data);
    status = atmSphereContainsSeg(&dst, &isect->hit_seg, &tnear, &tfar);
    if ( status & ATM_IS_TRUE ) { /* segment intersects sphere */
        isect->hit_status = ATM_HIT_VALID | ATM_HIT_SEG ;
        isect->hit_seg = isect->seg;
        if ( status & ATM_IS_START_IN ) /* start point within node */
            isect->hit_seg.length = 0.0f;
        else atmSegClip(&isect->hit_seg, &isect->seg, 0.0f, tnear);
        isect->hit_path = atsPathCopy(current_path);
    }

    atsPathPopNode(current_path);
}

extern AtmPolytope *_atsCullVolume ; /* current culling volume */

extern FxU32 _atsCullMode ;

/*-------------------------------------------------------------------
  Function: atsNodeVisible
  Date: 7/18/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Determine if a node is visible based on the current culling mode 
    and the nodes bounding sphere.
  Arguments:
    node   - being tested
  Return:
    Intersection status
  -------------------------------------------------------------------*/

FxU32 atsNodeVisible(AtsNode *node) {
    AtmSphere bsphere;
    extern AtrXform *_atrCurrentXform ;
    FxU32 result;

    /* don't cull nodes with zero radius, e.g. lights etc. */

    if ((( _atsCullMode & ATS_CULL_NODES) == 0 ) || ( node->bsphere.radius == 0.0f )){
        result = ATM_IS_TRUE;
    } else {
        atmSphereOrthoXform(&bsphere, &node->bsphere, _atrCurrentXform->data);

        result = atmPolytopeContainsSphere(_atsCullVolume, &bsphere);
    /* result = atmHalfSpaceContainsSphere(_atsCullVolume->planes, &bsphere); */
    }

#ifdef notdef
    /* printf("visibility result = 0x%lx\n", result); */

    if ( result == ATM_IS_FALSE ) {
        float d = atmDistPointToPoint(bsphere.center, &camMatrix.data[12]) ;
        
        if ( d < bsphere.radius ) {
            printf("bad exclusion dist = %f, radius = %f\n", d, bsphere.radius);
            result = atmHalfSpaceContainsSphere(_atsCullVolume->planes, &bsphere);
        }
    }
#endif

    return result;
}

void atsNodeNewType(AtsType *nt) {
    atsObjectNewType(nt);

    /* set default methods */
           
    nt->Init = NodeInit;
    nt->Clone = NodeClone;
    nt->SetMask = NodeSetMask;
    nt->Delete = NodeDelete;
    nt->Print = NodePrint;
    nt->Load = NodeLoad;
    nt->Store = NodeStore;
    nt->IsectNode = NodeIsectNode;
    nt->IsectSphere = NodeIsectSphere;
    nt->IsectSeg = NodeIsectSeg;

    return ;
}

void _atsNodeInitClass(void)
{
    atsNodeNewType(_ats_node_type);
    ((AtsType *)_ats_node_type)->name = "Node";
    ((AtsType *)_ats_node_type)->size = sizeof(AtsNode);
    ((AtsType *)_ats_node_type)->parent = _ats_object_type;

    current_path = atsPathNew();
}
