// Generic Port I/O driver for NT  VERSION 1.0
//
// Adapted from NT DDK ADLIB driver
//
// Robert R. Howell                 January 8, 1993
//
// Robert B. Nelson (Microsoft)     January 12, 1993
//      Cleaned up comments
//      Enabled and tested resource reporting
//      Added code to retrieve I/O address and port count from the Registry.
//
// Robert B. Nelson (Microsoft)     March 1, 1993
//      Added support for byte, word, and long I/O.
//      Added support for MIPS.
//      Fixed resource reporting.
//
// Robert B. Nelson (Microsoft)     May 1, 1993
//      Fixed port number validation.
//
// Robert B. Nelson (Microsoft)     Oct 25, 1993
//      Fixed MIPS support.
//

#include "fxgpio.h"
#include "stdlib.h"


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

/*++

Routine Description:
    This routine is the entry point for the driver.  It is responsible
    for setting the dispatch entry points in the driver object and creating
    the device object.  Any resources such as ports, interrupts and DMA
    channels used must be reported.  A symbolic link must be created between
    the device name and an entry in \DosDevices in order to allow Win32
    applications to open the device.

Arguments:
    
    DriverObject - Pointer to driver object created by the system.

Return Value:

    STATUS_SUCCESS if the driver initialized correctly, otherwise an error
    indicating the reason for failure.

--*/

{
    NTSTATUS Status;
    PDEVICE_OBJECT DeviceObject;
    UNICODE_STRING NtDeviceName;
    UNICODE_STRING Win32DeviceName;

    // Get UNICODE name for device.

    RtlInitUnicodeString(&NtDeviceName, GPD_DEVICE_NAME);

    Status = IoCreateDevice(                             // Create it.
                    DriverObject,
                    0,
                    &NtDeviceName,
                    GPD_TYPE,
                    0,
                    FALSE,                      // Not Exclusive
                    &DeviceObject
                    );



    if ( NT_SUCCESS(Status) )
    {
        // Initialize the driver object dispatch table.
        // NT sends requests to these routines.

        DriverObject->MajorFunction[IRP_MJ_CREATE]          = GpdDispatch;
        DriverObject->MajorFunction[IRP_MJ_CLOSE]           = GpdDispatch;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = GpdDispatch;
        DriverObject->DriverUnload                          = GpdUnload;

        //
        // Create a symbolic link, e.g. a name that a Win32 app can specify
        // to open the device
        //

        RtlInitUnicodeString(&Win32DeviceName, DOS_DEVICE_NAME);

        Status = IoCreateSymbolicLink( &Win32DeviceName, &NtDeviceName );

        if (!NT_SUCCESS(Status))    // If we we couldn't create the link then
        {                           //  abort installation.
            IoDeleteDevice(DeviceObject);
        }
    }

    return Status;
}

#if 0
NTSTATUS
GpdCreateDevice(
    IN   PWSTR              PrototypeName,
    IN   DEVICE_TYPE        DeviceType,
    IN   PDRIVER_OBJECT     DriverObject,
    OUT  PDEVICE_OBJECT     *ppDevObj
    )

/*++

Routine Description:
    This routine creates the device object and the symbolic link in
    \DosDevices.
    
    Ideally a name derived from a "Prototype", with a number appended at
    the end should be used.  For simplicity, just use the fixed name defined
    in the include file.  This means that only one device can be created.
    
    A symbolic link must be created between the device name and an entry
    in \DosDevices in order to allow Win32 applications to open the device.

Arguments:

    PrototypeName - Name base, # WOULD be appended to this.

    DeviceType - Type of device to create

    DriverObject - Pointer to driver object created by the system.

    ppDevObj - Pointer to place to store pointer to created device object

Return Value:

    STATUS_SUCCESS if the device and link are created correctly, otherwise
    an error indicating the reason for failure.

--*/


{
    NTSTATUS Status;                        // Status of utility calls
    UNICODE_STRING NtDeviceName;
    UNICODE_STRING Win32DeviceName;


    // Get UNICODE name for device.

    RtlInitUnicodeString(&NtDeviceName, PrototypeName);

    Status = IoCreateDevice(                             // Create it.
                    DriverObject,
                    sizeof(LOCAL_DEVICE_INFO),
                    &NtDeviceName,
                    DeviceType,
                    0,
                    FALSE,                      // Not Exclusive
                    ppDevObj
                    );

    if (!NT_SUCCESS(Status))
        return Status;             // Give up if create failed.

    // Clear local device info memory
    RtlZeroMemory((*ppDevObj)->DeviceExtension, sizeof(LOCAL_DEVICE_INFO));

    //
    // Set up the rest of the device info
    //  These are used for IRP_MJ_READ and IRP_MJ_WRITE which we don't use
    //    
    //  (*ppDevObj)->Flags |= DO_BUFFERED_IO;
    //  (*ppDevObj)->AlignmentRequirement = FILE_BYTE_ALIGNMENT;
    //

    RtlInitUnicodeString(&Win32DeviceName, DOS_DEVICE_NAME);

    Status = IoCreateSymbolicLink( &Win32DeviceName, &NtDeviceName );

    if (!NT_SUCCESS(Status))    // If we we couldn't create the link then
    {                           //  abort installation.
        IoDeleteDevice(*ppDevObj);
    }

    return Status;
}
#endif

   
NTSTATUS
GpdDispatch(
    IN    PDEVICE_OBJECT pDO,
    IN    PIRP pIrp             
    )

/*++

Routine Description:
    This routine is the dispatch handler for the driver.  It is responsible
    for processing the IRPs.

Arguments:
    
    pDO - Pointer to device object.

    pIrp - Pointer to the current IRP.

Return Value:

    STATUS_SUCCESS if the IRP was processed successfully, otherwise an error
    indicating the reason for failure.

--*/

{
    PIO_STACK_LOCATION pIrpStack;
    NTSTATUS Status;

    //  Initialize the irp info field.
    //      This is used to return the number of bytes transfered.

    pIrp->IoStatus.Information = 0;

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

    //  Set default return status
    Status = STATUS_NOT_IMPLEMENTED;

    // Dispatch based on major fcn code.

    switch (pIrpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
        case IRP_MJ_CLOSE:
            // We don't need any special processing on open/close so we'll
            // just return success.
            Status = STATUS_SUCCESS;
            break;

        case IRP_MJ_DEVICE_CONTROL:
            //  Dispatch on IOCTL
            switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode)
            {
            case IOCTL_GPD_READ_PORT_UCHAR:
            case IOCTL_GPD_READ_PORT_USHORT:
            case IOCTL_GPD_READ_PORT_ULONG:
                Status = GpdIoctlReadPort(
                            pDO,
                            pIrp,
                            pIrpStack,
                            pIrpStack->Parameters.DeviceIoControl.IoControlCode
                            );
                break;

            case IOCTL_GPD_WRITE_PORT_UCHAR:
            case IOCTL_GPD_WRITE_PORT_USHORT:
            case IOCTL_GPD_WRITE_PORT_ULONG:
                Status = GpdIoctlWritePort(
                            pDO, 
                            pIrp,
                            pIrpStack,
                            pIrpStack->Parameters.DeviceIoControl.IoControlCode
                            );
                break;
            }
            break;
    }

    // We're done with I/O request.  Record the status of the I/O action.
    pIrp->IoStatus.Status = Status;

    // Don't boost priority when returning since this took little time.
    IoCompleteRequest(pIrp, IO_NO_INCREMENT );

    return Status;
}


NTSTATUS
GpdIoctlReadPort(
    IN PDEVICE_OBJECT pDO,
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpStack,
    IN ULONG IoctlCode  )


/*++

Routine Description:
    This routine processes the IOCTLs which read from the ports.

Arguments:
    
    pDO         - our device
    pIrp        - IO request packet
    IrpStack    - The current stack location
    IoctlCode   - The ioctl code from the IRP

Return Value:
    STATUS_SUCCESS           -- OK

    STATUS_INVALID_PARAMETER -- The buffer sent to the driver
                                was too small to contain the
                                port, or the buffer which
                                would be sent back to the driver
                                was not a multiple of the data size.

    STATUS_ACCESS_VIOLATION  -- An illegal port number was given.

--*/

{
                                // NOTE:  Use METHOD_BUFFERED ioctls.
    PULONG pIOBuffer;           // Pointer to transfer buffer
                                //      (treated as an array of longs).
    ULONG InBufferSize;         // Amount of data avail. from caller.
    ULONG OutBufferSize;        // Max data that caller can accept.
    ULONG nPort;                // Port number to read
    ULONG DataBufferSize;
    PHYSICAL_ADDRESS PortAddress;
    PHYSICAL_ADDRESS MappedAddress;
    ULONG MemType;

    // Size of buffer containing data from application
    InBufferSize  = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

    // Size of buffer for data to be sent to application
    OutBufferSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    // NT copies inbuf here before entry and copies this to outbuf after
    // return, for METHOD_BUFFERED IOCTL's.
    pIOBuffer     = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

    // Check to ensure input buffer is big enough to hold a port number and
    // the output buffer is at least as big as the port data width.
    //
    switch (IoctlCode)
    {
    default:                    // There isn't really any default but 
	/* FALL THRU */         // this will quiet the compiler.
    case IOCTL_GPD_READ_PORT_UCHAR:
        DataBufferSize = sizeof(UCHAR);
        break;
    case IOCTL_GPD_READ_PORT_USHORT:
        DataBufferSize = sizeof(USHORT);
        break;
    case IOCTL_GPD_READ_PORT_ULONG:
        DataBufferSize = sizeof(ULONG);
        break;
    }

    if ( InBufferSize != sizeof(ULONG) || OutBufferSize < DataBufferSize )
    {
        return STATUS_INVALID_PARAMETER;
    }

    // Buffers are big enough.

    nPort = *pIOBuffer;             // Get the I/O port number from the buffer.

    PortAddress.LowPart  = nPort;
    PortAddress.HighPart = 0;

    // Convert the IO port address into a form NT likes.
    MemType = 1;                        // located in IO space
    HalTranslateBusAddress( Isa,
                            0,
                            PortAddress,
                            &MemType,
                            &MappedAddress );

    if (MemType == 0)
    {
        // Port is accessed through memory space
        // we aren't going to handle this today since we have to map it
        return STATUS_ACCESS_VIOLATION;
    }

    nPort = MappedAddress.LowPart;

    // Address is in I/O space
        
    switch (IoctlCode)
    {
    case IOCTL_GPD_READ_PORT_UCHAR:
        *(PUCHAR)pIOBuffer = READ_PORT_UCHAR(
                        (PUCHAR)nPort );
        break;
    case IOCTL_GPD_READ_PORT_USHORT:
        *(PUSHORT)pIOBuffer = READ_PORT_USHORT(
                        (PUSHORT)nPort );
        break;
    case IOCTL_GPD_READ_PORT_ULONG:
        *(PULONG)pIOBuffer = READ_PORT_ULONG(
                        (PULONG)nPort );
        break;
    }
    
    // Indicate # of bytes read
    //
    
    pIrp->IoStatus.Information = DataBufferSize;

    return STATUS_SUCCESS;
}


NTSTATUS
GpdIoctlWritePort(
    IN PDEVICE_OBJECT pDO,
    IN PIRP pIrp, 
    IN PIO_STACK_LOCATION IrpStack,
    IN ULONG IoctlCode
    )

/*++

Routine Description:
    This routine processes the IOCTLs which write to the ports.

Arguments:
    
    pDO         - our device
    pIrp        - IO request packet
    IrpStack    - The current stack location
    IoctlCode   - The ioctl code from the IRP

Return Value:
    STATUS_SUCCESS           -- OK

    STATUS_INVALID_PARAMETER -- The buffer sent to the driver
                                was too small to contain the
                                port, or the buffer which
                                would be sent back to the driver
                                was not a multiple of the data size.

    STATUS_ACCESS_VIOLATION  -- An illegal port number was given.

--*/

{
                                // NOTE:  Use METHOD_BUFFERED ioctls.
    PULONG pIOBuffer;           // Pointer to transfer buffer
                                //      (treated as array of longs).
    ULONG InBufferSize ;        // Amount of data avail. from caller.
    ULONG OutBufferSize ;       // Max data that caller can accept.
    ULONG nPort;                // Port number to read or write.
    ULONG DataBufferSize;
    PHYSICAL_ADDRESS PortAddress;
    PHYSICAL_ADDRESS MappedAddress;
    ULONG MemType;

    // Size of buffer containing data from application
    InBufferSize  = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

    // Size of buffer for data to be sent to application
    OutBufferSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    // NT copies inbuf here before entry and copies this to outbuf after return,
    // for METHOD_BUFFERED IOCTL's.
    pIOBuffer     = (PULONG) pIrp->AssociatedIrp.SystemBuffer;

    // We don't return any data on a write port.
    pIrp->IoStatus.Information = 0;
    
    // Check to ensure input buffer is big enough to hold a port number as well
    // as the data to write.
    //
    // The relative port # is a ULONG, and the data is the type appropriate to
    // the IOCTL.
    //

    switch (IoctlCode)
    {
    default:                    // There isn't really any default but 
	/* FALL THRU */         // this will quiet the compiler.
    case IOCTL_GPD_WRITE_PORT_UCHAR:
        DataBufferSize = sizeof(UCHAR);
        break;
    case IOCTL_GPD_WRITE_PORT_USHORT:
        DataBufferSize = sizeof(USHORT);
        break;
    case IOCTL_GPD_WRITE_PORT_ULONG:
        DataBufferSize = sizeof(ULONG);
        break;
    }

    if ( InBufferSize < (sizeof(ULONG) + DataBufferSize) )
    {
        return STATUS_INVALID_PARAMETER;
    }

    nPort = *pIOBuffer++;

    PortAddress.LowPart  = nPort;
    PortAddress.HighPart = 0;

    // Convert the IO port address into a form NT likes.
    MemType = 1;                        // located in IO space
    HalTranslateBusAddress( Isa,
                            0,
                            PortAddress,
                            &MemType,
                            &MappedAddress );

    if (MemType == 0)
    {
        // Port is accessed through memory space
        // we aren't going to handle this today since we have to map it
        return STATUS_ACCESS_VIOLATION;
    }

    nPort = MappedAddress.LowPart;

    // Address is in I/O space
        
    switch (IoctlCode)
    {
    case IOCTL_GPD_WRITE_PORT_UCHAR:
        WRITE_PORT_UCHAR(
            (PUCHAR)nPort,
            *(PUCHAR)pIOBuffer );
        break;
    case IOCTL_GPD_WRITE_PORT_USHORT:
        WRITE_PORT_USHORT(
            (PUSHORT)nPort,
            *(PUSHORT)pIOBuffer );
        break;
    case IOCTL_GPD_WRITE_PORT_ULONG:
        WRITE_PORT_ULONG(
            (PULONG)nPort,
            *(PULONG)pIOBuffer );
        break;
    }

    return STATUS_SUCCESS;
}


VOID
GpdUnload(
    PDRIVER_OBJECT DriverObject
    )

/*++

Routine Description:
    This routine prepares our driver to be unloaded.  It is responsible
    for freeing all resources allocated by DriverEntry as well as any 
    allocated while the driver was running.  The symbolic link must be
    deleted as well.

Arguments:
    
    DriverObject - Pointer to driver object created by the system.

Return Value:

    None

--*/

{
    UNICODE_STRING Win32DeviceName;

    // Assume all handles are closed down.
    // Delete the things we allocated - devices, symbolic links

    RtlInitUnicodeString(&Win32DeviceName, DOS_DEVICE_NAME);

    IoDeleteSymbolicLink(&Win32DeviceName);

    IoDeleteDevice(DriverObject->DeviceObject);
}
