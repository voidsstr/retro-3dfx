/*
** Copyright 1991-1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/
#include <windows.h>
#include "context.h"
#include "global.h"
#include <glide.h>
#include "sst_globals.h"

/* OPT 0.1.5 overbright (vertex-double mechanism): gate published by
** __glSSTLoadCombineFunction (SST/SST_TEX.C) when the collapsed Q3 world
** path -- two TMUs, texEnv0==GL_MODULATE && texEnv1==GL_MODULATE -- is
** active.  Q3 world vertex color is identityLight = 0.5 (r_overBrightBits=1)
** and relies on a free 2x it normally gets from the 2-pass lightmap blend
** (GL_DST_COLOR,GL_SRC_COLOR); our single-pass iterated*T0*T1 collapse has
** no such doubling, so we DOUBLE the iterated RGB here (clamp 255), feeding
** the legacy 1x combine 2*0.5=1.0.  The grColorCombineExt output-shift 2x
** was proven a no-op on live Napalm hardware -- do not resurrect it.
** Alpha is deliberately left unscaled. */
extern int __glSSTOverbright2xVtx;

/* OPT 0.1.6: NEUTRALIZED.  Live-hardware testing proved Q3's world never
** reaches these immediate-fill procs -- glDrawElements world geometry is
** compiled through GLCORE/S_VARRAY.C CompileElementsIndexed, so the 2x now
** lives THERE (on the compiled __GLvertex color, gated by the same
** __glSSTOverbright2xVtx).  The macro stays defined (call sites untouched)
** but must NOT double, or any path hitting both sites would get 4x. */
#define __GL_OB2X_CH(c) (c)

#define __GL_OB2X_TRI(vA,vB,vC)                                          \
    do {                                                                  \
        (void)&(vA); (void)&(vB); (void)&(vC);                            \
    } while (0)

/*
** The first three triangle drawers here handle the more general 
** cases of two sided lighting, so they have to do the signed area
** calculation in order to choose the right face color.  There are
** special versions of some cases down below, that handle one sided
** lighting.
*/

/*
** Generic triangle handling code.  This code is used when render mode
** is GL_RENDER and the polygon modes are not both fill.
*/
void __glSSTRenderTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                           __GLvertex *c)
{
    GLuint needs, modeFlags, faceNeeds;
    GLint ccw, colorFace, face;
    __GLfloat dxAC, dxBC, dyAC, dyBC, area;
    __GLvertex *pv;

    /* Compute signed area of the triangle */
#if __GL_SST_GLIDE_VTX
    dxAC = a->sst.x - c->sst.x;
    dxBC = b->sst.x - c->sst.x;
    dyAC = a->sst.y - c->sst.y;
    dyBC = b->sst.y - c->sst.y;
#else
    dxAC = a->window.x - c->window.x;
    dxBC = b->window.x - c->window.x;
    dyAC = a->window.y - c->window.y;
    dyBC = b->window.y - c->window.y;
#endif
    area = dxAC * dyBC - dxBC * dyAC;
    ccw = !(*(int *)&area >> 31);

    /*
    ** Figure out if face is culled or not.
    */
    face = gc->polygon.face[ccw];
    if (face == gc->polygon.cullFace) {
        /* Culled */
        return;
    }

    /*
    ** Pick face to use for coloring
    */
    modeFlags = gc->polygon.shader.modeFlags;
    if (modeFlags & __GL_SHADE_TWOSIDED) {
        colorFace = face;
        faceNeeds = gc->vertex.faceNeeds[face];
    } else {
        colorFace = __GL_FRONTFACE;
        faceNeeds = gc->vertex.faceNeeds[__GL_FRONTFACE];
    }

    /*
    ** Choose colors for the vertices.
    */
    needs = gc->vertex.needs;
    pv = gc->vertex.provoking;
    if (modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
        /* Smooth shading */
        a->color = &a->colors[colorFace];
        b->color = &b->colors[colorFace];
        c->color = &c->colors[colorFace];
        needs |= faceNeeds;
    } else {
        GLuint pvneeds;

        /*
        ** Validate the lighting (and color) information in the provoking
        ** vertex only.  Fill routines always use gc->vertex.provoking->color
        ** to find the color.
        */
        pv->color = &pv->colors[colorFace];
        a->color = pv->color;
        b->color = pv->color;
        c->color = pv->color;
        pvneeds = faceNeeds & (__GL_HAS_LIGHTING | 
                __GL_HAS_FRONT_COLOR | __GL_HAS_BACK_COLOR);
        if (~pv->hasAndClipCode & pvneeds) {
            DO_VALIDATE(gc, pv, pvneeds);
        }
    }

    /* Validate vertices */
    if (~a->hasAndClipCode & needs) DO_VALIDATE(gc, a, needs);
    if (~b->hasAndClipCode & needs) DO_VALIDATE(gc, b, needs);
    if (~c->hasAndClipCode & needs) DO_VALIDATE(gc, c, needs);

    /* Render triangle using the faces polygon mode */
    switch (gc->polygon.mode[face]) {
      case __GL_POLYGON_MODE_FILL:
          {
#if __GL_SST_GLIDE_VTX
              a->sst.r = a->color->r;
              a->sst.g = a->color->g;
              a->sst.b = a->color->b;
              a->sst.a = a->color->a;
              b->sst.r = b->color->r;
              b->sst.g = b->color->g;
              b->sst.b = b->color->b;
              b->sst.a = b->color->a;
              c->sst.r = c->color->r;
              c->sst.g = c->color->g;
              c->sst.b = c->color->b;
              c->sst.a = c->color->a;
              grDrawTriangle((GrVertex *)&a->sst, (GrVertex *)&b->sst, (GrVertex *)&c->sst);
#else
              GrVertex vtxA, vtxB, vtxC;

              vtxA.x = a->window.x;
              vtxA.y = a->window.y;
              vtxA.ooz = a->window.z;
              vtxA.r = a->color->r;
              vtxA.g = a->color->g;
              vtxA.b = a->color->b;

              vtxB.x = b->window.x;
              vtxB.y = b->window.y;
              vtxB.ooz = b->window.z;
              vtxB.r = b->color->r;
              vtxB.g = b->color->g;
              vtxB.b = b->color->b;

              vtxC.x = c->window.x;
              vtxC.y = c->window.y;
              vtxC.ooz = c->window.z;
              vtxC.r = c->color->r;
              vtxC.g = c->color->g;
              vtxC.b = c->color->b;

              /* OPT 0.1.5 overbright: 2x iterated RGB for the collapsed
              ** Q3 world path (see gate comment at top of file) */
              __GL_OB2X_TRI( vtxA, vtxB, vtxC );

              if ( gc->texture.currentTexture[0] &&
                   !gc->texture.currentTexture[1] ) {
                  /* zero not one */
                  vtxA.oow = a->texture[0].w;
                  vtxB.oow = b->texture[0].w;
                  vtxC.oow = c->texture[0].w;
                  vtxA.tmuvtx[0].sow = a->texture[0].x;
                  vtxA.tmuvtx[0].tow = a->texture[0].y;
                  vtxB.tmuvtx[0].sow = b->texture[0].x;
                  vtxB.tmuvtx[0].tow = b->texture[0].y;
                  vtxC.tmuvtx[0].sow = c->texture[0].x;
                  vtxC.tmuvtx[0].tow = c->texture[0].y;
              } else if ( !gc->texture.currentTexture[0] && 
                          gc->texture.currentTexture[1] ) {
                  /* one not zero */
                  vtxA.oow = a->texture[1].w;
                  vtxB.oow = b->texture[1].w;
                  vtxC.oow = c->texture[1].w;
                  vtxA.tmuvtx[0].sow = a->texture[1].x;
                  vtxA.tmuvtx[0].tow = a->texture[1].y;
                  vtxB.tmuvtx[0].sow = b->texture[1].x;
                  vtxB.tmuvtx[0].tow = b->texture[1].y;
                  vtxC.tmuvtx[0].sow = c->texture[1].x;
                  vtxC.tmuvtx[0].tow = c->texture[1].y;
              } else if ( gc->texture.currentTexture[0] ) {
                  /* both */
                  vtxA.oow = a->texture[1].w;
                  vtxB.oow = b->texture[1].w;
                  vtxC.oow = c->texture[1].w;
                  vtxA.tmuvtx[1].sow = a->texture[0].x;
                  vtxA.tmuvtx[1].tow = a->texture[0].y;
                  vtxA.tmuvtx[1].oow = a->texture[0].w;
                  vtxB.tmuvtx[1].sow = b->texture[0].x;
                  vtxB.tmuvtx[1].tow = b->texture[0].y;
                  vtxB.tmuvtx[1].oow = b->texture[0].w;
                  vtxC.tmuvtx[1].sow = c->texture[0].x;
                  vtxC.tmuvtx[1].tow = c->texture[0].y;
                  vtxC.tmuvtx[1].oow = c->texture[0].w;
                  vtxA.tmuvtx[0].sow = a->texture[1].x;
                  vtxA.tmuvtx[0].tow = a->texture[1].y;
                  vtxB.tmuvtx[0].sow = b->texture[1].x;
                  vtxB.tmuvtx[0].tow = b->texture[1].y;
                  vtxC.tmuvtx[0].sow = c->texture[1].x;
                  vtxC.tmuvtx[0].tow = c->texture[1].y;
              } else {
                  /* neither */
              }

              if (gc->state.enables.general & __GL_POLYGON_SMOOTH_ENABLE) {
                  grAADrawTriangle( &vtxA, &vtxB, &vtxC,
                                    a->boundaryEdge, b->boundaryEdge, c->boundaryEdge );
              } else {
                  grDrawTriangle( &vtxA, &vtxB, &vtxC );
              }
#endif
          }
        break;

      case __GL_POLYGON_MODE_POINT:
        if (a->boundaryEdge) (*gc->procs.renderPoint)(gc, a);
        if (b->boundaryEdge) (*gc->procs.renderPoint)(gc, b);
        if (c->boundaryEdge) (*gc->procs.renderPoint)(gc, c);
        break;

      case __GL_POLYGON_MODE_LINE:
        if (a->boundaryEdge) {
            (*gc->procs.renderLine)(gc, a, b);
        }
        if (b->boundaryEdge) {
            (*gc->procs.renderLine)(gc, b, c);
        }
        if (c->boundaryEdge) {
            (*gc->procs.renderLine)(gc, c, a);
        }
        break;
    }

    /* Restore color pointers */
    a->color = &a->colors[__GL_FRONTFACE];
    b->color = &b->colors[__GL_FRONTFACE];
    c->color = &c->colors[__GL_FRONTFACE];
    if (pv) pv->color = &pv->colors[__GL_FRONTFACE];
}

/************************************************************************/

/*
** Generic triangle handling code.  This code is used when render mode
** is GL_RENDER and both polygon modes are FILL and the triangle is
** being flat shaded.
*/
void __glSSTRenderFlatTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b, 
                               __GLvertex *c)
{
    GLuint ccw, needs, pvneeds, modeFlags, faceNeeds;
    __GLfloat dxAC, dxBC, dyAC, dyBC, area;
    GLint colorFace, face;
    __GLvertex *pv;

    /* Compute signed area of the triangle */
#if __GL_SST_GLIDE_VTX
    dxAC = a->sst.x - c->sst.x;
    dxBC = b->sst.x - c->sst.x;
    dyAC = a->sst.y - c->sst.y;
    dyBC = b->sst.y - c->sst.y;
#else
    dxAC = a->window.x - c->window.x;
    dxBC = b->window.x - c->window.x;
    dyAC = a->window.y - c->window.y;
    dyBC = b->window.y - c->window.y;
#endif
    area = dxAC * dyBC - dxBC * dyAC;
    if (!(*(int *)&area << 1)) {
        return;
    }
    ccw = !(*(int *)&area >> 31);

    /*
    ** Figure out if face is culled or not.
    */
    face = gc->polygon.face[ccw];
    if (face == gc->polygon.cullFace) {
        /* Culled */
        return;
    }

    /*
    ** Pick face to use for coloring
    */
    modeFlags = gc->polygon.shader.modeFlags;
    if (modeFlags & __GL_SHADE_TWOSIDED) {
        colorFace = face;
        faceNeeds = gc->vertex.faceNeeds[face];
    } else {
        colorFace = __GL_FRONTFACE;
        faceNeeds = gc->vertex.faceNeeds[__GL_FRONTFACE];
    }

    /*
    ** Choose colors for the vertices.
    */
    needs = gc->vertex.needs;
    pv = gc->vertex.provoking;

    /*
    ** Validate the lighting (and color) information in the provoking
    ** vertex only.  Fill routines always use gc->vertex.provoking->color
    ** to find the color.
    */
    pv->color = &pv->colors[colorFace];
    pvneeds = faceNeeds & (__GL_HAS_LIGHTING |
            __GL_HAS_FRONT_COLOR | __GL_HAS_BACK_COLOR);
    if (~pv->hasAndClipCode & pvneeds) {
        DO_VALIDATE(gc, pv, pvneeds);
    }
    /* Validate vertices */
    if (~a->hasAndClipCode & needs) DO_VALIDATE(gc, a, needs);
    if (~b->hasAndClipCode & needs) DO_VALIDATE(gc, b, needs);
    if (~c->hasAndClipCode & needs) DO_VALIDATE(gc, c, needs);

#if __GL_SST_GLIDE_VTX
    a->sst.r = b->sst.r = c->sst.r = pv->color->r;
    a->sst.g = b->sst.g = c->sst.g = pv->color->g;
    a->sst.b = b->sst.b = c->sst.b = pv->color->b;
    a->sst.a = b->sst.a = c->sst.a = pv->color->a;
    grDrawTriangle((GrVertex *)&a->sst, (GrVertex *)&b->sst, (GrVertex *)&c->sst);
#else
    /* Fill triangle */
    {
        GrVertex vtxA, vtxB, vtxC;
        float r, g, B, A;

        vtxA.x = a->window.x;
        vtxA.y = a->window.y;
        vtxA.ooz = a->window.z;
        vtxA.r = r = pv->color->r;
        vtxA.g = g = pv->color->g;
        vtxA.b = B = pv->color->b;
        vtxA.a = A = pv->color->a;

        vtxB.x = b->window.x;
        vtxB.y = b->window.y;
        vtxB.ooz = b->window.z;
        vtxB.r = r;
        vtxB.g = g;
        vtxB.b = B;
        vtxB.a = A;

        vtxC.x = c->window.x;
        vtxC.y = c->window.y;
        vtxC.ooz = c->window.z;
        vtxC.r = r;
        vtxC.g = g;
        vtxC.b = B;
        vtxC.a = A;

        /* OPT 0.1.5 overbright: 2x iterated RGB for the collapsed
        ** Q3 world path (see gate comment at top of file) */
        __GL_OB2X_TRI( vtxA, vtxB, vtxC );

        if ( gc->texture.currentTexture[0] &&
             !gc->texture.currentTexture[1] ) {
            /* zero not one */
            vtxA.oow = a->texture[0].w;
            vtxB.oow = b->texture[0].w;
            vtxC.oow = c->texture[0].w;
            vtxA.tmuvtx[0].sow = a->texture[0].x;
            vtxA.tmuvtx[0].tow = a->texture[0].y;
            vtxB.tmuvtx[0].sow = b->texture[0].x;
            vtxB.tmuvtx[0].tow = b->texture[0].y;
            vtxC.tmuvtx[0].sow = c->texture[0].x;
            vtxC.tmuvtx[0].tow = c->texture[0].y;
        } else if ( !gc->texture.currentTexture[0] && 
                    gc->texture.currentTexture[1] ) {
            /* one not zero */
            vtxA.oow = a->texture[1].w;
            vtxB.oow = b->texture[1].w;
            vtxC.oow = c->texture[1].w;
            vtxA.tmuvtx[0].sow = a->texture[1].x;
            vtxA.tmuvtx[0].tow = a->texture[1].y;
            vtxB.tmuvtx[0].sow = b->texture[1].x;
            vtxB.tmuvtx[0].tow = b->texture[1].y;
            vtxC.tmuvtx[0].sow = c->texture[1].x;
            vtxC.tmuvtx[0].tow = c->texture[1].y;
        } else if ( gc->texture.currentTexture[0] ) {
            /* both */
            vtxA.oow = a->texture[1].w;
            vtxB.oow = b->texture[1].w;
            vtxC.oow = c->texture[1].w;
            vtxA.tmuvtx[1].sow = a->texture[0].x;
            vtxA.tmuvtx[1].tow = a->texture[0].y;
            vtxA.tmuvtx[1].oow = a->texture[0].w;
            vtxB.tmuvtx[1].sow = b->texture[0].x;
            vtxB.tmuvtx[1].tow = b->texture[0].y;
            vtxB.tmuvtx[1].oow = b->texture[0].w;
            vtxC.tmuvtx[1].sow = c->texture[0].x;
            vtxC.tmuvtx[1].tow = c->texture[0].y;
            vtxC.tmuvtx[1].oow = c->texture[0].w;
            vtxA.tmuvtx[0].sow = a->texture[1].x;
            vtxA.tmuvtx[0].tow = a->texture[1].y;
            vtxB.tmuvtx[0].sow = b->texture[1].x;
            vtxB.tmuvtx[0].tow = b->texture[1].y;
            vtxC.tmuvtx[0].sow = c->texture[1].x;
            vtxC.tmuvtx[0].tow = c->texture[1].y;
        } else {
            /* neither */
        }
        
        if (gc->state.enables.general & __GL_POLYGON_SMOOTH_ENABLE) {
            grAADrawTriangle( &vtxA, &vtxB, &vtxC,
                              a->boundaryEdge, b->boundaryEdge, c->boundaryEdge );
        } else {
            grDrawTriangle( &vtxA, &vtxB, &vtxC );
        }
    }
#endif
    /* Restore color pointers */
    pv->color = &pv->colors[__GL_FRONTFACE];
}

/************************************************************************/

/*
** Generic triangle handling code.  This code is used when render mode
** is GL_RENDER and both polygon modes are FILL and the triangle is
** being smooth shaded.
*/
void __glSSTRenderSmoothTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b, 
                                 __GLvertex *c)
{
    GLuint needs, modeFlags;
    GLint ccw, colorFace, face;
    __GLfloat dxAC, dxBC, dyAC, dyBC, area;

    /* Compute signed area of the triangle */
#if __GL_SST_GLIDE_VTX
    dxAC = a->sst.x - c->sst.x;
    dxBC = b->sst.x - c->sst.x;
    dyAC = a->sst.y - c->sst.y;
    dyBC = b->sst.y - c->sst.y;
#else
    dxAC = a->window.x - c->window.x;
    dxBC = b->window.x - c->window.x;
    dyAC = a->window.y - c->window.y;
    dyBC = b->window.y - c->window.y;
#endif
    area = dxAC * dyBC - dxBC * dyAC;
    if (!(*(int *)&area << 1)) {
        return;
    }
    ccw = !(*(int *)&area >> 31);

    /*
    ** Figure out if face is culled or not.
    */
    face = gc->polygon.face[ccw];
    if (face == gc->polygon.cullFace) {
        /* Culled */
        return;
    }

    /*
    ** Pick face to use for coloring
    */
    modeFlags = gc->polygon.shader.modeFlags;
    needs = gc->vertex.needs;
    if (modeFlags & __GL_SHADE_TWOSIDED) {
        colorFace = face;
        needs |= gc->vertex.faceNeeds[face];
    } else {
        colorFace = __GL_FRONTFACE;
        needs |= gc->vertex.faceNeeds[__GL_FRONTFACE];
    }
    /*
    ** Choose colors for the vertices.
    */
    a->color = &a->colors[colorFace];
    b->color = &b->colors[colorFace];
    c->color = &c->colors[colorFace];

    /* Validate vertices */
    if (~a->hasAndClipCode & needs) DO_VALIDATE(gc, a, needs);
    if (~b->hasAndClipCode & needs) DO_VALIDATE(gc, b, needs);
    if (~c->hasAndClipCode & needs) DO_VALIDATE(gc, c, needs);

#if __GL_SST_GLIDE_VTX
    a->sst.r = a->color->r;
    a->sst.g = a->color->g;
    a->sst.b = a->color->b;
    a->sst.a = a->color->a;
    b->sst.r = b->color->r;
    b->sst.g = b->color->g;
    b->sst.b = b->color->b;
    b->sst.a = b->color->a;
    c->sst.r = c->color->r;
    c->sst.g = c->color->g;
    c->sst.b = c->color->b;
    c->sst.a = c->color->a;
    grDrawTriangle((GrVertex *)&a->sst, (GrVertex *)&b->sst, (GrVertex *)&c->sst);
#else
    /* Fill triangle */
    {
        GrVertex vtxA, vtxB, vtxC;

        vtxA.x = a->window.x;
        vtxA.y = a->window.y;
        vtxA.ooz = a->window.z;
        vtxA.r = a->color->r;
        vtxA.g = a->color->g;
        vtxA.b = a->color->b;
        vtxA.a = a->color->a;

        vtxB.x = b->window.x;
        vtxB.y = b->window.y;
        vtxB.ooz = b->window.z;
        vtxB.r = b->color->r;
        vtxB.g = b->color->g;
        vtxB.b = b->color->b;
        vtxB.a = b->color->a;

        vtxC.x = c->window.x;
        vtxC.y = c->window.y;
        vtxC.ooz = c->window.z;
        vtxC.r = c->color->r;
        vtxC.g = c->color->g;
        vtxC.b = c->color->b;
        vtxC.a = c->color->a;

        /* OPT 0.1.5 overbright: 2x iterated RGB for the collapsed
        ** Q3 world path (see gate comment at top of file) */
        __GL_OB2X_TRI( vtxA, vtxB, vtxC );

        if ( gc->texture.currentTexture[0] &&
             !gc->texture.currentTexture[1] ) {
            /* zero not one */
            vtxA.oow = a->texture[0].w;
            vtxB.oow = b->texture[0].w;
            vtxC.oow = c->texture[0].w;
            vtxA.tmuvtx[0].sow = a->texture[0].x;
            vtxA.tmuvtx[0].tow = a->texture[0].y;
            vtxB.tmuvtx[0].sow = b->texture[0].x;
            vtxB.tmuvtx[0].tow = b->texture[0].y;
            vtxC.tmuvtx[0].sow = c->texture[0].x;
            vtxC.tmuvtx[0].tow = c->texture[0].y;
        } else if ( !gc->texture.currentTexture[0] && 
                    gc->texture.currentTexture[1] ) {
            /* one not zero */
            vtxA.oow = a->texture[1].w;
            vtxB.oow = b->texture[1].w;
            vtxC.oow = c->texture[1].w;
            vtxA.tmuvtx[0].sow = a->texture[1].x;
            vtxA.tmuvtx[0].tow = a->texture[1].y;
            vtxB.tmuvtx[0].sow = b->texture[1].x;
            vtxB.tmuvtx[0].tow = b->texture[1].y;
            vtxC.tmuvtx[0].sow = c->texture[1].x;
            vtxC.tmuvtx[0].tow = c->texture[1].y;
        } else if ( gc->texture.currentTexture[0] ) {
            /* both */
            vtxA.oow = a->texture[1].w;
            vtxB.oow = b->texture[1].w;
            vtxC.oow = c->texture[1].w;
            vtxA.tmuvtx[1].sow = a->texture[0].x;
            vtxA.tmuvtx[1].tow = a->texture[0].y;
            vtxA.tmuvtx[1].oow = a->texture[0].w;
            vtxB.tmuvtx[1].sow = b->texture[0].x;
            vtxB.tmuvtx[1].tow = b->texture[0].y;
            vtxB.tmuvtx[1].oow = b->texture[0].w;
            vtxC.tmuvtx[1].sow = c->texture[0].x;
            vtxC.tmuvtx[1].tow = c->texture[0].y;
            vtxC.tmuvtx[1].oow = c->texture[0].w;
            vtxA.tmuvtx[0].sow = a->texture[1].x;
            vtxA.tmuvtx[0].tow = a->texture[1].y;
            vtxB.tmuvtx[0].sow = b->texture[1].x;
            vtxB.tmuvtx[0].tow = b->texture[1].y;
            vtxC.tmuvtx[0].sow = c->texture[1].x;
            vtxC.tmuvtx[0].tow = c->texture[1].y;
        } else {
            /* neither */
        }

        if (gc->state.enables.general & __GL_POLYGON_SMOOTH_ENABLE) {
            grAADrawTriangle( &vtxA, &vtxB, &vtxC,
                              a->boundaryEdge, b->boundaryEdge, c->boundaryEdge );
        } else {
            grDrawTriangle( &vtxA, &vtxB, &vtxC );
        }
    }
#endif

    /* Restore color pointers */
    a->color = &a->colors[__GL_FRONTFACE];
    b->color = &b->colors[__GL_FRONTFACE];
    c->color = &c->colors[__GL_FRONTFACE];
}

/********************************************************************/

/* flat shaded, one sided lighting */

void __glSSTRenderFlatOneSidedTriangle(__GLcontext *gc, __GLvertex *a,
                                       __GLvertex *b, __GLvertex *c)
{
    GLuint needs, pvneeds, faceNeeds;
    __GLvertex *pv;

    faceNeeds = gc->vertex.faceNeeds[__GL_FRONTFACE];
    needs = gc->vertex.needs;
    pv = gc->vertex.provoking;

    /*
    ** Validate the lighting (and color) information in the provoking
    ** vertex only.
    */
    pv->color = &pv->colors[__GL_FRONTFACE];
#if __GL_SST_GLIDE_VTX
    {
        a->sst.r = b->sst.r = c->sst.r = pv->color->r;
        a->sst.g = b->sst.g = c->sst.g = pv->color->g;
        a->sst.b = b->sst.b = c->sst.b = pv->color->b;
        a->sst.a = b->sst.a = c->sst.a = pv->color->a;
        grDrawTriangle((GrVertex *)&a->sst, (GrVertex *)&b->sst, (GrVertex *)&c->sst);
    }
#else  
    pvneeds = faceNeeds & (__GL_HAS_LIGHTING | __GL_HAS_FRONT_COLOR);
    if (~pv->hasAndClipCode & pvneeds) {
        DO_VALIDATE(gc, pv, pvneeds);
    }
    /* Validate vertices */
    if (~a->hasAndClipCode & needs) DO_VALIDATE(gc, a, needs);
    if (~b->hasAndClipCode & needs) DO_VALIDATE(gc, b, needs);
    if (~c->hasAndClipCode & needs) DO_VALIDATE(gc, c, needs);

    /* Fill triangle */
    {
        GrVertex vtxA, vtxB, vtxC;
        float r, g, B, A;

        vtxA.x = a->window.x;
        vtxA.y = a->window.y;
        vtxA.ooz = a->window.z;
        vtxA.r = r = pv->color->r;
        vtxA.g = g = pv->color->g;
        vtxA.b = B = pv->color->b;
        vtxA.a = A = pv->color->a;

        vtxB.x = b->window.x;
        vtxB.y = b->window.y;
        vtxB.ooz = b->window.z;
        vtxB.r = r;
        vtxB.g = g;
        vtxB.b = B;
        vtxB.a = A;

        vtxC.x = c->window.x;
        vtxC.y = c->window.y;
        vtxC.ooz = c->window.z;
        vtxC.r = r;
        vtxC.g = g;
        vtxC.b = B;
        vtxC.a = A;

        /* OPT 0.1.5 overbright: 2x iterated RGB for the collapsed
        ** Q3 world path (see gate comment at top of file) */
        __GL_OB2X_TRI( vtxA, vtxB, vtxC );

        if ( gc->texture.currentTexture[0] &&
             !gc->texture.currentTexture[1] ) {
            /* zero not one */
            vtxA.oow = a->texture[0].w;
            vtxB.oow = b->texture[0].w;
            vtxC.oow = c->texture[0].w;
            vtxA.tmuvtx[0].sow = a->texture[0].x;
            vtxA.tmuvtx[0].tow = a->texture[0].y;
            vtxB.tmuvtx[0].sow = b->texture[0].x;
            vtxB.tmuvtx[0].tow = b->texture[0].y;
            vtxC.tmuvtx[0].sow = c->texture[0].x;
            vtxC.tmuvtx[0].tow = c->texture[0].y;
        } else if ( !gc->texture.currentTexture[0] && 
                    gc->texture.currentTexture[1] ) {
            /* one not zero */
            vtxA.oow = a->texture[1].w;
            vtxB.oow = b->texture[1].w;
            vtxC.oow = c->texture[1].w;
            vtxA.tmuvtx[0].sow = a->texture[1].x;
            vtxA.tmuvtx[0].tow = a->texture[1].y;
            vtxB.tmuvtx[0].sow = b->texture[1].x;
            vtxB.tmuvtx[0].tow = b->texture[1].y;
            vtxC.tmuvtx[0].sow = c->texture[1].x;
            vtxC.tmuvtx[0].tow = c->texture[1].y;
        } else if ( gc->texture.currentTexture[0] ) {
            /* both */
            vtxA.oow = a->texture[1].w;
            vtxB.oow = b->texture[1].w;
            vtxC.oow = c->texture[1].w;
            vtxA.tmuvtx[1].sow = a->texture[0].x;
            vtxA.tmuvtx[1].tow = a->texture[0].y;
            vtxA.tmuvtx[1].oow = a->texture[0].w;
            vtxB.tmuvtx[1].sow = b->texture[0].x;
            vtxB.tmuvtx[1].tow = b->texture[0].y;
            vtxB.tmuvtx[1].oow = b->texture[0].w;
            vtxC.tmuvtx[1].sow = c->texture[0].x;
            vtxC.tmuvtx[1].tow = c->texture[0].y;
            vtxC.tmuvtx[1].oow = c->texture[0].w;
            vtxA.tmuvtx[0].sow = a->texture[1].x;
            vtxA.tmuvtx[0].tow = a->texture[1].y;
            vtxB.tmuvtx[0].sow = b->texture[1].x;
            vtxB.tmuvtx[0].tow = b->texture[1].y;
            vtxC.tmuvtx[0].sow = c->texture[1].x;
            vtxC.tmuvtx[0].tow = c->texture[1].y;
        } else {
            /* neither */
        }

        if (gc->state.enables.general & __GL_POLYGON_SMOOTH_ENABLE) {
            grAADrawTriangle( &vtxA, &vtxB, &vtxC, a->boundaryEdge, b->boundaryEdge, c->boundaryEdge );
        } else {
            grDrawTriangle( &vtxA, &vtxB, &vtxC );
        }
    }
#endif
}

/* smooth shaded, one sided lighting */

void __glSSTRenderSmoothOneSidedTriangle(__GLcontext *gc, __GLvertex *a,
                                         __GLvertex *b, __GLvertex *c)
{
#if __GL_SST_GLIDE_VTX
  grDrawTriangle((GrVertex *)&a->sst, (GrVertex *)&b->sst, (GrVertex *)&c->sst);
#else  
    GLuint needs;

    needs = gc->vertex.needs | gc->vertex.faceNeeds[__GL_FRONTFACE];

    /*
    ** Choose colors for the vertices.
    */
    a->color = &a->colors[__GL_FRONTFACE];
    b->color = &b->colors[__GL_FRONTFACE];
    c->color = &c->colors[__GL_FRONTFACE];

    /* Validate vertices */
    if (~a->hasAndClipCode & needs) DO_VALIDATE(gc, a, needs);
    if (~b->hasAndClipCode & needs) DO_VALIDATE(gc, b, needs);
    if (~c->hasAndClipCode & needs) DO_VALIDATE(gc, c, needs);

    /* Fill triangle */
    {
        GrVertex vtxA, vtxB, vtxC;

        vtxA.x = a->window.x;
        vtxA.y = a->window.y;
        vtxA.ooz = a->window.z;
        vtxA.r = a->color->r;
        vtxA.g = a->color->g;
        vtxA.b = a->color->b;
        vtxA.a = a->color->a;

        vtxB.x = b->window.x;
        vtxB.y = b->window.y;
        vtxB.ooz = b->window.z;
        vtxB.r = b->color->r;
        vtxB.g = b->color->g;
        vtxB.b = b->color->b;
        vtxB.a = b->color->a;

        vtxC.x = c->window.x;
        vtxC.y = c->window.y;
        vtxC.ooz = c->window.z;
        vtxC.r = c->color->r;
        vtxC.g = c->color->g;
        vtxC.b = c->color->b;
        vtxC.a = c->color->a;

        /* OPT 0.1.5 overbright: 2x iterated RGB for the collapsed
        ** Q3 world path (see gate comment at top of file) */
        __GL_OB2X_TRI( vtxA, vtxB, vtxC );

        if ( gc->texture.currentTexture[0] &&
             !gc->texture.currentTexture[1] ) {
            /* zero not one */
            vtxA.oow = a->texture[0].w;
            vtxB.oow = b->texture[0].w;
            vtxC.oow = c->texture[0].w;
            vtxA.tmuvtx[0].sow = a->texture[0].x;
            vtxA.tmuvtx[0].tow = a->texture[0].y;
            vtxB.tmuvtx[0].sow = b->texture[0].x;
            vtxB.tmuvtx[0].tow = b->texture[0].y;
            vtxC.tmuvtx[0].sow = c->texture[0].x;
            vtxC.tmuvtx[0].tow = c->texture[0].y;
        } else if ( !gc->texture.currentTexture[0] && 
                    gc->texture.currentTexture[1] ) {
            /* one not zero */
            vtxA.oow = a->texture[1].w;
            vtxB.oow = b->texture[1].w;
            vtxC.oow = c->texture[1].w;
            vtxA.tmuvtx[0].sow = a->texture[1].x;
            vtxA.tmuvtx[0].tow = a->texture[1].y;
            vtxB.tmuvtx[0].sow = b->texture[1].x;
            vtxB.tmuvtx[0].tow = b->texture[1].y;
            vtxC.tmuvtx[0].sow = c->texture[1].x;
            vtxC.tmuvtx[0].tow = c->texture[1].y;
        } else if ( gc->texture.currentTexture[0] ) {
            /* both */
            vtxA.oow = a->texture[1].w;
            vtxB.oow = b->texture[1].w;
            vtxC.oow = c->texture[1].w;
            vtxA.tmuvtx[1].sow = a->texture[0].x;
            vtxA.tmuvtx[1].tow = a->texture[0].y;
            vtxA.tmuvtx[1].oow = a->texture[0].w;
            vtxB.tmuvtx[1].sow = b->texture[0].x;
            vtxB.tmuvtx[1].tow = b->texture[0].y;
            vtxB.tmuvtx[1].oow = b->texture[0].w;
            vtxC.tmuvtx[1].sow = c->texture[0].x;
            vtxC.tmuvtx[1].tow = c->texture[0].y;
            vtxC.tmuvtx[1].oow = c->texture[0].w;
            vtxA.tmuvtx[0].sow = a->texture[1].x;
            vtxA.tmuvtx[0].tow = a->texture[1].y;
            vtxB.tmuvtx[0].sow = b->texture[1].x;
            vtxB.tmuvtx[0].tow = b->texture[1].y;
            vtxC.tmuvtx[0].sow = c->texture[1].x;
            vtxC.tmuvtx[0].tow = c->texture[1].y;
        } else {
            /* neither */
        }

        if (gc->state.enables.general & __GL_POLYGON_SMOOTH_ENABLE) {
            grAADrawTriangle( &vtxA, &vtxB, &vtxC, a->boundaryEdge, b->boundaryEdge, c->boundaryEdge );
        } else {
            grDrawTriangle( &vtxA, &vtxB, &vtxC );
        }
    }
#endif
}

/*
 *    Hack.  We "wrap" the triangle rendering proc in the cases that we know
 *    that we will be doing software rendering.  In this way, we are able to
 *    lock the frame buffer ptrs appropriately.
 *
 */

void __glSSTLockRenderTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                           __GLvertex *c)
{
    __GL_LOCK_BUFFERS(gc);
    
    (*gc->procs.WraprenderTriangle)(gc, a, b, c);

    __GL_UNLOCK_BUFFERS(gc);
}
