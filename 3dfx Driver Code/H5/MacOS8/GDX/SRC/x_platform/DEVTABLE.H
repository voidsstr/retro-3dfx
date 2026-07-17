/* -*-c++-*- */
/* $Header: devtable.h, 3, 10/11/00 8:34:31 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File name:   devtable.h
**
** Description: devtable structure, externals, and macros.
**
**
*/

#ifndef _DEVTABLE_H_
#define _DEVTABLE_H_

#include "macos8shim.h"
#include "h3g.h"
#include "sliaa.h"


typedef struct DEVTABLE *PDEVTABLE;
typedef struct DEVTABLE
{
  FxU32 IoBase;
  FxU32 dwVendorDeviceID;
  FxU32 dwType;
  FxU32 dwBus;
  FxU32 dwUnitNum;
  FxU32 dwDevFunc;
  FxU32 dwPCI;
  FxU32 lpDriverData;
  FxU32 RegBase[4];
  FxU32 PhysMemBase[4]; // ??
  FxU32 LfbBase;
  PDEVTABLE pSlave;
} DEVTABLE;

#if 0






#include "i2c/ds_i2c.h"
//#include "dfpdisp.h"

#define FIELDOFFSET(type, field) ((DWORD)(&((type *)0)->field))

#define POWERREGSIZE (12)
#define SGRAMMODE_INDEX (0x03)
#define DEFAULT_SGRAMMODE (0x37)
#define H3_DRAMMODE_REG (0x10d)
#define H3_DRAMMASK_REG (0x10e)
#define DEFAULT_SGRAMMASK (0xFFFFFFFF)
#define H3_DRAMCOLOR_REG (0x10f)
#define DEFAULT_SGRAMCOLOR (0x0)
#ifdef SLI_AA
#define VGAINIT0_INDEX (0x0a)
#define VGAINIT1_INDEX (0x0b)
#define POWERPCISIZE (5)
#endif

#ifdef POWER_TABLE
DWORD PowerRegOffset[POWERREGSIZE]={
   FIELDOFFSET(SstIORegs, dramInit1),
   FIELDOFFSET(SstIORegs, dramInit0),
   FIELDOFFSET(SstIORegs, miscInit0),
   FIELDOFFSET(SstIORegs, dramData),
   FIELDOFFSET(SstIORegs, miscInit1),
   FIELDOFFSET(SstIORegs, agpInit),
   FIELDOFFSET(SstIORegs, pllCtrl1),
   FIELDOFFSET(SstIORegs, pllCtrl2),
   FIELDOFFSET(SstIORegs, pciInit0),
   FIELDOFFSET(SstIORegs, tmuGbeInit),
   FIELDOFFSET(SstIORegs, vgaInit0),
   FIELDOFFSET(SstIORegs, vgaInit1),
   };

#ifdef SLI_AA
// FIX_ME this table also needs to have the decode adjust
DWORD PowerPCIOffset[POWERPCISIZE]={
   SST_PCI_MMIO_ID,
   SST_PCI_FB_ID,
   SST_PCI_IOBASE_ID,
   SST_PCI_COMMAND_ID,
   CFG_PCI_DECODE,
   };
#endif

#else
extern DWORD PowerRegOffset[POWERREGSIZE];
#endif



typedef struct devtable {
   // New Stuff to Manage Table with
   DWORD dwDevNode;
   DWORD dwVendorDeviceID;
#ifdef SLI_AA
   DWORD dwBus;
   DWORD dwDevFunc;
   DWORD dwUnitNum;
   DWORD dwType;
   struct devtable *pSlave;
   DWORD SliaaLfbBase;
   DWORD nPages;
   DWORD dwDDCCount[4];       // One for Each Potential Request
   DWORD PCISaveSpace[POWERPCISIZE];
#endif
   DWORD dwIMask;             // Current Interrupt Mask
   // Taken from h3irq.c
   HIRQ hIRQ;              
   VID IRQDesc;
   DWORD dwVCount;
   BOOL bVMInts;
   // Taken from h3vdd.c
   DWORD	IoBase;			// base of PCI relocatable IO space
   DWORD	PhysMemBase[2];		// physical register address space
   DWORD	RegBase[HWINFO_SST_MAX_BASE_INDEX];                // CPU linear address mapping to PhysMemBase[0]
   DWORD	LfbBase;                // CPU linear address mapping to PhysMemBase[1]
#ifdef WIN_CSIM
   DWORD	RealRegBase;                // CPU linear address mapping to PhysMemBase[0]
   DWORD	RealLfbBase;                // CPU linear address mapping to PhysMemBase[1]
   DWORD	FakeRegBase;                // CPU linear address mapping to Fake PhysMemBase[0]
   DWORD	FakeLfbBase;                // CPU linear address mapping to Fake PhysMemBase[1]
#endif
   DWORD   MemSizeInMB;		// # of megabytes of board memory
   DWORD lfbMemoryConfig[2];  // lfbMemoryConfig
   BOOL	InterruptsEnabled;
   BOOL	bIsVGA;
   FxU32 GdiDesktopEnabled;
   CMCONFIG        ConfigData;              // resources of board
   DISPLAYINFO     DispInfo;                // data from main VDD
   FxU32           Mtrr;                    // Memory Type range register
#ifdef VDDONLY
   DWORD          lpDriverData;          // Driver Data on a Per Device Basis  
#else
   GLOBALDATA      * lpDriverData;          // Driver Data on a Per Device Basis  
#endif
   DWORD dwPowerReg[POWERREGSIZE]; 
#ifdef SLI_AA
   VidProcConfig   Vpc;                 // Save the Video Mode for Future PlayBack
   CONF_SAVE SaveMem;                     // Save Configuration information so that we can later restore it
#endif
   BYTE bDPMSState;   
   BYTE bPowerState;                      // ACPI Adapter Last Power State
   BYTE bMonitorState;                    // ACPI Monitor State
   BYTE bVideoProcEn;                     // State of Video Proc Enable at D0
   BYTE bScratchPad[4];                   // "4" Bios scratchpad registers
   BYTE tvOutActive;                      // True if desktop is on TV out
   DWORD tvOutData[2];                    // Brooktree regs not readable so we
                                          // need to store state somewhere!
   BYTE bBIOSVersion[MAX_BIOS_VERSION_STRING]; // This is the BIOS Version String
   DWORD dwSubSystemID;                   // PCI2C[31:0]
   void *tvOutFn;                         // functions ptr for tvout calls
   DWORD nvramCacheInit;                  // boolean for nvram cache initialization
   BYTE nvramCache[128];                  // reading the real nvram is too slow
   DWORD tvPixelClock;                    // needed for Macrovision 7.1 compliance
   DWORD dwAllowCRTwithTV;                // allow CRT to be on when TV is on  (user interface)
   DWORD dwAllowPALandCRT;                // allow CRT to be on when PAL TV is on (feature lock out)
#ifndef OLDI2C  //[
   I2C_DEVINFO i2cInfo;                   // I2C bus configuration and state information
#endif //] !OLDI2C
   DFP_DISPLAY dfpDisp;                   // DFP display info and standard mode list
   DWORD biosBoardConfigInfo;
}  DEVTABLE, * PDEVTABLE;

extern DEVTABLE DevTable[];
extern PDEVTABLE pVGADevTable;
extern DWORD dwNumDevices;


#ifdef SLI_AA
PDEVTABLE InitDevNode(DWORD dwDevNode, DWORD dwUnitNum);
#else
PDEVTABLE InitDevNode(DWORD dwDevNode);
#endif
PDEVTABLE FindPDEVFromDevNode(DWORD dwDevNode);
#ifdef SLI_AA
void ClearAllSlaveBits(PDEVTABLE pDev);
void SetAllSlaveBits(PDEVTABLE pDev);
PDEVTABLE FindPDEVFromUnitNumber(DWORD dwUnitNumber);
PDEVTABLE FindPDEVFromDevNodeandUnitNum(DWORD dwDevNode, DWORD dwUnitNumber);
SstIORegs * DevNodeandUnitNumtoIORegs(DEVNODE devnode, DWORD dwUnitNum);
#endif
PDEVTABLE FindActiveBanshee(void);
#ifdef SLI_AA
PDEVTABLE FindActiveBansheewithCount(int nRequest);
BYTE * SwapPhysFB(PDEVTABLE pDev, DWORD dwOffset, DWORD dwOldSize, DWORD dwNewSize);
#endif
SstIORegs * DevNodetoIORegs(DEVNODE devnode);
void MapDeviceRegisters(PDEVTABLE pDev, DWORD dwSize);
void MapDeviceFB(PDEVTABLE pDev, DWORD * dwSize);


#endif





#endif
