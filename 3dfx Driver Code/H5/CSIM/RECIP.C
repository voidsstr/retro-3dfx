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
** $Date: 10/11/00 8:09:07 PM$
*/

#include "h3.h"
#include "h3sim.h"
#include "trexfunc.h"

void sstRecipInit(SstRegs *sst)
{
    FXUNUSED(sst);
    trx_x2_init_table();
    trx_log2_init_table();
    trx_inv_init_table();		// jimm's hardware accurate model
}

//----------------------------------------------------------------------
// convert 48-bit 1/w into floating point W that is used for fog and wbuffer
//----------------------------------------------------------------------
int sstWfloat64(SstRegs *sst, FxI64 w64)
{
    unsigned long exp, mask, i;
    FXUNUSED(sst);

    if((sst->renderMode & SST_RM_3D_MODE) == SST_RM_32BPP)
      {
	// if 1/w is >= 1 then W <= 1 which means the float value is 0
	// or if 1/w is negative then also return 0
	// we test both of these by testing the integer bits of w64
	if (FX_LO64(FX_SHR64(w64,SST_W64_FRACBITS))&SST_MASK(SST_W64_INTBITS)) return 0;
	
	// convert to .32 format
	i = FX_LO64(FX_SHL64(w64,32-SST_W64_FRACBITS));
	if (i == 0) return(0xFFFFFF);
	
	for(exp=0,mask=0x80000000; !(mask&i); mask>>=1)
	  exp++;
	
	GDBG_INFO(180,"sstWfloat64: exp=%d\n",exp);
	if (exp > 31) return 0xFFFFFF; 
	
	i <<= exp;					// create mantissa left justified
	// mask off the hidden one, or lsbs of mantissa
	i &= 0x7FFFFFFF;
	
	// NOTE: when index is 0, we end up incrementing the exponent!
	// and when 1/w = 1.0, exp is F so the increment carries thru and exp=0
	i = (exp<<19) + (0x80000-(i>>12));
	if (i > 0x1000000) i = 0x1000000;		// hardware actually does this
      }
    else  //15bpp and 16bpp
      {
	// if 1/w is >= 1 then W <= 1 which means the float value is 0
	// or if 1/w is negative then also return 0
	// we test both of these by testing the integer bits of w64
	if (FX_LO64(FX_SHR64(w64,SST_W64_FRACBITS))&SST_MASK(SST_W64_INTBITS)) return 0;

	// convert to .32 format
	i = FX_LO64(FX_SHL64(w64,32-SST_W64_FRACBITS));
	if (i == 0) return 0xFFFF00;
	for(exp=0,mask=0x80000000; !(mask&i); mask>>=1)
	  exp++;

	GDBG_INFO(180,"sstWfloat64: exp=%d\n",exp);
	if (exp > 15) return 0xFFFF00;
	i = i >> (16-exp);
	i &= 0x7FFF;

	// NOTE: when index is 0, we end up incrementing the exponent!
	// and when 1/w = 1.0, exp is F so the increment carries thru and exp=0
	i = (exp<<12) + (0x1000-(i>>3));
	
	//Shift up so it's 24 bits; then it will be properly aligned with the 32bpp value.
	i=i<<8;
      }

	GDBG_INFO(180,"sstWfloat64: returned %x\n",i);
    return i;
}

//----------------------------------------------------------------------
// compute texture u,v indicies as s/w and t/w, LOD not taken into account
// input comes from sst->_spanTrex.*64
// store the results in sst->trexData._*
//----------------------------------------------------------------------
void sstTextureUV(SstRegs *sst)
{
    TmuData *tmu = TMU_PRIVATE(sst);

#ifndef NO_FLOAT
    /* NO_FLOAT
     * another recipFlag, we don't care about un-bit-accurate rendering
     * in NO_FLOAT mode, so we can delete this block
     */

    // use TRUE FLOAT DIVISION and MULTIPLY and cheap optimization for AFFINE
    if (CSIM_PRIVATE(sst)->environment.recipFlag != 0) {
	float fw;
	FxI64 s64,t64,w64;

	s64 = tmu->spanTrex.s64;
	t64 = tmu->spanTrex.t64;
	w64 = tmu->spanTrex.w64;

	s64 = FX_SGNEXT64(s64,SST_ST64_SIZE-1);
	t64 = FX_SGNEXT64(t64,SST_ST64_SIZE-1);
	tmu->st.hw_w_is_neg = 0;

	if (sst->textureMode & SST_TPERSP_ST) {
	    // have to detect over/underflow
	    if (sst->textureMode & SST_TCLAMPW) {
		// if w is negative then force to texel [0,0]
		// negative W is defined as [-4K,0)
		if ((FX_HI64(w64)&0xF000)==0xF000) {
		    tmu->st.hw_w_is_neg = 1;
		    return;
		}
	    }
	    fw = FX_64TOFLOAT(w64);
	    if (FX_EQ064(w64))			// if w==0 
		w64 = FX_BIT64(0);		// set to minimum value
	    fw = (1<<SST_UV_FRACBITS)/fw;
	}
	else	// else affine => force w=1.0
            fw = (float)(1<<SST_UV_FRACBITS)/(float)FX_64TOFLOAT( FX_BIT64(SST_W64_FRACBITS) );
	tmu->u = (int)(fw * FX_64TOFLOAT(s64));
	tmu->v = (int)(fw * FX_64TOFLOAT(t64));
	return;
    }
#endif /* #ifndef NO_FLOAT */

    // use BIT-ACCURATE HARDWARE emulation
    {
	tmu->st.it_s_i64 = tmu->spanTrex.s64;		// load up input
	tmu->st.it_t_i64 = tmu->spanTrex.t64;
	tmu->st.it_w_inv_i64 = tmu->spanTrex.w64;
	tmu->st.tpersp_st = sst->textureMode & SST_TPERSP_ST;
	tmu->st.tmirrors = sst->tLOD & SST_TMIRRORS;
	tmu->st.tmirrort = sst->tLOD & SST_TMIRRORT;
	trx_st(&tmu->st);				// calculate results
	tmu->u = FX_LO64(tmu->st.hw_s_fxd_12_i64);
	tmu->v = FX_LO64(tmu->st.hw_t_fxd_12_i64);

	//Decide how much precision to use
	if(sst->tLOD & SST_TBIG)
	  {
	    //for 2048x2048 textures, use 11 bits of fraction
	    tmu->u = ((tmu->u>>4) << (SST_UV_FRACBITS + 3)) | tmu->st.hw_s_frac_11;
	    tmu->v = ((tmu->v>>4) << (SST_UV_FRACBITS + 3)) | tmu->st.hw_t_frac_11;
	  }
	else
	  {
	    //for 256x256 textures, use 8 bits of fraction
	    tmu->u = ((tmu->u>>4) << SST_UV_FRACBITS) | tmu->st.hw_s_frac_8;
	    tmu->v = ((tmu->v>>4) << SST_UV_FRACBITS) | tmu->st.hw_t_frac_8;
	  }
  
	GDBG_INFO(170,"\traw u,v= %x %x    clamp flags: u:%c%c v:%c%c w:%c\n",
		  tmu->u,tmu->v,
		  tmu->st.hw_s_fxd_is_neg ? '-':' ',
		  tmu->st.hw_s_fxd_clmp_pos ? '+':' ',
		  tmu->st.hw_t_fxd_is_neg ? '-':' ',
		  tmu->st.hw_t_fxd_clmp_pos ? '+':' ',
		  tmu->st.hw_w_is_neg ? '-':' ');
	// HACK: trx_st mask s,t to 8.4, here we take into account the clamp
	//	bits and either make the number very negative or positive
	//	while keeping the modulo the same
	tmu->u &= ~0x20000000;
	if (tmu->st.hw_s_fxd_is_neg)
	    tmu->u |= 0xC0000000;
	else if (tmu->st.hw_s_fxd_clmp_pos)
	    tmu->u |= 0x40000000;
	tmu->v &= ~0x20000000;
	if (tmu->st.hw_t_fxd_is_neg)
	    tmu->v |= 0xC0000000;
	else if (tmu->st.hw_t_fxd_clmp_pos)
	    tmu->v |= 0x40000000;
    }
}
