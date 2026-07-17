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
** $Date: 10/11/00 8:08:53 PM$
*/

#include <assert.h>
#include <h3.h>
#include "h3sim.h"

// hack for overriding automatic setting of the launched flag
// by high level csim write routine whenever the launch region is
// written to (e.g., when the last word of a hostblit is received,
// the command needs to terminate launch mode)
FxBool dontMessWithLaunched = 0;

// host blit error detection -- signal an error when a register other than
// the launch register is written when in the middle of a host blit
// command
//
FxBool InHostBlit = FXFALSE;

// host blit error detection -- signal an error when the launch register
// is written after the last dword of a host blit is received (e.g.,
// don't allow back to back host blits to be launched via continued
// writing of the launch registre
//
FxBool HostBlitComplete = FXFALSE;

//----------------------------------------------------------------------
// update dstXY for a blt command
//----------------------------------------------------------------------
void updateXY(SstGRegs *sstg)
{
    if (sstg->command & (SSTG_UPDATE_DSTX|SSTG_UPDATE_DSTY)) {
	FxU32 x = LOWORD(sstg->dstXY);
	FxU32 y = HIWORD(sstg->dstXY);
	if (sstg->command & SSTG_UPDATE_DSTX)
		x = (x + LOWORD(sstg->dstSize)) & SST_MASK(SSTG_XY_SIZE);
	if (sstg->command & SSTG_UPDATE_DSTY)
		y = (y + HIWORD(sstg->dstSize)) & SST_MASK(SSTG_XY_SIZE);
	sstg->dstXY = (y<<16) | x;
    }
}

//----------------------------------------------------------------------
// convenient print routine
// NOTE: for maximum CSIM performance compile without GDBG_INFO_ON
//----------------------------------------------------------------------
static void sstgPrintOut(SstGRegs *sstg, char *msg)
{
    GDBG_INFO(105,"%s(0x%x)\n", msg,sstg->command);
#ifdef GDBG_INFO_ON
    if (GDBG_GET_DEBUGLEVEL(125))	// delay this printout until after setup
	sstgPrintModes(sstg,"sstg.GO:");
    if (GDBG_GET_DEBUGLEVEL(126))	// delay this printout until after setup
	sstgPrintRegs(sstg,"");
#endif
}

//----------------------------------------------------------------------
// rectangle clip a pixel, return 0 if inside, 1 if clipped out
//----------------------------------------------------------------------
static int rectClip(SstGRegs *sstg, int x, int y)
{
    FxU32 clipMin, clipMax;

    if (sstg->command & SSTG_CLIPSELECT) {	// first select clip regs
	clipMin = sstg->clip1min;
	clipMax = sstg->clip1max;
    }
    else {
	clipMin = sstg->clip0min;
	clipMax = sstg->clip0max;
    }

    if (x < (signed)LOWORD(clipMin) || !(x < (signed)LOWORD(clipMax)) ||
	y < (signed)HIWORD(clipMin) || !(y < (signed)HIWORD(clipMax)))
    {
	GDBG_INFO(160,"pixel %d,%d rect-clipped out\n",x,y);
	return 1;
    }
    return 0;
}

//----------------------------------------------------------------------
// color conversion utilities used by the main routine down below
//----------------------------------------------------------------------

#if COLORTRANSLUT
// if converting 8bpp to 8bpp and CLUT is enabled, then use it
static FxU32 cvt8(SstGRegs *sstg, FxU32 csrc)
{
    if ((sstg->command & SSTG_EN_CLUT88) &&
	((sstg->dstFormat & SSTG_DST_FORMAT) == SSTG_PIXFMT_8BPP))
	return sstg->colorTransLut[csrc & 0xFF];

    return csrc & 0xFF;
}
#endif

static FxU32 cvt15(SstGRegs *sstg, FxU32 csrc)
{
    switch(sstg->dstFormat & SSTG_DST_FORMAT) {
	case SSTG_PIXFMT_8BPP:
		GDBG_ERROR("csimColorConvert", "15 bpp to 8bpp not supported\n");
		return 0xdeadbeef;

	case SSTG_PIXFMT_15BPP:
		return csrc & 0xFFFF;

	case SSTG_PIXFMT_16BPP:
		return ((csrc & 0x7FE0) << 1) | ((csrc & 0x200)>>4) | (csrc & 0x1F);

	case SSTG_PIXFMT_24BPP:
	case SSTG_PIXFMT_32BPP:
	{
		FxU32 r,g,b;
		r = csrc & 0x7C00;
		g = csrc & 0x03E0;
		b = csrc & 0x001F;
		return	(r<<9) | ((r<<4)&0x070000) |
			(g<<6) | ((g<<1)&0x000700) |
			(b<<3) | (b>>2);
	}
    default:
      assert(0);
      return(0xbadbad);
    }
}

static FxU32 cvt16(SstGRegs *sstg, FxU32 csrc)
{
    switch(sstg->dstFormat & SSTG_DST_FORMAT) {
	case SSTG_PIXFMT_8BPP:
		GDBG_ERROR("csimColorConvert", "16 bpp to 8bpp not supported\n");
		return 0xdeadbeef;

	case SSTG_PIXFMT_15BPP:
		return ((csrc & 0xFFC0) >> 1) | (csrc & 0x1F);

	case SSTG_PIXFMT_16BPP:
		return csrc & 0xFFFF;

	case SSTG_PIXFMT_24BPP:	
	case SSTG_PIXFMT_32BPP:
	{
		FxU32 r,g,b;
		r = csrc & 0xF800;
		g = csrc & 0x07E0;
		b = csrc & 0x001F;
		return	(r<<8) | ((r<<3)&0x070000) |
			(g<<5) | ((g>>1)&0x000300) |
			(b<<3) | (b>>2);
	}
    default:
      assert(0);
      return(0xbadbad);
    }
}

static FxU32 cvt24(SstGRegs *sstg, FxU32 csrc)
{
    switch(sstg->dstFormat & SSTG_DST_FORMAT) {
	case SSTG_PIXFMT_8BPP:
		GDBG_ERROR("csimColorConvert", "24 bpp to 8bpp not supported\n");
		return 0xdeadbeef;

	case SSTG_PIXFMT_15BPP:	
		return	((csrc & 0xF80000) >> 9) |
			((csrc & 0x00F800) >> 6) |
			((csrc & 0x0000F8) >> 3);

	case SSTG_PIXFMT_16BPP:
		return	((csrc & 0xF80000) >> 8) |
			((csrc & 0x00FC00) >> 5) |
			((csrc & 0x0000F8) >> 3);

	case SSTG_PIXFMT_24BPP:
		return csrc & 0xFFFFFF;

	case SSTG_PIXFMT_32BPP:
		return csrc & 0xFFFFFF;	// alpha = 00
    default:
      assert(0);
      return(0xbadbad);

    }
}

static FxU32 cvt32(SstGRegs *sstg, FxU32 csrc)
{
    switch(sstg->dstFormat & SSTG_DST_FORMAT) {
	case SSTG_PIXFMT_8BPP:
		GDBG_ERROR("csimColorConvert", "32 bpp to 8bpp not supported\n");
		return 0xdeadbeef;

	case SSTG_PIXFMT_15BPP:	
		return	((csrc & 0xF80000) >> 9) |
			((csrc & 0x00F800) >> 6) |
			((csrc & 0x0000F8) >> 3);

	case SSTG_PIXFMT_16BPP:	
		return	((csrc & 0xF80000) >> 8) |
			((csrc & 0x00FC00) >> 5) |
			((csrc & 0x0000F8) >> 3);

	case SSTG_PIXFMT_24BPP:
		csrc &= 0xFFFFFF;	// alpha = 00, fall thru
	case SSTG_PIXFMT_32BPP:
		return csrc;
    default:
      assert(0);
      return(0xbadbad);
    }
}

static FxU32 cvtYUV(SstGRegs *sstg, FxU32 csrc)
{
    FxU32 rgb = yuvTOrgbFixed(csrc);
    
    switch(sstg->dstFormat & SSTG_DST_FORMAT)
    {
      case SSTG_PIXFMT_8BPP:
	  GDBG_ERROR("csimColorConvert", "YUV to 8bpp not supported\n");
	  return 0xdeadbeef;

      case SSTG_PIXFMT_15BPP:
	  return ((rgb & 0xF80000) >> 9) |
	         ((rgb & 0x00F800) >> 6) |
	         ((rgb & 0x0000F8) >> 3);

      case SSTG_PIXFMT_16BPP:	
	  return ((rgb & 0xF80000) >> 8) |
       	         ((rgb & 0x00FC00) >> 5) |
	         ((rgb & 0x0000F8) >> 3);

      case SSTG_PIXFMT_24BPP:
      case SSTG_PIXFMT_32BPP:
	  rgb &= 0xFFFFFF;
	  return rgb;
    default:
      assert(0);
      return(0xbadbad);

    }
}


//----------------------------------------------------------------------
// convert a color from source format to destination format
// the source color comes in right justified and the resulting
// color is returned right justified
// NOTE: colorFore and colorBack regs are in destination format
// NOTE: colorTransLut entries are in destination format
// NOTE: this routine is used by the diagnostics (just not worth rewriting)
// SSTG_EN_CLUT88 only applies when src=dst=8bpp
//----------------------------------------------------------------------
FX_EXPORT FxU32 FX_CSTYLE
csimColorConvert(SstGRegs *sstg, FxU32 csrc)
{
    switch(sstg->srcFormat & SSTG_SRC_FORMAT) {
	case SSTG_PIXFMT_1BPP:	return (csrc & 1) ? sstg->colorFore : sstg->colorBack;

#if COLORTRANSLUT
//	case SSTG_PIXFMT_4BPP:	return sstg->colorTransLut[csrc & 0xF];

	case SSTG_PIXFMT_8BPP:	return cvt8(sstg, csrc);
#else
	case SSTG_PIXFMT_8BPP:	
		if ((sstg->dstFormat & SSTG_DST_FORMAT) == SSTG_PIXFMT_8BPP)
			return csrc & 0xFF;
		GDBG_ERROR("csimColorConvert", "8bpp to larger not supported\n");
		
#endif
	case SSTG_PIXFMT_15BPP:	return cvt15(sstg,csrc);

	case SSTG_PIXFMT_16BPP:	return cvt16(sstg,csrc);
		
	case SSTG_PIXFMT_24BPP:	return cvt24(sstg,csrc);

	case SSTG_PIXFMT_32BPP:	return cvt32(sstg,csrc);

      case SSTG_PIXFMT_422YUV:	    
      case SSTG_PIXFMT_422UYV:  return cvtYUV(sstg,csrc);

      default:
	  GDBG_ERROR("csimColorConvert", "invalid source pixel format\n");
	  return 0xdeadbeef;
    }
}

//----------------------------------------------------------------------
// expand a pixel thru the pattern register based on destination format
//----------------------------------------------------------------------
static FxU32 lookupPattern(SstGRegs *sstg, int x, int y, FxBool *vis)
{
    FxU32 cmd = sstg->command;

    // first fetch the pattern offset and bias X,Y
    x = (x+((cmd & SSTG_X_PATOFFSET)>>SSTG_X_PATOFFSET_SHIFT)) & 7;
    y = (y+((cmd & SSTG_Y_PATOFFSET)>>SSTG_Y_PATOFFSET_SHIFT)) & 7;
    if (sstg->commandEx & SSTG_PAT_FORCE_ROW0) y = 0;

    // now test to see if the pattern is monochrome
    if (cmd & SSTG_MONO_PATTERN) {
	FxU32 col = sstg->colorPattern[y>>2] >> ((7-x) + ((y&3)<<3));
	*vis = (col & 1) || !(cmd & SSTG_TRANSPARENT);
	return (col & 1) ? sstg->colorFore : sstg->colorBack;
    }
    else *vis = 1;

    // else pattern is in the format of the destination
    switch(sstg->dstFormat & SSTG_DST_FORMAT) {
	case SSTG_PIXFMT_32BPP:
	    return sstg->colorPattern[(y<<3) + x];

	case SSTG_PIXFMT_24BPP:
	    y = ((y<<3)+x);			// get byte index
	    y = y+y+y;
	    x = y & 3;				// which byte
	    y = y >> 2;				// which word
	    
	    switch(x) {
		case 0:	return 0xFFFFFF & sstg->colorPattern[y];
		case 1:	return 0xFFFFFF & (sstg->colorPattern[y] >> 8);
		case 2:	return 0xFFFFFF & ((sstg->colorPattern[y+1]<<16) | (sstg->colorPattern[y] >> 16));
		case 3:	return 0xFFFFFF & ((sstg->colorPattern[y+1]<<8) | (sstg->colorPattern[y] >> 24));
	    }

	case SSTG_PIXFMT_16BPP:
	case SSTG_PIXFMT_15BPP:
	    return 0xFFFF & (sstg->colorPattern[(y<<2) + (x>>1)] >> ((x&1)<<4));

	case SSTG_PIXFMT_8BPP:
	    return 0xFF & (sstg->colorPattern[(y<<1) + (x>>2)] >> ((x&3)<<3));
    default:
      assert(0);
      return(0xbadbad);

    }
}

//----------------------------------------------------------------------
// perform colorKey test, NOTE: r,g,b order doesn't really matter
//----------------------------------------------------------------------
static FxU32 colorKey(FxU32 col, FxU32 fmt, FxU32 min, FxU32 max)
{
    FxU32 r,g,b;
    FxU32 y, u, v;

    switch(fmt & SSTG_SRC_FORMAT) {
	case SSTG_PIXFMT_1BPP:
//	case SSTG_PIXFMT_4BPP:
		GDBG_ERROR("colorKey","colorKey not allowed in 1 or 4 bpp modes\n");
		return 0;

	case SSTG_PIXFMT_8BPP:
		col &= 0xFF;
		return (col >= (min&0xFF)) && (col <= (max&0xFF));

	case SSTG_PIXFMT_15BPP:
		r = col & 0x1F;
		g = (col>>5) & 0x1F;
		b = (col>>10) & 0x1F;
		return (r >= (min&0x1F)) && (r <= (max&0x1F)) &&
			(g >= ((min>>5)&0x1F)) && (g <= ((max>>5)&0x1F)) &&
			(b >= ((min>>10)&0x1F)) && (b <= ((max>>10)&0x1F));

	case SSTG_PIXFMT_16BPP:
		r = col & 0x1F;
		g = (col>>5) & 0x3F;
		b = (col>>11) & 0x1F;
		return (r >= (min&0x1F)) && (r <= (max&0x1F)) &&
			(g >= ((min>>5)&0x3F)) && (g <= ((max>>5)&0x3F)) &&
			(b >= ((min>>11)&0x1F)) && (b <= ((max>>11)&0x1F));

	case SSTG_PIXFMT_24BPP:
	case SSTG_PIXFMT_32BPP:
		r = col & 0xFF;
		g = (col>>8) & 0xFF;
		b = (col>>16) & 0xFF;
		return (r >= (min&0xFF)) && (r <= (max&0xFF)) &&
			(g >= ((min>>8)&0xFF)) && (g <= ((max>>8)&0xFF)) &&
			(b >= ((min>>16)&0xFF)) && (b <= ((max>>16)&0xFF));

      case SSTG_PIXFMT_422YUV:
      case SSTG_PIXFMT_422UYV:
	  // yuv already is placed into this format before we're called
	  v = col & 0xFF;
	  u = (col >> 8) & 0xFF;
	  y = (col >> 16) & 0xFF;
	  return (y >= (min&0xFF)) && (y <= (max&0xFF)) &&
	      (u >= ((min>>8)&0xFF)) && (u <= ((max>>8)&0xFF)) &&
	      (v >= ((min>>16)&0xFF)) && (v <= ((max>>16)&0xFF));	  
	  
	default:
	  GDBG_ERROR("colorKey","invalid color format 0x%x\n",fmt);
	  return(0);
    }
}

//----------------------------------------------------------------------
// perform raster operation for a pixel
//----------------------------------------------------------------------
static FxU32 doRop(SstRegs *sst, SstGRegs *sstg, int x, int y, FxU32 src, int srcKey, FxBool *vis)
{
    FxU32 rop, pat, cmd = sstg->command;
#define DST csimReadPixel(sst,CSIM_BUF_2D_DST,x,y)

    // select a rop based on colorkeying
    {
	int dstKey=0;

	if (sstg->commandEx & SSTG_EN_DST_COLORKEY_EX) {
	    dstKey = colorKey(DST,sstg->dstFormat,
				sstg->dstColorkeyMin,sstg->dstColorkeyMax);
	}
	srcKey = ((srcKey<<1) | dstKey)-1;	// form a 2-bit number
	if (srcKey < 0)
	    rop = sstg->command >> SSTG_ROP0_SHIFT; // rop0 is stored here
	else
	     rop = (sstg->rop >> (srcKey<<3)) & 0xFF;// index into the ROP register
	GDBG_INFO(144,"\tusing rop[%d]: %02x\n", srcKey+1, rop);
    }
    pat = lookupPattern(sstg,x,y,vis);

    if (SSTG_ISBINARYROP(rop)) {	// if simple binary rop
	GDBG_INFO(143,"\trop2....src dst: 0x%08x 0x%08x\n",src,DST);
	switch (rop>>4) {
	    case SSTG_ROP_ZERO>>4:	return 0;
	    case SSTG_ROP_NOR>>4:	return ~(src | DST);
	    case SSTG_ROP_ANDI>>4:	return DST & ~src;
	    case SSTG_ROP_NSRC>>4:	return ~src;
	    case SSTG_ROP_ANDR>>4:	return src & ~DST;
	    case SSTG_ROP_NDST>>4:	return ~DST;
	    case SSTG_ROP_XOR>>4:	return src ^ DST;
	    case SSTG_ROP_NAND>>4:	return ~(src & DST);
	    case SSTG_ROP_AND>>4:	return src & DST;
	    case SSTG_ROP_XNOR>>4:	return ~(src ^ DST);
	    case SSTG_ROP_DST>>4:	return DST;
	    case SSTG_ROP_ORI>>4:	return DST | ~src;
	    case SSTG_ROP_SRC>>4:	return src;
	    case SSTG_ROP_ORR>>4:	return src | ~DST;
	    case SSTG_ROP_OR>>4:	return src | DST;
	    case SSTG_ROP_ONE>>4:	return 0xFFFFFFFF;
	}
    }
    {					// ternary rop: depends on pattern
	FxU32 res;

	if (rop == SSTG_ROP_PATCOPY) {	// optimize most common case
	    GDBG_INFO(143,"\trop pat src xxx: 0x%08x 0x%08x\n",pat,src);
	    res = pat;
	}
	else {
	    FxU32 i,iend,dst,psd;
	    static FxU32 _bpp[] = {1,8,16,16,24,32,8,8};
	    dst = DST;
	    res = 0;
	    GDBG_INFO(143,"\trop pat src dst: 0x%08x 0x%08x 0x%08x\n",pat,src,dst);

	    // only rop as many bits as necessary!
	    iend = _bpp[(sstg->dstFormat & SSTG_DST_FORMAT)>>SSTG_DST_FORMAT_SHIFT];
	    pat <<= 32-iend;
	    src <<= 32-iend;
	    dst <<= 32-iend;
	    for (i=0; i<iend; i++) {	// simulate 32 * 8:1 mux
		res <<= 1;
		psd = ((pat>>29)&4) | ((src>>30)&2) | ((dst>>31)&1);
		res |= (rop >> psd) & 1;
		pat <<= 1;
		src <<= 1;
		dst <<= 1;
	    }
	}
	return res;
    }
}

//----------------------------------------------------------------------
// draw a pixel in the 2D engine
//----------------------------------------------------------------------
static void doPixel(SstRegs *sst, SstGRegs *sstg, int x, int y, FxU32 col, int srcKey)
{
  FxU32 originalColor;

    FxBool vis;
    if (rectClip(sstg,x,y))		// first perform rectClip function
	return;

    //For 1555 support we to record the original value of bit 15 (alpha)
    if(sstg->commandEx & SSTG_PRESERVE_MSB)
      {
	//Make sure we're in 1555
	if((sstg->dstFormat & SSTG_DST_FORMAT) != SSTG_PIXFMT_16BPP)
	  {
	    GDBG_ERROR("doPixel", "Preserving MSB must only be used with 1555\n");
	  }

	originalColor = csimReadPixel(sst,CSIM_BUF_2D_DST,x,y);
      }

    GDBG_INFO(141,"\tafter rect clip: 0x%08x\n",col);
    col = doRop(sst,sstg,x,y,col,srcKey,&vis);// apply raster operations


    //Restore the alpha bit if in 1555
    if(sstg->commandEx & SSTG_PRESERVE_MSB)
      col = (col & 0x7FFF) | (originalColor & 0x8000);

    if (vis == 0) {
	GDBG_INFO(148,"\tafter rasterops: transparent\n");
	return;
    }
    GDBG_INFO(148,"\tafter rasterops: 0x%08x\n",col);
    csimWritePixel(sst,CSIM_BUF_2D_DST,x,y,col);	// and send it to the framebuffer
    CSIMG_PRIVATE(sstg)->pixelsOut2d++;	// stats counter
    GDBG_INFO(149,"\t---- done pixel #%d ---- %d,%d ----\n",
		CSIMG_PRIVATE(sstg)->pixelsOut2d,x,y);
}

//----------------------------------------------------------------------
// setup a bresenham edge iterator (used by lines, polys, stretch blits)
//----------------------------------------------------------------------
static void bresSetup(SstGRegs *sstg, BresEdge *pe, FxU32 error,
			int x1, int y1, int x2, int y2)
{
    FxBool Xmajor;
    int adx,ady, dx,dy;
    int stretch;

    stretch = (sstg->command & SSTG_COMMAND) >> SSTG_COMMAND_SHIFT;
    stretch = (stretch == SSTG_STRETCH_BLT || stretch == SSTG_HOST_STRETCH_BLT);

    // setup the line, compute deltas and sort it
    adx = dx = x2 - x1;
    ady = dy = y2 - y1;
    pe->dx = dx;
    if (adx < 0) adx = -adx;
    if (ady < 0) ady = -ady;

    Xmajor = adx >= ady;
    if (Xmajor) {			// Xmajor
	if (stretch)
	    pe->err = 3*ady - 2*adx;
	else
	    pe->err = 2*ady - adx;
	pe->einc1 = 2*ady;
	pe->einc2 = 2*(ady-adx);
	pe->xinc1 = ISIGN(dx);
	pe->xinc2 = pe->xinc1;
	pe->yinc1 = 0;
	pe->yinc2 = ISIGN(dy);
    }
    else {				// Ymajor
	if (stretch)
	    pe->err = 2*adx - 1*ady;
	else
	    pe->err = 2*adx - ady;
	pe->einc1 = 2*adx;
	pe->einc2 = 2*(adx-ady);
	pe->xinc1 = 0;
	pe->xinc2 = ISIGN(dx);
	pe->yinc1 = ISIGN(dy);
	pe->yinc2 = pe->yinc1;
    }
    pe->x = x1;
    pe->y = y1;

    // if we are given an error term, then use it
    if (error & 0x80000000)
	pe->err = SIGN_EXTEND(error,16);

    // Give us NT compatible integer lines by biasing the error
    // down in the proper quadrants.

    // this is a clever way of modifying the comparison to <=0 intead of <0
    if ( (sstg->command & SSTG_REVERSIBLE) && (Xmajor ? (dy>0) : (dx>0)))
	pe->err--;
}

static void bresIterate(BresEdge *pe)
{
    if (pe->err < 0) {			// iterate the line
	pe->x += pe->xinc1;		// major axis
	pe->y += pe->yinc1;
	pe->err += pe->einc1;
    }
    else {				// major and minor axis
	pe->x += pe->xinc2;
	pe->y += pe->yinc2;
	pe->err += pe->einc2;
    }
}

//----------------------------------------------------------------------
// draw a line from srcXY to dstXY
//----------------------------------------------------------------------
static void line(SstRegs *sst, SstGRegs *sstg, int skiplast)
{
    int xs,ys;		// x,y source
    int xd,yd;		// x,y destination
    int done = 0;
    int solid  = ! (sstg->command & SSTG_EN_LINESTIPPLE);
    FxU32 monotrans, repeat, ipos, fpos, lssize;
    BresEdge *be = &CSIMG_PRIVATE(sstg)->bLeft;

    if (!solid) {
	monotrans = sstg->command & SSTG_TRANSPARENT;
	repeat = (sstg->lineStyle & SSTG_LSREPEAT)>>SSTG_LSREPEAT_SHIFT;
	ipos = (sstg->lineStyle & SSTG_LSPOS_INT)>>SSTG_LSPOS_INT_SHIFT;
	fpos = (sstg->lineStyle & SSTG_LSPOS_FRAC)>>SSTG_LSPOS_FRAC_SHIFT;
	lssize = (sstg->lineStyle & SSTG_LSSIZE)>>SSTG_LSSIZE_SHIFT;
	if (fpos > repeat)
	    GDBG_ERROR("line", "fractional stipple pos > repeat count\n");
	if (ipos > lssize)
	    GDBG_ERROR("line", "integer stipple pos > bit mask size\n");
    }

    // get source and destination coordinates from registers
    xs = SIGN_EXTEND(LOWORD(sstg->srcXY),SSTG_XY_SIZE);
    ys = SIGN_EXTEND(HIWORD(sstg->srcXY),SSTG_XY_SIZE);

    xd = SIGN_EXTEND(LOWORD(sstg->dstXY),SSTG_XY_SIZE);
    yd = SIGN_EXTEND(HIWORD(sstg->dstXY),SSTG_XY_SIZE);

    bresSetup(sstg, be, sstg->bresError0, xs,ys, xd,yd);

    while (!done) {
	if (be->yinc1==0 ? be->x == xd  : be->y == yd) {
	    done = 1;
	    if (skiplast) break;
	}
	// NOTE: solid lines don't update lineStyle counters
	if (solid)			// if solid unconditionally draw pixel
	    doPixel(sst,sstg,be->x,be->y,sstg->colorFore,0);
	else {
	    GDBG_INFO(137,"\t ls_pos=%d.%d\n",ipos,fpos);
	    if (sstg->lineStipple & (1<<ipos))	// stipple is '1'
		doPixel(sst,sstg,be->x,be->y,sstg->colorFore,0);
	    else if (monotrans) {		// stipple is '0'
		GDBG_INFO(148,"\t               : transparent\n");
		GDBG_INFO(149,"\t---- done pixel ---- %d,%d ----\n",be->x,be->y);
	    }
	    else
		doPixel(sst,sstg,be->x,be->y,sstg->colorBack,0);
	    if (fpos == repeat) {	// if equal to repeat
		fpos = 0;
		if (ipos == lssize)
		    ipos = 0;		// modulo lineStipple size
		else ipos++;		// then increment integer pos
	    }
	    else fpos++;		// increment fractional position
	}
	bresIterate(be);		// iterate the line
    }
    if (!solid) {			// return data to register
	sstg->lineStyle &= ~(SSTG_LSPOS_INT | SSTG_LSPOS_FRAC);
	sstg->lineStyle |= fpos << SSTG_LSPOS_FRAC_SHIFT;
	sstg->lineStyle |= ipos << SSTG_LSPOS_INT_SHIFT;
    }

    sstg->srcXY = sstg->dstXY;		// copy dst to src
}


// XXX NOTE: these statics prevent us from running multiple boards
static FxU32 last_data, last_count;

//----------------------------------------------------------------------
// expand a word (32-bits) of host data into serveral pixels
//----------------------------------------------------------------------
static FxU32 expand(SstGRegs *sstg,
		    FxU32 base,		// beginning byte offset in data
		    FxU32 bitbase,	// for mono src, starting pixel #
		    FxU32 data,
		    FxU32 *expandedPixels)
{
    FxU32 b, npixels, i;
    FxU32 x, y, u, v;

    if (sstg->srcFormat & SSTG_HOST_BYTE_SWIZZLE) {
	data = (data>>24) | ((data>>8)&0xFF00) | ((data<<8)&0xFF0000) | (data<<24);
    }
    if (sstg->srcFormat & SSTG_HOST_WORD_SWIZZLE) {
	data = (data>>16) | (data<<16);
    }

    switch(sstg->srcFormat & SSTG_SRC_FORMAT)
    {
      case SSTG_PIXFMT_1BPP:
	  gdbg_info(8,"skipping %d+%d bits\n", base*8, bitbase);
	  b = base*8 + bitbase;

	  // always expand all remaing bits regardless of width/packing
	  // (the hostBlit() function will ignore anything beyond the
	  // current destination row
	  npixels = 32 - b;		// # of pixels left in dword
	  for (i = 0; i < npixels; i++)
	  {
	      expandedPixels[i] = (data >> ((b&~7) + 7 - (b&7))) & 1;
	      GDBG_INFO(144,"\t       expanded[%d]: 0x%08x\n", i,
			expandedPixels[i]);
	      b++;
	  }
	  return npixels;	// possibly more than are left in dest. row

      case SSTG_PIXFMT_8BPP:
	  data >>= base<<3;		// right justify data
	  npixels = 4 - base;
	  for (i = 0; i < npixels; i++)
	  {
	      expandedPixels[i] = data & 0xFF;
	      GDBG_INFO(144,"\t       expanded[%d]: 0x%08x\n", i,
			expandedPixels[i]);
	      data >>= 8;
	  }
	  return npixels;
	  
      case SSTG_PIXFMT_15BPP:
      case SSTG_PIXFMT_16BPP:
	  data >>= base<<3;		// right justify data
	  npixels = (4 - base) >> 1;
	  for (i = 0; i < npixels; i++)
	  {
	      expandedPixels[i] = data & 0xFFFF;
	      GDBG_INFO(144,"\t       expanded[%d]: 0x%08x\n", i,
			expandedPixels[i]);
	      data >>= 16;
	  }
	  return npixels;

      case SSTG_PIXFMT_24BPP:
	  gdbg_info(8,"last_count= %d , base = %d\n", last_count, base);
	  if (last_count)
	  {	// DATA LEFT OVER, this word provides 4 bytes
	      if (last_count == 1)
	      {	// 1 byte + 4 == one 24bpp pixels
		  expandedPixels[0] = ((data&0xFFFF)<<8) | (last_data & 0xFF);
		  GDBG_INFO(144,"\t       expanded[0]: 0x%08x\n",expandedPixels[0]);
		  last_count = 2;
		  last_data = data >> 16;
		  return 1;
	      }
	      else if (last_count == 2)
	      {	// 2 bytes + 4 == two 24bpp pixels
		  expandedPixels[0] = ((data&0xFF)<<16) | (last_data & 0xFFFF);
		  expandedPixels[1] = (data>>8) & 0xFFFFFF;
		  GDBG_INFO(144,"\t       expanded[0]: 0x%08x\n",expandedPixels[0]);
		  GDBG_INFO(144,"\t       expanded[1]: 0x%08x\n",expandedPixels[1]);
		  last_count = 0;
		  last_data = 0xdeaddead;
		  return 2;
	      }
	      else
		  GDBG_ERROR("expand", "24 bpp, last_count > 2\n");
	  }
	  else
	      switch(base)
	      {		// NO DATA FROM THE LAST TIME
		case 0:
		    last_data = data>>24;	// save the MSB
		    last_count = 1;
		case 1:
		    if (base==1)
			data >>= 8;
		    expandedPixels[0] = data & 0xFFFFFF;
		    GDBG_INFO(144,"\t       expanded[0]: 0x%08x\n",expandedPixels[0]);
		    return 1;
		case 2:
		    last_data = data>>16;
		    last_count = 2;
		    return 0;
		case 3:
		    last_data = data>>24;	// save the MSB
		    last_count = 1;
		    return 0;
	      }
	  GDBG_ERROR("expand", "24 bpp source pixel format internal error\n");
	  return 0;

      case SSTG_PIXFMT_422YUV:
	  for (npixels = 0, x = base >> 1; x < 2; x++)
	  {
	      y = (data >> ((x & 1) ? 16 : 0)) & 0xFF;
	      u = (data >> 8) & 0xFF;
	      v = (data >> 24) & 0xFF;
	      expandedPixels[npixels++] = (y << 16) | (u << 8) | v;
	  }
	  return npixels;
	  
      case SSTG_PIXFMT_422UYV:
	  for (npixels = 0, x = base >> 1; x < 2; x++)
	  {
	      y = (data >> ((x & 1) ? 24 : 8)) & 0xFF;
	      u = (data >> 0) & 0xFF;
	      v = (data >> 16) & 0xFF;
	      expandedPixels[npixels++] = (y << 16) | (u << 8) | v;
	  }
	  return npixels;

      case SSTG_PIXFMT_32BPP:
	  expandedPixels[0] = data;
	  GDBG_INFO(144,"\t       expanded[0]: 0x%08x\n",expandedPixels[0]);
	  return 1;
      default: GDBG_ERROR("expand", "invalid source pixel format\n");
	  return 0;
    }
}

//----------------------------------------------------------------------
// HOST_BLT command: host to screen, 
//----------------------------------------------------------------------
static void hostBlt(SstRegs *sst, SstGRegs *sstg, FxU32 data)
{
    // XXX NOTE: these statics prevent us from running multiple boards
    // they should be moved into the CSIM PRIVATE area
    static int xd,yd, xd_save;	// x,y destination
    static int xsize,ysize;
    static FxU32 monotrans;
    static FxU32 current_byte, current_bit;
    static FxU32 new_row, initial_byte_save, initial_bit_save;
    static FxU32 stride, data_all_used;
    static FxU32 yinc;
    FxU32 col, i,pixCount, expandedPixels[32];
    FxU32 srcPack, srcFormat, pixFormat;

    srcPack = sstg->srcFormat & SSTG_SRC_PACK;
    srcFormat = sstg->srcFormat;
    pixFormat = srcFormat & SSTG_SRC_FORMAT;
    stride = (sstg->srcFormat & SSTG_SRC_LINEAR_STRIDE) >> SSTG_SRC_STRIDE_SHIFT;
    
    // if we are not launced then this is the first data write
    // so snapshot all the relevant data
    if (!CSIM_PRIVATE(sst)->launched)
    {
	sstgPrintOut(sstg,"SSTG_HOST_BLT");

	InHostBlit = FXTRUE;

	xd = SIGN_EXTEND(LOWORD(sstg->dstXY),SSTG_XY_SIZE);
	yd = SIGN_EXTEND(HIWORD(sstg->dstXY),SSTG_XY_SIZE);
        xsize = LOWORD(sstg->dstSize) & SST_MASK(SSTG_XY_SIZE);
        ysize = HIWORD(sstg->dstSize) & SST_MASK(SSTG_XY_SIZE);

	if (xsize == 0)
	    GDBG_INFO(140, "hostBlt(): hostBlt has zero width\n");

	if (ysize == 0)
	    GDBG_INFO(140, "hostBlt(): hostBlt has zero height\n");

	xd_save = xd;		// save this away because we update dstXY
	
	monotrans = (sstg->command & SSTG_TRANSPARENT) &&
		(pixFormat == SSTG_PIXFMT_1BPP);
	last_count = 0;			// reset partial data counter

	updateXY(sstg);			// optionally update dstXY

	if (pixFormat == SSTG_PIXFMT_1BPP)
	{
	    current_byte = (sstg->srcXY & 0x1F) / 8;
	    current_bit = (sstg->srcXY & 0x1F) % 8;
	}
	else
	{
	    // color bitmap
	    current_byte = sstg->srcXY & 3;
	    if ((pixFormat == SSTG_PIXFMT_15BPP) ||
		(pixFormat == SSTG_PIXFMT_16BPP))
	    {
		current_byte &= ~1;	/* force 16 bit starting offset */


		if ((srcPack == SSTG_SRC_PACK_SRC) && (stride & 1))
		{
		    GDBG_ERROR("hostBlt", "stride unaligned w/srcFormat");
		}
	    }
	    else if (pixFormat == SSTG_PIXFMT_32BPP)
	    {
		current_byte = 0;
		if ((srcPack == SSTG_SRC_PACK_SRC) && (stride & 3))
		{
		    GDBG_ERROR("hostBlt", "stride unaligned w/srcFormat");
		}
	    }
	    else if ((pixFormat == SSTG_PIXFMT_422YUV) ||
		     (pixFormat == SSTG_PIXFMT_422UYV))
	    {
		// mixture of 15/16bpp and 32bpp.  Can select the second pixel
		// in a 32bit word, but must have a 32-bit aligned stride
		current_byte &= ~1;
		if ((srcPack == SSTG_SRC_PACK_SRC) && (stride & 3))
		{
		    GDBG_ERROR("hostBlt", "stride unaligned w/srcFormat");
		}
	    }
	}

	if (srcPack == SSTG_SRC_PACK_8)
	    current_bit = 0;
	else if (srcPack == SSTG_SRC_PACK_16)
	{
	    current_byte &= ~1;		/* force 16 bit starting offset */
	    current_bit = 0;
	}
	else if (srcPack == SSTG_SRC_PACK_32)
	{
	    current_byte = 0;
	    current_bit = 0;
	}

	new_row = 0;
	initial_byte_save = current_byte;
	initial_bit_save  = current_bit;
	last_count = 0;
	last_data = 0xdeaddead;

	yinc = sstg->command & SSTG_YDIR ? -1 : 1;
    }

    if ((xsize == 0) || (ysize == 0))
    {
	// terminate host blit
	// XXX need to really force end of command 
	dontMessWithLaunched = FXTRUE;
	InHostBlit = FXFALSE;
	HostBlitComplete = FXFALSE;
	CSIM_PRIVATE(sst)->launched = 0;
	return;
    }

    if (new_row)
    {
	// this is only used when going to a new scanline in a stride packed
	// source bitmap
	current_byte = (initial_byte_save + (stride % 4)) % 4;
	initial_byte_save = current_byte; // save for next scanline
	current_bit = initial_bit_save;
	new_row = 0;
    }

    pixCount = expand(sstg, current_byte, current_bit, data, expandedPixels);
    
    // now process one pixel at a time
    for (i=0; i<pixCount; i++)
    {
	col = expandedPixels[i];
	GDBG_INFO(148,"\tusing [%d]\n",i);

	// for 1bpp source format, transparent bit applies
	if (monotrans && !(col&1)) {
	    GDBG_INFO(148,"\t               : transparent\n");
	    GDBG_INFO(149,"\t---- done pixel ---- %d,%d ----\n",xd,yd);
	}
	else {
	    int srcKey = 0;
	    if (sstg->commandEx & SSTG_EN_SRC_COLORKEY_EX) {
		srcKey = colorKey(col,srcFormat,
				sstg->srcColorkeyMin,sstg->srcColorkeyMax);
	    }
	    col = csimColorConvert(sstg, col);	// apply color conversion
	    GDBG_INFO(141,"\tafter src convt: 0x%08x\n",col);
	    doPixel(sst,sstg, xd,yd, col,srcKey);	// then throw it into the pixel pipe
	}

	xd += 1;
	xsize--;

	if (xsize <= 0)
	{		// done with this row
	    xd = xd_save;
	    yd += yinc;
            xsize = LOWORD(sstg->dstSize) & SST_MASK(SSTG_XY_SIZE);
	    ysize--;
	    if (ysize <= 0)
	    {
		GDBG_INFO(140, "HOSTBLT done\n");
		InHostBlit = FXFALSE;
		HostBlitComplete = FXTRUE;
		// terminate host blit
		dontMessWithLaunched = FXTRUE;
		CSIM_PRIVATE(sst)->launched = 0;
		return;
	    }
	    else if (srcPack == SSTG_SRC_PACK_SRC)
	    {
		new_row = 1;
		last_count = 0; // no saved bytes across stride packed rows
		return;		// terminate this dword
	    }

	    switch (srcPack)
	    {
	      case SSTG_SRC_PACK_32:
		  last_count = 0;	// throw away partial data for 24bpp
		  i = pixCount;	// terminate this DWORD
		  break;
	      case SSTG_SRC_PACK_8:	// src is either 1,8 or 24bpp
		  if (pixFormat == SSTG_PIXFMT_1BPP)
		  {
		      int j = 32 - pixCount;	// number of bits we skipped
		      j += i+1;		// actual bit position we're at
		      j &= 7;
		      if (j)
			  i += 8-j;	// skip some
		  }			// 8,24 bpp just keep going....
		  break;
	      case SSTG_SRC_PACK_16:	// src is either 1,8,15,16 or 24bpp
		  if (pixFormat == SSTG_PIXFMT_1BPP)
		  {
		      int j = 32 - pixCount;	// number of bits we skipped
		      j += i+1;		// actual bit position we're at
		      j &= 15;
		      if (j)
			  i += 16-j;	// skip some
		  }
		  else if (pixFormat == SSTG_PIXFMT_8BPP)
		  {
		      int j = 4 - pixCount;	// number of bytes we skipped
		      if ((j+i+1)&1)
			  i++;
		  }
		  else if (pixFormat == SSTG_PIXFMT_24BPP)
		  {
		      if (last_count == 1)	// if one byte left over,
			  last_count = 0;	// throw away partial data for 24bpp
		      if (pixCount==2 && i==0)
		      {	// recycle 2 of 3 bytes
			  i = pixCount;
			  last_count = 2;
			  last_data = expandedPixels[1] >> 8;
		      }
		  }
	    }
	}
    }

    // data word exhausted, but row isn't done yet, always start at
    // beginning of next word
    current_byte = 0;
    current_bit = 0;
}

//----------------------------------------------------------------------
// BLT command: screen-to-screen copy a rectangle defined by (dstXY,dstSize)
//----------------------------------------------------------------------
static void blt(SstRegs *sst, SstGRegs *sstg)
{
    int xs,ys;		// x,y source
    int xd,yd;		// x,y destination
    int xinc,yinc;	// x,y increments
    int xsize,ysize;
    FxU32 monotrans, col;

    ys = SIGN_EXTEND(HIWORD(sstg->srcXY),SSTG_XY_SIZE);
    yd = SIGN_EXTEND(HIWORD(sstg->dstXY),SSTG_XY_SIZE);
    if (ys < 0) GDBG_ERROR("blt", "source y is negative: %d\n",ys);

    xinc = sstg->command & SSTG_XDIR ? -1 : 1;
    yinc = sstg->command & SSTG_YDIR ? -1 : 1;
    monotrans = (sstg->command & SSTG_TRANSPARENT) &&
		((sstg->srcFormat & SSTG_SRC_FORMAT) == SSTG_PIXFMT_1BPP);

    for (ysize = HIWORD(sstg->dstSize) & SST_MASK(SSTG_XY_SIZE); ysize > 0; ysize--) {
	xs = SIGN_EXTEND(LOWORD(sstg->srcXY),SSTG_XY_SIZE);
	xd = SIGN_EXTEND(LOWORD(sstg->dstXY),SSTG_XY_SIZE);
	if (xs < 0) GDBG_ERROR("blt", "source x is negative: %d\n",xs);

        for (xsize = LOWORD(sstg->dstSize) & SST_MASK(SSTG_XY_SIZE); xsize > 0; xsize--) {
	    // read the source pixel
	    col = csimReadPixel(sst,CSIM_BUF_2D_SRC,xs,ys);
	    GDBG_INFO(140,"\tbltsource pixel: 0x%08x\n",col);

	    // for 1bpp source format, transparent bit applies
	    if (monotrans && !(col&1)) {
		GDBG_INFO(148,"\t               : transparent\n");
		GDBG_INFO(149,"\t---- done pixel ---- %d,%d ----\n",xd,yd);
	    }
	    else {
		int srcKey = 0;
		if (sstg->commandEx & SSTG_EN_SRC_COLORKEY_EX) {
		    srcKey = colorKey(col,sstg->srcFormat,
				      sstg->srcColorkeyMin,sstg->srcColorkeyMax);
		}
	        col = csimColorConvert(sstg, col);	// apply color conversion
		GDBG_INFO(141,"\tafter src convt: 0x%08x\n",col);
	        doPixel(sst,sstg, xd,yd, col,srcKey);	// then throw it into the pixel pipe
	    }
	    xs += xinc;
	    xd += xinc;
	}
	ys += yinc;
	yd += yinc;
    }

    updateXY(sstg);			// optionally update dstXY
}

//----------------------------------------------------------------------
// STRETCH_BLT command: screen-to-screen copy a rectangle defined by (dstXY,dstSize)
// we use the 2 polygon edge iterators to do the sampling for us
// the Left iterator does the X sampling, the Right iterator does the Y sampling
// By rasterizing a dummy line, the iterator's X term gives us the destination
// location and the Y term gives us the source location.  We run the loops
// for each pixel in the destination, so we run the bres iterators until
// the destination term moves (the X in the bres iterator moves).  Then we
// can read the source location from the Y term in the bres iterator.
//----------------------------------------------------------------------
static void stretchBlt(SstRegs *sst, SstGRegs *sstg)
{
    int xs,ys;			// x,y source
    int xd,yd, xdend;		// x,y destination
    int xinc,yinc;		// x,y increments
    int xdsize,ydsize;		// x,y dest sizes
    int xssize,yssize;		// x,y src sizes
    FxU32 monotrans, col;
    CsimPrivate *cpriv = CSIMG_PRIVATE(sstg);

    ys = SIGN_EXTEND(HIWORD(sstg->srcXY),SSTG_XY_SIZE);
    yd = SIGN_EXTEND(HIWORD(sstg->dstXY),SSTG_XY_SIZE);
    if (ys < 0) GDBG_ERROR("stretchblt", "source y is negative: %d\n",ys);

    xssize = LOWORD(sstg->srcSize) & SST_MASK(SSTG_XY_SIZE);
    yssize = HIWORD(sstg->srcSize) & SST_MASK(SSTG_XY_SIZE);
    ydsize = HIWORD(sstg->dstSize) & SST_MASK(SSTG_XY_SIZE);
#if 0
    if (xssize <= 0) GDBG_ERROR("stretchblt", "source x size is <= 0: %d\n",xssize);
    if (yssize <= 0) GDBG_ERROR("stretchblt", "source y size is  <= 0: %d\n",yssize);
    if (ydsize <= 0) GDBG_ERROR("stretchblt", "dest y size is  <= 0: %d\n",ydsize);
#endif
    xinc = sstg->command & SSTG_XDIR ? -1 : 1;
    yinc = sstg->command & SSTG_YDIR ? -1 : 1;
    if (xinc == -1)
	GDBG_ERROR("stretchblt", "x direction bit set\n");
    if (yinc == -1)
	GDBG_ERROR("stretchblt", "y direction bit set\n");
    xinc = 1;
    yinc = 1;

    // setup the right iterator to perform the Y sampling
    bresSetup(sstg, &cpriv->bRight, sstg->bresError1,
		yd,ys, yd + yinc*ydsize, ys + yinc*yssize);

    monotrans = (sstg->command & SSTG_TRANSPARENT) &&
		((sstg->srcFormat & SSTG_SRC_FORMAT) == SSTG_PIXFMT_1BPP);
    for (; ydsize > 0; ydsize--) {
	yd = cpriv->bRight.x;			// get dst Y from iterator.X
	ys = cpriv->bRight.y;			// get src Y from iterator Y
	bresIterate(&cpriv->bRight);
	if (yssize > ydsize) {			// if decimating in Y
	    while (cpriv->bRight.x == yd) {	// run the iterator until it moves
		ys = cpriv->bRight.y;		// remember previous src Y
		bresIterate(&cpriv->bRight);
	    }
	}

	// setup the left iterator to perform X sampling
	xs = SIGN_EXTEND(LOWORD(sstg->srcXY),SSTG_XY_SIZE);
	xd = SIGN_EXTEND(LOWORD(sstg->dstXY),SSTG_XY_SIZE);
	if (xs < 0) GDBG_ERROR("stretchblt", "source x is negative: %d\n",xs);
	xdsize = LOWORD(sstg->dstSize) & SST_MASK(SSTG_XY_SIZE);
	xdend = xd + xinc*xdsize;
	bresSetup(sstg, &cpriv->bLeft, sstg->bresError0,
			xd,xs, xdend, xs + xinc*xssize);

	for (; xdsize > 0; xdsize--) {
	    xd = cpriv->bLeft.x;		// get dst X from iterator X
	    xs = cpriv->bLeft.y;		// get src X from iterator Y
	    bresIterate(&cpriv->bLeft);
	    if (xssize > xdsize) {		// if decimating in X
		while (cpriv->bLeft.x == xd) {	// run the iterator until it moves
		    xs = cpriv->bLeft.y;	// remember previous src X
		    bresIterate(&cpriv->bLeft);
		}
	    }

	    // read the source pixel (bLeft.y contains source X)
	    col = csimReadPixel(sst,CSIM_BUF_2D_STRETCH_SRC,xs,ys);
	    GDBG_INFO(140,"\tbltsource pixel: 0x%08x\t%d,%d\n",col,xs,ys);

	    // for 1bpp source format, transparent bit applies
	    if (monotrans && !(col&1)) {
		GDBG_INFO(148,"\t               : transparent\n");
		GDBG_INFO(149,"\t---- done pixel ---- %d,%d ----\n",xd,yd);
	    }
	    else {
		int srcKey = 0;
		if (sstg->commandEx & SSTG_EN_SRC_COLORKEY_EX) {
		    srcKey = colorKey(col,sstg->srcFormat,
				      sstg->srcColorkeyMin,sstg->srcColorkeyMax);
		}
	        col = csimColorConvert(sstg, col);	// apply color conversion
		GDBG_INFO(141,"\tafter src convt: 0x%08x\n",col);
	        doPixel(sst,sstg, xd,yd, col,srcKey);	// then throw it into the pixel pipe
	    }
	}
    }

    updateXY(sstg);			// optionally update dstXY
}


//----------------------------------------------------------------------
// RECTFILL command: fill a rectangle defined by (dstXY,dstSize) with
//	colorFore.  Note that srcFormat does NOT effect colorFore
//----------------------------------------------------------------------
static void rectFill(SstRegs *sst, SstGRegs *sstg)
{
    int x,y,xstop,ystop;

    x = SIGN_EXTEND(LOWORD(sstg->dstXY),SSTG_XY_SIZE);
    y = SIGN_EXTEND(HIWORD(sstg->dstXY),SSTG_XY_SIZE);
    xstop = x + LOWORD(sstg->dstSize);
    ystop = y + HIWORD(sstg->dstSize);
    GDBG_INFO(127,"===RECTFILL: %d,%d to %d,%d\n",x,y,xstop-1,ystop-1);

    for (; y < ystop; y++)
    for (x = SIGN_EXTEND(LOWORD(sstg->dstXY),SSTG_XY_SIZE); x < xstop; x++) {
	doPixel(sst,sstg,x,y,sstg->colorFore,0);	// process the pixel
    }

    updateXY(sstg);
}


//----------------------------------------------------------------------
// POLYFILL command: fill a polygond, edge at a time
//----------------------------------------------------------------------
static void polyFill(SstRegs *sst, SstGRegs *sstg, FxU32 data)
{
    int x,y, l0x,l0y,r0x,r0y;
    int nextY;
    CsimPrivate *cpriv = CSIMG_PRIVATE(sstg);

    static int lastY;

    if (sstg->commandEx & SSTG_EN_SRC_COLORKEY_EX)
	GDBG_ERROR("polyFill","source chromakeying not allowed\n");

    l0x = SIGN_EXTEND(LOWORD(sstg->srcXY),SSTG_XY_SIZE);
    l0y = SIGN_EXTEND(HIWORD(sstg->srcXY),SSTG_XY_SIZE);
    r0x = SIGN_EXTEND(LOWORD(sstg->dstXY),SSTG_XY_SIZE);
    r0y = SIGN_EXTEND(HIWORD(sstg->dstXY),SSTG_XY_SIZE);
    if (!cpriv->launched) {
	GDBG_INFO(127,"===POLYFILL.init: L0=%d,%d  R0=%d,%d\n",l0x,l0y,r0x,r0y);
	if (l0y != r0y)
	    GDBG_ERROR("polyFill","l0y != r0y\n");
	cpriv->lastY = r0y;
    }
    
    x = SIGN_EXTEND(LOWORD(data),SSTG_XY_SIZE);
    y = SIGN_EXTEND(HIWORD(data),SSTG_XY_SIZE);
    if (l0y <= r0y) {		// replace the edge that runs out first
	sstg->srcXY = data;
	nextY = y < r0y ? y : r0y;
	GDBG_INFO(128,"===POLYFILL: L1=%d,%d  nextY=%d\n",x,y,nextY);
	bresSetup(sstg, &cpriv->bLeft, 0, l0x,l0y, x,y);
    }
    else {
	sstg->dstXY = data;
	nextY = y < l0y ? y : l0y;
	GDBG_INFO(128,"===POLYFILL: R1=%d,%d  nextY=%d\n",x,y,nextY);
	bresSetup(sstg, &cpriv->bRight, 0, r0x,r0y, x,y);
    }

    // fill all the scanlines up to nextY
    while (cpriv->lastY < nextY) {
	int xend;

	// XXX sanity check , keep this in for a while
	if (cpriv->bLeft.y != cpriv->lastY)
	GDBG_ERROR("polyFill","cpriv->bLeft.y != cpriv->lastY\n");

	x = cpriv->bLeft.x;			// get initial span limits
	xend = cpriv->bRight.x;

	if (cpriv->bLeft.dx < 0) {		// if left edge slopes down to the left
	    while (cpriv->bLeft.y == cpriv->lastY) {	// iterate the left edge
		x = cpriv->bLeft.x;			// until it drops down
		bresIterate(&cpriv->bLeft);		// save the previous X
	    }
	}
	if (cpriv->bRight.dx < 0) {		// if right edge slopes down to the left
	    while (cpriv->bRight.y == cpriv->lastY) {	// iterate the right edge
		xend = cpriv->bRight.x;			// until it drops down
		bresIterate(&cpriv->bRight);		// save the previous X
	    }
	}

	GDBG_INFO(130,"   polyspan.y=%d x=[%d,%d)\n",cpriv->lastY,x,xend);
	while (x < xend) {
	    doPixel(sst,sstg,x,cpriv->lastY,sstg->colorFore,0);
	    x++;
	}

	while (cpriv->bLeft.y == cpriv->lastY)		// now iterate the left edge
	    bresIterate(&cpriv->bLeft);			// until it drops down
	while (cpriv->bRight.y == cpriv->lastY)		// now iterate the right edge
	    bresIterate(&cpriv->bRight);		// until it drops down
	cpriv->lastY++;
    }
}

//----------------------------------------------------------------------
// EXECution procedure
//	NOTE: for maximum performance compile without GDBG_INFO_ON
//----------------------------------------------------------------------
void sstgGo(SstRegs *sst, SstGRegs *sstg)
{
// GMT: don't bother with this now 
//    if (!gdbg_get_debuglevel(0)) return;	// if benchmarking then exit now

    switch (sstg->command & SSTG_COMMAND) {	// decode command
	case SSTG_NOP:
		sstgPrintOut(sstg,"SSTG_NOP");
		break;
	case SSTG_BLT:				// screen-to-screen blit
		sstgPrintOut(sstg,"SSTG_BLT");
		blt(sst,sstg);
		break;
	case SSTG_STRETCH_BLT:			// stretch blit
		sstgPrintOut(sstg,"SSTG_STRETCH_BLT");
		stretchBlt(sst,sstg);
		break;
	case SSTG_HOST_BLT:			// host blit
		sstgPrintOut(sstg,"SSTG_HOST_BLT");
		GDBG_ERROR("sstgGo", "host blit with GO is invalid\n");
		break;
	case SSTG_HOST_STRETCH_BLT:		// host stretch blit
		sstgPrintOut(sstg,"SSTG_HOST_STRETCH_BLT");
		GDBG_ERROR("sstgGo", "host stretch blt with GO is invalid\n");
		break;
	case SSTG_RECTFILL:			// rectangle fill
		sstgPrintOut(sstg,"SSTG_RECTFILL");
		rectFill(sst,sstg);
		break;
	case SSTG_LINE:				// line
		sstgPrintOut(sstg,"SSTG_LINE");
		line(sst,sstg,0);
		break;
	case SSTG_POLYLINE:			// polyline
		sstgPrintOut(sstg,"SSTG_POLYLINE");
		line(sst,sstg,1);		// skip last pixel
		break;
	case SSTG_POLYFILL:			// polygon fill
		sstg->dstXY = sstg->srcXY;
		sstgPrintOut(sstg,"SSTG_POLYFILL");
		// nothing to do!
		break;
	default:
		GDBG_ERROR("sstgGo","bad COMMAND = 0x%x\n",sstg->command);
		break;
    }

    //------------------------------------------------------------------
    // all done
    //------------------------------------------------------------------
#ifdef GDBG_INFO_ON
//    if (GDBG_GET_DEBUGLEVEL(130)) sstgPrintRegs(sstg,"> GO:");
//    if (GDBG_GET_DEBUGLEVEL(125)) gdbg_printf("\n");
#endif
}

#ifdef __SST2_H__
#include "sst2asm.h"
#else
#include "h3asm.h"
#endif

void sstgLaunch(SstRegs *sst, SstGRegs *sstg, FxU32 data)
{
    switch (sstg->command & SSTG_COMMAND) {	// decode command
	case SSTG_NOP:
		GDBG_ERROR("launch","launch NOP not valid\n");
		break;
	case SSTG_BLT:				// screen-to-screen blit
	    if (sstg->command & SSTG_XDIR)
	    {
		if ((sstg->commandEx & SSTG_EN_SRC_COLORKEY_EX) ||
		    (sstg->commandEx & SSTG_EN_DST_COLORKEY_EX))
		{
		    GDBG_ERROR("launch", "-x blit with colorkey\n");
		}
#define GIMME(reg, regmask) ((reg & regmask) >> regmask##_SHIFT)
		if (GIMME(sstg->srcFormat, SSTG_SRC_FORMAT) !=
		    GIMME(sstg->dstFormat, SSTG_DST_FORMAT))
		{
		    GDBG_ERROR("launch", "-x blit with color conversion\n");
		}
	    }
	case SSTG_STRETCH_BLT:			// stretch blit
		sstg->srcXY = data & csimRegister2dInfo(SRCXY)->mask;
		sstgGo(sst, sstg);
		break;
	case SSTG_HOST_BLT:			// host blit
		hostBlt(sst, sstg, data);
		break;
	case SSTG_HOST_STRETCH_BLT:		// host stretch blit
		GDBG_ERROR("launch","launch HOST_STRETCH_BLT nyi\n");
		break;
	case SSTG_LINE:				// line
	case SSTG_POLYLINE:			// polyline
	case SSTG_RECTFILL:			// rectangle fill
		sstg->dstXY = data & csimRegister2dInfo(DSTXY)->mask;
		sstgGo(sst, sstg);
		break;
	case SSTG_POLYFILL:			// polygon fill
		polyFill(sst, sstg, data);
		break;
	default:
		GDBG_ERROR("launch","bad launch COMMAND = 0x%x\n",sstg->command);
		break;
    }
}
