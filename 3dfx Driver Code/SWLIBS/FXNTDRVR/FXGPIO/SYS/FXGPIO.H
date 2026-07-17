// genport.h    Include file for Generic Port I/O Example Driver
//
// Robert R. Howell         January 6, 1993
// Robert B. Nelson (MS)    January 12, 1993
//      Modified comments prior to posting to Compuserve
//      Changed Wkd* to Gpd*
// Robert B. Nelson (MS)    March 1, 1993
//      Cleanup and bug fixes for Beta 2
//
#include <ntddk.h>
#include <string.h>
#include <devioctl.h>
#include "gpioctl.h"        // Get IOCTL interface definitions

/* Default base port, and # of ports */
#define BASE_PORT       0xCF8
#define NUMBER_PORTS        8

// NT device name
#define GPD_DEVICE_NAME L"\\Device\\Gpd0"

// File system device name.   When you execute a CreateFile call to open the
// device, use "\\.\GpdDev", or, given C's conversion of \\ to \, use
// "\\\\.\\GpdDev"

#define DOS_DEVICE_NAME L"\\DosDevices\\GpdDev"


/********************* function prototypes ***********************************/
//

NTSTATUS    DriverEntry(       IN  PDRIVER_OBJECT DriverObject,
                               IN  PUNICODE_STRING RegistryPath );

#if 0
NTSTATUS    GpdCreateDevice(   IN  PWSTR szPrototypeName,
                               IN  DEVICE_TYPE DeviceType,
                               IN  PDRIVER_OBJECT DriverObject,
                               OUT PDEVICE_OBJECT *ppDevObj     );
#endif

NTSTATUS    GpdDispatch(       IN  PDEVICE_OBJECT pDO,
                               IN  PIRP pIrp                    );

NTSTATUS    GpdIoctlReadPort(  IN  PDEVICE_OBJECT pDO,
                               IN  PIRP pIrp,
                               IN  PIO_STACK_LOCATION IrpStack,
                               IN  ULONG IoctlCode              );

NTSTATUS    GpdIoctlWritePort( IN  PDEVICE_OBJECT pDO,
                               IN  PIRP pIrp,
                               IN  PIO_STACK_LOCATION IrpStack,
                               IN  ULONG IoctlCode              );

VOID        GpdUnload(         IN  PDRIVER_OBJECT DriverObject );

