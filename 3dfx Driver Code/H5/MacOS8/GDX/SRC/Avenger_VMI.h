#ifndef __AVENGER_VMI_H__
#define __AVENGER_VMI_H__

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
** File name:   avenger_VMI.h
**
**
** $Header: Avenger_VMI.h, 3, 10/11/00 8:33:48 PM, Brent$
**
** $History: avenger_VMI.h $
** 
** *****************  Version 1  *****************
** User: Kcd          Date: 6/03/99    Time: 6:45p
** Created in $/devel/h3/MacOS8/GDX/src
** MacOS 8 2D Display Driver
** 
**
*/


#define VMI_enable		0x00000001
#define VMI_CS_n		0x00000002
#define VMI_RD_n		0x00000004
#define VMI_W_n			0x00000008
#define VMI_RDY_n		0x00000010
#define VMI_DAT_OUT_n	0x00000020
#define VMI_DATA		0x00003fc0
#define VMI_ADDR		0x0003c000
#define TV_RESET_n		0x80000000

void VMI_IDLE(FxU32 regBase);
void VMI_ADDR_phase(FxU32 regBase,FxU32 addr);
void VMI_WRITE_phase(FxU32 regBase, FxU32 data);
void VMI_ENDW_phase(FxU32 regBase);
void VID_DELAY(FxU32 regBase);


#endif /* __AVENGER_VMI_H__ */