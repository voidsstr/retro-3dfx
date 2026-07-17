/*
** Copyright 1996, 1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
*/

#if __GL_SST
#define XY_FRAC_BITS		4
#define COLOR_FRAC_BITS		18
#else
#define XY_FRAC_BITS		11
#define COLOR_FRAC_BITS		16
#endif

#define Z_FRAC_BITS		0
#define STRQ_FRAC_BITS		16

#define FloatBits(a)		    (ftmp = (float) (a), *((int *)&ftmp))
#define FloatBias(bits)		    ((float) (3 << (22-(bits))))
#define BiasedFloatToFixed(a)	    ((FloatBits(a) & 0x7FFFFFL) - 0x400000L)
#define BiasedFloatToInt(a,bits)    TruncFixed(BiasedFloatToFixed(a),bits)

#define DoubleBits(a)		    (dtmp = (double) (a), *((int *)&dtmp))
#define DoubleBias(bits)	    ((double) (3 << 20) * (1UL << (31-(bits))))
#define BiasedDoubleToFixed(a)	    DoubleBits(a)
#define BiasedDoubleToInt(a,bits)   TruncFixed(BiasedDoubleToFixed(a),bits)

#define FloatToFixedI(a,bits)	    ((int) ((a) * (float) (1UL << (bits))))
#define FloatToFixedF(a,bits)	    BiasedFloatToFixed((a)+FloatBias(bits))
#define FloatToFixedD(a,bits)	    BiasedDoubleToFixed((a)+DoubleBias(bits))

#define FloatToInt(a,bits)	    ((int) (a))
#define FloatToFrac(a)		    ((FloatBits((a)+3.0F) << 9) ^ 0x80000000UL)
#define FloatToFixed(a,bits)	    FloatToFixedD(a,bits)
#define TruncFixed(a,bits)	    ((int) (a) >> (bits))

#define CoordToInt(a)		    TruncFixed((a), COLOR_FRAC_BITS)

#define DITHER_BITS		    4
#define DITHER_BITS_LOG2	    2
#define DITHER_MASK		    ((1 << DITHER_BITS_LOG2) - 1)
#define DITHER_SHIFT		    (COLOR_FRAC_BITS-DITHER_BITS)

#define COLOR_ROUND_NO_DITHER	    0.5F
#define COLOR_ROUND_DITHER	    (0.5F / (1 << DITHER_BITS))

#define XY_BIAS			    FloatBias(XY_FRAC_BITS)

typedef struct __GLtriRec {
    GLfloat c;		/* Area * 2 */

    int x, y;
    int y0Int, y1Int, y2Int;

    int x0Int, x1Int, x2Int;
    int dxdy0Int[2];
    int dxdy1Int[2];
    int dxdy2Int[2];

    int x0Frac;
    int x1Frac;
    int x2Frac;

    int dxdy0Frac;
    int dxdy1Frac;
    int dxdy2Frac;

    int dx, dxSpan;

    /* Color Buffer */
    GLubyte *cp, *cp0;
    int dcpdx, dcpdxSpan, dcpdy0[2];

    /* Depth Buffer */
    GLubyte *zp, *zp0;
    int dzpdx, dzpdxSpan, dzpdy0[2];

    /* Stencil Buffer */
    GLubyte *sp, *sp0;
    int dspdx, dspdxSpan, dspdy0[2];
    GLbitfield maskPostStencil;

    /* Fog Interpolation */
    int f, f0, dfdx, dfdxSpan, dfdy0[2];

    /* Depth Interpolation */
    int z, z0, dzdx, dzdxSpan, dzdy0[2];

    /* Color Interpolation */
    int r, r0, drdx, drdxSpan, drdy0[2];
    int g, g0, dgdx, dgdxSpan, dgdy0[2];
    int b, b0, dbdx, dbdxSpan, dbdy0[2];
    int a, a0, dadx, dadxSpan, dady0[2];

    /* Affine Texture Interpolation */
    int s, s0, dsdx, dsdxSpan, dsdy0[2];
    int t, t0, dtdx, dtdxSpan, dtdy0[2];
    float rho, rho0, drhodx, drhodxSpan, drhody0[2];

    /* Fixed-point Perspective Texture Interpolation */
    int q, q0, dqdx, dqdy0[2];
    int dummy, dummy0, ddummydx, ddummydy0[2];

    /* Floating-point Perspective Texture Interpolation */
    float fsw, fsw0, fdswdx, fdswdxSpan, fdswdxPWL, fdswdy0[2], fdswdy;
    float ftw, ftw0, fdtwdx, fdtwdxSpan, fdtwdxPWL, fdtwdy0[2], fdtwdy;
    float fqw, fqw0, fdqwdx, fdqwdxSpan, fdqwdxPWL, fdqwdy0[2], fdqwdy;

    float frhow, frhow0, fdrhowdx, fdrhowdxSpan, fdrhowdxPWL, fdrhowdy0[2];

    /* Texture Buffer */
    GLubyte *tp;
    int texWidth, texWidthLog2;
    int texHeight, texHeightLog2;

    /* Temporary Color Buffer */
    GLubyte colorBuf[4*32];

    /* Temporary Texture Buffer */
    GLubyte texelBuf[4*32];

    /* Dither Table Pointer */
    const int (*ditherTable)[35];
    const int *dither;

    /* Second Color Buffer for FRONT_AND_BACK */
    GLubyte *cp2, *cp20;
    int dcp2dx, dcp2dxSpan, dcp2dy0[2];

    /* Polygon Offset */
    int depthOffset;

    /* Context Pointer */
    __GLcontext *gc;
} __GLtri;

typedef void (__fastcall *__GLspanlet)(GLbitfield mask, __GLtri *tr);

#ifdef __GL_USE_INTEL_ASM
#define __FR_TRI_DX_OFF		(18*4)
#define __FR_TRI_ZP_OFF		(26*4)
#define __FR_TRI_DZPDX_OFF	(28*4)
#define __FR_TRI_Z_OFF		(38*4)
#define __FR_TRI_DZDX_OFF	(40*4)
#endif

extern const unsigned int __glFRMaskTable[33];
extern const int __glFRDitherTable[4][35];
extern const int __glFRDitherTableRtoL[4][35];
extern const void *__glFRDitherTables[2];
extern const GLubyte __glBlendTab[256*256];

/*****************************************************************/
/*               Texturing and blending macros                   */
/*****************************************************************/

/* Alpha blending, UByte components */
extern GLubyte __glSquareTable[256];
extern GLubyte __glSquareTable2[512];
#define __GL_SQ(x)	(__glSquareTable[x])
#define BLEND(a,b)	((__GL_SQ(a)+__GL_SQ(b)-__GL_SQ((a)-(b)))>>1)


/* SRC_ALPHA / ONE_MINUS_SRC_ALPHA UByte blend:
 * sc=src_color, sa=src_alpha, dc=dst_color
 */
#define SA_MSA(sc,sa,dc)	(BLEND(sc,sa)+BLEND(dc,255-(sa)))

/* Convert from Coord to UByte with small/no fraction */
#define RCoordToUByte(x)	((x)>>RUB2C)
#define GCoordToUByte(x)	((x)>>GUB2C)
#define BCoordToUByte(x)	((x)>>BUB2C)
#define ACoordToUByte(x)	((x)>>AUB2C)

/* Convert from UByte to shifted component */
#define RUByteToComp(c)		(((c)>>RFRAC)<<RSHIFT)
#define GUByteToComp(c)		(((c)>>GFRAC)<<GSHIFT)
#define BUByteToComp(c)		(((c)>>BFRAC)<<BSHIFT)
#define AUByteToComp(c)		(((c)>>AFRAC)<<ASHIFT)

/* MODULATE, leave result as UByte */
#define MODULATE_R_UB(tc,fc)	BLEND(tc,RCoordToUByte(fc))
#define MODULATE_G_UB(tc,fc)	BLEND(tc,GCoordToUByte(fc))
#define MODULATE_B_UB(tc,fc)	BLEND(tc,BCoordToUByte(fc))
#define MODULATE_A_UB(tc,fc)	BLEND(tc,ACoordToUByte(fc))

/* Undithered MODULATE */
#define MODULATE_R(tc,fc)	RUByteToComp(MODULATE_R_UB(tc,fc))
#define MODULATE_G(tc,fc)	GUByteToComp(MODULATE_G_UB(tc,fc))
#define MODULATE_B(tc,fc)	BUByteToComp(MODULATE_B_UB(tc,fc))
#define MODULATE_A(tc,fc)	AUByteToComp(MODULATE_A_UB(tc,fc))

/* Dithered MODULATE */
#define MODULATE_R_D(tc,fc,dd)	RUByteToComp(MODULATE_R_UB(tc,fc)+(dd))
#define MODULATE_G_D(tc,fc,dd)	GUByteToComp(MODULATE_G_UB(tc,fc)+(dd))
#define MODULATE_B_D(tc,fc,dd)	BUByteToComp(MODULATE_B_UB(tc,fc)+(dd))
#define MODULATE_A_D(tc,fc,dd)	AUByteToComp(MODULATE_A_UB(tc,fc)+(dd))

/* DECAL, leave result as UByte */
#define DECAL_R_UB(tc,ta,fc)	SA_MSA(tc,ta,RCoordToUByte(fc))
#define DECAL_G_UB(tc,ta,fc)	SA_MSA(tc,ta,GCoordToUByte(fc))
#define DECAL_B_UB(tc,ta,fc)	SA_MSA(tc,ta,BCoordToUByte(fc))

/* Undithered DECAL */
#define DECAL_R(tc,ta,fc)	RUByteToComp(DECAL_R_UB(tc,ta,fc))
#define DECAL_G(tc,ta,fc)	GUByteToComp(DECAL_G_UB(tc,ta,fc))
#define DECAL_B(tc,ta,fc)	BUByteToComp(DECAL_B_UB(tc,ta,fc))

/* Dithered DECAL */
#define DECAL_R_D(tc,ta,fc,dd)	RUByteToComp(RCLAMP_UB(DECAL_R_UB(tc,ta,fc)+(dd)))
#define DECAL_G_D(tc,ta,fc,dd)	GUByteToComp(GCLAMP_UB(DECAL_G_UB(tc,ta,fc)+(dd)))
#define DECAL_B_D(tc,ta,fc,dd)	BUByteToComp(BCLAMP_UB(DECAL_B_UB(tc,ta,fc)+(dd)))

/* Texture BLEND, leave result as UByte */
#define TBLEND_R_UB(tc,fc,bc)	SA_MSA(bc,tc,RCoordToUByte(fc))
#define TBLEND_G_UB(tc,fc,bc)	SA_MSA(bc,tc,GCoordToUByte(fc))
#define TBLEND_B_UB(tc,fc,bc)	SA_MSA(bc,tc,BCoordToUByte(fc))
#define TBLEND_A_UB(tc,fc,bc)	SA_MSA(bc,tc,ACoordToUByte(fc))

/* Undithered Texture BLEND */
#define TBLEND_R(tc,fc,bc)	RUByteToComp(TBLEND_R_UB(tc,fc,bc))
#define TBLEND_G(tc,fc,bc)	GUByteToComp(TBLEND_G_UB(tc,fc,bc))
#define TBLEND_B(tc,fc,bc)	BUByteToComp(TBLEND_B_UB(tc,fc,bc))
#define TBLEND_A(tc,fc,bc)	AUByteToComp(TBLEND_A_UB(tc,fc,bc))

/* Dithered Texture BLEND */
#define TBLEND_R_D(tc,fc,bc,dd)	RUByteToComp(TBLEND_R_UB(tc,fc,bc)+(dd))
#define TBLEND_G_D(tc,fc,bc,dd)	GUByteToComp(TBLEND_G_UB(tc,fc,bc)+(dd))
#define TBLEND_B_D(tc,fc,bc,dd)	BUByteToComp(TBLEND_B_UB(tc,fc,bc)+(dd))
#define TBLEND_A_D(tc,fc,bc,dd)	AUByteToComp(TBLEND_A_UB(tc,fc,bc)+(dd))
