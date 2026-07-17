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
** Filename: 3dsanim.c
**
** This file contains the functions that calculate the interpolation 
** between keyframes of 3D Studio data -- both Hermite Interpolation
** and Linear Interpolation
*/

/*-----------------------------  Includes -----------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "3ds.h"
#include "3dsinc.h"

/*----------------------------  Functions -----------------------------*/

/*---------------------------------------------------------------------
  Function: TDSHermiteSourceDerivative
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the Hermite source derivative
  Arguments: 
    tension - value amount of arc exaggeration between key
              positions
    continuity - value amount of direction and speed change between
                 key positions
    bias - value amount to overshoot a key position
    key_pt_left - key position previous to the source key position
    key_pt - source key position
    key_pt_right - key position following the source key position
  Return: floating point Hermite source derivative
---------------------------------------------------------------------*/
float TDSHermiteSourceDerivative( float tension, float continuity,
                                  float bias, float key_pt_left, 
                                  float key_pt, float key_pt_right )
{
  float deriv_coeff_a, deriv_coeff_b;
  float deriv_source;

  deriv_coeff_a = ( ( ( 1 - tension ) * ( 1 - continuity ) * ( 1 + bias ) ) / 2 );
  deriv_coeff_b = ( ( ( 1 - tension ) * ( 1 + continuity ) * ( 1 - bias ) ) / 2 );
  deriv_source = ( deriv_coeff_a * ( key_pt - key_pt_left ) ) +
                 ( deriv_coeff_b * ( key_pt_right - key_pt ) );

  return deriv_source;
}

/*---------------------------------------------------------------------
  Function: TDSHermiteDestiniationDerivative
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the Hermite destination derivation
  Arguments: 
    tension - value amount of arc exaggeration between key
              positions
    continuity - value amount of direction and speed change between
                 key positions
    bias - value amount to overshoot a key position
    key_pt_left - key position previous to the destination key
                  position
    key_pt - destination key position
    key_pt_right - key position following the destination key position
  Return: floating point Hermite destination derivative
---------------------------------------------------------------------*/
float TDSHermiteDestinationDerivative( float tension, float continuity,
                                       float bias, float key_pt_left,
                                       float key_pt, float key_pt_right )
{
  float deriv_coeff_a, deriv_coeff_b;
  float deriv_source;

  deriv_coeff_a = ( ( ( 1 - tension ) * ( 1 + continuity ) * ( 1 + bias ) ) / 2 );
  deriv_coeff_b = ( ( ( 1 - tension ) * ( 1 - continuity ) * ( 1 - bias ) ) / 2 );
  deriv_source = ( deriv_coeff_a * ( key_pt - key_pt_left ) ) +
                 ( deriv_coeff_b * ( key_pt_right - key_pt ) );

  return deriv_source;
}

/*---------------------------------------------------------------------
  Function: TDSHermiteInterpolate
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate the Hermite interpolation value used in 
               tweening frames of animation
  Arguments: 
    src_pt_left - key position previous to the source key position 
    src_pt - source key position = previous destination key position
    dest_pt - destination position = following source key position
    dest_pt_right - key position following destination position
    src_tension - value amount of arc exaggeration between source key
                  positions
    src_continuity - value amount of direction and speed change between
                     source key positions
    src_bias - value amount to overshoot a source key position
    dest_tension - value amount of arc exaggeration between distination
                   key positions
    dest_continuity - value amount of direction and speed change between
                      destination key positions
    dest_bias - value amount to overshoot a destination key position
    u - normalized vector representing the segmentation along a 
        parametric curve; value range [0.0..1.0]
  Return: floating point Hermite interpolation value
---------------------------------------------------------------------*/
float TDSHermiteInterpolate( float src_pt_left, float src_pt,
                             float dest_pt, float dest_pt_right,
                             float src_tension, float src_continuity,
                             float src_bias, float dest_tension,
                             float dest_continuity, float dest_bias,
                             float u )
{
  float h00, h01, h10, h11;
  float SD, DD;
  float hermite_interp_value;

  /* Hermite Interpolation Basis Functions -- if realtime, only update this 
     section once per frame and not per key point */
  h00 = 2*u*u*u - 3*u*u + 1;
  h01 = u*u*u - 2*u*u + u;
  h10 = -2*u*u*u + 3*u*u;
  h11 = u*u*u - u*u;

  /* calculate Hermite source derivative */
  SD = TDSHermiteSourceDerivative( src_tension, src_continuity, src_bias,
                                 src_pt_left, src_pt, dest_pt );

  /* calculate Hermite destination derivative */
  DD = TDSHermiteDestinationDerivative( dest_tension, dest_continuity, dest_bias,
                                      src_pt, dest_pt, dest_pt_right );

  /* calculate hermite interpolation value */
  hermite_interp_value = ( h00 * src_pt ) + ( h10 * dest_pt ) +
                         ( h01 * SD ) + ( h11 * DD );

  return hermite_interp_value;
}

/*---------------------------------------------------------------------
  Function: TDSLinearInterpolate
  Date: 4/3/96
  Library: 3DStudio Loader
  Description: calculate a linear interpolating value for tweening
               animation frames (good for robotic type movement)
  Arguments: 
    left - the from frame of the interpolation
    right - the to frame of the interpolation
    u - normalized vector representing the segmentation along a 
        parametric curve; value range [0.0..1.0]
  Return: floating point linear interpolation value
---------------------------------------------------------------------*/
float TDSLinearInterpolate( float left, float right, float u )
{
  return left + ( right - left ) * u;
}
