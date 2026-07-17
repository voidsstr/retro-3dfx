#include "vxd.h"
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
** $Revision: 4$
** $Date: 10/11/00 8:09:16 PM$
*/

#include <assert.h>

#include <h3.h>
#include "h3sim.h"

#define uchar FxU8
#define ulong FxU32
#define ulonglong FxI64
#define BITN(s,n)       (ulong)(((s) >> n) & 1)
#define EXP(s)          (((s) >> 23) & 0xFF)
#define MAN(s)          ((s) & 0x7FFFFF)
#define SIGN(s)         (((s) >> 31) & 1)

static void
clamp (ulong *value)
{
  if (EXP(*value)) return;
  else *value = 0;
  return;
}

static void
floatsub (ulong a, ulong b, ulong *s)
{
	ulong		sign, es, co, ov;
	ulong		num, rman, fs, tc;
	ulong		aman, bman, rexp;
	ulong		rman_ab, frexp0, frexp1;
	ulong		itman;
	ulong		tman00, tman01, tman10, tman11;
	ulong		tman, tctman;
	ulong		final_man;
	long     	final_exp;
	long		zero, mb, lb;
	ulong		tmp2;
	ulonglong	tmp;
	ulong		b33, b32, b31, b30, b29, b28, b27, b26;
	

	aman = (a) ? (MAN(a) | 0x800000) << 2 : 0;
	bman = (b) ? (MAN(b) | 0x800000) << 2 : 0;
	num = (EXP(a) < EXP(b)) ? EXP(b) - EXP(a) : EXP(a) - EXP(b);
	rman = (EXP(a) < EXP(b)) ? aman : bman;
	itman = num > 31 ? 0 : rman >> num;
	tman00 = aman + itman;
	tman01 = aman - itman;
	tman10 = itman + bman;
	tman11 = itman - bman;

	sign = SIGN(a);
	es = !(SIGN(a) ^ SIGN(b));
	rexp = (EXP(a) < EXP(b)) ? EXP(b) : EXP(a);
	rman_ab = (EXP(a) < EXP(b));

	if (rman_ab) {
		if (es) tman = tman11;
		else tman = tman10;
	}
	else {
		if (es) tman = tman01;
		else tman = tman00;
	}

	co = BITN(tman,26);
	fs = (sign & !co) | (co & !sign & es) | (co & sign & !es);
	ov = !es & co;
	tc = es & co;

	frexp0 = rexp;
	frexp1 = rexp + 1;

	tctman = ~tman + 1;

	if (!tc) {
		if (!ov)  {
			final_exp = frexp0;
			final_man = tman & 0x3FFFFFF;
		}
		else {
			final_exp = frexp1;
			final_man = ((tman>>1) & 0x3FFFFFF);
		}
	}
	else {
		if (!ov) {
			final_exp = frexp0;
			final_man = tctman & 0x3FFFFFF;
		}
		else {
			final_exp = frexp1;
			final_man = (tctman>>1) & 0x3FFFFFF;
		}
	}

/* fs, final_exp, final_man */
	zero = 0;
	mb = 0;

	if ((final_man >> 18) & 0xFF) {
		tmp = final_man & 0x3FFFFFF;
		tmp <<= 8;
		mb = 0;
	}
	else if ((final_man >> 10) & 0xFF) {
		tmp = final_man & 0x3FFFF;
		tmp <<= 16;
		mb = 1;
	}
	else if ((final_man >> 2) & 0xFF) {
		tmp = final_man & 0x3FF;
		tmp <<= 24;
		mb = 2;
	}
	else if ((final_man) & 3) {
		tmp = final_man & 0x3;
		tmp = tmp << 32;
		mb = 3;
	}
	else zero = 1;

/* Horrible Hack comming this way */
	b33 = BITN(tmp,33);
	b32 = BITN(tmp,32);
	b31 = BITN(tmp,31);
	b30 = BITN(tmp,30);
	b29 = BITN(tmp,29);
	b28 = BITN(tmp,28);
	b27 = BITN(tmp,27);
	b26 = BITN(tmp,26);

	if (b33) {
		tmp = (tmp >> 10) & 0x7FFFFF;
		lb = 0;
	}
	else if (b32) {
		tmp = (tmp >> 9) & 0x7FFFFF;
		lb = 1;
	}
	else if (b31) {
		tmp = (tmp >> 8) & 0x7FFFFF;
		lb = 2;
	}
	else if (b30) {
		tmp = (tmp >> 7) & 0x7FFFFF;
		lb = 3;
	}
	else if (b29) {
		tmp = (tmp >> 6) & 0x7FFFFF;
		lb = 4;
	}
	else if (b28) {
		tmp = (tmp >> 5) & 0x7FFFFF;
		lb = 5;
	}
	else if (b27) {
		tmp = (tmp >> 4) & 0x7FFFFF;
		lb = 6;
	}
	else if (b26) {
		tmp = (tmp >> 3) & 0x7FFFFF;
		lb = 7;
	}

	tmp2 = (ulong)tmp;

	zero = zero || ((final_exp - mb*8 - lb) <= 0);
	if (zero) *s = 0;
	else *s = (fs << 31) | ((final_exp - mb*8 - lb) << 23) | tmp2;
}

static ulong round(ulong lgrs)
{
	switch (lgrs) {
		case 0: return 0;
		case 1: return 0;
		case 2: return 0;
		case 3: return 0;
		case 4: return 0;
		case 5: return 1;
		case 6: return 1;
		case 7: return 1;
		case 8: return 0;
		case 9: return 0;
		case 10: return 0;
		case 11: return 0;
		case 12: return 1;
		case 13: return 1;
		case 14: return 1;
		case 15: return 1;
	}

	assert(0);
	return(0xFFFFFFFF);
}

static void 
mult_normalize(ulong im, ulong iexp, ulong *om, ulong *oe,
		ulong inc_a, ulong inc_b)

{
	ulong	tmp;
	ulong	shift;

	if (BITN(im,25)) {
		tmp = ((im >> 2) & 0xFFFFFF) + inc_a;
		shift = (BITN(tmp,24)) ? 2 : 1;
	}
	else {
		tmp = ((im >> 1) & 0xFFFFFF) + inc_b;
		shift = (BITN(tmp,24)) ? 1 : 0;
	}

	*oe = iexp + shift;
	*om = tmp & 0xFFFFFF;
}

static  void floatmul(ulong a, ulong b, ulong *p)
{
	ulong	s;
	long 	expout;
	ulong	lgrsa, lgrsb;
	ulong	sign;
	ulong	inc_a, inc_b;
	ulong	im, om, oe;
	uchar	zero;
	ulonglong e, f, prod;

	expout = EXP(a) + EXP(b);
	zero = 0;
	if ((EXP(a) == 0) || (EXP(b) == 0)) zero = 1;
	if (zero) expout = 0x7F;
	expout -= 0x7F;
	if (expout < 0) expout = 0;

	e = (a) ? MAN(a) | 0x800000 : 0;
	f = (b) ? MAN(b) | 0x800000 : 0;
	prod = e * f;
// rounding
	s = ((prod & 0x1FFFFF) != 0);
	lgrsa = (ulong)((prod >> 21) & 0xF) | s;
	lgrsb = (ulong)(((prod >> 21) & 0x7) << 1) | s;

	inc_a = round(lgrsa); 
	inc_b = round(lgrsb);

	im = (ulong)(prod >> 22) & 0x3FFFFFF;

	mult_normalize(im, expout, &om, &oe, inc_a, inc_b);

	sign = (zero) ? 0 : SIGN(a) ^ SIGN(b);

	*p = sign << 31;
	*p |= (oe << 23);
	*p |= (om & 0x7FFFFF);
}

static void
recip (ulong sig, ulong man, ulong exp, ulong *osig,
	ulong *oman, ulong *oexp)
{
	ulong	osman;
	ulong	remainder;
	ulong	tmp, vexp, sman, c;
	ulong	l, g, r;
	ulong	divisor, divisor2x, divisor3x;
	ulong	divisor4x, divisor5x, divisor6x;
	ulong	divisor7x;
	ulong	n1, n2, n3, n4, n5, n6, n7;
	ulong	qbits;

	divisor = man;
	divisor2x = man << 1;
	divisor3x = divisor + divisor2x;
	divisor4x = man << 2;
	divisor5x = divisor4x + divisor;
	divisor6x = divisor4x + divisor2x;
	divisor7x = (man << 3) - divisor;

	*osig = sig;
	vexp = (exp != 0) ? 0xFF - exp - 2 : 0; 
	vexp &= 0xFF;
	sman = 0;

	remainder = 0x800000;

	for (c = 0; c < 10; c++) {
		n1 = remainder - divisor7x;
		n2 = remainder - divisor6x;
		n3 = remainder - divisor5x;
		n4 = remainder - divisor4x;
		n5 = remainder - divisor3x;
		n6 = remainder - divisor2x;
		n7 = remainder - divisor;

                if (!(n1 & (1<<31))) {
                        qbits = 7;
                        remainder = n1 << 3;
                }
                else if (!(n2 &(1<<31))) {
                        qbits = 6;
                        remainder = n2 << 3;
                }
                else if (!(n3 & (1<<31))) {
                        qbits = 5;
                        remainder = n3 << 3;
                }
                else if (!(n4 & (1<<31))) {
                        qbits = 4;
                        remainder = n4 << 3;
                }
                else if (!(n5 & (1<<31))) {
                        qbits = 3;
                        remainder = n5 << 3;
                }
                else if (!(n6 & (1<<31))) {
                        qbits = 2;
                        remainder = n6 << 3;
                }
                else if (!(n7 & (1<<31))) {
                        qbits = 1;
                        remainder = n7 << 3;
                }
                else {
                        qbits = 0;
                        remainder = remainder << 3;
                }
 
		sman = ((sman & 0x1FFFFFF) << 3) | qbits;

	}

	osman = (sman >> 2) + 1;
	l = (sman >> 3) & 1;
	g = (sman >> 2) & 1;
	r = ((sman & 3) != 0);

	tmp = (l << 2) | (g << 1) | r;
	switch (tmp) {
		case 0:
			*oman = (BITN(sman,27)) ? (sman >> 4) : (sman >> 3);
			*oexp = (BITN(sman,27)) ? vexp + 1 : vexp;
			break;
		case 1:
			*oman = (BITN(sman,27)) ? (sman >> 4) : (sman >> 3);
			*oexp = (BITN(sman,27)) ? vexp + 1 : vexp;
			break;
		case 2:
			*oman = (BITN(sman,27)) ? (sman >> 4) : (sman >> 3);
			*oexp = (BITN(sman,27)) ? vexp + 1 : vexp;
			break;
		case 3:
			*oman = (BITN(osman,25)) ? (osman >> 2) : (osman >> 1);
			*oexp = (BITN(osman,25)) ? vexp + 1 : vexp;
			break;
		case 4:
			*oman = (BITN(sman,27)) ? (sman >> 4) : (sman >> 3);
			*oexp = (BITN(sman,27)) ? vexp + 1 : vexp;
			break;
		case 5:
			*oman = (BITN(sman,27)) ? (sman >> 4) : (sman >> 3);
			*oexp = (BITN(sman,27)) ? vexp + 1 : vexp;
			break;
		case 6:
			*oman = (BITN(osman,25)) ? (osman >> 2) : (osman >> 1);
			*oexp = (BITN(osman,25)) ? vexp + 1 : vexp;
			break;
		case 7:
			*oman = (BITN(osman,25)) ? (osman >> 2) : (osman >> 1);
			*oexp = (BITN(osman,25)) ? vexp + 1 : vexp;
			break;
	}

	*oman &= 0x7FFFFF;
}

static void
floatrecip(ulong in, ulong *out)
{
	ulong	osig, oexp, oman;

	recip (SIGN(in), MAN(in) | 0x800000, EXP(in), &osig, &oman, &oexp);

	*out = osig << 31;
	*out |= (oexp << 23);
	*out |= oman;
}

//----------------------------------------------------------------------
// convert a 32-bit single precision floating point value to a
// 64-bit fixed point value with a specified number of fraction bits
//----------------------------------------------------------------------
FxI64 _convertFloat2Fix64(FxU32 data, int bits)
{
    int exp;
    FxI64 data64;

//GDBG_INFO(19,"_convertFloat2Fix64(%08x,%d)\n",data,bits);
    exp = (data>>23) & 0xFF;            // peel off exponent
    bits = 150 - exp - bits;     	// compute shift amount
    data |= 0x800000;			// add in the hidden bit
    if (bits > 0) {
	if (bits > 31) bits = 31;
	FX_COPY32(data64,(data&0xFFFFFF) >> bits);
    }
    else {
	bits = -bits;
	if (bits > 63) bits = 63;
	FX_COPY32(data64,data&0xFFFFFF);
	data64 = FX_SHL64(data64,bits);
    }
    // negate after shift, this rounds down instead of to zero
    if (data & 0x80000000) data64 = FX_NEG64(data64);
//GDBG_INFO(19,"_convertFloat2Fix64 ==> %08x_%08x\n",FX_LO64(data64>>32),FX_LO64(data64));
    return data64;
}

//----------------------------------------------------------------------
// setup up a triangle, returns 0 if culled, 1 if OK to draw
//----------------------------------------------------------------------
int sstTriangleSetup (SstRegs *sst)
{
    int a,b,c;		// indicies into vertex array
    FxU32 culltest, smode;
    FxU32 dxAB, dxBC, dyAB, dyBC, dyAC;
    FxU32 dpAB, dpBC, dpdx, dpdy;
    FxU32 area,ooarea, im1,im2;
    CsimPrivate *cp = CSIM_PRIVATE(sst);
    SVertex *v0, *v1, *v2;

    smode = sst->sSetupMode;
    culltest = (smode & SST_SETUP_CULL_NEGATIVE) != 0;
    v0 = cp->tsuData.vArray + 0;
    v1 = cp->tsuData.vArray + 1;
    v2 = cp->tsuData.vArray + 2;

    //----------------------------------------------------------------------
    // compute area first (like the HW) and cull
    // NOTE: vArray.xy are already snapped to .4 precision
    floatsub(v0->x, v1->x, &dxAB);
    floatsub(v0->y, v1->y, &dyAB);
    floatsub(v1->x, v2->x, &dxBC);
    floatsub(v1->y, v2->y, &dyBC);
    floatsub(v0->y, v2->y, &dyAC);

    floatmul(dxAB, dyBC, &im1);
    floatmul(dxBC, dyAB, &im2);

    floatsub(im1, im2, &area);

#ifndef NO_FLOAT
    /* NO_FLOAT
     * just printing info out, hope we don't need it! :)
     */
    GDBG_INFO(126,"TRIANGLE SETUP: smode:%x %s%c  area=%g 0x%x\n",smode,
		smode & SST_SETUP_EN_CULLING ? "CULL": "no-cull",
		culltest ? '-' : '+',
		*(float *)&area,area);
    if (GDBG_GET_DEBUGLEVEL(128)) {
	GDBG_PRINTF("\tA.x = %8.4f\t0x%x\n", *(float *)&v0->x, v0->x);
	GDBG_PRINTF("\tA.y = %8.4f\t0x%x\n", *(float *)&v0->y, v0->y);
	GDBG_PRINTF("\tB.x = %8.4f\t0x%x\n", *(float *)&v1->x, v1->x);
	GDBG_PRINTF("\tB.y = %8.4f\t0x%x\n", *(float *)&v1->y, v1->y);
	GDBG_PRINTF("\tC.x = %8.4f\t0x%x\n", *(float *)&v2->x, v2->x);
	GDBG_PRINTF("\tC.y = %8.4f\t0x%x\n", *(float *)&v2->y, v2->y);
    }
#endif /* #ifndef NO_FLOAT */

    if ((area & 0x7FFFFFFF) == 0) {	// check for either pos or neg zero
	GDBG_INFO(127,"---zero area triangle culled\n");
	return 0;
    }
    if (smode & SST_SETUP_EN_CULLING) {
	int enpp = smode & SST_SETUP_DIS_PINGPONG ? 0 : cp->tsuData.pingpong;
	if (!((area ^ (culltest<<31) ^ enpp) & 0x80000000)) {
	    GDBG_INFO(127,"---backfaced triangle culled\n");
	    return 0;
	}
    }

    floatrecip(area, &ooarea);		// invert the area

    //----------------------------------------------------------------------
    // sort the vertices
    // a,b,c are the indices of the vertices from low to high Y
    {
	FxU32 sort = (SIGN(dyAB) << 2) | (SIGN(dyBC) << 1) | SIGN(dyAC);
	switch (sort) {
		case 4:
			a = 2; b = 0; c = 1;
			GDBG_INFO(127,"\tcab\n");
			break;
		case 5:
			a = 0; b = 2; c = 1;
			area ^= 0x80000000;
			GDBG_INFO(127,"\tacb\n");
			break;
		case 6: // Undefined.
			break;
		case 7:
			a = 0; b = 1; c = 2;
			GDBG_INFO(127,"\tabc\n");
			break;
		case 0:
			a = 2; b = 1; c = 0;
			GDBG_INFO(127,"\tcba\n");
			area ^= 0x80000000;
			break;
		case 1: // Undefined
			break;
		case 2: 
			a = 1; b = 2; c = 0;
			GDBG_INFO(127,"\tbca\n");
			break;
		case 3:
			a = 1; b = 0; c = 2;
			area ^= 0x80000000;
			GDBG_INFO(127,"\tbac\n");
			break;
	}
    }

    sst->triangleCMD = area;		// NOTE: only need the sign bit

    //----------------------------------------------------------------------
    // compute gradients on UNSORTED vertices
    // we use 0,1,2 to index vArray for computing gradients
    // we use a,b,c for setting vertex data (XY, Parameters)

#define SET_VERTEX_P(dest,src,bits,size)	\
	dest = FX_LO64(_convertFloat2Fix64(*(FxU32 *)&src,bits)) & SST_MASK(size)
#define SET_VERTEX64_P(dest,src,bits,size)	\
	dest = _convertFloat2Fix64(*(FxU32 *)&src,bits) & FX_MASK64(size)

#define SST_Z64_FRACSHIFT ((sst->renderMode & SST_RM_32BPP) ? SST_Z64_FRACBITS_32BPP : SST_Z_16BPP_FRACBITS)

#define SET_VERTEX_XY(dest,src)	  SET_VERTEX_P(dest,src,SST_XY_FRACBITS,SST_XY_SIZE)
#define SET_VERTEX_RGBA(dest,src) SET_VERTEX_P(dest,src,SST_RGBA_FRACBITS,SST_RGBA_SIZE)
#define SET_VERTEX_Z(dest,src)	  SET_VERTEX64_P(dest,src,SST_Z64_FRACSHIFT,SST_Z64_SIZE)
#define SET_VERTEX_ST(dest,src)	  SET_VERTEX64_P(dest,src,SST_ST64_FRACBITS,SST_ST64_SIZE)
#define SET_VERTEX_W(dest,src)	  SET_VERTEX64_P(dest,src,SST_W64_FRACBITS,SST_W64_SIZE)

#define COMPUTE_GRADIENT(dx,dy,p)	\
	ulong   n1, n2, n3, n4;		\
	clamp(&v0->p); 			\
	clamp(&v1->p); 			\
	clamp(&v2->p); 			\
	floatsub(v0->p,v1->p,&dpAB);	\
	floatsub(v1->p,v2->p,&dpBC);	\
	floatmul(dpAB, dxBC, &n1);	\
	floatmul(dpAB, dyBC, &n2);	\
	floatmul(dpBC, dxAB, &n3);	\
	floatmul(dpBC, dyAB, &n4);	\
	floatsub(n2, n4, &dpdx);	\
	floatsub(n3, n1, &dpdy);	\
	floatmul(dpdx, ooarea, &dpdx);	\
	floatmul(dpdy, ooarea, &dpdy)

#define SETUP_P(dx,dy,p,bits,size) {	\
	COMPUTE_GRADIENT(dx,dy,p);	\
	dx = FX_LO64(_convertFloat2Fix64(dpdx,bits)) & SST_MASK(size);	\
	dy = FX_LO64(_convertFloat2Fix64(dpdy,bits)) & SST_MASK(size); }

#define SETUP64_P(dx,dy,p,bits,size) {	\
	COMPUTE_GRADIENT(dx,dy,p);	\
	dx = _convertFloat2Fix64(dpdx,bits) & FX_MASK64(size);	\
	dy = _convertFloat2Fix64(dpdy,bits) & FX_MASK64(size); }

#define SETUP_RGBA(dx,dy,p)	SETUP_P(dx,dy,p,SST_RGBA_FRACBITS,SST_RGBA_SIZE)
#define SETUP64_Z(dx,dy,p)	SETUP64_P(dx,dy,p,SST_Z64_FRACSHIFT,SST_Z64_SIZE)
#define SETUP64_ST(dx,dy,p)	SETUP64_P(dx,dy,p,SST_ST64_FRACBITS,SST_ST64_SIZE)
#define SETUP64_W(dx,dy,p)	SETUP64_P(dx,dy,p,SST_W64_FRACBITS,SST_W64_SIZE)


    if (1) {		// always send XY (in sorted order!)
	// now convert the vertex data and place results in old SST-1 registers
	// NOTE: we only need them in FBI even though they are really broadcast
	SET_VERTEX_XY(sst->vA.x,cp->tsuData.vArray[a].x);
	SET_VERTEX_XY(sst->vA.y,cp->tsuData.vArray[a].y);

	SET_VERTEX_XY(sst->vB.x,cp->tsuData.vArray[b].x);
	SET_VERTEX_XY(sst->vB.y,cp->tsuData.vArray[b].y);

	SET_VERTEX_XY(sst->vC.x,cp->tsuData.vArray[c].x);
	SET_VERTEX_XY(sst->vC.y,cp->tsuData.vArray[c].y);
    }

    // now for each parameter
    // first send the data for vertex A
    // then compute and send the gradient
    if (smode & SST_SETUP_RGB) {
	SET_VERTEX_RGBA(sst->r,cp->tsuData.vArray[a].r);
	SET_VERTEX_RGBA(sst->g,cp->tsuData.vArray[a].g);
	SET_VERTEX_RGBA(sst->b,cp->tsuData.vArray[a].b);

	SETUP_RGBA(sst->drdx,sst->drdy,r);
	SETUP_RGBA(sst->dgdx,sst->dgdy,g);
	SETUP_RGBA(sst->dbdx,sst->dbdy,b);
    }

    if (smode & SST_SETUP_A) {
	SET_VERTEX_RGBA(sst->a,cp->tsuData.vArray[a].a);
	SETUP_RGBA(sst->dadx,sst->dady,a);
    }

    if (smode & SST_SETUP_Z) {
	SET_VERTEX_Z(cp->fbiData.z64,cp->tsuData.vArray[a].z);
	SETUP64_Z(cp->fbiData.dzdx64,cp->fbiData.dzdy64,z);

	// Make sure that we're backwards compatible in 16bpp
	// GMT: we did the conversion to produce 12 bits of fraction
	// now shift left 16 bits
#define ZSHL (SST_Z64_FRACBITS_16BPP - SST_Z_16BPP_FRACBITS)
	if((sst->renderMode & SST_RM_3D_MODE) != SST_RM_32BPP) {
	    cp->fbiData.z64 = FX_SHL64(cp->fbiData.z64, ZSHL);
	    cp->fbiData.dzdx64 = FX_SHL64(cp->fbiData.dzdx64, ZSHL);
	    cp->fbiData.dzdy64 = FX_SHL64(cp->fbiData.dzdy64, ZSHL);
	}
    }


    if (smode & SST_SETUP_Wfbi) {
	SET_VERTEX_W(cp->fbiData.w64,cp->tsuData.vArray[a].w);
	SETUP64_W(cp->fbiData.dwdx64,cp->fbiData.dwdy64,w);
	// broadcast upwards to TMU0,1,2
	TMU_PRIVATE(cp->trex+0)->w64 = cp->fbiData.w64;
	TMU_PRIVATE(cp->trex+0)->dwdx64 = cp->fbiData.dwdx64;
	TMU_PRIVATE(cp->trex+0)->dwdy64 = cp->fbiData.dwdy64;
	if (cp->chipMask & 0x4) {
	    TMU_PRIVATE(cp->trex+1)->w64 = cp->fbiData.w64;
	    TMU_PRIVATE(cp->trex+1)->dwdx64 = cp->fbiData.dwdx64;
	    TMU_PRIVATE(cp->trex+1)->dwdy64 = cp->fbiData.dwdy64;
	}
	if (cp->chipMask & 0x8) {
	    TMU_PRIVATE(cp->trex+2)->w64 = cp->fbiData.w64;
	    TMU_PRIVATE(cp->trex+2)->dwdx64 = cp->fbiData.dwdx64;
	    TMU_PRIVATE(cp->trex+2)->dwdy64 = cp->fbiData.dwdy64;
	}
    }
    if (smode & SST_SETUP_ST0) {
	SET_VERTEX_ST(TMU_PRIVATE(cp->trex+0)->s64,cp->tsuData.vArray[a].s0);
	SET_VERTEX_ST(TMU_PRIVATE(cp->trex+0)->t64,cp->tsuData.vArray[a].t0);
	SETUP64_W(TMU_PRIVATE(cp->trex+0)->dsdx64,TMU_PRIVATE(cp->trex+0)->dsdy64,s0);
	SETUP64_W(TMU_PRIVATE(cp->trex+0)->dtdx64,TMU_PRIVATE(cp->trex+0)->dtdy64,t0);
	// broadcast upwards to TMU1,2
	if (cp->chipMask & 0x4) {
	    TMU_PRIVATE(cp->trex+1)->s64 = TMU_PRIVATE(cp->trex+0)->s64;
	    TMU_PRIVATE(cp->trex+1)->dsdx64 = TMU_PRIVATE(cp->trex+0)->dsdx64;
	    TMU_PRIVATE(cp->trex+1)->dsdy64 = TMU_PRIVATE(cp->trex+0)->dsdy64;
	    TMU_PRIVATE(cp->trex+1)->t64 = TMU_PRIVATE(cp->trex+0)->t64;
	    TMU_PRIVATE(cp->trex+1)->dtdx64 = TMU_PRIVATE(cp->trex+0)->dtdx64;
	    TMU_PRIVATE(cp->trex+1)->dtdy64 = TMU_PRIVATE(cp->trex+0)->dtdy64;
	}
	if (cp->chipMask & 0x8) {
	    TMU_PRIVATE(cp->trex+2)->s64 = TMU_PRIVATE(cp->trex+0)->s64;
	    TMU_PRIVATE(cp->trex+2)->dsdx64 = TMU_PRIVATE(cp->trex+0)->dsdx64;
	    TMU_PRIVATE(cp->trex+2)->dsdy64 = TMU_PRIVATE(cp->trex+0)->dsdy64;
	    TMU_PRIVATE(cp->trex+2)->t64 = TMU_PRIVATE(cp->trex+0)->t64;
	    TMU_PRIVATE(cp->trex+2)->dtdx64 = TMU_PRIVATE(cp->trex+0)->dtdx64;
	    TMU_PRIVATE(cp->trex+2)->dtdy64 = TMU_PRIVATE(cp->trex+0)->dtdy64;
	}
    }
    if (smode & SST_SETUP_W0) {
	SET_VERTEX_W(TMU_PRIVATE(cp->trex+0)->w64,cp->tsuData.vArray[a].w0);
	SETUP64_W(TMU_PRIVATE(cp->trex+0)->dwdx64,TMU_PRIVATE(cp->trex+0)->dwdy64,w0);
	// broadcast upwards to TMU1
	if (cp->chipMask & 0x4) {
	    TMU_PRIVATE(cp->trex+1)->w64 = TMU_PRIVATE(cp->trex+0)->w64;
	    TMU_PRIVATE(cp->trex+1)->dwdx64 = TMU_PRIVATE(cp->trex+0)->dwdx64;
	    TMU_PRIVATE(cp->trex+1)->dwdy64 = TMU_PRIVATE(cp->trex+0)->dwdy64;
	}
	if (cp->chipMask & 0x8) {
	    TMU_PRIVATE(cp->trex+2)->w64 = TMU_PRIVATE(cp->trex+0)->w64;
	    TMU_PRIVATE(cp->trex+2)->dwdx64 = TMU_PRIVATE(cp->trex+0)->dwdx64;
	    TMU_PRIVATE(cp->trex+2)->dwdy64 = TMU_PRIVATE(cp->trex+0)->dwdy64;
	}
    }
    if (smode & SST_SETUP_ST1) {
	if (cp->chipMask & 0x4) {
	    SET_VERTEX_ST(TMU_PRIVATE(cp->trex+1)->s64,cp->tsuData.vArray[a].s1);
	    SET_VERTEX_ST(TMU_PRIVATE(cp->trex+1)->t64,cp->tsuData.vArray[a].t1);
	    SETUP64_W(TMU_PRIVATE(cp->trex+1)->dsdx64,TMU_PRIVATE(cp->trex+1)->dsdy64,s1);
	    SETUP64_W(TMU_PRIVATE(cp->trex+1)->dtdx64,TMU_PRIVATE(cp->trex+1)->dtdy64,t1);
	}
	if (cp->chipMask & 0x8) {
	    TMU_PRIVATE(cp->trex+2)->s64 = TMU_PRIVATE(cp->trex+1)->s64;
	    TMU_PRIVATE(cp->trex+2)->dsdx64 = TMU_PRIVATE(cp->trex+1)->dsdx64;
	    TMU_PRIVATE(cp->trex+2)->dsdy64 = TMU_PRIVATE(cp->trex+1)->dsdy64;
	    TMU_PRIVATE(cp->trex+2)->t64 = TMU_PRIVATE(cp->trex+1)->t64;
	    TMU_PRIVATE(cp->trex+2)->dtdx64 = TMU_PRIVATE(cp->trex+1)->dtdx64;
	    TMU_PRIVATE(cp->trex+2)->dtdy64 = TMU_PRIVATE(cp->trex+1)->dtdy64;
	}
    }
    if (smode & SST_SETUP_W1) {
	if (cp->chipMask & 0x4) {
	    SET_VERTEX_W(TMU_PRIVATE(cp->trex+1)->w64,cp->tsuData.vArray[a].w1);
	    SETUP64_W(TMU_PRIVATE(cp->trex+1)->dwdx64,TMU_PRIVATE(cp->trex+1)->dwdy64,w1);
	}
	if (cp->chipMask & 0x8) {
	    TMU_PRIVATE(cp->trex+2)->w64 = TMU_PRIVATE(cp->trex+1)->w64;
	    TMU_PRIVATE(cp->trex+2)->dwdx64 = TMU_PRIVATE(cp->trex+1)->dwdx64;
	    TMU_PRIVATE(cp->trex+2)->dwdy64 = TMU_PRIVATE(cp->trex+1)->dwdy64;
	}
    }

#if MAX_NUM_TMUS > 3
    if (cp->chipMask & 0x10)
	GDBG_ERROR("sstTriangleSetup", "We need more code for the 4th TMU!\n");
#endif
    return 1;
}

//----------------------------------------------------------------------
// grab a copy of all the vertex data from the vertex registers
// NOTE: also snaps X,Y to .4 precision (1/16 pixel)
//----------------------------------------------------------------------
void sstCopyVertex(SstRegs *sst)
{
    FxU32 itemp, exp;
    CsimPrivate *cp = CSIM_PRIVATE(sst);
    SVertex *p;

    p = cp->tsuData.vArray + cp->tsuData.vertexCount;
    GDBG_INFO(127,"setup: v[%d] <= setup_regs\n",cp->tsuData.vertexCount);

    // snapping is done via mantissa truncation
    // should we round????
    itemp = sst->sVx;
    exp = (itemp >> 23) & 0xFF;
    if (exp < 123) itemp = 0;		// smaller than 1/16
    else if (exp < 146) itemp &= 0xFFFFFFFF << (146-exp);
    p->x = itemp;

    itemp = sst->sVy;
    exp = (itemp >> 23) & 0xFF;
    if (exp < 123) itemp = 0;		// smaller than 1/16
    else if (exp < 146) itemp &= 0xFFFFFFFF << (146-exp);
    p->y = itemp;

    p->z = sst->sVz;
    p->w = sst->sOowfbi;

    p->r = sst->sRed;
    p->g = sst->sGreen;
    p->b = sst->sBlue;
    p->a = sst->sAlpha;

    p->s0 = sst->sSow0;
    p->t0 = sst->sTow0;
    p->w0 = sst->sOow0;

    p->s1 = sst->sSow1;
    p->t1 = sst->sTow1;
    p->w1 = sst->sOow1;

#ifndef NO_FLOAT
    /* NO_FLOAT
     * just printfs, hopefully we don't need these :)
     */
    if (GDBG_GET_DEBUGLEVEL(128)) {
	FxU32 smode = sst->sSetupMode;

	GDBG_PRINTF("\tx = %8.4f\t0x%x\n", *(float *)&p->x, p->x );
	GDBG_PRINTF("\ty = %8.4f\t0x%x\n", *(float *)&p->y, p->y );
	GDBG_PRINTF("\tz = %8.4f\t0x%x\n", *(float *)&p->z, p->z );
	GDBG_PRINTF("\tw = %8.4f\t0x%x\n", *(float *)&p->w, p->w );
 	if (smode & SST_SETUP_RGB) {
	    GDBG_PRINTF("\tr = %8.4f\t0x%x\n", *(float *)&p->r, p->r );
	    GDBG_PRINTF("\tg = %8.4f\t0x%x\n", *(float *)&p->g, p->g );
	    GDBG_PRINTF("\tb = %8.4f\t0x%x\n", *(float *)&p->b, p->b );
	}
 	if (smode & SST_SETUP_A) {
	    GDBG_PRINTF("\ta = %8.4f\t0x%x\n", *(float *)&p->a, p->a );
	}
 	if (smode & SST_SETUP_ST0) {
	    GDBG_PRINTF("\ts0 = %8.4f\t0x%x\n",*(float *)&p->s0,p->s0);
	    GDBG_PRINTF("\tt0 = %8.4f\t0x%x\n",*(float *)&p->t0,p->t0);
	}
 	if (smode & SST_SETUP_W0) {
	    GDBG_PRINTF("\tw0 = %8.4f\t0x%x\n",*(float *)&p->w0,p->w0);
	}
 	if (smode & SST_SETUP_ST1) {
	    GDBG_PRINTF("\ts1 = %8.4f\t0x%x\n",*(float *)&p->s1,p->s1);
	    GDBG_PRINTF("\tt1 = %8.4f\t0x%x\n",*(float *)&p->t1,p->t1);
	}
 	if (smode & SST_SETUP_W1) {
	    GDBG_PRINTF("\tw1 = %8.4f\t0x%x\n",*(float *)&p->w1,p->w1);
	}
    }
#endif /* #ifndef NO_FLOAT */
}
