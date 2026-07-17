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
** 3ds animation header
*/

#ifndef TDSANIM_H__
#define TDSANIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------  Includes -----------------------------*/

#include <stdio.h>
#include "3ds.h"

/*----------------------------  Prototypes ----------------------------*/

float TDSHermiteSourceDerivative( float tension, float continuity,
                                  float bias, float key_pt_left, 
                                  float key_pt, float key_pt_right );
float TDSHermiteDestinationDerivative( float tension, float continuity,
                                       float bias, float key_pt_left,
                                       float key_pt, float key_pt_right );
float TDSHermiteInterpolate( float src_pt_left, float src_pt,
                             float dest_pt, float dest_pt_right,
                             float src_tension, float src_continuity,
                             float src_bias, float dest_tension,
                             float dest_continuity, float dest_bias,
                             float u );
float TDSLinearInterpolate( float left, float right, float u );

#ifdef __cplusplus
};
#endif

#endif /* TDSANIM_H__ */
