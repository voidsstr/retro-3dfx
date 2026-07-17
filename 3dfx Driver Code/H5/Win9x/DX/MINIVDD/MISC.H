/* -*-c++-*- */
/* $Header: misc.h, 2, 10/11/00 8:54:43 PM, Brent$ */
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
** File name:   misc.h
**
** Description: Misc. macros, defines, and externs.
**
** $Revision: 2$
** $Date: 10/11/00 8:54:43 PM$
**
** $History: misc.h $
** 
** *****************  Version 7  *****************
** User: Michael      Date: 1/11/99    Time: 3:54p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 6  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

/* Ternary raster operations */
#define PATCOPY             0x00F00021L
#define SRCCOPY             0x00CC0020L
#define BLACKNESS           0x00000042L
#define WHITENESS           0x00FF0062L

#define  BS_SOLID           0
#define  BS_NULL            1
#define  BS_HOLLOW          1
#define  BS_HATCHED         2
#define  BS_PATTERN         3
#define  BS_INDEXED         4
#define  BS_DIBPATTERN      5
#define  BS_DIBPATTERNPT    6
#define  BS_PATTERN8X8      7
#define  BS_DIBPATTERN8X8   8

#define HIGHWORD 0xFFFF0000
#define LOWWORD  0x0000FFFF

/* macros for determining rop components */
/* The following macros RopDst,RopSrc, RopPat uses the high word of
   the 32 bit rop.
	 
	 The following exerpt explains how the macros work.
	 
	P  S  D  |	  PSo	  DPSoo
	_________|	 ______   _____
           |
	0  0  0  |	    0	    0
	0  0  1  |	    0	    1
	0  1  0  |	    1	    1
	0  1  1  |	    1	    1
	1  0  0  |	    1	    1
	1  0  1  |	    1	    1
	1  1  0  |	    1	    1
	1  1  1  |	    1	    1

	Operation index:  00FC	  00FE


In this case, "PSo" has the operation index 00FC (read from bottom up);
"DPSoo" has the index 00FE.  These values define the location of the
corresponding raster operation code in the following table.  The "PSo"
operation is in line 252 (FC hex) of the table; "DPSoo" in line 254
(hex FE).
*/

//Uses the high word.
// ie  00f00021 patcopy,  macros use 00f0
#define RopUsesSrc(rop) (0 != (0x33 & ( rop ^ (rop >> 2))))
#define RopUsesPat(rop) (0 != (0x0F & ( rop ^ (rop >> 4))))
#define RopUsesDst(rop) (0 != (0x55 & ( rop ^ (rop >> 1))))

#define WORDBIT(n) (1<<(n))


#define CSIM_REG_BASE 				(unsigned long FAR *) 0x10000000

#define PHYS_SCREEN_BASE_ADDR			0x0L
#define PHYS_CMDFIFO_BASE_ADDR			0x02ff000L
#define NON_MODAL_LFB 					SST_RAW_LFB_OFFSET

// SUGGESTION:  might want to keep lph3g in driver data
//extern SstGRegs  * lph3g;
//extern SstGRegs h3g;
//extern SstCRegs * lph3agp;
//extern CmdFifo	* lph3cmdfifo0;

extern int PacketCount;
extern DWORD LastPtr;

//extern void FAR sprintf1(char FAR *result, char FAR *str, ...) ;
extern void FAR mysprintf(char FAR *result, char FAR * str,int FAR *param);


extern void makeRoom(DWORD * cp, unsigned long n);


#define WRITEMONOSRC(src, srcoffset, numscan, dwordsinscan)\
	WriteMonoSrc(src, srcoffset, numscan, dwordsinscan)

#define WRITECOLORSRC(src, srcoffset, numscan, dwordsinscan)\
	WriteColorSrc(src, srcoffset, numscan, dwordsinscan)


#ifdef CSIM 

	#define REG_BASE CSIM_REG_BASE

	#define HWPTR lph3g 

	extern DWORD UseDibEng;
	extern int FIRST;

#else 
	#define REG_BASE 0xffffffff 
#endif




#ifdef STOP 
	#define INT_3 __asm int 3;
#else	
	#define INT_3 ;
	
#endif


