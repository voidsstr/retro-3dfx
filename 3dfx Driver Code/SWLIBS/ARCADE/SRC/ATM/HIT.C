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
** $Date: 10/11/00 7:33:42 PM$ 
**
*/

#include <float.h>
#include <atutil.h>
#include <atmath.h>

AtmHit *atmHitListAllocate(int n)
{
    return (AtmHit *)atuMemCalloc(1, n*sizeof(AtmHit));
}

FxU32 atmHitGetStatus(AtmHit *hit) {
    return hit->status;
}

FxU32 atmHitGetMask(AtmHit *hit) {
    return hit->mask;
}

FxBool atmHitGetSeg(AtmHit *hit, AtmSeg *seg) {
    if ( hit->status & ATB_HIT_SEG ) {
        *seg = hit->seg;
        return FXTRUE;
    } else return FXFALSE;
}

FxBool atmHitGetPoint(AtmHit *hit, AtmVector3 pt) {
    if ( hit->status & ATB_HIT_POINT ) {
        ATM_VEC3_COPY(pt, hit->point);
        return FXTRUE;
    } else return FXFALSE;
}

FxBool atmHitGetNormal(AtmHit *hit, AtmVector3 normal) {
    if ( hit->status & ATB_HIT_POINT ) {
        ATM_VEC3_COPY(normal, hit->normal);
        return FXTRUE;
    } else return FXFALSE;
}

FxBool atmHitGetTriangle(AtmHit *hit, 
                         AtmVector3 pt0, AtmVector3 pt1, AtmVector3 pt2) {
    if ( hit->status & ATB_HIT_VERTS ) {
        ATM_VEC3_COPY(pt0, hit->v[0]);
        ATM_VEC3_COPY(pt1, hit->v[1]);
        ATM_VEC3_COPY(pt2, hit->v[2]);
    } else return FXFALSE;

    return FXTRUE;
}

void atmHitListFree(AtmHit *hits) {
    atuMemFree(hits);
}
