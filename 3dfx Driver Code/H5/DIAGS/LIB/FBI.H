/*
// Definitions for FBI-specific routines
*/

#ifndef __FBI_H__
#define __FBI_H__

typedef unsigned long ulong;
typedef unsigned char unchar;

////////////////////////////////////////////////////////////////////

#define ALPHA_SHIFT 24
#define ALPHA (0xff << ALPHA_SHIFT)
#define RED_SHIFT 16
#define RED (0xff << RED_SHIFT)
#define GREEN_SHIFT 8
#define GREEN (0xff << GREEN_SHIFT)
#define BLUE_SHIFT 0
#define BLUE (0xff << BLUE_SHIFT)
#define YELLOW (RED|GREEN)

/* Typedefs */ 
typedef struct color_struct {
	short red;
	short green;
	short blue;
	short alpha;
} fbiColors;

typedef struct ablend_struct {
	unchar ablend_en;
	unchar alpha_fact_src;
	unchar alpha_fact_dst;
	unchar subtract;
	unchar reverse;
} ablendMode;

typedef struct fog_struct {
    ulong fog_enable;
    ulong fog_mult;
    ulong fog_add;
    ulong fog_constant;
    ulong fog_alpha;
    ulong fog_z;
    ulong fog_dither;
    ulong fog_zones;
} fogMode;

typedef struct combine_struct {
    ulong zero_other;
    ulong sub_clocal;
    ulong add_clocal;
    ulong add_alocal;
    ulong invert_other;
    ulong invert_local;
    ulong invert_add_local;
    ulong outshift;
    ulong mselect;
    ulong xor_gatein;          /* Calculated automatically */
    ulong invert_output;
    ulong inc_xor_out;
} CombineMode;

typedef struct ccombine_struct {
    ulong rgba_select;
    ulong localselect;
    ulong reverse_blend;
    CombineMode combineMode;
} ColorCombineMode;

/* Function prototypes */
void ablend( ablendMode *blendmode,
	     int ditsub, int dm,
	     fbiColors *src,
	     fbiColors *src_pre_fog,
	     fbiColors *dst,
	     fbiColors *expect );

void fog( fogMode *fogmode,
	  int x, int y,
	  fbiColors *src,
	  fbiColors *fog_color,
	  ulong *fog_table,
	  short a_iter,
	  short z_iter,
	  short w_iter_exp,
	  short w_iter_mant,
	  fbiColors *expect );

void colorcombine( unsigned long fbzColorPath,
		   unsigned long combineModereg,
		   fbiColors *rgba_iter,
		   fbiColors *rgba_tex,
		   fbiColors *rgba_lfb,
		   fbiColors *color1,
		   fbiColors *color0,
	           ulong z_iter,
	           ulong w_iter,
		   fbiColors *expect );
void combine( CombineMode *cMode,
	      short c_other,
	      short c_local,
	      short a_other,
	      short a_local,
	      short mselect_input4,
	      short mselect_input5,
	      short mselect_input6,
	      short mselect_input7,
	      short *expect );

#endif