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
** TBD: Need to add in more matrix support similar to D3D retained mode,
**      This should be done in the math library and then just used here.
**      It's not clear that AtrXform really adds value here, perhaps it
**      should just be a AtmMatrix4x4, I've done it this way for now 
**      to match cleanly with the rendering layer.
**      Also need to add lookat function to matrix library
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

static AtsType frame_type;
AtsType *_ats_frame_type = &frame_type;
static const FxU32 _binaryRevision = 1;

/*-------------------------------------------------------------------
  Function: atsFrameNew
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a new frame group node
  Arguments:
    None
  Return:
    a pointer to the new frame group
  -------------------------------------------------------------------*/

AtsNode* _atsFrameNew(char *where, FxU32 line) {
    return _atsNew(_ats_frame_type, where, line);
}

/*-------------------------------------------------------------------
  Function: atsFrameGetType
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns a pointer to the frame's type
  Arguments:
    None
  Return:
    pointer to frames type 
  -------------------------------------------------------------------*/

AtsType* atsFrameGetType(void) {
    return (AtsType *)_ats_frame_type;
}

/*-------------------------------------------------------------------
  Function: atsFrameXform
  Date: 5/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set a frame's transform
  Arguments:
    node  - the node
    xform - the transformation
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsFrameXform(AtsNode *node, AtrXform *xform) {
    AtsFrame *frame ;

    VALIDATE_FRAME(frame, node, "atsFrameGetMatrix");

    atrXformAssign( &frame->xform, xform);
}

/*-------------------------------------------------------------------
  Function: atsFrameGetXform
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get a frame's transform
  Arguments:
    node  - the node
  Return:
    pointer to frames transform 
  -------------------------------------------------------------------*/

AtrXform* atsFrameGetXform(AtsNode *node) {
    AtsFrame *frame ;

    VALIDATE_FRAME(frame, node, "atsFrameGetMatrix");

    return &( frame->xform );
}

/*-------------------------------------------------------------------
  Function: atsFrameStatic
  Date: 5/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set a frame's transform as being static
  Arguments:
    node   - the node
    static - is frames transform static
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsFrameStatic(AtsNode *node, FxBool is_static) {
    AtsFrame *frame ;

    VALIDATE_FRAME(frame, node, "atsFrameGetMatrix");

    if ( is_static )
         frame->flags |= ATS_TRANS_STATIC ;
    else frame->flags &= ~ATS_TRANS_STATIC ;

}

/*-------------------------------------------------------------------
  Function: atsFrameXform
  Date: 5/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Determine if transform is static
  Arguments:
    node  - the node
  Return:
    FXTRUE if frame is static, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool atsFrameGetStatic(AtsNode *node) {
    AtsFrame *frame ;

    VALIDATE_FRAME(frame, node, "atsFrameGetMatrix");

    return (( frame->flags & ATS_TRANS_STATIC ) != 0);
}

static void FrameInit(AtsObject *node)
{
    AtsFrame *frame ;

    VALIDATE_FRAME(frame, node, "FrameInit");

    ATS_PARENT_CALL(_ats_frame_type, Init)(node);

    atrXformSetIdentity( &frame->xform );
    frame->flags = 0 ;
}

static AtsNode *FrameClone(AtsObject* node, FxU32 mode) {
    AtsFrame *gsrc, *gdst ;
    AtsNode *ndst;

    VALIDATE_FRAME(gsrc, node, "FrameClone");

    ndst = ATS_PARENT_CALL(_ats_frame_type, Clone)(node, mode);

    assert(ndst);

    gdst = (AtsFrame *)ndst;

    gdst->xform = gsrc->xform;

    return ndst;
}

static FxBool FramePrint(const AtsObject* node, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    AtsFrame *frame ;

    VALIDATE_FRAME(frame, node, "FramePrint");

    ATS_PARENT_CALL(_ats_frame_type, Print)(node, stream, indent, verbose);

    atrXformPrint( &frame->xform, stream, indent+4 );

    return FXTRUE;
}

static FxBool FrameLoad(AtsObject *obj, FILE *stream){
    AtsFrame *frame ;
    FxU32 version;

    VALIDATE_FRAME(frame, obj, "FrameLoad");

    if (!ATS_PARENT_CALL(_ats_frame_type, Load)(obj, stream))
        return FXFALSE;
    
    CHECK( atuRead32( &version, 1, stream ));

    CHECK( version == _binaryRevision );

    CHECK( atuRead32( &frame->flags, 1, stream ));

    CHECK( atuRead32( &frame->xform, sizeof( frame->xform )>>2, stream ));

    return FXTRUE;
}

static FxBool FrameStore(AtsObject *obj, FILE *stream) {
    AtsFrame *frame ;

    VALIDATE_FRAME(frame, obj, "FrameStore");

    if (!ATS_PARENT_CALL(_ats_frame_type, Store)(obj, stream))
        return FXFALSE;
    
    CHECK( atuWrite32( &_binaryRevision, 1, stream ));

    CHECK( atuWrite32( &frame->flags, 1, stream ));

    CHECK( atuWrite32( &frame->xform, sizeof( frame->xform )>>2, stream ));

    return FXTRUE;
}

static FxBool FrameIsectNode(const AtsNode *node, const AtsNode *other) {
    AtsFrame *frame ;
    FxBool status;

    VALIDATE_FRAME(frame, node, "FrameIsectNode");

    atsPreCatPushXform( &(frame->xform) );
    
    status = ATS_PARENT_CALL(_ats_frame_type, IsectNode)(node, other);

    atsPopXform();
  
    return status;
}

static FxBool FrameIsectSphere(const AtsNode *node, const AtmSphere *bsphere) {
    AtsFrame *frame ;
    FxBool status;

    VALIDATE_FRAME(frame, node, "FrameIsectSphere");

    atsPreCatPushXform( &(frame->xform) );
    
    status = ATS_PARENT_CALL(_ats_frame_type, IsectSphere)(node, bsphere);

    atsPopXform();
  
    return status;
}

static void FrameIsectSeg(const AtsNode *node, AtmIsect *isect) {
    AtsFrame *frame ;

    VALIDATE_FRAME(frame, node, "FrameIsectSeg");

    atsPreCatPushXform( &(frame->xform) );
    
    ATS_PARENT_CALL(_ats_frame_type, IsectSeg)(node, isect);

    atsPopXform();
}

static void FrameDraw(AtsObject *node, FxU32 mask) {
    AtsFrame *frame ;

    VALIDATE_FRAME(frame, node, "FrameDraw");

    if ( ( frame->group.node.mask & mask ) == 0 )
        return;

    atrPreCatPushXform( &(frame->xform) );
    
    ATS_PARENT_CALL(_ats_frame_type, Draw)(node, mask);

    atrPopXform(FXTRUE);
}

void atsFrameNewType(AtsType *ft) {
    atsGroupNewType(ft);

    /* set default methods */
           
    ft->Init = FrameInit;
    ft->Clone = FrameClone;
    ft->Print = FramePrint;
    ft->Load = FrameLoad;
    ft->Store = FrameStore;
    ft->Draw = FrameDraw;
    ft->IsectNode = FrameIsectNode;
    ft->IsectSphere = FrameIsectSphere;
    ft->IsectSeg = FrameIsectSeg;

    return ;
}

void _atsFrameInitClass(void)
{
    atsFrameNewType(_ats_frame_type);
    _ats_frame_type->name = "Frame";
    _ats_frame_type->size = sizeof(AtsFrame);
    _ats_frame_type->parent = (AtsType *)_ats_group_type;
}
