/* -*-c++-*- */
/* $Header: hardware.c, 14, 10/11/00 8:54:35 PM, Brent$ */
/*
** Copyright (c) 1997-1999, 3Dfx Interactive, Inc.
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
** File name:   hardware.c
**
** Description: Misc. HW specific functions.
**
** $Revision: 14$
** $Date: 10/11/00 8:54:35 PM$
**
** $History: hardware.c $
** 
** *****************  Version 69  *****************
** User: Cwilcox      Date: 9/09/99    Time: 5:05p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Fixed lfbMemoryConfig accesses to support 32Mb and 64Mb configs.
** 
** *****************  Version 68  *****************
** User: Andrew       Date: 7/26/99    Time: 5:51p
** Updated in $/devel/h5/Win9x/dx/minivdd
** removed unused members and fixes for SLI/AA
** 
** *****************  Version 67  *****************
** User: Andrew       Date: 7/20/99    Time: 10:49a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Removed some code that referenced variables that were previously
** removed.
** 
** *****************  Version 66  *****************
** User: Cwilcox      Date: 7/16/99    Time: 4:26p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Removed obsolete heap*Start references.
** 
** *****************  Version 65  *****************
** User: Andrew       Date: 7/16/99    Time: 2:06p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Changed regBase and RegBase from single dword to array to support
** sparse register mapping
** 
** *****************  Version 64  *****************
** User: Andrew       Date: 7/09/99    Time: 4:24p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added new code to Set/Clear BUSY Bit for Low Power Modes
** 
** *****************  Version 63  *****************
** User: Andrew       Date: 7/07/99    Time: 2:43p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added for for DibEngine workaround
** 
** *****************  Version 61  *****************
** User: Andrew       Date: 6/25/99    Time: 9:59a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Code added to support SLI/AA
** 
** *****************  Version 60  *****************
** User: Andrew       Date: 6/17/99    Time: 12:42p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Removed HAL_CSIM ifdef
** 
** *****************  Version 59  *****************
** User: Cwilcox      Date: 6/16/99    Time: 10:30a
** Updated in $/devel/h5/Win9x/dx/minivdd
** Removed #ifdef H3_AGP_WORKAROUND code.
** 
** *****************  Version 58  *****************
** User: Andrew       Date: 6/15/99    Time: 4:58p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added code to set/clear Master/Slave Bits
** 
** *****************  Version 57  *****************
** User: Andrew       Date: 6/04/99    Time: 4:13p
** Updated in $/devel/h5/Win9x/dx/minivdd
** Added Code to clear Slave Flag
** 
** *****************  Version 55  *****************
** User: Andrew       Date: 5/13/99    Time: 4:15p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to use Fakelfb for WIN_CSIM CMDFIFO
** 
** *****************  Version 54  *****************
** User: Andrew       Date: 5/13/99    Time: 12:36p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added DEBUG_FIX so that function will work
** 
** *****************  Version 53  *****************
** User: Andrew       Date: 5/13/99    Time: 11:55a
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code to FXWAITFORIDLE so that spin on Status would not result in
** byte reads.
** 
** *****************  Version 52  *****************
** User: Andrew       Date: 5/10/99    Time: 1:36p
** Updated in $/devel/h3/Win95/dx/minivdd
** Changed PhysScreenAddr to RealregBase
** 
** *****************  Version 51  *****************
** User: Andrew       Date: 5/06/99    Time: 4:38p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added code for New Windows "C" Simulator
** 
** *****************  Version 50  *****************
** User: Cwilcox      Date: 4/22/99    Time: 2:18p
** Updated in $/devel/h3/Win95/dx/minivdd
** Removed code to setup memory timing, should be in BIOS.
** 
** 
** *****************  Version 49  *****************
** User: Stb_bseitsin Date: 4/09/99    Time: 12:39p
** Updated in $/devel/h3/win95/dx/minivdd
** Added Napalm registers. Added ifdef H5.
** 
** *****************  Version 48  *****************
** User: Andrew       Date: 3/17/99    Time: 4:41p
** Updated in $/devel/h3/Win95/dx/minivdd
** Added in a ifdef so that CmdFifo is 15 & 8 for Avenger and 9 & 2 for
** Banshee
** 
** *****************  Version 47  *****************
** User: Andrew       Date: 3/17/99    Time: 2:49p
** Updated in $/devel/h3/Win95/dx/minivdd
** changed FifoThresh to 15 & 8
** 
** *****************  Version 46  *****************
** User: Stb_skephart Date: 2/22/99    Time: 12:29p
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 45  *****************
** User: Stb_skephart Date: 2/19/99    Time: 6:08a
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 44  *****************
** User: Cwilcox      Date: 2/18/99    Time: 11:48a
** Updated in $/devel/h3/Win95/dx/minivdd
** Final removal of tiled/linear promotion.
** 
** *****************  Version 43  *****************
** User: Stb_skephart Date: 2/17/99    Time: 7:06p
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 42  *****************
** User: Stb_skephart Date: 2/17/99    Time: 3:49p
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 41  *****************
** User: Cwilcox      Date: 2/10/99    Time: 3:45p
** Updated in $/devel/h3/Win95/dx/minivdd
** Linear versus tiled promotion removal.
** 
** *****************  Version 40  *****************
** User: Stb_srogers  Date: 1/29/99    Time: 8:06a
** Updated in $/devel/h3/win95/dx/minivdd
** 
** *****************  Version 39  *****************
** User: Michael      Date: 1/08/99    Time: 1:51p
** Updated in $/devel/h3/Win95/dx/minivdd
** Implement the 3Dfx/STB unified header.
** 
** 38    11/04/98 2:17a Artg
** mysetcf and my setdf now uses h3write for csim builds.
** Allows DF option to work
** 
** 37    10/29/98 2:14p Martin
** Modification of cmdFifo macros to support future changes for
** super-sampling AA.
** 
** 36    8/25/98 12:31a Andrew
** Added a FXWAITFORIDLE to ensure cmdfifo is flushed
** 
** 35    8/07/98 6:44p Ken
** should fix random agp hangs/crashes.   fixes jedi knight menu
** on agp board/driver
** 
** 34    7/24/98 10:38p Ken
** changes to allow 2d driver to run properly synchronized with an AGP
** command fifo (although video memory fifo is still used when the desktop
** has the focus, e.g., a fullscreen 3d app isn't in the foreground)
** 
** 33    7/24/98 7:17p Miriam
** AGP command fifo only enabled for D3D.
** 
** 32    7/23/98 4:04p Ken
** added agp command fifo.   not added to NT build.  currently not
** functional on non-win98 systems without running a special enable apg
** script first, see Ken for that.   agp command fifo is enabled by
** setting the environment variable ACF=1 , all other settings disable it.
** Only turned on interatively in debugger in InitFifo call.   
** 
** 31    7/18/98 6:41p Ken
** added ability to use cmdfifo1 as the primary command fifo, #define
** PRIMARY_CMDFIFO at the top of inc\shared.h
** 
** 30    6/17/98 5:28p Miriam
** Optmize fifo fetch via vidpixelbufthold
** 
** 29    5/18/98 3:54p Ken
** upped command fifo threshold from 8 to 9, AA determined that 8 is
** insufficient.  tested 2d winbench 98 and performance looks the same
** 
** 28    5/15/98 4:35p Michael
** Backout my previous changes.  Add more changes for changing stide when
** tiled vs. linear.
** 
** 27    5/15/98 9:22a Michael
** Modify ResetInvariantState to pass in lpDst.
** 
** 26    5/11/98 5:24p Suninn
** fix aperture stride
** 
** 25    4/28/98 6:17p Artg
** cmdfifo debug save paket header address and data
** 
** 24    4/24/98 1:33p Ken
** first installment of FXENTER / FXLEAVE display driver drawing function
** work.  gdi32 only, no gdi16 yet.   works, but is slow, don't measure
** performance until all work is done, in about a week.
** also renamed fields in some blit parameter functions to be a bit more
** meaningful
** 
** 23    4/21/98 6:52p Ken
** clean up mode set, modes seem to set faster now too
** 
** 22    4/21/98 12:01a Ken
** added agp workaround (set ar=1) for readback unreliability
** 
** 21    4/15/98 6:41p Ken
** added unified header to all files, with revision, etc. info in it
** 
** 20    4/15/98 4:59p Suninn
** move dwordCount into shared.h
** 
** 19    4/03/98 10:51a Artg
** added extra arguments for new dpf
** 
** 18    3/31/98 11:40a Miriam
** Add prints, move data into global to enable global debugging, etc.
*/
//: hardware.c

// STB Begin Changes
#ifdef INCSTBPERF
#include "..\build\stbperf.inc"
#endif
// STB End Changes


#include "thunk32.h"
#include "h3g.h"

DWORD pfnCSIM;

// STB Begin Changes
// STB Begin Changes
// STB-SR 1/28/99 Adding support for 2D AGP Cmd Fifo
// Most of the code was taken from dd32/ddflip.c code
// inside the function TOAFIFO().
#ifdef STBPERF_2DAGPCMDFIFO

#ifdef DEBUG
/*----------------------------------------------------------------------
Function name:   outpd

Description:     put DWORD value dwData to port address

Return:          NONE
----------------------------------------------------------------------*/

void outpd(DWORD addr, DWORD dwData)
{
   DEBUG_FIX;

   _asm {mov   edx, addr}
   _asm {mov   eax, dwData}
   _asm {out   dx, eax}
}

/*----------------------------------------------------------------------
Function name:   outpw

Description:     put WORD value dwData to port address

Return:          NONE  
----------------------------------------------------------------------*/

void outpw(DWORD addr, DWORD dwData)
{
   DEBUG_FIX;

   _asm {mov   edx, addr}
   _asm {mov   eax, dwData}
   _asm {out   dx, ax}
}

/*----------------------------------------------------------------------
Function name:   inpd

Description:     get DWORD value dwData from port address and save to
                 eax to return 

Return:          NONE
----------------------------------------------------------------------*/

DWORD inpd(DWORD addr)
{
   DEBUG_FIX;

   _asm {mov   edx, addr}
   _asm {in   eax, dx}
}

/*----------------------------------------------------------------------
Function name:   inpd

Description:     get WORD value dwData from port address and save to
                 eax to return 

Return:          NONE
----------------------------------------------------------------------*/

WORD inpw(DWORD addr)
{
   DEBUG_FIX;

   _asm {mov   edx, addr}
   _asm {in   ax, dx}
}
#endif
/*----------------------------------------------------------------------
Function name:  DoConfig

Description:    copied from the dd32/ddflip function

Return:         int 

                0 - 
----------------------------------------------------------------------*/

int DoConfig(int id)
{
   DWORD dwSize1, dwSize2;
   DWORD physMem, physFB;
   int BusNum;
   int DevNum;
   int nid;
   WORD nCmd;

   DEBUG_FIX;

   for (BusNum=0; BusNum < 256; BusNum++)
      for (DevNum=0; DevNum<32; DevNum++)
         {
         outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11));
         nid = inpd(0xcfc);
         if (nid == id)
            {
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x4);
            nCmd = inpw(0xcfc);
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x4);
            outpw(0xcfc, nCmd & 0xFFFC);
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x10);
            physMem = inpd(0xcfc) & 0xFF000000;
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x10);
            outpd(0xcfc, 0xFFFFFFFF);
            dwSize1 = ~inpd(0xcfc) + 1;
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x10);
            outpd(0xcfc, physMem);
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x14);
            physFB = inpd(0xcfc) & 0xFF000000;
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x14);
            outpd(0xcfc, 0xFFFFFFFF);
            dwSize2 = ~inpd(0xcfc) + 1;
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x14);
            outpd(0xcfc, physFB);
            outpd(0xcf8, 0x80000000 | (BusNum << 16) | (DevNum << 11) | 0x4);
            outpw(0xcfc, nCmd);
            }
         }

   return 0;
}// DoConfig


#define FXGETBUSYSTATUS()      (GET(_FF(lpHWIOregs)->status) & SST_BUSY)
#define FXBUSYWAIT()           do{;}while((GET(_FF(lpHWIOregs)->status) & SST_BUSY))


#endif // #ifdef STBPERF_2DAGPCMDFIFO
// STB End Changes


#define MYCMDFIFO	_FF(lpCRegs)->PRIMARY_CMDFIFO
#include "agpcf.c"

/*----------------------------------------------------------------------
Function name:  InitFifo

Description:    Initialize the cmd fifo.
                
Information:
    fifobase in physical address of framebuffer
    fifosize in bytes

Return:         VOID
----------------------------------------------------------------------*/
void
InitFifo(FxU32 fifoBase, FxU32 fifoSize)
{
#ifdef CMDFIFO
  FxU32   startVirtualAddr; 
  DWORD   physAddr;
#endif // #ifdef CMDFIFO
  DWORD   baseSize;

  DEBUG_FIX;

  baseSize = (_FF(fifoSize) + 4095) & ~4095;

  FXWAITFORIDLE();

// STB Begin Changes
// STB-SR 1/28/99 Adding support for 2D AGP Cmd Fifo
// Most of the code was taken from dd32/ddflip.c code
// inside the function TOAFIFO().
#ifdef STBPERF_2DAGPCMDFIFO
  if (_FF(enableAGPCF))
  {
      FxU32  startVirtualAddr;
      DWORD fifoLength;

      // Base size should be 4 MB, need to change
      baseSize = (_FF(agpMain.sizeInB) + 4095) & ~4095;

      _FF(doAgpCF) = 1;

      _FF(mainFifo.base)  = (_FF(agpMain.linAddr ) + 4095) & ~4095;
      _FF(mainFifo.start) = _FF(mainFifo.base);

      // disable command fifo 0
      //
      SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.baseSize,  0 );

// MAGIC_SCRIPT was defined for the 9X driver in dd32/ddflip.c
// I asked Andrew Sobczyk about this and he said, "On Banshee, we could not 
// get AGP to work without doing this.  I have no explanation for why it makes 
// AGP work but it does."  The Magic Script code has since moved to the vxd, and
// since I don't think I can call a vxd function from within the 32-bit display
// driver attatched to the VXD, I'm using the old MAGIC SCRIPT function.
//#ifdef MAGIC_SCRIPT
      // Do a config causes this is a good thing
      DoConfig(0x0003121a);
//#endif

      //
      // h/w bug workaround for Swap hang
      //
     if (IS_NAPALM(_FF(VendorDeviceID)))
        {
        SETDW(_FF(lpCRegs)->cmdFifoThresh,             (20 << 5) | 8 );
        }
     else
        {
        SETDW(_FF(lpCRegs)->cmdFifoThresh,             (15 << 5) | 8 );
        }

	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.readPtrL,  _FF(agpMain.physAddr) );
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.readPtrH,  0);
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.aMin,      _FF(agpMain.physAddr) - 4);
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.aMax,      _FF(agpMain.physAddr) - 4);
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.depth,     0);
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.holeCount, 0);

	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.baseAddrL, (_FF(agpMain.physAddr) >> 12) );
	  SETDW(_FF(lpCRegs)->PRIMARY_CMDFIFO.baseSize,  SST_EN_CMDFIFO  |
                                            SST_CMDFIFO_AGP |
                                            SST_CMDFIFO_DISABLE_HOLES);

      startVirtualAddr     = _FF(agpMain.linAddr );
	  fifoLength           = _FF(agpMain.sizeInB );

      if (fifoLength > (4 * 1024L * 1024L))
	      fifoLength = 4 * 1024L * 1024L;
	
      _FF(cmdFifoBasePtr)  = (FxU32)&_FF(mainFifo.base);

	  CMDFIFOPTR           = startVirtualAddr;
	  CMDFIFOSTART         = startVirtualAddr;
	  CMDFIFOSPACE         = (fifoLength / 4) - 3;
	  CMDFIFOEND           = startVirtualAddr + fifoLength - 12;

	  CMDFIFOOFFSET        = startVirtualAddr - _FF(agpMain.physAddr);
	  CMDFIFOJMP           = SSTCP_PKT0_JMP_AGP | (((_FF(agpMain.physAddr) & 0x00FFFFFF) >> 2) << 6);
	  CMDFIFOJMP2          = (_FF(agpMain.physAddr) >> 25) & ((1L << 12) - 1);

	  CMDFIFOEPILOGPTR     = CMDFIFOSTART;
	  CMDFIFOUNBUMPEDWORDS = 0;
	  CMDFIFOBUMP          = 0;

	  _FF(InPacket)          = 0;
	  _FF(WordsLeftInPacket) = 0;
	  _FF(Wrapping)          = 0;
  }
  else
  {
#endif // #ifdef STBPERF_2DAGPCMDFIFO
// STB End Changes


  _FF(doAgpCF) = 0;

  _FF(mainFifo.base)  = (fifoBase + 4095) & ~4095;
#ifdef WIN_CSIM
  _FF(mainFifo.start) = lpDriverData->lfbFakeBase + _FF(mainFifo.base);
#else
  _FF(mainFifo.start) = lpDriverData->lfbBase + _FF(mainFifo.base);
#endif

  // disable command fifo 0
  //
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.baseSize,  0 );

#ifdef CMDFIFO
  //
  // h/w bug workaround for Swap hang
  //
  if (IS_NAPALM(_FF(VendorDeviceID)))
      {
      SETDW(_FF(lpCRegs)->cmdFifoThresh,             (20 << 5) | 8 );
      }
  else
      {
      SETDW(_FF(lpCRegs)->cmdFifoThresh,             (15 << 5) | 8 );
      }

  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.readPtrL,  _FF(mainFifo.base) );
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.readPtrH,  0 );
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.aMin,      _FF(mainFifo.base) - 4 );
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.aMax,      _FF(mainFifo.base) - 4 );
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.depth,     0 );
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.holeCount, 0 );

  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.baseAddrL, _FF(mainFifo.base) >> 12 );
  SETDW( _FF(lpCRegs)->PRIMARY_CMDFIFO.baseSize,  ((baseSize >> 12) - 1) | SST_EN_CMDFIFO );

  startVirtualAddr    = _FF(mainFifo.start);
  physAddr            =  ((GET(_FF(lpCRegs)->PRIMARY_CMDFIFO.baseAddrL) & 0x3FF) << 12);

  _FF(cmdFifoBasePtr) = (FxU32)&_FF(mainFifo.base);

  CMDFIFOPTR          = startVirtualAddr;
  CMDFIFOSTART        = startVirtualAddr;
  CMDFIFOSPACE        = (fifoSize / 4) - 2;
  CMDFIFOEND          = startVirtualAddr + fifoSize - 8;

  CMDFIFOOFFSET       = startVirtualAddr - physAddr;
  CMDFIFOJMP          = SSTCP_PKT0_JMP_LOCAL | ((physAddr >> 2) << 6);

  _FF(InPacket)          = 0;         
  _FF(WordsLeftInPacket) = 0;     
  _FF(Wrapping)          = 0;
  
#endif

// STB Begin Changes
#ifdef STBPERF_2DAGPCMDFIFO
   }
#endif

}


/*----------------------------------------------------------------------
Function name:  InitRegs

Description:    Initialize the chip's registers.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void
InitRegs()
{
    DWORD lfbMemoryConfig;
    DWORD screenSize;
    DWORD vidProcCfg;
    DWORD aperture;
#ifdef WIN_CSIM
    DWORD i;
#endif
    
    DEBUG_FIX;

    _FF(lpCRegs) = (SstCRegs *)(lpDriverData->regBase[HWINFO_SST_CMDFIFOREGS_INDEX]);
    _FF(lpGRegs) = (SstGRegs *)(lpDriverData->regBase[HWINFO_SST_2DREGS_INDEX]);
    _FF(lpIOregs) = (SstIORegs *)(lpDriverData->regBase[HWINFO_SST_IOREGS_INDEX]);

    _FF(cmdFifoBasePtr) = (FxU32)&_FF(mainFifo.base);

    FXWAITFORIDLE();
    
    screenSize = (lpDriverData->bi.biHeight << 16) | lpDriverData->bi.biWidth;

    if (_FF(ddPrimaryInTile))
    {
      lpDriverData->screenFormat = _FF(ddTileStride);
    }
    else
    {
      lpDriverData->screenFormat = _FF(pitch);
    }

    switch( lpDriverData->bpp)
    {
      case 8:  lpDriverData->screenFormat |= SSTG_PIXFMT_8BPP;  break;
      case 15: lpDriverData->screenFormat |= SSTG_PIXFMT_15BPP; break;
      case 16: lpDriverData->screenFormat |= SSTG_PIXFMT_16BPP; break;
      case 24: lpDriverData->screenFormat |= SSTG_PIXFMT_24BPP; break;
      case 32: lpDriverData->screenFormat |= SSTG_PIXFMT_32BPP; break;
    }

    // Clear primary surface (desktop).

    SETDW(_FF(lpGRegs)->commandEx,   0);
    SETDW(_FF(lpGRegs)->colorFore,   0);
    SETDW(_FF(lpGRegs)->clip0min,    0);
    SETDW(_FF(lpGRegs)->clip0max,    screenSize);
    SETDW(_FF(lpGRegs)->dstSize,     screenSize);
    SETDW(_FF(lpGRegs)->dstXY,       0); 
    SETDW(_FF(lpGRegs)->dstFormat,   lpDriverData->screenFormat);
    SETDW(_FF(lpGRegs)->dstBaseAddr, lpDriverData->gdiDesktopStart);
    SETDW(_FF(lpGRegs)->srcBaseAddr, lpDriverData->gdiDesktopStart);
    SETDW(_FF(lpGRegs)->command, 0xCC000000 | SSTG_GO | SSTG_RECTFILL);
    SETDW(_FF(lpIOregs)->vidPixelBufThold, _FF(vidPixelBufThold));

    switch(lpDriverData->ddTilePitch)
    {
      case 1024: aperture=0;break;
      case 2048: aperture=1;break;
      case 4096: aperture=2;break;
      case 8192: aperture=3;break;
      default: aperture=4;
    }       
     // initialize memory config for tile surface
    lfbMemoryConfig = 
       ((lpDriverData->ddTileMark >> 12) & 0x1fff) |        // tile aperture base bits(12:0)
      (((lpDriverData->ddTileMark >> 25) & 0x0003) << 23) | // tile aperture base bits(14:13)
      (aperture << 13L) |   // aperture at 13th bit
      (lpDriverData->ddTileStride << 16L); // at 16th bit
    SETDW(_FF(lpIOregs)->lfbMemoryConfig, lfbMemoryConfig);
#ifdef WIN_CSIM
   //
   // This fixes a page fault problem
   // For some reason when we have multiple chips we always snoop even if it is not enabled
   // Since lfbMemoryConfig can be hosed on the other chips this can cause problems
    for (i=1; i<_FF(dwNumUnits); i++)
      SETDW(((SstIORegs *)_FF(regBase[i * HWINFO_SST_MAX_NUM_CHIPS + HWINFO_SST_IOREGS_INDEX]))->lfbMemoryConfig, lfbMemoryConfig);
#endif

    // now enable the video processor
    //
#ifdef WIN_CSIM
    vidProcCfg = GET(((SstIORegs *)_FF(regRealBase))->vidProcCfg);
    SETDW(((SstIORegs *)_FF(regRealBase))->vidProcCfg, vidProcCfg | SST_VIDEO_PROCESSOR_EN);
#else
    vidProcCfg = GET(_FF(lpIOregs)->vidProcCfg);
    SETDW(_FF(lpIOregs)->vidProcCfg, vidProcCfg | SST_VIDEO_PROCESSOR_EN);
#endif
}


/*----------------------------------------------------------------------
Function name:  ResetInvariantState

Description:    Reset the registers that must not change.
                
Information:
    SIDE EFFECTS: based on the bits in resetMask: the screen's
    pDevice's stride will be updated; the 2d invariant registers
    will be set in the h/w.

Return:         VOID
----------------------------------------------------------------------*/
void
ResetInvariantState(FxU32 resetMask)
{
    FxU32 cmdMask, cmdWords, dstFormat;
    CMDFIFO_PROLOG(cmdFifo);

    DEBUG_FIX;

    cmdWords = 0;
    cmdMask = 0;

    if (resetMask & RESET_PDEV)
    {
	if (_FF(ddPrimaryInTile))
	{
	    _FF(gdiDesktopStart) |= SSTG_IS_TILED;
	    dstFormat = _FF(screenFormat) & ~(SSTG_DST_LINEAR_STRIDE |
					      SSTG_DST_TILE_STRIDE);
	    dstFormat |= _FF(ddTileStride) << SSTG_DST_STRIDE_SHIFT;
	    _FF(screenFormat) = dstFormat;
	}
	else
	{
	    // linear
	    _FF(gdiDesktopStart) &= ~(SSTG_IS_TILED);
	    dstFormat = _FF(screenFormat) & ~(SSTG_DST_LINEAR_STRIDE |
					      SSTG_DST_TILE_STRIDE);
	    dstFormat |= _FF(pitch) << SSTG_DST_STRIDE_SHIFT;
	    _FF(screenFormat) = dstFormat;
	}

	// now change the dib engine's stride so that it can render to
	// the desktop
    } 

    if (resetMask & RESET_DST)
    {
	cmdWords += 5;
	cmdMask |= (clip0minBit | clip0maxBit | dstBaseAddrBit | dstFormatBit |
		    commandExBit);
    }

    if (resetMask & RESET_SRC)
    {
	cmdWords += 1;
	cmdMask |= srcBaseAddrBit;
    }
    
    if (cmdWords > 0)
    {
	cmdWords += 1;
	CMDFIFO_SETUP(cmdFifo);
	CMDFIFO_CHECKROOM(cmdFifo, cmdWords);
	SETPH(cmdFifo, SSTCP_PKT2 | cmdMask);
	if (resetMask & RESET_DST)
	{
	    SET(cmdFifo, _FF(lpGRegs)->clip0min, 0);
	    SET(cmdFifo, _FF(lpGRegs)->clip0max,
		(lpDriverData->bi.biHeight << 16) |
		lpDriverData->bi.biWidth);   
	    SET(cmdFifo, _FF(lpGRegs)->dstBaseAddr,
		lpDriverData->gdiDesktopStart);
	    SET(cmdFifo, _FF(lpGRegs)->dstFormat, lpDriverData->screenFormat);
	}
	if (resetMask & RESET_SRC)
	{
	    SET(cmdFifo, _FF(lpGRegs)->srcBaseAddr,
		lpDriverData->gdiDesktopStart);
	}
	if (resetMask & RESET_DST)
	{
	    SET(cmdFifo, _FF(lpGRegs)->commandEx, 0);
	}
	
	BUMP(cmdWords);

	CMDFIFO_EPILOG(cmdFifo);   
    }

}



#pragma optimize("", off)
/*----------------------------------------------------------------------
Function name:  FXWAITFORIDLE

Description:    Wait while HW is busy.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void
FXWAITFORIDLE()
{
  FxU32 dwStatus;
 
  DEBUG_FIX;

  P6FENCE; // Flush write combine buffers

  if (_FF(doAgpCF) && CMDFIFOUNBUMPEDWORDS)
  {
    bumpAgp( CMDFIFOUNBUMPEDWORDS );
    
    do
    {
       dwStatus = GET(_FF(lpIOregs)->status);
    }
    while (dwStatus & SST_PCIFIFO_BUSY);
    
    while (GET(_FF(lpCRegs)->PRIMARY_CMDFIFO.depth) > 0);
  }

  do
  {
    dwStatus = GET(_FF(lpIOregs)->status);
  }
  while (dwStatus & SST_BUSY);

}
#pragma optimize("", on)

#ifdef SLI_AA
/*----------------------------------------------------------------------
Function name:  HardwareClearBit

Description:    Clear Slave Bit. Placed here since it knows THUNK_32 structures.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void HardwareClearBit(GLOBALDATA * lpDriverData)
{
   if (0x0 != lpDriverData)
      lpDriverData->gdiFlags &= ~(SDATA_GDIFLAGS_SLI_AA_SLAVE | SDATA_GDIFLAGS_SLI_AA_MASTER);
}

/*----------------------------------------------------------------------
Function name:  HardwareSetBit

Description:    Set Slave Bit. Placed here since it knows THUNK_32 structures.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void HardwareSetBit(GLOBALDATA * lpDriverData)
{
   if (0x0 != lpDriverData)
      {
      if ((SLI_AA_MASTER_DEVICE == lpDriverData->dwType) || (SLI_AA_REGULAR_DEVICE == lpDriverData->dwType))
         {
         if (!(lpDriverData->gdiFlags & SDATA_GDIFLAGS_HWC_EXCLUSIVE))
            lpDriverData->gdiFlags |= SDATA_GDIFLAGS_SLI_AA_MASTER;
         }
      else
         {
         if (!(lpDriverData->gdiFlags & SDATA_GDIFLAGS_HWC_EXCLUSIVE))
            lpDriverData->gdiFlags |= SDATA_GDIFLAGS_SLI_AA_SLAVE;
         }
      }
}
/*----------------------------------------------------------------------
Function name:  ClearBusyBit

Description:    Set the BUSY Bit.  Used when we go into Low Power
mode for SLI_AA Slaves
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void ClearBusyBit(GLOBALDATA * lpDriverData)
{
   if ((0x0 != lpDriverData) && (0x0 != lpDriverData->lpDeFlags))
      *lpDriverData->lpDeFlags &= ~BUSY;
}

/*----------------------------------------------------------------------
Function name:  SetBusyBit

Description:    Set the BUSY Bit.  Used when we go into Low Power
mode for SLI_AA Slaves
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void SetBusyBit(GLOBALDATA * lpDriverData)
{
   FxU32 *pDeFlags;

   if ((0x0 != lpDriverData) && (0x0 != lpDriverData->lpDeFlags))
      {
      pDeFlags = lpDriverData->lpDeFlags;
      __asm {
            mov   ebx, pDeFlags
            bts   WORD PTR [ebx], BUSY_BIT
            }
      }
}
#endif


//
// externals used in FXENTER / FXLEAVE
// 
char *fxEnterProcName;
FxU32 _SrcFormat;

#ifdef SLI_AA
GLOBALDATA DriverData;
/*----------------------------------------------------------------------
Function name:  GetFBAddr

Description:    Get Offset to start of Surface Currently Being Display
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
DWORD GetFBAddr(GLOBALDATA * lpDriverData)
{
   DWORD dwPitch;
   DWORD dwOffset;
   DWORD tileInX;
   DWORD tileInY;
   DWORD vidProcConfig;
   DWORD dwDisplayStart;
   DWORD dwTiled;   
   
   dwOffset = 0x0;   
   if (0x0 != lpDriverData)
      {
      vidProcConfig = GET(lpDriverData->lpIOregs->vidProcCfg);
      if (vidProcConfig & SST_DESKTOP_EN)
         {
         dwTiled = vidProcConfig & SST_DESKTOP_TILED_EN;
         dwDisplayStart = GET(lpDriverData->lpIOregs->vidDesktopStartAddr);
         }
      else
         {
         dwTiled = vidProcConfig & SST_OVERLAY_TILED_EN;
         dwDisplayStart = GET(lpDriverData->lpIOregs->vidCurrOverlayStartAddr);
         }

      if (dwTiled)
         {
         dwPitch = lpDriverData->ddTilePitch;
         dwOffset = dwDisplayStart & ~SSTG_IS_TILED;
         dwOffset -= lpDriverData->ddTileMark;
         tileInY = (dwOffset >> 12)/lpDriverData->ddTileStride;
         tileInX = (dwOffset >> 12)%lpDriverData->ddTileStride;
         dwOffset = (tileInY << 5) * dwPitch + (tileInX << 7) + lpDriverData->ddTileMark;
         }
      else
         {
         dwOffset = dwDisplayStart;
         }
      
      }

   return dwOffset;
}

/*----------------------------------------------------------------------
Function name:  GetPitch

Description:    Get Pitch
Information:

Return:         Pitch In Bytes
----------------------------------------------------------------------*/
DWORD GetPitch(GLOBALDATA * lpDriverData)
{
   DWORD dwPitch;
   DWORD dwTiled;
   DWORD vidProcConfig;

   dwPitch = 0x0;
   if (0x0 != lpDriverData)
      {
      vidProcConfig = GET(lpDriverData->lpIOregs->vidProcCfg);
      if (vidProcConfig & SST_DESKTOP_EN)
         dwTiled = vidProcConfig & SST_DESKTOP_TILED_EN;
      else
         dwTiled = vidProcConfig & SST_OVERLAY_TILED_EN;

      if (dwTiled)
         dwPitch = lpDriverData->ddTilePitch;
      else
         dwPitch = lpDriverData->pitch;
      }
      
   return dwPitch;
}

/*----------------------------------------------------------------------
Function name:  GetBPP

Description:    Get Bytes Per Pixel
Information:

Return:         Bytes Per Pixel
----------------------------------------------------------------------*/
DWORD GetBPP(GLOBALDATA * lpDriverData)
{
   DWORD dwReturn = 0x0;

   if (0x0 != lpDriverData)
      dwReturn = (DWORD)(lpDriverData->bpp + 7) >> 3;

   return dwReturn;
}

/*----------------------------------------------------------------------
Function name:  FXWAITFORIDLEWODF

Description:    Wait for Idle without DebugFix
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
FXWAITFORIDLEWODF(GLOBALDATA * lpDriverData)
{
  FxU32 dwStatus;
  volatile WORD fenceVar;

  //   P6FENCE; // Flush write combine buffers
  // P6 fence without Debug Fix

  __asm
  {
    xchg ax, fenceVar;
  }

  if (lpDriverData->doAgpCF && lpDriverData->mainFifo.unbumpedWords)
  {
    bumpAgp( lpDriverData->mainFifo.unbumpedWords );
    
    do
      {
      dwStatus = GET(lpDriverData->lpIOregs->status);
      }
    while (dwStatus & SST_PCIFIFO_BUSY);
    
    while (GET(lpDriverData->lpCRegs->PRIMARY_CMDFIFO.depth) > 0)
      ;
  }

   do
      {
      dwStatus = GET(lpDriverData->lpIOregs->status);
      }
   while (dwStatus & SST_BUSY);

}

/*----------------------------------------------------------------------
Function name:  InitFifowoDF

FIXUP's : Cursor, Back to Normal, 4-way since 2 will not have lpDriverData
      Promoting to new current state???

Description:    Initialize the cmd fifo.
                
Information:
    fifobase in physical address of framebuffer
    fifosize in bytes

Return:         VOID
----------------------------------------------------------------------*/
void InitFifowoDF(GLOBALDATA * lpDriverData, FxU32 fifoBase, FxU32 fifoSize)
{
#ifdef CMDFIFO
  FxU32   startVirtualAddr; 
  DWORD   physAddr;
#endif // #ifdef CMDFIFO
  DWORD   baseSize;

  baseSize = (lpDriverData->fifoSize + 4095) & ~4095;

  FXWAITFORIDLEWODF(lpDriverData);

  lpDriverData->doAgpCF = 0;

  lpDriverData->mainFifo.base  = (fifoBase + 4095) & ~4095;
#ifdef WIN_CSIM
  lpDriverData->mainFifo.start = lpDriverData->lfbFakeBase + lpDriverData->mainFifo.base;
#else
  lpDriverData->mainFifo.start = lpDriverData->lfbBase + lpDriverData->mainFifo.base;
#endif

  // disable command fifo 0
  //
  SETDW( lpDriverData->lpCRegs->PRIMARY_CMDFIFO.baseSize,  0 );

#ifdef CMDFIFO
  //
  // h/w bug workaround for Swap hang
  //
  if (IS_NAPALM(lpDriverData->VendorDeviceID))
      {
      SETDW(lpDriverData->lpCRegs->cmdFifoThresh,             (20 << 5) | 8 );
      }
  else
      {
      SETDW(lpDriverData->lpCRegs->cmdFifoThresh,             (15 << 5) | 8 );
      }

  SETDW( lpDriverData->lpCRegs->PRIMARY_CMDFIFO.readPtrL,  lpDriverData->mainFifo.base);
  SETDW( lpDriverData->lpCRegs->PRIMARY_CMDFIFO.readPtrH,  0 );
  SETDW( lpDriverData->lpCRegs->PRIMARY_CMDFIFO.aMin,      lpDriverData->mainFifo.base - 4 );
  SETDW( lpDriverData->lpCRegs->PRIMARY_CMDFIFO.aMax,      lpDriverData->mainFifo.base - 4 );
  SETDW( lpDriverData->lpCRegs->PRIMARY_CMDFIFO.depth,     0 );
  SETDW( lpDriverData->lpCRegs->PRIMARY_CMDFIFO.holeCount, 0 );

  SETDW( lpDriverData->lpCRegs->PRIMARY_CMDFIFO.baseAddrL, lpDriverData->mainFifo.base >> 12 );
  SETDW( lpDriverData->lpCRegs->PRIMARY_CMDFIFO.baseSize,  ((baseSize >> 12) - 1) | SST_EN_CMDFIFO );

  startVirtualAddr    = lpDriverData->mainFifo.start;
  physAddr            =  ((GET(lpDriverData->lpCRegs->PRIMARY_CMDFIFO.baseAddrL) & 0x3FF) << 12);

  lpDriverData->cmdFifoBasePtr = (FxU32)&lpDriverData->mainFifo.base;

  (DWORD)(((DWORD *)lpDriverData->cmdFifoBasePtr)[ 3 ]) = startVirtualAddr;
  (DWORD)(((DWORD *)lpDriverData->cmdFifoBasePtr)[ 1 ]) = startVirtualAddr;
  (DWORD)(((DWORD *)lpDriverData->cmdFifoBasePtr)[ 5 ]) = (fifoSize / 4) - 2;
  (DWORD)(((DWORD *)lpDriverData->cmdFifoBasePtr)[ 2 ]) = startVirtualAddr + fifoSize - 8;

  (DWORD)(((DWORD *)lpDriverData->cmdFifoBasePtr)[ 4 ]) = startVirtualAddr - physAddr;
  (DWORD)(((DWORD *)lpDriverData->cmdFifoBasePtr)[ 6 ]) = SSTCP_PKT0_JMP_LOCAL | ((physAddr >> 2) << 6);

  lpDriverData->InPacket          = 0;         
  lpDriverData->WordsLeftInPacket = 0;     
  lpDriverData->Wrapping         = 0;
  
#endif

}

/*----------------------------------------------------------------------
Function name:  InitFifoFixup

FIXUP's : Cursor, Back to Normal, 4-way since 2 will not have lpDriverData
      Promoting to new current state???

Description:    Initialize the cmd fifo.
                
Information:
    fifobase in physical address of framebuffer
    fifosize in bytes

Return:         VOID
----------------------------------------------------------------------*/
void InitFifoFixup(GLOBALDATA * lpDriverData)
{
   if (0x0 != lpDriverData)
         InitFifowoDF(lpDriverData, lpDriverData->mainFifo.base, lpDriverData->fifoSize);
}

/*----------------------------------------------------------------------
Function name:  InitRegswoDF

Description:    Initialize the chip's registers.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void InitRegswoDF(GLOBALDATA * lpMasterDriverData, GLOBALDATA * lpSlaveDriverData)
{
    DWORD lfbMemoryConfig;
    DWORD screenSize;
    DWORD vidProcCfg;
    DWORD aperture;
    

   if ((0x0 != lpSlaveDriverData) && (0x0 != lpMasterDriverData))
      {
      lpSlaveDriverData->lpCRegs = (SstCRegs *)(lpSlaveDriverData->regBase[HWINFO_SST_CMDFIFOREGS_INDEX]);
      lpSlaveDriverData->lpGRegs = (SstGRegs *)(lpSlaveDriverData->regBase[HWINFO_SST_2DREGS_INDEX]);
      lpSlaveDriverData->lpIOregs = (SstIORegs *)(lpSlaveDriverData->regBase[HWINFO_SST_IOREGS_INDEX]);

      lpSlaveDriverData->fifoSize = lpMasterDriverData->fifoSize;

      // If slave fifo is zero then make it a reasonable amount
      if (0x0 == lpSlaveDriverData->fifoSize)
         lpSlaveDriverData->fifoSize = 0x100000;

      lpSlaveDriverData->VendorDeviceID = lpMasterDriverData->VendorDeviceID;

      InitFifowoDF(lpSlaveDriverData, lpMasterDriverData->mainFifo.base, lpMasterDriverData->fifoSize);

      lpSlaveDriverData->cmdFifoBasePtr = (FxU32)&lpSlaveDriverData->mainFifo.base;

      FXWAITFORIDLEWODF(lpDriverData);
    
      screenSize = (lpMasterDriverData->bi.biHeight << 16) | lpMasterDriverData->bi.biWidth;

      if (lpMasterDriverData->ddPrimaryInTile)
         {
         lpSlaveDriverData->screenFormat = lpMasterDriverData->ddTileStride;
         }
      else
         {
         lpSlaveDriverData->screenFormat = lpMasterDriverData->pitch;
         }

      switch( lpMasterDriverData->bpp)
         {
         case 8:  
            lpSlaveDriverData->screenFormat |= SSTG_PIXFMT_8BPP;
            break;

         case 15: 
            lpSlaveDriverData->screenFormat |= SSTG_PIXFMT_15BPP; 
            break;

         case 16: 
            lpSlaveDriverData->screenFormat |= SSTG_PIXFMT_16BPP; 
            break;

         case 24: 
            lpSlaveDriverData->screenFormat |= SSTG_PIXFMT_24BPP; 
            break;

         case 32: 
            lpSlaveDriverData->screenFormat |= SSTG_PIXFMT_32BPP; 
            break;

         }

      // Clear primary surface (desktop).

      SETDW(lpSlaveDriverData->lpGRegs->commandEx,   0);
      SETDW(lpSlaveDriverData->lpGRegs->colorFore,   0);
      SETDW(lpSlaveDriverData->lpGRegs->clip0min,    0);
      SETDW(lpSlaveDriverData->lpGRegs->clip0max,    screenSize);
      SETDW(lpSlaveDriverData->lpGRegs->dstSize,     screenSize);
      SETDW(lpSlaveDriverData->lpGRegs->dstXY,       0);
      SETDW(lpSlaveDriverData->lpGRegs->dstFormat,   lpMasterDriverData->screenFormat);
      SETDW(lpSlaveDriverData->lpGRegs->dstBaseAddr, lpMasterDriverData->gdiDesktopStart);
      SETDW(lpSlaveDriverData->lpGRegs->srcBaseAddr, lpMasterDriverData->gdiDesktopStart);
      SETDW(lpSlaveDriverData->lpGRegs->command, 0xCC000000 | SSTG_GO | SSTG_RECTFILL);
      SETDW(lpSlaveDriverData->lpIOregs->vidPixelBufThold, _FF(vidPixelBufThold));

      switch(lpMasterDriverData->ddTilePitch)
         {
         case 1024: 
            aperture=0;
            break;

         case 2048: 
            aperture=1;
            break;

         case 4096: 
            aperture=2;
            break;
   
         case 8192: 
            aperture=3;
            break;

         default: 
            aperture=4;
         }       
      // initialize memory config for tile surface
      lfbMemoryConfig = 
       ((lpDriverData->ddTileMark >> 12) & 0x1fff) |        // tile aperture base bits(12:0)
      (((lpDriverData->ddTileMark >> 25) & 0x0003) << 23) | // tile aperture base bits(14:13)
      (aperture << 13L) |   // aperture at 13th bit
      (lpDriverData->ddTileStride << 16L); // at 16th bit
      SETDW(lpSlaveDriverData->lpIOregs->lfbMemoryConfig, lfbMemoryConfig);

      // now enable the video processor
      //
#ifdef WIN_CSIM
      vidProcCfg = GET(((SstIORegs *)lpSlaveDriverData->regRealBase)->vidProcCfg);
      SETDW(((SstIORegs *)lpSlaveDriverData->regRealBase)->vidProcCfg, vidProcCfg | SST_VIDEO_PROCESSOR_EN);
#else
      vidProcCfg = GET(lpSlaveDriverData->lpIOregs->vidProcCfg);
      SETDW(lpSlaveDriverData->lpIOregs->vidProcCfg, vidProcCfg | SST_VIDEO_PROCESSOR_EN);
#endif
      }
}

/*----------------------------------------------------------------------
Function name:  SaveMemConfig

Description:    Save Mode Information when we enter SLI/AA mode.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void SaveMemConfig(GLOBALDATA * lpDriverData, PCONF_SAVE pConf_Save, DWORD * regBase, DWORD lfbBase, DWORD SliaaLfbBase)
{
   int i;

   DriverData.lpPDevice32 = 0x0;
   if (0x0 != lpDriverData)
      {
      pConf_Save->fifoStart = lpDriverData->fifoStart;
      pConf_Save->fifoSize = lpDriverData->fifoSize;

#ifdef USE_FAKE_PHYS
      if (0x0 != lpDriverData->lpPDevice32)
         {
         pConf_Save->deBitsOffset = ((DIBENGINE *)lpDriverData->lpPDevice32)->deBitsOffset;
         ((DIBENGINE *)lpDriverData->lpPDevice32)->deBitsOffset = SliaaLfbBase;
         }
#else
      (void *)SliaaLfbBase;
#endif

      //This is needed for the Fake DriverData <the real one already has this>
      for (i=0; i<HWINFO_SST_MAX_CHIP_INDEX; i++)
         lpDriverData->regBase[i] = regBase[i];
      lpDriverData->lfbBase = lfbBase;

      //More Fixup of the Fake One
      DriverData.doAgpCF = 0;
      DriverData.mainFifo.unbumpedWords = 0x0;
      }
}

/*----------------------------------------------------------------------
Function name:  RestoreMemConfig

Description:    Restore Mode Information when we enter SLI/AA mode.
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void RestoreMemConfig(GLOBALDATA * lpDriverData, PCONF_SAVE pConf_Save, DWORD * regBase, DWORD lfbBase)
{
   int i;

   if (0x0 != lpDriverData)
      {
      lpDriverData->fifoStart = pConf_Save->fifoStart;
      lpDriverData->fifoSize = pConf_Save->fifoSize;

#ifdef USE_FAKE_PHYS
      if (0x0 != lpDriverData->lpPDevice32)
         {
         ((DIBENGINE *)lpDriverData->lpPDevice32)->deBitsOffset = pConf_Save->deBitsOffset;
         }
#endif
      //This is needed for the Fake DriverData <the real one already has this>
      for (i=0; i<HWINFO_SST_MAX_CHIP_INDEX; i++)
         lpDriverData->regBase[i] = regBase[i];
      lpDriverData->lfbBase = lfbBase;

      }
}

/*----------------------------------------------------------------------
Function name:  GetStrideAndAperature

Description:    Get the Stride and the Aperature
                
Information:

Return:         VOID
----------------------------------------------------------------------*/
void GetStrideAndAperature(GLOBALDATA * lpDriverData, DWORD * pStrideandAperature)
{
   DWORD aperture;

   if (0x0 != lpDriverData)
      {
      switch(lpDriverData->ddTilePitch)
         {
         case 1024: 
            aperture=0;
            break;

         case 2048: 
            aperture=1;
            break;

         case 4096: 
            aperture=2;
            break;

         case 8192: 
            aperture=3;
            break;

         default: 
            aperture=4;
         }

      *pStrideandAperature = (aperture << 13L) | (lpDriverData->ddTileStride << 16L);
   }

}

DWORD GetNumChips(GLOBALDATA *lpDriverData)
{
  return lpDriverData->dwNumUnits;
}

#endif

