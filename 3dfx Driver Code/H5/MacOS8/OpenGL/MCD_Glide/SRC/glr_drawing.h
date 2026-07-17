/*________________________________________________________________________________________
** 
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
**________________________________________________________________________________________
**
**  Description: Context creation and destruction methods for the Apple OpenGL plugin
**
** 
**
*/


#ifndef __GLR_DRAWING_H__
#define __GLR_DRAWING_H__


/*_____ vertex layout definition _____*/

typedef struct {
  float  sow;                   /* s texture ordinate (s over w) */
  float  tow;                   /* t texture ordinate (t over w) */  
  float  oow;                   /* 1/w (used mipmapping - really 0xfff/w) */
}  GrTmuVertex;

typedef struct
{
  float x, y;         /* X and Y in screen space */
  float ooz;          /* 65535/Z (used for Z-buffering) */
  float oow;          /* 1/W (used for W-buffering, texturing) */
  FxU32 pargb;
  float _r, _g, _b, _a;   /* R, G, B, A [0..255.0] */
  float z;            /* Z is ignored */
  GrTmuVertex  tmuvtx[GLIDE_NUM_TMU];
} GrVertex;

#define GR_VERTEX_X_OFFSET              0
#define GR_VERTEX_Y_OFFSET              1
#define GR_VERTEX_OOZ_OFFSET            2
#define GR_VERTEX_OOW_OFFSET            3
#define GR_VERTEX_R_OFFSET              4
#define GR_VERTEX_G_OFFSET              5
#define GR_VERTEX_B_OFFSET              6
#define GR_VERTEX_A_OFFSET              7
#define GR_VERTEX_Z_OFFSET              8
#define GR_VERTEX_SOW_TMU0_OFFSET       9
#define GR_VERTEX_TOW_TMU0_OFFSET       10
#define GR_VERTEX_OOW_TMU0_OFFSET       11
#define GR_VERTEX_SOW_TMU1_OFFSET       12
#define GR_VERTEX_TOW_TMU1_OFFSET       13
#define GR_VERTEX_OOW_TMU1_OFFSET       14
#if (GLIDE_NUM_TMU > 2)
#define GR_VERTEX_SOW_TMU2_OFFSET       15
#define GR_VERTEX_TOW_TMU2_OFFSET       16
#define GR_VERTEX_OOW_TMU2_OFFSET       17
#endif

enum {
   kGlrNoFanOrStrip,
   kGlrFan,
   kGlrStrip
};

/*_____ functions and macros definition _____*/

#if 1
inline FxU32 packARGB(float a, float r, float g, float b)
{
  FxU32 result;
  double zero = 0.0, maxf = 255.0;
  double _a, _r, _g, _b;
  FxI32 aa, rr, gg, bb;  
  
  /* Use doubles to prevent compiler from using frsp all
     over the place.  Note the careful use of 255.0f so
     that the compiler generates a single precision multiply. */
  _a = a * 255.0f;
  _a = __fsel(_a,_a,zero);
  _a = __fsel(maxf - _a, _a, maxf);
  _r = r * 255.0f;
  _r = __fsel(_r,_r,zero);
  _r = __fsel(maxf - _r, _r, maxf);
  _g = g * 255.0f;
  _g = __fsel(_g,_g,zero);
  _g = __fsel(maxf - _g, _g, maxf);
  _b = b * 255.0f;
  _b = __fsel(_b,_b,zero);
  _b = __fsel(maxf - _b, _b, maxf);
  aa = _a;
  rr = _r;
  gg = _g;
  bb = _b;
  result = (aa << 24) | (rr << 16) | (gg << 8) | bb;
  return result;
}
#else
inline FxU32 packARGB(float a, float r, float g, float b)
{
  FxU32 result;
  FxI32 aa, rr, gg, bb;  
  
  aa = (a * 255.0f);
  rr = (r * 255.0f);
  gg = (g * 255.0f);
  bb = (b * 255.0f);
#if 1
  if(aa < 0) aa = 0;
  else if(aa > 255) aa = 255;
  if(rr < 0) rr = 0;
  else if(rr > 255) rr = 255;
  if(gg < 0) gg = 0;
  else if(gg > 255) gg = 255;
  if(bb < 0) bb = 0;
  else if(bb > 255) bb = 255;
#endif
  result = (aa << 24) | (rr << 16) | (gg << 8) | bb;
  return result;
}
#endif

#define glrAbs(a) ((a) < 0.0 ? -(a) : (a))

#define glrClampAndScale( cD, cS ) \
{ \
	if ( cS < 0.0 ) cD = 0.0; \
	else if ( cS > 1.0 ) cD = 1.0; \
	else cD = cS; \
	cD *= 255.0; \
}

#if 0	
#define GLR_CLIP_XY( ctx, vd ) \
{ \
	if ( (vd).x < 0.0 ) (vd).x = 0.0; \
	if ( (vd).x > (ctx)->viewPort.size.w - 1.0 ) (vd).x = (ctx)->viewPort.size.w - 1.0; \
	if ( (vd).y < 0.0 ) (vd).y = 0.0; \
	if ( (vd).y > (ctx)->viewPort.size.h - 1.0 ) (vd).y = (ctx)->viewPort.size.h - 1.0; \
}
#else
#define GLR_CLIP_XY( ctx, vd ) 
#endif
	
#if 0
#define glrLoadVertexR(v1, v2) ((v2).r = (v1).color.r * 3.89105058366e-3) // (255 / GLT_FIXED_COLOR_EXACTLY_ONE)
#define glrLoadVertexG(v1, v2) ((v2).g = (v1).color.g * 3.89105058366e-3)
#define glrLoadVertexB(v1, v2) ((v2).b = (v1).color.b * 3.89105058366e-3)
#define glrLoadVertexA(v1, v2) ((v2).a = (v1).color.a * 3.89105058366e-3)
#else
#define glrLoadVertexR(v1, v2) glrClampAndScale( (v2).r, (v1).color.r )
#define glrLoadVertexG(v1, v2) glrClampAndScale( (v2).g, (v1).color.g )
#define glrLoadVertexB(v1, v2) glrClampAndScale( (v2).b, (v1).color.b )
#define glrLoadVertexA(v1, v2) glrClampAndScale( (v2).a, (v1).color.a )
#endif

#define glrLoadVertexRGB(v1, v2)\
{\
    (v2).pargb = packARGB(1.0f,(v1).color.r,(v1).color.g,(v1).color.b); \
}

#define glrLoadVertexRGBA(v1, v2)\
{\
    (v2).pargb = packARGB((v1).color.a,(v1).color.r,(v1).color.g,(v1).color.b); \
}

//#define W_SCALE ((float)0x0fff)
#define W_SCALE ((float)1.0f)

#define glrLoadVertexTexture0(ctx, vs, vd)\
{\
	(vd).tmuvtx[0].sow = (vs).texture[0].s * ctx->hw_texture[0]->s_scale;\
	(vd).tmuvtx[0].tow = (vs).texture[0].t * ctx->hw_texture[0]->t_scale;\
	(vd).tmuvtx[0].oow = (vs).texture[0].q;\
}

#define glrLoadVertexMultiTexture0(ctx, vs, vd)\
{\
	(vd).tmuvtx[0].sow = (vs).texture[1].s * ctx->hw_texture[0]->s_scale;\
	(vd).tmuvtx[0].tow = (vs).texture[1].t * ctx->hw_texture[0]->t_scale;\
	(vd).tmuvtx[0].oow = (vs).texture[1].q;\
}

#define glrLoadVertexMultiTexture1(ctx, vs, vd)\
{\
	(vd).tmuvtx[1].sow = (vs).texture[0].s * ctx->hw_texture[1]->s_scale;\
	(vd).tmuvtx[1].tow = (vs).texture[0].t * ctx->hw_texture[1]->t_scale;\
	(vd).tmuvtx[1].oow = (vs).texture[0].q;\
}


#define glrOffsetWindowZ(offset_z, window_z) ((offset_z) + (window_z))

#define glrLoadVertex(ctx, vs, vd)\
{\
	(vd).x = (vs).window.x;\
	(vd).y = (vs).window.y;\
	(vd).ooz = (vs).window.z;\
	\
	(vd).oow = (vs).fog;\
}

#define glrLoadVertex_Texture(ctx, vs, vd)\
{\
	(vd).x = (vs).window.x;\
	(vd).y = (vs).window.y;\
	(vd).ooz = (vs).window.z;\
	\
	(vd).oow = (vs).fog;\
	\
	glrLoadVertexTexture0(ctx, vs, vd);\
}

#define glrLoadVertex_Flat(ctx, vs, vd, vc)\
{\
	(vd).x = (vs).window.x;\
	(vd).y = (vs).window.y;\
	(vd).ooz = (vs).window.z;\
	\
	(vd).oow = (vs).fog;\
	\
	(vd).pargb = (vc).pargb;\
}

#define glrLoadVertex_FlatTexture(ctx, vs, vd, vc)\
{\
	(vd).x = (vs).window.x;\
	(vd).y = (vs).window.y;\
	(vd).ooz = (vs).window.z;\
	\
	(vd).oow = (vs).fog;\
	\
	(vd).pargb = (vc).pargb;\
	\
	glrLoadVertexTexture0(ctx, vs, vd);\
}

#define glrLoadVertex_Smooth(ctx, vs, vd)\
{\
	(vd).x = (vs).window.x;\
	(vd).y = (vs).window.y;\
	(vd).ooz = (vs).window.z;\
	\
	(vd).oow = (vs).fog;\
	\
	glrLoadVertexRGBA(vs, vd);\
}

#define glrLoadVertex_SmoothTexture(ctx, vs, vd)\
{\
	(vd).x = (vs).window.x;\
	(vd).y = (vs).window.y;\
	(vd).ooz = (vs).window.z;\
	\
	(vd).oow = (vs).fog;\
	\
	glrLoadVertexRGBA(vs, vd);\
	\
	glrLoadVertexTexture0(ctx, vs, vd);\
}

#define glrLoadVertex_MultiTexture(ctx, vs, vd)\
{\
	(vd).x = (vs).window.x;\
	(vd).y = (vs).window.y;\
	(vd).ooz = (vs).window.z;\
	\
	(vd).oow = (vs).fog;\
	\
	glrLoadVertexMultiTexture0(ctx, vs, vd);\
	glrLoadVertexMultiTexture1(ctx, vs, vd);\
}

#define glrLoadVertex_FlatMultiTexture(ctx, vs, vd, vc)\
{\
	(vd).x = (vs).window.x;\
	(vd).y = (vs).window.y;\
	(vd).ooz = (vs).window.z;\
	\
	(vd).oow = (vs).fog;\
	\
	(vd).pargb = (vc).pargb;\
	\
	glrLoadVertexMultiTexture0(ctx, vs, vd);\
	glrLoadVertexMultiTexture1(ctx, vs, vd);\
}

#define glrLoadVertex_SmoothMultiTexture(ctx, vs, vd)\
{\
	(vd).x = (vs).window.x;\
	(vd).y = (vs).window.y;\
	(vd).ooz = (vs).window.z;\
	\
	(vd).oow = (vs).fog;\
	\
	glrLoadVertexRGBA(vs, vd);\
	\
	glrLoadVertexMultiTexture0(ctx, vs, vd);\
	glrLoadVertexMultiTexture1(ctx, vs, vd);\
}

#define glrVASetupIndices(indices) \
	const GLuint *thePtr; \
	thePtr = indices;


#define glrVAGetNextVertexPtr(inVtx) \
{\
      theOffset = (*thePtr++) - inFirst; \
      inVtx = theVtx_base + theOffset; \
}

#define glrLoadVertexFromArray_Smooth(ctx, vs, vd)\
{\
	(vd).x = (vs)->vertex.x;\
	(vd).y = (vs)->vertex.y;\
	(vd).ooz = (vs)->vertex.z;\
	\
	(vd).oow = (vs)->vertex.w;\
	\
	(vd).pargb = (vs)->color;\
}

#define glrLoadVertexFromArray_SmoothTexture(ctx, vs, vd)\
{\
	(vd).x = (vs)->vertex.x;\
	(vd).y = (vs)->vertex.y;\
	(vd).ooz = (vs)->vertex.z;\
	\
	(vd).oow = (vs)->vertex.w;\
	\
	(vd).pargb = (vs)->color;\
	\
	(vd).tmuvtx[0].sow = (vs)->sw1 * ctx->hw_texture[0]->s_scale;\
	(vd).tmuvtx[0].tow = (vs)->tw1 * ctx->hw_texture[0]->t_scale;\
	(vd).tmuvtx[0].oow = (vd).oow;\
}

#define glrLoadVertexFromArray_SmoothMultiTexture(ctx, vs, vd)\
{\
	(vd).x = (vs)->vertex.x;\
	(vd).y = (vs)->vertex.y;\
	(vd).ooz = (vs)->vertex.z;\
	\
	(vd).oow = (vs)->vertex.w;\
	\
	(vd).pargb = (vs)->color;\
	\
	(vd).tmuvtx[0].sow = (vs)->sw2 * ctx->hw_texture[0]->s_scale;\
	(vd).tmuvtx[0].tow = (vs)->tw2 * ctx->hw_texture[0]->t_scale;\
	(vd).tmuvtx[0].oow = (vs)->oow2;\
	\
	(vd).tmuvtx[1].sow = (vs)->sw1 * ctx->hw_texture[1]->s_scale;\
	(vd).tmuvtx[1].tow = (vs)->tw1 * ctx->hw_texture[1]->t_scale;\
	(vd).tmuvtx[1].oow = (vd).oow;\
}

static inline FxU32 cullTri(const float *fa, const float *fb, const float *fc, FxI32 signMode)  
{
#if 1
  /* Do culling for current triangle starting at inVtx */
  FxI32 j;
//  const float 
//  *fa = (const float*)&va->window.x,
//  *fb = (const float*)&vb->window.x,
//  *fc = (const float*)&vc->window.x;
  const float 
    dxAB = fa[0] - fb[0],
    dxBC = fb[0] - fc[0], 
    dyAB = fa[1] - fb[1],
    dyBC = fb[1] - fc[1],
    area = dxAB * dyBC - dxBC * dyAB;
  j = *(FxI32 *)&area;

  if(((j & 0x7fffffff) == 0) ||
     (((FxI32)(j ^ signMode)) >= 0)) {
    return 0;
  }
#endif  
  return 1;
}

static inline FxU32 glrCullTex1Tri(const GLDVertexArrayTex1Element *va, const GLDVertexArrayTex1Element *vb, const GLDVertexArrayTex1Element *vc, FxI32 signMode)  
{
  /* Do culling for current triangle starting at inVtx */
  FxI32 j;
  const float 
  *fa = (const float*)&va->vertex.x,
  *fb = (const float*)&vb->vertex.x,
  *fc = (const float*)&vc->vertex.x;
  const float 
    dxAB = fa[0] - fb[0],
    dxBC = fb[0] - fc[0], 
    dyAB = fa[1] - fb[1],
    dyBC = fb[1] - fc[1],
    area = dxAB * dyBC - dxBC * dyAB;
  j = *(FxI32 *)&area;

  if(((j & 0x7fffffff) == 0) ||
     (((FxI32)(j ^ signMode)) >= 0)) {
    return 0;
  }
  return 1;
}

static inline void glrSetPrimitive(
	GLDContext			inContext,
	GLenum				primitive)
{
	#pragma unused(inContext, primitive)
}


#define LOAD_FOG     0x0001
#define LOAD_COLOR   0x0002
#define LOAD_TEX0    0x0004
#define LOAD_TEX1    0x0008
#define LOAD_G4      0x0010

inline static void glrLoadVertexGeneric(
	GLDContext inContext,
	const GLDVertex * vs,
	GrVertex * vd,
	GLuint options)		// options should be a constant for best optimization
{
	vd->x = vs->window.x;
	vd->y = vs->window.y;
	vd->ooz = vs->window.z;
	
	if(options | LOAD_FOG){
		vd->oow = vs->fog;
	}
	
	if(options | LOAD_COLOR){
	    vd->pargb = packARGB(vs->color.a, vs->color.r, vs->color.g, vs->color.b);
	}
	
	switch(options & (LOAD_TEX0 | LOAD_TEX1)){
	case LOAD_TEX0:
		glrLoadVertexTexture0(inContext, *vs, *vd);
		break;
	case LOAD_TEX1:
		glrLoadVertexMultiTexture0(inContext, *vs, *vd);
		break;
	case LOAD_TEX0 | LOAD_TEX1:
		glrLoadVertexMultiTexture0(inContext, *vs, *vd);
		glrLoadVertexMultiTexture1(inContext, *vs, *vd);
		break;
	}	

}




#endif /* __GLR_DRAWING_H__ */


