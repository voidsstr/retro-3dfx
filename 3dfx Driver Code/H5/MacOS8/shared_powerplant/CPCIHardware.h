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


#ifndef __CPCIHardware__
#define __CPCIHardware__


#include <NameRegistry.h>
#include <Quickdraw.h>

class	CPCIHardware {

	//_____ Initialization

public:
							CPCIHardware(
									RegEntryID *		inRegEntryID );
	virtual 				~CPCIHardware();

	virtual void			Init();

	//_____ Accessors

	virtual bool			IsActive();

	virtual RegEntryID *	GetRegEntryID( void ) { return &mRegEntryID;}
	
	virtual short			GetRefNum();

	virtual short			ReadPCIConfig(
									unsigned long		inAddress,
									long				inStartBit,
									long				inEndBit,
									unsigned long *		outValue);
	
	virtual long			GetMemorySize();




	virtual void			GetSlotName(
									Str255				outString);

	virtual void			GetModelName(
									Str255				outString);

	virtual void			GetPlatform(
									Str255				outString);

	virtual void			GetVersion(
									Str255				outString);
									
	virtual void			GetROMVersion(
									UInt8 *				outMajor,
									UInt8 *				outMinor,
									UInt8 *				outStage,
									UInt8 *				outRev);
									
	virtual void			GetBusTypeName(
									Str255				outString);

	virtual void			GetChipName(
									Str255				outString);

	virtual long			GetNumberOfChips();




	virtual short			SetROMData(
									char *				inData,
									long				inDataSize) = 0;

	virtual void			GetROMData(
									char **				outData,
									long *				outDataSize) = 0;




	virtual long			GetCurrentGraphicsFreq() = 0;

	virtual void			SetCurrentGraphicsFreq(
									long				inFreq) = 0;




	virtual short			GetSettingUL(
									const char *		inSettingName,
									UInt32 *			outSetting) = 0;

	virtual short			SetSettingUL(
									const char *		inSettingName,
									UInt32				inSetting) = 0;

	virtual short			GetSettingF(
									const char *		inSettingName,
									float *				outSetting) = 0;

	virtual short			SetSettingF(
									const char *		inSettingName,
									float				inSetting) = 0;




protected:
	
	RegEntryID				mRegEntryID;
	char					mSlotName[64];
	char					mModelName[64];
	char					mChipName[64];
	char					mBusTypeName[64];
	long					mNumberOfChips;
	long					mMemorySize;
	short                   mRefNum;
	GDHandle                mGDHandle;					
	
	
};


#endif __CPCIHardware__