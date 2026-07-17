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
** $Date: 10/11/00 8:11:07 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "../csim/h3sim.h"

static void fault() 
{
    *(int *)0 = 0;
}


char *
Xusage()
{
    gdbg_printf("\n");
    gdbg_printf("\"-xd #\"\tforce single dst format 0=>32, 1=>8, 2=>16, 3=>24 (default all)\n");
//    gdbg_printf("\"-xs #\"\tforce single src format 0=>1bpp, 1=>8, 2=>16, 3=>24, 4=>32 (def. all)\n");
    gdbg_printf("\"-xS\"\tshow source rectangles)\n");
//    gdbg_printf("\"-xh #\"\tforce height (default 2)\n");
//    gdbg_printf("\"-xH #\"\trandom heights from 0 to # (default 2)\n");
//    gdbg_printf("\"-xp #\"\tforce srcpack 0=>stride, 1=>8, 2=>16, 3=>32 (default all)\n");
//    gdbg_printf("\"-xw #\" width factor (default 1) (units of 128 bits)\n");
//    gdbg_printf("\"-xX\"\t randomly perturb destination X (default 128-bit aligned)\n");

    gdbg_printf("\n");
    exit(1);
    return(0);
}


static FxBool dstFmtSpecified = 0;
static FxU32 dstFmtInitialIndex = 0;
static FxBool widthFactor = 1;	/* max width in units of 128-bits */
static FxU32 height = 2;

FxU32 dstFormat;
FxU32 pixFormat;
FxU32 pixFormatSpecified = 0;
int srcX, dstX;
FxU32 strideIndex;
FxU32 stride_add;
int srcY, dstY;
FxU32 pix;
FxU32 doRandomDstX = 0;

FxU32 showSrcRect = 0;

FxU32 srcXInitial = 0;
FxU32 srcXspecified = 0;

FxU32 doRandomHeight = 0;
FxU32 maxRandomHeight;

FxU32 clipX, clipYindex, clipY, clipWidth, clipHeight, clipHeightIndex;
FxU32 clipWidthIndex;

FxU32 clipReg, widthIndex, heightIndex;

int dstXquadrant, dstYquadrant;
FxU32 centerPt, xCenter, yCenter, pos;


FxU32 clipRegInitial = 0;
FxU32 clipXinitial = 0;
FxU32 clipYInitialIndex = 0;
FxU32 clipWidthInitial = 0;
FxU32 clipHeightInitialIndex = 0;
FxU32 widthIndexInitial = 0;
FxU32 heightIndexInitial = 0;
int dstXquadInitial = -1;
int dstYquadInitial = -1;
FxU32 clipWidthIndexInitial = 0;
FxU32 centerPtInitial = 0;
FxU32 posInitial = 0;

FxU32 clipYlist[] = {0, 1};
FxU32 clipHeightList[] = {0, 1, 4};
FxU32 heightList[] = {0, 1, 2, 4};
FxU32 res;

FxU32 resXlist[] = {320, 320, 400, 512, 640, 800, 1024, 1152, 1280, 1600};
FxU32 resYlist[] = {200, 240, 300, 384, 480, 600,  768,  864, 1024, 1200};
FxU32 Nresolutions = sizeof(resXlist) / sizeof(FxU32);
FxU32 resInitial = 0;
FxU32 resInitialSpecified = 0;
    
char *pixfmtstring[] =
{
    "SSTG_PIXFMT_1BPP",
    "SSTG_PIXFMT_8BPP",    
    "SSTG_PIXFMT_16BPP",
    "SSTG_PIXFMT_24BPP",
    "SSTG_PIXFMT_32BPP",
    // XXX NEED YUV FORMATS!
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
	      case 'r':
		  sscanf(XGETARG(), "%i", &resInitial);
		  resInitialSpecified = 1;
		  break;
	      case 'S':
		  showSrcRect = 1;
		  gdbg_printf("INFO: showing src rectangle\n");
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
    // XXX NEED YUV FORMATS!
};

static FxU32 srcfmt_to_bytespp[] =
{
    1, 1, 2, 3, 4
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
my_do_blt(SstGRegs *sstg,
	  int srcX, int srcY, int dstX, int dstY, int width, int height,
	  FxU32 cmdops);

SstRegs *sst;

static FxU8 *screen;
static FxU8 *next_screen;

void
main(int argc, char **argv)
{
    int i;
    long y, h;
    int xd, yd;
    FxU32 rop, cmdops,cmdXops;
    FxU32 cfore,cback;

    SstGRegs *sstg;
    FxU32 dfIndex, maxWidth;
    long width ;
    int xs, ys;

//    fxHalPutenv("SST_FBI_MEM=8");

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);
    screen = sstg_alloc_shadow_memory();
    next_screen = sstg_alloc_shadow_memory();

    dstFmtInitialIndex = 0;

    y = 0;
    cmdXops = 0;
    rop = 0;
    cfore = 0xdeadbeef;
    cback = 0xbabecafe;

    cmdops = SSTG_ROP_SRC << SSTG_ROP0_SHIFT;

    XParseOpts(argc, argv);

    h = height;

    sstg_init_random_memory(screen);
    
    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip1min, 0x00000000);
    SET(sstg->rop, 0);
    SET(sstg->commandEx, 0);

  while (DIAG_STARTPASS())			// for each pass
  {
    for (dfIndex = dstFmtInitialIndex;		// for each dstFmt
	  dfIndex < 4; dfIndex++)
    {
        int stride;
	gdbg_info(1, "\n");
	gdbg_info(1, "dst format = %sbpp\n", pix_to_str[dfIndex]);
	dstFormat = pix_to_dstfmt[dfIndex];
	
	if ( diago.ytiled )
	  stride = CEIL(diago.xmaxscreen*4,SST_TILE_WIDTH);
	else
	  stride = diago.xmaxscreen*4;	
	SET(sstg->dstFormat, dstFormat | stride);
	SET(sstg->srcFormat, dstFormat | stride);

	maxWidth = ((128 / 8) / pix_to_bytepp[dfIndex]) * widthFactor;
	
	width = maxWidth + iRandom(maxWidth);
	height = 5 + iRandom(4);

	for (i = 0; i < 100; i++)
	{
	    clipX = (diago.xmaxscreen / 2) + iRandom(maxWidth);
	    clipY = diago.ymaxscreen / 2 + iRandom(2);
	    clipHeight = iRandom(height);
	    clipWidth = iRandom(width);
	    
	    if (i < 50)
	    {
		SET(sstg->clip1min, (clipY << 16) | clipX);
		SET(sstg->clip1max, ((clipY+clipHeight) << 16) |
		    (clipX + clipWidth));
		cmdops |= SSTG_CLIPSELECT;
		gdbg_info(2, "cliprect 1 enabled and set to [%d,%d], ",
			  clipX, clipY);
		gdbg_info_more(2, "(%d,%d)\n", clipX + clipWidth,
			       clipY + clipHeight);
	    }
	    else
	    {
		SET(sstg->clip0min, (clipY << 16) | clipX);
		SET(sstg->clip0max, ((clipY+clipHeight) << 16) |
		    (clipX + clipWidth));
		cmdops &= ~SSTG_CLIPSELECT;
		gdbg_info(2, "cliprect 0 enabled and set to [%d,%d], ",
			  clipX, clipY);
		gdbg_info_more(2, "(%d,%d)\n", clipX + clipWidth,
			       clipY + clipHeight);
	    }

	    xs = iRandom(diago.xmaxscreen - width);
	    ys = iRandom(diago.ymaxscreen - height);

	    xd = clipX - (width/2) + iRandom(width);
	    yd = clipY - (height/2) + iRandom(height);
	    
	    my_do_blt(sstg, xs, ys, xd, yd, width, height, cmdops);

	    if (showSrcRect)
	    {
		// highlight the source rect to make sure we're getting
		// a good distribution
		SET(sstg->clip0min, 0);
		SET(sstg->clip0max, (diago.ymaxscreen << 16) |
		    diago.xmaxscreen);

		SET(sstg->dstXY, (ys << 16) | xs);
		SET(sstg->command, (SSTG_ROP_XOR << SSTG_ROP0_SHIFT) |
		    (SSTG_RECTFILL << SSTG_COMMAND_SHIFT) |
		    SSTG_GO);

	    }
	    
	} // i

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


#define MYONSCREEN(x, y) \
	(((x) > 0) && ((y) > 0) && \
	 ((x) < (int)resXlist[res]) && ((y) < (int)resYlist[res]))

void
my_do_blt(SstGRegs *sstg,
	  int xs, int ys, int xd, int yd, int w, int h,
	  FxU32 cmd)
{
    FxU32 cdest;
    int xc, yc;
    int xsNew, ysNew, xdNew, ydNew;

    gdbg_info(2,"\n");
    gdbg_info(2,"xs,ys = %d,%d    xd,yd = %d,%d    w,h = %d,%d\n",
	      xs,ys, xd,yd, w,h);

    xsNew = xs;
    ysNew = ys;
    xdNew = xd;
    ydNew = yd;
    
    //--------------------------------------------

    cmd &= ~(SSTG_XDIR | SSTG_YDIR);		// clear out the direction bits
    if (xsNew < xdNew)
    {				// compute new direction bits
	cmd |= SSTG_XDIR;			// right to left
	xsNew += w-1;
	xdNew += w-1;
    }
    if (ysNew < ydNew)
    {
	cmd |= SSTG_YDIR;
	ysNew += h-1;
	ydNew += h-1;
    }

    if ((xsNew < 0) || (ysNew < 0))
	return;

    if (iRandom(1)) {				// command+GO
	SET(sstg->srcXY,(ysNew<<16) | (xsNew & 0xFFFF));
	SET(sstg->dstXY,(ydNew<<16) | (xdNew & 0xFFFF));
	SET(sstg->dstSize,(h<<16) | w);
	SET(sstg->command, cmd | SSTG_BLT | SSTG_GO);
    }
    else {					// LAUNCH
	SET(sstg->command, cmd | SSTG_BLT);
	SET(sstg->dstSize,(h<<16) | w);
	SET(sstg->dstXY,(ydNew<<16) | (xdNew & 0xFFFF));
	SET(sstg->launch[0],(ysNew<<16) | (xsNew & 0xFFFF));
    }

    sstg_idle(sst);		// wait for the command to complete

    //--------------------------------------------
    for (yc = yd-1; yc <= yd+h; yc++)	// check the entire rectangle
	for (xc = xd-1; xc <= xd+w; xc++)	// with a 1 pixel border
	    if (ONSCREEN(xc,yc))
	    {
		// compute predicted result
		cdest = sstg_get_screen_color(xc, yc, screen);

		if (xc < xd || xc >= xd+w || yc < yd || yc >= yd+h) {
		    // if outside rect, then unchanged
		    cdest = sstg_destination_mask(cdest);
		    gdbg_info(8,"checking %d,%d 0x%x (outside)\n",xc,yc,cdest);
		    sstg_test_screen_pixel(xc, yc, cdest);
		}
		else
		{
		    FxU32 csrc = sstg_get_screen_color((xc - xd) + xs,
						       (yc - yd) + ys,
						       screen);
gdbg_info(197, "private screen src(%d,%d) = 0x%08x\n",
	  (xc-xd) + xs, (yc-yd)+ys, csrc);


		    cdest = sstg_check_pixel(xc,yc,cmd,
					     0, //cmdXops
					     0, // rop
					     csrc,cdest,
					     0); // srcKey

gdbg_info(197, "...written to dst(%d,%d)as 0x%08x\n", xc, yc, cdest);

		    sstg_set_screen_color(xc, yc, next_screen, cdest);
		}
	    }

    //--------------------------------------------
    for (yc = yd; yc < yd+h; yc++)	// now copy the results
	for (xc = xd; xc < xd+w; xc++)	// to the shadow screen
	    if (ONSCREEN(xc,yc))
	    {
		sstg_set_screen_color(xc, yc, screen,
				      sstg_get_screen_color(xc, yc,
							    next_screen));
	    }
}
