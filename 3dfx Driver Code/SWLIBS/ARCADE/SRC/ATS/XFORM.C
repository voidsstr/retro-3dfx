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
** $Date: 10/11/00 7:34:47 PM$ 
**
*/
    
#include <math.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include "atscenep.h"

/*
**---------------------------------------------------------------
** Transform Stack for collision detection and bounding volume
** calculations in place of the atr equivalent functions which
** assume the initialization of the graphics hardware. For some
** tools initialization (or existence) of the hardware is not
** desirable.
**---------------------------------------------------------------
*/
static AtuStack *_atsXformStack = 0;
static AtrXform *_atsCurrentXform = 0;

/*-------------------------------------------------------------------
  Function: _atsInitXform
  Date: 6/23/96
  Implementor(s): jdt, mlwp
  Library: AT Scene manager
  Description:
    Private function to initialize the transform stack. 
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void _atsInitXform( void ) {
    AtrXform base;
    _atsXformStack = atuStackAllocate( 1 );
    atrXformSetIdentity( &base );
    atuStackInit( _atsXformStack, sizeof( AtrXform ), 16 );
    atuStackPush( _atsXformStack, &base );
    _atsCurrentXform = 
        (AtrXform*)(_atsXformStack->top - _atsXformStack->elementSize);
    return;
}

/*-------------------------------------------------------------------
  Function: _atsShutdownXform
  Date: 6/26/96
  Implementor(s): jdt, mlwp
  Library: AT Scene manager
  Description:
    Private function to free the transform stack. 
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void _atsShutdownXform( void ) {
    atuStackDeallocate( _atsXformStack );
    _atsXformStack = 0;
    _atsCurrentXform = 0;
}

/*-------------------------------------------------------------------
  Function: atsPushXform
  Date: 6/26/96
  Implementor(s): jdt, mlwp
  Library: AT Scene manager
  Description:
    Push a transform on the transform stack.
  Arguments:
    t - transform to push
  Return:
    none
  -------------------------------------------------------------------*/
void atsPushXform( AtrXform *t ){
#ifdef AT_DEBUGGING
    if ( !t ) 
        atuError( FXFALSE, "atsPushXform(): Invalid parameter.\n" );
#endif
    atuStackPush( _atsXformStack, t );
    _atsCurrentXform = 
        (AtrXform*)(_atsXformStack->top - _atsXformStack->elementSize);

    return;
}

/*-------------------------------------------------------------------
  Function: atsPopXform
  Date: 6/26/96
  Implementor(s): jdt, mlwp
  Library: AT Scene manager
  Description:  
    Pop the top of the Xform stack.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void atsPopXform( void ) {

    /* Don't allow stack to pop below 1 element */
    if ( _atsXformStack->top > 
         _atsXformStack->base + _atsXformStack->elementSize )
        _atsXformStack->top -= _atsXformStack->elementSize;
#ifdef AT_DEBUGGING
    else 
        atuError( FXFALSE, "atsPopXform(): Stack underflow.\n" );
#endif
    _atsCurrentXform = 
        (AtrXform*)(_atsXformStack->top - _atsXformStack->elementSize);

    return;
}

/*-------------------------------------------------------------------
  Function: atsPostCatPushXform
  Date: 6/26/96
  Implementor(s): jdt, mlwp
  Library: AT Scene manager
  Description:  
    Copy the top of the stack and post cat the argument transform
  Arguments:
    t - transform
  Return:
    none
  -------------------------------------------------------------------*/
void atsPostCatPushXform( AtrXform *t ) {
#ifdef AT_DEBUGGING
    if ( !t ) 
        atuError( FXFALSE, "atsPostCatPushXform(): Invalid parameter.\n" );
#endif
    atuStackPush( _atsXformStack, _atsCurrentXform );
    _atsCurrentXform = 
        (AtrXform*)(_atsXformStack->top - _atsXformStack->elementSize);
    atrXformCat( _atsCurrentXform, _atsCurrentXform, t );

    return;
}

/*-------------------------------------------------------------------
  Function: atsPreCatPushXform
  Date: 6/26/96
  Implementor(s): jdt, mlwp
  Library: AT Scene manager
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
void atsPreCatPushXform( AtrXform *t ) {
    AtmMatrix4x4 temp;
#ifdef AT_DEBUGGING
    if ( !t ) 
        atuError( FXFALSE, "atsPreCatPushXform(): Invalid parameter.\n" );
#endif
    atuStackPush( _atsXformStack, _atsCurrentXform );
    _atsCurrentXform = 
        (AtrXform*)(_atsXformStack->top - _atsXformStack->elementSize);

#if 1
    temp[0] = t->data[0] * _atsCurrentXform->data[0] +
              t->data[1] * _atsCurrentXform->data[4] +
              t->data[2] * _atsCurrentXform->data[8];

    temp[1] = t->data[0] * _atsCurrentXform->data[1] +
              t->data[1] * _atsCurrentXform->data[5] +
              t->data[2] * _atsCurrentXform->data[9];

    temp[2] = t->data[0] * _atsCurrentXform->data[2] +
              t->data[1] * _atsCurrentXform->data[6] +
              t->data[2] * _atsCurrentXform->data[10];

    temp[4] = t->data[4] * _atsCurrentXform->data[0] +
              t->data[5] * _atsCurrentXform->data[4] +
              t->data[6] * _atsCurrentXform->data[8];

    temp[5] = t->data[4] * _atsCurrentXform->data[1] +
              t->data[5] * _atsCurrentXform->data[5] +
              t->data[6] * _atsCurrentXform->data[9];

    temp[6] = t->data[4] * _atsCurrentXform->data[2] +
              t->data[5] * _atsCurrentXform->data[6] +
              t->data[6] * _atsCurrentXform->data[10];

    temp[8]  = t->data[8]  * _atsCurrentXform->data[0] +
               t->data[9]  * _atsCurrentXform->data[4] +
               t->data[10] * _atsCurrentXform->data[8];

    temp[9]  = t->data[8]  * _atsCurrentXform->data[1] +
               t->data[9]  * _atsCurrentXform->data[5] +
               t->data[10] * _atsCurrentXform->data[9];

    temp[10] = t->data[8]  * _atsCurrentXform->data[2] +
               t->data[9]  * _atsCurrentXform->data[6] +
               t->data[10] * _atsCurrentXform->data[10];

    _atsCurrentXform->data[12] += t->data[12] * _atsCurrentXform->data[0] +
                                  t->data[13] * _atsCurrentXform->data[4] +
                                  t->data[14] * _atsCurrentXform->data[8]; 
    _atsCurrentXform->data[13] += t->data[12] * _atsCurrentXform->data[1] +
                                  t->data[13] * _atsCurrentXform->data[5] +
                                  t->data[14] * _atsCurrentXform->data[9]; 
    _atsCurrentXform->data[14] += t->data[12] * _atsCurrentXform->data[2] +
                                  t->data[13] * _atsCurrentXform->data[6] +
                                  t->data[14] * _atsCurrentXform->data[10];

    _atsCurrentXform->data[0]  = temp[0];
    _atsCurrentXform->data[1]  = temp[1];
    _atsCurrentXform->data[2]  = temp[2];
    _atsCurrentXform->data[3]  = 0.0f;
    _atsCurrentXform->data[4]  = temp[4];
    _atsCurrentXform->data[5]  = temp[5];
    _atsCurrentXform->data[6]  = temp[6];
    _atsCurrentXform->data[7]  = 0.0f;
    _atsCurrentXform->data[8]  = temp[8];
    _atsCurrentXform->data[9]  = temp[9];
    _atsCurrentXform->data[10] = temp[10];
    _atsCurrentXform->data[11] = 0.0f;
    _atsCurrentXform->data[15] = 1.0f;
#else
    atsXformCat( _atsCurrentXform, t, _atsCurrentXform );
#endif

    return;
}

/*-------------------------------------------------------------------
  Function: atsQueryXform
  Date: 6/26/96
  Implementor(s): jdt, mlwp
  Library: AT Scene manager
  Description:
    Copies the top of the tranform stack into the memory location
    provided.
  Arguments:
    t - a pointer to an empty atsXform
  Return:
    none
  -------------------------------------------------------------------*/
void atsQueryXform( AtrXform *t ) {
#ifdef AT_DEBUGGING
    if ( !t ) 
        atuError( FXFALSE, "atsQueryXform(): Invalid parameter.\n" );
#endif
    atrXformAssign( t, _atsCurrentXform );
    return;
}
