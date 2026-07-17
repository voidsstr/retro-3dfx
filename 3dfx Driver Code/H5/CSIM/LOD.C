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
** $Revision: 2$
** $Date: 10/11/00 8:08:58 PM$
*/

#include <stdlib.h>
#include <math.h>

#include <h3.h>
#include "h3sim.h"
#include "trexfunc.h"

// if COMPARE is non-zero then compare true LOD with hardware calculated value
#if defined(KERNEL) || defined(__WATCOMC__)
#define COMPARE 0
#else
#define COMPARE 0
//#define COMPARE 1
#endif

//----------------------------------------------------------------------
#if COMPARE

// analyze the components of Rho and see what went wrong
static void _analyze(SstRegs *tmuregs)
{
    FxI64 s64,t64;
    double du, dv, rhox,rhoy, u,v,w,dwdx,dwdy;
    double TWO32 = pow(2.0,32.0);
    TmuData *tmu = TMU_PRIVATE(tmuregs);

    w = 1.0F/FX_64TOFLOAT(tmu->spanTrex.w64);
    dwdx = FX_64TOFLOAT(tmu->dwdx64);
    dwdy = FX_64TOFLOAT(tmu->dwdy64);
    s64 = tmu->spanTrex.s64;
    s64 = FX_SGNEXT64(s64,SST_ST64_SIZE-1);
    t64 = tmu->spanTrex.t64;
    t64 = FX_SGNEXT64(t64,SST_ST64_SIZE-1);
    u = w * FX_64TOFLOAT(s64);
    v = w * FX_64TOFLOAT(t64);

    gdbg_printf("analyze: S,T,W = %.6f %.6f %.6f\n",u,v,w*TWO32);
    // (dsdx - U*dwdx)*W
    // (dsdx - U*dwdx)*W
    du = (FX_64TOFLOAT(FX_SGNEXT64(tmu->dsdx64,SST_ST64_SIZE-1)) - u * dwdx) * w;
    dv = (FX_64TOFLOAT(FX_SGNEXT64(tmu->dtdx64,SST_ST64_SIZE-1)) - v * dwdx) * w;
    rhox = du*du + dv*dv;
    gdbg_printf("       :  dudx = %.6f dvdx = %.6f du+v=%.6f\n",du,dv,rhox);
    gdbg_printf("       :  dudx = %.6f - %.6f (%.6f * %.6f)\n",
			FX_64TOFLOAT(tmu->dsdx64)/TWO32,
			u*dwdx/TWO32,u,dwdx/TWO32);
    gdbg_printf("       :  dvdx = %.6f - %.6f (%.6f * %.6f)\n",
			FX_64TOFLOAT(tmu->dtdx64)/TWO32,
			v*dwdx/TWO32,v,dwdx/TWO32);
    
    du = (FX_64TOFLOAT(FX_SGNEXT64(tmu->dsdy64,SST_ST64_SIZE-1)) - u * dwdy) * w;
    dv = (FX_64TOFLOAT(FX_SGNEXT64(tmu->dtdy64,SST_ST64_SIZE-1)) - v * dwdy) * w;
    rhoy = du*du + dv*dv;
    gdbg_printf("       :  dudy = %.6f dvdy = %.6f du+v=%.6f\n",du,dv,rhoy);
    gdbg_printf("       :  dudy = %.6f - %.6f (%.6f * %.6f)\n",
			FX_64TOFLOAT(tmu->dsdy64)/TWO32,
			u*dwdy/TWO32,u,dwdy/TWO32);
    gdbg_printf("       :  dvdy = %.6f - %.6f (%.6f * %.6f)\n",
			FX_64TOFLOAT(tmu->dtdy64)/TWO32,
			v*dwdy/TWO32,v,dwdy/TWO32);
}

static void _compare(SstRegs *tmuregs, double approx, double god)
{
    static double errmax=0.0;
    double diff;
    CsimPrivate *cp = CSIM_PRIVATE(tmuregs);

    // do not worry about errors for negative LOD
    if (approx <= 0 && god <= 0) return;
    diff = god-approx;			// error
    if (diff < 0.0F) diff = -diff;	// absolute error
    if (diff > errmax) {
	gdbg_printf("errmax: %8g - %8g = %8g\n",god, approx, diff);
	errmax = diff;
    }
    if (diff > cp->environment.loderr) {
	gdbg_printf("LODERR: %8g - %8g = %8g\n",god, approx, diff);
	_analyze(tmuregs);
#if 0
	while (sst->_myNumber >= 0)	// locate FBI
	    sst = sst->_nextTrex;
	sstPrintRegs(sst,"LODERR:");
	sstPrintSpanRegs(sst,"LODERR",0);
#endif
    }
}

#endif

#ifndef NO_FLOAT
/* NO_FLOAT
 * _exactRho is only used in un-bit-accurate mode and COMPARE mode, so we
 * can safely delete the entire function if we don't need these
 */

//----------------------------------------------------------------------
// extremely accurate method in floating point
static double _exactRho(SstRegs *tmuregs)
{
    FxI64 s64,t64;
    double du, dv, rho, u,v,w,dwdx,dwdy;
    TmuData *tmu = TMU_PRIVATE(tmuregs);

    // if w is negative return 1.0 (force LOD 0)
    // negative W is defined as [-4K,0)
    if ((FX_HI64(tmu->spanTrex.w64)&0xF000)==0xF000)
	return 1.0;	
    if (FX_EQ064(tmu->spanTrex.w64))
	w = 1.0F/FX_64TOFLOAT(FX_BIT64(0));
    else
	w = 1.0F/FX_64TOFLOAT(tmu->spanTrex.w64);
    dwdx = FX_64TOFLOAT(tmu->dwdx64);
    dwdy = FX_64TOFLOAT(tmu->dwdy64);
    s64 = tmu->spanTrex.s64;
    s64 = FX_SGNEXT64(s64,SST_ST64_SIZE-1);
    t64 = tmu->spanTrex.t64;
    t64 = FX_SGNEXT64(t64,SST_ST64_SIZE-1);
    u = w * FX_64TOFLOAT(s64);
    v = w * FX_64TOFLOAT(t64);

    // (dsdx - U*dwdx)*W
    du = (FX_64TOFLOAT(FX_SGNEXT64(tmu->dsdx64,SST_ST64_SIZE-1)) - u * dwdx) * w;
    dv = (FX_64TOFLOAT(FX_SGNEXT64(tmu->dtdx64,SST_ST64_SIZE-1)) - v * dwdx) * w;
    rho = du*du + dv*dv;
    du = (FX_64TOFLOAT(FX_SGNEXT64(tmu->dsdy64,SST_ST64_SIZE-1)) - u * dwdy) * w;
    dv = (FX_64TOFLOAT(FX_SGNEXT64(tmu->dtdy64,SST_ST64_SIZE-1)) - v * dwdy) * w;
    du = du*du + dv*dv;
    if (du > rho) rho = du;		// take max
    return rho;
}
#endif /* #ifndef NO_FLOAT */

static int _sstLod(SstRegs *tmuregs)
{
    TmuData *tmu = TMU_PRIVATE(tmuregs);

    tmu->st.reg_dsdx_i64 = tmu->dsdx64;		// load up input
    tmu->st.reg_dsdy_i64 = tmu->dsdy64;
    tmu->st.reg_dtdx_i64 = tmu->dtdx64;
    tmu->st.reg_dtdy_i64 = tmu->dtdy64;
    tmu->st.reg_dwdx_i64 = tmu->dwdx64;
    tmu->st.reg_dwdy_i64 = tmu->dwdy64;
    trx_lod(&tmu->st);			// calculate results
    return tmu->st.hw_lod_7_8s;		// retrieve and return results
}

//----------------------------------------------------------------------
// return the LOD value computed by the hardware, 8.8 fixed point format
// LOD is computed as log(W) + log(rho)/2 where the logs are table based
//	and W is the reciprocal of the w iterator
//----------------------------------------------------------------------
int sstLod(SstRegs *tmuregs)
{
    int lod;
    CsimPrivate *cp = CSIM_PRIVATE(tmuregs);

#ifndef NO_FLOAT
    /* NO_FLOAT
     * we don't care about un-bit-accurate rendering in NO_FLOAT mode,
     * so we can delete this expression
     */
    if (cp->environment.recipFlag)
	return (int)(256*0.5*1.4427*log(_exactRho(tmuregs)));
#endif /* #ifndef NO_FLOAT */

    lod = _sstLod(tmuregs);			// returns .8 format
#if COMPARE
    _compare(tmuregs, lod/(double)(1<<8),		// optional compare
		0.5*1.4427*log(_exactRho(tmuregs)));	// log2(sqrt(rho))
#endif
    return lod;
}
