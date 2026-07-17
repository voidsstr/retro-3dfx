/* $Header: i2cmacro.h, 3, 10/11/00 8:55:47 PM, Brent$ */
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
** File name:   i2cmacro.h
**
** Description: Provides macros that hide OS-dependencies from the ds/di
**              I2C implementation.
**
** $Log: 
**  3    3dfx      1.1.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.1         09/22/99 Ryan Bissell    I2C updates  (multi-monitor
**       fixes)
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
**
*/

#ifndef __I2CMACRO_H__
#define __I2CMACRO_H__

#include "3dfx.h"
#include "fxpci.h" // needed to resolve compilation errors
#include "h3defs.h"

#define VDDONLY
#include "h3vdd.h"
#include "h3regs.h"
#include "devtable.h"
#include "ds_i2c.h"



#define I2C_DEVINFOPTR(pcontext)  ((PI2C_DEVINFO)&((PDEVTABLE)(pcontext))->i2cInfo)



#ifdef WIN_CSIM
  #define I2C_REGBASEPTR(pcontext) ((SstIORegs*)((PDEVTABLE)(pcontext))->RealRegBase)
#else
  #define I2C_REGBASEPTR(pcontext) ((SstIORegs*)((PDEVTABLE)(pcontext))->RegBase[HWINFO_SST_IOREGS_INDEX])
#endif



#define I2C_GETBUSINFO(pcontext, busid) (&(((PDEVTABLE)(pcontext))->i2cInfo.BusList[(busid)-1]))

/*
static void i2c_delaytime(FxU32 usecs)
{
   CM_Yield(usecs, CM_YIELD_RESUME_EXEC);
}

#define I2C_DELAYTIME(usecs) i2c_delaytime((usecs))
*/

#define I2C_DELAYTIME(usecs) CM_Yield((usecs), CM_YIELD_RESUME_EXEC);


static int IsVoodoo3(I2CCONTEXT pContext)
{
  PDEVTABLE pDev = (PDEVTABLE)pContext;
  return (pDev ? IS_VOODOO3(pDev->dwVendorDeviceID) : 0);
}


#ifdef DEBUG
#define DEBUGI2C(expr, args)  { if (expr) _Debug_Printf_Service##args; }
#else
#define DEBUGI2C(foo, bar)
#endif

#endif // __I2CMACRO_H__