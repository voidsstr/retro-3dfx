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

	pci.c

Abstract:

	This module contains the code that implements PCI stuff needed
	by the the H3 miniport driver.

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

UCHAR
H3PCIConfigReadUchar(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Offset
	)
{
	UCHAR value;

	VideoDebugPrint((3, "H3PCIConfigReadUchar(0x%02x)\n", Offset));
	VideoPortGetBusData(HwDeviceExtension,
						PCIConfiguration,
						0,
						&value,
						Offset,
						1);
	return value;
}

BOOLEAN
H3PCIConfigWriteUchar(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Offset,
	UCHAR Data
	)
{
	ULONG size;
	UCHAR value = Data;

	VideoDebugPrint((3, "H3PCIConfigWriteUchar(0x%02x, 0x%02x)\n", Offset, value));
	size = VideoPortSetBusData(HwDeviceExtension,
							PCIConfiguration,
							0,
							&value,
							Offset,
							1);

	return (BOOLEAN) (size == 1);
}


ULONG
H3PCIConfigReadUlong(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Offset
	)
{
	ULONG value;

	VideoDebugPrint((3, "H3PCIConfigReadUlong(0x%02x)\n", Offset));
	VideoPortGetBusData(HwDeviceExtension,
						PCIConfiguration,
						0,
						&value,
						Offset,
						4);
	return value;
}


BOOLEAN
H3PCIConfigWriteUlong(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Offset,
	ULONG Data
	)
{
	ULONG size, value = Data;

	VideoDebugPrint((3, "H3PCIConfigWriteUlong(0x%02x, 0x%08lx)\n", Offset, value));
	size = VideoPortSetBusData(HwDeviceExtension,
							PCIConfiguration,
							0,
							&value,
							Offset,
							4);

	return (BOOLEAN) (size == 4);
}


USHORT
H3PCIConfigReadUshort(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Offset
	)
{
	USHORT value;

	VideoDebugPrint((3, "H3PCIConfigReadUshort(0x%02x)\n", Offset));
	VideoPortGetBusData(HwDeviceExtension,
						PCIConfiguration,
						0,
						&value,
						Offset,
						2);
	return value;
}


BOOLEAN
H3PCIConfigWriteUshort(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Offset,
	USHORT Data
	)
{
	ULONG size, value = Data;

	VideoDebugPrint((3, "H3PCIConfigWriteUshort(0x%02x, 0x%04x)\n", Offset, value));
	size = VideoPortSetBusData(HwDeviceExtension,
							PCIConfiguration,
							0,
							&value,
							Offset,
							2);

	return (BOOLEAN) (size == 2);
}

#define DEFAULT_DEVICE_ID     TDFX_NAPALM_ID

VP_STATUS
H3ConfigurePCI(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  VP_STATUS         status;
  PCI_COMMON_CONFIG PCIBuffer;
  ULONG             Slot;
  USHORT            vid = TDFX_VENDOR_ID;
  USHORT            did = DEFAULT_DEVICE_ID;


  // Check the PCI bus for our device, and determine where
  // its memory and I/O ranges were mapped.

  VideoDebugPrint((1, "H3ConfigurePCI -\n"));

  // Ask the system to find the card.
  status = VideoPortGetAccessRanges(HwDeviceExtension,
                                    0,
                                    NULL,
                                    NUM_H3_ACCESS_RANGES,
                                    HwDeviceExtension->AccessRanges,
                                    &vid,
                                    &did,
                                    &Slot);
  if (status == NO_ERROR)
  {
    // The checked build of w2k shows the following assert when VideoPortGetBusData
    // is called here:
    //
    // "A miniport must only call VideoPortGetBusData with PCI_COMMON_HDR_LENGTH"
    //
    // since we need the DeviceSpecific range to check for AGP boards (among other things)
    // we'll use HalGetBusData instead of VideoPortGetBusData, adding yet another
    // so called "illegal import"
    //
    // why does microsoft think we don't need access to the device specific region
    // of our own device's pci config space?

    // Get PCI Config Space for this board
#ifdef SLI_AA
extern ULONG HalGetBusData(BUS_DATA_TYPE BusDataType,
                           ULONG         BusNumber,
                           ULONG         SlotNumber,
                           PVOID         Buffer,
                           ULONG         Length);

    HalGetBusData(PCIConfiguration,
                  HwDeviceExtension->BusNumber,
                  Slot,
                  &PCIBuffer,
                  sizeof(PCI_COMMON_CONFIG));
#else
    VideoPortGetBusData(HwDeviceExtension,
                        PCIConfiguration,
                        Slot,
                        &PCIBuffer,
                        0,
                        sizeof(PCI_COMMON_CONFIG));
#endif

#if DBG
    VideoDebugPrint((0, "Hey! I found a 3dfx "));
    switch (PCIBuffer.DeviceID)
    {
      //case TDFX_BANSHEE_ID:
      //  VideoDebugPrint((0, "Voodoo Banshee"));
      //  break;
      //case TDFX_BANSHEE_PLUS_ID:
      //  VideoDebugPrint((0, "Voodoo Banshee Plus"));
      //  break;
      case TDFX_AVENGER_ID:
        VideoDebugPrint((0, "Voodoo3"));
        //VideoDebugPrint((0, "Avenger"));
        break;
      case TDFX_NAPALM_ID:
      case 9:
        //VideoDebugPrint((0, "3 4000"));
        VideoDebugPrint((0, "Napalm"));
        break;
      case TDFX_NAPALM2_ID:
        VideoDebugPrint((0, "Napalm2"));
        break;
      default:
        VideoDebugPrint((0, "UNKNOWN, DeviceID=%04Xh", PCIBuffer.DeviceID));
        break;
    }
    VideoDebugPrint((0, "\n"));

    DumpAccessRanges(HwDeviceExtension->AccessRanges, NUM_H3_ACCESS_RANGES);
#endif

    // allow driver to load on any tdfx chip with a device id between 0x4 and 0xF
    if (! ((TDFX_VENDOR_ID == PCIBuffer.VendorID) &&
           ((0x04 <= PCIBuffer.DeviceID) && (0x0F >= PCIBuffer.DeviceID))))
      return ERROR_DEV_NOT_EXIST;

#ifdef SLI_AA
    // only succeed for the master device
    if (0 != (*(PPCI_SLOT_NUMBER)&Slot).u.bits.FunctionNumber)
    {
      VideoDebugPrint((0, "  H3ConfigurePCI - found device with nonzero functionNumber, failing load\n"));
      return ERROR_DEV_NOT_EXIST;
    }

    // We'll just say that function zero is always going to be the master.
    //  The slot number returned from getaccessranges has the function
    //  number embedded in it.  The problem is that the broken W2K functions
    //  VideoPortGetBusData and VideoPortSetBusData don't use the slot info.
    //  so later we'll have to use the HalGetBusDataByOffset and HalSetBusDataByOffset
    //  functions which MS probably won't be happy about but until they fix
    //  the W2K videoport functions to use the slot info it's either that
    //  or direct writes to cf8 / cfc.

    // we just failed the load on devices with a nonzero function number so IsMasterDevice is always
    // going to be TRUE
    HwDeviceExtension->IsMasterDevice = ((*(PPCI_SLOT_NUMBER) &Slot).u.bits.FunctionNumber == 0);

#endif

#if defined (AGP_FIFO_CODE)
    // assume it's a PCI card
    HwDeviceExtension->IsAGPCard = FALSE;
#endif
    // walk CapablitiesPtr chain to see if it's an AGP card
    if (0 != PCIBuffer.u.type0.CapabilitiesPtr)
    {
      ULONG offset, capID;

      offset = PCIBuffer.u.type0.CapabilitiesPtr;
      while (offset)
      {
        capID = *(ULONG *)&PCIBuffer.DeviceSpecific[offset - PCI_COMMON_HDR_LENGTH];

        // if the low word is 2 then it's an agp board
        if (0x2 == (capID & 0xFF))
        {
          VideoDebugPrint((0, "\tand it's an AGP board!!!  (conforms to AGP rev %d.%d)\n",
                           (capID & 0xF00000) >> 20, (capID & 0xF0000) >> 16));
          HwDeviceExtension->PciSpeed = 66;
#if defined (AGP_FIFO_CODE)
          HwDeviceExtension->IsAGPCard = TRUE;
#endif
          break;
        }

        // step to next CapabilitiesPtr
        offset = (capID & 0xFF00) >> 8;
      }
    }
#if 0
    // enable this workaround if the strapping incorrect
    // workaround for Napalm AGP board not being detected as AGP because of incorrect
    // strapping
    if ((66 != HwDeviceExtension->PciSpeed) &&
        (0x6 <= PCIBuffer.DeviceID) &&
        (1 == HwDeviceExtension->BusNumber))
    {
      VideoDebugPrint((0, "\tand it's an AGP board but it's strapped incorrectly!!!\n"));
      HwDeviceExtension->PciSpeed = 66;
#if defined (AGP_FIFO_CODE)
      HwDeviceExtension->IsAGPCard = TRUE;
#endif
    }
#endif

    HwDeviceExtension->PCIVendorID = PCIBuffer.VendorID;
    HwDeviceExtension->PCIDeviceID = PCIBuffer.DeviceID;
    HwDeviceExtension->PCISubVendorID = PCIBuffer.u.type0.SubVendorID;
    HwDeviceExtension->PCISubSystemID = PCIBuffer.u.type0.SubSystemID;

    HwDeviceExtension->PCISlot.u.AsULONG = Slot;
    HwDeviceExtension->ChipRevision = PCIBuffer.RevisionID;

    //
    // If we're a "secondary" device, we need to know and plan ahead
    //
    if ((PCIBuffer.Command & PCI_CMD_MAPPABLE_ACCESS_RANGES) == 0)
    {
      VideoDebugPrint((0, "This card is currently configured as a secondary device,\n"));
      VideoDebugPrint((0, "but we're going to activate the IO and memory mappings!\n"));

      HwDeviceExtension->IsSecondaryDevice = TRUE;

      //
      // Set the bits to permit mapping the card in. We need to set bit 9 of vgainit0
      // to prevent the card from allowing VGA legacy address decoding, but that will
      // happen somewhere else
      //
      PCIBuffer.Command |= PCI_CMD_MAPPABLE_ACCESS_RANGES;
      VideoPortSetBusData(HwDeviceExtension,
                          PCIConfiguration,
                          Slot,
                          &PCIBuffer,
                          0,
                          PCI_COMMON_HDR_LENGTH);

      VideoDebugPrint((0, "PCI Command reset to allow mappings\n"));
    }
  }

  return status;
}

#if DBG
#define PCI_DEBUG_LEVEL 2
VOID
DumpPCIConfigSpace(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG slot
	)
{
	PPCI_COMMON_CONFIG PciData;
	PLOCAL_PCI_COMMON_CONFIG PciX;

	UCHAR buffer[sizeof(PCI_COMMON_CONFIG)];
	ULONG j;

	PciData = (PPCI_COMMON_CONFIG) buffer;
	PciX = (PLOCAL_PCI_COMMON_CONFIG) buffer;

	j = VideoPortGetBusData (
			(PVOID)HwDeviceExtension,
			PCIConfiguration,
			slot,
			(PPCI_COMMON_CONFIG)PciData,
			0,
			PCI_COMMON_HDR_LENGTH
			);

	// trivially reject no PCI bus
	if (j == 0)
	{
		VideoDebugPrint((PCI_DEBUG_LEVEL, "No PCI bus!\n"));
		return;
	}

	// trivially reject no PCI device
	if (j == 2)
	{
		VideoDebugPrint((PCI_DEBUG_LEVEL, "No valid PCI device!\n"));
		return;
	}

	// don't report junk slots
	if (PciData->VendorID == 0xffff)
		return;

	// don't report anything but 3Dfx
	if (PciData->VendorID != TDFX_VENDOR_ID)
		return;

	VideoDebugPrint((PCI_DEBUG_LEVEL, "DumpPCIConfigSpace: ----------\n"));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  Slot:             %d\n",	slot));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  Vendor Id:        0x%04x\n", PciData->VendorID));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  Device Id:        0x%04x\n", PciData->DeviceID));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  Command:          0x%04x\n", PciData->Command));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  Status:           0x%04x\n", PciData->Status));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  Rev Id:           0x%02x\n", PciData->RevisionID));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  Class Code:       0x%06x\n", PciData->ProgIf));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  SubClass:         0x%02x\n", PciData->SubClass));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  Base Class:       0x%02x\n", PciData->BaseClass));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  CacheLineSize:    0x%02x\n", PciData->CacheLineSize));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  Latency:          0x%02x\n", PciData->LatencyTimer));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  Header Type:      0x%02x\n", PciData->HeaderType));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  BIST:             0x%02x\n", PciData->BIST));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  BaseAddress[0]:   0x%08x\n", PciData->u.type0.BaseAddresses[0]));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  BaseAddress[1]:   0x%08x\n", PciData->u.type0.BaseAddresses[1]));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  BaseAddress[2]:   0x%08x\n", PciData->u.type0.BaseAddresses[2]));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  BaseAddress[3]:   0x%08x\n", PciData->u.type0.BaseAddresses[3]));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  BaseAddress[4]:   0x%08x\n", PciData->u.type0.BaseAddresses[4]));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  BaseAddress[5]:   0x%08x\n", PciData->u.type0.BaseAddresses[5]));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  CIS:              0x%08x\n", PciData->u.type0.CIS));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  SubVendorID:      0x%04x\n", PciData->u.type0.SubVendorID));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  SubSystemID:      0x%04x\n", PciData->u.type0.SubSystemID));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "  ROMBaseAddress:   0x%08x\n", PciData->u.type0.ROMBaseAddress));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "------------------------------\n"));
	VideoDebugPrint((PCI_DEBUG_LEVEL, "Raw dump follows:-------------\n"));

	for (j=0; j<255; j++)
	{
		VideoDebugPrint((PCI_DEBUG_LEVEL, "%02x ", buffer[j]));
		if ((j % 8) == 7)
			VideoDebugPrint((PCI_DEBUG_LEVEL, "\n"));
	}
	VideoDebugPrint((PCI_DEBUG_LEVEL, "\n------------------------------\n"));

}
#endif
