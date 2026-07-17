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
** ALPHA PRODUCT!
*/

/*
** Filename: 3dsmatrx.c
**
** Matrix functionality for the 3DStudio loader.
*/

/*-----------------------------  Includes -----------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "3ds.h"

/*----------------------------  Functions -----------------------------*/

/*---------------------------------------------------------------------
  Function: TDSMatMakeIdent
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: create an identity matrix
  Arguments: 
    m - 4x4 matrix that will contain the identity matrix information
  Return: none
---------------------------------------------------------------------*/
void TDSMatMakeIdent( tds_matrix m )
{
  int i, j;

  for( i = 0; i < 4; i++ )
    {
      for( j = 0; j < 4; j++ )
        {
          if( i == j )
            m[i][j] = 1.0f;
          else
            m[i][j] = 0.0f;
        }
    }
}

/*---------------------------------------------------------------------
  Function: TDSPointMatMult
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: multiply a point by a 4x4 matrix
  Arguments: 
    result - 4x4 matrix that contains the result of the point times
             the 4x4 matrix ( p * 4x4 matrix )
    p - contains the vector information to be multiplied to the 4x4
        matrix
    m - 4x4 matrix containing matrix multiplying information
  Return: none
---------------------------------------------------------------------*/
void TDSPointMatMult( tds_point *result, tds_point *p, tds_matrix m )
{
  float w;
  tds_point ptmp;

  ptmp.x = ( p->x * m[0][0] ) + ( p->y * m[1][0] ) + 
    ( p->z * m[2][0] ) + m[3][0];
  ptmp.y = ( p->x * m[0][1] ) + ( p->y * m[1][1] ) + 
    ( p->z * m[2][1] ) + m[3][1];
  ptmp.z = ( p->x * m[0][2] ) + ( p->y * m[1][2] ) + 
    ( p->z * m[2][2] ) + m[3][2];
  w = ( p->x * m[0][3] ) + ( p->y * m[1][3] ) + 
    ( p->z * m[2][3] ) + m[3][3];
  if( w != 0.0f ) { ptmp.x /= w;  ptmp.y /= w;  ptmp.z /= w; }

  *result = ptmp;
}

/*---------------------------------------------------------------------
  Function: TDSMatMakeXRot
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: create a x rotation matrix
  Arguments: 
    m - 4x4 matrix containing the x rotation data
    radians - rotation angle in radians
  Return: none
---------------------------------------------------------------------*/
void TDSMatMakeXRot( tds_matrix m, float radians )
{
  float c = ( float )cos( radians );
  float s = ( float )sin( radians );

  m[0][0] = 1.0f;  m[0][1] = 0.0f;   m[0][2] = 0.0f;   m[0][3] = 0.0f;
  m[1][0] = 0.0f;  m[1][1] = c;      m[1][2] = s;      m[1][3] = 0.0f;
  m[2][0] = 0.0f;  m[2][1] = -s;     m[2][2] = c;      m[2][3] = 0.0f;
  m[3][0] = 0.0f;  m[3][1] = 0.0f;   m[3][2] = 0.0f;   m[3][3] = 1.0f;
}

/*---------------------------------------------------------------------
  Function: TDSMatMakeYRot
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: create a y rotation matrix
  Arguments: 
    m - 4x4 matrix containing the y rotation data
    radians - rotation angle in radians
  Return: none
---------------------------------------------------------------------*/
void TDSMatMakeYRot( tds_matrix m, float radians )
{
  float c = ( float )cos( radians );
  float s = ( float )sin( radians );

  m[0][0] = c;     m[0][1] = 0.0f;   m[0][2] = -s;     m[0][3] = 0.0f;
  m[1][0] = 0.0f;  m[1][1] = 1.0f;   m[1][2] = 0.0f;   m[1][3] = 0.0f;
  m[2][0] = s;     m[2][1] = 0.0f;   m[2][2] = c;      m[2][3] = 0.0f;
  m[3][0] = 0.0f;  m[3][1] = 0.0f;   m[3][2] = 0.0f;   m[3][3] = 1.0f;
}

/*---------------------------------------------------------------------
  Function: TDSMatMakeZRot
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: create a z rotation matrix
  Arguments: 
    m - 4x4 matrix containing the z rotation data
    radians - rotation angle in radians
  Return: none
---------------------------------------------------------------------*/
void TDSMatMakeZRot( tds_matrix m, float radians )
{
  float c = ( float )cos( radians );
  float s = ( float )sin( radians );

  m[0][0] = c;     m[0][1] = s;      m[0][2] = 0.0f;    m[0][3] = 0.0f;
  m[1][0] = -s;    m[1][1] = c;      m[1][2] = 0.0f;    m[1][3] = 0.0f;
  m[2][0] = 0.0f;  m[2][1] = 0.0f;   m[2][2] = 1.0f;    m[2][3] = 0.0f;
  m[3][0] = 0.0f;  m[3][1] = 0.0f;   m[3][2] = 0.0f;    m[3][3] = 1.0f;
}

/*---------------------------------------------------------------------
  Function: TDSMatMult
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: multiply two 4x4 matrices ( a * b )
  Arguments: 
    result - 4x4 matrix containing the result of the multiplication
             ( a * b )
    a - contains information for matrix 1
    b - contains information for matrix 2
  Return: none
---------------------------------------------------------------------*/
void TDSMatMult( tds_matrix result, tds_matrix a, tds_matrix b )
{
  int i, j, k;
  for (i=0; i<4; i++)
    {
      for (j=0; j<4; j++)
        {
          result[i][j] = 0.0f;
          for (k=0; k<4; k++) result[i][j] += 
            a[i][k] * b[k][j];
        }
    }
}

/*---------------------------------------------------------------------
  Function: TDSBuildRotMatrixAboutAxis
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate a 4x4 matrix for rotating objects about areas
               other than the origin
  Arguments: 
    result - 4x4 matrix containing the resulting rotation matrix
    axis_ - the point to rotate about
    angle - rotation angle in radians
  Return: none
---------------------------------------------------------------------*/
void TDSBuildRotMatrixAboutAxis( tds_matrix result, float axis_[3], float angle )
{
  float axis[3];
  float mag;
  tds_matrix rotx, roty, rotz, rotangle;
  tds_matrix rotxy, rotxy_inv, tmpmat;
  tds_matrix rotyx, rotyx_inv;
  float d;
  int i, j;

  angle *= -1;

  memcpy( axis, axis_, sizeof( float[3] ) );

  /* Make sure that the axis is normalized, it probably already is */
  mag = ( float )sqrt( axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2] );
  axis[0] /= mag;
  axis[1] /= mag;
  axis[2] /= mag;

  /* Create the matrices used to build this transformation */
  TDSMatMakeIdent( rotx );
  TDSMatMakeIdent( roty );
  TDSMatMakeIdent( rotz );
  TDSMatMakeIdent( rotangle );
  TDSMatMakeIdent( rotxy );
  TDSMatMakeIdent( rotyx );
  TDSMatMakeIdent( rotxy_inv );
  TDSMatMakeIdent( rotyx_inv );

  /* Create the x rotation matrix */
  d = ( float )sqrt( axis[1] * axis[1] + axis[2] * axis[2] );
  if( d != 0.0 )
    {
      rotx[1][1] = axis[2] / d;
      rotx[1][2] = axis[1] / d;
      rotx[2][1] = -axis[1] / d;
      rotx[2][2] = axis[2] / d;
    }
  
  /* Create the y rotation matrix */
  roty[0][0] = d;
  roty[2][0] = -axis[0];
  roty[0][2] = axis[0];
  roty[2][2] = d;
  
  /* Multiply x and y rotations together and calculate the inverse by
     transposition */
  TDSMatMult( rotxy, rotx, roty );
  TDSMatMakeIdent( rotxy_inv );
  for( i = 0; i < 3; i++ )
    for( j = 0; j < 3; j++ )
      rotxy_inv[i][j] = rotxy[j][i];
  
  /* Create a Z rotation matrix for the actual rotation angle */
  TDSMatMakeIdent( rotangle );
  rotangle[0][0] = ( float )cos( angle );
  rotangle[1][0] = -( float )sin( angle );
  rotangle[0][1] = ( float )sin( angle );
  rotangle[1][1] = ( float )cos( angle );
  
  /* Now do "rotx * roty * rotangle * roty_inv * rotx_inv" to get the 
     final result */
  TDSMatMult( tmpmat, rotxy, rotangle );
  TDSMatMult( result, tmpmat, rotxy_inv );
}

/*---------------------------------------------------------------------
  Function: TDSMatDeterminant
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate a matrix's determinant
  Arguments: 
    m - contains the matrix information
  Return: floating point matrix determinant
---------------------------------------------------------------------*/
float TDSMatDeterminant( tds_matrix m )
{
  float p[4];

  p[0] = m[0][0] * ( m[1][1] * ( m[2][2] * m[3][3] - m[2][3] * m[3][2] ) - 
                     m[1][2] * ( m[2][1] * m[3][3] - m[2][3] * m[3][1] ) +
                     m[1][3] * ( m[2][1] * m[3][2] - m[2][2] * m[3][1] ) );
  p[1] = m[0][1] * ( m[1][0] * ( m[2][2] * m[3][3] - m[2][3] * m[3][2] ) - 
                     m[1][2] * ( m[2][0] * m[3][3] - m[2][3] * m[3][0] ) +
                     m[1][3] * ( m[2][0] * m[3][2] - m[2][2] * m[3][0] ) );
  p[2] = m[0][2] * ( m[1][0] * ( m[2][1] * m[3][3] - m[2][3] * m[3][1] ) - 
                     m[1][1] * ( m[2][0] * m[3][3] - m[2][3] * m[3][0] ) +
                     m[1][3] * ( m[2][0] * m[3][1] - m[2][1] * m[3][0] ) );
  p[3] = m[0][3] * ( m[1][0] * ( m[2][1] * m[3][2] - m[2][2] * m[3][1] ) -
                     m[1][1] * ( m[2][0] * m[3][2] - m[2][2] * m[3][0] ) +
                     m[1][2] * ( m[2][0] * m[3][1] - m[2][1] * m[3][0] ) );

  return p[0] - p[1] + p[2] - p[3];
}

/*---------------------------------------------------------------------
  Function: TDSMatInvert4x4
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: invert a 4x4 matrix
  Arguments: 
    dst - destination for matrix information inversion
    src - matrix information to invert    
  Return: none
---------------------------------------------------------------------*/
void TDSMatInvert4x4( tds_matrix dst, tds_matrix src )
{
    float c00, c01, c02, /* cofactor matrix */
          c10, c11, c12,
          c20, c21, c22;
    
    float a00, a01, a02, /* adjugate */
          a10, a11, a12,
          a20, a21, a22;

    float det, oodet;    /* determinant,one over determinant */
    float t0, t1, t2;    /* intermediary translation value */

    /* Compute Matrix of Cofactors */    
    c00 =    src[1][1] * src[2][2] - src[2][1] * src[1][2];
    c01 = -( src[1][0] * src[2][2] - src[2][0] * src[1][2] );
    c02 =    src[1][0] * src[2][1] - src[2][0] * src[1][1];

    c10 = -( src[0][1] * src[2][2] - src[2][1] * src[0][2] );
    c11 =    src[0][0] * src[2][2] - src[2][0] * src[0][2];
    c12 = -( src[0][0] * src[2][1] - src[2][0] * src[0][1] );

    c20 =    src[0][1] * src[1][2] - src[1][1] * src[0][2];
    c21 = -( src[0][0] * src[1][2] - src[1][0] * src[0][2] );
    c22 =    src[0][0] * src[1][1] - src[1][0] * src[0][1];

    /* Create Adjugate which the the transpose of the matrix of cofactors */
    a00 = c00; a01 = c10; a02 = c20;
    a10 = c01; a11 = c11; a12 = c21;
    a20 = c02; a21 = c12; a22 = c22;

    /* Compute the determinate of src */
    /* which is elem(0,0) of cat(a,src) */
    det = a00 * src[0][0] + a01 * src[1][0] + a02 * src[2][0];

    if ( det == 0.0f ) {
#ifdef DEBUG
      printf( "Singular matrix, can't be inverted.\n" );
#endif
      dst[0][0] = 1.0f; dst[0][1] = 0.0f; dst[0][2] = 0.0f; dst[0][3] = 0.0f;
      dst[1][0] = 0.0f; dst[1][1] = 1.0f; dst[1][2] = 0.0f; dst[1][3] = 0.0f;
      dst[2][0] = 0.0f; dst[2][1] = 0.0f; dst[2][2] = 1.0f; dst[2][3] = 0.0f;
      dst[3][0] = 0.0f; dst[3][1] = 0.0f; dst[3][2] = 0.0f; dst[3][3] = 1.0f;
      return;
    }

    /* Compute Inverse of rotation submatrix */
    oodet = 1.0f / det;
    
    dst[0][0] = oodet * a00; dst[0][1] = oodet * a01; dst[0][2] = oodet * a02;
    dst[1][0] = oodet * a10; dst[1][1] = oodet * a11; dst[1][2] = oodet * a12;
    dst[2][0] = oodet * a20; dst[2][1] = oodet * a21; dst[2][2] = oodet * a22;

    /* Compute inverse of translation vector */
    t0 = -src[3][0]; t1 = -src[3][1]; t2 = -src[3][2];

    dst[3][0] = t0 * dst[0][0] + t1 * dst[1][0] + t2 * dst[2][0];
    dst[3][1] = t0 * dst[0][1] + t1 * dst[1][1] + t2 * dst[2][1];
    dst[3][2] = t0 * dst[0][2] + t1 * dst[1][2] + t2 * dst[2][2];
    dst[0][3] = dst[1][3] = dst[2][3] = 0.0f;
    dst[3][3] = 1.0f;
    return;
}

/*---------------------------------------------------------------------
  Function: TDSPrintMatrix
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: Print a 4x4 matrix
  Arguments: 
    m - matrix information to print
  Return: none
---------------------------------------------------------------------*/
void TDSPrintMatrix( tds_matrix m )
{
  int i, j;

  for( i = 0; i < 4; i++ )
    {
      for( j = 0; j < 4; j++ )
        {
          printf( "%f ", m[i][j] );
        }
      printf( "\n" );
    }
}
