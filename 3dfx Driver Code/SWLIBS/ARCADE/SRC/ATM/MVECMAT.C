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
** $Date: 10/11/00 7:33:43 PM$ 
**
*/

#include "atmath.h"
#include <atutil.h>
#include <math.h>

/*-------------------------------------------------------------------
  Function: atmOOSqrt
  Date: 4/9/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    fast one over sqrt approximation.  Mystic tarolli implementation.
  Arguments:
    number - number to take 1.0f / sqrt( x ) on
  Return:
    appx to 1.0f / sqrt( number )
  -------------------------------------------------------------------*/
float atmOOSqrt( float number ) {
    long i;
    float x2, y;
    float threehalfs = 1.5F;

    x2 = number * (float)0.5F;
    y = number;
    i = *(long *) &y;
    i = 0x5f3759df - (i>>1);
    y = *(float *)&i;

    y = y * (threehalfs - (x2 * y * y));
    y = y * (threehalfs - (x2 * y * y));
    return y;
}

/*-------------------------------------------------------------------
  Function: atmVatrix3x3SetIdentity
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Set the argument matrix to the identity matrix.
  Arguments:
    m - matrix to be cleared
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix3x3SetIdentity( AtmMatrix3x3 m ) {
#ifdef AT_DEBUGGING
    if ( !m ) 
        atuError( FXTRUE, "atmMatrix3x3SetIdentity(): Invalid parameter.\n" );
#endif
	m[0] = 1.0f, m[1] = 0.0f, m[2] = 0.0f;
	m[3] = 0.0f, m[4] = 1.0f, m[5] = 0.0f;
	m[6] = 0.0f, m[7] = 0.0f, m[8] = 1.0f;
	return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix4x4SetIdentity
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Set the argument matrix to the identity matrix.
  Arguments:
    m - matrix to be cleared
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix4x4SetIdentity( AtmMatrix4x4 m ) {
#ifdef AT_DEBUGGING
    if ( !m ) 
        atuError( FXTRUE, "atmMatrix4x4SetIdentity(): Invalid parameter.\n" );
#endif
    m[0]  = 1.0f, m[1]  = 0.0f, m[2]  = 0.0f, m[3]  = 0.0f;
    m[4]  = 0.0f, m[5]  = 1.0f, m[6]  = 0.0f, m[7]  = 0.0f;
    m[8]  = 0.0f, m[9]  = 0.0f, m[10] = 1.0f, m[11] = 0.0f;
    m[12] = 0.0f, m[13] = 0.0f, m[14] = 0.0f, m[15] = 1.0f;
    return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix3x3SetXRotation
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Sets a 3x3 matrix to a coordinate system that is rotated clockwise
    about the x-axis by a specified number of radians
  Arguments:
    radians - number of radians to rotate by
    m - matrix to set.
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix3x3SetXRotation( float radians, AtmMatrix3x3 m ) {
    float c = (float)cos( radians );
    float s = (float)sin( radians );

#ifdef AT_DEBUGGING
    if ( !m ) 
        atuError( FXTRUE, "atmMatrix3x3SetXRotation(): Invalid parameter.\n" );
#endif

	m[0] = 1.0f, m[1] = 0.0f, m[2] = 0.0f;
    m[3] = 0.0f, m[4] =    c, m[5] = s;   
    m[6] = 0.0f, m[7] =   -s, m[8] = c;
    return;
}


/*-------------------------------------------------------------------
  Function: atmMatrix3x3SetYRotation
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Sets a 3x3 matrix to a coordinate system that is rotated clockwise
    about the y-axis by a specified number of radians
  Arguments:
    radians - number of radians to rotate by
    m - matrix to set.
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix3x3SetYRotation( float radians, AtmMatrix3x3 m ) {
    float c = (float)cos( radians );
    float s = (float)sin( radians );

#ifdef AT_DEBUGGING
    if ( !m ) 
        atuError( FXTRUE, "atmMatrix3x3SetYRotation(): Invalid parameter.\n" );
#endif

    m[0] =    c, m[1] = 0.0f, m[2] =   -s;
    m[3] = 0.0f, m[4] = 1.0f, m[5] = 0.0f;
    m[6] =    s, m[7] = 0.0f, m[8] =    c;
    return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix3x3SetZRotation
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Sets a 3x3 matrix to a coordinate system that is rotated clockwise
    about the z-axis by a specified number of radians
  Arguments:
    radians - number of radians to rotate by
    m - matrix to set.
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix3x3SetZRotation( float radians, AtmMatrix3x3 m ) {
    float c = (float)cos( radians );
    float s = (float)sin( radians );

#ifdef AT_DEBUGGING
    if ( !m ) 
        atuError( FXTRUE, "atmMatrix3x3SetZRotation(): Invalid parameter.\n" );
#endif

    m[0] =    c, m[1] =    s, m[2] = 0.0f;
    m[3] =   -s, m[4] =    c, m[5] = 0.0f;
    m[6] = 0.0f, m[7] = 0.0f, m[8] = 1.0f;
    return;
}  

/*-------------------------------------------------------------------
  Function: atmMatrix4x4SetXRotation
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Sets a 4x4 matrix to a coordinate system that is rotated clockwise
    about the x-axis by a specified number of radians
  Arguments:
    radians - number of radians to rotate by
    m - matrix to set.
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix4x4SetXRotation( float radians, AtmMatrix4x4 m ) {
    float c = (float)cos( radians );
    float s = (float)sin( radians );

#ifdef AT_DEBUGGING
    if ( !m ) 
        atuError( FXTRUE, "atmMatrix4x4SetXRotation(): Invalid parameter.\n" );
#endif

	m[0]  = 1.0f, m[1]  = 0.0f, m[2]  = 0.0f;
    m[4]  = 0.0f, m[5]  =    c, m[6]  = s;   
    m[8]  = 0.0f, m[9]  =   -s, m[10] = c;
    m[12] = 0.0f, m[13] = 0.0f, m[14] = 0.0f;
    m[3] = m[7] = m[11] = 0.0f;
    m[15] = 1.0f;
    return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix4x4SetYRotation
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Sets a 4x4 matrix to a coordinate system that is rotated clockwise
    about the y-axis by a specified number of radians
  Arguments:
    radians - number of radians to rotate by
    m - matrix to set.
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix4x4SetYRotation( float radians, AtmMatrix4x4 m ) {
    float c = (float)cos( radians );
    float s = (float)sin( radians );

#ifdef AT_DEBUGGING
    if ( !m ) 
        atuError( FXTRUE, "atmMatrix4x4SetYRotation(): Invalid parameter.\n" );
#endif

    m[0] =    c, m[1] = 0.0f, m[2]  =   -s;
    m[4] = 0.0f, m[5] = 1.0f, m[6]  = 0.0f;
    m[8] =    s, m[9] = 0.0f, m[10] =    c;
    m[12] = 0.0f, m[13] = 0.0f, m[14] = 0.0f;
    m[3] = m[7] = m[11] = 0.0f;
    m[15] = 1.0f;
    return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix4x4SetZRotation
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Sets a 4x4 matrix to a coordinate system that is rotated clockwise
    about the z-axis by a specified number of radians
  Arguments:
    radians - number of radians to rotate by
    m - matrix to set.
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix4x4SetZRotation( float radians, AtmMatrix4x4 m ) {
    float c = (float)cos( radians );
    float s = (float)sin( radians );

#ifdef AT_DEBUGGING
    if ( !m ) 
        atuError( FXTRUE, "atmMatrix4x4SetZRotation(): Invalid parameter.\n" );
#endif

    m[0] =    c, m[1] =    s, m[2]  = 0.0f;
    m[4] =   -s, m[5] =    c, m[6]  = 0.0f;
    m[8] = 0.0f, m[9] = 0.0f, m[10] = 1.0f;
    m[12] = 0.0f, m[13] = 0.0f, m[14] = 0.0f;
    m[3] = m[7] = m[11] = 0.0f;
    m[15] = 1.0f;
    return;
}  

/*-------------------------------------------------------------------
  Function: atmMatrix3x3Cat
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Concatenate two 3x3 matrices and store result in a third.
  Arguments:
    mRight - right hand matrix in multiplication
    mLeft - left hand matrix in multiplication
    mDest - destination matrix where result is stored
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix3x3Cat( AtmMatrix3x3 mDest, 
                      AtmMatrix3x3 mLeft, 
                      AtmMatrix3x3 mRight ) {
	AtmMatrix3x3 temp;

#ifdef AT_DEBUGGING
    if ( !mDest || !mLeft || !mRight ) 
        atuError( FXTRUE, "atmMatrix3x3Cat(): Invalid parameter.\n" );
#endif

	temp[0] = mLeft[0] * mRight[0] +
			  mLeft[1] * mRight[3] +
			  mLeft[2] * mRight[6];
	temp[1] = mLeft[0] * mRight[1] +
			  mLeft[1] * mRight[4] +
			  mLeft[2] * mRight[7];
	temp[2] = mLeft[0] * mRight[2] +
			  mLeft[1] * mRight[5] +
			  mLeft[2] * mRight[8];

	temp[3] = mLeft[3] * mRight[0] +
			  mLeft[4] * mRight[3] +
			  mLeft[5] * mRight[6];
	temp[4] = mLeft[3] * mRight[1] +
			  mLeft[4] * mRight[4] +
			  mLeft[5] * mRight[7];
	temp[5] = mLeft[3] * mRight[2] +
			  mLeft[4] * mRight[5] +
			  mLeft[5] * mRight[8];

	temp[6] = mLeft[6] * mRight[0] +
			  mLeft[7] * mRight[3] +
			  mLeft[8] * mRight[6];
	temp[7] = mLeft[6] * mRight[1] +
			  mLeft[7] * mRight[4] +
			  mLeft[8] * mRight[7];
	temp[8] = mLeft[6] * mRight[2] +
			  mLeft[7] * mRight[5] +
			  mLeft[8] * mRight[8];

    mDest[0]  = temp[0];
    mDest[1]  = temp[1];
    mDest[2]  = temp[2];
    mDest[3]  = temp[3];
    mDest[4]  = temp[4];
    mDest[5]  = temp[5];
    mDest[6]  = temp[6];
    mDest[7]  = temp[7];
    mDest[8]  = temp[8];

	return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix4x4Cat
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Concatenate two 4x4 matrices and store result in a third.
  Arguments:
    mRight - right hand matrix in multiplication
    mLeft - left hand matrix in multiplication
    mDest - destination matrix where result is stored
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix4x4Cat( AtmMatrix4x4 mDest, 
                      AtmMatrix4x4 mLeft, 
                      AtmMatrix4x4 mRight ) {
    AtmMatrix4x4 temp;

#ifdef AT_DEBUGGING
    if ( !mDest || !mLeft || !mRight ) 
        atuError( FXTRUE, "atmMatrix4x4Cat(): Invalid parameter.\n" );
#endif

    temp[0] = mLeft[0] * mRight[0] +
              mLeft[1] * mRight[4] +
              mLeft[2] * mRight[8] +
              mLeft[3] * mRight[12];
    temp[1] = mLeft[0] * mRight[1] +
              mLeft[1] * mRight[5] +
              mLeft[2] * mRight[9] +
              mLeft[3] * mRight[13];
    temp[2] = mLeft[0] * mRight[2] +
              mLeft[1] * mRight[6] +
              mLeft[2] * mRight[10]+
              mLeft[3] * mRight[14];
    temp[3] = mLeft[0] * mRight[3] +
              mLeft[1] * mRight[7] +
              mLeft[2] * mRight[11]+
              mLeft[3] * mRight[15];

    temp[4] = mLeft[4] * mRight[0] +
              mLeft[5] * mRight[4] +
              mLeft[6] * mRight[8] +
              mLeft[7] * mRight[12];
    temp[5] = mLeft[4] * mRight[1] +
              mLeft[5] * mRight[5] +
              mLeft[6] * mRight[9] +
              mLeft[7] * mRight[13];
    temp[6] = mLeft[4] * mRight[2] +
              mLeft[5] * mRight[6] +
              mLeft[6] * mRight[10]+
              mLeft[7] * mRight[14];
    temp[7] = mLeft[4] * mRight[3] +
              mLeft[5] * mRight[7] +
              mLeft[6] * mRight[11]+
              mLeft[7] * mRight[15];

    temp[8]  = mLeft[8]  * mRight[0] +
               mLeft[9]  * mRight[4] +
               mLeft[10] * mRight[8] +
               mLeft[11] * mRight[12];
    temp[9]  = mLeft[8]  * mRight[1] +
               mLeft[9]  * mRight[5] +
               mLeft[10] * mRight[9] +
               mLeft[11] * mRight[13];
    temp[10] = mLeft[8]  * mRight[2] +
               mLeft[9]  * mRight[6] +
               mLeft[10] * mRight[10]+
               mLeft[11] * mRight[14];
    temp[11] = mLeft[8]  * mRight[3] +
               mLeft[9]  * mRight[7] +
               mLeft[10] * mRight[11]+
               mLeft[11] * mRight[15];

    temp[12] = mLeft[12] * mRight[0] +
               mLeft[13] * mRight[4] +
               mLeft[14] * mRight[8] +
               mLeft[15] * mRight[12];
    temp[13] = mLeft[12] * mRight[1] +
               mLeft[13] * mRight[5] +
               mLeft[14] * mRight[9] +
               mLeft[15] * mRight[13];
    temp[14] = mLeft[12] * mRight[2] +
               mLeft[13] * mRight[6] +
               mLeft[14] * mRight[10]+
               mLeft[15] * mRight[14];
    temp[15] = mLeft[12] * mRight[3] +
               mLeft[13] * mRight[7] +
               mLeft[14] * mRight[11]+
               mLeft[15] * mRight[15];

    mDest[0]  = temp[0];
    mDest[1]  = temp[1];
    mDest[2]  = temp[2];
    mDest[3]  = temp[3];
    mDest[4]  = temp[4];
    mDest[5]  = temp[5];
    mDest[6]  = temp[6];
    mDest[7]  = temp[7];
    mDest[8]  = temp[8];
    mDest[9]  = temp[9];
    mDest[10] = temp[10];
    mDest[11] = temp[11];
    mDest[12] = temp[12];
    mDest[13] = temp[13];
    mDest[14] = temp[14];
    mDest[15] = temp[15];

	return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix3x3Add
  Date: 4/30/96
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Componentwise addition of two matrices.
  Arguments:
    mDest - destination matrix
    mLeft - left source matrix
    mRight - right Source matrix
  Return:
    none
  -------------------------------------------------------------------*/
void  atmMatrix3x3Add( AtmMatrix3x3 mDest, 
                       AtmMatrix3x3 mLeft, 
                       AtmMatrix3x3 mRight ) {

    mDest[0] = mLeft[0] + mRight[0]; 
    mDest[1] = mLeft[1] + mRight[1]; 
    mDest[2] = mLeft[2] + mRight[2];
    mDest[3] = mLeft[3] + mRight[3];
    mDest[4] = mLeft[4] + mRight[4];
    mDest[5] = mLeft[5] + mRight[5];
    mDest[6] = mLeft[6] + mRight[6];
    mDest[7] = mLeft[7] + mRight[7];
    mDest[8] = mLeft[8] + mRight[8];
    return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix3x3Sub
  Date: 4/30/96
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Componentwise subtraction of two matrices.
  Arguments:
    mDest - destination matrix
    mLeft - left source matrix
    mRight - right Source matrix
  Return:
    none
  -------------------------------------------------------------------*/
void  atmMatrix3x3Sub( AtmMatrix3x3 mDest, 
                       AtmMatrix3x3 mLeft, 
                       AtmMatrix3x3 mRight ) {
    mDest[0] = mLeft[0] - mRight[0]; 
    mDest[1] = mLeft[1] - mRight[1]; 
    mDest[2] = mLeft[2] - mRight[2];
    mDest[3] = mLeft[3] - mRight[3];
    mDest[4] = mLeft[4] - mRight[4];
    mDest[5] = mLeft[5] - mRight[5];
    mDest[6] = mLeft[6] - mRight[6];
    mDest[7] = mLeft[7] - mRight[7];
    mDest[8] = mLeft[8] - mRight[8];
    return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix3x3Scale
  Date: 4/30/96
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Multiply all elements of a matrix by a scalar
  Arguments:
    mDest  - destination matrix
    mSrc   - left source matrix
    scalar - scalr
  Return:
    none
  -------------------------------------------------------------------*/
void  atmMatrix3x3Scale( AtmMatrix3x3 mDest, 
                         AtmMatrix3x3 mSrc,
                         float scalar ) {
    mDest[0] = mSrc[0] * scalar;
    mDest[1] = mSrc[1] * scalar;
    mDest[2] = mSrc[2] * scalar;
    mDest[3] = mSrc[3] * scalar;
    mDest[4] = mSrc[4] * scalar;
    mDest[5] = mSrc[5] * scalar;
    mDest[6] = mSrc[6] * scalar;
    mDest[7] = mSrc[7] * scalar;
    mDest[8] = mSrc[8] * scalar;
    return;
}


/*-------------------------------------------------------------------
  Function: atmMatrix3x3Print
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Prints a matrix to file stream.
  Arguments:
    indent - number of spaces to indent output
    m - matrix to print
    stream - standard C output stream
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix3x3Print( FILE *stream, const AtmMatrix3x3 m, FxU32 indent ){
#ifdef AT_DEBUGGING
    if ( !stream || !m )
        atuError( FXTRUE, "atmMatrix3x3Print(): Invalid parameter.\n" );
#endif
	fprintf( stream, "%*s%f %f %f\n", indent, " ", m[0], m[1], m[2] ); 
	fprintf( stream, "%*s%f %f %f\n", indent, " ", m[3], m[4], m[5] ); 
	fprintf( stream, "%*s%f %f %f\n", indent, " ", m[6], m[7], m[8] ); 
	return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix4x4Print
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Prints a matrix to file stream.
  Arguments:
    indent - number of spaces to indent output
    m - matrix to print
    stream - standard C output stream
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix4x4Print( FILE *stream, const AtmMatrix4x4 m, FxU32 indent ){
#ifdef AT_DEBUGGING
    if ( !stream || !m )
        atuError( FXTRUE, "atmMatrix4x4Print(): Invalid parameter.\n" );
#endif
	fprintf( stream, "%*s%f %f %f %f\n", indent, " ", m[0], m[1], m[2], m[3] ); 
	fprintf( stream, "%*s%f %f %f %f\n", indent, " ", m[4], m[5], m[6], m[7] ); 
	fprintf( stream, "%*s%f %f %f %f\n", indent, " ", m[8], m[9], m[10], m[11] ); 
	fprintf( stream, "%*s%f %f %f %f\n", indent, " ", m[12], m[13], m[14], m[15] ); 
	return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix3x3Transpose
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Compute the transpose of a 3x3 matrix and store it to another matrix.
  Arguments:
    src - source matrix
    dest - destination matrix
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix3x3Transpose( AtmMatrix3x3 dest, AtmMatrix3x3 src ) {
    float p1, p2, p3;
#ifdef AT_DEBUGGING
    if ( !dest || !src ) 
        atuError( FXTRUE, "atmMatrix3x3Transpose(): Invalid parameter.\n" );
#endif
    dest[0] = src[0];
    dest[4] = src[4];
    dest[8] = src[8];
    p1 = src[1];
    p2 = src[2];
    p3 = src[5];
    dest[1] = src[3];
    dest[2] = src[6];
    dest[5] = src[7];
    dest[3] = p1;
    dest[6] = p2;
    dest[7] = p3;
    return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix4x4Transpose
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Compute the transpose of a 4x4 matrix and store it to another matrix.
  Arguments:
    src - source matrix
    dest - destination matrix
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix4x4Transpose( AtmMatrix4x4 dest, AtmMatrix4x4 src ) {
    float p1, p2, p3;
#ifdef AT_DEBUGGING
    if ( !dest || !src ) 
        atuError( FXTRUE, "atmMatrix4x4Transpose(): Invalid parameter.\n" );
#endif
    dest[0]  = src[0];
    dest[5]  = src[5];
    dest[10] = src[10];

    p1 = src[1];
    p2 = src[2];
    p3 = src[6];

    dest[1] = src[4];
    dest[2] = src[8];
    dest[6] = src[9];

    dest[4] = p1;
    dest[8] = p2;
    dest[9] = p3;

    p1 = -1.0f * src[12];
    p2 = -1.0f * src[13];
    p3 = -1.0f * src[14];

    dest[12] = p1 * dest[0] + p2 * dest[4] + p3 * dest[8];
    dest[13] = p1 * dest[1] + p2 * dest[5] + p3 * dest[9];
    dest[14] = p1 * dest[2] + p2 * dest[6] + p3 * dest[10];

    return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix3x3Invert
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Compute the inverse of a matrix using cramer's rule.  A more stable
    version using the Gauss-Jordan algorithm will be implemented later
  Arguments:
    src - source matrix
    dest - destination matrix
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix3x3Invert( AtmMatrix3x3 dest, AtmMatrix3x3 src ) {
    float c00, c01, c02, /* cofactor matrix */
          c10, c11, c12,
          c20, c21, c22;
    
    float a00, a01, a02, /* adjugate */
          a10, a11, a12,
          a20, a21, a22;

    float det, oodet; /* determinate, one over dterminant */

#ifdef AT_DEBUGGING
    if ( !dest || !src ) 
        atuError( FXTRUE, "atmMatrix3x3Invert(): Invalid parameter.\n" );
#endif

    /* Compute the matrix of Cofactors */    
    c00 =    src[4] * src[8] - src[7] * src[5];
    c01 = -( src[3] * src[8] - src[6] * src[5] );
    c02 =    src[3] * src[7] - src[6] * src[4];

    c10 = -( src[1] * src[8] - src[7] * src[2] );
    c11 =    src[0] * src[8] - src[6] * src[2];
    c12 = -( src[0] * src[7] - src[6] * src[1] );

    c20 =    src[1] * src[5] - src[4] * src[2];
    c21 = -( src[0] * src[5] - src[3] * src[2] );
    c22 =    src[0] * src[4] - src[3] * src[1];

    /* Create Adjugate which the the transpose of the matrix of cofactors */
    a00 = c00; a01 = c10; a02 = c20;
    a10 = c01; a11 = c11; a12 = c21;
    a20 = c02; a21 = c12; a22 = c22;

    /* Compute the determinate of src */
    /* which is elem(0,0) of cat(a,src) */
    det = a00 * src[0] + a01 * src[3] + a02 * src[6];

    if ( det == 0.0f ) {
#       ifdef AT_DEBUG
            printf( "Singular matrix, can't be inverted.\n" );
#       endif
        dest[0] = 1.0f; dest[1] = 0.0f; dest[2] = 0.0f; 
        dest[3] = 0.0f; dest[4] = 1.0f; dest[5] = 0.0f; 
        dest[6] = 0.0f; dest[7] = 0.0f; dest[8] = 1.0f; 
        return;
    }
    
    /* Compute Inverse of the matrix */
    oodet = 1.0f / det;
    
    dest[0] = oodet * a00; dest[1] = oodet * a01; dest[2] = oodet * a02;
    dest[3] = oodet * a10; dest[4] = oodet * a11; dest[5] = oodet * a12;
    dest[6] = oodet * a20; dest[7] = oodet * a21; dest[8] = oodet * a22;

    return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix4x4Invert
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Compute the inverse of a matrix using cramer's rule.  A more stable
    version using the Gauss-Jordan algorithm will be implemented later.
    This implementation of 4x4 inversion is not at all general, it special
    cases the 3D rotation matrix and translation vector of a 3D 
    transformation embedded in a 4x4.  
    
    There are two implementations here, one that uses Cramer's rule, 
    one that uses Gaussian elimination.  Some analysis needs to be
    done to quantify the performance of each implementation.
  Arguments:
    src - source matrix
    dest - destination matrix
  Return:
    none
  -------------------------------------------------------------------*/
#if 1
void atmMatrix4x4Invert( AtmMatrix4x4 dest, AtmMatrix4x4 src ) {
    float c00, c01, c02, /* cofactor matrix */
          c10, c11, c12,
          c20, c21, c22;
    
    float a00, a01, a02, /* adjugate */
          a10, a11, a12,
          a20, a21, a22;

    float det, oodet; /* determinate, one over dterminant */
    float t0, t1, t2; /* intermediary translation value */

#ifdef AT_DEBUGGING
    if ( !dest || !src ) 
        atuError( FXTRUE, "atmMatrix4x4Invert(): Invalid parameter.\n" );
#endif

    /* Compute the matrix of Cofactors */    
    c00 =    src[5]  * src[10] - src[9]  * src[6];
    c01 = -( src[4]  * src[10] - src[8]  * src[6]  );
    c02 =    src[4]  * src[9]  - src[8]  * src[5];

    c10 = -( src[1]  * src[10] - src[9]  * src[2]  );
    c11 =    src[0]  * src[10] - src[8]  * src[2];
    c12 = -( src[0]  * src[9]  - src[8]  * src[1]  );

    c20 =    src[1]  * src[6]  - src[5]  * src[2];
    c21 = -( src[0]  * src[6]  - src[4]  * src[2]  );
    c22 =    src[0]  * src[5]  - src[4]  * src[1];

    /* Create Adjugate which the the transpose of the matrix of cofactors */
    a00 = c00; a01 = c10; a02 = c20;
    a10 = c01; a11 = c11; a12 = c21;
    a20 = c02; a21 = c12; a22 = c22;

    /* Compute the determinate of src */
    /* which is elem(0,0) of cat(a,src) */
    det = a00 * src[0] + a01 * src[4] + a02 * src[8];

    if ( det == 0.0f ) {
#       ifdef AT_DEBUG
            printf( "Singular matrix, can't be inverted.\n" );
#       endif
        dest[0]  = 1.0f; dest[1]  = 0.0f; dest[2]  = 0.0f; dest[3]  = 0.0f;
        dest[4]  = 0.0f; dest[5]  = 1.0f; dest[6]  = 0.0f; dest[7]  = 0.0f;
        dest[8]  = 0.0f; dest[9]  = 0.0f; dest[10] = 1.0f; dest[11] = 0.0f;
        dest[12] = 0.0f; dest[13] = 0.0f; dest[14] = 0.0f; dest[15] = 1.0f;
        return;
    }
    
    /* Compute Inverse of the matrix */
    oodet = 1.0f / det;

    dest[3] = dest[7] = dest[11] = 0.0f;
    
    dest[0] = oodet * a00; dest[1] = oodet * a01; dest[2]  = oodet * a02;
    dest[4] = oodet * a10; dest[5] = oodet * a11; dest[6]  = oodet * a12;
    dest[8] = oodet * a20; dest[9] = oodet * a21; dest[10] = oodet * a22;

    /* Compute inverse of translation vector */
    t0 = -src[12]; t1 = -src[13]; t2 = -src[14];

    dest[12] = t0 * dest[0] + t1 * dest[4] + t2 * dest[8];
    dest[13] = t0 * dest[1] + t1 * dest[5] + t2 * dest[9];
    dest[14] = t0 * dest[2] + t1 * dest[6] + t2 * dest[10];
    dest[15] = 1.0f;

    return;
}
#else
/* Code taken from graphics gems */
static float Identity[16] = {
   1.0f, 0.0f, 0.0f, 0.0f,
   0.0f, 1.0f, 0.0f, 0.0f,
   0.0f, 0.0f, 1.0f, 0.0f,
   0.0f, 0.0f, 0.0f, 1.0f
};
void atmMatrix4x4Invert( AtmMatrix4x4 a, AtmMatrix4x4 b ) {
#define MEMCPY memcpy
#define MAT(m,c,r) ((m)[(r)*4+(c)])

  float val, val2;
  int   i, j, k, ind;
  float tmp[16];

  MEMCPY(tmp, b,sizeof(float)*16);
  MEMCPY(a,Identity,sizeof(float)*16);

  for (i = 0; i != 4; i++) {

    val = MAT(tmp,i,i);			/* find pivot */
    ind = i;
    for (j = i + 1; j != 4; j++) {
      if (fabs(MAT(tmp,j,i)) > fabs(val)) {
	ind = j;
	val = MAT(tmp,j,i);
      }
    }

    if (ind != i) {			/* swap columns */
      for (j = 0; j != 4; j++) {
	val2 = MAT(a,i,j);
	MAT(a,i,j) = MAT(a,ind,j);
	MAT(a,ind,j) = val2;
	val2 = MAT(tmp,i,j);
	MAT(tmp,i,j) = MAT(tmp,ind,j);
	MAT(tmp,ind,j) = val2;
      }
    } else {
	}

    if (val == 0.0F) {	
       /* The matrix is singular (has no inverse).  This isn't really
	* an error since singular matrices can be used for projecting
	* shadows, etc.  We let the inverse be the identity matrix.
	*/
       fprintf(stderr,"Singular matrix, no inverse!\n");
       MEMCPY( a, Identity, 16*sizeof(float) );
       return;
    }

    val = 1.0f/val;
    for (j = 0; j != 4; j++) {
      MAT(tmp,i,j) *= val;
      MAT(a,i,j) *= val;
    }

    for (j = 0; j != 4; j++) {		/* eliminate column */
      if (j == i)
	continue;
      val = MAT(tmp,j,i);
      for (k = 0; k != 4; k++) {
	MAT(tmp,j,k) -= MAT(tmp,i,k) * val;
	MAT(a,j,k) -= MAT(a,i,k) * val;
      }
    }
  }
}
#undef MAT
#undef MEMCPY
#endif

/*-------------------------------------------------------------------
  Function: atmMatrix3x3Assign
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Assign one 3x3 matrix to another.
  Arguments:
    src - source matrix
    dest - destination matrix
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix3x3Assign( AtmMatrix3x3 dest, AtmMatrix3x3 src ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src ) 
        atuError( FXTRUE, "atmMatrix3x3Assign(): Invalid parameter.\n" );
#endif
    dest[0]  = src[0];
    dest[1]  = src[1];
    dest[2]  = src[2];
    dest[3]  = src[3];
    dest[4]  = src[4];
    dest[5]  = src[5];
    dest[6]  = src[6];
    dest[7]  = src[7];
    dest[8]  = src[8];
    return;
}

/*-------------------------------------------------------------------
  Function: atmMatrix4x4Assign
  Date: 3/18/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Assign one 4x4 matrix to another.
  Arguments:
    src - source matrix
    dest - destination matrix
  Return:
    none
  -------------------------------------------------------------------*/
void atmMatrix4x4Assign( AtmMatrix4x4 dest, AtmMatrix4x4 src ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src ) 
        atuError( FXTRUE, "atmMatrix4x4Assign(): Invalid parameter.\n" );
#endif
    dest[0]  = src[0];
    dest[1]  = src[1];
    dest[2]  = src[2];
    dest[3]  = src[3];

    dest[4]  = src[4];
    dest[5]  = src[5];
    dest[6]  = src[6];
    dest[7]  = src[7];

    dest[8]  = src[8];
    dest[9]  = src[9];
    dest[10] = src[10];
    dest[11] = src[11];

    dest[12] = src[12];
    dest[13] = src[13];
    dest[14] = src[14];
    dest[15] = src[15];
    return;
}

/*-------------------------------------------------------------------
  Function: atmVector3Set
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Math 
  Description:
    Initialize a 3D vector from components.
  Arguments:
    v - vector
    i - i scalar
    j - j scalar
    k - k scalar
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector3Set( AtmVector3 v, float i, float j, float k ) {
#ifdef AT_DEBUGGING
    if ( !v ) 
        atuError( FXTRUE, "atmVector3Set(): Invalid parameter.\n" );    
#endif	
	v[0] = i; v[1] = j; v[2] = k;
	return;
}

/*-------------------------------------------------------------------
  Function: atmVector2Set
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Math 
  Description:
    Initialize a 2D vector from components.
  Arguments:
    v - vector
    i - i scalar
    j - j scalar
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector2Set( AtmVector2 v, float i, float j ) {
#ifdef AT_DEBUGGING
    if ( !v ) 
        atuError( FXTRUE, "atmVector2Set(): Invalid parameter.\n" );    
#endif	
    v[0] = i; v[1] = j;
	return;
}

/*-------------------------------------------------------------------
  Function: atmVector3Add
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Add two 3D vectors and assign result to a third.
  Arguments:
    dest - sum vector
    src1 - 1st addend
    src2 - 2nd addend
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector3Add( AtmVector3 dest, AtmVector3 src1, AtmVector3 src2 ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src1 || !src2 ) 
        atuError( FXTRUE, "atmVector3Add(): Invalid parameter.\n" );    
#endif	
    dest[0] = src1[0] + src2[0];
	dest[1] = src1[1] + src2[1];
	dest[2] = src1[2] + src2[2];
	return;
}


/*-------------------------------------------------------------------
  Function: atmVector2Add
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Add two 2D vectors and assign result to a third.
  Arguments:
    dest - sum vector
    src1 - 1st addend
    src2 - 2nd addend
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector2Add( AtmVector2 dest, AtmVector2 src1, AtmVector2 src2 ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src1 || !src2 ) 
        atuError( FXTRUE, "atmVector2Add(): Invalid parameter.\n" );    
#endif	
    dest[0] = src1[0] + src2[0];
	dest[1] = src1[1] + src2[1];
	return;
}

/*-------------------------------------------------------------------
  Function: atmVector3Sub
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT_MATH
  Description:
    Subtract two 3D vectors and store result in a third.
  Arguments:
    dest - difference vector
    src1 - subtrahend vector
    src2 - subtractor vector
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector3Sub( AtmVector3 dest, AtmVector3 src1, AtmVector3 src2 ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src1 || !src2 ) 
        atuError( FXTRUE, "atmVector3Sub(): Invalid parameter.\n" );    
#endif	
    dest[0] = src1[0] - src2[0];
	dest[1] = src1[1] - src2[1];
	dest[2] = src1[2] - src2[2];
	return;
}

/*-------------------------------------------------------------------
  Function: atmVector2Sub
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT_MATH
  Description:
    Subtract two 2D vectors and store result in a third.
  Arguments:
    dest - difference vector
    src1 - subtrahend vector
    src2 - subtractor vector
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector2Sub( AtmVector2 dest, AtmVector2 src1, AtmVector2 src2 ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src1 || !src2 ) 
        atuError( FXTRUE, "atmVector2Sub(): Invalid parameter.\n" );    
#endif
	dest[0] = src1[0] - src2[0];
	dest[1] = src1[1] - src2[1];
	return;
}

/*-------------------------------------------------------------------
  Function: atmVector3Dot
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Compute the dot product of two vectors.
  Arguments:
    v1 - vector
    v2 - vector
  Return:
    floating point dot product of vectors
  -------------------------------------------------------------------*/
float atmVector3Dot( AtmVector3 v1, AtmVector3 v2 ) {
#ifdef AT_DEBUGGING
    if ( !v1 || !v2 ) 
        atuError( FXTRUE, "atmVector3Dot(): Invalid parameter.\n" );    
#endif
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


/*-------------------------------------------------------------------
  Function: atmVector3Dot
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Compute the cross product of two vectors and store it in a third.
  Arguments:
    dest - destination vector
    src1 - left hand vector
    src2 - right hand vector
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector3Cross( AtmVector3 dest, AtmVector3 src1, AtmVector3 src2 ) {
    AtmVector3 temp;
#ifdef AT_DEBUGGING
    if ( !dest || !src1 || !src2 ) 
        atuError( FXTRUE, "atmVector3Cross(): Invalid parameter.\n" );    
#endif
    temp[0] = src1[1] * src2[2] - src2[1] * src1[2];
    temp[1] = - ( src1[0] * src2[2] - src2[0] * src1[2] );
    temp[2] = src1[0] * src2[1] - src2[0] * src1[1];
    dest[0] = temp[0];
    dest[1] = temp[1];
    dest[2] = temp[2];
    return;
}

/*-------------------------------------------------------------------
  Function: atmVector3Normalize
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Normalize a vector and copy the normalized vector into another.
  Arguments:
    dest - destination vector
    src  - src vector
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector3Normalize( AtmVector3 dest, AtmVector3 src ) {
    float oomagnitude;
#ifdef AT_DEBUGGING 
    if ( src[0] == src[1] && src[1] == src[2] && src[2] == 0.0f )
        atuError( FXFALSE, "atmVector3Normalize(): zero magnitude vector.\n" );
#endif
    oomagnitude = atmOOSqrt( src[0] * src[0] + 
                             src[1] * src[1] + 
                             src[2] * src[2] );
    dest[0] = src[0] * oomagnitude;
    dest[1] = src[1] * oomagnitude;
    dest[2] = src[2] * oomagnitude;    
    return;
}

/*-------------------------------------------------------------------
  Function: atmVector3Scale
  Date: 3/16/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Multiply all of the elements of a vector by a scalar and store results
    in another vector.
  Arguments:
    dest   - destination vector
    src    - src vector
    scalar - scalar to be multiplied with components of src
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector3Scale( AtmVector3 dest, AtmVector3 src, float scalar ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src  ) 
        atuError( FXTRUE, "atmVector3Scale(): Invalid parameter.\n" );    
#endif
    dest[0] = src[0] * scalar;
    dest[1] = src[1] * scalar;
    dest[2] = src[2] * scalar;
    return;
}

/*-------------------------------------------------------------------
  Function: atmVector3Magnitude
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Compute the magnitude of a 3D vector.
  Arguments:
    v - vector
  Return:
    floating point magnitude
  -------------------------------------------------------------------*/
float atmVector3Magnitude( const AtmVector3 v ) {
#ifdef AT_DEBUGGING
    if ( !v ) 
        atuError( FXTRUE, "atmVector3Magnitude(): Invalid parameter.\n" );    
#endif
    return (float) sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] );     
}

/*-------------------------------------------------------------------
  Function: atmVector3Midpoint
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Compute the midpoint of two vectors and store it in a third.
  Arguments:
    v1 - one source vector
    v2 - the other source vector
    dest - destination where computed midpoint is stored
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector3Midpoint( AtmVector3 dest, AtmVector3 v1, AtmVector3 v2 ) {
#ifdef AT_DEBUGGING
    if ( !dest || !v1 || !v2 ) 
        atuError( FXTRUE, "atmVector3Midpoint(): Invalid parameter.\n" );    
#endif
    dest[0] = ( v1[0] + v2[0] ) / 2.0f;
    dest[1] = ( v1[1] + v2[1] ) / 2.0f;
    dest[2] = ( v1[2] + v2[2] ) / 2.0f;
} 

/*-------------------------------------------------------------------
  Function: atmVectorMatrix3x3Mult
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Multiply a 3D vector by a 3x3 Matrix
  Arguments:
    m - matrix
    v - vector
    vDest - destination vector
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector3Matrix3x3Mult( AtmVector3 vDest, AtmVector3 v, AtmMatrix3x3 m ) {
    AtmVector3 tmp;
#ifdef AT_DEBUGGING
    if ( !vDest || !v || !m ) 
        atuError( FXTRUE, "atmVector3Matrix3x3Mult(): Invalid parameter.\n" );    
#endif
    tmp[0]   = v[0] * m[0] + v[1] * m[3] + v[2] * m[6];
    tmp[1]   = v[0] * m[1] + v[1] * m[4] + v[2] * m[7];
    vDest[2] = v[0] * m[2] + v[1] * m[5] + v[2] * m[8];
    vDest[0] = tmp[0];
    vDest[1] = tmp[1];
    return;
}

/*-------------------------------------------------------------------
  Function: atmVector2Assign
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Assign one 2D vector to another.  As the ATM vector2 type is an
    array, you can't do simple assignment with it, hence this function.
  Arguments:
    src - src vector
    dest - vector to which src is assigned
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector2Assign( AtmVector2 dest, const AtmVector2 src ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src ) 
        atuError( FXTRUE, "atmVector2Assign(): Invalid parameter.\n" );    
#endif
    dest[0] = src[0];
    dest[1] = src[1];
    return;
}

/*-------------------------------------------------------------------
  Function: atmVector3Assign
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Assign one 3D vector to another.  As the ATM vector3 type is an
    array, you can't do simple assignment with it, hence this function.
  Arguments:
    src - src vector
    dest - vector to which src is assigned
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector3Assign( AtmVector3 dest, const AtmVector3 src ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src ) 
        atuError( FXTRUE, "atmVector3Assign(): Invalid parameter.\n" );    
#endif
    dest[0] = src[0];
    dest[1] = src[1];
    dest[2] = src[2];
    return;
}

/*-------------------------------------------------------------------
  Function: atmVector3Print
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    Print a 3D vector to the specified output stream.
  Arguments:
    stream - a standard C output stream
    v - vector to be printed
    indent - number of spaces to indent output, to facilitate readability
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector3Print( FILE *stream, const AtmVector3 v, FxU32 indent ) {
#ifdef AT_DEBUGGING
    if ( !stream || !v ) 
        atuError( FXTRUE, "atmVector3Print(): Invalid parameter.\n" );
#endif
	fprintf( stream, "%*s%f %f %f\n", indent, " ", v[0], v[1], v[2] );
}

/*-------------------------------------------------------------------
  Function: atmVector2Print
  Date: 3/17/96
  Implementor(s): jdt
  Library: AT Math
  Description:
    stream - a standard C output stream
    v - vector to be printed
    indent - number of spaces to indent output, to facilitate readability
  Return:
    none
  -------------------------------------------------------------------*/
void atmVector2Print( FILE *stream, const AtmVector2 v, FxU32 indent ) {
#ifdef AT_DEBUGGING
    if ( !stream || !v ) 
        atuError( FXTRUE, "atmVector2Print(): Invalid parameter.\n" );
#endif
	fprintf( stream, "%*s%f %f\n", indent, " ", v[0], v[1] );
}







