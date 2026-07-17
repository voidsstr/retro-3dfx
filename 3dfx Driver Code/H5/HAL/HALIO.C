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
** $Revision: 4$
** $Date: 10/11/00 8:31:09 PM$
*/

#include <assert.h>
#include <stdlib.h>

#include <h3.h>
#include "../csim/h3sim.h"
#include "init.h"

#ifdef HAL_HSIM
#include "tstbench.h"
#endif

//----------------------------------------------------------------------
//
// pre- and post-Swizzle address and data
//
// csim always operates on unswizzled data
// hsim always operates on swizzled data
//
// a,d -------------------->[csim]------------------------->
//       |                                          ^
//       |                                          |
//       |                                          |
//        --->[pre-swiz]--->[hsim]--->[post-swiz]---
//
//----------------------------------------------------------------------

void halStoreSwizzle( FxU32 addr, FxU32 base, FxU32 offset, FxU32 data, int nbytes ) 
{
  FxU32 a = offset;
  FxU32 d = data;
  SstRegs *sst = SST_FAKE_ADDRESS_GET_CSIM(addr);
  FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET(addr);
  CsimPrivate *cpriv = CSIM_PRIVATE(sst);
  SstIORegs *sstio = &cpriv->io;
  int bs=0, ws=0;

  if ( SST_IS_2D_ADDR(iaddr) ) {
    bs = sstio->miscInit0&SST_REGISTER_BYTE_SWIZZLE_EN;
    ws = sstio->miscInit0&SST_REGISTER_WORD_SWIZZLE_EN;
    a |= BIT(19);
  } else if ( SST_IS_3D_ADDR(iaddr) ) {
    bs = sstio->miscInit0&SST_REGISTER_BYTE_SWIZZLE_EN;
    ws = sstio->miscInit0&SST_REGISTER_WORD_SWIZZLE_EN;
    a |= BIT(20);
  } else if ( SST_IS_RAW_LFB_ADDR(iaddr) ) {
    bs = sstio->miscInit0&SST_RAWLFB_BYTE_SWIZZLE_EN;
    ws = sstio->miscInit0&SST_RAWLFB_WORD_SWIZZLE_EN;
  } 

  a += base;

  switch ( nbytes ) {
  case 1:
    if ( bs ) a ^= 0x3;
    if ( ws ) a ^= 0x2;
    if ( bs || ws )
      GDBG_INFO(121,"   W PCI8(0x%x,%11d(0x%08x)) %s %s\n",
                a,d,d,bs?"bs":"",ws?"ws":"");
#ifdef HAL_HSIM
    if ( halInfo.hsim )
      PCI_MEMW8(a,d);
#endif
#ifdef HAL_HW
    if ( halInfo.hw )
      *((FxU8 *)a) = (FxU8) d;
#endif
    break;
  case 2:
    if ( bs ) d = (data<<8) | (data>>8);
    if ( ws ) a ^= 0x2;
    if ( bs || ws )
      GDBG_INFO(121,"   W PCI16(0x%x,%11d(0x%08x)) %s %s\n",
                a,d,d,bs?"bs":"",ws?"ws":"");
#ifdef HAL_HSIM
    if ( halInfo.hsim )
      PCI_MEMW16(a,d);
#endif
#ifdef HAL_HW
    if ( halInfo.hw )
      *((FxU16 *)a) = (FxU16)d;
#endif
    break;
  case 4:
    if ( bs ) d = (d>>24) | ((d>>8)&0xFF00) | ((d<<8)&0xFF0000) | (d<<24);
    if ( ws ) d = (d>>16) | (d<<16);
    if ( bs || ws )
      GDBG_INFO(121,"     W PCI(0x%x,%11d(0x%08x)) %s %s\n",
                a,d,d,bs?"bs":"",ws?"ws":"");
#ifdef HAL_HSIM
    if ( halInfo.hsim )
      PCI_MEM_WR(a,d);
#endif
#ifdef HAL_HW
    if ( halInfo.hw )
      *((FxU32 *)a) = (FxU32)d;
#endif
    break;
  }

}

FxU32 halLoadSwizzle ( FxU32 addr, FxU32 base, FxU32 offset, int nbytes ) 
{
  FxU32 d;
  FxU32 a = offset;
  SstRegs *sst = SST_FAKE_ADDRESS_GET_CSIM(addr);
  FxU32 iaddr = SST_FAKE_ADDRESS_GET_OFFSET(addr);
  CsimPrivate *cpriv = CSIM_PRIVATE(sst);
  SstIORegs *sstio = &cpriv->io;
  int bs=0, ws=0;

  if ( SST_IS_2D_ADDR(iaddr) ) {
    bs = sstio->miscInit0&SST_REGISTER_BYTE_SWIZZLE_EN;
    ws = sstio->miscInit0&SST_REGISTER_WORD_SWIZZLE_EN;
    a |= BIT(19);
  } else if ( SST_IS_3D_ADDR(iaddr) ) {
    bs = sstio->miscInit0&SST_REGISTER_BYTE_SWIZZLE_EN;
    ws = sstio->miscInit0&SST_REGISTER_WORD_SWIZZLE_EN;
    a |= BIT(20);
  } else if ( SST_IS_RAW_LFB_ADDR(iaddr) ) {
    bs = sstio->miscInit0&SST_RAWLFB_BYTE_SWIZZLE_EN;
    ws = sstio->miscInit0&SST_RAWLFB_WORD_SWIZZLE_EN;
  } 
  
  a += base;

  switch ( nbytes ) {
  case 1:
    if ( bs ) a ^= 0x3;
    if ( ws ) a ^= 0x2;
#ifdef HAL_HSIM
    if ( halInfo.hsim )
      d = PCI_MEMR8(a);
#endif
#ifdef HAL_HW
    if ( halInfo.hw )
      d = *((FxU8 *)a);
#endif
    if ( bs || ws )
      GDBG_INFO(121,"     R PCI8(0x%x,%11d(0x%08x)) %s %s\n",
                a,d,d,bs?"bs":"",ws?"ws":"");
    break;
  case 2:
    if ( ws ) a ^= 0x2;
#ifdef HAL_HSIM
    if ( halInfo.hsim )
      d = PCI_MEMR16(a);
#endif
#ifdef HAL_HW
    if ( halInfo.hw )
      d = *((FxU16 *)a);
#endif
    if ( bs || ws )
      GDBG_INFO(121,"     R PCI16(0x%x,%11d(0x%08x)) %s %s\n",
                a,d,d,bs?"bs":"",ws?"ws":"");
    if ( bs ) d = ((d<<8)&0xFF00) | ((d>>8)&0xFF);
    break;
  case 4:
#ifdef HAL_HSIM
    if ( halInfo.hsim )
      d = PCI_MEM_RD(a);
#endif
#ifdef HAL_HW
    if ( halInfo.hw )
      d = *((FxU32 *)a);
#endif
    if ( bs || ws )
      GDBG_INFO(121,"     R PCI(0x%x,%11d(0x%08x)) %s %s\n",
                a,d,d,bs?"bs":"",ws?"ws":"");
    if ( bs ) d = (d>>24) | ((d>>8)&0xFF00) | ((d<<8)&0xFF0000) | (d<<24);
    if ( ws ) d = (d>>16) | (d<<16);
    break;
  }

  return d;
}

//----------------------------------------------------------------------
// The routines halStore* and halLoad* are the funnels through which all
// hardware accesses go.  These procs route the stores and loads to the
// appropriate simulators and hardware.
//
// NOTE: we make the simplifying assumption that on H3 the second memory
// segment (32 Mbyte of non-modal LFB space) that is physically mapped using
// MemoryBase1, is virtually mapped right after the the 1st 16 Mbyte region
// Since the entire HAL/csim works off of a phony virtual address, this is
// not a problem
//----------------------------------------------------------------------

// GMT: another hack
#ifdef KERNEL
static int memEnabled = 1;  // memory access really enabled -KMW
#else
static int memEnabled = 1;  // memory access enabled
#endif /* KERNEL */

static int ioEnabled;   // i/o port access enabled

//----------------------------------------------------------------------
// execute a 8 bit store to SST, only LFB access is allowed
//----------------------------------------------------------------------
void FX_EXPORT FX_CSTYLE
halStore8( volatile void *addr, FxU8 data )
{
    FxU32 bn, physBase, offset, iaddr;

    // do some sanity checks
    if (SST_BAD_ADDRESS(addr)) {
        GDBG_ERROR("SET8","bad address=0x%x  data=%d(0x%08x)\n",
                        addr,data,data);
        return;
    }

    // decode address
    bn = SST_FAKE_ADDRESS_GET_BOARD(addr);
    offset = SST_FAKE_ADDRESS_GET_BASE_OFFSET(addr);
    physBase = halInfo.boardInfo[bn].physAddr[SST_FAKE_ADDRESS_GET_BASE(addr)];
    iaddr = SST_FAKE_ADDRESS_GET_OFFSET(addr);

    if (halInfo.hw) {
        GDBG_INFO(121,"   W PCI8(0x%x,%11d(0x%08x))\n", physBase + offset,data,data);
#ifdef HAL_EXTREMELY_ANAL_BRINGUP
        /* Make sure there's free space in the PCI FIFO */
        while (!(*((FxU32 *) halInfo.boardInfo[bn].physAddr[0]) & 0x3f));
#endif
        if (halInfo.hw & HSIM_SWIZZLE) 
          halStoreSwizzle((FxU32)addr,physBase,offset, data, 1);
        else
          ((FxU8 *)physBase)[offset] = data;
    }

#ifdef HAL_HSIM
    {
      SstRegs *sst;
      CsimPrivate *cp;
      FxI32 chipIndex;
      
      if(halInfo.hsim) 
        {                 // if hardware simulating
          // if in texture territory AND using backdoor
          if ( SST_IS_TEX0_ADDR(iaddr) && (halInfo.hsim & HSIM_TREX_BACKDOOR_TEXWRITES)) 
            {
              for(chipIndex=0; chipIndex<halInfo.boardsFound; chipIndex++)
                {
                  sst = halInfo.boardInfo[chipIndex].sstCSIM;
                  cp = CSIM_PRIVATE(sst);
                  
                  sstTrexBackdoor(cp->trex + 0, offset-SST_TEX0_OFFSET, data, 1, tmu0TexturePort);
                }
            }     
          else if( SST_IS_TEX1_ADDR(iaddr) && (halInfo.hsim & HSIM_TREX_BACKDOOR_TEXWRITES)) 
            {
              for(chipIndex=0; chipIndex<halInfo.boardsFound; chipIndex++)
                {
                  sst = halInfo.boardInfo[chipIndex].sstCSIM;
                  cp = CSIM_PRIVATE(sst);

                  sstTrexBackdoor(cp->trex + 1, offset-SST_TEX1_OFFSET, data, 1, tmu1TexturePort);
                }
            }     
          else if( SST_IS_TEX2_ADDR(iaddr) && (halInfo.hsim & HSIM_TREX_BACKDOOR_TEXWRITES)) 
            {
              for(chipIndex=0; chipIndex<halInfo.boardsFound; chipIndex++)
                {
                  sst = halInfo.boardInfo[chipIndex].sstCSIM;
                  cp = CSIM_PRIVATE(sst);

                  sstTrexBackdoor(cp->trex + 0, offset-SST_TEX2_OFFSET, data, 1, largeTexturePort);
                }
            }     
          else 
            {         
              trexFifoCheckAllHsim();
              GDBG_INFO(121,"   W PCI8(0x%x,%11d(0x%08x))\n",physBase+offset,data,data);
              if (halInfo.hsim & HSIM_SWIZZLE) 
                halStoreSwizzle((FxU32)addr,physBase,offset, data, 1);
              else
                PCI_MEMW8(physBase + offset,data);
            }
        }
    }
#endif

    if (halInfo.csim && halInfo.csimio) {
        if (!memEnabled)
            GDBG_ERROR("SET8","memory access is not enabled\n");
        csimStore8( SST_FAKE_ADDRESS_GET_CSIM(addr), (FxU32)addr, data );
    }
}

//----------------------------------------------------------------------
// execute a 16 bit store to SST, only LFB access is allowed
//----------------------------------------------------------------------
void FX_EXPORT FX_CSTYLE
halStore16( volatile void *addr, FxU16 data )
{
    FxU32 bn, physBase, offset, iaddr;

    // do some sanity checks
    if (SST_BAD_ADDRESS(addr)) {
        GDBG_ERROR("SET16","bad address=0x%x  data=%d(0x%08x)\n",
                        addr,data,data);
        return;
    }
    if ((!((FxU32)addr & SST_RAW_LFB_OFFSET)) && (1 & (FxU32)addr)) {
        GDBG_ERROR("SET16","unaligned address=0x%x  data=%d(0x%08x)\n",
                        addr,data,data);
        return;
    }

    // decode address
    bn = SST_FAKE_ADDRESS_GET_BOARD(addr);
    offset = SST_FAKE_ADDRESS_GET_BASE_OFFSET(addr);
    physBase = halInfo.boardInfo[bn].physAddr[SST_FAKE_ADDRESS_GET_BASE(addr)];
    iaddr = SST_FAKE_ADDRESS_GET_OFFSET(addr);

    if (halInfo.hw) {
        GDBG_INFO(121,"   W PCI16(0x%x,%11d(0x%08x))\n",physBase+offset,data,data);
#ifdef HAL_EXTREMELY_ANAL_BRINGUP
        /* Make sure there's free space in the PCI FIFO */
        while (!(*((FxU32 *) halInfo.boardInfo[bn].physAddr[0]) & 0x3f));
#endif
        if (halInfo.hw & HSIM_SWIZZLE) 
          halStoreSwizzle((FxU32)addr,physBase,offset, data, 2);
        else
          ((FxU16 *)physBase)[offset>>1] = data;
    }

#ifdef HAL_HSIM
    {
      SstRegs *sst;
      CsimPrivate *cp;
      FxI32 chipIndex;
            
      if(halInfo.hsim) 
        {
          // if in texture territory AND using backdoor
          if ( SST_IS_TEX0_ADDR(iaddr) && (halInfo.hsim & HSIM_TREX_BACKDOOR_TEXWRITES)) 
            {
              for(chipIndex=0; chipIndex<halInfo.boardsFound; chipIndex++)
                {
                  sst = halInfo.boardInfo[chipIndex].sstCSIM;
                  cp = CSIM_PRIVATE(sst);
                  
                  sstTrexBackdoor(cp->trex + 0, offset-SST_TEX0_OFFSET, data, 2, tmu0TexturePort);
                }
            }     
          else if( SST_IS_TEX1_ADDR(iaddr) && (halInfo.hsim & HSIM_TREX_BACKDOOR_TEXWRITES)) 
            {
              for(chipIndex=0; chipIndex<halInfo.boardsFound; chipIndex++)
                {
                  sst = halInfo.boardInfo[chipIndex].sstCSIM;
                  cp = CSIM_PRIVATE(sst);
                  
                  sstTrexBackdoor(cp->trex + 1, offset-SST_TEX1_OFFSET, data, 2, tmu1TexturePort);
                }
            }     
          else if( SST_IS_TEX2_ADDR(iaddr) && (halInfo.hsim & HSIM_TREX_BACKDOOR_TEXWRITES)) 
            {
              for(chipIndex=0; chipIndex<halInfo.boardsFound; chipIndex++)
                {
                  sst = halInfo.boardInfo[chipIndex].sstCSIM;
                  cp = CSIM_PRIVATE(sst);

                  sstTrexBackdoor(cp->trex + 0, offset-SST_TEX2_OFFSET, data, 2, largeTexturePort);
                }
            }           
          else 
            {
              trexFifoCheckAllHsim();
              GDBG_INFO(121,"   W PCI16(0x%x,%11d(0x%08x))\n",physBase+offset,data,data);
              if (halInfo.hsim & HSIM_SWIZZLE) 
                halStoreSwizzle((FxU32)addr,physBase,offset, data, 2);
              else
                PCI_MEMW16(physBase+offset, (FxU32)data);
            }
        }
    }
#endif

    if (halInfo.csim && halInfo.csimio) {
        if (!memEnabled)
            GDBG_ERROR("SET16","memory access is not enabled\n");
        csimStore16( SST_FAKE_ADDRESS_GET_CSIM(addr), (FxU32)addr, data );
    }
}

//----------------------------------------------------------------------
// execute a 32 bit store, if HAL_CSIM is not defined this never gets called
//----------------------------------------------------------------------
void FX_EXPORT FX_CSTYLE
halStore32( volatile void *addr, FxU32 data )
{
    FxU32 bn, physBase, offset, iaddr;

    // do some sanity checks
    if (SST_BAD_ADDRESS(addr)) {
        GDBG_ERROR("SET","bad address=0x%x  data=%d(0x%08x)\n",
                        addr,data,data);
        return;
    }
    if ((!((FxU32)addr & SST_RAW_LFB_OFFSET)) && (3 & (FxU32)addr)) {
        GDBG_ERROR("SET","unaligned address=0x%x  data=%d(0x%08x)\n",
                        addr,data,data);
        return;
    }

    // decode address
    bn = SST_FAKE_ADDRESS_GET_BOARD(addr);
    offset = SST_FAKE_ADDRESS_GET_BASE_OFFSET(addr);
    physBase = halInfo.boardInfo[bn].physAddr[SST_FAKE_ADDRESS_GET_BASE(addr)];
    iaddr = SST_FAKE_ADDRESS_GET_OFFSET(addr);

    if (halInfo.hw) {
        GDBG_INFO(121,"     W PCI(0x%x,%11d(0x%08x))\n",physBase+offset,data,data);
#ifdef HAL_EXTREMELY_ANAL_BRINGUP
        /* Make sure there's free space in the PCI FIFO */
        while (!(*((FxU32 *) halInfo.boardInfo[bn].physAddr[0]) & 0x3f));
#endif
        if (halInfo.hw & HSIM_SWIZZLE) 
          halStoreSwizzle((FxU32)addr,physBase,offset, data, 4);
        else
          ((FxU32 *)physBase)[offset>>2] = data;
    }

#ifdef HAL_HSIM
    {
      SstRegs *sst;
      CsimPrivate *cp;
      FxI32 chipIndex;
      
      if(halInfo.hsim) 
        {                 // if hardware simulating
          // if in texture territory AND using backdoor
          if ( SST_IS_TEX0_ADDR(iaddr) && (halInfo.hsim & HSIM_TREX_BACKDOOR_TEXWRITES)) 
            {
              for(chipIndex=0; chipIndex<halInfo.boardsFound; chipIndex++)
                {
                  sst = halInfo.boardInfo[chipIndex].sstCSIM;
                  cp = CSIM_PRIVATE(sst);

                  sstTrexBackdoor(cp->trex + 0, offset-SST_TEX0_OFFSET, data, 4, tmu0TexturePort);
                }
            }     
          else if( SST_IS_TEX1_ADDR(iaddr) && (halInfo.hsim & HSIM_TREX_BACKDOOR_TEXWRITES)) 
            {
              for(chipIndex=0; chipIndex<halInfo.boardsFound; chipIndex++)
                {
                  sst = halInfo.boardInfo[chipIndex].sstCSIM;
                  cp = CSIM_PRIVATE(sst);

                  sstTrexBackdoor(cp->trex + 1, offset-SST_TEX1_OFFSET, data, 4, tmu1TexturePort);
                }
            }     
          else if( SST_IS_TEX2_ADDR(iaddr) && (halInfo.hsim & HSIM_TREX_BACKDOOR_TEXWRITES)) 
            {
              for(chipIndex=0; chipIndex<halInfo.boardsFound; chipIndex++)
                {
                  sst = halInfo.boardInfo[chipIndex].sstCSIM;
                  cp = CSIM_PRIVATE(sst);
                  
                  sstTrexBackdoor(cp->trex + 0, offset-SST_TEX2_OFFSET, data, 4, largeTexturePort);
                }
            }     
          else 
            {
              trexFifoCheckAllHsim();
              GDBG_INFO(121,"     W PCI(0x%x,%11d(0x%08x))\n",physBase+offset,data,data);
              if (halInfo.hsim & HSIM_SWIZZLE) 
                halStoreSwizzle((FxU32)addr,physBase,offset, data, 4);
              else
                PCI_MEM_WR(physBase + offset,data);
            }
        }
    }
#endif

    if (halInfo.csim && halInfo.csimio) {
        if (!memEnabled)
            GDBG_ERROR("SET","memory access is not enabled\n");
        csimStore32( SST_FAKE_ADDRESS_GET_CSIM(addr), (FxU32)addr, data );
    }
}

//----------------------------------------------------------------------
// same as halStore32 except data is a float to be shoved right into an
//      integer register.  NOTE: this code may be machine dependent
//----------------------------------------------------------------------
void FX_EXPORT FX_CSTYLE
halStore32f( volatile void *addr, FxFloat data )
{
    halStore32(addr,*(FxU32 *)&data);
}

//----------------------------------------------------------------------
// execute a 8 bit read
//----------------------------------------------------------------------
FxU8 FX_EXPORT FX_CSTYLE
halLoad8( volatile void *addr )
{
    FxU8 data = 0xbd;
    FxU32 bn, physBase, offset;

    // do some sanity checks
    if (SST_BAD_ADDRESS(addr)) {
        GDBG_ERROR("GET8","bad address=0x%x  data=%d(0x%08x)\n",
                        addr,data,data);
        return data;
    }

    // decode address
    bn = SST_FAKE_ADDRESS_GET_BOARD(addr);
    offset = SST_FAKE_ADDRESS_GET_BASE_OFFSET(addr);
    physBase = halInfo.boardInfo[bn].physAddr[SST_FAKE_ADDRESS_GET_BASE(addr)];

    // read CSIM first and save it
    if (halInfo.csim && halInfo.csimio) { 
        if (!memEnabled)
            GDBG_ERROR("GET8","memory access is not enabled\n");
        data = (FxU8)csimLoad8( SST_FAKE_ADDRESS_GET_CSIM(addr), (FxU32)addr );
        halInfo.csimLastRead = data;
    }

#ifdef HAL_HSIM
    if (halInfo.hsim) {
        trexFifoCheckAllHsim();
        if (halInfo.hsim & HSIM_SWIZZLE) 
          data = halLoadSwizzle((FxU32)addr,physBase,offset, 1);
        else
          data = PCI_MEMR8(physBase + offset);
        GDBG_INFO(121,"     R PCI8(0x%x,%11d(0x%08x))\n",
                       physBase+offset,data,data);
    }
#endif

    // lastly, read HW (if enabled) and return it
    if (halInfo.hw) {
        if (halInfo.hw & HSIM_SWIZZLE) 
          data = (FxU8) halLoadSwizzle((FxU32)addr,physBase,offset,1);
        else
          data = ((FxU8 *)physBase)[offset];
        GDBG_INFO(121,"     R PCI8(0x%x,%11d(0x%08x))\n",
                       physBase+offset,data,data);
    }
    return data;
}

//----------------------------------------------------------------------
// execute a 16 bit read
//----------------------------------------------------------------------
FxU16 FX_EXPORT FX_CSTYLE
halLoad16( volatile void *addr )
{
    FxU16 data = 0xbad0;
    FxU32 bn, physBase, offset;

    // do some sanity checks
    if (SST_BAD_ADDRESS(addr)) {
        GDBG_ERROR("GET16","bad address=0x%x  data=%d(0x%08x)\n",
                        addr,data,data);
        return data;
    }
    if ((!((FxU32)addr & SST_RAW_LFB_OFFSET)) && (1 & (FxU32)addr)) {
        GDBG_ERROR("GET16","unaligned address=0x%x  data=%d(0x%08x)\n",
                        addr,data,data);
        return data;
    }

    // decode address
    bn = SST_FAKE_ADDRESS_GET_BOARD(addr);
    offset = SST_FAKE_ADDRESS_GET_BASE_OFFSET(addr);
    physBase = halInfo.boardInfo[bn].physAddr[SST_FAKE_ADDRESS_GET_BASE(addr)];

    if (halInfo.csim && halInfo.csimio) {         // NOTE: we read CSIM first and save it
        if (!memEnabled)
            GDBG_ERROR("GET16","memory access is not enabled\n");
        data = (FxU16)csimLoad16( SST_FAKE_ADDRESS_GET_CSIM(addr), (FxU32)addr );
        halInfo.csimLastRead = data;
    }

#ifdef HAL_HSIM
    if (halInfo.hsim) {
        trexFifoCheckAllHsim();
        if (halInfo.hsim & HSIM_SWIZZLE) 
          data = halLoadSwizzle((FxU32)addr,physBase,offset, 2);
        else
          data = PCI_MEMR16(physBase + offset);
        GDBG_INFO(121,"     R PCI16(0x%x,%11d(0x%08x))\n",
                       physBase+offset,data,data);
    }
#endif

    // lastly, read HW (if enabled) and return it
    if (halInfo.hw) {
        if (halInfo.hw & HSIM_SWIZZLE) 
          data = (FxU16) halLoadSwizzle((FxU32)addr,physBase,offset, 2);
        else
          data = ((FxU16 *)physBase)[offset>>1];
        GDBG_INFO(121,"     R PCI16(0x%x,%11d(0x%08x))\n",
                       physBase+offset,data,data);
    }
    return data;
}

//----------------------------------------------------------------------
// execute a 32 bit read from SST
//----------------------------------------------------------------------
FxU32 FX_EXPORT FX_CSTYLE
halLoad32( volatile void *addr )
{
    FxU32 data = 0xdeadbeef;
    FxU32 bn, physBase, offset;

    // do some sanity checks
    if (SST_BAD_ADDRESS(addr)) {
        GDBG_ERROR("GET","bad address=0x%x  data=%d(0x%08x)\n",
                        addr,data,data);
        return data;
    }
    if ((!((FxU32)addr & SST_RAW_LFB_OFFSET)) && (3 & (FxU32)addr)) {
        GDBG_ERROR("GET","unaligned address=0x%x  data=%d(0x%08x)\n",
                        addr,data,data);
        return data;
    }

    // decode address
    bn = SST_FAKE_ADDRESS_GET_BOARD(addr);
    offset = SST_FAKE_ADDRESS_GET_BASE_OFFSET(addr);
    physBase = halInfo.boardInfo[bn].physAddr[SST_FAKE_ADDRESS_GET_BASE(addr)];

    if (halInfo.csim && halInfo.csimio) {         // NOTE: we read CSIM first and save it
        if (!memEnabled)
            GDBG_ERROR("GET","memory access is not enabled\n");
        data = csimLoad32( SST_FAKE_ADDRESS_GET_CSIM(addr), (FxU32)addr );
        halInfo.csimLastRead = data;
    }

#ifdef HAL_HSIM
    if (halInfo.hsim) {
        trexFifoCheckAllHsim();
        if (halInfo.hsim & HSIM_SWIZZLE) 
          data = halLoadSwizzle((FxU32)addr,physBase,offset, 4 );
        else
          data = PCI_MEM_RD(physBase + offset);
        GDBG_INFO(121,"     R PCI(0x%x,%11d(0x%08x))\n",
                       physBase+offset,data,data);
    }
#endif

    // lastly, read HW (if enabled) and return it
    if (halInfo.hw) {
        if (halInfo.hw & HSIM_SWIZZLE) 
          data = halLoadSwizzle((FxU32)addr,physBase,offset, 4 );
        else
          data = ((FxU32 *)physBase)[offset>>2];
        GDBG_INFO(121,"     R PCI(0x%x,%11d(0x%08x))\n",
                       physBase+offset,data,data);
    }
    return data;
}


//----------------------------------------------------------------------
// The halInPort* and halOutPort* routines handle all port i/o operations.
// These routines perform the i/o operations on all the active simulators
// and/or hardware.
//

#include <fxpci.h>
#ifdef WINSIM
#include <pcilib.h>
#else
#include "../../swlibs/newpci/pcilib/pcilib.h"
#endif

static FxU32 lastIO;

//----------------------------------------------------------------------
// execute an 32 bit config write operation
//----------------------------------------------------------------------
void FX_EXPORT FX_CSTYLE
halCfgStore32( FxU16 port, int mechanism,
               FxU32 bus_number, FxU32 device_number, 
               FxU32 function_number, FxU32 register_offset, 
               FxU32 data ) 
{
  FxU32 counter;
  
  GDBG_INFO(122,"     halCfgStore32 port=%d mechanism=%d bus_number=%d device_number=%d\n",
            port, mechanism, bus_number, device_number);
  GDBG_INFO(122,"     function_number=%d register_offset=%d\n",
            function_number, register_offset);

  assert(function_number < 8);
  assert(register_offset < 256);


#ifndef KERNEL
  if (halInfo.hw) {
    GDBG_INFO(121,"     W PCI CFG(0x%x,%11d(0x%08x))\n",port,data,data);
  }
#endif

#ifdef HAL_HSIM
  if (halInfo.hsim) {
    //The HSIM doesn't really have multiple PCI buses. As such, only
    //let accesses to bus 1 go through the HSIM
    if(bus_number == 1)
      {
	GDBG_INFO(121,"     W PCI CFG(0x%x,%11d(0x%08x))\n",port,data,data);
	if ( mechanism == 1 )
	  PCI_CFG_WR(register_offset,data,device_number);
	else
	  PCI_IOW32(port,data);
      }
  }
#endif

  if (halInfo.csim /* && halInfo.csimio */ ) 
    {
      if(mechanism == 1)
	{	  
	  for(counter=0; counter<HAL_MAX_BOARDS; counter++)
	    {	  
	      if(halInfo.boardInfo[counter].pciDeviceNumber == device_number &&
		 halInfo.boardInfo[counter].pciBusNumber    == bus_number &&
		 halInfo.boardInfo[counter].pciFunctionNumber == function_number)
		csimCfgStore32(halInfo.boardInfo[counter].sstCSIM, register_offset, data);
	    }	  
	}
      else
	{
	  GDBG_ERROR("halCfgStore32", "Only support mechanism 1 config writes\n");
	}
      
      GDBG_INFO(120,"       SET_IO(0x%x,%11d(0x%08x))\n",port,data,data);      
    }
  lastIO = 0;

  return;
}

//----------------------------------------------------------------------
// execute an 32 bit config read operation
//----------------------------------------------------------------------
FxU32 FX_EXPORT FX_CSTYLE
halCfgLoad32( FxU16 port, int mechanism,
              FxU32 bus_number, FxU32 device_number, 
              FxU32 function_number, FxU32 register_offset ) 
{
  FxU32 data = 0xFFFFFFFF;
  FxI32 counter;
  
  GDBG_INFO(122,"     halCfgLoad32 port=%d mechanism=%d bus_number=%d device_number=%d\n",
            port, mechanism, bus_number, device_number);
  GDBG_INFO(122,"     function_number=%d register_offset=%d\n",
            function_number, register_offset);
  
  assert(function_number < 8);
  assert(register_offset < 256);

  //In order to avoid the annoyance of scanning so many devices/functions,
  //is the hsim is active, we won't actually do the reads. This is also necessary
  //because the verilog napalm board only looks at 4 of the 5 bits of device number;
#ifdef HAL_HSIM
  if (halInfo.hsim) 
    {
      if(device_number > 4 || bus_number > 3)
        {
          data=0xFFFFFFFF;
          GDBG_INFO(121,"     R PCI IO(0x%x,%11d(0x%08x)) short circuited\n",port,data,data);
          
          return(data);
        }
  }
#endif


  if (halInfo.csim /* && halInfo.csimio */ ) {
    if ( mechanism == 1 ) {
      data = 0xFFFFFFFF;

      for(counter=0; counter<HAL_MAX_BOARDS; counter++)
	{	  
	  if(halInfo.boardInfo[counter].pciDeviceNumber == device_number &&
	     halInfo.boardInfo[counter].pciBusNumber    == bus_number &&
	     halInfo.boardInfo[counter].pciFunctionNumber == function_number)
	    {
	      /*GDBG_INFO(0, "Found Napalm: busNumber=%d deviceNumber=%d, functionNumber=%d\n",
		bus_number, device_number, function_number);*/
	      data = csimCfgLoad32(halInfo.boardInfo[counter].sstCSIM, register_offset);
	    }      
	}
    } else {                                          // config mechanism 2
      // just return FFFFFFFF (don't support this mode)
      // GMT: if real HW uses this mode, then value returned will be from
      // the real HW and not CSIM anyway
    }
    GDBG_INFO(120,"       GET_IO(0x%x,%11d(0x%08x))\n",port,data,data);
  }
  
#ifdef HAL_HSIM
  if (halInfo.hsim) {
    //The HSIM doesn't really have multiple PCI buses. As such, only
    //let accesses to bus 1 go through the HSIM
    if(bus_number == 1)
      {
	PCI_CFG_SET_FNCTN(function_number);
	if (mechanism == 1)                                 // mechanism 1
	  data = PCI_CFG_RD(register_offset,device_number);
	else 
	  data = (FxU32) PCI_IOR32(port);

	GDBG_INFO(121,"     R PCI IO(0x%x,%11d(0x%08x))\n",port,data,data);
      }
    else
      {
	data = 0xFFFFFFFF;
	GDBG_INFO(121,"     R PCI IO(0x%x) forced to 0x%x on bus %d\n",port,data, bus_number);
      }
  }
#endif

#ifndef KERNEL
  if (halInfo.hw) {
    GDBG_INFO(121,"     R PCI IO(0x%x,%11d(0x%08x))\n",port,data,data);
  }
#endif

  lastIO = 0;

  return data;
}

//----------------------------------------------------------------------
// execute an 8 bit port output operation
//----------------------------------------------------------------------
void 
halOutPort8( FxU16 port, FxU8 data ) 
{
  lastIO = 0;
#ifndef KERNEL
  if (halInfo.hw) {
    GDBG_INFO(121,"     W PCI IO8(0x%x,%11d(0x%08x))\n",port,data,data);
  }
#endif

  if (port == CONFIG_ADDRESS_PORT)
    lastIO = data;

#ifdef HAL_HSIM
  if (halInfo.hsim) {
    GDBG_INFO(121,"     W PCI IO8(0x%x,%11d(0x%08x))\n",port,data,data);
    if (port != CONFIG_ADDRESS_PORT)    // the hardware doesn't really see
        PCI_IOW8(port,data);            // this on the PCI bus
  }
#endif

  if (halInfo.csim && halInfo.csimio) {
    // csim just ignores port i/o operations
    GDBG_INFO(120,"       SET_IO8(0x%x,%11d(0x%08x))\n",port,data,data);
  }
}

//----------------------------------------------------------------------
// execute an 16 bit port output operation
//----------------------------------------------------------------------
void 
halOutPort16( FxU16 port, FxU16 data ) 
{
  lastIO = 0;
#ifndef KERNEL
  if (halInfo.hw) {
    GDBG_INFO(121,"     W PCI IO16(0x%x,%11d(0x%08x))\n",port,data,data);
  }
#endif

#ifdef HAL_HSIM
  if (halInfo.hsim) {
    GDBG_INFO(121,"     W PCI IO16(0x%x,%11d(0x%08x))\n",port,data,data);
    PCI_IOW16(port,data);
  }
#endif

  if (halInfo.csim && halInfo.csimio) {
    // csim just ignores port i/o operations
    GDBG_INFO(120,"       SET_IO16(0x%x,%11d(0x%08x))\n",port,data,data);
    if (port == CONFIG_ADDRESS_PORT) {
        GDBG_ERROR("halOutPort16","config mechanism 2 not supported\n");
    }
  }
}

//----------------------------------------------------------------------
// execute an 32 bit port output operation
//----------------------------------------------------------------------
void 
halOutPort32( FxU16 port, FxU32 data ) 
{
  FxI16 portOffset = SST_FAKE_PORT_GET_OFFSET(port);
  FxU16 bn, physBase;
  FxU32 bus_number, device_number, function_number, register_offset;

  // check for config cycles

  if (port == CONFIG_ADDRESS_PORT) {
    if (data & CONFIG_ADDRESS_ENABLE_BIT)
      lastIO = data;
    else
      lastIO = 0;
    return;
  } else if ( port == CONFIG_DATA_PORT && 
              (lastIO & CONFIG_ADDRESS_ENABLE_BIT) ) { // config mechanism 1
    bus_number = (lastIO >> 16) & 0xFF;
    device_number = (lastIO >> 11) & 0x1F;
    function_number = (lastIO >> 8) & 0x7;
    register_offset = (lastIO >>0) & 0xFC;
    halCfgStore32(port,1,bus_number,device_number,function_number,register_offset,data);
    return;
  } else if (lastIO == CONFIG_MAPPING_ENABLE_BYTE) {    // config mechanism 2
    device_number = ((port-CONFIG_MAPPING_OFFSET) >> 8) & 0xFF;
    register_offset = (port >>0) & 0xFC;
    function_number = 0;
    halCfgStore32(port,2,1,device_number,function_number,register_offset,data);
    return;
  }    

#ifdef HAL_NEW_PORT_IO
  // do some sanity checks
  if ( SST_BAD_PORT(port) ) {
    GDBG_ERROR("SETIO","bad port=0x%x  data=%d(0x%08x)\n",
               port,data,data);
    return;
  }
  if ( port & 3 ) {
    GDBG_ERROR("SETIO","unaligned port=0x%x  data=%d(0x%08x)\n",
               port,data,data);
    return;
  }

  bn = SST_FAKE_PORT_GET_BOARD(port);
  physBase = halInfo.boardInfo[bn].physPort;

  // perform the port write operation

#ifndef KERNEL
  if (halInfo.hw) {
    GDBG_INFO(121,"     W PCI IO(0x%x,%11d(0x%08x))\n",physBase+portOffset,data,data);
  }
#endif

#ifdef HAL_HSIM
  if (halInfo.hsim) {
    GDBG_INFO(121,"     W PCI IO(0x%x,%11d(0x%08x))\n",physBase+portOffset,data,data);
    PCI_IOW32(physBase+portOffset,data);
  }
#endif

  if (halInfo.csim && halInfo.csimio) {
    GDBG_INFO(120,"       SET_IO(0x%x,%11d(0x%08x))\n",port,data,data);
    lastIO = 0;
    if (!ioEnabled) 
      GDBG_ERROR("SETIO","port access is not enabled\n");
    csimOutPort32( SST_FAKE_PORT_GET_CSIM(port), port, data );
  }

#else // HAL_NEW_PORT_IO

#ifndef KERNEL
  if (halInfo.hw) {
    GDBG_INFO(121,"     W PCI IO(0x%x,%11d(0x%08x))\n",port,data,data);
  }
#endif

#ifdef HAL_HSIM
  if (halInfo.hsim) {
    GDBG_INFO(121,"     W PCI IO(0x%x,%11d(0x%08x))\n",port,data,data);
    PCI_IOW32(port,data);
  }
#endif

  if (halInfo.csim && halInfo.csimio) {
    GDBG_INFO(120,"       SET_IO(0x%x,%11d(0x%08x))\n",port,data,data);
    if (port == CONFIG_ADDRESS_PORT) {
        GDBG_ERROR("halOutPort32","config mechanism 2 not supported\n");
    } else {
      for(bn=0; bn<halInfo.boardsFound; bn++) {
        physBase = halInfo.boardInfo[bn].physPort;
        if ( port >= physBase  &&  port <= (physBase + 0xFF) )
          csimOutPort32(halInfo.boardInfo[bn].sstCSIM, (FxU16) (port-physBase), data);
      }
    }
  }
  
#endif // HAL_NEW_PORT_IO

  lastIO = 0;

}

//----------------------------------------------------------------------
// execute an 8 bit port input operation
//----------------------------------------------------------------------
FxU8
halInPort8( FxU16 port ) 
{
  FxU8 data = 0xFF;

  if (halInfo.hw) {
    GDBG_INFO(121,"     R PCI IO8(0x%x,%11d(0x%08x))\n",port,data,data);
    GDBG_ERROR("halInPort8","8-bit port input nyi for hardware\n");
  }

#ifdef HAL_HSIM
  if (halInfo.hsim) {
    data = (FxU8) PCI_IOR8(port);
    GDBG_INFO(121,"     R PCI IO8(0x%x,%11d(0x%08x))\n",port,data,data);
  }
#endif

  if (halInfo.csim && halInfo.csimio) {
    // csim just ignores port i/o operations
    GDBG_INFO(120,"       GET_IO8(0x%x,%11d(0x%08x))\n",port,data,data);
    GDBG_INFO(0,"halInPort8: WARNING, 8-bit port input nyi for CSIM\n");
  }
  lastIO = 0;

  return data;
}

//----------------------------------------------------------------------
// execute an 16 bit port input operation
//----------------------------------------------------------------------
FxU16
halInPort16( FxU16 port ) 
{
  FxU16 data = 0xFFFF;

  if (halInfo.hw) {
    GDBG_INFO(121,"     R PCI IO16(0x%x,%11d(0x%08x))\n",port,data,data);
    GDBG_ERROR("halInPort16","16-bit port input nyi for hardware\n");
  }

#ifdef HAL_HSIM
  if (halInfo.hsim) {
    data = (FxU16) PCI_IOR16(port);
    GDBG_INFO(121,"     R PCI IO16(0x%x,%11d(0x%08x))\n",port,data,data);
  }
#endif

  if (halInfo.csim && halInfo.csimio) {
    // csim just ignores port i/o operations
    GDBG_INFO(120,"       GET_IO16(0x%x,%11d(0x%08x))\n",port,data,data);
    GDBG_INFO(0,"halInPort16: WARNING, 16-bit port input nyi for CSIM\n");
  }
  lastIO = 0;

  return data;
}

//----------------------------------------------------------------------
// execute an 32 bit port input operation
//----------------------------------------------------------------------
FxU32
halInPort32( FxU16 port ) 
{
  FxI16 portOffset = SST_FAKE_PORT_GET_OFFSET(port);
  FxU16 bn, physBase;
  FxU32 data = 0xFFFFFFFF;
  FxU32 device_number = 0, function_number = 0, register_offset = 0;

  // check for config cycles
  if (port == CONFIG_DATA_PORT) {
    if (lastIO & CONFIG_ADDRESS_ENABLE_BIT) {           // config mechanism 1
      FxU32 bus_number =      (lastIO >> 16) & 0xFF;
      FxU32 device_number =   (lastIO >> 11) & 0x1F;
      FxU32 function_number = (lastIO >>  8) & 0x7;
      FxU32 register_offset = (lastIO >>  0) & 0xFC;
      data = halCfgLoad32(port,1,bus_number, device_number,function_number,register_offset);
      return data;
    } 
  } else if (lastIO == CONFIG_MAPPING_ENABLE_BYTE) {    // config mechanism 2
    data = halCfgLoad32(port,2,0,device_number,function_number,register_offset);
    return data;
  }

#ifdef HAL_NEW_PORT_IO
  // do some sanity checks
  if ( port != CONFIG_DATA_PORT && SST_BAD_PORT(port) ) {
    GDBG_ERROR("GETIO","bad port=0x%x  data=%d(0x%08x)\n",
               port,data,data);
    return data;
  }
  if ( port & 3 ) {
    GDBG_ERROR("GETIO","unaligned port=0x%x  data=%d(0x%08x)\n",
               port,data,data);
    return data;
  }

  bn = SST_FAKE_PORT_GET_BOARD(port);
  physBase = halInfo.boardInfo[bn].physPort;

  // perform the i/o read operation

  if (halInfo.csim && halInfo.csimio) {
    if (!ioEnabled) 
      GDBG_ERROR("GETIO","port access is not enabled\n");
    data = csimInPort32( SST_FAKE_PORT_GET_CSIM(port), port );
    GDBG_INFO(120,"       GET_IO(0x%x,%11d(0x%08x))\n",port,data,data);
  }

#ifdef HAL_HSIM
  if (halInfo.hsim) {
    data = (FxU32) PCI_IOR32(physBase+portOffset);
    GDBG_INFO(121,"     R PCI IO(0x%x,%11d(0x%08x))\n",physBase+portOffset,data,data);
  }
#endif
  
#ifndef KERNEL
  if (halInfo.hw) {
    GDBG_INFO(121,"     R PCI IO(0x%x,%11d(0x%08x))\n",physBase+portOffset,data,data);
  }
#endif

#else  // HAL_NEW_PORT_IO

  if (halInfo.csim && halInfo.csimio) {           // NOTE: we read CSIM first and save it
    int i=0;
    GDBG_INFO(120,"       GET_IO(0x%x,%11d(0x%08x))\n",port,data,data);
    for(bn=0; bn<halInfo.boardsFound; bn++) {
      physBase = halInfo.boardInfo[bn].physPort;
      if ( port >= physBase  &&  port <= (physBase + 0xFF) ) {
        i++;
        data = csimInPort32(halInfo.boardInfo[bn].sstCSIM, 
                            (FxU16) (port-physBase));
        halInfo.csimLastRead = data;
      }
      if ( i > 1 )
        GDBG_INFO(0,"halInPort32: WARNING, port address 0x%x maps to %d boards\n",port,i);
    }
  }
  
#ifdef HAL_HSIM
  if (halInfo.hsim) {
    data = (FxU32) PCI_IOR32(physBase+portOffset);
    GDBG_INFO(121,"     R PCI IO(0x%x,%11d(0x%08x))\n",port,data,data);
  }
#endif
  
#ifndef KERNEL
  if (halInfo.hw) {
    GDBG_INFO(121,"     R PCI IO(0x%x,%11d(0x%08x))\n",port,data,data);
  }
#endif

#endif // HAL_NEW_PORT_IO

  lastIO = 0;

  return data;
}

