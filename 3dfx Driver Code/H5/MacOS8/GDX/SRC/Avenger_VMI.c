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
** File name:   avenger_VMI.c
**
** Description: HAL layer for MacOS Display Driver.
**
** $Header: Avenger_VMI.c, 3, 10/11/00 8:33:47 PM, Brent$
**
** $History: avenger_VMI.c $
** 
** *****************  Version 1  *****************
** User: Kcd          Date: 6/03/99    Time: 6:45p
** Created in $/devel/h3/MacOS8/GDX/src
** MacOS 8 2D Display Driver
** 
**
*/

#include "GraphicsPriv.h"
#include "GraphicsPrivHwc.h"
#include "GraphicsPrivData.h"
#include "avenger_VMI.h"


void VMI_IDLE(FxU32 regBase)
{
	FxU32 reg;
	
	reg = IGET32(vidSerialParallelPort);
	reg = 0;
	reg = VMI_enable | VMI_CS_n | VMI_RD_n | VMI_W_n | VMI_DATA | VMI_ADDR | TV_RESET_n;
	ISET32(vidSerialParallelPort,reg);
} 
void VMI_ADDR_phase(FxU32 regBase, FxU32 addr)
{
	FxU32 reg;
	reg = IGET32(vidSerialParallelPort);
	reg &= ~VMI_CS_n;
	reg &= ~VMI_ADDR;
	reg |= addr << 14;
	ISET32(vidSerialParallelPort,reg);
}
void VMI_WRITE_phase(FxU32 regBase, FxU32 data)
{
	FxU32 reg = IGET32(vidSerialParallelPort);
	reg &= ~0x00000008;
	reg |= 0x00010000; // Hack
	reg &= ~VMI_DATA;
	reg |= data << 6;
	ISET32(vidSerialParallelPort,reg);
}
void VMI_ENDW_phase(FxU32 regBase)
{
	FxU32 reg;
	reg = IGET32(vidSerialParallelPort);
	reg |= 0x00010000;
	reg |= VMI_W_n;
	ISET32(vidSerialParallelPort,reg);
}
void VID_DELAY(FxU32 regBase)
{
	FxU32 reg = IGET32(miscInit0);
	reg &= 0xfff0ffff;
	reg |= 0x00060000;
	ISET32(miscInit0,reg);
}

