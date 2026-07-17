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
** File name:   fxexp.c
**
** Description: exponential routines
**
** $Revision: 2$
** $Date: 10/11/00 8:42:22 PM$
**
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 1     6/02/99 6:46a Michael
** Branch from H3
** 
** 6     1/26/99 5:29p Peterm
** Added unified header information
** 
** 5     12/09/98 6:59a Russ
** NT5 D3D changes for Banshee
**
** 4     7/24/98 1:37p Hohn
 *
 * 3     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/

#include "precomp.h"

#define EXP_THRESHOLD 1.0e-20
#define LN_2          0.6931471805599

double two_to_x(double x);
double fxExp(double x);

/*-------------------------------------------------------------------
Function Name:  two_to_x

Description:    returns two raised to the x power

Return:         double
-------------------------------------------------------------------*/
double two_to_x(double x)
{
  return(fxExp(x*LN_2));
}

/*-------------------------------------------------------------------
Function Name:  fxExp

Description:    returns approximation to exp(x)

Return:         double
-------------------------------------------------------------------*/
double fxExp(double x)
{
  double ratio=x, result=1.0,n=1.0,test;

  // some really small number close to zero
  if (x < -30)
    return(0.000000000001);

  do
    {
      result+=ratio;
      n+=1.0;
      ratio=ratio*x/n;
      test=ratio/result;
      if(test<0.0)
        test=-test;
    } while(test>EXP_THRESHOLD);
  return(result);
}


