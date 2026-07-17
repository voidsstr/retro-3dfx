#ifndef __CLIP_H__
#define __CLIP_H__

#ifdef USE_GLIDE3
#include "revtypes.h"
#else
#include <glide.h>
#endif // USE_GLIDE3

void SetClipVolume(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z);
unsigned long ComputeOutcode(const GrVertex *v);
int ClipAndDrawTriangle(const GrVertex *v0, const GrVertex *v1, const GrVertex *v2);
int ClipAndDrawLine(const GrVertex *v0, const GrVertex *v1);
int ClipPolyVerts(GrVertex *clipped_verts, int src_num_verts, const GrVertex *src_verts);

#endif // __CLIP_H__
