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


#ifndef __CVoodoo3Hardware__
#define __CVoodoo3Hardware__


#include "CPCIHardware.h"

struct TRomInfo;

class	CVoodoo3Hardware : public CPCIHardware {

	//_____ Initialization

public:
							CVoodoo3Hardware(
									RegEntryID *		inRegEntryID );
	virtual 				~CVoodoo3Hardware();

	virtual short			SetROMData(
									char *				inData,
									long				inDataSize);

	virtual void			GetROMData(
									char **				outData,
									long *				outDataSize);

	virtual void			GetPlatform(
									Str255				outString);

	virtual void			GetVersion(
									Str255				outString);
				
	void					GetROMVersion(
									UInt8 *				outMajor,
									UInt8 *				outMinor,
									UInt8 *				outStage,
									UInt8 *				outRev);
									
	virtual long			GetCurrentGraphicsFreq();
	
	virtual void			SetCurrentGraphicsFreq(
									long				inFreq);
	
	//virtual long			GetMemorySize();

	virtual short			GetSettingUL(
									const char *		inSettingName,
									UInt32 *			outSetting);

	virtual short			SetSettingUL(
									const char *		inSettingName,
									UInt32				inSetting);

	virtual short			GetSettingF(
									const char *		inSettingName,
									float *				outSetting);

	virtual short			SetSettingF(
									const char *		inSettingName,
									float				inSetting);


private:

	UInt32					mHwBaseAddress;
	UInt32					mLfbBaseAddress;
	UInt32					mIoPortAddress;
	UInt32					mRomBaseAddress;

	TRomInfo *				mRomInfo;
};


#endif __CVoodoo3Hardware__