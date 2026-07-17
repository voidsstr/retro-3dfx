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


#ifndef __CBansheeHardware__
#define __CBansheeHardware__


#include "CPCIHardware.h"

class	CBansheeHardware : public CPCIHardware {

	//_____ Initialization

public:
							CBansheeHardware(
									RegEntryID *		inRegEntryID );
	virtual 				~CBansheeHardware();

	virtual short			SetROMData(
									char *				inData,
									long				inDataSize);

	virtual void			GetROMData(
									char **				outData,
									long *				outDataSize);

	virtual long			GetCurrentGraphicsFreq();

	virtual void			SetCurrentGraphicsFreq(
									long				inFreq);

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
};


#endif __CBansheeHardware__