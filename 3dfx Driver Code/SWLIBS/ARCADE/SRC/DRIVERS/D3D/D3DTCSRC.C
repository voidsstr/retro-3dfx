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
** $Date: 10/11/00 7:35:06 PM$ 
**
*/

#include "atrender.h"
#include "fxatr.h"

/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC0_2DTC0
  Date: 4/2/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Implementation of texture coordinate source function callback
  Arguments:
    dest - destination vertex
    src  - source vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC0_2DTC0(AtrDstVertex *dest,           
                      AtrVertex *src,
                      AtrVertex *end ) {
    while( src < end ) {                        
        dest->s0   = src->s0 ;
        dest->t0   = src->t0 ;
        dest->oow0 = dest->oow;
        dest++;
        src++;
    }
    return;
}


/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC0_TC0
  Date: 4/2/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Implementation of texture coordinate source function callback
  Arguments:
    dest - destination vertex
    src  - source vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC0_TC0( AtrDstVertex *dest,           
                     AtrVertex *src,
                     AtrVertex *end ) {
    while( src < end ) {                        
        if ( dest->flags ) {
            dest->s0   = src->s0 ;
            dest->t0   = src->t0 ;
            dest->oow0 = 1.0f;
        } else {
            dest->s0   = src->s0 ;
            dest->t0   = src->t0 ;
            dest->oow0 = dest->oow;
        }
        dest++;
        src++;
    }
    return;
}


/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC1_TC0
  Date: 4/2/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Implementation of texture coordinate source function callback
  Arguments:
    dest - destination vertex
    src  - source vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC1_TC0( AtrDstVertex *dest,           
                    AtrVertex *src,
                    AtrVertex *end ) {
    while( src < end ) {                        
        if ( dest->flags ) {
            dest->s1   = src->s0 ;
            dest->t1   = src->t0 ;
            dest->oow1 = 1.0f;
        } else {
            dest->s1 = src->s0 ;
            dest->t1 = src->t0 ;
            dest->oow0 = dest->oow;
        }
        dest++;
        src++;
    }
    return;
}

/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC0_TC1
  Date: 4/2/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Implementation of texture coordinate source function callback
  Arguments:
    dest - destination vertex
    src  - source vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC0_TC1( AtrDstVertex *dest, 
                    AtrVertex *src,
                    AtrVertex *end ) {
    while( src < end ) {
        if ( dest->flags ) {
            dest->s0   = src->s1 ;
            dest->t0   = src->t1 ;
            dest->oow0 = 1.0f;
        } else {
            dest->s0   = src->s1 ;
            dest->t0   = src->t1 ;
            dest->oow0 = dest->oow;
        }   
        src++;
        dest++;
    }
    return;
}

/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC1_TC1
  Date: 4/2/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Implementation of texture coordinate source function callback
  Arguments:
    dest - destination vertex
    src  - source vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC1_TC1( AtrDstVertex *dest, 
                    AtrVertex *src,
                    AtrVertex *end ) {
    while( src < end ) {
        if ( dest->flags ) {
            dest->s1   = src->s1 ;
            dest->t1   = src->t1 ;
            dest->oow1 = 1.0f;
        } else {
            dest->s1   = src->s1 ;
            dest->t1   = src->t1 ;
            dest->oow1 = dest->oow;
        }   
        src++;
        dest++;
    }
    return;
}

/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC0_TC2
  Date: 4/2/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Implementation of texture coordinate source function callback
  Arguments:
    dest - destination vertex
    src  - source vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC0_TC2( AtrDstVertex *dest, 
                    AtrVertex *src,
                    AtrVertex *end ) {
    while( src < end ) {
        if ( dest->flags ) {
            dest->s0   = src->s2 ;
            dest->t0   = src->t2 ;
            dest->oow0 = 1.0f;
        } else {
            dest->s0   = src->s2 ;
            dest->t0   = src->t2 ;
            dest->oow0 = dest->oow;
        }
        src++;
        dest++;
    }
    return;
}

/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC1_TC2
  Date: 4/2/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Implementation of texture coordinate source function callback
  Arguments:
    dest - destination vertex
    src  - source vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC1_TC2( AtrDstVertex *dest, 
                    AtrVertex *src,
                    AtrVertex *end ) {
    while( src < end ) {
        if ( dest->flags ) {
            dest->s1   = src->s2 ;
            dest->t1   = src->t2 ;
            dest->oow1 = 1.0f;
        } else {
            dest->s1   = src->s2 ;
            dest->t1   = src->t2 ;
            dest->oow1 = dest->oow;
        }
        src++;
        dest++;
    }
    return;
}


/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC0_EMAP
  Date: 3/26/97
  Implementor(s): jdt
  Library: AT Render
  Description:
    Do the environment mapping calculation for one vertex.
  Arguments:
    src - src vertex
    dest - destination vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC0_EMAP( AtrDstVertex *dest, 
                      AtrVertex *src,
                      AtrVertex *end ) {
    AtmVector3 normal, u, r, tmp;
    AtrXform lcsToVCS, wcsToVCS;
    float dot, oom;
	float half, sScale, tScale;

    atrXformInvert( &wcsToVCS, &_atrCurrentCamera->lcsToWCS );
    atrXformCat( &lcsToVCS, _atrCurrentXform, &wcsToVCS );

	/* get scaling factor for texture coordinates
	 N.B. we assume square textures for environment mapping */

	_atrTexGetScale( 0, &sScale, &tScale );
	half = sScale * 0.5f;

    while( src < end ) {
        tmp[0] = src->x * lcsToVCS.data[0] +
                 src->y * lcsToVCS.data[4] +
                 src->z * lcsToVCS.data[8] +
                          lcsToVCS.data[12];
        tmp[1] = src->x * lcsToVCS.data[1] +
                 src->y * lcsToVCS.data[5] +
                 src->z * lcsToVCS.data[9] +
                          lcsToVCS.data[13];
        tmp[2] = src->x * lcsToVCS.data[2] +
                 src->y * lcsToVCS.data[6] +
                 src->z * lcsToVCS.data[10] +
                          lcsToVCS.data[14];
    
        if ( dest->flags == 0 ) {
            dest->oow0 = dest->oow;
        } else {
            dest->oow0 = 1.0f;
        }
    
        normal[0] = src->i * lcsToVCS.data[0] +
                    src->j * lcsToVCS.data[4] +
                    src->k * lcsToVCS.data[8];
        normal[1] = src->i * lcsToVCS.data[1] +
                    src->j * lcsToVCS.data[5] +
                    src->k * lcsToVCS.data[9];
        normal[2] = src->i * lcsToVCS.data[2] +
                    src->j * lcsToVCS.data[6] +
                    src->k * lcsToVCS.data[10];
    
        atmVector3Normalize( u, tmp );

        dot = normal[0] * u[0] + normal[1] * u[1] + normal[2] * u[2];
        dot *= 2.0f;
        r[0] = u[0] - dot * normal[0];
        r[1] = u[1] - dot * normal[1];
        r[2] = u[2] - dot * normal[2];

        r[2] += 1.0f;

        oom = half / atmOOSqrt( r[0] * r[0] + 
                                  r[1] * r[1] +
                                  r[2] * r[2] );
        dest->s0 = (r[0] *  oom + half);
        dest->t0 = (r[1] * -oom + half);
        dest++;
        src++;
    }
    return;
}


/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC1_EMAP
  Date: 3/26/97
  Implementor(s): jdt
  Library: AT Render
  Description:
    Do the environment mapping calculation for one vertex.
  Arguments:
    src - src vertex
    dest - destination vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC1_EMAP( AtrDstVertex *dest, 
                      AtrVertex *src,
                      AtrVertex *end ) {
    AtmVector3 normal, u, r, tmp;
    AtrXform lcsToVCS, wcsToVCS;
    float dot, oom;
	float half, sScale, tScale;

    atrXformInvert( &wcsToVCS, &_atrCurrentCamera->lcsToWCS );
    atrXformCat( &lcsToVCS, _atrCurrentXform, &wcsToVCS );

	/* get scaling factor for texture coordinates
	 N.B. we assume square textures for environment mapping */

	_atrTexGetScale( 1, &sScale, &tScale );
	half = sScale * 0.5f;

    while( src < end ) {
        tmp[0] = src->x * lcsToVCS.data[0] +
                 src->y * lcsToVCS.data[4] +
                 src->z * lcsToVCS.data[8] +
                          lcsToVCS.data[12];
        tmp[1] = src->x * lcsToVCS.data[1] +
                 src->y * lcsToVCS.data[5] +
                 src->z * lcsToVCS.data[9] +
                          lcsToVCS.data[13];
        tmp[2] = src->x * lcsToVCS.data[2] +
                 src->y * lcsToVCS.data[6] +
                 src->z * lcsToVCS.data[10] +
                          lcsToVCS.data[14];
    
        if ( dest->flags == 0 ) {
            dest->oow1 = dest->oow;
        } else {
            dest->oow1 = 1.0f;
        }
    
        normal[0] = src->i * lcsToVCS.data[0] +
                    src->j * lcsToVCS.data[4] +
                    src->k * lcsToVCS.data[8];
        normal[1] = src->i * lcsToVCS.data[1] +
                    src->j * lcsToVCS.data[5] +
                    src->k * lcsToVCS.data[9];
        normal[2] = src->i * lcsToVCS.data[2] +
                    src->j * lcsToVCS.data[6] +
                    src->k * lcsToVCS.data[10];
    
        atmVector3Normalize( u, tmp );

        dot = normal[0] * u[0] + normal[1] * u[1] + normal[2] * u[2];
        dot *= 2.0f;
        r[0] = u[0] - dot * normal[0];
        r[1] = u[1] - dot * normal[1];
        r[2] = u[2] - dot * normal[2];

        r[2] += 1.0f;

        oom = half / atmOOSqrt( r[0] * r[0] + 
                                  r[1] * r[1] +
                                  r[2] * r[2] );
        dest->s1 = (r[0] *  oom + half);
        dest->t1 = (r[1] * -oom + half);
        dest++;
        src++;
    }
    return;
}


/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC0_PROJECTED
  Date: 7/4/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Do the projected texture calculation
  Arguments:
    src - src vertex
    dest - destination vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC0_PROJECTED(AtrDstVertex *dest, 
                          AtrVertex    *src,
                          AtrVertex    *end ) {
    _AtrLightNode *n = _atrLightHead;
    FxU32 tag        = _atrRenderMaterial->projectedTag;
    float        vpS = 0.5f;
    float        vpT = 0.5f;

    /*--------------------------------------------
      Find the projector associated with the current
      material
      --------------------------------------------*/
    while( n ) {
        if (n->light.flags == ATR_LIGHT_PROJECTOR &&
            n->light.project.tag == tag ) break; 
            n = n->next;
    }

#ifdef AT_DEBUGGING
    if ( !n )
      atuError( FXTRUE, 
                "atrRender*(): Error, no light matches tag %d.\n",
                tag );
#else
    if ( !n ) return;
#endif

    /*--------------------------------------------
      For each vertex, transform into light pov and
      scale to projected texture dimensions.
      --------------------------------------------*/
    while( src < end ) {
        float ooq;

        /* project s, t, w */
        dest->s0 = 
        ( src->x * n->modelToProjector.data[0] +
          src->y * n->modelToProjector.data[4] +
          src->z * n->modelToProjector.data[8] +
          n->modelToProjector.data[12] ) * vpS;
        dest->t0 = 
        -1.0f * ( src->x * n->modelToProjector.data[1] +
          src->y * n->modelToProjector.data[5] +
          src->z * n->modelToProjector.data[9] +
          n->modelToProjector.data[13] ) * vpT;
        dest->oow0 = 
        src->x * n->modelToProjector.data[2] +
        src->y * n->modelToProjector.data[6] +
        src->z * n->modelToProjector.data[10] +
        n->modelToProjector.data[14];

        /* Do something to avoid dbz ( right thing? ) */
        if ( dest->oow0 == 0.0f ) {
            dest->oow0 = 0.001f;
        } 
        ooq = 1.0f / dest->oow0;

        /* project onto plane */
        dest->s0 = dest->s0 * ooq + vpS;
        dest->t0 = dest->t0 * ooq + vpT;

        /* prepare for tmapping */
        if ( dest->flags == 0 ) {
            dest->oow0 *= dest->oow;
        } else {
            dest->s0   *= dest->oow0;
            dest->t0   *= dest->oow0;
        }

        dest++;
        src++;
    }
    return;
}

/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC1_PROJECTED
  Date: 7/4/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Do the projected texture calculation
  Arguments:
    src - src vertex
    dest - destination vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC1_PROJECTED(AtrDstVertex *dest, 
                          AtrVertex    *src,
                          AtrVertex    *end ) {
    _AtrLightNode *n = _atrLightHead;
    FxU32 tag        = _atrRenderMaterial->projectedTag;
    float        vpS = 0.5f;
    float        vpT = 0.5f;

    /*--------------------------------------------
      Find the projector associated with the current
      material
      --------------------------------------------*/
    while( n ) {
        if (n->light.flags == ATR_LIGHT_PROJECTOR &&
            n->light.project.tag == tag ) break; 
            n = n->next;
    }

#ifdef AT_DEBUGGING
    if ( !n )
      atuError( FXTRUE, 
                "atrRender*(): Error, no light matches tag %d.\n",
                tag );
#else
    if ( !n ) return;
#endif

    /*--------------------------------------------
      For each vertex, transform into light pov and
      scale to projected texture dimensions.
      --------------------------------------------*/
    while( src < end ) {
        float ooq;

        /* project s, t, w */
        dest->s1 = 
        ( src->x * n->modelToProjector.data[0] +
          src->y * n->modelToProjector.data[4] +
          src->z * n->modelToProjector.data[8] +
          n->modelToProjector.data[12] ) * vpS;
        dest->t1 = 
        -1.0f * ( src->x * n->modelToProjector.data[1] +
          src->y * n->modelToProjector.data[5] +
          src->z * n->modelToProjector.data[9] +
          n->modelToProjector.data[13] ) * vpT;
        dest->oow1 = 
        src->x * n->modelToProjector.data[2] +
        src->y * n->modelToProjector.data[6] +
        src->z * n->modelToProjector.data[10] +
        n->modelToProjector.data[14];

        /* Do something to avoid dbz ( right thing? ) */
        if ( dest->oow1 == 0.0f ) {
            dest->oow1 = 0.001f;
        } 
        ooq = 1.0f / dest->oow1;

        /* project onto plane */
        dest->s1 = dest->s1 * ooq + vpS;
        dest->t1 = dest->t1 * ooq + vpT;

        /* prepare for tmapping */
        if ( dest->flags == 0 ) {
            dest->oow1 *= dest->oow;
        } 

        dest++;
        src++;
    }
    return;
}



/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC0_PLANAR
  Date: 7/22/96
  Implementor(s): jdt, da
  Library: AT Render
  Description:
    Do the PLANAR texture calculation
  Arguments:
    src - src vertex
    dest - destination vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC0_PLANAR(AtrDstVertex *dest, 
                       AtrVertex    *src,
                       AtrVertex    *end ) {
    AtrXform *mTW = _atrCurrentXform;
    float s, t;
    float scale = _atrRenderMaterial->planarScale;
    extern float _atrTexOffsetS, _atrTexOffsetT;

    while( src < end ) {
        s =     src->x * mTW->data[0] +
                src->y * mTW->data[4] +
                src->z * mTW->data[8] +
                mTW->data[12];

        t =     src->x * mTW->data[2] +
                src->y * mTW->data[6] +
                src->z * mTW->data[10] +
                mTW->data[14];

        /* TBD: this is a temporary hack for flipper */

        s += _atrTexOffsetS;
        t += _atrTexOffsetT;

        if ( dest->flags )
          dest->oow0 = 1.0f;
        else
          dest->oow0 = dest->oow;

        dest->s0 = s * scale;
        dest->t0 = t * scale;

        dest++;
        src++;
    }
    return;
}


/*-------------------------------------------------------------------
  Function: _atrD3D_TCSRC1_PLANAR
  Date: 7/22/96
  Implementor(s): jdt, da
  Library: AT Render
  Description:
    Do the PLANAR texture calculation
  Arguments:
    src - src vertex
    dest - destination vertex
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3D_TCSRC1_PLANAR(AtrDstVertex *dest, 
                       AtrVertex    *src,
                       AtrVertex    *end ) {
    AtrXform *mTW = _atrCurrentXform;
    float s, t;
    float scale = _atrRenderMaterial->planarScale*.004f;
    extern float _atrTexOffsetS, _atrTexOffsetT;

    while( src < end ) {
        s =     src->x * mTW->data[0] +
                src->y * mTW->data[4] +
                src->z * mTW->data[8] +
                mTW->data[12];

        t =     src->x * mTW->data[2] +
                src->y * mTW->data[6] +
                src->z * mTW->data[10] +
                mTW->data[14];

        /* TBD: this is a temporary hack for flipper */

        s += _atrTexOffsetS*256.0f;
        t += _atrTexOffsetT*256.0f;

        if ( dest->flags )
          dest->oow1 = 1.0f;
        else
          dest->oow1 = dest->oow;

        dest->s1 = s * scale;
        dest->t1 = t * scale;

        dest++;
        src++;
    }
    return;
}
