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
** $Header: GRAPHICSACCELERATION.H, 2, 10/11/00 8:38:38 PM, Brent$
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    MacOS Dev Tree1.0         01/28/00 Kenneth Dyke    
** $
** 
** 2     7/02/99 3:33p Kcd
** New headers & HRM integration.
**
*/

#ifndef GraphicsAcceleration_h
#define GraphicsAcceleration_h

/* some typedefs */

typedef OSErr  (*GAMainProcPtr) (OSType  selector, Ptr  params);

/* define some constants */

/* selector defs */
#define kInstallGraphicsAcceleration 1
#define kUninstallGraphicsAcceleration 2
#define kIsGraphicsAccelerationInstalled 3

/* simulated acceleration procedure prototypes */

OSErr  InitializeAccelerationHardware ();

#endif /* GraphicsAcceleration_h */
