/* -*-c++-*- */
/* $Header: devtable.c, 32, 10/11/00 8:54:03 PM, Brent$ */
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
** File name:   devtable.c
**
** Description: Initialize the DevNode table and supporting functions.
**
** $Revision: 32$
** $Date: 10/11/00 8:54:03 PM$
**
** $History: devtable.c $
** 
** *****************  Version 58  *****************
** User: Rbissell     Date: 9/07/99    Time: 2:15p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Deployed the new modularized I2C code.
** 
** *****************  Version 57  *****************
** User: Andrew       Date: 8/31/99    Time: 9:32a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to compile for WIN_CSIM wo SLI_AA
** 
** *****************  Version 56  *****************
** User: Cwilcox      Date: 8/30/99    Time: 4:04p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Updated Napalm device IDs to cover range from 0x6 to 0xF.
** 
** *****************  Version 55  *****************
** User: Andrew       Date: 8/05/99    Time: 1:01p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added Device ID to h3SgramInit Call
** 
** *****************  Version 54  *****************
** User: Andrew       Date: 7/30/99    Time: 1:59p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed MTTR_MASK to MTRR_MASK
** 
** *****************  Version 53  *****************
** User: Andrew       Date: 7/27/99    Time: 4:38p
** Updated in $/devel/h5/Win9x/dx/minivdd
** moved sim code into own file and delayed GetNapalm call
** 
** *****************  Version 51  *****************
** User: Andrew       Date: 7/26/99    Time: 5:59p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to Sparely Map, Fix a Problem with MTRR, and Read
** lfbMemoryConfig right for Napalm
** 
** *****************  Version 50  *****************
** User: Andrew       Date: 7/20/99    Time: 10:52a
** Updated in $/devel/h5/Win9x/dx/minivdd
** added code to allow multiple chip support
** 
** *****************  Version 49  *****************
** User: Andrew       Date: 7/16/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed regBase and RegBase from single dword to array to support
** sparse register mapping
** 
** *****************  Version 48  *****************
** User: Andrew       Date: 7/09/99    Time: 4:21p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added a couple of functions to support Power Management
** 
** *****************  Version 47  *****************
** User: Cwilcox      Date: 7/08/99    Time: 4:54p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Removed unused Napalm device IDs.
** 
** *****************  Version 46  *****************
** User: Cwilcox      Date: 7/08/99    Time: 1:08p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added runtime checking for Napalm versus Voodoo3.
** 
** *****************  Version 45  *****************
** User: Andrew       Date: 7/07/99    Time: 2:42p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code for DDC workaround
** 
** *****************  Version 43  *****************
** User: Andrew       Date: 6/25/99    Time: 4:29p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added Code to setup dwType to either Master, Slave or Regular
** 
** *****************  Version 42  *****************
** User: Andrew       Date: 6/25/99    Time: 10:03a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Numerous changes to support SLI/AA
** 
** *****************  Version 41  *****************
** User: Andrew       Date: 6/15/99    Time: 5:09p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Numerous changes to support SLI AA
** 
** *****************  Version 40  *****************
** User: Andrew       Date: 6/04/99    Time: 4:13p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to support UnitNumbers
** 
** *****************  Version 38  *****************
** User: Andrew       Date: 5/19/99    Time: 3:58p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added ifdef LINEAR to pass through Real FB and Fake if we are in tiled
** mode
** 
** *****************  Version 37  *****************
** User: Andrew       Date: 5/14/99    Time: 1:34p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Real,Fake Membase 0,1 to make management of these easier
** 
** *****************  Version 36  *****************
** User: Andrew       Date: 5/13/99    Time: 4:13p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed Real to Fake on Lfb to more accurate reflect its use
** 
** *****************  Version 35  *****************
** User: Andrew       Date: 5/06/99    Time: 4:37p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code for New Windows "C" Simulator
** 
** *****************  Version 34  *****************
** User: Cwilcox      Date: 4/23/99    Time: 4:15p
** Updated in $/devel/h3/Win95/dx/minivdd
** Completed Napalm 32mb/64Mb changes.
** 
** *****************  Version 33  *****************
** User: Andrew       Date: 3/18/99    Time: 3:49p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to record lfbMemoryConfig at boot
** 
** *****************  Version 32  *****************
** User: Andrew       Date: 3/10/99    Time: 4:30p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed so that Banshee VDD will not work on Avenger and visa versa
** 
** *****************  Version 31  *****************
** User: Xingc        Date: 3/06/99    Time: 1:59p
** Updated in $/devel/h3/Win95/dx/minivdd
** GetBiosVersion() only return version number without chip or memory info
** 
** *****************  Version 30  *****************
** User: Xingc        Date: 3/05/99    Time: 4:23p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fix PRS 4855, "QUERYGETBIOSVERSION not working correctly"
** 
** *****************  Version 29  *****************
** User: Stuartb      Date: 2/25/99    Time: 10:47a
** Updated in $/devel/h3/Win95/dx/minivdd
** Capture SubSystemID in InitDevNode and save in Devtable.
** 
** *****************  Version 28  *****************
** User: Stb_pzheng   Date: 2/24/99    Time: 6:04p
** Updated in $/devel/h3/win95/dx/minivdd
** Removed the code that sets tmuGbInit register so that bios can take
** care of it on its own
** 
** *****************  Version 27  *****************
** User: Andrew       Date: 1/29/99    Time: 10:45a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Code to save bios information
** 
** *****************  Version 26  *****************
** User: Jw           Date: 1/22/99    Time: 4:55p
** Updated in $/devel/h3/Win95/dx/minivdd
** Change pciSetMTRRAmdK6 to pciSetAmdK6MTRR to resolve name conflict in
** some other driver header file.
** 
** *****************  Version 25  *****************
** User: Jw           Date: 1/22/99    Time: 3:52p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add AMD K6/K7 MTRR support.
** 
** *****************  Version 24  *****************
** User: Michael      Date: 1/07/99    Time: 1:27p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 23  *****************
** User: Andrew       Date: 1/05/99    Time: 10:38a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Function GetBIOSVersion to read the version string out of the
** bios.
** 
** *****************  Version 22  *****************
** User: Andrew       Date: 12/09/98   Time: 5:26p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added H4_OEM device ID check
** 
** *****************  Version 21  *****************
** User: Andrew       Date: 11/24/98   Time: 8:33a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added Code to save the VendorDeviceID in the device table
** 
** *****************  Version 20  *****************
** User: Andrew       Date: 11/16/98   Time: 8:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to support Avenger
** 
** *****************  Version 19  *****************
** User: Andrew       Date: 10/26/98   Time: 9:35p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added the Win '98 calls to the MTRR routine to bring inline with GLOP
** 
** *****************  Version 18  *****************
** User: Ken          Date: 10/16/98   Time: 11:14a
** Updated in $/devel/h3/win95/dx/minivdd
** mirroring GLOP changes into TOT for clock change policy (don't touch
** PLLs at boot by default, they still can be tweaked from the registry
** though), and for the new "Hank" memory timing rules (dramInit1 bit 14
** is speed dependent), and memClock and grxClock registry keys are now
** string keys
** 
** *****************  Version 17  *****************
** User: Andrew       Date: 9/23/98    Time: 10:58p
** Updated in $/devel/h3/Win95/dx/minivdd
** Inited bPowerState and bMonitorState to D0
** 
** *****************  Version 16  *****************
** User: Andrew       Date: 8/22/98    Time: 12:06p
** Updated in $/devel/h3/Win95/dx/minivdd
** Removed set of VGA Legacy Bit for 2nd monitor as it was breaking DDC
** and reversed order of clear of MTRR
** 
** *****************  Version 15  *****************
** User: Ken          Date: 7/17/98    Time: 3:37p
** Updated in $/devel/h3/win95/dx/minivdd
** separated default clocks into independent default values settable from
** environment variables (GCLK for graphics clock), (MCLK for memory
** clock)
** 
** *****************  Version 2  *****************
** User: Ken          Date: 7/17/98    Time: 3:24p
** Updated in $/Releases/Banshee/A2_Merc/3dfx/devel/h3/Win95/dx/minivdd
** added separate graphics and memory clock defaults, settable in build
** from environment variables GCLK (graphics clock) and MCLK (memory
** clock)
** 
** *****************  Version 1  *****************
** User: Admin        Date: 7/16/98    Time: 2:49p
** Created in $/Releases/Banshee/A2_Merc/3dfx/devel/h3/Win95/dx/minivdd
** 
** *****************  Version 14  *****************
** User: Andrew       Date: 7/14/98    Time: 1:32p
** Updated in $/devel/h3/Win95/dx/minivdd
** Modified to check if IRQ has changed before supressing init
** 
** *****************  Version 13  *****************
** User: Andrew       Date: 7/14/98    Time: 9:35a
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed InitDevTable to check if DevNode has already been initialized
** to same physical address before blinding performing the init
** 
** *****************  Version 12  *****************
** User: Ken          Date: 7/10/98    Time: 9:00p
** Updated in $/devel/h3/win95/dx/minivdd
** memory / clock timing default changes.  default grxclock and memclock
** timings are now settable at compile time.   temporarily, set HR=B0 to
** compile for A2/A3 for minivdd only.  dramInit0 is now not touched in
** the win95 init sequence, unless there's an override in the registry
** 
** *****************  Version 11  *****************
** User: Ken          Date: 7/07/98    Time: 2:16p
** Updated in $/devel/h3/win95/dx/minivdd
** now look up to MTRR 20e/20f, and mark 16MB as write combine instead of
** just 4mb
** 
** *****************  Version 10  *****************
** User: Ken          Date: 7/02/98    Time: 6:23p
** Updated in $/devel/h3/win95/dx/minivdd
** added magic parameter values to h3InitSGRAM so that windows boot can
** skip the writing of the SGRAM registers
** 
** *****************  Version 9  *****************
** User: Andrew       Date: 7/01/98    Time: 10:25a
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed InitDevNode to update the bios variable in IoBase+0xD4 to a
** possible new IObase.  This is a problem since the bios gets at boot but
** may change at a latter time.
** 
** *****************  Version 8  *****************
** User: Andrew       Date: 6/23/98    Time: 7:11a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added new field to track power mgmt. Added new field to save scratchpad
** registers
** 
** *****************  Version 7  *****************
** User: Andrew       Date: 6/07/98    Time: 8:43a
** Updated in $/devel/h3/Win95/dx/minivdd
** Init new byte field for DPMS
** 
** *****************  Version 6  *****************
** User: Andrew       Date: 5/27/98    Time: 8:47a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed a problem with count of devices
** 
** *****************  Version 5  *****************
** User: Andrew       Date: 4/28/98    Time: 2:35p
** Updated in $/devel/h3/Win95/dx/minivdd
** Took out a check for already initialized devnode since windows was
** moving the physical address which was causing me some problems.
** 
** *****************  Version 4  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
** 
** *****************  Version 3  *****************
** User: Ken          Date: 4/09/98    Time: 10:20p
** Updated in $/devel/h3/win95/dx/minivdd
** updated to phil's new init code
**
*/

#define WIN40COMPAT
#include "h3vdd.h"
#include "h3.h"

#define VDDONLY
#include "h3g.h"
#include "h3cinitdd.h"
#include "devtable.h"
#include "h3irq.h"
#ifndef WINNT
#include "shared.h"	// NEW CPUID CODE
#endif
#undef  VDDONLY

#include "p6stuff.h"
#include "h3cinit.h"

#include "i2c/di_i2c.h"
#include "bios.h"

// Fix for building with Windows 98 DDK (Needs BOOLEAN type)
typedef BYTE BOOLEAN;
#undef WANTVXDWRAPS
#include <mtrr.h>

// Fix for building with Windows 98 DDK (VxDWraps already has wrappers)
#undef _PageReserve
#undef _PageCommitPhys
#undef _PageDecommit

#pragma VxD_LOCKED_DATA_SEG
DEVTABLE DevTable[MAX_BANSHEE_DEVICES];
PDEVTABLE pVGADevTable;
DWORD dwNumDevices;

#ifdef WINNT	// OLD CPUID CODE
FxU32 isP6;
#else
CPU_FEATURES cpuFeatures;
#endif

#ifdef SLI_AA
#define MEM_DECODE_SIZE (0x8000000) 
#define IO_DECODE_SIZE (0x100)
#endif

#ifdef WIN_CSIM
DWORD GetWINSIMIface(PDEVTABLE pDevTable, DWORD dwUnitNum);         
#endif

DWORD SparseMemBase0Map[] =
   {
   SST_IO_OFFSET,
   SST_CMDAGP_OFFSET,
   SST_2D_OFFSET,
   SST_3D_OFFSET,
   };

DWORD SparseMemBase0Size[] =
   {
   sizeof(SstIORegs),
   sizeof(SstCRegs),
   sizeof(SstGRegs),
   sizeof(SstRegs),
   };

#pragma VxD_ICODE_SEG

void InitSecondaryDispatchTable(DWORD dwDevNode);
void InitPower(void);
 
#pragma VxD_LOCKED_CODE_SEG

/*----------------------------------------------------------------------
Function name:  MTRRSetPhysical

Description:    Call the Win '98 MTRR Routines.  Don't use the
DDK one as it does not save ECX which can cause strange errors

Information:    

Return:  Unused       
----------------------------------------------------------------------*/
ULONG VXDINLINE MTRRSetPhysical(PVOID PhysicalAddress, ULONG NumberOfBytes, MEMORY_CACHING_TYPE CacheType) 
{

   ULONG        ulResult;

   _asm pushad
   _asm push CacheType;
   _asm push NumberOfBytes;
   _asm push 0;
   _asm push PhysicalAddress;
   VxDCall(MTRRSetPhysicalCacheTypeRange);
   _asm mov [ulResult], eax;

   _asm popad   
   return (ulResult);
}

/*----------------------------------------------------------------------
Function name:  PNP_NewDevNode

Description:    Start of PNP_NewDevNode

Information:    

Return:         CONFIGRET   CR_SUCCESS is always returned.
----------------------------------------------------------------------*/
CONFIGRET _cdecl
PNP_NewDevNode(DWORD dwCommand, DEVNODE dwDevNode, DWORD dwLoadtype)
{
   PDEVTABLE pDev;
   Debug_Printf("Command %x DevNode %x loadtype %x\n", dwCommand, dwDevNode, dwLoadtype);
   
#ifdef SLI_AA
   pDev = InitDevNode(dwDevNode, 0x0);
#else
   pDev = InitDevNode(dwDevNode);
#endif

   Debug_Printf("Init Secondary Dispatch Table\n");

   InitSecondaryDispatchTable(dwDevNode);

   return (CR_SUCCESS);   
}

#define MAP_MTRR(size) (~(size - 1) | 0x0800)

void our_outp(DWORD addr, DWORD dwData);



/*----------------------------------------------------------------------
Function name:  VDD_Get_Mini_Dispatch_Table

Description:    Get the Dispatch Table from VDD.

Information:    Uses in-line asm.

Return:         DWORD   value returned from the VxDCall.
----------------------------------------------------------------------*/
DWORD VXDINLINE
VDD_Get_Mini_Dispatch_Table(DWORD **pDT)
{
    DWORD dwR;

    __asm pushad;
    VxDCall(VDD_Get_Mini_Dispatch_Table);
    __asm mov dwR,ecx;
    __asm mov eax,pDT;
    __asm mov [eax],edi;
    __asm popad;
    return(dwR);
}


/*----------------------------------------------------------------------
Function name: GetDecodeSize

Description:    Return Decode Size

Information:    

Return: MemBase0/MemBase1 Decode Size
----------------------------------------------------------------------*/
DWORD GetDecodeSize(DWORD dwSize)
{
   DWORD dwReturn;

   switch (dwSize >> 20)
      {
      case 4:
         dwReturn = CFG_MEMBASE0_4MB_DECODE;
         break;

      case 8:
         dwReturn = CFG_MEMBASE0_8MB_DECODE;
         break;

      case 16:
         dwReturn = CFG_MEMBASE0_16MB_DECODE;
         break;

      case 32:
         dwReturn = CFG_MEMBASE0_32MB_DECODE;
         break;

      case 64:
         dwReturn = CFG_MEMBASE0_64MB_DECODE;
         break;

      case 128:
         dwReturn = CFG_MEMBASE0_128MB_DECODE;
         break;

      case 256:
         dwReturn = CFG_MEMBASE0_256MB_DECODE;
         break;

      case 512:
         dwReturn = CFG_MEMBASE0_512MB_DECODE;
         break;

      case 1024:
         dwReturn = CFG_MEMBASE0_1024MB_DECODE;
         break;

      default:
         dwReturn = CFG_MEMBASE0_128MB_DECODE;
         break;   
      }

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name: GetMemSize

Description:    Return Memory Size

Information:    

Return: MemBase0/MemBase1 Memory Size
----------------------------------------------------------------------*/
DWORD GetMemSize(DWORD dwSize)
{
   DWORD dwReturn;

   switch (dwSize)
      {
      case CFG_MEMBASE0_4MB_DECODE:
         dwReturn = 4;
         break;

      case CFG_MEMBASE0_8MB_DECODE:
         dwReturn = 8;
         break;

      case CFG_MEMBASE0_16MB_DECODE:
         dwReturn = 16;
         break;

      case CFG_MEMBASE0_32MB_DECODE:
         dwReturn = 32;
         break;

      case CFG_MEMBASE0_64MB_DECODE:
         dwReturn = 64;
         break;

      case CFG_MEMBASE0_128MB_DECODE:
         dwReturn = 128;
         break;

      case CFG_MEMBASE0_256MB_DECODE:
         dwReturn = 256;
         break;

      case CFG_MEMBASE0_512MB_DECODE:
         dwReturn = 512;
         break;

      case CFG_MEMBASE0_1024MB_DECODE:
         dwReturn = 1024;
         break;

      default:
         dwReturn = 128;
         break;   
      }

   return (dwReturn << 20);
}

/*----------------------------------------------------------------------
Function name:  DoBusType

Description:    Deterimine Bus Type --> AGP or PCI based on strapping
   options.

Information:    

Return:         TRUE --> PCI or unstrapped AGP
                FALSE --> AGP
----------------------------------------------------------------------*/
DWORD DoBusType(DWORD dwDevNode)
{
   BYTE PCIConfSpace[256];
   DWORD dwReturn = TRUE;
   int nCapIndex;
   
   CM_Call_Enumerator_Function(dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_DEVICE_ID, &PCIConfSpace, sizeof(PCIConfSpace), 0);

   for (nCapIndex = PCIConfSpace[SST_1ST_CAP]; 0x0 != nCapIndex; nCapIndex=PCIConfSpace[nCapIndex+1])
      {
      if (SST_AGP_CAP == PCIConfSpace[nCapIndex])
         {
         dwReturn = FALSE;
         break;
  	      } 
      }
         
 
   return dwReturn;
}

/*----------------------------------------------------------------------
Function name:  InitDevNode

Description:    Initialize the dev node.

Information:    

Return:         PDEVTABLE   pDev if success or,
                            NULL for failure.
----------------------------------------------------------------------*/
#ifdef SLI_AA
extern DWORD dwWin98;
DWORD dwDecodeSize[]={0x0, 0x0};
#endif
PDEVTABLE 
#ifdef SLI_AA
InitDevNode(DWORD dwDevNode, DWORD dwUnitNum)
#else
InitDevNode(DWORD dwDevNode)
#endif
{
   PDEVTABLE pDev = NULL;
   SstIORegs *lpIOregs;
   DWORD memBase;
   DWORD mapSize;
   DWORD IoBase;			// base of PCI relocatable IO space
   DWORD PhysMemBase[2];	// physical register address space
   FxU32 grxSpeedInMHz;
   FxU32 memSpeedInMHz;
   FxU32 sgramMode, sgramMask, sgramColor;
   FxU32 mtrr1Lo = 0, mtrr1Hi = 0, mtrr2Lo = 0, mtrr2Hi = 0;
   FxU32 foundMTRR;
   PDWORD pDispatch;
   DWORD dwNFunc;
   int i;
   int j;
   int nFound;
#ifdef SLI_AA
   SstIORegs * pMasterIORegs;   
   SstIORegs * pSlaveIORegs;   
   BYTE * pMasterRegs;
   BYTE * pSlaveRegs;
   DWORD dwVgaInit0;
   PDEVTABLE pBackDev;
   PDEVTABLE pNextSlave;
   PDEVTABLE pSaveSlave;
   int k;
   DWORD cfgInitEnable;
   DWORD McfgPCIDecode;
   DWORD ScfgPCIDecode;
   DWORD nChips;
#endif
   FxU32 pciInit0;
   DWORD dwMemoryType;

   nFound = FALSE;
   for (i=0; i< MAX_BANSHEE_DEVICES; i++)
      {
#ifdef SLI_AA
      if ((dwDevNode == DevTable[i].dwDevNode) && (dwUnitNum == DevTable[i].dwUnitNum))
#else
	   if (dwDevNode == DevTable[i].dwDevNode)
#endif
         {
         nFound = TRUE;
	      break;
         }  
	   if (0x0 == DevTable[i].dwDevNode)
	      {
	      dwNumDevices++;
	      break; 
	      }
      }

   if (MAX_BANSHEE_DEVICES != i)
      {
	   pDev = &DevTable[i];
	   DevTable[i].dwDevNode = dwDevNode;
#ifdef SLI_AA
	   DevTable[i].dwUnitNum = dwUnitNum;
#endif
	   DevTable[i].lpDriverData = NULL;
	   DevTable[i].bDPMSState = 0x0;
	   DevTable[i].bPowerState = CM_POWERSTATE_D0;
	   DevTable[i].bMonitorState = CM_POWERSTATE_D0;
	   // get display data from VDD
	   //
	   VDD_Get_DISPLAYINFO(&DevTable[i].DispInfo, sizeof(DevTable[i].DispInfo) );

   	Debug_Printf("Are they the same DevNode %x DevNode %x %x\n", dwDevNode, DevTable[i].DispInfo.diDevNodeHandle, DevTable[i].DispInfo.diInfoFlags);
#ifdef SLI_AA
      if (0x00 != dwUnitNum)
         {
         pDev->DispInfo.diDevNodeHandle = dwDevNode;
         pDev->DispInfo.diInfoFlags |= DEVICE_IS_NOT_VGA;
         }
#endif
   	// now we have the devnode, ask for the resources from
	   // Config manager
	   //
	   CM_Get_Alloc_Log_Conf( &DevTable[i].ConfigData, dwDevNode, 0 );

#ifdef SLI_AA
	   DevTable[i].SliaaLfbBase = NULL;
#endif

	   // and ask the PCI manager for the device ID
	   //

	   CM_Call_Enumerator_Function (dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_DEVICE_ID, &DevTable[i].dwVendorDeviceID, sizeof(DWORD), 0); 
   	CM_Call_Enumerator_Function (dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, SST_PCI_SSID, &DevTable[i].dwSubSystemID, sizeof(DWORD), 0); 

      if ((!IS_VOODOO3(DevTable[i].dwVendorDeviceID)) && (!IS_NAPALM(DevTable[i].dwVendorDeviceID)))
	      {
	      Debug_Printf( VNAME "Invalid Device ID 0x%x, expecting 0x%x or 0x%x-0x%x\n", DevTable[i].dwVendorDeviceID, SST_VENDOR_DEVICE_ID_H4, SST_VENDOR_DEVICE_ID_H5_6, SST_VENDOR_DEVICE_ID_H5_F );
         DevTable[i].dwDevNode = 0x0;
	      return(NULL);
	      }   

   	//
	   // find the PCI relocatable memory address space bases
	   //
	   j = 0;
	   memBase = 0;
#ifdef SLI_AA
      // Determine Size and Park Position
      CM_Call_Enumerator_Function (dwDevNode, PCI_ENUM_FUNC_GET_DEVICE_INFO, CFG_PCI_DECODE, &McfgPCIDecode, sizeof(DWORD), 0); 
      dwDecodeSize[0] = GetMemSize((McfgPCIDecode & CFG_MEMBASE0_DECODE) >> CFG_MEMBASE0_DECODE_SHIFT); 
      dwDecodeSize[1] = GetMemSize((McfgPCIDecode & CFG_MEMBASE1_DECODE) >> CFG_MEMBASE1_DECODE_SHIFT); 
#endif
	   while (j < DevTable[i].ConfigData.wNumMemWindows)
	      {
	      if (DevTable[i].ConfigData.dMemBase[j] < (1024 * 1024))
            {
		      j += 1;
		      continue;
            }

#ifdef SLI_AA
         // MemBase 0 need to be unique
         // MemBase 1 don't need to be unique for the slaves
         if (0 == memBase)
            PhysMemBase[memBase] = DevTable[i].ConfigData.dMemBase[j++] + dwUnitNum * dwDecodeSize[memBase];
         else
            {
            if (0 == dwUnitNum)
               PhysMemBase[memBase] = DevTable[i].ConfigData.dMemBase[j++];
            else   
               PhysMemBase[memBase] = DevTable[i].ConfigData.dMemBase[j++] + dwDecodeSize[memBase];
            }

         memBase++;
#else      
	      PhysMemBase[memBase++] = DevTable[i].ConfigData.dMemBase[j++];
#endif

	      if (memBase == H3_NUM_MEM_BASES)
		      break;		// found all the memory bases!
	      }

	   if (memBase != H3_NUM_MEM_BASES)
	      {
	      Debug_Printf( VNAME "Too few membases > 1MB, found %d, expecting %d\n", memBase, H3_NUM_MEM_BASES);
   	   DevTable[i].dwDevNode = 0x0;
	      return(NULL);
	      }
 
	   //
	   // find the PCI relocatable IO base
	   // 
	   j = 0;
      IoBase = 0;
	   while (j < DevTable[i].ConfigData.wNumIOPorts)
	      {
	      if (DevTable[i].ConfigData.wIOPortBase[j] < 0x400)
            {
		      j += 1;
		      continue;
            }
         IoBase = DevTable[i].ConfigData.wIOPortBase[j];
	      break;
	      }

	   if (IoBase == 0)
	      {
	      Debug_Printf( VNAME "Found no relocatable IO base\n");
	      DevTable[i].dwDevNode = 0x0;
	      return(NULL);
	      }

      // Ok, we already mapped in the resources for this node once
      // has it changed?
      if (TRUE == nFound)
         {
         if ((DevTable[i].PhysMemBase[0] == PhysMemBase[0]) &&
             (DevTable[i].PhysMemBase[1] == PhysMemBase[1]) &&
             (DevTable[i].IoBase == IoBase))
            {
            // If interrupts are disabled then return
            if (0 == DevTable[i].InterruptsEnabled)
               return pDev;

            // Fix me to we need to remove old interrupt?
            // If interrupt is the same then return
            if (DevTable[i].IRQDesc.VID_IRQ_Number == DevTable[i].ConfigData.bIRQRegisters[0])
               return pDev;
            }
         }

      for (memBase=0; memBase<H3_NUM_MEM_BASES; memBase++)
         DevTable[i].PhysMemBase[memBase] =  PhysMemBase[memBase];
      DevTable[i].IoBase = IoBase;      

#ifdef SLI_AA
      PCIGetBusDevFunc(dwDevNode, &DevTable[i].dwBus, &DevTable[i].dwDevFunc);

      DevTable[i].dwType = SLI_AA_REGULAR_DEVICE;

      // Multi-Function Device Case
      // If we have a function number then we are a slave
      nChips = 1;
      if (DevTable[i].dwDevFunc & 0x07)
         DevTable[i].dwType = SLI_AA_SLAVE_DEVICE;
      else
         {
#ifndef WIN_CSIM
         for (j=1; j<8; j++)
            {
            DWORD deviceID;
         
            // If we don't have slave how can we be a master?
            deviceID = PCI_Read_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc | j, 0x0);
		      if (IS_VOODOO3(deviceID) || IS_NAPALM(deviceID))
		         {
               DevTable[i].dwType = SLI_AA_MASTER_DEVICE;
               nChips++;
               }
            }
#endif
         }

      // If we have a unit number then we are a slave
      // FIX_ME This is a good place to add PCI_Decode for Both Master/Slave if this is Napalm
      if (0x0 != dwUnitNum)
         {
         DevTable[i].dwDevFunc |= dwUnitNum;

         // if we have a unit number then we are a slave      
         DevTable[i].dwType = SLI_AA_SLAVE_DEVICE;

#ifndef WIN_CSIM
         // Enable Writes regardless of Strapping Option Sizes!!!!!
         cfgInitEnable = PCI_Read_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc, CFG_INIT_ENABLE);
         PCI_Write_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc, CFG_INIT_ENABLE, cfgInitEnable | CFG_UPDATE_MEMBASE_LSBS);

         // Read Master PCI Decode
         McfgPCIDecode = PCI_Read_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc & ~0x07, CFG_PCI_DECODE);
         McfgPCIDecode = (McfgPCIDecode & (CFG_MEMBASE0_DECODE | CFG_MEMBASE1_DECODE)) << CFG_SNOOP_MEMBASE0_DECODE_SHIFT;
         
         ScfgPCIDecode = PCI_Read_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc, CFG_PCI_DECODE);
         ScfgPCIDecode &= ~(CFG_MEMBASE0_DECODE | CFG_MEMBASE1_DECODE | CFG_IOBASE_DECODE | CFG_SNOOP_MEMBASE0_DECODE | CFG_SNOOP_MEMBASE1_DECODE);
         ScfgPCIDecode = ScfgPCIDecode | McfgPCIDecode | (GetDecodeSize(32<<20) << CFG_MEMBASE0_DECODE_SHIFT) | (GetDecodeSize(dwDecodeSize[1]) << CFG_MEMBASE1_DECODE_SHIFT);
         PCI_Write_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc, CFG_PCI_DECODE, ScfgPCIDecode);

         // At this Time we need to do a little Initialization
         // Setup Membase0, MemBase1, IoBase
         PCI_Write_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc, SST_PCI_MMIO_ID, PhysMemBase[0]);      

         // Park it in Membase1 Space
         PCI_Write_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc, SST_PCI_FB_ID, PhysMemBase[1]);      

         PCI_Write_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc, SST_PCI_IOBASE_ID, IoBase);      
         PCI_Write_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc, CFG_INIT_ENABLE, cfgInitEnable);
   
         // Disable Master
         PCI_Write_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc & ~0x07, SST_PCI_COMMAND_ID, 0x02);      

         // Enable Us
         PCI_Write_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc, SST_PCI_COMMAND_ID, 0x03);      
#endif      

         }
#endif

      /* Map register space (memBase0). */
#ifdef CMDFIFO
	   mapSize = 0x2000000;  // register space is 32Mb fixed
#else
      mapSize = 0x8000000;  // register space is 128MB in Direct Write
#endif

#ifndef WIN_CSIM
      MapDeviceRegisters(pDev, mapSize);
	   if (DevTable[i].RegBase[0] == NULL)
	      {
	      Debug_Printf( VNAME "Unable to map memory base 0\n");
	      DevTable[i].dwDevNode = 0x0;
	      return(NULL);
	      }
#else
#ifdef SLI_AA
      if (0x0 == dwUnitNum)
         {
         MapDeviceRegisters(pDev, mapSize);
	      if (DevTable[i].RegBase[0] == NULL)
	         {
	         Debug_Printf( VNAME "Unable to map memory base 0\n");
	         DevTable[i].dwDevNode = 0x0;
	         return(NULL);
	         }
         DevTable[i].RealRegBase = DevTable[i].RegBase[0];
         }
      else
         GetWINSIMIface(pDev, dwUnitNum);
#else
      MapDeviceRegisters(pDev, mapSize);
	   if (DevTable[i].RegBase[0] == NULL)
	      {
	      Debug_Printf( VNAME "Unable to map memory base 0\n");
	      DevTable[i].dwDevNode = 0x0;
	      return(NULL);
	      }
      DevTable[i].RealRegBase = DevTable[i].RegBase[0];
#endif
#endif

      i2c_initialize((I2CCONTEXT)&DevTable[i]);

      // Determine Memory Bus Type <either DDR or SGRAM/SDRAM>
      if (IS_DAYTONA(DevTable[i].dwVendorDeviceID))
         {
         pMasterIORegs = DevNodetoIORegs(dwDevNode);
         dwMemoryType = pMasterIORegs->tmuGbeInit & SST_MCTL_MEMORY_CONFIG;
         if ((dwMemoryType >= SST_MCTL_DDR_64x256) && (dwMemoryType <= SST_MCTL_DDR_32x1024))
            DevTable[i].dwMemoryBus = MEMTYPE_DDR;
         }

#ifdef SLI_AA
      // Now that we have a Memory Mapped Register's Init the Device if it 
      // has not been
      if (0x0 != dwUnitNum)
         {
         pMasterIORegs = DevNodetoIORegs(dwDevNode);
         pMasterRegs = (BYTE *)pMasterIORegs;
         pSlaveIORegs = (SstIORegs *)DevTable[i].RegBase[HWINFO_SST_IOREGS_INDEX];
         pSlaveRegs = (BYTE *)pSlaveIORegs;
         for (j=0; j<POWERREGSIZE; j++)
            {
            if (SGRAMMODE_INDEX == j)
               {
               if (MEMTYPE_DDR == pDev->dwMemoryBus)
                  {
                  // Band Select and set DLL
                  pSlaveIORegs->dramData = DEFAULT_EXTENDED_MODE;
                  pSlaveIORegs->dramCommand = H3_DRAMMODE_REG;
                  pSlaveIORegs->dramData = DEFAULT_RESET_DDRMODE;
                  pSlaveIORegs->dramCommand = H3_DRAMMODE_REG;
                  pSlaveIORegs->dramData = DEFAULT_DDRMODE;
                  pSlaveIORegs->dramCommand = H3_DRAMMODE_REG;
                  }
               else
                  {
                  pSlaveIORegs->dramData = DEFAULT_SGRAMMODE;
                  pSlaveIORegs->dramCommand = H3_DRAMMODE_REG;
                  // In the BIOS we set this bit if it is SGRAM
                  if (!(pSlaveIORegs->dramInit1 & SST_MCTL_TYPE_SDRAM))
                     {
                     pSlaveIORegs->dramData = DEFAULT_SGRAMMASK;
                     pSlaveIORegs->dramCommand = H3_DRAMMASK_REG;
                     }
                  }            
               }
            else if (VGAINIT0_INDEX == j)
               {
               dwVgaInit0 = *(DWORD *)(pMasterRegs + PowerRegOffset[j]);
               dwVgaInit0 |= SST_VGA0_DISABLE_DECODE << SST_VGA0_LEGACY_DECODE_SHIFT;
               *(DWORD *)(pSlaveRegs + PowerRegOffset[j]) = dwVgaInit0;
               }
            else if (VGAINIT1_INDEX == j)
               continue;
            else            
               *(DWORD *)(pSlaveRegs + PowerRegOffset[j]) = *(DWORD *)(pMasterRegs + PowerRegOffset[j]);
            }
         // Enable Us in VGA Space
         our_outp(DevTable[i].IoBase + 0xc3, 0x01);
         our_outp(DevTable[i].IoBase + 0xc2, 0x01);
         }
#endif

	   // Here is where we need to do a little bit of fixing up
	   // The problem is that the Banshee BIOS keeps track of the
	   // IOBase in IOBase+D4[1C].  Windows in its infinite wisdom
	   // may change the value from what the system BIOS assigned at
	   // boot.  To fix this problem we need to update the value here.

	   our_outp(DevTable[i].IoBase + 0xD4, 0x1C);
	   our_outp(DevTable[i].IoBase + 0xD5, DevTable[i].IoBase >> 8);

      //??? should we turn on the hotplug interrupt before initializing DFP???
      //Enable hotplug interrupt
      if(IS_NAPALM(pDev->dwVendorDeviceID))
         pDev->dwIMask |= 0x04000000;

	   if (DevTable[i].InterruptsEnabled &&
	      (DevTable[i].ConfigData.bIRQRegisters[0] != 0))
	      {
	      Debug_Printf(VNAME "Using IRQ %d\n", DevTable[i].ConfigData.bIRQRegisters[0]);
   	   DevTable[i].InterruptsEnabled = InitializeInterrupts(DevTable[i].ConfigData.bIRQRegisters[0], &DevTable[i]);
	      }
	   else
	      {
	      Debug_Printf(VNAME "No IRQ assigned\n");
	      DevTable[i].InterruptsEnabled = 0;
	      }
 
	   //
   	// initialize h3 hardware
	   //

	   grxSpeedInMHz = H3_GRXCLOCK_IN_MHZ;

	   // quick sanity check on the clock frequency
	   //
	   if ((grxSpeedInMHz < 50) || (grxSpeedInMHz > 150))
	      grxSpeedInMHz = 100;

	   memSpeedInMHz = H3_MEMCLOCK_IN_MHZ;

	   // quick sanity check on the clock frequency
	   //
	   if ((memSpeedInMHz < 50) || (memSpeedInMHz > 150))
	      memSpeedInMHz = 100;

	   //
	   // Policy change: now the windows drivers don't touch the mem/grx
	   // PLLs at boot, whatever values are there, set by the BIOS at
	   // POST, remain there.  The registry overrides for both
	   // mem and grx are still enabled for those who wish to over/under
	   // clock.    Since this may change again, I'm keeping all the above
	   // clock code but commenting out the h3InitPlls call below

	   // h3InitPlls(DevTable[i].IoBase, grxSpeedInMHz, memSpeedInMHz);


	   // h3InitSgram won't mess with the SGRAM registers at all when we
	   // sending in these magic numbers for sgramMode and sgramMask

	   sgramMode = 0xDEADBEEF; // magic value (obviously!)
	   sgramMask = 0xF00DCAFE; // magic value (obviously!)
	   sgramColor = 0;

#ifndef  WIN_CSIM
	   DevTable[i].MemSizeInMB = h3InitSgram(DevTable[i].IoBase, sgramMode, sgramMask, sgramColor, DevTable[i].dwVendorDeviceID, NULL);  // default SGRAM vendor
#else
#ifdef SLI_AA
      if (0x0 == dwUnitNum)
   	   DevTable[i].MemSizeInMB = h3InitSgram(DevTable[i].IoBase, sgramMode, sgramMask, sgramColor, DevTable[i].dwVendorDeviceID, NULL);  // default SGRAM vendor
#else
   	DevTable[i].MemSizeInMB = h3InitSgram(DevTable[i].IoBase, sgramMode, sgramMask, sgramColor, DevTable[i].dwVendorDeviceID, NULL);  // default SGRAM vendor
#endif
#endif

      /* Map register space (memBase1). */

      switch (DevTable[i].MemSizeInMB)
	      {
         case 4:  
            mapSize = 0x0800000; 
            break; // map 8Mb   for 4Mb memory size

         case 8:  
            mapSize = 0x1000000; 
            break; // map 16Mb  for 8Mb memory size

         case 16: 
            mapSize = 0x2000000; 
            break; // map 32Mb  for 16Mb memory size

         case 32:
            mapSize = 0x4000000; 
            break; // map 64Mb  for 32Mb memory size
      
         case 64: 
            mapSize = 0x8000000; 
            break; // map 128Mb for 64Mb memory size

         default:   
            Debug_Printf( VNAME "Unrecognized memory configuration\n");
            return(NULL);
	      }

#ifdef SLI_AA
   if (SLI_AA_SLAVE_DEVICE == pDev->dwType)
      mapSize = dwDecodeSize[1];
   else
      {
      if (IS_NAPALM(pDev->dwVendorDeviceID))
         {
         DWORD Savestrap;
         DWORD strapInfo;
         DWORD newSize;
         pMasterIORegs = DevNodetoIORegs(dwDevNode);
         
         strapInfo = pMasterIORegs->strapInfo;
         Savestrap = strapInfo;
         strapInfo |= SST_STRAPINFO_REG_SELECT;
         pMasterIORegs->strapInfo = strapInfo;
         strapInfo = pMasterIORegs->strapInfo;
         
         pMasterIORegs->strapInfo = (Savestrap & ~SST_STRAPINFO_REG_SELECT);

         // Check for bad decode size
         newSize = GetMemSize((strapInfo & SST_STRAPINFO_MEMBASE1_DECODE) >> SST_STRAPINFO_MEMBASE1_DECODE_SHIFT);
         mapSize = dwDecodeSize[1];
         if (newSize != mapSize) 
            mapSize = newSize;

         // If MultiUnit we ask for twice as much!!!!
         if (nChips > 1)
            mapSize >>= 1;
         }
      }
#endif

#ifndef WIN_CSIM
	   MapDeviceFB(pDev, &mapSize);
	   if ((DevTable[i].LfbBase == 0xFFFFFFFF) && (dwUnitNum < 1))
	      {
	      Debug_Printf( VNAME "Unable to map memory base 1\n");
	      DevTable[i].dwDevNode = 0x0;
	      return(NULL);
	      }
#else
#ifdef SLI_AA
      if (0x0 == dwUnitNum)
         {
   	   MapDeviceFB(pDev, &mapSize);
	      if (DevTable[i].LfbBase == 0xFFFFFFFF)
	         {
	         Debug_Printf( VNAME "Unable to map memory base 1\n");
	         DevTable[i].dwDevNode = 0x0;
	         return(NULL);
	         }
         DevTable[i].RealLfbBase = DevTable[i].LfbBase;
         }
#else
      MapDeviceFB(pDev, &mapSize);
	   if (DevTable[i].LfbBase == 0xFFFFFFFF)
         {
	      Debug_Printf( VNAME "Unable to map memory base 1\n");
	      DevTable[i].dwDevNode = 0x0;
	      return(NULL);
	      }
      DevTable[i].RealLfbBase = DevTable[i].LfbBase;
#endif
#endif

	   // This may need to be fixed

	   if (!(DevTable[i].DispInfo.diInfoFlags & DEVICE_IS_NOT_VGA))
	      {
	      pVGADevTable = &DevTable[i];
	      h3InitVga(DevTable[i].IoBase, 1);	 // enable VGA decode
	      DevTable[i].bIsVGA = TRUE;
	      }
	   else
	      {
	      DevTable[i].bIsVGA = FALSE;
	      }

	   lpIOregs = (SstIORegs * )DevTable[i].RegBase[HWINFO_SST_IOREGS_INDEX];
      // Enable this when the C-Simulator is ready
#if 0
      if (IS_NAPALM(DevTable[i].dwVendorDeviceID))
         {
         DWORD lfbBit29;
	      lfbBit29 = lpIOregs->lfbMemoryConfig;
         SETDW(lpIOregs->lfbMemoryConfig, SST_RAW_LFB_UPDATE_READ_BIT);
	      DevTable[i].lfbMemoryConfig[0] = lpIOregs->lfbMemoryConfig;
         SETDW(lpIOregs->lfbMemoryConfig, SST_RAW_LFB_UPDATE_READ_BIT | SST_RAW_LFB_READ_TILE_COMPARE);
	      DevTable[i].lfbMemoryConfig[1] = lpIOregs->lfbMemoryConfig;
         SETDW(lpIOregs->lfbMemoryConfig, SST_RAW_LFB_UPDATE_READ_BIT | (lfbBit29 & SST_RAW_LFB_READ_TILE_COMPARE));
         }
      else
#endif
	      DevTable[i].lfbMemoryConfig[0]  = lpIOregs->lfbMemoryConfig;

      // At this point increase pciInit0
      // Skip this stuff on 
      pDev->dwPCI = DoBusType(dwDevNode);
#ifdef PCI_LATENCY_WHQL
      if (pDev->dwPCI)
         {
         pciInit0 = lpIOregs->pciInit0;

#ifdef SLI_AA
         // If single Unit then use 8 else use 10
         if (SLI_AA_REGULAR_DEVICE == pDev->dwType)
            pciInit0 = (pciInit0 & ~SST_PCI_LOWTHRESH) | (8 << SST_PCI_LOWTHRESH_SHIFT);
         else
            pciInit0 = (pciInit0 & ~SST_PCI_LOWTHRESH) | (10 << SST_PCI_LOWTHRESH_SHIFT) | SST_PCI_FORCE_FB_HIGH;

#else
         pciInit0 = (pciInit0 & ~SST_PCI_LOWTHRESH) | (8 << SST_PCI_LOWTHRESH_SHIFT);
#endif
         pciInit0 &=~(SST_PCI_DISABLE_IO|SST_PCI_DISABLE_MEM|SST_PCI_RETRY_INTERVAL);

         lpIOregs->pciInit0 = pciInit0;
         }
      else
#endif
         {
         pciInit0 = lpIOregs->pciInit0;
#ifdef SLI_AA
         // If single Unit then use 8 else use 10
         if (SLI_AA_REGULAR_DEVICE == pDev->dwType)
            pciInit0 = (pciInit0 & ~SST_PCI_LOWTHRESH) | (8 << SST_PCI_LOWTHRESH_SHIFT);
         else
            pciInit0 = (pciInit0 & ~SST_PCI_LOWTHRESH) | (10 << SST_PCI_LOWTHRESH_SHIFT);
#endif
         pciInit0 |=(SST_PCI_DISABLE_IO|SST_PCI_DISABLE_MEM);
         pciInit0 &= ~(SST_PCI_FORCE_FB_HIGH);
         lpIOregs->pciInit0 = pciInit0;
         }


#ifdef SLI_AA
      if (SLI_AA_SLAVE_DEVICE == DevTable[i].dwType)
         {
         // Good Time to 
         // Disable Us
         PCI_Write_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc, SST_PCI_COMMAND_ID, 0x02);      

         // Enable Master
         PCI_Write_Config(DevTable[i].dwBus, DevTable[i].dwDevFunc & ~0x07, SST_PCI_COMMAND_ID, 0x03);      
         // Find Master and link into chain
         for (j=0; j<MAX_BANSHEE_DEVICES; j++)
            {
            if ((DevTable[i].dwBus == DevTable[j].dwBus) && 
               ((DevTable[i].dwDevFunc & ~0x07) == DevTable[j].dwDevFunc))
               {
               pBackDev = &DevTable[j];
               pNextSlave = DevTable[j].pSlave;
               pSaveSlave = NULL;
               for (;pNextSlave != NULL;)
                  {
                  pBackDev = pNextSlave;
                  pNextSlave = pNextSlave->pSlave;
                  if (1 == pBackDev->dwUnitNum)
                     pSaveSlave = pBackDev;
                  }
   
               // If we get reenumerated then this will be true 
               // and this will cause a infinite loop
               if (pBackDev != &DevTable[i])
                  pBackDev->pSlave = &DevTable[i];

               // Is new Device Unit Number 1?
               if (NULL == pSaveSlave)
                  if (1 == DevTable[i].dwUnitNum)
                     pSaveSlave = &DevTable[i];

               // Duplicate Lfb Address .. start at First Slave
               pNextSlave = DevTable[j].pSlave;
               // If we have a valid Unit # 1 then duplicate it...
               if (NULL != pSaveSlave)
                  {
                  for (;pNextSlave != NULL; pNextSlave = pNextSlave->pSlave)
                     {
                     pNextSlave->LfbBase = pSaveSlave->LfbBase;
                     pNextSlave->nPages = pSaveSlave->nPages;
                     }
                   }

               for (k=0; k<HWINFO_SST_MAX_CHIP_INDEX; k++)
                  DevTable[j].RegBase[DevTable[i].dwUnitNum * HWINFO_SST_MAX_CHIP_INDEX + k] = DevTable[i].RegBase[k];
               break;
               }
            }      
         }
#endif


#ifdef WIN_CSIM
#ifdef SLI_AA
      if (0x0 != dwUnitNum)
         return pDev;
#endif
#endif

	   //
	   // are we on a P6 or a PII, or another processor that supports MTRR's?
	   //
#ifdef WINNT	// OLD CPUID CODE
	   CheckForP6();
#else
       GetCpuFeatures(&cpuFeatures);
#endif

#ifdef SLI_AA
      // If we are a Slave then skip MTRR mapping unless of course we are doing Multi-Mon
#ifdef WINNT	// OLD CPUID CODE
	   if ((!isP6) || (SLI_AA_SLAVE_DEVICE == pDev->dwType))
#else
	   if ((! (cpuFeatures.dwCpuFlags & CPU_FEATURE_P6_MTRR)) || (SLI_AA_SLAVE_DEVICE == pDev->dwType))
#endif //WINNT
#else
#ifdef WINNT	// OLD CPUID CODE
	   if (!isP6)
#else
	   if (! (cpuFeatures.dwCpuFlags & CPU_FEATURE_P6_MTRR))
#endif //WINNT
#endif //SLI_AA
	      return(pDev);		// no, just return now

#ifdef WINNT	// OLD CPUID CODE
	   if ( isP6 == P6_AMDK6_MTRRS )
#else
	   if (cpuFeatures.dwCpuFlags & CPU_FEATURE_K6_MTRR)
#endif
	      {
		   //
		   // AMD MTRR code for K6
		   //
		   //   If the os supports the MTRR calls for this CPU use them.
		   //

		   if (MTRRGetVersion()) 
		      {
			   if (0x0 != pDev->Mtrr)
			      {
				   // Uncache the old physical address
				   MTRRSetPhysical((PVOID)pDev->Mtrr, mapSize, MmNonCached);
			      }

			   MTRRSetPhysical((PVOID)(DevTable[i].PhysMemBase[1] & 0xFF000000), mapSize, MmFrameBufferCached);
			   pDev->Mtrr = DevTable[i].PhysMemBase[1] & 0xFF000000;
		      }
		   else 
		      {
			   // No OS MTRR support

			   pciSetAmdK6MTRR( 0, (PVOID)(DevTable[i].PhysMemBase[1] & 0xFF000000), mapSize, PciMemTypeWriteCombining );
		      }
	      }
#ifdef WINNT	// OLD CPUID CODE
   else if ( !(isP6 & (P6_NONINTEL_WITH_INTEL_MTRRS | P6_INTELCPU_WITH_MTRRS)) )	// Do we support Intel style MTRR's
#else
   else if (! (cpuFeatures.dwCpuFlags & CPU_FEATURE_P6_MTRR))	// Do we support Intel style MTRR's
#endif
	      {
	      return(pDev);		    // no, just return now
	      }
	   else
	      {

	      // Windows 95 or doesn't support setting MTRR's for this processor?
	      dwNFunc = VDD_Get_Mini_Dispatch_Table(&pDispatch);
	      if ( (dwNFunc < NBR_MINI_VDD_FUNCTIONS_41) | (MTRRGetVersion()==FALSE) )
	         {
		      mapSize = MAP_MTRR(mapSize);
		      //
		      // Yes we are, so mark the raw lfb space USWC (write combine).
		      // First, find a free mtrr set.   By convention, we should only use
		      // one of the the first 5 pairs
		      //

		      foundMTRR = 0;
		      if (0x0 != pDev->Mtrr)
		         {
		         // We have been here before so we can reuse the same Mtrr
		         // First set it to zero so that we can find it again
		         SetMTRR(pDev->Mtrr + 1, 0, 0);
		         SetMTRR(pDev->Mtrr, 0, 0);
		         }

		      for (pDev->Mtrr = 0x200; pDev->Mtrr <= 0x20e; pDev->Mtrr += 2)
		         {
		         GetMTRR(pDev->Mtrr, &mtrr1Lo, &mtrr1Hi);
		         GetMTRR(pDev->Mtrr + 1, &mtrr2Lo, &mtrr2Hi);

		         if ((mtrr1Lo == 0) && (mtrr1Hi == 0) &&
			          (mtrr2Lo == 0) && (mtrr2Hi == 0))
	               {
			         foundMTRR = 1;
			         break;
	               }

		         if ((mtrr1Lo == ((DevTable[i].PhysMemBase[1] & 0xFF000000) | 1)) &&
			          (mtrr1Hi == 0) &&
			          (mtrr2Lo == mapSize) &&
			          (mtrr2Hi == 0xf))
	               {
			         foundMTRR = 2;
			         break;
	               }
		         }

		      if (foundMTRR == 0)
		         {
	            pDev->Mtrr = 0;
		         Debug_Printf(VNAME "ACK!  No free MTRRs!\n");
		         return(pDev);
		         }

		      if (foundMTRR == 2)
	            {
		         return(pDev);		// our MTRR is already there
	            }

		      // first, clear both MTRRs
		      //
		      SetMTRR(pDev->Mtrr, 0, 0);
		      SetMTRR(pDev->Mtrr + 1, 0, 0);

		      mtrr1Lo = (DevTable[i].PhysMemBase[1] & 0xFF000000) | 1;
		      mtrr1Hi = 0;
		      mtrr2Lo = mapSize;
		      mtrr2Hi = 0xf;

		      // then, set them to the required values
		      //
		      SetMTRR(pDev->Mtrr, mtrr1Lo, mtrr1Hi);
		      SetMTRR(pDev->Mtrr + 1, mtrr2Lo, mtrr2Hi);
	         }
	      // Windows '98
		   else
	         { 
	         if (0x0 != pDev->Mtrr)
	         	{
		         // Uncache the old physical address
	            MTRRSetPhysical((PVOID)pDev->Mtrr, mapSize, MmNonCached);
	      	   }
      
	         MTRRSetPhysical((PVOID)(DevTable[i].PhysMemBase[1] & 0xFF000000), mapSize, MmFrameBufferCached);
	         pDev->Mtrr = DevTable[i].PhysMemBase[1] & 0xFF000000;
	         }
	      }
      }
   else
	   Debug_Printf("Max Device exceeded\n");

    return pDev;     
}


/*----------------------------------------------------------------------
Function name:  FindPDEVFromDevNode

Description:    Find the pDev from the DevNode table.

Information:    

Return:         PDEVTABLE   pDev if success or,
                            NULL for failure.
----------------------------------------------------------------------*/
PDEVTABLE
FindPDEVFromDevNode(DWORD dwDevNode)
{
   PDEVTABLE pReturn = NULL;
   int i;
   
   for (i=0; i<MAX_BANSHEE_DEVICES; i++)
      {
      // Is this the one
      if (dwDevNode == DevTable[i].dwDevNode)
         {
         pReturn = &DevTable[i];
         break;
         }

      // No More to check ??
      if (0x0 == DevTable[i].dwDevNode)
         break;
      }

   return pReturn;      
}

#ifdef SLI_AA
/*----------------------------------------------------------------------
Function name:  DevNodeandUnitNumtoIORegs

Description:    This function translates the devnode/Unit Num into a
                pointer to the IO Registers.
Information:    

Return:         (SstIORegs *)   the pDev->RegBase.
----------------------------------------------------------------------*/
SstIORegs * DevNodeandUnitNumtoIORegs(DEVNODE devnode, DWORD dwUnitNum)
{
   PDEVTABLE pDev;

   pDev = FindPDEVFromDevNodeandUnitNum(devnode, dwUnitNum);
   return (SstIORegs *)pDev->RegBase[HWINFO_SST_IOREGS_INDEX];
}

/*----------------------------------------------------------------------
Function name:  FindPDEVFromDevNodeandUnitNum

Description:    Find the pDev from the DevNode and Unit Number

Information:    

Return:         PDEVTABLE   pDev if success or,
                            NULL for failure.
----------------------------------------------------------------------*/
PDEVTABLE
FindPDEVFromDevNodeandUnitNum(DWORD dwDevNode, DWORD dwUnitNum)
{
   PDEVTABLE pReturn = NULL;
   int i;
   
   for (i=0; i<MAX_BANSHEE_DEVICES; i++)
      {
      // Is this the one
      if ((dwDevNode == DevTable[i].dwDevNode) && (dwUnitNum == DevTable[i].dwUnitNum))
         {
         pReturn = &DevTable[i];
         break;
         }

      // No More to check ??
      if (0x0 == DevTable[i].dwDevNode)
         break;
      }

   return pReturn;      
}

/*----------------------------------------------------------------------
Function name:  FindPDEVFromUnitNumber

Description:    Find the PDEV from Special Unit Number which is just
the DevTable Index

Information:    

Return:         PDEVTABLE   pDev if success or,
                            NULL for failure.
----------------------------------------------------------------------*/
PDEVTABLE
FindPDEVFromUnitNumber(DWORD dwUnitNumber)
{
   PDEVTABLE pReturn = NULL;
   pReturn = &DevTable[dwUnitNumber];
   return pReturn;      
}
#endif

/*----------------------------------------------------------------------
Function name:  InitSecondaryDispatchTable

Description:    Initialize a secondary displatch table.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void InitSecondaryDispatchTable(DWORD dwDevNode)
{
   PDWORD pDispatch;
   DWORD dwNFunc;
#ifdef SLI_AA
   DWORD i;
   DWORD dwNum;
#endif

   dwNFunc = VDD_Get_Mini_Dispatch_Table(&pDispatch);
#ifdef SLI_AA
   if (dwNFunc >= NBR_MINI_VDD_FUNCTIONS_41)
      dwWin98 = TRUE;
   else
      dwWin98 = FALSE;
#endif  

   pDispatch[REGISTER_DISPLAY_DRIVER]= (DWORD)RegisterDisplayDriver;

#ifdef SLI_AA
   // Initialize the additional Units
   dwNum = GetNumUnits(dwDevNode);
   for (i=1; i<(int)dwNum; i++)
      InitDevNode(dwDevNode, i);   
#else
   dwDevNode = dwDevNode;
#endif
      
   InitPower();
}


/*----------------------------------------------------------------------
Function name:  GetBIOSInfo

Description:    This function is used to get the BIOS Version and
                should only be called at Enable Time.  It also gets
                the sgramMode, and the biosBoardConfigInfo.

Information:    At Enable time in Win '98 the current bios is mapped
                in at C000:0000.

Return:         VOID
----------------------------------------------------------------------*/

void GetBIOSInfo(PDEVTABLE pDevTable, FxU32 vm)
{
   DWORD * pStrLoc;
   BYTE * pByteLoc;
   DWORD j;
   DWORD k;
   DWORD dwBIOSSize;
   PVMMCB hVM = (PVMMCB)vm;
   PWORD pFixSpot;
   PWORD pROMConfig;
   POEMTABLE pOEMConfig;
   char * pChar;
   

   pDevTable->bBIOSVersion[0]='\0';
   pDevTable->bBIOSVersion[MAX_BIOS_VERSION_STRING-1]='\0';
   // Get Address of Currently mapped in ROM @ C000:0000
	pStrLoc = (DWORD *)(0xC0000 + hVM->CB_High_Linear);
   pByteLoc = (BYTE *)pStrLoc;

   if (NULL == pByteLoc)
      return;

   // Get the size byte in the ROM header.  This is right after
   // 0x55 0xAA
   dwBIOSSize = pByteLoc[2]<<7;   

   for (j=0; j<dwBIOSSize; j++)
   {
      if ((BIOS_STRING_1 == pStrLoc[0]) && (BIOS_STRING_2 == pStrLoc[1]))
      {
            j = 0;

            pByteLoc = (BYTE *)&pStrLoc[2];
            for (k=0; k<MAX_BIOS_VERSION_STRING - 1; k++)
            {
                if(j)
                {
                    //stop if it will be next word
                    //or it is not a digit nor a char.
                    if((pByteLoc[k]==' ') || !pByteLoc[k] ||
                        ( ((pByteLoc[k] <'0') || (pByteLoc[k] >'9'))
                          &&((pByteLoc[k] <'A') || (pByteLoc[k] >'Z'))
                          &&((pByteLoc[k] <'a') || (pByteLoc[k] >'z'))
                          &&(pByteLoc[k]!='.')) )
                       
                    {
                       pDevTable->bBIOSVersion[j] ='\0';
                       break;
                    }
                    else
                     pDevTable->bBIOSVersion[j++] = pByteLoc[k];

                }
                else
                {
                   //only start with digit
                   if((pByteLoc[k] <= '9') &&
                    (pByteLoc[k] >= '0' ))
                  {
                      //must start at a word boundary
                      if( pByteLoc[k-1] == ' ') 
                       pDevTable->bBIOSVersion[j++] = pByteLoc[k];
                  }
                }
            }
            break;
     }
 
     pStrLoc = (DWORD *) ( pByteLoc + j);   //check it byte by byte
   }
   // While we are at it let's get the SGRAMMODE which is in the BIOS OEMTable
   // We may need it if we go into low power mode.
   pFixSpot = (PWORD)(0xC0000 + hVM->CB_High_Linear + ROM_CONFIG);
   pROMConfig = (PWORD)(0xC0000 + hVM->CB_High_Linear + *pFixSpot);
   pOEMConfig = (POEMTABLE)(0xC0000 + hVM->CB_High_Linear + *pROMConfig);      
   pDevTable->dwPowerReg[SGRAMMODE_INDEX] = pOEMConfig->sgramMode;


//Read the BIOS version using the string pointer in the BIOS OemConfigTable
   if (IS_NAPALM(pDevTable->dwVendorDeviceID))
   	if (*(((FxU16*)(pOEMConfig)-1)) >= FIRSTOEMSTRUCTCOMPATVERSION)
   	{
       // pick up Version Number pointed to by OEMConfig table in BIOS
      	pChar = (char*) (0xC0000 + hVM->CB_High_Linear + pOEMConfig->OEMBiosVersion);

       	for (k = 0; (k < MAX_BIOS_VERSION_LENGTH - 1) && (pChar[k]); k++)
       	{
         	pDevTable->bBIOSVersion[k] = pChar[k];
       	}
       	pDevTable->bBIOSVersion[k] = '\0';
   	}

    pDevTable->biosBoardConfigInfo = 0;
        // Test if the OEMConfig table version supports the OEM product name and OEM chip name feature.
        // The OEMConfig table version is the word in front of the structure so convert the pointer to the
        // structure to a word pointer and back up one word.
   if (IS_NAPALM(pDevTable->dwVendorDeviceID))
		{
        if (*(((FxU16*)(pOEMConfig)-1)) >= FIRSTOEMSTRUCTCOMPATVERSION)
        {

            // pick up Product Name pointed to by OEMConfig table in BIOS

            pChar = (char*) (0xC0000 + hVM->CB_High_Linear + pOEMConfig->OEMBoardName);

            for (k = 0; (k < MAX_BIOS_VERSION_LENGTH - 1) && (pChar[k])  &&
					!((pChar[k] == 'w') && (pChar[k+1] == 'i') && (pChar[k+2] == 't') && (pChar[k+3] == 'h')); k++)
            {
                pDevTable->bOEMBoardName[k] = pChar[k];
            }
            pDevTable->bOEMBoardName[k] = '\0';

            // pick up Chip Name pointed to by OEMConfig table in BIOS
            pChar = (char*) (0xC0000 + hVM->CB_High_Linear + pOEMConfig->OEMChipName);
            for (k = 0; (k < MAX_BIOS_VERSION_LENGTH - 1) && (pChar[k]); k++)
            {
                pDevTable->bChipName[k] = pChar[k];
            }
            pDevTable->bChipName[k] = '\0';

//Return board configuration DWORD
            pDevTable->biosBoardConfigInfo = pOEMConfig->BoardConfig;
        }
		}
	else
		{
//Since there isn't a board string in the BIOS, return board string as NULL.
            pDevTable->bChipName[0] = 0;
		}
#ifdef notPortedFromWinNT4
        else
        {
            // OEMConfig table version is 2 or less, it doesn't support this feature.
            pwszChip = (PWSTR) UNKNOWN_STRING;
            cbChip   = sizeof(UNKNOWN_STRING);
            pwszAdapterString = (PWSTR) UNKNOWN_STRING L" Board";
            cbAdapterString   = sizeof(UNKNOWN_STRING L" Board");
            pwszAdapterType   = (PWSTR) UNKNOWN_STRING L" Board";
            cbAdapterType     = sizeof(UNKNOWN_STRING L" Board");
        }
#endif //def notPortedFromWinNT4

}

//
//  Note the code below here is called from thunk32.c so
//  it is not VDD code so watch the stack
//
#define DEBUG_FIX                       \
{                                       \
    __asm   movzx   ebp, bp             \
    __asm   mov     eax, [ebp]          \
    __asm   movzx   eax, ax             \
    __asm   mov     [ebp], eax          \
}


/*----------------------------------------------------------------------
Function name:  FindLPFromDevNode

Description:    Find the lpDriverData based on the DevNode.

Information:    

Return:         DWORD   lpDriverData if success or,
                        0 for failure.
----------------------------------------------------------------------*/
#ifdef SLI_AA
// This is a hack to help GetNumUnits to work on two real devices...
#if 0
DWORD xinp(DWORD Addr) 
{
   DWORD dwReturn;

   DEBUG_FIX;

   _asm {mov   edx, Addr}
   _asm {xor   eax, eax}
   _asm {in    eax, dx}
   _asm {mov   dwReturn, eax}

   return dwReturn;
}

void xoutpw(DWORD addr, DWORD dwData)
{
   DEBUG_FIX;

   _asm {mov   edx, addr}
   _asm {mov   eax, dwData}
   _asm {out   dx, ax}
}

void xoutpd(DWORD addr, DWORD dwData)
{
   DEBUG_FIX;

   _asm {mov   edx, addr}
   _asm {mov   eax, dwData}
   _asm {out   dx, eax}
}

DWORD xinpw(DWORD Addr) 
{
   DWORD dwReturn;

   DEBUG_FIX;

   _asm {mov   edx, Addr}
   _asm {xor   eax, eax}
   _asm {in    ax, dx}
   _asm {mov   dwReturn, eax}

   return dwReturn;
}
#endif
#endif

DWORD
FindLPFromDevNode(DWORD dwDevNode)
{
   DWORD dwReturn;
   int i;
#ifdef SLI_AA
#if 0
   DWORD dwSave;
#endif
#endif
   
   DEBUG_FIX;
   
   dwReturn = 0x0;
   for (i=0; i<MAX_BANSHEE_DEVICES; i++)
      {
      // Is this the one
      if (dwDevNode == DevTable[i].dwDevNode)
         {
         dwReturn = DevTable[i].lpDriverData;
         break;
         }

      // No More to check ??
      if (0x0 == DevTable[i].dwDevNode)
         break;
      }

#ifdef SLI_AA
      if ((0x0 == dwReturn) && (dwDevNode < MAX_BANSHEE_DEVICES))
         {
         dwReturn = DevTable[dwDevNode].lpDriverData;
         }
#endif

   return dwReturn;      
}


/*----------------------------------------------------------------------
Function name:  SetLPFromDevNode

Description:    Set lpDriverData into the Dev table.

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void 
SetLPFromDevNode(DWORD dwDevNode, DWORD lpDriverData)
{
   int i;
   int nFound;   
   DEBUG_FIX;

   nFound = 0;
   for (i=0; i<MAX_BANSHEE_DEVICES; i++)
      {
      // Is this the one
      if (dwDevNode == DevTable[i].dwDevNode)
         {
         DevTable[i].lpDriverData = lpDriverData;
         nFound = 1;
         break;
         }

      // No More to check ??
      if (0x0 == DevTable[i].dwDevNode)
         break;
      }
#ifdef SLI_AA
   if ((0x0 == nFound) && (dwDevNode < MAX_BANSHEE_DEVICES))
      {
      DevTable[dwDevNode].lpDriverData = lpDriverData;
      }
#endif

}

#ifdef SLI_AA
/*----------------------------------------------------------------------
Function name:  ClearAllSlaveBits

Description:    Clear All Slave Bits in SLI AA mode

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void HardwareClearBit(DWORD);
void ClearAllSlaveBits(PDEVTABLE pDev)
{
   PDEVTABLE pSlave;

   for (pSlave = pDev; pSlave != NULL; pSlave = pSlave->pSlave)
      HardwareClearBit(pSlave->lpDriverData);
}

/*----------------------------------------------------------------------
Function name:  SetAllSlaveBits

Description:    Set All Slave Bits in SLI AA mode

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void HardwareSetBit(DWORD);
void SetAllSlaveBits(PDEVTABLE pDev)
{
   PDEVTABLE pSlave;

   for (pSlave = pDev; pSlave != NULL; pSlave = pSlave->pSlave)
      HardwareSetBit(pSlave->lpDriverData);
}
#endif

/*----------------------------------------------------------------------
Function name:  FindActiveBanshee

Description:    Lookup the active DevNode from the PCI command
                register bits.
Information:    

Return:         PDEVTABLE   pDev if success or,
                            NULL for failure.
----------------------------------------------------------------------*/
PDEVTABLE FindActiveBanshee(void)
{
   PDEVTABLE pReturn = NULL;
   WORD wCmd;
   int i;

   for (i=0; i<MAX_BANSHEE_DEVICES; i++)
      {
      // No more Banshee's ?
      if (0x0 == DevTable[i].dwDevNode)
         break;

      CM_Call_Enumerator_Function( DevTable[i].dwDevNode,
                                   PCI_ENUM_FUNC_GET_DEVICE_INFO,
                                   0x4, &wCmd, 
                                   sizeof(WORD), 0 );

      // This is the one!!!
      if (0x3 == (wCmd & 0x03))
         { 
         pReturn = &DevTable[i];
         break;
         }
      }

   return pReturn;
}

#ifdef SLI_AA
/*----------------------------------------------------------------------
Function name:  FindActiveBansheewithCount

Description:    Find the Unit we should run DDC on.  Win '98 gives
no indication of which unit we should run DDC on so we implement
a round robin scheduler where we equally distribute the DDC request between
the different devices <we assume that we have only to distribute between one
master and one slave>.

Information:    

Return:         PDEVTABLE   pDev if success or,
                            NULL for failure.
----------------------------------------------------------------------*/
DWORD dwLater=0;
PDEVTABLE FindActiveBansheewithCount(int nRequest)
{
   PDEVTABLE pDev;
   PDEVTABLE pMDev;

   pMDev = pDev = FindActiveBanshee();
   if ((NULL != pDev) && (nRequest < sizeof(pDev->dwDDCCount)))
      {
      // FIX_ME: dwLater is only needed for our simulation environment.
      // We init the device at Enable Time whereas the DDC request
      // comes in before that. In our simulation we are using a PCI
      // as the master and a AGP as the slave and as such the Bridge
      // has not been setup yet at GetNumUnits time but should
      // be in the real thing.  At this point we also have to move
      // InitDevNode of the slave early.
      if (dwLater)
         {
         if ((pDev->dwDDCCount[nRequest] & 1) && (SLI_AA_MASTER_DEVICE == pDev->dwType))
            {
            pDev = pDev->pSlave;
            }
         }
      pMDev->dwDDCCount[nRequest] = (pMDev->dwDDCCount[nRequest] + 1) & 0x01;
      }

   return pDev;
}
#endif

// AMD K6 has only two MTRRs

#define MTRR_LAST_AMDK6    1  

/*
** The MTRRs of the AMD K6 are not compatible to the MTRRs on Intel
** processors. There are two MTRRs which are in a single MSR.
*/

#define MTRR_MSR_AMDK6     0xC0000085

// Stuff for programming K6 MSRs
typedef struct MSRInfo_s {
  unsigned long
    msrNum,                     /* Which MSR? */
    msrLo, msrHi;               /* MSR Values  */
} MSRInfo;


/*----------------------------------------------------------------------
Function name:  genMTRRvalAmdK6

Description:    Computes bit pattern to program into a K6-style MTRR

Information:    Computes bit pattern to program into a K6-style MTRR
                based on the desired memory range base address, size,
                and type.
                
Return:         Returns FXFALSE if memory type unsupported, block size
                out of range, or block size not power of 2, or base
                address not multiple of block size.  Otherwise,
                returns FXTRUE.
----------------------------------------------------------------------*/

static FxBool
genMTRRvalAmdK6(void *physBaseAddress, FxU32 physSize, PciMemType type, FxU32 *MTRRval)
{
   FxBool retVal = FXFALSE;
   FxU32 memTypeBits = 0;
   FxU32 physAddrMask;

   /* Validate memory type */

   if (type == PciMemTypeUncacheable) {
      memTypeBits = 0x1;
   }
   else if (type == PciMemTypeWriteCombining) {
      memTypeBits = 0x2;
   }
   else {
      return retVal;
   }

   /* Validate memory range size */

   if (physSize < 128*1024) {         /* make sure size >= 128 K */ 
      return retVal;
   }

   if (physSize & (physSize - 1)) {   /* make sure size is power of two */
      return retVal;
   }

   /* Validate base address */

   if ((FxU32)physBaseAddress % physSize) {  /* make sure base is multiple of size */
      return retVal;
   }

   /* Convert the range size into a mask */

   physAddrMask = 0x7FFF;
   physSize >>=18;            /* 128K --> 0, 256K --> 1, etc */
   while (physSize) {
      physAddrMask <<= 1;
      physSize     >>= 1;
   }

   /* Now mask: 128K => 7FFF, 256K => 7FFE, 512K => 7FFC, ... , 4G => 0000 */

   *MTRRval = ((((FxU32) physBaseAddress >> 17) & 0x7FFF) << 17) |
              (physAddrMask & 0x7FFF) << 2        |
              memTypeBits;

   retVal = FXTRUE;

   return retVal;
}


/*----------------------------------------------------------------------
Function name:  pciSetAmdK6MTRR

Description:    Writes a K6-style MTRR

Information:    Set up the specified K6-style MTRR based on physical
                address, physical size, and type.

Return:         Returns FXTRUE if the MTRR is set, otherwise returns
                FXFALSE.
----------------------------------------------------------------------*/


/*
**  pciSetAmdK6MTRR - 
**
**  NOTE:  A zero for the physical size results in the MTRR being
**  cleared. 
**
*/
FxBool
pciSetAmdK6MTRR(FxU32 mtrrNum, void *physBaseAddr, FxU32 physSize,
                PciMemType type)
{
  FxBool
    res, 
    rVal = FXFALSE;

  FxU32
    MTRRval;

  MSRInfo
    inS;

  if (mtrrNum > MTRR_LAST_AMDK6)
    return rVal;

  inS.msrNum = MTRR_MSR_AMDK6;   /* One MSR for both MTRRs */

  if (physSize == 0) {  /* size of 0 implies clear MTRR */

     GetMTRR( MTRR_MSR_AMDK6, &inS.msrLo, &inS.msrHi);

     if (mtrrNum) {
        inS.msrHi = 0x0;   /* clear MTRR 1 */
     }
     else {
        inS.msrLo = 0x0;   /* clear MTRR 0 */
     }

     SetMTRR(MTRR_MSR_AMDK6, inS.msrLo, inS.msrHi);
  }
  else {

     // Generate masks and set the MTRR

     res = genMTRRvalAmdK6(physBaseAddr, physSize, type, &MTRRval);

     if (res == FXFALSE)
       return rVal;

     GetMTRR( MTRR_MSR_AMDK6, &inS.msrLo, &inS.msrHi);

     if (mtrrNum) {
        inS.msrHi = MTRRval;   /* program MTRR 1 */
     }
     else {
        inS.msrLo = MTRRval;   /* program MTRR 0 */
     }

     SetMTRR(MTRR_MSR_AMDK6, inS.msrLo, inS.msrHi);
  }

  rVal = FXTRUE;

  return rVal;
} // pciSetAmdK6MTRR


/*----------------------------------------------------------------------
Function name:  our_inpd

Description:    Read a double word

Information:    

Return:         DWORD read  
                        
----------------------------------------------------------------------*/
DWORD our_inpd(DWORD Addr) 
{
   DWORD dwReturn;

   _asm {mov   edx, Addr}
   _asm {xor   eax, eax}
   _asm {in    eax, dx}
   _asm {mov   dwReturn, eax}

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name:  our_outpw

Description:    write a word

Information:    

Return:         write a word  
                        
----------------------------------------------------------------------*/
void our_outpw(DWORD addr, DWORD dwData)
{
   _asm {mov   edx, addr}
   _asm {mov   eax, dwData}
   _asm {out   dx, ax}
}

/*----------------------------------------------------------------------
Function name:  our_outpd

Description:    write a dword

Information:    

Return:         write a dword  
                        
----------------------------------------------------------------------*/
void our_outpd(DWORD addr, DWORD dwData)
{
   _asm {mov   edx, addr}
   _asm {mov   eax, dwData}
   _asm {out   dx, eax}
}

/*----------------------------------------------------------------------
Function name:  our_inpw

Description:    read a word

Information:    

Return:         read a word  
                        
----------------------------------------------------------------------*/
DWORD our_inpw(DWORD Addr) 
{
   DWORD dwReturn;

   _asm {mov   edx, Addr}
   _asm {xor   eax, eax}
   _asm {in    ax, dx}
   _asm {mov   dwReturn, eax}

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name:  DevNodetoIORegs

Description:    This function translates the devnode into a
                pointer to the IO Registers.
Information:    

Return:         (SstIORegs *)   the pDev->RegBase.
----------------------------------------------------------------------*/
SstIORegs * DevNodetoIORegs(DEVNODE devnode)
{
   PDEVTABLE pDev;

   pDev = FindPDEVFromDevNode(devnode);
   return (SstIORegs *)pDev->RegBase[HWINFO_SST_IOREGS_INDEX];
}

/*----------------------------------------------------------------------
Function name: MapDeviceRegisters

Description:    This function maps a Device Register's physical addresses to
                Linear Addresses

Information:    

Return:         (void)
----------------------------------------------------------------------*/
void MapDeviceRegisters(PDEVTABLE pDev, DWORD dwSize)
{
   int i;
#ifdef SLI_AA
   DWORD PCIDecode;

   // if we are a slave then don't fat map
   if (SLI_AA_SLAVE_DEVICE == pDev->dwType)
      {
      for (i=0; i<sizeof(SparseMemBase0Map)/sizeof(DWORD); i++)
         pDev->RegBase[i] = (DWORD)_MapPhysToLinear(pDev->PhysMemBase[0] + SparseMemBase0Map[i], SparseMemBase0Size[i], 0x0);
      }      
   else
#endif
      {
      pDev->RegBase[0] = (DWORD) _MapPhysToLinear(pDev->PhysMemBase[0], dwSize, 0);
      for (i=0; i<sizeof(SparseMemBase0Map)/sizeof(DWORD); i++)
         pDev->RegBase[i] = pDev->RegBase[0] + SparseMemBase0Map[i];
      }

#ifdef SLI_AA   
   if (IS_NAPALM(pDev->dwVendorDeviceID))
      {
      // Set IO Decode and MemBase0 Decode to 0x100 and 32 MB
      PCIDecode = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, CFG_PCI_DECODE);
      PCIDecode = PCIDecode & ~(CFG_MEMBASE0_DECODE | CFG_IOBASE_DECODE);
      PCIDecode = PCIDecode | (GetDecodeSize(dwSize) << CFG_MEMBASE0_DECODE_SHIFT);      
      PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, CFG_PCI_DECODE, PCIDecode);
      }
#endif
}

/*----------------------------------------------------------------------
Function name: __PageReserve

Description:    
                

Information:    

Return:         
----------------------------------------------------------------------*/
DWORD VXDINLINE __PageReserve(DWORD Area, DWORD NumPages, DWORD Flags) 
{
    DWORD dwReturn;

    __asm pushad

    __asm push Flags     
    __asm push NumPages     
    __asm push Area
    VMMCall(_PageReserve);
    __asm add esp, 3*4;
    __asm mov dwReturn, eax;
    __asm popad
    return(dwReturn);
}

/*----------------------------------------------------------------------
Function name: __PageCommitPhys

Description:    
                

Information:    

Return:         
----------------------------------------------------------------------*/
DWORD VXDINLINE __PageCommitPhys(DWORD LinAddr, DWORD NumPages, DWORD FirstPage, DWORD Flags) 
{
    DWORD dwReturn;

    __asm pushad

    __asm push Flags     
    __asm push FirstPage     
    __asm push NumPages     
    __asm push LinAddr
    VMMCall(_PageCommitPhys);
    __asm add esp, 4*4;
    __asm mov dwReturn, eax;
    __asm popad
    return(dwReturn);
}

/*----------------------------------------------------------------------
Function name: __PageDecommit

Description:    
                

Information:    

Return:         
----------------------------------------------------------------------*/
DWORD VXDINLINE __PageDecommit(DWORD LinAddr, DWORD NumPages, DWORD Flags) 
{
    DWORD dwReturn;

    __asm pushad

    __asm push Flags     
    __asm push NumPages     
    __asm push LinAddr
    VMMCall(_PageDecommit);
    __asm add esp, 3*4;
    __asm mov dwReturn, eax;
    __asm popad
    return(dwReturn);
}

/*----------------------------------------------------------------------
Function name: MapDeviceFB

Description:    This function maps a FB physical address to
                a Linear Address

Information:    

Return:         (void)
----------------------------------------------------------------------*/
void MapDeviceFB(PDEVTABLE pDev, DWORD * pSize)
{
#ifdef SLI_AA
   DWORD PCIDecode;

   if (SLI_AA_SLAVE_DEVICE == pDev->dwType)
      {
      //pDev->nPages = (*pSize + 4095) >> 12;
      // Just Map in 16 MB for the First Slave and all the rest can share
      pDev->nPages = (0x1000000 + 4095) >> 12;
      if (1 == pDev->dwUnitNum)
         {
         // For Multi-Monitor we don't want to change the size
         
         pDev->LfbBase = __PageReserve(PR_SYSTEM, pDev->nPages, PR_FIXED);
         if (0xFFFFFFFF != pDev->LfbBase)
            {
            __PageCommitPhys(pDev->LfbBase >>12, pDev->nPages, pDev->PhysMemBase[1] >> 12, PC_INCR | PC_WRITEABLE | PC_USER);
            _LinPageLock(pDev->LfbBase >> 12, pDev->nPages, 0x0);
            }
         }
      else
         {
         // Need a way to get First Slave LfbBase Address
         pDev->LfbBase = 0x0;
         }
      }      
   else
      {
      pDev->LfbBase = (DWORD) _MapPhysToLinear(pDev->PhysMemBase[1], *pSize, 0);
      }

   if (IS_NAPALM(pDev->dwVendorDeviceID))
      {
      // Set MemBase1 Size to *pSize
      PCIDecode = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc, CFG_PCI_DECODE);
      PCIDecode = PCIDecode & ~(CFG_MEMBASE1_DECODE);
      PCIDecode = PCIDecode | (GetDecodeSize(*pSize) << CFG_MEMBASE1_DECODE_SHIFT);
      PCI_Write_Config(pDev->dwBus, pDev->dwDevFunc, CFG_PCI_DECODE, PCIDecode);
      }
#else
   pDev->LfbBase = (DWORD) _MapPhysToLinear(pDev->PhysMemBase[1], *pSize, 0);
#endif
}

#ifdef SLI_AA
/*----------------------------------------------------------------------
Function name: SwapPhysFB

Description:    This function maps a swaps FB physical addresses

Information:    

Return:         (void)
----------------------------------------------------------------------*/
#define PAGE_SIZE(A) (((A)+4095)>>12)
BYTE * SwapPhysFB(PDEVTABLE pDev, DWORD dwOffset, DWORD dwOldSize, DWORD dwNewSize)
{
   DWORD dwAddr;

   dwAddr = (pDev->PhysMemBase[1] + dwOffset) & ~(4095);
   dwOffset &= 4095;
   // if dwOffset > 0 then we will get 1 more page 
   dwNewSize = PAGE_SIZE(dwNewSize + dwOffset);
   dwOldSize = PAGE_SIZE(dwOldSize);

   // Unknow if this is all I have to do to swap Physical Pages
   __PageDecommit(pDev->LfbBase >>12, dwOldSize, 0x0);
   __PageCommitPhys(pDev->LfbBase >>12, dwNewSize, dwAddr >> 12, PC_INCR | PC_WRITEABLE | PC_USER);

   return (BYTE *)(pDev->LfbBase + dwOffset);
}
#endif
