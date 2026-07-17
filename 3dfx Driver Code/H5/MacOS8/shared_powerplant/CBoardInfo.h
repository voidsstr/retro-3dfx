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

#ifndef __CBoardInfo__
#define __CBoardInfo__

#include <LWindow.h>
#include <LListener.h>

class	CPCIHardware;

class	CBoardInfo :	public LWindow,
						public LListener {
public:
	enum { class_ID = 'BINF' };

	//_____ Initialization

							CBoardInfo();
							CBoardInfo(	
									LStream *			inStream);
	virtual					~CBoardInfo();
									
private:
	virtual void			FinishCreateSelf();

	//_____ Accessors
public:
	virtual void			SetPCIHardware(
									CPCIHardware *		inBoard);

	//_____ 

	virtual void			OffsetWindow(
									short				inIndex);
private:

	virtual void			ListenToMessage(
									MessageT			inMessage,
									void*				ioParam);
	
	CPCIHardware *			mBoard;	
};

#endif /* __CBoardInfo__ */