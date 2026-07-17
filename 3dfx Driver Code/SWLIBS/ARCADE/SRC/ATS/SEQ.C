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
** $Date: 10/11/00 7:34:43 PM$ 
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <atscenep.h>

static AtsType seq_type;
AtsType *_ats_seq_type = &seq_type;
static const FxU32 _binaryRevision = 1;

/*-------------------------------------------------------------------
  Function: atsSeqNew
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Create a new sequence group node
  Arguments:
    None
  Return:
    a pointer to the new sequence group
  -------------------------------------------------------------------*/

AtsNode* _atsSeqNew(char *where, FxU32 line) {
    return _atsNew(_ats_seq_type, where, line);
}

/*-------------------------------------------------------------------
  Function: atsSeqGetType
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Returns a pointer to the seq's type
  Arguments:
    None
  Return:
    pointer to seqs type 
  -------------------------------------------------------------------*/

AtsType* atsSeqGetType(void) {
    return (AtsType *)_ats_seq_type;
}

/*-------------------------------------------------------------------
  Function: atsSeqMode
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set a seq's mode
  Arguments:
    node  - the node
    mode  - the new mode
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsSeqMode(AtsNode* node, int mode) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "atsSeqMode");

#ifdef AT_DEBUGGING
    if (( mode != ATS_SEQ_CYCLE ) && ( mode != ATS_SEQ_SWING ))
        atuError( FXTRUE, "atsSeqMode(): Invalid mode %d.\n", mode );
#endif    
    seq->mode = mode;
}

/*-------------------------------------------------------------------
  Function: atsSeqGetMode
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    find a seq's mode
  Arguments:
    node  - the node
  Return:
    The sequences mode
  -------------------------------------------------------------------*/

int atsSeqGetMode(const AtsNode* node) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "atsSeqGetMode");

    return seq->mode ;
}

/*-------------------------------------------------------------------
  Function: atsSeqDuration
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set a seq's duration (time to display entire sequence)
  Arguments:
    node     - the node
    duration - time for sequence
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsSeqDuration(AtsNode* node, float duration) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "atsSeqDuration");

    seq->duration = duration;
}

/*-------------------------------------------------------------------
  Function: atsSeqGetDuration
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get a seq's duration
  Arguments:
    node  - the node
  Return:
    The time taken to display all nodes in sequence
  -------------------------------------------------------------------*/

float atsSeqGetDuration(const AtsNode *node) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "atsSeqGetDuration");

    return seq->duration ;
}

/*-------------------------------------------------------------------
  Function: atsSeqOffset
  Date: 7/11/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set a seq's offset (starting position in sequence)
  Arguments:
    node     - the node
    offset   - offset time for sequence
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsSeqOffset(AtsNode* node, float offset) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "atsSeqOffset");

    seq->offset = offset;
}

/*-------------------------------------------------------------------
  Function: atsSeqGetOffset
  Date: 7/11/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get a seq's offset
  Arguments:
    node  - the node
  Return:
    The amount to offset the cycle by
  -------------------------------------------------------------------*/

float atsSeqGetOffset(const AtsNode *node) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "atsSeqGetOffset");

    return seq->offset ;
}

/*-------------------------------------------------------------------
  Function: atsSeqControl
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Control the display of a sequnce
  Arguments:
    node    - the node
    numReps - number of repetions to display (<= 0 unlimited)
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsSeqControl(AtsNode* node, int action) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "atsSeqControl");

    switch ( action ) {
    case ATS_SEQ_STOP:
        seq->state = ATS_SEQ_STOPPED;
        break;
    case ATS_SEQ_START:
        seq->state = ATS_SEQ_RUNNING;
        seq->startTime = atsGetFrameTime();
        seq->framesRendered = 0;
        break;
    case ATS_SEQ_PAUSE:
        seq->state = ATS_SEQ_PAUSED;
        seq->startTime -= atsGetFrameTime(); 
        break;
    case ATS_SEQ_RESUME:
        if ( seq->state == ATS_SEQ_PAUSED ) {
            seq->state = ATS_SEQ_RUNNING;
            seq->startTime += atsGetFrameTime(); 
        }
        break;
    default:
        atuError(FXFALSE, "atsSeqControl: unknown action %d\n", action);
    }
}

/*-------------------------------------------------------------------
  Function: atsSeqReps
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set the number of times to display a seq
  Arguments:
    node    - the node
    numReps - number of repetions to display (<= 0 unlimited)
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsSeqReps(AtsNode* node, int numReps) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "atsSeqReps");

    seq->numReps = numReps;
}

/*-------------------------------------------------------------------
  Function: atsSeqGetReps
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Get number of times to display a sequence
  Arguments:
    node  - the node
  Return:
    Number of times to display sequence
  -------------------------------------------------------------------*/

int atsSeqGetReps(const AtsNode* node) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "atsSeqGetReps");

    return seq->numReps;
}

/*-------------------------------------------------------------------
  Function: atsSeqBasis
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Specify whether the sequence is time or frame based
  Arguments:
    node  - the node
    basis - sequence basis
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsSeqBasis(AtsNode *node, int basis) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "atsSeqGetReps");

#ifdef AT_DEBUGGING
    if ((basis != ATS_SEQ_TIME ) && ( basis != ATS_SEQ_FRAME ))
        atuError(FXTRUE, "atsSeqBasis: invalid basis %d\n", basis);
#endif

    seq->basis = basis;
}

/*-------------------------------------------------------------------
  Function: atsSeqGetBasis
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Inquire whether the sequence is time or frame based
  Arguments:
    node  - the node
  Return:
    The sequences basis
  -------------------------------------------------------------------*/

int  atsSeqGetBasis(AtsNode *node) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "atsSeqGetReps");

    return seq->basis;
}

/*-------------------------------------------------------------------
  Function: atsSeqGetFrame
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Determine which child (if any to display)
  Arguments:
    seq  - the sequence
  Return:
    Child node to display or NULL if don't display any
  -------------------------------------------------------------------*/

static AtsNode *SeqGetFrame(AtsSeq *seq) {
    int i;
    AtsGroup *group = (AtsGroup *)seq;
    float delta_t  = 0.0f;

    if ( seq->basis == ATS_SEQ_TIME ) {
        delta_t = atsGetFrameTime() - ( seq->startTime + seq->offset );
        if (( seq->state != ATS_SEQ_RUNNING ) || ( group->num_children == 0 ) ||
            ((seq->numReps > 0 ) && ( delta_t >= seq->numReps*seq->duration )))
            return NULL; /* nothing to display */
    
        if ( seq->mode == ATS_SEQ_CYCLE ) {
            i = (int)((delta_t/seq->duration )*group->num_children);
            i = i % group->num_children;
        } else { /* swing */
            int numFrames = group->num_children-2;
            i = (int)((delta_t/seq->duration )*numFrames);
            if ( i >= group->num_children )
                i = 2*group->num_children-i;
        }
    } else { /* frame based */
        if (( seq->state != ATS_SEQ_RUNNING ) || ( group->num_children == 0 ))
            return NULL; /* nothing to display */
        if ( seq->numReps != 0 ) {
            if ( seq->mode == ATS_SEQ_CYCLE ) {
                if ( seq->framesRendered >= seq->numReps*group->num_children )
                    return NULL; /* nothing to display */
            } else { /* swing */
                if ( seq->framesRendered >= (2*seq->numReps*group->num_children-2)){
                    return NULL; /* nothing to display */
                }
            }
        }

        if ( seq->mode == ATS_SEQ_CYCLE ) {
            i = (int)((delta_t/seq->duration )*group->num_children);
            i = seq->framesRendered % group->num_children;
        } else { /* swing */
            int numFrames = group->num_children-2;
            i = seq->framesRendered % (2*group->num_children-2);
            if ( i >= group->num_children )
                i = 2*group->num_children-2-i;
        }
    }
    
    return group->children[i];
}

/*-------------------------------------------------------------------
  Function: atsSeqNext
  Date: 7/31/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Advance to the next frame in the animation
  Arguments:
    seq  - the sequence
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsSeqNext(AtsNode *node) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "atsSeqGetReps");

    seq->framesRendered++;
}

static void SeqInit(AtsObject *node) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "SeqInit");

    ATS_PARENT_CALL(_ats_seq_type, Init)(node);

    seq->state = ATS_SEQ_RUNNING;
    seq->basis = ATS_SEQ_TIME;
    seq->numReps = 0;
    seq->framesRendered = 0;
    seq->mode = ATS_SEQ_CYCLE;
    seq->startTime = 0.0f;
    seq->duration = 1.0f;
}

static AtsNode *SeqClone(AtsObject* node, FxU32 mode) {
    AtsSeq *gsrc, *gdst ;
    AtsNode *ndst;

    VALIDATE_SEQ(gsrc, node, "SeqClone");

    ndst = ATS_PARENT_CALL(_ats_seq_type, Clone)(node, mode);

    assert(ndst);

    gdst = (AtsSeq *)ndst;

    gdst->state = gsrc->state ;
    gdst->basis = gsrc->basis ;
    gdst->numReps = gsrc->numReps ;
    gdst->framesRendered = gsrc->framesRendered ;
    gdst->mode = gsrc->mode ;
    gdst->startTime = gsrc->startTime ;
    gdst->duration = gsrc->duration ;

    return ndst;
}

static FxBool SeqPrint(const AtsObject* node, FILE *stream, 
                               FxU32 indent, FxU32 verbose) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, node, "SeqPrint");

    ATS_PARENT_CALL(_ats_seq_type, Print)(node, stream, indent, verbose);
    fprintf( stream, "%*smode %d\n", indent+4, " ", seq->mode);
    fprintf( stream, "%*sbasis %d\n", indent+4, " ", seq->basis);
    fprintf( stream, "%*sduration %d\n", indent+4, " ", seq->duration);
    fprintf( stream, "%*snum reps %d\n", indent+4, " ", seq->numReps);

    return FXTRUE;
}

static FxBool SeqLoad(AtsObject *obj, FILE *stream){
    AtsSeq *seq ;
    FxU32 version;

    VALIDATE_SEQ(seq, obj, "SeqLoad");

    if (!ATS_PARENT_CALL(_ats_seq_type, Load)(obj, stream))
        return FXFALSE;
    
    CHECK( atuRead32( &version, 1, stream ));

    CHECK( version == _binaryRevision );

    CHECK( atuRead32( &seq->mode, 1, stream ));

    CHECK( atuRead32( &seq->basis, 1, stream ));

    CHECK( atuRead32( &seq->duration, 1, stream ));

    CHECK( atuRead32( &seq->offset, 1, stream ));

    CHECK( atuRead32( &seq->numReps, 1, stream ));

    seq->state = ATS_SEQ_RUNNING;

    seq->startTime = 0.0f;

    return FXTRUE;
}

static FxBool SeqStore(AtsObject *obj, FILE *stream) {
    AtsSeq *seq ;

    VALIDATE_SEQ(seq, obj, "SeqStore");

    if (!ATS_PARENT_CALL(_ats_seq_type, Store)(obj, stream))
        return FXFALSE;
    
    CHECK( atuWrite32( &_binaryRevision, 1, stream ));

    CHECK( atuWrite32( &seq->mode, 1, stream ));

    CHECK( atuWrite32( &seq->basis, 1, stream ));

    CHECK( atuWrite32( &seq->duration, 1, stream ));

    CHECK( atuWrite32( &seq->offset, 1, stream ));

    CHECK( atuWrite32( &seq->numReps, 1, stream ));

    return FXTRUE;
}

static FxBool SeqIsectNode(const AtsNode *node, const AtsNode *other) {
    AtsSeq *seq ;
    FxBool status;
    AtsNode *c;

    VALIDATE_SEQ(seq, node, "SeqIsectNode");

    if (( c= SeqGetFrame(seq)) != NULL ) {
        status = ATS_NODE_CALL(c, IsectNode)(c, other);
    } else status = FXFALSE;

    return status;
}

static FxBool SeqIsectSphere(const AtsNode *node, const AtmSphere *bsphere) {
    AtsSeq *seq ;
    FxBool status;
    AtsNode *c;

    VALIDATE_SEQ(seq, node, "SeqIsectSphere");

    if (( c= SeqGetFrame(seq)) != NULL ) {
         status = ATS_NODE_CALL(c, IsectSphere)(c, bsphere);
    } else status = FXFALSE;

    return status;
}

static void SeqIsectSeg(const AtsNode *node, AtmIsect *isect) {
    AtsSeq *seq ;
    AtsNode *c;

    VALIDATE_SEQ(seq, node, "SeqIsectSeg");

    if (( c= SeqGetFrame(seq)) != NULL ) {
        ATS_NODE_CALL(c, IsectSeg)(c, isect);
    }
}

static void SeqDraw(AtsObject *node, FxU32 mask) {
    AtsSeq *seq ;
    AtsNode *c;

    VALIDATE_SEQ(seq, node, "SeqDraw");

    if ( ( seq->group.node.mask & mask ) == 0 )
        return;

    if (( c= SeqGetFrame(seq)) != NULL ) {
        ATS_NODE_CALL(c, Draw)(c, mask);
    }
}

void atsSeqNewType(AtsType *ft) {
    atsGroupNewType(ft);

    /* set default methods */
           
    ft->Init = SeqInit;
    ft->Clone = SeqClone;
    ft->Print = SeqPrint;
    ft->Load = SeqLoad;
    ft->Store = SeqStore;
    ft->Draw = SeqDraw;
    ft->IsectNode = SeqIsectNode;
    ft->IsectSphere = SeqIsectSphere;
    ft->IsectSeg = SeqIsectSeg;

    return ;
}

void _atsSeqInitClass(void) {
    atsSeqNewType(_ats_seq_type);
    _ats_seq_type->name = "Seq";
    _ats_seq_type->size = sizeof(AtsSeq);
    _ats_seq_type->parent = (AtsType *)_ats_group_type;
}
