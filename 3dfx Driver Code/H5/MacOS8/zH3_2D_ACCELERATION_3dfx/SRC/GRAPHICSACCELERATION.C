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
** $Header: GRAPHICSACCELERATION.C, 4, 10/11/00 8:38:37 PM, Brent$
** $Log: 
**  4    3dfx      1.2.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  3    MacOS Dev Tree1.2         04/02/00 Stephane Huaulme more region parsing
**       fixing...
**  2    MacOS Dev Tree1.1         03/25/00 Stephane Huaulme changed region parsing
**       (using same as CP)
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 2     7/02/99 3:33p Kcd
** New headers & HRM integration.
**
*/

#define ENABLE_BITBLIT 1
#define ENABLE_PATBLIT 1
#define ENABLE_RGNBLIT 1
#define ENABLE_PATRGNBLIT 1
#define ENABLE_SCALEBLIT 1
#define ENABLE_LINEBLIT 1
#define ENABLE_SLABBLIT 1


#include <Types.h>
#include <Memory.h>
#include <Errors.h>
#include <Quickdraw.h>
#include <NQDAcceleration.h>
#include <GraphicsAcceleration.h>

#include "h3defs.h"
#include "h3gdefs.h"
#include "minihwc.h"
#include "hwcio.h"
#include <GraphicsPrivHwc.h>
#include <CodeFragments.h>
#include "H3Acceleration.h"

/* some internal prototypes */
extern pascal OSErr  __initialize(const CFragInitBlock *inInitBlock);
extern pascal OSErr  __terminate();
static OSErr  InstallAcceleration ();
OSErr  UninstallAcceleration ();

/* some globals */
Boolean  accelerationInstalled = false;
void (*BlitMoveRect)(NQDDrawVars *drawVars, Rect *clipRect);

pascal OSErr  InitializeGraphicsAcceleration(const CFragInitBlock *inInitBlock)
{
  OSErr  theErr;

	theErr =   __initialize( inInitBlock );
	
	if(theErr != 0)
		return theErr;
		
	dprintf("InitializeGraphicsAcceleration\n");

/* initialize hardware */
  if ((theErr = InitializeAccelerationHardware ()) != noErr)
    return (theErr);

  return (noErr);
}

OSErr  GraphicsAcceleration(OSType  selector, Ptr  params)
{
  OSErr  theErr = noErr;

	dprintf("GraphicsAcceleration: %08lx %08lx\n",selector,params);

  switch (selector)
  {
    case kInstallGraphicsAcceleration :
      theErr = InstallAcceleration ();
      break;

    case kUninstallGraphicsAcceleration :
      theErr = UninstallAcceleration ();
      break;

    case kIsGraphicsAccelerationInstalled :
      *((long *) params) = accelerationInstalled;
      theErr = noErr;
      break;

    default :
      theErr = noErr;
  }

  return (theErr);
}

pascal OSErr  TerminateGraphicsAcceleration()
{
	dprintf("TerminateGraphicsAcceleration\n");

/* uninstall acceleration */
  UninstallAcceleration ();

	__terminate();
	
  return (noErr);
}

static OSErr  InstallAcceleration()
{
  NQDGetBlitProcParamBlock  paramBlock;

  if (!accelerationInstalled)
  { /* install acceleration hooks */
#if ENABLE_BITBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedRgnBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kBitBlitIndex;
    NQDMisc (kAddBlitProcPtr, (Int32 *) &paramBlock);
#endif
#if ENABLE_PATBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedPatBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kPatBlitIndex;
    NQDMisc (kAddBlitProcPtr, (Int32 *) &paramBlock);
#endif
#if ENABLE_RGNBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedRgnBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kRgnBlitIndex;
    NQDMisc (kAddBlitProcPtr, (Int32 *) &paramBlock);
#endif
#if ENABLE_PATRGNBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedPatRgnBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kPatRgnBlitIndex;
    NQDMisc (kAddBlitProcPtr, (Int32 *) &paramBlock);
#endif
#if ENABLE_SCALEBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedScaleBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kScaleBlitIndex;
    NQDMisc (kAddBlitProcPtr, (Int32 *) &paramBlock);
#endif
#if ENABLE_LINEBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedLineBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kLineBlitIndex;
    NQDMisc (kAddBlitProcPtr, (Int32 *) &paramBlock);
#endif
#if ENABLE_SLABBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedSlabBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kSlabBlitIndex;
    NQDMisc (kAddBlitProcPtr, (Int32 *) &paramBlock);
#endif
    accelerationInstalled = true;
  }

  return (noErr);
}

OSErr  UninstallAcceleration()
{
  NQDGetBlitProcParamBlock  paramBlock;

  if (accelerationInstalled)
  { /* remove acceleration hooks */
#if ENABLE_BITBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedRgnBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kBitBlitIndex;
    NQDMisc (kRemoveBlitProcPtr, (Int32 *) &paramBlock);
#endif
#if ENABLE_PATBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedPatBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kPatBlitIndex;
    NQDMisc (kRemoveBlitProcPtr, (Int32 *) &paramBlock);
#endif
#if ENABLE_RGNBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedRgnBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kRgnBlitIndex;
    NQDMisc (kRemoveBlitProcPtr, (Int32 *) &paramBlock);
#endif
#if ENABLE_PATRGNBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedPatRgnBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kPatRgnBlitIndex;
    NQDMisc (kRemoveBlitProcPtr, (Int32 *) &paramBlock);
#endif
#if ENABLE_SCALEBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedScaleBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kScaleBlitIndex;
    NQDMisc (kRemoveBlitProcPtr, (Int32 *) &paramBlock);
#endif
#if ENABLE_LINEBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedLineBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kLineBlitIndex;
    NQDMisc (kRemoveBlitProcPtr, (Int32 *) &paramBlock);
#endif
#if ENABLE_SLABBLIT
    paramBlock.getBlitProc = (NQDGetBlitProcPtr) GetAcceleratedSlabBlitProc;
    paramBlock.finishProc = H3BlitFinish;
    paramBlock.index = kSlabBlitIndex;
    NQDMisc (kRemoveBlitProcPtr, (Int32 *) &paramBlock);
#endif
    accelerationInstalled = false;
  }

  return (noErr);
}

#if GENERATINGCFM
ProcInfoType __procinfo =  kPascalStackBased;
#endif

