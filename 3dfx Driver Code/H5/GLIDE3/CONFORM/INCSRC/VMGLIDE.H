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

// virtual Glide Prototypes

#ifndef VMGLIDE_H
#define VMGLIDE_H
void vmgrAlphaCombine(GrCombineFunction_t function, GrCombineFactor_t factor,
		      GrCombineLocal_t local, GrCombineOther_t other,
		      FxBool invert,
		      float *rslt,
		      float iterated,
		      float constant,
		      float texture,
		      float hiz);

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
		      float texturea);

void vmgrDepthBufferFunction(GrCmpFnc_t func, 
                             GrDepthBufferMode_t mode,
                             float existing_depth, FxU32 existing_color,
                             float incoming_depth, FxU32 incoming_color,
                             FxU32 depth_bias,
                             float *depth_out, FxU32 *color_out);

void vmgrInit(conform_state *state);

void vmgrCoordinateSpace(conform_state *state, GrCoordinateSpaceMode_t mode);

void vmgrVertexLayout(conform_state *state, FxU32 param, FxI32 offset, FxU32 mode);

void vmgrDepthRange(conform_state *state, FxFloat n, FxFloat f);

void vmgrViewport(conform_state *state, FxI32 x, FxI32 y, FxI32 width, FxI32 height);

void vmgrViewportVertex(conform_state *state, Vertex *dst, Vertex *src);

void vmgrWindowToClip(conform_state *state, Vertex *dst, Vertex *src);

void vmgrMakeTriangle(conform_state *state, Triangle *t, Vertex *va, Vertex *vb, Vertex *vc);

FxBool vmgrInsideTriangle(float x, float y, Triangle *t, double tolerance);

FxBool vmgrOutsideTriangle(float x, float y, Triangle *t, double tolerance);

void vmgrInterpTriangle(float x, float y, conform_state *state, Triangle *t, Fragment *f);

void vmgrMakeLine(conform_state *state, Line *l, Vertex *va, Vertex *vb);

FxBool vmgrOnLine(float x, float y, Line *l, double tolerance);

void vmgrInterpLine(float x, float y, conform_state *state, Line *l, Fragment *f);

void vmgrClipWindow(conform_state *state, 
                    FxI32 minx, FxI32 miny, FxI32 maxx, FxI32 maxy);

FxBool vmgrGetPixels(conform_state *state, FxU32 *mem, GrBuffer_t buffer, 
                     BBox *pBox);

void* vmgrGetProcAddress( char *procName );

FxBool vmgrInsideClipWindow(conform_state *state, float x, float y);

#endif // VMGLIDE_H
