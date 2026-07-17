/* $Header: nvram.h, 3, 10/11/00 8:54:47 PM, Brent$ */
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
** File name:   nvram.h
**
** Description: Provides an interface to the serial EEPROM we use to store
**              boot-time configuration and user settings.
**
** $Log: 
**  3    3dfx      1.1.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.1         09/22/99 Ryan Bissell    I2C updates  (multi-monitor
**       fixes)
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
**
*/


#ifndef __NVRAM_H__
#define __NVRAM_H__


typedef void* NVRAMCONTEXT;


FxU8 nvramRead(NVRAMCONTEXT pContext, FxU8 offset);
int nvramWrite(NVRAMCONTEXT pContext, FxU8 offset, FxU32 numBytes, FxU8 *values);


#define NV_TVSTANDARD 0x00
#define NV_CONNECTOR  0x01

#endif