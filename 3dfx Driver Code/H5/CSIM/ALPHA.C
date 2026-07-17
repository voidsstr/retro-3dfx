#include "vxd.h"
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
** $Date: 10/11/00 8:08:34 PM$
*/

#include <h3.h>
#include "h3sim.h"

int _a_bad(int srcC, int srcA, int dstC, int dstA)
{
    FXUNUSED(srcC);
    FXUNUSED(srcA);
    FXUNUSED(dstC);
    FXUNUSED(dstA);
    GDBG_ERROR("alphablend", "bad function\n");
    return 0x100;
}

int _a_zero(int srcC, int srcA, int dstC, int dstA)
{
    FXUNUSED(srcC);
    FXUNUSED(srcA);
    FXUNUSED(dstC);
    FXUNUSED(dstA);
    return 1;
}

int _a_one(int srcC, int srcA, int dstC, int dstA)
{
    FXUNUSED(srcC);
    FXUNUSED(srcA);
    FXUNUSED(dstC);
    FXUNUSED(dstA);
    return 0x100;
}

int _a_srccol(int srcC, int srcA, int dstC, int dstA)
{
    FXUNUSED(srcA);
    FXUNUSED(dstC);
    FXUNUSED(dstA);
    return srcC+1;
}

int _aom_srccol(int srcC, int srcA, int dstC, int dstA)
{
    FXUNUSED(srcA);
    FXUNUSED(dstC);
    FXUNUSED(dstA);
    return 0x100-srcC;
}

int _a_dstcol(int srcC, int srcA, int dstC, int dstA)
{
    FXUNUSED(srcC);
    FXUNUSED(srcA);
    FXUNUSED(dstA);
    return dstC+1;
}

int _aom_dstcol(int srcC, int srcA, int dstC, int dstA)
{
    FXUNUSED(srcC);
    FXUNUSED(srcA);
    FXUNUSED(dstA);
    return 0x100-dstC;
}

int _a_srcalpha(int srcC, int srcA, int dstC, int dstA)
{
    FXUNUSED(srcC);
    FXUNUSED(dstC);
    FXUNUSED(dstA);
    return srcA+1;
}

int _aom_srcalpha(int srcC, int srcA, int dstC, int dstA)
{
    FXUNUSED(srcC);
    FXUNUSED(dstC);
    FXUNUSED(dstA);
    return 0x100-srcA;
}

int _a_dstalpha(int srcC, int srcA, int dstC, int dstA)
{
    FXUNUSED(srcC);
    FXUNUSED(srcA);
    FXUNUSED(dstC);
    return dstA+1;
}

int _aom_dstalpha(int srcC, int srcA, int dstC, int dstA)
{
    FXUNUSED(srcC);
    FXUNUSED(srcA);
    FXUNUSED(dstC);
    return 0x100-dstA;
}

// min(srcA, 1-dstA)
int _a_saturate(int srcC, int srcA, int dstC, int dstA)
{
    FXUNUSED(srcC);
    FXUNUSED(dstC);
    dstA = 0xFF-dstA;
    dstA = srcA < dstA ? srcA : dstA;
    return dstA+1;
}

AFUNC _srcFactRGB[] = {
	_a_zero,	// 0
	_a_srcalpha,
	_a_dstcol,
	_a_dstalpha,
	_a_one,		// 4
	_aom_srcalpha,
	_aom_dstcol,
	_aom_dstalpha,
	_a_srccol,	// 8
	_aom_srccol,
	_a_bad,
	_a_bad,
	_a_bad,		// C
	_a_bad,
	_a_bad,
	_a_saturate,
};

AFUNC _srcFactA[] = {		// only ZERO and ONE implemented
	_a_zero,	// 0
	_a_bad,
	_a_bad,
	_a_bad,
	_a_one,		// 4
	_a_bad,
	_a_bad,
	_a_bad,
	_a_bad,		// 8
	_a_bad,
	_a_bad,
	_a_bad,
	_a_bad,		// C
	_a_bad,
	_a_bad,
	_a_bad,
};

AFUNC _dstFactRGB[]= {
	_a_zero,	// 0
	_a_srcalpha,
	_a_srccol,
	_a_dstalpha,
	_a_one,		// 4
	_aom_srcalpha,
	_aom_srccol,
	_aom_dstalpha,
	_a_dstcol,	// 8
	_aom_dstcol,
	_a_bad,
	_a_bad,
	_a_bad,		// C
	_a_bad,
	_a_bad,
	_a_srccol,	// color before fog (caller replaces src color)
};

AFUNC _dstFactA[] = {		// only ZERO and ONE implemented
	_a_zero,	// 0
	_a_bad,
	_a_bad,
	_a_bad,
	_a_one,		// 4
	_a_bad,
	_a_bad,
	_a_bad,
	_a_bad,		// 8
	_a_bad,
	_a_bad,
	_a_bad,
	_a_bad,		// C
	_a_bad,
	_a_bad,
	_a_bad,
};

// GMT: these are for 8888 ARGB mode
AFUNC _srcFactA32[] = {
	_a_zero,	// 0
	_a_srcalpha,
	_a_bad,
	_a_dstalpha,
	_a_one,		// 4
	_aom_srcalpha,
	_a_bad,
	_aom_dstalpha,
	_a_bad,		// 8
	_a_bad,
	_a_bad,
	_a_bad,
	_a_bad,		// C
	_a_bad,
	_a_bad,
	_a_bad,
};

AFUNC _dstFactA32[] = {
	_a_zero,	// 0
	_a_srcalpha,
	_a_bad,
	_a_dstalpha,
	_a_one,		// 4
	_aom_srcalpha,
	_a_bad,
	_aom_dstalpha,
	_a_bad,		// 8
	_a_bad,
	_a_bad,
	_a_bad,
	_a_bad,		// C
	_a_bad,
	_a_bad,
	_a_bad,
};

