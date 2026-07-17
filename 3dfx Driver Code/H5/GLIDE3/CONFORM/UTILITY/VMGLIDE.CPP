/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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

/*-----------------------------------------------------------------------------
@func vmgrAlphaCombine
@date 1/06/99
@arg GrCombineFunction_t - function - alpha combine function
@arg GrCombineFactor_t   - factor   - alpha combine factor
@arg GrCombineLocal_t    - local    - alpha combine local source
@arg GrCombineOther_t    - other    - alpha combine other source
@arg FxBool              - invert   - alpha combine invert output
@arg float *             - rslt     - result of alpha operation
@arg float               - iterated - vertex alpha
@arg float               - constant - constant alpha
@arg float               - texture  - texture alpha
@arg float               - hiz      - z value for operation
@return none
@imp
@key
@sect
@html
Floating point emulation of the Glide Alpha Combine Functionality 
@end
-----------------------------------------------------------------------------*/
void vmgrAlphaCombine(GrCombineFunction_t function, GrCombineFactor_t factor,
		      GrCombineLocal_t local, GrCombineOther_t other,
		      FxBool invert,
		      float *rslt,
		      float iterated,
		      float constant,
		      float texture,
		      float hiz)
{
  float factorval;
  float localval;
  float otherval;
  
  switch(local) {
  case GR_COMBINE_LOCAL_ITERATED:
    localval = iterated;
    break;
  case GR_COMBINE_LOCAL_CONSTANT:
    localval = constant;
    break;
  case GR_COMBINE_LOCAL_DEPTH:
    localval = hiz;
    break;
  default:
    localval = 0.0f;
  }
  
  switch(other) {
  case GR_COMBINE_OTHER_ITERATED:
    otherval = iterated;
    break;
  case GR_COMBINE_OTHER_TEXTURE:
    otherval = texture;
    break;
  case GR_COMBINE_OTHER_CONSTANT:
    otherval = constant;
    break;
  default:
    otherval = 0.0f;
  }
  
  switch(factor) {
  case GR_COMBINE_FACTOR_ZERO:
    factorval = 0.0f;
    break;
  case GR_COMBINE_FACTOR_LOCAL:
    factorval = localval;
    break;
  case GR_COMBINE_FACTOR_OTHER_ALPHA:
    factorval = otherval;
    break;
  case GR_COMBINE_FACTOR_LOCAL_ALPHA:
    factorval = localval;
    break;
  case GR_COMBINE_FACTOR_TEXTURE_ALPHA:
    factorval = texture;
    break;
  case GR_COMBINE_FACTOR_ONE:
    factorval = 1.0f;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_LOCAL:
    factorval = 1.0f - localval;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA:
    factorval = 1.0f - otherval;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA:
    factorval = 1.0f - localval;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA:
    factorval = 1.0f - texture;
    break;
  default:
    factorval = 0.0f;
  }

  switch(function) {
  case GR_COMBINE_FUNCTION_ZERO:
    *rslt = 0.0f;
    break;
  case GR_COMBINE_FUNCTION_LOCAL:
    *rslt = localval;
    break;
  case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
    *rslt = localval;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER:
    *rslt = factorval * otherval;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
    *rslt = (factorval * otherval) + localval;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
    *rslt = (factorval * otherval) + localval;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
    *rslt = factorval * (otherval - localval);
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
    *rslt = (factorval * (otherval - localval)) + localval;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
    *rslt = (factorval * (otherval - localval)) + localval;
    break;
  case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
    *rslt = (factorval * (-localval)) + localval;
    break;
  case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
    *rslt = (factorval * (-localval)) + localval;
    break;
  }
  
  // invert
  if(invert) *rslt = 1.0f - *rslt;

  // Clamp
  if(*rslt < 0.0f) *rslt = 0.0f;
  if(*rslt > 1.0f) *rslt = 1.0f;
}

/*-----------------------------------------------------------------------------
@func vmgrColorCombine
@date 1/13/99
@arg GrCombineFunction_t   - function - alpha combine function
@arg GrCombineFactor_t     - factor   - alpha combine factor
@arg GrCombineLocal_t      - local    - alpha combine local source
@arg GrCombineOther_t      - other    - alpha combine other source
@arg FxBool                - invert   - alpha combine invert output
@arg float *               - rslt {r,g,b}  - result of alpha operation
@arg float                 - iterated{r,g,b,a} - vertex alpha
@arg float                 - constant{r,g,b,a} - constant alpha
@arg float                 - texture{r,g,b,a} - texture alpha
@return none
@imp
@key
@sect
@html
Floating point emulation of the Glide Alpha Combine Functionality 
@end
-----------------------------------------------------------------------------*/
void vmgrColorCombine(GrCombineFunction_t function, GrCombineFactor_t factor,
		      GrCombineLocal_t local, GrCombineOther_t other,
		      FxBool invert,
		      float *rsltr,
		      float *rsltg,
		      float *rsltb,
		      float iteratedr,
		      float iteratedg,
		      float iteratedb,
		      float iterateda,
		      float constantr,
		      float constantg,
		      float constantb,
		      float constanta,
		      float texturer,
		      float textureg,
		      float textureb,
		      float texturea)
{
  float factorr;
  float factorg;
  float factorb;
  float localr;
  float localg;
  float localb;
  float locala;
  float otherr;
  float otherg;
  float otherb;
  float othera;
  
  switch(local) {
  case GR_COMBINE_LOCAL_ITERATED:
    localr = iteratedr;
    localg = iteratedg;
    localb = iteratedb;
    locala = iterateda;
    break;
  case GR_COMBINE_LOCAL_CONSTANT:
    localr = constantr;
    localg = constantg;
    localb = constantb;
    locala = constanta;
    break;
  default:
    localr = 0.0f;
    localg = 0.0f;
    localg = 0.0f;
    locala = 0.0f;
  }
  
  switch(other) {
  case GR_COMBINE_OTHER_ITERATED:
    otherr = iteratedr;
    otherg = iteratedg;
    otherb = iteratedb;
    othera = iterateda;
    break;
  case GR_COMBINE_OTHER_TEXTURE:
    otherr = texturer;
    otherg = textureg;
    otherb = textureb;
    othera = texturea;
    break;
  case GR_COMBINE_OTHER_CONSTANT:
    otherr = constantr;
    otherg = constantg;
    otherb = constantb;
    othera = constanta;
    break;
  default:
    otherr = 0.0f;
    otherg = 0.0f;
    otherb = 0.0f;
    othera = 0.0f;
  }
  
  switch(factor) {
  case GR_COMBINE_FACTOR_ZERO:
    factorr = 0.0f;
    factorg = 0.0f;
    factorb = 0.0f;
    break;
  case GR_COMBINE_FACTOR_LOCAL:
    factorr = localr;
    factorg = localg;
    factorb = localb;
    break;
  case GR_COMBINE_FACTOR_OTHER_ALPHA:
//    factorr = othera;
//    factorg = othera;
//    factorb = othera;
    factorr = texturea;
    factorg = texturea;
    factorb = texturea;
    break;
  case GR_COMBINE_FACTOR_LOCAL_ALPHA:
    factorr = locala;
    factorg = locala;
    factorb = locala;
    break;
  case GR_COMBINE_FACTOR_TEXTURE_ALPHA:
    factorr = texturea;
    factorg = texturea;
    factorb = texturea;
    break;
  case GR_COMBINE_FACTOR_ONE:
    factorr = 1.0f;
    factorg = 1.0f;
    factorb = 1.0f;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_LOCAL:
    factorr = 1.0f - localr;
    factorg = 1.0f - localg;
    factorb = 1.0f - localb;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA:
//    factorr = 1.0f - othera;
//    factorg = 1.0f - othera;
//    factorb = 1.0f - othera;
    factorr = 1.0f - texturea;
    factorg = 1.0f - texturea;
    factorb = 1.0f - texturea;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA:
    factorr = 1.0f - locala;
    factorg = 1.0f - locala;
    factorb = 1.0f - locala;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA:
    factorr = 1.0f - texturea;
    factorg = 1.0f - texturea;
    factorb = 1.0f - texturea;
    break;
  default:
    factorr = 0.0f;
    factorg = 0.0f;
    factorb = 0.0f;
  }

  switch(function) {
  case GR_COMBINE_FUNCTION_ZERO:
    *rsltr = 0.0f;
    *rsltg = 0.0f;
    *rsltb = 0.0f;
    break;
  case GR_COMBINE_FUNCTION_LOCAL:
    *rsltr = localr;
    *rsltg = localg;
    *rsltb = localb;
    break;
  case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
    *rsltr = locala;
    *rsltg = locala;
    *rsltb = locala;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER:
    *rsltr = factorr * otherr;
    *rsltg = factorg * otherg;
    *rsltb = factorb * otherb;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
    *rsltr = (factorr * otherr) + localr;
    *rsltg = (factorg * otherg) + localg;
    *rsltb = (factorb * otherb) + localb;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
    *rsltr = (factorr * otherr) + locala;
    *rsltg = (factorg * otherg) + locala;
    *rsltb = (factorb * otherb) + locala;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
    *rsltr = factorr * (otherr - localr);
    *rsltg = factorg * (otherg - localg);
    *rsltb = factorb * (otherb - localb);
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
    *rsltr = (factorr * (otherr - localr)) + localr;
    *rsltg = (factorg * (otherg - localg)) + localg;
    *rsltb = (factorb * (otherb - localb)) + localb;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
    *rsltr = (factorr * (otherr - localr)) + locala;
    *rsltg = (factorg * (otherg - localg)) + locala;
    *rsltb = (factorb * (otherb - localb)) + locala;
    break;
  case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
    *rsltr = (factorr * (-localr)) + localr;
    *rsltg = (factorg * (-localg)) + localg;
    *rsltb = (factorb * (-localb)) + localb;
    break;
  case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
    *rsltr = (factorr * (-localr)) + locala;
    *rsltg = (factorg * (-localg)) + locala;
    *rsltb = (factorb * (-localb)) + locala;
    break;
  }
  
  // invert
  if(invert) *rsltr = 1.0f - *rsltr;
  if(invert) *rsltg = 1.0f - *rsltg;
  if(invert) *rsltb = 1.0f - *rsltb;

  // Clamp
  if(*rsltr < 0.0f) *rsltr = 0.0f;
  if(*rsltr > 1.0f) *rsltr = 1.0f;
  if(*rsltg < 0.0f) *rsltg = 0.0f;
  if(*rsltg > 1.0f) *rsltg = 1.0f;
  if(*rsltb < 0.0f) *rsltb = 0.0f;
  if(*rsltb > 1.0f) *rsltb = 1.0f;
}



/*-----------------------------------------------------------------------------
@func vmgrDepthBufferFunction
@date 1/06/99
@arg GrCmpFnc_t  - mode           - depth buffer comparison function
@arg FxBool      - w_buffering    - FXTRUE if we're w buffering, FXFALSE otherwise
@arg float       - existing_depth - The value in the depth buffer 
@arg FxU32       - existing_color - The color in the framebuffer
@arg float       - incoming_depth - The depth we're testing against the depth buffer
@arg FxU32       - incoming_color - The color we'd draw if we won the test
@arg float *     - depth_out      - The depth that wins the test
@arg FxU32 *     - color_out      - The color that should be drawn
@return none
@imp
@key
@sect
@html
Given two depth and color pairs, return the depth and color pair
that would win the depth buffer test passed in in 'mode'. It computes
as if (existing_depth, existing_color) exists, and 
(incoming_depth, incoming_color) is being tested to
see if it would draw. 
@end
-----------------------------------------------------------------------------*/

void vmgrDepthBufferFunction(GrCmpFnc_t func, 
                             GrDepthBufferMode_t mode,
                             float existing_depth, FxU32 existing_color,
                             float incoming_depth, FxU32 incoming_color,
                             FxU32 depth_bias,
                             float *depth_out, FxU32 *color_out)
{

  float test_depth = 0.0f;
  *depth_out = existing_depth; // init to failed
  *color_out = existing_color; // init to failed

  // Tweak, if we're doing wbuffering.
  if ((mode == GR_DEPTHBUFFER_WBUFFER) ||
      (mode == GR_DEPTHBUFFER_WBUFFER_COMPARE_TO_BIAS)) {
    if (incoming_depth == 1.0f) {
      // The hardware maps 1.0f to 0.0f as per this explanation from
      // Ashok:
      // 1/w = 1.0 is iterated which is actually an overflow for the 
      // iterated value 
      // the floating point convertion which inverts the 1/w value 
      // will consider this an underflow and clamp  exp=0 and mantissa =0. 
      // So 0 will be stored in the fb. 
      incoming_depth = 0.0f; 
    } else {
      if (incoming_depth != 0.0) {
        incoming_depth = 1.0f/incoming_depth; // compare using reciprocal
      }
    }
  }

  if ((mode == GR_DEPTHBUFFER_WBUFFER_COMPARE_TO_BIAS) ||
      (mode == GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS)) {
    test_depth = (float)depth_bias;
  } else {
    test_depth = incoming_depth + depth_bias;
    // WALENDO: I think the bias is added to the resultant depth for
    //          non COMPARE_TO_BIAS cases
    incoming_depth = incoming_depth + depth_bias; 
  }

  switch(func) {
  case GR_CMP_NEVER:
    break;
  case GR_CMP_LESS:
    if (test_depth < existing_depth) { // TODO: need tolerances
      *depth_out = incoming_depth; 
      *color_out = incoming_color;
    }
    break;
  case GR_CMP_EQUAL:
    if (test_depth == existing_depth) { // TODO: need tolerances
      *depth_out = incoming_depth; 
      *color_out = incoming_color;
    }
    break;
  case GR_CMP_LEQUAL:
    if (test_depth <= existing_depth) { // TODO: need tolerances
      *depth_out = incoming_depth; 
      *color_out = incoming_color;
    }
    break;
  case GR_CMP_GREATER:
    if (test_depth > existing_depth) { // TODO: need tolerances
      *depth_out = incoming_depth; 
      *color_out = incoming_color;
    }
    break;
  case GR_CMP_NOTEQUAL:
    if (test_depth != existing_depth) { // TODO: need tolerances
      *depth_out = incoming_depth; 
      *color_out = incoming_color;
    }
    break;
  case GR_CMP_GEQUAL:
    if (test_depth >= existing_depth) { // TODO: need tolerances
      *depth_out = incoming_depth; 
      *color_out = incoming_color;
    }
    break;
  default:
  case GR_CMP_ALWAYS:
      *depth_out = incoming_depth; 
      *color_out = incoming_color;
    break;
  }
}

void vmgrInit(conform_state *state)
{
  render_state *rstate = &state->rstate;

  memset(&rstate->vtx_info, 0, sizeof(rstate->vtx_info));
}

/*-----------------------------------------------------------------------------
@func vmgrCoordinateSpace
@date 3/02/99
@arg conform_state *         - state - pointer to conformance state
@arg GrCoordinateSpaceMode_t - mode -  coordinate mode
@return none
@imp
@key
@sect
@html
Set the vm & glide coordinate mode
@end
-----------------------------------------------------------------------------*/
void vmgrCoordinateSpace(conform_state *state, GrCoordinateSpaceMode_t mode)
{
  render_state *rstate = &state->rstate;

  if (mode == GR_WINDOW_COORDS)
       rstate->vtx_info.mode &= ~VERTEX_CLIP_COORDS;
  else rstate->vtx_info.mode |= VERTEX_CLIP_COORDS;

  grCoordinateSpace(mode);
}

/*-----------------------------------------------------------------------------
@func vmgrVertexLayout
@date 3/02/99
@arg conform_state * - state  - pointer to conformance state
@arg FxU32           - param  - parameter to enable or disable
@arg FxU32           - offset - parameter offset
@arg FxU32           - mode   -  enable/disable parameter
@return none
@imp
@key
@sect
@html
Set the vm & glide vertex layout
@end
-----------------------------------------------------------------------------*/
void vmgrVertexLayout(conform_state *state, FxU32 param, FxI32 offset, FxU32 mode)
{
  render_state *rstate = &state->rstate;

  if ( param == GR_PARAM_XY ) {
  } else if ( param == GR_PARAM_Z ) {
    if (mode) 
         rstate->vtx_info.mode |=  VERTEX_Z;
    else rstate->vtx_info.mode &= ~VERTEX_Z;
  } else if ( param == GR_PARAM_W ) {
    if (mode) 
         rstate->vtx_info.mode |=  VERTEX_W;
    else rstate->vtx_info.mode &= ~VERTEX_W;
  } else if ( param == GR_PARAM_Q ) {
    if (mode) 
         rstate->vtx_info.mode |=  VERTEX_Q;
    else rstate->vtx_info.mode &= ~VERTEX_Q;
  } else if ( param == GR_PARAM_FOG_EXT ) {
    if (mode) 
         rstate->vtx_info.mode |=  VERTEX_FOG;
    else rstate->vtx_info.mode &= ~VERTEX_FOG;
  } else if (( param >= GR_PARAM_A ) && ( param < (GR_PARAM_A + MAX_TMU ))) {
    FxU32 bit = 1 << ( param - GR_PARAM_A);
    if (mode) 
         rstate->vtx_info.aTMUMode |= bit;
    else rstate->vtx_info.aTMUMode &= ~bit;
  } else if (( param >= GR_PARAM_RGB ) && ( param < (GR_PARAM_RGB + MAX_TMU ))) {
    FxU32 bit = 1 << ( param - GR_PARAM_RGB);
    if (mode) 
         rstate->vtx_info.rgbTMUMode |= bit;
    else rstate->vtx_info.rgbTMUMode &= ~bit;
  } else if (( param >= GR_PARAM_PARGB ) && ( param < (GR_PARAM_PARGB + MAX_TMU ))) {
    FxU32 bit = 1 << ( param - GR_PARAM_PARGB);
    if (mode) 
         rstate->vtx_info.pargbTMUMode |= bit;
    else rstate->vtx_info.pargbTMUMode &= ~bit;
  } else if (( param >= GR_PARAM_ST0 ) && ( param < (GR_PARAM_ST0 + MAX_TMU ))) {
    FxU32 bit = 1 << ( param - GR_PARAM_ST0);
    if (mode) 
         rstate->vtx_info.stTMUMode |= bit;
    else rstate->vtx_info.stTMUMode &= ~bit;
  } else if (( param >= GR_PARAM_Q0 ) && ( param < (GR_PARAM_Q0 + MAX_TMU ))) {
    FxU32 bit = 1 << ( param - GR_PARAM_Q0);
    if (mode) 
         rstate->vtx_info.qTMUMode |= bit;
    else rstate->vtx_info.qTMUMode &= ~bit;
  } else {
    char outstr[128];
    sprintf(outstr, "unsupported parameter type %d\n", param);
    log_message(state,outstr);
  }

  grVertexLayout(param, offset, mode);
}

/*-----------------------------------------------------------------------------
@func vmgrDepthRange
@date 3/02/99
@arg conform_state * - state - pointer to conformance state
@arg FxFloat - n - near distance
@arg FxFloat - f - far distance
@return none
@imp
@key
@sect
@html
Set the vm & glide depth range.
@end
-----------------------------------------------------------------------------*/
void vmgrDepthRange(conform_state *state, FxFloat n, FxFloat f) 
{
  render_state *rstate = &state->rstate;

  rstate->depth_range.n = n;
  rstate->depth_range.f = f;

  grDepthRange(n, f);
}

/*-----------------------------------------------------------------------------
@func vmgrViewport
@date 3/02/99
@arg conform_state * - state - pointer to conformance state
@arg FxI32 - x - x coordinate of viewport
@arg FxI32 - y - y coordinate of viewport
@arg FxI32 - width - width of viewport
@arg FxI32 - height - width of viewport
@return none
@imp
@key
@sect
@html
Set the vm & glide viewport
@end
-----------------------------------------------------------------------------*/
void vmgrViewport(conform_state *state, FxI32 x, FxI32 y, FxI32 width, FxI32 height) 
{
  render_state *rstate = &state->rstate;

  rstate->viewport.x = state->conform_x;
  rstate->viewport.y = state->conform_y;
  rstate->viewport.width = state->conform_width; 
  rstate->viewport.height = state->conform_height;

  grViewport(x, y, width, height);
}

/*-----------------------------------------------------------------------------
@func vmgrViewportVertex
@date 3/02/99
@arg conform_state * - state - pointer to conformance state
@arg Vertex * - dst - transformed vertex (in window cordinates)
@arg Vertex * - src - orignal vertex (in clip/window coordinates).
@return none
@imp
@key
@sect
@html
Apply the current viewport transform to the specified vertex. If the current
coordinate mode is clip coordinates the src should be in clip coordinates 
otherwise it should be in window coordinates. The result should be placed in
dst and is in window coordinates.
@end
-----------------------------------------------------------------------------*/
void vmgrViewportVertex(conform_state *state, Vertex *dst, Vertex *src)
{
  render_state *rstate = &state->rstate;
  int tmu;

  if ( rstate->vtx_info.mode & VERTEX_CLIP_COORDS ) { // vertex in clip ccordinates
    float sx = rstate->viewport.width  * 0.5f;
    float sy = rstate->viewport.height * 0.5f;
    float ox = (FxFloat)(rstate->viewport.x + rstate->viewport.width  * 0.5f);
    float oy = (FxFloat)(rstate->viewport.y + rstate->viewport.height * 0.5f);
    float w = ( rstate->vtx_info.mode & VERTEX_W ) ? src->w : 1.0f;

    dst->w = 1/w;
    dst->x = (src->x*dst->w)*sx + ox;
    dst->y = (src->y*dst->w)*sy + oy;
    if ( rstate->vtx_info.mode & VERTEX_Q )
         dst->q = src->q * dst->w;
    else dst->q = src->w;
    if ( rstate->vtx_info.mode & VERTEX_Z ) {
      float hdepth = (rstate->depth_range.f - rstate->depth_range.n) * 0.5f;
      float oz     = (rstate->depth_range.f + rstate->depth_range.n) * 0.5f;
      dst->z = (src->z * dst->w)*hdepth + oz;
    }
    for (tmu = 0; tmu < MAX_TMU; tmu++) {
      if (rstate->vtx_info.rgbTMUMode & (1 << tmu)) {
         dst->tmu[tmu].r = src->tmu[tmu].r; 
         dst->tmu[tmu].g = src->tmu[tmu].g; 
         dst->tmu[tmu].b = src->tmu[tmu].b; 
      }
      if (rstate->vtx_info.aTMUMode & (1 << tmu)) {
         dst->tmu[tmu].a = src->tmu[tmu].a; 
      }
      if (rstate->vtx_info.stTMUMode & (1 << tmu)) {
          dst->tmu[tmu].s = src->tmu[tmu].s*dst->w;
          dst->tmu[tmu].t = src->tmu[tmu].t*dst->w; 
      }
      if (rstate->vtx_info.qTMUMode & (1 << tmu)) {
        dst->tmu[tmu].q = src->tmu[tmu].q*dst->w;
     } else {
       dst->tmu[tmu].q = ( rstate->vtx_info.mode & VERTEX_W ) ? dst->w : dst->q;
      }
    }
  } else { // vertex in window coordinates
    dst->w = src->w;
    dst->x = src->x;
    dst->y = src->y;
    if ( rstate->vtx_info.mode & VERTEX_Q )
         dst->q = src->q;
    else dst->q = src->w;
    if ( rstate->vtx_info.mode & VERTEX_Z )
        dst->z = src->z;
    for (tmu = 0; tmu < MAX_TMU; tmu++) {
      if (rstate->vtx_info.rgbTMUMode & (1 << tmu)) {
         dst->tmu[tmu].r = src->tmu[tmu].r/255.0f; 
         dst->tmu[tmu].g = src->tmu[tmu].g/255.0f; 
         dst->tmu[tmu].b = src->tmu[tmu].b/255.0f; 
      }
      if (rstate->vtx_info.aTMUMode & (1 << tmu)) {
         dst->tmu[tmu].a = src->tmu[tmu].a/255.0f; 
      }
      if (rstate->vtx_info.stTMUMode & (1 << tmu)) {
        // for NPT textures the s & t texture coordinates should be in the
        // range of 0..s/tMax and are passed through unchanged. For POT
        // textures we convert them into a normalized form for the vm.
        if ( !rstate->tex_info[tmu].npt ) {
          float sScale, tScale;
          if ( rstate->tex_info[tmu].aspect > 0 ) {
            sScale = 1.0f/256.0f;
            tScale = ((float)(1<<(rstate->tex_info[tmu].aspect)))/256.0f;
          } else {
            sScale = ((float)(1<<(-rstate->tex_info[tmu].aspect)))/256.0f;
            tScale = 1.0f/256.0f;
          }
          dst->tmu[tmu].s = sScale*src->tmu[tmu].s;
          dst->tmu[tmu].t = tScale*src->tmu[tmu].t; 
        } 
      }

      if (rstate->vtx_info.qTMUMode & (1 << tmu)) {
         dst->tmu[tmu].q = src->tmu[tmu].q;
      } else {
         dst->tmu[tmu].q = ( rstate->vtx_info.mode & VERTEX_W ) ? dst->w : dst->q;
      }
    }
  }

  for (tmu = 0; tmu < MAX_TMU; tmu++) {
    if (rstate->vtx_info.pargbTMUMode & (1 << tmu)) {
      FxU32 pargb = src->tmu[tmu].argb;
      dst->tmu[tmu].a = ((float)((pargb & 0xff000000) >> 24))/255.0f;
      dst->tmu[tmu].r = ((float)((pargb & 0xff0000) >> 16))/255.0f;
      dst->tmu[tmu].g = ((float)((pargb & 0xff00) >> 8))/255.0f;
      dst->tmu[tmu].b = ((float)( pargb & 0xff))/255.0f;
    }
  }
}

/*-----------------------------------------------------------------------------
@func vmgrWindowToClip
@date 3/02/99
@arg conform_state * - state - pointer to conformance state
@arg Vertex * - dst - transformed vertex (in clip coordinates)
@arg Vertex * - src - orignal vertex (in window coordinates)
@return none
@imp
@key
@sect
@html
Convert a vertex from window to clip coordinates. This routine assumes that
src->w holds the 1/w value.
@end
-----------------------------------------------------------------------------*/
void vmgrWindowToClip(conform_state *state, Vertex *dst, Vertex *src)
{
  render_state *rstate = &state->rstate;
  int tmu;
  float sx = rstate->viewport.width  * 0.5f;
  float sy = rstate->viewport.height * 0.5f;
  float ox = (FxFloat)(rstate->viewport.x + rstate->viewport.width  * 0.5f);
  float oy = (FxFloat)(rstate->viewport.y + rstate->viewport.height * 0.5f);
  float w = ( rstate->vtx_info.mode & VERTEX_W ) ? src->w : 1.0f;

  dst->w = 1/src->w;
  dst->x = (src->x - ox)*dst->w/sx;
  dst->y = (src->y - oy)*dst->w/sy;
  if ( rstate->vtx_info.mode & VERTEX_Q )
      dst->q = src->q * dst->w;
  if ( rstate->vtx_info.mode & VERTEX_Z ) {
    float hdepth = (rstate->depth_range.f - rstate->depth_range.n) * 0.5f;
    float oz     = (rstate->depth_range.f + rstate->depth_range.n) * 0.5f;
    dst->z = (src->z - oz ) * dst->w/hdepth;
  }
  for (tmu = 0; tmu < MAX_TMU; tmu++) {
    if (rstate->vtx_info.rgbTMUMode & (1 << tmu)) {
       dst->tmu[tmu].r = src->tmu[tmu].r/255.0f; 
       dst->tmu[tmu].g = src->tmu[tmu].g/255.0f; 
       dst->tmu[tmu].b = src->tmu[tmu].b/255.0f; 
    }

    if (rstate->vtx_info.aTMUMode & (1 << tmu)) {
       dst->tmu[tmu].a = src->tmu[tmu].a/255.0f; 
    }

    if (rstate->vtx_info.pargbTMUMode & (1 << tmu)) {
      dst->tmu[tmu].argb = src->tmu[tmu].argb;
    }

    if (rstate->vtx_info.stTMUMode & (1 << tmu)) {
      float sScale, tScale;
      // for NPT textures the s & t texture coordinates should be in the
      // range of 0..s/tMax and are passed through unchanged. For POT
      // textures we convert them into a normalized form for the vm.
      if ( rstate->tex_info[tmu].npt ) {
          sScale = 1.0f;
          tScale = 1.0f;
      } else {
        if ( rstate->tex_info[tmu].aspect > 0 ) {
          sScale = 1.0f/256.0f;
          tScale = ((float)(1<<(rstate->tex_info[tmu].aspect)))/256.0f;
        } else {
          sScale = ((float)(1<<(-rstate->tex_info[tmu].aspect)))/256.0f;
          tScale = 1.0f/256.0f;
        }
      }
      dst->tmu[tmu].s = sScale*src->tmu[tmu].s*dst->w;
      dst->tmu[tmu].t = tScale*src->tmu[tmu].t*dst->w; 
    }

    if (rstate->vtx_info.qTMUMode & (1 << tmu)) {
       dst->tmu[tmu].q = src->tmu[tmu].q;
    }
  }
}

void
areaTriangle(Triangle * t)
{
    double dxAB, dyAB, dxBC, dyBC;

    dxAB = t->vA.x - t->vB.x;
    dyAB = t->vA.y - t->vB.y;
    dxBC = t->vB.x - t->vC.x;
    dyBC = t->vB.y - t->vC.y;
    t->area = dxAB * dyBC - dxBC * dyAB;    // NOTE: result is in .8 format
}

double edge_error(float x, float y, Edge *e)
{
    return e->scale * ( e->c - y * e->dedx + x * e->dedy);
}

#define MIN(a, b) ( (a) < (b ) ? (a) : (b))
#define MAX(a, b) ( (a) > (b ) ? (a) : (b))
#define ABS(x) (((x) > 0 ) ? (x) : -(x))
#define SQRT(x) sqrt((x))
#define L2(d1, d2) SQRT(d1*d1+d2*d2);

/*-----------------------------------------------------------------------------
@func vmgrInterpTriangle
@date 3/02/99
@arg float - x - x coordinate of interpolation point
@arg float - y - y coordinate of interpolation point
@arg conform_state * - state - pointer to conformance state
@arg Triangle * - t - triangle being interpolated
@arg Fragment * - l - fragment value at specified location
@arg 
@return none
@imp
@key
@sect
@html
Compute the fragment value for a specified location and triangle.
@end
-----------------------------------------------------------------------------*/
void vmgrInterpTriangle(float x, float y, conform_state *state, Triangle *t, Fragment *f)
{
  render_state *rstate = &state->rstate;
  int tmu;
  double dx = x - t->vA.x;
  double dy = y - t->vA.y;

  f->x = x;
  f->y = y;
  f->q = t->vA.q+dx*t->grd.dqdx+dy*t->grd.dqdy;
  f->dq = L2(t->grd.dqdx, t->grd.dqdy); 
  f->w = t->vA.w+dx*t->grd.dwdx+dy*t->grd.dwdy;
  f->dw = L2(t->grd.dwdx, t->grd.dwdy); 

  if ( rstate->vtx_info.mode & VERTEX_Z ) {
    f->z = t->vA.z+dx*t->grd.dzdx+dy*t->grd.dzdy;
    f->dz = L2(t->grd.dzdx, t->grd.dzdy); 
  }

  if ( rstate->vtx_info.mode & VERTEX_FOG ) {
    f->f = t->vA.f+dx*t->grd.dfdx+dy*t->grd.dfdy;
    f->df = L2(t->grd.dfdx, t->grd.dfdy); 
  }

  for (tmu = 0; tmu < MAX_TMU; tmu++) {
    if ((rstate->vtx_info.rgbTMUMode & (1 << tmu)) ||
        (rstate->vtx_info.pargbTMUMode & (1 << tmu))) {
      f->tmu[tmu].r = t->vA.tmu[tmu].r+dx*t->grd.tmu[tmu].drdx+dy*t->grd.tmu[tmu].drdy;
      f->tmu[tmu].g = t->vA.tmu[tmu].g+dx*t->grd.tmu[tmu].dgdx+dy*t->grd.tmu[tmu].dgdy;
      f->tmu[tmu].b = t->vA.tmu[tmu].b+dx*t->grd.tmu[tmu].dbdx+dy*t->grd.tmu[tmu].dbdy;
      f->tmu[tmu].dr = L2(t->grd.tmu[tmu].drdx, t->grd.tmu[tmu].drdy);
      f->tmu[tmu].dg = L2(t->grd.tmu[tmu].dgdx, t->grd.tmu[tmu].dgdy);
      f->tmu[tmu].db = L2(t->grd.tmu[tmu].dbdx, t->grd.tmu[tmu].dbdy);
    }
    if ((rstate->vtx_info.aTMUMode & (1 << tmu)) ||
        (rstate->vtx_info.pargbTMUMode & (1 << tmu))) {
      f->tmu[tmu].a = t->vA.tmu[tmu].a+dx*t->grd.tmu[tmu].dadx+dy*t->grd.tmu[tmu].dady;
      f->tmu[tmu].da = L2(t->grd.tmu[tmu].dadx, t->grd.tmu[tmu].dady);
    }
    if (rstate->vtx_info.stTMUMode & (1 << tmu)) {
      f->tmu[tmu].s = t->vA.tmu[tmu].s+dx*t->grd.tmu[tmu].dsdx+dy*t->grd.tmu[tmu].dsdy;
      f->tmu[tmu].t = t->vA.tmu[tmu].t+dx*t->grd.tmu[tmu].dtdx+dy*t->grd.tmu[tmu].dtdy;
      f->tmu[tmu].ds = L2(t->grd.tmu[tmu].dsdx, t->grd.tmu[tmu].dsdy);
      f->tmu[tmu].dt = L2(t->grd.tmu[tmu].dtdx, t->grd.tmu[tmu].dtdy);
    }
    f->tmu[tmu].q = t->vA.tmu[tmu].q+dx*t->grd.tmu[tmu].dqdx+dy*t->grd.tmu[tmu].dqdy;
    f->tmu[tmu].dq = L2(t->grd.tmu[tmu].dqdx, t->grd.tmu[tmu].dqdy);
  }
}

/*-----------------------------------------------------------------------------
@func vmgrInsideTriangle
@date 3/02/99
@arg float - x - x coordinate of point being tested
@arg float - y - y coordinate of point being tested
@arg Triangle * - t - triangle
@arg double - tolerance - tolerance
@return none
@imp
@key
@sect
@html
Determine whether a specified location is inside a specified triangle to 
within a given tolerance.
@end
-----------------------------------------------------------------------------*/
FxBool vmgrInsideTriangle(float x, float y, Triangle *t, double tolerance)
{
  t->dAB = edge_error(x, y, &t->eAB);
  t->dAC = edge_error(x, y, &t->eAC); 
  t->dBC = edge_error(x, y, &t->eBC);

  return ( t->dAB >= -tolerance ) && ( t->dAC >= -tolerance ) && ( t->dBC >= -tolerance );
}

/*-----------------------------------------------------------------------------
@func vmgrOutsideTriangle
@date 3/02/99
@arg float - x - x coordinate of point being tested
@arg float - y - y coordinate of point being tested
@arg Triangle * - t - triangle
@arg double - tolerance - tolerance
@return none
@imp
@key
@sect
@html
Determine whether a specified location is outside a specified triangle to 
within a given tolerance.
@end
-----------------------------------------------------------------------------*/
FxBool vmgrOutsideTriangle(float x, float y, Triangle *t, double tolerance)
{
  t->dAB = edge_error(x, y, &t->eAB);
  t->dAC = edge_error(x, y, &t->eAC); 
  t->dBC = edge_error(x, y, &t->eBC);

  return  ( t->dAB <= tolerance ) || ( t->dAC <= tolerance ) || ( t->dBC <= tolerance );
}

void make_edge(Vertex *v0, Vertex *v1, Vertex *op, Edge *e)
{
  double length;

  e->dedx = v1->x - v0->x;
  e->dedy = v1->y - v0->y;
  e->c = v0->y * e->dedx - v0->x * e->dedy;
  length = L2(e->dedx, e->dedy);

  e->scale = (length != 0.0) ? 1/length : 0.0;

  if ( edge_error( op->x, op->y, e ) < 0 )
      e->scale = -e->scale;
}

#define GRADIENT(p, dpdx, dpdy) \
  dpAB = t->vB.p - t->vA.p; \
  dpBC = t->vC.p - t->vB.p; \
  t->grd.dpdx = (dpAB * t->eBC.dedy - dpBC * t->eAB.dedy)*t->ooarea; \
  t->grd.dpdy = (dpBC * t->eAB.dedx - dpAB * t->eBC.dedx)*t->ooarea;

void vmgrComputeGradients(conform_state *state, Triangle *t) 
{
  render_state *rstate = &state->rstate;
  int tmu;
  double dpAB, dpBC;

  GRADIENT(w, dwdx, dwdy); 
  GRADIENT(q, dqdx, dqdy); 

  if ( rstate->vtx_info.mode & VERTEX_Z )
    GRADIENT(z, dzdx, dzdy); 

  if ( rstate->vtx_info.mode & VERTEX_FOG )
    GRADIENT(f, dfdx, dfdy); 

  for (tmu = 0; tmu < MAX_TMU; tmu++) {
    if ((rstate->vtx_info.rgbTMUMode & (1 << tmu)) ||
        (rstate->vtx_info.pargbTMUMode & (1 << tmu))) {
   		GRADIENT(tmu[tmu].r, tmu[tmu].drdx, tmu[tmu].drdy);
    	GRADIENT(tmu[tmu].g, tmu[tmu].dgdx, tmu[tmu].dgdy);
    	GRADIENT(tmu[tmu].b, tmu[tmu].dbdx, tmu[tmu].dbdy);
    }
    if ((rstate->vtx_info.aTMUMode & (1 << tmu)) ||
        (rstate->vtx_info.pargbTMUMode & (1 << tmu))) {
    	GRADIENT(tmu[tmu].a, tmu[tmu].dadx, tmu[tmu].dady);
    }
    if (rstate->vtx_info.stTMUMode & (1 << tmu)) {
    	GRADIENT(tmu[tmu].s, tmu[tmu].dsdx, tmu[tmu].dsdy);
    	GRADIENT(tmu[tmu].t, tmu[tmu].dtdx, tmu[tmu].dtdy);
    }
    if (rstate->vtx_info.qTMUMode & (1 << tmu)) {
    	GRADIENT(tmu[tmu].q, tmu[tmu].dqdx, tmu[tmu].dqdy);
    }
  }
}

extern "C" {
  FX_ENTRY void FX_CALL gdPause(void);
  FX_ENTRY void FX_CALL gdWritePixel(GrBuffer_t buffer, FxI32 x, FxI32 y, FxU32 pixel);
}

/*-----------------------------------------------------------------------------
@func vmgrMakeTriangle
@date 3/02/99
@arg conform_state * - state - pointer to conformance state
@arg Triangle * - t - triangle being constructed
@arg Vertex * - vA -  first vertex of triangle
@arg Vertex * - vB -  second vertex of triangle
@arg Vertex * - vC -  third vertex of triangle
@return none
@imp
@key
@sect
@html
Conostruct a triangle given its three vertices
@end
-----------------------------------------------------------------------------*/
void vmgrMakeTriangle(conform_state *state, Triangle *t, Vertex *va, Vertex *vb, Vertex *vc)
{
  float min_x, max_x, min_y, max_y;

  vmgrViewportVertex(state, &t->vA, va); 
  vmgrViewportVertex(state, &t->vB, vb); 
  vmgrViewportVertex(state, &t->vC, vc); 

  make_edge(&t->vA, &t->vB, &t->vC, &t->eAB);
  make_edge(&t->vA, &t->vC, &t->vB, &t->eAC);
  make_edge(&t->vB, &t->vC, &t->vA, &t->eBC);

  /* compute triangle area */

  t->area = t->eAB.dedx * t->eBC.dedy - t->eBC.dedx * t->eAB.dedy;
  t->ooarea = 1.0/t->area;
  areaTriangle(t);

  /* compute parameter gradients */

  vmgrComputeGradients(state, t);

  /* compute bounding box */

  min_x = MIN(t->vA.x, t->vB.x); min_x = MIN(min_x, t->vC.x);
  min_y = MIN(t->vA.y, t->vB.y); min_y = MIN(min_y, t->vC.y);
  max_x = MAX(t->vA.x, t->vB.x); max_x = MAX(max_x, t->vC.x);
  max_y = MAX(t->vA.y, t->vB.y); max_y = MAX(max_y, t->vC.y);

  t->bbox.min_x = (int)floor(min_x);
  t->bbox.min_y = (int)floor(min_y);
  t->bbox.max_x = (int)ceil(max_x);
  t->bbox.max_y = (int)ceil(max_y);
}

/*-----------------------------------------------------------------------------
@func vmgrMakeLine
@date 3/02/99
@arg conform_state * - state - pointer to conformance state
@arg Line * - l - line being constructed
@return none
@imp
@key
@sect
@html
Construct a line given its endpoints
@end
-----------------------------------------------------------------------------*/
void vmgrMakeLine(conform_state *state, Line *l, Vertex *va, Vertex *vb)
{
  render_state *rstate = &state->rstate;
  double dp;
  int tmu;
  double length, scale;
  float min_x, max_x, min_y, max_y;

  vmgrViewportVertex(state, &l->vA, va); 
  vmgrViewportVertex(state, &l->vB, vb); 

  /* compute line equation */

  l->dedx = l->vB.x - l->vA.x;
  l->dedy = l->vB.y - l->vA.y;

  if (( ABS(l->dedx) < .5) && ( ABS(l->dedy) < 0.5 ))
    l->major = N_MAJOR; // degenerate line
  else if ( ABS(l->dedx) > ABS(l->dedy) ) {
    l->major = X_MAJOR;
    dp = l->dedx;
  } else {
    l->major = Y_MAJOR;
    dp = l->dedy;
  }

  l->c = l->vA.y * l->dedx - l->vA.x * l->dedy;
  length = SQRT(l->dedx*l->dedx+l->dedy*l->dedy);
  scale = (length != 0.0) ? 1/length : 0.0;

  l->dedx *= scale;
  l->dedy *= scale;
  l->c *= scale;

  /* compute gradient */

  l->dqdp = ( l->vB.q - l->vA.q)/dp;
  l->dwdp = ( l->vB.w - l->vA.w)/dp;

  if ( rstate->vtx_info.mode & VERTEX_Z )
      l->dzdp = ( l->vB.z - l->vA.z)/dp;

  if ( rstate->vtx_info.mode & VERTEX_FOG )
      l->dfdp = ( l->vB.f - l->vA.f)/dp;

  for (tmu = 0; tmu < MAX_TMU; tmu++) {
    if ((rstate->vtx_info.rgbTMUMode & (1 << tmu)) ||
        (rstate->vtx_info.pargbTMUMode & (1 << tmu))) {
        l->tmu[tmu].drdp = ( l->vB.tmu[tmu].r - l->vA.tmu[tmu].r)/dp;
        l->tmu[tmu].dgdp = ( l->vB.tmu[tmu].g - l->vA.tmu[tmu].g)/dp;
        l->tmu[tmu].dbdp = ( l->vB.tmu[tmu].b - l->vA.tmu[tmu].b)/dp;
    }
    if ((rstate->vtx_info.aTMUMode & (1 << tmu)) ||
        (rstate->vtx_info.pargbTMUMode & (1 << tmu))) {
        l->tmu[tmu].dadp = ( l->vB.tmu[tmu].a - l->vA.tmu[tmu].a)/dp;
    }
    if (rstate->vtx_info.stTMUMode & (1 << tmu)) {
        l->tmu[tmu].dsdp = ( l->vB.tmu[tmu].s - l->vA.tmu[tmu].s)/dp;
        l->tmu[tmu].dtdp = ( l->vB.tmu[tmu].t - l->vA.tmu[tmu].t)/dp;
    }
    l->tmu[tmu].dqdp = ( l->vB.tmu[tmu].q - l->vA.tmu[tmu].q)/dp;
  }

  /* compute bounding box */

  min_x = MIN(l->vA.x, l->vB.x);
  min_y = MIN(l->vA.y, l->vB.y);
  max_x = MAX(l->vA.x, l->vB.x);
  max_y = MAX(l->vA.y, l->vB.y);

  l->bbox.min_x = (int)floor(min_x);
  l->bbox.min_y = (int)floor(min_y);
  l->bbox.max_x = (int)ceil(max_x);
  l->bbox.max_y = (int)ceil(max_y);
}

/*-----------------------------------------------------------------------------
@func vmgrOnLine
@date 3/02/99
@arg float - x - x coordinate of point
@arg float - y - y coordinate of point
@arg Line * - l - line
@arg double - tolerance - tolerance
@return none
@imp
@key
@sect
@html
Test whether a given point is on a given line to within a specified tolerance
@end
-----------------------------------------------------------------------------*/
FxBool vmgrOnLine(float x, float y, Line *l, double tolerance)
{
  l->error = l->c - y * l->dedx + x * l->dedy;
  l->error = ABS(l->error);

  return ( l->error <= tolerance );
}

/*-----------------------------------------------------------------------------
@func vmgrInterpLine
@date 3/02/99
@arg float - x - x coordinate of point
@arg float - y - y coordinate of point
@arg conform_state * - state - pointer to conformance state
@arg Line * - l - line
@arg Fragment * - f - fragment value
@return none
@imp
@key
@sect
@html
Compute a fragment value at a given point on a line
@end
-----------------------------------------------------------------------------*/
void vmgrInterpLine(float x, float y, conform_state *state, Line *l, Fragment *f)
{
  render_state *rstate = &state->rstate;
  int tmu;
  double dx = x - l->vA.x;
  double dy = y - l->vA.y;
  float dp;

  switch ( l->major ) {
  case X_MAJOR: 
    dp = (x-l->vA.x)/(l->vB.x-l->vA.x);
    break;
  case Y_MAJOR: 
    dp = (y-l->vA.y)/(l->vB.y-l->vA.y);
    break;
  case N_MAJOR: 
    break;
  }

  f->x = x;
  f->y = y;
  f->q = l->vA.q+dp*l->dqdp;
  f->dq = ABS(l->dqdp); 
  f->w = l->vA.w+dp*l->dwdp;
  f->dw = ABS(l->dwdp); 

  if ( rstate->vtx_info.mode & VERTEX_Z ) {
    f->z = l->vA.z+dp*l->dzdp;
    f->dz = ABS(l->dzdp); 
  }

  if ( rstate->vtx_info.mode & VERTEX_FOG ) {
      f->f = l->vA.f+dp*l->dfdp;
      f->df = ABS(l->dfdp); 
  }

  for (tmu = 0; tmu < MAX_TMU; tmu++) {
    if ((rstate->vtx_info.rgbTMUMode & (1 << tmu)) ||
        (rstate->vtx_info.pargbTMUMode & (1 << tmu))) {
        f->tmu[tmu].r = l->vA.tmu[tmu].r+dp*l->tmu[tmu].drdp;
        f->tmu[tmu].g = l->vA.tmu[tmu].g+dp*l->tmu[tmu].dgdp;
        f->tmu[tmu].b = l->vA.tmu[tmu].b+dp*l->tmu[tmu].dbdp;
        f->tmu[tmu].dr = ABS(l->tmu[tmu].drdp);
        f->tmu[tmu].dg = ABS(l->tmu[tmu].dgdp);
        f->tmu[tmu].db = ABS(l->tmu[tmu].dbdp);
    }
    if ((rstate->vtx_info.aTMUMode & (1 << tmu)) ||
        (rstate->vtx_info.pargbTMUMode & (1 << tmu))) {
        f->tmu[tmu].a = l->vA.tmu[tmu].a+dp*l->tmu[tmu].dadp;
        f->tmu[tmu].da = ABS(l->tmu[tmu].dadp);
    }
    if (rstate->vtx_info.stTMUMode & (1 << tmu)) {
        f->tmu[tmu].s = l->vA.tmu[tmu].s+dp*l->tmu[tmu].dsdp;
        f->tmu[tmu].t = l->vA.tmu[tmu].t+dp*l->tmu[tmu].dtdp;
        f->tmu[tmu].ds = ABS(l->tmu[tmu].dsdp);
        f->tmu[tmu].dt = ABS(l->tmu[tmu].dtdp);
    }
    f->tmu[tmu].q = l->vA.tmu[tmu].q+dp*l->tmu[tmu].dqdp;
    f->tmu[tmu].dq = ABS(l->tmu[tmu].dqdp);
  }
}

/*-----------------------------------------------------------------------------
@func vmgrClipWindow
@date 3/02/99
@arg conform_state * - state - pointer to conformance state
@arg FxI32 - minx - The lower x screen coordinate of the clipping window.
@arg FxI32 - miny - The lower y screen coordinate of the clipping window.
@arg FxI32 - maxx - The upper x screen coordinate of the clipping window.
@arg FxI32 - maxy - The upper y screen coordinate of the clipping window.
@return none
@imp
@key
@sect
@html
Clamp the coords passed in to actually be on the screen, and call 
grClipWindow() with the clamped values. Also, store the clamped
values in the state->render_state structure. Note that the values
passed in here can be negative.
@end
-----------------------------------------------------------------------------*/
void vmgrClipWindow(conform_state *state, 
                    FxI32 minx, FxI32 miny, FxI32 maxx, FxI32 maxy)
{

  // clamp to screen space
  if (minx < 0) minx = 0;
  if (miny < 0) miny = 0;
  if (minx > state->screen_width) minx = state->screen_width;   // -1?
  if (miny > state->screen_height) miny = state->screen_height; // -1?

  if (maxx < 0) maxx = 0;
  if (maxy < 0) maxy = 0;
  if (maxx > state->screen_width) maxx = state->screen_width;   // -1?
  if (maxy > state->screen_height) maxy = state->screen_height; // -1?

  // store in the render_state
  state->rstate.clip_window.minx = minx;
  state->rstate.clip_window.miny = miny;
  state->rstate.clip_window.maxx = maxx;
  state->rstate.clip_window.maxy = maxy;

  grClipWindow(state->rstate.clip_window.minx,
               state->rstate.clip_window.miny,
               state->rstate.clip_window.maxx,
               state->rstate.clip_window.maxy);
}

/*-----------------------------------------------------------------------------
@func vmgrGetPixels
@date 3/09/99
@arg conform_state * - state - pointer to conformance state
@arg FxU32* - mem - The location at which to store the data. This buffer 
                     MUST be at least (width * height * sizeof(FxU32)) big!
@arg GrBuffer_t - buffer - Which buffer to read from.
@arg BBox * - pBox - A pointer to the region to read. This bbox will be
                     clamped to the screen and updated. If a NULL pointer
                     is passed in, the check_region in the conform_state 
                     will be used.
@return none
@imp
@key
@sect
@html
Read back a region from a given buffer. The BBox passed in is clamped to
the screen and updated in place. Data is returned in 32bit wide chunks,
regardless of the native format. (eg. if the native format is 16bit 565,
data will be returned with the 565 data in the least significant halves of
a 32bit words).
@end
-----------------------------------------------------------------------------*/
FxBool vmgrGetPixels(conform_state *state, FxU32 *mem, GrBuffer_t buffer, 
                     BBox *pBoundingBox)
{ 
  FxU8 *dataptr;
  FxU8 *bufferptr;
  int j, native_size;
  int boxWidth, boxHeight;
  GrLfbInfo_t linfo;
  BBox *pBox;

  // if no bounding box was passed in, use the one in the conform_state
  if (!pBoundingBox) {
    pBox = &state->check_region;
  } else {
    pBox = pBoundingBox;
  }

  // Clamp the passed in BBox to the screen.
  if (pBox->min_x < 0) 
    pBox->min_x = 0;
  if (pBox->min_y < 0) 
    pBox->min_y = 0;
  if (pBox->min_x > (state->screen_width-1)) 
    pBox->min_x = state->screen_width-1;
  if (pBox->min_y > (state->screen_height-1)) 
    pBox->min_y = state->screen_height-1;

  if (pBox->max_x < 0) 
    pBox->max_x = 0;
  if (pBox->max_y < 0)
     pBox->max_y = 0;
  if (pBox->max_x > (state->screen_width-1)) 
    pBox->max_x = state->screen_width-1;
  if (pBox->max_y > (state->screen_height-1)) 
    pBox->max_y = state->screen_height-1; 

  boxWidth = (pBox->max_x - pBox->min_x) + 1;
  boxHeight = (pBox->max_y - pBox->min_y) + 1;

  // CVG Glide3 grLfbLock() doesn't do anything with linfo.writeMode
  // for read only locks at the moment. This value should just pass
  // through.
  linfo.writeMode = GR_LFBWRITEMODE_565; 
  linfo.size = sizeof(GrLfbInfo_t);

  //  figure out the size of the native FB data
  switch(linfo.writeMode) {
    case GR_LFBWRITEMODE_565:
    case GR_LFBWRITEMODE_555:
    case GR_LFBWRITEMODE_1555:
      native_size = sizeof(FxU16); // 16 bit
      break;
    case GR_LFBWRITEMODE_888:
    case GR_LFBWRITEMODE_8888:
    case GR_LFBWRITEMODE_565_DEPTH:
    case GR_LFBWRITEMODE_555_DEPTH:
    case GR_LFBWRITEMODE_1555_DEPTH:
      native_size = sizeof(FxU32); // 32 bit    
      break;
    default:
    log_perror(state, "Unsupported writemode came back from grLfbLock()");
  }

  // If we've got a platform-specific "get_pixels()" routine, use it.
  if (state->pfp.get_pixels) {
    void (FX_CALL *gdGetPixels)(FxI32 buffer, FxU32 xmin, FxU32 ymin, 
                                FxU32 width, FxU32 height, FxU32* mem);
    gdGetPixels = 
      (void (FX_CALL *)(FxI32, FxU32, FxU32, 
                        FxU32, FxU32, FxU32*)) state->pfp.get_pixels;

    gdGetPixels(buffer,
                pBox->min_x,
                ( state->origin == GR_ORIGIN_UPPER_LEFT ) ? 
                    pBox->min_y :
                    state->screen_height - 1 - pBox->max_y,
                (pBox->max_x - pBox->min_x) + 1, // width
                (pBox->max_y - pBox->min_y) + 1, // height
                mem);

  } else { // just use normal lfb reads

    /**
     ** NOTE: The code for reading back from the LFB is pretty much the same 
     **       as in end_frame(). If you make a change here, it probably needs
     **       to be made there, too. (and vice-versa)
     **/
  
    if(!grLfbLock(GR_LFB_READ_ONLY, buffer,
                  GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXFALSE,
                  &linfo)) {
      // unable to lock buffer
      log_message(state,"vmgrGetPixels(): Could not lock buffer");
      return FXFALSE;
    }

    // read data from buffer
    dataptr = (FxU8 *)linfo.lfbPtr;
    switch(state->origin) {
    case GR_ORIGIN_UPPER_LEFT:
      dataptr += (pBox->min_y * linfo.strideInBytes)+
                 (pBox->min_x * native_size);
      break;
    case GR_ORIGIN_LOWER_LEFT:
      dataptr += ((state->screen_height -
                   (boxHeight + pBox->min_y))*
                  linfo.strideInBytes) + (pBox->min_x * native_size);
      break;
    }
  
    // read native data into our 32bit buffer scanline by scanline
    bufferptr = (FxU8*)mem;
    for(j = 0; j < boxHeight; j++) {
      lfb_read_data(state, bufferptr, dataptr, boxWidth * native_size);
      dataptr += linfo.strideInBytes;
      bufferptr += boxWidth * sizeof(FxU32);
    }
  
    // unlock
    grLfbUnlock(GR_LFB_READ_ONLY, buffer);

    // At this point, we've read the native format data into our
    // 32bit buffer. If the native data was 16 bit, a scanline takes
    // up half of our 32bit scanline. We need to expand each 16bit
    // pixel to take up the least significant half of 32bits.

    expand_native_buffer(state, &linfo, (FxU8*)mem, boxWidth, boxHeight);
  }

  // The memory image is always forced to be upper left origin
  // independent of screen image
  if(state->origin == GR_ORIGIN_LOWER_LEFT) {
    // invert memory buffer if origin is upper left
    FxU32 *startbuffer;
    FxU32 *endbuffer;
    FxU32 tmpval;
    int x, y;
    FxU32 *curbufptr = (FxU32*)mem;
    
    for(y = 0; y < (boxHeight/2); y++) {
      startbuffer = curbufptr + (y * boxWidth);
      endbuffer = curbufptr + ((boxHeight- y - 1) * boxWidth);
      for(x = 0; x < boxWidth; x++) {
        tmpval = *startbuffer;
        *startbuffer++ = *endbuffer;
        *endbuffer++ = tmpval;
      }
    }
  }

  return FXTRUE;
}

extern "C" {
  FX_ENTRY void FX_CALL gdPause(void);
  FX_ENTRY void FX_CALL gdLfbRead(FxU8 *memptr, FxU8 *lfbptr, int num_bytes);
  FX_ENTRY void FX_CALL gdLfbWrite(FxU8 *lfbptr, FxU8 *memptr, int num_bytes);
  FX_ENTRY FxBool FX_CALL gdGetPixels(FxI32 buffer, FxU32 xmin, FxU32 ymin, 
                                      FxU32 width, FxU32 height, FxU32* mem);
}

#ifdef __WIN32__
# include <windows.h> 
#endif // __WIN32__
void* vmgrGetProcAddress( char *procName )
{
  void* proc = NULL;

#ifdef __WIN32__
  HMODULE glideLib =  GetModuleHandle("glide3x.dll");

  if (glideLib) {
    proc = GetProcAddress(glideLib, procName);
  }

#else // not WIN32
  // Unix stuff goes here...
  if ( !strcmp( procName, "_gdPause@0")) {
      proc = (void *)&gdPause;
  } else if ( !strcmp( procName, "_gdLfbRead@12")) {
      proc = (void *)gdLfbRead;
  } else if ( !strcmp( procName, "_gdLfbWrite@12")) {
      proc = (void *)gdLfbWrite;
  } else if ( !strcmp( procName, "_gdGetPixels@24")) {
      proc = (void *)gdGetPixels;
  }
#endif // not WIN32

  return  proc;
}

/*------------------------------------------------------------------------
@func vmgrInsideClipWindow
@arg conform_state *state - the framework's state
@arg float x,y - the point to test
@return FXTRUE if the point is within the clip window, FXFALSE otherwise
@html
Test whether a given (x,y) is inside the current clip window (as specified
by a call to vmgrClipWindow()).
@end
-------------------------------------------------------------------------*/
FxBool vmgrInsideClipWindow(conform_state *state, float x, float y)
{
  return ( (x >= state->rstate.clip_window.minx) &&
           (y >= state->rstate.clip_window.miny) &&
           (x <= state->rstate.clip_window.maxx) &&
           (y <= state->rstate.clip_window.maxy) );
}
