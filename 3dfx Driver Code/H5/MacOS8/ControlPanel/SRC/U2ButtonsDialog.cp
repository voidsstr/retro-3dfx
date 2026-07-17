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


#include "U2ButtonsDialog.h"
#include "UTextLocalization.h"
#include "U2ButtonsDialogPPob.h"

#include <LDialogBox.h>
#include <LCaption.h>
#include <LStdControl.h>
#include <LPushButton.h>
#include <UModalDialogs.h>


/*______________________________________________________________________________

Show2ButtonsDialog

*/

short
Show2ButtonsDialog(
	LCommander *		inCommander,
	TextMessagesT		inMessageID,
	ButtonTextT			inDefaultButton,
	ButtonTextT			inAlternateButton)
{
	Boolean				theDone = false;
	Boolean				theOK = false;
	
	StDialogHandler theDialogHandler( WIND_CautionDialog, inCommander );
	LDialogBox * theDialog = (LDialogBox*) theDialogHandler.GetDialog();
	
	if ( theDialog )
	{
		LStdButton * theDefaultButton;
		LStdButton * theAltButton;
		LCaption * theCaption;

		theDefaultButton = (LStdButton*) theDialog->FindPaneByID(pane_CD_DefaultButton);
		if (theDefaultButton != nil)
		{
			switch( inDefaultButton ) {
				case kOK:
					theDefaultButton->SetDescriptor( GetLocalText(kOKText) );
					break;
				
				case kQuit:
					theDefaultButton->SetDescriptor( GetLocalText(kQuitText) );
					break;
				
				default:
					theDone = true;
					break;
			}
		}
		
		theAltButton = (LStdButton*) theDialog->FindPaneByID(pane_CD_AlternateButton);
		if (theAltButton != nil)
		{
			switch( inAlternateButton ) {
				case kCancel:
					theAltButton->SetDescriptor( GetLocalText(kCancelText) );
					theAltButton->Show();
					theAltButton->Enable();
					break;
				
				case kRestart:
					theAltButton->SetDescriptor( GetLocalText(kRestartText) );
					theAltButton->Show();
					theAltButton->Enable();
					break;
					
				case kNone:
					theAltButton->Hide();
					break;
				
				default:
					theDone = true;
					break;
			}
		}
				
		theCaption = (LCaption*) theDialog->FindPaneByID(pane_CD_MessageText);
		theCaption->SetDescriptor( GetLocalText(inMessageID) );
		// resize the window in function of the amount of text
		
		theDialog->Show();

		while ( !theDone ) 
		{
			MessageT	theMessage = theDialogHandler.DoDialog ();
			
			switch ( theMessage ) {
				case pane_CD_DefaultButton:
					theDone = true;
					theOK = true;
					break;
				
				case pane_CD_AlternateButton:
					theDone = true;
					break;
			}
		}
	}
	
	return theOK;
}
