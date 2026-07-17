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


#include "CNapalmHardware.h"
#include "hrm_user_settings.h"
#include "GraphicsPrivHwc.h"
#include "UTextLocalization.h"

#if ROM_FLASHING_ENABLE_NAPALM
#  include "URomFlasher.h"
#endif

#include <NameRegistry.h>
#include <DriverServices.h>
#include <PCI.h>
#include <stdio.h>
#include <string.h>
  
  
/*______________________________________________________________________________

Constructor

*/

CNapalmHardware::CNapalmHardware(
	RegEntryID *		inRegEntryID )
		: CPCIHardware( inRegEntryID )
{

#if ROM_FLASHING_ENABLE_NAPALM
	mRomInfo = new TRomInfo;
	mRomInfo->mManufacturer = MAMID_UNKNOWN;
	mRomInfo->mDevice = DEVID_UNKNOWN;
	mRomInfo->mSectorSize = 1;
	mRomInfo->mPlatform = PLATID_UNKNOWN;
	mRomInfo->mRegInfo = new hwcRegInfo;
	mRomInfo->mRegEntryID = inRegEntryID;
	mRomInfo->mBuffer = new TRomBuffer;
	mRomInfo->mBuffer->mBufferSize = 65536;

	ReadVoodoo3Mapping( mRomInfo );

	/* enable hardware */
    EnableVoodoo3( mRomInfo );

	ReadRomInfo( mRomInfo );
#endif

	strcpy( mChipName, "VSA-100" );
	if ( mModelName[6] == '5' )
	{
		mNumberOfChips = 2;
		mMemorySize = 64;
	}
	else
	{
		mMemorySize = 32;
	}
}


/*______________________________________________________________________________

Destructor

*/

CNapalmHardware::~CNapalmHardware()
{
}



/*______________________________________________________________________________

SetROMData

*/

short
CNapalmHardware::SetROMData(
	char *				/*inData*/,
	long				/*inDataSize*/)
{
	return -1;
}



/*______________________________________________________________________________

GetROMData

*/

void
CNapalmHardware::GetROMData(
	char **				outData,
	long *				outDataSize)
{
	*outData = 0;
	*outDataSize = 0;
}



/*______________________________________________________________________________

GetPlatform

*/

void
CNapalmHardware::GetPlatform(
	Str255				outString)
{
	TextMessagesT		thePlatform;
	
	thePlatform = kUnknownText;
	
#if ROM_FLASHING_ENABLE_NAPALM
	switch( mRomInfo->mPlatform ) {
		case PLATID_INTEL:
			thePlatform = kWintelText;
			break;
		
		case PLATID_OPENFIRMWARE:
			thePlatform = kMacintoshText;
			break;
		
		default:
			thePlatform = kUnknownText;
			break;
	}
#endif
	
	strcpy( (char*) outString, (char*) GetLocalText( thePlatform ) );
}


/*______________________________________________________________________________

GetVersion

*/

void
CNapalmHardware::GetVersion(
	Str255				outString)
{
	if ( mRefNum )
	{
		ParamBlockRec  theParamBlock;
		hwcControl_t   theData;
		hwcRequest_t   req;
		hwcResponse_t  res;
		OSErr          theSuccess = -1;
  	 		
		theParamBlock.cntrlParam.ioCompletion = 0;
		theParamBlock.cntrlParam.ioVRefNum = 0;
		theParamBlock.cntrlParam.ioCRefNum = mRefNum;
		*((void **) &(theParamBlock.cntrlParam.csParam)) = &theData;
		theParamBlock.cntrlParam.csCode = k3DfxNewRequest;
		theData.which = k3DfxGetDriverInfo;
		theData.request = &req;
		theData.response = &res;
    
		theSuccess = (OSErr) PBControlSync(&theParamBlock);

		if ( theSuccess == 0 )
		{
			char stage;
        
			switch( res.optData.driverInfoRes.stage )
			{
				case 0x20:
						stage = 'd';
						break;
            
				case 0x40:
						stage = 'a';
						break;
            
				case 0x60:
						stage = 'b';
						break;
            
				case 0x80:
						stage = 'f';
						break;
          
				default:
						stage = '?';
						break;
			}
			
			if ( res.optData.driverInfoRes.stage == 0x80 )
				if (res.optData.driverInfoRes.rev == 0)
					sprintf( (char*) &outString[1], "%d.%d",
                     	  res.optData.driverInfoRes.major, res.optData.driverInfoRes.minor );
                else
                	sprintf( (char*) &outString[1], "%d.%d.%d",
                     	  res.optData.driverInfoRes.major, res.optData.driverInfoRes.minor,
                     	  	res.optData.driverInfoRes.rev );
			else
				sprintf( (char*) &outString[1], "%d.%d%c%d",
                       res.optData.driverInfoRes.major, res.optData.driverInfoRes.minor,
                          stage, res.optData.driverInfoRes.rev );
            outString[0] = strlen( (char*) &outString[1] );
		}
	}
	else
	{
		strcpy( (char*) outString, (char*) GetLocalText( kUnknownText ) );
	}

}



/*______________________________________________________________________________

GetROMVersion

*/

void
CNapalmHardware::GetROMVersion(
	UInt8 *				outMajor,
	UInt8 *				outMinor,
	UInt8 *				outStage,
	UInt8 *				outRev)
{
	if ( mRefNum )
	{
		ParamBlockRec  theParamBlock;
		hwcControl_t   theData;
		hwcRequest_t   req;
		hwcResponse_t  res;
		OSErr          theSuccess = -1;
  	 		
		theParamBlock.cntrlParam.ioCompletion = 0;
		theParamBlock.cntrlParam.ioVRefNum = 0;
		theParamBlock.cntrlParam.ioCRefNum = mRefNum;
		*((void **) &(theParamBlock.cntrlParam.csParam)) = &theData;
		theParamBlock.cntrlParam.csCode = k3DfxNewRequest;
		theData.which = k3DfxGetDriverInfo;
		theData.request = &req;
		theData.response = &res;
    
		theSuccess = (OSErr) PBControlSync(&theParamBlock);

		if ( theSuccess == 0 )
		{	
			*outMajor = (int) res.optData.driverInfoRes.major;
			*outMinor = (int) res.optData.driverInfoRes.minor;
            *outStage = (int) res.optData.driverInfoRes.stage;
            *outRev = (int) res.optData.driverInfoRes.rev;
		}
	}
	else
	{
		*outMajor = *outMinor = *outStage = *outRev = 0;
	}

}

/*______________________________________________________________________________

GetCurrentGraphicsFreq

*/

long
CNapalmHardware::GetCurrentGraphicsFreq()
{
#if 0
    unsigned long theValue;
    long n,m,k;
    float freq;
    
//    HWC_IO_LOAD( inTarget->bInfo.regInfo, pllCtrl1, theValue );

    n = (theValue >> 8) & 0xFF;
    m = (theValue >> 2) & 0x3F;
    k = 1 << (theValue & 0x3);

    freq = (14.31818 * ((float) n + 2)) / ( ((float) m + 2 ) * ( (float) k ) );
    cls_printf( "Graphics Freq = %.2f Mhz, pllCtrl1 = 0x%08x\n", freq, theValue );

	return theFreq;
#endif
	
	return 0;
}


/*______________________________________________________________________________

SetCurrentGraphicsFreq

*/

void
CNapalmHardware::SetCurrentGraphicsFreq(
	long				inFreq)
{

	UInt32 theValueSize;
//	RegPropertyValueSize theValueSize;
	hwGfxNVram_t theGraphicsNonVolatile;
	OSErr theSuccess;


	/* Does the propertyName exist? Is the storage provided by the caller big enough to hold data? */
	theSuccess = RegistryPropertyGetSize( &mRegEntryID, kPreferredConfigurationName, &theValueSize);
	if ( !theSuccess )
	{
		RegPropertyModifiers theModifiers;
		theSuccess = RegistryPropertyGetMod(  &mRegEntryID, kPreferredConfigurationName, &theModifiers);
		theModifiers |= kRegPropertyValueIsSavedToNVRAM;
		theSuccess = RegistryPropertySetMod(  &mRegEntryID, kPreferredConfigurationName, theModifiers);

		theSuccess = RegistryPropertyGet( &mRegEntryID, kPreferredConfigurationName,
												&theGraphicsNonVolatile, &theValueSize);
		theGraphicsNonVolatile.halData = (theGraphicsNonVolatile.halData & ~(HALDATA_GRXCLOCK_MASK >> HALDATA_GRXCLOCK_SHIFT))
												| ((inFreq & (HALDATA_GRXCLOCK_MASK >> HALDATA_GRXCLOCK_SHIFT)) << HALDATA_GRXCLOCK_SHIFT );
		theSuccess = RegistryPropertySet( &mRegEntryID, kPreferredConfigurationName,
											&theGraphicsNonVolatile, sizeof(hwGfxNVram_t));

	}
}





/*______________________________________________________________________________

GetSetting

*/

short
CNapalmHardware::GetSettingUL(
	const char *		inSettingName,
	UInt32 *			outSetting)
{
	short				theResult;
	
	theResult =	hrm_GetSettingUL( mGDHandle, inSettingName,  outSetting);

	return theResult;
}





/*______________________________________________________________________________

SetSettingUL

*/
short
CNapalmHardware::SetSettingUL(
	const char *		inSettingName,
	UInt32				inSetting)
{
	short				theResult;
	
	theResult =	hrm_SetSettingUL( mGDHandle, inSettingName,  inSetting);

	return theResult;
}





/*______________________________________________________________________________

GetSettingF

*/

short
CNapalmHardware::GetSettingF(
	const char *		inSettingName,
	float *			outSetting)
{
	short				theResult;
	
	theResult =	hrm_GetSettingF( mGDHandle, inSettingName,  outSetting);

	return theResult;
}





/*______________________________________________________________________________

SetSettingF

*/
short
CNapalmHardware::SetSettingF(
	const char *		inSettingName,
	float				inSetting)
{
	short				theResult;
	
	theResult =	hrm_SetSettingF( mGDHandle, inSettingName,  inSetting);

	return theResult;
}

