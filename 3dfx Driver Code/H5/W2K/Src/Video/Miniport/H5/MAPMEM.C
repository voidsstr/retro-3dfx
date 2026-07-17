#include <ntddk.h>
#include <windef.h>
// DWF

VOID
VideoPortDebugPrint(
    ULONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    );

// physical memory device
#define _PHYSICAL_MEMORY_DEVICE  L"\\Device\\PhysicalMemory"

VOID * h3MapKernelMemoryToCurrentProcessSpace( PHYSICAL_ADDRESS sPhysicalAddress, 
	ULONG ulSize )
{
	UNICODE_STRING		uszDriverName;
	void *				pProcessRelativePointer = NULL;
	PHYSICAL_ADDRESS	sPhysicalAddressMapped;
	OBJECT_ATTRIBUTES	sObjectAttributes;
	void *				hDriver  = NULL;
	void *				pPhysicalMemoryObject = NULL;
	ULONG				ulLength;
	NTSTATUS			ntStatus;

	// initialize the information required to access the physical memory device
    RtlInitUnicodeString( &uszDriverName, _PHYSICAL_MEMORY_DEVICE ); 
    InitializeObjectAttributes(&sObjectAttributes, &uszDriverName, OBJ_CASE_INSENSITIVE, 
		( HANDLE )NULL, ( PSECURITY_DESCRIPTOR )NULL ); 
    ntStatus = ZwOpenSection( &hDriver, SECTION_ALL_ACCESS, &sObjectAttributes ); 
    if( !NT_SUCCESS( ntStatus ) )
	{ 
//		VideoDebugPrint( ( 0, "Unable To Map Kernel Memory To User Space!!!! - FATAL\n"));
        return( NULL );
    }
    ntStatus = ObReferenceObjectByHandle( hDriver, SECTION_ALL_ACCESS, 
		( POBJECT_TYPE )NULL, KernelMode, &pPhysicalMemoryObject, 
		( POBJECT_HANDLE_INFORMATION )NULL ); 
 
    if( !NT_SUCCESS( ntStatus ) ) 
	{ 
//		VideoDebugPrint((0, "Error : ObjReferenceObjectByHandle!!!! - FATAL\n"));
		ZwClose( hDriver ); 
		return( NULL );
    } 

    // map the section of the physical memory device into the address space of the current
	// process
	sPhysicalAddressMapped = sPhysicalAddress;
    ntStatus = ZwMapViewOfSection( hDriver, ( HANDLE )-1, &pProcessRelativePointer,
        0L, ulSize,	&sPhysicalAddressMapped, &ulSize,	ViewUnmap, 0, 
		PAGE_READWRITE | PAGE_NOCACHE );
 
    if( !NT_SUCCESS( ntStatus ) )
	{
//		VideoDebugPrint( ( 0, "Unable To Map View Of Section!!! - FATAL\n"));
		ZwClose( hDriver ); 
		return( NULL );
	}
	( ULONG )pProcessRelativePointer += ( ULONG )sPhysicalAddress.LowPart - 
		(ULONG)sPhysicalAddressMapped.LowPart; 

	// close the driver
	ZwClose( hDriver ); 

	// return a pointer to the address space
	return( pProcessRelativePointer );
}  

// DWFE