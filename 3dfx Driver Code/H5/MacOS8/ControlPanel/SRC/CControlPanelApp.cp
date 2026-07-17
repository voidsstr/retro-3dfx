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

#include "CControlPanelApp.h"
#include "CControlPanel.h"
#include "CAboutWindow.h"
#include "CUpdateDrv.h"
#include "CCheckRom.h"
#include "CFlashRomWindow.h"
#include "CUpdateRom.h"
#include "CEndUpdateRom.h"
#include "International_Lang.h"
#include "C2DPerfView.h"
#include "C3DPerfView.h"
#include "CDisplayView.h"
#include "CProfileView.h"
#include "CGameView.h"
#include "CDebugView.h"

#include "COpenGLView.h"
#include "CGlideView.h"
#include "CRaveView.h"

#include "CHardwareHost.h"
#include "CViewColumn.h"
#include "CViewCell.h"
#include "CKeyComboField.h"
#include "CPopupButton.h"
#include "CSettingsWindow.h"
#include "ControlPanelPPob.h"
#include "U2ButtonsDialog.h"
#include <LTextButton.h>

#include <LGrowZone.h>
#include <PP_Messages.h>
#include <PP_Resources.h>
#include <UDrawingState.h>
#include <UMemoryMgr.h>
#include <URegistrar.h>
#include <UEnvironment.h>
#include <UAttachments.h>
#include <LMenu.h>

#include <UControlRegistry.h>

#include <LWindow.h>
#include <LView.h>
#include <LCaption.h>
#include <LTabsControl.h>
#include <LPopupButton.h>
#include <LDragAndDrop.h>
#include <LDragTask.h>
#include <LVariableArray.h>
#include <LTextEditView.h>

#include <Appearance.h>

#include <UModalDialogs.h>
#include <UReanimator.h>
#include <LDialogBox.h>
#include <LStdControl.h>
#include <LIconPane.h>
#include <LScroller.h>
#include <LProgressBar.h>
#include <LMultiPanelView.h>

#include <LPicture.h>
#include <UGraphicUtils.h>

#include <Notification.h>
#include <string.h>

#include <Gestalt.h>

//Use this to tune the tick delay of the LYieldAttachment
#define 	kAppYieldQuantum 	-1
#define		kVoodooName			"Voodoo"	

CHardwareHost *	CControlPanelApp::sTheHost;

// ===========================================================================
//	¥ main
// ===========================================================================

int main()
{							
		// Set Debugging options
	SetDebugThrow_(debugAction_Alert);
	SetDebugSignal_(debugAction_Alert);

		// Initialize Memory Manager. Parameter is the number of
		// master pointer blocks to allocate
	InitializeHeap(3);
	
		// Initialize standard Toolbox managers
	UQDGlobals::InitializeToolbox(&qd);
	
	if (UEnvironment::HasFeature(env_HasThreadsManager))
		new UMainThread;

	
		// Install a GrowZone to catch low-memory situations	
	new LGrowZone(200000);

		// Create the application object and run
	CControlPanelApp	theApp;
	theApp.Run();
	
	return 0;
}


// ---------------------------------------------------------------------------
//	¥ CControlPanelApp								[public]
// ---------------------------------------------------------------------------
//	Application object constructor

CControlPanelApp::CControlPanelApp()
{
		// Register ourselves with the Appearance Manager
	if (UEnvironment::HasFeature(env_HasAppearance)) {
		::RegisterAppearanceClient();
	}

	RegisterClasses();
}


// ---------------------------------------------------------------------------
//	¥ CControlPanelApp								[public, virtual]
// ---------------------------------------------------------------------------
//	Application object destructor

CControlPanelApp::~CControlPanelApp()
{
	// Nothing
}


// ---------------------------------------------------------------------------
//	¥ StartUp										[protected, virtual]
// ---------------------------------------------------------------------------
//	Perform an action in response to the Open Application AppleEvent.
//	Here, issue the New command to open a window.

void
CControlPanelApp::StartUp()
{
	IsAboutWndExists = false;
	
	short 	theNumberOfTargets = 0;
	short	theNumberOfControlPanels = 0;
	
	sTheHost = new CHardwareHost;
	
	ThrowIfNil_(sTheHost);
	
	for (short i = 0; i <=7; i++)
		mBoardRec[i] = 0;
	
	theNumberOfTargets = sTheHost->GetNumberOfTargets();
	if ( theNumberOfTargets > 0 )
	{
		short i = 0;
		while ( i < theNumberOfTargets ) {

			theBoard = sTheHost->GetTarget( i );
			if ( theBoard->IsActive() )
			{
				CControlPanel * theBoardControlPanel = (CControlPanel*) LWindow::CreateWindow(ControlPanel_Window, this);

				if (theBoardControlPanel != nil)
				{
					theBoardControlPanel->SetVoodooVersion( CheckVoodooVersion( theBoard ) );
					
					theBoardControlPanel->Hide();
					theBoardControlPanel->SetPCIHardware( theBoard );
					theBoardControlPanel->CenterWindowOnDisplay();	
					theBoardControlPanel->DoCheckROM();
							
					//theBoardControlPanel->Show();
				}
			
				theNumberOfControlPanels++;
				mBoardRec[i] = theBoardControlPanel;
			}
			
			i++;
			
		}	
	}
	else 
	{
		Show2ButtonsDialog( this, kNoHardwareMessage, kQuit, kNone);
		ObeyCommand(cmd_Quit, nil);
	}
}


// ---------------------------------------------------------------------------
//	¥ ObeyCommand									[public, virtual]
// ---------------------------------------------------------------------------
//	Respond to Commands. Returns true if the Command was handled, false if not.

Boolean
CControlPanelApp::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = true;	// Assume we'll handle the command

	switch (inCommand) {
		
		case cmd_About: 
		{
			
			for (short i = 0; i <= 7; i++)
			{
				if (mBoardRec[i] != 0)
				{
					LMultiPanelView* mainMPV = (LMultiPanelView*) (mBoardRec[i]->FindPaneByID(main_MultiPanelView));
					ThrowIfNil_(mainMPV);	
					mainMPV->SwitchToPanel(1);
					LTabsControl* mainTabs = (LTabsControl*)  mBoardRec[i]->FindPaneByID(main_TabsControl);
					ThrowIfNil_(mainTabs);
					mainTabs->SetValue(1);
					mBoardRec[i]->Refresh();
				}
			}
			
			/*CControlPanel * theCPanel = (CControlPanel*) this->FindPaneByID(ControlPanel_Window);
			if (theCPanel != nil)
			{
				LMultiPanelView* mainMPV = (LMultiPanelView*) (theCPanel->FindPaneByID(main_MultiPanelView));
				ThrowIfNil_(mainMPV);	
				mainMPV->SwitchToPanel(1);
			}
			theAboutWindow = (CAboutWindow*) LWindow::CreateWindow(About_Window, this);
			if (theAboutWindow != nil)
			{
				theAboutWindow->SetPCIHardware( theBoard );
				theAboutWindow->Show();
			}*/
			break;
		}
	
		default: {
			cmdHandled = LApplication::ObeyCommand(inCommand, ioParam);
			break;
		}
	}
	
	return cmdHandled;
}


/*______________________________________________________________________________

	SetAboutWindowFlag

*/

void
CControlPanelApp::SetAboutWindowFlag(Boolean theFlag)
{
	IsAboutWndExists = theFlag;
		
}

// ---------------------------------------------------------------------------
//	¥ FindCommandStatus								[public, virtual]
// ---------------------------------------------------------------------------
//	Determine the status of a Command for the purposes of menu updating.

void
CControlPanelApp::FindCommandStatus(
	CommandT	inCommand,
	Boolean&	outEnabled,
	Boolean&	outUsesMark,
	UInt16&		outMark,
	Str255		outName)
{
	switch (inCommand) {

		case cmd_New: {
			outEnabled = true;
			break;
		}
			
		default: {
			LApplication::FindCommandStatus(inCommand, outEnabled,
											outUsesMark, outMark, outName);
			break;
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ RegisterClasses								[protected]
// ---------------------------------------------------------------------------
//	To reduce clutter within the Application object's constructor, class
//	registrations appear here in this seperate function for ease of use.

void
CControlPanelApp::RegisterClasses()
{
		// Register core PowerPlant classes.
	RegisterClass_(LWindow);
	RegisterClass_(LProgressBar);
	RegisterClass_(LView);
	RegisterClass_(LCaption);
	RegisterClass_(LIconPane);
	RegisterClass_(LPopupButton);
	RegisterClass_(LDialogBox);
	RegisterClass_(LStdButton);
	
	RegisterClass_(CControlPanel);
	RegisterClass_(LScroller);
	RegisterClass_(LPicture);
	RegisterClass_(LBorderAttachment);
	RegisterClass_(LPaintAttachment);
	
	RegisterClass_(C2DPerfView);
	RegisterClass_(C3DPerfView);
	RegisterClass_(CDisplayView);
	RegisterClass_(CProfileView);
	RegisterClass_(CGameView);
	RegisterClass_(CDebugView);
	
	RegisterClass_(COpenGLView);
	RegisterClass_(CGlideView);
	RegisterClass_(CRaveView);
	
	
	RegisterClass_(CSettingsWindow);
	RegisterClass_(CViewColumn);
	RegisterClass_(CViewCell);
	RegisterClass_(CKeyComboField);
	RegisterClass_(CAboutWindow);
	RegisterClass_(CUpdateDrv);
	RegisterClass_(CPopupButton);
	RegisterClass_(LTextEditView);
	
	RegisterClass_(CFlashRomWindow);
	RegisterClass_(CUpdateRom);
	RegisterClass_(CEndUpdateRom);
	
	RegisterClass_(LTextButton);
	
		// Register the Appearance Manager/GA classes. You may want
		// to remove this use of UControlRegistry and instead perform
		// a "manual" registration of the classes. This cuts down on
		// extra code being linked in and streamlines your app and
		// project. However, use UControlRegistry as a reference/index
		// for your work, and ensure to check UControlRegistry against
		// your registrations each PowerPlant release in case
		// any mappings might have changed.
		
		
	// Add a yield attachment.
	AddAttachment(new LYieldAttachment(kAppYieldQuantum));
	
	UControlRegistry::RegisterClasses();
}



/*______________________________________________________________________________

	CheckIfVoodoo3

*/

int
CControlPanelApp::CheckVoodooVersion(
	CPCIHardware *		inBoard)
{
	Str255			installedModel;
	UInt16			theStringStart;
	UInt16			theStringLength;
	
	
	inBoard->GetModelName( installedModel );

	//theStringStart = strlen( installedModel );
	theStringStart = installedModel[0];
	while ( --theStringStart >= 0 && installedModel[theStringStart] != kVoodooName[0] )
		;
	
	if ( theStringStart >= 0 )
	{
		theStringLength = strlen( kVoodooName );
		if ( !strncmp( kVoodooName, (char*) &installedModel[theStringStart], theStringLength ) )
			if (installedModel[theStringLength + theStringStart] == '3')
				return 3;
			else if (installedModel[theStringLength + theStringStart] == '4')
				return 4;
			else if (installedModel[theStringLength + theStringStart] == '5')
				return 5;
			else if (installedModel[theStringLength + theStringStart] == '6')
				return 6;
	}
	return 0;
}
