#ifndef H3CINITDD_MAC_H
#define H3CINITDD_MAC_H

/* -*-c++-*- */
/* $Header: h3cinitdd_mac.h, 4, 10/11/00 8:38:04 PM, Brent$ */
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 4$
** $Date: 10/11/00 8:38:04 PM$
*/
#include <stddef.h>

#if defined(__MRC__)
#define inline static
#endif

#include <3Dfx.h>

inline FxU8 FX_CALL
macIOReadByte(FxU32 ioBaseAddr, FxU16 ioAddr)
{
	__sync();
	__eieio();
	return *(volatile FxU8 *)(ioBaseAddr + ioAddr);
}

inline void FX_CALL 
macIOWriteByte(FxU32 ioBaseAddr, FxU16 ioAddr, FxU8 writeVal)
{
	__eieio();
	*(volatile FxU8 *)(ioBaseAddr + ioAddr) = writeVal;
	__sync();
}

inline void FX_CALL 
macIOWriteWord(FxU32 ioBaseAddr, FxU16 ioAddr, FxU16 writeVal)
{
	__eieio();
	*(volatile FxU8 *)(ioBaseAddr + ioAddr) = (FxU8)(writeVal & 0xff);
	__sync();
	__eieio();
	*(volatile FxU8 *)(ioBaseAddr + ioAddr + 1) = (FxU8) ((writeVal >> 8) & 0xff);
	__sync();
}

inline FxU32 FX_CALL 
macIOReadLong(FxU32 ioBaseAddr, FxU16 ioAddr)
{
	__sync();
	__eieio();
	return __lwbrx((void *)(ioBaseAddr + ioAddr), 0);
}

inline void FX_CALL 
macIOWriteLong(FxU32 ioBaseAddr, FxU16 ioAddr, FxU32 writeVal)
{
	__sync();
	__eieio();
	__stwbrx(writeVal, (void *)(ioBaseAddr + ioAddr), 0);
	__sync();
}

#define _inp(__pioAddr)             macIOReadByte(regBase, __pioAddr)
#define _outp(__pioAddr, __value)   macIOWriteByte(regBase, __pioAddr, __value)

#define _inpw(__pioAddr)            macIOReadWord(regBase, __pioAddr)
#define _outpw(__pioAddr, __value)  macIOWriteWord(regBase, __pioAddr, __value)

#define _inpd(__pioAddr)            macIOReadLong(regBase, __pioAddr)
#define _outpd(__pioAddr, __value)  macIOWriteLong(regBase, __pioAddr, __value)

#define SSTIOADDR(regName)          offsetof(SstIORegs, regName)
#define ISET32(addr, value)         _outpd(SSTIOADDR(addr), value)
#define IGET32(addr)                _inpd(SSTIOADDR(addr))

#define ISET8PHYS(a,b) \
	do { \
		GDBG_INFO(120, "OUT8:  Port 0x%x Value 0x%x\n", (a), (b)); \
		_outp((a), (FxU8)(b)); \
	} while(0)

#define ISET16PHYS(a,b) \
	do { \
		GDBG_INFO(120, "OUT16:  Port 0x%x Value 0x%x\n", (a), b); \
		_outpw((a), (FxU16) (b)); \
	} while (0)

#define IGET8PHYS(a) 	_inp(a)
#define IGET16PHYS(a) _inpw(a)

#define CHECKFORROOM \
	do { \
		/* Nothing */ \
	} while ( !(IGET32(status) & (FxU16)(0x3ff)) )


#define MESSAGE GDBG_PRINTF

#endif

#ifdef MACOS_GDX  

#endif
