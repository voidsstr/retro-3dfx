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


#include "CBansheeHardware.h"
#include <NameRegistry.h>
#include <DriverServices.h>
#include <PCI.h>


/*______________________________________________________________________________

Constructor

*/

CBansheeHardware::CBansheeHardware(
	RegEntryID *		inRegEntryID )
		: CPCIHardware( inRegEntryID )
{
	mRegEntryID = *inRegEntryID;
}


/*______________________________________________________________________________

Destructor

*/

CBansheeHardware::~CBansheeHardware()
{
}


/*______________________________________________________________________________

SetROMData

*/

short
CBansheeHardware::SetROMData(
	char *				/*inData*/,
	long				/*inDataSize*/)
{
	return -1;
}



/*______________________________________________________________________________

GetROMData

*/

void
CBansheeHardware::GetROMData(
	char **				outData,
	long *				outDataSize)
{
	*outData = 0;
	*outDataSize = 0;
}



/*______________________________________________________________________________

GetCurrentGraphicsFreq

*/

long
CBansheeHardware::GetCurrentGraphicsFreq()
{
	return 0;
}

/*______________________________________________________________________________

SetCurrentGraphicsFreq

*/

void
CBansheeHardware::SetCurrentGraphicsFreq(
	long				/*inFreq*/ )
{
}



/*______________________________________________________________________________

GetSettingUL

*/

short
CBansheeHardware::GetSettingUL(
	const char *		/*inSettingName*/,
	UInt32 *			outSetting)
{
	*outSetting = 0;
	return 0;
}





/*______________________________________________________________________________

SetSettingUL

*/
short
CBansheeHardware::SetSettingUL(
	const char *		/*inSettingName*/,
	UInt32				/*inSetting*/)
{
	return 0;
}



/*______________________________________________________________________________

GetSettingF

*/

short
CBansheeHardware::GetSettingF(
	const char *		/*inSettingName*/,
	float *			outSetting)
{
	*outSetting = 0.0;
	return 0;
}





/*______________________________________________________________________________

SetSettingUL

*/
short
CBansheeHardware::SetSettingF(
	const char *		/*inSettingName*/,
	float				/*inSetting*/)
{
	return 0;
}
