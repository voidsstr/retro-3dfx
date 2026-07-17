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
** $Date: 10/11/00 7:35:28 PM$ 
**
*/

#ifndef RGLIDE_H
#define RGLIDE_H
#include "atrender.h"
#include "fxatr.h"

void _atrGlideClipAndRenderTri( AtrDstVertex *a, AtrDstVertex *b,
                           AtrDstVertex *c, FxU32 clipCode );

void _atrGlideClipAndRenderTriWF( AtrDstVertex *a, AtrDstVertex *b,
                             AtrDstVertex *c, FxU32 clipCode );

void _atrGlideClipAndRenderSegment(AtrDstVertex *a, AtrDstVertex *b,
                              FxU32 clipCode );

void _atrGlideUpdateMaterial( AtrMaterial *m );

FxU32 CTransformVertices( AtrDstVertex *dest, AtrVertex *src,
                                  AtrVertex *end );
void _atrGlideSpecialTransformVertices( AtrDstVertex *dest, AtrVertex *src,
                                        AtrVertex *end );
void _atrGlideTransformVertices2D( AtrDstVertex *dest, AtrVertex *src,
                                   AtrVertex *end );
void _atrGlideRenderTri( AtrVertex *a, AtrVertex *b, AtrVertex *c );
void _atrGlideRenderTriSet( AtrTriSet *t );
void _atrGlideSpecialRenderTriSet( AtrTriSet *t );
void _atrGlideRenderTriWF( AtrVertex *a, AtrVertex *b, AtrVertex *c );
void _atrGlideRenderTriSetWF( AtrTriSet *t );
void _atrGlideTransformOpenTriSet( AtrOpenTriSet *t );
void _atrGlideSpecialTransformOpenTriSet( AtrOpenTriSet *t );
void _atrGlideRenderOpenTriSet( AtrOpenTriSet *t );
void _atrGlideRenderOpenTriSetWF( AtrOpenTriSet *t );
void _atrGlideRenderSegment( AtrVertex *a, AtrVertex *b );
void _atrGlideRender2DTri( AtrVertex *a, AtrVertex *b, AtrVertex *c );

extern AtrVertexFunc *_atrSpecialVertexCallback;

#endif
