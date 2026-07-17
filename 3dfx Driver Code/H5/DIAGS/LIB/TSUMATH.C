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
** $Date: 10/11/00 8:12:10 PM$
*/

#include "udiag.h"
#include "sstdiag.h"

#define uchar FxU8
#define ulong FxU32
#define ulonglong FxI64
#define BITN(s,n)       (ulong)(((s) >> n) & 1)
#define EXP(s)          (((s) >> 23) & 0xFF)
#define MAN(s)          ((s) & 0x7FFFFF)
#define SIGN(s)         (((s) >> 31) & 1)

void tsu_clamp(ulong *value)
{
  if ( EXP(*value) == 0 )
    *value = 0;
}

void tsu_floatsub (ulong a, ulong b, ulong *s)
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

void tsu_floatmul(ulong a, ulong b, ulong *p)
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

void tsu_floatrecip(ulong in, ulong *out)
{
	ulong	osig, oexp, oman;

	recip (SIGN(in), MAN(in) | 0x800000, EXP(in), &osig, &oman, &oexp);

	*out = osig << 31;
	*out |= (oexp << 23);
	*out |= oman;
}
