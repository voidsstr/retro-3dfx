#include <windows.h>
#include "context.h"
#include "global.h"
#include "fmemclr.h"
#include <glide.h>

/*
 * Re-numbered clip planes so that plane>>1 is an offset
 * into the vector [x y z].
 */
#define CC_RIGHT       0x01
#define CC_LEFT        0x02
#define CC_ABOVE       0x04
#define CC_BELOW       0x08
#define CC_BEHIND      0x10
#define CC_BEFORE      0x20

#define SHIFT_CC_RIGHT       0
#define SHIFT_CC_LEFT        1
#define SHIFT_CC_ABOVE       2
#define SHIFT_CC_BELOW       3
#define SHIFT_CC_BEHIND      4
#define SHIFT_CC_BEFORE      5

/* 
 * These are intended to be read-only!!! 
 * (which is why they aren't in the context
 */
// Vertex assembly state.
#define     SNOP_A      ( 0  * sizeof(int))

#define     STRI_A      ( 1  * sizeof(int))
#define     STRI_B      ( 2  * sizeof(int))
#define     STRI_C      ( 3  * sizeof(int))

#define     SQUAD_A     ( 4  * sizeof(int))
#define     SQUAD_B     ( 5  * sizeof(int))
#define     SQUAD_C     ( 6  * sizeof(int))
#define     SQUAD_D     ( 7  * sizeof(int))

#define     SFAN_A      ( 8  * sizeof(int))
#define     SFAN_B      ( 9  * sizeof(int))
#define     SFAN_C      ( 10 * sizeof(int))
#define     SFAN_D      ( 11 * sizeof(int))

#define     SSTRIP_A    ( 12 * sizeof(int))
#define     SSTRIP_B    ( 13 * sizeof(int))
#define     SSTRIP_C    ( 14 * sizeof(int))
#define     SSTRIP_D    ( 15 * sizeof(int))
#define     SSTRIP_E    ( 16 * sizeof(int))
#define     SSTRIP_F    ( 17 * sizeof(int))
#define     SSTRIP_G    ( 18 * sizeof(int))
#define     SSTRIP_H    ( 19 * sizeof(int))

static int _VDRAW[]= 
{  0,   0, 0, 1,   0, 0, 1, 1,   0, 0, 1, 1,   0, 0, 1, 3, 1, 3, 1, 3};

#define VERT(x) ((x) * ((int)sizeof(IVert)))

static int _VSTEP_A[]= 
{  VERT(0),   VERT(0), VERT(0), VERT(0),   VERT(0), VERT(0), VERT(0), VERT(0),   VERT(0), VERT(0), VERT(0), VERT(0),   VERT(0), VERT(0), VERT(1), VERT(1),-VERT(2), VERT(1), VERT(1),-VERT(2)};

static int _VSTEP_C[]= 
{  VERT(0),   VERT(1), VERT(1),-VERT(2),   VERT(1), VERT(1),-VERT(1),-VERT(1),   VERT(1), VERT(1),-VERT(1), VERT(1),   VERT(1), VERT(1),-VERT(2), VERT(1), VERT(1),-VERT(2), VERT(1), VERT(1)};

#undef VERT

static int _VNEXT[] = {
    SNOP_A   ,
    STRI_B   , STRI_C   , STRI_A   ,
    SQUAD_B  , SQUAD_C  , SQUAD_D  , SQUAD_A  ,
    SFAN_B   , SFAN_C   , SFAN_D   , SFAN_C   ,
    SSTRIP_B , SSTRIP_C , SSTRIP_D , SSTRIP_E , 
    SSTRIP_F , SSTRIP_G , SSTRIP_H , SSTRIP_C ,
};

static int _VSTART[16] = {
    SNOP_A    ,    // GL_POINTS
    /*SNOP_A*/ 0  ,    // GL_LINES
    SNOP_A    ,    // GL_LINE_LOOP
    /*SNOP_A*/ 2  ,    // GL_LINE_STRIP

    STRI_A     ,    // GL_TRIANGLES
    SSTRIP_A   ,    // GL_TRIANGLE_STRIP
    SFAN_A     ,    // GL_TRIANGLE_FAN
    SQUAD_A    ,    // GL_QUADS
    SSTRIP_A   ,    // GL_QUAD_STRIP
    SFAN_A     ,    // GL_POLYGON

    SNOP_A     ,    // FILL
    SNOP_A     ,    // FILL
    SNOP_A     ,    // FILL
    SNOP_A     ,    // FILL
    SNOP_A     ,    // FILL
    SNOP_A     ,    // FILL
};

void __glSSTClipAndDraw(unsigned int, IVert*, IVert*, IVert*);
void __glSSTClipAndDraw_B(unsigned int, IVert*, IVert*, IVert*);

void
__glSSTBegin(__GLcontext *gc, GLenum mode) {
    if ( mode == GL_LINES || mode == GL_LINE_STRIP ) {
        gc->primState.vA = &gc->primState.vertex[0];
        gc->primState.vB = &gc->primState.vertex[1];
    } else {
        gc->primState.vA =  gc->primState.vB = gc->primState.vC = 
            gc->primState.vertex;
    }
    gc->primState.vState = _VSTART[(mode - GL_POINTS) & 0xf];
    if (gc->texture.currentTexture[0]) {
        __GLmipMapLevel *lp = &gc->texture.currentTexture[0]->level[0];
        gc->primState.tc[0].sBias = GL_UINTCAST(lp->width2f) - 0x3f800000;
        gc->primState.tc[0].tBias = GL_UINTCAST(lp->height2f) - 0x3f800000;
    }
    if (gc->texture.currentTexture[1]) {
        __GLmipMapLevel *lp = &gc->texture.currentTexture[1]->level[0];
        gc->primState.tc[1].sBias = GL_UINTCAST(lp->width2f) - 0x3f800000;
        gc->primState.tc[1].tBias = GL_UINTCAST(lp->height2f) - 0x3f800000;
    }
}

void APIENTRY __glsstim_Vertex4fv_0_I(const GLfloat *v) {
    __GL_SETUP();
    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                                  // clip flags
    IVert       *s = ps->vC;                       // destination
    IVert       *savA, *savB, *savC;

    if (v[3] == 1.0f) {
        s->X = v[0] * m[0] + m[12];
        s->Y = v[1] * m[5] + m[13];
        s->Z = v[2] * m[10] + m[14];
        s->W = s->oow = 1.0f;
    } else {
        s->X = v[0] * m[0] +  v[3] * m[12];
        s->Y = v[1] * m[5] +  v[3] * m[13];
        s->Z = v[2] * m[10] + v[3] * m[14];
        s->W = s->oow = v[3];
    }

    if ( ((int)GL_UINTCAST( s->W )) <= 0 ) {
        s->oow = 1.0f;
        f = CC_BEFORE; 
    } else {
        s->oow = 1.0f / s->W;
        
        /* 
         * Everything below here till projection ought to be free because of
         * the oow computation above. It should use only integer arithmetic!
         */

        /* Clip codes */
        f  = (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Z) >> 31)+1);
             
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Y) >> 31)+1);
    
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->X) >> 31)+1);
    }

    /* Save away clip code */
    s->flags      = f;

    /* Copy colors, scale and clamp to 0..255 */
    s->r   = gc->state.current.color.r;
    s->g   = gc->state.current.color.g;
    s->b   = gc->state.current.color.b;
    s->a   = gc->state.current.color.a;

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[0].x) + gc->primState.tc[0].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[0].y) + gc->primState.tc[0].tBias;
    s->t0_oow = gc->state.current.texture[0].w;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;
    savC = ps->vC;

    /* Update new state */
    ps->vA     = (IVert *) (((char *)ps->vA) + _VSTEP_A[ps->vState >> 2]);
    ps->vB     = ps->vC;
    ps->vC     = (IVert *) (((char *)ps->vC) + _VSTEP_C[ps->vState >> 2]);
    vdraw      = _VDRAW  [ps->vState >> 2];
    ps->vState = _VNEXT  [ps->vState >> 2];

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * s->oow * vp->xScale + vp->xCenter;
        s->y   = s->Y   * s->oow * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * s->oow * vp->zScale + vp->zCenter;
        s->t0_sow = s->t0_sow * s->oow;
        s->t0_tow = s->t0_tow * s->oow;
	s->t0_oow = s->t0_oow * s->oow;
	s->oow = s->t0_oow;
    }

    if (vdraw) {
       if (vdraw & 2) { IVert *t = savA; savA = savB; savB = t; /* swap */}

       if (!(f | savA->flags | savB->flags)) {
          grDrawTriangle((GrVertex*) savA, (GrVertex*) savB, (GrVertex*) savC);
       } else {
          __glSSTClipAndDraw(SHIFT_CC_BEFORE, savA, savB, savC);
       }
    }
}

#ifndef __GL_USE_INTEL_ASM

void APIENTRY __glsstim_Vertex2fv_0(const GLfloat *v) {
    __GL_SETUP();
    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                                  // clip flags
    IVert       *s = ps->vC;                       // destination
    IVert       *savA, *savB, *savC;

    s->X = m[0] * v[0] + m[4] * v[1] + m[12]; 
    s->Y = m[1] * v[0] + m[5] * v[1] + m[13];
    s->Z = m[2] * v[0] + m[6] * v[1] + m[14];
    s->W = m[3] * v[0] + m[7] * v[1] + m[15];

    if ( ((int)GL_UINTCAST( s->W )) <= 0 ) {
        s->oow = 1.0f;
        f = CC_BEFORE; 
    } else {
        s->oow = 1.0f / s->W;
        
        /* 
         * Everything below here till projection ought to be free because of
         * the oow computation above. It should use only integer arithmetic!
         */

        /* Clip codes */
        f  = (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Z) >> 31)+1);
             
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Y) >> 31)+1);
    
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->X) >> 31)+1);
    }

    /* Save away clip code */
    s->flags      = f;

    /* Copy colors, scale and clamp to 0..255 */
    s->r   = gc->state.current.color.r;
    s->g   = gc->state.current.color.g;
    s->b   = gc->state.current.color.b;
    s->a   = gc->state.current.color.a;

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[0].x) + gc->primState.tc[0].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[0].y) + gc->primState.tc[0].tBias;
    s->t0_oow = gc->state.current.texture[0].w;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;
    savC = ps->vC;

    /* Update new state */
    ps->vA     = (IVert *) (((char *)ps->vA) + _VSTEP_A[ps->vState >> 2]);
    ps->vB     = ps->vC;
    ps->vC     = (IVert *) (((char *)ps->vC) + _VSTEP_C[ps->vState >> 2]);
    vdraw      = _VDRAW  [ps->vState >> 2];
    ps->vState = _VNEXT  [ps->vState >> 2];

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * s->oow * vp->xScale + vp->xCenter;
        s->y   = s->Y   * s->oow * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * s->oow * vp->zScale + vp->zCenter;
        s->t0_sow = s->t0_sow * s->oow;
        s->t0_tow = s->t0_tow * s->oow;
	s->t0_oow = s->t0_oow * s->oow;
	s->oow = s->t0_oow;
    }

    if (vdraw) {
       if (vdraw & 2) { IVert *t = savA; savA = savB; savB = t; /* swap */}

       if (!(f | savA->flags | savB->flags)) {
          grDrawTriangle((GrVertex*) savA, (GrVertex*) savB, (GrVertex*) savC);
       } else {
          __glSSTClipAndDraw(SHIFT_CC_BEFORE, savA, savB, savC);
       }
    }
}

void APIENTRY __glsstim_Vertex3fv_0_I(const GLfloat *v) {
    __GL_SETUP();
    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                                  // clip flags
    IVert       *s = ps->vC;                       // destination
    IVert       *savA, *savB, *savC;

    s->X = v[0] * m[0] + m[12];
    s->Y = v[1] * m[5] + m[13];
    s->Z = v[2] * m[10] + m[14];
    s->W = s->oow = 1.0f;

    /* Clip codes */
    f  = (((int)(0x3f800000-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
      ((GL_UINTCAST(s->Z) >> 31)+1);
             
    f <<= 2;
    f += (((int)(0x3f800000-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
      ((GL_UINTCAST(s->Y) >> 31)+1);
    
    f <<= 2;
    f += (((int)(0x3f800000-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
      ((GL_UINTCAST(s->X) >> 31)+1);

    /* Save away clip code */
    s->flags      = f;

    /* Copy colors, scale and clamp to 0..255 */
    s->r   = gc->state.current.color.r;
    s->g   = gc->state.current.color.g;
    s->b   = gc->state.current.color.b;
    s->a   = gc->state.current.color.a;

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[0].x) + gc->primState.tc[0].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[0].y) + gc->primState.tc[0].tBias;
    s->t0_oow = gc->state.current.texture[0].w;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;
    savC = ps->vC;

    /* Update new state */
    ps->vA     = (IVert *) (((char *)ps->vA) + _VSTEP_A[ps->vState >> 2]);
    ps->vB     = ps->vC;
    ps->vC     = (IVert *) (((char *)ps->vC) + _VSTEP_C[ps->vState >> 2]);
    vdraw      = _VDRAW  [ps->vState >> 2];
    ps->vState = _VNEXT  [ps->vState >> 2];

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * vp->xScale + vp->xCenter;
        s->y   = s->Y   * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * vp->zScale + vp->zCenter;
	s->oow = s->t0_oow;
    }

    if (vdraw) {
       if (vdraw & 2) { IVert *t = savA; savA = savB; savB = t; /* swap */}

       if (!(f | savA->flags | savB->flags)) {
          grDrawTriangle((GrVertex*) savA, (GrVertex*) savB, (GrVertex*) savC);
       } else {
          __glSSTClipAndDraw(SHIFT_CC_BEFORE, savA, savB, savC);
       }
    }
}

void APIENTRY __glsstim_Vertex3fv_0(const GLfloat *v) {
    __GL_SETUP();
    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                                  // clip flags
    IVert       *s = ps->vC;                       // destination
    IVert       *savA, *savB, *savC;

    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12]; 
    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13];
    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15];

    if ( ((int)GL_UINTCAST( s->W )) <= 0 ) {
        s->oow = 1.0f;
        f = CC_BEFORE; 
    } else {
        s->oow = 1.0f / s->W;
        
        /* 
         * Everything below here till projection ought to be free because of
         * the oow computation above. It should use only integer arithmetic!
         */

        /* Clip codes */
        f  = (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Z) >> 31)+1);
             
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Y) >> 31)+1);
    
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->X) >> 31)+1);
    }

    /* Save away clip code */
    s->flags      = f;

    /* Copy colors, scale and clamp to 0..255 */
    s->r   = gc->state.current.color.r;
    s->g   = gc->state.current.color.g;
    s->b   = gc->state.current.color.b;
    s->a   = gc->state.current.color.a;

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[0].x) + gc->primState.tc[0].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[0].y) + gc->primState.tc[0].tBias;
    s->t0_oow = gc->state.current.texture[0].w;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;
    savC = ps->vC;

    /* Update new state */
    ps->vA     = (IVert *) (((char *)ps->vA) + _VSTEP_A[ps->vState >> 2]);
    ps->vB     = ps->vC;
    ps->vC     = (IVert *) (((char *)ps->vC) + _VSTEP_C[ps->vState >> 2]);
    vdraw      = _VDRAW  [ps->vState >> 2];
    ps->vState = _VNEXT  [ps->vState >> 2];

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * s->oow * vp->xScale + vp->xCenter;
        s->y   = s->Y   * s->oow * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * s->oow * vp->zScale + vp->zCenter;
        s->t0_sow = s->t0_sow * s->oow;
        s->t0_tow = s->t0_tow * s->oow;
	s->t0_oow = s->t0_oow * s->oow;
	s->oow = s->t0_oow;
    }

    if (vdraw) {
       if (vdraw & 2) { IVert *t = savA; savA = savB; savB = t; /* swap */}

       if (!(f | savA->flags | savB->flags)) {
          grDrawTriangle((GrVertex*) savA, (GrVertex*) savB, (GrVertex*) savC);
       } else {
          __glSSTClipAndDraw(SHIFT_CC_BEFORE, savA, savB, savC);
       }
    }
}

void APIENTRY __glsstim_Vertex4fv_0(const GLfloat *v) {
    __GL_SETUP();
    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                                  // clip flags
    IVert       *s = ps->vC;                       // destination
    IVert       *savA, *savB, *savC;

    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12] * v[3]; 
    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13] * v[3];
    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];

    if ( ((int)GL_UINTCAST( s->W )) <= 0 ) {
        s->oow = 1.0f;
        f = CC_BEFORE; 
    } else {
        s->oow = 1.0f / s->W;
        
        /* 
         * Everything below here till projection ought to be free because of
         * the oow computation above. It should use only integer arithmetic!
         */

        /* Clip codes */
        f  = (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Z) >> 31)+1);
             
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Y) >> 31)+1);
    
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->X) >> 31)+1);
    }

    /* Save away clip code */
    s->flags      = f;

    /* Copy colors, scale and clamp to 0..255 */
    s->r   = gc->state.current.color.r;
    s->g   = gc->state.current.color.g;
    s->b   = gc->state.current.color.b;
    s->a   = gc->state.current.color.a;

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[0].x) + gc->primState.tc[0].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[0].y) + gc->primState.tc[0].tBias;
    s->t0_oow = gc->state.current.texture[0].w;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;
    savC = ps->vC;

    /* Update new state */
    ps->vA     = (IVert *) (((char *)ps->vA) + _VSTEP_A[ps->vState >> 2]);
    ps->vB     = ps->vC;
    ps->vC     = (IVert *) (((char *)ps->vC) + _VSTEP_C[ps->vState >> 2]);
    vdraw      = _VDRAW  [ps->vState >> 2];
    ps->vState = _VNEXT  [ps->vState >> 2];

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * s->oow * vp->xScale + vp->xCenter;
        s->y   = s->Y   * s->oow * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * s->oow * vp->zScale + vp->zCenter;
        s->t0_sow = s->t0_sow * s->oow;
        s->t0_tow = s->t0_tow * s->oow;
	s->t0_oow = s->t0_oow * s->oow;
	s->oow = s->t0_oow;
    }

    if (vdraw) {
       if (vdraw & 2) { IVert *t = savA; savA = savB; savB = t; /* swap */}

       if (!(f | savA->flags | savB->flags)) {
          grDrawTriangle((GrVertex*) savA, (GrVertex*) savB, (GrVertex*) savC);
       } else {
          __glSSTClipAndDraw(SHIFT_CC_BEFORE, savA, savB, savC);
       }
    }
}

void APIENTRY __glsstim_Vertex2fv_1(const GLfloat *v) {
    __GL_SETUP();
    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                                  // clip flags
    IVert       *s = ps->vC;                       // destination
    IVert       *savA, *savB, *savC;

    s->X = m[0] * v[0] + m[4] * v[1] + m[12]; 
    s->Y = m[1] * v[0] + m[5] * v[1] + m[13];
    s->Z = m[2] * v[0] + m[6] * v[1] + m[14];
    s->W = m[3] * v[0] + m[7] * v[1] + m[15];

    if ( ((int)GL_UINTCAST( s->W )) <= 0 ) {
        s->oow = 1.0f;
        f = CC_BEFORE; 
    } else {
        s->oow = 1.0f / s->W;
        
        /* 
         * Everything below here till projection ought to be free because of
         * the oow computation above. It should use only integer arithmetic!
         */

        /* Clip codes */
        f  = (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Z) >> 31)+1);
             
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Y) >> 31)+1);
    
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->X) >> 31)+1);
    }

    /* Save away clip code */
    s->flags      = f;

    /* Copy colors, scale and clamp to 0..255 */
    s->r   = gc->state.current.color.r;
    s->g   = gc->state.current.color.g;
    s->b   = gc->state.current.color.b;
    s->a   = gc->state.current.color.a;

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[1].x) + gc->primState.tc[1].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[1].y) + gc->primState.tc[1].tBias;
    s->t0_oow = gc->state.current.texture[1].w;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;
    savC = ps->vC;

    /* Update new state */
    ps->vA     = (IVert *) (((char *)ps->vA) + _VSTEP_A[ps->vState >> 2]);
    ps->vB     = ps->vC;
    ps->vC     = (IVert *) (((char *)ps->vC) + _VSTEP_C[ps->vState >> 2]);
    vdraw      = _VDRAW  [ps->vState >> 2];
    ps->vState = _VNEXT  [ps->vState >> 2];

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * s->oow * vp->xScale + vp->xCenter;
        s->y   = s->Y   * s->oow * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * s->oow * vp->zScale + vp->zCenter;
        s->t0_sow = s->t0_sow * s->oow;
        s->t0_tow = s->t0_tow * s->oow;
	s->t0_oow = s->t0_oow * s->oow;
	s->oow = s->t0_oow;
    }

    if (vdraw) {
       if (vdraw & 2) { IVert *t = savA; savA = savB; savB = t; /* swap */}

       if (!(f | savA->flags | savB->flags)) {
          grDrawTriangle((GrVertex*) savA, (GrVertex*) savB, (GrVertex*) savC);
       } else {
          __glSSTClipAndDraw(SHIFT_CC_BEFORE, savA, savB, savC);
       }
    }
}

void APIENTRY __glsstim_Vertex3fv_1(const GLfloat *v) {
    __GL_SETUP();
    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                                  // clip flags
    IVert       *s = ps->vC;                       // destination
    IVert       *savA, *savB, *savC;

    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12]; 
    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13];
    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15];

    if ( ((int)GL_UINTCAST( s->W )) <= 0 ) {
        s->oow = 1.0f;
        f = CC_BEFORE; 
    } else {
        s->oow = 1.0f / s->W;
        
        /* 
         * Everything below here till projection ought to be free because of
         * the oow computation above. It should use only integer arithmetic!
         */

        /* Clip codes */
        f  = (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Z) >> 31)+1);
             
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Y) >> 31)+1);
    
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->X) >> 31)+1);
    }

    /* Save away clip code */
    s->flags      = f;

    /* Copy colors, scale and clamp to 0..255 */
    s->r   = gc->state.current.color.r;
    s->g   = gc->state.current.color.g;
    s->b   = gc->state.current.color.b;
    s->a   = gc->state.current.color.a;

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[1].x) + gc->primState.tc[1].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[1].y) + gc->primState.tc[1].tBias;
    s->t0_oow = gc->state.current.texture[1].w;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;
    savC = ps->vC;

    /* Update new state */
    ps->vA     = (IVert *) (((char *)ps->vA) + _VSTEP_A[ps->vState >> 2]);
    ps->vB     = ps->vC;
    ps->vC     = (IVert *) (((char *)ps->vC) + _VSTEP_C[ps->vState >> 2]);
    vdraw      = _VDRAW  [ps->vState >> 2];
    ps->vState = _VNEXT  [ps->vState >> 2];

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * s->oow * vp->xScale + vp->xCenter;
        s->y   = s->Y   * s->oow * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * s->oow * vp->zScale + vp->zCenter;
        s->t0_sow = s->t0_sow * s->oow;
        s->t0_tow = s->t0_tow * s->oow;
	s->t0_oow = s->t0_oow * s->oow;
	s->oow = s->t0_oow;
    }

    if (vdraw) {
       if (vdraw & 2) { IVert *t = savA; savA = savB; savB = t; /* swap */}

       if (!(f | savA->flags | savB->flags)) {
          grDrawTriangle((GrVertex*) savA, (GrVertex*) savB, (GrVertex*) savC);
       } else {
          __glSSTClipAndDraw(SHIFT_CC_BEFORE, savA, savB, savC);
       }
    }
}

void APIENTRY __glsstim_Vertex4fv_1(const GLfloat *v) {
    __GL_SETUP();
    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                                  // clip flags
    IVert       *s = ps->vC;                       // destination
    IVert       *savA, *savB, *savC;

    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12] * v[3]; 
    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13] * v[3];
    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];

    if ( ((int)GL_UINTCAST( s->W )) <= 0 ) {
        s->oow = 1.0f;
        f = CC_BEFORE; 
    } else {
        s->oow = 1.0f / s->W;
        
        /* 
         * Everything below here till projection ought to be free because of
         * the oow computation above. It should use only integer arithmetic!
         */

        /* Clip codes */
        f  = (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Z) >> 31)+1);
             
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Y) >> 31)+1);
    
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->X) >> 31)+1);
    }

    /* Save away clip code */
    s->flags      = f;

    /* Copy colors, scale and clamp to 0..255 */
    s->r   = gc->state.current.color.r;
    s->g   = gc->state.current.color.g;
    s->b   = gc->state.current.color.b;
    s->a   = gc->state.current.color.a;

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[1].x) + gc->primState.tc[1].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[1].y) + gc->primState.tc[1].tBias;
    s->t0_oow = gc->state.current.texture[1].w;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;
    savC = ps->vC;

    /* Update new state */
    ps->vA     = (IVert *) (((char *)ps->vA) + _VSTEP_A[ps->vState >> 2]);
    ps->vB     = ps->vC;
    ps->vC     = (IVert *) (((char *)ps->vC) + _VSTEP_C[ps->vState >> 2]);
    vdraw      = _VDRAW  [ps->vState >> 2];
    ps->vState = _VNEXT  [ps->vState >> 2];

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * s->oow * vp->xScale + vp->xCenter;
        s->y   = s->Y   * s->oow * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * s->oow * vp->zScale + vp->zCenter;
        s->t0_sow = s->t0_sow * s->oow;
        s->t0_tow = s->t0_tow * s->oow;
	s->t0_oow = s->t0_oow * s->oow;
	s->oow = s->t0_oow;
    }

    if (vdraw) {
       if (vdraw & 2) { IVert *t = savA; savA = savB; savB = t; /* swap */}

       if (!(f | savA->flags | savB->flags)) {
          grDrawTriangle((GrVertex*) savA, (GrVertex*) savB, (GrVertex*) savC);
       } else {
          __glSSTClipAndDraw(SHIFT_CC_BEFORE, savA, savB, savC);
       }
    }
}


void APIENTRY __glsstim_Vertex2fv_B(const GLfloat *v) {
    __GL_SETUP();
    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                                  // clip flags
    IVert       *s = ps->vC;                       // destination
    IVert       *savA, *savB, *savC;

    s->X = m[0] * v[0] + m[4] * v[1] + m[12]; 
    s->Y = m[1] * v[0] + m[5] * v[1] + m[13];
    s->Z = m[2] * v[0] + m[6] * v[1] + m[14];
    s->W = m[3] * v[0] + m[7] * v[1] + m[15];

    if ( ((int)GL_UINTCAST( s->W )) <= 0 ) {
        s->oow = 1.0f;
        f = CC_BEFORE; 
    } else {
        s->oow = 1.0f / s->W;
        
        /* 
         * Everything below here till projection ought to be free because of
         * the oow computation above. It should use only integer arithmetic!
         */

        /* Clip codes */
        f  = (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Z) >> 31)+1);
             
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Y) >> 31)+1);
    
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->X) >> 31)+1);
    }

    /* Save away clip code */
    s->flags      = f;

    /* Copy colors, scale and clamp to 0..255 */
#if 0
    /* taco - don't bother since no it color */
    s->r   = gc->state.current.color.r;
    s->g   = gc->state.current.color.g;
    s->b   = gc->state.current.color.b;
    s->a   = gc->state.current.color.a;
#endif

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[1].x) + gc->primState.tc[1].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[1].y) + gc->primState.tc[1].tBias;
    GL_UINTCAST(s->t1_sow) = GL_UINTCAST(gc->state.current.texture[0].x) + gc->primState.tc[0].sBias;
    GL_UINTCAST(s->t1_tow) = GL_UINTCAST(gc->state.current.texture[0].y) + gc->primState.tc[0].tBias;
    s->t0_oow = gc->state.current.texture[1].w;
    s->t1_oow = gc->state.current.texture[0].w;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;
    savC = ps->vC;

    /* Update new state */
    ps->vA     = (IVert *) (((char *)ps->vA) + _VSTEP_A[ps->vState >> 2]);
    ps->vB     = ps->vC;
    ps->vC     = (IVert *) (((char *)ps->vC) + _VSTEP_C[ps->vState >> 2]);
    vdraw      = _VDRAW  [ps->vState >> 2];
    ps->vState = _VNEXT  [ps->vState >> 2];

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * s->oow * vp->xScale + vp->xCenter;
        s->y   = s->Y   * s->oow * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * s->oow * vp->zScale + vp->zCenter;
        s->t0_sow = s->t0_sow * s->oow;
        s->t0_tow = s->t0_tow * s->oow;
        s->t1_sow = s->t1_sow * s->oow;
        s->t1_tow = s->t1_tow * s->oow;
	s->t0_oow = s->t0_oow * s->oow;
	s->t1_oow = s->t1_oow * s->oow;
	s->oow = s->t0_oow;
    }

    if (vdraw) {
       if (vdraw & 2) { IVert *t = savA; savA = savB; savB = t; /* swap */}

       if (!(f | savA->flags | savB->flags)) {
          grDrawTriangle((GrVertex*) savA, (GrVertex*) savB, (GrVertex*) savC);
       } else {
          __glSSTClipAndDraw_B(SHIFT_CC_BEFORE, savA, savB, savC);
       }
    }
}

void APIENTRY __glsstim_Vertex3fv_B(const GLfloat *v) {
    __GL_SETUP();
    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                                  // clip flags
    IVert       *s = ps->vC;                       // destination
    IVert       *savA, *savB, *savC;

    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12]; 
    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13];
    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15];

    if ( ((int)GL_UINTCAST( s->W )) <= 0 ) {
        s->oow = 1.0f;
        f = CC_BEFORE; 
    } else {
        s->oow = 1.0f / s->W;
        
        /* 
         * Everything below here till projection ought to be free because of
         * the oow computation above. It should use only integer arithmetic!
         */

        /* Clip codes */
        f  = (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Z) >> 31)+1);
             
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Y) >> 31)+1);
    
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->X) >> 31)+1);
    }

    /* Save away clip code */
    s->flags      = f;

    /* Copy colors, scale and clamp to 0..255 */
#if 0
    /* taco - don't bother since no it color */
    s->r   = gc->state.current.color.r;
    s->g   = gc->state.current.color.g;
    s->b   = gc->state.current.color.b;
    s->a   = gc->state.current.color.a;
#endif

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[1].x) + gc->primState.tc[1].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[1].y) + gc->primState.tc[1].tBias;
    GL_UINTCAST(s->t1_sow) = GL_UINTCAST(gc->state.current.texture[0].x) + gc->primState.tc[0].sBias;
    GL_UINTCAST(s->t1_tow) = GL_UINTCAST(gc->state.current.texture[0].y) + gc->primState.tc[0].tBias;
    s->t0_oow = gc->state.current.texture[1].w;
    s->t1_oow = gc->state.current.texture[0].w;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;
    savC = ps->vC;

    /* Update new state */
    ps->vA     = (IVert *) (((char *)ps->vA) + _VSTEP_A[ps->vState >> 2]);
    ps->vB     = ps->vC;
    ps->vC     = (IVert *) (((char *)ps->vC) + _VSTEP_C[ps->vState >> 2]);
    vdraw      = _VDRAW  [ps->vState >> 2];
    ps->vState = _VNEXT  [ps->vState >> 2];

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * s->oow * vp->xScale + vp->xCenter;
        s->y   = s->Y   * s->oow * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * s->oow * vp->zScale + vp->zCenter;/* XXXTACO - ???? */
        s->t0_sow = s->t0_sow * s->oow;
        s->t0_tow = s->t0_tow * s->oow;
        s->t1_sow = s->t1_sow * s->oow;
        s->t1_tow = s->t1_tow * s->oow;
	s->t0_oow = s->t0_oow * s->oow;
	s->t1_oow = s->t1_oow * s->oow;
	s->oow = s->t0_oow;
    }

    if (vdraw) {
       if (vdraw & 2) { IVert *t = savA; savA = savB; savB = t; /* swap */}

       if (!(f | savA->flags | savB->flags)) {
          grDrawTriangle((GrVertex*) savA, (GrVertex*) savB, (GrVertex*) savC);
       } else {
          __glSSTClipAndDraw_B(SHIFT_CC_BEFORE, savA, savB, savC);
       }
    }
}

void APIENTRY __glsstim_Vertex4fv_B(const GLfloat *v) {
    __GL_SETUP();
    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                                  // clip flags
    IVert       *s = ps->vC;                       // destination
    IVert       *savA, *savB, *savC;

    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12] * v[3]; 
    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13] * v[3];
    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];

    if ( ((int)GL_UINTCAST( s->W )) <= 0 ) {
        s->oow = 1.0f;
        f = CC_BEFORE; 
    } else {
        s->oow = 1.0f / s->W;
        
        /* 
         * Everything below here till projection ought to be free because of
         * the oow computation above. It should use only integer arithmetic!
         */

        /* Clip codes */
        f  = (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Z) >> 31)+1);
             
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Y) >> 31)+1);
    
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->X) >> 31)+1);
    }

    /* Save away clip code */
    s->flags      = f;

    /* Copy colors, scale and clamp to 0..255 */
#if 0
    /* taco - don't bother since no it color */
    s->r   = gc->state.current.color.r;
    s->g   = gc->state.current.color.g;
    s->b   = gc->state.current.color.b;
    s->a   = gc->state.current.color.a;
#endif

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[1].x) + gc->primState.tc[1].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[1].y) + gc->primState.tc[1].tBias;
    GL_UINTCAST(s->t1_sow) = GL_UINTCAST(gc->state.current.texture[0].x) + gc->primState.tc[0].sBias;
    GL_UINTCAST(s->t1_tow) = GL_UINTCAST(gc->state.current.texture[0].y) + gc->primState.tc[0].tBias;
    s->t0_oow = gc->state.current.texture[1].w;
    s->t1_oow = gc->state.current.texture[0].w;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;
    savC = ps->vC;

    /* Update new state */
    ps->vA     = (IVert *) (((char *)ps->vA) + _VSTEP_A[ps->vState >> 2]);
    ps->vB     = ps->vC;
    ps->vC     = (IVert *) (((char *)ps->vC) + _VSTEP_C[ps->vState >> 2]);
    vdraw      = _VDRAW  [ps->vState >> 2];
    ps->vState = _VNEXT  [ps->vState >> 2];

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * s->oow * vp->xScale + vp->xCenter;
        s->y   = s->Y   * s->oow * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * s->oow * vp->zScale + vp->zCenter; /* XXXTACO - ??? */
        s->t0_sow = s->t0_sow * s->oow;
        s->t0_tow = s->t0_tow * s->oow;
        s->t1_sow = s->t1_sow * s->oow;
        s->t1_tow = s->t1_tow * s->oow;
	s->t0_oow = s->t0_oow * s->oow;
	s->t1_oow = s->t1_oow * s->oow;
	s->oow = s->t0_oow;
    }

    if (vdraw) {
       if (vdraw & 2) { IVert *t = savA; savA = savB; savB = t; /* swap */}

       if (!(f | savA->flags | savB->flags)) {
          grDrawTriangle((GrVertex*) savA, (GrVertex*) savB, (GrVertex*) savC);
       } else {
          __glSSTClipAndDraw_B(SHIFT_CC_BEFORE, savA, savB, savC);
       }
    }
}


static void 
__glSSTIntersect(  IVert *in,  IVert *out,  IVert *mid, unsigned plane) 
{
    __GL_SETUP();
    float din, dout, t, omt;
    float diff, iw, ow;
    float *vi = &in->X;
    float *vo = &out->X;

    /* Add for odd planes, subtract for even planes */
    if (plane & 1) {
        din  = vi[3] + vi[plane>>1];        // in->W  + in->Z;
        dout = vo[3] + vo[plane>>1];        // out->W + out->Z;
    } else {
        din  = vi[3] - vi[plane>>1];        // in->W  - in->Z;
        dout = vo[3] - vo[plane>>1];        // out->W - out->Z;
    }
    t = din / ( din - dout );
    omt = 1.0f - t; 
    mid->z = t;

    iw = (in ->flags) ? 1.0f : in ->W;
    ow = (out->flags) ? 1.0f : out->W;
    mid->X   = t * out->X   + omt * in->X;
    mid->Y   = t * out->Y   + omt * in->Y;
    mid->Z   = t * out->Z   + omt * in->Z;
    mid->W   = t * out->W   + omt * in->W;
    mid->t0_sow = t * out->t0_sow * ow + omt * in->t0_sow * iw; // * in->W;
    mid->t0_tow = t * out->t0_tow * ow + omt * in->t0_tow * iw; // * in->W;
    mid->t0_oow = t * out->t0_oow * ow + omt * in->t0_oow * iw; // * in->W;

    if (gc->state.light.shadingModel == GL_SMOOTH) {
        mid->r   = t * out->r   + omt * in->r;
        mid->g   = t * out->g   + omt * in->g;
        mid->b   = t * out->b   + omt * in->b;
        mid->a   = t * out->a   + omt * in->a;
    } else {
        mid->r   = in->r;
        mid->g   = in->g;
        mid->b   = in->b;
        mid->a   = in->a;

    }

    mid->flags = 0;

    /* Recompute Clip Codes - fallthrough is intentional */
    /* this is necessary so that numerical imprecision doesn't */
    /* lead to re-clip codes that will break clipper */
    switch( plane ) {
    case SHIFT_CC_BEFORE:
        diff = mid->W - mid->Z;
        mid->flags |= ((GL_UINTCAST(diff))>>31)<<SHIFT_CC_BEHIND;
    case SHIFT_CC_BEHIND:
        diff = mid->W + mid->Y;
        mid->flags |= ((GL_UINTCAST(diff))>>31)<<SHIFT_CC_BELOW;
    case SHIFT_CC_BELOW:
        diff = mid->W - mid->Y;
        mid->flags |= ((GL_UINTCAST(diff))>>31)<<SHIFT_CC_ABOVE;
    case SHIFT_CC_ABOVE:
        diff = mid->W + mid->X;
        mid->flags |= ((GL_UINTCAST(diff))>>31)<<SHIFT_CC_LEFT;
    case SHIFT_CC_LEFT:
        diff = mid->W - mid->X;
        mid->flags |= ((GL_UINTCAST(diff))>>31)<<SHIFT_CC_RIGHT;
    }

    /* if new vertex is in, then it will definitely be 
       drawn, time to project */
    if ( !mid->flags ) {
        __GLviewport   *vp     = &gc->state.viewport;
        
        mid->oow = 1.0f / mid->W;
        mid->x   = mid->X   * mid->oow * vp->xScale + vp->xCenter;
        mid->y   = mid->Y   * mid->oow * vp->yScale + vp->yCenter;
        mid->ooz = mid->Z   * mid->oow * vp->zScale + vp->zCenter;
        mid->t0_sow = mid->t0_sow * mid->oow;
        mid->t0_tow = mid->t0_tow * mid->oow;
	mid->t0_oow = mid->t0_oow * mid->oow;
        mid->oow = mid->t0_oow;
    }
    else {
        mid->oow = 1.0f;
    }
}

static void 
__glSSTIntersect_B(  IVert *in,  IVert *out,  IVert *mid, unsigned plane) 
{
    __GL_SETUP();
    float din, dout, t, omt;
    float diff, iw, ow;
    float *vi = &in->X;
    float *vo = &out->X;

    /* Add for odd planes, subtract for even planes */
    if (plane & 1) {
        din  = vi[3] + vi[plane>>1];        // in->W  + in->Z;
        dout = vo[3] + vo[plane>>1];        // out->W + out->Z;
    } else {
        din  = vi[3] - vi[plane>>1];        // in->W  - in->Z;
        dout = vo[3] - vo[plane>>1];        // out->W - out->Z;
    }
    t = din / ( din - dout );
    omt = 1.0f - t; 
    mid->z = t;

    iw = (in ->flags) ? 1.0f : in ->W;
    ow = (out->flags) ? 1.0f : out->W;
    mid->X   = t * out->X   + omt * in->X;
    mid->Y   = t * out->Y   + omt * in->Y;
    mid->Z   = t * out->Z   + omt * in->Z;
    mid->W   = t * out->W   + omt * in->W;
    mid->t0_sow = t * out->t0_sow * ow + omt * in->t0_sow * iw; // * in->W;
    mid->t0_tow = t * out->t0_tow * ow + omt * in->t0_tow * iw; // * in->W;
    mid->t0_oow = t * out->t0_oow * ow + omt * in->t0_oow * iw; // * in->W;
    mid->t1_sow = t * out->t1_sow * ow + omt * in->t1_sow * iw; // * in->W;
    mid->t1_tow = t * out->t1_tow * ow + omt * in->t1_tow * iw; // * in->W;
    mid->t1_oow = t * out->t1_oow * ow + omt * in->t1_oow * iw; // * in->W;

    mid->flags = 0;

    /* Recompute Clip Codes - fallthrough is intentional */
    /* this is necessary so that numerical imprecision doesn't */
    /* lead to re-clip codes that will break clipper */
    switch( plane ) {
    case SHIFT_CC_BEFORE:
        diff = mid->W - mid->Z;
        mid->flags |= ((GL_UINTCAST(diff))>>31)<<SHIFT_CC_BEHIND;
    case SHIFT_CC_BEHIND:
        diff = mid->W + mid->Y;
        mid->flags |= ((GL_UINTCAST(diff))>>31)<<SHIFT_CC_BELOW;
    case SHIFT_CC_BELOW:
        diff = mid->W - mid->Y;
        mid->flags |= ((GL_UINTCAST(diff))>>31)<<SHIFT_CC_ABOVE;
    case SHIFT_CC_ABOVE:
        diff = mid->W + mid->X;
        mid->flags |= ((GL_UINTCAST(diff))>>31)<<SHIFT_CC_LEFT;
    case SHIFT_CC_LEFT:
        diff = mid->W - mid->X;
        mid->flags |= ((GL_UINTCAST(diff))>>31)<<SHIFT_CC_RIGHT;
    }

    /* if new vertex is in, then it will definitely be 
       drawn, time to project */
    if ( !mid->flags ) {
        __GLviewport   *vp     = &gc->state.viewport;
        
        mid->oow = 1.0f / mid->W;
        mid->x   = mid->X   * mid->oow * vp->xScale + vp->xCenter;
        mid->y   = mid->Y   * mid->oow * vp->yScale + vp->yCenter;
        mid->ooz = mid->Z   * mid->oow * vp->zScale + vp->zCenter;
        mid->t0_sow = mid->t0_sow * mid->oow;
        mid->t0_tow = mid->t0_tow * mid->oow;
	mid->t0_oow = mid->t0_oow * mid->oow;
        mid->t1_sow = mid->t1_sow * mid->oow;
        mid->t1_tow = mid->t1_tow * mid->oow;
	mid->t1_oow = mid->t1_oow * mid->oow;
        mid->oow = mid->t0_oow;
    }
    else {
        mid->oow = 1.0f;
    }
}

#else // __GL_USE_INTEL_ASM

static unsigned _ccsign[] = {0x15, 0x16, 0x19, 0x1a, 0x25, 0x26, 0x29, 0x2a};
static unsigned _ccmask[] = {0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f};

#define flds    fld DWORD PTR
#define fmuls   fmul DWORD PTR
#define fdivs   fdiv DWORD PTR
#define fstsp   fstp DWORD PTR
#define fadds   fadd DWORD PTR
#define fsubs   fsub DWORD PTR
#define fdivrs  fdivr DWORD PTR
#define inW 12
#define inZ 8
#define inY 4
#define inX 0
#define m00 0
#define m01 4
#define m02 8
#define m03 12
#define m04 16
#define m05 20
#define m06 24
#define m07 28
#define m08 32
#define m09 36
#define m10 40
#define m11 44
#define m12 48
#define m13 52
#define m14 56
#define m15 60
#define ps __GLcontext.primState

#define _gc_ edi
#define v eax
#define vo ebp
#define mat ecx
#define code esi
#define iw   edx
#define ix   eax
#define iy   ebx
#define iz   ecx
#define vp ecx

#define _v$  20

__declspec(naked)
void APIENTRY __glsstim_Vertex2fv_0(const GLfloat *v) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     _gc_,  DWORD PTR __gl_contextArea
        mov     mat,  DWORD PTR [_gc_]__GLcontext.transform.modelView
        add     mat, 168
        mov     v,  _v$[esp]
        mov     vo, ps.vC[_gc_]

//    s->X = m[0] * v[0] + m[4] * v[1] + m[12]; 
//    s->Y = m[1] * v[0] + m[5] * v[1] + m[13];
//    s->Z = m[2] * v[0] + m[6] * v[1] + m[14];
//    s->W = m[3] * v[0] + m[7] * v[1] + m[15];
        flds    m15[mat]
        flds    m14[mat]
        flds    m13[mat]
        flds    m12[mat]

        flds    inY[v]
        fmuls   m07[mat]
        flds    inY[v]
        fmuls   m06[mat]
        flds    inY[v]
        fmuls   m05[mat]
        flds    inY[v]
        fmuls   m04[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inX[v]
        fmuls   m03[mat]
        flds    inX[v]
        fmuls   m02[mat]
        flds    inX[v]
        fmuls   m01[mat]
        flds    inX[v]
        fmuls   m00[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        fxch    st(3)
        fstsp   [vo]IVert.W
        fstsp   [vo]IVert.Y
        fstsp   [vo]IVert.Z
        fstsp   [vo]IVert.X

        flds    [_gc_]__GLcontext.constants.one
           
        mov     code, CC_BEFORE
        mov     iw, IVert.W[vo]

        add     iw, iw
        jc      AfterClip

        // W is positive

        fdivs   IVert.W[vo]

        mov     iz, IVert.Z[vo]
        mov     iy, IVert.Y[vo]

        mov     ix, IVert.X[vo]
        add     iz, iz

        adc     code, code
        add     iy, iy

        adc     code, code
        add     ix, ix

        adc     code, code
        cmp     iw, iz

        adc     iz, iz
        cmp     iw, iy

        adc     iz, iz
        cmp     iw, ix

        adc     iz, iz
        mov     iy, code

        and     iy, 7
        and     iz, 7

        mov     code, 0
        jz      AfterClip

        mov     iy, _ccsign[iy*4]
        mov     code, _ccmask[iz*4]

        and     code, iy
        nop
AfterClip:
        mov     eax, __GLcontext.state.current.color.r[_gc_]
        mov     ebx, __GLcontext.state.current.color.g[_gc_]
        mov     ecx, __GLcontext.state.current.color.b[_gc_]
        mov     edx, __GLcontext.state.current.color.a[_gc_]
        mov     IVert.r[vo], eax
        mov     IVert.g[vo], ebx
        mov     IVert.b[vo], ecx
        mov     IVert.a[vo], edx

        mov     eax, ps.tc[0].sBias[_gc_]
        mov     ebx, ps.tc[0].tBias[_gc_]
        mov     ecx, __GLcontext.state.current.texture[0].w[_gc_]
        add     eax, __GLcontext.state.current.texture[0].x[_gc_]
        add     ebx, __GLcontext.state.current.texture[0].y[_gc_]
	mov     IVert.t0_oow[vo], ecx
        mov     IVert.t0_sow[vo], eax
        mov     IVert.t0_tow[vo], ebx

        mov     IVert.flags[vo], code
        mov     ecx, ps.vState[_gc_]

        mov     ebx, ps.vB[_gc_]
        mov     eax, ps.vA[_gc_]

        mov     [esp-4], vo
        mov     [esp-8], ebx

        mov     [esp-12], eax
        mov     edx, _VSTEP_C[ecx]

        mov     ps.vB[_gc_], vo
        add     edx, vo

        mov     ps.vC[_gc_], edx
        mov     edx, _VSTEP_A[ecx]

        add     eax, edx
        mov     edx, _VDRAW[ecx]

        mov     ps.vA[_gc_], eax
        mov     ecx, _VNEXT[ecx]

        mov     ps.vState[_gc_], ecx
        lea     vp, __GLcontext.state.viewport[_gc_]

        test    code, code
        jnz     AfterViewport

        flds    IVert.t0_sow[vo]
        fmul    st(0), st(1)
        flds    IVert.t0_tow[vo]
        fmul    st(0), st(2)
        flds    IVert.t0_oow[vo]
        fmul    st(0), st(3)
        flds    IVert.Z[vo]
        fmul    st(0), st(4)
        flds    IVert.Y[vo]
        fmul    st(0), st(5)
        flds    IVert.X[vo]
        fmul    st(0), st(6)

        fxch    st(5)
        fstsp   IVert.t0_sow[vo]
        fxch    st(3)
        fstsp   IVert.t0_tow[vo]
        fxch    st(1)
        fstsp   IVert.t0_oow[vo]

        flds    __GLviewport.zScale[vp]
        fmulp   st(1), st
        flds    __GLviewport.yScale[vp]
        fmulp   st(2), st
        flds    __GLviewport.xScale[vp]
        fmulp   st(3), st

        fadds   __GLviewport.zCenter[vp]
        fxch    st(2)
        fadds   __GLviewport.xCenter[vp]
        fxch    st(1)
        fadds   __GLviewport.yCenter[vp]
        fxch    st(2)

        fstsp   IVert.ooz[vo]
        fstsp   IVert.x[vo]
        fstsp   IVert.y[vo]
        fstp    st(0)
        flds    IVert.t0_oow[vo]

AfterViewport:
        fstsp   IVert.oow[vo]

        test    edx, edx
        jnz     Draw

        pop     ebx
        pop     esi

        pop     edi
        pop     ebp

        ret     4
Draw:
        mov     eax, [esp -12]
        mov     ebx, [esp - 8]

        test    edx, 2
        jz      noSwap

        mov     [esp-12], ebx
        mov     [esp-8],eax
noSwap:
        mov     eax, IVert.flags[eax]
        add     esp, -12

        or      code, eax
        mov     ebx, IVert.flags[ebx]

        or      code, ebx
        jnz     DrawClipped

        call    grDrawTriangle

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
DrawClipped:
        push    5
        call    __glSSTClipAndDraw
        add     esp, 16
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
    }
}

__declspec(naked)
void APIENTRY __glsstim_Vertex3fv_0_I(const GLfloat *v) {
    __asm {
        mov     mat, 168
        push    ebp

        push    edi
        mov     _gc_,  DWORD PTR __gl_contextArea

	push    esi
        push    ebx

        add     mat,  DWORD PTR [_gc_]__GLcontext.transform.modelView
        mov     v,  _v$[esp]

        mov     vo, ps.vC[_gc_]

//    s->X = v[0] * m[0] + m[12];
//    s->Y = v[1] * m[5] + m[13];
//    s->Z = v[2] * m[10] + m[14];
//    s->W = s->oow = 1.0f;

        flds    m14[mat]		// Z
        flds    m13[mat]		// Z Y
        flds    m12[mat]		// Z Y X

        flds    inZ[v]			// Z Y X z
        fmuls   m10[mat]
        flds    inY[v]			// Z Y X z y
        fmuls   m05[mat]
        flds    inX[v]			// Z Y X z y x
        fmuls   m00[mat]

        flds    [_gc_]__GLcontext.constants.one	// Z Y X z y x 1.0
        fxch    st(3)				// Z Y X 1.0 y x z
        faddp   st(6), st			// Z Y X 1.0 y x
	fxch	st(1)				// Z Y X 1.0 x y
        faddp   st(4), st			// Z Y X 1.0 x
        faddp   st(2), st			// Z Y X 1.0

        fstp    [vo]IVert.W			// Z Y X 1.0
	fxch	st(2)				// X Y Z
        fstsp   [vo]IVert.Z			// X Y
        fstsp   [vo]IVert.Y			// X
        fstsp   [vo]IVert.X			//

        mov	iw, IVert.W[vo]
        mov     iz, IVert.Z[vo]

        mov     iy, IVert.Y[vo]
        mov     ix, IVert.X[vo]

	add	iw, iw
        add     iz, iz

        adc     code, code
        add     iy, iy

        adc     code, code
        add     ix, ix

        adc     code, code
        cmp     iw, iz

        adc     iz, iz
        cmp     iw, iy

        adc     iz, iz
        cmp     iw, ix

        adc     iz, iz
        mov     iy, code

        and     iy, 7
        and     iz, 7

        mov     code, 0
        jz      AfterClip

        mov     iy, _ccsign[iy*4]
        mov     code, _ccmask[iz*4]

        and     code, iy

AfterClip:
        mov     eax, __GLcontext.state.current.color.r[_gc_]
        mov     ebx, __GLcontext.state.current.color.g[_gc_]
        mov     ecx, __GLcontext.state.current.color.b[_gc_]
        mov     edx, __GLcontext.state.current.color.a[_gc_]
        mov     IVert.r[vo], eax
        mov     IVert.g[vo], ebx
        mov     IVert.b[vo], ecx
        mov     IVert.a[vo], edx

        mov     eax, ps.tc[0].sBias[_gc_]
        mov     ebx, ps.tc[0].tBias[_gc_]
        mov     ecx, __GLcontext.state.current.texture[0].w[_gc_]
        add     eax, __GLcontext.state.current.texture[0].x[_gc_]
        add     ebx, __GLcontext.state.current.texture[0].y[_gc_]
	mov     IVert.t0_oow[vo], ecx
        mov     IVert.t0_sow[vo], eax
        mov     IVert.t0_tow[vo], ebx

        mov     IVert.flags[vo], code
        mov     ecx, ps.vState[_gc_]

        mov     ebx, ps.vB[_gc_]
        mov     eax, ps.vA[_gc_]

        mov     [esp-4], vo
        mov     [esp-8], ebx

        mov     [esp-12], eax
        mov     edx, _VSTEP_C[ecx]

        mov     ps.vB[_gc_], vo
        add     edx, vo

        mov     ps.vC[_gc_], edx
        mov     edx, _VSTEP_A[ecx]

        add     eax, edx
        mov     edx, _VDRAW[ecx]

        mov     ps.vA[_gc_], eax
        mov     ecx, _VNEXT[ecx]

        mov     ps.vState[_gc_], ecx
        lea     vp, __GLcontext.state.viewport[_gc_]

        test    code, code
        jnz     AfterViewport

        flds    IVert.t0_oow[vo]		// q
        flds    IVert.Z[vo]			// q Z
        fmuls   __GLviewport.zScale[vp]
        flds    IVert.Y[vo]			// q Z Y
        fmuls   __GLviewport.yScale[vp]
        flds    IVert.X[vo]			// q Z Y X
        fmuls   __GLviewport.xScale[vp]

        fxch    st(2)				// q X Y Z

        fadds   __GLviewport.zCenter[vp]	// q X Y z
        fxch    st(1)				// q X z Y
        fadds   __GLviewport.yCenter[vp]	// q X z y
        fxch    st(2)				// q y z X
        fadds   __GLviewport.xCenter[vp]	// q y z x

        fxch    st(3)				// x y z q
        fstsp   IVert.oow[vo]
        fstsp   IVert.ooz[vo]
        fstsp   IVert.y[vo]
        fstsp   IVert.x[vo]

AfterViewport:

        test    edx, edx
        jnz     Draw

        pop     ebx
        pop     esi

        pop     edi
        pop     ebp

        ret     4
Draw:
        mov     eax, [esp -12]
        mov     ebx, [esp - 8]

        test    edx, 2
        jz      noSwap

        mov     [esp-12], ebx
        mov     [esp-8],eax
noSwap:
        mov     eax, IVert.flags[eax]
        add     esp, -12

        or      code, eax
        mov     ebx, IVert.flags[ebx]

        or      code, ebx
        jnz     DrawClipped

        call    grDrawTriangle

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
DrawClipped:
        push    5
        call    __glSSTClipAndDraw
        add     esp, 16
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
    }
}

__declspec(naked)
void APIENTRY __glsstim_Vertex3fv_0(const GLfloat *v) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     _gc_,  DWORD PTR __gl_contextArea
        mov     mat,  DWORD PTR [_gc_]__GLcontext.transform.modelView
        add     mat, 168
        mov     v,  _v$[esp]
        mov     vo, ps.vC[_gc_]

//    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12]; 
//    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13];
//    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
//    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15];
        flds    m15[mat]
        flds    m14[mat]
        flds    m13[mat]
        flds    m12[mat]

        flds    inZ[v]
        fmuls   m11[mat]
        flds    inZ[v]
        fmuls   m10[mat]
        flds    inZ[v]
        fmuls   m09[mat]
        flds    inZ[v]
        fmuls   m08[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inY[v]
        fmuls   m07[mat]
        flds    inY[v]
        fmuls   m06[mat]
        flds    inY[v]
        fmuls   m05[mat]
        flds    inY[v]
        fmuls   m04[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inX[v]
        fmuls   m03[mat]
        flds    inX[v]
        fmuls   m02[mat]
        flds    inX[v]
        fmuls   m01[mat]
        flds    inX[v]
        fmuls   m00[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        fxch    st(3)
        fstsp   [vo]IVert.W
        fstsp   [vo]IVert.Y
        fstsp   [vo]IVert.Z
        fstsp   [vo]IVert.X

        flds    [_gc_]__GLcontext.constants.one
           
        mov     code, CC_BEFORE
        mov     iw, IVert.W[vo]

        add     iw, iw
        jc      AfterClip

        // W is positive

        fdivs   IVert.W[vo]

        mov     iz, IVert.Z[vo]
        mov     iy, IVert.Y[vo]

        mov     ix, IVert.X[vo]
        add     iz, iz

        adc     code, code
        add     iy, iy

        adc     code, code
        add     ix, ix

        adc     code, code
        cmp     iw, iz

        adc     iz, iz
        cmp     iw, iy

        adc     iz, iz
        cmp     iw, ix

        adc     iz, iz
        mov     iy, code

        and     iy, 7
        and     iz, 7

        mov     code, 0
        jz      AfterClip

        mov     iy, _ccsign[iy*4]
        mov     code, _ccmask[iz*4]

        and     code, iy
        nop
AfterClip:
        mov     eax, __GLcontext.state.current.color.r[_gc_]
        mov     ebx, __GLcontext.state.current.color.g[_gc_]
        mov     ecx, __GLcontext.state.current.color.b[_gc_]
        mov     edx, __GLcontext.state.current.color.a[_gc_]
        mov     IVert.r[vo], eax
        mov     IVert.g[vo], ebx
        mov     IVert.b[vo], ecx
        mov     IVert.a[vo], edx

        mov     eax, ps.tc[0].sBias[_gc_]
        mov     ebx, ps.tc[0].tBias[_gc_]
        mov     ecx, __GLcontext.state.current.texture[0].w[_gc_]
        add     eax, __GLcontext.state.current.texture[0].x[_gc_]
        add     ebx, __GLcontext.state.current.texture[0].y[_gc_]
	mov     IVert.t0_oow[vo], ecx
        mov     IVert.t0_sow[vo], eax
        mov     IVert.t0_tow[vo], ebx

        mov     IVert.flags[vo], code
        mov     ecx, ps.vState[_gc_]

        mov     ebx, ps.vB[_gc_]
        mov     eax, ps.vA[_gc_]

        mov     [esp-4], vo
        mov     [esp-8], ebx

        mov     [esp-12], eax
        mov     edx, _VSTEP_C[ecx]

        mov     ps.vB[_gc_], vo
        add     edx, vo

        mov     ps.vC[_gc_], edx
        mov     edx, _VSTEP_A[ecx]

        add     eax, edx
        mov     edx, _VDRAW[ecx]

        mov     ps.vA[_gc_], eax
        mov     ecx, _VNEXT[ecx]

        mov     ps.vState[_gc_], ecx
        lea     vp, __GLcontext.state.viewport[_gc_]

        test    code, code
        jnz     AfterViewport

        flds    IVert.t0_sow[vo]
        fmul    st(0), st(1)
        flds    IVert.t0_tow[vo]
        fmul    st(0), st(2)
        flds    IVert.t0_oow[vo]
        fmul    st(0), st(3)
        flds    IVert.Z[vo]
        fmul    st(0), st(4)
        flds    IVert.Y[vo]
        fmul    st(0), st(5)
        flds    IVert.X[vo]
        fmul    st(0), st(6)

        fxch    st(5)
        fstsp   IVert.t0_sow[vo]
        fxch    st(3)
        fstsp   IVert.t0_tow[vo]
        fxch    st(1)
        fstsp   IVert.t0_oow[vo]

        flds    __GLviewport.zScale[vp]
        fmulp   st(1), st
        flds    __GLviewport.yScale[vp]
        fmulp   st(2), st
        flds    __GLviewport.xScale[vp]
        fmulp   st(3), st

        fadds   __GLviewport.zCenter[vp]
        fxch    st(2)
        fadds   __GLviewport.xCenter[vp]
        fxch    st(1)
        fadds   __GLviewport.yCenter[vp]
        fxch    st(2)

        fstsp   IVert.ooz[vo]
        fstsp   IVert.x[vo]
        fstsp   IVert.y[vo]
        fstp    st(0)
        flds    IVert.t0_oow[vo]

AfterViewport:
        fstsp   IVert.oow[vo]

        test    edx, edx
        jnz     Draw

        pop     ebx
        pop     esi

        pop     edi
        pop     ebp

        ret     4
Draw:
        mov     eax, [esp -12]
        mov     ebx, [esp - 8]

        test    edx, 2
        jz      noSwap

        mov     [esp-12], ebx
        mov     [esp-8],eax
noSwap:
        mov     eax, IVert.flags[eax]
        add     esp, -12

        or      code, eax
        mov     ebx, IVert.flags[ebx]

        or      code, ebx
        jnz     DrawClipped

        call    grDrawTriangle

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
DrawClipped:
        push    5
        call    __glSSTClipAndDraw
        add     esp, 16
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
    }
}

__declspec(naked)
void APIENTRY __glsstim_Vertex4fv_0(const GLfloat *v) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     _gc_,  DWORD PTR __gl_contextArea
        mov     mat,  DWORD PTR [_gc_]__GLcontext.transform.modelView
        add     mat, 168
        mov     v,  _v$[esp]
        mov     vo, ps.vC[_gc_]

//    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12] * v[3]; 
//    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13] * v[3];
//    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
//    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];
        flds    m15[mat]
        fmuls   inW[v]
        flds    m14[mat]
        fmuls   inW[v]
        flds    m13[mat]
        fmuls   inW[v]
        flds    m12[mat]
        fmuls   inW[v]

        flds    inZ[v]
        fmuls   m11[mat]
        flds    inZ[v]
        fmuls   m10[mat]
        flds    inZ[v]
        fmuls   m09[mat]
        flds    inZ[v]
        fmuls   m08[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inY[v]
        fmuls   m07[mat]
        flds    inY[v]
        fmuls   m06[mat]
        flds    inY[v]
        fmuls   m05[mat]
        flds    inY[v]
        fmuls   m04[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inX[v]
        fmuls   m03[mat]
        flds    inX[v]
        fmuls   m02[mat]
        flds    inX[v]
        fmuls   m01[mat]
        flds    inX[v]
        fmuls   m00[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        fxch    st(3)
        fstsp   [vo]IVert.W
        fstsp   [vo]IVert.Y
        fstsp   [vo]IVert.Z
        fstsp   [vo]IVert.X

        flds    [_gc_]__GLcontext.constants.one
           
        mov     code, CC_BEFORE
        mov     iw, IVert.W[vo]

        add     iw, iw
        jc      AfterClip

        // W is positive

        fdivs   IVert.W[vo]

        mov     iz, IVert.Z[vo]
        mov     iy, IVert.Y[vo]

        mov     ix, IVert.X[vo]
        add     iz, iz

        adc     code, code
        add     iy, iy

        adc     code, code
        add     ix, ix

        adc     code, code
        cmp     iw, iz

        adc     iz, iz
        cmp     iw, iy

        adc     iz, iz
        cmp     iw, ix

        adc     iz, iz
        mov     iy, code

        and     iy, 7
        and     iz, 7

        mov     code, 0
        jz      AfterClip

        mov     iy, _ccsign[iy*4]
        mov     code, _ccmask[iz*4]

        and     code, iy
        nop
AfterClip:
        mov     eax, __GLcontext.state.current.color.r[_gc_]
        mov     ebx, __GLcontext.state.current.color.g[_gc_]
        mov     ecx, __GLcontext.state.current.color.b[_gc_]
        mov     edx, __GLcontext.state.current.color.a[_gc_]
        mov     IVert.r[vo], eax
        mov     IVert.g[vo], ebx
        mov     IVert.b[vo], ecx
        mov     IVert.a[vo], edx

        mov     eax, ps.tc[0].sBias[_gc_]
        mov     ebx, ps.tc[0].tBias[_gc_]
        mov     ecx, __GLcontext.state.current.texture[0].w[_gc_]
        add     eax, __GLcontext.state.current.texture[0].x[_gc_]
        add     ebx, __GLcontext.state.current.texture[0].y[_gc_]
	mov     IVert.t0_oow[vo], ecx
        mov     IVert.t0_sow[vo], eax
        mov     IVert.t0_tow[vo], ebx

        mov     IVert.flags[vo], code
        mov     ecx, ps.vState[_gc_]

        mov     ebx, ps.vB[_gc_]
        mov     eax, ps.vA[_gc_]

        mov     [esp-4], vo
        mov     [esp-8], ebx

        mov     [esp-12], eax
        mov     edx, _VSTEP_C[ecx]

        mov     ps.vB[_gc_], vo
        add     edx, vo

        mov     ps.vC[_gc_], edx
        mov     edx, _VSTEP_A[ecx]

        add     eax, edx
        mov     edx, _VDRAW[ecx]

        mov     ps.vA[_gc_], eax
        mov     ecx, _VNEXT[ecx]

        mov     ps.vState[_gc_], ecx
        lea     vp, __GLcontext.state.viewport[_gc_]

        test    code, code
        jnz     AfterViewport

        flds    IVert.t0_sow[vo]
        fmul    st(0), st(1)
        flds    IVert.t0_tow[vo]
        fmul    st(0), st(2)
        flds    IVert.t0_oow[vo]
        fmul    st(0), st(3)
        flds    IVert.Z[vo]
        fmul    st(0), st(4)
        flds    IVert.Y[vo]
        fmul    st(0), st(5)
        flds    IVert.X[vo]
        fmul    st(0), st(6)

        fxch    st(5)
        fstsp   IVert.t0_sow[vo]
        fxch    st(3)
        fstsp   IVert.t0_tow[vo]
        fxch    st(1)
        fstsp   IVert.t0_oow[vo]

        flds    __GLviewport.zScale[vp]
        fmulp   st(1), st
        flds    __GLviewport.yScale[vp]
        fmulp   st(2), st
        flds    __GLviewport.xScale[vp]
        fmulp   st(3), st

        fadds   __GLviewport.zCenter[vp]
        fxch    st(2)
        fadds   __GLviewport.xCenter[vp]
        fxch    st(1)
        fadds   __GLviewport.yCenter[vp]
        fxch    st(2)

        fstsp   IVert.ooz[vo]
        fstsp   IVert.x[vo]
        fstsp   IVert.y[vo]
        fstp    st(0)
        flds    IVert.t0_oow[vo]

AfterViewport:
        fstsp   IVert.oow[vo]

        test    edx, edx
        jnz     Draw

        pop     ebx
        pop     esi

        pop     edi
        pop     ebp

        ret     4
Draw:
        mov     eax, [esp -12]
        mov     ebx, [esp - 8]

        test    edx, 2
        jz      noSwap

        mov     [esp-12], ebx
        mov     [esp-8],eax
noSwap:
        mov     eax, IVert.flags[eax]
        add     esp, -12

        or      code, eax
        mov     ebx, IVert.flags[ebx]

        or      code, ebx
        jnz     DrawClipped

        call    grDrawTriangle

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
DrawClipped:
        push    5
        call    __glSSTClipAndDraw
        add     esp, 16
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
    }
}

__declspec(naked)
void APIENTRY __glsstim_Vertex2fv_1(const GLfloat *v) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     _gc_,  DWORD PTR __gl_contextArea
        mov     mat,  DWORD PTR [_gc_]__GLcontext.transform.modelView
        add     mat, 168
        mov     v,  _v$[esp]
        mov     vo, ps.vC[_gc_]

//    s->X = m[0] * v[0] + m[4] * v[1] + m[12]; 
//    s->Y = m[1] * v[0] + m[5] * v[1] + m[13];
//    s->Z = m[2] * v[0] + m[6] * v[1] + m[14];
//    s->W = m[3] * v[0] + m[7] * v[1] + m[15];
        flds    m15[mat]
        flds    m14[mat]
        flds    m13[mat]
        flds    m12[mat]

        flds    inY[v]
        fmuls   m07[mat]
        flds    inY[v]
        fmuls   m06[mat]
        flds    inY[v]
        fmuls   m05[mat]
        flds    inY[v]
        fmuls   m04[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inX[v]
        fmuls   m03[mat]
        flds    inX[v]
        fmuls   m02[mat]
        flds    inX[v]
        fmuls   m01[mat]
        flds    inX[v]
        fmuls   m00[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        fxch    st(3)
        fstsp   [vo]IVert.W
        fstsp   [vo]IVert.Y
        fstsp   [vo]IVert.Z
        fstsp   [vo]IVert.X

        flds    [_gc_]__GLcontext.constants.one
           
        mov     code, CC_BEFORE
        mov     iw, IVert.W[vo]

        add     iw, iw
        jc      AfterClip

        // W is positive

        fdivs   IVert.W[vo]

        mov     iz, IVert.Z[vo]
        mov     iy, IVert.Y[vo]

        mov     ix, IVert.X[vo]
        add     iz, iz

        adc     code, code
        add     iy, iy

        adc     code, code
        add     ix, ix

        adc     code, code
        cmp     iw, iz

        adc     iz, iz
        cmp     iw, iy

        adc     iz, iz
        cmp     iw, ix

        adc     iz, iz
        mov     iy, code

        and     iy, 7
        and     iz, 7

        mov     code, 0
        jz      AfterClip

        mov     iy, _ccsign[iy*4]
        mov     code, _ccmask[iz*4]

        and     code, iy
        nop
AfterClip:
        mov     eax, __GLcontext.state.current.color.r[_gc_]
        mov     ebx, __GLcontext.state.current.color.g[_gc_]
        mov     ecx, __GLcontext.state.current.color.b[_gc_]
        mov     edx, __GLcontext.state.current.color.a[_gc_]
        mov     IVert.r[vo], eax
        mov     IVert.g[vo], ebx
        mov     IVert.b[vo], ecx
        mov     IVert.a[vo], edx

        mov     eax, (ps.tc[0].sBias+16)[_gc_]
        mov     ebx, (ps.tc[0].tBias+16)[_gc_]
        mov     ecx, (__GLcontext.state.current.texture[0].w+16)[_gc_]
        add     eax, (__GLcontext.state.current.texture[0].x+16)[_gc_]
        add     ebx, (__GLcontext.state.current.texture[0].y+16)[_gc_]
        mov     IVert.t0_oow[vo], ecx
        mov     IVert.t0_sow[vo], eax
        mov     IVert.t0_tow[vo], ebx

        mov     IVert.flags[vo], code
        mov     ecx, ps.vState[_gc_]

        mov     ebx, ps.vB[_gc_]
        mov     eax, ps.vA[_gc_]

        mov     [esp-4], vo
        mov     [esp-8], ebx

        mov     [esp-12], eax
        mov     edx, _VSTEP_C[ecx]

        mov     ps.vB[_gc_], vo
        add     edx, vo

        mov     ps.vC[_gc_], edx
        mov     edx, _VSTEP_A[ecx]

        add     eax, edx
        mov     edx, _VDRAW[ecx]

        mov     ps.vA[_gc_], eax
        mov     ecx, _VNEXT[ecx]

        mov     ps.vState[_gc_], ecx
        lea     vp, __GLcontext.state.viewport[_gc_]

        test    code, code
        jnz     AfterViewport

        flds    IVert.t0_sow[vo]
        fmul    st(0), st(1)
        flds    IVert.t0_tow[vo]
        fmul    st(0), st(2)
        flds    IVert.t0_oow[vo]
        fmul    st(0), st(3)
        flds    IVert.Z[vo]
        fmul    st(0), st(4)
        flds    IVert.Y[vo]
        fmul    st(0), st(5)
        flds    IVert.X[vo]
        fmul    st(0), st(6)

        fxch    st(5)
        fstsp   IVert.t0_sow[vo]
        fxch    st(3)
        fstsp   IVert.t0_tow[vo]
        fxch    st(1)
        fstsp   IVert.t0_oow[vo]

        flds    __GLviewport.zScale[vp]
        fmulp   st(1), st
        flds    __GLviewport.yScale[vp]
        fmulp   st(2), st
        flds    __GLviewport.xScale[vp]
        fmulp   st(3), st

        fadds   __GLviewport.zCenter[vp]
        fxch    st(2)
        fadds   __GLviewport.xCenter[vp]
        fxch    st(1)
        fadds   __GLviewport.yCenter[vp]
        fxch    st(2)

        fstsp   IVert.ooz[vo]
        fstsp   IVert.x[vo]
        fstsp   IVert.y[vo]
        fstp    st(0)
        flds    IVert.t0_oow[vo]

AfterViewport:
        fstsp   IVert.oow[vo]

        test    edx, edx
        jnz     Draw

        pop     ebx
        pop     esi

        pop     edi
        pop     ebp

        ret     4
Draw:
        mov     eax, [esp -12]
        mov     ebx, [esp - 8]

        test    edx, 2
        jz      noSwap

        mov     [esp-12], ebx
        mov     [esp-8],eax
noSwap:
        mov     eax, IVert.flags[eax]
        add     esp, -12

        or      code, eax
        mov     ebx, IVert.flags[ebx]

        or      code, ebx
        jnz     DrawClipped

        call    grDrawTriangle

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
DrawClipped:
        push    5
        call    __glSSTClipAndDraw
        add     esp, 16
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
    }
}

__declspec(naked)
void APIENTRY __glsstim_Vertex3fv_1(const GLfloat *v) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     _gc_,  DWORD PTR __gl_contextArea
        mov     mat,  DWORD PTR [_gc_]__GLcontext.transform.modelView
        add     mat, 168
        mov     v,  _v$[esp]
        mov     vo, ps.vC[_gc_]

//    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12]; 
//    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13];
//    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
//    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15];
        flds    m15[mat]
        flds    m14[mat]
        flds    m13[mat]
        flds    m12[mat]

        flds    inZ[v]
        fmuls   m11[mat]
        flds    inZ[v]
        fmuls   m10[mat]
        flds    inZ[v]
        fmuls   m09[mat]
        flds    inZ[v]
        fmuls   m08[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inY[v]
        fmuls   m07[mat]
        flds    inY[v]
        fmuls   m06[mat]
        flds    inY[v]
        fmuls   m05[mat]
        flds    inY[v]
        fmuls   m04[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inX[v]
        fmuls   m03[mat]
        flds    inX[v]
        fmuls   m02[mat]
        flds    inX[v]
        fmuls   m01[mat]
        flds    inX[v]
        fmuls   m00[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        fxch    st(3)
        fstsp   [vo]IVert.W
        fstsp   [vo]IVert.Y
        fstsp   [vo]IVert.Z
        fstsp   [vo]IVert.X

        flds    [_gc_]__GLcontext.constants.one
           
        mov     code, CC_BEFORE
        mov     iw, IVert.W[vo]

        add     iw, iw
        jc      AfterClip

        // W is positive

        fdivs   IVert.W[vo]

        mov     iz, IVert.Z[vo]
        mov     iy, IVert.Y[vo]

        mov     ix, IVert.X[vo]
        add     iz, iz

        adc     code, code
        add     iy, iy

        adc     code, code
        add     ix, ix

        adc     code, code
        cmp     iw, iz

        adc     iz, iz
        cmp     iw, iy

        adc     iz, iz
        cmp     iw, ix

        adc     iz, iz
        mov     iy, code

        and     iy, 7
        and     iz, 7

        mov     code, 0
        jz      AfterClip

        mov     iy, _ccsign[iy*4]
        mov     code, _ccmask[iz*4]

        and     code, iy
        nop
AfterClip:
        mov     eax, __GLcontext.state.current.color.r[_gc_]
        mov     ebx, __GLcontext.state.current.color.g[_gc_]
        mov     ecx, __GLcontext.state.current.color.b[_gc_]
        mov     edx, __GLcontext.state.current.color.a[_gc_]
        mov     IVert.r[vo], eax
        mov     IVert.g[vo], ebx
        mov     IVert.b[vo], ecx
        mov     IVert.a[vo], edx

        mov     eax, (ps.tc[0].sBias+16)[_gc_]
        mov     ebx, (ps.tc[0].tBias+16)[_gc_]
        mov     ecx, (__GLcontext.state.current.texture[0].w+16)[_gc_]
        add     eax, (__GLcontext.state.current.texture[0].x+16)[_gc_]
        add     ebx, (__GLcontext.state.current.texture[0].y+16)[_gc_]
        mov     IVert.t0_oow[vo], ecx
        mov     IVert.t0_sow[vo], eax
        mov     IVert.t0_tow[vo], ebx

        mov     IVert.flags[vo], code
        mov     ecx, ps.vState[_gc_]

        mov     ebx, ps.vB[_gc_]
        mov     eax, ps.vA[_gc_]

        mov     [esp-4], vo
        mov     [esp-8], ebx

        mov     [esp-12], eax
        mov     edx, _VSTEP_C[ecx]

        mov     ps.vB[_gc_], vo
        add     edx, vo

        mov     ps.vC[_gc_], edx
        mov     edx, _VSTEP_A[ecx]

        add     eax, edx
        mov     edx, _VDRAW[ecx]

        mov     ps.vA[_gc_], eax
        mov     ecx, _VNEXT[ecx]

        mov     ps.vState[_gc_], ecx
        lea     vp, __GLcontext.state.viewport[_gc_]

        test    code, code
        jnz     AfterViewport

        flds    IVert.t0_sow[vo]
        fmul    st(0), st(1)
        flds    IVert.t0_tow[vo]
        fmul    st(0), st(2)
        flds    IVert.t0_oow[vo]
        fmul    st(0), st(3)
        flds    IVert.Z[vo]
        fmul    st(0), st(4)
        flds    IVert.Y[vo]
        fmul    st(0), st(5)
        flds    IVert.X[vo]
        fmul    st(0), st(6)

        fxch    st(5)
        fstsp   IVert.t0_sow[vo]
        fxch    st(3)
        fstsp   IVert.t0_tow[vo]
        fxch    st(1)
        fstsp   IVert.t0_oow[vo]

        flds    __GLviewport.zScale[vp]
        fmulp   st(1), st
        flds    __GLviewport.yScale[vp]
        fmulp   st(2), st
        flds    __GLviewport.xScale[vp]
        fmulp   st(3), st

        fadds   __GLviewport.zCenter[vp]
        fxch    st(2)
        fadds   __GLviewport.xCenter[vp]
        fxch    st(1)
        fadds   __GLviewport.yCenter[vp]
        fxch    st(2)

        fstsp   IVert.ooz[vo]
        fstsp   IVert.x[vo]
        fstsp   IVert.y[vo]
        fstp    st(0)
        flds    IVert.t0_oow[vo]

AfterViewport:
        fstsp   IVert.oow[vo]

        test    edx, edx
        jnz     Draw

        pop     ebx
        pop     esi

        pop     edi
        pop     ebp

        ret     4
Draw:
        mov     eax, [esp -12]
        mov     ebx, [esp - 8]

        test    edx, 2
        jz      noSwap

        mov     [esp-12], ebx
        mov     [esp-8],eax
noSwap:
        mov     eax, IVert.flags[eax]
        add     esp, -12

        or      code, eax
        mov     ebx, IVert.flags[ebx]

        or      code, ebx
        jnz     DrawClipped

        call    grDrawTriangle

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
DrawClipped:
        push    5
        call    __glSSTClipAndDraw
        add     esp, 16
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
    }
}

__declspec(naked)
void APIENTRY __glsstim_Vertex4fv_1(const GLfloat *v) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     _gc_,  DWORD PTR __gl_contextArea
        mov     mat,  DWORD PTR [_gc_]__GLcontext.transform.modelView
        add     mat, 168
        mov     v,  _v$[esp]
        mov     vo, ps.vC[_gc_]

//    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12] * v[3]; 
//    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13] * v[3];
//    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
//    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];
        flds    m15[mat]
        fmuls   inW[v]
        flds    m14[mat]
        fmuls   inW[v]
        flds    m13[mat]
        fmuls   inW[v]
        flds    m12[mat]
        fmuls   inW[v]

        flds    inZ[v]
        fmuls   m11[mat]
        flds    inZ[v]
        fmuls   m10[mat]
        flds    inZ[v]
        fmuls   m09[mat]
        flds    inZ[v]
        fmuls   m08[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inY[v]
        fmuls   m07[mat]
        flds    inY[v]
        fmuls   m06[mat]
        flds    inY[v]
        fmuls   m05[mat]
        flds    inY[v]
        fmuls   m04[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inX[v]
        fmuls   m03[mat]
        flds    inX[v]
        fmuls   m02[mat]
        flds    inX[v]
        fmuls   m01[mat]
        flds    inX[v]
        fmuls   m00[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        fxch    st(3)
        fstsp   [vo]IVert.W
        fstsp   [vo]IVert.Y
        fstsp   [vo]IVert.Z
        fstsp   [vo]IVert.X

        flds    [_gc_]__GLcontext.constants.one
           
        mov     code, CC_BEFORE
        mov     iw, IVert.W[vo]

        add     iw, iw
        jc      AfterClip

        // W is positive

        fdivs   IVert.W[vo]

        mov     iz, IVert.Z[vo]
        mov     iy, IVert.Y[vo]

        mov     ix, IVert.X[vo]
        add     iz, iz

        adc     code, code
        add     iy, iy

        adc     code, code
        add     ix, ix

        adc     code, code
        cmp     iw, iz

        adc     iz, iz
        cmp     iw, iy

        adc     iz, iz
        cmp     iw, ix

        adc     iz, iz
        mov     iy, code

        and     iy, 7
        and     iz, 7

        mov     code, 0
        jz      AfterClip

        mov     iy, _ccsign[iy*4]
        mov     code, _ccmask[iz*4]

        and     code, iy
        nop
AfterClip:
        mov     eax, __GLcontext.state.current.color.r[_gc_]
        mov     ebx, __GLcontext.state.current.color.g[_gc_]
        mov     ecx, __GLcontext.state.current.color.b[_gc_]
        mov     edx, __GLcontext.state.current.color.a[_gc_]
        mov     IVert.r[vo], eax
        mov     IVert.g[vo], ebx
        mov     IVert.b[vo], ecx
        mov     IVert.a[vo], edx

        mov     eax, (ps.tc[0].sBias+16)[_gc_]
        mov     ebx, (ps.tc[0].tBias+16)[_gc_]
        mov     ecx, (__GLcontext.state.current.texture[0].w+16)[_gc_]
        add     eax, (__GLcontext.state.current.texture[0].x+16)[_gc_]
        add     ebx, (__GLcontext.state.current.texture[0].y+16)[_gc_]
        mov     IVert.t0_oow[vo], ecx
        mov     IVert.t0_sow[vo], eax
        mov     IVert.t0_tow[vo], ebx

        mov     IVert.flags[vo], code
        mov     ecx, ps.vState[_gc_]

        mov     ebx, ps.vB[_gc_]
        mov     eax, ps.vA[_gc_]

        mov     [esp-4], vo
        mov     [esp-8], ebx

        mov     [esp-12], eax
        mov     edx, _VSTEP_C[ecx]

        mov     ps.vB[_gc_], vo
        add     edx, vo

        mov     ps.vC[_gc_], edx
        mov     edx, _VSTEP_A[ecx]

        add     eax, edx
        mov     edx, _VDRAW[ecx]

        mov     ps.vA[_gc_], eax
        mov     ecx, _VNEXT[ecx]

        mov     ps.vState[_gc_], ecx
        lea     vp, __GLcontext.state.viewport[_gc_]

        test    code, code
        jnz     AfterViewport

        flds    IVert.t0_sow[vo]
        fmul    st(0), st(1)
        flds    IVert.t0_tow[vo]
        fmul    st(0), st(2)
        flds    IVert.t0_oow[vo]
        fmul    st(0), st(3)
        flds    IVert.Z[vo]
        fmul    st(0), st(4)
        flds    IVert.Y[vo]
        fmul    st(0), st(5)
        flds    IVert.X[vo]
        fmul    st(0), st(6)

        fxch    st(5)
        fstsp   IVert.t0_sow[vo]
        fxch    st(3)
        fstsp   IVert.t0_tow[vo]
        fxch    st(1)
        fstsp   IVert.t0_oow[vo]

        flds    __GLviewport.zScale[vp]
        fmulp   st(1), st
        flds    __GLviewport.yScale[vp]
        fmulp   st(2), st
        flds    __GLviewport.xScale[vp]
        fmulp   st(3), st

        fadds   __GLviewport.zCenter[vp]
        fxch    st(2)
        fadds   __GLviewport.xCenter[vp]
        fxch    st(1)
        fadds   __GLviewport.yCenter[vp]
        fxch    st(2)

        fstsp   IVert.ooz[vo]
        fstsp   IVert.x[vo]
        fstsp   IVert.y[vo]
        fstp    st(0)
        flds    IVert.t0_oow[vo]

AfterViewport:
        fstsp   IVert.oow[vo]

        test    edx, edx
        jnz     Draw

        pop     ebx
        pop     esi

        pop     edi
        pop     ebp

        ret     4
Draw:
        mov     eax, [esp -12]
        mov     ebx, [esp - 8]

        test    edx, 2
        jz      noSwap

        mov     [esp-12], ebx
        mov     [esp-8],eax
noSwap:
        mov     eax, IVert.flags[eax]
        add     esp, -12

        or      code, eax
        mov     ebx, IVert.flags[ebx]

        or      code, ebx
        jnz     DrawClipped

        call    grDrawTriangle

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
DrawClipped:
        push    5
        call    __glSSTClipAndDraw
        add     esp, 16
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
    }
}


__declspec(naked)
void APIENTRY __glsstim_Vertex2fv_B(const GLfloat *v) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     _gc_,  DWORD PTR __gl_contextArea
        mov     mat,  DWORD PTR [_gc_]__GLcontext.transform.modelView
        add     mat, 168
        mov     v,  _v$[esp]
        mov     vo, ps.vC[_gc_]

//    s->X = m[0] * v[0] + m[4] * v[1] + m[12]; 
//    s->Y = m[1] * v[0] + m[5] * v[1] + m[13];
//    s->Z = m[2] * v[0] + m[6] * v[1] + m[14];
//    s->W = m[3] * v[0] + m[7] * v[1] + m[15];
        flds    m15[mat]
        flds    m14[mat]
        flds    m13[mat]
        flds    m12[mat]

        flds    inY[v]
        fmuls   m07[mat]
        flds    inY[v]
        fmuls   m06[mat]
        flds    inY[v]
        fmuls   m05[mat]
        flds    inY[v]
        fmuls   m04[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inX[v]
        fmuls   m03[mat]
        flds    inX[v]
        fmuls   m02[mat]
        flds    inX[v]
        fmuls   m01[mat]
        flds    inX[v]
        fmuls   m00[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        fxch    st(3)
        fstsp   [vo]IVert.W
        fstsp   [vo]IVert.Y
        fstsp   [vo]IVert.Z
        fstsp   [vo]IVert.X

        flds    [_gc_]__GLcontext.constants.one
           
        mov     code, CC_BEFORE
        mov     iw, IVert.W[vo]

        add     iw, iw
        jc      AfterClip

        // W is positive

        fdivs   IVert.W[vo]

        mov     iz, IVert.Z[vo]
        mov     iy, IVert.Y[vo]

        mov     ix, IVert.X[vo]
        add     iz, iz

        adc     code, code
        add     iy, iy

        adc     code, code
        add     ix, ix

        adc     code, code
        cmp     iw, iz

        adc     iz, iz
        cmp     iw, iy

        adc     iz, iz
        cmp     iw, ix

        adc     iz, iz
        mov     iy, code

        and     iy, 7
        and     iz, 7

        mov     code, 0
        jz      AfterClip

        mov     iy, _ccsign[iy*4]
        mov     code, _ccmask[iz*4]

        and     code, iy
        nop
AfterClip:
        mov     eax, (ps.tc[0].sBias+16)[_gc_]
        mov     ebx, (ps.tc[0].tBias+16)[_gc_]
        mov     ecx, (__GLcontext.state.current.texture[0].w+16)[_gc_]
        add     eax, (__GLcontext.state.current.texture[0].x+16)[_gc_]
        add     ebx, (__GLcontext.state.current.texture[0].y+16)[_gc_]
        mov     IVert.t0_oow[vo], ecx
        mov     IVert.t0_sow[vo], eax
        mov     IVert.t0_tow[vo], ebx

        mov     ecx, ps.tc[0].sBias[_gc_]
        mov     edx, ps.tc[0].tBias[_gc_]
        mov     eax, __GLcontext.state.current.texture[0].w[_gc_]
        add     ecx, __GLcontext.state.current.texture[0].x[_gc_]
        add     edx, __GLcontext.state.current.texture[0].y[_gc_]
        mov     IVert.t1_oow[vo], eax
        mov     IVert.t1_sow[vo], ecx
        mov     IVert.t1_tow[vo], edx

        mov     IVert.flags[vo], code
        mov     ecx, ps.vState[_gc_]

        mov     ebx, ps.vB[_gc_]
        mov     eax, ps.vA[_gc_]

        mov     [esp-4], vo
        mov     [esp-8], ebx

        mov     [esp-12], eax
        mov     edx, _VSTEP_C[ecx]

        mov     ps.vB[_gc_], vo
        add     edx, vo

        mov     ps.vC[_gc_], edx
        mov     edx, _VSTEP_A[ecx]

        add     eax, edx
        mov     edx, _VDRAW[ecx]

        mov     ps.vA[_gc_], eax
        mov     ecx, _VNEXT[ecx]

        mov     ps.vState[_gc_], ecx
        lea     vp, __GLcontext.state.viewport[_gc_]

        test    code, code
        jnz     AfterViewport

        flds    IVert.t0_sow[vo]    ; W S0
        fmul    st(0), st(1)        ; W S0ow
        flds    IVert.t0_tow[vo]    ; W S0ow T0
        fmul    st(0), st(2)        ; W S0ow T0ow
        flds    IVert.t1_sow[vo]    ; W S0ow T0ow S1
        fmul    st(0), st(3)        ; W S0ow T0ow S1ow
        flds    IVert.t1_tow[vo]    ; W S0ow T0ow S1ow T1
        fmul    st(0), st(4)        ; W S0ow T0ow S1ow T1ow

        flds    IVert.Z[vo]         ; W S0ow T0ow S1ow T1ow Z
        fmul    st(0), st(5)        ; W S0ow T0ow S1ow T1ow Zow
        flds    IVert.Y[vo]         ; W S0ow T0ow S1ow T1ow Zow Y
        fmul    st(0), st(6)        ; W S0ow T0ow S1ow T1ow Zow Yow
        flds    IVert.X[vo]         ; W S0ow T0ow S1ow T1ow Zow Yow X
        fmul    st(0), st(7)        ; W S0ow T0ow S1ow T1ow Zow Yow Xow

        fxch    st(6)               ; W Xow T0ow S1ow T1ow Zow Yow S0ow
        fstsp   IVert.t0_sow[vo]    ; W Xow T0ow S1ow T1ow Zow Yow
        fxch    st(4)               ; W Xow Yow S1ow T1ow Zow T0ow
        fstsp   IVert.t0_tow[vo]    ; W Xow Yow S1ow T1ow Zow
        flds    IVert.t0_oow[vo]    ; W Xow Yow S1ow T1ow Zow Q0
        fmul    st(0), st(6)        ; W Xow Yow S1ow T1ow Zow Q0ow
        flds    IVert.t1_oow[vo]    ; W Xow Yow S1ow T1ow Zow Q0ow Q1
        fmul    st(0), st(7)        ; W Xow Yow S1ow T1ow Zow Q0ow Q10w
	
        fxch    st(4)               ; W Xow Yow Q1ow T1ow Zow Q0ow S10w
        fstsp   IVert.t1_sow[vo]    ; W Xow Yow Q1ow T1ow Zow Q0ow
        fxch    st(2)               ; W Xow Yow Q1ow Q0ow Zow T1ow
        fstsp   IVert.t1_tow[vo]    ; W Xow Yow Q1ow Q0ow Zow
        fxch    st(2)               ; W Xow Yow Zow Q0ow Q1ow
        fstsp   IVert.t1_oow[vo]    ; W Xow Yow Zow Q0ow
        fstsp   IVert.t0_oow[vo]    ; W Xow Yow Zow

        flds    __GLviewport.zScale[vp]
        fmulp   st(1), st
        flds    __GLviewport.yScale[vp]
        fmulp   st(2), st
        flds    __GLviewport.xScale[vp]
        fmulp   st(3), st

        fadds   __GLviewport.zCenter[vp]
        fxch    st(2)
        fadds   __GLviewport.xCenter[vp]
        fxch    st(1)
        fadds   __GLviewport.yCenter[vp]
        fxch    st(2)

        fstsp   IVert.ooz[vo]
        fstsp   IVert.x[vo]
        fstsp   IVert.y[vo]
        fstp    st(0)
        flds    IVert.t0_oow[vo]

AfterViewport:
        fstsp   IVert.oow[vo]

        test    edx, edx
        jnz     Draw

        pop     ebx
        pop     esi

        pop     edi
        pop     ebp

        ret     4
Draw:
        mov     eax, [esp -12]
        mov     ebx, [esp - 8]

        test    edx, 2
        jz      noSwap

        mov     [esp-12], ebx
        mov     [esp-8],eax
noSwap:
        mov     eax, IVert.flags[eax]
        add     esp, -12

        or      code, eax
        mov     ebx, IVert.flags[ebx]

        or      code, ebx
        jnz     DrawClipped

        call    grDrawTriangle

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
DrawClipped:
        push    5
        call    __glSSTClipAndDraw_B
        add     esp, 16
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
    }
}

__declspec(naked)
void APIENTRY __glsstim_Vertex3fv_B(const GLfloat *v) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     _gc_,  DWORD PTR __gl_contextArea
        mov     mat,  DWORD PTR [_gc_]__GLcontext.transform.modelView
        add     mat, 168
        mov     v,  _v$[esp]
        mov     vo, ps.vC[_gc_]

//    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12]; 
//    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13];
//    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
//    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15];
        flds    m15[mat]
        flds    m14[mat]
        flds    m13[mat]
        flds    m12[mat]

        flds    inZ[v]
        fmuls   m11[mat]
        flds    inZ[v]
        fmuls   m10[mat]
        flds    inZ[v]
        fmuls   m09[mat]
        flds    inZ[v]
        fmuls   m08[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inY[v]
        fmuls   m07[mat]
        flds    inY[v]
        fmuls   m06[mat]
        flds    inY[v]
        fmuls   m05[mat]
        flds    inY[v]
        fmuls   m04[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inX[v]
        fmuls   m03[mat]
        flds    inX[v]
        fmuls   m02[mat]
        flds    inX[v]
        fmuls   m01[mat]
        flds    inX[v]
        fmuls   m00[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        fxch    st(3)
        fstsp   [vo]IVert.W
        fstsp   [vo]IVert.Y
        fstsp   [vo]IVert.Z
        fstsp   [vo]IVert.X

        flds    [_gc_]__GLcontext.constants.one
           
        mov     code, CC_BEFORE
        mov     iw, IVert.W[vo]

        add     iw, iw
        jc      AfterClip

        // W is positive

        fdivs   IVert.W[vo]

        mov     iz, IVert.Z[vo]
        mov     iy, IVert.Y[vo]

        mov     ix, IVert.X[vo]
        add     iz, iz

        adc     code, code
        add     iy, iy

        adc     code, code
        add     ix, ix

        adc     code, code
        cmp     iw, iz

        adc     iz, iz
        cmp     iw, iy

        adc     iz, iz
        cmp     iw, ix

        adc     iz, iz
        mov     iy, code

        and     iy, 7
        and     iz, 7

        mov     code, 0
        jz      AfterClip

        mov     iy, _ccsign[iy*4]
        mov     code, _ccmask[iz*4]

        and     code, iy
        nop
AfterClip:
        mov     eax, (ps.tc[0].sBias+16)[_gc_]
        mov     ebx, (ps.tc[0].tBias+16)[_gc_]
        mov     ecx, (__GLcontext.state.current.texture[0].w+16)[_gc_]
        add     eax, (__GLcontext.state.current.texture[0].x+16)[_gc_]
        add     ebx, (__GLcontext.state.current.texture[0].y+16)[_gc_]
        mov     IVert.t0_oow[vo], ecx
        mov     IVert.t0_sow[vo], eax
        mov     IVert.t0_tow[vo], ebx

        mov     ecx, ps.tc[0].sBias[_gc_]
        mov     edx, ps.tc[0].tBias[_gc_]
        mov     eax, __GLcontext.state.current.texture[0].w[_gc_]
        add     ecx, __GLcontext.state.current.texture[0].x[_gc_]
        add     edx, __GLcontext.state.current.texture[0].y[_gc_]
        mov     IVert.t1_oow[vo], eax
        mov     IVert.t1_sow[vo], ecx
        mov     IVert.t1_tow[vo], edx

        mov     IVert.flags[vo], code
        mov     ecx, ps.vState[_gc_]

        mov     ebx, ps.vB[_gc_]
        mov     eax, ps.vA[_gc_]

        mov     [esp-4], vo
        mov     [esp-8], ebx

        mov     [esp-12], eax
        mov     edx, _VSTEP_C[ecx]

        mov     ps.vB[_gc_], vo
        add     edx, vo

        mov     ps.vC[_gc_], edx
        mov     edx, _VSTEP_A[ecx]

        add     eax, edx
        mov     edx, _VDRAW[ecx]

        mov     ps.vA[_gc_], eax
        mov     ecx, _VNEXT[ecx]

        mov     ps.vState[_gc_], ecx
        lea     vp, __GLcontext.state.viewport[_gc_]

        test    code, code
        jnz     AfterViewport

        flds    IVert.t0_sow[vo]    ; W S0
        fmul    st(0), st(1)        ; W S0ow
        flds    IVert.t0_tow[vo]    ; W S0ow T0
        fmul    st(0), st(2)        ; W S0ow T0ow
        flds    IVert.t1_sow[vo]    ; W S0ow T0ow S1
        fmul    st(0), st(3)        ; W S0ow T0ow S1ow
        flds    IVert.t1_tow[vo]    ; W S0ow T0ow S1ow T1
        fmul    st(0), st(4)        ; W S0ow T0ow S1ow T1ow

        flds    IVert.Z[vo]         ; W S0ow T0ow S1ow T1ow Z
        fmul    st(0), st(5)        ; W S0ow T0ow S1ow T1ow Zow
        flds    IVert.Y[vo]         ; W S0ow T0ow S1ow T1ow Zow Y
        fmul    st(0), st(6)        ; W S0ow T0ow S1ow T1ow Zow Yow
        flds    IVert.X[vo]         ; W S0ow T0ow S1ow T1ow Zow Yow X
        fmul    st(0), st(7)        ; W S0ow T0ow S1ow T1ow Zow Yow Xow

        fxch    st(6)               ; W Xow T0ow S1ow T1ow Zow Yow S0ow
        fstsp   IVert.t0_sow[vo]    ; W Xow T0ow S1ow T1ow Zow Yow
        fxch    st(4)               ; W Xow Yow S1ow T1ow Zow T0ow
        fstsp   IVert.t0_tow[vo]    ; W Xow Yow S1ow T1ow Zow
        flds    IVert.t0_oow[vo]    ; W Xow Yow S1ow T1ow Zow Q0
        fmul    st(0), st(6)        ; W Xow Yow S1ow T1ow Zow Q0ow
        flds    IVert.t1_oow[vo]    ; W Xow Yow S1ow T1ow Zow Q0ow Q1
        fmul    st(0), st(7)        ; W Xow Yow S1ow T1ow Zow Q0ow Q10w
	
        fxch    st(4)               ; W Xow Yow Q1ow T1ow Zow Q0ow S10w
        fstsp   IVert.t1_sow[vo]    ; W Xow Yow Q1ow T1ow Zow Q0ow
        fxch    st(2)               ; W Xow Yow Q1ow Q0ow Zow T1ow
        fstsp   IVert.t1_tow[vo]    ; W Xow Yow Q1ow Q0ow Zow
        fxch    st(2)               ; W Xow Yow Zow Q0ow Q1ow
        fstsp   IVert.t1_oow[vo]    ; W Xow Yow Zow Q0ow
        fstsp   IVert.t0_oow[vo]    ; W Xow Yow Zow

        flds    __GLviewport.zScale[vp]
        fmulp   st(1), st
        flds    __GLviewport.yScale[vp]
        fmulp   st(2), st
        flds    __GLviewport.xScale[vp]
        fmulp   st(3), st

        fadds   __GLviewport.zCenter[vp]
        fxch    st(2)
        fadds   __GLviewport.xCenter[vp]
        fxch    st(1)
        fadds   __GLviewport.yCenter[vp]
        fxch    st(2)

        fstsp   IVert.ooz[vo]
        fstsp   IVert.x[vo]
        fstsp   IVert.y[vo]
        fstp    st(0)
        flds    IVert.t0_oow[vo]

AfterViewport:
        fstsp   IVert.oow[vo]

        test    edx, edx
        jnz     Draw

        pop     ebx
        pop     esi

        pop     edi
        pop     ebp

        ret     4
Draw:
        mov     eax, [esp -12]
        mov     ebx, [esp - 8]

        test    edx, 2
        jz      noSwap

        mov     [esp-12], ebx
        mov     [esp-8],eax
noSwap:
        mov     eax, IVert.flags[eax]
        add     esp, -12

        or      code, eax
        mov     ebx, IVert.flags[ebx]

        or      code, ebx
        jnz     DrawClipped

        call    grDrawTriangle

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
DrawClipped:
        push    5
        call    __glSSTClipAndDraw_B
        add     esp, 16
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
    }
}

__declspec(naked)
void APIENTRY __glsstim_Vertex4fv_B(const GLfloat *v) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     _gc_,  DWORD PTR __gl_contextArea
        mov     mat,  DWORD PTR [_gc_]__GLcontext.transform.modelView
        add     mat, 168
        mov     v,  _v$[esp]
        mov     vo, ps.vC[_gc_]

//    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12] * v[3]; 
//    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13] * v[3];
//    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
//    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];
        flds    m15[mat]
        fmuls   inW[v]
        flds    m14[mat]
        fmuls   inW[v]
        flds    m13[mat]
        fmuls   inW[v]
        flds    m12[mat]
        fmuls   inW[v]

        flds    inZ[v]
        fmuls   m11[mat]
        flds    inZ[v]
        fmuls   m10[mat]
        flds    inZ[v]
        fmuls   m09[mat]
        flds    inZ[v]
        fmuls   m08[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inY[v]
        fmuls   m07[mat]
        flds    inY[v]
        fmuls   m06[mat]
        flds    inY[v]
        fmuls   m05[mat]
        flds    inY[v]
        fmuls   m04[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inX[v]
        fmuls   m03[mat]
        flds    inX[v]
        fmuls   m02[mat]
        flds    inX[v]
        fmuls   m01[mat]
        flds    inX[v]
        fmuls   m00[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        fxch    st(3)
        fstsp   [vo]IVert.W
        fstsp   [vo]IVert.Y
        fstsp   [vo]IVert.Z
        fstsp   [vo]IVert.X

        flds    [_gc_]__GLcontext.constants.one
           
        mov     code, CC_BEFORE
        mov     iw, IVert.W[vo]

        add     iw, iw
        jc      AfterClip

        // W is positive

        fdivs   IVert.W[vo]

        mov     iz, IVert.Z[vo]
        mov     iy, IVert.Y[vo]

        mov     ix, IVert.X[vo]
        add     iz, iz

        adc     code, code
        add     iy, iy

        adc     code, code
        add     ix, ix

        adc     code, code
        cmp     iw, iz

        adc     iz, iz
        cmp     iw, iy

        adc     iz, iz
        cmp     iw, ix

        adc     iz, iz
        mov     iy, code

        and     iy, 7
        and     iz, 7

        mov     code, 0
        jz      AfterClip

        mov     iy, _ccsign[iy*4]
        mov     code, _ccmask[iz*4]

        and     code, iy
        nop
AfterClip:
        mov     eax, (ps.tc[0].sBias+16)[_gc_]
        mov     ebx, (ps.tc[0].tBias+16)[_gc_]
        mov     ecx, (__GLcontext.state.current.texture[0].w+16)[_gc_]
        add     eax, (__GLcontext.state.current.texture[0].x+16)[_gc_]
        add     ebx, (__GLcontext.state.current.texture[0].y+16)[_gc_]
        mov     IVert.t0_oow[vo], ecx
        mov     IVert.t0_sow[vo], eax
        mov     IVert.t0_tow[vo], ebx

        mov     ecx, ps.tc[0].sBias[_gc_]
        mov     edx, ps.tc[0].tBias[_gc_]
        mov     eax, __GLcontext.state.current.texture[0].w[_gc_]
        add     ecx, __GLcontext.state.current.texture[0].x[_gc_]
        add     edx, __GLcontext.state.current.texture[0].y[_gc_]
        mov     IVert.t1_oow[vo], eax
        mov     IVert.t1_sow[vo], ecx
        mov     IVert.t1_tow[vo], edx

        mov     IVert.flags[vo], code
        mov     ecx, ps.vState[_gc_]

        mov     ebx, ps.vB[_gc_]
        mov     eax, ps.vA[_gc_]

        mov     [esp-4], vo
        mov     [esp-8], ebx

        mov     [esp-12], eax
        mov     edx, _VSTEP_C[ecx]

        mov     ps.vB[_gc_], vo
        add     edx, vo

        mov     ps.vC[_gc_], edx
        mov     edx, _VSTEP_A[ecx]

        add     eax, edx
        mov     edx, _VDRAW[ecx]

        mov     ps.vA[_gc_], eax
        mov     ecx, _VNEXT[ecx]

        mov     ps.vState[_gc_], ecx
        lea     vp, __GLcontext.state.viewport[_gc_]

        test    code, code
        jnz     AfterViewport

        flds    IVert.t0_sow[vo]    ; W S0
        fmul    st(0), st(1)        ; W S0ow
        flds    IVert.t0_tow[vo]    ; W S0ow T0
        fmul    st(0), st(2)        ; W S0ow T0ow
        flds    IVert.t1_sow[vo]    ; W S0ow T0ow S1
        fmul    st(0), st(3)        ; W S0ow T0ow S1ow
        flds    IVert.t1_tow[vo]    ; W S0ow T0ow S1ow T1
        fmul    st(0), st(4)        ; W S0ow T0ow S1ow T1ow

        flds    IVert.Z[vo]         ; W S0ow T0ow S1ow T1ow Z
        fmul    st(0), st(5)        ; W S0ow T0ow S1ow T1ow Zow
        flds    IVert.Y[vo]         ; W S0ow T0ow S1ow T1ow Zow Y
        fmul    st(0), st(6)        ; W S0ow T0ow S1ow T1ow Zow Yow
        flds    IVert.X[vo]         ; W S0ow T0ow S1ow T1ow Zow Yow X
        fmul    st(0), st(7)        ; W S0ow T0ow S1ow T1ow Zow Yow Xow

        fxch    st(6)               ; W Xow T0ow S1ow T1ow Zow Yow S0ow
        fstsp   IVert.t0_sow[vo]    ; W Xow T0ow S1ow T1ow Zow Yow
        fxch    st(4)               ; W Xow Yow S1ow T1ow Zow T0ow
        fstsp   IVert.t0_tow[vo]    ; W Xow Yow S1ow T1ow Zow
        flds    IVert.t0_oow[vo]    ; W Xow Yow S1ow T1ow Zow Q0
        fmul    st(0), st(6)        ; W Xow Yow S1ow T1ow Zow Q0ow
        flds    IVert.t1_oow[vo]    ; W Xow Yow S1ow T1ow Zow Q0ow Q1
        fmul    st(0), st(7)        ; W Xow Yow S1ow T1ow Zow Q0ow Q10w
	
        fxch    st(4)               ; W Xow Yow Q1ow T1ow Zow Q0ow S10w
        fstsp   IVert.t1_sow[vo]    ; W Xow Yow Q1ow T1ow Zow Q0ow
        fxch    st(2)               ; W Xow Yow Q1ow Q0ow Zow T1ow
        fstsp   IVert.t1_tow[vo]    ; W Xow Yow Q1ow Q0ow Zow
        fxch    st(2)               ; W Xow Yow Zow Q0ow Q1ow
        fstsp   IVert.t1_oow[vo]    ; W Xow Yow Zow Q0ow
        fstsp   IVert.t0_oow[vo]    ; W Xow Yow Zow

        flds    __GLviewport.zScale[vp]
        fmulp   st(1), st
        flds    __GLviewport.yScale[vp]
        fmulp   st(2), st
        flds    __GLviewport.xScale[vp]
        fmulp   st(3), st

        fadds   __GLviewport.zCenter[vp]
        fxch    st(2)
        fadds   __GLviewport.xCenter[vp]
        fxch    st(1)
        fadds   __GLviewport.yCenter[vp]
        fxch    st(2)

        fstsp   IVert.ooz[vo]
        fstsp   IVert.x[vo]
        fstsp   IVert.y[vo]
        fstp    st(0)
        flds    IVert.t0_oow[vo]

AfterViewport:
        fstsp   IVert.oow[vo]

        test    edx, edx
        jnz     Draw

        pop     ebx
        pop     esi

        pop     edi
        pop     ebp

        ret     4
Draw:
        mov     eax, [esp -12]
        mov     ebx, [esp - 8]

        test    edx, 2
        jz      noSwap

        mov     [esp-12], ebx
        mov     [esp-8],eax
noSwap:
        mov     eax, IVert.flags[eax]
        add     esp, -12

        or      code, eax
        mov     ebx, IVert.flags[ebx]

        or      code, ebx
        jnz     DrawClipped

        call    grDrawTriangle

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
DrawClipped:
        push    5
        call    __glSSTClipAndDraw_B
        add     esp, 16
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
    }
}

__declspec(naked) static void 
__glSSTIntersect(  IVert *in,  IVert *out,  IVert *mid, unsigned plane) {
        __asm {
#define _stackOff 16
#define _vi$    4+_stackOff
#define _vo$    8+_stackOff
#define _vm$    12+_stackOff
#define _plane$ 16+_stackOff

#undef vi
#undef vo
#undef plane
#define vi    eax
#define vo    edx
#define plane ecx
;
;    if (plane & 1) {
;            din  = vi[3] + vi[plane>>1];   // in->W  + in->Z;
;            dout = vo[3] + vo[plane>>1];   // out->W + out->Z;
;    } else {
;            din  = vi[3] - vi[plane>>1];   // in->W  - in->Z;
;            dout = vo[3] - vo[plane>>1];   // out->W - out->Z;
;    }
;
    mov     plane, (_plane$-_stackOff)[esp]
    mov     vi, (_vi$-_stackOff)[esp]

    add     plane, plane
    mov     vo, (_vo$-_stackOff)[esp]
    
    test    plane, 2
    jz      plane_even

//plane_odd:
    and     plane, 0fffffffch
    nop

    flds    IVert.W[vi]
    fadds   IVert.X[vi + plane]             ; din
    
    flds    IVert.W[vo]
    fadds   IVert.X[vo + plane]             ; dout
    jmp     plane_sync 

plane_even:
    flds    IVert.W[vi]                       ; din
    fsubs   IVert.X[vi + plane]

    flds    IVert.W[vo]
    fsubs   IVert.X[vo + plane]               ; dout

;   t = din / ( din - dout );
plane_sync:
    fsubr   st, st(1)                   ; di di-do
    fdivp   st(1), st                   ; t
;
;some clocks to spare here if you need 'em 
;
; Live registers: vi <eax> vo <edx> plane <ecx>
; Free registers: ebx, esi, edi, ebp
;
#undef vp
#define ip ebx
#define fo esi
#define fi edi
#define vm ebp
#define vp fi

    push    ebx
    push    esi
   
    push    edi
    push    ebp
;
;   always copy colors
;   use ecx (plane), will be reloaded below.
;
    mov     vm   , _vm$[esp]              ; clipped vertex
    mov     ebx  , IVert.r[vi]

    mov     ecx  , IVert.g[vi]
    mov     esi  , IVert.b[vi]

    mov     edi  , IVert.a[vi]
    mov     IVert.r[vm], ebx

    mov     IVert.g[vm], ecx
    mov     IVert.b[vm], esi

    mov     IVert.a[vm], edi
    mov     ecx, _plane$[esp]
;
;   planemask =  (1 << plane) -1
;   fi = vi->flags
;   fo = vo->flags
;
;   If a vertex is not clipped, its oow is valid, and s & t have already
;   been scaled by oow. So, we may need to "unproject" i.e, multiply s and t
;   by W before we interpolate in this clipping routine. 
;   fi and fo are used for this purpose.
;
;

    mov     ebx, 1
    mov     fo, IVert.flags[vo]

    shl     ebx, cl                     ; ecx = plane!!!
    mov     fi, IVert.flags[vi]

    mov     plane, ebx

    dec     plane                       ; (1 << plane) -1 is the plane mask
    
    mov     ebx, DWORD PTR __gl_contextArea
    fld     [ebx]__GLcontext.constants.one      ; t 1.0
    
    mov     ip, [ebx]__GLcontext.state.light.shadingModel
    fsub    st, st(1)                   ; t omt
;
;   mid->X   = t * out->X   + omt * in->X;
;   mid->Y   = t * out->Y   + omt * in->Y;
;   mid->Z   = t * out->Z   + omt * in->Z;
;   mid->W   = t * out->W   + omt * in->W;
;
    flds    IVert.X[vo]
    fmul    st, st(2)
    flds    IVert.X[vi]
    fmul    st, st(2)                   ; t omt xo xi

    flds    IVert.Y[vo]
    fmul    st, st(4)
    flds    IVert.Y[vi]
    fmul    st, st(4)                   ; t omt xo xi yo yi

    fxch    st(2)                       ; t omt xo yi yo xi
    faddp   st(3), st                   ; t omt X  yi yo

    flds    IVert.Z[vo]
    fmul    st, st(5)
    flds    IVert.Z[vi]
    fmul    st, st(5)                   ; t omt X  yi yo zo zi


    fxch    st(2)                       ; t omt X  yi zi zo yo
    faddp   st(3), st                   ; t omt X  Y  zi zo

    flds    IVert.W[vo]
    fmul    st, st(6)
    flds    IVert.W[vi]
    fmul    st, st(6)                   ; t omt X  Y  zi zo wo wi
    
    fxch    st(2)                       ; t omt X  Y  zi wi wo zo
    faddp   st(3), st                   ; t omt X  Y  Z  wi wo
    fxch    st(4)                       ; t omt wo Y  Z  wi X

    fstsp   IVert.X[vm]                       ; t omt wo Y  Z  wi
    faddp   st(3), st                   ; t omt W  Y  Z
    fstsp   IVert.Z[vm]
    fstsp   IVert.Y[vm]
    fstsp   IVert.W[vm]                       ; t omt
;
;   do this only if gc->primState.interpolate & 1
;   Otherwise, copy colors vrom *in -- note this has already been done above.
;
;   mid->r   = t * out->r   + omt * in->r;
;   mid->g   = t * out->g   + omt * in->g;
;   mid->b   = t * out->b   + omt * in->b;
;   mid->a   = t * out->a   + omt * in->a;
;
    test    ip, GL_SMOOTH
    jz      st_vo

    flds    IVert.r[vo]
    fmul    st, st(2)
    flds    IVert.r[vi]
    fmul    st, st(2)                   ; t omt ro ri

    flds    IVert.g[vo]
    fmul    st, st(4)
    flds    IVert.g[vi]
    fmul    st, st(4)                   ; t omt ro ri go gi

    fxch    st(2)                       ; t omt ro gi go ri
    faddp   st(3), st                   ; t omt R  gi go

    flds    IVert.b[vo]
    fmul    st, st(5)
    flds    IVert.b[vi]
    fmul    st, st(5)                   ; t omt R  gi go bo bi

    fxch    st(2)                       ; t omt R  gi bi bo go
    faddp   st(3), st                   ; t omt R  G  bi bo

    flds    IVert.a[vo]
    fmul    st, st(6)
    flds    IVert.a[vi]
    fmul    st, st(6)                   ; t omt R  G  bi bo ao ai
    
    fxch    st(2)                       ; t omt R  G  bi ai ao bo
    faddp   st(3), st                   ; t omt R  G  B  ai ao
    fxch    st(4)                       ; t omt ao G  B  ai R

    fstsp   IVert.r[vm]                       ; t omt ao G  B  ai
    faddp   st(3), st                   ; t omt A  G  B
    fstsp   IVert.b[vm]
    fstsp   IVert.g[vm]
    fstsp   IVert.a[vm]                       ; t omt

;
;   iw = (in ->flags) ? 1.0f : in ->W;
;   ow = (out->flags) ? 1.0f : out->W;
;
;   mid->sow0 = t * out->sow0 * ow + omt * in->sow0 * iw; // * in->W;
;   mid->tow = t * out->tow * ow + omt * in->tow * iw; // * in->W;
;
st_vo:
    flds    IVert.t0_sow[vo]                 
    fmul    st, st(2)
    flds    IVert.t0_tow[vo]
    fmul    st, st(3)
    flds    IVert.t0_oow[vo]
    fmul    st, st(4)			; t omt so to qo

    test    fo, fo
    jnz     st_vi
    flds    IVert.W[vo]
    fmul    st(3), st
    fmul    st(2), st
    fmulp   st(1), st                   ; t omt so to qo

st_vi:
    flds    IVert.t0_sow[vi]
    fmul    st, st(4)
    flds    IVert.t0_tow[vi]
    fmul    st, st(5)
    flds    IVert.t0_oow[vi]
    fmulp   st(6), st			; t qi so to qo si ti

    test    fi, fi
    jnz     st_add
    flds    IVert.W[vi]
    fmul    st(6), st
    fmul    st(2), st
    fmulp   st(1), st                   ; t qi so to qo si ti 
    
st_add:
    fxch    st(2)                       ; t qi so to ti si qo
    faddp   st(5), st                   ; t Q so to ti si 
    faddp   st(3), st                   ; t Q S to ti 
    faddp   st(1), st			; t Q S T
    fxch    st(2)                       ; t T S Q
    fstsp   IVert.t0_oow[vm]
    fstsp   IVert.t0_sow[vm]
    fstsp   IVert.t0_tow[vm]		; t

    fstsp    IVert.ooz[vm]		;

; Recompute new vertex's clip code.
#undef iw
#undef iy
#undef ix
#undef iz
#define cc eax
#define iw ebx
#define mm ecx
#define iz edx
#define iy esi
#define ix edi
#define vm ebp

    push    plane
    nop

    mov     iw , IVert.W[vm]  ;  1;
    mov     cc , CC_BEFORE

    add     iw , iw     ;  2;
    jc      clipWsync

    xor     cc , cc
    mov     iz, IVert.Z[vm]   ;  3;
    
    mov     iy, IVert.Y[vm]   ;  4;
    mov     ix, IVert.X[vm]

    xor     mm, mm      ;  5;
    add     iz, iz

    adc     cc, cc      ;  6;
    cmp     iw, iz

    adc     mm, mm      ;  7;
    add     iy, iy

    adc     cc, cc      ;  8;
    cmp     iw, iy

    adc     mm, mm      ;  9;
    add     ix, ix

    adc     cc, cc      ; 10;
    cmp     iw, ix

    adc     mm, mm      ; 11;

    mov     byte ptr (IVert.flags+1)[vm], cl
    mov     byte ptr (IVert.flags+2)[vm], al

    jz      clipWpass
    
    mov     eax, _ccsign[cc*4]       ; 12; AGI if fail
    mov     ecx, _ccmask[mm*4]       ; 12;

clipWpass:
    and     cc, mm      ; 13;
    nop

clipWsync:
    mov     ebx, DWORD PTR __gl_contextArea
    pop     plane
    lea     vp, [ebx]__GLcontext.state.viewport

    and     cc, plane
    jnz     clipDone

;   /* if new vertex is in, then it will definitely be 
;      drawn, time to project */
;   if ( !mid->flags ) {
;       float   *vp     = &_gc.viewport[0];
;       
;       mid->oow = 1.0f / mid->W;
;       mid->x   = mid->X   * mid->oow * vp[VP_XSCALE] + vp[VP_XOFFSET];
;       mid->y   = mid->Y   * mid->oow * vp[VP_YSCALE] + vp[VP_YOFFSET];
;       mid->ooz = mid->Z   * mid->oow * vp[VP_ZSCALE] + vp[VP_ZOFFSET];
;       mid->sow0 = mid->sow0 * mid->oow;
;       mid->tow = mid->tow * mid->oow;
;   }
;   else {
;       mid->oow = 1.0f;
;   }
;}
; projection
    flds    IVert.W[vm]     ;   ; W
    fdivrs  [ebx]__GLcontext.constants.one      ; oow

    flds    IVert.t0_sow[vm]		;  1; oow S
    fmul    st(0), st(1)		;  2; oow s
    flds    IVert.t0_tow[vm]		;  3; oow s T
    fmul    st(0), st(2)		;  4; oow s t
    flds    IVert.t0_oow[vm]		;   ; oow s t Q
    fmul    st(0), st(3)		;   ; oow s t q
    flds    IVert.Z[vm]			;  5; oow s t q Z
    fmul    st(0), st(4)		;  6; oow s t q z
    flds    IVert.Y[vm]			;  7; oow s t q z Y
    fmul    st(0), st(5)		;  8; oow s t q z y
    flds    IVert.X[vm]			;  9; oow s t q z y X
    fmul    st(0), st(6)		; 10; oow s t q z y x
    
    fxch    st(5)			;   ; oow x t q z y s 
    fstsp   IVert.t0_sow[vm]		; 11; oow x t q z y
    fxch    st(3)			;   ; oow x y q z t
    fstsp   IVert.t0_tow[vm]		; 12; oow x y q z
    fxch    st(1)			;   ; oow x y z q
    fstsp   IVert.t0_oow[vm]		; 12; oow x y z
    
    flds    __GLviewport.zScale[vp]	; 13; oow x y z sz
    fmulp   st(1), st			; 14; oow x y z
    flds    __GLviewport.yScale[vp]	; 15; oow x y z sy
    fmulp   st(2), st			; 16; oow x y z
    flds    __GLviewport.xScale[vp]	; 17; oow x y z sx
    fmulp   st(3), st			; 18; oow x y z
    
    fadds   __GLviewport.zCenter[vp]	; 19; oow x y z
    fxch    st(2)			;   ; oow z y x
    fadds   __GLviewport.xCenter[vp]	; 20; oow z y x
    fxch    st(1)			;   ; oow z x y
    fadds   __GLviewport.yCenter[vp]	; 21; oow z x y
    fxch    st(2)			;   ; oow y x z
    
    fstsp   IVert.ooz[vm]		; 22; oow y x
    fstsp   IVert.x[vm]			; 24; oow y
    fstsp   IVert.y[vm]			; 26; oow
    fstp    st(0)
    flds    IVert.t0_oow[vm]
    fstsp   IVert.oow[vm]		; 28; 
    
clipDone:
    mov     IVert.flags[vm], cc
    pop     ebp

    pop     edi
    pop     esi

    pop     ebx
    ret
}
}

__declspec(naked) static void 
__glSSTIntersect_B(  IVert *in,  IVert *out,  IVert *mid, unsigned plane) {
        __asm {
#define _stackOff 16
#define _vi$    4+_stackOff
#define _vo$    8+_stackOff
#define _vm$    12+_stackOff
#define _plane$ 16+_stackOff

#define vi    eax
#define vo    edx
#define plane ecx
;
;    if (plane & 1) {
;            din  = vi[3] + vi[plane>>1];   // in->W  + in->Z;
;            dout = vo[3] + vo[plane>>1];   // out->W + out->Z;
;    } else {
;            din  = vi[3] - vi[plane>>1];   // in->W  - in->Z;
;            dout = vo[3] - vo[plane>>1];   // out->W - out->Z;
;    }
;
    mov     plane, (_plane$-_stackOff)[esp]
    mov     vi, (_vi$-_stackOff)[esp]

    add     plane, plane
    mov     vo, (_vo$-_stackOff)[esp]
    
    test    plane, 2
    jz      plane_even

//plane_odd:
    and     plane, 0fffffffch
    nop

    flds    IVert.W[vi]
    fadds   IVert.X[vi + plane]             ; din
    
    flds    IVert.W[vo]
    fadds   IVert.X[vo + plane]             ; dout
    jmp     plane_sync 

plane_even:
    flds    IVert.W[vi]                       ; din
    fsubs   IVert.X[vi + plane]

    flds    IVert.W[vo]
    fsubs   IVert.X[vo + plane]               ; dout

;   t = din / ( din - dout );
plane_sync:
    fsubr   st, st(1)                   ; di di-do
    fdivp   st(1), st                   ; t
;
;some clocks to spare here if you need 'em 
;
; Live registers: vi <eax> vo <edx> plane <ecx>
; Free registers: ebx, esi, edi, ebp
;

    push    ebx
    push    esi
   
    push    edi
    push    ebp

    mov     vm   , _vm$[esp]              ; clipped vertex
    mov     ecx, _plane$[esp]

;
;   planemask =  (1 << plane) -1
;   fi = vi->flags
;   fo = vo->flags
;
;   If a vertex is not clipped, its oow is valid, and s & t have already
;   been scaled by oow. So, we may need to "unproject" i.e, multiply s and t
;   by W before we interpolate in this clipping routine. 
;   fi and fo are used for this purpose.
;
;

    mov     ebx, 1
    mov     fo, IVert.flags[vo]

    shl     ebx, cl                     ; ecx = plane!!!
    mov     fi, IVert.flags[vi]

    mov     plane, ebx

    dec     plane                       ; (1 << plane) -1 is the plane mask
    
    mov     ebx, DWORD PTR __gl_contextArea
    fld     [ebx]__GLcontext.constants.one      ; t 1.0
    
    fsub    st, st(1)                   ; t omt
;
;   mid->X   = t * out->X   + omt * in->X;
;   mid->Y   = t * out->Y   + omt * in->Y;
;   mid->Z   = t * out->Z   + omt * in->Z;
;   mid->W   = t * out->W   + omt * in->W;
;
    flds    IVert.X[vo]
    fmul    st, st(2)
    flds    IVert.X[vi]
    fmul    st, st(2)                   ; t omt xo xi

    flds    IVert.Y[vo]
    fmul    st, st(4)
    flds    IVert.Y[vi]
    fmul    st, st(4)                   ; t omt xo xi yo yi

    fxch    st(2)                       ; t omt xo yi yo xi
    faddp   st(3), st                   ; t omt X  yi yo

    flds    IVert.Z[vo]
    fmul    st, st(5)
    flds    IVert.Z[vi]
    fmul    st, st(5)                   ; t omt X  yi yo zo zi

    fxch    st(2)                       ; t omt X  yi zi zo yo
    faddp   st(3), st                   ; t omt X  Y  zi zo

    flds    IVert.W[vo]
    fmul    st, st(6)
    flds    IVert.W[vi]
    fmul    st, st(6)                   ; t omt X  Y  zi zo wo wi
    
    fxch    st(2)                       ; t omt X  Y  zi wi wo zo
    faddp   st(3), st                   ; t omt X  Y  Z  wi wo
    fxch    st(4)                       ; t omt wo Y  Z  wi X

    fstsp   IVert.X[vm]                       ; t omt wo Y  Z  wi
    faddp   st(3), st                   ; t omt W  Y  Z
    fstsp   IVert.Z[vm]
    fstsp   IVert.Y[vm]
    fstsp   IVert.W[vm]                       ; t omt

;
;   iw = (in ->flags) ? 1.0f : in ->W;
;   ow = (out->flags) ? 1.0f : out->W;
;
;   mid->sow0 = t * out->sow0 * ow + omt * in->sow0 * iw; // * in->W;
;   mid->tow = t * out->tow * ow + omt * in->tow * iw; // * in->W;
;
//st_vo:
    flds    IVert.t0_sow[vo]                 
    fmul    st, st(2)
    flds    IVert.t0_tow[vo]
    fmul    st, st(3)                   ; t omt so to

    test    fo, fo
    jnz     st_vi
    flds    IVert.W[vo]
    fmul    st(2), st
    fmulp   st(1), st                   ; t omt so to

st_vi:
    flds    IVert.t0_sow[vi]
    fmul    st, st(3)
    flds    IVert.t0_tow[vi]
    fmul    st, st(4)                   ; t omt so to si ti

    test    fi, fi
    jnz     st_add
    flds    IVert.W[vi]
    fmul    st(2), st
    fmulp   st(1), st                   ; t omt so to si ti
    
st_add:
    fxch    st(1)                       ; t omt so to ti si
    faddp   st(3), st                   ; t omt S  to ti
    faddp   st(1), st                   ; t omt S  T
    fxch                                ; t omt T  S
    fstsp   IVert.t0_sow[vm]
    fstsp   IVert.t0_tow[vm]            ; t omt

// Second tex coord
    flds    IVert.t1_sow[vo]                 
    fmul    st, st(2)
    flds    IVert.t1_tow[vo]
    fmul    st, st(3)                   ; t omt so to

    test    fo, fo
    jnz     st_vi1
    flds    IVert.W[vo]
    fmul    st(2), st
    fmulp   st(1), st                   ; t omt so to

st_vi1:
    flds    IVert.t1_sow[vi]
    fmul    st, st(3)
    flds    IVert.t1_tow[vi]
    fmul    st, st(4)                   ; t omt so to si ti

    test    fi, fi
    jnz     st_add1
    flds    IVert.W[vi]
    fmul    st(2), st
    fmulp   st(1), st                   ; t omt so to si ti
    
st_add1:
    fxch    st(1)                       ; t omt so to ti si
    faddp   st(3), st                   ; t omt S  to ti
    faddp   st(1), st                   ; t omt S  T
    fxch                                ; t omt T  S
    fstsp   IVert.t1_sow[vm]
    fstsp   IVert.t1_tow[vm]            ; t omt

    flds    IVert.t0_oow[vo]
    fmul    st, st(2)
    flds    IVert.t1_oow[vo]		
    fmul    st, st(3)			; t omt q0o q1o
    
    test    fo, fo
    jnz     q_vi
    flds    IVert.W[vo]
    fmul    st(2), st
    fmulp   st(1), st			; t omt q0o q1o

q_vi:
    flds    IVert.t0_oow[vi]
    fmul    st, st(3)			
    flds    IVert.t1_oow[vi]
    fmul    st, st(4)			; t omt q0o q1o q0i q1i

    test    fi, fi
    jnz     q_add
    flds    IVert.W[vi]
    fmul    st(2), st
    fmulp   st(1), st			; t omt q0o q1o q0i q1i

q_add:
    fxch    st(1)			; t omt q0o q1o q1i q0i
    faddp   st(3), st			; t omt Q0 q1o q1i
    faddp   st(1), st			; t omt Q0 Q1
    fxch				; t omt Q1 Q0
    fstsp   IVert.t0_oow[vm]		; t omt Q1
    fstsp   IVert.t1_oow[vm]		; t omt

    fstp    st(0)                       ; 
    fstsp    IVert.ooz[vm];


; Recompute new vertex's clip code.

    push    plane
    nop

    mov     iw , IVert.W[vm]  ;  1;
    mov     cc , CC_BEFORE

    add     iw , iw     ;  2;
    jc      clipWsync

    xor     cc , cc
    mov     iz, IVert.Z[vm]   ;  3;
    
    mov     iy, IVert.Y[vm]   ;  4;
    mov     ix, IVert.X[vm]

    xor     mm, mm      ;  5;
    add     iz, iz

    adc     cc, cc      ;  6;
    cmp     iw, iz

    adc     mm, mm      ;  7;
    add     iy, iy

    adc     cc, cc      ;  8;
    cmp     iw, iy

    adc     mm, mm      ;  9;
    add     ix, ix

    adc     cc, cc      ; 10;
    cmp     iw, ix

    adc     mm, mm      ; 11;

    mov     byte ptr (IVert.flags+1)[vm], cl
    mov     byte ptr (IVert.flags+2)[vm], al

    jz      clipWpass
    
    mov     eax, _ccsign[cc*4]       ; 12; AGI if fail
    mov     ecx, _ccmask[mm*4]       ; 12;

clipWpass:
    and     cc, mm      ; 13;
    nop

clipWsync:
    mov     ebx, DWORD PTR __gl_contextArea
    pop     plane
    lea     vp, [ebx]__GLcontext.state.viewport

    and     cc, plane
    jnz     clipDone

;   /* if new vertex is in, then it will definitely be 
;      drawn, time to project */
;   if ( !mid->flags ) {
;       float   *vp     = &_gc.viewport[0];
;       
;       mid->oow = 1.0f / mid->W;
;       mid->x   = mid->X   * mid->oow * vp[VP_XSCALE] + vp[VP_XOFFSET];
;       mid->y   = mid->Y   * mid->oow * vp[VP_YSCALE] + vp[VP_YOFFSET];
;       mid->ooz = mid->Z   * mid->oow * vp[VP_ZSCALE] + vp[VP_ZOFFSET];
;       mid->sow0 = mid->sow0 * mid->oow;
;       mid->tow = mid->tow * mid->oow;
;   }
;   else {
;       mid->oow = 1.0f;
;   }
;}
; projection
    flds    IVert.W[vm]     ;   ; W
    fdivrs  [ebx]__GLcontext.constants.one      ; oow

    flds    IVert.t0_sow[vm]        ;  1; oow S0
    fmul    st(0), st(1)            ;  2; oow s0
    flds    IVert.t0_tow[vm]        ;  3; oow s0 T0
    fmul    st(0), st(2)            ;  4; oow s0 t0
    flds    IVert.t1_sow[vm]        ;  5; oow s0 t0 S1
    fmul    st(0), st(3)            ;  6; oow s0 t0 s1
    flds    IVert.t1_tow[vm]        ;  7; oow s0 t0 s1 T1
    fmul    st(0), st(4)            ;  8; oow s0 t0 s1 t1
    flds    IVert.Z[vm]             ;  9; oow s0 t0 s1 t1 Z
    fmul    st(0), st(5)            ; 10; oow s0 t0 s1 t1 z
    flds    IVert.Y[vm]             ; 11; oow s0 t0 s1 t1 z Y
    fmul    st(0), st(6)            ; 12; oow s0 t0 s1 t1 z y
    flds    IVert.X[vm]             ; 13; oow s0 t0 s1 t1 z y X
    fmul    st(0), st(7)            ; 14; oow s0 t0 s1 t1 z y x
    
    fxch    st(6)                   ;   ; oow x t0 s1 t1 z y s0
    fstsp   IVert.t0_sow[vm]        ; 15; oow x t0 s1 t1 z y
    fxch    st(4)                   ;   ; oow x y s1 t1 z t0
    fstsp   IVert.t0_tow[vm]        ; 17; oow x y s1 t1 z

    flds    IVert.t0_oow[vm]		; oow x y s1 t1 z Q0
    fmul    st(0), st(6)		; oow x y s1 t1 z q0
    flds    IVert.t1_oow[vm]		; oow x y s1 t1 z q0 Q1
    fmul    st(0), st(7)		; oow x y s1 t1 z q0 q1
    fxch    st(4)                   ;   ; oow x y q1 t1 z q0 s1
    fstsp   IVert.t1_sow[vm]        ; 19; oow x y q1 t1 z q0
    fxch    st(2)			; oow x y q1 q0 z t1
    fstsp   IVert.t1_tow[vm]        ; 21; oow x y q1 q0 z
    fxch    st(2)			; oow x y z q0 q1
    fstsp   IVert.t1_oow[vm]		; oow x y z q0
    fstsp   IVert.t0_oow[vm]		; oow x y z
    
    flds    __GLviewport.zScale[vp]	; 13; oow x y z sz
    fmulp   st(1), st			; 14; oow x y z
    flds    __GLviewport.yScale[vp]	; 15; oow x y z sy
    fmulp   st(2), st			; 16; oow x y z
    flds    __GLviewport.xScale[vp]	; 17; oow x y z sx
    fmulp   st(3), st			; 18; oow x y z
    
    fadds   __GLviewport.zCenter[vp]	; 19; oow x y z
    fxch    st(2)			;   ; oow z y x
    fadds   __GLviewport.xCenter[vp]	; 20; oow z y x
    fxch    st(1)			;   ; oow z x y
    fadds   __GLviewport.yCenter[vp]	; 21; oow z x y
    fxch    st(2)			;   ; oow y x z
    
    fstsp   IVert.ooz[vm]         ; 22; oow y x
    fstsp   IVert.x[vm]           ; 24; oow y
    fstsp   IVert.y[vm]           ; 26; oow
    fstp    st(0)
    flds    IVert.t0_oow[vm]
    fstsp   IVert.oow[vm]         ; 28; 
    
clipDone:
    mov     IVert.flags[vm], cc
    pop     ebp

    pop     edi
    pop     esi

    pop     ebx
    ret
}
}

#undef _gc_
#undef v
#undef vo
#undef mat
#undef viewport
#undef xScale 
#undef xOffset
#undef yScale 
#undef yOffset
#undef zScale 
#undef zOffset
#undef ps
#undef vp
#undef iw
#undef ix
#undef iy
#undef iz

#endif // __GL_USE_INTEL_ASM

void 
__glSSTClipAndDraw( unsigned plane, IVert *a, IVert *b, IVert *c) 
{
    int  cc = 0;
    int  pmask = 1 << plane;
    IVert x, y;

    if ((a->flags | b->flags | c->flags) == 0) {
        grDrawTriangle((GrVertex*)a, (GrVertex*)b, (GrVertex*)c);
        return;
    }

    cc = (( ((a->flags & pmask) << 2) | 
            ((b->flags & pmask) << 1) | 
            ((c->flags & pmask)     )   ) >> plane);

    switch(cc) {
         // abc
    case 0:  // 000
        __glSSTClipAndDraw(plane-1, a, b, c);
        break;

    case 1:  // 001
        __glSSTIntersect(a, c, &x, plane);
        __glSSTIntersect(b, c, &y, plane);
        __glSSTClipAndDraw(plane-1, a,  b, &y);
        __glSSTClipAndDraw(plane-1, a, &y, &x);
        break;

    case 2:  // 010
        __glSSTIntersect(a, b, &x, plane);
        __glSSTIntersect(c, b, &y, plane);
        __glSSTClipAndDraw(plane-1,  a, &x, &y);
        __glSSTClipAndDraw(plane-1,  a, &y,  c);
        break;


    case 3:  // 011
        __glSSTIntersect(a, b, &x, plane);
        __glSSTIntersect(a, c, &y, plane);
        __glSSTClipAndDraw(plane-1, a, &x, &y);
        break;

    case 4:  // 100
        __glSSTIntersect(b, a, &x, plane);
        __glSSTIntersect(c, a, &y, plane);
        __glSSTClipAndDraw(plane-1,  b,  c, &y);
        __glSSTClipAndDraw(plane-1,  b, &y, &x);
        break;

    case 5:  // 101
        __glSSTIntersect(b, a, &x, plane);
        __glSSTIntersect(b, c, &y, plane);
        __glSSTClipAndDraw(plane-1, b, &y, &x);
        break;

    case 6:  // 110
        __glSSTIntersect(c, a, &x, plane);
        __glSSTIntersect(c, b, &y, plane);
        __glSSTClipAndDraw(plane-1, c, &x, &y);
        break;

    case 7:  // 111
        return;
    }
}

void 
__glSSTClipAndDraw_B( unsigned plane, IVert *a, IVert *b, IVert *c) 
{
    int  cc = 0;
    int  pmask = 1 << plane;
    IVert x, y;

    if ((a->flags | b->flags | c->flags) == 0) {
        grDrawTriangle((GrVertex*)a, (GrVertex*)b, (GrVertex*)c);
        return;
    }

    cc = (( ((a->flags & pmask) << 2) | 
            ((b->flags & pmask) << 1) | 
            ((c->flags & pmask)     )   ) >> plane);

    switch(cc) {
         // abc
    case 0:  // 000
        __glSSTClipAndDraw_B(plane-1, a, b, c);
        break;

    case 1:  // 001
        __glSSTIntersect_B(a, c, &x, plane);
        __glSSTIntersect_B(b, c, &y, plane);
        __glSSTClipAndDraw_B(plane-1, a,  b, &y);
        __glSSTClipAndDraw_B(plane-1, a, &y, &x);
        break;

    case 2:  // 010
        __glSSTIntersect_B(a, b, &x, plane);
        __glSSTIntersect_B(c, b, &y, plane);
        __glSSTClipAndDraw_B(plane-1,  a, &x, &y);
        __glSSTClipAndDraw_B(plane-1,  a, &y,  c);
        break;


    case 3:  // 011
        __glSSTIntersect_B(a, b, &x, plane);
        __glSSTIntersect_B(a, c, &y, plane);
        __glSSTClipAndDraw_B(plane-1, a, &x, &y);
        break;

    case 4:  // 100
        __glSSTIntersect_B(b, a, &x, plane);
        __glSSTIntersect_B(c, a, &y, plane);
        __glSSTClipAndDraw_B(plane-1,  b,  c, &y);
        __glSSTClipAndDraw_B(plane-1,  b, &y, &x);
        break;

    case 5:  // 101
        __glSSTIntersect_B(b, a, &x, plane);
        __glSSTIntersect_B(b, c, &y, plane);
        __glSSTClipAndDraw_B(plane-1, b, &y, &x);
        break;

    case 6:  // 110
        __glSSTIntersect_B(c, a, &x, plane);
        __glSSTIntersect_B(c, b, &y, plane);
        __glSSTClipAndDraw_B(plane-1, c, &x, &y);
        break;

    case 7:  // 111
        return;
    }
}

#ifdef __GL_USE_INTEL_ASM

#define ps __GLcontext.primState

#define _gc_ edi
#define v    eax
#define vo   ebp
#define mat  ecx
#define code esi
#define iw   edx
#define ix   eax
#define iy   ebx
#define iz   ecx
#define vp   ecx
#undef  _v$
#define _v$  20

__declspec( naked ) 
void APIENTRY __glsstim_Vertex3fv_L(const GLfloat *v) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     _gc_,  DWORD PTR __gl_contextArea
        mov     mat,  DWORD PTR [_gc_]__GLcontext.transform.modelView
        add     mat, 168
        mov     v,  _v$[esp]
        mov     vo, ps.vC[_gc_]

        sub     esp, 180

//    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12]; 
//    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13];
//    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
//    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15];
        flds    m15[mat]
        flds    m14[mat]
        flds    m13[mat]
        flds    m12[mat]

        flds    inZ[v]
        fmuls   m11[mat]
        flds    inZ[v]
        fmuls   m10[mat]
        flds    inZ[v]
        fmuls   m09[mat]
        flds    inZ[v]
        fmuls   m08[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inY[v]
        fmuls   m07[mat]
        flds    inY[v]
        fmuls   m06[mat]
        flds    inY[v]
        fmuls   m05[mat]
        flds    inY[v]
        fmuls   m04[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inX[v]
        fmuls   m03[mat]
        flds    inX[v]
        fmuls   m02[mat]
        flds    inX[v]
        fmuls   m01[mat]
        flds    inX[v]
        fmuls   m00[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        fxch    st(3)
        fstsp   [vo]IVert.W
        fstsp   [vo]IVert.Y
        fstsp   [vo]IVert.Z
        fstsp   [vo]IVert.X

        flds    [_gc_]__GLcontext.constants.one
           
        mov     code, CC_BEFORE
        mov     iw, IVert.W[vo]

        add     iw, iw
        jc      AfterClip

        // W is positive

        fdivs   IVert.W[vo]

        mov     iz, IVert.Z[vo]
        mov     iy, IVert.Y[vo]

        mov     ix, IVert.X[vo]
        add     iz, iz

        adc     code, code
        add     iy, iy

        adc     code, code
        add     ix, ix

        adc     code, code
        cmp     iw, iz

        adc     iz, iz
        cmp     iw, iy

        adc     iz, iz
        cmp     iw, ix

        adc     iz, iz
        mov     iy, code

        and     iy, 7
        and     iz, 7

        mov     code, 0
        jz      AfterClip

        mov     iy, _ccsign[iy*4]
        mov     code, _ccmask[iz*4]

        and     code, iy
        nop
AfterClip:
        mov     eax, __GLcontext.state.current.normal.x[_gc_]
        mov     ebx, __GLcontext.state.current.normal.y[_gc_]
        mov     ecx, __GLcontext.state.current.normal.z[_gc_]
        mov     edx, __GLcontext.state.current.normal.w[_gc_]
        mov     __GLvertex.normal.x[esp], eax
        mov     __GLvertex.normal.y[esp], ebx
        mov     __GLvertex.normal.z[esp], ecx
        mov     __GLvertex.normal.w[esp], edx

        mov     mat, DWORD PTR [_gc_]__GLcontext.transform.modelView
        add     mat, 84

        lea     v, __GLvertex.normal.x[esp]
      
        flds    inZ[v]     // z
        fmuls   m10[mat]   // Zz
        flds    inZ[v]     // Zz z
        fmuls   m09[mat]   // Zz Zy
        flds    inZ[v]     // Zz Zy z
        fmuls   m08[mat]   // Zz Zy Zx 

        flds    inY[v]     // Zz Zy Zx y
        fmuls   m06[mat]   // Zz Zy Zx Yz
        flds    inY[v]     // Zz Zy Zx Yz y
        fmuls   m05[mat]   // Zz Zy Zx Yz Yy
        flds    inY[v]     // Zz Zy Zx Yz Yy y
        fmuls   m04[mat]   // Zz Zy Zx Yz Yy Yx

        fxch    st(2)      // Zz Zy Zx Yx Yy Yz
        faddp   st(5), st  // ZYz Zy Zx Yx Yy
        faddp   st(3), st  // ZYz ZYy Zx Yx
        faddp   st(1), st  // ZYz ZYy ZYx

        flds    inX[v]     // ZYz ZYy ZYx x
        fmuls   m02[mat]   // ZYz ZYy ZYx Xz
        flds    inX[v]     // ZYz ZYy ZYx Xz x
        fmuls   m01[mat]   // ZYz ZYy ZYx Xz Xy
        flds    inX[v]     // ZYz ZYy ZYx Xz Xy x
        fmuls   m00[mat]   // ZYz ZYy ZYx Xz Xy Xx

        fxch    st(2)      // ZYz ZYy ZYx Xx Xy Xz
        faddp   st(5), st  // ZYXz ZYy ZYx Xx Xy 
        faddp   st(3), st  // ZYXz ZYXy ZYx Xx
        faddp   st(1), st  // ZYXz ZYXy ZYXx

        fxch    st(2)      // ZYXx ZYXy ZYXz 
        fstsp   [v+8]      // ZYXx ZYXy
        fstsp   [v+4]      // ZYXx 
        fstsp   [v]        // empty
   /* end new code        */

        push    esp
        push    0
        push    _gc_
        call    __glFastCalcRGBColor
        add     esp, 12

        mov     eax, __GLvertex.colors[0].r[esp]
        mov     ebx, __GLvertex.colors[0].g[esp]
        mov     ecx, __GLvertex.colors[0].b[esp]
        mov     edx, __GLvertex.colors[0].a[esp]

        mov     IVert.r[vo], eax
        mov     IVert.g[vo], ebx
        mov     IVert.b[vo], ecx
        mov     IVert.a[vo], edx

        mov     eax, ps.tc[0].sBias[_gc_]
        mov     ebx, ps.tc[0].tBias[_gc_]
        mov     ecx, __GLcontext.state.current.texture[0].w[_gc_]
        add     eax, __GLcontext.state.current.texture[0].x[_gc_]
        add     ebx, __GLcontext.state.current.texture[0].y[_gc_]
        mov     IVert.t0_oow[vo], ecx
        mov     IVert.t0_sow[vo], eax
        mov     IVert.t0_tow[vo], ebx

        mov     IVert.flags[vo], code
        mov     ecx, ps.vState[_gc_]

        mov     ebx, ps.vB[_gc_]
        mov     eax, ps.vA[_gc_]

        mov     [esp-4], vo
        mov     [esp-8], ebx

        mov     [esp-12], eax
        mov     edx, _VSTEP_C[ecx]

        mov     ps.vB[_gc_], vo
        add     edx, vo

        mov     ps.vC[_gc_], edx
        mov     edx, _VSTEP_A[ecx]

        add     eax, edx
        mov     edx, _VDRAW[ecx]

        mov     ps.vA[_gc_], eax
        mov     ecx, _VNEXT[ecx]

        mov     ps.vState[_gc_], ecx
        lea     vp, __GLcontext.state.viewport[_gc_]

        test    code, code
        jnz     AfterViewport

        flds    IVert.t0_sow[vo]
        fmul    st(0), st(1)
        flds    IVert.t0_tow[vo]
        fmul    st(0), st(2)
        flds    IVert.Z[vo]
        fmul    st(0), st(3)
        flds    IVert.Y[vo]
        fmul    st(0), st(4)
        flds    IVert.X[vo]
        fmul    st(0), st(5)

        fxch    st(4)
        fstsp   IVert.t0_sow[vo]
        fxch    st(2)
        fstsp   IVert.t0_tow[vo]

        flds    __GLviewport.zScale[vp]
        fmulp   st(1), st
        flds    __GLviewport.yScale[vp]
        fmulp   st(2), st
        flds    __GLviewport.xScale[vp]
        fmulp   st(3), st

        fadds   __GLviewport.zCenter[vp]
        fxch    st(2)
        fadds   __GLviewport.xCenter[vp]
        fxch    st(1)
        fadds   __GLviewport.yCenter[vp]
        fxch    st(2)

        fstsp   IVert.ooz[vo]
        fstsp   IVert.x[vo]
        fstsp   IVert.y[vo]

AfterViewport:
        fstsp   IVert.oow[vo]

        test    edx, edx
        jnz     Draw

        add     esp, 180

        pop     ebx
        pop     esi

        pop     edi
        pop     ebp

        ret     4
Draw:
        mov     eax, [esp -12]
        mov     ebx, [esp - 8]

        test    edx, 2
        jz      noSwap

        mov     [esp-12], ebx
        mov     [esp-8],eax
noSwap:
        mov     eax, IVert.flags[eax]
        add     esp, -12

        or      code, eax
        mov     ebx, IVert.flags[ebx]

        or      code, ebx
        jnz     DrawClipped

        call    grDrawTriangle

        add     esp, 180

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
DrawClipped:
        push    5
        call    __glSSTClipAndDraw
        add     esp, 16

        add     esp, 180
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
    }

}

#else

void APIENTRY __glsstim_Vertex3fv_L(const GLfloat *v) {
    __GL_SETUP();
    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                                  // clip flags
    IVert       *s = ps->vC;                       // destination
    IVert       *savA, *savB, *savC;
    __GLvertex vx;

    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12]; 
    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13];
    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15];

    if ( ((int)GL_UINTCAST( s->W )) <= 0 ) {
        s->oow = 1.0f;
        f = CC_BEFORE; 
    } else {
        s->oow = 1.0f / s->W;
        
        /* 
         * Everything below here till projection ought to be free because of
         * the oow computation above. It should use only integer arithmetic!
         */

        /* Clip codes */
        f  = (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Z) >> 31)+1);
             
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Y) >> 31)+1);
    
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->X) >> 31)+1);
    }

    /* Save away clip code */
    s->flags      = f;

    /* Copy normal */
    vx.normal.x = gc->state.current.normal.x;
    vx.normal.y = gc->state.current.normal.y;
    vx.normal.z = gc->state.current.normal.z;
    vx.normal.w = 0;

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[0].x) + gc->primState.tc[0].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[0].y) + gc->primState.tc[0].tBias;
    s->t0_oow = gc->state.current.texture[0].w;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;
    savC = ps->vC;

    /* Update new state */
    ps->vA     = (IVert *) (((char *)ps->vA) + _VSTEP_A[ps->vState >> 2]);
    ps->vB     = ps->vC;
    ps->vC     = (IVert *) (((char *)ps->vC) + _VSTEP_C[ps->vState >> 2]);
    vdraw      = _VDRAW  [ps->vState >> 2];
    ps->vState = _VNEXT  [ps->vState >> 2];

    {
        __GLtransform *tr = gc->transform.modelView;

        (*tr->inverseTranspose.xf3)(&vx.normal,
                &vx.normal.x, &tr->inverseTranspose);
        (*gc->procs.calcColor)(gc, __GL_FRONTFACE, &vx);
        s->r = vx.colors[__GL_FRONTFACE].r;            
        s->g = vx.colors[__GL_FRONTFACE].g;            
        s->b = vx.colors[__GL_FRONTFACE].b;            
        s->a = vx.colors[__GL_FRONTFACE].a;            
    }

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * s->oow * vp->xScale + vp->xCenter;
        s->y   = s->Y   * s->oow * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * s->oow * vp->zScale + vp->zCenter;
        s->t0_sow = s->t0_sow * s->oow;
        s->t0_tow = s->t0_tow * s->oow;
        s->t0_oow = s->t0_oow * s->oow;
        s->oow = s->t0_oow;
    }

    if (vdraw) {
       if (vdraw & 2) { IVert *t = savA; savA = savB; savB = t; /* swap */}

       if (!(f | savA->flags | savB->flags)) {
          grDrawTriangle((GrVertex*) savA, (GrVertex*) savB, (GrVertex*) savC);
       } else {
          __glSSTClipAndDraw(SHIFT_CC_BEFORE, savA, savB, savC);
       }
    }
}

#endif


static void __glSSTClipAndDrawAALine( unsigned plane, IVert *a, IVert *b );

static int _VDRAW_LINE[] = {
    0, 1,  // GL_LINES
    0, 1   // GL_LINE_STRIP
};

static int _VNEXT_LINE[] = {
    1, 0,  // GL_LINES
    3, 3   // GL_LINE_STRIP
};

void APIENTRY __glsstim_Vertex3fv_AALine(const GLfloat *v);

void APIENTRY __glsstim_Vertex2fv_AALine(const GLfloat *v) {
    float nv[3];
    
    nv[0] = v[0];
    nv[1] = v[1];
    nv[2] = 0.0f;
    __glsstim_Vertex3fv_AALine( nv );
}

void APIENTRY __glsstim_Vertex4fv_AALine(const GLfloat *v) {
    __glsstim_Vertex3fv_AALine( v );
}

#ifdef __GL_USE_INTEL_ASM


__declspec(naked)
void APIENTRY __glsstim_Vertex3fv_AALine(const GLfloat *v) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     _gc_,  DWORD PTR __gl_contextArea
        mov     mat,  DWORD PTR [_gc_]__GLcontext.transform.modelView
        add     mat, 168
        mov     v,  _v$[esp]
        mov     vo, ps.vB[_gc_]

//    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12]; 
//    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13];
//    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
//    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15];
        flds    m15[mat]
        flds    m14[mat]
        flds    m13[mat]
        flds    m12[mat]

        flds    inZ[v]
        fmuls   m11[mat]
        flds    inZ[v]
        fmuls   m10[mat]
        flds    inZ[v]
        fmuls   m09[mat]
        flds    inZ[v]
        fmuls   m08[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inY[v]
        fmuls   m07[mat]
        flds    inY[v]
        fmuls   m06[mat]
        flds    inY[v]
        fmuls   m05[mat]
        flds    inY[v]
        fmuls   m04[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        flds    inX[v]
        fmuls   m03[mat]
        flds    inX[v]
        fmuls   m02[mat]
        flds    inX[v]
        fmuls   m01[mat]
        flds    inX[v]
        fmuls   m00[mat]

        fxch    st(3)
        faddp   st(7), st
        faddp   st(4), st
        faddp   st(4), st
        faddp   st(1), st

        fxch    st(3)
        fstsp   [vo]IVert.W
        fstsp   [vo]IVert.Y
        fstsp   [vo]IVert.Z
        fstsp   [vo]IVert.X

        flds    [_gc_]__GLcontext.constants.one
           
        mov     code, CC_BEFORE
        mov     iw, IVert.W[vo]

        add     iw, iw
        jc      AfterClip

        // W is positive

        fdivs   IVert.W[vo]

        mov     iz, IVert.Z[vo]
        mov     iy, IVert.Y[vo]

        mov     ix, IVert.X[vo]
        add     iz, iz

        adc     code, code
        add     iy, iy

        adc     code, code
        add     ix, ix

        adc     code, code
        cmp     iw, iz

        adc     iz, iz
        cmp     iw, iy

        adc     iz, iz
        cmp     iw, ix

        adc     iz, iz
        mov     iy, code

        and     iy, 7
        and     iz, 7

        mov     code, 0
        jz      AfterClip

        mov     iy, _ccsign[iy*4]
        mov     code, _ccmask[iz*4]

        and     code, iy
        nop
AfterClip:
        mov     eax, __GLcontext.state.current.color.r[_gc_]
        mov     ebx, __GLcontext.state.current.color.g[_gc_]
        mov     ecx, __GLcontext.state.current.color.b[_gc_]
        mov     edx, __GLcontext.state.current.color.a[_gc_]
        mov     IVert.r[vo], eax
        mov     IVert.g[vo], ebx
        mov     IVert.b[vo], ecx
        mov     IVert.a[vo], edx

        mov     eax, ps.tc[0].sBias[_gc_]
        mov     ebx, ps.tc[0].tBias[_gc_]
        add     eax, __GLcontext.state.current.texture[0].x[_gc_]
        add     ebx, __GLcontext.state.current.texture[0].y[_gc_]
        mov     IVert.t0_sow[vo], eax
        mov     IVert.t0_tow[vo], ebx

        mov     IVert.flags[vo], code
        mov     ecx, ps.vState[_gc_]

        mov     ebx, ps.vB[_gc_]
        mov     eax, ps.vA[_gc_]

        mov     edx, _VDRAW_LINE[ecx*4]
        mov     ecx, _VNEXT_LINE[ecx*4]
   
        mov     ps.vA[_gc_], ebx
        mov     ps.vB[_gc_], eax

        mov     ps.vState[_gc_], ecx
        lea     vp, __GLcontext.state.viewport[_gc_]

#if 0

        mov     [esp-4], vo
        mov     [esp-8], ebx

        mov     [esp-12], eax
        mov     edx, _VSTEP_C[ecx]

        mov     ps.vB[_gc_], vo
        add     edx, vo

        mov     ps.vC[_gc_], edx
        mov     edx, _VSTEP_A[ecx]

        add     eax, edx
        mov     edx, _VDRAW[ecx]

        mov     ps.vA[_gc_], eax
        mov     ecx, _VNEXT[ecx]

        mov     ps.vState[_gc_], ecx
#endif
//
        test    code, code
        jnz     AfterViewport

        flds    IVert.t0_sow[vo]
        fmul    st(0), st(1)
        flds    IVert.t0_tow[vo]
        fmul    st(0), st(2)
        flds    IVert.Z[vo]
        fmul    st(0), st(3)
        flds    IVert.Y[vo]
        fmul    st(0), st(4)
        flds    IVert.X[vo]
        fmul    st(0), st(5)

        fxch    st(4)
        fstsp   IVert.t0_sow[vo]
        fxch    st(2)
        fstsp   IVert.t0_tow[vo]

        flds    __GLviewport.zScale[vp]
        fmulp   st(1), st
        flds    __GLviewport.yScale[vp]
        fmulp   st(2), st
        flds    __GLviewport.xScale[vp]
        fmulp   st(3), st

        fadds   __GLviewport.zCenter[vp]
        fxch    st(2)
        fadds   __GLviewport.xCenter[vp]
        fxch    st(1)
        fadds   __GLviewport.yCenter[vp]
        fxch    st(2)

        fstsp   IVert.ooz[vo]
        fstsp   IVert.x[vo]
        fstsp   IVert.y[vo]

AfterViewport:
        fstsp   IVert.oow[vo]

        test    edx, edx
        jnz     Draw

        pop     ebx
        pop     esi

        pop     edi
        pop     ebp

        ret     4
Draw:
        mov     [esp-8], ebx
        mov     [esp-4], eax

        mov     eax, IVert.flags[eax]
        mov     ebx, IVert.flags[ebx]

        add     esp, -8
        or      eax, ebx
              
        jnz     DrawClipped
        call    grAADrawLine

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
DrawClipped:
        push    5
        call    __glSSTClipAndDrawAALine
        add     esp, 12
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret     4
    }
}

#else

void APIENTRY __glsstim_Vertex3fv_AALine(const GLfloat *v) {
    __GL_SETUP();

    float     *m  = (float *)&gc->transform.modelView->mvp;
    __GLviewport     *vp = &gc->state.viewport;
    SSTPrimState *ps = &gc->primState;

    int         f, vdraw;                          // clip flags
    IVert       *s = ps->vB;                       // destination
    IVert       *savA, *savB;

    s->X = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12]; 
    s->Y = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13];
    s->Z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
    s->W = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15];

    if ( ((int)GL_UINTCAST( s->W )) <= 0 ) {
        s->oow = 1.0f;
        f = CC_BEFORE; 
    } else {
        s->oow = 1.0f / s->W;
        
        /* 
         * Everything below here till projection ought to be free because of
         * the oow computation above. It should use only integer arithmetic!
         */

        /* Clip codes */
        f  = (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Z) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Z) >> 31)+1);
             
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->Y) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->Y) >> 31)+1);
    
        f <<= 2;
        f += (((int)(GL_UINTCAST(s->W)-(GL_UINTCAST(s->X) & 0x7fffffff)))>>31) &
             ((GL_UINTCAST(s->X) >> 31)+1);
    }

    /* Save away clip code */
    s->flags      = f;

    /* Copy colors, scale and clamp to 0..255 */
    s->r   = gc->state.current.color.r;
    s->g   = gc->state.current.color.g;
    s->b   = gc->state.current.color.b;
    s->a   = gc->state.current.color.a;

    /* Copy scaled texture coords */
    GL_UINTCAST(s->t0_sow) = GL_UINTCAST(gc->state.current.texture[0].x) + gc->primState.tc[0].sBias;
    GL_UINTCAST(s->t0_tow) = GL_UINTCAST(gc->state.current.texture[0].y) + gc->primState.tc[0].tBias;

    /* Make triangles, and save in list of triangles to be drawn later */
    savA = ps->vA;
    savB = ps->vB;

    /* Update new state */
    ps->vA = ps->vB;
    ps->vB = savA;

    vdraw      = _VDRAW_LINE [ ps->vState ];
    ps->vState = _VNEXT_LINE [ ps->vState ];

    /* If vertex is inside frustum, project and xform to device */
    if (!f) {
        s->x   = s->X   * s->oow * vp->xScale + vp->xCenter;
        s->y   = s->Y   * s->oow * vp->yScale + vp->yCenter;
        s->ooz = s->Z   * s->oow * vp->zScale + vp->zCenter;
        s->t0_sow = s->t0_sow * s->oow;
        s->t0_tow = s->t0_tow * s->oow;
    }

    if (vdraw) {
       if ( !( f | savA->flags ) ) {
          grAADrawLine( ( GrVertex * ) savA, ( GrVertex * ) savB );
       } else {
          __glSSTClipAndDrawAALine(SHIFT_CC_BEFORE, savA, savB );
       }
    }
}

#endif

static void __glSSTClipAndDrawAALine( unsigned plane, IVert *a, IVert *b ) {

    int  cc = 0;
    int  pmask = 1 << plane;
    IVert x;

    if ((a->flags | b->flags ) == 0) {
        grAADrawLine( ( GrVertex * ) a, ( GrVertex * ) b );
        return;
    }

    cc = ( (((a->flags & pmask) << 1) | 
            ((b->flags & pmask) << 0)) >> plane );

    switch(cc) {
         // ab
    case 0:  // 00
        __glSSTClipAndDrawAALine( plane-1, a, b );
        break;
    case 1:  // 01
        __glSSTIntersect(a, b, &x, plane);
        __glSSTClipAndDrawAALine(plane-1, a, &x);
        break;
    case 2:  // 10
        __glSSTIntersect(a, b, &x, plane);
        __glSSTClipAndDrawAALine( plane-1, b, &x );
        break;
    case 3:  // 11
        return;
    }
}

