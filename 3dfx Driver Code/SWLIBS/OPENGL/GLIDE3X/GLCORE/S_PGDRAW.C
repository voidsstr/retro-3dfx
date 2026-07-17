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
#include "render.h"
#include "context.h"
#include "global.h"
#include "imports.h"

/* This routine sets gc->polygon.shader.cfb to gc->drawBuffer */

static void FillSubTriangle(__GLcontext *gc, GLint iyBottom, GLint iyTop)
{
    GLint ixLeft, ixRight;
    GLint ixLeftFrac, ixRightFrac;
    GLint dxLeftFrac, dxRightFrac;
    GLint dxLeftLittle, dxRightLittle;
    GLint dxLeftBig, dxRightBig;
    GLint spanWidth, clipY0, clipY1;
    GLuint modeFlags;

#ifndef WIN32
    __GLcolor colors[__GL_MAX_MAX_VIEWPORT];/*XXX oink */
    __GLcolor fbcolors[__GL_MAX_MAX_VIEWPORT];/*XXX oink */
    __GLstippleWord words[__GL_MAX_STIPPLE_WORDS];
#endif /* !WIN32 */

    ixLeft = gc->polygon.shader.ixLeft;
    ixLeftFrac = gc->polygon.shader.ixLeftFrac;
    ixRight = gc->polygon.shader.ixRight;
    ixRightFrac = gc->polygon.shader.ixRightFrac;
    clipY0 = gc->transform.clipY0;
    clipY1 = gc->transform.clipY1;
    dxLeftFrac = gc->polygon.shader.dxLeftFrac;
    dxLeftBig = gc->polygon.shader.dxLeftBig;
    dxLeftLittle = gc->polygon.shader.dxLeftLittle;
    dxRightFrac = gc->polygon.shader.dxRightFrac;
    dxRightBig = gc->polygon.shader.dxRightBig;
    dxRightLittle = gc->polygon.shader.dxRightLittle;
    modeFlags = gc->polygon.shader.modeFlags;

#ifndef WIN32
    gc->polygon.shader.colors = colors;/*XXX*/
    gc->polygon.shader.fbcolors = fbcolors;/*XXX*/
    gc->polygon.shader.stipplePat = words;
#endif /* !WIN32 */

    if (modeFlags & __GL_SHADE_STENCIL_TEST) {
        gc->polygon.shader.sbuf =
            __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
                              ixLeft, iyBottom);
    }
    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
        gc->polygon.shader.zbuf =
            __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLzValue*),
                            ixLeft, iyBottom);
    }
    gc->polygon.shader.cfb = gc->drawBuffer;
    while (iyBottom < iyTop) {
        spanWidth = ixRight - ixLeft;
        /*
        ** Only render spans that have non-zero width and which are
        ** not scissored out vertically.
        */
        if ((spanWidth > 0) && (iyBottom >= clipY0) && (iyBottom < clipY1)) {
            gc->polygon.shader.frag.x = ixLeft;
            gc->polygon.shader.frag.y = iyBottom;
            gc->polygon.shader.length = spanWidth;
            (*gc->procs.span.processSpan)(gc);
        }

        /* Advance right edge fixed point, adjusting for carry */
        ixRightFrac += dxRightFrac;
        if (ixRightFrac < 0) {
            /* Carry/Borrow'd. Use large step */
            ixRight += dxRightBig;
            ixRightFrac &= ~0x80000000;
        } else {
            ixRight += dxRightLittle;
        }

        iyBottom++;
        ixLeftFrac += dxLeftFrac;
        if (ixLeftFrac < 0) {
            /* Carry/Borrow'd.  Use large step */
            ixLeft += dxLeftBig;
            ixLeftFrac &= ~0x80000000;

            if (modeFlags & __GL_SHADE_RGB) {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                    gc->polygon.shader.frag.color.r += gc->polygon.shader.rBig;
                    gc->polygon.shader.frag.color.g += gc->polygon.shader.gBig;
                    gc->polygon.shader.frag.color.b += gc->polygon.shader.bBig;
                    gc->polygon.shader.frag.color.a += gc->polygon.shader.aBig;
                }
            } else {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                    gc->polygon.shader.frag.color.r += gc->polygon.shader.rBig;
                }
            }
            if (modeFlags & __GL_SHADE_TEXTURE) {
                gc->polygon.shader.frag.s += gc->polygon.shader.sBig;
                gc->polygon.shader.frag.t += gc->polygon.shader.tBig;
                gc->polygon.shader.frag.qw += gc->polygon.shader.qwBig;
                gc->polygon.shader.frag.rhow += gc->polygon.shader.rhowBig;
            }
            if (modeFlags & __GL_SHADE_STENCIL_TEST) {
                /* The implicit multiply is taken out of the loop */
                gc->polygon.shader.sbuf = (__GLstencilCell*)
                    ((GLubyte*) gc->polygon.shader.sbuf
                     + gc->polygon.shader.sbufBig);
            }
            if (modeFlags & __GL_SHADE_DEPTH_ITER) {
                gc->polygon.shader.frag.z += gc->polygon.shader.zBig;
            }

            if (modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
              gc->polygon.shader.offsetZ += gc->polygon.shader.zBig;
                gc->polygon.shader.outBoundCount += 
                  gc->polygon.shader.outCountBig;
#ifdef DEBUG
              if(gc->polygon.shader.outBoundCount > 0. &&
                 (gc->polygon.shader.outBoundCount < 
                  gc->polygon.shader.rangeCount))
                assert(gc->polygon.shader.offsetZ >= 0. &&
                       gc->polygon.shader.offsetZ <= gc->depthBuffer.scale);
#endif
            }

            if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                /* The implicit multiply is taken out of the loop */
                gc->polygon.shader.zbuf = (__GLzValue*)
                    ((GLubyte*) gc->polygon.shader.zbuf
                     + gc->polygon.shader.zbufBig);
            }
            if (modeFlags & __GL_SHADE_SLOW_FOG) {
                gc->polygon.shader.frag.f += gc->polygon.shader.fBig;
            }
        } else {
            /* Use small step */
            ixLeft += dxLeftLittle;
            if (modeFlags & __GL_SHADE_RGB) {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                  gc->polygon.shader.frag.color.r += gc->polygon.shader.rLittle;
                  gc->polygon.shader.frag.color.g += gc->polygon.shader.gLittle;
                  gc->polygon.shader.frag.color.b += gc->polygon.shader.bLittle;
                  gc->polygon.shader.frag.color.a += gc->polygon.shader.aLittle;
                }
            } else {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                  gc->polygon.shader.frag.color.r += gc->polygon.shader.rLittle;
                }
            }
            if (modeFlags & __GL_SHADE_TEXTURE) {
                gc->polygon.shader.frag.s += gc->polygon.shader.sLittle;
                gc->polygon.shader.frag.t += gc->polygon.shader.tLittle;
                gc->polygon.shader.frag.qw += gc->polygon.shader.qwLittle;
                gc->polygon.shader.frag.rhow += gc->polygon.shader.rhowLittle;
            }
            if (modeFlags & __GL_SHADE_STENCIL_TEST) {
                /* The implicit multiply is taken out of the loop */
                gc->polygon.shader.sbuf = (__GLstencilCell*)
                    ((GLubyte*) gc->polygon.shader.sbuf
                     + gc->polygon.shader.sbufLittle);
            }
            if (modeFlags & __GL_SHADE_DEPTH_ITER) {
                gc->polygon.shader.frag.z += gc->polygon.shader.zLittle;
            }

            if (modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
                gc->polygon.shader.offsetZ += gc->polygon.shader.zLittle;
                gc->polygon.shader.outBoundCount += 
                  gc->polygon.shader.outCountLittle;
#ifdef DEBUG
              if(gc->polygon.shader.outBoundCount > 0. &&
                 (gc->polygon.shader.outBoundCount < 
                  gc->polygon.shader.rangeCount))
                assert(gc->polygon.shader.offsetZ >= 0. &&
                       gc->polygon.shader.offsetZ <= gc->depthBuffer.scale);
#endif
            }
            if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                /* The implicit multiply is taken out of the loop */
                gc->polygon.shader.zbuf = (__GLzValue*)
                    ((GLubyte*) gc->polygon.shader.zbuf
                     + gc->polygon.shader.zbufLittle);
            }
            if (modeFlags & __GL_SHADE_SLOW_FOG) {
                gc->polygon.shader.frag.f += gc->polygon.shader.fLittle;
            }
        }
    }
    gc->polygon.shader.ixLeft = ixLeft;
    gc->polygon.shader.ixLeftFrac = ixLeftFrac;
    gc->polygon.shader.ixRight = ixRight;
    gc->polygon.shader.ixRightFrac = ixRightFrac;
}

#define __TWO_31 2147483648.0

#define __FRACTION(result,f) \
    result = (GLint) ((f) * __TWO_31)

void __glSnapXLeft(__GLcontext *gc, __GLfloat xLeft, __GLfloat dxdyLeft)
{
    __GLfloat little, dx;
    GLint ixLeft, ixLeftFrac, frac, lineBytes, elementSize, ilittle, ibig;

    ixLeft = (GLint) xLeft;
    dx = xLeft - ixLeft;
    __FRACTION(ixLeftFrac,dx);

    gc->polygon.shader.ixLeft = ixLeft + (((GLuint) ixLeftFrac) >> 31);
    gc->polygon.shader.ixLeftFrac = ixLeftFrac & ~0x80000000;

    /* Compute big and little steps */
    ilittle = (GLint) dxdyLeft;
    little = (__GLfloat) ilittle;
    if (dxdyLeft < 0) {
        ibig = ilittle - 1;
        dx = little - dxdyLeft;
        __FRACTION(frac,dx);
        gc->polygon.shader.dxLeftFrac = -frac;
    } else {
        ibig = ilittle + 1;
        dx = dxdyLeft - little;
        __FRACTION(frac,dx);
        gc->polygon.shader.dxLeftFrac = frac;
    }
    if (gc->polygon.shader.modeFlags & __GL_SHADE_STENCIL_TEST) {
        /*
        ** Compute the big and little stencil buffer steps.  We walk the
        ** memory pointers for the stencil buffer along the edge of the
        ** triangle as we walk the edge.  This way we don't have to
        ** recompute the buffer address as we go.
        */
        elementSize = gc->stencilBuffer.buf.elementSize;
        lineBytes = elementSize * gc->stencilBuffer.buf.outerWidth;
        gc->polygon.shader.sbufLittle = lineBytes + ilittle * elementSize;
        gc->polygon.shader.sbufBig = lineBytes + ibig * elementSize;
    }
    if (gc->polygon.shader.modeFlags & __GL_SHADE_DEPTH_TEST) {
        /*
        ** Compute the big and little depth buffer steps.  We walk the
        ** memory pointers for the depth buffer along the edge of the
        ** triangle as we walk the edge.  This way we don't have to
        ** recompute the buffer address as we go.
        */
        elementSize = gc->depthBuffer.buf.elementSize;
        lineBytes = elementSize * gc->depthBuffer.buf.outerWidth;
        gc->polygon.shader.zbufLittle = lineBytes + ilittle * elementSize;
        gc->polygon.shader.zbufBig = lineBytes + ibig * elementSize;
    }
    gc->polygon.shader.dxLeftLittle = ilittle;
    gc->polygon.shader.dxLeftBig = ibig;
}

void __glSnapXRight(__GLshade *sh, __GLfloat xRight, __GLfloat dxdyRight)
{
    __GLfloat little, big, dx;
    GLint ixRight, ixRightFrac, frac;

    ixRight = (GLint) xRight;
    dx = xRight - ixRight;
    __FRACTION(ixRightFrac,dx);

    sh->ixRight = ixRight + (((GLuint) ixRightFrac) >> 31);
    sh->ixRightFrac = ixRightFrac & ~0x80000000;

    /* Compute big and little steps */
    little = (__GLfloat) ((GLint) dxdyRight);
    if (dxdyRight < 0) {
        big = little - 1;
        dx = little - dxdyRight;
        __FRACTION(frac,dx);
        sh->dxRightFrac = -frac;
    } else {
        big = little + 1;
        dx = dxdyRight - little;
        __FRACTION(frac,dx);
        sh->dxRightFrac = frac;
    }
    sh->dxRightLittle = (GLint) little;
    sh->dxRightBig = (GLint) big;
}

__GLzValue __glPolygonOffsetZ(__GLcontext *gc, const __GLvertex *a, 
                              __GLfloat dx, __GLfloat dy)
{

    __GLshade *sh = &gc->polygon.shader;
    __GLfloat factor;
    __GLfloat maxdZ;
    __GLfloat bias;
    __GLfloat newZ;

    /*
    ** Calculate factor and bias
    */
    factor = gc->state.polygon.factor;
    bias = gc->state.polygon.units * gc->depthBuffer.minResolution;

    /*
    ** find the maximum Z slope with respect to X and Y
    */
    if(__GL_ABSF(sh->dzdxf) > __GL_ABSF(sh->dzdyf))
        maxdZ = __GL_ABSF(sh->dzdxf);
    else
        maxdZ = __GL_ABSF(sh->dzdyf);

    newZ = (a->window.z + dx*sh->dzdxf + dy*sh->dzdyf) +
           factor * maxdZ + 
           bias;

    /*
    ** Order the limits that might be crossed
    */
    if((GLint)sh->dzdx < 0) {
      sh->startZLimit = gc->depthBuffer.scale;
      sh->endZLimit = 0;
    } else {
      sh->startZLimit = 0;
      sh->endZLimit = gc->depthBuffer.scale;
    }

    sh->offsetZ = newZ;

    if(sh->dzdxf > 0.0) { /* bound is max z value */
      sh->outBoundCount = (gc->depthBuffer.scale - newZ)/sh->dzdxf;
      sh->rangeCount = (GLdouble)gc->depthBuffer.scale/sh->dzdxf;
    } else if(sh->dzdxf < 0.0) { /* bound is min z value */
      sh->outBoundCount = -newZ/sh->dzdxf;
      sh->rangeCount = gc->depthBuffer.scale/-sh->dzdxf;
    } else { /* dzdxf == 0; use a near-zero number, (no dz for entire span) */
      sh->outBoundCount = (gc->depthBuffer.scale - newZ)/
                          __GL_PGON_OFFSET_NEAR_ZERO;
      sh->rangeCount = gc->depthBuffer.scale/__GL_PGON_OFFSET_NEAR_ZERO;
    }

    return((__GLzValue)newZ);
}


static void SetInitialParameters(__GLcontext *gc, const __GLvertex *a,
                                 const __GLcolor *ac, __GLfloat aFog,
                                 __GLfloat dx, __GLfloat dy)
{
    __GLshade *sh = &gc->polygon.shader;
    __GLfloat little = sh->dxLeftLittle;
    __GLfloat big = sh->dxLeftBig;
    GLuint modeFlags = sh->modeFlags;

    if (big > little) {
        if (modeFlags & __GL_SHADE_RGB) {
            if (modeFlags & __GL_SHADE_SMOOTH) {
                sh->frag.color.r = ac->r + dx*sh->drdx + dy*sh->drdy;
                sh->rLittle = sh->drdy + little * sh->drdx;
                sh->rBig = sh->rLittle + sh->drdx;

                sh->frag.color.g = ac->g + dx*sh->dgdx + dy*sh->dgdy;
                sh->gLittle = sh->dgdy + little * sh->dgdx;
                sh->gBig = sh->gLittle + sh->dgdx;

                sh->frag.color.b = ac->b + dx*sh->dbdx + dy*sh->dbdy;
                sh->bLittle = sh->dbdy + little * sh->dbdx;
                sh->bBig = sh->bLittle + sh->dbdx;

                sh->frag.color.a = ac->a + dx*sh->dadx + dy*sh->dady;
                sh->aLittle = sh->dady + little * sh->dadx;
                sh->aBig =sh->aLittle + sh->dadx;
            }
        } else {
            if (modeFlags & __GL_SHADE_SMOOTH) {
                sh->frag.color.r = ac->r + dx*sh->drdx + dy*sh->drdy;
                sh->rLittle = sh->drdy + little * sh->drdx;
                sh->rBig = sh->rLittle + sh->drdx;
            }
        }
        if (modeFlags & __GL_SHADE_TEXTURE) {
            sh->frag.s = a->texture[0].x + dx*sh->dsdx + dy*sh->dsdy;
            sh->sLittle = sh->dsdy + little * sh->dsdx;
            sh->sBig = sh->sLittle + sh->dsdx;

            sh->frag.t = a->texture[0].y + dx*sh->dtdx + dy*sh->dtdy;
            sh->tLittle = sh->dtdy + little * sh->dtdx;
            sh->tBig = sh->tLittle + sh->dtdx;

            sh->frag.qw = a->texture[0].w + dx*sh->dqwdx + dy*sh->dqwdy;
            sh->qwLittle = sh->dqwdy + little * sh->dqwdx;
            sh->qwBig = sh->qwLittle + sh->dqwdx;

            sh->frag.rhow = a->rhow + dx*sh->drhowdx + dy*sh->drhowdy;
            sh->rhowLittle = sh->drhowdy + little*sh->drhowdx;
            sh->rhowBig = sh->rhowLittle + sh->drhowdx;
        }
        if (modeFlags & __GL_SHADE_DEPTH_ITER) {
            __GLfloat zLittle;

            zLittle = sh->dzdyf + little * sh->dzdxf;
            sh->zLittle = (GLint)zLittle;
            sh->zBig = (GLint)(zLittle + sh->dzdxf);

            if(gc->state.enables.general & __GL_POLYGON_OFFSET_FILL_ENABLE) {
                sh->frag.z = __glPolygonOffsetZ(gc, a, dx, dy);
                if(sh->dzdxf == 0.) { /* approximate dzdxf == 0. */
                  sh->outCountBig = -sh->zBig/__GL_PGON_OFFSET_NEAR_ZERO;
                  sh->outCountLittle = -sh->zLittle/__GL_PGON_OFFSET_NEAR_ZERO;
                } else {
                  sh->outCountBig = -sh->zBig/sh->dzdxf;
                  sh->outCountLittle = -sh->zLittle/sh->dzdxf;
                }
            } else {
                sh->frag.z = (__GLzValue)
                             (a->window.z + dx*sh->dzdxf + dy*sh->dzdyf);
            }
        }
        if (modeFlags & __GL_SHADE_SLOW_FOG) {
            sh->frag.f = aFog + dx*sh->dfdx + dy*sh->dfdy;
            sh->fLittle = sh->dfdy + little * sh->dfdx;
            sh->fBig = sh->fLittle + sh->dfdx;
        }
    } else {    
        if (modeFlags & __GL_SHADE_RGB) {
            if (modeFlags & __GL_SHADE_SMOOTH) {
                sh->frag.color.r = ac->r + dx*sh->drdx + dy*sh->drdy;
                sh->rLittle = sh->drdy + little * sh->drdx;
                sh->rBig = sh->rLittle - sh->drdx;
                sh->frag.color.g = ac->g + dx*sh->dgdx + dy*sh->dgdy;
                sh->gLittle = sh->dgdy + little * sh->dgdx;
                sh->gBig = sh->gLittle - sh->dgdx;

                sh->frag.color.b = ac->b + dx*sh->dbdx + dy*sh->dbdy;
                sh->bLittle = sh->dbdy + little * sh->dbdx;
                sh->bBig = sh->bLittle - sh->dbdx;

                sh->frag.color.a = ac->a + dx*sh->dadx + dy*sh->dady;
                sh->aLittle = sh->dady + little * sh->dadx;
                sh->aBig =sh->aLittle - sh->dadx;
            }
        } else {
            if (modeFlags & __GL_SHADE_SMOOTH) {
                sh->frag.color.r = ac->r + dx*sh->drdx + dy*sh->drdy;
                sh->rLittle = sh->drdy + little * sh->drdx;
                sh->rBig = sh->rLittle - sh->drdx;
            }
        }
        if (modeFlags & __GL_SHADE_TEXTURE) {
            sh->frag.s = a->texture[0].x + dx*sh->dsdx + dy*sh->dsdy;
            sh->sLittle = sh->dsdy + little * sh->dsdx;
            sh->sBig = sh->sLittle - sh->dsdx;

            sh->frag.t = a->texture[0].y + dx*sh->dtdx + dy*sh->dtdy;
            sh->tLittle = sh->dtdy + little * sh->dtdx;
            sh->tBig = sh->tLittle - sh->dtdx;

            sh->frag.qw = a->texture[0].w + dx*sh->dqwdx + dy*sh->dqwdy;
            sh->qwLittle = sh->dqwdy + little * sh->dqwdx;
            sh->qwBig = sh->qwLittle - sh->dqwdx;

            sh->frag.rhow = a->rhow + dx*sh->drhowdx + dy*sh->drhowdy;
            sh->rhowLittle = sh->drhowdy + little*sh->drhowdx;
            sh->rhowBig = sh->rhowLittle - sh->drhowdx;
        }
        if (modeFlags & __GL_SHADE_DEPTH_ITER) {
            __GLfloat zLittle;
            zLittle = sh->dzdyf + little * sh->dzdxf;
            sh->zLittle = (GLint)zLittle;
            sh->zBig = (GLint)(zLittle - sh->dzdxf);
            if(gc->state.enables.general & __GL_POLYGON_OFFSET_FILL_ENABLE) {
                sh->frag.z = __glPolygonOffsetZ(gc, a, dx, dy);
                if(sh->dzdxf == 0.) { /* approximate dzdxf == 0. */
                  sh->outCountBig = -sh->zBig/__GL_PGON_OFFSET_NEAR_ZERO;
                  sh->outCountLittle = -sh->zLittle/__GL_PGON_OFFSET_NEAR_ZERO;
                } else {
                  sh->outCountBig = -sh->zBig/sh->dzdxf;
                  sh->outCountLittle = -sh->zLittle/sh->dzdxf;
                }
            } else {
                sh->frag.z = (__GLzValue)
                             (a->window.z + dx*sh->dzdxf + dy*sh->dzdyf);
            }
        }
        if (modeFlags & __GL_SHADE_SLOW_FOG) {
            sh->frag.f = aFog + dx*sh->dfdx + dy*sh->dfdy;
            sh->fLittle = sh->dfdy + little * sh->dfdx;
            sh->fBig = sh->fLittle - sh->dfdx;
        }
    }
}

void __glFillTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                      __GLvertex *c, GLboolean ccw)

{
    __GLfloat oneOverArea, t1, t2, t3, t4;
    __GLfloat dxAC, dxBC, dyAC, dyBC;
    __GLfloat aFog, bFog;
    __GLfloat dxAB, dyAB;
    __GLfloat dx, dy, dxdyLeft, dxdyRight;
    __GLcolor *ac, *bc;
    GLint aIY, bIY, cIY;
    GLuint modeFlags;

    /* Pre-compute one over polygon area */
    oneOverArea = __glOne / gc->polygon.shader.area;

    /* Fetch some stuff we are going to reuse */
    modeFlags = gc->polygon.shader.modeFlags;
    dxAC = gc->polygon.shader.dxAC;
    dxBC = gc->polygon.shader.dxBC;
    dyAC = gc->polygon.shader.dyAC;
    dyBC = gc->polygon.shader.dyBC;
    ac = a->color;
    bc = b->color;

    /*
    ** Compute delta values for unit changes in x or y for each
    ** parameter.
    */
    t1 = dyAC * oneOverArea;
    t2 = dyBC * oneOverArea;
    t3 = dxAC * oneOverArea;
    t4 = dxBC * oneOverArea;
    if (modeFlags & __GL_SHADE_RGB) {
        if (modeFlags & __GL_SHADE_SMOOTH) {
            __GLfloat drAC, dgAC, dbAC, daAC;
            __GLfloat drBC, dgBC, dbBC, daBC;
            __GLcolor *cc;

            cc = c->color;
            drAC = ac->r - cc->r;
            drBC = bc->r - cc->r;
            gc->polygon.shader.drdx = drAC * t2 - drBC * t1;
            gc->polygon.shader.drdy = drBC * t3 - drAC * t4;
            dgAC = ac->g - cc->g;
            dgBC = bc->g - cc->g;
            gc->polygon.shader.dgdx = dgAC * t2 - dgBC * t1;
            gc->polygon.shader.dgdy = dgBC * t3 - dgAC * t4;
            dbAC = ac->b - cc->b;
            dbBC = bc->b - cc->b;
            gc->polygon.shader.dbdx = dbAC * t2 - dbBC * t1;
            gc->polygon.shader.dbdy = dbBC * t3 - dbAC * t4;
            daAC = ac->a - cc->a;
            daBC = bc->a - cc->a;
            gc->polygon.shader.dadx = daAC * t2 - daBC * t1;
            gc->polygon.shader.dady = daBC * t3 - daAC * t4;
        } else {
            __GLcolor *flatColor = gc->vertex.provoking->color;
            gc->polygon.shader.frag.color.r = flatColor->r;
            gc->polygon.shader.frag.color.g = flatColor->g;
            gc->polygon.shader.frag.color.b = flatColor->b;
            gc->polygon.shader.frag.color.a = flatColor->a;
        }
    } else {
        if (modeFlags & __GL_SHADE_SMOOTH) {
            __GLfloat drAC;
            __GLfloat drBC;
            __GLcolor *cc;

            cc = c->color;
            drAC = ac->r - cc->r;
            drBC = bc->r - cc->r;
            gc->polygon.shader.drdx = drAC * t2 - drBC * t1;
            gc->polygon.shader.drdy = drBC * t3 - drAC * t4;
        } else {
            __GLcolor *flatColor = gc->vertex.provoking->color;
            gc->polygon.shader.frag.color.r = flatColor->r;
        }
    }
    if (modeFlags & __GL_SHADE_TEXTURE) {
        __GLfloat dsAC, dsBC, dtAC, dtBC, dqwAC, dqwBC;
        __GLfloat rhowa, rhowb, rhowc;
        __GLfloat drhowAC, drhowBC;
        __GLtexture     *tex;
        __GLmipMapLevel *lp;
        __GLfloat sScale, tScale;

        tex = gc->texture.currentTexture[0];
        lp = &tex->level[0];
        sScale = lp->width2 / lp->width2f;
        tScale = lp->height2 / lp->height2f;

        dsAC = (a->texture[0].x*sScale) - (c->texture[0].x*sScale);
        dsBC = (b->texture[0].x*sScale) - (c->texture[0].x*sScale);
        gc->polygon.shader.dsdx = dsAC * t2 - dsBC * t1;
        gc->polygon.shader.dsdy = dsBC * t3 - dsAC * t4;
        dtAC = (a->texture[0].y*tScale) - (c->texture[0].y*tScale);
        dtBC = (b->texture[0].y*tScale) - (c->texture[0].y*tScale);
        gc->polygon.shader.dtdx = dtAC * t2 - dtBC * t1;
        gc->polygon.shader.dtdy = dtBC * t3 - dtAC * t4;
        dqwAC = a->texture[0].w - c->texture[0].w;
        dqwBC = b->texture[0].w - c->texture[0].w;
        gc->polygon.shader.dqwdx = dqwAC * t2 - dqwBC * t1;
        gc->polygon.shader.dqwdy = dqwBC * t3 - dqwAC * t4;


        /* calculate rho/w at the vertices */
        rhowa = (*gc->procs.calcPolygonRho)(gc, &gc->polygon.shader,
                                            (a->texture[0].x*sScale),
                                            (a->texture[0].y*tScale),
                                            a->texture[0].w)
            * a->texture[0].w;
        a->rhow = rhowa;
        rhowb = (*gc->procs.calcPolygonRho)(gc, &gc->polygon.shader,
                                            (b->texture[0].x*sScale),
                                            (b->texture[0].y*tScale),
                                            b->texture[0].w)
            * b->texture[0].w;
        b->rhow = rhowb;
        rhowc = (*gc->procs.calcPolygonRho)(gc, &gc->polygon.shader,
                                            (c->texture[0].x*sScale),
                                            (c->texture[0].y*tScale),
                                            c->texture[0].w)
            * c->texture[0].w;
        c->rhow = rhowc;

        drhowAC = rhowa - rhowc;
        drhowBC = rhowb - rhowc;
        gc->polygon.shader.drhowdx = drhowAC * t2 - drhowBC * t1;
        gc->polygon.shader.drhowdy = drhowBC * t3 - drhowAC * t4;
    }
    if (modeFlags & __GL_SHADE_DEPTH_ITER) {
        __GLfloat dzAC, dzBC;

        dzAC = a->window.z - c->window.z;
        dzBC = b->window.z - c->window.z;
        gc->polygon.shader.dzdxf = dzAC * t2 - dzBC * t1;
        gc->polygon.shader.dzdyf = dzBC * t3 - dzAC * t4;
        gc->polygon.shader.dzdx = (GLint) gc->polygon.shader.dzdxf;
        gc->polygon.shader.dzdxBig = 
            gc->polygon.shader.dzdx << __GL_STIPPLE_COUNT_BITS;
    }
    if (modeFlags & __GL_SHADE_SLOW_FOG) {
        __GLfloat dfAC, dfBC, cFog;

        if (gc->state.hints.fog == GL_NICEST) {
            /* Use eyeZ for interpolation value */
            aFog = a->eye.z;
            bFog = b->eye.z;
            cFog = c->eye.z;
        } else {
            /* Use fog(eyeZ) for interpolation value */
            aFog = a->fog;
            bFog = b->fog;
            cFog = c->fog;
        }
        dfAC = aFog - cFog;
        dfBC = bFog - cFog;
        gc->polygon.shader.dfdx = dfAC * t2 - dfBC * t1;
        gc->polygon.shader.dfdy = dfBC * t3 - dfAC * t4;
    }

    /* Snap each y coordinate to its pixel center */
    aIY = (GLint) ((a->window.y+__glHalf));
    bIY = (GLint) ((b->window.y+__glHalf));
    cIY = (GLint) ((c->window.y+__glHalf));

    /*
    ** This algorithim always fills from bottom to top, left to right.
    ** Because of this, ccw triangles are inherently faster because
    ** the parameter values need not be recomputed.
    */
    dxAB = (a->window.x+__glHalf) - (b->window.x+__glHalf);
    dyAB = (a->window.y+__glHalf) - (b->window.y+__glHalf);

    __GL_LOCK_BUFFERS(gc);

    if (ccw) {
        dxdyLeft = dxAC / dyAC;
        dy = (aIY + __glOne) - (a->window.y+__glHalf);
        __glSnapXLeft(gc, (a->window.x+__glHalf) + dy*dxdyLeft, dxdyLeft);
        dx = (gc->polygon.shader.ixLeft + __glOne) - (a->window.x+__glHalf);
        SetInitialParameters(gc, a, ac, aFog, dx, dy);
        if (aIY != bIY) {
                dxdyRight = dxAB / dyAB;
            __glSnapXRight(&gc->polygon.shader, (a->window.x+__glHalf) + dy*dxdyRight,
                       dxdyRight);
            FillSubTriangle(gc, aIY, bIY);
        }

        if (bIY != cIY) {
            dxdyRight = dxBC / dyBC;
            dy = (bIY + __glOne) - (b->window.y+__glHalf);
            __glSnapXRight(&gc->polygon.shader, (b->window.x+__glHalf) + dy*dxdyRight,
                       dxdyRight);
            FillSubTriangle(gc, bIY, cIY);
        }
    } else {
        dxdyRight = dxAC / dyAC;
        dy = (aIY + __glOne) - (a->window.y+__glHalf);
        __glSnapXRight(&gc->polygon.shader, (a->window.x+__glHalf) + dy*dxdyRight, dxdyRight);
        if (aIY != bIY) {
            dxdyLeft = dxAB / dyAB;
            __glSnapXLeft(gc, (a->window.x+__glHalf) + dy*dxdyLeft, dxdyLeft);
            dx = (gc->polygon.shader.ixLeft + __glOne) - (a->window.x+__glHalf);
            SetInitialParameters(gc, a, ac, aFog, dx, dy);
            FillSubTriangle(gc, aIY, bIY);
        }

        if (bIY != cIY) {
            dxdyLeft = dxBC / dyBC;
            dy = (bIY + __glOne) - (b->window.y+__glHalf);
            __glSnapXLeft(gc, (b->window.x+__glHalf) + dy*dxdyLeft, dxdyLeft);
            dx = (gc->polygon.shader.ixLeft + __glOne) - (b->window.x+__glHalf);
            SetInitialParameters(gc, b, bc, bFog, dx, dy);
            FillSubTriangle(gc, bIY, cIY);
        }
    }

    __GL_UNLOCK_BUFFERS(gc);
}

void __glFillFlatFogTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                             __GLvertex *c, GLboolean ccw)
{
    __GLcolor acol, bcol, ccol;
    __GLcolor *aocp, *bocp, *cocp;
    __GLvertex *pv;

    pv = gc->vertex.provoking;
    (*gc->procs.fogColor)(gc, &acol, pv->color, a->fog);
    (*gc->procs.fogColor)(gc, &bcol, pv->color, b->fog);
    (*gc->procs.fogColor)(gc, &ccol, pv->color, c->fog);
    aocp = a->color;
    bocp = b->color;
    cocp = c->color;
    a->color = &acol;
    b->color = &bcol;
    c->color = &ccol;

    (*gc->procs.fillTriangle2)(gc, a, b, c, ccw);

    a->color = aocp;
    b->color = bocp;
    c->color = cocp;
}
