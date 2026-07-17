/* $Header: ds_i2c.h, 5, 10/11/00 8:55:46 PM, Brent$ */
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
** File name:   ds_i2c.h
**
** Description: Defines the device-specific constructs for the ds/di
**              I2C implementation.
**
** $Log: 
**  5    3dfx      1.3.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  4    3dfx      1.3         12/22/99 Dan O'Connel    Change GetDdcInformation
**       and GetDfpEdidInformation to use the public I2C interfaces defined in
**       di_i2c.h rather than exposing interfaces in ds_i2c.h that should not be
**       public.
**  3    3dfx      1.2         11/22/99 Dan O'Connel    Enumerate Digital Flat
**       Panel device as a Monitor for Win2K and unload EDID information from the
**       DFP.
**  2    3dfx      1.1         09/22/99 Ryan Bissell    I2C-related updates
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
**
*/


#ifndef __DSI2C_H__
#define __DSI2C_H__


#include "di_i2c.h"
#include "i2csetup.h"


#ifndef I2C_STRETCH
#define I2C_STRETCH 50
#endif

#ifndef I2C_PEDANTIC
#define I2C_PEDANTIC 0
#endif

#ifndef I2C_MAXBUSES
#define I2C_MAXBUSES 10
#endif

#ifndef I2C_MAXMUXEN
#define I2C_MAXMUXEN 10
#endif

#ifndef I2C_MAXPURPOSES
#define I2C_MAXPURPOSES 10
#endif


#define tBUF   5
#define tLOW   5
#define tHIGH  4
#define tHDSTA 4
#define tSUSTO 4
#define tSUSTA 5
#define tHDDAT 1     // because 0 isn't practical
#define tSUDAT 1
#define tNODELAY 1   // because 0 isn't practical


typedef unsigned int FxBOOL;

typedef FxU8 (*I2C_GETSDA)(I2CCONTEXT pContext);
typedef FxU8 (*I2C_GETSCL)(I2CCONTEXT pContext);
typedef void (*I2C_SETSDA)(I2CCONTEXT pContext, int state);
typedef void (*I2C_SETSCL)(I2CCONTEXT pContext, int state);

typedef void (*I2C_SETMUX)(I2CCONTEXT pContext, FxU32 value);


#define I2C_NOTMUXED  0

#define MUX_NOTINUSE (0x00000000UL)

typedef struct __I2C_BUSINFO
{
  I2CKEY key;
  FxU8 speed;
  FxBOOL locked;

  FxU32 Purposes[I2C_MAXPURPOSES];

  I2CMUX muxid;
  FxU32 muxtoken;
  I2C_GETSDA funcGetSDA;
  I2C_GETSCL funcGetSCL;
  I2C_SETSDA funcSetSDA;
  I2C_SETSCL funcSetSCL;

} I2C_BUSINFO, *PI2C_BUSINFO;


typedef struct __I2C_DEVINFO
{
  FxU32 numbuses;
  FxU32 nummuxen;
  I2C_BUSINFO BusList[I2C_MAXBUSES];
  I2CMUX MuxList[I2C_MAXMUXEN];
  I2C_SETMUX MuxFunction[I2C_MAXMUXEN];

} I2C_DEVINFO, *PI2C_DEVINFO;



/*
 *
 * THESE FUNCTIONS ARE USED TO DESCRIBE TO DI_I2C.C HOW THINGS ARE CONNECTED UP
 *
 */
int ConfigureI2C(I2CCONTEXT pContext);
int SetUsageI2C(I2CCONTEXT pContext, I2CBUS busid, FxU32 usage);
I2CBUS InstantiateBusI2C(I2CCONTEXT pContext, I2C_GETSDA pfGetSDA, I2C_GETSCL pfGetSCL, I2C_SETSDA pfSetSDA, I2C_SETSCL pfSetSCL);
I2CMUX InstantiateMuxI2C(I2CCONTEXT pContext, I2C_SETMUX pfMux);
int AssociateBusToMuxI2C(I2CCONTEXT pContext, I2CBUS busid, I2CMUX muxid, FxU32 muxtoken);



#define I2C_MUXLIST(pcontext) I2C_DEVINFOPTR(pcontext)->MuxList
#define I2C_MUXFUNCTION(pcontext) I2C_DEVINFOPTR(pcontext)->MuxFunction
#define I2C_BUSLIST(pcontext) I2C_DEVINFOPTR(pcontext)->BusList

#define I2C_ISKEYSANE(pcontext, key)  ((key != I2C_NOTAKEY)                                 \
                                      && (I2C_BUSFROMKEY(key) != 0)                         \
                                      && (I2C_BUSFROMKEY(key) <= I2C_DEVINFOPTR(pcontext)->numbuses))


#define I2C_ISBUSLOCKED(pcontext,info) ((info).muxid ? I2C_MUXLIST(pcontext)[(info).muxid-1] : (info).locked)


// for a key to be valid, the bus' muxtoken must also match the token in I2C_MUXLIST
#define I2C_ISKEYVALID(pcontext,info,key) ( ((info).key == (key)) && ((info).muxid ? (I2C_MUXLIST(pcontext)[(info).muxid-1] == (info).muxtoken) : 1) )

#define I2C_CREATEKEY(bus,lock) ((((FxU32)(bus)) << 24) | ((FxU8)(lock) & 0x00FFFFFF))

#define I2C_BUSFROMKEY(key)   ((FxU32)((key) >> 24) & 0x000000FF)
#define I2C_LOCKFROMKEY(key)  ((FxU32)((key) & 0x00FFFFFF))


#endif // __DSI2C_H__

