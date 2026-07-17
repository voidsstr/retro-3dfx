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
** Last Edited: Thu May  8 21:45:51 1997 by psmith (Phil Smith (x2456)) on vlsi2
**
** $Revision: 2$
** $Date: 10/11/00 8:12:15 PM$
**
*/

int olyBilerp(FxU32 xfrac, FxU32 yfrac, FxU32 value00, FxU32 value01, 
					 FxU32 value10, FxU32 value11, FxU32 shift);
FxU32 olyExtractUndither(FxU32 x, FxU32 y,  SstIORegs *IORegs, int nextpixel);
FxU32 vidOutCSC ( FxU32 value, SstIORegs *IORegs, FxBool pixType); 
int vidOut(  );
static FxBool screenDump(FxU32 *cp, 
			 const char *filenameRoot,
			 FxU32 width, FxU32 height, FxU32 imgType);
FxU32 overlayProcess(  SstIORegs *IORegs, 
                 FxU32 x, FxU32 y,
                 FxU32 xdesk, FxU32 ydesk);
FxU32 redDiff(SstIORegs *IORegs,FxU32 value, FxU32 value00,FxU32  x, FxU32 y);
FxU32 greenDiff(SstIORegs *IORegs,FxU32 value, FxU32 value00,FxU32  x, FxU32 y);
FxU32 blueDiff(SstIORegs *IORegs,FxU32 value, FxU32 value00,FxU32  x, FxU32 y);
int  loadClut(FxU32 *dataIn , SstIORegs *IORegs);
int clutInit( SstIORegs *IORegs);
void vidInit(FxU32 width, FxU32 height , SstRegs *sst);
int vidTimingInit(FxU32 width, FxU32 height, SstRegs *sst);
void vidCleanup(SstRegs *sst);

// temporary ifdef, until Phil has added these routines.
#ifndef HAL_SIM
   void VGA_SET_MODE(int a, int b);
#endif
/*
   void PCI_IOW8(int addr, int data);
   void PCI_IOW16(int addr, int data);
   int  PCI_IOR8(int addr);


   void SET_IO8(int addr, int data);
   void SET_IO16(int addr, int data);
   int  GET_IO8(int addr);
*/
void SET_PIXEL(FxI32 buffer, int x, int y, FxU32 col);


