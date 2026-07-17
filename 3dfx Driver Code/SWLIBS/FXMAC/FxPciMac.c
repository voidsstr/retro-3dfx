/*-*-c++-*-*/
/* FXPCIMAC.C
 * Copyright (c) 1996 alt.drivers inc.
 * Portions Copyright (c) 1996 3Dfx Interactive, Inc.
 * All Rights Reserved
 *
 * $Header: FxPciMac.c, 5, 10/11/00 7:37:22 PM, Brent$
 * $Log: 
 *  5    3dfx      1.3.1.0     10/11/00 Brent           Forced check in to enforce
 *       branch.
 *  4    3dfx      1.3         07/20/00 Andrew  Bell    Rolling back all swlibs
 *       files to 2nd latest version due to an incorrect checkin.  This should
 *       bring things back to normal.
 *  3    3dfx      1.2         07/19/00 Dinesh Raja Savari Amirtharaj CSIM for the
 *       RTL release_3_2
 *  2    3dfx      1.1         05/27/00 Stephane Huaulme fixed pb with Universal
 *       Interface 3.3
 *  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
 * $
** 
** 7     7/02/99 1:23p Kcd
** Bug fix in vblank handling code.
** 
** 6     3/25/99 7:09p Kcd
** Tons of changed crap.
** 
** 5     3/14/98 1:10p Peter
** mac port happiness
 */

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define __MACERRORS__
#include <3Dfx.h>
#include <fxpci.h>
#include <fxpcimac.h>

#include <DriverServices.h>
#include <Gestalt.h>
#include <NameRegistry.h>
#include <PCI.h>


#if 1
/* PUBLIC DATA  */
const PciRegister PCI_VENDOR_ID       = { 0x0,  2, READ_ONLY };
const PciRegister PCI_DEVICE_ID       = { 0x2,  2, READ_ONLY };
const PciRegister PCI_COMMAND         = { 0x4,  2, READ_WRITE };
const PciRegister PCI_STATUS          = { 0x6,  2, READ_WRITE };
const PciRegister PCI_REVISION_ID     = { 0x8,  1, READ_ONLY };
const PciRegister PCI_CLASS_CODE      = { 0x9,  3, READ_ONLY };
const PciRegister PCI_CACHE_LINE_SIZE = { 0xC,  1, READ_WRITE };
const PciRegister PCI_LATENCY_TIMER   = { 0xD,  1, READ_WRITE };
const PciRegister PCI_HEADER_TYPE     = { 0xE,  1, READ_ONLY };
const PciRegister PCI_BIST            = { 0xF,  1, READ_WRITE };
const PciRegister PCI_BASE_ADDRESS_0  = { 0x10, 4, READ_WRITE };
const PciRegister PCI_BASE_ADDRESS_1  = { 0x14, 4, READ_WRITE };

const PciRegister PCI_IO_BASE_ADDRESS = { 0x18, 4, READ_WRITE };
const PciRegister PCI_SUBVENDOR_ID    = { 0x2C, 4, READ_ONLY };
const PciRegister PCI_SUBSYSTEM_ID    = { 0x2E, 4, READ_ONLY };
const PciRegister PCI_ROM_BASE_ADDRESS= { 0x30, 4, READ_WRITE };
const PciRegister PCI_CAP_PTR         = { 0x34, 4, READ_WRITE };

const PciRegister PCI_INTERRUPT_LINE  = { 0x3C, 1, READ_WRITE };
const PciRegister PCI_INTERRUPT_PIN   = { 0x3D, 1, READ_ONLY };
const PciRegister PCI_MIN_GNT         = { 0x3E, 1, READ_ONLY };
const PciRegister PCI_MAX_LAT         = { 0x3F, 1, READ_ONLY };

const PciRegister PCI_FAB_ID          = { 0x40, 1, READ_ONLY };
const PciRegister PCI_CONFIG_STATUS   = { 0x4C, 4, READ_WRITE };
const PciRegister PCI_CONFIG_SCRATCH  = { 0x50, 4, READ_WRITE };
const PciRegister PCI_AGP_CAP_ID      = { 0x54, 4, READ_ONLY };
const PciRegister PCI_AGP_STATUS      = { 0x58, 4, READ_ONLY };
const PciRegister PCI_AGP_CMD         = { 0x5C, 4, READ_WRITE };
const PciRegister PCI_ACPI_CAP_ID     = { 0x60, 4, READ_ONLY };
const PciRegister PCI_CNTRL_STATUS    = { 0x64, 4, READ_WRITE };

/* sst1 definitions left in for compatability */
const PciRegister PCI_SST1_INIT_ENABLE = { 0x40, 4, READ_WRITE }; 
const PciRegister PCI_SST1_BUS_SNOOP_0 = { 0x44, 4, READ_WRITE }; 
const PciRegister PCI_SST1_BUS_SNOOP_1 = { 0x48, 4, READ_WRITE }; 
const PciRegister PCI_SST1_CFG_STATUS  = { 0x4C, 4, READ_WRITE };
#endif
#if 1
// PCI PowerMac data
#define kMaxMacPCIDeviceCount (sizeof(long) * CHAR_BIT)
MacPciDevice sMacPciDeviceList[kMaxMacPCIDeviceCount];
#endif
/************************
**                     **
**  pciGetErrorString  **
**                     **
************************/
#if 1
typedef struct _PCIErr {
  FxU32 code;
  char *string;
} PCIErr, *PCIErrPtr;

static PCIErr pciError[] = {
  {PCI_ERR_NOERR, "No errors.\n"},
  {PCI_ERR_WINRTINIT, "WinRT initialization failure.\n" },
  {PCI_ERR_MEMMAPVXD, "Memmap VxD initialization failure.\n" },
  {PCI_ERR_MAPMEMDRV, "Mapmem driver initialization failure.\n" },
  {PCI_ERR_GENPORT, "Genport I/O initialization failure.\n" },
  {PCI_ERR_NO_BUS, "No PCI Bus detected.\n"},
  {PCI_ERR_NOTOPEN, "PCI library not open.\n" },
  {PCI_ERR_NOTOPEN2, "Closing unopened PCI library.\n" },
  {PCI_ERR_NOTOPEN3, "pciGetConfigData() on unopened library.\n" }, 
  {PCI_ERR_OUTOFRANGE, "Device_number is out of range.\n" },
  {PCI_ERR_NODEV, "Cannot read from a non-existant device.\n" },
  {PCI_ERR_NODEV2, "Cannot update config regs from non-existant device.\n" },
  {PCI_ERR_WRITEONLY, "Cannot read a WRITE_ONLY register.\n" },
  {PCI_ERR_READONLY, "Cannot write a READ_ONLY register.\n" },
  {PCI_ERR_PHARLAP, "Phar Lap returned an error trying to map memory.\n" },
  {PCI_ERR_WRONGVXD, "Expected VxD version V%d.%d, got V%d.%d\n"},
  {PCI_ERR_MEMMAP, "Memmap returned an error trying to map memory.\n" },
  {PCI_ERR_MAPMEM, "Mapmem returned an error trying to map memory.\n" },
  {PCI_ERR_WINRT, "Winrt returned an error trying to map memory.\n" },
  {PCI_ERR_VXDINUSE, "Mutual exclusion prohibits this\n" }
};
#endif
static FxU32 pciErrorCode = PCI_ERR_NOERR;
#if 1
const char *
pciGetErrorString(void)
{
#if HAS_VXD
  static char vxdErrString[120];
  if (pciErrorCode == PCI_ERR_WRONGVXD) {
    sprintf(vxdErrString, "Expected VXD version V%d.%d, got V%d.%d\n",
            FX_MAJOR_VER, FX_MINOR_VER,
            BYTE1(vxdVer), BYTE0(vxdVer));
    return vxdErrString;
  }
#endif /* HAS_VXD */

  return pciError[pciErrorCode].string;
} /* pciGetErrorString */

#endif
#if 1
FxU32
pciGetErrorCode(void)
{
  return pciError[pciErrorCode].code;
} /* pciGetErrorCode */
#endif
#define kGestaltSelector3dfx '3Dfx'

#if 0
/* This is the template code for the assembly fragment below */
static OSErr
myGestaltSelectorProc(OSType gestaltSel, long* gestaltVal)
{
  OSErr retVal = gestaltUndefSelectorErr;

  if (gestaltSel == kGestaltSelector3dfx) {
    *gestaltVal = NULL;
    retVal = noErr;
  }

  return retVal;
}
#endif

static unsigned long myGestaltSelectorProc[] = {
	0x7CC802A6UL, // mflr     r6
	0x3CA03344UL, // lis      r5, '3D'
	0x3800EA51UL, // li       r0, gestaltUndefSelectorErr
	0x38A56678UL, // addi     r5, r5, 'fx'
	0x7C032840UL, // cmplw    r3, r5
	0x40820018UL, // bne      * + 24
	0x48000005UL, // bl 			* + 4
	0x7C6802A6UL, // mflr     r3
	0x3863001CUL, // addi     r3, r3, 28
	0x38000000UL, // li       r0, 0
	0x90640000UL, // stw      r3, 0(r4)
	0x7CC803A6UL, // mtlr     r6
	0x7C030378UL, // mr       r3, r0
	0x4E800020UL, // blr
	0x00000000UL  // Magic Local Variable
};

static unsigned long* mySelectorTransitionVector[] = {
	myGestaltSelectorProc,
	NULL
};

static RoutineDescriptor myGestaltSelectorRD = BUILD_ROUTINE_DESCRIPTOR(uppSelectorFunctionProcInfo,
																																				mySelectorTransitionVector);
#if 1																																				
static FxBool 
BoardMutex(FxU32 cardNum, FxBool tryMapP)
{
  FxBool retVal = FXFALSE;

  /* If we have pci now check to see if someone already has the card
   * mapped.  
   */
  while(pciOpen()) {
    long gestaltVal;
    OSErr theErr = Gestalt(kGestaltSelector3dfx, &gestaltVal);
    
    /* Have we added our selector yet? */
    if (theErr == gestaltUndefSelectorErr) {
			unsigned long* sysHeapPtr = (unsigned long*)NewPtrSys(sizeof(myGestaltSelectorProc) +
																														sizeof(mySelectorTransitionVector) +
																														sizeof(myGestaltSelectorRD));
			/* Stash function stuff in the system heap */
			if (sysHeapPtr != NULL) {
				/* Move our proc which should have no relocatable
				 * references in it.
				 */
				BlockMoveDataUncached(myGestaltSelectorProc, 
															sysHeapPtr, 
															sizeof(myGestaltSelectorProc));

				/* Adjust the transition vector's code pointer to
				 * be the new one in the system heap. We don't care
				 * about the toc pointer since we don't reference
				 * any global data.
				 */
				mySelectorTransitionVector[0] = sysHeapPtr;
				BlockMoveDataUncached(mySelectorTransitionVector, 
															(unsigned char*)sysHeapPtr + sizeof(myGestaltSelectorProc), 
															sizeof(mySelectorTransitionVector));

				/* Adjust the RoutineDescriptor's proc pointer to be our new 
				 * adjusted transition vector.
				 */
				myGestaltSelectorRD.routineRecords[0].procDescriptor = (ProcPtr)((unsigned char*)sysHeapPtr +
																																				 sizeof(myGestaltSelectorProc));															
				BlockMoveDataUncached(&myGestaltSelectorRD, 
															((unsigned char*)sysHeapPtr + 
															 sizeof(myGestaltSelectorProc) + 
															 sizeof(mySelectorTransitionVector)),
															sizeof(myGestaltSelectorRD));

				/* Install our custom gestalt selector */
				if (NewGestalt(kGestaltSelector3dfx, 
											 (UniversalProcPtr)((unsigned char*)sysHeapPtr + 
											 										sizeof(myGestaltSelectorProc) + 
											 										sizeof(mySelectorTransitionVector))) != noErr) {
					DisposePtr((Ptr)sysHeapPtr);
				}
			}
    } else if (theErr == noErr) {
			unsigned long* boardMapFlagPtr = (unsigned long*)gestaltVal;
			const unsigned long boardMask = 0x01UL << cardNum;
			
			/* Is this board in use? */
			retVal = ((*boardMapFlagPtr & boardMask) == boardMask);
			
			/* If we're trying to map the board check to see that no
			 * one else has already mapped the board. Otherwise, check
			 * to see that the board is already mapped and clear the flag.
			 */
			if (tryMapP) {
				retVal = !retVal;
				if (retVal) *boardMapFlagPtr |= boardMask;
			} else {
				if (retVal) *boardMapFlagPtr &= ~boardMask;
			}
			
			break;
    }
  }

	if (!retVal) pciErrorCode = PCI_ERR_VXDINUSE;
  return retVal;
}
#endif

/* Routine that cleans up the system level mutex stuff when
 * we are exited, either normally by quitting, force quit, or
 * 'es' in macsbug.
 *
 * NB: Specifying this function is done at build time.
 */
#if !H3
#if __MWERKS__
/* MW linker generated cleanup stub */
extern void __terminate(void);
extern pascal void myCodeFragCleanup(void);

extern void resetVideo(void);

extern pascal void 
myCodeFragCleanup(void)
{
	UInt32 ii;

	for(ii = 0; ii < 15; ++ii) {
		if(sMacPciDeviceList[ii].found) {
			BoardMutex(ii, FXFALSE);
			sMacPciDeviceList[ii].found = false;
#if 0	
		
			RegistryEntryIDDispose(&gRegEntryId[ii]);
#endif
		}
	}
	// Restore display mode if it didn't get done already.
    SstCleanupMac();
#if H3	
	resetVideo();
#endif	
	__terminate();
}
#else /* !__MWERKS__ */
#error "Need CFM cleanup proc for this compiler"
#endif /* !__MWERKS__ */
#endif

#if 1

#if 1
/**************
**           **
**  pciOpen  **
**           **
**************/

FX_ENTRY FxBool FX_CALL 
pciOpen(void)
{
	long gestaltVal;

  /* According to the Apple docs, if the system supports the name
   * registry then it also has a pci bus. 
   */
  return ((Gestalt(gestaltNameRegistryVersion, &gestaltVal) == noErr) &&
          (gestaltVal >= 0));
}

/***************
**            **
**  pciClose  **
**            **
***************/

FX_ENTRY FxBool FX_CALL 
pciClose(void)
{
	return FXTRUE;
}

/**********************
**                   **
**  pciDeviceExists  **
**                   **
**********************/

FX_ENTRY FxBool FX_CALL 
pciDeviceExists(FxU32 device_number)
{
	/************************************************************ 
    This procedure assumes that pciFindCardMulti has been called
    to set up the "found" table corresponding to all the 
    PCI devices installed.
    *************************************************************/
	return (FxBool) sMacPciDeviceList[device_number].found;
}

/***********************
**                    **
**  pciGetConfigData  **
**                    **
***********************/

FX_ENTRY FxBool FX_CALL 
pciGetConfigData(PciRegister reg, FxU32 device_number, FxU32* data)
{
	FxBool retVal = FXFALSE;
	
	if(sMacPciDeviceList[device_number].found) {
		switch(reg.sizeInBytes) {
		case 1:
		{
			UInt8 tempData;

			retVal = (ExpMgrConfigReadByte(&sMacPciDeviceList[device_number].regEntryId, 
                                     (LogicalAddress)reg.regAddress, 
                                     &tempData) == noErr);
			*data = tempData;
		}
		break;
		
		case 2:
		{
			UInt16 tempData;
			
			retVal = (ExpMgrConfigReadWord(&sMacPciDeviceList[device_number].regEntryId, 
                                     (LogicalAddress)reg.regAddress, 
                                     &tempData) == noErr);
			*data = tempData;
		}
		break;
		
		case 3:
			*data = 0x040000;
			break;
			
		case 4:
		{
			UInt32 tempData;
			
			retVal = (ExpMgrConfigReadLong(&sMacPciDeviceList[device_number].regEntryId, 
                                     (LogicalAddress)reg.regAddress, 
                                     &tempData) == noErr);
			*data = tempData;
		}
		break;
		}
	}

	return retVal;
}

/***********************
**                    **
**  pciSetConfigData  **
**                    **
***********************/

FX_ENTRY FxBool FX_CALL 
pciSetConfigData(PciRegister reg,
                 FxU32       device_number,
                 FxU32*      data)
{
	if(sMacPciDeviceList[device_number].found)
	{
		switch(reg.sizeInBytes)
		{
		case 1:
			ExpMgrConfigWriteByte(&sMacPciDeviceList[device_number].regEntryId, (LogicalAddress) reg.regAddress, (UInt8) *data);
			break;

		case 2:
			ExpMgrConfigWriteWord(&sMacPciDeviceList[device_number].regEntryId, (LogicalAddress) reg.regAddress, (UInt16) *data);
			break;

		case 4:
			ExpMgrConfigWriteLong(&sMacPciDeviceList[device_number].regEntryId, (LogicalAddress) reg.regAddress, *data);
			break;
		}
	}

	return sMacPciDeviceList[device_number].found;
}

/***********************
**                    **
**  pciUnmapPhysical  **
**                    **
***********************/

FX_ENTRY void FX_CALL 
pciUnmapPhysical(FxU32 linear_addr,
                 FxU32 length)
{
	UInt32 ii;

	for(ii = 0; ii < 15; ++ii) {
		if(sMacPciDeviceList[ii].found) {
			/* FIXME - This doesn't really work right for multiple mappings to the same device. */
			if(linear_addr == sMacPciDeviceList[ii].pciPhysAddr[0]) {
				/* BoardMutex(ii, FXFALSE); */
				sMacPciDeviceList[ii].pciPhysAddr[0] = 0;
			}
		}
	}
}


/***********************
**                    **
**  pciFindCardMulti  **
**                    **
***********************/

FX_ENTRY FxBool FX_CALL 
pciFindCardMulti(FxU32  vendorId,
                 FxU32  deviceId,
                 FxU32* devNum,
                 FxU32  cardNum)
{
	OSErr err = noErr;
	RegEntryID   regEntryId;
	RegEntryIter regEntryIter;
	Boolean      done = true;
	RegIterationOp regIterOp;
	static FxU32 sCurSearchNum = 0;

	*devNum = cardNum;
	if(sMacPciDeviceList[cardNum].found) goto bail_0;

	err = RegistryEntryIDInit(&regEntryId);
	if(err != noErr) goto bail_0;

	err = RegistryEntryIterateCreate(&regEntryIter);
	if(err != noErr) goto bail_1;
	
	/* If this is the first time that we're called then
	 * start at the root of the device tree. Otherwise,
	 * continue from the last device we found.
	 */
	if (sCurSearchNum == 0) {
		regIterOp = kRegIterDescendants;
	} else {
		regIterOp = kRegIterContinue;
		RegistryEntryIterateSet(&regEntryIter, &sMacPciDeviceList[sCurSearchNum - 1].regEntryId);
	}

	/* Iterate through the device tree */
	do {
		static const char nameProperty[] = "name";
		Boolean doAddP = false;

		err = RegistryEntrySearch(&regEntryIter, regIterOp, &regEntryId, &done,
                              nameProperty, NULL, 0);

    /* If something really went wrong or there are no more entries in
     * the device tree then stop searching.  Done implies that the
     * current entry is not valid no matter what value err has.
     */
    if ((err != noErr) || done) break;

    /* Check to see if this device matches the caller's criteria.
     * DeviceId == 0xFFFF indicates that the caller wants all devices
     * from a particular vendor.  
     */
    {
      RegPropertyIter propIter;
      OSErr propErr;
				
      propErr = RegistryPropertyIterateCreate(&regEntryId, &propIter);
      if (propErr == noErr) {
        RegPropertyNameBuf propName;
        Boolean propDone;
        Boolean vendorP = false;
        Boolean deviceP = false;
				
        /* Get the vendor/deviceId property */
        do {
          propErr = RegistryPropertyIterate(&propIter, propName, &propDone);
          if ((propErr == noErr) && !propDone) {
            RegPropertyValueSize propSize;
            char propValBuf[256];
						
            if (((propErr = RegistryPropertyGetSize(&regEntryId, propName, &propSize)) == noErr) &&
                (propSize < sizeof(propValBuf))) {
						
              propErr = RegistryPropertyGet(&regEntryId, propName, &propValBuf, &propSize);	
              if (propErr == noErr) {
                unsigned long curPropVal = *(unsigned long*)propValBuf;

                if (strcmp(propName, "vendor-id") == 0) {
                  propDone = (curPropVal != vendorId);
                  vendorP = !propDone;
                } else if (strcmp(propName, "device-id") == 0) {
                  deviceP = ((deviceId == 0xFFFF) ||
                             (deviceId == curPropVal));
                }
              }
            }
          }
        } while((propErr == noErr) && !propDone);
							
        RegistryPropertyIterateDispose(&propIter);
        doAddP = (vendorP && deviceP);
      }
    }
		
    /* Did we get match all of the relevant criteria */
    if (doAddP) {			
#if DEBUG
      {
        char testPath[1024];
        OSErr testErr = RegistryCStrEntryToPath(&regEntryId, testPath, sizeof(testPath));
        //if (testErr == noErr) printf("%s\n", testPath);
      }
#endif /* DEBUG */
      
      /* Make a copy so that the OS knows that's its in use. */
      RegistryEntryIDCopy(&regEntryId, &sMacPciDeviceList[*devNum].regEntryId);
      sMacPciDeviceList[*devNum].found = true;
      sCurSearchNum++;

      break;
    }

    /* This device did not match so continue along the device tree. */
    regIterOp = kRegIterContinue;
  } while (!done && (err == noErr));

bail_2:
  RegistryEntryIterateDispose(&regEntryIter);

bail_1:
  RegistryEntryIDDispose(&regEntryId);

bail_0:
  /* This will only be set if it was found */
  return sMacPciDeviceList[*devNum].found;
}

/**********************
**                   **
**  pciMapCardMulti  **
**                   **
**********************/

#if 1
FX_ENTRY FxU32* FX_CALL 
pciMapCardMulti(FxU32  vendorId,
                FxU32  deviceId,
                FxI32  length,
                FxU32* devNum,
                FxU32  cardNum,
                FxU32 addressNum) 
{
#if macintosh
#pragma unused (deviceId, vendorId, length)
#endif
	UInt32 physAddr = 0, commandAndStatus;

	*devNum = cardNum;

	if (cardNum < kMaxMacPCIDeviceCount) {
		physAddr = sMacPciDeviceList[cardNum].pciPhysAddr[addressNum];
		
		/* If the board has not yet been mapped then try to map it
		 * here claiming the board mutex in the process.
		 */
		if(sMacPciDeviceList[cardNum].found && 
			 (physAddr == 0x00UL) &&
			 ((addressNum != 0) || 1 /* BoardMutex(cardNum, FXTRUE) */)) {
			OSErr theErr = ExpMgrConfigReadLong(&sMacPciDeviceList[cardNum].regEntryId, 
																					(LogicalAddress)(PCI_BASE_ADDRESS_0.regAddress + (addressNum << 2UL)), 
																					&physAddr);
			physAddr &= ~0xF;
			
			/* Make sure memory accesses are turned on. */
			theErr = ExpMgrConfigReadLong(&sMacPciDeviceList[cardNum].regEntryId, (LogicalAddress) 0x04, &commandAndStatus);
			commandAndStatus |= 0x02;
			ExpMgrConfigWriteLong(&sMacPciDeviceList[cardNum].regEntryId, (LogicalAddress) 0x04, commandAndStatus);

			/* Track the pci base addresses for the board so that
			 * we can do port i/o type things etc where we need a 
			 * device # rather than a linearAddress passed in which
			 * is often confusing because of bizarro pc port i/o crap.
			 */
			sMacPciDeviceList[cardNum].pciPhysAddr[addressNum] = physAddr;
#if 0			
			if(addressNum == 0)
			{
				/* Mark I/O space uncacheable, just because I'm paranoid */
				FlushProcessorCache(kCurrentAddressSpaceID, (LogicalAddress)(physAddr & 0xfffff000), 16*1024*1024);
	  			SetProcessorCacheMode(kCurrentAddressSpaceID, (const void*)(physAddr & 0xfffff000), 16*1024*1024,kProcessorCacheModeInhibited);
			}
			else if(addressNum == 1)
			{
				/* Mark I/O space cacheable writethrough, just because I'm paranoid */
				FlushProcessorCache(kCurrentAddressSpaceID, (LogicalAddress)(physAddr & 0xfffff000), 32*1024*1024);
	  			SetProcessorCacheMode(kCurrentAddressSpaceID, (const void*)(physAddr & 0xfffff000), 32*1024*1024,kProcessorCacheModeWriteThrough);
			}
#endif						
			if (theErr == noErr) {
				const char* addrPropertyName = "AAPL,address";
				RegPropertyValueSize addrBufSize;
				FxU32* addrBuf = NULL;
				
				theErr = RegistryPropertyGetSize(&sMacPciDeviceList[cardNum].regEntryId,
																				 addrPropertyName,
																				 &addrBufSize);
				
				if (theErr != noErr) goto __errExit;
				
				addrBuf = (FxU32*)NewPtr(addrBufSize);
				if (addrBuf == NULL) goto __errExit;
				
				theErr = RegistryPropertyGet(&sMacPciDeviceList[cardNum].regEntryId,
																		 addrPropertyName,
																		 addrBuf,
																		 &addrBufSize);
				if (theErr != noErr) goto __errExit;
				
				if (0) {
				__errExit:
					physAddr = 0x00UL;
				}
				
				if (addrBuf != NULL) DisposePtr((Ptr)addrBuf);
			}		
		}
	}

	return (FxU32*)physAddr;
}
#else
FX_ENTRY FxU32* FX_CALL 
pciMapCardMulti(FxU32  vendorId,
                FxU32  deviceId,
                FxI32  length,
                FxU32* devNum,
                FxU32  cardNum,
                FxU32 addressNum) 
{
#if macintosh
#pragma unused (deviceId, vendorId, length)
#endif
	UInt32 physAddr = 0, commandAndStatus;

	*devNum = cardNum;

	if(sMacPciDeviceList[cardNum].found && BoardMutex(cardNum, FXTRUE)) {
		OSErr theErr = ExpMgrConfigReadLong(&gRegEntryId[cardNum], (LogicalAddress) 0x10, &physAddr);
		physAddr &= ~0xF;
		
		linearAddresses[cardNum] = physAddr;
		
		theErr = ExpMgrConfigReadLong(&gRegEntryId[cardNum], (LogicalAddress) 0x04, &commandAndStatus);
		commandAndStatus |= 0x05;
		ExpMgrConfigWriteLong(&gRegEntryId[cardNum], (LogicalAddress) 0x04, commandAndStatus);
	  	//FlushProcessorCache(kCurrentAddressSpaceID, (LogicalAddress)(physAddr & 0xfffff000), 16*1024*1024);
	  	//SetProcessorCacheMode(kCurrentAddressSpaceID, (const void*)(physAddr & 0xfffff000), 16*1024*1024,
	  	//kProcessorCacheModeInhibited);
	}

	return (FxU32*)physAddr;
}
#endif

FX_ENTRY FxBool FX_CALL
pciFindMTRRMatch(FxU32 pBaseAddrs, FxU32 psz, PciMemType type, FxU32 *mtrrNum)
{
	return FXFALSE;
}

FX_ENTRY FxBool FX_CALL
pciFindFreeMTRR(FxU32 *mtrrNum)
{
	return FXFALSE;
}

FX_ENTRY FxBool FX_CALL
pciSetMTRR(FxU32 mtrrNo, FxU32 pBaseAddr, FxU32 psz, PciMemType type)
{
	return FXFALSE;
}

FX_ENTRY FxBool FX_CALL
pciSetMTRRAmdK6(FxU32 mtrrNo, FxU32 pBaseAddr, FxU32 psz, PciMemType type)
{
	return FXFALSE;
}

FX_ENTRY FxBool FX_CALL
pciSetPassThroughBase(FxU32* pBaseAddr, FxU32 baseAddrLen)
{
	return FXTRUE;
}

FX_ENTRY FxBool FX_CALL
pciLinearRangeSetPermission(const FxU32 addrBase, const FxU32 addrLen, const FxBool writeableP)
{
	return FXFALSE;
}

static MacPciDevice*
FindMacPciMacDevice(const FxU32 pciBaseAddr)
{
	MacPciDevice* retVal = NULL;
	FxU32 i;
	
	for(i = 0; i < sizeof(sMacPciDeviceList) / sizeof(MacPciDevice); i++) {
		if (sMacPciDeviceList[i].found) {
			FxU32 j;
			
			for(j = 0; j < sizeof(sMacPciDeviceList[i].pciPhysAddr) / sizeof(FxU32); j++) {
				retVal = sMacPciDeviceList + i;
				goto __foundDevice;
			}
		}
	}

__foundDevice:
	return retVal;
}
#endif

FX_ENTRY FxU8 FX_CALL 
macIOReadByte(FxU32 ioBaseAddr, FxU16 ioAddr)
{
	FxU8 retVal = 0x00;
#if 1
	FxU32 addr;
	
	addr = /* 0xfe000000 + */ ioBaseAddr + ioAddr;
	__sync();
	retVal = *(FxU8 *)addr;
	__sync();
#else	
	MacPciDevice* curDev = FindMacPciMacDevice(ioBaseAddr);
	if ((curDev != NULL) && (curDev->ioPortBaseIndex != 0xFFFFFFFFUL)) {
		__eieio();

		{
			OSErr theErr = ExpMgrIOReadByte(&curDev->regEntryId,
																			(LogicalAddress)(curDev->pciPhysAddr[curDev->ioPortBaseIndex] + ioAddr),
																			&retVal);
#if DEBUG
			if (theErr != noErr) DebugStr("\pExpMgrIOReadByte: Failed");
#endif
		}
	}
#endif	
	return retVal;
}

FX_ENTRY void FX_CALL 
macIOWriteByte(FxU32 ioBaseAddr, FxU16 ioAddr, FxU8 writeVal)
{
#if 1
	FxU32 addr;
	
	addr = /* 0xfe000000 + */ ioBaseAddr + ioAddr;
	__sync();
	*(FxU8 *)addr = writeVal;
	__sync();
#else	
	MacPciDevice* curDev = FindMacPciMacDevice(ioBaseAddr);	
	if ((curDev != NULL) && (curDev->ioPortBaseIndex != 0xFFFFFFFFUL)) {
#if 0
		*(volatile FxU8*)(curDev->pciPhysAddr[curDev->cmdBaseIndex] + ioAddr) = writeVal;
#else
		{
			OSErr theErr = ExpMgrIOWriteByte(&curDev->regEntryId,
																			 (LogicalAddress)(curDev->pciPhysAddr[curDev->ioPortBaseIndex] + ioAddr),
																			 writeVal);
#if DEBUG
			if (theErr != noErr) DebugStr("\pExpMgrIOWriteByte: Failed");
#endif
		}
#endif

		__eieio();
	}
#endif	
}
#if 0
FX_ENTRY FxU8 FX_CALL 
macIOReadWord(FxU32 ioBaseAddr, FxU16 ioAddr)
{
	MacPciDevice* curDev = FindMacPciMacDevice(ioBaseAddr);
	FxU8 retVal = 0x00;
	
	if (curDev != NULL) DebugStr("\pmacIOReadWord : Unimplemented");
	
	return retVal;
}
#endif

FX_ENTRY void FX_CALL 
macIOWriteWord(FxU32 ioBaseAddr, FxU16 ioAddr, FxU16 writeVal)
{
#if 1
	FxU32 addr;
	
	addr = /* 0xfe000000 + */ ioBaseAddr + ioAddr;
	__sync();
	*(FxU8 *)addr = (FxU8)(writeVal & 0xff);
	__sync();
	*(FxU8 *)(addr + 1) = (FxU8) ((writeVal >> 8) & 0xff);
	//__sthbrx(writeVal, (void *)addr, 0);
	__sync();
#else	
	MacPciDevice* curDev = FindMacPciMacDevice(ioBaseAddr);
	if (curDev != NULL) {
		macIOWriteByte(ioBaseAddr, ioAddr, (FxU8)(writeVal & 0xFF));
		macIOWriteByte(ioBaseAddr, (ioAddr + 0x01), (FxU8)((writeVal >> 0x08) & 0xFF));
	}
#endif	
}

FX_ENTRY FxU32 FX_CALL 
macIOReadLong(FxU32 ioBaseAddr, FxU16 ioAddr)
{
	FxU32 retVal = 0x00;	
#if 1
{
	FxU32 addr;
	
	addr = /* 0xfe000000 + */ ioBaseAddr + ioAddr;
	__sync();
	retVal = __lwbrx((void *)addr, 0);
	__sync();
#else	
	MacPciDevice* curDev = FindMacPciMacDevice(ioBaseAddr);
	if ((curDev != NULL) && (curDev->cmdBaseIndex != 0xFFFFFFFFUL)) {
		__eieio();
		{
			OSErr theErr = ExpMgrIOReadLong(&curDev->regEntryId,
																			(LogicalAddress)(curDev->pciPhysAddr[curDev->ioPortBaseIndex] + ioAddr),
																			&retVal);
#if DEBUG
			if (theErr != noErr) DebugStr("\pExpMgrIOReadLong: Failed");
#endif
		}
#endif
	}
	
	return retVal;
}

FX_ENTRY void FX_CALL 
macIOWriteLong(FxU32 ioBaseAddr, FxU16 ioAddr, FxU32 writeVal)
{
	
#if 1
	FxU32 addr;
	
	addr = /* 0xfe000000 + */ ioBaseAddr + ioAddr;
	__sync();
	__stwbrx(writeVal, (void *)addr, 0);
	__sync();
#else	
	MacPciDevice* curDev = FindMacPciMacDevice(ioBaseAddr);
	if ((curDev != NULL) && (curDev->cmdBaseIndex != 0xFFFFFFFFUL)) {
		{
			OSErr theErr = ExpMgrIOWriteLong(&curDev->regEntryId,
																			 (LogicalAddress)(curDev->pciPhysAddr[curDev->ioPortBaseIndex] + ioAddr),
																			 writeVal);
#if DEBUG
			if (theErr != noErr) DebugStr("\pExpMgrIOReadLong: Failed");
#endif
		}

		__eieio();
	}
#endif
}
#endif