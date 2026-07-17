/* -*-c++-*- */
/* $Header: devtable.h, 14, 10/11/00 8:54:05 PM, Brent$ */
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
** $Revision: 14$
** $Date: 10/11/00 8:54:05 PM$
**
** $History: devtable.h $
** 
** *****************  Version 31  *****************
** User: Rbissell     Date: 9/07/99    Time: 2:15p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Deployed the new modularized I2C code.
** 
** *****************  Version 30  *****************
** User: Rbissell     Date: 8/07/99    Time: 3:13p
** Updated in $/devel/h5/Win9x/dx/minivdd
** tvout merge from V3_OEM_100
** 
** *****************  Version 29  *****************
** User: Andrew       Date: 7/26/99    Time: 5:57p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to sparsely Map for Napalm and Multi-Chip and to Read
** lfbMemoryCOnfig right for Napalm
** 
** *****************  Version 28  *****************
** User: Andrew       Date: 7/16/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed regBase and RegBase from single dword to array to support
** sparse register mapping
** 
** *****************  Version 27  *****************
** User: Andrew       Date: 7/09/99    Time: 4:22p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added some new function prototypes and a array to save PCI Space on
** Slaves
** 
** *****************  Version 26  *****************
** User: Andrew       Date: 7/07/99    Time: 2:42p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added data type and calls for DDC workarounds
** 
** *****************  Version 25  *****************
** User: Andrew       Date: 6/25/99    Time: 10:01a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added two new constructions for SLI/AA
** 
** *****************  Version 24  *****************
** User: Andrew       Date: 6/15/99    Time: 5:09p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Numerous changes to support SLI AA
** 
** *****************  Version 23  *****************
** User: Andrew       Date: 6/04/99    Time: 4:13p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to support UnitNumbers
** 
** *****************  Version 21  *****************
** User: Andrew       Date: 5/14/99    Time: 1:34p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Real,Fake Membase 0,1 to make management of these easier
** 
** *****************  Version 20  *****************
** User: Andrew       Date: 5/13/99    Time: 4:13p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed Real to Fake on Lfb to more accurate reflect its use
** 
** *****************  Version 19  *****************
** User: Andrew       Date: 5/06/99    Time: 4:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code for New Windows "C" Simulator
** 
** *****************  Version 18  *****************
** User: Andrew       Date: 3/18/99    Time: 3:49p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added a field to record lfbMemoryConfig at Boot
** 
** *****************  Version 17  *****************
** User: Stuartb      Date: 3/11/99    Time: 2:45p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added DEVTABLE member tvOutFn, tvout function pointer to encoder
** specific calls
** 
** *****************  Version 16  *****************
** User: Stuartb      Date: 2/25/99    Time: 10:47a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added member dwSubSystemID.
** 
** *****************  Version 15  *****************
** User: Andrew       Date: 1/29/99    Time: 10:46a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added New table to save low power registers in
** 
** *****************  Version 14  *****************
** User: Jw           Date: 1/22/99    Time: 4:55p
** Updated in $/devel/h3/Win95/dx/minivdd
** Change pciSetMTRRAmdK6 to pciSetAmdK6MTRR to resolve name conflict in
** some other driver header file.
** 
** *****************  Version 13  *****************
** User: Jw           Date: 1/22/99    Time: 3:53p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add AMD K6/K7 MTRR support.
** 
** *****************  Version 12  *****************
** User: Andrew       Date: 1/21/99    Time: 5:45p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added New field to keep track of Interrupt Mask
** 
** *****************  Version 11  *****************
** User: Michael      Date: 1/07/99    Time: 1:27p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 10  *****************
** User: Andrew       Date: 1/05/99    Time: 10:39a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added some defines and a new entry to store bios version string in
** 
** *****************  Version 9  *****************
** User: Andrew       Date: 11/24/98   Time: 8:34a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added a VendorDevice ID field
** 
** *****************  Version 8  *****************
** User: Stuartb      Date: 10/22/98   Time: 10:29a
** Updated in $/devel/h3/Win95/dx/minivdd
** Alloc'd another DWORD for tvout data.  We don't need this now but we
** will
**  later.
** 
** *****************  Version 7  *****************
** User: Andrew       Date: 9/23/98    Time: 10:59p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added two byte fields to keep track of Monitor State and Video
** Processor State
** 
** *****************  Version 6  *****************
** User: Stuartb      Date: 9/10/98    Time: 2:44p
** Updated in $/devel/h3/Win95/dx/minivdd
** TVOUT work in prog, added 5 bytes in devtable for same.
** 
** *****************  Version 5  *****************
** User: Andrew       Date: 6/23/98    Time: 7:12a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added new fields to support power state and scratchpad registers
** 
** *****************  Version 4  *****************
** User: Andrew       Date: 6/07/98    Time: 8:44a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added new byte field to track DPMS state
** 
** *****************  Version 3  *****************
** User: Andrew       Date: 4/28/98    Time: 2:35p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Mtrr field
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#ifndef _DEVTABLE_H_
#define _DEVTABLE_H_

#include "h3g.h"
#include "sliaa.h"

#include "i2c/ds_i2c.h"

#include "dfpdisp.h"
#include "ddcdefs.h"

#define MAX_BANSHEE_DEVICES (16)
#define BIOS_STRING_1 (0x73726556)
#define BIOS_STRING_2 (0x206E6F69)

#define FIELDOFFSET(type, field) ((DWORD)(&((type *)0)->field))

#define POWERREGSIZE (12)
#define SGRAMMODE_INDEX (0x03)
#define DEFAULT_SGRAMMODE (0x37)
#define DEFAULT_RESET_DDRMODE (0x131)
#define DEFAULT_DDRMODE (0x31)
#define DEFAULT_EXTENDED_MODE (0x2001)
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

#define MEMTYPE_SDRAM (0x0)
#define MEMTYPE_DDR (0x1)

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
#ifdef RD_ABORT_ERROR
   SLI_AA_REQUEST SLIAARequest;
#endif
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
   DWORD dwVIACoreLogic;
   DWORD dwMemoryBus;         // 0=SDRAM/SGRAM;1=DDR
   DWORD dwPCI;               // True if PCI card else FALSE for AGP
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
   BYTE bOEMBoardName[MAX_BIOS_VERSION_STRING]; // This is the Board Name String
   BYTE bChipName[MAX_BIOS_VERSION_STRING]; // This is the Chip Name String
   DWORD dwSubSystemID;                   // PCI2C[31:0]
   void *tvOutFn;                         // functions ptr for tvout calls
   DWORD nvramCacheInit;                  // boolean for nvram cache initialization
   BYTE nvramCache[128];                  // reading the real nvram is too slow
   DWORD tvPixelClock;                    // needed for Macrovision 7.1 compliance
   DWORD dwAllowCRTwithTV;                // allow CRT to be on when TV is on  (user interface)
   DWORD dwAllowPALandCRT;                // allow CRT to be on when PAL TV is on (feature lock out)
   I2C_DEVINFO i2cInfo;                   // I2C bus configuration and state information
   DFP_DISPLAY dfpDisp;                   // DFP display info and standard mode list
   DWORD dwDFPState;                      // The flat panel state
                                          //      B0 - Connected
                                          //      B1 - Active
   DWORD biosBoardConfigInfo;
   BYTE bDfpActiveAtBoot;
   BYTE dfpEdidCache[A2_EDID_SIZE];                    // This is a copy of the DFP EDI information.
}  DEVTABLE, * PDEVTABLE;

extern DEVTABLE DevTable[];
extern PDEVTABLE pVGADevTable;
extern DWORD dwNumDevices;

FxBool pciSetAmdK6MTRR(FxU32 mtrrNum, void *physBaseAddr, FxU32 physSize, PciMemType type);
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
DWORD DoBusType(DWORD dwDevNode);

#endif
