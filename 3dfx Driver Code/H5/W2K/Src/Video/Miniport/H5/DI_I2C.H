/* $Header: di_i2c.h, 3, 10/11/00 8:47:04 PM, Brent$ */
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
** File name:   di_i2c.h
**
** Description: Defines the device-independent constructs for the ds/di
**              I2C implementation.
**
** $Log: 
**  3    3dfx      1.1.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.1         09/22/99 Ryan Bissell    I2C-related updates
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
**
*/

#ifndef __DII2C_H__
#define __DII2C_H__

typedef unsigned char  FxU8;
typedef unsigned short FxU16;
typedef unsigned long  FxU32;

typedef void* I2CCONTEXT;

#define LOW  0
#define HIGH 1

#define I2C_READSLAVE      ((FxU8)0x01)
#define I2C_WRITESLAVE     ((FxU8)0x00)

#define I2C_FAILURE        0L
#define I2C_SUCCESS        1L

#define I2C_NOTAPURPOSE    (0x00000000UL)    // this is a list initialization value
#define I2C_TVENCODER      (0x00000001UL)
#define I2C_MONITORDDC     (0x00000002UL)
#define I2C_MULTIMEDIA     (0x00000003UL)    // used for WDM stuff (tuners, decoders, ad nauseum)
#define I2C_FLATPANELDDC   (0x00000004UL)
#define I2C_SERIALEEPROM   (0x00000005UL)
#define I2C_LCDSCALER      (0x00000006UL)
#define I2C_LPTDEBUGGING   (0xDEADBEEFUL)    // can be used to use a parallel port as an i2c master

typedef FxU32 I2CKEY;
typedef FxU32 I2CMUX;
typedef FxU32 I2CBUS;

#define I2C_NOTAKEY        (0x00000000UL)
#define I2C_NOTABUS        (0x00000000UL)
#define I2C_NOTAMUX        (0x00000000UL)
#define I2C_NOTATOKEN      (0x00000000UL)

#define I2C_NORMALSPEED    (0x00000000UL)

// initialization
int i2c_initialize(I2CCONTEXT pContext);

// access control
I2CKEY i2c_getaccess(I2CCONTEXT pContext, FxU32 usage, FxU8 speed);
void   i2c_endaccess(I2CCONTEXT pContext, I2CKEY key);
void   i2c_setspeed(I2CCONTEXT pContext, I2CKEY key, FxU8 speed);

// exclusive high-level access
int i2c_stop(I2CCONTEXT pContext, I2CKEY key);
int i2c_start(I2CCONTEXT pContext, I2CKEY key);
int i2c_sendbyte(I2CCONTEXT pContext, I2CKEY key, FxU8 byte);
int i2c_readbyte(I2CCONTEXT pContext, I2CKEY key, FxU8* pbyte, int ack);
int i2c_sendbytes(I2CCONTEXT pContext, I2CKEY key, FxU16 numbytes, FxU8* pbytes);
int i2c_readbytes(I2CCONTEXT pContext, I2CKEY key, FxU16 numbytes, FxU8* pbytes, int acklastbyte);

// primitives
int i2c_getsda(I2CCONTEXT pContext, I2CKEY key);
int i2c_getscl(I2CCONTEXT pContext, I2CKEY key);
int i2c_setsda(I2CCONTEXT pContext, I2CKEY key, int state, FxU8 delay);
int i2c_setscl(I2CCONTEXT pContext, I2CKEY key, int state, FxU8 delay);

#endif // __DII2C_H__