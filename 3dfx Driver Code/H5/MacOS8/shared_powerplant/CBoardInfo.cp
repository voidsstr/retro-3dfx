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

#include "CBoardInfo.h"
#include "CPCIHardware.h"
#include "COverclockerPPob.h"
#include <LCaption.h>
#include <LStdControl.h>
#include <PP_Messages.h>


/*______________________________________________________________________________

Constructor

*/

CBoardInfo::CBoardInfo()
{
}


/*______________________________________________________________________________

Constructor (using a Stream, that's the one that will be used)

*/

CBoardInfo::CBoardInfo(
	LStream *			inStream)
	: LWindow ( inStream )
{
}


/*______________________________________________________________________________

Destructor

*/

CBoardInfo::~CBoardInfo()
{
}


/*______________________________________________________________________________

FinishCreateSelf

Hook up the buttons.

*/

void
CBoardInfo::FinishCreateSelf()
{
	LStdButton * theButtonP = (LStdButton*) FindPaneByID(pane_BI_Quit);
	theButtonP->AddListener( this );
}


/*______________________________________________________________________________

SetPCIHardware

*/

void
CBoardInfo::SetPCIHardware(
	CPCIHardware *		inBoard)
{
	if ( inBoard != 0 ) {
		Str255 theText;
		short theIndex;
		mBoard = inBoard;
	
		mBoard->GetSlotName( theText );
		LCaption * theCaption = (LCaption*) FindPaneByID( pane_BI_Slot );
		theCaption->SetDescriptor( theText );
		
		mBoard->GetModelName( theText );
		theIndex = theText[0];
		mBoard->GetSlotName( &theText[theIndex + 3] );
		theText[0] += theText[theIndex + 3] + 3;
		theText[theIndex + 1] = ' ';
		theText[theIndex + 2] = '-';
		theText[theIndex + 3] = ' ';
		SetDescriptor( theText );
		
		mBoard->GetPlatform( theText );
		theCaption = (LCaption*) FindPaneByID( pane_BI_CurrentPlatform );
		theCaption->SetDescriptor( theText );
		
		mBoard->GetVersion( theText );
		theCaption = (LCaption*) FindPaneByID( pane_BI_CurrentVersion );
		theCaption->SetDescriptor( theText );
		
	}

}


/*______________________________________________________________________________

OffsetWindow

*/

void
CBoardInfo::OffsetWindow(
	short				inIndex)
{
	if (inIndex != 0 ) {
		MoveWindowBy( 16 * inIndex, 32 * inIndex );
	}
}

/*______________________________________________________________________________

ListenToMessage

*/

void
CBoardInfo::ListenToMessage(
	MessageT	inMessage,
	void*		/*ioParam*/)
{	
	if ( inMessage == pane_BI_Quit) {
		ObeyCommand(PP_PowerPlant::cmd_Quit, nil);
	}
}
