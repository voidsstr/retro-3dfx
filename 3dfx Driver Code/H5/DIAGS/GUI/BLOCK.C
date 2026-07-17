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
** $Date: 10/11/00 8:11:03 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "../csim/h3sim.h"

static FxU8 *shadowMem;



char *
Xusage()
{
    gdbg_printf("\n");
    gdbg_printf("\"-xd #\"\tforce single dst format 0=>32, 1=>8, 2=>16, 3=>24 (default all)\n");
    gdbg_printf("\"-x[cms]\"\tturn off color pat(c), mono pat (m), solid (s) (default all on)\n");
    gdbg_printf("\"-xh #\"\theight (default 1)\n");
    gdbg_printf("\"-xp #\"\tforce single mono pattern (default 8 patterns)\n");
    gdbg_printf("\"-xw #\" width factor (default 1) (units of 128 bits)\n");
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
static FxU32 height = 1;

static FxBool doSolid = 1;
static FxBool doMonoPat = 1;
static FxBool doColorPat = 1;

// hack!  forcing pattern index to 0 to do just 1 pattern 
static FxBool doForcePatIndex = 1;
static FxU32 forcePatIndex = 0;

static FxU32 pat0;

static FxU32 patterns[] = { 0xa6, 0xFF, 0xCC, 0x33, 0xF0, 0x0F, 0x5a, 0xA5 };
static int patIndex;
static int NpatIndeces = sizeof(patterns) / sizeof(FxU32);


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
		  break;
	      case 'm':
		  doMonoPat = 0;
		  gdbg_printf("INFO: mono pattern case turned off\n");
		  break;
	      case 'p':
		  sscanf(XGETARG(), "%i", &forcePatIndex);
		  doForcePatIndex = 1;
		  gdbg_printf("INFO: forcing pattern0 %d = 0x%x\n",
			      forcePatIndex, patterns[forcePatIndex]);
		  break;
		  
	      case 's':
		  doSolid = 0;
		  GDBG_PRINTF("INFO: solid case turned off\n");
		  break;
		  
	      case 'w':
		  sscanf(XGETARG(), "%i", &widthFactor);
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
mydorect(SstRegs *sst,
       SstGRegs *sstg,
       long x, long y, long w, long h,
       FxU32 cmdops,
       FxU32 cmdXops,
       FxU32 rop,
       FxU32 dstFormat,
       FxU32 cfore,
       FxU32 cback);

#define BUMP_Y_COORD() \
{ \
    y += height; \
    if ((y + height - 1) > (FxU32)diago.ymaxscreen) \
    { \
	y = baseY; \
	if ((baseX + minWidth) > (FxU32)diago.xmaxscreen) \
	    baseX = 0; \
    } \
}


void
main (int argc, char **argv)
{
    SstIORegs *sstio;
    long x,y, w,h;
    FxU32 rop, cmdops,cmdXops;
    FxU32 cfore,cback;
    SstRegs *sst;
    SstGRegs *sstg;
    FxU32 dfIndex;
    FxI32 minWidth;
    FxI32 xLeft;
    FxU32 width;
    FxU32 pat;
    FxU32 miscInit1;

//    fxHalPutenv("SST_FBI_MEM=4");

    sst = SST_BEGIN2d(argc,argv);
    sstio = (SstIORegs *) SST_IO_ADDRESS(sst);
    sstg = SSTG_CHIP(sst);
    shadowMem = sstg_alloc_shadow_memory();

    dstFmtInitialIndex = 0;

    y = 0;
    cmdXops = 0;
    rop = 0;
    cfore = 0xdeadbeef;
    cback = 0xbabecafe;

    XParseOpts(argc, argv);

    h = height;

    sstg_init_random_memory(shadowMem);
    
    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);
    SET(sstg->rop, 0);
    SET(sstg->commandEx, 0);
    SET(sstg->colorBack, cback);		// now set the colors

    miscInit1 = GET(sstio->miscInit1);
    if (miscInit1 & SST_DISABLE_2D_BLOCK_WRITE)
    {
	gdbg_printf("!!! WARNING: 2D BLOCK WRITES ARE DISABLED !!!\n");
	gdbg_printf("!!! UNLESS YOU'RE DOING THIS ON PURPOSE, FIX YOUR\n");
	gdbg_printf("!!! ENVIRONMENT!\n");
    }
    miscInit1 &=  SST_BLOCK_WRITE_THRESH;
    miscInit1 >>= SST_BLOCK_WRITE_THRESH_SHIFT;
    if (miscInit1 != 2)
    {
	gdbg_printf("!!! WARNING: 2D BLOCK WRITE THRESHOLD IS NOT THE\n");
	gdbg_printf("!!! EXPECTED VALUE (expected=%d, actual=%d)\n",
		    2, miscInit1);
	gdbg_printf("!!! UNLESS YOU'RE DOING THIS ON PURPOSE, FIX YOUR\n");
	gdbg_printf("!!! ENVIRONMENT!\n");
    }
    
  while (DIAG_STARTPASS())			// for each pass
  {
    // cache pattern0alias
    sstg_setpattern_random(sstg);
    pat0 = CSIM_PRIVATE(diago.sstCSIM)->gui.colorPattern[0];
	
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
	SET(sstg->dstFormat, pix_to_dstfmt[dfIndex] | stride);

	// note, this is a block write maxWidth, assumes that
	// the block write threshold is 512 bits
	minWidth = ((512 / 8) / pix_to_bytepp[dfIndex]) * widthFactor;
	
	for (xLeft = -3; xLeft <= minWidth; xLeft++)
	{
	    // for negative x, just test a few starting points
	    if (xLeft == -3)
		x = -minWidth;
	    else if (xLeft == -2)
		x = -minWidth/2;
	    else
		x = xLeft;

	    for (width = minWidth; (FxI32)width <= (minWidth + 10); width++)
	    {
#if 0
		/* for negative coordinates, just test a subset of widths:
		 * 0, 1, maxwidth / 2, maxwidth, and maxwidth + 1
		 */
		if ((x < 0) && (width == 2))
		    w = (maxWidth / widthFactor) / 2;
		else if ((x < 0) && (width == 3))
		    w = (maxWidth / widthFactor);
		else if ((x < 0) && (width == 4))
		{
		    w = (maxWidth / widthFactor) + 1;
		    width = maxWidth + 1;	// force end of iteration
		}
		else
		    w = width;
#endif
		w = width;
		
		/* solid */
		if (doSolid)
		{
		    gdbg_info(1, "\n");
		    gdbg_info(1, "solid\n");
		    cfore = colRandom32();
		    SET(sstg->colorFore, cfore);
		    cmdops = (FxU32)0xcc << SSTG_ROP0_SHIFT;
		    mydorect(sst, sstg, x+baseX, y+baseY, w, h, cmdops,
			     cmdXops, rop, pix_to_dstfmt[dfIndex],
			     cfore, cback);

		    BUMP_Y_COORD();
		}
		
		if (doMonoPat)
		{
		    gdbg_info(1, "\n");
		    gdbg_info(1, "mono pattern\n");


		    cmdops = (FxU32)0xf0 << SSTG_ROP0_SHIFT;
		    cmdops |= SSTG_MONO_PATTERN;
		    cmdXops = SSTG_PAT_FORCE_ROW0;

		    for (patIndex = 0; patIndex < NpatIndeces; patIndex++)
		    {
			if (doForcePatIndex)
			    patIndex = forcePatIndex;
			
			pat = patterns[patIndex];
			SET(sstg->pattern0alias, (pat0 & ~0xffL) | pat);
			gdbg_info(6, "pattern0 is 0x%08x\n",
			  CSIM_PRIVATE(diago.sstCSIM)->gui.colorPattern[0]);

			// opaque case
			cfore = colRandom32();
			cback = colRandom32();
			SET(sstg->colorFore, cfore);
			SET(sstg->colorBack, cback);
			cmdops &= ~SSTG_TRANSPARENT;
			mydorect(sst, sstg, x+baseX, y+baseY, w, h, cmdops,
				 cmdXops, rop, pix_to_dstfmt[dfIndex],
				 cfore, cback);
			BUMP_Y_COORD();

			// transparent case
			cfore = colRandom32();
			SET(sstg->colorFore, cfore);
			cmdops |= SSTG_TRANSPARENT;
			mydorect(sst, sstg, x+baseX, y+baseY, w, h, cmdops,
				 cmdXops, rop, pix_to_dstfmt[dfIndex],
				 cfore, cback);
			BUMP_Y_COORD();

			if (doForcePatIndex)
			    break;
		    } // for patIndex

		} // if (doMonoPat)
		
		if (doColorPat)
		{
		    gdbg_info(1, "\n");
		    gdbg_info(1, "color pattern\n");

		    // restore the "true" color pattern 0
		    SET(sstg->pattern0alias, pat0);

		    cmdops = (FxU32)0xf0 << SSTG_ROP0_SHIFT;
		    cmdXops = 0;
		    mydorect(sst, sstg, x+baseX, y+baseY, w, h, cmdops,
			     cmdXops, rop, pix_to_dstfmt[dfIndex],
			     cfore, cback);		    
		    BUMP_Y_COORD();
		}
	    } // for width
	} // for xLeft
	if (dstFmtSpecified)
	    break;		// terminate iteration
    }
  }

  gdbg_printf("total pixels drawn: %d\n",
	      CSIMG_PRIVATE(&CSIM_PRIVATE(diago.sstCSIM)->gui)->pixelsOut2d);  


    if (getenv("DIAG_WAIT"))
	getchar();
    
    DIAG_PASS(0);
}

void
mydorect(SstRegs *sst,
       SstGRegs *sstg,
       long x, long y, long w, long h,
       FxU32 cmdops,
       FxU32 cmdXops,
       FxU32 rop,
       FxU32 dstFormat,
       FxU32 cfore,
       FxU32 cback)
{
    long xc, yc, minx, maxx;
    FxU32 cdest;
    
    gdbg_info(2,"x,y = %d,%d    w,h = %d,%d    pox,y = %d,%d\n",
	      x, y, w, h,
	      ((cmdops & SSTG_X_PATOFFSET) >>
	       SSTG_X_PATOFFSET_SHIFT) & 7,
	      ((cmdops & SSTG_Y_PATOFFSET) >>
	       SSTG_Y_PATOFFSET_SHIFT) & 7);
    sstg_print_stuff(cmdops, cmdXops, rop,
		     dstFormat, /* HACK! dstformat as
				 * srcformat (for now)
				 */
		     cfore, cback);

    SET(sstg->commandEx, cmdXops);

    sstg_drawrect(sstg,x,y,w,h, cmdops);
    sstg_idle(sst);			// wait for command to complete

    for (yc = y-1; yc <= y+h; yc++)	// check the entire rectangle
    {
	if ((x+w) < 0)
	{
	    minx = 0;
	    maxx = 0;
	}
	else
	{
	    if ((x == 0) && (w == 0))
	    {
		minx = 0;
		maxx = 1;
	    }
	    else
	    {
		minx = x-1;
		maxx = x+w;
	    }
	    
	}
	
	for (xc = minx; xc <= maxx; xc++)	// with a 1 pixel border
	{
	    if (ONSCREEN(xc,yc))
	    {
		// compute predicted result
		cdest = sstg_get_screen_color(xc, yc, (FxU8 *)shadowMem);
		if (xc < x || xc >= x+w || yc < y || yc >= y+h)
		{
		    // if outside rect, then unchanged
		    gdbg_info(8,"checking %d,%d 0x%x (outside)\n",xc,yc,
			      cdest);
		    // test the pixel
		    sstg_test_screen_pixel(xc, yc, cdest);
		}
		else
		{
		    cdest = sstg_check_pixel(xc,yc,cmdops,cmdXops, rop,
					     cfore, cdest, 0);
		    // update our shadow screen
		    sstg_set_screen_color(xc, yc, (FxU8 *)shadowMem, cdest);
		}
	    }
	}
    }
}
