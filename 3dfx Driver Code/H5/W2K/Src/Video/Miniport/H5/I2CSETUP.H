/* $Header: i2csetup.h, 3, 10/11/00 8:47:15 PM, Brent$ */
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
** File name:   i2csetup.h
**
** Description: Provides config overrides for ds/di i2c implementation.
**
** $Log: 
**  3    3dfx      1.1.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.1         02/17/00 Dan O'Connel    The V3TV checkin to create
**       a I2C_MULTIMEDIA usage commented out several SetUsageI2C calls and thereby
**       broke TvOut.  The underlying problem was the table sized by
**       I2C_MAXPURPOSES was full.  This fix increases the table size from 5 to 7
**       and reinstates the commented out code.
**  1    3dfx      1.0         09/22/99 Ryan Bissell    
** $
**
*/


#ifndef __I2CSETUP_H__
#define __I2CSETUP_H__


// The purpose of this file is to specify configuration overrides.

#define I2C_PEDANTIC    1

#define I2C_MAXBUSES    3
#define I2C_MAXMUXEN    1
#define I2C_MAXPURPOSES 7


#endif //__I2CSETUP_H__