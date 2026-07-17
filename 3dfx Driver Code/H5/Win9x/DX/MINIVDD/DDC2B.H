/* -*-c++-*- */
/* $Header: ddc2b.h, 4, 10/11/00 8:54:01 PM, Brent$ */
/*
** Copyright (c) 1995-1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   ddc.c
**
** Description: DDC macros andsupport.
**
** $Revision: 4$
** $Date: 10/11/00 8:54:01 PM$
**
** $History: ddc2b.h $
** 
** *****************  Version 6  *****************
** User: Rbissell     Date: 9/07/99    Time: 2:15p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Deployed the new modularized I2C code.
** 
** *****************  Version 4  *****************
** User: Michael      Date: 1/04/99    Time: 4:46p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 3  *****************
** User: Andrew       Date: 10/05/98   Time: 3:59p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added prototype
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#ifndef _DDC2B_H_
#define _DDC2B_H_

#include "i2c/di_i2c.h"

#include "ddcdefs.h"

#define FIELDOFFSET(type, field) ((DWORD)(&((type *)0)->field))

#define CLIENT_PTR_FLAT(seg, off) DoMapFlat(\
   FIELDOFFSET(struct Client_Reg_Struc, Client_##seg), \
   FIELDOFFSET(struct Client_Word_Reg_Struc, Client_##off))

int DDCSupport(PDEVTABLE pDevTable, PCRS pCR);
int ReadDDC(PDEVTABLE pDev, I2CKEY key, int nAddr, PBYTE pBuffer, int nSize);

#endif
