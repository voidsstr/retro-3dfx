/*
 ** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
 ** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
 ** rights reserved under the Copyright Laws of the United States.
 **
 ** $Header: fifo.c, 6, 10/11/00 8:21:18 PM, Brent$
 ** $Log: 
 **  6    3dfx      1.3.1.1     10/11/00 Brent           Forced check in to enforce
 **       branching.
 **  5    3dfx      1.3.1.0     07/21/00 Adam Briggs     don't try to bump > 0xffff
 **       at a time
 **  4    3dfx      1.3         03/23/00 Chris Dow       Added code to limit
 **       unfenced writes to 64K for coppermine hang problems.
 **  3    3dfx      1.2         02/08/00 Kenneth Dyke    Added slave FIFO handling
 **       code.
 **  2    3dfx      1.1         01/28/00 Adam Briggs     glide3->glide2 merge from
 **       napalm bringup
 **  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
 ** $
** 
** 42    6/21/99 12:57p Kcd
** Removed debug code.  Oops.
** 
** 41    6/18/99 1:15p Kcd
** PowerPC PCI Bump & Grind stuff.
** 
** 40    3/19/99 1:24p Peter
** change for calling convention change from asm fifo wrapping code
** 
** 39    3/14/99 1:42p Peter
** really invokve the ggggsoph
** 
** 38    3/13/99 9:48p Dow
** optimizations for B&G
** 
** 37    12/05/98 1:06p Dow
** Made some eschatology automatic
** 
** 36    10/21/98 10:41a Atai
** mod for HAL_CSIM
** 
** 35    7/24/98 2:03p Dow
** AGP Stuff
** 
** 34    7/23/98 3:19p Dow
** Bump & Grind Fix
** 
** 33    7/23/98 1:17a Dow
** Bump & Grind
** 
** 32    7/06/98 6:49p Jdt
** Protected in-memory command fifo code
** 
** 31    6/15/98 5:55p Hanson
** Added ifdef for hpflip interface code
** 
** 30    6/15/98 4:27p Hanson
** Fixed bug in my interface
** 
** 29    6/15/98 3:52p Hanson
** Added interface for hpflip.  Functions are not exported.
** Function added:
** _grGetCommandTransportInfo
** _grSetCommandTransportInfo
** 
** No support for these functions is needed.  It just makes my life a lot
** easier, to synch up with the latest glide.
** 
** 28    5/29/98 4:30p Peter
** Chris's swap pending thing
** 
** 27    4/22/98 11:48a Dow
** Fixed level
** 
** 26    4/22/98 8:41a Dow
** AGP Workaround
** 
** 25    4/05/98 2:32p Dow
** Fixed non-debug compiler error
** 
** 24    4/03/98 2:04p Dow
** Dos Glide Mods
** 
** 23    3/28/98 11:24a Dow
** itwo�
** 
** 21    2/17/98 12:50p Dow
** Added conditional fifo id.
** 
** 20    2/08/98 3:08p Dow
** FIFO Works
 * 
 * 18    12/17/97 4:45p Peter
 * groundwork for CrybabyGlide
 * 
 * 17    12/09/97 12:20p Peter
 * mac glide port
 * 
 * 16    12/09/97 10:28a Peter
 * cleaned up some frofanity
 * 
 * 15    12/05/97 4:26p Peter
 * watcom warnings
 * 
 * 14    12/03/97 11:34a Peter
 * dos debugging
 * 
 * 13    11/21/97 3:53p Peter
 * reset messages are controlled by gdbg_level
 * 
 * 12    11/19/97 6:04p Peter
 * actually exit if not reset
 * 
 * 11    11/18/97 4:36p Peter
 * chipfield stuff cleanup and w/ direct writes
 * 
 * 10    11/17/97 4:55p Peter
 * watcom warnings/chipfield stuff
 * 
 * 9     11/15/97 9:20p Peter
 * I am the sorriest f*cker on the face of the planet
 * 
 **
 */

#include <stdio.h>
#include <string.h>

#if defined(__WIN32__)
#include <windows.h>
#endif

#include <3dfx.h>
#include <glidesys.h>

#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include "fxglide.h"


#if GDBG_INFO_ON

static const char* h3SstRegNames[] = {
  "status",
  "intrCtrl",
  "vAx",
  "vAy",
  "vBx",
  "vBy",
  "vCx",
  "vCx",
  "r",
  "g",
  "b",
  "z",
  "a",
  "s",
  "t",
  "w",
  "drdx",
  "dgdx",
  "dbdx",
  "dzdx",
  "dadx",
  "dsdx",
  "dtdx",
  "dwdx",
  "drdy",
  "dgdy",
  "dbdy",
  "dzdy",
  "dady",
  "dsdy",
  "dtdy",
  "dwdy",
  "triangleCMD",
  "reservedA",
  "FvAx",
  "FvAy",
  "FvBx",
  "FvBy",
  "FvCx",
  "FvCy",
  "Fr",
  "Fg",
  "Fb",
  "Fz",
  "Fa",
  "Fs",
  "Ft",
  "Fw",
  "Fdrdx",
  "Fdgdx",
  "Fdbdx",
  "Fdzdx",
  "Fdadx",
  "Fdsdx",
  "Fdtdx",
  "Fdwdx",
  "Fdrdy",
  "Fdgdy",
  "Fdbdy",
  "Fdzdy",
  "Fdady",
  "Fdsdy",
  "Fdtdy",
  "Fdwdy",
  "FtriangleCMD",
  "fbzColorPath",
  "fogMode",
  "alphaMode",
  "fbzMode",
  "lfbMode",
  "clipLeftRight",
  "clipBottomTop",
  "nopCMD",
  "fastfillCMD",
  "swapbufferCMD",
  "fogColor",
  "zaColor",
  "chromaKey",
  "chromaRange",
  "userIntrCmd",
  "stipple",
  "c0",
  "c1",
  "fbiPixelsIn",
  "fbiChromaFail",
  "fbiZfuncFail",
  "fbiAfuncFail",
  "fbiPixelsOut",
  "fogTable00",
  "fogTable01",
  "fogTable02",
  "fogTable03",
  "fogTable04",
  "fogTable05",
  "fogTable06",
  "fogTable07",
  "fogTable08",
  "fogTable09",
  "fogTable0a",
  "fogTable0b",
  "fogTable0c",
  "fogTable0d",
  "fogTable0e",
  "fogTable0f",
  "fogTable10",
  "fogTable11",
  "fogTable12",
  "fogTable13",
  "fogTable14",
  "fogTable15",
  "fogTable16",
  "fogTable17",
  "fogTable18",
  "fogTable19",
  "fogTable1a",
  "fogTable1b",
  "fogTable1c",
  "fogTable1d",
  "fogTable1e",
  "fogTable1f",
  "reservedB0",
  "reservedB1",
  "reservedB2",
  "colBufferAddr",
  "colBufferStride",
  "auxBufferAddr",
  "auxBufferStride",
  "reservedC",
  "clipLeftRight1",
  "clipBottomTop1",
  "reservedD0",
  "reservedD1",
  "reservedD2",
  "reservedD3",
  "reservedD4",
  "reservedD5",
  "reservedE0",
  "reservedE1",
  "reservedE2",
  "reservedE3",
  "reservedE4",
  "reservedE5",
  "reservedE6",
  "reservedE7",
  "reservedF0",
  "reservedF1",
  "reservedF2",
  "swapBufferPend",
  "leftOverlayBuf",
  "rightOverlayBuf",
  "fbiSwapHistory",
  "fbiTrianglesOut",
  "sSetupMode",
  "sVx",
  "sVy",
  "sARGB",
  "sRed",
  "sGreen",
  "sBlue",
  "sAlpha",
  "sVz",
  "sOowfbi",
  "sOow0",
  "sSow0",
  "sTow0",
  "sOow1",
  "sSow1",
  "sTow1",
  "sDrawTriCMD",
  "sBeginTriCMD",
  "reservedG0",
  "reservedG1",
  "reservedG2",
  "reservedG3",
  "reservedG4",
  "reservedG5",
  "reservedH0",
  "reservedH1",
  "reservedH2",
  "reservedH3",
  "reservedH4",
  "reservedH5",
  "reservedH6",
  "reservedH7",
  "reservedI0",
  "reservedI1",
  "reservedI2",
  "reservedI3",
  "reservedI4",
  "reservedI5",
  "reservedI6",
  "reservedI7",
  "textureMode",
  "tLOD",
  "tDetail",
  "texBaseAddr",
  "texBaseAddr1",
  "texBaseAddr2",
  "texBaseAddr38",
  "trexInit0",
  "trexInit1",
  "nccTable0-0",
  "nccTable0-1",
  "nccTable0-2",
  "nccTable0-3",
  "nccTable0-4",
  "nccTable0-5",
  "nccTable0-6",
  "nccTable0-7",
  "nccTable0-8",
  "nccTable0-9",
  "nccTable0-a",
  "nccTable0-b",
  "nccTable1-0",
  "nccTable1-1",
  "nccTable1-2",
  "nccTable1-3",
  "nccTable1-4",
  "nccTable1-5",
  "nccTable1-6",
  "nccTable1-7",
  "nccTable1-8",
  "nccTable1-9",
  "nccTable1-a",
  "nccTable1-b",
  "tChromaKeyMin",
  "tChromaKeyMax",
};

static const char * h3SstIORegNames[] = {     
  "status",
  "pciInit0",
  "sipMonitor",
  "lfbMemoryConfig",
  "miscInit0",
  "miscInit1",
  "dramInit0",
  "dramInit1",
  "agpInit",
  "tmuGbeInit",
  "vgaInit0",
  "vgaInit1",
  "dramCommand",
  "dramData",
  "reservedZ0"
  "reservedZ0"
  "pllCtrl0",
  "pllCtrl1",
  "pllCtrl2",
  "dacMode",
  "dacAddr",
  "dacData",
  "vidMaxRGBDelta",
  "vidProcCfg",
  "hwCurPatAddr",
  "hwCurLoc",
  "hwCurC0",
  "hwCurC1",
  "vidInFormat",
  "vidInStatus",
  "vidSerialParallelPort",
  "vidInXDecimDeltas",
  "vidInDecimInitErrs",
  "vidInYDecimDeltas",
  "vidPixelBufThold",
  "vidChromaMin",
  "vidChromaMax",
  "vidCurrentLine",
  "vidScreenSize",
  "vidOverlayStartCoords",
  "vidOverlayEndScreenCoord",
  "vidOverlayDudx",
  "vidOverlayDudxOffsetSrcWidth",
  "vidOverlayDvdy",
  "vgaRegister[0]",
  "vgaRegister[1]",
  "vgaRegister[2]",
  "vgaRegister[3]",
  "vgaRegister[4]",
  "vgaRegister[5]",
  "vgaRegister[6]",
  "vgaRegister[7]",
  "vgaRegister[8]",
  "vgaRegister[9]",
  "vgaRegister[a]",
  "vgaRegister[b]",
  "vidOverlayDvdyOffset",
  "vidDesktopStartAddr",
  "vidDesktopOverlayStride",
  "vidInAddr0",
  "vidInAddr1",
  "vidInAddr2",
  "vidInStride",
  "vidCurrOverlayStartAddr",
} ;


#define GEN_INDEX(a) ((((FxU32) a) - ((FxU32) gc->reg_ptr)) >> 2)

void
_grFifoWriteDebug(FxU32 addr, FxU32 val, FxU32 fifoPtr)
{
  GR_DCL_GC;
  FxU32 index = GEN_INDEX(addr);
  
  GDBG_INFO(gc->myLevel + 199, "Storing to FIFO:\n");
  GDBG_INFO(gc->myLevel + 199, "  FIFO Ptr:    0x%x : 0x%X\n", fifoPtr, gc->cmdTransportInfo.fifoRoom);  
  if (index <= 0xff) { 
    GDBG_INFO(gc->myLevel + 199, "  Reg Name:    %s\n", h3SstRegNames[index]);
    GDBG_INFO(gc->myLevel + 199, "  Reg Num:     0x%X\n", index);
  } else {
    const char* strP;
    const FxU32 offset = (addr - (FxU32)gc->reg_ptr);
    
    if (offset >= HW_TEXTURE_OFFSET) {
      strP = "Texture";
    } else if (offset >= HW_LFB_OFFSET) {
      strP = "LFB";
      index = addr;
    } else if (offset >= HW_FIFO_OFFSET) {
      strP = "Cmd FIFO";
    } else {
      strP = "Woah!";
    }
    GDBG_INFO(gc->myLevel + 199, "  %s Addr:    0x%X\n",
      strP, index);
  }
  GDBG_INFO(gc->myLevel + 199, "  Value:       0x%X 0x%X\n", (index << 2), val);
  
#if !HAL_CSIM
  GDBG_INFO(120, "        SET(0x%X, %ld(0x%X)) 0 %s (0x%X)\n",
    0x10000000UL + (FxU32)(index << 2), val, val, 
    h3SstRegNames[index & 0xFF], fifoPtr);
#endif /* !HAL_CSIM */
} /* _grFifoWriteDebug */

void
_grFifoFWriteDebug(FxU32 addr, float val, FxU32 fifoPtr)
{
  GR_DCL_GC;
  FxU32 index = GEN_INDEX(addr);

  GDBG_INFO(gc->myLevel + 200, "Storing to FIFO:\n");
  GDBG_INFO(gc->myLevel + 200, "  FIFO Ptr:    0x%x\n", fifoPtr);
  if (index <= 0xff) {
    GDBG_INFO(gc->myLevel + 200, "  Reg Name:    %s\n", h3SstRegNames[index]);
    GDBG_INFO(gc->myLevel + 200, "  Reg Num:     0x%x\n", index);
  }
  GDBG_INFO(gc->myLevel + 200, "  Value:       %4.2f\n", val);

#if !HAL_CSIM
  GDBG_INFO(120, "        SET(0x%X, %4.2f (0x%X)) 0 %s\n", 
    0x10000000UL + (FxU32)(index << 2), val, *(const FxU32*)&val, 
    h3SstRegNames[index & 0xFF]);
#endif /* !HAL_CSIM */
} /* _grFifoFWriteDebug */

extern void
_grH3FifoDump_TriHdr(const FxU32 hdrVal)
{
  GR_DCL_GC;

  /* Dump Packet Header */
  GDBG_INFO(gc->myLevel + 200, "CMD Fifo Triangle Packet (0x%X)\n", hdrVal);
  GDBG_INFO(gc->myLevel + 200, "  # Vertex: 0x%X\n", 
    (hdrVal & SSTCP_PKT3_NUMVERTEX) >> SSTCP_PKT3_NUMVERTEX_SHIFT);
  GDBG_INFO(gc->myLevel + 200, "  RGB: %s\n",
    (hdrVal & SSTCP_PKT3_PACKEDCOLOR) ? "Packed" : "Separate");

  GDBG_INFO(gc->myLevel + 200, "  StripMode: %s\n",
    (((hdrVal & (0x01 << 22)) == 0) ? "Strip" : "Fan"));

  GDBG_INFO(gc->myLevel + 200, "  Culling: %s\n",
    (((hdrVal & (0x01 << 23)) == 0) ? "Disable" : "Enable"));

  GDBG_INFO(gc->myLevel + 200, "  CullingSign: %s\n",
    (((hdrVal & (0x01 << 24)) == 0) ? "Positive" : "Negative"));

  GDBG_INFO(gc->myLevel + 200, "  PingPongSign: %s\n",
    (((hdrVal & (0x01 << 25)) == 0) ? "Normal" : "Disable"));
  
  if (GDBG_GET_DEBUGLEVEL(gc->myLevel + 200)) {
    const FxU32 temp = (hdrVal & SSTCP_PKT3_PMASK);
    int i;

    GDBG_INFO(gc->myLevel + 200, "  Params: X Y");

    for(i = 10; i <= 17; i++) {
      static const char* paramSel[] = { "RGB", "Alpha", "Z", "Wb", "W0", "ST[0]", "W1", "ST[1]" };
      
      if ((temp & (0x01UL << i)) != 0) GDBG_PRINTF("%s ", paramSel[i - 10]);
    }
    GDBG_INFO(gc->myLevel + 200, "\n");
  }

  {
    const FxU32 temp = (hdrVal & SSTCP_PKT3_CMD) >> SSTCP_PKT3_CMD_SHIFT;
    const char* tempStr;

    switch(temp) {
    case 0x00: tempStr = "Independent";   break;
    case 0x01: tempStr = "NewStrip";      break;
    case 0x02: tempStr = "ContinueStrip"; break;
    default:   tempStr = "Reserved";      break;
    }
    GDBG_INFO(gc->myLevel + 200, "  Command: 0x%X(%s)\n", temp, tempStr);
  }
}

void
_grErrorCallback(const char* const procName,
                 const char* const format,
                 va_list           args)
{
  static FxBool inProcP = FXFALSE;

  if (!inProcP) {
    static char errMsgBuf[1024];
    
    inProcP = FXTRUE;
    {
      extern void (*GrErrorCallback)( const char *string, FxBool fatal );

      vsprintf(errMsgBuf, format, args);
      (*GrErrorCallback)(errMsgBuf, (GETENV("FX_ERROR_FAIL") != NULL));
    }
    inProcP = FXFALSE;
  }
}

#endif /* GDBG_INFO_ON */

#if USE_PACKET_FIFO

/* Routines privately exported so that the manufacturing diags
 * and other things can do register writes etc w/o having access
 * to the glide internals etc.
 */
extern void
_grSet32(volatile FxU32* const sstAddr, const FxU32 val)
{
#define FN_NAME "_grSet32"
  GR_DCL_GC;

  GR_ASSERT(sstAddr >= gc->base_ptr);
  GR_ASSERT(sstAddr <  &SST_TMU(gc->reg_ptr, GR_TMU0)->status);

  GR_SET_EXPECTED_SIZE(sizeof(FxU32), 1);
  GR_SET_INDEX(BROADCAST_ID, gc->reg_ptr, (sstAddr - gc->reg_ptr), val);
  GR_CHECK_SIZE();
#undef FN_NAME
}

extern FxU32
_grGet32(volatile FxU32* const sstAddr)
{
#define FN_NAME "_grGet32"
  GR_BEGIN_NOFIFOCHECK(FN_NAME, 88);
  GDBG_INFO_MORE(gc->myLevel, "(0x%X)\n", sstAddr);
  GR_RETURN(GR_GET(*sstAddr)); 
#undef FN_NAME
} /* _grGet32 */

#if FIFO_ASSERT_FULL
const FxU32 kFifoCheckMask = 0xFFFF;
FxU32 gFifoCheckCount = 0;
#endif

void
_grBumpNGrind() 
{
#define FN_NAME "_grCheckForBump"
  GR_BEGIN_NOFIFOCHECK(FN_NAME, 400);

#ifdef HAL_CSIM
  /* WTF? If the csim cannot do agp so we should just be doing the
   * same thing that the non-csim case is doing.  
   */
  GR_CAGP_SET(bump, 0);
#else /* !HAL_CSIM */
  FIFO_CACHE_FLUSH(gc->cmdTransportInfo.fifoPtr);
  P6FENCE;


  /* AH FUK: the bump register can only handle a 16bit value */
  while ((gc->cmdTransportInfo.fifoPtr - gc->cmdTransportInfo.lastBump) > 0xFFFFL)
  {
    GR_CAGP_SET(bump, 0xFFFFL);
    gc->cmdTransportInfo.lastBump += 0xFFFFL ;
  }

  if (gc->cmdTransportInfo.fifoPtr - gc->cmdTransportInfo.lastBump)
    GR_CAGP_SET(bump, gc->cmdTransportInfo.fifoPtr - gc->cmdTransportInfo.lastBump);

  gc->cmdTransportInfo.lastBump = gc->cmdTransportInfo.fifoPtr;
  gc->cmdTransportInfo.bumpPos = gc->cmdTransportInfo.fifoPtr + (gc->cmdTransportInfo.bumpSize);
  if (gc->cmdTransportInfo.bumpPos > gc->cmdTransportInfo.fifoEnd)
    gc->cmdTransportInfo.bumpPos = gc->cmdTransportInfo.fifoEnd;
#endif /* !HAL_CSIM */

  GR_ASSERT(gc->cmdTransportInfo.bumpPos != 0);
  GR_ASSERT(gc->cmdTransportInfo.lastBump != 0);
#undef FN_NAME
} /* _grCheckForBump */

FxU32 _grHwFifoPtrSlave(FxU32 slave, FxBool ignored);

void FX_CALL
_grCommandTransportMakeRoom(const FxI32 blockSize, const char* fName, const int fLine)
{
#define FN_NAME "_grCommandTransportMakeRoom"
  FxU32 wrapAddr = 0x00UL;
  FxU32 checks;

  GR_BEGIN_NOFIFOCHECK(FN_NAME, 400);

  if (!gc->cmdTransportInfo.autoBump)
    GR_BUMP_N_GRIND;


  GR_ASSERT(blockSize > 0);
  GR_ASSERT((FxU32)blockSize < gc->cmdTransportInfo.fifoSize);
  FIFO_ASSERT();

  /* Update the roomToXXX values w/ the # of writes since the last
   * fifo stall/wrap.  
   */
  {
    const FxI32 writes = (MIN(gc->cmdTransportInfo.roomToReadPtr, gc->cmdTransportInfo.roomToEnd) -
                          gc->cmdTransportInfo.fifoRoom);
    
    gc->cmdTransportInfo.roomToReadPtr   -= writes;
    gc->cmdTransportInfo.roomToEnd       -= writes;

#if GDBG_INFO_ON
    GDBG_INFO_MORE(gc->myLevel, ": (%s : %d)\n"
      "\tfifoBlock: (0x%X : 0x%X)\n"
      "\tfifoRoom: (0x%X : 0x%X) : (0x%X : 0x%X)\n"
      "\tfifo hw: (0x%X : 0x%X)\n",
      ((fName == NULL) ? "Unknown" : fName), fLine,
      (FxU32)gc->cmdTransportInfo.fifoPtr, blockSize,
      gc->cmdTransportInfo.roomToReadPtr, gc->cmdTransportInfo.roomToEnd, 
      gc->cmdTransportInfo.fifoRoom, writes,
      HW_FIFO_PTR(FXTRUE), gc->cmdTransportInfo.fifoRead);
    
#endif /* GDBG_INFO_ON */
    
    ASSERT_FAULT_IMMED((gc->cmdTransportInfo.roomToReadPtr >= 0) && 
      (gc->cmdTransportInfo.roomToEnd >= 0));
  }

  checks = 0;
  again:
  /* do we need to stall? */
  {
    FxU32 lastHwRead = gc->cmdTransportInfo.fifoRead;
    FxI32 roomToReadPtr = gc->cmdTransportInfo.roomToReadPtr;
    FxU32 stuckPolls = 0;

    while (roomToReadPtr < blockSize) {
      FxU32 curReadPtr = HW_FIFO_PTR(FXTRUE);
      FxU32 curReadDist = curReadPtr - lastHwRead;

      /* Handle slave chips.  This code lifted from cvg and modified
       * to deal with multiple slave chips. */
      if(gc->chipCount > 1) {
        FxU32 slave;
        for(slave = 1; slave < gc->chipCount; slave++) {              
          const FxU32 slaveReadPtr = _grHwFifoPtrSlave(slave, 0);
          const FxU32 slaveReadDist = (slaveReadPtr - lastHwRead);
          FxI32 distSlave = (FxI32)slaveReadDist;
          FxI32 distMaster = (FxI32)curReadDist;

          GR_ASSERT((slaveReadPtr >= (FxU32)gc->cmdTransportInfo.fifoStart) &&
                    (slaveReadPtr < (FxU32)gc->cmdTransportInfo.fifoEnd));
      
          /* Get the actual absolute distance to the respective fifo ptrs */
          if (distSlave < 0) distSlave += (FxI32)gc->cmdTransportInfo.fifoSize - FIFO_END_ADJUST;
          if (distMaster < 0) distMaster += (FxI32)gc->cmdTransportInfo.fifoSize - FIFO_END_ADJUST;

          /* Is this slave closer than the master? */
          if (distSlave < distMaster) {
            
            curReadDist = slaveReadDist;
            curReadPtr = slaveReadPtr;
          }
        }
      }

      /* retro3dfx: bounded fifo stall. If the accelerator wedges, the
       * hw read pointer stops advancing and this loop spins forever
       * (the checks>1000 diagnostic below is GDBG-only and just logs).
       * ~4M no-progress polls is seconds of wall time on real hw --
       * treat that as a wedge, pretend the fifo drained, and let the
       * caller proceed (matches the display driver's H3MakeRoom
       * WEDGE-BREAK behavior). */
      if (curReadDist == 0) {
        if (++stuckPolls > 4000000UL) {
          roomToReadPtr = blockSize; /* WEDGE-BREAK: force room */
          break;
        }
      } else
        stuckPolls = 0;

      checks++;

#ifdef GDBG_INFO_ON
      if (checks > 1000) {
        FxU32
          baseAddrL,
          baseSize,
          readPtrL,
          aMin,
          aMax,
          depth,
          holeCount;

        baseAddrL = GR_CAGP_GET(baseAddrL);
        baseSize = GR_CAGP_GET(baseSize);
        readPtrL = GR_CAGP_GET(readPtrL);
        aMin = GR_CAGP_GET(aMin);
        aMax = GR_CAGP_GET(aMax);
        depth = GR_CAGP_GET(depth);
        holeCount = GR_CAGP_GET(holeCount);

        GDBG_INFO(80,"Fifo check timeout:\n");
        GDBG_INFO(80,"\tbaseAddrL = 0x%x\n", baseAddrL);
        GDBG_INFO(80,"\tbaseSize  = 0x%x\n", baseSize);
        GDBG_INFO(80,"\treadPtrL  = 0x%x\n", readPtrL);
        GDBG_INFO(80,"\tdepth     = 0x%x\n", depth);
        GDBG_INFO(80,"\tholeCount = 0x%x\n", holeCount);
        GDBG_INFO(80,"\taMin      = 0x%x\n", aMin);
        GDBG_INFO(80,"\taMax      = 0x%x\n", aMax);

        if (
            (readPtrL < (baseAddrL << 12)) ||
            (readPtrL > ((baseAddrL + baseSize + 1) << 12))
            ) {
          GDBG_PRINTF("FATAL ERROR:  Read Pointer out of command buffer extents\n");
          exit(-1);
        }
        checks = 0;
      }
#endif

      GR_ASSERT((curReadPtr >= (FxU32)gc->cmdTransportInfo.fifoStart) &&
        (curReadPtr < (FxU32)gc->cmdTransportInfo.fifoEnd));

      roomToReadPtr += curReadDist;

      _GlideRoot.stats.fifoStalls++;
      _GlideRoot.stats.fifoStallDepth += GR_CAGP_GET(depth);

      /* Have we wrapped yet? */
      if (lastHwRead > curReadPtr) roomToReadPtr += (FxI32)gc->cmdTransportInfo.fifoSize - FIFO_END_ADJUST;
      lastHwRead = curReadPtr;
    }

    GR_ASSERT((lastHwRead >= (FxU32)gc->cmdTransportInfo.fifoStart) &&
              (lastHwRead < (FxU32)gc->cmdTransportInfo.fifoEnd));

    /* Update cached copies */
    gc->cmdTransportInfo.fifoRead = lastHwRead;
    gc->cmdTransportInfo.roomToReadPtr = roomToReadPtr;

    GDBG_INFO(gc->myLevel, "  Wait: (0x%X : 0x%X) : 0x%X\n", 
              gc->cmdTransportInfo.roomToReadPtr, gc->cmdTransportInfo.roomToEnd,
              gc->cmdTransportInfo.fifoRead);
  }
  
  /* Do we need to wrap to front? */
  if (gc->cmdTransportInfo.roomToEnd <= blockSize) {
    GDBG_INFO(gc->myLevel + 10, "  Pre-Wrap: (0x%X : 0x%X) : 0x%X\n", 
              gc->cmdTransportInfo.roomToReadPtr, gc->cmdTransportInfo.roomToEnd,
              gc->cmdTransportInfo.fifoRead);

    
    /* Set the jsr packet. 
     * NB: This command must be fenced.
     */
    FIFO_ASSERT();
    {
      P6FENCE;
      if (!gc->cmdTransportInfo.autoBump) {
#if __POWERPC__ && PCI_BUMP_N_GRIND
        SET_FIFO(*gc->cmdTransportInfo.fifoPtr++, gc->cmdTransportInfo.fifoJmpHdr[0]);
        FIFO_CACHE_FLUSH(gc->cmdTransportInfo.fifoPtr);
        P6FENCE;
        GR_CAGP_SET(bump, 1);
#else /* !(__POWERPC__ && PCI_BUMP_N_GRIND) */
        SET_FIFO(*gc->cmdTransportInfo.fifoPtr++, gc->cmdTransportInfo.fifoJmpHdr[0]);
        SET_FIFO(*gc->cmdTransportInfo.fifoPtr++, gc->cmdTransportInfo.fifoJmpHdr[1]);
#ifdef HAL_CSIM
        GR_CAGP_SET(bump, 0);
#else
        GR_CAGP_SET(bump, 2);
#endif
#endif /* !(__POWERPC__ && PCI_BUMP_N_GRIND) */
        gc->cmdTransportInfo.lastBump = gc->cmdTransportInfo.fifoStart;
      } else
        SET_FIFO(*gc->cmdTransportInfo.fifoPtr, gc->cmdTransportInfo.fifoJmpHdr[0]);
    }

    P6FENCE;
    
    wrapAddr = (FxU32)gc->cmdTransportInfo.fifoPtr;

    /* Update roomXXX fields for the actual wrap */
    gc->cmdTransportInfo.roomToReadPtr -= gc->cmdTransportInfo.roomToEnd;
    gc->cmdTransportInfo.roomToEnd = gc->cmdTransportInfo.fifoSize - FIFO_END_ADJUST;

#if GLIDE_USE_DEBUG_FIFO
    _GlideRoot.stats.fifoWraps++;
    _GlideRoot.stats.fifoWrapDepth += GR_GET(hw->cmdFifoDepth);
#endif

    /* Reset fifo ptr to start */ 
    gc->cmdTransportInfo.fifoPtr = gc->cmdTransportInfo.fifoStart;

    /*  We havn't really fenced here, but we set the lastFence for */
    /*  later calculations  */  
    gc->cmdTransportInfo.lastFence = gc->cmdTransportInfo.fifoStart;

#if GLIDE_USE_DEBUG_FIFO && !HAL_CSIM
    {
      FxU32* fifoPtr = gc->cmdTransportInfo.fifoShadowPtr;

      while(fifoPtr < gc->cmdTransportInfo.fifoShadowBase + (kDebugFifoSize >> 2)) 
        *fifoPtr++ = 0x00UL;
      gc->cmdTransportInfo.fifoShadowPtr = gc->cmdTransportInfo.fifoShadowBase;
    }
#endif

    GDBG_INFO(gc->myLevel + 10, "  Post-Wrap: (0x%X : 0x%X) : 0x%X\n", 
              gc->cmdTransportInfo.roomToReadPtr, gc->cmdTransportInfo.roomToEnd,
              gc->cmdTransportInfo.fifoRead);

    goto again;
  }
  
  /* compute room left */
  gc->cmdTransportInfo.fifoRoom = MIN(gc->cmdTransportInfo.roomToReadPtr, gc->cmdTransportInfo.roomToEnd);

#if GDBG_INFO_ON  
  GDBG_INFO(gc->myLevel, FN_NAME"_Done:\n"
            "\tfifoBlock: (0x%X : 0x%X)\n"
            "\tfifoRoom: (0x%X : 0x%X : 0x%X)\n"
            "\tfifo hw: (0x%X : 0x%X) : (0x%X : 0x%X : 0x%X)\n",
            (FxU32)gc->cmdTransportInfo.fifoPtr, blockSize,
            gc->cmdTransportInfo.roomToReadPtr, 
            gc->cmdTransportInfo.roomToEnd, gc->cmdTransportInfo.fifoRoom,
            HW_FIFO_PTR(FXTRUE), gc->cmdTransportInfo.fifoRead, 
            GR_CAGP_GET(depth), GR_CAGP_GET(holeCount), GR_GET(hw->status));
#endif /* GDBG_INFO_ON */

  /* NB: Work around for the buffer swap pending bug in the status
   * register.  All of the logic for keeping track of that is in
   * grBufferNumPending() 
   */
  grBufferNumPending();
  
  FIFO_ASSERT();
  GR_TRACE_EXIT(FN_NAME);
#undef FN_NAME
}

void
_grH3FifoDump_Linear(const FxU32* const linearPacketAddr)
{
#ifdef GDBG_INFO_ON
  FXUNUSED(h3SstIORegNames);
#endif
}


FxU32
_grHwFifoPtr(FxBool ignored)
{
  FxU32 rVal;
#if 1
  FxU32 status, readPtrL1, readPtrL2;
  GR_DCL_GC;

  FXUNUSED(ignored);

  do {
    readPtrL1 = GET(gc->cRegs->cmdFifo0.readPtrL);
    status = GET(gc->ioRegs->status);
    readPtrL2 = GET(gc->cRegs->cmdFifo0.readPtrL);
  } while (readPtrL1 != readPtrL2);
  rVal = ((FxU32)gc->cmdTransportInfo.fifoStart) + readPtrL2 - 
    (FxU32) gc->cmdTransportInfo.fifoOffset;
#else 
  GR_DCL_GC;
  rVal =
    (FxU32) gc->cmdTransportInfo.fifoStart +
    GET(gc->cRegs->cmdFifo0.readPtrL) -
    gc->cmdTransportInfo.fifoOffset;
#endif
  return rVal;

} /* _grHwFifoPtr */

FxU32
_grHwFifoPtrSlave(FxU32 slave, FxBool ignored)
{
  FxU32 rVal = 0;

  FxU32 status, readPtrL1, readPtrL2;
  GR_DCL_GC;

  FXUNUSED(ignored);

  /* V56K-SLAVE-GUARD: these pointers are MMIO into a slave chip.  If the
  ** slave was never mapped (or a remap left the cache stale) the reads
  ** below hit an unmapped kernel address and reset the box.  Report "no
  ** progress" instead -- the caller already treats that as a stall. */
  if ((slave == 0) || (slave > 3) ||
      (gc->slaveCRegs[slave-1] == NULL) || (gc->slaveSstRegs[slave-1] == NULL)) {
    static int v56kWarned = 0;
    if (v56kWarned < 8) {
      v56kWarned++;
      _grSliLog("SLAVE-UNMAPPED slave=%u cregs=%p sstregs=%p chips=%u\n",
                slave,
                (void *)(slave && slave <= 3 ? gc->slaveCRegs[slave-1] : NULL),
                (void *)(slave && slave <= 3 ? gc->slaveSstRegs[slave-1] : NULL),
                gc->chipCount);
    }
    return (FxU32)gc->cmdTransportInfo.fifoStart;
  }

  do {
    readPtrL1 = GET(gc->slaveCRegs[slave-1]->cmdFifo0.readPtrL);

    status = GET(gc->slaveSstRegs[slave-1]->status);

    readPtrL2 = GET(gc->slaveCRegs[slave-1]->cmdFifo0.readPtrL);
  } while (readPtrL1 != readPtrL2);

  rVal = (((FxU32)gc->cmdTransportInfo.fifoStart) + 
          readPtrL2 - 
          (FxU32)gc->cmdTransportInfo.fifoOffset);
  
  return rVal;
} /* _grHwFifoPtr */

#ifdef _HW_SECRET_
/* don't kill me Chris.  This function isn't exported */
/* it isn't in the header files or anything */
/* it automatically should be removed in the linker, */
/* for normal glide apps. */
typedef struct
{
   SstCRegs    *CRegs;
   SstGRegs    *Regs2D;
   SstRegs     *Regs3D;
   FxU32       num_color_buffer;
   FxU32       num_aux_buffer;
   FxU32       buffer_addr[5];
   FxU32       buffer_stride;
   FxU32       tram_offset;
}HwSecretInterface;

extern void
_grGetCommandTransportInfo(struct cmdTransportInfo* info,HwSecretInterface *the_data)
{
  FxU32 i,j;
  GR_DCL_GC;

  GR_ASSERT(info != NULL);
  *info = gc->cmdTransportInfo;

  the_data->CRegs=gc->cRegs;
  the_data->Regs2D=gc->gRegs;
  the_data->Regs3D=gc->sstRegs;
  the_data->num_color_buffer=gc->grColBuf;
  the_data->num_aux_buffer=gc->grAuxBuf;

  for (i=0;i<the_data->num_color_buffer;i++)
  {
    the_data->buffer_addr[i]=gc->buffers[i];
  }
  for (j=0;j<the_data->num_aux_buffer;i++,j++)
  {
    the_data->buffer_addr[i]=gc->buffers[i];
  }

  the_data->buffer_stride=gc->strideInTiles | SST_BUFFER_MEMORY_TILED;

  the_data->tram_offset=gc->tramOffset;
}

extern void
_grSetCommandTransportInfo(struct cmdTransportInfo* info)
{
   GR_DCL_GC;

   gc->cmdTransportInfo.fifoPtr=info->fifoPtr;
   gc->cmdTransportInfo.fifoRoom=0;
}
#endif _HW_SECRET_

#endif /* USE_PACKET_FIFO */

