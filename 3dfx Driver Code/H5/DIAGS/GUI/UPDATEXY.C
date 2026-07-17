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
** $Date: 10/11/00 8:11:44 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "../csim/h3sim.h"

FxU32 XCOVER[8][8];
FxU32 YCOVER[8][8];

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

static FxBool dstFmtSpecified = 1;
static FxU32 dstFmtInitialIndex = 1;
static FxBool widthFactor = 1;	/* max width in units of 128-bits */
static FxU32 height = 9;

static FxBool doSolid = 1;
static FxBool doMonoPat = 1;
static FxBool doColorPat = 1;

static FxBool doForcePatIndex = 0;
static FxU32 forcePatIndex = 0;

static FxU32 pat0;

static FxU32 patterns[] = { 0x0, 0xFF, 0xCC, 0x33, 0xF0, 0x0F, 0x5a, 0xA5 };
static int patIndex;
static int NpatIndeces = sizeof(patterns) / sizeof(FxU32);

static FxU32 patIndexInitial = 0;

static FxU32 destX, destY, patx, paty;


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
	      case 'C':
		  diago.checkEveryTriangle = 0;
		  GDBG_PRINTF("INFO: checkEveryTriangle = 0\n");
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
main (int argc, char **argv)
{
    FxU32 rop, cmdops,cmdXops;
    FxU32 cfore,cback;
    SstRegs *sst;
    SstGRegs *sstg;
    FxU32 dfIndex;
    FxU32 width = 9;
    FxU32 i;
    long dstX, dstY;

//    fxHalPutenv("SST_FBI_MEM=4");

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);

    dstFmtInitialIndex = 0;


    cmdXops = 0;
    rop = 0;
    cfore = 0xdeadbeef;
    cback = 0xbabecafe;

    XParseOpts(argc, argv);

    SET(sstg->clip0min, 0x00000000);
    SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);
    SET(sstg->rop, 0);
    SET(sstg->commandEx, 0);

    SET(sstg->colorFore, 0xdeadbeef);

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

	for (i = 0; i < 256; i++)
	{
	    cmdops = 0;
	    dstX = iRandom(diago.xmaxscreen);
	    dstY = iRandom(diago.ymaxscreen);
	    width = iRandom(diago.tsize);
	    height = iRandom(diago.tsize);
	    SET(sstg->dstXY, (dstY << 16) | dstX);
	    SET(sstg->dstSize, (height << 16) | width);

	    if (iRandom(1))
		cmdops |= SSTG_UPDATE_DSTX;
	    if (iRandom(1))
		cmdops |= SSTG_UPDATE_DSTY;
	
	    // note: drawing with rop 0 to produce black pixels in framebuffer
	    SET(sstg->command, cmdops | SSTG_RECTFILL | SSTG_GO);

	    if (cmdops & SSTG_UPDATE_DSTX)
		dstX += width;
	    if (cmdops & SSTG_UPDATE_DSTY)
		dstY += height;

	    // draw 1x1 rect & verify pixel touched is
	    // (dstX+width, dstY+height)

	    SET(sstg->dstSize, (1 << 16) | 1);
	    cmdops =  SSTG_RECTFILL | SSTG_GO |
		(SSTG_ROP_SRC << SSTG_ROP0_SHIFT);
	    SET(sstg->command, cmdops);

	    sstg_idle(sst);

	    if (diago.checkEveryTriangle)
	    {
		if (ONSCREEN(dstX, dstY))
		{
		    if (sstg_check_pixel(dstX, dstY, cmdops,
					 0, 0, 0xdeadbeef, 0, 0) ==
			sstg_destination_mask(0xdeadbeef))
		    {
			DIAG_FORCE_PIXEL(CSIM_BUF_2D_DST, dstX, dstY, 0);
		    }
		    else
		    {
			gdbg_printf("updatexy error, not forcing pixel\n");
		    }
		}
	    }
	    
	} // i

    if (dstFmtSpecified)
	break;		// terminate iteration
    } // dfIndex
  }

  gdbg_printf("total pixels drawn: %d\n",
	      CSIMG_PRIVATE(&CSIM_PRIVATE(diago.sstCSIM)->gui)->pixelsOut2d);  

    if (getenv("DIAG_WAIT"))
	getchar();
    
    DIAG_PASS(0);

#if 0
    for (i = 0; i < 8; i++)
	for (j = 0; j < 8; j++)
	    printf("XCOVER[%d][%d] = %d\n", i, j, XCOVER[i][j]);
    for (i = 0; i < 8; i++)
	for (j = 0; j < 8; j++)
	    printf("YCOVER[%d][%d] = %d\n", i, j, YCOVER[i][j]);
#endif
	
}
