#ifndef __sst_globals_h_
#define __sst_globals_h_

#include "sst_machdep.h"

void __glSSTEarlyInitContext(__GLcontext *gc);
GLboolean __glSSTDestroyContext(__GLcontext *gc);
void __glSSTEarlyInitTextureState(__GLcontext *gc);
void __glSSTEarlyInitPrimitiveState(__GLcontext *gc);
void __glSSTValidate(__GLcontext *gc);
void __glSSTPickAllProcs(__GLcontext *gc);
void __glSSTValidateDepthFunc(__GLcontext *gc);
void __glSSTValidateDepthTest(__GLcontext *gc);
void __glSSTValidateBlend(__GLcontext *gc);
void __glSSTValidateAlphaTest(__GLcontext *gc);
void __glSSTValidateAlphaFunc(__GLcontext *gc);
void __glSSTValidateCull(__GLcontext *gc);

void __glSSTInitRGB(__GLcolorBuffer *cfb, __GLcontext *gc );
void __glSSTInitAccum64(__GLaccumBuffer *afb, __GLcontext *gc );
void __glSSTInitDepth(__GLdepthBuffer *fb, __GLcontext *gc );
void __glSSTPickTriangleProcs(__GLcontext *gc);
void __glSSTPickLineProcs(__GLcontext *gc);
void __glSSTPickPointProcs(__GLcontext *gc);
void __glSSTPickTextureProcs(__GLcontext *gc);
void __glSSTPickPixelProcs(__GLcontext *gc);
void __glSSTPickReadPixels(__GLcontext *gc, GLint x, GLint y,
                           GLsizei width, GLsizei height,
                           GLenum format, GLenum type, const GLvoid *pixels);
void __glSSTPickDrawPixels(__GLcontext *gc, GLint width, GLint height,
                           GLenum format, GLenum type, const GLvoid *pixels,
                           GLboolean packed);
void __glSSTRenderSmoothTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                                 __GLvertex *c);
void __glSSTRenderFlatTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                               __GLvertex *c);
void __glSSTRenderTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                           __GLvertex *c);
void __glSSTLockRenderTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                           __GLvertex *c);
void __glSSTRenderFlatOneSidedTriangle(__GLcontext *gc, __GLvertex *a,
                                       __GLvertex *b, __GLvertex *c);
void __glSSTRenderSmoothOneSidedTriangle(__GLcontext *gc, __GLvertex *a,
                                         __GLvertex *b, __GLvertex *c);

void __glSSTRenderAliasedLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
void __glSSTRenderAliasedLine_Tex(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1);
void __glSSTRenderAliasedWideLine(__GLcontext *gc, __GLvertex *v0,
                                  __GLvertex *v1);
void __glSSTRenderAntiAliasedLine(__GLcontext *gc, __GLvertex *v0, 
                                  __GLvertex *v1);
void __glSSTRenderAntiAliasedLine_Tex(__GLcontext *gc, __GLvertex *v0, 
                                  __GLvertex *v1);
void __glSSTRenderAntiAliasedWideLine(__GLcontext *gc, __GLvertex *v0,
                                      __GLvertex *v1);
void __glSSTRenderAliasedPoint1_NoTex(__GLcontext *gc, __GLvertex *vx);
void __glSSTRenderAntiAliasedPoint1_NoTex(__GLcontext *gc, __GLvertex *vx);
void __glSSTRenderAliasedPoint1(__GLcontext *gc, __GLvertex *vx);
void __glSSTRenderAntiAliasedPoint1(__GLcontext *gc, __GLvertex *vx);
void __glSSTRenderAliasedPointN(__GLcontext *gc, __GLvertex *vx);
void __glSSTRenderAntiAliasedPointN(__GLcontext *gc, __GLvertex *vx);
void __glSSTComputePointSize(__GLcontext *gc, __GLvertex *vx);

void __glSSTInitTextureManager(__GLcontext *gc);
void __glSSTBindTexture(__GLcontext *gc, GLuint targetIndex, GLuint texture,
                        GLboolean freeingContext);
void __glSSTLoadCombineFunction(__GLcontext *gc);
void __glSSTEnableTexturing(__GLcontext *gc);
void __glSSTDisableTexturing(__GLcontext *gc);
void __glSSTFreeTextureState(__GLcontext *gc);

void __glSSTColorTableEXT(__GLcontext *gc, GLenum target, GLenum internalformat, 
                                  GLsizei width, GLenum format, GLenum type, 
                                  const void *table, GLboolean packed);

/************************************************************************/

/* XXXshui need to get unique name range */
#define __GL_SST_RGB_332   0x4000
#define __GL_SST_RGB_565   0x4001
#define __GL_SST_ARGB_4444 0x4002
#define __GL_SST_AI_88     0x4003
#define __GL_SST_AI_44     0x4004
#define __GL_SST_I_8       0x4005
#define __GL_SST_A_8       0x4006
#define __GL_SST_P_8       0x4007

/* amount of texture memory that can be allocated */
#define __GL_SST_TEXALLOC_NONE 0
#define __GL_SST_TEXALLOC_BASE 1
#define __GL_SST_TEXALLOC_STACK 2

/************************************************************************/
/*
** Primary dispatch tables
*/
extern __GLdispatchState __glSSTImmedState;
extern __GLdispatchState __glSSTListCompState;

/************************************************************************/

/* constants determined by the glide API */

#define __GL_SST_MAX_RED 0xff
#define __GL_SST_MAX_GREEN 0xff
#define __GL_SST_MAX_BLUE 0xff
#define __GL_SST_MAX_ALPHA 0xff

/************************************************************************/

/* No snapping in Glide 3 */
#define __GL_SST_SNAP_BIAS (0.0f)

/************************************************************************/

typedef struct {
  float  sow;                   /* s texture ordinate (s over w) */
  float  tow;                   /* t texture ordinate (t over w) */  
  float  oow;                   /* 1/w (used mipmapping - really 0xfff/w) */
}  GrTmuVertex;

/*
** GrVertex
** If these are changed the C & assembly language trisetup routines MUST
** be changed, for they will no longer work.
*/
typedef struct
{
  float x, y, z;                /* X, Y, and Z of scrn space -- Z is ignored */
  float r, g, b;                /* R, G, B, ([0..255.0]) */
  float ooz;                    /* 65535/Z (used for Z-buffering) */
  float a;                      /* Alpha [0..255.0] */
  float oow;                    /* 1/W (used for W-buffering, texturing) */
  GrTmuVertex  tmuvtx[GLIDE_NUM_TMU];
} GrVertex;


#include "db_trace.h"

#endif /* __sst_globals_h_ */
