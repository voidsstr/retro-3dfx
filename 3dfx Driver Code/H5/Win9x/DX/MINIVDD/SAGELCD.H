/* -*-c++-*- */
/* $Header: sagelcd.h, 2, 10/11/00 8:55:02 PM, Brent$ */
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
** File name:   sagelcd.h
**
** Description: BrookTree TV Out file.
**
** $Revision: 2$
** $Date: 10/11/00 8:55:02 PM$
**
** $History: sagelcd.h $
** 
** *****************  Version 2  *****************
** User: Michael      Date: 1/15/99    Time: 6:35a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
*/

#ifndef _SAGELCD_H_
#define _SAGELCD_H_

/********************************************************************************
*
*
*
*********************************************************************************/

//LCD Out section for now
#define LCDPLL_ADDR          (0x88)


//SerialParallel port register defines
#define H3_SERIALPARALLEL_HOSTPORT  0x00000001
#define H3_SERIALPARALLEL_VMICSN    0x00000002
#define H3_SERIALPARALLEL_VMIDSN    0x00000004
#define H3_SERIALPARALLEL_VMIRWN    0x00000008
#define H3_VMI_DATAOUT_DISABLE      0x00000020
#define H3_SERIALPARALLEL_VMI       0x10000000
#define H3_SERIALPARALLEL_GPIO1     0x20000000
#define H3_HPORT_MASK               0x0003ffc0
#define H3_HPORT_WR                 0x00008000
#define H3_HPORT_RD                 0x00010000
#define H3_HPORT_ALE                0x00020000

//56Hz SVGA register values
WORD wSageRegData[] =
{
   0xA7,    0x00,
   0x10,    0x00,
   0x11,    0x00,
   0x12,    0x12,
   0x41,    0x00,
   0x40,    0xfd,
   0xA3,    0xff,
   0xA4,    0x00,
   0xA5,    0xff,
   0xA6,    0x00,
   0xBC,    0x00,
   0xBD,    0x00,
   0xC4,    0x00,
   0xC5,    0x00,
   0xC6,    0x00,
   0xC7,    0x00,
   0xC0,    0x00,
   0xC1,    0x00,
   0xC2,    0xff,
   0xC3,    0x00,
   0xC8,    0x00,
   0xC9,    0x00,
   0xCA,    0x00,
   0xCB,    0x00,
   0x48,    0xff,
   0x49,    0x00,
   0x4A,    0x00,
   0x4B,    0x00,
   0x6A,    0x20,
   0x6B,    0x00,
   0x6C,    0x00,
   0x6D,    0x00,
   0x6E,    0x00,
   0xCC,    0x00,
   0xCD,    0x00,
   0x86,    0x00,
   0x87,    0x00,
   0x88,    0x00,
   0x89,    0x00,
   0x8A,    0x00,
   0x8B,    0x00,
   0x54,    0x00,
   0x55,    0x00,
   0x58,    0x00,
   0x59,    0x00,
   0x56,    0x00,
   0x57,    0x00,
   0x5A,    0x00,
   0x5B,    0x00,
   0x50,    0x00,
   0x51,    0x00,
   0x52,    0x00,
   0x53,    0x00,
   0x69,    0x00,
   0x80,    0xFF,
   0x9E,    0x60,
   0xCE,    0xF0,
   0x64,    0x00
};
#endif
