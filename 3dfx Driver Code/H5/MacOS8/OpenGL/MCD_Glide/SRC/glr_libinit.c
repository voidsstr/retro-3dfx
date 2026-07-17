/*________________________________________________________________________________________
** 
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
**________________________________________________________________________________________
**
**  Description: code fragment initialization and termination code.
**
** 
**
*/


#include "glr.h"
#include <CodeFragments.h>

static FSSpec *gFileSpec = NULL;

extern OSErr        __initialize(
                            const CFragInitBlock        *inInitBlock);

extern void         __terminate(void);

OSErr               glrInitializeLibrary(
                            const CFragInitBlock        *inInitBlock);

void                glrTerminateLibrary(void);


/*
________________________________________________________________________________________

      glrInitializeLibrary
________________________________________________________________________________________

*/

OSErr
glrInitializeLibrary(
    const CFragInitBlock        *inInitBlock)
{
    OSErr                       theSuccess;

    theSuccess = __initialize( inInitBlock );

    if( theSuccess == noErr )
    {
        if( inInitBlock->fragLocator.where == kDataForkCFragLocator )
            gFileSpec = inInitBlock->fragLocator.u.onDisk.fileSpec;

        DEBUG_INIT( gFileSpec->name );

    }

    return theSuccess;
}


/*
________________________________________________________________________________________

      glrTerminateLibrary
________________________________________________________________________________________

*/

void
glrTerminateLibrary(void)
{
#if 0
if (0)
	{
	  FxU32 i;
	  extern FxU32 nextEntry;
extern FxU32 _c0[10000];
extern FxU32 _c1[10000];
extern FxU32 _c2[10000];
extern FxU32 _c3[10000];
	  for(i = 0; i < nextEntry; i++)
	    glr_debug_printf("c[%10d]: %10d %10d %10d %10d\n",i,
	      _c0[i],_c1[i],_c2[i],_c3[i]);
	}	      
#endif
    DEBUG_TERMINATE();

    __terminate();
}

