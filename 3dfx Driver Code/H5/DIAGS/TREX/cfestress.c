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
** $Date: 10/11/00 8:18:54 PM$
** NYI 
** . tiled texture, YUV , LFB, colorbufs
** . byte aligned texture    
** . dst/src stride/width 14:0
** . 3DLFB auto checking
** . src memory shadow for non AGP 
*/

#include "allocate.h"
#include "udiag.h"
#include "sstdiag.h"
#include "../trex/stwtri.h"
#include "fbi.h"
#include "lfbutils.h"
#include "vidutils.h"

#define MODNAME "cfestress"
#ifdef ABS
#undef ABS
#endif
#define ABS(a) ((a) > 0 ? (a) : -(a))
#define GLEVEL 125
#define GWARN 2
#define SANITY 150
#define MAXLOOP 500

static char *space_str[] = {"LFB","YUV","3DLFB","TEX"};
static FxU32 gbpp[] = {
  SSTG_PIXFMT_1BPP, SSTG_PIXFMT_8BPP, SSTG_PIXFMT_15BPP, SSTG_PIXFMT_16BPP,
  SSTG_PIXFMT_24BPP, SSTG_PIXFMT_32BPP, SSTG_PIXFMT_422YUV
};

// various shadow state
static SstRegs sRegs;
static SstGRegs sGRegs;
static int pciDisableRetries;

typedef struct vidStuff {
  FxU32 width,stride;
  FxU32 height;
  FxU32 olyWidth, olyXOffset, olyXskew;
  FxU32 olyHeight, olyYOffset, olyYskew;
  FxU32 pixmode;
  FxU32 offset;
  FxU32 cursorx,cursory;
} VIDSTUFF;

VIDSTUFF vids;

// options
static int selectType;
static int selectInverse;
static int randomConfig;
static int waxStream;
static int dstFormat;
static int enableHblt,selectHblt;
static int enableSblt,selectSblt;
static int enableVideo,videoSelect,videoSize;
static int pType;
static int selectpType;
static int bigSize = 0;
static int debugPass = -1;
static int sanityPass = -1;
static int frameCount = 1;

static int wordTexAlign = 0;
// misc
static long nlfb,nyuv,n3dlfb,ntex,nrect,nline,ntriangle;
static long nplfb,npyuv,np3dlfb,nptex,nprect,npline,nptriangle;
static int waxpDepth = 4;
#define RSIZE 107
static FxU32 randoms[RSIZE];

static void agpRandomMem(SstRegs *sst, FxU32 *mem, int sizeBytes)
{
    int ii;
    unsigned int saveseed = getSeed();
    FxU32 col;
    FxU32 *pmem;
    unsigned rr;
    int randIndex = 0;
    int size;
    
    setSeed(999);		// always generate the same screen

    size = (sizeBytes+3)/4;
    gdbg_info( 2, "agpRandomMem: Initializing AGP 0x%x %d to random colors ... \n",mem,sizeBytes );

    rr = 0;
    // initialize the pool of random frame buffer values
    for (ii = 0; ii < RSIZE; ii++)
	randoms[ii] = iRandom(0xffffffff);

    for (pmem=mem, ii = 0; ii < size; ii++,pmem++ ) {
	col = randoms[randIndex] ^ rr;
	randIndex += 1;
	if (randIndex >= RSIZE) {
	  rr = iRandom(0xffffffff);
	  randIndex = 0;
	}
	
	agpWriteMem32(pmem,col);
    }
    setSeed(saveseed);
}


#define YUVSTRIDE 1024
#define YUVSELECT 0x300000

#define MAX_DST_STRIDE 0xFFF
typedef struct agpMoveCmd {
  FxU32 sizeBytes;
  FxU32 baseLow;
  FxU32 baseHigh;
  FxU32 srcWidth;
  FxU32 srcStride;
  FxU32 fbOffset;
  FxU32 dstStride;
  FxU32 space;
  FxU32 id;
} AGPMOVECMD;

static int mungeSize(SstRegs *sst,int sizeBytes)
{
  CsimPrivate *cpriv;
  cpriv = CSIM_PRIVATE(diago.sstCSIM);
  if ((unsigned int)sizeBytes*32 > cpriv->info->agpSizeInBytes) {
    gdbg_info(1,"Setting maximum buffer size to maximum agp memory %d bytes\n",
	      cpriv->info->agpSizeInBytes);
    return(cpriv->info->agpSizeInBytes/2); // save some for cmd fifo
  }
  if (sizeBytes*32 < 4096)
    return(4096);
  else
    return (sizeBytes*32);
}

static FxU32 agpMemSizeBytes(SstRegs *sst)
{
  CsimPrivate *cpriv;
  cpriv = CSIM_PRIVATE(diago.sstCSIM);
  return(cpriv->info->agpSizeInBytes);
}
static void start3dlfb(SstRegs *sst)
{
  FxU32 fbzColorPath;
  // pass thru
  fbzColorPath = SST_RGBSEL_LFB | 
    SST_ASEL_LFB |
    SST_CC_MULT |
    SST_CCA_MULT;
       
  SET( sst->fbzColorPath,fbzColorPath );
}
static void restoreTri(SstRegs *sst)
{
  SET(sst->fbzColorPath, SST_RGBSEL_RGBA | (diago.adjust?SST_PARMADJUST:0));
}

static FxU32 lfbSize(SstRegs *sst)
{
  int size;
  size = 2;
  switch( sst->lfbMode & SST_LFB_FORMAT ) {
  case SST_LFB_565:
  case SST_LFB_555:
  case SST_LFB_1555:
  case SST_LFB_ZZ:   
    size = 2;
    break;
  case SST_LFB_888:
  case SST_LFB_8888:
  case SST_LFB_Z565:
  case SST_LFB_Z555:
  case SST_LFB_Z1555:
  case SST_LFB_Z32:
    size = 4;
    break;
  }
  return(size);
}

static void lfbAlign(FxU32 *addr) 
{ 
  ulong size;
  size = lfbSize(&sRegs);
  *addr &= ~(size-1); // 2 byte aligned
  if (*addr == 0) *addr = size;
}


/* myParseOpts
 *
 * look for "-x" options and interpret them for this test
 *
 */

static char *
Xusage()
{
    gdbg_printf("Error Xusage::\n");
    gdbg_printf("\"-xf<n>\"\tselect 0:LFB 1:YUV 2:Texture 3:3DLFB 4:Random\n");
    gdbg_printf("\"-xf-<n>\"\tInverse select 0:LFB 1:YUV 2:Texture 3:3DLFB 4:Random\n");
    gdbg_printf("\"-xd<n>\"\tDebug Pass #\n");
    gdbg_printf("\"-xr\"\tDisable retries\n");
    gdbg_printf("\"-xc<n>\"\tP5 P6\n");
    gdbg_printf("\"-xD<n>\"\t2d dst format 0:32bpp 1:24bpp 9:random\n");
    gdbg_printf("\"-xH<n>\"\tHost blts 0:32bpp 1:24bpp 9:random\n");
    gdbg_printf("\"-xS<n>\"\tS2S blts  0:32bpp 1:24bpp 9:random\n");
    gdbg_printf("\"-xV<s><n>\"\tVideo: 0:Desktop 1:Desktop/mouse 5:Special stress 9: Random feature; Second decimal digit selects size; 99 is random size and random features; \n");
    gdbg_printf("\"-xb\"\tbig sizes\n");
    gdbg_printf("\"-xw<n>\"\t0:single stream 1:direct PCI 2:cmdfifo1 wax\n");
    gdbg_printf("\"-xt<n>\"\t0:Linear Space 1:Tiled 2:Randomised memory config every pass 3:Randomised memory config every 32 passes\n");
    gdbg_printf("\"-xC\"\tForce check every primitive\n");
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
    int tmp;
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
	    case 'H':
	      enableHblt = 1;
	      sscanf(XGETARG(), "%i", &selectHblt);
	      GDBG_PRINTF("INFO: HBlt select %d\n",selectHblt);
	      break;
	    case 'D':
	      sscanf(XGETARG(), "%i", &tmp);
	      switch(tmp) {
	      case 0:  dstFormat = SSTG_PIXFMT_32BPP>>SSTG_DST_FORMAT_SHIFT; break;
	      case 1:  dstFormat = SSTG_PIXFMT_24BPP>>SSTG_DST_FORMAT_SHIFT; break;
	      case 9:  dstFormat = (iRandom(0xFFFFFFFF) & SSTG_DST_FORMAT) >> SSTG_DST_FORMAT_SHIFT; break;
	      }
	      diago.dstFormat = dstFormat;
	      GDBG_PRINTF("INFO: 2d dst format %d\n",dstFormat);
	      break;
	    case 'S':
	      enableSblt = 1;
	      sscanf(XGETARG(), "%i", &selectSblt);
	      GDBG_PRINTF("INFO: SBlt select %d\n",selectSblt);
	      break;
	    case 'V':
	      enableVideo = 1;
	      sscanf(XGETARG(), "%i", &videoSelect);
	      videoSize = 0;
	      if (videoSelect >= 10) {
		videoSize = videoSelect / 10;
		videoSelect = videoSelect - videoSize*10;
	      }
	      GDBG_PRINTF("INFO: video select %d video size %d\n",videoSelect,videoSize);
	      done = 1;
	      break;
	    case 'b':
	      bigSize = diago.tsize * 32;
	      if (bigSize > 3000)
		bigSize = 3000;
	      done = 1;
	      break;
	    case 'C':
	      diago.checkEveryTriangle = 1;
	      done = 1;
	      break;
	    case 'w':
	      // %i ??
	      sscanf(XGETARG(), "%i", &waxStream);
	      GDBG_PRINTF("INFO: 2d select %d\n",waxStream);
	      done = 1;
	      break; 
	    case 'd':
	      // %i ??
	      sscanf(XGETARG(), "%i", &debugPass);
	      GDBG_PRINTF("INFO: debugPass %d\n",debugPass);
	      done = 1;
	      break; 
	    case 'r':
	      // %i ??
	      pciDisableRetries = 1;
	      GDBG_PRINTF("INFO: disable retries %d\n",pciDisableRetries);
	      done = 1;
	      break; 
	    case 'f':
	      if (opts[1] == '-') {
		selectInverse = 1;
		selectType = opts[2];
		opts++;
		GDBG_PRINTF("INFO: inverse selecting type %c\n",selectType);
	      }
	      else {
		selectInverse = 0;
		selectType = opts[1];
		GDBG_PRINTF("INFO: selecting type %c\n",selectType);
	      }
	      if (selectType != '0' && selectType != '1' && selectType != '2' && selectType != '3')
		Xusage();
	      opts++;
	      break;
	    case 't':
	      randomConfig = opts[1];
	      GDBG_PRINTF("INFO: selecting type %c\n",randomConfig);
	      done = 1;
	      break;
	    case 'c':
	      pType = opts[1];
	      selectpType = 1;
	      GDBG_PRINTF("INFO: selecting packet type %c\n",pType);
	      opts++;
	      break;
	    default:
	      Xusage();
	    }
	    opts += 1;
	}
    }
}

// scarved from hblt.c
static FxU32 screen[MAXSCREEN][MAXSCREEN];
static FxU32 host_pixels[MAXSCREEN*MAXSCREEN];

static int mrRandom(int s,int e)
{
  if (e > s)
    return(rRandom(s,e));
  else
    return(s);

}
// host blt simple ROP 32bpp
void h2sblt(SstRegs *sst,int size,int pass) 
{
  long xs,ys,xd,yd,stride;
  FxU32 cmdXops,cmdops,cfore,cback,cdest,rop;
  FxU32 srcFormat;
  FxU32 byte,addr,nextaddr;
  long ycEnd,yc,ycStart,ycInc,xc,ydir;
  long ww,hh;
  FxU32 col,pix;
  FxU32 xx,nn;
  FxU32 bit = 0;
  SstGRegs *sstg;
  sstg = SSTG_CHIP(sst);  
  if ((pass % 7) == 0) {			// init pattern every 7 times
    sstg_setpattern_random(sstg);
    sGRegs.dstBaseAddr = GET(sstg->dstBaseAddr);
    sGRegs.srcBaseAddr = GET(sstg->srcBaseAddr);
  }

  xyRandom(&xs,&ys);			// NOTE: source is on screen
  ww = mrRandom(1,size);
  hh = mrRandom(1,size);
  xyRandom(&xd,&yd);		// anywhere

  switch(selectHblt) {
  case 0:
    srcFormat = SSTG_PIXFMT_32BPP;
    stride = (iRandom(diago.xmaxscreen*4)/4 + 1) * 4;
    break;
  case 1:  
    srcFormat = SSTG_PIXFMT_24BPP;
    stride = (iRandom(diago.xmaxscreen*4)/3 + 1) * 3;
    break;
  case 9:
    //todo
    break;
  }

  srcFormat |= stride;
  sGRegs.srcFormat = srcFormat;
  SET(sstg->srcFormat, sGRegs.srcFormat);
  cmdops = 0;
  cmdXops = 0;
  cfore = 0x808080;    
  cdest = ONSCREEN(xd,yd) ? screen[yd][xd] : 0;
  rop = sstg_random_colors(sstg, cfore,&cback,cdest);
  cmdops |= SSTG_ROP_XOR << SSTG_ROP0_SHIFT;
  SET(sstg->rop, rop);		// set the rop
  SET(sstg->commandEx, cmdXops);

  cmdops |= SSTG_ROP_XOR << SSTG_ROP0_SHIFT;
  gdbg_info(2,"\n");
  gdbg_info(1,"HBLT xs,ys = %d,%d    xd,yd = %d,%d    w,h = %d,%d    pox,y = %d,%d\n",
	    xs,ys, xd,yd, ww,hh,
	    ((cmdops & SSTG_X_PATOFFSET)>>SSTG_X_PATOFFSET_SHIFT) & 7,
	    ((cmdops & SSTG_Y_PATOFFSET)>>SSTG_Y_PATOFFSET_SHIFT) & 7);
  sstg_print_stuff(cmdops, cmdXops, rop, srcFormat, cfore, cback);

  SET(sstg->command, cmdops | SSTG_HOST_BLT);

  stride = (srcFormat & SSTG_SRC_LINEAR_STRIDE) >> SSTG_SRC_STRIDE_SHIFT;
  addr = sstg_compute_blit_address(srcFormat, 0, xs, ys, ww);
  // addr =  ys*stride + xs*4;

  byte = addr & 3;
  gdbg_info(7, "hblt.exe: starting byte position: %d\n", byte);
  // should ignore everything above bit 2
  SET(sstg->srcXY, (iRandom(0xFFFFFFFF) & ~3) | byte);
  SET(sstg->dstXY,(yd<<16) | (xd & 0xFFFF));
  SET(sstg->dstSize,(hh<<16) | ww);

  ycInc = 1;
  ycEnd = yd + hh;
  ydir = cmdops & SSTG_YDIR;
  stride = srcFormat & SSTG_SRC_FORMAT;
  // assume SSTG_SRC_PACK_SRC
  switch(srcFormat & SSTG_SRC_FORMAT) {
  case SSTG_PIXFMT_24BPP:
    sendBltData0(sstg, byte, bit, xd, yd, ww,hh, ydir, srcFormat,
		 host_pixels);
    break;
  case SSTG_PIXFMT_32BPP:
    nextaddr = byte;
    nn = 0;
    // for each row of the blit
    for (yc = yd;yc < ycEnd;yc += ycInc) {
      addr = nextaddr;
      nextaddr = (addr + (stride % 4)) % 4;
      for (xx=0; xx < (unsigned int)ww; xx++) {
	col = iRandom(0xFFFFFFFF);
	if (pass >= debugPass) {
	  SET(sstg->launch[8], col);
	  if (ONSCREEN(xd+(int)xx,yc))
	    host_pixels[nn++] = col;
	}
      }
    }
    break;
  }

  if (diago.checkEveryTriangle && pass >= debugPass) {
    sstg_idle(sst);			// wait for the command to complete
    // positive y direction
    ycStart = yd - 1;
    ycEnd = yd + hh;
    ycInc = 1;
    pix = 0;
    // check the entire rectangle
    gdbg_info(1,"h2sblt: checking start\n");
    for (yc = ycStart; yc <= ycEnd; yc += ycInc)
      for (xc = xd-1; xc <= xd+ww; xc++)	// with a 1 pixel border
	if (ONSCREEN(xc,yc)) {
	  cdest = screen[yc][xc];		// compute predicted result
	  if (xc < xd || xc >= xd+ww ||
	      ((ycInc == 1) && ((yc < yd) || (yc >= yd+hh))) ||
	      ((ycInc == -1) && ((yc > yd) || (yc <= yd-hh))))
	    {
	      // if outside rect, then unchanged
	      cdest = sstg_destination_mask(cdest);
	      gdbg_info(199,"h2sblt: checking %d,%d 0x%x (outside)\n",xc,yc,cdest);
	      DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,xc,yc,cdest); // test the pixel
	    }
	  else {
	    FxU32 csrc = host_pixels[pix++];
	    int srcKey = sstg_src_colorkey(csrc);
		    
	    GDBG_INFO(10,"converting color 0x%x\n",csrc);
	    // NOTE: call into csim to convert colors
	    csrc = csimColorConvert(&CSIM_PRIVATE(diago.sstCSIM)->gui, csrc);
	    
	    cdest = sstg_check_pixel(xc,yc,cmdops,cmdXops, rop,csrc,cdest,srcKey);
	    screen[yc][xc] = cdest;	// update our shadow screen
	  }
	}
  }
}
#if 0
// host blt simple ROP 32bpp
void s2sblt(SstRegs *sst,int size,int pass) 
{
  int j,n,p, monotrans;
  long xs,ys, sw,sh, xd,yd, dw,dh, xc,yc, w, h;
  FxU32 rop, srcBase, srcFormat, dstFormat, destBA, cmdops,cmdXops;
  FxU32 cfore,cback,cdest;
  int dst_is_tiled = 0;
  int src_is_tiled = 0;
  FxU32 srcFormat;
  int pack;

  SstGRegs *sstg;
  sstg = SSTG_CHIP(sst);  

  switch(selectHblt) {
  case 0:
    srcFormat = SSTG_PIXFMT_32BPP;
    stride = (iRandom(diago.xmaxscreen*4)/4 + 1) * 4;
    break;
  case 1:  
    srcFormat = SSTG_PIXFMT_24BPP;
    stride = (iRandom(diago.xmaxscreen*4)/3 + 1) * 3;
    break;
  case 9:
    //todo
    break;
  }
  SET(sstg->srcFormat, srcFormat);
  // choose a source x, y, and a width and height
  //
  sstg_random_src_rect(CSIM_BUF_2D_STRETCH_SRC, 
		       srcFormat, sGRegs.srcBaseAddr, &xs, &ys, &sw, &sh);
  pack = (srcFormat & SSTG_SRC_PACK) >> SSTG_SRC_PACK_SHIFT;
  if (((xs != 0) || (ys != 0)) && (pack > 0)) {
      xs = 0;
      ys = 0;
  }
  xyRandom(&xd,&yd);			// generate random destination
  sstg_random_command_bits(&cmdops,&cmdXops);
  if ((SRCFORMAT == SSTG_PIXFMT_1BPP) ||
      (SRCFORMAT == SSTG_PIXFMT_422UYV) || 
      (SRCFORMAT == SSTG_PIXFMT_422YUV))
    cmdXops &= ~SSTG_EN_SRC_COLORKEY_EX;	// disable src colorkey

  cfore = 0x808080;    
  SET(sstg->colorFore, cfore);
  rop = sstg_random_colors(sstg, cfore,&cback,
			   sstg_get_screen_color(xd, yd, screen));
  if (diago.rectangular) {		// if random rops
    cmdops |= iRandom(0xFF) << SSTG_ROP0_SHIFT;
    rop = iRandom(0xFFFFFF);	// then pick totally random
  }
  else if (rop == 0) 			// else use SRC
    cmdops |= SSTG_ROP_SRC << SSTG_ROP0_SHIFT;
  else
    rop = SSTG_ROP_SRC << (rop-1)*8;
  
  if (iRandom(1))
    dw = mrRandom(sw, 5*sw);		// stretch X
  else
    dw = iRandom(sw);		// decimate X	
  if (iRandom(1))	    
    dh = mrRandom(sh, 5*sh);		// stretch Y
  else
    dh = iRandom(sh);		// decimate Y
  
  gdbg_info(2,"\n");
  gdbg_info(2,"xs,ys = %d,%d  sw,sh = %d,%d  xd,yd = %d,%d  dw,dh = %d,%d",
	    xs,ys, sw,sh, xd,yd, dw,dh);
  gdbg_info_more(2, "   pox,y = %d,%d\n",
		 ((cmdops & SSTG_X_PATOFFSET)
		  >> SSTG_X_PATOFFSET_SHIFT) & 7,
		 ((cmdops & SSTG_Y_PATOFFSET)
		  >>SSTG_Y_PATOFFSET_SHIFT) & 7);
  sstg_print_stuff(cmdops, cmdXops, rop, srcFormat, cfore, cback);

  //--------------------------------------------
  SET(sstg->rop, rop);		// set the rop
  SET(sstg->commandEx, cmdXops);
  
  cmdops &= ~(SSTG_XDIR | SSTG_YDIR);	// clear out the direction bits
  sstg_draw_S_blt(sstg,xs,ys,sw,sh,  xd,yd,dw,dh, cmdops);

}

#endif
// test a line by drawing it and checking every pixel
void test_line(SstRegs *sst,long x1, long y1, long x2, long y2, FxU32 csrc,int pass)
{
  SstGRegs *sstg;
  FxU32 cmd;
  int checkNeighbors;
  sstg = SSTG_CHIP(sst);
  checkNeighbors = 0;
  gdbg_info(2,"--line = %d,%d to %d,%d color=0x%08x\n",
	    x1,y1,x2,y2,csrc);
  cmd = SSTG_LINE | SSTG_REVERSIBLE | SSTG_ROP_XOR << SSTG_ROP0_SHIFT;
  SET(sstg->colorFore, csrc);
  if (pass >= debugPass) 
    sstg_drawline(sstg, x1,y1,x2,y2, cmd);
  else 
    iRandom(1); // remain in sync
  if (pass >= debugPass) {
    if (diago.checkEveryTriangle) {
      // ?? cmd fifo mode ??
      sstg_idle(sst);			// wait for the command to complete
      sstg_checkline(sstg, x1,y1,x2,y2, cmd,0, csrc,0,0, checkNeighbors);
    }
  }
}


static void line(SstRegs *sst,int size,int pass)
{
  long x1,x2,y1,y2;
  FxU32 csrc,len;
  xyRandom(&x1,&y1);				// pick random x,y
  nline++;
  // rough measure
  npline += 2*(ABS(x2-x1) + ABS(y2-y1));
  csrc = colRandom32();
  len = mrRandom(5,size);
  x2 = mrRandom(x1-len,x1+len);
  y2 = mrRandom(y1-len,y1+len);
  test_line(sst,x1,y1,x2,y2,csrc,pass);		// draw it
  test_line(sst,x2,y2,x1,y1,0,pass); // erase - assume REVERSIBLE
}


static void rect(SstRegs *sst,unsigned size,int pass)
{
  SstGRegs *sstg;
  FxU32 cmdops,cmdXops;
  FxU32 cfore,cback,rop,cdest;
  long xx,yy,ww,hh;
  long xc,yc;
  int border;
  sstg = SSTG_CHIP(sst);
  xyRandom(&xx,&yy);
  if (iRandom(7)==0) {			// generate some negative coords
    xx -= 4*size;
    yy -= 3*size;
  }
  ww = mrRandom(1,size);		// generate random width height
  hh = mrRandom(1,size);
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

  if (diago.checkEveryTriangle && pass >= debugPass) {
    sstg_idle(sst);			// wait for the command to complete
    border = 20;
    gdbg_info(1,"rect: checking start\n");
    for (yc = yy-border; yc <= yy+hh+border; yc++)	// check the entire rectangle
      for (xc = xx-border; xc <= xx+ww+border; xc++)	// with a 1 pixel border
	if (ONSCREEN(xc,yc)) {
	  cdest = screen[yc][xc];		// compute predicted result
	  if (xc < xx || xc >= xx+ww || yc < yy || yc >= yy+hh) {
	    // if outside rect, then unchanged
	    cdest = sstg_destination_mask(cdest);
	    gdbg_info(199,"rect: checking %d,%d 0x%x (outside)\n",xc,yc,cdest);
	    DIAG_TEST_PIXEL(CSIM_BUF_2D_DST,xc,yc,cdest);// test the pixel
	  }
	  else {
	    cdest = sstg_check_pixel(xc,yc,cmdops,cmdXops, rop,cfore,cdest,0);
	    screen[yc][xc] = cdest;	// update our shadow screen
	  }
	}
    //if (DIAG_EXCEEDED_PIXEL_LIMIT())
  }
}


static unsigned place3d(SstRegs *sst)
{
  FxU32 bufferSize;

  //Unallocate the existing buffers
  unlockByName("Color Buffer #0");
  unlockByName("Color Buffer #1");
  unlockByName("Color Buffer #2");
  unlockByName("Aux Buffer");
  unallocateAll();
  
  bufferSize = diago.ymaxscreen * diago.xmaxscreen * 2;

  // base and stride must be 16-byte aligned
  diagfb.colBufferAddr[0] = (FxU32) allocate(bufferSize, "Color Buffer #0", randomPlacement);  
  diagfb.colBufferStride[0] = diago.xmaxscreen * 2 ;
  diagfb.colBufferAddr[0] &= ~0xF;
  diagfb.colBufferStride[0] &= ~0xF;

  diagfb.colBufferAddr[1] = (FxU32) allocate(bufferSize, "Color Buffer #1", randomPlacement);  
  diagfb.colBufferStride[1] = diago.xmaxscreen * 2 ;
  diagfb.colBufferAddr[1] &= ~0xF;
  diagfb.colBufferStride[1] &= ~0xF;
    
  diagfb.colBufferStride[0] |= SST_BUFFER_MEMORY_LINEAR;
  diagfb.colBufferStride[1] |= SST_BUFFER_MEMORY_LINEAR;
  
  if ( diago.hasAuxBuffer ) {
    diagfb.auxBufferAddr = (FxU32) allocate(bufferSize, "Aux Buffer", randomPlacement);  
    diagfb.auxBufferStride = diago.xmaxscreen * 2 ;
    diagfb.auxBufferAddr &= ~0xF;
    diagfb.auxBufferStride &= ~0xF;
    diagfb.auxBufferStride |= SST_BUFFER_MEMORY_LINEAR;
  } else {
    diagfb.auxBufferAddr = 0x0;
    diagfb.auxBufferStride = 0x0;
  }
  
    
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

  return(0);
}


static void disableFifo() {
  // reach into privies..
  CSIM_PRIVATE(diago.sstCSIM)->inCmdFifoExecMode = 1;
}
static void enableFifo() {
  CSIM_PRIVATE(diago.sstCSIM)->inCmdFifoExecMode = 0;
}


static int tempwf;
static void pushWax(int direct)
{
  if (direct) {
    tempwf = diago.writeFifo; diago.writeFifo = 0; 
  }
  else
  switch(waxStream) {
  case 0: break;
  case 1: tempwf = diago.writeFifo; diago.writeFifo = 0; break;
  case 2: hb_selectFifo(1); break;
  }
  if (!diago.writeFifo && tempwf)
    disableFifo();
}
static void popWax(int direct) 
{
  if (!diago.writeFifo && tempwf)
    enableFifo();
  if (direct) {
    diago.writeFifo = tempwf;
  }
  else 
  switch(waxStream) {
  case 0: break;
  case 1: diago.writeFifo = tempwf;break;
  case 2: hb_restoreFifo(); break;
  }

}

static void pushDirect() { pushWax(1); }
static void popDirect() { popWax(1); }

static void nmlfbRead(SstRegs *sst)
{
  int ii;
  FxU32 daddr,data,cksum, space;
  unsigned burst,status;

  status = GET(sst->status);
  if (!pciDisableRetries || !(status & SST_PCIFIFO_BUSY)) {
    burst = iRandom(32);

    //Find some memory
    space = allocate(burst * 4, "nmlfbRead", randomPlacement);
    unallocateByName("nmlfbRead");
    
    daddr = SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET + space & ~0x3;

    cksum = 0;
    for (ii = 0; ii< (int)burst; ii++) {
      data = GET(*(FxU32 *)daddr);
      daddr += 4;
      cksum ^= data;
    }
    gdbg_info(1,"NMLFB read %d %x\n",burst,cksum);
  }
}


static int initVideoTest(SstRegs *sst,int vSize,int vSelect,
			 FxU32 vAddr) {
  SstIORegs *sstio;
  FxU32 tmp,x,y,vcfg;
  FxU32 red,green,blue;
  int randIndex = 0;
  unsigned rr;
  unsigned int saveseed = getSeed();
  int ii;
  float scale;
  pushDirect();
  sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
  
  vcfg = 0;
  vcfg |= SST_CURSOR_X11;
  vcfg |= SST_DESKTOP_EN; 
  if (vSelect == 9) {
    vcfg |= SST_OVERLAY_EN;  
    if (iRandom(1)) {
      vcfg |= SST_VIDEO_2X_MODE_EN;
      SET(sstio->dacMode,1); // put dac in 2x mode   
    }
    if (iRandom(1)) 
      vcfg |= SST_CHROMA_EN;
    if (iRandom(1)) {
      vcfg |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
      vcfg |= SST_OVERLAY_VERT_SCALE_EN;    
    }
    vcfg |= SST_OVERLAY_FILTER_POINT; 
  }
  else if (vSelect == 5) { // special buggy mode
    vcfg |= SST_OVERLAY_EN;  
    vcfg |= SST_VIDEO_2X_MODE_EN;
    SET(sstio->dacMode,1); // put dac in 2x mode   
    vcfg |= SST_OVERLAY_FILTER_POINT; 

  }
  else if (vSelect >= 1) {
    vcfg |= SST_CHROMA_EN;
    vcfg |= SST_OVERLAY_EN;  
    vcfg |= SST_OVERLAY_HORIZ_SCALE_EN;    // bilerp in horz and vert
    vcfg |= SST_OVERLAY_VERT_SCALE_EN;    
    vcfg |= SST_OVERLAY_FILTER_POINT; 
  }
  
  // vcfg |= SST_DESKTOP_PIXEL_PAL8;
  if (vSelect == 9) {
      vcfg |= iRandom(1) ? SST_DESKTOP_PIXEL_RGB565 : SST_DESKTOP_PIXEL_RGB24;
  }
  else if (vSelect == 5) { // special buggy mode
    vcfg |= SST_DESKTOP_PIXEL_RGB24;
  }
  else {
    vcfg |= SST_DESKTOP_PIXEL_RGB24;
  }
  vcfg |= SST_DESKTOP_CLUT_BYPASS;

  vcfg |= SST_OVERLAY_PIXEL_RGB565U;
  vcfg |= SST_OVERLAY_CLUT_BYPASS;
  // vcfg |= SST_DESKTOP_TILED_EN;
  // vcfg |= SST_OVERLAY_TILED_EN; 
  if (vSelect >= 2)
    vcfg |= SST_CURSOR_EN;

  vids.cursorx = 0;
  vids.cursory = 0;
  switch(vSize) {
  case 0:
    vids.width = (81/16)*16;
    vids.height = 33;
    vids.olyWidth = 75;
    vids.olyHeight = 30;
    vids.olyXOffset = 3;
    vids.olyYOffset = 2;
  case 1:
    vids.width = (640/16)*16;
    vids.height = 10;
    vids.olyWidth = 300;

    vids.olyHeight = 7;
    vids.olyXOffset = 0;
    vids.olyYOffset = 0;
    break;
  case 2:
    vids.width = (800/16)*16;
    vids.height = 30;
    vids.olyWidth = 300;
    vids.olyHeight = 3;
    vids.olyXOffset = 0;
    vids.olyYOffset = 0;
    break;
  case 3:
    vids.width = (380/16)*16;
    vids.height = 300;
    vids.olyWidth = 375;
    vids.olyHeight = 255;
    vids.olyXOffset = 3;
    vids.olyYOffset = 30;
    break;
  case 4: // mouse starts at bottom right corner of overlay
    // Video setting w=240 h=17 ow=73 oh=7 ox=37 oy=5 cx=147 cy=7 vidProcCfg c28cd82
    // Video setup: Overlay Enabled ; ; Hor Scale Enabled ; Vert Scale Enabled; ; Cursor Enabled 2X mode
    // Desktop RGB24
	       
    vids.width = (240/16)*16;
    vids.height = 17;
    vids.olyWidth = 73;
    vids.olyHeight = 7;
    vids.olyXOffset = 37;
    vids.olyYOffset = 5;
    vids.cursorx = 147;
    vids.cursory = 7;
    break;
  case 9:
    vids.width = (240/16)*16;
    vids.height = mrRandom(10,20);
    if (vcfg & SST_VIDEO_2X_MODE_EN) {
      vids.olyWidth = mrRandom(4,80);
      vids.olyHeight = mrRandom(1,8);
    } else {
      vids.olyWidth = mrRandom(1,80);
      vids.olyHeight = mrRandom(1,8);
    }
    // if scaling...oly is too small
    vids.olyXOffset = mrRandom(0,vids.width-vids.olyWidth);
    vids.olyYOffset = mrRandom(0,vids.height-vids.olyHeight);
    vids.cursorx = mrRandom(0,vids.width+1);
    vids.cursory = mrRandom(0,vids.height+1);
    break;
  }
  gdbg_printf("Video setting w=%d h=%d ow=%d oh=%d ox=%d oy=%d cx=%d cy=%d\nvidProcCfg %x\n",
	      vids.width,  vids.height,vids.olyWidth,vids.olyHeight,
	      vids.olyXOffset, vids.olyYOffset,
	      vids.cursorx,
	      vids.cursory,vcfg);
   gdbg_printf("Video setup: %s; %s; %s; %s; %s; %s %s\n",
	       vcfg & SST_OVERLAY_EN ? "Overlay Enabled " : "",
	       vcfg & SST_CHROMA_EN ? "Chroma Enabled ": "",
	       vcfg & SST_OVERLAY_HORIZ_SCALE_EN ? "Hor Scale Enabled ":"",
	       vcfg & SST_OVERLAY_VERT_SCALE_EN ? "Vert Scale Enabled":"",
	       vcfg & SST_OVERLAY_FILTER_POINT ? "Overlay Point Filter":"",
	       vcfg & SST_CURSOR_EN ? "Cursor Enabled" : "",
	       vcfg & SST_VIDEO_2X_MODE_EN ? "2X mode" : "");
   switch(vcfg & SST_DESKTOP_PIXEL_FORMAT) {
   case SST_DESKTOP_PIXEL_RGB565: gdbg_printf("Video: Desktop RGB565\n"); break;
   case SST_DESKTOP_PIXEL_RGB24: gdbg_printf("Video: Desktop RGB24\n"); break;
   }
  SET(sstio->vidProcCfg,vcfg);   
  vids.pixmode = vcfg;
  
  // cursor register setup
  SET(sstio->hwCurLoc, (vids.cursorx <<SST_CURSOR_X_SHIFT) 
      | (vids.cursory << SST_CURSOR_Y_SHIFT)); 
  SET(sstio->hwCurC0, 0x123456); 
  SET(sstio->hwCurC1, 0xedcba9);
  
  // video scaling attributes
  tmp = 0x70000; 
  if (vids.pixmode & SST_OVERLAY_HORIZ_SCALE_EN)
    scale = ((float)tmp)/(float)0x100000; // 0.20 number
  else scale = 1.f;
  SET(sstio->vidOverlayDudx,tmp);    
  tmp = 0x21000>>1; // only 19 bits, 
  tmp |= ((int)((float)vids.olyWidth * 2.0 * scale)+3)<<SST_OVERLAY_FETCH_SIZE_SHIFT;
  SET(sstio->vidOverlayDudxOffsetSrcWidth,tmp);    
  SET(sstio->vidOverlayDvdy,0xe0000);    
  SET(sstio->vidOverlayDvdyOffset,0x44000>>1);  
    
  SET(sstio->vidChromaMin, 0x000000);
  SET(sstio->vidChromaMax, 0x000077);


  // end of test specific
  // set video overlay start coords at 11, 51
  tmp = (vids.olyXOffset<<SST_OVERLAY_X_SHIFT); 
  tmp |= (vids.olyYOffset<<SST_OVERLAY_Y_SHIFT);
  vids.olyXskew = vids.olyYskew = 0;
  tmp |= (vids.olyXskew&0x3)<< SST_OVERLAY_XADJ_SHIFT;
  tmp |= (vids.olyYskew&0x3)<< SST_OVERLAY_YADJ_SHIFT;
  SET(sstio->vidOverlayStartCoords,tmp);    
  // set video overlay end coords at 101, 71
  // todo should be oly*-1
  tmp = ((vids.olyXOffset+vids.olyWidth-1)<<SST_OVERLAY_X_SHIFT); 
  tmp   |= ((vids.olyYOffset+vids.olyHeight-1)<<SST_OVERLAY_Y_SHIFT);
  SET(sstio->vidOverlayEndScreenCoord,tmp);
  
  tmp = ((vids.pixmode & SST_OVERLAY_CLUT_BYPASS)==0); 
  tmp |=((vids.pixmode & SST_DESKTOP_CLUT_BYPASS)==0);
  // test independant stuff      
  if(tmp != 0) // clut used
    {
      clutInit(sstio); //Initialize the color lookup tables
    }
  
  vids.stride = (vids.width+4)& 0xfffffff8; // word aligned
  switch(vids.pixmode& (0x7<<SST_DESKTOP_PIXEL_FORMAT_SHIFT)) {
  case(SST_DESKTOP_PIXEL_RGB565): 
    vids.stride = vids.stride*2;
  break;
  case(SST_DESKTOP_PIXEL_PAL8): 
    vids.stride= vids.stride*2;
  break;
  case(SST_DESKTOP_PIXEL_RGB24): // nothing to do.
    vids.stride = vids.stride*3;
  break;
  case(SST_DESKTOP_PIXEL_RGB32):  // nothing to do.
    vids.stride = vids.stride*4;
  break;
  }

  vids.offset = 0x1000; //0x100000
  tmp = vids.offset + vAddr;
  CSIM_PRIVATE(diago.sstCSIM)->io.vidCurrOverlayStartAddr 
    = tmp ; // one meg into it.

  gdbg_printf("videoinit: Setting overlay buffer location = 0x%x\n",tmp);

  SET(sst->leftOverlayBuf, tmp); // one meg into it.
  SET(sst->swapbufferCMD,0); // swap the buffer, so that left and write match
  
  SET(sstio->vidDesktopOverlayStride,(vids.stride<<SST_OVERLAY_STRIDE_SHIFT) | (vids.stride<<SST_DESKTOP_STRIDE_SHIFT));
  SET(sstio->vidDesktopStartAddr,vAddr);
  // do we need this ?? sst_idle_really(sst);

  tmp = (vids.width << SST_VIDEO_SCREEN_WIDTH_SHIFT ) 
    | (vids.height << SST_VIDEO_SCREEN_HEIGHT_SHIFT);
  SET(sstio->vidScreenSize,tmp);    
  SET(sstio->hwCurPatAddr
      , vids.offset + vAddr);
  
  // set max threshold. Initially very big
  tmp = 0x00100810;
  SET(sstio->vidMaxRGBDelta,tmp);


  setSeed(999);		// always generate the same screen
  // initialize the pool of random frame buffer values
  for (ii = 0; ii < RSIZE; ii++)
    randoms[ii] = iRandom(0xffffffff);

  if (vids.pixmode & SST_OVERLAY_EN) {
    // Place data in overlay buffer
    // Data is a smooth-ramping color with horizontal and vertical bars.
    for(y=0; y<(int)vids.olyHeight; y++) {
      for(x=0; x<(int)vids.olyWidth; x++) {
	tmp = ((x*y*7)>>7)& 0x1ff; 
	//if (tmp>255) tmp=255;
	tmp = (tmp<<16) | ((255-tmp)<< 8) | ((128-tmp) & 0xff);
	if((x%11==0)) tmp |= 0xffffff; // white stripes
	if((y%9==0)) tmp ^= 0xffffff; // invert data
	red = ((tmp>>16)& 0xff);
	green = ((tmp>>8)& 0xff);
	blue = ((tmp>>0)&0xff);
	// convert to current data format
	tmp = (tmp & 0xf80000)>>8 | (tmp & 0xfc00)>>5 | (tmp & 0x1f);
	SET_PIXEL(CSIM_BUF_OVERLAY, x, y, tmp);
      }
    }
  }
  // Place data in desktop buffer
  // Data is a smooth-ramping color with horizontal and vertical bars.
  for(y=0; y<(int)vids.height; y++) {
    for(x=0; x<(int)vids.width; x++) {
      tmp = ((x*y)>>7)& 0x1ff; 
      if (tmp>255) tmp=255;
      tmp = ((255-tmp)<<16) | ((x>>2)<< 8) | ((x<<3) & 0xff); 
      // lsbs follow addr.
      if(((x+y)%11==0)) tmp |= 0xffffff; // white stripes
      if(((x-y)%9==0)) tmp ^= 0xffffff; // invert data

      // convert to current data format
      switch(vids.pixmode& (0x7<<SST_DESKTOP_PIXEL_FORMAT_SHIFT))
	{
	case SST_DESKTOP_PIXEL_RGB565: 
	  tmp = (tmp & 0xf80000)>>8 | (tmp & 0xfc00)>>5 | (tmp & 0x1f);
	  break;
	case SST_DESKTOP_PIXEL_PAL8: 
	  tmp = tmp & 0xff; // one color;
	  break;
	case SST_DESKTOP_PIXEL_RGB24: // nothing to do. 
	  break;
	case SST_DESKTOP_PIXEL_RGB32:  // nothing to do.
	  break;
	}
      randIndex += 1;
      rr = 0;
      if (randIndex >= RSIZE) {
	rr = iRandom(0xffffffff);
	randIndex = 0;
      }
      tmp ^= randoms[randIndex];
      tmp ^= rr;
      SET_PIXEL(CSIM_BUF_DESKTOP, x, y, tmp);
    }
  }
  setSeed(saveseed);
  
  if (vids.pixmode & SST_CURSOR_EN) {
    // Setup cursor
    for(y=0; y<64; y++)  {
      for(x=0; x<64; x++) {
	tmp = (((x>>5) & 1) ^ ((x>>4) & 1) ^ ((x>>3) & 1)
	       ^ ((x>>2) & 1) ^ ((x>>1) & 1) ^ ((x>>0) & 1)
	       ^ ((y>>5) & 1) ^ ((y>>4) & 1) ^ ((y>>3) & 1)
	       ^ ((y>>2) & 1) ^ ((y>>1) & 1) ^ ((y>>0) & 1));
	tmp |= (((x>>5) & 1) ^ ((x>>4) & 1) ^ ((x>>3) & 1)
		^ ((x>>2) & 1) ^ ((x>>1) & 1) ^ ((x>>0) & 1)
		| ((y>>5) & 1) ^ ((y>>4) & 1) ^ ((y>>3) & 1)
		^ ((y>>2) & 1) ^ ((y>>1) & 1) ^ ((y>>0) & 1))<<1;
	SET_PIXEL(CSIM_BUF_CURSOR, x, y, tmp);
	
      }
    }
  }
  gdbg_printf("vidInit...w 0x%x(%d) h 0x%x(%d).\n",vids.width,vids.width,vids.height,vids.height);
  if (vids.pixmode & SST_VIDEO_2X_MODE_EN) 
    vidInit(vids.width/2, vids.height,sst);
  else
    vidInit(vids.width, vids.height,sst);
  gdbg_printf("vidOut....\n");
  vidOut(); // render the display image
  gdbg_printf("vidCleanup....\n");

  // turn on video 
  vids.pixmode |= SST_VIDEO_PROCESSOR_EN;
  SET(sstio->vidProcCfg,vids.pixmode);   

  // vidCleanup(sst);		// wait for the command to complete
  gdbg_printf("Done video test....\n");
  popDirect();

  return(1);
}

static void stressInit(SstIORegs *sstio,int noDisableRetries)
{
  FxU32 ttemp;
  pushDirect();
  ttemp = GET(sstio->pciInit0);
  if (iRandom(1) == 1
    && !noDisableRetries) {
    ttemp |= (SST_PCI_DISABLE_IO|SST_PCI_DISABLE_MEM);
    gdbg_printf("PCI Disable Retries\n");
  }
  else {
    ttemp &= ~(SST_PCI_DISABLE_IO|SST_PCI_DISABLE_MEM);
    gdbg_printf("PCI Enable Retries\n");
  }
  SET(sstio->pciInit0,ttemp);
  
  ttemp = GET(sstio->dramInit1);
  if (ttemp & SST_VIDEO_OVERRIDE_EN)
    gdbg_printf("DRAM video override enabled \n");
  else 
    gdbg_printf("DRAM video override disabled\n");
  
  if (enableVideo) {
    ttemp = GET(sstio->vgaInit0);
#define DISABLE_VGA_MEMORY_ACCESS BIT(12)
#define DISABLE_VGA_SGRAM_REFRESH BIT(22)
    ttemp |= DISABLE_VGA_MEMORY_ACCESS;
    ttemp |= DISABLE_VGA_SGRAM_REFRESH;
    gdbg_printf("VGA SGRAM memory accesses off\n");
    
    SET(sstio->vgaInit0,ttemp);
  }
  popDirect();
}

static void maskCmd(AGPMOVECMD *cmdp)
{
  cmdp->fbOffset &= SST_AGP_FRAME_BUFFER_OFFSET;	
  cmdp->baseLow &= SST_AGP_MOVE_BASELOW;
  cmdp->srcWidth &= SST_AGP_SRC_WIDTH;
  cmdp->baseHigh  = ((cmdp->baseHigh << SST_AGP_SRC_BASEHIGH_SHIFT) & SST_AGP_SRC_BASEHIGH)
				     >> SST_AGP_SRC_BASEHIGH_SHIFT;
  cmdp->srcStride  = ((cmdp->srcStride<<SST_AGP_SRC_STRIDE_SHIFT) & SST_AGP_SRC_STRIDE) 
				      >> SST_AGP_SRC_STRIDE_SHIFT;
}

void
main(int argc, char **argv)
{
    int ii;
    int pp;
    SstRegs *sst;
    SstCRegs *sstc;
    SstIORegs *sstio;
    int pBurst;
    // texture
    Triangle *pt;
    FxU32 texBaseAddr,shadowTexBase;
    int newTexBase;
    Triangle *tt;
    unsigned texSize;

    // 3d lfb
    FxU32 xx,yy;
    FxU32 pixelsToWrite;

    // yuv
    ulong newYuvBase;
    FxU32 yuvBaseAddr;
    FxU32 maxYuvWidth;
    ulong selectY;
    int xOff,yOff;
    FxU32 planarOff;

    FxU32 *agpMem;
    ulong size,totalSize;
    ulong dstOffset;
    FxU32 srcAddr;
    ulong srcOffset;
    int pass = 0;
    int loop1;
    int loop2;
    //p5
    int ww;
    int nWords,seats,cWords,remWords; 
    FxU32 p5Addr;
    FxU32 srcAddrHigh,srcAddrLow;
    FxU32 prevLow,prevHigh;
    FxU32 srcData,dstAddr,prevDstAddr,dstData;

    // 2d hblt
    SstGRegs *sstg;

    // 
    int lsize;

    AGPMOVECMD cmd;
    AGPMOVECMD *cmdp;
 
    FxU32 memConfig;
    FxU32 tiledOffsetBytes;
    FxU32 psize;
    cmdp = &cmd;

    diago.gui = 1;				// flag as 2D app
    dstFormat = SSTG_PIXFMT_32BPP>>SSTG_DST_FORMAT_SHIFT;
    diago.dstFormat = dstFormat;

    sst = SST_BEGIN(argc,argv);
    sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);
    sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
    sstg = SSTG_CHIP(sst);

    if(diago.bigAssTextures)
      tt = buildTriangle(2048, 2048);	
    else
      tt = buildTriangle(256, 256);

    // Options
    nlfb = nyuv= n3dlfb= ntex= nrect= nline= ntriangle = 0;
    nplfb = npyuv= np3dlfb= nptex= nprect= npline= nptriangle = 0;
    selectInverse = 0;
    selectType = '4';
    randomConfig = '0';
    pType = '5';
    selectpType = 0;
    waxStream = 0;
    enableHblt = 0;
    enableVideo = 0;
    pciDisableRetries = 0;

    diago.checkEveryTriangle = 0;
    XParseOpts(argc,argv);
    if (pType != '6')
      if (!diago.writeFifo) {
	GDBG_ERROR(MODNAME,"Packet 5 runs only in CMDFIFO mode\n");
	DIAG_FAIL();
      }
    if (diago.ytiled != 0) {
      gdbg_error(MODNAME, "Tiled NYI\n");
      DIAG_FAIL();
    }

    stressInit(sstio,!((waxStream==1) && pciDisableRetries));

    switch(waxStream) {
    case 0: 
      break;
    case 1: 
      break;
    case 2: 
      if (diago.whichFifo < 2) {
	GDBG_ERROR(MODNAME,"Both CMDFifos need to be activated\n");
	DIAG_FAIL();
      }
      break;
    }
    // SET(sstio->lfbMemoryConfig,0xa3FFF); 
    if (randomConfig != '0') {
      FxU32 tiledSpaceBeginPage;

      tiledSpaceBeginPage = mrRandom(16, 0x7FFF);
      
      memConfig = (SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE(tiledSpaceBeginPage)) |
	(iRandom(SST_RAW_LFB_ADDR_STRIDE_MAX) << SST_RAW_LFB_ADDR_STRIDE_SHIFT) & SST_RAW_LFB_ADDR_STRIDE |
	(mrRandom(1,0x7F) << SST_RAW_LFB_TILE_STRIDE_SHIFT) & SST_RAW_LFB_TILE_STRIDE;
      SET(sstio->lfbMemoryConfig,memConfig);
      GDBG_INFO(2,"LFB Memory Config 0x%08x\n",memConfig);
    } else {
      // all linear
      memConfig = GET(sstio->lfbMemoryConfig); //hack
      if (memConfig == 0)  { 
	memConfig = 0xa3FFF;
	// tiled space ??
	SET(sstio->lfbMemoryConfig,memConfig);
	GDBG_INFO(2,"LFB Memory Config 0x%08x\n",memConfig);
      }
    }
    GDBG_INFO(1,"LFB Memory Config 0x%08x\n",memConfig);

    //This unallocates everything and reallocates the 3d buffers
    place3d(sst);

    // 2d setup
    pushWax(1);
    if (diago.dstFormat < (SSTG_PIXFMT_8BPP>>SSTG_SRC_FORMAT_SHIFT) ||
	diago.dstFormat > (SSTG_PIXFMT_32BPP>>SSTG_SRC_FORMAT_SHIFT)) {
	GDBG_ERROR(MODNAME, "invalid destination format\n");
	DIAG_FAIL();
    }

    {
      FxU32 allocatedAddress;
      
      allocatedAddress = allocate(diago.ymaxscreen * diago.xmaxscreen * 4, "Wax Shit", randomPlacement);
      SET(sstg->dstBaseAddr,allocatedAddress);		// define a default surface
      SET(sstg->srcBaseAddr,allocatedAddress);		// Default 
    }
    waxpDepth = 4; // allocate enough for max
    SET(sstg->dstFormat,gbpp[diago.dstFormat] | diago.xmaxscreen*4);
    SET(sstg->srcFormat,gbpp[diago.dstFormat] | diago.xmaxscreen*4);
    sGRegs.clip0min = 0;
    SET(sstg->clip0min, 0);
    sGRegs.clip0max = (diago.ymaxscreen<<16) | diago.xmaxscreen;
    SET(sstg->clip0max,sGRegs.clip0max);
    sGRegs.dstBaseAddr = GET(sstg->dstBaseAddr);
    sGRegs.srcBaseAddr = GET(sstg->srcBaseAddr);
    // sGRegs.clip0min = GET(sstg->clip0min);
    // sGRegs.clip0max = GET(sstg->clip0max);
    sGRegs.colorBack = 0; // line
    SET(sstg->colorBack, sGRegs.colorBack);
    popWax(1);
      
    tiledOffsetBytes = (memConfig & SST_RAW_LFB_TILE_BEGIN_PAGE) >> SST_RAW_LFB_TILE_BEGIN_PAGE_SHIFT;
    size = diago.tsize;
    if (diago.writeFifo) {
      // blow out cmd fifo
      if (diago.ringSize < 0) 
	psize =  -diago.ringSize;
      else 
	psize = diago.ringSize;
    }
    else {
      if (selectpType ) {
	if (pType == '5') 
	  GDBG_ERROR("move","packet 5 runs only in CMD fifo mode\n");
	  }
    }


    psize -= 5; // some breathing room
    totalSize =  mungeSize(sst,size);
    lsize = (diago.tsize/4 + 1);  if (lsize == 0) lsize = 10;
    for (ii = 10;ii>0;ii--) {
      lsize = (lsize + diago.tsize/lsize)/2;  if (lsize == 0) lsize = 10;
    }
    if (lsize < 2) lsize = 2;

    agpMem = (FxU32 *)AGPMEMALLOC(sst,totalSize);
    if (agpMem == NULL) DIAG_FAIL();
    agpRandomMem(sst,agpMem,totalSize);

    // set up texture download
    tt->tex->tMode = texRandomFormat(iRandom(1),iRandom(1),iRandom(1)) 
	 | SST_TC_REPLACE | SST_TCA_REPLACE;
    SET(sst->textureMode,tt->tex->tMode);	// set texture mode just for fun
    texSize =  (256*256*5) & ~0xF;    
    texBaseAddr = allocate(texSize, "texture", randomPlacement);
    SET(sst->texBaseAddr,texBaseAddr&SST_TEXTURE_ADDRESS);
    GDBG_INFO(0,"Tex Base Addr 0x%08x\n",texBaseAddr);
    shadowTexBase = texBaseAddr;
    sRegs.texBaseAddr = texBaseAddr;
    tt->next = NULL;


    // Generate Random LFB Write Parameters
    sRegs.lfbMode = rndlfbMode( iRandom(NUM_TEST_OPTS-1), &pixelsToWrite );
    SET( sst->lfbMode, sRegs.lfbMode );
    sRegs.clipLeftRight = (0<<16) | diago.xmaxscreen;
    SET(sst->clipLeftRight, sRegs.clipLeftRight );
    sRegs.clipBottomTop = (0<<16) | diago.ymaxscreen;
    SET(sst->clipBottomTop,sRegs.clipLeftRight );
    
    // yuv
    yuvBaseAddr = allocate(diago.ymaxscreen * diago.xmaxscreen * 2, "yuv", randomPlacement);
    yuvBaseAddr &= ~0xF;			          
    SET(sstc->yuvBaseAddr,yuvBaseAddr  & SST_YUV_BASE_ADDR); 
    GDBG_INFO(0,"Yuv Base Addr 0x%08x\n",yuvBaseAddr);
   // linear
    maxYuvWidth = 512;
    SET(sstc->yuvStride,SST_YUV_MEMORY_LINEAR | maxYuvWidth);

    memoryMap();

    if (enableHblt) {
      pushWax(0);
      h2sblt(sst,lsize,pass);
      popWax(0);
    }

    if (enableVideo) {
      frameCount = initVideoTest(sst,videoSize,videoSelect,diagfb.colBufferAddr[1]);
    } else {  
      // Borrowed from sstdiag.c. Set up HW video 
#if !defined(CVG) && !defined(SST2)
    if ( diago.halInfo->hw) {
      // setup video for 3d diags
      int tiled = (diagfb.colBufferStride[0] & SST_BUFFER_MEMORY_TILED) ? 1 : 0; 
      int tstride = (diagfb.colBufferStride[0]&SST_BUFFER_TILE_STRIDE)>>SST_BUFFER_STRIDE_SHIFT;
      int stride = (diagfb.colBufferStride[0]&SST_BUFFER_LINEAR_STRIDE)>>SST_BUFFER_STRIDE_SHIFT;
      int i;
      gdbg_printf("init video overlay !!\n");
      sstInitVideoOverlay(sst,
			  1,				// 1=enable Overlay surface (OS), 1=disable
			  0,				// 1=enable OS stereo, 0=disable
			  0,				// 1=enable horizontal scaling, 0=disable
			  0,				// horizontal scale factor (ignored if not scaling)
			  0,				// 1=enable vertical scaling, 0=disable
			  0,				// vertical scale factor (ignored if not scaling)
			  0,				// filter mode
			  tiled,				// 0=OS linear, 1=tiled
			  SST_OVERLAY_PIXEL_RGB565D,	// pixel format of OS
			  0,				// bypass clut for OS?
			  0,				// 0=lower 256 CLUT entries, 1=upper 256
			  diagfb.colBufferAddr[0],	// board address of beginning of OS
			  tiled ? tstride : stride,	// distance between scanlines of the OS
			  diago.xmaxscreen*2);          // overlay width in bytes
      
      // swap thru all active buffers to load vidCurrOverlayStartAddr for initial buffer
      i = diago.saveBeforeSwap;             // save saveBeforeSwap state
      diago.saveBeforeSwap = 0;             // force 0 to skip saving of images/memory
	diago.gui = 0; // hack to work around GUI_CHECK crap in sstdiag.c
      DIAG_SWAPBUFFER_EX(1,0,0);            // swap
      DIAG_SWAPBUFFER_EX(1,0,0);      
      if (diago.triple)
	DIAG_SWAPBUFFER_EX(1,0,0);      
      // diagSwaps = 0;                        // reset swap counter
      diago.gui = 1;
      diago.saveBeforeSwap = i;             // restore saveBeforeSwap state
    }
#endif
    }

    
    while (DIAG_STARTPASS())	{		// for each pass
      int iii;
      iii = 10;
      while (iii-- > 0) {
      pass++;

      if (iRandom(2) == 1)
	nmlfbRead(sst);
      if (pass == sanityPass)cmdp->space = SSTCP_LFB_SPACE;  
      else {
      again_more:
	pBurst = iRandom(4);
	if (selectInverse) {
	  switch(selectType) {
	  case '0':  do { cmdp->space = iRandom(3); } while (cmdp->space == SSTCP_LFB_SPACE);
	    break;
	  case '1':  do { cmdp->space = iRandom(3); } while (cmdp->space == SSTCP_YUV_SPACE);
	    break;
	  case '2':  do { cmdp->space = iRandom(3); } while (cmdp->space == SSTCP_TEXPORT_SPACE);
	    break;
	  case '3': do { cmdp->space = iRandom(3); } while (cmdp->space == SSTCP_3DLFB_SPACE);
	    break;
	  default:
	    cmdp->space = iRandom(3);
	    break;
	  }
	}
	else 
	  switch(selectType) {
	  case '0':  cmdp->space = SSTCP_LFB_SPACE;
	    break;
	  case '1':cmdp->space = SSTCP_YUV_SPACE;
	    break;
	  case '2':cmdp->space = SSTCP_TEXPORT_SPACE;
	    break;
	  case '3':cmdp->space = SSTCP_3DLFB_SPACE;
	    break;
	  default:
	    switch(iRandom(7)) {
	    case 0:case 2: case 3:cmdp->space = SSTCP_TEXPORT_SPACE;
	      break;
	    case 1:case 4: case 5:cmdp->space = SSTCP_3DLFB_SPACE;
	      break;
	    case 6:  cmdp->space = SSTCP_LFB_SPACE;
	      break;
	    case 7:cmdp->space = SSTCP_YUV_SPACE;
	      break;
	    }
	    break;
	  }
      }

      if (randomConfig == '2' || (pass & 31) == 0 && pass > 0 && randomConfig == '3') {
      	if (randomConfig == '2' || iRandom(1)) {
	  sst_idle_really(sst);
	  memConfig = 
	    (mrRandom(16,0x1FFF) << SST_RAW_LFB_TILE_BEGIN_PAGE_SHIFT)
	    & SST_RAW_LFB_TILE_BEGIN_PAGE |
	    (iRandom(3) << SST_RAW_LFB_ADDR_STRIDE_SHIFT) & SST_RAW_LFB_ADDR_STRIDE |
	    (mrRandom(1,0x3F) << SST_RAW_LFB_TILE_STRIDE_SHIFT) & SST_RAW_LFB_TILE_STRIDE;
	  SET(sstio->lfbMemoryConfig,memConfig);
	  GDBG_INFO(2,"LFB Memory Config 0x%08x\n",memConfig);
	}
      }
      loop1 = 0;
    again_sizebytes:
      if (loop1++ > MAXLOOP) {
	gdbg_printf("WARNING:: +Diag quitting early : %s of size %d bytes from srcAddr {0x%01x,0x%08x} width %d stride %d\n",
		  space_str[cmdp->space],cmdp->sizeBytes,cmdp->baseHigh,cmdp->baseLow,
		  cmdp->srcWidth,cmdp->srcStride);
	gdbg_printf("\t\t\t\tto dstAddr 0x%08x stride %d\n",cmdp->fbOffset,cmdp->dstStride);
	gdbg_printf("\t\t\t\tsrcOffset 0x%08x dstOffset 0x%08x\n",srcOffset,dstOffset);
	DIAG_PASS(1);
	exit(1);
      }

      switch(cmdp->space) {
      case SSTCP_3DLFB_SPACE:
	cmdp->sizeBytes = mrRandom(4,size);
	break;
      case SSTCP_TEXPORT_SPACE:
	cmdp->sizeBytes = mrRandom(wordTexAlign ? 2:1,size);
	break;
      case SSTCP_YUV_SPACE:
	cmdp->sizeBytes = mrRandom(4,size);
	break;
      case SSTCP_LFB_SPACE:	
	cmdp->sizeBytes = mrRandom(4,size);
	break;
      }
      if (cmdp->space == SSTCP_3DLFB_SPACE) { 
	if (iRandom(1)) {
	  sRegs.lfbMode = rndlfbMode( iRandom(NUM_TEST_OPTS-1), &pixelsToWrite );
	  SET( sst->lfbMode, sRegs.lfbMode );
	}
	lfbAlign(&cmdp->sizeBytes);
      }
      // byte texture NYI
      if (cmdp->space == SSTCP_TEXPORT_SPACE) 
	if (wordTexAlign) cmdp->sizeBytes &= ~0x1; // 2 byte aligned

      if (cmdp->space == SSTCP_YUV_SPACE) {
	selectY = iRandom(1);
	if (selectY) 
	  cmdp->srcStride = mrRandom(4,(FxU32)lsize > maxYuvWidth/2 ? maxYuvWidth/2 : lsize);
	else
	  cmdp->srcStride = mrRandom(4,(FxU32)lsize > maxYuvWidth/4 ? maxYuvWidth/4 : lsize);
	cmdp->srcStride &= ~0x3;
      }
      else 
	if (iRandom(1)) 
	  cmdp->srcStride = mrRandom(4,lsize);
	else 
	  cmdp->srcStride = mrRandom(4,1024);

      if (cmdp->space == SSTCP_3DLFB_SPACE) lfbAlign(&cmdp->srcStride);
      if (cmdp->space == SSTCP_TEXPORT_SPACE) 
	if (wordTexAlign) cmdp->srcStride &= ~0x1; // 2 byte aligned

      if (cmdp->srcStride > cmdp->sizeBytes)
	cmdp->srcStride = cmdp->sizeBytes;
	
      switch(cmdp->space) {
      case SSTCP_3DLFB_SPACE:
	cmdp->srcWidth = mrRandom(4,cmdp->srcStride);
	lfbAlign(&cmdp->srcWidth);
	break;
      case SSTCP_TEXPORT_SPACE:
	cmdp->srcWidth = mrRandom(wordTexAlign ? 2:1,cmdp->srcStride);
	if (wordTexAlign) cmdp->srcWidth &= ~0x1; // 2 byte aligned
	break;
      case SSTCP_YUV_SPACE:
	cmdp->srcStride &= ~0x3;
	cmdp->srcWidth = mrRandom(4,cmdp->srcStride);
	cmdp->srcWidth &= ~0x3;
	break;
      case SSTCP_LFB_SPACE:
	cmdp->srcWidth = mrRandom(2,cmdp->srcStride);
	break;
      }
      // round up to width
      cmdp->sizeBytes = (cmdp->sizeBytes/cmdp->srcWidth + 1)*cmdp->srcWidth;
      srcOffset = (cmdp->sizeBytes/cmdp->srcWidth) * cmdp->srcStride;

      if (srcOffset > totalSize ) {
	goto again_sizebytes;
      }
      if (cmdp->space == SSTCP_3DLFB_SPACE && 
	  (FxU32)diago.ymaxscreen < (cmdp->sizeBytes/cmdp->srcWidth + 1))
	  goto again_sizebytes;
	

      srcAddr = (FxU32)agpMem;
      if (pass > 1) {
	srcAddr = (FxU32)agpMem + iRandom(totalSize - srcOffset);
	srcAddr &= ~0xF;
      }
      // gdbg_info(1,"Agp virtual src addr 0x%x\n",srcAddr);	
      //
      agpVirtToPhys((FxU32 *)srcAddr,&cmdp->baseHigh,&cmdp->baseLow);

      loop2 = 0;
    again_trashmem:
      if (loop2++ > MAXLOOP) {
	gdbg_printf("WARNING:: ++Diag quitting early : %s of size %d bytes from srcAddr {0x%01x,0x%08x} width %d stride %d\n",
		  space_str[cmdp->space],cmdp->sizeBytes,cmdp->baseHigh,cmdp->baseLow,
		  cmdp->srcWidth,cmdp->srcStride);
	gdbg_printf("\t\t\t\tto dstAddr 0x%08x stride %d\n",cmdp->fbOffset,cmdp->dstStride);
	gdbg_printf("\t\t\t\tsrcOffset 0x%08x dstOffset 0x%08x\n",srcOffset,dstOffset);
	DIAG_PASS(1);
	exit(1);
      }

      if (pass == sanityPass) {
	cmdp->fbOffset = GET(sst->colBufferAddr); // 0
	cmdp->dstStride = GET(sst->colBufferStride); // 0x500
	if (cmdp->srcWidth > cmdp->dstStride) 
	  cmdp->dstStride = cmdp->srcWidth;
	goto done;
      } 
      
      switch(cmdp->space) {
      case SSTCP_3DLFB_SPACE:
	cmdp->dstStride = 2048*lfbSize(&sRegs);
	break;
      case SSTCP_TEXPORT_SPACE:
	if (wordTexAlign) {
	  cmdp->dstStride = mrRandom(2,MAX_DST_STRIDE);
	  cmdp->dstStride &= ~0x1; // 2 byte aligned
	}
	else 	  cmdp->dstStride = mrRandom(1,MAX_DST_STRIDE);
	break;
      case SSTCP_YUV_SPACE:
	cmdp->dstStride = YUVSTRIDE;
	break;
      case SSTCP_LFB_SPACE:
	cmdp->dstStride = mrRandom(2,MAX_DST_STRIDE);
	break;
      }
      
      if (cmdp->srcWidth > cmdp->dstStride) {
	GDBG_INFO(GWARN,"Forcing srcwidth to destination stride %x\n",cmdp->dstStride);
	cmdp->dstStride = cmdp->srcWidth;
      }
      
      //  actual size of destination
      dstOffset = ((cmdp->sizeBytes/cmdp->srcWidth) + 1) * cmdp->dstStride;
      switch(cmdp->space) {
      case SSTCP_3DLFB_SPACE:
	xx = iRandom(diago.xmaxscreen-cmdp->srcWidth/lfbSize(&sRegs));
	yy = iRandom(diago.ymaxscreen-((cmdp->sizeBytes/cmdp->srcWidth) + 1));
	cmdp->fbOffset = (xx << SST_LFB_ADDR_X_SHIFT) & SST_LFB_ADDR_X |
	  (yy << SST_LFB_ADDR_Y_SHIFT) & SST_LFB_ADDR_Y;
	cmdp->fbOffset *= lfbSize(&sRegs);
	GDBG_INFO(1,"3DLFB xx %d yy %d \n",xx,yy);
	GDBG_INFO(SANITY,"ss %d sb %d sw %d \n",lfbSize(&sRegs),
		  ((cmdp->sizeBytes/cmdp->srcWidth) + 1),
		  cmdp->sizeBytes,cmdp->srcWidth);
	break;
      case SSTCP_TEXPORT_SPACE:

	//Randomly change texBaseAddr
	if(iRandom(1))
	  {
	    unallocateByName("texture");
	    texBaseAddr = allocate(texSize, "texture", randomPlacement);
	    sRegs.texBaseAddr = texBaseAddr;
	    newTexBase = 1;
	  }
	else
	  newTexBase = 0;

	cmdp->fbOffset = mrRandom(0,texSize);
	cmdp->fbOffset += SST_TEX0_OFFSET;
	cmdp->fbOffset &= ~0x1;
	break;
      case SSTCP_LFB_SPACE:
	cmdp->fbOffset = allocate(dstOffset, "LFB Shit", randomPlacement) & ~3;
	unallocateByName("LFB Shit");

	maskCmd(cmdp);
	gdbg_info(150,"Adjusting fbOffset to 0x%x\n",cmdp->fbOffset);
	break;
      case SSTCP_YUV_SPACE:
	// Calculate destination frame buffer size in bytes src lines * dst stride
	if (selectY) {
	  dstOffset = 2*(cmdp->sizeBytes/cmdp->srcWidth + 1) * cmdp->dstStride;
	}
	else {
	  dstOffset = 4*(cmdp->sizeBytes/cmdp->srcWidth + 1) * cmdp->dstStride;
	}
	planarOff = (cmdp->sizeBytes/cmdp->srcWidth + 1) * YUVSTRIDE;
	// randomly select among U V
	do { 
	  cmdp->fbOffset = iRandom(0x100000-planarOff) + 
	    (selectY ? 0 : iRandom(1) ? 0x100000 : 0x200000);
	  cmdp->fbOffset &= ~0x3;
	  xOff = (cmdp->fbOffset & SST_YUV_ADDR_X) >> SST_YUV_ADDR_X_SHIFT;
	} // check if offset blows out stride
	while (xOff + cmdp->srcWidth > YUVSTRIDE ||
	       xOff + cmdp->srcWidth > (selectY ? maxYuvWidth/2 : maxYuvWidth/4)); 
	
	yOff = (cmdp->fbOffset & SST_YUV_ADDR_Y) >> SST_YUV_ADDR_Y_SHIFT;
	if (cmdp->fbOffset & YUVSELECT) {
	  xOff <<= 1;
	  yOff <<= 1;
	}
	GDBG_INFO(SANITY,"YUV fbOffset %x Xoff %x yoff %x\n",
		  cmdp->fbOffset,xOff,yOff);
	newYuvBase = iRandom(1);
	if (newYuvBase) {
	  unallocateByName("yuv");
	  yuvBaseAddr = allocate(diago.ymaxscreen * diago.xmaxscreen * 2, "yuv", randomPlacement);
	  yuvBaseAddr &= ~0xF;			
	}
	break;
      } // case
      
      // min of 5 DWORDS
      if (cmdp->sizeBytes <= 1)
	  goto again_sizebytes;


      // do masking before check
      maskCmd(cmdp);

      if (cmdp->space == SSTCP_LFB_SPACE) {
	goto done;
      }
      if (cmdp->space == SSTCP_TEXPORT_SPACE) {
	if (iRandom(1)) {
	  // set up texture download
	  tt->tex->tMode = texRandomFormat(iRandom(1),iRandom(1), iRandom(1)) 
	    | SST_TC_REPLACE | SST_TCA_REPLACE;
	  SET(sst->textureMode,tt->tex->tMode);
	}
	if (newTexBase) {
	  SET(sst->texBaseAddr,texBaseAddr&SST_TEXTURE_ADDRESS);
	  GDBG_INFO(1,"Tex Base Addr 0x%08x\n",texBaseAddr);
	  shadowTexBase = texBaseAddr;
	}
	goto done;
	
      }
	else {
	  texBaseAddr = shadowTexBase;
	}
      }
      if (cmdp->space == SSTCP_3DLFB_SPACE) 
	goto done;
      if (cmdp->space == SSTCP_YUV_SPACE) {
	FxU32 maxOff;
	maxOff = xOff*2 + yOff*maxYuvWidth+dstOffset;
	// blow out aperture
	gdbg_info(SANITY,"planarOff 0x%08x\n",planarOff);
	if ((cmdp->fbOffset & YUVSELECT) != (cmdp->fbOffset + planarOff & YUVSELECT))
	  goto again_sizebytes;

	if (newYuvBase) {
	  sst_idle_really(sst);
	  SET(sstc->yuvBaseAddr,yuvBaseAddr & SST_YUV_BASE_ADDR);
	  GDBG_INFO(1,"Tex Base Addr 0x%08x\n",texBaseAddr);
	}
	goto done;
      }
      // failed 
      goto again_trashmem;
    done:
      cmdp->id = iRandom(1);
      switch(cmdp->space) {
      case SSTCP_3DLFB_SPACE:
	np3dlfb += cmdp->sizeBytes;
	n3dlfb++;
	break;
      case SSTCP_TEXPORT_SPACE:
	nptex += cmdp->sizeBytes;
	ntex++;
	break;
      case SSTCP_YUV_SPACE:
	npyuv += cmdp->sizeBytes;
	nyuv++;
	break;
      case SSTCP_LFB_SPACE:
	nplfb += cmdp->sizeBytes;
	nlfb++;
	break;
      }
      
      if (selectpType ) {
	if (pType == '5') pp = 1;
	else pp = 0;
      }
      else if (diago.writeFifo) 
	pp = iRandom(3);
      else pp = 0;


      switch(cmdp->space) {
      case SSTCP_3DLFB_SPACE:
	start3dlfb(sst);
	break;
      case SSTCP_TEXPORT_SPACE:
#if defined H3_A0 || defined H3_A1 || defined H3_A2
    {
      static i = 0;
      if ( i++ == 0 )
	GDBG_INFO(0,"HACK -- forcing NOP before texture download (H3 work-around)\n");
      SET(sst->nopCMD,0);
    }
#endif
	break;
      case SSTCP_YUV_SPACE:
	break;
      case SSTCP_LFB_SPACE:
	break;
      }

      // choose packet type
      switch(pp) { 
      case 0: case 2: case 3:
      if (pass >= debugPass) {
	GDBG_INFO(1,"MOVE %s of size %d bytes from srcAddr {0x%04x,0x%08x} width %d stride %d\n",
		  space_str[cmdp->space],cmdp->sizeBytes,cmdp->baseHigh,cmdp->baseLow,
		  cmdp->srcWidth,cmdp->srcStride);
	GDBG_INFO(1,"\t\t\t\tto dstAddr 0x%08x stride %d\n",cmdp->fbOffset,cmdp->dstStride);
	GDBG_INFO(2,"\t\t\t\tsrcOffset 0x%08x dstOffset 0x%08x\n",srcOffset,dstOffset);
	SET(sstc->agpReqSize,cmdp->sizeBytes);
	SET(sstc->hostAddrLow,cmdp->baseLow & SST_AGP_MOVE_BASELOW );
	SET(sstc->hostAddrHigh,
	    (cmdp->baseHigh << SST_AGP_SRC_BASEHIGH_SHIFT) & SST_AGP_SRC_BASEHIGH
	    | (cmdp->srcWidth & SST_AGP_SRC_WIDTH)
	    | (cmdp->srcStride <<SST_AGP_SRC_STRIDE_SHIFT) & SST_AGP_SRC_STRIDE);
	SET(sstc->graphicsAddr,cmdp->fbOffset & SST_AGP_FRAME_BUFFER_OFFSET);
	
	//      gdbg_info(1,"sanity %x %x %x\n",cmdp->dstStride, SST_AGP_DSTSTRIDE,cmdp->dstStride & SST_AGP_DSTSTRIDE);
	//      gdbg_info(1,"sanity %x\n",cmdp->srcStride);
	SET(sstc->graphicsStride,cmdp->dstStride & SST_AGP_DSTSTRIDE);
	SET(sstc->moveCMD,
	    (cmdp->id << SST_AGPMOVE_CMDID_SHIFT) & SST_AGPMOVE_CMDID | 
	    (cmdp->space << SST_AGPMOVE_SPACE_SHIFT) &  SST_AGPMOVE_SPACE);
	if (diago.writeFifo == 0)
	  sst_idle_really(sst);				// wait for the command to complete
      }
      break;
      case 1: // packet 5
      if (pass >= debugPass) {
	GDBG_INFO(1,"P5 (one or more) %s of size %d bytes from srcAddr {0x%01x,0x%08x} width %d stride %d\n",
		  space_str[cmdp->space],cmdp->sizeBytes,cmdp->baseHigh,cmdp->baseLow,
		  cmdp->srcWidth,cmdp->srcStride);
	GDBG_INFO(1,"\t\t\t\tto dstAddr 0x%08x stride %d\n",cmdp->fbOffset,cmdp->dstStride);
	GDBG_INFO(2,"\t\t\t\tsrcOffset 0x%08x dstOffset 0x%08x\n",srcOffset,dstOffset);
	dstAddr = cmdp->fbOffset;
	p5Addr = dstAddr;
	nWords = 1 + (cmdp->srcWidth - 4 + (dstAddr & 0x3))/4;
	seats = 1 + (cmdp->srcWidth - 4 + (dstAddr & 0x3) + 3)/4;
	if (seats == 1) nWords = 1;
	cWords = MIN((int)psize,nWords);
	ww = cWords; 
	remWords = nWords - cWords;
	GDBG_INFO(SANITY,"Initial nwords %d seats %d ; cwords %d remWords %d ww %d\n",nWords,seats,cWords,remWords,ww);
	dstData = 0;
	srcData = AGPRDP(cmdp->baseHigh,(cmdp->baseLow & ~0x3));
	for (ii=0,srcAddrHigh = cmdp->baseHigh,srcAddrLow = cmdp->baseLow,
	       prevLow = cmdp->baseLow,prevHigh = cmdp->baseHigh,	   
	       prevDstAddr = cmdp->fbOffset;
	     (FxU32)ii<cmdp->sizeBytes;ii++) {
	  if ((srcAddrLow & 0x3) == 0) {
	    srcData = AGPRDP(srcAddrHigh,srcAddrLow);
	    GDBG_INFO(150,"agp read src data 0x%x at L0x%x H0x%x\n",srcData,srcAddrLow,srcAddrHigh);
	  }
	  dstData = dstData | (0xFF & (srcData >> ((srcAddrLow&3) << 3))) << ((dstAddr&3) << 3) ;
	  // gdbg_info(1,"After dstAddr %x dstData %x\n",dstAddr,dstData);
	  if ((dstAddr & 0x3) == 0x3) {
	    if (ww == cWords) {
	      GDBG_INFO(SANITY,"+++ dstAddr %x nwords %d seats %d ; cwords %d remWords %d ww %d \n"
			,dstAddr,nWords,seats,cWords,remWords,ww);
	      GDBG_INFO(1,"\t\t\t\tinitial data 0x%0x\n",dstData);
	      cmdP5Start(cmdp->space,((1 << (p5Addr & 0x3))-1),0,cWords,p5Addr,dstData);
	      dstData = 0;
	    }
	    else {
	      cmdP5Data(dstData);
	      dstData = 0;
	    }
	    if (ww-- == 0) {
	      GDBG_ERROR("p5","Too many words ww %d\n",cWords);
	    }
	    if (ww == 0 && remWords > 0) { // multiple packets per line
	      cWords = MIN((int)psize,remWords);
	      p5Addr = dstAddr + 1;
	      ww = cWords;
	      GDBG_INFO(SANITY,"more pkts for this line .... dstAddr %x nwords %d seats %d ; cwords %d remWords %d\n",
			dstAddr,nWords,seats,cWords,remWords);
	      remWords -= cWords;
	      dstData = 0;
	    }
	  }
	  if (srcAddrLow == 0xFFFFFFFF)
	    GDBG_ERROR("moveCmd","Base address overflow NYI");
	  srcAddrLow++;
	  dstAddr++;
	  if ((srcAddrLow - prevLow) >= cmdp->srcWidth) {
	    srcAddrLow = prevLow + cmdp->srcStride;
	    prevLow = srcAddrLow;
	    prevHigh = srcAddrHigh;
	    if (ww != 0) { // or last word is first word !!
	      GDBG_INFO(SANITY,"last word is first word: dstAddr %x nwords %d seats %d ; cwords %d remWords %d\n",
			dstAddr,nWords,seats,cWords,remWords);
	      cmdP5Start(cmdp->space,((1 << (p5Addr & 0x3))-1),0,cWords,p5Addr,dstData);
	    }
	    else 
	    if (seats != nWords) {
	      GDBG_INFO(SANITY,"seats > nwords: dstAddr %x nwords %d seats %d ; cwords %d remWords %d\n",
			dstAddr,nWords,seats,cWords,remWords);
	      cmdP5Start(cmdp->space,(0xF << (dstAddr & 0x3)) & 0xF,0,1,(dstAddr&~0x3),dstData);
	    }
	    // todo tiled ??
	    dstAddr = prevDstAddr + cmdp->dstStride;
	    prevDstAddr = dstAddr;
	    // next scan line if any
	    nWords = 1 + (cmdp->srcWidth - 4 + (dstAddr & 0x3))/4;
	    seats = 1 + (cmdp->srcWidth - 4 + (dstAddr & 0x3) + 3)/4;
	    if (seats == 1) 
	      nWords = 1;
	    cWords = MIN((int)psize,nWords);
	    p5Addr = dstAddr;
	    ww = cWords; 
	    remWords = nWords - cWords;
	    GDBG_INFO(SANITY,"++++ dstAddr %x nwords %d seats %d ; cwords %d remWords %d\n",
		      dstAddr,nWords,seats,cWords,remWords);
	    dstData = 0;
	    srcData = AGPRDP(cmdp->baseHigh,(srcAddrLow & ~0x3));
	  }
	  // assert dstStride matches pci stride
	}
      }
      break;
      } // packet type
      switch(cmdp->space) {
      case SSTCP_3DLFB_SPACE:
	restoreTri(sst);
	break;
      case SSTCP_TEXPORT_SPACE:
	break;
      case SSTCP_YUV_SPACE:
	break;
      case SSTCP_LFB_SPACE:
	break;
      }
      if (pBurst > 0)
	goto again_more;
      pushWax(0);
      if (enableHblt)
	for (ii=0;ii<(int)iRandom(4);ii++) {
	  if (iRandom(1)) 
	    rect(sst,lsize/2,pass);
	  else h2sblt(sst,lsize/2,pass);
	}
      else
	for (ii=0;ii<(int)iRandom(2);ii++) {
	  if (bigSize && (iRandom(4) == 1))
	    rect(sst,lsize*8,pass);
	  else
	    rect(sst,lsize/2,pass);
	  if (enableHblt)
	    h2sblt(sst,lsize/2,pass);
	  
#if 0
	  if (iRandom(1)) 
	    line(sst,lsize,pass);
	  else
	    h2sblt(sst,lsize,pass);
#endif
	}
      popWax(0);

      pt = NULL;
      if (iRandom(1) && pass > 2) {
	for (ii=0;ii<(int)iRandom(8);ii++) {
	  int tsize;
	  static Triangle t;
	  tsize = mrRandom(5,50);
	  pt = &t;
	  ntriangle++;
	  if (bigSize && (iRandom(4) == 1)){
	    randomStressTriangle(pt,bigSize,1,1);
	  }
	  else
	    randomStressTriangle(pt,tsize,1,1);	// pick random triangle
						     
	  if (pass >= debugPass) {
	    drawTriangle(sst,pt,1,0,0);
	    if (diago.checkEveryTriangle) {
	      sst_idle(sst);				// wait for the command to complete
	      checkTriangle(pt,1,1,diago.adjust, insideTriangle,1,0,0);
	      eraseTriangle(sst,pt,1,0,0);		// erase the triangle
	    }
	  }
	}
      }

      if (frameCount > 0) {
	if (iRandom(8) == 2 && pass > 5) {
	  if (enableVideo) {
	    sst_vsync_inactive ( sst ) ;       
	    sst_vsync_active(sst);
	  }
	  frameCount--;
	}
      }
      if (diago.checkEveryTriangle && pass >= debugPass) {
	int type,inc;
	int nn;
	FxU32 saddr,daddr;
	FxU32 sdata,ddata,sprevaddr,dprevaddr;
	sst_idle_really(sst);				// wait for the command to complete
	switch(cmdp->space) {
	case  SSTCP_YUV_SPACE:
	  dprevaddr = daddr = SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET 
	    + (xOff*2 + yOff*maxYuvWidth) + (yuvBaseAddr & SST_YUV_BASE_ADDR);  
	  sprevaddr = saddr = srcAddr;
	  gdbg_info(150,"move %d: YUV Comparing at s %x : d %x (%d bytes)\n",pass,srcAddr,daddr,cmdp->sizeBytes);
	  sdata = AGPRDV(*(FxU32 *)(saddr & ~0x3));
	  sdata >>= ((saddr & 0x3)*8);
	  type = cmdp->fbOffset & YUVSELECT;
	  for (nn = 0;nn < (int)cmdp->sizeBytes;nn++) {
	    if ((saddr & 0x3) == 0) 
	      sdata = AGPRDV(*(FxU32 *)saddr);
	    switch(type) {
	    case 0:
	      ddata = GET8(*(FxU32 *)daddr);
	      if ((sdata & 0xFF) != (ddata & 0xFF)) {
		gdbg_error("move","Mismatch at s%x:d%x of %x:%x\n",
			   saddr,daddr,sdata & 0xFF,ddata & 0xFF);
		DIAG_FAIL();
	      }
	      daddr += 2;
	      break;
	    case 0x100000: 
	      inc = 1;
	    case 0x200000:
	      if (type == 0x200000) inc = 3;
	      ddata = GET8(*(FxU32 *)(daddr+inc));
	      if ((sdata & 0xFF) != (ddata & 0xFF)) {
		gdbg_error("move","Mismatch at s%x:d%x of %x:%x\n",
			   saddr,daddr+inc,sdata & 0xFF,ddata & 0xFF);
		DIAG_FAIL();
	      }
	      ddata = GET8(*(FxU32 *)(daddr+inc+maxYuvWidth));
	      if ((sdata & 0xFF) != (ddata & 0xFF)) {
		gdbg_error("move","Mismatch next at s%x:d%x of %x:%x\n",
			   saddr,daddr+inc+maxYuvWidth,sdata & 0xFF,ddata & 0xFF);
		DIAG_FAIL();
	      }
	      daddr += 4;
	      break;
	    default:
	      gdbg_error("move","Illegal YUV address  %x\n",cmdp->fbOffset);
	    }
	    // erase
	    // SET8(*(FxU32 *)daddr,0);
	    sdata >>= 8; 
	    saddr++;
	    if (saddr-sprevaddr >= cmdp->srcWidth) {
	      saddr = sprevaddr + cmdp->srcStride;
	      daddr = dprevaddr + maxYuvWidth;
	      if (type != 0)
		daddr += maxYuvWidth;
	      sprevaddr = saddr;
	      dprevaddr = daddr;
	      sdata = AGPRDV(*(FxU32 *)(saddr & ~0x3));
	      sdata >>= ((saddr & 0x3)*8);
	    }
	  }
	  
	  break;
	case SSTCP_3DLFB_SPACE:
	  break;
	case SSTCP_LFB_SPACE:
	case SSTCP_TEXPORT_SPACE:
	  if (cmdp->space == SSTCP_LFB_SPACE) {
	    dprevaddr = daddr = SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET + cmdp->fbOffset;  
	  }
	  else {
	    dprevaddr = daddr = SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET + 
	    cmdp->fbOffset+(shadowTexBase&SST_TEXTURE_ADDRESS) ;  
	  }

	  sprevaddr = saddr = srcAddr;
	  //	  gdbg_info(1,"daddr = %x %x %x %x %x\n",
	  //		    SST_BASE_ADDRESS(sst),SST_RAW_LFB_OFFSET,cmdp->fbOffset,shadowTexBase,SST_TEXTURE_ADDRESS);
	  gdbg_info(150,"move %d: Comparing at s %x : d %x (%d bytes)\n",pass,srcAddr,daddr,cmdp->sizeBytes);
	  sdata = AGPRDV(*(FxU32 *)(saddr & ~0x3));
	  sdata >>= ((saddr & 0x3)*8);
	  for (nn = 0;nn < (int)cmdp->sizeBytes;nn++) {
	    if ((saddr & 0x3) == 0) 
	      sdata = AGPRDV(*(FxU32 *)saddr);

	    ddata = GET8(*(FxU32 *)daddr);
	    if ((sdata & 0xFF) != (ddata & 0xFF)) {
	      gdbg_error("move","Mismatch at s%x:d%x of %x:%x\n",
	 		 saddr,daddr,sdata & 0xFF,ddata & 0xFF);
	      DIAG_FAIL();
	    }
	    // erase
	    // SET8(*(FxU32 *)daddr,0);
	    sdata >>= 8; 
	    saddr++;
	    daddr++;
	    if (saddr-sprevaddr >= cmdp->srcWidth) {
	      saddr = sprevaddr + cmdp->srcStride;
	      daddr = dprevaddr + cmdp->dstStride;
	      sprevaddr = saddr;
	      dprevaddr = daddr;
	      sdata = AGPRDV(*(FxU32 *)(saddr & ~0x3));
	      sdata >>= ((saddr & 0x3)*8);
	    }
	  }
	  break;
	} // case
      }
    }
    GDBG_INFO(0,"Packet Count: LFB %d YUV %d 3DLFB %d TexPort %d\n\t\tRect %d lines %d Triangles %d\n",
	      nlfb,nyuv,n3dlfb,ntex,nrect,nline,ntriangle);
    GDBG_INFO(0,"Rough Pixel Count: LFB %d YUV %d 3DLFB %d TexPort %d\n\t\tRect %d lines %d\n",
	      nplfb,npyuv,np3dlfb,nptex,nprect,npline);
    sst_idle_really(sst);
    if (enableVideo) 
      vidCleanup(sst);
    DIAG_PASS(0);	 
}





