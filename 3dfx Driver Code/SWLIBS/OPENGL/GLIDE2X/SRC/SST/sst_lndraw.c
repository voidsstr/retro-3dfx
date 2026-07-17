#include <windows.h>
#include "context.h"
#include "global.h"
#include "glmath.h"
#include <glide.h>

#include "sst_globals.h"

void __glSSTRenderAntiAliasedLine(__GLcontext *gc, __GLvertex *v0, 
                                  __GLvertex *v1)
{
    GrVertex vtxA, vtxB;
    __GLvertex *pv;

    if (gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
        pv = v0;
    } else {
        pv = v1; /* provoking vertex */
    }
    vtxA.x = v0->window.x;
    vtxA.y = v0->window.y;
    vtxA.ooz = v0->window.z;
    vtxA.r = pv->color->r;
    vtxA.g = pv->color->g;
    vtxA.b = pv->color->b;
    vtxA.a = pv->color->a;

    vtxB.x = v1->window.x;
    vtxB.y = v1->window.y;
    vtxB.ooz = v1->window.z;
    vtxB.r = v1->color->r;
    vtxB.g = v1->color->g;
    vtxB.b = v1->color->b;
    vtxB.a = v1->color->a;

#ifdef __GL_BENCH_RASTER    
    if (!gc->noGlide)
#endif
    grAADrawLine(&vtxA, &vtxB);
}

void __glSSTRenderAliasedLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    GrVertex vtxA, vtxB;
    __GLvertex *pv;

    if (gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
        pv = v0;
    } else {
        pv = v1; /* provoking vertex */
    }
    vtxA.x = v0->window.x;
    vtxA.y = v0->window.y;
    vtxA.ooz = v0->window.z;
    vtxA.r = pv->color->r;
    vtxA.g = pv->color->g;
    vtxA.b = pv->color->b;
    vtxA.a = pv->color->a;

    vtxB.x = v1->window.x;
    vtxB.y = v1->window.y;
    vtxB.ooz = v1->window.z;
    vtxB.r = v1->color->r;
    vtxB.g = v1->color->g;
    vtxB.b = v1->color->b;
    vtxB.a = v1->color->a;

#ifdef __GL_BENCH_RASTER    
    if (!gc->noGlide)
#endif
    grDrawLine(&vtxA, &vtxB);
}

void __glSSTRenderAliasedWideLine(__GLcontext *gc, __GLvertex *v0,
                                  __GLvertex *v1)
{
    GrVertex vtxA, vtxB, vtxC;
    __GLvertex *p0, *p1, *p0c, *p1c;
    __GLfloat x0, x1, y0, y1, z0, z1;
    __GLfloat dx, dy, w;
    GLint ix, iy;
    GLint cullBit, frontBit;
    GLboolean flat, cw;

    flat = !(gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT);
    w = gc->state.line.aliasedWidth / 2.0;
    dx = v1->window.x - v0->window.x;
    dy = v1->window.y - v0->window.y;
    ix = *(GLint *)&dx;
    iy = *(GLint *)&dy;
    if (ix < 0) {
        ix ^= 0x80000000; /* ix = -ix */
    }
    if (iy < 0) {
        iy ^= 0x80000000; /* iy = -iy */
    }

    /* find a winding that we know won't be culled */
    cullBit = (gc->state.polygon.cull == GL_FRONT);
    frontBit = (gc->state.polygon.frontFaceDirection == GL_CW);
    if (gc->constants.yInverted) {
        cw = !(cullBit ^ frontBit);
    } else {
        cw = (cullBit ^ frontBit);
    }

    if (ix > iy) {
        /* X major line */

        /* p0 is the left vertex */
        if (dx > 0) {
            p0 = p0c = v0;
            p1 = p1c = v1;
        } else {
            p0 = p0c = v1;
            p1 = p1c = v0;
        }
        if (flat) {
            /* override color pointers with provoking vertex */
            p0c = p1c = v1;
        }
        x0 = p0->window.x;
        y0 = p0->window.y;
        z0 = p0->window.z;
        x1 = p1->window.x;
        y1 = p1->window.y;
        z1 = p1->window.z;

        vtxA.x = x0;
        vtxA.y = y0 + w;
        vtxA.ooz = z0;
        vtxA.r = p0c->color->r;
        vtxA.g = p0c->color->g;
        vtxA.b = p0c->color->b;
        vtxA.a = p0c->color->a;

        vtxB.x = x0;
        vtxB.y = y0 - w;
        vtxB.ooz = z0;
        vtxB.r = p0c->color->r;
        vtxB.g = p0c->color->g;
        vtxB.b = p0c->color->b;
        vtxB.a = p0c->color->a;

        vtxA.oow = vtxB.oow = p0->texture[0].w;
        vtxA.tmuvtx[0].sow = vtxB.tmuvtx[0].sow = p0->texture[0].x;
        vtxA.tmuvtx[0].tow = vtxB.tmuvtx[0].tow = p0->texture[0].y;

        vtxC.x = x1;
        vtxC.y = y1 + w;
        vtxC.ooz = z1;
        vtxC.r = p1c->color->r;
        vtxC.g = p1c->color->g;
        vtxC.b = p1c->color->b;
        vtxC.a = p1c->color->a;

        vtxC.oow = p1->texture[0].w;
        vtxC.tmuvtx[0].sow = p1->texture[0].x;
        vtxC.tmuvtx[0].tow = p1->texture[0].y;

        if (cw) {
            grDrawTriangle( &vtxC, &vtxB, &vtxA );
        } else {
            grDrawTriangle( &vtxA, &vtxB, &vtxC );
        }

        vtxA.x = x1;
        vtxA.y = y1 - w;
        vtxA.ooz = z1;
        vtxA.r = p1c->color->r;
        vtxA.g = p1c->color->g;
        vtxA.b = p1c->color->b;
        vtxA.a = p1c->color->a;

        vtxA.oow = p1->texture[0].w;
        vtxA.tmuvtx[0].sow = p1->texture[0].x;
        vtxA.tmuvtx[0].tow = p1->texture[0].y;

        if (cw) {
            grDrawTriangle( &vtxB, &vtxC, &vtxA );
        } else {
            grDrawTriangle( &vtxA, &vtxC, &vtxB );
        }
        
    } else {
        /* Y major line */

        /* p0 is the lower vertex */
        if (dy > 0) {
            p0 = p0c = v0;
            p1 = p1c = v1;
        } else {
            p0 = p0c = v1;
            p1 = p1c = v0;
        }
        if (flat) {
            /* override color pointers with provoking vertex */
            p0c = p1c = v1;
        }
        x0 = p0->window.x;
        y0 = p0->window.y;
        z0 = p0->window.z;
        x1 = p1->window.x;
        y1 = p1->window.y;
        z1 = p0->window.z;

        vtxA.x = x0 - w;
        vtxA.y = y0;
        vtxA.ooz = z0;
        vtxA.r = p0c->color->r;
        vtxA.g = p0c->color->g;
        vtxA.b = p0c->color->b;
        vtxA.a = p0c->color->a;

        vtxB.x = x0 + w;
        vtxB.y = y0;
        vtxB.ooz = z0;
        vtxB.r = p0c->color->r;
        vtxB.g = p0c->color->g;
        vtxB.b = p0c->color->b;
        vtxB.a = p0c->color->a;

        vtxA.oow = vtxB.oow = p0->texture[0].w;
        vtxA.tmuvtx[0].sow = vtxB.tmuvtx[0].sow = p0->texture[0].x;
        vtxA.tmuvtx[0].tow = vtxB.tmuvtx[0].tow = p0->texture[0].y;

        vtxC.x = x1 - w;
        vtxC.y = y1;
        vtxC.ooz = z1;
        vtxC.r = p1c->color->r;
        vtxC.g = p1c->color->g;
        vtxC.b = p1c->color->b;
        vtxC.a = p1c->color->a;

        vtxC.oow = p1->texture[0].w;
        vtxC.tmuvtx[0].sow = p1->texture[0].x;
        vtxC.tmuvtx[0].tow = p1->texture[0].y;

        if (cw) {
            grDrawTriangle( &vtxC, &vtxB, &vtxA );
        } else {
            grDrawTriangle( &vtxA, &vtxB, &vtxC );
        }

        vtxA.x = x1 + w;
        vtxA.y = y1;
        vtxA.ooz = z1;
        vtxA.r = p1c->color->r;
        vtxA.g = p1c->color->g;
        vtxA.b = p1c->color->b;
        vtxA.a = p1c->color->a;

        vtxA.oow = p1->texture[0].w;
        vtxA.tmuvtx[0].sow = p1->texture[0].x;
        vtxA.tmuvtx[0].tow = p1->texture[0].y;

        if (cw) {
            grDrawTriangle( &vtxB, &vtxC, &vtxA );
        } else {
            grDrawTriangle( &vtxA, &vtxC, &vtxB );
        }
    }
}

void __glSSTRenderAntiAliasedWideLine(__GLcontext *gc, __GLvertex *v0,
                                      __GLvertex *v1)
{
#define GLIDE_AA 1
    GrVertex vtxA, vtxB, vtxC, vtxD;
#if !GLIDE_AA
    GrVertex vtxE, vtxF, vtxG, vtxH;
#endif
    __GLfloat x0, x1, y0, y1;
    __GLfloat deltax, deltay, w;
    __GLfloat perpx, perpy, px, py, nx, ny, dirx, diry, dist;
    const __GLfloat phi = 0.707;
    GLint cullBit, frontBit;
    GLboolean cw;
    __GLvertex *pv;

    if (gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
        pv = v0;
    } else {
        pv = v1; /* provoking vertex */
    }
    w = gc->state.line.smoothWidth / 2.0;
    deltax = v1->window.x - v0->window.x;
    deltay = v1->window.y - v0->window.y;
    dist = __GL_SQRTF(deltax*deltax + deltay*deltay);

    /* normalized vector from v0 to v1 */
    dirx = deltax / dist;
    diry = deltay / dist;

    /* rotate 90 degrees counterclockwise, scale to half line width */
    perpx = -w * diry;
    perpy = w * dirx;

    /* rotate dirx, diry 45 degrees clockwise */
    nx = phi * ( phi*dirx + phi*diry);
    ny = phi * (-phi*dirx + phi*diry);

    /* rotate dirx, diry 45 degrees counterclockwise */
    px = -ny;
    py =  nx;

    x0 = v0->window.x;
    y0 = v0->window.y;
    x1 = v1->window.x;
    y1 = v1->window.y;

    /* find a winding that we know won't be culled */
    cullBit = (gc->state.polygon.cull == GL_FRONT);
    frontBit = (gc->state.polygon.frontFaceDirection == GL_CW);
    cw = (cullBit ^ frontBit);

#if GLIDE_AA
    vtxA.x = x0 + perpx;
    vtxA.y = y0 + perpy;
    vtxB.x = x0 - perpx;
    vtxB.y = y0 - perpy;

    vtxC.x = x1 + perpx;
    vtxC.y = y1 + perpy;
    vtxD.x = x1 - perpx;
    vtxD.y = y1 - perpy;

    vtxA.r = vtxB.r = pv->color->r;
    vtxA.g = vtxB.g = pv->color->g;
    vtxA.b = vtxB.b = pv->color->b;
    
    vtxC.r = vtxD.r = v1->color->r;
    vtxC.g = vtxD.g = v1->color->g;
    vtxC.b = vtxD.b = v1->color->b;

    vtxA.oow = vtxB.oow = v0->texture[0].w;
    vtxA.tmuvtx[0].sow = vtxB.tmuvtx[0].sow = v0->texture[0].x;
    vtxA.tmuvtx[0].tow = vtxB.tmuvtx[0].tow = v0->texture[0].y;

    vtxC.oow = vtxD.oow = v1->texture[0].w;
    vtxC.tmuvtx[0].sow = vtxD.tmuvtx[0].sow = v1->texture[0].x;
    vtxC.tmuvtx[0].tow = vtxD.tmuvtx[0].tow = v1->texture[0].y;

    vtxA.a = vtxB.a = vtxC.a = vtxD.a = 255.0f;

    if (cw) {
        grAADrawTriangle( &vtxC, &vtxB, &vtxA, FXFALSE, FXTRUE, FXTRUE);
        grAADrawTriangle( &vtxB, &vtxC, &vtxD, FXFALSE, FXTRUE, FXTRUE);
    } else {
        grAADrawTriangle( &vtxA, &vtxB, &vtxC, FXTRUE, FXFALSE, FXTRUE);
        grAADrawTriangle( &vtxD, &vtxC, &vtxB, FXTRUE, FXFALSE, FXTRUE);
    }
#else
    vtxA.x = x0 + perpx + nx;
    vtxA.y = y0 + perpy + ny;
    vtxB.x = x0 + perpx - nx;
    vtxB.y = y0 + perpy - ny;

    vtxC.x = x0 - perpx + px;
    vtxC.y = y0 - perpy + py;
    vtxD.x = x0 - perpx - px;
    vtxD.y = y0 - perpy - py;

    vtxE.x = x1 - perpx - nx;
    vtxE.y = y1 - perpy - ny;
    vtxF.x = x1 - perpx + nx;
    vtxF.y = y1 - perpy + ny;

    vtxG.x = x1 + perpx - px;
    vtxG.y = y1 + perpy - py;
    vtxH.x = x1 + perpx + px;
    vtxH.y = y1 + perpy + py;

    vtxA.ooz = vtxB.ooz = vtxC.ooz = vtxD.ooz = v0->window.z;
    vtxE.ooz = vtxF.ooz = vtxG.ooz = vtxH.ooz = v1->window.z;

    vtxA.r = vtxB.r = vtxC.r = vtxD.r = pv->color->r;
    vtxA.g = vtxB.g = vtxC.g = vtxD.g = pv->color->g;
    vtxA.b = vtxB.b = vtxC.b = vtxD.b = pv->color->b;
    
    vtxE.r = vtxF.r = vtxG.r = vtxH.r = v1->color->r;
    vtxE.g = vtxF.g = vtxG.g = vtxH.g = v1->color->g;
    vtxE.b = vtxF.b = vtxG.b = vtxH.b = v1->color->b;

    vtxA.a = vtxC.a = vtxE.a = vtxG.a = 255.0f;
    vtxB.a = vtxD.a = vtxF.a = vtxH.a = 0.0f;

    grDrawTriangle( &vtxA, &vtxC, &vtxG );
    grDrawTriangle( &vtxG, &vtxC, &vtxE );
    grDrawTriangle( &vtxA, &vtxB, &vtxC );
    grDrawTriangle( &vtxD, &vtxC, &vtxB );
    grDrawTriangle( &vtxC, &vtxD, &vtxE );
    grDrawTriangle( &vtxF, &vtxE, &vtxD );
    grDrawTriangle( &vtxE, &vtxF, &vtxG );
    grDrawTriangle( &vtxH, &vtxG, &vtxF );
    grDrawTriangle( &vtxG, &vtxH, &vtxA );
    grDrawTriangle( &vtxB, &vtxA, &vtxH );
#endif
}

void __glSSTRenderAntiAliasedLine_Tex(__GLcontext *gc, __GLvertex *v0, 
                                      __GLvertex *v1)
{
    GrVertex vtxA, vtxB;
    __GLvertex *pv;

    if (gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
        pv = v0;
    } else {
        pv = v1; /* provoking vertex */
    }
    vtxA.x = v0->window.x;
    vtxA.y = v0->window.y;
    vtxA.ooz = v0->window.z;
    vtxA.oow = v0->texture[0].w;
    vtxA.r = pv->color->r;
    vtxA.g = pv->color->g;
    vtxA.b = pv->color->b;
    vtxA.a = pv->color->a;
    vtxA.tmuvtx[0].sow = v0->texture[0].x;
    vtxA.tmuvtx[0].tow = v0->texture[0].y;
    

    vtxB.x = v1->window.x;
    vtxB.y = v1->window.y;
    vtxB.ooz = v1->window.z;
    vtxB.oow = v1->texture[0].w;
    vtxB.r = v1->color->r;
    vtxB.g = v1->color->g;
    vtxB.b = v1->color->b;
    vtxB.a = v1->color->a;
    vtxB.tmuvtx[0].sow = v1->texture[0].x;
    vtxB.tmuvtx[0].tow = v1->texture[0].y;

#ifdef __GL_BENCH_RASTER    
    if (!gc->noGlide)
#endif
    grAADrawLine(&vtxA, &vtxB);
}

void __glSSTRenderAliasedLine_Tex(__GLcontext *gc, __GLvertex *v0, 
                                  __GLvertex *v1)
{
    GrVertex vtxA, vtxB;
    __GLvertex *pv;

    if (gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
        pv = v0;
    } else {
        pv = v1; /* provoking vertex */
    }
    vtxA.x = v0->window.x;
    vtxA.y = v0->window.y;
    vtxA.ooz = v0->window.z;
    vtxA.oow = v0->texture[0].w;
    vtxA.r = pv->color->r;
    vtxA.g = pv->color->g;
    vtxA.b = pv->color->b;
    vtxA.a = pv->color->a;
    vtxA.tmuvtx[0].sow = v0->texture[0].x;
    vtxA.tmuvtx[0].tow = v0->texture[0].y;
    

    vtxB.x = v1->window.x;
    vtxB.y = v1->window.y;
    vtxB.ooz = v1->window.z;
    vtxB.oow = v1->texture[0].w;
    vtxB.r = v1->color->r;
    vtxB.g = v1->color->g;
    vtxB.b = v1->color->b;
    vtxB.a = v1->color->a;
    vtxB.tmuvtx[0].sow = v1->texture[0].x;
    vtxB.tmuvtx[0].tow = v1->texture[0].y;

#ifdef __GL_BENCH_RASTER    
    if (!gc->noGlide)
#endif
    grDrawLine(&vtxA, &vtxB);
}


