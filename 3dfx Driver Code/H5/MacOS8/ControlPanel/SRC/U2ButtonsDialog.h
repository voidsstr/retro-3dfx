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

#ifndef __2_BUTTONS_DIALOG__
#define __2_BUTTONS_DIALOG__

#include "UTextLocalization.h"
#include <LCommander.h>

typedef enum {
	kQuit,
	kOK,
	kCancel,
	kRestart,
	kNone
} ButtonTextT;

#ifdef __cplusplus
extern "C" {
#endif

	short					Show2ButtonsDialog(
									LCommander *		inCommander,
									TextMessagesT		inMessageID,
									ButtonTextT			inDefaultButton,
									ButtonTextT			inAlternateButton);

#ifdef __cplusplus
}
#endif

#endif /* __2_BUTTONS_DIALOG__ */
