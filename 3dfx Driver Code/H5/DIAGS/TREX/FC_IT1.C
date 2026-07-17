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
** $Date: 10/11/00 8:19:04 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"


#define JM_BIT(n)  (1 << (n))

#define INT64_1632_TO_DOUBLE(x) (FX_64TODBL(FX_SGNEXT64((x), 47)) \
							/ FX_64TODBL(FX_CREATE64(1, 0)))
#define TRX_PR_INT64(VAR_NAME, VAR) (printf("%s = 0x%08x_%08x = %f\n", \
			VAR_NAME, FX_HI64(VAR), \
			FX_LO64(VAR),  INT64_1632_TO_DOUBLE(VAR)))


#if defined(__sparc__) && ! defined(__svr4__)   /* BSDish sunos only */
   #define TRX_LOG2_DBL(x) (log2(x))
#else
   #define TRX_LOG2_DBL(x) (log(x) / log(2.0))
#endif


#define SET_TLOD(sst_ptr, sstj_ptr) SET( (sst_ptr)->tLOD, \
			  ((sstj_ptr)->tlod.min << SST_LODMIN_SHIFT) 		\
			| ((sstj_ptr)->tlod.max << SST_LODMAX_SHIFT) 		\
			| ((sstj_ptr)->tlod.bias << SST_LODBIAS_SHIFT) 		\
		\
			| ((sstj_ptr)->tlod.odd ? SST_LOD_ODD : 0)		\
			| ((sstj_ptr)->tlod.tsplit ? SST_LOD_TSPLIT : 0)		\
			| ((sstj_ptr)->tlod.s_is_wider ? SST_LOD_S_IS_WIDER : 0)		\
		\
			| ((sstj_ptr)->tlod.aspect << SST_LOD_ASPECT_SHIFT) 			\
		\
			| ((sstj_ptr)->tlod.zerofrac ? SST_LOD_ZEROFRAC : 0)		\
			| ((sstj_ptr)->tlod.tdata_swizzle ? SST_TMULTIBASEADDR : 0)		\
			| ((sstj_ptr)->tlod.tdata_swizzle ? SST_TDATA_SWIZZLE : 0)		\
			| ((sstj_ptr)->tlod.tdata_swap ? SST_TDATA_SWAP : 0)		\
			| ((sstj_ptr)->tlod.tdirect_write ? SST_TDIRECT_WRITE : 0));


typedef struct {

	struct {
		FxU32 min;
		FxU32 max;
		FxU32 bias;
		FxU32 odd;
		FxU32 tsplit;
		FxU32 s_is_wider;
		FxU32 aspect;
		FxU32 zerofrac;
		FxU32 tmultibaseaddr;
		FxU32 tdata_swizzle;
		FxU32 tdata_swap;
		FxU32 tdirect_write;
	} tlod;

} SSTJ_STRUCT;



#define TRI_PARAM_DWDXY		0
#define TRI_PARAM_ST			1
#define TRI_PARAM_DSTDXY	2
#define TRI_PARAM_W			3


#define ABS_VAL(x)		(((x) > 0) ? (x) : -(x))
#define FLOAT_CAST(int32_x) ( *((float *) &( int32_x )) )

//#define FLOAT_CONV_1_31(int32_x) (((int32_x) & 0x80000000) \ -
//		| 0x3f800000 \ -
//		| (((((int32_x) & 0x80000000) ? ~(int32_x) : (int32_x)) >> 7) & 0x7fffff))



// **************************************************************************
float conv_16_32_2float (FxI64 in16_32)
{
	FxI32 res_sgn_i32;
	FxI64 res_mant_i64;
	FxI32 res_mant_i32;
	FxI32 res_exp_i32;

	FxI32 res_i32;


	res_mant_i64 = in16_32;

	if (FX_HI64(res_mant_i64) & 0x8000) {
		// negative
		res_sgn_i32 = 0x80000000;
		res_mant_i64 = FX_NEG64(res_mant_i64);
	} else {
		// positive
		res_sgn_i32 = 0x00000000;
	}

	// res_mant_i64 is now pos. 15.32

	res_exp_i32 = 0x46800000;	// 2^14

	while (((FX_HI64(res_mant_i64) & 0x4000) == 0) && (res_exp_i32 >= 0x00800000)) {
		res_mant_i64 = FX_SHL64(res_mant_i64, 1);
		res_exp_i32 -= 0x00800000;
	}
	// Leading one ends up in bit 46.  Shift it right to bit 23 where it masked 
	//   off because it is the implied one.
	res_mant_i32 = FX_LO64(FX_SHR64(res_mant_i64, 23)) & 0x007fffff;

	res_i32 = res_sgn_i32 | res_exp_i32 | res_mant_i32;

	// debug
	//TRX_PR_INT64("in16_32", in16_32);
	//printf(      "res_i32 = 0x%08x\n", res_i32);

	return( FLOAT_CAST(res_i32) );
}



// **************************************************************************
void do_1pix_tri_1q (
		SstRegs *sst,
		int *tri_cnt,
		long *tri_regs[4],	// register set for the group
		float tri_vals[4],		// values to write to the group for this triangle
		Triangle *t
		)
{
	// only write required registers to minimize test length
	// always write params that the iterators alter

	double dbl_sq, dbl_log_max, dbl_log_w, dbl_lod;
	FxI32 x_coord, y_coord;



	// calc lod
	dbl_sq = pow((((double) tri_vals[TRI_PARAM_DWDXY] 
			* (tri_vals[TRI_PARAM_ST] / tri_vals[TRI_PARAM_W]))
					- tri_vals[TRI_PARAM_DSTDXY]), 2.0);
	if (dbl_sq == 0.0)
		dbl_log_max = -1000000.0;
	else
		dbl_log_max = ((double) 0.5) * TRX_LOG2_DBL(ABS_VAL(dbl_sq));

	//	st->dbl_log_w = -1000000.;
	dbl_log_w = (tri_vals[TRI_PARAM_W] <= 0.0) ? -1000000.0 : 
						TRX_LOG2_DBL(1.0 / tri_vals[TRI_PARAM_W]);
	dbl_lod = dbl_log_w + dbl_log_max;



	gdbg_info(200, "fc_lod.c:     tri_cnt = %d\n", *tri_cnt);
	gdbg_info(200, "fc_lod.c:     (DWDXY * (ST / IT_W) - DSTDXY = (%f * (%f / %f)) - %f\n", 
								tri_vals[TRI_PARAM_DWDXY],
								tri_vals[TRI_PARAM_ST],
								tri_vals[TRI_PARAM_W],
								tri_vals[TRI_PARAM_DSTDXY]);
	gdbg_info(200, "fc_lod.c:     dbl_sq      = %f\n", dbl_sq);
	gdbg_info(200, "fc_lod.c:     dbl_log_max = %f\n", dbl_log_max);
	gdbg_info(200, "fc_lod.c:     dbl_log_w   = %f\n", dbl_log_w);
	gdbg_info(200, "fc_lod.c:     dbl_lod     = %f\n", dbl_lod);


	// dwd*
	SETF(*tri_regs[TRI_PARAM_DWDXY], tri_vals[TRI_PARAM_DWDXY]);

	// s,t
	SETF(*tri_regs[TRI_PARAM_ST], tri_vals[TRI_PARAM_ST]);
	if (tri_regs[TRI_PARAM_ST] != &(sst->Fs))
		SETF(sst->Fs, (float) 0.0);
	else
		SETF(sst->Ft, (float) 0.0);

	// dst/dxy
	SETF(*tri_regs[TRI_PARAM_DSTDXY], tri_vals[TRI_PARAM_DSTDXY]);

	// w
	SETF(sst->Fw, tri_vals[TRI_PARAM_W]);


	//	printStwTriangle(4,t);
	//if (!setupStwTriangle(t)) {		// setup STW slopes
	//	gdbg_error("main", "bad triangle setup\n");
	//}
	//printStwTriangle(4,t);
	//printStwTriangleSlopes(5,t);


	// xxx if this out later for hw only sim
	x_coord = (*tri_cnt) % 640 ;
	y_coord = (*tri_cnt) / 640 ;

 	SET(sst->vA.x, t->vA.x + x_coord * XY_ONE);
 	SET(sst->vA.y, t->vA.y + y_coord * XY_ONE);
 	SET(sst->vB.x, t->vB.x + x_coord * XY_ONE);
 	SET(sst->vB.y, t->vB.y + y_coord * XY_ONE);
 	SET(sst->vC.x, t->vC.x + x_coord * XY_ONE);
 	SET(sst->vC.y, t->vC.y + y_coord * XY_ONE);


	SET(sst->triangleCMD, t->area);
	(*tri_cnt)++;
}


float flt_clmp_w(float w_preclamp)
{
	if (w_preclamp > 60000.0)
		return((float) 60000.0);
	else if (w_preclamp < -4000.0)
		return((float) -4000.0);
	else
		return(w_preclamp);
}

float flt_clmp_16_32(float preclamp)
{
	if (preclamp > 32000.0)
		return((float) 32000.0);
	else if (preclamp < -32000.0)
		return((float) -32000.0);
	else
		return(preclamp);
}


float rand_16_32()
{
	FxI32 rand_mant, rand_exp;

	rand_mant = iRandom(0xFFFFFFFF);
	rand_exp = iRandom(0x0000003F) % 48;

	// mantissa range +-1
	// exponent range [-32, +15]
	return( (float) (
			  pow(2.0, (rand_exp - 32))			// exponent
			* ((float) rand_mant)					// range +- 2^31
			/ ((float) ((FxI32) 0x80000000))		// -(2^31)
			));

	//((rand_mant & 0x80000000) ? -1.0 : 1.0)
}



// **************************************************************************
// **************************************************************************
void
main (int argc, char **argv)
{
  Triangle *t;
	SstRegs *sst;

//	int reg_group;
//	int tri_cnt;

//	float tri_param[][4] = {
//		{9.0, 4.0, 4.0, 1.0},
//		{128.0, 128.0, ((128.0 * 128.0) + 8.0), 1.0},
//		{2.0, 1.0, 1.0, 1.0},
//		{0.0, 0.0, 1.0, 1.0}
//	};
//	long *tri_regs[4];	// register set for the group
//	float tri_vals[4];

	FxU32 test_textureMode;

//	int der_bit, der_sign, st_bit, st_sign;
//	int zero_prod, rand_mantissa, dstdxy_sign, dstdxy_bit_pos;

//	FxU32 rand1, rand2;

//	FxU32 x_coord, y_coord;
//	int cnt;
//	float dsdx, dtdx, dsdy, dtdy, lod_no_w;


	SSTJ_STRUCT sstj_tmp;
	SSTJ_STRUCT* sstj = &sstj_tmp;

//	FxU32 tmp_fxu32;
//	float w_flt, lod_flt, dsdx_flt;



	// 4 groups (other components == 0, w == 1.0)
	// 0:	dwdx	s	dsdx
	// 1:	dwdx	t	dtdx
	// 2:	dwdy	s	dsdy
	// 3:	dwdy	t	dtdy
	// general:  dwdxy  st  dstdxy

	// observeable range of output
	// x.8  [2^(-8) - lod_bias, 8.0 - lod_bias] 

	// tris/group = 50000 vectors / (1 tri/15 clk) / (2 clk/1 vec) / (4 groups)
	//    = ~400 tris/group


	sst = SST_BEGIN(argc,argv);

	if(diago.bigAssTextures)
	  t = buildTriangle(2048, 2048);
	else
	  t = buildTriangle(256, 256);

	t->next = NULL;
	diago.diff = 1;	// always diff screens if possible
	diago.adjust = 0;
	diago.perspective = 1;
	diago.bilinear = 1;	// always
	diago.loddither = 1;


	// use the 16-bit 565 RGB texture format, size = 8x8, in replace mode
	t->tex->tMode = (diago.perspective?SST_TPERSP_ST:0) | 
			SST_RGB565 | SST_TC_REPLACE |
			SST_TCA_ZERO_OTHER | SST_TCA_SUB_CLOCAL |
			SST_TCA_MLODFRAC | SST_TCA_ADD_CLOCAL |
			SST_TCA_REVERSE_BLEND;
	texRandomTextureMap(sst, diago.trex, 
				1, // yes mipmaps
				8, // 2^8
				8, // by 2^8 size texture
				t->tex);
	SET(sst->fbzColorPath, SST_RGBSEL_TREXOUT | SST_ENTEXTUREMAP |
			(diago.adjust?SST_PARMADJUST:0));
	// Draws random front and back, as well as alpha.  All 3 are checked at end
	if (diago.rgb == 16)
	    SET(sst->fbzMode, SST_RGBWRMASK | SST_ENALPHABUFFER |
		drawbufferRandom() | SST_ENRECTCLIP);
	else
	    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom() | SST_ENRECTCLIP);



	test_textureMode = 0
				| SST_TPERSP_ST
				| SST_TMINFILTER
				| SST_TMAGFILTER
			// | SST_TCLAMPW
				| SST_TLODDITHER		// exercises bias adder
			// | SST_TNCCSELECT
			// | SST_TCLAMPS
			// | SST_TCLAMPT

         	| SST_RGB565    // SST_TFORMAT

		// c_local
				| SST_TC_REPLACE  // Macro
			// | SST_TC_ZERO_OTHER
			// | SST_TC_SUB_CLOCAL
			// | SST_TC_MONE        // SST_TC_MSELECT
			// | SST_TC_REVERSE_BLEND
			// | SST_TC_ADD_CLOCAL
			// | SST_TC_ADD_ALOCAL
			// | SST_TC_INVERT_OUTPUT

		// a_local * lod_frac
				| SST_TCA_ZERO_OTHER
				| SST_TCA_SUB_CLOCAL
				| SST_TCA_MLODFRAC       // SST_TCA_MSELECT
			//	| SST_TCA_REVERSE_BLEND
				| SST_TCA_ADD_CLOCAL
			// | SST_TCA_ADD_ALOCAL
			// | SST_TCA_INVERT_OUTPUT

			// | SST_TRILINEAR
			// | SST_SEQ_8_DOWNLD
			;
	SET(SST_TREX(sst,t->tex->trex)->textureMode, test_textureMode);


	sstj->tlod.min = 0 << 2;
	sstj->tlod.max = 0 << 2;	// biggest map only - so more bits of s,t seen
	sstj->tlod.bias = 0;
	sstj->tlod.odd = 0;
	sstj->tlod.tsplit = 0;
	sstj->tlod.s_is_wider = 0;
	sstj->tlod.aspect = 0;
	sstj->tlod.zerofrac = 0;
	sstj->tlod.tmultibaseaddr = 0;
	sstj->tlod.tdata_swizzle = 0;
	sstj->tlod.tdata_swap = 0;
	sstj->tlod.tdirect_write = 0;

	SET_TLOD(sst, sstj);



	// 1 pix tri
//	t->vA.x = XY_ONE/2;
//	t->vA.y = 0 * XY_ONE;
//	t->vB.x = 1 * XY_ONE;
//	t->vB.y = 0 * XY_ONE;
//	t->vC.x = XY_ONE/2;
//	t->vC.y = 1 * XY_ONE;



	// *** tri #1
	// +-1k
	t->vA.x = -1000 * XY_ONE;
	t->vA.y = -1000 * XY_ONE;
	t->vB.x = -998 * XY_ONE;
	t->vB.y = -1000 * XY_ONE;
	t->vC.x = 1000 * XY_ONE;
	t->vC.y = 1000 * XY_ONE;



	areaTriangle(t);				// compute the area (before setup)
	// sortTriangle(t);				// sort it
	//drawStwTriangle(sst,t);			// set all basic registers
	SET(sst->vA.x, t->vA.x);
	SET(sst->vA.y, t->vA.y);
	SET(sst->vB.x, t->vB.x);
	SET(sst->vB.y, t->vB.y);
	SET(sst->vC.x, t->vC.x);
	SET(sst->vC.y, t->vC.y);


	SETF(sst->Fs, (float) -1000.0);
	SETF(sst->Fdsdx, (float) -128.0);
	SETF(sst->Fdsdy, (float) 256.0);

	SETF(sst->Ft, (float) -1000.0);
	SETF(sst->Fdtdx, (float) -128.0);
	SETF(sst->Fdtdy, (float) 256.0);

	SETF(sst->Fw, (float) 16.0);
	SETF(sst->Fdwdx, (float) 0.00201234);
	SETF(sst->Fdwdy, (float) -0.00101234);


	SET(sst->triangleCMD, t->area);



	// *** tri #2
	// +-1k
	t->vA.x =  1000 * XY_ONE;
	t->vA.y = -1000 * XY_ONE;
	t->vB.x =  1003 * XY_ONE;
	t->vB.y = -1000 * XY_ONE;
	t->vC.x =  -900 * XY_ONE;
	t->vC.y =  1000 * XY_ONE;


	areaTriangle(t);				// compute the area (before setup)
	// sortTriangle(t);				// sort it
	//drawStwTriangle(sst,t);			// set all basic registers
	SET(sst->vA.x, t->vA.x);
	SET(sst->vA.y, t->vA.y);
	SET(sst->vB.x, t->vB.x);
	SET(sst->vB.y, t->vB.y);
	SET(sst->vC.x, t->vC.x);
	SET(sst->vC.y, t->vC.y);


	SETF(sst->Fs, (float) -500.0);
	SETF(sst->Fdsdx, (float) -128.5);
	SETF(sst->Fdsdy, (float) 256.5);

	SETF(sst->Ft, (float) -500.0);
	SETF(sst->Fdtdx, (float) -128.5);
	SETF(sst->Fdtdy, (float) 256.5);

	SETF(sst->Fw, (float) 18.0);
	SETF(sst->Fdwdx, (float) 0.00200234);
	SETF(sst->Fdwdy, (float) -0.00100234);


	SET(sst->triangleCMD, t->area);



	// *** tri #3
	// small one on-screen to verify completion of other two
	t->vA.x =   600 * XY_ONE;
	t->vA.y =   240 * XY_ONE;
	t->vB.x =   610 * XY_ONE;
	t->vB.y =   240 * XY_ONE;
	t->vC.x =   600 * XY_ONE;
	t->vC.y =   250 * XY_ONE;


	areaTriangle(t);				// compute the area (before setup)
	// sortTriangle(t);				// sort it
	//drawStwTriangle(sst,t);			// set all basic registers
	SET(sst->vA.x, t->vA.x);
	SET(sst->vA.y, t->vA.y);
	SET(sst->vB.x, t->vB.x);
	SET(sst->vB.y, t->vB.y);
	SET(sst->vC.x, t->vC.x);
	SET(sst->vC.y, t->vC.y);


	SETF(sst->Fs, (float) -510.0);
	SETF(sst->Fdsdx, (float) -128.7);
	SETF(sst->Fdsdy, (float) 256.7);

	SETF(sst->Ft, (float) -510.0);
	SETF(sst->Fdtdx, (float) -128.7);
	SETF(sst->Fdtdy, (float) 256.7);

	SETF(sst->Fw, (float) 18.0);
	SETF(sst->Fdwdx, (float) 0.00210234);
	SETF(sst->Fdwdy, (float) -0.00110234);


	SET(sst->triangleCMD, t->area);



// ****************************************************************************************
#if 0

	tri_cnt = 0;

	for (reg_group = 0; reg_group <= 3; reg_group++) {			// do 4 groups
		gdbg_info(200, "fc_lod.c:  reg_group = %d\n", reg_group);

		switch (reg_group) {
			case 0:
				tri_regs[TRI_PARAM_DWDXY]	= &(sst->Fdwdx);
				tri_regs[TRI_PARAM_ST]	= &(sst->Fs);
				tri_regs[TRI_PARAM_DSTDXY]	= &(sst->Fdsdx);
				break;
			case 1:
				tri_regs[TRI_PARAM_DWDXY]	= &(sst->Fdwdx);
				tri_regs[TRI_PARAM_ST]	= &(sst->Ft);
				tri_regs[TRI_PARAM_DSTDXY]	= &(sst->Fdtdx);
				break;
			case 2:
				tri_regs[TRI_PARAM_DWDXY]	= &(sst->Fdwdy);
				tri_regs[TRI_PARAM_ST]	= &(sst->Fs);
				tri_regs[TRI_PARAM_DSTDXY]	= &(sst->Fdsdy);
				break;
			case 3:
				tri_regs[TRI_PARAM_DWDXY]	= &(sst->Fdwdy);
				tri_regs[TRI_PARAM_ST]	= &(sst->Ft);
				tri_regs[TRI_PARAM_DSTDXY]	= &(sst->Fdtdy);
				break;
			default:
				printf("fc_lod.c:  panic\n");
				exit(1);
				break;
		}

		// zero out the unused registers for this group
		if (tri_regs[TRI_PARAM_DWDXY] != &(sst->Fdwdx))
			SETF(sst->Fdwdx, (float) 0.0);
		if (tri_regs[TRI_PARAM_DWDXY] != &(sst->Fdwdy))
			SETF(sst->Fdwdy, (float) 0.0);

		//SETF(sst->Fs, (float) 0.0);
		//SETF(sst->Ft, (float) 0.0);
		//SETF(sst->Fw, (float) 0.0);

		if (tri_regs[TRI_PARAM_DSTDXY] != &(sst->Fdsdx))
			SETF(sst->Fdsdx, (float) 0.0);
		if (tri_regs[TRI_PARAM_DSTDXY] != &(sst->Fdtdx))
			SETF(sst->Fdtdx, (float) 0.0);
		if (tri_regs[TRI_PARAM_DSTDXY] != &(sst->Fdsdy))
			SETF(sst->Fdsdy, (float) 0.0);
		if (tri_regs[TRI_PARAM_DSTDXY] != &(sst->Fdtdy))
			SETF(sst->Fdtdy, (float) 0.0);




//hide - old
//			for (tri = 0; tri < (sizeof(tri_param) / sizeof(float) / 4); tri++) {			
//
//				tri_vals[TRI_PARAM_DWDXY] = tri_param[tri][TRI_PARAM_DWDXY];
//				tri_vals[TRI_PARAM_ST] = tri_param[tri][TRI_PARAM_ST];
//				tri_vals[TRI_PARAM_DSTDXY] = tri_param[tri][TRI_PARAM_DSTDXY];
//				tri_vals[TRI_PARAM_W] = 1.0;
//
//				do_1pix_tri_1q(sst, &tri_cnt, tri_regs, tri_vals, t);
//			}
//


		// multipliers
		// 			walk off end so does full 1's or 0's
		//		walk 0,1 pos.
		//			der
		//       st
		//		walk 0,1 neg.
		//			der
		//       st
		//
		// 128.0 = 0x43000000 float
		// 64.0 = 0x42800000 float
		// bit 11 is sign bit, 1/0 is put in bit 10 OR bits 0 through 10
		for (der_bit = 0; der_bit <= 10; der_bit++) {			// 10 is leading 0/1
			for (der_sign = 0 ; der_sign <= 1; der_sign++) {
				for (st_bit = 0; st_bit <= 10; st_bit++) {			// 10 is leading 0/1
					for (st_sign = 0 ; st_sign <= 1; st_sign++) {

						// form value as a signed 8.4 mantissa, and put it into a float
						tri_vals[TRI_PARAM_DWDXY] = conv_16_32_2float(
								FX_SHL64(FX_XOR64((der_sign ? FX_COMP64(FX_CREATE64(0,0)) 
																	: FX_CREATE64(0,0)),
										 (FX_CREATE64(0, (FxU32) (BIT(10) | BIT(der_bit))))), 28));
						tri_vals[TRI_PARAM_ST] = conv_16_32_2float(
								FX_SHL64(FX_XOR64((st_sign ? FX_COMP64(FX_CREATE64(0,0)) 
																	: FX_CREATE64(0,0)),
										 (FX_CREATE64(0, (FxU32) (BIT(10) | BIT(st_bit))))), 28));

						tri_vals[TRI_PARAM_DSTDXY] = (tri_vals[TRI_PARAM_DWDXY] 
										* tri_vals[TRI_PARAM_ST]) + (float) 16.0;
						tri_vals[TRI_PARAM_W] = (float) 1.0;
	
						do_1pix_tri_1q(sst, &tri_cnt, tri_regs, tri_vals, t);
					}
				}
			}
		}



		// swap
		// shift right
		// add
		// absolute value
		// normalize
		//    2 * 2 * 2 * 47 ~= 400 triangles
		for (zero_prod = 0; zero_prod <= 1; zero_prod++) {
			for (rand_mantissa = 0; rand_mantissa <= 1; rand_mantissa++) {
				for (dstdxy_sign = 0; dstdxy_sign <= 1; dstdxy_sign++) {
					for (dstdxy_bit_pos = 0; dstdxy_bit_pos <= 46; dstdxy_bit_pos++) {

						gdbg_info(200, "fc_lod.c:  zero_prod=%d, rand_mantissa=%d"
											", dstdxy_sign=%d, dstdxy_bit_pos=%d\n", 
									zero_prod, rand_mantissa, dstdxy_sign, dstdxy_bit_pos);

						if (!rand_mantissa) {
							// no random mantissa
							tri_vals[TRI_PARAM_DSTDXY] = conv_16_32_2float(
								FX_XOR64((dstdxy_sign ? FX_COMP64(FX_CREATE64(0,0)) 
															: FX_CREATE64(0,0)),
								FX_SHL64(FX_CREATE64(0,1), dstdxy_bit_pos)));

							tri_vals[TRI_PARAM_DWDXY] = (float) ((dstdxy_sign ? -1.0 : 1.0) * 1.999);
						} else {
							// random mantissa
							rand1 = iRandom(0x00003FFF);
							rand2 = iRandom(0xFFFFFFFF);

							tri_vals[TRI_PARAM_DSTDXY] = conv_16_32_2float(
									FX_XOR64((dstdxy_sign ? 
												FX_COMP64(FX_CREATE64(0,0)) : FX_CREATE64(0,0)),
												FX_SHR64((
													FX_OR64(FX_CREATE64(0x00004000, 0x0), 
																FX_CREATE64(rand1, rand2))
												), 46 - dstdxy_bit_pos)));
							//debug
							//printf("fc_lod.c:  FX_OR64()=0x%08x_%08x\n", 
							//					FX_HI64( FX_OR64(FX_CREATE64(0x00004000, 0x0), 
							//									FX_CREATE64(rand1, rand2)) ),
							//					FX_LO64( FX_OR64(FX_CREATE64(0x00004000, 0x0), 
							//									FX_CREATE64(rand1, rand2)) ));
							//printf("fc_lod.c:  tri_vals[TRI_PARAM_DSTDXY]=%f\n", 
							//						tri_vals[TRI_PARAM_DSTDXY]);

							// random range (4.0, 2.0]
							tri_vals[TRI_PARAM_DWDXY] = 
										((float) (dstdxy_sign ? -1.0 : 1.0))
										* ((float) 2.0)
										* ((float) (0x8000 | iRandom(0x00007FFF))) 
										/ ((float) (0x8000));
						}

						// this causes extreme w's and exponents!
						if (zero_prod)
							tri_vals[TRI_PARAM_DWDXY] = (float) 0.0;


						// use w to bias into norm range (or at least closer)
						tri_vals[TRI_PARAM_W] = ((float) 1.0) / ((float) 
								pow(2.0, (((double) 4.0) - TRX_LOG2_DBL(
								ABS_VAL(tri_vals[TRI_PARAM_DWDXY] * 1.001
												- tri_vals[TRI_PARAM_DSTDXY])))));
						tri_vals[TRI_PARAM_W] = flt_clmp_w(tri_vals[TRI_PARAM_W]);

						// compensate s/t for w!=1.0
						tri_vals[TRI_PARAM_ST] = ((float) 1.001) * tri_vals[TRI_PARAM_W];
						tri_vals[TRI_PARAM_ST] = flt_clmp_16_32(tri_vals[TRI_PARAM_ST]);


						do_1pix_tri_1q(sst, &tri_cnt, tri_regs, tri_vals, t);
					}
				}
			}
		}

	}	// end of lod quarters



	// not in quarters:
	// swap, shift, add
	// max - do slightly different magnitudes
	// csa
	// clamp - make sure +,- extreme are used


	for (cnt = 0; cnt < 200; cnt++) {

		// zero the products and control the output of each 
		//    quarter with 4 der.
		SETF(sst->Fdwdx, (float) 0.0);
		SETF(sst->Fdwdy, (float) 0.0);
		SETF(sst->Fs, (float) 0.0);
		SETF(sst->Ft, (float) 0.0);


		dsdx = flt_clmp_16_32(rand_16_32());
		dtdx = flt_clmp_16_32(rand_16_32());
		dsdy = flt_clmp_16_32(rand_16_32());
		dtdy = flt_clmp_16_32(rand_16_32());

		SETF(sst->Fdsdx, dsdx);
		SETF(sst->Fdtdx, dtdx);
		SETF(sst->Fdsdy, dsdy);
		SETF(sst->Fdtdy, dtdy);

		lod_no_w = (float) (0.5 * TRX_LOG2_DBL(
				MAX(pow(dsdx, 2.0) + pow(dtdx, 2.0), 
					(pow(dsdy, 2.0) + pow(dtdy, 2.0)))));


		// set w to bring lod toward norm range - tests w range and mantissas
		//   LOD = 4.0 = log2(w) + lod_no_w
		//   2 ^ (4.0 - lod_no_w) = w
		SETF(sst->Fw, (float) flt_clmp_w(pow(2.0, (4.0 - lod_no_w))));


		x_coord = (tri_cnt) % 640;
		y_coord = (tri_cnt) / 640;

		SET(sst->vA.x, t->vA.x + x_coord * XY_ONE);
		SET(sst->vA.y, t->vA.y + y_coord * XY_ONE);
		SET(sst->vB.x, t->vB.x + x_coord * XY_ONE);
		SET(sst->vB.y, t->vB.y + y_coord * XY_ONE);
		SET(sst->vC.x, t->vC.x + x_coord * XY_ONE);
		SET(sst->vC.y, t->vC.y + y_coord * XY_ONE);


		SET(sst->triangleCMD, t->area);
		(tri_cnt)++;
	}




	// no loddither
	// tclampw
	// lod biases
	// w is neg to clamp lod
	// lodmin, lodmax values with various lod
	// lod zero frac
	// trilinear with trex_odd=0,1, lod odd,even
	// detail factor


	//Can't do // now use multi base addr
	//SET(sst->texBaseAddr, iRandom(BIT(19 - 1)));
	//SET(sst->texBaseAddr1, iRandom(BIT(19 - 1)));
	//SET(sst->texBaseAddr2, iRandom(BIT(19 - 1)));
	//SET(sst->texBaseAddr38, iRandom(BIT(19 - 1)));


	// zero the products and control LOD with der.
	SETF(sst->Fdwdx, (float) 0.0);
	SETF(sst->Fdwdy, (float) 0.0);

	SETF(sst->Fdtdx, (float) 0.0);
	SETF(sst->Fdsdy, (float) 0.0);
	SETF(sst->Fdtdy, (float) 0.0);



	for (cnt = 0; cnt < 400; cnt++) {

		// change minfilter to point
		// turn on tclampw
		// tc to c_local * det_fact
		test_textureMode = 0
					| SST_TPERSP_ST
					| SST_TMINFILTER
				//	| SST_TMAGFILTER
					| SST_TCLAMPW
					| (iRandom(0x1) ? SST_TLODDITHER : 0)	// exercises bias adder
				// | SST_TNCCSELECT
				// | SST_TCLAMPS
				// | SST_TCLAMPT

					| SST_RGB565    // SST_TFORMAT

			// c_local * det_fact
					| SST_TC_ZERO_OTHER
					| SST_TC_SUB_CLOCAL
					| SST_TC_MLOD        // SST_TC_MSELECT = detail factor
				// | SST_TC_REVERSE_BLEND
					| SST_TC_ADD_CLOCAL
				// | SST_TC_ADD_ALOCAL
				// | SST_TC_INVERT_OUTPUT

			// a_local * lod_frac
					| SST_TCA_ZERO_OTHER
					| SST_TCA_SUB_CLOCAL
					| SST_TCA_MLODFRAC       // SST_TCA_MSELECT
				//	| SST_TCA_REVERSE_BLEND
					| SST_TCA_ADD_CLOCAL
				// | SST_TCA_ADD_ALOCAL
				// | SST_TCA_INVERT_OUTPUT

				// | SST_TRILINEAR
				// | SST_SEQ_8_DOWNLD
				;
		SET(SST_TREX(sst,t->tex->trex)->textureMode, test_textureMode);


		SET(sst->tDetail, iRandom(BIT(17) - 1));


		sstj->tlod.min = iRandom(0x3f) % (8 << 2);
		sstj->tlod.max = iRandom(0x3f) % (8 << 2);
		if (sstj->tlod.min > sstj->tlod.max) {	// swap if out of order
			tmp_fxu32 = sstj->tlod.min;
			sstj->tlod.min = sstj->tlod.max;
			sstj->tlod.max = tmp_fxu32;
		}
		sstj->tlod.bias = iRandom(0x3f);
		sstj->tlod.odd = iRandom(0x1);
		sstj->tlod.tsplit = 0; // CAN'T DO iRandom(0x1);
		sstj->tlod.s_is_wider = iRandom(0x1);
		sstj->tlod.aspect = 0; // CAN'T DO iRandom(0x3);
		sstj->tlod.zerofrac = (iRandom(0x7) == 0) ? 1 : 0;	// 1/8 of the time
		sstj->tlod.tmultibaseaddr = 0; //CAN'T DO iRandom(0x1);
		sstj->tlod.tdata_swizzle = 0;
		sstj->tlod.tdata_swap = 0;
		sstj->tlod.tdirect_write = 0;

		SET_TLOD(sst, sstj);



		// s,t have no effect on LOD, use random for various texel values
		SETF(sst->Fs, (float) flt_clmp_16_32(rand_16_32()));
		SETF(sst->Ft, (float) flt_clmp_16_32(rand_16_32()));

		// rand w [1.0, 2.0) to multiply by s,t - picks up st mult coverage
		w_flt = ((float) (0x40000000 | iRandom(0x3fffffff))) 
				/ ((float) (0x40000000));
		SETF(sst->Fw, w_flt);


		if ((tri_cnt & 7) == 0) {
			// don't confine lod for 1/8 of the tris
			SETF(sst->Fdsdx, (float) flt_clmp_16_32(rand_16_32()));

		} else {
			// set dsdx (considering lodbias and w) to produce lod in range
			//    loddither will perturb slightly

			// select rand lod in range
			lod_flt = 4.0 * ((float) (iRandom(0x7fffffff))) 
							  / ((float) (0x40000000));

			//   lod_flt = log2(w_flt) + .5 * log2(dsdx)
			//   2 * (lod_flt - log2(w_flt)) = log2(dsdx)
			//   2 ^ (2 * (lod_flt - log2(w_flt)) = dsdx
			dsdx_flt = (float) (pow(2.0, (((double) lod_flt) - TRX_LOG2_DBL(w_flt))));
			SETF(sst->Fdsdx, dsdx_flt);
		}


		x_coord = (tri_cnt) % 640;
		y_coord = (tri_cnt) / 640;

		SET(sst->vA.x, t->vA.x + x_coord * XY_ONE);
		SET(sst->vA.y, t->vA.y + y_coord * XY_ONE);
		SET(sst->vB.x, t->vB.x + x_coord * XY_ONE);
		SET(sst->vB.y, t->vB.y + y_coord * XY_ONE);
		SET(sst->vC.x, t->vC.x + x_coord * XY_ONE);
		SET(sst->vC.y, t->vC.y + y_coord * XY_ONE);


		SET(sst->triangleCMD, t->area);
		(tri_cnt)++;
	}


	gdbg_info(2, "fc_lod.c:  Total triangles:  tri_cnt = %d\n", tri_cnt);


#endif
// ****************************************************************************************

	DIAG_PASS(0);

}


