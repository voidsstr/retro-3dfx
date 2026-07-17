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
** $Date: 10/11/00 7:34:26 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

static AtsType anim_type;
AtsType *_ats_anim_type = &anim_type;
static const FxU32 _binaryRevision = 1;

#define VALIDATE_ANIM(gp, np, func) \
                    ATS_VALIDATE_NODE(gp, np, AtsAnim, _ats_anim_type, func)

/*-------------------------------------------------------------------
  Function: atsAnimNew
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a new animation group node
  Arguments:
    None
  Return:
    a pointer to the new animation group
  -------------------------------------------------------------------*/

AtsNode* _atsAnimNew(char *where, FxU32 line) {
    return _atsNew(_ats_anim_type, where, line);
}

/*-------------------------------------------------------------------
  Function: atsAnimGetType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns a pointer to the anim's type
  Arguments:
    None
  Return:
    pointer to anims type 
  -------------------------------------------------------------------*/

AtsType* atsAnimGetType(void) {
    return (AtsType *)_ats_anim_type;
}

/*-------------------------------------------------------------------
  Function: atsAnimInstanceNames
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set the animations instance names 
  Arguments:
    anim           - the animation group
    instance_names - names of all the instances in group
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsAnimInstanceNames(AtsNode *node, char **instance_names) {
    AtsAnim *anim ;

    VALIDATE_ANIM(anim, node, "atsAnimInstanceNames");

    if (anim->instance_names)
        atuMemFree(anim->instance_names);

    anim->instance_names = instance_names;
}

/*-------------------------------------------------------------------
  Function: atsAnimGetInstanceNames
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the animations instance names 
  Arguments:
    anim           - the animation group
  Return:
    The names of each of the instances
  -------------------------------------------------------------------*/

char **atsAnimGetInstanceNames(AtsNode *node) {
    AtsAnim *anim ;

    VALIDATE_ANIM(anim, node, "atsAnimGetInstanceNames");

    return anim->instance_names;
}

/*-------------------------------------------------------------------
  Function: atsAnimNameToIndex
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    return the index of an instance with specified name, else return -1
  Arguments:
    anim       - the anim 
    name       - the instances name 
  Return:
    index of instance if present else -1
  -------------------------------------------------------------------*/

int atsAnimNameToIndex(AtsNode* a, const char *name) {
    AtsAnim *anim ;
    AtsNode **children;
    int i;

    VALIDATE_ANIM(anim, a, "atsAnimNameToIndex");

    children = anim->group.children;

    if ( anim->instance_names == NULL )
        return -1;

    for ( i = 0; i < anim->group.num_children; i++ ) {
        if ( strcmp(anim->instance_names[i], name) == 0 ) {
            return i;
        }
    }

    return -1;
}

/*-------------------------------------------------------------------
  Function: atsAnimXforms
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set the animation matrices
  Arguments:
    anim           - the animation group
    num_frames     - number of frames in the animation
    xforms         - an array of transforms
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsAnimXforms(AtsNode *node, FxU16 num_frames, AtrXform *xforms) {
    AtsAnim *anim ;

    VALIDATE_ANIM(anim, node, "atsAnimXforms");

    anim->num_frames = num_frames;

    if ( anim->xforms != xforms ) {
        atsUnrefDelete(anim->xforms);

        anim->xforms = xforms;

        atsRef(xforms);
    }
}

/*-------------------------------------------------------------------
  Function: atsAnimGetXforms
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns the anims matrices and frame counts
  Arguments:
    anim         - the animation group
    num_frames   - number of animation frames
    xforms       - an array of transforms
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsAnimGetXforms(AtsNode *node, FxU16 *num_frames, AtrXform **xforms) {
    AtsAnim *anim ;

    VALIDATE_ANIM(anim, node, "atsAnimGetXforms");

    *num_frames = anim->num_frames;
    *xforms = anim->xforms;
}

/*-------------------------------------------------------------------
  Function: atsAnimFrame
  Date: 5/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set the current frame in an animation
  Arguments:
    anim  - the animation group
    frame - the current frame
  Return:
  -------------------------------------------------------------------*/

void atsAnimFrame(AtsNode *node, const FxU16 frame) {
    AtsAnim *anim ;

    VALIDATE_ANIM(anim, node, "atsAnimFrame");

    anim->current_frame = frame;
}

/*-------------------------------------------------------------------
  Function: atsAnimGetFrame
  Date: 5/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get the current frame in an animation
  Arguments:
    anim - the animation group
  Return:
    the current frame
  -------------------------------------------------------------------*/

FxU16 atsAnimGetFrame(AtsNode *node) {
    AtsAnim *anim ;

    VALIDATE_ANIM(anim, node, "atsAnimGetFrame");

    return ( anim->current_frame );
}

/*-------------------------------------------------------------------
  Function: atsAnimGetFrameCount
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get the number of frames in an animation
  Arguments:
    anim - the animation group
  Return:
    the number of frames
  -------------------------------------------------------------------*/

FxU16 atsAnimGetNumFrames(AtsNode *node) {
    AtsAnim *anim ;

    VALIDATE_ANIM(anim, node, "atsAnimGetNumFrames");

    return ( anim->num_frames );
}

/*-------------------------------------------------------------------
  Function: atsAnimNextFrame
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Advance to the next frame in an animation
  Arguments:
    anim - the animation group
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsAnimNextFrame(AtsNode *node) {
    AtsAnim *anim ;

    VALIDATE_ANIM(anim, node, "atsAnimNextFrame");

    anim->current_frame++;

    /* if at end cycle */

    if ( anim->current_frame >= anim->num_frames ) {
        anim->current_frame = 0;
    }
}

static void AnimInit(AtsObject *node)
{
    AtsAnim *anim ;

    VALIDATE_ANIM(anim, node, "AnimInit");

    anim->current_frame = 0;
    anim->num_frames = 0;
    anim->xforms = NULL;
    anim->instance_names = NULL;

    ATS_PARENT_CALL(_ats_anim_type, Init)(node);
}

static AtsNode *AnimClone(AtsObject* node, FxU32 mode) {
    AtsAnim *gsrc, *gdst ;
    AtsNode *ndst;

    VALIDATE_ANIM(gsrc, node, "AnimClone");

    ndst = ATS_PARENT_CALL(_ats_anim_type, Clone)(node, mode);

    assert(ndst);

    gdst = (AtsAnim *)ndst;

    gdst->current_frame = gsrc->current_frame;
    gdst->num_frames = gsrc->num_frames;
    gdst->xforms = gsrc->xforms;
    atsRef(gdst->xforms);

    return ndst;
}

static void AnimDelete(AtsObject *node) {
    AtsAnim *anim ;

    VALIDATE_ANIM(anim, node, "AnimDelete");

    atsUnrefDelete(anim->xforms);

    ATS_PARENT_CALL(_ats_anim_type, Delete)(node);
}

static FxBool AnimPrint(const AtsObject* node, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    int i;
    AtsAnim *anim ;
    AtrXform *t;
    char **in;

    VALIDATE_ANIM(anim, node, "AnimPrint");

    ATS_PARENT_CALL(_ats_anim_type, Print)(node, stream, indent, verbose);

    /* if we don't have a list of frames we are done */

    if (( anim->xforms != NULL ) && (( verbose & ATS_PRINT_TRANSFORMS ) != 0)) {

        t = anim->xforms;

        fprintf( stream, "%*s xforms {\n", indent, " ");

        i = anim->num_frames*anim->group.num_children;

        while ( i-- > 0 )
            atrXformPrint( t++, stream, indent+4 );

        fprintf( stream, "%*s }\n", indent, " ");
    }

    if ( anim->instance_names ) {
        fprintf( stream, "%*s instance names {\n", indent, " ");
        in = anim->instance_names;
        while ( *in != NULL ) {
            fprintf( stream, "%*s \"%s\" \n", indent+4, " ", *in);
            in++;
        }
        fprintf( stream, "%*s }\n", indent, " ");
    }

    return FXTRUE;
}

static FxBool AnimLoad(AtsObject *obj, FILE *stream){
    AtsAnim *anim ;
    FxU32 num_names;
    char **instance_names;
    FxU16 i;
    FxU32 version;

    VALIDATE_ANIM(anim, obj, "AnimLoad");

    if (!ATS_PARENT_CALL(_ats_anim_type, Load)(obj, stream))
        return FXFALSE;

    CHECK ( atuRead32( &version, 1, stream ));

    CHECK ( version == _binaryRevision )
    
    CHECK ( atuRead16( &anim->current_frame, 1, stream ));
    
    CHECK ( atuRead16( &anim->num_frames, 1, stream ));
    
    CHECK ( atuRead32( &anim->xforms, 1, stream ));

    CHECK ( atuRead32( &num_names, 1, stream ));

    if ( num_names > 0 ) {
        instance_names = (char **)atuMemCalloc(num_names+1, sizeof(char *));
        instance_names[num_names] = NULL;

        for ( i = 0; i < num_names; i++ )
            if (!atsLoadString(&instance_names[i], stream))
                return FXFALSE;

        atsAnimInstanceNames(obj, instance_names);
    }

    return FXTRUE;
}

static FxBool AnimFixup(AtsObject *obj) {
    AtsAnim *anim ;
    FxU16 num_frames;
    AtrXform *xforms;

    VALIDATE_ANIM(anim, obj, "AnimFixup");

    if (!ATS_PARENT_CALL(_ats_anim_type, Fixup)(obj))
        return FXFALSE;

    if ( anim->xforms != NULL ) {
        xforms = (AtrXform *)atsIDToPointer( anim->xforms );

        num_frames = anim->num_frames;
        anim->num_frames = 0;
        anim->xforms = NULL;

        atsAnimXforms(obj, num_frames, xforms);
    }

    return FXTRUE;
}

static FxBool AnimStore(AtsObject *obj, FILE *stream) {
    AtsAnim *anim ;
    void *id;
    FxU32 num_names;
    FxU16 i;

    VALIDATE_ANIM(anim, obj, "AnimStore");

    if (!ATS_PARENT_CALL(_ats_anim_type, Store)(obj, stream))
        return FXFALSE;

    CHECK ( atuWrite32( &_binaryRevision, 1, stream ));

    CHECK ( atuWrite16( &anim->current_frame, 1, stream ));
    
    CHECK ( atuWrite16( &anim->num_frames, 1, stream ));
    
    if ( anim->xforms )
        id = atsPointerToID( anim->xforms );
    else id = 0;

    CHECK ( atuWrite32( &id, 1, stream ));

    if ( anim->instance_names ) {
        for ( num_names = 0; anim->instance_names[num_names] != NULL; 
              num_names++){}
    } else num_names = 0;

    CHECK ( atuWrite32( &num_names, 1, stream ));

    for ( i = 0; i < num_names; i++ )
        if (!atsStoreString(anim->instance_names[i], stream))
            return FXFALSE;

    return FXTRUE;
}

static void AnimEnumerateReferences(AtsObject *obj) {
    AtsAnim *anim ;

    VALIDATE_ANIM(anim, obj, "AnimEnumerateReferences");

    ATS_PARENT_CALL(_ats_anim_type, EnumerateReferences)(obj);

    if ( anim->xforms )
        ATS_NODE_CALL(anim->xforms, EnumerateReferences)(anim->xforms);
}

static FxBool AnimIsectSphere(const AtsNode *node, const AtmSphere *bsphere) {
    AtsAnim *anim ;
    AtsGroup *group ;
    int i;
    AtrXform *t;
    FxBool status;

    VALIDATE_ANIM(anim, node, "AnimIsectSphere");

    if ( anim->xforms == NULL ) {
        return ATS_PARENT_CALL(_ats_anim_type, IsectSphere)(node, bsphere);
    }

    group = &(anim->group);

    t = anim->xforms+anim->current_frame*group->num_children;

    for ( i = 0; i < group->num_children; i++ ) {
        atsPreCatPushXform( t++ );
        status = ATS_NODE_CALL(group->children[i], IsectSphere)(
                                     group->children[i], bsphere);
        atsPopXform();

        /* return as soon as we have an intersection */

        if (status)
            return FXTRUE;
    }

    return FXFALSE;
}

static void AnimIsectSeg(const AtsNode *node, AtmIsect *isect) {
    AtsAnim *anim ;
    AtsGroup *group ;
    int i;
    AtrXform *t;

    VALIDATE_ANIM(anim, node, "AnimIsectSeg");

    if ( anim->xforms == NULL ) {
        ATS_PARENT_CALL(_ats_anim_type, IsectSeg)(node, isect);
    }

    group = &(anim->group);

    t = anim->xforms+anim->current_frame*group->num_children;

    for ( i = 0; i < group->num_children; i++ ) {
        atsPreCatPushXform( t++ );
        ATS_NODE_CALL(group->children[i], IsectSeg)(
                                     group->children[i], isect);
        atsPopXform();
    }
}

static FxBool AnimIsectNode(const AtsNode *node, const AtsNode *other) {
    AtsAnim *anim ;
    AtsGroup *group ;
    int i;
    AtrXform *t;
    FxBool status;

    VALIDATE_ANIM(anim, node, "AnimIsectNode");

    if ( anim->xforms == NULL ) {
        return ATS_PARENT_CALL(_ats_anim_type, IsectNode)(node, other);
    }

    group = &(anim->group);

    t = anim->xforms+anim->current_frame*group->num_children;

    for ( i = 0; i < group->num_children; i++ ) {
        atsPreCatPushXform( t++ );
        status = ATS_NODE_CALL(group->children[i], IsectNode)(
                                      group->children[i], other);
        atsPopXform();
        if ( status )
            return FXTRUE;
    }

    return FXFALSE;
}

static void AnimDrawFrame(AtsObject *node, FxU32 frame, FxU32 mask) {
    AtsAnim *anim ;
    AtsGroup *group ;
    int i;
    AtrXform *t;

    VALIDATE_ANIM(anim, node, "AnimDraw");

    if ( ( anim->group.node.mask & mask ) == 0 )
        return;

    group = &(anim->group);

    t = anim->xforms+frame*group->num_children;

    for ( i = 0; i < group->num_children; i++ ) {
        atrPreCatPushXform( t++ );
        ATS_NODE_CALL(group->children[i], Draw)(group->children[i], mask);
        atrPopXform(FXTRUE);
    }
}

static void AnimDraw(AtsObject *node, FxU32 mask) {
    AtsAnim *anim ;

    VALIDATE_ANIM(anim, node, "AnimDraw");

    /* if we don't have a list of frames treat as a group */

    if ( anim->xforms == NULL ) {
        ATS_PARENT_CALL(_ats_anim_type, Draw)(node, mask);
        return ;
    }

    AnimDrawFrame(node, anim->current_frame, mask);
}

static void AnimTraverseVertices(AtsObject* node, AtsTravVertexFunc func, 
                              void *data) {
    AtsAnim *anim ;
    AtsGroup *group ;
    AtrXform *t;
    int i, frame;

    VALIDATE_ANIM(anim, node, "AnimTraverseVertices");

    if ( anim->xforms == NULL ) {
        ATS_PARENT_CALL(_ats_anim_type, TraverseVertices)(node, func, data);
        return ;
    }

    group = &(anim->group);

    t = anim->xforms;

    /* we traverse all frames for all children */

    for ( frame = 0; frame < anim->num_frames; frame++ ) {

        for ( i = 0; i < group->num_children; i++ ) {
            atsPreCatPushXform( t );
            ATS_NODE_CALL(group->children[i], TraverseVertices)
                            (group->children[i], func, data);
            atsPopXform(); /* TBD: do we really need to update state? */
            t++;
        }
    }
}

void atsAnimNewType(AtsType *at) {
    atsGroupNewType(at);

    /* set default methods */
           
    at->Init = AnimInit;
    at->Clone = AnimClone;
    at->Delete = AnimDelete;
    at->Print = AnimPrint;
    at->Load = AnimLoad;
    at->Fixup = AnimFixup;
    at->Store = AnimStore;
    at->EnumerateReferences = AnimEnumerateReferences;
    at->Draw = AnimDraw;
    at->TraverseVertices = AnimTraverseVertices;
    at->IsectNode = AnimIsectNode;
    at->IsectSphere = AnimIsectSphere;
    at->IsectSeg = AnimIsectSeg;

    return ;
}

void _atsAnimInitClass(void)
{
    atsAnimNewType(_ats_anim_type);
    _ats_anim_type->name = "Anim";
    _ats_anim_type->size = sizeof(AtsAnim);
    _ats_anim_type->parent = (AtsType *)_ats_group_type;
}
