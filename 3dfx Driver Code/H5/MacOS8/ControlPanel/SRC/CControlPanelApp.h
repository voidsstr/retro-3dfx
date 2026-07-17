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

#pragma once

#include <PP_Prefix.h>
#include <LApplication.h>
#include <UThread.h>
#include "CAboutWIndow.h"

#include "CControlPanel.h"

class CHardwareHost;

class CControlPanelApp : public PP_PowerPlant::LApplication {

public:
							CControlPanelApp();
	virtual					~CControlPanelApp();

	virtual Boolean			ObeyCommand(
								CommandT			inCommand,
								void*				ioParam = nil);	

	virtual void			FindCommandStatus(
								CommandT			inCommand,
								Boolean&			outEnabled,
								Boolean&			outUsesMark,
								UInt16&				outMark,
								Str255				outName);
	
	void					SetAboutWindowFlag(Boolean theFlag);
	
	int						CheckVoodooVersion(
								CPCIHardware *		inBoard);
								
									
protected:
	virtual void			StartUp();
	
			void			RegisterClasses();
			
	static CHardwareHost *	sTheHost;
	CControlPanel *			mBoardRec[7];
	Boolean					IsAboutWndExists;
	CAboutWindow *			theAboutWindow;
	CPCIHardware * 			theBoard;
};
