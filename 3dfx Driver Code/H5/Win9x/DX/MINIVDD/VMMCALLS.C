/* -*-c++-*- */
/* $Header: VMMCALLS.C, 2, 10/11/00 8:53:46 PM, Brent$ */
/*
** Copyright (c) 1999-1999, 3Dfx Interactive, Inc.
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
** File name:   vmmcalls.c
**
** Description: use extra vmmcalls that cannot be used if h3vdd.h is included
**
** $Revision: 2$
** $Date: 10/11/00 8:53:46 PM$
**
** $History: VMMCALLS.C $
** 
** *****************  Version 4  *****************
** User: Xingc        Date: 3/29/99    Time: 7:25a
** Updated in $/devel/h3/Win95/dx/minivdd
** Delete an int 3
** 
** *****************  Version 3  *****************
** User: Xingc        Date: 3/26/99    Time: 12:20p
** Updated in $/devel/h3/Win95/dx/minivdd
** Preserve ECX and EDX befor make VMMCALL
** 
** *****************  Version 2  *****************
** User: Xingc        Date: 3/25/99    Time: 5:47p
** Updated in $/devel/h3/Win95/dx/minivdd
** Add an int 3 for test
** 
** *****************  Version 1  *****************
** User: Xingc        Date: 3/24/99    Time: 10:09a
** Created in $/devel/h3/Win95/dx/minivdd
** Use extra VMMCalls that cannot be used if h3vdd.h is included
**
*/

#define WANTVXDWRAPS
#include <basedef.h>
#include <VXDWraps.h>
#include <VMM.h>
#include <debug.h>
#include <vwin32.h>

DWORD PhysicalToLinear( DWORD dwPhysical)
{
 DWORD dwRet;
   _asm push ecx         //keep ecx,edx
   _asm push edx
   _asm push 0
   _asm push 0x400       //one page only
   _asm push dwPhysical
   VMMCall (_MapPhysToLinear);
   _asm add  esp, 12
   _asm pop  edx
   _asm pop  ecx
   _asm mov    dwRet,eax

  return dwRet;
}

DWORD GetDBLinear(DWORD  DBLinear )
{
  DWORD dwRet;
  DWORD dwOffset;
  DWORD dwPhysAddr;
  DWORD dwLocked;
   //First find the physcal address of DBLinear
    _asm mov    eax, cr3
    _asm mov    dwRet, eax
    dwRet = PhysicalToLinear( dwRet & 0xFFFFF000);

    if( dwRet == 0xFFFFFFFF)
       return 1;      //Error
    //Now dwRet has the linear address of CR3

    dwOffset = DBLinear >> 22;
    dwRet +=dwOffset * 4;
    //find page table address -- a physical address
    dwOffset = * ((DWORD *)dwRet);

    dwRet = PhysicalToLinear( dwOffset & 0xFFFFF000);

    if( dwRet == 0xFFFFFFFF)
       return 2;

    dwOffset = (DBLinear & 0x3FF000) >> 12;
    dwRet +=dwOffset * 4;
    //find page start address -- a physical address
    dwPhysAddr = * ((DWORD *)dwRet);

    //Reserve on page of linear address to map to
    //this physical address too.
    _asm  push  ecx         //keep ecx 
    _asm  push  edx         //keep edx 
    _asm  push  PR_FIXED
    _asm  push  1
    _asm  push  PR_SHARED
    VMMCall (_PageReserve);
    _asm  add  esp, 12
    _asm  pop  edx
    _asm  pop  ecx
    _asm  mov   dwRet,eax

    if( dwRet == 0xFFFFFFFF)
       return 3;

    dwLocked = dwRet;
    dwPhysAddr >>= 12;
    dwRet >>= 12;

     _asm push  ecx         //keep ecx 
     _asm push  edx         //keep edx 
     _asm push  PC_USER|PC_WRITEABLE
     _asm push  dwPhysAddr
     _asm push  1
     _asm push  dwRet
     VMMCall (_PageCommitPhys);
     _asm  add  esp, 16
     _asm  pop  edx
     _asm  pop  ecx
     _asm mov dwRet ,eax

     if( !dwRet )
         return 4;

     //Page lock it
      _asm push ecx
      _asm push PAGEMAPGLOBAL
      _asm push 1
      _asm mov  eax, dwLocked
      _asm shr  eax, 12
      _asm push eax
      VMMCall (_LinPageLock);
     _asm  add  esp, 12
     _asm  pop  ecx

      dwLocked |= DBLinear & 0xFFF;

      return dwLocked;

}

DWORD GetExtraAddr(DIOCPARAMETERS * lpParams)
{
   if( (lpParams->cbOutBuffer >= 4) &&
       ( lpParams->cbInBuffer >= 4))
   {
      *(DWORD *)lpParams->lpvOutBuffer = GetDBLinear( *(DWORD *)lpParams->lpvInBuffer);
      return 0;
    }
    else
      return 1;
}
