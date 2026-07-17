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
** $Date: 10/11/00 8:19:10 PM$
** NYI 
** . tiled texture, YUV
** . dst stride 14:0
** . byte aligned texture    
** . src stride/width 14:0
** . tiled LFB
** . 3DLFB auto checking
** . p5
** . src memory shadow for non AGP 
*/

#include "udiag.h"
#include "sstdiag.h"
#include "../trex/stwtri.h"
#include "fbi.h"
#include "lfbutils.h"

#define GLEVEL 2
#define GWARN 2
#define SANITY 150
static int debugPass = -1;
static int sanityPass = -1;
static int pType;
static int selectpType;
static char *space_str[] = {"LFB","YUV","3DLFB","TEX"};
static SstRegs sRegs;
static void agpRandomMem(SstRegs *sst, FxU32 *mem, int sizeBytes)
{
    int ii;
    unsigned int saveseed = getSeed();
    FxU32 col;
    FxU32 *pmem;
    unsigned rr;
    int randIndex = 0;
    static FxU32 randoms[107];
    int size;
    
    setSeed(999);		// always generate the same screen

    size = (sizeBytes+3)/4;
    gdbg_info( 2, "agpRandomMem: Initializing AGP 0x%x %d to random colors ... \n",mem,sizeBytes );

    rr = 0;
    // initialize the pool of random frame buffer values
    for (ii = 0; ii < sizeof(randoms); ii++)
	randoms[ii] = iRandom(0xffffffff);

    for (pmem=mem, ii = 0; ii < size; ii++,pmem++ ) {
	col = randoms[randIndex] ^ rr;
	randIndex += 1;
	if (randIndex >= sizeof(randoms)) {
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

static int checkAgpSize(SstRegs *sst,int sizeBytes)
{
  CsimPrivate *cpriv;
  cpriv = CSIM_PRIVATE(diago.sstCSIM);
  if ((unsigned int)sizeBytes > cpriv->info->agpSizeInBytes) {
    gdbg_info(1,"Setting maximum buffer size to maximum agp memory %d bytes\n",
	      cpriv->info->agpSizeInBytes);
    return(cpriv->info->agpSizeInBytes/2); // save half for cmd fifo
  }
  return(sizeBytes);
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
    SST_CC_REVERSE_BLEND | 
    SST_CCA_REVERSE_BLEND;
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

static int selectType;
static int selectInverse;
static int randomConfig;
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
    gdbg_printf("\"-xp<n>\"\tP5 P6\n");
    gdbg_printf("\"-xt<n>\"\t0:Linear Space 1:Tiled 2:Randomised memory config every pass 3:Randomised memory config every 32 passes\n");
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
	    case 'p':
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

void
main (int argc, char **argv)
{
    int ii;
    int pp;
    SstRegs *sst;
    SstCRegs *sstc;
    SstIORegs *sstio;
    FxU32 ttemp;
    // texture
    Triangle *pt;
    Triangle *t;
    FxU32 texBaseAddr,shadowTexBase;
    int newTexBase;
    Triangle *tt;

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
    int sanity;

    //p5
	int ww;
	int nWords,seats,cWords,remWords; 
	FxU32 p5Addr;
	FxU32 srcAddrHigh,srcAddrLow;
	FxU32 prevLow,prevHigh;
	FxU32 srcData,dstAddr,prevDstAddr,dstData;

    AGPMOVECMD cmd;
    AGPMOVECMD *cmdp;
 
    int down;
    FxU32 memConfig;
    FxU32 tiledOffsetBytes;
    FxU32 psize;
    cmdp = &cmd;

    sst = SST_BEGIN(argc,argv);
    sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);
    sstio = (SstIORegs *)SST_IO_ADDRESS(sst);

    if(diago.bigAssTextures)
      {
	t = buildTriangle(2048, 2048);
	tt = buildTriangle(2048, 2048);
      }
    else
      {
	t = buildTriangle(256, 256);
	tt = buildTriangle(256, 256);
      }


    selectInverse = 0;
    selectType = '4';
    randomConfig = '0';
    pType = '5';
    selectpType = 0;

    XParseOpts(argc,argv);
    // SET(sstio->lfbMemoryConfig,0xa3FFF); 
    if (randomConfig != '0') {
      FxU32 tiledSpaceBeginPage;

      tiledSpaceBeginPage = rRandom(diago.minTrashMem / SST_TILE_SIZE, 0x7FFF);
      
      memConfig = (SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE(tiledSpaceBeginPage)) |
	(iRandom(SST_RAW_LFB_ADDR_STRIDE_MAX) << SST_RAW_LFB_ADDR_STRIDE_SHIFT) & SST_RAW_LFB_ADDR_STRIDE |
	(rRandom(1,0x7F) << SST_RAW_LFB_TILE_STRIDE_SHIFT) & SST_RAW_LFB_TILE_STRIDE;
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

    totalSize =  checkAgpSize(sst,size);

    agpMem = (FxU32 *)AGPMEMALLOC(sst,totalSize);
    if (agpMem == NULL) DIAG_FAIL();
    agpRandomMem(sst,agpMem,totalSize);

    // set up texture download
    tt->tex->tMode = texRandomFormat(iRandom(1),iRandom(1),iRandom(1)) 
	 | SST_TC_REPLACE | SST_TCA_REPLACE;
    SET(sst->textureMode,tt->tex->tMode);	// set texture mode just for fun
    texBaseAddr = 0;
    SET(sst->texBaseAddr,texBaseAddr&SST_TEXTURE_ADDRESS);
    GDBG_INFO(1,"Tex Base Addr 0x%08x\n",texBaseAddr);
    shadowTexBase = texBaseAddr;
    tt->next = NULL;

    // Generate Random LFB Write Parameters
    sRegs.lfbMode = rndlfbMode( iRandom(NUM_TEST_OPTS-1), &pixelsToWrite );
    sRegs.lfbMode &= ~SST_LFB_ENPIXPIPE;
    SET( sst->lfbMode, sRegs.lfbMode );
    sRegs.clipLeftRight = (0<<16) | diago.xmaxscreen;
    SET(sst->clipLeftRight, sRegs.clipLeftRight );
    sRegs.clipBottomTop = (0<<16) | diago.ymaxscreen;
    SET(sst->clipBottomTop,sRegs.clipLeftRight );
    
    // yuv
    do {
      yuvBaseAddr  = sstFbMemRalloc(4096);
      yuvBaseAddr &= ~0xF;			
    }
    while (yuvBaseAddr < diagfb.auxBufferAddr &&
		yuvBaseAddr + 8192 >= diagfb.colBufferAddr[0]);
      
    SET(sstc->yuvBaseAddr,yuvBaseAddr  & SST_YUV_BASE_ADDR); 
    GDBG_INFO(1,"Yuv Base Addr 0x%08x\n",yuvBaseAddr);
   // linear
    maxYuvWidth = 512;
    SET(sstc->yuvStride,SST_YUV_MEMORY_LINEAR | maxYuvWidth);

    while (DIAG_STARTPASS())	{		// for each pass
      pass++;
      if (pass == sanityPass)cmdp->space = SSTCP_LFB_SPACE;  
      else {
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
	    cmdp->space = iRandom(3);
	    break;
	  }
      }
      if (randomConfig == '2' || (pass & 31) == 0 && pass > 0 && randomConfig == '3') {
      	if (randomConfig == '2' || iRandom(1)) {
	  sst_idle_really(sst);
	  memConfig = 
	    (rRandom(diago.minTrashMem >> 12,0x1FFF) << SST_RAW_LFB_TILE_BEGIN_PAGE_SHIFT)
	    & SST_RAW_LFB_TILE_BEGIN_PAGE |
	    (iRandom(3) << SST_RAW_LFB_ADDR_STRIDE_SHIFT) & SST_RAW_LFB_ADDR_STRIDE |
	    (rRandom(1,0x3F) << SST_RAW_LFB_TILE_STRIDE_SHIFT) & SST_RAW_LFB_TILE_STRIDE;
	  SET(sstio->lfbMemoryConfig,memConfig);
	  GDBG_INFO(2,"LFB Memory Config 0x%08x\n",memConfig);
	}
      }
    again_sizebytes:
      switch(cmdp->space) {
      case SSTCP_3DLFB_SPACE:
	cmdp->sizeBytes = rRandom(2,size);
	break;
      case SSTCP_TEXPORT_SPACE:
	cmdp->sizeBytes = rRandom(2,size);
	break;
      case SSTCP_YUV_SPACE:
	cmdp->sizeBytes = rRandom(4,size);
	break;
      case SSTCP_LFB_SPACE:
	cmdp->sizeBytes = rRandom(2,size);
	break;
      }
      if (cmdp->space == SSTCP_3DLFB_SPACE) { 
	if (iRandom(1)) {
	  sRegs.lfbMode = rndlfbMode( iRandom(NUM_TEST_OPTS-1), &pixelsToWrite );
	  // bypass pixel pipe
	  sRegs.lfbMode &= ~SST_LFB_ENPIXPIPE;
	  SET( sst->lfbMode, sRegs.lfbMode );
	}
	lfbAlign(&cmdp->sizeBytes);
      }
      // byte texture NYI
      if (cmdp->space == SSTCP_TEXPORT_SPACE) cmdp->sizeBytes &= ~0x1; // 2 byte aligned

      if (cmdp->space == SSTCP_YUV_SPACE) {
	selectY = iRandom(1);
	if (selectY) 
	  cmdp->srcStride = rRandom(4,size > maxYuvWidth/2 ? maxYuvWidth/2 : size);
	else
	  cmdp->srcStride = rRandom(4,size > maxYuvWidth/4 ? maxYuvWidth/4 : size);
	cmdp->srcStride &= ~0x3;
      }
      else 
	if (iRandom(1)) 
	  cmdp->srcStride = rRandom(2,size);
	else 
	  cmdp->srcStride = rRandom(2,1024);

      if (cmdp->space == SSTCP_3DLFB_SPACE) lfbAlign(&cmdp->srcStride);
      if (cmdp->space == SSTCP_TEXPORT_SPACE) cmdp->srcStride &= ~0x1; // 2 byte aligned

      if (cmdp->srcStride > cmdp->sizeBytes)
	cmdp->srcStride = cmdp->sizeBytes;
	
      switch(cmdp->space) {
      case SSTCP_3DLFB_SPACE:
	cmdp->srcWidth = rRandom(2,cmdp->srcStride);
	lfbAlign(&cmdp->srcWidth);
	break;
      case SSTCP_TEXPORT_SPACE:
	cmdp->srcWidth = rRandom(2,cmdp->srcStride);
	cmdp->srcWidth &= ~0x1; // 2 byte aligned
	break;
      case SSTCP_YUV_SPACE:
	cmdp->srcStride &= ~0x3;
	cmdp->srcWidth = rRandom(4,cmdp->srcStride);
	cmdp->srcWidth &= ~0x3;
	break;
      case SSTCP_LFB_SPACE:
	cmdp->srcWidth = rRandom(2,cmdp->srcStride);
	break;
      }
      // round up to width
      cmdp->sizeBytes = (cmdp->sizeBytes/cmdp->srcWidth + 1)*cmdp->srcWidth;
      srcOffset = (cmdp->sizeBytes/cmdp->srcWidth) * cmdp->srcStride;

      if (srcOffset > totalSize ) {
	goto again_sizebytes;
      }
      if (cmdp->space == SSTCP_3DLFB_SPACE && 
	  (unsigned int)diago.ymaxscreen < (cmdp->sizeBytes/cmdp->srcWidth + 1))
	goto again_sizebytes;

      srcAddr = (FxU32)agpMem;
      if (iRandom(1) && pass > 1) {
	srcAddr = (FxU32)agpMem + totalSize - srcOffset;
      }
      // gdbg_info(1,"Agp virtual src addr 0x%x\n",srcAddr);	
      //
      agpVirtToPhys((FxU32 *)srcAddr,&cmdp->baseHigh,&cmdp->baseLow);

      sanity = 100;
    again_visible:
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
	cmdp->dstStride = rRandom(2,MAX_DST_STRIDE);
	cmdp->dstStride &= ~0x1; // 2 byte aligned
	break;
      case SSTCP_YUV_SPACE:
	cmdp->dstStride = YUVSTRIDE;
	break;
      case SSTCP_LFB_SPACE:
	cmdp->dstStride = rRandom(2,MAX_DST_STRIDE);
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
	newTexBase = iRandom(1);
	if (newTexBase) {
	  texBaseAddr = sstFbMemRalloc(dstOffset+cmdp->fbOffset);
	  texBaseAddr &= ~0xF;			
	}
	// linear space
	ttemp = (diago.maxTrashMem - texBaseAddr > 0x1FFFFF) ? 0x1FFFFF :
	  (diago.maxTrashMem - texBaseAddr);
	cmdp->fbOffset = rRandom(2,ttemp-dstOffset);
	cmdp->fbOffset &= ~0x1;
	break;
      case SSTCP_LFB_SPACE:
	cmdp->fbOffset = sstFbMemRalloc(dstOffset);
	if (cmdp->fbOffset < tiledOffsetBytes  &&
	    cmdp->fbOffset+dstOffset >= tiledOffsetBytes) {
	  down = iRandom(1);
	  if (down) down = (tiledOffsetBytes > dstOffset);
	  else down = sstTrashMem(tiledOffsetBytes,dstOffset);
	  if (down) 
	    cmdp->fbOffset = tiledOffsetBytes - dstOffset;
	  else 
	    cmdp->fbOffset = tiledOffsetBytes;
	}
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
	} // check if offset blows out stride
	while (((cmdp->fbOffset & SST_YUV_ADDR_X) >> SST_YUV_ADDR_X_SHIFT) + cmdp->srcWidth 
	       > YUVSTRIDE); 
	
	xOff = (cmdp->fbOffset & SST_YUV_ADDR_X) >> SST_YUV_ADDR_X_SHIFT;
	yOff = (cmdp->fbOffset & SST_YUV_ADDR_Y) >> SST_YUV_ADDR_Y_SHIFT;
	if (cmdp->fbOffset & YUVSELECT) {
	  xOff <<= 1;
	  yOff <<= 1;
	}
	GDBG_INFO(GLEVEL,"YUV fbOffset %x Xoff %x yoff %x\n",
		  cmdp->fbOffset,xOff,yOff);
	// newYuvBase = iRandom(1);
	newYuvBase = 0;
	if (newYuvBase) {
	  yuvBaseAddr = sstFbMemRalloc(dstOffset);
	  yuvBaseAddr &= ~0xF;  // NYI todo
	}
	break;
      } // case
      
      if (sanity-- == 0) {
	gdbg_printf("WARNING::Diag quitting early\n");
	DIAG_PASS(1);
      }
      
      if (cmdp->space == SSTCP_LFB_SPACE) {
	if (sstTrashMem(cmdp->fbOffset,dstOffset))
	  goto again_sizebytes;
	if (!(cmdp->fbOffset < diagfb.auxBufferAddr &&
	      cmdp->fbOffset + dstOffset >= diagfb.colBufferAddr[0])) 
	  goto done;
      }
      if (cmdp->space == SSTCP_TEXPORT_SPACE) {
	if (sstTrashMem(cmdp->fbOffset,dstOffset+texBaseAddr))
	  goto again_sizebytes;
	if (!(cmdp->fbOffset+texBaseAddr < diagfb.auxBufferAddr &&
	      cmdp->fbOffset+texBaseAddr+dstOffset >= diagfb.colBufferAddr[0])) {
	  if (iRandom(1)) {
	    // set up texture download
	    tt->tex->tMode = texRandomFormat(iRandom(1),iRandom(1),iRandom(1)) 
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
	gdbg_info(2,"planarOff 0x%08x\n",planarOff);
	if ((cmdp->fbOffset & YUVSELECT) != (cmdp->fbOffset + planarOff & YUVSELECT))
	  goto again_sizebytes;
	if (sstTrashMem(yuvBaseAddr,maxOff)) 
	  goto again_sizebytes;
	if (!((yuvBaseAddr & SST_YUV_BASE_ADDR) < diagfb.auxBufferAddr &&  
	      (yuvBaseAddr & SST_YUV_BASE_ADDR) + maxOff >= diagfb.colBufferAddr[0])) {
	  if (newYuvBase) {
	    SET(sstc->yuvBaseAddr,yuvBaseAddr & SST_YUV_BASE_ADDR);
	    GDBG_INFO(1,"Tex Base Addr 0x%08x\n",texBaseAddr);
	  }
	  goto done;
	}
      }
      // failed visible test
      goto again_visible;
    done:
      cmdp->id = iRandom(1);
      
      if (selectpType ) {
	if (pType == '5') pp = 1;
	else pp = 0;
      }
      else if (diago.writeFifo) 
	pp = iRandom(1);
      else pp = 0;

      switch(cmdp->space) {
      case SSTCP_3DLFB_SPACE:
	start3dlfb(sst);
	break;
      case SSTCP_TEXPORT_SPACE:
	break;
      case SSTCP_YUV_SPACE:
	break;
      case SSTCP_LFB_SPACE:
	break;
      }
   
      switch(pp) {
      case 0: 
      if (pass >= debugPass) {
	GDBG_INFO(1,"MOVE %s of size %d bytes from srcAddr {0x%01x,0x%08x} width %d stride %d\n",
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
	GDBG_INFO(1,"Packet 5 %s of size %d bytes from srcAddr {0x%01x,0x%08x} width %d stride %d\n",
		  space_str[cmdp->space],cmdp->sizeBytes,cmdp->baseHigh,cmdp->baseLow,
		  cmdp->srcWidth,cmdp->srcStride);
	GDBG_INFO(1,"\t\t\t\tto dstAddr 0x%08x stride %d\n",cmdp->fbOffset,cmdp->dstStride);
	GDBG_INFO(2,"\t\t\t\tsrcOffset 0x%08x dstOffset 0x%08x\n",srcOffset,dstOffset);
	dstAddr = cmdp->fbOffset;
	p5Addr = dstAddr;
	nWords = 1 + (cmdp->srcWidth - 4 + (dstAddr & 0x3))/4;
	seats = 1 + (cmdp->srcWidth - 4 + (dstAddr & 0x3) + 3)/4;
	if (seats == 1) nWords = 1;
	cWords = MIN(psize,(unsigned int)nWords);
	ww = cWords; 
	remWords = nWords - cWords;
	GDBG_INFO(SANITY,"Initial nwords %d seats %d ; cwords %d remWords %d ww %d\n",nWords,seats,cWords,remWords,ww);
	dstData = 0;
	srcData = AGPRDP(cmdp->baseHigh,(cmdp->baseLow & ~0x3));
	for (ii=0,srcAddrHigh = cmdp->baseHigh,srcAddrLow = cmdp->baseLow,
	       prevLow = cmdp->baseLow,prevHigh = cmdp->baseHigh,	   
	       prevDstAddr = cmdp->fbOffset;
	     (unsigned int)ii<cmdp->sizeBytes;ii++) {
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
	      cWords = MIN(psize,(unsigned int)remWords);
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
	    cWords = MIN(psize,(unsigned int)nWords);
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

      pt = NULL;
      if (iRandom(1) && pass > 2) {
	for (ii=0;(unsigned int)ii<iRandom(4);ii++) {
	  int tsize;
	  
	  tsize = rRandom(5,50);
	  pt = t;
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
	  for (nn = 0;(unsigned int)nn < cmdp->sizeBytes;nn++) {
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
	  for (nn = 0;(unsigned int)nn < cmdp->sizeBytes;nn++) {
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
    DIAG_PASS(0);	 
}





