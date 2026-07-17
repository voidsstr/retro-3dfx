/* -*-c++-*- */
/* $Header: realize.c, 2, 10/11/00 8:55:01 PM, Brent$ */
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
** File name:   realize.c
**
** Description: The GDI SelectBitmap and BitmapBits functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:55:01 PM$
**
** $History: realize.c $
** 
** *****************  Version 12  *****************
** User: Stb_shanna   Date: 2/26/99    Time: 4:49p
** Updated in $/devel/h3/win95/dx/minivdd
** Fixes PRS 4423 - need to tell the device a dword aligned pitch because
** we pad the data down below to make it so
** 
** *****************  Version 11  *****************
** User: Cwilcox      Date: 2/10/99    Time: 3:46p
** Updated in $/devel/h3/Win95/dx/minivdd
** Linear versus tiled promotion removal.
** 
** *****************  Version 10  *****************
** User: Michael      Date: 1/15/99    Time: 6:35a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 9  *****************
** User: Andrew       Date: 11/30/98   Time: 1:10a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to check to see if the device is busy before using it.  If
** it is busy we may be in low power mode which means that device will
** respond with all  f's which could make us wait forever.
** 
** *****************  Version 8  *****************
** User: Andrew       Date: 9/22/98    Time: 11:56a
** Updated in $/devel/h3/Win95/dx/minivdd
** Modified Mark's Fix for 2685 to make a flat pointer from the
** Selector/Offset in the Device Bit map and added the fix to another
** spot.
** 
** *****************  Version 7  *****************
** User: Andrew       Date: 9/15/98    Time: 8:04a
** Updated in $/devel/h3/Win95/dx/minivdd
** Found a problem with Myst 1024x768 24/32 bpp and Page fault.  Problem
** is when dwCount != width* height.
** 
** *****************  Version 6  *****************
** User: Andrew       Date: 8/31/98    Time: 11:28p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed a problem with a pointer to a variable on the stack
** 
** *****************  Version 5  *****************
** User: Andrew       Date: 7/27/98    Time: 11:15a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed problem where Punt flag was being overwritten
** 
** *****************  Version 4  *****************
** User: Ken          Date: 7/24/98    Time: 10:38p
** Updated in $/devel/h3/win95/dx/minivdd
** changes to allow 2d driver to run properly synchronized with an AGP
** command fifo (although video memory fifo is still used when the desktop
** has the focus, e.g., a fullscreen 3d app isn't in the foreground)
** 
** *****************  Version 3  *****************
** User: Andrew       Date: 7/23/98    Time: 12:22p
** Updated in $/devel/h3/Win95/dx/minivdd
** Accelerated BitmapBits
** 
** *****************  Version 2  *****************
** User: Ken          Date: 4/15/98    Time: 6:42p
** Updated in $/devel/h3/win95/dx/minivdd
** added unified header to all files, with revision, etc. info in it
**
*/

//: realize.c


#include "h3.h"
#include "thunk32.h"
#include "h3g.h"
#include "entrleav.h"

extern DWORD HandleFormat(DIBENGINE FAR * Src);

int HostToFB(LPDIBENGINE lpDevice, LPBYTE lpBits, DWORD dwSrcAdj);
int FBToHost(LPDIBENGINE lpDevice, LPBYTE lpBits, DWORD dwSrcAdj);
int FBToFB(LPDIBENGINE lpDst, LPDIBENGINE lpSrc);


/*----------------------------------------------------------------------
Function name:  FxSelectBitmap

Description:    32-bit side of the GDI SelectBitmap function.
                
Information:    Function is always punted to the dib engine.

Return:         BOOL    FALSE is always returned.
----------------------------------------------------------------------*/
BOOL FxSelectBitmap
(
    SelectBitmapParams* pParams
)
{
    DEBUG_FIX

    bPunt = TRUE;

    return FALSE;
}


/*----------------------------------------------------------------------
Function name:  FxBitmapBits

Description:    32-bit side of the GDI BitmapBits function.
                
Information:    Looks like return should be DWORD.

Return:         BOOL    pParams->dwCount
----------------------------------------------------------------------*/
#define DBB_SET (1)
#define DBB_GET (2)
#define DBB_COPY (4)
#define DBB_SETWITHFILLER (8)

BOOL FxBitmapBits
(
    BitmapBitsParams* pParams
)
{
   DWORD dwCount;
   DWORD fFlags;
   DWORD dwSize;
   DWORD dwNum;
   DWORD dwAddr;
   DWORD dwSaveAddr;
   DWORD dwHeight;
   DWORD i;
   LPDIBENGINE lpDevice;
   LPDIBENGINE lpSrc;
   LPBYTE lpBits;
   LPBYTE lpHost;
   WORD wSaveHeight;
   DEBUG_FIX;
   
   FarToFlat(pParams->lpDevice, lpDevice);
   FarToFlat(pParams->lpBits, lpBits);

   dwCount = pParams->dwCount;
   fFlags = pParams->fFlags;

   bPunt = TRUE;
   if (DBB_SET == fFlags) 
      {
   	if ((DWORD)lpDevice->deWidthBytes * (DWORD)lpDevice->deHeight == dwCount)
         HostToFB(lpDevice, lpBits, 0x0);      
      }
   else if (DBB_GET == fFlags)
      {
   	if ((DWORD)lpDevice->deWidthBytes * (DWORD)lpDevice->deHeight == dwCount)
         FBToHost(lpDevice, lpBits, lpDevice->deDeltaScan - lpDevice->deWidthBytes);
      }      
   else if (DBB_COPY == fFlags)
      {
      lpSrc = (LPDIBENGINE)lpBits;
      if ((VRAM|OFFSCREEN) == (lpSrc->deFlags & (VRAM|OFFSCREEN)))
         FBToFB(lpDevice, lpSrc);
      else if ((DWORD)lpDevice->deWidthBytes * (DWORD)lpDevice->deHeight == dwCount)
         {
         SelToFlat(lpSrc->deBitsSelector, lpHost);
         lpHost += lpSrc->deBitsOffset;
         HostToFB(lpDevice, lpHost, lpSrc->deDeltaScan - lpSrc->deWidthBytes);
         }
      }      
   else if (DBB_SETWITHFILLER == fFlags)
      {
      // If size is less then 64K then its just a DBB_SET
      dwSize = lpDevice->deHeight * lpDevice->deWidthBytes; 
      if (dwSize < 0x10000)
         HostToFB(lpDevice, lpBits, 0x0);
      else
         {
         // else size > 64K in which case we divide into 64K segments
         // and walk the transfer over the filler areas
         // the filler is caused by Windows not wanting to divide
         // a scanline across segments         

         // calculate number of 64K segments
         dwNum = dwSize >> 16;
         dwHeight = 0x10000/lpDevice->deWidthBytes;
      
         // we calculated the size without the filler
         // calculate number of extra data at the end we skip and 
         // need to add to the actual size
         // we need to do this so that the part < 64K will work out
         dwSize = dwSize + dwNum * (0x10000 % lpDevice->deWidthBytes);      
         // this is probably unnecessary but we may have more 64 K transfers
         // now
         dwNum = dwSize >> 16;

         dwAddr = dwHeight * lpDevice->deDeltaScan;
         dwSaveAddr = lpDevice->deBitsOffset;
         wSaveHeight = lpDevice->deHeight;
         lpDevice->deHeight = (WORD)dwHeight;
         for (i=0; i<dwNum; i++)
            {
            HostToFB(lpDevice, lpBits, 0x0);
            lpBits += 0x10000;
            lpDevice->deBitsOffset += dwAddr;
            }
         dwSize = dwSize & 0xFFFF;
         if (dwSize)
            {
            lpDevice->deHeight = (WORD)(dwSize/lpDevice->deWidthBytes);
            HostToFB(lpDevice, lpBits, 0x0);
            }
         lpDevice->deHeight = wSaveHeight;
         lpDevice->deBitsOffset = dwSaveAddr;
         }
      }

   return dwCount;
}


/*----------------------------------------------------------------------
Function name:  HostToFB

Description:    Simple Host to Screen Transfer for Bitmapbits.
                
Information:    

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
int HostToFB(LPDIBENGINE lpDevice, LPBYTE lpBits, DWORD dwSrcAdj)
{
   DWORD dwWhole;
   DWORD dwPartial;
   DWORD dwTotal;
   DWORD dwData;
   LPBYTE lpDstByte;
   DWORD * lpSrc;
   DWORD i;
   DWORD j;
   DWORD      dwFP1616;
   DWORD ddFormat;
   CMDFIFO_PROLOG(cf);

   DEBUG_FIX;   

   dwWhole = lpDevice->deWidthBytes >> 2;
   dwPartial = lpDevice->deWidthBytes & 0x03;

   dwTotal = dwWhole;
   if (dwPartial)
      dwTotal++; 

   FXENTER("HostToFB", lpDevice->deFlags, (hwBitmapBits && hwBitmapBitsHS),
            cf, lpDevice, FXENTER_NO_SRC, FXENTER_NO_SRCFORMAT,
            FXENTER_NO_RECT, 0, 0, 0, 0,
            FXENTER_NO_RECT, 0, 0, 0, 0);

   CMDFIFO_CHECKROOM(cf, 6);
   SETPH(cf, (SSTCP_PKT2 | srcFormatBit | srcXYBit | dstSizeBit |
		   dstXYBit | commandBit)); 
   // PRS 4423 - need to tell the device a dword aligned pitch, 
   //            because we pad the data down below to make it so
   ddFormat = HandleFormat(lpDevice)+dwPartial;
   SET(cf, _FF(lpGRegs)->srcFormat, ddFormat);
   SET(cf, _FF(lpGRegs)->srcXY, 0x0);
   SET(cf, _FF(lpGRegs)->dstSize, R32(lpDevice->deHeight, lpDevice->deWidth));
   SET(cf, _FF(lpGRegs)->dstXY, 0x0);
   SETC(cf, _FF(lpGRegs)->command, SSTG_ROP_SRCCOPY | SSTG_HOST_BLT);
   BUMP(6);

   // Do it ScanLine by ScanLine
   for (i=0; i<lpDevice->deHeight; i++)
      {
      CMDFIFO_CHECKROOM(cf, dwTotal + 1);
      SETPH(cf, (SSTCP_PKT1 | SSTCP_PKT1_2D | (LAUNCH_REG_1 << SSTCP_REGBASE_SHIFT) | ((dwTotal) << SSTCP_PKT1_NWORDS_SHIFT)));
      lpSrc = (DWORD *)lpBits;
      for (j=0; j<dwWhole; j++)
	      SET(cf, _FF(lpGRegs)->launch[0], *lpSrc++);
               
      lpBits = (LPBYTE)lpSrc;
      if (dwPartial)
         {
         lpDstByte = (LPBYTE)&dwData;
         STACKFP(lpDstByte, dwFP1616)
         FarToFlat(dwFP1616, lpDstByte);
         dwData = 0;
         for (j=0; j<dwPartial; j++)
            *lpDstByte++ = *lpBits++;
	      SET(cf, _FF(lpGRegs)->launch[0], dwData);
         }                    
      lpBits += dwSrcAdj;
      }         

   bPunt = FALSE;
   FXLEAVE("HostToFB", cf, lpDevice);

   return 1;
}


/*----------------------------------------------------------------------
Function name:  FBToHost

Description:    Simple Screen to Host Transfer for Bitmapbits.
                
Information:    

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
int FBToHost(LPDIBENGINE lpDevice, LPBYTE lpBits, DWORD dwSrcAdj)
{
   DWORD dwWhole;
   DWORD dwPartial;
   LPBYTE lpSrcByte;
   LPBYTE lpHost;
   DWORD * lpSrc;
   DWORD * lpDst;
   DWORD i;
   DWORD j;

   DEBUG_FIX;

#ifdef DEBUG
   if (hwBitmapBits & hwBitmapBitsSH)
      return 0;
#endif

   // Check to see if we are busy before spinning on the device
   // else we could wait here forever
   if (*lpDriverData->lpDeFlags & BUSY)
      return 1;

   // Spin until engine is free
   FXWAITFORIDLE();

   dwWhole = lpDevice->deWidthBytes >> 2;
   dwPartial = lpDevice->deWidthBytes & 0x03;
 
   // Do it ScanLine by ScanLine
   lpSrc = (DWORD *)lpDevice->deBitsOffset;

   // For memory bitmaps, not cached ones...
   if ((VRAM|OFFSCREEN) != (lpDevice->deFlags & (VRAM | OFFSCREEN) ) )
   {
      SelToFlat(lpDevice->deBitsSelector, lpHost);
      lpHost += lpDevice->deBitsOffset;
      lpSrc = (DWORD *)lpHost;
   }

   for (i=0; i<lpDevice->deHeight; i++)
      {
      lpDst = (DWORD *)lpBits;
      for (j=0; j<dwWhole; j++)
   	      *lpDst++ = *lpSrc++;
               
      lpBits = (LPBYTE)lpDst;
      lpSrcByte = (LPBYTE)lpSrc;
      if (dwPartial)
         {
         for (j=0; j<dwPartial; j++)
            *lpBits++ = *lpSrcByte++;
         }                    
      lpSrcByte += dwSrcAdj;
      lpSrc = (DWORD *)lpSrcByte;
      }         

   bPunt = FALSE;
   return 1;
}


/*----------------------------------------------------------------------
Function name:  FBToFB

Description:    Simple Screen to Screen Transfer for Bitmapbits.
                
Information:    

Return:         INT     1 is always returned.
----------------------------------------------------------------------*/
int FBToFB(LPDIBENGINE lpDst, LPDIBENGINE lpSrc)
{
   CMDFIFO_PROLOG(cf);

   DEBUG_FIX;   

   FXENTER("FBToFB", lpDst->deFlags, (hwBitmapBits && hwBitmapBitsSS),
            cf, lpDst, lpSrc, FXENTER_NO_SRCFORMAT,
            FXENTER_NO_RECT, 0, 0, 0, 0,
            FXENTER_NO_RECT, 0, 0, 0, 0);

   CMDFIFO_CHECKROOM(cf, 5);
   SETPH(cf, (SSTCP_PKT2 | srcXYBit | dstSizeBit | dstXYBit | commandBit)); 
   SET(cf, _FF(lpGRegs)->srcXY, 0x0);
   SET(cf, _FF(lpGRegs)->dstSize, R32(lpDst->deHeight, lpDst->deWidth));
   SET(cf, _FF(lpGRegs)->dstXY, 0x0);
   SETC(cf, _FF(lpGRegs)->command, SSTG_ROP_SRCCOPY | SSTG_GO | SSTG_BLT);
   BUMP(5);

   bPunt = FALSE;
   FXLEAVE("FBToFB", cf, lpDst);

   return 1;
}
