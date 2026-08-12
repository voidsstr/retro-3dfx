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

	h3registry.c

Abstract:

	This module contains the code that implements registry routines
	for the H3 miniport driver.

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

#define FULL_DRIVER_COMPILE 		1

#if FULL_DRIVER_COMPILE

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

#else	// miniport driver

#include "miniport.h"

#endif	// FULL_DRIVER_COMPILE

#include "dderror.h"
#include "devioctl.h"

#include "ntddvdeo.h"
#include "video.h"
#include "h3.h"
#include "localpci.h"

//#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,H3QueryRegistryValue)
#pragma alloc_text(PAGE,H3SetRegistryValue)
//#endif

#ifndef DMT_ENABLED
#ifdef CUST_INF_MODERATES
// STBNW JAC 1-8-99
// STBNW JAC 2-12-99  Updated with new modes...

// typedef MODERATES is located in H3.h.

MODERATES modeRates[] =
{
	// This is a table of all the mode strings that we attemp to read
	// from the registry.  If you add a new driver mode, then you'll
	// need to add an entry here because if it's not in this table,
	// then we won't go looking for it in the registry.
	//
	// The first element is special.  If it appears in the registry it will override
	// all other mode entries.  The rates specified by MODES_ALL_MODES will be the
	// only valid rates across all modes and color depths.
	//
	// One other special case:  If the rates in the registry were preceeded
	// by -1, e.g. "-1,70,85", it means the listed rates are the ones
	// to EXCLUDE.  Normally, the list of rates specify which ones
	// are valid.  The bExcludeRates flag tells use how to interpret
	// the refreshRates[], i.e. include or exclude.
	//
	//
	{ L"MODES_ALL_MODES",		0,		0,		0	 },	// **MUST** be first entry

	{ L"MODES_8_320,200", 		8,		320,	200	 },
	{ L"MODES_8_320,240", 		8,		320,	240	 },
	{ L"MODES_8_400,300", 		8,		400,	300	 },
	{ L"MODES_8_512,384", 		8,		512,	384	 },
	{ L"MODES_8_640,400", 		8,		640,	400	 },
	{ L"MODES_8_640,480", 		8,		640,	480	 },
	{ L"MODES_8_720,480", 		8,		720,	480	 },
	{ L"MODES_8_720,576", 		8,		720,	576	 },
	{ L"MODES_8_800,600", 		8,		800,	600  },
	{ L"MODES_8_960,720", 		8,		960,	720  },
	{ L"MODES_8_1024,768", 		8,		1024,	768  },
	{ L"MODES_8_1152,864", 		8,		1152,	864  },
	{ L"MODES_8_1280,960",		8,		1280,	960  },
	{ L"MODES_8_1280,1024",		8,		1280,	1024 },
	{ L"MODES_8_1600,1024", 	8,		1600,	1024 },
	{ L"MODES_8_1600,1200",		8,		1600,	1200 },
	{ L"MODES_8_1792,1344",		8,		1792,	1344 },
	{ L"MODES_8_1856,1392",		8,		1856,	1392 },
	{ L"MODES_8_1920,1080",		8,		1920,	1080 },
	{ L"MODES_8_1920,1200",		8,		1920,	1200 },
	{ L"MODES_8_1920,1440",		8,		1920,	1440 },
	{ L"MODES_8_2046,1536", 	8,		2046,	1536 },

	{ L"MODES_16_320,200", 		16,		320,	200	 },
	{ L"MODES_16_320,240", 		16,		320,	240	 },
	{ L"MODES_16_400,300", 		16,		400,	300	 },
	{ L"MODES_16_512,384", 		16,		512,	384	 },
	{ L"MODES_16_640,400", 		16,		640,	400	 },
	{ L"MODES_16_640,480", 		16,		640,	480	 },
	{ L"MODES_16_720,480", 		16,		720,	480	 },
	{ L"MODES_16_720,576", 		16,		720,	576	 },
	{ L"MODES_16_800,600", 		16,		800,	600  },
	{ L"MODES_16_960,720", 		16,		960,	720  },
	{ L"MODES_16_1024,768",		16,		1024,	768  },
	{ L"MODES_16_1152,864",		16,		1152,	864  },
	{ L"MODES_16_1280,960",		16,		1280,	960  },
	{ L"MODES_16_1280,1024", 	16,		1280,	1024 },
	{ L"MODES_16_1600,1024", 	16,		1600,	1024 },
	{ L"MODES_16_1600,1200", 	16,		1600,	1200 },
	{ L"MODES_16_1792,1344", 	16,		1792,	1344 },
	{ L"MODES_16_1856,1392", 	16,		1856,	1392 },
	{ L"MODES_16_1920,1080",	16,		1920,	1080 },
	{ L"MODES_16_1920,1200",	16,		1920,	1200 },
	{ L"MODES_16_1920,1440", 	16,		1920,	1440 },
	{ L"MODES_16_2046,1536", 	16,		2046,	1536 },

	{ L"MODES_24_320,200", 		24,		320,	200	 },
	{ L"MODES_24_320,240", 		24,		320,	240	 },
	{ L"MODES_24_400,300", 		24,		400,	300	 },
	{ L"MODES_24_512,384", 		24,		512,	384	 },
	{ L"MODES_24_640,400", 		24,		640,	400	 },
	{ L"MODES_24_640,480", 		24,		640,	480	 },
	{ L"MODES_24_720,480", 		24,		720,	480	 },
	{ L"MODES_24_720,576", 		24,		720,	576	 },
	{ L"MODES_24_800,600", 		24,		800,	600  },
	{ L"MODES_24_960,720", 		24,		960,	720  },
	{ L"MODES_24_1024,768",		24,		1024,	768  },
	{ L"MODES_24_1152,864",		24,		1152,	864  },
	{ L"MODES_24_1280,960",		24,		1280,	960  },
	{ L"MODES_24_1280,1024", 	24,		1280,	1024 },
	{ L"MODES_24_1600,1024", 	24,		1600,	1024 },
	{ L"MODES_24_1600,1200", 	24,		1600,	1200 },
	{ L"MODES_24_1792,1344", 	24,		1792,	1344 },
	{ L"MODES_24_1856,1392", 	24,		1856,	1392 },
	{ L"MODES_24_1920,1080",	24,		1920,	1080 },
	{ L"MODES_24_1920,1200",	24,		1920,	1200 },
	{ L"MODES_24_1920,1440", 	24,		1920,	1440 },
	{ L"MODES_24_2046,1536", 	24,		2046,	1536 },

	{ L"MODES_32_320,200", 		32,		320,	200	 },
	{ L"MODES_32_320,240", 		32,		320,	240	 },
	{ L"MODES_32_400,300", 		32,		400,	300	 },
	{ L"MODES_32_512,384", 		32,		512,	384	 },
	{ L"MODES_32_640,400", 		32,		640,	400	 },
	{ L"MODES_32_640,480",		32,		640,	480	 },
	{ L"MODES_32_720,480", 		32,		720,	480	 },
	{ L"MODES_32_720,576", 		32,		720,	576	 },
	{ L"MODES_32_800,600", 		32,		800,	600  },
	{ L"MODES_32_960,720", 		32,		960,	720  },
	{ L"MODES_32_1024,768",		32,		1024,	768  },
	{ L"MODES_32_1152,864",		32,		1152,	864  },
	{ L"MODES_32_1280,960",		32,		1280,	960  },
	{ L"MODES_32_1280,1024", 	32,		1280,	1024 },
	{ L"MODES_32_1600,1024", 	32,		1600,	1024 },
	{ L"MODES_32_1600,1200", 	32,		1600,	1200 },
	{ L"MODES_32_1792,1344", 	32,		1792,	1344 },
	{ L"MODES_32_1856,1392", 	32,		1856,	1392 },
	{ L"MODES_32_1920,1080",	32,		1920,	1080 },
	{ L"MODES_32_1920,1200",	32,		1920,	1200 },
	{ L"MODES_32_1920,1440", 	32,		1920,	1440 },
	{ L"MODES_32_2046,1536", 	32,		2046,	1536 },
	{ NULL }						// Terminator
};

#endif // CUST_INF_MODERATES
#endif // ifndef DMT_ENABLED

PVOID
H3CopyUnicodeString(
	PVOID SourceString
	)
{
	PUNICODE_STRING SrcString = SourceString;
	PUNICODE_STRING DestString;

	DestString = (PUNICODE_STRING)ExAllocatePool(
									PagedPool,
									SrcString->MaximumLength + sizeof(UNICODE_STRING));
	if (DestString == NULL)
	{
		VideoDebugPrint((1, "H3CopyUnicodeString: ExAllocatePool failed\n"));
		return(NULL);
	}

	DestString->MaximumLength = SrcString->MaximumLength;
	DestString->Buffer = (PWSTR)(DestString+1);
	RtlCopyUnicodeString(DestString, SrcString);

	return(DestString);
}

VP_STATUS
H3GetGammaTable(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	)
/*++

Routine Description:

	This routine gets the gamma table after validating the input, and returns
	the table to the caller

Arguments:

	HwDeviceExtension - Pointer to the miniport driver's device extension.

Return Value:

	None.

--*/

{
	VP_STATUS status = ERROR_INVALID_PARAMETER;
	int i;

	UNREFERENCED_PARAMETER(i);

	VideoDebugPrint((1,"H3GetGammaTable\n"));

	RequestPacket->StatusBlock->Information = (NUM_CLUT_ENTRIES * sizeof(ULONG));

	if (RequestPacket->OutputBufferLength < RequestPacket->StatusBlock->Information)
		return ERROR_INSUFFICIENT_BUFFER;

	status = VideoPortGetRegistryParameters(
					HwDeviceExtension,
					L"GammaTable",
					FALSE,
					H3RetrieveGammaCallback,
					RequestPacket->OutputBuffer);

	if (status != NO_ERROR)
	{
		VideoDebugPrint((0,"\n\nVoodoo3:\n\tGamma Table in Registry is corrupt or missing!\n\n"));
		return status;
	}
	else
	{
		PULONG tmp = (PULONG)RequestPacket->OutputBuffer;
#if DBG
		VideoDebugPrint((2,"Gamma table retrieved from registry is as follows:\n"));
		for (i=0; i<NUM_CLUT_ENTRIES; i++)
		{
			VideoDebugPrint((2,"%06lx ", tmp[i]));
			if ((i % 8) == 7)
				VideoDebugPrint((2,"\n"));
		}
#endif
		memcpy(HwDeviceExtension->GammaTable, tmp, (NUM_CLUT_ENTRIES * sizeof(ULONG)));
	}
	return NO_ERROR;
}

static const char GAMMATABLE[] = "GammaTable";

VP_STATUS
H3SetGammaTable(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	)
/*++

Routine Description:

	This routine sets the gamma table after validating the input, using the
	table provided by the caller

Arguments:

	HwDeviceExtension - Pointer to the miniport driver's device extension.

	RequestPacket - the packet for the data

Return Value:

	NO_ERROR, ERROR_INVALID_PARAMETER, or ERROR_INSUFFICIENT_BUFFER

--*/

{
	VP_STATUS status = ERROR_INVALID_PARAMETER;

	VideoDebugPrint((1,"H3SetGammaTable\n"));

	RequestPacket->StatusBlock->Information = (NUM_CLUT_ENTRIES * sizeof(ULONG));

	if (RequestPacket->InputBufferLength < RequestPacket->StatusBlock->Information)
		return ERROR_INSUFFICIENT_BUFFER;

	memcpy(HwDeviceExtension->GammaTable,
		   (PULONG)RequestPacket->InputBuffer,
		   (NUM_CLUT_ENTRIES * sizeof(ULONG)));

	status = VideoPortSetRegistryParameters(HwDeviceExtension,
											(PUSHORT)L"GammaTable",
											RequestPacket->InputBuffer,
											(NUM_CLUT_ENTRIES * sizeof(ULONG)));

	H3UpdateGamma(HwDeviceExtension,
                (PH3_MEMBASE0)HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX],
                (PUCHAR)HwDeviceExtension->MappedAddress[SST_IO_INDEX],
                HwDeviceExtension->GammaTable);

	return NO_ERROR;
}

VP_STATUS
H3GetGlideGammaTable(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	)
/*++

Routine Description:

	This routine gets the gamma table after validating the input, and returns
	the table to the caller

Arguments:

	HwDeviceExtension - Pointer to the miniport driver's device extension.

Return Value:

	None.

--*/

{
	VP_STATUS status = ERROR_INVALID_PARAMETER;
	VIDEO_REQUEST_PACKET Packet;
	STATUS_BLOCK StatusBlock;
	TDFX_SET_VALUE_INFO ValInfo;
	UCHAR InputBuffer[] = "GlideGammaTable";
	int i;

	UNREFERENCED_PARAMETER(Packet);
	UNREFERENCED_PARAMETER(StatusBlock);
	UNREFERENCED_PARAMETER(ValInfo);

	VideoDebugPrint((1,"H3GetGlideGammaTable\n"));

	RequestPacket->StatusBlock->Information = (NUM_CLUT_ENTRIES * sizeof(ULONG));

	if (RequestPacket->OutputBufferLength < RequestPacket->StatusBlock->Information)
		return ERROR_INSUFFICIENT_BUFFER;

	StatusBlock.Information = (NUM_CLUT_ENTRIES * sizeof(ULONG));

	Packet.StatusBlock = (PVOID)&StatusBlock;
	Packet.InputBuffer = (PVOID)InputBuffer;
	Packet.InputBufferLength = strlen(InputBuffer);
	Packet.OutputBuffer = (PVOID)&ValInfo;
	Packet.OutputBufferLength = sizeof(TDFX_SET_VALUE_INFO);

	status = H3GetRegistryValue(HwDeviceExtension,&Packet);

	if (status != NO_ERROR)
	{
		VideoDebugPrint((0,"\n\nVoodoo3:\n\tGlide Gamma Table in Registry is corrupt or missing!\n\n"));
		return status;
	}
	else
	{
		PULONG tmp = (PULONG)RequestPacket->OutputBuffer;
#if DBG
		VideoDebugPrint((2,"Glide Gamma table retrieved from registry is as follows:\n"));
		for (i=0; i<NUM_CLUT_ENTRIES; i++)
		{
			VideoDebugPrint((2,"%06lx ", tmp[i]));
			if ((i % 8) == 7)
				VideoDebugPrint((2,"\n"));
		}
#endif
		memcpy(HwDeviceExtension->GlideGammaTable, tmp, (NUM_CLUT_ENTRIES * sizeof(ULONG)));
	}
	return NO_ERROR;
}

VP_STATUS
H3SetGlideGammaTable(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	)
/*++

Routine Description:

	This routine sets the gamma table after validating the input, using the
	table provided by the caller

Arguments:

	HwDeviceExtension - Pointer to the miniport driver's device extension.

	RequestPacket - the packet for the data

Return Value:

	NO_ERROR, ERROR_INVALID_PARAMETER, or ERROR_INSUFFICIENT_BUFFER

--*/

{
	PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
	VP_STATUS status = ERROR_INVALID_PARAMETER;
	VIDEO_REQUEST_PACKET Packet;
	TDFX_SET_VALUE_INFO ValInfo;

	VideoDebugPrint((1,"H3SetGlideGammaTable\n"));
	RequestPacket->StatusBlock->Information = (NUM_CLUT_ENTRIES * sizeof(ULONG));

	if (RequestPacket->InputBufferLength < RequestPacket->StatusBlock->Information)
		return ERROR_INSUFFICIENT_BUFFER;

	memcpy(RequestPacket->InputBuffer,
		   hwDeviceExtension->GlideGammaTable,
		   (NUM_CLUT_ENTRIES * sizeof(ULONG)));

//
// Don't do the hardware update, because we don't do this until the display driver enters
// exclusive mode
//
//	H3UpdateGamma(hwDeviceExtension, hwDeviceExtension->GlideGammaTable);

	ValInfo.ValueNameLength = 15;
	memcpy("GlideGammaTable",
		   &ValInfo.ValueName,
		   ValInfo.ValueNameLength);
	ValInfo.Type = REG_BINARY;
	ValInfo.DataLength = (NUM_CLUT_ENTRIES * sizeof(ULONG));
	memcpy(RequestPacket->InputBuffer,
		   &ValInfo.Data,
		   ValInfo.DataLength);

	status = H3SetRegistryValue(HwDeviceExtension,&Packet);

	return NO_ERROR;
}

VP_STATUS
H3RetrieveGammaCallback(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVOID Context,
	PWSTR ValueName,
	PVOID ValueData,
	ULONG ValueLength
	)

/*++

Routine Description:

	This routine is used to read back the gamma LUT from the registry.

Arguments:

	HwDeviceExtension - Supplies a pointer to the miniport's device extension.

	Context - Context value passed to the get registry paramters routine

	ValueName - Name of the value requested.

	ValueData - Pointer to the requested data.

	ValueLength - Length of the requested data.

Return Value:

	if the variable doesn't exist return an error,
	else copy the gamma lut into the supplied pointer

--*/

{
	UNREFERENCED_PARAMETER(HwDeviceExtension);
	UNREFERENCED_PARAMETER(ValueName);

	VideoDebugPrint((0,"H3RetrieveGammaCallback -\n"));
	if (ValueLength != (NUM_CLUT_ENTRIES * sizeof(ULONG)))
	{
		VideoDebugPrint((0,"H3RetrieveGammaCallback got ValueLength of %d\n", ValueLength));
		return ERROR_INVALID_PARAMETER;
	}

	if (Context)
	{
		VideoPortMoveMemory(Context, ValueData, (NUM_CLUT_ENTRIES * sizeof(ULONG)));
	}
	else
		return STATUS_UNSUCCESSFUL;

	return NO_ERROR;

} // end H3RetrieveGammaCallback()

VP_STATUS
H3RegistryCallback(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVOID Context,
	PWSTR ValueName,
	PVOID ValueData,
	ULONG ValueLength
	)

/*++

Routine Description:

	This routine determines if the alternate register set was requested via
	the registry.

Arguments:

	HwDeviceExtension - Supplies a pointer to the miniport's device extension.

	Context - Context value passed to the get registry paramters routine.

	ValueName - Name of the value requested.

	ValueData - Pointer to the requested data.

	ValueLength - Length of the requested data.

Return Value:

	returns NO_ERROR if the paramter was TRUE.
	returns ERROR_INVALID_PARAMETER otherwise.

--*/

{
	UNREFERENCED_PARAMETER(HwDeviceExtension);
	UNREFERENCED_PARAMETER(ValueName);

	VideoDebugPrint((2, "H3RegistryCallback - "));

	if (ValueLength)
	{
		if (Context)
		{
			*(ULONG *)Context = *(PULONG)ValueData;
		}
		else
		if (*((PULONG)ValueData) != 0)

			return ERROR_INVALID_PARAMETER;

		return NO_ERROR;
	}
	else
	{
		return ERROR_INVALID_PARAMETER;
	}
}

#ifndef DMT_ENABLED
#ifdef CUST_INF_MODERATES
// STBNW JAC 1-8-99


VP_STATUS
H3GetRefreshRatesCallback(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVOID Context,
	PWSTR ValueName,
	PVOID ValueData,
	ULONG ValueLength
	)
{
	if ( !Context )
		return ERROR_INVALID_PARAMETER;

	if ( !ValueLength )			// No data associated with valueName
	{
		*(char*)Context = '\0';
		return NO_ERROR;
	}
	else
	{
#if (_WIN32_WINNT >= 0x0500)
    PWSTR   pucStr;
    PCHAR   pansiStr;

    pucStr = (PWSTR)ValueData;
    pansiStr = (PCHAR)Context;

    while (*pucStr)
      *pansiStr++ = (*pucStr++ & 0xFF);
    *pansiStr = '\0';
#else
		UNICODE_STRING		  ucStr;
		ANSI_STRING 		  ansiStr;

		// String characters are stored in the registry as double byte (wide) characters.
		// Convert from wide characters to single byte characters.
		//
		RtlInitUnicodeString(&ucStr, (PWSTR)ValueData);
		ucStr.Length = (USHORT)ValueLength;
		RtlUnicodeStringToAnsiString(&ansiStr, &ucStr, TRUE);

		memcpy((char*)Context, ansiStr.Buffer, ansiStr.Length);
		((char*)Context)[ansiStr.Length] = '\0';

		RtlFreeAnsiString(&ansiStr);
#endif

		return NO_ERROR;
	}
}

VOID
H3ReadRefreshRates ( PHW_DEVICE_EXTENSION HwDeviceExtension )
{
	int			i,j,k,ndx,value,ch;
	VP_STATUS 	status;
	char		tmpBuff[512];

	i = 0;
	for ( i = 0; modeRates[i].regStr != NULL; i++ )
	{

		modeRates[i].bFoundInReg   = FALSE;
		modeRates[i].bExcludeRates = FALSE;
		memset( (PVOID)&modeRates[i].refreshRates, 0, sizeof(ULONG) * MAXNUM_REFRESHRATES_PERMODE );

		VideoDebugPrint((2, "Reading modeRates[%d%] from registry\n", i));
		status = VideoPortGetRegistryParameters(HwDeviceExtension,
												modeRates[ i ].regStr,
												FALSE,
												H3GetRefreshRatesCallback,
												&tmpBuff);
	
		if (status != NO_ERROR)
		{
			VideoDebugPrint((2, "\trefresh rate info not present or read error\n"));

			// If we are here, then the mode was not even in the registry.  So
			// we will try to use the refresh rates associated with the 8bpp mode.
			//
			if ( i > 0 )		// Don't do this for the special MODES_ALL_MODES reg entry
			{
				// Search through our table looking for the 8bpp mode
				//
				for ( k = 1; modeRates[k].regStr != NULL; k++ )
				{

					if ( (modeRates[k].bpp  == 8    ) &&
						 (modeRates[k].hres == modeRates[i].hres) &&
						 (modeRates[k].vres == modeRates[i].vres) )
					{
						if ( modeRates[k].bFoundInReg )			// Q: This 8bpp have any refresh rates?
						{
							modeRates[i].bFoundInReg   = TRUE;
							modeRates[i].bExcludeRates = modeRates[k].bExcludeRates;
							memcpy( (PVOID)&modeRates[i].refreshRates,
									(PVOID)&modeRates[k].refreshRates,
									sizeof(ULONG) * MAXNUM_REFRESHRATES_PERMODE );
						}
						break;
					}
				}
			}
		}
		else
		{
			ANSI_STRING ansi;
			BOOLEAN		bStoreRate = FALSE;;

			VideoDebugPrint((2, "\tgot refresh rate info: %s", tmpBuff));

			// Prepare rates string for processing...
			//
#if (_WIN32_WINNT >= 0x0500)
      ansi.Length = strlen(tmpBuff);
      ansi.MaximumLength = sizeof(tmpBuff);
      ansi.Buffer = tmpBuff;
#else
			RtlInitAnsiString ( &ansi, tmpBuff );
#endif

			// If the mode string was in the registry, but no refresh rates were specified,
			// then use the rates associated with the same mode in 8bpp.
			//
			if ( ansi.Length == 0 )
			{
				// Search through our table looking for the 8bpp mode
				//
				for ( k = 1; modeRates[k].regStr != NULL; k++ )
				{
					if ( (modeRates[k].bpp  == 8    ) &&
						 (modeRates[k].hres == modeRates[i].hres) &&
						 (modeRates[k].vres == modeRates[i].vres) )
					{
						if ( modeRates[k].bFoundInReg )			// Q: This 8bpp have any refresh rates?
						{
							modeRates[i].bFoundInReg   = TRUE;
							modeRates[i].bExcludeRates = modeRates[k].bExcludeRates;
							memcpy( (PVOID)&modeRates[i].refreshRates,
									(PVOID)&modeRates[k].refreshRates,
									sizeof(ULONG) * MAXNUM_REFRESHRATES_PERMODE );
						}
						break;
					}
				}

			}
			else	// Mode string present in registry and has values
			{
				// Is this a string of EXCLUDE rates?  If so, the first value will
				// be -1, e.g. "-1,85,120"
				//
				if ( (ansi.Length > 2 ) && (ansi.Buffer[0] == '-') && (ansi.Buffer[1] == '1') )
				{
					modeRates[i].bExcludeRates = TRUE;

					// Now increment to the next NUMERIC character in string.
					//
					for ( j = 2, ch = ansi.Buffer[ j ] - '0';
						  ( (ch < 0 || ch > 9) && (j < ansi.Length) );
						  ch = ansi.Buffer[ ++j ] - '0' )
					{
						; 	// Everything is done in the for-loop statement
					}
					
				}
				else
					j = 0;	// No special handling, start at beginning of string

				// Now we have to convert the rates from a string to individual integer values.
				//
				for ( ndx = 0, value = 0; j < ansi.Length ; j++ )
				{

					int ch = ansi.Buffer[ j ] - '0';	// Convert current ascii character to a "number"

					if ( ch >= 0 && ch <= 9 )			// Is it a number?
					{
						value = (value * 10) + ch;		// Continue summing the value
						bStoreRate = TRUE;
					}
					else								// We've hit a non-number ascii character
					{
						if ( bStoreRate )
						{
							if ( ndx < MAXNUM_REFRESHRATES_PERMODE )
							{
								modeRates[ i ].refreshRates[ ndx ] = value;	// Save off what we've calculated so far as the current refresh rate
								ndx++;
								value = 0;
								bStoreRate = FALSE;
								modeRates[i].bFoundInReg = TRUE;
							}
							else
								break;					// No more room in refreshRates[].  We're done.
						}

						if ( j+1 >= ansi.Length )
							break;						// We're done parsing this string of values

						// Now skip past any non-NUMERIC characters.
						// We increment j until the .Buffer[j+1] character is the next
						// NUMERIC character in the string buffer.  Then, when j is
						// incremented at the top of this loop, we'll be at the next valid
						// NUMERIC character.
						//
						ch = ansi.Buffer[ j+1 ] - '0';
						while ( (ch <= 0 || ch > 9) && (j+1 < ansi.Length) )
						{
							j++;						// Kids, don't try this at home!
							ch = ansi.Buffer[ j+1 ] - '0';
						}

					}
				} // for () -- scan through string buffer and convert to integer rate values

				// Save off the last calculated value, if there is one.
				//
				if ( bStoreRate && (value >= 0) && (ndx < MAXNUM_REFRESHRATES_PERMODE) )
				{
					modeRates[ i ].refreshRates[ ndx ] = value;
					modeRates[ i ].bFoundInReg = TRUE;
				}

				// Since the MODE_ALL_MODES overrides all other refresh rate info,
				// no need to read any other mode info from registry.
				//
				if ( i == 0 )							// Found "MODES_ALL_MODES" in the registry
					break;

			} // else Mode string present in registry and has values
		} // else got a string from the registry
	} // for () -- loop through mode table, inquire of registry for rates for the mode
}
#endif // CUST_INF_MODERATES
#endif // ifndef DMT_ENABLED


VOID
H3GetRegistrySettings(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	)
/*++

Routine Description:

	Collect the settings from the registry for the device extension

Arguments:

	HwDeviceExtension - Supplies a pointer to the miniport's device extension.

Return Value:

	NO_ERROR, or appropriate to the failed registry call

--*/
{
	VIDEO_REQUEST_PACKET Packet;
	STATUS_BLOCK StatusBlock;
	TDFX_SET_VALUE_INFO ValInfo;
	UCHAR InputBuffer[32];
	VP_STATUS status;
	ULONG i, tmp, table[256];
	UCHAR InputBufferString[] = "GlideGammaTable";

	UNREFERENCED_PARAMETER(Packet);
	UNREFERENCED_PARAMETER(StatusBlock);
	UNREFERENCED_PARAMETER(ValInfo);

	VideoDebugPrint((0, "H3GetRegistrySettings -\n"));

	tmp = 0;	// Eliminate compiler error.

#if ENABLE_UNATTENDED_INSTALL_CHECK
	VideoDebugPrint((2, "Checking for UNATTENDED_INSTALLATION in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"UNATTENDED_INSTALLATION",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for UNATTENDED_INSTALLATION failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for UNATTENDED_INSTALLATION\n", tmp));
		HwDeviceExtension->UnattendedInstall = tmp;
	}
#endif

#if DBG
	VideoDebugPrint((2, "Checking for MiniportDebugLevel in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"MiniportDebugLevel",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for MiniportDebugLevel failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for MiniportDebugLevel\n", tmp));
		HwDeviceExtension->MiniportDebugLevel = tmp;
	}
#endif
	//
	// When we have a secondary device, some things are different
	//
	if (HwDeviceExtension->IsSecondaryDevice)
	{
		VideoDebugPrint((2, "Checking for miscInit1 default in registry\n"));
		status = VideoPortGetRegistryParameters(HwDeviceExtension,
												(PWSTR)L"miscInit1",
												FALSE,
												H3RegistryCallback,
												&tmp);
		if (status != NO_ERROR)
			VideoDebugPrint((0, "Registry call for miscInit1 failed, status=0x%x\n", status));
		else
		{
			VideoDebugPrint((2, "\tgot %08lx for miscInit1\n", tmp));
			HwDeviceExtension->miscInit1 = tmp;
		}

		VideoDebugPrint((2, "Checking for dramInit0 default in registry\n"));
		status = VideoPortGetRegistryParameters(HwDeviceExtension,
												(PWSTR)L"dramInit0",
												FALSE,
												H3RegistryCallback,
												&tmp);
		if (status != NO_ERROR)
			VideoDebugPrint((0, "Registry call for dramInit0 failed, status=0x%x\n", status));
		else
		{
			VideoDebugPrint((2, "\tgot %08lx for dramInit0\n", tmp));
			HwDeviceExtension->dramInit0 = tmp;
		}

		VideoDebugPrint((2, "Checking for dramInit1 default in registry\n"));
		status = VideoPortGetRegistryParameters(HwDeviceExtension,
												(PWSTR)L"dramInit1",
												FALSE,
												H3RegistryCallback,
												&tmp);
		if (status != NO_ERROR)
			VideoDebugPrint((0, "Registry call for dramInit1 failed, status=0x%x\n", status));
		else
		{
			VideoDebugPrint((2, "\tgot %08lx for dramInit1\n", tmp));
			HwDeviceExtension->dramInit1 = tmp;
		}

		VideoDebugPrint((2, "UseNonBIOSModeSet defaulted to TRUE -- secondary device!\n"));
	}
	else
	{
		VideoDebugPrint((2, "Checking for UseNonBIOSModeSet in registry\n"));
		status = VideoPortGetRegistryParameters(HwDeviceExtension,
												(PWSTR)L"UseNonBIOSModeSet",
												FALSE,
												H3RegistryCallback,
												&tmp);
		if (status != NO_ERROR)
			VideoDebugPrint((0, "Registry call for UseNonBIOSModeSet failed, status=0x%x\n", status));
		else
		{
			VideoDebugPrint((2, "\tgot %03d for UseNonBIOSModeSet\n", tmp));
			HwDeviceExtension->UseNonBIOSModeSet = tmp;
		}
	}

#ifndef DMT_ENABLED
#ifdef CUST_INF_MODERATES
// STBNW JAC 1-8-99
	H3ReadRefreshRates ( HwDeviceExtension );
#endif // CUST_INF_MODERATES
#endif // ifndef DMT_ENABLED

	VideoDebugPrint((2, "Checking for GammaTable in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"GammaTable",
											FALSE,
											H3RetrieveGammaCallback,
											&table);
	if (status != NO_ERROR)
	{
		VideoDebugPrint((0, "Registry call for GammaTable failed, status=0x%x\n", status));
		VideoPortSetRegistryParameters(HwDeviceExtension,
									   (PWSTR)L"GammaTable",
									   HwDeviceExtension->GammaTable,
									   (NUM_CLUT_ENTRIES * sizeof(ULONG)));
	}
	else
	{
		for (i = 0; i < NUM_CLUT_ENTRIES; i++)
		{
			HwDeviceExtension->GammaTable[i] = table[i] & 0x00ffffff;
		}
	}

#if DBG
	if (HwDeviceExtension->MiniportDebugLevel & (DBG_GAMMA_STUFF | DBG_REGISTRY))
	{
		VideoDebugPrint((2,"Gamma table retrieved from registry is as follows:\n"));
		for (i=0; i<NUM_CLUT_ENTRIES; i++)
		{
			VideoDebugPrint((2,"%06lx ", HwDeviceExtension->GammaTable[i]));
			if ((i % 8) == 7)
				VideoDebugPrint((2,"\n"));
		}
	}
#endif
	VideoDebugPrint((2, "Checking for GlideGammaTable in registry\n"));

	StatusBlock.Information = (NUM_CLUT_ENTRIES * sizeof(ULONG));

	Packet.StatusBlock = (PVOID)&StatusBlock;
	Packet.InputBuffer = (PVOID)"GlideGammaTable";
	Packet.InputBufferLength = strlen(Packet.InputBuffer);
	Packet.OutputBuffer = (PVOID)&ValInfo;
	Packet.OutputBufferLength = sizeof(TDFX_SET_VALUE_INFO);

	status = H3GetRegistryValue(HwDeviceExtension,&Packet);
	memcpy(&ValInfo.Data,
		   table,
		   (NUM_CLUT_ENTRIES * sizeof(ULONG)));

	if (status != NO_ERROR)
	{
		VideoDebugPrint((0, "Registry call for GlideGammaTable failed, status=0x%x\n", status));

		ValInfo.ValueNameLength = 15;
		memcpy(InputBufferString, // NVH - Don't use a static string here. // "GlideGammaTable",
			   &ValInfo.ValueName,
			   ValInfo.ValueNameLength);
		ValInfo.Type = REG_BINARY;
		ValInfo.DataLength = (NUM_CLUT_ENTRIES * sizeof(ULONG));
		memcpy(HwDeviceExtension->GlideGammaTable,
			   &ValInfo.Data,
			   ValInfo.DataLength);

		status = H3SetRegistryValue(HwDeviceExtension,&Packet);
	}
	else
	{
		for (i = 0; i < NUM_CLUT_ENTRIES; i++)
		{
			HwDeviceExtension->GlideGammaTable[i] = table[i] & 0x00ffffff;
		}
	}

#if DBG
	if (HwDeviceExtension->MiniportDebugLevel & (DBG_GAMMA_STUFF | DBG_REGISTRY))
	{
		VideoDebugPrint((2,"Glide Gamma table retrieved from registry is as follows:\n"));
		for (i=0; i<NUM_CLUT_ENTRIES; i++)
		{
			VideoDebugPrint((2,"%06lx ", HwDeviceExtension->GlideGammaTable[i]));
			if ((i % 8) == 7)
				VideoDebugPrint((2,"\n"));
		}
	}
#endif

	VideoDebugPrint((2, "Checking for UseSoftwareCursor in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"UseSoftwareCursor",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for UseSoftwareCursor failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for UseSoftwareCursor\n", tmp));
		HwDeviceExtension->UseSoftwareCursor = tmp;
	}

	VideoDebugPrint((2, "Checking for GraphicsClocking in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"GraphicsClocking",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for GraphicsClocking failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for GraphicsClocking\n", tmp));
		HwDeviceExtension->GraphicsClocking = tmp;
	}

#if 0
#ifdef SLI_AA
	VideoDebugPrint((2, "Checking for 32BPP_RENDERING in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"32BPP_RENDERING",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for 32BPP_RENDERING failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for 32BPP_RENDERING\n", tmp));
		HwDeviceExtension->render32bpp = tmp;
	}

	VideoDebugPrint((2, "Checking for DISABLE_SLI in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"DISABLE_SLI",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for DISABLE_SLI failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for DISABLE_SLI\n", tmp));
		HwDeviceExtension->disableSli = tmp;
	}

	VideoDebugPrint((2, "Checking for ENABLE_AA in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"ENABLE_AA",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for ENABLE_AA failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for ENABLE_AA\n", tmp));
		HwDeviceExtension->AAenable = tmp;
	}

	VideoDebugPrint((2, "Checking for AA_SAMPLES in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"AA_SAMPLES",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for AA_SAMPLES failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for AA_SAMPLES\n", tmp));
		HwDeviceExtension->AAsamples = tmp;
	}

	VideoDebugPrint((2, "Checking for 2PPC in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"2PPC",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for 2PPC failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for 2PPC\n", tmp));
		HwDeviceExtension->twoPPC = tmp;
	}

	VideoDebugPrint((2, "Checking for 2PPC_LOG2_BAND_HEIGHT in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"2PPC_LOG2_BAND_HEIGHT",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for 2PPC_LOG2_BAND_HEIGHT failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for 2PPC_LOG2_BAND_HEIGHT\n", tmp));
		HwDeviceExtension->twoPPCLog2BandHeight = tmp;
	}

	VideoDebugPrint((2, "Checking for GUARDBAND_CLIPPING in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"GUARDBAND_CLIPPING",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for GUARDBAND_CLIPPING failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for GUARDBAND_CLIPPING\n", tmp));
		HwDeviceExtension->guardbandClip = tmp;
	}

	VideoDebugPrint((2, "Checking for CBC in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"CBC",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for CBC failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for CBC\n", tmp));
		HwDeviceExtension->cbc = tmp;
	}

	VideoDebugPrint((2, "Checking for DIGITAL_SLI_AA in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"DIGITAL_SLI_AA",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for DIGITAL_SLI_AA failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for DIGITAL_SLI_AA\n", tmp));
		HwDeviceExtension->digitalSliAAEnable = tmp;
	}

	VideoDebugPrint((2, "Checking for SLI_BAND_HEIGHT in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"SLI_BAND_HEIGHT",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for SLI_BAND_HEIGHT failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for SLI_BAND_HEIGHT\n", tmp));
		HwDeviceExtension->sliBandHeight = tmp;
	}

	VideoDebugPrint((2, "Checking for SWAPBUFFER_ALGO in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"SWAPBUFFER_ALGO",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for SWAPBUFFER_ALGO failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for SWAPBUFFER_ALGO\n", tmp));
		HwDeviceExtension->swapAlgorithmType = tmp;
	}
#endif
#endif

#if (_WIN32_WINNT >= 0x500) && defined (AGP_FIFO_CODE)
	VideoDebugPrint((2, "Checking for AGP command fifo in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"AFifo",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
		VideoDebugPrint((0, "Registry call for AGP command fifo failed, status=0x%x\n", status));
	else
	{
		VideoDebugPrint((2, "\tgot %03d for AFifo\n", tmp));
		HwDeviceExtension->AFifo = tmp;
	}
#endif

	VideoDebugPrint((2, "Checking for CustNum in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"CustNum",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
	{
		VideoDebugPrint((0, "Registry call for CustNum failed, status=0x%x\n", status));
		HwDeviceExtension->CustNum = 0;
	}
	else
	{
		VideoDebugPrint((2, "\tgot %03d for CustNum\n", tmp));
		HwDeviceExtension->CustNum = tmp;
	}

	VideoDebugPrint((2, "Checking for AllowPALCRT in registry\n"));
	status = VideoPortGetRegistryParameters(HwDeviceExtension,
											(PWSTR)L"AllowPALCRT",
											FALSE,
											H3RegistryCallback,
											&tmp);
	if (status != NO_ERROR)
	{
		HwDeviceExtension->bAllowPALCRT = FALSE;
	}
	else
	{
		VideoDebugPrint((2, "\tFound AllowPALCRT\n"));
		HwDeviceExtension->bAllowPALCRT = TRUE;
	}
}


/***************************************************************************
*
* FUNCTION: 	  H3CopyDriverRegistryPathToDeviceExtension
*
* DESCRIPTION:  
*
****************************************************************************/

VOID
H3CopyDriverRegistryPathToDeviceExtension(PHW_DEVICE_EXTENSION  hwDeviceExtension,
                                          PWSTR                 pDriverRegistryPath)
{
  VP_STATUS       status;
  UNICODE_STRING  ucDriverRegistryPath;
#if DBG
  ANSI_STRING     ansiDriverRegistryPath;
#endif

  hwDeviceExtension->pDriverRegistryPath = NULL;

  RtlInitUnicodeString(&ucDriverRegistryPath, pDriverRegistryPath);

#if DBG
  RtlUnicodeStringToAnsiString(&ansiDriverRegistryPath, &ucDriverRegistryPath, TRUE);

  VideoDebugPrint((0, "DriverRegistryPath = %s\n", ansiDriverRegistryPath.Buffer));

  RtlFreeAnsiString(&ansiDriverRegistryPath);
#endif

  status = VideoPortAllocateBuffer(hwDeviceExtension,
                                   ucDriverRegistryPath.Length + sizeof(WCHAR),
                                   &hwDeviceExtension->pDriverRegistryPath);
#ifndef MS_VIEW
  
  if (! NT_SUCCESS(status))
  {
    // if VideoPortAllocateBuffer failed, then we'll try ExAllocatePool
    hwDeviceExtension->pDriverRegistryPath = ExAllocatePool(PagedPool,
                                                            ucDriverRegistryPath.Length + sizeof(WCHAR));
    // if ExAllocatePool fails then hwDeviceExtension->pDriverRegistyPath will be NULL
    // and we'll just have to fail all reads from the registry
  }
  
#endif
  if (NULL != hwDeviceExtension->pDriverRegistryPath)
  {
    memcpy(hwDeviceExtension->pDriverRegistryPath,
           pDriverRegistryPath,
           ucDriverRegistryPath.Length + sizeof(WCHAR));
  }
#if DBG
  else
  {
    VideoDebugPrint((0, "Failed to allocate memory for DriverRegistryPath!\n"));
  }
#endif
}

#ifdef MS_VIEW
#define ALLOW_W2K_ILLEGAL_IMPORTS   0
#else
#define ALLOW_W2K_ILLEGAL_IMPORTS	1
#endif

#if ALLOW_W2K_ILLEGAL_IMPORTS
VP_STATUS
H3ZwQueryRegistryValue(PHW_DEVICE_EXTENSION HwDeviceExtension,
                       PVIDEO_REQUEST_PACKET pRequestPacket);
VP_STATUS
H3ZwSetRegistryValue(PHW_DEVICE_EXTENSION HwDeviceExtension,
                     PVIDEO_REQUEST_PACKET pRequestPacket);
VP_STATUS
CheckForOpenGLICD(PWSTR);

#pragma alloc_text(PAGE,H3ZwQueryRegistryValue)
#pragma alloc_text(PAGE,H3ZwSetRegistryValue)
#pragma alloc_text(PAGE,CheckForOpenGLICD)

/***************************************************************************
*
* FUNCTION: 	H3ZwQueryRegistryValue
*
* DESCRIPTION:
*
****************************************************************************/

VP_STATUS
H3ZwQueryRegistryValue(PHW_DEVICE_EXTENSION hwDeviceExtension,
                       PVIDEO_REQUEST_PACKET pRequestPacket)
{
  typedef struct _H3_SEARCH_TYPE
  {
    WCHAR *pRoot;
    WCHAR *pSubkey;
  } H3_SEARCH_TYPE;

  // at this point, I haven't been able to get access to HKEY_CURRENT_USER
  // from the miniport but this is the search order that the 3dfx tools property
  // sheet expects us to use

  static const H3_SEARCH_TYPE H3SearchOrder[] =
  {
    { (PWCHAR)L"\\Registry\\Machine\\",           (PWCHAR)L"\\D3D"   },
    { (PWCHAR)L"\\Registry\\Machine\\",           (PWCHAR)NULL       },
  };

  static const WCHAR ICDDevNodeKey[]  = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\OpenGLDrivers";

  const H3_SEARCH_TYPE          *pSearchLoc;
  VP_STATUS                     status;
  HANDLE                        hkey;
  OBJECT_ATTRIBUTES             keyAttr;
  UNICODE_STRING                ucKeyName;
  WCHAR                         KeyName[256];
  PWSTR                         pswzKey;
  int                           i;
  ANSI_STRING                   ansiValueName;
  UNICODE_STRING                ucValueName;
  UNICODE_STRING                ucValueData;
  ANSI_STRING                   ansiValueData;
  unsigned char                 buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+1024];
  KEY_VALUE_PARTIAL_INFORMATION *pValueInfo;
  ULONG                         retLength;
  BOOLEAN                       bICDCheck;
  WCHAR                         *pRoot;


	VideoDebugPrint((2, "H3ZwQueryRegistryValue - %s\n", pRequestPacket->InputBuffer));

  // if it's 3Dfx, grab the OpenGL name from the ICDDevNodeKey
  if (0 == memcmp(pRequestPacket->InputBuffer, "3Dfx\0", 5))
  {
    VideoDebugPrint((2, "  using ICD node\n"));
    bICDCheck = TRUE;
    pswzKey   = (PWCHAR)ICDDevNodeKey;

    // if the search order gets rearranged, the following line must change
    pSearchLoc = &H3SearchOrder[sizeof(H3SearchOrder)/sizeof(H3SearchOrder[0]) - 1];
    pRoot = pSearchLoc->pRoot;
  }
  // If it's an FX, Gl or SSTH3_ prefixed parameter, use the tdfx
  // dev node to collect the information
  else if (0 == memcmp(pRequestPacket->InputBuffer, "SSTH3_", 6))
  {
    VideoDebugPrint((2, "using 3dfx node\n"));
    bICDCheck = FALSE;
    pswzKey   = hwDeviceExtension->pDriverRegistryPath,
    pSearchLoc = &H3SearchOrder[0];
    pRoot = NULL;
  }
  // fail anything else
  else
  {
    return ERROR_DEV_NOT_EXIST;
  }

  // loop over possible registry locations
  for (/* pSearchLoc should have been initialized above */ ;
       pSearchLoc < &H3SearchOrder[sizeof(H3SearchOrder)/sizeof(H3SearchOrder[0])];
       pSearchLoc++)
  {
    // initialize unicode key name
    ucKeyName.MaximumLength = sizeof(KeyName);
    ucKeyName.Length = 0;
    ucKeyName.Buffer = KeyName;

    if (NULL != pRoot)
      RtlAppendUnicodeToString(&ucKeyName, pRoot);
    RtlAppendUnicodeToString(&ucKeyName, (PWSTR) pswzKey);

    // if we have a non NULL subkey
    // tack the subkey to the end of the DevNodeKey
    if (NULL != pSearchLoc->pSubkey)
      RtlAppendUnicodeToString(&ucKeyName, pSearchLoc->pSubkey);

    // open the key
    InitializeObjectAttributes(&keyAttr,
                               &ucKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = ZwOpenKey(&hkey, KEY_READ , &keyAttr);
    if (NT_SUCCESS(status))
    {
      // initialize unicode value name
      RtlInitAnsiString(&ansiValueName, (PCHAR)pRequestPacket->InputBuffer);
      RtlAnsiStringToUnicodeString(&ucValueName, &ansiValueName, TRUE);

      // query for ValueName's data
      pValueInfo = (KEY_VALUE_PARTIAL_INFORMATION *)&buffer[0];
      status = ZwQueryValueKey(hkey,
                               &ucValueName,
                               KeyValuePartialInformation,
                               pValueInfo,
                               sizeof(buffer),
                               &retLength);
      if (NT_SUCCESS(status))
      {
        TDFX_QUERY_VALUE_INFO *pTDFXValueInfo = pRequestPacket->OutputBuffer;

        // copy data into output buffer
        switch (pValueInfo->Type)
        {
          // data is a unicode nul terminated string
          case REG_SZ:
            if (bICDCheck)
            {
              status = CheckForOpenGLICD((PWSTR)pValueInfo->Data);
              if (! NT_SUCCESS(status))
              {
                VideoDebugPrint((0,"  ICD file not installed!\n"));
                status = ERROR_DEV_NOT_EXIST;
                break;
              }
              VideoDebugPrint((2,"  ICD file installed!\n"));
            }

            // convert unicode data string to ansi data string
            RtlInitUnicodeString(&ucValueData, (PWSTR)pValueInfo->Data);
            RtlUnicodeStringToAnsiString(&ansiValueData, &ucValueData, TRUE);

            // if there's enough room, copy ansi data string to output buffer
            if ((ULONG)(ansiValueData.Length+1) <= pRequestPacket->OutputBufferLength)
            {
              memcpy(pTDFXValueInfo->Data,
                     ansiValueData.Buffer,
                     ansiValueData.Length);
              pTDFXValueInfo->Data[ansiValueData.Length] = '\0';

              // fill in type and DataLength in output buffer
              pTDFXValueInfo->DataLength = ansiValueData.Length + 1;
              pTDFXValueInfo->Type = pValueInfo->Type;

              pRequestPacket->StatusBlock->Information = sizeof(TDFX_QUERY_VALUE_INFO);
            }
            else
            {
              VideoDebugPrint((0, "  insufficient buffer size %ld\n", pTDFXValueInfo->DataLength));
              status = ERROR_INSUFFICIENT_BUFFER;
            }

            RtlFreeAnsiString(&ansiValueData);
            break;

          // data is a dword
          case REG_DWORD:
          // data is binary
          case REG_BINARY:
            // if there's enough room, copy data to output buffer
            if (pValueInfo->DataLength <= pTDFXValueInfo->DataLength)
            {
              memcpy(pTDFXValueInfo->Data,
                     pValueInfo->Data,
                     pValueInfo->DataLength);

              // fill in type and DataLength in output buffer
              pTDFXValueInfo->DataLength = pValueInfo->DataLength;
              pTDFXValueInfo->Type = pValueInfo->Type;

              pRequestPacket->StatusBlock->Information = sizeof(TDFX_QUERY_VALUE_INFO);
            }
            else
            {
              status = ERROR_INSUFFICIENT_BUFFER;
            }
            break;

          // don't understand any other data formats
          default:
            status = ERROR_DEV_NOT_EXIST;
            break;
        }

        RtlFreeUnicodeString(&ucValueName);
        ZwClose(hkey);
        return status;
      }
      else
      {
      	VideoDebugPrint((2, "ZwQueryValueKey Failed, status = 0x%x\n", status));
      }

      RtlFreeUnicodeString(&ucValueName);
      ZwClose(hkey);
    }
    else
    {
    	VideoDebugPrint((2, "ZwOpenKey Failed, status = 0x%x\n", status));
    }
  }

  return ERROR_DEV_NOT_EXIST;
}

/***************************************************************************
*
* FUNCTION: 	H3ZwSetRegistryValue
*
* DESCRIPTION:
*
****************************************************************************/

VP_STATUS
H3ZwSetRegistryValue(PHW_DEVICE_EXTENSION hwDeviceExtension,
                     PVIDEO_REQUEST_PACKET pRequestPacket)
{
  VP_STATUS           status;
  HANDLE              hkey;
  OBJECT_ATTRIBUTES   keyAttr;
  UNICODE_STRING      ucKeyName;
  TDFX_SET_VALUE_INFO *pTDFXValueInfo = pRequestPacket->InputBuffer;
  int                 i;


	VideoDebugPrint((2, "H3ZwSetRegistryValue - %s\n", pTDFXValueInfo->ValueName));

  // If it's not an FX or Gl prefixed parameter, fail the request
  if (! (0 == memcmp(pRequestPacket->InputBuffer, "SSTH3_", 6)))
  {
    return ERROR_DEV_NOT_EXIST;
  }

  // open the key
  RtlInitUnicodeString(&ucKeyName, hwDeviceExtension->pDriverRegistryPath);
  InitializeObjectAttributes(&keyAttr,
                             &ucKeyName,
                             OBJ_CASE_INSENSITIVE,
                             NULL,
                             NULL);
  status = ZwOpenKey(&hkey, KEY_ALL_ACCESS, &keyAttr);
  if (NT_SUCCESS(status))
  {
    ANSI_STRING     ansiValueName;
    UNICODE_STRING  ucValueName;
    UNICODE_STRING  ucValueData;
    ANSI_STRING     ansiValueData;

    // convert value name to a unicode string
    RtlInitAnsiString(&ansiValueName, (PCHAR)pTDFXValueInfo->ValueName);
    RtlAnsiStringToUnicodeString(&ucValueName, &ansiValueName, TRUE);

    // initialize data to write
    switch (pTDFXValueInfo->Type)
    {
      case REG_SZ:
        // convert data to a unicode string
        RtlInitAnsiString(&ansiValueData, (PCHAR)pTDFXValueInfo->Data);
        RtlAnsiStringToUnicodeString(&ucValueData, &ansiValueData, TRUE);
        // write data to registry
        status = ZwSetValueKey(hkey,
                               &ucValueName,
                               0,
                               pTDFXValueInfo->Type,
                               ucValueData.Buffer,
                               ucValueData.Length+2);
        RtlFreeUnicodeString(&ucValueData);
        break;

      case REG_DWORD:
      case REG_BINARY:
        // write data to registry
        status = ZwSetValueKey(hkey,
                               &ucValueName,
                               0,
                               pTDFXValueInfo->Type,
                               pTDFXValueInfo->Data,
                               pTDFXValueInfo->DataLength);
        break;
    }

    RtlFreeUnicodeString(&ucValueName);
    ZwClose(hkey);
    pRequestPacket->StatusBlock->Information = 0;
  }

  if (!NT_SUCCESS(status))
    status = ERROR_DEV_NOT_EXIST;

  return status;
}

/***************************************************************************
*
* FUNCTION:     CheckForOpenGLICD()
*
* DESCRIPTION:	This attempts to open the file "3dfxogl.dll". We should
*				change this to use the string value collected from the
*				registry, but for now this'll have to do.
*
****************************************************************************/

VP_STATUS
CheckForOpenGLICD(PWSTR pFileName)
{
  static const WCHAR  PathName[] = L"\\SystemRoot\\System32\\";

  NTSTATUS          ntStatus;
  HANDLE            MHandle;
  IO_STATUS_BLOCK   IoStatusBlk;
  OBJECT_ATTRIBUTES ObjAttributes;
  UNICODE_STRING		UniFileName;
  WCHAR             FileName[64];
  ULONG             dummy = 0;
  LARGE_INTEGER     Offset = {0};


  // initialize unicode filename
  UniFileName.MaximumLength = sizeof(FileName);
  UniFileName.Length = 0;
  UniFileName.Buffer = FileName;
  RtlAppendUnicodeToString(&UniFileName, (PWSTR)PathName);
  RtlAppendUnicodeToString(&UniFileName, pFileName);

  InitializeObjectAttributes(&ObjAttributes,
                             &UniFileName,
                             OBJ_CASE_INSENSITIVE,
                             NULL,
                             NULL);
					
  ntStatus = ZwCreateFile(&MHandle,
                          GENERIC_READ,
                          &ObjAttributes,
                          &IoStatusBlk,
                          0,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ,
                          FILE_OPEN,				// fail if not present
                          FILE_NON_DIRECTORY_FILE,
                          NULL,
                          0);

  VideoDebugPrint((2,"IoStatusBlk.Status      = 0x%08lx\n", (ULONG) IoStatusBlk.Status));
  VideoDebugPrint((2,"IoStatusBlk.Information = 0x%08lx\n", IoStatusBlk.Information));

  if (NT_SUCCESS(ntStatus))
    ZwClose(MHandle);

  return ntStatus;
}
#endif

typedef struct _H3REGCBDATA
{
  ULONG   ulLength;
#ifndef MS_VIEW
  UCHAR   *pBuffer;
#else
  union 
  {
    UCHAR   *pBuffer;
    WCHAR   *pUCBuffer;
  };  
#endif
} H3REGCBDATA;

/***************************************************************************
*
* FUNCTION:     H3QueryRegistryValueCallback
*
* DESCRIPTION:
*
****************************************************************************/

VP_STATUS H3QueryRegistryValueCallback(PHW_DEVICE_EXTENSION,PVOID,PWSTR,PVOID,ULONG);
#pragma alloc_text(PAGE,H3QueryRegistryValueCallback)

VP_STATUS
H3QueryRegistryValueCallback ( PHW_DEVICE_EXTENSION hwDeviceExtension,
                               PVOID Context,
                               PWSTR ValueName,
                               PVOID ValueData,
                               ULONG ValueLength )
{
  if (Context)
  {
    H3REGCBDATA *pH3RegData = (H3REGCBDATA *)Context;


    // make sure the buffer is large enough to hold the data
    if (ValueLength > pH3RegData->ulLength)
      return STATUS_UNSUCCESSFUL;

    pH3RegData->ulLength = ValueLength;
    memcpy(pH3RegData->pBuffer, ValueData, ValueLength);
  }
  else
    return STATUS_UNSUCCESSFUL;

  return NO_ERROR;
}

/***************************************************************************
*
* FUNCTION:     SSTH3QueryRegistryValueCallback
*
* DESCRIPTION:
*
****************************************************************************/

VP_STATUS SSTH3QueryRegistryValueCallback(PHW_DEVICE_EXTENSION,PVOID,PWSTR,PVOID,ULONG);
#pragma alloc_text(PAGE,SSTH3QueryRegistryValueCallback)

VP_STATUS
SSTH3QueryRegistryValueCallback ( PHW_DEVICE_EXTENSION hwDeviceExtension,
                                  PVOID Context,
                                  PWSTR ValueName,
                                  PVOID ValueData,
                                  ULONG ValueLength )
{
  if (Context)
  {
    H3REGCBDATA *pH3RegData = (H3REGCBDATA *)Context;


    // make sure the buffer is large enough to hold the data
    if ((ValueLength / 2) > pH3RegData->ulLength)
      return STATUS_UNSUCCESSFUL;

    // convert unicode string to ansi string
    {
      PWSTR   pucStr;
      PCHAR   pansiStr;

      pucStr = (PWSTR)ValueData;
      pansiStr = pH3RegData->pBuffer;

      while (*pucStr)
        *pansiStr++ = (*pucStr++ & 0xFF);
      *pansiStr = '\0';
      pH3RegData->ulLength = strlen(pH3RegData->pBuffer) + 1;
    }
  }
  else
    return STATUS_UNSUCCESSFUL;

  return NO_ERROR;
}

/***************************************************************************
*
* FUNCTION: 	H3QueryRegistryValue
*
* DESCRIPTION:
*
****************************************************************************/

#define MY_DBG_LVL        2

VP_STATUS
H3QueryRegistryValue ( PHW_DEVICE_EXTENSION hwDeviceExtension,
                       PVIDEO_REQUEST_PACKET pRequestPacket )
{
  VP_STATUS       status;
  CHAR            *pAnsiValueName;
  WCHAR           ucValueName[256];
  UCHAR           ValueData[256*sizeof(ULONG)];
  H3REGCBDATA     H3RegData;
  int             i;


  VideoDebugPrint((MY_DBG_LVL, "H3QueryRegistryValue - %s\n", pRequestPacket->InputBuffer));

#if ALLOW_W2K_ILLEGAL_IMPORTS
  // if it's an OpenGL, Glide or SSTH3_ name use Zw functions to access registry
  if ((15 <= pRequestPacket->InputBufferLength) &&
      (0 == memcmp(pRequestPacket->InputBuffer, "GlideGammaTable", 15)))
  {
    // fall thru to default driver location for "GlideGammaTable"
  }
  else if ((0 == memcmp(pRequestPacket->InputBuffer, "3Dfx\0", 5)) ||
           (0 == memcmp(pRequestPacket->InputBuffer, "SSTH3_", 6)))
  {
    status = H3ZwQueryRegistryValue(hwDeviceExtension, pRequestPacket);
    if (NO_ERROR == status)
      return status;
    // otherwise fall thru and try the default driver location
  }
#endif

  H3RegData.ulLength = sizeof(ValueData);
  H3RegData.pBuffer  = ValueData;

  // initialize unicode value name
  pAnsiValueName = (PCHAR)pRequestPacket->InputBuffer;
  i = 0;
  while ((*pAnsiValueName) && (i < 255))
    ucValueName[i++] = (0 << 16) | (*pAnsiValueName++ & 0xFF);
  ucValueName[i] = 0;

  if (0 == memcmp(pRequestPacket->InputBuffer,"SSTH3_", 6))
  {
    // query for ValueName's data
    status = VideoPortGetRegistryParameters(hwDeviceExtension,
                                            ucValueName,
                                            (UCHAR)FALSE,
                                            SSTH3QueryRegistryValueCallback,
                                            (PVOID)&H3RegData);
    if (NO_ERROR == status)
    {
      TDFX_QUERY_VALUE_INFO *pTDFXValueInfo = pRequestPacket->OutputBuffer;


      // if there's enough room, copy data to output buffer
      if (H3RegData.ulLength <= pTDFXValueInfo->DataLength)
      {
        memcpy(pTDFXValueInfo->Data, H3RegData.pBuffer, H3RegData.ulLength);

        // fill in type and DataLength in output buffer
        pTDFXValueInfo->DataLength = H3RegData.ulLength;
        pTDFXValueInfo->Type = REG_SZ;

        VideoDebugPrint((MY_DBG_LVL, "  returned value - %s\n", pTDFXValueInfo->Data));

        pRequestPacket->StatusBlock->Information = sizeof(TDFX_QUERY_VALUE_INFO);
      }
      else
      {
        status = ERROR_INSUFFICIENT_BUFFER;
      }
    }
    else
    {
      VideoDebugPrint((MY_DBG_LVL, "  VideoPortGetRegistryParameters failed - %lXh\n", status));
    }
  }
  else
  {
    // query for ValueName's data
    status = VideoPortGetRegistryParameters(hwDeviceExtension,
                                            ucValueName,
                                            //ucValueName.Buffer,
                                            (UCHAR)FALSE,
                                            H3QueryRegistryValueCallback,
                                            (PVOID)&H3RegData);
    if (NO_ERROR == status)
    {
      TDFX_QUERY_VALUE_INFO *pTDFXValueInfo = pRequestPacket->OutputBuffer;


      // if there's enough room, copy data to output buffer
      if (H3RegData.ulLength <= pTDFXValueInfo->DataLength)
      {
        memcpy(pTDFXValueInfo->Data,H3RegData.pBuffer,H3RegData.ulLength);

        // fill in type and DataLength in output buffer
        pTDFXValueInfo->DataLength = H3RegData.ulLength;
        if (sizeof(ULONG) == H3RegData.ulLength)
          pTDFXValueInfo->Type = REG_DWORD;
        else
          pTDFXValueInfo->Type = REG_BINARY;

        pRequestPacket->StatusBlock->Information = sizeof(TDFX_QUERY_VALUE_INFO);
      }
      else
      {
        status = ERROR_INSUFFICIENT_BUFFER;
      }
    }
    else
    {
      VideoDebugPrint((MY_DBG_LVL, "  VideoPortGetRegistryParameters failed - %lXh\n", status));
    }
  }

  return status;
}

/***************************************************************************
*
* FUNCTION: 	H3GetRegistryValue
*
* DESCRIPTION:
*
****************************************************************************/

VP_STATUS
H3GetRegistryValue(PHW_DEVICE_EXTENSION hwDeviceExtension,
                   PVIDEO_REQUEST_PACKET pRequestPacket)
{
  VideoDebugPrint((MY_DBG_LVL, "H3GetRegistryValue - %s\n", pRequestPacket->InputBuffer));
  return H3QueryRegistryValue(hwDeviceExtension,pRequestPacket);
}

/***************************************************************************
*
* FUNCTION: 	H3SetRegistryValue
*
* DESCRIPTION:
*
****************************************************************************/

VP_STATUS
H3SetRegistryValue(PHW_DEVICE_EXTENSION hwDeviceExtension,
                   PVIDEO_REQUEST_PACKET pRequestPacket)
{
  VP_STATUS       status;
	TDFX_SET_VALUE_INFO *pTDFXValueInfo = pRequestPacket->InputBuffer;
  CHAR            *pAnsiValueName;
  WCHAR           ucValueName[256];
  WCHAR           ucValueData[256];
  int             i;


  VideoDebugPrint((MY_DBG_LVL, "H3SetRegistryValue - %s\n", pTDFXValueInfo->ValueName));

#if ALLOW_W2K_ILLEGAL_IMPORTS
  // if it's an OpenGL, Glide or SSTH3_ name use Zw functions to access registry
  if ((15 <= pRequestPacket->InputBufferLength) &&
      (0 == memcmp(pRequestPacket->InputBuffer, "GlideGammaTable", 15)))
  {
    // fall thru to default driver location for "GlideGammaTable"
  }
  else if ((0 == memcmp(pTDFXValueInfo->ValueName, "3Dfx\0", 5)) ||
           (0 == memcmp(pTDFXValueInfo->ValueName, "SSTH3_", 6)))
  {
    status = H3ZwSetRegistryValue(hwDeviceExtension, pRequestPacket);
    if (NO_ERROR == status)
      return status;
    // otherwise fall thru and try the default driver location
  }
#endif

  // convert value name to a unicode string
  pAnsiValueName = pTDFXValueInfo->ValueName;
  i = 0;
  while ((*pAnsiValueName) && (i < 255))
    ucValueName[i++] = (0 << 16) | (*pAnsiValueName++ & 0xFF);
  ucValueName[i] = 0;

  // initialize data to write
  switch (pTDFXValueInfo->Type)
  {
    case REG_SZ:
      // convert ansi to unicode
      pAnsiValueName = pTDFXValueInfo->Data;
      i = 0;
      while ((*pAnsiValueName) && (i < 255))
        ucValueData[i++] = (0 << 16) | (*pAnsiValueName++ & 0xFF);
      ucValueData[i] = 0;
      // write unicode string to registry
      status = VideoPortSetRegistryParameters(hwDeviceExtension,
                                              ucValueName,
                                              ucValueData,
                                              2*(strlen(pTDFXValueInfo->Data)+1));
      break;

    case REG_DWORD:
    case REG_BINARY:
      // write data to registry
      status = VideoPortSetRegistryParameters(hwDeviceExtension,
                                              ucValueName,
                                              pTDFXValueInfo->Data,
                                              pTDFXValueInfo->DataLength);
      break;
  }

  pRequestPacket->StatusBlock->Information = 0;

  return status;
}

#if ENABLE_LOG_FILE

#pragma alloc_text(PAGE,H3WriteLogFile)

/***************************************************************************
*
* FUNCTION:     H3WriteLogFile()
*
* DESCRIPTION:
*
****************************************************************************/

VP_STATUS
H3WriteLogFile ( PVOID pBuffer, ULONG BytesToWrite )
{
  /* retro3dfx: original path was \DosDevices\C:\3dfxvs.log, but on the .124
     dual-boot box C: is the Win98 FAT volume and the active NT system volume is
     D: — the kernel write to C: silently failed. Try D: first (active system
     volume), then C: as a fallback. */
  static const WCHAR  FileNameD[] = L"\\DosDevices\\D:\\3dfxvs.log";
  static const WCHAR  FileNameC[] = L"\\DosDevices\\C:\\3dfxvs.log";
  const WCHAR        *FileName = FileNameD;
  int                 retroTry;
  HANDLE              hFile;
  VP_STATUS           status;
  UNICODE_STRING      ucFileName;
  OBJECT_ATTRIBUTES   fileAttr;
  IO_STATUS_BLOCK     fileIoStatus;
  LARGE_INTEGER       maxFileSize;


  // start with a 20MB file if it doesn't already exist
  maxFileSize.QuadPart = 0x1400000;

  /* retro3dfx: try D: (active NT system volume) then C:. File I/O here is only
     valid at PASSIVE_LEVEL; the IOCTL path runs there. */
  status = ERROR_DEV_NOT_EXIST;
  for (retroTry = 0; retroTry < 2; retroTry++)
  {
    FileName = (0 == retroTry) ? FileNameD : FileNameC;
    RtlInitUnicodeString(&ucFileName, FileName);

    InitializeObjectAttributes(&fileAttr,
                               &ucFileName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    // Open the file
    status = ZwCreateFile(&hFile,
                          SYNCHRONIZE | FILE_APPEND_DATA,
                          &fileAttr,
                          &fileIoStatus,
                          &maxFileSize,
                          FILE_ATTRIBUTE_NORMAL,
                          0,
                          FILE_OPEN_IF,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    if (NT_SUCCESS(status))
    {
      // Write to the file
      status = ZwWriteFile(hFile,
                           NULL,
                           NULL,
                           NULL,
                           &fileIoStatus,
                           pBuffer,
                           BytesToWrite,
                           NULL,
                           NULL);

      ZwClose(hFile);
      break;    // wrote successfully (or write failed but file opened) — done
    }
  }

	if (! NT_SUCCESS(status))
    status = ERROR_DEV_NOT_EXIST;

  return status;
}
#endif // ENABLE_LOG_FILE


#ifdef DMT_ENABLED

#define DISABLE_HARDCODED_REGPATH   1

//=======================================================================================
// Function name: di_RegOpenKey 
//
// Description:   Open a registry key and return the HKEY handle.
//				
// Information:    
//
// Return:         LONG - 0 if successful, otherwise failure code  
//
//=======================================================================================
LONG di_RegOpenKey( ULONG mainkey, CHAR * keyname, HKEY *hkeyptr )
{
VP_STATUS           status;
OBJECT_ATTRIBUTES   keyAttr;
UNICODE_STRING      ucKeyName;
ANSI_STRING     ansiKeyName;
CHAR keypath[512];	


#ifdef DISABLE_HARDCODED_REGPATH
	strcpy( keypath, keyname );
#else
	switch ( mainkey )
	{
		case HKEY_LOCAL_MACHINE:
			strcpy( keypath, "\\Registry\\Machine\\" );
			break;
		case HKEY_CLASSES_ROOT:		// TODO fill in appropriate keypath value
			break;
		case HKEY_CURRENT_USER:
			strcpy( keypath, "\\Registry\\User\\CurrentUser\\" );
			break;
		case HKEY_USERS:			// TODO fill in appropriate keypath value
			break;
		case HKEY_CURRENT_CONFIG:	// TODO fill in appropriate keypath value
			break;
	}

	strcat( keypath, keyname );
#endif

	*hkeyptr = NULL;

	 
  	RtlInitAnsiString(&ansiKeyName, keypath);
  	RtlAnsiStringToUnicodeString(&ucKeyName, &ansiKeyName, TRUE);

  	// open the key
  	InitializeObjectAttributes(&keyAttr, &ucKeyName, OBJ_CASE_INSENSITIVE, 0, 0 );
  	status = ZwOpenKey( hkeyptr, KEY_ALL_ACCESS, &keyAttr );

    // convert value name to a unicode string
  	RtlFreeUnicodeString(&ucKeyName);

  	return( (LONG)status );
}

//=======================================================================================
// Function name: di_RegCloseKey 
//
// Description:   Close a registry key. 
//				
// Information:    
//
// Return:        LONG - 0 if successful, otherwise failure code  
//
//=======================================================================================
LONG di_RegCloseKey( HKEY hkey )
{
VP_STATUS status;


  	status = ZwClose( hkey );

  	return( (LONG)status );
}

//=======================================================================================
// Function name: di_RegQueryValueEx 
//
// Description:   Reads a registry value using a previously opened key... See 
//				  "RegQueryValueEx" in the MSDN
// Information:    
//
// Return:        LONG - 0 if successful, otherwise failure code  
//
//=======================================================================================
LONG di_RegQueryValueEx( HKEY hkey, CHAR * pValName, ULONG * reserved, ULONG * pType, CHAR * pData, ULONG * retLength )
{
VP_STATUS status;
UNICODE_STRING ucValueName;
ANSI_STRING ansiValueName;
UNICODE_STRING ucValueData;
ANSI_STRING ansiValueData;
UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+512];
KEY_VALUE_PARTIAL_INFORMATION *pValueInfo;
ULONG maxLength = *retLength;

    // initialize unicode value name
    RtlInitAnsiString(&ansiValueName, pValName);
    RtlAnsiStringToUnicodeString(&ucValueName, &ansiValueName, TRUE);

    // query for ValueName's data
    pValueInfo = (KEY_VALUE_PARTIAL_INFORMATION *)&buffer[0];
    status = ZwQueryValueKey(hkey,
                               &ucValueName,
                               KeyValuePartialInformation,
                               pValueInfo,
                               sizeof(buffer),
                               retLength);

    RtlFreeUnicodeString(&ucValueName);

	if (NT_SUCCESS(status))
    {
      	*pType = pValueInfo->Type;

	  	if ( pData )
		{
			switch ( *pType )
			{
				case REG_SZ:
				case REG_EXPAND_SZ:
				case REG_MULTI_SZ:
			    	// convert unicode data string to ansi data string
           			RtlInitUnicodeString(&ucValueData, (PWSTR)pValueInfo->Data);
           			RtlUnicodeStringToAnsiString(&ansiValueData, &ucValueData, TRUE);
					*retLength = ansiValueData.MaximumLength;
					if ( *retLength <= maxLength )
					{
						// copy data into output buffer
						memcpy(pData, ansiValueData.Buffer, *retLength);
           				pData[ansiValueData.Length] = '\0';
					}
					else
						status = ERROR_MORE_DATA;
		   			RtlFreeAnsiString(&ansiValueData);
					break;

				case REG_DWORD:
				case REG_BINARY:
					*retLength -= 12;	// NT adds on 12 to all BINARY and DWORD fields
					if ( *retLength <= maxLength )
	  					memcpy( pData, pValueInfo->Data, *retLength );
					else
						status = ERROR_MORE_DATA;
					break;
			}		
		}
	}

  	return( (LONG)status );
}

//=======================================================================================
// Function name: di_RegSetValueEx 
//
// Description:   Writes a registry value using a previously opened key... See 
//				  "RegSetValueEx" in the MSDN
//				
// Information:    
//
// Return:        LONG - 0 if successful, otherwise failure code  
//
//=======================================================================================
LONG di_RegSetValueEx( HKEY hkey, CHAR * pValName, ULONG reserved, ULONG Type, CHAR * pData, ULONG Length )
{
VP_STATUS status;
UNICODE_STRING ucValueName;
ANSI_STRING ansiValueName;
UNICODE_STRING ucValueData;
ANSI_STRING ansiValueData;


    // initialize unicode value name
    RtlInitAnsiString(&ansiValueName, pValName);
    RtlAnsiStringToUnicodeString(&ucValueName, &ansiValueName, TRUE);

	switch ( Type )
	{
		case REG_SZ:
		case REG_EXPAND_SZ:
		case REG_MULTI_SZ:
		    // initialize unicode value data
    		RtlInitAnsiString(&ansiValueData, pData);
    		RtlAnsiStringToUnicodeString(&ucValueData, &ansiValueData, TRUE);

    		status = ZwSetValueKey(hkey, &ucValueName, 0, Type,	ucValueData.Buffer, ucValueData.Length+2);  //add two to the length to include the zero terminator

			RtlFreeUnicodeString(&ucValueData);
			break;

		default:
		    // set ValueName's data
    		status = ZwSetValueKey(hkey, &ucValueName, 0, Type,	pData, Length);
	  		break;

	}

	RtlFreeUnicodeString(&ucValueName);

  	return( (LONG)status );
}

//=======================================================================================
// Function name: di_RegEnumKey
//
// Description:    
//				
// Information:    
//
// Return:         LONG - 0 if successful, otherwise failure code  
//
//=======================================================================================
LONG di_RegEnumKey( HKEY hkey, ULONG index, CHAR * pKeyName, ULONG length )
{
VP_STATUS status;
UNICODE_STRING ucValueData;
ANSI_STRING ansiValueData;
UCHAR buffer[sizeof(KEY_BASIC_INFORMATION)+512];
KEY_BASIC_INFORMATION *pValueInfo;
ULONG retLength;


    // query for ValueName's data
    pValueInfo = (KEY_BASIC_INFORMATION *)&buffer[0];
    status = ZwEnumerateKey(hkey, 
    						index, 
                            KeyBasicInformation,
                            pValueInfo,
                            sizeof(buffer),
                            &retLength);

	if (NT_SUCCESS(status))
    {
		pValueInfo->Name[pValueInfo->NameLength / 2] = '\0';

	   	// convert unicode data string to ansi data string
    	RtlInitUnicodeString(&ucValueData, (PWSTR)pValueInfo->Name);
    	RtlUnicodeStringToAnsiString(&ansiValueData, &ucValueData, TRUE);
		memcpy(pKeyName, ansiValueData.Buffer, ansiValueData.Length);
    	pKeyName[ansiValueData.Length] = '\0';
    	RtlFreeAnsiString(&ansiValueData);
	}

	// NOTE: Except for SUCCESS, the error codes returned here will not match   
	//       up to the error codes returned by the original RegEnumKey function.
  	return( (LONG)status );
}

//=======================================================================================
// Function name: di_FindDevNode 
//
// Description:   Use the videoport registry functions to write a value in the devnode path,
//				  then search for our special value among the possible devnodes.
//				  When found, fill the HwDeviceExtension->DevNodeKeyPath.
// Information:    
//
// Return:        void  
//
//=======================================================================================
#ifdef DISABLE_HARDCODED_REGPATH
void di_FindDevNode( PHW_DEVICE_EXTENSION HwDeviceExtension )
{
  UNICODE_STRING  ucDriverRegistryPath;
  ANSI_STRING     ansiDriverRegistryPath;


  RtlInitUnicodeString(&ucDriverRegistryPath, HwDeviceExtension->pDriverRegistryPath);
  RtlUnicodeStringToAnsiString(&ansiDriverRegistryPath, &ucDriverRegistryPath, TRUE);

  memcpy(HwDeviceExtension->DevNodeKeyPath,
         ansiDriverRegistryPath.Buffer,
         ansiDriverRegistryPath.Length);
  HwDeviceExtension->DevNodeKeyPath[ansiDriverRegistryPath.Length] = '\0';

  VideoDebugPrint((0, "di_FindDevNode: DriverRegistryPath = %s\n", HwDeviceExtension->DevNodeKeyPath));

  RtlFreeAnsiString(&ansiDriverRegistryPath);
}
#else
void di_FindDevNode( PHW_DEVICE_EXTENSION HwDeviceExtension )
{
static CHAR DevNodePath[] = NT_DEVNODE_PATH_STR;  
static CHAR SearchMsg[] = NT_DEVNODE_SEARCH_STR;
static CHAR SearchResetMsg[] = NT_DEVNODE_RESET_STR;
CHAR Data[sizeof(SearchMsg)];
VP_STATUS status;
UNICODE_STRING ucValueName;
ANSI_STRING ansiValueName;
HKEY hkey;
ULONG len;
ULONG Type;
int	i;

#if (_WIN32_WINNT < 0x0500)
	strcpy( HwDeviceExtension->DevNodeKeyPath, NT_DEVNODE_PATH_STR	);
	return;
#endif // (_WIN32_WINNT < 0x0500)

	// initialize unicode value name
    RtlInitAnsiString(&ansiValueName, NT_DEVNODE_SEARCH_KEY_STR);
    RtlAnsiStringToUnicodeString(&ucValueName, &ansiValueName, TRUE);

    // write data to search for in the registry
    status = VideoPortSetRegistryParameters( HwDeviceExtension,
                                              ucValueName.Buffer,
                                              &SearchMsg,
                                              sizeof(SearchMsg));
	// start with device 9, and work our down to Device0
	for ( i = 9; i >= 0; i-- )
	{
		DevNodePath[sizeof(DevNodePath) - 2] = (char) i + 0x30;
		if ( di_RegOpenKey( HKEY_LOCAL_MACHINE, DevNodePath, &hkey ) )
			continue;

		len = sizeof(Data);
		di_RegQueryValueEx( hkey, NT_DEVNODE_SEARCH_KEY_STR, 0, &Type, Data, &len );
		di_RegCloseKey( hkey );

		if ( memcmp( Data, SearchMsg, sizeof(SearchMsg) ) == 0 )
		{
			// We found our devnode, now save it away.
			strcpy( HwDeviceExtension->DevNodeKeyPath, DevNodePath	);
			break;
		}		
	}
	// reset data to search for in the registry
    status = VideoPortSetRegistryParameters( HwDeviceExtension,
                                              ucValueName.Buffer,
                                              &SearchResetMsg,
                                              sizeof(SearchResetMsg));

	RtlFreeUnicodeString(&ucValueName);

}
#endif

//=======================================================================================
// Function name: di_MemoryAlloc 
//
// Description: 	Allocate a block of memory according to the specified size   
//				
// Information:    
//
// Return:         	void * to allocated memory block 
//
//=======================================================================================
#ifdef MS_VIEW
void * di_MemoryAlloc( PHW_DEVICE_EXTENSION HwDeviceExtension, ULONG size )
{
	PHYSICAL_ADDRESS    physaddress = { 0xffffffff };
#else
void * di_MemoryAlloc( ULONG size )
{
#endif
    
#ifdef MS_VIEW
    return ( VideoPortAllocateContiguousMemory ( HwDeviceExtension, size, physaddress ));
#else
	return ( ExAllocatePool( NonPagedPool, size ));
#endif    
}


//=======================================================================================
// Function name:  ds_CalculatePitch
//
// Description:    Device specific function for calculating the pitch at a given bit depth
//				
// Information:    
//
// Return:         DWORD 	Calculated pitch value
//
//=======================================================================================
ULONG ds_CalculatePitch( ULONG Width, ULONG Bpp )
{
ULONG pitch;


	pitch =	Width;

	if ( pitch == 2046 )	// Pitch of 2046 modes is really 2048
		pitch += 2;
	
	switch ( Bpp )
	{
		case 16:
			pitch *= 2;
			//pitch += ( pitch % 128 );	 // right now the miniport does not know about tiled modes
			break;
		case 24:
			pitch *= 3;
			break;
		case 32:
			pitch *= 4;
			break;
	}

	return ( pitch );
}

//=======================================================================================
// Function name:  	di_xtoa
//
// Description:    	Converts an long to a character string. This function is called by
//				   	di_itoa(), di_ultoa(), and di_itoa().
// Information:    
//       			val - number to be converted (int, long or unsigned long)
//       			int radix - base to convert into
//       			char *buf - ptr to buffer to place result
//
// Return:         	VOID
//
//=======================================================================================
void di_xtoa ( unsigned long val, char *buf, unsigned radix, int is_neg	)
{
char *p;                // pointer to traverse string 
char *firstdig;         // pointer to first digit 
char temp;              // temp char 
unsigned digval;        // value of digit 


    p = buf;

    if (is_neg) 
    {
        // negative, so output '-' and negate 
        *p++ = '-';
        val = (unsigned long)(-(long)val);
    }

    firstdig = p;           // save pointer to first digit 

    do 
    {
        digval = (unsigned) (val % radix);
        val /= radix;       // get next digit 

        // convert to ascii and store 
        if (digval > 9)
            *p++ = (char) (digval - 10 + 'a');  // a letter 
        else
            *p++ = (char) (digval + '0');       // a digit 
    } while (val > 0);

        // We now have the digit of the number in the buffer, but in reverse
        //   order.  Thus we reverse them now. 

    *p-- = '\0';            // terminate string; p points to last digit 

    do 
    {
        temp = *p;
        *p = *firstdig;
        *firstdig = temp;   // swap *p and *firstdig 
        --p;
        ++firstdig;         // advance to next two digits 
    } while (firstdig < p); // repeat until halfway 
}

//=======================================================================================
// Function name:  	di_itoa
//
// Description:    	Converts an int to a character string. This function calls 
//					di_xtoa() with neg flag set correctly.
// Information:    
//       			val - number to be converted (integer )
//       			int radix - base to convert into
//       			char *buf - ptr to buffer to place result
//
// Return:         	VOID
//
//=======================================================================================
char * di_itoa ( int val, char *buf, int radix )
{
    if (radix == 10 && val < 0)
        di_xtoa((unsigned long)val, buf, radix, 1);
    else
        di_xtoa((unsigned long)(unsigned int)val, buf, radix, 0);
    return buf;
}

//=======================================================================================
// Function name:  	di_ltoa
//
// Description:    	Converts an long to a character string. This function calls 
//					di_xtoa() with neg flag set correctly.
// Information:    
//       			val - number to be converted ( long )
//       			int radix - base to convert into
//       			char *buf - ptr to buffer to place result
//
// Return:         	VOID
//
//=======================================================================================
char * di_ltoa ( long val, char *buf, int radix )
{
	di_xtoa(( unsigned long )val, buf, radix, (radix == 10 && val < 0));
    return buf;
}

//=======================================================================================
// Function name:  	di_ultoa
//
// Description:    	Converts an unsigned long to a character string. This function calls 
//					di_xtoa() with neg flag set correctly.
// Information:    
//       			val - number to be converted ( unsigned long )
//       			int radix - base to convert into
//       			char *buf - ptr to buffer to place result
//
// Return:         	VOID
//
//=======================================================================================
char * di_ultoa ( unsigned long val, char *buf,	int radix )
{
    di_xtoa(val, buf, radix, 0);
    return buf;
}

//=======================================================================================
// The following tables are used for conversion of ASCII chars to numeric values 
//=======================================================================================

unsigned char HexCovertTable[] =	{ 
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 0
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 16
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 32
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,255,255,255,255,255,255,		// 48
255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,		// 64
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 80
255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,		// 96
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 112
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 128
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 144
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 160
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 176
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 192
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 208
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,		// 224
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255			// 240
};

ULONG HexMultiplyTable[] = {
0x1,
0x10,
0x100,
0x1000,
0x10000,
0x100000,
0x1000000,
0x10000000	
};

// #pragma optimize("",off)

//=======================================================================================
// Function name:  	di_atox
//
// Description:    	Converts a character string value to a ulong. This function also determines
//	 				if the passed string value is in hex format, and returns a numeric ulong  
// 					value. If the string value is not preceeded by "0x", then this function
//					simply returns the result of a call to the standard library function atol().
// Information:    
//       			char *buf - ptr to buffer that holds the string value.
//
// Return:         	ulong value interpreted from the character string.
//
//=======================================================================================
long di_atox( char *buf )
{
ULONG retval = 0;
ULONG mult;
int i;
int j;


	// Search through non-numerical chars until we reach one we do recognize.
	for ( i = 0; HexCovertTable[buf[i]] == 255; i++ )  
	{								    
		if ( buf[i] == 0 )
			return( retval );
	}

	// If we find a "0x" in front of a string, we assume it is in HEX format.
	if ( buf[i] == '0' && ( buf[i+1] == 'x' || buf[i+1] == 'X') )
		i += 2;
	else
		return( atol( buf ) );	// Call the standard library atol function 

	// Search through numerical chars until we reach one that is non numerical.
	for ( j = i; HexCovertTable[buf[j]] != 255; j++ )  
		;

	// Limit the number of characters read to 8 so we do not overflow a ULONG and crash.
	if ( ( j - i ) > 8 )
		j = i + 8;
			
	for ( j--; i <= j; i++ )
	{
		mult = (ULONG)HexCovertTable[buf[i]]; 
		retval += ( HexMultiplyTable[j-i] * mult );	
	}

    return ( retval );
}

//=======================================================================================
// Function name:  	di_atoi
//
// Description:    	Converts a character string value to a integer. This function calls 
//					the main worker function di_atox(). 
// Information:    
//       			char *buf - ptr to buffer that holds the string value.
//
// Return:         	integer value interpreted from the character string.
//
//=======================================================================================
int di_atoi( char *buf )
{
	return( (int)di_atox( buf ) );
}

//=======================================================================================
// Function name:  	di_atol
//
// Description:    	Converts a character string value to a long. This function calls 
//					the main worker function di_atox(). 
// Information:    
//       			char *buf - ptr to buffer that holds the string value.
//
// Return:         	long value interpreted from the character string.
//
//=======================================================================================
long di_atol( char *buf )
{
	return( (long)di_atox( buf ) );
}

//=======================================================================================
// Function name:  	di_atoul
//
// Description:    	Converts a character string value to a ulong. This function calls 
//					the main worker function di_atox(). 
// Information:    
//       			char *buf - ptr to buffer that holds the string value.
//
// Return:         	ulong value interpreted from the character string.
//
//=======================================================================================
ULONG di_atoul( char *buf )
{
	return( di_atox( buf ) );
}

//=======================================================================================
// The baseline timings contain VESA and Industry Standard timing values.
//=======================================================================================

TIMING_PARAMS baseline_timings[] = {
//
// 	x ,   y ,ref, Htot,HsynS,HsynE,	Vtot,VsynS,VsynE,flg,  PixClock,CW,GT,CK
// 
{  320,  200, 70,  400,  328,  376,  449,  413,  415,  5,  12587500, 8, 0, 0 },
{  320,  400, 70,  400,  328,  376,  449,  413,  415,  4,  12587500, 8, 0, 0 },
{  360,  200, 70,  450,  369,  423,  449,  413,  415,  5,  14161000, 8, 0, 0 },
{  360,  400, 70,  450,  369,  423,  449,  413,  415,  4,  14161000, 8, 0, 0 },
{  640,  200, 70,  800,  656,  752,  449,  413,  415,  5,  25175000, 8, 0, 0 },
{  640,  350, 70,  800,  656,  752,  449,  387,  389,  8,  25175000, 8, 0, 0 },
{  640,  350, 85,  832,  672,  736,  445,  382,  385,  8,  31500000, 8, 0, 0 },
{  640,  400, 70,  800,  656,  752,  449,  413,  415,  4,  25175000, 8, 0, 0 },
{  640,  400, 85,  832,  672,  736,  445,  401,  404,  4,  31500000, 8, 0, 0 },
{  640,  480, 60,  800,  656,  752,  525,  490,  492, 12,  25175000, 8, 0, 0 },
{  640,  480, 72,  832,  664,  704,  520,  489,  492, 12,  31500000, 8, 0, 0 },
{  640,  480, 75,  840,  656,  720,  500,  481,  484, 12,  31500000, 8, 0, 0 },
{  640,  480, 85,  832,  696,  752,  509,  481,  484, 12,  36000000, 8, 0, 0 },
{  720,  400, 70,  900,  738,  846,  449,  413,  415,  4,  28322000, 9, 0, 0 },
{  720,  400, 85,  936,  756,  828,  446,  401,  404,  4,  35500000, 9, 0, 0 },
{  800,  600, 56, 1024,  824,  896,  625,  601,  603,  0,  36000000, 8, 0, 0 },
{  800,  600, 60, 1056,  840,  968,  628,  601,  605,  0,  40000000, 8, 0, 0 },
{  800,  600, 72, 1040,  856,  976,  666,  637,  643,  0,  50000000, 8, 0, 0 },
{  800,  600, 75, 1056,  816,  896,  625,  601,  604,  0,  49500000, 8, 0, 0 },
{  800,  600, 85, 1048,  832,  896,  631,  601,  604,  0,  56250000, 8, 0, 0 },
{ 1024,  768, 60, 1344, 1048, 1184,  806,  771,  777, 12,  65000000, 8, 0, 0 },
{ 1024,  768, 70, 1328, 1048, 1184,  806,  771,  777, 12,  75000000, 8, 0, 0 },
{ 1024,  768, 75, 1312, 1040, 1136,  800,  769,  772,  0,  78750000, 8, 0, 0 },
{ 1024,  768, 85, 1376, 1072, 1168,  808,  769,  772,  0,  94500000, 8, 0, 0 },
{ 1152,  864, 75, 1600, 1216, 1344,  900,  865,  868,  0, 108000000, 8, 0, 0 },
{ 1280,  960, 60, 1800, 1376, 1488, 1000,  961,  964,  0, 108000000, 8, 0, 0 },
{ 1280,  960, 85, 1728, 1344, 1504, 1011,  961,  964,  0, 148500000, 8, 0, 0 },
{ 1280, 1024, 60, 1688, 1328, 1440, 1066, 1025, 1028,  0, 108000000, 8, 0, 0 },
{ 1280, 1024, 75, 1688, 1296, 1440, 1066, 1025, 1028,  0, 135000000, 8, 0, 0 },
{ 1280, 1024, 85, 1728, 1344, 1504, 1072, 1025, 1028,  0, 157500000, 8, 0, 0 },
{ 1600, 1200, 60, 2160, 1664, 1856, 1250, 1201, 1204,  0, 162000000, 8, 0, 0 },
{ 1600, 1200, 65, 2160, 1664, 1856, 1250, 1201, 1204,  0, 175500000, 8, 0, 0 },
{ 1600, 1200, 70, 2160, 1664, 1856, 1250, 1201, 1204,  0, 189000000, 8, 0, 0 },
{ 1600, 1200, 75, 2160, 1664, 1856, 1250, 1201, 1204,  0, 202500000, 8, 0, 0 },
{ 1600, 1200, 85, 2160, 1664, 1856, 1250, 1201, 1204,  0, 229500000, 8, 0, 0 },
{ 1792, 1344, 60, 2448, 1920, 2120, 1394, 1345, 1348,  4, 204750000, 8, 0, 0 },
{ 1792, 1344, 75, 2456, 1888, 2104, 1417, 1345, 1348,  4, 261000000, 8, 0, 0 },
{ 1856, 1392, 60, 2528, 1952, 2176, 1439, 1393, 1396,  4, 218250000, 8, 0, 0 },
{ 1856, 1392, 75, 2560, 1984, 2208, 1500, 1393, 1396,  4, 288000000, 8, 0, 0 },
{ 1920, 1440, 60, 2600, 2048, 2256, 1500, 1441, 1444,  4, 234000000, 8, 0, 0 },
{ 1920, 1440, 75, 2640, 2064, 2288, 1500, 1441, 1444,  4, 297000000, 8, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};


//=======================================================================================
// Function name:  di_GetNumParamsInString
//
// Description:    Search a string for fields separated by commas and return the field count
//
// Information:    
//
// Return:         INT     Number of fields in the passed string
//
//=======================================================================================
int di_GetNumParamsInString( char * paramstr )
{
int fieldcount = 0;
int i;


	for ( i = 0; paramstr[i] != 0 ; i++ )	// count the commas in the parameter string
	{										// to determine if we are opting to use detailed
		if ( paramstr[i] == ',' )			// alternate timings or the simpler VESA 3 based timings.
			fieldcount++;
	}

	fieldcount++;

	return ( fieldcount );
}

//=======================================================================================
// Function name:  di_ParseStringForInteger
//
// Description:    Search a string for an int with fields separated by commas
//
// Information:    
//
// Return:         INT     Result returned by the call or,
//                        -1L if failure.
//=======================================================================================
int di_ParseStringForInteger( char *buf, int fieldnum )
{
char * chptr = buf;


	if ( fieldnum < 1 )	   	// Handle invalid field number
		return( -1 );

	fieldnum--;

	while ( fieldnum-- )
		{
			chptr = strchr( chptr, ',' );

			if ( chptr == NULL )
				return(	-1 );

			chptr++;
		}
	
	return( di_atoi( chptr ) );
}

//=======================================================================================
// Function name:  di_ParseStringForLong
//
// Description:    Search a string for a long with fields separated by commas
//
// Information:    
//
// Return:         LONG    Result returned by the call or,
//                        -1L if failure.
//=======================================================================================
long di_ParseStringForLong( char *buf, int fieldnum )
{
char * chptr = buf;


	if ( fieldnum < 1 )	   	// Handle invalid field number
		return( -1 );

	fieldnum--;

	while ( fieldnum-- )
		{
			chptr = strchr( chptr, ',' );

			if ( chptr == NULL )
				return(	-1 );

			chptr++;
		}
	
	return( di_atol( chptr ) );
}

//=======================================================================================
// Function name:  di_ParseStringForULong
//
// Description:    Search a string for a ULONG with fields separated by commas
//
// Information:    
//
// Return:         ULONG   Result returned by the call or,
//                         0xFFFFFFFF if failure.
//=======================================================================================
ULONG di_ParseStringForULong( char *buf, int fieldnum )
{
char * chptr = buf;


	if ( fieldnum < 1 )	   	// Handle invalid field number
		return( 0xFFFFFFFF );

	fieldnum--;

	while ( fieldnum-- )
		{
			chptr = strchr( chptr, ',' );

			if ( chptr == NULL )
				return(	0xFFFFFFFF );

			chptr++;
		}
	
	return( di_atoul( chptr ) );
}

//=======================================================================================
// Function name:  di_ParseBitDepthString
//
// Description:    Return a value that represents which color depths are available
//				   within a capability string.
// Information:    
//
// Return:         ULONG   Bits that represent each supported bit depth.
//                         
//=======================================================================================
ULONG di_ParseBitDepthString( char * capstr )
{
int i;
ULONG retval = 0;
char bppstr[MAX_CAP_STRING_SIZE];
char * bpp_ptr;


	strcpy( bppstr, capstr );

	bpp_ptr = strstr( bppstr, BPP_CAP_STR );

	if ( bpp_ptr )
	{
		// Move to the start of the bitdepth portion of the string
		bpp_ptr += sizeof(BPP_CAP_STR);	
		
		for ( i = 0; bppstr[i] != ',', bppstr[i] != 0; i++ )
			;
			
		bppstr[i] = 0;	// Append NULL to end of the string we are interested in
		
		if ( strstr( bppstr, "+8" ) )	 
			retval |= BPP_8_CAP_FLAG;
		if ( strstr( bppstr, "+16" ) )	 
			retval |= BPP_16_CAP_FLAG;
		if ( strstr( bppstr, "+24" ) )	 
			retval |= BPP_24_CAP_FLAG;
		if ( strstr( bppstr, "+32" ) )	 
			retval |= BPP_32_CAP_FLAG;
	}

	return( retval );
}

//=======================================================================================
// Function name:  di_QueryNumBitDepthsInString
//
// Description:    Return a value that represents how many color depths are available
//				   within a capability string.
// Information:    
//
// Return:         int   Number of color depths.
//                         
//=======================================================================================
int di_QueryNumBitDepthsInString( char * capstr )
{
int retval = 0;
ULONG bpp_support;


	bpp_support = di_ParseBitDepthString( capstr );

	if ( bpp_support & BPP_8_CAP_FLAG )
		retval++;
	if ( bpp_support & BPP_16_CAP_FLAG )
		retval++;
	if ( bpp_support & BPP_24_CAP_FLAG )
		retval++;
	if ( bpp_support & BPP_32_CAP_FLAG )
		retval++;

	return ( retval );
}

//=======================================================================================
// Function name:  di_ParseStringForCaps
//
// Description:    Parse the capabilities string for fields separated by commas, and
//				   return the appropriate bits for each capabliity.
// Information:    
//
// Return:         ULONG   Bits that represent each supported capability.
//                         
//=======================================================================================
ULONG di_ParseStringForCaps( char * capstr )
{
ULONG retval;


	retval = di_ParseBitDepthString( capstr );

	if ( strstr( capstr, DDRAW_CAPABLE_STR ) )
		retval |= DDRAW_CAPABLE;
	if ( strstr( capstr, TVOUT_DESKTOP_CAPABLE_STR ) )
		retval |= TVOUT_DESKTOP_CAPABLE;
	if ( strstr( capstr, TVOUT_DDRAW_CAPABLE_STR ) )
		retval |= TVOUT_DDRAW_CAPABLE;
	if ( strstr( capstr, DFP_DESKTOP_CAPABLE_STR ) )
		retval |= DFP_DESKTOP_CAPABLE;
	if ( strstr( capstr, DFP_DDRAW_CAPABLE_STR ) )
		retval |= DFP_DDRAW_CAPABLE;
	if ( strstr( capstr, NTSC_CAPABLE_STR ) )
		retval |= NTSC_CAPABLE;
	if ( strstr( capstr, PAL_CAPABLE_STR ) )
		retval |= PAL_CAPABLE;

	return( retval );	
}

//=======================================================================================
// Function name:  di_VerifyChecksumParam
//
// Description:    Calculate checksum from fields in the timings parameter string, and
//				   indicate pass or fail.
// Information:    
//
// Return:         BOOLEAN   TRUE if the checksum matches or,
//                        FALSE if failure.
//=======================================================================================
BOOLEAN di_VerifyChecksumParam( HKEY hkey, char *paramstr )
{
char * checkptr;
char nbrstr[MAX_NUMERIC_STRING_SIZE];
ULONG checksum;
UCHAR bytechecksum;
int fieldnum;


	// If we are to use Generalized Timing Formulas, then no verification is required
	if ( strstr( paramstr, "GTF" ) || strstr( paramstr, "gtf" ) )
		return( TRUE );	

	if ( di_GetNumParamsInString( paramstr ) <= CHECKSUM_FIELD )
	{
		// Detailed refresh parameter is not needed for timing calculations, but it is in our
		// parameter list for our convienience... However, it is still needed for the checksum. 
		checksum = di_ParseStringForULong( paramstr, REFRESH_FIELD ); 
		
		checksum += di_ParseStringForInteger( paramstr, HTOTAL_FIELD );
		checksum += di_ParseStringForInteger( paramstr, HSYNCSTART_FIELD );
		checksum += di_ParseStringForInteger( paramstr, HSYNCEND_FIELD );
		checksum += di_ParseStringForInteger( paramstr, VTOTAL_FIELD );
		checksum += di_ParseStringForInteger( paramstr, VSYNCSTART_FIELD );
		checksum += di_ParseStringForInteger( paramstr, VSYNCEND_FIELD );
		checksum += di_ParseStringForInteger( paramstr, CRTCFLAG_FIELD );
		checksum += ( di_ParseStringForULong( paramstr, PIXCLOCK_FIELD ) / 10000 );
		checksum += di_ParseStringForInteger( paramstr, CHARWIDTH_FIELD );

		checkptr = strstr( paramstr, "check" );

		if ( checkptr )
		{
			checkptr[0] = 0;
			di_ultoa( checksum, nbrstr, DECIMAL );
			strcat( paramstr, nbrstr );

			di_RegSetValueEx(hkey,
	                      TIMINGS_VALUE_STR,	// NULL ptr = Set Default value
	                      0,
	                      REG_SZ,
	                      (UCHAR *)paramstr,
	                      strlen(paramstr));
		}

		// If the checksum does not match, than the registry may be corrupt,
		// or the user has tampered with these values.
		if ( checksum != di_ParseStringForULong( paramstr, CHECKSUM_FIELD ) )
			return( FALSE );
	}
	else   	// use detailed CRTC table style timings
	{
		bytechecksum = 0;
		for ( fieldnum = 1; fieldnum < ALT_CHECKSUM_FIELD; fieldnum++ )
			bytechecksum += (UCHAR)di_ParseStringForInteger( paramstr,	fieldnum );

		checkptr = strstr( paramstr, "check" );

		if ( checkptr )
		{
			checkptr[0] = 0;
			di_ultoa( bytechecksum, nbrstr, DECIMAL );
			strcat( paramstr, nbrstr );

			di_RegSetValueEx(hkey,
	                      DFP_TIMINGS_VALUE_STR,
	                      0,
	                      REG_SZ,
	                      (UCHAR *)paramstr,
	                      strlen(paramstr));
		}

		// If the checksum does not match, than the registry may be corrupt,
		// or the user has tampered with these values.
		if ( bytechecksum != di_ParseStringForInteger( paramstr, ALT_CHECKSUM_FIELD ) )
			return( FALSE );
	}
			
	return( TRUE );	
}

//=======================================================================================
// Function name:  di_EnumBitDepth
//
// Description:    
//				
// Information:    
//
// Return:         
//				
//=======================================================================================
int di_EnumBitDepth( char * capstr, int index )
{
int i;
int retval = 0;
char bppstr[MAX_CAP_STRING_SIZE];
char * bpp_ptr;


	strcpy( bppstr, capstr );

	bpp_ptr = strstr( bppstr, BPP_CAP_STR );

	if ( bpp_ptr )
	{
		// Move to the start of the bitdepth portion of the string
		bpp_ptr += sizeof(BPP_CAP_STR);	
		
		for ( i = 0; bppstr[i] != ',', bppstr[i] != 0; i++ )
			;
			
		bppstr[i] = 0;	// Append NULL to end of the string we are interested in
		
		i = index - 1;

		if ( strstr( bppstr, "+32" ) )	 
		{
			if ( i-- <= 0 ) 
				return( 32 );
		}
		if ( strstr( bppstr, "+24" ) )
		{	 
			if ( i-- <= 0 ) 
				return( 24 );
		}
		if ( strstr( bppstr, "+16" ) )
		{	 
			if ( i-- <= 0 ) 
				return( 16 );
		}
		if ( strstr( bppstr, "+8" ) )
		{		 
			if ( i-- <= 0 ) 
				return( 8 );
		}
	}

	return( 0 );
}

//=======================================================================================
// Function name:  di_RegOpenTimingsKey
//
// Description:    Opens the main timings key that branches of of the devnode. 
//				
// Information:    
//
// Return:         int CR_SUCCESS - If successful,	
//					   CR_FAILURE - If failure
//=======================================================================================
int	di_RegOpenTimingsKey( PHW_DEVICE_EXTENSION HwDeviceExtension, char * DevNodeKey, HKEY * hkey )
{
	strcpy( DevNodeKey, HwDeviceExtension->DevNodeKeyPath );
	strcat( DevNodeKey, TIMINGS_KEY_STR );

    // attempt to open the main key
    if (CR_SUCCESS != di_RegOpenKey(HKEY_LOCAL_MACHINE,
                                        DevNodeKey,
                                        hkey))
    {
		return CR_FAILURE;
	}

	return CR_SUCCESS;
}

//=======================================================================================
// Function name:  di_GetBaselineTiming
//
// Description:    Searches baseline timing table for a match in width, height,
//				   and	refresh and loads the parameters into a timing struct.
//
// Information:    This function should be called only when the User has tampered with
//                 the TIMINGS registry entries, or the registry is corrupt.
//
// Return:         TRUE  - If an exact match is found in the baseline table
//  			   FALSE - If match is not found
//=======================================================================================
BOOLEAN di_GetBaselineTiming( TIMING_PARAMS *pTprm )
{
int i;
TIMING_PARAMS *pBaseline = &baseline_timings[0];


	for ( i = 0; pBaseline->width != 0; i++ )
	{
		if ( pBaseline->width == pTprm->width &&
			 pBaseline->height == pTprm->height &&
			 pBaseline->refresh == pTprm->refresh )
		{

			pTprm->HTotal = pBaseline->HTotal;
			pTprm->HSyncStart = pBaseline->HSyncStart;
			pTprm->HSyncEnd = pBaseline->HSyncEnd;
			pTprm->VTotal = pBaseline->VTotal;
			pTprm->VSyncStart = pBaseline->VSyncStart;
			pTprm->VSyncEnd = pBaseline->VSyncEnd;
			pTprm->CRTCflags = pBaseline->CRTCflags;
			pTprm->PixelClock = pBaseline->PixelClock;
			pTprm->CharWidth = pBaseline->CharWidth;
			pTprm->UseGTF = FALSE;
			pTprm->UseAltTiming = FALSE;
			return TRUE;
		}

		pBaseline = &baseline_timings[i];
	}

	pTprm->UseGTF = TRUE;	  	// Use GTF timing as a safety if we cannot find a matching entry
	pTprm->CharWidth = DEFAULT_CHAR_WIDTH;
	pTprm->CRTCflags = DEFAULT_CRTC_FLAGS;
	
	return FALSE;
}

//=======================================================================================
// Function name:  di_GetRegistryTiming
//
// Description:    Parses the TIMINGS Default key in the registry and 
//				   loads parameters into a timing struct.
// Information:    
//
// Return:         VOID
//
//=======================================================================================
void di_GetRegistryTiming( TIMING_PARAMS *pTprm, char * paramstr )
{
int i;
ULONG fout;
ULONG N, M, K;


	pTprm->UseGTF = FALSE;

	if ( di_GetNumParamsInString( paramstr ) <= CHECKSUM_FIELD )		
	{
		pTprm->HTotal = di_ParseStringForInteger( paramstr, HTOTAL_FIELD );
		pTprm->HSyncStart = di_ParseStringForInteger( paramstr, HSYNCSTART_FIELD );
		pTprm->HSyncEnd = di_ParseStringForInteger( paramstr, HSYNCEND_FIELD );
		pTprm->VTotal = di_ParseStringForInteger( paramstr, VTOTAL_FIELD );
		pTprm->VSyncStart = di_ParseStringForInteger( paramstr, VSYNCSTART_FIELD );
		pTprm->VSyncEnd = di_ParseStringForInteger( paramstr, VSYNCEND_FIELD );
		pTprm->CRTCflags = di_ParseStringForInteger( paramstr, CRTCFLAG_FIELD );
		pTprm->PixelClock = di_ParseStringForULong( paramstr, PIXCLOCK_FIELD );
		pTprm->CharWidth = di_ParseStringForInteger( paramstr, CHARWIDTH_FIELD );
		pTprm->UseAltTiming = FALSE;
	}
	else	// use detailed CRTC table style timings
	{
		for ( i = 0; i < ALT_CHECKSUM_FIELD; i++ )
			pTprm->AltTiming[i] = (UCHAR)di_ParseStringForInteger( paramstr, i + 1 );

		if ( pTprm->AltTiming[ALT_SR1] & 1 )
			pTprm->CharWidth = 8;
		else
			pTprm->CharWidth = 9;

		pTprm->HTotal = pTprm->AltTiming[ALT_HTOTAL] + 5;
		pTprm->HTotal *= 8;		// Convert chars to pixels

		pTprm->CRTCflags = 0;

		if ( pTprm->AltTiming[ALT_MISC_OUTPUT] & 0x80 )	// Setup polarity flags
			pTprm->CRTCflags |= 8;
		if ( pTprm->AltTiming[ALT_MISC_OUTPUT] & 0x40 )
			pTprm->CRTCflags |= 4;
		if ( pTprm->width < 640 )	  // Assume Scanline doubling if width is less than 640
			pTprm->CRTCflags |= 1;

		// Bit	Description
		// 1:0	K, Post divider value
		// 7:2	M, PLL input divider
		// 15:8	N, PLL multiplier

		// fout = 14.31818 * (N + 2) / (M + 2) / (2 ^ K).
		N = (ULONG)(pTprm->AltTiming[ALT_PLLCTRL0_HIGH] + 2 );
		M = (ULONG)(pTprm->AltTiming[ALT_PLLCTRL0_LOW] >> 2 ) + 2;
		K = (ULONG)(pTprm->AltTiming[ALT_PLLCTRL0_LOW] & 3) - 1;

		K = ( 2 << K );

		fout = ( 1431818 * N ) / M; 
		fout = fout / K;
		pTprm->PixelClock = fout * 10;

		pTprm->UseAltTiming = TRUE;
	}

	if ( strstr( paramstr, "GTF" ) || strstr( paramstr, "gtf" ) )
	{
		pTprm->UseGTF = TRUE;

		if ( pTprm->CharWidth == -1 )  	// GTF character width defaults to 8
			pTprm->CharWidth = DEFAULT_CHAR_WIDTH;

		if ( pTprm->CRTCflags == -1 )  	// GTF flags defaults to H-NEG, V-POS
			pTprm->CRTCflags = DEFAULT_CRTC_FLAGS;
	}
}

//=======================================================================================
// Function name:  di_QueryNumRegModes
//
// Description:    Counts the TIMINGS\XRES\YRES\REF keys in the registry for the 
//				   total number of modes.
//
// Information:    
//
// Return:         INT - Total number of modes found in registry.
//=======================================================================================
int di_QueryNumRegistryModes( PHW_DEVICE_EXTENSION HwDeviceExtension )
{
#ifndef MS_VIEW

HKEY hkey1 = 0;
HKEY hkey2 = 0;
HKEY hkey3 = 0;
int index1 = 0;
int index2 = 0;
int total = 0;
char subkey[MAX_NUMERIC_STRING_SIZE];
char subkey2[MAX_NUMERIC_STRING_SIZE];
char capstr[MAX_CAP_STRING_SIZE];
char DevNodeKey[MAX_TIMINGS_REG_KEY_LEN];
char DevNodeModeKey[sizeof(DevNodeKey)+sizeof(subkey)];
char DevNodeRefrKey[sizeof(DevNodeModeKey)+16];
ULONG length = sizeof(subkey);
ULONG caplength = sizeof(capstr);
ULONG type;


	if ( CR_FAILURE == di_RegOpenTimingsKey( HwDeviceExtension, DevNodeKey, &hkey1 ))
		return( 0 );
	
	// enumerate the list of mode entries
    while (CR_SUCCESS == di_RegEnumKey( hkey1, index1, subkey, length ))
    {
		strcpy( DevNodeModeKey, DevNodeKey );
		strcat( DevNodeModeKey, "\\" );
		strcat( DevNodeModeKey, subkey );
		index2 = 0;

      	// attempt to open the mode sub key
    	if ( CR_SUCCESS != di_RegOpenKey(HKEY_LOCAL_MACHINE, DevNodeModeKey, &hkey2))
			break;

		// enumerate the list of refresh rate entries
    	while ( CR_SUCCESS == di_RegEnumKey( hkey2, index2, subkey2, length ))
		{
			strcpy( DevNodeRefrKey, DevNodeModeKey );
			strcat( DevNodeRefrKey, "\\" );
			strcat( DevNodeRefrKey, subkey2 );

      		// attempt to open the refresh rate sub key
    		if (CR_SUCCESS != di_RegOpenKey(HKEY_LOCAL_MACHINE, DevNodeRefrKey, &hkey3))
				break;

			index2++;
			caplength = sizeof(capstr);

			// the key exists so attempt to read the value of varname
    		if (CR_SUCCESS != di_RegQueryValueEx(hkey3,
                                         SUPPORT_VALUE_STR,	
                                         0,
                                         &type,
                                         (UCHAR *)capstr,
                                         &caplength))
    		{
				continue;  // Oops,  no "Supported" entry was found
			}

			di_RegCloseKey(hkey3);

			total += di_QueryNumBitDepthsInString( capstr );

		}

		index1++;
		di_RegCloseKey(hkey2);
	}

	di_RegCloseKey(hkey1);

	return(total);

#else // MS_VIEW

#define MODE_KEY   L"Timings\\Modes"
WCHAR RATE_KEY[24] = {0};
WCHAR BIT_DEPTH_KEY[32] = {0};

WCHAR ModeList[255] = {0};
WCHAR RateList[255] = {0};
WCHAR BitDepthList[255] = {0};

WCHAR *Mode, *RefreshRate;
H3REGCBDATA H3RegData_Mode, H3RegData_Rates, H3RegData_BPP;
int counter = 0, i = 0;
VP_STATUS status;

H3RegData_Mode.pUCBuffer = ModeList;
H3RegData_Mode.ulLength = 512;


	//parse the modes key to get the list of modes - stever
	status = VideoPortGetRegistryParameters( HwDeviceExtension,
                                             MODE_KEY,
                                             FALSE,
											 H3QueryRegistryValueCallback,
											 &H3RegData_Mode);

    Mode = wcstok( ModeList, L";" );
    while (Mode != NULL)
    {
        wcscpy(RATE_KEY, L"Timings\\" );
        wcscat( RATE_KEY, Mode );
        wcscat( RATE_KEY, L"\\Rates" );

        H3RegData_Rates.pUCBuffer = RateList;
        H3RegData_Rates.ulLength = 512;

        status = VideoPortGetRegistryParameters( HwDeviceExtension,
                                                 (PWSTR)RATE_KEY,
                                                 FALSE,
                                                 H3QueryRegistryValueCallback,
                                                 &H3RegData_Rates);

        RefreshRate = wcstok( RateList, L";");
        while (RefreshRate != NULL)
        {
            wcscpy(BIT_DEPTH_KEY, L"Timings\\" );
            wcscat( BIT_DEPTH_KEY, Mode );
            wcscat( BIT_DEPTH_KEY, L"\\" );
            wcscat( BIT_DEPTH_KEY, RefreshRate );
            wcscat( BIT_DEPTH_KEY, L"\\Supported" );
            
            H3RegData_BPP.pUCBuffer = BitDepthList;
            H3RegData_BPP.ulLength = 512;

            status = VideoPortGetRegistryParameters( HwDeviceExtension,
                                                     (PWSTR)BIT_DEPTH_KEY,
                                                     FALSE,
                                                     H3QueryRegistryValueCallback,
                                                     &H3RegData_BPP);
            
            i = 0;
            while (BitDepthList[i] != '\0')
	        {
		        if (BitDepthList[i] == '8'  ||
                   (BitDepthList[i] == '1' && BitDepthList[i+1] == '6') ||
                   (BitDepthList[i] == '2' && BitDepthList[i+1] == '4') ||
                   (BitDepthList[i] == '3' && BitDepthList[i+1] == '2'))
		    	       counter++;  //count the number of modes for each refresh for each desktop size
		        i++;
	        }
            RefreshRate = wcstok( RefreshRate + wcslen(RefreshRate) + 1, L";" );
        }

        Mode = wcstok( Mode + wcslen(Mode) + 1, L";" );
    }
	return(counter);

#endif // MS_VIEW
}

//=======================================================================================
// Function name:  di_EnumRegistryMode
//
// Description:    Enumerates the TIMINGS\XRES,YRES\REF keys in the registry,
//                 and fills in the appropriate MODEINFO structure.
// Information:    
//
// Return:         TRUE - If a mode was found according to the given index.
// 				   FALSE - If a mode was NOT found	according to the given index.
//=======================================================================================
#ifndef MS_VIEW
BOOLEAN di_EnumRegistryMode( PHW_DEVICE_EXTENSION HwDeviceExtension, H3_VIDEO_FREQUENCIES * VideoFreq, int index )
#else
BOOLEAN di_EnumRegistryMode( PHW_DEVICE_EXTENSION HwDeviceExtension, H3_VIDEO_FREQUENCIES * VideoFreq )
#endif
{
#ifndef MS_VIEW

TIMING_PARAMS timingparams;
HKEY hkey1 = 0;
HKEY hkey2 = 0;
HKEY hkey3 = 0;
int index1 = 0;
int index2 = 0;
int total = 0;
long pixelclock;
long htotal;
char subkey[MAX_NUMERIC_STRING_SIZE];
char subkey2[MAX_NUMERIC_STRING_SIZE];
char paramstr[MAX_TIMING_STRING_SIZE];
char capstr[MAX_CAP_STRING_SIZE];
char DevNodeKey[MAX_TIMINGS_REG_KEY_LEN];
char DevNodeModeKey[MAX_TIMINGS_REG_KEY_LEN+sizeof(subkey)];
char DevNodeRefrKey[sizeof(DevNodeModeKey)+16];
ULONG type;
ULONG capflag;
ULONG length = sizeof(subkey);
ULONG length2 = sizeof(subkey2);
ULONG caplength = sizeof(capstr);
ULONG paramlength = sizeof(paramstr);
BOOLEAN modefound = FALSE;
BOOLEAN passchecksum;
BOOLEAN read_alt_timings;


	if ( CR_FAILURE == di_RegOpenTimingsKey( HwDeviceExtension, DevNodeKey, &hkey1 ))
		return( FALSE );
	
	// enumerate the list of mode entries
    while (CR_SUCCESS == di_RegEnumKey( hkey1, index1, subkey, length ))
    {
		strcpy( DevNodeModeKey, DevNodeKey );
		strcat( DevNodeModeKey, "\\" );
		strcat( DevNodeModeKey, subkey );
		index2 = 0;

      	// attempt to open the mode sub key
    	if ( CR_SUCCESS != di_RegOpenKey(HKEY_LOCAL_MACHINE, DevNodeModeKey, &hkey2))
			break;

		// enumerate the list of refresh rate entries
    	while ( CR_SUCCESS == di_RegEnumKey( hkey2, index2, subkey2, length2 ))
		{
			strcpy( DevNodeRefrKey, DevNodeModeKey );
			strcat( DevNodeRefrKey, "\\" );
			strcat( DevNodeRefrKey, subkey2 );

      		// attempt to open the refresh rate sub key
    		if (CR_SUCCESS != di_RegOpenKey(HKEY_LOCAL_MACHINE, DevNodeRefrKey, &hkey3))
				break;

			index2++;
			caplength = sizeof(capstr);

			// the key exists so attempt to read the value of varname
    		if (CR_SUCCESS != di_RegQueryValueEx(hkey3,
                                         SUPPORT_VALUE_STR,	
                                         0,
                                         &type,
                                         (UCHAR *)capstr,
                                         &caplength))
    		{
				di_RegCloseKey(hkey3);
				continue;  // Oops,  no "Supported" entry
			}

			paramlength = sizeof(paramstr);

			// the key exists so attempt to read the parameter string

			// alternate timing entries for DFPs are obsolete, so read_alt_timings is always FALSE.  DanO 05/09/00
			read_alt_timings = FALSE;

			if ( read_alt_timings == FALSE )
			{
	    		if (CR_SUCCESS != di_RegQueryValueEx(hkey3,
	                                         TIMINGS_VALUE_STR,	
	                                         0,
	                                         &type,
	                                         (UCHAR *)paramstr,
	                                         &paramlength))
	    		{
					di_RegCloseKey(hkey3);
					continue;  // Oops,  no "Default" entry
				}
			}

			capflag = di_ParseStringForCaps( capstr );

			total += di_QueryNumBitDepthsInString( capstr );

			if ( total > index )
			{
				timingparams.width = di_ParseStringForInteger( subkey, 1 );
				timingparams.height = di_ParseStringForInteger( subkey, 2 );
				timingparams.refresh = di_ParseStringForInteger( subkey2, 1 );
	
				// check for user tampering
				passchecksum = di_VerifyChecksumParam( hkey3, paramstr );

				if ( passchecksum )	// If there is no evidence of user tampering
					di_GetRegistryTiming( &timingparams, paramstr );
				else 				// User has tampered with registry!
					di_GetBaselineTiming( &timingparams );	

				if ( timingparams.UseGTF == TRUE || HwDeviceExtension->RegistryForceGTF == TRUE )
				{
					if ( !( timingparams.CRTCflags & 1 ) )		// If not Double Scanned
						timingparams.CRTCflags = DEFAULT_CRTC_FLAGS;

					di_GetGTF_Timing( &timingparams );
				}

				// save 1024x768@60 parameters to use as base for centered DFP support
                if ((timingparams.width == 1024) &&
                        (timingparams.height == 768) &&
                        (timingparams.refresh == 60))
                {
                    memcpy( &HwDeviceExtension->centeredDfpBaseTimings, 
                            &timingparams, 
                            sizeof(TIMING_PARAMS) );
                }

                ds_Calc_CRTC_table( HwDeviceExtension, &timingparams, timingparams.width, timingparams.height, VideoFreq->RegistryVideoData);

				VideoFreq->ScreenWidth = timingparams.width;
				VideoFreq->ScreenHeight = timingparams.height;
				VideoFreq->ScreenFrequency = timingparams.refresh;
				VideoFreq->BitsPerPel = di_EnumBitDepth( capstr, total - index );
				VideoFreq->VideoData = VideoFreq->RegistryVideoData;

				VideoFreq->ModeEntry = &VideoFreq->RegistryModeEntry;

				VideoFreq->ModeEntry->Int10ModeNumberContiguous = 0;
				VideoFreq->ModeEntry->Int10ModeNumberNoncontiguous	= 0;

				switch ( VideoFreq->BitsPerPel )
				{
					case 8:
 						memcpy( &VideoFreq->ModeEntry->ModeInformation, 
 								&BasicModeInfo_8BPP, 
 								sizeof(VIDEO_MODE_INFORMATION) );
						break;
					case 16:
 						memcpy( &VideoFreq->ModeEntry->ModeInformation, 
 								&BasicModeInfo_16BPP, 
 								sizeof(VIDEO_MODE_INFORMATION) );
						break;
					case 32:
	 					memcpy( &VideoFreq->ModeEntry->ModeInformation, 
 								&BasicModeInfo_32BPP, 
 								sizeof(VIDEO_MODE_INFORMATION) );
						break;
				}

				if ( timingparams.CRTCflags & 1 )	// If the mode is Double Scanned retouch the ModeInfo struct
				{	
					VideoFreq->ModeEntry->ModeInformation.DriverSpecificAttributeFlags |= CAPS_SCAN_LINE_DOUBLED;
					VideoFreq->ModeEntry->ModeInformation.DriverSpecificAttributeFlags |= CAPS_SW_POINTER;
				}

       			if ((IS_VOODOO3) && ( timingparams.width == 2048 ))	// Voodoo3 must use SW cursor at this res
					VideoFreq->ModeEntry->ModeInformation.DriverSpecificAttributeFlags |= CAPS_SW_POINTER;
	
				VideoFreq->ModeEntry->ModeInformation.VisScreenWidth = timingparams.width;
				VideoFreq->ModeEntry->ModeInformation.VisScreenHeight = timingparams.height;
				VideoFreq->ModeEntry->ModeInformation.ScreenStride = ds_CalculatePitch( VideoFreq->ScreenWidth, VideoFreq->BitsPerPel );
				VideoFreq->ModeEntry->ScreenStrideContiguous = VideoFreq->ModeEntry->ModeInformation.ScreenStride;

				if ( capflag & TVOUT_DESKTOP_CAPABLE )
					VideoFreq->isValidTvOutMode = TRUE;

				if ( capflag & TVOUT_DDRAW_CAPABLE )
					VideoFreq->isValidTvOutMode = TRUE;

				di_RegCloseKey(hkey3);
				modefound = TRUE;
				break;
			}

			di_RegCloseKey(hkey3);
		}

		if ( modefound )
			break;

		index1++;
		di_RegCloseKey(hkey2);
	}

	di_RegCloseKey(hkey1);

	return( modefound );

#else // MS_VIEW

ANSI_STRING ModeANSI, RefreshANSI;
UNICODE_STRING ModeUC, RefreshUC;
TIMING_PARAMS timingparams;
char paramstr[MAX_TIMING_STRING_SIZE];
char capstr[MAX_CAP_STRING_SIZE];
ULONG capflag;
ULONG caplength = sizeof(capstr);
ULONG paramlength = sizeof(paramstr);
BOOLEAN modefound = FALSE;
BOOLEAN passchecksum;
BOOLEAN read_alt_timings;
#define MODE_KEY   L"Timings\\Modes"
WCHAR RATE_KEY[24] = {0};
WCHAR GTF_KEY[24] = {0};
WCHAR BIT_DEPTH_KEY[32] = {0};
WCHAR ModeList[255] = {0};
WCHAR RateList[255] = {0};
char  BitDepthList[255] = {0};

WCHAR *Mode, *RefreshRate;
H3REGCBDATA H3RegData_Mode, H3RegData_Rates, H3RegData_BPP, H3RegData_GTF;
//FIX for build error int counter = 0, i = 0, index = 0, BitDepth = 0;
int counter = 0, i = 0, BitDepth = 0;
ULONG index = 0;
//END OF FIX
VP_STATUS status;

H3RegData_Mode.pUCBuffer = ModeList;
H3RegData_Mode.ulLength = 512;


	
    while ( index < HwDeviceExtension->RegistryNumModes )
    {

	    status = VideoPortGetRegistryParameters( HwDeviceExtension,
                                                 MODE_KEY,
                                                 FALSE,
											     H3QueryRegistryValueCallback,
											     &H3RegData_Mode);

        Mode = wcstok( ModeList, L";" );
        
        while (Mode != NULL)
        {
            ModeUC.Buffer = Mode;
            ModeUC.Length = wcslen(Mode) * 2;
            ModeUC.MaximumLength = ModeUC.Length + 2;
            ModeANSI.Buffer = NULL;
            ModeANSI.Length = 0;
            ModeANSI.MaximumLength = 0;
            wcscpy(RATE_KEY, L"Timings\\" );
            wcscat( RATE_KEY, Mode );
            wcscat( RATE_KEY, L"\\Rates" );

            H3RegData_Rates.pUCBuffer = RateList;
            H3RegData_Rates.ulLength = 512;

            status = VideoPortGetRegistryParameters( HwDeviceExtension,
                                                     (PWSTR)RATE_KEY,
                                                     FALSE,
                                                     H3QueryRegistryValueCallback,
                                                     &H3RegData_Rates);

            RefreshRate = wcstok( RateList, L";");
            
            while (RefreshRate != NULL)
            {
                RefreshUC.Buffer = RefreshRate;
                RefreshUC.Length = wcslen( RefreshRate ) * 2;
                RefreshUC.MaximumLength = RefreshUC.Length + 2;
                RefreshANSI.Buffer = NULL;
                RefreshANSI.Length = 0;
                RefreshANSI.MaximumLength = 0;
                wcscpy( BIT_DEPTH_KEY, L"Timings\\" );
                wcscat( BIT_DEPTH_KEY, Mode );
                wcscat( BIT_DEPTH_KEY, L"\\" );
                wcscat( BIT_DEPTH_KEY, RefreshRate );
                wcscpy( GTF_KEY, BIT_DEPTH_KEY );
                wcscat( BIT_DEPTH_KEY, L"\\Supported" );
            
                H3RegData_BPP.pBuffer = BitDepthList;
                H3RegData_BPP.ulLength = 512;

                status = VideoPortGetRegistryParameters( HwDeviceExtension,
                                                         (PWSTR)BIT_DEPTH_KEY,
                                                         FALSE,
                                                         SSTH3QueryRegistryValueCallback,
                                                         &H3RegData_BPP);
                //got BitDepthList now

                
                
                
                i = 0;
                while (BitDepthList[i] != '\0')
	            {
		            if (BitDepthList[i] == '8')
                    {
                        BitDepth = 8;
                    }
                    else if (BitDepthList[i] == '1' && BitDepthList[i+1] == '6')
                    {
                        BitDepth = 16;
                    }
                    else if (BitDepthList[i] == '2' && BitDepthList[i+1] == '4')
                    {
                        BitDepth = 24;
                    }
                    else if (BitDepthList[i] == '3' && BitDepthList[i+1] == '2')
                    {
                        BitDepth = 32;
                    }
                    
                    if ((BitDepth == 8)  || 
                        (BitDepth == 16) ||
                        (BitDepth == 24) ||
                        (BitDepth == 32))
                    {

                        paramlength = sizeof(paramstr);

			            // the key exists so attempt to read the parameter string
                        // alternate timing entries for DFPs are obsolete, so read_alt_timings is always FALSE.  DanO 05/09/00
			            read_alt_timings = FALSE;

			            if ( read_alt_timings == FALSE )
			            {
                            wcscat( GTF_KEY, L"\\" );
	    		            H3RegData_GTF.pBuffer = paramstr;
                            H3RegData_GTF.ulLength = 512;
                            status = VideoPortGetRegistryParameters( HwDeviceExtension,
                                                                     (PWSTR)GTF_KEY,
                                                                     FALSE,
                                                                     SSTH3QueryRegistryValueCallback,
                                                                     &H3RegData_GTF);
                			                
			            }

			            capflag = di_ParseStringForCaps( BitDepthList ); 

			            RtlUnicodeStringToAnsiString( &ModeANSI, &ModeUC, TRUE);
                        RtlUnicodeStringToAnsiString( &RefreshANSI, &RefreshUC, TRUE);
                
				        timingparams.width = di_ParseStringForInteger( ModeANSI.Buffer, 1 );
				        timingparams.height = di_ParseStringForInteger( ModeANSI.Buffer, 2 );
				        timingparams.refresh = di_ParseStringForInteger( RefreshANSI.Buffer, 1 );

	                    RtlFreeAnsiString( &ModeANSI );
                        RtlFreeAnsiString( &RefreshANSI );
            		    // check for user tampering
				        passchecksum = TRUE;//di_VerifyChecksumParam( hkey3, paramstr );

				        if ( passchecksum )	// If there is no evidence of user tampering
				            di_GetRegistryTiming( &timingparams, paramstr );
				        else 				// User has tampered with registry!
				            di_GetBaselineTiming( &timingparams );	

				        if ( timingparams.UseGTF == TRUE || HwDeviceExtension->RegistryForceGTF == TRUE )
				        {
				            if ( !( timingparams.CRTCflags & 1 ) )		// If not Double Scanned
					            timingparams.CRTCflags = DEFAULT_CRTC_FLAGS;

				            di_GetGTF_Timing( &timingparams );
				        }
            
				        // save 1024x768@60 parameters to use as base for centered DFP support
                        if ((timingparams.width == 1024) &&
                                (timingparams.height == 768) &&
                                (timingparams.refresh == 60))
                        {
                            memcpy( &HwDeviceExtension->centeredDfpBaseTimings, 
                                    &timingparams, 
                                    sizeof(TIMING_PARAMS) );
                        }

                        ds_Calc_CRTC_table( HwDeviceExtension, &timingparams, timingparams.width, timingparams.height, VideoFreq->RegistryVideoData);

				        VideoFreq->ScreenWidth = timingparams.width;
				        VideoFreq->ScreenHeight = timingparams.height;
				        VideoFreq->ScreenFrequency = timingparams.refresh;
				        VideoFreq->BitsPerPel = BitDepth;
				        VideoFreq->VideoData = VideoFreq->RegistryVideoData;

				        VideoFreq->ModeEntry = &VideoFreq->RegistryModeEntry;

				        VideoFreq->ModeEntry->Int10ModeNumberContiguous = 0;
				        VideoFreq->ModeEntry->Int10ModeNumberNoncontiguous	= 0;

				        switch ( VideoFreq->BitsPerPel )
				        {
				            case 8:
 					            memcpy( &VideoFreq->ModeEntry->ModeInformation, 
 						                &BasicModeInfo_8BPP, 
 						                sizeof(VIDEO_MODE_INFORMATION) );
					            break;
				            case 16:
 					            memcpy( &VideoFreq->ModeEntry->ModeInformation, 
 						                &BasicModeInfo_16BPP, 
 						                sizeof(VIDEO_MODE_INFORMATION) );
					            break;
				            case 32:
	 				            memcpy( &VideoFreq->ModeEntry->ModeInformation, 
 						                &BasicModeInfo_32BPP, 
 						                sizeof(VIDEO_MODE_INFORMATION) );
					            break;
				        }

				        if ( timingparams.CRTCflags & 1 )	// If the mode is Double Scanned retouch the ModeInfo struct
				        {	
				            VideoFreq->ModeEntry->ModeInformation.DriverSpecificAttributeFlags |= CAPS_SCAN_LINE_DOUBLED;
				            VideoFreq->ModeEntry->ModeInformation.DriverSpecificAttributeFlags |= CAPS_SW_POINTER;
				        }

       			        if ((IS_VOODOO3) && ( timingparams.width == 2048 ))	// Voodoo3 must use SW cursor at this res
				            VideoFreq->ModeEntry->ModeInformation.DriverSpecificAttributeFlags |= CAPS_SW_POINTER;
	                
				        VideoFreq->ModeEntry->ModeInformation.VisScreenWidth = timingparams.width;
				        VideoFreq->ModeEntry->ModeInformation.VisScreenHeight = timingparams.height;
				        VideoFreq->ModeEntry->ModeInformation.ScreenStride = ds_CalculatePitch( VideoFreq->ScreenWidth, VideoFreq->BitsPerPel );
				        VideoFreq->ModeEntry->ScreenStrideContiguous = VideoFreq->ModeEntry->ModeInformation.ScreenStride;

				        if ( capflag & TVOUT_DESKTOP_CAPABLE )
				            VideoFreq->isValidTvOutMode = TRUE;

				        if ( capflag & TVOUT_DDRAW_CAPABLE )
				            VideoFreq->isValidTvOutMode = TRUE;

				        BitDepth = 0;
				        modefound = TRUE;
                        VideoFreq->ModeIndex = index;
                        index++;
                        VideoFreq++;
                        if (index == 255)
                            goto Done;
                    }
	      
		            i++;
	            }
           
		        RefreshRate = wcstok( RefreshRate + wcslen(RefreshRate) + 1, L";" );
            }
            Mode = wcstok( Mode + wcslen(Mode) + 1, L";" );
        }
        
    }
    
Done:
    return (modefound);//temp    

#endif MS_VIEW
}

//=======================================================================================
// Function name:  di_GetRegistryGTFOverride
//
// Description:    Reads the TIMINGS\UseGTF entry in the registry,
// 				   and fills a BOOLEAN with TRUE or FALSE according to the value.
// Information:    
//
// Return:         TRUE - If the registry entry was found and read.
//				   FALSE - If the registry entry was NOT read.
//=======================================================================================
BOOLEAN di_GetRegistryGTFOverride( PHW_DEVICE_EXTENSION HwDeviceExtension, BOOLEAN * UseGTF )
{
#ifndef MS_VIEW
HKEY hkey;
char subkey[] = USE_GTF_VALUE_STR;
char nbrstr[MAX_NUMERIC_STRING_SIZE];
char DevNodeKey[MAX_TIMINGS_REG_KEY_LEN];
ULONG type;
ULONG length = sizeof(nbrstr);


	if ( CR_FAILURE == di_RegOpenTimingsKey( HwDeviceExtension, DevNodeKey, &hkey ))
		return( FALSE );
	
    // the key exists so attempt to read the value of varname
    if (CR_SUCCESS != di_RegQueryValueEx(hkey,
                                         subkey,	// NULL = Get Default value
                                         0,
                                         &type,
                                         (UCHAR *)nbrstr,
                                         &length))
    {
		di_RegCloseKey(hkey);
		return FALSE;
	}

	di_RegCloseKey(hkey);

	if ( atoi(&nbrstr[0]) )
		*UseGTF = TRUE;
	else
		*UseGTF = FALSE;

	return TRUE;

#else //MS_VIEW

#define GTF_KEY   L"Timings\\UseGTF"

VP_STATUS status;
WCHAR GTFFlag[4] = {0};
H3REGCBDATA H3RegGTFOverride;

H3RegGTFOverride.pUCBuffer = GTFFlag;
H3RegGTFOverride.ulLength = sizeof(GTFFlag);


    status = VideoPortGetRegistryParameters( HwDeviceExtension,
                                             GTF_KEY,
                                             FALSE,
											 H3QueryRegistryValueCallback,
											 &H3RegGTFOverride);

    if (GTFFlag[0] == '0')
       *UseGTF = FALSE;
    else
       *UseGTF = TRUE;

    if (status)
        return(FALSE);
    else
        return(TRUE);

#endif //MS_VIEW
}

//=======================================================================================
// Function name:  di_GetRegistryOptRefreshLimit
//
// Description:    Reads the TIMINGS\OptimalRefreshLimit entry in the registry,
//				   and fills a integer with according to the value. Also reads the
//				   TIMINGS\OptimalNonEDIDLimit entry and fills that integer with that value.
// Information:    
//
// Return:         TRUE - If the registry entry was found and read.
//				   FALSE - If the registry entry was NOT read.
//=======================================================================================
BOOLEAN di_GetRegistryOptRefreshLimit( PHW_DEVICE_EXTENSION HwDeviceExtension, int * OptRefreshLim, int * OptNonEDIDLim )
{
HKEY hkey;
char subkey1[] = OPT_REFRESH_LIMIT_STR;
char subkey2[] = OPT_NONEDID_LIMIT_STR;
char nbrstr[MAX_NUMERIC_STRING_SIZE];
char DevNodeKey[MAX_TIMINGS_REG_KEY_LEN];
ULONG type;
ULONG length = sizeof(nbrstr);


	*OptRefreshLim = MAX_REFRESH_RATE;
	*OptNonEDIDLim = MAX_NONEDID_RATE;

	if ( CR_FAILURE == di_RegOpenTimingsKey( HwDeviceExtension, DevNodeKey, &hkey ))
		return( FALSE );
	
    // the key exists so attempt to read the value of varname
    if (CR_SUCCESS != di_RegQueryValueEx(hkey,
                                         subkey1,	// NULL = Get Default value
                                         0,
                                         &type,
                                         (UCHAR *)nbrstr,
                                         &length))
    {
		di_RegCloseKey(hkey);
		return FALSE;
	}

	if ( strstr( nbrstr, "MAX_EDID" ) || strstr( nbrstr, "Max_EDID" ) )
		*OptRefreshLim = MAX_EDID_LIMIT;
	else
		*OptRefreshLim = atoi(&nbrstr[0]);

    // the key exists so attempt to read the value of varname
    if (CR_SUCCESS != di_RegQueryValueEx(hkey,
                                         subkey2,	// NULL = Get Default value
                                         0,
                                         &type,
                                         (UCHAR *)nbrstr,
                                         &length))
    {
		di_RegCloseKey(hkey);
		return FALSE;
	}

	di_RegCloseKey(hkey);

	*OptNonEDIDLim = atoi(&nbrstr[0]);

	return TRUE;
}

//=======================================================================================
// Function name:  di_AllocModeTable
//
// Description:    Allocates space for the master mode list array, and fills the passed
//				   integer address with the total number of available modes.
//
// Information:    
//
// Return:         VOID	*  Non Null if successful	
//							   Null if failure
//=======================================================================================
void * di_AllocModeTable( PHW_DEVICE_EXTENSION HwDeviceExtension, ULONG * nummodes )
{
ULONG bufsize;
void * bufptr = 0;
ULONG numfoundmodes;

	
	*nummodes = di_QueryNumRegistryModes( HwDeviceExtension );
  // NT only supports up to 255 modes
  if (*nummodes > 255)
    *nummodes = 255;

	numfoundmodes = *nummodes;

    if ( numfoundmodes == 0 )
		numfoundmodes = 1;		// Allow space for 1 default mode if registry fails us.

	// The reason why we add one to the allocation is that some support functions
	// detect the end of the mode list by comparing the width to 0, so we need the
	// last structure entry to be empty and initialized to zero.
	bufsize = ( ( numfoundmodes + 1 ) * sizeof( H3_VIDEO_FREQUENCIES ) );

#ifdef MS_VIEW
    bufptr = di_MemoryAlloc( HwDeviceExtension, bufsize );	 
#else
	bufptr = di_MemoryAlloc( bufsize );
#endif    

	if ( bufptr )
	{
        memset(bufptr, 0, bufsize);
	 
		HwDeviceExtension->RegistryVideoFreqs = ( H3_VIDEO_FREQUENCIES *)bufptr;	
	}

	return ( bufptr );
}

//=======================================================================================
// Function name:  di_BuildDefaultMode
//
// Description:    Generate a single definition of a default mode in the event of a catastropic
//				   registry read failure to prevent coming up in standard VGA mode.
//				   
// Information:    
//
// Return:         VOID.
//=======================================================================================
void di_BuildDefaultMode( H3_VIDEO_FREQUENCIES * VideoFreq )
{
	// The current default mode is the first entry in the table... 640x480x8x60hz
	memcpy( VideoFreq , &H4GenericFrequencyTable[0], sizeof( H3_VIDEO_FREQUENCIES ));
	memcpy( &VideoFreq->RegistryModeEntry, DefaultMode, sizeof(H3_VIDEO_MODES) );
	VideoFreq->ModeEntry = &VideoFreq->RegistryModeEntry;
	VideoFreq->ModeIndex = 1;

}

//=======================================================================================
// Function name:  di_SwapFrequencyStructs
//
// Description:    Exchange the contents of two H3_VIDEO_FREQUENCIES structures.
//				   This is accomplished by a memcopy operation followed by a retouching
//				   of the pointers that need relocating.
// Information:    
//
// Return:         VOID.
//=======================================================================================
void di_SwapFrequencyStructs( H3_VIDEO_FREQUENCIES * VideoFreq1, H3_VIDEO_FREQUENCIES * VideoFreq2 )
{
H3_VIDEO_FREQUENCIES TempVideoFreq;


	memcpy( &TempVideoFreq, VideoFreq2, sizeof(H3_VIDEO_FREQUENCIES) );
	memcpy( VideoFreq2, VideoFreq1, sizeof(H3_VIDEO_FREQUENCIES) );
	memcpy( VideoFreq1, &TempVideoFreq, sizeof(H3_VIDEO_FREQUENCIES) );
	VideoFreq1->VideoData = VideoFreq1->RegistryVideoData;
	VideoFreq1->ModeEntry = &VideoFreq1->RegistryModeEntry;
	VideoFreq2->VideoData = VideoFreq2->RegistryVideoData;
	VideoFreq2->ModeEntry = &VideoFreq2->RegistryModeEntry;

}

//=======================================================================================
// Function name:  di_SortModeTable
//
// Description:    Examines all Mode entrys in the Mode Table,	and places them
//				   in ascending order by BPP first, then width, then height, then refresh.
//
// Information:    Several of the routines in this driver require a sorted mode list.
//
// Return:         VOID.
//=======================================================================================

void di_SortModeTable( H3_VIDEO_FREQUENCIES * VideoFreq, int nummodes )
{
  int i,j;


  for (i = 0; i < nummodes-1; i++)
  {
    for (j = i+1; j < nummodes; j++)
    {
      if ( VideoFreq[i].BitsPerPel > VideoFreq[j].BitsPerPel )
      {
        di_SwapFrequencyStructs( &VideoFreq[i], &VideoFreq[j] );
      }
    }
  }

  for (i = 0; i < nummodes-1; i++)
  {
    for (j = i+1; j < nummodes; j++)
    {
      if ( VideoFreq[i].BitsPerPel == VideoFreq[j].BitsPerPel &&
           VideoFreq[i].ScreenWidth > VideoFreq[j].ScreenWidth )
      {
        di_SwapFrequencyStructs( &VideoFreq[i], &VideoFreq[j] );
      }
    }
  }

  for (i = 0; i < nummodes-1; i++)
  {
    for (j = i+1; j < nummodes; j++)
    {
      if ( VideoFreq[i].BitsPerPel == VideoFreq[j].BitsPerPel &&
           VideoFreq[i].ScreenWidth == VideoFreq[j].ScreenWidth && 
           VideoFreq[i].ScreenHeight > VideoFreq[j].ScreenHeight )
      {
        di_SwapFrequencyStructs( &VideoFreq[i], &VideoFreq[j] );
      }
    }
  }

  for (i = 0; i < nummodes-1; i++)
  {
    for (j = i+1; j < nummodes; j++)
    {
      if ( VideoFreq[i].BitsPerPel == VideoFreq[j].BitsPerPel &&
           VideoFreq[i].ScreenWidth == VideoFreq[j].ScreenWidth && 
           VideoFreq[i].ScreenHeight == VideoFreq[j].ScreenHeight &&
           VideoFreq[i].ScreenFrequency > VideoFreq[j].ScreenFrequency )
      {
        di_SwapFrequencyStructs( &VideoFreq[i], &VideoFreq[j] );
      }
    }
  }
}

//=======================================================================================
// Function name:  di_SortModeTableEx
//
// Description:    move desktop modes to start of mode list array
//                 move low res (game) modes to end of mode list array
//                 sort desktop and low res modes separately
//
// Information:    
//
// Return:         
//=======================================================================================

//#define DUMP_DMT_LIST

void di_SortModeTableEx( H3_VIDEO_FREQUENCIES * VideoFreq, int nummodes )
{
  int i,j;


#if DBG && defined(DUMP_DMT_LIST)
  VideoDebugPrint((0, "di_SortModeTableEx - unsorted mode list\n"));
  for (j = 0; j < nummodes; j++)
  {
    VideoDebugPrint((0, "  %ld x %ld x %ld @ %ldHz\n",
             VideoFreq[j].ScreenWidth,
             VideoFreq[j].ScreenHeight,
             VideoFreq[j].BitsPerPel,
             VideoFreq[j].ScreenFrequency));
  }
#endif

  // move all desktop modes to start of array
  // and all low res (game) modes to end of array
  i = 0;
  j = nummodes-1;

  // skip any low res modes already at the end of the array
  while ((j >= 0) && (VideoFreq[j].ScreenHeight < 480))
    j--;

  while (i < j)
  {
    if (VideoFreq[i].ScreenHeight < 480)
    {
      di_SwapFrequencyStructs(&VideoFreq[i], &VideoFreq[j]);
      j--;
      // skip any low res modes already at the end of the array
      while ((j >= i) && (VideoFreq[j].ScreenHeight < 480))
        j--;
    }
    else
    {
      i++;
    }
  }
  // i is now the index of the last desktop mode
  // so increment it once to get the count of desktop modes
  i++;

#if DBG && defined(DUMP_DMT_LIST)
  VideoDebugPrint((0, "di_SortModeTableEx - unsorted desktop mode list\n"));
  for (j = 0; j < i; j++)
  {
    VideoDebugPrint((0, "  %ld x %ld x %ld @ %ldHz\n",
             VideoFreq[j].ScreenWidth,
             VideoFreq[j].ScreenHeight,
             VideoFreq[j].BitsPerPel,
             VideoFreq[j].ScreenFrequency));
  }
  VideoDebugPrint((0, "di_SortModeTableEx - unsorted low res mode list\n"));
  for (j = i; j < nummodes; j++)
  {
    VideoDebugPrint((0, "  %ld x %ld x %ld @ %ldHz\n",
             VideoFreq[j].ScreenWidth,
             VideoFreq[j].ScreenHeight,
             VideoFreq[j].BitsPerPel,
             VideoFreq[j].ScreenFrequency));
  }
#endif

  // sort desktop modes
  di_SortModeTable(&VideoFreq[0], i);

  // sort low res modes
  di_SortModeTable(&VideoFreq[i], nummodes-i);

#if DBG && defined(DUMP_DMT_LIST)
  VideoDebugPrint((0, "di_SortModeTableEx - sorted mode list\n"));
  for (j = 0; j < nummodes; j++)
  {
    VideoDebugPrint((0, "  %ld x %ld x %ld @ %ldHz\n",
             VideoFreq[j].ScreenWidth,
             VideoFreq[j].ScreenHeight,
             VideoFreq[j].BitsPerPel,
             VideoFreq[j].ScreenFrequency));
  }
#endif

  for (i = 0; i < nummodes; i++)
    VideoFreq[i].ModeIndex = i;
}

//=======================================================================================
// Function name:  di_FillModeTable
//
// Description:    Reads in all TIMINGS mode entrys in the registry,
// 				   and places them in the Mode Info Table ( unsorted ).
// Information:    
//
// Return:         VOID.
//=======================================================================================
void di_FillModeTable( PHW_DEVICE_EXTENSION HwDeviceExtension )
{
ULONG index = 0;
BOOLEAN UseGTF_AllModes = FALSE;
H3_VIDEO_FREQUENCIES * VideoFreq;


	di_GetRegistryGTFOverride( HwDeviceExtension, &UseGTF_AllModes ); // Force use of GTF for all modes?

	HwDeviceExtension->RegistryForceGTF = UseGTF_AllModes;

	VideoFreq = HwDeviceExtension->RegistryVideoFreqs;

	if ( HwDeviceExtension->RegistryNumModes == 0 )
	{
		di_BuildDefaultMode( VideoFreq );
		HwDeviceExtension->RegistryNumModes = 1;
		return;
	}

#ifndef MS_VIEW
	while ( index < HwDeviceExtension->RegistryNumModes )
	{
		di_EnumRegistryMode( HwDeviceExtension, VideoFreq, index );

		VideoFreq->ModeIndex = index;

		VideoFreq++;
		index++;
	}
#else
	di_EnumRegistryMode( HwDeviceExtension, VideoFreq );
#endif //MS_VIEW

	di_SortModeTableEx( HwDeviceExtension->RegistryVideoFreqs, (int)HwDeviceExtension->RegistryNumModes );

}

// #pragma optimize("",on)

#endif // DMT_ENABLED
