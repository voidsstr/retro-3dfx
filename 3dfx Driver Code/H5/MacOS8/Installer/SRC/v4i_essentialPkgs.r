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


#include "InstallerTypes.r"
#include "v4_installer.h"
#include "DrvVersion.h"

resource 'inpk' ( kEssentialPackage ) {
	format0 {
		showsOnCustom,
		removable,
		forceRestart,
		0,
		0,
		
		"Voodoo for Macintosh Essentials",		
								
		{		/* Package parts list */
		/*	RsrcType,	RsrcID			*/											
			//'inpk',		kHRMPackage;
			'inpk',		k2DaccelPackage;
			'inpk', 	kCPanelPackage;
		},
	}
};


resource 'inpc' ( kEssentialPackage ) {
	format1 {
		kINSTALLER_DATE,					/* date ( from DrvVersion.h ) */
		kINSTALLER_VERSION,					/* version ( from DrvVersion.h ) */
		0,							/* RAM required for package ( in Kb ) */

		9128,							/* icon rsrc ID ( 'ICN#', 'icl4', 'icl8' ) */
								
		kEssentialPackage,					/* 'TEXT' resource ID of item  
										   containing package description */
		
	}
};


data 'TEXT' ( kEssentialPackage ) {
	"Essential software components for Macintosh Voodoo"
};

