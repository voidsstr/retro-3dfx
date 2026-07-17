/*++
**
** Copyright (c) 1997-1998, 3Dfx Interactive, Inc.
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

Module Name:

    h3.c

Abstract:

    This module contains the code that implements the H3 miniport driver.

Environment:

    Kernel mode

Revision History:

--*/

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "h3.h"
#include "localpci.h"
#include "cmdcnst.h"

#define MAX(a,b)	(a >= b) ? a : b
#define MIN(a,b)	(a <= b) ? a : b

#if ENABLE_ADDRESS_LIST_ARRAY
PULONG
H3AllocFindAddress(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	PTDFX_LINEARADDR_INFO pInfo
	)
{
	unsigned long k;

	pInfo->ulLinearBase = (ULONG) NULL;

	for (k = 0; k < MAX_ADDRESS_TABLE_SIZE; k++)
	{
		if (   (HwDeviceExtension->AddressList[k] == 0)
//DanO 06/01/99 The following is an illegal import for W2K but we need it for now to make the OpenGL ICD work.
//DanO 06/01/99 #if (_WIN32_WINNT < 0x0500)
        || (HwDeviceExtension->AddressProcess[k] != IoGetCurrentProcess())
//DanO 06/01/99 #endif
       )
			continue;

		if (((unsigned long)pInfo->ulAddress - (unsigned long)HwDeviceExtension->AddressList[k]) < HwDeviceExtension->MemBase1Length)
		{
			pInfo->ulLinearBase = (PULONG) HwDeviceExtension->AddressList[k];
			break;
		}
	}

	return (PULONG) pInfo->ulLinearBase;
}

VOID
H3RefCountAlloc(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Addr
	)
{
	int j, k;
	BOOLEAN found = FALSE;
	
	//
	// Check to see if there are any maps outstanding
	//
	for (k = 0; k < MAX_ADDRESS_TABLE_SIZE; k++)
	{
		if (   (HwDeviceExtension->AddressList[k] == Addr)
//DanO 06/01/99 The following is an illegal import for W2K but we need it for now to make the OpenGL ICD work.
//DanO 06/01/99 #if (_WIN32_WINNT < 0x0500)
        && (HwDeviceExtension->AddressProcess[k] == IoGetCurrentProcess())
//DanO 06/01/99 #endif
       )
		{
			found = TRUE;
			break;
		}
	}

	if (!found)
	{
		//
		// we didn't find it, add it to the list
		//
		for (j = 0; j < MAX_ADDRESS_TABLE_SIZE; j++)
		{
			if (HwDeviceExtension->AddressList[j] == 0)
				break;
		}

		if (j == MAX_ADDRESS_TABLE_SIZE)
			VideoDebugPrint((0, "Address table is full -- we're screwed!\n"));
		else
		{
			HwDeviceExtension->AddressList[j] = Addr;
			HwDeviceExtension->AddressAllocCount[j] = 1;
//DanO 06/01/99 The following is an illegal import for W2K but we need it for now to make the OpenGL ICD work.
//DanO 06/01/99 #if (_WIN32_WINNT < 0x0500)
      HwDeviceExtension->AddressProcess[j] = IoGetCurrentProcess();
//DanO 06/01/99 #endif
		}
	}
	else
		++HwDeviceExtension->AddressAllocCount[k];

	DumpAddressTable(HwDeviceExtension);
}

VOID
H3RefCountFree(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket,
	ULONG Addr
	)
{
	int k;
	BOOLEAN found = FALSE;

	DumpAddressTable(HwDeviceExtension);

	//
	// Check to see if there are any maps outstanding
	//

	for (k = 0; k < MAX_ADDRESS_TABLE_SIZE; k++)
	{
		if (   (HwDeviceExtension->AddressList[k] == Addr)
//DanO 06/01/99 The following is an illegal import for W2K but we need it for now to make the OpenGL ICD work.
//DanO 06/01/99 #if (_WIN32_WINNT < 0x0500)
        && (HwDeviceExtension->AddressProcess[k] == IoGetCurrentProcess())
//DanO 06/01/99 #endif
        )
		{
			found = TRUE;
			break;
		}
	}

	if (!found)
	{
		//
		// we didn't find it, because they lied
		//
		RequestPacket->StatusBlock->Status = NO_ERROR; // ERROR_INVALID_PARAMETER;
	}
	else
	{
		--HwDeviceExtension->AddressAllocCount[k];
		if (HwDeviceExtension->AddressAllocCount[k] < 1)
		{
			// delete the entry from the list -- no one is referencing it...
			HwDeviceExtension->AddressAllocCount[k] = 0;
			HwDeviceExtension->AddressList[k]       = 0L;
#if (_WIN32_WINNT < 0x0500)
			HwDeviceExtension->AddressProcess[k]    = 0L;
#endif
		}
	}
}

VOID
DumpAddressTable(
    PHW_DEVICE_EXTENSION HwDeviceExtension
	)
{
	int i;
	
	UNREFERENCED_PARAMETER(HwDeviceExtension);
	UNREFERENCED_PARAMETER(i);

#if DBG
	VideoDebugPrint((2, "\nMapped Address Table\n-N-  Address     ProcId   refcount\n"));
	VideoDebugPrint((2, "--- ---------- ---------- --------\n"));
	for (i = 0; i < MAX_ADDRESS_TABLE_SIZE; i++)
	{
		if (HwDeviceExtension->AddressList[i] || HwDeviceExtension->AddressAllocCount[i])
#if (_WIN32_WINNT < 0x0500)
      VideoDebugPrint((2, "%03d 0x%08lx 0x%08lx    %2d\n",
                       i,
                       HwDeviceExtension->AddressList[i],
                       HwDeviceExtension->AddressProcess[i],
                       HwDeviceExtension->AddressAllocCount[i]));
#else
      VideoDebugPrint((2, "%03d 0x%08lx    %2d\n",
                       i,
                       HwDeviceExtension->AddressList[i],
                       HwDeviceExtension->AddressAllocCount[i]));
#endif
	}
	VideoDebugPrint((2, "--- ---------- ---------- --------\n\n"));
#endif
}
#endif

VOID
DumpAccessRanges (
	PVIDEO_ACCESS_RANGE ranges,
	UCHAR count
	)
/*++

Routine Description:

    Dump the access ranges

Arguments:

    ranges - the array of access ranges

    count - the last entry to dump

Return Value:

    none

--*/
{
	LONG n;

	UNREFERENCED_PARAMETER(ranges);

	VideoDebugPrint((2, "Access range dump:\n"));
	VideoDebugPrint((2, "Index   --PHYSICAL ADDRESS--  Range Length   InIoSpace\n"));

	for (n = 0; n < count; n++)
	{
		VideoDebugPrint((2, "  %d",	n));
		VideoDebugPrint((2, "     0x%08lx 0x%08lx", ranges[n].RangeStart.LowPart, ranges[n].RangeStart.HighPart));
		VideoDebugPrint((2, "  0x%08lx        %d\n", ranges[n].RangeLength, ranges[n].RangeInIoSpace));
	}
	VideoDebugPrint((2, "\n"));
}

VOID
DumpModeInformation(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG modeNumber,
    PH3_VIDEO_MODES ModeEntry
	)
/*++

Routine Description:

    Dump the Mode information

Arguments:

    modeNumber - the mode index

    ModeEntry - a pointer to the mode entry

Return Value:

    none

--*/
{
	UNREFERENCED_PARAMETER(HwDeviceExtension);
	UNREFERENCED_PARAMETER(modeNumber);
	UNREFERENCED_PARAMETER(ModeEntry);

	if (!ModeEntry)
	{
		VideoDebugPrint((0, "DumpModeInformation - ModeEntry is NULL!\n"));
		return;
	}

	VideoDebugPrint((2, "\nMode %d structure dump\n", modeNumber));
	VideoDebugPrint((2, "  Int10ModeNumberContiguous    = 0x%04lx\n", ModeEntry->Int10ModeNumberContiguous));
	VideoDebugPrint((2, "  Int10ModeNumberNoncontiguous = 0x%04lx\n", ModeEntry->Int10ModeNumberNoncontiguous));
	VideoDebugPrint((2, "  ScreenStrideContiguous       = %04ld\n", ModeEntry->ScreenStrideContiguous));
	VideoDebugPrint((2, "    Length                       = %04ld\n", ModeEntry->ModeInformation.Length));
	VideoDebugPrint((2, "    Index                        = %04ld\n", ModeEntry->ModeInformation.ModeIndex));
	VideoDebugPrint((2, "    VisScreenWidth               = %04ld\n", ModeEntry->ModeInformation.VisScreenWidth));
	VideoDebugPrint((2, "    VisScreenHeight              = %04ld\n", ModeEntry->ModeInformation.VisScreenHeight));
	VideoDebugPrint((2, "    ScreenStride                 = %04ld\n", ModeEntry->ModeInformation.ScreenStride));
	VideoDebugPrint((2, "    NumberOfPlanes               = %02ld\n", ModeEntry->ModeInformation.NumberOfPlanes));
	VideoDebugPrint((2, "    BitsPerPlane                 = %02ld\n", ModeEntry->ModeInformation.BitsPerPlane));
	VideoDebugPrint((2, "    Frequency                    = %04ld\n", ModeEntry->ModeInformation.Frequency));
	VideoDebugPrint((2, "    XMillimeter                  = %04ld\n", ModeEntry->ModeInformation.XMillimeter));
	VideoDebugPrint((2, "    YMillimeter                  = %04ld\n", ModeEntry->ModeInformation.YMillimeter));
	VideoDebugPrint((2, "    NumberRedBits                = %02ld\n", ModeEntry->ModeInformation.NumberRedBits));
	VideoDebugPrint((2, "    NumberGreenBits              = %02ld\n", ModeEntry->ModeInformation.NumberGreenBits));
	VideoDebugPrint((2, "    NumberBlueBits               = %02ld\n", ModeEntry->ModeInformation.NumberBlueBits));
	VideoDebugPrint((2, "    RedMask                      = 0x%06lx\n", ModeEntry->ModeInformation.RedMask));
	VideoDebugPrint((2, "    GreenMask                    = 0x%06lx\n", ModeEntry->ModeInformation.GreenMask));
	VideoDebugPrint((2, "    BlueMask                     = 0x%06lx\n", ModeEntry->ModeInformation.BlueMask));
	VideoDebugPrint((2, "    AttributeFlags               = %08ld\n", ModeEntry->ModeInformation.AttributeFlags));
	VideoDebugPrint((2, "    VideoMemoryBitmapWidth       = %06ld\n", ModeEntry->ModeInformation.VideoMemoryBitmapWidth));
	VideoDebugPrint((2, "    VideoMemoryBitmapHeight      = %06ld\n", ModeEntry->ModeInformation.VideoMemoryBitmapHeight));
	VideoDebugPrint((2, "    DriverSpecificAttributeFlags = 0x%08lx\n\n", ModeEntry->ModeInformation.DriverSpecificAttributeFlags));

}


#if DBG

#if 1
#define INPB(a)     _inp((a))
#define INPD(a)     _inpd((a))
#define OUTPB(a,b)  _outp((a),(b))
#define OUTPD(a,b)  _outpd((a),(b))
#else
#define INPB(a)     VideoPortReadPortUchar((PULONG)(a))
#define INPD(a)     VideoPortReadPortUlong((PULONG)(a))
#define OUTPB(a,b)  VideoPortWritePortUchar((PUCHAR)(a),(UCHAR)(b))
#define OUTPD(a,b)  VideoPortWritePortUlong((PULONG)(a),(ULONG)(b))
#endif

//#define ENABLE_PCI_CFG_CYCLES   0
#define ENABLE_PCI_CFG_CYCLES   1

#if ENABLE_PCI_CFG_CYCLES
#define PCI_ADDR_PORT		        0xCF8
#define PCI_DATA_PORT		        0xCFC

typedef struct tagCFGREG
{
  ULONG   res1   : 2;   // bits  0-1
  ULONG   regNum : 6;   // bits  2-7
  ULONG   func   : 3;   // bits  8-10
  ULONG   devNum : 5;   // bits 11-15
  ULONG   busNum : 8;   // bits 16-23
  ULONG   res2   : 7;   // bits 24-30
  ULONG   enable : 1;   // bit  31
} CFGREG;
#endif

typedef struct _H3_PCI_CONFIG         // offset
{
  USHORT  VendorID;                   // 0
  USHORT  DeviceID;                   // 2
  USHORT  Command;                    // 4
  USHORT  Status;                     // 6
  UCHAR   RevisionID;                 // 8
  UCHAR   ProgIf;                     // 9
  UCHAR   SubClass;                   // 10
  UCHAR   BaseClass;                  // 11
  UCHAR   CacheLineSize;              // 12
  UCHAR   LatencyTimer;               // 13
  UCHAR   HeaderType;                 // 14
  UCHAR   BIST;                       // 15

  union
  {
    struct _H3_PCI_HEADER_TYPE_0
    {
      ULONG   BaseAddresses[PCI_TYPE0_ADDRESSES]; // 16, 20, 24, 28, 32, 36
      ULONG   CIS;                    // 40
      USHORT  SubVendorID;            // 44
      USHORT  SubSystemID;            // 46
      ULONG   ROMBaseAddress;         // 48
      UCHAR   CapabilitiesPtr;        // 52
      UCHAR   Reserved1[3];           // 53, 54, 55
      ULONG   Reserved2;              // 56
      UCHAR   InterruptLine;          // 60
      UCHAR   InterruptPin;           // 61
      UCHAR   MinimumGrant;           // 62
      UCHAR   MaximumLatency;         // 63
    } type0;
  } u;

  ULONG   fabId;                      // 64
  UCHAR   Reserved3[76-64];           // 68
  ULONG   cfgStatus;                  // 76
  ULONG   cfgScratch;                 // 80
  ULONG   agpCapId;                   // 84
  ULONG   agpStatus;                  // 88
  ULONG   agpCommand;                 // 92
  ULONG   ACPICapId;                  // 96
  ULONG   ACPICntrlStatus;            // 100
} H3_PCI_CONFIG, *PH3_PCI_CONFIG;

static VOID MyDumpPCIConfigSpace(PHW_DEVICE_EXTENSION HwDeviceExtension, ULONG DebugLevel);

#if ENABLE_PCI_CFG_CYCLES
static ULONG FindPCISlot(USHORT,USHORT);

static ULONG
FindPCISlot(USHORT VendorId, USHORT DeviceId)
{
  CFGREG        cfgReg;
  unsigned int  busNum, devNum, func;
  unsigned int  data;


  VideoDebugPrint((3, "Installed PCI Devices\n"));
  VideoDebugPrint((3, "  Bus  Dev  Fcn  VenDevID\n"));

  memset(&cfgReg, 0, sizeof(cfgReg));
  cfgReg.enable = 1;

  for (busNum = 0; busNum < 256; busNum++)
  {
    cfgReg.busNum = busNum;
    for (devNum = 0; devNum < 32; devNum++)
    {
      cfgReg.devNum = devNum;
      for (func = 0; func < 8; func++)
      {
        cfgReg.func = func;

        _outpd(PCI_ADDR_PORT, *(unsigned int *)&cfgReg);
        data = _inpd(PCI_DATA_PORT);

        if (-1 != data)
        {
          VideoDebugPrint((3, "  %03ld  %03ld  %03ld  %08lXh\n",
                           cfgReg.busNum, cfgReg.devNum, cfgReg.func,
                           data));
#ifndef MAKELONG
#define MAKELONG(a, b)      (((a) & 0xffff) | (((b) & 0xffff) << 16))
#endif
          // is it the device we're looking for?
          if ((unsigned int)MAKELONG(VendorId, DeviceId) == data)
          {
            // we should check for multiple boards here
            return *(ULONG *)&cfgReg;
          }
        }
      } // func for loop
    } // devNum for loop
  } // busNum for loop
  return 0xFFFFFFFF;
}
#endif

static VOID
MyDumpPCIConfigSpace(PHW_DEVICE_EXTENSION HwDeviceExtension, ULONG DebugLevel)
{
	PUCHAR            pIO = (PUCHAR) HwDeviceExtension->MappedAddress[SST_IO_INDEX];
  PH3_MEMBASE0      RegisterMap = (PH3_MEMBASE0)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
  ULONG             vgaInit0;
  int               i;
  PCI_COMMON_CONFIG PCIBuffer;
  PH3_PCI_CONFIG    pH3PciBuffer;
  unsigned int      regNum;
  unsigned int      data;
#if ENABLE_PCI_CFG_CYCLES
  CFGREG            cfgReg;
#endif


  for (i = 0; i < 3; i++)
  {
    switch (i)
    {
      case 0:
        VideoDebugPrint((DebugLevel, "PCI Config Space using VideoPortGetBusData\n"));

        VideoPortGetBusData(HwDeviceExtension,
                  PCIConfiguration,
		              HwDeviceExtension->PCISlot.u.AsULONG,
                  &PCIBuffer,
                  0,
                  PCI_COMMON_HDR_LENGTH);

        break;

      case 1:
#if ENABLE_PCI_CFG_CYCLES
        data = FindPCISlot(HwDeviceExtension->PCIVendorID, HwDeviceExtension->PCIDeviceID);
        if (data == 0xFFFFFFFF)
          continue;

        cfgReg = *(CFGREG *)&data;
        VideoDebugPrint((DebugLevel, "PCI Config Space using I/O to CF8 & CFC\n"));
        for (regNum = 0; regNum < 64; regNum++)
        {
          cfgReg.regNum = regNum;
          _outpd(PCI_ADDR_PORT, *(unsigned int *)&cfgReg);
          data = _inpd(PCI_DATA_PORT);

          *(ULONG *)((ULONG *)&PCIBuffer + regNum) = data;
        }
#else
        continue;
#endif

        break;

      case 2:
        VideoDebugPrint((DebugLevel, "PCI Config Space using special Banshee access thru CR1C\n"));

        vgaInit0 = (int)RegisterMap->vgaInit0;
        RegisterMap->vgaInit0 &= 0xFFFFFF3F;
        for (regNum = 0; regNum < 64*4; regNum++)
        {
          OUTPB(pIO + 0xD4, 0x1C);
          OUTPB(pIO + 0xD5, (UCHAR)regNum);
          *(UCHAR *)((UCHAR *)&PCIBuffer + regNum) = INPB(pIO + 0xD5);
        }
        RegisterMap->vgaInit0 = (ULONG)vgaInit0;

        break;
    }


    VideoDebugPrint((DebugLevel, "  VendorID         = %04Xh\n",  PCIBuffer.VendorID                ));
    VideoDebugPrint((DebugLevel, "  DeviceID         = %04Xh\n",  PCIBuffer.DeviceID                ));
    VideoDebugPrint((DebugLevel, "  Command          = %04Xh\n",  PCIBuffer.Command                 ));
    VideoDebugPrint((DebugLevel, "  Status           = %04Xh\n",  PCIBuffer.Status                  ));
    VideoDebugPrint((DebugLevel, "  RevisionID       = %02Xh\n",  PCIBuffer.RevisionID              ));
    VideoDebugPrint((DebugLevel, "  ProgIf           = %02Xh\n",  PCIBuffer.ProgIf                  ));
    VideoDebugPrint((DebugLevel, "  SubClass         = %02Xh\n",  PCIBuffer.SubClass                ));
    VideoDebugPrint((DebugLevel, "  BaseClass        = %02Xh\n",  PCIBuffer.BaseClass               ));
    VideoDebugPrint((DebugLevel, "  CacheLineSize    = %02Xh\n",  PCIBuffer.CacheLineSize           ));
    VideoDebugPrint((DebugLevel, "  LatencyTimer     = %02Xh\n",  PCIBuffer.LatencyTimer            ));
    VideoDebugPrint((DebugLevel, "  HeaderType       = %02Xh\n",  PCIBuffer.HeaderType              ));
    VideoDebugPrint((DebugLevel, "  BIST             = %02Xh\n",  PCIBuffer.BIST                    ));
    VideoDebugPrint((DebugLevel, "  BaseAddresses[0] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[0]));
    VideoDebugPrint((DebugLevel, "  BaseAddresses[1] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[1]));
    VideoDebugPrint((DebugLevel, "  BaseAddresses[2] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[2]));
    VideoDebugPrint((DebugLevel, "  BaseAddresses[3] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[3]));
    VideoDebugPrint((DebugLevel, "  BaseAddresses[4] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[4]));
    VideoDebugPrint((DebugLevel, "  BaseAddresses[5] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[5]));
    VideoDebugPrint((DebugLevel, "  CIS              = %08lXh\n", PCIBuffer.u.type0.CIS             ));
    VideoDebugPrint((DebugLevel, "  SubVendorID      = %04Xh\n",  PCIBuffer.u.type0.SubVendorID     ));
    VideoDebugPrint((DebugLevel, "  SubSystemID      = %04Xh\n",  PCIBuffer.u.type0.SubSystemID     ));
    VideoDebugPrint((DebugLevel, "  ROMBaseAddress   = %08lXh\n", PCIBuffer.u.type0.ROMBaseAddress  ));
#if (_WIN32_WINNT >= 0x0500)
    VideoDebugPrint((DebugLevel, "  CapabilitiesPtr  = %02Xh\n",  PCIBuffer.u.type0.CapabilitiesPtr ));
#endif
    VideoDebugPrint((DebugLevel, "  InterruptLine    = %02Xh\n",  PCIBuffer.u.type0.InterruptLine   ));
    VideoDebugPrint((DebugLevel, "  InterruptPin     = %02Xh\n",  PCIBuffer.u.type0.InterruptPin    ));
    VideoDebugPrint((DebugLevel, "  MinimumGrant     = %02Xh\n",  PCIBuffer.u.type0.MinimumGrant    ));
    VideoDebugPrint((DebugLevel, "  MaximumLatency   = %02Xh\n",  PCIBuffer.u.type0.MaximumLatency  ));

    pH3PciBuffer = (PVOID)&PCIBuffer;
    VideoDebugPrint((DebugLevel, "   fabId           = %08lXh\n", pH3PciBuffer->fabId          ));
    VideoDebugPrint((DebugLevel, "   cfgStatus       = %08lXh\n", pH3PciBuffer->cfgStatus      ));
    VideoDebugPrint((DebugLevel, "   cfgScratch      = %08lXh\n", pH3PciBuffer->cfgScratch     ));
    VideoDebugPrint((DebugLevel, "   agpCapId        = %08lXh\n", pH3PciBuffer->agpCapId       ));
    VideoDebugPrint((DebugLevel, "   agpStatus       = %08lXh\n", pH3PciBuffer->agpStatus      ));
    VideoDebugPrint((DebugLevel, "   agpCommand      = %08lXh\n", pH3PciBuffer->agpCommand     ));
    VideoDebugPrint((DebugLevel, "   ACPICapId       = %08lXh\n", pH3PciBuffer->ACPICapId      ));
    VideoDebugPrint((DebugLevel, "   ACPICntrlStatus = %08lXh\n", pH3PciBuffer->ACPICntrlStatus));

  }
}

VOID
DumpH3Regs(PHW_DEVICE_EXTENSION HwDeviceExtension, ULONG DebugLevel)
{
	PUCHAR            pIO = (PUCHAR) HwDeviceExtension->MappedAddress[SST_IO_INDEX];
  PH3_MEMBASE0      RegisterMap = (PH3_MEMBASE0)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
#if REDUCED_MEMORY_MAPPINGS
  PH3_2D_REGISTERS  RegisterMap2D = (PH3_2D_REGISTERS)((UCHAR *)HwDeviceExtension->MappedAddress[SST_2D_REGS_INDEX]);
  PH3_3D_REGISTERS  RegisterMap3D = (PH3_3D_REGISTERS)((UCHAR *)HwDeviceExtension->MappedAddress[SST_3D_REGS_INDEX]);
#else
  PH3_2D_REGISTERS  RegisterMap2D = (PH3_2D_REGISTERS)((UCHAR *)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX] + SST_2D_OFFSET);
  PH3_3D_REGISTERS  RegisterMap3D = (PH3_3D_REGISTERS)((UCHAR *)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX] + SST_3D_OFFSET);
#endif
  int i;


  MyDumpPCIConfigSpace(HwDeviceExtension, DebugLevel);



  VideoDebugPrint((DebugLevel, "IO Base0 Regs\n"));

  VideoDebugPrint((DebugLevel, "  STATUS                       = %08lXh\n", INPD(pIO + STATUS                      )));
  VideoDebugPrint((DebugLevel, "  PCIINIT0                     = %08lXh\n", INPD(pIO + PCIINIT0                    )));
  VideoDebugPrint((DebugLevel, "  SIPMONITOR                   = %08lXh\n", INPD(pIO + SIPMONITOR                  )));
  VideoDebugPrint((DebugLevel, "  LFBMEMORYCONFIG              = %08lXh\n", INPD(pIO + LFBMEMORYCONFIG             )));
  VideoDebugPrint((DebugLevel, "  MISCINIT0                    = %08lXh\n", INPD(pIO + MISCINIT0                   )));
  VideoDebugPrint((DebugLevel, "  MISCINIT1                    = %08lXh\n", INPD(pIO + MISCINIT1                   )));
  VideoDebugPrint((DebugLevel, "  DRAMINIT0                    = %08lXh\n", INPD(pIO + DRAMINIT0                   )));
  VideoDebugPrint((DebugLevel, "  DRAMINIT1                    = %08lXh\n", INPD(pIO + DRAMINIT1                   )));
  VideoDebugPrint((DebugLevel, "  AGPINIT                      = %08lXh\n", INPD(pIO + AGPINIT                     )));
  VideoDebugPrint((DebugLevel, "  TMUGBEINIT                   = %08lXh\n", INPD(pIO + TMUGBEINIT                  )));
  VideoDebugPrint((DebugLevel, "  VGAINIT0                     = %08lXh\n", INPD(pIO + VGAINIT0                    )));
  VideoDebugPrint((DebugLevel, "  VGAINIT1                     = %08lXh\n", INPD(pIO + VGAINIT1                    )));
  VideoDebugPrint((DebugLevel, "  DRAMCOMMAND                  = %08lXh\n", INPD(pIO + DRAMCOMMAND                 )));
  VideoDebugPrint((DebugLevel, "  DRAMDATA                     = %08lXh\n", INPD(pIO + DRAMDATA                    )));
  VideoDebugPrint((DebugLevel, "  RESERVEDZ_0                  = %08lXh\n", INPD(pIO + RESERVEDZ_0                 )));
  VideoDebugPrint((DebugLevel, "  RESERVEDZ_1                  = %08lXh\n", INPD(pIO + RESERVEDZ_1                 )));
  VideoDebugPrint((DebugLevel, "  PLLCTRL0                     = %08lXh\n", INPD(pIO + PLLCTRL0                    )));
  VideoDebugPrint((DebugLevel, "  PLLCTRL1                     = %08lXh\n", INPD(pIO + PLLCTRL1                    )));
  VideoDebugPrint((DebugLevel, "  PLLCTRL2                     = %08lXh\n", INPD(pIO + PLLCTRL2                    )));
  VideoDebugPrint((DebugLevel, "  DACMODE                      = %08lXh\n", INPD(pIO + DACMODE                     )));
  VideoDebugPrint((DebugLevel, "  DACADDR                      = %08lXh\n", INPD(pIO + DACADDR                     )));
  VideoDebugPrint((DebugLevel, "  DACDATA                      = %08lXh\n", INPD(pIO + DACDATA                     )));
  VideoDebugPrint((DebugLevel, "  VIDMAXRGBDELTA               = %08lXh\n", INPD(pIO + VIDMAXRGBDELTA              )));
  VideoDebugPrint((DebugLevel, "  VIDPROCCFG                   = %08lXh\n", INPD(pIO + VIDPROCCFG                  )));
  VideoDebugPrint((DebugLevel, "  HWCURPATADDR                 = %08lXh\n", INPD(pIO + HWCURPATADDR                )));
  VideoDebugPrint((DebugLevel, "  HWCURLOC                     = %08lXh\n", INPD(pIO + HWCURLOC                    )));
  VideoDebugPrint((DebugLevel, "  HWCURC0                      = %08lXh\n", INPD(pIO + HWCURC0                     )));
  VideoDebugPrint((DebugLevel, "  HWCURC1                      = %08lXh\n", INPD(pIO + HWCURC1                     )));
  VideoDebugPrint((DebugLevel, "  VIDINFORMAT                  = %08lXh\n", INPD(pIO + VIDINFORMAT                 )));
  VideoDebugPrint((DebugLevel, "  VIDINSTATUS                  = %08lXh\n", INPD(pIO + VIDINSTATUS                 )));
  VideoDebugPrint((DebugLevel, "  VIDSERIALPARALLELPORT        = %08lXh\n", INPD(pIO + VIDSERIALPARALLELPORT       )));
  VideoDebugPrint((DebugLevel, "  VIDINXDECIMDELTAS            = %08lXh\n", INPD(pIO + VIDINXDECIMDELTAS           )));
  VideoDebugPrint((DebugLevel, "  VIDINDECIMINITERRS           = %08lXh\n", INPD(pIO + VIDINDECIMINITERRS          )));
  VideoDebugPrint((DebugLevel, "  VIDINYDECIMDELTAS            = %08lXh\n", INPD(pIO + VIDINYDECIMDELTAS           )));
  VideoDebugPrint((DebugLevel, "  VIDPIXELBUFTHOLD             = %08lXh\n", INPD(pIO + VIDPIXELBUFTHOLD            )));
  VideoDebugPrint((DebugLevel, "  VIDCHROMAMIN                 = %08lXh\n", INPD(pIO + VIDCHROMAMIN                )));
  VideoDebugPrint((DebugLevel, "  VIDCHROMAMAX                 = %08lXh\n", INPD(pIO + VIDCHROMAMAX                )));
  VideoDebugPrint((DebugLevel, "  VIDCURRENTLINE               = %08lXh\n", INPD(pIO + VIDCURRENTLINE              )));
  VideoDebugPrint((DebugLevel, "  VIDSCREENSIZE                = %08lXh\n", INPD(pIO + VIDSCREENSIZE               )));
  VideoDebugPrint((DebugLevel, "  VIDOVERLAYSTARTCOORDS        = %08lXh\n", INPD(pIO + VIDOVERLAYSTARTCOORDS       )));
  VideoDebugPrint((DebugLevel, "  VIDOVERLAYENDCOORDS          = %08lXh\n", INPD(pIO + VIDOVERLAYENDCOORDS         )));
  VideoDebugPrint((DebugLevel, "  VIDOVERLAYDUDX               = %08lXh\n", INPD(pIO + VIDOVERLAYDUDX              )));
  VideoDebugPrint((DebugLevel, "  VIDOVERLAYDUDXOFFSETSRCWIDTH = %08lXh\n", INPD(pIO + VIDOVERLAYDUDXOFFSETSRCWIDTH)));
  VideoDebugPrint((DebugLevel, "  VIDOVERLAYDVDY               = %08lXh\n", INPD(pIO + VIDOVERLAYDVDY              )));

  VideoDebugPrint((DebugLevel, "  VIDOVERLAYDVDYOFFSET         = %08lXh\n", INPD(pIO + VIDOVERLAYDVDYOFFSET        )));
  VideoDebugPrint((DebugLevel, "  VIDDESKTOPSTARTADDR          = %08lXh\n", INPD(pIO + VIDDESKTOPSTARTADDR         )));
  VideoDebugPrint((DebugLevel, "  VIDDESKTOPOVERLAYSTRIDE      = %08lXh\n", INPD(pIO + VIDDESKTOPOVERLAYSTRIDE     )));
  VideoDebugPrint((DebugLevel, "  VIDINADDR0                   = %08lXh\n", INPD(pIO + VIDINADDR0                  )));
  VideoDebugPrint((DebugLevel, "  VIDINADDR1                   = %08lXh\n", INPD(pIO + VIDINADDR1                  )));
  VideoDebugPrint((DebugLevel, "  VIDINADDR2                   = %08lXh\n", INPD(pIO + VIDINADDR2                  )));
  VideoDebugPrint((DebugLevel, "  VIDINSTRIDE                  = %08lXh\n", INPD(pIO + VIDINSTRIDE                 )));
  VideoDebugPrint((DebugLevel, "  VIDCURROVERLAYSTARTADDR      = %08lXh\n", INPD(pIO + VIDCURROVERLAYSTARTADDR     )));



  VideoDebugPrint((DebugLevel, "General Regs\n"));

  VideoDebugPrint((DebugLevel, "  MiscOutput         = %02Xh\n", INPB(pIO + 0xCC)));
  VideoDebugPrint((DebugLevel, "  Input Status 0     = %02Xh\n", INPB(pIO + 0xC2)));
  VideoDebugPrint((DebugLevel, "  Input Status 1     = %02Xh\n", INPB(pIO + 0xDA)));
  VideoDebugPrint((DebugLevel, "  Feature Control    = %02Xh\n", INPB(pIO + 0xCA)));
  VideoDebugPrint((DebugLevel, "  Motherboard Enable = %02Xh\n", INPB(pIO + 0xC3)));
  //VideoDebugPrint((DebugLevel, "  Adapter Enable     = %02Xh\n", INPB(pIO + ??)));
  VideoDebugPrint((DebugLevel, "  Subsystem Enable   = %02Xh\n", INPB(pIO + 0xCE)));



  VideoDebugPrint((DebugLevel, "CRTC Regs\n"));

  for (i = 0; i <= 0x18; i++)
  {
    OUTPB(pIO + 0xD4, i);
    VideoDebugPrint((DebugLevel, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xD5)));
  }
  for (i = 0x1A; i <= 0x1F; i++)
  {
    OUTPB(pIO + 0xD4, i);
    VideoDebugPrint((DebugLevel, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xD5)));
  }
  OUTPB(pIO + 0xD4, 0x22);
  VideoDebugPrint((DebugLevel, "  Index 22h = %02Xh\n", i, INPB(pIO + 0xD5)));
  OUTPB(pIO + 0xD4, 0x24);
  VideoDebugPrint((DebugLevel, "  Index 24h = %02Xh\n", i, INPB(pIO + 0xD5)));
  OUTPB(pIO + 0xD4, 0x26);
  VideoDebugPrint((DebugLevel, "  Index 26h = %02Xh\n", i, INPB(pIO + 0xD5)));



  VideoDebugPrint((DebugLevel, "Sequencer Regs\n"));

  for (i = 0; i <= 0x4; i++)
  {
    OUTPB(pIO + 0xC4, i);
    VideoDebugPrint((DebugLevel, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xC5)));
  }



  VideoDebugPrint((DebugLevel, "Graphics Controller Regs\n"));

  for (i = 0; i <= 0x8; i++)
  {
    OUTPB(pIO + 0xCE, i);
    VideoDebugPrint((DebugLevel, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xCF)));
  }



#if 0
  VideoDebugPrint((DebugLevel, "Attribute Controller Regs\n"));

  for (i = 0; i <= 0x14; i++)
  {
    OUTPB(pIO + 0xC0, i);
    VideoDebugPrint((DebugLevel, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xC0)));
  }
#endif



  VideoDebugPrint((DebugLevel, "MemBase0 Regs\n"));

  VideoDebugPrint((DebugLevel, "  status                       = %08lXh\n", RegisterMap->status                      ));
  VideoDebugPrint((DebugLevel, "  pciInit0                     = %08lXh\n", RegisterMap->pciInit0                    ));
  VideoDebugPrint((DebugLevel, "  sipMonitor                   = %08lXh\n", RegisterMap->sipMonitor                  ));
  VideoDebugPrint((DebugLevel, "  lfbMemoryConfig              = %08lXh\n", RegisterMap->lfbMemoryConfig             ));
  VideoDebugPrint((DebugLevel, "  miscInit0                    = %08lXh\n", RegisterMap->miscInit0                   ));
  VideoDebugPrint((DebugLevel, "  miscInit1                    = %08lXh\n", RegisterMap->miscInit1                   ));
  VideoDebugPrint((DebugLevel, "  dramInit0                    = %08lXh\n", RegisterMap->dramInit0                   ));
  VideoDebugPrint((DebugLevel, "  dramInit1                    = %08lXh\n", RegisterMap->dramInit1                   ));
  VideoDebugPrint((DebugLevel, "  agpInit                      = %08lXh\n", RegisterMap->agpInit                     ));
  VideoDebugPrint((DebugLevel, "  tmuGbeInit                   = %08lXh\n", RegisterMap->tmuGbeInit                  ));
  VideoDebugPrint((DebugLevel, "  vgaInit0                     = %08lXh\n", RegisterMap->vgaInit0                    ));
  VideoDebugPrint((DebugLevel, "  vgaInit1                     = %08lXh\n", RegisterMap->vgaInit1                    ));
  VideoDebugPrint((DebugLevel, "  dramCommand                  = %08lXh\n", RegisterMap->dramCommand                 ));
  VideoDebugPrint((DebugLevel, "  dramData                     = %08lXh\n", RegisterMap->dramData                    ));
  VideoDebugPrint((DebugLevel, "  _reserved00                  = %08lXh\n", RegisterMap->_reserved00                 ));
#define TVOUT_SUPPORTED 1
#ifdef TVOUT_SUPPORTED
  VideoDebugPrint((DebugLevel, "  vidTvOutBlankVCount          = %08lXh\n", RegisterMap->vidTvOutBlankVCount         ));
#else
  VideoDebugPrint((DebugLevel, "  _reserved01                  = %08lXh\n", RegisterMap->_reserved01                 ));
#endif
  VideoDebugPrint((DebugLevel, "  pllCtrl0                     = %08lXh\n", RegisterMap->pllCtrl0                    ));
  VideoDebugPrint((DebugLevel, "  pllCtrl1                     = %08lXh\n", RegisterMap->pllCtrl1                    ));
  VideoDebugPrint((DebugLevel, "  pllCtrl2                     = %08lXh\n", RegisterMap->pllCtrl2                    ));
  VideoDebugPrint((DebugLevel, "  dacMode                      = %08lXh\n", RegisterMap->dacMode                     ));
  VideoDebugPrint((DebugLevel, "  dacAddr                      = %08lXh\n", RegisterMap->dacAddr                     ));
  VideoDebugPrint((DebugLevel, "  dacData                      = %08lXh\n", RegisterMap->dacData                     ));
  VideoDebugPrint((DebugLevel, "  vidMaxRGBDelta               = %08lXh\n", RegisterMap->vidMaxRGBDelta              ));
  VideoDebugPrint((DebugLevel, "  vidProcCfg                   = %08lXh\n", RegisterMap->vidProcCfg                  ));
  VideoDebugPrint((DebugLevel, "  hwCurPatAddr                 = %08lXh\n", RegisterMap->hwCurPatAddr                ));
  VideoDebugPrint((DebugLevel, "  hwCurLoc                     = %08lXh\n", RegisterMap->hwCurLoc                    ));
  VideoDebugPrint((DebugLevel, "  hwCurC0                      = %08lXh\n", RegisterMap->hwCurC0                     ));
  VideoDebugPrint((DebugLevel, "  hwCurC1                      = %08lXh\n", RegisterMap->hwCurC1                     ));
  VideoDebugPrint((DebugLevel, "  vidInFormat                  = %08lXh\n", RegisterMap->vidInFormat                 ));
#ifdef TVOUT_SUPPORTED
  VideoDebugPrint((DebugLevel, "  vidTvOutBlankHCount          = %08lXh\n", RegisterMap->vidTvOutBlankHCount         ));
#else
  VideoDebugPrint((DebugLevel, "  vidInStatus                  = %08lXh\n", RegisterMap->vidInStatus                 ));
#endif
  VideoDebugPrint((DebugLevel, "  vidSerialParallelPort        = %08lXh\n", RegisterMap->vidSerialParallelPort       ));
  VideoDebugPrint((DebugLevel, "  vidInXDecimDeltas            = %08lXh\n", RegisterMap->vidInXDecimDeltas           ));
  VideoDebugPrint((DebugLevel, "  vidInDecimInitErrs           = %08lXh\n", RegisterMap->vidInDecimInitErrs          ));
  VideoDebugPrint((DebugLevel, "  vidInYDecimDeltas            = %08lXh\n", RegisterMap->vidInYDecimDeltas           ));
  VideoDebugPrint((DebugLevel, "  vidPixelBufThold             = %08lXh\n", RegisterMap->vidPixelBufThold            ));
  VideoDebugPrint((DebugLevel, "  vidChromaMin                 = %08lXh\n", RegisterMap->vidChromaMin                ));
  VideoDebugPrint((DebugLevel, "  vidChromaMax                 = %08lXh\n", RegisterMap->vidChromaMax                ));
  VideoDebugPrint((DebugLevel, "  vidCurrentLine               = %08lXh\n", RegisterMap->vidCurrentLine              ));
  VideoDebugPrint((DebugLevel, "  vidScreenSize                = %08lXh\n", RegisterMap->vidScreenSize               ));
  VideoDebugPrint((DebugLevel, "  vidOverlayStartCoords        = %08lXh\n", RegisterMap->vidOverlayStartCoords       ));
  VideoDebugPrint((DebugLevel, "  vidOverlayEndScreenCoords    = %08lXh\n", RegisterMap->vidOverlayEndScreenCoords   ));
  VideoDebugPrint((DebugLevel, "  vidOverlayDudx               = %08lXh\n", RegisterMap->vidOverlayDudx              ));
  VideoDebugPrint((DebugLevel, "  vidOverlayDudxOffsetSrcWidth = %08lXh\n", RegisterMap->vidOverlayDudxOffsetSrcWidth));
  VideoDebugPrint((DebugLevel, "  vidOverlayDvdy               = %08lXh\n", RegisterMap->vidOverlayDvdy              ));
  VideoDebugPrint((DebugLevel, "  vidOverlayDvdyOffset         = %08lXh\n", RegisterMap->vidOverlayDvdyOffset        ));
  VideoDebugPrint((DebugLevel, "  vidDesktopStartAddr          = %08lXh\n", RegisterMap->vidDesktopStartAddr         ));
  VideoDebugPrint((DebugLevel, "  vidDesktopOverlayStride      = %08lXh\n", RegisterMap->vidDesktopOverlayStride     ));
  VideoDebugPrint((DebugLevel, "  vidInAddr0                   = %08lXh\n", RegisterMap->vidInAddr0                  ));
  VideoDebugPrint((DebugLevel, "  vidInAddr1                   = %08lXh\n", RegisterMap->vidInAddr1                  ));
  VideoDebugPrint((DebugLevel, "  vidInAddr2                   = %08lXh\n", RegisterMap->vidInAddr2                  ));
  VideoDebugPrint((DebugLevel, "  vidInStride                  = %08lXh\n", RegisterMap->vidInStride                 ));
  VideoDebugPrint((DebugLevel, "  vidCurrOverlayStartAddr      = %08lXh\n", RegisterMap->vidCurrOverlayStartAddr     ));



  VideoDebugPrint((DebugLevel, "2D Regs\n"));

  VideoDebugPrint((DebugLevel, "  status           = %08lXh\n", RegisterMap2D->status          ));
  VideoDebugPrint((DebugLevel, "  unused           = %08lXh\n", RegisterMap2D->unused          ));
  VideoDebugPrint((DebugLevel, "  clip0Min         = %08lXh\n", RegisterMap2D->clip0Min        ));
  VideoDebugPrint((DebugLevel, "  clip0Max         = %08lXh\n", RegisterMap2D->clip0Max        ));
  VideoDebugPrint((DebugLevel, "  dstBaseAddr      = %08lXh\n", RegisterMap2D->dstBaseAddr     ));
  VideoDebugPrint((DebugLevel, "  dstFormat        = %08lXh\n", RegisterMap2D->dstFormat       ));
  VideoDebugPrint((DebugLevel, "  srcColorKeyMin   = %08lXh\n", RegisterMap2D->srcColorKeyMin  ));
  VideoDebugPrint((DebugLevel, "  srcColorKeyMax   = %08lXh\n", RegisterMap2D->srcColorKeyMax  ));
  VideoDebugPrint((DebugLevel, "  dstColorKeyMin   = %08lXh\n", RegisterMap2D->dstColorKeyMin  ));
  VideoDebugPrint((DebugLevel, "  dstColorKeyMax   = %08lXh\n", RegisterMap2D->dstColorKeyMax  ));
  VideoDebugPrint((DebugLevel, "  bresError0       = %08lXh\n", RegisterMap2D->bresError0      ));
  VideoDebugPrint((DebugLevel, "  bresError1       = %08lXh\n", RegisterMap2D->bresError1      ));
  VideoDebugPrint((DebugLevel, "  rop              = %08lXh\n", RegisterMap2D->rop             ));
  VideoDebugPrint((DebugLevel, "  srcBaseAddr      = %08lXh\n", RegisterMap2D->srcBaseAddr     ));
  VideoDebugPrint((DebugLevel, "  commandEx        = %08lXh\n", RegisterMap2D->commandEx       ));
  VideoDebugPrint((DebugLevel, "  lineStipple      = %08lXh\n", RegisterMap2D->lineStipple     ));
  VideoDebugPrint((DebugLevel, "  lineStyle        = %08lXh\n", RegisterMap2D->lineStyle       ));
  VideoDebugPrint((DebugLevel, "  pattern0alias    = %08lXh\n", RegisterMap2D->pattern0alias   ));
  VideoDebugPrint((DebugLevel, "  pattern1alias    = %08lXh\n", RegisterMap2D->pattern1alias   ));
  VideoDebugPrint((DebugLevel, "  clip1min         = %08lXh\n", RegisterMap2D->clip1min        ));
  VideoDebugPrint((DebugLevel, "  clip1max         = %08lXh\n", RegisterMap2D->clip1max        ));
  VideoDebugPrint((DebugLevel, "  srcFormat        = %08lXh\n", RegisterMap2D->srcFormat       ));
  VideoDebugPrint((DebugLevel, "  srcSize          = %08lXh\n", RegisterMap2D->srcSize         ));
  VideoDebugPrint((DebugLevel, "  srcXY            = %08lXh\n", RegisterMap2D->srcXY           ));
  VideoDebugPrint((DebugLevel, "  colorBack        = %08lXh\n", RegisterMap2D->colorBack       ));
  VideoDebugPrint((DebugLevel, "  colorFore        = %08lXh\n", RegisterMap2D->colorFore       ));
  VideoDebugPrint((DebugLevel, "  dstSize          = %08lXh\n", RegisterMap2D->dstSize         ));
  VideoDebugPrint((DebugLevel, "  dstXY            = %08lXh\n", RegisterMap2D->dstXY           ));
  VideoDebugPrint((DebugLevel, "  command          = %08lXh\n", RegisterMap2D->command         ));
#if 0
  for (i = 0; i < 16; i++)
  {
    VideoDebugPrint((DebugLevel, "  launchArea[%02ld]   = %08lXh\n", i, RegisterMap2D->launchArea[16]  ));
  }
#endif
  for (i = 0; i < 32; i++)
  {
    VideoDebugPrint((DebugLevel, "  colorPattern[%02ld] = %08lXh\n", i, RegisterMap2D->colorPattern[32]));
  }



#if 0
  VideoDebugPrint((DebugLevel, "3D regs\n"));

  VideoDebugPrint((DebugLevel, "  status             = %08lXh\n", RegisterMap3D->status              ));
  VideoDebugPrint((DebugLevel, "  intrCtrl           = %08lXh\n", RegisterMap3D->intrCtrl            ));
  VideoDebugPrint((DebugLevel, "  vA.x               = %08lXh\n", RegisterMap3D->vA.x                ));
  VideoDebugPrint((DebugLevel, "  vA.y               = %08lXh\n", RegisterMap3D->vA.y                ));
  VideoDebugPrint((DebugLevel, "  vB.x               = %08lXh\n", RegisterMap3D->vB.x                ));
  VideoDebugPrint((DebugLevel, "  vB.y               = %08lXh\n", RegisterMap3D->vB.y                ));
  VideoDebugPrint((DebugLevel, "  vC.x               = %08lXh\n", RegisterMap3D->vC.x                ));
  VideoDebugPrint((DebugLevel, "  vC.y               = %08lXh\n", RegisterMap3D->vC.y                ));
  VideoDebugPrint((DebugLevel, "  r                  = %08lXh\n", RegisterMap3D->r                   ));
  VideoDebugPrint((DebugLevel, "  g                  = %08lXh\n", RegisterMap3D->g                   ));
  VideoDebugPrint((DebugLevel, "  b                  = %08lXh\n", RegisterMap3D->b                   ));
  VideoDebugPrint((DebugLevel, "  z                  = %08lXh\n", RegisterMap3D->z                   ));
  VideoDebugPrint((DebugLevel, "  s                  = %08lXh\n", RegisterMap3D->s                   ));
  VideoDebugPrint((DebugLevel, "  t                  = %08lXh\n", RegisterMap3D->t                   ));
  VideoDebugPrint((DebugLevel, "  a                  = %08lXh\n", RegisterMap3D->a                   ));
  VideoDebugPrint((DebugLevel, "  w                  = %08lXh\n", RegisterMap3D->w                   ));
  VideoDebugPrint((DebugLevel, "  drdx               = %08lXh\n", RegisterMap3D->drdx                ));
  VideoDebugPrint((DebugLevel, "  dgdx               = %08lXh\n", RegisterMap3D->dgdx                ));
  VideoDebugPrint((DebugLevel, "  dbdx               = %08lXh\n", RegisterMap3D->dbdx                ));
  VideoDebugPrint((DebugLevel, "  dzdx               = %08lXh\n", RegisterMap3D->dzdx                ));
  VideoDebugPrint((DebugLevel, "  dadx               = %08lXh\n", RegisterMap3D->dadx                ));
  VideoDebugPrint((DebugLevel, "  dsdx               = %08lXh\n", RegisterMap3D->dsdx                ));
  VideoDebugPrint((DebugLevel, "  dtdx               = %08lXh\n", RegisterMap3D->dtdx                ));
  VideoDebugPrint((DebugLevel, "  dwdx               = %08lXh\n", RegisterMap3D->dwdx                ));
  VideoDebugPrint((DebugLevel, "  drdy               = %08lXh\n", RegisterMap3D->drdy                ));
  VideoDebugPrint((DebugLevel, "  dgdy               = %08lXh\n", RegisterMap3D->dgdy                ));
  VideoDebugPrint((DebugLevel, "  dbdy               = %08lXh\n", RegisterMap3D->dbdy                ));
  VideoDebugPrint((DebugLevel, "  dzdy               = %08lXh\n", RegisterMap3D->dzdy                ));
  VideoDebugPrint((DebugLevel, "  dady               = %08lXh\n", RegisterMap3D->dady                ));
  VideoDebugPrint((DebugLevel, "  dsdy               = %08lXh\n", RegisterMap3D->dsdy                ));
  VideoDebugPrint((DebugLevel, "  dtdy               = %08lXh\n", RegisterMap3D->dtdy                ));
  VideoDebugPrint((DebugLevel, "  dwdy               = %08lXh\n", RegisterMap3D->dwdy                ));
  VideoDebugPrint((DebugLevel, "  triangleCMD        = %08lXh\n", RegisterMap3D->triangleCMD         ));
  VideoDebugPrint((DebugLevel, "  FvA.x              = %08lXh\n", RegisterMap3D->FvA.x               ));
  VideoDebugPrint((DebugLevel, "  FvA.y              = %08lXh\n", RegisterMap3D->FvA.y               ));
  VideoDebugPrint((DebugLevel, "  FvB.x              = %08lXh\n", RegisterMap3D->FvB.x               ));
  VideoDebugPrint((DebugLevel, "  FvB.y              = %08lXh\n", RegisterMap3D->FvB.y               ));
  VideoDebugPrint((DebugLevel, "  FvC.x              = %08lXh\n", RegisterMap3D->FvC.x               ));
  VideoDebugPrint((DebugLevel, "  FvC.y              = %08lXh\n", RegisterMap3D->FvC.y               ));
  VideoDebugPrint((DebugLevel, "  Fr                 = %08lXh\n", RegisterMap3D->Fr                  ));
  VideoDebugPrint((DebugLevel, "  Fg                 = %08lXh\n", RegisterMap3D->Fg                  ));
  VideoDebugPrint((DebugLevel, "  Fb                 = %08lXh\n", RegisterMap3D->Fb                  ));
  VideoDebugPrint((DebugLevel, "  Fz                 = %08lXh\n", RegisterMap3D->Fz                  ));
  VideoDebugPrint((DebugLevel, "  Fs                 = %08lXh\n", RegisterMap3D->Fs                  ));
  VideoDebugPrint((DebugLevel, "  Ft                 = %08lXh\n", RegisterMap3D->Ft                  ));
  VideoDebugPrint((DebugLevel, "  Fa                 = %08lXh\n", RegisterMap3D->Fa                  ));
  VideoDebugPrint((DebugLevel, "  Fw                 = %08lXh\n", RegisterMap3D->Fw                  ));
  VideoDebugPrint((DebugLevel, "  Fdrdx              = %08lXh\n", RegisterMap3D->Fdrdx               ));
  VideoDebugPrint((DebugLevel, "  Fdgdx              = %08lXh\n", RegisterMap3D->Fdgdx               ));
  VideoDebugPrint((DebugLevel, "  Fdbdx              = %08lXh\n", RegisterMap3D->Fdbdx               ));
  VideoDebugPrint((DebugLevel, "  Fdzdx              = %08lXh\n", RegisterMap3D->Fdzdx               ));
  VideoDebugPrint((DebugLevel, "  Fdadx              = %08lXh\n", RegisterMap3D->Fdadx               ));
  VideoDebugPrint((DebugLevel, "  Fdsdx              = %08lXh\n", RegisterMap3D->Fdsdx               ));
  VideoDebugPrint((DebugLevel, "  Fdtdx              = %08lXh\n", RegisterMap3D->Fdtdx               ));
  VideoDebugPrint((DebugLevel, "  Fdwdx              = %08lXh\n", RegisterMap3D->Fdwdx               ));
  VideoDebugPrint((DebugLevel, "  Fdrdy              = %08lXh\n", RegisterMap3D->Fdrdy               ));
  VideoDebugPrint((DebugLevel, "  Fdgdy              = %08lXh\n", RegisterMap3D->Fdgdy               ));
  VideoDebugPrint((DebugLevel, "  Fdbdy              = %08lXh\n", RegisterMap3D->Fdbdy               ));
  VideoDebugPrint((DebugLevel, "  Fdzdy              = %08lXh\n", RegisterMap3D->Fdzdy               ));
  VideoDebugPrint((DebugLevel, "  Fdady              = %08lXh\n", RegisterMap3D->Fdady               ));
  VideoDebugPrint((DebugLevel, "  Fdsdy              = %08lXh\n", RegisterMap3D->Fdsdy               ));
  VideoDebugPrint((DebugLevel, "  Fdtdy              = %08lXh\n", RegisterMap3D->Fdtdy               ));
  VideoDebugPrint((DebugLevel, "  Fdwdy              = %08lXh\n", RegisterMap3D->Fdwdy               ));
  VideoDebugPrint((DebugLevel, "  FtriangleCMD       = %08lXh\n", RegisterMap3D->FtriangleCMD        ));
  VideoDebugPrint((DebugLevel, "  fbzColorPath       = %08lXh\n", RegisterMap3D->fbzColorPath        ));
  VideoDebugPrint((DebugLevel, "  fogMode            = %08lXh\n", RegisterMap3D->fogMode             ));
  VideoDebugPrint((DebugLevel, "  alphaMode          = %08lXh\n", RegisterMap3D->alphaMode           ));
  VideoDebugPrint((DebugLevel, "  fbzMode            = %08lXh\n", RegisterMap3D->fbzMode             ));
  VideoDebugPrint((DebugLevel, "  lfbMode            = %08lXh\n", RegisterMap3D->lfbMode             ));
  VideoDebugPrint((DebugLevel, "  clipLeftRight      = %08lXh\n", RegisterMap3D->clipLeftRight       ));
  VideoDebugPrint((DebugLevel, "  clipTopBottom      = %08lXh\n", RegisterMap3D->clipTopBottom       ));
  VideoDebugPrint((DebugLevel, "  nopCMD             = %08lXh\n", RegisterMap3D->nopCMD              ));
  VideoDebugPrint((DebugLevel, "  fastfillCMD        = %08lXh\n", RegisterMap3D->fastfillCMD         ));
  VideoDebugPrint((DebugLevel, "  swapbufferCMD      = %08lXh\n", RegisterMap3D->swapbufferCMD       ));
  VideoDebugPrint((DebugLevel, "  fogColor           = %08lXh\n", RegisterMap3D->fogColor            ));
  VideoDebugPrint((DebugLevel, "  zaColor            = %08lXh\n", RegisterMap3D->zaColor             ));
  VideoDebugPrint((DebugLevel, "  chromaKey          = %08lXh\n", RegisterMap3D->chromaKey           ));
  VideoDebugPrint((DebugLevel, "  chromaRange        = %08lXh\n", RegisterMap3D->chromaRange         ));
  VideoDebugPrint((DebugLevel, "  userIntrCmd        = %08lXh\n", RegisterMap3D->userIntrCmd         ));
  VideoDebugPrint((DebugLevel, "  stipple            = %08lXh\n", RegisterMap3D->stipple             ));
  VideoDebugPrint((DebugLevel, "  c0                 = %08lXh\n", RegisterMap3D->c0                  ));
  VideoDebugPrint((DebugLevel, "  c1                 = %08lXh\n", RegisterMap3D->c1                  ));
  VideoDebugPrint((DebugLevel, "  stats.fbiPixelsIn  = %08lXh\n", RegisterMap3D->stats.fbiPixelsIn   ));
  VideoDebugPrint((DebugLevel, "  stats.fbiChromaFail= %08lXh\n", RegisterMap3D->stats.fbiChromaFail ));
  VideoDebugPrint((DebugLevel, "  stats.fbiZfuncFail = %08lXh\n", RegisterMap3D->stats.fbiZfuncFail  ));
  VideoDebugPrint((DebugLevel, "  stats.fbiAfuncFail = %08lXh\n", RegisterMap3D->stats.fbiAfuncFail  ));
  VideoDebugPrint((DebugLevel, "  stats.fbiPixelsOut = %08lXh\n", RegisterMap3D->stats.fbiPixelsOut  ));
  for (i = 0; i < 32; i++)
  {
    VideoDebugPrint((DebugLevel, "  fogTable[%02ld]       = %08lXh\n", i, RegisterMap3D->fogTable[32]));
  }
  VideoDebugPrint((DebugLevel, "  renderMode         = %08lXh\n", RegisterMap3D->renderMode          ));
  VideoDebugPrint((DebugLevel, "  stencilMode        = %08lXh\n", RegisterMap3D->stencilMode         ));
  VideoDebugPrint((DebugLevel, "  stencilOp          = %08lXh\n", RegisterMap3D->stencilOp           ));
  VideoDebugPrint((DebugLevel, "  colBufferAddr      = %08lXh\n", RegisterMap3D->colBufferAddr       ));
  VideoDebugPrint((DebugLevel, "  colBufferStride    = %08lXh\n", RegisterMap3D->colBufferStride     ));
  VideoDebugPrint((DebugLevel, "  auxBufferAddr      = %08lXh\n", RegisterMap3D->auxBufferAddr       ));
  VideoDebugPrint((DebugLevel, "  auxBufferStride    = %08lXh\n", RegisterMap3D->auxBufferStride     ));
  VideoDebugPrint((DebugLevel, "  clipLeftRight1     = %08lXh\n", RegisterMap3D->clipLeftRight1      ));
  VideoDebugPrint((DebugLevel, "  clipTopBottom1     = %08lXh\n", RegisterMap3D->clipTopBottom1      ));
  VideoDebugPrint((DebugLevel, "  combineMode        = %08lXh\n", RegisterMap3D->combineMode         ));
  VideoDebugPrint((DebugLevel, "  sliCtrl            = %08lXh\n", RegisterMap3D->sliCtrl             ));
  VideoDebugPrint((DebugLevel, "  aaCtrl             = %08lXh\n", RegisterMap3D->aaCtrl              ));
  VideoDebugPrint((DebugLevel, "  chipMask           = %08lXh\n", RegisterMap3D->chipMask            ));
  VideoDebugPrint((DebugLevel, "  leftDesktopBuf     = %08lXh\n", RegisterMap3D->leftDesktopBuf      ));
  VideoDebugPrint((DebugLevel, "  swapBufferPend     = %08lXh\n", RegisterMap3D->swapBufferPend      ));
  VideoDebugPrint((DebugLevel, "  leftOverlayBuf     = %08lXh\n", RegisterMap3D->leftOverlayBuf      ));
  VideoDebugPrint((DebugLevel, "  rightOverlayBuf    = %08lXh\n", RegisterMap3D->rightOverlayBuf     ));
  VideoDebugPrint((DebugLevel, "  fbiSwapHistory     = %08lXh\n", RegisterMap3D->fbiSwapHistory      ));
  VideoDebugPrint((DebugLevel, "  fbiTrianglesOut    = %08lXh\n", RegisterMap3D->fbiTrianglesOut     ));
  VideoDebugPrint((DebugLevel, "  sSetupMode         = %08lXh\n", RegisterMap3D->sSetupMode          ));
  VideoDebugPrint((DebugLevel, "  sVx                = %08lXh\n", RegisterMap3D->sVx                 ));
  VideoDebugPrint((DebugLevel, "  sVy                = %08lXh\n", RegisterMap3D->sVy                 ));
  VideoDebugPrint((DebugLevel, "  sARGB              = %08lXh\n", RegisterMap3D->sARGB               ));
  VideoDebugPrint((DebugLevel, "  sRed               = %08lXh\n", RegisterMap3D->sRed                ));
  VideoDebugPrint((DebugLevel, "  sGreen             = %08lXh\n", RegisterMap3D->sGreen              ));
  VideoDebugPrint((DebugLevel, "  sBlue              = %08lXh\n", RegisterMap3D->sBlue               ));
  VideoDebugPrint((DebugLevel, "  sAlpha             = %08lXh\n", RegisterMap3D->sAlpha              ));
  VideoDebugPrint((DebugLevel, "  sVz                = %08lXh\n", RegisterMap3D->sVz                 ));
  VideoDebugPrint((DebugLevel, "  sOowfbi            = %08lXh\n", RegisterMap3D->sOowfbi             ));
  VideoDebugPrint((DebugLevel, "  sOow0              = %08lXh\n", RegisterMap3D->sOow0               ));
  VideoDebugPrint((DebugLevel, "  sSow0              = %08lXh\n", RegisterMap3D->sSow0               ));
  VideoDebugPrint((DebugLevel, "  sTow0              = %08lXh\n", RegisterMap3D->sTow0               ));
  VideoDebugPrint((DebugLevel, "  sOow1              = %08lXh\n", RegisterMap3D->sOow1               ));
  VideoDebugPrint((DebugLevel, "  sSow1              = %08lXh\n", RegisterMap3D->sSow1               ));
  VideoDebugPrint((DebugLevel, "  sTow1              = %08lXh\n", RegisterMap3D->sTow1               ));
  VideoDebugPrint((DebugLevel, "  sDrawTriCMD        = %08lXh\n", RegisterMap3D->sDrawTriCMD         ));
  VideoDebugPrint((DebugLevel, "  sBeginTriCMD       = %08lXh\n", RegisterMap3D->sBeginTriCMD        ));
  VideoDebugPrint((DebugLevel, "  textureMode        = %08lXh\n", RegisterMap3D->textureMode         ));
  VideoDebugPrint((DebugLevel, "  tLOD               = %08lXh\n", RegisterMap3D->tLOD                ));
  VideoDebugPrint((DebugLevel, "  tDetail            = %08lXh\n", RegisterMap3D->tDetail             ));
  VideoDebugPrint((DebugLevel, "  texBaseAddr        = %08lXh\n", RegisterMap3D->texBaseAddr         ));
  VideoDebugPrint((DebugLevel, "  texBaseAddr1       = %08lXh\n", RegisterMap3D->texBaseAddr1        ));
  VideoDebugPrint((DebugLevel, "  texBaseAddr2       = %08lXh\n", RegisterMap3D->texBaseAddr2        ));
  VideoDebugPrint((DebugLevel, "  texBaseAddr38      = %08lXh\n", RegisterMap3D->texBaseAddr38       ));
  VideoDebugPrint((DebugLevel, "  trexInit0          = %08lXh\n", RegisterMap3D->trexInit0           ));
  VideoDebugPrint((DebugLevel, "  trexInit1          = %08lXh\n", RegisterMap3D->trexInit1           ));
  for (i = 0; i < 12; i++)
  {
    VideoDebugPrint((DebugLevel, "  nccTable0[%02ld]      = %08lXh\n", i, RegisterMap3D->nccTable0[12]));
  }
  for (i = 0; i < 12; i++)
  {
    VideoDebugPrint((DebugLevel, "  nccTable1[%02ld]      = %08lXh\n", i, RegisterMap3D->nccTable1[12]));
  }
#endif
}

#define FLAGS_AC_BIT        0x00040000
#define FLAGS_ID_BIT        0x00200000

#define	STEPPING_MASK				0x0000000F
#define	MODEL_MASK					0x000000F0
#define	FAMILY_MASK					0x00000F00
#define	UP_TYPE_MASK				0x00003000

#define	FAMILY_386					0x00000300
#define	FAMILY_486					0x00000400
#define	FAMILY_PENTIUM			0x00000500
#define	FAMILY_PENTIUM_PRO	0x00000600

#define	FEATURE_FPU					0x00000001
#define	FEATURE_VME					0x00000002
#define	FEATURE_PSE					0x00000008
#define	FEATURE_TSC					0x00000010
#define	FEATURE_MSR					0x00000020
#define	FEATURE_PAE					0x00000040
#define	FEATURE_MCE					0x00000080
#define	FEATURE_CX8					0x00000100
#define	FEATURE_APIC				0x00000200
#define	FEATURE_MTRR				0x00001000
#define	FEATURE_PGE					0x00002000
#define	FEATURE_MCA					0x00004000
#define	FEATURE_CMOV				0x00008000
#define FEATURE_PAT         0x00010000
#define FEATURE_MMX         0x00800000

typedef struct tagCPUINFO
{
  unsigned long   VersionInfo;
  unsigned long   FeatureInfo;
} CPUINFO;

#define CPUID     __asm _emit 0x0F __asm _emit 0xA2

int
GetuPVersionInfo(CPUINFO *pCpuInfo)
{
  ULONG highestVal;
  int   rc;


  __asm
  {
      mov   esi,pCpuInfo

      pushfd
      pushad

      pushfd                  // transfer eflags to eax
      pop   eax

      mov   ecx,eax           // save copy of eflags

      xor   eax,FLAGS_ID_BIT  // toggle ID bit
      push  eax               // transfer eax back to eflags
      popfd
      pushfd                  // transfer eflags back to eax
      pop   eax

      // if ID bit did not stay put, the CPUID instruction is not
      // supported.  If CPUID is not supported, it must be a 386 or 486
      xor   eax,ecx
      jnz   has_cpuid

      mov   DWORD PTR [esi],0
      mov   DWORD PTR [esi+4],0
      xor   eax,eax
      jmp   done

has_cpuid:
      xor   eax,eax           // call cpuid with 0 in eax
      CPUID                   // to get vendor id string
                              // and highest value of eax recognized

      mov   highestVal,eax    // save highest value

      cmp   ebx,'uneG'        // ebx,edx,ecx should
      jne   notIntel          // have 'GenuineIntel'
      cmp   edx,'Ieni'
      jne   notIntel
      cmp   ecx,'letn'
      je    isIntel

notIntel:
      mov   DWORD PTR [esi],0
      mov   DWORD PTR [esi+4],0
      xor   eax,eax
      jmp   done

isIntel:
      mov   eax,1
      cmp   eax,highestVal
      jle   hasSig

      mov   DWORD PTR [esi],0
      mov   DWORD PTR [esi+4],0
      xor   eax,eax
      jmp   done

hasSig:
      CPUID
      mov   DWORD PTR [esi],eax
      mov   DWORD PTR [esi+4],edx
      mov   eax,1

done:
      mov   rc,eax
      popad
      popfd
  }

  return rc;
}

int
IsMTRRSupported(VOID)
{
  CPUINFO CpuInfo;


  if (GetuPVersionInfo(&CpuInfo) &&
      (FEATURE_MTRR & CpuInfo.FeatureInfo))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

int
IsPATSupported(VOID)
{
  CPUINFO CpuInfo;


  if (GetuPVersionInfo(&CpuInfo) &&
      (FEATURE_PAT & CpuInfo.FeatureInfo))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

#define MSR_MTRRcapsReg             0x0FE
#define MSR_MTRRphysBase0Reg        0x200
#define MSR_MTRRphysMask0Reg        0x201
#define MSR_MTRRphysBase1Reg        0x202
#define MSR_MTRRphysMask1Reg        0x203
#define MSR_MTRRphysBase2Reg        0x204
#define MSR_MTRRphysMask2Reg        0x205
#define MSR_MTRRphysBase3Reg        0x206
#define MSR_MTRRphysMask3Reg        0x207
#define MSR_MTRRphysBase4Reg        0x208
#define MSR_MTRRphysMask4Reg        0x209
#define MSR_MTRRphysBase5Reg        0x20A
#define MSR_MTRRphysMask5Reg        0x20B
#define MSR_MTRRphysBase6Reg        0x20C
#define MSR_MTRRphysMask6Reg        0x20D
#define MSR_MTRRphysBase7Reg        0x20E
#define MSR_MTRRphysMask7Reg        0x20F
#define MSR_MTRRfix64K_00000Reg     0x250
#define MSR_MTRRfix64K_80000Reg     0x258
#define MSR_MTRRfix64K_A0000Reg     0x259
#define MSR_MTRRfix64K_C0000Reg     0x268
#define MSR_MTRRfix64K_C8000Reg     0x269
#define MSR_MTRRfix64K_D0000Reg     0x26A
#define MSR_MTRRfix64K_D8000Reg     0x26B
#define MSR_MTRRfix64K_E0000Reg     0x26C
#define MSR_MTRRfix64K_E8000Reg     0x26D
#define MSR_MTRRfix64K_F0000Reg     0x26E
#define MSR_MTRRfix64K_F8000Reg     0x26F
#define MSR_MTRRdefTypeReg          0x2FF

#define MSR_PAT                     0x277

#define MTRRCap_WCBit               0x400
#define MTRRCap_FixBit              0x100
#define MTRRCap_VCNTMask            0x0FF

#define MTRRDef_DefMemTypeMask      0x0FF
#define MTRRDef_FixedMTRREnabledBit 0x0400
#define MTRRDef_MTRREnabledBit      0x0800

#define MTRRPhysBase_TypeMask       0xFF
#define MTRRPhysBase_PhysBaseHi     0x3F
#define MTRRPhysBase_PhysBaseLo     0xFFFFF000
#define MTRRPhysMask_ValidBit       0x0800
#define MTRRPhysMask_PhysMaskHi     0x3F
#define MTRRPhysMask_PhysMaskLo     0xFFFFF000

#define RDMSR   __asm _emit 0x0F __asm _emit 0x32

VOID
DumpVariableMTRRInfo(ULONG baseHi, ULONG baseLo, ULONG maskHi, ULONG maskLo, ULONG DebugLevel)
{
  ULONG start, end;


  if (maskLo & MTRRPhysMask_ValidBit)
  {
    VideoDebugPrint((DebugLevel, "    PhysBase = %02lX%08lXh\n", baseHi & MTRRPhysBase_PhysBaseHi, baseLo & MTRRPhysBase_PhysBaseLo));
    VideoDebugPrint((DebugLevel, "    PhysMask = %02lX%08lXh\n", maskHi & MTRRPhysMask_PhysMaskHi, maskLo & MTRRPhysMask_PhysMaskLo));
    VideoDebugPrint((DebugLevel, "    Type     = "));
    switch (baseLo & MTRRDef_DefMemTypeMask)
    {
      case 0:
        VideoDebugPrint((DebugLevel, "Uncacheable  (0)\n"));
        break;
      case 1:
        VideoDebugPrint((DebugLevel, "Write Combining  (1)\n"));
        break;
      case 4:
        VideoDebugPrint((DebugLevel, "Write-through  (4)\n"));
        break;
      case 5:
        VideoDebugPrint((DebugLevel, "Write-protected  (5)\n"));
        break;
      case 6:
        VideoDebugPrint((DebugLevel, "Writeback  (6)\n"));
        break;
      default:
        VideoDebugPrint((DebugLevel, "reserved  (%ld)\n", baseLo & MTRRDef_DefMemTypeMask));
        break;
    }
    start = baseLo & MTRRPhysBase_PhysBaseLo;
    end   = start + ~(maskLo & MTRRPhysMask_PhysMaskLo);
    VideoDebugPrint((DebugLevel, "    range(?) = %08lXh - %08lX  (%ldMB)  this may be bogus\n",
                     start, end, (end - start + 1) / (1024 * 1024)));
  }
}

VOID
DumpPATInfo(ULONG patMSRHi, ULONG patMSRLo, ULONG DebugLevel)
{
  ULONG i, pat;

  for (i = 0; i < 8; i++)
  {
    if (i < 4)
      pat = (patMSRLo >> (i * 8)) & 0x07;
    else
      pat = (patMSRHi >> ((i - 4) * 8)) & 0x07;

    VideoDebugPrint((DebugLevel, "    PA%ld = %lXh - ", i, pat));
    switch (pat)
    {
      case 0:
        VideoDebugPrint((DebugLevel, "Uncacheable\n"));
        break;
      case 1:
        VideoDebugPrint((DebugLevel, "Write Combining\n"));
        break;
      case 4:
        VideoDebugPrint((DebugLevel, "Write-through\n"));
        break;
      case 5:
        VideoDebugPrint((DebugLevel, "Write-protected\n"));
        break;
      case 6:
        VideoDebugPrint((DebugLevel, "Writeback\n"));
        break;
      case 7:
        VideoDebugPrint((DebugLevel, "Uncached\n"));
        break;
      default:
        VideoDebugPrint((DebugLevel, "reserved\n"));
        break;
    }
  }
}

VOID
DumpMTRRs(PHW_DEVICE_EXTENSION HwDeviceExtension, ULONG DebugLevel)
{
  ULONG dataLo;
  ULONG dataHi;
  ULONG baseLo;
  ULONG baseHi;


  // first make sure the cpu supports MTRR's
  if (! IsMTRRSupported())
    return;

  __asm
  {
      mov   ecx,MSR_MTRRcapsReg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRCaps         = %08lX%08lXh\n", dataHi, dataLo));
  VideoDebugPrint((DebugLevel, "    Write Combining     %s\n", (dataLo & MTRRCap_WCBit) ? "enabled" : "disabled"));
  VideoDebugPrint((DebugLevel, "    FixedRangeRegisters %s\n", (dataLo & MTRRCap_FixBit) ? "enabled" : "disabled"));
  VideoDebugPrint((DebugLevel, "    Variable Range Reg Count = %ld\n", dataLo & MTRRCap_VCNTMask));

  __asm
  {
      mov   ecx,MSR_MTRRdefTypeReg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRdefType      = %08lX%08lXh\n", dataHi, dataLo));
  VideoDebugPrint((DebugLevel, "    MTRR's       %s\n", (dataLo & MTRRDef_MTRREnabledBit) ? "enabled" : "disabled"));
  VideoDebugPrint((DebugLevel, "    Fixed MTRR's %s\n", (dataLo & MTRRDef_FixedMTRREnabledBit) ? "enabled" : "disabled"));
  VideoDebugPrint((DebugLevel, "    Default Memory Type = "));
  switch (dataLo & MTRRDef_DefMemTypeMask)
  {
    case 0:
      VideoDebugPrint((DebugLevel, "Uncacheable  (0)\n"));
      break;
    case 1:
      VideoDebugPrint((DebugLevel, "Write Combining  (1)\n"));
      break;
    case 4:
      VideoDebugPrint((DebugLevel, "Write-through  (4)\n"));
      break;
    case 5:
      VideoDebugPrint((DebugLevel, "Write-protected  (5)\n"));
      break;
    case 6:
      VideoDebugPrint((DebugLevel, "Writeback  (6)\n"));
      break;
    default:
      VideoDebugPrint((DebugLevel, "reserved  (%ld)\n", dataLo & MTRRDef_DefMemTypeMask));
      break;
  }

  __asm
  {
      mov   ecx,MSR_MTRRphysBase0Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysBase0    = %08lX%08lXh\n", dataHi, dataLo));
  baseLo = dataLo;
  baseHi = dataHi;

  __asm
  {
      mov   ecx,MSR_MTRRphysMask0Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysMask0    = %08lX%08lXh\n", dataHi, dataLo));
  DumpVariableMTRRInfo(baseHi, baseLo, dataHi, dataLo, DebugLevel);

  __asm
  {
      mov   ecx,MSR_MTRRphysBase1Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysBase1    = %08lX%08lXh\n", dataHi, dataLo));
  baseLo = dataLo;
  baseHi = dataHi;

  __asm
  {
      mov   ecx,MSR_MTRRphysMask1Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysMask1    = %08lX%08lXh\n", dataHi, dataLo));
  DumpVariableMTRRInfo(baseHi, baseLo, dataHi, dataLo, DebugLevel);

  __asm
  {
      mov   ecx,MSR_MTRRphysBase2Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysBase2    = %08lX%08lXh\n", dataHi, dataLo));
  baseLo = dataLo;
  baseHi = dataHi;

  __asm
  {
      mov   ecx,MSR_MTRRphysMask2Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysMask2    = %08lX%08lXh\n", dataHi, dataLo));
  DumpVariableMTRRInfo(baseHi, baseLo, dataHi, dataLo, DebugLevel);

  __asm
  {
      mov   ecx,MSR_MTRRphysBase3Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysBase3    = %08lX%08lXh\n", dataHi, dataLo));
  baseLo = dataLo;
  baseHi = dataHi;

  __asm
  {
      mov   ecx,MSR_MTRRphysMask3Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysMask3    = %08lX%08lXh\n", dataHi, dataLo));
  DumpVariableMTRRInfo(baseHi, baseLo, dataHi, dataLo, DebugLevel);

  __asm
  {
      mov   ecx,MSR_MTRRphysBase4Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysBase4    = %08lX%08lXh\n", dataHi, dataLo));
  baseLo = dataLo;
  baseHi = dataHi;

  __asm
  {
      mov   ecx,MSR_MTRRphysMask4Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysMask4    = %08lX%08lXh\n", dataHi, dataLo));
  DumpVariableMTRRInfo(baseHi, baseLo, dataHi, dataLo, DebugLevel);

  __asm
  {
      mov   ecx,MSR_MTRRphysBase5Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysBase5    = %08lX%08lXh\n", dataHi, dataLo));
  baseLo = dataLo;
  baseHi = dataHi;

  __asm
  {
      mov   ecx,MSR_MTRRphysMask5Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysMask5    = %08lX%08lXh\n", dataHi, dataLo));
  DumpVariableMTRRInfo(baseHi, baseLo, dataHi, dataLo, DebugLevel);

  __asm
  {
      mov   ecx,MSR_MTRRphysBase6Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysBase6    = %08lX%08lXh\n", dataHi, dataLo));
  baseLo = dataLo;
  baseHi = dataHi;

  __asm
  {
      mov   ecx,MSR_MTRRphysMask6Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysMask6    = %08lX%08lXh\n", dataHi, dataLo));
  DumpVariableMTRRInfo(baseHi, baseLo, dataHi, dataLo, DebugLevel);

  __asm
  {
      mov   ecx,MSR_MTRRphysBase7Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysBase7    = %08lX%08lXh\n", dataHi, dataLo));
  baseLo = dataLo;
  baseHi = dataHi;

  __asm
  {
      mov   ecx,MSR_MTRRphysMask7Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRphysMask7    = %08lX%08lXh\n", dataHi, dataLo));
  DumpVariableMTRRInfo(baseHi, baseLo, dataHi, dataLo, DebugLevel);

  __asm
  {
      mov   ecx,MSR_MTRRfix64K_00000Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRfix64K_00000 = %08lX%08lXh\n", dataHi, dataLo));

  __asm
  {
      mov   ecx,MSR_MTRRfix64K_80000Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRfix64K_80000 = %08lX%08lXh\n", dataHi, dataLo));

  __asm
  {
      mov   ecx,MSR_MTRRfix64K_A0000Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRfix64K_A0000 = %08lX%08lXh\n", dataHi, dataLo));

  __asm
  {
      mov   ecx,MSR_MTRRfix64K_C0000Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRfix64K_C0000 = %08lX%08lXh\n", dataHi, dataLo));

  __asm
  {
      mov   ecx,MSR_MTRRfix64K_C8000Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRfix64K_C8000 = %08lX%08lXh\n", dataHi, dataLo));

  __asm
  {
      mov   ecx,MSR_MTRRfix64K_D0000Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRfix64K_D0000 = %08lX%08lXh\n", dataHi, dataLo));

  __asm
  {
      mov   ecx,MSR_MTRRfix64K_D8000Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRfix64K_D8000 = %08lX%08lXh\n", dataHi, dataLo));

  __asm
  {
      mov   ecx,MSR_MTRRfix64K_E0000Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRfix64K_E0000 = %08lX%08lXh\n", dataHi, dataLo));

  __asm
  {
      mov   ecx,MSR_MTRRfix64K_E8000Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRfix64K_E8000 = %08lX%08lXh\n", dataHi, dataLo));

  __asm
  {
      mov   ecx,MSR_MTRRfix64K_F0000Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRfix64K_F0000 = %08lX%08lXh\n", dataHi, dataLo));

  __asm
  {
      mov   ecx,MSR_MTRRfix64K_F8000Reg
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "MTRRfix64K_F8000 = %08lX%08lXh\n", dataHi, dataLo));


  // make sure the cpu supports PAT's
  if (! IsPATSupported())
  {
    VideoDebugPrint((DebugLevel, "CPU doesn't support Page Attribute Table (PAT)\n"));
    VideoDebugPrint((DebugLevel, "  The framebuffer should be shown above in one of the MTRR's\n"));
    return;
  }

  __asm
  {
      mov   ecx,MSR_PAT
      RDMSR
      mov   dataLo,eax
      mov   dataHi,edx
  }
  VideoDebugPrint((DebugLevel, "PAT MSR = %08lX%08lXh\n", dataHi, dataLo));
  DumpPATInfo(dataHi, dataLo, DebugLevel);

  __asm
  {
      mov   eax,cr3
      mov   dataLo,eax
  }
  VideoDebugPrint((DebugLevel, "CR3 = %08lXh\n", dataLo));
  VideoDebugPrint((DebugLevel, "    Page Directory Base = %08lXh\n", dataLo & 0xFFFFF000));
  VideoDebugPrint((DebugLevel, "    PCD = %ld\n", dataLo & 0x10));
  VideoDebugPrint((DebugLevel, "    PWT = %ld\n", dataLo & 0x8));
}
#endif

