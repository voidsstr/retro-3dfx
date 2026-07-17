/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
**
*/

#ifndef __CHardwareHost__
#define __CHardwareHost__

#include "CPCIHardware.h"

#define kHHMaxNumberOfTargets 8

typedef CPCIHardware *		(*TargetFuncPtr)(
									RegEntryID *		inEntry );


class	CHardwareHost {

	//_____ Initialization

public:
							CHardwareHost();
	virtual 				~CHardwareHost();
	
	virtual long			Scan();

	//_____ Accessors

	virtual long			GetNumberOfTargets() { return sNumberOfTargets; }
	
	virtual CPCIHardware *	GetTarget(
									long				inTargetID );
	
	//_____ Internals

private:

	virtual void			ScanAvailableHardware(
									long				inVendorID,
									long				inDeviceID,
									TargetFuncPtr		inTargetFuncPtr );
	
	virtual unsigned long	GetPropertyUInt32(
									RegEntryID *		inEntry,
									char *				inPropertyName );
	
	virtual void			AddTarget(
									RegEntryID *		inEntry,
									TargetFuncPtr		inTargetFuncPtr );
	
	static CPCIHardware *	NewBansheeTarget(
									RegEntryID *		inEntry );
	
	static CPCIHardware *	NewVoodoo3Target(
									RegEntryID *		inEntry );
	
	static CPCIHardware *	NewNapalmTarget(
									RegEntryID *		inEntry );
	
protected:
	static CPCIHardware *	sTarget[kHHMaxNumberOfTargets];
	static long				sNumberOfTargets;

};

#endif /* __CHardwareHost__ */