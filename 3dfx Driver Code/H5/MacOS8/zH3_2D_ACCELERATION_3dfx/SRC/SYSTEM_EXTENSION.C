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
** $Header: SYSTEM_EXTENSION.C, 2, 10/11/00 8:38:44 PM, Brent$
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 2     7/02/99 3:32p Kcd
** Added icon display at startup.
**
*/

#include <Memory.h>
#include <Resources.h>
#include <CodeFragments.h>
#include <Types.h>

#include "GraphicsAcceleration.h"
#include "ShowInitIcon.h"

OSErr    InitializeInterface();


OSErr
InitializeInterface()
{

  THz                theCurrentZone;
  OSErr              theErr = -1;
  Str255             theCFMErr;
  Handle             theAccelerationCfrg = 0;
  Str63              theFragmentName = "\pGraphicsAcceleration";
  CFragConnectionID  theFragmentConnID;
  GAMainProcPtr      theFragmentMain;

/*
	DebugStr("\p InitializeInterface");
*/

  theCurrentZone = GetZone ();
  SetZone (SystemZone ());
		
  theAccelerationCfrg = GetNamedResource ('accl', (ConstStr255Param) "\pGraphicsAcceleration");
  if (theAccelerationCfrg == NULL) {
    SetZone (theCurrentZone);
    return (-1);
  }
		
  HLock (theAccelerationCfrg);
  DetachResource (theAccelerationCfrg);

  theErr = GetMemFragment (	*theAccelerationCfrg, GetHandleSize (theAccelerationCfrg),
                            theFragmentName, kPrivateCFragCopy, &theFragmentConnID,
                            (Ptr *) &theFragmentMain, theCFMErr);
									
  if ((theErr == noErr) && (theFragmentMain != 0) ) {
    theErr = (*theFragmentMain) (kInstallGraphicsAcceleration, nil);
	if(theErr == 0) {
	  ShowInitIcon(128,true);
	} else {
	  ShowInitIcon(129,true);
	}
		
  } else {
	ShowInitIcon(129,true);
    DisposeHandle (theAccelerationCfrg);
  }

  SetZone (theCurrentZone);
      
  return (theErr);
}
