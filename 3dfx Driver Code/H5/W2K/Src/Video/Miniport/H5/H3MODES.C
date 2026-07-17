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

	h3modes.c

Abstract:

	This module contains the code that does the mode list validation,
	and the workup to the mode set IOCTL service.

Environment:

	Kernel mode

Revision History:

--*/

#ifdef INCSTBCUST
#ifndef MS_VIEW
#include "..\..\..\..\build\stbcust.inc"
#else
#include "stbcust.inc"
#endif
#endif

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "h3.h"
#include "localpci.h"
#include "cmdcnst.h"
#include "dfp.h"

VP_STATUS
H3ValidateModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension
	)
{
	PH3_VIDEO_FREQUENCIES Freq;
	PH3_VIDEO_MODES ModeEntry;
	PH3_VIDEO_FREQUENCIES FrequencyTable;
	VP_STATUS status = ERROR_INVALID_PARAMETER;
	ULONG ModeIndex;
	BOOLEAN FixedFreq;
	ULONG temp, memSize;
	ULONG i, k;

	/////////////////////////////////////////////////////////////////////////
	// XXX Here we will prune valid modes, based on rules according to the chip
	// capabilities and memory requirements.  It would be better if we
	// could make the VESA call to determine the modes that the BIOS
	// supports; however, that requires a buffer and I don't have the
	// time to get it working with our Int 10 support.

	VideoDebugPrint((1, "H3ValidateModes -\n"));

	//
	// We prune modes so that we will not annoy the user by presenting
	// modes in the 'Video Applet' which we know the user can't use.
	//

	HwDeviceExtension->NumAvailableModes = 0;
	HwDeviceExtension->NumTotalModes = 0;

	//
	// Since there are a number of frequencies possible for each
	// distinct resolution/colour depth, we cycle through the
	// frequency table and find the appropriate mode entry for that
	// frequency entry.
	//

	if (HwDeviceExtension->BiosPresent && (HwDeviceExtension->UseNonBIOSModeSet == 0))
	{
		FrequencyTable = HwDeviceExtension->Int10FrequencyTable;
		FixedFreq = FALSE;
	}
	else
	{
		//
		// There is no BIOS, or we're not supposed to use it, so construct a
		// mode list from whatever fixed frequency tables we have for this chip.
		//
		FrequencyTable = HwDeviceExtension->FixedFrequencyTable;
		FixedFreq = TRUE;
	}

	ModeIndex = 0;

#if DEBUG_MODES
	//
	// Dump out the mode and frequency tables prior to the scan,
	// for verification purposes...
	//
	VideoDebugPrint((2, "Mode Table is as follows:\n"));
	VideoDebugPrint((2, "index XXXX YYYY DDD INTC INTN\n"));

	for (ModeEntry = H3Modes, i = 0; i < NumH3VideoModes; ModeEntry++, i++)
	{
		VideoDebugPrint((2, "  %03d %04d %04d %03d %04x %04x\n",
			i,
			ModeEntry->ModeInformation.VisScreenWidth,
			ModeEntry->ModeInformation.VisScreenHeight,
			ModeEntry->ModeInformation.BitsPerPlane,
			ModeEntry->Int10ModeNumberContiguous,
			ModeEntry->Int10ModeNumberNoncontiguous));
	}
	VideoDebugPrint((2, "\n"));
	VideoDebugPrint((2, "Frequency Table is as follows:\n"));
	VideoDebugPrint((2, "index XXXX YYYY DDD RR\n"));

	for (Freq = FrequencyTable, k = 0; Freq->BitsPerPel != 0; Freq++, k++)
	{
		VideoDebugPrint((2, "  %03d %04d %04d %03d %02d\n",
			k,
			Freq->ScreenWidth,
			Freq->ScreenHeight,
			Freq->BitsPerPel,
			Freq->ScreenFrequency));
	}
	VideoDebugPrint((2, "\n"));
#endif
	
	for (Freq = FrequencyTable, ModeIndex = 0, k = 0;
		 Freq->BitsPerPel != 0;
		 Freq++, ModeIndex++, k++)
	{
#if DEBUG_MODES
		VideoDebugPrint((2, "Frequency index %03d ", k));
#endif
		temp = FALSE;

		//
		// Find the mode for this entry.  First, assume we won't find one.
		//

		Freq->ModeValid = FALSE;
		Freq->ModeIndex = ModeIndex;


#ifndef DMT_ENABLED
#ifdef CUST_INF_MODERATES
// STBNW JAC 1-8-99
//
// This determines if the current mode, Freq, is allowed.  If not,
// it simply skips back to the top of the loop since the two lines
// above disable the mode by default.
//
{
		int 	ndx = -1;			// Index into our table that matches the current mode
		BOOLEAN bRefreshRateValid;

		// During initialization, we scanned the registry looking for
		// modes/refresh rates and stored them in our table.  So now,
		// we compare the current Freq-> mode against our table.  If we
		// find a match, we look at the refresh rates array in our table.
		// If Freq->ScreenFrequency is not in our list of rates for this
		// mode, then we disable this mode.
		//
		// However, we have two special cases:
		//		- MODES_ALL_MODES
		//		- "Exclude rates"
		//
		// MODES_ALL_MODES is a special key in the registry that gives the
		// valid rates for all modes.  If this is present, then it overrides
		// all other individual mode information in the registry.
		// MODES_ALL_MODES appears as the first entry in our mode/rates table.
		//
		// "Exclude rates":  If the rates in the registry were preceeded
		// by -1, e.g. "-1,70,85", it means the listed rates are the ones
		// to EXCLUDE.  Normally, the list of rates specify which ones
		// are valid.  The bExcludeRates flag tells use how to interpret
		// modeRates[].refreshRates[].
		//
		if ( modeRates[ 0 ].bFoundInReg )		// Check for MODES_ALL_MODES
		{
			ndx = 0;
		}
		else			// MODES_ALL_MODES override not found, loop through entire table
		{
			for ( i = 1; modeRates[ i ].regStr != NULL; i++ )
			{
				if (  modeRates[ i ].bFoundInReg &&
					 (modeRates[ i ].bpp  == Freq->BitsPerPel  ) &&
					 (modeRates[ i ].hres == Freq->ScreenWidth ) &&
					 (modeRates[ i ].vres == Freq->ScreenHeight)  )
				 {
					ndx = i;
					break;
				 }
			}
		}

		// We've found a match in our mode/rates table for the current mode.
		// Now check Freq's refresh rate against our list of valid refresh rates.
		//
		if ( ndx >= 0 )
		{

			// Default settings
			//
			if ( modeRates[ ndx ].bExcludeRates )
				bRefreshRateValid = TRUE;				// Assume its valid until proven otherwise
			else
				bRefreshRateValid = FALSE;				// Assume its invalid until proven otherwise

			for ( i = 0; i < MAXNUM_REFRESHRATES_PERMODE; i++ )
			{
				if ( modeRates[ ndx ].refreshRates[ i ] == Freq->ScreenFrequency )
				{
					// If flag is set, then it means to exclude the refresh rate.
					// Otherwise, consider it valid.
					//
					if ( modeRates[ ndx ].bExcludeRates )
						bRefreshRateValid = FALSE;
					else
						bRefreshRateValid = TRUE;

					break;
						
				}
			}
		}
		else
			// Mode not in table, so allow refresh rate.
			//
			bRefreshRateValid = TRUE;		

		// Skip this mode/rate combo if the refresh rate is not allowed.
		//
		if ( !bRefreshRateValid )
			continue;    //**********************************************************************

}
#endif // CUST_INF_MODERATES
#endif // ifndef DMT_ENABLED


#ifdef DMT_ENABLED	
		// Under DMT The ModeEntry always matches up to the Frequency entry,
		// so a search for ModeEntry in not needed.
		ModeEntry = Freq->ModeEntry;
#else // DMT_ENABLED
		for (ModeEntry = H3Modes, i = 0; i < NumH3VideoModes; ModeEntry++, i++)
#endif // DMT_ENABLED
		{
			if ((Freq->BitsPerPel  == ModeEntry->ModeInformation.BitsPerPlane) &&
				(Freq->ScreenWidth == ModeEntry->ModeInformation.VisScreenWidth) &&
				(Freq->ScreenHeight == ModeEntry->ModeInformation.VisScreenHeight))
			{
#if DEBUG_MODES
				VideoDebugPrint((2, "%04dx%04dx%02d, ",
					ModeEntry->ModeInformation.VisScreenWidth,
					ModeEntry->ModeInformation.VisScreenHeight,
					ModeEntry->ModeInformation.BitsPerPlane
					));
#endif
				//
				// We've found a mode table entry that matches this frequency
				// table entry.  Now we'll figure out if we can actually do
				// this mode/frequency combination.  For now, assume we'll
				// succeed.
				//

				temp = TRUE;
				Freq->ModeEntry = ModeEntry;
				Freq->ModeValid = TRUE;

				//
				// Flags for private communication with the Banshee display driver.
				//

				ModeEntry->ModeInformation.DriverSpecificAttributeFlags |=
					HwDeviceExtension->Capabilities;

				//
				// Rules for the quirks and capabilities of specific implementations
				// of Banshee go here. For now, there are no known defects requiring
				// the application of restriction rules by mode or frequency...
				//
				if (FixedFreq == TRUE)
				{
					// do nothing
				}
				else if ((ModeEntry->Int10ModeNumberContiguous == 0)
					 &&  (ModeEntry->Int10ModeNumberNoncontiguous == 0))
				{
					//
					// reject the mode because we require a BIOS, and cannot
					// set the mode because there's no mode number!
					//
					Freq->ModeValid = FALSE;
				}

				//
				// Check to see if there's enough memory on the card to support the
				// mode under scrutiny
				//

				memSize = ModeEntry->ModeInformation.VisScreenWidth * ModeEntry->ModeInformation.VisScreenHeight * ModeEntry->ModeInformation.BitsPerPlane;
				memSize = memSize >> 3;

#if DEBUG_MODES
				VideoDebugPrint((2, "memory needed = %07ld", memSize));
#endif

				if (HwDeviceExtension->AdapterMemorySize < memSize)
				{
					Freq->ModeValid = FALSE;
				}

				//
				// Don't forget to count it if it's still a valid mode after
				// applying all those rules.
				//

				if (Freq->ModeValid)
				{
          if ((IS_VOODOO3) &&
              (2048 == Freq->ScreenWidth) &&
              (32   == Freq->BitsPerPel))
          {
            // V3 can't do 2048x1536x32
            // so trim those modes out of mode list on V3
            // but leave the ModeValid bit set, so we can support multimon with
            // a napalm with same miniport
          }
          else
          {
					  HwDeviceExtension->NumAvailableModes++;
          }
				}

#if DEBUG_MODES
				VideoDebugPrint((2, ", index %03d %s\n", i, Freq->ModeValid ? "VALID" : "INVALID"));
#endif

#ifndef DMT_ENABLED
				// We've found a mode for this frequency entry, so we
				// can break out of the mode loop.
				// This break is only required for non-DMT_ENABLED code.	
				break;
#endif // DMT_ENABLED
	
			}
		}
		if (!temp && !Freq->ModeValid)
		{
#if DEBUG_MODES
			VideoDebugPrint((2, " - no mode\n"));
#endif
		}
	}

	if (HwDeviceExtension->NumAvailableModes == 0)
	{
		VideoDebugPrint((0, "\n\nNo modes available!\n\n"));
		return ERROR_DEV_NOT_EXIST;
	}

	HwDeviceExtension->NumTotalModes = ModeIndex;

	VideoDebugPrint((1, "There are %d modes available, last index is %d\n",
		HwDeviceExtension->NumAvailableModes,
		HwDeviceExtension->NumTotalModes));

	return NO_ERROR;
}

VP_STATUS
H3SetCurrentMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	)
{
	VIDEO_X86_BIOS_ARGUMENTS biosArguments;

	PH3_VIDEO_MODES ModeEntry;
	PH3_VIDEO_FREQUENCIES FrequencyEntry;

	VP_STATUS status = ERROR_INVALID_PARAMETER;
	ULONG modeNumber, x;

    VideoDebugPrint((1, "SetCurrentMode\n"));

	//
	// Check if the size of the data in the input buffer is large enough.
	//

	if (RequestPacket->InputBufferLength < sizeof(VIDEO_MODE))
		return ERROR_INSUFFICIENT_BUFFER;

	//
	// Find the correct entries in the H3_VIDEO_MODES and H3_VIDEO_FREQUENCIES
	// tables that correspond to this mode number. (Remember that each mode in
	// the H3_VIDEO_MODES table can have a number of possible frequencies
	// associated with it.)
	//

	modeNumber = ((PVIDEO_MODE) RequestPacket->InputBuffer)->RequestedMode;

	if (modeNumber >= HwDeviceExtension->NumTotalModes)
	{
		VideoDebugPrint((2,"Mode index requested (%d) is out of range\n", modeNumber));
		return status;
	}

	VideoDebugPrint((2,"            Mode index requested = %d\n", modeNumber));

	//
	// Choose: set the mode with *xor* without the BIOS
	//
	if (HwDeviceExtension->BiosPresent
	&& HwDeviceExtension->UseNonBIOSModeSet == 0)
	{
		FrequencyEntry = &HwDeviceExtension->Int10FrequencyTable[modeNumber];

		if (!(FrequencyEntry->ModeValid))
		{
			VideoDebugPrint((1, "H3: bad mode index (%d) in set mode\n", modeNumber));
			return status;
		}

		ModeEntry = FrequencyEntry->ModeEntry;
#if DBG
		DumpModeInformation(HwDeviceExtension, modeNumber, ModeEntry);
#endif
		//
		// At this point, 'ModeEntry' and 'FrequencyEntry' point to the
		// necessary table entries required for setting the requested mode.
		//

		VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

		//
		// Set the refresh rate. If the card doesn't support it, or we don't
		// know what values to use, the requested frequency will be '1', which
		// means 'use the hardware default refresh.'
		//

		VideoDebugPrint((2, "Setting refresh rate to "));
		if (FrequencyEntry->ScreenFrequency != 1)
		{
			switch (FrequencyEntry->ScreenFrequency)
			{
      case 56:
			case 60:
			case 65:
			case 70:
			case 72:
			case 75:
			case 76:
			case 80:
			case 85:
			case 100:
			case 120:
			case 140:
			case 150:
			case 160:
				VideoDebugPrint((2, "%02d Hz\n", FrequencyEntry->ScreenFrequency));
				break;
			default:
				VideoDebugPrint((2, "Invalid refresh rate (%d Hz), using default\n",
					FrequencyEntry->ScreenFrequency));
				FrequencyEntry->ScreenFrequency = DEFAULT_REFRESH_RATE;
				break;
			}
		}

    status = H3SetRefreshRate(HwDeviceExtension,
                              (PH3_MEMBASE0)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX],
                              (PUCHAR)HwDeviceExtension->MappedAddress[SST_IO_INDEX],
                              FrequencyEntry);
		if (status != NO_ERROR)
		{
			VideoDebugPrint((0, "Couldn't set the requested refresh rate!\n"));
			return status;
		}

		//
		// If there's a valid BIOS mode number, attempt an Int 10
		// The basic invalid BIOS mode number is zero.
		//

		if (ModeEntry->Int10ModeNumberContiguous || ModeEntry->Int10ModeNumberNoncontiguous)
		{
			//
			// First try the modeset with the 'Contiguous' mode:
			//

			biosArguments.Ebx = ModeEntry->Int10ModeNumberContiguous;
			biosArguments.Eax = 0x4f02;

			status = VideoPortInt10(HwDeviceExtension, &biosArguments);

			if (status != NO_ERROR)
			{
				VideoDebugPrint((1, "H3: first int10 call FAILED\n"));
			}
			else
			{
				VideoDebugPrint((2, "H3: int10, mode %03x succeeded\n",
					ModeEntry->Int10ModeNumberContiguous));
			}

			VideoDebugPrint((1, "biosArguments.Eax = %0x\n", biosArguments.Eax));

			if ((status == NO_ERROR) && ((biosArguments.Eax & 0xff00) == 0)) {
				//
				// The contiguous mode set succeeded.
				//

				ModeEntry->ModeInformation.ScreenStride = ModeEntry->ScreenStrideContiguous;
			}
			else
			{
				//
				// Try again with the 'Noncontiguous' mode:
				//

				VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

				biosArguments.Ebx = ModeEntry->Int10ModeNumberNoncontiguous;
				biosArguments.Eax = 0x4f02;

				status = VideoPortInt10(HwDeviceExtension, &biosArguments);

				if (status != NO_ERROR)
				{
					VideoDebugPrint((0, "H3: second int10 call FAILED\n"));
				}
				else
				{
					VideoDebugPrint((2, "H3: int10, mode %03x succeeded\n",
									ModeEntry->Int10ModeNumberNoncontiguous));
				}
			}
		}
		else
		{
			//
			// We're hosed - there's a BIOS, but the int 10 failed because
			// the mode numbers are corrupt!
			//
			// Set status to error, and hope that we can use the fixed
			// table mode set to get us out of this bind!
			//
			VideoDebugPrint((0, "H3: No INT10 mode number for mode set!!!\n"));

			status = ERROR_INVALID_FUNCTION;
		}
	}

	//
	// If it failed, we may not be able to perform int10 due
	// to BIOS emulation problems.
	//
	// Do a table mode-set.  First we need to find the
	// right mode table in the fixed Frequency tables.
	//

	//
	// We get into this block because either the BIOS set mode failed, or we're
	// using the non-BIOS mode set code via the registry option
	//
	// If we fell through the previous block, this is our last chance to make
	// it work
	//
	if ((status != NO_ERROR)
	||	(!HwDeviceExtension->BiosPresent
	||   HwDeviceExtension->UseNonBIOSModeSet == 1))
	{

		VideoDebugPrint((2, "H3: Trying fixed mode-set\n"));


		//
		// A problem occured during the int10, or we're not using the
		// BIOS set mode capabilities.	Let's see if we can recover.
		//

		//
		// Let see if we are using a fixed mode table number because we
		// don't have a BIOS, or insist on not using it
		//

		if (!HwDeviceExtension->BiosPresent
		||  (HwDeviceExtension->UseNonBIOSModeSet == 1))
		{
			VideoDebugPrint((2, "H3: fixed frequency mode-set, index = %d\n", modeNumber));

			FrequencyEntry = &HwDeviceExtension->FixedFrequencyTable[modeNumber];

		}
		else
		{
			PH3_VIDEO_FREQUENCIES oldFrequencyEntry = FrequencyEntry;
			PH3_VIDEO_FREQUENCIES newFrequencyEntry;
			PH3_VIDEO_FREQUENCIES bestFrequencyEntry;

			//
			// Okay, we constructed our original mode list assuming
			// we could use Int 10, but we have just discovered the
			// Int 10 didn't work -- probably because there was a
			// problem with the BIOS emulator.	To recover, we will now
			// try to find the best mode in the Fixed Frequency table to
			// match the requested mode.
			//

			FrequencyEntry = NULL;
			bestFrequencyEntry = NULL;

			VideoDebugPrint((2, "H3: fixed frequency mode-set, searching for a mode\n"));

			for (newFrequencyEntry = &HwDeviceExtension->FixedFrequencyTable[0];
				 newFrequencyEntry->BitsPerPel != 0;
				 newFrequencyEntry++) {

				//
				// Check for a matching mode.
				//

				if ((newFrequencyEntry->BitsPerPel	== oldFrequencyEntry->BitsPerPel) &&
					(newFrequencyEntry->ScreenWidth == oldFrequencyEntry->ScreenWidth) &&
					(newFrequencyEntry->ScreenHeight == oldFrequencyEntry->ScreenHeight))
				{
					if (FrequencyEntry == NULL)
					{
						//
						// Remember the first mode that matched, ignoring
						// the frequency.
						//

						FrequencyEntry = newFrequencyEntry;
					}

					if (newFrequencyEntry->ScreenFrequency <= oldFrequencyEntry->ScreenFrequency) {
						//
						// Ideally, we would like to choose the frequency
						// that is closest to, but less than or equal to,
						// the requested frequency.
						//

						if ((bestFrequencyEntry == NULL)
						||  (bestFrequencyEntry->ScreenFrequency < newFrequencyEntry->ScreenFrequency))
						{
							bestFrequencyEntry = newFrequencyEntry;
						}
					}
				}
			}

			//
			// Use the preferred frequency setting, if there is one.
			//

			if (bestFrequencyEntry != NULL)
			{
				FrequencyEntry = bestFrequencyEntry;
			}
			else
			{
				//
				// we have no valid mode, we must return failure
				//
				VideoDebugPrint((0, "H3: no valid Fixed Frequency mode\n"));
				return ERROR_INVALID_PARAMETER;
			}

			//
			// Our new ModeEntry is the same as the old.
			//

			FrequencyEntry->ModeEntry = oldFrequencyEntry->ModeEntry;
			FrequencyEntry->ModeValid = TRUE;

			VideoDebugPrint((2, "H3: Selected Fixed Frequency mode from int 10:\n"));
			VideoDebugPrint((2, "    Bits Per Pel: %d\n", FrequencyEntry->BitsPerPel));
			VideoDebugPrint((2, "    Screen Width: %d\n", FrequencyEntry->ScreenWidth));
			VideoDebugPrint((2, "    Frequency:    %d\n", FrequencyEntry->ScreenFrequency));

		}

		ModeEntry = FrequencyEntry->ModeEntry;

		//
		// NOTE:
		// We have to set the ActiveFrequencyEntry since the SetHWMode
		// function depends on this variable to set the CRTC registers.
		// So lets set it here, and it will get reset to the same
		// value after we set the mode.
		//

		HwDeviceExtension->ActiveFrequencyEntry = FrequencyEntry;

		//
		// Set the refresh rate. If the card doesn't support it, or we don't
		// know what values to use, the requested frequency will be '1', which
		// means 'use the hardware default refresh.'
		//

		VideoDebugPrint((2, "Setting refresh rate to "));
		if (FrequencyEntry->ScreenFrequency != 1)
		{
			switch (FrequencyEntry->ScreenFrequency)
			{
      case 56:
			case 60:
			case 65:
			case 70:
			case 72:
			case 75:
			case 76:
			case 80:
			case 85:
			case 100:
			case 120:
			case 140:
			case 150:
			case 160:
				VideoDebugPrint((2, "%02d Hz\n", FrequencyEntry->ScreenFrequency));
				break;
			default:
				VideoDebugPrint((2, "Invalid refresh rate (%d Hz), using default\n",
					FrequencyEntry->ScreenFrequency));
				FrequencyEntry->ScreenFrequency = DEFAULT_REFRESH_RATE;
				break;
			}
		}

		//
		// Set the mode -- the refresh rate gets set in here, too
		//
    status = H3SetMode(HwDeviceExtension,
                       (PH3_MEMBASE0)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX],
                       (PUCHAR)HwDeviceExtension->MappedAddress[SST_IO_INDEX]);
		if (status != NO_ERROR)
		{
			VideoDebugPrint((0, "Couldn't set mode!\n"));
			return status;
		}
	}

	VideoDebugPrint((2, "Collecting Stride information\n"));

#if BIOS_VBE_BROKEN
	//
	// bios function 4f06 is currently broken, so just take the stride
	// from the mode entry, and we'll live with that for now...
	//
	ModeEntry->ModeInformation.ScreenStride = ModeEntry->ScreenStrideContiguous;
	VideoDebugPrint((2, "ScreenStride set to %ld\n", ModeEntry->ModeInformation.ScreenStride));
#else
	//
	// Call Int 10, function 0x4f06 to obtain the correct screen pitch
	//
	if (HwDeviceExtension->BiosPresent && (HwDeviceExtension->UseNonBIOSModeSet == 0))
	{
		VideoPortZeroMemory(&biosArguments,sizeof(VIDEO_X86_BIOS_ARGUMENTS));

		biosArguments.Ebx = 0x0001;
		biosArguments.Eax = 0x4f06;

		status = VideoPortInt10(HwDeviceExtension, &biosArguments);

		//
		// Check to see if the Bios supported this function, and if so
		// update the screen stride for this mode.
		//

		if ((status == NO_ERROR) && (biosArguments.Eax & 0xffff) == 0x004f)
		{
			ModeEntry->ModeInformation.ScreenStride = biosArguments.Ebx;
			VideoDebugPrint((3, "biosArguments = %08x, biosArguments.Ebx = %0x\n", biosArguments, biosArguments.Ebx));
		}
		else
		{
			//
			// We will use the default value in the mode table.
			//
			ModeEntry->ModeInformation.ScreenStride = ModeEntry->ScreenStrideContiguous;
		}
	}
#endif
	//
	// Save the mode since we know the rest will work.
	//

	HwDeviceExtension->ActiveModeEntry = ModeEntry;
	HwDeviceExtension->ActiveFrequencyEntry = FrequencyEntry;

	//
	// Record the fact that we are in an H3 mode, and
	// that we need to be reset.
	//

	HwDeviceExtension->bNeedReset = TRUE;

	VideoDebugPrint((2, "Updating VIDEO_MODE_INFORMATION\n"));

	//////////////////////////////////////////////////////////////////
	// Update VIDEO_MODE_INFORMATION fields
	//
	// Now that we've set the mode, we now know the screen stride, and
	// so can update some fields in the VIDEO_MODE_INFORMATION
	// structure for this mode.  The display driver is expected to
	// call IOCTL_VIDEO_QUERY_CURRENT_MODE to query these corrected
	// values.
	//

	//
	// Calculate the bitmap width.
	// We currently assume the bitmap width is equivalent to the stride.
	//

	VideoDebugPrint((2, "BitsPerPlane = %ld\n", ModeEntry->ModeInformation.BitsPerPlane));
	VideoDebugPrint((2, "ScreenStride = %ld\n", ModeEntry->ModeInformation.ScreenStride));

	x = ModeEntry->ModeInformation.BitsPerPlane;

	ModeEntry->ModeInformation.VideoMemoryBitmapWidth =
		(ModeEntry->ModeInformation.ScreenStride << 3) / x;

	VideoDebugPrint((2, "VideoMemoryBitmapWidth = %d\n",
		ModeEntry->ModeInformation.VideoMemoryBitmapWidth));

	//
	// If we're in a mode that the BIOS doesn't really support, it may
	// have reported back a bogus screen width.
	//

	if (ModeEntry->ModeInformation.VideoMemoryBitmapWidth <
		ModeEntry->ModeInformation.VisScreenWidth)
	{
		VideoDebugPrint((0, "H3: BIOS returned invalid width, bitmap width=%ld, screen width=%ld\n",
			ModeEntry->ModeInformation.VideoMemoryBitmapWidth,
			ModeEntry->ModeInformation.VisScreenWidth
			));
		return ERROR_INVALID_PARAMETER;
	}

	//
	// Calculate the bitmap height.
	//

	ModeEntry->ModeInformation.VideoMemoryBitmapHeight =
		HwDeviceExtension->AdapterMemorySize /
		ModeEntry->ModeInformation.ScreenStride;

	VideoDebugPrint((2, "VideoMemoryBitmapHeight = %d\n",
		ModeEntry->ModeInformation.VideoMemoryBitmapHeight));

	VideoDebugPrint((1, "IOCTL_VIDEO_SET_CURRENT_MODE okay!\n"));

	return NO_ERROR;
}

VP_STATUS
H3QueryAvailableModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	)
{
	PH3_VIDEO_FREQUENCIES FrequencyEntry;
	PH3_VIDEO_FREQUENCIES FrequencyTable;
	PVIDEO_MODE_INFORMATION modeInformation;
	VP_STATUS status = ERROR_INVALID_PARAMETER;

    VideoDebugPrint((1, "QueryAvailableModes\n"));
    VideoDebugPrint((2, "\t\tThere are %d available modes\n", HwDeviceExtension->NumAvailableModes));

	if (RequestPacket->OutputBufferLength <
		(RequestPacket->StatusBlock->Information =
			 HwDeviceExtension->NumAvailableModes * sizeof(VIDEO_MODE_INFORMATION)))
	{
		status = ERROR_INSUFFICIENT_BUFFER;
	}
	else
	{
		modeInformation = RequestPacket->OutputBuffer;

		if (HwDeviceExtension->BiosPresent && HwDeviceExtension->UseNonBIOSModeSet == 0)
		{
			FrequencyTable = HwDeviceExtension->Int10FrequencyTable;
		}
		else
		{
			FrequencyTable = HwDeviceExtension->FixedFrequencyTable;
		}

		for (FrequencyEntry = FrequencyTable; FrequencyEntry->BitsPerPel != 0; FrequencyEntry++)
		{
			if (FrequencyEntry->ModeValid)
			{
        if (((IS_VOODOO3) &&
            (2048 == FrequencyEntry->ScreenWidth) &&
            (32   == FrequencyEntry->BitsPerPel))
          // V3 can't do 2048x1536x32
          // so trim those modes out of mode list on V3
          // but leave the ModeValid bit set, so we can support multimon with
          // a napalm with same miniport
            ||
           ( HwDeviceExtension->tvOutActive  &&
             (!FrequencyEntry->isValidTvOutMode)))
	      // test if TvOut is active and this mode is NOT valid for TvOut

        {
        //NO-OP
        }
        else
        {
          *modeInformation = FrequencyEntry->ModeEntry->ModeInformation;
          modeInformation->Frequency = FrequencyEntry->ScreenFrequency;
          modeInformation->ModeIndex = FrequencyEntry->ModeIndex;
  
          modeInformation++;
        }
			}
		}
		status = NO_ERROR;
	}
	return status;
}

VP_STATUS
H3QueryNumAvailableModes(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	)
{
	VP_STATUS status = ERROR_INVALID_PARAMETER;

	VideoDebugPrint((1, "QueryNumAvailableModes\n"));
	VideoDebugPrint((2, "\t\tThere are %d available modes\n",
					HwDeviceExtension->NumAvailableModes));

	//
	// Find out the size of the data to be put in the the buffer and
	// return that in the status information (whether or not the
	// information is there). If the buffer passed in is not large
	// enough return an appropriate error code.
	//

	if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(VIDEO_NUM_MODES)))
		return ERROR_INSUFFICIENT_BUFFER;

	((PVIDEO_NUM_MODES)RequestPacket->OutputBuffer)->NumModes =
		HwDeviceExtension->NumAvailableModes;

	((PVIDEO_NUM_MODES)RequestPacket->OutputBuffer)->ModeInformationLength =
		sizeof(VIDEO_MODE_INFORMATION);

	return NO_ERROR;
}

VP_STATUS
H3QueryCurrentMode(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	)
{
	VP_STATUS status = ERROR_INVALID_PARAMETER;

	VideoDebugPrint((1, "QueryCurrentMode\n"));

#if DEBUG_MODES
	if (HwDeviceExtension->ActiveModeEntry != NULL)
	{
		VideoDebugPrint((2, "\t\tThe current mode is index=%d, x=%d, y=%d, bpp=%d, stride=%d\n",
			HwDeviceExtension->ActiveModeEntry->ModeInformation.ModeIndex,
			HwDeviceExtension->ActiveModeEntry->ModeInformation.VisScreenWidth,
			HwDeviceExtension->ActiveModeEntry->ModeInformation.VisScreenHeight,
			HwDeviceExtension->ActiveModeEntry->ModeInformation.BitsPerPlane,
			HwDeviceExtension->ActiveModeEntry->ModeInformation.ScreenStride
			));
	}
	else
		VideoDebugPrint((1, "\t\tThere is no active mode!\n"));

	if (HwDeviceExtension->ActiveFrequencyEntry != NULL)
	{
		VideoDebugPrint((2, "\t\tThe current frequency entry is index=%d, frequency=%d\n",
			HwDeviceExtension->ActiveFrequencyEntry->ModeIndex,
			HwDeviceExtension->ActiveFrequencyEntry->ScreenFrequency
			));
	}
	else
		VideoDebugPrint((1, "\t\tThere is no active frequency entry!\n"));
#endif

	if (RequestPacket->OutputBufferLength < (RequestPacket->StatusBlock->Information = sizeof(VIDEO_MODE_INFORMATION)))
	{
		status = ERROR_INSUFFICIENT_BUFFER;
	}
	else
	{
		*((PVIDEO_MODE_INFORMATION)RequestPacket->OutputBuffer) =
			HwDeviceExtension->ActiveModeEntry->ModeInformation;

		((PVIDEO_MODE_INFORMATION)RequestPacket->OutputBuffer)->Frequency =
			HwDeviceExtension->ActiveFrequencyEntry->ScreenFrequency;

		status = NO_ERROR;
	}
	return status;
}
