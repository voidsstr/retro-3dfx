/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** File name:   fxf2i.c
**
** Description: fast float to int conversion
**
** $Revision: 2$
** $Date: 10/11/00 8:49:20 PM$
**
** $Log: 
**  2    3dfx      1.0.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 2     8/06/99 6:09p Pzheng
** Corrected a bug in float to int conversion routine
** 
** 1     6/02/99 6:46a Michael
** Branch from H3
** 
** 7     1/30/99 11:02p Adrians
** Fixed a bug in float2int whereby the function, when passed a very small
** number, was trying to shift by greater than 32 bits which will not
** work.
** 
** 6     1/26/99 5:29p Peterm
** Added unified header information
** 
** 5     12/09/98 6:59a Russ
** NT5 D3D changes for Banshee
**
** 4     7/24/98 1:37p Hohn
**
** 3     5/06/98 6:10p Adrians
** Changes for DX6 into DX5 driver.
**
** 1     4/29/98 6:31p Adrians
** Created
 *
 * 2     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/

#include "precomp.h"

#ifndef WINNT
#include <windows.h>
#endif

/*-------------------------------------------------------------------
Function Name:  float2int

Description:    fast float to int conversion

Return:         int
-------------------------------------------------------------------*/
int float2int(float f)
{
  int  expnt, mantissa;
  BOOL neg = FALSE;
  
  mantissa = expnt = *(int *)&f;

//  D3DPRINT( 255,"float2Int float=0x%x ",*(int *)&f );

  // special case
  if( mantissa == 0 )
    return 0;

  if( expnt & 0x80000000 )
    neg = TRUE;

  // remove the exponent and add in implicit one
  mantissa = (mantissa & 0x007fffff) | 0x00800000;

  // exponent - remove mantissa and adjust for bias
  expnt = 23 - (((expnt & 0x7F800000) >> 23) - 127);

  // shift by the exponent to get rid of the exponent. Need to use
  // 23 - expnt because expnt is number of positions to the left of
  // decimal and we want to get rid of the positions to the right of decimal
/*
  if( expnt > 31 )
    mantissa = 0;
  else
    mantissa = mantissa >> expnt;
*/
//PingZ 8/5/99  The above line is wrong.  It only works if expnt is less than 23. If expnt is 
//greater than 23 but less than 31 it will convert matissa to zero!
//Here is the correct way to do it.

   if(expnt > 0) 
      mantissa >>= expnt;
   else
      mantissa <<= (-expnt);

  if (neg)
    mantissa = -mantissa;

//  D3DPRINT( 0,"float2Int int=%d ",mantissa );

  return mantissa;
}

