//
//		INIT.CPP - OEM specific initialization routines for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				4/11/98
//		Last Modified:	5/13/98
//
//		Routines in this file:
//		OEMInit				Retrieve vital statistics about the adapter
//	   OEMSetNativeMode	Set a native mode with access to the linear space
//		OEMSetVGAMode		Turn on VGA mode and set text mode
//		OEMTerminate		Cleanup allocated variables and prepare for exit
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<i86.h>
#include "ediag.h"
#include "oem.h"

//
//		OEMInitCardData - Retrieve vital statistics about the adapter
//
//		Entry:	lpcard	Pointer to card structure to fill
//					wSel		Selector to a flat (linear) descriptor
//		Exit:		<BOOL>	Success flag (TRUE = Initialized, FALSE = Card doesn't exist)
//
BOOL OEMInit (CARDINFO *lpcard, WORD wSel)
{
	DWORD	dwRetVal;

	wSelFlat = wSel;

	// Memory information
	lpcard->bInVGAMode = TRUE;


	// Load default values
	lpcard->physAddr0 = 0;
	lpcard->nSizeAddr0 = 0;
	lpcard->physAddr1 = 0;
	lpcard->nSizeAddr1 = 0;
	lpcard->physAddr2 = 0;
	lpcard->nSizeAddr2 = 0;
	lpcard->physAddr3 = 0;
	lpcard->nSizeAddr3 = 0;
	lpcard->physAddr4 = 0;
	lpcard->nSizeAddr4 = 0;
	lpcard->physAddr5 = 0;
	lpcard->nSizeAddr5 = 0;
	lpcard->physAddr6 = 0;
	lpcard->nSizeAddr6 = 0;
	lpcard->physAddr7 = 0;
	lpcard->nSizeAddr7 = 0;
	lpcard->physROM = 0;
	lpcard->ioBase = 0;
	lpcard->lpModeList = NULL;

	// PCI information
	lpcard->pciVendorID = VENDOR_ID;
	lpcard->pciDeviceID = DEVICE_ID;

	// Attempt to locate the graphics adapter. If it doesn't exist,
	// then fail the function and return immediately. If it does
	// exist, then grab all the memory locations and store them in
	// the CARDINFO data structure.
	if (!FindPCIDev (lpcard->pciVendorID, lpcard->pciDeviceID, 
                         &byPCIBusID, &byPCIDevID))
      	{
             LogComment("No Banshee Found (VendorID %04Hh, DeviceID %04Xh).",
                      lpcard->pciVendorID, lpcard->pciDeviceID);

             lpcard->pciVendorID = VENDOR_ID;
             lpcard->pciDeviceID = V3_DEVICE_ID;
	     if (!FindPCIDev (lpcard->pciVendorID, lpcard->pciDeviceID, 
                              &byPCIBusID, &byPCIDevID))
            {
		// Display error message and quit
		LogComment ("\nERROR: No Voodoo3 Found VendorID %04Xh, DeviceID %04Xh.", 
                            lpcard->pciVendorID, lpcard->pciDeviceID);

               lpcard->pciVendorID = VENDOR_ID;
               lpcard->pciDeviceID = NAPALM_DEVICE_ID;
	        if (!FindPCIDev (lpcard->pciVendorID, lpcard->pciDeviceID, 
                              &byPCIBusID, &byPCIDevID))
               {
		  // Display error message and quit
		  LogComment ("\nERROR: No Napalm Device Found: VendorID %04Xh, DeviceID %04Xh.", 
                            lpcard->pciVendorID, lpcard->pciDeviceID);
		  return (FALSE);
               }

             }
	}

	// Get Memory Base Address 1 - Linear Frame Buffer
	if (ReadPCIDWord (byPCIBusID, byPCIDevID, 0x14, &dwRetVal))
		lpcard->physAddr0 = (dwRetVal & ~0xF);

	// Get Memory Base Address 0 - Regs, Texture buff,YUV spc,3d LFB
	if (ReadPCIDWord (byPCIBusID, byPCIDevID, 0x10, &dwRetVal))
		physRegBase = (dwRetVal & ~0xF);

	// Get Memory Base Address 2 - IO Base Address
	if (ReadPCIDWord (byPCIBusID, byPCIDevID, 0x18, &dwRetVal))
		lpcard->ioBase = (WORD) (dwRetVal & ~0x3);

	if (GetVBEInfo (&vbeInfoBanshee))
	{
		lpcard->lpModeList = vbeInfoBanshee.VideoModePtr;
		lpcard->nSizeAddr0 = vbeInfoBanshee.TotalMemory * 64 * 1024;
	}
	return (TRUE);
};

//
//	   OEMSetNativeMode - Set a native mode with access to the linear space
//
//		Entry:	lpcard	CARD info
//		Exit:	   <BOOL>	Success flag (TRUE = In native mode, FALSE = Not)
//
BOOL OEMSetNativeMode (LPCARDINFO lpcard)
{
	if (!SetMode (0x8101))
	{
		LogWriteString ("failed\n");
		OEMSetVGAMode (lpcard);
		return (FALSE);
	}
	lpcard->bInVGAMode = FALSE;
	return (TRUE);
}

//
//		OEMSetVGAMode - Turn on VGA mode and set text mode
//
//		Entry:	lpcard	Pointer to card info
//		Exit:		<BOOL>	Success flag (TRUE = In VGA mode, FALSE = Not)
//
BOOL OEMSetVGAMode (LPCARDINFO lpcard)
{
	lpcard->bInVGAMode = TRUE;
	SetMode (0x03);
	return (TRUE);
}

//
//		OEMTerminate - Cleanup allocated variables and prepare for exit
//
//		Entry:	lpci		Pointer to CARDINFO data structure
//		Exit:		<BOOL>	Success flag (TRUE = Successful, FALSE = Not)
//
//		Note:	This is the LAST call into this library.
//
BOOL OEMTerminate (LPCARDINFO lpci)
{
	lpci = lpci;					// Prevent compiler warnings
	return (TRUE);
}

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
