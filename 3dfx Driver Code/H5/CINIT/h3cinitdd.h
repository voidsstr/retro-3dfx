/* -*-c++-*- */
/* $Header: h3cinitdd.h, 3, 10/11/00 8:08:14 PM, Brent$ */
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
** $Revision: 3$
** $Date: 10/11/00 8:08:14 PM$
*/
#include <conio.h>
#include <stddef.h>

#if defined(__WATCOMC__)
#define _inp inp
#define _outp outp

#define _inpw inpw
#define _outpw outpw

#define _inpd inpd
#define _outpd outpd

#endif


#define SSTIOADDR(regName)      ((FxU16)offsetof(SstIORegs, regName))

#define ISET32(addr, value)     _outpd((FxU16) ((FxU16) regBase + (FxU16) (SSTIOADDR(addr))), value)
#define IGET32(addr)            _inpd((FxU16) ((FxU16) regBase + (FxU16) (SSTIOADDR(addr))))

#define ISET8PHYS(a,b) {\
FxU16 port = (FxU16) (regBase) + (FxU16) (a);\
GDBG_INFO(120, "OUT8:  Port 0x%x Value 0x%x\n", port, b);\
_outp(port, (FxU8) (b));}

#define ISET16PHYS(a,b) {\
FxU16 port = (FxU16)(regBase) + (FxU16)(a);\
GDBG_INFO(120, "OUT16:  Port 0x%x Value 0x%x\n", port, b);\
_outpw(port, (FxU16) (b));}

#define IGET8PHYS(a) _inp((FxU16) ((FxU16) (regBase) + (FxU16) (a)))
#define IGET16PHYS(a) _inpw((FxU16) ((FxU16) (regBase) + (FxU16)(a)))

#define CHECKFORROOM while (! (_inp((FxU16) regBase) & (FxU16)(0x3f)))
#define MESSAGE GDBG_PRINTF

#ifdef OLD


#define ISET32(a,b)\
GDBG_INFO(120, "SET32: Register 0x%x Value 0x%x\n", (FxU32) (&((SstIORegs *)regBase)->a) - (FxU32) regBase, b); \
((FxU32) (((SstIORegs *) regBase)->a)) = (FxU32) b

#define IGET32(a) ((FxU32) (((SstIORegs *) regBase)->a))





#endif /* #ifdef OLD */
