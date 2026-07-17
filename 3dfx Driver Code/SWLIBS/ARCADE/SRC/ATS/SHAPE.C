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
** $Date: 10/11/00 7:34:44 PM$ 
**
** TBD: Add in support for sorting objects based on material, transparency etc.
**/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

#define PART_INC 10
static const FxU32 _trisetBinaryRevision = 1;
static const FxU32 _shapeBinaryRevision = 1;
static const FxU32 _cmeshBinaryRevision = 1;

static const char  _triDataType[]     = "atrTriSet";
static const char  _cmeshDataType[]   = "atmCmesh";

static AtsType shape_type;
AtsType *_ats_shape_type = &shape_type;

static AtsType geometry_type;
AtsType *_ats_geometry_type = &geometry_type;

static AtsType triset_type;
AtsType *_ats_triset_type = &triset_type;

static AtsType cmesh_type;
AtsType *_ats_cmesh_type = &cmesh_type;

/*-------------------------------------------------------------------
  Function: atsShapeNew
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a new shape node
  Arguments:
    None
  Return:
    a pointer to the new shape node
  -------------------------------------------------------------------*/

AtsNode* _atsShapeNew(char *where, FxU32 line) {
    return _atsNew(_ats_shape_type, where, line);
}

/*-------------------------------------------------------------------
  Function: atsShapeGetType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns a pointer to the shape's type
  Arguments:
    None
  Return:
    pointer to shapes type 
  -------------------------------------------------------------------*/

AtsType* atsShapeGetType(void) {
    return (AtsType *)_ats_shape_type;
}

static void _atsShapeExtend(AtsShape *shape, int n) {
    if ( shape->num_parts+n > shape->max_parts ) {
        shape->max_parts += n+PART_INC;
        shape->parts = atuMemRealloc(shape->parts,
                                  shape->max_parts*sizeof(AtsShapePart));
        assert(shape->parts);
    } 
}

/*-------------------------------------------------------------------
  Function: atsShapeAddPart
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Add another part to the shape
  Arguments:
    node - the shape
    geometry - new geometry
    material - new material
  Return:
    FXTRUE if successful
    FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool atsShapeAddPart(AtsNode *node, AtsObject *g, AtsObject *material) {
    AtsNode *geometry;
    AtsShape *shape ;
#ifdef AT_DEBUGGING
    const char *func = "ShapeAddGeomtry";
#endif

    VALIDATE_SHAPE(shape, node, func);
  
    /* VALIDATE_GEOMETRY(geometry, g, func); */
    geometry = (AtsNode *)g;

    _atsShapeExtend(shape, 1);

    shape->parts[shape->num_parts].geometry = geometry;
    shape->parts[shape->num_parts].material = material;

    shape->num_parts++;
    atsRef(geometry);
    atsRef(material);

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsShapeGetNumParts
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns a pointer to the shape's type
  Arguments:
    node - the shape
  Return:
    current length of parts list
  -------------------------------------------------------------------*/

int atsShapeGetNumParts(AtsNode *node) {
    AtsShape *shape ;

    VALIDATE_SHAPE(shape, node, "atsShapeGetNumParts");
  
    return shape->num_parts;
}

/*-------------------------------------------------------------------
  Function: atsShapePart
  Date: 7/29/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set the geometry and material associated with a part
  Arguments:
    node     - the shape
    i        - part number reqested
    geometry - the part's geometry
    material - the part's material
  Return:
    FXTRUE   - if parts number was valid
    FXFALSE  - invalid part number
  TBD:
    Need to fix reference counting
  -------------------------------------------------------------------*/

FxBool atsShapePart(AtsNode *node, int index, AtsObject *geometry, 
                       AtsObject *material) {
    AtsShape *shape ;

    VALIDATE_SHAPE(shape, node, "atsShapeGetPart");
  
    if ( ( index < 0 ) || ( index >= shape->num_parts )) {
        return FXFALSE;
    }

    shape->parts[index].geometry = geometry; 
    shape->parts[index].material = material; 

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsShapeGetPart
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns the geometry and material associated with a part
  Arguments:
    node     - the shape
    i        - part number reqested
    geometry - the part's geometry
    material - the part's material
  Return:
    FXTRUE   - if parts number was valid
    FXFALSE  - invalid part number
  -------------------------------------------------------------------*/

FxBool atsShapeGetPart(AtsNode *node, int index, AtsObject **geometry, 
                       AtsObject **material) {
    AtsShape *shape ;

    VALIDATE_SHAPE(shape, node, "atsShapeGetPart");
  
    if ( ( index < 0 ) || ( index >= shape->num_parts )) {
        return FXFALSE;
    }

    *geometry = shape->parts[index].geometry; 
    *material = shape->parts[index].material; 

    return FXTRUE;
}

static void ShapeInit(AtsObject *node)
{
    AtsShape *shape ;

    VALIDATE_SHAPE(shape, node, "ShapeInit");

    ATS_PARENT_CALL(_ats_shape_type, Init)(node);

    shape->num_parts = 0;
    shape->max_parts = 0;
    shape->parts = NULL;
}

static void ShapeDelete(AtsObject *node) {
    AtsShape *shape ;
    int i;

    VALIDATE_SHAPE(shape, node, "ShapeDelete");
 
    /* decrement reference count on each of shapes parts */

    for ( i = 0; i < shape->num_parts; i++ ) {
        atsUnrefDelete(shape->parts[i].geometry);
        atsUnrefDelete(shape->parts[i].material);
    }

    atuMemFree(shape->parts);

    ATS_PARENT_CALL(_ats_shape_type, Delete)(node);
}

/* TBD: need to make meshes and materials into real objects in order
 *      to reference count them 
 */

static AtsNode *ShapeClone(AtsObject* node, FxU32 mode) {
    AtsShape *ssrc, *sdst ;
    AtsNode *ndst;
    int i;

    VALIDATE_SHAPE(ssrc, node, "ShapeClone");

    ndst = ATS_PARENT_CALL(_ats_shape_type, Clone)(node, mode);

    assert(ndst);

    sdst = (AtsShape *)ndst;

    _atsShapeExtend(sdst, ssrc->num_parts-1);

    for ( i = 0; i < ssrc->num_parts; i++ ) {
        sdst->parts[i].geometry = atsClone(ssrc->parts[i].geometry, mode);
        sdst->parts[i].material = ssrc->parts[i].material;
    }

    sdst->num_parts = ssrc->num_parts;

    return ndst;
}

static FxBool ShapePrint(const AtsObject* node, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    AtsShape *shape ;
    int i;

    VALIDATE_SHAPE(shape, node, "ShapePrint");

    ATS_PARENT_CALL(_ats_shape_type, Print)(node, stream, indent, verbose);

    fprintf( stream, "%*s parts {\n", indent, " ");

    for ( i = 0; i < shape->num_parts; i++ ) {
        fprintf( stream, "%*s {\n", indent+4, " ");
        atsPrint(shape->parts[i].geometry, stream, indent+8, verbose);
        atsPrint(shape->parts[i].material, stream, indent+8, verbose);
        fprintf( stream, "%*s }\n", indent+4, " ");
    }

    fprintf( stream, "%*s }\n", indent, " ");

    return FXTRUE;
}

static FxBool ShapeLoad(AtsObject *obj, FILE *stream) {
    AtsShape *shape ;
    void  **id_list;
    FxU32 version;

    VALIDATE_SHAPE(shape, obj, "ShapeLoad");

    if (!ATS_PARENT_CALL(_ats_shape_type, Load)(obj, stream))
        return FXFALSE;
    
    CHECK ( atuRead32( &version, 1, stream ) );

    CHECK ( version == _shapeBinaryRevision );

    CHECK ( atuRead32( &shape->num_parts, 1, stream ) );

    if ( shape->num_parts > 0 ) {

        if ((id_list = (void **)atuMemMalloc(2*sizeof(void *)*shape->num_parts)) == NULL )
            return FXFALSE;

        CHECK ( atuRead32( id_list, 2*shape->num_parts, stream ));

        shape->parts = (AtsShapePart *)id_list;
    }

    return FXTRUE;
}

static FxBool ShapeFixup(AtsObject *obj) {
    AtsShape *shape ;
    int  num_parts;
    void **id_list;
    int i;
    AtsObject   *geo;
    AtsMaterial *mat;

    VALIDATE_SHAPE(shape, obj, "ShapeFixup");

    if (!ATS_PARENT_CALL(_ats_shape_type, Fixup)(obj))
        return FXFALSE;

    if ( shape->num_parts == 0 )
        return FXTRUE;

    id_list = (void **)shape->parts;
    shape->parts = NULL;
    num_parts = shape->num_parts;
    shape->num_parts = 0;

    for ( i = 0; i < num_parts; i++ ) {
        if ((mat = atsIDToPointer(id_list[2*i])) == NULL )
            return FXFALSE;
        if ((geo = atsIDToPointer(id_list[2*i+1])) == NULL )
            return FXFALSE;
        if (!atsShapeAddPart(obj, geo, mat))
            return FXFALSE;
    }

    atuMemFree(id_list);

    return FXTRUE;
}

static FxBool ShapeStore(AtsObject *obj, FILE *stream) {
    AtsShape *shape ;
    void *id;
    int i;

    VALIDATE_SHAPE(shape, obj, "ShapeStore");

    if (!ATS_PARENT_CALL(_ats_shape_type, Store)(obj, stream))
        return FXFALSE;
    
    CHECK ( atuWrite32( &_shapeBinaryRevision, 1, stream ) );

    CHECK ( atuWrite32( &shape->num_parts, 1, stream ) );

    for ( i = 0; i < shape->num_parts; i++ ) {
        id = atsPointerToID( shape->parts[i].material );
        CHECK ( atuWrite32( &id, 1, stream ));
        id = atsPointerToID( shape->parts[i].geometry );
        CHECK ( atuWrite32( &id, 1, stream ));
    }

    return FXTRUE;
}

static void ShapeEnumerateReferences(AtsObject *obj) {
    AtsShape *shape ;
    int i;

    VALIDATE_SHAPE(shape, obj, "ShapeStore");

    ATS_PARENT_CALL(_ats_shape_type, EnumerateReferences)(obj);

    for ( i = 0; i < shape->num_parts; i++ ) {
        ATS_NODE_CALL(shape->parts[i].material, EnumerateReferences)(shape->parts[i].material);
        ATS_NODE_CALL(shape->parts[i].geometry, EnumerateReferences)(shape->parts[i].geometry);
    }
}

static void ShapeDraw(AtsObject *node, FxU32 mask) {
    AtsShape *shape ;
    int i;

    VALIDATE_SHAPE(shape, node, "ShapeDraw");

    if ( ( shape->node.mask & mask ) == 0 )
        return;

    if ( atsNodeVisible(node) == ATM_IS_FALSE )
        return;

    for ( i = 0; i < shape->num_parts; i++ ) {
        (* _atsPushMaterial)( shape->parts[i].material);
        ATS_NODE_CALL(shape->parts[i].geometry, Draw)(shape->parts[i].geometry,
                                                      mask);
        (* _atsPopMaterial ) ( FXFALSE );
    }
}

static void ShapeTraverseVertices(AtsObject* node, AtsTravVertexFunc func, 
                              void *data) {
    AtsShape *shape ;
    int i;

    VALIDATE_SHAPE(shape, node, "ShapeTraverseVertices");

    for ( i = 0; i < shape->num_parts; i++ ) {
        ATS_NODE_CALL(shape->parts[i].geometry, TraverseVertices)
                        (shape->parts[i].geometry, func, data);
    }
}

static FxU32 ShapeTraverseNodes(AtsObject* node, AtsTravFunc func, void *data) {
    AtsShape *shape ;
    int i;
    FxU32 status = 0;

    VALIDATE_SHAPE(shape, node, "ShapeTraverseNodes");

    status = ATS_PARENT_CALL(_ats_shape_type, Traverse)(node, func, data);

    switch ( status ) {
    case  ATM_TRAV_PRUNE:
        return ATM_TRAV_CONT;

    case ATM_TRAV_CONT:
        break;

    case ATM_TRAV_TERM:
        return ATM_TRAV_TERM;
    }

    for ( i = 0; i < shape->num_parts; i++ ) {
        status = ATS_NODE_CALL(shape->parts[i].geometry, Traverse)
                                     (shape->parts[i].geometry, func, data);
        switch ( status ) {
        case  ATM_TRAV_PRUNE:
            return ATM_TRAV_CONT;

        case ATM_TRAV_CONT:
            break;

        case ATM_TRAV_TERM:
            return ATM_TRAV_TERM;
        }

        status = ATS_NODE_CALL(shape->parts[i].material, Traverse)
                                     (shape->parts[i].material, func, data);
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

void atsShapeNewType(AtsType *gt) {
    atsNodeNewType(gt);

    /* set default methods */
           
    gt->Init                = ShapeInit;
    gt->Delete              = ShapeDelete;
    gt->Clone               = ShapeClone;
    gt->Print               = ShapePrint;
    gt->Load                = ShapeLoad;
    gt->Fixup               = ShapeFixup;
    gt->Store               = ShapeStore;
    gt->EnumerateReferences = ShapeEnumerateReferences;
    gt->Draw                = ShapeDraw;
    gt->Traverse            = ShapeTraverseNodes;
    gt->TraverseVertices    = ShapeTraverseVertices;
    return ;
}

void _atsShapeInitClass(void) {
    atsShapeNewType(_ats_shape_type);
    _ats_shape_type->name   = "Shape";
    _ats_shape_type->size   = sizeof(AtsShape);
    _ats_shape_type->parent = _ats_node_type;
}

AtrTriSet *_atsTriSetNew( char *where, FxU32 line ) {
    AtrTriSet *t;

    if ((t =  atrTriSetAllocate( 1 )) == NULL ) {
        atuError(FXTRUE, "%s(%d) _atsTriSetNew: not enough memory\n",
             where, line);
    }

    atuMemType(t, _ats_triset_type->index);

    return t;
}

void _atsGeometryInitClass(void) {
    atsNodeNewType(_ats_geometry_type);
    _ats_geometry_type->name   = "Geometry";
    _ats_geometry_type->size   = 0; /* abstract class */
    _ats_geometry_type->parent = _ats_node_type;
}

static void TriSetInit(AtsObject *node) {
    FXUNUSED(node);
    /* ATS_PARENT_CALL(_ats_triset_type, Init)(node); */
}

static void TriSetDelete(AtsObject *node) {
    AtrTriSet *t;

    VALIDATE_TRISET(t, node, "TriSetDelete"); 

    atrTriSetDeallocate( t ) ;
}

static AtsNode *TriSetClone(AtsObject* node, FxU32 mode) {
    AtrTriSet *t;

    VALIDATE_TRISET(t, node, "TriSetClone"); 

    switch ( mode ) {
    case ATS_CLONE_HIERARCHY: /* clone only the hierarchy */
        atsRef(node);
        return node;
    case ATS_CLONE_ALL:       /* clone the hierarchy and geometry */
        atuError( FXTRUE, "TriSetClone(): ATS_CLONE_ALL Not Implemented yet.\n" );
        return NULL;
    default:
        atuError( FXTRUE, "TriSetClone(): unknown clone method %d\n", mode );
        return NULL;
    }
}

static FxBool TriSetPrint(const AtsObject* node, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    AtrTriSet *t;
    FxU32 numVerts = 0;
    _AtrTriSetNode *nodes ;
    FxBool hasLightMaps = FXFALSE;

    FXUNUSED(verbose);

    VALIDATE_TRISET(t, node, "TriSetPrint"); 

    for ( nodes = t->nodes; nodes; nodes = nodes->next ) {
        numVerts += nodes->numVertices;
        if ( nodes->lightMaps != NULL )
            hasLightMaps = FXTRUE;
    }

    fprintf( stream, "%*s numVerts = %d\n", indent, " ", numVerts);
    fprintf( stream, "%*s has light maps =  %s\n", indent, " ",
             hasLightMaps? "TRUE":"FALSE");
    
    return FXTRUE;
}

static FxBool TriSetLoad(AtsObject *obj, FILE *stream) {
    AtrTriSet *t;
    FxU32 rev;
    _AtrTriSetNode *n;
    _AtrTriSetNode *lastNode = 0;
    FxU32 flag;
    FxU32 index;
    char  type[64];

    VALIDATE_TRISET(t, obj, "TriSetLoad"); 

#ifdef AT_DEBUGGING
    if ( !t || !stream ) 
        atuError( FXTRUE, "TriSetLoad(): Invalid parameter.\n" );
#endif    

    CHECK(atuRead32( &rev, 1, stream ));
    CHECK( rev == _trisetBinaryRevision );

    CHECK(atuRead8( type, sizeof( _triDataType ), stream ));
    CHECK(!strcmp( type, _triDataType ));

    CHECK( atuRead32(&t->bbMinX, 1, stream ));
    CHECK( atuRead32(&t->bbMinY, 1, stream ));
    CHECK( atuRead32(&t->bbMaxX, 1, stream ));
    CHECK( atuRead32(&t->bbMaxY, 1, stream ));
    CHECK( atuRead32(t->texturePosition, 2, stream ));

    CHECK( atuRead32( &flag, 1, stream ));

    while( flag ) {
        FxU32 numTris, numLightMaps;
        n = atuMemCalloc( sizeof( _AtrTriSetNode ), 1 );
        if ( t->nodes == 0 ) t->nodes = n;
        else lastNode->next = n;
        lastNode = n;
        CHECK(atuRead32( &n->numVertices, 1, stream ));
        n->vertices = atuMemAlignedCalloc( n->numVertices, 
                                           sizeof( AtrVertex ),
                                           32 );
        CHECK(atuRead32( n->vertices, 
                       ( sizeof( AtrVertex ) * n->numVertices)>>2,
                       stream ));
        CHECK(atuRead32( &numTris, 1, stream ));
        n->connectivity = atuMemAlignedCalloc( numTris+1, sizeof( FxU32 ), 32 );
        CHECK(atuRead32( n->connectivity, numTris, stream ));
        CHECK(atuRead32( &numLightMaps, 1, stream ));
        if ( numLightMaps > 0 ) {
            n->lightMaps = atuMemAlignedCalloc( numLightMaps, sizeof( AtrTexHandle), 4 );
            for( index = 0; index < numLightMaps; index++ ) {
                AtrImg *img = atsImageNew();
                CHECK ( atsImageLoad( img, stream )) ;
                n->lightMaps[index] = atrTexNewHandle(ATR_TEXELFX_0);
                atrTexAssociate( n->lightMaps[index], img );
            }
        } else n->lightMaps = NULL;
        CHECK(atuRead32( &flag, 1, stream ));
    }

    return FXTRUE;
}

static FxBool TriSetFixup(AtsObject *obj) {
    AtrTriSet *t ;

    VALIDATE_TRISET(t, obj, "TriSetFixup");

    if (!ATS_PARENT_CALL(_ats_triset_type, Fixup)(obj))
        return FXFALSE;

    return FXTRUE;
}

static FxBool TriSetStore(AtsObject *obj, FILE *stream) {
    AtrTriSet *t;
    _AtrTriSetNode *n;
    FxU32 zero = 0;
    FxU32 numTris;

    VALIDATE_TRISET(t, obj, "TriSetStore"); 

#ifdef AT_DEBUGGING
    if ( !t || !stream ) 
        atuError( FXTRUE, "TriSetStore(): Invalid parameter.\n" );
#endif    
    CHECK ( atuWrite32( &_trisetBinaryRevision, 1, stream ) );

    CHECK( atuWrite8( _triDataType, sizeof( _triDataType ), stream ));

    n = t->nodes;

    CHECK( atuWrite32(&t->bbMinX, 1, stream ));
    CHECK( atuWrite32(&t->bbMinY, 1, stream ));
    CHECK( atuWrite32(&t->bbMaxX, 1, stream ));
    CHECK( atuWrite32(&t->bbMaxY, 1, stream ));
    CHECK( atuWrite32(t->texturePosition, 2, stream ));

    while( n ) {
        FxU32 one = 1;
        FxU32 zero = 0;
        FxU32 *connectivity;

        CHECK(atuWrite32( &one, 1, stream ));
        CHECK(atuWrite32( &n->numVertices, 1, stream ));
        CHECK(atuWrite32( n->vertices, 
                       ( sizeof( AtrVertex ) * n->numVertices)>>2,
                       stream ));
        connectivity = n->connectivity;
        numTris = 0;
        while( *connectivity ) {
            connectivity++;
            numTris++;
        }
        CHECK(atuWrite32( &numTris, 1, stream ));
        CHECK(atuWrite32( n->connectivity, numTris, stream ));

        if ( n->lightMaps != NULL ) {
            FxU32 i;
            /* if lightMaps are present we have one for each triangle */
            CHECK(atuWrite32( &numTris, 1, stream ));

            for ( i = 0; i < numTris; i++ ) {
                AtrTexInfo info;
                atrTexInfo( n->lightMaps[i], &info );
                CHECK(atsImageStore( info.img, stream ));
            }
        } else {
            CHECK(atuWrite32( &zero, 1, stream ));
        }
        n = n->next;
    }
    CHECK(atuWrite32( &zero, 1, stream ));

    return FXTRUE;
}

static void TriSetEnumerateReferences(AtsObject *obj) {
    AtrTriSet *t ;

    VALIDATE_TRISET(t, obj, "TriSetFixup");

    ATS_PARENT_CALL(_ats_triset_type, EnumerateReferences)(obj);

    return;
}

static void TriSetDraw(AtsObject *node, FxU32 mask) {
    AtrTriSet *t;

    VALIDATE_TRISET(t, node, "TriSetDraw"); 

    FXUNUSED(mask);

    if ( _atsRenderMode == ATS_RM_WIREFRAME ) 
         atrRenderTriSetWF( t );
    else atrRenderTriSet( t ) ;
}

static void TriSetTraverseVertices(AtsObject* node, AtsTravVertexFunc func, 
                              void *data) {
    AtrTriSet *t;
    _AtrTriSetNode *n ;

    VALIDATE_TRISET(t, node, "TriSetTraverseVertices"); 

    n = t->nodes;

    while( n ) {
        (* func)(n->numVertices, n->vertices, data);
        n = n->next;
    }
}

void _atsTriSetInitClass(void) {
    atsNodeNewType(_ats_triset_type);
    _ats_triset_type->name   = "TriSet";
    _ats_triset_type->size   = sizeof(AtrTriSet);
    _ats_triset_type->parent = _ats_geometry_type;

    /* initialize methods */

    _ats_triset_type->Init                = TriSetInit;
    _ats_triset_type->Delete              = TriSetDelete;
    _ats_triset_type->Clone               = TriSetClone;
    _ats_triset_type->Print               = TriSetPrint;
    _ats_triset_type->Load                = TriSetLoad;
    _ats_triset_type->Fixup               = TriSetFixup;
    _ats_triset_type->Store               = TriSetStore;
    _ats_triset_type->EnumerateReferences = TriSetEnumerateReferences;
    _ats_triset_type->Draw                = TriSetDraw;
    _ats_triset_type->TraverseVertices    = TriSetTraverseVertices;

    return ;
}

AtmCMesh *_atsCMeshNew( char *where, FxU32 line ) {
    AtmCMesh *cmesh;

    if (( cmesh =  atmCMeshNew()) == NULL ) {
        atuError(FXTRUE, "%s(%d) _atsTriSetNew: not enough memory\n",
             where, line);
    }

    atuMemType(cmesh, _ats_cmesh_type->index);

    return cmesh;
}

static void CMeshInit(AtsObject *obj) {
    AtmCMesh *cmesh;

    VALIDATE_CMESH(cmesh, obj, "CMeshInit"); 

    ATS_PARENT_CALL(_ats_cmesh_type, Init)(obj);
}

static void CMeshDelete(AtsObject *obj) {
    AtmCMesh *cmesh;

    VALIDATE_CMESH(cmesh, obj, "CMeshDelete"); 

    atmCMeshDelete(cmesh);
}

static void CMeshDraw(AtsObject *obj, FxU32 mask) {
    AtmCMesh *cmesh;

    FXUNUSED(mask);

    VALIDATE_CMESH(cmesh, obj, "CMeshDraw"); 

    atmCMeshDraw(cmesh, FXFALSE);

}

static AtsObject *CMeshClone(AtsObject* obj, FxU32 mode) {
    AtmCMesh *cmesh_src, *cmesh_dst;

    VALIDATE_CMESH(cmesh_src, obj, "CMeshClone"); 

    switch ( mode ) {
    case ATS_CLONE_HIERARCHY: /* clone only the hierarchy */
        atsRef(obj);
        return obj;
    case ATS_CLONE_ALL:       /* clone the hierarchy and geometry */
        cmesh_dst = atmCMeshClone(cmesh_src);
        atuMemType(cmesh_dst, _ats_cmesh_type->index);
        return (AtsObject *)cmesh_dst;
    default:
        atuError( FXTRUE, "CMeshClone(): unknown clone method %d\n", mode );
        return NULL;
    }
}

static FxBool CMeshPrint(const AtsObject* obj, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    AtmCMesh *cmesh;

    VALIDATE_CMESH(cmesh, obj, "CMeshPrint"); 

    atmCMeshPrint(cmesh, stream, indent, verbose);

    return FXTRUE;
}

void _atsCMeshAllocateGridCells( AtmCMesh *cmesh );
void _atsCMeshPopulateGridCells( AtmCMesh *cmesh );

static FxBool 
CMeshLoad(AtsObject *obj, FILE *stream) {
    AtmCMesh *cmesh;
    char type[64];
    FxU32 rev;

    VALIDATE_CMESH(cmesh, obj, "CMeshLoad"); 

#ifdef AT_DEBUGGING
    if ( !cmesh || !stream ) 
        atuError( FXTRUE, "CMeshStore(): Invalid parameter.\n" );
#endif    

    CHECK( atuRead32( &rev, 1, stream ));

    CHECK( rev == _cmeshBinaryRevision );

    CHECK(atuRead8( type, sizeof( _cmeshDataType ), stream ));
    CHECK(!strcmp( type, _cmeshDataType ));

    CHECK( atuRead32( &cmesh->gridMesh, 1, stream));
    CHECK( atuRead32( &cmesh->int_x_size, 1, stream)); 
    CHECK( atuRead32( &cmesh->int_z_size, 1, stream));
    CHECK( atuRead32( &cmesh->mask, 1, stream));
    CHECK( atuRead32( &cmesh->bounds, 6, stream));
    CHECK( atuRead32( &cmesh->grid_unit_size, 1, stream));
    CHECK( atuRead32( &cmesh->num_tris, 1, stream));

    cmesh->max_tris = cmesh->num_tris;

    /* read in triangles */

    if ( cmesh->num_tris > 0 ) {
        cmesh->tri_list = ( AtmCTri * )atuMemMalloc( sizeof( AtmCTri ) * cmesh->max_tris );

        if ( cmesh->tri_list == NULL )
            return FXFALSE;

        CHECK( atuRead32( cmesh->tri_list, 
                          ( cmesh->num_tris*sizeof(AtmCTri))>>2, stream ));
    }

    /* if grided compute cells */

    if (cmesh->gridMesh) {
        _atsCMeshAllocateGridCells( cmesh );
        _atsCMeshPopulateGridCells( cmesh );
    }

    return FXTRUE;
}

static FxBool CMeshStore(AtsObject *obj, FILE *stream) {
    AtmCMesh *cmesh;

    VALIDATE_CMESH(cmesh, obj, "CMeshStore"); 

#ifdef AT_DEBUGGING
    if ( !cmesh || !stream ) 
        atuError( FXTRUE, "CMeshStore(): Invalid parameter.\n" );
#endif    

    CHECK( atuWrite32( &_cmeshBinaryRevision, 1, stream ));

    CHECK(atuWrite8( _cmeshDataType, sizeof( _cmeshDataType ), stream ));

    CHECK( atuWrite32( &cmesh->gridMesh, 1, stream));
    CHECK( atuWrite32( &cmesh->int_x_size, 1, stream)); 
    CHECK( atuWrite32( &cmesh->int_z_size, 1, stream));
    CHECK( atuWrite32( &cmesh->mask, 1, stream));
    CHECK( atuWrite32( &cmesh->bounds, 6, stream));
    CHECK( atuWrite32( &cmesh->grid_unit_size, 1, stream));
    CHECK( atuWrite32( &cmesh->num_tris, 1, stream));

    /* write out triangles */

    if ( cmesh->num_tris > 0 ) {
        CHECK( atuWrite32( cmesh->tri_list, 
                          ( cmesh->num_tris*sizeof(AtmCTri))>>2, stream ));
    }

    /* cells (if present will be regenerated at load time */

    return FXTRUE;
}

void _atsCMeshInitClass(void) {
    atsNodeNewType(_ats_cmesh_type);
    _ats_cmesh_type->name = "CMesh";
    _ats_cmesh_type->size = sizeof(AtmCMesh);
    _ats_cmesh_type->parent = _ats_object_type;

    /* initialize methods */

    _ats_cmesh_type->Init = CMeshInit;
    _ats_cmesh_type->Delete = CMeshDelete;
    _ats_cmesh_type->Clone = CMeshClone;
    _ats_cmesh_type->Print = CMeshPrint;
    _ats_cmesh_type->Draw = CMeshDraw;
    _ats_cmesh_type->Load = CMeshLoad;
    _ats_cmesh_type->Store = CMeshStore;

    return ;
}
