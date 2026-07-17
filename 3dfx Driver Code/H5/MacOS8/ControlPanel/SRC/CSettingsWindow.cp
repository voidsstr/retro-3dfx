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

#include "CSettingsWindow.h"
#include "CViewColumn.h"
#include "CViewCell.h"

#include "ControlPanelPPob.h"
#include <LCaption.h>
#include <LStdControl.h>
#include <LMultiPanelView.h>
#include <LView.h>
#include <LPopupGroupBox.h>
#include <LPushButton.h>

#include <LArray.h>
#include <TArrayIterator.h>
#include <LListener.h>
#include <UReanimator.h>

#include <PP_Messages.h>
#include <String.h>


/*______________________________________________________________________________

	Constructor

*/

CSettingsWindow::CSettingsWindow()
{
}


/*______________________________________________________________________________

	Constructor (using a Stream, that's the one that will be used)

*/

CSettingsWindow::CSettingsWindow(
	LStream *			inStream)
	: LWindow ( inStream )
{
	mGIP = nil;
}


/*______________________________________________________________________________

	Destructor

*/

CSettingsWindow::~CSettingsWindow()
{
}


#pragma mark -

/*______________________________________________________________________________

	FinishCreateSelf

*/

void 
CSettingsWindow::FinishCreateSelf()
{		
	LPushButton* theButton = dynamic_cast <LPushButton*>
											(this->FindPaneByID(SetGame_ButtCancel));
	theButton->AddListener(this);
	theButton = dynamic_cast <LPushButton*> (this->FindPaneByID(SetGame_ButtSave));
	theButton->AddListener(this);
}



#pragma mark -

/*______________________________________________________________________________

	ListenToMessage

*/


void CSettingsWindow::ListenToMessage(MessageT inMessage, void *ioParam)
{
	UInt32 longParam = *(UInt32*)ioParam;
	
	switch(inMessage) {
		case msg_SetGameCancl:
		{
			this->DoClose();
			break;
		}
		
		case msg_SetGameSave:
		{
			// Save the New Settings
			SaveNewSettings(GetmGIP());
			
			this->DoClose();
			break;
		}	
		
	
		default:
			break;
	}
}

#pragma mark -


/*______________________________________________________________________________

	SetGameInfoSettings

*/

void
CSettingsWindow::SetGameInfoSettings(GameInfoP	inGIP)
{
	Str255 temp;
	
	LCaption * Title = (LCaption*) this->FindPaneByID(SetGame_CaptTitle);
	strcpy((char *) &temp[1], inGIP->GameName);
	temp[0] = strlen( inGIP->GameName );
	Title->SetDescriptor(temp);
	
}


/*______________________________________________________________________________

	SetmGIP

*/

void
CSettingsWindow::SetmGIP(GameInfoP	inGIP)
{
	mGIP = inGIP;
}


/*______________________________________________________________________________

	GetmGIP

*/

GameInfoP
CSettingsWindow::GetmGIP()
{
	return	mGIP;
}

/*______________________________________________________________________________

	SaveNewSettings

*/

void
CSettingsWindow::SaveNewSettings(GameInfoP	/*inGIP*/)
{
	// Save the new settings here !
	
}
