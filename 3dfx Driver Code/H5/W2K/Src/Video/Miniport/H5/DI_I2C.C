/* $Header: di_i2c.c, 5, 10/11/00 8:47:03 PM, Brent$ */
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
** File name:   di_i2c.c
**
** Description: Contains the device-independent portion of the ds/di
**              I2C implementation.  This includes the code that
**              actually implements the I2C protocol.
**
** $Log: 
**  5    3dfx      1.2.1.1     10/11/00 Brent           Forced check in to enforce
**       branching.
**  4    3dfx      1.2.1.0     09/06/00 Brent           Forced revision for (NO
**       INTENAL CHANGES) integrity check
**  3    3dfx      1.2         01/07/00 Ryan Bissell    Fixed a wrongful-negation
**       bug in i2c_readbytes(), spotted by Dale "Eagle Eye" Kenaston, and a timing
**       issue spotted by myself.
**  2    3dfx      1.1         09/22/99 Ryan Bissell    I2C-related updates
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
**
*/


#include "i2cmacro.h"
#include "di_i2c.h"
#include "ds_i2c.h"


#define I2C_PRIVATE static



/*----------------------------------------------------------------------
Function name:  InstantiateBusI2C

Description:    Initializes space for a I2C_BUSINFO struct using
                the parameters given.
                
Side effects:   On entry:   none.
                On exit:    adds an entry to BusList

Return:         (I2CBUS) a busid value for use with SetUsageI2C() and 
                         and AssociateBusToMuxI2C(), or I2C_NOTABUS
                         on error.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
I2CBUS InstantiateBusI2C(I2CCONTEXT pContext, I2C_GETSDA pfGetSDA, I2C_GETSCL pfGetSCL, I2C_SETSDA pfSetSDA, I2C_SETSCL pfSetSCL)
{
  int i;
  I2CBUS busid;
  PI2C_BUSINFO pbusinfo;
  PI2C_DEVINFO pinfo = I2C_DEVINFOPTR(pContext);

  if (!pinfo)
    return I2C_NOTABUS;

  if (!(pfGetSDA && pfGetSCL && pfSetSDA && pfSetSCL))
    return I2C_NOTABUS;

  busid = pinfo->numbuses +1;

  if (busid > I2C_MAXBUSES)
    return I2C_NOTABUS;

  pbusinfo = I2C_GETBUSINFO(pContext, busid);
  if (!pbusinfo)
    return I2C_NOTABUS;

  pbusinfo->key = I2C_NOTAKEY;
  pbusinfo->locked = FXFALSE;
  pbusinfo->muxid = I2C_NOTAMUX;
  pbusinfo->muxtoken = I2C_NOTATOKEN;

  for (i=0; i < I2C_MAXPURPOSES; i++)
    pbusinfo->Purposes[i] = I2C_NOTAPURPOSE;

  pbusinfo->funcGetSDA = pfGetSDA;
  pbusinfo->funcGetSCL = pfGetSCL;
  pbusinfo->funcSetSDA = pfSetSDA;
  pbusinfo->funcSetSCL = pfSetSCL;

  pinfo->numbuses += 1;
  return busid;
}

/*----------------------------------------------------------------------
Function name:  InstantiateMuxI2C

Description:    Assigns a muxid to the given mux function, so that
                it can be referenced by "AssociateBusToMuxI2C()".
                
Side effects:   On entry:   none.
                On exit:    adds an entry to MuxList and MuxFunction

Return:         (I2CMUX) a muxid value for use with AssociateBusToMuxI2C(), 
                         or I2C_NOTAMUX on error.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
I2CMUX InstantiateMuxI2C(I2CCONTEXT pContext, I2C_SETMUX pfMux)
{
  I2CMUX muxid;
  PI2C_DEVINFO pinfo = I2C_DEVINFOPTR(pContext);

  if (!(pinfo && pfMux))
    return I2C_NOTABUS;

  muxid = pinfo->nummuxen +1;

  if (muxid > I2C_MAXMUXEN)
    return I2C_NOTAMUX;

  pinfo->MuxFunction[pinfo->nummuxen] = pfMux;
  pinfo->MuxList[pinfo->nummuxen] = MUX_NOTINUSE;

  pinfo->nummuxen = muxid;
  return muxid;
}


/*----------------------------------------------------------------------
Function name:  SetUsageMuxI2C

Description:    Allows the caller to specify a usage code for
                a specific bus.  The caller may call this repeatedly
                for any given bus, up to I2C_MAXPURPOSES times
                per bus.
                
Side effects:   On entry:   Bus must have already been instantiated
                On exit:    adds an entry to the bus' "Purposes" list
  
Return:         (I2CBUS) a busid value for use with SetUsageI2C() and 
                         and AssociateBusToMuxI2C(), or I2C_NOTABUS
                         on error.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int SetUsageI2C(I2CCONTEXT pContext, I2CBUS busid, FxU32 usage)
{
  int i;
  PI2C_BUSINFO pbusinfo;
  PI2C_DEVINFO pinfo = I2C_DEVINFOPTR(pContext);

  if (!pinfo)
    return I2C_FAILURE;

  if (!busid || (busid > pinfo->numbuses))
    return I2C_FAILURE;

  pbusinfo = &pinfo->BusList[busid-1];

  // find the first index that contains zero (I2C_NOTAPURPOSE), and replace it
  for (i=0; (pbusinfo->Purposes[i] && (i < I2C_MAXPURPOSES)); i++)
    NULL;

  if (i >= I2C_MAXPURPOSES)
    return I2C_FAILURE;

  pbusinfo->Purposes[i] = usage;
  return I2C_SUCCESS;
}


/*----------------------------------------------------------------------
Function name:  AssociateBusToMuxI2C

Description:    Allows the caller to indicate that a specific bus is
                dependent upon a specific setting of a specific mux.
                The setting is referred to as a "mux token".
                
Side effects:   On entry:   Both bus and mux must have already been instantiated
                On exit:    Logically binds the given bus to the given mux.

Return:         (int) I2C_SUCCESS if successful; I2C_FAILURE otherwise.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int AssociateBusToMuxI2C(I2CCONTEXT pContext, I2CBUS busid, I2CMUX muxid, FxU32 muxtoken)
{
  PI2C_BUSINFO pbusinfo;

  if (!busid || (busid > I2C_DEVINFOPTR(pContext)->numbuses))
    return I2C_FAILURE;

  pbusinfo = &I2C_DEVINFOPTR(pContext)->BusList[busid-1];

  pbusinfo->muxid = muxid;         // this is the function to call
  pbusinfo->muxtoken = muxtoken;   // this is the value to give that function when using this bus
  return I2C_SUCCESS;
}



/*----------------------------------------------------------------------
Function name:  IsKeyValid (internal use only)

Description:    Reports whether or not the given key is valid.
                The corresponding bus is implied by the key's value.
                
Side effects:   On entry:   none.
                On exit:    none.

Return:         (int) I2C_SUCCESS if successful; I2C_FAILURE otherwise.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
I2C_PRIVATE int IsKeyValid(I2CCONTEXT pContext, I2CKEY key)
{
  I2C_BUSINFO info;

  if (!I2C_ISKEYSANE(pContext, key))
    return I2C_FAILURE;

  info = *I2C_GETBUSINFO(pContext, I2C_BUSFROMKEY(key));

  return (I2C_ISBUSLOCKED(pContext,info) && I2C_ISKEYVALID(pContext,info,key));
}


/*----------------------------------------------------------------------
Function name:  setSDA (internal use only)

Description:    Sets the bus SDA line to the specified state
                
Side effects:   On entry:   none.
                On exit:    the SDA line will be high, unless there is
                           some sort of bus contention.

Return:         (int) I2C_SUCCESS if successful; I2C_FAILURE otherwise.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
I2C_PRIVATE int setSDA(I2CCONTEXT pContext, PI2C_BUSINFO pinfo, int state, FxU8 delay)
{
  pinfo->funcSetSDA(pContext, !!state);
  I2C_DELAYTIME(delay*pinfo->speed);

  return ((pinfo->funcGetSDA(pContext) == !!state) ? I2C_SUCCESS : I2C_FAILURE);
}


/*----------------------------------------------------------------------
Function name:  setSCL (internal use only)

Description:    Sets the bus SCL line to the specified state
                
Side effects:   On entry:   none.
                On exit:    the SCL line will be high, unless there is
                           some sort of bus contention, or a slave device
                           stretched the clock longer than we were
                           willing to wait.

Return:         (int) I2C_SUCCESS if successful; I2C_FAILURE otherwise.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
I2C_PRIVATE int setSCL(I2CCONTEXT pContext, PI2C_BUSINFO pinfo, int state, FxU8 delay)
{
  int i;
  pinfo->funcSetSCL(pContext, !!state);

  if (state)
  { // the slave may try to stretch the clock
    for (i=I2C_STRETCH; !pinfo->funcGetSCL(pContext) && i; i--)
      I2C_DELAYTIME(delay*pinfo->speed);

    DEBUGI2C(!pinfo->funcGetSCL(pContext), ("I2C: setSCL(\"Slave stretched clock for too long.\")\n"));
  }

  I2C_DELAYTIME(delay*pinfo->speed);

  return ((pinfo->funcGetSCL(pContext) == !!state) ? I2C_SUCCESS : I2C_FAILURE);
}


/*----------------------------------------------------------------------
Function name:  getSDA (internal use only)

Description:    returns the current state of the bus' SDA line
                
Side effects:   On entry:   none.
                On exit:    none.

Return:         (int) HIGH if SDA is high; LOW otherwise.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
I2C_PRIVATE FxU8 getSDA(I2CCONTEXT pContext, PI2C_BUSINFO pinfo)
{
  return pinfo->funcGetSDA(pContext);
}


/*----------------------------------------------------------------------
Function name:  getSCL (internal use only)

Description:    returns the current state of the bus' SCL line
                
Side effects:   On entry:   none.
                On exit:    none.

Return:         (int) HIGH if SCL is high; LOW otherwise.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
I2C_PRIVATE FxU8 getSCL(I2CCONTEXT pContext, PI2C_BUSINFO pinfo)
{
  return pinfo->funcGetSCL(pContext);
}


/*----------------------------------------------------------------------
Function name:  i2c_initialize

Description:    initializes the I2C MRI
                
Side effects:   On entry:   none.
                On exit:    I2C support is ready for use

Return:         (int)  I2C_SUCCESS (1) or I2C_FAILURE (0)
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int i2c_initialize(I2CCONTEXT pContext)
{
  int i;
  PI2C_DEVINFO pinfo = I2C_DEVINFOPTR(pContext);
  
  if (!pinfo)
    return I2C_FAILURE;

  pinfo->numbuses = 0;
  pinfo->nummuxen = 0;

  for (i=0; i < I2C_MAXMUXEN; i++)
    pinfo->MuxList[i] = MUX_NOTINUSE;

  return ConfigureI2C(pContext);
}


/*----------------------------------------------------------------------
Function name:  i2c_getaccess

Description:    Locks a bus for use by a single process or thread.
                
Side effects:   On entry:   none.
                On exit:    Mux (if applicable) is configured so that
                           this bus can be used.

Return:         (I2CKEY)    A key for the bus, or I2C_NOTAKEY on failure.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
I2CKEY i2c_getaccess(I2CCONTEXT pContext, FxU32 usage, FxU8 speed)
{
  int i, j;
  PI2C_BUSINFO pinfo;
  I2CBUS busid = I2C_NOTABUS; //0

  // search for a bus that advertises the desired usage.
  for (i=I2C_DEVINFOPTR(pContext)->numbuses; i && !busid; i--)
    for (j=I2C_MAXPURPOSES; j && !busid; j--)
      if (I2C_BUSLIST(pContext)[i-1].Purposes[j-1] == usage)
        busid = i;

  if (!busid)
    return I2C_NOTAKEY; // no bus supports this usage

  pinfo = &I2C_BUSLIST(pContext)[busid-1];

  if (I2C_ISBUSLOCKED(pContext, *pinfo))
    return I2C_NOTAKEY; // bus (or mux) is already in use

  pinfo->speed = (speed ? speed : 1);

  // if the bus relies on a mux, then claim that mux.
  if (pinfo->muxid)
  {
    I2C_MUXLIST(pContext)[pinfo->muxid-1] = pinfo->muxtoken;
    I2C_MUXFUNCTION(pContext)[pinfo->muxid-1](pContext, pinfo->muxtoken);
  }
   
  pinfo->locked = FXTRUE;
  pinfo->key = I2C_CREATEKEY(busid, ((pinfo->key & 0x00FFFFFF)+1 ? pinfo->key+1 : 1));
  return pinfo->key;
}



/*----------------------------------------------------------------------
Function name:  i2c_endaccess

Description:    Unlocks a previously locked bus.
                
Side effects:   On entry:   The bus must be presently locked.
                On exit:    The bus is no longer locked.

Return:         (void)      Nothing.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
void i2c_endaccess(I2CCONTEXT pContext, I2CKEY key)
{
  I2C_BUSINFO* pinfo;

  if (!IsKeyValid(pContext, key))
    return;

  pinfo = I2C_GETBUSINFO(pContext, I2C_BUSFROMKEY(key));

  // if there's a mux, free it.
  if (pinfo->muxid)
    I2C_MUXLIST(pContext)[pinfo->muxid-1] = MUX_NOTINUSE;

  pinfo->locked = FXFALSE;
}


/*----------------------------------------------------------------------
Function name:  i2c_setspeed

Description:    Sets the delay multiplier for the given bus
                
Side effects:   On entry:   The given key must be valid.
                On exit:    The delay multiplier is changed.

Return:         (void)      Nothing.
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
void i2c_setspeed(I2CCONTEXT pContext, I2CKEY key, FxU8 speed)
{
  I2C_BUSINFO* pinfo;

  if (!IsKeyValid(pContext, key))
    return;

  pinfo = I2C_GETBUSINFO(pContext, I2C_BUSFROMKEY(key));
  pinfo->speed = (speed ? speed : 1);
}



/*----------------------------------------------------------------------
Function name:  i2c_stop

Description:    Generates a stop condition on the bus
                
Side effects:   On entry:   The given key must be valid.
                On exit:    SDA is high, and SCL is high

Return:         (int)  I2C_SUCCESS (1) or I2C_FAILURE (0)
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int i2c_stop(I2CCONTEXT pContext, I2CKEY key)
{
  int result=1;
  I2C_BUSINFO* pBus;

  if (!IsKeyValid(pContext, key))
    return I2C_FAILURE;

  pBus = I2C_GETBUSINFO(pContext, I2C_BUSFROMKEY(key));

  // stop is defined as SCL going HIGH while SDA is LOW, followed by SDA going HIGH.
  // some setup is necessary to ensure that SDA and SCL are LOW before doing this.
  //
  // SDA                       ____________ minimum timing requirements:
  //                          / [----3----]    [1] slave might stretch this
  //                         /                 [2] = 4.0 uSec (tSU-STO)   
  //     ___________________/                  [3] = 4.7 uSec (tBUF)
  //                 [--2--]                   
  // SCL             ______________________    
  //                /                          
  //     [---1---] /                           
  //     _________/                         


  result &= setSCL(pContext, pBus, 0, tNODELAY);
  result &= setSDA(pContext, pBus, 0, tNODELAY);
  result &= setSCL(pContext, pBus, 1, tSUSTO);    // [1] and [2]
  result &= setSDA(pContext, pBus, 1, tBUF);      // [3]

  return (result ? I2C_SUCCESS : I2C_FAILURE);
}



/*----------------------------------------------------------------------
Function name:  i2c_start

Description:    Generates a start condition on the bus
                
Side effects:   On entry:   The given key must be valid.
                On exit:    SDA is low, and SCL is low

Return:         (int)  I2C_SUCCESS (1) or I2C_FAILURE (0)
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int i2c_start(I2CCONTEXT pContext, I2CKEY key)
{
  int result=1;
  I2C_BUSINFO* pBus;

  if (!IsKeyValid(pContext, key))
    return I2C_FAILURE;

  pBus = I2C_GETBUSINFO(pContext, I2C_BUSFROMKEY(key));

  // start is defined as SDA going LOW while SCL is HIGH, followed by SCL going LOW
  // some setup is necessary to ensure that SDA and SCL are HIGH before doing this.
  //
  // SDA _________                          minimum timing requirements:   
  //     [---1---]\                            [1] = 4.7 uSec (tSU-STA)
  //               \                           [2] = 4.0 uSec (tHD-STA)
  //                \______________________    [3] = 4.7 uSec (tLOW)
  //                 [--2--] 
  // SCL ___________________
  //                        \
  //                         \  
  //                          \____________
  //                            [----3----]

  result &= setSDA(pContext, pBus, 1, tNODELAY);
  result &= setSCL(pContext, pBus, 1, tSUSTA);   // [1]
  result &= setSDA(pContext, pBus, 0, tHDSTA);   // [2]
  result &= setSCL(pContext, pBus, 0, tLOW);     // [3]

  return (result ? I2C_SUCCESS : I2C_FAILURE);
}



/*----------------------------------------------------------------------
Function name:  i2c_sendbyte

Description:    Sends a byte across the bus
                
Side effects:   On entry:   expects key to be valid, SCL to be low
                On exit:    SDA is high, SCL is low

Return:         (int)  I2C_SUCCESS (1) or I2C_FAILURE (0)
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int i2c_sendbyte(I2CCONTEXT pContext, I2CKEY key, FxU8 byte)
{
  int i;
  int ack;
  int result=1;
  int master=1;
  I2C_BUSINFO* pBus;

  if (!IsKeyValid(pContext, key))
    return I2C_FAILURE;

  pBus = I2C_GETBUSINFO(pContext, I2C_BUSFROMKEY(key));

  // send the byte
  for (i=8; i && master; i--)
  {
    master = setSDA(pContext, pBus, (byte >> (i-1)) & 0x01, tSUDAT);

#if I2C_PEDANTIC
    DEBUGI2C(!master, ("I2C: sendbyte(\"Looks like we lost the bus arbitration-- backing off.\")\n"));
    result &= master;
#endif

    result &= setSCL(pContext, pBus, 1,                     tHIGH);
    result &= setSCL(pContext, pBus, 0,                     tLOW);
  }

  if (master)
  {
    // wait for ack
    setSDA(pContext, pBus, 1, tSUDAT);  // don't check result of this; we expect it to be "wrong" (ack)
    for (i=I2C_STRETCH; getSDA(pContext, pBus) && i; i--)
      I2C_DELAYTIME(tSUDAT*pBus->speed);

    ack = getSDA(pContext, pBus) ? I2C_FAILURE : I2C_SUCCESS;

    // clock in the ack bit
    result &= setSCL(pContext, pBus, 1,                     tHIGH);
    result &= setSCL(pContext, pBus, 0,                     tLOW);
  }

  DEBUGI2C(!ack,    ("I2C: sendbyte(\"Slave took to long to acknowledge.\")\n"));
  DEBUGI2C(!result, ("I2C: sendbyte(\"Bus errors occurred.\")\n"));
  return I2C_PEDANTIC ? (ack && result) : ack;
}



/*----------------------------------------------------------------------
Function name:  i2c_readbyte

Description:    reads a byte from the bus, and ACKs it if ack!=0
                
Side effects:   On entry:   Expects SCL low
                On exit:    SDA is high, SCL is low

Return:         (int)  I2C_SUCCESS (1) or I2C_FAILURE (0)
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int i2c_readbyte(I2CCONTEXT pContext, I2CKEY key, FxU8* pbyte, int ack)
{
  int i;
  I2C_BUSINFO* pBus;
  int result=I2C_SUCCESS;

  if (!IsKeyValid(pContext, key))
    return I2C_FAILURE;

  pBus = I2C_GETBUSINFO(pContext, I2C_BUSFROMKEY(key));

  *pbyte = 0;
  for (i=8; i; i--)   //read the byte
  {
    result &=  setSCL(pContext, pBus, 1, tHIGH);
    *pbyte <<= 1;
    *pbyte |=  getSDA(pContext, pBus);
    result &=  setSCL(pContext, pBus, 0, tLOW);
  }

  // send the ack/nack bit
  result &= setSDA(pContext, pBus, !ack, tSUDAT);
  result &= setSCL(pContext, pBus, 1,    tHIGH);
  result &= setSCL(pContext, pBus, 0,    tLOW);

  // don't check result here, because slave may be holding this low.
  setSDA(pContext, pBus, 1,    tSUDAT);

  DEBUGI2C(!result, ("I2C: readbyte(\"Bus errors occurred.\")\n"));
  return (I2C_PEDANTIC ? result : I2C_SUCCESS);
}



/*----------------------------------------------------------------------
Function name:  i2c_sendbytes

Description:    Sends a series of bytes across the bus
                
Side effects:   On entry:   expects key to be valid, SCL to be low
                On exit:    SDA is high, SCL is low

Return:         (int)  I2C_SUCCESS (1) or I2C_FAILURE (0)
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int i2c_sendbytes(I2CCONTEXT pContext, I2CKEY key, FxU16 numbytes, FxU8* pbytes)
{
  FxU32 i;
  int result=I2C_SUCCESS;

  if (!IsKeyValid(pContext, key))
    return I2C_FAILURE;

  for (i=0; i < numbytes; i++)
    result &= i2c_sendbyte(pContext, key, pbytes[i]);

  return result;
}



/*----------------------------------------------------------------------
Function name:  i2c_readbytes

Description:    reads a series of bytes from the bus, and ACKs the 
                last one if acklastbyte!=0
                
Side effects:   On entry:   expects key to be valid, SCL to be low
                On exit:    SDA is high, SCL is low

Return:         (int)  I2C_SUCCESS (1) or I2C_FAILURE (0)
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int i2c_readbytes(I2CCONTEXT pContext, I2CKEY key, FxU16 numbytes, FxU8* pbytes, int acklastbyte)
{
  FxU32 i;
  int result=I2C_SUCCESS;

  if (!IsKeyValid(pContext, key))
    return I2C_FAILURE;

  for (i=numbytes; i; i--)
    result &= i2c_readbyte(pContext, key, &pbytes[numbytes-i], (acklastbyte && (i==1)));

  return result;
}


/*----------------------------------------------------------------------
Function name:  i2c_getsda

Description:    DANGER, WILL ROBINSON!!! Only use this function if you
                absolutely have to.
                
Side effects:   On entry:   expects key to be valid
                On exit:    if the key isn't valid, this function returns
                            HIGH (to avoid the appearance that the client
                            ACKnowledged.)  Otherwise, it returns the
                            current state of SCL.

Return:         (int)  HIGH (1) or LOW (0)
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int i2c_getsda(I2CCONTEXT pContext, I2CKEY key)
{
  I2C_BUSINFO* pBus;

  if (!IsKeyValid(pContext, key))
    return HIGH;

  pBus = I2C_GETBUSINFO(pContext, I2C_BUSFROMKEY(key));

  return getSDA(pContext, pBus);
}


/*----------------------------------------------------------------------
Function name:  i2c_getscl

Description:    DANGER, WILL ROBINSON!!! Only use this function if you
                absolutely have to.
                
Side effects:   On entry:   expects key to be valid
                On exit:    if the key isn't valid, this function returns
                            HIGH (to avoid the appearance that the client
                            ACKnowledged.)  Otherwise, it returns the
                            current state of SCL.

Return:         (int)  HIGH (1) or LOW (0)
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int i2c_getscl(I2CCONTEXT pContext, I2CKEY key)
{
  I2C_BUSINFO* pBus;

  if (!IsKeyValid(pContext, key))
    return HIGH;

  pBus = I2C_GETBUSINFO(pContext, I2C_BUSFROMKEY(key));
  return getSCL(pContext, pBus);
}


/*----------------------------------------------------------------------
Function name:  i2c_setsda

Description:    DANGER, WILL ROBINSON!!! Only use this function if you
                absolutely have to.
                
Side effects:   On entry:   expects key to be valid
                On exit:    sets SDA to the desired state, and delays
                           for a total of speed*delay microseconds.

Return:         (int)  I2C_SUCCESS (1) or I2C_FAILURE (0)
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int i2c_setsda(I2CCONTEXT pContext, I2CKEY key, int state, FxU8 delay)
{
  I2C_BUSINFO* pBus;

  if (!IsKeyValid(pContext, key))
    return I2C_FAILURE;

  pBus = I2C_GETBUSINFO(pContext, I2C_BUSFROMKEY(key));
  return setSDA(pContext, pBus, state, delay);
}


/*----------------------------------------------------------------------
Function name:  i2c_setscl

Description:    DANGER, WILL ROBINSON!!! Only use this function if you
                absolutely have to.
                
Side effects:   On entry:   expects key to be valid
                On exit:    sets SCL to the desired state, and delays
                            for a total of speed*delay microseconds.

Return:         (int)  I2C_SUCCESS (1) or I2C_FAILURE (0)
------------------------------------------------------------------------
FORMATTING:     Indention is 2 spaces, no tab characters
----------------------------------------------------------------------*/
int i2c_setscl(I2CCONTEXT pContext, I2CKEY key, int state, FxU8 delay)
{
  I2C_BUSINFO* pBus;

  if (!IsKeyValid(pContext, key))
    return I2C_FAILURE;

  pBus = I2C_GETBUSINFO(pContext, I2C_BUSFROMKEY(key));
  return setSCL(pContext, pBus, state, delay);
}


#undef I2C_PRIVATE