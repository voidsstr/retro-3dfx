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

		if (((unsigned long)pInfo->ulAddress - (unsigned long)HwDeviceExtension->AddressList[k]) < 0x1000000)
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
#define OUTPD(a,b)  _outpd((a))
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

static VOID MyDumpPCIConfigSpace(PHW_DEVICE_EXTENSION HwDeviceExtension);

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
MyDumpPCIConfigSpace(PHW_DEVICE_EXTENSION HwDeviceExtension)
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
        VideoDebugPrint((3, "PCI Config Space using VideoPortGetBusData\n"));

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
        VideoDebugPrint((3, "PCI Config Space using I/O to CF8 & CFC\n"));
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
        VideoDebugPrint((3, "PCI Config Space using special Banshee access thru CR1C\n"));

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


    VideoDebugPrint((3, "  VendorID         = %04Xh\n",  PCIBuffer.VendorID                ));
    VideoDebugPrint((3, "  DeviceID         = %04Xh\n",  PCIBuffer.DeviceID                ));
    VideoDebugPrint((3, "  Command          = %04Xh\n",  PCIBuffer.Command                 ));
    VideoDebugPrint((3, "  Status           = %04Xh\n",  PCIBuffer.Status                  ));
    VideoDebugPrint((3, "  RevisionID       = %02Xh\n",  PCIBuffer.RevisionID              ));
    VideoDebugPrint((3, "  ProgIf           = %02Xh\n",  PCIBuffer.ProgIf                  ));
    VideoDebugPrint((3, "  SubClass         = %02Xh\n",  PCIBuffer.SubClass                ));
    VideoDebugPrint((3, "  BaseClass        = %02Xh\n",  PCIBuffer.BaseClass               ));
    VideoDebugPrint((3, "  CacheLineSize    = %02Xh\n",  PCIBuffer.CacheLineSize           ));
    VideoDebugPrint((3, "  LatencyTimer     = %02Xh\n",  PCIBuffer.LatencyTimer            ));
    VideoDebugPrint((3, "  HeaderType       = %02Xh\n",  PCIBuffer.HeaderType              ));
    VideoDebugPrint((3, "  BIST             = %02Xh\n",  PCIBuffer.BIST                    ));
    VideoDebugPrint((3, "  BaseAddresses[0] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[0]));
    VideoDebugPrint((3, "  BaseAddresses[1] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[1]));
    VideoDebugPrint((3, "  BaseAddresses[2] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[2]));
    VideoDebugPrint((3, "  BaseAddresses[3] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[3]));
    VideoDebugPrint((3, "  BaseAddresses[4] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[4]));
    VideoDebugPrint((3, "  BaseAddresses[5] = %08lXh\n", PCIBuffer.u.type0.BaseAddresses[5]));
    VideoDebugPrint((3, "  CIS              = %08lXh\n", PCIBuffer.u.type0.CIS             ));
    VideoDebugPrint((3, "  SubVendorID      = %04Xh\n",  PCIBuffer.u.type0.SubVendorID     ));
    VideoDebugPrint((3, "  SubSystemID      = %04Xh\n",  PCIBuffer.u.type0.SubSystemID     ));
    VideoDebugPrint((3, "  ROMBaseAddress   = %08lXh\n", PCIBuffer.u.type0.ROMBaseAddress  ));
#if (_WIN32_WINNT >= 0x0500)
    VideoDebugPrint((3, "  CapabilitiesPtr  = %02Xh\n",  PCIBuffer.u.type0.CapabilitiesPtr ));
#endif
    VideoDebugPrint((3, "  InterruptLine    = %02Xh\n",  PCIBuffer.u.type0.InterruptLine   ));
    VideoDebugPrint((3, "  InterruptPin     = %02Xh\n",  PCIBuffer.u.type0.InterruptPin    ));
    VideoDebugPrint((3, "  MinimumGrant     = %02Xh\n",  PCIBuffer.u.type0.MinimumGrant    ));
    VideoDebugPrint((3, "  MaximumLatency   = %02Xh\n",  PCIBuffer.u.type0.MaximumLatency  ));

    pH3PciBuffer = (PVOID)&PCIBuffer;
    VideoDebugPrint((3, "   fabId           = %08lXh\n", pH3PciBuffer->fabId          ));
    VideoDebugPrint((3, "   cfgStatus       = %08lXh\n", pH3PciBuffer->cfgStatus      ));
    VideoDebugPrint((3, "   cfgScratch      = %08lXh\n", pH3PciBuffer->cfgScratch     ));
    VideoDebugPrint((3, "   agpCapId        = %08lXh\n", pH3PciBuffer->agpCapId       ));
    VideoDebugPrint((3, "   agpStatus       = %08lXh\n", pH3PciBuffer->agpStatus      ));
    VideoDebugPrint((3, "   agpCommand      = %08lXh\n", pH3PciBuffer->agpCommand     ));
    VideoDebugPrint((3, "   ACPICapId       = %08lXh\n", pH3PciBuffer->ACPICapId      ));
    VideoDebugPrint((3, "   ACPICntrlStatus = %08lXh\n", pH3PciBuffer->ACPICntrlStatus));

  }
}

VOID
DumpH3Regs(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
	PUCHAR            pIO = (PUCHAR) HwDeviceExtension->MappedAddress[SST_IO_INDEX];
  PH3_MEMBASE0      RegisterMap = (PH3_MEMBASE0)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
#if REDUCED_MEMORY_MAPPINGS
  PH3_2D_REGISTERS  RegisterMap2D = (PH3_2D_REGISTERS)((UCHAR *)HwDeviceExtension->MappedAddress[SST_2D_REGS_INDEX]);
  PH3_3D_REGISTERS  RegisterMap3D = (PH3_3D_REGISTERS)((UCHAR *)HwDeviceExtension->MappedAddress[SST_3D_REGS_INDEX]);
#else
  PH3_2D_REGISTERS  RegisterMap2D = (PH3_2D_REGISTERS)((UCHAR *)HwDeviceExtension->MappedAddress[MEMBASE_ZERO] + SST_2D_OFFSET);
  PH3_3D_REGISTERS  RegisterMap3D = (PH3_3D_REGISTERS)((UCHAR *)HwDeviceExtension->MappedAddress[MEMBASE_ZERO] + SST_3D_OFFSET);
#endif
  int i;


  MyDumpPCIConfigSpace(HwDeviceExtension);



  VideoDebugPrint((3, "IO Base0 Regs\n"));

  VideoDebugPrint((3, "  STATUS                       = %08lXh\n", INPD(pIO + STATUS                      )));
  VideoDebugPrint((3, "  PCIINIT0                     = %08lXh\n", INPD(pIO + PCIINIT0                    )));
  VideoDebugPrint((3, "  SIPMONITOR                   = %08lXh\n", INPD(pIO + SIPMONITOR                  )));
  VideoDebugPrint((3, "  LFBMEMORYCONFIG              = %08lXh\n", INPD(pIO + LFBMEMORYCONFIG             )));
  VideoDebugPrint((3, "  MISCINIT0                    = %08lXh\n", INPD(pIO + MISCINIT0                   )));
  VideoDebugPrint((3, "  MISCINIT1                    = %08lXh\n", INPD(pIO + MISCINIT1                   )));
  VideoDebugPrint((3, "  DRAMINIT0                    = %08lXh\n", INPD(pIO + DRAMINIT0                   )));
  VideoDebugPrint((3, "  DRAMINIT1                    = %08lXh\n", INPD(pIO + DRAMINIT1                   )));
  VideoDebugPrint((3, "  AGPINIT                      = %08lXh\n", INPD(pIO + AGPINIT                     )));
  VideoDebugPrint((3, "  TMUGBEINIT                   = %08lXh\n", INPD(pIO + TMUGBEINIT                  )));
  VideoDebugPrint((3, "  VGAINIT0                     = %08lXh\n", INPD(pIO + VGAINIT0                    )));
  VideoDebugPrint((3, "  VGAINIT1                     = %08lXh\n", INPD(pIO + VGAINIT1                    )));
  VideoDebugPrint((3, "  DRAMCOMMAND                  = %08lXh\n", INPD(pIO + DRAMCOMMAND                 )));
  VideoDebugPrint((3, "  DRAMDATA                     = %08lXh\n", INPD(pIO + DRAMDATA                    )));
  VideoDebugPrint((3, "  RESERVEDZ_0                  = %08lXh\n", INPD(pIO + RESERVEDZ_0                 )));
  VideoDebugPrint((3, "  RESERVEDZ_1                  = %08lXh\n", INPD(pIO + RESERVEDZ_1                 )));
  VideoDebugPrint((3, "  PLLCTRL0                     = %08lXh\n", INPD(pIO + PLLCTRL0                    )));
  VideoDebugPrint((3, "  PLLCTRL1                     = %08lXh\n", INPD(pIO + PLLCTRL1                    )));
  VideoDebugPrint((3, "  PLLCTRL2                     = %08lXh\n", INPD(pIO + PLLCTRL2                    )));
  VideoDebugPrint((3, "  DACMODE                      = %08lXh\n", INPD(pIO + DACMODE                     )));
  VideoDebugPrint((3, "  DACADDR                      = %08lXh\n", INPD(pIO + DACADDR                     )));
  VideoDebugPrint((3, "  DACDATA                      = %08lXh\n", INPD(pIO + DACDATA                     )));
  VideoDebugPrint((3, "  VIDMAXRGBDELTA               = %08lXh\n", INPD(pIO + VIDMAXRGBDELTA              )));
  VideoDebugPrint((3, "  VIDPROCCFG                   = %08lXh\n", INPD(pIO + VIDPROCCFG                  )));
  VideoDebugPrint((3, "  HWCURPATADDR                 = %08lXh\n", INPD(pIO + HWCURPATADDR                )));
  VideoDebugPrint((3, "  HWCURLOC                     = %08lXh\n", INPD(pIO + HWCURLOC                    )));
  VideoDebugPrint((3, "  HWCURC0                      = %08lXh\n", INPD(pIO + HWCURC0                     )));
  VideoDebugPrint((3, "  HWCURC1                      = %08lXh\n", INPD(pIO + HWCURC1                     )));
  VideoDebugPrint((3, "  VIDINFORMAT                  = %08lXh\n", INPD(pIO + VIDINFORMAT                 )));
  VideoDebugPrint((3, "  VIDINSTATUS                  = %08lXh\n", INPD(pIO + VIDINSTATUS                 )));
  VideoDebugPrint((3, "  VIDSERIALPARALLELPORT        = %08lXh\n", INPD(pIO + VIDSERIALPARALLELPORT       )));
  VideoDebugPrint((3, "  VIDINXDECIMDELTAS            = %08lXh\n", INPD(pIO + VIDINXDECIMDELTAS           )));
  VideoDebugPrint((3, "  VIDINDECIMINITERRS           = %08lXh\n", INPD(pIO + VIDINDECIMINITERRS          )));
  VideoDebugPrint((3, "  VIDINYDECIMDELTAS            = %08lXh\n", INPD(pIO + VIDINYDECIMDELTAS           )));
  VideoDebugPrint((3, "  VIDPIXELBUFTHOLD             = %08lXh\n", INPD(pIO + VIDPIXELBUFTHOLD            )));
  VideoDebugPrint((3, "  VIDCHROMAMIN                 = %08lXh\n", INPD(pIO + VIDCHROMAMIN                )));
  VideoDebugPrint((3, "  VIDCHROMAMAX                 = %08lXh\n", INPD(pIO + VIDCHROMAMAX                )));
  VideoDebugPrint((3, "  VIDCURRENTLINE               = %08lXh\n", INPD(pIO + VIDCURRENTLINE              )));
  VideoDebugPrint((3, "  VIDSCREENSIZE                = %08lXh\n", INPD(pIO + VIDSCREENSIZE               )));
  VideoDebugPrint((3, "  VIDOVERLAYSTARTCOORDS        = %08lXh\n", INPD(pIO + VIDOVERLAYSTARTCOORDS       )));
  VideoDebugPrint((3, "  VIDOVERLAYENDCOORDS          = %08lXh\n", INPD(pIO + VIDOVERLAYENDCOORDS         )));
  VideoDebugPrint((3, "  VIDOVERLAYDUDX               = %08lXh\n", INPD(pIO + VIDOVERLAYDUDX              )));
  VideoDebugPrint((3, "  VIDOVERLAYDUDXOFFSETSRCWIDTH = %08lXh\n", INPD(pIO + VIDOVERLAYDUDXOFFSETSRCWIDTH)));
  VideoDebugPrint((3, "  VIDOVERLAYDVDY               = %08lXh\n", INPD(pIO + VIDOVERLAYDVDY              )));

  VideoDebugPrint((3, "  VIDOVERLAYDVDYOFFSET         = %08lXh\n", INPD(pIO + VIDOVERLAYDVDYOFFSET        )));
  VideoDebugPrint((3, "  VIDDESKTOPSTARTADDR          = %08lXh\n", INPD(pIO + VIDDESKTOPSTARTADDR         )));
  VideoDebugPrint((3, "  VIDDESKTOPOVERLAYSTRIDE      = %08lXh\n", INPD(pIO + VIDDESKTOPOVERLAYSTRIDE     )));
  VideoDebugPrint((3, "  VIDINADDR0                   = %08lXh\n", INPD(pIO + VIDINADDR0                  )));
  VideoDebugPrint((3, "  VIDINADDR1                   = %08lXh\n", INPD(pIO + VIDINADDR1                  )));
  VideoDebugPrint((3, "  VIDINADDR2                   = %08lXh\n", INPD(pIO + VIDINADDR2                  )));
  VideoDebugPrint((3, "  VIDINSTRIDE                  = %08lXh\n", INPD(pIO + VIDINSTRIDE                 )));
  VideoDebugPrint((3, "  VIDCURROVERLAYSTARTADDR      = %08lXh\n", INPD(pIO + VIDCURROVERLAYSTARTADDR     )));



  VideoDebugPrint((3, "General Regs\n"));

  VideoDebugPrint((3, "  MiscOutput         = %02Xh\n", INPB(pIO + 0xCC)));
  VideoDebugPrint((3, "  Input Status 0     = %02Xh\n", INPB(pIO + 0xC2)));
  VideoDebugPrint((3, "  Input Status 1     = %02Xh\n", INPB(pIO + 0xDA)));
  VideoDebugPrint((3, "  Feature Control    = %02Xh\n", INPB(pIO + 0xCA)));
  VideoDebugPrint((3, "  Motherboard Enable = %02Xh\n", INPB(pIO + 0xC3)));
  //VideoDebugPrint((3, "  Adapter Enable     = %02Xh\n", INPB(pIO + ??)));
  VideoDebugPrint((3, "  Subsystem Enable   = %02Xh\n", INPB(pIO + 0xCE)));



  VideoDebugPrint((3, "CRTC Regs\n"));

  for (i = 0; i <= 0x18; i++)
  {
    OUTPB(pIO + 0xD4, i);
    VideoDebugPrint((3, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xD5)));
  }
  for (i = 0x1A; i <= 0x1F; i++)
  {
    OUTPB(pIO + 0xD4, i);
    VideoDebugPrint((3, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xD5)));
  }
  OUTPB(pIO + 0xD4, 0x22);
  VideoDebugPrint((3, "  Index 22h = %02Xh\n", i, INPB(pIO + 0xD5)));
  OUTPB(pIO + 0xD4, 0x24);
  VideoDebugPrint((3, "  Index 24h = %02Xh\n", i, INPB(pIO + 0xD5)));
  OUTPB(pIO + 0xD4, 0x26);
  VideoDebugPrint((3, "  Index 26h = %02Xh\n", i, INPB(pIO + 0xD5)));



  VideoDebugPrint((3, "Sequencer Regs\n"));

  for (i = 0; i <= 0x4; i++)
  {
    OUTPB(pIO + 0xC4, i);
    VideoDebugPrint((3, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xC5)));
  }



  VideoDebugPrint((3, "Graphics Controller Regs\n"));

  for (i = 0; i <= 0x8; i++)
  {
    OUTPB(pIO + 0xCE, i);
    VideoDebugPrint((3, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xCF)));
  }



#if 0
  VideoDebugPrint((3, "Attribute Controller Regs\n"));

  for (i = 0; i <= 0x14; i++)
  {
    OUTPB(pIO + 0xC0, i);
    VideoDebugPrint((3, "  Index %02Xh = %02Xh\n", i, INPB(pIO + 0xC0)));
  }
#endif



  VideoDebugPrint((3, "MemBase0 Regs\n"));

  VideoDebugPrint((3, "  status                       = %08lXh\n", RegisterMap->status                      ));
  VideoDebugPrint((3, "  pciInit0                     = %08lXh\n", RegisterMap->pciInit0                    ));
  VideoDebugPrint((3, "  sipMonitor                   = %08lXh\n", RegisterMap->sipMonitor                  ));
  VideoDebugPrint((3, "  lfbMemoryConfig              = %08lXh\n", RegisterMap->lfbMemoryConfig             ));
  VideoDebugPrint((3, "  miscInit0                    = %08lXh\n", RegisterMap->miscInit0                   ));
  VideoDebugPrint((3, "  miscInit1                    = %08lXh\n", RegisterMap->miscInit1                   ));
  VideoDebugPrint((3, "  dramInit0                    = %08lXh\n", RegisterMap->dramInit0                   ));
  VideoDebugPrint((3, "  dramInit1                    = %08lXh\n", RegisterMap->dramInit1                   ));
  VideoDebugPrint((3, "  agpInit                      = %08lXh\n", RegisterMap->agpInit                     ));
  VideoDebugPrint((3, "  tmuGbeInit                   = %08lXh\n", RegisterMap->tmuGbeInit                  ));
  VideoDebugPrint((3, "  vgaInit0                     = %08lXh\n", RegisterMap->vgaInit0                    ));
  VideoDebugPrint((3, "  vgaInit1                     = %08lXh\n", RegisterMap->vgaInit1                    ));
  VideoDebugPrint((3, "  dramCommand                  = %08lXh\n", RegisterMap->dramCommand                 ));
  VideoDebugPrint((3, "  dramData                     = %08lXh\n", RegisterMap->dramData                    ));
  VideoDebugPrint((3, "  _reserved00                  = %08lXh\n", RegisterMap->_reserved00                 ));
#define TVOUT_SUPPORTED 1
#ifdef TVOUT_SUPPORTED
  VideoDebugPrint((3, "  vidTvOutBlankVCount          = %08lXh\n", RegisterMap->vidTvOutBlankVCount         ));
#else
  VideoDebugPrint((3, "  _reserved01                  = %08lXh\n", RegisterMap->_reserved01                 ));
#endif
  VideoDebugPrint((3, "  pllCtrl0                     = %08lXh\n", RegisterMap->pllCtrl0                    ));
  VideoDebugPrint((3, "  pllCtrl1                     = %08lXh\n", RegisterMap->pllCtrl1                    ));
  VideoDebugPrint((3, "  pllCtrl2                     = %08lXh\n", RegisterMap->pllCtrl2                    ));
  VideoDebugPrint((3, "  dacMode                      = %08lXh\n", RegisterMap->dacMode                     ));
  VideoDebugPrint((3, "  dacAddr                      = %08lXh\n", RegisterMap->dacAddr                     ));
  VideoDebugPrint((3, "  dacData                      = %08lXh\n", RegisterMap->dacData                     ));
  VideoDebugPrint((3, "  vidMaxRGBDelta               = %08lXh\n", RegisterMap->vidMaxRGBDelta              ));
  VideoDebugPrint((3, "  vidProcCfg                   = %08lXh\n", RegisterMap->vidProcCfg                  ));
  VideoDebugPrint((3, "  hwCurPatAddr                 = %08lXh\n", RegisterMap->hwCurPatAddr                ));
  VideoDebugPrint((3, "  hwCurLoc                     = %08lXh\n", RegisterMap->hwCurLoc                    ));
  VideoDebugPrint((3, "  hwCurC0                      = %08lXh\n", RegisterMap->hwCurC0                     ));
  VideoDebugPrint((3, "  hwCurC1                      = %08lXh\n", RegisterMap->hwCurC1                     ));
  VideoDebugPrint((3, "  vidInFormat                  = %08lXh\n", RegisterMap->vidInFormat                 ));
#ifdef TVOUT_SUPPORTED
  VideoDebugPrint((3, "  vidTvOutBlankHCount          = %08lXh\n", RegisterMap->vidTvOutBlankHCount         ));
#else
  VideoDebugPrint((3, "  vidInStatus                  = %08lXh\n", RegisterMap->vidInStatus                 ));
#endif
  VideoDebugPrint((3, "  vidSerialParallelPort        = %08lXh\n", RegisterMap->vidSerialParallelPort       ));
  VideoDebugPrint((3, "  vidInXDecimDeltas            = %08lXh\n", RegisterMap->vidInXDecimDeltas           ));
  VideoDebugPrint((3, "  vidInDecimInitErrs           = %08lXh\n", RegisterMap->vidInDecimInitErrs          ));
  VideoDebugPrint((3, "  vidInYDecimDeltas            = %08lXh\n", RegisterMap->vidInYDecimDeltas           ));
  VideoDebugPrint((3, "  vidPixelBufThold             = %08lXh\n", RegisterMap->vidPixelBufThold            ));
  VideoDebugPrint((3, "  vidChromaMin                 = %08lXh\n", RegisterMap->vidChromaMin                ));
  VideoDebugPrint((3, "  vidChromaMax                 = %08lXh\n", RegisterMap->vidChromaMax                ));
  VideoDebugPrint((3, "  vidCurrentLine               = %08lXh\n", RegisterMap->vidCurrentLine              ));
  VideoDebugPrint((3, "  vidScreenSize                = %08lXh\n", RegisterMap->vidScreenSize               ));
  VideoDebugPrint((3, "  vidOverlayStartCoords        = %08lXh\n", RegisterMap->vidOverlayStartCoords       ));
  VideoDebugPrint((3, "  vidOverlayEndScreenCoords    = %08lXh\n", RegisterMap->vidOverlayEndScreenCoords   ));
  VideoDebugPrint((3, "  vidOverlayDudx               = %08lXh\n", RegisterMap->vidOverlayDudx              ));
  VideoDebugPrint((3, "  vidOverlayDudxOffsetSrcWidth = %08lXh\n", RegisterMap->vidOverlayDudxOffsetSrcWidth));
  VideoDebugPrint((3, "  vidOverlayDvdy               = %08lXh\n", RegisterMap->vidOverlayDvdy              ));
  VideoDebugPrint((3, "  vidOverlayDvdyOffset         = %08lXh\n", RegisterMap->vidOverlayDvdyOffset        ));
  VideoDebugPrint((3, "  vidDesktopStartAddr          = %08lXh\n", RegisterMap->vidDesktopStartAddr         ));
  VideoDebugPrint((3, "  vidDesktopOverlayStride      = %08lXh\n", RegisterMap->vidDesktopOverlayStride     ));
  VideoDebugPrint((3, "  vidInAddr0                   = %08lXh\n", RegisterMap->vidInAddr0                  ));
  VideoDebugPrint((3, "  vidInAddr1                   = %08lXh\n", RegisterMap->vidInAddr1                  ));
  VideoDebugPrint((3, "  vidInAddr2                   = %08lXh\n", RegisterMap->vidInAddr2                  ));
  VideoDebugPrint((3, "  vidInStride                  = %08lXh\n", RegisterMap->vidInStride                 ));
  VideoDebugPrint((3, "  vidCurrOverlayStartAddr      = %08lXh\n", RegisterMap->vidCurrOverlayStartAddr     ));



  VideoDebugPrint((3, "2D Regs\n"));

  VideoDebugPrint((3, "  status           = %08lXh\n", RegisterMap2D->status          ));
  VideoDebugPrint((3, "  unused           = %08lXh\n", RegisterMap2D->unused          ));
  VideoDebugPrint((3, "  clip0Min         = %08lXh\n", RegisterMap2D->clip0Min        ));
  VideoDebugPrint((3, "  clip0Max         = %08lXh\n", RegisterMap2D->clip0Max        ));
  VideoDebugPrint((3, "  dstBaseAddr      = %08lXh\n", RegisterMap2D->dstBaseAddr     ));
  VideoDebugPrint((3, "  dstFormat        = %08lXh\n", RegisterMap2D->dstFormat       ));
  VideoDebugPrint((3, "  srcColorKeyMin   = %08lXh\n", RegisterMap2D->srcColorKeyMin  ));
  VideoDebugPrint((3, "  srcColorKeyMax   = %08lXh\n", RegisterMap2D->srcColorKeyMax  ));
  VideoDebugPrint((3, "  dstColorKeyMin   = %08lXh\n", RegisterMap2D->dstColorKeyMin  ));
  VideoDebugPrint((3, "  dstColorKeyMax   = %08lXh\n", RegisterMap2D->dstColorKeyMax  ));
  VideoDebugPrint((3, "  bresError0       = %08lXh\n", RegisterMap2D->bresError0      ));
  VideoDebugPrint((3, "  bresError1       = %08lXh\n", RegisterMap2D->bresError1      ));
  VideoDebugPrint((3, "  rop              = %08lXh\n", RegisterMap2D->rop             ));
  VideoDebugPrint((3, "  srcBaseAddr      = %08lXh\n", RegisterMap2D->srcBaseAddr     ));
  VideoDebugPrint((3, "  commandEx        = %08lXh\n", RegisterMap2D->commandEx       ));
  VideoDebugPrint((3, "  lineStipple      = %08lXh\n", RegisterMap2D->lineStipple     ));
  VideoDebugPrint((3, "  lineStyle        = %08lXh\n", RegisterMap2D->lineStyle       ));
  VideoDebugPrint((3, "  pattern0alias    = %08lXh\n", RegisterMap2D->pattern0alias   ));
  VideoDebugPrint((3, "  pattern1alias    = %08lXh\n", RegisterMap2D->pattern1alias   ));
  VideoDebugPrint((3, "  clip1min         = %08lXh\n", RegisterMap2D->clip1min        ));
  VideoDebugPrint((3, "  clip1max         = %08lXh\n", RegisterMap2D->clip1max        ));
  VideoDebugPrint((3, "  srcFormat        = %08lXh\n", RegisterMap2D->srcFormat       ));
  VideoDebugPrint((3, "  srcSize          = %08lXh\n", RegisterMap2D->srcSize         ));
  VideoDebugPrint((3, "  srcXY            = %08lXh\n", RegisterMap2D->srcXY           ));
  VideoDebugPrint((3, "  colorBack        = %08lXh\n", RegisterMap2D->colorBack       ));
  VideoDebugPrint((3, "  colorFore        = %08lXh\n", RegisterMap2D->colorFore       ));
  VideoDebugPrint((3, "  dstSize          = %08lXh\n", RegisterMap2D->dstSize         ));
  VideoDebugPrint((3, "  dstXY            = %08lXh\n", RegisterMap2D->dstXY           ));
  VideoDebugPrint((3, "  command          = %08lXh\n", RegisterMap2D->command         ));
#if 0
  for (i = 0; i < 16; i++)
  {
    VideoDebugPrint((3, "  launchArea[%02ld]   = %08lXh\n", i, RegisterMap2D->launchArea[16]  ));
  }
#endif
  for (i = 0; i < 32; i++)
  {
    VideoDebugPrint((3, "  colorPattern[%02ld] = %08lXh\n", i, RegisterMap2D->colorPattern[32]));
  }



#if 0
  VideoDebugPrint((3, "3D regs\n"));

  VideoDebugPrint((3, "  status             = %08lXh\n", RegisterMap3D->status              ));
  VideoDebugPrint((3, "  intrCtrl           = %08lXh\n", RegisterMap3D->intrCtrl            ));
  VideoDebugPrint((3, "  vA.x               = %08lXh\n", RegisterMap3D->vA.x                ));
  VideoDebugPrint((3, "  vA.y               = %08lXh\n", RegisterMap3D->vA.y                ));
  VideoDebugPrint((3, "  vB.x               = %08lXh\n", RegisterMap3D->vB.x                ));
  VideoDebugPrint((3, "  vB.y               = %08lXh\n", RegisterMap3D->vB.y                ));
  VideoDebugPrint((3, "  vC.x               = %08lXh\n", RegisterMap3D->vC.x                ));
  VideoDebugPrint((3, "  vC.y               = %08lXh\n", RegisterMap3D->vC.y                ));
  VideoDebugPrint((3, "  r                  = %08lXh\n", RegisterMap3D->r                   ));
  VideoDebugPrint((3, "  g                  = %08lXh\n", RegisterMap3D->g                   ));
  VideoDebugPrint((3, "  b                  = %08lXh\n", RegisterMap3D->b                   ));
  VideoDebugPrint((3, "  z                  = %08lXh\n", RegisterMap3D->z                   ));
  VideoDebugPrint((3, "  s                  = %08lXh\n", RegisterMap3D->s                   ));
  VideoDebugPrint((3, "  t                  = %08lXh\n", RegisterMap3D->t                   ));
  VideoDebugPrint((3, "  a                  = %08lXh\n", RegisterMap3D->a                   ));
  VideoDebugPrint((3, "  w                  = %08lXh\n", RegisterMap3D->w                   ));
  VideoDebugPrint((3, "  drdx               = %08lXh\n", RegisterMap3D->drdx                ));
  VideoDebugPrint((3, "  dgdx               = %08lXh\n", RegisterMap3D->dgdx                ));
  VideoDebugPrint((3, "  dbdx               = %08lXh\n", RegisterMap3D->dbdx                ));
  VideoDebugPrint((3, "  dzdx               = %08lXh\n", RegisterMap3D->dzdx                ));
  VideoDebugPrint((3, "  dadx               = %08lXh\n", RegisterMap3D->dadx                ));
  VideoDebugPrint((3, "  dsdx               = %08lXh\n", RegisterMap3D->dsdx                ));
  VideoDebugPrint((3, "  dtdx               = %08lXh\n", RegisterMap3D->dtdx                ));
  VideoDebugPrint((3, "  dwdx               = %08lXh\n", RegisterMap3D->dwdx                ));
  VideoDebugPrint((3, "  drdy               = %08lXh\n", RegisterMap3D->drdy                ));
  VideoDebugPrint((3, "  dgdy               = %08lXh\n", RegisterMap3D->dgdy                ));
  VideoDebugPrint((3, "  dbdy               = %08lXh\n", RegisterMap3D->dbdy                ));
  VideoDebugPrint((3, "  dzdy               = %08lXh\n", RegisterMap3D->dzdy                ));
  VideoDebugPrint((3, "  dady               = %08lXh\n", RegisterMap3D->dady                ));
  VideoDebugPrint((3, "  dsdy               = %08lXh\n", RegisterMap3D->dsdy                ));
  VideoDebugPrint((3, "  dtdy               = %08lXh\n", RegisterMap3D->dtdy                ));
  VideoDebugPrint((3, "  dwdy               = %08lXh\n", RegisterMap3D->dwdy                ));
  VideoDebugPrint((3, "  triangleCMD        = %08lXh\n", RegisterMap3D->triangleCMD         ));
  VideoDebugPrint((3, "  FvA.x              = %08lXh\n", RegisterMap3D->FvA.x               ));
  VideoDebugPrint((3, "  FvA.y              = %08lXh\n", RegisterMap3D->FvA.y               ));
  VideoDebugPrint((3, "  FvB.x              = %08lXh\n", RegisterMap3D->FvB.x               ));
  VideoDebugPrint((3, "  FvB.y              = %08lXh\n", RegisterMap3D->FvB.y               ));
  VideoDebugPrint((3, "  FvC.x              = %08lXh\n", RegisterMap3D->FvC.x               ));
  VideoDebugPrint((3, "  FvC.y              = %08lXh\n", RegisterMap3D->FvC.y               ));
  VideoDebugPrint((3, "  Fr                 = %08lXh\n", RegisterMap3D->Fr                  ));
  VideoDebugPrint((3, "  Fg                 = %08lXh\n", RegisterMap3D->Fg                  ));
  VideoDebugPrint((3, "  Fb                 = %08lXh\n", RegisterMap3D->Fb                  ));
  VideoDebugPrint((3, "  Fz                 = %08lXh\n", RegisterMap3D->Fz                  ));
  VideoDebugPrint((3, "  Fs                 = %08lXh\n", RegisterMap3D->Fs                  ));
  VideoDebugPrint((3, "  Ft                 = %08lXh\n", RegisterMap3D->Ft                  ));
  VideoDebugPrint((3, "  Fa                 = %08lXh\n", RegisterMap3D->Fa                  ));
  VideoDebugPrint((3, "  Fw                 = %08lXh\n", RegisterMap3D->Fw                  ));
  VideoDebugPrint((3, "  Fdrdx              = %08lXh\n", RegisterMap3D->Fdrdx               ));
  VideoDebugPrint((3, "  Fdgdx              = %08lXh\n", RegisterMap3D->Fdgdx               ));
  VideoDebugPrint((3, "  Fdbdx              = %08lXh\n", RegisterMap3D->Fdbdx               ));
  VideoDebugPrint((3, "  Fdzdx              = %08lXh\n", RegisterMap3D->Fdzdx               ));
  VideoDebugPrint((3, "  Fdadx              = %08lXh\n", RegisterMap3D->Fdadx               ));
  VideoDebugPrint((3, "  Fdsdx              = %08lXh\n", RegisterMap3D->Fdsdx               ));
  VideoDebugPrint((3, "  Fdtdx              = %08lXh\n", RegisterMap3D->Fdtdx               ));
  VideoDebugPrint((3, "  Fdwdx              = %08lXh\n", RegisterMap3D->Fdwdx               ));
  VideoDebugPrint((3, "  Fdrdy              = %08lXh\n", RegisterMap3D->Fdrdy               ));
  VideoDebugPrint((3, "  Fdgdy              = %08lXh\n", RegisterMap3D->Fdgdy               ));
  VideoDebugPrint((3, "  Fdbdy              = %08lXh\n", RegisterMap3D->Fdbdy               ));
  VideoDebugPrint((3, "  Fdzdy              = %08lXh\n", RegisterMap3D->Fdzdy               ));
  VideoDebugPrint((3, "  Fdady              = %08lXh\n", RegisterMap3D->Fdady               ));
  VideoDebugPrint((3, "  Fdsdy              = %08lXh\n", RegisterMap3D->Fdsdy               ));
  VideoDebugPrint((3, "  Fdtdy              = %08lXh\n", RegisterMap3D->Fdtdy               ));
  VideoDebugPrint((3, "  Fdwdy              = %08lXh\n", RegisterMap3D->Fdwdy               ));
  VideoDebugPrint((3, "  FtriangleCMD       = %08lXh\n", RegisterMap3D->FtriangleCMD        ));
  VideoDebugPrint((3, "  fbzColorPath       = %08lXh\n", RegisterMap3D->fbzColorPath        ));
  VideoDebugPrint((3, "  fogMode            = %08lXh\n", RegisterMap3D->fogMode             ));
  VideoDebugPrint((3, "  alphaMode          = %08lXh\n", RegisterMap3D->alphaMode           ));
  VideoDebugPrint((3, "  fbzMode            = %08lXh\n", RegisterMap3D->fbzMode             ));
  VideoDebugPrint((3, "  lfbMode            = %08lXh\n", RegisterMap3D->lfbMode             ));
  VideoDebugPrint((3, "  clipLeftRight      = %08lXh\n", RegisterMap3D->clipLeftRight       ));
  VideoDebugPrint((3, "  clipTopBottom      = %08lXh\n", RegisterMap3D->clipTopBottom       ));
  VideoDebugPrint((3, "  nopCMD             = %08lXh\n", RegisterMap3D->nopCMD              ));
  VideoDebugPrint((3, "  fastfillCMD        = %08lXh\n", RegisterMap3D->fastfillCMD         ));
  VideoDebugPrint((3, "  swapbufferCMD      = %08lXh\n", RegisterMap3D->swapbufferCMD       ));
  VideoDebugPrint((3, "  fogColor           = %08lXh\n", RegisterMap3D->fogColor            ));
  VideoDebugPrint((3, "  zaColor            = %08lXh\n", RegisterMap3D->zaColor             ));
  VideoDebugPrint((3, "  chromaKey          = %08lXh\n", RegisterMap3D->chromaKey           ));
  VideoDebugPrint((3, "  chromaRange        = %08lXh\n", RegisterMap3D->chromaRange         ));
  VideoDebugPrint((3, "  userIntrCmd        = %08lXh\n", RegisterMap3D->userIntrCmd         ));
  VideoDebugPrint((3, "  stipple            = %08lXh\n", RegisterMap3D->stipple             ));
  VideoDebugPrint((3, "  c0                 = %08lXh\n", RegisterMap3D->c0                  ));
  VideoDebugPrint((3, "  c1                 = %08lXh\n", RegisterMap3D->c1                  ));
  VideoDebugPrint((3, "  stats.fbiPixelsIn  = %08lXh\n", RegisterMap3D->stats.fbiPixelsIn   ));
  VideoDebugPrint((3, "  stats.fbiChromaFail= %08lXh\n", RegisterMap3D->stats.fbiChromaFail ));
  VideoDebugPrint((3, "  stats.fbiZfuncFail = %08lXh\n", RegisterMap3D->stats.fbiZfuncFail  ));
  VideoDebugPrint((3, "  stats.fbiAfuncFail = %08lXh\n", RegisterMap3D->stats.fbiAfuncFail  ));
  VideoDebugPrint((3, "  stats.fbiPixelsOut = %08lXh\n", RegisterMap3D->stats.fbiPixelsOut  ));
  for (i = 0; i < 32; i++)
  {
    VideoDebugPrint((3, "  fogTable[%02ld]       = %08lXh\n", i, RegisterMap3D->fogTable[32]));
  }
  VideoDebugPrint((3, "  renderMode         = %08lXh\n", RegisterMap3D->renderMode          ));
  VideoDebugPrint((3, "  stencilMode        = %08lXh\n", RegisterMap3D->stencilMode         ));
  VideoDebugPrint((3, "  stencilOp          = %08lXh\n", RegisterMap3D->stencilOp           ));
  VideoDebugPrint((3, "  colBufferAddr      = %08lXh\n", RegisterMap3D->colBufferAddr       ));
  VideoDebugPrint((3, "  colBufferStride    = %08lXh\n", RegisterMap3D->colBufferStride     ));
  VideoDebugPrint((3, "  auxBufferAddr      = %08lXh\n", RegisterMap3D->auxBufferAddr       ));
  VideoDebugPrint((3, "  auxBufferStride    = %08lXh\n", RegisterMap3D->auxBufferStride     ));
  VideoDebugPrint((3, "  clipLeftRight1     = %08lXh\n", RegisterMap3D->clipLeftRight1      ));
  VideoDebugPrint((3, "  clipTopBottom1     = %08lXh\n", RegisterMap3D->clipTopBottom1      ));
  VideoDebugPrint((3, "  combineMode        = %08lXh\n", RegisterMap3D->combineMode         ));
  VideoDebugPrint((3, "  sliCtrl            = %08lXh\n", RegisterMap3D->sliCtrl             ));
  VideoDebugPrint((3, "  aaCtrl             = %08lXh\n", RegisterMap3D->aaCtrl              ));
  VideoDebugPrint((3, "  chipMask           = %08lXh\n", RegisterMap3D->chipMask            ));
  VideoDebugPrint((3, "  leftDesktopBuf     = %08lXh\n", RegisterMap3D->leftDesktopBuf      ));
  VideoDebugPrint((3, "  swapBufferPend     = %08lXh\n", RegisterMap3D->swapBufferPend      ));
  VideoDebugPrint((3, "  leftOverlayBuf     = %08lXh\n", RegisterMap3D->leftOverlayBuf      ));
  VideoDebugPrint((3, "  rightOverlayBuf    = %08lXh\n", RegisterMap3D->rightOverlayBuf     ));
  VideoDebugPrint((3, "  fbiSwapHistory     = %08lXh\n", RegisterMap3D->fbiSwapHistory      ));
  VideoDebugPrint((3, "  fbiTrianglesOut    = %08lXh\n", RegisterMap3D->fbiTrianglesOut     ));
  VideoDebugPrint((3, "  sSetupMode         = %08lXh\n", RegisterMap3D->sSetupMode          ));
  VideoDebugPrint((3, "  sVx                = %08lXh\n", RegisterMap3D->sVx                 ));
  VideoDebugPrint((3, "  sVy                = %08lXh\n", RegisterMap3D->sVy                 ));
  VideoDebugPrint((3, "  sARGB              = %08lXh\n", RegisterMap3D->sARGB               ));
  VideoDebugPrint((3, "  sRed               = %08lXh\n", RegisterMap3D->sRed                ));
  VideoDebugPrint((3, "  sGreen             = %08lXh\n", RegisterMap3D->sGreen              ));
  VideoDebugPrint((3, "  sBlue              = %08lXh\n", RegisterMap3D->sBlue               ));
  VideoDebugPrint((3, "  sAlpha             = %08lXh\n", RegisterMap3D->sAlpha              ));
  VideoDebugPrint((3, "  sVz                = %08lXh\n", RegisterMap3D->sVz                 ));
  VideoDebugPrint((3, "  sOowfbi            = %08lXh\n", RegisterMap3D->sOowfbi             ));
  VideoDebugPrint((3, "  sOow0              = %08lXh\n", RegisterMap3D->sOow0               ));
  VideoDebugPrint((3, "  sSow0              = %08lXh\n", RegisterMap3D->sSow0               ));
  VideoDebugPrint((3, "  sTow0              = %08lXh\n", RegisterMap3D->sTow0               ));
  VideoDebugPrint((3, "  sOow1              = %08lXh\n", RegisterMap3D->sOow1               ));
  VideoDebugPrint((3, "  sSow1              = %08lXh\n", RegisterMap3D->sSow1               ));
  VideoDebugPrint((3, "  sTow1              = %08lXh\n", RegisterMap3D->sTow1               ));
  VideoDebugPrint((3, "  sDrawTriCMD        = %08lXh\n", RegisterMap3D->sDrawTriCMD         ));
  VideoDebugPrint((3, "  sBeginTriCMD       = %08lXh\n", RegisterMap3D->sBeginTriCMD        ));
  VideoDebugPrint((3, "  textureMode        = %08lXh\n", RegisterMap3D->textureMode         ));
  VideoDebugPrint((3, "  tLOD               = %08lXh\n", RegisterMap3D->tLOD                ));
  VideoDebugPrint((3, "  tDetail            = %08lXh\n", RegisterMap3D->tDetail             ));
  VideoDebugPrint((3, "  texBaseAddr        = %08lXh\n", RegisterMap3D->texBaseAddr         ));
  VideoDebugPrint((3, "  texBaseAddr1       = %08lXh\n", RegisterMap3D->texBaseAddr1        ));
  VideoDebugPrint((3, "  texBaseAddr2       = %08lXh\n", RegisterMap3D->texBaseAddr2        ));
  VideoDebugPrint((3, "  texBaseAddr38      = %08lXh\n", RegisterMap3D->texBaseAddr38       ));
  VideoDebugPrint((3, "  trexInit0          = %08lXh\n", RegisterMap3D->trexInit0           ));
  VideoDebugPrint((3, "  trexInit1          = %08lXh\n", RegisterMap3D->trexInit1           ));
  for (i = 0; i < 12; i++)
  {
    VideoDebugPrint((3, "  nccTable0[%02ld]      = %08lXh\n", i, RegisterMap3D->nccTable0[12]));
  }
  for (i = 0; i < 12; i++)
  {
    VideoDebugPrint((3, "  nccTable1[%02ld]      = %08lXh\n", i, RegisterMap3D->nccTable1[12]));
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
DumpVariableMTRRInfo(baseHi, baseLo, maskHi, maskLo, DebugLevel)
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
}
#endif

