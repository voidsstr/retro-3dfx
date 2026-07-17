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
 ** $Header: cvg.c, 2, 10/11/00 8:21:01 PM, Brent$
 ** $Log: 
 **  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
 **       branching.
 **  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
 ** $
** 
** 36    12/03/98 11:27p Dow
** Code 'cleanup' heç
 * 
 * 34    11/06/97 3:46p Peter
 * dos ovl build problem
 * 
 * 33    11/04/97 5:04p Peter
 * cataclysm part deux
 * 
 * 32    11/03/97 3:43p Peter
 * h3/cvg cataclysm
 * 
 * 31    11/01/97 10:01a Peter
 * tri dispatch stuff
 * 
 * 30    10/29/97 4:59p Peter
 * fixed csim/non-debug stupidity
 * 
 * 29    10/23/97 5:27p Peter
 * sli fifo thing
 * 
 * 28    10/20/97 5:07p Peter
 * gd.120 debugging fix
 * 
 * 27    10/19/97 10:59a Peter
 * gd.120 trace w/o simulator
 * 
 * 26    10/16/97 10:35a Peter
 * fixed gdbg_level==0 thing
 * 
 * 25    10/08/97 11:31a Peter
 * debugging messages
 * 
 * 24    9/30/97 1:03p Peter
 * more debugging code
 * 
 * 23    9/24/97 1:28p Peter
 * assert if grXX call inside lock/unlock
 * 
 * 22    9/20/97 4:41p Peter
 * tri_setf fixup/big fifo
 * 
 * 21    9/20/97 10:53a Peter
 * keep track of palette stats
 * 
 * 20    9/15/97 7:31p Peter
 * more cmdfifo cleanup, fixed normal buffer clear, banner in the right
 * place, lfb's are on, Hmmmm.. probably more
 * 
 * 19    9/10/97 10:13p Peter
 * fifo logic from GaryT, non-normalized fp first cut
 * 
 * 18    9/03/97 2:11p Peter
 * start gdbg_info cleanup, fixed zero area no-tsu triangle effage
 * 
 * 17    8/18/97 3:52p Peter
 * pre-hw arrival fixes/cleanup
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

#if (GLIDE_PLATFORM & GLIDE_HW_CVG)
#if GDBG_INFO_ON

const char*
cvgRegNames[] = {
  "status",                     /* 0x00 */
  "intrCtrl",                   /* 0x01 */
  "vAx",                        /* 0x02 */
  "vAy",                        /* 0x03 */
  "vBx",                        /* 0x04 */
  "vBy",                        /* 0x05 */
  "vCx",                        /* 0x06 */
  "vCy",                        /* 0x07 */
  "r",                          /* 0x08 */
  "g",                          /* 0x09 */
  "b",                          /* 0x0A */
  "z",                          /* 0x0B */
  "a",                          /* 0x0C */
  "s",                          /* 0x0D */
  "t",                          /* 0x0E */
  "w",                          /* 0x0F */
  "drdx",                       /* 0x10 */
  "dgdx",                       /* 0x11 */
  "dbdx",                       /* 0x12 */
  "dzdx",                       /* 0x13 */
  "dadx",                       /* 0x14 */
  "dsdx",                       /* 0x15 */
  "dtdx",                       /* 0x16 */
  "dwdx",                       /* 0x17 */
  "drdy",                       /* 0x18 */
  "dgdy",                       /* 0x19 */
  "dbdy",                       /* 0x1A */
  "dzdy",                       /* 0x1B */
  "dady",                       /* 0x1C */
  "dsdy",                       /* 0x1D */
  "dtdy",                       /* 0x1E */
  "dwdy",                       /* 0x1F */
  "triangleCMD",                /* 0x20 */
  "reserved021",                /* 0x21 */
  "FvAx",                       /* 0x22 */
  "FvAy",                       /* 0x23 */
  "FvBx",                       /* 0x24 */
  "FvBy",                       /* 0x25 */
  "FvCx",                       /* 0x26 */
  "FvCy",                       /* 0x27 */
  "Fr",                         /* 0x28 */
  "Fg",                         /* 0x29 */
  "Fb",                         /* 0x2A */
  "Fz",                         /* 0x2B */
  "Fa",                         /* 0x2C */
  "Fs",                         /* 0x2D */
  "Ft",                         /* 0x2E */
  "Fw",                         /* 0x2F */
  "Fdrdx",                      /* 0x30 */
  "Fdgdx",                      /* 0x31 */
  "Fdbdx",                      /* 0x32 */
  "Fdzdx",                      /* 0x33 */
  "Fdadx",                      /* 0x34 */
  "Fdsdx",                      /* 0x35 */
  "Fdtdx",                      /* 0x36 */
  "Fdwdx",                      /* 0x37 */
  "Fdrdy",                      /* 0x38 */
  "Fdgdy",                      /* 0x39 */
  "Fdbdy",                      /* 0x3A */
  "Fdzdy",                      /* 0x3B */
  "Fdady",                      /* 0x3C */
  "Fdsdy",                      /* 0x3D */
  "Fdtdy",                      /* 0x3E */
  "Fdwdy",                      /* 0x3F */
  "FtriangleCMD",               /* 0x40 */
  "fbzColorPath",               /* 0x41 */
  "fogMode",                    /* 0x42 */
  "alphaMode",                  /* 0x43 */
  "fbzMode",                    /* 0x44 */
  "lfbMode",                    /* 0x45 */
  "clipLeftRight",              /* 0x46 */
  "clipLowYHighY",              /* 0x47 */
  "nopCMD",                     /* 0x48 */
  "fastfillCMD",                /* 0x49 */
  "swapbufferCMD",              /* 0x4A */
  "fogColor",                   /* 0x4B */
  "zaColor",                    /* 0x4C */
  "chromaKey",                  /* 0x4D */
  "chromaRange",                /* 0x4E */
  "userIntrCMD",                /* 0x4F */
  "stipple",                    /* 0x50 */
  "color0",                     /* 0x51 */
  "color1",                     /* 0x52 */
  "fbiPixelsIn",                /* 0x53 */
  "fbiChromaFail",              /* 0x54 */
  "fbiZfuncFail",               /* 0x55 */
  "fbiAfuncFail",               /* 0x56 */
  "fbiPixelsOut",               /* 0x57 */
  "fogTable0001",               /* 0x58 */
  "fogTable0203",               /* 0x59 */
  "fogTable0405",               /* 0x5A */
  "fogTable0607",               /* 0x5B */
  "fogTable0809",               /* 0x5C */
  "fogTable0A0B",               /* 0x5D */
  "fogTable0C0D",               /* 0x5E */
  "fogTable0E0F",               /* 0x5F */
  "fogTable1011",               /* 0x60 */
  "fogTable1213",               /* 0x61 */
  "fogTable1415",               /* 0x62 */
  "fogTable1617",               /* 0x63 */
  "fogTable1819",               /* 0x64 */
  "fogTable1A1B",               /* 0x65 */
  "fogTable1C1D",               /* 0x66 */
  "fogTable1E1F",               /* 0x67 */
  "fogTable2021",               /* 0x68 */
  "fogTable2223",               /* 0x69 */
  "fogTable2425",               /* 0x6A */
  "fogTable2627",               /* 0x6B */
  "fogTable2829",               /* 0x6C */
  "fogTable2A2B",               /* 0x6D */
  "fogTable2C2D",               /* 0x6E */
  "fogTable2E2F",               /* 0x6F */
  "fogTable3031",               /* 0x70 */
  "fogTable3233",               /* 0x71 */
  "fogTable3435",               /* 0x72 */
  "fogTable3637",               /* 0x73 */
  "fogTable3839",               /* 0x74 */
  "fogTable3A3B",               /* 0x75 */
  "fogTable3C3D",               /* 0x76 */
  "fogTable3E3F",               /* 0x77 */
  "cmdFifoBaseAddr",            /* 0x78 */
  "cmdFifoBump",                /* 0x79 */
  "cmdFifoRdPtr",               /* 0x7A */
  "cmdFifoAMin",                /* 0x7B */
  "cmdFifoAMax",                /* 0x7C */
  "cmdFifoDepth",               /* 0x7D */
  "cmdFifoHoles",               /* 0x7E */
  "reserved07F",                /* 0x7F */
  "fbiInit4",                   /* 0x80 */
  "vRetrace",                   /* 0x81 */
  "backPorch",                  /* 0x82 */
  "videoDim",                   /* 0x83 */
  "fbiInit0",                   /* 0x84 */
  "fbiInit1",                   /* 0x85 */
  "fbiInit2",                   /* 0x86 */
  "fbiInit3",                   /* 0x87 */
  "hSync",                      /* 0x88 */
  "vSync",                      /* 0x89 */
  "clutData",                   /* 0x8A */
  "dacData",                    /* 0x8B */
  "maxRGBDelta",                /* 0x8C */
  "hBorder",                    /* 0x8D */
  "vBorder",                    /* 0x8E */
  "borderColor",                /* 0x8F */
  "hvRetrace",                  /* 0x90 */
  "fbiInit5",                   /* 0x91 */
  "fbiInit6",                   /* 0x92 */
  "fbiInit7",                   /* 0x93 */
  "reserved094",                /* 0x94 */
  "reserved095",                /* 0x95 */
  "fbiSwapHistory",             /* 0x96 */
  "fbiTriOut",                  /* 0x97 */
  "sSetupMode",                 /* 0x98 */
  "sVx",                        /* 0x99 */
  "sVy",                        /* 0x9A */
  "sARGB",                      /* 0x9B */
  "sRed",                       /* 0x9C */
  "sGreen",                     /* 0x9D */
  "sBlue",                      /* 0x9E */
  "sAlpha",                     /* 0x9F */
  "sVz",                        /* 0xA0 */
  "sWb",                        /* 0xA1 */
  "sWtmu0",                     /* 0xA2 */
  "sS/W0",                      /* 0xA3 */
  "sT/W0",                      /* 0xA4 */
  "sWtmu1",                     /* 0xA5 */
  "sS/Wtmu1",                   /* 0xA6 */
  "sT/Wtmu1",                   /* 0xA7 */
  "sDrawTriCMD",                /* 0xA8 */
  "sBeginTriCMD",               /* 0xA9 */
  "reserved0AA",                /* 0xAA */
  "reserved0AB",                /* 0xAB */
  "reserved0AC",                /* 0xAC */
  "reserved0AD",                /* 0xAD */
  "reserved0AE",                /* 0xAE */
  "reserved0AF",                /* 0xAF */
  "bltSrcBase",                 /* 0xB0 */
  "bltDstBase",                 /* 0xB1 */
  "bltXYStrides",               /* 0xB2 */
  "bltSrcChroma",               /* 0xB3 */
  "bltDstChroma",               /* 0xB4 */
  "bltClipX",                   /* 0xB5 */
  "bltClipY",                   /* 0xB6 */
  "reserved067",                /* 0xB7 */
  "bltSrcXY",                   /* 0xB8 */
  "bltDstXY",                   /* 0xB9 */
  "bltSize",                    /* 0xBA */
  "bltRop",                     /* 0xBB */
  "bltColor",                   /* 0xBC */
  "reserved067",                /* 0xBD */
  "bltCommand",                 /* 0xBE */
  "bltData",                    /* 0xBF */
  "textureMode",                /* 0xc0 */
  "tLOD",                       /* 0xc1 */
  "tDetail",                    /* 0xc2 */
  "texBaseAddr",                /* 0xc3 */
  "texBaseAddr1",               /* 0xc4 */
  "texBaseAddr2",               /* 0xc5 */
  "texBaseAddr38",              /* 0xc6 */
  "trexInit0",                  /* 0xc7 */
  "trexInit1",                  /* 0xc8 */
  "nccTable0_0",                /* 0xc9 */
  "nccTable0_1",                /* 0xca */
  "nccTable0_2",                /* 0xcb */
  "nccTable0_3",                /* 0xcc */
  "nccTable0_4",                /* 0xcd */
  "nccTable0_5",                /* 0xce */
  "nccTable0_6",                /* 0xcf */
  "nccTable0_7",                /* 0xd0 */
  "nccTable0_8",                /* 0xd1 */
  "nccTable0_9",                /* 0xd2 */
  "nccTable0_A",                /* 0xd3 */
  "nccTable0_B",                /* 0xd4 */
  "nccTable1_0",                /* 0xd5 */
  "nccTable1_1",                /* 0xd6 */
  "nccTable1_2",                /* 0xd7 */
  "nccTable1_3",                /* 0xd8 */
  "nccTable1_4",                /* 0xd9 */
  "nccTable1_5",                /* 0xda */
  "nccTable1_6",                /* 0xdb */
  "nccTable1_7",                /* 0xdc */
  "nccTable1_8",                /* 0xdd */
  "nccTable1_9",                /* 0xde */
  "nccTable1_A",                /* 0xdf */
  "nccTable1_B",                /* 0xe0 */
  "reserved0E1",                /* 0xe1 */
  "reserved0E2",                /* 0xe2 */
  "reserved0E3",                /* 0xe3 */
  "reserved0E4",                /* 0xe4 */
  "reserved0E5",                /* 0xe5 */
  "reserved0E6",                /* 0xe6 */
  "reserved0E7",                /* 0xe7 */
  "reserved0E8",                /* 0xe8 */
  "reserved0E9",                /* 0xe9 */
  "reserved0EA",                /* 0xea */
  "reserved0EB",                /* 0xeb */
  "reserved0EC",                /* 0xec */
  "reserved0ED",                /* 0xed */
  "reserved0EE",                /* 0xee */
  "reserved0EF",                /* 0xef */
  "reserved0F0",                /* 0xf0 */
  "reserved0F1",                /* 0xf1 */
  "reserved0F2",                /* 0xf2 */
  "reserved0F3",                /* 0xf3 */
  "reserved0F4",                /* 0xf4 */
  "reserved0F5",                /* 0xf5 */
  "reserved0F6",                /* 0xf6 */
  "reserved0F7",                /* 0xf7 */
  "reserved0F8",                /* 0xf8 */
  "reserved0F9",                /* 0xf9 */
  "reserved0FA",                /* 0xfa */
  "reserved0FB",                /* 0xfb */
  "reserved0FC",                /* 0xfc */
  "reserved0FD",                /* 0xfd */
  "reserved0FE",                /* 0xfe */
  "reserved0FF",                /* 0xff */  
};

#define GEN_INDEX(a) ((((FxU32) a) - ((FxU32) gc->reg_ptr)) >> 2)

void
_grFifoWriteDebug(FxU32 addr, FxU32 val, FxU32 fifoPtr)
{
  GR_DCL_GC;
  FxU32 index = GEN_INDEX(addr);

  GDBG_INFO(gc->myLevel + 199, "Storing to FIFO:\n");
  GDBG_INFO(gc->myLevel + 199, "  FIFO Ptr:    0x%x : 0x%X\n", fifoPtr, gc->cmdTransportInfo.fifoRoom);  
  if (index <= 0xff) { 
    GDBG_INFO(gc->myLevel + 199, "  Reg Name:    %s\n", cvgRegNames[index]);
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
  GDBG_INFO(120, "        SET(0x%X, %ld(0x%X)) 0 %s\n", 
            0x10000000UL + (FxU32)(index << 2), val, val, 
            cvgRegNames[index & 0xFF]);
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
    GDBG_INFO(gc->myLevel + 200, "  Reg Name:    %s\n", cvgRegNames[index]);
    GDBG_INFO(gc->myLevel + 200, "  Reg Num:     0x%x\n", index);
  }
  GDBG_INFO(gc->myLevel + 200, "  Value:       %4.2f\n", val);

#if !HAL_CSIM
  GDBG_INFO(120, "        SET(0x%X, %4.2f (0x%X)) 0 %s\n", 
            0x10000000UL + (FxU32)(index << 2), val, *(const FxU32*)&val, 
            cvgRegNames[index & 0xFF]);
#endif /* !HAL_CSIM */
} /* _grFifoFWriteDebug */

extern void
_grCVGFifoDump_TriHdr(const FxU32 hdrVal)
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
  
  if (GDBG_GET_DEBUGLEVEL(85)) {
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

extern void
_grCVGFifoDump_Linear(const FxU32* const linearPacketAddr)
{

}

void
_grErrorCallback(const char* const procName,
                 const char* const format,
                 va_list           args)
{
#if defined(__WIN32__)
  {
    GR_DCL_GC;
    GR_DCL_HW;
    static char errMsgBuf[1024];
    
    vsprintf(errMsgBuf, format, args);
    MessageBox(NULL, errMsgBuf, procName, MB_OK);
  }
#endif
    
  if (getenv("FX_GD_ERROR_FAIL") != NULL) ASSERT(0);
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
  GR_DCL_GC;

  GR_SET_EXPECTED_SIZE(sizeof(FxU32), 1);
  GR_SET(*sstAddr, val);
  GR_CHECK_SIZE();
}

extern FxU32
_grGet32(volatile FxU32* const sstAddr)
{
  return GR_GET(*sstAddr); 
}

#if FIFO_ASSERT_FULL
const FxU32 kFifoCheckMask = 0xFFFFFF;
FxU32 gFifoCheckCount = 0;
#endif

void GR_CDECL
_FifoMakeRoom(const FxI32 blockSize, const char* fName, const int fLine)
{
#define FN_NAME "_FifoMakeRoom"
  GR_BEGIN_NOFIFOCHECK(FN_NAME, 400);

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

#if !GLIDE_INIT_HAL
    if (gc->scanline_interleaved) {
      const SstRegs* slaveHw = (const SstRegs*)gc->slave_ptr;

      GDBG_INFO_MORE(gc->myLevel, "\tsli: 0x%X : (0x%X : 0x%X : 0x%X)\n",
                     HW_FIFO_PTR(FXFALSE), 
                     GR_GET(slaveHw->cmdFifoDepth), 
                     GR_GET(slaveHw->cmdFifoHoles), 
                     GR_GET(slaveHw->status));
    }
#endif /* !GLIDE_INIT_HAL */
#endif /* GDBG_INFO_ON */

    ASSERT_THIS_SHIT((gc->cmdTransportInfo.roomToReadPtr >= 0) && (gc->cmdTransportInfo.roomToEnd >= 0));
  }

again:
  /* do we need to stall? */
  {
    FxU32 lastHwRead = gc->cmdTransportInfo.fifoRead;
    FxI32 roomToReadPtr = gc->cmdTransportInfo.roomToReadPtr;

    while (roomToReadPtr < blockSize) {
      FxU32 curReadPtr = HW_FIFO_PTR(FXTRUE);
      FxU32 curReadDist = curReadPtr - lastHwRead;

#if !GLIDE_INIT_HAL
      /* If we're in an sli configuration then we need to make sure
       * that we don't catch up to either the slave as well.
       */
      if (gc->scanline_interleaved) {
        const FxU32 slaveReadPtr = HW_FIFO_PTR(FXFALSE);
        const FxU32 slaveReadDist = (slaveReadPtr - lastHwRead);

        /* Is the slave closer than the master? */
        if (slaveReadDist < curReadDist) {
#if GDBG_INFO_ON  
          {
            const SstRegs* slaveHw = (const SstRegs*)gc->slave_ptr; 
            GDBG_INFO(gc->myLevel, "  Wait sli: 0x%X : (0x%X : 0x%X : 0x%X)\n"
                      "\tMaster: 0x%X : 0x%X\n"
                      "\tSlave : 0x%X : 0x%X\n",
                      HW_FIFO_PTR(FXFALSE), 
                      GR_GET(slaveHw->cmdFifoDepth), 
                      GR_GET(slaveHw->cmdFifoHoles), 
                      GR_GET(slaveHw->status),
                      curReadPtr, curReadDist,
                      slaveReadPtr, slaveReadDist);
          }
#endif /* GDBG_INFO_ON */

          curReadDist = slaveReadDist;
          curReadPtr = slaveReadPtr;
        }
      }
#endif /* !GLIDE_INIT_HAL */

      roomToReadPtr += curReadDist;

      _GlideRoot.stats.fifoStalls++;
      _GlideRoot.stats.fifoStallDepth += GR_GET(hw->cmdFifoDepth);

      /* Have we wrapped yet? */
      if (lastHwRead > curReadPtr) roomToReadPtr += (FxI32)gc->cmdTransportInfo.fifoSize - FIFO_END_ADJUST;
      lastHwRead = curReadPtr;
    }

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

    /* Set the packet. This must be fenced. */
    FIFO_ASSERT();
    {
      SET(*gc->cmdTransportInfo.fifoPtr, gc->cmdTransportInfo.fifoJmpHdr); 
      P6FENCE;
    }
    FIFO_ASSERT();

    /* Update roomXXX fields for the actual wrap */
    gc->cmdTransportInfo.roomToReadPtr -= gc->cmdTransportInfo.roomToEnd;
    gc->cmdTransportInfo.roomToEnd = gc->cmdTransportInfo.fifoSize - FIFO_END_ADJUST;

    _GlideRoot.stats.fifoWraps++;
    _GlideRoot.stats.fifoWrapDepth += GR_GET(hw->cmdFifoDepth);

    /* Reset fifo ptr to start */ 
    gc->cmdTransportInfo.fifoPtr = gc->cmdTransportInfo.fifoStart;

#if GLIDE_USE_DEBUG_FIFO && !HAL_CSIM
    gc->cmdTransportInfo.fifoShadowPtr = gc->cmdTransportInfo.fifoShadowBase;
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
            GR_GET(hw->cmdFifoDepth), GR_GET(hw->cmdFifoHoles), GR_GET(hw->status));

#if !GLIDE_INIT_HAL
  if (gc->scanline_interleaved) {
    const SstRegs* slaveHw = (const SstRegs*)gc->slave_ptr;
    
    GDBG_INFO_MORE(gc->myLevel, "\tsli: 0x%X : (0x%X : 0x%X : 0x%X)\n",
                   HW_FIFO_PTR(FXFALSE), 
                   GR_GET(slaveHw->cmdFifoDepth), 
                   GR_GET(slaveHw->cmdFifoHoles), 
                   GR_GET(slaveHw->status));
  }
#endif /* !GLIDE_INIT_HAL */
#endif /* GDBG_INFO_ON */
  
  FIFO_ASSERT();
#undef FN_NAME
}
#endif /* USE_PACKET_FIFO */
#endif /* (GLIDE_PLATFORM & GLIDE_HW_CVG) */
