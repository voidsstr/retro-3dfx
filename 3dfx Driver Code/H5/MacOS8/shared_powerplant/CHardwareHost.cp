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

#include "CHardwareHost.h"
#include "CBansheeHardware.h"
#include "CVoodoo3Hardware.h"
#include "CNapalmHardware.h"
#include <NameRegistry.h>
#include <CodeFragments.h>
#include <string.h>

#define k3dfxVendorID		0x121A
#define kBansheeDeviceID	3
#define kVoodoo3DeviceID	5
#define kNapalmDeviceID		9

	
#define kVendorIDPropertyStr "vendor-id"
#define kDeviceIDPropertyStr "device-id"



long						CHardwareHost::sNumberOfTargets = 0;
CPCIHardware *				CHardwareHost::sTarget[kHHMaxNumberOfTargets];



/*______________________________________________________________________________

Constructor

*/

CHardwareHost::CHardwareHost()
{
	if ( sNumberOfTargets == 0 ) {
		long i = kHHMaxNumberOfTargets - 1;
		while ( i >= 0 )
			sTarget[ i-- ] = NULL;
	};
	
	Scan();
	
}



/*______________________________________________________________________________

Destructor

*/

CHardwareHost::~CHardwareHost()
{
}


/*______________________________________________________________________________

Scan

*/

long
CHardwareHost::Scan()
{
//	ScanAvailableHardware( k3dfxVendorID, kBansheeDeviceID, NewBansheeTarget );
	ScanAvailableHardware( k3dfxVendorID, kVoodoo3DeviceID, NewVoodoo3Target );
	ScanAvailableHardware( k3dfxVendorID, kNapalmDeviceID, NewNapalmTarget );
	
	return 0;
}



/*______________________________________________________________________________

GetTarget

*/

CPCIHardware *
CHardwareHost::GetTarget(
	long				inTargetID )
{
	if ( inTargetID < kHHMaxNumberOfTargets ) {
		return sTarget[ inTargetID ];
	} else {
		return NULL;
	}
}



/*______________________________________________________________________________

ScanAvailableHardware

*/

void
CHardwareHost::ScanAvailableHardware(
	long				inVendorID,
	long				inDeviceID,
	TargetFuncPtr		inTargetFuncPtr )
{
	RegEntryID 			theEntry; 
	RegEntryIter 		theEntryCookie; 
	RegEntryIterationOp theEntryIterOp; 
	Boolean 			theDone;
	OSStatus 			theErr = noErr;
	
	// initialize our RegEntry and the cookie (the cookie will point to the root
	// of the Name Registry.

	RegistryEntryIDInit( &theEntry );
	theErr = RegistryEntryIterateCreate( &theEntryCookie );
	if ( theErr == noErr) {

		// search the registry for a matching vendor-id property
		theEntryIterOp = kRegIterDescendants;

		do {
			// look for the vendor-id.
			theErr = RegistryEntrySearch( &theEntryCookie, theEntryIterOp, &theEntry,
											&theDone, kVendorIDPropertyStr,
											&inVendorID, 4);

			if (!theDone && (theErr == noErr)) 
			{ 
				// we found something, let's check the device ID
				if ( GetPropertyUInt32( &theEntry, kDeviceIDPropertyStr ) == inDeviceID ) {
					AddTarget( &theEntry, inTargetFuncPtr );
				} else {
					RegistryEntryIDDispose( &theEntry );
				}
			}

			theEntryIterOp = kRegIterContinue;
		}  while( !theDone && theErr == noErr);
		
		RegistryEntryIDDispose( &theEntry );
	}
}



/*______________________________________________________________________________

GetPropertyUInt32

*/

unsigned long
CHardwareHost::GetPropertyUInt32(
	RegEntryID *		inEntry,
	char *				inPropertyName )
{
	RegPropertyNameBuf	thePropertyName;
	RegPropertyIter 	thePropertyCookie; 
	Boolean 			theDone;
	unsigned long		theSize = 4;
	long				theResult = 0;
	OSStatus 			theErr = noErr;


	theErr = RegistryPropertyIterateCreate( inEntry, &thePropertyCookie);
	if ( theErr == noErr) {
			
		do {
			// look for the named property
			theErr = RegistryPropertyIterate( &thePropertyCookie, thePropertyName, &theDone);

			if ( !theDone && (theErr == noErr)) {
				if ( strcmp( thePropertyName, inPropertyName ) == 0) {


					theErr = RegistryPropertyGet( inEntry, inPropertyName,
													&theResult, &theSize);
					if ( theErr == noErr ) theDone = true;
				}
			}
		}  while( !theDone && theErr == noErr);
	}
	
	RegistryPropertyIterateDispose( &thePropertyCookie );

	return theResult;
}



/*______________________________________________________________________________

AddTarget

*/

void
CHardwareHost::AddTarget(
	RegEntryID *		inEntry,
	TargetFuncPtr		inTargetFuncPtr )
{
	if ( sNumberOfTargets < kHHMaxNumberOfTargets ) {
		sTarget[ sNumberOfTargets ] = (*inTargetFuncPtr)( inEntry );
		sNumberOfTargets++;
	}
}


/*______________________________________________________________________________

NewBansheeTarget

*/

CPCIHardware *
CHardwareHost::NewBansheeTarget(
	RegEntryID *		inEntry )
{
	return new CBansheeHardware( inEntry );
}


/*______________________________________________________________________________

NewVoodoo3Target

*/

CPCIHardware *
CHardwareHost::NewVoodoo3Target(
	RegEntryID *		inEntry )
{
	return new CVoodoo3Hardware( inEntry );
}


/*______________________________________________________________________________

NewNapalmTarget

*/

CPCIHardware *
CHardwareHost::NewNapalmTarget(
	RegEntryID *		inEntry )
{
	return new CNapalmHardware( inEntry );
}



