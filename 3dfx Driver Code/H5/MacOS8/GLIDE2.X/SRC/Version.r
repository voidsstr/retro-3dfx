#include <MacTypes.r>

#define __VERSION_REZ__
#define macintosh

#include "MacGlide2_release.h"
#include "rcver.h"

#include <DrvVersion.h>

Resource 'vers' (1, "") 
{
	MANVERSION,
	((MANREVISION / 10) << 4) | (MANREVISION % 10),
#if DEBUG
	development,
#else /* !DEBUG */
	release,
#endif /* !DEBUG */
	0,
	verUS,
	VERSIONSTR,
	HWSTR" "VERSIONSTR" for Mac OS"
};


resource 'vers' (2) {
	MANVERSION,
	((MANREVISION / 10) << 4) | (MANREVISION % 10),
#if DEBUG
	development,
#else /* !DEBUG */
	release,
#endif /* !DEBUG */
	0x0,
	0,
	VERSIONSTR,
	kGLIB2X_DESCRIPTION
};
