#ifndef __AVENGER_INTERRUPT_HANDLING_H__
#define __AVENGER_INTERRUPT_HANDLING_H__

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
** File name:   avenger_interrupt_handling.h
**
**
** $Header: Avenger_Interrupt_Handling.h, 3, 10/11/00 8:33:46 PM, Brent$
**
** $History: avenger_interrupt_handling.h $
** 
** *****************  Version 2  *****************
** User: Shuaulme     Date: 7/27/99    Time: 3:11p
** Updated in $/devel/h3/MacOS8/GDX/src
** interrupt handling code
** 
** *****************  Version 1  *****************
** User: Kcd          Date: 6/03/99    Time: 6:45p
** Created in $/devel/h3/MacOS8/GDX/src
** MacOS 8 2D Display Driver
** 
**
*/


void 	AvengerHandleVBLInterrupts(void *vblRefCon);
void 	AvengerEnableVBLInterrupts(void *vblRefCon);
unsigned char 	AvengerDisableVBLInterrupts(void *vblRefCon);
void 	AvengerWaitForVBL();


#endif /* __AVENGER_INTERRUPT_HANDLING_H__ */