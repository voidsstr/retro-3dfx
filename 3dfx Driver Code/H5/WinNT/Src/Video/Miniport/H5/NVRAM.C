/* $Header: nvram.c, 7, 10/11/00 8:58:36 PM, Brent$ */
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
** File name:   nvram.c
**
** Description: Provides an interface to the serial EEPROM we use to store
**              boot-time configuration and user settings.
**
** $Log: 
**  7    3dfx      1.5.3.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  6    3dfx      1.5         12/21/99 Dan O'Connel    Previous mechanism was
**       unreliable in writing to NVRAM.  Change to new mechanism that prvides for
**       retries.
**  5    3dfx      1.4         12/15/99 Dan O'Connel    Correct type that caused
**       build problem with Win2K.
**  4    3dfx      1.3         12/15/99 Dan O'Connel    Optimize NvramWrite so
**       checksum is not written unless it has possibly changed.
**  3    3dfx      1.2         12/15/99 Dan O'Connel    Correct the location that
**       the NVRAM version is being written too.  Previously the code was
**       attempting to write it to location 0xFE in a NVRAM area that was only 0x7F
**       bytes long.
**  2    3dfx      1.1         09/22/99 Ryan Bissell    I2C updates  (multi-monitor
**       fixes)
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
**
*/


#include "3dfx.h"

#ifdef WINNT
#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"
#include "h3.h"
#else
#include "h3vdd.h"
#include "h3.h"
#define VDDONLY
#include "devtable.h"
#undef  VDDONLY
#endif


#include "bios.h"
#include "nvram.h"

// Please Note: this module is intended to be operating system independent, O/S structure differences are
// hidden by the following macros.
#ifdef WINNT
#define NVRAM_DEVINFOPTR(pcontext) ((PHW_DEVICE_EXTENSION)(pcontext))
#else
#define NVRAM_DEVINFOPTR(pcontext) ((PDEVTABLE)(pcontext))
#endif


#define NV_PRIVATE static

#define NV_ACCESSRATE 10

#define NV_READADDRESS  ((FxU8)0xA1)
#define NV_WRITEADDRESS ((FxU8)0xA0)

/*----------------------------------------------------------------------
Function name:  nvramWriteByte

Description:    Write value at addr which should be in the range 0 -> 127

Information:    Returns 0 on failure, 1 on success

Return:         int
----------------------------------------------------------------------*/
FxI32 nvramWriteByte (NVRAMCONTEXT pContext,  I2CKEY key, FxU8 addr, FxU8 value)
{
  int retries = 10;
  int status = 0;
	
  while ((status != 1) && --retries)
  {
    status = 1;
    status &= i2c_start(pContext, key);
    status &= i2c_sendbyte(pContext, key, NV_WRITEADDRESS);
    status &= i2c_sendbyte(pContext, key, addr);
 
    status &= i2c_sendbyte(pContext, key, value);
    status &= i2c_stop(pContext, key);
  }

  return (retries <= 0 ? 0 : status);
}


/*----------------------------------------------------------------------
Function name: nvramRead128

Description:   Reads the contents of the 128-byte serial EEPROM
               into the given buffer.
                
Side effects:  On entry:   none.
               On exit:    none

Returns:       (int) -1 on failure, or checksum (which should be zero.) 
------------------------------------------------------------------------
FORMATTING:    Indention is 2 spaces, no tab characters.
----------------------------------------------------------------------*/
NV_PRIVATE int nvramRead128(NVRAMCONTEXT pContext, I2CKEY i2ckey, FxU8 *buffer128)
{
  FxU32 sum;
  I2CKEY key;
  FxU32 i, status;

  if (i2ckey == I2C_NOTAKEY)
    key = i2c_getaccess(pContext, I2C_SERIALEEPROM, NV_ACCESSRATE);
  else
    key = i2ckey;

  if (key == I2C_NOTAKEY)
    return (-1);


  status = 1;
  status &= i2c_start(pContext, key);
  status &= i2c_sendbyte(pContext, key, NV_WRITEADDRESS);
  status &= i2c_sendbyte(pContext, key, 0x00);

  status &= i2c_start(pContext, key);
  status &= i2c_sendbyte(pContext, key, NV_READADDRESS);

  for (sum=0,i=0; status && i<128; i++)
  {
    status &= i2c_readbyte(pContext, key, &buffer128[i], (i != 127));
    sum += buffer128[i];
  }
  status &= i2c_stop(pContext, key);


  if (i2ckey == I2C_NOTAKEY)
    i2c_endaccess(pContext, key);

  return status ? (sum & 0xFF) : (-1);
}


/*----------------------------------------------------------------------
Function name: nvramUpdateChecksum

Description:   Uses the nvram cache to compute the checksum, and
               updates both the cache, and the nvram with the new
               checksum value.
                
Side effects:  On entry:   none.
               On exit:    none

Returns:       (int) non-zero on success, 0 on failure. 
------------------------------------------------------------------------
FORMATTING:    Indention is 2 spaces, no tab characters.
----------------------------------------------------------------------*/
NV_PRIVATE int nvramUpdateChecksum(NVRAMCONTEXT pContext, I2CKEY key)
{
  int i, sum, status;

  for (i=0,sum=0; i<127; i++)
    sum -= NVRAM_DEVINFOPTR(pContext)->nvramCache[i];

  NVRAM_DEVINFOPTR(pContext)->nvramCache[127] = (FxU8)(sum & 0x000000FF);

  status = nvramWriteByte(pContext, key, 127, NVRAM_DEVINFOPTR(pContext)->nvramCache[127]);

  return status;
}



/*----------------------------------------------------------------------
Function name: internalNvramWrite

Description:   Writes the specified values to the specified range of
               offsets of the 128-byte serial EEPROM.
                
Side effects:  On entry:   none.
               On exit:    none

Returns:       (int) non-zero on success, 0 on failure. 
------------------------------------------------------------------------
FORMATTING:    Indention is 2 spaces, no tab characters.
----------------------------------------------------------------------*/
int internalNvramWrite(NVRAMCONTEXT pContext, I2CKEY key, FxU8 offset, FxU32 numBytes, FxU8 *values, FxBool forceWrite)
{
  int i, status=1;
  FxBool nvramChanged = FALSE;

  if (0 == numBytes)
    return (1);

  for (i=0; i<(int)numBytes; i++)
  {
    if (forceWrite || NVRAM_DEVINFOPTR(pContext)->nvramCache[i+offset] != values[i])
    {
      status &= nvramWriteByte(pContext, key, (FxU8)(i+offset), values[i]);
      NVRAM_DEVINFOPTR(pContext)->nvramCache[i+offset] = values[i];
      nvramChanged = TRUE;
    }
  }

  if (nvramChanged)
      status &= nvramUpdateChecksum(pContext, key);

  return status;
}



/*----------------------------------------------------------------------
Function name: nvramInitCache

Description:   Initializes the nvram cache.
                
Side effects:  On entry:   none.
               On exit:    none

Returns:       (int) non-zero on success, 0 on failure. 
------------------------------------------------------------------------
FORMATTING:    Indention is 2 spaces, no tab characters.
----------------------------------------------------------------------*/
NV_PRIVATE int nvramInitCache(NVRAMCONTEXT pContext, I2CKEY i2ckey)
{
  int sum;
  I2CKEY key;
  int i, status;

  if (NVRAM_DEVINFOPTR(pContext)->nvramCacheInit)
    return 1;

  if (i2ckey == I2C_NOTAKEY)
    key = i2c_getaccess(pContext, I2C_SERIALEEPROM, NV_ACCESSRATE);
  else
    key = i2ckey;

  //If we still don't have a key, something is wrong.  But bailing out
  //isn't a nice option, so pretend the checksum failed.
  sum = (key == I2C_NOTAKEY) ? 1 : nvramRead128(pContext, key, NVRAM_DEVINFOPTR(pContext)->nvramCache);

  if (sum)  //oops-- nvram contents are hosed (or we couldn't get a key)
  {
    for (i=0; i<127; i++)   //everything but the last byte
    {
      switch (i)  //init to defaults
      {
        case 0x00:  NVRAM_DEVINFOPTR(pContext)->nvramCache[i] = BIOS_NVx00_NTSC;        break;
        case 0x01:  NVRAM_DEVINFOPTR(pContext)->nvramCache[i] = BIOS_NVx01_AUTODETECT;  break;
        case 0x7E:  NVRAM_DEVINFOPTR(pContext)->nvramCache[i] = BIOS_NVx7E_VERSIONCODE; break;
        default:    NVRAM_DEVINFOPTR(pContext)->nvramCache[i] = 0xFF;                   break;
      }
    }
    
    if (key != I2C_NOTAKEY)
    {
      //since the nvram data seems to be bad, re-initialize it
      status = internalNvramWrite(pContext, key, 0, 127, &(NVRAM_DEVINFOPTR(pContext)->nvramCache[0]), TRUE);
    }
  }

  if (i2ckey == I2C_NOTAKEY)    //if we locked the bus here, unlock it here.
    i2c_endaccess(pContext, key);

  NVRAM_DEVINFOPTR(pContext)->nvramCacheInit = 1;
  return (status && key);
}




/*----------------------------------------------------------------------
Function name: nvramRead

Description:   Returns the value located at the specified offset
               of the 128-byte serial EEPROM.
                
Side effects:  On entry:   none.
               On exit:    none

Returns:       (FxU8) the value requested
------------------------------------------------------------------------
FORMATTING:    Indention is 2 spaces, no tab characters.
----------------------------------------------------------------------*/
FxU8 nvramRead(NVRAMCONTEXT pContext, FxU8 offset)
{
  if (0 == NVRAM_DEVINFOPTR(pContext)->nvramCacheInit)
    nvramInitCache(pContext, I2C_NOTAKEY);

  return (FxU8)NVRAM_DEVINFOPTR(pContext)->nvramCache[offset];
}


/*----------------------------------------------------------------------
Function name: nvramWrite

Description:   Writes the specified values to the specified range of
               offsets of the 128-byte serial EEPROM.
                
Side effects:  On entry:   none.
               On exit:    none

Returns:       (int) non-zero on success, 0 on failure. 
------------------------------------------------------------------------
FORMATTING:    Indention is 2 spaces, no tab characters.
----------------------------------------------------------------------*/
int nvramWrite(NVRAMCONTEXT pContext, FxU8 offset, FxU32 numBytes, FxU8 *values)
{
  I2CKEY key;
  int status;

  key = i2c_getaccess(pContext, I2C_SERIALEEPROM, NV_ACCESSRATE);
  if (key == I2C_NOTAKEY)
    return (0);

  if (0 == NVRAM_DEVINFOPTR(pContext)->nvramCacheInit)
    nvramInitCache(pContext, key);

  status = internalNvramWrite(pContext, key, offset, numBytes, values, FALSE);

  i2c_endaccess(pContext, key);

  return status;
}

#undef NV_PRIVATE

