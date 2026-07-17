/* -*-c++-*- */
/* $Header: cmclient.h, 2, 10/11/00 8:53:59 PM, Brent$ */
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
** File name:   cmclient.c
**
** Description:  The CMINFO structure and related flags.
**
** $Revision: 2$
** $Date: 10/11/00 8:53:59 PM$
**
** $History: cmclient.h $
** 
** *****************  Version 3  *****************
** User: Michael      Date: 1/04/99    Time: 4:46p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

//: cmclient.h


#ifndef __CMCLIENT_H__
#define __CMCLIENT_H__


// flags for CMINFO.ulFlags

#define CMI_DIRTY   0x00000001L
#define CMI_SRCXY   0x80000000L


// When a DIBENGINE structure has both VRAM & OFFSCREEN
// bits set in deFlags, deReserved points to a CMINFO
// structure.

typedef struct
{
    DWORD dwPrivate;    // you will go to hell if you touch this
    DWORD dwFlags;      // if dst, you must do: ulFlags |= CMI_DIRTY;
    DWORD dwBaseAddr;   // write to (dst|src)BaseAddr
    DWORD dwFormat;     // write to (dst|src)Format
    DWORD dwSrcXY;      // add to srcXY if (ulFlags & CMI_SRCXY)
}
CMINFO, FAR* LPCMINFO;


#endif
