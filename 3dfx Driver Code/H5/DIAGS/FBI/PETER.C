#include <assert.h>

#include "udiag.h"
#include "sstdiag.h"


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
//
//       This is a diag that runs Peter's AGP self test
//
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


/*

AGP test mode documentation


Napalm is equipped with a special AGP test mode.  This mode will enable the testing of AGP bus transfers at both 2
X and 4X speeds without requiring functionality from any part of Napalm other than the pci_core in a system enviro
nment. A detailed description of the operations of this special AGP test mode is contained in this document.

1.Test program will set up in memory 2 Qwords of test data.
2.Test program will then do a PCI config write to the agp_test_ctrl register at address 0x98:
agp_test_ctrl[31:3] = starting address of the test data in memory from step 1.
agp_test_ctrl[2:1] = reserved
agp_test_ctrl[0] = set to 1 to start test
         
        i.e. If the test data starts at address  0x12340000, then the agp_test_ctrl register should be written wit
h
              0x12340001.
3.Napalm will wait until the AGP is idle and there are no outstanding transactions and then start the AGP self-tes
t.
4.At the start of the AGP self-test, Napalm will issue an AGP read request to the address specified in agp_test_ct
rl[31:3] to read the 2 Qwords of test data.
5.AGP self-test is done when the requested data is returned by the Northbridge. At this point the 2 Qwords of test
 data will be placed into the 4 agp_test_data registers and agp_test_ctrl[0] is set back to 0.
6.The test program should read the agp_test_ctrl register periodically and will know that the AGP self test is com
plete when a 0 is read back in agp_test_ctrl[0].
7.The test program can now read the agp_test_data registers at addresses 0xa8, 0xa4, 0xa0, and 0x9c, with the foll
owing contents:
0xa8: Upper 32 bits of Qword #2
0xa4: Lower 32 bits of Qword #2
0xa0: Upper 32 bits of Qword #1
0x9c: Lower 32 bits of Qword #1
8.A simple comparison between the contents of the agp_test_data registers and the original 2 Qwords used as test d
ata will reveal whether the AGP section of Napalm is working.

All addresses mentioned are in the PCI config space
*/


const PciRegister AGP_TEST_CTRL   = {0x98, 4, READ_WRITE};
const PciRegister AGP_TEST_WORD_0 = {0xa4, 4, READ_WRITE};
const PciRegister AGP_TEST_WORD_1 = {0xa8, 4, READ_WRITE};
const PciRegister AGP_TEST_WORD_2 = {0x9c, 4, READ_WRITE};
const PciRegister AGP_TEST_WORD_3 = {0xa0, 4, READ_WRITE};


FxU32 configureAGPMemory(SstRegs *sst);

int main(int argc, char **argv)
{
  SstRegs *sst;
  SstIORegs *sstio;
  CsimPrivate *cpriv;
  FxDeviceInfo *info;

  FxU32 agpMemory;
  FxU32 words[4], returnedWords[4];

  FxU32 agp_test_ctrl;
  int i;

	
  sst = SST_BEGIN(argc, argv);
  sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
  cpriv = CSIM_PRIVATE(diago.sstCSIM);

  info = &diago.halInfo->boardInfo[0];
  assert(info != NULL);



  //Set up the agp memory used for test
  agpMemory = configureAGPMemory(sst);
  
  while(DIAG_STARTPASS())
    {
      //Pick some random shizit to write to memory
      words[0] = iRandom(~0);
      words[1] = iRandom(~0);
      words[2] = iRandom(~0);
      words[3] = iRandom(~0);

      GDBG_INFO(0, "0x08%x: 0x08%x\n", agpMemory +  0, words[0]);
      GDBG_INFO(0, "0x08%x: 0x08%x\n", agpMemory +  4, words[1]);
      GDBG_INFO(0, "0x08%x: 0x08%x\n", agpMemory +  8, words[2]);
      GDBG_INFO(0, "0x08%x: 0x08%x\n", agpMemory + 12, words[3]);

      //Step 1
      //Write to agp memory
      agpWriteMem32((FxU32 *)(agpMemory +  0), words[0]);
      agpWriteMem32((FxU32 *)(agpMemory +  4), words[1]);
      agpWriteMem32((FxU32 *)(agpMemory +  8), words[2]);
      agpWriteMem32((FxU32 *)(agpMemory + 12), words[3]);

      //Step 2
      //Start test
      assert(!(agpMemory & 0xf));  //Make sure memory is aligned on 8 bytes boundaries
      agp_test_ctrl = (agpMemory & (~0xF)) | 1;
      pciSetConfigData(AGP_TEST_CTRL, info->deviceNumber, &agp_test_ctrl);

      //Step 3

      //Step 4

      //Step 5

      //Step 6
      //Wait until the test is complete
      while(agp_test_ctrl & 1)
	{
	  //It would probably be a good idea to sleep here
	  pciGetConfigData(AGP_TEST_CTRL, info->deviceNumber, &agp_test_ctrl);
	}
      
      //Step 7
      //Read back result from pci config space
      pciGetConfigData(AGP_TEST_WORD_0, info->deviceNumber, &returnedWords[0]);
      pciGetConfigData(AGP_TEST_WORD_1, info->deviceNumber, &returnedWords[1]);
      pciGetConfigData(AGP_TEST_WORD_2, info->deviceNumber, &returnedWords[2]);
      pciGetConfigData(AGP_TEST_WORD_3, info->deviceNumber, &returnedWords[3]);

      //Step 8
      //Compare results
      for(i=0; i<4; i++)
	if(words[i] != returnedWords[i])
	  GDBG_ERROR("peter::main", "Expecting word %d to be 0x%08x, but got 0x%08x\n",
		     i, words[i], returnedWords[i]);

    }
  
  DIAG_PASS(0);

  return(0);
}

//Allocate a little bit of AGP memory to work with
FxU32 configureAGPMemory(SstRegs *sst)
{
  FxI32 agpMemorySize;
  FxU32 agpMemory;

  agpMemInit();
  
  agpMemorySize=4096;

  agpMemory = (FxU32)AGPMEMALLOC(sst, agpMemorySize);
  GDBG_INFO(0, "agpMemory = 0x%x\n", agpMemory);

  if(agpMemory == 0)
    {
      GDBG_ERROR("configureAGPMemory", "Can't allocate agp memory! %s(%d)\n",
		 __FILE__, __LINE__);
      DIAG_FAIL();
    }

  return(agpMemory);
}



