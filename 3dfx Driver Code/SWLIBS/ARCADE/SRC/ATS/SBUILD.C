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
** $Date: 10/11/00 7:34:42 PM$ 
**
** TBD: need to handle two sided surfaces to handle backface culling &
** normal direction.
**/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

/* TBD we should really batch this especially as these are fixed size 
   this would save a copy 
*/  
 
#define VTX_INC 1000;

static AtrVertex *vlist = NULL;
static FxU32 maxVerts = 0;
static FxU32 numVerts = 0;

#define TRI_INC 100;

typedef struct {
    FxU16 i0, i1, i2;
    AtsMaterial *mat;
} AtsTriangle;

static FxU16 maxTri = 0;
static AtsTriangle *tlist = NULL;
static FxU16 numTri ;
static AtsMaterial *currentMaterial ;

static AtsPrim prim;
static builderInitialized = FXFALSE;
static FxU32 maxPrimVerts = 0;
static FxU32 maxPrimIdx   = 0;

static AtsNode *shape;

static void builderAddTriangle(FxU32 i0, FxU32 i1, FxU32 i2) {
    FxU32 bytesNeeded;
    AtsTriangle *t;

    if (numTri >= maxTri ) {
        maxTri += TRI_INC;
        bytesNeeded = maxTri*sizeof(AtsTriangle);
        tlist = (AtsTriangle *)atuMemRealloc(tlist, bytesNeeded);

        if ( tlist == NULL ) 
            atuError(FXTRUE, "out of memory in atsShapeAddPrim\n");
    }

    t = tlist+numTri;

    t->i0 = (FxU16)i0; t->i1 = (FxU16)i1; t->i2= (FxU16)i2; t->mat = currentMaterial;
    numTri++;
}

static void computeFacetNormal(AtsPrim *p) {
    FxU16 i0, i1, i2;
    AtmVector3 p01, p02;
    float l, invl;

    switch ( p->lBind ) {
    case ATS_BIND_INDEXED_VERTEX:
        i0 = p->lIdx[0];
        i1 = p->lIdx[1];
        i2 = p->lIdx[2];
        break;
    case ATS_BIND_PER_VERTEX:
        i0 = 0;
        i1 = 1;
        i2 = 2;
        break;
    default:
        atuError(FXTRUE, "Invalid location binding: %d\n", p->lBind);
        break;
    }

    ATM_VEC3_SUB(p01, p->locations[1], p->locations[0]);
    ATM_VEC3_SUB(p02, p->locations[2], p->locations[0]);

    if ( p->flags & ATS_PRIM_CCW )
         atmVector3Cross(p->normals[0], p02, p01);
    else atmVector3Cross(p->normals[0], p01, p02);

    if ((l = atmSqrt(ATM_VEC3_DOT(p->normals[0], p->normals[0]))) == 0.0f)
        return ;

    invl = 1.0f/l;

    ATM_VEC3_SCALE(p->normals[0], invl, p->normals[0]);
    p->nBind = ATS_BIND_PER_FACET;
}

static void builderAddVertices(AtsPrim *p) {
    FxU32 bytesNeeded;
    FxU16 idx = 0;
    AtrVertex *v;
    FxU16 i;

    if ( numVerts+p->numVerts >= maxVerts ) {
        maxVerts += p->numVerts + VTX_INC;
        bytesNeeded = maxVerts*sizeof(AtrVertex);
        vlist = (AtrVertex *)atuMemRealloc(vlist, bytesNeeded);
        if ( vlist == NULL )
            atuError(FXTRUE, "Out of memory in atsShapeAddPrim\n");
    }

    for ( i = 0, v = vlist+numVerts; i < p->numVerts; i++, v++ ) {
       memset( v, 0, sizeof( AtrVertex ) );
       switch ( p->lBind ) {
       case ATS_BIND_INDEXED_VERTEX:
           idx = p->lIdx[i];
           break;
       case ATS_BIND_PER_VERTEX:
           idx = i;
           break;
       default:
           atuError(FXTRUE, "Invalid location binding: %d\n", p->lBind);
           break;
       }
       v->x = p->locations[idx][0]; 
       v->y = p->locations[idx][1]; 
       v->z = p->locations[idx][2]; 

       if ( p->nBind != ATS_BIND_NONE ) {
           switch ( p->nBind ) {
           case ATS_BIND_PER_FACET:
               idx = 0;
               break; 
           case ATS_BIND_INDEXED_VERTEX:
               idx = p->nIdx[i];
               break;
           case ATS_BIND_PER_VERTEX:
               idx = i;
               break;
           default:
               atuError(FXTRUE, "Invalid normal binding: %d\n", p->nBind);
               break;
           }
           v->i = p->normals[idx][0]; 
           v->j = p->normals[idx][1]; 
           v->k = p->normals[idx][2]; 
       }
        
       if ( p->cBind != ATS_BIND_NONE ) {
           switch ( p->cBind ) {
           case ATS_BIND_PER_FACET:
               idx = 0;
               break; 
           case ATS_BIND_INDEXED_VERTEX:
               idx = p->cIdx[i];
               break;
           case ATS_BIND_PER_VERTEX:
               idx = i;
               break;
           default:
               atuError(FXTRUE, "Invalid normal binding: %d\n", p->cBind);
               break;
           }
           v->r = p->colors[idx][0]; 
           v->g = p->colors[idx][1]; 
           v->b = p->colors[idx][2]; 
           v->a = p->colors[idx][3]; 
       }
        
       if ( p->tBind != ATS_BIND_NONE ) {
           switch ( p->tBind ) {
           case ATS_BIND_INDEXED_VERTEX:
               idx = p->tIdx[i];
               break;
           case ATS_BIND_PER_VERTEX:
               idx = i;
               break;
           default:
               atuError(FXTRUE, "Invalid texture binding: %d\n", p->tBind);
               break;
           }
           v->s0 = p->texCoords[idx][0]; 
           v->t0 = p->texCoords[idx][1]; 
       }
    }

    numVerts += p->numVerts;
}

/*-------------------------------------------------------------------
  Function: atsShapeBuildInit
  Date: 6/21/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Initialize the shape builder
  Arguments:
    maxVerts -  maximum number of vertices in primitive arrays
    maxIdx   -  maximum number of indices in index arrays
  Return:
    Pointer to primitive
  -------------------------------------------------------------------*/

#define GROW(_v, _t, _c) ((_t *)( atuMemRealloc(_v, (_c)*sizeof(_t))))

AtsPrim *atsShapeBuildInit(FxU32 maxVerts, FxU32 maxIdx) {

    numTri = 0;
    numVerts = 0;
    currentMaterial = NULL;

    prim.flags = 0;

    if (!builderInitialized) {
        prim.locations = NULL;
        prim.normals = NULL;
        prim.colors = NULL;
        prim.texCoords = NULL;

        prim.lIdx = NULL;
        prim.nIdx = NULL; 
        prim.cIdx = NULL; 
        prim.tIdx = NULL;

        builderInitialized = FXTRUE;
    }
 
    if ( maxVerts > maxPrimVerts ) {
        prim.locations = GROW(prim.locations, AtmVector3, maxVerts);
        prim.normals = GROW(prim.normals, AtmVector3, maxVerts);
        prim.colors = GROW(prim.colors, AtmVector4, maxVerts);
        prim.texCoords = GROW(prim.texCoords, AtmVector2, maxVerts);

        if (!prim.locations || !prim.normals || !prim.colors || !prim.texCoords )
            atuError(FXTRUE, "Out of memory in atmShapeBuildInit\n");

        maxPrimVerts = maxVerts;
    }

    if ( maxIdx > maxPrimIdx ) {
        prim.lIdx = GROW(prim.lIdx, FxU16, maxIdx);
        prim.nIdx = GROW(prim.nIdx, FxU16, maxIdx); 
        prim.cIdx = GROW(prim.cIdx, FxU16, maxIdx); 
        prim.tIdx = GROW(prim.tIdx, FxU16, maxIdx);

        if (!prim.lIdx || !prim.nIdx || !prim.cIdx || !prim.tIdx )
            atuError(FXTRUE, "Out of memory in atmShapeBuildInit\n");

        maxPrimIdx = maxIdx;
    }

    shape = atsShapeNew();

    return &prim;
}

/*-------------------------------------------------------------------
  Function: atsShapeAddPrim
  Date: 6/21/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Add another primitive to the shape
  Arguments:
    flags - control how primitive will be generated
  Return:
    FXTRUE on success, FXFALSE in error
  -------------------------------------------------------------------*/

FxBool atsShapeAddPrim(void) {
    FxU32 base = numVerts;
    FxU16 i;

    if ( prim.primType != ATS_PRIM_POLYGON ) {
        atuError(FXFALSE, "Invalid primitive type\n");
        return FXFALSE;
    }

    if ( prim.numVerts< 3) {
        atuError(FXFALSE, "Invalid degenerate primitive\n ");
        return FXFALSE;
    }

    if ( prim.flags & ATS_PRIM_GEN_FACET_NORMAL )
       computeFacetNormal(&prim);

    builderAddVertices(&prim);

    for ( i = 0; i < prim.numVerts-2; i++ ) {
        if ( prim.flags & ( ATS_PRIM_CCW | ATS_PRIM_TWO_SIDED ))
            builderAddTriangle(base, base+i+2, base+i+1);
        if ( ! ( prim.flags & ATS_PRIM_CCW ) ||
               ( prim.flags & ATS_PRIM_TWO_SIDED ))
            builderAddTriangle(base, base+i+1, base+i+2);
    }

    return FXTRUE;
}

/* ensure shape has a tri set with the specified material */

static void shapeCheckMaterial(AtsNode *shape, AtsMaterial *mat) {
    int num_parts = atsShapeGetNumParts(shape);
    AtsMaterial *mtmp;
    AtsNode *gtmp;
    int i;

    for ( i = 0; i < num_parts; i++ ) {
        atsShapeGetPart(shape, i, (void *)&gtmp, (void *)&mtmp);

        if ( mtmp == mat )
            return ;
    }

    gtmp = (AtsNode *)atsTriSetNew();

    atsShapeAddPart(shape, gtmp, mat);
}

/*-------------------------------------------------------------------
  Function: atsShapeMaterial
  Date: 6/21/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Specify the current material for a shape
  Arguments:
    mat -  the current material
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsShapeMaterial(AtsMaterial *mat) {
    currentMaterial = mat;

    shapeCheckMaterial(shape, mat);
}

/*-------------------------------------------------------------------
  Function: atsShapeConstruct
  Date: 6/21/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a shape object with the previously specified primitives
    and materials.
  Arguments:
    autoNormals - automatically generate normals for the tri sets
  Return:
    Pointer to constructed shape
  -------------------------------------------------------------------*/

AtsNode *atsShapeConstruct(FxBool autoNormals) {
    FxU32 pNum, tNum;
    FxU32 numParts;
    AtsTriangle *t;
    AtmSphere bsphere;

    /* process all triangles in object */

    numParts = atsShapeGetNumParts(shape);

    if ( numParts == 0 ) /* TBD temporary sanity check */
        return atsGroupNew();

    for ( pNum = 0 ; pNum < numParts; pNum++ ) {
        AtsMaterial *triMat;
        AtrTriSet *triSet;

        /* add all triangles with the parts material */

        atsShapeGetPart(shape, pNum, (void *)&triSet, (void *)&triMat);
        atrTriSetBegin( triSet );

        for ( tNum = 0, t = tlist; tNum < numTri; tNum++, t++ ) {
            if ( t->mat != triMat )
                continue;

             atrTriSetVertex( triSet, vlist+t->i0);
             atrTriSetVertex( triSet, vlist+t->i1);
             atrTriSetVertex( triSet, vlist+t->i2);
        }

        if ( autoNormals )
            atrTriSetCalcNormals();

        atrTriSetEnd( triSet );
    }

    atsComputeBSphere(shape, &bsphere);

    atsNodeBSphere(shape, &bsphere);

    return shape;
}
