/*
** Copyright (c) 1995-1997, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:18:58 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "stwtri.h"
#include "fbi.h"
#include "lfbutils.h"

#define ABS(a) ((a) > 0 ? (a) : -(a))
#define GLEVEL 125
#define GWARN 2
#define SANITY 150
#define MAXLOOP 500

static int debugPass = -1;
static int waxpDepth;
static int randomTexturing = 0;
static FxU32 gbpp[] = {
  SSTG_PIXFMT_1BPP, SSTG_PIXFMT_8BPP, SSTG_PIXFMT_15BPP, SSTG_PIXFMT_16BPP,
  SSTG_PIXFMT_24BPP, SSTG_PIXFMT_32BPP, SSTG_PIXFMT_422YUV
};
static SstRegs sRegs;
static SstGRegs sGRegs;
static long nrect,ntriangle;
static long nprect;

#define MODNAME "ddtest"

#define MAX_DST_STRIDE 0xFFF

/* myParseOpts
 *
 * look for "-x" options and interpret them for this test
 *
 */

static char *
Xusage()
{
    gdbg_printf("Error Xusage::\n");
    gdbg_printf("\"-xd<n>\"\tDebug Pass #\n");
    gdbg_printf("\n");
    exit(1);
    return(0);
}


#define XGETARG() opts[1] ? done = 1, ++opts : \
			  (--argc > 0) ? done = 1, *++argv : \
					 (char *)Xusage()

static void
XParseOpts(int argc, char **argv)
{
    char *opts = 0;
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
	      // %i ??
	      sscanf(XGETARG(), "%i", &debugPass);
	      GDBG_PRINTF("INFO: debugPass %d\n",debugPass);
	      done = 1;
	      break; 
#if 0
	    case 'c':
	      pType = opts[1];
	      selectpType = 1;
	      GDBG_PRINTF("INFO: selecting type %c\n",pType);
	      opts++;
	      break;
#endif
	    default:
	      Xusage();
	    }
	    opts++;
	}
    }
}


// scarved from hblt.c
static FxU32 screen[MAXSCREEN][MAXSCREEN];

static void rect(SstRegs *sst,unsigned size,int pass)
{
  SstGRegs *sstg;
  FxU32 cmdops,cmdXops;
  FxU32 cfore,cback,rop,cdest;
  long xx,yy,ww,hh;
  sstg = SSTG_CHIP(sst);
  xyRandom(&xx,&yy);
  if (iRandom(7)==0) {			// generate some negative coords
    xx -= 4*size;
    yy -= 3*size;
  }
  ww = rRandom(1,size);		// generate random width height
  hh = rRandom(1,size);
  nrect++;
  nprect += (ww*hh);
  cmdops = 0;
  cmdXops = 0;
  cfore = colRandom32();
  cdest = ONSCREEN(xx,yy) ? screen[yy][xx] : 0;
  rop = sstg_random_colors(sstg, cfore,&cback,cdest); // ternary
  SET(sstg->colorFore,cfore);
  cmdops |= SSTG_ROP_XOR << SSTG_ROP0_SHIFT;
  SET(sstg->rop, rop);		// set the rop
  SET(sstg->commandEx, cmdXops);

  gdbg_info(1,"RECT x,y = %d,%d    w,h = %d,%d    pox,y = %d,%d\n",
	    xx,yy,ww,hh,
	    ((cmdops & SSTG_X_PATOFFSET)>>SSTG_X_PATOFFSET_SHIFT) & 7,
	    ((cmdops & SSTG_Y_PATOFFSET)>>SSTG_Y_PATOFFSET_SHIFT) & 7);
  // sstg_print_stuff(cmdops, cmdXops, rop, srcFormat, cfore, cback);
  sstg_print_stuff(cmdops, cmdXops, rop, 0, cfore, cback);

  if (pass >= debugPass)
    sstg_drawrect(sstg,xx,yy,ww,hh, cmdops);
  else 
    iRandom(1); // remain in sync
}

static unsigned place3d(SstRegs *sst,unsigned startMem,unsigned maxMem)
{
  unsigned maxFbMem;
    // base and stride must be 16-byte aligned
    diagfb.colBufferAddr[0] = startMem;
    diagfb.colBufferStride[0] = diago.xmaxscreen * 2 ;
    diagfb.colBufferAddr[0] &= ~0xF;
    diagfb.colBufferStride[0] &= ~0xF;
	
    diagfb.colBufferAddr[1] = diagfb.colBufferAddr[0]  +
      + diago.ymaxscreen * diagfb.colBufferStride[0] + 64;
    diagfb.colBufferStride[1] = diago.xmaxscreen * 2;
    diagfb.colBufferAddr[1] &= ~0xF;
    diagfb.colBufferStride[1] &= ~0xF;
	
    maxFbMem = diagfb.colBufferAddr[1]  + diago.ymaxscreen * diagfb.colBufferStride[1];
	
    diagfb.colBufferStride[0] |= SST_BUFFER_MEMORY_LINEAR;
    diagfb.colBufferStride[1] |= SST_BUFFER_MEMORY_LINEAR;
      
    if ( diago.hasAuxBuffer ) {
	  diagfb.auxBufferAddr = maxFbMem + 64;
	  diagfb.auxBufferStride = diago.xmaxscreen * 2;
	  diagfb.auxBufferAddr &= ~0xF;
	  diagfb.auxBufferStride &= ~0xF;
	  maxFbMem = diagfb.auxBufferAddr + diago.ymaxscreen * diagfb.auxBufferStride;
	  diagfb.auxBufferStride |= SST_BUFFER_MEMORY_LINEAR;
      } else {
	diagfb.auxBufferAddr = 0x0;
	diagfb.auxBufferStride = 0x0;
      }
      
    // check that buffers fit in memory
    if ( maxFbMem > maxMem ) 
      GDBG_ERROR("place3d","insufficient memory for framebuffers and CMD FIFO\n");


    SET(sst->colBufferAddr,diagfb.colBufferAddr[0]);
    SET(sst->colBufferStride,diagfb.colBufferStride[0]);
    SET(sst->auxBufferAddr,diagfb.auxBufferAddr);
    SET(sst->auxBufferStride,diagfb.auxBufferStride);

    GDBG_INFO(0,"+virtual Front buffer addr,stride=0x%08x,0x%08x %s\n",
	      diagfb.colBufferAddr[0],diagfb.colBufferStride[0]&(~SST_BUFFER_MEMORY_TYPE),
	      (diagfb.colBufferStride[0] & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED ?
	      "tiled" : "linear");
    GDBG_INFO(0,"+virtual Back  buffer addr,stride=0x%08x,0x%08x %s\n",
	      diagfb.colBufferAddr[1],diagfb.colBufferStride[1]&(~SST_BUFFER_MEMORY_TYPE),
	      (diagfb.colBufferStride[1] & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED ?
	      "tiled" : "linear");
    GDBG_INFO(0,"+virtual Aux   buffer addr,stride=0x%08x,0x%08x %s %s\n",
	      diagfb.auxBufferAddr,diagfb.auxBufferStride&(~SST_BUFFER_MEMORY_TYPE),
	      (diagfb.auxBufferStride & SST_BUFFER_MEMORY_TYPE) == SST_BUFFER_MEMORY_TILED ?
	      "tiled" : "linear", diago.hasAuxBuffer ? "" : "(not active)" );

    diagfb.inRegister = 0;
    drawbufferRandom();  // if necessary, reload color buf base/stride regs based on -d flag
    return(maxFbMem);
}



void
my_do_blt(SstGRegs *sstg,
	  int xs, int ys, int xd, int yd, int w, int h,
	  FxU32 cmd)
{
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
}

enum CSM {  TEST_WAX,   TEST_3D   };
void
main(int argc, char **argv)
{
    int ii;
    int ttp;
    SstRegs *sst;
    SstCRegs *sstc;
    SstIORegs *sstio;
    FxU32 fbzCP;
    
    FxU32 memConfig;
    FxU32 tiledOffsetBytes;
    FxU32 psize;
    int bigSize,size;
    int pass = 0;
    Triangle *pt;

    // 2d hblt
    SstGRegs *sstg;
    unsigned waxEnd;
    FxU32 waxStart;

    // 
    int lsize;
    unsigned maxFbMem;

    Triangle tri[3];

    sst = SST_BEGIN2d(argc,argv);
    diago.dstFormat = SSTG_PIXFMT_16BPP >> SSTG_SRC_FORMAT_SHIFT;
    sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);
    sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
    sstg = SSTG_CHIP(sst);

    XParseOpts(argc,argv);

    
    GDBG_INFO(1,"LFB Memory Config 0x%08x\n",memConfig);
    gdbg_info(1,"check et %d\n",diago.checkEveryTriangle);

    waxStart = diago.minTrashMem;
    // 2d setup
     
    if (diago.dstFormat < (SSTG_PIXFMT_8BPP>>SSTG_SRC_FORMAT_SHIFT) ||
	diago.dstFormat > (SSTG_PIXFMT_32BPP>>SSTG_SRC_FORMAT_SHIFT)) {
	GDBG_ERROR(MODNAME, "invalid destination format\n");
	DIAG_FAIL();
    }
    gdbg_printf("Setting dst/src to 0x%x\n",waxStart);
    SET(sstg->dstBaseAddr,waxStart);		// define a default surface
    SET(sstg->srcBaseAddr,waxStart);		// Default 
    waxpDepth = 4;
    SET(sstg->dstFormat,gbpp[diago.dstFormat] | diago.xmaxscreen*2);
    SET(sstg->srcFormat,gbpp[diago.dstFormat] | diago.xmaxscreen*2);
    sGRegs.clip0min = 0;
    SET(sstg->clip0min, 0);
    sGRegs.clip0max = (diago.ymaxscreen<<16) | diago.xmaxscreen;
    SET(sstg->clip0max,sGRegs.clip0max);
    sGRegs.dstBaseAddr = waxStart;
    sGRegs.srcBaseAddr = waxStart;
    // sGRegs.clip0min = GET(sstg->clip0min);
    // sGRegs.clip0max = GET(sstg->clip0max);
    sGRegs.colorBack = 0; // line
    SET(sstg->colorBack, sGRegs.colorBack);
    SET(sst->c1, 0xabcd0123);
      
    tiledOffsetBytes = (memConfig & SST_RAW_LFB_TILE_BEGIN_PAGE) >> SST_RAW_LFB_TILE_BEGIN_PAGE_SHIFT;

    size = diago.tsize;
    if (diago.writeFifo) {
      // blow out cmd fifo
      if (diago.ringSize < 0) 
	psize =  -diago.ringSize;
      else 
	psize = diago.ringSize;
    }


    psize -= 5; // some breathing room
    lsize = (diago.tsize/4 + 1);  if (lsize == 0) lsize = 10;
    for (ii = 10;ii>0;ii--) {
      lsize = (lsize + diago.tsize/lsize)/2;  if (lsize == 0) lsize = 10;
    }
    if (lsize < 2) lsize = 2;

    waxEnd = diago.ymaxscreen * diago.xmaxscreen * waxpDepth + waxStart;

    maxFbMem = place3d(sst,waxStart,diago.maxTrashMem);

    sRegs.clipLeftRight = (0<<16) | diago.xmaxscreen;
    SET(sst->clipLeftRight, sRegs.clipLeftRight );
    sRegs.clipBottomTop = (0<<16) | diago.ymaxscreen;
    SET(sst->clipBottomTop,sRegs.clipLeftRight );
    

    while (DIAG_STARTPASS())	{		// for each pass
      pass++;
      ttp = iRandom(TEST_3D);
      switch(ttp) {
      case TEST_WAX:
	for (ii=0;ii<iRandom(7);ii++) {
	  if (bigSize && (iRandom(4) == 1))
	    rect(sst,lsize*16,pass);
	  else
	    rect(sst,lsize,pass);
	  
#if 0
	  if (iRandom(1)) 
	    line(sst,lsize,pass);
	  else
	    h32sblt(sst,lsize,pass);
#endif
	}
	break;
      case TEST_3D:
	pt = NULL;
	for (ii=0;ii<3;ii++) {
	    pt = &tri[ii];
	    ntriangle++;
	    pt->tex.tMode = SST_TPERSP_ST | 
			SST_RGB565 | SST_TC_REPLACE | SST_TCA_REPLACE;
	    texRandomTextureMap(sst,diago.trex,0, 3,3,&pt->tex);
	    fbzCP =  SST_RGBSEL_TREXOUT | SST_ENTEXTUREMAP | SST_PARMADJUST;
	    SET(sst->fbzColorPath,fbzCP);
	    SET(sst->fbzMode, SST_RGBWRMASK | drawbufferRandom() | 
		(texRandomZAbuffer(sst,pt->tex.tMode) ? SST_ENALPHABUFFER : 0));
	again:
	    randomTriangle(pt,lsize,1);	// pick random triangle
	    randomSt1418Triangle(pt);		// with random texcoords
	    //	printStwTriangle(4,pt);
	    if (diago.perspective)			// if testing perspective
	      randomW230Triangle(pt);		// generate random W

	    areaTriangle(pt);			// compute the area (before setup)
	    if (!setupStwTriangle(pt))		// setup STW slopes
	      goto again;				// reject bad slopes
	    sortTriangle(pt);			// sort it
	    printStwTriangle(4,pt);
	    printStwTriangleSlopes(5,pt);
	    
	    if (randomTexturing)
	      if (iRandom(1)) {
		fbzCP ^= SST_ENTEXTUREMAP;		// toggle texturing
		if ((fbzCP & SST_ENTEXTUREMAP)==0) {
		  fbzCP &= ~SST_RGBSELECT;
		  fbzCP |= SST_RGBSEL_C1;
		}
		else {
		  fbzCP &= ~SST_RGBSELECT;
		  fbzCP |= SST_RGBSEL_TREXOUT;
		}
		SET(sst->fbzColorPath, fbzCP);
	      }
	    // NOTE: sub-pixel parameter adjustment is OFF so we may end up with
	    //	 color overflows/underflows etc

	    if (pass >= debugPass) {
	      drawStwTriangle(sst,pt);
#if 0
	      if (diago.checkEveryTriangle) {
		sst_idle(sst);				// wait for the command to complete
		if (fbzCP & SST_ENTEXTUREMAP)
		  checkTriangle(pt,1,1,diago.adjust, insideStTriangle,0,0,0);
		eraseTriangle(sst,pt,1,0,0);		// erase the triangle
	      }
#endif
	    }
	}
	for (ii=0;ii<3;ii++) {
	  int xmin,ymin, w,h;
	  pt = &tri[ii];	
	  // s2s
	  xmin = pt->vA.x;			
	  if (pt->vB.x < xmin) xmin = pt->vB.x;
	  if (pt->vC.x < xmin) xmin = pt->vC.x;
	  xmin >>= SST_XY_FRACBITS;
	  w = pt->vA.x;
	  if (pt->vB.x > w) w = pt->vB.x;
	  if (pt->vC.x > w) w = pt->vC.x;
	  w >>= SST_XY_FRACBITS;
	  w = w - xmin + 1;
	  
	  ymin = pt->vA.y>>SST_XY_FRACBITS;
	  h = (pt->vC.y>>SST_XY_FRACBITS) - ymin + 1;
	  my_do_blt(sstg,
		    xmin,ymin,
		    xmin+iRandom(lsize),ymin+iRandom(lsize),
		    w,h,
		    SSTG_ROP_XOR << SSTG_ROP0_SHIFT);
	  if (diago.checkEveryTriangle) {
	    sst_idle_really(sst);				// wait for the command to complete
	    DIAG_DIFFMEMORY();
	  }
	}
	if (iRandom(5) == 3 &&  pass > 5) {
	  int size;
	  int hsim;
	  FxU32 addr,data,cdata;
	  FxU32 start;
	  size = rRandom(8,256);
	  start = sstFbMemRalloc(size) & ~0x3;
	  sst_idle_really(sst);
	  gdbg_printf("Test Idle...\n");
	  for (addr = SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET + start;
	       addr < SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET + start+size;
	       addr += 4) {
	    // ??? does GET compare csim and hsim ???
	    data = GET(*(FxU32 *)addr);
	    hsim = diago.halInfo->hsim;
	    diago.halInfo->hsim = 0;
	    cdata = GET(*(FxU32 *)addr);
	    diago.halInfo->hsim = hsim;
	    if (data != cdata) {
	      GDBG_ERROR(MODNAME,"csim/hsim mismatch at 0x%08x --> 0x%08x:0x%08x\n",
			 addr,cdata,data);
	      DIAG_FAIL();
	    }
	  }
	  DIAG_DIFFMEMORY();
	}
	break;
      } // case (ttp)
    } // startpass
    DIAG_PASS(0);	 
}


