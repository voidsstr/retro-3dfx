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


#include "UTextLocalization.h"


/*______________________________________________________________________________

GetLocalText

*/

unsigned char *
GetLocalText(
	TextMessagesT		inMessageID)
{
	unsigned char *		theResult = 0;
	
	switch( inMessageID ) {
		case kQuitText: theResult = "\pQuit\0"; break;
		case kOKText: theResult = "\pOK\0"; break;
		case kCancelText: theResult = "\pCancel\0"; break;
		case kRestartText: theResult = "\pRestart\0"; break;
		case kUnknownText: theResult = "\pUnknown\0"; break;
				
		case kWintelText: theResult = "\pIntel PC\0"; break;
		case kMacintoshText: theResult = "\pMacintosh\0"; break;

		case kFlashSucceedMessage: theResult = "\pMacTools firmware upgrade was a complete success.However, in order to use it, you need to restart your computer.\0"; break;
		case kNoHardwareMessage: theResult = "\pNo qualifying 3dfx hardware could be found. You cannot use this software.\0"; break;
	}
	
	return theResult;
}
