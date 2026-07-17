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
** $Revision: 4$
** $Date: 10/11/00 8:11:53 PM$
*/

#include <assert.h>

#include "udiag.h"
#include "sstdiag.h"
#include "hsimio.h"

FxU32 compositePixelReadWorker(FxI32 buffer, FxI32 x, FxI32 y, FxBool useCSIM);
FxU32 pixelReadWorker(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y, FxBool useCSIM);
void pixelWriteWorker(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y, FxU32 data, FxBool useCSIM);
FxU32 memoryReadWorker(FxI32 chipIndex, FxI32 addr, FxI32 nBytes, FxBool useCSIM);
void memoryWriteWorker(FxI32 chipIndex, FxI32 addr, FxU32 data, FxI32 nBytes, FxBool useCSIM);

#ifndef CVG // H3 PS HACK

//----------------------------------------------------------------------
// Forces the desired base/stride values directly into the csim's 
// internal registers (if op == _DIAG_FORCE_BUFFER_STORE) or restores
// them to their original contents (if op == _DIAG_FORCE_BUFFER_RESTORE).  
//
// Each store operation must be followed by a restore operation.
//
// if (op == _DIAG_FORCE_BUFFER_STORE)
//    buffer is the virtual buffer number to load
//    return value is a buffer number to use for accessing the desired data
// if (op == _DIAG_FORCE_BUFFER_RESTORE)
//    buffer is a "don't care" input
//    return value is the original buffer value specified during the store
//
// Example: to force access to virtual buffer 1
//    buf = 1;
//    buf = _DIAG_FORCE_BUFFER(buf,_DIAG_FORCE_BUFFER_STORE);
//    function(...,buf,...);
//    buf = _DIAG_FORCE_BUFFER(buf/*don't care*/,_DIAG_FORCE_BUFFER_RESTORE);
//----------------------------------------------------------------------

#define _DIAG_FORCE_BUFFER_STORE    0
#define _DIAG_FORCE_BUFFER_RESTORE  1

SstRegs* getChipSstCSIM(FxI32 chipIndex)
{
  assert(chipIndex >= 0);
  assert(chipIndex < 4);
  
  if(chipIndex == 0)
    return(diago.sstCSIM);
  else
    return(diago.sstChildrenCSIM[chipIndex-1]);
}


static FxI32 _DIAG_FORCE_BUFFER(SstRegs *sstCSIM, FxBool primarySurface, FxI32 buffer, int op)
{
  static FxI32 saveBuffer;
  static FxU32 base, stride;
  static int expect = _DIAG_FORCE_BUFFER_STORE;
  static int restoreReqd = 0;

  // ensure that store/restore calls are paired
  if ( op != expect ) 
    GDBG_ERROR("_DIAG_FORCE_BUFFER","store/restore calls must be paired "
               "(received=%s,expected=%d)\n",
               op == _DIAG_FORCE_BUFFER_STORE ? "STORE" : "RESTORE",
               expect == _DIAG_FORCE_BUFFER_STORE ? "STORE" : "RESTORE");

  // toggle expected operation for next invocation
  expect = !expect;

  // save/restore 3D buffer state
  if ( op == _DIAG_FORCE_BUFFER_STORE ) {
    
    saveBuffer = buffer;

    if ( buffer == CSIM_BUF_3D_FRONT || 
         buffer == CSIM_BUF_3D_BACK  ||
         buffer == CSIM_BUF_3D_TRIPLE ) {
      
      // store current contents of col buffer base/stride registers
      restoreReqd = 1;
      base = sstCSIM->colBufferAddr;
      stride = sstCSIM->colBufferStride;
      
      // stuff the registers with desired values
      if ( buffer == CSIM_BUF_3D_FRONT ) {
	if(primarySurface)
	  sstCSIM->colBufferAddr = diagfb.colBufferAddr[0];
	else
	  sstCSIM->colBufferAddr = diagfb.colBufferAddrSecondary[0];
	sstCSIM->colBufferStride = diagfb.colBufferStride[0];
      } else if ( buffer == CSIM_BUF_3D_BACK ) {
	if(primarySurface)
	  sstCSIM->colBufferAddr = diagfb.colBufferAddr[1];
	else
	  sstCSIM->colBufferAddr = diagfb.colBufferAddrSecondary[1];
        sstCSIM->colBufferStride = diagfb.colBufferStride[1];
      } else if ( buffer == CSIM_BUF_3D_TRIPLE ) {
	if(primarySurface)
	  sstCSIM->colBufferAddr = diagfb.colBufferAddr[2];
	else
	  sstCSIM->colBufferAddr = diagfb.colBufferAddrSecondary[2];
        sstCSIM->colBufferStride = diagfb.colBufferStride[2];
      }
      
      // use the (physical) color buffer to access the desired (virtual) buffer
      buffer = CSIM_BUF_3D_COLOR;   

    } 
    
  } else {  // _DIAG_FORCE_BUFFER_RESTORE
    
    if ( restoreReqd ) {
      sstCSIM->colBufferAddr = base;             // be conservative, always restore
      sstCSIM->colBufferStride = stride;
      restoreReqd = 0;
    } 

    buffer = saveBuffer;

  }

  return buffer;
}

#endif

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//
//                        Worker functions
//
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

FxU32 compositePixelReadWorker(FxI32 buffer, FxI32 x, FxI32 y, FxBool useCSIM)
{
  if (buffer == CSIM_BUF_3D_FRONT || 
      buffer == CSIM_BUF_3D_BACK || 
      buffer == CSIM_BUF_3D_TRIPLE)
    {
      FxU32 r[4], g[4], b[4], a[4];
      FxU32 color[4];
      FxU32 compositeR, compositeG, compositeB, compositeA;
      FxU32 compositeColor;
      FxI32 sampleIndex;
      
      //Check to see if we're running in AA
      if(!diago.aaEnabled)
	return(pixelReadWorker(0, buffer, x, y, useCSIM));
      
      //Zero out the composite stuff
      for(sampleIndex=0; sampleIndex<(FxI32)diago.aaSampleCount; sampleIndex++)
	{
	  r[sampleIndex] = 0;
	  g[sampleIndex] = 0;
	  b[sampleIndex] = 0;
	  a[sampleIndex] = 0;
	}
      compositeR=0;
      compositeG=0;
      compositeB=0;
      compositeA=0;

      //Read the samples and convert to 8888
      for(sampleIndex=0; sampleIndex<(FxI32)diago.aaSampleCount; sampleIndex++)
	{
	  color[sampleIndex] = pixelReadWorker(sampleIndex, buffer, x, y, useCSIM);
	  a[sampleIndex] = (color[sampleIndex] >> 24) & 0xFF;
	  r[sampleIndex] = (color[sampleIndex] >> 16) & 0xFF;
	  g[sampleIndex] = (color[sampleIndex] >>  8) & 0xFF;
	  b[sampleIndex] = (color[sampleIndex] >>  0) & 0xFF;
	  
	  compositeR += r[sampleIndex];
	  compositeG += g[sampleIndex];
	  compositeB += b[sampleIndex];
	  compositeA += a[sampleIndex];
	}
      
      if(sampleIndex == 2)
	{
	  compositeR >>= 1;
	  compositeG >>= 1;
	  compositeB >>= 1;
	  compositeA >>= 1;
	}
      else if(sampleIndex == 4)
	{
	  compositeR >>= 2;
	  compositeG >>= 2;
	  compositeB >>= 2;
	  compositeA >>= 2;
	}
      else
	assert(0);      

      compositeColor = (compositeA << 24) | (compositeR << 16) | (compositeG << 8) | (compositeB << 0);

      return(compositeColor);
    }
  else //Non-color buffers
    {
      //Just read the primary sample
      return(pixelReadWorker(0, buffer, x, y, useCSIM));
    }
  
  assert(0);
  return(0);
} 

FxU32 pixelReadWorker(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y, FxBool useCSIM)
{
  SstRegs *sstCSIM;
  CsimPrivate *cp;
  FxI32 sliY;
  FxBool usingPrimaryBuffers;
  FxU32 data;

  //Find the chip to use
  if(sampleIndex <= 1)
    sstCSIM = csimFindPixelOwner(y, &sliY);
  else
    sstCSIM = csimFindSecondaryPixelOwner(y, &sliY);
  cp=CSIM_PRIVATE(sstCSIM);

  //Figure out which buffers to use
  if((sampleIndex & 1) == 0)
    usingPrimaryBuffers = FXTRUE;
  else
    usingPrimaryBuffers = FXFALSE;
  
  //Make sure we're pointing at the correct buffers
  if(usingPrimaryBuffers)
    {
      sstCSIM->colBufferAddr = COL_BUFFER_ADDR_PRIMARY(sstCSIM);
      sstCSIM->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY(sstCSIM);
    }
  else
    { //secondary buffers
      sstCSIM->colBufferAddr = COL_BUFFER_ADDR_SECONDARY(sstCSIM);
      sstCSIM->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY(sstCSIM);      
    }

  buffer = _DIAG_FORCE_BUFFER(sstCSIM, usingPrimaryBuffers, buffer,_DIAG_FORCE_BUFFER_STORE);

  if(useCSIM)
    data = readPixel(sstCSIM, buffer, x, y, cp->memory, "csim");
  else
    data = readPixel(sstCSIM, buffer, x, y, cp->hsim_memory, "hsim");       

  buffer = _DIAG_FORCE_BUFFER(sstCSIM, usingPrimaryBuffers, buffer,_DIAG_FORCE_BUFFER_RESTORE);
  if (buffer == CSIM_BUF_3D_FRONT || 
      buffer == CSIM_BUF_3D_BACK || 
      buffer == CSIM_BUF_3D_TRIPLE)
    {
      if (diago.rgb == 16) 
	data = ((data & 0xF800) << 8) | ((data & 0x07E0) << 5) |((data & 0x001F) << 3);
      if (diago.rgb == 15) 
	data = ((data & 0x8000) << 16) | ((data & 0x7C00) << 9) | 
	  ((data & 0x03E0) << 6) |((data & 0x001F) << 3);
    }

  //Make sure we put things back the way they were
  if(cp->environment.aaPrimaryBuffers)
    {
      sstCSIM->colBufferAddr = COL_BUFFER_ADDR_PRIMARY(sstCSIM);
      sstCSIM->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY(sstCSIM);
    }
  else
    {
      sstCSIM->colBufferAddr = COL_BUFFER_ADDR_SECONDARY(sstCSIM);
      sstCSIM->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY(sstCSIM);      
    }

  return(data);
}

void pixelWriteWorker(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y, FxU32 data, FxBool useCSIM)
{
  SstRegs *sstCSIM;
  CsimPrivate *cp;
  FxI32 sliY;
  FxBool usingPrimaryBuffers;

  //Find the chip to use
  if(sampleIndex <= 1)
    sstCSIM = csimFindPixelOwner(y, &sliY);
  else
    sstCSIM = csimFindSecondaryPixelOwner(y, &sliY);
  cp=CSIM_PRIVATE(sstCSIM);

  //Figure out which buffers to use
  if((sampleIndex & 1) == 0)
    usingPrimaryBuffers = FXTRUE;
  else
    usingPrimaryBuffers = FXFALSE;
  
  //Make sure we're pointing at the correct buffers
  if(usingPrimaryBuffers)
    {
      sstCSIM->colBufferAddr = COL_BUFFER_ADDR_PRIMARY(sstCSIM);
      sstCSIM->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY(sstCSIM);
    }
  else
    { //secondary buffers
      sstCSIM->colBufferAddr = COL_BUFFER_ADDR_SECONDARY(sstCSIM);
      sstCSIM->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY(sstCSIM);      
    }

  buffer = _DIAG_FORCE_BUFFER(sstCSIM, usingPrimaryBuffers, buffer,_DIAG_FORCE_BUFFER_STORE);

  if(useCSIM)
    writePixel(sstCSIM, buffer, x, y, data, cp->memory, "csim");
  else
    writePixel(sstCSIM, buffer, x, y, data, cp->hsim_memory, "hsim");

  
  buffer = _DIAG_FORCE_BUFFER(sstCSIM, usingPrimaryBuffers, buffer,_DIAG_FORCE_BUFFER_RESTORE);

  //Make sure we put things back the way they were
  if(cp->environment.aaPrimaryBuffers)
    {
      sstCSIM->colBufferAddr = COL_BUFFER_ADDR_PRIMARY(sstCSIM);
      sstCSIM->auxBufferAddr = AUX_BUFFER_ADDR_PRIMARY(sstCSIM);
    }
  else
    {
      sstCSIM->colBufferAddr = COL_BUFFER_ADDR_SECONDARY(sstCSIM);
      sstCSIM->auxBufferAddr = AUX_BUFFER_ADDR_SECONDARY(sstCSIM);      
    }
}


FxU32 memoryReadWorker(FxI32 chipIndex, FxI32 addr, FxI32 nBytes, FxBool useCSIM)
{
  SstRegs *sstCSIM=getChipSstCSIM(chipIndex);
  CsimPrivate *cp=CSIM_PRIVATE(sstCSIM);
  FxU32 data=0xC0FFEE;
  volatile FxU8 *memory;
  char *name;
  char csimName[] = "csim";
  char hsimName[] = "hsim";

  if(useCSIM)
    {
      memory = cp->memory;
      name = csimName;
    }
  else
    {
      memory = cp->hsim_memory;
      name = hsimName;
    }
  
  switch(nBytes) 
    {
    case 1: 
      data = readMem8(memory, addr, cp->memorySizeInBytes, name);
      break;
    case 2: 
      data = readMem16(memory, addr, cp->memorySizeInBytes, name);
      break;
    case 4: 
      data = readMem32(memory, addr, cp->memorySizeInBytes, name);
      break;
    default:
      GDBG_ERROR("memoryReadWorker","%s invalid nbytes of %d\n",name, nBytes); 
      break;
    }
  
  return data;
}

void memoryWriteWorker(FxI32 chipIndex, FxI32 addr, FxU32 data, FxI32 nBytes, FxBool useCSIM)
{
  SstRegs *sstCSIM=getChipSstCSIM(chipIndex);
  CsimPrivate *cp=CSIM_PRIVATE(sstCSIM);
  volatile FxU8 *memory;
  char *name;
  char csimName[] = "csim";
  char hsimName[] = "hsim";

  if(useCSIM)
    {
      memory = cp->memory;
      name = csimName;
    }
  else
    {
      memory = cp->hsim_memory;
      name = hsimName;
    }
  
  switch(nBytes) 
    {
    case 1: 
      writeMem8(memory, addr, data, cp->memorySizeInBytes, name);
      break;
    case 2: 
      writeMem16(memory, addr, data, cp->memorySizeInBytes, name);
      break;
    case 4: 
      writeMem32(memory, addr, data, cp->memorySizeInBytes, name);
      break;
    default:
      GDBG_ERROR("memoryWriteWorker","%s invalid nbytes of %d\n", name, nBytes); 
      break;
  }  
}

 
//----------------------------------------------------------------------
//-----------------------  CSIM Section --------------------------------
//-----------------------  CSIM Section --------------------------------

// NOTE: for 3D color buffers the READ routines convert to 888 RGB
// NOTE: color data for the WRITE routines is already in native format
FxU32 CSIM_PIXEL_RD(FxI32 buffer, int x, int y)
{
  return(csimPixelRead(0, buffer, x, y));
}

void CSIM_PIXEL_WR(FxI32 buffer, int x, int y, FxU32 data)
{
  csimPixelWrite(0, buffer, x, y, data);
}

// backdoor read from csim memory
FxU32 CSIM_MEM_RD(int addr, int nbytes)
{
  return(csimMemoryRead(0, addr, nbytes));
}

// backdoor write to csim memory
void CSIM_MEM_WR(int addr, FxU32 data, int nbytes)
{
  csimMemoryWrite(0, addr, data, nbytes);
}



FxU32 csimPixelRead(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y)
{
  return(pixelReadWorker(sampleIndex, buffer, x, y, FXTRUE));
}

FxU32 csimPixelReadCompositeBuffer(FxI32 buffer, int x, int y)
{
  return(compositePixelReadWorker(buffer, x, y, FXTRUE));
}

void csimPixelWrite(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y, FxU32 data)
{
  pixelWriteWorker(sampleIndex, buffer, x, y, data, FXTRUE);
}


FxU32 csimMemoryRead(FxI32 chipIndex, FxI32 addr, FxI32 nBytes)
{
  return(memoryReadWorker(chipIndex, addr, nBytes, FXTRUE));
}

void  csimMemoryWrite(FxI32 chipIndex, FxI32 addr, FxU32 data, FxI32 nBytes)
{
  memoryWriteWorker(chipIndex, addr, data, nBytes, FXTRUE);
}

//----------------------------------------------------------------------
//-----------------------  HSIM Section --------------------------------
//-----------------------  HSIM Section --------------------------------
FxU32 HSIM_PIXEL_RD(FxI32 buffer, int x, int y)
{
  return(hsimPixelRead(0, buffer, x, y));
}

void HSIM_PIXEL_WR(FxI32 buffer, int x, int y, FxU32 data)
{
  hsimPixelWrite(0, buffer, x, y, data);
}

// backdoor read from hsim memory
FxU32 HSIM_MEM_RD(int addr, int nbytes)
{
  return(hsimMemoryRead(0, addr, nbytes));
}

// backdoor write to hsim memory
void HSIM_MEM_WR(int addr, FxU32 data, int nbytes)
{
  hsimMemoryWrite(0, addr, data, nbytes);
}

FxU32 hsimPixelRead(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y)
{
  return(pixelReadWorker(sampleIndex, buffer, x, y, FXFALSE));
}

FxU32 hsimPixelReadCompositeBuffer(FxI32 buffer, int x, int y)
{
  return(compositePixelReadWorker(buffer, x, y, FXFALSE));
}

void  hsimPixelWrite(FxI32 sampleIndex, FxI32 buffer, FxI32 x, FxI32 y, FxU32 data)
{
  pixelWriteWorker(sampleIndex, buffer, x, y, data, FXFALSE);
}


FxU32 hsimMemoryRead(FxI32 chipIndex, FxI32 addr, FxI32 nBytes)
{
  return(memoryReadWorker(chipIndex, addr, nBytes, FXFALSE));
}

void  hsimMemoryWrite(FxI32 chipIndex, FxI32 addr, FxU32 data, FxI32 nBytes)
{
  memoryWriteWorker(chipIndex, addr, data, nBytes, FXFALSE);
}

  
//----------------------------------------------------------------------
//-----------------------  HW Section ----------------------------------
//-----------------------  HW Section ----------------------------------
#include "../csim/h3asm.h"

// GMT: we make a gross assumption that we are only dealing with one board
//      at a time when we are running diags, and that it is number 0
//      this macro returns the actual HW address even when we are running CSIM
//#define sstRealHW() (FxU32 *)(diago.halInfo->boardInfo[0].sstHW)
//#define sstRealHWRawLfb() (FxU8 *)(diago.halInfo->boardInfo[0].physAddr[1])

FxU32* sstRealHW(FxU32 chipIndex)
{
  assert(chipIndex < (FxU32)diago.chipCount);

  return((FxU32 *)diago.halInfo->boardInfo[chipIndex].sstHW);
}

FxU8* sstRealHWRawLfb(FxU32 chipIndex)
{
  assert(chipIndex < (FxU32)diago.chipCount);

  return((FxU8 *)(diago.halInfo->boardInfo[chipIndex].physAddr[1]));
}

static int fastMode = 0;
static FxU32 lfbModeSave, fbzModeSave, lfbMemoryConfigSave;

void HW_PIXEL_FAST_BEGIN(FxU32 chipIndex, FxI32 buffer)
{
    FxU32 *sst = sstRealHW(chipIndex);

    // force raw lfb memory to be all linear
    lfbMemoryConfigSave = sst[LFBMEMORYCONFIG>>2];
    sst[LFBMEMORYCONFIG>>2] = (~0x0) & SST_RAW_LFB_TILE_BEGIN_PAGE;

    sst_idle_really(diago.sst);               // wait for idle after changing mode regs
    fastMode = 1;
}

// restore the real HW's lfbMode and fbzMode registers
void HW_PIXEL_FAST_END(FxU32 chipIndex)
{
    FxU32 *sst = sstRealHW(chipIndex);
    sst[LFBMEMORYCONFIG>>2] = lfbMemoryConfigSave; 
    fastMode = 0;
}

// write a hardware pixel bypassing the simulator
void HW_PIXEL_WR(FxI32 buffer, int x, int y, FxU32 data)
{
  FxU32 *sst = sstRealHW(0);
  FxU8 *sstRawLfb = sstRealHWRawLfb(0);
  int _fastMode = fastMode;
  
  if (!_fastMode) HW_PIXEL_FAST_BEGIN(0, buffer);
  
#ifdef HAL_CSIM
  FXUNUSED(sst);
  buffer = _DIAG_FORCE_BUFFER(diago.sstCSIM, FXTRUE, buffer,_DIAG_FORCE_BUFFER_STORE);
  writePixel(diago.sstCSIM,buffer,x,y,data,sstRawLfb,"hw");
  buffer = _DIAG_FORCE_BUFFER(diago.sstCSIM, FXTRUE, buffer,_DIAG_FORCE_BUFFER_RESTORE);
#else
  { 
    static int i = 0;
    if ( i++ == 0 )
      GDBG_INFO(0,"HW_PIXEL_WR disabled because CSIM isn't available\n");
  }
#endif
  
  if (!_fastMode) HW_PIXEL_FAST_END(0);
}

// read a hardware pixel bypassing the simulator
FxU32 HW_PIXEL_RD(FxI32 buffer, int x, int y)
{
  FxU32 data, *sst = sstRealHW(0);
  FxU8 *sstRawLfb = sstRealHWRawLfb(0);
  int _fastMode = fastMode;
  
  if (!_fastMode) HW_PIXEL_FAST_BEGIN(0, buffer);
  
#ifdef HAL_CSIM
  FXUNUSED(sst);
  buffer = _DIAG_FORCE_BUFFER(diago.sstCSIM, FXTRUE, buffer,_DIAG_FORCE_BUFFER_STORE);
  data = readPixel(diago.sstCSIM,buffer,x,y,sstRawLfb,"hw");
  buffer = _DIAG_FORCE_BUFFER(diago.sstCSIM, FXTRUE, buffer,_DIAG_FORCE_BUFFER_RESTORE);
  if (buffer == CSIM_BUF_3D_FRONT || 
      buffer == CSIM_BUF_3D_BACK || 
      buffer == CSIM_BUF_3D_TRIPLE) 
    // expand to 32-bit ARGB
    data = ((data & 0xF800) << 8) | ((data & 0x07E0) << 5) |((data & 0x001F) << 3);
#else
  { 
    static int i = 0;
    if ( i++ == 0 )
      GDBG_INFO(0,"HW_PIXEL_RD disabled because CSIM isn't available\n");
  }
#endif

  if (!_fastMode) HW_PIXEL_FAST_END(0);
  return data;
}

// backdoor write to hw memory
void HW_MEM_WR(FxU32 chipIndex, int addr, FxU32 data, int nbytes)
{
  int _fastMode = fastMode;
  
  FxU8 *sstRawLfb = sstRealHWRawLfb(chipIndex);
  CsimPrivate *cp = CSIM_PRIVATE(diago.sstCSIM);
  
  if (!_fastMode) HW_PIXEL_FAST_BEGIN(chipIndex, 0);
  
  switch (nbytes) {
  case 1: ((FxU8 *)sstRawLfb)[addr] = (FxU8) data; break;
  case 2: ((FxU16 *)sstRawLfb)[addr>>1] = (FxU16) data; break;
  case 4: ((FxU32 *)sstRawLfb)[addr>>2] = (FxU32) data; break;
  default:
    GDBG_ERROR("HW_MEM_WR","invalid nbytes of %d\n",nbytes); break;
  }
  GDBG_INFO(199,"HW_MEM_WR[0x%x] <= 0x%x \n",addr,data);     

  if (!_fastMode) HW_PIXEL_FAST_END(chipIndex);
}

// backdoor write to hw memory
void HW_MEM_WR2(int addr, FxU8 *data, int nbytes)
{
  int i, _fastMode = fastMode;
  
  FxU8 *sstRawLfb = sstRealHWRawLfb(0);
  CsimPrivate *cp = CSIM_PRIVATE(diago.sstCSIM);
  
  if ( GDBG_GET_DEBUGLEVEL(199) ) 
    for ( i=0; i<nbytes; i++ ) 
      GDBG_INFO(199,"HW_MEM_WR2[0x%x] <= 0x%x \n",addr+i,data[i]);
  
  if (!_fastMode) HW_PIXEL_FAST_BEGIN(0, 0);

  memcpy(sstRawLfb+addr,data,nbytes);

  if (!_fastMode) HW_PIXEL_FAST_END(0);
}

// backdoor read from hw memory
FxU32 HW_MEM_RD(FxU32 chipIndex, int addr, int nbytes)
{
  FxU32 data = 0;
  int _fastMode = fastMode;

  FxU8 *sstRawLfb = sstRealHWRawLfb(chipIndex);
  CsimPrivate *cp = CSIM_PRIVATE(diago.sstCSIM);
    
  if (!_fastMode) HW_PIXEL_FAST_BEGIN(chipIndex, 0);

  switch (nbytes) {
  case 1: data = ((FxU8 *)sstRawLfb)[addr]; break;
  case 2: data = ((FxU16 *)sstRawLfb)[addr>>1]; break;
  case 4: data = ((FxU32 *)sstRawLfb)[addr>>2]; break;
  default:
    GDBG_ERROR("HW_MEM_RD","invalid nbytes of %d\n",nbytes); break;
  }
  GDBG_INFO(199,"HW_MEM_RD[0x%x] => 0x%x \n",addr,data);     

  if (!_fastMode) HW_PIXEL_FAST_END(chipIndex);

  return data;
}

// backdoor read from hw memory
void HW_MEM_RD2(int addr, FxU8 *data, int nbytes)
{
  int i, _fastMode = fastMode;
  
  FxU8 *sstRawLfb = sstRealHWRawLfb(0);
  CsimPrivate *cp = CSIM_PRIVATE(diago.sstCSIM);
  
  if (!_fastMode) HW_PIXEL_FAST_BEGIN(0, 0);
  
  memcpy(data,sstRawLfb+addr,nbytes);
  
  if (!_fastMode) HW_PIXEL_FAST_END(0);
  
  if ( GDBG_GET_DEBUGLEVEL(199) ) 
    for ( i=0; i<nbytes; i++ ) 
      GDBG_INFO(199,"HW_MEM_RD2[0x%x] => 0x%x \n",addr+i,data[i]);
}

// backdoor hw memory comparison
FxU32 HW_MEM_CMP2(FxU32 chipIndex, int addr, FxU8 *data, int nbytes)
{
  int i, nwords, _fastMode = fastMode;

  FxU8 *sstRawLfb;
  FxU32 *hw32, *d32;
  FxU8 *hw8;
  FxU32 result = 0;

  CsimPrivate *cp;

  cp = CSIM_PRIVATE(diago.sstCSIM);  
  sstRealHWRawLfb(chipIndex);
  sstRawLfb = sstRealHWRawLfb(chipIndex);

  if (!_fastMode) HW_PIXEL_FAST_BEGIN(0, 0);

  // head -- leading bytes
  hw8 = (FxU8*) (sstRawLfb+addr);
  while ( addr & 0x3 ) {  // unaligned
    result |= (*data++) ^ (*hw8++);
    addr++;
    nbytes--;
  }

  // body -- full words
  hw32 = (FxU32*) (sstRawLfb+addr);
  d32 = (FxU32*) data;
  nwords = nbytes/4;
  for ( i=0; i<nwords; i++ )
    result |= (*d32++) ^ (*hw32++);
  addr += nwords*4;
  nbytes -= nwords*4;
  data += nwords*4;

  // tail -- trailing bytes
  hw8 = (FxU8*) (sstRawLfb+addr);
  while (nbytes>0) {
    result |= (sstRawLfb[addr++]) ^ (*hw8++);
    nbytes--;
  }
  
  if (!_fastMode) HW_PIXEL_FAST_END(0);

  return result;
}
