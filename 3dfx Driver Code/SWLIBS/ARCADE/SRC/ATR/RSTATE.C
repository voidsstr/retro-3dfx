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
 ** $Date: 10/11/00 7:34:16 PM$ 
 **
 */

#include <atutil.h>
#include "atrender.h"
#include "fxatr.h"
#include <math.h>
#include <glide.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <texus.h>
#if macintosh
#include <types.h> /* for DebugStr */
#endif

extern TxErrorCallbackFnc_t _txErrorCallback;

#ifdef AT_DRIVER_GLIDE
AtrDriver * _atrGlideInit( AtrDriverInfo *driverInfo );
#endif
#ifdef AT_DRIVER_RAVE
AtrDriver * _atrRAVInit( AtrDriverInfo *driverInfo );
#endif
#ifdef AT_DRIVER_D3D
AtrDriver * _atrD3DInit( AtrDriverInfo *driverInfo );
#endif
AtrDriver * _atrXXXInit( AtrDriverInfo *driverInfo );

typedef struct {
    char *name;
    AtrDriver *(* init)(void *AtrDriverInfo);
} AtrDriverEntry;

static AtrDriverEntry driverTable[] = {
#ifdef AT_DRIVER_GLIDE
                                       { "Glide", _atrGlideInit },
#endif
#ifdef AT_DRIVER_D3D
                                       { "D3D", _atrD3DInit },
                                       { "D3DS", _atrD3DInit },
#endif
#ifdef AT_DRIVER_RAVE
                                       { "Rave", _atrRAVInit },
#endif
                                       { "XXX", _atrXXXInit },
                                       { NULL, NULL }}; /* list must be null terminated */

float _atrSnapBias = 0.0f;
float _atrDriverXOffset = 0.0f;
float _atrDriverYOffset = 0.0f;

/* TBD: this is an ugly hack, need a texture matrix here */

float _atrTexOffsetS = 0.0f, _atrTexOffsetT = 0.0f;

void atrSetTexPlanarOffset( float s, float t)    {
    _atrTexOffsetS = s;
    _atrTexOffsetT = t;
}
/* end Denis */

/*
 **---------------------------------------------------------------
 ** Render Cache, Globals
 **---------------------------------------------------------------
 */
_AtrRenderCache *_atrRenderCache;
AtrDriver *_atrDriver = NULL, *_atrSavedDriver = NULL;
AtrBuffer _atrRenderBuffer;
float _atrTwoFiftyFive = 255.0f;

/*-------------------------------------------------------------------
  Function: _atrInitRenderCache
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
  Private function to initialize the render cache at library startup.
  Arguments:
  none
  Return:
  none
  -------------------------------------------------------------------*/
static void _atrInitRenderCache( void ) {
    if ( sizeof( _AtrRenderCache ) != 4096 ) {
        atuError( FXTRUE, 
                 "_atrInitRenderCache(): Internal data structure"
                 " no longer aligned(%d).\nCheck fxatr.h\n", 
                 sizeof( _AtrRenderCache ) );
    }    
    _atrRenderCache = atuMemAlignedCalloc( sizeof( _AtrRenderCache ), 1, 32 );
}

/*-------------------------------------------------------------------
  Function: _atrShutdownRenderCache
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
  Private function to free the render cache at library shutdown.
  Arguments:
  none
  Return:
  none
  -------------------------------------------------------------------*/
static void _atrShutdownRenderCache( void ) {
    atuMemFree( _atrRenderCache );
    _atrRenderCache = 0;
}

/*
 **---------------------------------------------------------------
 ** Transform Stack
 **---------------------------------------------------------------
 */
AtuStack *_atrXformStack = 0;
AtrXform *_atrCurrentXform = 0;

/*-------------------------------------------------------------------
  Function: _atrInitXform
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
  Private function to initialize the transform stack. 
  Arguments:
  none
  Return:
  none
  -------------------------------------------------------------------*/
static void _atrInitXform( void ) {
    AtrXform base;
    _atrXformStack = atuStackAllocate( 1 );
    atrXformSetIdentity( &base );
    atuStackInit( _atrXformStack, sizeof( AtrXform ), 16 );
    atuStackPush( _atrXformStack, &base );
    _atrCurrentXform = 
      (AtrXform*)(_atrXformStack->top - _atrXformStack->elementSize);
    return;
}

/*-------------------------------------------------------------------
  Function: _atrShutdownXform
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
  Private function to free the transform stack. 
  Arguments:
  none
  Return:
  none
  -------------------------------------------------------------------*/
static void _atrShutdownXform( void ) {
    atuStackDeallocate( _atrXformStack );
    _atrXformStack = 0;
    _atrCurrentXform = 0;
}

/*-------------------------------------------------------------------
  Function: _atrUpdateXform
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Render
  Description:
  Do all of the work to update the renderer state when
  the current xform changes
  Arguments:
  none
  Return:
  none
  -------------------------------------------------------------------*/
void _atrUpdateXform( void ) {
    AtrXform wcsToLCS;

    _atrRenderCache->lcsToCCS.data[0] = 
      _atrCurrentXform->data[0] * _atrCurrentCamera->wcsToCCS.data[0] +
      _atrCurrentXform->data[1] * _atrCurrentCamera->wcsToCCS.data[4] +
      _atrCurrentXform->data[2] * _atrCurrentCamera->wcsToCCS.data[8];

    _atrRenderCache->lcsToCCS.data[1] = 
      _atrCurrentXform->data[0] * _atrCurrentCamera->wcsToCCS.data[1] +
      _atrCurrentXform->data[1] * _atrCurrentCamera->wcsToCCS.data[5] +
      _atrCurrentXform->data[2] * _atrCurrentCamera->wcsToCCS.data[9];

    _atrRenderCache->lcsToCCS.data[2] = 
      _atrCurrentXform->data[0] * _atrCurrentCamera->wcsToCCS.data[2] +
      _atrCurrentXform->data[1] * _atrCurrentCamera->wcsToCCS.data[6] +
      _atrCurrentXform->data[2] * _atrCurrentCamera->wcsToCCS.data[10];

    _atrRenderCache->lcsToCCS.data[4] = 
      _atrCurrentXform->data[4] * _atrCurrentCamera->wcsToCCS.data[0] +
      _atrCurrentXform->data[5] * _atrCurrentCamera->wcsToCCS.data[4] +
      _atrCurrentXform->data[6] * _atrCurrentCamera->wcsToCCS.data[8];

    _atrRenderCache->lcsToCCS.data[5] = 
      _atrCurrentXform->data[4] * _atrCurrentCamera->wcsToCCS.data[1] +
      _atrCurrentXform->data[5] * _atrCurrentCamera->wcsToCCS.data[5] +
      _atrCurrentXform->data[6] * _atrCurrentCamera->wcsToCCS.data[9];

    _atrRenderCache->lcsToCCS.data[6] = 
      _atrCurrentXform->data[4] * _atrCurrentCamera->wcsToCCS.data[2] +
      _atrCurrentXform->data[5] * _atrCurrentCamera->wcsToCCS.data[6] +
      _atrCurrentXform->data[6] * _atrCurrentCamera->wcsToCCS.data[10];

    _atrRenderCache->lcsToCCS.data[8]  = 
      _atrCurrentXform->data[8]  * _atrCurrentCamera->wcsToCCS.data[0] +
      _atrCurrentXform->data[9]  * _atrCurrentCamera->wcsToCCS.data[4] +
      _atrCurrentXform->data[10] * _atrCurrentCamera->wcsToCCS.data[8];

    _atrRenderCache->lcsToCCS.data[9]  = 
      _atrCurrentXform->data[8]  * _atrCurrentCamera->wcsToCCS.data[1] +
      _atrCurrentXform->data[9]  * _atrCurrentCamera->wcsToCCS.data[5] +
      _atrCurrentXform->data[10] * _atrCurrentCamera->wcsToCCS.data[9];

    _atrRenderCache->lcsToCCS.data[10] = 
      _atrCurrentXform->data[8]  * _atrCurrentCamera->wcsToCCS.data[2] +
      _atrCurrentXform->data[9]  * _atrCurrentCamera->wcsToCCS.data[6] +
      _atrCurrentXform->data[10] * _atrCurrentCamera->wcsToCCS.data[10];

    _atrRenderCache->lcsToCCS.data[12] = 
      _atrCurrentXform->data[12] * _atrCurrentCamera->wcsToCCS.data[0] +
      _atrCurrentXform->data[13] * _atrCurrentCamera->wcsToCCS.data[4] +
      _atrCurrentXform->data[14] * _atrCurrentCamera->wcsToCCS.data[8] +
      _atrCurrentCamera->wcsToCCS.data[12]; 

    _atrRenderCache->lcsToCCS.data[13] = 
      _atrCurrentXform->data[12] * _atrCurrentCamera->wcsToCCS.data[1] +
      _atrCurrentXform->data[13] * _atrCurrentCamera->wcsToCCS.data[5] +
      _atrCurrentXform->data[14] * _atrCurrentCamera->wcsToCCS.data[9] +
      _atrCurrentCamera->wcsToCCS.data[13]; 
    
    _atrRenderCache->lcsToCCS.data[14] = 
      _atrCurrentXform->data[12] * _atrCurrentCamera->wcsToCCS.data[2]  +
      _atrCurrentXform->data[13] * _atrCurrentCamera->wcsToCCS.data[6]  +
      _atrCurrentXform->data[14] * _atrCurrentCamera->wcsToCCS.data[10] +
      _atrCurrentCamera->wcsToCCS.data[14];

    _atrRenderCache->lcsToCCS.data[3]  = 0.0f;
    _atrRenderCache->lcsToCCS.data[7]  = 0.0f;
    _atrRenderCache->lcsToCCS.data[11] = 0.0f;
    _atrRenderCache->lcsToCCS.data[15] = 1.0f;

    atrXformInvert( &wcsToLCS, _atrCurrentXform );

    _atrRenderCache->cameraPositionInLCS[0] = 
      _atrCurrentCamera->lcsToWCS.data[12] * wcsToLCS.data[0] +
      _atrCurrentCamera->lcsToWCS.data[13] * wcsToLCS.data[4] +
      _atrCurrentCamera->lcsToWCS.data[14] * wcsToLCS.data[8] +
      wcsToLCS.data[12];
    _atrRenderCache->cameraPositionInLCS[1] = 
      _atrCurrentCamera->lcsToWCS.data[12] * wcsToLCS.data[1] +
      _atrCurrentCamera->lcsToWCS.data[13] * wcsToLCS.data[5] +
      _atrCurrentCamera->lcsToWCS.data[14] * wcsToLCS.data[9] +
      wcsToLCS.data[13];
    _atrRenderCache->cameraPositionInLCS[2] = 
      _atrCurrentCamera->lcsToWCS.data[12] * wcsToLCS.data[2] +
      _atrCurrentCamera->lcsToWCS.data[13] * wcsToLCS.data[6] +
      _atrCurrentCamera->lcsToWCS.data[14] * wcsToLCS.data[10] +
      wcsToLCS.data[14];

    {
        _AtrLightNode *l = _atrLightHead;
        while( l ) {
            if ( l->light.flags == ATR_LIGHT_PROJECTOR ) {
                /* Update projector matrix */
                AtrXform worldToProjector;
                atrXformInvert(&worldToProjector, 
                               &l->light.lcsToWCS);
                worldToProjector.data[0]  *= l->fovXScale;
                worldToProjector.data[4]  *= l->fovXScale;
                worldToProjector.data[8]  *= l->fovXScale;
                worldToProjector.data[12] *= l->fovXScale;
                worldToProjector.data[1]  *= l->fovYScale;
                worldToProjector.data[5]  *= l->fovYScale;
                worldToProjector.data[9]  *= l->fovYScale;
                worldToProjector.data[13] *= l->fovYScale;
                atrXformCat(&l->modelToProjector,
                            _atrCurrentXform,
                            &worldToProjector);
            } else {
                l->vf[0] = 
                  - (l->light.lcsToWCS.data[8]  * wcsToLCS.data[0] +
                     l->light.lcsToWCS.data[9]  * wcsToLCS.data[4] +
                     l->light.lcsToWCS.data[10] * wcsToLCS.data[8] ); 
                l->vf[1] = 
                  - (l->light.lcsToWCS.data[8]  * wcsToLCS.data[1] +
                     l->light.lcsToWCS.data[9]  * wcsToLCS.data[5] +
                     l->light.lcsToWCS.data[10] * wcsToLCS.data[9] );
                l->vf[2] = 
                  - (l->light.lcsToWCS.data[8]  * wcsToLCS.data[2] +
                     l->light.lcsToWCS.data[9]  * wcsToLCS.data[6] +
                     l->light.lcsToWCS.data[10] * wcsToLCS.data[10] );
            }
            l = l->next;
        }
    }
}

/*-------------------------------------------------------------------
  Function: atrPushXform
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
  Push a transform on the transform stack.
  Arguments:
  t - transform to push
  Return:
  none
  -------------------------------------------------------------------*/
void atrPushXform( AtrXform *t ){
#ifdef AT_DEBUGGING
    if ( !t ) 
      atuError( FXFALSE, "atrPushXform(): Invalid parameter.\n" );
#endif
    atuStackPush( _atrXformStack, t );
    _atrCurrentXform = 
      (AtrXform*)(_atrXformStack->top - _atrXformStack->elementSize);

    _atrUpdateXform();

    return;
}

/*-------------------------------------------------------------------
  Function: atrPopXform
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:  
    Pop the top of the Xform stack.
  Arguments:
    update - update state of renderer if true
  Return:
    none
  -------------------------------------------------------------------*/
void atrPopXform( FxBool update ) {

    /* Don't allow stack to pop below 1 element */
    if ( _atrXformStack->top > 
         _atrXformStack->base + _atrXformStack->elementSize )
        _atrXformStack->top -= _atrXformStack->elementSize;
#ifdef AT_DEBUGGING
    else 
        atuError( FXFALSE, "atrPopXform(): Stack underflow.\n" );
#endif
    _atrCurrentXform = 
        (AtrXform*)(_atrXformStack->top - _atrXformStack->elementSize);

    if ( update ) _atrUpdateXform();

    return;
}

/*-------------------------------------------------------------------
  Function: atrPostCatPushXform
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:  
    Copy the top of the stack and post cat the argument transform
  Arguments:
    t - transform
  Return:
    none
  -------------------------------------------------------------------*/
void atrPostCatPushXform( AtrXform *t ) {
#ifdef AT_DEBUGGING
    if ( !t ) 
        atuError( FXFALSE, "atrPostCatPushXform(): Invalid parameter.\n" );
#endif
    atuStackPush( _atrXformStack, _atrCurrentXform );
    _atrCurrentXform = 
        (AtrXform*)(_atrXformStack->top - _atrXformStack->elementSize);
    atrXformCat( _atrCurrentXform, _atrCurrentXform, t );

    _atrUpdateXform();

    return;
}

/*-------------------------------------------------------------------
  Function: atrPreCatPushXform
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:  
    Copy the top of the stack and pre cat the argument transform.  This
    will be the most commonly used xform stack manipulation function as
    it corresponds to adding a local transformation to whatever global 
    transformation currently is embodied in the top of the transform
    stack.
  Arguments:
    t - transform
  Return:
    none
  -------------------------------------------------------------------*/
void atrPreCatPushXform( AtrXform *t ) {
    AtmMatrix4x4 temp;
#ifdef AT_DEBUGGING
    if ( !t ) 
        atuError( FXFALSE, "atrPreCatPushXform(): Invalid parameter.\n" );
#endif
    atuStackPush( _atrXformStack, _atrCurrentXform );
    _atrCurrentXform = 
        (AtrXform*)(_atrXformStack->top - _atrXformStack->elementSize);

#if 1
    temp[0] = t->data[0] * _atrCurrentXform->data[0] +
              t->data[1] * _atrCurrentXform->data[4] +
              t->data[2] * _atrCurrentXform->data[8];

    temp[1] = t->data[0] * _atrCurrentXform->data[1] +
              t->data[1] * _atrCurrentXform->data[5] +
              t->data[2] * _atrCurrentXform->data[9];

    temp[2] = t->data[0] * _atrCurrentXform->data[2] +
              t->data[1] * _atrCurrentXform->data[6] +
              t->data[2] * _atrCurrentXform->data[10];

    temp[4] = t->data[4] * _atrCurrentXform->data[0] +
              t->data[5] * _atrCurrentXform->data[4] +
              t->data[6] * _atrCurrentXform->data[8];

    temp[5] = t->data[4] * _atrCurrentXform->data[1] +
              t->data[5] * _atrCurrentXform->data[5] +
              t->data[6] * _atrCurrentXform->data[9];

    temp[6] = t->data[4] * _atrCurrentXform->data[2] +
              t->data[5] * _atrCurrentXform->data[6] +
              t->data[6] * _atrCurrentXform->data[10];

    temp[8]  = t->data[8]  * _atrCurrentXform->data[0] +
               t->data[9]  * _atrCurrentXform->data[4] +
               t->data[10] * _atrCurrentXform->data[8];

    temp[9]  = t->data[8]  * _atrCurrentXform->data[1] +
               t->data[9]  * _atrCurrentXform->data[5] +
               t->data[10] * _atrCurrentXform->data[9];

    temp[10] = t->data[8]  * _atrCurrentXform->data[2] +
               t->data[9]  * _atrCurrentXform->data[6] +
               t->data[10] * _atrCurrentXform->data[10];

    _atrCurrentXform->data[12] += t->data[12] * _atrCurrentXform->data[0] +
                                  t->data[13] * _atrCurrentXform->data[4] +
                                  t->data[14] * _atrCurrentXform->data[8]; 
    _atrCurrentXform->data[13] += t->data[12] * _atrCurrentXform->data[1] +
                                  t->data[13] * _atrCurrentXform->data[5] +
                                  t->data[14] * _atrCurrentXform->data[9]; 
    _atrCurrentXform->data[14] += t->data[12] * _atrCurrentXform->data[2] +
                                  t->data[13] * _atrCurrentXform->data[6] +
                                  t->data[14] * _atrCurrentXform->data[10];

    _atrCurrentXform->data[0]  = temp[0];
    _atrCurrentXform->data[1]  = temp[1];
    _atrCurrentXform->data[2]  = temp[2];
    _atrCurrentXform->data[3]  = 0.0f;
    _atrCurrentXform->data[4]  = temp[4];
    _atrCurrentXform->data[5]  = temp[5];
    _atrCurrentXform->data[6]  = temp[6];
    _atrCurrentXform->data[7]  = 0.0f;
    _atrCurrentXform->data[8]  = temp[8];
    _atrCurrentXform->data[9]  = temp[9];
    _atrCurrentXform->data[10] = temp[10];
    _atrCurrentXform->data[11] = 0.0f;
    _atrCurrentXform->data[15] = 1.0f;
#else
    atrXformCat( _atrCurrentXform, t, _atrCurrentXform );
#endif

    _atrUpdateXform();
    
    return;
}

/*-------------------------------------------------------------------
  Function: atrQueryXform
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Copies the top of the tranform stack into the memory location
    provided.
  Arguments:
    t - a pointer to an empty atrXform
  Return:
    none
  -------------------------------------------------------------------*/
void atrQueryXform( AtrXform *t ) {
#ifdef AT_DEBUGGING
    if ( !t ) 
        atuError( FXFALSE, "atrQueryXform(): Invalid parameter.\n" );
#endif
    atrXformAssign( t, _atrCurrentXform );
    return;
}


/*
**---------------------------------------------------------------
** Environment Stack
**---------------------------------------------------------------
*/
AtrEnv   *_atrCurrentEnv;
AtuStack *_atrEnvStack;

/*-------------------------------------------------------------------
  Function: _atrInitEnv
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Private function to initialize the env stack. 
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrInitEnv( void ) {
    AtrEnv base;
    base.flags = 0;
    base.fogColor.r = 0.0f;
    base.fogColor.g = 0.0f;
    base.fogColor.b = 0.0f;
    _atrEnvStack = atuStackAllocate( 1 );
    atuStackInit( _atrEnvStack, sizeof( AtrEnv ), 16 );
    atuStackPush( _atrEnvStack, &base );
    _atrCurrentEnv = 
        (AtrEnv*)(_atrEnvStack->top - _atrEnvStack->elementSize);
    return;
}

/*-------------------------------------------------------------------
  Function: _atrShutdownEnv
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Private function to free the env stack. 
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrShutdownEnv( void ) {
    atuStackDeallocate( _atrEnvStack );
    _atrEnvStack = 0;
    _atrCurrentEnv = 0;
}

/*-------------------------------------------------------------------
  Function: atrPushEnv
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Push an Env on the Env stack.
  Arguments:
    e - env to push
  Return:
    none
  -------------------------------------------------------------------*/

void atrPushEnv( AtrEnv *e ){
    
#ifdef AT_DEBUGGING
    if ( !e ) 
        atuError( FXFALSE, "atrPushEnv(): Invalid parameter.\n" );
#endif

    atuStackPush( _atrEnvStack, e );

     _atrCurrentEnv = 
        (AtrEnv*)(_atrEnvStack->top - _atrEnvStack->elementSize);

    DRV_FUNC(UpdateEnv)( e );
    e->flags &= ~ATR_FOG_TABLE_UPDATE;

    return;
}
   
/*-------------------------------------------------------------------
  Function: atrPopEnv
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:  
   Pop the top of the env stack.
  Arguments:
    update - update state after pop if true
  Return:
    none
  -------------------------------------------------------------------*/
void atrPopEnv( FxBool update ) {

   /* Don't allow stack to pop below 1 element */
    if ( _atrEnvStack->top > 
         _atrEnvStack->base + _atrEnvStack->elementSize )
        _atrEnvStack->top -= _atrEnvStack->elementSize;
#ifdef AT_DEBUGGING
    else 
        atuError( FXFALSE, "atrPopEnv(): Stack underflow.\n" );
#endif
    _atrCurrentEnv = 
        (AtrEnv*)(_atrEnvStack->top - _atrEnvStack->elementSize);

    if ( update ) {
        DRV_FUNC(UpdateEnv)( _atrCurrentEnv );
    }

    return;
}
    
/*-------------------------------------------------------------------
  Function: atrQueryEnv
  Date: 3/22/96
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Retrieve a copy of the current environment.
  Arguments:
    e - storage into which to copy the current environment
  Return:
    none
  -------------------------------------------------------------------*/
void atrQueryEnv( AtrEnv *e ) {
    *e = *_atrCurrentEnv;
    return;
}
    
    
/*
**---------------------------------------------------------------
** Material Stack
**---------------------------------------------------------------
*/
AtuStack    *_atrMaterialStack = 0;
AtrMaterial *_atrCurrentMaterial = 0;
    
/*-------------------------------------------------------------------
  Function: _atrInitMaterial
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Private function to initialize the material stack. 
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrInitMaterial( void ) {
    AtrMaterial base;

    _atrMaterialStack = atuStackAllocate( 1 );

    memset( &base, 0, sizeof( AtrMaterial ) );
    atrMaterialSetup( &base, ATR_MAT_GSHADE );
    base.diffuse.r = .5f;
    base.diffuse.g = .5f;
    base.diffuse.b = .5f;
    
    DRV_FUNC(UpdateMaterial)( &base );

    atuStackInit( _atrMaterialStack, sizeof( AtrMaterial ), 16 );
    atuStackPush( _atrMaterialStack, &base );
    _atrCurrentMaterial = 
       (AtrMaterial*)(_atrMaterialStack->top - 
                      _atrMaterialStack->elementSize);
    return;
}

/*-------------------------------------------------------------------
  Function: _atrShutdownMaterial
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Private function to free the material stack. 
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrShutdownMaterial( void ) {
    atuStackDeallocate( _atrMaterialStack );
    _atrMaterialStack = 0;
    _atrCurrentMaterial = 0;
}

/*-------------------------------------------------------------------
  Function: _atrPushMaterialSuccessors
  Date: 3/22/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Recursive function to correctly stack up successor materials.
  Arguments:
    m - material pointer
  Return:
    pointer to material on stack.
  -------------------------------------------------------------------*/
static AtrMaterial *_atrPushMaterialSuccessors( AtrMaterial *m ) {
    AtrMaterial *tmp;
    if ( m == 0 ) 
        return 0;
    tmp = _atrPushMaterialSuccessors( m->successor );
    atuStackPush( _atrMaterialStack, m );
    ((AtrMaterial*)(_atrMaterialStack->top - 
                    _atrMaterialStack->elementSize))->successor = tmp;
    return (AtrMaterial*)(_atrMaterialStack->top - 
                          _atrMaterialStack->elementSize);
}
 
/*-------------------------------------------------------------------
  Function: atrPushMaterial
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Push a material on the material stack.  If the material is chained,
    that is if it has a successor member, then it all the successors
    will be pushed first, then the library idle state will be set to 
    the state of the top material.
  Arguments:
    m - material to push
  Return:
    none
  -------------------------------------------------------------------*/
void atrPushMaterial( AtrMaterial *m ){

#ifdef AT_DEBUGGING
    if ( !m ) 
        atuError( FXFALSE, "atrPushMaterial(): Invalid parameter.\n" );
#endif

    _atrCurrentMaterial = _atrPushMaterialSuccessors( m );
    DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
    return;
}

/*-------------------------------------------------------------------
  Function: atrPopMaterial
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:  
    Pop the top of the Material stack.
  Arguments:
    update - update state of renderer if true
  Return:
    none
  -------------------------------------------------------------------*/
void atrPopMaterial( FxBool update ) {
    /* Don't allow stack to pop below 1 element */
    if ( _atrMaterialStack->top == _atrMaterialStack->base + 
                                   _atrMaterialStack->elementSize )
        return;

    while( _atrCurrentMaterial->successor ) {
        _atrMaterialStack->top -= _atrMaterialStack->elementSize;
        _atrCurrentMaterial = 
            (AtrMaterial*)(_atrMaterialStack->top - 
                           _atrMaterialStack->elementSize);
    }

    _atrMaterialStack->top -= _atrMaterialStack->elementSize;
    _atrCurrentMaterial = 
        (AtrMaterial*)(_atrMaterialStack->top - 
                       _atrMaterialStack->elementSize);


    if ( update ) DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
    return;
}
    
/*-------------------------------------------------------------------
  Function: atrQueryMaterial
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Redner
  Description: 
    Copy the material off of the top of the material stack.  If
    a multi-pass material, then the top of the stack is copied
    down, and the successor pointer is nulled out.
  Arguments:
    m - pointer to storage for material
  Return:
    none
  -------------------------------------------------------------------*/
void atrQueryMaterial( AtrMaterial *m ) {
    *m = *_atrCurrentMaterial;
    m->successor = 0;
    return;    
}


/*
**---------------------------------------------------------------
** Current Camera
**---------------------------------------------------------------
*/
AtrCamera *_atrCurrentCamera = 0;


/*-------------------------------------------------------------------
  Function: atrQueryLCSEyePoint
  Date: 7/3
  Implementor(s): jdt
  Library: AT Render
  Description: 
  Fill in an atmVector3 with the current camera position in 
  local coordinates
  Arguments:
  eyepoint - vector into which to copy eyepoint
  Return:
  none
  -------------------------------------------------------------------*/
void atrQueryLCSEyePoint( AtmVector3 eyepoint ) {
    eyepoint[0] = _atrRenderCache->cameraPositionInLCS[0];
    eyepoint[1] = _atrRenderCache->cameraPositionInLCS[1];
    eyepoint[2] = _atrRenderCache->cameraPositionInLCS[2];
    return;
}


/*-------------------------------------------------------------------
  Function: _atrInitCamera
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    private function - do startup initialization of the current camera
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrInitCamera( void ) {
    _atrCurrentCamera = atrCameraAllocate( 1 );
    if ( !_atrCurrentXform ) 
        atuError( FXTRUE, "_atrInitCamera(): Library initialization failure." );
    atrSelectCamera( _atrCurrentCamera );
    return;
}

/*-------------------------------------------------------------------
  Function: _atrShutdownCamera
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    private function - do shutdown cleanup on current camera
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrShutdownCamera( void ) {
    atrCameraDeallocate( _atrCurrentCamera );
}

/*-------------------------------------------------------------------
  Function: atrSelectCamera
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Select a camera as the current camera to render.  This effectively 
    takes a snapshot of the camera's state.  
  Arguments:
    c - camera to select
  Return:
    none
  -------------------------------------------------------------------*/
void atrSelectCamera( AtrCamera *c ) {
    AtrXform vcsToCCS;
    AtrXform wcsToVCS;
    AtrXform vcsToLCS;
    float ta, left, right, top, bottom;

#ifdef AT_DEBUGGING
    if ( !c ) 
        atuError( FXTRUE, "atrSelectCamera(): Invalid parameter.\n" );
    if ( c->nearClip > c->farClip ) 
        atuError( FXTRUE, "Invalid Camera, nearclip > farclip\n" );        
#endif    

    *_atrCurrentCamera = *c;

    _atrRenderCache->nearClip = _atrCurrentCamera->nearClip;
    _atrRenderCache->farClip = _atrCurrentCamera->farClip;

    /* vcsToCCS Transform */
    atrXformSetIdentity( &vcsToCCS );

	ta = (float) tan( _atrCurrentCamera->fovRadians / 2.0f );


    if ( c->flags & ATR_CAMERA_FOVY ) {
        vcsToCCS.data[5] = 1.0f / ta ;
        vcsToCCS.data[0] = vcsToCCS.data[5] / _atrCurrentCamera->aspectRatio;
    } else {
        vcsToCCS.data[0] = 1.0f / ta ;
        vcsToCCS.data[5] = vcsToCCS.data[0] * _atrCurrentCamera->aspectRatio;
    }

    left =  (float)tan(_atrCurrentCamera->xOffset-0.5f*_atrCurrentCamera->fovRadians);
    right =  (float)tan(_atrCurrentCamera->xOffset+0.5f*_atrCurrentCamera->fovRadians);
    top =  (float)(tan(_atrCurrentCamera->yOffset+0.5f*_atrCurrentCamera->fovRadians)/
                   _atrCurrentCamera->aspectRatio ) ;
    bottom =  (float)(tan(_atrCurrentCamera->yOffset-0.5f*_atrCurrentCamera->fovRadians)/
                   _atrCurrentCamera->aspectRatio ) ;

    vcsToCCS.data[8] = _atrCurrentCamera->xOffset;

    atrXformInvert( &wcsToVCS, &_atrCurrentCamera->lcsToWCS );
    atrXformCat( &_atrCurrentCamera->wcsToCCS, &wcsToVCS, &vcsToCCS );

    atrXformCat( &_atrRenderCache->lcsToCCS,
                 _atrCurrentXform,
                 &_atrCurrentCamera->wcsToCCS );
    
    atrXformInvert( &vcsToLCS, _atrCurrentXform );
    atrXformCat( &vcsToLCS, &_atrCurrentCamera->lcsToWCS, &vcsToLCS );
    _atrRenderCache->cameraPositionInLCS[0] = vcsToLCS.data[12];
    _atrRenderCache->cameraPositionInLCS[1] = vcsToLCS.data[13];
    _atrRenderCache->cameraPositionInLCS[2] = vcsToLCS.data[14];
    return;
}

/*-------------------------------------------------------------------
  Function: atrQueryCamera
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Retrieve a copy of the current camera.
  Arguments:
  Return:
  -------------------------------------------------------------------*/
void atrQueryCamera( AtrCamera *c ) {
#ifdef AT_DEBUGGING
    if ( !c ) 
        atuError( FXTRUE, "atrQueryCamera(): Invalid parameter\n" );

    if ( !_atrCurrentCamera ) 
        atuError( FXTRUE, "atrQueryCamera(): Library not initialized.\n" );
#endif    
    *c = *_atrCurrentCamera;
    return;
}

/*
**---------------------------------------------------------------
** Current Canvas
**---------------------------------------------------------------
*/

AtrCanvas *_atrCurrentCanvas;

/*-------------------------------------------------------------------
  Function: _atrInitCanvas
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    private function to initialize the current canvas at startup.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrInitCanvas( void ) {
    _atrCurrentCanvas = atuMemCalloc( sizeof( AtrCanvas ), 1 );
    _atrCurrentCanvas->xMin = 0;
    _atrCurrentCanvas->yMin = 0;
    _atrCurrentCanvas->xMax = _atrDriver->caps.width;
    _atrCurrentCanvas->yMax = _atrDriver->caps.height;
    atrSelectCanvas( _atrCurrentCanvas );
    return;
}

/*-------------------------------------------------------------------
  Function: _atrShudownCanvas
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Cleanup atrCurrentCanvas at shutdown
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrShutdownCanvas( void ) {
    atuMemFree( _atrCurrentCanvas );
    return;
}

/*-------------------------------------------------------------------
  Function: atrSelectCanvas
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Make a new canvas the current output window.
  Arguments:
    c - canvas to be selected as current
  Return:
    none
  -------------------------------------------------------------------*/
void atrSelectCanvas( AtrCanvas *c ) {
    AtrCanvas c1;

    c1.xMin = ATM_MAX(0, c->xMin);
    c1.yMin = ATM_MAX(0, c->yMin);
    c1.xMax = ATM_MIN(_atrDriver->caps.width, c->xMax);
    c1.yMax = ATM_MIN(_atrDriver->caps.height, c->yMax);

#ifdef AT_DEBUGGING
    if ( !c ) 
        atuError( FXTRUE, "atrSelectCanvas(): Invalid paramerter.\n" );

    if ( c1.xMin >= c1.xMax || c1.yMin >= c1.yMax )
        atuError( FXTRUE, 
            "atrSelectCanvas(): Bad boundary definition for canvas.\n" );

    if ( !_atrCurrentCanvas ) 
        atuError( FXTRUE, "atrSelectCanvas(): Library not initialized.\n" );
#endif
    *_atrCurrentCanvas = c1;

    /* scale viewport slightly so we never write pixels outside of framebuffer
       due to inaccuracies in floating point clipping/transformatuion */
    
    _atrRenderCache->xScale = (float) ( c1.xMax - c1.xMin - 0.5f ) / 2.0f;
    _atrRenderCache->yScale = (float) ( c1.yMax - c1.yMin - 0.5f) / 2.0f;
    _atrRenderCache->xOffset = _atrRenderCache->xScale + 
                               (float) c1.xMin + _atrSnapBias + _atrDriverXOffset;
    _atrRenderCache->yOffset = _atrRenderCache->yScale + 
                               (float) c1.yMin + _atrSnapBias + _atrDriverYOffset;

    /* For window clear */
    DRV_FUNC(ClipWindow)( c1.xMin, c1.yMin, c1.xMax, c1.yMax );

    return;
}

/*-------------------------------------------------------------------
  Function: atrQueryCanvas
  Date: 3/20/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Get a copy of the current canvas
  Arguments:
    c - storage for current canvas data
  Return:
    none
  -------------------------------------------------------------------*/
void atrQueryCanvas( AtrCanvas *c ) {
#ifdef AT_DEBUGGING
    if ( !c ) 
        atuError( FXTRUE, "atrQueryCanvas(): Invalid paramerter.\n" );

    if ( !_atrCurrentCanvas ) 
        atuError( FXTRUE, "atrQueryCanvas(): Library not initialized.\n" );
#endif
    *c = *_atrCurrentCanvas;
    return;
}

/*
**---------------------------------------------------------------
** Lights
**---------------------------------------------------------------
*/
_AtrLightNode   *_atrLightHead;
_AtrLightNode   *_atrLightUnused;
_AtrLightNode   _atrLightArray[ATR_MAX_LIGHTS];

/*-------------------------------------------------------------------
  Function: _atrInitLight
  Date: 3/21/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Initialize internal light structure for library startup.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrInitLight( void ) {
    FxU32 counter;

    /* Create Circular Linked List */
    for( counter = 0; counter < ATR_MAX_LIGHTS; counter++ ) {
        _atrLightArray[counter].next = &_atrLightArray[counter+1];
    }
    _atrLightArray[ATR_MAX_LIGHTS-1].next = 0;

    _atrLightUnused = &_atrLightArray[0];
    _atrLightHead = 0;
}

/*-------------------------------------------------------------------
  Function: _atrShutdownLight
  Date: 3/21/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Clean up internal light data at shutdown.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrShutdownLight( void ) {
    _atrInitLight();
}


/*-------------------------------------------------------------------
  Function: _atrUpdateLights
  Date: 5/30/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Set all per-light cache values and function pointers
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void _atrUpdateLights( AtrMaterial *mat ) {
    AtrXform lightToLCS;
    FxU32 accum = 0;
    FxU32 clamp = 0;
    FxU32 mode = (mat->isTwoSided)?1:0;
    _AtrLightNode *n = _atrLightHead;

    if ( mat->sysFlags & ATR_LIGHTFUNC_DIFFUSE ) {
        _atrRenderCache->matBaseColor.r = mat->emissive.r * 255.0f;
        _atrRenderCache->matBaseColor.g = mat->emissive.g * 255.0f;
        _atrRenderCache->matBaseColor.b = mat->emissive.b * 255.0f;
    } else {
        _atrRenderCache->matBaseColor.r = 0.0f;
        _atrRenderCache->matBaseColor.g = 0.0f;
        _atrRenderCache->matBaseColor.b = 0.0f;
    }

    while( n ) {
        if ( n->next == 0 ) clamp = 1;
        n->lf = _atrLightTable[mode][n->light.flags][accum][clamp];
        accum = 1;
        switch( n->light.flags ) {
          case ATR_LIGHT_DIRECTED:
            atrXformInvert( &lightToLCS, _atrCurrentXform );
            atrXformCat( &lightToLCS, &n->light.lcsToWCS, &lightToLCS );
            n->vf[0] = -lightToLCS.data[8];
            n->vf[1] = -lightToLCS.data[9];
            n->vf[2] = -lightToLCS.data[10];
            if ( mat->sysFlags & 
                ATR_LIGHTFUNC_DIFFUSE ) {
                n->ld.r = mat->diffuse.r * n->light.color.r * 255.0f;
                n->ld.g = mat->diffuse.g * n->light.color.g * 255.0f;
                n->ld.b = mat->diffuse.b * n->light.color.b * 255.0f;
            }
            if ( mat->sysFlags & 
                ATR_LIGHTFUNC_SPECULAR ) {
                n->ls.r = mat->specular.r * n->light.color.r * 255.0f;
                n->ls.g = mat->specular.g * n->light.color.g * 255.0f;
                n->ls.b = mat->specular.b * n->light.color.b * 255.0f;
            }
            break;
          case ATR_LIGHT_AMBIENT:
            if ( mat->sysFlags & 
                ATR_LIGHTFUNC_DIFFUSE ) {
                _atrRenderCache->matBaseColor.r += 
                  n->light.color.r * mat->diffuse.r * 255.0f;
                _atrRenderCache->matBaseColor.g += 
                  n->light.color.g * mat->diffuse.g * 255.0f;
                _atrRenderCache->matBaseColor.b += 
                  n->light.color.b * mat->diffuse.b * 255.0f;
            }
            break;
          case ATR_LIGHT_POSITIONAL:
          case ATR_LIGHT_ATTENUATED:
            atrXformInvert( &lightToLCS, _atrCurrentXform );
            atrXformCat( &lightToLCS, &n->light.lcsToWCS, &lightToLCS );
            n->lp[0] = lightToLCS.data[12];
            n->lp[1] = lightToLCS.data[13];
            n->lp[2] = lightToLCS.data[14];
            if ( mat->sysFlags & 
                ATR_LIGHTFUNC_DIFFUSE ) {
                n->ld.r = mat->diffuse.r * n->light.color.r * 255.0f;
                n->ld.g = mat->diffuse.g * n->light.color.g * 255.0f;
                n->ld.b = mat->diffuse.b * n->light.color.b * 255.0f;
            }
            if ( mat->sysFlags & 
                ATR_LIGHTFUNC_SPECULAR ) {
                n->ls.r = mat->specular.r * n->light.color.r * 255.0f;
                n->ls.g = mat->specular.g * n->light.color.g * 255.0f;
                n->ls.b = mat->specular.b * n->light.color.b * 255.0f;
            }
            break;
          case ATR_LIGHT_PROJECTOR:
            {
                /* Update projector matrix */
                AtrXform worldToProjector;
                atrXformInvert(&worldToProjector, 
                               &n->light.lcsToWCS);
                worldToProjector.data[0]  *= n->fovXScale;
                worldToProjector.data[4]  *= n->fovXScale;
                worldToProjector.data[8]  *= n->fovXScale;
                worldToProjector.data[12] *= n->fovXScale;
                worldToProjector.data[1]  *= n->fovYScale;
                worldToProjector.data[5]  *= n->fovYScale;
                worldToProjector.data[9]  *= n->fovYScale;
                worldToProjector.data[13] *= n->fovYScale;
                atrXformCat(&n->modelToProjector,
                            _atrCurrentXform,
                            &worldToProjector);
            }
            break;
        }
        n = n->next;
    } 
    return;
}


/*-------------------------------------------------------------------
  Function: atrAddLight
  Date: 3/21/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Add a light to the scene and associate it with a user specified handle.
  Arguments:
    tag - user specified handle for light
    l - pointer to light description    
  Return:
    none
  -------------------------------------------------------------------*/
void atrAddLight( AtrLight *l, FxU32 tag ) {
    _AtrLightNode *n = _atrLightUnused;
    
#ifdef AT_DEBUGGING
    if ( !l ) 
      atuError( FXTRUE, "atrAddLight(): Invalid parameter.\n" );
#endif
    
    if ( !_atrLightUnused )
      atuError( FXTRUE, "atrAddLight(): Too many lights.  Max: %d\n",
               ATR_MAX_LIGHTS );
    
    
    _atrLightUnused = n->next;
    n->next = _atrLightHead;
    _atrLightHead = n;
    
    if ( l->flags == ATR_LIGHT_PROJECTOR ) {
        n->fovXScale = 
          l->project.nearClip * (float)tan( l->project.fovInX / 2.0f  );
        n->fovYScale = 
          l->project.nearClip * (float)tan( l->project.fovInY / 2.0f  );
    }
      
    n->tag = tag;
    n->light = *l;
    
    _atrUpdateLights( _atrCurrentMaterial );
    
    return;
}

/*-------------------------------------------------------------------
  Function: atrRemoveLight
  Date: 3/21/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Remove all lights from the scene with the give tag.
  Arguments:
    tag - integer handle identifying lights that had been previously
          added
  Return:
    number of lights removed
  -------------------------------------------------------------------*/
FxU32 atrRemoveLight( FxU32 tag ) {
    FxU32 count   = 0;
    _AtrLightNode *lp = _atrLightHead;

    if ( !lp ) return count;

    while( lp->next ) {
        if ( lp->next->tag == tag ) {
            _AtrLightNode *tmp = lp->next;
            lp->next = tmp->next;
            tmp->next = _atrLightUnused;
            _atrLightUnused = tmp;
            count++;
        }
        lp = lp->next;
        if ( !lp ) break;
    }
    if ( _atrLightHead->tag == tag ) {
        lp = _atrLightHead;
        _atrLightHead = lp->next;
        lp->next = _atrLightUnused;
        _atrLightUnused = lp;
        count++;
    }

    _atrUpdateLights( _atrCurrentMaterial );

    return count;
}

/*
**---------------------------------------------------------------
** General Data
**---------------------------------------------------------------
*/
/*
**---------------------------------------------------------------
** Library Init Code
**---------------------------------------------------------------
*/

static void _atrErrorCallback( FxBool fatal, const char *format, ... ) {
    va_list args;
    #if macintosh
    int len;
    extern unsigned char err_buffer[];
    extern int debugger_avail;

    if(debugger_avail) {

        va_start( args, format );
        vsprintf((void*)(err_buffer+1), format, args);
        va_end( args );
        len = strlen((void*)(err_buffer+1));
        if(len > 255) {
            len = 255;
        }
        err_buffer[0] = (unsigned char)len;
        DebugStr(err_buffer);
    }
    #else
    va_start( args, format );
    vfprintf( stdout, format, args );
    va_end( args );
    #endif

    if ( fatal ) {
		if ( _atrDriver != NULL ) {
			DRV_FUNC(Shutdown)();
		}
		exit( -1 );    
    }
	
    return;
}

/*-------------------------------------------------------------------
  Function: atrInit
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Initiliaze the library and hardware graphics subsystem. The specified 
    driver becomes the default context.
  Arguments:
    driverName - which driver to use
    driverInfo - additional information to describe specific configuration
  Return:
    driver context
  TBD:
    Allow either (both) arguments to be NULL, defaulting to default 
    driver and configaration.
  -------------------------------------------------------------------*/
typedef void (*fptr)(int);

static void _atrFpuExceptionHandler( int x ) {
    FXUNUSED( x );
    puts( "FP Exception" );
    exit( 1 );
}

static void 
_txATBErrorCallback( const char *string, FxBool fatal ){
    atuError(fatal, "Texus: %s", string );
}

AtrContext atrInit( char *driverName, AtrDriverInfo *driverInfo) {
    AtrDriverEntry *drv;

    _txErrorCallback = _txATBErrorCallback;

    if ( driverInfo == NULL ) {
        static AtrDriverInfo info;

        driverInfo = &info;
        info.width = 640;
        info.height = 480;
        info.refreshRate = GR_REFRESH_60Hz,
        info.numBuffers = 2; /* double buffer */
        info.smoothingMode = GR_SMOOTHING_ENABLE;
        info.fullScreen = 1;
        info.info = NULL;
    }

    _atrDriver = NULL;
 
    if ( driverName == NULL ) {
        drv = driverTable;
    } else {
        for ( drv = driverTable; drv->name != NULL; drv++ ) {
            if ( atuStringCompare(driverName, drv->name)) {
               break;
            }
        }
    }

    if ( drv->name == NULL ) {
        atuError(FXTRUE, "atrInit: can't find driver %s\n", 
                 driverName ? driverName : "Unspecified");
    }

    /* Floating Point Exception Handler */
    /* signal( SIGFPE, _atrFpuExceptionHandler ); */

    driverInfo->driverName = atuStrDup(drv->name);

    if ((_atrDriver = (drv->init)(driverInfo)) == NULL ) {
        atuError( FXTRUE, "atrInit: can't initialize driver\n" );
    }

    atrRenderBuffer( ATR_BUFFER_BACKBUFFER );

    _atrInitRenderCache();
    _atrInitXform();
    _atrInitCamera();
    _atrInitCanvas();
    _atrInitLight();
    _atrInitEnv();
    _atrInitMaterial();

#   ifdef AT_STATISTICS
    atrStatsReset();
#   endif
    return _atrDriver;
}

/*-------------------------------------------------------------------
  Function: atrIsDrawable
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Determine if a context can currently be drawn to
  Arguments:
    ctx - the rendering context
  Return:
    FXTRUE if the context can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool atrIsDrawable( AtrContext ctx ) {
    return DRV_FUNC(IsDrawable)( ctx );
}

/*-------------------------------------------------------------------
  Function: atrPause
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Application is pausing, display primary window surface
  Arguments:
    flag - FXTRUE if application is pausing, FXFALSE otherwise
  Return:
    FXTRUE if the pause was succesful, false otherwise
  -------------------------------------------------------------------*/

FxBool
atrPause(FxBool flag) {
	if ( _atrDriver != NULL )
		return DRV_FUNC(Pause)(flag);

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrResize
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Resizes the resources associated with a graphics context
  Arguments:
    ctx - the rendering context
    w   - new width
    h   - new height
  Return:
    FXTRUE if the resize was succesful, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool
atrResize(AtrContext ctx, int w, int h) {
    return DRV_FUNC(Resize)(ctx, w, h) ;
}

/*-------------------------------------------------------------------
  Function: atrMove
  Date: 3/14/97
  Implementor(s): mlwp
  Library: AT Render
  Description:
    move the resources associated with a graphics context
  Arguments:
    ctx - the rendering context
    x   - new x coordinate
    y   - new y coordinate
  Return:
    FXTRUE if the move was succesful, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool
atrMove(AtrContext ctx, int x, int y) {
    return DRV_FUNC(Move)(ctx, x, y) ;
}

/*-------------------------------------------------------------------
  Function: atrShutdown
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Shutdown the library and graphics subsystem.  Also free all resources
    allocated by library.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/

void atrShutdown( void ) {

    if ( ( _atrDriver != NULL ) && !getenv( "AT_NOSHUTDOWN" ) )
        DRV_FUNC(Shutdown)();

    _atrDriver = NULL;

    _atrShutdownRenderCache();
    _atrShutdownXform();
    _atrShutdownCamera();
    _atrShutdownCanvas();
    _atrShutdownLight();
    _atrShutdownEnv();
    _atrShutdownMaterial();
    return;
}


/*-------------------------------------------------------------------
  Function: atrQueryDriverCaps
  Date: 6/10
  Implementor(s): jdt
  Library: AT Render
  Description:
    Fill in a data structure describing hardware detected by 
    atr on initialization.  Must be called between atrInit and
    atrShutdown
  Arguments:
    ctx  - driver context
    caps - pointer to AtrDriverCaps structure
  Return:
    none
  -------------------------------------------------------------------*/


void atrQueryDriverCaps( AtrDriverCaps *caps ) {
#   ifdef AT_DEBUGGING
    if ( !caps ) 
        atuError( FXTRUE, "atrQueryDriverCaps: invalid caps pointer.\n" );
    if ( _atrDriver == NULL )
        atuError( FXTRUE, "atrQueryDriverCaps: is only valid when library "
                  "has posession of hardware.\n"
                  "Bracket calls in atrInit()/atrShutdown().\n" );
#   endif
    *caps = _atrDriver->caps;
    return;
}



/*-------------------------------------------------------------------
  Function: atrExit
  Date: 4/26/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Save all renderer state for this instance of the
    arcade toolbox.  No library operations may be performed
    until a matching call to atrEnter() is made.  These two
    functions allow multiple arcade toolbox applications to
    run simultaneously.  Exit/Enter pairs are very expensive.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void atrExit( void ) {
    _atrSavedDriver = _atrDriver;
    _atrDriver = NULL;
}

/*-------------------------------------------------------------------
  Function: atrEnter
  Date: 4/26/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Restore renderer state after a call to atrExit has
    saved the state. See atrExit().
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void atrEnter( void ) {
    _atrDriver = _atrSavedDriver;
}

/*-------------------------------------------------------------------
  Function: atrRenderBuffer
  Date: 4/26/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Set the buffer for all subsequent rendering operations.
  Arguments:
    buf - buffer to render to, one of
      ATR_BUFFER_BACKBUFFER   - this is the default
      ATR_BUFFER_FRONTBUFFER
  Return:
    none
  -------------------------------------------------------------------*/

void atrRenderBuffer( AtrBuffer buf ) {
#ifdef AT_DEBUGGING
    if ( buf > GR_BUFFER_BACKBUFFER )
      atuError( FXTRUE, "atrRenderBuffer: invalid buffer.\n" );
#endif    
    _atrRenderBuffer = buf;
    DRV_FUNC(RenderBuffer)( buf );
}

/*-------------------------------------------------------------------
  Function: atrDitherMode
  Date: 11/27/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Set the dither mode
  Arguments:
    mode - dither mode to render with, one of
      ATR_DITHER_DISABLE - this is the default
      ATR_DITHER_2x2
      ATR_DITHER_4x4
  Return:
    none
  -------------------------------------------------------------------*/

void atrDitherMode( AtrDitherMode mode ) {
#ifdef AT_DEBUGGING
    if ( mode > ATR_DITHER_4x4 )
      atuError( FXTRUE, "atrDitherMode: invalid dither mode 0x%lx.\n", mode );
#endif    
    DRV_FUNC(DitherMode)( mode );
}
