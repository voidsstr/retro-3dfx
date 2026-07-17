#include <stdio.h>
#include <conio.h>
#include "flash.h"
#include "vidin.h"

#define CONFIG_ADDRESS_PORT 0xCF8
#define CONFIG_DATA_PORT    0xCFC
#define CONFIG_ADDRESS_ENABLE_BIT 0x80000000UL
#define CONFIG_MAPPING_ENABLE_BYTE 0x80
#define CONFIG_MAPPING_DISABLE_BYTE 0x00
#define CONFIG_MAPPING_OFFSET    0xC000

#if 0
const PciRegister PCI_VENDOR_ID       = { 0x0,  2, READ_ONLY };
const PciRegister PCI_DEVICE_ID       = { 0x2,  2, READ_ONLY };
const PciRegister PCI_COMMAND         = { 0x4,  2, READ_WRITE };
const PciRegister PCI_STATUS          = { 0x6,  2, READ_WRITE };
const PciRegister PCI_REVISION_ID     = { 0x8,  1, READ_ONLY };
const PciRegister PCI_CLASS_CODE      = { 0x9,  3, READ_ONLY };
const PciRegister PCI_CACHE_LINE_SIZE = { 0xC,  1, READ_WRITE };
const PciRegister PCI_LATENCY_TIMER   = { 0xD,  1, READ_WRITE };
const PciRegister PCI_HEADER_TYPE     = { 0xE,  1, READ_ONLY };
const PciRegister PCI_BIST            = { 0xF,  1, READ_WRITE };
const PciRegister PCI_BASE_ADDRESS_0  = { 0x10, 4, READ_WRITE };
const PciRegister PCI_BASE_ADDRESS_1  = { 0x14, 4, READ_WRITE };
const PciRegister PCI_BASE_ADDRESS_2  = { 0x18, 4, READ_WRITE };
const PciRegister PCI_BASE_ADDRESS_3  = { 0x1C, 4, READ_WRITE };
const PciRegister PCI_BASE_ADDRESS_4  = { 0x20, 4, READ_WRITE };
const PciRegister PCI_BASE_ADDRESS_5  = { 0x24, 4, READ_WRITE };
const PciRegister PCI_BASE_ADDRESS_6  = { 0x28, 4, READ_WRITE };
const PciRegister PCI_BASE_ADDRESS_7  = { 0x2c, 4, READ_WRITE };
const PciRegister PCI_BASE_ADDRESS_8  = { 0x30, 4, READ_WRITE };
#endif
const PciRegister PCI_SUBVENDOR_ID    = { 0x2c, 2, READ_WRITE };
const PciRegister PCI_SUBDEVICE_ID    = { 0x2e, 2, READ_WRITE };


static UINT8 configMechanism, busDetected;

UINT32
_pciCreateConfigAddress (UINT32 bus_number, UINT32 device_number,  
                         UINT32 function_number, UINT32 register_offset)
{ 
	UINT32 retval = CONFIG_ADDRESS_ENABLE_BIT;
  
	retval |= ( bus_number & 0xFF ) << 16;
	retval |= ( device_number & 0x1F ) << 11;
	retval |= ( function_number & 0x7 ) << 8;
	retval |= ( register_offset & 0xFC );
	return retval;
}

UINT16 _pciCreateConfigSpaceMapping (UINT32 device_number, UINT32 register_offset)
{

  UINT16 retval = 0;

  retval |= (UINT16) (( device_number & 0xFF ) << 8);
  retval |= (UINT16) ( register_offset & 0xFC );

  retval += CONFIG_MAPPING_OFFSET;
  return retval;
} /* _pciCreateConfigSpaceMapping */

UINT32 _pciFetchRegister( UINT32 offset, UINT32 size_in_bytes, 
                   UINT32 device_number, UINT32 config_mechanism  )
{ 
	UINT32 retval;
	UINT32 slot, bus;
  
	bus  = device_number >> 5;
	slot = device_number & 0x1f;
  
	if ( config_mechanism == 1 ) 
	{
		_outpd (CONFIG_ADDRESS_PORT,
			_pciCreateConfigAddress (bus, slot, 0, offset));
		retval = _inpd (CONFIG_DATA_PORT);
		retval >>= 8 * (offset & 0x3);
	}
	else
	{                      /* config mechanism 2 */
		outp (CONFIG_ADDRESS_PORT, CONFIG_MAPPING_ENABLE_BYTE );
		retval = _inpd (_pciCreateConfigSpaceMapping (device_number, offset)); 
		retval >>= 8 * ( offset & 0x3 );
		outp (CONFIG_ADDRESS_PORT, CONFIG_MAPPING_DISABLE_BYTE );
	}
  
	switch (size_in_bytes) 
	{
		case 1:
			retval &= 0xFF;
			break;
		case 2:
			retval &= 0xFFFF;
			break;
		default:                      /* 4 bytes */
			break;
	}
		return retval;
} /* _pciFetchRegister */


BOOL PCIEnumerate (void)
{
	int deviceNumber;
	int regVal, slot, bus;

  	for (deviceNumber = 0; deviceNumber < MAX_PCI_DEVICES; deviceNumber++) 
	{        
		bus =   deviceNumber >> 5;
		slot = (deviceNumber & 0x1f);

		_outpd (CONFIG_ADDRESS_PORT, 
			_pciCreateConfigAddress (bus, slot, 0, 0x0));
		regVal = _inpd (CONFIG_DATA_PORT);

		if ((regVal & 0xFFFF) != 0xFFFF)
		{
			busDetected = TRUE;
			configMechanism = 1;

            if((regVal & 0xffff) == 0x121a)
			{
			   wNumBoards++;
			   sBoardsDesc[wNumBoards-1].wSlotNum = slot;
			   sBoardsDesc[wNumBoards-1].wBusID = bus;
			   sBoardsDesc[wNumBoards-1].wVendorID = regVal & 0xffff;
			   sBoardsDesc[wNumBoards-1].wDeviceID = (WORD) ((regVal & 0xffff0000) >> 16);

               regVal = _pciFetchRegister (PCI_SUBDEVICE_ID.regAddress,
                                PCI_SUBDEVICE_ID.sizeInBytes, 
                                deviceNumber, configMechanism);

	           sBoardsDesc[wNumBoards-1].wSubDeviceID = regVal & 0xFFFF;

	           regVal = _pciFetchRegister (PCI_SUBVENDOR_ID.regAddress,
                                PCI_SUBVENDOR_ID.sizeInBytes, 
                                deviceNumber, configMechanism);
	           sBoardsDesc[wNumBoards-1].wSubVendorID = regVal & 0xFFFF;
			}
		}
	}

  	if(wNumBoards > 0)
	{
	   return (TRUE);
	}
	else
	{
	   return(FALSE);
	}
}
