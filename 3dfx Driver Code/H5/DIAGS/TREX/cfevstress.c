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
** $Date: 10/11/00 8:18:56 PM$
** NYI 
** . tiled texture, YUV
** . dst stride 14:0
** . byte aligned texture    
** . src stride/width 14:0
** . tiled LFB
** . 3DLFB auto checking
** . src memory shadow for non AGP 
** . byte enables for nmlfb
** . VBE rd/wr pass 32K boundary
** . Memory CMDF with pkt 6
** . vbe csim random mimic
** . add video
** . VGA 0xA000,VBE 0xA000
** . Random vga IO reads/writes
** Done
** . NMLFB Read
** . mode 12/3 switching
*/

#include "udiag.h"
#include "sstdiag.h"
#include "../trex/stwtri.h"
#include "fbi.h"
#include "lfbutils.h"

#define MODNAME "cfevstress"
#define ABS(a) ((a) > 0 ? (a) : -(a))
#define GLEVEL 125
#define GWARN 2
#define SANITY 150
#define MAXLOOP 500

static int bigSize = 0;
static int burst = 0;
static int debugPass = -1;
static int sanityPass = -1;
static int pType,vType;
static int selectpType;
static char *space_str[] = {"LFB","YUV","3DLFB","TEX"};
static FxU32 gbpp[] = {
  SSTG_PIXFMT_1BPP, SSTG_PIXFMT_8BPP, SSTG_PIXFMT_15BPP, SSTG_PIXFMT_16BPP,
  SSTG_PIXFMT_24BPP, SSTG_PIXFMT_32BPP, SSTG_PIXFMT_422YUV
};
static SstRegs sRegs;
static SstGRegs sGRegs;
static int selectType;
static int selectInverse;
static int randomConfig;
static int waxStream;
static int waxpDepth;

static long nlfb,nyuv,n3dlfb,ntex,nrect,nline,ntriangle;
static long nplfb,npyuv,np3dlfb,nptex,nprect,npline,nptriangle;
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

enum CSM { TEST_P5, TEST_P6, TEST_WAX0,TEST_WAX1,   TEST_3D ,TEST_IDLE,TEST_VGA };
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
  if (sizeBytes*32 > cpriv->info->agpSizeInBytes) {
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

#include "h3_vga_modes.h"
void VGA_SET_MODE(unsigned mode,unsigned io16);
void 	PCI_IOW8();
void 	PCI_IOW16();
void 	PCI_IOW32();

ulong PCI_IO_READ();
ulong PCI_IOR8();
ulong PCI_IOR16();
ulong PCI_IOR32();
//
#define SST_VGA_VBEWR SST_MASK(10)
#define SST_VGA_VBERD_SHIFT 10
#define SST_VGA_VBERD (SST_MASK(10) << SST_VGA_VBERD_SHIFT)
#define SST_VGA_CHAIN4 BIT(20)
// 
#define VBESIZE (32*1024)
#define VMEMSIZE (2*VBESIZE)
//

static ulong vShadowMem[VMEMSIZE/4];
static ulong vShadowWrBase;
static ulong vShadowRdBase;
static ulong vbeBaseAddr;
static ulong vgaMode;
static FxU32 vgaBase;

// used only to get IOBASE
#include "h3_def.h"
//todo: memory mapped IO XXXX NO more mem mapped vga??
static FxU32 vgaRegRead(FxU32 addr,FxU32 index,FxU32 daddr) {
  int rr;
  FxU32 data;

  rr = iRandom(1);
  if (!diago.halInfo->hsim) return(0xdeadbeef);
  switch(rr) {
  case 0:
    PCI_IOW8( addr, index);
    data = PCI_IOR8(daddr);
    break;
  case 1:
    PCI_IOW8(IOBASE+(addr & 0xFF), index);
    data = PCI_IOR8(IOBASE+(daddr & 0xFF));
    break;
/*
  case 2:
    PCI_MEMW8(MEMBASE0+(addr & 0xFF), index);
    data = PCI_MEMR8(MEMBASE0+(daddr & 0xFF));
    break;
    */
  }
  
  return(data);
}

FxU32 PCI_IOR8(FxU32 addr);
static FxU32 vgaRead8(FxU32 daddr) {
  int rr;
  FxU32 data;
  rr = iRandom(1);
  if (!diago.halInfo->hsim) return(0xdeadbeef);
  switch(rr) {
  case 0:
    data = PCI_IOR8(daddr);
    break;
  case 1:
    data = PCI_IOR8(IOBASE+(daddr & 0xFF));
    break;
  }
  gdbg_printf("vgaRead8 %x\n",data);
  return(data);
}

static void vgaRegWrite(FxU32 addr,FxU32 index,FxU32 daddr,FxU32 data) {
  int rr;
  rr = iRandom(3);
  if (!diago.halInfo->hsim) return;
  switch(rr) {
  case 0:
    PCI_IOW8( addr, index);
    PCI_IOW8(daddr,data);
    break;
  case 1:
    PCI_IOW16( addr, data << 8 | index);
    break;
  case 2:
    PCI_IOW8(IOBASE+(addr & 0xFF), index);
    PCI_IOW8(IOBASE+(daddr & 0xFF),data);
    break;
  case 3:
    PCI_IOW16(IOBASE+(addr & 0xFF), data << 8 | index);
    break;
    // case 4: memory write
  }
}

static void vgaWrite16(FxU32 addr,FxU32 data) {
  int rr;
  rr = iRandom(3);
  if (!diago.halInfo->hsim) return;
  switch(rr) {
  case 0:
    PCI_IOW8( addr, data);
    PCI_IOW8(addr+1,data >> 8);
    break;
  case 1:
    PCI_IOW16( addr, data);
    break;
  case 2:
    PCI_IOW8( IOBASE+(addr & 0xFF), data);
    PCI_IOW8(IOBASE+(addr & 0xFF)+1,data >> 8);
    break;
  case 3:
    PCI_IOW16(IOBASE+(addr & 0xFF), data);
    break;
  }
}

static void vgaWrite8(FxU32 daddr,FxU32 data) {
  int rr;
  rr = iRandom(1);
  if (!diago.halInfo->hsim) return;
  switch(rr) {
  case 0:
    PCI_IOW8(daddr,data);
    break;
  case 1:
    PCI_IOW8(IOBASE+(daddr & 0xFF),data);
    break;
  }
}

// todo: Memorybase0, IO Base0 space also
static void vgaRegTest(SstRegs *sst) 
{
  FxU32 ii,expect;
  SstIORegs *sstio;
  ulong *vga_mode;
  gdbg_printf("vgaRegTest...\n");
  sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
  switch(iRandom(2)) {
  case 0:
  expect = vgaRegRead(0x03B4, iRandom(0x24),0x3B5);
  break;
  case 1:
  expect = vgaRegRead(0x03C4, iRandom(0x4),0x3C5);
  break;
  case 2:
  expect = vgaRegRead(0x03CE, iRandom(0x8),0x3CF);
  break;
  }

  switch (vgaMode) {
  case 0x0:	vga_mode = mode0_200;      break;
  case 0x2:	vga_mode = mode2_480;      break;
  case 0x3:	vga_mode = mode03_200;     break;
  case 0x4:	vga_mode = mode4_200;      break;
  case 0x6:	vga_mode = mode6_200;      break;
  case 0xe:	vga_mode = mode0e_200;     break;
  case 0x10:	vga_mode = mode10_350;     break;
  case 0x12:	vga_mode = mode12_480;     break;
  case 0x13:	vga_mode = mode13_200;     break;
  case 0xFE:	vga_mode = modeFF_16;      break;
  case 0xFF:	vga_mode = modeFF_16;      break;
  default: 	
    gdbg_error("vgaRegTest","Unexpected mode\n");
    DIAG_FAIL();
  }
  // 
#if 0
  gdbg_info(1,"Vga Reg Test\n");
  expect = dd = 0x11223344;
  vgaRead8(0x3DA);
  for (ii = 0;ii < 4;ii++) {
    vgaWrite8(0x3C0,ii);
    vgaWrite8(0x3C0,dd++);
  }
  vgaRead8(0x3DA);
  for (ii = 0;ii < 4;ii++,expect++) {
    vgaWrite8(0x3C0,ii);
    ddo = vgaRead8(0x3C1);
    if (diago.halInfo->hsim) 
      if ((0x3F & expect) != ddo) {
	gdbg_error("vgaRegTest","0x3C1: Unexpected I/O read data from VGA. Expected 0x%x, received: 0x%x\n", 
		   (0x3F & expect),ddo);
	DIAG_FAIL();
      }
    vgaRead8(0x3DA);
  }

#endif
  // Lets read some register data from the core just to make sure we are awake...
  ii = vgaRegRead(0x03C4, 0x00,0x3C5);
  expect = vga_mode[SEQ_REG];
  if (diago.halInfo->hsim) 
  if(expect != ii) {
    gdbg_error("vgaRegTest","0x3c5: Unexpected I/O read data from VGA. Expected 0x%x, received: 0x%x\n", 
	       expect,ii);
    DIAG_FAIL();
  }
  ii = vgaRegRead(0x03C4, 0x01,0x3C5);
  expect = vga_mode[SEQ_REG+1];
  if (diago.halInfo->hsim) 
  if(expect != ii) {
    gdbg_error("vgaRegTest","0x3C5: Unexpected I/O read data from VGA. Expected 0x%x, received: 0x%x\n", 
	       expect ,ii);
    DIAG_FAIL();
  }
  
}
static ulong vgaAperture()
{
  ulong apert;
  switch (vgaMode) {
  case 0x0:	apert = 0xB8000; break;
  case 0x2:	apert = 0xB8000; break;
  case 0x3:	apert = 0xB8000; break; // ?????
  case 0x4:	apert = 0xB8000; break;
  case 0x6:	apert = 0xB8000; break;
  case 0xe:	apert = 0xA0000; break;
  case 0x10:	apert = 0xA0000; break;
  case 0x12:	apert = 0xA0000; break;
  case 0x13:	apert = 0xA0000; break;
  case 0xFE:	apert = 0xA0000; break;
  case 0xFF:	apert = 0xA0000; break;
  default: 	
    gdbg_error("vgaAperture","Unexpected mode\n");
    DIAG_FAIL();
  }
  return(apert);
}

//static FxU8 paletteShadow[][];
// palette read/writes are sloooow...
static void vgaPalette(int all)
{
  int ii;
  int maxii,startii;
  FxU32 red,green,blue,stc;
  gdbg_printf("vgaPalette...\n");
  if (all) {
    startii=stc=0;maxii=32;
  } else {
    startii = iRandom(0xF0);
    maxii = rRandom(1,0xF);
    stc = iRandom(0xFF);
  }
  for(ii=startii;ii<maxii;ii=ii+1) {
      vgaWrite8(0x3c8, ii);		
      vgaWrite8(0x3c9, ((stc+ii)&0x3f));	
      vgaWrite8(0x3c9, ((stc+ii+1)&0x3f));	
      vgaWrite8(0x3c9, ((stc+ii+2)&0x3f));	
    }
  // auto inc
  gdbg_printf("vga Palette check %d...\n",maxii);
  vgaWrite8(0x3c7, startii);		
  for(ii=startii;ii<maxii;ii=ii+1)    {
    red = vgaRead8(0x3c9);		
    if (diago.halInfo->hsim) 
    if(red != (stc+ii & 0x3f)) {
      GDBG_ERROR("vgapalette","\nError: Unexpected palette read on RED entry 0x%x --> expect 0x%0x: got 0x%0x:!!\n", ii, stc+ii&0x3f, red);
      DIAG_FAIL();		
    }
    green = vgaRead8(0x3c9); 		
      
    if (diago.halInfo->hsim) 
    if(green != ((stc+ii+1) & 0x3f)) {
      GDBG_ERROR("vgapalette","\nError: Unexpected palette read on GREEN entry 0x%x --> expect 0x%0x: got 0x%0x:!!\n", ii, stc+ii+1&0x3f, green);
      DIAG_FAIL();		
    }
    blue = vgaRead8(0x3c9); 		
    if (diago.halInfo->hsim) 
    if(blue != ((stc+ii+2) & 0x3f)) {
      GDBG_ERROR("vgapalette","\nError: Unexpected palette read on BLUE entry 0x%x --> expect 0x%0x: got 0x%0x:!!\n", ii, stc+ii+2&0x3f, blue);
      DIAG_FAIL();		
    }
  }
  
  // random 
  gdbg_printf("vga Palette Random check %d...\n",maxii);
  for(ii=startii;ii<maxii;ii=ii+1) {
    ii = rRandom(startii,maxii-1);
    vgaWrite8(0x3c7, ii);
    red = vgaRead8(0x3c9);
    if (diago.halInfo->hsim) 
    if(red != (stc+ii & 0x3f)) {
      GDBG_ERROR("vgapalette","\nError: Unexpected palette read on RED entry 0x%x --> expect 0x%0x: got 0x%0x:!!\n", ii, stc+ii&0x3f, red);
      DIAG_FAIL();		
    }
    green = vgaRead8(0x3c9); 
  if (diago.halInfo->hsim) 
    if(green != ((stc+ii+1) & 0x3f)) {
      GDBG_ERROR("vgapalette","\nError: Unexpected palette read on GREEN entry 0x%x --> expect 0x%0x: got 0x%0x:!!\n", ii, stc+ii+1&0x3f, green);
      DIAG_FAIL();		
    }
    blue = vgaRead8(0x3c9); 	
  if (diago.halInfo->hsim) 
    if(blue != ((stc+ii+2) & 0x3f)) {
      GDBG_ERROR("vgapalette","\nError: Unexpected palette read on BLUE entry 0x%x --> expect 0x%0x: got 0x%0x:!!\n", ii, stc+ii+2&0x3f, blue);
      DIAG_FAIL();		
    }
  }
}

static void vgaSetMode(SstRegs *sst,int mode) 
{
  vgaMode = mode;
  gdbg_info(1,"Vga set mode %x\n",mode);
  if (diago.halInfo->hsim) {
    VGA_SET_MODE(mode,0);
  }
  vgaRegTest(sst);
}


static void vbeMemWrB(SstRegs *sst,ulong addr, ulong data,ulong be_n)
{
  FxU32 daddr;
  int savedhsim;
  GDBG_INFO(125,"       VBESET(0x%x,%11d(0x%08x)) %d\tVBE ben0x%01x\n",
		      addr,data,data, 0,be_n);
  if (diago.halInfo->hsim)
    PCI_MEM_WR_B(addr,data,be_n);

  savedhsim = diago.halInfo->hsim;
  diago.halInfo->hsim = 0;
    daddr = SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET + addr + vShadowWrBase +
      vbeBaseAddr*VBESIZE - vgaAperture();  
    if (be_n == 0) 
      SET(*(FxU32 *)daddr,data);
    else {
      if ((be_n & 0x1) == 0)
	SET8(*(FxU32 *)daddr,data);
      if ((be_n & 0x2) == 0)
	SET8(*(FxU32 *)(daddr+1),data >> 8);
      if ((be_n & 0x4) == 0)
	SET8(*(FxU32 *)(daddr+2),data >> 16);
      if ((be_n & 0x8) == 0)
	SET8(*(FxU32 *)(daddr+3),data >> 24);
    }
  diago.halInfo->hsim = savedhsim;
}
static ulong vbeMemRd(SstRegs *sst,ulong addr)
{
  FxU32 daddr;
  FxU32 data;
  if (diago.halInfo->hsim)
    data = PCI_MEM_RD(addr);
  else {
    daddr = SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET + addr + vShadowRdBase +
      vbeBaseAddr*VBESIZE;  
    data = GET(*(FxU32 *)daddr);
  }

  GDBG_INFO(125,"       VBEGET(0x%x,%11d(0x%08x)) %d\tVBE\n",
		      addr,data,data, 0);
  return(data);
}

static void disableFifo() {
  // reach into privies..
  CSIM_PRIVATE(diago.sstCSIM)->inCmdFifoExecMode = 1;
}
static void enableFifo() {
  CSIM_PRIVATE(diago.sstCSIM)->inCmdFifoExecMode = 0;
}


static int voldwf = -1;
static void pushV()
{
  voldwf = diago.writeFifo;
  diago.writeFifo = 0; 
}
static void popV() 
{
  diago.writeFifo = voldwf; 
  voldwf = -1;
}

static void vWriteShadowMem(ulong addr,ulong data,ulong be_n) 
{
  int ii;
  if (addr + vShadowWrBase > VMEMSIZE) 
    GDBG_ERROR("vShadowMem","Overflow %d %d\n",addr,vShadowWrBase);

  ii = (vShadowWrBase + addr)/4;
  if ((be_n & 0x1) == 0) {
    vShadowMem[ii] &= ~0xFF;
    vShadowMem[ii] |= data & 0xFF;
  }
  if ((be_n & 0x2) == 0) {
    vShadowMem[ii] &= ~0xFF00;
    vShadowMem[ii] |= (data & 0xFF00);
  }
  if ((be_n & 0x4) == 0) {
    vShadowMem[ii] &= ~0xFF0000;
    vShadowMem[ii] |= (data & 0xFF0000);
  }
  if ((be_n & 0x8) == 0) {
    vShadowMem[ii] &= ~0xFF000000;
    vShadowMem[ii] |= (data & 0xFF000000);
  }
}
static ulong vReadShadowMem(ulong addr) 
{
  if (addr + vShadowRdBase > VMEMSIZE)
    GDBG_ERROR("vReadShadowMem","Overflow %d %d\n",addr,vShadowRdBase);
  return(vShadowMem[(vShadowRdBase + addr)/4]);
}
#if 0
#include "mode13.h"
static void SetRegs (LPPARMENTRY lpParms)
{
  int	i;
  WORD	wCRTC;
  
  // Blank screen during the register changes (will be
  // re-enabled at the end of this routine)
  vgaRead8 (INPUT_MSTATUS_1); vgaRead8 (INPUT_CSTATUS_1);
  vgaWrite8 (ATC_INDEX, 0x00);
  
  // Load the Sequencer and the clock
  vgaWrite16 (SEQ_INDEX, 0x0100);		// Sync reset
  for (i = 0; i < 4; i++)
    {
      vgaWrite8 (SEQ_INDEX, (BYTE) (i + 1));
      vgaWrite8 (SEQ_DATA, lpParms->seq_data[i]);
    }
  
  vgaWrite8 (MISC_OUTPUT, lpParms->misc);
  vgaWrite16 (SEQ_INDEX, 0x0300);		// End sync reset
  
  // Load the CRTC
  wCRTC = CRTC_MINDEX;
  if (lpParms->misc & 1) wCRTC = CRTC_CINDEX;
  vgaWrite16 (wCRTC, 0x2011);				// Disable write protection
  for (i = 0; i < 25; i++)
    {
      vgaWrite8 (wCRTC, (BYTE) i);
      vgaWrite8 ((WORD) (wCRTC + 1), lpParms->crtc_data[i]);
    }
  
  // Load the GDC
  for (i = 0; i < 9; i++)
    {
      vgaWrite8 (GDC_INDEX, (BYTE) i);
      vgaWrite8 (GDC_DATA, lpParms->gdc_data[i]);
    }
  
  // Load the attribute controller
  vgaRead8 ((WORD) (wCRTC + 6));
  for (i = 0; i < 20; i++)
    {
      vgaWrite8 (ATC_INDEX, (BYTE) i);
      vgaWrite8 (ATC_INDEX, lpParms->atc_data[i]);
    }
  vgaWrite8 (ATC_INDEX, 0x34);			// VGA register not in parm table (enable
  vgaWrite8 (ATC_INDEX, 0x00);			// CRTC palette access, as well)
}

static void vgaMode13(SstRegs *sst,int pass) 
{
  unsigned ii,jj,wXRes,wYRes;
  unsigned clr,clrOrg;
  unsigned addr;
  static int nCount = 0;
  char fname[128];
  int retval;

  // +SST_VGA_ONLY
  /* adapter initialization sequence */
  PCI_IOW8(0x46e8,0x18);
  PCI_IOW8(0x102,0x01);
  PCI_IOW8(0x46e8,0x08);
  /* motherboard initialization sequence */
  // vgaWrite8(0x3c3,0x01);

  vgaSetMode(sst,0x12);
  vgaWrite8(0x3c8,0);
  for (ii=0;ii<256;ii++) {
    gdbg_printf("dac set %d..\n",ii);
    vgaWrite8(0x3c9,dac[ii][0]);
    vgaWrite8(0x3c9,dac[ii][1]);
    vgaWrite8(0x3c9,dac[ii][2]);
  }
  wXRes = 160;
  wYRes = 20;  
  gdbg_printf("Set regs..\n");
  SetRegs (&parmSmallX);
  for (ii = 0; ii < wXRes; ii++) {
    addr = 0xA000 + (ii / 4);
    jj = 1 << (ii & 3);
    vgaWrite8(SEQ_INDEX, 0x02);
    vgaWrite8(SEQ_DATA, (BYTE) jj);
    clr = clrOrg++;
    gdbg_printf("Drawing row %d (max %d)...\n",ii,wXRes);
    for (jj = 0; jj < (int) (wYRes - 1); jj++)	{
      PCI_MEM_WR_B(addr,
		   ((unsigned int)clr)<<((addr&0x3)*8),
		   1<<(addr&0x3) ^ 0xf);
     clr++;
     addr += wXRes/4;
    }
  }
  sprintf (fname, "FR%02d%02d%02X.BMP", 3,0, nCount++);
  gdbg_printf("Frame capture...\n");
  // vgapli.h
#define CAP_SCREEN     0
#define CAP_OVERSCAN    1
#define CAP_COMPOSITE  2

#define BLINK_OFF 0
#define BLINK_ON  1

  retval =  IntCaptureFrame(fname, 
			    CAP_SCREEN,
			    BLINK_ON,BLINK_ON,0,0);  // io.c

}
#endif 

static void vgaWriteLatches(SstRegs *sst) 
{
  int ii;
  unsigned data,result;
  vgaSetMode(sst,0x12);
  gdbg_printf("Testing VGA write latch tests\n");
  
  for(ii=0;ii<4;ii=ii+1)
    {
      data = iRandom(0xffffffff);	// 
      vgaWrite16(0x3ce, 0x0004);
      vgaWrite16(0x3d4, ((data &0xff) << 8) |0x22);	// Write latches Byte 0
      vgaWrite16(0x3ce, 0x0104);
      vgaWrite16(0x3d4, (((data>> 8) &0xff) << 8) |0x22);	// Write latches Byte 1
      vgaWrite16(0x3ce, 0x0204);
      vgaWrite16(0x3d4, (((data >>16) &0xff) << 8) |0x22);	// Write latches Byte 2
      vgaWrite16(0x3ce, 0x0304);
      vgaWrite16(0x3d4, (((data>>24) &0xff) << 8) |0x22);	// Write latches Byte 3
      vgaWrite8(0x3d4, 0x22);	// Write latches Byte 0

      vgaWrite16(0x3ce, 0x0004);
      result = vgaRead8(0x3d5);
      vgaWrite16(0x3ce, 0x0104);
      result |= vgaRead8(0x3d5) << 8;
      vgaWrite16(0x3ce, 0x0204);
      result |= vgaRead8(0x3d5) << 16;
      vgaWrite16(0x3ce, 0x0304);
      result |= vgaRead8(0x3d5) << 24;
      if (diago.halInfo->hsim) 
      if(result != data)
	{
	  gdbg_error("VgaWriteLatches",
		     "Error:	Unexpected read of write latch data from VGA. Expected 0x%x, received: 0x%x\n", data, result);
	  DIAG_FAIL();
	}
    }
}	


static void vgaTest(SstRegs *sst,int pass) 
{
  pushV();
  gdbg_printf("vgaTest start.............\n");
  // vgaMode13(sst,pass);
  vgaWriteLatches(sst);
  if (iRandom(16) == 1)
    vgaPalette(0);
  if (iRandom(4) == 1)
    vgaRegTest(sst);
  popV();

}

static FxU32 vgaSetup(SstRegs *sst)
{
  SstIORegs *sstio;
  FxU32 temp;
  sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
  pushV();

  vgaSetMode(sst,0x0);

  vgaBase =  ((diago.minTrashMem + 0xFFFF) & 0x10000);
  gdbg_printf("vgaBase %x\n",vgaBase);
  //
  temp = GET(sstio->vgaInit0);
  temp |= (vgaBase/0x10000) << 14;
  SET(sstio->vgaInit0,temp);
  temp = GET(sstio->vgaInit0);
  popV();
  gdbg_printf("VGA init0 0x%08x\n",temp);
  
  vgaRegTest(sst);
  vgaPalette(1);
  vbeBaseAddr = 0xdeadbeef;
  return(vgaBase + 0x10000);
}

static FxU32 vbeSetup(SstRegs *sst)
{
  SstIORegs *sstio;
  int ii;
  FxU32 data,rdata;
  FxU32 temp;
  sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
  pushV();
  //vgaSetMode(sst,0x12);
  vgaBase = 0xdeadbeef;
  vgaSetMode(sst,0x0);
  //
  temp = GET(sstio->vgaInit1);
  temp |= SST_VGA_CHAIN4;
  if (diago.minTrashMem < VBESIZE)
    vbeBaseAddr = 1;
  else
    vbeBaseAddr = (diago.minTrashMem + VBESIZE - 1) / VBESIZE;
  temp |= vbeBaseAddr  & SST_VGA_VBEWR;
  temp |= (vbeBaseAddr << SST_VGA_VBERD_SHIFT) & SST_VGA_VBERD;
  SET(sstio->vgaInit1,temp);
  temp = GET(sstio->vgaInit1);
  gdbg_printf("VGA init1 0x%08x\n",temp);

  vShadowWrBase =  0; // bytes
  vShadowRdBase =  0;
  for (ii=0;ii < VMEMSIZE/4; ii++) {
    vWriteShadowMem(ii*4,0,0); // zero out shadow
  }
#if 0
  for (ii=0;ii < VMEMSIZE/4; ii++) {
    data = iRandom(0xffffffff);
    vbeMemWrB(sst,vgaAperture(),data,0);
    vWriteShadowMem(ii*4,data,0);
  }
#endif
  popV();
  if (diago.halInfo->hsim) {
    int err = 0;
    ulong vaddr;
    data = 0xa1b2c3de;
    vaddr = (vgaAperture() & 0xF0000);
    PCI_MEM_WR_B(vaddr + 0x10000,data,0);
    if (data == PCI_MEM_RD(vaddr + 0x10000)) {
	GDBG_ERROR("Vbe","Chip responding to 0x%05x\n",vaddr + 0x10000);
	err = 1;
    }
    PCI_MEM_WR_B(vgaAperture(),data,0);
    rdata = PCI_MEM_RD(vgaAperture());
    if (data != rdata) {
	GDBG_ERROR("Vbe","Unable to write to 0x%05x  --> 0x%08x:0x%08x\n",
		   vgaAperture(), data,rdata);
	err = 1;
    }

    PCI_MEM_WR_B(vgaAperture(),0,0);
    data = 0xabcd1359;
    PCI_MEM_WR_B(vgaAperture() + 0x7C0,data,0);
    rdata = PCI_MEM_RD(vgaAperture() + 0x7C0);
    if (data != rdata) {
	GDBG_ERROR("Vbe","Unable to write at 0x%05x --> 0x%08x:0x%08x\n",
		   vgaAperture() + 0x7C0, data,rdata);
	err = 1;
    }
    PCI_MEM_WR_B(vgaAperture() + 0x7C0,0,0);

    data = 0x12345678;
    PCI_MEM_WR_B(vaddr + 0x10000,data,0);
    if (data == PCI_MEM_RD(vaddr + 0x10000)) {
	GDBG_ERROR("Vbe","Chip responding to 0x%05x\n",vaddr + 0x10000);
	err = 1;
    }
    data = 0xaabbeeff;
    PCI_MEM_WR_B(vaddr - 0x10000,data,0);
    if (data == PCI_MEM_RD(vaddr - 0x10000)) {
	GDBG_ERROR("Vbe","Chip responding to 0x%05x\n",vaddr - 0x10000);
	err = 1;
    }
    if (err)
      DIAG_FAIL();
  }
  vgaRegTest(sst);
  vgaPalette(1);
  return(vbeBaseAddr*VBESIZE + VMEMSIZE);
}

static FxU32 vSetup(SstRegs *sst) {
  if (vType == '2') {
    return(diago.minTrashMem);
  }

  if (vType == '0')
    return(vgaSetup(sst));
  else
    return(vbeSetup(sst));
}



enum {VBE_TEST_MODE,VBE_TEST_RW,VBE_TEST_WRITE,VBE_TEST_READ,VBE_LAST };

static void vbeLfb(SstRegs *sst,int pass) 
{
  int v1;
  int vSize;
  int vBase;
  SstIORegs *sstio;
  int ii;
  ulong data,ben;
  ulong temp;
  pushV();
  gdbg_printf("vbeLfb start.............\n");

  if (iRandom(4) == 1) { // VBE_TEST_INIT:
    sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
    temp = GET(sstio->vgaInit1);
    vShadowWrBase = iRandom(1)*VBESIZE;
    vShadowRdBase = iRandom(1)*VBESIZE;
    temp &= ~(SST_VGA_VBEWR | SST_VGA_VBERD);
    temp |= (vbeBaseAddr + vShadowWrBase/VBESIZE)  & SST_VGA_VBEWR;
    temp |= (vbeBaseAddr + vShadowRdBase/VBESIZE << SST_VGA_VBERD_SHIFT) & SST_VGA_VBERD;
    SET(sstio->vgaInit1,temp);
    gdbg_printf("VGA init1 0x%08x\n",temp);
  }

  if (pass < 2)
    v1 = VBE_TEST_WRITE;
  else
    v1 = iRandom(VBE_LAST-1);
  if (iRandom(16) == 1)
    vgaPalette(0);

  if (iRandom(4) == 1)
    vgaRegTest(sst);
  switch(v1) {
  case VBE_TEST_MODE:
    vgaRegTest(sst);
    if (vgaMode == 0x12) 
      vgaSetMode(sst,0x0);
    else
      vgaSetMode(sst,0x12);
  case VBE_TEST_WRITE:
    if (v1 == VBE_TEST_WRITE) {
      vBase = iRandom(2000);
      // vSize = rRandom(4,VBESIZE-vBase);
      vSize = rRandom(4,64);
    }
    else { 
      vBase = 0;
      vSize = 16;
    }
    for (ii=0;ii < vSize/4; ii++) {
      data = iRandom(0xffffffff);
      ben = iRandom(0xf);
      if (pass  < 2) ben = 0;
      vbeMemWrB(sst,vgaAperture()+ii*4,data,ben);
      vWriteShadowMem(ii*4,data,ben);
    }
    vgaRegTest(sst);
    if (v1 == 0)
      if (iRandom(1) || pass < 2)
	break;
  case VBE_TEST_READ:
    if (v1 == VBE_TEST_READ) {
      vBase = iRandom(2000);
      //vSize = rRandom(4,VBESIZE-vBase);
      vSize = rRandom(4,64);
    }
    for (ii=0;ii < vSize/4; ii++) {
      data = vbeMemRd(sst,vgaAperture()+ii*4);
      if (diago.halInfo->hsim) 
      if (data != vReadShadowMem(ii*4)) {
	GDBG_ERROR("Vbe","Data/shadow mismatch at %x ==> 0x%08x : 0x%08x\n",ii*4,data,vReadShadowMem(ii*4));
	DIAG_FAIL();
      }
    }
  case VBE_TEST_RW:
    vBase = iRandom(2000);
    //vSize = rRandom(4,VBESIZE-vBase);
    vSize = rRandom(4,64);
    for (ii=0;ii < vSize/4; ii++) {
      data = vbeMemRd(sst,vgaAperture()+ii*4);
      if (diago.halInfo->hsim) 
      if (data != vReadShadowMem(ii*4)) {
	GDBG_ERROR("Vbe","Data/shadow mismatch at %x ==> 0x%08x : 0x%08x\n",ii*4,data,vReadShadowMem(ii*4));
	DIAG_FAIL();
      }
      ben = iRandom(0xf);
      if (pass  < 2) ben = 0; 
      vbeMemWrB(sst,vgaAperture()+ii*4,data,ben);
      vWriteShadowMem(ii*4,data,ben);
    }
    vgaRegTest(sst);
  }
  //gdbg_printf("vbeLfb done.............\n");
  popV();
}


static void nmlfbRead(SstRegs *sst,FxU32 base)
{
  int ii;
  FxU32 daddr,data,cksum;
  unsigned burst,status;

  status = GET(sst->status);
  burst = iRandom(32);
  daddr = SST_BASE_ADDRESS(sst) + SST_RAW_LFB_OFFSET + 
    rRandom(base+burst,diago.maxTrashMem-burst) & ~0x3;
  cksum = 0;
  for (ii = 0; ii< burst; ii++) {
    data = GET(*(FxU32 *)daddr);
    daddr += 4;
    cksum ^= data;
  }
  gdbg_printf("NMLFB read %d %x\n",burst,cksum);
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
    gdbg_printf("\"-xc<n>\"\tSelect 5:P5 6:P6\n");
    gdbg_printf("\"-xv<n>\"\tSelect 0:vga 1:vbe 2:none 3:random\n");
    gdbg_printf("\"-xb\"\tbig sizes\n");
    gdbg_printf("\"-xB\"\tburst\n");
    gdbg_printf("\"-xw<n>\"\t0:single stream 1:direct PCI wax 2:cmdfifo1 wax\n");
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
	    case 'b':
	      bigSize = diago.tsize * 32;
	      if (bigSize > 1000)
		bigSize = 1000;
	      break;
	    case 'B':
	      burst = 1;
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
	      GDBG_PRINTF("INFO: selecting ptype %c\n",pType);
	      opts++;
	      break;
	    case 'v':
	      vType = opts[1];
	      GDBG_PRINTF("INFO: selecting vtype %c\n",vType);
	      opts++;
	      break;
	    default:
	      Xusage();
	    }
	    opts++;
	}
    }
}



static int trashTexMem(FxU32 base,unsigned sizeBytes)
{
  return(base + sizeBytes > sRegs.texBaseAddr);

}
static int trash3dMem(FxU32 base,unsigned sizeBytes)
{
  return(base < diagfb.auxBufferAddr &&
	      base + sizeBytes >= diagfb.colBufferAddr[0]);

}

static int trash2dMem(FxU32 base,unsigned sizeBytes)
{
  // cpriv->windows ???
  //SET(sstg->clip0min, 0x00000000);
  //SET(sstg->clip0max,(diago.ymaxscreen<<16) | diago.xmaxscreen);
  //SET(sstg->dstBaseAddr,diago.minTrashMem);		// define a default surface
  //SET(sstg->srcBaseAddr,diago.minTrashMem);		// Default 

  int trash;
  FxU32 gbase;
  FxU32 clip0max;
  gbase = sGRegs.dstBaseAddr;
  clip0max = sGRegs.clip0max;
  trash = (base < gbase + waxpDepth*(clip0max >> 16)*(clip0max & SST_MASK(16)));
  gbase = sGRegs.srcBaseAddr;
  trash |= (base < gbase + waxpDepth*(clip0max >> 16)*(clip0max & SST_MASK(16)));
  return(trash);
}

static int trashMem(FxU32 base,unsigned sizeBytes)
{
  return(trash2dMem(base,sizeBytes) || trash3dMem(base,sizeBytes) ||
	 trashTexMem(base,sizeBytes));
}
// scarved from hblt.c
static FxU32 screen[MAXSCREEN][MAXSCREEN];
static FxU32 host_pixels[MAXSCREEN*MAXSCREEN];

// host blt simple ROP 32bpp
void h32sblt(SstRegs *sst,int size,int pass) 
{
  long xs,ys,xd,yd,stride;
  FxU32 cmdXops,cmdops,cfore,cback,cdest,rop;
  FxU32 srcFormat;
  FxU32 byte,addr,nextaddr;
  long ycEnd,yc,ycStart,ycInc,xc;
  long ww,hh;
  FxU32 col,pix;
  FxU32 xx,nn;

  SstGRegs *sstg;
  sstg = SSTG_CHIP(sst);  
  if ((pass % 7) == 0) {			// init pattern every 7 times
    sstg_setpattern_random(sstg);
    sGRegs.dstBaseAddr = GET(sstg->dstBaseAddr);
    sGRegs.srcBaseAddr = GET(sstg->srcBaseAddr);
  }

  xyRandom(&xs,&ys);			// NOTE: source is on screen
  ww = rRandom(1,size);
  hh = rRandom(1,size);
  xyRandom(&xd,&yd);		// anywhere

  srcFormat = SSTG_PIXFMT_32BPP;
  stride = (iRandom(diago.xmaxscreen*4)/4 + 1) * 4;
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
  gdbg_printf("HBLT xs,ys = %d,%d    xd,yd = %d,%d    w,h = %d,%d    pox,y = %d,%d\n",
	    xs,ys, xd,yd, ww,hh,
	    ((cmdops & SSTG_X_PATOFFSET)>>SSTG_X_PATOFFSET_SHIFT) & 7,
	    ((cmdops & SSTG_Y_PATOFFSET)>>SSTG_Y_PATOFFSET_SHIFT) & 7);
  sstg_print_stuff(cmdops, cmdXops, rop, srcFormat, cfore, cback);

  SET(sstg->command, cmdops | SSTG_HOST_BLT);

  stride = (srcFormat & SSTG_SRC_LINEAR_STRIDE) >> SSTG_SRC_STRIDE_SHIFT;
  // addr = sstg_compute_blit_address(srcFormat, 0, xs, ys, w);
  addr =  ys*stride + xs*4;

  byte = addr & 3;
  gdbg_info(7, "hblt.exe: starting byte position: %d\n", byte);
  // should ignore everything above bit 2
  SET(sstg->srcXY, (iRandom(0xFFFFFFFF) & ~3) | byte);
  SET(sstg->dstXY,(yd<<16) | (xd & 0xFFFF));
  SET(sstg->dstSize,(hh<<16) | ww);

  ycInc = 1;
  ycEnd = yd + hh;
  stride = srcFormat & SSTG_SRC_FORMAT;
  nextaddr = byte;
  nn = 0;
  // for each row of the blit
  for (yc = yd;yc < ycEnd;yc += ycInc) {
    addr = nextaddr;
    nextaddr = (addr + (stride % 4)) % 4;
    for (xx=0; xx < ww; xx++) {
      col = iRandom(0xFFFFFFFF);
      if (pass >= debugPass) {
	SET(sstg->launch[8], col);
	if (ONSCREEN(xd+xx,yc))
	  host_pixels[nn++] = col;
      }
    }
  }

    gdbg_info(1,"check et %d\n",diago.checkEveryTriangle);

  if (diago.checkEveryTriangle && pass >= debugPass) {
    sstg_idle(sst);			// wait for the command to complete
    // positive y direction
    ycStart = yd - 1;
    ycEnd = yd + hh;
    ycInc = 1;
    pix = 0;
    // check the entire rectangle
    gdbg_info(1,"h32sblt: checking start\n");
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
	      gdbg_info(199,"h32sblt: checking %d,%d 0x%x (outside)\n",xc,yc,cdest);
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
  len = rRandom(5,size);
  x2 = rRandom(x1-len,x1+len);
  y2 = rRandom(y1-len,y1+len);
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

  gdbg_printf("RECT x,y = %d,%d    w,h = %d,%d    pox,y = %d,%d\n",
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


void
main(int argc, char **argv)
{
    int ii;
    int pp;
    int ttp;
    SstRegs *sst;
    SstCRegs *sstc;
    SstIORegs *sstio;
    FxU32 ttemp;
    // texture
    Triangle *pt;
    FxU32 texBaseAddr,shadowTexBase;
    int newTexBase;
    static Triangle tt;
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
    unsigned waxEnd;
    FxU32 waxStart;

    // 
    int lsize;
    unsigned maxFbMem;

    AGPMOVECMD cmd;
    AGPMOVECMD *cmdp;
 
    int ibb,mbb;

    int down;
    FxU32 memConfig;
    FxU32 tiledOffsetBytes;
    FxU32 psize;
    cmdp = &cmd;


    diago.gui = 1;				// flag as 2D app
    diago.dstFormat = SSTG_PIXFMT_32BPP>>SSTG_SRC_FORMAT_SHIFT;
    sst = SST_BEGIN(argc,argv);
    sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);
    sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
    sstg = SSTG_CHIP(sst);

    nlfb = nyuv= n3dlfb= ntex= nrect= nline= ntriangle = 0;
    nplfb = npyuv= np3dlfb= nptex= nprect= npline= nptriangle = 0;
    selectInverse = 0;
    selectType = '4';
    randomConfig = '0';
    pType = '5';
    vType = '0';
    selectpType = 0;
    waxStream = 0;

    XParseOpts(argc,argv);
    if (pType == '5')
      if (!diago.writeFifo) {
	GDBG_ERROR("cfestress","Packet 5 runs only in CMDFIFO mode\n");
	DIAG_FAIL();
      }
	
    if (diago.ytiled != 0) {
      gdbg_error(MODNAME, "Tiled NYI\n");
      DIAG_FAIL();
    }
    switch(waxStream) {
    case 0: 
      break;
    case 1: 
      break;
    case 2: 
      if (diago.whichFifo < 2) {
	GDBG_ERROR("cfestress","Both CMDFifos need to be activated\n");
	DIAG_FAIL();
      }
      break;
    }
    // SET(sstio->lfbMemoryConfig,0xa3FFF); 
    if (randomConfig != '0') {
      memConfig = (rRandom(diago.minTrashMem >> 12,0x1FFF) << SST_RAW_LFB_TILE_BEGIN_PAGE_SHIFT)
	& SST_RAW_LFB_TILE_BEGIN_PAGE |
	(iRandom(3) << SST_RAW_LFB_ADDR_STRIDE_SHIFT) & SST_RAW_LFB_ADDR_STRIDE |
	(rRandom(1,0x3F) << SST_RAW_LFB_TILE_STRIDE_SHIFT) & SST_RAW_LFB_TILE_STRIDE;
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
    gdbg_info(1,"check et %d\n",diago.checkEveryTriangle);
    waxStart = vSetup(sst);
    // 2d setup
     
    pushWax(1);
    if (diago.dstFormat < (SSTG_PIXFMT_8BPP>>SSTG_SRC_FORMAT_SHIFT) ||
	diago.dstFormat > (SSTG_PIXFMT_32BPP>>SSTG_SRC_FORMAT_SHIFT)) {
	GDBG_ERROR("cfestress", "invalid destination format\n");
	DIAG_FAIL();
    }
    gdbg_printf("Setting dst/src to 0x%x\n",waxStart);
    SET(sstg->dstBaseAddr,waxStart);		// define a default surface
    SET(sstg->srcBaseAddr,waxStart);		// Default 
    waxpDepth = 4;
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
    tt.tex.tMode = texRandomFormat(iRandom(1),iRandom(1),iRandom(1)) 
	 | SST_TC_REPLACE | SST_TCA_REPLACE;
    SET(sst->textureMode,tt.tex.tMode);	// set texture mode just for fun
    texSize =  (256*256*5) & ~0xF;
    texBaseAddr = (diago.maxTrashMem - texSize)& ~0xF;
    SET(sst->texBaseAddr,texBaseAddr&SST_TEXTURE_ADDRESS);
    GDBG_INFO(0,"Tex Base Addr 0x%08x\n",texBaseAddr);
    shadowTexBase = texBaseAddr;
    sRegs.texBaseAddr = texBaseAddr;
    tt.next = NULL;

    waxEnd = diago.ymaxscreen * diago.xmaxscreen * waxpDepth + waxStart;
    maxFbMem = place3d(sst,waxEnd,texBaseAddr);
    // Generate Random LFB Write Parameters
    sRegs.lfbMode = rndlfbMode( iRandom(NUM_TEST_OPTS-1), &pixelsToWrite );
    SET( sst->lfbMode, sRegs.lfbMode );
    sRegs.clipLeftRight = (0<<16) | diago.xmaxscreen;
    SET(sst->clipLeftRight, sRegs.clipLeftRight );
    sRegs.clipBottomTop = (0<<16) | diago.ymaxscreen;
    SET(sst->clipBottomTop,sRegs.clipLeftRight );
    
    // yuv
    yuvBaseAddr = rRandom(maxFbMem,maxFbMem+8192);
    yuvBaseAddr &= ~0xF;			
      
    SET(sstc->yuvBaseAddr,yuvBaseAddr  & SST_YUV_BASE_ADDR); 
    GDBG_INFO(0,"Yuv Base Addr 0x%08x\n",yuvBaseAddr);
   // linear
    maxYuvWidth = 512;
    SET(sstc->yuvStride,SST_YUV_MEMORY_LINEAR | maxYuvWidth);


    while (DIAG_STARTPASS())	{		// for each pass
      pass++;
      if (pass == 1) {
	ttp = TEST_P5;
	if (vType != '2')
	  ttp = TEST_VGA;
      }
      else {
	if (vType != '2')
	  ttp = iRandom(TEST_VGA);
	else
	  ttp = iRandom(TEST_VGA-1);
      }
      if (iRandom(4) == 1)
	nmlfbRead(sst,maxFbMem);
      if (burst)
	// mbb = rRandom(4,32);
	mbb = rRandom(4,10);
      else mbb = 1;
      gdbg_printf("mbb %d\n",mbb);
      for (ibb = 0;ibb < mbb;ibb++) {
      switch(ttp) {
      case TEST_P5:
      case TEST_P6:
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
	loop1 = 0;
      again_sizebytes:
	if (loop1++ > MAXLOOP) {
	  gdbg_printf("WARNING:: +Diag quitting early %d\n",cmdp->space);
	  DIAG_PASS(1);
	  exit(1);
	}
	
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
	    SET( sst->lfbMode, sRegs.lfbMode );
	  }
	  lfbAlign(&cmdp->sizeBytes);
	}
	// byte texture NYI
	if (cmdp->space == SSTCP_TEXPORT_SPACE) cmdp->sizeBytes &= ~0x1; // 2 byte aligned
	
	if (cmdp->space == SSTCP_YUV_SPACE) {
	  selectY = iRandom(1);
	  if (selectY) 
	    cmdp->srcStride = rRandom(4,lsize > maxYuvWidth/2 ? maxYuvWidth/2 : lsize);
	  else
	    cmdp->srcStride = rRandom(4,lsize > maxYuvWidth/4 ? maxYuvWidth/4 : lsize);
	  cmdp->srcStride &= ~0x3;
	}
	else 
	  if (iRandom(1)) 
	    cmdp->srcStride = rRandom(2,lsize);
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
	    diago.ymaxscreen < (cmdp->sizeBytes/cmdp->srcWidth + 1))
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
	  gdbg_printf("WARNING:: ++Diag quitting early\n");
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
	  // newTexBase = iRandom(1);
	  newTexBase = 0;
	  if (newTexBase) {
	    texBaseAddr = sstFbMemRalloc(dstOffset+cmdp->fbOffset);
	    texBaseAddr &= ~0xF;			
	    sRegs.texBaseAddr = texBaseAddr;
	  }
	  // 2Meg port
	  ttemp = (diago.maxTrashMem - texBaseAddr > 0x1FFFFF) ? 0x1FFFFF :
	    (diago.maxTrashMem - texBaseAddr);
	  // maxTrash overflow is handled below
	  ttemp = (dstOffset > ttemp ? 0 : ttemp - dstOffset);
	  if (ttemp == 0)
	    cmdp->fbOffset = 0;
	  else
	    cmdp->fbOffset = rRandom(0,ttemp);
	  cmdp->fbOffset &= ~0x1;
	  break;
	case SSTCP_LFB_SPACE:
	  // cmdp->fbOffset = sstFbMemRalloc(dstOffset);
	  cmdp->fbOffset = rRandom(maxFbMem,sRegs.texBaseAddr-dstOffset);
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
	  // newYuvBase = iRandom(1);
	  newYuvBase = 0;
	  if (newYuvBase) {
	    yuvBaseAddr = sstFbMemRalloc(dstOffset);
	    yuvBaseAddr &= ~0xF;  // NYI todo
	  }
	  break;
	} // case
	
	// min of 5 DWORDS
	if (cmdp->sizeBytes <= 1)
	  goto again_sizebytes;
	
	if (cmdp->space == SSTCP_LFB_SPACE) {
	  if (sstTrashMem(cmdp->fbOffset,dstOffset))
	    goto again_sizebytes;
	  if (!trashMem(cmdp->fbOffset,dstOffset))
	    goto done;
	}
	if (cmdp->space == SSTCP_TEXPORT_SPACE) {
	  if (sstTrashMem(cmdp->fbOffset+texBaseAddr,dstOffset))
	    goto again_sizebytes;
	  if (!(trash2dMem(cmdp->fbOffset+texBaseAddr,dstOffset)
		|| trash3dMem(cmdp->fbOffset+texBaseAddr,dstOffset))) {
	    if (iRandom(1)) {
	      // set up texture download
	      tt.tex.tMode = texRandomFormat(iRandom(1),iRandom(1),iRandom(1)) 
		   | SST_TC_REPLACE | SST_TCA_REPLACE;
	      SET(sst->textureMode,tt.tex.tMode);
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
	  if (sstTrashMem(yuvBaseAddr,maxOff)) 
	    goto again_sizebytes;
	  if (!trashMem((yuvBaseAddr & SST_YUV_BASE_ADDR),maxOff)) {
	    if (newYuvBase) {
	      SET(sstc->yuvBaseAddr,yuvBaseAddr & SST_YUV_BASE_ADDR);
	      GDBG_INFO(1,"Tex Base Addr 0x%08x\n",texBaseAddr);
	    }
	    goto done;
	  }
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
	  if (pType == '5') pp = TEST_P5;
	  else if (pType == '6')  pp = TEST_P6;
	  else pp = ttp;
	}
	else if (diago.writeFifo) 
	  pp = ttp;
	else pp = TEST_P6;
	
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
	// choose packet type
	switch(pp) { 
	case TEST_P6: 
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
	case TEST_P5: // packet 5
	  if (pass >= debugPass) {
	    GDBG_INFO(1,"P5 (one or more) %s of size %d bytes from srcAddr {0x%01x,0x%08x} width %d stride %d\n",
		      space_str[cmdp->space],cmdp->sizeBytes,cmdp->baseHigh,cmdp->baseLow,
		      cmdp->srcWidth,cmdp->srcStride);
	    GDBG_INFO(1,"\t\t\t\tto dstAddr 0x%08x stride %d\n",cmdp->fbOffset,cmdp->dstStride);
	    GDBG_INFO(2,"\t\t\t\tsrcOffset 0x%08x dstOffset 0x%08x\n",srcOffset,dstOffset);
	    dstAddr = cmdp->fbOffset;
	    p5Addr = dstAddr;
	    nWords = 1 + (cmdp->srcWidth - 4 + (dstAddr & 0x3))/4;
	    seats = 1 + (cmdp->srcWidth - 4 + ((dstAddr & 0x3) + 3))/4;
	    if (seats == 1) nWords = 1;
	    cWords = MIN(psize,nWords);
	    ww = cWords; 
	    remWords = nWords - cWords;
	    GDBG_INFO(SANITY,"Initial nwords %d seats %d ; cwords %d remWords %d ww %d\n",nWords,seats,cWords,remWords,ww);
	    dstData = 0;
	    srcData = AGPRDP(cmdp->baseHigh,(cmdp->baseLow & ~0x3));
	    for (ii=0,srcAddrHigh = cmdp->baseHigh,srcAddrLow = cmdp->baseLow,
		   prevLow = cmdp->baseLow,prevHigh = cmdp->baseHigh,	   
		   prevDstAddr = cmdp->fbOffset;
		 ii<cmdp->sizeBytes;ii++) {
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
		  cWords = MIN(psize,remWords);
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
		cWords = MIN(psize,nWords);
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

	break;
      case TEST_WAX0:
      case TEST_WAX1:
	pushWax(0);
	for (ii=0;ii<iRandom(7);ii++) {
	  if (bigSize && (iRandom(4) == 1))
	    rect(sst,lsize*8,pass);
	  else
	    rect(sst,lsize/2,pass);
	  
#if 0
	  if (iRandom(1)) 
	    line(sst,lsize,pass);
	  else
	    h32sblt(sst,lsize/2,pass);
#endif
	}
	popWax(0);
	break;
      case TEST_VGA:
	if (vType == '0')
	  vgaTest(sst,pass);
	else  if (vType == '1')
	    vbeLfb(sst,pass);
	else if (iRandom(1))
	  vgaTest(sst,pass);
	else
	  vbeLfb(sst,pass);
	break;
      case TEST_3D:
	pt = NULL;
	if (pass > 2) {
	  for (ii=0;ii<iRandom(3);ii++) {
	    static Triangle t;
	    pt = &t;
	    ntriangle++;
	    if (bigSize && (iRandom(4) == 1))
	      randomStressTriangle(pt,lsize*4,1,1);
	    else
	      randomStressTriangle(pt,lsize,1,1);	// pick random triangle
	    
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
	break;
      case TEST_IDLE:
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
	  ibb = mbb;
	}
	break;
      } // case (ttp)
      if (diago.checkEveryTriangle && pass >= debugPass && 
	  (pp == TEST_P5 || pp == TEST_P6)) {
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
	  for (nn = 0;nn < cmdp->sizeBytes;nn++) {
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
	  for (nn = 0;nn < cmdp->sizeBytes;nn++) {
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
	 } // case cmdp->space
	} // mbb
      } // 
      } // startpass
    GDBG_INFO(0,"Packet Count: LFB %d YUV %d 3DLFB %d TexPort %d\n\t\tRect %d lines %d Triangles %d\n",
	      nlfb,nyuv,n3dlfb,ntex,nrect,nline,ntriangle);
    GDBG_INFO(0,"Rough Pixel Count: LFB %d YUV %d 3DLFB %d TexPort %d\n\t\tRect %d lines %d\n",
	      nplfb,npyuv,np3dlfb,nptex,nprect,npline);
    DIAG_PASS(0);	 
}

