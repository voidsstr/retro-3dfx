//
//		DATA.CPP - OEM specific data definitions for EDIAG.EXE
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
//		Written by:		Larry Coffey
//		Date:				4/15/98
//		Last Modified:	5/13/98
//
#include	<stdio.h>
#define	_DATAFILE_
#include "ediag.h"
#include "oem.h"

BYTE					byPCIBusID = 0;	// PCI Bus ID (returned by "FindPCIDev")
BYTE					byPCIDevID = 0;	// PCI Device ID (returned by "FindPCIDev")
DWORD					physRegBase = 0;	// PCI MemBase0
WORD					wSelFlat = 0;		// Flat selector (set in OEMInit)
VBEINFOBLOCK		vbeInfoBanshee;	// VBE Information about the Banshee
VBEMODEINFOBLOCK	vbemiBanshee;		// VBE Mode information

//
//		Copyright (c) 1994-1998 Elpin Systems, Inc.
//		All rights reserved.
//
