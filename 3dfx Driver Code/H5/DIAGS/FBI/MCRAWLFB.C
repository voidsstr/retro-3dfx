#include <assert.h>
#include <stddef.h>

#include "udiag.h"
#include "sstdiag.h"



FxBool checkMemories32(FxBool shouldPass, FxU32 chipMask, FxU32 address, FxU32 data);
void configureChips(int nSamples);
FxBool setMemories32(FxU32 chipMask, FxU32 address, FxU32 data);
void testLfbMemoryConfig(SstRegs *sst);
void testLfbReads(void);
void testLfbWrites(void);
FxBool testRead(FxBool shouldPass, FxU32 address, FxU32 data);

int main(int argc, char **argv)
{
  SstRegs *sst;
  int fakeArgc;
  char *fakeArgv[10];
  CsimPrivate *cp;
  int i;

  //Set up the command-line arguments. This diag disregards command line args
  fakeArgc=0;
  fakeArgv[fakeArgc++] = argv[0];
  fakeArgv[fakeArgc++] = "--chipCount";
  fakeArgv[fakeArgc++] = "4";
  fakeArgv[fakeArgc++] = "--enableSLI";

  //Scan for -H argument
  for(i=0; i<argc; i++)
    {
      if(!strcmp(argv[i], "-H"))
	fakeArgv[fakeArgc++] = "-H";
    }

  sst = SST_BEGIN(fakeArgc, fakeArgv);
  cp = CSIM_PRIVATE(diago.halInfo->boardInfo[0].sstCSIM);

  if(diago.chipCount != 4)
    {
      GDBG_ERROR("mcrawlfb::main", "Must run with 4 chips!\n");
      DIAG_FAIL();
    }

  if(cp->memorySizeInBytes != 8*1024*1024)
    {
      GDBG_ERROR("mcrawlfb::main", "Must run with 8MB framebuffers!\n");
      DIAG_FAIL();
    }
  
  if(diago.width != 640)
    {
      GDBG_ERROR("mcrawlfb::main", "Must run in 640x480\n");
      DIAG_FAIL();
    }
  
  while(DIAG_STARTPASS())
    {      
      //Make sure we're setup as 32 bits per pixel framebuffer
      SET(sst->renderMode, SST_RM_32BPP | SST_RM_RGBA_WMASK);

      testLfbMemoryConfig(sst);      
      testLfbWrites();      
      testLfbReads();
    }
  DIAG_PASS(0);

  return(0);
}


//This is a behind the back memory check.
//This function will prevent this diag from running with 
//a command fifo.
FxBool checkMemories32(FxBool shouldPass, FxU32 chipMask, FxU32 address, FxU32 data)
{
  FxU32 chipIndex;
  SstRegs *sst;
  CsimPrivate *cp;
  FxU32 memoryValue;

  assert((address & 3) == 0);

  #ifdef HAL_HSIM
  if(diago.halInfo->hsim)
    {
      //If running against hsim, idle the chip
      sst_idle(diago.halInfo->boardInfo[0].virtAddr[0]);
    }
  #endif


  for(chipIndex=0; chipIndex<4; chipIndex++)
    {
      if(!((1 << chipIndex) & chipMask))
	continue;
      
      sst = diago.halInfo->boardInfo[chipIndex].sstCSIM;
      cp = CSIM_PRIVATE(sst);
      assert(cp->memory);

      memoryValue = *((FxU32 *)(&cp->memory[address]));
      
      #ifdef ENDB
      memoryValue = 
	(((memoryValue >> 24) & 0xFF) <<  0) |
	(((memoryValue >> 16) & 0xFF) <<  8) |
	(((memoryValue >>  8) & 0xFF) << 16) |
	(((memoryValue >>  0) & 0xFF) << 24);
      #endif

      if(shouldPass && memoryValue != data)
	{
	  GDBG_ERROR("checkMemories32(csim)", "Expected %s[0x%08x] = 0x%08x, got 0x%08x\n",
		     cp->environment.name, address, data, memoryValue);
	  return(FXFALSE);
	}
      else if((!shouldPass) && memoryValue == data)
	{
	  GDBG_ERROR("checkMemories32(csim)", "Expected %s[0x%08x] != 0x%08x, got 0x%08x\n",
		     cp->environment.name, address, data, memoryValue);
	  return(FXFALSE);
	}

      #ifdef HAL_HSIM
      if(diago.halInfo->hsim)
	{
	  assert(cp->hsim_memory);
	  memoryValue = *((FxU32 *)(&cp->hsim_memory[address]));
	  
          #ifdef ENDB
	  memoryValue = 
	    (((memoryValue >> 24) & 0xFF) <<  0) |
	    (((memoryValue >> 16) & 0xFF) <<  8) |
	    (((memoryValue >>  8) & 0xFF) << 16) |
	    (((memoryValue >>  0) & 0xFF) << 24);
          #endif

	  if(shouldPass && memoryValue != data)
	    {
	      GDBG_ERROR("checkMemories32(hsim)", "Expected %s[0x%08x] = 0x%08x, got 0x%08x\n",
			 cp->environment.name, address, data, memoryValue);
	      return(FXFALSE);
	    }
	  else if((!shouldPass) && memoryValue == data)
	    {
	      GDBG_ERROR("checkMemories32(hsim)", "Expected %s[0x%08x] != 0x%08x, got 0x%08x\n",
			 cp->environment.name, address, data, memoryValue);
	      return(FXFALSE);
	    }
	  
	}
      #endif

    }

  return(FXTRUE);
}

//This configures the sliCtrl and aaCtrl registers
void configureChips(int nSamples)
{
  SstRegs *sst;
  SstIORegs *sstio;
  FxI32 chipIndex;
  FxU32 aaCtrl, sliCtrl;
  FxU32 cfgSliLfbCtrl, cfgAADepthBufferAperture, cfgAALfbCtrl;
  FxU32 renderMask, compareMask, scanMask, logNChips;

  /*
Here's what the memory looks like
8MB
7MB  
6MB  
5MB  
4MB  <- Tile space starts here
3MB  
2MB
1MB  <- Secondary psuedo-tile space start here  (Page 256)
0MB  

Using 11 bits of X in tiled write addresses
Tile stride of 20.
Color buffer at y=0     Pages [0,   300)
Depth buffer at y=480   Pages [300, 600)
   */
  
  assert(diago.halInfo->boardsFound == 4);
  
  for(chipIndex=0; chipIndex<4; chipIndex++)
    {
      sst = diago.halInfo->boardInfo[chipIndex].virtAddr[0];
      sstio = (SstIORegs *)SST_IO_ADDRESS(sst);

      //Figure out what to write to registers
      switch(nSamples)
	{
	case 1:
	  aaCtrl = 0;
	  renderMask  = 3;
	  compareMask = chipIndex;
	  scanMask    = 0;
	  logNChips   = 2;
	  break;

	case 2:
	  aaCtrl = SST_AA_CONTROL_AA_ENABLE;
	  renderMask  = 3;
	  compareMask = chipIndex;
	  scanMask    = 0;
	  logNChips   = 2;
	  break;

	case 4:
	  aaCtrl = SST_AA_CONTROL_AA_ENABLE;
	  renderMask  = 1;
	  compareMask = chipIndex/2;
	  scanMask    = 0;
	  logNChips   = 1;
	  break;

	default:
	  assert(0);
	}
            
      sliCtrl =  
	(renderMask  << SST_SLI_CONTROL_RENDER_MASK_SHIFT) |	
	(compareMask << SST_SLI_CONTROL_COMPARE_MASK_SHIFT) |
	(scanMask    << SST_SLI_CONTROL_SCAN_MASK_SHIFT) |
	(logNChips   << SST_SLI_CONTROL_LOG2_CHIP_COUNT_SHIFT) |
	SST_SLI_CONTROL_SLI_ENABLE;
      cfgSliLfbCtrl = 
	(renderMask  << SST_SLI_LFB_RENDERMASK_SHIFT) |
	(compareMask << SST_SLI_LFB_COMPAREMASK_SHIFT) |
	(scanMask    << SST_SLI_LFB_SCANMASK_SHIFT) |
	(logNChips   << SST_SLI_LFB_NUMCHIPS_LOG2_SHIFT) |
	SST_SLI_LFB_CPU_WRITE_ENABLE |
	SST_SLI_LFB_DISPATCH_WRITE_ENABLE |
	SST_SLI_LFB_READ_ENABLE;
      cfgAADepthBufferAperture = 
	(0x4F0 << SST_AA_DEPTH_BUFFER_APERTURE_BEGIN_SHIFT) |
	(0x5E0 << SST_AA_DEPTH_BUFFER_APERTURE_END_SHIFT);
      cfgAALfbCtrl = 
	((1024*1024/16) << SST_SECONDARY_BUFFER_BASE_SHIFT) |
	SST_AA_LFB_CPU_WRITE_ENABLE |
	SST_AA_LFB_DISPATCH_WRITE_ENABLE |
	SST_AA_LFB_READ_ENABLE |
	SST_AA_LFB_READ_FORMAT_32BPP |
	((nSamples == 4) ? SST_AA_LFB_RD_DIVIDE_BY_FOUR : 0);

      SET(sst->aaCtrl,  aaCtrl);
      SET(sst->sliCtrl, sliCtrl);      
      halCfgStore32(0x3FC, 1, diago.halInfo->boardInfo[chipIndex].pciBusNumber,
		    diago.halInfo->boardInfo[chipIndex].pciDeviceNumber,
		    diago.halInfo->boardInfo[chipIndex].pciFunctionNumber,
		    offsetof(SstPCIConfigRegs, cfgSliLfbCtrl),
		    cfgSliLfbCtrl);
      halCfgStore32(0x3FC, 1, diago.halInfo->boardInfo[chipIndex].pciBusNumber,
		    diago.halInfo->boardInfo[chipIndex].pciDeviceNumber,
		    diago.halInfo->boardInfo[chipIndex].pciFunctionNumber,
		    offsetof(SstPCIConfigRegs, cfgAADepthBufferAperture),
		    cfgAADepthBufferAperture);
      halCfgStore32(0x3FC, 1, diago.halInfo->boardInfo[chipIndex].pciBusNumber,
		    diago.halInfo->boardInfo[chipIndex].pciDeviceNumber,
		    diago.halInfo->boardInfo[chipIndex].pciFunctionNumber,
		    offsetof(SstPCIConfigRegs, cfgAALfbCtrl),
		    cfgAALfbCtrl);			     

		  
      //Set tiled memory beginning at 4MB
      SET(sstio->lfbMemoryConfig, SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE(4*1024*1024/SST_TILE_SIZE) |
	  SST_RAW_LFB_ADDR_STRIDE_2K | ((640 * 4 / SST_TILE_WIDTH) << SST_RAW_LFB_TILE_STRIDE_SHIFT));  

      //Disable tile compare marker
      SET(sstio->lfbMemoryConfig, SST_RAW_LFB_WRITE_CONTROL);
    }
}

//This sticks values in the appropriate chips' memories
FxBool setMemories32(FxU32 chipMask, FxU32 address, FxU32 data)
{
  FxU32 chipIndex;
  SstRegs *sst;
  CsimPrivate *cp;

  assert((address & 3) == 0);

  for(chipIndex=0; chipIndex<4; chipIndex++)
    {
      if(!((1 << chipIndex) & chipMask))
	continue;
      
      sst = diago.halInfo->boardInfo[chipIndex].sstCSIM;
      cp = CSIM_PRIVATE(sst);
      assert(cp->memory);
      
      *((FxU32 *)(&cp->memory[address])) = data;
    }

  return(FXTRUE);
}

//This tests the modal properties of lfbMemoryConfig
void testLfbMemoryConfig(SstRegs *sst)
{
  CsimPrivate *cp;
  SstIORegs *sstio;
  FxU32 lfbMemoryTileCtrl, lfbMemoryTileCompare;
  FxU32 maximumTile, tileSpaceBeginTile, tileSpaceCompareTile, data;
  FxI32 i;

  sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
  cp = CSIM_PRIVATE(diago.sstCSIM);
  
  for(i=0; i<20; i++)
    {
      maximumTile = (cp->memorySizeInBytes - 1) / SST_TILE_SIZE;
      tileSpaceBeginTile   = rRandom(0, maximumTile);
      tileSpaceCompareTile = rRandom(tileSpaceBeginTile, maximumTile);
  
      //Write lfbMemoryTileCtrl
      GDBG_INFO(145, "Write lfbMemoryTileCtrl\n");
      lfbMemoryTileCtrl = SST_RAW_LFB_TILE_BEGIN_PAGE_MUNGE(tileSpaceBeginTile) |
	SST_RAW_LFB_ADDR_STRIDE_2K | (20 << SST_RAW_LFB_TILE_STRIDE_SHIFT);
      SET(sstio->lfbMemoryConfig, lfbMemoryTileCtrl);

      //Write lfbMemoryTileCompare
      GDBG_INFO(145, "Write lfbMemoryTileCompare\n");
      lfbMemoryTileCompare = tileSpaceCompareTile;
      SET(sstio->lfbMemoryConfig, lfbMemoryTileCompare | SST_RAW_LFB_WRITE_CONTROL);

      //Read back lfbMemoryTileCtrl;
      GDBG_INFO(145, "Read back lfbMemoryTileCtrl\n");
      SET(sstio->lfbMemoryConfig, SST_RAW_LFB_UPDATE_CONTROL);
      data = GET(sstio->lfbMemoryConfig);
      DIAG_TESTREG32("lfbMemoryConfig (lfbMemoryTileCtrl)", 
		     lfbMemoryTileCtrl & (~LFB_MEMORY_CONFIG_MASK), 
		     data & (~LFB_MEMORY_CONFIG_MASK));
      
      //Read back lfbMemoryTileCompare;
      GDBG_INFO(145, "Read back lfbMemoryTileCompare\n");
      SET(sstio->lfbMemoryConfig, SST_RAW_LFB_UPDATE_CONTROL | SST_RAW_LFB_READ_CONTROL);
      data = GET(sstio->lfbMemoryConfig);
      DIAG_TESTREG32("lfbMemoryConfig (lfbMemoryTileCtrl)", 
		     lfbMemoryTileCompare & (~LFB_MEMORY_CONFIG_MASK), 
		     data & (~LFB_MEMORY_CONFIG_MASK));      
    }
}


void testLfbReads(void)
{
  SstRegs *sst[4];
  SstIORegs *sstio[4];
  FxU8 *rawLFB[4];
  CsimPrivate *cp[4];
  FxI32 i;

  for(i=0; i<4; i++)
    {
      sst[i] = diago.halInfo->boardInfo[i].virtAddr[0];
      sstio[i] = (SstIORegs *)SST_IO_ADDRESS(sst[i]);
      rawLFB[i] = (FxU8*)(SST_RAW_LFB_OFFSET + SST_BASE_ADDRESS(sst[i]));
      cp[i]  = CSIM_PRIVATE(diago.halInfo->boardInfo[i].sstCSIM);

      if(i==0)
	assert(cp[i]->environment.parentDevice);
      else
	assert(!cp[i]->environment.parentDevice);
    }

  //////////////////////////////////////////////////////////////
  //
  //Set up 4 way SLI, 1 sample per chip
  //
  //////////////////////////////////////////////////////////////
  configureChips(1);

  //Do linear reads
  setMemories32(0x1, 0, 0x12712708);
  setMemories32(0xE, 0, 0x61852470);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0], 0x12712708);
  testRead(FXTRUE, (FxU32)&rawLFB[1][0], 0x61852470);
  testRead(FXTRUE, (FxU32)&rawLFB[2][0], 0x61852470);
  testRead(FXTRUE, (FxU32)&rawLFB[3][0], 0x61852470);

  setMemories32(0x1, 0x3FFFFC, 0x12712676);
  setMemories32(0xE, 0x3FFFFC, 0x16728433);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x3FFFFC], 0x12712676);
  testRead(FXTRUE, (FxU32)&rawLFB[1][0x3FFFFC], 0x16728433);
  testRead(FXTRUE, (FxU32)&rawLFB[2][0x3FFFFC], 0x16728433);
  testRead(FXTRUE, (FxU32)&rawLFB[3][0x3FFFFC], 0x16728433);

  //Do tiled reads
  setMemories32(0x1, 0x400000, 0x56943781);
  setMemories32(0x2, 0x400000, 0x56943782);
  setMemories32(0x4, 0x400000, 0x56943783);
  setMemories32(0x8, 0x400000, 0x56943784);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x400000], 0x56943781);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x400800], 0x56943782);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x401000], 0x56943783);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x401800], 0x56943784);


  //Read from framebuffer (Make sure it doesn't average)
  setMemories32(0x1, 0x43CC00, 0x62713432);
  setMemories32(0x2, 0x43CC00, 0x32546789);
  setMemories32(0x4, 0x43CC00, 0x61784523);
  setMemories32(0x8, 0x43CC00, 0x61299321);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x4F0000], 0x62713432);
    
  
  //Make sure tileCompare works
  setMemories32(0xF, 0x500000, 0x51433678);
  testRead(FXFALSE, (FxU32)&rawLFB[0][0x500000], 0x51433678);
  testRead(FXFALSE, (FxU32)&rawLFB[1][0x500000], 0x51433678);
  testRead(FXFALSE, (FxU32)&rawLFB[2][0x500000], 0x51433678);
  testRead(FXFALSE, (FxU32)&rawLFB[3][0x500000], 0x51433678);

  //Enable and move the tileCompare line to 5.5MB
  SET(sstio[0]->lfbMemoryConfig, SST_RAW_LFB_WRITE_CONTROL | (0x580000 / SST_TILE_SIZE) |
      LFB_MEMORY_TILE_COMPARE_USE_TILE_COMPARE);   

  testRead(FXTRUE, (FxU32)&rawLFB[0][0x500000], 0x51433678);
  testRead(FXTRUE, (FxU32)&rawLFB[1][0x500000], 0x51433678);
  testRead(FXTRUE, (FxU32)&rawLFB[2][0x500000], 0x51433678);
  testRead(FXTRUE, (FxU32)&rawLFB[3][0x500000], 0x51433678);



  //////////////////////////////////////////////////////////////
  //
  //Set up 4 way SLI, 2 samples per chip
  //
  //////////////////////////////////////////////////////////////
  configureChips(2);

  //Do linear reads
  setMemories32(0x1, 0, 0x54316784);
  setMemories32(0xE, 0, 0x12673833);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0], 0x54316784);
  testRead(FXTRUE, (FxU32)&rawLFB[1][0], 0x12673833);
  testRead(FXTRUE, (FxU32)&rawLFB[2][0], 0x12673833);
  testRead(FXTRUE, (FxU32)&rawLFB[3][0], 0x12673833);

  setMemories32(0x1, 0x3FFFFC, 0x46327132);
  setMemories32(0xE, 0x3FFFFC, 0x64721323);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x3FFFFC], 0x46327132);
  testRead(FXTRUE, (FxU32)&rawLFB[1][0x3FFFFC], 0x64721323);
  testRead(FXTRUE, (FxU32)&rawLFB[2][0x3FFFFC], 0x64721323);
  testRead(FXTRUE, (FxU32)&rawLFB[3][0x3FFFFC], 0x64721323);

  //Do tiled reads
  setMemories32(0x1, 0x400000, 0x01020304);
  setMemories32(0x1, 0x100000, 0x05040302);
  setMemories32(0x2, 0x400000, 0x03030303);
  setMemories32(0x2, 0x100000, 0x05050505);
  setMemories32(0x4, 0x400000, 0x03030303);
  setMemories32(0x4, 0x100000, 0x07070707);
  setMemories32(0x8, 0x400000, 0x03030303);
  setMemories32(0x8, 0x100000, 0x09090909);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x400000], 0x03030303);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x400800], 0x04040404);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x401000], 0x05050505);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x401800], 0x06060606);

  //Read from framebuffer (Make sure it doesn't average)
  setMemories32(0x1, 0x43CC00, 0x21734203);
  setMemories32(0x2, 0x43CC00, 0x62154793);
  setMemories32(0x4, 0x43CC00, 0x15384673);
  setMemories32(0x8, 0x43CC00, 0x65743184);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x4F0000], 0x21734203);

  //Make sure tileCompare works
  setMemories32(0xF, 0x500000, 0x18293743);
  testRead(FXFALSE, (FxU32)&rawLFB[0][0x500000], 0x18293743);
  testRead(FXFALSE, (FxU32)&rawLFB[1][0x500000], 0x18293743);
  testRead(FXFALSE, (FxU32)&rawLFB[2][0x500000], 0x18293743);
  testRead(FXFALSE, (FxU32)&rawLFB[3][0x500000], 0x18293743);

  //Enable and move the tileCompare line to 5.5MB
  SET(sstio[0]->lfbMemoryConfig, SST_RAW_LFB_WRITE_CONTROL | (0x580000 / SST_TILE_SIZE) |
      LFB_MEMORY_TILE_COMPARE_USE_TILE_COMPARE);   

  testRead(FXTRUE, (FxU32)&rawLFB[0][0x500000], 0x18293743);
  testRead(FXTRUE, (FxU32)&rawLFB[1][0x500000], 0x18293743);
  testRead(FXTRUE, (FxU32)&rawLFB[2][0x500000], 0x18293743);
  testRead(FXTRUE, (FxU32)&rawLFB[3][0x500000], 0x18293743);


  //////////////////////////////////////////////////////////////
  //
  //Set up 4 way SLI, 2 samples per chip
  //
  //////////////////////////////////////////////////////////////
  configureChips(4);

  //Do linear reads
  setMemories32(0x1, 0, 0xa79fe9af);
  setMemories32(0xE, 0, 0xbd7b7dde);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0], 0xa79fe9af);
  testRead(FXTRUE, (FxU32)&rawLFB[1][0], 0xbd7b7dde);
  testRead(FXTRUE, (FxU32)&rawLFB[2][0], 0xbd7b7dde);
  testRead(FXTRUE, (FxU32)&rawLFB[3][0], 0xbd7b7dde);

  setMemories32(0x1, 0x3FFFFC, 0xa8a8a8a8);
  setMemories32(0xE, 0x3FFFFC, 0xbcbcbcbc);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x3FFFFC], 0xa8a8a8a8);
  testRead(FXTRUE, (FxU32)&rawLFB[1][0x3FFFFC], 0xbcbcbcbc);
  testRead(FXTRUE, (FxU32)&rawLFB[2][0x3FFFFC], 0xbcbcbcbc);
  testRead(FXTRUE, (FxU32)&rawLFB[3][0x3FFFFC], 0xbcbcbcbc);
  
  //Do tiled reads
  setMemories32(0x1, 0x400000, 0x01010101);
  setMemories32(0x1, 0x100000, 0x02020202);
  setMemories32(0x2, 0x400000, 0x03030303);
  setMemories32(0x2, 0x100000, 0x06060606);
  setMemories32(0x4, 0x400000, 0x03030300);
  setMemories32(0x4, 0x100000, 0x04040400);
  setMemories32(0x8, 0x400000, 0x05050500);
  setMemories32(0x8, 0x100000, 0x08080800);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x400000], 0x03030303);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x400800], 0x05050500);
  
  //Read from framebuffer (Make sure it doesn't average)
  setMemories32(0x1, 0x48C800, 0x43272134);
  setMemories32(0x2, 0x48C800, 0x43812432);
  setMemories32(0x4, 0x48C800, 0xaf7fa7f9);
  setMemories32(0x8, 0x48C800, 0xc8c8c80e);
  testRead(FXTRUE, (FxU32)&rawLFB[0][0x4F0000], 0x43272134);

  //Make sure tileCompare works
  setMemories32(0xF, 0x500000, 0x412389ae);
  testRead(FXFALSE, (FxU32)&rawLFB[0][0x500000], 0x412389ae);
  testRead(FXFALSE, (FxU32)&rawLFB[1][0x500000], 0x412389ae);
  testRead(FXFALSE, (FxU32)&rawLFB[2][0x500000], 0x412389ae);
  testRead(FXFALSE, (FxU32)&rawLFB[3][0x500000], 0x412389ae);

  //Enable and move the tileCompare line to 5.5MB
  SET(sstio[0]->lfbMemoryConfig, SST_RAW_LFB_WRITE_CONTROL | (0x580000 / SST_TILE_SIZE) |
      LFB_MEMORY_TILE_COMPARE_USE_TILE_COMPARE);   

  testRead(FXTRUE, (FxU32)&rawLFB[0][0x500000], 0x412389ae);
  testRead(FXTRUE, (FxU32)&rawLFB[1][0x500000], 0x412389ae); 
  testRead(FXTRUE, (FxU32)&rawLFB[2][0x500000], 0x412389ae); 
  testRead(FXTRUE, (FxU32)&rawLFB[3][0x500000], 0x412389ae); 
}


//This tests multi-chip lfb writes
void testLfbWrites(void)
{
  SstRegs *parentSST;
  SstIORegs *parentSSTIO;
  FxU8 *parentRawLFB;
  CsimPrivate *parentCP;
  

  parentSST = diago.halInfo->boardInfo[0].virtAddr[0];
  parentSSTIO = (SstIORegs *)SST_IO_ADDRESS(parentSST);
  parentRawLFB = (FxU8*)(SST_RAW_LFB_OFFSET + SST_BASE_ADDRESS(parentSST));
  parentCP  = CSIM_PRIVATE(diago.halInfo->boardInfo[0].sstCSIM);
  assert(parentCP->environment.parentDevice);

  //////////////////////////////////////////////////////////////
  //
  //Set up 4 way SLI, 1 sample per chip
  //
  //////////////////////////////////////////////////////////////
  configureChips(1);

  //Do some linear writes
  SET(parentRawLFB[0], 0xbdefaced);
  checkMemories32(FXTRUE, 0xF, 0, 0xbdefaced);

  SET(parentRawLFB[0x100000], 0x12345678);
  checkMemories32(FXTRUE, 0xF, 0x100000, 0x12345678);

  SET(parentRawLFB[0x3FFFFC], 0x18766644);
  checkMemories32(FXTRUE, 0xF, 0x3FFFFC, 0x18766644);

  //Do some tiled writes
  SET(parentRawLFB[0x400000], 0xabcd0123);
  checkMemories32(FXTRUE, 0x1, 0x400000, 0xabcd0123);
  checkMemories32(FXTRUE, 0xE, 0x400000, 0x00000000);

  SET(parentRawLFB[0x400800], 0x0fedface);
  checkMemories32(FXTRUE, 0x2, 0x400000, 0x0fedface);
  checkMemories32(FXTRUE, 0xC, 0x400000, 0x00000000);

  SET(parentRawLFB[0x401000], 0x09871234);
  checkMemories32(FXTRUE, 0x4, 0x400000, 0x09871234);
  checkMemories32(FXTRUE, 0x8, 0x400000, 0x00000000);

  SET(parentRawLFB[0x401800], 0x54326789);
  checkMemories32(FXTRUE, 0x8, 0x400000, 0x54326789);

  //Enable and move the tileCompare line to 5.5MB
  SET(parentSSTIO->lfbMemoryConfig, SST_RAW_LFB_WRITE_CONTROL | (0x580000 / SST_TILE_SIZE) |
      LFB_MEMORY_TILE_COMPARE_USE_TILE_COMPARE);   

  SET(parentRawLFB[0x500000], 0x1a2b3c4d);
  checkMemories32(FXTRUE, 0xF, 0x500000, 0x1a2b3c4d);




  //////////////////////////////////////////////////////////////
  //
  //Set up 4 way SLI, 2 samples per chip
  //
  //////////////////////////////////////////////////////////////
  configureChips(2);

  //Do some linear writes
  SET(parentRawLFB[0], 0xa6cb8712);
  checkMemories32(FXTRUE, 0xF, 0, 0xa6cb8712);

  SET(parentRawLFB[0x100000], 0x71292612);
  checkMemories32(FXTRUE, 0xF, 0x100000, 0x71292612);

  SET(parentRawLFB[0x3FFFFC], 0xfe7a7163);
  checkMemories32(FXTRUE, 0xF, 0x3FFFFC, 0xfe7a7163);


  //Do tiled writes
  SET(parentRawLFB[0x400000], 0x23146612);   //Write 1
  SET(parentRawLFB[0x400800], 0x12346718);   //Write 2
  SET(parentRawLFB[0x401000], 0xab6d7ab6);   //Write 3
  SET(parentRawLFB[0x401800], 0xaf453b2e);   //Write 4

  //Check write 1
  checkMemories32(FXTRUE, 0x1, 0x400000, 0x23146612);
  checkMemories32(FXTRUE, 0x1, 0x100000, 0x23146612);  

  //Check write 2
  checkMemories32(FXTRUE, 0x2, 0x400000, 0x12346718);
  checkMemories32(FXTRUE, 0x2, 0x100000, 0x12346718);

  //Check write 3
  checkMemories32(FXTRUE, 0x4, 0x400000, 0xab6d7ab6);
  checkMemories32(FXTRUE, 0x4, 0x100000, 0xab6d7ab6);

  //Check write 4
  checkMemories32(FXTRUE, 0x8, 0x400000, 0xaf453b2e);
  checkMemories32(FXTRUE, 0x8, 0x100000, 0xaf453b2e);


  //Enable and move the tileCompare line to 5.5MB
  SET(parentSSTIO->lfbMemoryConfig, SST_RAW_LFB_WRITE_CONTROL | (0x580000 / SST_TILE_SIZE) |
      LFB_MEMORY_TILE_COMPARE_USE_TILE_COMPARE);   

  SET(parentRawLFB[0x500000], 0x14231272);
  checkMemories32(FXTRUE, 0xF, 0x500000, 0x14231272);
  checkMemories32(FXFALSE, 0xF, 0x100000, 0x14231272);




  //////////////////////////////////////////////////////////////
  //
  //Set up 4 way SLI, 2 samples per chip
  //
  //////////////////////////////////////////////////////////////
  configureChips(4);

  //Do some linear writes
  SET(parentRawLFB[0], 0x26116246);
  checkMemories32(FXTRUE, 0xF, 0, 0x26116246);

  SET(parentRawLFB[0x100000], 0xaf7e7afe);
  checkMemories32(FXTRUE, 0xF, 0x100000, 0xaf7e7afe);

  SET(parentRawLFB[0x3FFFFC], 0xa68fabe7);
  checkMemories32(FXTRUE, 0xF, 0x3FFFFC, 0xa68fabe7);

  //Do Tiled writes
  SET(parentRawLFB[0x400000], 0xaf67e7a7);   //Write 1
  SET(parentRawLFB[0x400800], 0x23146612);   //Write 2

  //Check write 1
  checkMemories32(FXTRUE, 0x3, 0x400000, 0xaf67e7a7);
  checkMemories32(FXTRUE, 0x3, 0x100000, 0xaf67e7a7);

  //Check write 2
  checkMemories32(FXTRUE, 0xC, 0x400000, 0x23146612);
  checkMemories32(FXTRUE, 0xC, 0x100000, 0x23146612);


  //Enable and move the tileCompare line to 5.5MB
  SET(parentSSTIO->lfbMemoryConfig, SST_RAW_LFB_WRITE_CONTROL | (0x580000 / SST_TILE_SIZE) |
      LFB_MEMORY_TILE_COMPARE_USE_TILE_COMPARE);   

  SET(parentRawLFB[0x500000], 0x6fed6fed);
  checkMemories32(FXTRUE, 0xF, 0x500000, 0x6fed6fed);
  checkMemories32(FXFALSE, 0xF, 0x100000, 0x6fed6fed);


}


FxBool testRead(FxBool shouldPass, FxU32 address, FxU32 data)
{
  FxU32 memoryValue;
  
  assert((address & 3) == 0);
        
  memoryValue = halLoad32((void *)address);

  #ifdef ENDB
  memoryValue = 
    (((memoryValue >> 24) & 0xFF) <<  0) |
    (((memoryValue >> 16) & 0xFF) <<  8) |
    (((memoryValue >>  8) & 0xFF) << 16) |
    (((memoryValue >>  0) & 0xFF) << 24);
  #endif

  if(shouldPass && memoryValue != data)
    {
      GDBG_ERROR("checkMemories32", "Expected GET(0x%08x) = 0x%08x, got 0x%08x\n",
		 address, data, memoryValue);
      return(FXFALSE);
    }
  else if((!shouldPass) && memoryValue == data)
    {
      GDBG_ERROR("checkMemories32", "Expected GET(0x%08x) != 0x%08x, got 0x%08x\n",
		 address, data, memoryValue);
      return(FXFALSE);
    }
  
  return(FXTRUE);
}
