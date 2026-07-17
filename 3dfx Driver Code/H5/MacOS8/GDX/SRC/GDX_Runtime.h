#ifndef __GDX_RUNTIME_H__
#define __GDX_RUNTIME_H__

/*
** Copyright (c) 1996-1999, 3Dfx Interactive, Inc.
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
** File name:   gdx_runtime.h
**
** Description: miscelaneous hardware registers access methods.
**
** $Header: GDX_Runtime.h, 3, 10/11/00 8:33:58 PM, Brent$
**
** $History: gdx_runtime.h $
** 
** *****************  Version 2  *****************
** User: Kcd          Date: 8/23/99    Time: 2:21p
** Updated in $/devel/h3/MacOS8/GDX/src
** Added proper swizzling for 16-bit modes.
** 
** *****************  Version 1  *****************
** User: Kcd          Date: 6/03/99    Time: 6:45p
** Created in $/devel/h3/MacOS8/GDX/src
** MacOS 8 2D Display Driver
** 
**
*/

#if defined(FX_DLL_ENABLE)
#define FX_DLL_DEFINITION
#endif

#include <3dfx.h>
#include <fxdll.h>

#define P6FENCE	__sync()

void  	__swizzleWrite32_8(volatile FxU32 *d, FxU32 s);
void  	__swizzleWrite32_16(volatile FxU32 *d, FxU32 s);
void  	__swizzleWrite16_16(volatile FxU32 *d, FxU32 s);
void  	__swizzleWrite32_32(volatile FxU32 *d, FxU32 s);



#endif /* !__GDX_RUNTIME_H__ */
