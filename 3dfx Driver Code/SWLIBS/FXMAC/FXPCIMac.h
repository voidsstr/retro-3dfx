#ifndef __FX_PCIMAC_H__
#define __FX_PCIMAC_H__

#include <limits.h>

#include <3dfx.h>
#include <FxPci.h>
#include <DriverServices.h>

/* The maximum # of pci devices that we can keep track of here.
 * This is more a limitation of the current mutex-ing scheme
 * not the mac pci bus.
 */
#define kMaxMacPCIDeviceCount (sizeof(long) * CHAR_BIT)
#define kMaxPciBaseAddress		9

typedef struct {
	/* true if pciFindCardMulti sucessfully found a device system.
	 * The remainder of the fields are only valid if this is true.
	 */
	Boolean			found;
	
	/* Copy of the mac device id for a given mapped device. This
	 * id includes all of the base addresses and i/o ports 
	 * associated w/ the actual device.
	 */
	RegEntryID	regEntryId;
	
	/* List of the mapped pci physical addresses for a given device.
	 * 
	 * NB: While we have constants/storage for 0-8 addresses some of
	 * these are listed as being reserved in the original pci spec.
	 */
	FxU32				pciPhysAddr[kMaxPciBaseAddress];
	
	/* Legacy support for pc i/o port crap because some writes need
	 * to go to specific spaces, but the i/o port code often confuses
	 * which space it is actually going to. These values are the 
	 * indicies into the pciPhysAddr array above for the base address
	 * to offset when passed to the ExpMgrIOXXX routines.
	 */
	FxU32				cmdBaseIndex;
	FxU32				ioPortBaseIndex;
} MacPciDevice;

extern MacPciDevice gMacPciDeviceList[kMaxMacPCIDeviceCount];

FX_ENTRY FxBool FX_CALL
macSetPciBaseAddr(FxU32 deviceId,
									FxU32 cmdBaseIndex,
									FxU32 ioPortBaseIndex);

FX_ENTRY FxU8 FX_CALL 
macIOReadByte(FxU32 ioBaseAddr, FxU16 ioAddr);

FX_ENTRY void FX_CALL 
macIOWriteByte(FxU32 ioBaseAddr, FxU16 ioAddr, FxU8 writeVal);

FX_ENTRY FxU8 FX_CALL 
macIOReadWord(FxU32 ioBaseAddr, FxU16 ioAddr);

FX_ENTRY void FX_CALL 
macIOWriteWord(FxU32 ioBaseAddr, FxU16 ioAddr, FxU16 writeVal);

FX_ENTRY FxU32 FX_CALL 
macIOReadLong(FxU32 ioBaseAddr, FxU16 ioAddr);

FX_ENTRY void FX_CALL 
macIOWriteLong(FxU32 ioBaseAddr, FxU16 ioAddr, FxU32 writeVal);

#endif /* __FX_PCIMAC_H__ */