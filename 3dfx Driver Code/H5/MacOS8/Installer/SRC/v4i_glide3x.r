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

resource 'inpk' ( kGlide3xPackage ) {
	format0 {
		showsOnCustom,
		removable,
		dontForceRestart,
		kGlide3xPackage,
		0,
		
		"Glide3.x driver",		
								
		{		/* Package parts list	*/
		/*	RsrcType,	RsrcID			*/											
			'infa',		kGlide3xFile;
		},
	}
};


resource 'inpc' ( kGlide3xPackage ) {
	format1 {
		kINSTALLER_DATE,					/* date ( from DrvVersion.h ) */
		310,							/* version ( 1.0.0 ) */
		0,							/* RAM required for package ( in Kb ) */

		kGlide3xIcon,							/* icon rsrc ID ( 'ICN#', 'icl4', 'icl8' ) */
								
		kGlide3xPackage,				/* 'TEXT' resource ID of item  
										   containing package description */

	}
};


data 'TEXT' ( kGlide3xPackage ) {
	"This driver is necessary to accelerate 3D applications that make use of the Glide API."
};

resource 'infa' (kGlide3xFile) {
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
		kGlide3xTargetFile,
		{	
			kGlide3xSourceFile,
			0,
			0
		},
		0,
		0,
		0,
		"Glide3.x"
	}
};


resource 'infs' ( kGlide3xSourceFile ) {
		'shlb',
		'3Dfx',
		0x0,
		noSearchForFile,
		TypeCrNeedNotMatch,
		"MacVoodoo:parts:3dfx GlideLib3.x"
};



resource 'intf' (kGlide3xTargetFile) {
	format1 {
		noSearchForFile, 
		TypeCrMustMatch,
		'shlb',
		'3Dfx',
		0,
		1,
		1,
		0,
		"special-extn:3dfx GlideLib3.x"
	}
};
