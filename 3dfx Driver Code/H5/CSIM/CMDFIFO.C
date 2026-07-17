#include "vxd.h"

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
** $Date: 10/11/00 8:08:38 PM$
*/

#include <assert.h> 
#include <stdlib.h>

#include <h3.h>
#include "h3sim.h"
#ifdef __SST2_H__
#include "sst2asm.h"
#else
#include "h3asm.h"
#endif

static char *space_str[] = {"LFB","YUV","3DLFB","TEX"};

static void csimCmdFifoRawLfbWrite(SstRegs *sst, FxU32 address, FxU32 data, FxU32 nBytes);

static FxU32 cmdReadMem32(SstRegs *sst,CmdFifo *fifo,FxU32 addr)
{
  CsimPrivate *cpriv = CSIM_PRIVATE(sst);
  FxU32 data;
  if (fifo->baseSize & SST_CMDFIFO_AGP) 
    data = AGPRDV(*(FxU32 *)addr);
  else
    data = csimReadMem32(CSIM_PRIVATE(sst),addr);
  GDBG_INFO(125,"cmdReadMem32(%d) 0x%x 0x%x \n",(&cpriv->cmd.cmdFifo0 == fifo ? 0:1),addr,data);
  return(data);
}

static void packet0(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv, FxU32 data,int pass)
{
    FxU32 addr;
    static FxU32 returnAddress;
    static FxU32 agpAddr; // hack !!
    switch(pass) {
    case 0:
      fifopriv->registerPtr = 0xdeadbeef;
      fifopriv->wordCount = 0;
      if ((data & (SSTCP_PKT0_FUNC|SSTCP_PKT)) != SSTCP_PKT0_JMP_AGP)  {
	fifopriv->bogusCount = (data & SSTCP_BOGUS_WORDS)>>SSTCP_BOGUS_WORDS_SHIFT;
	fifopriv->state = BOGUS_DISCARD;
	if (fifopriv->bogusCount) {
	  GDBG_ERROR("packet0", "invalid bogus count, must be 0\n");
	  fifopriv->bogusCount = 0;
	}
      }
      switch (data & (SSTCP_PKT0_FUNC|SSTCP_PKT)) {
      case SSTCP_PKT0_NOP:
	GDBG_INFO(125,"NOP packet  bogus=%d\n",fifopriv->bogusCount);
	break;
      case SSTCP_PKT0_JMP_LOCAL:
	addr = (SSTCP_PKT0_ADDR & data) >> (SSTCP_PKT0_ADDR_SHIFT-2);
	GDBG_INFO(125,"JMP_LOCAL packet  bogus=%d\n",fifopriv->bogusCount);
	if (addr != (fifo->baseAddrL<<12))
	  GDBG_ERROR("packet0", "invalid jump address: 0x%x\n",addr);
	fifo->readPtrL = addr;
	break;
      case SSTCP_PKT0_JSR:
	if (returnAddress) {
	  GDBG_ERROR("packet0", "invalid JSR, already in a JSR\n");
	}
	returnAddress = 1;
	fifopriv->state = NEW_PACKET;
	addr = (SSTCP_PKT0_ADDR & data) >> (SSTCP_PKT0_ADDR_SHIFT-2);
	GDBG_INFO(125,"JSR packet  addr=0x%x  bogus=%d\n",addr,fifopriv->bogusCount);
	while (returnAddress) {
	  csimCmdFifoExec(sst,fifo,fifopriv,cmdReadMem32(sst,fifo,
	     (fifo->baseSize & SST_CMDFIFO_AGP) ? (FxU32)agpPhysToVirt(fifo->baseAddrL >> 20,addr)
							 : (FxU32)addr));
	  addr += 4;
	}
	break;
      case SSTCP_PKT0_RET:
	if (returnAddress == 0) {
	  GDBG_ERROR("packet0", "invalid RET, not in a JSR\n");
	}
	returnAddress = 0;
	GDBG_INFO(125,"RET packet\n");
	break;
      case SSTCP_PKT0_JMP_AGP:
	addr = (SSTCP_PKT0_ADDR & data) >> (SSTCP_PKT0_ADDR_SHIFT-2);
	fifopriv->wordCount = 1;
	GDBG_INFO(125,"JMP_AGP packet  bogus=%d %x\n",fifopriv->bogusCount,addr);
	if (addr != ((fifo->baseAddrL<<12) & SST_MASK(25)))
	  GDBG_ERROR("packet0", "invalid jump address: 0x%x\n",addr);
	agpAddr = addr;
	
	break;
      default:
	GDBG_ERROR("packet0", "invalid func field\n");
	break;
      }
      break;
    case 1:
      fifopriv->wordCount = 0;
      fifopriv->bogusCount = (data & SSTCP_BOGUS_WORDS)>>SSTCP_BOGUS_WORDS_SHIFT;
      fifopriv->state = BOGUS_DISCARD;
      if (fifopriv->bogusCount) {
	GDBG_ERROR("packet0", "invalid bogus count, must be 0\n");
	fifopriv->bogusCount = 0;
      }
      fifopriv->wordCount = 1;
      if (data != (fifo->baseAddrL>>13))
	GDBG_ERROR("packet0", "invalid jump address");
      fifo->readPtrL = (data << 25) | agpAddr;
      fifo->readPtrH = (data >> 7);
      GDBG_INFO(125,"JMP_AGP packet  bogus=%d L0x%08x H0x%04x\n",fifopriv->bogusCount,
		fifo->readPtrL,fifo->readPtrH);
      break;
    }
}

static void packet1(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv, FxU32 data)
{
    fifopriv->registerPtr = ((data & SSTCP_REGBASE) >> SSTCP_REGBASE_SHIFT)*4;
    fifopriv->registerPtr += (data & SSTCP_PKT1_2D) ? SST_2D_OFFSET : SST_3D_OFFSET;
    fifopriv->wordCount = (data & SSTCP_PKT1_NWORDS) >> SSTCP_PKT1_NWORDS_SHIFT;
    fifopriv->bogusCount = 0;
    GDBG_INFO(125,"P1 packet %d words @ 0x%x\n",fifopriv->wordCount,fifopriv->registerPtr);
    if (fifopriv->wordCount == 0)
	GDBG_ERROR("packet1","0 length packet is illegal\n");
}

static void packet2(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv, FxU32 data)
{
    fifopriv->registerPtr = SST_2D_OFFSET + CLIP0MIN;
    fifopriv->wordCount = (data & SSTCP_PKT2_MASK) >> SSTCP_PKT2_MASK_SHIFT;
    fifopriv->bogusCount = 0;
    GDBG_INFO(125,"P2 packet mask=%x @ 0x%x\n",fifopriv->wordCount,fifopriv->registerPtr);
    if (fifopriv->wordCount == 0)
	GDBG_ERROR("packet2","0 length packet is illegal\n");
}

static void packet3(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv, FxU32 data)
{
    static char *cmd_str[3] = {"BDDBDD","BDDDDD","DDDDDD"};
    int n;
    FxU32 smode;
    FxU32 sstbase = (FxU32)(CSIM_PRIVATE(sst)->info->virtAddr[0]) - SST_3D_OFFSET;

    fifopriv->registerPtr = SST_3D_OFFSET + SSETUPMODE;
    fifopriv->vertexLimit = (data & SSTCP_PKT3_NUMVERTEX) >> SSTCP_PKT3_NUMVERTEX_SHIFT;
    fifopriv->bogusCount = (data & SSTCP_BOGUS_WORDS)>>SSTCP_BOGUS_WORDS_SHIFT;
    smode = 	((data & SSTCP_PKT3_SMODE)>>(SSTCP_PKT3_SMODE_SHIFT-16)) | 
		((data & SSTCP_PKT3_PMASK) >> SSTCP_PKT3_PMASK_SHIFT);
    GDBG_INFO(125,"P3 packet %s (%s) verts=%d  sSetupMode = 0x%x  bogus=%d\n",
		data & SSTCP_PKT3_PACKEDCOLOR ? "pARGB" : "",
		cmd_str[(data&SSTCP_PKT3_CMD)>>SSTCP_PKT3_CMD_SHIFT],
	fifopriv->vertexLimit,smode,fifopriv->bogusCount);
    if (fifopriv->vertexLimit == 0)
	GDBG_ERROR("packet3","0 vertex packet is illegal\n");
    fifopriv->wordCount = n=1;			// always start at 1
    fifopriv->vertexCount = 0;
    fifopriv->vertexData[n++] = SVX;		// always requires X,Y
    fifopriv->vertexData[n++] = SVY;
    csimStore32(sst,sstbase+SST_3D_OFFSET+SSETUPMODE,smode);

    if (sst->sSetupMode & SST_SETUP_RGB) {
	if (data & SSTCP_PKT3_PACKEDCOLOR) {
	    fifopriv->vertexData[n++] = SARGB;
	}
	else {
	    fifopriv->vertexData[n++] = SRED;
	    fifopriv->vertexData[n++] = SGREEN;
	    fifopriv->vertexData[n++] = SBLUE;
	}
    }
    if (sst->sSetupMode & SST_SETUP_A) {
	if (data & SSTCP_PKT3_PACKEDCOLOR) {
	    if ((sst->sSetupMode & SST_SETUP_RGB)==0) {
		fifopriv->vertexData[n++] = SARGB;
	    }
	}
	else
	    fifopriv->vertexData[n++] = SALPHA;
    }
    if (sst->sSetupMode & SST_SETUP_Z) {
	fifopriv->vertexData[n++] = SVZ;
    }
    if (sst->sSetupMode & SST_SETUP_Wfbi) {
	fifopriv->vertexData[n++] = SOOWFBI;
    }
    if (sst->sSetupMode & SST_SETUP_W0) {
	fifopriv->vertexData[n++] = SOOW0;
    }
    if (sst->sSetupMode & SST_SETUP_ST0) {
	fifopriv->vertexData[n++] = SSOW0;
	fifopriv->vertexData[n++] = STOW0;
    }
    if (sst->sSetupMode & SST_SETUP_W1) {
	fifopriv->vertexData[n++] = SOOW1;
    }
    if (sst->sSetupMode & SST_SETUP_ST1) {
	fifopriv->vertexData[n++] = SSOW1;
	fifopriv->vertexData[n++] = STOW1;
    }
    fifopriv->vertexData[n] = 0;
    if (n >= sizeof(fifopriv->vertexData)/sizeof(fifopriv->vertexData[0]))
	GDBG_ERROR("packet3","fifopriv->vertexData overflow\n");
}

static void packet4(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv, FxU32 data)
{
    fifopriv->registerPtr = ((data & SSTCP_REGBASE) >> SSTCP_REGBASE_SHIFT)*4;
    fifopriv->registerPtr += (data & SSTCP_PKT4_2D) ? SST_2D_OFFSET : SST_3D_OFFSET;
    fifopriv->wordCount = (data & SSTCP_PKT4_MASK) >> SSTCP_PKT4_MASK_SHIFT;
    fifopriv->bogusCount = (data & SSTCP_BOGUS_WORDS)>>SSTCP_BOGUS_WORDS_SHIFT;
    GDBG_INFO(125,"P4 packet mask=%x @ 0x%x  bogus=%d\n",
		fifopriv->wordCount,fifopriv->registerPtr,fifopriv->bogusCount);
    if (fifopriv->wordCount == 0)
	GDBG_ERROR("packet4","0 length packet is illegal\n");
}

static void packet5(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv, FxU32 data)
{
    fifopriv->wordCount = (data & SSTCP_PKT5_NWORDS) >> SSTCP_PKT5_NWORDS_SHIFT;
    fifopriv->vertexCount = 0;	// state indicator
    fifopriv->registerPtr = 0;
    fifopriv->bogusCount = 0;
    GDBG_INFO(125,"P5 packet %d words, %s\n", fifopriv->wordCount,
		space_str[(data&SSTCP_PKT5_SPACE)>>SSTCP_PKT5_SPACE_SHIFT]);
    if (fifopriv->wordCount == 0)
	GDBG_ERROR("packet5","0 length packet is illegal\n");
}
//
// Move command functions
//
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

FxU32 h3IncYRamAddr(FxU32 offset,FxU32 tileBegin,FxU32 tileStride,FxU32 stride)
{
  FxU32 rOffset;
  if (offset < tileBegin << 12) 
    rOffset = offset + stride;
  else {
    switch(tileStride) {
    case 0:
      rOffset = offset + (1 << 10);
      break;
    case 1:
      rOffset = offset + (1 << 11);
      break;
    case 2:
      rOffset = offset + (1 << 12);
      break;
    case 3:
      rOffset = offset + (1 << 13);
      break;
      
    }
  }
  return(rOffset);
}

FxU32 h3PciToRamAddr(FxU32 offset,FxU32 tileBegin,FxU32 tileCompareBegin,FxU32 tileStride,FxU32 tilesX)
{
  FxU32 rOffset,page;
  FxU32 xOff,yOff;

  if (offset < tileCompareBegin << 12) 
    rOffset = offset;
  else {
    if (tileStride > 3)
      GDBG_ERROR("h3PciToRamAddress","Illegal tile stride %d\n",tileStride);
    offset &= SST_MASK(27);
    offset = offset - (tileBegin << 12);
    switch(tileStride) {
    case 0:
      xOff = offset & SST_MASK(10);yOff = offset >> 10;
      break;
    case 1:
      xOff = offset & SST_MASK(11);yOff = offset >> 11;
      break;
    case 2:
      xOff = offset & SST_MASK(12);yOff = offset >> 12;
      break;
    case 3:
      xOff = offset & SST_MASK(13);yOff = offset >> 13;
      break;
    }
    page = (yOff/32)*tilesX + xOff/128;
    rOffset = ((page & 0x3FFF) << 12) + (xOff&0x7f) + (yOff&31)*128;
  }
  GDBG_INFO(199,"offset %x tileBegin %d tileStride %d tilesX %d rOffset %x\n",
	    offset,tileBegin,tileStride,tilesX,rOffset);
  return(rOffset);
}

void csimExecuteMoveCmd(SstRegs *sst)
{
  FxU32 sstBase = (FxU32)(CSIM_PRIVATE(sst)->info->virtAddr[0]) - SST_3D_OFFSET;
  FxU32 ii,srcData,srcAddrLow,srcAddrHigh,prevLow,prevHigh;
  FxU32 tileBegin,tileCompareBegin,tileStride,tilesX;
  FxU32 dstAddr,prevDstAddr,ramAddr;
  FxU32 dstData;
  FxU32 lfbsize;

  AGPMOVECMD cmd;
  AGPMOVECMD *cmdp = &cmd;
  CsimPrivate *cpriv = CSIM_PRIVATE(sst);
  SstIORegs *sstio = &cpriv->io;

  cmdp->sizeBytes = cpriv->cmd.agpReqSize;
  cmdp->baseLow = cpriv->cmd.hostAddrLow;
  cmdp->baseHigh = (cpriv->cmd.hostAddrHigh &SST_AGP_SRC_BASEHIGH) >> SST_AGP_SRC_BASEHIGH_SHIFT ;
  cmdp->srcWidth = cpriv->cmd.hostAddrHigh &SST_AGP_SRC_WIDTH;
  cmdp->srcStride = (cpriv->cmd.hostAddrHigh &SST_AGP_SRC_STRIDE) >> SST_AGP_SRC_STRIDE_SHIFT;
  cmdp->fbOffset = cpriv->cmd.graphicsAddr & SST_AGP_FRAME_BUFFER_OFFSET ;
  cmdp->dstStride = cpriv->cmd.graphicsStride & SST_AGP_DSTSTRIDE;
  cmdp->id = (cpriv->cmd.moveCMD & SST_AGPMOVE_CMDID) >> SST_AGPMOVE_CMDID_SHIFT;
  cmdp->space = cpriv->cmd.moveCMD & SST_AGPMOVE_SPACE;

  if (cmdp->sizeBytes % cmdp->srcWidth)
    GDBG_ERROR("moveCmd","Size %d is not a multiple of width %d\n",cmdp->sizeBytes,cmdp->srcWidth);
    
  GDBG_INFO(125,"%d: AGP %s move of size %d bytes at L0x%x H0x%x; src width %d stride %d \n\tframe buffer offset 0x%x dst stride 0x%x\n", 
	    cmdp->id,space_str[cmdp->space >> SST_AGPMOVE_SPACE_SHIFT], 
	    cmdp->sizeBytes,cmdp->baseLow,cmdp->baseHigh,
	    cmdp->srcWidth,cmdp->srcStride,cmdp->fbOffset,
	    cmdp->dstStride);
  if (cmdp->srcWidth > cmdp->srcStride)
    GDBG_ERROR("moveCmd","Width %d is greater than stride %d\n",cmdp->srcWidth,cmdp->srcStride);
  if (cmdp->srcWidth > cmdp->dstStride)
    GDBG_ERROR("moveCmd","Src width %d is bigger than dst stride %d\n",cmdp->srcWidth,
	       cmdp->dstStride);
    
  switch(cmdp->space) {
  case SST_AGPMOVE_LFB:
    tileBegin = SST_RAW_LFB_TILE_BEGIN_PAGE_UNMUNGE(sstio->lfbMemoryTileCtrl & SST_RAW_LFB_TILE_BEGIN_PAGE);

    //This determines the line at which rawlfb read/writes are considered to be tiled
    //tileBegin is the actual page where the tiled space begins
    if(sstio->lfbMemoryTileCompare & LFB_MEMORY_TILE_COMPARE_USE_TILE_COMPARE)
      tileCompareBegin = ((sstio->lfbMemoryTileCompare & LFB_MEMORY_TILE_COMPARE_TILE_BEGIN_PAGE) >>
			  LFB_MEMORY_TILE_COMPARE_TILE_BEGIN_PAGE_SHIFT);
    else
      tileCompareBegin = tileBegin;

    if(tileCompareBegin < tileBegin)
      GDBG_ERROR("csimExecuteMoveCmd", "Oh shizit! tileCompareBegin < tileBegin! %s(%d)\n", __FILE__, __LINE__);
          
    tileStride =  (sstio->lfbMemoryTileCtrl & SST_RAW_LFB_ADDR_STRIDE) >> SST_RAW_LFB_ADDR_STRIDE_SHIFT;
    tilesX = (sstio->lfbMemoryTileCtrl & SST_RAW_LFB_TILE_STRIDE ) >> SST_RAW_LFB_TILE_STRIDE_SHIFT;

    srcData = AGPRDP(cmdp->baseHigh,(cmdp->baseLow & ~0x3));
    for (ii=0,srcAddrHigh = cmdp->baseHigh,srcAddrLow = cmdp->baseLow,
	   prevLow = cmdp->baseLow,prevHigh = cmdp->baseHigh,	   
	   dstAddr = cmdp->fbOffset,prevDstAddr = cmdp->fbOffset;
	 ii<cmdp->sizeBytes;ii++) {
      if ((srcAddrLow & 0x3) == 0) {
	srcData = AGPRDP(srcAddrHigh,srcAddrLow);
	GDBG_INFO(150,"agp Move src data 0x%x at L0x%x H0x%x\n",srcData,srcAddrLow,srcAddrHigh);
      }
      ramAddr = h3PciToRamAddr(dstAddr,tileBegin,tileCompareBegin,tileStride,tilesX);
      dstData = srcData >> ((srcAddrLow&3) << 3);

      GDBG_INFO(150,"agp Move data 0x%x to ramAddr 0x%x\n",dstData,ramAddr);
      csimWriteMem8(cpriv,ramAddr,dstData);
      if (srcAddrLow == 0xFFFFFFFF)
	GDBG_ERROR("moveCmd","Base address overflow NYI");
      srcAddrLow++;
      dstAddr++;
      if ((srcAddrLow - prevLow) >= cmdp->srcWidth) {
	srcAddrLow = prevLow + cmdp->srcStride;
	prevLow = srcAddrLow;
	prevHigh = srcAddrHigh;
	dstAddr = h3IncYRamAddr(prevDstAddr,tileBegin,tileStride,cmdp->dstStride);
	prevDstAddr = dstAddr;
	srcData = AGPRDP(cmdp->baseHigh,(srcAddrLow & ~0x3));
      }
      // assert dstStride matches pci stride
    }
    break;
  case SST_AGPMOVE_YUV:
    sstBase += SST_YUV_OFFSET;
    if (cmdp->srcWidth & 0x3 || cmdp->fbOffset & 0x3 || cmdp->baseLow & 0x3) 
      GDBG_ERROR("moveCmd","YUV Non DWORD aligned NYI  %x %x %x\n",cmdp->srcWidth,cmdp->fbOffset,cmdp->baseLow);
    srcData = AGPRDP(cmdp->baseHigh,(cmdp->baseLow & ~0x3));
    for (ii=0,srcAddrHigh = cmdp->baseHigh,srcAddrLow = cmdp->baseLow,
	   prevLow = cmdp->baseLow,prevHigh = cmdp->baseHigh,	   
	   dstAddr = cmdp->fbOffset,prevDstAddr = cmdp->fbOffset;
	 ii<cmdp->sizeBytes;ii += 4) {
      if ((srcAddrLow & 0x3) == 0) {
	srcData = AGPRDP(srcAddrHigh,srcAddrLow);
	GDBG_INFO(150,"agp Move src data 0x%x at L0x%x H0x%x\n",srcData,srcAddrLow,srcAddrHigh);
      }
      dstData = srcData >> ((srcAddrLow&3) << 3);

      GDBG_INFO(150,"agp Move data 0x%x to ramAddr 0x%x\n",dstData,dstAddr);
      csimStore32(sst,sstBase+dstAddr,dstData);
      if (srcAddrLow >= 0xFFFFFFFE)
	GDBG_ERROR("moveCmd","Base address overflow NYI");
      srcAddrLow += 4;
      dstAddr += 4;
      if ((srcAddrLow - prevLow) >= cmdp->srcWidth) {
	srcAddrLow = prevLow + cmdp->srcStride;
	prevLow = srcAddrLow;
	prevHigh = srcAddrHigh;
	dstAddr = prevDstAddr + cmdp->dstStride;
	prevDstAddr = dstAddr;
	srcData = AGPRDP(cmdp->baseHigh,(srcAddrLow & ~0x3));
      }
      // assert dstStride matches pci stride
    }
    break;
  case SST_AGPMOVE_3DLFB:
    sstBase += SST_LFB_OFFSET;
    lfbsize = csimFbiLfbSize(sst);
    // todo:: stride has to be word aligned
    if (cmdp->srcWidth & 0x1 || cmdp->fbOffset & 0x1 || cmdp->baseLow & 0x1) 
      GDBG_ERROR("moveCmd","3DLFB Non word aligned  %x %x %x\n",cmdp->srcWidth,cmdp->fbOffset,cmdp->baseLow);
    srcData = AGPRDP(cmdp->baseHigh,(cmdp->baseLow & ~0x3));
    for (ii=0,srcAddrHigh = cmdp->baseHigh,srcAddrLow = cmdp->baseLow,
	   prevLow = cmdp->baseLow,prevHigh = cmdp->baseHigh,	   
	   dstAddr = cmdp->fbOffset,prevDstAddr = cmdp->fbOffset;
	 ii<cmdp->sizeBytes;ii += lfbsize) {
      if ((srcAddrLow & 0x3) == 0) {
	srcData = AGPRDP(srcAddrHigh,srcAddrLow);
	GDBG_INFO(150,"agp Move src data 0x%x at L0x%x H0x%x\n",srcData,srcAddrLow,srcAddrHigh);
      }
      dstData = srcData >> ((srcAddrLow&3) << 3);

      GDBG_INFO(150,"agp Move data 0x%x to ramAddr 0x%x\n",dstData,dstAddr);
      if (lfbsize == 2)
	csimStore16(sst,sstBase+dstAddr,(FxU16)dstData);
      else
	csimStore32(sst,sstBase+dstAddr,(FxU32)dstData);
	
      if (srcAddrLow >= 0xFFFFFFFE)
	GDBG_ERROR("moveCmd","Base address overflow NYI");
      srcAddrLow += lfbsize;
      dstAddr += lfbsize;
      if ((srcAddrLow - prevLow) >= cmdp->srcWidth) {
	srcAddrLow = prevLow + cmdp->srcStride;
	prevLow = srcAddrLow;
	prevHigh = srcAddrHigh;
	dstAddr = prevDstAddr + cmdp->dstStride;
	prevDstAddr = dstAddr;
	srcData = AGPRDP(cmdp->baseHigh,(srcAddrLow & ~0x3));
      }
      // assert dstStride matches pci stride
    }
    break;
  case SST_AGPMOVE_TEXPORT:
    //The hardware is f'ed up. Consequently, the csim needs to
    //be f'ed up and do the wrong thing.
    //sstBase += SST_TEX_OFFSET;
    srcData = AGPRDP(cmdp->baseHigh,(cmdp->baseLow & ~0x3));
    for (ii=0,srcAddrHigh = cmdp->baseHigh,srcAddrLow = cmdp->baseLow,
	   prevLow = cmdp->baseLow,prevHigh = cmdp->baseHigh,	   
	   dstAddr = cmdp->fbOffset,prevDstAddr = cmdp->fbOffset;
	 ii<cmdp->sizeBytes;ii += 1) {
      if ((srcAddrLow & 0x3) == 0) {
	srcData = AGPRDP(srcAddrHigh,srcAddrLow);
	GDBG_INFO(150,"agp Move src data 0x%x at L0x%x H0x%x\n",srcData,srcAddrLow,srcAddrHigh);
      }
      dstData = srcData >> ((srcAddrLow&3) << 3);

      GDBG_INFO(150,"agp Move data 0x%x to ramAddr 0x%x\n",dstData,dstAddr);
      csimStore8(sst,sstBase+dstAddr,(FxU8)dstData);
      if (srcAddrLow >= 0xFFFFFFFE)
	GDBG_ERROR("moveCmd","Base address overflow NYI");
      srcAddrLow++;
      dstAddr++;
      if ((srcAddrLow - prevLow) >= cmdp->srcWidth) {
	srcAddrLow = prevLow + cmdp->srcStride;
	prevLow = srcAddrLow;
	prevHigh = srcAddrHigh;
	dstAddr = prevDstAddr + cmdp->dstStride;
	prevDstAddr = dstAddr;
	srcData = AGPRDP(cmdp->baseHigh,(srcAddrLow & ~0x3));
      }
      // assert dstStride matches pci stride
    }
    break;
  }
}

static void packet6(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv, FxU32 data)
{
  CsimPrivate *cpriv = CSIM_PRIVATE(sst);
  fifopriv->wordCount = 4;
  fifopriv->vertexCount = 0;	// state indicator
  fifopriv->registerPtr = 0;
  fifopriv->bogusCount = 0;
  cpriv->cmd.agpReqSize = (data  & SSTCP_PKT6_NBYTES) >> SSTCP_PKT6_NBYTES_SHIFT;
  cpriv->cmd.moveCMD = (data & SSTCP_PKT6_SPACE) | 
    (((&cpriv->cmd.cmdFifo0 == fifo) ? 0 : 1) << SST_AGPMOVE_CMDID_SHIFT) & 
     SST_AGPMOVE_CMDID;
  GDBG_INFO(125,"P6 packet %d bytes, %s\n", cpriv->cmd.agpReqSize,
	    space_str[(data&SSTCP_PKT6_SPACE)>>SSTCP_PKT6_SPACE_SHIFT]);
  /*
  */
}

//----------------------------------------------------------------------
// execute a word from the command fifo region
//----------------------------------------------------------------------
void csimCmdFifoExec(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv, FxU32 data)
{
    FxU32 sstbase = (FxU32)(CSIM_PRIVATE(sst)->info->virtAddr[0]) - SST_3D_OFFSET;
    FxU32 chipindex;
    CsimPrivate *cpriv = CSIM_PRIVATE(sst);

    GDBG_INFO(124,"\t\texec(%d) @ %x : %x\n", (&cpriv->cmd.cmdFifo0 == fifo ? 0:1),
	      fifo->readPtrL-4, data);
    
    for(chipindex=0; chipindex<halInfo.boardsFound; chipindex++)
      CSIM_PRIVATE(halInfo.boardInfo[chipindex].sstCSIM)->inCmdFifoExecMode = 1;

    switch (fifopriv->state) {
	case BOGUS_DISCARD:
	    GDBG_INFO(125,"\t\t  discard %d\n",fifopriv->bogusCount);
	    fifopriv->bogusCount--;
	    break;
	case NEW_PACKET:	// new packet word
	    fifopriv->packetWord = data;
	    fifopriv->state = data & SSTCP_PKT;
	    switch(fifopriv->state) {
		case SSTCP_PKT0:
		    packet0(sst,fifo,fifopriv,data,0);
		    break;
		case SSTCP_PKT1:
		    packet1(sst,fifo,fifopriv,data);
		    break;
		case SSTCP_PKT2:
		    packet2(sst,fifo,fifopriv,data);
		    break;
		case SSTCP_PKT3:
		    packet3(sst,fifo,fifopriv,data);
		    break;
		case SSTCP_PKT4:
		    packet4(sst,fifo,fifopriv,data);
		    break;
		case SSTCP_PKT5:
		    packet5(sst,fifo,fifopriv,data);
		    break;
		case SSTCP_PKT6:
		    packet6(sst,fifo,fifopriv,data);
		    break;
		default:
		    GDBG_ERROR("csimCmdFifoExec", "invalid packet type %d\n",data & SSTCP_PKT);
		    break;
	    }
	    break;

	case 0:
	    if (fifopriv->wordCount == 0)
	     GDBG_ERROR("csimCmdFifoExec", "bad internal state\n");
	    else {
	      packet0(sst,fifo,fifopriv,data,1);
	      fifopriv->wordCount--;
	    }
	  
	    break;

	case 1:
	// XXX calling csimStore32 will mess up the replay.exe tool since the write is
	// XXX output twice to the LOG file, once as a CMDFIFO write and once as a register write
	    csimStore32(sst,sstbase+fifopriv->registerPtr,data);
	    if (fifopriv->packetWord & SSTCP_INC)
		fifopriv->registerPtr += 4;
	    fifopriv->wordCount--;
	    break;

	case 3:
	    fifopriv->registerPtr = fifopriv->vertexData[fifopriv->wordCount] + SST_3D_OFFSET;
	    csimStore32(sst,sstbase+fifopriv->registerPtr,data);// write this reg
	    fifopriv->wordCount++;				// move onto next datum

	    if (fifopriv->vertexData[fifopriv->wordCount] == 0) {	// if done
		FxU32 reg;
		// all done with vertex, now outut command
		reg = SDRAWTRICMD;
		switch (fifopriv->packetWord & SSTCP_PKT3_CMD) {
		    case SSTCP_PKT3_BDDBDD:
			if ((fifopriv->vertexCount%3) == 0)
			    reg = SBEGINTRICMD;
			break;
		    case SSTCP_PKT3_BDDDDD:
			if (fifopriv->vertexCount == 0)
			    reg = SBEGINTRICMD;
			break;
		}
		csimStore32(sst,sstbase+SST_3D_OFFSET+reg,0);
		if (++fifopriv->vertexCount == fifopriv->vertexLimit) {
		    fifopriv->wordCount = 0;
		    fifopriv->state = BOGUS_DISCARD;
		}
		fifopriv->wordCount = 1;
	    }
	    break;

	case 2:
	case 4:
	    while ((fifopriv->wordCount & 1)==0) {		// find first one
		fifopriv->registerPtr += 4;
		fifopriv->wordCount >>= 1;
	    }
	    csimStore32(sst,sstbase+fifopriv->registerPtr,data);// write this reg
	    fifopriv->wordCount >>= 1;			// move onto next bit
	    fifopriv->registerPtr += 4;
	    if (fifopriv->wordCount == 0)
		fifopriv->state = BOGUS_DISCARD;
	    break;

	case 5:
	    if (fifopriv->vertexCount == 0) {		// if 1st word then get address
		fifopriv->registerPtr = data & SSTCP_PKT5_BASEADDR;
		fifopriv->vertexCount = 0;
	    }
	    else {
		FxU32 bmask = 0x0;
		if (fifopriv->vertexCount == 1) {		// word 2, apply byte masks
		    bmask |= (fifopriv->packetWord & SSTCP_PKT5_BYTEN_W2)>>SSTCP_PKT5_BYTEN_W2_SHIFT;
		}
		else if (fifopriv->wordCount == 1) {	// word N, apply byte masks
		    bmask |= (fifopriv->packetWord & SSTCP_PKT5_BYTEN_WN)>>SSTCP_PKT5_BYTEN_WN_SHIFT;
		}
		switch (fifopriv->packetWord & SSTCP_PKT5_SPACE) {
		    case SSTCP_PKT5_LFB:
		      sstbase += SST_RAW_LFB_OFFSET;
			if (~bmask & 0x1)
			  csimCmdFifoRawLfbWrite( sst,sstbase+fifopriv->registerPtr, data & 0xFF,1);
			if (~bmask & 0x2)
			  csimCmdFifoRawLfbWrite( sst,sstbase+fifopriv->registerPtr+1,data >> 8 & 0xFF,1);
			if (~bmask & 0x4)
			  csimCmdFifoRawLfbWrite( sst,sstbase+fifopriv->registerPtr+2,data >> 16 & 0xFF,1);
			if (~bmask & 0x8)
			  csimCmdFifoRawLfbWrite( sst,sstbase+fifopriv->registerPtr+3,data >> 24 & 0xFF,1);
			break;
		    case SSTCP_PKT5_YUV:
			sstbase += SST_YUV_OFFSET;
			goto dobmask;
		    case SSTCP_PKT5_3DLFB:
			sstbase += SST_LFB_OFFSET;
			goto dobmask;
		    case SSTCP_PKT5_TEXPORT:
		      sstbase += SST_TEX_OFFSET;    
		      
			if (~bmask & 0x1)
			  csimStore8( sst,sstbase+fifopriv->registerPtr, (FxU8)(data & 0xFF));
			if (~bmask & 0x2)
			  csimStore8( sst,sstbase+fifopriv->registerPtr+1,(FxU8)((data >> 8) & 0xFF));
			if (~bmask & 0x4)
			  csimStore8( sst,sstbase+fifopriv->registerPtr+2,(FxU8)(data >> 16 & 0xFF));
			if (~bmask & 0x8)
			  csimStore8( sst,sstbase+fifopriv->registerPtr+3,(FxU8)(data >> 24 & 0xFF));
			break;
		    dobmask:
			switch (bmask) {	// note bmask is active low
			    case 0x0:		// write all 4 bytes
				csimStore32(sst,sstbase+fifopriv->registerPtr,data);
				break;
			    case 0x3:		// write upper word
				csimStore16(sst,sstbase+fifopriv->registerPtr+2,(FxU16)(data>>16));
				break;
			    case 0xC:		// write lower word
				csimStore16(sst,sstbase+fifopriv->registerPtr,(FxU16)data);
				break;
			    case 0xE:           // write lowest byte
			        csimStore8(sst,sstbase+fifopriv->registerPtr,(FxU8)(data & 0xFF));
				break;
			    case 0xD:           // write second-lowest byte
			        csimStore8(sst,sstbase+fifopriv->registerPtr+1,(FxU8)(data>>8 & 0xFF));
				break;
			    case 0xB:           // write second-highest byte
			        csimStore8(sst,sstbase+fifopriv->registerPtr+2,(FxU8)(data>>16 & 0xFF));
				break;
			    case 0x7:           // write highest byte
			        csimStore8(sst,sstbase+fifopriv->registerPtr+3,(FxU8)(data>>24 & 0xFF));
				break;
			    default:
				GDBG_ERROR("csimCmdFifoExec", "invalide byte mask 0x%x\n",bmask);
				break;
			}
			break;
		    default:
			GDBG_ERROR("csimCmdFifoExec", "invalid pkt 5 space field\n");
			break;
		}

		fifopriv->registerPtr += 4;
		fifopriv->wordCount--;
	    }
	    fifopriv->vertexCount++;
	    break;
	case 6:
	  switch(fifopriv->vertexCount) {		
	  case 0:
	    cpriv->cmd.hostAddrLow = data;
	    break;
	  case 1:
	    cpriv->cmd.hostAddrHigh = data;
	    break;
	  case 2:
	    cpriv->cmd.graphicsAddr = data;
	    break;
	  case 3:
	    cpriv->cmd.graphicsStride = data;
	    csimExecuteMoveCmd(sst);
	    break;
	    }
	  fifopriv->vertexCount++;
	  fifopriv->wordCount--;
	  break;
	default:
		GDBG_ERROR("csimCmdFifoExec", "bad internal state\n");
		break;
    }
    if (fifopriv->bogusCount == 0) {
	if ((fifopriv->state == BOGUS_DISCARD) || (fifopriv->wordCount==0))
	    fifopriv->state = NEW_PACKET;
    }

    for(chipindex=0; chipindex<halInfo.boardsFound; chipindex++)
      CSIM_PRIVATE(halInfo.boardInfo[chipindex].sstCSIM)->inCmdFifoExecMode = 0;
}

//----------------------------------------------------------------------
// process a write to the command fifo region
// amin,amax - should be in h3defs.h and checked against regs.c
#define SST_IS_AMIN_NEGATIVE(addr) (BIT(24) & (addr))
#define SST_IS_AMAX_NEGATIVE(addr) (BIT(24) & (addr))
#define SST_AMIN(addr)             (SST_MASK(25) & (addr))
#define SST_AMAX(addr)             (SST_MASK(25) & (addr))
//----------------------------------------------------------------------
void csimCmdFifoWrite( SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv,
			FxU32 iaddr, FxU32 data )
{
    CsimPrivate *cp = CSIM_PRIVATE(sst);

    if (iaddr >= (unsigned)cp->memorySizeInBytes) {
	GDBG_ERROR("csimCmdFifoWrite","memory offset 0x%x too large\n",iaddr);
	*(int *)0 = 0;		// cause a fault right away
    }
    if (fifo->baseSize & SST_CMDFIFO_DISABLE_HOLES)	// if hole counting is disabled
	return;						// then return
    // GDBG_INFO(124,"sanity B amax %x amin %x depth %x holecount %x\n",
    //		fifo->aMax,fifo->aMin,fifo->depth,fifo->holeCount);
    GDBG_INFO(124,"\t\tcmdfifo(%d):",(&cp->cmd.cmdFifo0 == fifo ? 0:1));

    if (iaddr > fifo->aMax  || SST_IS_AMAX_NEGATIVE(fifo->aMax)) {
	GDBG_INFO_MORE(124," > %x, ",fifo->aMax);
	if ( SST_IS_AMAX_NEGATIVE(fifo->aMax)) {
	  if (SST_AMAX(fifo->aMax+4) != 0)
	    GDBG_ERROR("csimCmdFifoWrite","fifo->aMax is not -4\n");
	  fifo->holeCount += (iaddr)>>2;
	}
	else 
	  fifo->holeCount += (iaddr - fifo->aMax - 4)>>2;
	GDBG_INFO_MORE(124,"Holes = %d  Depth = %d",fifo->holeCount,fifo->depth);
	fifo->aMax = iaddr;
    }
    else if (iaddr == fifo->aMax) {
	GDBG_INFO_MORE(124,"\n");
	GDBG_ERROR("csimCmdFifoWrite","wrote cmdFifoAmax twice\n");
    }
    else if ((iaddr >= fifo->aMin)
	     || SST_IS_AMIN_NEGATIVE(fifo->aMin)) {
    	fifo->holeCount--;
	GDBG_INFO_MORE(124," < %x, Holes = %d  Depth = %d",fifo->aMax,fifo->holeCount,fifo->depth);
    }
    else if (fifo->holeCount) {
	GDBG_INFO_MORE(124,"\n");
	GDBG_ERROR("csimCmdFifoWrite","CMDFIFO auto wrap, hole count = %d\n",fifo->holeCount);
    }
    else {
	GDBG_INFO_MORE(124, "CMDFIFO auto wrap, ");
	fifo->holeCount += (iaddr - (fifo->baseAddrL<<12))>>2;
	fifo->aMin= (fifo->baseAddrL<<12)-4;
	fifo->aMax = iaddr;
    }

    if (fifo->holeCount == 0) {
      if ( SST_IS_AMIN_NEGATIVE(fifo->aMin)) {
	if (SST_AMIN(fifo->aMin+4) != 0) 
	  GDBG_ERROR("csimCmdFifoWrite","fifo->aMIN is not -4\n");
	fifo->depth += (fifo->aMax + 4)>>2;
      }
      else
	fifo->depth += (fifo->aMax - fifo->aMin)>>2;
      fifo->aMin = fifo->aMax;
      if (fifo->depth) sst->status |= SST_BUSY;
    }
    else sst->status |= SST_BUSY;
    GDBG_INFO_MORE(124,"\n");

    //GDBG_INFO(124,"sanity A amax %x amin %x depth %x holecount %x\n",
    //	fifo->aMax,fifo->aMin,fifo->depth,fifo->holeCount);
    // if a video driver is the client, execute them right away
    if (halInfo.csim < 0)
      csimCmdFifoExecuteN(sst,fifo,fifopriv,fifo->depth);
    else
      csimCmdFifoExecuteSome(sst);
}

//----------------------------------------------------------------------
// execute N commands from the CMD FIFO
//----------------------------------------------------------------------
void csimCmdFifoExecuteN(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv, FxU32 n)
{

    CsimPrivate *cpriv = CSIM_PRIVATE(sst);
    FxU32 readp;
    // now execute some stuff
    while (n-- > 0) {
	FxU32 data;

	// get the next DWORD from the CMD fifo
	cpriv->environment.allowAccessesToCommandFifoRegion = FXTRUE;
	data = cmdReadMem32(sst,fifo,
	     (fifo->baseSize & SST_CMDFIFO_AGP) ? (FxU32)agpPhysToVirt(fifo->readPtrH,fifo->readPtrL)
							 : (FxU32)fifo->readPtrL);
	cpriv->environment.allowAccessesToCommandFifoRegion = FXFALSE;
	fifo->readPtrL += 4;		// bump the read pointer
	if (fifo->readPtrL < 4)
	  fifo->readPtrH++;
	csimCmdFifoExec(sst,fifo,fifopriv,data);
	readp = (fifo->readPtrL>>12) | fifo->readPtrH << 13;
	// compare addresses (in pages)
	if (readp > fifo->baseAddrL + (fifo->baseSize & SST_CMDFIFO_SIZE)+1)
	  {
	    GDBG_INFO(124,"\t\tCMDFIFO(%d) read rollover\n",(&cpriv->cmd.cmdFifo0 == fifo ? 0:1));
	    fifo->readPtrL = fifo->baseAddrL << 12;
	    fifo->readPtrH = fifo->baseAddrL >> 13;
	  }
	fifo->depth--;
	if (fifo->depth == 0)		// recompute busy bits
	    csimRecomputeBusy(sst);
    }
}

//----------------------------------------------------------------------
// this executes some commands from the CMD FIFO if it is enabled
// it gets called whenever the Depth or ReadPtr registers are read (wait for idle)
// or when something is entered into the command fifo
// it purposely lags behind to stress flow control logic
// NOTE: when running with HSIM or HW, the actual hardware's registers
// are read for flow control, so we let the CSIM execute everything immediately
//----------------------------------------------------------------------
void _csimCmdFifoExecuteSome(SstRegs *sst, CmdFifo *fifo, CmdFifoPriv *fifopriv)
{
    static FxU32 randx = 3;
    FxU32 e,n;
    CsimPrivate *cpriv = CSIM_PRIVATE(sst);

    if (fifo->depth == 0) return;
    if (!(fifo->baseSize & SST_EN_CMDFIFO))
	GDBG_ERROR("csimCmdFifoExecuteSome","cmdFifoDepth != 0 and CMDFIFO disabled\n");

    if (halInfo.hsim || halInfo.hw) {	// if running with HSIM or HW
	n = fifo->depth;		// execute them all
    }
    else {	// note we use our own little rand
	e = fifo->depth;
	// sometimes don't execute any
	if (e < 4) n = 0x1000000;
	else if (e < 32) n = 0x3000000;
	else n = 0xF000000;
	if (randx & n) {
	    randx = randx*1103515245 + 12345;
	    n = 0;
	}
	else {
	    if (e < 4) e = 29;
	    else if (e < 8) e = 28;
	    else if (e < 16) e = 27;
	    else if (e < 32) e = 26;
	    else e = 25;
	    do {
		randx = randx*1103515245 + 12345;
		n = (randx & 0x7fffffff) >> e;
	    } while (n > fifo->depth);
	}
}
    GDBG_INFO(124,"\t\texecute(%d) batch of %d, depth=%d\n", 
	      (&cpriv->cmd.cmdFifo0 == fifo ? 0:1),n,fifo->depth);
    csimCmdFifoExecuteN(sst, fifo, fifopriv, n);
}

void csimCmdFifoExecuteSome(SstRegs *sst)
{
    CsimPrivate *cpriv = CSIM_PRIVATE(sst);
    _csimCmdFifoExecuteSome(sst,&cpriv->cmd.cmdFifo0,&cpriv->fifo0data);
    _csimCmdFifoExecuteSome(sst,&cpriv->cmd.cmdFifo1,&cpriv->fifo1data);
}

//This broadcasts to all chips. This is a hopti hack. The
//command fifo junk isn't very modular and doesn't work well
//with multiple chips
void csimCmdFifoRawLfbWrite(SstRegs *sst, FxU32 address, FxU32 data, FxU32 nBytes)
{
  FxU32 chipIndex;
  SstRegs *thisSST;
  CsimPrivate *cp;

  
  //This should only be going to one device (the parent)
  cp=CSIM_PRIVATE(sst);
  assert(cp->environment.parentDevice);

  for(chipIndex=0; chipIndex<halInfo.boardsFound; chipIndex++)
    {
      thisSST = halInfo.boardInfo[chipIndex].sstCSIM;
      csimRawLfbWrite( thisSST, address, data, nBytes);
    }
}
