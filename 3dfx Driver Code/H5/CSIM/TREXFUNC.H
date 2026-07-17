/* 
#**********************************************************************
# $Id: trexfunc.h,v 1.14 1996/03/23 03:34:45 jimm Exp $
# module_cmnt:  trex functions for 1/w, x^2 and log2(x), header file
#   these functions reside in:  trexfunc.c, trextst.c, trexfnc2.c
# 
# jm  tab=3
#**********************************************************************
#
# Copyright (c) 1997, 3Dfx Interactive, Inc.
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
# the contents of this file may not be disclosed to third parties, copied or
# duplicated in any form, in whole or in part, without the prior written
# permission of 3Dfx Interactive, Inc.
#
# RESTRICTED RIGHTS LEGEND:
# Use, duplication or disclosure by the Government is subject to restrictions
# as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
# and Computer Software clause at DFARS 252.227-7013, and/or in similar or
# successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
# rights reserved under the Copyright Laws of the United States.
#
#**********************************************************************
#
# $Revision: 3$
# $Date: 10/11/00 8:09:25 PM$
#
# $Log: 
#  3    3dfx      1.1.1.0     10/11/00 Brent           Forced check in to enforce
#       branching.
#  2    3dfx      1.1         09/21/99 Larry  warner   For future compilation
#       under MSVC6, make definitions of INT32 and UINT32 conditional at compile
#       time.
#  1    3dfx      1.0         09/12/99 StarTeam VTS Administrator 
# $
** 
** 6     4/21/99 2:12p Devil
** Add 3 bits of precision to (s/w) * (1/1/w) value for 2048x2048
** textures.
** This was added to be consistent with hardware
 * 
 * 4     11/25/97 5:20p Ken
 * removed size of array from extern declaration
 * 
 * 3     4/05/97 8:11a Tarolli
 * added texture mirroring
 * 
 * 2     2/28/97 2:25p Tarolli
 * changed bilinear blending from .4 to .8
 * NOTE: currently it's masked to .4
 * 
 * 1     2/25/97 8:27a Tarolli
 * 
 * 10    4/05/96 9:33a Tarolli
 * added 8-bit sequential download support
# Revision 1.14  1996/03/23 03:34:45  jimm
# mem2 addition
#
 * Revision 1.13  1995/09/06  03:56:42  jimm
 * added detail_factor
 *
 * Revision 1.12  1995/09/04  17:47:11  jimm
 * added more to tx_tst.c and incorporated trexfunc.c changes from Gary
 *
 * Revision 1.11  1995/09/01  19:03:57  jimm
 * new copy to gary
 *
 * Revision 1.10  1995/08/27  17:04:10  jimm
 * mc.v changes complete - outputs mem addr data ctl
 *
 * Revision 1.9  1995/08/25  08:22:27  jimm
 * added and fixed TREX register fields
 *
 * Revision 1.8  1995/08/07  22:00:04  jimm
 * changed debug code and fixed s/t_is_neg bug
 *
 * Revision 1.7  1995/08/02  05:24:15  jimm
 * incorporated changes from Brian
 *
 * Revision 1.6  1995/07/30  17:52:17  jimm
 * cleanup from Garys inputs adjusted lod error criteria
 *
 * Revision 1.5  1995/07/26  23:26:22  jimm
 * FX_ macroized and working the same as before
 *
 * Revision 1.4  1995/07/25  16:41:16  jimm
 * stlod tst working after files split
 *
#*/

#ifndef __TREXFUNC_H__
#define __TREXFUNC_H__


/************************************************************************/
/* general stuff                                                        */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>

/* #include "trex_64.h" */

#include "fx64.h"


#ifndef _BASETSD_H_
typedef signed long		INT32;
typedef unsigned long	UINT32;
#endif
typedef FxI64				INT64;
//typedef FxU64				UINT64;


#ifndef HAL_CSIM
	/* gcc on Sun */
	/* performance impact only */
	#define TRX_ERROR_CHECK 1

	#define TRX_PRINT_TABLES 0
	#define TRX_PRINT_STATS 0
	#define TRX_PRINT_VERILOG 0

#else
	/* PC, or CSIM on sparc */
	/* these must be zero because they mask non-PC code */
	#define TRX_ERROR_CHECK 0

	#define TRX_PRINT_TABLES 0
	#define TRX_PRINT_STATS 0
	#define TRX_PRINT_VERILOG 0

#endif



#if defined(__sparc__) && ! defined(__svr4__)   /* BSDish sunos only */
	#define TRX_LOG2_DBL(x)	(log2(x))
#else
	#define TRX_LOG2_DBL(x)	(log(x) / log(2.0))
#endif


/* moved into struct */
/* extern INT64 signMask; */

/* --- need to alias to fx64.h bitwise AND */
/* #define INT64_SIGN(x) ((x) & signMask) */
#define INT64_SIGN(x) (FX_AND64( (x) , st->signMask_i64))

#define TRX_SGNEXT32(a, n) ( (a) | -((a) & (1 << (n))))


#ifdef __unix__
	#ifndef HAL_CSIM
		// this is only used when debug stuff is exposed:
		#include "trex_64.h"

	#endif
#endif




/************************************************************************/
/* TRX_INV_   1/w                                                       */
/************************************************************************/


/* defines to control precisions used: */
/* lookup table input width */
#define TRX_INV_TABLE_INPUT_BITS 10

/* lookup table output width */
#define TRX_INV_LOOKUP_TBL_OUTPUT_BITS 20
#define TRX_INV_DO_LOOKUP_GEN_ROUND 1

/* amount of precision used for interpolation between successive */
/*     lookup entries (equal to number of LSBs of input sent to mult) */
#define TRX_INV_INTERPOLATION_BITS 9

/* shift the slope by N bits to reduce table and multiplier size */
#define TRX_INV_SLOPE_DROP_LSBS 0

/* round output off */
#define TRX_INV_OUTPUT_DROP_LSBS 0



/* the product is shifted right (and rounded before added to the lookup value */
#define TRX_INV_PROD_RIGHT_SHIFT (TRX_INV_INTERPOLATION_BITS - TRX_INV_SLOPE_DROP_LSBS)

#define TRX_INV_TOTAL_INPUT_BITS (1 + TRX_INV_TABLE_INPUT_BITS + TRX_INV_INTERPOLATION_BITS)
							/* this includes the implied leading 1 */
#define TRX_INV_TOTAL_OUTPUT_BITS (TRX_INV_LOOKUP_TBL_OUTPUT_BITS - TRX_INV_OUTPUT_DROP_LSBS)


/* this is calculated at table init time by observing output range: */
extern UINT32 trx_inv_slope_tbl_bits;

/* UINT32 trx_inv_table[(1 << TRX_INV_TABLE_INPUT_BITS) + 1]; */
extern UINT32 trx_inv_lookup[(1 << TRX_INV_TABLE_INPUT_BITS) + 1];
extern UINT32 trx_inv_slope[1 << TRX_INV_TABLE_INPUT_BITS];



/********** functions **********/
/* generate a verilog case statement from the table */

/* trexfunc.c */
void trx_inv_init_table();
UINT32 trx_inv_calc_w_inv(UINT32 inval);

/* trextst.c */
void trx_inv_gen_ver_case();
void trx_inv_check_w_inv();


/************************************************************************/
/* TRX_X2_   x^2                                                        */
/************************************************************************/

/* defines to control precisions used: */

/* input has implied leading one */
#define TRX_X2_INPUT_BITS 8
/* lookup table input width */
#define TRX_X2_TABLE_INPUT_BITS (TRX_X2_INPUT_BITS - 1)

/* number of bits to drop from product */
#define TRX_X2_DROP_OUTPUT_LSBS 8


/* lookup table output width */
extern UINT32 trx_x2_tbl_out_bits;


extern UINT32 trx_x2_table[(1 << TRX_X2_TABLE_INPUT_BITS) + 1];



/********** functions **********/
/* generate a verilog case statement from the table */
/* trextst.c */
void trx_x2_gen_ver_case();

/* trexfunc.c */
void trx_x2_init_table();



/************************************************************************/
/* TRX_LOG2_   log2(x)                                                  */
/************************************************************************/
/*
 * Input is treated as having range of [1,2)  mant=1.111...1
 * Output should have range of [0,1)  mant=.111...1
 */


/* defines to control precisions used: */

/* input has implied leading one */
#define INPUT_BITS 8
/* lookup table input width */
#define TABLE_INPUT_BITS (INPUT_BITS - 1)

/* number of bits to shift log2(x) before convert to int., controls output precision */
#define OUTPUT_SHIFT 8


/* lookup table output width */
extern UINT32	trx_log2_tbl_out_bits;


extern UINT32 trx_log2_table[];



/********** functions **********/
/* generate a verilog case statement from the table */
/* trextst.c */
void trx_log2_gen_ver_case();

/* trexfunc.c */
void trx_log2_init_table();



/************************************************************************/
/* TRX_LOD_                                                             */
/************************************************************************/
/*
 * calculate S, T, LOD from iterator and state values 
 */

/* data structure to hold inputs and outputs of trx_st(), trx_lod() */
typedef struct {

	/* For bug in one of the PC compilers */
	INT64 signMask_i64; /* must be init to = 0x800000000000; */
	INT64 signMask39_i64; /* must be init to = 0x4000000000; */


	/*** input:  ***/

	INT32 tt_input_data;			// 32 bits

	// texel attributes for type of access
	INT32 tex_yiq_write;
	INT32 is_tex_write;


	INT32 fbzcolorpath;

	// yiq write
	//    tnccselect is used set from reg write address for table select
	INT32 yiq_wr_addr;	// 4
	INT32 yiq_wr_data;	// 32

	// texture write
	INT32 seq_8_downld;
	INT32 tex_wr_addr;	// 22:2, {2 trex, 4 lod, 8 t, 7 s}  two empty lsb's
	INT32 tex_wr_data;	// 32


	// ** trexInit0 **
	INT32 rgn_mref_en;			// 1 bits
	INT32 rgn_mref_cnt;			// 9
	INT32 rgn_page_size;			// 2
	INT32 rgn_sec_ras_en;		// 1
	INT32 rgn_sec_ras_bit;		// 1
	INT32 rgn_mem_type;			// 3
	INT32 rgn_mem_data_size;	// 1
	INT32 rgn_do_extra_cas;		// 1
	INT32 rgn_mem2;		// 1

	// ** trexInit1 **
	INT32 rgn_sl_int_en;			// 1
	INT32 rgn_sl_int_slave;		// 1
	INT32 rgn_ft_fifo_sil;		// 5
	INT32 rgn_tt_fifo_sil;		// 5
	INT32 rgn_ft_stall_3w;		// 1
	INT32 rgn_tt_stall_3w;		// 1
	INT32 rgn_tf_ck_del_adj;	// 4
	INT32 rgn_reset_fifos;		// 1
	INT32 rgn_reset_graphics;	// 1


	/* register bits referenced for LODB,LODB_frac,LODBI,bilinear_en */
	/* which are a function of LOD.    */
	/* These aren't necessary for w,s,t,lod calc.  */
	/* textureMode reg */
	INT32 tpersp_st;
	INT32 tminfilter;
	INT32 tmagfilter;
	/* INT32 tprojected; 7/6/95 */ 
	INT32 tclampw;
	INT32 tloddither;
	INT32 tnccselect;
	INT32 tclamps;
	INT32 tclampt;
	INT32 tmirrors;
	INT32 tmirrort;
	INT32 tformat;

	/* add other tcombine stuff here */
	INT32 tc_ctl;
	INT32 tca_ctl;

	INT32 trilinear;

	/* tLOD reg */
	INT32 lodmin;
	INT32 lodmax;
	INT32 lodbias;
	INT32 lod_odd;
	INT32 lod_tsplit;
	INT32 lod_s_is_wider;
	INT32 lod_aspect;
	INT32 lod_zerofrac;
	INT32 tmultibaseaddr;
	INT32 tdata_swizzle;
	INT32 tdata_swap;
	INT32 tdirect_write;

	/* tdetail reg */
	INT32 detail_max;
	INT32 detail_bias;
	INT32 detail_scale;

	/* base addresses */
	INT32 texbaseaddr;
	INT32 texbaseaddr1;
	INT32 texbaseaddr2;
	INT32 texbaseaddr3_8;

	/* yiq table */
	INT32 yiq[24];


	/* registers, set on a triangle basis:   16.32 signed */
	INT64 reg_dsdx_i64;
	INT64 reg_dsdy_i64;
	INT64 reg_dtdx_i64;
	INT64 reg_dtdy_i64;
	INT64 reg_dwdx_i64;
	INT64 reg_dwdy_i64;

	/* iterator values, set on a per pixel basis:   16.32 signed */
	INT64 it_w_inv_i64;
	INT64 it_s_i64;
	INT64 it_t_i64;
	INT32 x, y;	/* not necessary for w,s,t,lod calc.  */





	/*** hw intermediate and output values:  ***/
	/* integers should be bit accurate */
	UINT32 hw_w_is_neg;
	INT64 hw_w_inv_clmp_48_i64;	/* w_inv clamped positive */

	UINT32 hw_w_exp_6;			/* w exp, as in spec. */
	UINT32 hw_w_mant_19;			/* w float mantissa, leading 1 */

	/* pure s,t */
	INT64 hw_s_mant_39s_i64;
	INT32 hw_s_exp_7;
	INT64 hw_t_mant_39s_i64;
	INT32 hw_t_exp_7;

	/* s,t to addr gen. module */
	INT32 hw_s_fxd_is_neg;
	INT32 hw_s_fxd_clmp_pos;
	INT64 hw_s_fxd_12_i64;
	INT32 hw_t_fxd_is_neg;
	INT32 hw_t_fxd_clmp_pos;
	INT64 hw_t_fxd_12_i64;

	/* s,t to LOD calc. module */
	INT64 hw_s_lod_mant_12s_i64;
	INT32 hw_s_lod_exp_7;
	INT64 hw_t_lod_mant_12s_i64;
	INT32 hw_t_lod_exp_7;

	/* GMT: hack to save away U,V .8 fractions */
	INT32 hw_s_frac_8;
	INT32 hw_t_frac_8;

        /* Devil: Similar to Gary's above hack */
        INT32 hw_s_frac_11;
        INT32 hw_t_frac_11;
	
	/* trx_lod() */
	INT32 sq_xs_mant_8;
	UINT32 sq_xs_exp_8;
	INT32 sq_xt_mant_8;
	UINT32 sq_xt_exp_8;
	INT32 sq_ys_mant_8;
	UINT32 sq_ys_exp_8;
	INT32 sq_yt_mant_8;
	UINT32 sq_yt_exp_8;

   UINT32 hw_max_mant_8;
	UINT32 hw_max_exp_9;

	INT32 hw_log_max_d2_8_9;	/* log of the max, divided by 2, */
			/* 8.9 positive */
	INT32 hw_log_w_6_9;			/* log of w, 6.9 positive */

	INT32 hw_lod_8_9s;			/* lod before trunc, clamp 8.9 signed */
	INT32 hw_lod_7_8s;			/* lod  7.8 signed [-64,64] */

	INT32 hw_lodb_4_8;
	INT32 hw_lodb0;
	INT32 hw_bilinear_en;
	INT32 hw_lodb_frac_8;
	INT32 hw_lodbi_4;

	INT32 hw_detail_factor_8;


	/* trx_addr_gen() */
	INT32 hw_s_frac_4;
	INT32 hw_s_sh_8_4;
	INT32 hw_s_dec_8_4;
	INT32 hw_s_inc_8_4;
	INT32 hw_s0_8;
	INT32 hw_s1_8;
	INT32 hw_wt_s1_1_4;	/* weight for s1, [0,16] */

	INT32 hw_t_frac_4;
	INT32 hw_t_sh_8_4;
	INT32 hw_t_dec_8_4;
	INT32 hw_t_inc_8_4;
	INT32 hw_t0_8;
	INT32 hw_t1_8;
	INT32 hw_wt_t1_1_4;	/* weight for t1, [0,16] */

	INT32 offs_00_15;
	INT32 offs_01_15;
	INT32 offs_10_15;
	INT32 offs_11_15;

	INT32 mem_byte_sel00;
	INT32 mem_byte_sel01;
	INT32 mem_byte_sel10;
	INT32 mem_byte_sel11;

	INT32 base_addr_19;

	INT32 mem_addr_00_19;
	INT32 mem_addr_01_19;
	INT32 mem_addr_10_19;
	INT32 mem_addr_11_19;


	INT32 mem_ras0;	// = !ras1
	INT32 mem_cas0;
	INT32 mem_cas1;
	INT32 mem_cas2;
	INT32 mem_cas3;

	INT32 mem_row_00_9;
	INT32 mem_row_01_9;
	INT32 mem_row_10_9;
	INT32 mem_row_11_9;

	INT32 mem_col_00_int_9;
	INT32 mem_col_01_int_9;
	INT32 mem_col_10_int_9;
	INT32 mem_col_11_int_9;

	INT32 mem_col_00_9;
	INT32 mem_col_01_9;
	INT32 mem_col_10_9;
	INT32 mem_col_11_9;


	INT32 wr_data_int;	// internal 32 bit

	INT32 mem_wr_data_00_16;
	INT32 mem_wr_data_01_16;
	INT32 mem_wr_data_10_16;
	INT32 mem_wr_data_11_16;


	INT32 tf_argb_32;



	/*** double calculation output:  ***/
	/* this is in the struct so that intermediate values can be used */
	/*    for the check */
	double dbl_dsdx;
	double dbl_dsdy;
	double dbl_dtdx;
	double dbl_dtdy;
	double dbl_dwdx;
	double dbl_dwdy;

	double dbl_w;
	double dbl_s;
	double dbl_t;

	double dbl_xsq1;
	double dbl_xsq2;
	double dbl_xsum;

	double dbl_ysq1;
	double dbl_ysq2;
	double dbl_ysum;

	double dbl_max;
	double dbl_log_max;
	double dbl_log_w;

	double dbl_lod;
	double dbl_lodb;

	double dbl_bilinear_en;

	double dbl_lodb_frac;
	INT32 dbl_lodb_lsb;
	double dbl_lodbi;


	/* convert values from hardware for the checks */
	double hw_w_dbl;
	double hw_s_dbl;
	double hw_t_dbl;
	double hw_max_dbl;
	double hw_log_max_d2_dbl;
	double hw_log_w_dbl;
	double hw_lod_dbl;

	/* errors */
	double max_st_err_dbl;  /* maximum s,t error allowed */
	double w_err_dbl;
	double s_err_dbl;
	double t_err_dbl;
	double lod_err_dbl;

	INT32 check_passed;	/* result of check, set by trx_chk_stlod() */

} TRX_STLOD_STRUCT;



/********** functions **********/
/* trexfunc.c */
void trx_st(TRX_STLOD_STRUCT* st);			/* hardware bit accurate s,t calc. */
void trx_lod(TRX_STLOD_STRUCT* st);			/* hardware bit accurate lod calc. */

/* trextst.c */
void trx_chk_stlod(TRX_STLOD_STRUCT* st);	/* check stlod, calculate in double */
void trx_pr_stlod(TRX_STLOD_STRUCT* st);	/* print stlod */

/* trexfunc.c */
void trx_error(TRX_STLOD_STRUCT* st);

void trx_detail_factor(TRX_STLOD_STRUCT* st);

/* trexfnc2.c */
void trx_addr_gen(TRX_STLOD_STRUCT* st);


#endif /* __TREXFUNC_H__ */

