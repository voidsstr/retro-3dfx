/* $Header: runtime.c, 3, 10/11/00 8:52:19 PM, Brent$ */
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
** File Name:	RUNTIME.C
**
** Description: 
**
** $Revision: 3$
** $Date: 10/11/00 8:52:19 PM$
**
** $History: runtime.c $
** 
** *****************  Version 10  *****************
** User: Cwilcox      Date: 8/11/99    Time: 11:25a
** Updated in $/devel/h5/Win9x/dx/dd32
** Fixed problem that caused sign to be discarded in ddatof.
** 
** *****************  Version 9  *****************
** User: Pzheng       Date: 8/06/99    Time: 6:07p
** Updated in $/devel/h5/Win9x/dx/dd32
** Corrected a bug in ddftol and added dddtol that converts a double to
** long
** 
** *****************  Version 7  *****************
** User: Edwin        Date: 5/30/99    Time: 6:23p
** Updated in $/devel/h3/Win95/dx/dd32
** Remove ifdef MM, multi-monitor is always enabled.
** 
** *****************  Version 6  *****************
** User: Adrians      Date: 3/30/99    Time: 3:53p
** Updated in $/devel/h3/Win95/dx/dd32
** Remove a complier warning.
** 
** *****************  Version 5  *****************
** User: Michael      Date: 12/31/98   Time: 7:35a
** Updated in $/devel/h3/Win95/dx/dd32
** Implement the 3Dfx/STB unified header.
**
*/

#include <math.h>
#include <stdio.h>
#include "windows.h"
#include "precomp.h"
#include "ddglobal.h"
#include "pow.h"

#ifdef DEBUG
#pragma  optimize ("",off)
#else
#pragma  optimize ("",on)
#endif


/*----------------------------------------------------------------------
Function name: ddatoi

Description:   

Return:        int
----------------------------------------------------------------------*/
int ddatoi( const char *s )
{
   int i, n, sign;

   for( i=0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
     ;
     
   sign = 1;
   if (s[i] == '+' || s[i] == '-')
     sign = (s[i++] == '+') ? 1 : -1;
     
   for (n = 0; s[i] >= '0' && s[i] <= '9'; i++)
     n = 10 * n + s[i] - '0';
     
   return(sign * n);
}// ddatoi


/*----------------------------------------------------------------------
Function name: dd_itof_inv

Description:   Generate the reciprocal of i; return as float.

Return:        float the reciprocal of i
----------------------------------------------------------------------*/
float dd_itof_inv( int i )
{
   return( (float) 1.0 / (float) i );
}//  dd_itof_inv


/*----------------------------------------------------------------------
Function name: ddftol

Description:   

Return:        unsigned long
----------------------------------------------------------------------*/
unsigned long ddftol( float f )
{
   long       expnt, mantissa;
   BOOL       neg = FALSE;

   mantissa = expnt = *( long *) &f;

   // Special Case
   if( mantissa == 0 )
     return 0;

   if( expnt & 0x80000000 )
     neg = TRUE;

   // remove the exponent and add in implicit one
   mantissa &= 0x007fffff;
   mantissa |= 0x00800000;

   // exponent - remove mantissa and adjust for bias
   expnt &= 0x7F800000;
   expnt >>= 23;
   expnt -= 127;

   // shift by the exponent to get rid of the exponent. Need to use
   // 23 - expnt because expnt is number of posistions to the left of
   // decimal and we want to get rid of the posistions to the right of
   // the decimal.
   
//   mantissa >>= (23 - expnt);
//PingZ 8/5/99  The above line is wrong.  It only works if expnt is less than 23. If expnt is 
//greater than 23 it will convert matissa to zero!
//Here is the correct way to do it.

   expnt = 23 - expnt;

   if(expnt > 0) 
      mantissa >>= expnt;
   else
      mantissa <<= (-expnt);


   if( neg )
     mantissa = -mantissa;

   return mantissa;

   // return( (long) f );

}// ddftol


/*----------------------------------------------------------------------
Function name: dddtol   (Created by Ping Zheng 6/3/99)

Description:   convert double to long.  The trick is to only keep the upper 32bit of
the mantissa

Return:        unsigned long
----------------------------------------------------------------------*/
unsigned long dddtol( double f )
{
   long       expnt, mantissa;
   DWORD      trails;
   BOOL       neg = FALSE;

   trails = *( long *) &f;
   trails &= 0xffe00000;
   trails >>= 21;
   mantissa = expnt = *(long *)(( long *)&f + 1);

   if( expnt & 0x80000000 )
     neg = TRUE;

   // remove the exponent and add in implicit one
   mantissa &= 0x000fffff;
   mantissa |= 0x00100000;

   mantissa <<= 11;
   mantissa |= trails;

   // exponent - remove mantissa and adjust for bias

   expnt &= 0x7FF00000;
   expnt >>= 20;
   expnt -= 1023;

   expnt = 31 - expnt;

   if(expnt > 0) 
      mantissa >>= expnt;
   else
      mantissa <<= (-expnt);


   if( neg )
     mantissa = -mantissa;

   return mantissa;

   // return( (long) f );

}// dddftol

/*----------------------------------------------------------------------
Function name: ddatof

Description:   

Return:        double
----------------------------------------------------------------------*/
double ddatof( const char *s )
{
   float left, right, sign, mult;
   int i;

   for( i=0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
     ;

   if (s[i] == '+' || s[i] == '-')
     sign = (s[i++] == '+') ? 1.0f : -1.0f;
   else
     sign = 1.0f; 
 
   for (left = 0.0f; s[i] >= '0' && s[i] <= '9'; i++)
     left = (left * 10.0f) + (s[i] - '0');

   if (s[i++] != '.')
     return( sign * left );
     
   for (mult = 0.1f, right = 0.0f; s[i] >= '0' && s[i] <= '9'; i++, mult /= 10.0f)
     right = right + (mult * (s[i] - '0'));

   return (sign * (left + right));

   // return( atof(s) );     // Tests the runtime library
}// ddatof


/*----------------------------------------------------------------------
Function name: ddsscanf

Description:   

Return:        int
----------------------------------------------------------------------*/
int ddsscanf(char *s, const char *fmt, FxU32 *result)
{
   int i, n, sign;

   // XXX convert to lower case for safety, speed not important here
   for (i=0; '\0' != s[i]; i++)
      {
      if ((s[i]>='A') && (s[i]<='Z'))
         s[i]=s[i]-'A'+'a';
      }

   if ( fmt[0] == '%' && fmt[1] == 'i' )
   {
     for( i=0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
       ;
     sign = 1;
     if (s[i] == '+' || s[i] == '-')
       sign = (s[i++] == '+') ? 1 : -1;

     if (s[i] == '0' && s[i+1] == 'x' )    //HEX
     {
       i += 2;
       for (n = 0; ( (s[i] >= '0' && s[i] <= '9') ||
                     (s[i] >= 'a' && s[i] <= 'f') ); i++ )
       {
         if (s[i] >= '0' && s[i] <= '9')
            n = 16 * n + s[i] - '0';
         else
            n = 16 * n + (s[i] - 'a' + 10);
       }
       *result = (FxU32) (sign * n);
       return 0;
     }                                    //INTEGER
     else
     {
       for (n = 0; s[i] >= '0' && s[i] <= '9'; i++)
         n = 10 * n + s[i] - '0';
       *result = (FxU32) (sign * n);
       return 0;
     }
   }

  return -1;
}//  ddsscanf


/*----------------------------------------------------------------------
Function name: ddxtoa

Description:   

Return:        int
----------------------------------------------------------------------*/
int ddxtoa( DWORD val, char *result)
{
   // Postive Numbers Only

   DWORD   temp_val, rem;
   int     i=0;
   int     digits=0;

   temp_val = val;
   while(  (temp_val /= 16) > 0 )
      digits++;

   temp_val = val;

   result[digits+1] = '\n';

   for( i=digits; i>=0; --i)
   {
      rem = temp_val % 16;
      temp_val /= 16;

      if( rem >= 0 &&  rem <= 9)
      {
        result[i] = (char) rem;
        result[i] += '0';
      }
      else
      {
        result[i] = (char) (rem - 10);
        result[i] += 'a';
      }
   }
   return 1;
}// ddxtoa

/*----------------------------------------------------------------------
Function name: dditoa

Description:   

Return:        int
----------------------------------------------------------------------*/
int dditoa( int val, char *result)
{
   DWORD     temp_val, rem; // sign;
   int     i=0;
   int     digits=0;


   temp_val = (DWORD) val;
   while(  (temp_val /= 10) != 0 )
      digits++;


   temp_val = (DWORD) val;
   for( i=digits; i>=0; --i)
   {
      rem = temp_val % 10;
      temp_val /= 10;
      result[i] = (char) rem;
      result[i] += '0';
   }

   result[digits+1] = '\n';
   return 1;
}// dditoa


/*----------------------------------------------------------------------
Function name: ddxtoi

Description:   

Return:        int val
----------------------------------------------------------------------*/
int ddxtoi( DWORD val)
{
   DWORD   xin, nibble;
   int     i, sum=0;

   xin = val;
   sum = xin & 0xF;
   xin >>= 4;

   for( i=1; i<8; i++)
   {
      nibble = xin & 0xF;
      sum += (nibble * 16 * i);
      xin >>= 4;
   }
   return val;
}// ddxtoi


/*----------------------------------------------------------------------
Function name: ddpow

Description:   

Return:        float return value of pow_4_R
----------------------------------------------------------------------*/
float  ddpow( double x, double y)
{
    float fx, fy;

    fx = (float) x;
    fy = (float) y;
    return(  pow_4_R( fx , fy) );
}// ddpow


/*----------------------------------------------------------------------
Function name: regGetDouble

Description:   Query the registry for a keyword; if found, set *lpFl
               to the data associated with the keyword.

Return:        BOOL
               TRUE  - the keyword was found; *lpFl has valid data
			   FALSE - the keyword was not found; *lpFl is invalid
----------------------------------------------------------------------*/
BOOL regGetDouble( HKEY hkey, LPSTR keyname, double* lpFl )
{
    DWORD    type;
    DWORD    value;
    DWORD    size;
    float    f;

    // Don't worry about default if query fails, its handled higher up

    size = sizeof( value );
   
    if( !RegQueryValueEx( hkey, keyname, NULL, &type, (CONST LPBYTE)&value, &size ))
    {
        if( type == REG_DWORD )
        {
            // We are reading a DWORD that is really a FLOAT.

            f = *( float *) &value;

            *lpFl =  (double) f;
            return( TRUE );
        }
    }
    return( FALSE );
}// regGetDouble


/*----------------------------------------------------------------------
Function name: regGetDword

Description:   Query the registry for a keyword; if found,
               set *lpDw to the data associated with the keyword.

Return:        BOOL
               TRUE  - the keyword was found; *lpDw has valid data
			   FALSE - the keyword was not found; *lpDw is invalid
----------------------------------------------------------------------*/
BOOL regGetDword( HKEY hkey, LPSTR keyname, LPDWORD lpDw )
{
    DWORD    type;
    DWORD    value;
    DWORD    size;

    // Don't worry about default if query fails, its handled higher up

    size = sizeof( value );

    if( !RegQueryValueEx( hkey, keyname, NULL, &type, (CONST LPBYTE)&value, &size ))
    {
        if( type == REG_DWORD )
        {
            *lpDw = value;                              
            return( TRUE );
        }
    }
    return( FALSE );
}// regGetDword 


/*----------------------------------------------------------------------
Function name: int_exp

Description:   

Return:        float retval
----------------------------------------------------------------------*/
float  int_exp(float zbase, int zexp)
{
   int count;
   float retval=1.0f;

   if (zexp==0)
      return (1.0f);

   for(count=0; count<zexp;count++)
   {
      retval = (float)(retval*zbase);
   }
   
  return (retval);
}// int_exp


/*----------------------------------------------------------------------
Function name: MY_LN

Description:   

Return:        float retval
----------------------------------------------------------------------*/
float MY_LN( float u)
{
   float retval;
   float u_m_u0, df, df1, df2 , df3, df4;
  
   int index;
   float z_u0, temp;
   
   //  expand around the nearest 1/100
   index=FTOL(u*100);
   z_u0= ((float)(index))/100;

   if (z_u0==0.0f)
     z_u0+=0.01f;

   u_m_u0= u-z_u0;

   df= (u_m_u0/z_u0);

   df1= df;
   df2=df1 * df;
   df3=df2 * df;
   df4=df3 * df;
  
   temp=LN_HUN_TAB[index];
   retval = LN_HUN_TAB[index] + df1 - (df2/factoral[2]) + (2*df2/factoral[3])
     -(6*df3/factoral[4]);

   return(retval);
}// MY_LN


/*----------------------------------------------------------------------
Function name: pow_4_R

Description:   pow approximates the power of x to the y power
               using taylor series.  Pow handles 0<x<=4, any y
               that is a real number.

			   The taylor series approx of power function requires
               the natural log fuction which is also approximated
               by taylor series.  The natural log is approximated
               by expanding around the closest 1/100 real number.
               The natural log of 0+ to 4 increments of .01 are
               stored in a table.
			   
Return:        float retval
----------------------------------------------------------------------*/
float pow_4_R(float fbase, float fexp)
{
   int z_exp;
   float lnbase, lnbase1, lnbase2, lnbase3, lnbase4, y_m_y0;
   float retval, fexp_pos;
   float ay0;

//  restrict functions range.  if range is exceeded return a value
//  that can be used to calculate gamma   
//  No negative range

  if( fbase > 4.0f )
    return 1.4f;

  if( fbase < 0.0f )
    return 1.4f;

  if ( fexp == 0.0f )
    return 1.0f;
   
  if ( fexp < 0.0f )
    fexp_pos = -1 * fexp;
  else 
    fexp_pos=fexp ;
    z_exp = FTOL( fexp_pos+0.5f );
   
    ay0 = int_exp( fbase, z_exp );
   
    y_m_y0 = fexp_pos-z_exp;
   
    lnbase = MY_LN( fbase ) * y_m_y0;
    lnbase1 = lnbase * ay0;
    lnbase2 = lnbase1 * lnbase;
    lnbase3 = lnbase2 * lnbase; 
    lnbase4 = lnbase3 * lnbase; 

    retval = ay0 + lnbase1 +lnbase2/factoral[2] + lnbase3/factoral[3]   ;

    if( fexp > 0.0f )
      return( retval );
    else
      return( (1.0f/retval) );
}//  pow_4_R



