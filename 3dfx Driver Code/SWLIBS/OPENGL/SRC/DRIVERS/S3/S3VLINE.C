/*
** Copyright 1996, 1997, Silicon Graphics, Inc.
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
**
*/

#include "s3vcontext.h"
#include "global.h"
#include "xform.h"
#include "string.h"
#include "fr_modes.h"
#include "fr_tri.h"
#include "imports.h" /* for __GL_ABSF */
#include "s3virge.h"


void __glS3VRenderFlatLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    float y0, y1, x, x0, x1;
    int y0Int, y1Int;
    float dyr;
    float dxdy;
    float ysnap0;
    float FR_COLOR_ROUND = COLOR_ROUND_DITHER;
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;
    GLuint s3VirgeRegBase;
    volatile __glS3VLineEngineRegisters *lineEngine;
    volatile unsigned *lineCmd;
    unsigned r,g,b,a;
    float ftmp;
    double dtmp;
    unsigned x1Int, x0Int;
    int dyInt, dxInt;
    unsigned dir;
    int xdelta;

    /* Set the color registers */

    if (v0->window.y > v1->window.y) {
        x0 = v0->window.x;
        y0 = v0->window.y;
        
        x1 = v1->window.x;
        y1 = v1->window.y;
    }
    else {
        x0 = v1->window.x;
        y0 = v1->window.y;
        
        x1 = v0->window.x;
        y1 = v0->window.y;
    }

    x0Int = BiasedFloatToInt(x0, XY_FRAC_BITS);
    x1Int = BiasedFloatToInt(x1, XY_FRAC_BITS);
    y0Int = BiasedFloatToInt(y0, XY_FRAC_BITS);
    y1Int = BiasedFloatToInt(y1, XY_FRAC_BITS);
    dyInt = y0Int - y1Int;
    dxInt = x0Int - x1Int;

    if (!dxInt && !dyInt) return;

    if (dyInt) dyr = 1.0f/(y0-y1);
    else       dyr = 0.0f;

    dxdy = (float)(x0-x1)*dyr;
    s3VirgeRegBase = hwcx->regBase;
    lineEngine = hwcx->lineEngine;
    lineCmd    = (volatile unsigned *)(s3VirgeRegBase + 0xB100);

    ysnap0 = (1.0f - ((float) (y0Int + XY_BIAS + 1) - y0));
    x = x0 + -dxdy * ysnap0;
    WaitForQueue(10)
    *lineCmd =   S3D_COMMAND_3D |
                 VIRGE_CMD_NOOP;

    if (x0Int > x1Int) {
        dir = 0;
    }
    else {
        dir = 0x80000000;
    }
    xdelta = FloatToFixed(-dxdy, 20);
    lineEngine->YStart    = y0Int;
    lineEngine->endPoints = x1Int | (x0Int << 16);
    if (dyInt > dxInt) {
        lineEngine->XStart = BiasedFloatToFixed(x0) << (20 - XY_FRAC_BITS);
    }
    else {
        lineEngine->XStart = BiasedFloatToFixed(x) << (20 - XY_FRAC_BITS);
    }
    lineEngine->dXY       = xdelta;
   
    g = FloatToFixed(v1->color->g + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    b = FloatToFixed(v1->color->b + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    lineEngine->BGStart = (b >> 6) | ((g >> 6) << 16);

    r = FloatToFixed(v1->color->r + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    a = FloatToFixed(v1->color->a + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    lineEngine->ARStart = 0x7fff0000 | (r >> 6);

    lineEngine->YCount    = (dyInt+1) | dir;
    *lineCmd =   S3D_COMMAND_3D |
               VIRGE_CMD_LINE3D |
  	       VIRGE_ZFUNC_ALWAYS |
	      hwcx->hwBlendFunc |
	                     4 ;
}


void
__glS3VRenderSmoothLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    float y0, y1, x, x0, x1, r0, r1, g0, g1, b0, b1, a0, a1;
    int y0Int, y1Int;
    float dyr;
    float dxdy, drdy, dgdy, dbdy, dady;
    float ysnap0;
    float FR_COLOR_ROUND = COLOR_ROUND_DITHER;
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;
    GLuint s3VirgeRegBase;
    volatile __glS3VLineEngineRegisters *lineEngine;
    volatile unsigned *lineCmd;
    unsigned r,g,b,a;
    float ftmp;
    double dtmp;
    unsigned x1Int, x0Int;
    int dyInt, dxInt;
    unsigned dir;
    int xdelta;
    short rslope, gslope, bslope;

    /* Set the color registers */

    if (v0->window.y > v1->window.y) {
        x0 = v0->window.x;
        y0 = v0->window.y;
        
        r0 = v0->color->r;
        g0 = v0->color->g;
        b0 = v0->color->b;
        a0 = v0->color->a;
  
        x1 = v1->window.x;
        y1 = v1->window.y;

        r1 = v1->color->r;
        g1 = v1->color->g;
        b1 = v1->color->b;
        a1 = v1->color->a;
      
    }
    else {
        x0 = v1->window.x;
        y0 = v1->window.y;
        
        r0 = v1->color->r;
        g0 = v1->color->g;
        b0 = v1->color->b;
        a0 = v1->color->a;

        x1 = v0->window.x;
        y1 = v0->window.y;

        r1 = v0->color->r;
        g1 = v0->color->g;
        b1 = v0->color->b;
        a1 = v0->color->a;
    }

    x0Int = BiasedFloatToInt(x0, XY_FRAC_BITS);
    x1Int = BiasedFloatToInt(x1, XY_FRAC_BITS);
    y0Int = BiasedFloatToInt(y0, XY_FRAC_BITS);
    y1Int = BiasedFloatToInt(y1, XY_FRAC_BITS);
    dyInt = y0Int - y1Int;
    dxInt = x0Int - x1Int;

    if (!dxInt && !dyInt) return;

    if (dyInt) dyr = 1.0f/(y0-y1);
    else       dyr = 0.0f;

    dxdy = (float)(x0-x1)*dyr;
    drdy = (float)(r0-r1)*dyr;
    dgdy = (float)(g0-g1)*dyr;
    dbdy = (float)(b0-b1)*dyr;
    dady = (float)(a0-a1)*dyr;

    s3VirgeRegBase = hwcx->regBase;
    lineEngine = hwcx->lineEngine;
    lineCmd    = (volatile unsigned *)(s3VirgeRegBase + 0xB100);

    ysnap0 = (1.0f - ((float) (y0Int + XY_BIAS + 1) - y0));
    x = x0 + -dxdy * ysnap0;
    
    WaitForQueue(12)
    *lineCmd =   S3D_COMMAND_3D |
                 VIRGE_CMD_NOOP;

    if (x0Int > x1Int) {
        dir = 0;
    }
    else {
        dir = 0x80000000;
    }
    xdelta = FloatToFixed(-dxdy, 20);
    lineEngine->YStart    = y0Int;
    lineEngine->endPoints = x1Int | (x0Int << 16);
    if (dyInt > dxInt) {
        lineEngine->XStart = BiasedFloatToFixed(x0) << (20 - XY_FRAC_BITS);
    }
    else {
        lineEngine->XStart = BiasedFloatToFixed(x) << (20 - XY_FRAC_BITS);
    }
    lineEngine->dXY       = xdelta;
   
    g = FloatToFixed(g0 + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    b = FloatToFixed(b0 + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    lineEngine->BGStart = (b >> 6) | ((g >> 6) << 16);
    gslope = FloatToFixed(-dgdy + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    bslope = FloatToFixed(-dgdy + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    lineEngine->dBGY     = (bslope >> 6) | ((gslope>>6) << 16);

    r = FloatToFixed(r0 + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    a = FloatToFixed(a0 + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    lineEngine->ARStart = 0x7fff0000 | (r >> 6);
    rslope = FloatToFixed(-drdy + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    lineEngine->dARY    = (rslope >> 6);

    lineEngine->YCount    = (dyInt+1) | dir;
    *lineCmd =   S3D_COMMAND_3D |
               VIRGE_CMD_LINE3D |
  	       VIRGE_ZFUNC_ALWAYS |
	      hwcx->hwBlendFunc |
	                     4 ;
}


void
__glS3VRenderFlatDepthLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    float y0, y1, x, x0, x1, z0, z1;
    int y0Int, y1Int;
    float dyr;
    float dxdy, dzdy;
    float ysnap0;
    float FR_COLOR_ROUND = COLOR_ROUND_DITHER;
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;
    GLuint s3VirgeRegBase;
    volatile __glS3VLineEngineRegisters *lineEngine;
    volatile unsigned *lineCmd;
    unsigned r,g,b,a;
    float ftmp;
    double dtmp;
    unsigned x1Int, x0Int;
    int dyInt, dxInt;
    unsigned dir;
    int xdelta;

    /* Set the color registers */

    if (v0->window.y > v1->window.y) {
        x0 = v0->window.x;
        y0 = v0->window.y;
        z0 = v0->window.z;
        
        x1 = v1->window.x;
        y1 = v1->window.y;
        z1 = v1->window.z;
    }
    else {
        x0 = v1->window.x;
        y0 = v1->window.y;
        z0 = v1->window.z;
        
        x1 = v0->window.x;
        y1 = v0->window.y;
        z1 = v0->window.z;
    }

    x0Int = BiasedFloatToInt(x0, XY_FRAC_BITS);
    x1Int = BiasedFloatToInt(x1, XY_FRAC_BITS);
    y0Int = BiasedFloatToInt(y0, XY_FRAC_BITS);
    y1Int = BiasedFloatToInt(y1, XY_FRAC_BITS);
    dyInt = y0Int - y1Int;
    dxInt = x0Int - x1Int;

    if (!dxInt && !dyInt) return;

    if (dyInt) dyr = 1.0f/(y0-y1);
    else       dyr = 0.0f;

    dxdy = (float)(x0-x1)*dyr;
    dzdy = (float)(z0-z1)*dyr;
    s3VirgeRegBase = hwcx->regBase;
    lineEngine = hwcx->lineEngine;
    lineCmd    = (volatile unsigned *)(s3VirgeRegBase + 0xB100);

    ysnap0 = (1.0f - ((float) (y0Int + XY_BIAS + 1) - y0));
    x = x0 + -dxdy * ysnap0;
    
    WaitForQueue(12)
    *lineCmd =   S3D_COMMAND_3D |
                 VIRGE_CMD_NOOP;

    if (x0Int > x1Int) {
        dir = 0;
    }
    else {
        dir = 0x80000000;
    }
    xdelta = FloatToFixed(-dxdy, 20);
    lineEngine->YStart    = y0Int;
    lineEngine->endPoints = x1Int | (x0Int << 16);
    if (dyInt > dxInt) {
        lineEngine->XStart = BiasedFloatToFixed(x0) << (20 - XY_FRAC_BITS);
    }
    else {
        lineEngine->XStart    = BiasedFloatToFixed(x) << (20 - XY_FRAC_BITS);
    }
    lineEngine->dXY       = xdelta;
   
    g = FloatToFixed(v1->color->g + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    b = FloatToFixed(v1->color->b + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    lineEngine->BGStart = (b >> 6) | ((g >> 6) << 16);

    r = FloatToFixed(v1->color->r + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    a = FloatToFixed(v1->color->a + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    lineEngine->ARStart = 0x7fff0000 | (r >> 6);

    lineEngine->ZStart    = FloatToFixed(z0, 0);
    lineEngine->dZY       = FloatToFixed(-dzdy, 0);
    lineEngine->YCount    = (dyInt+1) | dir;

    *lineCmd =   S3D_COMMAND_3D |
               VIRGE_CMD_LINE3D |
  	          hwcx->hwZFunc |
	      hwcx->hwBlendFunc |
	                     4 ;
}


void
__glS3VRenderSmoothDepthLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    float y0, y1, x, x0, x1, z0, z1, r0, r1, g0, g1, b0, b1, a0, a1;
    int y0Int, y1Int;
    float dyr;
    float dxdy, dzdy, drdy, dgdy, dbdy, dady;
    float ysnap0;
    float FR_COLOR_ROUND = COLOR_ROUND_DITHER;
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;
    GLuint s3VirgeRegBase;
    volatile __glS3VLineEngineRegisters *lineEngine;
    volatile unsigned *lineCmd;
    unsigned r,g,b,a;
    float ftmp;
    double dtmp;
    unsigned x1Int, x0Int;
    int dyInt, dxInt;
    unsigned dir;
    int xdelta;
    short rslope, gslope, bslope;

    /* Set the color registers */

    if (v0->window.y > v1->window.y) {
        x0 = v0->window.x;
        y0 = v0->window.y;
        z0 = v0->window.y;
        
        r0 = v0->color->r;
        g0 = v0->color->g;
        b0 = v0->color->b;
        a0 = v0->color->a;
  
        x1 = v1->window.x;
        y1 = v1->window.y;
        z1 = v1->window.y;

        r1 = v1->color->r;
        g1 = v1->color->g;
        b1 = v1->color->b;
        a1 = v1->color->a;
    }
    else {
        x0 = v1->window.x;
        y0 = v1->window.y;
        z1 = v1->window.y;
        
        r0 = v1->color->r;
        g0 = v1->color->g;
        b0 = v1->color->b;
        a0 = v1->color->a;

        x1 = v0->window.x;
        y1 = v0->window.y;
        z0 = v0->window.y;

        r1 = v0->color->r;
        g1 = v0->color->g;
        b1 = v0->color->b;
        a1 = v0->color->a;
    }

    x0Int = BiasedFloatToInt(x0, XY_FRAC_BITS);
    x1Int = BiasedFloatToInt(x1, XY_FRAC_BITS);
    y0Int = BiasedFloatToInt(y0, XY_FRAC_BITS);
    y1Int = BiasedFloatToInt(y1, XY_FRAC_BITS);
    dyInt = y0Int - y1Int;
    dxInt = x0Int - x1Int;

    if (!dxInt && !dyInt) return;

    if (dyInt) dyr = 1.0f/(y0-y1);
    else       dyr = 0.0f;

    dxdy = (float)(x0-x1)*dyr;
    dzdy = (float)(z0-z1)*dyr;
    drdy = (float)(r0-r1)*dyr;
    dgdy = (float)(g0-g1)*dyr;
    dbdy = (float)(b0-b1)*dyr;
    dady = (float)(a0-a1)*dyr;

    s3VirgeRegBase = hwcx->regBase;
    lineEngine = hwcx->lineEngine;
    lineCmd    = (volatile unsigned *)(s3VirgeRegBase + 0xB100);

    ysnap0 = (1.0f - ((float) (y0Int + XY_BIAS + 1) - y0));
    x = x0 + -dxdy * ysnap0;

    WaitForQueue(14)
    *lineCmd =   S3D_COMMAND_3D |
                 VIRGE_CMD_NOOP;

    if (x0Int > x1Int) {
        dir = 0;
    }
    else {
        dir = 0x80000000;
    }
    xdelta = FloatToFixed(-dxdy, 20);

    lineEngine->YStart    = y0Int;
    lineEngine->endPoints = x1Int | (x0Int << 16);
    if (dyInt > dxInt) {
        lineEngine->XStart = BiasedFloatToFixed(x0) << (20 - XY_FRAC_BITS);
    }
    else {
        lineEngine->XStart    = BiasedFloatToFixed(x) << (20 - XY_FRAC_BITS);
    }
    lineEngine->dXY       = xdelta;
   
    g = FloatToFixed(g0 + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    b = FloatToFixed(b0 + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    lineEngine->BGStart = (b >> 6) | ((g >> 6) << 16);
    gslope = FloatToFixed(-dgdy + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    bslope = FloatToFixed(-dgdy + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    lineEngine->dBGY     = (bslope >> 6) | ((gslope>>6) << 16);

    r = FloatToFixed(r0 + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    a = FloatToFixed(a0 + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    lineEngine->ARStart = 0x7fff0000 | (r >> 6);
    rslope = FloatToFixed(-drdy + FR_COLOR_ROUND, COLOR_FRAC_BITS);
    lineEngine->dARY    = (rslope >> 6);

    lineEngine->ZStart    = FloatToFixed(z0, 0);
    lineEngine->dZY       = FloatToFixed(-dzdy, 0);
    lineEngine->YCount    = (dyInt+1) | dir;
    *lineCmd =   S3D_COMMAND_3D |
               VIRGE_CMD_LINE3D |
  	          hwcx->hwZFunc |
	      hwcx->hwBlendFunc |
	                     4 ;
}


