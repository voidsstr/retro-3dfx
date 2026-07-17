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



resource 'infr' (766) {
	format0 {{
		/* execute first true rule, if none are true then return false */
		pickFirst, { 1000, 1100 },
	}}
};


resource 'inrl' (1000) {
	format0 {{
		/* this returns false unless CPU is Power Macintosh */
		CheckGestalt{ gestaltSystemType, { gestaltPPCsysa }},
		
		/* if Power Macintosh CPU add that package */
		AddCustomItems{{ kEssentialPackage, kSeperatorLine,
							kOpenGLPackage, kSeperatorLine,
							kGlidePackage, kSeperatorLine,
							kRavePackage }},
	}}
};


resource 'inrl' (1100) {
	format0 {{
		/* if not a PowerMac, then we cannot install */
		AddUserDescription{ "This Macintosh cannot use this software" },
	}}
};





