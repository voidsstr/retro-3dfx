/*
 * Copyright (c) 1996, 3Dfx Interactive, Inc.
 * All Rights Reserved.
 */

#include <ntddk.h>

/*
 *  16 Busses ( of possible 256, I am making the
 *              assumption that busses are numbered
 *              in increasing order and that no
 *              PC will have more than 16 busses )
 */
#define PCI_MAX_BUSSES	16

void           P6FixBrokenIntelChipset( void );
static LONG    FindPCIDevice( USHORT Match_VendorID, USHORT Match_DeviceID, ULONG *pBus, ULONG *pSlot );
static UCHAR   GetConfigByte( ULONG bus, ULONG slot, ULONG index);
static VOID    SetConfigByte( ULONG bus, ULONG slot, ULONG index, UCHAR value);
LONG           P6_IsFixDisabledInRegistry( void );


WCHAR          registryPathBuffer[] = L"\\Registry\\Machine\\SOFTWARE\\3Dfx Interactive\\Shared";
UNICODE_STRING registryPathUnicodeString;

WCHAR          registryValueNameBuffer[] = L"DisableP6WCfix";
UNICODE_STRING registryValueNameUnicodeString;

/******************************************************************************
 *  P6FixBrokenIntelChipset
 *
 *       On an Intel PCI chipset combination of Host bridge ID 0x1237 and
 *       ISA bridge IDE 0x7000:
 *       
 *       There are two bits, one in the PCI bridge, and one in the ISA bridge,
 *       that when set a certain way, cause P6 systems to lock up when access
 *       to WC memory (write combine) on PCI is performed simultaneously with
 *       an ISA access.  Here is the table of bits:
 *
 *       PCI     ISA     works?
 *       ---     ---     ------
 *       0       0       Yes
 *       0       1       Yes
 *       1       0       No
 *       1       1       Yes
 *
 *       This routine detects the failing case and sets the ISA bit to a 1
 *       to avoid the failure.
 *
 *	      We assume that the bridge devices we are looking for will be on
 *       PCI bus number 0.
 */
void
P6FixBrokenIntelChipset( void )
{
	ULONG    bus;
	ULONG		slot;
	UCHAR		val;

	if( (FindPCIDevice(0x8086, 0x1237, &bus, &slot) == TRUE ) && // Check PCI
		 ((GetConfigByte(bus,slot,0x53) & 0x20)  > 0 )         &&
		 (FindPCIDevice(0x8086, 0x7000, &bus, &slot) == TRUE ) && // Check ISA
		 (( (val=GetConfigByte(bus,slot,0x82)) & 0x02 ) == 0 )  )
				SetConfigByte(bus, slot, 0x82, (UCHAR) (val | 0x02));
	return;
}


/******************************************************************************
 *  FindPCIDevice
 *
 *    Search PCI bus 0 through PCI_MAX_BUSSES, from device 0 through
 *    PCI_MAX_DEVICES for the device that matches the passed vendor and
 *    device ID's.
 *
 *    RETURNS:
 *
 *       Device found:        TRUE - *pBUS gets bus number of found device
 *                                   *pSlot gets slot number of device
 *
 *       Device not found:    FALSE
 *
 *
 *    USES:  HalGetBusDataByOffset
 */
static LONG
FindPCIDevice( USHORT Match_VendorID,
               USHORT Match_DeviceID,
               ULONG *pBus,
               ULONG *pSlot ) {
	ULONG                bus;
	ULONG                device;
	USHORT               ConfigBuf[2];

	for( bus=0 ; bus < PCI_MAX_BUSSES ; bus++ ) {
	   for( device=0 ; device < PCI_MAX_DEVICES ; device++ ) {
			if( HalGetBusDataByOffset(
				   PCIConfiguration,
				   bus,
				   device,
				   (void *) ConfigBuf,
				   0,
				   sizeof(ConfigBuf) )  < sizeof(ConfigBuf) )
				continue;	/* No device at this bus/slot */
			else {	/* Found a device, is it the right one? */
				if( (ConfigBuf[0] == Match_VendorID) &&
				    (ConfigBuf[1] == Match_DeviceID) ) {
				   *pBus = bus;
					*pSlot = device;
				   return TRUE;
				}
			}
		}
	}
	return FALSE;		/* PCI device not found */
}

/******************************************************************************
 *  GetConfigByte - get a byte from the PCI config space of the PCI device.
 *
 *    Bus   = PCI bus number of the PCI device.
 *  	Slot	= PCI slot number of the PCI device.
 *		Index	= Offset of the byte to return in the PCI config space.
 *
 *  RETURNS:
 *		Byte data at the indicated slot/index.
 *
 *  USES:  HalGetBusDataByOffset
 */
static UCHAR
GetConfigByte( ULONG bus, ULONG slot, ULONG index)
{
	UCHAR		PCIConfigData;

	HalGetBusDataByOffset( PCIConfiguration,
		bus,
		slot,
		&PCIConfigData,
		index,
		1					 );

	return PCIConfigData;
}

/******************************************************************************
 *  SetConfigByte - set a byte in the PCI config space of a PCI device.
 *
 *    Bus   = PCI bus number of the PCI device.
 *  	Slot	= PCI slot number of the PCI device.
 *		Index	= Offset of the byte to return in the PCI config space.
 *		Value = Byte data to set.
 *
 *  RETURNS:
 *		Nothing
 *
 *  USES:  HalSetBusDataByOffset
 */
static VOID
SetConfigByte( ULONG bus, ULONG slot, ULONG index, UCHAR value)
{
	UCHAR		PCIConfigData;

	PCIConfigData = value;
	HalSetBusDataByOffset( PCIConfiguration,
		bus,
		slot,
		&PCIConfigData,
		index,
		1					 );
}

/*****************************************************************************
 *
 *   P6_IsFixDisabledInRegistry - Check registry to determine whether to do
 *                                P6_FixBrokenIntelChipset
 *
 *       Check the registry entry defined by the variables registryPathBuffer
 *       and registryValueNameBuffer (currently = \Registry\Machine\SOFTWARE\
 *       3Dfx Interactive\Shared\DisableP6WCfix which translates to
 *       \HK_LOCAL_MACHINE\SOFTWARE\3Dfx Interactive\Shared\DisableP6WCfix
 *       on NT).
 *
 *       If there is no such registry entry,
 *       		return 0L,                           // Don't disable the fix
 *       otherwise
 *       		return (Value of registry entry).
 */
LONG
P6_IsFixDisabledInRegistry( void )
{
	HANDLE  			keyhandle;
	ULONG				ResultLength;
	OBJECT_ATTRIBUTES		objectAttributes;
	KEY_VALUE_PARTIAL_INFORMATION	KeyInfo;
	LONG				retval;

	RtlInitUnicodeString (&registryPathUnicodeString,
                          registryPathBuffer);

	RtlInitUnicodeString (&registryValueNameUnicodeString,
                          registryValueNameBuffer);

	InitializeObjectAttributes (&objectAttributes,
                               &registryPathUnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               (HANDLE) NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

	retval = 0L;			// Default = Don't disable fix.

	// Open Key, Query Key value, close key
	if( ZwOpenKey(&keyhandle,KEY_READ,&objectAttributes) == STATUS_SUCCESS ) {
		if( ZwQueryValueKey( keyhandle,
                    &registryValueNameUnicodeString,
                    KeyValuePartialInformation,
                    &KeyInfo,
                    sizeof( KeyInfo ),
                    &ResultLength )  == STATUS_SUCCESS )
			retval = *(LONG *) KeyInfo.Data;
		ZwClose( keyhandle );		// Close Key
   }

	return retval;
}
