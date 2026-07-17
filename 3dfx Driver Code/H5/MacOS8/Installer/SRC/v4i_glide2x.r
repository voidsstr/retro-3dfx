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

resource 'inpk' ( kGlide2xPackage ) {
	format0 {
		showsOnCustom,
		removable,
		dontForceRestart,
		kGlide2xPackage,
		0,
		
		"Glide2.x driver",		
								
		{		/* Package parts list	*/
		/*	RsrcType,	RsrcID			*/											
			'infa',		kGlide2xFile;
		},
	}
};


resource 'inpc' ( kGlide2xPackage ) {
	format1 {
		kINSTALLER_DATE,					/* date ( from DrvVersion.h ) */
		260,							/* version ( 2.5.6 ) */
		0,							/* RAM required for package ( in Kb ) */

		kGlide2xIcon,							/* icon rsrc ID ( 'ICN#', 'icl4', 'icl8' ) */
								
		kGlide2xPackage,				/* 'TEXT' resource ID of item  
										   containing package description */

	}
};


data 'TEXT' ( kGlide2xPackage ) {
	"This driver is necessary to accelerate 3D applications that make use of the Glide API."
};

resource 'infa' (kGlide2xFile) {
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
		kGlide2xTargetFile,
		{	
			kGlide2xSourceFile,
			0,
			0
		},
		0,
		0,
		0,
		"Glide2.x"
	}
};


resource 'infs' ( kGlide2xSourceFile ) {
		'shlb',
		'3DFx',
		0x0,
		noSearchForFile,
		TypeCrNeedNotMatch,
		"MacVoodoo:parts:3Dfx GlideLib2.x"
};



resource 'intf' (kGlide2xTargetFile) {
	format1 {
		noSearchForFile, 
		TypeCrMustMatch,
		'shlb',
		'3DFx',
		0,
		1,
		1,
		0,
		"special-extn:3dfx GlideLib2.x"
	}
};
