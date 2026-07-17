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


resource 'inpk' ( kSeperatorLine ) {
	format0 {
		showsOnCustom,			// if a subpackage, show in Custom Install
		
		notRemovable,			// don't allow selection of seperator line 
								// for removal
								
		dontForceRestart,		// don't make user reboot after installation

		0,						// no comments for a seperator line

		0,						// no size for a seperator line

		"-",					// display a dashed line in Custom Install 

		{	
								// parts list is empty for seperator line
		},
	}
};
