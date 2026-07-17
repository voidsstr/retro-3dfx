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
 **
 ** $Revision: 4$ 
 ** $Date: 10/11/00 7:38:39 PM$ 
 **
 */

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <3dfx.h>
#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <fxmemmap.h>
#include "fxpci.h"


/* Stuff for programming MSRs */
typedef struct MSRInfo_s {
  unsigned long
    msrNum,                     /* Which MSR? */
    msrLo, msrHi;               /* MSR Values  */
} MSRInfo;

/*---------------------------------------------------------------------------
**
**  MTRR Code
**
**  It's possible that this code belongs elsewhere, but since it
**  communicates with the VXD, it needs to be in here for now.  If at
**  some later date, we rearchitecth the whole VXD thing, then we
**  should revisit this code and find a good place for it.
*----------------------------------------------------------------------------*/

#if !defined(MAPPL_MAPMEM_MINIPORT)

#ifdef MAPPL_DPMI
#define DOGETMSR(a,b) \
DpmiGetMSR((FxU32) &a, (FxU32) &b);


#define DOSETMSR(a,b) \
DpmiSetMSR((FxU32) &a, (FxU32) &b);

#elif defined(MAPPL_MEMMAP_VXD)

HANDLE hMemmapFile;

/* NOTE THIS IS TRICKY:  nret MUST be defined by the calling routine!!! */
#define DOGETMSR(a,b) \
DeviceIoControl( hMemmapFile, GETMSR, &a, sizeof(a), &b, sizeof(b), &nret, NULL);

#define DOSETMSR(a,b) \
DeviceIoControl( hMemmapFile, SETMSR, &a, sizeof(a), &b, sizeof(b), &nret, NULL);
#endif


#define MTRR_VALID         0x800
#define MTRR_BASE_FIRST    0x200
#define MTRR_MASK_FIRST    0x201
#define MTRR_BASE_LAST     0x20a
#define MTRR_MASK_LAST     0x20b

#define MTRR_FIRST         0
#define MTRR_LAST          5

#define MTRRPHYSBASE2MSR(mtrr)  ((mtrr) + MTRR_BASE_FIRST)
#define MTRRPHYSMASK2MSR(mtrr)  ((mtrr) + MTRR_MASK_FIRST)
#define MSR2MTRR(msr)      (((msr) - MTRR_BASE_FIRST) & 0xfffffffe)

/*
**  A Note On MTRRs:
**
**  MTRRs are special cases of MSRs (model specific registers).  They
**  work in pairs, specified as follows (the number is the MSR number):
*/
#define MTRR_PHYSBASE_0    0x200
#define MTRR_PHYSMASK_0    0x201
#define MTRR_PHYSBASE_1    0x202
#define MTRR_PHYSMASK_1    0x203
#define MTRR_PHYSBASE_2    0x204
#define MTRR_PHYSMASK_2    0x205
#define MTRR_PHYSBASE_3    0x206
#define MTRR_PHYSMASK_3    0x207
#define MTRR_PHYSBASE_4    0x208
#define MTRR_PHYSMASK_4    0x209
#define MTRR_PHYSBASE_5    0x20a
#define MTRR_PHYSMASK_5    0x20b

/*
**  So, the PHYSBASE_N is the base address for a particular MTRR and
**  uses a whole MSR.  It has the type encoded in the bottom three
**  nibbles (12 bits).
**
**  The PHYSMASK_N is the physical mask for a particular MTRR and uses
**  the MSR immediately following the MSR which specifies the physical
**  base and type.
**
**  See the headers of the functions genPhysBase and genPhysMask to
**  find out how to generate the PhysBase value and the PhysMask
**  value.  If you want to see the place from which this information
**  was obtained, refer to the document Pentium(r) Pro Family
**  Developer's Manual Volume 3: Operating System Writer's Guide,
**  Chpater 11:  Memory Cache Control.  Note that the terms PhysBase
**  and PhysMask come from there.
*/

/* genPhysBase - Generate PhysBase 
**
**  MTRRPhysBase:
**  Bits 7:0     Memory Type
**  Bits 11:8    Reserved
**  Bits 35:12   PhysBase
**  Bits 63:36   Reserved
**
**  The Intel P6 documentation pines away about how the PhysBase field
**  of this register contains an integer power of two multiple of
**  0x1000, but guess what?  The number (if it has a legal value) is
**  already set up that way, so, we just leave it TF alone, and then
**  OR in the type (the types specified in fxpci.h match the Intel
**  P6 mem types indentically).  
*/
static FxBool 
genPhysBase(FxU32 physBaseAddress, PciMemType type, FxU32 *physBase) 
{
  FxBool
    ret = FXFALSE;

  /* Validate type */
  switch (type) {
  case PciMemTypeUncacheable:
  case PciMemTypeWriteCombining:
  case PciMemTypeWriteThrough:
  case PciMemTypeWriteProtected:
  case PciMemTypeWriteback:
    break;
  default:
    /* Invalid Type */
    return ret;
    break;
  }

  /* Validate base address -- make sure it's on a 4K boundary */
  if (physBaseAddress & 0xfff) { 
    /* Invalid start address */
    return ret;
  }

  /* 
  **  We now have a valid size and type.
  **  Generate the MTRR value for PhysBase.
  */
  *physBase = physBaseAddress | type;

  return (ret = FXTRUE);

} /* genPhysBase */

/*  genPhysMask - Generate PhysMask:
**
**  MTRRPhysMask:
**  Bits 10:0    Reserved
**  Bit  11      Valid
**  Bits 35:12   PhysMask
**  Bits 63:36   Reserved
**
**  The PhysMask field works as follows:
**  from Bit 12 to bit 35, there are N zeros, and m ones, where n is
**  the power of two to multiply by 4069 to get the size you want,
**  and m is 35 - n.   i.e.:
**
**  size = 2^n * 0x1000
**  m = 35 - m
**
**  Since we have the lowBit from above, we know which power of 2
**  the size is, so we simply need to shift it right by 12 to see
**  how many of the bits between 12 an 35 are 0.  We then shift
**  0xffffffff left by that many bits to get the right mask.
**
*/
static FxBool
genPhysMask(FxU32 physSize, FxU32 *physMask)
{
  FxBool
    ret = FXFALSE;

  FxU32
    bitRes,
    lowBit;

  /*
  **  Validate size
  **
  **  Size must be an integer power of 2, and it must be greater than
  **  4K (4096d, 1000h, 10000o, 1000000000000b :-)
  */

  /* Do it this way instead of a mask, so we find zero if it's there */
  if (physSize < 0x1000) {
    /* invalid size */
    return ret;
  }
    
  /*
  ** Make sure it's an integer power of 2:
  ** Any power of 2 (except 0) ANDed with its
  ** integer predecessor is always 0.
  */ 
  if (physSize & (physSize - 1)) {
    return ret;
  }

  /* Find the lowest bit set in physSize */
  lowBit = 11;
  do {
    ++lowBit;    
    bitRes = (physSize >> lowBit) & 1;
  } while ((bitRes == 0) && (lowBit < 0x20));

  
  /* See above for explanation of 0x800 (2**11) */
  *physMask =  (0xffffffff << lowBit) | MTRR_VALID;  
  
  return (ret = FXTRUE);

} /* genPhysMask */

#endif /* MAPPL_MAPMEM_MINIPORT */

/*
**  pciFindMTRRMatch - find an MTRR that matches this one.
**
**  We return the MTRR number which is a special-case MSR calculated
**  in the following manner:
**
**  (matchBaseMSR - baseMTRR0) >> 2
*/
FX_EXPORT FxBool FX_CSTYLE
pciFindMTRRMatch(FxU32 physBaseAddress, FxU32 physSize,
                 PciMemType type, FxU32 *mtrrNum)
{
#ifndef MAPPL_MAPMEM_MINIPORT
  FxBool
    res,
    foundMSR = FXFALSE,
    rVal = FXFALSE;

  FxU32
    nret,
    physBase, physMask;

  MSRInfo
    inS, outS;

  FXUNUSED(nret);               /* avoid lame-assed watcom warning  */

  /* Get PhysBase */
  res = genPhysBase(physBaseAddress, type, &physBase);

  if (res == FXFALSE)
    return rVal;

  /* Get PhysMask */
  res = genPhysMask(physSize, &physMask);

  if (res == FXFALSE)
    return rVal;
  
  inS.msrNum = MTRR_BASE_FIRST;

  do {
    DOGETMSR(inS,outS);

    if (outS.msrLo == physBase) {
      inS.msrNum++;

      DOGETMSR(inS, outS);

      if (outS.msrLo == physMask) {
        *mtrrNum = MSR2MTRR(inS.msrNum);
        foundMSR = FXTRUE;
      }
    }
    inS.msrNum++;
  } while (!foundMSR && (inS.msrNum <= MTRR_BASE_LAST));

  if (foundMSR)
    rVal = FXTRUE;

  return rVal;  
#else
  return FXFALSE;               /* Doesn't matter on NT yet */
#endif
} /* pciFindMTRRMatch */

/*
**  pciFindFreeMTRR - find an MTRR that matches this one.
**
**  We return the MTRR number which is a special-case MSR calculated
**  in the following manner:
**
**  (freeBaseMSR - baseMTRR0) >> 2
**
**  NOTE:  This routine will return the lowest-numbered free MTRR
*/
FX_EXPORT FxBool FX_CSTYLE
pciFindFreeMTRR(FxU32 *mtrrNum)
{
#ifndef MAPPL_MAPMEM_MINIPORT
  FxBool
    foundFree = FXFALSE;

  FxU32
    nret;

  MSRInfo
    inS, outS;

  FXUNUSED(nret);               /* avoid lame-assed watcom warning */

  for (
       inS.msrNum = MTRR_BASE_FIRST + 1;
       (inS.msrNum < MTRR_BASE_LAST + 1) && !foundFree;
       inS.msrNum += 2
       ) {

    DOGETMSR(inS, outS);

    if ((outS.msrLo & 0x800) == 0) {
      foundFree = FXTRUE;
      *mtrrNum = MSR2MTRR(inS.msrNum);
    }
  }

  return foundFree;
#else
  return FXFALSE;               /* Doesn't matter on NT yet */
#endif
} /* pciFindFreeMTRR */

/*
**  pciSetMTRR - set up a specified MTRR based on physical address, physical
**               size, and type.
**
**  NOTE:  A zero for the physical size results in the MTRR being
**  cleared. 
**
*/
FX_EXPORT FxBool FX_CSTYLE
pciSetMTRR(FxU32 mtrrNum, FxU32 physBaseAddr, FxU32 physSize,
           PciMemType type)
{
#ifndef MAPPL_MAPMEM_MINIPORT
  FxBool
    res, 
    rVal = FXFALSE;

  FxU32
    nret,
    physBase, physMask;

  MSRInfo
    inS, outS;

  FXUNUSED(nret);               /* avoid lame-assed watcom warning */

  if (mtrrNum > MTRR_LAST)
    return rVal;


  inS.msrNum = MTRRPHYSBASE2MSR(mtrrNum);

  if (physSize == 0) {
    /* Clear the MTRR */
    
    inS.msrLo = 0x0;
    inS.msrHi = 0x0;


    DOSETMSR(inS, outS);

    inS.msrNum++;

    DOSETMSR(inS, outS);
    
  } else {
    /* Generate masks and set the MTRR */

    res = genPhysBase(physBaseAddr, type, &physBase);
    if (res == FXFALSE)
      return rVal;

    res = genPhysMask(physSize, &physMask);
    
    inS.msrLo = physBase;
    inS.msrHi = 0x0;
    
    DOSETMSR(inS, outS);

    inS.msrNum++;
    inS.msrLo = physMask;
    inS.msrHi = 0xf;
    
    DOSETMSR(inS, outS);
  }

  rVal = FXTRUE;

  return rVal;
#else
  return FXFALSE;
#endif
} /* pciSetMTRR */  
