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

#ifndef __CControlPanel__
#define __CControlPanel__

#include <LWindow.h>
#include <LStream.h>
#include <LListener.h>
#include <LArray.h>
#include <TArray.h>
#include <LMultiPanelView.h>
#include <LString.h>
#include <LPopupButton.h>

#include "CPCIHardware.h"
#include "ControlPanelPPob.h"




class	CPCIHardware;

class	CControlPanel :	public LWindow, public LListener
										 {
public:
	enum { class_ID = 'BINF' };

	//_____ Initialization

							CControlPanel();
							CControlPanel(	
									LStream *			inStream);
	virtual					~CControlPanel();


public:
	//_____ Accessors
	void					SetPCIHardware(
									CPCIHardware *		inBoard);
	
	CPCIHardware *			GetPCIHardware();
	
	void					InitPCIHardware();

									
	void					ClickInGoAway(
									const EventRecord& 	inMacEvent);
									
	void					FinishCreateSelf();

	void					FinishCreateSelf_Games();

///	void					ListenToControls();
	
	void					ListenToMessage(MessageT inMessage, void *ioParam);
	
//	void					IterateSubPanes(
//									LPane *				thePane);
	
	virtual void			OffsetWindow(
									short				inIndex);
									
	virtual void			CenterWindowOnDisplay();
	
	void					ClickInDrag(
									const EventRecord	&inMacEvent);
									
	void					DoCheckROM();
	
	void					SaveCurrentDisplaySettings();
	
	void					SetVoodooVersion(
									int					inValue);
	
private:

	CPCIHardware *			mBoard;	
	LMultiPanelView*		mainMPV;
	int						mVVersion;


};

#endif /* __CControlPanel__ */