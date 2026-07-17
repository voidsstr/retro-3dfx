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


resource 'inpk' ( k2DaccelPackage ) {
	format0 {
		showsOnCustom,
		removable,
		dontForceRestart,
		k2DaccelPackage,
		0,
		
		"Graphics Accelerator",		
								
		{		/* Package parts list	*/
		/*	RsrcType,	RsrcID			*/											
			'infa',		k2DaccelFile;
		},
	}
};


resource 'inpc' ( k2DaccelPackage ) {
	format1 {
		6281998,						/* date ( 06/28/99 ) */
		256,							/* version ( 1.0.0 ) */
		350,							/* RAM required for package ( in Kb ) */

		9128,							/* icon rsrc ID ( 'ICN#', 'icl4', 'icl8' ) */
								
		k2DaccelPackage,				/* 'TEXT' resource ID of item  
										   containing package description */

	}
};


data 'TEXT' ( k2DaccelPackage ) {
	"This component is necessary to use Voodoo3"
};

resource 'infa' (k2DaccelFile) {
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
		k2DaccelTargetFile,
		{	
			k2DaccelSourceFile,
			0,
			0
		},
		0,
		0,
		0,
		"Graphics Accelerator"
	}
};


resource 'infs' ( k2DaccelSourceFile ) {
		'INIT',
		'3DFX',
		0x0,
		noSearchForFile,
		TypeCrNeedNotMatch,
		"Voodoo3_beta:parts:3Dfx Graphics Accelerator"
};



resource 'intf' (k2DaccelTargetFile) {
	format1 {
		noSearchForFile, 
		TypeCrMustMatch,
		'INIT',
		'3DFX',
		0,
		1,
		1,
		0,
		"special-extn:3Dfx Graphics Accelerator"
	}
};
