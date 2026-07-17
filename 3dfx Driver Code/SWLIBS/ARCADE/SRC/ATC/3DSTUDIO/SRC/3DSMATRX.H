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
** Filename: 3dsmatrx.h
**
** This file contains all the prototypes for the 3dsmatrx.c file.
*/

#ifndef TDSMATRX_H__
#define TDSMATRX_H__

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------  Includes -----------------------------*/

#include <stdio.h>
#include "3ds.h"

/*----------------------------  Prototypes ----------------------------*/

void   TDSMatMakeIdent( tds_matrix m );
void   TDSPointMatMult( tds_point *result, tds_point *p, tds_matrix m );
void   TDSMatMakeXRot( tds_matrix m, float radians );
void   TDSMatMakeYRot( tds_matrix m, float radians );
void   TDSMatMakeZRot( tds_matrix m, float radians );
void   TDSMatMult( tds_matrix result, tds_matrix a, tds_matrix b );
void   TDSBuildRotMatrixAboutAxis( tds_matrix result, float axis_[3], float angle );
float  TDSMatDeterminant( tds_matrix m );
void   TDSMatInvert4x4( tds_matrix dst, tds_matrix src );
void   TDSPrintMatrix( tds_matrix mat );

#ifdef __cplusplus
};
#endif

#endif /* TDSMATRX_H__ */
