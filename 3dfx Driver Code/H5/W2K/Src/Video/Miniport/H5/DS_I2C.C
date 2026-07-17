/* $Header: ds_i2c.c, 7, 10/11/00 8:47:05 PM, Brent$ */
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
** File name:   ds_i2c.c
**
** Description: Contains the device-specific portion of the ds/di
**              I2C implementation.
**
** $Log: 
**  7    3dfx      1.5.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  6    3dfx      1.5         02/17/00 Dan O'Connel    The V3TV checkin to create
**       a I2C_MULTIMEDIA usage commented out several SetUsageI2C calls and thereby
**       broke TvOut.  The underlying problem was the table sized by
**       I2C_MAXPURPOSES was full.  This fix increases the table size from 5 to 7
**       and reinstates the commented out code.
**  5    3dfx      1.4         02/09/00 Lauren Post     V3TV Win2k Enablement fixes
**  4    3dfx      1.3         01/16/00 Steve Rogers    Fixing DDC detection for
**       Windows
**  3    3dfx      1.2         10/28/99 Christopher Wilcox Hardware definition
**       changes to merge divergent h3defs.h.
** 
**  2    3dfx      1.1         09/22/99 Ryan Bissell    I2C updates  (multi-monitor
**       fixes)
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
**
*/


#include "i2cmacro.h"
#include "di_i2c.h"
#include "ds_i2c.h"


// None of these may equate to zero!!!
#define MUX_I2CDEVICES    1   //Avenger only
#define MUX_MONITORDDC    2   //Napalm only
#define MUX_FLATPANELDDC  3   //Avenger and Napalm



int MUX_Napalm(I2CCONTEXT pContext, FxU32 value)
{
  //RYAN@TODO,990902:  The polarity and the actual bit for this mux is not yet known.
  //srogers,000116: It looks like from the schematics, I've seen, we should at least
  // set the DDC Enable on.  Also, the schematics say to set GPIO_1 to 0 for monitor
  // detection and set GPIO_1 to 1 for FLATPANEL detection
  I2C_REGBASEPTR(pContext)->vidSerialParallelPort |= SST_SERPAR_DDC_EN;
  switch (value)
  {
    case MUX_MONITORDDC:
      I2C_REGBASEPTR(pContext)->vidSerialParallelPort &= ~SST_SERPAR_GPIO_1;
      DEBUGI2C(1, ("I2C: i2c_getaccess(\"Access granted for MONITOR DDC purposes.\")\n"));
      break;

    case MUX_FLATPANELDDC:
      I2C_REGBASEPTR(pContext)->vidSerialParallelPort |= SST_SERPAR_GPIO_1;
      DEBUGI2C(1, ("I2C: i2c_getaccess(\"Access granted for FLATPANEL DDC purposes.\")\n"));
      break;

    default:
      return I2C_FAILURE;
  }

  return I2C_SUCCESS;
}



int MUX_Avenger(I2CCONTEXT pContext, FxU32 value)
{
  switch (value)
  {
    case MUX_MONITORDDC:
      // this is not really a mux; just ensures the port is enabled.
      I2C_REGBASEPTR(pContext)->vidSerialParallelPort |= SST_SERPAR_DDC_EN;
      DEBUGI2C(1, ("I2C: i2c_getaccess(\"Access granted for MONITOR DDC purposes.\")\n"));
      break;

    case MUX_I2CDEVICES:
      I2C_REGBASEPTR(pContext)->vidSerialParallelPort |= SST_SERPAR_I2C_EN;  //ensure port is enabled
      I2C_REGBASEPTR(pContext)->vidSerialParallelPort &= ~SST_SERPAR_GPIO_1;
      DEBUGI2C(1, ("I2C: i2c_getaccess(\"Access granted for GENERAL I2C purposes.\")\n"));
      break;

    case MUX_FLATPANELDDC:
      I2C_REGBASEPTR(pContext)->vidSerialParallelPort |= SST_SERPAR_I2C_EN;  //ensure port is enabled
      I2C_REGBASEPTR(pContext)->vidSerialParallelPort |= SST_SERPAR_GPIO_1;
      DEBUGI2C(1, ("I2C: i2c_getaccess(\"Access granted for FLATPANEL DDC purposes.\")\n"));
      break;

    default:
      return I2C_FAILURE;
  }

  DEBUGI2C(1, ("I2C: i2c_getaccess(\"I2C error-detect mode is %s.\")\n", I2C_PEDANTIC ? "PEDANTIC" : "LAX"));
  return I2C_SUCCESS;
}



FxU8 DDC_GetSDA(I2CCONTEXT pContext)
{
  return (FxU8)!!(I2C_REGBASEPTR(pContext)->vidSerialParallelPort & SST_SERPAR_DDC_DDA_IN);
}



FxU8 DDC_GetSCL(I2CCONTEXT pContext)
{
  return (FxU8)!!(I2C_REGBASEPTR(pContext)->vidSerialParallelPort & SST_SERPAR_DDC_DCK_IN);
}



void DDC_SetSDA(I2CCONTEXT pContext, int state)
{
  FxU32 value;
  volatile FxU32* pRegister;

  // since "I2C_REGBASEPTR" could be a function, only use it once.
  pRegister = &(I2C_REGBASEPTR(pContext)->vidSerialParallelPort);

  value = *pRegister;
  value &= ~SST_SERPAR_DDC_DDA_OUT;
  value |= (state ? SST_SERPAR_DDC_DDA_OUT : 0);
  *pRegister = value;
}


void DDC_SetSCL(I2CCONTEXT pContext, int state)
{
  FxU32 value;
  volatile FxU32* pRegister;

  // since "I2C_REGBASEPTR" could be a function, only use it once.
  pRegister = &(I2C_REGBASEPTR(pContext)->vidSerialParallelPort);

  value = *pRegister;
  value &= ~SST_SERPAR_DDC_DCK_OUT;
  value |= (state ? SST_SERPAR_DDC_DCK_OUT : 0);
  *pRegister = value;
}



FxU8 I2C_GetSDA(I2CCONTEXT pContext)
{
  return (FxU8)!!(I2C_REGBASEPTR(pContext)->vidSerialParallelPort & SST_SERPAR_I2C_DSA_IN);
}



FxU8 I2C_GetSCL(I2CCONTEXT pContext)
{
  return (FxU8)!!(I2C_REGBASEPTR(pContext)->vidSerialParallelPort & SST_SERPAR_I2C_SCK_IN);
}



void I2C_SetSDA(I2CCONTEXT pContext, int state)
{
  FxU32 value;
  volatile FxU32* pRegister;

  // since "I2C_REGBASEPTR" could be a function, only use it once.
  pRegister = &(I2C_REGBASEPTR(pContext)->vidSerialParallelPort);

  value = *pRegister;
  value &= ~SST_SERPAR_I2C_DSA_OUT;
  value |= (state ? SST_SERPAR_I2C_DSA_OUT : 0);
  *pRegister = value;
}



void I2C_SetSCL(I2CCONTEXT pContext, int state)
{
  FxU32 value;
  volatile FxU32* pRegister;

  // since "I2C_REGBASEPTR" could be a function, only use it once.
  pRegister = &(I2C_REGBASEPTR(pContext)->vidSerialParallelPort);

  value = *pRegister;
  value &= ~SST_SERPAR_I2C_SCK_OUT;
  value |= (state ? SST_SERPAR_I2C_SCK_OUT : 0);
  *pRegister = value;
}



int ConfigureI2C(I2CCONTEXT pContext)
{
  I2CBUS busid;
  I2CMUX muxid;


  if (IsVoodoo3(pContext))  // RYAN@HACK, since we have to support two different board designs.
  {
    muxid = InstantiateMuxI2C(pContext, MUX_Avenger);

    busid = InstantiateBusI2C(pContext, DDC_GetSDA, DDC_GetSCL, DDC_SetSDA, DDC_SetSCL);
    SetUsageI2C(pContext, busid, I2C_MONITORDDC);
    AssociateBusToMuxI2C(pContext, busid, muxid, MUX_MONITORDDC); //last param cannot be zero
    
    busid = InstantiateBusI2C(pContext, I2C_GetSDA, I2C_GetSCL, I2C_SetSDA, I2C_SetSCL);
    SetUsageI2C(pContext, busid, I2C_FLATPANELDDC);
    AssociateBusToMuxI2C(pContext, busid, muxid, MUX_FLATPANELDDC); //last param cannot be zero 

    busid = InstantiateBusI2C(pContext, I2C_GetSDA, I2C_GetSCL, I2C_SetSDA, I2C_SetSCL);
	SetUsageI2C(pContext, busid, I2C_MULTIMEDIA);
    SetUsageI2C(pContext, busid, I2C_TVENCODER);
    SetUsageI2C(pContext, busid, I2C_LCDSCALER);
    SetUsageI2C(pContext, busid, I2C_SERIALEEPROM);
    AssociateBusToMuxI2C(pContext, busid, muxid, MUX_I2CDEVICES); //last param cannot be zero
  }
  else
  {
    muxid = InstantiateMuxI2C(pContext, MUX_Napalm);

    busid = InstantiateBusI2C(pContext, DDC_GetSDA, DDC_GetSCL, DDC_SetSDA, DDC_SetSCL);
    SetUsageI2C(pContext, busid, I2C_MONITORDDC);
    AssociateBusToMuxI2C(pContext, busid, muxid, MUX_MONITORDDC); //last param cannot be zero 

    busid = InstantiateBusI2C(pContext, DDC_GetSDA, DDC_GetSCL, DDC_SetSDA, DDC_SetSCL);
    SetUsageI2C(pContext, busid, I2C_FLATPANELDDC);
    AssociateBusToMuxI2C(pContext, busid, muxid, MUX_FLATPANELDDC); //last param cannot be zero

    busid = InstantiateBusI2C(pContext, I2C_GetSDA, I2C_GetSCL, I2C_SetSDA, I2C_SetSCL);
    SetUsageI2C(pContext, busid, I2C_TVENCODER);
    SetUsageI2C(pContext, busid, I2C_LCDSCALER);
    SetUsageI2C(pContext, busid, I2C_SERIALEEPROM);
	SetUsageI2C(pContext, busid, I2C_MULTIMEDIA);
  }

  
  return I2C_SUCCESS;
}






