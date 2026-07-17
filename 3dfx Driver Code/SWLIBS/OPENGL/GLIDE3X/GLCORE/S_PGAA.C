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


/*
** Normal form of a line: Ax + By + C = 0.  When evaluated at a point P,
** the value is zero when P is on the line.  For points off the line,
** the sign of the value determines which side of the line P is on.
*/
typedef struct {
    __GLfloat a, b, c;

    /*
    ** The sign of an edge is determined by plugging the third vertex
    ** of the triangle into the line equation.  This flag is GL_TRUE when
    ** the sign is positive.
    */
    GLboolean edgeSign;
} __glLineEquation;

/*
** Machine state for rendering triangles.
*/
typedef struct {
    __GLfloat dyAB;
    __GLfloat dyBC;
    __glLineEquation ab;
    __glLineEquation bc;
    __glLineEquation ca;
    __GLfloat area;
    GLint areaSign;
} __glTriangleMachine;

/*
** Plane equation coefficients.  One plane equation exists for each of
** the parameters being computed across the surface of the triangle.
*/
typedef struct {
    __GLfloat a, b, c, d;
} __glPlaneEquation;

/*
** Cache for some of the coverage computation constants.
*/
typedef struct {
    __GLfloat dx, dy;
    GLint samples;
    GLint samplesSquared;
    __GLfloat samplesSquaredInv;
    GLboolean lastCoverageWasOne;
    __GLfloat leftDelta, rightDelta;
    __GLfloat bottomDelta, topDelta;
} __glCoverageStuff;

/*
** Cache for all the parameters being passed to FillAASubTriangle
*/
typedef struct {
    __glPlaneEquation rp, gp, bp, ap;   /* the color plane equations */
    __glPlaneEquation zp;               /* the z plane eq. */
    __glPlaneEquation sp, tp, qwp;      /* eqs for the texture coords */
    __glPlaneEquation rhow;             /* for the texture detail */
    __glPlaneEquation ezp;              /* this is for the eye z */
    __glTriangleMachine tm;
    __glCoverageStuff cs;
    __GLcontext *gc;
} __glFillAASubTriangleInfo;

/*
** Compute the constants A, B and C for a line equation in the general
** form:  Ax + By + C = 0.  A given point at (x,y) can be plugged into
** the left side of the equation and yield a number which indiates whether
** or not the point is on the line.  If the result is zero, then the point
** is on the line.  The sign of the result determines which side of
** the line the point is on.  To handle tie cases properly we need a way
** to assign a point on the edge to only one triangle.  To do this, we
** look at the sign of the equation evaluated at "c".  For edges whose
** sign at "c" is positive, we allow points on the edge to be in the
** triangle.
*/
static void FindLineEquation(__glLineEquation *eq, const __GLcoord *a,
                            const __GLcoord *b, const __GLcoord *c)
{
    __GLfloat dy, dx, valueAtC;

    /*
    ** Sort a,b so that the ordering of the verticies is consistent,
    ** regardless of the order given to this procedure.
    */
    if (b->y < a->y) {
        const __GLcoord *temp = b;
        b = a;
        a = temp;
    } else
    if ((b->y == a->y) && (b->x < a->x)) {
        const __GLcoord *temp = b;
        b = a;
        a = temp;
    }

    dy = b->y - a->y;
    dx = b->x - a->x;
    eq->a = -dy;
    eq->b = dx;
    eq->c = dy * a->x - dx * a->y;

    valueAtC = eq->a * c->x + eq->b * c->y + eq->c;
    if (valueAtC > 0) {
        eq->edgeSign = GL_TRUE;
    } else {
        eq->edgeSign = GL_FALSE;
    }
}

/*
** Given three points in (x,y,p) find the plane equation coeffecients
** for the plane that contains the three points.  First find the cross
** product of two of the vectors defined by the three points, then
** use one of the points to find "d".
*/
static void FindPlaneEquation(__GLcontext *gc, __glPlaneEquation *eq,
                              const __GLvertex *a, const __GLvertex *b,
                              const __GLvertex *c, __GLfloat p1,
                              __GLfloat p2, __GLfloat p3)
{
    __GLfloat v1x, v1y, v1p;
    __GLfloat v2x, v2y, v2p;
    __GLfloat nx, ny, np, k;

    /* find vector v1 */
    v1x = b->window.x - a->window.x;
    v1y = b->window.y - a->window.y;
    v1p = p2 - p1;

    /* find vector v2 */
    v2x = c->window.x - a->window.x;
    v2y = c->window.y - a->window.y;
    v2p = p3 - p1;

    /* find the cross product (== normal) for the plane */
    nx = v1y*v2p - v1p*v2y;
    ny = v1p*v2x - v1x*v2p;
    np = v1x*v2y - v1y*v2x;

    /*
    ** V dot N = k.  Find k.  We can use any of the three points on
    ** the plane, so we use a.
    */
    k = (a->window.x - __glHalf)*nx + (a->window.y - __glHalf)*ny + p1*np;

    /*
    ** Finally, setup the plane equation coeffecients.  Force c to be one
    ** by dividing everything through by c.
    */
    eq->a = nx / np;
    eq->b = ny / np;
    eq->c = ((__GLfloat) 1.0);
    eq->d = -k / np;
}

/*
** Solve for p in the plane equation.
*/
static __GLfloat FindP(__glPlaneEquation *eq, __GLfloat x, __GLfloat y)
{
    return -(eq->a * x + eq->b * y + eq->d);
}

/*
** See if a given point is on the same side of the edge as the other
** vertex in the triangle not part of this edge.  When the line
** equation evaluates to zero, make points which are on lines with
** a negative edge sign (edgeSign GL_FALSE) part of the triangle.
*/
#define In(eq,x,y) \
    (((eq)->a * (x) + (eq)->b * (y) + (eq)->c > 0) == (eq)->edgeSign)

/*
** Determine if the point x,y is in or out of the triangle.  Evaluate
** each line equation for the point and compare the sign of the result
** with the edgeSign flag.
*/
#define Inside(tm,x,y) \
    (In(&(tm)->ab, x, y) && In(&(tm)->bc, x, y) && In(&(tm)->ca, x, y))

#define FILTER_WIDTH    ((__GLfloat) 1.0)
#define FILTER_HEIGHT   ((__GLfloat) 1.0)

/*
** Precompute stuff that is constant for all coverage tests.
*/
static void ComputeCoverageStuff(__glCoverageStuff *cs, GLint samples)
{
    __GLfloat dx, dy, fs = samples;
    __GLfloat half = ((__GLfloat) 0.5);

    cs->dx = dx = FILTER_WIDTH / fs;
    cs->dy = dy = FILTER_HEIGHT / fs;
    cs->leftDelta = -(FILTER_WIDTH / 2) + dx * half;
    cs->rightDelta = (FILTER_WIDTH / 2) - dx * half;
    cs->bottomDelta = -(FILTER_HEIGHT / 2) + dy * half;
    cs->topDelta = (FILTER_HEIGHT / 2) - dy * half;
    cs->samplesSquared = samples * samples;
    cs->samplesSquaredInv = ((__GLfloat) 1.0) / cs->samplesSquared;
    cs->samples = samples;
}

/*
** Return an estimate of the pixel coverage using sub-sampling.
*/
static __GLfloat Coverage(__glTriangleMachine *tm, __GLfloat *xs,
                          __GLfloat *ys, __glCoverageStuff *cs)
{
    GLint xx, yy, hits, samples;
    __GLfloat dx, dy, yBottom, px, py;
    __GLfloat minX, minY, maxX, maxY;

    hits = 0;
    samples = cs->samples;
    dx = cs->dx;
    dy = cs->dy;
    px = *xs + cs->leftDelta;
    yBottom = *ys + cs->bottomDelta;

    /*
    ** If the last coverage was one (the pixel to the left in x from us),
    ** then if the upper right and lower right sample positions are
    ** also in then this entire pixel must be in.
    */
    if (cs->lastCoverageWasOne) {
        __GLfloat urx, ury;
        urx = *xs + cs->rightDelta;
        ury = *ys + cs->topDelta;
        if (Inside(tm, urx, ury) && Inside(tm, urx, yBottom)) {
            return ((__GLfloat) 1.0);
        }
    }

    /*
    ** Setup minimum and maximum x,y coordinates.  The min and max values
    ** are used to find a "good" point that is actually within the
    ** triangle so that parameter values can be computed correctly.
    */
    minX = 999999;
    maxX = __glMinusOne;
    minY = 999999;
    maxY = __glMinusOne;
    for (xx = 0; xx < samples; xx++) {
        py = yBottom;
        for (yy = 0; yy < samples; yy++) {
            if (Inside(tm, px, py)) {
                if (px < minX) minX = px;
                if (px > maxX) maxX = px;
                if (py < minY) minY = py;
                if (py > maxY) maxY = py;
                hits++;
            }
            py += dy;
        }
        px += dx;
    }
    if (hits) {
        /*
        ** Return the average of the two coordinates which is guaranteed
        ** to be in the triangle.
        */
        *xs = (minX + maxX) * ((__GLfloat) 0.5);
        *ys = (minY + maxY) * ((__GLfloat) 0.5);
        if (hits == cs->samplesSquared) {
            /* Keep track when the last coverage was one */
            cs->lastCoverageWasOne = GL_TRUE;
            return ((__GLfloat) 1.0);
        }
    }
    cs->lastCoverageWasOne = GL_FALSE;
    return hits * cs->samplesSquaredInv;
}

/* 
 * macro to process an AA pixel.  
 * a subroutine is avoided for speed.
 */
#define PROCESS_PIXEL \
            /* \
             * we are done with coordinate check.\
             * Let's get serious and start rendering.\
             */\
            if( modeFlags & __GL_SHADE_STIPPLE ) {\
                /* check against stipple pattern */\
                GLint row, col;\
                if( yInverted ) {\
                    row = (windowHeight - (iyBottom - viewYAdjust) - 1)\
                        & (__GL_STIPPLE_BITS-1);\
                } else {\
                    row = iyBottom & (__GL_STIPPLE_BITS-1);\
                }\
                col = x & (__GL_STIPPLE_BITS-1);\
                if( (gc->polygon.stipple[row] & __GL_STIPPLE_SHIFT(col)) == 0 ) {\
                    /* we hit a void.  try next pixel */\
                    continue;\
                }\
            }\
\
            /* calculate the fragment now */\
\
            /*\
             ** Fill in fragment for rendering.  First compute the color\
             ** of the fragment.\
             */\
            if (modeFlags & __GL_SHADE_SMOOTH) {\
                frag.color.r = FindP(&info->rp, xs, ys);\
                if (rgbMode) {\
                    frag.color.g = FindP(&info->gp, xs, ys);\
                    frag.color.b = FindP(&info->bp, xs, ys);\
                    frag.color.a = FindP(&info->ap, xs, ys);\
                }\
            } else {\
                frag.color.r = flatColor->r;\
                if (rgbMode) {\
                    frag.color.g = flatColor->g;\
                    frag.color.b = flatColor->b;\
                    frag.color.a = flatColor->a;\
                }\
            }\
\
            /*\
             ** Texture the fragment.\
             */\
            if (modeFlags & __GL_SHADE_TEXTURE) {\
                __GLfloat qw, qwinv, s, sw, t, tw, rho, rhow;\
                \
                qwinv = FindP(&info->qwp, xs, ys);\
                rhow = FindP(&info->rhow, xs, ys);\
                s = FindP(&info->sp, xs, ys);\
                t = FindP(&info->tp, xs, ys);\
                qw = __glOne / qwinv;\
                sw = s * qw;\
                tw = t * qw;\
                rho = rhow * qw;\
                (*gc->procs.texture)(gc, &frag.color, sw, tw, rho);\
            }\
\
            /*\
             ** Fog the resulting color.\
             */\
            if (modeFlags & __GL_SHADE_SLOW_FOG) {\
                __GLfloat eyeZ = FindP(&info->ezp, xs, ys);\
                __glFogFragmentSlow(gc, &frag, eyeZ);\
            }\
\
            /*\
             ** Apply anti-aliasing effect\
             */\
            if (rgbMode) {\
                frag.color.a *= coverage;\
            } else {\
                frag.color.r =\
                    __glBuildAntiAliasIndex(frag.color.r,\
                                            coverage);\
            }\
\
            /*\
             ** Finally, render the fragment.\
             */\
            frag.x = (GLint)xs;\
            frag.y = (GLint)ys;\
            if (modeFlags & __GL_SHADE_DEPTH_ITER) {\
                frag.z = (__GLzValue)FindP(&info->zp, xs, ys);\
            }\
            (*gc->procs.store)(gc->drawBuffer, &frag);\


/* This routine sets gc->polygon.shader.cfb to gc->drawBuffer */

static void FillAASubTriangle(GLint iyBottom, GLint iyTop,
                              __glFillAASubTriangleInfo *info )
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

    /* some caches */
    GLboolean rgbMode;
    __GLcolor *flatColor;
    __GLcontext *gc = info->gc;
    __glTriangleMachine *tm = &(info->tm);
    __glCoverageStuff *cs = &(info->cs);
    GLboolean yInverted = gc->constants.yInverted;
    GLint windowHeight = gc->constants.height;
    GLint viewYAdjust = gc->constants.viewportYAdjust;

    /* used for antialiasing */
    __GLfloat zero = __glZero;
    __GLfloat half = __glHalf;
    __GLfloat one = __glOne;
    GLint leftDiff;

    __GLshade *sh = &gc->polygon.shader;

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

    /* for AA */
    rgbMode = modeFlags & __GL_SHADE_RGB;
    flatColor = gc->vertex.provoking->color;

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
        /*
         * before we play around filling spans, let's figure out
         * which pixels are partially covered due to anti-aliasing.
         *
         * The following algorithm makes the assumption that since
         * ixLeft and ixRight are on the edge, their coverage is not 0.
         * There is a guard statement however to avoid filling infinite spans.
         * Starting from ixLeft we go left processing pixels till we
         * hit coverage==0.  Then we jump back to ixLeft going to the right
         * processing pixels till we hit coverage==1 or coverage==0.  If
         * we hit coverage==0, then there are no internal pixels, so we are
         * done with this scanline.  If we hit coverage==1, mark this pixel
         * as the beginning of the scanline.  Perform the symmetric operation
         * for ixRight.  Finally, if there is a span of pixels with coverage==1
         * render them using span.processSpan, ottherwise go ahead as usual
         * to calculate the parameters for the next scanline.
         */
        GLint x;
        __GLfloat xs, ys;
        __GLfloat coverage;
        GLint xSpanLeft, xSpanRight;
        __GLfragment frag;
        GLboolean doRightEdge = GL_TRUE;

        /* fix left edge first */
        for( x = ixLeft;; x-- ) {
            xs = x + half;
            ys = iyBottom + half;
            cs->lastCoverageWasOne = GL_FALSE;
            coverage = Coverage( tm, &xs, &ys, cs );
            if( coverage == zero ) break;

            PROCESS_PIXEL;
        }
        for( x = ixLeft+1;; x++ ) {
            xs = x + half;
            ys = iyBottom + half;
            cs->lastCoverageWasOne = GL_FALSE;
            coverage = Coverage( tm, &xs, &ys, cs );
            if( coverage == zero ) {
                doRightEdge = GL_FALSE; /* don't do right edge */
                xSpanLeft = -1;
                break;
            }
            if( coverage == one ) {
                xSpanLeft = x;
                break;
            }

            PROCESS_PIXEL;
        }

        /* exit in case of an emergency */
        if( xSpanLeft > ixRight ) { /* something's fishy */
            xSpanLeft = -1;
            doRightEdge = GL_FALSE;
        }

        /* now fix right edge */
        if( doRightEdge == GL_TRUE ) {
            for( x = ixRight;; x++ ) {
                xs = x + half;
                ys = iyBottom + half;
                cs->lastCoverageWasOne = GL_FALSE;
                coverage = Coverage( tm, &xs, &ys, cs );
                if( coverage == zero ) break;

                PROCESS_PIXEL;
            }
            for( x = ixRight-1;; x-- ) {
                xs = x + half;
                ys = iyBottom + half;
                cs->lastCoverageWasOne = GL_FALSE;
                coverage = Coverage( tm, &xs, &ys, cs );
                if( coverage == zero ) {
                    xSpanRight = -1;
                    break;
                }
                if( coverage == one ) {
                    xSpanRight = x;
                    break;
                }

                PROCESS_PIXEL;
            }
        }

        /* after all these pixels are done, fix the iteration values */
        if( xSpanLeft != -1 ) {
            leftDiff = xSpanLeft - ixLeft;

            if( rgbMode ) {
                if( modeFlags & __GL_SHADE_SMOOTH ) {
                    sh->frag.color.r += leftDiff * sh->drdx;
                    sh->frag.color.g += leftDiff * sh->dgdx;
                    sh->frag.color.b += leftDiff * sh->dbdx;
                    sh->frag.color.a += leftDiff * sh->dadx;
                }
                if( modeFlags & __GL_SHADE_TEXTURE ) {
                    sh->frag.s += leftDiff * sh->dsdx;
                    sh->frag.t += leftDiff * sh->dtdx;
                    sh->frag.qw += leftDiff * sh->dqwdx;
                    sh->frag.rhow += leftDiff * sh->drhowdx;
                }
            } else {
                if( modeFlags & __GL_SHADE_SMOOTH ) {
                    sh->frag.color.r += leftDiff * sh->drdy;
                }
            }
            if( modeFlags & __GL_SHADE_DEPTH_ITER ) {
                sh->frag.z += leftDiff * sh->dzdx;
            }
            if( modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
                sh->offsetZ += leftDiff * sh->dzdx;
                if(sh->dzdxf != 0.)
                  gc->polygon.shader.outBoundCount += 
                    -(leftDiff * sh->dzdx)/sh->dzdxf;
            }
            if( modeFlags & __GL_SHADE_DEPTH_TEST ) {
                sh->zbuf += leftDiff;
            }
            if( modeFlags & __GL_SHADE_STENCIL_TEST ) {
                sh->sbuf += leftDiff;
            }
            if( modeFlags & __GL_SHADE_SLOW_FOG ) {
                sh->frag.f += leftDiff * sh->dfdx;
            }
        } else {
            leftDiff = 0;
        }
        
        /* render span if it exists */
        if( (xSpanLeft != -1) && (xSpanRight != -1) ) {
            spanWidth = xSpanRight - xSpanLeft + 1;
            /*
             ** Only render spans that have non-zero width and which are
             ** not scissored out vertically.
             */
            if ((spanWidth > 0) && (iyBottom >= clipY0) && 
                (iyBottom < clipY1)) {
                gc->polygon.shader.frag.x = xSpanLeft;
                gc->polygon.shader.frag.y = iyBottom;
                gc->polygon.shader.length = spanWidth;
                (*gc->procs.span.processSpan)(gc);
            }
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
                    sh->frag.color.r += sh->rBig - leftDiff * sh->drdx;
                    sh->frag.color.g += sh->gBig - leftDiff * sh->dgdx;
                    sh->frag.color.b += sh->bBig - leftDiff * sh->dbdx;
                    sh->frag.color.a += sh->aBig - leftDiff * sh->dadx;
                }
                if (modeFlags & __GL_SHADE_TEXTURE) {
                    sh->frag.s += sh->sBig - leftDiff * sh->dsdx;
                    sh->frag.t += sh->tBig - leftDiff * sh->dtdx;
                    sh->frag.qw += sh->qwBig - leftDiff * sh->dqwdx;
                    sh->frag.rhow += sh->rhowBig - leftDiff * sh->drhowdx;
                }
            } else {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                    sh->frag.color.r += sh->rBig - leftDiff * sh->drdx;
                }
            }
            if (modeFlags & __GL_SHADE_STENCIL_TEST) {
                /* The implicit multiply is taken out of the loop */
                sh->sbuf = (__GLstencilCell*)
                    ((GLubyte*) sh->sbuf + sh->sbufBig);
            }
            if (modeFlags & __GL_SHADE_DEPTH_ITER) {
                sh->frag.z += sh->zBig - leftDiff * sh->dzdx;
            }
            if( modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
                sh->offsetZ += sh->zBig - leftDiff * sh->dzdx;
                if(sh->dzdxf == 0.) /* approximate 0 dzdxf */
                  gc->polygon.shader.outBoundCount += 
                    -sh->zBig/__GL_PGON_OFFSET_NEAR_ZERO;
                    else
                  gc->polygon.shader.outBoundCount += 
                    -(sh->zBig - leftDiff * sh->dzdx)/sh->dzdxf;
            }
            if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                sh->zbuf -= leftDiff;
                /* The implicit multiply is taken out of the loop */
                sh->zbuf = (__GLzValue*)
                    ((GLubyte*) sh->zbuf + sh->zbufBig);
            }
            if (modeFlags & __GL_SHADE_SLOW_FOG) {
                sh->frag.f += sh->fBig - leftDiff * sh->dfdx;
            }
        } else {
            /* Use small step */
            ixLeft += dxLeftLittle;
            if (modeFlags & __GL_SHADE_RGB) {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                    sh->frag.color.r += sh->rLittle - leftDiff * sh->drdx;
                    sh->frag.color.g += sh->gLittle - leftDiff * sh->dgdx;
                    sh->frag.color.b += sh->bLittle - leftDiff * sh->dbdx;
                    sh->frag.color.a += sh->aLittle - leftDiff * sh->dadx;
                }
                if (modeFlags & __GL_SHADE_TEXTURE) {
                    sh->frag.s += sh->sLittle - leftDiff * sh->dsdx;
                    sh->frag.t += sh->tLittle - leftDiff * sh->dtdx;
                    sh->frag.qw += sh->qwLittle - leftDiff * sh->dqwdx;
                    sh->frag.rhow += sh->rhowLittle - leftDiff * sh->drhowdx;
                }
            } else {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                    sh->frag.color.r += sh->rLittle - leftDiff * sh->drdx;
                }
            }
            if (modeFlags & __GL_SHADE_STENCIL_TEST) {
                /* The implicit multiply is taken out of the loop */
                gc->polygon.shader.sbuf = (__GLstencilCell*)
                    ((GLubyte*) gc->polygon.shader.sbuf
                     + gc->polygon.shader.sbufLittle);
            }
            if (modeFlags & __GL_SHADE_DEPTH_ITER) {
                sh->frag.z += sh->zLittle - leftDiff * sh->dzdx;
            }
            if( modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
                sh->offsetZ += sh->zLittle - leftDiff * sh->dzdx;
                if(sh->dzdxf == 0.) /* approximate dzdx = 0 */
                  sh->outBoundCount += -sh->zLittle/__GL_PGON_OFFSET_NEAR_ZERO;
                else
                  sh->outBoundCount += 
                    -(sh->zLittle - leftDiff * sh->dzdx)/sh->dzdxf;
            }
            if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                sh->zbuf -= leftDiff;
                /* The implicit multiply is taken out of the loop */
                sh->zbuf = (__GLzValue*)
                    ((GLubyte*) sh->zbuf + sh->zbufLittle);
            }
            if (modeFlags & __GL_SHADE_SLOW_FOG) {
                sh->frag.f += sh->fLittle - leftDiff * sh->dfdx;
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
        } else {
            if (modeFlags & __GL_SHADE_SMOOTH) {
                sh->frag.color.r = ac->r + dx*sh->drdx + dy*sh->drdy;
                sh->rLittle = sh->drdy + little * sh->drdx;
                sh->rBig = sh->rLittle + sh->drdx;
            }
        }
        if (modeFlags & __GL_SHADE_DEPTH_ITER) {
            __GLfloat zLittle;
            if(gc->state.enables.general & __GL_POLYGON_OFFSET_FILL_ENABLE) {
                sh->frag.z = __glPolygonOffsetZ(gc, a, dx, dy);
            } else {
                sh->frag.z = (__GLzValue)
                             (a->window.z + dx*sh->dzdxf + dy*sh->dzdyf);
            }
            zLittle = sh->dzdyf + little * sh->dzdxf;
            sh->zLittle = (GLint)zLittle;
            sh->zBig = (GLint)(zLittle + sh->dzdxf);
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
        } else {
            if (modeFlags & __GL_SHADE_SMOOTH) {
                sh->frag.color.r = ac->r + dx*sh->drdx + dy*sh->drdy;
                sh->rLittle = sh->drdy + little * sh->drdx;
                sh->rBig = sh->rLittle - sh->drdx;
            }
        }
        if (modeFlags & __GL_SHADE_DEPTH_ITER) {
            __GLfloat zLittle;
            if(gc->state.enables.general & __GL_POLYGON_OFFSET_FILL_ENABLE) {
                sh->frag.z = __glPolygonOffsetZ(gc, a, dx, dy);
            } else {
                sh->frag.z = (__GLzValue)
                             (a->window.z + dx*sh->dzdxf + dy*sh->dzdyf);
            }
            zLittle = sh->dzdyf + little * sh->dzdxf;
            sh->zLittle = (GLint)zLittle;
            sh->zBig = (GLint)(zLittle - sh->dzdxf);
        }
        if (modeFlags & __GL_SHADE_SLOW_FOG) {
            sh->frag.f = aFog + dx*sh->dfdx + dy*sh->dfdy;
            sh->fLittle = sh->dfdy + little * sh->dfdx;
            sh->fBig = sh->fLittle - sh->dfdx;
        }
    }
}

/*
** Force f to have no more precision that the subpixel precision allows.
** Even though "f" is biased this still works and does not generate an overflow.
*/
#define __GL_FIX_PRECISION(f)   \
        ((__GLfloat)((GLint)((f) * (1 << gc->constants.subpixelBits))) \
        / (1 << gc->constants.subpixelBits))

void __glFillAntiAliasedTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
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
    __GLfloat area;
    __GLcoord asnap, bsnap, csnap;

    /* structure in order to communicate with FillAASubTriangle */
    __glFillAASubTriangleInfo info;

    /* used for antialiasing */
    GLint samples;

    /* Fetch some stuff we are going to reuse */
    dxAC = gc->polygon.shader.dxAC;
    dxBC = gc->polygon.shader.dxBC;
    dyAC = gc->polygon.shader.dyAC;
    dyBC = gc->polygon.shader.dyBC;

    info.gc = gc;

    /*
    ** figure out the area.
    **
    ** Recompute the area of the triangle after constraining the incoming
    ** coordinates to the subpixel precision.  The viewport bias gives
    ** more precision (typically) than the subpixel precision.  Because of
    ** this the algorithim below can fail to reject an essentially empty
    ** triangle and instead fill a large area.  The scan converter fill
    ** routines (eg polydraw.c) don't have this trouble because of the
    ** very nature of edge walking.
    **
    ** NOTE: Notice that here as in other places, when the area calculation
    ** is done we are careful to do it as a series of subtractions followed by
    ** multiplications.  This is done to guarantee that no overflow will
    ** occur (remember that the coordinates are biased by a potentially large
    ** number, and that multiplying two biased numbers will square the bias).
    */
    {
        __GLfloat ax, bx, cx, ay, by, cy;
        __GLfloat zero = __glZero;

        asnap.x = ax = __GL_FIX_PRECISION(a->window.x - __glHalf);
        bsnap.x = bx = __GL_FIX_PRECISION(b->window.x - __glHalf);
        csnap.x = cx = __GL_FIX_PRECISION(c->window.x - __glHalf);
        asnap.y = ay = __GL_FIX_PRECISION(a->window.y - __glHalf);
        bsnap.y = by = __GL_FIX_PRECISION(b->window.y - __glHalf);
        csnap.y = cy = __GL_FIX_PRECISION(c->window.y - __glHalf);
        asnap.z = a->window.z;
        bsnap.z = b->window.z;
        csnap.z = c->window.z;
        area = (ax - cx) * (by - cy) - (bx - cx) * (ay - cy);
        if (area == zero) {
            return;
        }
    }

    /* Pre-compute one over polygon area */
    oneOverArea = __glOne / area;

    /* Fetch some more stuff we are going to reuse */
    modeFlags = gc->polygon.shader.modeFlags;
    ac = a->color;
    bc = b->color;

    /*
     * precompute parameters used for antialiasing
     */
    FindLineEquation( &info.tm.ab, &asnap, &bsnap, &csnap );
    FindLineEquation( &info.tm.bc, &bsnap, &csnap, &asnap );
    FindLineEquation( &info.tm.ca, &csnap, &asnap, &bsnap );
    if( gc->state.hints.polygonSmooth == GL_NICEST ) {
        samples = 8;
    } else {
        samples = 4;
    }
    ComputeCoverageStuff( &info.cs, samples );

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

            /* for AA */
            FindPlaneEquation( gc, &info.rp, a, b, c, ac->r, bc->r, cc->r );
            FindPlaneEquation( gc, &info.gp, a, b, c, ac->g, bc->g, cc->g );
            FindPlaneEquation( gc, &info.bp, a, b, c, ac->b, bc->b, cc->b );
            FindPlaneEquation( gc, &info.ap, a, b, c, ac->a, bc->a, cc->a );
        } else {
            __GLcolor *flatColor = gc->vertex.provoking->color;
            gc->polygon.shader.frag.color.r = flatColor->r;
            gc->polygon.shader.frag.color.g = flatColor->g;
            gc->polygon.shader.frag.color.b = flatColor->b;
            gc->polygon.shader.frag.color.a = flatColor->a;
        }
        if (modeFlags & __GL_SHADE_TEXTURE) {
            __GLfloat dsAC, dsBC, dtAC, dtBC, dqwAC, dqwBC;
            __GLfloat rhowa, rhowb, rhowc;
            __GLfloat drhowAC, drhowBC;

            dsAC = a->texture[0].x - c->texture[0].x;
            dsBC = b->texture[0].x - c->texture[0].x;
            gc->polygon.shader.dsdx = dsAC * t2 - dsBC * t1;
            gc->polygon.shader.dsdy = dsBC * t3 - dsAC * t4;
            dtAC = a->texture[0].y - c->texture[0].y;
            dtBC = b->texture[0].y - c->texture[0].y;
            gc->polygon.shader.dtdx = dtAC * t2 - dtBC * t1;
            gc->polygon.shader.dtdy = dtBC * t3 - dtAC * t4;
            dqwAC = a->texture[0].w - c->texture[0].w;
            dqwBC = b->texture[0].w - c->texture[0].w;
            gc->polygon.shader.dqwdx = dqwAC * t2 - dqwBC * t1;
            gc->polygon.shader.dqwdy = dqwBC * t3 - dqwAC * t4;

            /* calc rho/w at the vertices */
            rhowa = (*gc->procs.calcPolygonRho)(gc, &gc->polygon.shader,
                                                a->texture[0].x,
                                                a->texture[0].y,
                                                a->texture[0].w)
                * a->texture[0].w;
            a->rhow = rhowa;
            rhowb = (*gc->procs.calcPolygonRho)(gc, &gc->polygon.shader,
                                                b->texture[0].x,
                                                b->texture[0].y,
                                                c->texture[0].w)
                * b->texture[0].w;
            b->rhow = rhowb;
            rhowc = (*gc->procs.calcPolygonRho)(gc, &gc->polygon.shader,
                                                c->texture[0].x,
                                                c->texture[0].y,
                                                c->texture[0].w)
                * c->texture[0].w;
            c->rhow = rhowc;

            drhowAC = rhowa - rhowc;
            drhowBC = rhowb - rhowc;
            gc->polygon.shader.drhowdx = drhowAC * t2 - drhowBC * t1;
            gc->polygon.shader.drhowdy = drhowBC * t3 - drhowAC * t4;

            /* for AA */
            {
                FindPlaneEquation(gc, &info.sp, a, b, c, 
                                  a->texture[0].x,
                                  b->texture[0].x,
                                  c->texture[0].x);
                FindPlaneEquation(gc, &info.tp, a, b, c, 
                                  a->texture[0].y,
                                  b->texture[0].y,
                                  c->texture[0].y);
                FindPlaneEquation(gc, &info.qwp, a, b, c, 
                                  a->texture[0].w,
                                  b->texture[0].w,
                                  c->texture[0].w);
                FindPlaneEquation(gc, &info.rhow, a, b, c,
                                  a->rhow, b->rhow, c->rhow);
            }
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

            /* for AA */
            FindPlaneEquation( gc, &info.rp, a, b, c, ac->r, bc->r, cc->r );
        } else {
            __GLcolor *flatColor = gc->vertex.provoking->color;
            gc->polygon.shader.frag.color.r = flatColor->r;
        }
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

        /* for AA */
        FindPlaneEquation( gc, &info.zp, a, b, c, 
                          a->window.z, b->window.z, c->window.z );
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

        /* for AA */
        FindPlaneEquation( gc, &info.ezp, a, b, c, a->eye.z, b->eye.z, c->eye.z );
    }

    /* Snap each y coordinate to its pixel center */
    aIY = (GLint) (a->window.y);
    bIY = (GLint) (b->window.y);
    cIY = (GLint) (c->window.y);

    __GL_LOCK_BUFFERS(gc);

    /*
    ** This algorithim always fills from bottom to top, left to right.
    ** Because of this, ccw triangles are inherently faster because
    ** the parameter values need not be recomputed.
    */
    dxAB = a->window.x - b->window.x;
    dyAB = a->window.y - b->window.y;
    if (ccw) {
        dxdyLeft = dxAC / dyAC;
        dy = (aIY + __glOne) - a->window.y;
        __glSnapXLeft(gc, a->window.x + dy*dxdyLeft, dxdyLeft);
        dx = (gc->polygon.shader.ixLeft + __glOne) - a->window.x;
        SetInitialParameters(gc, a, ac, aFog, dx, dy);
        if (aIY != bIY) {
                dxdyRight = dxAB / dyAB;
            __glSnapXRight(&gc->polygon.shader, a->window.x + dy*dxdyRight,
                       dxdyRight);
            FillAASubTriangle(aIY, bIY, &info );
        }

        if (bIY != cIY) {
            dxdyRight = dxBC / dyBC;
            dy = (bIY + __glOne) - b->window.y;
            __glSnapXRight(&gc->polygon.shader, b->window.x + dy*dxdyRight,
                       dxdyRight);
            FillAASubTriangle(bIY, cIY, &info );
        }
    } else {
        dxdyRight = dxAC / dyAC;
        dy = (aIY + __glOne) - a->window.y;
        __glSnapXRight(&gc->polygon.shader, a->window.x + dy*dxdyRight, dxdyRight);
        if (aIY != bIY) {
            dxdyLeft = dxAB / dyAB;
            __glSnapXLeft(gc, a->window.x + dy*dxdyLeft, dxdyLeft);
            dx = (gc->polygon.shader.ixLeft + __glOne) - a->window.x;
            SetInitialParameters(gc, a, ac, aFog, dx, dy);
            FillAASubTriangle(aIY, bIY, &info );
        }

        if (bIY != cIY) {
            dxdyLeft = dxBC / dyBC;
            dy = (bIY + __glOne) - b->window.y;
            __glSnapXLeft(gc, b->window.x + dy*dxdyLeft, dxdyLeft);
            dx = (gc->polygon.shader.ixLeft + __glOne) - b->window.x;
            SetInitialParameters(gc, b, bc, bFog, dx, dy);
            FillAASubTriangle(bIY, cIY, &info );
        }
    }
    __GL_UNLOCK_BUFFERS(gc);
}

