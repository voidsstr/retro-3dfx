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

resource 'inpk' ( kAllVoodoo4Packages ) {
	format0 {
		showsOnCustom,
		removable,
		forceRestart,
		kAllVoodoo4Packages,
		0,
		
		"Complete Voodoo for Macintosh drivers installation",		
								
		{		/* Package parts list	*/
		/*	RsrcType,	RsrcID			*/											
			'inpk',		kEssentialPackage;
			'inpk',		kGlidePackage;
			'inpk',		kOpenGLPackage;
			'inpk',		kRavePackage;
			'inpk',		kCPanelPackage,
		},
	}
};

resource 'inpc' ( kAllVoodoo4Packages ) {
	format1 {
		kINSTALLER_DATE,					/* date ( from DrvVersion.h ) */
		kINSTALLER_VERSION,					/* version ( from DrvVersion.h ) */
		0,							/* RAM required for package ( in Kb ) */

		9128,							/* icon rsrc ID ( 'ICN#', 'icl4', 'icl8' ) */
								
		kAllVoodoo4Packages,			/* 'TEXT' resource ID of item  
										   containing package description */
		
	}
};


data 'TEXT' ( kAllVoodoo4Packages ) {
	"Complete installation of the all the Voodoo for Macintosh Drivers"
};
