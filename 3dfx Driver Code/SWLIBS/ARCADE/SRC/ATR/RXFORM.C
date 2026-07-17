/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 7:34:24 PM$ 
**
*/

#include "atrender.h"
#include <string.h>
#include <math.h>

/*
**-----------------------------------------------------------------
** File Scope Data
**-----------------------------------------------------------------
*/

static const char  _dataType[]     = "atrXform";
static const FxU32 _binaryRevision = 0;


/*-------------------------------------------------------------------
  Function: atrXformAllocate
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate and initialize a Xform, which is just a wrapper for
    a 4x4 Matrix.  Matrix allocations are aligned just like vertices.
  Arguments:
    num - number of transforms to allocate and assign to the pointer
  Return:
    new array of xforms
  -------------------------------------------------------------------*/
AtrXform *atrXformAllocate( FxU32 num ) {
    AtrXform *t;
    FxU32 index;

#ifdef AT_DEBUGGING
    if ( num == 0 ) 
        atuError( FXTRUE, "atrXformAllocate(): Invalid parameter.\n" );
#endif

    t = ( AtrXform * ) atuMemAlignedCalloc( sizeof( AtrXform ), 
                                            num, 32 );
    for ( index = 0; index < num; index++ ) {
        atrXformSetIdentity( t + index );
    }
    return t;
}

/*-------------------------------------------------------------------
  Function: atrXformDeallocate
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Safely deallocate arrays of atrXforms.
  Arguments:
    t - pointer to transforms
    num - number of transforms to deallocate
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformDeallocate( AtrXform t[] ) {

#ifdef AT_DEBUGGING
    if ( !t )
        atuError( FXTRUE, "atrXformDeallocate(): Invalid parameter.\n" );
#endif

    atuMemFree( t );
}

/*-------------------------------------------------------------------
  Function: AtrXformDefault
  Date: 4/16
  Implementor(s): jdt
  Library: AT Render
  Description:
    Initialize data structure to default values

    For Xform
        Data Structure Initialized to identity;
  Arguments:
    t - xform to initialize
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformDefault( AtrXform *t ) {
#ifdef AT_DEBUGGING
    if ( !t )
        atuError( FXTRUE, "atrXformDefault(): Invalid parameter.\n" );
#endif
    atrXformSetIdentity( t );
    return;
}


/*-------------------------------------------------------------------
  Function: atrXformStore
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Store a transform out to a file stream.
  Arguments:
    stream - file stream
    t - transform to store
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrXformStore( AtrXform *t, FILE *stream ) {
    FxU32 count;
#ifdef AT_DEBUGGING
    if ( !t || !stream ) 
        atuError( FXTRUE, "atrXformStore(): Invalid parameter.\n" );
#endif    
    count = fwrite( &_binaryRevision, 
                    sizeof( _binaryRevision ), 
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;
    count = fwrite( _dataType,
                    sizeof( _dataType ), 
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;

    count = fwrite( t,
                    sizeof( AtrXform ),
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrXformLoad
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Load a transform from a file stream.
  Arguments:
    stream - file stream
    t - memory into which to load transform
  Return:
    FXTRUE  - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrXformLoad( AtrXform *t, FILE *stream ) {
    FxU32 rev;
    char  type[64];
    FxU32 count;

#ifdef AT_DEBUGGING
    if ( !t || !stream ) 
        atuError( FXTRUE, "atrXformLoad(): Invalid parameter.\n" );
#endif    

    count = fread( &rev, sizeof( _binaryRevision ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( rev != _binaryRevision ) return FXFALSE;
    count = fread( type, sizeof( _dataType ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( strcmp( type, _dataType ) ) return FXFALSE;
    count = fread( t, sizeof( AtrXform ), 1, stream );
    if ( count != 1 ) return FXFALSE;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrXformAssign
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Assign one transform to another.
  Arguments:
    src - source transform
    dest - destination transform
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformAssign( AtrXform *dest, AtrXform *src ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src )
        atuError( FXTRUE, "atrXformAssign(): Invalid paramter.\n" );
#endif
    *dest = *src;
    return;
}

/*-------------------------------------------------------------------
  Function: atrXformClone
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate a new transform and initialize it from the provided
    transform.
  Arguments:
    src - transform to be cloned
  Return:
    clone of src
  -------------------------------------------------------------------*/
AtrXform *atrXformClone( AtrXform *src ) {
    AtrXform *tmp;

#ifdef AT_DEBUGGING
    if ( !src )
        atuError( FXTRUE, "atrXformClone(): Invalid paramter.\n" );
#endif

    tmp = atrXformAllocate( 1 );
    atmMatrix4x4Assign( tmp->data, src->data );
    return tmp;
}

/*-------------------------------------------------------------------
  Function: atrXformPrint
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Format and print a transform to an output stream
  Arguments:
    indent - spaces to append to all output
    stream - output stream
    t - transform to format and print
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformPrint( AtrXform *t,
                        FILE *stream, 
                        FxU32 indent ) {
    
#ifdef AT_DEBUGGING
    if ( !t || !stream )
        atuError( FXTRUE, "atrXformPrint(): Invalid paramter.\n" );
#endif

    fprintf( stream, "%*sXform:\n", indent, " " );
    atmMatrix4x4Print( stream, t->data, indent );

    return;
}

/*-------------------------------------------------------------------
  Function: atrXformSetIdentity
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Set a transform to the identity and relocate to the origin
  Arguments:
    t - transform to reset
  Return:
    none
  -------------------------------------------------------------------*/    
void atrXformSetIdentity( AtrXform *t ) {

#ifdef AT_DEBUGGING
    if ( !t )
        atuError( FXTRUE, "atrXformSetIdentity(): Invalid paramter.\n" );
#endif

    atmMatrix4x4SetIdentity( t->data );
    return;
}

/*-------------------------------------------------------------------
  Function: atrXformInvert
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Invert a transform and assign it to another
  Arguments:
    dest - memory target for invertex matrix
    src - transform to be inverted
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformInvert( AtrXform *dest, AtrXform *src ) {

#ifdef AT_DEBUGGING
    if ( !src || !dest )
        atuError( FXTRUE, "atrXformSetInvert(): Invalid paramter.\n" );
#endif

    atmMatrix4x4Invert( dest->data, src->data );
    return;
}

/*-------------------------------------------------------------------
  Function: atrXformCat
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Concatenate two transforms and store the result in a third
  Arguments:
    right - right hand transform in multiplication
    left - left hand transform in multiplication
    dest - destination transform
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformCat( AtrXform *dest, 
                      AtrXform *left, 
                      AtrXform *right ) {

#ifdef AT_DEBUGGING
    if ( !dest || !left || !right )
        atuError( FXTRUE, "atrXformCat(): Invalid paramter.\n" );
#endif

    atmMatrix4x4Cat( dest->data, left->data, right->data );
}

/*-------------------------------------------------------------------
  Function: atrXformSetXRotation
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Set the transform to the specified rotation about the x axis.  The
    translational component of the matrix is unaffected.
  Arguments:
    radians - angle to rotate in radians
    t - transform
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformSetXRotation( AtrXform *t, float radians ) {

#ifdef AT_DEBUGGING
    if ( !t )
        atuError( FXTRUE, "atrXformSetXRotation(): Invalid paramter.\n" );
#endif

    atmMatrix4x4SetXRotation( radians, t->data );
}

/*-------------------------------------------------------------------
  Function: atrXformSetYRotation
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Set the transform to the specified rotation about the x axis.  The
    translational component of the matrix is unaffected.
  Arguments:
    radians - angle to rotate in radians
    t - transform
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformSetYRotation( AtrXform *t, float radians ) {

#ifdef AT_DEBUGGING
    if ( !t )
        atuError( FXTRUE, "atrXformSetYRotation(): Invalid paramter.\n" );
#endif

    atmMatrix4x4SetYRotation( radians, t->data );
}

/*-------------------------------------------------------------------
  Function: atrXformSetZRotation
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Set the transform to the specified rotation about the x axis.  The
    translational component of the matrix is unaffected.
  Arguments:
    radians - angle to rotate in radians
    t - transform
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformSetZRotation( AtrXform *t, float radians ) {

#ifdef AT_DEBUGGING
    if ( !t )
        atuError( FXTRUE, "atrXformSetZRotation(): Invalid paramter.\n" );
#endif

    atmMatrix4x4SetZRotation( radians, t->data );
}

/*-------------------------------------------------------------------
  Function: atrXformSetRotation
  Date: 4/30/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Create a rotation about an arbitrary vector
  Arguments:
    t - destination for new rotation transform
    v - vector to use as an axis of rotation
    radians - angle of rotation ( clockwise
              if v were pointing directly at the viewer )
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformSetRotation( AtrXform *t, AtmVector3 v, float radians ) {
    AtmMatrix3x3 s, uut, r;
    static AtmMatrix3x3 I = { 1.0f, 0.0f, 0.0f,
                              0.0f, 1.0f, 0.0f,
                              0.0f, 0.0f, 1.0f };
    AtmVector3   u;

    /* u = norm( v ) */
    atmVector3Normalize( u, v );

    /* s =  0   z' -y'
           -z'  0   x'
            y' -x'  0
    */
#   define x (u[0])
#   define y (u[1])
#   define z (u[2])

    s[0] = 0.0f, s[1] =    z, s[2] =   -y;
    s[3] =   -z, s[4] = 0.0f, s[5] =    x;
    s[6] =    y, s[7] =   -x, s[8] = 0.0f;

    /* uut = u^t * u */

    uut[0] = x*x, uut[1] = x*y, uut[2] = x*z;
    uut[3] = y*x, uut[4] = y*y, uut[5] = y*z;
    uut[6] = z*x, uut[7] = z*y, uut[8] = z*z;

#   undef x
#   undef y
#   undef z

    /* R = uut + cos(radians)(I-uut) + sin(radians)S */
    atmMatrix3x3Scale( s, s, (float)sin( radians ) );
    atmMatrix3x3Sub( r, I, uut );
    atmMatrix3x3Scale( r, r, (float)cos( radians ) );
    atmMatrix3x3Add( r, r, uut );
    atmMatrix3x3Add( r, r, s );
    t->data[0]  = r[0];
    t->data[1]  = r[1];
    t->data[2]  = r[2];
    t->data[3]  = 0.0f;
    t->data[4]  = r[3];
    t->data[5]  = r[4];
    t->data[6]  = r[5];
    t->data[7]  = 0.0f;
    t->data[8]  = r[6];
    t->data[9]  = r[7];
    t->data[10] = r[8];
    t->data[11] = 0.0f;
    t->data[12] = 0.0f;
    t->data[13] = 0.0f;
    t->data[14] = 0.0f;
    t->data[15] = 1.0f;
    return;
}

/*-------------------------------------------------------------------
  Function: atrXformSetTranslation
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Create a transform that translation by the specified displacements
    in x, y, and z.
  Arguments:
    x, y, z - elementary components of translation
    t - transform to initialize
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformSetTranslation( AtrXform *t, float x, float y, float z ) {

#ifdef AT_DEBUGGING
    if ( !t )
        atuError( FXTRUE, "atrXformSetTranslation(): Invalid paramter.\n" );
#endif

    t->data[0]  = 1.0f;
    t->data[1]  = 0.0f;
    t->data[2]  = 0.0f;
    t->data[3]  = 0.0f;

    t->data[4]  = 0.0f;
    t->data[5]  = 1.0f;
    t->data[6]  = 0.0f;
    t->data[7]  = 0.0f;

    t->data[8]  = 0.0f;
    t->data[9]  = 0.0f;
    t->data[10] = 1.0f;
    t->data[11] = 0.0f;

    t->data[12] = x;
    t->data[13] = y;
    t->data[14] = z;
    t->data[15] = 1.0f;
}

/*-------------------------------------------------------------------
  Function: atrXformVectorMult
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Multiply a vector against a transform and store it in another
    vector.
  Arguments:
    t - transform
    src - source vector
    dest - destination vector
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformVectorMult( AtmVector3 dest, 
                         AtmVector3 src, 
                         AtrXform *t ) {
    AtmVector3 tmp;
#ifdef AT_DEBUGGING
    if ( !t || !src || !dest )
        atuError( FXTRUE, "atrXformVectorMult(): Invalid paramter.\n" );
#endif
    tmp[0]   = src[0] * t->data[0] + 
               src[1] * t->data[4] + 
               src[2] * t->data[8] +
               t->data[12];
    tmp[1]   = src[0] * t->data[1] + 
               src[1] * t->data[5] + 
               src[2] * t->data[9] +
               t->data[13];
    tmp[2]   = src[0] * t->data[2]  + 
               src[1] * t->data[6]  + 
               src[2] * t->data[10] +
               t->data[14];
    dest[0] = tmp[0];
    dest[1] = tmp[1];
    dest[2] = tmp[2];
    return;
}

/*-------------------------------------------------------------------
  Function: atrXformPointAt
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Create a transform that points points ( a camera for example ) 
    towards the target point.  A source location, target location,
    and up vector need to be specified.
  Arguments:
    up     - up vector ( must be normalized )
    target - point in space to point at
    source - desired location of viewpoint
    t      - transform to modify
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformPointAt( AtrXform *t, AtmVector3 target, 
                      AtmVector3 source, AtmVector3 up ) {
    float mag;

/*
**  R - &t->data[0]
**  U - &t->data[4]
**  F - &t->data[8]
**  T - &t->data[12]
**  right = up X forward
**  up = forward X right
*/
    atmVector3Sub( &t->data[8], target, source );
    mag = atmVector3Magnitude( &t->data[8] );
    if ( mag == 0.0f ) return;

    atmVector3Scale(  &t->data[8],  &t->data[8], 1.0f / mag );
    atmVector3Cross(  &t->data[0],  up,          &t->data[8] );
    atmVector3Normalize( &t->data[0], &t->data[0] );
    atmVector3Cross(  &t->data[4],  &t->data[8], &t->data[0] );
    atmVector3Assign( &t->data[12], source );

    t->data[3]  = t->data[7] = t->data[11] = 0.0f;
    t->data[15] = 1.0f;

    return;
}


/*-------------------------------------------------------------------
  Function: atrXformReturnToOrthonormal
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Make the rotational component of the src transformation an
    orthonormal basis in R^3.  This has no effect on the translational
    component of the matrix.

    This is sometimes useful to correct for "creep" that occurs
    when accumulating transformation over many hundreds of frames
    of animation.

    This operation is incompatible with scaling of any kind, as
    well as non orientation preserving transformations.

  Arguments:
    src - source transform
    dest - destination transform
  Return:
    none
  -------------------------------------------------------------------*/
void atrXformReturnToOrthonormal( AtrXform *dest, AtrXform *src ) {
    AtmVector3 right, up, forward;
/*
**  R - &t->data[0]
**  U - &t->data[4]
**  F - &t->data[8]
**  T - &t->data[12]
**  right = up X forward
**  up = forward X right
*/

    atmVector3Normalize( forward, &src->data[8] );
    atmVector3Normalize( up,      &src->data[4] );

    atmVector3Cross( right, up,      forward );
    atmVector3Cross( up,    forward, right );

    atmVector3Assign( &dest->data[0], right );
    atmVector3Assign( &dest->data[4], up );
    atmVector3Assign( &dest->data[8], forward );

    return;    
}

