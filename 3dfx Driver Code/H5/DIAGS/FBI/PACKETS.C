#include <assert.h>
#include <string.h>

#include "udiag.h"
#include "sstdiag.h"

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//
//  This is an ad hoc piece of poop diag targeted at testing some
//  random packet stuff. It basically tests the packet 5 and packet 6
//  can both touch the entire 128MB memory space.  
//
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


FxU32 configureCmdFifo(SstRegs *sst);
void configureAGPMemory(SstRegs *sst);
void disableCmdFifo(SstRegs *sst);
FxU32 insertPacket5(SstRegs * sst, FxU32 cmdFifoPtr, FxU32 pciStride);
FxU32 insertPacket6(SstRegs * sst, FxU32 cmdFifoPtr, FxU32 pciStride);

FxU8 *agpMemory;

int main(int argc, char **argv)
{
  SstRegs *sst;
  SstIORegs *sstio;
  CsimPrivate *cpriv;
  FxU32 cmdFifoPtr;
  FxI32 i, j, pciStrideIndex, pciStride;
  FxU32 pciStrideValues[] = {SST_RAW_LFB_ADDR_STRIDE_1K,
			     SST_RAW_LFB_ADDR_STRIDE_2K,
			     SST_RAW_LFB_ADDR_STRIDE_4K,
			     SST_RAW_LFB_ADDR_STRIDE_8K};
  FxBool onlyPacket5 = FXFALSE;

  //We're skipping SST_RAW_LFB_ADDR_STRIDE_16K, because we're assuming it won't be used
		
  //Go through and look for --onlyPacket5 option
  for(i=1; i<argc; i++)
    {
      if(!strcmp(argv[i], "--onlyPacket5"))
	{	  
	  onlyPacket5 = FXTRUE;
	  
	  for(j=i+1; j<argc; j++)
	    argv[j-1] = argv[j];
	  argc--;
	  i--;
	}
    }	     

  sst = SST_BEGIN(argc, argv);
  sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
  cpriv = CSIM_PRIVATE(diago.sstCSIM);

  if(diago.chipCount > 1)
    {
      GDBG_ERROR("packets::main", "Can only run packets single chip\n");
      DIAG_FAIL();
    }

  //Make sure that we're running with 64MB
  if(cpriv->memorySizeInBytes != 64*1024*1024)
    {
      GDBG_ERROR("packets::main", "Must run with 64MB framebuffer\n");
      DIAG_FAIL();
    }

  //Set up the agp memory used for scratch and the command fifo
  configureAGPMemory(sst);


  while(DIAG_STARTPASS())
    {

      //Go through all the pciStrides
      for(pciStrideIndex=0; pciStrideIndex<sizeof(pciStrideValues) / sizeof(FxU32); pciStrideIndex++)
	{ 
	  //Idle the f'er
	  fxHalIdleNoNop(sst);
	  
	  pciStride = 1 << (10 + pciStrideIndex);
	  GDBG_INFO(0, "Using pci stride %d\n", pciStride);
	  
	  //Set up the pci stride. Also, set the tile stride to span the whole pci stride
	  SET(sstio->lfbMemoryConfig, SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE(0) |
	      pciStrideValues[pciStrideIndex] | ((pciStride / 128) << SST_RAW_LFB_TILE_STRIDE_SHIFT));
	  
	  //Set up the command FIFO
	  cmdFifoPtr = configureCmdFifo(sst);
	  
	  //Put a bunch of packets in the command FIFO
	  for(i=0; i<20; i++)
	    {
	      if(onlyPacket5)
		cmdFifoPtr = insertPacket5(sst, cmdFifoPtr, pciStride);
	      else
		{
		  if(iRandom(1))
		    cmdFifoPtr = insertPacket5(sst, cmdFifoPtr, pciStride);
		  else
		    cmdFifoPtr = insertPacket6(sst, cmdFifoPtr, pciStride);	      
		}
	    }

	  //Turn the command FIFO off
	  disableCmdFifo(sst);
	}
    }
  
  GDBG_INFO(0, "Done inserting packets.\n");
  
  DIAG_PASS(0);

  return(0);
}

//Configure a 1MB(well almost 1MB) command fifo at the bottom of the framebuffer
FxU32 configureCmdFifo(SstRegs *sst)
{
  SstCRegs *sstc;

  FxU32 cmdFifoPtr, agpReadPtr;

  sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);

  //Configure a 32KB command FIFO at the bottom of AGP memory
  agpReadPtr = (diago.halInfo->boardInfo[0].agpBaseAddrL & ~0xFFF) + 4;

  //Turn the command FIFO off to be safe
  SET(sstc->cmdFifo0.baseSize, 0);

  assert(diago.halInfo->boardInfo[0].agpBaseAddrH == 0);
  SET(sstc->cmdFifo0.baseAddrL, diago.halInfo->boardInfo[0].agpBaseAddrL >> 12);
  SET(sstc->cmdFifo0.readPtrL, agpReadPtr);
  SET(sstc->cmdFifo0.readPtrH, 0);
  SET(sstc->cmdFifo0.aMin, agpReadPtr - 4);
  SET(sstc->cmdFifo0.aMax, agpReadPtr - 4);
  SET(sstc->cmdFifo0.depth, 0);
  SET(sstc->cmdFifo0.holeCount, 0);

  //Only Jesus knows what this does
  SET(sstc->cmdFifoThresh, 0x122);  


  //Enable the command fifo
  SET(sstc->cmdFifo0.baseSize, (7 << SST_BASESIZE_SHIFT) | SST_CMDFIFOEN | 
      SST_CMDFIFOAGP | SST_HOLECNTDISABLE);      


  cmdFifoPtr = ((FxU32)agpMemory) + 4;

  return(cmdFifoPtr);
}


//Allocate a little bit of AGP memory to work with
void configureAGPMemory(SstRegs *sst)
{
  FxI32 i, agpMemorySize;

  agpMemorySize=65536;

  agpMemory = (FxU8 *)AGPMEMALLOC(sst, agpMemorySize);
  GDBG_INFO(0, "agpMemory = 0x%x\n", agpMemory);

  if(agpMemory == NULL)
    {
      GDBG_ERROR("configureAGPMemory", "Can't allocate agp memory! %s(%d)\n",
		 __FILE__, __LINE__);
      DIAG_FAIL();
    }

  //Write some garbage to the AGP memory
  for(i=0; i<agpMemorySize; i+=4)
    agpWriteMem32((FxU32 *)(agpMemory + i), iRandom(0xFFFFFFFF));
}

void disableCmdFifo(SstRegs *sst)
{
  SstCRegs *sstc;

  sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);
  
  //Idle the f'er
  fxHalIdleNoNop(sst);
  
  //Turn the command fifo off
  SET(sstc->cmdFifo0.baseSize, 0);
}

//This inserts a random raw lfb packet
FxU32 insertPacket5(SstRegs * sst, FxU32 cmdFifoPtr, FxU32 pciStride)
{
  FxU32 baseAddress;
  FxI32 i, dataCount;
  FxU32 originalCmdFifoPtr = cmdFifoPtr;
  SstCRegs *sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);
  FxU32 maxY;

  GDBG_INFO(3, "insertPacket5(cmdFifoPtr = 0x%08x)\n", cmdFifoPtr);

  //Build the packet header
  dataCount = iRandom(7) + 1;

  agpWriteMem32((void *)cmdFifoPtr, 
		SSTCP_PKT5 | 
		(dataCount << SSTCP_PKT5_NWORDS_SHIFT) |
		SSTCP_PKT5_LFB);
  cmdFifoPtr+=4;

  maxY = (64*1024*1024/pciStride) - 1;
  baseAddress = (iRandom(maxY) * pciStride);
  agpWriteMem32((void *)cmdFifoPtr, baseAddress);
  cmdFifoPtr+=4;
  
  GDBG_INFO(4, "insertPacket5: pciStride=%d  maxY=%d  baseAddress=0x%08x  dataCount=%d\n",
	    pciStride, maxY, baseAddress, dataCount);

  //Generate the packet data
  for(i=0; i< dataCount; i++)
    {
      agpWriteMem32((void*)cmdFifoPtr, iRandom(0xFFFFFFFF));
      cmdFifoPtr+=4;
    }

  if(cmdFifoPtr - (FxU32)agpMemory >= 32768)
    {
      GDBG_ERROR("insertPacket5", "AGP command FIFO got too big\n");
      DIAG_FAIL();
    }

  //Bump the aMin, aMax up
  SET(sstc->cmdFifo0.bump, (((cmdFifoPtr - originalCmdFifoPtr))/4));

  return(cmdFifoPtr);
}

FxU32 insertPacket6(SstRegs *sst, FxU32 cmdFifoPtr, FxU32 pciStride)
{
  FxU32 baseAddress;
  FxI32 dataCount;
  FxU32 maxY;
  
  //AGP physical address
  FxU32 agpBaseAddrH, agpBaseAddrL, agpSizeInBytes;

  SstCRegs *sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);

  agpBaseAddrH = diago.halInfo->boardInfo[0].agpBaseAddrH;
  agpBaseAddrL = diago.halInfo->boardInfo[0].agpBaseAddrL;
  agpSizeInBytes = diago.halInfo->boardInfo[0].agpSizeInBytes;

  if(agpSizeInBytes < 65536)
    {
      GDBG_ERROR("insertPacket6", "Must have at least 64KB of AGP memory!");
      DIAG_FAIL();
    }

  GDBG_INFO(3, "insertPacket6(cmdFifoPtr = 0x%08x)\n", cmdFifoPtr);

  //Build the packet header (word 0)
  dataCount = 4;
  
  agpWriteMem32((void *)cmdFifoPtr, 
		SSTCP_PKT6 | 
		(dataCount << SSTCP_PKT6_NBYTES_SHIFT) |
		SSTCP_PKT6_LFB);
  cmdFifoPtr += 4;

  //AGP Low address (word 1)
  agpWriteMem32((void *)cmdFifoPtr, agpBaseAddrL + 32768 + (iRandom(32000) & ~0x3));
  cmdFifoPtr += 4;

  //word 2
  agpWriteMem32((void *)cmdFifoPtr,
		(4 & SSTCP_PKT6_SRC_WIDTH) |
		((4 << SSTCP_PKT6_SRC_STRIDE_SHIFT) & SSTCP_PKT6_SRC_STRIDE));
  cmdFifoPtr += 4;

  //word 3
  maxY = (64*1024*1024 / pciStride) - 1;
  baseAddress = (iRandom(maxY) * pciStride);
  GDBG_INFO(4, "insertPacket6: pciStride=%d  maxY=%d  baseAddress=0x%08x  dataCount=%d\n",
	    pciStride, maxY, baseAddress, dataCount);


  agpWriteMem32((void *)cmdFifoPtr, baseAddress);
  cmdFifoPtr += 4;
  
  //word 4
  agpWriteMem32((void *)cmdFifoPtr,SSTCP_PKT6_DST_STRIDE & 4);
  cmdFifoPtr += 4;

  //Bump the aMin, aMax up
  SET(sstc->cmdFifo0.bump, 5);
  

  return(cmdFifoPtr);
}
