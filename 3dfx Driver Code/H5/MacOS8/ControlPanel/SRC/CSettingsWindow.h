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

#ifndef __CSettingsWindow__
#define __CSettingsWindow__

#include <LWindow.h>
#include <LStream.h>
#include <LListener.h>
#include <LArray.h>
#include <TArray.h>
#include "ControlPanelPPob.h"

class	CSettingsWindow :	public LWindow, public LListener
										 {
public:
	enum { class_ID = 'GSET' };

	//_____ Initialization

							CSettingsWindow();
							CSettingsWindow(	
									LStream *			inStream);
	virtual					~CSettingsWindow();
	
	void					SetGameInfoSettings(
									GameInfoP			inGIP);
	
public:
	//_____ Accessors
	void					FinishCreateSelf();
	
	void					ListenToMessage(
									MessageT 			inMessage, 
									void *				ioParam);
	
	void					SetmGIP(
									GameInfoP			inGIP);
			
	GameInfoP				GetmGIP();

	void					SaveNewSettings(
									GameInfoP			inGIP);
	

private:

	GameInfoP				mGIP;

};

#endif /* __CSettingsWindow__ */