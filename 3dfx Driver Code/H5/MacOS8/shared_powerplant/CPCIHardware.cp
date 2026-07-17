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


#include "CPCIHardware.h"
#include "UTextLocalization.h"

#include <NameRegistry.h>
#include <DriverServices.h>
#include <PCI.h>
#include <stdio.h>
#include <string.h>
#include <QuickDraw.h>

#define kDriverRefNumPropertyStr "driver-ref"
#define kPCISlotNameProperty "AAPL,slot-name"
#define kPCIModelProperty "model"


#pragma mark === CLASS: CPCIHardware


#pragma mark -
#pragma mark === Initialization


/*______________________________________________________________________________

Constructor

*/

CPCIHardware::CPCIHardware(
	RegEntryID *		inRegEntryID )
{
	mRegEntryID = *inRegEntryID;
	
	Init();
}


/*______________________________________________________________________________

Destructor

*/

CPCIHardware::~CPCIHardware()
{
}



/*______________________________________________________________________________

Initialize

*/

void
CPCIHardware::Init()
{
	RegPropertyValueSize thePropSize;
	OSErr theSuccess = noErr;						
	GDHandle	theDevice  = ::GetDeviceList();

	/* Get slot name too. */
	thePropSize = sizeof( mSlotName );
	theSuccess = RegistryPropertyGet(&mRegEntryID, kPCISlotNameProperty, mSlotName, &thePropSize);
				
	/* Get model name too. */
	thePropSize = sizeof( mModelName );
	theSuccess = RegistryPropertyGet(&mRegEntryID, kPCIModelProperty, mModelName, &thePropSize);

	mRefNum = 0;
	thePropSize = sizeof( mRefNum );
	theSuccess = RegistryPropertyGet(&mRegEntryID, kDriverRefNumPropertyStr, &mRefNum, &thePropSize);

	if ( mRefNum )
	{
		while (theDevice != nil)
		{
	
			if ( (*theDevice)->gdRefNum == mRefNum )
			{
				mGDHandle = theDevice;
				break;
			}

	  		theDevice = ::GetNextDevice(theDevice);
	  	}
  			
	}
	
	if ( !strcmp( mSlotName, "SLOT-A" ) )
		strcpy( mBusTypeName, "AGP" );
	else
		strcpy( mBusTypeName, "PCI" );

	strcpy( mChipName, (char*) GetLocalText( kUnknownText ) );
	
	mNumberOfChips = 1;
	
	mMemorySize = 16;

}


/*______________________________________________________________________________

GetSlotName

*/

void
CPCIHardware::GetSlotName(
	Str255				inString)
{
	strcpy( (char*) &inString[1], mSlotName );
	inString[0] = strlen( mSlotName );
}


/*______________________________________________________________________________

GetModelName

*/

void
CPCIHardware::GetModelName(
	Str255				inString)
{
	strcpy( (char*) &inString[1], mModelName );
	inString[0] = strlen( mModelName );
}




/*______________________________________________________________________________

GetBusTypeName

*/

void
CPCIHardware::GetBusTypeName(
	Str255				inString)
{
	strcpy( (char*) &inString[1], mBusTypeName );
	inString[0] = strlen( mBusTypeName );
}


/*______________________________________________________________________________

GetChipName

*/

void
CPCIHardware::GetChipName(
	Str255				inString)
{
	strcpy( (char*) &inString[1], mChipName );
	inString[0] = strlen( mChipName );
}




/*______________________________________________________________________________

GetNumberOfChips

*/

long
CPCIHardware::GetNumberOfChips()
{
	return mNumberOfChips;
}




/*______________________________________________________________________________

GetMemorySize

*/

long
CPCIHardware::GetMemorySize()
{
	return mMemorySize;
}





/*______________________________________________________________________________

GetPlatform

*/

void
CPCIHardware::GetPlatform(
	Str255				inString)
{
	strcpy( (char*) inString, (char*) GetLocalText( kUnknownText ) );
}


/*______________________________________________________________________________

GetVersion

*/

void
CPCIHardware::GetVersion(
	Str255				inString)
{
	strcpy( (char*) inString, (char*) GetLocalText( kUnknownText ) );
}


/*______________________________________________________________________________

GetROMVersion

*/

void
CPCIHardware::GetROMVersion(
	UInt8 *				outMajor,
	UInt8 *				outMinor,
	UInt8 *				outStage,
	UInt8 *				outRev)
{
	
}

									

/*______________________________________________________________________________

GetRefNum

*/

short
CPCIHardware::GetRefNum()
{
	return mRefNum;
}




/*______________________________________________________________________________

IsActive

*/

bool
CPCIHardware::IsActive()
{
	return ( mRefNum < 0 );
}




/*______________________________________________________________________________

ReadPCIConfig

*/

short
CPCIHardware::ReadPCIConfig(
	unsigned long		inAddress,
	long				inStartBit,
	long				inEndBit,
	unsigned long *		outValue)
{
	OSErr				theSuccess;

	*outValue = 0;

	if ( inStartBit < 8 ) {
		unsigned char		theValue;
		theSuccess = ExpMgrConfigReadByte( &mRegEntryID, (void*) inAddress, &theValue );
		*outValue = theValue;
	} else if ( inStartBit < 16 ) {
		unsigned short		theValue;
		theSuccess = ExpMgrConfigReadWord( &mRegEntryID, (void*) inAddress, &theValue );
		*outValue = theValue;
	} else if ( inStartBit < 32 ) {
		unsigned long		theValue;
		theSuccess = ExpMgrConfigReadLong( &mRegEntryID, (void*) inAddress, &theValue );
		*outValue = theValue;
	} else {
		theSuccess = -1;
	} 

	*outValue >>= inEndBit;
	return theSuccess;
}


