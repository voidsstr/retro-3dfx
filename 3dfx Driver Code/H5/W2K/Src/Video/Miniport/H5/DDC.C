/*++

Copyright (c) 1997	Microsoft Corporation
Copyright (c) 1999  3dfx Interactive, Inc.

Module Name:

	ddc.c

Abstract:

	This module contains the code that support DDC querying..

Environment:

	Kernel mode

Revision History:

** $Revision: 8$
** $Date: 10/11/00 8:46:30 PM$
**
** $Log: 
**  8    3dfx      1.4.1.0.1.1 10/11/00 Brent           Forced check in to enforce
**       branching.
**  7    3dfx      1.4.1.0.1.0 07/21/00 Dan O'Connel    Change driver so the EDID
**       is read only one time, and this allows us to patch the EDID in some cases
**       before we hand it off to Win2K.
**       Sync. up with other changes made in Win9x driver.
**  6    3dfx      1.4.1.0     05/18/00 Dan O'Connel    Major clean up of DFP
**       support code. Restructures DFP code to simplify interfaces.
** 
**       Also let Win2K know that TvOut is a child device, and correct handling of
**       BIOS shared scratch registers to work on a multi-monitor system.
** 
**  5    3dfx      1.4         02/01/00 Russ Lind       changes to use
**       H3_MAPPEDADDRESS_INDICES
**  4    3dfx      1.3         12/22/99 Dan O'Connel    Change GetDdcInformation
**       and GetDfpEdidInformation to use the public I2C interfaces defined in
**       di_i2c.h rather than exposing interfaces in ds_i2c.h that should not be
**       public.
**  3    3dfx      1.2         12/07/99 Dan O'Connel    Change driver to not
**       attempt to detect a DFP if running on a Voodoo3 card.  No Voodoo3 cards
**       released to the public had DFP support, and the driver previously was
**       detecting imaginary DFPs.
**  2    3dfx      1.1         11/22/99 Dan O'Connel    Enumerate Digital Flat
**       Panel device as a Monitor for Win2K and unload EDID information from the
**       DFP.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $

--*/
#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "h3.h"
#include "cmdcnst.h"
#include "di_i2c.h"
#include "dfp.h"

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
WriteClockLineDDC(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	UCHAR ucData
	)
{
    i2c_setscl(HwDeviceExtension, HwDeviceExtension->ddcKey, ucData, 1/*tNODELAY*/);
}

VOID
WriteDataLineDDC(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	UCHAR ucData
	)
{
    i2c_setsda(HwDeviceExtension, HwDeviceExtension->ddcKey, ucData,  1/*tNODELAY*/);
}

BOOLEAN
ReadClockLineDDC(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	)
{
    return((BOOLEAN)i2c_getscl(HwDeviceExtension, HwDeviceExtension->ddcKey));
}


BOOLEAN
ReadDataLineDDC(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	)
{
    return((BOOLEAN)i2c_getsda(HwDeviceExtension, HwDeviceExtension->ddcKey));
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
    I2CKEY key;

	//
	//  Select DDC path to the Monitor
	//
    key = i2c_getaccess(HwDeviceExtension, I2C_MONITORDDC, I2C_NORMALSPEED);

    HwDeviceExtension->ddcKey = key;

	//
	// Get DDC Information if all the registers are setup properly.
	//

	i2c.WriteClockLine = WriteClockLineDDC;
	i2c.WriteDataLine  = WriteDataLineDDC;
	i2c.ReadClockLine  = ReadClockLineDDC;
	i2c.ReadDataLine   = ReadDataLineDDC;
	i2c.WaitVsync      = WaitForVsyncActive;

	i2c.Size = sizeof(I2C_FNC_TABLE);

	bRet = VideoPortDDCMonitorHelper(HwDeviceExtension,
	                                 &i2c,
	                                 pQueryBuffer,
	                                 BufferSize);

	//
	// Release DDC path to the Monitor
	//
    i2c_endaccess(HwDeviceExtension, key);

	return (bRet);

}

BOOLEAN
GetDfpEdidInformation(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PUCHAR pQueryBuffer,
	ULONG BufferSize
	)

/*++

Routine Description:

	Reads the basic EDID structure from the DFP using I2C.

Arguments:

	HwDeviceExtension - Points to per-adapter device extension.

	pQueryBuffer 	  - Buffer where information will be stored.

	BufferSize		  - Size of the buffer to fill.

Return Value:

	Whether the call succeeded or not (TRUE or FALSE).

--*/

{
	BOOLEAN       bRet = FALSE;
	I2C_FNC_TABLE i2c;
    I2CKEY key;

    // if we don't think a panel is present don't let MS run because it returns the wrong result.
    if (!isPanelPresent(HwDeviceExtension))
        return(FALSE);

#if 0
	//
	// Select I2C path to the DFP
	//
    key = i2c_getaccess(HwDeviceExtension, I2C_FLATPANELDDC, I2C_NORMALSPEED);

    HwDeviceExtension->ddcKey = key;

	//
	// Get DDC Information if all the registers are setup properly.
	//

	i2c.WriteClockLine = WriteClockLineDDC;
	i2c.WriteDataLine  = WriteDataLineDDC;
	i2c.ReadClockLine  = ReadClockLineDDC;
	i2c.ReadDataLine   = ReadDataLineDDC;
	i2c.WaitVsync      = WaitForVsyncActive;

	i2c.Size = sizeof(I2C_FNC_TABLE);

	bRet = VideoPortDDCMonitorHelper(HwDeviceExtension,
	                                 &i2c,
	                                 pQueryBuffer,
	                                 BufferSize);

	//
	// Release DDC path to the DFP
	//
    i2c_endaccess(HwDeviceExtension, key);
#else
    bRet = (panelEDID(HwDeviceExtension, pQueryBuffer, BufferSize) >= 0);
#endif
	return (bRet);

}

#endif // (_WIN32_WINNT >= 0x500)
