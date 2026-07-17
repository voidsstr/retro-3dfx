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

    localpci.h

Abstract:

    PCI Definitions for our device specific PCI data
	Rather than culling out the device specific information
	byte by byte, we can simply use this stucture and suck
	it all up in one fell swoop.

Author:

    Bob Lacey		17 October 1997

Revision History:

--*/
typedef struct _LOCAL_PCI_COMMON_CONFIG {
    USHORT	VendorID;           // (ro)
    USHORT	DeviceID;           // (ro)
    USHORT	Command;            // (rw) Device control
    USHORT	Status;				// (ro+)
    UCHAR	RevisionID;         // (ro)
    UCHAR	ClassCode[3];		// (ro)
    UCHAR	SubClass;           // (ro)
    UCHAR	BaseClass;          // (ro)
    UCHAR	CacheLineSize;      // (ro+)
    UCHAR	LatencyTimer;       // (ro+)
    UCHAR	HeaderType;         // (ro)
    UCHAR	BIST;               // (ro) Built in self test
	ULONG	MemBaseAddr0;		// (rw) write 0xffffffff to reset
	ULONG	MemBaseAddr1;		// (rw) write 0xffffffff to reset
	ULONG	IOBaseAddr;			// (rw) write 0xffffffff to reset
	ULONG	_reserved0[4];		// ++Reserved++
	USHORT	SubVendorID;		// board manufacturer ID
	USHORT	SubSystemID;		// board type (refault read by ROM)
	ULONG	ROMBaseAddr;		// (rw) write 0xfffffffe to reset
	ULONG	Capabilities_Ptr;	// (ro) start of capability list
	ULONG	_reserved1;			// ++Reserved++
    UCHAR	InterruptLine;		// (rw) 0xff means no connection
	UCHAR	InterruptPin;		// (ro)
    UCHAR	MinimumGrant;		// (ro)
    UCHAR	MaximumLatency;		// (ro)
	ULONG	fabID;				// (rw) (31:4 rw, 3:0 ro)
	ULONG	_reserved2;			// ++Reserved++
	ULONG	cfgStatus;			// (ro)
	ULONG	cfgScratch;			// (rw)
	ULONG	AGP_Cap_ID;			// (ro)
	ULONG	AGP_Status;			// (ro)
	ULONG	AGP_Cmd;			// (rw)
	ULONG	ACPI_Cap_ID;		// (ro)
	ULONG	ACPI_CSR;			// (rw)
	UCHAR	_reserved3[164];	// ++Reserved++

} LOCAL_PCI_COMMON_CONFIG, *PLOCAL_PCI_COMMON_CONFIG;
