/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Revision: 2$
** $Date: 10/11/00 8:12:05 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

// some simple print macros
#define PV(v)	PXY(v.x),PXY(v.y)
#define PXY(x)	printFix("%4d.%x",x,SST_XY_FRACBITS)
#define PC(c)	printFix("%4d.%03x",c,SST_RGBA_FRACBITS)
#define PZ(z)	printFix64(diago.rgb==32?"%8x.%05x":"%6x.%07x",z,SST_Z64_FRACBITS)
#define PW(w)	printFix64("%6x.%08x",w,SST_W64_FRACBITS)

void printTriangle(int level, Triangle *t, int RGB, int A, int Z)
{
    gdbg_info(level,"triangle = %s,%s to %s,%s to %s,%s\n",
		PV(t->vA),PV(t->vB),PV(t->vC));
    if (RGB) {
	gdbg_info(level,"       r =   %s   to   %s   to   %s\n",
		PC(t->vA.r),PC(t->vB.r),PC(t->vC.r));
	gdbg_info(level,"       g =   %s   to   %s   to   %s\n",
		PC(t->vA.g),PC(t->vB.g),PC(t->vC.g));
	gdbg_info(level,"       b =   %s   to   %s   to   %s\n",
		PC(t->vA.b),PC(t->vB.b),PC(t->vC.b));
    }
    if (A) {
	gdbg_info(level,"       a =   %s   to   %s   to   %s\n",
		PC(t->vA.a),PC(t->vB.a),PC(t->vC.a));
    }
    if (Z > 0) {
	gdbg_info(level,"       z =   %s   to   %s   to   %s\n",
		PZ(t->vA.z64),PZ(t->vB.z64),PZ(t->vC.z64));
    }
    else if (Z < 0) {		// display W
	gdbg_info(level,"       w =   %s  to  %s  to  %s\n",
		PW(t->vA.w),PW(t->vB.w),PW(t->vC.w));
    }
}

void printTriangleSlopes(int level, Triangle *t, int RGB, int A, int Z)
{
    if (RGB) {
	gdbg_info(level,"    drdx =   %s    drdy = %s\n",PC(t->drdx),PC(t->drdy));
	gdbg_info(level,"    dgdx =   %s    dgdy = %s\n",PC(t->dgdx),PC(t->dgdy));
	gdbg_info(level,"    dbdx =   %s    dbdy = %s\n",PC(t->dbdx),PC(t->dbdy));
    }
    if (A) {
	gdbg_info(level,"    dadx =   %s    dady = %s\n",PC(t->dadx),PC(t->dady));
    }
    if (Z > 0 ) {
	gdbg_info(level,"    dzdx =   %s    dzdy = %s\n",PZ(t->dzdx64),PZ(t->dzdy64));
    }
    else if (Z < 0) {		// display W
	gdbg_info(level,"    dwdx =   %s    dwdy = %s\n",PW(t->dwdx),PW(t->dwdy));
    }
}

//---------------------------------------------------------------------
// truncate the integer part of a vertex's x-position to a column-of-eight boundary
void align8Fixed(long *x)
{
  int s = (*x>=0) ? 1 : -1;
  long xalign = s*(*x);
  if (iRandom(1))
    xalign |= (7*XY_ONE);    // right edge of column
  else
    xalign &= ~(7*XY_ONE);   // left edge of column
  xalign *= s;
  GDBG_INFO(30,"align8Fixed: x=%s -> %s\n",PXY(*x),PXY(xalign));
  *x = xalign;
}

void align8Float(float *fx)
{
  int i, s;
  float fxalign;
  double fint, frac;

  // extract integer part, fractional part, sign
  frac = modf(*fx,&fint);
  i = (int) fint;
  s = (i>=0) ? 1 : -1;
  i *= s;

  // align integer part to left/right edge of column
  if (iRandom(1))
    i |= 7;    // right edge of column
  else
    i &= ~7;   // left edge of column

  // recombine int, fraction, sign
  fxalign = (float)(s*i) + (float)frac;
  GDBG_INFO(30,"align8Float: fx=%f -> %f\n",*fx,fxalign);
  *fx = fxalign;
}

//---------------------------------------------------------------------
// generate a random triangle, possibly degenerate
// negative size forces the triangle to be right-angled
// the 'onscreen' argument controls whether the triangle is limited to onscreen
// NOTE: coordinates are fractional!
void randomTriangle(Triangle *t, int size, int onscreen)
{
    if (onscreen) {
	t->vA.x = iRandom(diago.xmaxscreen*XY_ONE-1);
	t->vA.y = iRandom(diago.ymaxscreen*XY_ONE-1);
    }
    else {
	t->vA.x = rRandom(-10*XY_ONE,10*XY_ONE);	// +- 10
	t->vA.y = rRandom(-10*XY_ONE,10*XY_ONE);
	// randomly choose left,right or bottom,top edge
	if (iRandom(1)) t->vA.x += diago.xmaxscreen*XY_ONE;
	if (iRandom(1)) t->vA.y += diago.ymaxscreen*XY_ONE;
    }
    if ( diago.columnOf8Align )
      if ( iRandom(3) == 1 ) 
	align8Fixed(&t->vA.x);

    t->vA.fx = fix2float(&t->vA.x,SST_XY_FRACBITS);
    t->vA.fy = fix2float(&t->vA.y,SST_XY_FRACBITS);
    randomTriangle1(t,size,onscreen);
}

// same as randomTriangle, but vertex A is already specified
void randomTriangle1(Triangle *t, int size, int onscreen)
{
    int tries=0;	// infinite loop sanity checker

    if (size >= 0) {	// free-form triangle
	do {
	    if (++tries > 1000) goto errExit;
	    t->vB.x = t->vA.x + rRandom(-size*XY_ONE,size*XY_ONE);
	    t->vB.y = t->vA.y + rRandom(-size*XY_ONE,size*XY_ONE);
	    if ( diago.columnOf8Align )
	      if ( iRandom(3) == 1 ) 
		align8Fixed(&t->vB.x);
	    t->vB.fx = fix2float(&t->vB.x,SST_XY_FRACBITS);
	    t->vB.fy = fix2float(&t->vB.y,SST_XY_FRACBITS);
	} while (onscreen && !ONSCREEN_FRAC(t->vB.x,t->vB.y));
	do {
	    if (++tries > 1000) goto errExit;
	    t->vC.x = t->vB.x + rRandom(-size*XY_ONE,size*XY_ONE);
	    t->vC.y = t->vB.y + rRandom(-size*XY_ONE,size*XY_ONE);
	    if ( diago.columnOf8Align )
	      if ( iRandom(3) == 1 ) 
		align8Fixed(&t->vC.x);
	    t->vC.fx = fix2float(&t->vC.x,SST_XY_FRACBITS);
	    t->vC.fy = fix2float(&t->vC.y,SST_XY_FRACBITS);
	} while (onscreen && !ONSCREEN_FRAC(t->vC.x,t->vC.y));
    }
    else {		// force a right angle
	double a, f = -size * XY_ONE;

    try_again:
	a = aRandom();
	if (++tries > 1000) goto errExit;
	t->vB.x = (long)(0.5 + t->vA.x + f * cos(a));
	t->vB.y = (long)(0.5 + t->vA.y + f * sin(a));
	t->vB.fx = fix2float(&t->vB.x,SST_XY_FRACBITS);
	t->vB.fy = fix2float(&t->vB.y,SST_XY_FRACBITS);
	if (onscreen && !ONSCREEN_FRAC(t->vB.x,t->vB.y)) goto try_again;
	t->vC.x = (long)(0.5 + t->vA.x - f * sin(a));
	t->vC.y = (long)(0.5 + t->vA.y + f * cos(a));
	t->vC.fx = fix2float(&t->vC.x,SST_XY_FRACBITS);
	t->vC.fy = fix2float(&t->vC.y,SST_XY_FRACBITS);
	if (onscreen && !ONSCREEN_FRAC(t->vC.x,t->vC.y)) goto try_again;
    }
    return;
errExit:
    gdbg_error("randomTriangle1","too many tries\n");
    DIAG_FAIL();
}

void randomFloatTriangle(Triangle *t, int size, int onscreen)
{
    int tries;		// infinite loop sanity counters
    int loops=0;

loop:
    if (++loops > 10) goto errExit;
    tries = 0;
    do { t->vA.fx = fexpRandom(-6,10); } while (t->vA.fx >= diago.xmaxscreen);
    do { t->vA.fy = fexpRandom(-6,10); } while (t->vA.fy >= diago.ymaxscreen);
    if ( diago.columnOf8Align )
      if ( iRandom(3) == 1 ) 
	align8Float(&t->vA.fx);
    t->vA.x = float2fix(t->vA.fx,SST_XY_FRACBITS);
    t->vA.y = float2fix(t->vA.fy,SST_XY_FRACBITS);
    if (size >= 0) {	// free-form triangle
	do {
	    if (++tries > 100) goto loop;
	    t->vB.fx = t->vA.fx + rfRandom(-size,size);
	    t->vB.fy = t->vA.fy + rfRandom(-size,size);
	    if ( diago.columnOf8Align )
	      if ( iRandom(3) == 1 ) 
		align8Float(&t->vB.fx);
	    t->vB.x = float2fix(t->vB.fx,SST_XY_FRACBITS);
	    t->vB.y = float2fix(t->vB.fy,SST_XY_FRACBITS);
	} while (onscreen && !ONSCREEN_FRAC(t->vB.x,t->vB.y));
	do {
	    if (++tries > 100) goto loop;
	    t->vC.fx = t->vB.fx + rfRandom(-size,size);
	    t->vC.fy = t->vB.fy + rfRandom(-size,size);
	    if ( diago.columnOf8Align )
	      if ( iRandom(3) == 1 ) 
		align8Float(&t->vC.fx);
	    t->vC.x = float2fix(t->vC.fx,SST_XY_FRACBITS);
	    t->vC.y = float2fix(t->vC.fy,SST_XY_FRACBITS);
	} while (onscreen && !ONSCREEN_FRAC(t->vC.x,t->vC.y));
    }
    else {		// force a right angle
	double a, f = -size;

    try_again:
	a = aRandom();
	if (++tries > 100) goto loop;
	t->vB.fx = (float)(t->vA.fx + f * cos(a));
	t->vB.fy = (float)(t->vA.fy + f * sin(a));
	t->vB.x = float2fix(t->vB.fx,SST_XY_FRACBITS);
	t->vB.y = float2fix(t->vB.fy,SST_XY_FRACBITS);
	if (onscreen && !ONSCREEN_FRAC(t->vB.x,t->vB.y)) goto try_again;
	t->vC.fx = (float)(t->vA.fx - f * sin(a));
	t->vC.fy = (float)(t->vA.fy + f * cos(a));
	t->vC.x = float2fix(t->vC.fx,SST_XY_FRACBITS);
	t->vC.y = float2fix(t->vC.fy,SST_XY_FRACBITS);
	if (onscreen && !ONSCREEN_FRAC(t->vC.x,t->vC.y)) goto try_again;
    }
    return;
errExit:
    gdbg_error("randomFloatTriangle","too many tries\n");
    DIAG_FAIL();
}

//----------------------------------------------------------------------
// initialize a new random triangle with non-zero area, optionally offscreen
// sometimes we generate the same triangle, 
// sometimes a totally random one, sometimes one close by
//----------------------------------------------------------------------
void randomStressTriangle(Triangle *t, int size, int onscreen, int i)
{
    do {
	if (i < 0) i = iRandom(3);
	gdbg_info(5,"randomStressTriangle(size=%d, onscreen=%d, case=%d, seed=%u\n",
			size,onscreen,i,getSeed());

	switch (i) {
	    case 0:		// bottom vertex close to top of last triangle
		t->vA.x = t->vC.x + rRandom(-1*XY_ONE,1*XY_ONE);
		t->vA.y = t->vC.y - iRandom(3*XY_ONE);
		t->vA.fx = fix2float(&t->vA.x,SST_XY_FRACBITS);
		t->vA.fy = fix2float(&t->vA.y,SST_XY_FRACBITS);
		// if vertex A is offscreen then giveup
		if (onscreen && !ONSCREEN_FRAC(t->vA.x,t->vA.y)) goto giveup;
		{
		    int n = 0;
		    do {
			if (n++ > 10) goto giveup;
			randomTriangle1(t, size, onscreen);
		    } while (t->vA.y > t->vB.y || t->vA.y > t->vC.y);
		}
		break;
    
	    case 1:		// totally new random triangle, mostly onscreen
	    giveup:
		randomTriangle(t,size,onscreen);
		break;

	    case 2:		// new triangle close to center of last
		t->vA.x = (t->vA.x + t->vB.x + t->vC.x)/3;
		t->vA.y = (t->vA.y + t->vB.y + t->vC.y)/3;
		t->vA.x = t->vA.x + rRandom(-2*XY_ONE,2*XY_ONE);
		t->vA.y = t->vA.y + rRandom(-2*XY_ONE,2*XY_ONE);
		t->vA.fx = fix2float(&t->vA.x,SST_XY_FRACBITS);
		t->vA.fy = fix2float(&t->vA.y,SST_XY_FRACBITS);
		// if vertex A is offscreen then giveup
		if (onscreen && !ONSCREEN_FRAC(t->vA.x,t->vA.y)) goto giveup;

		randomTriangle1(t,size,onscreen);
		break;

	    case 3:
	    default:		// exact same triangle
		break;
      }
      areaTriangle(t);		// compute the area (before setup)
    } while (t->area == 0);

    randomFloatRgbaTriangle(t);			// random colors
    randomFloatZTriangle(t);			// random Z
    randomFloatWTriangle(t);			// random Z
    setupFloatTriangle(t,1,1,1,1);		// setup RGBAZ slopes
    sortTriangle(t);				// sort it
    printTriangle(3,t,1,1,1);
    printTriangleSlopes(4,t,1,1,1);
}

// generate random RGBA values for a triangle
void randomRgbaTriangle(Triangle *t)
{
    t->vA.r = iRandom(255*(1<<SST_RGBA_FRACBITS));
    t->vA.g = iRandom(255*(1<<SST_RGBA_FRACBITS));
    t->vA.b = iRandom(255*(1<<SST_RGBA_FRACBITS));
    t->vA.a = iRandom(255*(1<<SST_RGBA_FRACBITS));
    t->vB.r = iRandom(255*(1<<SST_RGBA_FRACBITS));
    t->vB.g = iRandom(255*(1<<SST_RGBA_FRACBITS));
    t->vB.b = iRandom(255*(1<<SST_RGBA_FRACBITS));
    t->vB.a = iRandom(255*(1<<SST_RGBA_FRACBITS));
    t->vC.r = iRandom(255*(1<<SST_RGBA_FRACBITS));
    t->vC.g = iRandom(255*(1<<SST_RGBA_FRACBITS));
    t->vC.b = iRandom(255*(1<<SST_RGBA_FRACBITS));
    t->vC.a = iRandom(255*(1<<SST_RGBA_FRACBITS));
}

void randomFloatRgbaTriangle(Triangle *t)
{
    t->vA.fr = fexpRandom(-2,SST_RGBA_INTBITS);
    t->vA.fg = fexpRandom(-3,SST_RGBA_INTBITS);
    t->vA.fb = fexpRandom(-4,SST_RGBA_INTBITS);
    t->vA.fa = fexpRandom(-5,SST_RGBA_INTBITS);
    t->vB.fr = fexpRandom(-5,SST_RGBA_INTBITS);
    t->vB.fg = fexpRandom(-4,SST_RGBA_INTBITS);
    t->vB.fb = fexpRandom(-3,SST_RGBA_INTBITS);
    t->vB.fa = fexpRandom(-2,SST_RGBA_INTBITS);
    t->vC.fr = fexpRandom(-1,SST_RGBA_INTBITS);
    t->vC.fg = fexpRandom(-3,SST_RGBA_INTBITS);
    t->vC.fb = fexpRandom(-2,SST_RGBA_INTBITS);
    t->vC.fa = fexpRandom(-4,SST_RGBA_INTBITS);
    t->vA.r = float2fix(t->vA.fr,SST_RGBA_FRACBITS);
    t->vA.g = float2fix(t->vA.fg,SST_RGBA_FRACBITS);
    t->vA.b = float2fix(t->vA.fb,SST_RGBA_FRACBITS);
    t->vA.a = float2fix(t->vA.fa,SST_RGBA_FRACBITS);
    t->vB.r = float2fix(t->vB.fr,SST_RGBA_FRACBITS);
    t->vB.g = float2fix(t->vB.fg,SST_RGBA_FRACBITS);
    t->vB.b = float2fix(t->vB.fb,SST_RGBA_FRACBITS);
    t->vB.a = float2fix(t->vB.fa,SST_RGBA_FRACBITS);
    t->vC.r = float2fix(t->vC.fr,SST_RGBA_FRACBITS);
    t->vC.g = float2fix(t->vC.fg,SST_RGBA_FRACBITS);
    t->vC.b = float2fix(t->vC.fb,SST_RGBA_FRACBITS);
    t->vC.a = float2fix(t->vC.fa,SST_RGBA_FRACBITS);
}

// generate random Z values for a triangle
void randomZTriangle(Triangle *t)
{
    t->vA.z64 = (iRandom(1<<(SST_Z_INTBITS+SST_Z_FRACBITS))-1);
    t->vA.z64 <<= SST_Z64_SIZE-SST_Z_16BPP_SIZE;
    t->vB.z64 = (iRandom(1<<(SST_Z_INTBITS+SST_Z_FRACBITS))-1);
    t->vB.z64 <<= SST_Z64_SIZE-SST_Z_16BPP_SIZE;
    t->vC.z64 = (iRandom(1<<(SST_Z_INTBITS+SST_Z_FRACBITS))-1);
    t->vC.z64 <<= SST_Z64_SIZE-SST_Z_16BPP_SIZE;
}

void randomFloatZTriangle(Triangle *t)
{
    t->vA.fz = fexpRandom(-5,SST_Z_INTBITS);
    t->vB.fz = fexpRandom(-5,SST_Z_INTBITS);
    t->vC.fz = fexpRandom(-5,SST_Z_INTBITS);
    t->vA.z64 = float2fix64(t->vA.fz,SST_Z64_FRACBITS);
    t->vB.z64 = float2fix64(t->vB.fz,SST_Z64_FRACBITS);
    t->vC.z64 = float2fix64(t->vC.fz,SST_Z64_FRACBITS);
}

void randomFloatWTriangle(Triangle *t)
{
    int minexp = diago.rgb==32 ? -30 : -18;
    t->vA.fw = fexpRandom(minexp,0);
    t->vB.fw = fexpRandom(minexp,0);
    t->vC.fw = fexpRandom(minexp,0);
    t->vA.w = float2fix64(t->vA.fw,SST_W64_FRACBITS);
    t->vB.w = float2fix64(t->vB.fw,SST_W64_FRACBITS);
    t->vC.w = float2fix64(t->vC.fw,SST_W64_FRACBITS);
}

//---------------------------------------------------------------------
static float plusMinus(void)
{
    return iRandom(1) ? 1.0F : -1.0F;
}

// GMT: we now allow negative W values as all the code is in sync
void randomFloatStwTriangle_Setup(Triangle *t)
{
    float s;
    double dx,dy;

    gdbg_info(5,"randomFloatStwTriangle_Setup: seed=%u\n",getSeed());
    if (diago.perspective) {
	t->vA.fw = fexpRandom(-30,0);
	if (iRandom(4)==0) t->vA.fw = -t->vA.fw;	// negative W once in a while
	s = plusMinus();
	t->vA.fs = s * t->vA.fw * rfRandom(1,300);
	s = plusMinus();
	t->vA.ft = s * t->vA.fw * rfRandom(1,300);
	s = plusMinus();
	t->fdsdx = s * t->vA.fs * fexpRandom(-8,1);	// random slopes
	s = plusMinus();
	t->fdsdy = s * t->vA.fs * fexpRandom(-8,1);
	s = plusMinus();
	t->fdtdx = s * t->vA.ft * fexpRandom(-8,1);
	s = plusMinus();
	t->fdtdy = s * t->vA.ft * fexpRandom(-8,1);
	s = plusMinus();
	t->fdwdx = s * t->vA.fw * fexpRandom(-8,1);
	s = plusMinus();
	t->fdwdy = s * t->vA.fw * fexpRandom(-8,1);
    }
    else {
	s = plusMinus();
	t->vA.fs = s * fexpRandom(-3,16);	// random ST vertex
	s = plusMinus();
	t->vA.ft = s * fexpRandom(-3,16);
	t->vA.fw = fexpRandom(-23,0);
	if (iRandom(4)==0) t->vA.fw = -t->vA.fw;	// negate W once in a while
	// NOTE we can let S,T wrap around legal range
	s = plusMinus();
	t->fdsdx = s * fexpRandom(-3,16);	// random slopes
	s = plusMinus();
	t->fdsdy = s * fexpRandom(-3,16);
	s = plusMinus();
	t->fdtdx = s * fexpRandom(-3,16);
	s = plusMinus();
	t->fdtdy = s * fexpRandom(-3,16);
	t->fdwdx = 0.0F;
	t->fdwdy = 0.0F;
    }
    dx = (double)(t->vB.x - t->vA.x);		// calculate vB,vC
    dy = (double)(t->vB.y - t->vA.y);		// from vA and slopes
    t->vB.fs = (float)(t->vA.fs + (dx * t->fdsdx)/XY_ONE + (dy * t->fdsdy)/XY_ONE);
    t->vB.ft = (float)(t->vA.ft + (dx * t->fdtdx)/XY_ONE + (dy * t->fdtdy)/XY_ONE);
    t->vB.fw = (float)(t->vA.fw + (dx * t->fdwdx)/XY_ONE + (dy * t->fdwdy)/XY_ONE);
    dx = (double)(t->vC.x - t->vA.x);
    dy = (double)(t->vC.y - t->vA.y);
    t->vC.fs = (float)(t->vA.fs + (dx * t->fdsdx)/XY_ONE + (dy * t->fdsdy)/XY_ONE);
    t->vC.ft = (float)(t->vA.ft + (dx * t->fdtdx)/XY_ONE + (dy * t->fdtdy)/XY_ONE);
    t->vC.fw = (float)(t->vA.fw + (dx * t->fdwdx)/XY_ONE + (dy * t->fdwdy)/XY_ONE);

    t->vA.s = float2fix64(t->vA.fs,SST_ST64_FRACBITS);
    t->vA.t = float2fix64(t->vA.ft,SST_ST64_FRACBITS);
    t->vA.w = float2fix64(t->vA.fw,SST_W64_FRACBITS);
    t->vB.s = float2fix64(t->vB.fs,SST_ST64_FRACBITS);
    t->vB.t = float2fix64(t->vB.ft,SST_ST64_FRACBITS);
    t->vB.w = float2fix64(t->vB.fw,SST_W64_FRACBITS);
    t->vC.s = float2fix64(t->vC.fs,SST_ST64_FRACBITS);
    t->vC.t = float2fix64(t->vC.ft,SST_ST64_FRACBITS);
    t->vC.w = float2fix64(t->vC.fw,SST_W64_FRACBITS);
    t->dsdx = float2fix64(t->fdsdx,SST_ST64_FRACBITS);
    t->dsdy = float2fix64(t->fdsdy,SST_ST64_FRACBITS);
    t->dtdx = float2fix64(t->fdtdx,SST_ST64_FRACBITS);
    t->dtdy = float2fix64(t->fdtdy,SST_ST64_FRACBITS);
    t->dwdx = float2fix64(t->fdwdx,SST_W64_FRACBITS);
    t->dwdy = float2fix64(t->fdwdy,SST_W64_FRACBITS);
}

//---------------------------------------------------------------------
// now compute signed area of the triangle, area > 0 impies ccw
void areaTriangle(Triangle *t)
{
    FxI64 dxAB, dyAB, dxBC, dyBC;

    dxAB = t->vA.x - t->vB.x;
    dyAB = t->vA.y - t->vB.y;
    dxBC = t->vB.x - t->vC.x;
    dyBC = t->vB.y - t->vC.y;
    dxAB = dxAB * dyBC - dxBC * dyAB;	// NOTE: result is in .8 format
    if (dxAB > 0x7FFFFFFF || dxAB < (signed int)0x80000000)
	GDBG_ERROR("areaTriangle", "invalid area = %d\n", dxAB);
    t->area = (int)dxAB;
}

// compute the RGBAZ slopes for a triangle, assume area already computed
void setupTriangle(Triangle *t, int RGB, int A, int Z)
{
    float ooarea;
    float dxAB, dyAB, dxBC, dyBC;

    if(t->area != 0)
      ooarea = (float)(XY_ONE)/t->area;
    else
      ooarea = 1000;

    dxAB = (float)(t->vA.x - t->vB.x);
    dyAB = (float)(t->vA.y - t->vB.y);
    dxBC = (float)(t->vB.x - t->vC.x);
    dyBC = (float)(t->vB.y - t->vC.y);

    if (RGB) {
	t->drdx = (long)(((t->vA.r-t->vB.r) * dyBC - (t->vB.r-t->vC.r) * dyAB) * ooarea);
	t->drdy = (long)(((t->vB.r-t->vC.r) * dxAB - (t->vA.r-t->vB.r) * dxBC) * ooarea);
	t->dgdx = (long)(((t->vA.g-t->vB.g) * dyBC - (t->vB.g-t->vC.g) * dyAB) * ooarea);
	t->dgdy = (long)(((t->vB.g-t->vC.g) * dxAB - (t->vA.g-t->vB.g) * dxBC) * ooarea);
	t->dbdx = (long)(((t->vA.b-t->vB.b) * dyBC - (t->vB.b-t->vC.b) * dyAB) * ooarea);
	t->dbdy = (long)(((t->vB.b-t->vC.b) * dxAB - (t->vA.b-t->vB.b) * dxBC) * ooarea);
    }
    if (A) {
	t->dadx = (long)(((t->vA.a-t->vB.a) * dyBC - (t->vB.a-t->vC.a) * dyAB) * ooarea);
	t->dady = (long)(((t->vB.a-t->vC.a) * dxAB - (t->vA.a-t->vB.a) * dxBC) * ooarea);
    }
    if (Z) {
	t->dzdx64 = (FxI64)(((t->vA.z64-t->vB.z64) * dyBC - (t->vB.z64-t->vC.z64) * dyAB) * ooarea);
	t->dzdy64 = (FxI64)(((t->vB.z64-t->vC.z64) * dxAB - (t->vA.z64-t->vB.z64) * dxBC) * ooarea);
	// mask off any bits that cannot be set via the integer register
	t->dzdx64 &= ~FX_MASK64(SST_Z64_SIZE-SST_Z_32BPP_SIZE);
	t->dzdy64 &= ~FX_MASK64(SST_Z64_SIZE-SST_Z_32BPP_SIZE);
    }
}

void tsu_gradient(FxU32 p0, FxU32 p1, FxU32 p2,
	FxU32 dxAB, FxU32 dyAB, FxU32 dxBC, FxU32 dyBC, FxU32 ooarea,
	FxU32 *dpdx, FxU32 *dpdy)
{
	FxU32 dpAB, dpBC;
	FxU32   n1, n2, n3, n4;

	tsu_clamp(&p0);
	tsu_clamp(&p1);
	tsu_clamp(&p2);
	tsu_floatsub(p0,p1,&dpAB);
	tsu_floatsub(p1,p2,&dpBC);
	tsu_floatmul(dpAB, dxBC, &n1);
	tsu_floatmul(dpAB, dyBC, &n2);
	tsu_floatmul(dpBC, dxAB, &n3);
	tsu_floatmul(dpBC, dyAB, &n4);
	tsu_floatsub(n2, n4, dpdx);
	tsu_floatsub(n3, n1, dpdy);
	tsu_floatmul(*dpdx, ooarea, dpdx);
	tsu_floatmul(*dpdy, ooarea, dpdy);
}

#define TSU_GRADIENT(p,dpdx,dpdy) \
	tsu_gradient(*(FxU32 *)&t->vA.p,*(FxU32 *)&t->vB.p,*(FxU32 *)&t->vC.p, \
			dxAB,dyAB,dxBC,dyBC,ooarea,\
			(FxU32 *)&t->dpdx,(FxU32 *)&t->dpdy)

void setupFloatTriangle(Triangle *t, int RGB, int A, int Z, int W)
{
#if 1
    FxU32 dxAB, dxBC, dyAB, dyBC;
    FxU32 area,ooarea, im1,im2;

    //----------------------------------------------------------------------
    // compute area first (like the HW) and cull
    // NOTE: vArray.xy are already snapped to .4 precision
    tsu_floatsub(*(FxU32 *)&t->vA.fx, *(FxU32 *)&t->vB.fx, &dxAB);
    tsu_floatsub(*(FxU32 *)&t->vA.fy, *(FxU32 *)&t->vB.fy, &dyAB);
    tsu_floatsub(*(FxU32 *)&t->vB.fx, *(FxU32 *)&t->vC.fx, &dxBC);
    tsu_floatsub(*(FxU32 *)&t->vB.fy, *(FxU32 *)&t->vC.fy, &dyBC);

    tsu_floatmul(dxAB, dyBC, &im1);
    tsu_floatmul(dxBC, dyAB, &im2);
    tsu_floatsub(im1, im2, &area);
    tsu_floatrecip(area, &ooarea);		// invert the area
    if (RGB) {
	TSU_GRADIENT(fr,fdrdx,fdrdy);
	TSU_GRADIENT(fg,fdgdx,fdgdy);
	TSU_GRADIENT(fb,fdbdx,fdbdy);
	t->drdx = float2fix(t->fdrdx,SST_RGBA_FRACBITS);
	t->drdy = float2fix(t->fdrdy,SST_RGBA_FRACBITS);
	t->dgdx = float2fix(t->fdgdx,SST_RGBA_FRACBITS);
	t->dgdy = float2fix(t->fdgdy,SST_RGBA_FRACBITS);
	t->dbdx = float2fix(t->fdbdx,SST_RGBA_FRACBITS);
	t->dbdy = float2fix(t->fdbdy,SST_RGBA_FRACBITS);
    }
    if (A) {
	TSU_GRADIENT(fa,fdadx,fdady);
	t->dadx = float2fix(t->fdadx,SST_RGBA_FRACBITS);
	t->dady = float2fix(t->fdady,SST_RGBA_FRACBITS);
    }
    if (Z) {
	TSU_GRADIENT(fz,fdzdx,fdzdy);
	if (diago.rgb < 32) {
	    t->dzdx64 = float2fix64(t->fdzdx,SST_Z_16BPP_FRACBITS);
	    t->dzdx64 <<= SST_Z64_FRACBITS_16BPP - SST_Z_16BPP_FRACBITS;
	    t->dzdy64 = float2fix64(t->fdzdy,SST_Z_16BPP_FRACBITS);
	    t->dzdy64 <<= SST_Z64_FRACBITS_16BPP - SST_Z_16BPP_FRACBITS;
	}
	else {
	    t->dzdx64 = float2fix64(t->fdzdx,SST_Z64_FRACBITS);
	    t->dzdy64 = float2fix64(t->fdzdy,SST_Z64_FRACBITS);
	}
    }
    if (W) {	// always setup all 3 of STW
	TSU_GRADIENT(fw,fdwdx,fdwdy);
	t->dwdx = float2fix64(t->fdwdx,SST_W64_FRACBITS);
	t->dwdy = float2fix64(t->fdwdy,SST_W64_FRACBITS);
	TSU_GRADIENT(fs,fdsdx,fdsdy);
	t->dsdx = float2fix64(t->fdsdx,SST_ST64_FRACBITS);
	t->dsdy = float2fix64(t->fdsdy,SST_ST64_FRACBITS);
	TSU_GRADIENT(ft,fdtdx,fdtdy);
	t->dtdx = float2fix64(t->fdtdx,SST_ST64_FRACBITS);
	t->dtdy = float2fix64(t->fdtdy,SST_ST64_FRACBITS);
    }
#else
    float ooarea;
    float dxAC, dyAC, dxBC, dyBC;

    ooarea = (float)XY_ONE/t->area;
    dxAC = (float)(t->vA.x - t->vC.x);
    dyAC = (float)(t->vA.y - t->vC.y);
    dxBC = (float)(t->vB.x - t->vC.x);
    dyBC = (float)(t->vB.y - t->vC.y);

    if (RGB) {
	t->fdrdx = ((t->vA.fr-t->vC.fr) * dyBC - (t->vB.fr-t->vC.fr) * dyAC) * ooarea;
	t->fdrdy = ((t->vB.fr-t->vC.fr) * dxAC - (t->vA.fr-t->vC.fr) * dxBC) * ooarea;
	t->fdgdx = ((t->vA.fg-t->vC.fg) * dyBC - (t->vB.fg-t->vC.fg) * dyAC) * ooarea;
	t->fdgdy = ((t->vB.fg-t->vC.fg) * dxAC - (t->vA.fg-t->vC.fg) * dxBC) * ooarea;
	t->fdbdx = ((t->vA.fb-t->vC.fb) * dyBC - (t->vB.fb-t->vC.fb) * dyAC) * ooarea;
	t->fdbdy = ((t->vB.fb-t->vC.fb) * dxAC - (t->vA.fb-t->vC.fb) * dxBC) * ooarea;
	t->drdx = float2fix(t->fdrdx,SST_RGBA_FRACBITS);
	t->drdy = float2fix(t->fdrdy,SST_RGBA_FRACBITS);
	t->dgdx = float2fix(t->fdgdx,SST_RGBA_FRACBITS);
	t->dgdy = float2fix(t->fdgdy,SST_RGBA_FRACBITS);
	t->dbdx = float2fix(t->fdbdx,SST_RGBA_FRACBITS);
	t->dbdy = float2fix(t->fdbdy,SST_RGBA_FRACBITS);
    }
    if (A) {
	t->fdadx = ((t->vA.fa-t->vC.fa) * dyBC - (t->vB.fa-t->vC.fa) * dyAC) * ooarea;
	t->fdady = ((t->vB.fa-t->vC.fa) * dxAC - (t->vA.fa-t->vC.fa) * dxBC) * ooarea;
	t->dadx = float2fix(t->fdadx,SST_RGBA_FRACBITS);
	t->dady = float2fix(t->fdady,SST_RGBA_FRACBITS);
    }
    if (Z) {
	t->fdzdx = ((t->vA.fz-t->vC.fz) * dyBC - (t->vB.fz-t->vC.fz) * dyAC) * ooarea;
	t->fdzdy = ((t->vB.fz-t->vC.fz) * dxAC - (t->vA.fz-t->vC.fz) * dxBC) * ooarea;
	t->dzdx64 = float2fix64(t->fdzdx,SST_Z64_FRACSHIFT);
	t->dzdy64 = float2fix64(t->fdzdy,SST_Z64_FRACSHIFT);
    }
    if (W) {
	t->fdwdx = ((t->vA.fw-t->vC.fw) * dyBC - (t->vB.fw-t->vC.fw) * dyAC) * ooarea;
	t->fdwdy = ((t->vB.fw-t->vC.fw) * dxAC - (t->vA.fw-t->vC.fw) * dxBC) * ooarea;
	t->dwdx = float2fix64(t->fdwdx,SST_W64_FRACBITS);
	t->dwdy = float2fix64(t->fdwdy,SST_W64_FRACBITS);

	t->fdsdx = ((t->vA.fs-t->vC.fs) * dyBC - (t->vB.fs-t->vC.fs) * dyAC) * ooarea;
	t->fdsdy = ((t->vB.fs-t->vC.fs) * dxAC - (t->vA.fs-t->vC.fs) * dxBC) * ooarea;
	t->dsdx = float2fix64(t->fdsdx,SST_W64_FRACBITS);
	t->dsdy = float2fix64(t->fdsdy,SST_W64_FRACBITS);

	t->fdtdx = ((t->vA.ft-t->vC.ft) * dyBC - (t->vB.ft-t->vC.ft) * dyAC) * ooarea;
	t->fdtdy = ((t->vB.ft-t->vC.ft) * dxAC - (t->vA.ft-t->vC.ft) * dxBC) * ooarea;
	t->dtdx = float2fix64(t->fdtdx,SST_W64_FRACBITS);
	t->dtdy = float2fix64(t->fdtdy,SST_W64_FRACBITS);
    }
#endif
}

//---------------------------------------------------------------------
// draw a triangle, assumes vertices already sorted in Y
void drawTriangle(SstRegs *sst, Triangle *t, int RGB, int A, int Z)
{
  //setPixelsPerClock toggles between 1 and 2 pixels per clock rendering
  //if appropriate (i.e. --pixelsPerClock <= 0)
  setPixelsPerClock(sst);

    SET(sst->vA.x,t->vA.x);
    SET(sst->vA.y,t->vA.y);
    SET(sst->vB.x,t->vB.x);
    SET(sst->vB.y,t->vB.y);
    SET(sst->vC.x,t->vC.x);
    SET(sst->vC.y,t->vC.y);
    if (RGB) {
	SET(sst->r,t->vA.r);
	SET(sst->g,t->vA.g);
	SET(sst->b,t->vA.b);
	SET(sst->drdx,t->drdx);
	SET(sst->dgdx,t->dgdx);
	SET(sst->dbdx,t->dbdx);
	SET(sst->drdy,t->drdy);
	SET(sst->dgdy,t->dgdy);
	SET(sst->dbdy,t->dbdy);
    }
    if (A) {
	SET(sst->a,t->vA.a);
	SET(sst->dadx,t->dadx);
	SET(sst->dady,t->dady);
    }
    if (Z) {
	SET(sst->z,(FxU32)(t->vA.z64>>(SST_Z64_SIZE-SST_Z_16BPP_SIZE)));
	SET(sst->dzdx,(FxU32)(t->dzdx64>>(SST_Z64_SIZE-SST_Z_16BPP_SIZE)));
	SET(sst->dzdy,(FxU32)(t->dzdy64>>(SST_Z64_SIZE-SST_Z_16BPP_SIZE)));
    }
    if (t->area != 0)
	SET(sst->triangleCMD,t->area);
}

// use alternate register mappings
void drawAltTriangle(SstRegs *sst, Triangle *t, int RGB, int A, int Z)
{
  //setPixelsPerClock toggles between 1 and 2 pixels per clock rendering
  //if appropriate (i.e. --pixelsPerClock <= 0)
  setPixelsPerClock(sst);

    sst = SST_WRAP(sst,0x80);		// set high bit in WRAP field

    SET(sst->vA.x,t->vA.x);
    SET(sst->vA.y,t->vA.y);
    SET(sst->vB.x,t->vB.x);
    SET(sst->vB.y,t->vB.y);
    SET(sst->vC.x,t->vC.x);
    SET(sst->vC.y,t->vC.y);
    if (RGB) {
	SET(sst->r_ALT,t->vA.r);
	SET(sst->drdx_ALT,t->drdx);
	SET(sst->drdy_ALT,t->drdy);
	SET(sst->g_ALT,t->vA.g);
	SET(sst->dgdx_ALT,t->dgdx);
	SET(sst->dgdy_ALT,t->dgdy);
	SET(sst->b_ALT,t->vA.b);
	SET(sst->dbdx_ALT,t->dbdx);
	SET(sst->dbdy_ALT,t->dbdy);
    }
    if (A) {
	SET(sst->a_ALT,t->vA.a);
	SET(sst->dadx_ALT,t->dadx);
	SET(sst->dady_ALT,t->dady);
    }
    if (Z) {
	SET(sst->z_ALT,(FxU32)(t->vA.z64>>(SST_Z64_SIZE-SST_Z_16BPP_SIZE)));
	SET(sst->dzdx_ALT,(FxU32)(t->dzdx64>>(SST_Z64_SIZE-SST_Z_16BPP_SIZE)));
	SET(sst->dzdy_ALT,(FxU32)(t->dzdy64>>(SST_Z64_SIZE-SST_Z_16BPP_SIZE)));
    }
    if (t->area != 0)
	SET(sst->triangleCMD,t->area);
}

// only test floating verticies if nothing else is float
void drawFloatTriangle(SstRegs *sst, Triangle *t, int RGB, int A, int Z, int W)
{
  //setPixelsPerClock toggles between 1 and 2 pixels per clock rendering
  //if appropriate (i.e. --pixelsPerClock <= 0)
  setPixelsPerClock(sst);

    if (RGB || A || Z || W) {
	SET(sst->vA.x,t->vA.x);
	SET(sst->vA.y,t->vA.y);
	SET(sst->vB.x,t->vB.x);
	SET(sst->vB.y,t->vB.y);
	SET(sst->vC.x,t->vC.x);
	SET(sst->vC.y,t->vC.y);
    }
    else {
	SETF(sst->FvA.x,t->vA.fx);
	SETF(sst->FvA.y,t->vA.fy);
	SETF(sst->FvB.x,t->vB.fx);
	SETF(sst->FvB.y,t->vB.fy);
	SETF(sst->FvC.x,t->vC.fx);
	SETF(sst->FvC.y,t->vC.fy);
    }
    if (RGB) {
	SETF(sst->Fr,t->vA.fr);
	SETF(sst->Fg,t->vA.fg);
	SETF(sst->Fb,t->vA.fb);
	SETF(sst->Fdrdx,t->fdrdx);
	SETF(sst->Fdgdx,t->fdgdx);
	SETF(sst->Fdbdx,t->fdbdx);
	SETF(sst->Fdrdy,t->fdrdy);
	SETF(sst->Fdgdy,t->fdgdy);
	SETF(sst->Fdbdy,t->fdbdy);
    }
    if (A) {
	SETF(sst->Fa,t->vA.fa);
	SETF(sst->Fdadx,t->fdadx);
	SETF(sst->Fdady,t->fdady);
    }
    if (Z) {
	SETF(sst->Fz,t->vA.fz);
	SETF(sst->Fdzdx,t->fdzdx);
	SETF(sst->Fdzdy,t->fdzdy);
    }
    if (W) {
	SETF(sst->Fw,t->vA.fw);
	SETF(sst->Fdwdx,t->fdwdx);
	SETF(sst->Fdwdy,t->fdwdy);
    }
    if (t->area != 0)
	SETF(sst->FtriangleCMD,(float)t->area);
}

// always uses floating verticies
void drawAltFloatTriangle(SstRegs *sst, Triangle *t, int RGB, int A, int Z, int W)
{
  //setPixelsPerClock toggles between 1 and 2 pixels per clock rendering
  //if appropriate (i.e. --pixelsPerClock <= 0)
  setPixelsPerClock(sst);

    sst = SST_WRAP(sst,0x80);		// set high bit in WRAP field

    SETF(sst->FvA.x,t->vA.fx);
    SETF(sst->FvA.y,t->vA.fy);
    SETF(sst->FvB.x,t->vB.fx);
    SETF(sst->FvB.y,t->vB.fy);
    SETF(sst->FvC.x,t->vC.fx);
    SETF(sst->FvC.y,t->vC.fy);
    if (RGB) {				// use a wacky order just for fun
	SETF(sst->Fr_ALT,t->vA.fr);
	SETF(sst->Fdrdx_ALT,t->fdrdx);
	SETF(sst->Fdrdy_ALT,t->fdrdy);
	SETF(sst->Fb_ALT,t->vA.fb);
	SETF(sst->Fdgdx_ALT,t->fdgdx);
	SETF(sst->Fdbdx_ALT,t->fdbdx);
	SETF(sst->Fdgdy_ALT,t->fdgdy);
	SETF(sst->Fdbdy_ALT,t->fdbdy);
	SETF(sst->Fg_ALT,t->vA.fg);
    }
    if (A) {
	SETF(sst->Fa_ALT,t->vA.fa);
	SETF(sst->Fdadx_ALT,t->fdadx);
	SETF(sst->Fdady_ALT,t->fdady);
    }
    if (Z) {
	SETF(sst->Fz_ALT,t->vA.fz);
	SETF(sst->Fdzdx_ALT,t->fdzdx);
	SETF(sst->Fdzdy_ALT,t->fdzdy);
    }
    if (W) {
	SETF(sst->Fw_ALT,t->vA.fw);
	SETF(sst->Fdwdx_ALT,t->fdwdx);
	SETF(sst->Fdwdy_ALT,t->fdwdy);
    }
    if (t->area != 0)
	SETF(sst->FtriangleCMD,(float)t->area);
}

//---------------------------------------------------------------------
// sort the verticies from small Y to large Y
// returns whether triangle direction was reversed
int sortTriangle(Triangle *t)
{
    Vertex tv;

    if (t->vA.y < t->vB.y) {
	if (t->vB.y > t->vC.y) {
	    if (t->vA.y < t->vC.y) {
		tv = t->vB; t->vB = t->vC; t->vC = tv;
		t->area = -t->area;
		return 1;	// reversed order of triangle
	    }
	    else {
		tv = t->vA; t->vA = t->vC; t->vC = t->vB; t->vB = tv;
	    }
	}
	// else its already sorted
    }
    else {
	if (t->vB.y < t->vC.y) {
	    if (t->vA.y < t->vC.y) {
		tv = t->vA; t->vA = t->vB; t->vB = tv;
		t->area = -t->area;
		return 1;	// reversed order of triangle
	    }
	    else {
		tv = t->vA; t->vA = t->vB; t->vB = t->vC; t->vC = tv;
	    }
	}
	else {
	    tv = t->vA; t->vA = t->vC; t->vC = tv;
	    t->area = -t->area;
	    return 1;	// reversed order of triangle
	}
    }
    return 0;
}

//---------------------------------------------------------------------
// return whether or not x,y is inside the triangle by returning the color
int insideTriangle(Triangle *t, int x, int y, unsigned long col)
{
    FxI64 inAB, inAC, inBC;

    if (t->area == 0) return 0;
    x = (x<<SST_XY_FRACBITS) + XY_HALF;
    y = (y<<SST_XY_FRACBITS) + XY_HALF;
    inAB = (x - t->vA.x) * (FxI64)(t->vB.y - t->vA.y) -
	   (y - t->vA.y) * (FxI64)(t->vB.x - t->vA.x);
    if (inAB == 0)			// special case: flat bottom
	if (t->vB.y == t->vA.y) inAB = -t->area;	// always in
    inAC = (x - t->vA.x) * (FxI64)(t->vC.y - t->vA.y) -
	   (y - t->vA.y) * (FxI64)(t->vC.x - t->vA.x);
    inBC = (x - t->vB.x) * (FxI64)(t->vC.y - t->vB.y) -
	   (y - t->vB.y) * (FxI64)(t->vC.x - t->vB.x);
    if (inBC == 0)			// special case: flat top
	if (t->vB.y == t->vC.y) inBC = t->area;		// always out
    if (t->area > 0)
	return (inAB < 0) && (inBC < 0) && (inAC >= 0) ? col : 0;
    else
	return (inAB >= 0) && (inBC >= 0) && (inAC < 0) ? col : 0;
}

//----------------------------------------------------------------------
// check the triangle by checking each pixel in its bounding box, where the
// bounding box is expanded by 'slop' pixels around the extent of the triangle
// for each pixel, check either for color=csrc or any/all of RGB,A,Z
void checkTriangle(Triangle *t, unsigned long csrc, int slop,
			int subPixAdjust, PFNTRICHECK ptcInside,
			int checkRGB, int checkA, int checkZ)
{
  FxI32 chipIndex, sampleIndex, maxSampleIndex;
  FxI32 actualSampleIndex;
  SstRegs *sstCSIM;
  CsimPrivate *cp;
  FxI32 sliY;

    int checkCSRC = !(checkRGB || checkA || checkZ);
    int x,y, xmin,xmax,yStop;
    Vertex vertexA, vertexB, vertexC;
    FxI32 xOffset, yOffset;

    if ((diago.halInfo->hsim & HSIM_TREX_STANDALONE) || !diago.checkEveryTriangle)
	return;
    gdbg_info(18,"checkTriangle(*,csrc=0x%x,%d,*,*,%d,%d,%d)\n",
			csrc,slop,checkRGB,checkA,checkZ);

    //Back up the original triangle vertices
    vertexA = t->vA;
    vertexB = t->vB;
    vertexC = t->vC;

    for(chipIndex=0; chipIndex<diago.chipCount; chipIndex++)
      {
	if(diago.aaEnabled)	  
	  maxSampleIndex = 2;
	else
	  maxSampleIndex = 1;

	//GDBG_INFO(0, "maxSampleIndex=%d\n", maxSampleIndex);

	for(sampleIndex=0; sampleIndex<maxSampleIndex; sampleIndex++)
	  {
	    if(chipIndex == 0)
	      sstCSIM = diago.sstCSIM;
	    else
	      sstCSIM = diago.sstChildrenCSIM[chipIndex-1];
	    cp = CSIM_PRIVATE(sstCSIM);	    

	    //Determine the jitter value
	    if(sampleIndex == 0)
	      {
		xOffset = (shadowRegisters3D[chipIndex][0].aaCtrl & SST_AA_CONTROL_PRIMARY_X_OFFSET) 
		  >> SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT;
		yOffset = (shadowRegisters3D[chipIndex][0].aaCtrl & SST_AA_CONTROL_PRIMARY_Y_OFFSET)
		  >> SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT;		
	      }
	    else
	      {
		xOffset = (shadowRegisters3D[chipIndex][0].aaCtrl & SST_AA_CONTROL_SECONDARY_X_OFFSET) 
		  >> SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT;
		yOffset = (shadowRegisters3D[chipIndex][0].aaCtrl & SST_AA_CONTROL_SECONDARY_Y_OFFSET)
		  >> SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT;		
	      }
	    
	    if(diago.aaEnabled)
	      actualSampleIndex = (sampleIndex + 2 * (chipIndex & 1)) % diago.aaSampleCount;
	    else
	      actualSampleIndex = 0;

	    t->currentSampleIndex = actualSampleIndex;

	    xOffset = SIGN_EXTEND(xOffset, 7) + cp->environment.triangleOffsetX;
	    yOffset = SIGN_EXTEND(yOffset, 7) + cp->environment.triangleOffsetY;

	    GDBG_INFO(10, "CheckTriangle %d,%d: xOffset=0x%x yOffset=0x%x\n", chipIndex,
		      sampleIndex, xOffset, yOffset);
	    GDBG_INFO(10, "actualSampleIndex = %d   offset=(%d,%d)\n", actualSampleIndex,
		      xOffset, yOffset);
	
	    //Add in the jitter
	    t->vA.x = vertexA.x + xOffset; 
	    t->vA.y = vertexA.y + yOffset;
	    t->vB.x = vertexB.x + xOffset;
	    t->vB.y = vertexB.y + yOffset;
	    t->vC.x = vertexC.x + xOffset;
	    t->vC.y = vertexC.y + yOffset;

	    if(t->next)
	      {
		t->next->vA.x = t->vA.x;
		t->next->vA.y = t->vA.y;
		t->next->vB.x = t->vB.x;
		t->next->vB.y = t->vB.y;
		t->next->vC.x = t->vC.x;
		t->next->vC.y = t->vC.y;
	      }
	    
	    GDBG_INFO(10, "checkTriangle: 0x%x,0x%x  0x%x,0x%x  0x%x,0x%x\n",
		      t->vA.x, t->vA.y, t->vB.x, t->vB.y, t->vC.x, t->vC.y);
	    
	    xmin = t->vA.x;				// compute the triangle extents
	    if (t->vB.x < xmin) xmin = t->vB.x;
	    if (t->vC.x < xmin) xmin = t->vC.x;
	    xmin >>= SST_XY_FRACBITS;
	    xmin -= slop;
	    xmax = t->vA.x;
	    if (t->vB.x > xmax) xmax = t->vB.x;
	    if (t->vC.x > xmax) xmax = t->vC.x;
	    xmax >>= SST_XY_FRACBITS;
	    xmax += slop;
	    yStop = slop + (t->vC.y>>SST_XY_FRACBITS);
	    if (xmin < 0) xmin = 0;			// clip to screen
	    if (xmax > diago.xmaxscreen) xmax = diago.xmaxscreen;
	    
	    
	    for (y = (t->vA.y>>SST_XY_FRACBITS)-slop; y <= yStop; y++) {
	      if(!csimChipOwnsPixel(sstCSIM, y, &sliY))
		{
		  //GDBG_INFO(0, "chipIndex=%d scan y=%d skipped\n", chipIndex, y);
		  continue;
		}
	      
	      if (ONSCREEN(0,y))			// only check onscreen
		for (x = xmin; x<=xmax; x++) {
		  if (ONSCREEN(x,y)) {
		    FxU32 cinside = ptcInside(t,x,y,csrc);	// cinside is also the color
		    gdbg_info(19,"   check = x,y:%d,%d %s\n",x,y,cinside?"in":"out");
		    if (cinside) {
		      int dx, dy;
		      int a;
		      
		      if (subPixAdjust) {		// .4 distance to pixel center
			dx = (x<<SST_XY_FRACBITS) + XY_HALF - t->vA.x;
			dy = (y<<SST_XY_FRACBITS) + XY_HALF - t->vA.y;
		      }
		      else {			// integer distance in .4 format
			dx = (x - (t->vA.x>>SST_XY_FRACBITS))<<SST_XY_FRACBITS;
			dy = (y - (t->vA.y>>SST_XY_FRACBITS))<<SST_XY_FRACBITS;
		      }
		      if (checkCSRC) {
			diagTestPixel(actualSampleIndex, diago.curdrawbuffer, x, y, cinside);
		      }
		      if (checkA)
			{
			  if (checkA > 900) {		// TSU hack
			    a = cinside >> 24;		// get constant alpha
			  }
			  else {
			    a = t->vA.a + ((dx*t->dadx)>>SST_XY_FRACBITS) +
			      ((dy*t->dady)>>SST_XY_FRACBITS);
			    a >>= SST_RGBA_FRACBITS;
			    if (a == 0x100) a = 0xFF;	// clamp
			    if (a == 0xFFFFFFFF) a = 0;
			  }
			}
		      else a = 0;
		      cinside &= 0x00FFFFFF;		// mask out alpha
		      if (checkRGB) {
			int r,g,b;
			r = t->vA.r + ((dx*t->drdx)>>SST_XY_FRACBITS) +
			  ((dy*t->drdy)>>SST_XY_FRACBITS);
			g = t->vA.g + ((dx*t->dgdx)>>SST_XY_FRACBITS) +
			  ((dy*t->dgdy)>>SST_XY_FRACBITS);
			b = t->vA.b + ((dx*t->dbdx)>>SST_XY_FRACBITS) +
			  ((dy*t->dbdy)>>SST_XY_FRACBITS);
			r >>= SST_RGBA_FRACBITS;	// 8.0 format
			g >>= SST_RGBA_FRACBITS;
			b >>= SST_RGBA_FRACBITS;
			if (r == 0x100) r = 0xFF;	// check for 1 unit overflow
			if (g == 0x100) g = 0xFF;
			if (b == 0x100) b = 0xFF;
			if (r == 0xFFFFFFFF) r = 0;	// check for 1 unit underflow
			if (g == 0xFFFFFFFF) g = 0;
			if (b == 0xFFFFFFFF) b = 0;
			cinside  = (r & 0xFF) <<16;
			cinside |= (g & 0xFF) <<8;
			cinside |= (b & 0xFF) <<0;
		      }
		      if (diago.rgb != 16)
			cinside |= a << 24;		// merge alpha back in
		      cinside = sst_argb_form_result(cinside,0,x,y);
		      if (checkRGB || (checkA && (diago.rgb!=16)))
			diagTestPixel(actualSampleIndex, diago.curdrawbuffer, x, y, cinside);
		      if (checkA && (diago.rgb==16)) {
			diagTestPixel(actualSampleIndex, CSIM_BUF_3D_AUX1, x, y, a & 0xFF);
		      }
		      if (checkZ > 0) {			// Z buffering
			FxU32 z;
			FxI64 z64;
			FxU32 zmax;
			if (diago.rgb<32) {
			  zmax = 0xFFFF;
			  z64 = t->vA.z64;		// limit to 4.16.12
			  z64 += ((dx*t->dzdx64)>>SST_XY_FRACBITS) & ~FX_MASK64(SST_Z64_SIZE-SST_Z_32BPP_SIZE);
			  z64 += ((dy*t->dzdy64)>>SST_XY_FRACBITS) & ~FX_MASK64(SST_Z64_SIZE-SST_Z_32BPP_SIZE);
			}
			else {
			  zmax = 0xFFFFFF;
			  z64 = t->vA.z64 + ((dx*t->dzdx64)>>SST_XY_FRACBITS) +
			    ((dy*t->dzdy64)>>SST_XY_FRACBITS);
			}
			
			z = (int)(z64 >> SST_Z64_FRACBITS);
			if (z == zmax+1) z = zmax;	// clamp
			if (z == 0xFFFFFFFF) z = 0;
			if (checkZ > 900)		// TSU diag
			  z = (z>>8)&0xFF;		// SST_ALOCAL_Z enabled
			diagTestPixel(actualSampleIndex, CSIM_BUF_3D_AUX1, x, y, z & zmax);
		      }
		      if (checkZ < 0) {			// W buffering
			FxI64 w;
			w = t->vA.w + ((dx*t->dwdx)>>SST_XY_FRACBITS) +
			  ((dy*t->dwdy)>>SST_XY_FRACBITS);
			if (checkZ < -900) {		// SST_ALOCAL_W enabled
			  diagTestPixel(actualSampleIndex, CSIM_BUF_3D_AUX1, x, y, FX_HI64(w) & 0x00FF);
			}
			else 
			  diagTestPixel(actualSampleIndex, CSIM_BUF_3D_AUX1, x, y, wFloat64(w));
		      }
		      gdbg_info(19,"------------------\n");
		    }
		    else {		// outside the triangle
		      if (checkRGB || checkCSRC) diagTestPixel(actualSampleIndex, diago.curdrawbuffer, x, y, 0);
		      if (checkA || checkZ) diagTestPixel(actualSampleIndex, CSIM_BUF_3D_AUX1, x, y, 0);
		    }
		  }
		}
	    }
	  }
      }

    //Restore the original triangle vertices
    t->vA = vertexA;
    t->vB = vertexB;
    t->vC = vertexC;
}

//----------------------------------------------------------------------
// erase the triangle by forcing each pixel in its bounding box to 0
void eraseTriangle(SstRegs *sst, Triangle *t, int RGB, int A, int Z)
{
    int xmin,ymin, w,h;
    FxI32 sampleIndex, maxSampleIndex;
    Vertex vA, vB, vC;
    FxI32 xOffset, yOffset;

    if ((diago.halInfo->hsim & HSIM_TREX_STANDALONE) || !diago.checkEveryTriangle)
	return;

    //GDBG_INFO(0, "Running eraseTriangle\n");

    if(diago.aaEnabled)
      maxSampleIndex = diago.aaSampleCount;
    else
      maxSampleIndex = 1;

    for(sampleIndex=0; sampleIndex<maxSampleIndex; sampleIndex++)
      {
	//Determine the jitter value
	//This assumes that all AA/SLI pairs have the same jitter masks
	switch(sampleIndex)
	  {
	  case 0:
	    xOffset = (shadowRegisters3D[0][0].aaCtrl & SST_AA_CONTROL_PRIMARY_X_OFFSET) 
	      >> SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT;
	    yOffset = (shadowRegisters3D[0][0].aaCtrl & SST_AA_CONTROL_PRIMARY_Y_OFFSET)
	      >> SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT;		
	    break;

	  case 1:
	    xOffset = (shadowRegisters3D[0][0].aaCtrl & SST_AA_CONTROL_SECONDARY_X_OFFSET) 
	      >> SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT;
	    yOffset = (shadowRegisters3D[0][0].aaCtrl & SST_AA_CONTROL_SECONDARY_Y_OFFSET)
	      >> SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT;		
	    break;

	  case 2:
	    xOffset = (shadowRegisters3D[1][0].aaCtrl & SST_AA_CONTROL_PRIMARY_X_OFFSET) 
	      >> SST_AA_CONTROL_PRIMARY_X_OFFSET_SHIFT;
	    yOffset = (shadowRegisters3D[1][0].aaCtrl & SST_AA_CONTROL_PRIMARY_Y_OFFSET)
	      >> SST_AA_CONTROL_PRIMARY_Y_OFFSET_SHIFT;		
	    break;

	  case 3:
	    xOffset = (shadowRegisters3D[1][0].aaCtrl & SST_AA_CONTROL_SECONDARY_X_OFFSET) 
	      >> SST_AA_CONTROL_SECONDARY_X_OFFSET_SHIFT;
	    yOffset = (shadowRegisters3D[1][0].aaCtrl & SST_AA_CONTROL_SECONDARY_Y_OFFSET)
	      >> SST_AA_CONTROL_SECONDARY_Y_OFFSET_SHIFT;		
	    break;
	    
	  default:
	    assert(0);
	  }
	

	vA.x = t->vA.x + xOffset;
	vA.y = t->vA.y + yOffset;
	vB.x = t->vB.x + xOffset;
	vB.y = t->vB.y + yOffset;
	vC.x = t->vC.x + xOffset;
	vC.y = t->vC.y + yOffset;

	/*
	GDBG_INFO(0, "eraseTriangle sampleIndex=%d: 0x%x,0x%x  0x%x,0x%x  0x%x,0x%x\n",
		  sampleIndex, vA.x, vA.y, vB.x, vB.y, vC.x, vC.y);
	*/
	
	xmin = t->vA.x;				// compute the triangle extents
	if (t->vB.x < xmin) xmin = t->vB.x;
	if (t->vC.x < xmin) xmin = t->vC.x;
	xmin >>= SST_XY_FRACBITS;
	w = t->vA.x;
	if (t->vB.x > w) w = t->vB.x;
	if (t->vC.x > w) w = t->vC.x;
	w >>= SST_XY_FRACBITS;
	w = w - xmin + 1;
	
	ymin = t->vA.y>>SST_XY_FRACBITS;
	h = (t->vC.y>>SST_XY_FRACBITS) - ymin + 1;
	
	if (RGB || (A && (diago.rgb==15)))
	  DIAG_FORCE_RECT(diago.curdrawbuffer,xmin,ymin, w,h, 0);
	if ((A&(diago.rgb!=15)) || Z)
	  DIAG_FORCE_RECT(CSIM_BUF_3D_AUX1,xmin,ymin, w,h, 0);
      }
}
