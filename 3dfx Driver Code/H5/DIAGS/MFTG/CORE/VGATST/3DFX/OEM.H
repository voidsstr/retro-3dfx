//
//		OEM.H - OEM specific equates, typedefs, and structures for EDIAG.EXE
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				4/15/98
//		Last Modified:	5/13/98
//
//		Note:	3Dfx specific data
//
#define  BANSHEE						1
#define  VENDOR_ID	0x121A		// Default for Banshee
#define	 DEVICE_ID	0x03		// Default for Banshee
#define  V3_DEVICE_ID   0x05		// Default for VooDoo3
#define  NAPALM_DEVICE_ID   0x09		// Default for VooDoo4

// Function prototypes
WORD LookupModeNumber (WORD, WORD, WORD, WORD);
DWORD GetCRC (LPCARDINFO, WORD);

// Data definitions
#ifndef _DATAFILE_
extern	BYTE					byPCIBusID;
extern	BYTE					byPCIDevID;
extern	DWORD					physRegBase;
extern	WORD					wSelFlat;
extern	VBEINFOBLOCK		vbeInfoBanshee;
extern	VBEMODEINFOBLOCK	vbemiBanshee;
#endif

//
//		Copyright (c) 1998 Elpin Systems, Inc.
//		All rights reserved.
//
