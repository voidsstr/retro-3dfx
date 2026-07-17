/*++


 Copyright (c) 1997, 3Dfx Interactive, Inc.
 All Rights Reserved.

Module Name:

    ntremap.c

Abstract:

    Allow 3Dfx Voodoo graphics board to be remapped out of the way of the
    unclaimed space that the evil S3 868/968 is using.

Environment:

    kernel mode only

Notes:


Revision History:

--*/


#include "ntddk.h"
#include "ntremap.h"
#include "stdarg.h"



NTSTATUS
NTRemapDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
NTRemapUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
NTRemapMapTheMemory(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID      ioBuffer,
    IN ULONG          inputBufferLength,
    IN ULONG          outputBufferLength
    );



#if DBG
#define NTRemapKdPrint(arg) DbgPrint arg
#else
#define NTRemapKdPrint(arg)
#endif



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    Installable driver initialization entry point.
    This entry point is called directly by the I/O system.

Arguments:

    DriverObject - pointer to the driver object

    RegistryPath - pointer to a unicode string representing the path
                   to driver-specific key in the registry

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise

--*/
{

    PDEVICE_OBJECT deviceObject = NULL;
    NTSTATUS       ntStatus;
    WCHAR          deviceNameBuffer[] = L"\\Device\\NTRemap";
    UNICODE_STRING deviceNameUnicodeString;
    WCHAR          deviceLinkBuffer[] = L"\\DosDevices\\NTREMAP";
    UNICODE_STRING deviceLinkUnicodeString;



    NTRemapKdPrint (("NTREMAP.SYS: entering DriverEntry\n"));


    //
    // Create an EXCLUSIVE device object (only 1 thread at a time
    // can make requests to this device)
    //

    RtlInitUnicodeString (&deviceNameUnicodeString,
                          deviceNameBuffer);

    ntStatus = IoCreateDevice (DriverObject,
                               0,
                               &deviceNameUnicodeString,
                               FILE_DEVICE_NTREMAP,
                               0,
                               TRUE,
                               &deviceObject
                               );

    if (NT_SUCCESS(ntStatus))
    {
        //
        // Create dispatch points for device control, create, close.
        //

        DriverObject->MajorFunction[IRP_MJ_CREATE]         =
        DriverObject->MajorFunction[IRP_MJ_CLOSE]          =
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = NTRemapDispatch;
        DriverObject->DriverUnload                         = NTRemapUnload;



        //
        // Create a symbolic link, e.g. a name that a Win32 app can specify
        // to open the device
        //

        RtlInitUnicodeString(&deviceLinkUnicodeString,
                             deviceLinkBuffer);

        ntStatus = IoCreateSymbolicLink (&deviceLinkUnicodeString,
                                         &deviceNameUnicodeString);

        if (!NT_SUCCESS(ntStatus))
        {
            //
            // Symbolic link creation failed- note this & then delete the
            // device object (it's useless if a Win32 app can't get at it).
            //

            NTRemapKdPrint (("NTREMAP.SYS: IoCreateSymbolicLink failed\n"));

            IoDeleteDevice (deviceObject);
        }
        else
        {
            // Detect and correct write combining problem on Intel P6 chipset.
            LONG	IsNTRemapEnabledInRegistry(void);
            void	RemapTheBoard(void);

            if( IsNTRemapEnabledInRegistry() != 0L )     // OK with user?
                RemapTheBoard();                         // Yes - do it.
        }
    }
    else
    {
        NTRemapKdPrint (("NTREMAP.SYS: IoCreateDevice failed\n"));
    }

    return ntStatus;
}



NTSTATUS
NTRemapDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    Process the IRPs sent to this device.

Arguments:

    DeviceObject - pointer to a device object

    Irp          - pointer to an I/O Request Packet

Return Value:


--*/
{
    PIO_STACK_LOCATION irpStack;
    PVOID              ioBuffer;
    ULONG              inputBufferLength;
    ULONG              outputBufferLength;
    ULONG              ioControlCode;
    NTSTATUS           ntStatus;


    //
    // Init to default settings- we only expect 1 type of
    //     IOCTL to roll through here, all others an error.
    //

    Irp->IoStatus.Status      = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;


    //
    // Get a pointer to the current location in the Irp. This is where
    //     the function codes and parameters are located.
    //

    irpStack = IoGetCurrentIrpStackLocation(Irp);


    //
    // Get the pointer to the input/output buffer and it's length
    //

    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;


    switch (irpStack->MajorFunction)
    {
    case IRP_MJ_CREATE:

        NTRemapKdPrint (("NTREMAP.SYS: IRP_MJ_CREATE\n"));

        break;



    case IRP_MJ_CLOSE:

        NTRemapKdPrint (("NTREMAP.SYS: IRP_MJ_CLOSE\n"));

        break;



    case IRP_MJ_DEVICE_CONTROL:

        // ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

        NTRemapKdPrint (("NTREMAP.SYS: unknown IRP_MJ_DEVICE_CONTROL\n"));

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

        break;
    }


    //
    // DON'T get cute and try to use the status field of
    // the irp in the return status.  That IRP IS GONE as
    // soon as you call IoCompleteRequest.
    //

    ntStatus = Irp->IoStatus.Status;

    IoCompleteRequest(Irp,
                      IO_NO_INCREMENT);


    //
    // We never have pending operation so always return the status code.
    //

    return ntStatus;
}



VOID
NTRemapUnload(
    IN PDRIVER_OBJECT DriverObject
    )
/*++

Routine Description:

    Just delete the associated device & return.

Arguments:

    DriverObject - pointer to a driver object

Return Value:


--*/
{
    WCHAR                  deviceLinkBuffer[]  = L"\\DosDevices\\NTREMAP";
    UNICODE_STRING         deviceLinkUnicodeString;



    //
    // Free any resources
    //



    //
    // Delete the symbolic link
    //

    RtlInitUnicodeString (&deviceLinkUnicodeString,
                          deviceLinkBuffer
                          );

    IoDeleteSymbolicLink (&deviceLinkUnicodeString);



    //
    // Delete the device object
    //

    NTRemapKdPrint (("NTREMAP.SYS: unloading\n"));

    IoDeleteDevice (DriverObject->DeviceObject);
}
