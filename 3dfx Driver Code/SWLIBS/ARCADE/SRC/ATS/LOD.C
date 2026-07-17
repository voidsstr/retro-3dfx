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
** $Date: 10/11/00 7:34:35 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

static AtsType lod_type;
AtsType *_ats_lod_type = &lod_type;
static const FxU32 _binaryRevision = 0;

#define RANGE_INC 10

static void _atsLODExtend(AtsLOD *lod, int n) {
    if ( n >= lod->maxRanges ) {
        lod->maxRanges += n+RANGE_INC;
        lod->range = atuMemRealloc(lod->range,
                                   lod->maxRanges*sizeof(float));
        if (lod->range == NULL )
            atuError(FXTRUE, "Out of memory extending LOD range\n");
    } 
}

/*-------------------------------------------------------------------
/*-------------------------------------------------------------------
  Function: atsLODNew
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a new LOD group node
  Arguments:
    None
  Return:
    a pointer to the new LOD group
  -------------------------------------------------------------------*/

AtsNode* _atsLODNew(char *where, FxU32 line) {
    return _atsNew(_ats_lod_type, where, line);
}

/*-------------------------------------------------------------------
  Function: atsLODGetType
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns a pointer to the lod's type
  Arguments:
    None
  Return:
    pointer to lods type 
  -------------------------------------------------------------------*/

AtsType* atsLODGetType(void) {
    return (AtsType *)_ats_lod_type;
}

/*-------------------------------------------------------------------
  Function: atsLODCenter
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set a lod's center
  Arguments:
    node    - the node
    center  - the new center
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsLODCenter(AtsNode* node, AtmVector3 center) {
    AtsLOD *lod ;

    VALIDATE_LOD(lod, node, "atsLODCenter");

    ATM_VEC3_COPY(lod->center, center);
}

/*-------------------------------------------------------------------
  Function: atsLODGetCenter
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get a lod's center
  Arguments:
    node    - the node
    center  - the returned center
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsLODGetCenter(AtsNode* node, AtmVector3 center) {
    AtsLOD *lod ;

    VALIDATE_LOD(lod, node, "atsLODGetCenter");

    ATM_VEC3_COPY(center, lod->center);
}

/*-------------------------------------------------------------------
  Function: atsLODRange
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set a child's range (switch in distance)
  Arguments:
    node    - the node
    index   - which child
    range   - child's range
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsLODRange(AtsNode* node, int index, float range) {
    AtsLOD *lod ;

    VALIDATE_LOD(lod, node, "atsLODRange");

    _atsLODExtend(lod, index);

    lod->range[index] = range;
}

/*-------------------------------------------------------------------
  Function: atsLODGetNumRanges
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get number of ranges in LOD
  Arguments:
    node    - the node
  Return:
    number of ranges in LOD
  -------------------------------------------------------------------*/

int atsLODGetNumRanges(const AtsNode* node) {
    AtsLOD *lod ;

    VALIDATE_LOD(lod, node, "atsLODGetNumRanges");

    return ATM_MIN(lod->maxRanges, lod->group.num_children);
}

/*-------------------------------------------------------------------
  Function: atsLODGetRange
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get a lod's range
  Arguments:
    node    - the node
    index   - which range to get
  Return:
    LOD's range
  -------------------------------------------------------------------*/

float atsLODGetRange(const AtsNode* node, int index) {
    AtsLOD *lod ;
    int numRanges ;

    VALIDATE_LOD(lod, node, "atsLODGetCenter");

    numRanges = ATM_MIN(lod->maxRanges, lod->group.num_children) ;

#ifdef AT_DEBUGGING
    if ( index >= numRanges ) {
        atuError(FXTRUE, "atsLODGetRange: index out of range %d, max %d\n",
                 index, numRanges);
    }
#endif

    return lod->range[index];
}

/*-------------------------------------------------------------------
  Function: LODGetChild
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Determine which child (if any to display)
  Arguments:
    lod  - the LOD group
  Return:
    Child node to display or NULL if don't display any
  -------------------------------------------------------------------*/

static AtsNode *LODGetChild(AtsLOD *lod) {
    AtsGroup *group = (AtsGroup *)lod;

    if ( group->num_children > 0 )
        return group->children[0];
    else return NULL;
}

static void LODInit(AtsObject *node) {
    AtsLOD *lod ;

    VALIDATE_LOD(lod, node, "LODInit");

    ATS_PARENT_CALL(_ats_lod_type, Init)(node);

    ATM_VEC3_SET(lod->center, 0.0f, 0.0f, 0.0f);
    lod->maxRanges = 0;
    lod->range = NULL;
}

static AtsNode *LODClone(AtsObject* node, FxU32 mode) {
    AtsLOD *gsrc, *gdst ;
    AtsNode *ndst;
    int i;

    VALIDATE_LOD(gsrc, node, "LODClone");

    ndst = ATS_PARENT_CALL(_ats_lod_type, Clone)(node, mode);

    assert(ndst);

    gdst = (AtsLOD *)ndst;

    _atsLODExtend(gdst, gsrc->maxRanges);

    ATM_VEC3_COPY(gdst->center, gsrc->center);

    for ( i = 0; i < gsrc->maxRanges; i++ )
        gdst->range[i] = gsrc->range[i];

    gdst->maxRanges = gsrc->maxRanges;

    return ndst;
}

static FxBool LODPrint(const AtsObject* node, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    AtsLOD *lod ;
    int i;
    int numRanges ;

    VALIDATE_LOD(lod, node, "LODPrint");

    ATS_PARENT_CALL(_ats_lod_type, Print)(node, stream, indent, verbose);

    numRanges = ATM_MIN(lod->maxRanges, lod->group.num_children) ;
    fprintf( stream, "%*scenter %f %f %f \n", indent+4, " ", 
                      lod->center[0], lod->center[1], lod->center[2]);
    fprintf( stream, "%*s", indent+4, " ");
    for ( i = 0; i < numRanges; i++ )
        fprintf(stream, "%f ", lod->range[i]);

    return FXTRUE;
}

static FxBool LODLoad(AtsObject *obj, FILE *stream){
    AtsLOD *lod ;
    FxU32 version;
    int maxRanges;

    VALIDATE_LOD(lod, obj, "LODLoad");

    if (!ATS_PARENT_CALL(_ats_lod_type, Load)(obj, stream))
        return FXFALSE;
    
    CHECK( atuRead32( &version, 1, stream ));

    CHECK( version == _binaryRevision );

    CHECK( atuRead32( lod->center, 3, stream ));

    CHECK( atuRead32( &maxRanges, 1, stream ));

    _atsLODExtend(lod, maxRanges);

    CHECK( atuRead32( lod->range, maxRanges, stream ));

    return FXTRUE;
}

static FxBool LODStore(AtsObject *obj, FILE *stream) {
    AtsLOD *lod ;

    VALIDATE_LOD(lod, obj, "LODStore");

    if (!ATS_PARENT_CALL(_ats_lod_type, Store)(obj, stream))
        return FXFALSE;
    
    CHECK( atuWrite32( &_binaryRevision, 1, stream ));

    CHECK( atuWrite32( lod->center, 3, stream ));

    CHECK( atuWrite32( &lod->maxRanges, 1, stream ));

    CHECK( atuWrite32( lod->range, lod->maxRanges, stream ));

    return FXTRUE;
}

static FxBool LODIsectNode(const AtsNode *node, const AtsNode *other) {
    AtsLOD *lod ;
    FxBool status;
    AtsNode *c;

    VALIDATE_LOD(lod, node, "LODIsectNode");

    if (( c= LODGetChild(lod)) != NULL ) {
        status = ATS_NODE_CALL(c, IsectNode)(c, other);
    } else status = FXFALSE;

    return status;
}

static FxBool LODIsectSphere(const AtsNode *node, const AtmSphere *bsphere) {
    AtsLOD *lod ;
    FxBool status;
    AtsNode *c;

    VALIDATE_LOD(lod, node, "LODIsectSphere");

    if (( c= LODGetChild(lod)) != NULL ) {
         status = ATS_NODE_CALL(c, IsectSphere)(c, bsphere);
    } else status = FXFALSE;

    return status;
}

static void LODIsectSeg(const AtsNode *node, AtmIsect *isect) {
    AtsLOD *lod ;
    AtsNode *c;

    VALIDATE_LOD(lod, node, "LODIsectSeg");

    if (( c= LODGetChild(lod)) != NULL ) {
        ATS_NODE_CALL(c, IsectSeg)(c, isect);
    }
}

/*-------------------------------------------------------------------
  Function: atsLODControl
  Date: 7/25/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Control which LOD is being selected. The scale factor can be used for
    load balancing and to accomadate different fields of view.
  Arguments:
    lodMode - 
       ATS_LOD_MAX:      display highest level of detail
       ATS_LOD_OFF:      same as ATS_LOD_MAX
       ATS_LOD_MIN:      display lowest level of detail
       ATS_LOD_LEVEL:    display level of detail specified by param
       ATS_LOD_DISTANCE: display level of detail determined by distance
                         from the eye point scaled by param.
  Return:
    pointer to lods type 
  -------------------------------------------------------------------*/

static lodMode = ATS_LOD_DISTANCE;
static float lodScale = 1.0f;

void atsLODControl(int mode, float param) {
    switch ( mode ) {
    case ATS_LOD_OFF:
    case ATS_LOD_MAX:
    case ATS_LOD_MIN:
        break;
    case ATS_LOD_LEVEL:
       lodScale = param;
       break;
    case ATS_LOD_DISTANCE:
       lodScale = param;
       break;
    default:
       atuError(FXFALSE, "unknown lod mode %d\n", lodMode);
       return;
    }

    lodMode = mode;
}

void atsGetLODControl(int *mode, float *param) {
    *mode = lodMode;
    *param = lodScale;    
}

static void LODDraw(AtsObject *node, FxU32 mask) {
    AtsLOD *lod ;
    AtsNode *c;
    AtmVector3 eyepoint;
    float d;
    int i, j, numRanges;

    VALIDATE_LOD(lod, node, "LODDraw");

    if ( ( lod->group.node.mask & mask ) == 0 )
        return;

    /* determine child to display */

    numRanges = ATM_MIN(lod->maxRanges, lod->group.num_children) ;

    if ( numRanges == 0 )
        return;
    
    switch ( lodMode ) {
    case ATS_LOD_OFF:
    case ATS_LOD_MAX:
        i = 0;
        break;
    case ATS_LOD_MIN:
        i = numRanges-1;
        break;
    case ATS_LOD_LEVEL:
       i = ATM_MIN((int)lodScale, numRanges-1);
       break;
    case ATS_LOD_DISTANCE:
        atrQueryLCSEyePoint(eyepoint);

        d = lodScale*atmDistPointToPoint(eyepoint, lod->center);
        i = -1;

        for ( j = 0; j < numRanges; j++ ) {
            if ( lod->range[j] < d ) {
                if ( i == -1 ) {
                    i = j;
                } else if ( lod->range[j] > lod->range[i] ) {
                    i = j;
                }
            }
        }

       break;
    default:
       atuError(FXTRUE, "LODDraw: invalid lod mode %d\n", lodMode);
       return;
    }

	if ( i >= 0 ) {
		c = lod->group.children[i];

		ATS_NODE_CALL(c, Draw)(c, mask);
	}
}

void atsLODNewType(AtsType *ft) {
    atsGroupNewType(ft);

    /* set default methods */
           
    ft->Init = LODInit;
    ft->Clone = LODClone;
    ft->Print = LODPrint;
    ft->Load = LODLoad;
    ft->Store = LODStore;
    ft->Draw = LODDraw;
    ft->IsectNode = LODIsectNode;
    ft->IsectSphere = LODIsectSphere;
    ft->IsectSeg = LODIsectSeg;

    return ;
}

void _atsLODInitClass(void) {
    atsLODNewType(_ats_lod_type);
    _ats_lod_type->name = "LOD";
    _ats_lod_type->size = sizeof(AtsLOD);
    _ats_lod_type->parent = (AtsType *)_ats_group_type;
}
