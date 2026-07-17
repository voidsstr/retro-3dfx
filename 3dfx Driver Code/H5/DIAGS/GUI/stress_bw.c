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
** $Date: 10/11/00 8:11:40 PM$
*/

#include <stdio.h>

#include "udiag.h"
#include "sstdiag.h"

#if defined(__sparc__)
#define min(a,b) (((a) < (b)) ? a : b)
#define max(a,b) (((a) > (b)) ? a : b)
#endif /* if defined(__sparc__) */

//
// These are the "do this once in a while" frequency variables that are
// not entire registers, but usually bit fields within them; or, are
// "concept" things to do that may affect groups of registers.
// They're used as arguments to the DO() macro (see comment accompanying
// "#define DO()" below).
//

static int tiledDest;	// do tiled destination surface?
static int tiledSrc;	// do tiled source surface?
static int rop0;	// do randomly selected rop0?
static int repeatedRectFill;	// do another rectFill, just change dstXY?
static int rectFillLaunch;	// use launch mode instead of GO bit?
static int primitive;	// change the command primitive?
static int fullRangeXY;	// generate dstXY in the full legal range
static int monotrans;	// monochrome transparency
static int reversible;	// reversible lines
static int updateX;	// update X after command
static int updateY;	// update Y after command
static int lineStipple;	// do line stippling
static int monopattern;
static int xdir;	// negative x dir
static int _ydir;	// negative y dir
static int xpatoff;	// random X pattern offset
static int ypatoff;	// random Y pattern offset
static int clipselect;	// use 2nd clip register
static int srcColorkey;
static int dstColorkey;;
static int vsyncWait;
static int patRow0;
static int zeroClipMin;
static int maxClipMax;
static int fixedPrimType;
static int repeatedLineDraw;
static int lineLaunch;
static int src_EQUALS_dst;	// make the src surface match the dst ?
static int launchBlt;
static int hostBltUpdateXY;
static int tooFewHblitWords;
static int tooManyHblitWords;

SstGRegs change;
SstGRegs new;

static long maxX = 0x0FFF;
static long maxY = 0x0FFF;
static long minX = -0xFFE;
static long minY = -0xFFE;
static fixedPrim;
static src_is_dst;
static int repeatable = 0;
static int initMemory = 1;
static FxU32 dumpStatePass;
static FxBool doDumpState = 0;
static FxBool doLoadState = 0;
static int iterationsPerPass = 20;
static FxBool diffMemory = 0;

//
// garbage saved pixel receptable used by the sstgdiag host blit
// helper routines -- not used here in stress
//
static FxU32 host_pixels[MAXSCREEN*MAXSCREEN];


char *
Xusage()
{
    gdbg_printf("\n");
    gdbg_printf("\"-xd #\"\tforce single dst format 0=>32, 1=>8, 2=>16, 3=>24 (default all)\n");
    gdbg_printf("\"-xtd\"\tturn off tiled destination\n");
    gdbg_printf("\"-xts\"\tturn off tiled source\n");
    gdbg_printf("\"-xmaxx=#\"\tclamp max destination x to this value\n");
    gdbg_printf("\"-xmaxy=#\"\tclamp max destination y to this value\n");
    gdbg_printf("\"-xminx=#\"\tclamp min destination x to this value\n");
    gdbg_printf("\"-xminy=#\"\tclamp min destination y to this value\n");
    gdbg_printf("\"-xprim=#\"\tdraw only primitive #\n");
    gdbg_printf("\"-xzcm=#\"\tset zero clipmin freq. = #\n");
    gdbg_printf("\"-xmcm=#\"\tset max clipmax = #\n");
    gdbg_printf("\"-xdba=#\"\tset dest. base addr freq = #\n");
    gdbg_printf("\"-xonscreen\"\tshorthand for minx=0 miny=0 maxx=640 maxy=480 dba=0 zcm=1 mcm=1\n");
    gdbg_printf("\"-xrepeatable\"\tforce down known state at the start of every pass\n");
    gdbg_printf("\"-xrandmem\"\tdon't fill memory with random data before starting\n");
    gdbg_printf("\"-xdp=#\"\tdump register state to file state.txt when pass = #\n");
    gdbg_printf("\"-xloadstate\"\tload state from file state.txt\n");
    gdbg_printf("\"-xhbltuxy=%d\"\tdo repeated updatexy passes of hblt freq=#\n");
    gdbg_printf("\"-xitpp=%d\"\tdo # iterations (roughly, # of prmitives) per pass\n");
    gdbg_printf("\"-xsed=#\"\tfreq. of src surface == dst surface (def. 2)\n");
    gdbg_printf("\"-xvsync=#\"\tfreq. of sync to vertical (def. 0)\n");

    exit(1);
    return(0);
}

static FxU32 pix_to_dstfmt[] = 
{
    SSTG_PIXFMT_32BPP, 
    SSTG_PIXFMT_8BPP,
    SSTG_PIXFMT_16BPP,
    SSTG_PIXFMT_24BPP,
};


/* myParseOpts
 *
 * look for "-x" options and interpret them for this test
 *
 */

#define XGETARG() opts[1] ? done = 1, ++opts : \
			  (--argc > 0) ? done = 1, *++argv : \
					 (char *)Xusage()

FxU32 dstFmt;

void
XParseOpts(int argc, char **argv, SstGRegs *mysstg)
{
    char *opts = 0, *arg;
    FxBool aopt = 0;
    FxU32 bopt = 0;
    FxBool done;
    FxU32 dstFmtIndex;
    
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
	      case 'C':
		  diffMemory = 1;
		  GDBG_PRINTF("INFO: diffing memory after every primitive\n");
		  break;

	      case 'd':
		  arg = XGETARG();
		  if (strncmp(arg, "ba=", 3) == 0)
		  {
		      sscanf(arg, "ba=%i", &change.dstBaseAddr);
		      GDBG_PRINTF("INFO: change.dstBaseAddr=%d\n",
				  change.dstBaseAddr);
		  }
		  else if (strncmp(arg, "p=", 2) == 0)
		  {
		      sscanf(arg, "p=%u", &dumpStatePass);
		      doDumpState = 1;
		      GDBG_PRINTF("INFO: dumping state on pass #%d\n",
				  dumpStatePass);
		  }
		  else
		  {
		      sscanf(arg, "%i", &dstFmtIndex);
		      if ((dstFmtIndex < 0) || (dstFmtIndex > 3))
		      {
			  GDBG_ERROR("stress2d", "bad dst format index\n");
		      }
		      dstFmt = pix_to_dstfmt[dstFmtIndex];
		      GDBG_PRINTF("INFO: dst pixel format = %d\n", dstFmt);
		      change.dstFormat = 0;
		      mysstg->dstFormat = dstFmt;
		      mysstg->dstFormat |= (2560 << SSTG_DST_STRIDE_SHIFT);
		  }
		  break;
	      case 'h':
		  arg = XGETARG();
		  if (strncmp(arg, "bltuxy=", 6) == 0)
		  {
		      sscanf(arg, "bltuxy=%d", &hostBltUpdateXY);
		      GDBG_PRINTF("INFO: hostBltUpdateXY=%d\n",
				  hostBltUpdateXY);
		  }
		  else
		      Xusage();
		  break;
	      case 'i':
		  arg = XGETARG();
		  if (strncmp(arg, "tpp=", 4) == 0)
		  {
		      sscanf(arg, "tpp=%d", &iterationsPerPass);
		      GDBG_PRINTF("INFO: iterationsPerPass=%d\n",
				  iterationsPerPass);
		  }
		  else
		      Xusage();
		  break;
	      case 'l':
		  arg = XGETARG();
		  if (strncmp(arg, "oadstate", 8) == 0)
		  {
		      doLoadState = 1;
		      GDBG_PRINTF("INFO: loading state from state.txt\n");
		  }
		  break;
	      case 't':
		  arg = XGETARG();
		  if (strcmp(arg, "d") == 0)
		  {
		      tiledDest = 0;
		      GDBG_PRINTF("INFO: tiled destinations disabled\n");
		  }
		  else if (strcmp(arg, "s") == 0)
		  {
		      tiledSrc = 0;
		      GDBG_PRINTF("INFO: tiled sources disabled\n");
		  }
		  else if (strncmp(arg, "fw=", 3) == 0)
		  {
		      sscanf(arg, "fw=%d", &tooFewHblitWords);
		      GDBG_PRINTF("INFO: tooFewHblt freq. = %d\n",
				  tooFewHblitWords);
		  }
		  else if (strncmp(arg, "mw=", 3) == 0)
		  {
		      sscanf(arg, "mw=%d", &tooManyHblitWords);
		      GDBG_PRINTF("INFO: tooManyHblitWords freq. = %d\n",
				  tooManyHblitWords);
		  }
		  break;
	      case 'm':
		  arg = XGETARG();
		  if (strncmp(arg, "axx=", 4) == 0)
		  {
		      sscanf(arg, "axx=%i", &maxX);
		      GDBG_PRINTF("INFO: maxX = %d\n", maxX);
		  }
		  else if (strncmp(arg, "axy=", 4) == 0)
		  {
		      sscanf(arg, "axy=%i", &maxY);
		      GDBG_PRINTF("INFO: maxY = %d\n", maxY);
		  }
		  else if (strncmp(arg, "inx=", 4) == 0)
		  {
		      sscanf(arg, "inx=%i", &minX);
		      GDBG_PRINTF("INFO: minX = %d\n", minX);
		  }
		  else if (strncmp(arg, "iny=", 4) == 0)
		  {
		      sscanf(arg, "iny=%i", &minY);
		      GDBG_PRINTF("INFO: minY = %d\n", minY);
		  }
		  else if (strncmp(arg, "cm=", 3) == 0)
		  {
		      sscanf(arg, "cm=%i", &maxClipMax);
		      GDBG_PRINTF("INFO: maxClipMax = %d\n", maxClipMax);
		  }
		  else
		      Xusage();
		  break;
	      case 'o':
		  arg = XGETARG();		  
		  if (strncmp(arg, "nscreen", 7) == 0)
		  {
		      minX = minY = 0;
		      maxX = 640;
		      maxY = 480;
		      maxClipMax = 1;
		      zeroClipMin = 1;
		      change.dstBaseAddr = 0;
		      GDBG_PRINTF("INFO: minX = %d\n", minX);
		      GDBG_PRINTF("INFO: minY = %d\n", minY);
		      GDBG_PRINTF("INFO: maxX = %d\n", maxX);
		      GDBG_PRINTF("INFO: maxY = %d\n", maxY);
		      GDBG_PRINTF("INFO: zeroClipMin = %d\n", zeroClipMin);
		      GDBG_PRINTF("INFO: maxClipMax = %d\n", maxClipMax);
		      GDBG_PRINTF("INFO: change.dstBaseAddr=%d\n",
				  change.dstBaseAddr);		      
		  }
		  else
		      Xusage;
		  break;
	      case 'p':
		  arg = XGETARG();
		  if (strncmp(arg, "rim=", 4) == 0)
		  {
		      fixedPrimType = 1;
		      sscanf(arg, "rim=%i", &fixedPrim);
		      fixedPrim <<= SSTG_COMMAND_SHIFT;
		      fixedPrim &= SSTG_COMMAND;
		      GDBG_PRINTF("INFO: fixed primitive = %d (shifted)\n",
				  fixedPrim);
		  }
		  else
		      Xusage;
		  break;
	      case 'r':
		  arg = XGETARG();
		  if (strncmp(arg, "epeatable", 9) == 0)
		  {
		      repeatable = 1;
		      GDBG_PRINTF("INFO: repeatable passes\n");
		  }
		  else if (strncmp(arg, "andmem", 6) == 0)
		  {
		      initMemory = 0;
		      GDBG_PRINTF("INFO: not setting memory to random values\n");
		  }

	      case 's':
		  arg = XGETARG();
		  if (strncmp(arg, "ed=", 3) == 0)
		  {
		      sscanf(arg, "ed=%i", &src_EQUALS_dst);
		      GDBG_PRINTF("INFO: src_EQUALS_dst set to %d\n",
				  src_EQUALS_dst);
		  }
		  break;

	      case 'v':
		  arg = XGETARG();
		  if (strncmp(arg, "sync=", 5) == 0)
		  {
		      sscanf(arg, "sync=%i", &vsyncWait);
		      GDBG_PRINTF("INFO: vsyncWait = %d\n", vsyncWait);
		  }
		  break;

	      case 'z':
		  arg = XGETARG();
		  if (strncmp(arg, "cm=", 3) == 0)
		  {
		      sscanf(arg, "cm=%i", &zeroClipMin);
		      GDBG_PRINTF("INFO: zeroClipMin = %d\n", zeroClipMin);
		  }
		  break;

	      default:
		  Xusage();
	    }
	    
	    opts += 1;
	}
    }
}


static void 
clear_new_bits()
{
    int cp;
    
    new.clip0min = 0;
    new.clip0max = 0;
    new.dstBaseAddr = 0;
    new.dstFormat = 0;
    new.srcColorkeyMin = 0;
    new.srcColorkeyMax = 0;
    new.dstColorkeyMin = 0;
    new.dstColorkeyMax = 0;
    new.bresError0 = 0;
    new.bresError1 = 0;
    new.rop = 0;
    new.srcBaseAddr = 0;
    new.commandEx = 0;
    new.lineStipple = 0;
    new.lineStyle = 0;
    new.pattern0alias = 0;
    new.pattern1alias = 0;
    new.clip1min = 0;
    new.clip1max = 0;
    new.srcFormat = 0;
    new.srcSize = 0;
    new.srcXY = 0;
    new.colorBack = 0;
    new.colorFore = 0;
    new.dstSize = 0;
    new.dstXY = 0;
    new.command = 0;

    for (cp = 0; cp < 64; cp++)
	new.colorPattern[cp] = 0;
}


static void
set_initial_state(SstGRegs *mysstg)
{
    int cp;
    FxU32 savedseed;
    
    mysstg->clip0min = 0;
    mysstg->clip0max = (diago.ymaxscreen << 16) | diago.xmaxscreen;
    mysstg->dstBaseAddr = diago.minTrashMem;
    mysstg->dstFormat = SSTG_PIXFMT_32BPP | (diago.xmaxscreen * 4);
    mysstg->srcColorkeyMin = 0;
    mysstg->srcColorkeyMax = 0;
    mysstg->dstColorkeyMin = 0;
    mysstg->dstColorkeyMax = 0;
    mysstg->bresError0 = 0;
    mysstg->bresError1 = 0;
    mysstg->rop = 0;
    mysstg->srcBaseAddr = mysstg->dstBaseAddr;
    mysstg->commandEx = 0;
    mysstg->lineStipple = 0;
    mysstg->lineStyle = 0;
    mysstg->pattern0alias = 0;
    mysstg->pattern1alias = 0;
    mysstg->clip1min = 0;
    mysstg->clip1max = mysstg->clip0max;
    mysstg->srcFormat = mysstg->dstFormat;
    mysstg->srcSize = 0;
    mysstg->srcXY = 0;
    mysstg->colorBack = 0;
    mysstg->colorFore = 0xdeadbeef;
    mysstg->dstSize = (diago.tsize << 16) | diago.tsize;
    mysstg->dstXY = 0;
    mysstg->command = (SSTG_RECTFILL << SSTG_COMMAND_SHIFT) |
			(SSTG_ROP_SRC << SSTG_ROP0_SHIFT);

    savedseed = getSeed();
    setSeed(1);
    for (cp = 0; cp < 64; cp++)
	mysstg->colorPattern[cp] = iRandom(0xFFFFFFFF);
    setSeed(savedseed);

    clear_new_bits();

}

static void
set_initial_change_frequencies(SstGRegs *change)
{
    int cp;
    
    change->clip0min = 6;
    change->clip0max = 1;
    change->dstBaseAddr = 1;
    change->dstFormat = 3;
    change->srcColorkeyMin = 3;
    change->srcColorkeyMax = 3;
    change->dstColorkeyMin = 3;
    change->dstColorkeyMax = 3;
    change->bresError0 = 1;
    change->bresError1 = 1;
    change->rop = 1;
    change->srcBaseAddr = 1;
    change->commandEx = 1;
    change->lineStipple = 1;
    change->lineStyle = 1;
    change->pattern0alias = 3;
    change->pattern1alias = 3;
    change->clip1min = 1;
    change->clip1max = 1;
    change->srcFormat = 3;
    change->srcSize = 1;
    change->srcXY = 1;
    change->colorBack = 1;
    change->colorFore = 1;
    change->dstSize = 1;
    change->dstXY = 1;
    change->command = 1;

    for (cp = 0; cp < 64; cp++)
 	change->colorPattern[cp] = 10;

    //
    // non-register based frequency variables
    // ("things to change once in a while")
    //
    tiledDest = 2;
    tiledSrc = 2;
    rop0 = -1;
    repeatedRectFill = 3;
    rectFillLaunch = 2;
    primitive = 1;
    fullRangeXY = 4;
    monotrans = 3;
    reversible = 3;
    updateX = 3;
    updateY = 3;
    lineStipple = 2;
    monopattern = 3;
    xdir = 3;
    _ydir = 3;
    xpatoff = 2;
    ypatoff = 2;
    clipselect = 3;
    srcColorkey = 4;
    dstColorkey = 4;
    vsyncWait = 0;
    patRow0 = 3;
    zeroClipMin = 3;
    maxClipMax = 3;
    fixedPrimType = 0;
    repeatedLineDraw = 2;
    lineLaunch = 2;
    src_EQUALS_dst = 2;
    launchBlt = 2;
    hostBltUpdateXY = 3;
    tooFewHblitWords = 0;
    tooManyHblitWords = 0;
}



static void
dumpStateToFile(SstGRegs *mysstg)
{
    int cp;
    FILE *fp = fopen("state.txt", "w");
    
    fprintf(fp, "%u\t//seed\n", getSeed());
    fprintf(fp, "0x%08x\t%s\n", mysstg->clip0min, "//clip0min");
    fprintf(fp, "0x%08x\t%s\n", mysstg->clip0max, "//clip0max");
    fprintf(fp, "0x%08x\t%s\n", mysstg->dstBaseAddr, "//dstBaseAddr");
    fprintf(fp, "0x%08x\t%s\n", mysstg->dstFormat, "//dstFormat");
    fprintf(fp, "0x%08x\t%s\n", mysstg->srcColorkeyMin, "//srcColorkeyMin");
    fprintf(fp, "0x%08x\t%s\n", mysstg->srcColorkeyMax, "//srcColorkeyMax");
    fprintf(fp, "0x%08x\t%s\n", mysstg->dstColorkeyMin, "//dstColorkeyMin");
    fprintf(fp, "0x%08x\t%s\n", mysstg->dstColorkeyMax, "//dstColorkeyMax");
    fprintf(fp, "0x%08x\t%s\n", mysstg->bresError0, "//bresError0");
    fprintf(fp, "0x%08x\t%s\n", mysstg->bresError1, "//bresError1");
    fprintf(fp, "0x%08x\t%s\n", mysstg->rop, "//rop");
    fprintf(fp, "0x%08x\t%s\n", mysstg->srcBaseAddr, "//srcBaseAddr");
    fprintf(fp, "0x%08x\t%s\n", mysstg->commandEx, "//commandEx");
    fprintf(fp, "0x%08x\t%s\n", mysstg->lineStipple, "//lineStipple");
    fprintf(fp, "0x%08x\t%s\n", mysstg->lineStyle, "//lineStyle");
    fprintf(fp, "0x%08x\t%s\n", mysstg->pattern0alias, "//pattern0alias");
    fprintf(fp, "0x%08x\t%s\n", mysstg->pattern1alias, "//pattern1alias");
    fprintf(fp, "0x%08x\t%s\n", mysstg->clip1min, "//clip1min");
    fprintf(fp, "0x%08x\t%s\n", mysstg->clip1max, "//clip1max");
    fprintf(fp, "0x%08x\t%s\n", mysstg->srcFormat, "//srcFormat");
    fprintf(fp, "0x%08x\t%s\n", mysstg->srcSize, "//srcSize");
    fprintf(fp, "0x%08x\t%s\n", mysstg->srcXY, "//srcXY");
    fprintf(fp, "0x%08x\t%s\n", mysstg->colorBack, "//colorBack");
    fprintf(fp, "0x%08x\t%s\n", mysstg->colorFore, "//colorFore");
    fprintf(fp, "0x%08x\t%s\n", mysstg->dstSize, "//dstSize");
    fprintf(fp, "0x%08x\t%s\n", mysstg->dstXY, "//dstXY");
    fprintf(fp, "0x%08x\t%s\n", mysstg->command, "//command");

    for (cp = 0; cp < 64; cp++)
 	fprintf(fp, "0x%08x\t%s%d\n", mysstg->colorPattern[cp],
		"//colorpattern#", cp);
    fflush(fp);
    fclose(fp);
}


static void
loadStateFromFile(SstGRegs *mysstg)
{
    int cp;
    FILE *fp = fopen("state.txt", "r");
    FxU32 seed;
    char buf[200];
    
    if (fp == (FILE *)NULL)
    {
	GDBG_ERROR("loadStateFromFile", "couldn't open file state.txt\n");
	DIAG_FAIL();
    }
    
    fscanf(fp, "%u\t//seed\n", &seed);
    setSeed(seed);
    printf("loadStateFromFile: set seed to %u\n", seed);
    
    fscanf(fp, "0x%08x\t%s\n", &mysstg->clip0min, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->clip0max, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->dstBaseAddr, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->dstFormat, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->srcColorkeyMin, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->srcColorkeyMax, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->dstColorkeyMin, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->dstColorkeyMax, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->bresError0, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->bresError1, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->rop, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->srcBaseAddr, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->commandEx, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->lineStipple, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->lineStyle, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->pattern0alias, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->pattern1alias, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->clip1min, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->clip1max, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->srcFormat, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->srcSize, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->srcXY, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->colorBack, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->colorFore, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->dstSize, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->dstXY, buf);
    fscanf(fp, "0x%08x\t%s\n", &mysstg->command, buf);

    for (cp = 0; cp < 64; cp++)
 	fscanf(fp, "0x%08x\t%s\n", &mysstg->colorPattern[cp], buf);

    fflush(fp);
    fclose(fp);
}

static void
dump_state(SstGRegs *sstg, SstGRegs *mysstg)
{
    int cp;
    
    SET(sstg->clip0min, mysstg->clip0min);
    SET(sstg->clip0max, mysstg->clip0max);
    SET(sstg->dstBaseAddr, mysstg->dstBaseAddr);
    SET(sstg->dstFormat, mysstg->dstFormat);
    SET(sstg->srcColorkeyMin, mysstg->srcColorkeyMin);
    SET(sstg->srcColorkeyMax, mysstg->srcColorkeyMax);
    SET(sstg->dstColorkeyMin, mysstg->dstColorkeyMin);
    SET(sstg->dstColorkeyMax, mysstg->dstColorkeyMax);
    SET(sstg->bresError0, mysstg->bresError0);
    SET(sstg->bresError1, mysstg->bresError1);
    SET(sstg->rop, mysstg->rop);
    SET(sstg->srcBaseAddr, mysstg->srcBaseAddr);
    SET(sstg->commandEx, mysstg->commandEx);
    SET(sstg->lineStipple, mysstg->lineStipple);
    SET(sstg->lineStyle, mysstg->lineStyle);
    SET(sstg->pattern0alias, mysstg->pattern0alias);
    SET(sstg->pattern1alias, mysstg->pattern1alias);
    SET(sstg->clip1min, mysstg->clip1min);
    SET(sstg->clip1max, mysstg->clip1max);
    SET(sstg->srcFormat, mysstg->srcFormat);
    SET(sstg->srcSize, mysstg->srcSize);
    SET(sstg->srcXY, mysstg->srcXY);
    SET(sstg->colorBack, mysstg->colorBack);
    SET(sstg->colorFore, mysstg->colorFore);
    SET(sstg->dstSize, mysstg->dstSize);
    SET(sstg->dstXY, mysstg->dstXY);
    SET(sstg->command, mysstg->command);

    for (cp = 0; cp < 64; cp++)
	SET(sstg->colorPattern[cp], mysstg->colorPattern[cp]);
}


//
// given a register, determine whether to change it or not
//
// -1 means always "do" the action (e.g., set new value in a register
//  0 means never do the action
//  > 0 means do the action 1 out of N times
//
#define DO_REG(r) (((change.r == -1) || (iRandom((change.r) - 1) == 0)) \
	    && (change.r != 0))
#define DO(x) ((((x) == -1) || (iRandom((x)-1) == 0)) && ((x) != 0))

#define SETREG(reg) \
	if (new.reg) \
	{ \
	    SET(sstg->reg, mysstg.reg); \
	    new.reg = 0; \
	}

#define MAX_TILE_STRIDE 64

FxU32
sstg_random_dstFormat(SstGRegs *mysstg)
{
    FxU32 df, stride, align;

    df = rRandom((SSTG_PIXFMT_8BPP >> SSTG_DST_FORMAT_SHIFT),
		 (SSTG_PIXFMT_32BPP >> SSTG_DST_FORMAT_SHIFT));

    df <<= SSTG_DST_FORMAT_SHIFT;

    if (df == SSTG_PIXFMT_15BPP)
	df = SSTG_PIXFMT_16BPP;

    switch (df)
    {
      case SSTG_PIXFMT_8BPP:
      case SSTG_PIXFMT_24BPP: align = 1; break;
      case SSTG_PIXFMT_16BPP: align = 2; break;
      case SSTG_PIXFMT_32BPP: align = 4; break;
    }
    
    if ((mysstg->dstBaseAddr) & SSTG_IS_TILED)
    {
#ifdef BIGTILESTRIDES
	// tiled destination
	if (iRandom(10) == 0)
	    stride = iRandom(SSTG_DST_TILE_STRIDE);	// some BIG tile strides
	else
#endif /* #ifdef BIGTILESTRIDES */
	    stride = rRandom(1, MAX_TILE_STRIDE);	// more reasonable
    }
    else
    {
	// linear destination
	stride = iRandom(SSTG_DST_LINEAR_STRIDE);
	stride &= ~(align - 1);
    }
    
    if (change.dstBaseAddr != 0)
	df |= stride;
    else
	df |= 640*4;

    return df;
}

//
// return a destination size that fits in between the destination base
// address and the end of usable memory
//
FxU32
sstg_random_dstSurface(SstGRegs *mysstg)
{
    long x, y, width, height;
    FxU32 dstSize;
    int saved_tsize;

    //
    // a tad hacky.  Calculate a random destination surface using the
    // ranodm source destination function.  It's actually a general function
    // since it takes the base and format as parameters, but only works
    // because the source and destination format registers are basically
    // symmetrical (every valid destination base/format is also a valid
    // source destination/source, but not vice-versa)
    //
    // another filthy hack: sstg_random_src_rect uses diago.tsize as the
    // maximum width / height.  We change this value (saving and restoring
    // the "real" diago.tsize) to get a larger range of destination
    // surface sizes
    
    saved_tsize = diago.tsize;
    diago.tsize = rRandom(1, 0x0FFF);
    sstg_random_src_rect(CSIM_BUF_2D_SRC, mysstg->dstFormat,
			 mysstg->dstBaseAddr, &x, &y, &width, &height);
    diago.tsize = saved_tsize;
    
    //
    // dimension of the returned surface is from (0, 0) to (x+width, y+height)
    //
    dstSize = ((y + height) << 16) | (x + width);
    return dstSize;
}



#define GIMME(reg, regmask) ((reg & regmask) >> regmask##_SHIFT)


void
sstg_random_src_rect2(int buffer, FxU32 dstBase,
		      FxU32 srcFormat, FxU32 srcBase,
		      long *wOut, long *hOut)
{
    long w, h;
    FxU32 maxArea;
    FxU32 dstBaseAddr = GIMME(dstBase, SSTG_BASEADDR);
    FxU32 srcBaseAddr = GIMME(srcBase, SSTG_BASEADDR);

    if (dstBaseAddr < srcBaseAddr)
	maxArea = diago.maxTrashMem - srcBaseAddr;
    else if (srcBaseAddr < dstBaseAddr)
	maxArea = dstBaseAddr - srcBaseAddr;
    else
    {
	GDBG_PRINTF("warning: sstg_random_src_rect2", "srcBA == dstBA\n");
    }

    w = iRandom(diago.tsize - 1) + 1;
    h = iRandom(diago.tsize - 1) + 1;    

    // find a surface size that fits between  srcBaseAddr and
    // (srcBaseAddr + maxArea)
    //
    while (sstg_src_sizeInBytes(buffer, srcBase, srcFormat, w, h) > maxArea)
    {
	if ((w == 1) && (h == 1))
	{
	    *wOut = 0;
	    *hOut = 0;
	    return;
	}

	if (iRandom(3) == 0)
	    w -= 1;
	else
	    h -= 1;

	if (w == 0)
	{
	    w = 1;
	    h -= 1;
	}
	if (h == 0)
	{
	    h = 1;
	    w -= 1;
	}

	if (w == 0)
	    w = 1;
    }
    *wOut = w;
    *hOut = h;
}


//
// return a source surface size that fits in memory and does not overlap the
// destination surface.  assumes that dstBaseAddr != srcBaseAddr unless
// src_is_dst is set, when they're assumed to be the same
//
FxU32
sstg_random_srcSurface(SstGRegs *mysstg)
{
    long width, height, x, y;
    FxU32 srcSize;
    int saved_tsize;

    //
    // a tad hacky.  Calculate a random destination surface using the
    // ranodm source destination function.  It's actually a general function
    // since it takes the base and format as parameters, but only works
    // because the source and destination format registers are basically
    // symmetrical (every valid destination base/format is also a valid
    // source destination/source, but not vice-versa)
    //
    // another filthy hack: sstg_random_src_rect uses diago.tsize as the
    // maximum width / height.  We change this value (saving and restoring
    // the "real" diago.tsize) to get a larger range of destination
    // surface sizes
    
    saved_tsize = diago.tsize;
    diago.tsize = rRandom(1, 0x0FFF);
    if (src_is_dst)
    {
	sstg_random_src_rect(CSIM_BUF_2D_SRC, mysstg->dstFormat,
			     mysstg->dstBaseAddr, &x, &y, &width, &height);
	srcSize = ((y + height) << 16) | (x + width);
    }
    else
    {
	sstg_random_src_rect2(CSIM_BUF_2D_SRC, mysstg->dstBaseAddr,
			      mysstg->srcFormat,
			      mysstg->srcBaseAddr, &width, &height);
	srcSize = (height << 16) | width;
    }
    diago.tsize = saved_tsize;
    
    return srcSize;
}



void
align_baseAddr_format(FxU32 *baseAddr, FxU32 *format, FxU32 *newBA,
		      FxU32 *newFmt)
{
    FxU32 newDstBA;
    FxU32 align, oldStride, newStride;
    int setnewstride = 0;
    
    // ack, hopefully works because src and dst formats are nearly identical
    switch (*format & SSTG_SRC_FORMAT)
    {
      case SSTG_PIXFMT_1BPP:
      case SSTG_PIXFMT_8BPP:
      case SSTG_PIXFMT_24BPP: align = 1; break;
      case SSTG_PIXFMT_16BPP: align = 2; break;
      case SSTG_PIXFMT_422YUV:
      case SSTG_PIXFMT_422UYV:
      case SSTG_PIXFMT_32BPP: align = 4; break;
      default:
	  GDBG_ERROR("align_baseAddr", "bad format\n");
    }

    newDstBA = *baseAddr & ~(align - 1);

    if (newDstBA != *baseAddr)
    {
	*baseAddr = newDstBA;
	*newBA = 1;
    }

    // depends on SSTG_DST_STRIDE == SSTG_SRC_STRIDE
    oldStride = ((*format)&SSTG_DST_LINEAR_STRIDE)>>SSTG_DST_STRIDE_SHIFT;
    if (!(newDstBA & SSTG_IS_TILED))
	newStride = oldStride & ~(align - 1);
    else if ((oldStride > MAX_TILE_STRIDE) || (oldStride == 0))
	newStride = rRandom(1, MAX_TILE_STRIDE);
    else
    {
	newStride = oldStride;
	setnewstride = 1;
    }

    if ((oldStride != newStride) || setnewstride)
    {
	*format &= ~SSTG_DST_LINEAR_STRIDE;
	*format |= newStride << SSTG_DST_STRIDE_SHIFT;
	*newFmt = 1;
    }
}

#define IS_NEGATIVE(x) ((x) & (1 << 12))

//
// generate an (x,y) point in the range (minX or 0, min(maxX, width)),
//					 minY or 0, min(maxY, height))	
// 
void
generate_signed_XY(long *xOut, long *yOut, FxU32 width, FxU32 height)
{
    long x, y;
    
    if (DO(fullRangeXY))
    {
	do
	    x = rRandom(minX, maxX);
	while ((x > 0) && ((FxU32)x >= width));

	do
	    y =  rRandom(minY, maxY);
	while ((y > 0) && ((FxU32)y >= height));
    }
    else
    {
	if (width > 0)
	    x = iRandom(min(maxX, (long)width - 1));
	else
	    x = 0;
	if (height > 0)
	    y = iRandom(min(maxY, (long)height - 1));
	else
	    y = 0;
    }
    *xOut = x;
    *yOut = y;
}


//
// generate an (x,y) point in the range (minX or 0, min(maxX, width)),
//					 minY or 0, min(maxY, height))	
// 

void
generate_unsigned_XY(long *xOut, long *yOut, FxU32 width, FxU32 height)
{
    long x, y;
    
    if (width > 0)
	x = iRandom(min(maxX, (long)width - 1));
    else
	x = 0;
    if (height > 0)
	y = iRandom(min(maxY, (long)height - 1));
    else
	y = 0;

    *xOut = x;
    *yOut = y;
}


//
// generate a line of random length, up to diago.tsize^2, but that doesn't
// go outside of the width, height of the destination surface
//
void
generate_line_endpoint(SstGRegs *mysstg, FxU32 width, FxU32 height)
{
    long dstX, dstY;
    long srcX, srcY;


    if (IS_NEGATIVE(mysstg->srcXY & 0xFFFF))
	srcX = ((long)mysstg->srcXY << (31 - 12)) >> (31 - 12);
    else
	srcX = mysstg->srcXY & 0x0FFF;
    
    if (IS_NEGATIVE((mysstg->srcXY >> 16) & 0xFFFF))
	srcY = ((long)mysstg->srcXY << (31 - (12+16))) >> (31 - 12);
    else
	srcY = (mysstg->srcXY >> 16) & 0x1FFF;	

#if 0
    do
    {
	dstX = srcX + rRandom(-diago.tsize * diago.tsize / 2,
			      diago.tsize * diago.tsize / 2);
    } while (((dstX > 0) && (dstX > (1 << 12))) ||
	     ((dstX < 0) && (dstX <= -(1 << 12))));
    do
    {
	dstY = srcY + rRandom(-diago.tsize * diago.tsize / 2,
			      diago.tsize * diago.tsize / 2);
    } while (((dstY > 0) && (dstY > (1 << 12))) ||
	     ((dstY < 0) && (dstY <= -(1 << 12))));
    
    if ((dstX > 0) && ((FxU32)dstX >= width))
	dstX = width - 1;
    if ((dstY > 0) && ((FxU32)dstY >= height))
	dstY = height - 1;
#else
    do
    {
	generate_signed_XY(&dstX, &dstY, width, height);
    } while ((dstX < minX) || (dstX > maxX) || (dstY < minY) ||
	     (dstY > maxY) || (abs(dstX-srcX) > (diago.tsize * diago.tsize/2))
	     || (abs(dstY-srcY) > (diago.tsize * diago.tsize/2)));
#endif
    if (width == 0)
	dstX = 0;
    if (height == 0)
	dstY = 0;

    mysstg->dstXY = (dstY << 16) | (dstX & 0xFFFF);
    gdbg_info(5, "srcXY = 0x%08x, srcX = %d, srcY = %d\n", mysstg->srcXY, srcX,
	   srcY);
    gdbg_info(5, "dstXY = 0x%08x, dstX = %d, dstY = %d\n", mysstg->dstXY, dstX,
	   dstY);
}


//
// given a surface of size (width, height) generate a subrectangle
// in this surface and set the dstXY and dstSize registers
//
void
generate_dstXY_dstSize(SstGRegs *mysstg, FxU32 width, FxU32 height)
{
    long x, y;
    FxU32 w, h;

    generate_signed_XY(&x, &y, width, height);

    if (x < 0)
	w = iRandom(min((FxU32)diago.tsize, width - 1));
    else
	w = iRandom(min((FxU32)diago.tsize, max(0, width - x)));
    if (((x+w) >= width) && (w > 0))
	w -= 1;
	
    if (y < 0)
	h = iRandom(min((FxU32)diago.tsize, height - 1));
    else    
	h = iRandom(min((FxU32)diago.tsize, max(0, height - y)));
    if (((y+h) >= height) && (h > 0))
	h -= 1;

    if (w > (FxU32)diago.tsize)
	w = diago.tsize;
    if (h > (FxU32)diago.tsize)
	h = diago.tsize;

    mysstg->dstXY = ((y & 0xFFFF) << 16) | (x & 0xFFFF);
    mysstg->dstSize = (h << 16) | w;
    gdbg_info(6, "dstXY = 0x%08x, dstSize = 0x%08x\n", mysstg->dstXY,
	      mysstg->dstSize);
    
}


//
// given a source surface of size (width, height) generate a subrectangle
// in this surface and set the dstXY and dstSize registers
//
void
generate_srcXY_srcSize(SstGRegs *mysstg, FxU32 width, FxU32 height)
{
    long x, y;
    FxU32 w, h, pack;

    generate_unsigned_XY(&x, &y, width, height);

    pack = GIMME(mysstg->srcFormat, SSTG_SRC_PACK);
    if (((x != 0) || (y != 0)) && (pack != SSTG_SRC_PACK_SRC))
    {
	x = 0;
	y = 0;
    }

    w = iRandom(min((FxU32)diago.tsize, max(0, width - x)));
    h = iRandom(min((FxU32)diago.tsize, max(0, height - y)));

    mysstg->srcXY = ((y & 0xFFFF) << 16) | (x & 0xFFFF);
    mysstg->srcSize = (h << 16) | w;
    gdbg_info(6, "srcXY = 0x%08x, srcSize = 0x%08x\n", mysstg->srcXY,
	      mysstg->srcSize);
}



FxU32
sstg_random_srcFormat2(FxU32 dstFormat, int src_is_tiled)
{
    FxU32 srcFormat, stride, pack, align;

    do
    {
	do
	{
	    srcFormat = iRandom(9);
	} while ((srcFormat == 6) || (srcFormat == 7));
		
	srcFormat <<= SSTG_SRC_FORMAT_SHIFT;
	// no 15bpp support, make it be 16bpp
	if (srcFormat == SSTG_PIXFMT_15BPP)
	    srcFormat = SSTG_PIXFMT_16BPP;
    }
    while (!sstg_compatible_src_dst(srcFormat, dstFormat));

    switch (srcFormat)
    {
      case SSTG_PIXFMT_1BPP:
      case SSTG_PIXFMT_8BPP:
      case SSTG_PIXFMT_24BPP: align = 1; break;
      case SSTG_PIXFMT_16BPP: align = 2; break;
      case SSTG_PIXFMT_422YUV:
      case SSTG_PIXFMT_422UYV:
      case SSTG_PIXFMT_32BPP: align = 4; break;
    }

    if (src_is_tiled)
    {
#ifdef BIGTILESTRIDES
	// tiled destination
	if (iRandom(10) == 0)
	    stride = iRandom(SSTG_SRC_TILE_STRIDE);	// some BIG tile strides
	else
#endif /* #ifdef BIGTILESTRIDES */
	    stride = rRandom(1, MAX_TILE_STRIDE);	// more reasonable
    }
    else
    {
	// linear source
	stride = iRandom(SSTG_SRC_LINEAR_STRIDE);
	stride &= ~(align - 1);
    }
	
    pack = iRandom(3);

    if (src_is_tiled)
	pack = SSTG_SRC_PACK_SRC >> SSTG_SRC_PACK_SHIFT;
    else
    {
	if ((srcFormat == SSTG_PIXFMT_15BPP) ||
	    (srcFormat == SSTG_PIXFMT_16BPP))
	{
	    if (pack == 1)
		pack = 0;
	}
	else if ((srcFormat == SSTG_PIXFMT_32BPP) ||
		 (srcFormat == SSTG_PIXFMT_422YUV) ||
		 (srcFormat == SSTG_PIXFMT_422UYV))
	{
	    if (pack == 1)
		pack = 0;
	    if (pack == 2)
		pack = 3;
	}
    }

    srcFormat |= pack << SSTG_SRC_PACK_SHIFT;
    srcFormat |= stride;

    return srcFormat;
}

void
generate_src_surf_in_dest(SstGRegs *mysstg, FxU32 dstWidth, FxU32 dstHeight)
{
}
 
#define INSIDE(x, y) ((x >= lxD) && (x <= rxD) && (y >= tyD) && (y <= byD))

FxBool
src_dst_overlap(FxU32 dstX, FxU32 dstY, FxU32 srcX, FxU32 srcY,
		FxU32 width, FxU32 height)
{
    long xs, ys;
    long xd, yd;
    long lxD, rxD, tyD, byD, lxS, rxS, tyS, byS;
    
    if (IS_NEGATIVE(srcX))
	xs = ((long)srcX << (31 - 12)) >> (31 - 12);
    else
	xs = srcX & 0x0FFF;

    if (IS_NEGATIVE(srcY))
	ys = ((long)srcY << (31 - 12)) >> (31 - 12);
    else
	ys = srcY & 0x1FFF;

    xd = (long)dstX;
    if (xd < 0)
    {
	GDBG_ERROR("src_dst_overlap", "funky dstX conversion\n");
	DIAG_FAIL();
    }
    yd = (long)dstY;
    if (yd < 0)
    {
	GDBG_ERROR("src_dst_overlap", "funky dstY conversion\n");
	DIAG_FAIL();
    }

    lxD = xd;
    rxD = xd + width - 1;
    tyD = yd;
    byD = yd + height - 1;

    lxS = xs;
    rxS = xs + width - 1;
    tyS = ys;
    byS = ys + height - 1;
    
    if (INSIDE(lxS, byS))
	return 1;
    if (INSIDE(lxS, tyS))
	return 1;
    if (INSIDE(rxS, tyS))
	return 1;
    if (INSIDE(rxS, byS))
	return 1;

    return 0;
}

//
// extract the signed x and y values from a h/w packed XY register
//
void
unpackXY(FxU32 packedXY, long *xOut, long *yOut)
{
    FxU32 srcX = packedXY & 0x1FFF;
    FxU32 srcY = (packedXY >> 16) & 0x1FFF;
    long xs, ys;
    
    if (IS_NEGATIVE(srcX))
	xs = ((long)srcX << (31 - 12)) >> (31 - 12);
    else
	xs = srcX & 0x0FFF;

    if (IS_NEGATIVE(srcY))
	ys = ((long)srcY << (31 - 12)) >> (31 - 12);
    else
	ys = srcY & 0x1FFF;

    *xOut = xs;
    *yOut = ys;
}

FxBool
polyWantaClipper(SstGRegs *mysstg, Polygon *p, FxU32 width, FxU32 height)
{
    int i;
    
    if ((p->xl >= (long)width) ||
	(p->xr >= (long)width) ||
	(p->yt >= (long)height) ||
	(p->yb >= (long)height))
	return 1;
    else
	return 0;
    
    for (i = 0; i < p->nLeft; i++)
    {
	if ((p->leftEdges[i].x >= (long)width) ||
	    (p->leftEdges[i].y >= (long)height))
	    return 1;
    }
    for (i = 0; i < p->nRight; i++)
    {
	if ((p->rightEdges[i].x >= (long)width) ||
	    (p->rightEdges[i].y >= (long)height))
	    return 1;
    }

    return 0;
}


static FxU32 _bpp[] =
{
    0, 1, 2, 2, 3, 4
};


void
main(int argc, char **argv)
{
    SstRegs *sst;
    SstGRegs *sstg;
    SstGRegs mysstg;
    int i, firstLaunch, first_cp, last_cp, cp, repeatCount, launchit;
    int nblits, doitAgain, j;
    int srcBelowDst, firsttime;
    FxU32 newDstX, newDstY, newDstXY, style, temp, cmd;
    FxU32 srcSurfaceSize, destSurfaceSize, dstSurfWidth;
    FxU32 srcSurfWidth, dstSurfHeight, srcSurfHeight;
    long srcX, srcY;
    FxU32 width, height;
    FxU32 dstX, dstY;
    long xd, yd, xs, ys, xd2, yd2, xd3, yd3;
    FxU32 w, h;
    FxU32 bit, byte;
    FxU32 stride;
    FxU32 newFmt;
    int ydir;
    FxU32 clipmin, clipmax;
    Polygon poly;
    int count;
    FxU32 pass = 0;
    FxBool alreadyDiffed = 0;
    FxBool doingTooMany = 0;
    FxU32 strideBytes, widthBytes, mybpp, pixFormat;

    sst = SST_BEGIN2d(argc,argv);
    sstg = SSTG_CHIP(sst);

    set_initial_state(&mysstg);
    set_initial_change_frequencies(&change);
    XParseOpts(argc, argv, &mysstg);
    dump_state(sstg, &mysstg);

    if (initMemory)
	sstg_init_random_memory(NULL);

    while (DIAG_STARTPASS())			// for each pass
    {
    if (doDumpState && (++pass == dumpStatePass))
	dumpStateToFile(&mysstg);
    if (doLoadState)
    {
	loadStateFromFile(&mysstg);
	dump_state(sstg, &mysstg);
	doLoadState = 0;
    }
    
    for (i = 0; i < iterationsPerPass; i++)
    {
	if ((i == 0) && repeatable)
	{
	    set_initial_state(&mysstg);
	    set_initial_change_frequencies(&change);
	    XParseOpts(argc, argv, &mysstg);
	    dump_state(sstg, &mysstg);
	}
	    
	// set up destination surface -- base address and format
	if (DO_REG(dstBaseAddr))
	{
	    mysstg.dstBaseAddr = rRandom(diago.minTrashMem,
					 diago.maxTrashMem - 4);
	    if (DO(tiledDest))
	    {
#if 0
		// prevent the case where a baseaddress is close to the
		// right side of the last tile in memory
		if (((mysstg.dstBaseAddr & SST_TILE_WIDTH_MASK) >=
							(SST_TILE_WIDTH - 1))
		    && ((mysstg.dstBaseAddr + 2 + SST_TILE_SIZE -
			 SST_TILE_WIDTH) > diago.maxTrashMem))
		{
		    mysstg.dstBaseAddr -= 0;
		}
#endif /* #if 0 */

		mysstg.dstBaseAddr |= SSTG_IS_TILED;
	    }
	    
	    new.dstBaseAddr = 1;
	}
	if (DO_REG(dstFormat) || new.dstBaseAddr)
	{
	    mysstg.dstFormat = sstg_random_dstFormat(&mysstg);
	    new.dstFormat = 1;
	}

	if (new.dstBaseAddr || new.dstFormat)
	{
	    // fix up dstBase to match dstFormat
	    align_baseAddr_format((FxU32 *)&mysstg.dstBaseAddr,
				  (FxU32 *)&mysstg.dstFormat,
				  (FxU32 *)&new.dstBaseAddr,
				  (FxU32 *)&new.dstFormat);
	    
	    if (!(mysstg.dstBaseAddr & SSTG_IS_TILED) &&
		(change.dstFormat == 0))
	    {
		// fixed destination format.  we may have previously
		// switched to a tiled srcFormat and now selected linear
		// again, which means changing to a very
		// small stride setting.  When a non-tiled base is
		// selected again, set the stride back to the fixed
		// stride (when the destination format is set to never change)
		FxU32 oldStride = (mysstg.dstFormat & SSTG_DST_LINEAR_STRIDE) >>
					    SSTG_DST_STRIDE_SHIFT;
		if (oldStride != 2048)
		{
		    mysstg.dstFormat &= ~SSTG_DST_LINEAR_STRIDE;
		    mysstg.dstFormat |= 2048 << SSTG_DST_STRIDE_SHIFT;
		    new.dstFormat = 1;
		}
	    }

	    SETREG(dstBaseAddr);
	}
	
	// command register stuff
	if (DO_REG(command))
	{
	    if (DO(monotrans))
	    {
		mysstg.command ^= SSTG_TRANSPARENT;
		new.command = 1;
	    }
	    if (DO(reversible))
	    {
		mysstg.command ^= SSTG_REVERSIBLE;
		new.command = 1;
	    }
	    if (DO(updateX))
	    {
		mysstg.command ^= SSTG_UPDATE_DSTX;
		new.command = 1;
	    }
	    if (DO(updateY))
	    {
		mysstg.command ^= SSTG_UPDATE_DSTY;
		new.command = 1;
	    }
	    if (DO(lineStipple))
	    {
		mysstg.command ^= SSTG_EN_LINESTIPPLE;
		new.command = 1;
	    }
	    if (DO(monopattern))
	    {
		mysstg.command ^= SSTG_MONO_PATTERN;
		new.command = 1;
	    }
	    if (DO(xdir))
	    {
		mysstg.command ^= SSTG_XDIR;
		new.command = 1;
	    }
	    if (DO(_ydir))
	    {
		mysstg.command ^= SSTG_YDIR;
		new.command = 1;
	    }
	    if (DO(xpatoff))
	    {
		mysstg.command &= ~SSTG_X_PATOFFSET;
		mysstg.command |= iRandom(0x7) << SSTG_X_PATOFFSET_SHIFT;
		new.command = 1;
	    }
	    if (DO(ypatoff))
	    {
		mysstg.command &= ~SSTG_Y_PATOFFSET;
		mysstg.command |= iRandom(0x7) << SSTG_Y_PATOFFSET_SHIFT;
		new.command = 1;
	    }
	    if (DO(clipselect))
	    {
		mysstg.command ^= SSTG_CLIPSELECT;
		new.command = 1;
	    }
	    if (DO(rop0))
	    {
		mysstg.command &= ~SSTG_ROP0;
		mysstg.command |= iRandom(0xFF) << SSTG_ROP0_SHIFT;
		new.command = 1;
	    }
	    if (DO(primitive))
	    {
		int new_command;
		
		mysstg.command &= ~SSTG_COMMAND;
		do 
		{
		    new_command = rRandom(1, 8);
		} while (new_command == 4);
		
		mysstg.command |= new_command;
		new.command = 1;
	    }
	}

	if (DO_REG(lineStipple))
	{
	    mysstg.lineStipple = iRandom(0xFFFFFFFF);
	    new.lineStipple = 1;
	}
	if (DO_REG(lineStyle))
	{
	    style = iRandom(iRandom(iRandom(0xFF)));	// repeat count
	    style = iRandom(style);	// skew it towards smaller ones
	    style |= (temp=iRandom(0x1F)) << SSTG_LSSIZE_SHIFT;	// size
	    style |= iRandom(temp) << SSTG_LSPOS_INT_SHIFT;	// int pos
	    style |= iRandom((style&0xFF)) << SSTG_LSPOS_FRAC_SHIFT;// frac pos
	    mysstg.lineStyle = style;
	    new.lineStyle = 1;
	}

	if (DO(fixedPrimType))
	{
	    mysstg.command &= ~SSTG_COMMAND;
	    mysstg.command |= fixedPrim;
	    new.command = 1;
	}

	if (DO_REG(commandEx))
	{
	    if (DO(srcColorkey))
	    {
		mysstg.commandEx ^= SSTG_EN_SRC_COLORKEY_EX;
		new.commandEx = 1;
	    }
	    if (DO(dstColorkey))
	    {
		mysstg.commandEx ^= SSTG_EN_DST_COLORKEY_EX;
		new.commandEx = 1;
	    }
	    if (DO(vsyncWait))
	    {
#ifdef H3_A0
		mysstg.commandEx |= SSTG_WAIT_FOR_VSYNC_EX;
		new.commandEx = 1;
#endif
	    }
	    else
	    {
		mysstg.commandEx &= ~SSTG_WAIT_FOR_VSYNC_EX;
		new.commandEx = 1;
	    }
	    
	    if (DO(patRow0))
	    {
		mysstg.commandEx |= SSTG_PAT_FORCE_ROW0;
		new.commandEx = 1;
	    }
	    else
	    {
		mysstg.commandEx &= ~SSTG_PAT_FORCE_ROW0;
		new.commandEx = 1;
	    }
	}
mysstg.commandEx &= ~SSTG_WAIT_FOR_VSYNC_EX;	

	// always need to adjust the dstSize to fit in board memory
	// note that destSize, dstWidth, dstHeight are the size of
	// the *destination surface*, the primitive drawn will most likely
	// be a subrectangle inside the dest. surface
	//
	count = 0;
	do 
	{
	    destSurfaceSize = sstg_random_dstSurface(&mysstg);
	    dstSurfWidth = destSurfaceSize & 0xFFFF;
	    dstSurfHeight = (destSurfaceSize >> 16) & 0xFFFF;
	    count += 1;
	} while ((count < 4) &&
		 ((mysstg.command & SSTG_COMMAND) == SSTG_POLYFILL) &&
		 ((dstSurfWidth < 4) || (dstSurfHeight < 4)));
	if (count >= 4)
	{
	    i -= 1;
	    continue;
	}

	// if the destination stride is less than the destination
	// surface width, then don't allow destination reads (read/write
	// hazard in the h/w)
	// first, calculate stride in bytes
	if (mysstg.dstBaseAddr & SSTG_IS_TILED)
	{
	    strideBytes = ((mysstg.dstFormat & SSTG_DST_TILE_STRIDE) >>
			   SSTG_DST_STRIDE_SHIFT) * 128;
	}
	else
	{
	    strideBytes = (mysstg.dstFormat & SSTG_DST_LINEAR_STRIDE) >>
						SSTG_DST_STRIDE_SHIFT;
	}
	// then, find destination width in bytes
	pixFormat = (mysstg.dstFormat & SSTG_DST_FORMAT) >>
					    SSTG_DST_FORMAT_SHIFT;
	mybpp = _bpp[pixFormat];
	gdbg_info(15, "strideBytes = %d\n", strideBytes);
	widthBytes = dstSurfWidth * mybpp;
	gdbg_info(15, "widthBytes = %d", widthBytes);
	if (widthBytes > strideBytes)
	{
	    dstSurfWidth = strideBytes / mybpp;
	    gdbg_info_more(15,", now = %d / %d = %d pixels\n",
			   strideBytes, mybpp, dstSurfWidth);
	}
	else
	    gdbg_info_more(15, "\n");

	if (strideBytes < mybpp)
	{
	    gdbg_info(44, "strideBytes too small, force SRCCOPY rop,");
	    gdbg_info_more(44, " no color keying\n");

	    mysstg.command &= ~SSTG_ROP0;
	    mysstg.command |= SSTG_ROP_SRC << SSTG_ROP0_SHIFT;
	    mysstg.commandEx &= ~SSTG_EN_SRC_COLORKEY_EX;
	    mysstg.commandEx &= ~SSTG_EN_DST_COLORKEY_EX;
	    new.commandEx = 1;
	}
		
	// set up source surface -- base address and format
	if (DO_REG(srcBaseAddr))
	{
	    if (DO(src_EQUALS_dst))
	    {
		src_is_dst = 1;
	    }
	    else
	    {
		src_is_dst = 0;
		
		if (iRandom(1) == 0)
		    srcBelowDst = 1;
		else
		    srcBelowDst = 0;
		firsttime = 0;
		do
		{
		    if (srcBelowDst)
		    {
			mysstg.srcBaseAddr =
			    rRandom(diago.minTrashMem,
				    (mysstg.dstBaseAddr & SSTG_BASEADDR) - 4);
			srcBelowDst = 0;
		    }
		    else
		    {
			mysstg.srcBaseAddr =
			    sstg_random_srcBaseAddr2(mysstg.dstFormat,
						     mysstg.dstBaseAddr,
						     dstSurfWidth,
						     dstSurfHeight);
			srcBelowDst = 1;
		    }
		} while ((mysstg.srcBaseAddr >= (FxU32)diago.maxTrashMem) &&
			 (firsttime++ == 0));

		// if we can't fit the destination surafce, toss this
		// iteration and start over, hopefully with a new dst
		// surface
		if (mysstg.srcBaseAddr >= (FxU32)diago.maxTrashMem)
		{
		    i -= 1;
		    continue;
		}
		
		if (DO(tiledSrc))
		{
#if 0
		    if (((mysstg.srcBaseAddr & SST_TILE_WIDTH_MASK) >=
						(SST_TILE_WIDTH - 1))
			&& ((mysstg.srcBaseAddr + 2 + SST_TILE_SIZE -
			     SST_TILE_WIDTH) > diago.maxTrashMem))
		{
		    mysstg.srcBaseAddr -= 0;
		}
#endif /* #if 0 */		    
		    
		    mysstg.srcBaseAddr |= SSTG_IS_TILED;
		}
		
		new.srcBaseAddr = 1;
	    }
	}

	if (src_is_dst)
	{
	    mysstg.srcBaseAddr = mysstg.dstBaseAddr;
	    mysstg.srcFormat = mysstg.dstFormat;
	    new.srcBaseAddr = 1;
	    new.srcFormat = 1;
	}
	else if (DO_REG(srcFormat) || (mysstg.srcBaseAddr & SSTG_IS_TILED) ||
		 (GIMME(mysstg.command, SSTG_COMMAND) == SSTG_BLT) ||
		 (GIMME(mysstg.command, SSTG_COMMAND) == SSTG_STRETCH_BLT) ||
		 (GIMME(mysstg.command, SSTG_COMMAND) == SSTG_HOST_BLT))
	{
	    // select a srcFormat that is compatible with the current
	    // dstFormat
	    mysstg.srcFormat = sstg_random_srcFormat2(mysstg.dstFormat,
						      (mysstg.srcBaseAddr &
						       SSTG_IS_TILED));
	    new.srcFormat = 1;

	    // make adjustments for incompatible settings
	    if ((mysstg.srcFormat & SSTG_SRC_FORMAT) == SSTG_PIXFMT_1BPP)
	    {
		mysstg.commandEx &= ~SSTG_EN_SRC_COLORKEY_EX;
		new.commandEx = 1;
	    }
	}

	// if src_is_dst, there's no reason to adjust the srcFormat
	if (!src_is_dst && (new.srcBaseAddr || new.srcFormat))
	{
	    // fix up dstBase to match dstFormat
	    align_baseAddr_format((FxU32 *)&mysstg.srcBaseAddr,
				  (FxU32 *)&mysstg.srcFormat,
				  (FxU32 *)&new.srcBaseAddr,
				  (FxU32 *)&new.srcFormat);
	    if (!(mysstg.srcBaseAddr & SSTG_IS_TILED) &&
		(change.srcBaseAddr == 0))
	    {
		// put back linear stride if we switched to tiled space
		// and we've fixed the source format on the command line
		FxU32 oldStride = (mysstg.srcFormat & SSTG_SRC_TILE_STRIDE) >>
					    SSTG_SRC_STRIDE_SHIFT;
		if (oldStride != 2048)
		{
		    mysstg.srcFormat &= ~SSTG_DST_TILE_STRIDE;
		    mysstg.srcFormat |= 2048 << SSTG_DST_STRIDE_SHIFT;
		    new.srcFormat = 1;
		}
	    }
	}
	SETREG(srcBaseAddr);
	if (GIMME(mysstg.srcBaseAddr,SSTG_BASEADDR) > (FxU32)diago.maxTrashMem)
	{
	    GDBG_ERROR("stress2d", "bad src base addr\n");
	    DIAG_FAIL();
	}

	// allocate a source surface, the actual primitive
	// drawn will probably use a subrectangle of this surface
	// 
	if (src_is_dst)
	{
	    srcSurfWidth = dstSurfWidth;
	    srcSurfHeight = dstSurfHeight;
	}
	else
	{
	    srcSurfaceSize = sstg_random_srcSurface(&mysstg);
	    srcSurfWidth = srcSurfaceSize & 0xFFFF;
	    srcSurfHeight = (srcSurfaceSize >> 16) & 0xFFFF;
	}

	// foreground color
	if (DO_REG(colorFore))
	{
	    mysstg.colorFore = iRandom(0xFFFFFFFF);
	    new.colorFore = 1;
	}

	// background color
	if (DO_REG(colorBack))
	{
	    mysstg.colorBack = iRandom(0xFFFFFFFF);
	    new.colorBack = 1;
	}
	
	if (DO_REG(colorPattern[0]))
	{
	    first_cp = iRandom(63);
	    last_cp = rRandom(first_cp, 63);
	    for (cp = first_cp; cp <= last_cp; cp++)
	    {
		mysstg.colorPattern[cp] = iRandom(0xFFFFFFFF);
		new.colorPattern[cp] = 1;
	    }
	}
	if (DO_REG(pattern0alias))
	{
	    mysstg.pattern0alias = iRandom(0xFFFFFFFF);
	    new.pattern0alias = 1;
	}
	if (DO_REG(pattern1alias))
	{
	    mysstg.pattern1alias = iRandom(0xFFFFFFFF);
	    new.pattern1alias = 1;
	}
	if (DO_REG(srcColorkeyMin))
	{
	    mysstg.srcColorkeyMin = iRandom(0xFFFFFFFF);
	    new.srcColorkeyMin = 1;
	}
	if (DO_REG(srcColorkeyMax))
	{
	    mysstg.srcColorkeyMax = iRandom(0xFFFFFFFF);
	    new.srcColorkeyMax = 1;
	}
	if (DO_REG(dstColorkeyMin))
	{
	    mysstg.dstColorkeyMin = iRandom(0xFFFFFFFF);
	    new.dstColorkeyMin = 1;
	}
	if (DO_REG(dstColorkeyMax))
	{
	    mysstg.dstColorkeyMax = iRandom(0xFFFFFFFF);
	    new.dstColorkeyMax = 1;
	}
	if (DO_REG(rop))
	{
	    mysstg.rop = iRandom(0xFFFFFFFF);
	    new.rop = 1;
	}

	//
	// choose command source and destination rectangles, as appropriate
	// for the given command, from the source and destination
	// surfaces
	//
	cmd = GIMME(mysstg.command, SSTG_COMMAND);
	switch (cmd)
	{
	  case SSTG_RECTFILL:
	  case SSTG_HOST_BLT:
	      mysstg.command &= ~SSTG_XDIR;
	      generate_dstXY_dstSize(&mysstg, dstSurfWidth, dstSurfHeight);
	      break;

	  case SSTG_POLYFILL:
	      // only care about dstXY, the starting point
	      // might have to be careful about having the polygon
	      // fall outside of the destination surface.
	      count = 0;
	      do
	      {
		  generate_dstXY_dstSize(&mysstg, dstSurfWidth, dstSurfHeight);
		  count += 1;
	      } while ((count < 10) &&
		       (((mysstg.dstSize & 0xFFFF) < 4) ||
			(((mysstg.dstSize >> 16) & 0xFFFF) < 4)));
	      if (count >= 10)
	      {
		  i -= 1;
		  continue;
	      }
	      
	      mysstg.commandEx &= ~SSTG_EN_SRC_COLORKEY_EX;
	      new.commandEx = 1;
	      break;

	  case SSTG_STRETCH_BLT:
	  case SSTG_BLT:
	      count = 0;
tryItAgain:
	      generate_dstXY_dstSize(&mysstg, dstSurfWidth, dstSurfHeight);
	      generate_srcXY_srcSize(&mysstg, srcSurfWidth, srcSurfHeight);
	      if (cmd == SSTG_BLT)
	      {
		  // take the smaller of the src/dst rectangle's sizes
		  // only for screen to screen
		  width = min(mysstg.dstSize & 0xFFFF,
			      mysstg.srcSize & 0xFFFF);
		  height = min((mysstg.dstSize >> 16) & 0xFFFF,
			       (mysstg.srcSize >> 16) & 0xFFFF);
		  mysstg.dstSize = (height << 16) | width;
	      }
	      else
	      {
		  width = max(mysstg.dstSize & 0xFFFF,
			      mysstg.srcSize & 0xFFFF);
		  height = max((mysstg.dstSize >> 16) & 0xFFFF,
			       (mysstg.srcSize >> 16) & 0xFFFF);

	      }

	      if (src_is_dst)
	      {
		  dstX = mysstg.dstXY & 0x1FFF;
		  dstY = (mysstg.dstXY >> 16) & 0x1FFF;
		  srcX = mysstg.srcXY & 0x1FFF;
		  srcY = (mysstg.srcXY >> 16) & 0x1FFF;

		  // look for overlapping rectangles and set/clear the
		  // direction flags to make the blit work
		  // if no overlap, allow whatever random settings have
		  // made it into the command register
		  if (src_dst_overlap(dstX, dstY, srcX, srcY, width, height))
		  {
		      if (cmd == SSTG_STRETCH_BLT)
		      {
			  // stretch blit can't deal with overlapping
			  // source / dest (without a lot more effort!)
			  // ugh, it's a goto, but who cares
			  count += 1;
			  if (count > 10)
			  {
			      i -= 1;
			      continue;
			  }
			  goto tryItAgain;
		      }
		      
		      if (IS_NEGATIVE(dstY) || ((long)dstY < srcY))
		      {
			  // must copy top down
			  mysstg.command &= ~SSTG_YDIR;
		      }
		      else if (!IS_NEGATIVE(dstY) && ((long)dstY > srcY))
		      {
			  // must copy bottom up
			  mysstg.command |= SSTG_YDIR;
		      }
		      else if (srcY == (long)dstY)
		      {
			  if (IS_NEGATIVE(dstX) || ((long)dstX < srcX))
			  {
			      // must copy left to right
			      mysstg.command &= ~SSTG_XDIR;
			  }
			  else
			  {
			      // must copy right to left
			      mysstg.command |= SSTG_XDIR;
			      mysstg.commandEx &= ~(SSTG_EN_SRC_COLORKEY_EX |
						    SSTG_EN_DST_COLORKEY_EX);
			      new.commandEx = 1;
			  }
		      }
		  } // if overlap
	      } // if src_is_dst
gdbg_info(7, "DEST:ba = 0x%08x, Frmt=0x%08x, XY=(%d,%d), size=(%d,%d)\n",
	  mysstg.dstBaseAddr, mysstg.dstFormat,
	  (short)(mysstg.dstXY & 0xFFFF),
	  (short)((mysstg.dstXY >> 16) & 0xFFFF),
	  mysstg.dstSize & 0xFFFF,
	  (mysstg.dstSize >> 16) & 0xFFFF);

gdbg_info(7, "SRC: ba = 0x%08x, Frmt=0x%08x, XY=(%d,%d), size=(%d,%d)\n",
	  mysstg.srcBaseAddr, mysstg.srcFormat,
	  (short)(mysstg.srcXY & 0xFFFF),
	  (short)((mysstg.srcXY >> 16) & 0xFFFF),
	  mysstg.srcSize & 0xFFFF,
	  (mysstg.srcSize >> 16) & 0xFFFF);

	      break;
	      
	  case SSTG_POLYLINE:
	  case SSTG_LINE:
	      generate_signed_XY(&srcX, &srcY, dstSurfWidth, dstSurfHeight);
	      mysstg.srcXY = (srcY << 16) | (srcX & 0xFFFF);
	      if (mysstg.commandEx & SSTG_EN_SRC_COLORKEY_EX)
	      {
		  mysstg.commandEx &= ~SSTG_EN_SRC_COLORKEY_EX;
		  new.commandEx = 1;
	      }
	      break;
	      
	  default:
	      GDBG_ERROR("stress2d", "trying to to host to screen stretch!\n");
	}

	if (DO_REG(clip0min))
	{
	    // do something "funny" with the clip registers
	    mysstg.clip0min = iRandom(0xFFFFFFFF);
	    mysstg.clip0max = iRandom(0xFFFFFFFF);
	    mysstg.clip1min = iRandom(0xFFFFFFFF);
	    mysstg.clip1max = iRandom(0xFFFFFFFF);
	    new.clip0min = new.clip0max = new.clip1min = new.clip1max = 1;
	}
	else
	{
	    // set the selected clip register so that the current
	    // destination size is unclipped, or partially clipped, or
	    // completely clipped
	    clipmin = mysstg.dstXY & 0xFFFF;
	    clipmax = (clipmin + mysstg.dstSize) & 0xFFFF;

	    clipmin |= mysstg.dstXY & 0xFFFF0000;
	    if (IS_NEGATIVE(clipmin) || IS_NEGATIVE(clipmin >> 16))
		clipmin = 0;

	    clipmax |= (clipmin & 0xFFFF0000) +	(mysstg.dstSize & 0xFFFF0000);
	}
	
	if (DO(zeroClipMin))
	    clipmin = 0;
	if (DO(maxClipMax))
	    clipmax = 0xFFFFFFFF;

	if (mysstg.command & SSTG_CLIPSELECT)
	{
	    if (mysstg.clip1min != clipmin)
	    {
		mysstg.clip1min = clipmin;
		new.clip1min = 1;
	    }
	    if (mysstg.clip1max != clipmax)
	    {
		mysstg.clip1max = clipmax;
		new.clip1max = 1;
	    }
	}
	else
	{
	    if (mysstg.clip0min != clipmin)
	    {
		mysstg.clip0min = clipmin;
		new.clip0min = 1;
	    }
	    if (mysstg.clip0max != clipmax)
	    {
		mysstg.clip0max = clipmax;
		new.clip0max = 1;
	    }
	}


	SETREG(pattern0alias);
	SETREG(pattern1alias);
	
	SETREG(lineStyle);
	SETREG(lineStipple);

	SETREG(clip0min);
	SETREG(clip0max);
	SETREG(clip1min);
	SETREG(clip1max);	

	SETREG(dstFormat);
	SETREG(srcFormat);
	SETREG(colorFore);
	SETREG(colorBack);

	SETREG(commandEx);
	SETREG(rop);
	SETREG(srcColorkeyMin);
	SETREG(srcColorkeyMax);
	SETREG(dstColorkeyMin);
	SETREG(dstColorkeyMax);

	for (cp = 0; cp < 64; cp++)
	{
	    SETREG(colorPattern[cp]);
	}

	//
	// finally, actually issue a command
	//
	switch ((mysstg.command & SSTG_COMMAND) >> SSTG_COMMAND_SHIFT)
	{
	  case SSTG_RECTFILL:
	      firstLaunch = 1;
	      SET(sstg->dstSize, mysstg.dstSize);
	      do // potentially several rectFill commands
	      {
		  if (DO(rectFillLaunch))
		  {
		      if (firstLaunch)
		      {
			  SET(sstg->command, mysstg.command);
		      }
		      SET(sstg->launch[iRandom(31)], mysstg.dstXY);
		  }
		  else
		  {
		      SET(sstg->dstXY, mysstg.dstXY);
		      SET(sstg->command, mysstg.command | SSTG_GO);
		  }
		  firstLaunch = 0;
		  newDstX = (mysstg.dstXY & 0xFFFF)
		      - iRandom((mysstg.dstSize & 0xFFFF) / 2);
		  newDstY = ((mysstg.dstXY >> 16) & 0xFFFF)
		      - iRandom(((mysstg.dstSize >> 16) & 0xFFFF) / 2);
		  newDstXY = (newDstY << 16) | newDstX;
		  mysstg.dstXY = newDstXY;
		  if (diffMemory)
		  {
		      DIAG_DIFFMEMORY();
		      alreadyDiffed = 1;
		  }
		  
	      } while (DO(repeatedRectFill));
	      break;

	  case SSTG_LINE:
	  case SSTG_POLYLINE:
	      firstLaunch = 1;
	      SET(sstg->srcXY, mysstg.srcXY);
	      if (DO(repeatedLineDraw))
		  repeatCount = rRandom(1, 6);
	      else
		  repeatCount = 1;
	      launchit = DO(lineLaunch);
	      do
	      {
		  generate_line_endpoint(&mysstg, dstSurfWidth, dstSurfHeight);
		  if (launchit)
		  {
		      if (firstLaunch)
		      {
			  SET(sstg->command, mysstg.command);
		      }
		      SET(sstg->launch[iRandom(31)], mysstg.dstXY);
		  }
		  else
		  {
		      SET(sstg->dstXY, mysstg.dstXY);
		      SET(sstg->command, mysstg.command | SSTG_GO);
		  }
		  mysstg.srcXY = mysstg.dstXY;
		  firstLaunch = 0;
		  if (diffMemory)
		  {
		      DIAG_DIFFMEMORY();
		      alreadyDiffed = 1;
		  }
		  
	      } while (--repeatCount > 0);
	      break;

	  case SSTG_BLT:
	      if ((mysstg.command & SSTG_XDIR) &&
		  (GIMME(mysstg.srcFormat, SSTG_SRC_FORMAT) !=
		   GIMME(mysstg.dstFormat, SSTG_DST_FORMAT)))
	      {
		  // don't allow src conversion with negative X direction
		  // we can safely turn off xdir here because color conversion
		  // isn't done within the same surface in stress2d
		  mysstg.command &= ~SSTG_XDIR;
	      }
	      if ((mysstg.command & SSTG_XDIR) &&
		   ((mysstg.commandEx & SSTG_EN_SRC_COLORKEY_EX) || 
		    (mysstg.commandEx & SSTG_EN_DST_COLORKEY_EX)))
	      {
		  if (src_is_dst)
		  {
		      // same surface blit, can't turn off Xdir
		      mysstg.commandEx &= ~SSTG_EN_SRC_COLORKEY_EX;
		      mysstg.commandEx &= ~SSTG_EN_DST_COLORKEY_EX;
		      SET(sstg->commandEx, mysstg.commandEx);
		  }
		  else
		  {
		      // not same surface blit, turn off Xdir (cuz we don't
		      // really need it, cuz src and dst don't overlap)
		      mysstg.command &= ~SSTG_XDIR;
		  }
	      }
		      
	      unpackXY(mysstg.dstXY, &xd, &yd);
	      unpackXY(mysstg.srcXY, &xs, &ys);
	      if ((mysstg.command & SSTG_XDIR) && (width > 0))
	      {
		  xs += width - 1;
		  xd += width - 1;
	      }
	      if ((mysstg.command & SSTG_YDIR) && (height > 0))
	      {
		  ys += height - 1;
		  yd += height - 1;
	      }
	      mysstg.dstXY = (yd << 16) | (xd & 0xFFFF);
	      mysstg.srcXY = (ys << 16) | (xs & 0xFFFF);
	      
	      SET(sstg->dstXY, mysstg.dstXY);
	      SET(sstg->dstSize, (height << 16) | width);
	      if (DO(launchBlt))
	      {
		  SET(sstg->command, mysstg.command);
		  SET(sstg->launch[iRandom(31)], mysstg.srcXY);
	      }
	      else
	      {
		  SET(sstg->srcXY, mysstg.srcXY);
		  SET(sstg->command, mysstg.command | SSTG_GO);
	      }
	      break;

	  case SSTG_HOST_BLT:
	      SET(sstg->dstXY, mysstg.dstXY);
	      SET(sstg->dstSize, mysstg.dstSize);
	      SET(sstg->command, mysstg.command);
	      ydir = mysstg.command & SSTG_YDIR;
	      unpackXY(mysstg.dstXY, &xd, &yd);
	      w = mysstg.dstSize & 0xFFFF;
	      h = (mysstg.dstSize >> 16) & 0xFFFF;
	      byte = iRandom(3);
	      bit = iRandom(7);
	      if ((mysstg.srcFormat & SSTG_SRC_FORMAT) == SSTG_PIXFMT_1BPP)
	      {
		  SET(sstg->srcXY, (iRandom(0xFFFFFFFF) & ~31) |
						      ((byte * 8) + bit));
	      }
	      else
	      {
		  SET(sstg->srcXY, (iRandom(0xFFFFFFFF) & ~3) | byte);
	      }
	      
	      // we need to put in a stride that matches the source format
	      stride = 0;
	      switch (mysstg.srcFormat & SSTG_SRC_FORMAT)
	      {
		case SSTG_PIXFMT_1BPP:
		case SSTG_PIXFMT_8BPP:
		case SSTG_PIXFMT_24BPP:
		    stride = iRandom(3);
		    break;
		case SSTG_PIXFMT_15BPP:
		case SSTG_PIXFMT_16BPP:
		    stride = iRandom(1) * 2;
		    break;
	      }
	      newFmt = mysstg.srcFormat & ~SSTG_SRC_LINEAR_STRIDE;
	      newFmt |= (stride << SSTG_SRC_STRIDE_SHIFT);
	      if (newFmt != mysstg.srcFormat)
	      {
		  mysstg.srcFormat = newFmt;
		  SET(sstg->srcFormat, mysstg.srcFormat);
	      }
		    
	      doitAgain = (DO(hostBltUpdateXY) &&
			   ((mysstg.command & SSTG_UPDATE_DSTX) ||
			    (mysstg.command & SSTG_UPDATE_DSTY)));
#if 0
	      if (doitAgain)
		  nblits = rRandom(2, 3);
	      else
		  nblits = 1;
#endif
	      // KMW -- multiple launch host blits have been removed
	      nblits = 1;

#if 0
	      while ((mysstg.command & SSTG_UPDATE_DSTX) &&
		     ((int)(xd + (w * nblits)) > (int)dstSurfWidth))
	      {
		  nblits -= 1;
		  if (nblits == 0)
		  {
		      GDBG_ERROR("stress2d", "nblits should never be 0\n");
		  }
	      }
	      while ((mysstg.command & SSTG_UPDATE_DSTY) &&
		     ((int)(yd + (h * nblits)) > (int)dstSurfHeight))
	      {
		  nblits -= 1;
		  if (nblits == 0)
		  {
		      GDBG_ERROR("stress2d", "nblits should never be 0\n");
		  }
	      }
#endif
	      
	      if (DO(tooFewHblitWords))
	      {
		  gdbg_info(9, "doing (possibly) too few hblit words\n");
		  
		  SET(sstg->clip0max, 0);
		  SET(sstg->clip1max, 0);

		  for (j = 0; j < (FxI32)iRandom(10) + 1; j++)
		  {
		      SET(sstg->launch[iRandom(31)], 0xdeadbeef);
		  }

		  SET(sstg->clip0max, mysstg.clip0max);
		  SET(sstg->clip1max, mysstg.clip1max);

		  break;
	      }
	      
	      if (DO(tooManyHblitWords))
	      {
		  doingTooMany = 1;
		  gdbg_info(9, "doing too many hblit words\n");
		  SET(sstg->clip0max, 0);
		  SET(sstg->clip1max, 0);
	      }
	      
	      for (j = 1; j <= nblits; j++)
	      {
		  switch(mysstg.srcFormat & SSTG_SRC_PACK)
		  {
		    case SSTG_SRC_PACK_SRC:
			sendBltData0(sstg, byte, bit, xd, yd, w,h, ydir,
				     mysstg.srcFormat, host_pixels);
			break;
		    case SSTG_SRC_PACK_8:
			sendBltDataPacked(sstg, 8, byte, xd,yd, w,h, ydir,
					  mysstg.srcFormat, host_pixels);
			break;
		    case SSTG_SRC_PACK_16:
			sendBltDataPacked(sstg, 16, byte, xd,yd, w,h, ydir,
					  mysstg.srcFormat, host_pixels);
			break;
		    case SSTG_SRC_PACK_32:
			sendBltDataPacked(sstg, 32, byte, xd,yd, w,h, ydir,
					  mysstg.srcFormat, host_pixels);
			break;
		  }
	      }

	      if (doingTooMany)
	      {
		  for (j = 0; j < (FxI32)iRandom(10) + 1; j++)
		  {
		      SET(sstg->launch[iRandom(31)], 0xdeadbeef);
		  }
		  SET(sstg->clip0max, mysstg.clip0max);
		  SET(sstg->clip1max, mysstg.clip1max);
	      }
	      
	      break;

	  case SSTG_POLYFILL:
	      switch (iRandom(2))
	      {
		case 0:
		{    
		    poly.numVerts = rRandom(3, 10);
		    unpackXY(mysstg.dstXY, &xd, &yd);
		    sstg_random_poly(&poly, xd, yd, 1);
		    if (polyWantaClipper(&mysstg, &poly,
					 dstSurfWidth, dstSurfHeight))
		    {
			// but seriously,checks to see whether the poly extends
			// beyond the range of the destination surface, if so
			// set the clip regs to the dest surface to prevent
			// memory overruns
			if (mysstg.command & SSTG_CLIPSELECT)
			{
			    mysstg.clip1max = (dstSurfHeight << 16) |
				dstSurfWidth;
			    SET(sstg->clip1max, mysstg.clip1max);
			}
			else
			{
			    mysstg.clip0max = (dstSurfHeight << 16) |
				dstSurfWidth;
			    SET(sstg->clip0max, mysstg.clip0max);
			}
		    }
		    sstg_draw_poly(sstg,&poly, mysstg.command, 0);
		}
		break;

		case 1:
		{
		    // a rectangle drawn via 4 triangles with random
		    // center point
		    unpackXY(mysstg.dstXY, &xd, &yd);
		    w = mysstg.dstSize & 0xFFFF;
		    h = (mysstg.dstSize >> 16) & 0xFFFF;

		    sstg_draw_poly_rectangle(sstg, xd, yd,
					     mysstg.dstSize & 0xFFFF,
					     (mysstg.dstSize >> 16) & 0xFFFF,
					     mysstg.command);
		}
		break;

		case 2:
		{
		    count = 0;
		    // random triangle 
		    unpackXY(mysstg.dstXY, &xd, &yd);
		    do
		    {
			generate_signed_XY(&xd2, &yd2, dstSurfWidth,
					   dstSurfHeight);
			generate_signed_XY(&xd3, &yd3, dstSurfWidth,
					   dstSurfHeight);
			count += 1;
		    } while ((count < 20) &&
			     (sstg_colinear(xd, yd, xd2, yd2, xd3, yd3) ||
			      (sstg_area(xd, yd, xd2, yd2, xd3, yd3) >
				       (FxU32)(diago.tsize * diago.tsize))));
		    if (count >= 20)
		    {
			i -= 1;
			continue;	// dst surf probably too small
		    }

		    sstg_draw_triangle(sstg, xd, yd, xd2, yd2, xd3, yd3,
				       mysstg.command);
		} // case 2
		break;
		
	      } // switch polygon case

	      break;

	  case SSTG_STRETCH_BLT:
	      mysstg.command &= ~(SSTG_XDIR | SSTG_YDIR);
	      SET(sstg->dstXY, mysstg.dstXY);
	      SET(sstg->dstSize, mysstg.dstSize);
	      SET(sstg->srcSize, mysstg.srcSize);
	      if (DO(launchBlt))
	      {
		  SET(sstg->command, mysstg.command);
		  SET(sstg->launch[iRandom(31)], mysstg.srcXY);
	      }
	      else
	      {
		  SET(sstg->srcXY, mysstg.srcXY);
		  SET(sstg->command, mysstg.command | SSTG_GO);
	      }
	      break;
	      
	  case SSTG_HOST_STRETCH_BLT:
	      // ain't no such thing.  let's try this iteration again
	      i -= 1;
	      break;
     
	  default:
	      GDBG_ERROR("stress2d", "bad command primitive\n");
	} // switch command type

	// GREG - Generate a Horizontal line between each primitive!
	generate_signed_XY(&srcX, &srcY, dstSurfWidth, dstSurfHeight);
	mysstg.srcXY = (srcY << 16) | (srcX & 0xFFFF);
	if (srcX && (1<<12)) mysstg.dstXY = (srcY << 16) | ((srcX+128+iRandom(0xf)) & 0xFFFF);
	else mysstg.dstXY = (srcY << 16) | ((srcX-128-iRandom(0xf)) & 0xFFFF);
	SETREG(colorFore);
	SET(sstg->srcXY, mysstg.srcXY);
	SET(sstg->dstXY, mysstg.dstXY);
	SET(sstg->command, 0xcc000006 | SSTG_GO);

	if (diffMemory && !alreadyDiffed)
	{
	    DIAG_DIFFMEMORY();
	}
	alreadyDiffed = 0;

	clear_new_bits();
	
	if (DIAG_EXCEEDED_PIXEL_LIMIT())
	    break;
    } // for i

    } // while DIAG_STARTPASS()

    gdbg_printf("total pixels drawn: %d\n",
		CSIMG_PRIVATE(&CSIM_PRIVATE(diago.sstCSIM)->gui)->pixelsOut2d);

    if (getenv("DIAG_WAIT"))
	getchar();

    DIAG_PASS(0);
 }
