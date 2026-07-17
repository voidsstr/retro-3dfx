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
** $Date: 10/11/00 8:30:56 PM$
*/

#include <stdlib.h>
#include <h3.h>
#include "../csim/csim.h"
#include <fx64.h>

#undef ENDB  // AGP PLI module expects native endian format
#define GDBG_LEVEL 198
#define PRIVATE static
#define PUBLIC
typedef unsigned long boolean;
#define true 1
#define false 0

PRIVATE FxU32 agpFree; // AGP Free location in bytes
//
// Init agp memory
//
FX_EXPORT void FX_CSTYLE
agpMemInit() 
{
  agpFree = 0;
}

//
// Allocate agp memory segment of requested size
//
FX_EXPORT FxU32 * FX_CSTYLE
agpMemAlloc(SstRegs *sst,unsigned sizeBytes)
{
  FxU32 bn;
  FxDeviceInfo *info;
  FxU32 addr;

  sizeBytes = (sizeBytes+7) & ~0x7;
  if (fxHalVaddrToBoardNumber( sst, &bn ))	// find the board
    info = &halInfo.boardInfo[bn];
  else
    return(NULL);
  if (agpFree+sizeBytes > info->agpSizeInBytes) {
    GDBG_ERROR("agpMemAlloc","Not enough agp memory left (%d bytes) to allocate %d bytes\n",
	       info->agpSizeInBytes-agpFree,sizeBytes);
    return(NULL);
  }
  addr = (FxU32)info->agpVirtAddr + agpFree;
  agpFree += sizeBytes;
  GDBG_INFO(120,"agpMemAlloc: Allocating %d bytes of agp memory at 0x%x\n",sizeBytes,addr);
  return((FxU32 *)addr);
}


//
// write a 32-bit value into agp memory at the given virtual address
//
FX_EXPORT void FX_CSTYLE
agpWriteMem32(FxU32 *_virtAddr, FxU32 data) 
{
  FxU32 offset, bn, virtAddr = (FxU32)_virtAddr;
  FxDeviceInfo *info;

  // do some sanity checks
  if (SST_BAD_ADDRESS(virtAddr)) {
    GDBG_ERROR("agpWriteMem32","bad address=0x%x  data=%d(0x%08x)\n",
	       virtAddr,data,data);
    return;
  }
  bn = SST_FAKE_ADDRESS_GET_BOARD(virtAddr);

  offset = SST_FAKE_ADDRESS_GET_OFFSET(virtAddr);
  info = &halInfo.boardInfo[bn];

  if ( offset > info->agpSizeInBytes ) {
    GDBG_ERROR("agpWriteMem32","virtual address 0x%08x out of range\n",virtAddr);
    return;
  }

  if ( offset & 3 ) {
    GDBG_ERROR("agpWriteMem32","unaligned 32-bit write 0x%x\n",virtAddr);
    return;
  }

  if (GDBG_GET_DEBUGLEVEL(GDBG_LEVEL))
    GDBG_INFO(GDBG_LEVEL,"agpWriteMem32(0x%08x,0x%08x)\n",virtAddr,data);

#ifdef ENDB
  info->agpMem[offset] = (FxU8)(data & 0xFF);
  info->agpMem[offset+1] = (FxU8)((data>>8) & 0xFF);
  info->agpMem[offset+2] = (FxU8)((data>>16) & 0xFF);
  info->agpMem[offset+3] = (FxU8)((data>>24) & 0xFF);
#else
  ((FxU32 *)info->agpMem)[offset>>2] = data;
#endif

}

//
// read a 32-bit value from agp memory at the given virtual address
//
FX_EXPORT FxU32 FX_CSTYLE
agpReadMem32(FxU32 *_virtAddr)
{
  FxU32 offset, data, bn, virtAddr = (FxU32)_virtAddr;
  FxDeviceInfo *info;
  
  // do some sanity checks
  if (SST_BAD_ADDRESS(virtAddr)) {
    GDBG_ERROR("agpReadMem32","bad address=0x%x\n",virtAddr);
    return 0;
  }
  bn = SST_FAKE_ADDRESS_GET_BOARD(virtAddr);

  offset = SST_FAKE_ADDRESS_GET_OFFSET(virtAddr);
  info = &halInfo.boardInfo[bn];

  if ( offset > info->agpSizeInBytes ) {
    GDBG_ERROR("agpReadMem32","virtual address 0x%08x out of range\n");
    return 0;
  }

  if ( offset & 3 ) {
    GDBG_ERROR("agpReadMem32","unaligned 32-bit read 0x%x\n",offset);
    return 0;
  }
  
#ifdef ENDB
  data = ( info->agpMem[offset] 
	   | info->agpMem[offset+1] << 8
	   | info->agpMem[offset+2] << 16
	   | info->agpMem[offset+3] << 24 );
#else
  data = ((FxU32 *)info->agpMem)[offset>>2];
#endif
  
  if (GDBG_GET_DEBUGLEVEL(GDBG_LEVEL))
    GDBG_INFO(GDBG_LEVEL,"agpReadMem32(0x%08x) => 0x%08x\n",offset,data);
  
  return data;
}

//
// convert from a 36-bit AGP physical address to the corresponding
// 32-bit virtual address
//
FX_EXPORT FxU32 * FX_CSTYLE
agpPhysToVirt( FxU32 physAddrH, FxU32 physAddrL )
{
  FxU32 maxPhysH, maxPhysL;
  FxU32 minPhysH, minPhysL;
  FxU32 *virtAddr;
  FxI64 phys, base, delta, bn;
  FxDeviceInfo *info;
  int found = 0;

  for ( bn=0; bn<HAL_MAX_BOARDS; bn++ ) {
    info = &halInfo.boardInfo[bn];

    // lower bound of AGP phys addr
    minPhysH = info->agpBaseAddrH;
    minPhysL = info->agpBaseAddrL;
    
    // upper bound of AGP phys addr
    maxPhysH = minPhysH;
    maxPhysL = minPhysL + info->agpSizeInBytes;
    if ( maxPhysL < minPhysL )       // detect overflow
      maxPhysH++;
    
    // check that physAddr is within [minPhys,maxPhys)
    
    if ( ((physAddrH > minPhysH) || (physAddrH == minPhysH && physAddrL >= minPhysL)) &&
	 ((physAddrH < maxPhysH) || (physAddrH == maxPhysH && physAddrL < maxPhysL)) ) {
      found = 1;
      break;
    }
  }

  if ( !found ) {
    GDBG_ERROR("agpPhysToVirt","physical address H:L 0x%01x:0x%08x out of range\n",
	       physAddrH,physAddrL);
    return (FxU32 *)0xdeadbeef;
  }
  
  // compute the offset, phys - base
  
  FX_SET64(phys,physAddrH,physAddrL);
  FX_SET64(base,info->agpBaseAddrH,info->agpBaseAddrL);
  delta = FX_SUB64(phys,base);        // must fit in 32 bits since agpSizeInBytes is only 32 bits
  if ( FX_HI64(delta) ) {    // must fit in 32 bits since agpSizeInBytes is only 32 bits
    GDBG_ERROR("agpPhysToVirt","internal error, physAddr - baseAddr = 0x%08x:%08x\n",
	       FX_HI64(delta),FX_LO64(delta));
    return (FxU32 *)0xdeadbeef;
  }

  virtAddr = (FxU32 *) &(info->agpVirtAddr[ FX_LO64(delta) ]);

  if (GDBG_GET_DEBUGLEVEL(GDBG_LEVEL))
    GDBG_INFO(GDBG_LEVEL,"agpPhysToVirt Phys(0x%08x,0x%08x) => Virt(0x%08x)\n",physAddrH,physAddrL,virtAddr);

  return virtAddr;

}

//
// convert from a 32-bit AGP virtual address to the corresponding
// 36-bit physical address
//
FX_EXPORT void FX_CSTYLE
agpVirtToPhys( FxU32 *virtAddr, FxU32 *physAddrH, FxU32 *physAddrL ) 
{
  FxU32 offset, bn;
  FxDeviceInfo *info;

  // do some sanity checks
  if (SST_BAD_ADDRESS(virtAddr)) {
    GDBG_ERROR("agpVirtToPhys","bad virtual address=0x%08x\n",virtAddr);
    *physAddrL = 0xdeadbeef;
    return;
  }
  bn = SST_FAKE_ADDRESS_GET_BOARD(virtAddr);

  offset = SST_FAKE_ADDRESS_GET_OFFSET(virtAddr);
  info = &halInfo.boardInfo[bn];

  if ( offset > info->agpSizeInBytes ) {
    GDBG_ERROR("agpVirtToPhys","virtual address 0x%08x out of range offset %x bn %x\n",virtAddr,
	       offset,bn);
    *physAddrH = *physAddrL = 0;
    *physAddrL = 0xdeadbeef;
    return;
  }

  *physAddrL = info->agpBaseAddrL + offset;
  *physAddrH = info->agpBaseAddrH;
  
  if ( *physAddrL < offset )
    (*physAddrH)++;

  if (GDBG_GET_DEBUGLEVEL(GDBG_LEVEL))
    GDBG_INFO(GDBG_LEVEL,"agpVirtToPhys Virt(0x%08x) => Phys(0x%08x,0x%08x)\n",virtAddr,*physAddrH,*physAddrL);
}


