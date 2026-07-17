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
** $Date: 10/11/00 7:34:07 PM$ 
**
*/

#include "atrender.h"
#include "fxatr.h"

/*-------------------------------------------------------------------
  Function: _atrIRGBSRC_LIGHTING
  Date: 5/30/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Do all lighting computations
  Arguments:
    dest - destination vertex
    src - src vertex
    end - end condition vertex 
  Return:
    none
  -------------------------------------------------------------------*/
void _atrIRGBSRC_LIGHTING( AtrDstVertex dest[],
                           AtrVertex src[],
                           AtrVertex *end ) {
    _AtrLightNode *l = _atrLightHead;
    FxU32 offset = ((_atrRenderMaterial->sysFlags & ATR_LIGHTFUNC_MASK)
                    >> ATR_LIGHTFUNC_SHIFT) - 
                   1;

    while( l ) {
        l->lf[offset]( dest, src, end, l );
        l = l->next;
    }
}


/*-------------------------------------------------------------------
  Function: _atrIRGBSRC_STATIC
  Date: 4/2/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Implementation of lighting irgbsrc callback    
  Arguments:
    dest - destination vertex
    src  - source vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrIRGBSRC_STATIC( AtrDstVertex *dest, 
                         AtrVertex *src,
                         AtrVertex *end ) {
    while( src < end ) {
        dest->r = src->r * _atrTwoFiftyFive;
        dest->g = src->g * _atrTwoFiftyFive;
        dest->b = src->b * _atrTwoFiftyFive;
        dest++;
        src++;
    }
    return;
}

/*-------------------------------------------------------------------
  Function: _atrIRGBSRC_STATIC
  Date: 4/2/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Implementation of lighting irgbsrc callback    
  Arguments:
    dest - destination vertex
    src  - source vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrIASRC_STATIC( AtrDstVertex *dest, 
                       AtrVertex *src,
                       AtrVertex *end ) {
    while( src < end ) {
        dest->a = src->a * _atrTwoFiftyFive;
        dest++;
        src++;
    }
    return;
}
