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
** File name:   gdx_runtime.c
**
** Description: miscelaneous hardware registers access methods.
**
** $Header: GDX_Runtime.c, 4, 10/11/00 8:33:56 PM, Brent$
**
** $History: gdx_runtime.c $
** 
** *****************  Version 3  *****************
** User: Kcd          Date: 8/23/99    Time: 2:21p
** Updated in $/devel/h3/MacOS8/GDX/src
** Added proper swizzling for 16-bit modes.
** 
** *****************  Version 2  *****************
** User: Kcd          Date: 7/02/99    Time: 4:31p
** Updated in $/devel/h3/MacOS8/GDX/src
** Added some debugging info.
** 
** *****************  Version 1  *****************
** User: Kcd          Date: 6/03/99    Time: 6:45p
** Created in $/devel/h3/MacOS8/GDX/src
** MacOS 8 2D Display Driver
** 
**
*/

#include <DCon.h>
#include "gdx_runtime.h"

void (*__hackSwizzle)(volatile FxU32 *d, FxU32 s);

/* Full byte swap */
void  __swizzleWrite32_8(volatile FxU32 *d, FxU32 s)
{
#if 0
	if(__hackSwizzle != __swizzleWrite32_8) {
		dprintf("Called __swizzleWrite32_8 incorrectly!\n");
		__hackSwizzle(d, s);
		return;
	}
#endif	
#if PCI_COPYBACK
	if(((FxU32)d & 31) == 0) {
		__dcbf((void *)d,-4);
		__dcbz((void *)d,0);
	}
#endif	
	__stwbrx(s,(void *)d, 0);
}

/* Swap words */
void  __swizzleWrite32_16(volatile FxU32 *d, FxU32 s)
{
	FxU32 temp = (s >> 16) | (s << 16);
#if 0	
	if(__hackSwizzle != __swizzleWrite32_16) {
		dprintf("Called __swizzleWrite32_16 incorrectly!\n");
		__hackSwizzle(d, s);
		return;
	}
#endif	
#if PCI_COPYBACK
	if(((FxU32)d & 31) == 0) {
		__dcbf((void *)d,-4);
		__dcbz((void *)d,0);
	}
#endif	
	*d = temp;
}

/* Swap bytes within words */
void  __swizzleWrite16_16(volatile FxU32 *d, FxU32 s)
{
	FxU32 temp = (s >> 16) | (s << 16);
#if 0	
	if(__hackSwizzle != __swizzleWrite32_16) {
		dprintf("Called __swizzleWrite32_16 incorrectly!\n");
		__hackSwizzle(d, s);
		return;
	}
#endif	
#if PCI_COPYBACK
	if(((FxU32)d & 31) == 0) {
		__dcbf((void *)d,-4);
		__dcbz((void *)d,0);
	}
#endif	
	__stwbrx(temp,(void *)d, 0);
}

/* Pass through */
void  __swizzleWrite32_32(volatile FxU32 *d, FxU32 s)
{
#if 0
	if(__hackSwizzle != __swizzleWrite32_32) {
		dprintf("Called __swizzleWrite32_32 incorrectly!\n");
		__hackSwizzle(d, s);
		return;
	}
#endif
#if PCI_COPYBACK
	if(((FxU32)d & 31) == 0) {
		__dcbf((void *)d,-4);
		__dcbz((void *)d,0);
	}
#endif	
	*d = s;
}

