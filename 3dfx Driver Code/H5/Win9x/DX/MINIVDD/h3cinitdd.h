/* -*-c++-*- */
/* $Header: h3cinitdd.h, 2, 10/11/00 8:54:21 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** File name:   h3cinitdd.h
**
** Description: Register I/O routines for VDD.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:21 PM$
**
** $History: h3cinitdd.h $
** 
** *****************  Version 8  *****************
** User: Cwilcox      Date: 1/22/99    Time: 2:08p
** Updated in $/devel/h3/Win95/dx/minivdd
** Minor revisions to clean up compiler warnings.
** 
** *****************  Version 7  *****************
** User: Michael      Date: 1/07/99    Time: 1:27p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 6  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#ifndef __H3CINITDD_H__
#define __H3CINITDD_H__

#include <stddef.h>

#ifndef VDDONLY
// only include these routines once in the minvidd


/*----------------------------------------------------------------------
Function name:  my_outpd

Description:    Perform an "out dx, eax".

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
my_outpd(FxU16 addr, FxU32 value)
{
    __asm
    {
	mov	dx, addr;
	mov	eax, value;
	out	dx, eax;
    }
}


/*----------------------------------------------------------------------
Function name:  my_outp

Description:    Perform an "out dx, al".

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
my_outp(FxU16 addr, FxU8 value)
{
    __asm
    {
	mov	dx, addr;
	mov	al, value;
	out	dx, al;
    }
}


/*----------------------------------------------------------------------
Function name:  my_outpw

Description:    Perform an "out dx, ax".

Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
my_outpw(FxU16 addr, FxU16 value)
{
    __asm
    {
	mov	dx, addr;
	mov	ax, value;
	out	dx, ax;
    }
}


/*----------------------------------------------------------------------
Function name:  my_inpd

Description:    Perform an "in eax, dx".

Information:    

Return:         FxU32   value of EAX.
----------------------------------------------------------------------*/
FxU32
my_inpd(FxU16 addr)
{
    FxU32 retval;

    __asm 
    {
	mov	dx, addr;
	in	eax, dx;
	mov	retval, eax;
    }

    return retval;
}


/*----------------------------------------------------------------------
Function name:  my_inp

Description:    Perform an "in al, dx".

Information:    

Return:         FxU8   value of AL.
----------------------------------------------------------------------*/
FxU8
my_inp(FxU16 addr)
{
    FxU8 retval;

    __asm 
    {
	mov	dx, addr;
	in	al, dx;
	mov	retval, al;
    }

    return retval;
}

#else  // #ifndef VDDONLY
FxU32 my_inpd(FxU16 addr);
void my_outpd(FxU16 addr, FxU32 value);
#endif // #ifndef VDDONLY

#ifdef VDDONLY
#define IOBASE (pDev->IoBase)
#else  // #ifdef VDDONLY
#define IOBASE regBase
#endif // #ifdef VDDONLY

#define SSTIOADDR(regName)	((FxU16)offsetof(SstIORegs, regName))
#define ISET32(addr, value)	my_outpd((FxU16)(IOBASE + SSTIOADDR(addr)), (FxU32)(value))
#define IGET32(addr)		my_inpd((FxU16)(IOBASE + SSTIOADDR(addr)))
#define MESSAGE			1 ? (void)0 : (void)
#define CHECKFORROOM		// as nothing
#define ISET8PHYS(addr, value)	my_outp((FxU16)(IOBASE + addr), (FxU8)(value))
#define IGET8PHYS(addr)		my_inp((FxU16)(IOBASE + addr))
#define ISET16PHYS(addr, value)	my_outpw((FxU16)(IOBASE + addr), (FxU16)(value))

#ifdef GDBG_INFO
#undef GDBG_INFO
#define GDBG_INFO		
#endif

#ifdef GDBG_ERROR
#undef GDBG_ERROR
#define GDBG_ERROR		1 ? (void)0 : (void)
#endif

#endif // #ifndef __H3CINITDD_H__
