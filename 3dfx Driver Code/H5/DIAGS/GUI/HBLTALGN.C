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
** $Date: 10/11/00 8:11:19 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "../csim/h3sim.h"

static FxU8 *shadowMem;

static void fault() 
{
    *(int *)0 = 0;
}

static FxU32 host_pixels[MAXSCREEN*MAXSCREEN];

char *
Xusage()
{
    gdbg_printf("\n");
    gdbg_printf("\"-xd #\"\tforce single dst format 0=>32, 1=>8, 2=>16, 3=>24 (default all)\n");
    gdbg_printf("\"-xs #\"\tforce single src format 0=>1bpp, 1=>8, 2=>16, 3=>24, 4=>32 \n\t5=>422YUV 6=>422UYV (def. all)\n");
    gdbg_printf("\"-xS #\"\tforce initial alignment (default all per pixfmt)\n");
    gdbg_printf("\"-xh #\"\tforce height (default 2)\n");
    gdbg_printf("\"-xH #\"\trandom heights from 0 to # (default 2)\n");
    gdbg_printf("\"-xp #\"\tforce srcpack 0=>stride, 1=>8, 2=>16, 3=>32 (default all)\n");
    gdbg_printf("\"-xw #\" width factor (default 1) (units of 128 bits)\n");
    gdbg_printf("\"-xX\"\t randomly perturb destination X (default 128-bit aligned)\n");

    gdbg_printf("\n");
    exit(1);
    return(0);
}


/* these are used for "wrapping" small scan line output
 * from the bottom of the display screen to the top again, so that multiple
 * passes don't overlap
 */
static FxU32 baseX = 0;		/* x origin of all coordinates */
static FxU32 baseXInc = 128;
static FxU32 baseY = 0;		/* y origin of all coordinates */

static FxBool dstFmtSpecified = 0;
static FxU32 dstFmtInitialIndex = 0;
static FxBool widthFactor = 1;	/* max width in units of 128-bits */
static FxU32 height = 2;

static FxBool doSolid = 1;
static FxBool doMonoPat = 1;
static FxBool doColorPat = 1;

static FxBool doForcePatIndex = 0;
static FxU32 forcePatIndex = 0;

static FxU32 pat0;

static FxU32 patterns[] = { 0x0, 0xFF, 0xCC, 0x33, 0xF0, 0x0F, 0x5a, 0xA5 };
static int patIndex;
static int NpatIndeces = sizeof(patterns) / sizeof(FxU32);

FxU32 dstFormat, sfIndex;
FxU32 srcFmtInitialIndex = 0;
FxU32 srcFormat;
FxU32 pixFormat;
FxU32 pixFormatSpecified = 0;
FxU32 packIndex, packInitialIndex = 0;
FxU32 packingSpecified = 0;
FxU32 packing, maxSrcX;
FxU32 srcX, dstX;
FxU32 strideIndex;
FxU32 stride_add;
FxU32 srcY, dstY;
FxU32 pix;
FxU32 doRandomDstX = 0;

FxU32 srcXInitial = 0;
FxU32 srcXspecified = 0;

FxU32 doRandomHeight = 0;
FxU32 maxRandomHeight;

char *packListString[] =
{
    "SSTG_SRC_PACK_SRC",
    "SSTG_SRC_PACK_8",
    "SSTG_SRC_PACK_16",
    "SSTG_SRC_PACK_32"
};

char *pixfmtstring[] =
{
    "SSTG_PIXFMT_1BPP",
    "SSTG_PIXFMT_8BPP",    
    "SSTG_PIXFMT_16BPP",
    "SSTG_PIXFMT_24BPP",
    "SSTG_PIXFMT_32BPP",
    "SSTG_PIXFMT_422YUV",
    "SSTG_PIXFMT_422UYV"
};
/* myParseOpts
 *
 * look for "-x" options and interpret them for this test
 *
 */

#define XGETARG() opts[1] ? done = 1, ++opts : \
			  (--argc > 0) ? done = 1, *++argv : \
					 (char *)Xusage()

void
XParseOpts(int argc, char **argv)
{
    char *opts = 0;
    FxBool aopt = 0;
    FxU32 bopt = 0;
    FxBool done;
    
    while ((--argc > 0) && (**++argv))
    {
	if (argv[0][0] != '-')
	    continue;
	if (argv[0][1] != 'x')
	    continue;

	/* now parse all extended parameters */
	done = 0;
	opts = &argv[0][2];
	if (*opts == '\0')
	    Xusage();
	
	while (!done && *opts)
	{
	    switch (*opts)
	    {
	      case 'a':
		  aopt = 1;
		  break;
	      case 'b':
		  sscanf(XGETARG(), "%i", &bopt);
		  break;
	      case 'c':
		  doColorPat = 0;
		  GDBG_PRINTF("INFO: color pattern case turned off\n");
		  break;
	      case 'd':
		  sscanf(XGETARG(), "%i", &dstFmtInitialIndex);
		  dstFmtSpecified = 1;
		  break;
	      case 'h':
		  sscanf(XGETARG(), "%i", &height);
		  gdbg_printf("INFO: height set to %d\n", height);
		  break;
	      case 'H':
		  sscanf(XGETARG(), "%i", &maxRandomHeight);
		  gdbg_printf("INFO: random heights: 0 to %d\n",
			      maxRandomHeight);
		  doRandomHeight = 1;
		  break;
	      case 'm':
		  doMonoPat = 0;
		  gdbg_printf("INFO: mono pattern case turned off\n");
		  break;
	      case 'p':
		  sscanf(XGETARG(), "%i", &packInitialIndex);
		  packingSpecified = 1;
		  gdbg_printf("INFO: forcing packing to %s\n",
			      packListString[packInitialIndex]);
		  break;


		  
	      case 's':
		  sscanf(XGETARG(), "%i", &srcFmtInitialIndex);
		  pixFormatSpecified = 1;
		  gdbg_printf("INFO: forcing pixfmt to %s\n",
			      pixfmtstring[srcFmtInitialIndex]);
		  break;

	      case 'S':
		  sscanf(XGETARG(), "%i", &srcXInitial);
		  srcXspecified = 1;
		  gdbg_printf("INFO: forcing srcX to %d\n", srcXInitial);
		  break;
		  
	      case 'w':
		  sscanf(XGETARG(), "%i", &widthFactor);
		  break;

	      case 'X':
		  doRandomDstX = 1;
		  gdbg_printf("INFO: randomly perterbed dstXs\n");
		  break;

	      default:
		  Xusage();
	    }
	    
	    opts += 1;
	}
    }
}


/* tests all possible 128-bit access byte rails for solid, mono pattern,
 * and color pattern fills
 * Iterates:
 *	foreach dstFormat
 *	 foreach start address within 128-bits
 *	  foreach width within 128-bits (including spilling over)
 *	   solid test
 *	    rops SRCCOPY, DSTINVERT (or optionally all rops, or random rops)
 *	   mono pattern test
 *	    rops PATCOPY (optionally all rops, or random rops)
 *	   color pattern test	
 *	    rops PATCOPY (optionally all rops, or random rops)
 */



static FxU32 pix_to_dstfmt[] = 
{
    SSTG_PIXFMT_32BPP, 
    SSTG_PIXFMT_8BPP,
    SSTG_PIXFMT_16BPP,
    SSTG_PIXFMT_24BPP,
};

static FxU32 srcfmt[] =
{
    SSTG_PIXFMT_1BPP,
    SSTG_PIXFMT_8BPP,    
    SSTG_PIXFMT_16BPP,
    SSTG_PIXFMT_24BPP,
    SSTG_PIXFMT_32BPP,
    SSTG_PIXFMT_422YUV,
    SSTG_PIXFMT_422UYV
};

static FxU32 srcfmt_to_bytespp[] =
{
    1, 1, 2, 3, 4, 4, 4
};


static FxU32 packing_list[] =
{
    SSTG_SRC_PACK_SRC,
    SSTG_SRC_PACK_8,
    SSTG_SRC_PACK_16,
    SSTG_SRC_PACK_32
};

static FxU32 stride_list[] =
{
    0, 1, 2, 3, 128
};

FxU32 NstrideIndeces = sizeof(stride_list) / sizeof(FxU32);


static FxU32 pix_to_bytepp[] = 
{
    4,
    1,
    2,
    3
};

static void dorect(void);

static FxI32 lastX[] = {3, 15, 7, 3};
static char *pix_to_str[] = {"32", "8", "16", "24"};


void
my_do_hblt(SstGRegs *sstg, long srcX, long dstX, long dstY,
	   long width, long height, FxU32 cmdops);

#define BUMP_Y_COORD() \
{ \
    dstY += height; \
    if ((dstY + height - 1) > (FxU32)diago.ymaxscreen) \
    { \
	dstY = baseY; \
	baseX += baseXInc; \
	if ((baseX + maxWidth) > (FxU32)diago.xmaxscreen) \
	    baseX = 0; \
    } \
}


SstRegs *sst;

void
main(int argc, char **argv)
{
    long y, h;
    FxU32 rop, cmdops,cmdXops;
    FxU32 cfore,cback;

    SstGRegs *sstg;
    FxU32 dfIndex, maxWidth;
    long width;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);
    shadowMem = sstg_alloc_shadow_memory();

    dstFmtInitialIndex = 0;

    y = 0;
    cmdXops = 0;
    rop = 0;
    cfore = 0xdeadbeef;
    cback = 0xbabecafe;

    cmdops = SSTG_ROP_SRC << SSTG_ROP0_SHIFT;

    XParseOpts(argc, argv);

    h = height;

    sstg_init_random_memory(shadowMem);
    
    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);
    SET(sstg->rop, 0);
    SET(sstg->commandEx, 0);
    SET(sstg->colorBack, cback);		// now set the colors

  while (DIAG_STARTPASS())			// for each pass
  {
    for (dfIndex = dstFmtInitialIndex;		// for each dstFmt
	  dfIndex < 4; dfIndex++)
    {
        int stride;
	gdbg_info(1, "\n");
	gdbg_info(1, "dst format = %sbpp\n", pix_to_str[dfIndex]);	
	if ( diago.ytiled )
	  stride = CEIL(diago.xmaxscreen*4,SST_TILE_WIDTH);
	else
	  stride = diago.xmaxscreen*4;
	dstFormat = pix_to_dstfmt[dfIndex];
	
	SET(sstg->dstFormat, dstFormat | stride);

	maxWidth = ((128 / 8) / pix_to_bytepp[dfIndex]) * widthFactor;

      for (sfIndex = srcFmtInitialIndex; sfIndex < 7; sfIndex++)
      {
	  pixFormat = srcfmt[sfIndex];
	  if (!sstg_compatible_src_dst(pixFormat, dstFormat))
	      continue;
	  
      for (packIndex = packInitialIndex; packIndex < 4; packIndex++)
      {
	  packing = packing_list[packIndex];
	  if (!sstg_compatible_srcfmt_pack(pixFormat, packing))
	      continue;
	  
	  if (packing == SSTG_SRC_PACK_SRC)
	  {
	      // stride "packing"
	      gdbg_info(1, "stride packing\n");
	  }
	  else
	  {
	      // truly packed
	      gdbg_info(1, "packed: 0x%08x\n", packing);
	  }
	      
	  switch (pixFormat)
	  {
	    case SSTG_PIXFMT_1BPP:
		maxSrcX = 31;
		break;
	    case SSTG_PIXFMT_8BPP:
		maxSrcX = 3;
		break;
	    case SSTG_PIXFMT_16BPP:
	    case SSTG_PIXFMT_422YUV:
	    case SSTG_PIXFMT_422UYV:
		maxSrcX = 2;
		break;
	    case SSTG_PIXFMT_24BPP:
		maxSrcX = 3;
		break;
	    case SSTG_PIXFMT_32BPP:
		maxSrcX = 0;
		break;
	    default:
		gdbg_printf("invalid pixFormat: 0x%x\n", pixFormat);
		fault();
	  }
	  
	  for (srcX = srcXInitial; srcX <= maxSrcX; srcX++)

//	  for (width = 0; width <= diago.tsize; width++)
	  for (width = 0; width < maxWidth * 1.5; width++)
	  for (strideIndex = 0; strideIndex < NstrideIndeces;
	       strideIndex++)
	  {
	      stride_add = stride_list[strideIndex];

	      // for 16bpp and yuv formats, srcx can only be 0 and 2 bytes
	      if (((pixFormat == SSTG_PIXFMT_16BPP) ||
		   (pixFormat == SSTG_PIXFMT_422YUV) ||
		   (pixFormat == SSTG_PIXFMT_422UYV))
		  && ((srcX & 1) != 0))
		  continue;
		  
	      // don't allow strides that aren't a multiple of the pixFormat's
	      // bytespp
	      if (((pixFormat == SSTG_PIXFMT_32BPP) ||
		   (pixFormat == SSTG_PIXFMT_422YUV) ||
		   (pixFormat == SSTG_PIXFMT_422UYV))
		  && ((stride_add & 3) != 0))
		  continue;
	      else if ((pixFormat == SSTG_PIXFMT_16BPP) &&
		       ((stride_add & 1) != 0))
		  continue;
	      
	      pix = 0;

	      srcFormat = pixFormat | packing |
		  ((width * srcfmt_to_bytespp[sfIndex]) + stride_add);

	      SET(sstg->srcFormat, srcFormat);

	      if (doRandomHeight)
		  height = iRandom(maxRandomHeight);
	      
	      // do it and check results
	      my_do_hblt(sstg, srcX,
			 baseX + (doRandomDstX ? iRandom(maxWidth) : 0),
			 dstY, width, height, cmdops);
	      
	      BUMP_Y_COORD();

	      if (packing != SSTG_SRC_PACK_SRC)
		  break;	// packed format, don't iterate stride
	  }

	  if (packingSpecified)
	      break;
      } // packing
      if (pixFormatSpecified)
	  break;
      } // pixFormat
	  if (dstFmtSpecified)
	      break;		// terminate iteration
    } // dstformat

  } // diag pass #

    gdbg_printf("total pixels drawn: %d\n",
		CSIMG_PRIVATE(&CSIM_PRIVATE(diago.sstCSIM)->gui)->pixelsOut2d);

    if (getenv("DIAG_WAIT"))
	getchar();
    
    DIAG_PASS(0);
}


//----------------------------------------------------------------------
// draw a host-to-scr blt,
// note: changed a  bit from hblt.c version, xs is explicit bit/byte
// offset now, and there's no ys
// 
//----------------------------------------------------------------------
void sstg_drawHblt(SstGRegs *sstg, int xs, int xd, int yd, int w, int h, FxU32 cmd)
{
    FxU32 bit, byte;
    
    SET(sstg->command, cmd | SSTG_HOST_BLT);

    // just pass xs right through
    if (pixFormat == SSTG_PIXFMT_1BPP)
    {
	byte = (xs & 0x1F) / 8;
	bit = (xs & 0x1F) % 8;
	SET(sstg->srcXY, (iRandom(0xFFFFFFFF) & ~0x1F) | xs);
    }
    else
    {
	byte = xs & 3;
	SET(sstg->srcXY, (iRandom(0xFFFFFFFF) & ~3) | xs);
    }

    SET(sstg->dstXY,(yd<<16) | (xd & 0xFFFF));
    SET(sstg->dstSize,(h<<16) | w);

    switch(srcFormat & SSTG_SRC_PACK) {
	case SSTG_SRC_PACK_SRC:
		sendBltData0(sstg, byte, bit, xd, yd, w,h, 0 /* ydir */,
			     srcFormat, host_pixels);
		break;
	case SSTG_SRC_PACK_8:
		sendBltDataPacked(sstg, 8, byte, xd,yd, w,h, 0 /* ydir */,
			     srcFormat, host_pixels);
		break;
	case SSTG_SRC_PACK_16:
		sendBltDataPacked(sstg, 16, byte, xd,yd, w,h, 0 /* ydir */,
			     srcFormat, host_pixels);
		break;
	case SSTG_SRC_PACK_32:
		sendBltDataPacked(sstg, 32, byte, xd,yd, w,h, 0 /* ydir */,
			     srcFormat, host_pixels);
		break;
    }
}

// end  snarfed host blit code (from hblt.c)


void
my_do_hblt(SstGRegs *sstg, long srcX, long dstX, long dstY,
	   long width, long height, FxU32 cmdops)
{
    FxU32 monotrans, cdest;
    FxU32 cfore, cback;
    long xc, yc;

    if (pixFormat == SSTG_PIXFMT_1BPP)
    {
	cfore = iRandom(0xFFFFFFFF);
	cback = iRandom(0xFFFFFFFF);
	SET(sstg->colorFore, cfore);
	SET(sstg->colorBack, cback);
    }
	
    gdbg_info(2,"\n");
    gdbg_info(2,"xs,ys = %d,%d    xd,yd = %d,%d    w,h = %d,%d    pox,y = %d,%d\n",
	      srcX, srcY, dstX, dstY, width, height,
	      ((cmdops & SSTG_X_PATOFFSET)>>SSTG_X_PATOFFSET_SHIFT) & 7,
	      ((cmdops & SSTG_Y_PATOFFSET)>>SSTG_Y_PATOFFSET_SHIFT) & 7);
    sstg_print_stuff(cmdops, 0, SSTG_ROP_SRC,
		     pixFormat, cfore, cback);

    sstg_drawHblt(sstg, srcX, dstX, dstY, width, height, cmdops);

    sstg_idle(sst);			// wait for the command to complete

    monotrans = (cmdops & SSTG_TRANSPARENT) &&
	((srcFormat & SSTG_SRC_FORMAT) == SSTG_PIXFMT_1BPP);

    for (yc = dstY-1; yc <= dstY+height; yc++)	// check the entire rectangle
    for (xc = dstX-1; xc <= dstX+width; xc++)	// with a 1 pixel border
	if (ONSCREEN(xc,yc))
	{
	    cdest = sstg_get_screen_color(xc, yc, (FxU8 *)shadowMem);
	    if (xc < dstX || xc >= dstX+width || yc < dstY ||
		yc >= dstY+height)
	    {
		// if outside rect, then unchanged
		cdest = sstg_destination_mask(cdest);
		gdbg_info(8,"checking %d,%d 0x%x (outside)\n",xc,yc,cdest);
		sstg_test_screen_pixel(xc, yc, cdest);
//		DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,xc,yc,cdest); // test the pixel
	    }
	    else
	    {
		FxU32 csrc = host_pixels[pix++];
		if (monotrans && !(csrc&1))
		{
		    gdbg_info(8,"checking %d,%d 0x%x (monotrans)\n",xc,yc,cdest);
		    cdest = sstg_check_pixel(xc,yc, cmdops, 0, SSTG_ROP_SRC,
					     csrc, cdest, 0);
//		    DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,xc,yc,cdest); // test the pixel
		    continue;
		}
		GDBG_INFO(8,"converting color 0x%x\n",csrc);
		// NOTE: call into csim to convert colors
		csrc = csimColorConvert(&CSIM_PRIVATE(diago.sstCSIM)->gui, csrc);
		cdest = sstg_check_pixel(xc,yc,cmdops, 0, SSTG_ROP_SRC,
					 csrc, cdest, 0);
		// update our shadow screen
		sstg_set_screen_color(xc, yc, (FxU8 *)shadowMem, cdest);

//		cdest = sstg_check_pixel(xc,yc,cmdops,cmdXops, rop,csrc,cdest);
//		screen[yc][xc] = cdest;	// update our shadow screen
	    }
	}

}

