/*
** Copyright (c) 1999 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
*/

#ifndef MAC_SFC_H
#define MAC_SFC_H 1

#include <3dfx.h>
#include <gdebug.h>
#include <glide.h>
#include "gsfc.h"

#include <h3cinit.h>
#include "setmode.h"

#include <QuickDraw.h>
#include <Displays.h>
#include <GraphicsPriv.h>
#include <GraphicsPrivHwc.h>

#if 0
typedef struct GDXSurfaceDesc_s
{
	hrmBoard_t *owner;			/* Which display device owns this memory */
	FxU32		surface;		/* Linear memory pointer */
	FxU32		width;			/* Width of surface in pixels */
	FxU32		height;			/* Height of surface in pixels */
	FxU32		pitch;			/* Bytes from one row of pixels to the next */
	FxU32		pixelFormat;	/* Bytes per pixel */
	FxU32		flags;			/* Misc flags */
	void        (*notifyProc)(void *userData, FxU32 code);
	void        *userData;
	CGrafPtr	offscreenPort;
	GDHandle	offscreenDevice;
} GDXSurfaceDesc_t;
#endif

typedef struct GDXSurface_s GDXSurface_t;

extern void *gdxSurfaceAlloc(hwcBoardInfo *bInfo, hrmBoard_t *board, GrSurfaceDesc_t *desc);
extern void gdxSurfaceFree(void *sfc);
extern void gdxSurfaceGetDesc(void *sfc, GrSurfaceDesc_t *desc);

#endif	/* MAC_SFC_H not defined */
