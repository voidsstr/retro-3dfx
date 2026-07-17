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

#include "Types.r"
#include "MacTypes.r"

#include "International_Lang.h"


resource 'vers' (2) {
	kMACTOOLS_MAJOR_REV,
	kMACTOOLS_MINOR_REV,
	kMACTOOLS_STAGE_REV,
	kMACTOOLS_LEVEL_REV,
	kMACTOOLS_REGION_CODE,
	kMACTOOLS_VERSION,
	kMACTOOLS_DESCRIPTION
};

resource 'vers' (1) {
	kMACTOOLS_MAJOR_REV,
	kMACTOOLS_MINOR_REV,
	kMACTOOLS_STAGE_REV,
	kMACTOOLS_LEVEL_REV,
	kMACTOOLS_REGION_CODE,
	kMACTOOLS_VERSION,
	kMACTOOLS_VER_FULL
};

resource 'FREF' (128, purgeable) {
	'APPC',
	0,
	""
};
resource 'BNDL' (128, purgeable) {
	'3Dfx',
	0,
	{
		'FREF',
		{
			0, 128
		},
		'ICN#',
		{
			0, 128
		}
	}
};

data '3Dfx' (0, "Owner resource") {
	kGLOBAL_OWNER
};



resource 'ALRT' (204, "Low Memory Warning", locked, preload) {
	{40, 20, 142, 380},
	204,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, silent
	},
	alertPositionMainScreen
};

resource 'ALRT' (128, "About Box", purgeable) {
	{40, 20, 120, 410},
	128,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (204, "Low Memory Warning", locked, preload) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{69, 289, 89, 347},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{10, 78, 56, 348},
		StaticText {
			disabled,
			"Memory is getting full. Please try to al"
			"leviate the problem by closing some docu"
			"ments. "
		}
	}
};

resource 'DITL' (128, "About Box", purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{47, 319, 67, 377},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{13, 23, 33, 390},
		StaticText {
			disabled,
			"3dfx Control Panel"
		},
		/* [3] */
		{33, 23, 53, 237},
		StaticText {
			disabled,
			"©1995-1998 Metrowerks Inc."
		}
	}
};



