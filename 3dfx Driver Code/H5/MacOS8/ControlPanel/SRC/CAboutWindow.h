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

#ifndef __CABOUTWINDOW_H__
#define __CABOUTWINDOW_H__

#include <LView.h>
#include <LListener.h>
#include <LMultiPanelView.h>
#include <LView.h>
#include <LCaption.h>
#include <LPushButton.h>
#include "CControlPanel.h"

#include "CPCIHardware.h"


class	CAboutWindow :	public LView, public LListener
										 {
public:
	enum { class_ID = 'ABTV' };

	//_____ Initialization

							CAboutWindow();
							CAboutWindow(	
									LStream *			inStream);
	virtual					~CAboutWindow();


public:
	//_____ Accessors
										
	void					FinishCreateSelf();

	void					ListenToMessage(
								MessageT 				inMessage, 
								void *					ioParam);
	
	void					UpdateDriverNow();
	
	void					CleanThreads();

	void					SetPCIHardware(
								CPCIHardware *			inBoard);
		
	void					SetCPWindow(
								CControlPanel *		inCPanel);


private:
	
	CPCIHardware *			mBoard;
	CControlPanel *			mCPanel;

};

#endif /* __CABOUTWINDOW_H__ */