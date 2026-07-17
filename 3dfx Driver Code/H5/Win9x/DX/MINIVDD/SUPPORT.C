/* -*-c++-*- */
/* $Header: support.c, 2, 10/11/00 8:55:10 PM, Brent$ */
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
** File name:   support.c
**
** Description: Misc support functions.
**
** $Revision: 2$
** $Date: 10/11/00 8:55:10 PM$
**
** $History: support.c $
** 
** *****************  Version 41  *****************
** User: Andrew       Date: 8/04/99    Time: 12:58p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added Source Clip of Save Under
** 
** *****************  Version 40  *****************
** User: Andrew       Date: 7/30/99    Time: 3:12p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Forgot to enable SW Cursor for WIN_SIM in support.c
** 
** *****************  Version 38  *****************
** User: Andrew       Date: 5/06/99    Time: 4:38p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code for New Windows "C" Simulator
** 
** *****************  Version 37  *****************
** User: Andrew       Date: 3/24/99    Time: 2:52p
** Updated in $/devel/h3/Win95/dx/minivdd
** Was using Xsize for Y coordinate when I should have been using Ysize
** 
** *****************  Version 36  *****************
** User: Michael      Date: 1/15/99    Time: 7:01a
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** *****************  Version 35  *****************
** User: Andrew       Date: 10/06/98   Time: 1:35p
** Updated in $/devel/h3/Win95/dx/minivdd
** changed lpGRegs to _FF(lpGRegs)
** 
** *****************  Version 34  *****************
** User: Andrew       Date: 10/06/98   Time: 9:01a
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed to support the new way we draw cursors
** 
** *****************  Version 33  *****************
** User: Andrew       Date: 9/11/98    Time: 10:15a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to Handle large negative X & Y's.
** 
** *****************  Version 32  *****************
** User: Andrew       Date: 9/10/98    Time: 12:15p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added a fix to FxBeginAccess32 to allow Cursor Format BPP != Screen
** Format BPP
** 
** *****************  Version 31  *****************
** User: Andrew       Date: 8/18/98    Time: 9:50p
** Updated in $/devel/h3/Win95/dx/minivdd
** Fixed a problem where the a changed hot spot would cause us to restore
** to the wrong spot.
** 
** *****************  Version 30  *****************
** User: Ken          Date: 7/28/98    Time: 6:37p
** Updated in $/devel/h3/win95/dx/minivdd
** fixed the 20bpp brush problem w/out modifying the brush structure
** itself
** 
** *****************  Version 29  *****************
** User: Ken          Date: 7/28/98    Time: 11:21a
** Updated in $/devel/h3/win95/dx/minivdd
** added patch (not necessarily a fix!) for occasional crashes due to
** walking off the end of a brush structure and page faulting in 16bpp in
** HandleBrush
** 
** *****************  Version 28  *****************
** User: Ken          Date: 7/24/98    Time: 10:38p
** Updated in $/devel/h3/win95/dx/minivdd
** changes to allow 2d driver to run properly synchronized with an AGP
** command fifo (although video memory fifo is still used when the desktop
** has the focus, e.g., a fullscreen 3d app isn't in the foreground)
** 
** *****************  Version 27  *****************
** User: Andrew       Date: 7/15/98    Time: 9:30a
** Updated in $/devel/h3/Win95/dx/minivdd
** Move SW Cursor register setup to FxBeginAccess32 and FxEndAccess32
** 
** *****************  Version 26  *****************
** User: Michael      Date: 6/23/98    Time: 4:28p
** Updated in $/devel/h3/Win95/dx/minivdd
** FredW (IGX) - Restore commandEx.  If we don't restore commandEx, the
** source compare operation remains enabled.  Then, any matching colors
** cause problems.  (eg. A memory to screen blt of a background bitmap!).
** Fixes 1829 and 1978.
**
** *****************  Version 25  *****************
** User: Michael      Date: 6/01/98    Time: 10:32a
** Updated in $/devel/h3/Win95/dx/minivdd
** Fred W's (igx) fix for dither at 8-bpp with a solid brush.  Fixes #1671
** and 1678.  Also added unified header with revision, etc.
**
**
*/

#include "thunk32.h"
#include "h3g.h"

LPDIBENGINE lpDestDev;
LPDIBENGINE lpSrcDev;

LPCMINFO lpDestInfo;
LPCMINFO lpSrcInfo;

#ifdef DEBUG
int GottaCursor32();
#endif

DIB_Brush1* lpPBrush;

DWORD dwRop;
DWORD dwCmd;

DWORD RopConvertTable[17]={
   0x00000000,   //   Blackness 0x00
   0x11000000,   //   DPon  0x5
   0x22000000,   //   DPna   0xA
   0x33000000,   //   Pn      0xf
   0x44000000,   //   Pda      0x50
   0x55000000,   //   Dn      0x55
   0x66000000,   //   DPx      0x5A
   0x77000000,   //   DPan   0x5F
   0x88000000,   //   DPa      0xA0
   0x99000000,   //   DPxn   0xA5
   0xAA000000,   //   D      0xAA
   0xBB000000,   //   DPno   0xAF
   0xCC000000,   //   S      0xF0
   0xDD000000,   //   PDno   0xF5
   0xEE000000,   //   DpO      0xFA
   0xFF000000   //   WHITENESS   0xFF
   };


/*----------------------------------------------------------------------
Function name:  HandleBrush

Description:    Handle a brush given the bits, rop and drawmode.
                
Information:    

Return:         DWORD   value for the command data register
----------------------------------------------------------------------*/
DWORD HandleBrush
(
   DIB_Brush8  * lpBrush,
   DWORD Rop,
   DRAWMODE  * lpDrawMode
)
{

   DWORD dwtemp, dwpatcount, i;
   int patcount;
   BYTE mono0[4];
   BYTE mono1[4];
   DWORD dwXprntFlg;
   DWORD brushBpp;

   DWORD solidColor1, solidColor2;
   DWORD * lpPattern;
    CMDFIFO_PROLOG(cmdFifo);
    DEBUG_FIX
    CMDFIFO_SETUP(cmdFifo);

   lpPattern = (DWORD *) lpBrush->dp8BrushBits;
   switch(lpBrush->dp8BrushStyle)
   {
      case BS_SOLID:
         /* Dispatch is quick with the switch statement above, but...
          * we can get confused. BS_SOLID implies a solid color if
          * COLORSOLID is set. Otherwise, its a dithered color. In
          * high color modes this isn't a big concern, but in 8bpp
          * mode (specifically in the Paint * applet color selector)
          * it *is* a big deal. Some colors *must* be rendered as
          * dithered, which we will only do by taking this as a
          * patterned brush!
          */
         if ( (!(lpBrush->dp8BrushFlags & COLORSOLID)) )
         {
             goto process_pattern;
         }

         // I don't think you get mono brushes with solid style
         #ifdef DEBUG
            if ( lpBrush->dp8BrushFlags&MONOVALID)
               {
               //DPF(" MONOVALID WITH BS_SOLID");
               //__asm int 3
               }

         #endif
         // color
         if(!RopUsesSrc(Rop >> 16)  )
         {

         //   DPF(" BS_SOLID ROP convert ");
         //   INT_3
            CMDFIFO_CHECKROOM(cmdFifo,2);
            SETPH(cmdFifo , SSTCP_PKT2 | colorForeBit);
            SET(cmdFifo, _FF(lpGRegs)->colorFore, *lpPattern);
            BUMP( 2 );
            CMDFIFO_EPILOG(cmdFifo);

            // use rop that converts Pat arg into src arg in rop
            dwtemp=RopConvertTable[((Rop >> 16 ) & 0x3L) |
                              (((Rop >> 16) & 0x30L) >> 2)];
            return dwtemp;
         }
         else
         {
            // since rop uses src can't use pat to src switch.
            // DANGER DANGER WILL ROBINSON
            // Load first row of pattern only for solid
            //DPF(" BS_SOLID ROP uses Src ");
            //INT_3

            //patcount =  (8 * lpBrush->dp8BrushBpp >> 3);


            // I will have to write to commandExtra to
            // set the ForceRow0 bit with separate packet
            // since its out of sequence.
            // I will ask for dwords for the commandExta
            // packet and pattern packet with one checkmemfifo.
            // The commandExtra packet takes 2 dwords

            if ( lpBrush->dp8BrushBpp == 24)
            {
                solidColor1=(*(DWORD FAR *)&lpBrush->dp8BrushBits) & 0x00FFFFFF;
                solidColor2= solidColor1;
                CMDFIFO_CHECKROOM(cmdFifo,9);

                SETPH(cmdFifo, SSTCP_PKT1|
                            SSTCP_PKT1_2D|
                            SSTCP_INC|
                            PATTERN_REG_1<< SSTCP_REGBASE_SHIFT|
                            6 << SSTCP_PKT1_NWORDS_SHIFT);

                SET(cmdFifo, _FF(lpGRegs)->colorPattern[0],
                    (solidColor1<< 24 | solidColor2 ));

                SET(cmdFifo, _FF(lpGRegs)->colorPattern[1],
                    (solidColor1<<16 | solidColor2>>8 ));

                SET(cmdFifo, _FF(lpGRegs)->colorPattern[2],
                    (solidColor1<<8 | solidColor2>>16 ));

                SET(cmdFifo, _FF(lpGRegs)->colorPattern[3],
                    (solidColor1<< 24 | solidColor2 ));

                SET(cmdFifo, _FF(lpGRegs)->colorPattern[4],
                    (solidColor1<<16 | solidColor2>>8 ));

                SET(cmdFifo, _FF(lpGRegs)->colorPattern[5],
                    (solidColor1<<8 | solidColor2>>16 ));

                SETPH(cmdFifo, SSTCP_PKT2| commandExBit);
                SET(cmdFifo, _FF(lpGRegs)->commandEx, SSTG_PAT_FORCE_ROW0);

                BUMP(9);
            }

            else
            {
                // dwpatcount is number dwords in the brush
                // dwpatcount is DWORD type so if shift left
                // by 16 you won't end up with 0.
                dwpatcount =  (8 * lpBrush->dp8BrushBpp >> 3) >> 2;
                CMDFIFO_CHECKROOM(cmdFifo,dwpatcount + 3);

                SETPH(cmdFifo, SSTCP_PKT1|
                            SSTCP_PKT1_2D|
                            SSTCP_INC|
                            PATTERN_REG_1<< SSTCP_REGBASE_SHIFT|
                            dwpatcount << SSTCP_PKT1_NWORDS_SHIFT);

                for(i = 0; i < dwpatcount ; i++)
                {
                   SET(cmdFifo, _FF(lpGRegs)->colorPattern[i],
                       *(DWORD FAR *)&lpBrush->dp8BrushBits);

                }
                SETPH(cmdFifo, SSTCP_PKT2| commandExBit);
                SET(cmdFifo, _FF(lpGRegs)->commandEx, SSTG_PAT_FORCE_ROW0);

                BUMP(dwpatcount + 3);
            }

            CMDFIFO_EPILOG(cmdFifo);
                _FF(ClearCommandEx) = 1;
            return((Rop&0xFFFF0000)<<8);


         }
         break;

      case BS_NULL:
         //DPF(" BS_NULL/BS_HOLLOW ");
         // Note that NULL/HOLLOW brush cannot be trivially
         // rejected.  There still may need to be processing
         // contingent on the rop.
         // use rop that converts Pat arg into src arg in rop
         return((Rop&0xFFFF0000)<<8);
         break;

      case BS_HATCHED:
         /* According to documentation, physical background color is
            stored in dpxBgColor and foreground in dpxFgColor, then
            draw the bit pattern in dpxBrushmask.
            Above applies if dib engine realizes the brush.
         */
         //DPF(" BS_HATCHED ");

         //INT_3
         for (i = 0; i < 4; i++)
         {
            mono0[i] = (lpBrush->dp8BrushMask[4*i]);
            mono1[i] = (lpBrush->dp8BrushMask[4*i+16]);
         }
         CMDFIFO_CHECKROOM(cmdFifo,5);
         SETPH(cmdFifo, SSTCP_PKT2|
                   pattern0aliasBit|
                   pattern1aliasBit|
                   colorBackBit|
                   colorForeBit);

         // load mono pattern
         // DANGER WILL ROBINSON
         // dibengine realizes mono brushes has dword packed
         SET(cmdFifo, _FF(lpGRegs)->pattern0alias, *(DWORD*) mono0 );
         SET(cmdFifo, _FF(lpGRegs)->pattern1alias, *(DWORD*) mono1 );

         SET(cmdFifo, _FF(lpGRegs)->colorBack, lpBrush->dp8BgColor);
         SET(cmdFifo, _FF(lpGRegs)->colorFore, lpBrush->dp8FgColor);

         BUMP(5);
         CMDFIFO_EPILOG(cmdFifo);
         dwXprntFlg = 0L;
         if (lpDrawMode->bkMode & BKMODE_TRANSPARENT)
         {
            dwXprntFlg = SSTG_TRANSPARENT;
         }
         return (SSTG_MONO_PATTERN | dwXprntFlg |
               ((Rop &0xffff0000) << 8) );
         break;

      case BS_PATTERN:
process_pattern:
         /* cases of concern:
            monosolid
               how can you have solid mono?
            monovalid
               is there a case where monovalid is set and
               patternmono not?
            patternmono
               if patternmono is set and mono valid is not
               do I use color brush?
         assuming that the pattern is mono only if MONOVALID AND
         PATTERN MONO IS SET
         */

         #ifdef DEBUG
         //decodebrushflag(lpBrush);

         /*   When a CreatePatternBrush is used, a brush with
               bits MONOVALID and not PATTERN MONO is given to
            the bitblt call.  There appears to be no way to
            create a mono brush from without using mono bitmap
            so it appears that the correct filter is to check
            PATTERNMONO for monochrome brushes.
         */

         if ((lpBrush->dp8BrushFlags & PATTERNMONO) &&
            !(lpBrush->dp8BrushFlags & MONOVALID) )
         {
            __asm int 3
         //   DPF ("Pattern blt:PATTERNMONO AND NOT MONOVALID ");
         }
         #endif


         // color case
         if ( !(lpBrush->dp8BrushFlags & PATTERNMONO) )
         {
            //DPF(" BS_PATTERN COLOR ");

#ifdef DEBUG
            if ( !(lpBrush->dp8BrushBpp && 7) )

            {
               __asm int 3
            }
#endif
            // in BYTES
	    brushBpp = lpBrush->dp8BrushBpp;
	    if (brushBpp == 20)
		brushBpp = 16;
	    
            patcount = (64 * brushBpp >> 3) ;
            dwpatcount = (DWORD) (patcount >> 2);
            CMDFIFO_CHECKROOM(cmdFifo,dwpatcount+1);
            SETPH(cmdFifo, SSTCP_PKT1|
                        SSTCP_PKT1_2D|
                        SSTCP_INC|
                        // register number starts with clipmin0(2)
                        PATTERN_REG_1 << SSTCP_REGBASE_SHIFT|
                        dwpatcount << SSTCP_PKT1_NWORDS_SHIFT);
            //for(i = 0; i < (64 * lpBrush->dp8BrushBpp >> 3) ; i+=4)
            //{
            //   dwtemp =  *(DWORD FAR *)&lpBrush->dp8BrushBits[i];
            //   SET(cmdFifo, PTR(cp+i+4) , _FF(lpGRegs)->colorPattern[i>>2], dwtemp);
            //}

            for(i = 0; i < dwpatcount; i++)
            {
               SET(cmdFifo,_FF(lpGRegs)->colorPattern[i], lpPattern[i]);

            }
            BUMP(dwpatcount+1);
            CMDFIFO_EPILOG(cmdFifo);
            return( ((Rop & 0xFFFF0000) << 8) );
               //  hardware aligns pattern to screen
               //  no pattern alignment needed.
         }
         else
         {
         // mono case
         // The mono brush is declared as BYTE +BRUSHSIZE*4]
         // which would imply dibeng realizes mono brushes
         // as dword packed.
   //      DPF(" BS_PATTERN  MONO");
         for (i = 0; i < 4; i++)
         {
            mono0[i] = (lpBrush->dp8BrushMono[4*i]);
            mono1[i] = (lpBrush->dp8BrushMono[4*i+16]);
         }
         CMDFIFO_CHECKROOM(cmdFifo,5);
         SETPH(cmdFifo, SSTCP_PKT2|
                   pattern0aliasBit|
                   pattern1aliasBit|
                   colorBackBit|
                   colorForeBit);
         SET(cmdFifo, _FF(lpGRegs)->pattern0alias, *(DWORD*) mono0 );
         SET(cmdFifo, _FF(lpGRegs)->pattern1alias, *(DWORD*) mono1 );

         SET(cmdFifo, _FF(lpGRegs)->colorBack, lpDrawMode->TextColor );
         SET(cmdFifo, _FF(lpGRegs)->colorFore, lpDrawMode->bkColor);

         BUMP(5);
         CMDFIFO_EPILOG(cmdFifo);
         dwtemp = ( ((Rop & 0xFFFF0000) << 8) |
               SSTG_MONO_PATTERN );
         return dwtemp;

         }


         break;

      case BS_INDEXED:
         //DPF(" BS_INDEXED ");
         __asm int 3
         break;

      case BS_DIBPATTERN:
         //DPF(" BS_DIBPATTERN ");
         //INT_3   ;
         patcount = (64 * lpBrush->dp8BrushBpp >> 3) ;
         dwpatcount = (DWORD) (patcount >> 2);
         CMDFIFO_CHECKROOM(cmdFifo, dwpatcount+1);
         SETPH(cmdFifo, SSTCP_PKT1|
                     SSTCP_PKT1_2D|
                     SSTCP_INC|
                     // register number starts with clipmin0(2)
                     PATTERN_REG_1 << SSTCP_REGBASE_SHIFT|
                     dwpatcount << SSTCP_PKT1_NWORDS_SHIFT);

         //for(i = 0; i < (64 * lpBrush->dp8BrushBpp >> 3) ; i+=4)
         for(i = 0; i < dwpatcount ; i++)
         {
            dwtemp =  lpPattern[i];
            SET(cmdFifo, _FF(lpGRegs)->colorPattern[i], dwtemp);
         }
         BUMP(dwpatcount + 1);
         CMDFIFO_EPILOG(cmdFifo);
         return( ((Rop & 0xFFFF0000) << 2) );

         break;

      case BS_DIBPATTERNPT:
         //DPF(" BS_DIBPATTERNPT ");
         __asm int 3
         break;

      case BS_PATTERN8X8:
         //DPF(" BS_PATTERN8X8 ");
         __asm int 3
         break;

      case BS_DIBPATTERN8X8:
         //DPF(" BS_DIBPATTERN8X8 ");
         __asm int 3
         break;

   }
return 1;
}


/*----------------------------------------------------------------------
Function name:  FxBeginAccess32

Description:    The BeginAccess function
                
Information:    
    Args:
        LPDIBENGINE lpDst
        int left
        int top
        int right
        int bottom
        DWORD flags

Return:         DWORD   1 is always returned.
----------------------------------------------------------------------*/
DWORD FxBeginAccess32(DWORD flags)
{
   DWORD srcFormat;
   int Xsize;
   int Ysize;
   int X;
   int Y;
   int nSize;
   DWORD PktHeader;
   CMDFIFO_PROLOG(cmdFifo);
   DEBUG_FIX;

   X = _FF(LastCursorPosX);
   Y = _FF(LastCursorPosY);
   Xsize = SWCURSOR_WIDTH;
   Ysize = SWCURSOR_HEIGHT;
   if (X < 0)
      {
      Xsize += X;
      X = 0;
      }

   if (X + SWCURSOR_WIDTH > _FF(hres))
      {
      Xsize = _FF(hres) - X;
      }

   if (Y < 0)
      {
      Ysize += Y;
      Y = 0;
      }

   if (Y + SWCURSOR_WIDTH > _FF(vres))
      {
      Ysize = _FF(vres) - Y;
      }

   // Do we need to display anything?
   if ((Xsize > 0) && (Ysize > 0))
      {
      CMDFIFO_SETUP(cmdFifo);

       //exclude cursor

//   DPF(DBG_DEBUG, 100, "FxBeginAccess32\n\r");
      _FF(gdiFlags) |= SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED;

      nSize = 12;
      PktHeader = SSTCP_PKT2 | dstBaseAddrBit | dstFormatBit | srcBaseAddrBit |
         srcFormatBit | srcXYBit | dstSizeBit | dstXYBit | commandBit;

      if (_FF(gdiFlags) & SDATA_GDIFLAGS_DST_WAS_DEVBIT)
         {
         nSize += 3;
         PktHeader = PktHeader | clip0minBit | clip0maxBit | commandExBit;
         }

      CMDFIFO_CHECKROOM(cmdFifo, nSize);
      SETPH(cmdFifo, PktHeader);

      if (PktHeader & clip0minBit)
         {
      	SET(cmdFifo, _FF(lpGRegs)->clip0min, 0x0);
   	   SET(cmdFifo, _FF(lpGRegs)->clip0max, (_FF(bi).biHeight << 16) | _FF(bi).biWidth);   
         }

      SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr,_FF(gdiDesktopStart));
      SET(cmdFifo, _FF(lpGRegs)->dstFormat, _FF(screenFormat));

      SET(cmdFifo, _FF(lpGRegs)->srcBaseAddr, _FF(SWcursorExclusionStart));

      if (PktHeader & commandExBit)
         {
         SET(cmdFifo, _FF(lpGRegs)->commandEx, 0x0);
         }

      srcFormat = _FF(screenFormat);
      srcFormat &= ~(SSTG_SRC_PACK);
      srcFormat &= 0xFFFFC000;
      srcFormat |= EXCLUSION_BMP_STRIDE;
      SET(cmdFifo, _FF(lpGRegs)->srcFormat, srcFormat);
      SET(cmdFifo, _FF(lpGRegs)->srcXY, 0UL);


      SET(cmdFifo, _FF(lpGRegs)->dstSize, (Ysize << 16) | Xsize);
      SET(cmdFifo, _FF(lpGRegs)->dstXY, (Y<<16) | X);
      SETC(cmdFifo, _FF(lpGRegs)->command,
          SSTG_GO |
          0xCC000000 |
          SSTG_BLT);

      BUMP(nSize - 3);

      // restore srcBaseAddr and srcFormat
      SETPH(cmdFifo,   SSTCP_PKT2 | srcBaseAddrBit | srcFormatBit);
      SET(cmdFifo,_FF(lpGRegs)->srcBaseAddr, _FF(gdiDesktopStart));
      SET(cmdFifo,_FF(lpGRegs)->srcFormat, _FF(screenFormat));
      BUMP(3);
      CMDFIFO_EPILOG(cmdFifo);

#ifdef DEBUG_CURSOR
      GottaCursor32();
#endif
   }

   return 1;
}


/*----------------------------------------------------------------------
Function name:  FxEndAccess32

Description:    The EndAccess function
                
Information:    
    Args:
        LPDIBENGINE lpDst
        int left
        int top
        int right
        int bottom
        DWORD flags

Return:         DWORD   1 is always returned.
----------------------------------------------------------------------*/
DWORD FxEndAccess32(DWORD flags)
{
   int Xsize;
   int Ysize;
   int X;
   int Y;
   int Xcur;
   int Ycur;
   int X1;
   int Y1;
   int nSize;
   DWORD PktHeader;
   DWORD  dstFormat;
   DWORD drawCursor;
   CMDFIFO_PROLOG(cmdFifo);
   DEBUG_FIX;
   CMDFIFO_SETUP(cmdFifo);

   X1 = X = _FF(CursorPosX) - _FF(HotspotX);
   Y1 = Y = _FF(CursorPosY) - _FF(HotspotY);
   Xsize = SWCURSOR_WIDTH;
   Ysize = SWCURSOR_HEIGHT;
   Xcur=0;
   Ycur=0;
   if (X < 0)
      {
      Xsize += X;
      Xcur = SWCURSOR_WIDTH - Xsize;
      X = 0;
      }

   if (X + SWCURSOR_WIDTH > _FF(hres))
      {
      Xsize = _FF(hres) - X;
      }

   if (Y < 0)
      {
      Ysize += Y;
      Ycur = SWCURSOR_HEIGHT - Ysize;
      Y = 0;
      }

   if (Y + SWCURSOR_WIDTH > _FF(vres))
      {
      Ysize = _FF(vres) - Y;
      }

   if ((Xsize > 0) && (Ysize > 0))
      {
      drawCursor = ((flags & CURSOREXCLUDE) && (SDATA_GDIFLAGS_CURSOR_ENABLED == (SDATA_GDIFLAGS_CURSOR_ENABLED & _FF(gdiFlags))));
      //   DPF(DBG_DEBUG, 100, "FxBeginAccess32\r\n");
      // offscreen mem for cursor exclusion is always dwordpacked
      dstFormat = _FF(screenFormat);
      dstFormat &= ~(SSTG_SRC_PACK);
      // stride of offscreen cursorexclusion is 64 dwords
      dstFormat &= 0xFFFFC000;
      dstFormat |= EXCLUSION_BMP_STRIDE;

      nSize = 9;
      PktHeader = SSTCP_PKT2 | dstBaseAddrBit | dstFormatBit | srcBaseAddrBit |
            srcFormatBit | srcXYBit | dstSizeBit | dstXYBit | commandBit;

      if (_FF(gdiFlags) & SDATA_GDIFLAGS_DST_WAS_DEVBIT)
         {
         nSize += 3;
         PktHeader = PktHeader | clip0minBit | clip0maxBit | commandExBit;
         }

      CMDFIFO_CHECKROOM(cmdFifo, nSize);
      SETPH(cmdFifo, PktHeader);

      if (PktHeader & clip0minBit)
         {
      	SET(cmdFifo, _FF(lpGRegs)->clip0min, 0x0);
   	   SET(cmdFifo, _FF(lpGRegs)->clip0max, (_FF(bi).biHeight << 16) | _FF(bi).biWidth);   
         }

      SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr, _FF(SWcursorExclusionStart));
      SET(cmdFifo, _FF(lpGRegs)->dstFormat, dstFormat);
      SET(cmdFifo, _FF(lpGRegs)->srcBaseAddr, _FF(gdiDesktopStart));

      if (PktHeader & commandExBit)
         {
         SET(cmdFifo, _FF(lpGRegs)->commandEx, 0x0);
         }

      SET(cmdFifo, _FF(lpGRegs)->srcFormat, _FF(screenFormat));
      SET(cmdFifo, _FF(lpGRegs)->srcXY,(Y<<16) | X);
      SET(cmdFifo, _FF(lpGRegs)->dstSize, (Ysize << 16) | Xsize);
      SET(cmdFifo, _FF(lpGRegs)->dstXY, 0L);
      SETC(cmdFifo, _FF(lpGRegs)->command,
          SSTG_GO |
          0xCC000000 |
          SSTG_BLT);

      BUMP(nSize);

      if (drawCursor)
         {

         // Get Screen
         CMDFIFO_CHECKROOM(cmdFifo, 12);
         SETPH(cmdFifo, SSTCP_PKT2|
                clip0minBit|
                clip0maxBit|
                dstBaseAddrBit|
                dstFormatBit|
                srcBaseAddrBit|
                commandExBit|
                srcFormatBit|
                srcXYBit|
                dstSizeBit|
                dstXYBit|
                commandBit);
        
      	SET(cmdFifo, _FF(lpGRegs)->clip0min, 0);
      	SET(cmdFifo, _FF(lpGRegs)->clip0max, (_FF(bi).biHeight << 16) | _FF(bi).biWidth);   
         SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr, _FF(SWcursorSrcStart));
         SET(cmdFifo, _FF(lpGRegs)->dstFormat, dstFormat);
         SET(cmdFifo, _FF(lpGRegs)->srcBaseAddr, _FF(gdiDesktopStart));
         SET(cmdFifo, _FF(lpGRegs)->commandEx, 0x0);
         SET(cmdFifo, _FF(lpGRegs)->srcFormat, _FF(screenFormat));

         SET(cmdFifo, _FF(lpGRegs)->srcXY, ((DWORD) Y << 16) | (DWORD) X );
         SET(cmdFifo, _FF(lpGRegs)->dstSize, ((DWORD) Ysize << 16) | (DWORD)Xsize );
         SET(cmdFifo, _FF(lpGRegs)->dstXY,((DWORD) Ycur << 16) | (DWORD)Xcur);
         SETC(cmdFifo, _FF(lpGRegs)->command,
              SSTG_GO |
              0xCC000000 |
              SSTG_BLT);

         BUMP(12);        
         CMDFIFO_CHECKROOM(cmdFifo, 7);
         SETPH(cmdFifo, SSTCP_PKT2|
               srcBaseAddrBit     |
               srcFormatBit       |
               srcXYBit           |
               dstSizeBit         |
               dstXYBit           |
               commandBit
               );  
        
         SET(cmdFifo,  _FF(lpGRegs)->srcBaseAddr, _FF(SWcursorAndStart));
         SET(cmdFifo,  _FF(lpGRegs)->srcFormat, _FF(SWcursorFormat));
         SET(cmdFifo,  _FF(lpGRegs)->srcXY, ((DWORD) Ycur << 16) | (DWORD)Xcur);
         SET(cmdFifo,  _FF(lpGRegs)->dstSize , ((DWORD) Ysize << 16) | (DWORD)Xsize );
         SET(cmdFifo,  _FF(lpGRegs)->dstXY, ((DWORD) Ycur << 16) | (DWORD)Xcur);
         SETC(cmdFifo,  _FF(lpGRegs)->command,
              SSTG_GO|
              SSTG_BLT|
              SSTG_ROP_AND << SSTG_ROP0_SHIFT);
        
         BUMP(7);

         CMDFIFO_CHECKROOM(cmdFifo, 3);
         SETPH(cmdFifo, SSTCP_PKT2|
               srcBaseAddrBit     |
               commandBit
               );  
        
         SET(cmdFifo,  _FF(lpGRegs)->srcBaseAddr, _FF(SWcursorXorStart));
         SETC(cmdFifo,  _FF(lpGRegs)->command,
              SSTG_GO|
              SSTG_BLT|
              SSTG_ROP_XOR << SSTG_ROP0_SHIFT);
           
         BUMP(3);

         CMDFIFO_CHECKROOM(cmdFifo, 7);
         SETPH(cmdFifo, SSTCP_PKT2|
               dstBaseAddrBit     |
               dstFormatBit       |
               srcBaseAddrBit     |
               srcFormatBit       |
               dstXYBit           |
               commandBit
               );  
        
         SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr, _FF(gdiDesktopStart));
         SET(cmdFifo, _FF(lpGRegs)->dstFormat, _FF(screenFormat));
         SET(cmdFifo,  _FF(lpGRegs)->srcBaseAddr, _FF(SWcursorSrcStart));
         SET(cmdFifo, _FF(lpGRegs)->srcFormat, dstFormat);
         SET(cmdFifo,  _FF(lpGRegs)->dstXY,( ((DWORD)(Y & 0x1FFF) <<16) ) | (DWORD)(X & 0x1FFF));
         SETC(cmdFifo,  _FF(lpGRegs)->command,
              SSTG_GO|
              SSTG_BLT|
              0xCC000000);
        
         BUMP(7);
         nSize = 3;
         PktHeader = SSTCP_PKT2 | srcBaseAddrBit | srcFormatBit;
         }
      else
         {
         nSize = 3;
         PktHeader = SSTCP_PKT2 | dstBaseAddrBit | dstFormatBit;
         if (_FF(gdiFlags) & SDATA_GDIFLAGS_SRC_WAS_DEVBIT)
            {
            nSize += 2;
            PktHeader = PktHeader | srcBaseAddrBit | srcFormatBit;
            }
         }

      CMDFIFO_CHECKROOM(cmdFifo, nSize);
      SETPH(cmdFifo, PktHeader);

      if (PktHeader & dstBaseAddrBit)
         {
         SET(cmdFifo,_FF(lpGRegs)->dstBaseAddr, _FF(gdiDesktopStart));
         SET(cmdFifo,_FF(lpGRegs)->dstFormat, _FF(screenFormat));
         }

      if (PktHeader & srcBaseAddrBit)
         SET(cmdFifo,_FF(lpGRegs)->srcBaseAddr, _FF(gdiDesktopStart));

      // Fixed Moved Here so that we can clear some gdiflags 
      // restore commandEx - defects 1978 and 1829. If we don't restore
      // commandEx, the source compare operation remains enabled. Then,
      // any matching colors cause problems. (eg. A memory to screen
      // blit of a background bitmap!)
      if (PktHeader & commandExBit)
         SET(cmdFifo, _FF(lpGRegs)->commandEx, 0x0);

      if (PktHeader & srcFormatBit)
         SET(cmdFifo,_FF(lpGRegs)->srcFormat, _FF(screenFormat));

      BUMP(nSize);

      CMDFIFO_EPILOG(cmdFifo);
#ifdef DEBUG_CURSOR
      GottaCursor32();
#endif

      _FF(gdiFlags) &= ~(SDATA_GDIFLAGS_CURSOR_EXCLUDE | SDATA_GDIFLAGS_CURSOR_IS_EXCLUDED |
               SDATA_GDIFLAGS_DST_WAS_DEVBIT | SDATA_GDIFLAGS_SRC_WAS_DEVBIT);
      }

   // Current Cursor Position May have been updated so use the ones that are
   // most accurate
#if 0
   _FF(LastCursorPosX) = X1 + _FF(HotspotX);
   _FF(LastCursorPosY) = Y1 + _FF(HotspotY);
#else
   _FF(LastCursorPosX) = X1;
   _FF(LastCursorPosY) = Y1;
#endif
   return 1;

}


/*----------------------------------------------------------------------
Function name:  GottaCursor32

Description:    This is a debug routine to find spots where we
                copy the cursor into the save under buffer.
                This is a no-no.

Information:    Surrounded by #ifdef DEBUG_CURSOR.

Return:         DWORD   1 for succcess, 0 for failure.
----------------------------------------------------------------------*/
#ifdef DEBUG_CURSOR
int nGottaCursor = 0;
int GottaCursor32()
{
   DWORD dwSave;
   DWORD dwCursor;
   DWORD dw1;
   DWORD dw2;
   DWORD dwMask;
   int i;
   int j;
   int nReturn;
   int nOffset;
   int nCount;
   int nTotal;

   DEBUG_FIX;

   nReturn = 0;
   nCount = 0;
   nTotal = 0;
   dwSave = _FF(lfbBase) + _FF(SWcursorExclusionStart);
   dwCursor = _FF(lfbBase) + _FF(SWcursorBitmapStart);

   FXWAITFORIDLE();
   
   nOffset = 0;
   for (j=0; j<32; j++)
      {
      for (i=0; i<16; i++)
         {
         dw1 = *(DWORD *)(dwCursor + nOffset + (i<<2));
         if (dw1 == 0x13141314L)
            continue;
         if ((dw1 & 0xFFFF0000L) == 0x13140000L)
            dwMask = 0xFFFFL;
         else if ((dw1 & 0xFFFFL) == 0x1314L)
            dwMask = 0xFFFF0000L;
         else
            dwMask = 0xFFFFFFFFL;

         dw2 = *(DWORD *)(dwSave + nOffset + (i<<2));

         if ((dw1 & dwMask) == (dw2 & dwMask))
            nCount++;
         nTotal++;
         }

      nOffset += 0x80;
      }

   if ((nTotal) && (nTotal == nCount))
      {
      nReturn = 0x01;
      if (nGottaCursor)
         __asm  int 03
      }

   return nReturn;
}
#endif


/*----------------------------------------------------------------------
Function name:  DoIntersect

Description:    Evaluate whether or not two rectangles intersect.
                One rectange is always the cursor rect.  Routine
                used bitmask to evalulate all possibilities at once.
Information:    

Return:         DWORD   TRUE if interect, FALSE if no intersect.
----------------------------------------------------------------------*/
int IsIntersect[] = {  // y2 x2 y1 x1
   FALSE,            // 0  0  0  0
   FALSE,            // 0  0  0  1
   FALSE,            // 0  0  1  0
   TRUE,             // 0  0  1  1
   FALSE,            // 0  1  0  0
   FALSE,            // 0  1  0  1
   TRUE,             // 0  1  1  0
   TRUE,             // 0  1  1  1
   FALSE,            // 1  0  0  0
   TRUE,             // 1  0  0  1
   FALSE,            // 1  0  1  0
   TRUE,             // 1  0  1  1
   TRUE,             // 1  1  0  0
   TRUE,             // 1  1  0  1
   TRUE,             // 1  1  1  0
   TRUE,             // 1  1  1  1
   };
#define RANGE(val,lo,hi) ((lo) <= (val) && (val) <= (hi))
int DoIntersect (int left, int top, int right, int bottom)
{
   int bIntersect;
   int cl, cr, ct, cb;
   DEBUG_FIX;

#if 0
   cl = _FF(LastCursorPosX) - _FF(HotspotX);
   cr = cl + SWCURSOR_WIDTH;
   ct = _FF(LastCursorPosY) - _FF(HotspotY);
   cb = ct + SWCURSOR_HEIGHT;
#else
   cl = _FF(LastCursorPosX);
   cr = cl + SWCURSOR_WIDTH;
   ct = _FF(LastCursorPosY);
   cb = ct + SWCURSOR_HEIGHT;
#endif

   if (right - left <= SWCURSOR_WIDTH)
      {
      bIntersect = RANGE(left, cl, cr);
      bIntersect |= (RANGE(right, cl, cr) << 2);
      }
   else
      {
      bIntersect = RANGE(cl, left, right);
      bIntersect |= (RANGE(cr, left, right) << 2);
      }

   if (bottom - top <= SWCURSOR_WIDTH)
      {
      bIntersect |= (RANGE(top, ct, cb) << 1);
      bIntersect |= (RANGE(bottom, ct, cb) << 3);
      }
   else
      {
      bIntersect |= (RANGE(ct, top, bottom) << 1);
      bIntersect |= (RANGE(cb, top, bottom) << 3);
      }

   return IsIntersect[bIntersect];
}
