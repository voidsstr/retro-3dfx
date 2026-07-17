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
** $Date: 10/11/00 8:11:28 PM$
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

static FxBool doForcePatIndex = 0;
static FxU32 forcePatIndex = 0;

static FxU32 pat0;

static FxU32 patterns[] = { 0x0, 0xFF, 0xCC, 0x33, 0xF0, 0x0F, 0x5a, 0xA5 };
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
	      case 'd':
		  sscanf(XGETARG(), "%i", &dstFmtInitialIndex);
		  dstFmtSpecified = 1;
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
main (int argc, char **argv)
{
    FxU32 rop, cmdops,cmdXops;
    FxU32 cfore,cback;
    SstRegs *sst;
    SstGRegs *sstg;
    FxU32 dfIndex;
    FxU32 cdest;
    long xc, yc;
    int maxVerts;
    FxI32 dstX, dstY;
    FxI32 width, height;
//    FxI32 p1x, p1y, p2x, p2y, p3x, p3y;

    int i;
    
    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);
    shadowMem = sstg_alloc_shadow_memory();

    // Print Out Option Description
    if ( diago.printOpts ) {
	gdbg_printf( "poly option description:\n" );
	gdbg_printf( " -O # -> max vertices per polygon, default is 10\n");
	DIAG_FAIL();
    }
    maxVerts = 10;
    if (diago.option) {
	maxVerts = diago.option;
	if (maxVerts < 3) {
	    GDBG_ERROR("main", "maxVerts is too small (%d is <3)\n",maxVerts);
	    DIAG_FAIL();
	}
    }

    dstFmtInitialIndex = 0;

    cmdXops = 0;
    rop = 0;
    cfore = 0xdeadbeef;
    cback = 0xbabecafe;

    XParseOpts(argc, argv);

    // sstg_init_random_memory(shadowMem);
    
    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);
    SET(sstg->rop, 0);
    SET(sstg->commandEx, 0);
    SET(sstg->colorBack, cback);		// now set the colors
    SET(sstg->colorFore, cfore);		// now set the colors

    cmdops = SSTG_ROP_XOR << SSTG_ROP0_SHIFT;

//    sstg_draw_triangle(sstg, 50, 50, 25, 300, 55, 400, cmdops);
//    getchar();
//    exit(0);

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
	    SET(sstg->dstFormat, pix_to_dstfmt[dfIndex] | stride);

	    for (i = 0; i < 20; i++)
	    {
		cfore = iRandom(0xFFFFFF);
		SET(sstg->colorFore, cfore);
		dstX = iRandom(diago.xmaxscreen - 1);
		dstY = iRandom(diago.ymaxscreen - 1);
		width = rRandom(3, diago.tsize);
		height = rRandom(3, diago.tsize);
		sstg_draw_poly_rectangle(sstg, dstX, dstY, width, height,
					 cmdops);

            sstg_idle(sst);			// wait for command to complete

	    // check the entire rectangle, with a 1 pixel border	
	    for (yc = dstY - 1; yc <= dstY+height-1; yc++)
	    for (xc = dstX - 1; xc <= dstX+width-1; xc++)
	    {
	    if (ONSCREEN(xc,yc))
	    {
		// compute predicted result
		cdest = sstg_get_screen_color(xc, yc, (FxU8 *)shadowMem);
		if (xc < dstX || xc >= dstX+width-1 || yc < dstY
		    || yc >= dstY+height-1)
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
	    } // if (ONSCREEN(xc,yc))
	    } // for xc

	    if (DIAG_EXCEEDED_PIXEL_LIMIT())
		break;	    
	    } // i
	    
	    if (dstFmtSpecified)
		break;		// terminate iteration
	}

    } // diag pass

  gdbg_printf("total pixels drawn: %d\n",
	      CSIMG_PRIVATE(&CSIM_PRIVATE(diago.sstCSIM)->gui)->pixelsOut2d);  



    if (getenv("DIAG_WAIT"))
	getchar();
    
    DIAG_PASS(0);
}
