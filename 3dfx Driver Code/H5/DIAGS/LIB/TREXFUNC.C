#include "vxd.h"

/* 
#**********************************************************************
# $Id: trexfunc.c,v 1.41 1996/05/30 03:26:56 jimm Exp $
# module_cmnt:  trex functions for 1/w, x^2 and log2(x), s,t,lod, code for the PC
# 
# (note the 8.3 filename, so that more limited OS's can use this file ;) )
# 
# This isn't optimized software, it's a hardware model, bit accurate. 
# (money-back guarantee).  If you need this to be modified, please please 
# ask me to do it.  I will update this file with hardware changes 
# periodically.  
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
# $Revision: 2$
# $Date: 10/11/00 8:12:07 PM$
#
# $Log: 
#  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
#       branching.
#  1    3dfx      1.0         09/12/99 StarTeam VTS Administrator 
# $
** 
** 9     4/21/99 2:12p Devil
** Add 3 bits of precision to (s/w) * (1/1/w) value for 2048x2048
** textures.
** This was added to be consistent with hardware
 * 
 * 6     11/25/97 5:24p Ken
 * added NO_FLOAT compile time trex tables, and provision for dumping them
 * from the non-NO_FLOAT case
 * 
 * 5     6/16/97 10:14p Tarolli
 * fixed shifting bug
 * 
 * 4     4/05/97 8:53a Tarolli
 * changed texture mirroring with clamp
 * 
 * 3     3/20/97 10:30p Ken
 * csim in a vxd
 * 
 * 2     2/28/97 2:25p Tarolli
 * changed bilinear blending from .4 to .8
 * NOTE: currently it's masked to .4
 * 
 * 1     2/25/97 8:33a Tarolli
 * 
 * 17    5/29/96 8:45p Jimm
 * 
 * print info for debug which is ifdef'd out
# Revision 1.41  1996/05/30 03:26:56  jimm
# input val printing for fc_lod test debug
#
# Revision 1.40  1996/04/08 04:01:40  jimm
# added code for backdoor fast 8-bit writes
#
 * Revision 1.39  1995/10/30  05:03:16  jimm
 * fixed w neg detail factor priority
 *
 * Revision 1.38  1995/10/30  01:31:23  jimm
 * clamp detf to max if w is negative
 *
 * Revision 1.37  1995/10/27  16:22:55  jimm
 * change st clamp to match hw
 *
 * Revision 1.36  1995/10/26  21:24:52  jimm
 * fixed >64 shift
 *
 * Revision 1.35  1995/09/29  22:31:21  jimm
 * fix tclampw for t as was done for s
 *
 * Revision 1.34  1995/09/23  15:44:43  jimm
 * mask off carry out of bias_minus_lod_8_7s - 50 of 50 up
 *
 * Revision 1.33  1995/09/17  12:39:28  jimm
 * fixed diags 30 of 30
 *
 * Revision 1.32  1995/09/15  17:35:42  jimm
 * added debug info
 *
 * Revision 1.31  1995/09/14  15:24:28  jimm
 * simple test lod and detail ok
 *
 * Revision 1.30  1995/09/12  16:24:11  jimm
 * fix for kernel
 *
 * Revision 1.29  1995/09/12  09:45:17  jimm
 * works with pipelog mech.
 *
 * Revision 1.28  1995/09/09  04:51:22  jimm
 * kernel ifndefs and some warning fixes
 *
 * Revision 1.27  1995/09/06  03:54:52  jimm
 * added detail_factor and fixed s t swap
 *
 * Revision 1.26  1995/09/04  17:47:08  jimm
 * added more to tx_tst.c and incorporated trexfunc.c changes from Gary
 *
 * Revision 1.25  1995/09/04  00:22:46  jimm
 * before changing res_exp_9 to res_exp_8
 *
 * Revision 1.24  1995/09/03  23:25:44  jimm
 * before s t exponent polarity change
 *
 * Revision 1.23  1995/08/25  08:22:25  jimm
 * added and fixed TREX register fields
 *
 * Revision 1.22  1995/08/08  00:33:49  jimm
 * changed lodb to lod for final GSIM segment
 * first time for Brian to run flames, wasp, pyramid
 *
 * Revision 1.21  1995/08/07  22:00:03  jimm
 * changed debug code and fixed s/t_is_neg bug
 *
 * Revision 1.20  1995/08/05  02:41:26  jimm
 * lod clamp adjustments
 *
 * Revision 1.19  1995/08/03  06:31:57  jimm
 * mul32 again
 *
 * Revision 1.18  1995/08/03  05:55:45  jimm
 * mul64->mul32
 *
 * Revision 1.17  1995/08/03  02:46:46  jimm
 * fixed ifdef TRX_ERROR_CHECK
 *
 * Revision 1.16  1995/08/02  05:24:13  jimm
 * incorporated changes from Brian
 *
 * Revision 1.15  1995/07/30  17:52:16  jimm
 * cleanup from Garys inputs adjusted lod error criteria
 *
# Revision 1.14  1995/07/27  18:33:38  jimm
# ifndef KERNEL to mask printf(), exit()
#
# Revision 1.13  1995/07/26  23:26:20  jimm
# FX_ macroized and working the same as before
#
# Revision 1.12  1995/07/25  16:41:17  jimm
# stlod tst working after files split
#
# Revision 1.11  1995/07/23  23:47:47  jimm
# changed trexfunc.c to use fx64.h not trex_64.h
#
# Revision 1.10  1995/07/21  13:03:37  jimm
# header+
#
*/


/*
 * Contains these functions, protos are in trexfunc.h:
 *  void trx_error(TRX_STLOD_STRUCT* st)
 *  void trx_inv_init_table()
 *  UINT32 trx_inv_calc_w_inv(UINT32 inval)
 *  void trx_x2_init_table()
 *  void trx_log2_init_table()
 *  void trx_st(TRX_STLOD_STRUCT* st)
 *  void lod_calc_sq_term(TRX_CALC_SQ_TERM_STRUCT* sq)
 *  void trx_lod(TRX_STLOD_STRUCT* st)
 */

/*
 * A few notes.  We now use #ifdef __sparc__ instead of sparc or SUN.  I liberally chopped 
 * out a lot of code, probably too much (like Verilog print outs).  If you are ambitious, 
 * separate this code out into separate routines and ifdef out a call to them.  That keeps 
 * the core logic code to a minimum in one file.  I would like to keep trexfunc.c to just 
 * the routines that I have in my version of it.  So if you wouldn't mind creating another
 * file to hold the routines I deleted, that would be super.
 * 
 * P.S. fx64.h can be found in  //archer/e/tdfx/engr/devel/swlibs/src/lib/3dfx
 */


#include "trexfunc.h"


#define TRX_PRINT_INPUT_VALS 0
#define TRX_SQ_DEBUG 0
#define TRX_DEBUG_PRINT_DETAIL 0

/* moved into st struct */
/* INT64 signMask = 0x800000000000; */
/* INT64 signMask39 = 0x4000000000; */


// decalarations for tables
UINT32 trx_x2_tbl_out_bits;
UINT32 trx_x2_table[(1 << TRX_X2_TABLE_INPUT_BITS) + 1];
UINT32 trx_inv_slope_tbl_bits;
UINT32 trx_log2_tbl_out_bits;

#ifndef NO_FLOAT
UINT32 trx_inv_lookup[(1 << TRX_INV_TABLE_INPUT_BITS) + 1];
UINT32 trx_inv_slope[1 << TRX_INV_TABLE_INPUT_BITS];
UINT32 trx_log2_table[(1 << TABLE_INPUT_BITS) + 1];
#else
#include "trxtabls.h"
#endif /* #ifndef NO_FLOAT */




/* data structure to hold inputs and outputs of lod_calc_sq_term() */
/*    this is put here so that it's scope is only in this file     */
typedef struct {
   /*** input:  ***/
	INT64 dw_mant_12s_i64;
	UINT32 dw_exp_6;

	INT64 st_mant_12s_i64;
	UINT32 st_exp_7;

	INT64 dst_mant_24s_i64;
	UINT32 dst_exp_6;


   /*** output:  ***/
	UINT32 res_mant_8;
	UINT32 res_exp_8;

} TRX_CALC_SQ_TERM_STRUCT;




/***************************************************************************/
/* trex error:  central place to handle error cases                        */
void trx_error(TRX_STLOD_STRUCT* st)
{

FXUNUSED( st );  // to avoid warnings

#if TRX_ERROR_CHECK
	if (st != NULL) {
   	trx_chk_stlod(st);
		trx_pr_stlod(st);
	}
#endif

#ifndef KERNEL
	printf("Internal fatal error, file %s, line: %d", __FILE__, __LINE__);
	exit(1);
#endif
}


/************************************************************************/
/* TRX_INV_   1/w                                                       */
/************************************************************************/


/***************************************************************************/
void trx_inv_init_table()
{
#ifndef NO_FLOAT	// NO_FLOAT_work
    /* NO_FLOAT
     * this requires work.  replace run-time generated UINT32 tables
     * with compile-time tables for trx_inv_lookup[], and trx_inv_slope[]

     */
     
	UINT32 i;
	double a;

	UINT32 lookup_min = 0x100000;
	UINT32 lookup_max = 0;
	UINT32 slope_min = 0x100000;
	UINT32 slope_max = 0;

	UINT32 slope;


	/* check type sizes */
#ifndef KERNEL
	if (sizeof(UINT32) != 4) {
		printf("wrong UINT32 size\n");
		exit(1);
	}
//	if (sizeof(UINT64) != 8) {
//		printf("wrong UINT64 size\n");
//		exit(1);
//	}
	if (sizeof(INT64) != 8) {
		printf("wrong INT64 size\n");
		exit(1);
	}
#endif


	/* this fills one more lookup entry than the actual hardware table so that it */
	/*   can be used to calculate the last slope */
	for(i = 0; i <= (1 << TRX_INV_TABLE_INPUT_BITS); i++) {
		/* W is defined as follows: 1.i [where 0 <= i < 2^TRX_INV_TABLE_INPUT_BITS] */
		/* 1 <= W <= 2**16 */
		/* W is a normalized floating point number (leading one) */

		a = (double) i / (double) ((1 << TRX_INV_TABLE_INPUT_BITS));
		a += (double) 1.0;
		a = (double) 1.0 / a;			/* .5 < a <= 1.0 */
#ifndef KERNEL
		if (a < (double) .5 || a > (double) 1.0) {
			printf("Error: Unexpected table value!\n");
			exit(1);
		}
#endif

		/* Convert into integer form with proper bits of precision */
		a *= (double) (1 << (TRX_INV_LOOKUP_TBL_OUTPUT_BITS - 1));

		/* Round */
		if (TRX_INV_DO_LOOKUP_GEN_ROUND)
			a += (double) 0.5;

		trx_inv_lookup[i] = (UINT32) a;

		/* check size of table entry */
#ifndef KERNEL
		if (trx_inv_lookup[i] >= (1 << TRX_INV_LOOKUP_TBL_OUTPUT_BITS)) {
			printf("entry too large Table[%ld] = 0x%lx\n", i, trx_inv_lookup[i]);
			exit(1);
		}
#endif

		/* record range of table entries */
		if (lookup_min > trx_inv_lookup[i])
			lookup_min = trx_inv_lookup[i];
		if (lookup_max < trx_inv_lookup[i])
			lookup_max = trx_inv_lookup[i];
	}

	/* derive the slopes between the lookup values */
	/*  --- pos slopes for now */
	for(i = 0; i <= ((1 << TRX_INV_TABLE_INPUT_BITS) - 1); i++) {

		slope = trx_inv_lookup[i] - trx_inv_lookup[i + 1];

		trx_inv_slope[i] = (slope >> TRX_INV_SLOPE_DROP_LSBS)
#if TRX_INV_SLOPE_DROP_LSBS == 0
				/* no bits truncated, so don't round */
				;
#else
				/* round when shifting */
				+ ((slope >> (TRX_INV_SLOPE_DROP_LSBS - 1)) & 1);
#endif

		/* record range of table entries */
		if (slope_min > trx_inv_slope[i])
			slope_min = trx_inv_slope[i];
		if (slope_max < trx_inv_slope[i])
			slope_max = trx_inv_slope[i];
	}

	/* clear the last entry in the lookup table to ensure that it isn't */
	/*     used later */
	trx_inv_lookup[1 << TRX_INV_TABLE_INPUT_BITS] = 0xffffffff;

	/* calculate slope table bits */
	trx_inv_slope_tbl_bits = 0;
	i = slope_max;
	while (i != 0) {
		trx_inv_slope_tbl_bits++;
		i >>= 1;
	}

#undef TRX_PRINT_TABLES2
#ifdef TRX_PRINT_TABLES2
	/* for the NO_FLOAT case, generate these tables using a non-
	 * NO_FLOAT DLL, and then include this as a pre-initialized array
	 */
	printf("FxU32 trx_inv_lookup[] = \n");
	printf("{\n");
	for (i = 0; i <= (1 << TRX_INV_TABLE_INPUT_BITS); i++)
	    printf("0x%08x,\n", trx_inv_lookup[i]);
	printf("}\n");

	printf("FxU32 trx_inv_slope[] = \n");
	printf("{\n");
	for (i = 0; i <= ((1 << TRX_INV_TABLE_INPUT_BITS) - 1); i++)
	    printf("0x%08x,\n", trx_inv_slope[i]);
	printf("}\n");
	
#endif /* #ifdef TRX_PRINT_TABLES2 */


#if TRX_PRINT_TABLES
#if 1
	/* print table */
	printf("i    lookup   slope\n");
	for(i = 0; i <= ((1 << TRX_INV_TABLE_INPUT_BITS) - 1); i++)
		printf("%03x  %08x %08x\n", i, trx_inv_lookup[i], trx_inv_slope[i]);
	printf("\n");
#else
	/* print table */
	printf("i    lookup   slope      adder    error\n");
	for(i = 0; i <= ((1 << TRX_INV_TABLE_INPUT_BITS) - 1); i++)
		printf("%03x  %08x %08x   %08x %08x\n", i, trx_inv_lookup[i], trx_inv_slope[i],
				(0x80000 - ((INT32) i << 8)), ((0x80000 - (i << 8)) - trx_inv_lookup[i]));
	printf("\n");
#endif
#endif


#if TRX_PRINT_STATS
	/* print table stats, min/max of lookup, slope */
	printf("/*\n");
	printf("TRX_INV_TOTAL_INPUT_BITS:       %d  (includes implied 1)\n", TRX_INV_TOTAL_INPUT_BITS);
	printf("\n");
	printf("TRX_INV_TABLE_INPUT_BITS:       %d\n", TRX_INV_TABLE_INPUT_BITS);
	printf("TRX_INV_LOOKUP_TBL_OUTPUT_BITS: %d\n", TRX_INV_LOOKUP_TBL_OUTPUT_BITS);
	printf("TRX_INV_DO_LOOKUP_GEN_ROUND:    %d\n", TRX_INV_DO_LOOKUP_GEN_ROUND);
	printf("TRX_INV_INTERPOLATION_BITS:     %d\n", TRX_INV_INTERPOLATION_BITS);
	printf("TRX_INV_SLOPE_DROP_LSBS:        %d\n", TRX_INV_SLOPE_DROP_LSBS);
	printf("trx_inv_slope_tbl_bits:         %d\n", trx_inv_slope_tbl_bits);
	printf("\n");
	printf("lookup range:  %x-%x\n", lookup_min, lookup_max);
	printf("slope range:   %x-%x\n", slope_min, slope_max);
	printf("\n");
	printf("TRX_INV_PROD_RIGHT_SHIFT:       %d\n", TRX_INV_PROD_RIGHT_SHIFT);
	printf("TRX_INV_OUTPUT_DROP_LSBS:       %d\n", TRX_INV_OUTPUT_DROP_LSBS);
	printf("TRX_INV_TOTAL_OUTPUT_BITS:      %d  (max. of 100...00)\n", TRX_INV_TOTAL_OUTPUT_BITS);
	printf("\n");
#endif

#if TRX_PRINT_VERILOG
	trx_inv_gen_ver_case();
#endif
#endif /* #ifndef NO_FLOAT */
}



/***************************************************************************/
UINT32 trx_inv_calc_w_inv(UINT32 inval)
{
	UINT32 tbl_index;
	UINT32 interp_factor;
	UINT32 lookup;
	UINT32 slope;
	UINT32 prod;
	UINT32 result;


#if TRX_ERROR_CHECK
	/* check range of input */
	if (inval & ~((1 << (TRX_INV_TOTAL_INPUT_BITS - 1)) - 1)) {
		printf("trx_inv_calc_w_inv:  input beyond range: %x\n", inval);
		exit(1);
	}
#endif

	/* printf("trx_inv_calc_w_inv():  inval = %x\n", inval); */

	tbl_index = inval >> TRX_INV_INTERPOLATION_BITS;
	interp_factor = inval & ((1 << TRX_INV_INTERPOLATION_BITS) - 1);

	lookup = trx_inv_lookup[tbl_index];
	slope = trx_inv_slope[tbl_index];

	prod = interp_factor * slope;
	/* shift and round the product */
	prod = (prod >> TRX_INV_PROD_RIGHT_SHIFT)
			+ (((TRX_INV_PROD_RIGHT_SHIFT != 0) 
			&& ((prod >> (TRX_INV_PROD_RIGHT_SHIFT - 1)) & 1)) ? 1 : 0);
	/* '? 1 : 0' is in case a && b != {0,1} */

	result = lookup - prod;

#if TRX_DEBUG_PRINT_DETAIL
	printf("trx_inv_calc_w_inv():  slope = %x\n", slope);
	printf("trx_inv_calc_w_inv():  lookup = %x\n", lookup);
	printf("trx_inv_calc_w_inv():  prod = %x\n", prod);

	printf("trx_inv_calc_w_inv():  result = %x\n", result);
#endif
	return (result);
}


/************************************************************************/
/* TRX_X2_   x^2                                                        */
/************************************************************************/

/***************************************************************************/
void trx_x2_init_table()
{
	UINT32 i;

	UINT32 lookup_min = 0x100000;
	UINT32 lookup_max = 0;



	/* this fills one more lookup entry than the actual hardware table so that it */
	/*   can be used to calculate the last slope */
	for(i = 0; i <= ((1 << TRX_X2_TABLE_INPUT_BITS) - 1); i++) {
 
		/* adds 1/2 for round before truncate */
		trx_x2_table[i] = (((1 << TRX_X2_TABLE_INPUT_BITS) | i) * ((1 << TRX_X2_TABLE_INPUT_BITS) | i) 
								+ (1 << (TRX_X2_DROP_OUTPUT_LSBS - 1))) 
					>> TRX_X2_DROP_OUTPUT_LSBS;

		/* record range of table entries */
		if (lookup_min > trx_x2_table[i])
			lookup_min = trx_x2_table[i];
		if (lookup_max < trx_x2_table[i])
			lookup_max = trx_x2_table[i];
	}

	/* calculate table bits */
	trx_x2_tbl_out_bits = 0;
	i = lookup_max;
	while (i != 0) {
		trx_x2_tbl_out_bits++;
		i >>= 1;
	}


#if TRX_PRINT_TABLES
	/* print table */
	printf("/*\n");
	printf("i    lookup\n");
	for(i = 0; i <= ((1 << TRX_X2_TABLE_INPUT_BITS) - 1); i++)
		printf("%03x  %04x\n", i, trx_x2_table[i]);
	printf("\n");
	printf("*/\n");
#endif

#if TRX_PRINT_STATS
	/* print table stats, min/max of lookup, slope */
	printf("/*\n");
	printf("TRX_X2_INPUT_BITS:             %d  (includes implied 1)\n", TRX_X2_INPUT_BITS);
	printf("TRX_X2_TABLE_INPUT_BITS:       %d\n", TRX_X2_TABLE_INPUT_BITS);
	printf("TRX_X2_DROP_OUTPUT_LSBS:       %d\n", TRX_X2_DROP_OUTPUT_LSBS);
	printf("\n");
	printf("trx_x2_tbl_out_bits:           %d\n", trx_x2_tbl_out_bits);
	printf("lookup range:  %x-%x\n", lookup_min, lookup_max);
	printf("f(%x) = %x\n", 1 << TRX_X2_TABLE_INPUT_BITS, trx_x2_table[0]);
	printf("f(%x) = %x\n", (1 << (TRX_X2_TABLE_INPUT_BITS + 1)) - 1, 
						trx_x2_table[(1 << TRX_X2_TABLE_INPUT_BITS) - 1]);
	printf("*/\n");
#endif

#if TRX_PRINT_VERILOG
	trx_x2_gen_ver_case();
#endif
}




/************************************************************************/
/* TRX_LOG2_   log2(x)                                                  */
/************************************************************************/
/*
 * Input is treated as having range of [1,2)  mant=1.111...1
 * Output should have range of [0,1)  mant=.111...1
 */

/***************************************************************************/
void trx_log2_init_table()
{
#ifndef NO_FLOAT	// NO_FLOAT_work 
    /* NO_FLOAT
     * this requires work.  replace run-time generated UINT32 table
     * with compile-time tables for trx_log2_table[]    
     */

	UINT32 i;
	double log_double, lookup_table_double;

	UINT32 lookup_min = 0x100000;
	UINT32 lookup_max = 0;


	/* this fills one more lookup entry than the actual hardware table so that it */
	/*   can be used to calculate the last slope */
	for(i = 0; i <= ((1 << TABLE_INPUT_BITS) - 1); i++) {

		log_double = TRX_LOG2_DBL((double) ((1 << TABLE_INPUT_BITS) | i) 
										/ (double) (1 << TABLE_INPUT_BITS));
 
		lookup_table_double = (double) (1 << OUTPUT_SHIFT) * log_double 
						+ (double) 0.5;		/* adds 1/2 for round before truncate */

		if ((lookup_table_double < (double) 0.0) 
				|| (lookup_table_double > 
							(double) 1.0 * (double) (1 << OUTPUT_SHIFT) + (double) 0.5)) {
#ifndef KERNEL
			printf("Error: Unexpected table value!\n");
			exit(1);
#endif
		}

		trx_log2_table[i] = (UINT32) lookup_table_double;

		/* record range of table entries */
		if (lookup_min > trx_log2_table[i])
			lookup_min = trx_log2_table[i];
		if (lookup_max < trx_log2_table[i])
			lookup_max = trx_log2_table[i];
	}

	/* calculate table bits */
	trx_log2_tbl_out_bits = 0;
	i = lookup_max;
	while (i != 0) {
		trx_log2_tbl_out_bits++;
		i >>= 1;
	}

#ifdef TRX_PRINT_TABLES2
	printf("FxU32 trx_log2_table[] = \n");
	printf("{\n");
	for (i = 0; i <= (1 << TRX_INV_TABLE_INPUT_BITS); i++)
	    printf("0x%08x,\n", trx_log2_table[i]);	
	printf("}\n");
#endif /* #ifdef TRX_PRINT_TABLES2 */
	

#if TRX_PRINT_TABLES
	printf("/*\n");
	/* print table */
	printf("i  inp_double  log_double  lookup\n");
	for(i = 0; i <= ((1 << TABLE_INPUT_BITS) - 1); i++)
		printf("%03x  %f   %f  %04x\n", 
					i, (double) ((1 << TABLE_INPUT_BITS) | i) 
							/ (double) (1 << TABLE_INPUT_BITS), 
					TRX_LOG2_DBL((double) ((1 << TABLE_INPUT_BITS) | i) 
							/ (double) (1 << TABLE_INPUT_BITS)), 
					trx_log2_table[i]);
	printf("*/\n");
	printf("\n");
#endif

#if TRX_PRINT_STATS
	/* print table stats, min/max of lookup, slope */
	printf("/*\n");
	printf("INPUT_BITS:             %d  (includes implied 1)\n", INPUT_BITS);
	printf("TABLE_INPUT_BITS:       %d\n", TABLE_INPUT_BITS);
	printf("OUTPUT_SHIFT:           %d\n", OUTPUT_SHIFT);
	printf("\n");
	printf("trx_log2_tbl_out_bits:           %d\n", trx_log2_tbl_out_bits);
	printf("lookup range:  %x-%x\n", lookup_min, lookup_max);
	printf("f(%x) = %x\n", 1 << TABLE_INPUT_BITS, trx_log2_table[0]);
	printf("f(%x) = %x\n", (1 << (TABLE_INPUT_BITS + 1)) - 1,
						trx_log2_table[(1 << TABLE_INPUT_BITS) - 1]);
	printf("*/\n");
#endif

#if TRX_PRINT_VERILOG
	trx_log2_gen_ver_case();
#endif
#endif /* #ifndef NO_FLOAT */
}




/************************************************************************/
/* TRX_ST_                                                             */
/************************************************************************/
/*
 * calculate W, S, T from iterator and state values 
 */

/***************************************************************************/
/* hardware bit accurate s,t calc. */
void trx_st(TRX_STLOD_STRUCT* st)
{
	INT64 w_inv_mant_19_i64;	/* w_inv float mantissa, implied leading 1 */

	// xxx UINT64 it_s_sgn_64, it_t_sgn_64;
	INT32 it_s_sgn_hi, it_t_sgn_hi;
	INT64 it_s_mant_20s_i64, it_t_mant_20s_i64;
	UINT32 it_s_exp_5, it_t_exp_5;

	INT32 hw_s_fxd_exp_7, hw_t_fxd_exp_7;

	UINT32 hw_s_lod_sgn_hi, hw_t_lod_sgn_hi;
	INT32 hw_s_norm_adj, hw_t_norm_adj;


	/**** hardware - bit accurate ****/
	/* 1/w w_inv has range [-4k,60k) */
	if (st->tpersp_st) {
		/* clamp if negative or 0  [-4k,0]  */
		// xxx if (((UINT64) st->it_w_inv_i64) >= (((UINT64)0xf000) << 32)) {
		if ((FX_HI64(st->it_w_inv_i64) & 0xf000) == 0xf000) {
			/* printf("clamping w because it is [-4k,0)\n"); */
			// xxx st->hw_w_inv_clmp_48_i64 = (UINT64)0x1;
			FX_SET64(st->hw_w_inv_clmp_48_i64, 0, 0x1);
			st->hw_w_is_neg = 1;
		// xxx } else if (((UINT64) st->it_w_inv_i64) == (UINT64)0) {
		} else if (FX_EQ064(st->it_w_inv_i64)) {
			/* printf("clamping w because it is 0\n"); */
			// xxx st->hw_w_inv_clmp_48_i64 = (UINT64)0x1;
			FX_SET64(st->hw_w_inv_clmp_48_i64, 0, 0x1);
			st->hw_w_is_neg = 0;
		} else {
			st->hw_w_inv_clmp_48_i64 = st->it_w_inv_i64;	/* [0,60k) */
			st->hw_w_is_neg = 0;
		} 

		/* floating point w_inv */
		w_inv_mant_19_i64 = st->hw_w_inv_clmp_48_i64;
		st->hw_w_exp_6 = 0;
		// xxx while (!INT64_SIGN(w_inv_mant_19_i64)) {
		while ((FX_HI64(w_inv_mant_19_i64) & (1 << (47 - 32))) == 0) {
			// xxx w_inv_mant_19_i64 <<= 1;
			w_inv_mant_19_i64 = FX_SHL64(w_inv_mant_19_i64, 1);
			st->hw_w_exp_6++;
		}

		/* right adjust and trim leading 1 */
		// xxx w_inv_mant_19_i64 = (w_inv_mant_19_i64 >> 28) & (UINT64)0x7ffff;
		w_inv_mant_19_i64 = FX_AND64(FX_SHR64(w_inv_mant_19_i64, 28), FX_CREATE64(0, 0x7ffff));

#if TRX_DEBUG_PRINT_DETAIL
		TRX_PR_INT64("w_inv_mant_19_i64", w_inv_mant_19_i64);
		printf("st->hw_w_exp_6 = 0x%x\n", st->hw_w_exp_6);
#endif

		// xxx st->hw_w_mant_19 = trx_inv_calc_w_inv((UINT32) w_inv_mant_19_i64);
		st->hw_w_mant_19 = trx_inv_calc_w_inv(FX_LO64(w_inv_mant_19_i64));

		/* adjust if out of range */
		if (st->hw_w_mant_19 & 0x80000) {
			st->hw_w_mant_19 = 0x40000;
			st->hw_w_exp_6++;
		}
	} else {
		/* no persp. correct, force W=1.0 */
		FX_SET64(st->hw_w_inv_clmp_48_i64, 1, 0);	// 1.0
		st->hw_w_mant_19 = 0x40000;
		st->hw_w_exp_6 = 16;
		st->hw_w_is_neg = 0;
	} 

#if TRX_ERROR_CHECK
	/* check range of w exp and mantissa */
	if (st->hw_w_exp_6 > 48) {
		printf("st->hw_w_exp_6 out of range\n");
		exit(1);
	}
	if ((st->hw_w_mant_19 < 0x40000) || (st->hw_w_mant_19 > 0x7ffff)) {
		printf("st->hw_w_mant_19 out of range\n");
		exit(1);
	}
#endif


	/* s/w, t/w int to float, signed, maybe leading 0/1 */
	it_s_mant_20s_i64 = st->it_s_i64;
	it_s_exp_5 = 28;
	// xxx it_s_sgn_i64 = INT64_SIGN(st->it_s_i64);
   it_s_sgn_hi = FX_HI64(it_s_mant_20s_i64) & (1 << (47 - 32));
	// xxx it_s_mant_20s_i64 <<= 1;
   it_s_mant_20s_i64 = FX_SHL64(it_s_mant_20s_i64, 1);
	// xxx while (!(INT64_SIGN(it_s_mant_20s_i64) ^ it_s_sgn_i64) && (it_s_exp_5 > 0)) {
   //while (FX_EQ064(FX_XOR64(INT64_SIGN(it_s_mant_20s_i64), it_s_sgn_i64)) && (it_s_exp_5 > 0)) {
   while (((FX_HI64(it_s_mant_20s_i64) & (1 << (47 - 32))) == it_s_sgn_hi) && (it_s_exp_5 > 0)) {
		// xxx it_s_mant_20s_i64 <<= 1;
		it_s_mant_20s_i64 = FX_SHL64(it_s_mant_20s_i64, 1);
		it_s_exp_5--;
	}
	/* right adjust */
	// xxx it_s_mant_20s_i64 = (it_s_mant_20s_i64 >> 29) & (UINT64)0xfffff;
	it_s_mant_20s_i64 = FX_AND64(FX_SHR64(it_s_mant_20s_i64, 29), FX_CREATE64(0, 0xfffff));

	it_s_exp_5 = 28 - it_s_exp_5;

#if TRX_DEBUG_PRINT_DETAIL
	TRX_PR_INT64("it_s_mant_20s_i64", it_s_mant_20s_i64);
	printf("it_s_exp_5 = %d\n", it_s_exp_5);
	printf("\n");
#endif

	it_t_mant_20s_i64 = st->it_t_i64;
	it_t_exp_5 = 28;
	// xxx it_t_sgn_64 = INT64_SIGN(st->it_t_i64);
   it_t_sgn_hi = FX_HI64(it_t_mant_20s_i64) & (1 << (47 - 32));
	// xxx it_t_mant_20s_i64 <<= 1;
   it_t_mant_20s_i64 = FX_SHL64(it_t_mant_20s_i64, 1);
	// xxx while (!(INT64_SIGN(it_t_mant_20s_i64) ^ it_t_sgn_64) && (it_t_exp_5 > 0)) {
	//while (FX_EQ064(FX_XOR64(INT64_SIGN(it_t_mant_20s_i64), it_t_sgn_64)) && (it_t_exp_5 > 0)) {
   while (((FX_HI64(it_t_mant_20s_i64) & (1 << (47 - 32))) == it_t_sgn_hi) && (it_t_exp_5 > 0)) {
		// xxx it_t_mant_20s_i64 <<= 1;
		it_t_mant_20s_i64 = FX_SHL64(it_t_mant_20s_i64, 1);
		it_t_exp_5--;
	}
	/* right adjust */
	// xxx it_t_mant_20s_i64 = (it_t_mant_20s_i64 >> 29) & (UINT64)0xfffff;
	it_t_mant_20s_i64 = FX_AND64(FX_SHR64(it_t_mant_20s_i64, 29), FX_CREATE64(0, 0xfffff));

	it_t_exp_5 = 28 - it_t_exp_5;

	/* TRX_PR_INT64("it_t_mant_20s_i64", it_t_mant_20s_i64); */
	/* printf("it_t_exp_5 = %d\n", it_t_exp_5); */


#if TRX_ERROR_CHECK
	/* check range of it_s,it_t exponents */
	if (it_s_exp_5 > 28) {
		printf("it_s_exp_5 out of range\n");
		exit(1);
	}
	if (it_t_exp_5 > 28) {
		printf("it_t_exp_5 out of range\n");
		exit(1);
	}
#endif


	/* (s/w)*w  (t/w)*w */
	/*     sign extends (s/w), (t/w) before multiply */
	/*     leaves mantissas sign extended  */
	// xxx st->hw_s_mant_39s_i64 = (((INT64) st->hw_w_mant_19)
	// xxx 		* ((INT64) (it_s_mant_20s_i64 | ((it_s_mant_20s_i64 & 0x80000) ? ~0xfffff : 0))));
/*
	st->hw_s_mant_39s_i64 = FX_MUL64(FX_CREATE64(0, (INT32) st->hw_w_mant_19),
	 		((FX_LO64(it_s_mant_20s_i64) & 0x80000) 
					? FX_OR64(it_s_mant_20s_i64, FX_CREATE64(~0, ~0xfffff)) : it_s_mant_20s_i64));
*/
//	st->hw_s_mant_39s_i64 = FX_MUL64(FX_CREATE64(0, (INT32) st->hw_w_mant_19), 
//												FX_SGNEXT64(it_s_mant_20s_i64, 19));
	st->hw_s_mant_39s_i64 = FX_MUL32( ((INT32) st->hw_w_mant_19), 
										(INT32) TRX_SGNEXT32((INT32) FX_LO64(it_s_mant_20s_i64), 19) );
	st->hw_s_exp_7 = st->hw_w_exp_6 + 28 - it_s_exp_5;


#if TRX_DEBUG_PRINT_DETAIL
	TRX_PR_INT64("st->hw_s_mant_39s_i64",  st->hw_s_mant_39s_i64);
	printf("st->hw_s_exp_7 = %d\n", st->hw_s_exp_7);
	printf("\n");
#endif

	// xxx st->hw_t_mant_39s_i64 = (((INT64) st->hw_w_mant_19)
	// xxx 		* ((INT64) (it_t_mant_20s_i64 | ((it_t_mant_20s_i64 & 0x80000) ? ~0xfffff : 0))));
//	st->hw_t_mant_39s_i64 = FX_MUL64(FX_CREATE64(0, (INT32) st->hw_w_mant_19), 
//												FX_SGNEXT64(it_t_mant_20s_i64, 19));
	st->hw_t_mant_39s_i64 = FX_MUL32( ((INT32) st->hw_w_mant_19), 
										(INT32) TRX_SGNEXT32((INT32) FX_LO64(it_t_mant_20s_i64), 19) );
	st->hw_t_exp_7 = st->hw_w_exp_6 + 28 - it_t_exp_5;

#if TRX_DEBUG_PRINT_DETAIL
	TRX_PR_INT64("st->hw_t_mant_39s_i64",  st->hw_t_mant_39s_i64);
	printf("st->hw_t_exp_7 = %d\n", st->hw_t_exp_7);
	printf("\n");
#endif

#if TRX_ERROR_CHECK
	/* check range of s,t mant,exponents */
	if ((st->hw_s_mant_39s_i64 < -st->signMask39_i64) || (st->hw_s_mant_39s_i64 >= st->signMask39_i64)) {
		printf("st->hw_s_mant_39s_i64 out of range\n");
		exit(1);
	}
	if (st->hw_s_exp_7 > 76) {
		printf("st->hw_s_exp_7 out of range\n");
		exit(1);
	}
	if ((st->hw_t_mant_39s_i64 < -st->signMask39_i64) || (st->hw_t_mant_39s_i64 >= st->signMask39_i64)) {
		printf("st->hw_t_mant_39s_i64 out of range\n");
		exit(1);
	}
	if (st->hw_t_exp_7 > 76) {
		printf("st->hw_t_exp_7 out of range\n");
		exit(1);
	}
#endif

	/* s,t to send to the address generation unit */
	/*    right shift is arithmetic (sign bit is shifted into the MSBs */
	/*    mantissa can be entirely shifted off the right (exp=0) */
	/*    mantissa can be entirely clipped as MSBs (exp=76) */
 	// xxx st->hw_s_fxd_is_neg = ((st->hw_s_mant_39s_i64 & st->signMask39_i64) != 0);
 	st->hw_s_fxd_is_neg = ((FX_HI64(st->hw_s_mant_39s_i64) & (1 << (38 - 32))) != 0);
	if (st->hw_s_exp_7 <= 12)
		hw_s_fxd_exp_7 = 13;
	else
		hw_s_fxd_exp_7 = st->hw_s_exp_7;
 	// xxx st->hw_s_fxd_12_i64 = ((INT64) (st->hw_s_mant_39s_i64 << 14)) >> (76 - st->hw_s_exp_7);
 	st->hw_s_fxd_12_i64 = FX_SHR64(FX_SHL64(st->hw_s_mant_39s_i64, 14), (76 - hw_s_fxd_exp_7));
	// GMT: stash .8 fraction away
	st->hw_s_frac_8 = 0xFF & FX_LO64(FX_SHR64(FX_SHL64(st->hw_s_mant_39s_i64, 18), (76 - hw_s_fxd_exp_7)));
	// Devil: stash .11 fraction away
	st->hw_s_frac_11 = 0x7FF & FX_LO64(FX_SHR64(FX_SHL64(st->hw_s_mant_39s_i64, 21), (76 - hw_s_fxd_exp_7)));
 	// xxx st->hw_s_fxd_clmp_pos = !st->hw_s_fxd_is_neg && (st->hw_s_fxd_12_i64 & (~0xfff));
  	st->hw_s_fxd_clmp_pos = !st->hw_s_fxd_is_neg 
				&& (FX_HI64(st->hw_s_fxd_12_i64) || (FX_LO64(st->hw_s_fxd_12_i64) & 
				~(st->tmirrors ? 0x1fff : 0xfff)));
 	// xxx st->hw_s_fxd_12_i64 &= 0xfff;
	// GMT: increased mask from 0xfff to 0x1fff for texture mirroring
 	st->hw_s_fxd_12_i64 = FX_AND64(st->hw_s_fxd_12_i64, FX_CREATE64(0, 0x1fff));

#if TRX_DEBUG_PRINT_DETAIL
	TRX_PR_INT64("st->hw_s_fxd_12_i64", st->hw_s_fxd_12_i64);
	printf("st->hw_s_fxd_is_neg= %d\n", st->hw_s_fxd_is_neg);
	printf("st->hw_s_fxd_clmp_pos= %d\n", st->hw_s_fxd_clmp_pos);
	printf("\n");
#endif

 	// xxx st->hw_t_fxd_is_neg = ((st->hw_t_mant_39s_i64 & st->signMask39_i64) != 0);
 	st->hw_t_fxd_is_neg = ((FX_HI64(st->hw_t_mant_39s_i64) & (1 << (38 - 32))) != 0);
	if (st->hw_t_exp_7 <= 12)
		hw_t_fxd_exp_7 = 13;
	else
		hw_t_fxd_exp_7 = st->hw_t_exp_7;
 	// xxx st->hw_t_fxd_12_i64 = ((INT64) (st->hw_t_mant_39s_i64 << 14)) >> (76 - hw_t_fxd_exp_7);
 	st->hw_t_fxd_12_i64 = FX_SHR64(FX_SHL64(st->hw_t_mant_39s_i64, 14), (76 - hw_t_fxd_exp_7));
	// GMT: stash .8 fraction away
	st->hw_t_frac_8 = 0xFF & FX_LO64(FX_SHR64(FX_SHL64(st->hw_t_mant_39s_i64, 18), (76 - hw_t_fxd_exp_7)));
	// Devil: stash .11 fraction away
	st->hw_t_frac_11 = 0x7FF & FX_LO64(FX_SHR64(FX_SHL64(st->hw_t_mant_39s_i64, 21), (76 - hw_t_fxd_exp_7)));
 	// xxx st->hw_t_fxd_clmp_pos = !st->hw_t_fxd_is_neg && (st->hw_t_fxd_12_i64 & (~0xfff));
  	st->hw_t_fxd_clmp_pos = !st->hw_t_fxd_is_neg 
				&& (FX_HI64(st->hw_t_fxd_12_i64) || (FX_LO64(st->hw_t_fxd_12_i64) &
				~(st->tmirrort ? 0x1fff : 0xfff)));
 	// xxx st->hw_t_fxd_12_i64 &= 0xfff;
	// GMT: increased mask from 0xfff to 0x1fff for texture mirroring
 	st->hw_t_fxd_12_i64 = FX_AND64(st->hw_t_fxd_12_i64, FX_CREATE64(0, 0x1fff));

	/* TRX_PR_INT64("st->hw_t_fxd_12_i64", st->hw_t_fxd_12_i64); */
	/* printf("st->hw_t_fxd_is_neg= %d\n", st->hw_t_fxd_is_neg); */
	/* printf("st->hw_t_fxd_clmp_pos= %d\n", st->hw_t_fxd_clmp_pos); */



	/* s,t to send to the LOD calc. module */
	st->hw_s_lod_mant_12s_i64 = st->hw_s_mant_39s_i64;
	// xxx hw_s_lod_sgn_i64 = st->hw_s_mant_39s_i64 & st->signMask39_i64;
	hw_s_lod_sgn_hi = FX_HI64(st->hw_s_mant_39s_i64) & (1 << (38 - 32));
	hw_s_norm_adj = 20;
	// xxx st->hw_s_lod_mant_12s_i64 <<= 1;
	st->hw_s_lod_mant_12s_i64 = FX_SHL64(st->hw_s_lod_mant_12s_i64, 1);
	// xxx while (!((st->hw_s_lod_mant_12s_i64 & st->signMask39_i64) ^ hw_s_lod_sgn_i64) && (hw_s_norm_adj > 0)) {
	while (!((FX_HI64(st->hw_s_lod_mant_12s_i64) & (1 << (38 -32))) ^ hw_s_lod_sgn_hi) && (hw_s_norm_adj > 0)) {
		// xxx st->hw_s_lod_mant_12s_i64 <<= 1;
		st->hw_s_lod_mant_12s_i64 = FX_SHL64(st->hw_s_lod_mant_12s_i64, 1);
		hw_s_norm_adj--;
	}
	st->hw_s_lod_exp_7 = st->hw_s_exp_7 + hw_s_norm_adj;
	/* right adjust */
	// xxx st->hw_s_lod_mant_12s_i64 = (st->hw_s_lod_mant_12s_i64 >> 28) & (UINT64)0xfff;
	st->hw_s_lod_mant_12s_i64 = FX_AND64(FX_SHR64(st->hw_s_lod_mant_12s_i64, 28), FX_CREATE64(0, 0xfff));

/* add start */

	/* TRX_PR_INT64("st->hw_s_lod_mant_12s_i64", st->hw_s_lod_mant_12s_i64); */
	/* printf("st->hw_s_lod_exp_7 = %d\n", st->hw_s_lod_exp_7); */


	st->hw_t_lod_mant_12s_i64 = st->hw_t_mant_39s_i64;
	// xxx hw_t_lod_sgn_i64 = st->hw_t_mant_39s_i64 & st->signMask39_i64;
	hw_t_lod_sgn_hi = FX_HI64(st->hw_t_mant_39s_i64) & (1 << (38 - 32));
	hw_t_norm_adj = 20;
	// xxx st->hw_t_lod_mant_12s_i64 <<= 1;
	st->hw_t_lod_mant_12s_i64 = FX_SHL64(st->hw_t_lod_mant_12s_i64, 1);
	// xxx while (!((st->hw_t_lod_mant_12s_i64 & st->signMask39_i64) ^ hw_t_lod_sgn_i64) && (hw_t_norm_adj > 0)) {
	while (!((FX_HI64(st->hw_t_lod_mant_12s_i64) & (1 << (38 -32))) ^ hw_t_lod_sgn_hi) && (hw_t_norm_adj > 0)) {
		// xxx st->hw_t_lod_mant_12s_i64 <<= 1;
		st->hw_t_lod_mant_12s_i64 = FX_SHL64(st->hw_t_lod_mant_12s_i64, 1);
		hw_t_norm_adj--;
	}
	st->hw_t_lod_exp_7 = st->hw_t_exp_7 + hw_t_norm_adj;
	/* right adjust */
	// xxx st->hw_t_lod_mant_12s_i64 = (st->hw_t_lod_mant_12s_i64 >> 28) & (UINT64)0xfff;
	st->hw_t_lod_mant_12s_i64 = FX_AND64(FX_SHR64(st->hw_t_lod_mant_12s_i64, 28), FX_CREATE64(0, 0xfff));

	/* TRX_PR_INT64("st->hw_t_lod_mant_12s_i64", st->hw_t_lod_mant_12s_i64); */
	/* printf("st->hw_t_lod_exp_7 = %d\n", st->hw_t_lod_exp_7); */

#if TRX_ERROR_CHECK
	/* check range of s_lod,t_lod exponents */
	if (st->hw_s_lod_exp_7 > 96) {
		printf("st->hw_s_lod_exp_7 out of range\n");
		exit(1);
	}
	if (st->hw_t_lod_exp_7 > 96) {
		printf("st->hw_t_lod_exp_7 out of range\n");
		exit(1);
	}
#endif
}



/***************************************************************************/
/* lod:  calculate square term                                             */
/*       sets output in res_mant_8, res_exp_8, but also corrupts           */
/*       dst_mant_24s_i64                                                      */
void lod_calc_sq_term(TRX_CALC_SQ_TERM_STRUCT* sq)
{


	INT32 prod_mant_24s;
	UINT32 prod_exp_7;

	INT32 dst_mant_25s;	/* local, int32, sign extended to 25 bits version */

	/* exponent difference for adding squares */
	INT32 exp_diff;

	INT32 greater_mant_25s, lesser_mant_25s;
	INT32 sum_mant;	/* sum, abs value mantissa */
	UINT32 sub_exp_7;


	/* sign extend before multiply */
	// xxx prod_mant_24s = 
	// xxx 	((INT32) ((sq->dw_mant_12s_i64 & 0x800) ?
	// xxx 	(sq->dw_mant_12s_i64 | ~0xfff) : (sq->dw_mant_12s_i64 & 0xfff)))
	// xxx 	* ((INT32) ((sq->st_mant_12s_i64 & 0x800) ? 
	// xxx 	(sq->st_mant_12s_i64 | ~0xfff) : (sq->st_mant_12s_i64 & 0xfff)));
	prod_mant_24s = ((INT32) TRX_SGNEXT32(FX_LO64(sq->dw_mant_12s_i64), 11))
						* ((INT32) TRX_SGNEXT32(FX_LO64(sq->st_mant_12s_i64), 11));
	prod_exp_7 = sq->dw_exp_6 + sq->st_exp_7;


	/* sign extend dst_mant_25s, prod_mant_24s is already sign extended */
	// xxx dst_mant_25s = (INT32) ((sq->dst_mant_24s_i64 & 0x800000) ?
	// xxx 				(sq->dst_mant_24s_i64 | ~0xffffff) : (sq->dst_mant_24s_i64 & 0xffffff));
	dst_mant_25s = TRX_SGNEXT32(FX_LO64(sq->dst_mant_24s_i64), 23);

#if TRX_SQ_DEBUG
	printf("prod_mant_24s = 0x%x\n", prod_mant_24s);
	printf("prod_exp_7 = %d\n", prod_exp_7);
	printf("dst_mant_25s = 0x%x\n", dst_mant_25s);
	printf("sq->dst_exp_6 = %d\n", sq->dst_exp_6);
#endif

	/* swap, shift and ~ lesser mant */
	exp_diff = prod_exp_7 - sq->dst_exp_6;
	if ((dst_mant_25s == 0) || ((exp_diff > 0) && (prod_mant_24s != 0))) {
		sub_exp_7 = prod_exp_7;
		greater_mant_25s = prod_mant_24s; // 25 bits?
		lesser_mant_25s = ~(dst_mant_25s >> ((exp_diff > 23) ? 23 : exp_diff));
	} else {
		sub_exp_7 = sq->dst_exp_6;
		greater_mant_25s = dst_mant_25s;
		lesser_mant_25s = ~(prod_mant_24s >> ((exp_diff < -23) ? 23 : -exp_diff));
	}

	/* 25-bit signed result */
	sum_mant = (greater_mant_25s + lesser_mant_25s + 1) & 0x1ffffff;	

#if TRX_SQ_DEBUG
	printf("greater_mant_25s = 0x%x\n", greater_mant_25s);
	printf("lesser_mant_25s = 0x%x\n", lesser_mant_25s);
	printf("sum_mant    = 0x%x\n", sum_mant);
#endif

	/* absolute value */
	if (sum_mant & (1 << 24))
		sum_mant = (~sum_mant + 1) & 0xffffff;	/* 24-bit positive result */

#if TRX_SQ_DEBUG
	printf("sum_mant(abs)= 0x%x\n", sum_mant);
#endif

	/* full normalize, leading 1 */
	if (sum_mant == 0) {

/* add end */

		/* mant is zero, so set to lowest expressable value */
		sum_mant = 0x80;
		sq->res_exp_8 = 0;
	} else {
		sq->res_exp_8 = sub_exp_7 + 23;
		while (!(sum_mant & (1 << 23))) {
			sum_mant <<= 1;
			sq->res_exp_8--;
		}
	}

	/* right adjust and trim leading 1, lookup x^2 */
	sq->res_mant_8 = trx_x2_table[(sum_mant >> 16) & 0x7f];
	//sq->res_exp_8 <<= 1;

#if TRX_SQ_DEBUG
	printf("sq->res_mant_8 = 0x%x\n", sq->res_mant_8);
	printf("sq->res_exp_8 = %d\n", sq->res_exp_8);

	printf("log2(sq term)/2= %f\n", TRX_LOG2_DBL(((double) sq->res_mant_8) 
				* pow(2, (double) ((int) (sq->res_exp_8 << 1) - 8 - 180))) / 2.0) ;
	printf("\n");
#endif

}



/***************************************************************************/
/* hardware bit accurate lod calc. */
void trx_lod(TRX_STLOD_STRUCT* st)
{
	/* allocate and create a pointer to the lod_calc_sq_term() i/o struct */
	TRX_CALC_SQ_TERM_STRUCT sq_tmp;
	TRX_CALC_SQ_TERM_STRUCT* sq = &sq_tmp;

	// xxx INT64 int_sgn_i64;	/* integer sign for int to float conv. */
	INT32 int_sgn_hi;	/* integer sign for int to float conv. */

	/* 4 square terms */
	//INT32 sq_xs_mant_8;
	//UINT32 sq_xs_exp_9;
	//INT32 sq_xt_mant_8;
	//UINT32 sq_xt_exp_9;
	//INT32 sq_ys_mant_8;
	//UINT32 sq_ys_exp_9;
	//INT32 sq_yt_mant_8;
	//UINT32 sq_yt_exp_9;

	/* exponent difference for adding squares */
	INT32 exp_diff;

	/* two square sums */
	INT32 sq_xsum_mant_9;
	UINT32 sq_xsum_exp_9;
	INT32 sq_ysum_mant_9;
	UINT32 sq_ysum_exp_9;



#if TRX_PRINT_INPUT_VALS

	printf("\n");
	printf("trexfunc.c:  TRX_PRINT_INPUT_VALS\n");

	TRX_PR_INT64("  it_s_i64    ", st->it_s_i64);
	TRX_PR_INT64("  reg_dsdx_i64", st->reg_dsdx_i64);
	TRX_PR_INT64("  reg_dsdy_i64", st->reg_dsdy_i64);
	TRX_PR_INT64("  it_t_i64    ", st->it_t_i64);
	TRX_PR_INT64("  reg_dtdx_i64", st->reg_dtdx_i64);
	TRX_PR_INT64("  reg_dtdy_i64", st->reg_dtdy_i64);
	printf("  tpersp_st = %d\n", st->tpersp_st);
	TRX_PR_INT64("  it_w_inv_i64", st->it_w_inv_i64);
	TRX_PR_INT64("  reg_dwdx_i64", st->reg_dwdx_i64);
	TRX_PR_INT64("  reg_dwdy_i64", st->reg_dwdy_i64);

	printf("\n");
#endif


	/*** x pair ***/
	/* dwdx */
   sq->dw_mant_12s_i64 = st->reg_dwdx_i64;
   sq->dw_exp_6 = 36;
   // xxx int_sgn_i64 = INT64_SIGN(sq->dw_mant_12s_i64);
   int_sgn_hi = FX_HI64(sq->dw_mant_12s_i64) & (1 << (47 - 32));
   // xxx sq->dw_mant_12s_i64 <<= 1;
   sq->dw_mant_12s_i64 = FX_SHL64(sq->dw_mant_12s_i64, 1);
   // xxx while (!(INT64_SIGN(sq->dw_mant_12s_i64) ^ int_sgn_i64) && (sq->dw_exp_6 > 0)) {
   while (((FX_HI64(sq->dw_mant_12s_i64) & (1 << (47 - 32))) == int_sgn_hi) && (sq->dw_exp_6 > 0)) {
      // xxx sq->dw_mant_12s_i64 <<= 1;
   	sq->dw_mant_12s_i64 = FX_SHL64(sq->dw_mant_12s_i64, 1);
      sq->dw_exp_6--;
   }
   /* right adjust */
   // xxx sq->dw_mant_12s_i64 = (sq->dw_mant_12s_i64 >> 37) & (UINT64)0xfff;
   sq->dw_mant_12s_i64 = FX_AND64(FX_SHR64(sq->dw_mant_12s_i64, 37), FX_CREATE64(0, 0xfff));

	/* s */
	sq->st_mant_12s_i64 = st->hw_s_lod_mant_12s_i64;
	sq->st_exp_7 = st->hw_s_lod_exp_7;

#if TRX_PRINT_INPUT_VALS
	TRX_PR_INT64("  hw_s_lod_mant_12s_i64", st->hw_s_lod_mant_12s_i64);
	printf(      "  hw_s_lod_exp_7        = %d\n", st->hw_s_lod_exp_7);
	TRX_PR_INT64("  dw_mant_12s_i64(dwdx)", sq->dw_mant_12s_i64);
	printf(      "  dw_exp_6(dwdx)        = %d\n", sq->dw_exp_6);
	printf("\n");
#endif

	/* dsdx */
   sq->dst_mant_24s_i64 = st->reg_dsdx_i64;
   sq->dst_exp_6 = 24 + 59;
   // xxx int_sgn_i64 = INT64_SIGN(sq->dst_mant_24s_i64);
	int_sgn_hi = FX_HI64(sq->dst_mant_24s_i64) & (1 << (47 - 32));
   // xxx sq->dst_mant_24s_i64 <<= 1;
	sq->dst_mant_24s_i64 = FX_SHL64(sq->dst_mant_24s_i64, 1);
   // xxx while (!(INT64_SIGN(sq->dst_mant_24s_i64) ^ int_sgn_i64) && (sq->dst_exp_6 > 59)) {
	while (((FX_HI64(sq->dst_mant_24s_i64) & (1 << (47 - 32))) == int_sgn_hi) && (sq->dst_exp_6 > 59)) {
      // xxx sq->dst_mant_24s_i64 <<= 1;
		sq->dst_mant_24s_i64 = FX_SHL64(sq->dst_mant_24s_i64, 1);
      sq->dst_exp_6--;
   }
   /* right adjust */
   // xxx sq->dst_mant_24s_i64 = (sq->dst_mant_24s_i64 >> 25) & (UINT64)0xffffff;
	sq->dst_mant_24s_i64 = FX_AND64(FX_SHR64(sq->dst_mant_24s_i64, 25), FX_CREATE64(0, 0xffffff));

	lod_calc_sq_term(sq);
	st->sq_xs_mant_8 = sq->res_mant_8;
	st->sq_xs_exp_8 = sq->res_exp_8;


	/* t */
	sq->st_mant_12s_i64 = st->hw_t_lod_mant_12s_i64;
	sq->st_exp_7 = st->hw_t_lod_exp_7;

	/* dtdx */
   sq->dst_mant_24s_i64 = st->reg_dtdx_i64;
   sq->dst_exp_6 = 24 + 59;
   // xxx int_sgn_i64 = INT64_SIGN(sq->dst_mant_24s_i64);
   int_sgn_hi = FX_HI64(sq->dst_mant_24s_i64) & (1 << (47 - 32));
   // xxx sq->dst_mant_24s_i64 <<= 1;
   sq->dst_mant_24s_i64 = FX_SHL64(sq->dst_mant_24s_i64, 1);
   // xxx while (!(INT64_SIGN(sq->dst_mant_24s_i64) ^ int_sgn_i64) && (sq->dst_exp_6 > 59)) {
   while (((FX_HI64(sq->dst_mant_24s_i64) & (1 << (47 - 32))) == int_sgn_hi) && (sq->dst_exp_6 > 59)) {
      // xxx sq->dst_mant_24s_i64 <<= 1;
      sq->dst_mant_24s_i64 = FX_SHL64(sq->dst_mant_24s_i64, 1);
      sq->dst_exp_6--;
   }
   /* right adjust */
   // xxx sq->dst_mant_24s_i64 = (sq->dst_mant_24s_i64 >> 25) & (UINT64)0xffffff;
   sq->dst_mant_24s_i64 = FX_AND64(FX_SHR64(sq->dst_mant_24s_i64, 25), FX_CREATE64(0, 0xffffff));

	lod_calc_sq_term(sq);
	st->sq_xt_mant_8 = sq->res_mant_8;
	st->sq_xt_exp_8 = sq->res_exp_8;




	/*** y pair ***/
	/* dwdy */
   sq->dw_mant_12s_i64 = st->reg_dwdy_i64;
   sq->dw_exp_6 = 36;
   // xxx int_sgn_i64 = INT64_SIGN(sq->dw_mant_12s_i64);
   int_sgn_hi = FX_HI64(sq->dw_mant_12s_i64) & (1 << (47 - 32));
   // xxx sq->dw_mant_12s_i64 <<= 1;
   sq->dw_mant_12s_i64 = FX_SHL64(sq->dw_mant_12s_i64, 1);
   // xxx while (!(INT64_SIGN(sq->dw_mant_12s_i64) ^ int_sgn_i64) && (sq->dw_exp_6 > 0)) {
   while (((FX_HI64(sq->dw_mant_12s_i64) & (1 << (47 - 32))) == int_sgn_hi) && (sq->dw_exp_6 > 0)) {
      // xxx sq->dw_mant_12s_i64 <<= 1;
   	sq->dw_mant_12s_i64 = FX_SHL64(sq->dw_mant_12s_i64, 1);
      sq->dw_exp_6--;
   }
   /* right adjust */
   // xxx sq->dw_mant_12s_i64 = (sq->dw_mant_12s_i64 >> 37) & (UINT64)0xfff;
   sq->dw_mant_12s_i64 = FX_AND64(FX_SHR64(sq->dw_mant_12s_i64, 37), FX_CREATE64(0, 0xfff));


	/* s */
	sq->st_mant_12s_i64 = st->hw_s_lod_mant_12s_i64;
	sq->st_exp_7 = st->hw_s_lod_exp_7;


	/* dsdy */
   sq->dst_mant_24s_i64 = st->reg_dsdy_i64;
   sq->dst_exp_6 = 24 + 59;
   // xxx int_sgn_i64 = INT64_SIGN(sq->dst_mant_24s_i64);
	int_sgn_hi = FX_HI64(sq->dst_mant_24s_i64) & (1 << (47 - 32));
   // xxx sq->dst_mant_24s_i64 <<= 1;
	sq->dst_mant_24s_i64 = FX_SHL64(sq->dst_mant_24s_i64, 1);
   // xxx while (!(INT64_SIGN(sq->dst_mant_24s_i64) ^ int_sgn_i64) && (sq->dst_exp_6 > 59)) {
	while (((FX_HI64(sq->dst_mant_24s_i64) & (1 << (47 - 32))) == int_sgn_hi) && (sq->dst_exp_6 > 59)) {
      // xxx sq->dst_mant_24s_i64 <<= 1;
		sq->dst_mant_24s_i64 = FX_SHL64(sq->dst_mant_24s_i64, 1);
      sq->dst_exp_6--;
   }
   /* right adjust */
   // xxx sq->dst_mant_24s_i64 = (sq->dst_mant_24s_i64 >> 25) & (UINT64)0xffffff;
	sq->dst_mant_24s_i64 = FX_AND64(FX_SHR64(sq->dst_mant_24s_i64, 25), FX_CREATE64(0, 0xffffff));

	lod_calc_sq_term(sq);
	st->sq_ys_mant_8 = sq->res_mant_8;
	st->sq_ys_exp_8 = sq->res_exp_8;



	/* t */
	sq->st_mant_12s_i64 = st->hw_t_lod_mant_12s_i64;
	sq->st_exp_7 = st->hw_t_lod_exp_7;


	/* dtdy */
   sq->dst_mant_24s_i64 = st->reg_dtdy_i64;
   sq->dst_exp_6 = 24 + 59;
   // xxx int_sgn_i64 = INT64_SIGN(sq->dst_mant_24s_i64);
	int_sgn_hi = FX_HI64(sq->dst_mant_24s_i64) & (1 << (47 - 32));
   // xxx sq->dst_mant_24s_i64 <<= 1;
	sq->dst_mant_24s_i64 = FX_SHL64(sq->dst_mant_24s_i64, 1);
   // xxx while (!(INT64_SIGN(sq->dst_mant_24s_i64) ^ int_sgn_i64) && (sq->dst_exp_6 > 59)) {
	while (((FX_HI64(sq->dst_mant_24s_i64) & (1 << (47 - 32))) == int_sgn_hi) && (sq->dst_exp_6 > 59)) {
      // xxx sq->dst_mant_24s_i64 <<= 1;
		sq->dst_mant_24s_i64 = FX_SHL64(sq->dst_mant_24s_i64, 1);
      sq->dst_exp_6--;
   }
   /* right adjust */
   // xxx sq->dst_mant_24s_i64 = (sq->dst_mant_24s_i64 >> 25) & (UINT64)0xffffff;
	sq->dst_mant_24s_i64 = FX_AND64(FX_SHR64(sq->dst_mant_24s_i64, 25), FX_CREATE64(0, 0xffffff));

	lod_calc_sq_term(sq);
	st->sq_yt_mant_8 = sq->res_mant_8;
	st->sq_yt_exp_8 = sq->res_exp_8;


	/* add each pair of square terms */
	exp_diff = st->sq_xs_exp_8 - st->sq_xt_exp_8;
	if (exp_diff >= 0) {
		sq_xsum_mant_9 = st->sq_xs_mant_8 + 
							(st->sq_xt_mant_8 >> ((exp_diff > 4) ? 8 : (2 * exp_diff)));
		sq_xsum_exp_9 = st->sq_xs_exp_8 << 1;
	} else {
		sq_xsum_mant_9 = st->sq_xt_mant_8 + 
							(st->sq_xs_mant_8 >> ((exp_diff < -4) ? 8 : (-2 * exp_diff)));
		sq_xsum_exp_9 = st->sq_xt_exp_8 << 1;
	}
	exp_diff = st->sq_ys_exp_8 - st->sq_yt_exp_8;
	if (exp_diff >= 0) {
		sq_ysum_mant_9 = st->sq_ys_mant_8 + 
							(st->sq_yt_mant_8 >> ((exp_diff > 4) ? 8 : (2 * exp_diff)));
		sq_ysum_exp_9 = st->sq_ys_exp_8 << 1;
	} else {
		sq_ysum_mant_9 = st->sq_yt_mant_8 + 
							(st->sq_ys_mant_8 >> ((exp_diff < -4) ? 8 : (-2 * exp_diff)));
		sq_ysum_exp_9 = st->sq_yt_exp_8 << 1;
	}

#if TRX_SQ_DEBUG
	printf("sq_xsum_mant_9 = 0x%x\n", sq_xsum_mant_9);
	printf("sq_xsum_exp_9 = %d\n", sq_xsum_exp_9);
	printf("sq_ysum_mant_9 = 0x%x\n", sq_ysum_mant_9);
	printf("sq_ysum_exp_9 = %d\n", sq_ysum_exp_9);
	printf("\n");
#endif

	/* normalize sums */
#if TRX_ERROR_CHECK
	if (!(sq_xsum_mant_9 & 0x1c0)) {
		printf("sq_xsum_mant_9 has 3 leading 0's\n");
		exit(1);
	}
	if (!(sq_ysum_mant_9 & 0x1c0)) {
		printf("sq_ysum_mant_9 has 3 leading 0's\n");
		exit(1);
	}
#endif
	if (!(sq_xsum_mant_9 & 0x100)) {
		if (!(sq_xsum_mant_9 & 0x080)) {
			/* two leading 0's */
			sq_xsum_mant_9 <<= 2;
		} else {
			/* one leading 0 */
			sq_xsum_mant_9 <<= 1;
			sq_xsum_exp_9++;
		}
	} else {
		/* already has leading 1 */
		sq_xsum_exp_9 += 2;
		sq_xsum_mant_9 &= ~1; /* trunc lsb */
	}
	if (!(sq_ysum_mant_9 & 0x100)) {
		if (!(sq_ysum_mant_9 & 0x080)) {
			/* two leading 0's */
			sq_ysum_mant_9 <<= 2;
		} else {
			/* one leading 0 */
			sq_ysum_mant_9 <<= 1;
			sq_ysum_exp_9++;
		}
	} else {
		/* already has leading 1 */
		sq_ysum_exp_9 += 2;
		sq_ysum_mant_9 &= ~1; /* trunc lsb */
	}
#if TRX_SQ_DEBUG
	printf("norm sq_xsum_mant_9 = 0x%x\n", sq_xsum_mant_9);
	printf("norm sq_xsum_exp_9 = %d\n", sq_xsum_exp_9);
	printf("norm sq_ysum_mant_9 = 0x%x\n", sq_ysum_mant_9);
	printf("norm sq_ysum_exp_9 = %d\n", sq_ysum_exp_9);
	printf("\n");
#endif


#if TRX_ERROR_CHECK
	/* check exponent ranges */
	if ((sq_xsum_exp_9 > 310) || (sq_ysum_exp_9 > 310)) {
		printf("sq_?sum_exp_9 out of range\n");
		printf("sq_xsum_exp_9=%d\n", sq_xsum_exp_9);
		printf("sq_ysum_exp_9=%d\n", sq_ysum_exp_9);
		trx_error(st);
	}
#endif

	/* select max of the two sums, trunc 1 lsb from mantissa */
	if ((sq_xsum_exp_9 > sq_ysum_exp_9) || ((sq_xsum_exp_9 == sq_ysum_exp_9)
			&& (sq_xsum_mant_9 >= sq_ysum_mant_9))) {
		st->hw_max_mant_8 = sq_xsum_mant_9 >> 1;
		st->hw_max_exp_9 = sq_xsum_exp_9;
	} else {
		st->hw_max_mant_8 = sq_ysum_mant_9 >> 1;
		st->hw_max_exp_9 = sq_ysum_exp_9;
	}

#if TRX_SQ_DEBUG
	printf("st->hw_max_mant_8 = 0x%x\n", st->hw_max_mant_8);
	printf("st->hw_max_exp_9 = %d\n", st->hw_max_exp_9);
	printf("\n");
#endif


	/* log of the max, divided by 2, produces 8.9 positive */
	/* exponent contributes 8.1, table gives 8 lsbs of fraction */
	st->hw_log_max_d2_8_9 = (st->hw_max_exp_9 << 8) 
					| trx_log2_table[st->hw_max_mant_8 & 0x7f];
#if TRX_ERROR_CHECK
	/* check range */
	if (st->hw_log_max_d2_8_9 > (313 << 8)) {  /* > 156.5 */
		printf("st->hw_log_max_d2_8_9 out of range = 0x%x\n", st->hw_log_max_d2_8_9);
		exit(1);
	}
#endif


	/* log of w */
	/* --- uses 8 log table bits out for now, but needs 9 to match log of max */
	st->hw_log_w_6_9 = (st->hw_w_exp_6 << 9) 
					| (trx_log2_table[(st->hw_w_mant_19 >> 11) & 0x7f] << 1);
#if TRX_ERROR_CHECK
	/* check range */
	if (st->hw_log_w_6_9 > (49 << 9)) {
		printf("st->hw_log_w_6_9 out of range = 0x%x\n", st->hw_log_w_6_9);
		exit(1);
	}
#endif

	/* LOD, subtracting out bias */
	st->hw_lod_8_9s = (st->hw_log_max_d2_8_9 + st->hw_log_w_6_9 - (107 << 9));

#if TRX_ERROR_CHECK
	/* check range of LOD */
	if ((st->hw_lod_8_9s < (-107 << 9)) || (st->hw_lod_8_9s >= (197 << 8))) { 
					/* last term is 98.5 */
		printf("st->hw_lod_8_9s out of range = 0x%x\n", st->hw_lod_8_9s);
		exit(1);
	}
#endif
	
	/* trunc and clamp to 7.8 [-64,64) */
	st->hw_lod_7_8s = st->hw_lod_8_9s >> 1;
	if ((st->hw_lod_7_8s & 0xc000) == 0x8000)
		st->hw_lod_7_8s = ~0x3fff;	/* clamp neg. */
	else if ((st->hw_lod_7_8s & 0xc000) == 0x4000)
		st->hw_lod_7_8s = 0x3fff;	/* clamp pos. */


#if TRX_PRINT_INPUT_VALS
	printf("  pure hw LOD clamped to 7.8 = %f (float)\n", (((float) (st->hw_lod_7_8s)) / 256.0));
#endif



#ifdef HAL_CSIM
	if (st->hw_w_is_neg) {
		st->hw_lod_7_8s = 0;
	}
#else

	/* apply bias */
	st->hw_lodb_4_8 = st->hw_lod_7_8s + 
				(((st->lodbias & 0x20) ? (st->lodbias | ~0x1f) : (st->lodbias & 0x1f))
				<< 6);

	/* apply dither */
	if (st->tloddither) {
		switch (((st->y & 1) << 1) | (st->x & 1)) {
			case 0 : st->hw_lodb_4_8 += 0 << 6; break;
			case 1 : st->hw_lodb_4_8 += 2 << 6; break;
			case 2 : st->hw_lodb_4_8 += 3 << 6; break;
			case 3 : st->hw_lodb_4_8 += 1 << 6; break;
#ifndef KERNEL
			default : printf("bad case\n"); exit(1);
#endif
		}
	}

	/* clamp, bilinear_en */
	if (((st->hw_lodb_4_8 >> 6) < st->lodmin) || st->hw_w_is_neg) {
		st->hw_lodb_4_8 = st->lodmin << 6;		// clears the fraction
		st->hw_bilinear_en = st->tmagfilter;
	} else if ((st->hw_lodb_4_8 >> 6) >= st->lodmax) {
		st->hw_lodb_4_8 = st->lodmax << 6;		// clears the fraction
		st->hw_bilinear_en = st->tminfilter;
	} else {
		st->hw_bilinear_en = st->tminfilter;
	}

	if (st->lod_zerofrac)
		st->hw_lodb_frac_8 = 0;
	else
		st->hw_lodb_frac_8 = st->hw_lodb_4_8 & 0xff;


	st->hw_lodb0 = (st->hw_lodb_4_8 >> 8) & 1;

	st->hw_lodbi_4 = st->hw_lodb_4_8 >> 8;
	if (st->trilinear && ((!st->lod_odd && (st->hw_lodbi_4 & 1))
					|| (st->lod_odd && !(st->hw_lodbi_4 & 1)))) {
		/* increment */
		if (st->hw_lodbi_4 == 8)
			st->hw_lodbi_4 = 7;
		else
			st->hw_lodbi_4 += 1;
	}

#if TRX_ERROR_CHECK
	/* check range of LODBI */
	if (st->hw_lodbi_4 > 8) { 
		printf("st->hw_lodbi_4 out of range\n");
		exit(1);
	}
#endif
#endif // ifdef HAL_CSIM


}


#ifndef HAL_CSIM
/***************************************************************************/
/* detail factor - calculate 8 bit detail factor and put into st           */
void trx_detail_factor(TRX_STLOD_STRUCT* st)
{
	// min(detail_max, (detail_bias  - LOD) << detail_scale)
	// detail_bias is 6.0 signed
	// detail_scale is 8 bits
	// detail_max is 0.8 unsigned

	INT32 bias_minus_lod_8_7s;


	// shift and sign extend each to 8.7 signed
	bias_minus_lod_8_7s = (((((st->detail_bias & 0x20) ? 0xc0 : 0) | st->detail_bias) << 7)
			+ ~((((st->hw_lod_7_8s & 0x4000) << 1) | st->hw_lod_7_8s) >> 1)) & 0x7fff;

	if (st->hw_w_is_neg) {
		st->hw_detail_factor_8 = st->detail_max;
	} else if (bias_minus_lod_8_7s & 0x4000) {
		// sign bit is set
		st->hw_detail_factor_8 = 0;
	} else {
		st->hw_detail_factor_8 = bias_minus_lod_8_7s >> (7 - st->detail_scale);

		if (st->hw_detail_factor_8 > st->detail_max)
			st->hw_detail_factor_8 = st->detail_max;
	}

}
#endif // ifndef HAL_CSIM



// **********************************************************************
// **********************************************************************
// old trexfnc2.c inserted here:
/*
 * Contains these functions, protos are in trexfunc.h:
 *  UINT32 trx_addr_int(TRX_STLOD_STRUCT* st, UINT32 s_8, UINT32 t_8)
 *  void trx_addr_gen(TRX_STLOD_STRUCT* st)
 */


/***************************************************************************/
/* address interleave - form texture memory offset from s,t                */
UINT32 trx_addr_int(TRX_STLOD_STRUCT* st, UINT32 lod_4, UINT32 t_8, UINT32 s_8)
{
	INT32 s[8];
	INT32 t[8];
	INT32 e[8];
	INT32 i;
	INT32 t_ns;	/* not split texture (odd/even sets of levels */
	INT32 offs_15;	/* offset */

#define BIT_CONCAT(b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0) \
		(((b14) << 14) | ((b13) << 13) | ((b12) << 12) | ((b11) << 11) | \
		((b10) << 10) | ((b9) << 9) | ((b8) << 8) | ((b7) << 7) | \
		((b6) << 6) | ((b5) << 5) | ((b4) << 4) | ((b3) << 3) | \
		((b2) << 2) | ((b1) << 1) | ((b0) << 0))


	for (i = 0; i <= 7; i++) {
		s[i] = s_8 & 1;
		s_8 >>= 1;
	}

	for (i = 0; i <= 7; i++) {
		t[i] = t_8 & 1;
		t_8 >>= 1;
	}

	if (st->lod_aspect != 0) {
		if (st->lod_s_is_wider) {
			for (i = 0; i <= 7; i++)
				e[i] = s[i];
		} else {
			for (i = 0; i <= 7; i++)
				e[i] = t[i];
		}
	}

	t_ns = st->lod_tsplit ? 0 : 1;

	/* these will translate almost straight into verilog */
	switch (st->lod_aspect) {
		case 0 : 
			/* 1x1 */
			switch (lod_4) {
case 0 : offs_15 = BIT_CONCAT(   0,t[7],s[7],t[6],s[6],t[5],s[5],t[4],s[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 1 : offs_15 = BIT_CONCAT(t_ns,   0,   0,t[6],s[6],t[5],s[5],t[4],s[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 2 : offs_15 = BIT_CONCAT(   1,   0,t_ns,   0,   0,t[5],s[5],t[4],s[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 3 : offs_15 = BIT_CONCAT(t_ns,   0,   1,   0,t_ns,   0,   0,t[4],s[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 4 : offs_15 = BIT_CONCAT(   1,   0,t_ns,   0,   1,   0,t_ns,   0,   0,t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 5 : offs_15 = BIT_CONCAT(t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   0,t[2],s[2],t[1],s[1]);
break;
case 6 : offs_15 = BIT_CONCAT(   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   0,t[1],s[1]);
break;
case 7 : offs_15 = BIT_CONCAT(t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   0);
break;
case 8 : offs_15 = BIT_CONCAT(   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns);
break;
#ifndef KERNEL
				default : printf("bad case\n"); exit(1);
#endif
			}
			break;
		case 1 :
			/* 2x1 */
			switch (lod_4) {
case 0 : offs_15 = BIT_CONCAT(   0,   0,e[7],t[6],s[6],t[5],s[5],t[4],s[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 1 : offs_15 = BIT_CONCAT(   0,t_ns,   0,   0,e[6],t[5],s[5],t[4],s[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 2 : offs_15 = BIT_CONCAT(   0,   1,   0,t_ns,   0,   0,e[5],t[4],s[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 3 : offs_15 = BIT_CONCAT(   0,t_ns,   0,   1,   0,t_ns,   0,   0,e[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 4 : offs_15 = BIT_CONCAT(   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   0,e[3],t[2],s[2],t[1],s[1]);
break;
case 5 : offs_15 = BIT_CONCAT(   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   0,e[2],t[1],s[1]);
break;
case 6 : offs_15 = BIT_CONCAT(   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   0,e[1]);
break;
case 7 : offs_15 = BIT_CONCAT(   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0);
break;
case 8 : offs_15 = BIT_CONCAT(   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   1);
break;
#ifndef KERNEL
				default : printf("bad case\n"); exit(1);
#endif
			}
			break;

		case 2 :
			/* 4x1 */
			switch (lod_4) {
case 0 : offs_15 = BIT_CONCAT(   0,   0,   0,e[7],e[6],t[5],s[5],t[4],s[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 1 : offs_15 = BIT_CONCAT(   0,   0,t_ns,   0,   0,e[6],e[5],t[4],s[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 2 : offs_15 = BIT_CONCAT(   0,   0,   1,   0,t_ns,   0,   0,e[5],e[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 3 : offs_15 = BIT_CONCAT(   0,   0,t_ns,   0,   1,   0,t_ns,   0,   0,e[4],e[3],t[2],s[2],t[1],s[1]);
break;
case 4 : offs_15 = BIT_CONCAT(   0,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   0,e[3],e[2],t[1],s[1]);
break;
case 5 : offs_15 = BIT_CONCAT(   0,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   0,e[2],e[1]);
break;
case 6 : offs_15 = BIT_CONCAT(   0,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,e[1]);
break;
case 7 : offs_15 = BIT_CONCAT(   0,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,t_ns,   0);
break;
case 8 : offs_15 = BIT_CONCAT(   0,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   1,t_ns);
break;
#ifndef KERNEL
				default : printf("bad case\n"); exit(1);
#endif
			}
			break;

		case 3 :
			/* 8x1 */
			switch (lod_4) {
case 0 : offs_15 = BIT_CONCAT(   0,   0,   0,   0,e[7],e[6],e[5],t[4],s[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 1 : offs_15 = BIT_CONCAT(   0,   0,   0,t_ns,   0,   0,e[6],e[5],e[4],t[3],s[3],t[2],s[2],t[1],s[1]);
break;
case 2 : offs_15 = BIT_CONCAT(   0,   0,   0,   1,   0,t_ns,   0,   0,e[5],e[4],e[3],t[2],s[2],t[1],s[1]);
break;
case 3 : offs_15 = BIT_CONCAT(   0,   0,   0,t_ns,   0,   1,   0,t_ns,   0,   0,e[4],e[3],e[2],t[1],s[1]);
break;
case 4 : offs_15 = BIT_CONCAT(   0,   0,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   0,e[3],e[2],e[1]);
break;
case 5 : offs_15 = BIT_CONCAT(   0,   0,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,e[2],e[1]);
break;
case 6 : offs_15 = BIT_CONCAT(   0,   0,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,t_ns,   0,e[1]);
break;
case 7 : offs_15 = BIT_CONCAT(   0,   0,   0,t_ns,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   1,t_ns,   0);
break;
case 8 : offs_15 = BIT_CONCAT(   0,   0,   0,   1,   0,t_ns,   0,   1,   0,t_ns,   0,   1,t_ns,   1,t_ns);
break;
#ifndef KERNEL
				default : printf("bad case\n"); exit(1);
#endif
			}
			break;

#ifndef KERNEL
		default : printf("bad case\n"); exit(1);
#endif
	}

	return(offs_15 & 0x7fff);
}






/***************************************************************************/
/* hardware bit accurate address calc.                                     */
/* uses s,t,lodbi to calculate address and bilinear blend weights          */
/* follows psuedo-code in spec                                             */
void trx_addr_gen(TRX_STLOD_STRUCT* st)
{
	INT32 tbl_lod_4 = -1;
	INT32 ras_mask;

	INT32 small_tex_mask;
	INT32 wr_data_int;

	INT32 tex_wr_addr_f8;


  	if (st->seq_8_downld)
		 tex_wr_addr_f8 = (st->tex_wr_addr & ~0x1ff) | ((st->tex_wr_addr & 0x0ff) << 1);
	 else
		 tex_wr_addr_f8 = st->tex_wr_addr;


  	if (st->tex_yiq_write && st->is_tex_write && st->tdirect_write) {
		// is a direct texture write

		// 15 bit offset, lsb selects which 32-bit word to write
		st->offs_00_15 = (tex_wr_addr_f8 >> (1 + 2)) & 0x7fff;
		st->offs_01_15 = st->offs_00_15;
		st->offs_10_15 = st->offs_00_15;
		st->offs_11_15 = st->offs_00_15;
	
	} else {
		if (st->tex_yiq_write && st->is_tex_write) {
			// is a texture write, not direct_write, so set lod, t, s as indicated in PCI address
			st->hw_s0_8 = ((tex_wr_addr_f8 >> (2)) & 0x7f) << 1;	// s[0] is not in pci address
			st->hw_s1_8 = st->hw_s0_8;
			st->hw_wt_s1_1_4 = -1;
			st->hw_t0_8 = (tex_wr_addr_f8 >> (7 + 2)) & 0xfe; // t[0] masked - don't care
			st->hw_t1_8 = st->hw_t0_8;
			st->hw_wt_t1_1_4 = -1;

			tbl_lod_4 = (tex_wr_addr_f8 >> (8 + 7 + 2)) & 0xf;

		} else {
			// not a texture write

			/*** s ***/
			st->hw_s_sh_8_4 =  (FX_LO64(st->hw_s_fxd_12_i64) >> st->hw_lodbi_4) & 0xfff;
			st->hw_s_frac_4 = st->hw_s_sh_8_4 & 0xf;

			/* if (st->hw_w_is_neg && st->tprojected) { 7/6/95 */
//			if (st->hw_w_is_neg && st->tclampw) {
//				/* force to center of 0 */
//				st->hw_s_dec_8_4 = 0;
//				st->hw_s_inc_8_4 = 0;
//				st->hw_s_frac_4 = 0x8;

			if (st->hw_bilinear_en) {
				/* bilinear */
				st->hw_s_dec_8_4 = st->hw_s_sh_8_4 - 8;
				st->hw_s_inc_8_4 = st->hw_s_sh_8_4 + 8;
				/* st->hw_s_frac_4 is untouched */
			} else {
				/* point sampled */
				st->hw_s_dec_8_4 = st->hw_s_sh_8_4;
				st->hw_s_inc_8_4 = st->hw_s_sh_8_4;
				st->hw_s_frac_4 = 0x8;
			}

			if (	(st->tclamps && (st->hw_s_fxd_is_neg || (st->hw_s_dec_8_4 < 0))
											&& !st->hw_s_fxd_clmp_pos)
					|| (st->hw_w_is_neg && st->tclampw)
					|| (st->hw_lodbi_4 == 8)
					|| ((st->hw_lodbi_4 == 7) && !st->lod_s_is_wider && (st->lod_aspect >= 1))
					|| ((st->hw_lodbi_4 == 6) && !st->lod_s_is_wider && (st->lod_aspect >= 2))
					|| ((st->hw_lodbi_4 == 5) && !st->lod_s_is_wider && (st->lod_aspect >= 3))
					) {
				/* clamp to 0 */
				st->hw_s0_8 = 0;
				st->hw_s1_8 = 0;	/* don't care */
				st->hw_wt_s1_1_4 = 0;
			} else if (st->tclamps && (st->hw_s_fxd_clmp_pos || ((st->hw_s_inc_8_4 & 0xff0)
							& (~0xfff >> (st->hw_lodbi_4 + (st->lod_s_is_wider ? 0 : st->lod_aspect)))))) {
				/* clamp to pos. max */
				st->hw_s0_8 = 0xff;	/* don't care */
				st->hw_s1_8 = 0xff;
				st->hw_wt_s1_1_4 = 0x10;
			} else {
				/* no clamp */

				/* address and texel weight swizzling */
				if (!(st->hw_s_dec_8_4 & 0x10)) {
					/* hw_s_dec_8_4 is even */
					st->hw_s0_8 = st->hw_s_dec_8_4 >> 4;
					st->hw_s1_8 = st->hw_s_inc_8_4 >> 4;
					st->hw_wt_s1_1_4 = st->hw_s_frac_4 ^ 0x8;
				} else {
					/* hw_s_dec_8_4 is odd */
					st->hw_s0_8 = st->hw_s_inc_8_4 >> 4;
					st->hw_s1_8 = st->hw_s_dec_8_4 >> 4;
					st->hw_wt_s1_1_4 = 0x10 - (st->hw_s_frac_4 ^ 0x8);
				}
			}


			/*** t ***/
			st->hw_t_sh_8_4 =  (FX_LO64(st->hw_t_fxd_12_i64) >> st->hw_lodbi_4) & 0xfff;
			st->hw_t_frac_4 = st->hw_t_sh_8_4 & 0xf;

			/* if (st->hw_w_is_neg && st->tprojected) { 7/6/95 */
//			if (st->hw_w_is_neg && st->tclampw) {
//				/* force to center of 0 */
//				st->hw_t_dec_8_4 = 0;
//				st->hw_t_inc_8_4 = 0;
//				st->hw_t_frac_4 = 0x8;
//			} else if (st->hw_bilinear_en) {
			if (st->hw_bilinear_en) {
				/* bilinear */
				st->hw_t_dec_8_4 = st->hw_t_sh_8_4 - 8;
				st->hw_t_inc_8_4 = st->hw_t_sh_8_4 + 8;
				/* st->hw_t_frac_4 is untouched */
			} else {
				/* point sampled */
				st->hw_t_dec_8_4 = st->hw_t_sh_8_4;
				st->hw_t_inc_8_4 = st->hw_t_sh_8_4;
				st->hw_t_frac_4 = 0x8;
			}

			if (	(st->tclampt && (st->hw_t_fxd_is_neg || (st->hw_t_dec_8_4 < 0))
											&& !st->hw_t_fxd_clmp_pos)
					|| (st->hw_w_is_neg && st->tclampw)
					|| (st->hw_lodbi_4 == 8)
					|| ((st->hw_lodbi_4 == 7) && st->lod_s_is_wider && (st->lod_aspect >= 1))
					|| ((st->hw_lodbi_4 == 6) && st->lod_s_is_wider && (st->lod_aspect >= 2))
					|| ((st->hw_lodbi_4 == 5) && st->lod_s_is_wider && (st->lod_aspect >= 3))
					) {
				/* clamp to 0 */
				st->hw_t0_8 = 0;
				st->hw_t1_8 = 0;	/* don't care */
				st->hw_wt_t1_1_4 = 0;
			} else if (st->tclampt && (st->hw_t_fxd_clmp_pos || (st->hw_t_inc_8_4 
							& (~0xfff << (8 - st->hw_lodbi_4 - (!st->lod_s_is_wider ? 
							0 : st->lod_aspect)))))) {
				/* clamp to pos. max */
				st->hw_t0_8 = 0xff;	/* don't care */
				st->hw_t1_8 = 0xff;
				st->hw_wt_t1_1_4 = 0x10;
			} else {
				/* no clamp */

				/* address and texel weight swizzling */
				if (!(st->hw_t_dec_8_4 & 0x10)) {
					/* hw_t_dec_8_4 is even */
					st->hw_t0_8 = st->hw_t_dec_8_4 >> 4;
					st->hw_t1_8 = st->hw_t_inc_8_4 >> 4;
					st->hw_wt_t1_1_4 = st->hw_t_frac_4 ^ 0x8;
				} else {
					/* hw_t_dec_8_4 is odd */
					st->hw_t0_8 = st->hw_t_inc_8_4 >> 4;
					st->hw_t1_8 = st->hw_t_dec_8_4 >> 4;
					st->hw_wt_t1_1_4 = 0x10 - (st->hw_t_frac_4 ^ 0x8);
				}
			}

			tbl_lod_4 = st->hw_lodbi_4;
		}

		/* form offset from lod,s,t for texel read or write that is not direct_write */
		st->offs_00_15 = trx_addr_int(st, tbl_lod_4, st->hw_t0_8, st->hw_s0_8);
		st->offs_01_15 = trx_addr_int(st, tbl_lod_4, st->hw_t0_8, st->hw_s1_8);
		st->offs_10_15 = trx_addr_int(st, tbl_lod_4, st->hw_t1_8, st->hw_s0_8);
		st->offs_11_15 = trx_addr_int(st, tbl_lod_4, st->hw_t1_8, st->hw_s1_8);
	}



	if (st->rgn_mem_data_size			// 4 256k x 8 (1MByte) config., 8-bit texel only
			|| (st->tex_yiq_write && st->is_tex_write && st->tdirect_write)
												// direct write
			|| (st->tformat & 0x8)) {	// 16-bit texture
		// no offs shift, force byte select to 0
		st->mem_byte_sel00 = 0;
		st->mem_byte_sel01 = 0;
		st->mem_byte_sel10 = 0;
		st->mem_byte_sel11 = 0;
	} else {
		// is 8-bit texture
		// shift right 1 bit, saving LSB for byte select
		st->mem_byte_sel00 = st->offs_00_15 & 1;
		st->offs_00_15 >>= 1;
		st->mem_byte_sel01 = st->offs_01_15 & 1;
		st->offs_01_15 >>= 1;
		st->mem_byte_sel10 = st->offs_10_15 & 1;
		st->offs_10_15 >>= 1;
		st->mem_byte_sel11 = st->offs_11_15 & 1;
		st->offs_11_15 >>= 1;
	}


	// select the base address and add offset
	if (!st->tmultibaseaddr || (st->hw_lodbi_4 == 0)
				|| st->tex_yiq_write) {
		st->base_addr_19 = st->texbaseaddr;
	} else if (st->hw_lodbi_4 == 1) {
		st->base_addr_19 = st->texbaseaddr1;
	} else if (st->hw_lodbi_4 == 2) {
		st->base_addr_19 = st->texbaseaddr2;
	} else {
		st->base_addr_19 = st->texbaseaddr3_8;
	}

	st->mem_addr_00_19 = st->base_addr_19 + st->offs_00_15;
	st->mem_addr_01_19 = st->base_addr_19 + st->offs_01_15;
	st->mem_addr_10_19 = st->base_addr_19 + st->offs_10_15;
	st->mem_addr_11_19 = st->base_addr_19 + st->offs_11_15;


	switch (st->rgn_page_size) {
		case 0 : 
			// 8 bits
			st->mem_row_00_9 = (st->mem_addr_00_19 >> 8) & 0x1ff;
			st->mem_row_01_9 = (st->mem_addr_01_19 >> 8) & 0x1ff;
			st->mem_row_10_9 = (st->mem_addr_10_19 >> 8) & 0x1ff;
			st->mem_row_11_9 = (st->mem_addr_11_19 >> 8) & 0x1ff;

			st->mem_col_00_int_9 = st->mem_addr_00_19 & 0xff;
			st->mem_col_01_int_9 = st->mem_addr_01_19 & 0xff;
			st->mem_col_10_int_9 = st->mem_addr_10_19 & 0xff;
			st->mem_col_11_int_9 = st->mem_addr_11_19 & 0xff;
		break;

		case 1 : 
			// 9 bits
			st->mem_row_00_9 = (st->mem_addr_00_19 >> 9) & 0x1ff;
			st->mem_row_01_9 = (st->mem_addr_01_19 >> 9) & 0x1ff;
			st->mem_row_10_9 = (st->mem_addr_10_19 >> 9) & 0x1ff;
			st->mem_row_11_9 = (st->mem_addr_11_19 >> 9) & 0x1ff;

			st->mem_col_00_int_9 = st->mem_addr_00_19 & 0x1ff;
			st->mem_col_01_int_9 = st->mem_addr_01_19 & 0x1ff;
			st->mem_col_10_int_9 = st->mem_addr_10_19 & 0x1ff;
			st->mem_col_11_int_9 = st->mem_addr_11_19 & 0x1ff;
		break;

		case 2 : 
			// 10 bits
#ifndef KERNEL
			printf("rgn_page_size of 10 bits not supported\n"); 
			exit(1);
#endif
		break;

#ifndef KERNEL
		default : 
			printf("bad rgn_page_size\n"); 
			exit(1);
#endif
	}


	if (!st->rgn_sec_ras_en) {
		// one bank
		st->mem_ras0 = 1;
	} else {
		// two banks
		switch (st->rgn_sec_ras_bit) {
			case 0 : ras_mask = (1 << 18); break;
			case 1 : ras_mask = (1 << 17); break;
#ifndef KERNEL
			default : printf("bad rgn_sec_ras_bit\n"); exit(1);
#endif
		}

		st->mem_ras0 = (st->mem_addr_00_19 & ras_mask) ? 0 : 1;

#if TRX_ERROR_CHECK
		if (		((st->mem_addr_00_19 & ras_mask) != (st->mem_addr_01_19 & ras_mask))
				|| ((st->mem_addr_00_19 & ras_mask) != (st->mem_addr_10_19 & ras_mask))
				|| ((st->mem_addr_00_19 & ras_mask) != (st->mem_addr_11_19 & ras_mask))
				) {
#ifndef KERNEL
			printf("texel access spans two banks\n"); 
			exit(1);
#endif
		}
#endif
	}


	// cas and write data
	small_tex_mask = -10; // init bogus value

	if (!st->tex_yiq_write) {
		// is a read, so all cas's
		st->mem_cas0 = 1;
		st->mem_cas1 = 1;
		st->mem_cas2 = 1;
		st->mem_cas3 = 1;

	} else {
		if (!st->is_tex_write) {
			// yiq write, so no cas's
			st->mem_cas0 = 0;
			st->mem_cas1 = 0;
			st->mem_cas2 = 0;
			st->mem_cas3 = 0;

		} else {
			if (st->tdirect_write) {

				// is a direct texture write
				if (! ((tex_wr_addr_f8 >> (2)) & 0x1)) {
					st->mem_cas0 = 1;
					st->mem_cas1 = 1;
					st->mem_cas2 = 0;
					st->mem_cas3 = 0;
				} else {
					st->mem_cas0 = 0;
					st->mem_cas1 = 0;
					st->mem_cas2 = 1;
					st->mem_cas3 = 1;
				}
			} else {
				// is a texture write, not direct

				// mask half of write for small 8-bit textures (low or high bytes, as 
				//        a function of byte sel)
				small_tex_mask = !(st->tformat & 0x8) && (
								(tbl_lod_4 == 8)
							|| (tbl_lod_4 == 7)
							|| ((tbl_lod_4 == 6) && !st->lod_s_is_wider && (st->lod_aspect >= 1))
							|| ((tbl_lod_4 == 5) && !st->lod_s_is_wider && (st->lod_aspect >= 2))
							|| ((tbl_lod_4 == 4) && !st->lod_s_is_wider && (st->lod_aspect >= 3))
						);

				if (! ((tex_wr_addr_f8 >> (7 + 2)) & 0x1)) {
					// t[0] = 0
					st->mem_cas0 = !small_tex_mask || !st->mem_byte_sel00;
					st->mem_cas1 = !small_tex_mask || st->mem_byte_sel00;
					st->mem_cas2 = 0;
					st->mem_cas3 = 0;
				} else {
					// t[0] = 1
					st->mem_cas0 = 0;
					st->mem_cas1 = 0;
					st->mem_cas2 = !small_tex_mask || !st->mem_byte_sel00;
					st->mem_cas3 = !small_tex_mask || st->mem_byte_sel00;
				}
			}
		}
	}

	if (st->rgn_mem_data_size) {
		// low bytes only, so no cas1,3
		st->mem_cas1 = 0;
		st->mem_cas3 = 0;
	}


	// load column addresses
	if (st->mem_cas0 || st->mem_cas1) {
		st->mem_col_00_9 = st->mem_col_00_int_9;
		st->mem_col_01_9 = st->mem_col_01_int_9;
	}
	if (st->mem_cas2 || st->mem_cas3) {
		st->mem_col_10_9 = st->mem_col_10_int_9;
		st->mem_col_11_9 = st->mem_col_11_int_9;
	}

	// load data
	if (st->tex_yiq_write) {

		// assemble the write data
		if (!st->tdirect_write && (! (st->tformat & 0x8))) {
			// 8-bit, not direct write, swap middle two bytes
			wr_data_int = (st->tex_wr_data & 0xff0000ff)
					  		| ((st->tex_wr_data << 8) & 0x00ff0000)
					  		| ((st->tex_wr_data >> 8) & 0x0000ff00);

#if TRX_ERROR_CHECK
			if (small_tex_mask == -10) {
				printf("using uninitialized small_tex_mask\n"); 
				exit(1);
			}
#endif
			if (small_tex_mask && st->mem_byte_sel00) {
				// 2xN or 1xN write to upper bytes, so hi-lo byte swap
				wr_data_int = ((wr_data_int << 8) & 0xff00ff00)
					  			| ((wr_data_int >> 8) & 0x00ff00ff);
			}
		} else {
			// 16-bit or direct write
			wr_data_int = st->tex_wr_data;
		}


		if (st->mem_cas0) {
			// load low bytes
			st->mem_wr_data_00_16 &= ~0xffff00ff; 
			st->mem_wr_data_00_16 |= 0x00ff & wr_data_int; 

			st->mem_wr_data_01_16 &= ~0xffff00ff; 
			st->mem_wr_data_01_16 |= 0x00ff & (wr_data_int >> 16); 
		}

		if (st->mem_cas1) {
			// load high bytes
			st->mem_wr_data_00_16 &= ~0xffffff00; 
			st->mem_wr_data_00_16 |= 0xff00 & wr_data_int; 

			st->mem_wr_data_01_16 &= ~0xffffff00; 
			st->mem_wr_data_01_16 |= 0xff00 & (wr_data_int >> 16); 
		}

		if (st->mem_cas2) {
			// load low bytes
			st->mem_wr_data_10_16 &= ~0xffff00ff; 
			st->mem_wr_data_10_16 |= 0x00ff & wr_data_int; 

			st->mem_wr_data_11_16 &= ~0xffff00ff; 
			st->mem_wr_data_11_16 |= 0x00ff & (wr_data_int >> 16); 
		}

		if (st->mem_cas3) {
			// load high bytes
			st->mem_wr_data_10_16 &= ~0xffffff00; 
			st->mem_wr_data_10_16 |= 0xff00 & wr_data_int; 

			st->mem_wr_data_11_16 &= ~0xffffff00; 
			st->mem_wr_data_11_16 |= 0xff00 & (wr_data_int >> 16); 
		}
	}

}
