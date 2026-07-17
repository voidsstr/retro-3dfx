/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:19:19 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"

#define PV(v)	PXY(v.x),PXY(v.y)
#define PXY(x)	printFix("%4d.%x",x,SST_XY_FRACBITS)
#define PST(w)	printFix64("%6x.%08x",w,SST_ST64_FRACBITS)
#define PW(w)	printFix64("%6x.%08x",w,SST_W64_FRACBITS)

//---------------------------------------------------------------------------
// these routines are similar to the ones in SSTTRI.C except that these deal
// with textured triangles
//---------------------------------------------------------------------------
void printStwTriangle(int level, Triangle *t)
{
    gdbg_info(level,"triangle = %s,%s to %s,%s to %s,%s\n",
		PV(t->vA),PV(t->vB),PV(t->vC));
    {
	gdbg_info(level,"       s =   %s  to  %s  to  %s\n",
		PST(t->vA.s),PST(t->vB.s),PST(t->vC.s));
	gdbg_info(level,"       t =   %s  to  %s  to  %s\n",
		PST(t->vA.t),PST(t->vB.t),PST(t->vC.t));
	gdbg_info(level,"       w =   %s  to  %s  to  %s\n",
		PW(t->vA.w),PW(t->vB.w),PW(t->vC.w));
    }
}

void printStwTriangleSlopes(int level, Triangle *t)
{
    gdbg_info(level,"    dsdx =   %s    dsdy = %s\n",PST(t->dsdx),PST(t->dsdy));
    gdbg_info(level,"    dtdx =   %s    dtdy = %s\n",PST(t->dtdx),PST(t->dtdy));
    gdbg_info(level,"    dwdx =   %s    dwdy = %s\n",PW(t->dwdx),PW(t->dwdy));
}

// generate random ST values for a triangle, don't be too harsh
static	// GMT: this needs work, to make it more like randomSt1418Triangle
void randomStTriangle(Triangle *t)
{
    t->vA.s = iRandom64(SST_MASK64(48));
    t->vA.t = iRandom64(SST_MASK64(48));
    t->vA.w = FX_BIT64(32);
    t->vB.s = iRandom64(SST_MASK64(48));
    t->vB.t = iRandom64(SST_MASK64(48));
    t->vB.w = FX_BIT64(32);
    t->vC.s = iRandom64(SST_MASK64(48));
    t->vC.t = iRandom64(SST_MASK64(48));
    t->vC.w = FX_BIT64(32);
}

// NOTE: the macro name is a misnomer as it might not be 14.18
#define SH1418 (64-SST_ST64_FRACBITS-SST_ST_INTBITS)
#define SIGN_EXTEND1418(x) ((((FxI64)x)<<SH1418)>>SH1418)

//---------------------------------------------------------------------
// generate random 14.18 ST values for a triangle, don't be too harsh
// the 14.18 number is stored in an FxI64 with the binary point in the same
// location as the 16.32 numbers
void randomSt1418Triangle(Triangle *t)
{
    int ds,dt;			// log2 of s,t variation across triangle
    int st_one;

    st_one = 0x100<<SST_ST_FRACBITS;			// 256.0 for S,T
    t->vA.s = rRandom64(-st_one,2*st_one);
    t->vA.t = rRandom64(-st_one,2*st_one);
    t->vA.s <<= SST_ST64_FRACBITS-SST_ST_FRACBITS;	// 16.32 format
    t->vA.t <<= SST_ST64_FRACBITS-SST_ST_FRACBITS;
    t->vA.w = FX_BIT64(SST_W64_FRACBITS);		// 1.0
    st_one = 1<<SST_ST_FRACBITS;			// 1 texel for S,T
    ds = (0x200<<iRandom(20))+st_one;			// random 14.18 format
    dt = (0x200<<iRandom(20))+st_one;
    t->vB.s = t->vA.s + (rRandom64(-ds,ds)<<(SST_ST64_FRACBITS-SST_ST_FRACBITS));
    t->vB.t = t->vA.t + (rRandom64(-dt,dt)<<(SST_ST64_FRACBITS-SST_ST_FRACBITS));
    t->vB.w = FX_BIT64(SST_W64_FRACBITS);		// 1.0
    t->vC.s = t->vA.s + (rRandom64(-ds,ds)<<(SST_ST64_FRACBITS-SST_ST_FRACBITS));
    t->vC.t = t->vA.t + (rRandom64(-dt,dt)<<(SST_ST64_FRACBITS-SST_ST_FRACBITS));
    t->vC.w = FX_BIT64(SST_W64_FRACBITS);		// 1.0
    t->vA.s = SIGN_EXTEND1418(t->vA.s);
    t->vA.t = SIGN_EXTEND1418(t->vA.t);
    t->vB.s = SIGN_EXTEND1418(t->vB.s);
    t->vB.t = SIGN_EXTEND1418(t->vB.t);
    t->vC.s = SIGN_EXTEND1418(t->vC.s);
    t->vC.t = SIGN_EXTEND1418(t->vC.t);
}

//---------------------------------------------------------------------
// generate random 2.30 format W coords stored in 16.32 format
// with an even distribution of exponents
// NOTE: we assume that S,T coords are already setup and we will then
//	divide them by W
void randomW230Triangle(Triangle *t)
{
    // generate largest possible positive integer value then shift
    // right to get exponent
    t->vA.w = iRandom(0x7FFFFFFF)>>iRandom(29);
    t->vB.w = iRandom(0x7FFFFFFF)>>iRandom(29);
    t->vC.w = iRandom(0x7FFFFFFF)>>iRandom(29);
    // make the values positive most of the time (negative 1/16 of time)
    if (iRandom(4)==0) t->vA.w =- t->vA.w;
    if (iRandom(4)==0) t->vB.w =- t->vB.w;
    if (iRandom(4)==0) t->vC.w =- t->vC.w;

    // convert S,T back into 14.18 format 
    t->vA.s >>= (SST_ST64_FRACBITS-SST_ST_FRACBITS);
    t->vA.t >>= (SST_ST64_FRACBITS-SST_ST_FRACBITS);
    t->vB.s >>= (SST_ST64_FRACBITS-SST_ST_FRACBITS);
    t->vB.t >>= (SST_ST64_FRACBITS-SST_ST_FRACBITS);
    t->vC.s >>= (SST_ST64_FRACBITS-SST_ST_FRACBITS);
    t->vC.t >>= (SST_ST64_FRACBITS-SST_ST_FRACBITS);
    // multiply by 2.30 1/w and return to 14.18 format
    t->vA.s = (t->vA.s * t->vA.w) >> SST_W_FRACBITS;
    t->vA.t = (t->vA.t * t->vA.w) >> SST_W_FRACBITS;
    t->vB.s = (t->vB.s * t->vB.w) >> SST_W_FRACBITS;
    t->vB.t = (t->vB.t * t->vB.w) >> SST_W_FRACBITS;
    t->vC.s = (t->vC.s * t->vC.w) >> SST_W_FRACBITS;
    t->vC.t = (t->vC.t * t->vC.w) >> SST_W_FRACBITS;
    // convert S,T back to 16.32 format
    t->vA.s <<= SST_ST64_FRACBITS-SST_ST_FRACBITS;	// 16.32 format
    t->vA.t <<= SST_ST64_FRACBITS-SST_ST_FRACBITS;
    t->vB.s <<= SST_ST64_FRACBITS-SST_ST_FRACBITS;	// 16.32 format
    t->vB.t <<= SST_ST64_FRACBITS-SST_ST_FRACBITS;
    t->vC.s <<= SST_ST64_FRACBITS-SST_ST_FRACBITS;	// 16.32 format
    t->vC.t <<= SST_ST64_FRACBITS-SST_ST_FRACBITS;
    // convert W to 16.32
    t->vA.w <<= SST_W64_FRACBITS-SST_W_FRACBITS;	// 16.32 format
    t->vB.w <<= SST_W64_FRACBITS-SST_W_FRACBITS;
    t->vC.w <<= SST_W64_FRACBITS-SST_W_FRACBITS;
}

#define SET1418(a,b) SET(a,(unsigned long)(b>>(SST_ST64_FRACBITS-SST_ST_FRACBITS)))
#define SET230(a,b) SET(a,(unsigned long)(b>>(SST_W64_FRACBITS-SST_W_FRACBITS)))

//---------------------------------------------------------------------
// draw a triangle, assumes vertices already sorted in Y
void drawStwTriangle(SstRegs *sst, Triangle *t)
{
    SET(sst->vA.x,t->vA.x);
    SET(sst->vA.y,t->vA.y);
    SET(sst->vB.x,t->vB.x);
    SET(sst->vB.y,t->vB.y);
    SET(sst->vC.x,t->vC.x);
    SET(sst->vC.y,t->vC.y);
    {
	SET1418(sst->s,t->vA.s);
	SET1418(sst->t,t->vA.t);
	SET230(sst->w,t->vA.w);
	SET1418(sst->dsdx,t->dsdx);
	SET1418(sst->dtdx,t->dtdx);
	SET230(sst->dwdx,t->dwdx);
	SET1418(sst->dsdy,t->dsdy);
	SET1418(sst->dtdy,t->dtdy);
	SET230(sst->dwdy,t->dwdy);
    }
    if (t->area != 0)
	SET(sst->triangleCMD,t->area);
}

//---------------------------------------------------------------------
// draw a triangle, assumes vertices already sorted in Y
// this version takes 2 sets of texture coordinates
void drawStwTriangle2(SstRegs *sst, Triangle *t, Triangle *t1)
{
  //We don't want to change the number of pixels per clock here.
  //We can never run in 2ppc with multi-texturing on

    SET(sst->vA.x,t->vA.x);
    SET(sst->vA.y,t->vA.y);
    SET(sst->vB.x,t->vB.x);
    SET(sst->vB.y,t->vB.y);
    SET(sst->vC.x,t->vC.x);
    SET(sst->vC.y,t->vC.y);
    {
	SET1418(SST_TREX(sst,t->tex->trex)->s,t->vA.s);
	SET1418(SST_TREX(sst,t->tex->trex)->t,t->vA.t);
	SET230(SST_TREX(sst,t->tex->trex)->w,t->vA.w);
	SET1418(SST_TREX(sst,t->tex->trex)->dsdx,t->dsdx);
	SET1418(SST_TREX(sst,t->tex->trex)->dtdx,t->dtdx);
	SET230(SST_TREX(sst,t->tex->trex)->dwdx,t->dwdx);
	SET1418(SST_TREX(sst,t->tex->trex)->dsdy,t->dsdy);
	SET1418(SST_TREX(sst,t->tex->trex)->dtdy,t->dtdy);
	SET230(SST_TREX(sst,t->tex->trex)->dwdy,t->dwdy);
    }
    {
	SET1418(SST_TREX(sst,t1->tex->trex)->s,t1->vA.s);
	SET1418(SST_TREX(sst,t1->tex->trex)->t,t1->vA.t);
	SET230(SST_TREX(sst,t1->tex->trex)->w,t1->vA.w);
	SET1418(SST_TREX(sst,t1->tex->trex)->dsdx,t1->dsdx);
	SET1418(SST_TREX(sst,t1->tex->trex)->dtdx,t1->dtdx);
	SET230(SST_TREX(sst,t1->tex->trex)->dwdx,t1->dwdx);
	SET1418(SST_TREX(sst,t1->tex->trex)->dsdy,t1->dsdy);
	SET1418(SST_TREX(sst,t1->tex->trex)->dtdy,t1->dtdy);
	SET230(SST_TREX(sst,t1->tex->trex)->dwdy,t1->dwdy);
    }
    if (t->area != 0)
	SET(sst->triangleCMD,t->area);
}

//---------------------------------------------------------------------
// compute the STW slopes for a triangle, assume area already computed
// return 1 upon success, 0 on error (out-of-range slopes)
int setupStwTriangle(Triangle *t)
{
    double ooarea;
    double dxAB, dyAB, dxBC, dyBC;

    ooarea = (double)(XY_ONE)/t->area;
    dxAB = (double)(t->vA.x - t->vB.x);
    dyAB = (double)(t->vA.y - t->vB.y);
    dxBC = (double)(t->vB.x - t->vC.x);
    dyBC = (double)(t->vB.y - t->vC.y);

    {
	t->dsdx = (FxI64)(((t->vA.s-t->vB.s) * dyBC - (t->vB.s-t->vC.s) * dyAB) * ooarea);
	t->dsdy = (FxI64)(((t->vB.s-t->vC.s) * dxAB - (t->vA.s-t->vB.s) * dxBC) * ooarea);
	t->dtdx = (FxI64)(((t->vA.t-t->vB.t) * dyBC - (t->vB.t-t->vC.t) * dyAB) * ooarea);
	t->dtdy = (FxI64)(((t->vB.t-t->vC.t) * dxAB - (t->vA.t-t->vB.t) * dxBC) * ooarea);
	t->dwdx = (FxI64)(((t->vA.w-t->vB.w) * dyBC - (t->vB.w-t->vC.w) * dyAB) * ooarea);
	t->dwdy = (FxI64)(((t->vB.w-t->vC.w) * dxAB - (t->vA.w-t->vB.w) * dxBC) * ooarea);
    }

    // check for positive overflow
    if (t->dsdx >= FX_BIT64(45)) {gdbg_info(1,"dsdx overflow\n"); return 0;}
    if (t->dsdy >= FX_BIT64(45)) {gdbg_info(1,"dsdy overflow\n"); return 0;}
    if (t->dtdx >= FX_BIT64(45)) {gdbg_info(1,"dtdx overflow\n"); return 0;}
    if (t->dtdy >= FX_BIT64(45)) {gdbg_info(1,"dtdy overflow\n"); return 0;}
    if (t->dwdx >= FX_BIT64(32)) {gdbg_info(1,"dwdx overflow\n"); return 0;}
    if (t->dwdy >= FX_BIT64(32)) {gdbg_info(1,"dwdy overflow\n"); return 0;}
    // check for negative overflow
    if (t->dsdx < (FxI64)~FX_MASK64(45)) {gdbg_info(1,"dsdx underflow\n"); return 0;}
    if (t->dsdy < (FxI64)~FX_MASK64(45)) {gdbg_info(1,"dsdy underflow\n"); return 0;}
    if (t->dtdx < (FxI64)~FX_MASK64(45)) {gdbg_info(1,"dtdx underflow\n"); return 0;}
    if (t->dtdy < (FxI64)~FX_MASK64(45)) {gdbg_info(1,"dtdy underflow\n"); return 0;}
    if (t->dwdx < (FxI64)~FX_MASK64(32)) {gdbg_info(1,"dwdx underflow\n"); return 0;}
    if (t->dwdy < (FxI64)~FX_MASK64(32)) {gdbg_info(1,"dwdy underflow\n"); return 0;}

    // need to mask these down to the precision that we give to the chip
    t->dsdx &= (SST_MASK64(64)) << (SST_ST64_FRACBITS-SST_ST_FRACBITS);
    t->dsdy &= (SST_MASK64(64)) << (SST_ST64_FRACBITS-SST_ST_FRACBITS);
    t->dtdx &= (SST_MASK64(64)) << (SST_ST64_FRACBITS-SST_ST_FRACBITS);
    t->dtdy &= (SST_MASK64(64)) << (SST_ST64_FRACBITS-SST_ST_FRACBITS);
    t->dwdx &= (SST_MASK64(64)) << (SST_W64_FRACBITS-SST_W_FRACBITS);
    t->dwdy &= (SST_MASK64(64)) << (SST_W64_FRACBITS-SST_W_FRACBITS);
    return 1;
}

// return the texture color if inside triangle
int insideStTriangle(Triangle *t, int x, int y, unsigned long col)
{
    if (insideTriangle(t,x,y,col)) {
	unsigned char rgba[4];

	texTo8888(rgba,t,x,y);		// texture lookup, return 8888 rgba
	if (diago.vsync) {		// hack for testing SST_CC_MATMU and SST_CC_RGBTMU
	    int rf,gf,bf,af;		// rgba factors
	    if (diago.vsync == (SST_CC_MATMU|SST_CCA_MATMU)) {
	        rf = gf = bf = af = rgba[3];
		rgba[3] = (0x23 * (af+1)) >> 8;
		goto domult;
	    }
	    else if (diago.vsync == SST_CC_MRGBTMU) {
		rf = rgba[0];
		gf = rgba[1];
		bf = rgba[2];
	    domult:
		rgba[0] = (0xFE * (rf+1)) >> 8;
		rgba[1] = (0xCD * (gf+1)) >> 8;
		rgba[2] = (0xBA * (bf+1)) >> 8;
	    }
	    else if (diago.vsync == (SST_CC_ADD_CLOCAL | SST_CC_ADD_ALOCAL)) {
		if (rgba[0] > 0x7F)
		    rgba[0] = 0xFF;
		else
		    rgba[0] += rgba[0];
		if (rgba[1] > 0x7F)
		    rgba[1] = 0xFF;
		else
		    rgba[1] += rgba[1];
		if (rgba[2] > 0x7F)
		    rgba[2] = 0xFF;
		else
		    rgba[2] += rgba[2];
	    }
	    else if (diago.vsync == (SST_CCA_ADD_CLOCAL | SST_CCA_ADD_ALOCAL)) {
		if (rgba[3] > 0x7F)
		    rgba[3] = 0xFF;
		else
		    rgba[3] += rgba[3];
	    }
	    else if (diago.vsync == -1) {
	    }
	    else if (diago.vsync == -2) {
		rgba[0] = rgba[3];
		rgba[1] = rgba[3];
		rgba[2] = rgba[3];
	    }
	    else
		GDBG_ERROR("insideStTriangle", "invalid diago.vsync value\n");
	}
	// GMT: note this doesn't handle renderMode 15bpp settings
	col = (rgba[0]<<16) | (rgba[1]<<8) | rgba[2];
	if (diago.rgb == 16) {
	    // a bit of a hack - we know that insideStTriangle only gets called
	    // when we are testing a triangle, so we optionally test alpha here	  
	    if ((texFormatHasAlpha(t->tex->tMode) ||
		(t->tex->tChromarange & SST_ENCHROMAKEY_TMU) ||
		(t->next && texFormatHasAlpha(t->next->tex->tMode))))
	      diagTestPixel(t->currentSampleIndex, CSIM_BUF_3D_AUX1, x, y, rgba[3]);
	    return 0x00F8FCF8 & col;
	}
	col |= rgba[3] << 24;		// merge in alpha
	if (diago.rgb == 15)
	    return 0x80F8F8F8 & col;
	return col;
    }
    return 0;
}
