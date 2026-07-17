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

VP_STATUS
H3ConfigurePCI(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	)
/*++

Routine Description:

	This routine is called to do the PCI detection of the adapters.
	By doing PCI detection, we will know a PCI card is present (as opposed
	to a VL or ISA card) which will let us call the HAL to do some extra
	configuration, and eventually use PLUG and PLAY to detect the cards.

Arguments:

	HwDeviceExtension - Supplies a pointer to the miniport's device extension.

Return Value:

	TRUE if we found the card in a PCI slot, FALSE otherwise

	On return, the access range array contained in HwDeviceExtension is
	updated to the number of access ranges used by the card.

--*/

{
	PCI_COMMON_CONFIG	PCIBuffer;
	ULONG				j, Slot;
	VP_STATUS			status;

	VideoDebugPrint((1, "H3ConfigurePCI -\n"));

	for (Slot=0; Slot<32; Slot++)
	{
		j = VideoPortGetBusData(HwDeviceExtension,
								PCIConfiguration,
								Slot,
								(PVOID) &PCIBuffer,
								0,
								sizeof(ULONG));

		if (j == 0) 	// no PCI bus
		{
			VideoDebugPrint((0, "No PCI bus\n"));

			return ERROR_DEV_NOT_EXIST;
		}

		if (j == 2) 	// no PCI device in this slot.
		{				// if it's function 0 then abandon the slot.
			continue;
		}

		HwDeviceExtension->PCIVendorID = PCIBuffer.VendorID;
		HwDeviceExtension->PCIDeviceID = PCIBuffer.DeviceID;

#if DBG
		DumpPCIConfigSpace(HwDeviceExtension, Slot);
#endif

		if (PCIBuffer.VendorID == TDFX_VENDOR_ID)
		{
			VideoDebugPrint((2, "\t Found 3DFX chip %04x in Slot[0x%02x]\n", PCIBuffer.DeviceID, Slot));
      // allow driver to load on any tdfx chip with a device id between 0x4 and 0xF
      if (((TDFX_VENDOR_ID == PCIBuffer.VendorID) &&
           ((0x04 <= PCIBuffer.DeviceID) && (0x0F >= PCIBuffer.DeviceID))))
			{
				//
				// We found a Napalm or avenger, so let's allocate the resources
				//
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
#endif

				VideoPortGetBusData(HwDeviceExtension,
									PCIConfiguration,
									Slot,
									(PPCI_COMMON_CONFIG) &PCIBuffer,
									0,
									sizeof(PCI_COMMON_CONFIG));

#define CapabilitiesPtr   Reserved2[0]
    // walk CapablitiesPtr chain to see if it's an AGP card
    if (0 != (UCHAR)PCIBuffer.u.type0.CapabilitiesPtr)
    {
      ULONG offset, capID;

      offset = (UCHAR)PCIBuffer.u.type0.CapabilitiesPtr;
      while (offset)
      {
        capID = *(ULONG *)&PCIBuffer.DeviceSpecific[offset - PCI_COMMON_HDR_LENGTH];

        // if the low word is 2 then it's an agp board
        if (0x2 == (capID & 0xFF))
        {
          VideoDebugPrint((0, "\tand it's an AGP board!!!  (conforms to AGP rev %d.%d)\n",
                           (capID & 0xF00000) >> 20, (capID & 0xF0000) >> 16));
          HwDeviceExtension->PciSpeed = 66;
          break;
        }

        // step to next CapabilitiesPtr
        offset = (capID & 0xFF00) >> 8;
      }
    }

#if DBG
				VideoDebugPrint((2, "membase0addr = 0x%08lx\n", PCIBuffer.u.type0.BaseAddresses[0]));
				VideoDebugPrint((2, "membase1addr = 0x%08lx\n", PCIBuffer.u.type0.BaseAddresses[1]));
				VideoDebugPrint((2, "iobaseaddr   = 0x%08lx\n", PCIBuffer.u.type0.BaseAddresses[2]));
				VideoDebugPrint((2, "rombase0addr = 0x%08lx\n", PCIBuffer.u.type0.ROMBaseAddress));
#endif
				VideoDebugPrint((2, "Collecting bus-relative addresses from PCI config\n"));

				//
				// Collect the bus-relative address for membase[0,1], iobase, and ROMbase
				//

				HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeStart.LowPart = PCIBuffer.u.type0.BaseAddresses[0];
				HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeLength = H3AccessRanges[MEMBASE_ZERO].RangeLength;
				HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeInIoSpace = H3AccessRanges[MEMBASE_ZERO].RangeInIoSpace;
				HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeVisible = H3AccessRanges[MEMBASE_ZERO].RangeVisible;
				HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeShareable = H3AccessRanges[MEMBASE_ZERO].RangeShareable;

				HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart.LowPart = PCIBuffer.u.type0.BaseAddresses[1] & 0xfffffff0;
				HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeLength = H3AccessRanges[MEMBASE_ONE].RangeLength;
				HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeInIoSpace = H3AccessRanges[MEMBASE_ONE].RangeInIoSpace;
				HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeVisible = H3AccessRanges[MEMBASE_ONE].RangeVisible;
				HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeShareable = H3AccessRanges[MEMBASE_ONE].RangeShareable;

				HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeStart.LowPart = PCIBuffer.u.type0.BaseAddresses[2] & 0xfffffff0;
				HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeLength = H3AccessRanges[IOBASE_ZERO].RangeLength;
				HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeInIoSpace = H3AccessRanges[IOBASE_ZERO].RangeInIoSpace;
				HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeVisible = H3AccessRanges[IOBASE_ZERO].RangeVisible;
				HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeShareable = H3AccessRanges[IOBASE_ZERO].RangeShareable;

				HwDeviceExtension->AccessRanges[ROMBASE].RangeStart.LowPart = PCIBuffer.u.type0.ROMBaseAddress & 0xffff0000;
				HwDeviceExtension->AccessRanges[ROMBASE].RangeStart.LowPart = 0xc0000;
				HwDeviceExtension->AccessRanges[ROMBASE].RangeLength = H3AccessRanges[ROMBASE].RangeLength;
				HwDeviceExtension->AccessRanges[ROMBASE].RangeInIoSpace = H3AccessRanges[ROMBASE].RangeInIoSpace;
				HwDeviceExtension->AccessRanges[ROMBASE].RangeVisible = H3AccessRanges[ROMBASE].RangeVisible;
				HwDeviceExtension->AccessRanges[ROMBASE].RangeShareable = H3AccessRanges[ROMBASE].RangeShareable;

#if DBG
				DumpAccessRanges(HwDeviceExtension->AccessRanges, NUM_H3_ACCESS_RANGES);
#endif

				//
				// Try and find someplace to locate the 64mb window needed.
				//
				// IMPORTANT NOTE : We are only reserving the 64MEG physical
				// address space, to avoid hardware conflicts between devices.
				// We will not MAP the entire 64 MEG address space since this
				// would take up too many system resources.  We will only map
				// the actual amount of memory when we get called in
				// IOCTL_VIDEO_MAP_VIDEO_MEMORY.
				//

				status = VideoPortGetAccessRanges(HwDeviceExtension,
												  0, //1,
												  NULL, //&ioResource,
												  NUM_H3_ACCESS_RANGES, //1,
												  HwDeviceExtension->AccessRanges,
												  &PCIBuffer.VendorID,
												  &PCIBuffer.DeviceID,
												  &Slot);

				if (status == NO_ERROR)
				{
					VideoDebugPrint((3, "VideoPortGetAccessRanges okay!\n"));
					//
					// Store the PCI Slot number, VendorId, DeviceId and RevId in case we need
					// them in the future.
					//

					HwDeviceExtension->PCIVendorID = PCIBuffer.VendorID;
					HwDeviceExtension->PCIDeviceID = PCIBuffer.DeviceID;
                    HwDeviceExtension->PCISubVendorID = PCIBuffer.u.type0.SubVendorID;
                    HwDeviceExtension->PCISubSystemID = PCIBuffer.u.type0.SubSystemID;

					HwDeviceExtension->PCISlot.u.AsULONG = Slot;
					HwDeviceExtension->ChipRevision = PCIBuffer.RevisionID;

					//
					// We found the PCI card, so there is no reason to continue.
					// Return TRUE to indicate a card was in a PCI Slot.
					//

					return NO_ERROR;
				}
				else
				{
					//
					// Should we return from here? or continue looking for PCI
					// devices.
					//
					VideoDebugPrint((1, "H3: Oops, VideoPortGetAccessRanges failed, status=0x%x\n", status));
				}
			}
		}
	}

	VideoDebugPrint((1, "H3ConfigurePCI fails\n"));

	return ERROR_DEV_NOT_EXIST;

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
