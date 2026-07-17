/*
 * Copyright (c) 1997, 3Dfx Interactive, Inc.
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

static LONG FindPCIDevice( USHORT Match_VendorID, USHORT Match_DeviceID, ULONG *pBus, ULONG *pSlot );
static UCHAR GetConfigByte( ULONG bus, ULONG slot, ULONG index);
static ULONG GetConfigDword( ULONG bus, ULONG slot, ULONG index);
static VOID SetConfigByte( ULONG bus, ULONG slot, ULONG index, UCHAR value);
static VOID SetConfigDword( ULONG bus, ULONG slot, ULONG index, ULONG value);
LONG	IsNTRemapEnabledInRegistry( void );
void	RemapTheBoard(void);
void	PickRemapAddress( ULONG bus, ULONG *newaddress);
void	SetBoardAddress( ULONG bus, ULONG slot, ULONG address );
LONG  GetForceAddress( void );
ULONG IsAddressConflict( ULONG bus, ULONG slot, ULONG s3slot );


//-------------------------------------

WCHAR          registryPathBuffer[] = L"\\Registry\\Machine\\SOFTWARE\\3Dfx Interactive\\NTRemap";
UNICODE_STRING registryPathUnicodeString;

WCHAR          registryValueNameBuffer[] = L"EnableNTRemap";
UNICODE_STRING registryValueNameUnicodeString;

//-------------------------------------

WCHAR          registryPathBuffer_ForceAddress[] = L"\\Registry\\Machine\\SOFTWARE\\3Dfx Interactive\\NTRemap";
UNICODE_STRING registryPathUnicodeString_ForceAddress;

WCHAR          registryValueNameBuffer_ForceAddress[] = L"ForceAddress";
UNICODE_STRING registryValueNameUnicodeString_ForceAddress;


/******************************************************************************
 *  RemapTheBoard
 *
 */
void
RemapTheBoard( void )
{
	ULONG		bus;       // PCI bus number of the SST1
	ULONG		slot;      // PCI slot number of the SST1
	ULONG		s3bus;     // PCI bus number of the S3 card
	ULONG		s3slot;    // PCI slot number of the S3 card
	ULONG		forceaddress;
	ULONG		newaddress;

	if( FindPCIDevice(0x121a, 0x0001, &bus, &slot) == TRUE )   // Found SST1
	    if( (forceaddress = GetForceAddress())  ==  0L )
	    {
	        if( (FindPCIDevice(0x5333, 0x88f0, &s3bus, &s3slot) == TRUE )  // If S3 868/968
	            || (FindPCIDevice(0x5333, 0x8880, &s3bus, &s3slot) == TRUE)   )
	            if( (bus==s3bus) && IsAddressConflict( bus, slot, s3slot ) )
	            {            
	                PickRemapAddress(bus, &newaddress);
	                if( newaddress )
					        SetBoardAddress( bus, slot, newaddress );
	            }
	    }
	    else	// Force address
	        SetBoardAddress( bus, slot, forceaddress );

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
 *  GetConfigDword - get a Dword from the PCI config space of the PCI device.
 *
 *  	Slot	= PCI slot number of the PCI device.
 *		Index	= Offset of the Dword to return in the PCI config space.
 *
 *  RETURNS:
 *		Dword data at the indicated slot/index.
 *
 *  USES:  HalGetBusDataByOffset
 */
static ULONG
GetConfigDword( ULONG bus, ULONG slot, ULONG index)
{
	ULONG		PCIConfigData;

	HalGetBusDataByOffset( PCIConfiguration,
		bus,
		slot,
		&PCIConfigData,
		index,
		4					 );

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


/******************************************************************************
 *  SetConfigDword - set a Dword in the PCI config space of a PCI device.
 *
 *  	Slot	= PCI slot number of the PCI device.
 *		Index	= Offset of the Dword to return in the PCI config space.
 *		Value = Dword data to set.
 *
 *  RETURNS:
 *		Nothing
 *
 *  USES:  HalSetBusDataByOffset
 */
static VOID
SetConfigDword( ULONG bus, ULONG slot, ULONG index, ULONG value)
{
	ULONG		PCIConfigData;

	PCIConfigData = value;
	HalSetBusDataByOffset( PCIConfiguration,
		bus,
		slot,
		&PCIConfigData,
		index,
		4					 );
}

/*****************************************************************************
 *
 *   IsNTRemapEnabledInRegistry - Check registry to determine whether to do
 *                                RemapTheBoard().
 *
 *       Check the registry entry defined by the variables registryPathBuffer
 *       and registryValueNameBuffer (currently = \Registry\Machine\SOFTWARE\
 *       3Dfx Interactive\NTREMAP\EnableNTRemap which translates to
 *       \HK_LOCAL_MACHINE\SOFTWARE\3Dfx Interactive\NTREMAP\EnableNTRemap
 *       on NT).
 *
 *       If there is no such registry entry,
 *       		return 0L,                           // Don't enable NT remap
 *       otherwise
 *       		return (Value of registry entry).
 */
LONG
IsNTRemapEnabledInRegistry( void )
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

	retval = 0L;			// Default = Don't remap.

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


/*****************************************************************************
 *
 *       Check the registry entry \Registry\Machine\SOFTWARE\
 *       3Dfx Interactive\NTREMAP\ForceAddress which translates to
 *       \HK_LOCAL_MACHINE\SOFTWARE\3Dfx Interactive\NTREMAP\ForceAddress
 *       on NT).
 *
 *       If there is no such registry entry,
 *       		return 0L,                           // Don't force remap
 *       otherwise
 *       		return (Value of registry entry).
 *
 */
LONG
GetForceAddress( void )
{
	HANDLE  			keyhandle;
	ULONG				ResultLength;
	OBJECT_ATTRIBUTES		objectAttributes;
	KEY_VALUE_PARTIAL_INFORMATION	KeyInfo;
	LONG				retval;

	RtlInitUnicodeString (&registryPathUnicodeString_ForceAddress,
                          registryPathBuffer_ForceAddress);

	RtlInitUnicodeString (&registryValueNameUnicodeString_ForceAddress,
                          registryValueNameBuffer_ForceAddress);

	InitializeObjectAttributes (&objectAttributes,
                               &registryPathUnicodeString_ForceAddress,
                               OBJ_CASE_INSENSITIVE,
                               (HANDLE) NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

	retval = 0L;			// Default 0L == Don't force remap.

	// Open Key, Query Key value, close key
	if( ZwOpenKey(&keyhandle,KEY_READ,&objectAttributes) == STATUS_SUCCESS ) {
		if( ZwQueryValueKey( keyhandle,
                    &registryValueNameUnicodeString_ForceAddress,
                    KeyValuePartialInformation,
                    &KeyInfo,
                    sizeof( KeyInfo ),
                    &ResultLength )  == STATUS_SUCCESS )
			retval = *(LONG *) KeyInfo.Data;
		ZwClose( keyhandle );		// Close Key
   }

	return retval;
}


/*****************************************************************************
 *	SetBoardAddress - Remap the SST1 board to the given address
 *
 */
void
SetBoardAddress( ULONG bus, ULONG slot, ULONG address )
{
	ULONG oldrange;
	ULONG newrange;

	// We align newrange to size of SST1 address range before setting it.

	oldrange = GetConfigDword(bus, slot, 0x10);
	newrange = (oldrange & 0x0000000fL) | ( address & 0xff000000L );
	SetConfigDword(bus, slot, 0x10, newrange );
}


/*****************************************************************************
 *	IsConflict - Check if there is an address conflict between S3 and SST1
 *
 */
ULONG
IsAddressConflict( ULONG bus, ULONG slot, ULONG s3slot )
{
	ULONG sst1range;
	ULONG s3range;

	sst1range = GetConfigDword( bus, slot, 0x10) & 0xfffffff0L;
	s3range = GetConfigDword( bus, s3slot, 0x10) & 0xfffffff0L;

	if( (sst1range - s3range) < 0x04000000LU )
	    return 1L;    // Conflict
	else
	    return 0L;    // No conflict
}

void
PickRemapAddress( ULONG bus, ULONG *newaddress )
{
	ULONG                device;
	ULONG                lowaddress = 0xff000000L;
	ULONG                baseaddress[6];
	USHORT               ConfigBuf[2];
	ULONG                tmp;
	USHORT               i;

	// Find lowest address >= 0xE0000000
	for( device=0 ; device < PCI_MAX_DEVICES ; device++ ) {
		if( HalGetBusDataByOffset(
				PCIConfiguration,
				bus,
				device,
				(void *) ConfigBuf,
				0,
				sizeof(ConfigBuf) )  < sizeof(ConfigBuf) )
			continue;	/* No device at this bus/slot */
		else {
			if( (ConfigBuf[0] != 0xffff ) &&
			    (ConfigBuf[1] != 0xffff ) ) {
	              if( HalGetBusDataByOffset(
	                  PCIConfiguration,
	                  bus,
	                  device,
	                  (void *) baseaddress,
	                  0x10,
	                  sizeof(baseaddress) )  < sizeof(baseaddress) )
	                      continue;
						else {
							for( i = 0 ;
							     i < sizeof(baseaddress)/sizeof(baseaddress[0]);
								  i++ ) {
									tmp=baseaddress[i];
									if( (tmp == 0L) || (tmp & 0x7) )
										continue;
									tmp &= 0xfffffff0L;
									if( (tmp >= 0xf0000000L) && (tmp < lowaddress) )
										lowaddress = tmp;
							}
						}
			}
		}
	}

	/* Remap the 3Dfx board below the lowest existing address,
	 * but if we go too low we may conflict with the OS so make the user
	 * force a remap through the registry (by returning 0).
    *
    * Align newaddress to size of SST1 address range.
	 */
	if( lowaddress >= 0xf0000000L )
		*newaddress = (lowaddress - 0x02000000L) & 0xff000000L;
	else
		*newaddress = 0L;

	return;
}
