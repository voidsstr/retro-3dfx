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

#include "C2DPerfView.h"
#include "C3DPerfView.h"
#include "CDisplayView.h"
#include "CProfileView.h"
#include "CGameView.h"
#include "CDebugView.h"
#include "COpenGLView.h"
#include "CGlideView.h"
#include "CRaveView.h"

#include "CCheckRom.h"

#include "CPCIHardware.h"
#include "CViewColumn.h"
#include "CViewCell.h"
#include "CSettingsWindow.h"
#include "hrm_user_settings.h"
#include <Gestalt.h>

#include "ControlPanelPPob.h"
#include <LCaption.h>
#include <LStdControl.h>
#include <LMultiPanelView.h>
#include <LView.h>
#include <LPopupGroupBox.h>
#include <LPushButton.h>
#include <LPopupButton.h>
#include <LSlider.h>
#include <LMenu.h>
#include <LString.h>

#include <LArray.h>
#include <TArrayIterator.h>
#include <LListener.h>
#include <UReanimator.h>
#include <String.h>

#include <PP_Messages.h>

#include <UDesktop.h>


#include <LMenu.h>


/*______________________________________________________________________________

	Constructor

*/

CControlPanel::CControlPanel()
{
	
}


/*______________________________________________________________________________

	Constructor (using a Stream, that's the one that will be used)

*/

CControlPanel::CControlPanel(
	LStream *			inStream)
	: LWindow ( inStream )
{
	StartListening();
	mBoard = nil;

}


/*______________________________________________________________________________

	Destructor

*/

CControlPanel::~CControlPanel()
{
	StopListening();
	SaveCurrentDisplaySettings();
}


#pragma mark -

/*______________________________________________________________________________

	FinishCreateSelf

*/

void 
CControlPanel::FinishCreateSelf()
{	
	StopListening();	
	
	// Get the main multi panel view and load up all the panels
	mainMPV = dynamic_cast<LMultiPanelView*> (this->FindPaneByID(main_MultiPanelView));
	ThrowIfNil_(mainMPV);	
	mainMPV->CreateAllPanels();
	mainMPV->SwitchToPanel(1);

	// Set the FinishCreateSelf for all the views
	FinishCreateSelf_Games();
	
	StartListening();
}





/*______________________________________________________________________________

FinishCreateSelf_Games

*/

void
CControlPanel::FinishCreateSelf_Games()
{
	// Get the 3DPerf View (Panel)
	LView*	GamesView = (LView*) this->FindPaneByID(game_Main);
	
	// Setup Edit Button
	LPushButton*		theButton = dynamic_cast<LPushButton*>
										(GamesView->FindPaneByID(game_ButtEdit));
	if (theButton != nil) theButton->AddListener(this);
		
	// Setup Add Button
	theButton = dynamic_cast <LPushButton*>
										(GamesView->FindPaneByID(game_ButtAdd));
	if (theButton != nil) theButton->AddListener(this);
	
	// Setup Delete Button
	theButton = dynamic_cast <LPushButton*>
										(GamesView->FindPaneByID(game_ButtDelete));
	if (theButton != nil) theButton->AddListener(this);
	
	
}




#pragma mark -


/*______________________________________________________________________________

	ListenToMessage

*/


void CControlPanel::ListenToMessage(MessageT inMessage, void *ioParam)
{
	UInt32 		longParam = *(UInt32*)ioParam;
	
	switch(inMessage) {
		case pane_BI_Quit:
		{
			ObeyCommand(PP_PowerPlant::cmd_Quit, nil);
			break;
		}
		
		case msg_mainTabs:
		{
			LMultiPanelView * mainPanels = (LMultiPanelView *)FindPaneByID(main_MultiPanelView);

			if(mainPanels)
				mainPanels->SwitchToPanel(longParam);
			break;
		}
		
		case msg_Games_bEdit:
		{
			GameInfoP	theGIP;
			
			CViewColumn* theColumnView = dynamic_cast<CViewColumn*>
												(this->FindPaneByID(game_ViewColumn));
			
			CViewCell* currentCell = theColumnView->GetLastCellSelected();
			
			CSettingsWindow* theSettingsWindow = (CSettingsWindow*) LWindow::CreateWindow(GameSettings_Window, this);
			
			if (theSettingsWindow != nil)
			{
				theGIP = currentCell->GetGameInfoPtr();
				theSettingsWindow->SetGameInfoSettings(theGIP);
				theSettingsWindow->SetmGIP(theGIP);
				
				theSettingsWindow->Show();
			}
			break;
		}
		
		case msg_Games_bAdd:
		{
			break;
		}
		case msg_Games_bDelete:
		{
			break;
		}
	
		/*		Display View		*/	
		
		default:
			break;
	}
}





/*______________________________________________________________________________

	ClickInGoAway

*/

void 
CControlPanel::ClickInGoAway(const EventRecord& inMacEvent)
{
	if (::TrackGoAway(mMacWindowP, inMacEvent.where)) {
		GetSuperCommander()->ObeyCommand(cmd_Quit, nil);
	}
}





/*______________________________________________________________________________

	OffsetWindow

*/

void
CControlPanel::OffsetWindow(
	short				inIndex)
{
	if (inIndex != 0 ) {
		MoveWindowBy( 16 * inIndex, 32 * inIndex );
	}
}





/*______________________________________________________________________________

	SetPCIHardware

*/

void
CControlPanel::SetPCIHardware(
	CPCIHardware *		inBoard)
{
	if ( inBoard != 0 ) 
	{
		mBoard = inBoard;
		
		/* Set the inBoard Value to all the views */
		InitPCIHardware();
		
	}

}



/*______________________________________________________________________________

	DoCheckROM

*/

void
CControlPanel::DoCheckROM()
{
	//this->Hide();
	
	/*	Check if the current ROM with the MT RomU.rsrc	*/
	CCheckRom *	theBoardRom = (CCheckRom*) new CCheckRom(mBoard, this);
	if (theBoardRom != nil)
		theBoardRom->CheckRomProcess();
					
//					delete (theBoardRom);	
	//this->Show();

}


	

/*______________________________________________________________________________

	GetPCIHardware

*/

CPCIHardware *
CControlPanel::GetPCIHardware()
{
	return mBoard;
}





/*______________________________________________________________________________

	InitPCIHardware

*/

void
CControlPanel::InitPCIHardware()
{
	C2DPerfView* the2DPerfView = (C2DPerfView*) this->FindPaneByID(perf2D_Main);
	the2DPerfView->SetVoodooVersion( mVVersion );
	the2DPerfView->SetPCIHardware( mBoard );
	
	C3DPerfView* the3DPerfView = (C3DPerfView*) this->FindPaneByID(perf3D_Main);
	the3DPerfView->SetVoodooVersion( mVVersion );
	the3DPerfView->SetPCIHardware( mBoard );
	
	CDisplayView* theDisplayView = (CDisplayView*) this->FindPaneByID(display_Main);
	theDisplayView->SetPCIHardware( mBoard );
	theDisplayView->SetControlPanelPtr( this );
	
	CProfileView* theProfileView = (CProfileView*) this->FindPaneByID(profile_Main);
	theProfileView->SetVoodooVersion( mVVersion );
	theProfileView->SetPCIHardware( mBoard );
	theProfileView->Set2DPerfView( the2DPerfView );
	theProfileView->Set3DPerfView( the3DPerfView );
	
	
	CGameView* theGameView = (CGameView*) this->FindPaneByID(game_Main);
	theGameView->SetPCIHardware( mBoard );
	
	CDebugView* theDebugView = (CDebugView*) this->FindPaneByID(debug_Main);
	theDebugView->SetPCIHardware( mBoard );
	
	CAboutWindow* theAboutView = (CAboutWindow*) this->FindPaneByID(about_Main);
	theAboutView->SetPCIHardware( mBoard );
	theAboutView->SetCPWindow( this );
	
}





/*______________________________________________________________________________

	CenterWindowOnDisplay

*/

void
CControlPanel::CenterWindowOnDisplay()
{
	GDHandle	theDevice  = ::GetDeviceList();

	while (theDevice != nil) {
	
		if ( (*theDevice)->gdRefNum == mBoard->GetRefNum() ) {
		
			Rect 	r = (**theDevice).gdRect;
			long	x,y;
			
			x = r.right - r.left;
			y = r.bottom - r.top;
			
			x = ( x - mFrameSize.width) / 2 + r.left;
			y = ( y - mFrameSize.height) / 2 + r.top;
			
	  		MoveWindowTo( x, y );
	  		mFrameLocation.h = y;
  			mFrameLocation.v = x;
	  	}

  		theDevice = ::GetNextDevice(theDevice);
	}
}


/*______________________________________________________________________________

	ClickInDrag

*/

void
CControlPanel::ClickInDrag(
	const EventRecord	&inMacEvent)
{
	GDHandle	theDevice  = ::GetDeviceList();

	while (theDevice != nil) {
	
		if ( (*theDevice)->gdRefNum == mBoard->GetRefNum() ) {
		
			Rect 	r = (**theDevice).gdRect;		
												// Save old bounds (content region
												//	is in global coords)
			Rect	bounds = (*((WindowPeek)mMacWindowP)->contRgn)->rgnBBox;
			
			//Rect	dragRect = (**(GetGrayRgn())).rgnBBox;
			
			Rect	dragRect;
			
			dragRect.left = ((&inMacEvent)->where.h - bounds.left) + r.left;
			dragRect.top = ((&inMacEvent)->where.v - bounds.top) + 20 + r.top;
			dragRect.right = r.right - ((bounds.left + mFrameSize.width) - (&inMacEvent)->where.h);
			dragRect.bottom = r.bottom - ((bounds.top + mFrameSize.height) - (&inMacEvent)->where.v);

			::MacInsetRect(&dragRect, 4, 4);
			UDesktop::DragDeskWindow(this, inMacEvent, dragRect);
			
				// DragWindow will move the window. However, send AppleEvent to
				// set the bounds (but don't execute it), so that Script engines
				// can record the action.
				
												// Compare old and new bounds
			if (!::MacEqualRect(&bounds,
							    &(*((WindowPeek)mMacWindowP)->contRgn)->rgnBBox)) {
							
				Point	newPosition =
							topLeft((*((WindowPeek)mMacWindowP)->contRgn)->rgnBBox);
							
												// Send but don't execute
												//   SetPosition AppleEvent
				SendAESetPosition(newPosition, ExecuteAE_No);
				
				AdjustUserBounds();				// Moving Window changes user
												//   bounds for zooming
												
				
			}
					

	  	}
  			
  		theDevice = ::GetNextDevice(theDevice);
	}

}


/*______________________________________________________________________________

	SaveCurrentDisplaySettings

*/

void
CControlPanel::SaveCurrentDisplaySettings()
{
	Boolean		displayMgrPresent;
	long		value = 0;

	Gestalt(gestaltDisplayMgrAttr,&value);
	displayMgrPresent=value&(1<<gestaltDisplayMgrPresent);
	if (displayMgrPresent)
		DMSaveScreenPrefs(NULL, NULL, NULL);
}



/*______________________________________________________________________________

	SetIsV3

*/

void
CControlPanel::SetVoodooVersion(
	int		inValue)
{
	mVVersion = inValue;
}
