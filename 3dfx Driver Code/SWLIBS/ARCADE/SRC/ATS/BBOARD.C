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
** $Date: 10/11/00 7:34:30 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <atscenep.h>

static AtsType bboard_type;
AtsType *_ats_bboard_type = &bboard_type;
static const FxU32 _binaryRevision = 1;
extern AtsType *_ats_shape_type ;

#define VALIDATE_BBOARD(sp, np, func) \
                    ATS_VALIDATE_NODE(sp, np, AtsBboard, _ats_bboard_type, func)

#define POS_INC 10

typedef struct {
    AtmVector3 dir;
    int axisKind;
} BboardAxes;

#define ATS_BB_XAXIS    0
#define ATS_BB_YAXIS    1
#define ATS_BB_ZAXIS    2
#define ATS_BB_NXAXIS   3
#define ATS_BB_NYAXIS   4
#define ATS_BB_NZAXIS   5
#define ATS_BB_GAXIS    6   /* general case */

static BboardAxes axes[] = { { 1.0f,  0.0f,  0.0f, ATS_BB_XAXIS},
                             {-1.0f,  0.0f,  0.0f, ATS_BB_NXAXIS},
                             { 0.0f,  1.0f,  0.0f, ATS_BB_YAXIS},
                             { 0.0f, -1.0f,  0.0f, ATS_BB_NYAXIS},
                             { 0.0f,  0.0f,  1.0f, ATS_BB_ZAXIS},
                             { 0.0f,  0.0f, -1.0f, ATS_BB_NXAXIS}};

#define NUM_AXES (sizeof(axes)/sizeof(BboardAxes))

static void determineAxis(AtsBboard *bboard) {
    int i;

    if ( bboard->mode != ATS_BB_AXIAL ) 
        return;

    for ( i = 0; i < NUM_AXES; i++ ) {
        if ( ATM_VEC3_EQUAL(bboard->axis, axes[i].dir)) {
            bboard->axisKind = axes[i].axisKind; 
            return;
        }
    }

    bboard->axisKind = ATS_BB_GAXIS; 
}

static void _atsBboardExtend(AtsBboard *bboard, int n) {
    if ( n >= bboard->maxPos ) {
        bboard->maxPos += n+POS_INC;
        bboard->pos = (AtmVector3*)atuMemRealloc(bboard->pos,
                                             bboard->maxPos*sizeof(AtmVector3));
        if (bboard->pos == NULL )
            atuError(FXTRUE, "Out of memory extending Bboard range\n");
    } 
}

/*-------------------------------------------------------------------
  Function: atsBboardNew
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a new Bboard group node
  Arguments:
    None
  Return:
    a pointer to the new Bboard group
  -------------------------------------------------------------------*/

AtsNode* _atsBboardNew(char *where, FxU32 line) {
    return _atsNew(_ats_bboard_type, where, line);
}

/*-------------------------------------------------------------------
  Function: atsBboardGetType
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns a pointer to the bboard's type
  Arguments:
    None
  Return:
    pointer to bboards type 
  -------------------------------------------------------------------*/

AtsType* atsBboardGetType(void) {
    return (AtsType *)_ats_bboard_type;
}

/*-------------------------------------------------------------------
  Function: atsBboardMode
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set a billboards mode
  Arguments:
    billboard - billboard whose mode is being set
    mode      - mode of operation
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsBboardMode(AtsNode* node, int mode) {
    AtsBboard *bboard ;

    VALIDATE_BBOARD(bboard, node, "atsBboardMode");
 
    switch ( mode ) {
    case ATS_BB_AXIAL:
    case ATS_BB_POINT:
    case ATS_BB_VIEW:
        break;
    default:
        atuError(FXTRUE, "atsBboardMode: unknown mode %d\n", mode);
    }

    bboard->mode = mode;
    determineAxis(bboard);
}

/*-------------------------------------------------------------------
  Function: atsBboardGetMode
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns a bboard's mode
  Arguments:
    billboard - billboard whose mode is being set
  Return:
    mode of operation
  -------------------------------------------------------------------*/

int atsBboardGetMode(AtsNode* node) {
    AtsBboard *bboard ;

    VALIDATE_BBOARD(bboard, node, "atsBboardMode");
 
    return bboard->mode ;
}

/*-------------------------------------------------------------------
  Function: atsBboardAxis
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set a bboard's axis
  Arguments:
    node    - the node
    axis    - the new axis
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsBboardAxis(AtsNode* node, AtmVector3 axis) {
    AtsBboard *bboard ;

    VALIDATE_BBOARD(bboard, node, "atsBboardAxis");

    ATM_VEC3_COPY(bboard->axis, axis);

    determineAxis(bboard);
}

/*-------------------------------------------------------------------
  Function: atsBboardGetAxis
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get a bboard's axis
  Arguments:
    node    - the node
    axis    - the returned axis
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsBboardGetAxis(AtsNode* node, AtmVector3 axis) {
    AtsBboard *bboard ;

    VALIDATE_BBOARD(bboard, node, "atsBboardGetAxis");

    ATM_VEC3_COPY(axis, bboard->axis);
}

/*-------------------------------------------------------------------
  Function: atsBboardPos
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set a part's position
  Arguments:
    node    - the node
    index   - which part
    pos     - part's position
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsBboardPos(AtsNode* node, int index, AtmVector3 pos) {
    AtsBboard *bboard ;

    VALIDATE_BBOARD(bboard, node, "atsBboardPos");

#ifdef AT_DEBUGGING
    if ((index < 0 ) || ( index > bboard->shape.num_parts )) {
        atuError(FXTRUE, "atsBboardPos: index out of range %d\n", index);
    }
#endif

    _atsBboardExtend(bboard, index);

    ATM_VEC3_COPY(bboard->pos[index], pos);
}

/*-------------------------------------------------------------------
  Function: atsBboardGetPos
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get the position of a specified part
  Arguments:
    node    - the node
    index   - which part
    pos     - the part's position
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsBboardGetPos(AtsNode* node, int index, AtmVector3 pos) {
    AtsBboard *bboard ;

    VALIDATE_BBOARD(bboard, node, "atsBboardGetNumRanges");

#ifdef AT_DEBUGGING
    if ((index < 0 ) || ( index > bboard->shape.num_parts )) {
        atuError(FXTRUE, "atsBboardGetPos: index out of range %d\n", index);
    }
#endif

    ATM_VEC3_COPY(pos, bboard->pos[index]);
}

static void BboardInit(AtsObject *node) {
    AtsBboard *bboard ;

    VALIDATE_BBOARD(bboard, node, "BboardInit");

    ATS_PARENT_CALL(_ats_bboard_type, Init)(node);

    bboard->mode = ATS_BB_AXIAL;
    bboard->axisKind = ATS_BB_YAXIS ;
    ATM_VEC3_SET(bboard->axis, 0.0f, 1.0f, 0.0f);
    bboard->maxPos = 0;
    bboard->pos = NULL;
}

static AtsNode *BboardClone(AtsObject* node, FxU32 mode) {
    AtsBboard *gsrc, *gdst ;
    AtsNode *ndst;
    int i;

    VALIDATE_BBOARD(gsrc, node, "BboardClone");

    ndst = ATS_PARENT_CALL(_ats_bboard_type, Clone)(node, mode);

    assert(ndst);

    gdst = (AtsBboard *)ndst;

    gdst->mode = gsrc->mode;

    _atsBboardExtend(gdst, gsrc->maxPos);

    ATM_VEC3_COPY(gdst->axis, gsrc->axis);

    for ( i = 0; i < gsrc->maxPos; i++ )
        ATM_VEC3_COPY(gdst->pos[i], gsrc->pos[i]);

    gdst->maxPos = gsrc->maxPos;

    return ndst;
}

static FxBool BboardPrint(const AtsObject* node, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    AtsBboard *bboard ;
    int i;
    int numPos ;
    char *modeString = "None";

    VALIDATE_BBOARD(bboard, node, "BboardPrint");

    ATS_PARENT_CALL(_ats_bboard_type, Print)(node, stream, indent, verbose);

    switch ( bboard->mode ) {
    case ATS_BB_AXIAL:
        modeString = "ATS_BB_AXIAL";
        break;
    case ATS_BB_POINT:
        modeString = "ATS_BB_POINT";
        break;
    case ATS_BB_VIEW:
        modeString = "ATS_BB_VIEW";
        break;
    default:
        atuError(FXTRUE, "atsBboardMode: unknown mode %d\n", bboard->mode);
    }

    fprintf( stream, "%*smode %f %f %f \n", indent+4, " ", modeString);

    numPos = ATM_MIN(bboard->maxPos, bboard->shape.num_parts) ;
    fprintf( stream, "%*saxis %f %f %f \n", indent+4, " ", 
                      bboard->axis[0], bboard->axis[1], bboard->axis[2]);
    fprintf( stream, "%*s", indent+4, " ");
    for ( i = 0; i < numPos; i++ )
        fprintf( stream, "%*saxis %f %f %f \n", indent+8, " ", 
                      bboard->pos[i][0], bboard->pos[i][1], bboard->pos[i][2]);

    return FXTRUE;
}

static FxBool BboardLoad(AtsObject *obj, FILE *stream){
    AtsBboard *bboard ;
    FxU32 version;
    int i, maxPos;

    VALIDATE_BBOARD(bboard, obj, "BboardLoad");

    if (!ATS_PARENT_CALL(_ats_bboard_type, Load)(obj, stream))
        return FXFALSE;
    
    CHECK( atuRead32( &version, 1, stream ));

    CHECK( version == _binaryRevision )

    CHECK( atuRead32( &bboard->mode, 1, stream ));

    CHECK( atuRead32(&bboard->axisKind, 1, stream ));

    CHECK( atuRead32( bboard->axis, 3, stream ));

    CHECK( atuRead32( &maxPos, 1, stream ));

    _atsBboardExtend(bboard, maxPos);

    for ( i = 0; i < maxPos; i++ )
        CHECK( atuRead32( &bboard->pos[i][0], 3, stream ));

    return FXTRUE;
}

static FxBool BboardStore(AtsObject *obj, FILE *stream) {
    AtsBboard *bboard ;
    int i;

    VALIDATE_BBOARD(bboard, obj, "BboardStore");

    if (!ATS_PARENT_CALL(_ats_bboard_type, Store)(obj, stream))
        return FXFALSE;
    
    CHECK( atuWrite32( &_binaryRevision, 1, stream ));

    CHECK( atuWrite32( &bboard->mode, 1, stream ));

    CHECK( atuWrite32(&bboard->axisKind, 1, stream ));

    CHECK( atuWrite32( bboard->axis, 3, stream ));

    CHECK( atuWrite32( &bboard->maxPos, 1, stream ));

    for ( i = 0; i < bboard->maxPos; i++ )
        CHECK(atuWrite32( &bboard->pos[i][0], 3, stream ));

    return FXTRUE;
}

static FxBool BboardIsectNode(const AtsNode *node, const AtsNode *other) {
    AtsBboard *bboard ;

    VALIDATE_BBOARD(bboard, node, "BboardIsectNode");

    return (!ATS_PARENT_CALL(_ats_bboard_type, IsectNode)(node, other));
}

static FxBool BboardIsectSphere(const AtsNode *node, const AtmSphere *bsphere) {
    AtsBboard *bboard ;

    VALIDATE_BBOARD(bboard, node, "BboardIsectSphere");

    return (!ATS_PARENT_CALL(_ats_bboard_type, IsectSphere)(node, bsphere));
}

static void BboardIsectSeg(const AtsNode *node, AtmIsect *isect) {
    AtsBboard *bboard ;

    VALIDATE_BBOARD(bboard, node, "BboardIsectSphere");

    ATS_PARENT_CALL(_ats_bboard_type, IsectSeg)(node, isect);
}

static AtmVector3 billNorm = { 0.0f, 0.0f, -1.0f };

static void axialXform(AtsBboard *bboard, AtmVector3 eyePoint, AtmVector3 pos, 
                  AtrXform *xform) {
    AtmVector3 lineOfSight, losPlane, tmp, normPlane ;
    float angle;
    AtrXform trans;

    ATM_VEC3_SUB(lineOfSight, eyePoint, pos);

    switch ( bboard->axisKind ) {
    case ATS_BB_XAXIS:
        angle = (float)atan2(lineOfSight[1], lineOfSight[2]);
        atrXformSetXRotation(xform, -angle);
        break;
    case ATS_BB_YAXIS:
        angle = (float)atan2(lineOfSight[0], -lineOfSight[2]);
        atrXformSetYRotation(xform, -angle);
        break;
    case ATS_BB_ZAXIS:
        angle = (float)atan2(lineOfSight[0], -lineOfSight[1]);
        atrXformSetZRotation(xform, -angle);
        break;
    case ATS_BB_NXAXIS:
        angle = (float)atan2(lineOfSight[1], -lineOfSight[2]);
        atrXformSetXRotation(xform, -angle);
        break;
    case ATS_BB_NYAXIS:
        angle = (float)atan2(lineOfSight[0], lineOfSight[2]);
        atrXformSetYRotation(xform, -angle);
        break;
    case ATS_BB_NZAXIS:
        angle = (float)atan2(lineOfSight[0], lineOfSight[1]);
        atrXformSetZRotation(xform, angle);
        break;
    case ATS_BB_GAXIS: /* general case */
	/* calculate the plane containing the axis and the line of sight */

        atmVector3Cross(losPlane, bboard->axis, lineOfSight);
        atmVector3Normalize(losPlane, losPlane);

	/* calculate the plane containing the axis and the normal */

        atmVector3Cross( normPlane, bboard->axis, billNorm);
        atmVector3Normalize(normPlane, normPlane);

	/* calculate the angle between the two planes, which is the
	   angle that the normal needs to be rotated by to bring
	   it into the plane of the axis and lineOfSight
         */

	angle = (float)acos(atmVector3Dot(losPlane, normPlane));

	/* check for the correct direction in which to rotate bboard */

	atmVector3Cross(tmp, billNorm, lineOfSight);
	    
	if (atmVector3Dot(tmp, bboard->axis) < 0.0)
	    angle = ATM_PI - angle;
	else
	    angle -= ATM_PI;

        atrXformSetRotation( xform, bboard->axis, angle);
    
        break;
    }

    atrXformSetTranslation(&trans, pos[0], pos[1], pos[2]);
    atrXformCat(xform, xform, &trans);
}

static AtmVector3 origin = { 0.0f, 0.0f, 0.0f};
static AtmVector3 up = { 0.0f, 1.0f, 0.0f};

static void pointXform(AtsBboard *bboard, AtmVector3 eyePoint, AtmVector3 pos, 
                  AtrXform *xform) {
    AtmVector3 lineOfSight;
    AtrXform trans;

    FXUNUSED(bboard);

    ATM_VEC3_SUB(lineOfSight, eyePoint, pos);

    atrXformPointAt(xform, lineOfSight, origin, up);

    atrXformSetTranslation(&trans, pos[0], pos[1], pos[2]);

    atrXformCat(xform, xform, &trans);
}

static void viewXform(AtsBboard *bboard, AtmVector3 eyePoint, AtmVector3 pos, 
                 AtrXform *xform) {

    FXUNUSED(bboard);
    FXUNUSED(eyePoint);

    atrQueryXform(xform);
    ATM_VEC3_ADD(pos, pos, &xform->data[12]);
    atrXformSetTranslation(xform, pos[0], pos[1], pos[2]);
}

static void BboardDraw(AtsObject *node, FxU32 mask) {
    AtsBboard *bboard ;
    AtmVector3 eyePoint;
    int i, numPos;
    AtrXform xform;

    VALIDATE_BBOARD(bboard, node, "BboardDraw");

    if ( ( bboard->shape.node.mask & mask ) == 0 )
        return;

    if ( atsNodeVisible(node) == ATM_IS_FALSE )
        return;

    numPos = ATM_MIN(bboard->maxPos, bboard->shape.num_parts) ;

    atrQueryLCSEyePoint(eyePoint);

    for ( i = 0; i < numPos; i++ ) {
        (* _atsPushMaterial)( bboard->shape.parts[i].material);
        switch ( bboard->mode ) {
        case ATS_BB_AXIAL:
            axialXform(bboard, eyePoint, bboard->pos[i], &xform);
            atrPreCatPushXform( &xform );
            ATS_NODE_CALL(bboard->shape.parts[i].geometry, Draw)
                  (bboard->shape.parts[i].geometry, mask);
            break;
        case ATS_BB_POINT:
            pointXform(bboard, eyePoint, bboard->pos[i], &xform);
            atrPreCatPushXform( &xform );
            ATS_NODE_CALL(bboard->shape.parts[i].geometry, Draw)
                  (bboard->shape.parts[i].geometry, mask);
            break;
        case ATS_BB_VIEW:
            viewXform(bboard, eyePoint, bboard->pos[i], &xform);
            atrPushXform( &xform );
            ATS_NODE_CALL(bboard->shape.parts[i].geometry, Draw)
                  (bboard->shape.parts[i].geometry, mask);
            break;
        default:
            atuError(FXTRUE, "atsBboardMode: unknown mode %d\n", bboard->mode);
        }
        atrPopXform(FXTRUE);
        (* _atsPopMaterial ) ( FXFALSE );
    }
}

static void BboardTraverseVertices(AtsObject* node, AtsTravVertexFunc func, 
                              void *data) {
    AtsBboard *bboard ;
    int i;
    AtrXform trans;
    float *pos;

    VALIDATE_BBOARD(bboard, node, "BboardTraverseVertices");

    for ( i = 0; i < bboard->shape.num_parts; i++ ) {
        pos = bboard->pos[i];
        atrXformSetTranslation(&trans, pos[0], pos[1], pos[2]);
        atsPreCatPushXform( &trans );
        ATS_NODE_CALL(bboard->shape.parts[i].geometry, TraverseVertices)
                        (bboard->shape.parts[i].geometry, func, data);
        atsPopXform();
    }
}

void atsBboardNewType(AtsType *ft) {
    atsShapeNewType(ft);

    /* set default methods */
           
    ft->Init = BboardInit;
    ft->Clone = BboardClone;
    ft->Print = BboardPrint;
    ft->Load = BboardLoad;
    ft->Store = BboardStore;
    ft->Draw = BboardDraw;
    ft->IsectNode = BboardIsectNode;
    ft->IsectSphere = BboardIsectSphere;
    ft->IsectSeg = BboardIsectSeg;
    ft->TraverseVertices = BboardTraverseVertices;

    return ;
}

void _atsBboardInitClass(void) {
    atsBboardNewType(_ats_bboard_type);
    _ats_bboard_type->name = "Bboard";
    _ats_bboard_type->size = sizeof(AtsBboard);
    _ats_bboard_type->parent = (AtsType *)_ats_shape_type;
}
