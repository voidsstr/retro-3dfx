#include "udiag.h"
#include "sstdiag.h"

FxBool testMemory(FxU32 value);
FxBool fill(SstRegs *sst, FxU32 chipIndex, FxU32 size, FxU32 value);

int main(int argc, char **argv)
{
  SstRegs *sst;
  FxU32 chipIndex;

  sst = SST_BEGIN(argc,argv);

  //Disable snooping
  for(chipIndex=0; chipIndex<(FxU32)diago.chipCount; chipIndex++)
    {
      FxU32 cfgInitEnable;
      const PciRegister CFG_INIT_ENABLE = { 65, 3, READ_WRITE };

      pciGetConfigData(CFG_INIT_ENABLE, diago.halInfo->boardInfo[chipIndex].deviceNumber, &cfgInitEnable);
      cfgInitEnable &= ~(SST_ADDRESS_SNOOP_ENABLE | SST_MEMBASE0_SNOOP_ENABLE |
			 SST_MEMBASE1_SNOOP_ENABLE | SST_INIT_REGISTER_SNOOP_ENABLE);
      pciSetConfigData(CFG_INIT_ENABLE, diago.halInfo->boardInfo[chipIndex].deviceNumber, &cfgInitEnable);
    }

  while(DIAG_STARTPASS())
    {
      testMemory(0xFFFFFFFF);
      testMemory(0x00000000);
      testMemory(0xa5a5a5a5);
      testMemory(0x5a5a5a5a);
    }
  DIAG_PASS(-1);

  return(0);
}

FxBool fill(SstRegs *sst, FxU32 chipIndex, FxU32 size, FxU32 value)
{
  FxU32 index;
  FxU32 *rawLFB;
  FxU32 readValue;

  GDBG_INFO(0, "Filling chip %d's %dMB of framebuffer with 0x%08x\n", 
	    chipIndex, size>>20, value);

  rawLFB = (FxU32*)(SST_RAW_LFB_OFFSET + SST_BASE_ADDRESS(sst));
  
  //Write the data
  for(index=0; index<size; index+=4)
    SET(rawLFB[index/4], value);

  //Read the data
  for(index=0; index<size; index+=4)
    {
      readValue = GET(rawLFB[index/4]);
      
      if(readValue != value)
	{
	  GDBG_ERROR("Shit!", "Chip %d failed at address 0x%08x. Wrote 0x%08x, but read back 0x%08x.\n",
		     chipIndex, index, value, readValue);
	  return(FXFALSE);
	}
    }
  

  return(FXTRUE);
}

FxBool testMemory(FxU32 value)
{
  FxU32 size;
  FxU32 chipIndex;
  SstRegs *sst;
  SstIORegs *sstio;
  FxBool passed=FXTRUE;
  
  size = CSIM_PRIVATE(diago.sstCSIM)->memorySizeInBytes;

  for(chipIndex=0; chipIndex<(FxU32)diago.chipCount; chipIndex++)
    {
      if(chipIndex == 0)
	sst = diago.sst;
      else
	sst = diago.sstChildren[chipIndex-1];
      sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
      
      //Set lfbMemoryConfig to make all memory linear
      SET(sstio->lfbMemoryConfig, SST_RAW_LFB_TILE_BEGIN_PAGE);

      //fill memory and check
      passed = fill(sst, chipIndex, size, value);

      if(!passed)
	return(FXFALSE);
    }
  
  return(FXTRUE);
}
