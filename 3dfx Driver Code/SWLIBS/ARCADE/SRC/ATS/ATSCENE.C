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
** $Date: 10/11/00 7:34:28 PM$ 
**
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <fxos.h>
#include <atscenep.h>

FxU32 _atsRenderMode = ATS_RM_SOLID;

AtsPushMaterialFunc _atsPushMaterial;
AtsPopMaterialFunc _atsPopMaterial;

/*-------------------------------------------------------------------
  Function: atsInit
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Initialize ATB's scene manager library
  Arguments:
    none
  Return:
    nothing
  -------------------------------------------------------------------*/

FxBool atsInit(void)
{
    _atsInitXform( );

    if ( !_atsTextureInit( ) ) {
        atuError(FXTRUE, "Could not initialize texture manager\n");
    }

    _atsObjectInitClass();
    _atsNodeInitClass();
    _atsGroupInitClass();
    _atsDictInitClass();
    _atsFrameInitClass();
    _atsAnimInitClass();
    _atsShapeInitClass();
    _atsGeometryInitClass();
    _atsTriSetInitClass();
    _atsMaterialInitClass();
    _atsImageInitClass();
    _atsTextureInitClass();
    _atsCMeshInitClass();
    _atsSeqInitClass();
    _atsLODInitClass();
    _atsBboardInitClass();

    _atsInitConverters();

    _atsPushMaterial = ( AtsPushMaterialFunc)atrPushMaterial;
    _atsPopMaterial = ( AtsPopMaterialFunc)atrPopMaterial;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atsShutdown
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Shutdown ATB's scene manager library
  Arguments:
    none
  Return:
    nothing
  -------------------------------------------------------------------*/

void atsShutdown(void)
{
    _atsTermConverters();
    _atsTextureTerm();
    _atsShutdownXform( );
}

/*-------------------------------------------------------------------
  Function: atsRenderMode
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set the current render mode
  Arguments:
    renderMode - the desired mode
  Return:
    FXTRUE  on success
    FXFALSE on error
  -------------------------------------------------------------------*/

FxBool atsRenderMode(FxU32 renderMode) {
    switch (renderMode) {
    case ATS_RM_SOLID:
    case ATS_RM_WIREFRAME:
        _atsRenderMode = renderMode;
        return FXTRUE;
    default:
        atuError(FXFALSE, "Invalid render mode %d\n", renderMode);
        return FXFALSE;
    }
}

/*-------------------------------------------------------------------
  Function: atsMaterialFuncs
  Date: 4/24/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Basic facility for modifying attribute state when rendering a primitive
  Arguments:
    push - function to call when setting a material state
    pop  - function to call when done using a material
  Return:
    FXTRUE  on success
    FXFALSE on error
  -------------------------------------------------------------------*/

FxBool atsMaterialFuncs( AtsPushMaterialFunc push, AtsPopMaterialFunc pop) {
    _atsPushMaterial = push;
    _atsPopMaterial = pop;

    return FXTRUE;
}

static float frameTime = 0.0f;

/*-------------------------------------------------------------------
  Function: atsFrameBegin
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Perform any start of frame actions
  Arguments:
    None
  Return:
    None
  -------------------------------------------------------------------*/

void atsFrameBegin(void) {
    frameTime = fxTime();
}

/*-------------------------------------------------------------------
  Function: atsGetFrameTime
  Date: 7/1/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the current frame time
  Arguments:
    None
  Return:
    return current frame start time in seconds (floating point)
  -------------------------------------------------------------------*/

float atsGetFrameTime(void) {
    return frameTime;
}

/*-------------------------------------------------------------------
  Function: atsCullVolume
  Date: 7/18/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Specify the culling volume. In the interests of performance the view 
    volume passed in is not copied, rather the pointer is just saved and
    later used during the rendering process. For this reason the caller 
    should ensure that it is not deallocated or changed while the scene
    is being rendered.
   
  Arguments:
    viewVolume - volume to be used in culling
  Return:
    Nothing
  -------------------------------------------------------------------*/

AtmPolytope *_atsCullVolume = NULL;
FxU32 _atsCullMode = ATS_CULL_NONE;

void atsCullVolume(AtmPolytope *viewVolume) {
    _atsCullVolume = viewVolume;
}

/*-------------------------------------------------------------------
  Function: atsGetCullVolume
  Date: 7/18/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the culling volume. 
   
  Arguments:
    None
  Return:
    Current culling volume
  -------------------------------------------------------------------*/

AtmPolytope *atsGetCullVolume(void) {
    return _atsCullVolume ;
}

/*-------------------------------------------------------------------
  Function: atsCullMode
  Date: 7/18/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Set the current culling mode. 
   
  Arguments:
    cullMode - new culling mode
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsCullMode(FxU32 cullMode) {
    if (( cullMode & ~( ATS_CULL_NONE | ATS_CULL_NODES )) != 0 ) {
        atuError(FXFALSE, "Unsupported cull mode 0x%lx\n", cullMode);
        return;
    }

    _atsCullMode = cullMode;
}

/*-------------------------------------------------------------------
  Function: atsGetCullMode
  Date: 7/18/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Return the culling mode. 
   
  Arguments:
    None
  Return:
    Current culling volume
  -------------------------------------------------------------------*/

FxU32 atsGetCullMode(void) {
    return _atsCullMode;
}

/*-------------------------------------------------------------------
  Function: atsSelectCamera
  Date: 7/17/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Specify the camera with which to view the scene
  Arguments:
    c - camera to view scene with
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsSelectCamera( AtrCamera *c ) {
    AtmPlane tmp;
    static AtmPlane planes[6];
    static AtmPolytope viewVolume;
    float tanfov, xHither, xYon, yHither, yYon;
    AtmVector3 lth, rth, rbh, lbh, lty, rty, rby, lby;

    /* compute viewing frustum for culling */

    tanfov = (float) tan( c->fovRadians / 2.0f); 
    xHither = tanfov*c->nearClip;
    xYon = tanfov*c->farClip;
    yHither = xHither/c->aspectRatio;
    yYon = xYon/c->aspectRatio;
    lth[0] = -xHither; lth[1] =  yHither; lth[2] = c->nearClip;
    rth[0] =  xHither; rth[1] =  yHither; rth[2] = c->nearClip;
    rbh[0] =  xHither; rbh[1] = -yHither; rbh[2] = c->nearClip;
    lbh[0] = -xHither; lbh[1] = -yHither; lbh[2] = c->nearClip;

    lty[0] = -xYon; lty[1] =  yYon; lty[2] = c->farClip;
    rty[0] =  xYon; rty[1] =  yYon; rty[2] = c->farClip;
    rby[0] =  xYon; rby[1] = -yYon; rby[2] = c->farClip;
    lby[0] = -xYon; lby[1] = -yYon; lby[2] = c->farClip;

    /* we choose the vertices in th frustum which give the largest
       dynamic range in order to reduce numerical errors
     */

    /* near clipping plane */

    atmPlaneFromPoints(&tmp, lth, rth, rbh);
    atmPlaneOrthoXform(&planes[0], &tmp, c->lcsToWCS.data);

    /* far clipping plane */

    atmPlaneFromPoints(&tmp, rby, rty, lty);
    atmPlaneOrthoXform(&planes[1], &tmp, c->lcsToWCS.data);

    /* left clipping plane */

    atmPlaneFromPoints(&tmp, lby, lty, lth);
    atmPlaneOrthoXform(&planes[2], &tmp, c->lcsToWCS.data);

    /* right clipping plane */

    atmPlaneFromPoints(&tmp, rty, rby, rbh);
    atmPlaneOrthoXform(&planes[3], &tmp, c->lcsToWCS.data);

    /* top clipping plane */

    atmPlaneFromPoints(&tmp, lty, rty, rth);
    atmPlaneOrthoXform(&planes[4], &tmp, c->lcsToWCS.data);

    /* bottom clipping plane */

    atmPlaneFromPoints(&tmp, rbh, rby, lby);
    atmPlaneOrthoXform(&planes[5], &tmp, c->lcsToWCS.data);

    viewVolume.numPlanes = 6;
    viewVolume.planes = planes;
    _atsCullVolume = &viewVolume;
    atrSelectCamera( c );
}


/*-------------------------------------------------------------------
  Function: atsQueryCamera
  Date: 7/17/96
  Implementor(s): mlwp
  Library: AT Scene Manager Library
  Description: 
    Query the camera with which to view the scene
  Arguments:
    c - camera being used to view scene
  Return:
    Nothing
  -------------------------------------------------------------------*/

void atsQueryCamera( AtrCamera *c ) {
    atrQueryCamera( c );
}
