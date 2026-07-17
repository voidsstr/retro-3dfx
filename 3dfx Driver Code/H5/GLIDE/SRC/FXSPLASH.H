/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Header: fxsplash.h, 2, 10/11/00 8:21:26 PM, Brent$
** $Log: 
**  2    3dfx      1.0.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 1     2/19/99 5:50p Peter
** type definitions for new splash screen
*/

#ifndef _FX_SPLASH_H_
#define _FX_SPLASH_H_

#include "3dfx.h"
#define FX_DLL_DEFINITION
#include "fxdll.h"
#include "glide.h"

typedef FxBool 
(FX_CALL* GrSplashInitProc)(FxU32 hWnd,
                            FxU32 screenWidth, FxU32 screenHeight,
                            FxU32 numColBuf, FxU32 numAuxBuf,
                            GrColorFormat_t colorFormat);

typedef void
(FX_CALL* GrSplashShutdownProc)(void);

typedef void 
(FX_CALL* GrSplashProc)(float x, float y,
                        float w, float h,
                        FxU32 frameNumber);

typedef const void*
(FX_CALL* GrSplashPlugProc)(FxU32* w, FxU32* h,
                            FxI32* strideInBytes,
                            GrLfbWriteMode_t* format);

#endif /* _FX_SPLASH_H_ */
