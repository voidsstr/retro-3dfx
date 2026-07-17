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
#include "v3_installer.h"


resource 'inpk' ( kRaveExtensionPackage ) {
	format0 {
		showsOnCustom,
		removable,
		dontForceRestart,
		kRaveExtensionPackage,
		0,
		
		"Rave driver",		
								
		{		/* Package parts list	*/
		/*	RsrcType,	RsrcID			*/											
			'infa',		kRaveExtensionFile;
		},
	}
};


resource 'inpc' ( kRaveExtensionPackage ) {
	format1 {
		6281998,						/* date ( 06/28/99 ) */
		100,							/* version ( 1.0.0 ) */
		350,							/* RAM required for package ( in Kb ) */

		9128,							/* icon rsrc ID ( 'ICN#', 'icl4', 'icl8' ) */
								
		kRaveExtensionPackage,			/* 'TEXT' resource ID of item  
										   containing package description */

	}
};


data 'TEXT' ( kRaveExtensionPackage ) {
	"This driver necessary to play games using the RAVE API"
};

resource 'infa' (kRaveExtensionFile) {
	format1 {
		deleteWhenRemoving,
		deleteWhenInstalling,
		copy,						
		dontIgnoreLockedFile,
		dontSetFileLocked,
		useSrcCrDateToCompare,		
		srcNeedExist,
		rsrcForkInRsrcFork,
		leaveAloneIfNewer,			
		updateExisting,				
		copyIfNewOrUpdate,
		rsrcFork,
		dataFork,

		0,
		0x0,
		kRaveExtensionTargetFile,
		{	
			kRaveExtensionSourceFile,
			0,
			0
		},
		0,
		0,
		0,
		"RAVE driver"
	}
};


resource 'infs' ( kRaveExtensionSourceFile ) {
		'shlb',
		'tnsl',
		0x0,
		noSearchForFile,
		TypeCrNeedNotMatch,
		"Voodoo3:parts:3Dfx Rave"
};



resource 'intf' (kRaveExtensionTargetFile) {
	format1 {
		noSearchForFile, 
		TypeCrMustMatch,
		'shlb',
		'tnsl',
		0,
		1,
		1,
		0,
		"special-extn:3Dfx Rave"
	}
};
