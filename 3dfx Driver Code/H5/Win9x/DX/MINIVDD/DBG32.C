/* -*-c++-*- */
/* $Header: dbg32.c, 2, 10/11/00 8:54:00 PM, Brent$ */
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
** File name:   dbg32.c
**
** Description: Debugging functions and support.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:00 PM$
**
** $History: dbg32.c $
** 
** *****************  Version 15  *****************
** User: Michael      Date: 1/04/99    Time: 4:46p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 14  *****************
** User: Ken          Date: 7/18/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added ability to use cmdfifo1 as the primary command fifo, #define
** PRIMARY_CMDFIFO at the top of inc\shared.h
** 
** *****************  Version 13  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#include "h3.h"
#include "thunk32.h"
#include "h3g.h"
typedef unsigned char UCHAR;

extern CallOutputDebugString(DWORD buf, DWORD MsgAddr);
extern DWORD pollCount;
extern DWORD breakStall;
extern DWORD pollStall;

#define MAXCHARSTRLEN 100 /* Limit length to prevent buffer overflow */

int FAR *param;
char * buffer;

   static long powxx[] = { 1, 
                         10, 
                         100, 
                         1000, 
                         10000, 
                         100000,
                         1000000,
                         10000000,
                         100000000,
                         1000000000};

static char hexstr[] = "0123456789ABCDEF";


/*----------------------------------------------------------------------
Function name:  mysprintf

Description:    Formatting for Debugging output for the driver
                and WGL.
Information:    
  INPUT:  Similar to Standard vsprintf
    %X or %x	Hex format of a UINT
    %d  	    Decimal format of a UINT
	%D			Decimal format of a ULONG
	%L or %l   Hex format of a ULONG
    %s or %S   Ascii Zero terminated "C" string output
	%c or %C   Single character (parameter is a UCHAR or UINT)

Return:         VOID    Formatted string into first parameter
----------------------------------------------------------------------*/
VOID FAR mysprintf(char FAR *result, char FAR * str,int FAR *param)
{
   char FAR *charparam;
   char FAR *p;           // initial buffer pointer
   #define POW_SIZE	 (sizeof(powxx)/sizeof(long))
   int temp,i;
   unsigned long ltemp;
   long dtemp;

   DEBUG_FIX;

   p = result;          // initial buffer pointer
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
               ltemp = *(ULONG FAR *)param;
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
               ltemp = *(ULONG FAR *)param;
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
               charparam = *(char FAR * FAR*)param;
               param += sizeof(char FAR *)/sizeof(int);
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
                dtemp = *(long FAR *)param;
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
                    for (i = 1; i < POW_SIZE-1 && dtemp >= powxx[i]; ++i) {}
                    do 
                    {
                        i--;
                        *p++ = '0' + (UCHAR)(dtemp / powxx[i]);
                        dtemp = dtemp % powxx[i];
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
Function name:  InitDebugLevel

Description:    Initializes debug levels to 0.  It gets called by
                InitThunks32 so it will be called by every enable.
Information:    

Return:         VOID
----------------------------------------------------------------------*/
VOID InitDebugLevel(VOID)
{
   int i;
   DEBUG_FIX;

   for (i=0; i<MAX_GDI_FCN; i++)
   {
       lpDriverData->MsgFlag[i]=0;
   }

   //lpDriverData->MsgFlag[DBG_BITBLT]=6;
   lpDriverData->MsgFlag[DBG_DEBUG]=256;
}
 
   

/* ************************************************** 
 *  Debuglevel defines

       DBG_VALIDATEMODE        0
       DBG_BITBLT              1
       DBG_COLORINFO           2
       DBG_CONTROL             3
       DBG_DISABLE             4
       DBG_ENABLE              5
       DBG_ENUMDFONTS          6
       DBG_ENUMOBJ             7
       DBG_OUTPUT              8
       DBG_PIXEL               9
       DBG_REALIzEOBJECT       10
       DBG_STRBLT              11
       DBG_SCANLR              12
       DBG_DEVICEMODE          13
       DBG_EXTTEXTOUT          14
       DBG_GETCHARWIDTH        15
       DBG_DEVICEBITMAP        16
       DBG_FASTBORDER          17
       DBG_SETATTRIBUTE        18
//       DIBBLT              19
       DBG_DEVICEBITMAPBITS    19
       DBG_CREATEDIBITMAP      20
//       DIBTODEVICE         21
       DBG_SETDIBITSTODEVICE   21
       DBG_SETPALETTE          22
       DBG_GETPALETTE          23
       DBG_SETPALETTETRANSLATE 24
       DBG_GETPALETTETRANSLATE 25
       DBG_UPDATECOLORS        26
       DBG_STRETCHBLT          27
       DBG_STRETCHDIBITS       28
       DBG_SELECTBITMAP        29
       DBG_BITMAPBITS          30
       DBG_REENABLE            31
       DBG_SAVESCREENBITMAP    32
       DBG_INQUIRE             33 
       DBG_SETCURSOR           34
       DBG_MOVECURSOR          35
       DBG_CHECkCURSOR         36
       DBG_DEGUG               MAX_GDI_FCN -1
 ************************************************** */


/*----------------------------------------------------------------------
Function name:  dprintf

Description:    Debugging output for the driver.

Information:    
    ss:ebp             old bp
    ss:ebp+4           return address
    ss:ebp+8           GDI function signiture
    ss:ebp+c           debug level
    ss:ebp+10          ptr to string
    ss:ebp+14          ptr to var args 

    usage sample 
    DPF(DBG_BITBLT, 5 , " bitblt pattern %p\r\n",lpDst );

Return:         VOID
----------------------------------------------------------------------*/
VOID FAR dprintf1(DWORD GDI_fcn, DWORD dbg_level, char FAR *str,...) {
	DWORD flat_ss_bp;
	DWORD temp;
    // for easy offsets
    GLOBALDATA * lpdrvdata;
    DWORD * lpmsgflags;


    DEBUG_FIX;

	
     lpdrvdata=NULL;
    // raise or lower debug level for specific GDI fcn though lpmsgflag
    lpmsgflags= lpDriverData->MsgFlag;

    if(lpDriverData->MsgFlag[GDI_fcn] > dbg_level)
    {
        // since  you can't use pointers to locals,
        // flatten ss:bp into ds:12345678 flat ptr
    	__asm{
    		mov word ptr [temp], bp
    		mov ax, ss
    		mov word ptr [temp +2], ax
    		}
    	FarToFlat(temp,flat_ss_bp);
    
    	buffer=(char *) lpDriverData->charbuffer;
    
        // var args parametets are pointed by flat_ss_bp + 0x14
        mysprintf(buffer,str,(int *)(flat_ss_bp + 0x14));
    
        CallOutputDebugString(lpDriverData->bufferptr16,
    							lpDriverData->MsgFcn);
    }
}


/*----------------------------------------------------------------------
Function name:  sprintf

Description:    String Interface to vsprintf

Information:    

Return:         VOID
----------------------------------------------------------------------*/
VOID FAR sprintf1(char FAR *result, char FAR *str, ...) 
{
   int FAR *param = (int FAR *)(1 + &str);
   mysprintf(result,str,param);
}


FxU32 ctReset = 0;
FxU32 ctEndEIP = 0;
FxU32 ctStartEIP = 0;


/*----------------------------------------------------------------------
Function name:  BreakStall

Description:    BreakStall breaks if the Banshee status
                register is busy.
Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
BreakStall()
{
    DWORD status;

    DEBUG_FIX;
   
    if (ctReset)
	return;		// look out for recursive call
    
    if (hwbreakStall)
    {
	status = GET(_FF(lpIOregs)->status);
	if ( status & SST_BUSY )
	{
	    __asm int 3;
	}
    }
}

FxU32 ctNumResets = 10;
FxU32 ctNumNops = 16;


/*----------------------------------------------------------------------
Function name:  ctCrashRecover

Description:    Crash Test recovery mechanism.
                
Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
ctCrashRecover()
{
    FxU32 miscInit0, miscInit1;
    FxU32 i, j;
    CMDFIFO_PROLOG(cmdFifo);

    DEBUG_FIX;
    
    for (i = 0; i < ctNumResets; i++)
    {
	// turn off the command fifo
	SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.baseSize, 0);

	// reset FBI, 2D, command fifo
	miscInit0 = GET(_FF(lpIOregs)->miscInit0);
	SETDW(_FF(lpIOregs)->miscInit0, (miscInit0 |
					 SST_GRX_RESET |
					 SST_FBI_FIFO_RESET |
					 SST_2D_RESET));
	GET(_FF(lpIOregs)->status);		// delay
	SETDW(_FF(lpIOregs)->miscInit0, miscInit0);
	GET(_FF(lpIOregs)->status);		// delay

	// reset command fifo
	miscInit1 = GET(_FF(lpIOregs)->miscInit1);
	SETDW(_FF(lpIOregs)->miscInit1, (miscInit1 | SST_CMDSTREAM_RESET));
	GET(_FF(lpIOregs)->status);		// delay
	SETDW(_FF(lpIOregs)->miscInit1, miscInit1);
	GET(_FF(lpIOregs)->status);		// delay
    
	InitRegs();
	InitFifo(lpDriverData->fifoStart, lpDriverData->fifoSize);

	CMDFIFO_SETUP(cmdFifo);
	CMDFIFO_CHECKROOM(cmdFifo, ctNumNops);
	for (j = 0; j < ctNumNops; j++)
	{
	    SETPH(cmdFifo, SSTCP_PKT0_NOP);
	}
	CMDFIFO_EPILOG(cmdFifo);
    }
}


/*----------------------------------------------------------------------
Function name:  PollStall

Description:    PollStall breaks if the Banshee status register
                is busy for some predeterminated time. 
Information:    

Return:         VOID
----------------------------------------------------------------------*/
void
PollStall()
{
    BOOL wentIdle;
    DWORD i;
    DWORD status;

    DEBUG_FIX;

    wentIdle = FALSE;
    i = 0;

    if (ctReset)		// look out for recursive call
	return;

    if (hwpollStall)
    {
	while( (i<pollCount) && !(wentIdle) && !ctReset) 
	{
	    status = GET(_FF(lpIOregs)->status);
	    if ( !(  status & SST_BUSY  ) ) 
	    {
		wentIdle=TRUE;
	    }
	    i++;
	}

	if (!wentIdle)
	{
	    __asm int 3;
	}
    }

    if (ctReset)
    {
	ctCrashRecover();
	ctReset = 0;
    }
}


/*----------------------------------------------------------------------
Function name:  PCIFree

Description:    PCIFree waits until the PCI fifo has room
                for a write.
Information:    

Return:         BOOL    TRUE if success,
                        FALSE if failure.
----------------------------------------------------------------------*/
BOOL PCIFree()
{
   BOOL freeSpace;
   DWORD i;
   DWORD status;
   DEBUG_FIX
   i=0;
   freeSpace=FALSE;
   if(hwpciFree)
   {
      while ( ( i< pollCount)  && (!freeSpace) ) 
      {
         status= GET(_FF(lpIOregs)->status);
         if( status & 0x00001fL) 
         {
             freeSpace=TRUE;
         }
         i++;

      }
      if(!freeSpace)
      { 
         __asm int 3;
      }
   }
   return freeSpace;
}

#if 0
/*----------------------------------------------------------------------
Function name:  decodebrushflag

Description:    dump the name of the brush flag to the
                debug output device.

Information:    Not currently used!

Return:         VOID
----------------------------------------------------------------------*/
VOID decodebrushflag( DIB_Brush8 * lpBrush)
{

	if ( lpBrush->dp8BrushFlags & COLORSOLID) 
	{
		DPF("Brush:  COLORSOLID");
	}

	if ( lpBrush->dp8BrushFlags & MONOSOLID) 
	{
		DPF("Brush:  MONOSOLID");
	}	

	if ( lpBrush->dp8BrushFlags & PATTERNMONO) 
	{
		DPF("Brush:  PATTERNMONO");
	}	

	if ( lpBrush->dp8BrushFlags & MONOVALID) 
	{
		DPF("Brush:  MONOVALID");
	}	

	if ( lpBrush->dp8BrushFlags & MASKVALID) 
	{
		DPF("Brush:  MASKVALID");
	}	

	if ( lpBrush->dp8BrushFlags & PRIVATEDATA) 
	{
		DPF("Brush:  PRIVATEDATA");
	}	

}	


/*----------------------------------------------------------------------
Function name:  decodebrushstyle

Description:    dump the name of the brush style to the
                debug output device.

Information:    Not currently used!

Return:         VOID
----------------------------------------------------------------------*/
VOID decodebrushstyle (WORD style)
{
	switch(style)
	{
		case BS_SOLID:
			DPF(" BS_SOLID ");
			break;
		case BS_NULL:
			DPF(" BS_NULL/BS_HOLLOW ");
			break;
		case BS_HATCHED:
			DPF(" BS_HATCHED ");
			break;
		case BS_PATTERN:
			DPF(" BS_PATTERN ");
			break;
		case BS_INDEXED: 
			DPF(" BS_INDEXED ");
			break;
		case BS_DIBPATTERN:
			DPF(" BS_DIBPATTERN ");
			break;
		case BS_DIBPATTERNPT:
			DPF(" BS_DIBPATTERNPT ");
			break;
		case BS_PATTERN8X8:
			DPF(" BS_PATTERN8X8 ");
			break;
		case BS_DIBPATTERN8X8:
			DPF(" BS_DIBPATTERN8X8 ");
			break;
	}
}
#endif
