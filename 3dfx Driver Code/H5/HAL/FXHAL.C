/* -*-c++-*- */

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
** $Date: 10/11/00 8:31:00 PM$
*/

#include <stdlib.h>


#include <h3.h>
#ifdef WINSIM
#include "h3sim.h"
#include "fxhal.h"
#else
#include "../csim/h3sim.h"
#include <fxpci.h>
#include <fxagp.h>
#endif

#include "init.h"
#ifdef HAL_HW
#include <h3cinit.h>
#endif /* #ifdef HAL_HW */

// Allow gwhat to find info
#define VERSIONSTR    "HalCode " "$Revision: 4$" "\0"
static char codeIdent[] = "@#% " VERSIONSTR ;

HalInfo halInfo = { 1, 0, 0, 0 };       // Csim, Hsim, HW, #boards

// get and translate a simple numeric environment variable
static int genv(const char *name, const int defaultVal, const char *msg)
{
    int level=3, val=defaultVal;

    if (GETENV(name)) {
        SSCANF(GETENV(name), "%i", &val);
        level = 0;
    }
    if (val) GDBG_INFO(level,msg,val);
    return val;
}

// translate a virtual address to a boardNumber index
FxBool fxHalVaddrToBoardNumber( SstRegs *sst, FxU32 *boardNumber )
{
    FxU32 n;

    if (sst == NULL)
        return FXFALSE;
    for(n=0; n<halInfo.boardsFound; n++) {
        if (halInfo.boardInfo[n].virtAddr[0] == sst) {
            *boardNumber = n;
            return FXTRUE;
        }
    }
    return FXFALSE;
}

// reset a board info structure to its unitialized state
void fxHalResetBoardInfo( FxDeviceInfo *info )
{
    int j;

    info->size = sizeof(FxDeviceInfo);
    info->virtAddr[0] = NULL;
    info->virtAddr[1] = NULL;
    info->physAddr[0] = 0;
    info->physAddr[1] = 0;
    info->virtPort = 0;
    info->physPort = 0;
    info->deviceNumber = DEAD;
    info->pciBusNumber = DEAD;
    info->pciDeviceNumber = DEAD;
    info->pciFunctionNumber = DEAD;
    info->vendorID = DEAD;
    info->deviceID = DEAD;
    info->fbiRevision = DEAD;
    info->fbiConfig = DEAD;
    info->fbiMemType = DEAD;
    info->fbiVideoWidth = DEAD;
    info->fbiVideoHeight = DEAD;
    info->fbiVideoRefresh = DEAD;
    info->fbiMemoryFifoEn = 0;
    info->tmuRevision = DEAD;
    info->numberTmus = 0;               // allows for CSIM bootup
    info->tmuConfig = DEAD;
    info->fbiMemSize = DEAD;
    for(j=0; j<MAX_NUM_TMUS; j++) {
        info->tmuMemSize[j] = DEAD;
        info->tmuInit0[j] = DEAD;
        info->tmuInit1[j] = DEAD;
    }
    info->initGrxClkDone = 0;

    info->agpMem = (FxU8 *)DEAD;               // AGP memory
    info->agpSizeInBytes = 0;
    info->agpBaseAddrH = DEAD;
    info->agpBaseAddrL = DEAD;
    info->agpRqDepth = DEAD;

}

//---------------------------------------------------------------------------
// The first initialization routine
//---------------------------------------------------------------------------
HalInfo * FX_CSTYLE
fxHalInit( FxU32 flags )
{
    char *buf;
    int i;
    static FxBool halInitialized;

#if !defined(FX_DLL_ENABLE)
    GDBG_INIT();        // if static library, call this here instead of DllMain
#endif

    if (halInitialized)
        return &halInfo;

    for(i = 0; i < HAL_MAX_BOARDS; i++) {
        fxHalResetBoardInfo( &halInfo.boardInfo[i] );
    }
    halInitialized = 1;

    /* Make Watcom happy */
    codeIdent[0];
    GDBG_INFO(1,"fxHalInit(0x%x) %s (Headers %s)\n",
                flags, VERSIONSTR, HAL_H_REV);
    halInfo.video = 1;
    halInfo.pollLimit = 3000;
#ifdef GDBG_INFO_ON
        halInfo.pollLimit /= 2;
#endif
    halInfo.pollCount = 0;
#ifndef KERNEL
    if (buf = GETENV("HAL_VIDEO")) {
        halInfo.video = atoi(buf);

        gdbg_info(0,"HAL_VIDEO = %d\n",halInfo.video);
    }
    if (buf = GETENV("HAL_POLL")) {
        halInfo.pollLimit = atoi(buf);
        gdbg_info(0,"HAL_POLL = %d\n",halInfo.pollLimit);
    }
  #ifdef HAL_HW
    if (buf = GETENV("HAL_HW"))
        halInfo.hw = atoi(buf);
    else
        halInfo.hw = 0;
  #else
    halInfo.hw = 0;             // if real HW code is disabled turn off real HW
  #endif
    if (buf = GETENV("HAL_HSIM"))
        halInfo.hsim = atoi(buf);
  #ifdef HAL_CSIM
    if (halInfo.hw == 0 || halInfo.hsim)
        halInfo.csim = 1;               // enable C simulator if no HW or HSIM
    if (buf = GETENV("HAL_CSIM"))
        halInfo.csim = atoi(buf);
    if (buf = GETENV("HAL_CSIMIO"))
        halInfo.csimio = atoi(buf);
    else
        halInfo.csimio = 1;             // enable mirroring of HSIM/HW register accesses
  #endif

#else
    buf = 0;
    halInfo.csimio = 1;      // enable mirroring of HSIM/HW register accesses
#endif /* #ifndef KERNEL */

    if (halInfo.hw && halInfo.hsim)
        GDBG_ERROR("fxHalInit", "HW and HSIM at the same time\n");
    return &halInfo;

} /* fxHalInit */

#ifndef KERNEL
// a DLL's environment is different from the main program so we have
// to put stuff into the environment from within the DLL
FX_EXPORT void FX_CSTYLE
fxHalPutenv( char * buf )
{
    char *buf1 = malloc(strlen(buf)+1);
    strcpy(buf1,buf);   // unix requires a unique memory location
    putenv(buf1);       // and so we don't free it
}

static PciHwcCallbacks halIoCallbacks = {
        0,
        &halInPort8,
        &halInPort16,
        &halInPort32,
        &halOutPort8,
        &halOutPort16,
        &halOutPort32,
};

//---------------------------------------------------------------------------
//  Returns the number of 3Dfx boards in the system
//---------------------------------------------------------------------------
FX_EXPORT FxU32 FX_CSTYLE fxHalNumBoardsInSystem(void)
{
    FxU32 vendorID = _3DFX_PCI_ID;     /* 3Dfx Vendor ID */
    FxU32 deviceID = 0xFFFF;           /* Find any 3Dfx board */
    FxU32 j, n;
    FxU32 function;

    if (halInfo.boardsFound) {
        GDBG_INFO(1,"fxHalNumBoardsInSystem() => %d\n",halInfo.boardsFound);
        return(halInfo.boardsFound);
    }

    GDBG_INFO(1,"fxHalNumBoardsInSystem()\n");
    fxHalInit(0);                       // just to be safe
    if(GETENV("SST_BOARDS"))
        halInfo.boardsFound = ATOI(GETENV("SST_BOARDS"));
    else {
        halInfo.boardsFound = 0;
        GDBG_INFO(2,"  calling pciOpenEx() for bus scan\n");
        halIoCallbacks.doHW = halInfo.hw;
        if (pciOpenEx(&halIoCallbacks) == FXFALSE)
            GDBG_ERROR("fxHalNumBoardsInSystem", "pciOpenEx() failed: %s\n",pciGetErrorString());

        for(j=0; j<HAL_MAX_BOARDS; j++) {
	  if(pciFindCardMultiFunc(vendorID, deviceID, &n, &function, j))
	    halInfo.boardsFound++;
        }
    }
    GDBG_INFO(2,"fxHalNumBoardsInSystem() => %d\n",halInfo.boardsFound);
    return(halInfo.boardsFound);
}
#endif

//---------------------------------------------------------------------------
// The initialization dispatcher:
// figure out what environments are enabled and intialize all of them
//---------------------------------------------------------------------------
SstRegs * FX_CSTYLE
fxHalMapBoard( FxU32 bn )
{
    GDBG_INFO(1,"fxHalMapBoard(%d)\n",bn);

    fxHalInit(0);                       // just to be safe
#ifdef HAL_HSIM
    // if HSIM code is enabled then initialize the VCS simulator
    // which can never run without CSIM along side it
    if (halInfo.hsim) {
      static FxBool neverSetup = FXTRUE;

      if(neverSetup)
	{
	  GDBG_INFO(3,"    setting up hardware simulation\n");
	  SST_HW_SIMSETUP();                      // do HSIM simulator setup
	  HSIM_CHIP_SETUP();                      // do HSIM chip setup

	  neverSetup = FXFALSE;
	}
    }
#endif

#ifndef KERNEL
    GDBG_INFO(2,"  calling pciOpenEx() for bus scan\n");
    halIoCallbacks.doHW = halInfo.hw;
    if (pciOpenEx(&halIoCallbacks) == FXFALSE)
        GDBG_ERROR("fxHalMapBoard", "pciOpenEx() failed: %s\n",pciGetErrorString());
    if (halInfo.boardsFound == 0)       // the 1st time, scan the bus and find
        fxHalNumBoardsInSystem();       // all the boards
#else /* #ifndef KERNEL */
#ifdef HAL_CSIM
    if (halInfo.csim != -1)
        halInfo.csim = 1;           // enable C simulator, but don't turn off
                                    // driver mode if it's already on (== -1)
#ifdef WINSIM
    halInfo.csim = -1;              // Enable CMDFIFO simulator mode
#endif /* WINSIM */
    halInfo.hw = 0;             // if real HW code is disabled turn off real HW
#endif /* #ifdef HAL_CSIM */
#endif /* ifndef KERNEL */

    // do some sanity checks first
#ifdef WINNT
    if(bn > halInfo.boardsFound)
#else
    if(bn >= halInfo.boardsFound)
#endif
      {
	GDBG_INFO(0, "Oh shit! Invalid boardNumber %d passed to fxHalMapBoard %s(%d)\n",
		  bn, __FILE__, __LINE__);
	return(NULL);
      }
    if (halInfo.hw && halInfo.hsim)
        GDBG_ERROR("fxHalMapBoard", "HW and HSIM at the same time\n");

#ifdef HAL_HW
    // if HW code is enabled then initialize the real HW
    if (halInfo.hw) {
      // HACK in some relavent information:
      FxU32 deviceID;

        // Find and map baseAddress0 into virtual memory,
      for(deviceID = 0x4; deviceID < 0x10; deviceID++)
	{
	  halInfo.boardInfo[bn].physAddr[0] = (FxU32)
	    pciMapCardMulti(_3DFX_PCI_ID,
			    deviceID,
			    0x8000000,
			    &halInfo.boardInfo[bn].deviceNumber,
			    bn, 0);

	  if(halInfo.boardInfo[bn].physAddr[0] != 0)
	    break;
	}

        if (halInfo.boardInfo[bn].sstHW = (SstRegs *)halInfo.boardInfo[bn].physAddr[0])
            GDBG_INFO(2,"    HW   enabled: 0x%x\n",
                      halInfo.boardInfo[bn].physAddr[0]);

        // Find and map baseAddress1 into virtual memory,
      for(deviceID = 0x4; deviceID < 0x10; deviceID++)
	{	  
	  halInfo.boardInfo[bn].physAddr[1] = (FxU32)
	    pciMapCardMulti(_3DFX_PCI_ID,
			    deviceID,
			    0x8000000,
			    &halInfo.boardInfo[bn].deviceNumber,
			    bn, 1);
	  
	  if(halInfo.boardInfo[bn].physAddr[1] != 0)
	    break;
	}


#ifdef HAL_CSIM
        // when compiled with HAL_CSIM always send data thru halStore/Load funnels
        halInfo.boardInfo[bn].virtAddr[0] = SST_FAKE_ADDRESS_MAKE(bn);
#endif
	if ( ! fxHalInitUSWC(halInfo.boardInfo[bn].virtAddr[0]) ) {
	  GDBG_ERROR("fxHalMapBoard","unable to turn on WriteCombine for board %d\n",bn);
	}
	
    }
#endif

#ifdef HAL_HSIM
    if (halInfo.hsim) {
        halInfo.csim = 1;               // enable C simulator if no HW or HSIM
# ifndef KERNEL
        if (pciFindCardMultiFunc(_3DFX_PCI_ID, 0xFFFF, &halInfo.boardInfo[bn].deviceNumber,
				 &halInfo.boardInfo[bn].pciFunctionNumber, bn)) {
            // NOTE: returned value for SST is always the CSIM value
            halInfo.boardInfo[bn].virtAddr[0] = SST_FAKE_ADDRESS_MAKE(bn);
            GDBG_INFO(2,"    HSIM enabled: 0x%x (CSIM fake address)\n",halInfo.boardInfo[bn].virtAddr[0]);
        }
# endif
    }
#endif

#ifdef HAL_CSIM
    // if CSIM code is enabled then initialize the simulator
    if (halInfo.csim) {
#ifdef KERNEL
        // hard code all these
        halInfo.boardInfo[bn].virtAddr[0] = SST_FAKE_ADDRESS_MAKE(bn);
        halInfo.boardInfo[bn].physAddr[0] = 0x0000000;
        halInfo.boardInfo[bn].physAddr[1] = 0x2000000;
        halInfo.boardInfo[bn].virtPort = SST_FAKE_PORT_MAKE(bn);
        halInfo.boardInfo[bn].physPort = 0x00;
#ifndef WINSIM
        halInfo.boardsFound++;
#endif // !WINSIM
#else
        if (halInfo.boardInfo[bn].virtAddr[0] == NULL) {
	  if(pciFindCardMultiFunc(_3DFX_PCI_ID, 0xFFFF, &halInfo.boardInfo[bn].deviceNumber,
				  &halInfo.boardInfo[bn].pciFunctionNumber, bn))
	    halInfo.boardInfo[bn].virtAddr[0] = SST_FAKE_ADDRESS_MAKE(bn);
        }
#endif

        if (halInfo.boardInfo[bn].virtAddr[0]) {
	  //From now on, the diags(in sstdiags.c) or whatever is using the csim will need to
	  //initialize the CSIM itself.
#ifdef WINNT
	  halInfo.boardInfo[bn].sstCSIM = csimInit( bn );
#else
	  //halInfo.boardInfo[bn].sstCSIM = csimInit( bn );
#endif
            GDBG_INFO(2,"    CSIM enabled: 0x%x\n",halInfo.boardInfo[bn].virtAddr[0]);

	    //Only open a window for the parent Napalm
	    if(bn == 0)
	      if (guiOpen( bn ) == FXFALSE)       // open up a CSIM main control window
                halInfo.video = 0;
        }
    }
#endif

    // do some sanity checking
    if (halInfo.boardInfo[bn].virtAddr[0] == NULL) {
        GDBG_ERROR("fxHalMapBoard","HAL could not find board number %d\n",bn);
        exit(1);
    }

#ifdef KERNEL
        halInfo.boardInfo[bn].vendorID = _3DFX_PCI_ID;
        halInfo.boardInfo[bn].deviceID = CSIM_DEFAULT_DEVICE_ID;
        halInfo.boardInfo[bn].fbiRevision = CSIM_DEFAULT_FBI_REV;
        halInfo.boardInfo[bn].deviceNumber = 0;
#else
{
    FxU32 cmd;
    FxU32 dn = halInfo.boardInfo[bn].deviceNumber;

    PCICFG_RD(PCI_VENDOR_ID, dn, halInfo.boardInfo[bn].vendorID);
    PCICFG_RD(PCI_DEVICE_ID, dn, halInfo.boardInfo[bn].deviceID);
    PCICFG_RD(PCI_REVISION_ID, dn, halInfo.boardInfo[bn].fbiRevision);
#ifdef HAL_HSIM  // psmith hack
    PCICFG_RD(PCI_BASE_ADDRESS_0, dn, halInfo.boardInfo[bn].physAddr[0]);
    halInfo.boardInfo[bn].physAddr[0] &= 0xFFFFFFF0;
    PCICFG_RD(PCI_BASE_ADDRESS_1, dn, halInfo.boardInfo[bn].physAddr[1]);
    halInfo.boardInfo[bn].physAddr[1] &= 0xFFFFFFF0;
    {
      FxU32 data;
      PCICFG_RD(PCI_IO_BASE_ADDRESS, dn, data);
      halInfo.boardInfo[bn].physPort = (FxU16)(data & 0xFFFFFF00);
    }
#endif
#ifdef HAL_CSIM
    halInfo.boardInfo[bn].virtPort = SST_FAKE_PORT_MAKE(bn);
#elif defined HAL_HW
    if (!halInfo.csim)
      halInfo.boardInfo[bn].virtPort = halInfo.boardInfo[bn].physPort;
#endif
    PCICFG_RD(PCI_COMMAND, dn, cmd);
    cmd |= SST_PCIMEM_ACCESS_EN;        // enable memory accesses
    cmd |= SST_PCIIO_ACCESS_EN;         // enable i/o port accesses
    PCICFG_WR(PCI_COMMAND, dn, cmd);
}
#endif

    if (halInfo.boardInfo[bn].vendorID != _3DFX_PCI_ID) {
        GDBG_ERROR("fxHalMapBoard", "vendorID != 3Dfx 0x%x\n",_3DFX_PCI_ID);
        exit(1);
    }

    GDBG_INFO(0,"fxHalMapBoard: 2nd base address not mapped yet\n");

    GDBG_INFO(3,"    virtAddr[0]:0x%08x physAddr[0]:0x%08x\n",
                        halInfo.boardInfo[bn].virtAddr[0],
                        halInfo.boardInfo[bn].physAddr[0]);
    GDBG_INFO(3,"    virtAddr[1]:0x%08x physAddr[1]:0x%08x\n",
                        halInfo.boardInfo[bn].virtAddr[1],
                        halInfo.boardInfo[bn].physAddr[1]);
    GDBG_INFO(3,"    physPort:0x%08x\n",
                        halInfo.boardInfo[bn].physPort);

    GDBG_INFO(2,"fxHalMapBoard done,  deviceNumber:0x%x(%d) boards=%d\n",
                        halInfo.boardInfo[bn].deviceNumber,
                        halInfo.boardInfo[bn].deviceNumber,
                        halInfo.boardsFound);
    return halInfo.boardInfo[bn].virtAddr[0];
}

FxBool FX_CSTYLE fxHalInitRegisters( SstRegs *sst )
{
    int save120 = GDBG_GET_DEBUGLEVEL(120);
    SstIORegs *sstio = (SstIORegs *)SST_IO_ADDRESS(sst);
    FxDeviceInfo *info;
    int bn;

#ifdef HAL_HW
    FxU32 pciCommandReg, grxSpeedInMHz, ioBase;
    FxU32 sgramMode, sgramMask, sgramColor, blockWriteThresh, reset;
#endif

    GDBG_INFO(1,"fxHalInitRegisters(0x%x)\n",sst);
    if(!sst)
        return(FXFALSE);

    if (fxHalVaddrToBoardNumber( sst, &bn ))    // find the board
        info = &halInfo.boardInfo[bn];
    else
        return FXFALSE;

    // shut up the init code for now
    GDBG_SET_DEBUGLEVEL(120,GDBG_GET_DEBUGLEVEL(320));

#ifdef HAL_HW
    if (halInfo.hw)
    {
        //
        // h/w chip initialization!
        //

        // enable PCI memory and i/o decode
        //
        pciCommandReg =
            BIT(0) |            // enable i/o decode
            BIT(1);             // enable memory decode

#if defined H3_A0 || defined H3_A1 || defined H3_A2
	// reset 2d section to work around the problem of performing
	// dramCommand's after certain blt operations.  This workaround
	// also has the side effect of clearing commandEx.
	GDBG_INFO(0,"HACK -- resetting 2d section\n");
	ISET(sstio->miscInit0,BIT(5));
	ISET(sstio->miscInit0,0x0);
#else
	// clear the wait-on-vsync bit so dramCommand's won't
	// hang waiting for a vsync
	{
	  SstGRegs *sstg = (SstGRegs *)SST_GUI_ADDRESS(sst);
	  GDBG_INFO(0,"HACK -- forcing CommandEx = 0\n");
	  ISET(sstg->commandEx,0x0);
	}
#endif
        pciSetConfigData(PCI_COMMAND, info->deviceNumber, &pciCommandReg);

	GDBG_INFO(0,"HACK -- forcing PCI read waitState\n");
        ISET(sstio->pciInit0,
             IGET(sstio->pciInit0) |
             SST_PCI_READ_WS);

        if (GETENV("HAL_NOINIT") == NULL || atoi(GETENV("HAL_NOINIT")) == 0)
        {
	
	  if (GETENV("SSTH3_SGRAM_MODE"))
            sgramMode = atoi(GETENV("SSTH3_SGRAM_MODE"));
	  else if (GETENV("SSTH3_SGRAM_222") &&
		   (atoi(GETENV("SSTH3_SGRAM_222")) != 0))
            sgramMode = 0x27;
	  else
            sgramMode = 0x37;
	
	  if (GETENV("SSTH3_SGRAM_MASK"))
            sgramMask = atoi(GETENV("SSTH3_SGRAM_MASK"));
	  else
            sgramMask = 0xFFFFFFFF;
	
	  if (GETENV("SSTH3_SGRAM_COLOR"))
            sgramColor = atoi(GETENV("SSTH3_SGRAM_COLOR"));
	  else
            sgramColor = 0;
	
	  if (GETENV("SSTH3_GRXCLOCK"))
            grxSpeedInMHz = atoi(GETENV("SSTH3_GRXCLOCK"));
	  else
            grxSpeedInMHz = DEFAULT_GRXCLK_SPEED;
	  	
	  if (GETENV("SSTH3_BW_THRESH"))
            blockWriteThresh = atoi(GETENV("SSTH3_BW_THRESH"));
	  else
            blockWriteThresh = 2;

	  // backdoor programming of dramInit1 as a function of mclk frequency
	  if (! GETENV("SSTH3_DRAMINIT1")) {
	      fxHalPutenv("SSTH3_DRAMINIT1=0x56c031");
	  }

	  // by default, reset the chip at the beginning of the run (unless SSTH3_NO_RESET==1)
	  if ( GETENV("SSTH3_NO_RESET") )
	    reset = atoi(GETENV("SSTH3_NO_RESET")) == 1 ? 0 : 1;
	  else
	    reset = 1;
	  if ( ! reset )
	    GDBG_INFO(0,"*** SKIPPING RESET ***\n");

	  pciGetConfigData(PCI_IO_BASE_ADDRESS, info->deviceNumber, &ioBase);
	  ioBase &= ~1;
	
#ifdef H4
	  h4InitPlls(ioBase, info->deviceID, grxSpeedInMHz);
#else
	  h3InitPlls(ioBase, grxSpeedInMHz, memSpeedInMHz);
#endif
	  if ( reset )
	    h3InitResetAll(ioBase);
	  h3InitSgram(ioBase, sgramMode, sgramMask, sgramColor,GETENV("SSTH3_SGRAM_VENDOR"));
	  h3InitVga(ioBase, FXFALSE); // don't enable VGA decode
	  h3InitBlockWrite(ioBase,blockWriteThresh,blockWriteThresh);  // enable unless threshhold == 0
	  h3InitMeasureSiProcess(ioBase);
        }
    }
#endif

    fxHalIdleNoNop( sst );

    // sync csim with hardware's value of dramInit1
    // used by csim to determine whether SGRAM or SDRAM is being used
    ISET(sstio->dramInit1, IGET(sstio->dramInit1));

    /* Enable triangle alternate register mapping */
    ISET(sstio->miscInit1, IGET(sstio->miscInit1) | SST_ALT_REGMAPPING );

    /* Turn off swapping and swizzling */
    ISET(sstio->miscInit0, IGET(sstio->miscInit0) &
	 ~(SST_REGISTER_WORD_SWIZZLE_EN | SST_REGISTER_BYTE_SWIZZLE_EN |
	   SST_RAWLFB_WORD_SWIZZLE_EN | SST_RAWLFB_BYTE_SWIZZLE_EN)
	 );

    fxHalIdleNoNop( sst );

    // init the rendering registers and get device info
    fxHalInitRenderingRegisters(sst);
    fxHalInitGuiRegisters( (SstGRegs *)SST_GUI_ADDRESS(sst) );
    fxHalInitCmdAgpRegisters( (SstCRegs *)SST_CMDAGP_ADDRESS(sst) );
    if(fxHalFillDeviceInfo(sst) == FXFALSE)
        return(FXFALSE);

    // set raw lfb to use linear memory addressing
    GDBG_INFO(1,"fxHalInitRegisters PS WARNING: forcing raw lfb to use linear memory\n");
    ISET(sstio->lfbMemoryConfig,(~0x0) & SST_RAW_LFB_TILE_BEGIN_PAGE);

    GDBG_INFO(1,"fxHalInitRegisters PS WARNING: skipping init of video registers\n");

#ifdef HAL_HW
    // HACK FIXME:  This can't be used in REAL init code!!!!
    if (halInfo.hw) {
        // To quote hsimio.c, "We make a gross assumption that we're only using
        // one  board."
        //
        memset( (void *)halInfo.boardInfo[0].physAddr[1], 0x0, halInfo.boardInfo[0].fbiMemSize << 20 );
    }
#endif



    GDBG_SET_DEBUGLEVEL(120,save120);   // restore level 120 debug
    return FXTRUE;
}

FxBool FX_EXPORT FX_CSTYLE
fxHalInitCmdAgpRegisters( SstCRegs *sstc )
{
    GDBG_INFO(2,"fxHalInitCmdAgpRegisters(0x%x)\n",sstc);
    if(!sstc)
      return(FXFALSE);

    ISET(sstc->agpReqSize, 0x0);
    ISET(sstc->hostAddrLow, 0x0);
    ISET(sstc->hostAddrHigh, 0x0);
    ISET(sstc->graphicsAddr, 0x0);
    ISET(sstc->graphicsStride, 0x0);

    ISET(sstc->cmdFifo0.baseSize, 0x0); // turn off cmdfifo0
    ISET(sstc->cmdFifo0.baseAddrL, 0x0);
    ISET(sstc->cmdFifo0.bump, 0x0);
    ISET(sstc->cmdFifo0.readPtrL, 0x0);
    ISET(sstc->cmdFifo0.readPtrH, 0x0);
    ISET(sstc->cmdFifo0.aMin, 0x0);
    ISET(sstc->cmdFifo0.aMax, 0x0);
    ISET(sstc->cmdFifo0.depth, 0x0);
    ISET(sstc->cmdFifo0.holeCount, 0x0);

    ISET(sstc->cmdFifo1.baseSize, 0x0); // turn off cmdfifo0
    ISET(sstc->cmdFifo1.baseAddrL, 0x0);
    ISET(sstc->cmdFifo1.bump, 0x0);
    ISET(sstc->cmdFifo1.readPtrL, 0x0);
    ISET(sstc->cmdFifo1.readPtrH, 0x0);
    ISET(sstc->cmdFifo1.aMin, 0x0);
    ISET(sstc->cmdFifo1.aMax, 0x0);
    ISET(sstc->cmdFifo1.depth, 0x0);
    ISET(sstc->cmdFifo1.holeCount, 0x0);

    ISET(sstc->cmdFifoThresh, 0x0);
    ISET(sstc->yuvBaseAddr, 0x0);
    ISET(sstc->yuvStride, 0x0);

    return FXTRUE;
}


FxBool FX_EXPORT FX_CSTYLE
fxHalInitGuiRegisters( SstGRegs *sstg )
{
    GDBG_INFO(2,"fxHalInitGuiRegisters(0x%x)\n",sstg);
    if(!sstg)
      return(FXFALSE);

    ISET(sstg->clip0min, 0x0);
    ISET(sstg->clip0max, 0x0);
    ISET(sstg->dstBaseAddr, 0x0);
    ISET(sstg->dstFormat, 0x0);
    ISET(sstg->srcColorkeyMin, 0x0);
    ISET(sstg->srcColorkeyMax, 0x0);
    ISET(sstg->dstColorkeyMin, 0x0);
    ISET(sstg->dstColorkeyMax, 0x0);
    ISET(sstg->bresError0, 0x0);
    ISET(sstg->bresError1, 0x0);
    ISET(sstg->rop, 0x0);
    ISET(sstg->srcBaseAddr, 0x0);
    ISET(sstg->commandEx, 0x0);
    ISET(sstg->lineStipple, 0x0);
    ISET(sstg->lineStyle, 0x0);
    ISET(sstg->pattern0alias, 0x0);
    ISET(sstg->pattern1alias, 0x0);
    ISET(sstg->clip1min, 0x0);
    ISET(sstg->clip1max, 0x0);
    ISET(sstg->srcFormat, 0x0);
    ISET(sstg->srcSize, 0x0);
    ISET(sstg->srcXY, 0x0);
    ISET(sstg->colorBack, 0x0);
    ISET(sstg->colorFore, 0x0);
    ISET(sstg->dstSize, 0x0);
    ISET(sstg->dstXY, 0x0);
    ISET(sstg->command, 0x0);

    return FXTRUE;
}

FxBool FX_EXPORT FX_CSTYLE
fxHalInitRenderingRegisters( SstRegs *sst )
{
    FxU32 i;
    SstGRegs *sstg = (SstGRegs *)SST_GUI_ADDRESS(sst);

    GDBG_INFO(2,"fxHalInitRenderingRegisters(0x%x)\n",sst);
    if(!sst)
        return(FXFALSE);

    ISET(sst->vA.x, 0x0);
    ISET(sst->vA.y, 0x0);
    ISET(sst->vB.x, 0x0);
    ISET(sst->vB.y, 0x0);
    ISET(sst->vC.x, 0x0);
    ISET(sst->vC.y, 0x0);

    ISET(sst->r, 0x0);
    ISET(sst->g, 0x0);
    ISET(sst->b, 0x0);
    ISET(sst->z, 0x0);
    ISET(sst->a, 0x0);
    ISET(sst->s, 0x0);
    ISET(sst->t, 0x0);
    ISET(sst->w, 0x0);

    ISET(sst->drdx, 0x0);
    ISET(sst->dgdx, 0x0);
    ISET(sst->dbdx, 0x0);
    ISET(sst->dzdx, 0x0);
    ISET(sst->dadx, 0x0);
    ISET(sst->dsdx, 0x0);
    ISET(sst->dtdx, 0x0);
    ISET(sst->dwdx, 0x0);

    ISET(sst->drdy, 0x0);
    ISET(sst->dgdy, 0x0);
    ISET(sst->dbdy, 0x0);
    ISET(sst->dzdy, 0x0);
    ISET(sst->dady, 0x0);
    ISET(sst->dsdy, 0x0);
    ISET(sst->dtdy, 0x0);
    ISET(sst->dwdy, 0x0);

    ISET(sst->fbzColorPath, 0x0);
    ISET(sst->fogMode, 0x0);
    ISET(sst->alphaMode, 0x0);
    ISET(sst->fbzMode, 0x0);
    ISET(sst->lfbMode, 0x0);
    ISET(sst->clipLeftRight, 0x0);
    ISET(sst->clipBottomTop, 0x0);
    ISET(sst->fogColor, 0x0);
    ISET(sst->zaColor, 0x0);
    ISET(sst->chromaKey, 0x0);
    ISET(sst->chromaRange, 0x0);
    ISET(sst->stipple, 0x0);
    ISET(sst->c0, 0x0);
    ISET(sst->c1, 0x0);

    ISET(sst->nopCMD, 0xF);   // Clear fbistat registers

    for (i=0; i<32; i++)
        ISET(sst->fogTable[i], 0x0);

    ISET(sst->sSetupMode, 0x0);
    ISET(sst->sVx, 0x0);
    ISET(sst->sVy, 0x0);
    ISET(sst->sARGB, 0x0);
    ISET(sst->sRed, 0x0);
    ISET(sst->sGreen, 0x0);
    ISET(sst->sBlue, 0x0);
    ISET(sst->sAlpha, 0x0);
    ISET(sst->sVz, 0x0);
    ISET(sst->sOowfbi, 0x0);
    ISET(sst->sOow0, 0x0);
    ISET(sst->sSow0, 0x0);
    ISET(sst->sTow0, 0x0);
    ISET(sst->sOow1, 0x0);
    ISET(sst->sSow1, 0x0);
    ISET(sst->sTow1, 0x0);

    ISET(sst->clipLeftRight1, 0x0);
    ISET(sst->clipBottomTop1, 0x0);

    // clear TMU registers
    ISET(sst->textureMode, 0x0);
    ISET(sst->tLOD, 0x0);
    ISET(sst->tDetail, 0x0);
    ISET(sst->texBaseAddr, 0x0);
    ISET(sst->texBaseAddr1, 0x0);
    ISET(sst->texBaseAddr2, 0x0);
    ISET(sst->texBaseAddr38, 0x0);

    for (i=0; i<12; i++)
        ISET(sst->nccTable0[i], 0x0);
    for (i=0; i<12; i++)
        ISET(sst->nccTable1[i], 0x0);

    // clear gui registers
    ISET(sstg->clip0min, 0x0);
    ISET(sstg->clip0max, 0x0);
    ISET(sstg->dstBaseAddr, 0x0);
    ISET(sstg->dstFormat, 0x0);
    ISET(sstg->srcColorkeyMin, 0x0);
    ISET(sstg->srcColorkeyMax, 0x0);
    ISET(sstg->dstColorkeyMin, 0x0);
    ISET(sstg->dstColorkeyMax, 0x0);
    ISET(sstg->bresError0, 0x0);
    ISET(sstg->bresError1, 0x0);
    ISET(sstg->rop, 0x0);
    ISET(sstg->srcBaseAddr, 0x0);
    ISET(sstg->commandEx, 0x0);
    ISET(sstg->lineStipple, 0x0);
    ISET(sstg->lineStyle, 0x0);
    ISET(sstg->pattern0alias, 0x0);
    ISET(sstg->pattern1alias, 0x0);
    ISET(sstg->clip1min, 0x0);
    ISET(sstg->clip1max, 0x0);
    ISET(sstg->srcFormat, 0x0);
    ISET(sstg->srcSize, 0x0);
    ISET(sstg->srcXY, 0x0);
    ISET(sstg->colorBack, 0x0);
    ISET(sstg->colorFore, 0x0);
    ISET(sstg->dstSize, 0x0);
    ISET(sstg->dstXY, 0x0);
    ISET(sstg->command, 0x0);

    fxHalIdle(sst);
    return FXTRUE;
}

FxBool FX_EXPORT FX_CSTYLE
fxHalShutdown( SstRegs *sst )
{
    FxU32 bn;           // board number

    GDBG_INFO(1,"fxHalShutdown(0x%x)\n",sst);
    if (!fxHalVaddrToBoardNumber( sst, &bn ))
        return(FXFALSE);
    if (halInfo.hw) {

      if ( halInfo.boardInfo[bn].agpSizeInBytes != 0 ) {
#ifdef HAL_HW
	if ( ! fxAGPUncommit((FxU32)halInfo.boardInfo[bn].agpMem,0,halInfo.boardInfo[bn].agpSizeInBytes>>12) ) {
	  GDBG_ERROR("fxHalShutdown","%s\n",fxAGPGetErrorString());
	} else {
	  if ( ! fxAGPFree((FxU32)halInfo.boardInfo[bn].agpMem) ) {
	    GDBG_ERROR("fxHalShutdown","%s\n",fxAGPGetErrorString());
	  } else {
	    fxAGPShutdown();
	  }
	}
#endif
      }
        // GMT: shut down the real hardware
        GDBG_INFO(0,"fxHalShutdown(0x%x) NYI on real HW, need to shutdown video GMT\n",sst);
    }
    if (halInfo.csim) {
        guiShutdown(SST_FAKE_ADDRESS_GET_CSIM(sst));
        csimShutdown(SST_FAKE_ADDRESS_GET_CSIM(sst));
    }
    fxHalResetBoardInfo(&halInfo.boardInfo[bn]);
    return FXTRUE;
}

void FX_EXPORT FX_CSTYLE
fxHalShutdownAll( void )
{
    FxU32 bn;
    GDBG_INFO(1,"fxHalShutdownAll()\n");
    for (bn = 0; bn < HAL_MAX_BOARDS; bn++) {
        fxHalShutdown(halInfo.boardInfo[bn].virtAddr[0]);
    }
#ifndef KERNEL
    pciClose();
#endif
}

FxBool FX_EXPORT FX_CSTYLE
fxHalGetDeviceInfo( SstRegs *sst, FxDeviceInfo *info )
{
    FxU32 bn;

#if !defined(FX_DLL_ENABLE)
    GDBG_INIT();
#endif
    GDBG_INFO(1,"fxHalGetDeviceInfo(0x%x,*)\n",sst);
    if (fxHalVaddrToBoardNumber( sst, &bn )) {  // find the board
        *info = halInfo.boardInfo[bn];          // copy the info
        return FXTRUE;                          // return success
    }
    return FXFALSE;
}

/*
** fxHalInitUSWC
**
** Sets up memory caching on P6 systems.
**
*/
FX_EXPORT FxBool FX_CSTYLE
fxHalInitUSWC(SstRegs *sst)
{
  GDBG_INFO(1,"fxHalInitUSWC(0x%x)\n",sst);
#if ! defined( __unix__ ) && ! defined( KERNEL )
  {
  FxBool res;
  FxU32  physAddr;
  FxU32  bn;    // board number
  static FxU32  mtrr;

  /* Turn on Write combining for wrap 1-n  */
  if (!fxHalVaddrToBoardNumber( sst, &bn )) {   // find the board
    GDBG_ERROR("fxHalInitUSWC","bad SST address: 0x%x\n",sst);
    return FXFALSE;
  }
  physAddr = halInfo.boardInfo[bn].physAddr[1];

  /* For some reason, there sometimes is a 008 at the end of the
  physical address, so mask that puppy RTF out */
  physAddr &= 0xfffff000;

  /* Set up USWC MTRR */
  res = pciFindMTRRMatch(physAddr, 0x2000000,
                         PciMemTypeWriteCombining, &mtrr);

  if (!res)
    res = pciFindFreeMTRR(&mtrr);
  else
    return FXTRUE;              /* It's already there.  We're done. */

  if (res)
    pciSetMTRR(mtrr, physAddr, 0x1000000, PciMemTypeWriteCombining);
  }
#endif
  return FXTRUE;
} /* fxHalInitUSWC */


FX_EXPORT void FX_CSTYLE
fxHalInitVideoOverlaySurface(
    SstRegs *sst,
    FxU32 enable,               // 1=enable Overlay surface (OS), 1=disable
    FxU32 stereo,               // 1=enable OS stereo, 0=disable
    FxU32 horizScaling,         // 1=enable horizontal scaling, 0=disable
    FxU32 dudx,                 // horizontal scale factor (ignored if not
                                // scaling)
    FxU32 verticalScaling,      // 1=enable vertical scaling, 0=disable
    FxU32 dvdy,                 // vertical scale factor (ignored if not
                                // scaling)
    FxU32 filterMode,           // duh
    FxU32 tiled,                // 0=OS linear, 1=tiled
    FxU32 pixFmt,               // pixel format of OS
    FxU32 clutBypass,           // bypass clut for OS?
    FxU32 clutSelect,           // 0=lower 256 CLUT entries, 1=upper 256
    FxU32 startAddress,         // board address of beginning of OS
    FxU32 stride)               // distance between scanlines of the OS, in
                                // units of bytes for linear OS's and tiles for
                                // tiled OS's
{
  FxU32 bn;

#ifndef KERNEL
  FxU32 ioBase;
#endif

  FxDeviceInfo *info;

  if (!fxHalVaddrToBoardNumber( sst, &bn )) {   // find the board
    GDBG_ERROR("fxHalInitUSWC","bad SST address: 0x%x\n",sst);
  }

  info = &halInfo.boardInfo[bn];

#ifndef KERNEL
  pciGetConfigData(PCI_IO_BASE_ADDRESS, info->deviceNumber, &ioBase);
  ioBase &= ~1;
#endif

#ifdef HAL_HW
  h3InitVideoOverlaySurface(
      ioBase, enable, stereo, horizScaling, dudx, verticalScaling, dvdy,
      filterMode, tiled, pixFmt, clutBypass, clutSelect, startAddress,
      stride);
#endif
}  /* fxHalInitVideoOverlaySurface */
