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

#include "CControlPanel.h"
#include "CControlPanelApp.h"
#include "CAboutWindow.h"
#include "CUpdateDrv.h"
#include "ControlPanelPPob.h"
#include "CGETVerThread.h"
#include "CCheckRom.h"
#include "CPCIHardware.h"
#include "CInternetConfig.h"

#include <LTextButton.h>
#include <LPushButton.h>
#include <LThread.h>



/*______________________________________________________________________________

	Constructor

*/

CAboutWindow::CAboutWindow()
{
	
}


/*______________________________________________________________________________

	Constructor (using a Stream, that's the one that will be used)

*/

CAboutWindow::CAboutWindow(
	LStream *			inStream)
	: LView ( inStream )
{
}


/*______________________________________________________________________________

	Destructor

*/

CAboutWindow::~CAboutWindow()
{
	//CControlPanelApp* theApp = (CControlPanelApp*) FindWindowByID(ControlPanel_Window);
	//theApp->SetAboutWindowFlag(false);
	
}


#pragma mark -

/*______________________________________________________________________________

	FinishCreateSelf

*/

void 
CAboutWindow::FinishCreateSelf()
{	
	Str255		temp, temp2;
	
	LPushButton* theOKButton = (LPushButton*) FindPaneByID(about_OK);
	if (theOKButton != nil)		theOKButton->AddListener(this);
	
	LPushButton* theUpdtButton = (LPushButton*) FindPaneByID(about_UpdateDriver);
	if (theUpdtButton != nil)		theUpdtButton->AddListener(this);
	
	LCaption *	theVersionCapt = (LCaption*) FindPaneByID(about_Version);
	if (theVersionCapt != nil)
	{
		sprintf( (char*) &temp[1], "Version : %s", kMACTOOLS_ABTWND_VER);
		temp[0] = strlen( (char*) &temp[1] );
		theVersionCapt->SetDescriptor( temp );
	}		
	
	LCaption *	theBuildVersionCapt = (LCaption*) FindPaneByID(about_BuildVersion);
	if (theBuildVersionCapt != nil)
	{
		sprintf( (char*) &temp2[1], "Build : %s", kMACTOOLS_BUID_VERSION);
		temp2[0] = strlen( (char*) &temp2[1] );
		theBuildVersionCapt->SetDescriptor( temp2 );
	}	
	
	
	LTextButton * thetdfxURL = (LTextButton*) FindPaneByID(about_tdfxURL);
	if (thetdfxURL != nil)
	{
		thetdfxURL->AddListener(this);
	}
	
}



/*______________________________________________________________________________

	ListenToMessage

*/


void CAboutWindow::ListenToMessage(
	MessageT 	inMessage, 
	void *		ioParam)
{
	UInt32 		longParam = *(UInt32*)ioParam;

	switch(inMessage) 
	{
		/*case about_OK:
		{
			CleanThreads();
			this->DoClose();
			break;
		}*/
		
		case about_UpdateDriver:
		{
			UpdateDriverNow();
			break;
		}
		case 'mURL':
		{
			CInternetConfig* mIC = new CInternetConfig( 'CŸRL' ); // test application signature
			
			if ( mIC )
			{
				OSErr err;
				
				err = mIC->Start();
				if ( err == noErr )
				{
					mIC->DoURL("\phttp://www.3dfx.com/mac/");
					LTextButton* theText = (LTextButton*) FindPaneByID('mURL');
					theText->SetValue(Button_Off);
				}
			}
			
			break;
		}
		
		default:
			break;
	}
}

#pragma mark -



/*______________________________________________________________________________

	SetPCIHardware

*/

void
CAboutWindow::SetPCIHardware(
	CPCIHardware *		inBoard)
{
	Boolean		NoErr;
	
	if ( inBoard != 0 )
	{
		mBoard = inBoard;
	}

}


/*______________________________________________________________________________

	SetCPWindow

*/

void
CAboutWindow::SetCPWindow(
	CControlPanel *		inCPanel)
{
		mCPanel = inCPanel;
}

/*______________________________________________________________________________

	UpdateDriverNow

*/

void
CAboutWindow::UpdateDriverNow()
{
	/* Create the Update Driver Window */
	CUpdateDrv* mDisplayWindow = (CUpdateDrv*) LWindow::CreateWindow(UpdateDrv_Window, mCPanel);
	ThrowIfNil_(mDisplayWindow);
		
}


/*______________________________________________________________________________

	CleanThreads()

*/

void
CAboutWindow::CleanThreads()
{
	//if (theThread != nil)
		//theThread->ExitThreads();
}