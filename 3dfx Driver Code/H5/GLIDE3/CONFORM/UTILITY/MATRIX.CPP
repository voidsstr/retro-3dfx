/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
*/

#include <math.h>
#include "conform.h"
#include "matrix.h"

/**
 *  Matrix Utilities. Mostly stolen from other tests (eg. test25.c)
 */

void mat_make_x_rot(matrix m, float radians)
{
  float c = (float)cos( radians );
  float s = (float)sin( radians );

  m[0][0] = 1.0f;  m[0][1] = 0.0f;    m[0][2] = 0.0f;    m[0][3] = 0.0f;
  m[1][0] = 0.0f;  m[1][1] = c;       m[1][2] = s;       m[1][3] = 0.0f;
  m[2][0] = 0.0f;  m[2][1] = -s;      m[2][2] = c;       m[2][3] = 0.0f;
  m[3][0] = 0.0f;  m[3][1] = 0.0f;    m[3][2] = 0.0f;    m[3][3] = 1.0f;
}

void mat_make_y_rot(matrix m, float radians)
{
  float c = (float)cos(radians);
  float s = (float)sin(radians);

  m[0][0] = c;     m[0][1] = 0.0f;    m[0][2] = -s;      m[0][3] = 0.0f;
  m[1][0] = 0.0f;  m[1][1] = 1.0f;    m[1][2] = 0.0f;    m[1][3] = 0.0f;
  m[2][0] = s;     m[2][1] = 0.0f;    m[2][2] = c;       m[2][3] = 0.0f;
  m[3][0] = 0.0f;  m[3][1] = 0.0f;    m[3][2] = 0.0f;    m[3][3] = 1.0f;
}

void mat_make_z_rot(matrix m, float radians)
{
  float c = (float)cos(radians);
  float s = (float)sin(radians);

  m[0][0] = c;     m[0][1] = s;       m[0][2] = 0.0f;    m[0][3] = 0.0f;
  m[1][0] = -s;    m[1][1] = c;       m[1][2] = 0.0f;    m[1][3] = 0.0f;
  m[2][0] = 0.0f;  m[2][1] = 0.0f;    m[2][2] = 1.0f;    m[2][3] = 0.0f;
  m[3][0] = 0.0f;  m[3][1] = 0.0f;    m[3][2] = 0.0f;    m[3][3] = 1.0f;
}

void make_make_ident(matrix m)
{
  m[0][0] = 1.0f;  m[0][1] = 0.0f;    m[0][2] = 0.0f;    m[0][3] = 0.0f;
  m[1][0] = 0.0f;  m[1][1] = 1.0f;    m[1][2] = 0.0f;    m[1][3] = 0.0f;
  m[2][0] = 0.0f;  m[2][1] = 0.0f;    m[2][2] = 1.0f;    m[2][3] = 0.0f;
  m[3][0] = 0.0f;  m[3][1] = 0.0f;    m[3][2] = 0.0f;    m[3][3] = 1.0f;
}

void mat_point_mul(GrVertex *result, GrVertex *v, matrix m)
{
  float w;
  GrVertex ptmp;

  ptmp = *v;
  ptmp.x = ( v->x * m[0][0] ) + ( v->y * m[1][0] ) + 
    ( v->z * m[2][0] ) + m[3][0];
  ptmp.y = ( v->x * m[0][1] ) + ( v->y * m[1][1] ) + 
    ( v->z * m[2][1] ) + m[3][1];
  ptmp.z = ( v->x * m[0][2] ) + ( v->y * m[1][2] ) + 
    ( v->z * m[2][2] ) + m[3][2];
  w = ( v->x * m[0][3] ) + ( v->y * m[1][3] ) + 
    ( v->z * m[2][3] ) + m[3][3];
  if( w != 0.0f ) { ptmp.x /= w;  ptmp.y /= w;  ptmp.z /= w; }
  *result = ptmp;
}

void mat_vertex_mul(Vertex *result, Vertex *v, matrix m)
{
  float w;
  Vertex ptmp;

  ptmp = *v;
  ptmp.x = ( v->x * m[0][0] ) + ( v->y * m[1][0] ) +
    ( v->z * m[2][0] ) + m[3][0];
  ptmp.y = ( v->x * m[0][1] ) + ( v->y * m[1][1] ) +
    ( v->z * m[2][1] ) + m[3][1];
  ptmp.z = ( v->x * m[0][2] ) + ( v->y * m[1][2] ) +
    ( v->z * m[2][2] ) + m[3][2];
  w = ( v->x * m[0][3] ) + ( v->y * m[1][3] ) +
    ( v->z * m[2][3] ) + m[3][3];
  if( w != 0.0f ) { ptmp.x /= w;  ptmp.y /= w;  ptmp.z /= w; }
  *result = ptmp;
}
