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
** $Date: 10/11/00 8:11:29 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "../csim/h3sim.h"

#if defined(__sparc__)
#define min(a,b) (((a) < (b)) ? a : b)
#define max(a,b) (((a) > (b)) ? a : b)
#endif /* if defined(__sparc__) */


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

FxU32 nbits(FxU32 n)
{
    FxU32 retval;
    
    do
    {
	retval += n & 1;
	n >>= 1;
    } while (n > 0);

    return retval;
}

typedef enum _bstate
{
    LEFTSIDE,
    INSIDE,
    RIGHTSIDE
} boundarystate_t;


void
main (int argc, char **argv)
{
    FxU32 rop, cmdops,cmdXops;
    FxU32 cfore,cback, cfore2;
    SstRegs *sst;
    SstGRegs *sstg;
    FxU32 dfIndex;
    FxU32 cdest;
    long xc, yc;
    int maxVerts;
    FxI32 p1x, p1y, p2x, p2y, p3x, p3y;
    FxI32 bbXleft, bbXright, bbYtop, bbYbottom;
    boundarystate_t position;

//    fxHalPutenv("SST_FBI_MEM=4");
    int i;
    
    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);

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

    // sstg_init_random_memory(NULL);
    
    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);
    SET(sstg->rop, 0);
    SET(sstg->commandEx, 0);
    SET(sstg->colorBack, cback);		// now set the colors
    SET(sstg->colorFore, cfore);		// now set the colors



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
	  unsigned long j = 0, colinear = 0, noncolinear = 0;
	  
	  cfore = rRandom(1, 0xFFFFFFFF);
	  SET(sstg->colorFore, cfore);
	  do
	  {
	      p1x = rRandom(5, diago.xmaxscreen);
	      p1y = rRandom(5, diago.ymaxscreen);
	      p2x = rRandom(p1x - diago.tsize, p1x + diago.tsize);
	      p2y = rRandom(p1y - diago.tsize, p1y + diago.tsize);
	      p3x = rRandom(p2x - diago.tsize, p2x + diago.tsize);
	      p3y = rRandom(p2y - diago.tsize, p2y + diago.tsize);

	  } while (sstg_colinear(p1x, p1y, p2x, p2y, p3x, p3y) ||
		   (sstg_area(p1x, p1y, p2x, p2y, p3x, p3y) >
					(FxU32)(diago.tsize * diago.tsize)));
	  
	  cmdops = SSTG_ROP_SRC  << SSTG_ROP0_SHIFT;
	  cmdops |= SSTG_REVERSIBLE;
	  sstg_draw_triangle(sstg, p1x, p1y, p2x, p2y, p3x, p3y, cmdops);

	  // pick an outline color "sufficiently" different from the interior
	  do 
	  {
	      cfore2 = rRandom(1, 0xFFFFFFFF);
	  } while (nbits(cfore2 ^ cfore) < 2);
	  
	  SET(sstg->colorFore, cfore2);
	  SET(sstg->srcXY, (p1y << 16) | (p1x & 0xFFFF));
	  SET(sstg->command, (SSTG_ROP_SRC << SSTG_ROP0_SHIFT) |
	      SSTG_POLYLINE | SSTG_REVERSIBLE );
	  SET(sstg->launch[iRandom(31)], (p2y << 16) | (p2x & 0xFFFF));
	  SET(sstg->launch[iRandom(31)], (p3y << 16) | (p3x & 0xFFFF));
	  SET(sstg->launch[iRandom(31)], (p1y << 16) | (p1x & 0xFFFF));
	  
	  sstg_idle(sst);			// wait for command to complete

	  // check the bounding box of the polygon for color transitions
	  // that indicate "holes" between the interior and the edge
     if (diago.checkEveryTriangle)
     {
	 bbXleft = min(min(p1x, p2x), p3x) - 2;
	 bbYtop = min(min(p1y, p2y), p3y) - 2;
	 bbXright = max(max(p1x, p2x), p3x) + 2;
	 bbYbottom = max(max(p1y, p2y), p3y) + 2;
	  
	 for (yc = bbYtop; yc <= bbYbottom; yc++)
	 {
	     position = LEFTSIDE;
	     for (xc = bbXleft; xc <= bbXright; xc++)
	     {
		 if (!ONSCREEN(xc, yc))
		     continue;
	      
		 cdest = csimReadPixel(diago.sstCSIM, CSIM_BUF_2D_SRC, xc, yc);

		 if (cdest != 0)
		 {
		     if (position == LEFTSIDE)
			 position = INSIDE;
		     else if (position == RIGHTSIDE)
			 GDBG_ERROR("poly3", "interior pixel found at (%d,%d) after right edge detected\n", xc, yc);
		 }
		 else if (position == INSIDE)
		     position = RIGHTSIDE;
	     } // for xc
	 } // for yc
	  
	 DIAG_FORCE_RECT(CSIM_BUF_2D_DST,
			 bbXleft,
			 bbYtop,
			 bbXright - bbXleft + 1,
			 bbYbottom - bbYtop + 1,
			 0);	  
     } // if diago.checkEveryTriangle
     

	  if (DIAG_EXCEEDED_PIXEL_LIMIT())
	      break;	    
      } // i
	    
	 if (dstFmtSpecified)
	     break;		// terminate iteration
	  if (DIAG_EXCEEDED_PIXEL_LIMIT())
	      break;	    
     }

    } // diag pass

  gdbg_printf("total pixels drawn: %d\n",
	      CSIMG_PRIVATE(&CSIM_PRIVATE(diago.sstCSIM)->gui)->pixelsOut2d);  



    if (getenv("DIAG_WAIT"))
	getchar();
    
    DIAG_PASS(0);
}
