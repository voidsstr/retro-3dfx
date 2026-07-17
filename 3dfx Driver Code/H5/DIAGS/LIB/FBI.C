/*
 * Library Routines for FBI-specific tests
 * 
 * $Log: 
 *  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
 *       branching.
 *  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
 * $
** 
** 15    1/08/99 10:34a Zelsnack
** added alphablend subtract modes
** 
** 13    12/10/98 8:42a Tarolli
** 
** 12    11/24/98 9:23p Tarolli
** added new SAMECOLOR alpha modes
** 
** 11    11/23/98 3:05p Tarolli
** fixed ablend for 32bpp mode
** 
** 10    11/22/98 10:15p Tarolli
** 32 bpp diags fixes
 * 
 * 6     8/01/97 8:20a Tarolli
 * added alpahblend dither subtract code
 * 
 * 5     3/15/97 6:50p Tarolli
 * added LFB fixes
 * 
 * 3     3/04/97 5:27p Tarolli
 * fixed SST_ALOCAL_W bug in fbzColorPath
 * 
 * 2     2/27/97 1:37p Tarolli
 * HSIM restructure and cleanup
 * 
 * 1     2/25/97 8:59a Tarolli
 * 
 * 4     9/02/95 8:14a Tarolli
 * got rid of warnings for clean compile
 * 
 * 3     8/25/95 1:40a Jdt
 * Hacked out a bunch of functions that wouldn't 
 * compile under msvc.  These were functions that
 * were not used for any diags.  They were only
 * used by the hardware folks for their own dark purposes.
 * 
 * 2     8/24/95 5:22p Sellers
 * Modified dither routined to perform 2x2 dithering
 * Modified z_iter input of CCombine to be a "ulong"
 * Added structure element "inc_xor_out" in CCombineStruct
 * Added other miscellaneous functions...
 * Revision 1.1  1995/07/31  17:56:27  sellers
 * Initial revision
 *
 *
 */

#include "udiag.h"
#include "fbi.h"

/*************************************
 * Depth and Alpha Test Function
 *		- Returns 1 if test passes
 *		- Returns 0 if test fails
 ************************************/
ulong adepth_function(src, dst, func)
	ulong src, dst, func;
{
	ulong gt = (src > dst) ? 1 : 0;
	ulong lt = (src < dst) ? 1 : 0;
	ulong eq = (src == dst) ? 1 : 0;
	ulong ret_val;

	switch(func) {
		case 0:					// NEVER
			ret_val = 0;
			break;
		case 1:					// LT
			ret_val = lt;
			break;
		case 2:					// EQ
			ret_val = eq;
			break;
		case 3:					// LTE
			ret_val = (lt | eq);
			break;
		case 4:					// GT
			ret_val = gt;
			break;
		case 5:					// NEQ
			ret_val = eq ^ 0x1;
			break;
		case 6:					// GTE
			ret_val = (gt | eq);
			break;
		case 7:					// ALWAYS
			ret_val = 1;
			break;
		default:
			GDBG_ERROR("adepth_function", "Unknown function %d...\n", func);
			DIAG_FAIL();
			break;
	}
	GDBG_INFO(110,"adepth_function(): SRC:0x%x DST:0x%x FUNC:0x%x (%d %d %d) ret:%d\n",
		src, dst, func, gt, lt, eq, ret_val);
	return(ret_val);
}

/*************************************
 * alpha blend
 ************************************/
void ablend(blendmode, ditsub, dm, src, src_pre_fog, dst, expect)
	ablendMode *blendmode;
	int ditsub, dm;
	fbiColors *src;
	fbiColors *src_pre_fog;
	fbiColors *dst;
	fbiColors *expect;
{
	short alpha_src_red, alpha_src_green, alpha_src_blue;
	short alpha_dst_red, alpha_dst_green, alpha_dst_blue;
	short p_src_red, p_src_blue, p_src_green;
	short p_dst_red, p_dst_blue, p_dst_green;
	short dstR, dstG, dstB;
	
	if(!blendmode->ablend_en) {
		expect->red = src->red;
		expect->green = src->green;
		expect->blue = src->blue;
		return;
	}

	dstR = dst->red;
	dstG = dst->green;
	dstB = dst->blue;
	if (ditsub) {
	    int tmp = 8-dm;			// subtract out dither matrix
	    dstR += tmp>>1;
	    dstG += tmp>>(diago.rgb==16?2:1);
	    dstB += tmp>>1;
	    if (dstR<0) dstR = 0;		// clamp low
	    if (dstG<0) dstG = 0;
	    if (dstB<0) dstB = 0;
	}
	if (diago.rgb < 32) {
	    dstR += dstR >> 5;			// add in MSBS, expand to 0xFF
	    dstG += dstG >> (diago.rgb==16?6:5);
	    dstB += dstB >> 5;
	}
	if (dstR > 0xFF) dstR = 0xFF;		// and clamp high
	if (dstG > 0xFF) dstG = 0xFF;
	if (dstB > 0xFF) dstB = 0xFF;

	switch(blendmode->alpha_fact_src) {
		case 0:
			alpha_src_red = 0x0;
			alpha_src_green = 0x0;
			alpha_src_blue = 0x0;
			break;
		case 1:
			alpha_src_red = src->alpha;
			alpha_src_green = src->alpha;
			alpha_src_blue = src->alpha;
			break;
		case 2:
			alpha_src_red = dstR;
			alpha_src_green = dstG;
			alpha_src_blue = dstB;
			break;
		case 3:
			alpha_src_red = dst->alpha;
			alpha_src_green = dst->alpha;
			alpha_src_blue = dst->alpha;
			break;
		case 4:
			alpha_src_red = 0xff;
			alpha_src_green = 0xff;
			alpha_src_blue = 0xff;
			break;
		case 5:
			alpha_src_red = (~(src->alpha)) & 0xff;
			alpha_src_green = (~(src->alpha)) & 0xff;
			alpha_src_blue = (~(src->alpha)) & 0xff;
			break;
		case 6:
			alpha_src_red = (~dstR) & 0xff;
			alpha_src_green = (~dstG) & 0xff;
			alpha_src_blue = (~dstB) & 0xff;
			break;
		case 7:
			alpha_src_red = (~(dst->alpha)) & 0xff;
			alpha_src_green = (~(dst->alpha)) & 0xff;
			alpha_src_blue = (~(dst->alpha)) & 0xff;
			break;
		case 8:
			alpha_src_red = src->red;
			alpha_src_green = src->green;
			alpha_src_blue = src->blue;
			break;
		case 9:
			alpha_src_red = (~src->red) & 0xff;
			alpha_src_green = (~src->green) & 0xff;
			alpha_src_blue = (~src->blue) & 0xff;
			break;
		case 15:
			if(src->alpha > ((~(dst->alpha)) & 0xff)) {
				alpha_src_red = (~(dst->alpha)) & 0xff;
				alpha_src_green = (~(dst->alpha)) & 0xff;
				alpha_src_blue = (~(dst->alpha)) & 0xff;
			} else {
				alpha_src_red = src->alpha;
				alpha_src_green = src->alpha;
				alpha_src_blue = src->alpha;
			}
			break;
		default:
			GDBG_ERROR("ablend", "Unexpected alpha_fact_src of %d\n",
				blendmode->alpha_fact_src);
			DIAG_FAIL();
			break;
	}
	switch(blendmode->alpha_fact_dst) {
		case 0:
			alpha_dst_red = 0x0;
			alpha_dst_green = 0x0;
			alpha_dst_blue = 0x0;
			break;
		case 1:
			alpha_dst_red = src->alpha;
			alpha_dst_green = src->alpha;
			alpha_dst_blue = src->alpha;
			break;
		case 2:
			alpha_dst_red = src->red;
			alpha_dst_green = src->green;
			alpha_dst_blue = src->blue;
			break;
		case 3:
			alpha_dst_red = dst->alpha;
			alpha_dst_green = dst->alpha;
			alpha_dst_blue = dst->alpha;
			break;
		case 4:
			alpha_dst_red = 0xff;
			alpha_dst_green = 0xff;
			alpha_dst_blue = 0xff;
			break;
		case 5:
			alpha_dst_red = (~(src->alpha)) & 0xff;
			alpha_dst_green = (~(src->alpha)) & 0xff;
			alpha_dst_blue = (~(src->alpha)) & 0xff;
			break;
		case 6:
			alpha_dst_red = (~(src->red)) & 0xff;
			alpha_dst_green = (~(src->green)) & 0xff;
			alpha_dst_blue = (~(src->blue)) & 0xff;
			break;
		case 7:
			alpha_dst_red = (~(dst->alpha)) & 0xff;
			alpha_dst_green = (~(dst->alpha)) & 0xff;
			alpha_dst_blue = (~(dst->alpha)) & 0xff;
			break;
		case 8:
			alpha_dst_red = dstR;
			alpha_dst_green = dstG;
			alpha_dst_blue = dstB;
			break;
		case 9:
			alpha_dst_red = (~dstR) & 0xff;
			alpha_dst_green = (~dstG) & 0xff;
			alpha_dst_blue = (~dstB) & 0xff;
			break;
		case 15:
			alpha_dst_red = src_pre_fog->red;
			alpha_dst_green = src_pre_fog->green;
			alpha_dst_blue = src_pre_fog->blue;
			break;
		default:
			GDBG_ERROR("ablend", "Unexpected alpha_fact_dst of %d\n",
				blendmode->alpha_fact_dst);
			DIAG_FAIL();
			break;
	}

	/* Bias alpha values */
	alpha_src_red++;
	alpha_src_green++;
	alpha_src_blue++;
	alpha_dst_red++;
	alpha_dst_green++;
	alpha_dst_blue++;

	/* Multiply colors by alpha values */
	p_src_red = ((src->red * alpha_src_red) >> 8) & 0xff;
	p_src_green = ((src->green * alpha_src_green) >> 8) & 0xff;
	p_src_blue = ((src->blue * alpha_src_blue) >> 8) & 0xff;
	p_dst_red = ((dstR * alpha_dst_red) >> 8) & 0xff;
	p_dst_green = ((dstG * alpha_dst_green) >> 8) & 0xff;
	p_dst_blue = ((dstB * alpha_dst_blue) >> 8) & 0xff;

	/* Add */
	if (blendmode->subtract) {
	    p_src_red -= p_dst_red;
	    p_src_green -= p_dst_green;
	    p_src_blue -= p_dst_blue;
	    if (blendmode->reverse) {
		p_src_red = -p_src_red;
		p_src_green = -p_src_green;
		p_src_blue = -p_src_blue;
	    }
	}
	else {
	    p_src_red += p_dst_red;
	    p_src_green += p_dst_green;
	    p_src_blue += p_dst_blue;
	}

	/* Clamp */
	if(p_src_red < 0) p_src_red = 0;
	if(p_src_green < 0) p_src_green = 0;
	if(p_src_blue < 0) p_src_blue = 0;
	if(p_src_red > 255) p_src_red = 255;
	if(p_src_green > 255) p_src_green = 255;
	if(p_src_blue > 255) p_src_blue = 255;

	expect->red = p_src_red;
	expect->green = p_src_green;
	expect->blue = p_src_blue;

	return;
}

/*************************************
 * Fog 
 ************************************/
void fog(fogmode, x,y, src, fog_color, fog_table, a_iter, z_iter, w_iter_exp,
	w_iter_mant, expect)
	fogMode *fogmode;
	fbiColors *src;
	fbiColors *fog_color;
	ulong *fog_table;
	short a_iter;
	short z_iter;
	short w_iter_exp;
	short w_iter_mant;
	fbiColors *expect;
{
	short beta, delta_beta;
	fbiColors fog_color_mux, fog_src_mux, fog_src;

	GDBG_INFO(100,"FOG(): EN:%d MULT:%d ADD:%d CONST:%d ALPHA:%d Z:%d DIT:%d ZONE:%d\n",
		fogmode->fog_enable, fogmode->fog_mult, fogmode->fog_add,
		fogmode->fog_constant, fogmode->fog_alpha, fogmode->fog_z,
		fogmode->fog_dither, fogmode->fog_zones);
	GDBG_INFO(100,"\tW_MANT:0x%x W_EXP:0x%x A_ITER:0x%x Z_ITER:0x%x \n",
		w_iter_mant, w_iter_exp, a_iter, z_iter);
	expect->alpha = src->alpha;
	if(!fogmode->fog_enable) {
		/* Passthrough */
		expect->red = src->red;
		expect->green = src->green;
		expect->blue = src->blue;
		return;
	}

	/* Calculate fog beta factor */
	if(fogmode->fog_z && fogmode->fog_alpha) 
		beta = w_iter_exp;	// GMT: hack
	else if(fogmode->fog_z) 
		beta = z_iter;		/* assume 0 <= z_iter < 256 */
	else if(fogmode->fog_alpha)
		beta = a_iter;		/* assume 0 <= a_iter < 256 */
	else {
		/* Calculate beta using fog table */
		/* Assume w_iter[11:0], w_exp[3:0] */
		ulong index = ((w_iter_mant >> 10) & 0x3) | ((w_iter_exp & 0xf) << 2);
		ulong delta_index = (w_iter_mant >> 2) & 0xff;
		long hold;

#if 0
		printf("index:0x%x  delta_index:0x%x  data:0x%x\n",
			index, delta_index, fog_table[index]);
		printf("w_iter_mant:0x%x w_iter_exp:0x%x shift:0x%x ver_shift:0x%x\n",
			w_iter_mant, w_iter_exp, (w_iter_mant >> 10), (w_iter_mant >> 2));
		fflush(stdout);
#endif
		beta = (short)((fog_table[index] & 0xff00) >> 8);
		// GMT: clear out the lower 2 bits of delta
		delta_beta = (short)(fog_table[index] & 0xfc);
		// 6.2 * 0.8 results in 6.10
		hold = delta_beta * delta_index;

		if (fogmode->fog_zones) {	// if fog zones enabled
		    if (fog_table[index] & 2) {	// and 2nd lsb is set
			hold = -hold;
		    }
		}
		if (fogmode->fog_dither) {
		    int d = (((y&1)^(x&1))<<3) | ((y&1)<<2) | ((y&2)^(x&2)) | ((y&2)>>1);
		    hold += d << 6;
//printf("dit = %d\n",d);
		}
#if 0
		printf("orig beta:0x%x delta_beta:0x%x pre_hold:0x%x\n",
			beta, delta_beta, hold); fflush(stdout);
#endif
		beta += (short)(hold >> 10);

		/* Sanity */
		if((beta > 255) || (beta < 0)) {
			GDBG_ERROR("fog", "Unexpected fog beta value of %d!\n", beta);
			DIAG_FAIL();
		}
	}

	/* Adjust beta to 1.8 format */
	beta++;
#if 0
	printf("beta is 0x%x\n", beta); fflush(stdout);
#endif

	/* Select proper fog color */
	if(!fogmode->fog_add) {
		fog_color_mux.red = fog_color->red;
		fog_color_mux.green = fog_color->green;
		fog_color_mux.blue = fog_color->blue;
	} else {
		fog_color_mux.red = 0x0;
		fog_color_mux.green = 0x0;
		fog_color_mux.blue = 0x0;
	}

	/* Select proper src color */
	if(!fogmode->fog_mult) {
		fog_src_mux.red = src->red;
		fog_src_mux.green = src->green;
		fog_src_mux.blue = src->blue;
	} else {
		fog_src_mux.red = 0x0;
		fog_src_mux.green = 0x0;
		fog_src_mux.blue = 0x0;
	}

	if(fogmode->fog_constant) {
		fog_src.red = fog_color->red;
		fog_src.green = fog_color->green;
		fog_src.blue = fog_color->blue;
	} else {
		int r, g, b;

		r = fog_color_mux.red - fog_src_mux.red;
#if 0
		printf("fog_color_mux.red:0x%x(%d)  fog_src_mux.red:0x%x(%d)  Diff:0x%x(%d)\n",
			fog_color_mux.red, fog_color_mux.red, fog_src_mux.red, fog_src_mux.red, r, r);
		fflush(stdout);
#endif
		g = fog_color_mux.green - fog_src_mux.green;
		b = fog_color_mux.blue - fog_src_mux.blue;

		/* Multiply... */
		r *= (int) beta;
		g *= (int) beta;
		b *= (int) beta;

		fog_src.red = (short) (r >> 8);
		fog_src.green = (short) (g >> 8);
		fog_src.blue = (short) (b >> 8);
	}

	/* Final Add */
	expect->red = fog_src.red + fog_src_mux.red;
	expect->blue = fog_src.blue + fog_src_mux.blue;
	expect->green = fog_src.green + fog_src_mux.green;

	/* Clamp */
	if(expect->red > 255) expect->red = 255;
	if(expect->green > 255) expect->green = 255;
	if(expect->blue > 255) expect->blue = 255;
}

/*************************************
 * Color Combine 
 ************************************/
void colorcombine(fbzColorPath, combineModereg,
	rgba_iter, rgba_tex, rgba_lfb,
	color1, color0, z_iter, w_iter, expect)
	FxU32 fbzColorPath, combineModereg;
	fbiColors *rgba_iter, *rgba_tex, *rgba_lfb, *color1, *color0;
	ulong z_iter;
	ulong w_iter;
	fbiColors *expect;
{
    short red_c_other, green_c_other, blue_c_other, a_other;
    short red_c_local, green_c_local, blue_c_local, a_local;
    fbiColors mselect7;
    ColorCombineMode ccombinemode, acombinemode;

    if (combineModereg & SST_CM_USE_COMBINE_MODE) {
	GDBG_INFO(100,"Using combineMode %08x\n",combineModereg);
	ccombinemode.rgba_select = (combineModereg & SST_CM_CC_OTHERSELECT) >> SST_CM_CC_OTHERSELECT_SHIFT;
	ccombinemode.localselect = (combineModereg & SST_CM_CC_LOCALSELECT) >> SST_CM_CC_LOCALSELECT_SHIFT;
	ccombinemode.combineMode.invert_other = (combineModereg & SST_CM_CC_INVERT_OTHER)>>SST_CM_CC_INVERT_OTHER_SHIFT;
	ccombinemode.combineMode.invert_local = (combineModereg & SST_CM_CC_INVERT_LOCAL)>>SST_CM_CC_INVERT_LOCAL_SHIFT;
	ccombinemode.combineMode.invert_add_local = (combineModereg & SST_CM_CC_INVERT_ADD_LOCAL) !=0;
	ccombinemode.combineMode.outshift = (combineModereg & SST_CM_CC_OUTSHIFT)>>SST_CM_CC_OUTSHIFT_SHIFT;

	acombinemode.rgba_select = (combineModereg & SST_CM_CCA_OTHERSELECT) >> SST_CM_CCA_OTHERSELECT_SHIFT;
	acombinemode.localselect = (combineModereg & SST_CM_CCA_LOCALSELECT) >> SST_CM_CCA_LOCALSELECT_SHIFT;
	acombinemode.combineMode.invert_other = (combineModereg & SST_CM_CCA_INVERT_OTHER)>>SST_CM_CCA_INVERT_OTHER_SHIFT;
	acombinemode.combineMode.invert_local = (combineModereg & SST_CM_CCA_INVERT_LOCAL)>>SST_CM_CCA_INVERT_LOCAL_SHIFT;
	acombinemode.combineMode.invert_add_local = (combineModereg & SST_CM_CCA_INVERT_ADD_LOCAL) !=0;
	acombinemode.combineMode.outshift = (combineModereg & SST_CM_CCA_OUTSHIFT)>>SST_CM_CCA_OUTSHIFT_SHIFT;
	switch((combineModereg & SST_CM_CC_MSELECT_7)>>SST_CM_CC_MSELECT_7_SHIFT)
	{
		case 0: mselect7 = *rgba_iter; break;
		case 1: mselect7 = *color1; break;
		case 2: mselect7.red = mselect7.green = mselect7.blue = rgba_iter->alpha; break;
		case 3: mselect7.red = mselect7.green = mselect7.blue = color1->alpha; break;
	}
    }
    else {
	GDBG_INFO(100,"Using fbzColorPath %08x\n",fbzColorPath);
	ccombinemode.rgba_select = (fbzColorPath & SST_RGBSELECT) >> SST_RGBSELECT_SHIFT;
	ccombinemode.localselect = (fbzColorPath & SST_LOCALSELECT) >> SST_LOCALSELECT_SHIFT;
	ccombinemode.combineMode.invert_other = 0;
	ccombinemode.combineMode.invert_local = 0;
	ccombinemode.combineMode.invert_add_local = 0;
	ccombinemode.combineMode.outshift = 0;

	acombinemode.rgba_select = (fbzColorPath & SST_ASELECT) >> SST_ASELECT_SHIFT;
	acombinemode.localselect = (fbzColorPath & SST_ALOCALSELECT) >> SST_ALOCALSELECT_SHIFT;
	acombinemode.combineMode.invert_other = 0;
	acombinemode.combineMode.invert_local = 0;
	acombinemode.combineMode.invert_add_local = 0;
	acombinemode.combineMode.outshift = 0;
	mselect7 = *rgba_iter;
    }

	ccombinemode.combineMode.mselect = (fbzColorPath & SST_CC_MSELECT) >> SST_CC_MSELECT_SHIFT;
	ccombinemode.reverse_blend = (fbzColorPath & SST_CC_REVERSE_BLEND) !=0;
	ccombinemode.combineMode.zero_other = (fbzColorPath & SST_CC_ZERO_OTHER) !=0;
	ccombinemode.combineMode.sub_clocal = (fbzColorPath & SST_CC_SUB_CLOCAL) !=0;
	ccombinemode.combineMode.add_clocal = (fbzColorPath & SST_CC_ADD_CLOCAL) !=0;
	ccombinemode.combineMode.add_alocal = (fbzColorPath & SST_CC_ADD_ALOCAL) !=0;
	ccombinemode.combineMode.invert_output = (fbzColorPath & SST_CC_INVERT_OUTPUT) !=0;
	ccombinemode.combineMode.inc_xor_out = 1;

	acombinemode.combineMode.mselect = (fbzColorPath & SST_CCA_MSELECT) >> SST_CCA_MSELECT_SHIFT;
	acombinemode.reverse_blend = (fbzColorPath & SST_CCA_REVERSE_BLEND) !=0;
	acombinemode.combineMode.zero_other = (fbzColorPath & SST_CCA_ZERO_OTHER) !=0;
	acombinemode.combineMode.sub_clocal = (fbzColorPath & SST_CCA_SUB_CLOCAL) !=0;
	acombinemode.combineMode.add_clocal = (fbzColorPath & SST_CCA_ADD_CLOCAL) !=0;
	acombinemode.combineMode.add_alocal = (fbzColorPath & SST_CCA_ADD_ALOCAL) !=0;
	acombinemode.combineMode.invert_output = (fbzColorPath & SST_CCA_INVERT_OUTPUT) !=0;
	acombinemode.combineMode.inc_xor_out = 1;

	ccombinemode.combineMode.xor_gatein = ccombinemode.reverse_blend ? 0 : 0xFF;
	acombinemode.combineMode.xor_gatein = acombinemode.reverse_blend ? 0 : 0xFF;

	GDBG_INFO(100,"Color Combine: RGBA_ITER:0x%x RGBA_TEX:0x%x\n",
		(rgba_iter->alpha << ALPHA_SHIFT) |
		(rgba_iter->red << RED_SHIFT) |
		(rgba_iter->green << GREEN_SHIFT) |
		(rgba_iter->blue << BLUE_SHIFT),
		(rgba_tex->alpha << ALPHA_SHIFT) |
		(rgba_tex->red << RED_SHIFT) |
		(rgba_tex->green << GREEN_SHIFT) |
		(rgba_tex->blue << BLUE_SHIFT));
	GDBG_INFO(100,"\tRGB_SEL:%d RGB_LOCALSEL:%d RGB_REVBLEND:%d RGB_ADD_ALOCAL:%d\n",
		ccombinemode.rgba_select, ccombinemode.localselect,
		ccombinemode.reverse_blend, ccombinemode.combineMode.add_alocal);
	GDBG_INFO(100,"\tRGB_ZERO_OTHER:%d RGB_SUB_CLOCAL:%d RGB_ADD_CLOCAL:%d\n",
		ccombinemode.combineMode.zero_other,
		ccombinemode.combineMode.sub_clocal,
		ccombinemode.combineMode.add_clocal);
	GDBG_INFO(100,"\tRGB_MSELECT:%d RGB_XORGATEIN:%x RGB_INVOUTPUT:%d\n",
		ccombinemode.combineMode.mselect,
		ccombinemode.combineMode.xor_gatein,
		ccombinemode.combineMode.invert_output);
	GDBG_INFO(100,"\tZ_ITER:%0x W_ITER:%0x A_LOCALSEL:%d\n", z_iter, w_iter,
		acombinemode.localselect);

	switch(ccombinemode.rgba_select) {
		case 0:
			red_c_other = rgba_iter->red;
			green_c_other = rgba_iter->green;
			blue_c_other = rgba_iter->blue;
			break;
		case 1:
			red_c_other = rgba_tex->red;
			green_c_other = rgba_tex->green;
			blue_c_other = rgba_tex->blue;
			break;
		case 2:
			red_c_other = color1->red;
			green_c_other = color1->green;
			blue_c_other = color1->blue;
			break;
		case 3:
			red_c_other = rgba_lfb->red;
			green_c_other = rgba_lfb->green;
			blue_c_other = rgba_lfb->blue;
			break;
		case 4:
			red_c_other = rgba_iter->alpha;
			green_c_other = rgba_iter->alpha;
			blue_c_other = rgba_iter->alpha;
			break;
		case 5:
			red_c_other = rgba_tex->alpha;
			green_c_other = rgba_tex->alpha;
			blue_c_other = rgba_tex->alpha;
			break;
		case 6:
			red_c_other = color1->alpha;
			green_c_other = color1->alpha;
			blue_c_other = color1->alpha;
			break;
		case 7:
		default:
			red_c_other = 0;
			green_c_other = 0;
			blue_c_other = 0;
			break;
	}

	switch(ccombinemode.localselect) {
		case 0:
			red_c_local = rgba_iter->red;
			green_c_local = rgba_iter->green;
			blue_c_local = rgba_iter->blue;
			break;
		case 1:
			red_c_local = color0->red;
			green_c_local = color0->green;
			blue_c_local = color0->blue;
			break;
		case 2:
			red_c_local = rgba_tex->red;
			green_c_local = rgba_tex->green;
			blue_c_local = rgba_tex->blue;
			break;
		case 3:
			red_c_local = rgba_iter->alpha;
			green_c_local = rgba_iter->alpha;
			blue_c_local = rgba_iter->alpha;
			break;
		case 4:
			red_c_local = color0->alpha;
			green_c_local = color0->alpha;
			blue_c_local = color0->alpha;
			break;
		case 5:
			red_c_local = rgba_tex->alpha;
			green_c_local = rgba_tex->alpha;
			blue_c_local = rgba_tex->alpha;
			break;
		case 6:
		case 7:
		default:
			red_c_local = 0;
			green_c_local = 0;
			blue_c_local = 0;
			break;
	}

	switch(acombinemode.localselect) {
		case 0:
			a_local = rgba_iter->alpha;
			break;
		case 1:
			a_local = color0->alpha;
			break;
		case 2:
			a_local = (short)z_iter;
			break;
		case 3:
			if (acombinemode.rgba_select == 3)
				a_local = (short)z_iter;	// LFB
			else
				a_local = (short)w_iter;
			break;
	}

	switch(acombinemode.rgba_select) {
		case 0:
			a_other = rgba_iter->alpha;
			break;
		case 1:
			a_other = rgba_tex->alpha;
			break;
		case 2:
			a_other = color1->alpha;
			break;
		case 3:
			a_other = rgba_lfb->alpha;
			break;
		default:
			break;
	}
	combine(&(ccombinemode.combineMode),
		red_c_other, red_c_local,
		a_other, a_local,
		rgba_tex->alpha,	// mselect 4
		rgba_tex->red,		// mselect 5
		0,			// mselect 6
		mselect7.red,		// mselect 7
		&(expect->red));
	combine(&(ccombinemode.combineMode),
		green_c_other, green_c_local,
		a_other, a_local,
		rgba_tex->alpha,	// mselect 4
		rgba_tex->green,	// mselect 5
		0,			// mselect 6
		mselect7.green,		// mselect 7
		&(expect->green));
	combine(&(ccombinemode.combineMode), 
		blue_c_other, blue_c_local,
		a_other, a_local,
		rgba_tex->alpha,	// mselect 4
		rgba_tex->blue,		// mselect 5
		0,			// mselect 6
		mselect7.blue,		// mselect 7
		&(expect->blue));
	combine(&(acombinemode.combineMode),
		a_other, a_local,
		a_other, a_local, 
		rgba_tex->alpha,	// mselect 4
		rgba_iter->alpha,	// mselect 5
		color1->alpha,		// mselect 6
		0,			// mselect 7
		&(expect->alpha));
}

/*************************************
 * Color Combine (Slice)
 ************************************/
void combine(cMode, c_other, c_local, a_other, a_local,
	mselect_input4,
	mselect_input5,
	mselect_input6,
	mselect_input7,
	expect)
	CombineMode *cMode;
	short c_other, c_local, a_other, a_local;
	short mselect_input4, mselect_input5, mselect_input6, mselect_input7;
	short *expect;
{
	int mux_zero_other, mux_sub_clocal, mux_mselect, mux_add_local;
	int mult_a, mult_b, mult_out, adder_out;
	int xor_adj;

	// OTHER
	mux_zero_other = (cMode->zero_other) ? 0x0 : (int) c_other;
	switch (cMode->invert_other) {
	    case 0:	// +x
			break;
	    case 1:	// -x
			mux_zero_other = -mux_zero_other;
			break;
	    case 2:	// 1-x
			mux_zero_other = 0xFF - mux_zero_other;
			break;
	    case 3:	// x-.5
			mux_zero_other -= 0x80;
			break;
	}
	GDBG_INFO(100,"other:0x%x \n", mux_zero_other);

	// LOCAL
	mux_sub_clocal = (cMode->sub_clocal) ? (int) c_local : 0x0;
	switch (cMode->invert_local) {
	    case 0:	// -x
			mux_sub_clocal = -mux_sub_clocal;
			break;
	    case 1:	// +x
			break;
	    case 2:	// 1-x
			mux_sub_clocal = 0xFF - mux_sub_clocal;
			break;
	    case 3:	// x-.5
			mux_sub_clocal -= 0x80;
			break;
	}
	GDBG_INFO(100,"local:0x%x \n", mux_sub_clocal);
	mult_a = mux_zero_other + mux_sub_clocal;

	// MULT
	switch(cMode->mselect) {
		case 0:
			mux_mselect = (int) 0x0;
			break;
		case 1:
			mux_mselect = (int) c_local;
			break;
		case 2:
			mux_mselect = (int) a_other;
			break;
		case 3:
			mux_mselect = (int) a_local;
			break;
		case 4:
			mux_mselect = (int) mselect_input4;
			break;
		case 5:
			mux_mselect = (int) mselect_input5;
			break;
		case 6:
			mux_mselect = (int) mselect_input6;
			break;
		case 7:
			mux_mselect = (int) mselect_input7;
			break;
		default:
			GDBG_ERROR("combine", "invalid mselect = %d\n",cMode->mselect);
			break;
	}
	xor_adj = cMode->inc_xor_out ? 1 : 0;
	mult_b = (mux_mselect ^ cMode->xor_gatein) + xor_adj;
	mult_out = (mult_a * mult_b) >> 8;
	GDBG_INFO(100,"mult_a:0x%x mult_b:0x%x mult_out:0x%x\n", mult_a, mult_b, mult_out);

	switch((cMode->add_clocal << 1) | cMode->add_alocal) {
		case 0:
		case 3:
			mux_add_local = (int) 0x0;
			break;
		case 1:
			mux_add_local = (int) a_local;
			break;
		case 2:
			mux_add_local = (int) c_local;
			break;
		default:
			mux_add_local = (int) 0xffff;
			break;
	}
	// optional 1-x (XOR)
	if (cMode->invert_add_local)
	    mux_add_local = 0xFF - mux_add_local;

	adder_out = mult_out + mux_add_local;

	GDBG_INFO(100,"Mult_out:0x%x(%d), mux_add_local:0x%x(%d), adder_out:0x%x\n",
		mult_out, mult_out, mux_add_local, mux_add_local, adder_out);
	GDBG_INFO(100,"adder_out(pre shift) = 0x%x(%d)  shift=%d\n", adder_out, adder_out,cMode->outshift);

	// Shift and Clamp
	adder_out <<= cMode->outshift;
	if(adder_out > 255) adder_out = 255;
	if(adder_out < 0) adder_out = 0;

	GDBG_INFO(100,"adder_out(post Clamp) = 0x%x(%d)\n", adder_out, adder_out);

	/* Invert */
	*expect = (cMode->invert_output) ? ((~adder_out) & 0xff) : adder_out;
}

