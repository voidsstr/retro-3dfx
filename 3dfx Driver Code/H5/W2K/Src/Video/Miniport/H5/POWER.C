/*++
**
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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

    power.c

Abstract:

    This module contains the code that implements the 3Dfx miniport driver.

Environment:

    Kernel mode

Revision History:

--*/
#define FULL_DRIVER_COMPILE
#ifdef FULL_DRIVER_COMPILE
#include "ntddk.h"
// some definitions to make video.h compile without miniport.h
typedef PVOID	PEMULATOR_ACCESS_ENTRY;
typedef PVOID	PBANKED_SECTION_ROUTINE;	// needed for 3.51
#ifdef	_X86_
#undef ALLOC_PRAGMA
#endif
#if DBG
#undef PAGED_CODE
#endif
#endif
#include "dderror.h"
#include "devioctl.h"
#ifndef FULL_DRIVER_COMPILE
#include "miniport.h"
#endif

#include "ntddvdeo.h"
#include "video.h"
#include "h3.h"
#include "localpci.h"
#include "cmdcnst.h"

#if (_WIN32_WINNT >= 0x500)

#ifdef SLI_AA
#include "sliaa.h"
#include "dfp.h"
#endif

VOID H3_SetDPMSPowerState (PHW_DEVICE_EXTENSION, ULONG, UCHAR);

#pragma alloc_text(PAGE, H3GetChildDescriptor)
#pragma alloc_text(PAGE, H3GetPowerState)
#pragma alloc_text(PAGE, H3SetPowerState)


#define QUERY_MONITOR_ID			0x22446688
#define QUERY_NONDDC_MONITOR_ID 	0x11223344
#define QUERY_V3TV_ID				0x33446688
#define V3TVName  L"3dfxV3TV"
#define V3TV_VPX3225D               0x7230
SHORT VPXDetectDevice(PHW_DEVICE_EXTENSION HwDeviceExtension);

// the following is a hardcoded EDID for device "PNP09FE" which Microsoft defines
// as "Generic Television".  The main effect of this is for the Device Manager to 
// describe the monitor with a name a human can understand instead of "Default Monitor".
const FxU8 tvEdid[] = {0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
                       0x41,0xd0,0xfe,0x09,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,
                       0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x00,0x21,0x00,0x00,0x01,0x01,
                       0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
                       0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
                       0x00,0xFC,0x00, '3', 'd', 'f', 'x', ' ',
                        'T', 'V', 'o', 'u', 't',0x0a, ' ', ' ',
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


ULONG
H3GetChildDescriptor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CHILD_ENUM_INFO ChildEnumInfo,
    PVIDEO_CHILD_TYPE pChildType,
    PUCHAR pChildDescriptor,
    PULONG pHwId,
    PULONG pUnused
    )

/*++

Routine Description:

    Enumerate all devices controlled by the 3Dfx graphics chip.
    This includes DDC monitors attached to the board, as well as other devices
    which may be connected to a proprietary bus.

Arguments:

    HwDeviceExtension - Pointer to our hardware device extension structure.

    ChildIndex        - Index of the child the system wants informaion for.

    pChildType        - Type of child we are enumerating - monitor, I2C ...

    pChildDescriptor  - Identification structure of the device (EDID, string)

    ppHwId            - Private unique 32 bit ID to passed back to the miniport

    pMoreChildren     - Should the miniport be called

Return Value:

    TRUE if the child device existed, FALSE if it did not.

Note:

    In the event of a failure return, none of the fields are valid except for
    the return value and the pMoreChildren field.

--*/

{
	ULONG status = NO_ERROR;
    int sum;
    int ii;

    switch (ChildEnumInfo->ChildIndex)
	{
    case 0:

        //
        // Case 0 is used to enumerate devices found by the ACPI firmware.
        //
        // Since we do not support ACPI devices yet, we must return failure.
        //

        status = ERROR_NO_MORE_DEVICES;
        break;

    case 1:

        //
        // This is the Analog Monitor.
        //

        *pChildType = Monitor;

        //
        // Obtain the EDID structure via DDC.
        //


        if (GetDdcInformation(HwDeviceExtension,
                              pChildDescriptor,
                              ChildEnumInfo->ChildDescriptorSize))
        {
            *pHwId = QUERY_MONITOR_ID;

            VideoDebugPrint((1, "H3GetChildDescriptor - successfully read EDID structure\n"));

        } else {

            //
            // Alway return TRUE, since we always have a monitor output
            // on the card and it just may not be a detectable device.
            //

            *pHwId = QUERY_NONDDC_MONITOR_ID;

            VideoDebugPrint((1, "H3GetChildDescriptor - DDC not supported\n"));

        }

        //status = ERROR_MORE_DATA;
		status = VIDEO_ENUM_MORE_DEVICES;

        break;

    case 2:

        //
        // Handle the Digital Flat Panel if one is present.

        *pChildType = Monitor;

        //
        // Obtain the EDID structure via DDC.
        //

        if (GetDfpEdidInformation(HwDeviceExtension,
                              pChildDescriptor,
                              ChildEnumInfo->ChildDescriptorSize))
        {
            *pHwId = QUERY_DFP_ID;

            VideoDebugPrint((1, "H3GetChildDescriptor - successfully read DFP EDID structure\n"));

//???            status = ERROR_MORE_DATA;
            status = VIDEO_ENUM_MORE_DEVICES;

        } else {

            //
            // If we can't detect a DFP assume we don't have one.
            //

            VideoDebugPrint((1, "H3GetChildDescriptor - DFP not found\n"));

            status = ERROR_INVALID_NAME;  // the DFP device is not present, but there may be other devices.
        }

        break;

    case 3:

        //
        // Handle V3TV if it's present.
        //

		*pChildType = Other;
		//is this the right check to verify if V3TV is installed??
#ifndef MS_VIEW
		if (VPXDetectDevice(HwDeviceExtension) == V3TV_VPX3225D)
#else
		if (0) //Disable V3TV for Whistler "in-box" builds
#endif
		{
			status = VIDEO_ENUM_MORE_DEVICES;
			*pHwId = QUERY_V3TV_ID;  //really should query through I2c if v3tv device is installed
		    RtlCopyMemory((PVOID)pChildDescriptor, (PVOID)V3TVName, sizeof(V3TVName));
			ChildEnumInfo->ChildDescriptorSize= sizeof (V3TVName);

			VideoDebugPrint((0, "H3GetChildDescriptor - successfully read V3TV structure\n"));

//???            status = ERROR_MORE_DATA;
            status = VIDEO_ENUM_MORE_DEVICES;
		}
        else
            status = ERROR_INVALID_NAME;  // the V3TV device is not present, but there may be other devices.

        break;

    case 4:

        //
        // Handle TvOut if present.
        //

        *pChildType = Monitor;

        if (HwDeviceExtension->tvOutCapable)
        {
            *pHwId = QUERY_TVOUT_ID;

		    // copy hardcoded TvOut EDID
		    RtlCopyMemory((PVOID)pChildDescriptor, (PVOID)tvEdid, sizeof(tvEdid));
		    ChildEnumInfo->ChildDescriptorSize= sizeof(tvEdid);

            //  compute the EDID checksum
            for (sum = ii = 0; ii < (sizeof(tvEdid)-1); ii++)
		        sum += tvEdid[ii];
            (FxU8)pChildDescriptor[sizeof(tvEdid)-1] = -sum;  //fill in checksum

            //status = ERROR_MORE_DATA;
		    status = VIDEO_ENUM_MORE_DEVICES;
        }
        else
            status = ERROR_INVALID_NAME;  // the V3TV device is not present, but there may be other devices.

        break;

    default:

        status = ERROR_NO_MORE_DEVICES;
        break;
    }

    VideoDebugPrint((1, "H3GetChildDescriptor - returning %d\n", status));
    return status;
}

VP_STATUS
H3GetPowerState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT VideoPowerManagement
    )

/*++

Routine Description:

    This function is called to see if a given device can go into a given
    power state.

Arguments:

    HwDeviceExtension - Pointer to our hardware device extension structure.

    HwDeviceId           - Private unique 32 bit ID identifing the device.
                           0xFFFFFFFF indicates the 3Dfx card itself.

    VideoPowerManagement - Pointer to the power management structure which
                           indicates the power state in question.

Return Value:

    NO_ERROR if the device can go into the requested power state,
    otherwise an appropriate error code is returned.

--*/

{
    //
    // We only support power setting for the monitor.  Make sure the
    // HwDeviceId matches one the the monitors we could report.
    //

    if ((HwDeviceId == QUERY_NONDDC_MONITOR_ID) || 
        (HwDeviceId == QUERY_DFP_ID) ||
		(HwDeviceId == QUERY_MONITOR_ID))
	{
        //
        // We can support a monitor DPMS by using the extended VGA registers.
        // We can support a DFP by turning on/off the TMDS link.
        //

        switch (VideoPowerManagement->PowerState)
        {
            case VideoPowerHibernate:
            case VideoPowerOn:
            case VideoPowerStandBy:
            case VideoPowerSuspend:
            case VideoPowerOff:
                return NO_ERROR;

            default:
                //
                // Unexpected power state.
                //
                VideoDebugPrint((1, "Unexpected power state.\n"));
                return ERROR_INVALID_PARAMETER;
        }
    }
	else if ((HwDeviceId == DISPLAY_ADAPTER_HW_ID) ||
				(HwDeviceId == QUERY_V3TV_ID))
	{

        //
        // We are querying power support for the graphics card.
        //

        switch (VideoPowerManagement->PowerState)
		{
        case VideoPowerOn:
        case VideoPowerStandBy:
        case VideoPowerHibernate:

            return NO_ERROR;

        case VideoPowerOff:
        case VideoPowerSuspend:

            //
            // Indicate that we can't do VideoPowerOff, because
            // we have no way of coming back when power is re-applied
            // to the card.
            //

            return NO_ERROR;  //ERROR_INVALID_FUNCTION;

        default:
#if DBG
            ASSERT(FALSE);
#endif
            return ERROR_INVALID_PARAMETER;
        }
    }
	else if (HwDeviceId == QUERY_TVOUT_ID)
    {
        //
        // We are querying power support for TvOut
        //

        switch (VideoPowerManagement->PowerState)
		{
        case VideoPowerOn:
        case VideoPowerStandBy:
        case VideoPowerHibernate:
        case VideoPowerOff:
        case VideoPowerSuspend:

            return NO_ERROR;  // Just say we can do everything for now.

        default:
#if DBG
            ASSERT(FALSE);
#endif
            return ERROR_INVALID_PARAMETER;
        }
    }
	else
	{
#if DBG
        VideoDebugPrint((0, "Unknown HwDeviceId"));
        ASSERT(FALSE);
#endif
        return ERROR_INVALID_PARAMETER;
    }
}

VP_STATUS
H3SetPowerState(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG HwDeviceId,
    PVIDEO_POWER_MANAGEMENT VideoPowerManagement
    )

/*++

Routine Description:

    Set the power state for a given device.

Arguments:

    HwDeviceExtension - Pointer to our hardware device extension structure.

    HwDeviceId        - Private unique 32 bit ID identifing the device.

    VideoPowerManagement - Power state information.

Return Value:

    TRUE if power state can be set,
    FALSE otherwise.

--*/

{
    //
    // We should be called with one of 3 device IDs.
    //  QUERY_NONDDC_MONITOR_ID corresponds to a non-DDC2B monitor.
    //  QUERY_MONITOR_ID corresponds to a DDC2B providing monitor.
    //  DISPLAY_ADAPTER_HW_ID corresponds to the video board.
    //

    if ((HwDeviceId == QUERY_NONDDC_MONITOR_ID) 
		|| (HwDeviceId == QUERY_MONITOR_ID))
	{
        //
        // We support monitor DPMS via the Sync Control Register
        //  and the Miscellaneous Control 2 Register on the DAC.
        //
        //
#if DBG
        VideoDebugPrint((0, "H3SetPowerState - (monitor)  new state=%ld\n",
                         VideoPowerManagement->PowerState));
#endif

        // don't mess with CRT unless it is active.
        if (HwDeviceExtension->monitorActive)
        {
        switch (VideoPowerManagement->PowerState)
            {
            case VideoPowerHibernate:
            case VideoPowerOn:
                //
                // VSYNC and HSYNC output normal.
                // BLANK CONTROL not set.
                //
                H3_SetDPMSPowerState (HwDeviceExtension, (ULONG) 0x00, (UCHAR) 0x00);
                break;

            case VideoPowerStandBy:
                //
                // VSYNC normal, HSYNC pulled low.
                // BLANK CONTROL set.
                //
                H3_SetDPMSPowerState (HwDeviceExtension, (ULONG) 0x08, (UCHAR) 0x20);
                break;

            case VideoPowerSuspend:
                //
                // VSYNC pulled low, HSYNC normal.
                // BLANK CONTROL set.
                //
                H3_SetDPMSPowerState (HwDeviceExtension, (ULONG) 0x02, (UCHAR) 0x20);
                break;

            case VideoPowerOff:
                //
                // VSYNC and HSYNC pulled low.
                // BLANK CONTROL set.
                //
                H3_SetDPMSPowerState (HwDeviceExtension, (ULONG) 0x0A, (UCHAR) 0x20);
                break;

            default:
                //
                // Unexpected power state.
                //
#if DBG
                VideoDebugPrint((0, "Unknown power state.\n"));
                ASSERT(FALSE);
#endif
                return ERROR_INVALID_PARAMETER;
            }
        }
        return NO_ERROR;
    }
    else if (HwDeviceId == QUERY_DFP_ID) 
	{
#if DBG
        VideoDebugPrint((0, "H3SetPowerState - (DFP)  new state=%ld\n",
                         VideoPowerManagement->PowerState));
#endif

        switch (VideoPowerManagement->PowerState)
            {
            case VideoPowerHibernate:
            case VideoPowerOn:
                DfpSetPowerOn(HwDeviceExtension);
                break;

            case VideoPowerStandBy:
            case VideoPowerSuspend:
            case VideoPowerOff:
                DfpSetPowerOff(HwDeviceExtension);
                break;

            default:
                //
                // Unexpected power state.
                //
#if DBG
                VideoDebugPrint((0, "Unknown DFP power state.\n"));
                ASSERT(FALSE);
#endif
                return ERROR_INVALID_PARAMETER;
            }
        return NO_ERROR;
    }
    else if (HwDeviceId == DISPLAY_ADAPTER_HW_ID)
    {
#if DBG
        VideoDebugPrint((0, "H3SetPowerState - (adapter)  old state=%ld  new state=%ld\n",
                         HwDeviceExtension->PowerState,
                         VideoPowerManagement->PowerState));
#endif

        // if we were in S3, then reinit the board
        if ((VideoPowerSuspend == HwDeviceExtension->PowerState) ||
            (VideoPowerOff     == HwDeviceExtension->PowerState))
        {
            PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];

            H3InitializeSecondaryDevice(HwDeviceExtension,
                                        HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX],
                                        HwDeviceExtension->MappedAddress[SST_IO_INDEX]);
            if (! HwDeviceExtension->IsSecondaryDevice)
                RegisterMap->vgaInit0 = SST_VGA0_EXTENSIONS | SST_VGA0_WAKEUP_SELECT;
#ifdef SLI_AA
            InitializeSlaveChipsPCIConfigSpace(HwDeviceExtension);
            InitializeSlaveChipsInitRegs(HwDeviceExtension);
#endif
        }

#if DBG
        DumpH3Regs(HwDeviceExtension, 3);
#endif

        // save new adapter power state
        HwDeviceExtension->PowerState = VideoPowerManagement->PowerState;

        switch (VideoPowerManagement->PowerState)
        {
            case VideoPowerOn:
            case VideoPowerHibernate:
            case VideoPowerStandBy:

                return NO_ERROR;

            case VideoPowerSuspend:
            case VideoPowerOff:

                return NO_ERROR;  //ERROR_INVALID_PARAMETER;

            default:

                //
                // We indicated in H3GetPowerState that we couldn't
                // do VideoPowerOff.  So we should not get a call to
                // do it here.
                //
#if DBG
                ASSERT(FALSE);
#endif
                return ERROR_INVALID_PARAMETER;
        }

    }
	else if (HwDeviceId == QUERY_V3TV_ID)
	{
        switch (VideoPowerManagement->PowerState)
        {
            case VideoPowerOn:
            case VideoPowerHibernate:
            case VideoPowerStandBy:

                return NO_ERROR;

            case VideoPowerSuspend:
            case VideoPowerOff:

                return NO_ERROR;  //ERROR_INVALID_PARAMETER;

            default:

                //
                // We indicated in H3GetPowerState that we couldn't
                // do VideoPowerOff.  So we should not get a call to
                // do it here.
                //
#if DBG
                ASSERT(FALSE);
#endif
                return ERROR_INVALID_PARAMETER;
        }
	}
	else if (HwDeviceId == QUERY_TVOUT_ID)
	{
        switch (VideoPowerManagement->PowerState)
        {
            case VideoPowerOn:
            case VideoPowerHibernate:
            case VideoPowerStandBy:
            case VideoPowerSuspend:
            case VideoPowerOff:

                return NO_ERROR;  // Indicate we handled it but don't actually do anything.???

            default:

                //
#if DBG
                ASSERT(FALSE);
#endif
                return ERROR_INVALID_PARAMETER;
        }
	}
	else
	{
#if DBG
        VideoDebugPrint((0, "Unknown HwDeviceId"));
        //ASSERT(FALSE);
#endif
        return ERROR_INVALID_PARAMETER;
    }
}


VP_STATUS
H3GetPowerManagement(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	)
{
	VP_STATUS status = ERROR_INVALID_FUNCTION;

	VideoDebugPrint((1, "GetPowerManagement\n"));

	//
	// Since we don't support power management right now, tell them so.
	// Later we will return the size of the power management structure
	// which tells the videoport that power management is supported.
	//

	RequestPacket->StatusBlock->Information = 0;

	status = NO_ERROR;

	return status;
}


VP_STATUS
H3SetPowerManagement(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	)
{
	VP_STATUS status = ERROR_INVALID_FUNCTION;

	VideoDebugPrint((1, "SetPowerManagement (not supported)\n"));

	status = ERROR_INVALID_FUNCTION;

	return status;
}

//
// VOID H3_SetDPMSPowerState (PHW_DEVICE_EXTENSION HwDeviceExtension,
//                             ULONG ulDacModemask,
//                             UCHAR ucClockingModemask);
//
// Power Management support function.  Sets up the Blank, Horizontal Sync,
//  and Vertical Sync based upon the OR masks passed in.
//
// Notes on Setting:
//
// DACMODE REGISTER
//
//    ON          bits 0-3 = 0x0 (VSYNC & HSYNC normal)
//    STANDBY     bits 0-3 = 0x8 (VSYNC normal, HSYNC pulled low)
//    SUSPEND     bits 0-3 = 0x2 (VSYNC pulled low, HSYNC normal)
//    OFF         bits 0-3 = 0xA (VSYNC & HSYNC pulled low)
//
// CLOCKING MODE REGISTER (Sequencer index 1)
//
//    ON          bit 5 = 0 (not blanked)
//    STANDBY     bit 5 = 1 (blanked)
//    SUSPEND     bit 5 = 1 (blanked)
//    OFF         bit 5 = 1 (blanked)
//
VOID H3_SetDPMSPowerState (PHW_DEVICE_EXTENSION HwDeviceExtension,
                            ULONG ulDacModemask,
                            UCHAR ucClockingModemask)
{
	PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
	PUCHAR pIO = (PUCHAR) HwDeviceExtension->MappedAddress[SST_IO_INDEX];
	ULONG ulDacMode;
	UCHAR ucClockingMode;
	UCHAR ucSeqIndex;

	// Save Sequencer Index

	ucSeqIndex = VideoPortReadPortUchar((PUCHAR)(pIO + 0x0c4));

	//
	// Set up the ClockingMode Register.
	//

	VideoPortWritePortUchar((PUCHAR)(pIO + 0x0c4), (UCHAR)(0x01));
	ucClockingMode = VideoPortReadPortUchar((PUCHAR)(pIO + 0x0c5));
	ucClockingMode &= ~BIT(5);
	ucClockingMode |= ucClockingModemask;
	VideoPortWritePortUchar((PUCHAR)(pIO + 0x0c5), (UCHAR)(ucClockingMode));

	//
	// Set up the DacMode Register.
	//

	ulDacMode = RegisterMap->dacMode;
	ulDacMode &= ~0x1e;
	ulDacMode |= ulDacModemask;
	RegisterMap->dacMode = ulDacMode;

	// Restore Sequencer Index

	VideoPortWritePortUchar((PUCHAR)(pIO + 0x0c4), ucSeqIndex);
}


   
#endif // (_WIN32_WINNT >= 0x500)

