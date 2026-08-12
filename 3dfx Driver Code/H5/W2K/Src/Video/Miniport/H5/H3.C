/*++
**
** Copyright (c) 1997-1998, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.

Module Name:

	h3.c

Abstract:

	This module contains the code that implements the H3 miniport driver.

Environment:

	Kernel mode

Revision History:

--*/

#ifdef INCSTBCUST
#ifndef MS_VIEW
#include "..\..\..\..\build\stbcust.inc"
#else
#include "stbcust.inc"
#endif
#endif

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "h3.h"
#include "localpci.h"
#include "cmdcnst.h"
#include "3dfx.h"

#define TVOUT_SUPPORTED 1
#ifdef TVOUT_SUPPORTED
#define VDDONLY
#include "tv.h"
#undef VDDONLY
#include "bt868.h"
#include "dfp.h"
#endif //def TVOUT_SUPPORTED
#include "bios.h"

#if (_WIN32_WINNT >= 0x500)
#include "acpiioct.h"
#include "tvout.h"
#endif

#ifdef SLI_AA
#include "sliaa.h"
#endif

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,DriverEntry)
#pragma alloc_text(PAGE,H3FindAdapter)
#pragma alloc_text(PAGE,H3DetermineMemorySize)
#pragma alloc_text(PAGE,H3MapAccessRanges)
#pragma alloc_text(PAGE,H3RecordChipType)
#pragma alloc_text(PAGE,H3Initialize)
#pragma alloc_text(PAGE,H3StartIO)
#pragma alloc_text(PAGE,H3SetColorLookup)
#pragma alloc_text(PAGE,H3GetGammaTable)
#pragma alloc_text(PAGE,H3SetGammaTable)
#pragma alloc_text(PAGE,H3UpdateGamma)
// WARNING!  H3ResetHw must not be in paged memory!
//#pragma alloc_text(PAGE,H3ResetHw)
#endif

#define UNKNOWN_STRING				L"Unknown"

// Globally declare these at the module level
// to support detection of KNI caps in ICD
// in H3Initialize and H3StartIO.
VOID	__cdecl TestForP6(void);
FxU32	isP6;

// START ALT-TAB CHANGES
VOID * h3MapKernelMemoryToCurrentProcessSpace( PHYSICAL_ADDRESS sPhysicalAddress,
	ULONG ulSize );
NTKERNELAPI
PVOID
MmAllocateNonCachedMemory (
    IN ULONG NumberOfBytes
    );
NTKERNELAPI
PHYSICAL_ADDRESS
MmGetPhysicalAddress (
    IN PVOID BaseAddress
    );
#ifdef MS_VIEW
/*  Illegal w2k import
NTKERNELAPI
PVOID
MmAllocateContiguousMemory (
    IN ULONG NumberOfBytes,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress
    );
    */
#else
NTKERNELAPI
PVOID
MmAllocateContiguousMemory (
    IN ULONG NumberOfBytes,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress
    );
#endif

// END ALT-TAB CHANGES

ULONG
DriverEntry (
	PVOID Context1,
	PVOID Context2
	)

/*++

Routine Description:

	Installable driver initialization entry point.
	This entry point is called directly by the I/O system.

Arguments:

	Context1 - First context value passed by the operating system. This is
		the value with which the miniport driver calls VideoPortInitialize().

	Context2 - Second context value passed by the operating system. This is
		the value with which the miniport driver calls VideoPortInitialize().

Return Value:

	Status from VideoPortInitialize()

--*/

{

	VIDEO_HW_INITIALIZATION_DATA hwInitData;
	ULONG initializationStatus;
#if (_WIN32_WINNT < 0x0500)
	DriverEntryInfo EntryInfo;
#endif

	//
	// Zero out structure.
	//

	VideoPortZeroMemory(&hwInitData, sizeof(VIDEO_HW_INITIALIZATION_DATA));

	//
	// Specify sizes of structure and extension.
	//

	hwInitData.HwInitDataSize = sizeof(VIDEO_HW_INITIALIZATION_DATA);

#if (_WIN32_WINNT < 0x0500)
	//
	// make a hard copy of the registry path for the driver.
	//
	Context2 = H3CopyUnicodeString(Context2);
	//
	// At various points we may need to get hold of the DriverObject and the
	// RegistryPath. So pass them through to FindAdapter to store in the
	// HwDeviceExtension.
	//

	EntryInfo.DriverObject = Context1;
	EntryInfo.RegistryPath = Context2;
	EntryInfo.HwDeviceExtension = NULL;
	EntryInfo.BoardNumber = 0;
	EntryInfo.SlotDeviceNumber = 0;
	EntryInfo.VPConnectsInterrupt = 0;
#endif

	//
	// Set entry points.
	//

	hwInitData.HwFindAdapter = H3FindAdapter;
	hwInitData.HwInitialize = H3Initialize;
	hwInitData.HwStartIO = H3StartIO;
	hwInitData.HwResetHw = H3ResetHw;

#if (_WIN32_WINNT >= 0x500)
	hwInitData.HwGetVideoChildDescriptor = H3GetChildDescriptor;
	hwInitData.HwGetPowerState = H3GetPowerState;
	hwInitData.HwSetPowerState = H3SetPowerState;
#endif

#if  ENABLE_IRQ
	hwInitData.HwInterrupt = H3VidInterrupt;
#else
	hwInitData.HwInterrupt =NULL;
#endif
#if  KMVT
    hwInitData.HwQueryInterface = QueryDriverInterface;
#endif

	//
	// Determine the size we require for the device extension.
	//

	hwInitData.HwDeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);
#if (_WIN32_WINNT < 0x500)
	hwInitData.StartingDeviceNumber = 0;
#endif
	//
	// Once all the relevant information has been stored, call the video
	// port driver to do the initialization.
	//

	//
	// NOTE: since this driver currently supports one adapter, we will return
	// as soon as we find a device. Eventually, we'll support more than one
	// instance on buses. Normally one would call for each bus type and
	// return the smallest value.
	//

	//
	// We only support PCI. AGP cards show up as being 66MHz PCI cards.
	//

	hwInitData.AdapterInterfaceType = PCIBus;

#if (_WIN32_WINNT >= 0x0500)
	initializationStatus = VideoPortInitialize(Context1,
											   Context2,
											   &hwInitData,
											   NULL);	// Must be NULL for NT5!
#else
	initializationStatus = VideoPortInitialize(Context1,
											   Context2,
											   &hwInitData,
											   &EntryInfo);
	//
	// The video port driver has completely initialized.
	//
	if ((EntryInfo.BoardNumber > 0) && (EntryInfo.HwDeviceExtension != NULL))
	{
		((PHW_DEVICE_EXTENSION)(EntryInfo.HwDeviceExtension))->NumberOfAdapters =
																EntryInfo.BoardNumber;
	}
#endif

	return initializationStatus;

} // end DriverEntry()

void InitializeNbChipsInRegistry(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
#if defined(GTF_TIMINGS) || defined(DMT_ENABLED)
    // Note this code is in funcapi.c in Win9x driver.
#ifndef MS_VIEW
#define REG_D3D_SINGLE   "D3D\\SingleChipAASLI"
#define REG_D3D_DUAL     "D3D\\DualChipAASLI"
#define REG_D3D_QUAD     "D3D\\QuadChipAASLI"    

#define REG_GLIDE_SINGLE   "Glide\\SingleChipAASLI"
#define REG_GLIDE_DUAL     "Glide\\DualChipAASLI"
#define REG_GLIDE_QUAD     "Glide\\QuadChipAASLI"
#else
#define REG_D3D_SINGLE   L"D3D\\SingleChipAASLI"
#define REG_D3D_DUAL     L"D3D\\DualChipAASLI"
#define REG_D3D_QUAD     L"D3D\\QuadChipAASLI"    
#define REG_GLIDE_SINGLE   L"Glide\\SingleChipAASLI"
#define REG_GLIDE_DUAL     L"Glide\\DualChipAASLI"
#define REG_GLIDE_QUAD     L"Glide\\QuadChipAASLI" 
#endif //MS_VIEW    


#define REG_DISABLED "Disabled"
#define REG_CONTROL  "Control"
#define REG_ENABLE	 "0"
#define REG_DISABLE	 "2"

    char tempStr[256];  
    ULONG length = sizeof(tempStr); 

#ifndef MS_VIEW
	char nodeKey[256];
#else
	WCHAR nodeKey[256];
	VP_STATUS status;
#endif //MS_VIEW

    char * singleStatus; 
    char * dualStatus;
    char * quadStatus;
    HKEY hkey;
    ULONG type;

    /* Turn on different tweaks based on the number of graphic chips */
    switch (HwDeviceExtension->numUnits)
    {
    case 1:
        {
            singleStatus = REG_ENABLE; 
            dualStatus   = REG_DISABLE;
            quadStatus   = REG_DISABLE;

            break;
        }

    case 2:
        {
            singleStatus = REG_DISABLE; 
            dualStatus   = REG_ENABLE;
            quadStatus   = REG_DISABLE;

            break;
        }

    case 4:
        {
            singleStatus = REG_DISABLE; 
            dualStatus   = REG_DISABLE;
            quadStatus   = REG_ENABLE;

            break;

        }

    default:
        {
            singleStatus = REG_DISABLE; 
            dualStatus   = REG_DISABLE;
            quadStatus   = REG_DISABLE;

            break;
        }
    }


    /* Write out if the registry entries exist. All tweaks will have a Control key - so 
    check for it */

#ifndef MS_VIEW
    /* D3D First */

	strcpy( nodeKey, HwDeviceExtension->DevNodeKeyPath );
	strcat( nodeKey, "\\" );
	strcat( nodeKey, REG_D3D_SINGLE );
    if (di_RegOpenKey( HKEY_LOCAL_MACHINE, nodeKey, &hkey ) == CR_SUCCESS)
    {
        if (di_RegQueryValueEx(hkey, REG_CONTROL, 0, &type,
                tempStr, &length) == CR_SUCCESS)
        {
            di_RegSetValueEx(hkey, REG_DISABLED, 0,
                    REG_SZ, singleStatus, sizeof(REG_DISABLE));
        }
        di_RegCloseKey( hkey );
    }

	strcpy( nodeKey, HwDeviceExtension->DevNodeKeyPath );
	strcat( nodeKey, "\\" );
	strcat( nodeKey, REG_D3D_DUAL );
    if (di_RegOpenKey( HKEY_LOCAL_MACHINE, nodeKey, &hkey ) == CR_SUCCESS)
    {
        if (di_RegQueryValueEx(hkey, REG_CONTROL, 0, &type,
                tempStr, &length) == CR_SUCCESS)
        {
            di_RegSetValueEx(hkey, REG_DISABLED, 0,
                    REG_SZ, dualStatus, sizeof(REG_DISABLE));
        }
        di_RegCloseKey( hkey );
    }

	strcpy( nodeKey, HwDeviceExtension->DevNodeKeyPath );
	strcat( nodeKey, "\\" );
	strcat( nodeKey, REG_D3D_QUAD );
    if (di_RegOpenKey( HKEY_LOCAL_MACHINE, nodeKey, &hkey ) == CR_SUCCESS)
    {
        if (di_RegQueryValueEx(hkey, REG_CONTROL, 0, &type,
                tempStr, &length) == CR_SUCCESS)
        {
            di_RegSetValueEx(hkey, REG_DISABLED, 0,
                    REG_SZ, quadStatus, sizeof(REG_DISABLE));
        }
        di_RegCloseKey( hkey );
    }


    /* Then the Glide */
	strcpy( nodeKey, HwDeviceExtension->DevNodeKeyPath );
	strcat( nodeKey, "\\" );
	strcat( nodeKey, REG_GLIDE_SINGLE );
    if (di_RegOpenKey( HKEY_LOCAL_MACHINE, nodeKey, &hkey ) == CR_SUCCESS)
    {
        if (di_RegQueryValueEx(hkey, REG_CONTROL, 0, &type,
                tempStr, &length) == CR_SUCCESS)
        {
            di_RegSetValueEx(hkey, REG_DISABLED, 0,
                    REG_SZ, singleStatus, sizeof(REG_DISABLE));
        }
        di_RegCloseKey( hkey );
    }

	strcpy( nodeKey, HwDeviceExtension->DevNodeKeyPath );
	strcat( nodeKey, "\\" );
	strcat( nodeKey, REG_GLIDE_DUAL );
    if (di_RegOpenKey( HKEY_LOCAL_MACHINE, nodeKey, &hkey ) == CR_SUCCESS)
    {
        if (di_RegQueryValueEx(hkey, REG_CONTROL, 0, &type,
                tempStr, &length) == CR_SUCCESS)
        {
            di_RegSetValueEx(hkey, REG_DISABLED, 0,
                    REG_SZ, dualStatus, sizeof(REG_DISABLE));
        }
        di_RegCloseKey( hkey );
    }

	strcpy( nodeKey, HwDeviceExtension->DevNodeKeyPath );
	strcat( nodeKey, "\\" );
	strcat( nodeKey, REG_GLIDE_QUAD );
    if (di_RegOpenKey( HKEY_LOCAL_MACHINE, nodeKey, &hkey ) == CR_SUCCESS)
    {
        if (di_RegQueryValueEx(hkey, REG_CONTROL, 0, &type,
                tempStr, &length) == CR_SUCCESS)
        {
            di_RegSetValueEx(hkey, REG_DISABLED, 0,
                    REG_SZ, quadStatus, sizeof(REG_DISABLE));
        }
        di_RegCloseKey( hkey );
    }
#else
	/* D3D First */
    wcscpy( nodeKey, REG_D3D_SINGLE );
    
	status = VideoPortSetRegistryParameters( HwDeviceExtension,
											 (PWSTR)nodeKey,
											 singleStatus,
											 sizeof(REG_DISABLE));
	
	wcscpy( nodeKey, REG_D3D_DUAL );
    
	status = VideoPortSetRegistryParameters( HwDeviceExtension,
											 (PWSTR)nodeKey,
											 dualStatus,
											 sizeof(REG_DISABLE));

	wcscpy( nodeKey, REG_D3D_QUAD );

	status = VideoPortSetRegistryParameters( HwDeviceExtension,
											 (PWSTR)nodeKey,
											 quadStatus,
											 sizeof(REG_DISABLE));

    /* Then the Glide */
	wcscpy( nodeKey, REG_GLIDE_SINGLE );

	status = VideoPortSetRegistryParameters( HwDeviceExtension,
											 (PWSTR)nodeKey,
											 singleStatus,
											 sizeof(REG_DISABLE));

	wcscpy( nodeKey, REG_GLIDE_DUAL );

	status = VideoPortSetRegistryParameters( HwDeviceExtension,
											 (PWSTR)nodeKey,
											 dualStatus,
											 sizeof(REG_DISABLE));

	wcscpy( nodeKey, REG_GLIDE_QUAD );

	status = VideoPortSetRegistryParameters( HwDeviceExtension,
											 (PWSTR)nodeKey,
											 quadStatus,
											 sizeof(REG_DISABLE));

#endif //MS_VIEW

#endif
}

VP_STATUS
H3FindAdapter(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVOID HwContext,
	PWSTR ArgumentString,
	PVIDEO_PORT_CONFIG_INFO ConfigInfo,
	PUCHAR Again
	)

/*++

Routine Description:

	This routine is called to determine if the adapter for this driver
	is present in the system.
	If it is present, the function fills out some information describing
	the adapter.

Arguments:

	HwDeviceExtension - Supplies the miniport driver's adapter storage. This
		storage is initialized to zero before this call.

	HwContext - Supplies the context value which was passed to
		VideoPortInitialize().

	ArgumentString - Suuplies a NULL terminated ASCII string. This string
		originates from the user.

	ConfigInfo - Returns the configuration information structure which is
		filled by the miniport driver. This structure is initialized with
		any knwon configuration information (such as SystemIoBusNumber) by
		the port driver. Where possible, drivers should have one set of
		defaults which do not require any supplied configuration information.

	Again - Indicates if the miniport driver wants the port driver to call
		its VIDEO_HW_FIND_ADAPTER function again with a new device extension
		and the same config info. This is used by the miniport drivers which
		can search for several adapters on a bus.

Return Value:

	This routine must return:

	NO_ERROR - Indicates a host adapter was found and the
		configuration information was successfully determined.

	ERROR_INVALID_PARAMETER - Indicates an adapter was found but there was an
		error obtaining the configuration information. If possible an error
		should be logged.

	ERROR_DEV_NOT_EXIST - Indicates no host adapter was found for the
		supplied configuration information.

--*/

{
#if (_WIN32_WINNT < 0x0500)
	pDriverEntryInfo pEntryInfo = HwContext;
#endif
	PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
	VP_STATUS status;
	MM_SYSTEMSIZE SystemSize;

	int i,j;

	UNREFERENCED_PARAMETER(ArgumentString);

	VideoDebugPrint((0, "H3FindAdapter\n"));

	//
	// Make sure the size of the structure is at least as large as what we
	// are expecting (check version of the config info structure).
	//

	if (ConfigInfo->Length < sizeof(VIDEO_PORT_CONFIG_INFO))
		return (ERROR_INVALID_PARAMETER);

	hwDeviceExtension->MiniportDebugLevel = DBG_KITCHEN_SINK;	// debugging use only
	hwDeviceExtension->UseSoftwareCursor  = 0;					// debugging use only
	hwDeviceExtension->UseNonBIOSModeSet  = 0;		// XXX default is to NOT use BIOS
	hwDeviceExtension->MemClocking		  = 0;
	hwDeviceExtension->GraphicsClocking   = 0;
	hwDeviceExtension->miscInit1	      = SGRAM_DEFAULT_MISCINIT1;	// Assume SGRAMs
	hwDeviceExtension->dramInit0	      = SGRAM_DEFAULT_DRAMINIT0;	// Assume SGRAMs
	hwDeviceExtension->dramInit1	      = SGRAM_DEFAULT_DRAMINIT1;	// Assume SGRAMs
	hwDeviceExtension->SDRAMPresent		  = FALSE;	// Assume we're SGRAM
	hwDeviceExtension->SGRAMMode		  = 0x37;	// XXX -- It's a magic number
	hwDeviceExtension->SGRAMChips		  = 1;		// One set of SGRAMs
	hwDeviceExtension->SGRAMMemorySize	  = 4;		// Assume 8Mb SGRAMs
	hwDeviceExtension->BiosPresent        = FALSE;	// Assume no BIOS
	hwDeviceExtension->BiosSize			  = 0;		//   as above
	hwDeviceExtension->BiosVersion[0]	  = '\0';	//   as above
	hwDeviceExtension->OldBiosPresent	  = FALSE;	// Assume new BIOS if there is one
	hwDeviceExtension->IsSecondaryDevice  = FALSE;	// Assume we're a primary device
#if ENABLE_UNATTENDED_INSTALL_CHECK
	hwDeviceExtension->UnattendedInstall  = FALSE;	// Gateway-specific flag
#endif
#if (_WIN32_WINNT >= 0x0500) && defined (AGP_FIFO_CODE)
	hwDeviceExtension->AFifo              = 0;
#endif
#if 0
#ifdef SLI_AA
	hwDeviceExtension->render32bpp		  = 1;
	hwDeviceExtension->disableSli		  = 0;
	hwDeviceExtension->AAenable			  = 0;
	hwDeviceExtension->AAsamples		  = 2;
	hwDeviceExtension->twoPPC			  = 1;
	hwDeviceExtension->twoPPCLog2BandHeight = 0;  // Adjust this
	hwDeviceExtension->guardbandClip	  = 1;
	hwDeviceExtension->cbc				  = 0;
	hwDeviceExtension->digitalSliAAEnable = 1;
	hwDeviceExtension->sliBandHeight	  = 1;
	hwDeviceExtension->swapAlgorithmType  = 1;
#endif
#endif


#if ENABLE_ADDRESS_LIST_ARRAY
	for (i = 0; i < MAX_ADDRESS_TABLE_SIZE; i++)
	{
		hwDeviceExtension->AddressList[i] = 0L;
		hwDeviceExtension->AddressAllocCount[i]  = 0;
	}
	DumpAddressTable(hwDeviceExtension);
#endif

	for (i = 0; i < NUM_CLUT_ENTRIES; i++)
	{
		j = ((i << 16) | (i << 8) | i);
		hwDeviceExtension->GammaTable[i]      = j;
	}

	memcpy(DefaultGlideGammaTable,
		   hwDeviceExtension->GlideGammaTable,
		   (NUM_CLUT_ENTRIES * sizeof(ULONG)));

#if (_WIN32_WINNT < 0x500)
	hwDeviceExtension->DriverObject = pEntryInfo->DriverObject;
	hwDeviceExtension->BoardNumber	= pEntryInfo->BoardNumber;
	hwDeviceExtension->RegistryPath = pEntryInfo->RegistryPath;
#endif

#ifdef SLI_AA
	// Save the bus number.

	HwDeviceExtension->BusNumber = ConfigInfo->SystemIoBusNumber;
#endif

	//
	// Detect the PCI card
	//

	status = H3ConfigurePCI(HwDeviceExtension);
	if (status != NO_ERROR)
		return status;
#ifdef SLI_AA
  DetectNumUnits(HwDeviceExtension);
#endif

	//
	// Suck in the registry settings
	//
	H3GetRegistrySettings(HwDeviceExtension);

	//
	// pick up capabilities on the way.
	//

	hwDeviceExtension->Capabilities = 0;

#if ENABLE_UNATTENDED_INSTALL_CHECK
	//
	// Find the approximate memory size of system memory. This matters,
	// because there are issues associated with some systems that have
	// low-memory (small < 20 and medium < 33 sized) configurations that
	// may have an impact on how the driver runs
	//
	SystemSize = MmQuerySystemSize();
	VideoDebugPrint((0, "System has "));
	switch (SystemSize) {
	case MmSmallSystem:
		VideoDebugPrint((0, "SMALL"));
		break;
	case MmMediumSystem:
		VideoDebugPrint((0, "MEDIUM"));
		break;
	case MmLargeSystem:
		//
		// In this memory configuration (> 32 Mb), reduced access ranges
		// are not necessary, so disallow reduced access ranges.
		//
		VideoDebugPrint((0, "LARGE"));
		hwDeviceExtension->UnattendedInstall = FALSE;
		break;
	}
	VideoDebugPrint((0, " memory configuration\n"));
#endif

	//
	// Verify the access ranges are mappable, and map them in...
	//
	status = H3MapAccessRanges(HwDeviceExtension);
	if (status != NO_ERROR)
		return status;

	//
	// Make sure the chip type we detected is correctly stored in
	// the DeviceExtension, and that the DAC type is correctly
	// stored as well. Write the final values to the registry.
	//

	status = H3RecordChipType(HwDeviceExtension);
	if (status != NO_ERROR)
		return status;

	//
	// If we're on a secondary device, we must POST the card,
	// which amounts to initialization
	//
	if (HwDeviceExtension->IsSecondaryDevice)
		H3InitializeSecondaryDevice(HwDeviceExtension,
                                HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX],
                                HwDeviceExtension->MappedAddress[SST_IO_INDEX]);

#ifdef SLI_AA
  InitializeSlaveChipsPCIConfigSpace(HwDeviceExtension);
  MapSlaveChipsAccessRanges(HwDeviceExtension);

#if DBG
  if (IS_NAPALM)
  {
    PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) hwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];

    VideoDebugPrint((0, "Power On Strapping Pins\n"));
    RegisterMap->_reserved00 = RegisterMap->_reserved00 & ~1;
    VideoDebugPrint((0, "  StrapInfo0=0x%08lXh\n", RegisterMap->_reserved00));
    RegisterMap->_reserved00 = RegisterMap->_reserved00 | 1;
    VideoDebugPrint((0, "  StrapInfo1=0x%08lXh\n", RegisterMap->_reserved00));
  }
#endif
#endif

#ifndef SLI_AA
	//
	// Size the memory on the Voodoo board, and write the
	// value to the registry
	//

	status = H3DetermineMemorySize(HwDeviceExtension);
	if (status != NO_ERROR)
		return status;
#endif

#if (_WIN32_WINNT >= 0x500) && defined (AGP_FIFO_CODE)

    // Note: There is only one AGP card in a system.

    // Is this an AGP system and are we initializing the AGP card,
    // and does the user want AGP command fifo?

    if( (HwDeviceExtension->IsAGPCard == TRUE ) &&
        (HwDeviceExtension->AFifo != 0) &&
        VideoPortGetAgpServices(HwDeviceExtension,&HwDeviceExtension->AgpServices) == TRUE )
    {
        ULONG               ulPages;
        PHYSICAL_ADDRESS    physaddr_null = { 0 };

        // The user asked for AGP command fifo so attempt to map it in.

        HwDeviceExtension->agpinfo.agpFifoSizeInB = 0;    // Assume failure -- size 0
        ulPages = BYTES_TO_PAGES( AGP_FIFO_MAX_SIZE );

        HwDeviceExtension->agpinfo.physAddr = (*HwDeviceExtension->AgpServices.AgpReservePhysical)(
                                                HwDeviceExtension,
                                                ulPages,
                                                TRUE,                   // Use USWC caching
                                                &HwDeviceExtension->agpinfo.PhysicalReserveContext);

        if ( HwDeviceExtension->agpinfo.physAddr.QuadPart != physaddr_null.QuadPart )
        {
            if ((*HwDeviceExtension->AgpServices.AgpCommitPhysical)(
                    HwDeviceExtension,
                    HwDeviceExtension->agpinfo.PhysicalReserveContext,
                    ulPages,
                    0                       // Offset into context
                    ) == TRUE)
            {

                if ((HwDeviceExtension->agpinfo.virtualAddr =
                    (*HwDeviceExtension->AgpServices.AgpReserveVirtual)(
                        HwDeviceExtension,
                        NULL,    // Process Handle - this may puke - want kernel space
                        HwDeviceExtension->agpinfo.PhysicalReserveContext,
                        &HwDeviceExtension->agpinfo.VirtualReserveContext
                    )) != NULL )
                {

                    if ((HwDeviceExtension->agpinfo.virtualAddr =
                        (*HwDeviceExtension->AgpServices.AgpCommitVirtual)(
                                HwDeviceExtension,
                                HwDeviceExtension->agpinfo.VirtualReserveContext,
                                ulPages,
                                0       // Offset
                    	)) != NULL)
                    {
                		// Set AGP capability flag and AGP fifo size

                		HwDeviceExtension->Capabilities |= CAPS_AGP_FIFO;
                        HwDeviceExtension->agpinfo.agpFifoSizeInB = AGP_FIFO_MAX_SIZE;
                    }
                    else
                    {
                        (*HwDeviceExtension->AgpServices.AgpReleaseVirtual)(
                            HwDeviceExtension,
                            HwDeviceExtension->agpinfo.VirtualReserveContext
                        );
                        goto agpfail_2;
                    }
                }
                else
                {
agpfail_2:
                    (*HwDeviceExtension->AgpServices.AgpFreePhysical)(
                        HwDeviceExtension,
                        HwDeviceExtension->agpinfo.PhysicalReserveContext,
                        ulPages,
                        0
                    );
                    goto agpfail_1;
                }
		   }
		   else
		   {
agpfail_1:
                (*HwDeviceExtension->AgpServices.AgpReleasePhysical)(
                        HwDeviceExtension,
                        HwDeviceExtension->agpinfo.PhysicalReserveContext
                );
		   }
		}
	}
#endif

  H3CopyDriverRegistryPathToDeviceExtension(hwDeviceExtension, ConfigInfo->DriverRegistryPath);

	i2c_initialize(HwDeviceExtension);	// Must initialize before using in DMT code

	//
	// Validate the video modes and the frequency table
	//

#ifdef GTF_TIMINGS
	HwDeviceExtension->MonitorIsGTF = di_IsMonitorGTF( HwDeviceExtension );   
#endif // GTF_TIMINGS

#ifdef DMT_ENABLED
#ifndef MS_VIEW
	di_FindDevNode( HwDeviceExtension );
#endif
	di_AllocModeTable( HwDeviceExtension, &HwDeviceExtension->RegistryNumModes );
	di_FillModeTable( HwDeviceExtension );
	NumH3VideoModes = HwDeviceExtension->RegistryNumModes;
	HwDeviceExtension->FixedFrequencyTable = HwDeviceExtension->RegistryVideoFreqs;
	HwDeviceExtension->UseNonBIOSModeSet = TRUE;	// Force the use of Non Bios mode sets
#endif // DMT_ENABLED

	status = H3ValidateModes(HwDeviceExtension);
	if (status != NO_ERROR)
		return status;

#if (_WIN32_WINNT < 0x500)
	pEntryInfo->HwDeviceExtension = HwDeviceExtension;

	pEntryInfo->BoardNumber++;
#endif

#if (_WIN32_WINNT >= 0x0500)
	ConfigInfo->VdmPhysicalVideoMemoryAddress.LowPart  = 0x00000000;
	ConfigInfo->VdmPhysicalVideoMemoryAddress.HighPart = 0x00000000;
	ConfigInfo->VdmPhysicalVideoMemoryLength           = 0x00000000;
#else
	/////////////////////////////////////////////////////////////////////////
	//
	// We have this so that the int10 will also work on the VGA also if we
	// use it in this driver.
	//

	ConfigInfo->VdmPhysicalVideoMemoryAddress.LowPart  = 0x000A0000;
	ConfigInfo->VdmPhysicalVideoMemoryAddress.HighPart = 0x00000000;
	ConfigInfo->VdmPhysicalVideoMemoryLength		   = 0x00020000;
#endif

	//
	// Clear out the Emulator entries and the state size since this driver
	// does not support them.
	//

	ConfigInfo->NumEmulatorAccessEntries	 = 0;
	ConfigInfo->EmulatorAccessEntries		 = NULL;
	ConfigInfo->EmulatorAccessEntriesContext = 0;

	//
	// This driver does not do SAVE/RESTORE of hardware state.
	//

	ConfigInfo->HardwareStateSize = 0;

	//
	// Save the physical addresses in hwDeviceExtension
	//

	//
	// Frame buffer and memory-mapped I/O information.
	//

	hwDeviceExtension->PhysicalMemBaseAddr0 = hwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeStart;
	hwDeviceExtension->MemBase0Length		= hwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeLength;
	hwDeviceExtension->MemBase0InIOSpace	= hwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeInIoSpace;

	hwDeviceExtension->PhysicalMemBaseAddr1 = hwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart;
	hwDeviceExtension->MemBase1Length		= hwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeLength;
	hwDeviceExtension->MemBase1InIOSpace	= hwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeInIoSpace;

	//
	// IO Port information
	// Get the base address, starting at zero and map all registers
	//

	hwDeviceExtension->PhysicalIOBaseAddr	= hwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeStart;
	hwDeviceExtension->IOBaseLength 		= hwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeLength;
	hwDeviceExtension->IOBaseInIoSpace		= hwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeInIoSpace;

	//
	// We do not call again - this is a one-headed implementation
	//
#if (_WIN32_WINNT >= 0x0500)
	*Again = 1;
#else
	*Again = 0;
#endif

	//
	// Indicate a successful completion status.
	//

  VideoDebugPrint((0, "H3FindAdapter - done\n"));
	return status;

} // end H3FindAdapter()

VOID
GetInitRegsFromBios(PHW_DEVICE_EXTENSION HwDeviceExtension,
                    PUCHAR               pBios)
{
  OEMTABLE  *pOemTable;
  USHORT     usTemp;
  ULONG      GRXClock;


  usTemp = *(USHORT *)&pBios[ROM_CONFIG];
  usTemp = *(USHORT *)&pBios[usTemp];
  pOemTable = (OEMTABLE *)&pBios[usTemp];

  VideoDebugPrint((0, "GetInitRegsFromBios\n"));
  VideoDebugPrint((0, "  pciInit0  = %08lXh\n", pOemTable->pciInit0 ));
  VideoDebugPrint((0, "  miscInit0 = %08lXh\n", pOemTable->miscInit0));
  VideoDebugPrint((0, "  miscInit1 = %08lXh\n", pOemTable->miscInit1));
  VideoDebugPrint((0, "  dramInit0 = %08lXh\n", pOemTable->dramInit0));
  VideoDebugPrint((0, "  dramInit1 = %08lXh\n", pOemTable->dramInit1));
  VideoDebugPrint((0, "  agpInit0  = %08lXh\n", pOemTable->agpInit0 ));
  VideoDebugPrint((0, "  pllCtrl1  = %08lXh\n", pOemTable->pllCtrl1 ));
  VideoDebugPrint((0, "  pllCtrl2  = %08lXh\n", pOemTable->pllCtrl2 ));
  VideoDebugPrint((0, "  sgramMode = %08lXh\n", pOemTable->sgramMode));

  HwDeviceExtension->pciInit0  = pOemTable->pciInit0;
  HwDeviceExtension->miscInit0 = pOemTable->miscInit0;
  HwDeviceExtension->miscInit1 = pOemTable->miscInit1 & ~SST_MISCINIT1_STRAP_MASK;
  HwDeviceExtension->dramInit0 = pOemTable->dramInit0 & ~SST_DRAMINIT0_STRAP_MASK;
  HwDeviceExtension->dramInit1 = pOemTable->dramInit1 & ~SST_DRAMINIT1_STRAP_MASK;
  HwDeviceExtension->agpInit0  = pOemTable->agpInit0;
  HwDeviceExtension->pllCtrl1  = pOemTable->pllCtrl1;
  //HwDeviceExtension->pllCtrl2  = pOemTable->pllCtrl2; // we don't need this
  HwDeviceExtension->sgramMode = pOemTable->sgramMode;


  if (HwDeviceExtension->GraphicsClocking != 0)
  {
    VideoDebugPrint((0, "  Graphics Clock override in registry = %ld MHz\n", HwDeviceExtension->GraphicsClocking));

    // verify the value set is in range
    GRXClock = HwDeviceExtension->GraphicsClocking;
    if (IS_VOODOO3)
    {
      if ((GRXClock > H4_CLOCK_MINIMUM) &&
          (GRXClock < H4_CLOCK_MAXIMUM) &&
          (H4pllTable[GRXClock] != NO_CLOCK))
        HwDeviceExtension->pllCtrl1 = H4pllTable[GRXClock];
    }
    else if (IS_NAPALM)
    {
      if ((GRXClock > H5_CLOCK_MINIMUM) &&
          (GRXClock < H5_CLOCK_MAXIMUM) &&
          (H4pllTable[GRXClock] != NO_CLOCK))
        HwDeviceExtension->pllCtrl1 = H4pllTable[GRXClock];
		}

    VideoDebugPrint((0, "  new pllCtrl1     = %08lXh\n", HwDeviceExtension->pllCtrl1));
  }

  if ((IS_NAPALM) && (*(((FxU16*)(pOemTable)-1)) >= FIRSTOEMSTRUCTCOMPATVERSION))
  {
    // if it's an SDRAM Napalm, set bit 15 of miscInit1 (disable 2D block write)
    if (0x1 == (BIOS_BOARDCONFIG_MEMORYTYPE & pOemTable->BoardConfig))
    {
      HwDeviceExtension->miscInit1 |= SST_DISABLE_2D_BLOCK_WRITE;
    }
  }
}

VP_STATUS
H3RecordChipType(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	)
/*++

Routine Description:

	This routine should only be called if we found a PCI card.
	The routine will fill in the ChipType field of the
	HwDeviceExtension. Fill in the capabilities bits for the
	card, and set a wide character string representing the chip.

	There's no way to distinguish between A2 and A3 boards.

Arguments:

	hwDeviceExtension - Pointer to the miniport's device extension.

Return Value:

	NO_ERROR			- all is well, and the card's chip type and
						  other information was correctly collected.

	ERROR_DEV_NOT_EXIST - we couldn't detect the banshee, and we're in
						  disorder.
--*/

{
#define MAX_ROM_SCAN	512 	// Size of the ROM we map in

#if VARIANTS_EXIST
	BOOLEAN DetectH3 = TRUE;
#endif

	PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
	PUCHAR pIO = (PUCHAR) hwDeviceExtension->MappedAddress[SST_IO_INDEX];
	PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) hwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
	BOOLEAN extensionsDisabled = FALSE;
	PVOID romAddress;
	PCI_COMMON_CONFIG PCIBuffer;
	PUCHAR pucString;
	PULONG pulString;
	ULONG ulROMbase;
	ULONG k, lCount;
	USHORT swzBiosVersion[MAX_BIOS_VERSION_LENGTH] = TDFX_ROM_EARLY_VERSION;
	// these things we save in the registry
	PWSTR pwszChip;
	ULONG cbChip;
	PWSTR pwszAdapterString;
	ULONG cbAdapterString;
	PWSTR pwszAdapterType;
	ULONG cbAdapterType;
	PWSTR pwszDAC = (PWSTR) DEFAULT_DAC_STRING;
	ULONG cbDAC = sizeof(DEFAULT_DAC_STRING);
	PWSTR pwszBIOS;
	ULONG cbBIOS;
	POEMTABLE pOEMConfig;
	FxU16* pFixSpot;
	FxU16* pROMConfig;
    char * pChar;
	USHORT wszAdapterString[MAX_BIOS_VERSION_LENGTH + 5]; // (40 + 4) printable + 1 null = 45
	USHORT wszChipString[MAX_BIOS_VERSION_LENGTH];

	VideoDebugPrint((1, "H3RecordChipType -\n"));

	HwDeviceExtension->FixedFrequencyTable = H4GenericFrequencyTable;
	HwDeviceExtension->Int10FrequencyTable = H4GenericInt10FrequencyTable;

	//
	// We're assured of the following capabilities
	//
	if (HwDeviceExtension->UseSoftwareCursor)
		HwDeviceExtension->Capabilities = CAPS_SW_POINTER;
	else
		HwDeviceExtension->Capabilities = 0;

	//
	// For the unattended installation process, we don't need to check the
	// BIOS, since the driver will be coming up again with this flag cleared
	// and we will be able to access the BIOS correctly then. In this case,
	// just say that the BIOS is unreadable, and we'll get it on the reboot.
	//
#if ENABLE_UNATTENDED_INSTALL_CHECK
	if (HwDeviceExtension->UnattendedInstall)
	{
		HwDeviceExtension->BiosVersion[0] = '\0';
	}
	else
#endif
	{
		//
		// Determine if a BIOS is present.
		//
		// NOTE: At this point we have detected a supported board
		//
		// Look for a 3DFX name signature in the ROM. If we find
		// one, then we can set the modes and refresh rate to nice values.
		//
		// Before we can do that, we must remap the BIOS by setting the
		// PCI Address decode to map it into the beginning of the frame
		// buffer. Once this is done, we collect the string, and then
		// restore the decoding address for the BIOS.

		//
		// Get PCI Config Space for this board
		//
		VideoPortGetBusData(HwDeviceExtension,
							PCIConfiguration,
							HwDeviceExtension->PCISlot.u.AsULONG,
							&PCIBuffer,
							0,
							PCI_COMMON_HDR_LENGTH);

		// save the original ROM address
		ulROMbase = PCIBuffer.u.type0.ROMBaseAddress;


		// Use the frame buffer mapping to overlay the ROM
		PCIBuffer.u.type0.ROMBaseAddress =  (PCIBuffer.u.type0.BaseAddresses[0] + BIOS_REMAP_LOCATION);
		PCIBuffer.u.type0.ROMBaseAddress &= ~0x1;

		VideoDebugPrint((0,"New PCI ROM Address           = 0x%08lx\n", PCIBuffer.u.type0.ROMBaseAddress));
		VideoPortSetBusData(HwDeviceExtension,
							PCIConfiguration,
							HwDeviceExtension->PCISlot.u.AsULONG,
							&PCIBuffer,
							0,
							PCI_COMMON_HDR_LENGTH);

		PCIBuffer.u.type0.ROMBaseAddress |= 0x1;

		VideoPortSetBusData(HwDeviceExtension,
							PCIConfiguration,
							HwDeviceExtension->PCISlot.u.AsULONG,
							&PCIBuffer,
							0,
							PCI_COMMON_HDR_LENGTH);

#if REDUCED_MEMORY_MAPPINGS
    {
      PHYSICAL_ADDRESS  physAddr;


      physAddr.LowPart = PCIBuffer.u.type0.BaseAddresses[0] + BIOS_REMAP_LOCATION;
      physAddr.HighPart = 0;
      pucString = VideoPortGetDeviceBase(HwDeviceExtension,
                                         physAddr,
                                         64 * 1024,
                                         HwDeviceExtension->AccessRanges[0].RangeInIoSpace);
      pulString = (PULONG) pucString;
      if (NULL == pucString)
      {
        VideoDebugPrint((0, "H3RecordChipType: GetDeviceBase failed"));
        HwDeviceExtension->BiosSize = 0;
      }
      else
        HwDeviceExtension->BiosSize = pucString[2] << 7;
    }
#else
		pucString = (PUCHAR) ((ULONG)HwDeviceExtension->MappedAddress[MEMBASE_ZERO] + BIOS_REMAP_LOCATION);
		pulString = (PULONG) pucString;

		//
		// Now we're set up to read the BIOS
		// The size of the BIOS (in kb) is located in the third byte
		//
		HwDeviceExtension->BiosSize = pucString[2] << 7;
#endif

        if (IS_VOODOO3)
        {
            //
            // Collect the string (if any) for later use...
            //
            for (lCount = 0; lCount < HwDeviceExtension->BiosSize; lCount++)
            {
                if ((BIOS_STRING_1 == *(PULONG)((PCHAR)pulString + lCount)) &&
                        (BIOS_STRING_2 == *(PULONG)((PCHAR)pulString + lCount + sizeof(ULONG))))
                {
                    pucString = (PUCHAR)pulString + lCount + (2*sizeof(ULONG)) ;

                    for (k = 0; k < MAX_BIOS_VERSION_LENGTH - 1; k++)
                        HwDeviceExtension->BiosVersion[k] = pucString[k];

                    HwDeviceExtension->BiosVersion[MAX_BIOS_VERSION_LENGTH] = '\0';

                    break;
                }
            }
        }
        else if (IS_NAPALM)
        {
            // follow pointers to the OEMConfig table in BIOS
            #if REDUCED_MEMORY_MAPPINGS
            pFixSpot   = (FxU16*)((ULONG)pulString + ROM_CONFIG);
            pROMConfig = (FxU16*)((ULONG)pulString + *pFixSpot);
            pOEMConfig = (POEMTABLE)((ULONG)pulString + *pROMConfig);
            #else
            pFixSpot = (FxU16*)((ULONG)HwDeviceExtension->MappedAddress[MEMBASE_ZERO] + BIOS_REMAP_LOCATION + ROM_CONFIG);
            pROMConfig = (FxU16*)((ULONG)HwDeviceExtension->MappedAddress[MEMBASE_ZERO] + BIOS_REMAP_LOCATION + *pFixSpot);
            pOEMConfig = (POEMTABLE)((ULONG)HwDeviceExtension->MappedAddress[MEMBASE_ZERO] + BIOS_REMAP_LOCATION + *pROMConfig);
            #endif

            // Test if the OEMConfig table version supports the OEM product name and OEM chip name feature
            // The OEMConfig table version is the word in front of the structure so convert the pointer to the
            // structure to a word pointer and back up one word.
            if (*(((FxU16*)(pOEMConfig)-1)) >= FIRSTOEMSTRUCTCOMPATVERSION)
            {
                // pick up Version Number pointed to by OEMConfig table in BIOS
                #if REDUCED_MEMORY_MAPPINGS
                pChar = (char*)((ULONG)pulString + pOEMConfig->OEMBiosVersion);
                #else
                pChar = (char*)((ULONG)HwDeviceExtension->MappedAddress[MEMBASE_ZERO] + BIOS_REMAP_LOCATION + pOEMConfig->OEMBiosVersion);
                #endif
                for (k = 0; (k < MAX_BIOS_VERSION_LENGTH - 1) && (pChar[k]); k++)
                {
                    HwDeviceExtension->BiosVersion[k] = pChar[k];
                }
                HwDeviceExtension->BiosVersion[k] = '\0';
            }
        }

#if DBG
		VideoDebugPrint((0,"PCI BaseAddress[0] (modified) = 0x%08lx\n", (PCIBuffer.u.type0.BaseAddresses[0] + BIOS_REMAP_LOCATION)));
		VideoDebugPrint((0,"PCI ROM Address               = 0x%08lx\n", ulROMbase));
		VideoDebugPrint((0,"Bios Size                     = %05ld\n", HwDeviceExtension->BiosSize));
		VideoDebugPrint((0,"Version String                = \"%s\"\n", HwDeviceExtension->BiosVersion));
#endif
	}

	if (HwDeviceExtension->BiosVersion[0] == '\0')
	{
		//
		// No BIOS detected, or we're on someone else's bios
		// we need to extract the necessary info for their stuff
		//
		VideoDebugPrint((0, "H3RecordChipType: someone else's bios, or no bios at all!\n"));
		HwDeviceExtension->BiosPresent         = FALSE;
		HwDeviceExtension->UseNonBIOSModeSet   = TRUE;
		HwDeviceExtension->Int10FrequencyTable = NULL;
		HwDeviceExtension->FixedFrequencyTable = H4GenericFrequencyTable;
		HwDeviceExtension->UseNonBIOSModeSet   = TRUE;
		pwszBIOS                               = (PWSTR) NO_ROM_STRING;
		cbBIOS                                 = sizeof(NO_ROM_STRING);
		//
		// XXX more needs to be done here!
		//
	}
	else
	{
		//
		// There's a BIOS. Are we allowed to use it?
		//
		if (VideoPortScanRom(hwDeviceExtension,
							 pucString,
							 (ULONG) ROM_LENGTH,
							 (PUCHAR) TDFX_ROM_SIGNATURE))
		{
			VideoDebugPrint((1, "Found a BIOS with a 3Dfx Interactive signature!\n"));
			HwDeviceExtension->BiosPresent = TRUE;
		}

		if (VideoPortScanRom(HwDeviceExtension,
							 pucString,
							 (ULONG) ROM_LENGTH,
							 (PUCHAR) TDFX_ROM_ORIGINAL_VERSION))
		{
			VideoDebugPrint((1, "\nBIOS Version 1.00.01 (old BIOS)\n"));
			HwDeviceExtension->OldBiosPresent = TRUE;
		}
		else
			VideoDebugPrint((1, "\nBIOS Version 1.00.03 or later (newer BIOS)\n"));

		//
		// Here we do a little string magic, to copy the version string. We exploit the
		// knowledge that the string is null if there's no BIOS, and that the first space
		// occurs after the version string, but before "Copyright"
		//
		k = 0;
		while ((HwDeviceExtension->BiosVersion[k] != ' ')
	    &&     ((k + 19) < MAX_BIOS_VERSION_LENGTH))
		{
			swzBiosVersion[19 + k] = (USHORT) HwDeviceExtension->BiosVersion[k];
			k++;
		}
		swzBiosVersion[19 + k] = (USHORT) '\0';

#if DBG
		VideoDebugPrint((0,"Version String (as collected) is : \""));
		for (k = 0; k < MAX_BIOS_VERSION_LENGTH; k++)
		{
			VideoDebugPrint((0, "%c", (CHAR) swzBiosVersion[k]));
			if (swzBiosVersion[k] == '\0')
				break;
		}
		VideoDebugPrint((0, "\"\n"));
#endif
		pwszBIOS = (PWSTR) swzBiosVersion;
		cbBIOS   = sizeof(swzBiosVersion);

		if (HwDeviceExtension->IsSecondaryDevice)
		{
			HwDeviceExtension->UseNonBIOSModeSet   = TRUE;
			HwDeviceExtension->Int10FrequencyTable = NULL;
			pwszBIOS = (PWSTR) SECONDARY_DEVICE_NO_ROM;
			cbBIOS	 = sizeof(SECONDARY_DEVICE_NO_ROM);
		}
		else
			HwDeviceExtension->Int10FrequencyTable = H4VariableInt10FrequencyTable;
    GetInitRegsFromBios(HwDeviceExtension, (PUCHAR)pulString);
	}

	//
	// NOTE: At this point we have detected a 3Dfx board was located on the AGP or PCI bus
	//
	// Check the device ID to determine which device we're running on
	//
    HwDeviceExtension->biosBoardConfigInfo = 0;

    if (IS_VOODOO3)
    {
        pwszChip = (PWSTR) H4DEFAULT_CHIP_STRING;
        cbChip   = sizeof(H4DEFAULT_CHIP_STRING);
        // Since 3Dfx/STB is the supplier of Voodoo3 boards
        // it appears that reference boards have a SubVendorID of FFFF
        // and production boards have 3Dfx's PCI Vendor ID
        if (0xFFFF == PCIBuffer.u.type0.SubVendorID)
        {
            pwszAdapterString = (PWSTR) H4DEFAULT_ADAPTER_STRING;
            cbAdapterString   = sizeof(H4DEFAULT_ADAPTER_STRING);
            pwszAdapterType   = (PWSTR) H4DEFAULT_ADAPTER_TYPE;
            cbAdapterType     = sizeof(H4DEFAULT_ADAPTER_TYPE);
        }
        else
        {
            pwszAdapterString = (PWSTR) H4DEFAULT_CHIP_STRING L" Board";
            cbAdapterString   = sizeof(H4DEFAULT_CHIP_STRING L" Board");
            pwszAdapterType   = (PWSTR) H4DEFAULT_CHIP_STRING L" Board";
            cbAdapterType     = sizeof(H4DEFAULT_CHIP_STRING L" Board");
        }
    }
    else if (IS_NAPALM)
    {
        // follow pointers to the OEMConfig table in BIOS
        #if REDUCED_MEMORY_MAPPINGS
        pFixSpot   = (FxU16*)((ULONG)pulString + ROM_CONFIG);
        pROMConfig = (FxU16*)((ULONG)pulString + *pFixSpot);
        pOEMConfig = (POEMTABLE)((ULONG)pulString + *pROMConfig);
        #else
        pFixSpot = (FxU16*)((ULONG)HwDeviceExtension->MappedAddress[MEMBASE_ZERO] + BIOS_REMAP_LOCATION + ROM_CONFIG);
        pROMConfig = (FxU16*)((ULONG)HwDeviceExtension->MappedAddress[MEMBASE_ZERO] + BIOS_REMAP_LOCATION + *pFixSpot);
        pOEMConfig = (POEMTABLE)((ULONG)HwDeviceExtension->MappedAddress[MEMBASE_ZERO] + BIOS_REMAP_LOCATION + *pROMConfig);
        #endif

        // Test if the OEMConfig table version supports the OEM product name and OEM chip name feature.
        // The OEMConfig table version is the word in front of the structure so convert the pointer to the
        // structure to a word pointer and back up one word.
        if (*(((FxU16*)(pOEMConfig)-1)) >= FIRSTOEMSTRUCTCOMPATVERSION)
        {
            ULONG spaceCnt = 0;
            ULONG i;

            // pick up Product Name pointed to by OEMConfig table in BIOS
            #if REDUCED_MEMORY_MAPPINGS
            pChar = (char*)((ULONG)pulString + pOEMConfig->OEMBoardName);
            #else
            pChar = (char*)((ULONG)HwDeviceExtension->MappedAddress[MEMBASE_ZERO] + BIOS_REMAP_LOCATION + pOEMConfig->OEMBoardName);
            #endif
            for (k = 0, i = 0; (i < MAX_BIOS_VERSION_LENGTH - 1) && (pChar[i]); k++, i++)
            {
                // construct wide characters from ascii characters on the fly
                ((char *)wszAdapterString)[(k*2)] = pChar[i];
                ((char *)wszAdapterString)[(k*2)+1] = '\0';
                if (' ' == pChar[i])
                {
                  spaceCnt++;
                  if (2 == spaceCnt)
                  {
                    if (HwDeviceExtension->PciSpeed == 66)
                    {
                      wszAdapterString[++k] = 'A';
                      wszAdapterString[++k] = 'G';
                      wszAdapterString[++k] = 'P';
                    }
                    else
                    {
                      wszAdapterString[++k] = 'P';
                      wszAdapterString[++k] = 'C';
                      wszAdapterString[++k] = 'I';
                    }
                    wszAdapterString[++k] = ' ';
                  }
                }
            }

            if (2 > spaceCnt)
            {
              // it seems the string from the bios ends with a ' ' char, but lets check for one anyway
              if (' ' != pChar[i])
                wszAdapterString[k++] = ' ';

              // Append "PCI" or "AGP" to string depending on card type. 
              if (HwDeviceExtension->PciSpeed == 66)
              {
                wszAdapterString[k++] = 'A';
                wszAdapterString[k++] = 'G';
                wszAdapterString[k++] = 'P';
              }
              else
              {
                wszAdapterString[k++] = 'P';
                wszAdapterString[k++] = 'C';
                wszAdapterString[k++] = 'I';
              }
            }
            wszAdapterString[k++] = 0;

            pwszAdapterString = wszAdapterString;
            cbAdapterString   = k*2;
            pwszAdapterType   = wszAdapterString;
            cbAdapterType     = k*2;

            // pick up Chip Name pointed to by OEMConfig table in BIOS
            #if REDUCED_MEMORY_MAPPINGS
            pChar = (char*)((ULONG)pulString + pOEMConfig->OEMChipName);
            #else
            pChar = (char*)((ULONG)HwDeviceExtension->MappedAddress[MEMBASE_ZERO] + BIOS_REMAP_LOCATION + pOEMConfig->OEMChipName);
            #endif
            for (k = 0; (k < MAX_BIOS_VERSION_LENGTH - 1) && (pChar[k]); k++)
            {
                // construct wide characters from ascii characters on the fly
                ((char *)wszChipString)[(k*2)] = pChar[k];
                ((char *)wszChipString)[(k*2)+1] = '\0';
            }
            wszChipString[k++] = 0;

            pwszChip          = wszChipString;
            cbChip            = k*2;

            HwDeviceExtension->biosBoardConfigInfo = pOEMConfig->BoardConfig;
        }
        else
        {
            // OEMConfig table version is 2 or less, it doesn't support this feature.
            pwszChip = (PWSTR) UNKNOWN_STRING;
            cbChip   = sizeof(UNKNOWN_STRING);
            pwszAdapterString = (PWSTR) UNKNOWN_STRING L" Board";
            cbAdapterString   = sizeof(UNKNOWN_STRING L" Board");
            pwszAdapterType   = (PWSTR) UNKNOWN_STRING L" Board";
            cbAdapterType     = sizeof(UNKNOWN_STRING L" Board");
        }
    }
    else
    {
        // Not a valid device
        //
        // Before we puke, restore the original value for the ROM
        // address (even if it usually is zero) to preserve order
        //
        if (HwDeviceExtension->BiosPresent)
        {
            PCIBuffer.u.type0.ROMBaseAddress = ulROMbase;
            VideoPortSetBusData(HwDeviceExtension,
                    PCIConfiguration,
                    HwDeviceExtension->PCISlot.u.AsULONG,
                    &PCIBuffer,
                    0,
                    PCI_COMMON_HDR_LENGTH);
        }
        return ERROR_DEV_NOT_EXIST;
    }

#if REDUCED_MEMORY_MAPPINGS
  // pucString was modified above so we'll release pulString
  if (NULL != pulString)
    VideoPortFreeDeviceBase(HwDeviceExtension,pulString);
#endif

    //
	// We now have a complete description of the hardware.
	// Save the information to the registry so it can be used by
	// configuration programs - such as the display applet
	//

	VideoPortSetRegistryParameters(hwDeviceExtension,
								   (PWSTR)L"HardwareInformation.ChipType",
								   pwszChip,
								   cbChip);

	VideoPortSetRegistryParameters(hwDeviceExtension,
								   (PWSTR)L"HardwareInformation.DACType",
								   pwszDAC,
								   cbDAC);

	VideoPortSetRegistryParameters(hwDeviceExtension,
								   (PWSTR)L"HardwareInformation.AdapterString",
								   pwszAdapterString,
								   cbAdapterString);

	VideoPortSetRegistryParameters(HwDeviceExtension,
								   (PWSTR)L"HardwareInformation.AdapterType",
								   pwszAdapterType,
								   cbAdapterType);

	VideoPortSetRegistryParameters(HwDeviceExtension,
								   (PWSTR)L"HardwareInformation.BiosInformation",
								   pwszBIOS,
								   cbBIOS);

	VideoPortSetRegistryParameters(HwDeviceExtension,
								   (PWSTR)L"HardwareInformation.BiosString",
								   pwszBIOS,
								   cbBIOS);
	//
	// Restore the original value for the ROM address (even if it usually is zero)
	//
	PCIBuffer.u.type0.ROMBaseAddress = ulROMbase;
	VideoPortSetBusData(HwDeviceExtension,
						PCIConfiguration,
						HwDeviceExtension->PCISlot.u.AsULONG,
						&PCIBuffer,
						0,
						PCI_COMMON_HDR_LENGTH);
	return NO_ERROR;
}

#if REDUCED_MEMORY_MAPPINGS
VP_STATUS
H3MapAccessRanges(PHW_DEVICE_EXTENSION HwDeviceExtension)
{
  static const ULONG SparseMemBase0Map[] =
  {
    SST_IO_OFFSET,                            // SST_IO_REGS_INDEX      aka MEMBASE_ZERO
    0,                                        // SST_FB_INDEX           aka MEMBASE_ONE
    0,                                        // SST_IO_INDEX           aka IOBASE_ZERO
    SST_CMDAGP_OFFSET,                        // SST_CMDAGP_REGS_INDEX
    SST_2D_OFFSET,                            // SST_2D_REGS_INDEX
    SST_3D_OFFSET,                            // SST_3D_REGS_INDEX
    SST_YUV_OFFSET                            // SST_YUV_PLANAR_INDEX
  };
  static const ULONG SparseMemBase0Size[] =
  {
    (sizeof(SstIORegs) + 4095) & ~4095,       // SST_IO_REGS_INDEX      aka MEMBASE_ZERO
    0,                                        // SST_FB_INDEX           aka MEMBASE_ONE
    0,                                        // SST_IO_INDEX           aka IOBASE_ZERO
    (sizeof(SstCRegs)  + 4095) & ~4095,       // SST_CMDAGP_REGS_INDEX
    (sizeof(SstGRegs)  + 4095) & ~4095,       // SST_2D_REGS_INDEX
    (sizeof(SstRegs)   + 4095) & ~4095,       // SST_3D_REGS_INDEX
    (0xE80000 - 0xC00000)                     // SST_YUV_PLANAR_INDEX   only 2.5MB of the YUV planar region is useful
  };

  VP_STATUS         status = ERROR_INVALID_PARAMETER;
  PHYSICAL_ADDRESS  physAddr;
  ULONG             i;


  VideoDebugPrint((1, "H3MapAccessRanges\n"));

  // Check to see if there is a hardware resource conflict.
  status = VideoPortVerifyAccessRanges(HwDeviceExtension,
                                       NUM_H3_ACCESS_RANGES,
                                       HwDeviceExtension->AccessRanges);
  if (status != NO_ERROR)
  {
    VideoDebugPrint((0, "H3MapAccessRanges: VerifyAccessRanges failed, status=%08lXh\n", status));
    return status;
  }

#if (_WIN32_WINNT >= 0x0500) || (SHARE_KERNEL_MAPS_WITH_DISPLAY)
  // allocate the frame buffer as WC here and we'll just share the pointer
  // with the gdi driver when it calls StartIO(MAP_VIDEO_MEMORY)
  HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeInIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;
#endif

  // map the master here

  // map the io regs of the memory mapped registers first
  // so we can see how much memory is on the board
  memcpy(&physAddr,
         &HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeStart,
         sizeof(PHYSICAL_ADDRESS));

  physAddr.LowPart += SparseMemBase0Map[SST_IO_REGS_INDEX];

  HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX] = VideoPortGetDeviceBase(HwDeviceExtension,
                                                                               physAddr,
                                                                               SparseMemBase0Size[SST_IO_REGS_INDEX],
                                                                               HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeInIoSpace);
  if (NULL == HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX])
  {
    VideoDebugPrint((0, "H3MapAccessRanges: GetDeviceBase mapping failed on membase0, io regs\n"));
    return ERROR_INVALID_PARAMETER;
  }

  // find out how big the frame buffer is
  status = H3DetermineMemorySize(HwDeviceExtension);
  if (status != NO_ERROR)
  {
    VideoDebugPrint((0, "H3MapAccessRanges: H3DetermineMemorySize failed, status=%08lXh\n", status));
    return status;
  }

  //
  // V56K-256MB: everything downstream derives from AdapterMemorySize*2 -- the
  // VideoPortGetDeviceBase below, and MEMBASE1_DECODE_SIZE /
  // MEMBASE1_MASTER_TO_SLAVE_SPACING in SLIAA.C (both literally
  // 2*AdapterMemorySize).  Nothing validates it against the BAR PnP actually
  // reserved, so a 256MB VBIOS whose BAR1 was not widened would make the chip
  // decode 128MB out of a 64MB window and park the slaves on addresses nobody
  // owns -- a bus hang before video comes up.  Clamp it, rounded DOWN to a
  // power of two: any other value falls through GetDecodeSize()'s default arm
  // to a 128MB decode, i.e. exactly the failure this guard exists to prevent.
  //
  {
    ULONG bar1Len    = HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeLength;
    ULONG maxPerChip = bar1Len >> 1;          // need 2x for the tiled aperture
    ULONG pow2       = 0x00400000;            // 4MB floor

    while ((pow2 << 1) <= maxPerChip)
      pow2 <<= 1;

    /* V56K-MEMOBS: MEASURE BEFORE ACTING.  The 256MB guard is real -- nothing
    ** validates AdapterMemorySize against the BAR that must hold 2x it -- but an
    ** earlier version of it CLAMPED on a value this code had never actually
    ** observed, because VideoDebugPrint is compiled out of the free build.  So
    ** publish the numbers to the registry (readable with one REGREAD, the same
    ** trick used for Retro3dfxSli*) and change NOTHING until they have been read
    ** off a cold, healthy board.  MemBase1Length is captured in FindAdapter
    ** (H3.C:916), so it is definitely valid here; AccessRanges is recorded too so
    ** the two can be compared. */
    {
      ULONG obsBar1  = HwDeviceExtension->MemBase1Length;
      ULONG obsRange = bar1Len;
      ULONG obsChip  = HwDeviceExtension->AdapterMemorySize;
      ULONG obsWould = (obsChip > pow2) ? pow2 : 0;   /* 0 = clamp would NOT fire */

      VideoPortSetRegistryParameters(HwDeviceExtension, L"Retro3dfxMemPerChip",
                                     &obsChip, sizeof(obsChip));
      VideoPortSetRegistryParameters(HwDeviceExtension, L"Retro3dfxMemBar1Len",
                                     &obsBar1, sizeof(obsBar1));
      VideoPortSetRegistryParameters(HwDeviceExtension, L"Retro3dfxMemRangeLen",
                                     &obsRange, sizeof(obsRange));
      VideoPortSetRegistryParameters(HwDeviceExtension, L"Retro3dfxMemWouldClamp",
                                     &obsWould, sizeof(obsWould));
      VideoPortSetRegistryParameters(HwDeviceExtension, L"Retro3dfxMemUnits",
                                     &HwDeviceExtension->numUnits,
                                     sizeof(HwDeviceExtension->numUnits));
    }

    VideoDebugPrint((0, "3dfx MEM: perChip=%08lXh bar1Len=%08lXh need2x=%08lXh units=%ld total=%08lXh\n",
                     HwDeviceExtension->AdapterMemorySize,
                     bar1Len,
                     HwDeviceExtension->AdapterMemorySize * 2,
                     HwDeviceExtension->numUnits,
                     HwDeviceExtension->AdapterMemorySize * HwDeviceExtension->numUnits));
  }

  // map frame buffer
  VideoDebugPrint((0, "MemBase1 AccessRange\n"));
  VideoDebugPrint((0, "  RangeStart=%lX%08lXh  Length=%08lXh  InIoSpace=%lXh\n",
                   HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart.HighPart,
                   HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart.LowPart,
                   HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeLength,
                   HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeInIoSpace));

#if (_WIN32_WINNT >= 0x0500)
  // map 2x the frame buffer size, this is required for tiled mode support
  HwDeviceExtension->MappedAddress[SST_FB_INDEX] = VideoPortGetDeviceBase(HwDeviceExtension,
                                                                          HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart,
                                                                          HwDeviceExtension->AdapterMemorySize * 2,
                                                                          HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeInIoSpace);
#else
  // map 1x the frame buffer size, the NT4 driver doesn't support tiled memory
  HwDeviceExtension->MappedAddress[SST_FB_INDEX] = VideoPortGetDeviceBase(HwDeviceExtension,
                                                                          HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart,
                                                                          HwDeviceExtension->AdapterMemorySize,
                                                                          HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeInIoSpace);
#endif
  if (NULL == HwDeviceExtension->MappedAddress[SST_FB_INDEX])
  {
    VideoDebugPrint((0, "H3MapAccessRanges: GetDeviceBase mapping failed on membase1\n"));
    return ERROR_INVALID_PARAMETER;
  }

  VideoDebugPrint((0, "MemBase1 Mapped Range\n"));
#if (_WIN32_WINNT >= 0x0500)
  VideoDebugPrint((0, "  PhysAddr=%lX%08lXh -> MappedAddress=%08lX  Length=%08lXh\n",
                   HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart.HighPart,
                   HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart.LowPart,
                   HwDeviceExtension->MappedAddress[SST_FB_INDEX],
                   HwDeviceExtension->AdapterMemorySize * 2));
#else
  VideoDebugPrint((0, "  PhysAddr=%lX%08lXh -> MappedAddress=%08lX  Length=%08lXh\n",
                   HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart.HighPart,
                   HwDeviceExtension->AccessRanges[MEMBASE_ONE].RangeStart.LowPart,
                   HwDeviceExtension->MappedAddress[SST_FB_INDEX],
                   HwDeviceExtension->AdapterMemorySize));
#endif

  // map io space, only 256 bytes
  VideoDebugPrint((0, "IOBase AccessRange\n"));
  VideoDebugPrint((0, "  RangeStart=%lX%08lXh  Length=%08lXh  InIoSpace=%lXh\n",
                   HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeStart.HighPart,
                   HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeStart.LowPart,
                   HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeLength,
                   HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeInIoSpace));

  HwDeviceExtension->MappedAddress[SST_IO_INDEX] = VideoPortGetDeviceBase(HwDeviceExtension,
                                                                          HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeStart,
                                                                          256,
                                                                          HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeInIoSpace);
  if (NULL == HwDeviceExtension->MappedAddress[SST_IO_INDEX])
  {
    VideoDebugPrint((0, "H3MapAccessRanges: DeviceBase mapping failed on io space\n"));
    return ERROR_INVALID_PARAMETER;
  }

  VideoDebugPrint((0, "IOBase Mapped Range -\n"));
  VideoDebugPrint((0, "  PhysAddr=%lX%08lXh -> MappedAddress=%08lX  Length=%08lXh\n",
                   HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeStart.HighPart,
                   HwDeviceExtension->AccessRanges[IOBASE_ZERO].RangeStart.LowPart,
                   HwDeviceExtension->MappedAddress[SST_IO_INDEX],
                   256));

  // sparsely map the rest of the memory mapped registers
  VideoDebugPrint((0, "MemBase0 AccessRange\n"));
  VideoDebugPrint((0, "  RangeStart=%lX%08lXh  Length=%08lXh  InIoSpace=%lXh\n",
                   HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeStart.HighPart,
                   HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeStart.LowPart,
                   HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeLength,
                   HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeInIoSpace));
  VideoDebugPrint((0, "MemBase0 Mapped Range\n"));
  VideoDebugPrint((0, "  PhysAddr=%lX%08lXh -> MappedAddress=%08lX  Length=%08lXh\n",
                   physAddr.HighPart, physAddr.LowPart,
                   HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX],
                   SparseMemBase0Size[SST_IO_REGS_INDEX]));

  for (i = SST_CMDAGP_REGS_INDEX; i < SST_MAX_INDEX; i++)
  {
    memcpy(&physAddr,
           &HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeStart,
           sizeof(PHYSICAL_ADDRESS));

    physAddr.LowPart += SparseMemBase0Map[i];

    HwDeviceExtension->MappedAddress[i] = VideoPortGetDeviceBase(HwDeviceExtension,
                                                                 physAddr,
                                                                 SparseMemBase0Size[i],
                                                                 HwDeviceExtension->AccessRanges[MEMBASE_ZERO].RangeInIoSpace);
    if (NULL == HwDeviceExtension->MappedAddress[i])
    {
      VideoDebugPrint((0, "H3MapAccessRanges: GetDeviceBase mapping failed on membase0, i=%ld\n", i));
      return ERROR_INVALID_PARAMETER;
    }

    VideoDebugPrint((0, "  PhysAddr=%lX%08lXh -> MappedAddress=%08lX  Length=%08lXh\n",
                     physAddr.HighPart, physAddr.LowPart,
                     HwDeviceExtension->MappedAddress[i],
                     SparseMemBase0Size[i]));
  }

  // If we're a secondary device, disallow VGA legacy address decoding
  if (HwDeviceExtension->IsSecondaryDevice)
  {
    PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];

    // Turn on bit 9 to disable response to legacy address decoding.
    RegisterMap->vgaInit0 |= SST_VGA0_LEGACY_DECODE;
    VideoDebugPrint((0, "vgaInit0 legacy address decode disabled!\n"));
  }

  return status;
}
#else
VP_STATUS
H3MapAccessRanges(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	)

/*++

Routine Description:

	This routine is called to verify and map the memeory required for the
	board

Arguments:

	HwDeviceExtension - Supplies the miniport driver's adapter storage. This
		storage is initialized to zero before this call.

	NumAccessRanges - the number of access ranges we need to map

	accessRange - the actual access range information

Return Value:

	This routine must return:

	NO_ERROR - Indicates a host adapter's access ranges were successfully mapped

	ERROR_INVALID_PARAMETER - Indicates an adapter, but the parameters in the
				access range structure conflicted with another device, or was
				corrupt.
--*/

{
	PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
	VP_STATUS status = ERROR_INVALID_PARAMETER;
	ULONG i;

	VideoDebugPrint((1, "H3MapAccessRanges -\n"));

#if (_WIN32_WINNT >= 0x0500) && MS_BUILD && 0
	// NT5 Bld 1959 runs out of kernel mode PTE's and banshee won't load
	// so if we can, we need to limit the amount of memory we map to what
	// we really need
  // memory mapped regs = 4MB + 64kB
  hwDeviceExtension->AccessRanges[0].RangeLength = 4 * 1024 * 1024 + 64 * 1024;
  // frame buffer = 16MB
  hwDeviceExtension->AccessRanges[1].RangeLength = 16 * 1024 * 1024;
#endif

#if ENABLE_UNATTENDED_INSTALL_CHECK
	if (hwDeviceExtension->UnattendedInstall)	// GATEWAY_UNATTENDED_INSTALLATION_SUPPORTED
	{
		VideoDebugPrint((0, "Hard-coding rangelengths for VideoPortVerifyAccessRanges\n"));
		hwDeviceExtension->AccessRanges[0].RangeLength    = H3SmallAccessRanges[0].RangeLength;
		hwDeviceExtension->AccessRanges[0].RangeInIoSpace = H3SmallAccessRanges[0].RangeInIoSpace;
		hwDeviceExtension->AccessRanges[1].RangeLength    = H3SmallAccessRanges[1].RangeLength;
		hwDeviceExtension->AccessRanges[1].RangeInIoSpace = H3SmallAccessRanges[1].RangeInIoSpace;
	}
#endif

	//
	// Check to see if there is a hardware resource conflict.
	//
	status = VideoPortVerifyAccessRanges(hwDeviceExtension,
										 NUM_H3_ACCESS_RANGES,
										 hwDeviceExtension->AccessRanges);

	if (status != NO_ERROR)
	{
		VideoDebugPrint((0, "H3VerifyAccessRanges: Access Range conflict, status = 0x%08lx\n", status));

		return status;
	}

#if ENABLE_UNATTENDED_INSTALL_CHECK
	if (hwDeviceExtension->UnattendedInstall)	// GATEWAY_UNATTENDED_INSTALLATION_SUPPORTED
	{
		VideoDebugPrint((0, "Hard-coding rangelengths for VideoPortGetDeviceBase\n"));
		hwDeviceExtension->AccessRanges[0].RangeLength    = H3SmallAccessRanges[0].RangeLength;
		hwDeviceExtension->AccessRanges[0].RangeInIoSpace = H3SmallAccessRanges[0].RangeInIoSpace;
		hwDeviceExtension->AccessRanges[1].RangeLength    = H3SmallAccessRanges[1].RangeLength;
		hwDeviceExtension->AccessRanges[1].RangeInIoSpace = H3SmallAccessRanges[1].RangeInIoSpace;
	}
#endif

	//
	// Get the mapped addresses for the frame buffer, and all the
	// registers.  We will not map the linear frame buffer or linear BIOS
	// because the miniport does not need to access it.
	//
#if (_WIN32_WINNT >= 0x0500) || (SHARE_KERNEL_MAPS_WITH_DISPLAY)
  // allocate the frame buffer as WC here and we'll just share the pointer
  // with the gdi driver when it calls StartIO(MAP_VIDEO_MEMORY)
#if ENABLE_UNATTENDED_INSTALL_CHECK
  if (!hwDeviceExtension->UnattendedInstall)
#endif
    hwDeviceExtension->AccessRanges[1].RangeInIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;
#endif

#if SLI_AA
  // map the master here

  // 32MB for memory mapped registers
  hwDeviceExtension->MappedAddress[0] = VideoPortGetDeviceBase(hwDeviceExtension,
                                                               hwDeviceExtension->AccessRanges[0].RangeStart,
                                                               hwDeviceExtension->AccessRanges[0].RangeLength,
                                                               hwDeviceExtension->AccessRanges[0].RangeInIoSpace);
  if (NULL == hwDeviceExtension->MappedAddress[0])
  {
    VideoDebugPrint((0, "H3VerifyAccessRanges: DeviceBase mapping failed on membase0\n"));
    return ERROR_INVALID_PARAMETER;
  }

  // find out how big the frame buffer is

  // Size the memory on the Voodoo board, and write the
  // value to the registry
  status = H3DetermineMemorySize(HwDeviceExtension);
  if (status != NO_ERROR)
    return status;

  // 2x the frame buffer size
  hwDeviceExtension->MappedAddress[1] = VideoPortGetDeviceBase(hwDeviceExtension,
                                                               hwDeviceExtension->AccessRanges[1].RangeStart,
                                                               hwDeviceExtension->AdapterMemorySize * 2,
                                                               hwDeviceExtension->AccessRanges[1].RangeInIoSpace);
  if (NULL == hwDeviceExtension->MappedAddress[1])
  {
    VideoDebugPrint((0, "H3VerifyAccessRanges: DeviceBase mapping failed on membase1\n"));
    return ERROR_INVALID_PARAMETER;
  }

  // 256 bytes of io space
  hwDeviceExtension->MappedAddress[2] = VideoPortGetDeviceBase(hwDeviceExtension,
                                                               hwDeviceExtension->AccessRanges[2].RangeStart,
                                                               hwDeviceExtension->AccessRanges[2].RangeLength,
                                                               hwDeviceExtension->AccessRanges[2].RangeInIoSpace);
  if (NULL == hwDeviceExtension->MappedAddress[2])
  {
    VideoDebugPrint((0, "H3VerifyAccessRanges: DeviceBase mapping failed on io space\n"));
    return ERROR_INVALID_PARAMETER;
  }
#else
	for (i = 0; i < NUM_H3_ACCESS_RANGES; i++)
	{
		if ((hwDeviceExtension->MappedAddress[i] =
				VideoPortGetDeviceBase(hwDeviceExtension,
										hwDeviceExtension->AccessRanges[i].RangeStart,
										hwDeviceExtension->AccessRanges[i].RangeLength,
										hwDeviceExtension->AccessRanges[i].RangeInIoSpace)) == NULL)
		{

			VideoDebugPrint((0, "H3VerifyAccessRanges: DeviceBase mapping failed on index=%d\n", i));
			return ERROR_INVALID_PARAMETER;
		}
	}
#endif

#if DBG

	DumpAccessRanges(hwDeviceExtension->AccessRanges, NUM_H3_ACCESS_RANGES);

	for (i = 0; i < NUM_H3_ACCESS_RANGES; i++)
	{
		VideoDebugPrint((3, "AccessRange[%d] (%08lx %08lx), len=%08lx, InIoSpace=%d, MappedAddress=%08lx\n",
			i,
			hwDeviceExtension->AccessRanges[i].RangeStart.LowPart,
			hwDeviceExtension->AccessRanges[i].RangeStart.HighPart,
			hwDeviceExtension->AccessRanges[i].RangeLength,
			hwDeviceExtension->AccessRanges[i].RangeInIoSpace,
			hwDeviceExtension->MappedAddress[i]
			));
	}
#endif

	//
	// If we're a secondary device, disallow VGA legacy address decoding
	//
	if (HwDeviceExtension->IsSecondaryDevice)
	{
		PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) hwDeviceExtension->MappedAddress[MEMBASE_ZERO];

		// Turn on bit 9 to disable response to legacy address decoding.
		RegisterMap->vgaInit0 |= SST_VGA0_LEGACY_DECODE;
		VideoDebugPrint((0, "vgaInit0 legacy address decode disabled!\n"));
	}

	return status;
}
#endif

VP_STATUS
H3DetermineMemorySize(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	)
{
	PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
	PH3_MEMBASE0 RegisterMap;
	ULONG memindex, memvalue;
	ULONG memType;
	ULONG memSize;
	ULONG adapterReportedMemSize;
	ULONG partSize;
	ULONG nChips;
	RegisterMap = (PH3_MEMBASE0) hwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];

	VideoDebugPrint((1, "H3DetermineMemorySize -\n"));

	//
	// Get the size of the video memory.
	//

	memindex = 0;
	memvalue = RegisterMap->dramInit0;
	memType  = RegisterMap->dramInit1;

  if (IS_NAPALM)
  {
    nChips = ((memvalue & SST_SGRAM_NUM_CHIPSETS) == 0) ? 4 : 8;

    switch(memvalue & SST_SGRAM_TYPE_H5)
    {
      case SST_SGRAM_TYPE_8MBIT:   partSize = 8;    break;
      case SST_SGRAM_TYPE_16MBIT:  partSize = 16;   break;
      case SST_SGRAM_TYPE_32MBIT:  partSize = 32;   break;
      case SST_SGRAM_TYPE_64MBIT:  partSize = 64;   break;
      case SST_SGRAM_TYPE_128MBIT: partSize = 128;  break;

      default:
        VideoDebugPrint((0, "H3DetermineMemorySize - Invalid SGRAM type\n"));
        partSize = 8;  		break;  // invalid sgram type!
    }

#if 0
#define Stringize(L)      #L
#define MakeString(M,L)   M(L)
#define MYLINE            MakeString(Stringize,__LINE__)
#define __FILELINE__      __FILE__"(" MYLINE ") "

#pragma message(__FILELINE__ "remove this crazy hack when we get a decent agp board")
    // assume partSize is 64MBit on agp boards
    if (66 == HwDeviceExtension->PciSpeed)
      partSize = 64;
#endif

    memSize = (nChips * partSize) / 8; // in MBytes

    if (memType & SST_MCTL_TYPE_SDRAM)
    {
      hwDeviceExtension->SDRAMPresent = TRUE;
      hwDeviceExtension->SDRAMMemorySize = memSize;

      // Report no SGRAM memory
      hwDeviceExtension->SGRAMMemorySize = 0;
    }
    else
    {
      hwDeviceExtension->SGRAMMemorySize = memSize;
    }

    hwDeviceExtension->AdapterMemorySize = memSize * (1024 * 1024);
  }
  else if (IS_VOODOO3)
  {
    //
    // Check bit 30 of the dramInit1 for the SDRAM type.  If we find
    // it, then we use the "other" method of sizing memory, because
    // the SDRAM shows up as being a single bank of RAM.
    //
    if (memType & SST_MCTL_TYPE_SDRAM)
    {
      VideoDebugPrint((2, "16Mb of SDRAM\n"));
      hwDeviceExtension->SDRAMPresent = TRUE;

      // XXX XXX XXX
      // We currently only do 16Mb SDRAM boards, but this will change
      hwDeviceExtension->SDRAMMemorySize = 16;
      memindex = 2;
      hwDeviceExtension->AdapterMemorySize = gacjMemorySize[memindex];

      // Report no SGRAM memory
      hwDeviceExtension->SGRAMMemorySize = 0;
    }
    else
    {
      //
      // read draminit0 bits 26 and 27 for the memory type and number of
      // sgrams installed. The choices are simple: 4, 8, or 16 meg...
      //

      if (memvalue & SST_SGRAM_TYPE_8MBIT)
      {
        // 8meg chips
        VideoDebugPrint((2, "16Mb SGRAMs\n"));
        hwDeviceExtension->SGRAMMemorySize = 4;
        memindex = 0;
      }
      else if (memvalue & SST_SGRAM_TYPE_16MBIT)
      {
        // 16meg chips
        VideoDebugPrint((2, "16Mb SGRAMs\n"));
        hwDeviceExtension->SGRAMMemorySize = 8;
        memindex++;
      }
      else
      {
        VideoDebugPrint((0, "Unknown SGRAM type!\n"));
      }

      if (memvalue & SST_SGRAM_NUM_CHIPSETS)
      {
        // two sets
        hwDeviceExtension->SGRAMChips = 2;
        memindex++;
      }

      hwDeviceExtension->SGRAMMemorySize *= hwDeviceExtension->SGRAMChips;

      hwDeviceExtension->AdapterMemorySize = gacjMemorySize[memindex];
    }
  }

    if (IS_NAPALM)
	    adapterReportedMemSize = hwDeviceExtension->AdapterMemorySize*HwDeviceExtension->numUnits;
    else
	    adapterReportedMemSize = hwDeviceExtension->AdapterMemorySize;

	VideoPortSetRegistryParameters(hwDeviceExtension,
								   (PWSTR)L"HardwareInformation.MemorySize",
								   &adapterReportedMemSize,
								   sizeof(ULONG));

	VideoDebugPrint((1, "3dfx memory size = %08lXh\n", adapterReportedMemSize));

#if ENABLE_UNATTENDED_INSTALL_CHECK
	if (hwDeviceExtension->UnattendedInstall)
	{
		//
		// Gimmick the memory size so the display driver doesn't puke
		//
		hwDeviceExtension->AdapterMemorySize = hwDeviceExtension->AccessRanges[1].RangeLength;
		VideoDebugPrint((1, "will be reported as = 0x%08x\n\n", hwDeviceExtension->AdapterMemorySize));
	}
#endif

	return NO_ERROR;
}

VOID
H3InitializeSecondaryDevice(PHW_DEVICE_EXTENSION  HwDeviceExtension,
                            PH3_MEMBASE0          RegisterMap,
                            PUCHAR                pIO)
{
  ULONG   ulSavedBits;


  // Check bit 30 of the dramInit1 for the SDRAM type
  // only check this if it's a master device
  if (RegisterMap == HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX])
  {
    if (RegisterMap->dramInit1 & SST_MCTL_TYPE_SDRAM)
      HwDeviceExtension->SDRAMPresent = TRUE;
  }

  // pciInit0
  ulSavedBits = HwDeviceExtension->pciInit0;
  ulSavedBits &= ~SST_PCI_LOWTHRESH;
  if (1 == HwDeviceExtension->numUnits)
    ulSavedBits |= (DEFAULT_PCI_LOWTHRESH_VAL << SST_PCI_LOWTHRESH_SHIFT);
  else
  {
#if PCI_LATENCY_REQUIRED_TO_PASS
    ulSavedBits |= ((DEFAULT_2CHIP_PCI_LOWTHRESH_VAL << SST_PCI_LOWTHRESH_SHIFT) |
                    SST_PCI_FORCE_FB_HIGH);
#else
    ulSavedBits |= (DEFAULT_2CHIP_PCI_LOWTHRESH_VAL << SST_PCI_LOWTHRESH_SHIFT);
#endif
  }
#if PCI_LATENCY_REQUIRED_TO_PASS
  // fix for PCI Latency DriverScenario failure
  ulSavedBits &= ~(SST_PCI_RETRY_INTERVAL | SST_PCI_DISABLE_IO | SST_PCI_DISABLE_MEM);
  if (IS_NAPALM)
  {
    if (66 == HwDeviceExtension->PciSpeed)
    {
      // agp board gets an automatic pass on PCI Latency DriverScenario
      // assuming a FoxFireII is installed in the system
      // so leave these bits enabled in order to pass Win2000 GDI tests on agp boards
      ulSavedBits |= (SST_PCI_DISABLE_IO | SST_PCI_DISABLE_MEM | (0 << SST_PCI_RETRY_INTERVAL_SHIFT));
    }
  }
#else
  ulSavedBits &= ~(SST_PCI_RETRY_INTERVAL | SST_PCI_DISABLE_IO | SST_PCI_DISABLE_MEM);
  ulSavedBits |= (SST_PCI_DISABLE_IO | SST_PCI_DISABLE_MEM | (0 << SST_PCI_RETRY_INTERVAL_SHIFT));
#endif
  RegisterMap->pciInit0 = ulSavedBits;

  // Add wait states if this is an AGP board
  if (HwDeviceExtension->PciSpeed == 66)
    RegisterMap->pciInit0 |= (SST_PCI_READ_WS | SST_PCI_WRITE_WS);

  // Set vgaInit0
  RegisterMap->vgaInit0 = SST_VGA0_EXTENSIONS    |
                          SST_VGA0_WAKEUP_SELECT |
                          SST_VGA0_LEGACY_DECODE;

  // dramInit1
  ulSavedBits = RegisterMap->dramInit1 & SST_DRAMINIT1_STRAP_MASK;
  RegisterMap->dramInit1 = HwDeviceExtension->dramInit1 | ulSavedBits;

  // dramInit0
  ulSavedBits = RegisterMap->dramInit0 & SST_DRAMINIT0_STRAP_MASK;
  RegisterMap->dramInit0 = HwDeviceExtension->dramInit0 | ulSavedBits;

#if 0
  // let's not stomp on this bit
  // Clear number of chipsets if we're using SDRAM
  if (HwDeviceExtension->SDRAMPresent)
    RegisterMap->dramInit0 &= ~SST_SGRAM_NUM_CHIPSETS;
#endif

  // miscInit0
  RegisterMap->miscInit0 = HwDeviceExtension->miscInit0;

  // RAM timings
  RegisterMap->dramData = HwDeviceExtension->sgramMode;
  RegisterMap->dramCommand = 0x00000010D;

  if (HwDeviceExtension->SDRAMPresent == FALSE)
  {
    RegisterMap->dramData = DEFAULT_SGRAM_NOSDRAM;
    RegisterMap->dramCommand = 0x00000010E;
  }

  // miscInit1
  ulSavedBits = RegisterMap->miscInit1 & SST_MISCINIT1_STRAP_MASK;
  RegisterMap->miscInit1 = HwDeviceExtension->miscInit1 | ulSavedBits;

  // agpInit0
  RegisterMap->agpInit = HwDeviceExtension->agpInit0;

  // pllctrl1
  RegisterMap->pllCtrl1 = HwDeviceExtension->pllCtrl1;

  // lfbMemoryConfig
  RegisterMap->lfbMemoryConfig |= SST_RAW_LFB_CONFIGURATION;

  // per Xing Cong, set vidPixelBufThold to 0x10410 for overlay BW
  RegisterMap->vidPixelBufThold = 0x10410;

  //
  // And now we do a little magic...
  //
  VideoPortWritePortUchar((PUCHAR)(pIO + 0xc3), 0x01);
  VideoPortWritePortUchar((PUCHAR)(pIO + 0xc2), 0x76);
  VideoPortWritePortUshort((PUSHORT)(pIO + 0xce), 0x0506);
}

BOOLEAN
H3Initialize(
	PHW_DEVICE_EXTENSION HwDeviceExtension
	)

/*++

Routine Description:

	This routine does one time initialization of the device.

Arguments:

	HwDeviceExtension - Supplies a pointer to the miniport's device extension.

Return Value:


	Always returns TRUE since this routine can never fail.

--*/

{
	PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
	PUCHAR pIO = (PUCHAR) HwDeviceExtension->MappedAddress[SST_IO_INDEX];

	ULONG temp;
	ULONG i, j;
	ULONG GRXClock, MemClock;
    UCHAR ucTemp;

	VideoDebugPrint((0, "H3Initialize -\n"));

    // write registry variable used by "3dfx Tools", this call must be after di_FindDevNode.
    InitializeNbChipsInRegistry(HwDeviceExtension);

	//
	// Reset the board to a default mode. If we're a secondary device, there's extra hoops
	// to jump through
	//
	if (HwDeviceExtension->IsSecondaryDevice)
		H3InitializeSecondaryDevice(HwDeviceExtension,
                                HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX],
                                HwDeviceExtension->MappedAddress[SST_IO_INDEX]);

	// Sometimes NT calls the VGA driver without calling the bios so
	// we must set the hardware to a state that it can pick up from.

	VideoPortWritePortUlong((PULONG)(pIO + VIDPROCCFG), 0);
	VideoPortWritePortUlong((PULONG)(pIO + VGAINIT1), 0);
	temp = VideoPortReadPortUlong((PULONG)(pIO + VGAINIT0));
	temp &= ~BIT(12);		 // turn on VGA's screen refresh.
	VideoPortWritePortUlong((PULONG)(pIO + VGAINIT0), temp);

	temp = VideoPortReadPortUlong((PULONG)(pIO + DACMODE));
	temp &= ~(BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4));
	VideoPortWritePortUlong((PULONG)(pIO + DACMODE), temp);

#if (_WIN32_WINNT < 0x0500)
	// Blank the screen.

	VideoPortWritePortUchar((PUCHAR)(pIO + 0xc4), (UCHAR) 0x01);
	ucTemp = VideoPortReadPortUchar((PUCHAR)(pIO + 0xc5));
	VideoPortWritePortUchar((PUCHAR)(pIO + 0xc5), (UCHAR) (ucTemp | 0x20));
#endif

	// Reset extended timing bits so VGA modes will work.

	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT) 0x001a);
	VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT) 0x001b);

	// Don't set clocks unless they are in the registry.
	// Clocks set in the registry will have nonzero values.

  VideoDebugPrint((0, "  GraphicsClocking = %08lXh\n", HwDeviceExtension->GraphicsClocking));
  VideoDebugPrint((0, "  org pllCtrl1     = %08lXh\n", RegisterMap->pllCtrl1));
	if (HwDeviceExtension->GraphicsClocking != 0)
	{
		// verify the value set is in range
		GRXClock = HwDeviceExtension->GraphicsClocking;
    if (IS_VOODOO3)
		{
			if ((GRXClock > H4_CLOCK_MINIMUM)
			&&  (GRXClock < H4_CLOCK_MAXIMUM)
			&&  (H4pllTable[GRXClock] != NO_CLOCK))
				RegisterMap->pllCtrl1 = H4pllTable[GRXClock];
		}
    else if (IS_NAPALM)
		{
			if ((GRXClock > H5_CLOCK_MINIMUM)
			&&  (GRXClock < H5_CLOCK_MAXIMUM)
			&&  (H4pllTable[GRXClock] != NO_CLOCK))
				RegisterMap->pllCtrl1 = H4pllTable[GRXClock];
		}

    VideoDebugPrint((0, "  new pllCtrl1     = %08lXh\n", RegisterMap->pllCtrl1));
	}

  // set PCI FIFO empty retries low water mark to 0x8
  temp = RegisterMap->pciInit0;
  temp &= ~SST_PCI_LOWTHRESH;
  if (1 == HwDeviceExtension->numUnits)
    temp |= (DEFAULT_PCI_LOWTHRESH_VAL << SST_PCI_LOWTHRESH_SHIFT);
  else
  {
#if PCI_LATENCY_REQUIRED_TO_PASS
    temp |= ((DEFAULT_2CHIP_PCI_LOWTHRESH_VAL << SST_PCI_LOWTHRESH_SHIFT) |
             SST_PCI_FORCE_FB_HIGH);
#else
    temp |= (DEFAULT_2CHIP_PCI_LOWTHRESH_VAL << SST_PCI_LOWTHRESH_SHIFT);
#endif
  }
#if PCI_LATENCY_REQUIRED_TO_PASS
  // fix for PCI Latency DriverScenario failure
  temp &= ~(SST_PCI_RETRY_INTERVAL | SST_PCI_DISABLE_IO | SST_PCI_DISABLE_MEM);
  if (IS_NAPALM)
  {
    if (66 == HwDeviceExtension->PciSpeed)
    {
      // agp board gets an automatic pass on PCI Latency DriverScenario
      // assuming a FoxFireII is installed in the system
      // so leave these bits enabled in order to pass Win2000 GDI tests on agp boards
      temp |= (SST_PCI_DISABLE_IO | SST_PCI_DISABLE_MEM | (0 << SST_PCI_RETRY_INTERVAL_SHIFT));
    }
  }
#else
  temp &= ~(SST_PCI_RETRY_INTERVAL | SST_PCI_DISABLE_IO | SST_PCI_DISABLE_MEM);
  temp |= (SST_PCI_DISABLE_IO | SST_PCI_DISABLE_MEM | (0 << SST_PCI_RETRY_INTERVAL_SHIFT));
#endif
  RegisterMap->pciInit0 = temp;

  // per Xing Cong, set vidPixelBufThold to 0x10410 for overlay BW
  RegisterMap->vidPixelBufThold = 0x10410;

	//
	// Load the CLUT
	//
	temp = RegisterMap->miscInit1;
	temp |= SST_H3_CLUTINVERT;
	RegisterMap->miscInit1 = temp;

#if (_WIN32_WINNT < 0x0500)
	for (i = 0; i < 0x100; i++)
	{
		VideoPortWritePortUlong((PULONG)(pIO + DACADDR), i);
		temp = VideoPortReadPortUlong((PULONG)(pIO + DACADDR));

		j = i & 0xff;
		temp = ((j << 16) | (j << 8) | j);

		VideoPortWritePortUlong((PULONG)(pIO + DACDATA), temp);
		temp = VideoPortReadPortUlong((PULONG)(pIO + DACDATA));
	}

	H3UpdateGamma(HwDeviceExtension, RegisterMap, pIO, HwDeviceExtension->GammaTable);
#endif

#ifdef SLI_AA
  InitializeSlaveChipsInitRegs(HwDeviceExtension);
#endif

	//again, per andy
	//
	// clear bit 10 and 11 in the vidProcCfg register so that the CLUT
	// is not bypassed
	//
	temp = VideoPortReadPortUlong((PULONG)(pIO + VIDPROCCFG));

	temp &= ~SST_BYPASS_CLUT;

	VideoPortWritePortUlong((PULONG)(pIO + VIDPROCCFG), temp);

    QueryForVMIPld( HwDeviceExtension);
#if DBG
  VideoDebugPrint((0, "Access range dump:\n"));
  VideoDebugPrint((0, "Index   --PHYSICAL ADDRESS--   Range Length   InIoSpace\n"));

  for (i = 0; i < NUM_H3_ACCESS_RANGES; i++)
  {
    VideoDebugPrint((0, "  %d  ", i));
    VideoDebugPrint((0, "    0x%08lX%08lX ", HwDeviceExtension->AccessRanges[i].RangeStart.HighPart, HwDeviceExtension->AccessRanges[i].RangeStart.LowPart));
    VideoDebugPrint((0, "    0x%08lX        %d\n", HwDeviceExtension->AccessRanges[i].RangeLength, HwDeviceExtension->AccessRanges[i].RangeInIoSpace));
  }
  VideoDebugPrint((0, "\n"));

  DumpMTRRs(HwDeviceExtension, 0);
#endif

	VideoDebugPrint((0, "H3Initialize - done\n"));

#ifdef TVOUT_SUPPORTED
	HwDeviceExtension->tvOutCapable = (BOOLEAN)BT868_SetupTvOut(HwDeviceExtension);

	// determine monitor status
	temp = VideoPortReadPortUlong((PULONG)(pIO + DACMODE));
	HwDeviceExtension->monitorActive = ((temp & SST_DACMODE_DPMS_BITS) == SST_DACMODE_DPMS_ON);

	// if this is a "customer number 7" driver don't allow simultanous CRT and TvOut.
	if ((HwDeviceExtension->tvOutActive) && (HwDeviceExtension->CustNum == 7))
	    HwDeviceExtension->monitorActive = FALSE;

// DFP stuff
    DFP_Initialize(HwDeviceExtension);
#endif //def TVOUT_SUPPORTED

	// check OS support for KNI instructions
	isP6 = 0;
	TestForP6();

	return TRUE;

} // end H3Initialize()

BOOLEAN
H3ResetHw(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	ULONG Columns,
	ULONG Rows
	)

/*++

Routine Description:

	This routine preps the card for return to a VGA mode.

	This routine is called during system shutdown.	By returning
	a FALSE we inform the HAL to do an int 10 to go into text
	mode before shutting down.	Shutdown could fail with some
	cards without this.

	We do some clean up before returning so that the int 10
	will work.

Arguments:

	HwDeviceExtension - Supplies a pointer to the miniport's device extension.

Return Value:

	The return value of FALSE informs the hal to go into text mode.

--*/

{
	PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
	PUCHAR pIO = (PUCHAR) hwDeviceExtension->MappedAddress[SST_IO_INDEX];
	ULONG temp;
	UCHAR ucTemp;

	UNREFERENCED_PARAMETER(Columns);
	UNREFERENCED_PARAMETER(Rows);

//	VideoDebugPrint((1, "H3ResetHw -\n"));

	//
	// We don't want to execute this reset code if we are not
	// currently in an H3 mode!
	//

	if (!hwDeviceExtension->bNeedReset)
	{
//		VideoDebugPrint((1, "no reset required - done\n"));

		return FALSE;
	}

	//
	// Reset the board to a default mode. If we're a secondary device, there's extra hoops
	// to jump through
	//
	if (HwDeviceExtension->IsSecondaryDevice)
		H3InitializeSecondaryDevice(HwDeviceExtension,
                                HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX],
                                HwDeviceExtension->MappedAddress[SST_IO_INDEX]);

	if (hwDeviceExtension->UseNonBIOSModeSet || hwDeviceExtension->BiosPresent == FALSE)
	{
		// Sometimes NT calls the VGA driver without calling the bios so
		// we must set the hardware to a state that it can pick up from.

		VideoPortWritePortUlong((PULONG)(pIO + VIDPROCCFG), 0);
		VideoPortWritePortUlong((PULONG)(pIO + VGAINIT1), 0);
		temp = VideoPortReadPortUlong((PULONG)(pIO + VGAINIT0));
		temp &= ~BIT(12);		 // turn on VGA's screen refresh.
		VideoPortWritePortUlong((PULONG)(pIO + VGAINIT0), temp);

		temp = VideoPortReadPortUlong((PULONG)(pIO + DACMODE));
		temp &= ~(BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4));
		VideoPortWritePortUlong((PULONG)(pIO + DACMODE), temp);

#if (_WIN32_WINNT < 0x0500)
		// Blank the screen.

		VideoPortWritePortUchar((PUCHAR)(pIO + 0xc4), (UCHAR) 0x01);
		ucTemp = VideoPortReadPortUchar((PUCHAR)(pIO + 0xc5));
		VideoPortWritePortUchar((PUCHAR)(pIO + 0xc5), (UCHAR) (ucTemp | 0x20));
#endif
		// Reset extended timing bits so VGA modes will work.

		VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT) 0x001a);
		VideoPortWritePortUshort((PUSHORT)(pIO + 0xd4), (USHORT) 0x001b);

	}

	hwDeviceExtension->bNeedReset = FALSE;

	return FALSE;
}

/*----------------------------------------------------------------------
Function name:  CRT_SetActiveState

Description:    Enable or Disable the CRT device as selected by value of alternate
                display device mask passed in.

Information:

Return:         FxU32     STB_FUNCTION_VGA if success,
                          0 if the current mode does not allow completion.
----------------------------------------------------------------------*/
FxU32 CRT_SetActiveState (PHW_DEVICE_EXTENSION HwDeviceExtension, FxU32 activeStateMask)
{
    PUCHAR pIO = (PUCHAR) HwDeviceExtension->MappedAddress[SST_IO_INDEX];
    ULONG result = STB_FUNCTION_VGA;
    ULONG ulTemp = VideoPortReadPortUlong((PULONG)(pIO + DACMODE));


    // check if CRT is supposed to be on.
    if ((activeStateMask & STB_FUNCTION_VGA) == STB_FUNCTION_VGA)
    { // CRT should be turned on
        // check if CRT if off and needs to be turned on
        if (!HwDeviceExtension->monitorActive)
        {
            // CRT is off and needs to be turned on.
            ulTemp &= ~SST_DACMODE_DPMS_SUSPEND;
            HwDeviceExtension->monitorActive = TRUE;
            VideoPortWritePortUlong((PULONG)(pIO + DACMODE), ulTemp);
        }

    }
    else
    {// CRT should be turned off
        // check if CRT is on and needs to be turned off
        if (HwDeviceExtension->monitorActive)
        {
            ulTemp |= SST_DACMODE_DPMS_SUSPEND;
            HwDeviceExtension->monitorActive = FALSE;
            VideoPortWritePortUlong((PULONG)(pIO + DACMODE), ulTemp);
        }

    }
    return(result);
}

BOOLEAN
H3StartIO(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	)

/*++

Routine Description:

	This routine is the main execution routine for the miniport driver. It
	acceptss a Video Request Packet, performs the request, and then returns
	with the appropriate status.

Arguments:

	HwDeviceExtension - Supplies a pointer to the miniport's device extension.

	RequestPacket - Pointer to the video request packet. This structure
		contains all the parameters passed to the VideoIoControl function.

Return Value:


--*/

{
	PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
	VP_STATUS status;

	PVIDEO_SHARE_MEMORY pShareMemory;
	PVIDEO_SHARE_MEMORY_INFORMATION pShareMemoryInformation;
	PHYSICAL_ADDRESS shareAddress;
	PVOID virtualAddress;
	ULONG sharedViewSize;
	ULONG inIoSpace;
#ifdef TVOUT_SUPPORTED
	ULONG ulTemp;
	ULONG requestType;
#endif //def TVOUT_SUPPORTED

	PUCHAR pIO = (PUCHAR) hwDeviceExtension->MappedAddress[SST_IO_INDEX];
	PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) hwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];

	//
	// Switch on the IoContolCode in the RequestPacket. It indicates which
	// function must be performed by the driver.
	//

	VideoDebugPrint((1, "H3StartIO - "));
	switch (RequestPacket->IoControlCode)
	{
// START ALT-TAB CHANGES
	case IOCTL_ALLOCATE_NONCACHED_MEMORY:

		VideoDebugPrint((1, "ALLOCATE_NONCACHED_MEMORY\n"));			
		{
			void * pMemory;
			PHYSICAL_ADDRESS sPhysicalAddress;

			if (RequestPacket->InputBufferLength < sizeof(ULONG))
			{
				status = ERROR_INSUFFICIENT_BUFFER;
				break;
			}

			sPhysicalAddress.LowPart = 0xffffffff;
			sPhysicalAddress.HighPart = 0x00;
#ifdef MS_VIEW
			pMemory = VideoPortAllocateContiguousMemory ( hwDeviceExtension, ( ULONG )*( ULONG *)RequestPacket->InputBuffer,
				sPhysicalAddress );
#else			
			pMemory = MmAllocateContiguousMemory((ULONG)*(ULONG*)RequestPacket->InputBuffer, sPhysicalAddress);
#endif
    
			*( ULONG * )RequestPacket->OutputBuffer = ( ULONG )pMemory;
			RequestPacket->StatusBlock->Information = sizeof( ULONG );
			status = NO_ERROR;
		}
		break;

	case IOCTL_MAP_MEMORY_TO_CURRENT_PROCESS:
		VideoDebugPrint((1, "MAP_MEMORY_TO_CURRENT_PROCESS\n"));			
		{
			PHYSICAL_ADDRESS sPhysicalMemory;
			void * pProcessRelativePointer;

			if (RequestPacket->InputBufferLength < ( sizeof(void *) + sizeof( ULONG ) ) )
			{
				status = ERROR_INSUFFICIENT_BUFFER;
				break;
			}

			sPhysicalMemory = MmGetPhysicalAddress( ( void * )*( ULONG * )RequestPacket->InputBuffer );
			pProcessRelativePointer = h3MapKernelMemoryToCurrentProcessSpace(
				sPhysicalMemory, ( ULONG )*( ( ULONG * )RequestPacket->InputBuffer + 1 ) );

			// return a process relative pointer to the memory
			*( ULONG * )RequestPacket->OutputBuffer = ( ULONG )pProcessRelativePointer;
				
			RequestPacket->StatusBlock->Information = sizeof( ULONG );
			status = NO_ERROR;
		}
		break;
// END ALT-TAB CHANGES

	case IOCTL_VIDEO_MAP_VIDEO_MEMORY:

		VideoDebugPrint((1, "MapVideoMemory\n"));
		{
			PVIDEO_MEMORY_INFORMATION memoryInformation;
			ULONG physicalFrameLength;

			if ( (RequestPacket->OutputBufferLength <
				  (RequestPacket->StatusBlock->Information =
										 sizeof(VIDEO_MEMORY_INFORMATION))) ||
				 (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)) )
			{
				status = ERROR_INSUFFICIENT_BUFFER;
				break;
			}

			memoryInformation = RequestPacket->OutputBuffer;

			memoryInformation->VideoRamBase = ((PVIDEO_MEMORY)
					(RequestPacket->InputBuffer))->RequestedVirtualAddress;

#if (_WIN32_WINNT >= 0x0500) || (SHARE_KERNEL_MAPS_WITH_DISPLAY)
      if (NULL == memoryInformation->VideoRamBase)
      {
        memoryInformation->VideoRamBase = hwDeviceExtension->MappedAddress[SST_FB_INDEX];
        status = NO_ERROR;
      }
      else
      {
#endif
			physicalFrameLength = hwDeviceExtension->MemBase1Length;

			inIoSpace = hwDeviceExtension->MemBase1InIOSpace;

			//
			// Generally, we only map the precise amount of memory present on the
			// board.  In our case, we must map the entire address range because
			// we decode the higher addresses of the frame buffer as tiled memory
			//
			// Performance:
			//
			// Enable USWC on the P6 processor.
			// We only do it for the frame buffer - memory mapped registers can
			// not be mapped USWC because write combining the registers would
			// cause very bad things to happen !
			//

#if ENABLE_UNATTENDED_INSTALL_CHECK
			if (!HwDeviceExtension->UnattendedInstall)
#endif
				inIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;

			status = VideoPortMapMemory(hwDeviceExtension,
										hwDeviceExtension->PhysicalMemBaseAddr1,
										&physicalFrameLength,
										&inIoSpace,
										&(memoryInformation->VideoRamBase));
#if (_WIN32_WINNT >= 0x0500) || (SHARE_KERNEL_MAPS_WITH_DISPLAY)
      }
#endif

#if ENABLE_ADDRESS_LIST_ARRAY
			if (status == NO_ERROR)
				H3RefCountAlloc(HwDeviceExtension,
							    (ULONG)memoryInformation->VideoRamBase);
#endif

			//
			// The frame buffer and virtual memory are equivalent in this
			// case.
			//

			memoryInformation->FrameBufferBase	 = memoryInformation->VideoRamBase;
			memoryInformation->VideoRamLength	 = hwDeviceExtension->MemBase1Length;

			//
			// XXX XXX XXX
			// We report the amount of RAM present rather than the size of the
			// address space.  This capability should really be moved to a private IOCTL
			//
			// memoryInformation->FrameBufferLength = hwDeviceExtension->MemBase1Length;
			memoryInformation->FrameBufferLength = hwDeviceExtension->AdapterMemorySize;
		}

		break;


	case IOCTL_VIDEO_UNMAP_VIDEO_MEMORY:

		VideoDebugPrint((1, "UnMapVideoMemory\n"));

		if (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY))
		{
			status = ERROR_INSUFFICIENT_BUFFER;
			break;
		}

#if (_WIN32_WINNT >= 0x0500) || (SHARE_KERNEL_MAPS_WITH_DISPLAY)
    // if the RequestedVirtualAddress is the miniport's framebuffer ptr
    // then there's nothing to do
    if (hwDeviceExtension->MappedAddress[SST_FB_INDEX] ==
        ((PVIDEO_MEMORY)(RequestPacket->InputBuffer))->RequestedVirtualAddress)
    {
      status = NO_ERROR;
    }
    else
    {
#endif
		status = VideoPortUnmapMemory(hwDeviceExtension,
									  ((PVIDEO_MEMORY)
									   (RequestPacket->InputBuffer))->RequestedVirtualAddress,
									  0);
#if (_WIN32_WINNT >= 0x0500) || (SHARE_KERNEL_MAPS_WITH_DISPLAY)
    }
#endif

#if ENABLE_ADDRESS_LIST_ARRAY
		if (status == NO_ERROR)
			H3RefCountFree(HwDeviceExtension,
						   RequestPacket,
						   (ULONG)((PVIDEO_MEMORY)(RequestPacket->InputBuffer))->RequestedVirtualAddress);
#endif
		break;


	case IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES:
			
    VideoDebugPrint((1, "QueryPublicAccessRanges\n"));

		{
      PVIDEO_PUBLIC_ACCESS_RANGES portAccess;
      ULONG physicalPortLength;

#if REDUCED_MEMORY_MAPPINGS
#if (_MAP_FRAMEBUFFER_ONCE == 1)
      RequestPacket->StatusBlock->Information = (SST_MAX_INDEX - 1) * sizeof(VIDEO_PUBLIC_ACCESS_RANGES);
#else
      RequestPacket->StatusBlock->Information = SST_MAX_INDEX * sizeof(VIDEO_PUBLIC_ACCESS_RANGES);
#endif
#else
#if (_MAP_FRAMEBUFFER_ONCE == 1)
			RequestPacket->StatusBlock->Information = 2 * sizeof(VIDEO_PUBLIC_ACCESS_RANGES);
#else
			RequestPacket->StatusBlock->Information = 3 * sizeof(VIDEO_PUBLIC_ACCESS_RANGES);
#endif
#endif
      if (RequestPacket->OutputBufferLength < RequestPacket->StatusBlock->Information)
      {
        VideoDebugPrint((1, "invalid buffer size (%d), expected %d\n",
                         RequestPacket->OutputBufferLength, RequestPacket->StatusBlock->Information));
        status = ERROR_INSUFFICIENT_BUFFER;
        break;
      }

      status = ERROR_INVALID_PARAMETER;

      portAccess = RequestPacket->OutputBuffer;
#if (_WIN32_WINNT >= 0x0500) || (SHARE_KERNEL_MAPS_WITH_DISPLAY)
      // memory mapped io regs
      portAccess->VirtualAddress  = hwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
      portAccess->InIoSpace       = hwDeviceExtension->MemBase0InIOSpace;
      portAccess->MappedInIoSpace = hwDeviceExtension->MemBase0InIOSpace;
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess->VirtualAddress);
#endif
      portAccess++;

#if (_MAP_FRAMEBUFFER_ONCE == 0)
      // frame buffer
      portAccess->VirtualAddress  = hwDeviceExtension->MappedAddress[SST_FB_INDEX];
      portAccess->InIoSpace       = hwDeviceExtension->MemBase1InIOSpace;
      portAccess->MappedInIoSpace = hwDeviceExtension->MemBase1InIOSpace;
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess->VirtualAddress);
#endif
      portAccess++;
#endif

      // io
      portAccess->VirtualAddress  = hwDeviceExtension->MappedAddress[SST_IO_INDEX];
      portAccess->InIoSpace       = hwDeviceExtension->IOBaseInIoSpace;
      portAccess->MappedInIoSpace = hwDeviceExtension->IOBaseInIoSpace;
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess->VirtualAddress);
#endif
#if REDUCED_MEMORY_MAPPINGS
      portAccess++;

      // memory mapped cmdagp regs
      portAccess->VirtualAddress  = hwDeviceExtension->MappedAddress[SST_CMDAGP_REGS_INDEX];
      portAccess->InIoSpace       = hwDeviceExtension->MemBase0InIOSpace;
      portAccess->MappedInIoSpace = hwDeviceExtension->MemBase0InIOSpace;
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess->VirtualAddress);
#endif
      portAccess++;

      // memory mapped 2d regs
      portAccess->VirtualAddress  = hwDeviceExtension->MappedAddress[SST_2D_REGS_INDEX];
      portAccess->InIoSpace       = hwDeviceExtension->MemBase0InIOSpace;
      portAccess->MappedInIoSpace = hwDeviceExtension->MemBase0InIOSpace;
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess->VirtualAddress);
#endif
      portAccess++;

      // memory mapped 3d regs
      portAccess->VirtualAddress  = hwDeviceExtension->MappedAddress[SST_3D_REGS_INDEX];
      portAccess->InIoSpace       = hwDeviceExtension->MemBase0InIOSpace;
      portAccess->MappedInIoSpace = hwDeviceExtension->MemBase0InIOSpace;
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess->VirtualAddress);
#endif
      portAccess++;

      // memory mapped yuv planar
      portAccess->VirtualAddress  = hwDeviceExtension->MappedAddress[SST_YUV_PLANAR_INDEX];
      portAccess->InIoSpace       = hwDeviceExtension->MemBase0InIOSpace;
      portAccess->MappedInIoSpace = hwDeviceExtension->MemBase0InIOSpace;
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess->VirtualAddress);
#endif
#endif

			status = NO_ERROR;
#else
		   portAccess->VirtualAddress  = (PVOID) NULL;	  // Requested VA
		   portAccess->InIoSpace	   = hwDeviceExtension->MemBase0InIOSpace;
		   portAccess->MappedInIoSpace = portAccess->InIoSpace;

		   physicalPortLength = hwDeviceExtension->MemBase0Length;

		   VideoDebugPrint((2, "HwDeviceExtension      %08lx\n", (ULONG)HwDeviceExtension));
		   VideoDebugPrint((2, "Physical Address       %08lx %08lx\n", hwDeviceExtension->PhysicalMemBaseAddr0.HighPart, hwDeviceExtension->PhysicalMemBaseAddr0.LowPart));
		   VideoDebugPrint((2, "Physical Memory Length %08lx\n", (ULONG)physicalPortLength));
		   VideoDebugPrint((2, "InIOSpace              %02x\n", (ULONG)portAccess->MappedInIoSpace));
		   VideoDebugPrint((2, "Virtual Address        %08lx\n", (ULONG)portAccess->VirtualAddress));

		   status = VideoPortMapMemory(hwDeviceExtension,
									   hwDeviceExtension->PhysicalMemBaseAddr0,
									   &physicalPortLength,
									   &(portAccess->MappedInIoSpace),
									   &(portAccess->VirtualAddress));


		   if (status == NO_ERROR)
		   {
#if ENABLE_ADDRESS_LIST_ARRAY
			   H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess->VirtualAddress);
#endif

			   portAccess++;

			   portAccess->VirtualAddress  = (PVOID) NULL;	  // Requested VA
			   portAccess->InIoSpace	   = hwDeviceExtension->MemBase1InIOSpace;
			   //
			   // Since this mapping is the frame buffer, we must enable write
			   // combining for best performance. Do it explicitly.
			   //
#if ENABLE_UNATTENDED_INSTALL_CHECK
				if (!HwDeviceExtension->UnattendedInstall)
#endif
				   portAccess->MappedInIoSpace = portAccess->InIoSpace | VIDEO_MEMORY_SPACE_P6CACHE;
				else
				   portAccess->MappedInIoSpace = portAccess->InIoSpace;

			   physicalPortLength = hwDeviceExtension->MemBase1Length;

			   status = VideoPortMapMemory(hwDeviceExtension,
										   hwDeviceExtension->PhysicalMemBaseAddr1,
										   &physicalPortLength,
										   &(portAccess->MappedInIoSpace),
										   &(portAccess->VirtualAddress));
			   if (status == NO_ERROR)
			   {
#if ENABLE_ADDRESS_LIST_ARRAY
				   H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess->VirtualAddress);
#endif

				   portAccess++;

				   portAccess->VirtualAddress  = (PVOID) NULL;	  // Requested VA
				   portAccess->InIoSpace	   = hwDeviceExtension->IOBaseInIoSpace;
				   portAccess->MappedInIoSpace = portAccess->InIoSpace;

				   physicalPortLength = hwDeviceExtension->IOBaseLength;

				   status = VideoPortMapMemory(hwDeviceExtension,
										   hwDeviceExtension->PhysicalIOBaseAddr,
										   &physicalPortLength,
										   &(portAccess->MappedInIoSpace),
										   &(portAccess->VirtualAddress));

#if ENABLE_ADDRESS_LIST_ARRAY
				   if (status == NO_ERROR)
					   H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess->VirtualAddress);
#endif
				}
			}
#endif
		}

		break;

	case IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES:

    VideoDebugPrint((1, "FreePublicAccessRanges\n"));

		{
      PVIDEO_MEMORY mappedMemory;

#if REDUCED_MEMORY_MAPPINGS
#if (_MAP_FRAMEBUFFER_ONCE == 1)
      RequestPacket->StatusBlock->Information = (SST_MAX_INDEX - 1) * sizeof(VIDEO_MEMORY);
#else
      RequestPacket->StatusBlock->Information = SST_MAX_INDEX * sizeof(VIDEO_MEMORY);
#endif
#else
#if (_MAP_FRAMEBUFFER_ONCE == 1)
			RequestPacket->StatusBlock->Information = 2 * sizeof(VIDEO_MEMORY);
#else
			RequestPacket->StatusBlock->Information = 3 * sizeof(VIDEO_MEMORY);
#endif
#endif
      if (RequestPacket->InputBufferLength < RequestPacket->StatusBlock->Information)
      {
        VideoDebugPrint((1, "invalid buffer size (%d), expected %d\n",
                         RequestPacket->OutputBufferLength, RequestPacket->StatusBlock->Information));

        status = ERROR_INSUFFICIENT_BUFFER;
        break;
      }
			
      status = ERROR_INVALID_PARAMETER;

      // memory mapped io regs
      mappedMemory = RequestPacket->InputBuffer;
#if (_WIN32_WINNT >= 0x0500) || (SHARE_KERNEL_MAPS_WITH_DISPLAY)
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountFree(HwDeviceExtension,
                     RequestPacket,
                     (ULONG)mappedMemory->RequestedVirtualAddress);
#endif

      mappedMemory++;

#if (_MAP_FRAMEBUFFER_ONCE == 0)
      // frame buffer
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountFree(HwDeviceExtension,
                     RequestPacket,
                     (ULONG)mappedMemory->RequestedVirtualAddress);
#endif

			mappedMemory++;
#endif

      // io
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountFree(HwDeviceExtension,
                     RequestPacket,
                     (ULONG)mappedMemory->RequestedVirtualAddress);
#endif
#if REDUCED_MEMORY_MAPPINGS
			mappedMemory++;

      // cmdagp regs
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountFree(HwDeviceExtension,
                     RequestPacket,
                     (ULONG)mappedMemory->RequestedVirtualAddress);
#endif
			mappedMemory++;

      // 2d regs
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountFree(HwDeviceExtension,
                     RequestPacket,
                     (ULONG)mappedMemory->RequestedVirtualAddress);
#endif
			mappedMemory++;

      // 3d regs
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountFree(HwDeviceExtension,
                     RequestPacket,
                     (ULONG)mappedMemory->RequestedVirtualAddress);
#endif
			mappedMemory++;

      // yuv planar
#if ENABLE_ADDRESS_LIST_ARRAY
      H3RefCountFree(HwDeviceExtension,
                     RequestPacket,
                     (ULONG)mappedMemory->RequestedVirtualAddress);
#endif
#endif

      status = NO_ERROR;
#else
			if (mappedMemory->RequestedVirtualAddress != NULL)
			{
				status = VideoPortUnmapMemory(hwDeviceExtension,
											  mappedMemory->RequestedVirtualAddress,
											  0);
			}
			else
				break;

			if (status == NO_ERROR)
			{
#if ENABLE_ADDRESS_LIST_ARRAY
				H3RefCountFree(HwDeviceExtension,
							   RequestPacket,
							   (ULONG)mappedMemory->RequestedVirtualAddress);
#endif

				mappedMemory++;

				status = VideoPortUnmapMemory(hwDeviceExtension,
											  mappedMemory->RequestedVirtualAddress,
											  0);
#if ENABLE_ADDRESS_LIST_ARRAY
				if (status == NO_ERROR)
					H3RefCountFree(HwDeviceExtension,
								   RequestPacket,
								   (ULONG)mappedMemory->RequestedVirtualAddress);
#endif

			}

			if (status == NO_ERROR)
			{
				mappedMemory++;

				status = VideoPortUnmapMemory(hwDeviceExtension,
											  mappedMemory->RequestedVirtualAddress,
											  0);

#if ENABLE_ADDRESS_LIST_ARRAY
				if (status == NO_ERROR)
					H3RefCountFree(HwDeviceExtension,
								   RequestPacket,
								   (ULONG)mappedMemory->RequestedVirtualAddress);
#endif
			}
#endif
		}

		break;


	case IOCTL_VIDEO_QUERY_AVAIL_MODES:

		VideoDebugPrint((1, "H3QueryAvailableModes\n"));

		status = H3QueryAvailableModes(HwDeviceExtension, RequestPacket);

		break;

	case IOCTL_VIDEO_QUERY_CURRENT_MODE:

		VideoDebugPrint((1, "H3QueryCurrentMode\n"));

#if DBG
    DumpMTRRs(HwDeviceExtension, 2);
    DumpH3Regs(HwDeviceExtension, 3);
#endif

		status = H3QueryCurrentMode(HwDeviceExtension, RequestPacket);

		break;

	case IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES:

		VideoDebugPrint((1, "H3QueryNumAvailableModes\n"));

		status = H3QueryNumAvailableModes(HwDeviceExtension, RequestPacket);

		break;

	case IOCTL_VIDEO_SET_CURRENT_MODE:

		VideoDebugPrint((1, "H3SetCurrentMode\n"));

#ifdef SLI_AA
    // make sure sli/aa are disabled when we exit glide
    if (IS_NAPALM)
    {
      CHIPINFO  ChipInfo;

      memset(&ChipInfo, 0, sizeof(ChipInfo));
      ChipInfo.dwChips = HwDeviceExtension->numUnits;
      H3_SETUP_SLI_AA(HwDeviceExtension, SLI_AA_DISABLE, &ChipInfo, NULL);
    }
#endif

		status = H3SetCurrentMode(HwDeviceExtension, RequestPacket);

		break;

	case IOCTL_VIDEO_SET_COLOR_REGISTERS:

		VideoDebugPrint((1, "H3SetColorLookup\n"));

		status = H3SetColorLookup(HwDeviceExtension, RequestPacket);

		break;

	case IOCTL_VIDEO_RESET_DEVICE:

		VideoDebugPrint((1, "ResetDevice\n"));

		//
		// Prep the card to return to a VGA mode
		//

		H3ResetHw(HwDeviceExtension, 0, 0);

		//
		// XXX this only works for card zero in a multi-monitor system
		//
		// If we're going to do this under multi-monitor, we need to do
		// it a different way
		//
		if (hwDeviceExtension->BiosPresent && !hwDeviceExtension->UseNonBIOSModeSet)
		{
			VIDEO_X86_BIOS_ARGUMENTS biosArguments;

			VideoDebugPrint((2, "H3 RESET_DEVICE - About to do int10\n"));

			//
			// Do an Int10 to mode 3 will put the board to a known state.
			//

			VideoPortZeroMemory(&biosArguments, sizeof(VIDEO_X86_BIOS_ARGUMENTS));

			biosArguments.Eax = 0x0003;

			VideoPortInt10(HwDeviceExtension, &biosArguments);

			VideoDebugPrint((2, "H3 RESET_DEVICE - Did int10\n"));
		}

		status = NO_ERROR;
		break;

	case IOCTL_VIDEO_SHARE_VIDEO_MEMORY:

		VideoDebugPrint((1, "ShareVideoMemory\n"));

		if ((RequestPacket->OutputBufferLength < sizeof(VIDEO_SHARE_MEMORY_INFORMATION))
		||  (RequestPacket->InputBufferLength < sizeof(VIDEO_MEMORY)))
		{
			VideoDebugPrint((0, "IOCTL_VIDEO_SHARE_VIDEO_MEMORY - ERROR_INSUFFICIENT_BUFFER\n"));
			status = ERROR_INSUFFICIENT_BUFFER;
			break;
		}

		pShareMemory = RequestPacket->InputBuffer;

#if (_WIN32_WINNT >= 0x0500)
    // to allow for tiled mode, let the ViewSize be larger than the AdapterMemorySize
    // but it still needs to be no larger than the mapped memory
    if ((pShareMemory->ViewOffset > hwDeviceExtension->MemBase1Length) ||
        ((pShareMemory->ViewOffset + pShareMemory->ViewSize) > hwDeviceExtension->MemBase1Length))
#else
		if ((pShareMemory->ViewOffset > hwDeviceExtension->AdapterMemorySize)
		||  ((pShareMemory->ViewOffset + pShareMemory->ViewSize) > hwDeviceExtension->AdapterMemorySize))
#endif
		{
			VideoDebugPrint((0, "IOCTL_VIDEO_SHARE_VIDEO_MEMORY - ERROR_INVALID_PARAMETER\n"));
			status = ERROR_INVALID_PARAMETER;
			break;
		}

		RequestPacket->StatusBlock->Information =
									sizeof(VIDEO_SHARE_MEMORY_INFORMATION);

		//
		// Beware: the input buffer and the output buffer are the same
		// buffer, and therefore data should not be copied from one to the
		// other
		//

		virtualAddress = pShareMemory->ProcessHandle;
		sharedViewSize = pShareMemory->ViewSize;

		inIoSpace = hwDeviceExtension->MemBase1InIOSpace;

		//
		// NOTE: we are ignoring ViewOffset
		//

		shareAddress.QuadPart = hwDeviceExtension->PhysicalMemBaseAddr1.QuadPart;
#if DEBUG_SHARING
		VideoDebugPrint((2,"SharedViewOffset = 0x%08lx\n", pShareMemory->ViewOffset));
		VideoDebugPrint((2,"VirtualAddress   = 0x%08lx\n", virtualAddress));
		VideoDebugPrint((2,"SharedViewSize   = 0x%08lx\n", sharedViewSize));
#endif
		//
		// The frame buffer is always mapped linearly
		// There will never be a banked mapping!!!
		//
		//
		// Performance:
		//
		// Enable USWC on the P6 processor.
		// We only do it for the frame buffer - memory mapped registers can
		// not be mapped USWC because write combining the registers would
		// cause very bad things to happen !
		//

#if ENABLE_UNATTENDED_INSTALL_CHECK
		if (!HwDeviceExtension->UnattendedInstall)
#endif
			inIoSpace |= VIDEO_MEMORY_SPACE_P6CACHE;

		//
		// Unlike the MAP_MEMORY IOCTL, in this case we can not map extra
		// address space since the application could actually use the
		// pointer we return to it to touch locations in the address space
		// that do not have actual video memory in them.
		//
		// An app doing this would cause the machine to crash.
		//
		// However, because the caching policy for USWC in the P6 is on
		// *physical* addresses, this memory mapping will "piggy back" on
		// the normal frame buffer mapping, and therefore also benefit
		// from USWC ! Cool side-effect !!!
		//

		status = VideoPortMapMemory(hwDeviceExtension,
									shareAddress,
									&sharedViewSize,
									&inIoSpace,
									&virtualAddress);

		pShareMemoryInformation = RequestPacket->OutputBuffer;

		pShareMemoryInformation->SharedViewOffset = pShareMemory->ViewOffset;
		pShareMemoryInformation->VirtualAddress   = virtualAddress;
		pShareMemoryInformation->SharedViewSize   = sharedViewSize;
#if DEBUG_SHARING
		VideoDebugPrint((2,"After VideoPortMapMemory\n"));
		VideoDebugPrint((2,"SharedViewOffset = 0x%08lx\n", pShareMemory->ViewOffset));
		VideoDebugPrint((2,"VirtualAddress   = 0x%08lx\n", virtualAddress));
		VideoDebugPrint((2,"SharedViewSize   = 0x%08lx\n", sharedViewSize));
#endif
#if ENABLE_ADDRESS_LIST_ARRAY
		if (status == NO_ERROR)
			H3RefCountAlloc(HwDeviceExtension, (ULONG)virtualAddress);
#endif

		break;


	case IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY:

		VideoDebugPrint((1, "UnshareVideoMemory\n"));

		if (RequestPacket->InputBufferLength < sizeof(VIDEO_SHARE_MEMORY))
		{
			status = ERROR_INSUFFICIENT_BUFFER;
			break;
		}

		pShareMemory = RequestPacket->InputBuffer;
#if DEBUG_SHARING
		VideoDebugPrint((2,"ProcessHandle           = 0x%08lx\n", pShareMemory->ProcessHandle));
		VideoDebugPrint((2,"ViewOffset              = 0x%08lx\n", pShareMemory->ViewOffset));
		VideoDebugPrint((2,"ViewSize                = 0x%08lx\n", pShareMemory->ViewSize));
		VideoDebugPrint((2,"RequestedVirtualAddress = 0x%08lx\n", pShareMemory->RequestedVirtualAddress));
#endif

		status = VideoPortUnmapMemory(hwDeviceExtension,
									  pShareMemory->RequestedVirtualAddress,
									  pShareMemory->ProcessHandle);
#if ENABLE_ADDRESS_LIST_ARRAY
		if (status == NO_ERROR)
			H3RefCountFree(HwDeviceExtension, RequestPacket, (ULONG)pShareMemory->RequestedVirtualAddress);
#endif

		break;

    case IOCTL_VIDEO_QUERY_GLIDE_ACCESS_RANGES:
			
      VideoDebugPrint((1, "QueryGlideAccessRanges\n"));

      {
        PVIDEO_PUBLIC_ACCESS_RANGES portAccess;
        ULONG physicalPortLength;
        HANDLE processHandle = *(HANDLE *)RequestPacket->InputBuffer;
			
        if ( RequestPacket->InputBufferLength < sizeof(HANDLE) )
        {
          VideoDebugPrint((0, "IOCTL_VIDEO_QUERY_GLIDE_ACCESS_RANGES - ERROR_INSUFFICIENT_BUFFER\n"));
          status = ERROR_INSUFFICIENT_BUFFER;
          break;
        }

        RequestPacket->StatusBlock->Information = 3 * sizeof(VIDEO_PUBLIC_ACCESS_RANGES);
#ifdef SLI_AA
        RequestPacket->StatusBlock->Information += (HWINFO_SST_MAX_NUM_CHIPS * HWINFO_SST_MAX_CHIP_INDEX) * sizeof(VIDEO_PUBLIC_ACCESS_RANGES);
#endif
        if (RequestPacket->OutputBufferLength < RequestPacket->StatusBlock->Information)
        {
          VideoDebugPrint((1, "invalid buffer size (%d), expected %d\n",
                           RequestPacket->OutputBufferLength, RequestPacket->StatusBlock->Information));
          status = ERROR_INSUFFICIENT_BUFFER;
          break;
        }

        portAccess = RequestPacket->OutputBuffer;

        // map membase0 (the memory mapped regs) to user mode
        portAccess[0].VirtualAddress = (PVOID)processHandle;  // Process handle/Requested VA
        portAccess[0].InIoSpace = hwDeviceExtension->MemBase0InIOSpace;
        portAccess[0].MappedInIoSpace = portAccess[0].InIoSpace;

        physicalPortLength = hwDeviceExtension->MemBase0Length;

#if DBG
        VideoDebugPrint((2, "portAccess[0].VirtualAddress  = %08lx\n", portAccess[0].VirtualAddress));
        VideoDebugPrint((2, "portAccess[0].InIoSpace       = %08lx\n", portAccess[0].InIoSpace));
        VideoDebugPrint((2, "portAccess[0].MappedInIoSpace = %08lx\n", portAccess[0].MappedInIoSpace));
#endif
        status = VideoPortMapMemory(hwDeviceExtension,
                                    hwDeviceExtension->PhysicalMemBaseAddr0,
                                    &physicalPortLength,
                                    &(portAccess[0].MappedInIoSpace),
                                    &(portAccess[0].VirtualAddress));

        if (status != NO_ERROR)
        {
          VideoDebugPrint((0, "  glide membase0 mapping failed\n"));
          break;
        }

#if ENABLE_ADDRESS_LIST_ARRAY
        H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess[0].VirtualAddress);
#endif

        // map membase1 (the frame buffer) to user mode
        portAccess[1].VirtualAddress = (PVOID)processHandle;  // Glide process handle
        portAccess[1].InIoSpace = hwDeviceExtension->MemBase1InIOSpace;
        //
        // Since this mapping is the frame buffer, we must enable write
        // combining for best performance. Do it explicitly.
        //
#if ENABLE_UNATTENDED_INSTALL_CHECK
        if (HwDeviceExtension->UnattendedInstall)
          portAccess[1].MappedInIoSpace = portAccess[1].InIoSpace;
        else
#endif
          portAccess[1].MappedInIoSpace = portAccess[1].InIoSpace | VIDEO_MEMORY_SPACE_P6CACHE;

        physicalPortLength = hwDeviceExtension->MemBase1Length;

        status = VideoPortMapMemory(hwDeviceExtension,
                                    hwDeviceExtension->PhysicalMemBaseAddr1,
                                    &physicalPortLength,
                                    &(portAccess[1].MappedInIoSpace),
                                    &(portAccess[1].VirtualAddress));
        if (status != NO_ERROR)
        {
          VideoDebugPrint((0, "  glide membase1 mapping failed\n"));
          break;
        }

#if ENABLE_ADDRESS_LIST_ARRAY
        H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess[1].VirtualAddress);
#endif

        // map i/o regs to user mode
        portAccess[2].VirtualAddress = (PVOID)processHandle;  // Glide process handle
        portAccess[2].InIoSpace = hwDeviceExtension->IOBaseInIoSpace;
        portAccess[2].MappedInIoSpace = portAccess[2].InIoSpace;
  
        physicalPortLength = hwDeviceExtension->IOBaseLength;
  
        status = VideoPortMapMemory(hwDeviceExtension,
                                    hwDeviceExtension->PhysicalIOBaseAddr,
                                    &physicalPortLength,
                                    &(portAccess[2].MappedInIoSpace),
                                    &(portAccess[2].VirtualAddress));

        if (status != NO_ERROR)
        {
          VideoDebugPrint((0, "  glide i/o mapping failed\n"));
          break;
        }

#if ENABLE_ADDRESS_LIST_ARRAY
        H3RefCountAlloc(hwDeviceExtension, (ULONG)portAccess[2].VirtualAddress);
#endif

#ifdef SLI_AA
        status = GlideMapSlaveChips(HwDeviceExtension, processHandle, RequestPacket->OutputBuffer);
#endif
      }

      break;

    case IOCTL_VIDEO_FREE_GLIDE_ACCESS_RANGES:

      VideoDebugPrint((1, "FreeGlideAccessRanges\n"));

      {
        PVIDEO_MEMORY mappedMemory;
        GLIDE_BASE_INFO *pglideBaseInfo;

        if (RequestPacket->InputBufferLength < sizeof(GLIDE_BASE_INFO))
        {
          VideoDebugPrint((1, "invalid buffer size (%d), expected %d\n",
                           RequestPacket->InputBufferLength, sizeof(GLIDE_BASE_INFO)));

          status = ERROR_INSUFFICIENT_BUFFER;
          break;
        }

        status = NO_ERROR;

        pglideBaseInfo = RequestPacket->InputBuffer;

        // unmap membase0
        mappedMemory = &pglideBaseInfo->VideoMemory[0];
        status = VideoPortUnmapMemory(hwDeviceExtension,
                                      mappedMemory->RequestedVirtualAddress,
                                      pglideBaseInfo->hProcess);

        if (status != NO_ERROR)
        {
					VideoDebugPrint((0, "IOCTL_VIDEO_FREE_GLIDE_ACCESS_RANGES failed on first range, result=%x\n", status));
          break;
        }

#if ENABLE_ADDRESS_LIST_ARRAY
        H3RefCountFree(HwDeviceExtension,
                       RequestPacket,
                       (ULONG)mappedMemory->RequestedVirtualAddress);
#endif

        // unmap frame buffer
        mappedMemory = &pglideBaseInfo->VideoMemory[1];
        status = VideoPortUnmapMemory(hwDeviceExtension,
                                      mappedMemory->RequestedVirtualAddress,
                                      pglideBaseInfo->hProcess);
        if (status != NO_ERROR)
        {
					VideoDebugPrint((0, "IOCTL_VIDEO_FREE_GLIDE_ACCESS_RANGES failed on second range, result=%x\n", status));
          break;
        }

#if ENABLE_ADDRESS_LIST_ARRAY
        H3RefCountFree(HwDeviceExtension,
                       RequestPacket,
                       (ULONG)mappedMemory->RequestedVirtualAddress);
#endif

        // unmap i/o
        mappedMemory = &pglideBaseInfo->VideoMemory[2];
        status = VideoPortUnmapMemory(hwDeviceExtension,
                                      mappedMemory->RequestedVirtualAddress,
                                      pglideBaseInfo->hProcess);
        if (status != NO_ERROR)
        {
					VideoDebugPrint((0, "IOCTL_VIDEO_FREE_GLIDE_ACCESS_RANGES failed on third range, result=%x\n", status));
          break;
        }

#if ENABLE_ADDRESS_LIST_ARRAY
        H3RefCountFree(HwDeviceExtension,
                       RequestPacket,
                       (ULONG)mappedMemory->RequestedVirtualAddress);
#endif

#ifdef SLI_AA
        status = GlideUnmapSlaveChips(HwDeviceExtension, pglideBaseInfo);
#endif
      }

      break;

	case IOCTL_3DFX_QUERY_REGISTRY_VALUE:

		VideoDebugPrint((1, "H3QueryRegistryValue\n"));

		status = H3QueryRegistryValue(HwDeviceExtension,RequestPacket);

		break;

	case IOCTL_3DFX_SET_REGISTRY_VALUE:
		VideoDebugPrint((1, "H3SetRegistryValue\n"));
#if DBG && 0
		if (RequestPacket->InputBufferLength)
		{
			unsigned int i;
			TDFX_SET_VALUE_INFO *pInfo = (TDFX_SET_VALUE_INFO *)RequestPacket->InputBuffer;

			VideoDebugPrint((3, "ValueNameLength = %d\n",pInfo->ValueNameLength));
			VideoDebugPrint((3, "ValueName       = %s\n",pInfo->ValueName));
			VideoDebugPrint((3, "Type            = %d\n",pInfo->Type));
			VideoDebugPrint((3, "DataLength      = %d\n",pInfo->DataLength));
			VideoDebugPrint((3, "Data\n-------------------------------------------------------\n"));
			VideoDebugPrint((3, "In  buflength=%d\n",RequestPacket->InputBufferLength));
			for (i = 0; i < pInfo->DataLength; i++)
			{
				VideoDebugPrint((3, "%02x", 0xff & pInfo->Data[i]));
				if ((i % 32) == 31)
					VideoDebugPrint((3, "\n"));
			}
					VideoDebugPrint((3, "-------------------------------------------------------\n"));
		}
#endif
		status = H3SetRegistryValue(HwDeviceExtension,RequestPacket);

		break;

#if (_WIN32_WINNT >= 0x500)
	case IOCTL_VIDEO_SET_POWER_MANAGEMENT:

		VideoDebugPrint((1, "H3SetPowerManagement\n"));

		status = H3SetPowerManagement(HwDeviceExtension, RequestPacket);

		break;

	case IOCTL_VIDEO_GET_POWER_MANAGEMENT:

		VideoDebugPrint((1, "H3GetPowerManagement\n"));

		status = H3GetPowerManagement(HwDeviceExtension, RequestPacket);

		break;
#if 0 // Answering this IOCTL results in death under NT 5 -- be warned
	case IOCTL_ACPI_EVAL_METHOD:

		VideoDebugPrint((1, "IOCTL_ACPI_EVAL_METHOD\n"));

		VideoDebugPrint((3, "H3StartIO: Received IOCTL_ACPI_EVAL_METHOD request\n"));
		// Erick Smith of Microsoft says we should just ignore these ioctls
		// Unfortunately, if we report the status below, we get assertion failures in the
		// videoport driver, so for now we just play dumb and report the usual failure
		// for undefined ioctl calls.
    // fix for MS RAID bug 274784
    // CHK BUILDS: Banshee's HwStartIO fails and returns error code 78, which
    // cannot be recognized by videoport.
    status = ERROR_INVALID_FUNCTION;

		break;
#endif
#endif

#if ENABLE_LOG_FILE
	case IOCTL_3DFX_WRITE_LOG_FILE:

		VideoDebugPrint((1, "H3WriteLogFile\n"));

		status = H3WriteLogFile(RequestPacket->InputBuffer,
								RequestPacket->InputBufferLength);

		/* retro3dfx observability: persist a counter + the last write status to
		   the registry (IRQL-safe VideoPort API, unlike the file write) so we can
		   read from usermode whether the WRITE_LOG_FILE IOCTL is reaching the
		   miniport and whether ZwCreateFile/ZwWriteFile succeeded. */
		{
			static ULONG retroLogIoctlCount = 0;
			ULONG retroStatus = (ULONG)status;
			retroLogIoctlCount++;
			VideoPortSetRegistryParameters(HwDeviceExtension,
										   L"Retro3dfxLogIoctlCount",
										   &retroLogIoctlCount,
										   sizeof(retroLogIoctlCount));
			VideoPortSetRegistryParameters(HwDeviceExtension,
										   L"Retro3dfxLogLastStatus",
										   &retroStatus,
										   sizeof(retroStatus));
		}

		/* retro3dfx: satisfy videoprt's output-buffer contract so the IOCTL is
		   actually forwarded to this handler. */
		if (RequestPacket->OutputBufferLength >= sizeof(ULONG))
		{
			*(ULONG *)RequestPacket->OutputBuffer = (ULONG)status;
			RequestPacket->StatusBlock->Information = sizeof(ULONG);
		}
		break;
#endif

	case IOCTL_3DFX_IDENTITY_INFO:

		VideoDebugPrint((1, "3DfxIdentityInfo\n"));
		{
			PTDFX_IDENTITY_INFO pInfo;

			RequestPacket->StatusBlock->Information = sizeof(TDFX_IDENTITY_INFO);

			if (RequestPacket->OutputBufferLength < RequestPacket->StatusBlock->Information)
			{
				status = ERROR_INSUFFICIENT_BUFFER;
				break;
			}

			pInfo = RequestPacket->OutputBuffer;

			// Copy the relevant information
			pInfo->VendorID		= HwDeviceExtension->PCIVendorID;
			pInfo->DeviceID		= HwDeviceExtension->PCIDeviceID;
			pInfo->ChipRevision = HwDeviceExtension->ChipRevision;
			pInfo->CpuType      = isP6;
			pInfo->CustomerNumber = HwDeviceExtension->CustNum;
            pInfo->usSubSystemID  = HwDeviceExtension->PCISubSystemID;

			status = NO_ERROR;
		}

		break;

	case IOCTL_VIDEO_REG_SAVE_GAMMA_LUT:

		VideoDebugPrint((1, "H3SetGammaTable\n"));

		status = H3SetGammaTable(HwDeviceExtension, RequestPacket);

		break;

	case IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT:

		VideoDebugPrint((1, "H3GetGammaTable\n"));

		status = H3GetGammaTable(HwDeviceExtension, RequestPacket);

		break;

#if ENABLE_ADDRESS_LIST_ARRAY
	case IOCTL_3DFX_LINEARBASE_INFO:

		VideoDebugPrint((1, "3DfxLinearBase\n"));

		RequestPacket->StatusBlock->Information = sizeof(TDFX_LINEARADDR_INFO);

		if (RequestPacket->OutputBufferLength < RequestPacket->StatusBlock->Information)
		{
			status = ERROR_INSUFFICIENT_BUFFER;
			break;
		}
		else
		{
			PTDFX_LINEARADDR_INFO pInfo = RequestPacket->InputBuffer,
								  pOut  = RequestPacket->OutputBuffer;

			pOut->ulAddress    = (PULONG) pInfo->ulAddress;
			pOut->ulLinearBase = H3AllocFindAddress(HwDeviceExtension, pOut);

			if (pOut->ulLinearBase == NULL)
				status = ERROR_INVALID_PARAMETER;
			else
				status = NO_ERROR;
		}
		break;
#endif

	case IOCTL_3DFX_GET_BIOS_VERSION:

		VideoDebugPrint((1, "3DfxGetBIOSVersion\n"));

		RequestPacket->StatusBlock->Information = MAX_BIOS_VERSION_LENGTH;
		if (RequestPacket->OutputBufferLength < RequestPacket->StatusBlock->Information)
		{
			status = ERROR_INSUFFICIENT_BUFFER;
			break;
		}

		//
		// Copy the BIOS version string
		//
		VideoDebugPrint((1, "BIOS Version = \"%s\"\n", HwDeviceExtension->BiosVersion));
		memcpy(RequestPacket->OutputBuffer, HwDeviceExtension->BiosVersion, MAX_BIOS_VERSION_LENGTH);

		status = NO_ERROR;
		break;

#if (_WIN32_WINNT >= 0x0500) && defined (AGP_FIFO_CODE)
	case IOCTL_3DFX_GET_AGP_FIFO_INFO:

		VideoDebugPrint((1, "3DfxGetAGPFifoInfo\n"));

#if DBG
        if ( sizeof(H3_AGP_INFO) != RequestPacket->OutputBufferLength )
        {
    		VideoDebugPrint((0, "H3_AGP_INFO size mismatch -- miniport size != display size\n"));
    		DbgBreakPoint();
        }
#endif

		if ( RequestPacket->OutputBufferLength < sizeof(H3_AGP_INFO) )
		{
			status = ERROR_INSUFFICIENT_BUFFER;
			break;
		}

		RequestPacket->StatusBlock->Information = sizeof(H3_AGP_INFO);

		//
		// Copy the AGP fifo info structure
		//
		memcpy(RequestPacket->OutputBuffer, &HwDeviceExtension->agpinfo, sizeof(H3_AGP_INFO));

		status = NO_ERROR;
		break;
#endif

  case IOCTL_VIDEO_HANDLE_VIDEOPARAMETERS:
    VideoDebugPrint((1, "VIDEO_HANDLE_VIDEOPARAMETERS\n"));
    {
      status = BT868_ProcessVideoParameters(HwDeviceExtension,RequestPacket);

      break;
    }

#if (_WIN32_WINNT >= 0x0500)

#if 0  // implementation still in progress
       // ??? Ihaven't found the part of Win2K that uses these interfaces yet.  DanO 11/22/99
	case IOCTL_VIDEO_VALIDATE_CHILD_STATE_CONFIGURATION:
		VideoDebugPrint((1, "VIDEO_VALIDATE_CHILD_STATE_CONFIG\n"));
		{
        ULONG temp;

            _asm {int 3};
            temp = ((PVIDEO_CHILD_STATE_CONFIGURATION)RequestPacket->InputBuffer)->Count;
            temp = ((PVIDEO_CHILD_STATE_CONFIGURATION)RequestPacket->InputBuffer)->ChildStateArray[0].Id;
            temp = ((PVIDEO_CHILD_STATE_CONFIGURATION)RequestPacket->InputBuffer)->ChildStateArray[0].State;
            temp = ((PVIDEO_CHILD_STATE_CONFIGURATION)RequestPacket->InputBuffer)->ChildStateArray[1].Id;
            temp = ((PVIDEO_CHILD_STATE_CONFIGURATION)RequestPacket->InputBuffer)->ChildStateArray[1].State;

            if ( RequestPacket->OutputBufferLength < sizeof(ULONG) )
            {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }
            RequestPacket->StatusBlock->Information = sizeof(ULONG);
            *(ULONG *)RequestPacket->OutputBuffer = 1;  //Don't proceed with state switch.
            status = NO_ERROR;
            break;
		}

	case IOCTL_VIDEO_SET_CHILD_STATE_CONFIGURATION:
		VideoDebugPrint((1, "VIDEO_SET_CHILD_STATE_CONFIG\n"));
		{
        ULONG temp;

            _asm {int 3};
            temp = ((PVIDEO_CHILD_STATE_CONFIGURATION)RequestPacket->InputBuffer)->Count;
            temp = ((PVIDEO_CHILD_STATE_CONFIGURATION)RequestPacket->InputBuffer)->ChildStateArray[0].Id;
            temp = ((PVIDEO_CHILD_STATE_CONFIGURATION)RequestPacket->InputBuffer)->ChildStateArray[0].State;
            temp = ((PVIDEO_CHILD_STATE_CONFIGURATION)RequestPacket->InputBuffer)->ChildStateArray[1].Id;
            temp = ((PVIDEO_CHILD_STATE_CONFIGURATION)RequestPacket->InputBuffer)->ChildStateArray[1].State;

            status = NO_ERROR;
			break;
		}
#endif   // implementation still in progress

    case IOCTL_VIDEO_GET_CHILD_STATE:
      VideoDebugPrint((1, "VIDEO_GET_CHILD_STATE\n"));
        {
            // get state (active/inactive) of child device specified in input buffer
            ULONG ulChildId;

            ulChildId = *(DWORD *)RequestPacket->InputBuffer;

            if ( RequestPacket->OutputBufferLength < sizeof(ULONG) )
            {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }
            RequestPacket->StatusBlock->Information = sizeof(ULONG);
            // check for analog monitor child device
            if ((ulChildId == QUERY_MONITOR_ID) || (ulChildId == QUERY_NONDDC_MONITOR_ID))
            {
                *(ULONG *)RequestPacket->OutputBuffer = HwDeviceExtension->monitorActive;
                status = NO_ERROR;
            }
            // check for digital flat panel child device
            else if (ulChildId == QUERY_DFP_ID)
            {
                *(ULONG *)RequestPacket->OutputBuffer = isPanelActive(HwDeviceExtension);
                status = NO_ERROR;
            }
            // check for TvOut child device
            else if (ulChildId == QUERY_TVOUT_ID)
            {
                *(ULONG *)RequestPacket->OutputBuffer = HwDeviceExtension->tvOutActive;
                status = NO_ERROR;
            }
			else if (ulChildId == QUERY_V3TV_ID)
			{
				*(ULONG *)RequestPacket->OutputBuffer = 1; //v3tv add more info
				status = NO_ERROR;
			}
		    else
		        status = ERROR_INVALID_FUNCTION;
            break;
        }

    case IOCTL_VMI_FUNCTION:
      VideoDebugPrint((1, "VMI_FUNCTION\n"));
      {
        status = VDDVMIFunctions(HwDeviceExtension, RequestPacket);
        break;
      }
#endif

#if KMVT || ENABLE_IRQ
    case IOCTL_ALLOC_KMVT_MEMORY:
      VideoDebugPrint((1, "ALL_KMVT_MEMORY\n"));
      {
        status = H3AllocatKMBuff(HwDeviceExtension,RequestPacket);
        break;
      }
	case IOCTL_V3TV_SCALE_FUNCTION:
		VideoDebugPrint((1, "V3TVScaleFunction\n"));
		{
			status = WDMV3TVScale(HwDeviceExtension, RequestPacket);
			break;	
		}
#endif

#ifdef TVOUT_SUPPORTED
    case IOCTL_QUERY_MONITOR:
        VideoDebugPrint((1, "QUERY_MONITOR\n"));
        {
        TVGETSTANDARD localPacket;
        TDFX_QUERY_MONITOR_INFO * monitorResults;

        RequestPacket->StatusBlock->Information = sizeof(TDFX_QUERY_MONITOR_INFO);
        if (RequestPacket->OutputBufferLength < RequestPacket->StatusBlock->Information)
        {
            status = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        requestType = ((ULONG *)RequestPacket->InputBuffer)[0];

        monitorResults = RequestPacket->OutputBuffer;

        ulTemp = VideoPortReadPortUlong((PULONG)(pIO + DACMODE));
        if (!(ulTemp & SST_DACMODE_DPMS_SUSPEND))
            monitorResults->monitorStatus = MONITOR_IS_ENABLED; //monitor is enabled
        else
            monitorResults->monitorStatus = 0; // monitor is disabled

        // decide if the driver should enforce the fact that simultanious PAL TvOut and monitor are not allowed
        if (HwDeviceExtension->tvOutActive && !(HwDeviceExtension->bAllowPALCRT))
        {
            BT868_GetStandard (HwDeviceExtension, &localPacket);
            if (localPacket.dwStandard  != VP_TV_STANDARD_NTSC_M)
            {
                monitorResults->monitorStatus = 0;                 //say the monitor is off
                monitorResults->monitorControl = DISABLE_MONITOR;  //set the monitor off checkbox
                requestType = 2;									 //turn the monitor off
            }
        }

        if (requestType > 0)
        {
            ulTemp = 0;
            if (requestType == 1)  // enable monitor
            {
                ulTemp &= ~SST_DACMODE_DPMS_SUSPEND;
                HwDeviceExtension->monitorActive = TRUE;
            }
            else if (requestType == 2)  // disable monitor
            {
                ulTemp |= SST_DACMODE_DPMS_SUSPEND;
                HwDeviceExtension->monitorActive = FALSE;
            }
        VideoPortWritePortUlong((PULONG)(pIO + DACMODE), ulTemp);
        }

        status = NO_ERROR;
        break;
        }


	//TV-Out support function
    case IOCTL_TV_TRANSACTION:
      VideoDebugPrint((1, "TV_TRANSACTION\n"));
      if (!HwDeviceExtension->tvOutCapable)
      {
		status = ERROR_INVALID_FUNCTION;
        break;
      }

      // process the request
      status = BT868_ProcessRequest(HwDeviceExtension,RequestPacket);

      break;

    //Digital Flat Panel support function
    case IOCTL_DFP_TRANSACTION:
      VideoDebugPrint((1, "DFP_TRANSACTION\n"));
#if 0
      if (!HwDeviceExtension->bDfpPresent)
      {
		status = ERROR_INVALID_FUNCTION;
        break;
      }
#endif

      // process the request
      status = DFP_ProcessRequest(HwDeviceExtension,RequestPacket);

      break;

    //  Handle request which selects which set of alternate displays is turned on or off.
    case IOCTL_SET_ALTERNATE_DISPLAYS:
        VideoDebugPrint((1, "SET_ALTERNATE_DISPLAYS\n"));
        {
            ULONG result = 0;
            BOOLEAN errorOccurred = FALSE;

            if (RequestPacket->InputBufferLength < sizeof(ULONG))
            {
                VideoDebugPrint((1, "  invalid buffer size (%d), expected %d\n",
                        RequestPacket->InputBufferLength, sizeof(ULONG)));

                status = ERROR_INSUFFICIENT_BUFFER;
            }
            else
            {
                RequestPacket->StatusBlock->Information =
                        sizeof(result);
                if (RequestPacket->OutputBufferLength <
                        RequestPacket->StatusBlock->Information)
                {
                    status = ERROR_INSUFFICIENT_BUFFER;
                }
                else
                {
                    // set TvOut display as requested.
                    result |= BT868_SetActiveState(HwDeviceExtension,((ULONG *) RequestPacket->InputBuffer)[0]);

                    // set DFP display as requested.
                    result |= DFP_SetActiveState(HwDeviceExtension,((ULONG *) RequestPacket->InputBuffer)[0]);

                    // set VGA display as requested.
                    result |= CRT_SetActiveState(HwDeviceExtension,((ULONG *) RequestPacket->InputBuffer)[0]);
                    ((ULONG *) RequestPacket->OutputBuffer)[0] = result;
		            status = NO_ERROR;
                }
            }
            break;
        }
#endif //def TVOUT_SUPPORTED

    case IOCTL_GET_MISC_VALUES:

        VideoDebugPrint((1, "GET_MISC_VALUES\n"));
        {
            PTDFX_MISC_INFO pInfo;

            RequestPacket->StatusBlock->Information = sizeof(TDFX_MISC_INFO);

            if (RequestPacket->OutputBufferLength < RequestPacket->StatusBlock->Information)
            {
                status = ERROR_INSUFFICIENT_BUFFER;
                break;
            }

            pInfo = RequestPacket->OutputBuffer;

            if (IS_VOODOO3)
            {
                pInfo->dwDefaultClock = H4_DEFAULT_CLOCK;  //??? not correct
                pInfo->dwMinClock = H4_CLOCK_MINIMUM;
                pInfo->dwMaxClock = H4_CLOCK_MAXIMUM;
            }
            else
            {
                pInfo->dwDefaultClock = HwDeviceExtension->pllCtrl1 / 10000;
                pInfo->dwMinClock = H5_CLOCK_MINIMUM;
                pInfo->dwMaxClock = H5_CLOCK_MAXIMUM;
            }
            status = NO_ERROR;
        }
        break;

#ifdef SLI_AA
    case IOCTL_3DFX_SLI_AA_INFO:

      VideoDebugPrint((1, "3dfxSLIAAInfo\n"));
      {
        PTDFX_SLI_AA_INFO pInfo;

        RequestPacket->StatusBlock->Information = sizeof(TDFX_SLI_AA_INFO);

        if (RequestPacket->OutputBufferLength < RequestPacket->StatusBlock->Information)
        {
          status = ERROR_INSUFFICIENT_BUFFER;
          break;
        }

        pInfo = RequestPacket->OutputBuffer;

        memset(pInfo, 0, RequestPacket->OutputBufferLength);

        pInfo->numUnits = HwDeviceExtension->numUnits;
        memcpy(&pInfo->sliMappedAddress[0][0],
               &HwDeviceExtension->sliMappedAddress[0][0],
               HwDeviceExtension->numUnits * HWINFO_SST_MAX_CHIP_INDEX * sizeof(HwDeviceExtension->sliMappedAddress[0][0]));

        status = NO_ERROR;
      }

      break;

    case IOCTL_3DFX_SLI_AA_ENABLE:
      VideoDebugPrint((1, "3dfx SLIAA Enable\n"));
      {
        SLI_AA_REQUEST  *pRequest;

        if (RequestPacket->InputBufferLength < sizeof(SLI_AA_REQUEST))
        {
          VideoDebugPrint((1, "  invalid buffer size (%d), expected %d\n",
                           RequestPacket->InputBufferLength, sizeof(SLI_AA_REQUEST)));

          status = ERROR_INSUFFICIENT_BUFFER;
        }
        else if (IS_NAPALM)
        {
          pRequest = RequestPacket->InputBuffer;
          EnableSLIAA(HwDeviceExtension, pRequest);

          status = NO_ERROR;
        }
        else
          status = ERROR_INVALID_FUNCTION;
      }
      break;

    case IOCTL_3DFX_SLI_AA_DISABLE:
      VideoDebugPrint((1, "3dfx SLIAA Disable\n"));
      {
        SLI_AA_REQUEST  *pRequest;

        if (RequestPacket->InputBufferLength < sizeof(SLI_AA_REQUEST))
        {
          VideoDebugPrint((1, "  invalid buffer size (%d), expected %d\n",
                           RequestPacket->InputBufferLength, sizeof(SLI_AA_REQUEST)));

          status = ERROR_INSUFFICIENT_BUFFER;
        }
        else if (IS_NAPALM)
        {
          pRequest = RequestPacket->InputBuffer;
          DisableSLIAA(HwDeviceExtension, pRequest);

          status = NO_ERROR;
        }
        else
          status = ERROR_INVALID_FUNCTION;
      }
      break;

    case IOCTL_3DFX_PCI_OP:
      VideoDebugPrint((1, "3dfx PCI OP\n"));
      {
        HWPCIOP *pHwPCIOp;

        if (RequestPacket->InputBufferLength < sizeof(HWPCIOP))
        {
          VideoDebugPrint((1, "  invalid buffer size (%d), expected %d\n",
                           RequestPacket->InputBufferLength, sizeof(HWPCIOP)));

          status = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        {
          pHwPCIOp = RequestPacket->InputBuffer;

          if (H3G_PCI_READ == pHwPCIOp->dwOp)
          {
            RequestPacket->StatusBlock->Information = sizeof(HWPCIOP);

            if (RequestPacket->OutputBufferLength < sizeof(HWPCIOP))
            {
              status = ERROR_INSUFFICIENT_BUFFER;
            }
            else
            {
              // this only works because the input & output buffers in the RequestPacket
              // are the same
              pHwPCIOp->dwValue = PCI_CFG_RD(pHwPCIOp->dwOffset, pHwPCIOp->dwFunc);
              status = NO_ERROR;
            }
          }
          else if (H3G_PCI_WRITE == pHwPCIOp->dwOp)
          {
            PCI_CFG_WR(pHwPCIOp->dwOffset, pHwPCIOp->dwValue, pHwPCIOp->dwFunc);
            status = NO_ERROR;
          }
          else
            status = ERROR_INVALID_FUNCTION;
        }
      }
      break;
#ifdef RD_ABORT_ERROR
    case IOCTL_3DFX_ENABLE_SLI_READ:
      VideoDebugPrint((1, "Enable SLI Read\n"));
      SLI_Read_Enable(HwDeviceExtension);
      RequestPacket->StatusBlock->Information = 0;
      status = NO_ERROR;
      break;

    case IOCTL_3DFX_DISABLE_SLI_READ:
      VideoDebugPrint((1, "Disable SLI Read\n"));
      SLI_Read_Disable(HwDeviceExtension);
      RequestPacket->StatusBlock->Information = 0;
      status = NO_ERROR;
      break;
#endif
#endif

    case IOCTL_3DFX_GET_CURRENT_PROCESS_ID:
      VideoDebugPrint((1, "GetCurrentProcessId\n"));
      {
        HANDLE *pCurrentProcessId;

        extern HANDLE PsGetCurrentProcessId(void);


        RequestPacket->StatusBlock->Information = sizeof(ULONG);

        if (RequestPacket->OutputBufferLength < RequestPacket->StatusBlock->Information)
        {
          status = ERROR_INSUFFICIENT_BUFFER;
          break;
        }

        pCurrentProcessId = RequestPacket->OutputBuffer;

        *pCurrentProcessId = PsGetCurrentProcessId();

        status = NO_ERROR;
      }
      break;

	//
	// if we get here, an invalid IoControlCode was specified.
	//

	default:
#if DBG
		VideoDebugPrint((0, "H3StartIO: Fell through routine - invalid command (%x)\n", RequestPacket->IoControlCode));
		//DbgBreakPoint();
#endif
		status = ERROR_INVALID_FUNCTION;

		break;

	}

//#if DBG
  // The checked build of w2k shows the following assert when a status it doesn't expect is returned
  // from H3StartIO:
  //
  //
  // Invalid return value from HwStartIo!
  //
  // *** Assertion failed: FALSE
  // ***   Source File: D:\nt\private\ntos\video\port\videoprt.c, line 5143
  //
  // > 
  // Break, Ignore, Terminate Process or Terminate Thread (bipt)? i
  //
  //
  // We need to return an error code from dderror.h otherwise the videoport pukes.
  // For no particular reason, I'll choose ERROR_DEV_NOT_EXIST as a default
  //
  // ioctls that return invalid status codes should be fixed!
  //
  // suspicious ioctls include:
  // IOCTL_VIDEO_HANDLE_VIDEOPARAMETERS:
  //    BT868_ProcessVideoParameters returns -1
  // IOCTL_TV_TRANSACTION:
  //    status = 0xDEADBEEF and
  //    BT868_ProcessRequest returns -1
  // IOCTL_DFP_TRANSACTION:
  //    status = 0xDEADBEEF and
  //    DFP_ProcessRequest returns -1
  //
  // The DEADBEEF returned by the TV_TRANSACTION ioctl causes the videoprt to assert on every mode
  // change on the w2k checked build!
  
  switch (status)
  {
    case NO_ERROR:
    case ERROR_INVALID_FUNCTION:
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_DEV_NOT_EXIST:
    case ERROR_INVALID_PARAMETER:
    case ERROR_INSUFFICIENT_BUFFER:
    case ERROR_INVALID_NAME:
    case ERROR_MORE_DATA:
    case ERROR_IO_PENDING:
    case ERROR_DEVICE_REINITIALIZATION_NEEDED:
    case ERROR_CONTINUE:
    case ERROR_NO_MORE_DEVICES:
      // status is valid, it's defined in dderror.h
      break;
    default:
      VideoDebugPrint((0, "\n\n\nH5: WARNING:\n\n"));
      VideoDebugPrint((0, "H5:   Invalid status being returned from H3StartIO, ioctl=%08lXh, status=%08lXh\n",
                       RequestPacket->IoControlCode, status));

      // in fxioctl.h, we define the function numbers used in the CTL_CODE macro
      // extracting the Function from the CTL_CODE is similar to the DEVICE_TYPE_FROM_CTL_CODE
      // macro in devioctl.h
#define FUNCTION_FROM_CTL_CODE(ctrlCode)  (((ULONG)(ctrlCode & 0x00003FFC)) >> 2)

      VideoDebugPrint((0, "H5:     Offending ioctl function = 0x%X, please fix the status codes used here!\n",
                       FUNCTION_FROM_CTL_CODE(RequestPacket->IoControlCode)));

      VideoDebugPrint((0, "H5:   Resetting status to ERROR_DEV_NOT_EXIST\n\n\n"));

      status = ERROR_DEV_NOT_EXIST;
      break;
  }
//#endif

	VideoDebugPrint((2, "\t\tLeaving H3StartIO routine, status = %x\n", status));

	RequestPacket->StatusBlock->Status = status;

	return TRUE;

} // end H3StartIO()


VP_STATUS
H3SetColorLookup(
	PHW_DEVICE_EXTENSION HwDeviceExtension,
	PVIDEO_REQUEST_PACKET RequestPacket
	)

/*++

Routine Description:

	This routine sets a specified portion of the color lookup table settings.

Arguments:

	HwDeviceExtension - Pointer to the miniport driver's device extension.

	RequestPacket - The block containing the data

Return Value:

	None.

--*/

{
	PH3_MEMBASE0 RegisterMap = (PH3_MEMBASE0) HwDeviceExtension->MappedAddress[SST_IO_REGS_INDEX];
	PUCHAR pIO = (PUCHAR) HwDeviceExtension->MappedAddress[SST_IO_INDEX];

	PVIDEO_CLUT pClutBuffer;
	ULONG ClutBufferSize, cRed, cGreen, cBlue, temp;

	int i;

	VideoDebugPrint((0, "H3SetColorLookup\n"));

	pClutBuffer = (PVIDEO_CLUT) RequestPacket->InputBuffer;
	ClutBufferSize = RequestPacket->InputBufferLength;

	//
	// Check if the size of the data in the input buffer is large enough.
	//

	if ((ClutBufferSize < sizeof(VIDEO_CLUT) - sizeof(ULONG))
	||	(ClutBufferSize < sizeof(VIDEO_CLUT) + (sizeof(ULONG) * (pClutBuffer->NumEntries - 1))))
		return ERROR_INSUFFICIENT_BUFFER;

	//
	// Check to see if the parameters are valid.
	//

	if ((pClutBuffer->NumEntries == 0)
	||	(pClutBuffer->FirstEntry > VIDEO_MAX_COLOR_REGISTER)
	||	(pClutBuffer->FirstEntry + pClutBuffer->NumEntries > VIDEO_MAX_COLOR_REGISTER + 1))
		return ERROR_INVALID_PARAMETER;

	//
	//	Set CLUT registers directly on the hardware
	//

	for (i = 0; i < pClutBuffer->NumEntries; i++)
	{
		cRed   = pClutBuffer->LookupTable[i].RgbArray.Red;
		cGreen = pClutBuffer->LookupTable[i].RgbArray.Green;
		cBlue  = pClutBuffer->LookupTable[i].RgbArray.Blue;

		VideoPortWritePortUlong((PULONG)(pIO + DACADDR), i);
		temp = VideoPortReadPortUlong((PULONG)(pIO + DACADDR));

		temp = ((cRed << 16) | (cGreen << 8) | cBlue);

		VideoPortWritePortUlong((PULONG)(pIO + DACDATA), temp);
		temp = VideoPortReadPortUlong((PULONG)(pIO + DACADDR));
	}

	H3UpdateGamma(HwDeviceExtension, RegisterMap, pIO, HwDeviceExtension->GammaTable);

	return NO_ERROR;

} // end H3SetColorLookup()

VOID
H3UpdateGamma(PHW_DEVICE_EXTENSION  HwDeviceExtension,
              PH3_MEMBASE0          RegisterMap,
              PUCHAR                pIO,
              PULONG                GammaTable)

/*++

Routine Description:

	This routine performs the update of the gamma table

Arguments:

	HwDeviceExtension - Pointer to the miniport driver's device extension.

Return Value:

	none.

--*/

{
	PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;

	ULONG temp;
	int i;

	VideoDebugPrint((0, "H3UpdateGamma\n"));

	//
	// Write the Gamma Table
	//
	for (i = 0; i < 0x100; i++)
	{
		VideoPortWritePortUlong((PULONG)(pIO + DACADDR), 0x100 + i);
		temp = VideoPortReadPortUlong((PULONG)(pIO + DACADDR));

		VideoPortWritePortUlong((PULONG)(pIO + DACDATA), GammaTable[i]);
		temp = VideoPortReadPortUlong((PULONG)(pIO + DACDATA));
	}
}


