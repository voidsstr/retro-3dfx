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
** Filename: 3dsvectr.c
**
** This file contains the vector math information utilized by 3D Studio.
*/

/*-----------------------------  Includes -----------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "3ds.h"

/*----------------------------  Functions -----------------------------*/

/*---------------------------------------------------------------------
  Function: TDSVecSub
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: subtracts vector b from vector a and stores the result
               in vector result
  Arguments: 
    result - contains the result of a - b 
    a - contains the x, y, and z component information for vector 1
    b - contains the x, y, and z component informatino for vector 2
  Return: none
---------------------------------------------------------------------*/
void TDSVecSub( tds_point *result, tds_point *a, tds_point *b )
{
  result->x = a->x - b->x;
  result->y = a->y - b->y;
  result->z = a->z - b->z;
}

/*---------------------------------------------------------------------
  Function: TDSVecAdd
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: add vector b to vector a and stores the result in vector
               result
  Arguments: 
    result - contains the result of a + b
    a - contains the x, y, and z component information for vector 1
    b - contains the x, y, and z component information for vector 2
  Return: none
---------------------------------------------------------------------*/
void TDSVecAdd( tds_point *result, tds_point *a, tds_point *b )
{
  result->x = a->x + b->x;
  result->y = a->y + b->y;
  result->z = a->z + b->z;
}

/*---------------------------------------------------------------------
  Function: TDSVecCross
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculates the cross product, a cross b
  Arguments: 
    v - contains the results of a cross b
    a - contains the x, y, and z component information for vector 1
    b - contains the x, y, and z component information for vector 2
  Return: none
---------------------------------------------------------------------*/
void TDSVecCross( tds_point *v, tds_point *a, tds_point *b )
{
  v->x = a->y * b->z - a->z * b->y;
  v->y = a->z * b->x - a->x * b->z;
  v->z = a->x * b->y - b->x * a->y;
}

/*---------------------------------------------------------------------
  Function: TDSVecNorm
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: normalizes the vector
  Arguments: 
    v - contains the vector component information to be normalized
  Return: none
---------------------------------------------------------------------*/
void TDSVecNorm( tds_point *v )
{
  float mag;

  mag = ( float )sqrt( v->x * v->x + v->y * v->y + v->z * v->z );

  /* check for magnitude equal zero */
  if( mag != 0 )
    {
      v->x /= mag;
      v->y /= mag;
      v->z /= mag;
    }
}

/*---------------------------------------------------------------------
  Function: TDSVecDot
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculates the dot product, a dot b
  Arguments: 
    a - contains the x, y, and z component information for vector 1
    b - contains the x, y, and z component information for vector 2
  Return: floating point dot product result
---------------------------------------------------------------------*/
float TDSVecDot( tds_point *a, tds_point *b )
{
  return a->x * b->x + a->y * b->y + a->z * b->z;
}
