/* -*-c++-*- */
/* $Header: bltdata.c, 2, 10/11/00 8:53:52 PM, Brent$ */
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
** File name:   bltdata.c
**
** Description: Support functions for Bitblt.  Handle Mono Blt data.
**
** $Revision: 2$
** $Date: 10/11/00 8:53:52 PM$
**
** $History: bltdata.c $
** 
** *****************  Version 9  *****************
** User: Michael      Date: 1/04/99    Time: 1:20p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 8  *****************
** User: Ken          Date: 4/15/98    Time: 6:41p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

#include "h3.h"
#include "thunk32.h"
#include "bitblt.h"


/*----------------------------------------------------------------------
Function name:  SendMonoBltData

Description:    Greg Mendel's version of data moving code.

Information:

Return:         VOID
----------------------------------------------------------------------*/
void SendMonoBltData(DWORD xs, DWORD ys, 
                     DWORD width, DWORD height,
                     LPBITMAP  lpSrc)
{
//xs = start x
//ys = start y
//base = surface base addr
//stride = src byte stride
//h = blt height in pixels
//w = blt width in pixels

   DWORD yc;
   DWORD stride, stride_shift;
   DWORD x;
   DWORD * addr;
   DWORD  base, min_dwords, byte_correct;
   DWORD  base_addr, line_addr, byte_width, dwordsInScan;
   DWORD totalDwords;

   CMDFIFO_PROLOG(cmdFifo);
   DEBUG_FIX;
   CMDFIFO_SETUP(cmdFifo);

   FarToFlat((DWORD)lpSrc->bmBits, base);
   stride=lpSrc->bmWidthBytes;
   base_addr = ( base + (ys*stride) + (xs>>3) );

   byte_width = (width +(xs & 7) +7)>>3;

   stride_shift = stride & 3;
   
    if (stride_shift==0)          // Nice dword stride
    {
        dwordsInScan = ((byte_width+((xs>>3)&3)+3)>>2);
        totalDwords = dwordsInScan * height;

        // use xs&0xFFE0>>3 to get (xs/32)*4
        addr = (DWORD*)(base + (ys*stride) + ((xs&0xFFE0)>>3));

//        DPF(" Zero case\r\n");
        CMDFIFO_CHECKROOM(cmdFifo, totalDwords + 1);
        SETPH(cmdFifo, SSTCP_PKT1| 
             SSTCP_PKT1_2D|
             LAUNCH_REG_1<<SSTCP_REGBASE_SHIFT|
             totalDwords  <<SSTCP_PKT1_NWORDS_SHIFT); 
        for (yc=0; yc<height; yc++) 
        {
            for (x=0; x<dwordsInScan; x++)  
            {
                SET(cmdFifo, _FF(lpGRegs)->launch[0], addr[x]); 
            }
            (DWORD) addr += stride;
        }
        BUMP(totalDwords + 1);
    } 
    else             // Nasty non-dword stride
    {

         // When (byte_width%4)==1, it doesn't really
         // matter that we have a non-dword stride, because
         // we'll always send the same # of dwords each line
         if ((byte_width&3)==1) 
         {
//            DPF(" (byte_width%4 = 1) case\r\n");
            dwordsInScan = (byte_width>>2) + 1;   
            totalDwords= dwordsInScan * height;
            // No matter how many byte of garbage at the start
            // of a line, there are 3 byte garbage total
            line_addr = base_addr;   // (line_addr is a byte address)
            CMDFIFO_CHECKROOM(cmdFifo, totalDwords + 1);
            SETPH(cmdFifo, SSTCP_PKT1| 
                SSTCP_PKT1_2D|
                LAUNCH_REG_1<<SSTCP_REGBASE_SHIFT|
                totalDwords  <<SSTCP_PKT1_NWORDS_SHIFT); 
            for (yc=0; yc<height; yc++) 
            {
                addr = (DWORD *)(line_addr&0xfffffffc);   // (addr is a dword *)
                line_addr += stride;
                for (x=0; x<dwordsInScan; x++) 
                {
                    SET(cmdFifo, _FF(lpGRegs)->launch[0], addr[x]); 
                }
            }
            BUMP(totalDwords + 1);
        }  
        // This is the nastiest case possible
        else 
        {  
//            DPF(" ELSE  case\r\n");
//	    DPF(" w %x, h %x , xs %xs, ys %ys \r\n",
//		width, height, xs, ys);
            min_dwords = (byte_width+3)>>2;
            byte_correct = (byte_width-1)&3;
   
            line_addr = base_addr;   // (line_addr is a byte address)
            for (yc=0; yc<height; yc++)  
            {
                addr = (DWORD*)(line_addr&0xfffffffc);   // (addr is a dword *)
                dwordsInScan = min_dwords + (((line_addr&3)+byte_correct)>>2);
                line_addr += stride;

                CMDFIFO_CHECKROOM(cmdFifo, dwordsInScan + 1);
                SETPH(cmdFifo, SSTCP_PKT1| 
                      SSTCP_PKT1_2D|
                      LAUNCH_REG_1<<SSTCP_REGBASE_SHIFT|
                      dwordsInScan  <<SSTCP_PKT1_NWORDS_SHIFT); 
                for (x=0; x<dwordsInScan; x++) 
                {
                    SET(cmdFifo, _FF(lpGRegs)->launch[0], addr[x]); 
                }
                BUMP(dwordInScan + 1);
             }
        }
    }
    CMDFIFO_EPILOG(cmdFifo);
}


/*----------------------------------------------------------------------
Function name:  SendMonoBltDataT

Description:    Adapted from gary T's code.  Not completely
                debugged but sends the correct amout of data.
                look in $devel\h3\diag\lib\sstgdiag.c  and
                $devel\h3\diag\gui\hsblt.c.   (02/21/98 agin)

Information:

Return:         VOID
----------------------------------------------------------------------*/
void SendMonoBltDataT(DWORD xs, DWORD ys, 
                      DWORD w, DWORD h,
                      LPBITMAP  lpSrc)
{
   DWORD yc, n;
   DWORD addr, next_addr;
   DWORD offset, byte, bit, stride;
   DWORD x,k;
   DWORD * bltData;
   DWORD * lpData;
   DWORD totalDwords;

   CMDFIFO_PROLOG(cmdFifo);
   DEBUG_FIX;
   CMDFIFO_SETUP(cmdFifo);

   FarToFlat((DWORD)lpSrc->bmBits, lpData);

   n=0;
   stride=lpSrc->bmWidthBytes;
   offset = ( stride * ys) + (xs>>3);
   bltData =  (DWORD *) ((DWORD)lpData + offset );
   byte = (DWORD) bltData & 3;
   bit = (byte * 8) + (xs & 7);
   next_addr = (DWORD) bltData & 3;



   // for each row of the blit
   for (yc = ys;  yc < ys+h ; yc++)
   {
      k=0;
      addr = next_addr;
      next_addr = (addr + (stride & 3))  & 3;

      if ((addr & 3) || (bit & 7))
      {
         CMDFIFO_CHECKROOM(cmdFifo, 2);
         SETPH(cmdFifo, SSTCP_PKT1| 
                        SSTCP_PKT1_2D|
                        LAUNCH_REG_1<<SSTCP_REGBASE_SHIFT|
                        1L <<SSTCP_PKT1_NWORDS_SHIFT); 
         SET(cmdFifo, _FF(lpGRegs)->launch[0], bltData[k]);
         BUMP(2);
         k=1;

         x = 32 - ((addr & 3) * 8 + (bit & 7)) ;  // compute starting bit 
         if (w < x)
         {
            x = w;
         }
      }
      else
      {
         x = 0;
      }

      totalDwords = ((w - x)/32) + 1;
      CMDFIFO_CHECKROOM(cmdFifo, totalDwords + 1);
      SETPH(cmdFifo, SSTCP_PKT1| 
            SSTCP_PKT1_2D|
            LAUNCH_REG_1<<SSTCP_REGBASE_SHIFT|
            totalDwords <<SSTCP_PKT1_NWORDS_SHIFT); 
      for (x=x; x <= w; x+=32)
      {
         SET(cmdFifo, _FF(lpGRegs)->launch[0], bltData[k]);
         k++;
      }
      BUMP(totalDword+1);
      bltData = (DWORD*) ((DWORD) bltData + stride);
   }
   CMDFIFO_EPILOG(cmdFifo);
}
