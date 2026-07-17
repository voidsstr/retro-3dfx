/*++

Copyright (c) 1997	Microsoft Corporation

Module Name:

	ddc.c

Abstract:

	This module contains the code that support DDC querying..

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
#include "cmdcnst.h"

#if (_WIN32_WINNT >= 0x500)

#if 0
#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,GetDdcInformation)
#endif
#endif

/****************************************************************
;		DDC register
;
;		Controls the individual toggling of bits in vidSerialParallelPort
;		to produce clock and data pulses, and in the end provides a delay.
;
; VidSerialParallelPort is defined as follows:
;
;	   ...	22	21	20	19 ...	 SCW = CLK	Write
; --------|---|---|---|---|--- 	 SDW = DATA Write
;	   ...|SDR|SCR|SDW|SCW|... 	 SCR = CLK	Read
; ---------------------------- 	 SDR = DATA Read
;
;		Input:	
;				Using MemBase0 in PHW_DEVICE_EXTENSION 
;				UCHAR ucData
;					Bit 7:2 = 0
;					Bit 1	= SDA
;					Bit 0	= SCL
;		Output:
;
;****************************************************************/


VOID
WriteClockLine(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	UCHAR ucData
	)
{
	PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
	ULONG ulPortData;

	//
	//  read the current value and reset the clock line.
	//

	ulPortData = ( RegisterMap->vidSerialParallelPort & ~BIT(19)) | ((ULONG) ucData << 19);
	RegisterMap->vidSerialParallelPort = ulPortData;
}

VOID
WriteDataLine(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	UCHAR ucData
	)
{
	PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
	ULONG ulPortData;

	//
	//	read the current value and reset the data line.
	//

	ulPortData = ( RegisterMap->vidSerialParallelPort & ~BIT(20)) | ((ULONG) ucData << 20);
	RegisterMap->vidSerialParallelPort = ulPortData;
}

BOOLEAN
ReadClockLine(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	)
{
	PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];

	return (BOOLEAN) (RegisterMap->vidSerialParallelPort >> 21) & 0x01;
}


BOOLEAN
ReadDataLine(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	)
{
	PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];

	return (BOOLEAN) (RegisterMap->vidSerialParallelPort >> 22) & 0x01;
}

VOID
WaitForVsyncActive(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	)
{
	PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
	PUCHAR pIO = (PUCHAR) HwDeviceExtension->MappedAddress[SST_IO_INDEX];
	ULONG vsyncPolarity;

	vsyncPolarity = (ULONG) VideoPortReadPortUchar((PUCHAR)(pIO + 0x0cc)) & 0x80;

	if( vsyncPolarity == 0 )
	{
	    while ((RegisterMap->status & SST_VRETRACE ) != 0) ;
	    while ((RegisterMap->status & SST_VRETRACE ) == 0) ;
	}
	else
	{
	    while ((RegisterMap->status & SST_VRETRACE ) == 0) ;
	    while ((RegisterMap->status & SST_VRETRACE ) != 0) ;
	}
}

BOOLEAN
GetDdcInformation(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PUCHAR pQueryBuffer,
	ULONG BufferSize
	)

/*++

Routine Description:

	Reads the basic EDID structure from the monitor using DDC2.

Arguments:

	HwDeviceExtension - Points to per-adapter device extension.

	pQueryBuffer 	  - Buffer where information will be stored.

	BufferSize		  - Size of the buffer to fill.

Return Value:

	Whether the call succeeded or not (TRUE or FALSE).

--*/

{
	PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
	BOOLEAN       bRet = FALSE;
	I2C_FNC_TABLE i2c;
	ULONG         ulOldVSP;

	//
	//  Enable DDC in the vidSerialParallelPort
	//

	ulOldVSP = RegisterMap->vidSerialParallelPort;
	RegisterMap->vidSerialParallelPort |= BIT(18);

	//
	// Get DDC Information if all the registers are setup properly.
	//

	i2c.WriteClockLine = WriteClockLine;
	i2c.WriteDataLine  = WriteDataLine;
	i2c.ReadClockLine  = ReadClockLine;
	i2c.ReadDataLine   = ReadDataLine;
	i2c.WaitVsync      = WaitForVsyncActive;

	i2c.Size = sizeof(I2C_FNC_TABLE);

	bRet = VideoPortDDCMonitorHelper(HwDeviceExtension,
	                                 &i2c,
	                                 pQueryBuffer,
	                                 BufferSize);

	//
	// restore the original vidSerialParallelPort register value
	//

	RegisterMap->vidSerialParallelPort = ulOldVSP;

	return (bRet);

}

#endif // (_WIN32_WINNT >= 0x500)
