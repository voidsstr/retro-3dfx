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
** $Revision: 11$
** $Date: 10/11/00 7:39:47 PM$
*/

#include <stdlib.h>
#include <fxhwc.h>
#include <fx64.h>

#define GDBG_LEVEL 198

/*---------------------------------------------------------------------------
   AGP memory is a global resource, create AGP memory based on a fake address
   to ensure writes go through HWC.
   XXX: add support for real hardware
  ---------------------------------------------------------------------------*/
FX_EXPORT FxBool FX_CSTYLE
hwcAGPInit(void)
{
  FxU32 maxAddr = 0-hwcInfo.agpSizeInBytes; 
  GDBG_INFO(1,"hwcAGPInit\n");

  _hwcInitMemRegion(&hwcInfo.agpMemReg, 0, hwcInfo.agpSizeInBytes);
#ifdef HWC_HW
  GDBG_ERROR("hwcAGPInit", "HWC AGP support nyi for hardware\n");
#endif

  hwcInfo.agpVirtAddr = (FxU8 *) (1<<28);
  hwcInfo.agpRqDepth = 0x0;

  GDBG_INFO(3,"Base hi:lo = 0x%01x:%08x, size = %d bytes\n",
	        hwcInfo.agpBaseAddrH,hwcInfo.agpBaseAddrL,hwcInfo.agpSizeInBytes);

  return FXTRUE;
}

FX_EXPORT FxU32 FX_CSTYLE
hwcAGPWriteMem32(volatile void *_virtAddr, FxU32 data) 
{
#define FN_NAME "hwcAGPWriteMem32"
  FxU8 *virtAddr = (FxU8 *)_virtAddr;
  FxU32 offset, bn;
  HwcContext *hwc;

  GDBG_INFO(3,"%s(0x%lx(0x%lx), 0x%lx)\n", FN_NAME, 
            _virtAddr, (FxU32)_virtAddr-(FxU32)hwcInfo.agpVirtAddr+hwcInfo.agpBaseAddrL , data);

  /* do some sanity checks */

  if (virtAddr < hwcInfo.agpVirtAddr ) {
    GDBG_ERROR(FN_NAME,"bad address=0x%x  data=%d(0x%08x)\n",
	       virtAddr,data,data);
    return data;
  }

  if ( virtAddr >= ( hwcInfo.agpVirtAddr + hwcInfo.agpSizeInBytes)) {
    GDBG_ERROR(FN_NAME,"virtual address 0x%08x out of range\n",virtAddr);
    return data;
  }

  if ( (FxU32)virtAddr & 0x3 ) {
    GDBG_ERROR(FN_NAME,"misaligned address=0x%x  data=%d(0x%08x)\n",
	       virtAddr,data,data);
    return data;
  }

  offset = HWC_FAKE_ADDRESS_GET_OFFSET(virtAddr);

  if (GDBG_GET_DEBUGLEVEL(GDBG_LEVEL))
    GDBG_INFO(GDBG_LEVEL,"%s(0x%08x,0x%08x)\n", FN_NAME, virtAddr,data);

  /* write data to each simulators version of agp memory for each mapped board */

  for (bn = 0; bn < HWC_MAX_BOARDS; bn++) {
    hwc = &hwcInfo.boardInfo[bn];
    if ( hwc->state == HWC_CTX_MAPPED ) {
      HwcSimulator *hws ;

      if ( !hwc->agpEnabled ) {
        GDBG_ERROR(FN_NAME,"AGP not enabled=0x%x  data=%d(0x%08x)\n",
	               virtAddr,data,data);
        continue;
      }

      for (hws = hwc->hws; hws; hws = hws->next ) {
        hwcIoVector(hws, HWC_AGP_WRITE, 32, (FxU32) virtAddr, data);
#ifdef ENDB
        hws->agpMem[offset] = (FxU8)(data & 0xFF);
        hws->agpMem[offset+1] = (FxU8)((data>>8) & 0xFF);
        hws->agpMem[offset+2] = (FxU8)((data>>16) & 0xFF);
        hws->agpMem[offset+3] = (FxU8)((data>>24) & 0xFF);
#else
        (* hws->WriteAGPMemory)(hws, offset, data);
        // ((FxU32 *)hws->agpMem)[offset>>2] = data;
#endif
      }
    } 
  }
  return data;
#undef FN_NAME
}

/*----------------------------------------------------------------------
   same as hwcAGPWriteMem32 except data is a float 
  ----------------------------------------------------------------------*/
FX_EXPORT FxFloat FX_CSTYLE
hwcAGPWriteMem32f(volatile void *addr, FxFloat data)
{
	hwcAGPWriteMem32(addr, *(FxU32 *) & data);

	return data;
}

/* read a 32-bit value from agp memory at the given virtual address */
// XXX we should add a function to test if results are same, also to read
// data from one specific simulator or device

FX_EXPORT FxU32 FX_CSTYLE
hwcAGPReadMem32(volatile void *_virtAddr)
{
#define FN_NAME "hwcAGPReadMem32"
  FxU8 *virtAddr = (FxU8 *)_virtAddr;
  FxU32 offset, bn, data = 0xdeadbeef;
  HwcContext *hwc;

  GDBG_INFO(3,"%s(0x%lx(0x%lx))\n", FN_NAME, 
            _virtAddr, (FxU32)_virtAddr-(FxU32)hwcInfo.agpVirtAddr+hwcInfo.agpBaseAddrL);

  /* do some sanity checks */

  if (virtAddr < hwcInfo.agpVirtAddr ) {
    GDBG_ERROR(FN_NAME,"bad address=0x%x  data=%d(0x%08x)\n",
	       virtAddr,data,data);
    return data;
  }

  if ( virtAddr >= ( hwcInfo.agpVirtAddr + hwcInfo.agpSizeInBytes)) {
    GDBG_ERROR(FN_NAME,"virtual address 0x%08x out of range\n",virtAddr);
    return data;
  }

  if (  (FxU32)virtAddr & 0x3 ) {
    GDBG_ERROR(FN_NAME,"misaligned address=0x%x\n", virtAddr);
    return 0;
  }

  if (GDBG_GET_DEBUGLEVEL(GDBG_LEVEL))
    GDBG_INFO(GDBG_LEVEL,"%s(0x%08x)\n",FN_NAME, virtAddr);
  
  offset = HWC_FAKE_ADDRESS_GET_OFFSET(virtAddr);

  /* read data from each simulators version of agp memory for each mapped board */

  for (bn = 0; bn < HWC_MAX_BOARDS; bn++) {
    hwc = &hwcInfo.boardInfo[bn];
    if ( hwc->state == HWC_CTX_MAPPED ) {
      HwcSimulator *hws ;

      if ( !hwc->agpEnabled ) {
        GDBG_ERROR("%s","AGP not enabled=0x%x  data=%d(0x%08x)\n",
	               FN_NAME, virtAddr,data,data);
        continue;
      }

      for (hws = hwc->hws; hws; hws = hws->next ) {
#ifdef ENDB
          data = ( hws->agpMem[offset]
                   | hws->agpMem[offset+1] << 8
                   | hws->agpMem[offset+2] << 16
                   | hws->agpMem[offset+3] << 24 );
#else
          (* hws->ReadAGPMemory)(hws, offset, &data);
          // data = ((FxU32 *)hws->agpMem)[offset>>2];
#endif
	      hwcIoVector(hws, HWC_AGP_READ, 32, (FxU32) virtAddr, data);
      }
    }
  }
  
  if (GDBG_GET_DEBUGLEVEL(GDBG_LEVEL))
    GDBG_INFO(GDBG_LEVEL,"%s(0x%08x) => 0x%08x\n",FN_NAME, offset,data);
  
  return data;
#undef FN_NAME
}

/*
   convert from a 36-bit AGP physical address to the corresponding
   32-bit virtual address
*/
FX_EXPORT FxU32 * FX_CSTYLE
hwcAGPPhysToVirt( FxU32 physAddrH, FxU32 physAddrL )
{
  FxU32 maxPhysH, maxPhysL;
  FxU32 minPhysH, minPhysL;
  FxU32 *virtAddr;
  FxI64 phys, base, delta;
  int found = 0;

  GDBG_INFO(3,"hwcAGPPhysToVirt(0x%lx, 0x%lx)\n", physAddrH, physAddrL);

  /* lower bound of AGP phys addr */
  minPhysH = hwcInfo.agpBaseAddrH;
  minPhysL = hwcInfo.agpBaseAddrL;
    
  /* upper bound of AGP phys addr */
  maxPhysH = minPhysH;
  maxPhysL = minPhysL + hwcInfo.agpSizeInBytes;
  if ( maxPhysL < minPhysL )       /* detect overflow */
    maxPhysH++;
    
  /* check that physAddr is within [minPhys,maxPhys) */
    
  if ( ((physAddrH > minPhysH) || (physAddrH == minPhysH && physAddrL >= minPhysL)) &&
	 ((physAddrH < maxPhysH) || (physAddrH == maxPhysH && physAddrL < maxPhysL)) ) {
      found = 1;
  } else {
    GDBG_ERROR("agpPhysToVirt","physical address H:L 0x%01x:0x%08x out of range\n",
	       physAddrH,physAddrL);
    return (FxU32 *)0xdeadbeef;
  }
  
  /* compute the offset, phys - base */
  
  FX_SET64(phys,physAddrH,physAddrL);
  FX_SET64(base,hwcInfo.agpBaseAddrH,hwcInfo.agpBaseAddrL);
  delta = FX_SUB64(phys,base);  /* must fit in 32 bits since agpSizeInBytes is only 32 bits */
  if ( FX_HI64(delta) ) {    /* must fit in 32 bits since agpSizeInBytes is only 32 bits */
    GDBG_ERROR("agpPhysToVirt","internal error, physAddr - baseAddr = 0x%08x:%08x\n",
	       FX_HI64(delta),FX_LO64(delta));
    return (FxU32 *)0xdeadbeef;
  }

  virtAddr = (FxU32 *) &(hwcInfo.agpVirtAddr[ FX_LO64(delta) ]);

  if (GDBG_GET_DEBUGLEVEL(GDBG_LEVEL))
    GDBG_INFO(GDBG_LEVEL,"agpPhysToVirt Phys(0x%08x,0x%08x) => Virt(0x%08x)\n",physAddrH,physAddrL,virtAddr);

  return virtAddr;
}

/*
   convert from a 32-bit AGP virtual address to the corresponding
   36-bit physical address
*/
FX_EXPORT void FX_CSTYLE
hwcAGPVirtToPhys( void *_virtAddr, FxU32 *physAddrH, FxU32 *physAddrL ) 
{
  FxU8 *virtAddr = (FxU8 *)_virtAddr;
  FxU32 offset;

  GDBG_INFO(3,"hwcAGPVirtToPhys(0x%lx)\n", _virtAddr);

  /* do some sanity checks */

  if (virtAddr < hwcInfo.agpVirtAddr ) {
    GDBG_ERROR("hwcAGPVirtToPhys","bad address=0x%x)\n", virtAddr);
    return;
  }

  offset = virtAddr-hwcInfo.agpVirtAddr;

  if ( offset > hwcInfo.agpSizeInBytes ) {
    GDBG_ERROR("hwcAGPVirtToPhys","virtual address 0x%08x out of range offset %x\n",
               virtAddr, offset);
    *physAddrH = *physAddrL = 0;
    *physAddrL = 0xdeadbeef;
    return;
  }

  *physAddrL = hwcInfo.agpBaseAddrL + offset;
  *physAddrH = hwcInfo.agpBaseAddrH;
  
  if ( *physAddrL < offset )
    (*physAddrH)++;

  if (GDBG_GET_DEBUGLEVEL(GDBG_LEVEL))
    GDBG_INFO(GDBG_LEVEL,"agpVirtToPhys Virt(0x%08x) => Phys(0x%08x,0x%08x)\n",virtAddr,*physAddrH,*physAddrL);
}
