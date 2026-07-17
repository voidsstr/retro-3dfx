/* $Header: dbg.c, 2, 10/11/00 8:50:53 PM, Brent$ */
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
** File name:   dbg.c
**
** Description: Debug support functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:50:53 PM$
**
** $History: dbg.c $
** 
** *****************  Version 4  *****************
** User: Michael      Date: 12/28/98   Time: 11:23a
** Updated in $/devel/h3/Win95/dx/dd16
** Added the 3Dfx/STB unified file/funciton header.  Add VSS keywords to
** files where this was missing.
**
*/

#include "header.h"
typedef unsigned char UCHAR;

/*----------------------------------------------------------------------
Function name:  mysprintf

Description:    Formatting for debugging output for the driver and WGL.

Information:    Similar to Standard vsprintf
                %X or %x    Hex format of a UINT
                %d          Decimal format of a UINT
                %D          Decimal format of a ULONG
                %L or %l    Hex format of a ULONG
                %s or %S    Ascii Zero terminated "C" string output
                %c or %C    Single character (parameter is a UCHAR or UINT)
                Used for debugging only!

Return:         Formatted string into first parameter
----------------------------------------------------------------------*/

#define MAXCHARSTRLEN 100 /* Limit length to prevent buffer overflow */

void far mysprintf(char far *result, char far * str,int far *param)
{
   char far *charparam;
   char far *p = result;          // initial buffer pointer
   static char hexstr[] = "0123456789ABCDEF";
   #define POW_SIZE	 (sizeof(pow)/sizeof(long))
   static long pow[] = { 1, 
                         10, 
                         100, 
                         1000, 
                         10000, 
                         100000,
                         1000000,
                         10000000,
                         100000000,
                         1000000000};

   int temp,i;
   unsigned long ltemp;
   long dtemp;
   while (*str) 
   {
      if (*str != '%') 
      {
         if (*str == '\n') *p++ = '\r';     // 'C' style newline handling
         *p++ = *str++;
      } 
      else 
      {
         str++;                                 // Skip '%'
         switch (*str) 
         {
            case 'x':                            // Hex output (2 bytes)
            case 'X':                            // Hex output
               temp = *param;                      // Fetch parameter
               param++;
               *p++ = hexstr[(temp >> 12) & 0xF];
               *p++ = hexstr[(temp >>  8) & 0xF];
               *p++ = hexstr[(temp >>  4) & 0xF];
               *p++ = hexstr[(temp      ) & 0xF];
               str ++;
               break;
            case 'l':
            case 'L':                           // Handle long (assume integer)
               ltemp = *(ULONG far *)param;
               param += sizeof(ULONG)/sizeof(int);
               *p++ = hexstr[(ltemp >> 28) & 0xF];
               *p++ = hexstr[(ltemp >> 24) & 0xF];
               *p++ = hexstr[(ltemp >> 20) & 0xF];
               *p++ = hexstr[(ltemp >> 16) & 0xF];
               *p++ = hexstr[(ltemp >> 12) & 0xF];
               *p++ = hexstr[(ltemp >>  8) & 0xF];
               *p++ = hexstr[(ltemp >>  4) & 0xF];
               *p++ = hexstr[(ltemp      ) & 0xF];
               str ++;
               break;
            case 'p':
            case 'P':                 // Handle pointer (segment:offset)
               ltemp = *(ULONG far *)param;
               param += sizeof(ULONG) / sizeof(int);
               *p++ = hexstr[(ltemp >> 28) & 0xF];
               *p++ = hexstr[(ltemp >> 24) & 0xF];
               *p++ = hexstr[(ltemp >> 20) & 0xF];
               *p++ = hexstr[(ltemp >> 16) & 0xF];
               *p++ = ':';
               *p++ = hexstr[(ltemp >> 12) & 0xF];
               *p++ = hexstr[(ltemp >>  8) & 0xF];
               *p++ = hexstr[(ltemp >>  4) & 0xF];
               *p++ = hexstr[(ltemp      ) & 0xF];
               str ++;
               break;
            case 'c':   // Char, 1 byte
            case 'C':
               temp = *param;                              // Fetch parameter
               param++;
               *p++ = hexstr[(temp >>  4) & 0xF];
               *p++ = hexstr[(temp      ) & 0xF];
               str ++;
               break;
            case 's':
            case 'S':
               charparam = *(char far * far*)param;
               param += sizeof(char far *)/sizeof(int);
               if (charparam == 0) 
               {
                  *p++ = '(';
                  *p++ = 'N';
                  *p++ = 'U';
                  *p++ = 'L';
                  *p++ = 'L';
                  *p++ = ')';
               }
               else 
               {
                   i = 0;
                   while (*charparam && (i < MAXCHARSTRLEN)) 
                   {
                      *p++ = *charparam++;
                      i++;
                   }
                }
                str ++;
		    	break;
			 case 'd':
                dtemp =(long) *param;              // Fetch parameter
                param++;
                goto do_decimal;
			 case 'D':
                dtemp = *(long far *)param;
                param += sizeof(ULONG)/sizeof(int);
do_decimal:
                str++;
                if (dtemp < 0) 
                {
                    dtemp = -dtemp;
                    *p++ = '-';
                }
                if (dtemp == 0) 
                {
                    *p++ = '0';
                } else 
                {
                    for (i = 1; i < POW_SIZE-1 && dtemp >= pow[i]; ++i) {}
                    do 
                    {
                        i--;
                        *p++ = '0' + (UCHAR)(dtemp / pow[i]);
                        dtemp = dtemp % pow[i];
                     } while (i > 0);
                 }
                 break;
         default:                      // Unknown % case, just output it
            *p++ = *str++;
            break;
         }
      }
   }
   *p = 0;
}


/*----------------------------------------------------------------------
Function name:  dprintf

Description:    Debugging output for the driver and WGL.

Information:    Similar to fprintf
                Used for debugging only!

Return:         VOID
----------------------------------------------------------------------*/
void far dprintf(char far *str,...) {
   char buffer[1024];
   int far *param = (int far *)(1+&str);      // First parameter on the stack
   mysprintf(buffer,str,param);
   OutputDebugString(buffer);
}


/*----------------------------------------------------------------------
Function name:  sprintf

Description:    String Interface to vsprintf.

Information:    Similar to sprintf
                Used for debugging only!

Return:         VOID
----------------------------------------------------------------------*/
void far sprintf1(char far *result, char far *str, ...) {
   int far *param = (int far *)(1 + &str);
   mysprintf(result,str,param);
}


/*----------------------------------------------------------------------
Function name:  decodebrushflag

Description:    Print (via debug functions) brush type based on lpBrush.

Information:    Used for debugging only!

Return:         VOID
----------------------------------------------------------------------*/
void decodebrushflag( DIB_Brush8 * lpBrush)
{

	if ( lpBrush->dp8BrushFlags & COLORSOLID) 
	{
		DPF(DBGLVL_NORMAL, "Brush:  COLORSOLID");
	}

	if ( lpBrush->dp8BrushFlags & MONOSOLID) 
	{
		DPF(DBGLVL_NORMAL, "Brush:  MONOSOLID");
	}	

	if ( lpBrush->dp8BrushFlags & PATTERNMONO) 
	{
		DPF(DBGLVL_NORMAL, "Brush:  PATTERNMONO");
	}	

	if ( lpBrush->dp8BrushFlags & MONOVALID) 
	{
		DPF(DBGLVL_NORMAL, "Brush:  MONOVALID");
	}	

	if ( lpBrush->dp8BrushFlags & MASKVALID) 
	{
		DPF(DBGLVL_NORMAL, "Brush:  MASKVALID");
	}	

	if ( lpBrush->dp8BrushFlags & PRIVATEDATA) 
	{
		DPF(DBGLVL_NORMAL, "Brush:  PRIVATEDATA");
	}	

}	


/*----------------------------------------------------------------------
Function name:  decodebrushstyle

Description:    Print (via debug functions) brush style based on lpBrush.

Information:    Used for debugging only!

Return:         VOID
----------------------------------------------------------------------*/
void decodebrushstyle (WORD style)
{
	switch(style)
	{
		case BS_SOLID:
			DPF(DBGLVL_NORMAL, " BS_SOLID ");
			break;
		case BS_NULL:
			DPF(DBGLVL_NORMAL, " BS_NULL/BS_HOLLOW ");
			break;
		case BS_HATCHED:
			DPF(DBGLVL_NORMAL, " BS_HATCHED ");
			break;
		case BS_PATTERN:
			DPF(DBGLVL_NORMAL, " BS_PATTERN ");
			break;
		case BS_INDEXED: 
			DPF(DBGLVL_NORMAL, " BS_INDEXED ");
			break;
		case BS_DIBPATTERN:
			DPF(DBGLVL_NORMAL, " BS_DIBPATTERN ");
			break;
		case BS_DIBPATTERNPT:
			DPF(DBGLVL_NORMAL, " BS_DIBPATTERNPT ");
			break;
		case BS_PATTERN8X8:
			DPF(DBGLVL_NORMAL, " BS_PATTERN8X8 ");
			break;
		case BS_DIBPATTERN8X8:
			DPF(DBGLVL_NORMAL, " BS_DIBPATTERN8X8 ");
			break;
	}
}

