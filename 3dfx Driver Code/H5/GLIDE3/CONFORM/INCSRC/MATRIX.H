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

#ifndef _MATRIX_H_
#define _MATRIX_H_

#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif

#define DEG2RAD( x ) ( (x) * M_PI / 180.0f )

typedef float matrix[4][4];

// Prototypes from matrix.cpp
void mat_make_x_rot(matrix m, float radians);
void mat_make_y_rot(matrix m, float radians);
void mat_make_z_rot(matrix m, float radians);
void make_make_ident(matrix m);
void mat_point_mul(GrVertex *result, GrVertex *v, matrix m);
void mat_vertex_mul(Vertex *result, Vertex *v, matrix m);

#endif // _MATRIX_H_
