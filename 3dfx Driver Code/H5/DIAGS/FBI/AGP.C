#include <assert.h>
#include <string.h>

#include "udiag.h"
#include "sstdiag.h"

FxU32 configureAGPMemory(SstRegs *sst, FxU32 agpMemorySize);

int main(int argc, char **argv)
{
  SstRegs *sst;
  SstIORegs *sstio;
  SstCRegs *sstc;
  CsimPrivate *cpriv;
  FxU32 agpMemory, agpPhysicalMemory, agpMemorySize, i;
  FxU32 rawLFB;
  FxU32 lfbData, agpData[1024];

  sst = SST_BEGIN(argc, argv);
  sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
  sstc = (SstCRegs *)SST_CMDAGP_ADDRESS(sst);
  cpriv = CSIM_PRIVATE(diago.sstCSIM);
  rawLFB = SST_RAW_LFB_OFFSET + SST_BASE_ADDRESS(sst);

  //Set up the agp memory
  agpMemorySize = 65536;
  agpMemory = configureAGPMemory(sst, agpMemorySize);
  
  while(DIAG_STARTPASS())
    {
      //Write some garbage to the AGP memory
      for(i=0; i<16; i+=4)
	{
	  agpData[i/4] = iRandom(0xFFFFFFFF);
	  agpWriteMem32((FxU32 *)(agpMemory + i), agpData[i/4]);
	}

      agpPhysicalMemory = diago.halInfo->boardInfo[0].agpBaseAddrL;

      //Perform an AGP mem move
      SET(sstc->agpReqSize, 16);
      SET(sstc->hostAddrLow, agpPhysicalMemory);
      SET(sstc->hostAddrHigh, ((16 << SST_AGPWIDTH_SHIFT) & SST_AGPWIDTH)
	  | ((16 << SST_AGPSTRIDE_SHIFT) & SST_AGPSTRIDE));
      SET(sstc->graphicsAddr, 0);
      SET(sstc->graphicsStride, 16);
      SET(sstc->moveCMD, SST_AGPMEMTYPE_RAWLFB | SST_CMDFIFO_0);
      fxHalIdle(sst);
      
      //Check result of move
      for(i=0; i<16; i += 4)
	{
	  lfbData = halLoad32((FxU32 *)(rawLFB + i));
	  
	  if(lfbData != agpData[i/4])
	    GDBG_ERROR("agp::main", "index %3d: lfbData=0x%08x agpData=0x%08x\n",
		       i, lfbData, agpData[i/4]);
	}
      
    }
  DIAG_PASS(0);

  return(0);
}


//Allocate a little bit of AGP memory to work with
FxU32 configureAGPMemory(SstRegs *sst, FxU32 agpMemorySize)
{
  FxU32 agpMemory;

  agpMemory = (FxU32)AGPMEMALLOC(sst, agpMemorySize);
  GDBG_INFO(0, "agpMemory = 0x%x\n", agpMemory);

  if(agpMemory == (FxU32)NULL)
    {
      GDBG_ERROR("configureAGPMemory", "Can't allocate agp memory! %s(%d)\n",
		 __FILE__, __LINE__);
      DIAG_FAIL();
    }

  return(agpMemory);
}

